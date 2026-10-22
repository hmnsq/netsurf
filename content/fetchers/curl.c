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
#شامل "utils/inet.h"

#شامل <assert.h>
#شامل <errno.h>
#شامل <inttypes.h>
#شامل <stdbool.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <time.h>
#شامل <sys/stat.h>

#شامل <libwapcaplet/libwapcaplet.h>
#شامل <nsutils/time.h>

#شامل "utils/corestrings.h"
#شامل "utils/hashmap.h"
#شامل "utils/nsoption.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/utils.h"
#شامل "utils/ring.h"
#شامل "utils/useragent.h"
#شامل "utils/file.h"
#شامل "utils/string.h"
#شامل "netsurf/fetch.h"
#شامل "netsurf/misc.h"
#شامل "desktop/gui_internal.h"

#شامل "content/fetch.h"
#شامل "content/fetchers.h"
#شامل "content/fetchers/curl.h"
#شامل "content/urldb.h"

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

#شامل <openssl/ssl.h>
#شامل <openssl/x509v3.h>

#وگرنه /* WITH_OPENSSL */

تعریفنوع نویسه X509;

ایستا تهی X509_free(X509 *cert)
{
	آزاد(cert);
}

#endif /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
ایستا uint32_t
curl_fetch_ssl_کلید_hash(تهی *کلید)
{
	nsurl *url = کلید;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *port = nsurl_get_component(url, NSURL_PORT);
	uint32_t hash;

	اگر (port == پوچ)
		port = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_مقدار(hostname) ^ lwc_string_hash_مقدار(port);

	lwc_string_unref(hostname);
	lwc_string_unref(port);

	بازگشت hash;
}

