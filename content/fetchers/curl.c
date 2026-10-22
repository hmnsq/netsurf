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
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/inet.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <libwapcaplet/libwapcaplet.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nsutils/time.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/hashmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/useragent.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/file.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/string.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers/curl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/urldb.h"

/**
 * maximum number of progress notifications per second
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UPDATES_ਪ੍ਰਤੀ_ਦੂਜਾ 2

/**
 * The ciphersuites the browser is prepared to use for TLS1.3
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIPHER_SUITES						\
	"TLS_AES_256_GCM_SHA384:"				\
	"TLS_CHACHA20_POLY1305_SHA256:"				\
	"TLS_AES_128_GCM_SHA256"

/**
 * The ciphersuites the browser is prepared to use for TLS<1.3
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIPHER_LIST						\
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIPHER_LIST_LEGACY					\
	/* as above */						\
	CIPHER_LIST":"						\
	/* enable (non-PFS) RSA AES GCM suites */		\
	"RSA+AESGCM:"						\
	/* enable (non-PFS) RSA (mac=SHA1) AES CBC suites */	\
	"RSA+AES+SHA1"

/* Open SSL compatability for certificate handling */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_OPENSSL

#ਅਨ੍ਤਰ੍ਭਾਵ <openssl/ssl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <openssl/x509v3.h>

#ਨਹੀਂਤਾਂ /* WITH_OPENSSL */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅੱਖਰ X509;

ਸਥਿਰ ਖਾਲੀ X509_free(X509 *cert)
{
	ਮੁਕਤ(cert);
}

#ਜੇ_ਅੰਤ /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
ਸਥਿਰ uint32_t
curl_fetch_ssl_ਕੁੰਜੀ_hash(ਖਾਲੀ *ਕੁੰਜੀ)
{
	nsurl *url = ਕੁੰਜੀ;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *ਪੋਰਟ = nsurl_get_component(url, NSURL_ਪੋਰਟ);
	uint32_t hash;

	ਜੇ (ਪੋਰਟ == ਨਿਰਾ)
		ਪੋਰਟ = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_ਮੁੱਲ(hostname) ^ lwc_string_hash_ਮੁੱਲ(ਪੋਰਟ);

	lwc_string_unref(hostname);
	lwc_string_unref(ਪੋਰਟ);

	ਵਾਪਸ hash;
}

