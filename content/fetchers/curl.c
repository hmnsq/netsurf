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
#ಅನ್ತರ್ಭಾವ "utils/inet.h"

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>

#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>
#ಅನ್ತರ್ಭಾವ <nsutils/time.h>

#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/hashmap.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/ring.h"
#ಅನ್ತರ್ಭಾವ "utils/useragent.h"
#ಅನ್ತರ್ಭಾವ "utils/file.h"
#ಅನ್ತರ್ಭಾವ "utils/string.h"
#ಅನ್ತರ್ಭಾವ "netsurf/fetch.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers/curl.h"
#ಅನ್ತರ್ಭಾವ "content/urldb.h"

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

#ಅನ್ತರ್ಭಾವ <openssl/ssl.h>
#ಅನ್ತರ್ಭಾವ <openssl/x509v3.h>

#ಅನ್ಯಥಾ /* WITH_OPENSSL */

ಪ್ರಕಾರನಿರ್ದೇಶ ವರ್ಣ X509;

ಸ್ಥಿರ ಶೂನ್ಯ X509_free(X509 *cert)
{
	ಮುಕ್ತ(cert);
}

#endif /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
ಸ್ಥಿರ uint32_t
curl_fetch_ssl_ಕೀಲಿ_hash(ಶೂನ್ಯ *ಕೀಲಿ)
{
	nsurl *url = ಕೀಲಿ;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *port = nsurl_get_component(url, NSURL_PORT);
	uint32_t hash;

	ಯದಿ (port == ರಿಕ್ತ)
		port = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_ಮೌಲ್ಯ(hostname) ^ lwc_string_hash_ಮೌಲ್ಯ(port);

	lwc_string_unref(hostname);
	lwc_string_unref(port);

	ಪ್ರತ್ಯಾಗಮ hash;
}

