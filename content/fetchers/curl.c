/*
 * Copyright 2006-2019 Daniel Silverstone <dsilvers@digital-scurf.org>
 * Copyright 2010-2018 Vincent Sanders <vince@netsurf-browser.org>
 * Copyright 2007 James Bursa <bursa@users.sourceforge.net>
 *
 * This file is part of NetSurf.
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 * implementation of fetching of data from http and https schemes.
 *
 * This implementation uses libcurl's 'multi' interface.
 *
 * The CURL handles are cached in the curl_handle_ring.
 */

/* must come first to ensure winsock2.h vs windows.h ordering issues */
#அந்தர்பாவ "utils/inet.h"

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <inttypes.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <sys/stat.h>

#அந்தர்பாவ <libwapcaplet/libwapcaplet.h>
#அந்தர்பாவ <nsutils/time.h>

#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/hashmap.h"
#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/ring.h"
#அந்தர்பாவ "utils/useragent.h"
#அந்தர்பாவ "utils/file.h"
#அந்தர்பாவ "utils/string.h"
#அந்தர்பாவ "netsurf/fetch.h"
#அந்தர்பாவ "netsurf/misc.h"
#அந்தர்பாவ "desktop/gui_internal.h"

#அந்தர்பாவ "content/fetch.h"
#அந்தர்பாவ "content/fetchers.h"
#அந்தர்பாவ "content/fetchers/curl.h"
#அந்தர்பாவ "content/urldb.h"

/**
 * maximum number of progress notifications per second
 */
#define UPDATES_PER_SECOND 2

/**
 * The ciphersuites the browser is prepared to use for TLS1.3
 */
#define CIPHER_SUITES						\
	"TLS_AES_256_GCM_SHA384:"				\
	"TLS_CHACHA20_POLY1305_SHA256:"				\
	"TLS_AES_128_GCM_SHA256"

/**
 * The ciphersuites the browser is prepared to use for TLS<1.3
 */
#define CIPHER_LIST						\
	/* disable everything */				\
	"-ALL:"							\
	/* enable TLSv1.2 ECDHE AES GCM suites */		\
	"EECDH+AESGCM+TLSv1.2:"					\
	/* enable ECDHE CHACHA20/POLY1305 suites */		\
	"EECDH+CHACHA20:"					\
	/* Sort above by strength */				\
	"@STRENGTH:"						\
	/* enable ECDHE (auth=RSA, mac=SHA1) AES CBC suites */	\
	"EECDH+aRSA+AES+SHA1"

/**
 * The legacy cipher suites the browser is prepared to use for TLS<1.3
 */
#define CIPHER_LIST_LEGACY					\
	/* as above */						\
	CIPHER_LIST":"						\
	/* enable (non-PFS) RSA AES GCM suites */		\
	"RSA+AESGCM:"						\
	/* enable (non-PFS) RSA (mac=SHA1) AES CBC suites */	\
	"RSA+AES+SHA1"

/* Open SSL compatability for certificate handling */
#ifdef WITH_OPENSSL

#அந்தர்பாவ <openssl/ssl.h>
#அந்தர்பாவ <openssl/x509v3.h>

#இல்லையெனில் /* WITH_OPENSSL */

வகைவரை எழுத்து X509;

நிலையான வெறுமை X509_free(X509 *cert)
{
	விடுவி(cert);
}

#endif /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
நிலையான uint32_t
curl_fetch_ssl_சாவி_hash(வெறுமை *சாவி)
{
	nsurl *url = சாவி;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *port = nsurl_get_component(url, NSURL_PORT);
	uint32_t hash;

	எனில் (port == வெற்று)
		port = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_மதிப்பு(hostname) ^ lwc_string_hash_மதிப்பு(port);

	lwc_string_unref(hostname);
	lwc_string_unref(port);

	திரும்பு hash;
}

