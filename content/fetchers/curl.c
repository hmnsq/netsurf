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
#അന്തര്ഭാവ "utils/inet.h"

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <inttypes.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <sys/stat.h>

#അന്തര്ഭാവ <libwapcaplet/libwapcaplet.h>
#അന്തര്ഭാവ <nsutils/time.h>

#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/hashmap.h"
#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/ring.h"
#അന്തര്ഭാവ "utils/useragent.h"
#അന്തര്ഭാവ "utils/file.h"
#അന്തര്ഭാവ "utils/string.h"
#അന്തര്ഭാവ "netsurf/fetch.h"
#അന്തര്ഭാവ "netsurf/misc.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"

#അന്തര്ഭാവ "content/fetch.h"
#അന്തര്ഭാവ "content/fetchers.h"
#അന്തര്ഭാവ "content/fetchers/curl.h"
#അന്തര്ഭാവ "content/urldb.h"

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

#അന്തര്ഭാവ <openssl/ssl.h>
#അന്തര്ഭാവ <openssl/x509v3.h>

#അല്ലെങ്കിൽ /* WITH_OPENSSL */

തരനിർവചനം അക്ഷരം X509;

സ്ഥിതം ശൂന്യം X509_free(X509 *cert)
{
	സ്വതന്ത്രം(cert);
}

#endif /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
സ്ഥിതം uint32_t
curl_fetch_ssl_താക്കോൽ_hash(ശൂന്യം *താക്കോൽ)
{
	nsurl *url = താക്കോൽ;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *port = nsurl_get_component(url, NSURL_PORT);
	uint32_t hash;

	എങ്കിൽ (port == ശൂന്യ)
		port = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_മൂല്യം(hostname) ^ lwc_string_hash_മൂല്യം(port);

	lwc_string_unref(hostname);
	lwc_string_unref(port);

	തിരികെ hash;
}