/* We only compare the hostname and port */
ಸ್ಥಿರ bool
curl_fetch_ssl_ಕೀಲಿ_eq(ಶೂನ್ಯ *key1, ಶೂನ್ಯ *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_PORT);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_PORT);
	bool iseq = false;

	ಯದಿ (port1 == ರಿಕ್ತ)
		port1 = lwc_string_ref(corestring_lwc_443);
	ಯದಿ (port2 == ರಿಕ್ತ)
		port2 = lwc_string_ref(corestring_lwc_443);

	ಯದಿ (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		ಗಚ್ಛ out;

	iseq = false;
	ಯದಿ (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		ಗಚ್ಛ out;

out:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	ಪ್ರತ್ಯಾಗಮ iseq;
}

ಸ್ಥಿರ ಶೂನ್ಯ *
curl_fetch_ssl_ಮೌಲ್ಯ_alloc(ಶೂನ್ಯ *ಕೀಲಿ)
{
	ಸಂರಚನಾ cert_chain *out;

	ಯದಿ (cert_chain_alloc(0, &out) != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ out;
}

ಸ್ಥಿರ ಶೂನ್ಯ
curl_fetch_ssl_ಮೌಲ್ಯ_destroy(ಶೂನ್ಯ *ಮೌಲ್ಯ)
{
	ಸಂರಚನಾ cert_chain *chain = ಮೌಲ್ಯ;
	ಯದಿ (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

ಸ್ಥಿರ hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.ಕೀಲಿ_clone = (hashmap_ಕೀಲಿ_clone_t)nsurl_ref,
	.ಕೀಲಿ_destroy = (hashmap_ಕೀಲಿ_destroy_t)nsurl_unref,
	.ಕೀಲಿ_eq = curl_fetch_ssl_ಕೀಲಿ_eq,
	.ಕೀಲಿ_hash = curl_fetch_ssl_ಕೀಲಿ_hash,
	.ಮೌಲ್ಯ_alloc = curl_fetch_ssl_ಮೌಲ್ಯ_alloc,
	.ಮೌಲ್ಯ_destroy = curl_fetch_ssl_ಮೌಲ್ಯ_destroy,
};

ಸ್ಥಿರ hashmap_t *curl_fetch_ssl_hashmap = ರಿಕ್ತ;

/** SSL certificate info */
ಸಂರಚನಾ cert_info {
	X509 *cert;		/**< Pointer to certificate */
	long err;		/**< OpenSSL error code */
};

#ಯದಿ LIBCURL_VERSION_ಅಂಕೆ >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_XFERINFOFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_XFERINFODATA
#define NSCURL_PROGRESS_T curl_off_t
#ಅನ್ಯಥಾ
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_PROGRESSFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_PROGRESSDATA
#define NSCURL_PROGRESS_T ದ್ವಿಗುಣ
#endif

#ಯದಿ LIBCURL_VERSION_ಅಂಕೆ >= 0x073800 /* 7.56.0 depricated curl_formadd */
#define NSCURL_POSTDATA_T curl_mime
#define NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#define NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#ಅನ್ಯಥಾ
#define NSCURL_POSTDATA_T ಸಂರಚನಾ curl_httppost
#define NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#define NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#endif

/** Information for a single fetch. */
ಸಂರಚನಾ curl_fetch_info {
	ಸಂರಚನಾ fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool sent_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_headers;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	ಸಂರಚನಾ curl_slist *headers;	/**< List of request headers. */
	ವರ್ಣ *location;		/**< Response Location header, or 0. */
	unsigned long content_length;	/**< Response Content-Length, or 0. */
	ವರ್ಣ *cookie_string;	/**< Cookie string for this fetch */
	ವರ್ಣ *realm;		/**< HTTP Auth Realm */
	ಸಂರಚನಾ fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	long http_code; /**< HTTP result code from cURL. */

	uint64_t last_progress_update;	/**< Time of last progress update */
	ಪೂರ್ಣಾಙ್ಕ cert_depth; /**< deepest certificate in use */
	ಸಂರಚನಾ cert_info cert_data[MAX_CERT_DEPTH]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
ಸಂರಚನಾ cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	ಸಂರಚನಾ cache_handle *r_prev; /**< Previous cached handle in ring. */
	ಸಂರಚನಾ cache_handle *r_next; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
ಸ್ಥಿರ CURL *fetch_blank_curl;

/** Ring of cached handles */
ಸ್ಥಿರ ಸಂರಚನಾ cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
ಸ್ಥಿರ bool curl_with_openssl;

/** Error buffer for cURL. */
ಸ್ಥಿರ ವರ್ಣ fetch_error_buffer[CURL_ERROR_SIZE];

/** Proxy authentication details. */
ಸ್ಥಿರ ವರ್ಣ fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
ಸ್ಥಿರ bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
ಸ್ಥಿರ bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_data(scheme));
	curl_fetchers_registered++;
	ಪ್ರತ್ಯಾಗಮ true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_finalise(lwc_string *scheme)
{
	ಸಂರಚನಾ cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_data(scheme));
	ಯದಿ (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		ಯದಿ (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = ರಿಕ್ತ;
	}

	/* Free anything remaining in the cached curl handle ring */
	ಯಾವತ್ (curl_handle_ring != ರಿಕ್ತ) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
		curl_easy_cleanup(h->handle);
		ಮುಕ್ತ(h);
	}
}


/**
 * Check if this fetcher can fetch a url.
 *
 * \param url The url to check.
 * \return true if the fetcher supports the url else false.
 */
ಸ್ಥಿರ bool fetch_curl_can_fetch(ಧ್ರುವ nsurl *url)
{
	ಪ್ರತ್ಯಾಗಮ nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
ಸ್ಥಿರ ಸಂರಚನಾ fetch_postdata *
fetch_curl_alloc_postdata(ಧ್ರುವ ವರ್ಣ *post_urlenc,
			  ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *post_multipart)
{
	ಸಂರಚನಾ fetch_postdata *postdata;
	postdata = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ fetch_postdata));
	ಯದಿ (postdata != ರಿಕ್ತ) {

		ಯದಿ (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->data.urlenc = strdup(post_urlenc);
			ಯದಿ (postdata->data.urlenc == ರಿಕ್ತ) {
				ಮುಕ್ತ(postdata);
				postdata = ರಿಕ್ತ;
			}
		} ಅನ್ಯಥಾ ಯದಿ (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->data.multipart = fetch_multipart_data_clone(post_multipart);
			ಯದಿ (postdata->data.multipart == ರಿಕ್ತ) {
				ಮುಕ್ತ(postdata);
				postdata = ರಿಕ್ತ;
			}
		} ಅನ್ಯಥಾ {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	ಪ್ರತ್ಯಾಗಮ postdata;
}

/**
 * free postdata
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_free_postdata(ಸಂರಚನಾ fetch_postdata *postdata)
{
	ಯದಿ (postdata != ರಿಕ್ತ) {
		ವಿಕಲ್ಪ (postdata->type) {
		ಪಕ್ಷ FETCH_POSTDATA_NONE:
			ಭಙ್ಗ;
		ಪಕ್ಷ FETCH_POSTDATA_URLENC:
			ಮುಕ್ತ(postdata->data.urlenc);
			ಭಙ್ಗ;
		ಪಕ್ಷ FETCH_POSTDATA_MULTIPART:
			fetch_multipart_data_destroy(postdata->data.multipart);
			ಭಙ್ಗ;
		}

		ಮುಕ್ತ(postdata);
	}
}

/**
 *construct a new fetch structure
 */
ಸ್ಥಿರ ಸಂರಚನಾ curl_fetch_info *fetch_alloc(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ curl_fetch_info *fetch;
	fetch = malloc(ಪರಿಮಾಣ (*fetch));
	ಯದಿ (fetch == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	fetch->curl_handle = ರಿಕ್ತ;
	fetch->sent_ssl_chain = false;
	fetch->had_headers = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->headers = ರಿಕ್ತ;
	fetch->url = ರಿಕ್ತ;
	fetch->host = ರಿಕ್ತ;
	fetch->location = ರಿಕ್ತ;
	fetch->content_length = 0;
	fetch->http_code = 0;
	fetch->cookie_string = ರಿಕ್ತ;
	fetch->realm = ರಿಕ್ತ;
	fetch->last_progress_update = 0;
	fetch->postdata = ರಿಕ್ತ;
	fetch->curl_postdata = ರಿಕ್ತ;

	/* Clear certificate chain data */
	memset(fetch->cert_data, 0, ಪರಿಮಾಣ(fetch->cert_data));
	fetch->cert_depth = -1;

	ಪ್ರತ್ಯಾಗಮ fetch;
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
ಸ್ಥಿರ ಶೂನ್ಯ *
fetch_curl_setup(ಸಂರಚನಾ fetch *parent_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ಧ್ರುವ ವರ್ಣ *post_urlenc,
		 ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *post_multipart,
		 ಧ್ರುವ ವರ್ಣ **headers)
{
	ಸಂರಚನಾ curl_fetch_info *fetch;
	ಸಂರಚನಾ curl_slist *slist;
	ಪೂರ್ಣಾಙ್ಕ i;

	fetch = fetch_alloc();
	ಯದಿ (fetch == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = parent_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	ಯದಿ (fetch->host == ರಿಕ್ತ) {
		ಗಚ್ಛ failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	ಯದಿ (fetch->postdata == ರಿಕ್ತ) {
		ಗಚ್ಛ failed;
	}

#define APPEND(list, ಮೌಲ್ಯ) \
	slist = curl_slist_append(list, ಮೌಲ್ಯ);		\
	ಯದಿ (slist == ರಿಕ್ತ)				\
		ಗಚ್ಛ failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->headers, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->headers, "Expect:");

	ಯದಿ ((nsoption_charp(accept_language) != ರಿಕ್ತ) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		ವರ್ಣ s[80];
		snprintf(s, ಪರಿಮಾಣ s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[ಪರಿಮಾಣ s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	ಯದಿ (nsoption_charp(accept_charset) != ರಿಕ್ತ &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		ವರ್ಣ s[80];
		snprintf(s, ಪರಿಮಾಣ s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[ಪರಿಮಾಣ s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	ಯದಿ (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->headers, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	ಪ್ರತಿ (i = 0; headers[i] != ರಿಕ್ತ; i++) {
		APPEND(fetch->headers, headers[i]);
	}

	ಪ್ರತ್ಯಾಗಮ fetch;

#undef APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->headers);
	ಮುಕ್ತ(fetch);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


#ifdef WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
ಸ್ಥಿರ ಸಂರಚನಾ cert_chain *
fetch_curl_get_cached_chain(ಸಂರಚನಾ curl_fetch_info *f)
{
	ಸಂರಚನಾ cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	ಯದಿ (chain == ರಿಕ್ತ) {
		chain = hashmap_ಸೇರಿಸು(curl_fetch_ssl_hashmap, f->url);
	}

	ಪ್ರತ್ಯಾಗಮ chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
ಸ್ಥಿರ ಶೂನ್ಯ
fetch_curl_store_certs_in_cache(ಸಂರಚನಾ curl_fetch_info *f)
{
	size_t depth;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_DEPTH];
	ಸಂರಚನಾ cert_chain chain, *cached_chain;
	ಸಂರಚನಾ cert_info *certs;

	memset(&chain, 0, ಪರಿಮಾಣ(chain));

	certs = f->cert_data;
	chain.depth = f->cert_depth + 1; /* 0 indexed certificate depth */

	ಪ್ರತಿ (depth = 0; depth < chain.depth; depth++) {
		ಯದಿ (certs[depth].cert == ರಿಕ್ತ) {
			/* This certificate is missing, skip it */
			chain.certs[depth].err = SSL_CERT_ERR_CERT_MISSING;
			ಅನುವರ್ತ;
		}

		/* error code (if any) */
		ವಿಕಲ್ಪ (certs[depth].err) {
		ಪಕ್ಷ X509_V_OK:
			chain.certs[depth].err = SSL_CERT_ERR_OK;
			ಭಙ್ಗ;

		ಪಕ್ಷ X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			/* fallthrough */
		ಪಕ್ಷ X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_ಕೀಲಿ:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_ISSUER;
			ಭಙ್ಗ;

		ಪಕ್ಷ X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		ಪಕ್ಷ X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		ಪಕ್ಷ X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		ಪಕ್ಷ X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_SIG;
			ಭಙ್ಗ;

		ಪಕ್ಷ X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		ಪಕ್ಷ X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_YOUNG;
			ಭಙ್ಗ;

		ಪಕ್ಷ X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		ಪಕ್ಷ X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_OLD;
			ಭಙ್ಗ;

		ಪಕ್ಷ X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			chain.certs[depth].err = SSL_CERT_ERR_SELF_SIGNED;
			ಭಙ್ಗ;

		ಪಕ್ಷ X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			chain.certs[depth].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			ಭಙ್ಗ;

		ಪಕ್ಷ X509_V_ERR_CERT_REVOKED:
			chain.certs[depth].err = SSL_CERT_ERR_REVOKED;
			ಭಙ್ಗ;

		ಪಕ್ಷ X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[depth].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			ಭಙ್ಗ;

		ಸ್ವಭಾವ:
			chain.certs[depth].err = SSL_CERT_ERR_UNKNOWN;
			ಭಙ್ಗ;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[depth].cert);
		BIO_get_mem_ptr(mem, &buf[depth]);
		(ಶೂನ್ಯ) BIO_set_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[depth].der = (uint8_t *)buf[depth]->data;
		chain.certs[depth].der_length = buf[depth]->length;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	ಯದಿ (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	ಪ್ರತಿ (depth = 0; depth < chain.depth; depth++) {
		ಯದಿ (chain.certs[depth].err == SSL_CERT_ERR_CERT_MISSING) {
			ಅನುವರ್ತ;
		}
		ಯದಿ (buf[depth] != ರಿಕ್ತ) {
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
fetch_curl_verify_callback(ಪೂರ್ಣಾಙ್ಕ verify_ok, X509_STORE_CTX *x509_ctx)
{
	ಪೂರ್ಣಾಙ್ಕ depth;
	ಸಂರಚನಾ curl_fetch_info *fetch;

	depth = X509_STORE_CTX_get_error_depth(x509_ctx);
	fetch = X509_STORE_CTX_get_app_data(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	ಯದಿ (depth >= MAX_CERT_DEPTH) {
		X509_STORE_CTX_set_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* record the max depth */
	ಯದಿ (depth > fetch->cert_depth) {
		fetch->cert_depth = depth;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	ಯದಿ (!fetch->cert_data[depth].cert) {
		fetch->cert_data[depth].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_up_ref(fetch->cert_data[depth].cert);
		fetch->cert_data[depth].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	ಯದಿ (depth > 0) {
		verify_ok = 1;
	} ಅನ್ಯಥಾ {
		/* search for deeper certificates in the chain with errors */
		ಪ್ರತಿ (depth = fetch->cert_depth; depth > 0; depth--) {
			ಯದಿ (fetch->cert_data[depth].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_set_error(x509_ctx, fetch->cert_data[depth].err);
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ verify_ok;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, ಶೂನ್ಯ *parm)
{
	ಸಂರಚನಾ curl_fetch_info *f = (ಸಂರಚನಾ curl_fetch_info *) parm;
	ಪೂರ್ಣಾಙ್ಕ ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_set_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_data(f->host),
					 lwc_string_length(f->host));

	/* Store fetch struct in context for verify callback */
	ಯದಿ (ok) {
		ok = X509_STORE_CTX_set_app_data(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	ಯದಿ (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_in_cache(f);

	ಪ್ರತ್ಯಾಗಮ ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
ಸ್ಥಿರ CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, ಶೂನ್ಯ *_sslctx, ಶೂನ್ಯ *parm)
{
	ಸಂರಚನಾ curl_fetch_info *f = (ಸಂರಚನಾ curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	long options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_set_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	ಯದಿ (f->downgrade_tls) {
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

	ಪ್ರತ್ಯಾಗಮ CURLE_OK;
}


#endif /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
ಸ್ಥಿರ ಶೂನ್ಯ
fetch_curl_report_certs_upstream(ಸಂರಚನಾ curl_fetch_info *f)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಸಂರಚನಾ cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	ಯದಿ (chain != ರಿಕ್ತ) {
		ಸಂದೇಶ.type = FETCH_CERTS;
		ಸಂದೇಶ.data.chain = chain;

		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
	}

	f->sent_ssl_chain = true;
}

#ಯದಿ LIBCURL_VERSION_ಅಂಕೆ >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
ಸಂರಚನಾ curl_mime_ctx {
	ವರ್ಣ *buffer;
	curl_off_t size;
	curl_off_t position;
};

ಸ್ಥಿರ size_t mime_data_read_callback(ವರ್ಣ *buffer, size_t size, size_t nitems, ಶೂನ್ಯ *arg)
{
	ಸಂರಚನಾ curl_mime_ctx *mctx = (ಸಂರಚನಾ curl_mime_ctx *) arg;
	curl_off_t sz = mctx->size - mctx->position;

	nitems *= size;
	ಯದಿ(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	ಯದಿ(sz) {
		memcpy(buffer, mctx->buffer + mctx->position, sz);
	}
	mctx->position += sz;
	ಪ್ರತ್ಯಾಗಮ sz;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mime_data_seek_callback(ಶೂನ್ಯ *arg, curl_off_t offset, ಪೂರ್ಣಾಙ್ಕ origin)
{
	ಸಂರಚನಾ curl_mime_ctx *mctx = (ಸಂರಚನಾ curl_mime_ctx *) arg;

	ವಿಕಲ್ಪ(origin) {
	ಪಕ್ಷ SEEK_END:
		offset += mctx->size;
		ಭಙ್ಗ;
	ಪಕ್ಷ SEEK_CUR:
		offset += mctx->position;
		ಭಙ್ಗ;
	}

	ಯದಿ(offset < 0) {
		ಪ್ರತ್ಯಾಗಮ CURL_SEEKFUNC_FAIL;
	}
	mctx->position = offset;
	ಪ್ರತ್ಯಾಗಮ CURL_SEEKFUNC_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ mime_data_free_callback(ಶೂನ್ಯ *arg)
{
	ಸಂರಚನಾ curl_mime_ctx *mctx = (ಸಂರಚನಾ curl_mime_ctx *) arg;
	ಮುಕ್ತ(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
ಸ್ಥಿರ curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode code = CURLE_OK;
	size_t ಮೌಲ್ಯ_len;

	cmime = curl_mime_init(chandle);
	ಯದಿ (cmime == ರಿಕ್ತ) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* iterate post data */
	ಪ್ರತಿ (; multipart != ರಿಕ್ತ; multipart = multipart->next) {
		part = curl_mime_addpart(cmime);
		ಯದಿ (part == ರಿಕ್ತ) {
			ಗಚ್ಛ convert_failed;
		}

		code = curl_mime_name(part, multipart->name);
		ಯದಿ (code != CURLE_OK) {
			ಗಚ್ಛ convert_failed;
		}

		ಮೌಲ್ಯ_len = strlen(multipart->ಮೌಲ್ಯ);

		ಯದಿ (multipart->file && ಮೌಲ್ಯ_len==0) {
			/* file entries with no filename require special handling */
			code=curl_mime_data(part, multipart->ಮೌಲ್ಯ, ಮೌಲ್ಯ_len);
			ಯದಿ (code != CURLE_OK) {
				ಗಚ್ಛ convert_failed;
			}

			code = curl_mime_filename(part, "");
			ಯದಿ (code != CURLE_OK) {
				ಗಚ್ಛ convert_failed;
			}

			code = curl_mime_type(part, "application/octet-stream");
			ಯದಿ (code != CURLE_OK) {
				ಗಚ್ಛ convert_failed;
			}

		} ಅನ್ಯಥಾ ಯದಿ(multipart->file) {
			/* file entry */
			nserror ret;
			ವರ್ಣ *leafname = ರಿಕ್ತ;
			ವರ್ಣ *mimetype = ರಿಕ್ತ;

			code = curl_mime_filedata(part, multipart->rawfile);
			ಯದಿ (code != CURLE_OK) {
				ಗಚ್ಛ convert_failed;
			}

			ret = guit->file->basename(multipart->ಮೌಲ್ಯ, &leafname, ರಿಕ್ತ);
			ಯದಿ (ret != NSERROR_OK) {
				ಗಚ್ಛ convert_failed;
			}
			code = curl_mime_filename(part, leafname);
			ಮುಕ್ತ(leafname);
			ಯದಿ (code != CURLE_OK) {
				ಗಚ್ಛ convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->ಮೌಲ್ಯ);
			ಯದಿ (mimetype == ರಿಕ್ತ) {
				mimetype=strdup("text/plain");
			}
			ಯದಿ (mimetype == ರಿಕ್ತ) {
				ಗಚ್ಛ convert_failed;
			}
			code = curl_mime_type(part, mimetype);
			ಮುಕ್ತ(mimetype);
			ಯದಿ (code != CURLE_OK) {
				ಗಚ್ಛ convert_failed;
			}

		} ಅನ್ಯಥಾ {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			ಸಂರಚನಾ curl_mime_ctx *cb_ctx;
			cb_ctx = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ curl_mime_ctx));
			ಯದಿ (cb_ctx == ರಿಕ್ತ) {
				ಗಚ್ಛ convert_failed;
			}
			cb_ctx->buffer = multipart->ಮೌಲ್ಯ;
			cb_ctx->size = ಮೌಲ್ಯ_len;
			cb_ctx->position = 0;
			code = curl_mime_data_cb(part,
						 ಮೌಲ್ಯ_len,
						 mime_data_read_callback,
						 mime_data_seek_callback,
						 mime_data_free_callback,
						 cb_ctx);
			ಯದಿ (code != CURLE_OK) {
				ಮುಕ್ತ(cb_ctx);
				ಗಚ್ಛ convert_failed;
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", code);
	curl_mime_free(cmime);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

#ಅನ್ಯಥಾ /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
ಸ್ಥಿರ ಸಂರಚನಾ curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *control)
{
	ಸಂರಚನಾ curl_httppost *post = ರಿಕ್ತ, *last = ರಿಕ್ತ;
	CURLFORMcode code;
	nserror ret;

	ಪ್ರತಿ (; control; control = control->next) {
		ಯದಿ (control->file) {
			ವರ್ಣ *leafname = ರಿಕ್ತ;
			ret = guit->file->basename(control->ಮೌಲ್ಯ, &leafname, ರಿಕ್ತ);
			ಯದಿ (ret != NSERROR_OK) {
				ಅನುವರ್ತ;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			ಯದಿ (control->ಮೌಲ್ಯ[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				ಸ್ಥಿರ ವರ್ಣ buf;

				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_BUFFER, control->ಮೌಲ್ಯ,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_END);
				ಯದಿ (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", code,
					      control->name);
			} ಅನ್ಯಥಾ {
				ವರ್ಣ *mimetype = guit->fetch->mimetype(control->ಮೌಲ್ಯ);
				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_END);
				ಯದಿ (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      code,
					      control->name,
					      control->ಮೌಲ್ಯ);
				ಮುಕ್ತ(mimetype);
			}
			ಮುಕ್ತ(leafname);
		} ಅನ್ಯಥಾ {
			code = curl_formadd(&post, &last,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->ಮೌಲ್ಯ,
					    CURLFORM_END);
			ಯದಿ (code != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", code,
				      control->name, control->ಮೌಲ್ಯ);
		}
	}

	ಪ್ರತ್ಯಾಗಮ post;
}

#endif  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
ಸ್ಥಿರ CURLcode fetch_curl_set_postdata(ಸಂರಚನಾ curl_fetch_info *f)
{
	CURLcode code = CURLE_OK;

#undef SETOPT
#define SETOPT(option, ಮೌಲ್ಯ) { \
	code = curl_easy_setopt(f->curl_handle, option, ಮೌಲ್ಯ);	\
	ಯದಿ (code != CURLE_OK)					\
		ಪ್ರತ್ಯಾಗಮ code;					\
	}

	ವಿಕಲ್ಪ (f->postdata->type) {
	ಪಕ್ಷ FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, ರಿಕ್ತ);
		SETOPT(NSCURL_POSTDATA_CURLOPT, ರಿಕ್ತ);
		SETOPT(CURLOPT_HTTPGET, 1L);
		ಭಙ್ಗ;

	ಪಕ್ಷ FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, ರಿಕ್ತ);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->data.urlenc);
		ಭಙ್ಗ;

	ಪಕ್ಷ FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, ರಿಕ್ತ);
		SETOPT(CURLOPT_HTTPGET, 0L);
		ಯದಿ (f->curl_postdata == ರಿಕ್ತ) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->data.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ code;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
ಸ್ಥಿರ CURLcode fetch_curl_set_options(ಸಂರಚನಾ curl_fetch_info *f)
{
	CURLcode code;
	ಧ್ರುವ ವರ್ಣ *auth;

#undef SETOPT
#define SETOPT(option, ಮೌಲ್ಯ) { \
	code = curl_easy_setopt(f->curl_handle, option, ಮೌಲ್ಯ);	\
	ಯದಿ (code != CURLE_OK)					\
		ಪ್ರತ್ಯಾಗಮ code;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_PRIVATE, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_PROGRESS_DATA, f);
	SETOPT(CURLOPT_HTTPHEADER, f->headers);
	code = fetch_curl_set_postdata(f);
	ಯದಿ (code != CURLE_OK) {
		ಪ್ರತ್ಯಾಗಮ code;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	ಯದಿ (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} ಅನ್ಯಥಾ {
		SETOPT(CURLOPT_COOKIE, ರಿಕ್ತ);
	}

	ಯದಿ ((auth = urldb_get_auth_details(f->url, ರಿಕ್ತ)) != ರಿಕ್ತ) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} ಅನ್ಯಥಾ {
		SETOPT(CURLOPT_USERPWD, ರಿಕ್ತ);
	}

	/* set up proxy options */
	ಯದಿ (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != ರಿಕ್ತ) &&
	    (strncmp(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (long) nsoption_int(http_proxy_port));

#ಯದಿ LIBCURL_VERSION_ಅಂಕೆ >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#endif

		ಯದಿ (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(long) CURLAUTH_BASIC :
					(long) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					ಪರಿಮಾಣ fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} ಅನ್ಯಥಾ {
		SETOPT(CURLOPT_PROXY, ರಿಕ್ತ);
	}


	ಯದಿ (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	ಯದಿ (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		ಯದಿ (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, ರಿಕ್ತ);
			SETOPT(CURLOPT_SSL_CTX_DATA, ರಿಕ್ತ);
		}
	} ಅನ್ಯಥಾ {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef WITH_OPENSSL
		ಯದಿ (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_DATA, f);
		}
#endif
	}

	ಪ್ರತ್ಯಾಗಮ CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
ಸ್ಥಿರ bool
fetch_curl_initiate_fetch(ಸಂರಚನಾ curl_fetch_info *fetch, CURL *handle)
{
	CURLcode code;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	code = fetch_curl_set_options(fetch);
	ಯದಿ (code != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_add_handle(fetch_curl_multi, fetch->curl_handle);
	ಪ್ರತಿಜ್ಞಾ(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
ಸ್ಥಿರ CURL *fetch_curl_get_handle(lwc_string *host)
{
	ಸಂರಚನಾ cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	ಯದಿ (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		ಮುಕ್ತ(h);
	} ಅನ್ಯಥಾ {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	ಪ್ರತ್ಯಾಗಮ ret;
}


/**
 * Dispatch a single job
 */
ಸ್ಥಿರ bool fetch_curl_start(ಶೂನ್ಯ *vfetch)
{
	ಸಂರಚನಾ curl_fetch_info *fetch = (ಸಂರಚನಾ curl_fetch_info*)vfetch;
	ಯದಿ (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		ಪ್ರತ್ಯಾಗಮ false;
	}
	ಪ್ರತ್ಯಾಗಮ fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#ಯದಿ LIBCURL_VERSION_ಅಂಕೆ >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	ಪ್ರತ್ಯಾಗಮ;
#ಅನ್ಯಥಾ
	ಸಂರಚನಾ cache_handle *h = 0;
	ಪೂರ್ಣಾಙ್ಕ c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	ಯದಿ (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		ಪ್ರತ್ಯಾಗಮ;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(ಸಂರಚನಾ cache_handle, curl_handle_ring, c);
	ಯದಿ (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		ಯದಿ (curl_handle_ring != ರಿಕ್ತ) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_next;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} ಅನ್ಯಥಾ {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		ಪ್ರತ್ಯಾಗಮ;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (ಸಂರಚನಾ cache_handle*)malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_ಸೇರಿಸು(curl_handle_ring, h);
#endif
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_stop(ಸಂರಚನಾ curl_fetch_info *f)
{
	CURLMcode codem;

	ಪ್ರತಿಜ್ಞಾ(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	ಯದಿ (f->curl_handle) {
		/* remove from curl multi handle */
		codem = curl_multi_remove_handle(fetch_curl_multi,
				f->curl_handle);
		ಪ್ರತಿಜ್ಞಾ(codem == CURLM_OK);
		/* Put this curl handle into the cache if wanted. */
		fetch_curl_cache_handle(f->curl_handle, f->host);
		f->curl_handle = 0;
	}

	fetch_remove_from_queues(f->fetch_handle);
}


/**
 * Abort a fetch.
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_abort(ಶೂನ್ಯ *vf)
{
	ಸಂರಚನಾ curl_fetch_info *f = (ಸಂರಚನಾ curl_fetch_info *)vf;
	ಪ್ರತಿಜ್ಞಾ(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	ಯದಿ (f->curl_handle) {
		ಯದಿ (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->abort = true;
		} ಅನ್ಯಥಾ {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} ಅನ್ಯಥಾ {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_free(ಶೂನ್ಯ *vf)
{
	ಸಂರಚನಾ curl_fetch_info *f = (ಸಂರಚನಾ curl_fetch_info *)vf;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	ಮುಕ್ತ(f->location);
	ಮುಕ್ತ(f->cookie_string);
	ಮುಕ್ತ(f->realm);
	ಯದಿ (f->headers) {
		curl_slist_free_all(f->headers);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	ಪ್ರತಿ (i = 0; i < MAX_CERT_DEPTH; i++) {
		ಯದಿ (f->cert_data[i].cert != ರಿಕ್ತ) {
			X509_free(f->cert_data[i].cert);
		}
	}

	ಮುಕ್ತ(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
ಸ್ಥಿರ bool fetch_curl_process_headers(ಸಂರಚನಾ curl_fetch_info *f)
{
	long http_code;
	CURLcode code;
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	f->had_headers = true;

	ಯದಿ (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		ಪ್ರತಿಜ್ಞಾ(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}
	http_code = f->http_code;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_code);

	ಯದಿ ((http_code == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		ಸಂದೇಶ.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* handle HTTP redirects (3xx response codes) */
	ಯದಿ (HTTP_RESPONSE_IS_3XX(http_code) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		ಸಂದೇಶ.type = FETCH_REDIRECT;
		ಸಂದೇಶ.data.redirect = f->location;
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* handle HTTP 401 (Authentication errors) */
	ಯದಿ (http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		ಸಂದೇಶ.type = FETCH_AUTH;
		ಸಂದೇಶ.data.auth.realm = f->realm;
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	ಯದಿ (f->only_2xx &&
	    strncmp(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_code) == false)) {
		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.data.error = messages_get("Not2xx");
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
		ಪ್ರತ್ಯಾಗಮ true;
	}

	ಯದಿ (f->abort)
		ಪ್ರತ್ಯಾಗಮ true;

	ಪ್ರತ್ಯಾಗಮ false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_done(CURL *curl_handle, CURLcode result)
{
	bool finished = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	ಸಂರಚನಾ curl_fetch_info *f;
	ವರ್ಣ **_hideous_hack = (ವರ್ಣ **) (ಶೂನ್ಯ *) &f;
	CURLcode code;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	code = curl_easy_getinfo(curl_handle, CURLINFO_PRIVATE, _hideous_hack);
	ಪ್ರತಿಜ್ಞಾ(code == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	ಯದಿ ((abort_fetch == false) &&
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
		ಯದಿ (f->stopped ||
		    (!f->had_headers &&	fetch_curl_process_headers(f))) {
			; /* redirect with no body or similar */
		} ಅನ್ಯಥಾ {
			finished = true;
		}
	} ಅನ್ಯಥಾ ಯದಿ (result == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		ಯದಿ (!f->had_headers && fetch_curl_process_headers(f))
			; /* redirect with partial body, or similar */
		ಅನ್ಯಥಾ {
			finished = true;
		}
	} ಅನ್ಯಥಾ ಯದಿ (result == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} ಅನ್ಯಥಾ ಯದಿ (result == CURLE_SSL_PEER_CERTIFICATE ||
		   result == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} ಅನ್ಯಥಾ {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", result);
		error = true;
	}

	fetch_curl_stop(f);

	ಯದಿ (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	ಯದಿ (abort_fetch) {
		; /* fetch was aborted: no callback */
	} ಅನ್ಯಥಾ ಯದಿ (finished) {
		fetch_ಸಂದೇಶ ಸಂದೇಶ;
		ಸಂದೇಶ.type = FETCH_FINISHED;
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
	} ಅನ್ಯಥಾ ಯದಿ (cert) {
		/* user needs to validate certificate with issue */
		fetch_ಸಂದೇಶ ಸಂದೇಶ;
		ಸಂದೇಶ.type = FETCH_CERT_ERR;
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
	} ಅನ್ಯಥಾ ಯದಿ (error) {
		fetch_ಸಂದೇಶ ಸಂದೇಶ;
		ವಿಕಲ್ಪ (result) {
		ಪಕ್ಷ CURLE_SSL_CONNECT_ERROR:
			ಸಂದೇಶ.type = FETCH_SSL_ERR;
			ಭಙ್ಗ;

		ಪಕ್ಷ CURLE_OPERATION_TIMEDOUT:
			ಸಂದೇಶ.type = FETCH_TIMEDOUT;
			ಸಂದೇಶ.data.error = curl_easy_strerror(result);
			ಭಙ್ಗ;

		ಸ್ವಭಾವ:
			ಸಂದೇಶ.type = FETCH_ERROR;
			ಸಂದೇಶ.data.error = curl_easy_strerror(result);
		}

		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
	}

	fetch_free(f->fetch_handle);
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_poll(lwc_string *scheme_ignored)
{
	ಪೂರ್ಣಾಙ್ಕ running, queue;
	CURLMcode codem;
	CURLMsg *curl_ಸಂದೇಶ;

	ಯದಿ (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_set, write_fd_set, exc_fd_set;
		ಪೂರ್ಣಾಙ್ಕ max_fd = -1;
		ಪೂರ್ಣಾಙ್ಕ i;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&exc_fd_set);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_set, &write_fd_set,
				&exc_fd_set, &max_fd);
		ಪ್ರತಿಜ್ಞಾ(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		ಪ್ರತಿ (i = 0; i <= max_fd; i++) {
			bool ಪಠ = false;
			bool ಲಿಖ = false;
			bool error = false;

			ಯದಿ (FD_ISSET(i, &read_fd_set)) {
				ಪಠ = true;
			}
			ಯದಿ (FD_ISSET(i, &write_fd_set)) {
				ಲಿಖ = true;
			}
			ಯದಿ (FD_ISSET(i, &exc_fd_set)) {
				error = true;
			}
			ಯದಿ (ಪಠ || ಲಿಖ || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      ಪಠ ? "read" : "    ",
				      ಲಿಖ ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	ಕುರು {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		ಯದಿ (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			ಪ್ರತ್ಯಾಗಮ;
		}
	} ಯಾವತ್ (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_ಸಂದೇಶ = curl_multi_info_read(fetch_curl_multi, &queue);
	ಯಾವತ್ (curl_ಸಂದೇಶ) {
		ವಿಕಲ್ಪ (curl_ಸಂದೇಶ->ಸಂದೇಶ) {
			ಪಕ್ಷ CURLMSG_DONE:
				fetch_curl_done(curl_ಸಂದೇಶ->easy_handle,
						curl_ಸಂದೇಶ->data.result);
				ಭಙ್ಗ;
			ಸ್ವಭಾವ:
				ಭಙ್ಗ;
		}
		curl_ಸಂದೇಶ = curl_multi_info_read(fetch_curl_multi, &queue);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
fetch_curl_progress(ಶೂನ್ಯ *clientp,
		    NSCURL_PROGRESS_T dltotal,
		    NSCURL_PROGRESS_T dlnow,
		    NSCURL_PROGRESS_T ultotal,
		    NSCURL_PROGRESS_T ulnow)
{
	ಸ್ಥಿರ ವರ್ಣ fetch_progress_buffer[256]; /**< Progress buffer for cURL */
	ಸಂರಚನಾ curl_fetch_info *f = (ಸಂರಚನಾ curl_fetch_info *) clientp;
	uint64_t time_now_ms;
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	ಯದಿ (f->abort) {
		ಪ್ರತ್ಯಾಗಮ 0;
        }

	ಸಂದೇಶ.type = FETCH_PROGRESS;
	ಸಂದೇಶ.data.progress = fetch_progress_buffer;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&time_now_ms);
#define UPDATE_DELAY_MS (1000 / UPDATES_PER_SECOND)
	ಯದಿ (time_now_ms - f->last_progress_update < UPDATE_DELAY_MS) {
		ಪ್ರತ್ಯಾಗಮ 0;
        }
#undef UPDATE_DELAY_MS
	f->last_progress_update = time_now_ms;

	ಯದಿ (dltotal > 0) {
		snprintf(fetch_progress_buffer, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
	} ಅನ್ಯಥಾ {
		snprintf(fetch_progress_buffer, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}


/**
 * Format curl debug for nslog
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 ವರ್ಣ *data,
		 size_t size,
		 ಶೂನ್ಯ *userptr)
{
	ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ s_infotype[CURLINFO_END][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	ವಿಕಲ್ಪ(type) {
	ಪಕ್ಷ CURLINFO_TEXT:
	ಪಕ್ಷ CURLINFO_HEADER_OUT:
	ಪಕ್ಷ CURLINFO_HEADER_IN:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (ಪೂರ್ಣಾಙ್ಕ)size - 1, data);
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ curl_socket_t fetch_curl_socket_open(ಶೂನ್ಯ *clientp,
		curlsocktype purpose, ಸಂರಚನಾ curl_sockaddr *address)
{
	(ಶೂನ್ಯ) clientp;
	(ಶೂನ್ಯ) purpose;

	ಪ್ರತ್ಯಾಗಮ (curl_socket_t) guit->fetch->socket_open(
			address->family, address->socktype,
			address->protocol);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fetch_curl_socket_close(ಶೂನ್ಯ *clientp, curl_socket_t item)
{
	(ಶೂನ್ಯ) clientp;

	ಪ್ರತ್ಯಾಗಮ guit->fetch->socket_close((ಪೂರ್ಣಾಙ್ಕ) item);
}

/**
 * Callback function for cURL.
 */
ಸ್ಥಿರ size_t fetch_curl_data(ವರ್ಣ *data, size_t size, size_t nmemb, ಶೂನ್ಯ *_f)
{
	ಸಂರಚನಾ curl_fetch_info *f = _f;
	CURLcode code;
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	/* ensure we only have to get this information once */
	ಯದಿ (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		ಪ್ರತಿಜ್ಞಾ(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	ಯದಿ (f->http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_code = 0;
		ಪ್ರತ್ಯಾಗಮ size * nmemb;
	}

	ಯದಿ (f->abort || (!f->had_headers && fetch_curl_process_headers(f))) {
		f->stopped = true;
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* send data to the caller */
	ಸಂದೇಶ.type = FETCH_DATA;
	ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *) data;
	ಸಂದೇಶ.data.header_or_data.len = size * nmemb;
	fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);

	ಯದಿ (f->abort) {
		f->stopped = true;
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ size * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
ಸ್ಥಿರ size_t
fetch_curl_header(ವರ್ಣ *data, size_t size, size_t nmemb, ಶೂನ್ಯ *_f)
{
	ಸಂರಚನಾ curl_fetch_info *f = _f;
	ಪೂರ್ಣಾಙ್ಕ i;
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	size *= nmemb;

	ಯದಿ (f->abort) {
		f->stopped = true;
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಯದಿ (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	ಸಂದೇಶ.type = FETCH_HEADER;
	ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *) data;
	ಸಂದೇಶ.data.header_or_data.len = size;
	fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);

#define SKIP_ST(o) ಪ್ರತಿ (i = (o); i < (ಪೂರ್ಣಾಙ್ಕ) size && (data[i] == ' ' || data[i] == '\t'); i++)

	ಯದಿ (12 < size && strncasecmp(data, "Location:", 9) == 0) {
		/* extract Location header */
		ಮುಕ್ತ(f->location);
		f->location = malloc(size);
		ಯದಿ (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			ಪ್ರತ್ಯಾಗಮ size;
		}
		SKIP_ST(9);
		strncpy(f->location, data + i, size - i);
		f->location[size - i] = '\0';
		ಪ್ರತಿ (i = size - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} ಅನ್ಯಥಾ ಯದಿ (15 < size && strncasecmp(data, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		ಯದಿ (i < (ಪೂರ್ಣಾಙ್ಕ)size && '0' <= data[i] && data[i] <= '9')
			f->content_length = atol(data + i);
	} ಅನ್ಯಥಾ ಯದಿ (17 < size && strncasecmp(data, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		ಯಾವತ್ (i < (ಪೂರ್ಣಾಙ್ಕ) size - 5 &&
				strncasecmp(data + i, "realm", 5))
			i++;
		ಯಾವತ್ (i < (ಪೂರ್ಣಾಙ್ಕ) size - 1 && data[++i] != '"')
			/* */;
		i++;

		ಯದಿ (i < (ಪೂರ್ಣಾಙ್ಕ) size) {
			size_t end = i;

			ಯಾವತ್ (end < size && data[end] != '"')
				++end;

			ಯದಿ (end < size) {
				ಮುಕ್ತ(f->realm);
				f->realm = malloc(end - i + 1);
				ಯದಿ (f->realm != ರಿಕ್ತ) {
					strncpy(f->realm, data + i, end - i);
					f->realm[end - i] = '\0';
				}
			}
		}
	} ಅನ್ಯಥಾ ಯದಿ (11 < size && strncasecmp(data, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_set_cookie(f->fetch_handle, &data[i]);
	}

	ಪ್ರತ್ಯಾಗಮ size;
#undef SKIP_ST
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fetch_curl_fdset(lwc_string *scheme, fd_set *read_set,
			    fd_set *write_set, fd_set *error_set)
{
	CURLMcode code;
	ಪೂರ್ಣಾಙ್ಕ maxfd = -1;

	code = curl_multi_fdset(fetch_curl_multi,
				read_set,
				write_set,
				error_set,
				&maxfd);
	ಪ್ರತಿಜ್ಞಾ(code == CURLM_OK);

	ಪ್ರತ್ಯಾಗಮ maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_register(ಶೂನ್ಯ)
{
	CURLcode code;
	curl_version_info_data *data;
	ಪೂರ್ಣಾಙ್ಕ i;
	lwc_string *scheme;
	ಧ್ರುವ ಸಂರಚನಾ fetcher_operation_table fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.setup = fetch_curl_setup,
		.start = fetch_curl_start,
		.abort = fetch_curl_abort,
		.ಮುಕ್ತ = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#ಯದಿ LIBCURL_VERSION_ಅಂಕೆ >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, ರಿಕ್ತ, ರಿಕ್ತ);
	ಯದಿ (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} ಅನ್ಯಥಾ {
		curl_with_openssl = false;
	}
#endif

	NSLOG(netsurf, INFO, "curl_version %s", curl_version());

	code = curl_global_init(CURL_GLOBAL_ALL);
	ಯದಿ (code != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		ಪ್ರತ್ಯಾಗಮ NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	ಯದಿ (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		ಪ್ರತ್ಯಾಗಮ NSERROR_INIT_FAILED;
	}

#ಯದಿ LIBCURL_VERSION_ಅಂಕೆ >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		ಪೂರ್ಣಾಙ್ಕ maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#undef SETOPT
#define SETOPT(option, ಮೌಲ್ಯ) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, ಮೌಲ್ಯ);	\
	ಯದಿ (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		ಗಚ್ಛ curl_multi_setopt_failed;				\
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
	ಯದಿ (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		ಪ್ರತ್ಯಾಗಮ NSERROR_INIT_FAILED;
	}

#undef SETOPT
#define SETOPT(option, ಮೌಲ್ಯ) \
	code = curl_easy_setopt(fetch_blank_curl, option, ಮೌಲ್ಯ);	\
	ಯದಿ (code != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		ಗಚ್ಛ curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_buffer);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	ಯದಿ (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} ಅನ್ಯಥಾ {
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
	SETOPT(CURLOPT_ಕೆಳಗಿನ_SPEED_LIMIT, 1L);
	SETOPT(CURLOPT_ಕೆಳಗಿನ_SPEED_TIME, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (long)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	ಯದಿ (nsoption_charp(ca_bundle) &&
	    strcmp(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	ಯದಿ (nsoption_charp(ca_path) && strcmp(nsoption_charp(ca_path), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_path));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_path));
	}

#ಯದಿ LIBCURL_VERSION_ಅಂಕೆ < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	code = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, ರಿಕ್ತ);
	ಯದಿ (code != CURLE_OK) {
		curl_with_openssl = false;
	} ಅನ್ಯಥಾ {
		curl_with_openssl = true;
	}
#endif

	ಯದಿ (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#ಯದಿ LIBCURL_VERSION_ಅಂಕೆ >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		code = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		ಯದಿ (code != CURLE_OK && code != CURLE_NOT_BUILT_IN)
			ಗಚ್ಛ curl_easy_setopt_failed;
#endif
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	data = curl_version_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	ಯದಿ (curl_fetch_ssl_hashmap == ರಿಕ್ತ) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ಪ್ರತಿ (i = 0; data->protocols[i]; i++) {
		ಯದಿ (strcmp(data->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} ಅನ್ಯಥಾ ಯದಿ (strcmp(data->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} ಅನ್ಯಥಾ {
			/* Ignore non-http(s) protocols */
			ಅನುವರ್ತ;
		}

		ಯದಿ (fetcher_add(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      data->protocols[i]);
		}
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	ಪ್ರತ್ಯಾಗಮ NSERROR_INIT_FAILED;

#ಯದಿ LIBCURL_VERSION_ಅಂಕೆ >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	ಪ್ರತ್ಯಾಗಮ NSERROR_INIT_FAILED;
#endif
}
