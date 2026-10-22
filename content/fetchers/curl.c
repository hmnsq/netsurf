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
#ವ್ಯಾಖ್ಯಾನಿಸು UPDATES_ಪ್ರತಿ_ಎರಡನೇ 2

/**
 * The ciphersuites the browser is prepared to use for TLS1.3
 */
#ವ್ಯಾಖ್ಯಾನಿಸು CIPHER_SUITES						\
	"TLS_AES_256_GCM_SHA384:"				\
	"TLS_CHACHA20_POLY1305_SHA256:"				\
	"TLS_AES_128_GCM_SHA256"

/**
 * The ciphersuites the browser is prepared to use for TLS<1.3
 */
#ವ್ಯಾಖ್ಯಾನಿಸು CIPHER_LIST						\
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
#ವ್ಯಾಖ್ಯಾನಿಸು CIPHER_LIST_LEGACY					\
	/* as above */						\
	CIPHER_LIST":"						\
	/* enable (non-PFS) RSA AES GCM suites */		\
	"RSA+AESGCM:"						\
	/* enable (non-PFS) RSA (mac=SHA1) AES CBC suites */	\
	"RSA+AES+SHA1"

/* Open SSL compatability for certificate handling */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WITH_OPENSSL

#ಅನ್ತರ್ಭಾವ <openssl/ssl.h>
#ಅನ್ತರ್ಭಾವ <openssl/x509v3.h>

#ಇಲ್ಲದಿದ್ದರೆ /* WITH_OPENSSL */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಅಕ್ಷರ X509;

ಸ್ಥಿರ ಶೂನ್ಯ X509_free(X509 *cert)
{
	ಬಿಡುಗಡೆ(cert);
}

#ಆದರೆ_ಅಂತ್ಯ /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
ಸ್ಥಿರ uint32_t
curl_fetch_ssl_ಕೀಲಿ_hash(ಶೂನ್ಯ *ಕೀಲಿ)
{
	nsurl *url = ಕೀಲಿ;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *ಪೋರ್ಟ್ = nsurl_get_component(url, NSURL_ಪೋರ್ಟ್);
	uint32_t hash;

	ಆದರೆ (ಪೋರ್ಟ್ == ಖಾಲಿ)
		ಪೋರ್ಟ್ = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_ಮೌಲ್ಯ(hostname) ^ lwc_string_hash_ಮೌಲ್ಯ(ಪೋರ್ಟ್);

	lwc_string_unref(hostname);
	lwc_string_unref(ಪೋರ್ಟ್);

	ಹಿಂತಿರುಗಿ hash;
}