/* We only compare the hostname and port */
സ്ഥിതം bool
curl_fetch_ssl_താക്കോൽ_eq(ശൂന്യം *key1, ശൂന്യം *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_PORT);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_PORT);
	bool iseq = false;

	എങ്കിൽ (port1 == ശൂന്യ)
		port1 = lwc_string_ref(corestring_lwc_443);
	എങ്കിൽ (port2 == ശൂന്യ)
		port2 = lwc_string_ref(corestring_lwc_443);

	എങ്കിൽ (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		പോകുക out;

	iseq = false;
	എങ്കിൽ (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		പോകുക out;

out:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	തിരികെ iseq;
}

സ്ഥിതം ശൂന്യം *
curl_fetch_ssl_മൂല്യം_alloc(ശൂന്യം *താക്കോൽ)
{
	ഘടന cert_chain *out;

	എങ്കിൽ (cert_chain_alloc(0, &out) != NSERROR_OK) {
		തിരികെ ശൂന്യ;
	}

	തിരികെ out;
}

സ്ഥിതം ശൂന്യം
curl_fetch_ssl_മൂല്യം_destroy(ശൂന്യം *മൂല്യം)
{
	ഘടന cert_chain *chain = മൂല്യം;
	എങ്കിൽ (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

സ്ഥിതം hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.താക്കോൽ_clone = (hashmap_താക്കോൽ_clone_t)nsurl_ref,
	.താക്കോൽ_destroy = (hashmap_താക്കോൽ_destroy_t)nsurl_unref,
	.താക്കോൽ_eq = curl_fetch_ssl_താക്കോൽ_eq,
	.താക്കോൽ_hash = curl_fetch_ssl_താക്കോൽ_hash,
	.മൂല്യം_alloc = curl_fetch_ssl_മൂല്യം_alloc,
	.മൂല്യം_destroy = curl_fetch_ssl_മൂല്യം_destroy,
};

സ്ഥിതം hashmap_t *curl_fetch_ssl_hashmap = ശൂന്യ;

/** SSL certificate info */
ഘടന cert_info {
	X509 *cert;		/**< Pointer to certificate */
	long err;		/**< OpenSSL error code */
};

#എങ്കിൽ LIBCURL_VERSION_അക്കം >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_XFERINFOFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_XFERINFODATA
#define NSCURL_PROGRESS_T curl_off_t
#അല്ലെങ്കിൽ
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_PROGRESSFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_PROGRESSDATA
#define NSCURL_PROGRESS_T ഇരട്ട
#endif

#എങ്കിൽ LIBCURL_VERSION_അക്കം >= 0x073800 /* 7.56.0 depricated curl_formadd */
#define NSCURL_POSTDATA_T curl_mime
#define NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#define NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#അല്ലെങ്കിൽ
#define NSCURL_POSTDATA_T ഘടന curl_httppost
#define NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#define NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#endif

/** Information for a single fetch. */
ഘടന curl_fetch_info {
	ഘടന fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool sent_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_headers;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	ഘടന curl_slist *headers;	/**< List of request headers. */
	അക്ഷരം *location;		/**< Response Location header, or 0. */
	unsigned long content_length;	/**< Response Content-Length, or 0. */
	അക്ഷരം *cookie_string;	/**< Cookie string for this fetch */
	അക്ഷരം *realm;		/**< HTTP Auth Realm */
	ഘടന fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	long http_code; /**< HTTP result code from cURL. */

	uint64_t last_progress_update;	/**< Time of last progress update */
	പൂർണ്ണസംഖ്യ cert_depth; /**< deepest certificate in use */
	ഘടന cert_info cert_data[MAX_CERT_DEPTH]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
ഘടന cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	ഘടന cache_handle *r_prev; /**< Previous cached handle in ring. */
	ഘടന cache_handle *r_next; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
സ്ഥിതം CURL *fetch_blank_curl;

/** Ring of cached handles */
സ്ഥിതം ഘടന cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
സ്ഥിതം പൂർണ്ണസംഖ്യ curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
സ്ഥിതം bool curl_with_openssl;

/** Error buffer for cURL. */
സ്ഥിതം അക്ഷരം fetch_error_buffer[CURL_ERROR_SIZE];

/** Proxy authentication details. */
സ്ഥിതം അക്ഷരം fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
സ്ഥിതം bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
സ്ഥിതം bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_data(scheme));
	curl_fetchers_registered++;
	തിരികെ true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
സ്ഥിതം ശൂന്യം fetch_curl_finalise(lwc_string *scheme)
{
	ഘടന cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_data(scheme));
	എങ്കിൽ (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		എങ്കിൽ (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = ശൂന്യ;
	}

	/* Free anything remaining in the cached curl handle ring */
	ആകുമ്പോൾ (curl_handle_ring != ശൂന്യ) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
		curl_easy_cleanup(h->handle);
		സ്വതന്ത്രം(h);
	}
}


/**
 * Check if this fetcher can fetch a url.
 *
 * \param url The url to check.
 * \return true if the fetcher supports the url else false.
 */
സ്ഥിതം bool fetch_curl_can_fetch(സ്ഥിരം nsurl *url)
{
	തിരികെ nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
സ്ഥിതം ഘടന fetch_postdata *
fetch_curl_alloc_postdata(സ്ഥിരം അക്ഷരം *post_urlenc,
			  സ്ഥിരം ഘടന fetch_multipart_data *post_multipart)
{
	ഘടന fetch_postdata *postdata;
	postdata = calloc(1, വലുപ്പം(ഘടന fetch_postdata));
	എങ്കിൽ (postdata != ശൂന്യ) {

		എങ്കിൽ (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->data.urlenc = strdup(post_urlenc);
			എങ്കിൽ (postdata->data.urlenc == ശൂന്യ) {
				സ്വതന്ത്രം(postdata);
				postdata = ശൂന്യ;
			}
		} അല്ലെങ്കിൽ എങ്കിൽ (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->data.multipart = fetch_multipart_data_clone(post_multipart);
			എങ്കിൽ (postdata->data.multipart == ശൂന്യ) {
				സ്വതന്ത്രം(postdata);
				postdata = ശൂന്യ;
			}
		} അല്ലെങ്കിൽ {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	തിരികെ postdata;
}

/**
 * free postdata
 */
സ്ഥിതം ശൂന്യം fetch_curl_free_postdata(ഘടന fetch_postdata *postdata)
{
	എങ്കിൽ (postdata != ശൂന്യ) {
		തിരഞ്ഞെടുപ്പ് (postdata->type) {
		അവസ്ഥ FETCH_POSTDATA_NONE:
			നിർത്തുക;
		അവസ്ഥ FETCH_POSTDATA_URLENC:
			സ്വതന്ത്രം(postdata->data.urlenc);
			നിർത്തുക;
		അവസ്ഥ FETCH_POSTDATA_MULTIPART:
			fetch_multipart_data_destroy(postdata->data.multipart);
			നിർത്തുക;
		}

		സ്വതന്ത്രം(postdata);
	}
}

/**
 *construct a new fetch structure
 */
സ്ഥിതം ഘടന curl_fetch_info *fetch_alloc(ശൂന്യം)
{
	ഘടന curl_fetch_info *fetch;
	fetch = malloc(വലുപ്പം (*fetch));
	എങ്കിൽ (fetch == ശൂന്യ)
		തിരികെ ശൂന്യ;

	fetch->curl_handle = ശൂന്യ;
	fetch->sent_ssl_chain = false;
	fetch->had_headers = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->headers = ശൂന്യ;
	fetch->url = ശൂന്യ;
	fetch->host = ശൂന്യ;
	fetch->location = ശൂന്യ;
	fetch->content_length = 0;
	fetch->http_code = 0;
	fetch->cookie_string = ശൂന്യ;
	fetch->realm = ശൂന്യ;
	fetch->last_progress_update = 0;
	fetch->postdata = ശൂന്യ;
	fetch->curl_postdata = ശൂന്യ;

	/* Clear certificate chain data */
	memset(fetch->cert_data, 0, വലുപ്പം(fetch->cert_data));
	fetch->cert_depth = -1;

	തിരികെ fetch;
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
സ്ഥിതം ശൂന്യം *
fetch_curl_setup(ഘടന fetch *parent_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 സ്ഥിരം അക്ഷരം *post_urlenc,
		 സ്ഥിരം ഘടന fetch_multipart_data *post_multipart,
		 സ്ഥിരം അക്ഷരം **headers)
{
	ഘടന curl_fetch_info *fetch;
	ഘടന curl_slist *slist;
	പൂർണ്ണസംഖ്യ i;

	fetch = fetch_alloc();
	എങ്കിൽ (fetch == ശൂന്യ)
		തിരികെ ശൂന്യ;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = parent_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	എങ്കിൽ (fetch->host == ശൂന്യ) {
		പോകുക failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	എങ്കിൽ (fetch->postdata == ശൂന്യ) {
		പോകുക failed;
	}

#define APPEND(list, മൂല്യം) \
	slist = curl_slist_append(list, മൂല്യം);		\
	എങ്കിൽ (slist == ശൂന്യ)				\
		പോകുക failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->headers, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->headers, "Expect:");

	എങ്കിൽ ((nsoption_charp(accept_language) != ശൂന്യ) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		അക്ഷരം s[80];
		snprintf(s, വലുപ്പം s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[വലുപ്പം s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	എങ്കിൽ (nsoption_charp(accept_charset) != ശൂന്യ &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		അക്ഷരം s[80];
		snprintf(s, വലുപ്പം s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[വലുപ്പം s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	എങ്കിൽ (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->headers, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	വേണ്ടി (i = 0; headers[i] != ശൂന്യ; i++) {
		APPEND(fetch->headers, headers[i]);
	}

	തിരികെ fetch;

#undef APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->headers);
	സ്വതന്ത്രം(fetch);
	തിരികെ ശൂന്യ;
}


#ifdef WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
സ്ഥിതം ഘടന cert_chain *
fetch_curl_get_cached_chain(ഘടന curl_fetch_info *f)
{
	ഘടന cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	എങ്കിൽ (chain == ശൂന്യ) {
		chain = hashmap_തിരുകുക(curl_fetch_ssl_hashmap, f->url);
	}

	തിരികെ chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
സ്ഥിതം ശൂന്യം
fetch_curl_store_certs_in_cache(ഘടന curl_fetch_info *f)
{
	size_t depth;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_DEPTH];
	ഘടന cert_chain chain, *cached_chain;
	ഘടന cert_info *certs;

	memset(&chain, 0, വലുപ്പം(chain));

	certs = f->cert_data;
	chain.depth = f->cert_depth + 1; /* 0 indexed certificate depth */

	വേണ്ടി (depth = 0; depth < chain.depth; depth++) {
		എങ്കിൽ (certs[depth].cert == ശൂന്യ) {
			/* This certificate is missing, skip it */
			chain.certs[depth].err = SSL_CERT_ERR_CERT_MISSING;
			തുടരുക;
		}

		/* error code (if any) */
		തിരഞ്ഞെടുപ്പ് (certs[depth].err) {
		അവസ്ഥ X509_V_OK:
			chain.certs[depth].err = SSL_CERT_ERR_OK;
			നിർത്തുക;

		അവസ്ഥ X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			/* fallthrough */
		അവസ്ഥ X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_താക്കോൽ:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_ISSUER;
			നിർത്തുക;

		അവസ്ഥ X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		അവസ്ഥ X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		അവസ്ഥ X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		അവസ്ഥ X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_SIG;
			നിർത്തുക;

		അവസ്ഥ X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		അവസ്ഥ X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_YOUNG;
			നിർത്തുക;

		അവസ്ഥ X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		അവസ്ഥ X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_OLD;
			നിർത്തുക;

		അവസ്ഥ X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			chain.certs[depth].err = SSL_CERT_ERR_SELF_SIGNED;
			നിർത്തുക;

		അവസ്ഥ X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			chain.certs[depth].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			നിർത്തുക;

		അവസ്ഥ X509_V_ERR_CERT_REVOKED:
			chain.certs[depth].err = SSL_CERT_ERR_REVOKED;
			നിർത്തുക;

		അവസ്ഥ X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[depth].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			നിർത്തുക;

		സ്ഥിരസ്ഥിതി:
			chain.certs[depth].err = SSL_CERT_ERR_UNKNOWN;
			നിർത്തുക;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[depth].cert);
		BIO_get_mem_ptr(mem, &buf[depth]);
		(ശൂന്യം) BIO_set_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[depth].der = (uint8_t *)buf[depth]->data;
		chain.certs[depth].der_length = buf[depth]->length;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	എങ്കിൽ (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	വേണ്ടി (depth = 0; depth < chain.depth; depth++) {
		എങ്കിൽ (chain.certs[depth].err == SSL_CERT_ERR_CERT_MISSING) {
			തുടരുക;
		}
		എങ്കിൽ (buf[depth] != ശൂന്യ) {
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
സ്ഥിതം പൂർണ്ണസംഖ്യ
fetch_curl_verify_callback(പൂർണ്ണസംഖ്യ verify_ok, X509_STORE_CTX *x509_ctx)
{
	പൂർണ്ണസംഖ്യ depth;
	ഘടന curl_fetch_info *fetch;

	depth = X509_STORE_CTX_get_error_depth(x509_ctx);
	fetch = X509_STORE_CTX_get_app_data(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	എങ്കിൽ (depth >= MAX_CERT_DEPTH) {
		X509_STORE_CTX_set_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		തിരികെ 0;
	}

	/* record the max depth */
	എങ്കിൽ (depth > fetch->cert_depth) {
		fetch->cert_depth = depth;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	എങ്കിൽ (!fetch->cert_data[depth].cert) {
		fetch->cert_data[depth].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_up_ref(fetch->cert_data[depth].cert);
		fetch->cert_data[depth].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	എങ്കിൽ (depth > 0) {
		verify_ok = 1;
	} അല്ലെങ്കിൽ {
		/* search for deeper certificates in the chain with errors */
		വേണ്ടി (depth = fetch->cert_depth; depth > 0; depth--) {
			എങ്കിൽ (fetch->cert_data[depth].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_set_error(x509_ctx, fetch->cert_data[depth].err);
			}
		}
	}

	തിരികെ verify_ok;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, ശൂന്യം *parm)
{
	ഘടന curl_fetch_info *f = (ഘടന curl_fetch_info *) parm;
	പൂർണ്ണസംഖ്യ ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_set_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_data(f->host),
					 lwc_string_length(f->host));

	/* Store fetch struct in context for verify callback */
	എങ്കിൽ (ok) {
		ok = X509_STORE_CTX_set_app_data(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	എങ്കിൽ (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_in_cache(f);

	തിരികെ ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
സ്ഥിതം CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, ശൂന്യം *_sslctx, ശൂന്യം *parm)
{
	ഘടന curl_fetch_info *f = (ഘടന curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	long options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_set_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	എങ്കിൽ (f->downgrade_tls) {
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

	തിരികെ CURLE_OK;
}


#endif /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
സ്ഥിതം ശൂന്യം
fetch_curl_report_certs_upstream(ഘടന curl_fetch_info *f)
{
	fetch_സന്ദേശം സന്ദേശം;
	ഘടന cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	എങ്കിൽ (chain != ശൂന്യ) {
		സന്ദേശം.type = FETCH_CERTS;
		സന്ദേശം.data.chain = chain;

		fetch_send_callback(&സന്ദേശം, f->fetch_handle);
	}

	f->sent_ssl_chain = true;
}

#എങ്കിൽ LIBCURL_VERSION_അക്കം >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
ഘടന curl_mime_ctx {
	അക്ഷരം *buffer;
	curl_off_t size;
	curl_off_t position;
};

സ്ഥിതം size_t mime_data_read_callback(അക്ഷരം *buffer, size_t size, size_t nitems, ശൂന്യം *arg)
{
	ഘടന curl_mime_ctx *mctx = (ഘടന curl_mime_ctx *) arg;
	curl_off_t sz = mctx->size - mctx->position;

	nitems *= size;
	എങ്കിൽ(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	എങ്കിൽ(sz) {
		memcpy(buffer, mctx->buffer + mctx->position, sz);
	}
	mctx->position += sz;
	തിരികെ sz;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ mime_data_seek_callback(ശൂന്യം *arg, curl_off_t offset, പൂർണ്ണസംഖ്യ origin)
{
	ഘടന curl_mime_ctx *mctx = (ഘടന curl_mime_ctx *) arg;

	തിരഞ്ഞെടുപ്പ്(origin) {
	അവസ്ഥ SEEK_END:
		offset += mctx->size;
		നിർത്തുക;
	അവസ്ഥ SEEK_CUR:
		offset += mctx->position;
		നിർത്തുക;
	}

	എങ്കിൽ(offset < 0) {
		തിരികെ CURL_SEEKFUNC_FAIL;
	}
	mctx->position = offset;
	തിരികെ CURL_SEEKFUNC_OK;
}

സ്ഥിതം ശൂന്യം mime_data_free_callback(ശൂന്യം *arg)
{
	ഘടന curl_mime_ctx *mctx = (ഘടന curl_mime_ctx *) arg;
	സ്വതന്ത്രം(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
സ്ഥിതം curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    സ്ഥിരം ഘടന fetch_multipart_data *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode code = CURLE_OK;
	size_t മൂല്യം_len;

	cmime = curl_mime_init(chandle);
	എങ്കിൽ (cmime == ശൂന്യ) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		തിരികെ ശൂന്യ;
	}

	/* iterate post data */
	വേണ്ടി (; multipart != ശൂന്യ; multipart = multipart->next) {
		part = curl_mime_addpart(cmime);
		എങ്കിൽ (part == ശൂന്യ) {
			പോകുക convert_failed;
		}

		code = curl_mime_name(part, multipart->name);
		എങ്കിൽ (code != CURLE_OK) {
			പോകുക convert_failed;
		}

		മൂല്യം_len = strlen(multipart->മൂല്യം);

		എങ്കിൽ (multipart->file && മൂല്യം_len==0) {
			/* file entries with no filename require special handling */
			code=curl_mime_data(part, multipart->മൂല്യം, മൂല്യം_len);
			എങ്കിൽ (code != CURLE_OK) {
				പോകുക convert_failed;
			}

			code = curl_mime_filename(part, "");
			എങ്കിൽ (code != CURLE_OK) {
				പോകുക convert_failed;
			}

			code = curl_mime_type(part, "application/octet-stream");
			എങ്കിൽ (code != CURLE_OK) {
				പോകുക convert_failed;
			}

		} അല്ലെങ്കിൽ എങ്കിൽ(multipart->file) {
			/* file entry */
			nserror ret;
			അക്ഷരം *leafname = ശൂന്യ;
			അക്ഷരം *mimetype = ശൂന്യ;

			code = curl_mime_filedata(part, multipart->rawfile);
			എങ്കിൽ (code != CURLE_OK) {
				പോകുക convert_failed;
			}

			ret = guit->file->basename(multipart->മൂല്യം, &leafname, ശൂന്യ);
			എങ്കിൽ (ret != NSERROR_OK) {
				പോകുക convert_failed;
			}
			code = curl_mime_filename(part, leafname);
			സ്വതന്ത്രം(leafname);
			എങ്കിൽ (code != CURLE_OK) {
				പോകുക convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->മൂല്യം);
			എങ്കിൽ (mimetype == ശൂന്യ) {
				mimetype=strdup("text/plain");
			}
			എങ്കിൽ (mimetype == ശൂന്യ) {
				പോകുക convert_failed;
			}
			code = curl_mime_type(part, mimetype);
			സ്വതന്ത്രം(mimetype);
			എങ്കിൽ (code != CURLE_OK) {
				പോകുക convert_failed;
			}

		} അല്ലെങ്കിൽ {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			ഘടന curl_mime_ctx *cb_ctx;
			cb_ctx = malloc(വലുപ്പം(ഘടന curl_mime_ctx));
			എങ്കിൽ (cb_ctx == ശൂന്യ) {
				പോകുക convert_failed;
			}
			cb_ctx->buffer = multipart->മൂല്യം;
			cb_ctx->size = മൂല്യം_len;
			cb_ctx->position = 0;
			code = curl_mime_data_cb(part,
						 മൂല്യം_len,
						 mime_data_read_callback,
						 mime_data_seek_callback,
						 mime_data_free_callback,
						 cb_ctx);
			എങ്കിൽ (code != CURLE_OK) {
				സ്വതന്ത്രം(cb_ctx);
				പോകുക convert_failed;
			}
		}
	}

	തിരികെ cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", code);
	curl_mime_free(cmime);
	തിരികെ ശൂന്യ;
}

#അല്ലെങ്കിൽ /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
സ്ഥിതം ഘടന curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    സ്ഥിരം ഘടന fetch_multipart_data *control)
{
	ഘടന curl_httppost *post = ശൂന്യ, *last = ശൂന്യ;
	CURLFORMcode code;
	nserror ret;

	വേണ്ടി (; control; control = control->next) {
		എങ്കിൽ (control->file) {
			അക്ഷരം *leafname = ശൂന്യ;
			ret = guit->file->basename(control->മൂല്യം, &leafname, ശൂന്യ);
			എങ്കിൽ (ret != NSERROR_OK) {
				തുടരുക;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			എങ്കിൽ (control->മൂല്യം[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				സ്ഥിതം അക്ഷരം buf;

				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_BUFFER, control->മൂല്യം,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_END);
				എങ്കിൽ (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", code,
					      control->name);
			} അല്ലെങ്കിൽ {
				അക്ഷരം *mimetype = guit->fetch->mimetype(control->മൂല്യം);
				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_END);
				എങ്കിൽ (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      code,
					      control->name,
					      control->മൂല്യം);
				സ്വതന്ത്രം(mimetype);
			}
			സ്വതന്ത്രം(leafname);
		} അല്ലെങ്കിൽ {
			code = curl_formadd(&post, &last,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->മൂല്യം,
					    CURLFORM_END);
			എങ്കിൽ (code != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", code,
				      control->name, control->മൂല്യം);
		}
	}

	തിരികെ post;
}

#endif  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
സ്ഥിതം CURLcode fetch_curl_set_postdata(ഘടന curl_fetch_info *f)
{
	CURLcode code = CURLE_OK;

#undef SETOPT
#define SETOPT(option, മൂല്യം) { \
	code = curl_easy_setopt(f->curl_handle, option, മൂല്യം);	\
	എങ്കിൽ (code != CURLE_OK)					\
		തിരികെ code;					\
	}

	തിരഞ്ഞെടുപ്പ് (f->postdata->type) {
	അവസ്ഥ FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, ശൂന്യ);
		SETOPT(NSCURL_POSTDATA_CURLOPT, ശൂന്യ);
		SETOPT(CURLOPT_HTTPGET, 1L);
		നിർത്തുക;

	അവസ്ഥ FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, ശൂന്യ);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->data.urlenc);
		നിർത്തുക;

	അവസ്ഥ FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, ശൂന്യ);
		SETOPT(CURLOPT_HTTPGET, 0L);
		എങ്കിൽ (f->curl_postdata == ശൂന്യ) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->data.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		നിർത്തുക;
	}
	തിരികെ code;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
സ്ഥിതം CURLcode fetch_curl_set_options(ഘടന curl_fetch_info *f)
{
	CURLcode code;
	സ്ഥിരം അക്ഷരം *auth;

#undef SETOPT
#define SETOPT(option, മൂല്യം) { \
	code = curl_easy_setopt(f->curl_handle, option, മൂല്യം);	\
	എങ്കിൽ (code != CURLE_OK)					\
		തിരികെ code;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_PRIVATE, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_PROGRESS_DATA, f);
	SETOPT(CURLOPT_HTTPHEADER, f->headers);
	code = fetch_curl_set_postdata(f);
	എങ്കിൽ (code != CURLE_OK) {
		തിരികെ code;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	എങ്കിൽ (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} അല്ലെങ്കിൽ {
		SETOPT(CURLOPT_COOKIE, ശൂന്യ);
	}

	എങ്കിൽ ((auth = urldb_get_auth_details(f->url, ശൂന്യ)) != ശൂന്യ) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} അല്ലെങ്കിൽ {
		SETOPT(CURLOPT_USERPWD, ശൂന്യ);
	}

	/* set up proxy options */
	എങ്കിൽ (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != ശൂന്യ) &&
	    (strncmp(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (long) nsoption_int(http_proxy_port));

#എങ്കിൽ LIBCURL_VERSION_അക്കം >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#endif

		എങ്കിൽ (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(long) CURLAUTH_BASIC :
					(long) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					വലുപ്പം fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} അല്ലെങ്കിൽ {
		SETOPT(CURLOPT_PROXY, ശൂന്യ);
	}


	എങ്കിൽ (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	എങ്കിൽ (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		എങ്കിൽ (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, ശൂന്യ);
			SETOPT(CURLOPT_SSL_CTX_DATA, ശൂന്യ);
		}
	} അല്ലെങ്കിൽ {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef WITH_OPENSSL
		എങ്കിൽ (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_DATA, f);
		}
#endif
	}

	തിരികെ CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
സ്ഥിതം bool
fetch_curl_initiate_fetch(ഘടന curl_fetch_info *fetch, CURL *handle)
{
	CURLcode code;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	code = fetch_curl_set_options(fetch);
	എങ്കിൽ (code != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		തിരികെ false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_add_handle(fetch_curl_multi, fetch->curl_handle);
	പ്രതിജ്ഞാ(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	തിരികെ true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
സ്ഥിതം CURL *fetch_curl_get_handle(lwc_string *host)
{
	ഘടന cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	എങ്കിൽ (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		സ്വതന്ത്രം(h);
	} അല്ലെങ്കിൽ {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	തിരികെ ret;
}


/**
 * Dispatch a single job
 */
സ്ഥിതം bool fetch_curl_start(ശൂന്യം *vfetch)
{
	ഘടന curl_fetch_info *fetch = (ഘടന curl_fetch_info*)vfetch;
	എങ്കിൽ (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		തിരികെ false;
	}
	തിരികെ fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
സ്ഥിതം ശൂന്യം fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#എങ്കിൽ LIBCURL_VERSION_അക്കം >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	തിരികെ;
#അല്ലെങ്കിൽ
	ഘടന cache_handle *h = 0;
	പൂർണ്ണസംഖ്യ c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	എങ്കിൽ (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		തിരികെ;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(ഘടന cache_handle, curl_handle_ring, c);
	എങ്കിൽ (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		എങ്കിൽ (curl_handle_ring != ശൂന്യ) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_next;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} അല്ലെങ്കിൽ {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		തിരികെ;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (ഘടന cache_handle*)malloc(വലുപ്പം(ഘടന cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_തിരുകുക(curl_handle_ring, h);
#endif
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
സ്ഥിതം ശൂന്യം fetch_curl_stop(ഘടന curl_fetch_info *f)
{
	CURLMcode codem;

	പ്രതിജ്ഞാ(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	എങ്കിൽ (f->curl_handle) {
		/* remove from curl multi handle */
		codem = curl_multi_remove_handle(fetch_curl_multi,
				f->curl_handle);
		പ്രതിജ്ഞാ(codem == CURLM_OK);
		/* Put this curl handle into the cache if wanted. */
		fetch_curl_cache_handle(f->curl_handle, f->host);
		f->curl_handle = 0;
	}

	fetch_remove_from_queues(f->fetch_handle);
}


/**
 * Abort a fetch.
 */
സ്ഥിതം ശൂന്യം fetch_curl_abort(ശൂന്യം *vf)
{
	ഘടന curl_fetch_info *f = (ഘടന curl_fetch_info *)vf;
	പ്രതിജ്ഞാ(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	എങ്കിൽ (f->curl_handle) {
		എങ്കിൽ (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->abort = true;
		} അല്ലെങ്കിൽ {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} അല്ലെങ്കിൽ {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
സ്ഥിതം ശൂന്യം fetch_curl_free(ശൂന്യം *vf)
{
	ഘടന curl_fetch_info *f = (ഘടന curl_fetch_info *)vf;
	പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	സ്വതന്ത്രം(f->location);
	സ്വതന്ത്രം(f->cookie_string);
	സ്വതന്ത്രം(f->realm);
	എങ്കിൽ (f->headers) {
		curl_slist_free_all(f->headers);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	വേണ്ടി (i = 0; i < MAX_CERT_DEPTH; i++) {
		എങ്കിൽ (f->cert_data[i].cert != ശൂന്യ) {
			X509_free(f->cert_data[i].cert);
		}
	}

	സ്വതന്ത്രം(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
സ്ഥിതം bool fetch_curl_process_headers(ഘടന curl_fetch_info *f)
{
	long http_code;
	CURLcode code;
	fetch_സന്ദേശം സന്ദേശം;

	f->had_headers = true;

	എങ്കിൽ (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		പ്രതിജ്ഞാ(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}
	http_code = f->http_code;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_code);

	എങ്കിൽ ((http_code == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		സന്ദേശം.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&സന്ദേശം, f->fetch_handle);
		തിരികെ true;
	}

	/* handle HTTP redirects (3xx response codes) */
	എങ്കിൽ (HTTP_RESPONSE_IS_3XX(http_code) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		സന്ദേശം.type = FETCH_REDIRECT;
		സന്ദേശം.data.redirect = f->location;
		fetch_send_callback(&സന്ദേശം, f->fetch_handle);
		തിരികെ true;
	}

	/* handle HTTP 401 (Authentication errors) */
	എങ്കിൽ (http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		സന്ദേശം.type = FETCH_AUTH;
		സന്ദേശം.data.auth.realm = f->realm;
		fetch_send_callback(&സന്ദേശം, f->fetch_handle);
		തിരികെ true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	എങ്കിൽ (f->only_2xx &&
	    strncmp(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_code) == false)) {
		സന്ദേശം.type = FETCH_ERROR;
		സന്ദേശം.data.error = messages_get("Not2xx");
		fetch_send_callback(&സന്ദേശം, f->fetch_handle);
		തിരികെ true;
	}

	എങ്കിൽ (f->abort)
		തിരികെ true;

	തിരികെ false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
സ്ഥിതം ശൂന്യം fetch_curl_done(CURL *curl_handle, CURLcode result)
{
	bool finished = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	ഘടന curl_fetch_info *f;
	അക്ഷരം **_hideous_hack = (അക്ഷരം **) (ശൂന്യം *) &f;
	CURLcode code;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	code = curl_easy_getinfo(curl_handle, CURLINFO_PRIVATE, _hideous_hack);
	പ്രതിജ്ഞാ(code == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	എങ്കിൽ ((abort_fetch == false) &&
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
		എങ്കിൽ (f->stopped ||
		    (!f->had_headers &&	fetch_curl_process_headers(f))) {
			; /* redirect with no body or similar */
		} അല്ലെങ്കിൽ {
			finished = true;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (result == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		എങ്കിൽ (!f->had_headers && fetch_curl_process_headers(f))
			; /* redirect with partial body, or similar */
		അല്ലെങ്കിൽ {
			finished = true;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (result == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} അല്ലെങ്കിൽ എങ്കിൽ (result == CURLE_SSL_PEER_CERTIFICATE ||
		   result == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} അല്ലെങ്കിൽ {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", result);
		error = true;
	}

	fetch_curl_stop(f);

	എങ്കിൽ (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	എങ്കിൽ (abort_fetch) {
		; /* fetch was aborted: no callback */
	} അല്ലെങ്കിൽ എങ്കിൽ (finished) {
		fetch_സന്ദേശം സന്ദേശം;
		സന്ദേശം.type = FETCH_FINISHED;
		fetch_send_callback(&സന്ദേശം, f->fetch_handle);
	} അല്ലെങ്കിൽ എങ്കിൽ (cert) {
		/* user needs to validate certificate with issue */
		fetch_സന്ദേശം സന്ദേശം;
		സന്ദേശം.type = FETCH_CERT_ERR;
		fetch_send_callback(&സന്ദേശം, f->fetch_handle);
	} അല്ലെങ്കിൽ എങ്കിൽ (error) {
		fetch_സന്ദേശം സന്ദേശം;
		തിരഞ്ഞെടുപ്പ് (result) {
		അവസ്ഥ CURLE_SSL_CONNECT_ERROR:
			സന്ദേശം.type = FETCH_SSL_ERR;
			നിർത്തുക;

		അവസ്ഥ CURLE_OPERATION_TIMEDOUT:
			സന്ദേശം.type = FETCH_TIMEDOUT;
			സന്ദേശം.data.error = curl_easy_strerror(result);
			നിർത്തുക;

		സ്ഥിരസ്ഥിതി:
			സന്ദേശം.type = FETCH_ERROR;
			സന്ദേശം.data.error = curl_easy_strerror(result);
		}

		fetch_send_callback(&സന്ദേശം, f->fetch_handle);
	}

	fetch_free(f->fetch_handle);
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */
സ്ഥിതം ശൂന്യം fetch_curl_poll(lwc_string *scheme_ignored)
{
	പൂർണ്ണസംഖ്യ running, queue;
	CURLMcode codem;
	CURLMsg *curl_സന്ദേശം;

	എങ്കിൽ (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_set, write_fd_set, exc_fd_set;
		പൂർണ്ണസംഖ്യ max_fd = -1;
		പൂർണ്ണസംഖ്യ i;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&exc_fd_set);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_set, &write_fd_set,
				&exc_fd_set, &max_fd);
		പ്രതിജ്ഞാ(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		വേണ്ടി (i = 0; i <= max_fd; i++) {
			bool പഠ = false;
			bool ലിഖ = false;
			bool error = false;

			എങ്കിൽ (FD_ISSET(i, &read_fd_set)) {
				പഠ = true;
			}
			എങ്കിൽ (FD_ISSET(i, &write_fd_set)) {
				ലിഖ = true;
			}
			എങ്കിൽ (FD_ISSET(i, &exc_fd_set)) {
				error = true;
			}
			എങ്കിൽ (പഠ || ലിഖ || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      പഠ ? "read" : "    ",
				      ലിഖ ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	ചെയ്യുക {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		എങ്കിൽ (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			തിരികെ;
		}
	} ആകുമ്പോൾ (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_സന്ദേശം = curl_multi_info_read(fetch_curl_multi, &queue);
	ആകുമ്പോൾ (curl_സന്ദേശം) {
		തിരഞ്ഞെടുപ്പ് (curl_സന്ദേശം->സന്ദേശം) {
			അവസ്ഥ CURLMSG_DONE:
				fetch_curl_done(curl_സന്ദേശം->easy_handle,
						curl_സന്ദേശം->data.result);
				നിർത്തുക;
			സ്ഥിരസ്ഥിതി:
				നിർത്തുക;
		}
		curl_സന്ദേശം = curl_multi_info_read(fetch_curl_multi, &queue);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ
fetch_curl_progress(ശൂന്യം *clientp,
		    NSCURL_PROGRESS_T dltotal,
		    NSCURL_PROGRESS_T dlnow,
		    NSCURL_PROGRESS_T ultotal,
		    NSCURL_PROGRESS_T ulnow)
{
	സ്ഥിതം അക്ഷരം fetch_progress_buffer[256]; /**< Progress buffer for cURL */
	ഘടന curl_fetch_info *f = (ഘടന curl_fetch_info *) clientp;
	uint64_t time_now_ms;
	fetch_സന്ദേശം സന്ദേശം;

	എങ്കിൽ (f->abort) {
		തിരികെ 0;
        }

	സന്ദേശം.type = FETCH_PROGRESS;
	സന്ദേശം.data.progress = fetch_progress_buffer;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&time_now_ms);
#define UPDATE_DELAY_MS (1000 / UPDATES_PER_SECOND)
	എങ്കിൽ (time_now_ms - f->last_progress_update < UPDATE_DELAY_MS) {
		തിരികെ 0;
        }
#undef UPDATE_DELAY_MS
	f->last_progress_update = time_now_ms;

	എങ്കിൽ (dltotal > 0) {
		snprintf(fetch_progress_buffer, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&സന്ദേശം, f->fetch_handle);
	} അല്ലെങ്കിൽ {
		snprintf(fetch_progress_buffer, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&സന്ദേശം, f->fetch_handle);
	}

	തിരികെ 0;
}


/**
 * Format curl debug for nslog
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 അക്ഷരം *data,
		 size_t size,
		 ശൂന്യം *userptr)
{
	സ്ഥിതം സ്ഥിരം അക്ഷരം s_infotype[CURLINFO_END][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	തിരഞ്ഞെടുപ്പ്(type) {
	അവസ്ഥ CURLINFO_TEXT:
	അവസ്ഥ CURLINFO_HEADER_OUT:
	അവസ്ഥ CURLINFO_HEADER_IN:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (പൂർണ്ണസംഖ്യ)size - 1, data);
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}
	തിരികെ 0;
}


സ്ഥിതം curl_socket_t fetch_curl_socket_open(ശൂന്യം *clientp,
		curlsocktype purpose, ഘടന curl_sockaddr *address)
{
	(ശൂന്യം) clientp;
	(ശൂന്യം) purpose;

	തിരികെ (curl_socket_t) guit->fetch->socket_open(
			address->family, address->socktype,
			address->protocol);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ fetch_curl_socket_close(ശൂന്യം *clientp, curl_socket_t item)
{
	(ശൂന്യം) clientp;

	തിരികെ guit->fetch->socket_close((പൂർണ്ണസംഖ്യ) item);
}

/**
 * Callback function for cURL.
 */
സ്ഥിതം size_t fetch_curl_data(അക്ഷരം *data, size_t size, size_t nmemb, ശൂന്യം *_f)
{
	ഘടന curl_fetch_info *f = _f;
	CURLcode code;
	fetch_സന്ദേശം സന്ദേശം;

	/* ensure we only have to get this information once */
	എങ്കിൽ (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		പ്രതിജ്ഞാ(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	എങ്കിൽ (f->http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_code = 0;
		തിരികെ size * nmemb;
	}

	എങ്കിൽ (f->abort || (!f->had_headers && fetch_curl_process_headers(f))) {
		f->stopped = true;
		തിരികെ 0;
	}

	/* send data to the caller */
	സന്ദേശം.type = FETCH_DATA;
	സന്ദേശം.data.header_or_data.buf = (സ്ഥിരം uint8_t *) data;
	സന്ദേശം.data.header_or_data.len = size * nmemb;
	fetch_send_callback(&സന്ദേശം, f->fetch_handle);

	എങ്കിൽ (f->abort) {
		f->stopped = true;
		തിരികെ 0;
	}

	തിരികെ size * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
സ്ഥിതം size_t
fetch_curl_header(അക്ഷരം *data, size_t size, size_t nmemb, ശൂന്യം *_f)
{
	ഘടന curl_fetch_info *f = _f;
	പൂർണ്ണസംഖ്യ i;
	fetch_സന്ദേശം സന്ദേശം;
	size *= nmemb;

	എങ്കിൽ (f->abort) {
		f->stopped = true;
		തിരികെ 0;
	}

	എങ്കിൽ (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	സന്ദേശം.type = FETCH_HEADER;
	സന്ദേശം.data.header_or_data.buf = (സ്ഥിരം uint8_t *) data;
	സന്ദേശം.data.header_or_data.len = size;
	fetch_send_callback(&സന്ദേശം, f->fetch_handle);

#define SKIP_ST(o) വേണ്ടി (i = (o); i < (പൂർണ്ണസംഖ്യ) size && (data[i] == ' ' || data[i] == '\t'); i++)

	എങ്കിൽ (12 < size && strncasecmp(data, "Location:", 9) == 0) {
		/* extract Location header */
		സ്വതന്ത്രം(f->location);
		f->location = malloc(size);
		എങ്കിൽ (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			തിരികെ size;
		}
		SKIP_ST(9);
		strncpy(f->location, data + i, size - i);
		f->location[size - i] = '\0';
		വേണ്ടി (i = size - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} അല്ലെങ്കിൽ എങ്കിൽ (15 < size && strncasecmp(data, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		എങ്കിൽ (i < (പൂർണ്ണസംഖ്യ)size && '0' <= data[i] && data[i] <= '9')
			f->content_length = atol(data + i);
	} അല്ലെങ്കിൽ എങ്കിൽ (17 < size && strncasecmp(data, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		ആകുമ്പോൾ (i < (പൂർണ്ണസംഖ്യ) size - 5 &&
				strncasecmp(data + i, "realm", 5))
			i++;
		ആകുമ്പോൾ (i < (പൂർണ്ണസംഖ്യ) size - 1 && data[++i] != '"')
			/* */;
		i++;

		എങ്കിൽ (i < (പൂർണ്ണസംഖ്യ) size) {
			size_t end = i;

			ആകുമ്പോൾ (end < size && data[end] != '"')
				++end;

			എങ്കിൽ (end < size) {
				സ്വതന്ത്രം(f->realm);
				f->realm = malloc(end - i + 1);
				എങ്കിൽ (f->realm != ശൂന്യ) {
					strncpy(f->realm, data + i, end - i);
					f->realm[end - i] = '\0';
				}
			}
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (11 < size && strncasecmp(data, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_set_cookie(f->fetch_handle, &data[i]);
	}

	തിരികെ size;
#undef SKIP_ST
}

സ്ഥിതം പൂർണ്ണസംഖ്യ fetch_curl_fdset(lwc_string *scheme, fd_set *read_set,
			    fd_set *write_set, fd_set *error_set)
{
	CURLMcode code;
	പൂർണ്ണസംഖ്യ maxfd = -1;

	code = curl_multi_fdset(fetch_curl_multi,
				read_set,
				write_set,
				error_set,
				&maxfd);
	പ്രതിജ്ഞാ(code == CURLM_OK);

	തിരികെ maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_register(ശൂന്യം)
{
	CURLcode code;
	curl_version_info_data *data;
	പൂർണ്ണസംഖ്യ i;
	lwc_string *scheme;
	സ്ഥിരം ഘടന fetcher_operation_table fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.setup = fetch_curl_setup,
		.start = fetch_curl_start,
		.abort = fetch_curl_abort,
		.സ്വതന്ത്രം = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#എങ്കിൽ LIBCURL_VERSION_അക്കം >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, ശൂന്യ, ശൂന്യ);
	എങ്കിൽ (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} അല്ലെങ്കിൽ {
		curl_with_openssl = false;
	}
#endif

	NSLOG(netsurf, INFO, "curl_version %s", curl_version());

	code = curl_global_init(CURL_GLOBAL_ALL);
	എങ്കിൽ (code != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		തിരികെ NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	എങ്കിൽ (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		തിരികെ NSERROR_INIT_FAILED;
	}

#എങ്കിൽ LIBCURL_VERSION_അക്കം >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		പൂർണ്ണസംഖ്യ maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#undef SETOPT
#define SETOPT(option, മൂല്യം) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, മൂല്യം);	\
	എങ്കിൽ (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		പോകുക curl_multi_setopt_failed;				\
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
	എങ്കിൽ (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		തിരികെ NSERROR_INIT_FAILED;
	}

#undef SETOPT
#define SETOPT(option, മൂല്യം) \
	code = curl_easy_setopt(fetch_blank_curl, option, മൂല്യം);	\
	എങ്കിൽ (code != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		പോകുക curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_buffer);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	എങ്കിൽ (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} അല്ലെങ്കിൽ {
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
	SETOPT(CURLOPT_താഴെയുള്ള_SPEED_LIMIT, 1L);
	SETOPT(CURLOPT_താഴെയുള്ള_SPEED_TIME, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (long)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	എങ്കിൽ (nsoption_charp(ca_bundle) &&
	    strcmp(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	എങ്കിൽ (nsoption_charp(ca_path) && strcmp(nsoption_charp(ca_path), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_path));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_path));
	}

#എങ്കിൽ LIBCURL_VERSION_അക്കം < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	code = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, ശൂന്യ);
	എങ്കിൽ (code != CURLE_OK) {
		curl_with_openssl = false;
	} അല്ലെങ്കിൽ {
		curl_with_openssl = true;
	}
#endif

	എങ്കിൽ (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#എങ്കിൽ LIBCURL_VERSION_അക്കം >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		code = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		എങ്കിൽ (code != CURLE_OK && code != CURLE_NOT_BUILT_IN)
			പോകുക curl_easy_setopt_failed;
#endif
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	data = curl_version_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	എങ്കിൽ (curl_fetch_ssl_hashmap == ശൂന്യ) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		തിരികെ NSERROR_NOMEM;
	}

	വേണ്ടി (i = 0; data->protocols[i]; i++) {
		എങ്കിൽ (strcmp(data->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(data->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} അല്ലെങ്കിൽ {
			/* Ignore non-http(s) protocols */
			തുടരുക;
		}

		എങ്കിൽ (fetcher_add(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      data->protocols[i]);
		}
	}

	തിരികെ NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	തിരികെ NSERROR_INIT_FAILED;

#എങ്കിൽ LIBCURL_VERSION_അക്കം >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	തിരികെ NSERROR_INIT_FAILED;
#endif
}