/* We only compare the hostname and port */
நிலையான bool
curl_fetch_ssl_சாவி_eq(வெறுமை *key1, வெறுமை *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_PORT);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_PORT);
	bool iseq = false;

	எனில் (port1 == வெற்று)
		port1 = lwc_string_ref(corestring_lwc_443);
	எனில் (port2 == வெற்று)
		port2 = lwc_string_ref(corestring_lwc_443);

	எனில் (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		செல் out;

	iseq = false;
	எனில் (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		செல் out;

out:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	திரும்பு iseq;
}

நிலையான வெறுமை *
curl_fetch_ssl_மதிப்பு_alloc(வெறுமை *சாவி)
{
	கட்டமைப்பு cert_chain *out;

	எனில் (cert_chain_alloc(0, &out) != NSERROR_OK) {
		திரும்பு வெற்று;
	}

	திரும்பு out;
}

நிலையான வெறுமை
curl_fetch_ssl_மதிப்பு_destroy(வெறுமை *மதிப்பு)
{
	கட்டமைப்பு cert_chain *chain = மதிப்பு;
	எனில் (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

நிலையான hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.சாவி_clone = (hashmap_சாவி_clone_t)nsurl_ref,
	.சாவி_destroy = (hashmap_சாவி_destroy_t)nsurl_unref,
	.சாவி_eq = curl_fetch_ssl_சாவி_eq,
	.சாவி_hash = curl_fetch_ssl_சாவி_hash,
	.மதிப்பு_alloc = curl_fetch_ssl_மதிப்பு_alloc,
	.மதிப்பு_destroy = curl_fetch_ssl_மதிப்பு_destroy,
};

நிலையான hashmap_t *curl_fetch_ssl_hashmap = வெற்று;

/** SSL certificate info */
கட்டமைப்பு cert_info {
	X509 *cert;		/**< Pointer to certificate */
	long err;		/**< OpenSSL error code */
};

#எனில் LIBCURL_VERSION_இலக்கம் >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_XFERINFOFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_XFERINFODATA
#define NSCURL_PROGRESS_T curl_off_t
#இல்லையெனில்
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_PROGRESSFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_PROGRESSDATA
#define NSCURL_PROGRESS_T இரட்டை
#endif

#எனில் LIBCURL_VERSION_இலக்கம் >= 0x073800 /* 7.56.0 depricated curl_formadd */
#define NSCURL_POSTDATA_T curl_mime
#define NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#define NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#இல்லையெனில்
#define NSCURL_POSTDATA_T கட்டமைப்பு curl_httppost
#define NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#define NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#endif

/** Information for a single fetch. */
கட்டமைப்பு curl_fetch_info {
	கட்டமைப்பு fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool sent_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_headers;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	கட்டமைப்பு curl_slist *headers;	/**< List of request headers. */
	எழுத்து *location;		/**< Response Location header, or 0. */
	unsigned long content_length;	/**< Response Content-Length, or 0. */
	எழுத்து *cookie_string;	/**< Cookie string for this fetch */
	எழுத்து *realm;		/**< HTTP Auth Realm */
	கட்டமைப்பு fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	long http_code; /**< HTTP result code from cURL. */

	uint64_t last_progress_update;	/**< Time of last progress update */
	முழு cert_depth; /**< deepest certificate in use */
	கட்டமைப்பு cert_info cert_data[MAX_CERT_DEPTH]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
கட்டமைப்பு cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	கட்டமைப்பு cache_handle *r_prev; /**< Previous cached handle in ring. */
	கட்டமைப்பு cache_handle *r_next; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
நிலையான CURL *fetch_blank_curl;

/** Ring of cached handles */
நிலையான கட்டமைப்பு cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
நிலையான முழு curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
நிலையான bool curl_with_openssl;

/** Error buffer for cURL. */
நிலையான எழுத்து fetch_error_buffer[CURL_ERROR_SIZE];

/** Proxy authentication details. */
நிலையான எழுத்து fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
நிலையான bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
நிலையான bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_data(scheme));
	curl_fetchers_registered++;
	திரும்பு true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
நிலையான வெறுமை fetch_curl_finalise(lwc_string *scheme)
{
	கட்டமைப்பு cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_data(scheme));
	எனில் (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		எனில் (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = வெற்று;
	}

	/* Free anything remaining in the cached curl handle ring */
	போது (curl_handle_ring != வெற்று) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
		curl_easy_cleanup(h->handle);
		விடுவி(h);
	}
}


/**
 * Check if this fetcher can fetch a url.
 *
 * \param url The url to check.
 * \return true if the fetcher supports the url else false.
 */
நிலையான bool fetch_curl_can_fetch(மாறிலி nsurl *url)
{
	திரும்பு nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
நிலையான கட்டமைப்பு fetch_postdata *
fetch_curl_alloc_postdata(மாறிலி எழுத்து *post_urlenc,
			  மாறிலி கட்டமைப்பு fetch_multipart_data *post_multipart)
{
	கட்டமைப்பு fetch_postdata *postdata;
	postdata = calloc(1, அளவு(கட்டமைப்பு fetch_postdata));
	எனில் (postdata != வெற்று) {

		எனில் (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->data.urlenc = strdup(post_urlenc);
			எனில் (postdata->data.urlenc == வெற்று) {
				விடுவி(postdata);
				postdata = வெற்று;
			}
		} இல்லையெனில் எனில் (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->data.multipart = fetch_multipart_data_clone(post_multipart);
			எனில் (postdata->data.multipart == வெற்று) {
				விடுவி(postdata);
				postdata = வெற்று;
			}
		} இல்லையெனில் {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	திரும்பு postdata;
}

/**
 * free postdata
 */
நிலையான வெறுமை fetch_curl_free_postdata(கட்டமைப்பு fetch_postdata *postdata)
{
	எனில் (postdata != வெற்று) {
		தேர்வு (postdata->type) {
		நிலை FETCH_POSTDATA_NONE:
			நிறுத்து;
		நிலை FETCH_POSTDATA_URLENC:
			விடுவி(postdata->data.urlenc);
			நிறுத்து;
		நிலை FETCH_POSTDATA_MULTIPART:
			fetch_multipart_data_destroy(postdata->data.multipart);
			நிறுத்து;
		}

		விடுவி(postdata);
	}
}

/**
 *construct a new fetch structure
 */
நிலையான கட்டமைப்பு curl_fetch_info *fetch_alloc(வெறுமை)
{
	கட்டமைப்பு curl_fetch_info *fetch;
	fetch = malloc(அளவு (*fetch));
	எனில் (fetch == வெற்று)
		திரும்பு வெற்று;

	fetch->curl_handle = வெற்று;
	fetch->sent_ssl_chain = false;
	fetch->had_headers = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->headers = வெற்று;
	fetch->url = வெற்று;
	fetch->host = வெற்று;
	fetch->location = வெற்று;
	fetch->content_length = 0;
	fetch->http_code = 0;
	fetch->cookie_string = வெற்று;
	fetch->realm = வெற்று;
	fetch->last_progress_update = 0;
	fetch->postdata = வெற்று;
	fetch->curl_postdata = வெற்று;

	/* Clear certificate chain data */
	memset(fetch->cert_data, 0, அளவு(fetch->cert_data));
	fetch->cert_depth = -1;

	திரும்பு fetch;
}

/**
 * Start fetching data for the given URL.
 *
 * The function returns immediately. The fetch may be queued for later
 * processing.
 *
 * A pointer to an opaque struct curl_fetch_info is returned, which can be
 * passed to fetch_abort() to abort the fetch at any time. Returns 0 if memory
 * is exhausted (or some other fatal error occurred).
 *
 * The caller must supply a callback function which is called when anything
 * interesting happens. The callback function is first called with msg
 * FETCH_HEADER, with the header in data, then one or more times
 * with FETCH_DATA with some data for the url, and finally with
 * FETCH_FINISHED. Alternatively, FETCH_ERROR indicates an error occurred:
 * data contains an error message. FETCH_REDIRECT may replace the FETCH_HEADER,
 * FETCH_DATA, FETCH_FINISHED sequence if the server sends a replacement URL.
 *
 * Some private data can be passed as the last parameter to fetch_start, and
 * callbacks will contain this.
 */
நிலையான வெறுமை *
fetch_curl_setup(கட்டமைப்பு fetch *parent_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 மாறிலி எழுத்து *post_urlenc,
		 மாறிலி கட்டமைப்பு fetch_multipart_data *post_multipart,
		 மாறிலி எழுத்து **headers)
{
	கட்டமைப்பு curl_fetch_info *fetch;
	கட்டமைப்பு curl_slist *slist;
	முழு i;

	fetch = fetch_alloc();
	எனில் (fetch == வெற்று)
		திரும்பு வெற்று;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = parent_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	எனில் (fetch->host == வெற்று) {
		செல் failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	எனில் (fetch->postdata == வெற்று) {
		செல் failed;
	}

#define APPEND(list, மதிப்பு) \
	slist = curl_slist_append(list, மதிப்பு);		\
	எனில் (slist == வெற்று)				\
		செல் failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->headers, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->headers, "Expect:");

	எனில் ((nsoption_charp(accept_language) != வெற்று) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		எழுத்து s[80];
		snprintf(s, அளவு s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[அளவு s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	எனில் (nsoption_charp(accept_charset) != வெற்று &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		எழுத்து s[80];
		snprintf(s, அளவு s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[அளவு s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	எனில் (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->headers, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	க்கு (i = 0; headers[i] != வெற்று; i++) {
		APPEND(fetch->headers, headers[i]);
	}

	திரும்பு fetch;

#undef APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->headers);
	விடுவி(fetch);
	திரும்பு வெற்று;
}


#ifdef WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
நிலையான கட்டமைப்பு cert_chain *
fetch_curl_get_cached_chain(கட்டமைப்பு curl_fetch_info *f)
{
	கட்டமைப்பு cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	எனில் (chain == வெற்று) {
		chain = hashmap_செருகு(curl_fetch_ssl_hashmap, f->url);
	}

	திரும்பு chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
நிலையான வெறுமை
fetch_curl_store_certs_in_cache(கட்டமைப்பு curl_fetch_info *f)
{
	size_t depth;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_DEPTH];
	கட்டமைப்பு cert_chain chain, *cached_chain;
	கட்டமைப்பு cert_info *certs;

	memset(&chain, 0, அளவு(chain));

	certs = f->cert_data;
	chain.depth = f->cert_depth + 1; /* 0 indexed certificate depth */

	க்கு (depth = 0; depth < chain.depth; depth++) {
		எனில் (certs[depth].cert == வெற்று) {
			/* This certificate is missing, skip it */
			chain.certs[depth].err = SSL_CERT_ERR_CERT_MISSING;
			தொடர்;
		}

		/* error code (if any) */
		தேர்வு (certs[depth].err) {
		நிலை X509_V_OK:
			chain.certs[depth].err = SSL_CERT_ERR_OK;
			நிறுத்து;

		நிலை X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			/* fallthrough */
		நிலை X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_சாவி:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_ISSUER;
			நிறுத்து;

		நிலை X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		நிலை X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		நிலை X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		நிலை X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_SIG;
			நிறுத்து;

		நிலை X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		நிலை X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_YOUNG;
			நிறுத்து;

		நிலை X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		நிலை X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_OLD;
			நிறுத்து;

		நிலை X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			chain.certs[depth].err = SSL_CERT_ERR_SELF_SIGNED;
			நிறுத்து;

		நிலை X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			chain.certs[depth].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			நிறுத்து;

		நிலை X509_V_ERR_CERT_REVOKED:
			chain.certs[depth].err = SSL_CERT_ERR_REVOKED;
			நிறுத்து;

		நிலை X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[depth].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			நிறுத்து;

		இயல்பு:
			chain.certs[depth].err = SSL_CERT_ERR_UNKNOWN;
			நிறுத்து;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[depth].cert);
		BIO_get_mem_ptr(mem, &buf[depth]);
		(வெறுமை) BIO_set_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[depth].der = (uint8_t *)buf[depth]->data;
		chain.certs[depth].der_length = buf[depth]->length;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	எனில் (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	க்கு (depth = 0; depth < chain.depth; depth++) {
		எனில் (chain.certs[depth].err == SSL_CERT_ERR_CERT_MISSING) {
			தொடர்;
		}
		எனில் (buf[depth] != வெற்று) {
			BUF_MEM_free(buf[depth]);
		}
	}
}

/**
 * OpenSSL Certificate verification callback
 *
 * Called for each certificate in a chain being verified. OpenSSL
 * calls this in deepest first order from the certificate authority to
 * the peer certificate at position 0.
 *
 * Each certificate is stored in the fetch context the first time it
 * is presented. If an error is encountered it is only returned for
 * the peer certificate at position 0 allowing the enumeration of the
 * entire chain not stopping early at the depth of the erroring
 * certificate.
 *
 * \param verify_ok 0 if the caller has already determined the chain
 *                   has errors else 1
 * \param x509_ctx certificate context being verified
 * \return 1 to indicate verification should continue and 0 to indicate
 *          verification should stop.
 */
நிலையான முழு
fetch_curl_verify_callback(முழு verify_ok, X509_STORE_CTX *x509_ctx)
{
	முழு depth;
	கட்டமைப்பு curl_fetch_info *fetch;

	depth = X509_STORE_CTX_get_error_depth(x509_ctx);
	fetch = X509_STORE_CTX_get_app_data(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	எனில் (depth >= MAX_CERT_DEPTH) {
		X509_STORE_CTX_set_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		திரும்பு 0;
	}

	/* record the max depth */
	எனில் (depth > fetch->cert_depth) {
		fetch->cert_depth = depth;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	எனில் (!fetch->cert_data[depth].cert) {
		fetch->cert_data[depth].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_up_ref(fetch->cert_data[depth].cert);
		fetch->cert_data[depth].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	எனில் (depth > 0) {
		verify_ok = 1;
	} இல்லையெனில் {
		/* search for deeper certificates in the chain with errors */
		க்கு (depth = fetch->cert_depth; depth > 0; depth--) {
			எனில் (fetch->cert_data[depth].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_set_error(x509_ctx, fetch->cert_data[depth].err);
			}
		}
	}

	திரும்பு verify_ok;
}


/**
 * OpenSSL certificate chain verification callback
 *
 * Verifies certificate chain by calling standard implementation after
 * setting up context for the certificate callback.
 *
 * \param x509_ctx The certificate store to validate
 * \param parm The fetch context.
 * \return 1 to indicate verification success and 0 to indicate verification failure.
 */
நிலையான முழு fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, வெறுமை *parm)
{
	கட்டமைப்பு curl_fetch_info *f = (கட்டமைப்பு curl_fetch_info *) parm;
	முழு ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_set_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_data(f->host),
					 lwc_string_length(f->host));

	/* Store fetch struct in context for verify callback */
	எனில் (ok) {
		ok = X509_STORE_CTX_set_app_data(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	எனில் (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_in_cache(f);

	திரும்பு ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
நிலையான CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, வெறுமை *_sslctx, வெறுமை *parm)
{
	கட்டமைப்பு curl_fetch_info *f = (கட்டமைப்பு curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	long options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_set_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	எனில் (f->downgrade_tls) {
		/* Disable TLS 1.3 if the server can't cope with it */
#ifdef SSL_OP_NO_TLSv1_3
		options |= SSL_OP_NO_TLSv1_3;
#endif
#ifdef SSL_MODE_SEND_FALLBACK_SCSV
		/* Ensure server rejects the connection if downgraded too far */
		SSL_CTX_set_mode(sslctx, SSL_MODE_SEND_FALLBACK_SCSV);
#endif
	}

	SSL_CTX_set_options(sslctx, options);

#ifdef SSL_OP_NO_TICKET
	SSL_CTX_clear_options(sslctx, SSL_OP_NO_TICKET);
#endif

	திரும்பு CURLE_OK;
}


#endif /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
நிலையான வெறுமை
fetch_curl_report_certs_upstream(கட்டமைப்பு curl_fetch_info *f)
{
	fetch_செய்தி செய்தி;
	கட்டமைப்பு cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	எனில் (chain != வெற்று) {
		செய்தி.type = FETCH_CERTS;
		செய்தி.data.chain = chain;

		fetch_send_callback(&செய்தி, f->fetch_handle);
	}

	f->sent_ssl_chain = true;
}

#எனில் LIBCURL_VERSION_இலக்கம் >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
கட்டமைப்பு curl_mime_ctx {
	எழுத்து *buffer;
	curl_off_t size;
	curl_off_t position;
};

நிலையான size_t mime_data_read_callback(எழுத்து *buffer, size_t size, size_t nitems, வெறுமை *arg)
{
	கட்டமைப்பு curl_mime_ctx *mctx = (கட்டமைப்பு curl_mime_ctx *) arg;
	curl_off_t sz = mctx->size - mctx->position;

	nitems *= size;
	எனில்(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	எனில்(sz) {
		memcpy(buffer, mctx->buffer + mctx->position, sz);
	}
	mctx->position += sz;
	திரும்பு sz;
}

நிலையான முழு mime_data_seek_callback(வெறுமை *arg, curl_off_t offset, முழு origin)
{
	கட்டமைப்பு curl_mime_ctx *mctx = (கட்டமைப்பு curl_mime_ctx *) arg;

	தேர்வு(origin) {
	நிலை SEEK_END:
		offset += mctx->size;
		நிறுத்து;
	நிலை SEEK_CUR:
		offset += mctx->position;
		நிறுத்து;
	}

	எனில்(offset < 0) {
		திரும்பு CURL_SEEKFUNC_FAIL;
	}
	mctx->position = offset;
	திரும்பு CURL_SEEKFUNC_OK;
}

நிலையான வெறுமை mime_data_free_callback(வெறுமை *arg)
{
	கட்டமைப்பு curl_mime_ctx *mctx = (கட்டமைப்பு curl_mime_ctx *) arg;
	விடுவி(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
நிலையான curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    மாறிலி கட்டமைப்பு fetch_multipart_data *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode code = CURLE_OK;
	size_t மதிப்பு_len;

	cmime = curl_mime_init(chandle);
	எனில் (cmime == வெற்று) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		திரும்பு வெற்று;
	}

	/* iterate post data */
	க்கு (; multipart != வெற்று; multipart = multipart->next) {
		part = curl_mime_addpart(cmime);
		எனில் (part == வெற்று) {
			செல் convert_failed;
		}

		code = curl_mime_name(part, multipart->name);
		எனில் (code != CURLE_OK) {
			செல் convert_failed;
		}

		மதிப்பு_len = strlen(multipart->மதிப்பு);

		எனில் (multipart->file && மதிப்பு_len==0) {
			/* file entries with no filename require special handling */
			code=curl_mime_data(part, multipart->மதிப்பு, மதிப்பு_len);
			எனில் (code != CURLE_OK) {
				செல் convert_failed;
			}

			code = curl_mime_filename(part, "");
			எனில் (code != CURLE_OK) {
				செல் convert_failed;
			}

			code = curl_mime_type(part, "application/octet-stream");
			எனில் (code != CURLE_OK) {
				செல் convert_failed;
			}

		} இல்லையெனில் எனில்(multipart->file) {
			/* file entry */
			nserror ret;
			எழுத்து *leafname = வெற்று;
			எழுத்து *mimetype = வெற்று;

			code = curl_mime_filedata(part, multipart->rawfile);
			எனில் (code != CURLE_OK) {
				செல் convert_failed;
			}

			ret = guit->file->basename(multipart->மதிப்பு, &leafname, வெற்று);
			எனில் (ret != NSERROR_OK) {
				செல் convert_failed;
			}
			code = curl_mime_filename(part, leafname);
			விடுவி(leafname);
			எனில் (code != CURLE_OK) {
				செல் convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->மதிப்பு);
			எனில் (mimetype == வெற்று) {
				mimetype=strdup("text/plain");
			}
			எனில் (mimetype == வெற்று) {
				செல் convert_failed;
			}
			code = curl_mime_type(part, mimetype);
			விடுவி(mimetype);
			எனில் (code != CURLE_OK) {
				செல் convert_failed;
			}

		} இல்லையெனில் {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			கட்டமைப்பு curl_mime_ctx *cb_ctx;
			cb_ctx = malloc(அளவு(கட்டமைப்பு curl_mime_ctx));
			எனில் (cb_ctx == வெற்று) {
				செல் convert_failed;
			}
			cb_ctx->buffer = multipart->மதிப்பு;
			cb_ctx->size = மதிப்பு_len;
			cb_ctx->position = 0;
			code = curl_mime_data_cb(part,
						 மதிப்பு_len,
						 mime_data_read_callback,
						 mime_data_seek_callback,
						 mime_data_free_callback,
						 cb_ctx);
			எனில் (code != CURLE_OK) {
				விடுவி(cb_ctx);
				செல் convert_failed;
			}
		}
	}

	திரும்பு cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", code);
	curl_mime_free(cmime);
	திரும்பு வெற்று;
}

#இல்லையெனில் /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
நிலையான கட்டமைப்பு curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    மாறிலி கட்டமைப்பு fetch_multipart_data *control)
{
	கட்டமைப்பு curl_httppost *post = வெற்று, *last = வெற்று;
	CURLFORMcode code;
	nserror ret;

	க்கு (; control; control = control->next) {
		எனில் (control->file) {
			எழுத்து *leafname = வெற்று;
			ret = guit->file->basename(control->மதிப்பு, &leafname, வெற்று);
			எனில் (ret != NSERROR_OK) {
				தொடர்;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			எனில் (control->மதிப்பு[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				நிலையான எழுத்து buf;

				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_BUFFER, control->மதிப்பு,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_END);
				எனில் (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", code,
					      control->name);
			} இல்லையெனில் {
				எழுத்து *mimetype = guit->fetch->mimetype(control->மதிப்பு);
				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_END);
				எனில் (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      code,
					      control->name,
					      control->மதிப்பு);
				விடுவி(mimetype);
			}
			விடுவி(leafname);
		} இல்லையெனில் {
			code = curl_formadd(&post, &last,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->மதிப்பு,
					    CURLFORM_END);
			எனில் (code != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", code,
				      control->name, control->மதிப்பு);
		}
	}

	திரும்பு post;
}

#endif  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
நிலையான CURLcode fetch_curl_set_postdata(கட்டமைப்பு curl_fetch_info *f)
{
	CURLcode code = CURLE_OK;

#undef SETOPT
#define SETOPT(option, மதிப்பு) { \
	code = curl_easy_setopt(f->curl_handle, option, மதிப்பு);	\
	எனில் (code != CURLE_OK)					\
		திரும்பு code;					\
	}

	தேர்வு (f->postdata->type) {
	நிலை FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, வெற்று);
		SETOPT(NSCURL_POSTDATA_CURLOPT, வெற்று);
		SETOPT(CURLOPT_HTTPGET, 1L);
		நிறுத்து;

	நிலை FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, வெற்று);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->data.urlenc);
		நிறுத்து;

	நிலை FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, வெற்று);
		SETOPT(CURLOPT_HTTPGET, 0L);
		எனில் (f->curl_postdata == வெற்று) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->data.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		நிறுத்து;
	}
	திரும்பு code;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
நிலையான CURLcode fetch_curl_set_options(கட்டமைப்பு curl_fetch_info *f)
{
	CURLcode code;
	மாறிலி எழுத்து *auth;

#undef SETOPT
#define SETOPT(option, மதிப்பு) { \
	code = curl_easy_setopt(f->curl_handle, option, மதிப்பு);	\
	எனில் (code != CURLE_OK)					\
		திரும்பு code;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_PRIVATE, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_PROGRESS_DATA, f);
	SETOPT(CURLOPT_HTTPHEADER, f->headers);
	code = fetch_curl_set_postdata(f);
	எனில் (code != CURLE_OK) {
		திரும்பு code;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	எனில் (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} இல்லையெனில் {
		SETOPT(CURLOPT_COOKIE, வெற்று);
	}

	எனில் ((auth = urldb_get_auth_details(f->url, வெற்று)) != வெற்று) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} இல்லையெனில் {
		SETOPT(CURLOPT_USERPWD, வெற்று);
	}

	/* set up proxy options */
	எனில் (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != வெற்று) &&
	    (strncmp(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (long) nsoption_int(http_proxy_port));

#எனில் LIBCURL_VERSION_இலக்கம் >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#endif

		எனில் (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(long) CURLAUTH_BASIC :
					(long) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					அளவு fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} இல்லையெனில் {
		SETOPT(CURLOPT_PROXY, வெற்று);
	}


	எனில் (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	எனில் (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		எனில் (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, வெற்று);
			SETOPT(CURLOPT_SSL_CTX_DATA, வெற்று);
		}
	} இல்லையெனில் {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef WITH_OPENSSL
		எனில் (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_DATA, f);
		}
#endif
	}

	திரும்பு CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
நிலையான bool
fetch_curl_initiate_fetch(கட்டமைப்பு curl_fetch_info *fetch, CURL *handle)
{
	CURLcode code;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	code = fetch_curl_set_options(fetch);
	எனில் (code != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		திரும்பு false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_add_handle(fetch_curl_multi, fetch->curl_handle);
	ப்ரதிஜ்ஞா(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	திரும்பு true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
நிலையான CURL *fetch_curl_get_handle(lwc_string *host)
{
	கட்டமைப்பு cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	எனில் (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		விடுவி(h);
	} இல்லையெனில் {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	திரும்பு ret;
}


/**
 * Dispatch a single job
 */
நிலையான bool fetch_curl_start(வெறுமை *vfetch)
{
	கட்டமைப்பு curl_fetch_info *fetch = (கட்டமைப்பு curl_fetch_info*)vfetch;
	எனில் (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		திரும்பு false;
	}
	திரும்பு fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
நிலையான வெறுமை fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#எனில் LIBCURL_VERSION_இலக்கம் >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	திரும்பு;
#இல்லையெனில்
	கட்டமைப்பு cache_handle *h = 0;
	முழு c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	எனில் (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		திரும்பு;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(கட்டமைப்பு cache_handle, curl_handle_ring, c);
	எனில் (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		எனில் (curl_handle_ring != வெற்று) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_next;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} இல்லையெனில் {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		திரும்பு;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (கட்டமைப்பு cache_handle*)malloc(அளவு(கட்டமைப்பு cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_செருகு(curl_handle_ring, h);
#endif
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
நிலையான வெறுமை fetch_curl_stop(கட்டமைப்பு curl_fetch_info *f)
{
	CURLMcode codem;

	ப்ரதிஜ்ஞா(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	எனில் (f->curl_handle) {
		/* remove from curl multi handle */
		codem = curl_multi_remove_handle(fetch_curl_multi,
				f->curl_handle);
		ப்ரதிஜ்ஞா(codem == CURLM_OK);
		/* Put this curl handle into the cache if wanted. */
		fetch_curl_cache_handle(f->curl_handle, f->host);
		f->curl_handle = 0;
	}

	fetch_remove_from_queues(f->fetch_handle);
}


/**
 * Abort a fetch.
 */
நிலையான வெறுமை fetch_curl_abort(வெறுமை *vf)
{
	கட்டமைப்பு curl_fetch_info *f = (கட்டமைப்பு curl_fetch_info *)vf;
	ப்ரதிஜ்ஞா(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	எனில் (f->curl_handle) {
		எனில் (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->abort = true;
		} இல்லையெனில் {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} இல்லையெனில் {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
நிலையான வெறுமை fetch_curl_free(வெறுமை *vf)
{
	கட்டமைப்பு curl_fetch_info *f = (கட்டமைப்பு curl_fetch_info *)vf;
	முழு i;

	எனில் (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	விடுவி(f->location);
	விடுவி(f->cookie_string);
	விடுவி(f->realm);
	எனில் (f->headers) {
		curl_slist_free_all(f->headers);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	க்கு (i = 0; i < MAX_CERT_DEPTH; i++) {
		எனில் (f->cert_data[i].cert != வெற்று) {
			X509_free(f->cert_data[i].cert);
		}
	}

	விடுவி(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
நிலையான bool fetch_curl_process_headers(கட்டமைப்பு curl_fetch_info *f)
{
	long http_code;
	CURLcode code;
	fetch_செய்தி செய்தி;

	f->had_headers = true;

	எனில் (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		ப்ரதிஜ்ஞா(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}
	http_code = f->http_code;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_code);

	எனில் ((http_code == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		செய்தி.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&செய்தி, f->fetch_handle);
		திரும்பு true;
	}

	/* handle HTTP redirects (3xx response codes) */
	எனில் (HTTP_RESPONSE_IS_3XX(http_code) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		செய்தி.type = FETCH_REDIRECT;
		செய்தி.data.redirect = f->location;
		fetch_send_callback(&செய்தி, f->fetch_handle);
		திரும்பு true;
	}

	/* handle HTTP 401 (Authentication errors) */
	எனில் (http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		செய்தி.type = FETCH_AUTH;
		செய்தி.data.auth.realm = f->realm;
		fetch_send_callback(&செய்தி, f->fetch_handle);
		திரும்பு true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	எனில் (f->only_2xx &&
	    strncmp(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_code) == false)) {
		செய்தி.type = FETCH_ERROR;
		செய்தி.data.error = messages_get("Not2xx");
		fetch_send_callback(&செய்தி, f->fetch_handle);
		திரும்பு true;
	}

	எனில் (f->abort)
		திரும்பு true;

	திரும்பு false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
நிலையான வெறுமை fetch_curl_done(CURL *curl_handle, CURLcode result)
{
	bool finished = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	கட்டமைப்பு curl_fetch_info *f;
	எழுத்து **_hideous_hack = (எழுத்து **) (வெறுமை *) &f;
	CURLcode code;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	code = curl_easy_getinfo(curl_handle, CURLINFO_PRIVATE, _hideous_hack);
	ப்ரதிஜ்ஞா(code == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	எனில் ((abort_fetch == false) &&
	    (result == CURLE_OK ||
	     ((result == CURLE_WRITE_ERROR) && (f->stopped == false)))) {
		/* fetch completed normally or the server fed us a junk gzip
		 * stream (usually in the form of garbage at the end of the
		 * stream). Curl will have fed us all but the last chunk of
		 * decoded data, which is sad as, if we'd received the last
		 * chunk, too, we'd be able to render the whole object.
		 * As is, we'll just have to accept that the end of the
		 * object will be truncated in this case and leave it to
		 * the content handlers to cope.
		 */
		எனில் (f->stopped ||
		    (!f->had_headers &&	fetch_curl_process_headers(f))) {
			; /* redirect with no body or similar */
		} இல்லையெனில் {
			finished = true;
		}
	} இல்லையெனில் எனில் (result == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		எனில் (!f->had_headers && fetch_curl_process_headers(f))
			; /* redirect with partial body, or similar */
		இல்லையெனில் {
			finished = true;
		}
	} இல்லையெனில் எனில் (result == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} இல்லையெனில் எனில் (result == CURLE_SSL_PEER_CERTIFICATE ||
		   result == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} இல்லையெனில் {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", result);
		error = true;
	}

	fetch_curl_stop(f);

	எனில் (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	எனில் (abort_fetch) {
		; /* fetch was aborted: no callback */
	} இல்லையெனில் எனில் (finished) {
		fetch_செய்தி செய்தி;
		செய்தி.type = FETCH_FINISHED;
		fetch_send_callback(&செய்தி, f->fetch_handle);
	} இல்லையெனில் எனில் (cert) {
		/* user needs to validate certificate with issue */
		fetch_செய்தி செய்தி;
		செய்தி.type = FETCH_CERT_ERR;
		fetch_send_callback(&செய்தி, f->fetch_handle);
	} இல்லையெனில் எனில் (error) {
		fetch_செய்தி செய்தி;
		தேர்வு (result) {
		நிலை CURLE_SSL_CONNECT_ERROR:
			செய்தி.type = FETCH_SSL_ERR;
			நிறுத்து;

		நிலை CURLE_OPERATION_TIMEDOUT:
			செய்தி.type = FETCH_TIMEDOUT;
			செய்தி.data.error = curl_easy_strerror(result);
			நிறுத்து;

		இயல்பு:
			செய்தி.type = FETCH_ERROR;
			செய்தி.data.error = curl_easy_strerror(result);
		}

		fetch_send_callback(&செய்தி, f->fetch_handle);
	}

	fetch_free(f->fetch_handle);
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */
நிலையான வெறுமை fetch_curl_poll(lwc_string *scheme_ignored)
{
	முழு running, queue;
	CURLMcode codem;
	CURLMsg *curl_செய்தி;

	எனில் (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_set, write_fd_set, exc_fd_set;
		முழு max_fd = -1;
		முழு i;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&exc_fd_set);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_set, &write_fd_set,
				&exc_fd_set, &max_fd);
		ப்ரதிஜ்ஞா(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		க்கு (i = 0; i <= max_fd; i++) {
			bool பட = false;
			bool லிக = false;
			bool error = false;

			எனில் (FD_ISSET(i, &read_fd_set)) {
				பட = true;
			}
			எனில் (FD_ISSET(i, &write_fd_set)) {
				லிக = true;
			}
			எனில் (FD_ISSET(i, &exc_fd_set)) {
				error = true;
			}
			எனில் (பட || லிக || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      பட ? "read" : "    ",
				      லிக ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	செய் {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		எனில் (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			திரும்பு;
		}
	} போது (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_செய்தி = curl_multi_info_read(fetch_curl_multi, &queue);
	போது (curl_செய்தி) {
		தேர்வு (curl_செய்தி->செய்தி) {
			நிலை CURLMSG_DONE:
				fetch_curl_done(curl_செய்தி->easy_handle,
						curl_செய்தி->data.result);
				நிறுத்து;
			இயல்பு:
				நிறுத்து;
		}
		curl_செய்தி = curl_multi_info_read(fetch_curl_multi, &queue);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
நிலையான முழு
fetch_curl_progress(வெறுமை *clientp,
		    NSCURL_PROGRESS_T dltotal,
		    NSCURL_PROGRESS_T dlnow,
		    NSCURL_PROGRESS_T ultotal,
		    NSCURL_PROGRESS_T ulnow)
{
	நிலையான எழுத்து fetch_progress_buffer[256]; /**< Progress buffer for cURL */
	கட்டமைப்பு curl_fetch_info *f = (கட்டமைப்பு curl_fetch_info *) clientp;
	uint64_t time_now_ms;
	fetch_செய்தி செய்தி;

	எனில் (f->abort) {
		திரும்பு 0;
        }

	செய்தி.type = FETCH_PROGRESS;
	செய்தி.data.progress = fetch_progress_buffer;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&time_now_ms);
#define UPDATE_DELAY_MS (1000 / UPDATES_PER_SECOND)
	எனில் (time_now_ms - f->last_progress_update < UPDATE_DELAY_MS) {
		திரும்பு 0;
        }
#undef UPDATE_DELAY_MS
	f->last_progress_update = time_now_ms;

	எனில் (dltotal > 0) {
		snprintf(fetch_progress_buffer, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&செய்தி, f->fetch_handle);
	} இல்லையெனில் {
		snprintf(fetch_progress_buffer, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&செய்தி, f->fetch_handle);
	}

	திரும்பு 0;
}


/**
 * Format curl debug for nslog
 */
நிலையான முழு
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 எழுத்து *data,
		 size_t size,
		 வெறுமை *userptr)
{
	நிலையான மாறிலி எழுத்து s_infotype[CURLINFO_END][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	தேர்வு(type) {
	நிலை CURLINFO_TEXT:
	நிலை CURLINFO_HEADER_OUT:
	நிலை CURLINFO_HEADER_IN:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (முழு)size - 1, data);
		நிறுத்து;

	இயல்பு:
		நிறுத்து;
	}
	திரும்பு 0;
}


நிலையான curl_socket_t fetch_curl_socket_open(வெறுமை *clientp,
		curlsocktype purpose, கட்டமைப்பு curl_sockaddr *address)
{
	(வெறுமை) clientp;
	(வெறுமை) purpose;

	திரும்பு (curl_socket_t) guit->fetch->socket_open(
			address->family, address->socktype,
			address->protocol);
}

நிலையான முழு fetch_curl_socket_close(வெறுமை *clientp, curl_socket_t item)
{
	(வெறுமை) clientp;

	திரும்பு guit->fetch->socket_close((முழு) item);
}

/**
 * Callback function for cURL.
 */
நிலையான size_t fetch_curl_data(எழுத்து *data, size_t size, size_t nmemb, வெறுமை *_f)
{
	கட்டமைப்பு curl_fetch_info *f = _f;
	CURLcode code;
	fetch_செய்தி செய்தி;

	/* ensure we only have to get this information once */
	எனில் (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		ப்ரதிஜ்ஞா(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	எனில் (f->http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_code = 0;
		திரும்பு size * nmemb;
	}

	எனில் (f->abort || (!f->had_headers && fetch_curl_process_headers(f))) {
		f->stopped = true;
		திரும்பு 0;
	}

	/* send data to the caller */
	செய்தி.type = FETCH_DATA;
	செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *) data;
	செய்தி.data.header_or_data.len = size * nmemb;
	fetch_send_callback(&செய்தி, f->fetch_handle);

	எனில் (f->abort) {
		f->stopped = true;
		திரும்பு 0;
	}

	திரும்பு size * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
நிலையான size_t
fetch_curl_header(எழுத்து *data, size_t size, size_t nmemb, வெறுமை *_f)
{
	கட்டமைப்பு curl_fetch_info *f = _f;
	முழு i;
	fetch_செய்தி செய்தி;
	size *= nmemb;

	எனில் (f->abort) {
		f->stopped = true;
		திரும்பு 0;
	}

	எனில் (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	செய்தி.type = FETCH_HEADER;
	செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *) data;
	செய்தி.data.header_or_data.len = size;
	fetch_send_callback(&செய்தி, f->fetch_handle);

#define SKIP_ST(o) க்கு (i = (o); i < (முழு) size && (data[i] == ' ' || data[i] == '\t'); i++)

	எனில் (12 < size && strncasecmp(data, "Location:", 9) == 0) {
		/* extract Location header */
		விடுவி(f->location);
		f->location = malloc(size);
		எனில் (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			திரும்பு size;
		}
		SKIP_ST(9);
		strncpy(f->location, data + i, size - i);
		f->location[size - i] = '\0';
		க்கு (i = size - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} இல்லையெனில் எனில் (15 < size && strncasecmp(data, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		எனில் (i < (முழு)size && '0' <= data[i] && data[i] <= '9')
			f->content_length = atol(data + i);
	} இல்லையெனில் எனில் (17 < size && strncasecmp(data, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		போது (i < (முழு) size - 5 &&
				strncasecmp(data + i, "realm", 5))
			i++;
		போது (i < (முழு) size - 1 && data[++i] != '"')
			/* */;
		i++;

		எனில் (i < (முழு) size) {
			size_t end = i;

			போது (end < size && data[end] != '"')
				++end;

			எனில் (end < size) {
				விடுவி(f->realm);
				f->realm = malloc(end - i + 1);
				எனில் (f->realm != வெற்று) {
					strncpy(f->realm, data + i, end - i);
					f->realm[end - i] = '\0';
				}
			}
		}
	} இல்லையெனில் எனில் (11 < size && strncasecmp(data, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_set_cookie(f->fetch_handle, &data[i]);
	}

	திரும்பு size;
#undef SKIP_ST
}

நிலையான முழு fetch_curl_fdset(lwc_string *scheme, fd_set *read_set,
			    fd_set *write_set, fd_set *error_set)
{
	CURLMcode code;
	முழு maxfd = -1;

	code = curl_multi_fdset(fetch_curl_multi,
				read_set,
				write_set,
				error_set,
				&maxfd);
	ப்ரதிஜ்ஞா(code == CURLM_OK);

	திரும்பு maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_register(வெறுமை)
{
	CURLcode code;
	curl_version_info_data *data;
	முழு i;
	lwc_string *scheme;
	மாறிலி கட்டமைப்பு fetcher_operation_table fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.setup = fetch_curl_setup,
		.start = fetch_curl_start,
		.abort = fetch_curl_abort,
		.விடுவி = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#எனில் LIBCURL_VERSION_இலக்கம் >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, வெற்று, வெற்று);
	எனில் (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} இல்லையெனில் {
		curl_with_openssl = false;
	}
#endif

	NSLOG(netsurf, INFO, "curl_version %s", curl_version());

	code = curl_global_init(CURL_GLOBAL_ALL);
	எனில் (code != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		திரும்பு NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	எனில் (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		திரும்பு NSERROR_INIT_FAILED;
	}

#எனில் LIBCURL_VERSION_இலக்கம் >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		முழு maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#undef SETOPT
#define SETOPT(option, மதிப்பு) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, மதிப்பு);	\
	எனில் (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		செல் curl_multi_setopt_failed;				\
	}

		SETOPT(CURLMOPT_MAXCONNECTS, (long)maxconnects);
		SETOPT(CURLMOPT_MAX_TOTAL_CONNECTIONS, (long)maxconnects);
		SETOPT(CURLMOPT_MAX_HOST_CONNECTIONS, (long)nsoption_int(max_fetchers_per_host));
	}
#endif

	/* Create a curl easy handle with the options that are common to all
	 *  fetches.
	 */
	fetch_blank_curl = curl_easy_init();
	எனில் (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		திரும்பு NSERROR_INIT_FAILED;
	}

#undef SETOPT
#define SETOPT(option, மதிப்பு) \
	code = curl_easy_setopt(fetch_blank_curl, option, மதிப்பு);	\
	எனில் (code != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		செல் curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_buffer);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	எனில் (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} இல்லையெனில் {
		SETOPT(CURLOPT_VERBOSE, 1);
	}

	/* Currently we explode if curl uses HTTP2, so force 1.1. */
	SETOPT(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

	SETOPT(CURLOPT_WRITEFUNCTION, fetch_curl_data);
	SETOPT(CURLOPT_HEADERFUNCTION, fetch_curl_header);
	SETOPT(NSCURLOPT_PROGRESS_FUNCTION, fetch_curl_progress);
	SETOPT(CURLOPT_NOPROGRESS, 0L);
	SETOPT(CURLOPT_USERAGENT, user_agent_string());
	SETOPT(CURLOPT_ENCODING, "gzip");
	SETOPT(CURLOPT_கீழ்_SPEED_LIMIT, 1L);
	SETOPT(CURLOPT_கீழ்_SPEED_TIME, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (long)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	எனில் (nsoption_charp(ca_bundle) &&
	    strcmp(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	எனில் (nsoption_charp(ca_path) && strcmp(nsoption_charp(ca_path), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_path));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_path));
	}

#எனில் LIBCURL_VERSION_இலக்கம் < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	code = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, வெற்று);
	எனில் (code != CURLE_OK) {
		curl_with_openssl = false;
	} இல்லையெனில் {
		curl_with_openssl = true;
	}
#endif

	எனில் (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#எனில் LIBCURL_VERSION_இலக்கம் >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		code = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		எனில் (code != CURLE_OK && code != CURLE_NOT_BUILT_IN)
			செல் curl_easy_setopt_failed;
#endif
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	data = curl_version_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	எனில் (curl_fetch_ssl_hashmap == வெற்று) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		திரும்பு NSERROR_NOMEM;
	}

	க்கு (i = 0; data->protocols[i]; i++) {
		எனில் (strcmp(data->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} இல்லையெனில் எனில் (strcmp(data->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} இல்லையெனில் {
			/* Ignore non-http(s) protocols */
			தொடர்;
		}

		எனில் (fetcher_add(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      data->protocols[i]);
		}
	}

	திரும்பு NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	திரும்பு NSERROR_INIT_FAILED;

#எனில் LIBCURL_VERSION_இலக்கம் >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	திரும்பு NSERROR_INIT_FAILED;
#endif
}