/* We only compare the hostname and port */
ਸਥਿਰ bool
curl_fetch_ssl_ਕੁੰਜੀ_eq(ਖਾਲੀ *key1, ਖਾਲੀ *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_ਪੋਰਟ);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_ਪੋਰਟ);
	bool iseq = false;

	ਜੇ (port1 == ਨਿਰਾ)
		port1 = lwc_string_ref(corestring_lwc_443);
	ਜੇ (port2 == ਨਿਰਾ)
		port2 = lwc_string_ref(corestring_lwc_443);

	ਜੇ (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		ਜਾਓ ਬਾਹਰਲਾ;

	iseq = false;
	ਜੇ (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		ਜਾਓ ਬਾਹਰਲਾ;

ਬਾਹਰਲਾ:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	ਵਾਪਸ iseq;
}

ਸਥਿਰ ਖਾਲੀ *
curl_fetch_ssl_ਮੁੱਲ_alloc(ਖਾਲੀ *ਕੁੰਜੀ)
{
	ਬਣਤਰ cert_chain *ਬਾਹਰਲਾ;

	ਜੇ (cert_chain_alloc(0, &ਬਾਹਰਲਾ) != NSERROR_OK) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ ਬਾਹਰਲਾ;
}

ਸਥਿਰ ਖਾਲੀ
curl_fetch_ssl_ਮੁੱਲ_destroy(ਖਾਲੀ *ਮੁੱਲ)
{
	ਬਣਤਰ cert_chain *chain = ਮੁੱਲ;
	ਜੇ (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

ਸਥਿਰ hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.ਕੁੰਜੀ_clone = (hashmap_ਕੁੰਜੀ_clone_t)nsurl_ref,
	.ਕੁੰਜੀ_destroy = (hashmap_ਕੁੰਜੀ_destroy_t)nsurl_unref,
	.ਕੁੰਜੀ_eq = curl_fetch_ssl_ਕੁੰਜੀ_eq,
	.ਕੁੰਜੀ_hash = curl_fetch_ssl_ਕੁੰਜੀ_hash,
	.ਮੁੱਲ_alloc = curl_fetch_ssl_ਮੁੱਲ_alloc,
	.ਮੁੱਲ_destroy = curl_fetch_ssl_ਮੁੱਲ_destroy,
};

ਸਥਿਰ hashmap_t *curl_fetch_ssl_hashmap = ਨਿਰਾ;

/** SSL certificate info */
ਬਣਤਰ cert_info {
	X509 *cert;		/**< Pointer to certificate */
	ਲੰਮਾ err;		/**< OpenSSL error code */
};

#ਜੇ LIBCURL_ਸੰਸਕਰਣ_ਅੰਕ >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSCURLOPT_ਤਰੱਕੀ_FUNCTION CURLOPT_XFERINFOFUNCTION
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSCURLOPT_ਤਰੱਕੀ_ਅੰਕੜੇ CURLOPT_XFERINFODATA
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSCURL_ਤਰੱਕੀ_T curl_off_t
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSCURLOPT_ਤਰੱਕੀ_FUNCTION CURLOPT_PROGRESSFUNCTION
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSCURLOPT_ਤਰੱਕੀ_ਅੰਕੜੇ CURLOPT_PROGRESSDATA
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSCURL_ਤਰੱਕੀ_T ਦੁੱਗਣਾ
#ਜੇ_ਅੰਤ

#ਜੇ LIBCURL_ਸੰਸਕਰਣ_ਅੰਕ >= 0x073800 /* 7.56.0 depricated curl_formadd */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSCURL_POSTDATA_T curl_mime
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSCURL_POSTDATA_T ਬਣਤਰ curl_httppost
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#ਜੇ_ਅੰਤ

/** Information for a single fetch. */
ਬਣਤਰ curl_fetch_info {
	ਬਣਤਰ fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool ਭੇਜਿਆ_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_ਸਿਰਲੇਖਾਂ;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	ਬਣਤਰ curl_slist *ਸਿਰਲੇਖਾਂ;	/**< List of request headers. */
	ਅੱਖਰ *location;		/**< Response Location header, or 0. */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ content_ਲੰਮਾਈ;	/**< Response Content-Length, or 0. */
	ਅੱਖਰ *cookie_string;	/**< Cookie string for this fetch */
	ਅੱਖਰ *realm;		/**< HTTP Auth Realm */
	ਬਣਤਰ fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	ਲੰਮਾ http_ਕੋਡ; /**< HTTP result code from cURL. */

	uint64_t ਆਖਰੀ_ਤਰੱਕੀ_update;	/**< Time of last progress update */
	ਪੂਰਨ cert_ਡੂੰਘਾਈ; /**< deepest certificate in use */
	ਬਣਤਰ cert_info cert_ਅੰਕੜੇ[MAX_CERT_ਡੂੰਘਾਈ]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
ਬਣਤਰ cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	ਬਣਤਰ cache_handle *r_prev; /**< Previous cached handle in ring. */
	ਬਣਤਰ cache_handle *r_ਅਗਲਾ; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
ਸਥਿਰ CURL *fetch_blank_curl;

/** Ring of cached handles */
ਸਥਿਰ ਬਣਤਰ cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
ਸਥਿਰ ਪੂਰਨ curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
ਸਥਿਰ bool curl_with_openssl;

/** Error buffer for cURL. */
ਸਥਿਰ ਅੱਖਰ fetch_error_ਬਫਰ[CURL_ERROR_ਮਾਪ];

/** Proxy authentication details. */
ਸਥਿਰ ਅੱਖਰ fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
ਸਥਿਰ bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
ਸਥਿਰ bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_ਅੰਕੜੇ(scheme));
	curl_fetchers_registered++;
	ਵਾਪਸ true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
ਸਥਿਰ ਖਾਲੀ fetch_curl_finalise(lwc_string *scheme)
{
	ਬਣਤਰ cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_ਅੰਕੜੇ(scheme));
	ਜੇ (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		ਜੇ (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = ਨਿਰਾ;
	}

	/* Free anything remaining in the cached curl handle ring */
	ਜਦੋਂ (curl_handle_ring != ਨਿਰਾ) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
		curl_easy_cleanup(h->handle);
		ਮੁਕਤ(h);
	}
}


/**
 * Check if this fetcher can fetch a url.
 *
 * \param url The url to check.
 * \return true if the fetcher supports the url else false.
 */
ਸਥਿਰ bool fetch_curl_can_fetch(ਅਚਲ nsurl *url)
{
	ਵਾਪਸ nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
ਸਥਿਰ ਬਣਤਰ fetch_postdata *
fetch_curl_alloc_postdata(ਅਚਲ ਅੱਖਰ *post_urlenc,
			  ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *post_multipart)
{
	ਬਣਤਰ fetch_postdata *postdata;
	postdata = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ fetch_postdata));
	ਜੇ (postdata != ਨਿਰਾ) {

		ਜੇ (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->ਅੰਕੜੇ.urlenc = strdup(post_urlenc);
			ਜੇ (postdata->ਅੰਕੜੇ.urlenc == ਨਿਰਾ) {
				ਮੁਕਤ(postdata);
				postdata = ਨਿਰਾ;
			}
		} ਨਹੀਂਤਾਂ ਜੇ (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->ਅੰਕੜੇ.multipart = fetch_multipart_ਅੰਕੜੇ_clone(post_multipart);
			ਜੇ (postdata->ਅੰਕੜੇ.multipart == ਨਿਰਾ) {
				ਮੁਕਤ(postdata);
				postdata = ਨਿਰਾ;
			}
		} ਨਹੀਂਤਾਂ {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	ਵਾਪਸ postdata;
}

/**
 * free postdata
 */
ਸਥਿਰ ਖਾਲੀ fetch_curl_free_postdata(ਬਣਤਰ fetch_postdata *postdata)
{
	ਜੇ (postdata != ਨਿਰਾ) {
		ਚੋਣ (postdata->type) {
		ਹਾਲਤ FETCH_POSTDATA_NONE:
			ਰੋਕੋ;
		ਹਾਲਤ FETCH_POSTDATA_URLENC:
			ਮੁਕਤ(postdata->ਅੰਕੜੇ.urlenc);
			ਰੋਕੋ;
		ਹਾਲਤ FETCH_POSTDATA_MULTIPART:
			fetch_multipart_ਅੰਕੜੇ_destroy(postdata->ਅੰਕੜੇ.multipart);
			ਰੋਕੋ;
		}

		ਮੁਕਤ(postdata);
	}
}

/**
 *construct a new fetch structure
 */
ਸਥਿਰ ਬਣਤਰ curl_fetch_info *fetch_alloc(ਖਾਲੀ)
{
	ਬਣਤਰ curl_fetch_info *fetch;
	fetch = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ (*fetch));
	ਜੇ (fetch == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	fetch->curl_handle = ਨਿਰਾ;
	fetch->ਭੇਜਿਆ_ssl_chain = false;
	fetch->had_ਸਿਰਲੇਖਾਂ = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->ਸਿਰਲੇਖਾਂ = ਨਿਰਾ;
	fetch->url = ਨਿਰਾ;
	fetch->host = ਨਿਰਾ;
	fetch->location = ਨਿਰਾ;
	fetch->content_ਲੰਮਾਈ = 0;
	fetch->http_ਕੋਡ = 0;
	fetch->cookie_string = ਨਿਰਾ;
	fetch->realm = ਨਿਰਾ;
	fetch->ਆਖਰੀ_ਤਰੱਕੀ_update = 0;
	fetch->postdata = ਨਿਰਾ;
	fetch->curl_postdata = ਨਿਰਾ;

	/* Clear certificate chain data */
	memset(fetch->cert_ਅੰਕੜੇ, 0, ਆਕਾਰ(fetch->cert_ਅੰਕੜੇ));
	fetch->cert_ਡੂੰਘਾਈ = -1;

	ਵਾਪਸ fetch;
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
ਸਥਿਰ ਖਾਲੀ *
fetch_curl_ਤਿਆਰੀ(ਬਣਤਰ fetch *ਮਾਪਾ_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ਅਚਲ ਅੱਖਰ *post_urlenc,
		 ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *post_multipart,
		 ਅਚਲ ਅੱਖਰ **ਸਿਰਲੇਖਾਂ)
{
	ਬਣਤਰ curl_fetch_info *fetch;
	ਬਣਤਰ curl_slist *slist;
	ਪੂਰਨ i;

	fetch = fetch_alloc();
	ਜੇ (fetch == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = ਮਾਪਾ_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	ਜੇ (fetch->host == ਨਿਰਾ) {
		ਜਾਓ failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	ਜੇ (fetch->postdata == ਨਿਰਾ) {
		ਜਾਓ failed;
	}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ APPEND(list, ਮੁੱਲ) \
	slist = curl_slist_append(list, ਮੁੱਲ);		\
	ਜੇ (slist == ਨਿਰਾ)				\
		ਜਾਓ failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->ਸਿਰਲੇਖਾਂ, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->ਸਿਰਲੇਖਾਂ, "Expect:");

	ਜੇ ((nsoption_charp(accept_language) != ਨਿਰਾ) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		ਅੱਖਰ s[80];
		snprintf(s, ਆਕਾਰ s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[ਆਕਾਰ s - 1] = 0;
		APPEND(fetch->ਸਿਰਲੇਖਾਂ, s);
	}

	ਜੇ (nsoption_charp(accept_charset) != ਨਿਰਾ &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		ਅੱਖਰ s[80];
		snprintf(s, ਆਕਾਰ s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[ਆਕਾਰ s - 1] = 0;
		APPEND(fetch->ਸਿਰਲੇਖਾਂ, s);
	}

	ਜੇ (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->ਸਿਰਲੇਖਾਂ, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	ਲਈ (i = 0; ਸਿਰਲੇਖਾਂ[i] != ਨਿਰਾ; i++) {
		APPEND(fetch->ਸਿਰਲੇਖਾਂ, ਸਿਰਲੇਖਾਂ[i]);
	}

	ਵਾਪਸ fetch;

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->ਸਿਰਲੇਖਾਂ);
	ਮੁਕਤ(fetch);
	ਵਾਪਸ ਨਿਰਾ;
}


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
ਸਥਿਰ ਬਣਤਰ cert_chain *
fetch_curl_get_cached_chain(ਬਣਤਰ curl_fetch_info *f)
{
	ਬਣਤਰ cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	ਜੇ (chain == ਨਿਰਾ) {
		chain = hashmap_ਪਾਓ(curl_fetch_ssl_hashmap, f->url);
	}

	ਵਾਪਸ chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
ਸਥਿਰ ਖਾਲੀ
fetch_curl_store_certs_ਵਿੱਚ_cache(ਬਣਤਰ curl_fetch_info *f)
{
	size_t ਡੂੰਘਾਈ;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_ਡੂੰਘਾਈ];
	ਬਣਤਰ cert_chain chain, *cached_chain;
	ਬਣਤਰ cert_info *certs;

	memset(&chain, 0, ਆਕਾਰ(chain));

	certs = f->cert_ਅੰਕੜੇ;
	chain.ਡੂੰਘਾਈ = f->cert_ਡੂੰਘਾਈ + 1; /* 0 indexed certificate depth */

	ਲਈ (ਡੂੰਘਾਈ = 0; ਡੂੰਘਾਈ < chain.ਡੂੰਘਾਈ; ਡੂੰਘਾਈ++) {
		ਜੇ (certs[ਡੂੰਘਾਈ].cert == ਨਿਰਾ) {
			/* This certificate is missing, skip it */
			chain.certs[ਡੂੰਘਾਈ].err = SSL_CERT_ERR_CERT_MISSING;
			ਜਾਰੀ;
		}

		/* error code (if any) */
		ਚੋਣ (certs[ਡੂੰਘਾਈ].err) {
		ਹਾਲਤ X509_V_OK:
			chain.certs[ਡੂੰਘਾਈ].err = SSL_CERT_ERR_OK;
			ਰੋਕੋ;

		ਹਾਲਤ X509_V_ERR_UNABLE_ਮੰਜ਼ਿਲ_GET_ISSUER_CERT:
			/* fallthrough */
		ਹਾਲਤ X509_V_ERR_UNABLE_ਮੰਜ਼ਿਲ_DECODE_ISSUER_ਜਨਤਕ_ਕੁੰਜੀ:
			chain.certs[ਡੂੰਘਾਈ].err = SSL_CERT_ERR_BAD_ISSUER;
			ਰੋਕੋ;

		ਹਾਲਤ X509_V_ERR_UNABLE_ਮੰਜ਼ਿਲ_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		ਹਾਲਤ X509_V_ERR_UNABLE_ਮੰਜ਼ਿਲ_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		ਹਾਲਤ X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		ਹਾਲਤ X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[ਡੂੰਘਾਈ].err = SSL_CERT_ERR_BAD_SIG;
			ਰੋਕੋ;

		ਹਾਲਤ X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		ਹਾਲਤ X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[ਡੂੰਘਾਈ].err = SSL_CERT_ERR_TOO_YOUNG;
			ਰੋਕੋ;

		ਹਾਲਤ X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		ਹਾਲਤ X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[ਡੂੰਘਾਈ].err = SSL_CERT_ERR_TOO_OLD;
			ਰੋਕੋ;

		ਹਾਲਤ X509_V_ERR_ਡੂੰਘਾਈ_ZERO_SELF_SIGNED_CERT:
			chain.certs[ਡੂੰਘਾਈ].err = SSL_CERT_ERR_SELF_SIGNED;
			ਰੋਕੋ;

		ਹਾਲਤ X509_V_ERR_SELF_SIGNED_CERT_ਵਿੱਚ_CHAIN:
			chain.certs[ਡੂੰਘਾਈ].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			ਰੋਕੋ;

		ਹਾਲਤ X509_V_ERR_CERT_REVOKED:
			chain.certs[ਡੂੰਘਾਈ].err = SSL_CERT_ERR_REVOKED;
			ਰੋਕੋ;

		ਹਾਲਤ X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[ਡੂੰਘਾਈ].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			ਰੋਕੋ;

		ਮੂਲ:
			chain.certs[ਡੂੰਘਾਈ].err = SSL_CERT_ERR_UNKNOWN;
			ਰੋਕੋ;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[ਡੂੰਘਾਈ].cert);
		BIO_get_mem_ptr(mem, &buf[ਡੂੰਘਾਈ]);
		(ਖਾਲੀ) BIO_ਸੈੱਟ_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[ਡੂੰਘਾਈ].der = (uint8_t *)buf[ਡੂੰਘਾਈ]->ਅੰਕੜੇ;
		chain.certs[ਡੂੰਘਾਈ].der_ਲੰਮਾਈ = buf[ਡੂੰਘਾਈ]->ਲੰਮਾਈ;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	ਜੇ (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	ਲਈ (ਡੂੰਘਾਈ = 0; ਡੂੰਘਾਈ < chain.ਡੂੰਘਾਈ; ਡੂੰਘਾਈ++) {
		ਜੇ (chain.certs[ਡੂੰਘਾਈ].err == SSL_CERT_ERR_CERT_MISSING) {
			ਜਾਰੀ;
		}
		ਜੇ (buf[ਡੂੰਘਾਈ] != ਨਿਰਾ) {
			BUF_MEM_free(buf[ਡੂੰਘਾਈ]);
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
ਸਥਿਰ ਪੂਰਨ
fetch_curl_verify_callback(ਪੂਰਨ verify_ok, X509_STORE_CTX *x509_ctx)
{
	ਪੂਰਨ ਡੂੰਘਾਈ;
	ਬਣਤਰ curl_fetch_info *fetch;

	ਡੂੰਘਾਈ = X509_STORE_CTX_get_error_ਡੂੰਘਾਈ(x509_ctx);
	fetch = X509_STORE_CTX_get_app_ਅੰਕੜੇ(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	ਜੇ (ਡੂੰਘਾਈ >= MAX_CERT_ਡੂੰਘਾਈ) {
		X509_STORE_CTX_ਸੈੱਟ_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		ਵਾਪਸ 0;
	}

	/* record the max depth */
	ਜੇ (ਡੂੰਘਾਈ > fetch->cert_ਡੂੰਘਾਈ) {
		fetch->cert_ਡੂੰਘਾਈ = ਡੂੰਘਾਈ;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	ਜੇ (!fetch->cert_ਅੰਕੜੇ[ਡੂੰਘਾਈ].cert) {
		fetch->cert_ਅੰਕੜੇ[ਡੂੰਘਾਈ].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_ਉੱਪਰ_ref(fetch->cert_ਅੰਕੜੇ[ਡੂੰਘਾਈ].cert);
		fetch->cert_ਅੰਕੜੇ[ਡੂੰਘਾਈ].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	ਜੇ (ਡੂੰਘਾਈ > 0) {
		verify_ok = 1;
	} ਨਹੀਂਤਾਂ {
		/* search for deeper certificates in the chain with errors */
		ਲਈ (ਡੂੰਘਾਈ = fetch->cert_ਡੂੰਘਾਈ; ਡੂੰਘਾਈ > 0; ਡੂੰਘਾਈ--) {
			ਜੇ (fetch->cert_ਅੰਕੜੇ[ਡੂੰਘਾਈ].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_ਸੈੱਟ_error(x509_ctx, fetch->cert_ਅੰਕੜੇ[ਡੂੰਘਾਈ].err);
			}
		}
	}

	ਵਾਪਸ verify_ok;
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
ਸਥਿਰ ਪੂਰਨ fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, ਖਾਲੀ *parm)
{
	ਬਣਤਰ curl_fetch_info *f = (ਬਣਤਰ curl_fetch_info *) parm;
	ਪੂਰਨ ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_ਸੈੱਟ_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_ਅੰਕੜੇ(f->host),
					 lwc_string_ਲੰਮਾਈ(f->host));

	/* Store fetch struct in context for verify callback */
	ਜੇ (ok) {
		ok = X509_STORE_CTX_ਸੈੱਟ_app_ਅੰਕੜੇ(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	ਜੇ (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_ਵਿੱਚ_cache(f);

	ਵਾਪਸ ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
ਸਥਿਰ CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, ਖਾਲੀ *_sslctx, ਖਾਲੀ *parm)
{
	ਬਣਤਰ curl_fetch_info *f = (ਬਣਤਰ curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	ਲੰਮਾ options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_ਸੈੱਟ_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_ਸੈੱਟ_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	ਜੇ (f->downgrade_tls) {
		/* Disable TLS 1.3 if the server can't cope with it */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SSL_OP_NO_TLSv1_3
		options |= SSL_OP_NO_TLSv1_3;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SSL_ਬਹੁਲਕ_SEND_FALLBACK_SCSV
		/* Ensure server rejects the connection if downgraded too far */
		SSL_CTX_ਸੈੱਟ_ਬਹੁਲਕ(sslctx, SSL_ਬਹੁਲਕ_SEND_FALLBACK_SCSV);
#ਜੇ_ਅੰਤ
	}

	SSL_CTX_ਸੈੱਟ_options(sslctx, options);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SSL_OP_NO_TICKET
	SSL_CTX_clear_options(sslctx, SSL_OP_NO_TICKET);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ CURLE_OK;
}


#ਜੇ_ਅੰਤ /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
ਸਥਿਰ ਖਾਲੀ
fetch_curl_report_certs_upstream(ਬਣਤਰ curl_fetch_info *f)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	ਬਣਤਰ cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	ਜੇ (chain != ਨਿਰਾ) {
		ਸੁਨੇਹਾ.type = FETCH_CERTS;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.chain = chain;

		fetch_send_callback(&ਸੁਨੇਹਾ, f->fetch_handle);
	}

	f->ਭੇਜਿਆ_ssl_chain = true;
}

#ਜੇ LIBCURL_ਸੰਸਕਰਣ_ਅੰਕ >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
ਬਣਤਰ curl_mime_ctx {
	ਅੱਖਰ *ਬਫਰ;
	curl_off_t ਮਾਪ;
	curl_off_t position;
};

ਸਥਿਰ size_t mime_ਅੰਕੜੇ_read_callback(ਅੱਖਰ *ਬਫਰ, size_t ਮਾਪ, size_t nitems, ਖਾਲੀ *arg)
{
	ਬਣਤਰ curl_mime_ctx *mctx = (ਬਣਤਰ curl_mime_ctx *) arg;
	curl_off_t sz = mctx->ਮਾਪ - mctx->position;

	nitems *= ਮਾਪ;
	ਜੇ(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	ਜੇ(sz) {
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਫਰ, mctx->ਬਫਰ + mctx->position, sz);
	}
	mctx->position += sz;
	ਵਾਪਸ sz;
}

ਸਥਿਰ ਪੂਰਨ mime_ਅੰਕੜੇ_seek_callback(ਖਾਲੀ *arg, curl_off_t ਵਿੱਥ, ਪੂਰਨ origin)
{
	ਬਣਤਰ curl_mime_ctx *mctx = (ਬਣਤਰ curl_mime_ctx *) arg;

	ਚੋਣ(origin) {
	ਹਾਲਤ SEEK_END:
		ਵਿੱਥ += mctx->ਮਾਪ;
		ਰੋਕੋ;
	ਹਾਲਤ SEEK_CUR:
		ਵਿੱਥ += mctx->position;
		ਰੋਕੋ;
	}

	ਜੇ(ਵਿੱਥ < 0) {
		ਵਾਪਸ CURL_SEEKFUNC_FAIL;
	}
	mctx->position = ਵਿੱਥ;
	ਵਾਪਸ CURL_SEEKFUNC_OK;
}

ਸਥਿਰ ਖਾਲੀ mime_ਅੰਕੜੇ_free_callback(ਖਾਲੀ *arg)
{
	ਬਣਤਰ curl_mime_ctx *mctx = (ਬਣਤਰ curl_mime_ctx *) arg;
	ਮੁਕਤ(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
ਸਥਿਰ curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode ਕੋਡ = CURLE_OK;
	size_t ਮੁੱਲ_len;

	cmime = curl_mime_init(chandle);
	ਜੇ (cmime == ਨਿਰਾ) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* iterate post data */
	ਲਈ (; multipart != ਨਿਰਾ; multipart = multipart->ਅਗਲਾ) {
		part = curl_mime_addpart(cmime);
		ਜੇ (part == ਨਿਰਾ) {
			ਜਾਓ convert_failed;
		}

		ਕੋਡ = curl_mime_name(part, multipart->name);
		ਜੇ (ਕੋਡ != CURLE_OK) {
			ਜਾਓ convert_failed;
		}

		ਮੁੱਲ_len = ਲੜੀ_ਲੰਬਾਈ(multipart->ਮੁੱਲ);

		ਜੇ (multipart->file && ਮੁੱਲ_len==0) {
			/* file entries with no filename require special handling */
			ਕੋਡ=curl_mime_ਅੰਕੜੇ(part, multipart->ਮੁੱਲ, ਮੁੱਲ_len);
			ਜੇ (ਕੋਡ != CURLE_OK) {
				ਜਾਓ convert_failed;
			}

			ਕੋਡ = curl_mime_filename(part, "");
			ਜੇ (ਕੋਡ != CURLE_OK) {
				ਜਾਓ convert_failed;
			}

			ਕੋਡ = curl_mime_type(part, "application/octet-stream");
			ਜੇ (ਕੋਡ != CURLE_OK) {
				ਜਾਓ convert_failed;
			}

		} ਨਹੀਂਤਾਂ ਜੇ(multipart->file) {
			/* file entry */
			nserror ret;
			ਅੱਖਰ *leafname = ਨਿਰਾ;
			ਅੱਖਰ *mimetype = ਨਿਰਾ;

			ਕੋਡ = curl_mime_filedata(part, multipart->rawfile);
			ਜੇ (ਕੋਡ != CURLE_OK) {
				ਜਾਓ convert_failed;
			}

			ret = guit->file->basename(multipart->ਮੁੱਲ, &leafname, ਨਿਰਾ);
			ਜੇ (ret != NSERROR_OK) {
				ਜਾਓ convert_failed;
			}
			ਕੋਡ = curl_mime_filename(part, leafname);
			ਮੁਕਤ(leafname);
			ਜੇ (ਕੋਡ != CURLE_OK) {
				ਜਾਓ convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->ਮੁੱਲ);
			ਜੇ (mimetype == ਨਿਰਾ) {
				mimetype=strdup("text/plain");
			}
			ਜੇ (mimetype == ਨਿਰਾ) {
				ਜਾਓ convert_failed;
			}
			ਕੋਡ = curl_mime_type(part, mimetype);
			ਮੁਕਤ(mimetype);
			ਜੇ (ਕੋਡ != CURLE_OK) {
				ਜਾਓ convert_failed;
			}

		} ਨਹੀਂਤਾਂ {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			ਬਣਤਰ curl_mime_ctx *cb_ctx;
			cb_ctx = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ curl_mime_ctx));
			ਜੇ (cb_ctx == ਨਿਰਾ) {
				ਜਾਓ convert_failed;
			}
			cb_ctx->ਬਫਰ = multipart->ਮੁੱਲ;
			cb_ctx->ਮਾਪ = ਮੁੱਲ_len;
			cb_ctx->position = 0;
			ਕੋਡ = curl_mime_ਅੰਕੜੇ_cb(part,
						 ਮੁੱਲ_len,
						 mime_ਅੰਕੜੇ_read_callback,
						 mime_ਅੰਕੜੇ_seek_callback,
						 mime_ਅੰਕੜੇ_free_callback,
						 cb_ctx);
			ਜੇ (ਕੋਡ != CURLE_OK) {
				ਮੁਕਤ(cb_ctx);
				ਜਾਓ convert_failed;
			}
		}
	}

	ਵਾਪਸ cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", ਕੋਡ);
	curl_mime_free(cmime);
	ਵਾਪਸ ਨਿਰਾ;
}

#ਨਹੀਂਤਾਂ /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
ਸਥਿਰ ਬਣਤਰ curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *control)
{
	ਬਣਤਰ curl_httppost *post = ਨਿਰਾ, *ਆਖਰੀ = ਨਿਰਾ;
	CURLFORMcode ਕੋਡ;
	nserror ret;

	ਲਈ (; control; control = control->ਅਗਲਾ) {
		ਜੇ (control->file) {
			ਅੱਖਰ *leafname = ਨਿਰਾ;
			ret = guit->file->basename(control->ਮੁੱਲ, &leafname, ਨਿਰਾ);
			ਜੇ (ret != NSERROR_OK) {
				ਜਾਰੀ;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			ਜੇ (control->ਮੁੱਲ[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				ਸਥਿਰ ਅੱਖਰ buf;

				ਕੋਡ = curl_formadd(&post, &ਆਖਰੀ,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_ਬਫਰ, control->ਮੁੱਲ,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_ਸਿਰਾ);
				ਜੇ (ਕੋਡ != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", ਕੋਡ,
					      control->name);
			} ਨਹੀਂਤਾਂ {
				ਅੱਖਰ *mimetype = guit->fetch->mimetype(control->ਮੁੱਲ);
				ਕੋਡ = curl_formadd(&post, &ਆਖਰੀ,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_ਸਿਰਾ);
				ਜੇ (ਕੋਡ != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      ਕੋਡ,
					      control->name,
					      control->ਮੁੱਲ);
				ਮੁਕਤ(mimetype);
			}
			ਮੁਕਤ(leafname);
		} ਨਹੀਂਤਾਂ {
			ਕੋਡ = curl_formadd(&post, &ਆਖਰੀ,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->ਮੁੱਲ,
					    CURLFORM_ਸਿਰਾ);
			ਜੇ (ਕੋਡ != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", ਕੋਡ,
				      control->name, control->ਮੁੱਲ);
		}
	}

	ਵਾਪਸ post;
}

#ਜੇ_ਅੰਤ  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
ਸਥਿਰ CURLcode fetch_curl_ਸੈੱਟ_postdata(ਬਣਤਰ curl_fetch_info *f)
{
	CURLcode ਕੋਡ = CURLE_OK;

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SETOPT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SETOPT(option, ਮੁੱਲ) { \
	ਕੋਡ = curl_easy_setopt(f->curl_handle, option, ਮੁੱਲ);	\
	ਜੇ (ਕੋਡ != CURLE_OK)					\
		ਵਾਪਸ ਕੋਡ;					\
	}

	ਚੋਣ (f->postdata->type) {
	ਹਾਲਤ FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, ਨਿਰਾ);
		SETOPT(NSCURL_POSTDATA_CURLOPT, ਨਿਰਾ);
		SETOPT(CURLOPT_HTTPGET, 1L);
		ਰੋਕੋ;

	ਹਾਲਤ FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, ਨਿਰਾ);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->ਅੰਕੜੇ.urlenc);
		ਰੋਕੋ;

	ਹਾਲਤ FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, ਨਿਰਾ);
		SETOPT(CURLOPT_HTTPGET, 0L);
		ਜੇ (f->curl_postdata == ਨਿਰਾ) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->ਅੰਕੜੇ.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		ਰੋਕੋ;
	}
	ਵਾਪਸ ਕੋਡ;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
ਸਥਿਰ CURLcode fetch_curl_ਸੈੱਟ_options(ਬਣਤਰ curl_fetch_info *f)
{
	CURLcode ਕੋਡ;
	ਅਚਲ ਅੱਖਰ *auth;

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SETOPT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SETOPT(option, ਮੁੱਲ) { \
	ਕੋਡ = curl_easy_setopt(f->curl_handle, option, ਮੁੱਲ);	\
	ਜੇ (ਕੋਡ != CURLE_OK)					\
		ਵਾਪਸ ਕੋਡ;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_ਨਿੱਜੀ, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_ਤਰੱਕੀ_ਅੰਕੜੇ, f);
	SETOPT(CURLOPT_HTTPHEADER, f->ਸਿਰਲੇਖਾਂ);
	ਕੋਡ = fetch_curl_ਸੈੱਟ_postdata(f);
	ਜੇ (ਕੋਡ != CURLE_OK) {
		ਵਾਪਸ ਕੋਡ;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	ਜੇ (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} ਨਹੀਂਤਾਂ {
		SETOPT(CURLOPT_COOKIE, ਨਿਰਾ);
	}

	ਜੇ ((auth = urldb_get_auth_details(f->url, ਨਿਰਾ)) != ਨਿਰਾ) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} ਨਹੀਂਤਾਂ {
		SETOPT(CURLOPT_USERPWD, ਨਿਰਾ);
	}

	/* set up proxy options */
	ਜੇ (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != ਨਿਰਾ) &&
	    (ਲੜੀ_ਤੁਲਨਾ(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (ਲੰਮਾ) nsoption_int(http_proxy_ਪੋਰਟ));

#ਜੇ LIBCURL_ਸੰਸਕਰਣ_ਅੰਕ >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#ਜੇ_ਅੰਤ

		ਜੇ (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(ਲੰਮਾ) CURLAUTH_BASIC :
					(ਲੰਮਾ) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					ਆਕਾਰ fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} ਨਹੀਂਤਾਂ {
		SETOPT(CURLOPT_PROXY, ਨਿਰਾ);
	}


	ਜੇ (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	ਜੇ (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		ਜੇ (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, ਨਿਰਾ);
			SETOPT(CURLOPT_SSL_CTX_ਅੰਕੜੇ, ਨਿਰਾ);
		}
	} ਨਹੀਂਤਾਂ {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_OPENSSL
		ਜੇ (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_ਅੰਕੜੇ, f);
		}
#ਜੇ_ਅੰਤ
	}

	ਵਾਪਸ CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
ਸਥਿਰ bool
fetch_curl_initiate_fetch(ਬਣਤਰ curl_fetch_info *fetch, CURL *handle)
{
	CURLcode ਕੋਡ;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	ਕੋਡ = fetch_curl_ਸੈੱਟ_options(fetch);
	ਜੇ (ਕੋਡ != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		ਵਾਪਸ false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_ਜੋੜੋ_handle(fetch_curl_multi, fetch->curl_handle);
	ਪ੍ਰਤਿਜ੍ਞਾ(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	ਵਾਪਸ true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
ਸਥਿਰ CURL *fetch_curl_get_handle(lwc_string *host)
{
	ਬਣਤਰ cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	ਜੇ (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		ਮੁਕਤ(h);
	} ਨਹੀਂਤਾਂ {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	ਵਾਪਸ ret;
}


/**
 * Dispatch a single job
 */
ਸਥਿਰ bool fetch_curl_ਸ਼ੁਰੂ(ਖਾਲੀ *vfetch)
{
	ਬਣਤਰ curl_fetch_info *fetch = (ਬਣਤਰ curl_fetch_info*)vfetch;
	ਜੇ (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		ਵਾਪਸ false;
	}
	ਵਾਪਸ fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
ਸਥਿਰ ਖਾਲੀ fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#ਜੇ LIBCURL_ਸੰਸਕਰਣ_ਅੰਕ >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	ਵਾਪਸ;
#ਨਹੀਂਤਾਂ
	ਬਣਤਰ cache_handle *h = 0;
	ਪੂਰਨ c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	ਜੇ (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		ਵਾਪਸ;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(ਬਣਤਰ cache_handle, curl_handle_ring, c);
	ਜੇ (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		ਜੇ (curl_handle_ring != ਨਿਰਾ) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_ਅਗਲਾ;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} ਨਹੀਂਤਾਂ {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		ਵਾਪਸ;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (ਬਣਤਰ cache_handle*)ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_ਪਾਓ(curl_handle_ring, h);
#ਜੇ_ਅੰਤ
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
ਸਥਿਰ ਖਾਲੀ fetch_curl_stop(ਬਣਤਰ curl_fetch_info *f)
{
	CURLMcode codem;

	ਪ੍ਰਤਿਜ੍ਞਾ(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	ਜੇ (f->curl_handle) {
		/* remove from curl multi handle */
		codem = curl_multi_remove_handle(fetch_curl_multi,
				f->curl_handle);
		ਪ੍ਰਤਿਜ੍ਞਾ(codem == CURLM_OK);
		/* Put this curl handle into the cache if wanted. */
		fetch_curl_cache_handle(f->curl_handle, f->host);
		f->curl_handle = 0;
	}

	fetch_remove_from_queues(f->fetch_handle);
}


/**
 * Abort a fetch.
 */
ਸਥਿਰ ਖਾਲੀ fetch_curl_abort(ਖਾਲੀ *vf)
{
	ਬਣਤਰ curl_fetch_info *f = (ਬਣਤਰ curl_fetch_info *)vf;
	ਪ੍ਰਤਿਜ੍ਞਾ(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	ਜੇ (f->curl_handle) {
		ਜੇ (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->abort = true;
		} ਨਹੀਂਤਾਂ {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} ਨਹੀਂਤਾਂ {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
ਸਥਿਰ ਖਾਲੀ fetch_curl_free(ਖਾਲੀ *vf)
{
	ਬਣਤਰ curl_fetch_info *f = (ਬਣਤਰ curl_fetch_info *)vf;
	ਪੂਰਨ i;

	ਜੇ (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	ਮੁਕਤ(f->location);
	ਮੁਕਤ(f->cookie_string);
	ਮੁਕਤ(f->realm);
	ਜੇ (f->ਸਿਰਲੇਖਾਂ) {
		curl_slist_free_all(f->ਸਿਰਲੇਖਾਂ);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	ਲਈ (i = 0; i < MAX_CERT_ਡੂੰਘਾਈ; i++) {
		ਜੇ (f->cert_ਅੰਕੜੇ[i].cert != ਨਿਰਾ) {
			X509_free(f->cert_ਅੰਕੜੇ[i].cert);
		}
	}

	ਮੁਕਤ(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
ਸਥਿਰ bool fetch_curl_process_ਸਿਰਲੇਖਾਂ(ਬਣਤਰ curl_fetch_info *f)
{
	ਲੰਮਾ http_ਕੋਡ;
	CURLcode ਕੋਡ;
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;

	f->had_ਸਿਰਲੇਖਾਂ = true;

	ਜੇ (!f->http_ਕੋਡ) {
		ਕੋਡ = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_ਕੋਡ,
					 &f->http_ਕੋਡ);
		ਪ੍ਰਤਿਜ੍ਞਾ(ਕੋਡ == CURLE_OK);
		fetch_ਸੈੱਟ_http_ਕੋਡ(f->fetch_handle, f->http_ਕੋਡ);
	}
	http_ਕੋਡ = f->http_ਕੋਡ;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_ਕੋਡ);

	ਜੇ ((http_ਕੋਡ == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		ਸੁਨੇਹਾ.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&ਸੁਨੇਹਾ, f->fetch_handle);
		ਵਾਪਸ true;
	}

	/* handle HTTP redirects (3xx response codes) */
	ਜੇ (HTTP_RESPONSE_IS_3XX(http_ਕੋਡ) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		ਸੁਨੇਹਾ.type = FETCH_REDIRECT;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.redirect = f->location;
		fetch_send_callback(&ਸੁਨੇਹਾ, f->fetch_handle);
		ਵਾਪਸ true;
	}

	/* handle HTTP 401 (Authentication errors) */
	ਜੇ (http_ਕੋਡ == HTTP_RESPONSE_UNAUTHORIZED) {
		ਸੁਨੇਹਾ.type = FETCH_AUTH;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.auth.realm = f->realm;
		fetch_send_callback(&ਸੁਨੇਹਾ, f->fetch_handle);
		ਵਾਪਸ true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	ਜੇ (f->only_2xx &&
	    ਲੜੀ_ਤੁਲਨਾ(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_ਕੋਡ) == false)) {
		ਸੁਨੇਹਾ.type = FETCH_ERROR;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.error = messages_get("Not2xx");
		fetch_send_callback(&ਸੁਨੇਹਾ, f->fetch_handle);
		ਵਾਪਸ true;
	}

	ਜੇ (f->abort)
		ਵਾਪਸ true;

	ਵਾਪਸ false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
ਸਥਿਰ ਖਾਲੀ fetch_curl_ਪੂਰਾ(CURL *curl_handle, CURLcode ਨਤੀਜਾ)
{
	bool ਮੁਕੰਮਲ = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	ਬਣਤਰ curl_fetch_info *f;
	ਅੱਖਰ **_hideous_hack = (ਅੱਖਰ **) (ਖਾਲੀ *) &f;
	CURLcode ਕੋਡ;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	ਕੋਡ = curl_easy_getinfo(curl_handle, CURLINFO_ਨਿੱਜੀ, _hideous_hack);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਕੋਡ == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	ਜੇ ((abort_fetch == false) &&
	    (ਨਤੀਜਾ == CURLE_OK ||
	     ((ਨਤੀਜਾ == CURLE_WRITE_ERROR) && (f->stopped == false)))) {
		/* fetch completed normally or the server fed us a junk gzip
		 * stream (usually in the form of garbage at the end of the
		 * stream). Curl will have fed us all but the last chunk of
		 * decoded data, which is sad as, if we'd received the last
		 * chunk, too, we'd be able to render the whole object.
		 * As is, we'll just have to accept that the end of the
		 * object will be truncated in this case and leave it to
		 * the content handlers to cope.
		 */
		ਜੇ (f->stopped ||
		    (!f->had_ਸਿਰਲੇਖਾਂ &&	fetch_curl_process_ਸਿਰਲੇਖਾਂ(f))) {
			; /* redirect with no body or similar */
		} ਨਹੀਂਤਾਂ {
			ਮੁਕੰਮਲ = true;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (ਨਤੀਜਾ == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		ਜੇ (!f->had_ਸਿਰਲੇਖਾਂ && fetch_curl_process_ਸਿਰਲੇਖਾਂ(f))
			; /* redirect with partial body, or similar */
		ਨਹੀਂਤਾਂ {
			ਮੁਕੰਮਲ = true;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (ਨਤੀਜਾ == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} ਨਹੀਂਤਾਂ ਜੇ (ਨਤੀਜਾ == CURLE_SSL_PEER_CERTIFICATE ||
		   ਨਤੀਜਾ == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} ਨਹੀਂਤਾਂ {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", ਨਤੀਜਾ);
		error = true;
	}

	fetch_curl_stop(f);

	ਜੇ (f->ਭੇਜਿਆ_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	ਜੇ (abort_fetch) {
		; /* fetch was aborted: no callback */
	} ਨਹੀਂਤਾਂ ਜੇ (ਮੁਕੰਮਲ) {
		fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
		ਸੁਨੇਹਾ.type = FETCH_ਮੁਕੰਮਲ;
		fetch_send_callback(&ਸੁਨੇਹਾ, f->fetch_handle);
	} ਨਹੀਂਤਾਂ ਜੇ (cert) {
		/* user needs to validate certificate with issue */
		fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
		ਸੁਨੇਹਾ.type = FETCH_CERT_ERR;
		fetch_send_callback(&ਸੁਨੇਹਾ, f->fetch_handle);
	} ਨਹੀਂਤਾਂ ਜੇ (error) {
		fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
		ਚੋਣ (ਨਤੀਜਾ) {
		ਹਾਲਤ CURLE_SSL_CONNECT_ERROR:
			ਸੁਨੇਹਾ.type = FETCH_SSL_ERR;
			ਰੋਕੋ;

		ਹਾਲਤ CURLE_OPERATION_TIMEDOUT:
			ਸੁਨੇਹਾ.type = FETCH_TIMEDOUT;
			ਸੁਨੇਹਾ.ਅੰਕੜੇ.error = curl_easy_strerror(ਨਤੀਜਾ);
			ਰੋਕੋ;

		ਮੂਲ:
			ਸੁਨੇਹਾ.type = FETCH_ERROR;
			ਸੁਨੇਹਾ.ਅੰਕੜੇ.error = curl_easy_strerror(ਨਤੀਜਾ);
		}

		fetch_send_callback(&ਸੁਨੇਹਾ, f->fetch_handle);
	}

	fetch_free(f->fetch_handle);
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */
ਸਥਿਰ ਖਾਲੀ fetch_curl_poll(lwc_string *scheme_ignored)
{
	ਪੂਰਨ running, ਕਤਾਰ;
	CURLMcode codem;
	CURLMsg *curl_ਸੁਨੇਹਾ;

	ਜੇ (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_ਸੈੱਟ, write_fd_ਸੈੱਟ, exc_fd_ਸੈੱਟ;
		ਪੂਰਨ max_fd = -1;
		ਪੂਰਨ i;

		FD_ZERO(&read_fd_ਸੈੱਟ);
		FD_ZERO(&write_fd_ਸੈੱਟ);
		FD_ZERO(&exc_fd_ਸੈੱਟ);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_ਸੈੱਟ, &write_fd_ਸੈੱਟ,
				&exc_fd_ਸੈੱਟ, &max_fd);
		ਪ੍ਰਤਿਜ੍ਞਾ(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		ਲਈ (i = 0; i <= max_fd; i++) {
			bool ਪਠ = false;
			bool ਲਿਖ = false;
			bool error = false;

			ਜੇ (FD_ISSET(i, &read_fd_ਸੈੱਟ)) {
				ਪਠ = true;
			}
			ਜੇ (FD_ISSET(i, &write_fd_ਸੈੱਟ)) {
				ਲਿਖ = true;
			}
			ਜੇ (FD_ISSET(i, &exc_fd_ਸੈੱਟ)) {
				error = true;
			}
			ਜੇ (ਪਠ || ਲਿਖ || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      ਪਠ ? "read" : "    ",
				      ਲਿਖ ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	ਕਰੋ {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		ਜੇ (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			ਵਾਪਸ;
		}
	} ਜਦੋਂ (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_ਸੁਨੇਹਾ = curl_multi_info_read(fetch_curl_multi, &ਕਤਾਰ);
	ਜਦੋਂ (curl_ਸੁਨੇਹਾ) {
		ਚੋਣ (curl_ਸੁਨੇਹਾ->ਸੁਨੇਹਾ) {
			ਹਾਲਤ CURLMSG_ਪੂਰਾ:
				fetch_curl_ਪੂਰਾ(curl_ਸੁਨੇਹਾ->easy_handle,
						curl_ਸੁਨੇਹਾ->ਅੰਕੜੇ.ਨਤੀਜਾ);
				ਰੋਕੋ;
			ਮੂਲ:
				ਰੋਕੋ;
		}
		curl_ਸੁਨੇਹਾ = curl_multi_info_read(fetch_curl_multi, &ਕਤਾਰ);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
ਸਥਿਰ ਪੂਰਨ
fetch_curl_ਤਰੱਕੀ(ਖਾਲੀ *clientp,
		    NSCURL_ਤਰੱਕੀ_T dltotal,
		    NSCURL_ਤਰੱਕੀ_T dlnow,
		    NSCURL_ਤਰੱਕੀ_T ultotal,
		    NSCURL_ਤਰੱਕੀ_T ulnow)
{
	ਸਥਿਰ ਅੱਖਰ fetch_ਤਰੱਕੀ_ਬਫਰ[256]; /**< Progress buffer for cURL */
	ਬਣਤਰ curl_fetch_info *f = (ਬਣਤਰ curl_fetch_info *) clientp;
	uint64_t ਸਮਾਂ_now_ms;
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;

	ਜੇ (f->abort) {
		ਵਾਪਸ 0;
        }

	ਸੁਨੇਹਾ.type = FETCH_ਤਰੱਕੀ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਤਰੱਕੀ = fetch_ਤਰੱਕੀ_ਬਫਰ;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&ਸਮਾਂ_now_ms);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UPDATE_DELAY_MS (1000 / UPDATES_ਪ੍ਰਤੀ_ਦੂਜਾ)
	ਜੇ (ਸਮਾਂ_now_ms - f->ਆਖਰੀ_ਤਰੱਕੀ_update < UPDATE_DELAY_MS) {
		ਵਾਪਸ 0;
        }
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ UPDATE_DELAY_MS
	f->ਆਖਰੀ_ਤਰੱਕੀ_update = ਸਮਾਂ_now_ms;

	ਜੇ (dltotal > 0) {
		snprintf(fetch_ਤਰੱਕੀ_ਬਫਰ, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&ਸੁਨੇਹਾ, f->fetch_handle);
	} ਨਹੀਂਤਾਂ {
		snprintf(fetch_ਤਰੱਕੀ_ਬਫਰ, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&ਸੁਨੇਹਾ, f->fetch_handle);
	}

	ਵਾਪਸ 0;
}


/**
 * Format curl debug for nslog
 */
ਸਥਿਰ ਪੂਰਨ
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 ਅੱਖਰ *ਅੰਕੜੇ,
		 size_t ਮਾਪ,
		 ਖਾਲੀ *userptr)
{
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ s_infotype[CURLINFO_ਸਿਰਾ][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	ਚੋਣ(type) {
	ਹਾਲਤ CURLINFO_ਪਾਠ:
	ਹਾਲਤ CURLINFO_ਸਿਰਲੇਖ_ਬਾਹਰਲਾ:
	ਹਾਲਤ CURLINFO_ਸਿਰਲੇਖ_ਵਿੱਚ:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (ਪੂਰਨ)ਮਾਪ - 1, ਅੰਕੜੇ);
		ਰੋਕੋ;

	ਮੂਲ:
		ਰੋਕੋ;
	}
	ਵਾਪਸ 0;
}


ਸਥਿਰ curl_socket_t fetch_curl_socket_open(ਖਾਲੀ *clientp,
		curlsocktype purpose, ਬਣਤਰ curl_sockaddr *ਪਤਾ)
{
	(ਖਾਲੀ) clientp;
	(ਖਾਲੀ) purpose;

	ਵਾਪਸ (curl_socket_t) guit->fetch->socket_open(
			ਪਤਾ->family, ਪਤਾ->socktype,
			ਪਤਾ->protocol);
}

ਸਥਿਰ ਪੂਰਨ fetch_curl_socket_close(ਖਾਲੀ *clientp, curl_socket_t ਵਸਤੂ)
{
	(ਖਾਲੀ) clientp;

	ਵਾਪਸ guit->fetch->socket_close((ਪੂਰਨ) ਵਸਤੂ);
}

/**
 * Callback function for cURL.
 */
ਸਥਿਰ size_t fetch_curl_ਅੰਕੜੇ(ਅੱਖਰ *ਅੰਕੜੇ, size_t ਮਾਪ, size_t nmemb, ਖਾਲੀ *_f)
{
	ਬਣਤਰ curl_fetch_info *f = _f;
	CURLcode ਕੋਡ;
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;

	/* ensure we only have to get this information once */
	ਜੇ (!f->http_ਕੋਡ) {
		ਕੋਡ = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_ਕੋਡ,
					 &f->http_ਕੋਡ);
		ਪ੍ਰਤਿਜ੍ਞਾ(ਕੋਡ == CURLE_OK);
		fetch_ਸੈੱਟ_http_ਕੋਡ(f->fetch_handle, f->http_ਕੋਡ);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	ਜੇ (f->http_ਕੋਡ == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_ਕੋਡ = 0;
		ਵਾਪਸ ਮਾਪ * nmemb;
	}

	ਜੇ (f->abort || (!f->had_ਸਿਰਲੇਖਾਂ && fetch_curl_process_ਸਿਰਲੇਖਾਂ(f))) {
		f->stopped = true;
		ਵਾਪਸ 0;
	}

	/* send data to the caller */
	ਸੁਨੇਹਾ.type = FETCH_ਅੰਕੜੇ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *) ਅੰਕੜੇ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਮਾਪ * nmemb;
	fetch_send_callback(&ਸੁਨੇਹਾ, f->fetch_handle);

	ਜੇ (f->abort) {
		f->stopped = true;
		ਵਾਪਸ 0;
	}

	ਵਾਪਸ ਮਾਪ * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
ਸਥਿਰ size_t
fetch_curl_ਸਿਰਲੇਖ(ਅੱਖਰ *ਅੰਕੜੇ, size_t ਮਾਪ, size_t nmemb, ਖਾਲੀ *_f)
{
	ਬਣਤਰ curl_fetch_info *f = _f;
	ਪੂਰਨ i;
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	ਮਾਪ *= nmemb;

	ਜੇ (f->abort) {
		f->stopped = true;
		ਵਾਪਸ 0;
	}

	ਜੇ (f->ਭੇਜਿਆ_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	ਸੁਨੇਹਾ.type = FETCH_ਸਿਰਲੇਖ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *) ਅੰਕੜੇ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਮਾਪ;
	fetch_send_callback(&ਸੁਨੇਹਾ, f->fetch_handle);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SKIP_ST(o) ਲਈ (i = (o); i < (ਪੂਰਨ) ਮਾਪ && (ਅੰਕੜੇ[i] == ' ' || ਅੰਕੜੇ[i] == '\t'); i++)

	ਜੇ (12 < ਮਾਪ && strncasecmp(ਅੰਕੜੇ, "Location:", 9) == 0) {
		/* extract Location header */
		ਮੁਕਤ(f->location);
		f->location = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ);
		ਜੇ (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			ਵਾਪਸ ਮਾਪ;
		}
		SKIP_ST(9);
		strncpy(f->location, ਅੰਕੜੇ + i, ਮਾਪ - i);
		f->location[ਮਾਪ - i] = '\0';
		ਲਈ (i = ਮਾਪ - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} ਨਹੀਂਤਾਂ ਜੇ (15 < ਮਾਪ && strncasecmp(ਅੰਕੜੇ, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		ਜੇ (i < (ਪੂਰਨ)ਮਾਪ && '0' <= ਅੰਕੜੇ[i] && ਅੰਕੜੇ[i] <= '9')
			f->content_ਲੰਮਾਈ = atol(ਅੰਕੜੇ + i);
	} ਨਹੀਂਤਾਂ ਜੇ (17 < ਮਾਪ && strncasecmp(ਅੰਕੜੇ, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		ਜਦੋਂ (i < (ਪੂਰਨ) ਮਾਪ - 5 &&
				strncasecmp(ਅੰਕੜੇ + i, "realm", 5))
			i++;
		ਜਦੋਂ (i < (ਪੂਰਨ) ਮਾਪ - 1 && ਅੰਕੜੇ[++i] != '"')
			/* */;
		i++;

		ਜੇ (i < (ਪੂਰਨ) ਮਾਪ) {
			size_t ਸਿਰਾ = i;

			ਜਦੋਂ (ਸਿਰਾ < ਮਾਪ && ਅੰਕੜੇ[ਸਿਰਾ] != '"')
				++ਸਿਰਾ;

			ਜੇ (ਸਿਰਾ < ਮਾਪ) {
				ਮੁਕਤ(f->realm);
				f->realm = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਸਿਰਾ - i + 1);
				ਜੇ (f->realm != ਨਿਰਾ) {
					strncpy(f->realm, ਅੰਕੜੇ + i, ਸਿਰਾ - i);
					f->realm[ਸਿਰਾ - i] = '\0';
				}
			}
		}
	} ਨਹੀਂਤਾਂ ਜੇ (11 < ਮਾਪ && strncasecmp(ਅੰਕੜੇ, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_ਸੈੱਟ_cookie(f->fetch_handle, &ਅੰਕੜੇ[i]);
	}

	ਵਾਪਸ ਮਾਪ;
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SKIP_ST
}

ਸਥਿਰ ਪੂਰਨ fetch_curl_fdset(lwc_string *scheme, fd_set *read_ਸੈੱਟ,
			    fd_set *write_ਸੈੱਟ, fd_set *error_ਸੈੱਟ)
{
	CURLMcode ਕੋਡ;
	ਪੂਰਨ maxfd = -1;

	ਕੋਡ = curl_multi_fdset(fetch_curl_multi,
				read_ਸੈੱਟ,
				write_ਸੈੱਟ,
				error_ਸੈੱਟ,
				&maxfd);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਕੋਡ == CURLM_OK);

	ਵਾਪਸ maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_ਰਜਿਸਟਰ(ਖਾਲੀ)
{
	CURLcode ਕੋਡ;
	curl_ਸੰਸਕਰਣ_info_ਅੰਕੜੇ *ਅੰਕੜੇ;
	ਪੂਰਨ i;
	lwc_string *scheme;
	ਅਚਲ ਬਣਤਰ fetcher_operation_ਸਾਰਣੀ fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.ਤਿਆਰੀ = fetch_curl_ਤਿਆਰੀ,
		.ਸ਼ੁਰੂ = fetch_curl_ਸ਼ੁਰੂ,
		.abort = fetch_curl_abort,
		.ਮੁਕਤ = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#ਜੇ LIBCURL_ਸੰਸਕਰਣ_ਅੰਕ >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, ਨਿਰਾ, ਨਿਰਾ);
	ਜੇ (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} ਨਹੀਂਤਾਂ {
		curl_with_openssl = false;
	}
#ਜੇ_ਅੰਤ

	NSLOG(netsurf, INFO, "curl_version %s", curl_ਸੰਸਕਰਣ());

	ਕੋਡ = curl_global_init(CURL_GLOBAL_ALL);
	ਜੇ (ਕੋਡ != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	ਜੇ (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}

#ਜੇ LIBCURL_ਸੰਸਕਰਣ_ਅੰਕ >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		ਪੂਰਨ maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SETOPT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SETOPT(option, ਮੁੱਲ) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, ਮੁੱਲ);	\
	ਜੇ (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		ਜਾਓ curl_multi_setopt_failed;				\
	}

		SETOPT(CURLMOPT_MAXCONNECTS, (ਲੰਮਾ)maxconnects);
		SETOPT(CURLMOPT_MAX_ਕੁੱਲ_CONNECTIONS, (ਲੰਮਾ)maxconnects);
		SETOPT(CURLMOPT_MAX_HOST_CONNECTIONS, (ਲੰਮਾ)nsoption_int(max_fetchers_ਪ੍ਰਤੀ_host));
	}
#ਜੇ_ਅੰਤ

	/* Create a curl easy handle with the options that are common to all
	 *  fetches.
	 */
	fetch_blank_curl = curl_easy_init();
	ਜੇ (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SETOPT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SETOPT(option, ਮੁੱਲ) \
	ਕੋਡ = curl_easy_setopt(fetch_blank_curl, option, ਮੁੱਲ);	\
	ਜੇ (ਕੋਡ != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		ਜਾਓ curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_ਬਫਰ);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	ਜੇ (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} ਨਹੀਂਤਾਂ {
		SETOPT(CURLOPT_VERBOSE, 1);
	}

	/* Currently we explode if curl uses HTTP2, so force 1.1. */
	SETOPT(CURLOPT_HTTP_ਸੰਸਕਰਣ, CURL_HTTP_ਸੰਸਕਰਣ_1_1);

	SETOPT(CURLOPT_WRITEFUNCTION, fetch_curl_ਅੰਕੜੇ);
	SETOPT(CURLOPT_HEADERFUNCTION, fetch_curl_ਸਿਰਲੇਖ);
	SETOPT(NSCURLOPT_ਤਰੱਕੀ_FUNCTION, fetch_curl_ਤਰੱਕੀ);
	SETOPT(CURLOPT_NOPROGRESS, 0L);
	SETOPT(CURLOPT_USERAGENT, user_agent_string());
	SETOPT(CURLOPT_ENCODING, "gzip");
	SETOPT(CURLOPT_ਨੀਵਾਂ_ਰਫਤਾਰ_ਹੱਦਬੰਦੀ, 1L);
	SETOPT(CURLOPT_ਨੀਵਾਂ_ਰਫਤਾਰ_ਸਮਾਂ, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (ਲੰਮਾ)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	ਜੇ (nsoption_charp(ca_bundle) &&
	    strcmp(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	ਜੇ (nsoption_charp(ca_ਰਸਤਾ) && strcmp(nsoption_charp(ca_ਰਸਤਾ), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_ਰਸਤਾ));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_ਰਸਤਾ));
	}

#ਜੇ LIBCURL_ਸੰਸਕਰਣ_ਅੰਕ < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	ਕੋਡ = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, ਨਿਰਾ);
	ਜੇ (ਕੋਡ != CURLE_OK) {
		curl_with_openssl = false;
	} ਨਹੀਂਤਾਂ {
		curl_with_openssl = true;
	}
#ਜੇ_ਅੰਤ

	ਜੇ (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#ਜੇ LIBCURL_ਸੰਸਕਰਣ_ਅੰਕ >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		ਕੋਡ = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		ਜੇ (ਕੋਡ != CURLE_OK && ਕੋਡ != CURLE_NOT_BUILT_ਵਿੱਚ)
			ਜਾਓ curl_easy_setopt_failed;
#ਜੇ_ਅੰਤ
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	ਅੰਕੜੇ = curl_ਸੰਸਕਰਣ_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	ਜੇ (curl_fetch_ssl_hashmap == ਨਿਰਾ) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਲਈ (i = 0; ਅੰਕੜੇ->protocols[i]; i++) {
		ਜੇ (strcmp(ਅੰਕੜੇ->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} ਨਹੀਂਤਾਂ ਜੇ (strcmp(ਅੰਕੜੇ->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} ਨਹੀਂਤਾਂ {
			/* Ignore non-http(s) protocols */
			ਜਾਰੀ;
		}

		ਜੇ (fetcher_ਜੋੜੋ(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      ਅੰਕੜੇ->protocols[i]);
		}
	}

	ਵਾਪਸ NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	ਵਾਪਸ NSERROR_INIT_FAILED;

#ਜੇ LIBCURL_ਸੰਸਕਰਣ_ਅੰਕ >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	ਵਾਪਸ NSERROR_INIT_FAILED;
#ਜੇ_ਅੰਤ
}