/* We only compare the hostname and port */
ایستا bool
curl_fetch_ssl_کلید_eq(تهی *key1, تهی *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_PORT);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_PORT);
	bool iseq = false;

	اگر (port1 == پوچ)
		port1 = lwc_string_ref(corestring_lwc_443);
	اگر (port2 == پوچ)
		port2 = lwc_string_ref(corestring_lwc_443);

	اگر (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		برو out;

	iseq = false;
	اگر (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		برو out;

out:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	بازگشت iseq;
}

ایستا تهی *
curl_fetch_ssl_مقدار_alloc(تهی *کلید)
{
	ساختار cert_chain *out;

	اگر (cert_chain_alloc(0, &out) != NSERROR_OK) {
		بازگشت پوچ;
	}

	بازگشت out;
}

ایستا تهی
curl_fetch_ssl_مقدار_destroy(تهی *مقدار)
{
	ساختار cert_chain *chain = مقدار;
	اگر (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

ایستا hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.کلید_clone = (hashmap_کلید_clone_t)nsurl_ref,
	.کلید_destroy = (hashmap_کلید_destroy_t)nsurl_unref,
	.کلید_eq = curl_fetch_ssl_کلید_eq,
	.کلید_hash = curl_fetch_ssl_کلید_hash,
	.مقدار_alloc = curl_fetch_ssl_مقدار_alloc,
	.مقدار_destroy = curl_fetch_ssl_مقدار_destroy,
};

ایستا hashmap_t *curl_fetch_ssl_hashmap = پوچ;

/** SSL certificate info */
ساختار cert_info {
	X509 *cert;		/**< Pointer to certificate */
	long err;		/**< OpenSSL error code */
};

#اگر LIBCURL_VERSION_رقم >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_XFERINFOFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_XFERINFODATA
#define NSCURL_PROGRESS_T curl_off_t
#وگرنه
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_PROGRESSFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_PROGRESSDATA
#define NSCURL_PROGRESS_T دوگانه
#endif

#اگر LIBCURL_VERSION_رقم >= 0x073800 /* 7.56.0 depricated curl_formadd */
#define NSCURL_POSTDATA_T curl_mime
#define NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#define NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#وگرنه
#define NSCURL_POSTDATA_T ساختار curl_httppost
#define NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#define NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#endif

/** Information for a single fetch. */
ساختار curl_fetch_info {
	ساختار fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool sent_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_headers;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	ساختار curl_slist *headers;	/**< List of request headers. */
	نویسه *location;		/**< Response Location header, or 0. */
	unsigned long content_length;	/**< Response Content-Length, or 0. */
	نویسه *cookie_string;	/**< Cookie string for this fetch */
	نویسه *realm;		/**< HTTP Auth Realm */
	ساختار fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	long http_code; /**< HTTP result code from cURL. */

	uint64_t last_progress_update;	/**< Time of last progress update */
	صحیح cert_depth; /**< deepest certificate in use */
	ساختار cert_info cert_data[MAX_CERT_DEPTH]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
ساختار cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	ساختار cache_handle *r_prev; /**< Previous cached handle in ring. */
	ساختار cache_handle *r_next; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
ایستا CURL *fetch_blank_curl;

/** Ring of cached handles */
ایستا ساختار cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
ایستا صحیح curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
ایستا bool curl_with_openssl;

/** Error buffer for cURL. */
ایستا نویسه fetch_error_buffer[CURL_ERROR_SIZE];

/** Proxy authentication details. */
ایستا نویسه fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
ایستا bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
ایستا bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_data(scheme));
	curl_fetchers_registered++;
	بازگشت true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
ایستا تهی fetch_curl_finalise(lwc_string *scheme)
{
	ساختار cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_data(scheme));
	اگر (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		اگر (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = پوچ;
	}

	/* Free anything remaining in the cached curl handle ring */
	وقتی (curl_handle_ring != پوچ) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
		curl_easy_cleanup(h->handle);
		آزاد(h);
	}
}


/**
 * Check if this fetcher can fetch a url.
 *
 * \param url The url to check.
 * \return true if the fetcher supports the url else false.
 */
ایستا bool fetch_curl_can_fetch(ثابت nsurl *url)
{
	بازگشت nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
ایستا ساختار fetch_postdata *
fetch_curl_alloc_postdata(ثابت نویسه *post_urlenc,
			  ثابت ساختار fetch_multipart_data *post_multipart)
{
	ساختار fetch_postdata *postdata;
	postdata = calloc(1, اندازه(ساختار fetch_postdata));
	اگر (postdata != پوچ) {

		اگر (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->data.urlenc = strdup(post_urlenc);
			اگر (postdata->data.urlenc == پوچ) {
				آزاد(postdata);
				postdata = پوچ;
			}
		} وگرنه اگر (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->data.multipart = fetch_multipart_data_clone(post_multipart);
			اگر (postdata->data.multipart == پوچ) {
				آزاد(postdata);
				postdata = پوچ;
			}
		} وگرنه {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	بازگشت postdata;
}

/**
 * free postdata
 */
ایستا تهی fetch_curl_free_postdata(ساختار fetch_postdata *postdata)
{
	اگر (postdata != پوچ) {
		انتخاب (postdata->type) {
		حالت FETCH_POSTDATA_NONE:
			بشکن;
		حالت FETCH_POSTDATA_URLENC:
			آزاد(postdata->data.urlenc);
			بشکن;
		حالت FETCH_POSTDATA_MULTIPART:
			fetch_multipart_data_destroy(postdata->data.multipart);
			بشکن;
		}

		آزاد(postdata);
	}
}

/**
 *construct a new fetch structure
 */
ایستا ساختار curl_fetch_info *fetch_alloc(تهی)
{
	ساختار curl_fetch_info *fetch;
	fetch = malloc(اندازه (*fetch));
	اگر (fetch == پوچ)
		بازگشت پوچ;

	fetch->curl_handle = پوچ;
	fetch->sent_ssl_chain = false;
	fetch->had_headers = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->headers = پوچ;
	fetch->url = پوچ;
	fetch->host = پوچ;
	fetch->location = پوچ;
	fetch->content_length = 0;
	fetch->http_code = 0;
	fetch->cookie_string = پوچ;
	fetch->realm = پوچ;
	fetch->last_progress_update = 0;
	fetch->postdata = پوچ;
	fetch->curl_postdata = پوچ;

	/* Clear certificate chain data */
	memset(fetch->cert_data, 0, اندازه(fetch->cert_data));
	fetch->cert_depth = -1;

	بازگشت fetch;
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
ایستا تهی *
fetch_curl_setup(ساختار fetch *parent_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ثابت نویسه *post_urlenc,
		 ثابت ساختار fetch_multipart_data *post_multipart,
		 ثابت نویسه **headers)
{
	ساختار curl_fetch_info *fetch;
	ساختار curl_slist *slist;
	صحیح i;

	fetch = fetch_alloc();
	اگر (fetch == پوچ)
		بازگشت پوچ;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = parent_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	اگر (fetch->host == پوچ) {
		برو failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	اگر (fetch->postdata == پوچ) {
		برو failed;
	}

#define APPEND(list, مقدار) \
	slist = curl_slist_append(list, مقدار);		\
	اگر (slist == پوچ)				\
		برو failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->headers, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->headers, "Expect:");

	اگر ((nsoption_charp(accept_language) != پوچ) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		نویسه s[80];
		snprintf(s, اندازه s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[اندازه s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	اگر (nsoption_charp(accept_charset) != پوچ &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		نویسه s[80];
		snprintf(s, اندازه s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[اندازه s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	اگر (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->headers, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	برای (i = 0; headers[i] != پوچ; i++) {
		APPEND(fetch->headers, headers[i]);
	}

	بازگشت fetch;

#undef APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->headers);
	آزاد(fetch);
	بازگشت پوچ;
}


#ifdef WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
ایستا ساختار cert_chain *
fetch_curl_get_cached_chain(ساختار curl_fetch_info *f)
{
	ساختار cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	اگر (chain == پوچ) {
		chain = hashmap_درج(curl_fetch_ssl_hashmap, f->url);
	}

	بازگشت chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
ایستا تهی
fetch_curl_store_certs_in_cache(ساختار curl_fetch_info *f)
{
	size_t depth;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_DEPTH];
	ساختار cert_chain chain, *cached_chain;
	ساختار cert_info *certs;

	memset(&chain, 0, اندازه(chain));

	certs = f->cert_data;
	chain.depth = f->cert_depth + 1; /* 0 indexed certificate depth */

	برای (depth = 0; depth < chain.depth; depth++) {
		اگر (certs[depth].cert == پوچ) {
			/* This certificate is missing, skip it */
			chain.certs[depth].err = SSL_CERT_ERR_CERT_MISSING;
			ادامه;
		}

		/* error code (if any) */
		انتخاب (certs[depth].err) {
		حالت X509_V_OK:
			chain.certs[depth].err = SSL_CERT_ERR_OK;
			بشکن;

		حالت X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			/* fallthrough */
		حالت X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_کلید:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_ISSUER;
			بشکن;

		حالت X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		حالت X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		حالت X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		حالت X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_SIG;
			بشکن;

		حالت X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		حالت X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_YOUNG;
			بشکن;

		حالت X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		حالت X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_OLD;
			بشکن;

		حالت X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			chain.certs[depth].err = SSL_CERT_ERR_SELF_SIGNED;
			بشکن;

		حالت X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			chain.certs[depth].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			بشکن;

		حالت X509_V_ERR_CERT_REVOKED:
			chain.certs[depth].err = SSL_CERT_ERR_REVOKED;
			بشکن;

		حالت X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[depth].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			بشکن;

		پیشفرض:
			chain.certs[depth].err = SSL_CERT_ERR_UNKNOWN;
			بشکن;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[depth].cert);
		BIO_get_mem_ptr(mem, &buf[depth]);
		(تهی) BIO_set_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[depth].der = (uint8_t *)buf[depth]->data;
		chain.certs[depth].der_length = buf[depth]->length;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	اگر (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	برای (depth = 0; depth < chain.depth; depth++) {
		اگر (chain.certs[depth].err == SSL_CERT_ERR_CERT_MISSING) {
			ادامه;
		}
		اگر (buf[depth] != پوچ) {
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
ایستا صحیح
fetch_curl_verify_callback(صحیح verify_ok, X509_STORE_CTX *x509_ctx)
{
	صحیح depth;
	ساختار curl_fetch_info *fetch;

	depth = X509_STORE_CTX_get_error_depth(x509_ctx);
	fetch = X509_STORE_CTX_get_app_data(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	اگر (depth >= MAX_CERT_DEPTH) {
		X509_STORE_CTX_set_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		بازگشت 0;
	}

	/* record the max depth */
	اگر (depth > fetch->cert_depth) {
		fetch->cert_depth = depth;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	اگر (!fetch->cert_data[depth].cert) {
		fetch->cert_data[depth].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_up_ref(fetch->cert_data[depth].cert);
		fetch->cert_data[depth].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	اگر (depth > 0) {
		verify_ok = 1;
	} وگرنه {
		/* search for deeper certificates in the chain with errors */
		برای (depth = fetch->cert_depth; depth > 0; depth--) {
			اگر (fetch->cert_data[depth].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_set_error(x509_ctx, fetch->cert_data[depth].err);
			}
		}
	}

	بازگشت verify_ok;
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
ایستا صحیح fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, تهی *parm)
{
	ساختار curl_fetch_info *f = (ساختار curl_fetch_info *) parm;
	صحیح ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_set_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_data(f->host),
					 lwc_string_length(f->host));

	/* Store fetch struct in context for verify callback */
	اگر (ok) {
		ok = X509_STORE_CTX_set_app_data(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	اگر (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_in_cache(f);

	بازگشت ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
ایستا CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, تهی *_sslctx, تهی *parm)
{
	ساختار curl_fetch_info *f = (ساختار curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	long options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_set_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	اگر (f->downgrade_tls) {
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

	بازگشت CURLE_OK;
}


#endif /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
ایستا تهی
fetch_curl_report_certs_upstream(ساختار curl_fetch_info *f)
{
	fetch_پیام پیام;
	ساختار cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	اگر (chain != پوچ) {
		پیام.type = FETCH_CERTS;
		پیام.data.chain = chain;

		fetch_send_callback(&پیام, f->fetch_handle);
	}

	f->sent_ssl_chain = true;
}

#اگر LIBCURL_VERSION_رقم >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
ساختار curl_mime_ctx {
	نویسه *buffer;
	curl_off_t size;
	curl_off_t position;
};

ایستا size_t mime_data_read_callback(نویسه *buffer, size_t size, size_t nitems, تهی *arg)
{
	ساختار curl_mime_ctx *mctx = (ساختار curl_mime_ctx *) arg;
	curl_off_t sz = mctx->size - mctx->position;

	nitems *= size;
	اگر(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	اگر(sz) {
		memcpy(buffer, mctx->buffer + mctx->position, sz);
	}
	mctx->position += sz;
	بازگشت sz;
}

ایستا صحیح mime_data_seek_callback(تهی *arg, curl_off_t offset, صحیح origin)
{
	ساختار curl_mime_ctx *mctx = (ساختار curl_mime_ctx *) arg;

	انتخاب(origin) {
	حالت SEEK_END:
		offset += mctx->size;
		بشکن;
	حالت SEEK_CUR:
		offset += mctx->position;
		بشکن;
	}

	اگر(offset < 0) {
		بازگشت CURL_SEEKFUNC_FAIL;
	}
	mctx->position = offset;
	بازگشت CURL_SEEKFUNC_OK;
}

ایستا تهی mime_data_free_callback(تهی *arg)
{
	ساختار curl_mime_ctx *mctx = (ساختار curl_mime_ctx *) arg;
	آزاد(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
ایستا curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    ثابت ساختار fetch_multipart_data *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode code = CURLE_OK;
	size_t مقدار_len;

	cmime = curl_mime_init(chandle);
	اگر (cmime == پوچ) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		بازگشت پوچ;
	}

	/* iterate post data */
	برای (; multipart != پوچ; multipart = multipart->next) {
		part = curl_mime_addpart(cmime);
		اگر (part == پوچ) {
			برو convert_failed;
		}

		code = curl_mime_name(part, multipart->name);
		اگر (code != CURLE_OK) {
			برو convert_failed;
		}

		مقدار_len = strlen(multipart->مقدار);

		اگر (multipart->file && مقدار_len==0) {
			/* file entries with no filename require special handling */
			code=curl_mime_data(part, multipart->مقدار, مقدار_len);
			اگر (code != CURLE_OK) {
				برو convert_failed;
			}

			code = curl_mime_filename(part, "");
			اگر (code != CURLE_OK) {
				برو convert_failed;
			}

			code = curl_mime_type(part, "application/octet-stream");
			اگر (code != CURLE_OK) {
				برو convert_failed;
			}

		} وگرنه اگر(multipart->file) {
			/* file entry */
			nserror ret;
			نویسه *leafname = پوچ;
			نویسه *mimetype = پوچ;

			code = curl_mime_filedata(part, multipart->rawfile);
			اگر (code != CURLE_OK) {
				برو convert_failed;
			}

			ret = guit->file->basename(multipart->مقدار, &leafname, پوچ);
			اگر (ret != NSERROR_OK) {
				برو convert_failed;
			}
			code = curl_mime_filename(part, leafname);
			آزاد(leafname);
			اگر (code != CURLE_OK) {
				برو convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->مقدار);
			اگر (mimetype == پوچ) {
				mimetype=strdup("text/plain");
			}
			اگر (mimetype == پوچ) {
				برو convert_failed;
			}
			code = curl_mime_type(part, mimetype);
			آزاد(mimetype);
			اگر (code != CURLE_OK) {
				برو convert_failed;
			}

		} وگرنه {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			ساختار curl_mime_ctx *cb_ctx;
			cb_ctx = malloc(اندازه(ساختار curl_mime_ctx));
			اگر (cb_ctx == پوچ) {
				برو convert_failed;
			}
			cb_ctx->buffer = multipart->مقدار;
			cb_ctx->size = مقدار_len;
			cb_ctx->position = 0;
			code = curl_mime_data_cb(part,
						 مقدار_len,
						 mime_data_read_callback,
						 mime_data_seek_callback,
						 mime_data_free_callback,
						 cb_ctx);
			اگر (code != CURLE_OK) {
				آزاد(cb_ctx);
				برو convert_failed;
			}
		}
	}

	بازگشت cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", code);
	curl_mime_free(cmime);
	بازگشت پوچ;
}

#وگرنه /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
ایستا ساختار curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    ثابت ساختار fetch_multipart_data *control)
{
	ساختار curl_httppost *post = پوچ, *last = پوچ;
	CURLFORMcode code;
	nserror ret;

	برای (; control; control = control->next) {
		اگر (control->file) {
			نویسه *leafname = پوچ;
			ret = guit->file->basename(control->مقدار, &leafname, پوچ);
			اگر (ret != NSERROR_OK) {
				ادامه;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			اگر (control->مقدار[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				ایستا نویسه buf;

				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_BUFFER, control->مقدار,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_END);
				اگر (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", code,
					      control->name);
			} وگرنه {
				نویسه *mimetype = guit->fetch->mimetype(control->مقدار);
				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_END);
				اگر (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      code,
					      control->name,
					      control->مقدار);
				آزاد(mimetype);
			}
			آزاد(leafname);
		} وگرنه {
			code = curl_formadd(&post, &last,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->مقدار,
					    CURLFORM_END);
			اگر (code != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", code,
				      control->name, control->مقدار);
		}
	}

	بازگشت post;
}

#endif  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
ایستا CURLcode fetch_curl_set_postdata(ساختار curl_fetch_info *f)
{
	CURLcode code = CURLE_OK;

#undef SETOPT
#define SETOPT(option, مقدار) { \
	code = curl_easy_setopt(f->curl_handle, option, مقدار);	\
	اگر (code != CURLE_OK)					\
		بازگشت code;					\
	}

	انتخاب (f->postdata->type) {
	حالت FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, پوچ);
		SETOPT(NSCURL_POSTDATA_CURLOPT, پوچ);
		SETOPT(CURLOPT_HTTPGET, 1L);
		بشکن;

	حالت FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, پوچ);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->data.urlenc);
		بشکن;

	حالت FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, پوچ);
		SETOPT(CURLOPT_HTTPGET, 0L);
		اگر (f->curl_postdata == پوچ) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->data.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		بشکن;
	}
	بازگشت code;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
ایستا CURLcode fetch_curl_set_options(ساختار curl_fetch_info *f)
{
	CURLcode code;
	ثابت نویسه *auth;

#undef SETOPT
#define SETOPT(option, مقدار) { \
	code = curl_easy_setopt(f->curl_handle, option, مقدار);	\
	اگر (code != CURLE_OK)					\
		بازگشت code;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_PRIVATE, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_PROGRESS_DATA, f);
	SETOPT(CURLOPT_HTTPHEADER, f->headers);
	code = fetch_curl_set_postdata(f);
	اگر (code != CURLE_OK) {
		بازگشت code;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	اگر (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} وگرنه {
		SETOPT(CURLOPT_COOKIE, پوچ);
	}

	اگر ((auth = urldb_get_auth_details(f->url, پوچ)) != پوچ) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} وگرنه {
		SETOPT(CURLOPT_USERPWD, پوچ);
	}

	/* set up proxy options */
	اگر (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != پوچ) &&
	    (strncmp(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (long) nsoption_int(http_proxy_port));

#اگر LIBCURL_VERSION_رقم >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#endif

		اگر (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(long) CURLAUTH_BASIC :
					(long) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					اندازه fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} وگرنه {
		SETOPT(CURLOPT_PROXY, پوچ);
	}


	اگر (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	اگر (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		اگر (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, پوچ);
			SETOPT(CURLOPT_SSL_CTX_DATA, پوچ);
		}
	} وگرنه {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef WITH_OPENSSL
		اگر (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_DATA, f);
		}
#endif
	}

	بازگشت CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
ایستا bool
fetch_curl_initiate_fetch(ساختار curl_fetch_info *fetch, CURL *handle)
{
	CURLcode code;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	code = fetch_curl_set_options(fetch);
	اگر (code != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		بازگشت false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_add_handle(fetch_curl_multi, fetch->curl_handle);
	أكد(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	بازگشت true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
ایستا CURL *fetch_curl_get_handle(lwc_string *host)
{
	ساختار cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	اگر (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		آزاد(h);
	} وگرنه {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	بازگشت ret;
}


/**
 * Dispatch a single job
 */
ایستا bool fetch_curl_start(تهی *vfetch)
{
	ساختار curl_fetch_info *fetch = (ساختار curl_fetch_info*)vfetch;
	اگر (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		بازگشت false;
	}
	بازگشت fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
ایستا تهی fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#اگر LIBCURL_VERSION_رقم >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	بازگشت;
#وگرنه
	ساختار cache_handle *h = 0;
	صحیح c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	اگر (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		بازگشت;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(ساختار cache_handle, curl_handle_ring, c);
	اگر (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		اگر (curl_handle_ring != پوچ) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_next;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} وگرنه {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		بازگشت;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (ساختار cache_handle*)malloc(اندازه(ساختار cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_درج(curl_handle_ring, h);
#endif
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
ایستا تهی fetch_curl_stop(ساختار curl_fetch_info *f)
{
	CURLMcode codem;

	أكد(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	اگر (f->curl_handle) {
		/* remove from curl multi handle */
		codem = curl_multi_remove_handle(fetch_curl_multi,
				f->curl_handle);
		أكد(codem == CURLM_OK);
		/* Put this curl handle into the cache if wanted. */
		fetch_curl_cache_handle(f->curl_handle, f->host);
		f->curl_handle = 0;
	}

	fetch_remove_from_queues(f->fetch_handle);
}


/**
 * Abort a fetch.
 */
ایستا تهی fetch_curl_abort(تهی *vf)
{
	ساختار curl_fetch_info *f = (ساختار curl_fetch_info *)vf;
	أكد(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	اگر (f->curl_handle) {
		اگر (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->abort = true;
		} وگرنه {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} وگرنه {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
ایستا تهی fetch_curl_free(تهی *vf)
{
	ساختار curl_fetch_info *f = (ساختار curl_fetch_info *)vf;
	صحیح i;

	اگر (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	آزاد(f->location);
	آزاد(f->cookie_string);
	آزاد(f->realm);
	اگر (f->headers) {
		curl_slist_free_all(f->headers);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	برای (i = 0; i < MAX_CERT_DEPTH; i++) {
		اگر (f->cert_data[i].cert != پوچ) {
			X509_free(f->cert_data[i].cert);
		}
	}

	آزاد(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
ایستا bool fetch_curl_process_headers(ساختار curl_fetch_info *f)
{
	long http_code;
	CURLcode code;
	fetch_پیام پیام;

	f->had_headers = true;

	اگر (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		أكد(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}
	http_code = f->http_code;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_code);

	اگر ((http_code == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		پیام.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&پیام, f->fetch_handle);
		بازگشت true;
	}

	/* handle HTTP redirects (3xx response codes) */
	اگر (HTTP_RESPONSE_IS_3XX(http_code) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		پیام.type = FETCH_REDIRECT;
		پیام.data.redirect = f->location;
		fetch_send_callback(&پیام, f->fetch_handle);
		بازگشت true;
	}

	/* handle HTTP 401 (Authentication errors) */
	اگر (http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		پیام.type = FETCH_AUTH;
		پیام.data.auth.realm = f->realm;
		fetch_send_callback(&پیام, f->fetch_handle);
		بازگشت true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	اگر (f->only_2xx &&
	    strncmp(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_code) == false)) {
		پیام.type = FETCH_ERROR;
		پیام.data.error = messages_get("Not2xx");
		fetch_send_callback(&پیام, f->fetch_handle);
		بازگشت true;
	}

	اگر (f->abort)
		بازگشت true;

	بازگشت false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
ایستا تهی fetch_curl_done(CURL *curl_handle, CURLcode result)
{
	bool finished = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	ساختار curl_fetch_info *f;
	نویسه **_hideous_hack = (نویسه **) (تهی *) &f;
	CURLcode code;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	code = curl_easy_getinfo(curl_handle, CURLINFO_PRIVATE, _hideous_hack);
	أكد(code == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	اگر ((abort_fetch == false) &&
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
		اگر (f->stopped ||
		    (!f->had_headers &&	fetch_curl_process_headers(f))) {
			; /* redirect with no body or similar */
		} وگرنه {
			finished = true;
		}
	} وگرنه اگر (result == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		اگر (!f->had_headers && fetch_curl_process_headers(f))
			; /* redirect with partial body, or similar */
		وگرنه {
			finished = true;
		}
	} وگرنه اگر (result == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} وگرنه اگر (result == CURLE_SSL_PEER_CERTIFICATE ||
		   result == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} وگرنه {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", result);
		error = true;
	}

	fetch_curl_stop(f);

	اگر (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	اگر (abort_fetch) {
		; /* fetch was aborted: no callback */
	} وگرنه اگر (finished) {
		fetch_پیام پیام;
		پیام.type = FETCH_FINISHED;
		fetch_send_callback(&پیام, f->fetch_handle);
	} وگرنه اگر (cert) {
		/* user needs to validate certificate with issue */
		fetch_پیام پیام;
		پیام.type = FETCH_CERT_ERR;
		fetch_send_callback(&پیام, f->fetch_handle);
	} وگرنه اگر (error) {
		fetch_پیام پیام;
		انتخاب (result) {
		حالت CURLE_SSL_CONNECT_ERROR:
			پیام.type = FETCH_SSL_ERR;
			بشکن;

		حالت CURLE_OPERATION_TIMEDOUT:
			پیام.type = FETCH_TIMEDOUT;
			پیام.data.error = curl_easy_strerror(result);
			بشکن;

		پیشفرض:
			پیام.type = FETCH_ERROR;
			پیام.data.error = curl_easy_strerror(result);
		}

		fetch_send_callback(&پیام, f->fetch_handle);
	}

	fetch_free(f->fetch_handle);
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */
ایستا تهی fetch_curl_poll(lwc_string *scheme_ignored)
{
	صحیح running, queue;
	CURLMcode codem;
	CURLMsg *curl_پیام;

	اگر (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_set, write_fd_set, exc_fd_set;
		صحیح max_fd = -1;
		صحیح i;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&exc_fd_set);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_set, &write_fd_set,
				&exc_fd_set, &max_fd);
		أكد(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		برای (i = 0; i <= max_fd; i++) {
			bool اقرأ = false;
			bool اكتب = false;
			bool error = false;

			اگر (FD_ISSET(i, &read_fd_set)) {
				اقرأ = true;
			}
			اگر (FD_ISSET(i, &write_fd_set)) {
				اكتب = true;
			}
			اگر (FD_ISSET(i, &exc_fd_set)) {
				error = true;
			}
			اگر (اقرأ || اكتب || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      اقرأ ? "read" : "    ",
				      اكتب ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	انجامبده {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		اگر (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			بازگشت;
		}
	} وقتی (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_پیام = curl_multi_info_read(fetch_curl_multi, &queue);
	وقتی (curl_پیام) {
		انتخاب (curl_پیام->پیام) {
			حالت CURLMSG_DONE:
				fetch_curl_done(curl_پیام->easy_handle,
						curl_پیام->data.result);
				بشکن;
			پیشفرض:
				بشکن;
		}
		curl_پیام = curl_multi_info_read(fetch_curl_multi, &queue);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
ایستا صحیح
fetch_curl_progress(تهی *clientp,
		    NSCURL_PROGRESS_T dltotal,
		    NSCURL_PROGRESS_T dlnow,
		    NSCURL_PROGRESS_T ultotal,
		    NSCURL_PROGRESS_T ulnow)
{
	ایستا نویسه fetch_progress_buffer[256]; /**< Progress buffer for cURL */
	ساختار curl_fetch_info *f = (ساختار curl_fetch_info *) clientp;
	uint64_t time_now_ms;
	fetch_پیام پیام;

	اگر (f->abort) {
		بازگشت 0;
        }

	پیام.type = FETCH_PROGRESS;
	پیام.data.progress = fetch_progress_buffer;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&time_now_ms);
#define UPDATE_DELAY_MS (1000 / UPDATES_PER_SECOND)
	اگر (time_now_ms - f->last_progress_update < UPDATE_DELAY_MS) {
		بازگشت 0;
        }
#undef UPDATE_DELAY_MS
	f->last_progress_update = time_now_ms;

	اگر (dltotal > 0) {
		snprintf(fetch_progress_buffer, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&پیام, f->fetch_handle);
	} وگرنه {
		snprintf(fetch_progress_buffer, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&پیام, f->fetch_handle);
	}

	بازگشت 0;
}


/**
 * Format curl debug for nslog
 */
ایستا صحیح
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 نویسه *data,
		 size_t size,
		 تهی *userptr)
{
	ایستا ثابت نویسه s_infotype[CURLINFO_END][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	انتخاب(type) {
	حالت CURLINFO_TEXT:
	حالت CURLINFO_HEADER_OUT:
	حالت CURLINFO_HEADER_IN:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (صحیح)size - 1, data);
		بشکن;

	پیشفرض:
		بشکن;
	}
	بازگشت 0;
}


ایستا curl_socket_t fetch_curl_socket_open(تهی *clientp,
		curlsocktype purpose, ساختار curl_sockaddr *address)
{
	(تهی) clientp;
	(تهی) purpose;

	بازگشت (curl_socket_t) guit->fetch->socket_open(
			address->family, address->socktype,
			address->protocol);
}

ایستا صحیح fetch_curl_socket_close(تهی *clientp, curl_socket_t item)
{
	(تهی) clientp;

	بازگشت guit->fetch->socket_close((صحیح) item);
}

/**
 * Callback function for cURL.
 */
ایستا size_t fetch_curl_data(نویسه *data, size_t size, size_t nmemb, تهی *_f)
{
	ساختار curl_fetch_info *f = _f;
	CURLcode code;
	fetch_پیام پیام;

	/* ensure we only have to get this information once */
	اگر (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		أكد(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	اگر (f->http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_code = 0;
		بازگشت size * nmemb;
	}

	اگر (f->abort || (!f->had_headers && fetch_curl_process_headers(f))) {
		f->stopped = true;
		بازگشت 0;
	}

	/* send data to the caller */
	پیام.type = FETCH_DATA;
	پیام.data.header_or_data.buf = (ثابت uint8_t *) data;
	پیام.data.header_or_data.len = size * nmemb;
	fetch_send_callback(&پیام, f->fetch_handle);

	اگر (f->abort) {
		f->stopped = true;
		بازگشت 0;
	}

	بازگشت size * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
ایستا size_t
fetch_curl_header(نویسه *data, size_t size, size_t nmemb, تهی *_f)
{
	ساختار curl_fetch_info *f = _f;
	صحیح i;
	fetch_پیام پیام;
	size *= nmemb;

	اگر (f->abort) {
		f->stopped = true;
		بازگشت 0;
	}

	اگر (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	پیام.type = FETCH_HEADER;
	پیام.data.header_or_data.buf = (ثابت uint8_t *) data;
	پیام.data.header_or_data.len = size;
	fetch_send_callback(&پیام, f->fetch_handle);

#define SKIP_ST(o) برای (i = (o); i < (صحیح) size && (data[i] == ' ' || data[i] == '\t'); i++)

	اگر (12 < size && strncasecmp(data, "Location:", 9) == 0) {
		/* extract Location header */
		آزاد(f->location);
		f->location = malloc(size);
		اگر (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			بازگشت size;
		}
		SKIP_ST(9);
		strncpy(f->location, data + i, size - i);
		f->location[size - i] = '\0';
		برای (i = size - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} وگرنه اگر (15 < size && strncasecmp(data, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		اگر (i < (صحیح)size && '0' <= data[i] && data[i] <= '9')
			f->content_length = atol(data + i);
	} وگرنه اگر (17 < size && strncasecmp(data, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		وقتی (i < (صحیح) size - 5 &&
				strncasecmp(data + i, "realm", 5))
			i++;
		وقتی (i < (صحیح) size - 1 && data[++i] != '"')
			/* */;
		i++;

		اگر (i < (صحیح) size) {
			size_t end = i;

			وقتی (end < size && data[end] != '"')
				++end;

			اگر (end < size) {
				آزاد(f->realm);
				f->realm = malloc(end - i + 1);
				اگر (f->realm != پوچ) {
					strncpy(f->realm, data + i, end - i);
					f->realm[end - i] = '\0';
				}
			}
		}
	} وگرنه اگر (11 < size && strncasecmp(data, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_set_cookie(f->fetch_handle, &data[i]);
	}

	بازگشت size;
#undef SKIP_ST
}

ایستا صحیح fetch_curl_fdset(lwc_string *scheme, fd_set *read_set,
			    fd_set *write_set, fd_set *error_set)
{
	CURLMcode code;
	صحیح maxfd = -1;

	code = curl_multi_fdset(fetch_curl_multi,
				read_set,
				write_set,
				error_set,
				&maxfd);
	أكد(code == CURLM_OK);

	بازگشت maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_register(تهی)
{
	CURLcode code;
	curl_version_info_data *data;
	صحیح i;
	lwc_string *scheme;
	ثابت ساختار fetcher_operation_table fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.setup = fetch_curl_setup,
		.start = fetch_curl_start,
		.abort = fetch_curl_abort,
		.آزاد = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#اگر LIBCURL_VERSION_رقم >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, پوچ, پوچ);
	اگر (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} وگرنه {
		curl_with_openssl = false;
	}
#endif

	NSLOG(netsurf, INFO, "curl_version %s", curl_version());

	code = curl_global_init(CURL_GLOBAL_ALL);
	اگر (code != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		بازگشت NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	اگر (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		بازگشت NSERROR_INIT_FAILED;
	}

#اگر LIBCURL_VERSION_رقم >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		صحیح maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#undef SETOPT
#define SETOPT(option, مقدار) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, مقدار);	\
	اگر (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		برو curl_multi_setopt_failed;				\
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
	اگر (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		بازگشت NSERROR_INIT_FAILED;
	}

#undef SETOPT
#define SETOPT(option, مقدار) \
	code = curl_easy_setopt(fetch_blank_curl, option, مقدار);	\
	اگر (code != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		برو curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_buffer);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	اگر (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} وگرنه {
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
	SETOPT(CURLOPT_کم_SPEED_LIMIT, 1L);
	SETOPT(CURLOPT_کم_SPEED_TIME, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (long)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	اگر (nsoption_charp(ca_bundle) &&
	    strcmp(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	اگر (nsoption_charp(ca_path) && strcmp(nsoption_charp(ca_path), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_path));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_path));
	}

#اگر LIBCURL_VERSION_رقم < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	code = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, پوچ);
	اگر (code != CURLE_OK) {
		curl_with_openssl = false;
	} وگرنه {
		curl_with_openssl = true;
	}
#endif

	اگر (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#اگر LIBCURL_VERSION_رقم >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		code = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		اگر (code != CURLE_OK && code != CURLE_NOT_BUILT_IN)
			برو curl_easy_setopt_failed;
#endif
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	data = curl_version_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	اگر (curl_fetch_ssl_hashmap == پوچ) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		بازگشت NSERROR_NOMEM;
	}

	برای (i = 0; data->protocols[i]; i++) {
		اگر (strcmp(data->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} وگرنه اگر (strcmp(data->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} وگرنه {
			/* Ignore non-http(s) protocols */
			ادامه;
		}

		اگر (fetcher_add(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      data->protocols[i]);
		}
	}

	بازگشت NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	بازگشت NSERROR_INIT_FAILED;

#اگر LIBCURL_VERSION_رقم >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	بازگشت NSERROR_INIT_FAILED;
#endif
}