/* We only compare the hostname and port */
ಸ್ಥಿರ bool
curl_fetch_ssl_ಕೀಲಿ_eq(ಶೂನ್ಯ *key1, ಶೂನ್ಯ *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_ಪೋರ್ಟ್);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_ಪೋರ್ಟ್);
	bool iseq = false;

	ಆದರೆ (port1 == ಖಾಲಿ)
		port1 = lwc_string_ref(corestring_lwc_443);
	ಆದರೆ (port2 == ಖಾಲಿ)
		port2 = lwc_string_ref(corestring_lwc_443);

	ಆದರೆ (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		ಹೋಗು ಹೊರಹೋಗು;

	iseq = false;
	ಆದರೆ (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		ಹೋಗು ಹೊರಹೋಗು;

ಹೊರಹೋಗು:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	ಹಿಂತಿರುಗಿ iseq;
}

ಸ್ಥಿರ ಶೂನ್ಯ *
curl_fetch_ssl_ಮೌಲ್ಯ_alloc(ಶೂನ್ಯ *ಕೀಲಿ)
{
	ರಚನೆ cert_chain *ಹೊರಹೋಗು;

	ಆದರೆ (cert_chain_alloc(0, &ಹೊರಹೋಗು) != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ ಹೊರಹೋಗು;
}

ಸ್ಥಿರ ಶೂನ್ಯ
curl_fetch_ssl_ಮೌಲ್ಯ_destroy(ಶೂನ್ಯ *ಮೌಲ್ಯ)
{
	ರಚನೆ cert_chain *chain = ಮೌಲ್ಯ;
	ಆದರೆ (cert_chain_free(chain) != NSERROR_OK) {
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

ಸ್ಥಿರ hashmap_t *curl_fetch_ssl_hashmap = ಖಾಲಿ;

/** SSL certificate info */
ರಚನೆ cert_info {
	X509 *cert;		/**< Pointer to certificate */
	ದೀರ್ಘ err;		/**< OpenSSL error code */
};

#ಆದರೆ LIBCURL_ಆವೃತ್ತಿ_ಅಂಕೆ >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#ವ್ಯಾಖ್ಯಾನಿಸು NSCURLOPT_ಪ್ರಗತಿ_FUNCTION CURLOPT_XFERINFOFUNCTION
#ವ್ಯಾಖ್ಯಾನಿಸು NSCURLOPT_ಪ್ರಗತಿ_ದತ್ತಾಂಶ CURLOPT_XFERINFODATA
#ವ್ಯಾಖ್ಯಾನಿಸು NSCURL_ಪ್ರಗತಿ_T curl_off_t
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು NSCURLOPT_ಪ್ರಗತಿ_FUNCTION CURLOPT_PROGRESSFUNCTION
#ವ್ಯಾಖ್ಯಾನಿಸು NSCURLOPT_ಪ್ರಗತಿ_ದತ್ತಾಂಶ CURLOPT_PROGRESSDATA
#ವ್ಯಾಖ್ಯಾನಿಸು NSCURL_ಪ್ರಗತಿ_T ದ್ವಿಗುಣ
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ LIBCURL_ಆವೃತ್ತಿ_ಅಂಕೆ >= 0x073800 /* 7.56.0 depricated curl_formadd */
#ವ್ಯಾಖ್ಯಾನಿಸು NSCURL_POSTDATA_T curl_mime
#ವ್ಯಾಖ್ಯಾನಿಸು NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#ವ್ಯಾಖ್ಯಾನಿಸು NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು NSCURL_POSTDATA_T ರಚನೆ curl_httppost
#ವ್ಯಾಖ್ಯಾನಿಸು NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#ವ್ಯಾಖ್ಯಾನಿಸು NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#ಆದರೆ_ಅಂತ್ಯ

/** Information for a single fetch. */
ರಚನೆ curl_fetch_info {
	ರಚನೆ fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool ಕಳುಹಿಸಿದ_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_ಶೀರ್ಷಿಕೆಗಳು;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	ರಚನೆ curl_slist *ಶೀರ್ಷಿಕೆಗಳು;	/**< List of request headers. */
	ಅಕ್ಷರ *location;		/**< Response Location header, or 0. */
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ content_ಉದ್ದಳತೆ;	/**< Response Content-Length, or 0. */
	ಅಕ್ಷರ *cookie_string;	/**< Cookie string for this fetch */
	ಅಕ್ಷರ *realm;		/**< HTTP Auth Realm */
	ರಚನೆ fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	ದೀರ್ಘ http_ಸಂಕೇತ; /**< HTTP result code from cURL. */

	uint64_t ಕೊನೆಯ_ಪ್ರಗತಿ_update;	/**< Time of last progress update */
	ಪೂರ್ಣಾಂಕ cert_ಆಳ; /**< deepest certificate in use */
	ರಚನೆ cert_info cert_ದತ್ತಾಂಶ[MAX_CERT_ಆಳ]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
ರಚನೆ cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	ರಚನೆ cache_handle *r_prev; /**< Previous cached handle in ring. */
	ರಚನೆ cache_handle *r_ಮುಂದಿನ; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
ಸ್ಥಿರ CURL *fetch_blank_curl;

/** Ring of cached handles */
ಸ್ಥಿರ ರಚನೆ cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
ಸ್ಥಿರ bool curl_with_openssl;

/** Error buffer for cURL. */
ಸ್ಥಿರ ಅಕ್ಷರ fetch_error_ಬಫರ್[CURL_ERROR_ಗಾತ್ರ];

/** Proxy authentication details. */
ಸ್ಥಿರ ಅಕ್ಷರ fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
ಸ್ಥಿರ bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
ಸ್ಥಿರ bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_ದತ್ತಾಂಶ(scheme));
	curl_fetchers_registered++;
	ಹಿಂತಿರುಗಿ true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_finalise(lwc_string *scheme)
{
	ರಚನೆ cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_ದತ್ತಾಂಶ(scheme));
	ಆದರೆ (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		ಆದರೆ (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = ಖಾಲಿ;
	}

	/* Free anything remaining in the cached curl handle ring */
	ಆಗ (curl_handle_ring != ಖಾಲಿ) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
		curl_easy_cleanup(h->handle);
		ಬಿಡುಗಡೆ(h);
	}
}


/**
 * Check if this fetcher can fetch a url.
 *
 * \param url The url to check.
 * \return true if the fetcher supports the url else false.
 */
ಸ್ಥಿರ bool fetch_curl_can_fetch(ಸ್ಥಿರಾಂಕ nsurl *url)
{
	ಹಿಂತಿರುಗಿ nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
ಸ್ಥಿರ ರಚನೆ fetch_postdata *
fetch_curl_alloc_postdata(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *post_urlenc,
			  ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *post_multipart)
{
	ರಚನೆ fetch_postdata *postdata;
	postdata = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ fetch_postdata));
	ಆದರೆ (postdata != ಖಾಲಿ) {

		ಆದರೆ (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->ದತ್ತಾಂಶ.urlenc = strdup(post_urlenc);
			ಆದರೆ (postdata->ದತ್ತಾಂಶ.urlenc == ಖಾಲಿ) {
				ಬಿಡುಗಡೆ(postdata);
				postdata = ಖಾಲಿ;
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->ದತ್ತಾಂಶ.multipart = fetch_multipart_ದತ್ತಾಂಶ_clone(post_multipart);
			ಆದರೆ (postdata->ದತ್ತಾಂಶ.multipart == ಖಾಲಿ) {
				ಬಿಡುಗಡೆ(postdata);
				postdata = ಖಾಲಿ;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	ಹಿಂತಿರುಗಿ postdata;
}

/**
 * free postdata
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_free_postdata(ರಚನೆ fetch_postdata *postdata)
{
	ಆದರೆ (postdata != ಖಾಲಿ) {
		ಆಯ್ಕೆ (postdata->type) {
		ಸ್ಥಿತಿ FETCH_POSTDATA_NONE:
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ FETCH_POSTDATA_URLENC:
			ಬಿಡುಗಡೆ(postdata->ದತ್ತಾಂಶ.urlenc);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ FETCH_POSTDATA_MULTIPART:
			fetch_multipart_ದತ್ತಾಂಶ_destroy(postdata->ದತ್ತಾಂಶ.multipart);
			ನಿಲ್ಲಿಸು;
		}

		ಬಿಡುಗಡೆ(postdata);
	}
}

/**
 *construct a new fetch structure
 */
ಸ್ಥಿರ ರಚನೆ curl_fetch_info *fetch_alloc(ಶೂನ್ಯ)
{
	ರಚನೆ curl_fetch_info *fetch;
	fetch = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ (*fetch));
	ಆದರೆ (fetch == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	fetch->curl_handle = ಖಾಲಿ;
	fetch->ಕಳುಹಿಸಿದ_ssl_chain = false;
	fetch->had_ಶೀರ್ಷಿಕೆಗಳು = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->ಶೀರ್ಷಿಕೆಗಳು = ಖಾಲಿ;
	fetch->url = ಖಾಲಿ;
	fetch->host = ಖಾಲಿ;
	fetch->location = ಖಾಲಿ;
	fetch->content_ಉದ್ದಳತೆ = 0;
	fetch->http_ಸಂಕೇತ = 0;
	fetch->cookie_string = ಖಾಲಿ;
	fetch->realm = ಖಾಲಿ;
	fetch->ಕೊನೆಯ_ಪ್ರಗತಿ_update = 0;
	fetch->postdata = ಖಾಲಿ;
	fetch->curl_postdata = ಖಾಲಿ;

	/* Clear certificate chain data */
	memset(fetch->cert_ದತ್ತಾಂಶ, 0, ಗಾತ್ರ(fetch->cert_ದತ್ತಾಂಶ));
	fetch->cert_ಆಳ = -1;

	ಹಿಂತಿರುಗಿ fetch;
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
fetch_curl_ಸಿದ್ಧತೆ(ರಚನೆ fetch *ಪೋಷಕ_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *post_urlenc,
		 ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *post_multipart,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಶೀರ್ಷಿಕೆಗಳು)
{
	ರಚನೆ curl_fetch_info *fetch;
	ರಚನೆ curl_slist *slist;
	ಪೂರ್ಣಾಂಕ i;

	fetch = fetch_alloc();
	ಆದರೆ (fetch == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = ಪೋಷಕ_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	ಆದರೆ (fetch->host == ಖಾಲಿ) {
		ಹೋಗು failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	ಆದರೆ (fetch->postdata == ಖಾಲಿ) {
		ಹೋಗು failed;
	}

#ವ್ಯಾಖ್ಯಾನಿಸು APPEND(list, ಮೌಲ್ಯ) \
	slist = curl_slist_append(list, ಮೌಲ್ಯ);		\
	ಆದರೆ (slist == ಖಾಲಿ)				\
		ಹೋಗು failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->ಶೀರ್ಷಿಕೆಗಳು, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->ಶೀರ್ಷಿಕೆಗಳು, "Expect:");

	ಆದರೆ ((nsoption_charp(accept_language) != ಖಾಲಿ) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		ಅಕ್ಷರ s[80];
		snprintf(s, ಗಾತ್ರ s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[ಗಾತ್ರ s - 1] = 0;
		APPEND(fetch->ಶೀರ್ಷಿಕೆಗಳು, s);
	}

	ಆದರೆ (nsoption_charp(accept_charset) != ಖಾಲಿ &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		ಅಕ್ಷರ s[80];
		snprintf(s, ಗಾತ್ರ s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[ಗಾತ್ರ s - 1] = 0;
		APPEND(fetch->ಶೀರ್ಷಿಕೆಗಳು, s);
	}

	ಆದರೆ (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->ಶೀರ್ಷಿಕೆಗಳು, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	ಗಾಗಿ (i = 0; ಶೀರ್ಷಿಕೆಗಳು[i] != ಖಾಲಿ; i++) {
		APPEND(fetch->ಶೀರ್ಷಿಕೆಗಳು, ಶೀರ್ಷಿಕೆಗಳು[i]);
	}

	ಹಿಂತಿರುಗಿ fetch;

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->ಶೀರ್ಷಿಕೆಗಳು);
	ಬಿಡುಗಡೆ(fetch);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
ಸ್ಥಿರ ರಚನೆ cert_chain *
fetch_curl_get_cached_chain(ರಚನೆ curl_fetch_info *f)
{
	ರಚನೆ cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	ಆದರೆ (chain == ಖಾಲಿ) {
		chain = hashmap_ಸೇರಿಸು(curl_fetch_ssl_hashmap, f->url);
	}

	ಹಿಂತಿರುಗಿ chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
ಸ್ಥಿರ ಶೂನ್ಯ
fetch_curl_store_certs_ಒಳ_cache(ರಚನೆ curl_fetch_info *f)
{
	size_t ಆಳ;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_ಆಳ];
	ರಚನೆ cert_chain chain, *cached_chain;
	ರಚನೆ cert_info *certs;

	memset(&chain, 0, ಗಾತ್ರ(chain));

	certs = f->cert_ದತ್ತಾಂಶ;
	chain.ಆಳ = f->cert_ಆಳ + 1; /* 0 indexed certificate depth */

	ಗಾಗಿ (ಆಳ = 0; ಆಳ < chain.ಆಳ; ಆಳ++) {
		ಆದರೆ (certs[ಆಳ].cert == ಖಾಲಿ) {
			/* This certificate is missing, skip it */
			chain.certs[ಆಳ].err = SSL_CERT_ERR_CERT_MISSING;
			ಮುಂದುವರಿ;
		}

		/* error code (if any) */
		ಆಯ್ಕೆ (certs[ಆಳ].err) {
		ಸ್ಥಿತಿ X509_V_OK:
			chain.certs[ಆಳ].err = SSL_CERT_ERR_OK;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ X509_V_ERR_UNABLE_ಗಮ್ಯ_GET_ISSUER_CERT:
			/* fallthrough */
		ಸ್ಥಿತಿ X509_V_ERR_UNABLE_ಗಮ್ಯ_DECODE_ISSUER_ಬಹಿರಂಗ_ಕೀಲಿ:
			chain.certs[ಆಳ].err = SSL_CERT_ERR_BAD_ISSUER;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ X509_V_ERR_UNABLE_ಗಮ್ಯ_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		ಸ್ಥಿತಿ X509_V_ERR_UNABLE_ಗಮ್ಯ_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		ಸ್ಥಿತಿ X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		ಸ್ಥಿತಿ X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[ಆಳ].err = SSL_CERT_ERR_BAD_SIG;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		ಸ್ಥಿತಿ X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[ಆಳ].err = SSL_CERT_ERR_TOO_YOUNG;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		ಸ್ಥಿತಿ X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[ಆಳ].err = SSL_CERT_ERR_TOO_OLD;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ X509_V_ERR_ಆಳ_ZERO_SELF_SIGNED_CERT:
			chain.certs[ಆಳ].err = SSL_CERT_ERR_SELF_SIGNED;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ X509_V_ERR_SELF_SIGNED_CERT_ಒಳ_CHAIN:
			chain.certs[ಆಳ].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ X509_V_ERR_CERT_REVOKED:
			chain.certs[ಆಳ].err = SSL_CERT_ERR_REVOKED;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[ಆಳ].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			ನಿಲ್ಲಿಸು;

		ಪೂರ್ವನಿಯೋಜಿತ:
			chain.certs[ಆಳ].err = SSL_CERT_ERR_UNKNOWN;
			ನಿಲ್ಲಿಸು;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[ಆಳ].cert);
		BIO_get_mem_ptr(mem, &buf[ಆಳ]);
		(ಶೂನ್ಯ) BIO_ಗಣ_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[ಆಳ].der = (uint8_t *)buf[ಆಳ]->ದತ್ತಾಂಶ;
		chain.certs[ಆಳ].der_ಉದ್ದಳತೆ = buf[ಆಳ]->ಉದ್ದಳತೆ;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	ಆದರೆ (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	ಗಾಗಿ (ಆಳ = 0; ಆಳ < chain.ಆಳ; ಆಳ++) {
		ಆದರೆ (chain.certs[ಆಳ].err == SSL_CERT_ERR_CERT_MISSING) {
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (buf[ಆಳ] != ಖಾಲಿ) {
			BUF_MEM_free(buf[ಆಳ]);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
fetch_curl_verify_callback(ಪೂರ್ಣಾಂಕ verify_ok, X509_STORE_CTX *x509_ctx)
{
	ಪೂರ್ಣಾಂಕ ಆಳ;
	ರಚನೆ curl_fetch_info *fetch;

	ಆಳ = X509_STORE_CTX_get_error_ಆಳ(x509_ctx);
	fetch = X509_STORE_CTX_get_app_ದತ್ತಾಂಶ(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	ಆದರೆ (ಆಳ >= MAX_CERT_ಆಳ) {
		X509_STORE_CTX_ಗಣ_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		ಹಿಂತಿರುಗಿ 0;
	}

	/* record the max depth */
	ಆದರೆ (ಆಳ > fetch->cert_ಆಳ) {
		fetch->cert_ಆಳ = ಆಳ;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	ಆದರೆ (!fetch->cert_ದತ್ತಾಂಶ[ಆಳ].cert) {
		fetch->cert_ದತ್ತಾಂಶ[ಆಳ].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_ಮೇಲೆ_ref(fetch->cert_ದತ್ತಾಂಶ[ಆಳ].cert);
		fetch->cert_ದತ್ತಾಂಶ[ಆಳ].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	ಆದರೆ (ಆಳ > 0) {
		verify_ok = 1;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* search for deeper certificates in the chain with errors */
		ಗಾಗಿ (ಆಳ = fetch->cert_ಆಳ; ಆಳ > 0; ಆಳ--) {
			ಆದರೆ (fetch->cert_ದತ್ತಾಂಶ[ಆಳ].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_ಗಣ_error(x509_ctx, fetch->cert_ದತ್ತಾಂಶ[ಆಳ].err);
			}
		}
	}

	ಹಿಂತಿರುಗಿ verify_ok;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, ಶೂನ್ಯ *parm)
{
	ರಚನೆ curl_fetch_info *f = (ರಚನೆ curl_fetch_info *) parm;
	ಪೂರ್ಣಾಂಕ ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_ಗಣ_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_ದತ್ತಾಂಶ(f->host),
					 lwc_string_ಉದ್ದಳತೆ(f->host));

	/* Store fetch struct in context for verify callback */
	ಆದರೆ (ok) {
		ok = X509_STORE_CTX_ಗಣ_app_ದತ್ತಾಂಶ(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	ಆದರೆ (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_ಒಳ_cache(f);

	ಹಿಂತಿರುಗಿ ok;
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
	ರಚನೆ curl_fetch_info *f = (ರಚನೆ curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	ದೀರ್ಘ options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_ಗಣ_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_ಗಣ_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	ಆದರೆ (f->downgrade_tls) {
		/* Disable TLS 1.3 if the server can't cope with it */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SSL_OP_NO_TLSv1_3
		options |= SSL_OP_NO_TLSv1_3;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SSL_ಬಹುಲಕ_SEND_FALLBACK_SCSV
		/* Ensure server rejects the connection if downgraded too far */
		SSL_CTX_ಗಣ_ಬಹುಲಕ(sslctx, SSL_ಬಹುಲಕ_SEND_FALLBACK_SCSV);
#ಆದರೆ_ಅಂತ್ಯ
	}

	SSL_CTX_ಗಣ_options(sslctx, options);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SSL_OP_NO_TICKET
	SSL_CTX_clear_options(sslctx, SSL_OP_NO_TICKET);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ CURLE_OK;
}


#ಆದರೆ_ಅಂತ್ಯ /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
ಸ್ಥಿರ ಶೂನ್ಯ
fetch_curl_report_certs_upstream(ರಚನೆ curl_fetch_info *f)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ರಚನೆ cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	ಆದರೆ (chain != ಖಾಲಿ) {
		ಸಂದೇಶ.type = FETCH_CERTS;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.chain = chain;

		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
	}

	f->ಕಳುಹಿಸಿದ_ssl_chain = true;
}

#ಆದರೆ LIBCURL_ಆವೃತ್ತಿ_ಅಂಕೆ >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
ರಚನೆ curl_mime_ctx {
	ಅಕ್ಷರ *ಬಫರ್;
	curl_off_t ಗಾತ್ರ_2;
	curl_off_t position;
};

ಸ್ಥಿರ size_t mime_ದತ್ತಾಂಶ_read_callback(ಅಕ್ಷರ *ಬಫರ್, size_t ಗಾತ್ರ_2, size_t nitems, ಶೂನ್ಯ *arg)
{
	ರಚನೆ curl_mime_ctx *mctx = (ರಚನೆ curl_mime_ctx *) arg;
	curl_off_t sz = mctx->ಗಾತ್ರ_2 - mctx->position;

	nitems *= ಗಾತ್ರ_2;
	ಆದರೆ(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	ಆದರೆ(sz) {
		ಸ್ಮರಣೆ_ನಕಲು(ಬಫರ್, mctx->ಬಫರ್ + mctx->position, sz);
	}
	mctx->position += sz;
	ಹಿಂತಿರುಗಿ sz;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mime_ದತ್ತಾಂಶ_seek_callback(ಶೂನ್ಯ *arg, curl_off_t ಸ್ಥಾನಾಂತರ, ಪೂರ್ಣಾಂಕ origin)
{
	ರಚನೆ curl_mime_ctx *mctx = (ರಚನೆ curl_mime_ctx *) arg;

	ಆಯ್ಕೆ(origin) {
	ಸ್ಥಿತಿ SEEK_END:
		ಸ್ಥಾನಾಂತರ += mctx->ಗಾತ್ರ_2;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ SEEK_CUR:
		ಸ್ಥಾನಾಂತರ += mctx->position;
		ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ(ಸ್ಥಾನಾಂತರ < 0) {
		ಹಿಂತಿರುಗಿ CURL_SEEKFUNC_FAIL;
	}
	mctx->position = ಸ್ಥಾನಾಂತರ;
	ಹಿಂತಿರುಗಿ CURL_SEEKFUNC_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ mime_ದತ್ತಾಂಶ_free_callback(ಶೂನ್ಯ *arg)
{
	ರಚನೆ curl_mime_ctx *mctx = (ರಚನೆ curl_mime_ctx *) arg;
	ಬಿಡುಗಡೆ(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
ಸ್ಥಿರ curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode ಸಂಕೇತ_2 = CURLE_OK;
	size_t ಮೌಲ್ಯ_len;

	cmime = curl_mime_init(chandle);
	ಆದರೆ (cmime == ಖಾಲಿ) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* iterate post data */
	ಗಾಗಿ (; multipart != ಖಾಲಿ; multipart = multipart->ಮುಂದಿನ) {
		part = curl_mime_addpart(cmime);
		ಆದರೆ (part == ಖಾಲಿ) {
			ಹೋಗು convert_failed;
		}

		ಸಂಕೇತ_2 = curl_mime_name(part, multipart->name);
		ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK) {
			ಹೋಗು convert_failed;
		}

		ಮೌಲ್ಯ_len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(multipart->ಮೌಲ್ಯ);

		ಆದರೆ (multipart->file && ಮೌಲ್ಯ_len==0) {
			/* file entries with no filename require special handling */
			ಸಂಕೇತ_2=curl_mime_ದತ್ತಾಂಶ(part, multipart->ಮೌಲ್ಯ, ಮೌಲ್ಯ_len);
			ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK) {
				ಹೋಗು convert_failed;
			}

			ಸಂಕೇತ_2 = curl_mime_filename(part, "");
			ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK) {
				ಹೋಗು convert_failed;
			}

			ಸಂಕೇತ_2 = curl_mime_type(part, "application/octet-stream");
			ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK) {
				ಹೋಗು convert_failed;
			}

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(multipart->file) {
			/* file entry */
			nserror ret;
			ಅಕ್ಷರ *leafname = ಖಾಲಿ;
			ಅಕ್ಷರ *mimetype = ಖಾಲಿ;

			ಸಂಕೇತ_2 = curl_mime_filedata(part, multipart->rawfile);
			ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK) {
				ಹೋಗು convert_failed;
			}

			ret = guit->file->basename(multipart->ಮೌಲ್ಯ, &leafname, ಖಾಲಿ);
			ಆದರೆ (ret != NSERROR_OK) {
				ಹೋಗು convert_failed;
			}
			ಸಂಕೇತ_2 = curl_mime_filename(part, leafname);
			ಬಿಡುಗಡೆ(leafname);
			ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK) {
				ಹೋಗು convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->ಮೌಲ್ಯ);
			ಆದರೆ (mimetype == ಖಾಲಿ) {
				mimetype=strdup("text/plain");
			}
			ಆದರೆ (mimetype == ಖಾಲಿ) {
				ಹೋಗು convert_failed;
			}
			ಸಂಕೇತ_2 = curl_mime_type(part, mimetype);
			ಬಿಡುಗಡೆ(mimetype);
			ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK) {
				ಹೋಗು convert_failed;
			}

		} ಇಲ್ಲದಿದ್ದರೆ {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			ರಚನೆ curl_mime_ctx *cb_ctx;
			cb_ctx = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ curl_mime_ctx));
			ಆದರೆ (cb_ctx == ಖಾಲಿ) {
				ಹೋಗು convert_failed;
			}
			cb_ctx->ಬಫರ್ = multipart->ಮೌಲ್ಯ;
			cb_ctx->ಗಾತ್ರ_2 = ಮೌಲ್ಯ_len;
			cb_ctx->position = 0;
			ಸಂಕೇತ_2 = curl_mime_ದತ್ತಾಂಶ_cb(part,
						 ಮೌಲ್ಯ_len,
						 mime_ದತ್ತಾಂಶ_read_callback,
						 mime_ದತ್ತಾಂಶ_seek_callback,
						 mime_ದತ್ತಾಂಶ_free_callback,
						 cb_ctx);
			ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK) {
				ಬಿಡುಗಡೆ(cb_ctx);
				ಹೋಗು convert_failed;
			}
		}
	}

	ಹಿಂತಿರುಗಿ cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", ಸಂಕೇತ_2);
	curl_mime_free(cmime);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

#ಇಲ್ಲದಿದ್ದರೆ /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
ಸ್ಥಿರ ರಚನೆ curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *control)
{
	ರಚನೆ curl_httppost *post = ಖಾಲಿ, *ಕೊನೆಯ = ಖಾಲಿ;
	CURLFORMcode ಸಂಕೇತ_2;
	nserror ret;

	ಗಾಗಿ (; control; control = control->ಮುಂದಿನ) {
		ಆದರೆ (control->file) {
			ಅಕ್ಷರ *leafname = ಖಾಲಿ;
			ret = guit->file->basename(control->ಮೌಲ್ಯ, &leafname, ಖಾಲಿ);
			ಆದರೆ (ret != NSERROR_OK) {
				ಮುಂದುವರಿ;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			ಆದರೆ (control->ಮೌಲ್ಯ[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				ಸ್ಥಿರ ಅಕ್ಷರ buf;

				ಸಂಕೇತ_2 = curl_formadd(&post, &ಕೊನೆಯ,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_ಬಫರ್, control->ಮೌಲ್ಯ,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_ಕೊನೆ);
				ಆದರೆ (ಸಂಕೇತ_2 != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", ಸಂಕೇತ_2,
					      control->name);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಅಕ್ಷರ *mimetype = guit->fetch->mimetype(control->ಮೌಲ್ಯ);
				ಸಂಕೇತ_2 = curl_formadd(&post, &ಕೊನೆಯ,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_ಕೊನೆ);
				ಆದರೆ (ಸಂಕೇತ_2 != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      ಸಂಕೇತ_2,
					      control->name,
					      control->ಮೌಲ್ಯ);
				ಬಿಡುಗಡೆ(mimetype);
			}
			ಬಿಡುಗಡೆ(leafname);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಸಂಕೇತ_2 = curl_formadd(&post, &ಕೊನೆಯ,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->ಮೌಲ್ಯ,
					    CURLFORM_ಕೊನೆ);
			ಆದರೆ (ಸಂಕೇತ_2 != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", ಸಂಕೇತ_2,
				      control->name, control->ಮೌಲ್ಯ);
		}
	}

	ಹಿಂತಿರುಗಿ post;
}

#ಆದರೆ_ಅಂತ್ಯ  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
ಸ್ಥಿರ CURLcode fetch_curl_ಗಣ_postdata(ರಚನೆ curl_fetch_info *f)
{
	CURLcode ಸಂಕೇತ_2 = CURLE_OK;

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SETOPT
#ವ್ಯಾಖ್ಯಾನಿಸು SETOPT(option, ಮೌಲ್ಯ) { \
	ಸಂಕೇತ_2 = curl_easy_setopt(f->curl_handle, option, ಮೌಲ್ಯ);	\
	ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK)					\
		ಹಿಂತಿರುಗಿ ಸಂಕೇತ_2;					\
	}

	ಆಯ್ಕೆ (f->postdata->type) {
	ಸ್ಥಿತಿ FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, ಖಾಲಿ);
		SETOPT(NSCURL_POSTDATA_CURLOPT, ಖಾಲಿ);
		SETOPT(CURLOPT_HTTPGET, 1L);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, ಖಾಲಿ);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->ದತ್ತಾಂಶ.urlenc);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, ಖಾಲಿ);
		SETOPT(CURLOPT_HTTPGET, 0L);
		ಆದರೆ (f->curl_postdata == ಖಾಲಿ) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->ದತ್ತಾಂಶ.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ ಸಂಕೇತ_2;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
ಸ್ಥಿರ CURLcode fetch_curl_ಗಣ_options(ರಚನೆ curl_fetch_info *f)
{
	CURLcode ಸಂಕೇತ_2;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *auth;

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SETOPT
#ವ್ಯಾಖ್ಯಾನಿಸು SETOPT(option, ಮೌಲ್ಯ) { \
	ಸಂಕೇತ_2 = curl_easy_setopt(f->curl_handle, option, ಮೌಲ್ಯ);	\
	ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK)					\
		ಹಿಂತಿರುಗಿ ಸಂಕೇತ_2;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_ಖಾಸಗಿ, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_ಪ್ರಗತಿ_ದತ್ತಾಂಶ, f);
	SETOPT(CURLOPT_HTTPHEADER, f->ಶೀರ್ಷಿಕೆಗಳು);
	ಸಂಕೇತ_2 = fetch_curl_ಗಣ_postdata(f);
	ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK) {
		ಹಿಂತಿರುಗಿ ಸಂಕೇತ_2;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	ಆದರೆ (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} ಇಲ್ಲದಿದ್ದರೆ {
		SETOPT(CURLOPT_COOKIE, ಖಾಲಿ);
	}

	ಆದರೆ ((auth = urldb_get_auth_details(f->url, ಖಾಲಿ)) != ಖಾಲಿ) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} ಇಲ್ಲದಿದ್ದರೆ {
		SETOPT(CURLOPT_USERPWD, ಖಾಲಿ);
	}

	/* set up proxy options */
	ಆದರೆ (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != ಖಾಲಿ) &&
	    (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (ದೀರ್ಘ) nsoption_int(http_proxy_ಪೋರ್ಟ್));

#ಆದರೆ LIBCURL_ಆವೃತ್ತಿ_ಅಂಕೆ >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#ಆದರೆ_ಅಂತ್ಯ

		ಆದರೆ (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(ದೀರ್ಘ) CURLAUTH_BASIC :
					(ದೀರ್ಘ) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					ಗಾತ್ರ fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		SETOPT(CURLOPT_PROXY, ಖಾಲಿ);
	}


	ಆದರೆ (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	ಆದರೆ (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		ಆದರೆ (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, ಖಾಲಿ);
			SETOPT(CURLOPT_SSL_CTX_ದತ್ತಾಂಶ, ಖಾಲಿ);
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WITH_OPENSSL
		ಆದರೆ (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_ದತ್ತಾಂಶ, f);
		}
#ಆದರೆ_ಅಂತ್ಯ
	}

	ಹಿಂತಿರುಗಿ CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
ಸ್ಥಿರ bool
fetch_curl_initiate_fetch(ರಚನೆ curl_fetch_info *fetch, CURL *handle)
{
	CURLcode ಸಂಕೇತ_2;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	ಸಂಕೇತ_2 = fetch_curl_ಗಣ_options(fetch);
	ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		ಹಿಂತಿರುಗಿ false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_ಕೂಡಿಸು_handle(fetch_curl_multi, fetch->curl_handle);
	ಪ್ರತಿಜ್ಞಾ(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
ಸ್ಥಿರ CURL *fetch_curl_get_handle(lwc_string *host)
{
	ರಚನೆ cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	ಆದರೆ (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		ಬಿಡುಗಡೆ(h);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	ಹಿಂತಿರುಗಿ ret;
}


/**
 * Dispatch a single job
 */
ಸ್ಥಿರ bool fetch_curl_ಆರಂಭ(ಶೂನ್ಯ *vfetch)
{
	ರಚನೆ curl_fetch_info *fetch = (ರಚನೆ curl_fetch_info*)vfetch;
	ಆದರೆ (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		ಹಿಂತಿರುಗಿ false;
	}
	ಹಿಂತಿರುಗಿ fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#ಆದರೆ LIBCURL_ಆವೃತ್ತಿ_ಅಂಕೆ >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	ಹಿಂತಿರುಗಿ;
#ಇಲ್ಲದಿದ್ದರೆ
	ರಚನೆ cache_handle *h = 0;
	ಪೂರ್ಣಾಂಕ c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	ಆದರೆ (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		ಹಿಂತಿರುಗಿ;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(ರಚನೆ cache_handle, curl_handle_ring, c);
	ಆದರೆ (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		ಆದರೆ (curl_handle_ring != ಖಾಲಿ) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_ಮುಂದಿನ;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		ಹಿಂತಿರುಗಿ;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (ರಚನೆ cache_handle*)ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_ಸೇರಿಸು(curl_handle_ring, h);
#ಆದರೆ_ಅಂತ್ಯ
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_stop(ರಚನೆ curl_fetch_info *f)
{
	CURLMcode codem;

	ಪ್ರತಿಜ್ಞಾ(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	ಆದರೆ (f->curl_handle) {
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
	ರಚನೆ curl_fetch_info *f = (ರಚನೆ curl_fetch_info *)vf;
	ಪ್ರತಿಜ್ಞಾ(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	ಆದರೆ (f->curl_handle) {
		ಆದರೆ (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->abort = true;
		} ಇಲ್ಲದಿದ್ದರೆ {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_free(ಶೂನ್ಯ *vf)
{
	ರಚನೆ curl_fetch_info *f = (ರಚನೆ curl_fetch_info *)vf;
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	ಬಿಡುಗಡೆ(f->location);
	ಬಿಡುಗಡೆ(f->cookie_string);
	ಬಿಡುಗಡೆ(f->realm);
	ಆದರೆ (f->ಶೀರ್ಷಿಕೆಗಳು) {
		curl_slist_free_all(f->ಶೀರ್ಷಿಕೆಗಳು);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	ಗಾಗಿ (i = 0; i < MAX_CERT_ಆಳ; i++) {
		ಆದರೆ (f->cert_ದತ್ತಾಂಶ[i].cert != ಖಾಲಿ) {
			X509_free(f->cert_ದತ್ತಾಂಶ[i].cert);
		}
	}

	ಬಿಡುಗಡೆ(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
ಸ್ಥಿರ bool fetch_curl_process_ಶೀರ್ಷಿಕೆಗಳು(ರಚನೆ curl_fetch_info *f)
{
	ದೀರ್ಘ http_ಸಂಕೇತ;
	CURLcode ಸಂಕೇತ_2;
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	f->had_ಶೀರ್ಷಿಕೆಗಳು = true;

	ಆದರೆ (!f->http_ಸಂಕೇತ) {
		ಸಂಕೇತ_2 = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_ಸಂಕೇತ,
					 &f->http_ಸಂಕೇತ);
		ಪ್ರತಿಜ್ಞಾ(ಸಂಕೇತ_2 == CURLE_OK);
		fetch_ಗಣ_http_ಸಂಕೇತ(f->fetch_handle, f->http_ಸಂಕೇತ);
	}
	http_ಸಂಕೇತ = f->http_ಸಂಕೇತ;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_ಸಂಕೇತ);

	ಆದರೆ ((http_ಸಂಕೇತ == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		ಸಂದೇಶ.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
		ಹಿಂತಿರುಗಿ true;
	}

	/* handle HTTP redirects (3xx response codes) */
	ಆದರೆ (HTTP_RESPONSE_IS_3XX(http_ಸಂಕೇತ) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		ಸಂದೇಶ.type = FETCH_REDIRECT;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.redirect = f->location;
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
		ಹಿಂತಿರುಗಿ true;
	}

	/* handle HTTP 401 (Authentication errors) */
	ಆದರೆ (http_ಸಂಕೇತ == HTTP_RESPONSE_UNAUTHORIZED) {
		ಸಂದೇಶ.type = FETCH_AUTH;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.auth.realm = f->realm;
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
		ಹಿಂತಿರುಗಿ true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	ಆದರೆ (f->only_2xx &&
	    ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_ಸಂಕೇತ) == false)) {
		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.error = messages_get("Not2xx");
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
		ಹಿಂತಿರುಗಿ true;
	}

	ಆದರೆ (f->abort)
		ಹಿಂತಿರುಗಿ true;

	ಹಿಂತಿರುಗಿ false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_curl_ಮುಗಿದ(CURL *curl_handle, CURLcode ಫಲಿತಾಂಶ)
{
	bool ಪೂರ್ಣಗೊಂಡ = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	ರಚನೆ curl_fetch_info *f;
	ಅಕ್ಷರ **_hideous_hack = (ಅಕ್ಷರ **) (ಶೂನ್ಯ *) &f;
	CURLcode ಸಂಕೇತ_2;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	ಸಂಕೇತ_2 = curl_easy_getinfo(curl_handle, CURLINFO_ಖಾಸಗಿ, _hideous_hack);
	ಪ್ರತಿಜ್ಞಾ(ಸಂಕೇತ_2 == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	ಆದರೆ ((abort_fetch == false) &&
	    (ಫಲಿತಾಂಶ == CURLE_OK ||
	     ((ಫಲಿತಾಂಶ == CURLE_WRITE_ERROR) && (f->stopped == false)))) {
		/* fetch completed normally or the server fed us a junk gzip
		 * stream (usually in the form of garbage at the end of the
		 * stream). Curl will have fed us all but the last chunk of
		 * decoded data, which is sad as, if we'd received the last
		 * chunk, too, we'd be able to render the whole object.
		 * As is, we'll just have to accept that the end of the
		 * object will be truncated in this case and leave it to
		 * the content handlers to cope.
		 */
		ಆದರೆ (f->stopped ||
		    (!f->had_ಶೀರ್ಷಿಕೆಗಳು &&	fetch_curl_process_ಶೀರ್ಷಿಕೆಗಳು(f))) {
			; /* redirect with no body or similar */
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಪೂರ್ಣಗೊಂಡ = true;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಫಲಿತಾಂಶ == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		ಆದರೆ (!f->had_ಶೀರ್ಷಿಕೆಗಳು && fetch_curl_process_ಶೀರ್ಷಿಕೆಗಳು(f))
			; /* redirect with partial body, or similar */
		ಇಲ್ಲದಿದ್ದರೆ {
			ಪೂರ್ಣಗೊಂಡ = true;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಫಲಿತಾಂಶ == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಫಲಿತಾಂಶ == CURLE_SSL_PEER_CERTIFICATE ||
		   ಫಲಿತಾಂಶ == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} ಇಲ್ಲದಿದ್ದರೆ {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", ಫಲಿತಾಂಶ);
		error = true;
	}

	fetch_curl_stop(f);

	ಆದರೆ (f->ಕಳುಹಿಸಿದ_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	ಆದರೆ (abort_fetch) {
		; /* fetch was aborted: no callback */
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಪೂರ್ಣಗೊಂಡ) {
		fetch_ಸಂದೇಶ ಸಂದೇಶ;
		ಸಂದೇಶ.type = FETCH_ಪೂರ್ಣಗೊಂಡ;
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cert) {
		/* user needs to validate certificate with issue */
		fetch_ಸಂದೇಶ ಸಂದೇಶ;
		ಸಂದೇಶ.type = FETCH_CERT_ERR;
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (error) {
		fetch_ಸಂದೇಶ ಸಂದೇಶ;
		ಆಯ್ಕೆ (ಫಲಿತಾಂಶ) {
		ಸ್ಥಿತಿ CURLE_SSL_CONNECT_ERROR:
			ಸಂದೇಶ.type = FETCH_SSL_ERR;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ CURLE_OPERATION_TIMEDOUT:
			ಸಂದೇಶ.type = FETCH_TIMEDOUT;
			ಸಂದೇಶ.ದತ್ತಾಂಶ.error = curl_easy_strerror(ಫಲಿತಾಂಶ);
			ನಿಲ್ಲಿಸು;

		ಪೂರ್ವನಿಯೋಜಿತ:
			ಸಂದೇಶ.type = FETCH_ERROR;
			ಸಂದೇಶ.ದತ್ತಾಂಶ.error = curl_easy_strerror(ಫಲಿತಾಂಶ);
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
	ಪೂರ್ಣಾಂಕ running, ಸರತಿ;
	CURLMcode codem;
	CURLMsg *curl_ಸಂದೇಶ;

	ಆದರೆ (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_ಗಣ, write_fd_ಗಣ, exc_fd_ಗಣ;
		ಪೂರ್ಣಾಂಕ max_fd = -1;
		ಪೂರ್ಣಾಂಕ i;

		FD_ZERO(&read_fd_ಗಣ);
		FD_ZERO(&write_fd_ಗಣ);
		FD_ZERO(&exc_fd_ಗಣ);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_ಗಣ, &write_fd_ಗಣ,
				&exc_fd_ಗಣ, &max_fd);
		ಪ್ರತಿಜ್ಞಾ(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		ಗಾಗಿ (i = 0; i <= max_fd; i++) {
			bool ಪಠ = false;
			bool ಲಿಖ = false;
			bool error = false;

			ಆದರೆ (FD_ISSET(i, &read_fd_ಗಣ)) {
				ಪಠ = true;
			}
			ಆದರೆ (FD_ISSET(i, &write_fd_ಗಣ)) {
				ಲಿಖ = true;
			}
			ಆದರೆ (FD_ISSET(i, &exc_fd_ಗಣ)) {
				error = true;
			}
			ಆದರೆ (ಪಠ || ಲಿಖ || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      ಪಠ ? "read" : "    ",
				      ಲಿಖ ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	ಮಾಡು {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		ಆದರೆ (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			ಹಿಂತಿರುಗಿ;
		}
	} ಆಗ (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_ಸಂದೇಶ = curl_multi_info_read(fetch_curl_multi, &ಸರತಿ);
	ಆಗ (curl_ಸಂದೇಶ) {
		ಆಯ್ಕೆ (curl_ಸಂದೇಶ->ಸಂದೇಶ) {
			ಸ್ಥಿತಿ CURLMSG_ಮುಗಿದ:
				fetch_curl_ಮುಗಿದ(curl_ಸಂದೇಶ->easy_handle,
						curl_ಸಂದೇಶ->ದತ್ತಾಂಶ.ಫಲಿತಾಂಶ);
				ನಿಲ್ಲಿಸು;
			ಪೂರ್ವನಿಯೋಜಿತ:
				ನಿಲ್ಲಿಸು;
		}
		curl_ಸಂದೇಶ = curl_multi_info_read(fetch_curl_multi, &ಸರತಿ);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
fetch_curl_ಪ್ರಗತಿ(ಶೂನ್ಯ *clientp,
		    NSCURL_ಪ್ರಗತಿ_T dltotal,
		    NSCURL_ಪ್ರಗತಿ_T dlnow,
		    NSCURL_ಪ್ರಗತಿ_T ultotal,
		    NSCURL_ಪ್ರಗತಿ_T ulnow)
{
	ಸ್ಥಿರ ಅಕ್ಷರ fetch_ಪ್ರಗತಿ_ಬಫರ್[256]; /**< Progress buffer for cURL */
	ರಚನೆ curl_fetch_info *f = (ರಚನೆ curl_fetch_info *) clientp;
	uint64_t ಸಮಯ_now_ms;
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	ಆದರೆ (f->abort) {
		ಹಿಂತಿರುಗಿ 0;
        }

	ಸಂದೇಶ.type = FETCH_ಪ್ರಗತಿ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಪ್ರಗತಿ = fetch_ಪ್ರಗತಿ_ಬಫರ್;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&ಸಮಯ_now_ms);
#ವ್ಯಾಖ್ಯಾನಿಸು UPDATE_DELAY_MS (1000 / UPDATES_ಪ್ರತಿ_ಎರಡನೇ)
	ಆದರೆ (ಸಮಯ_now_ms - f->ಕೊನೆಯ_ಪ್ರಗತಿ_update < UPDATE_DELAY_MS) {
		ಹಿಂತಿರುಗಿ 0;
        }
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು UPDATE_DELAY_MS
	f->ಕೊನೆಯ_ಪ್ರಗತಿ_update = ಸಮಯ_now_ms;

	ಆದರೆ (dltotal > 0) {
		snprintf(fetch_ಪ್ರಗತಿ_ಬಫರ್, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
	} ಇಲ್ಲದಿದ್ದರೆ {
		snprintf(fetch_ಪ್ರಗತಿ_ಬಫರ್, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);
	}

	ಹಿಂತಿರುಗಿ 0;
}


/**
 * Format curl debug for nslog
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 ಅಕ್ಷರ *ದತ್ತಾಂಶ,
		 size_t ಗಾತ್ರ_2,
		 ಶೂನ್ಯ *userptr)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ s_infotype[CURLINFO_ಕೊನೆ][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	ಆಯ್ಕೆ(type) {
	ಸ್ಥಿತಿ CURLINFO_ಪಠ್ಯ:
	ಸ್ಥಿತಿ CURLINFO_ಶೀರ್ಷಿಕೆ_ಹೊರಹೋಗು:
	ಸ್ಥಿತಿ CURLINFO_ಶೀರ್ಷಿಕೆ_ಒಳ:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ_2 - 1, ದತ್ತಾಂಶ);
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ curl_socket_t fetch_curl_socket_open(ಶೂನ್ಯ *clientp,
		curlsocktype purpose, ರಚನೆ curl_sockaddr *ವಿಳಾಸ)
{
	(ಶೂನ್ಯ) clientp;
	(ಶೂನ್ಯ) purpose;

	ಹಿಂತಿರುಗಿ (curl_socket_t) guit->fetch->socket_open(
			ವಿಳಾಸ->family, ವಿಳಾಸ->socktype,
			ವಿಳಾಸ->protocol);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fetch_curl_socket_close(ಶೂನ್ಯ *clientp, curl_socket_t ಅಂಶ)
{
	(ಶೂನ್ಯ) clientp;

	ಹಿಂತಿರುಗಿ guit->fetch->socket_close((ಪೂರ್ಣಾಂಕ) ಅಂಶ);
}

/**
 * Callback function for cURL.
 */
ಸ್ಥಿರ size_t fetch_curl_ದತ್ತಾಂಶ(ಅಕ್ಷರ *ದತ್ತಾಂಶ, size_t ಗಾತ್ರ_2, size_t nmemb, ಶೂನ್ಯ *_f)
{
	ರಚನೆ curl_fetch_info *f = _f;
	CURLcode ಸಂಕೇತ_2;
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	/* ensure we only have to get this information once */
	ಆದರೆ (!f->http_ಸಂಕೇತ) {
		ಸಂಕೇತ_2 = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_ಸಂಕೇತ,
					 &f->http_ಸಂಕೇತ);
		ಪ್ರತಿಜ್ಞಾ(ಸಂಕೇತ_2 == CURLE_OK);
		fetch_ಗಣ_http_ಸಂಕೇತ(f->fetch_handle, f->http_ಸಂಕೇತ);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	ಆದರೆ (f->http_ಸಂಕೇತ == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_ಸಂಕೇತ = 0;
		ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2 * nmemb;
	}

	ಆದರೆ (f->abort || (!f->had_ಶೀರ್ಷಿಕೆಗಳು && fetch_curl_process_ಶೀರ್ಷಿಕೆಗಳು(f))) {
		f->stopped = true;
		ಹಿಂತಿರುಗಿ 0;
	}

	/* send data to the caller */
	ಸಂದೇಶ.type = FETCH_ದತ್ತಾಂಶ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *) ದತ್ತಾಂಶ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಗಾತ್ರ_2 * nmemb;
	fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);

	ಆದರೆ (f->abort) {
		f->stopped = true;
		ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2 * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
ಸ್ಥಿರ size_t
fetch_curl_ಶೀರ್ಷಿಕೆ(ಅಕ್ಷರ *ದತ್ತಾಂಶ, size_t ಗಾತ್ರ_2, size_t nmemb, ಶೂನ್ಯ *_f)
{
	ರಚನೆ curl_fetch_info *f = _f;
	ಪೂರ್ಣಾಂಕ i;
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಗಾತ್ರ_2 *= nmemb;

	ಆದರೆ (f->abort) {
		f->stopped = true;
		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (f->ಕಳುಹಿಸಿದ_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	ಸಂದೇಶ.type = FETCH_ಶೀರ್ಷಿಕೆ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *) ದತ್ತಾಂಶ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಗಾತ್ರ_2;
	fetch_send_callback(&ಸಂದೇಶ, f->fetch_handle);

#ವ್ಯಾಖ್ಯಾನಿಸು SKIP_ST(o) ಗಾಗಿ (i = (o); i < (ಪೂರ್ಣಾಂಕ) ಗಾತ್ರ_2 && (ದತ್ತಾಂಶ[i] == ' ' || ದತ್ತಾಂಶ[i] == '\t'); i++)

	ಆದರೆ (12 < ಗಾತ್ರ_2 && strncasecmp(ದತ್ತಾಂಶ, "Location:", 9) == 0) {
		/* extract Location header */
		ಬಿಡುಗಡೆ(f->location);
		f->location = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2);
		ಆದರೆ (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
		}
		SKIP_ST(9);
		strncpy(f->location, ದತ್ತಾಂಶ + i, ಗಾತ್ರ_2 - i);
		f->location[ಗಾತ್ರ_2 - i] = '\0';
		ಗಾಗಿ (i = ಗಾತ್ರ_2 - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (15 < ಗಾತ್ರ_2 && strncasecmp(ದತ್ತಾಂಶ, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		ಆದರೆ (i < (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ_2 && '0' <= ದತ್ತಾಂಶ[i] && ದತ್ತಾಂಶ[i] <= '9')
			f->content_ಉದ್ದಳತೆ = atol(ದತ್ತಾಂಶ + i);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (17 < ಗಾತ್ರ_2 && strncasecmp(ದತ್ತಾಂಶ, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		ಆಗ (i < (ಪೂರ್ಣಾಂಕ) ಗಾತ್ರ_2 - 5 &&
				strncasecmp(ದತ್ತಾಂಶ + i, "realm", 5))
			i++;
		ಆಗ (i < (ಪೂರ್ಣಾಂಕ) ಗಾತ್ರ_2 - 1 && ದತ್ತಾಂಶ[++i] != '"')
			/* */;
		i++;

		ಆದರೆ (i < (ಪೂರ್ಣಾಂಕ) ಗಾತ್ರ_2) {
			size_t ಕೊನೆ = i;

			ಆಗ (ಕೊನೆ < ಗಾತ್ರ_2 && ದತ್ತಾಂಶ[ಕೊನೆ] != '"')
				++ಕೊನೆ;

			ಆದರೆ (ಕೊನೆ < ಗಾತ್ರ_2) {
				ಬಿಡುಗಡೆ(f->realm);
				f->realm = ಸ್ಮರಣೆ_ಕೊಡು(ಕೊನೆ - i + 1);
				ಆದರೆ (f->realm != ಖಾಲಿ) {
					strncpy(f->realm, ದತ್ತಾಂಶ + i, ಕೊನೆ - i);
					f->realm[ಕೊನೆ - i] = '\0';
				}
			}
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (11 < ಗಾತ್ರ_2 && strncasecmp(ದತ್ತಾಂಶ, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_ಗಣ_cookie(f->fetch_handle, &ದತ್ತಾಂಶ[i]);
	}

	ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SKIP_ST
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fetch_curl_fdset(lwc_string *scheme, fd_set *read_ಗಣ,
			    fd_set *write_ಗಣ, fd_set *error_ಗಣ)
{
	CURLMcode ಸಂಕೇತ_2;
	ಪೂರ್ಣಾಂಕ maxfd = -1;

	ಸಂಕೇತ_2 = curl_multi_fdset(fetch_curl_multi,
				read_ಗಣ,
				write_ಗಣ,
				error_ಗಣ,
				&maxfd);
	ಪ್ರತಿಜ್ಞಾ(ಸಂಕೇತ_2 == CURLM_OK);

	ಹಿಂತಿರುಗಿ maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_ರಿಜಿಸ್ಟರ್(ಶೂನ್ಯ)
{
	CURLcode ಸಂಕೇತ_2;
	curl_ಆವೃತ್ತಿ_info_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ i;
	lwc_string *scheme;
	ಸ್ಥಿರಾಂಕ ರಚನೆ fetcher_operation_ಕೋಷ್ಟಕ fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.ಸಿದ್ಧತೆ = fetch_curl_ಸಿದ್ಧತೆ,
		.ಆರಂಭ = fetch_curl_ಆರಂಭ,
		.abort = fetch_curl_abort,
		.ಬಿಡುಗಡೆ = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#ಆದರೆ LIBCURL_ಆವೃತ್ತಿ_ಅಂಕೆ >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, ಖಾಲಿ, ಖಾಲಿ);
	ಆದರೆ (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} ಇಲ್ಲದಿದ್ದರೆ {
		curl_with_openssl = false;
	}
#ಆದರೆ_ಅಂತ್ಯ

	NSLOG(netsurf, INFO, "curl_version %s", curl_ಆವೃತ್ತಿ());

	ಸಂಕೇತ_2 = curl_global_init(CURL_GLOBAL_ALL);
	ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	ಆದರೆ (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
	}

#ಆದರೆ LIBCURL_ಆವೃತ್ತಿ_ಅಂಕೆ >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		ಪೂರ್ಣಾಂಕ maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SETOPT
#ವ್ಯಾಖ್ಯಾನಿಸು SETOPT(option, ಮೌಲ್ಯ) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, ಮೌಲ್ಯ);	\
	ಆದರೆ (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		ಹೋಗು curl_multi_setopt_failed;				\
	}

		SETOPT(CURLMOPT_MAXCONNECTS, (ದೀರ್ಘ)maxconnects);
		SETOPT(CURLMOPT_MAX_ಒಟ್ಟು_CONNECTIONS, (ದೀರ್ಘ)maxconnects);
		SETOPT(CURLMOPT_MAX_HOST_CONNECTIONS, (ದೀರ್ಘ)nsoption_int(max_fetchers_ಪ್ರತಿ_host));
	}
#ಆದರೆ_ಅಂತ್ಯ

	/* Create a curl easy handle with the options that are common to all
	 *  fetches.
	 */
	fetch_blank_curl = curl_easy_init();
	ಆದರೆ (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
	}

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SETOPT
#ವ್ಯಾಖ್ಯಾನಿಸು SETOPT(option, ಮೌಲ್ಯ) \
	ಸಂಕೇತ_2 = curl_easy_setopt(fetch_blank_curl, option, ಮೌಲ್ಯ);	\
	ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		ಹೋಗು curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_ಬಫರ್);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	ಆದರೆ (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} ಇಲ್ಲದಿದ್ದರೆ {
		SETOPT(CURLOPT_VERBOSE, 1);
	}

	/* Currently we explode if curl uses HTTP2, so force 1.1. */
	SETOPT(CURLOPT_HTTP_ಆವೃತ್ತಿ, CURL_HTTP_ಆವೃತ್ತಿ_1_1);

	SETOPT(CURLOPT_WRITEFUNCTION, fetch_curl_ದತ್ತಾಂಶ);
	SETOPT(CURLOPT_HEADERFUNCTION, fetch_curl_ಶೀರ್ಷಿಕೆ);
	SETOPT(NSCURLOPT_ಪ್ರಗತಿ_FUNCTION, fetch_curl_ಪ್ರಗತಿ);
	SETOPT(CURLOPT_NOPROGRESS, 0L);
	SETOPT(CURLOPT_USERAGENT, user_agent_string());
	SETOPT(CURLOPT_ENCODING, "gzip");
	SETOPT(CURLOPT_ಕೆಳಗಿನ_ವೇಗ_ಎಲ್ಲೆ, 1L);
	SETOPT(CURLOPT_ಕೆಳಗಿನ_ವೇಗ_ಸಮಯ, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (ದೀರ್ಘ)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	ಆದರೆ (nsoption_charp(ca_bundle) &&
	    strcmp(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	ಆದರೆ (nsoption_charp(ca_ಮಾರ್ಗ) && strcmp(nsoption_charp(ca_ಮಾರ್ಗ), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_ಮಾರ್ಗ));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_ಮಾರ್ಗ));
	}

#ಆದರೆ LIBCURL_ಆವೃತ್ತಿ_ಅಂಕೆ < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	ಸಂಕೇತ_2 = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, ಖಾಲಿ);
	ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK) {
		curl_with_openssl = false;
	} ಇಲ್ಲದಿದ್ದರೆ {
		curl_with_openssl = true;
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#ಆದರೆ LIBCURL_ಆವೃತ್ತಿ_ಅಂಕೆ >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		ಸಂಕೇತ_2 = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		ಆದರೆ (ಸಂಕೇತ_2 != CURLE_OK && ಸಂಕೇತ_2 != CURLE_NOT_BUILT_ಒಳ)
			ಹೋಗು curl_easy_setopt_failed;
#ಆದರೆ_ಅಂತ್ಯ
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	ದತ್ತಾಂಶ = curl_ಆವೃತ್ತಿ_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	ಆದರೆ (curl_fetch_ssl_hashmap == ಖಾಲಿ) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಗಾಗಿ (i = 0; ದತ್ತಾಂಶ->protocols[i]; i++) {
		ಆದರೆ (strcmp(ದತ್ತಾಂಶ->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(ದತ್ತಾಂಶ->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Ignore non-http(s) protocols */
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (fetcher_ಕೂಡಿಸು(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      ದತ್ತಾಂಶ->protocols[i]);
		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;

#ಆದರೆ LIBCURL_ಆವೃತ್ತಿ_ಅಂಕೆ >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
#ಆದರೆ_ಅಂತ್ಯ
}
