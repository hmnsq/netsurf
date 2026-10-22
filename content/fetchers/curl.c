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
#ଅନ୍ତର୍ଭାଵ "utils/inet.h"

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>

#ଅନ୍ତର୍ଭାଵ <libwapcaplet/libwapcaplet.h>
#ଅନ୍ତର୍ଭାଵ <nsutils/time.h>

#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/hashmap.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/ring.h"
#ଅନ୍ତର୍ଭାଵ "utils/useragent.h"
#ଅନ୍ତର୍ଭାଵ "utils/file.h"
#ଅନ୍ତର୍ଭାଵ "utils/string.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/fetch.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/misc.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"

#ଅନ୍ତର୍ଭାଵ "content/fetch.h"
#ଅନ୍ତର୍ଭାଵ "content/fetchers.h"
#ଅନ୍ତର୍ଭାଵ "content/fetchers/curl.h"
#ଅନ୍ତର୍ଭାଵ "content/urldb.h"

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

#ଅନ୍ତର୍ଭାଵ <openssl/ssl.h>
#ଅନ୍ତର୍ଭାଵ <openssl/x509v3.h>

#ଅନ୍ୟଥା /* WITH_OPENSSL */

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଅକ୍ଷର X509;

ସ୍ଥିର ଶୂନ୍ୟ X509_free(X509 *cert)
{
	ମୁକ୍ତ(cert);
}

#endif /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
ସ୍ଥିର uint32_t
curl_fetch_ssl_ଚାବି_hash(ଶୂନ୍ୟ *ଚାବି)
{
	nsurl *url = ଚାବି;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *port = nsurl_get_component(url, NSURL_PORT);
	uint32_t hash;

	ଯଦି (port == ଖାଲି)
		port = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_ମୂଲ୍ୟ(hostname) ^ lwc_string_hash_ମୂଲ୍ୟ(port);

	lwc_string_unref(hostname);
	lwc_string_unref(port);

	ଫେରନ୍ତୁ hash;
}

/* We only compare the hostname and port */
ସ୍ଥିର bool
curl_fetch_ssl_ଚାବି_eq(ଶୂନ୍ୟ *key1, ଶୂନ୍ୟ *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_PORT);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_PORT);
	bool iseq = false;

	ଯଦି (port1 == ଖାଲି)
		port1 = lwc_string_ref(corestring_lwc_443);
	ଯଦି (port2 == ଖାଲି)
		port2 = lwc_string_ref(corestring_lwc_443);

	ଯଦି (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		ଯାଅ out;

	iseq = false;
	ଯଦି (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		ଯାଅ out;

out:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	ଫେରନ୍ତୁ iseq;
}

ସ୍ଥିର ଶୂନ୍ୟ *
curl_fetch_ssl_ମୂଲ୍ୟ_alloc(ଶୂନ୍ୟ *ଚାବି)
{
	ସଂରଚନା cert_chain *out;

	ଯଦି (cert_chain_alloc(0, &out) != NSERROR_OK) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ out;
}

ସ୍ଥିର ଶୂନ୍ୟ
curl_fetch_ssl_ମୂଲ୍ୟ_destroy(ଶୂନ୍ୟ *ମୂଲ୍ୟ)
{
	ସଂରଚନା cert_chain *chain = ମୂଲ୍ୟ;
	ଯଦି (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

ସ୍ଥିର hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.ଚାବି_clone = (hashmap_ଚାବି_clone_t)nsurl_ref,
	.ଚାବି_destroy = (hashmap_ଚାବି_destroy_t)nsurl_unref,
	.ଚାବି_eq = curl_fetch_ssl_ଚାବି_eq,
	.ଚାବି_hash = curl_fetch_ssl_ଚାବି_hash,
	.ମୂଲ୍ୟ_alloc = curl_fetch_ssl_ମୂଲ୍ୟ_alloc,
	.ମୂଲ୍ୟ_destroy = curl_fetch_ssl_ମୂଲ୍ୟ_destroy,
};

ସ୍ଥିର hashmap_t *curl_fetch_ssl_hashmap = ଖାଲି;

/** SSL certificate info */
ସଂରଚନା cert_info {
	X509 *cert;		/**< Pointer to certificate */
	long err;		/**< OpenSSL error code */
};

#ଯଦି LIBCURL_VERSION_ଅଙ୍କ >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_XFERINFOFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_XFERINFODATA
#define NSCURL_PROGRESS_T curl_off_t
#ଅନ୍ୟଥା
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_PROGRESSFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_PROGRESSDATA
#define NSCURL_PROGRESS_T ଦ୍ୱିଗୁଣ
#endif

#ଯଦି LIBCURL_VERSION_ଅଙ୍କ >= 0x073800 /* 7.56.0 depricated curl_formadd */
#define NSCURL_POSTDATA_T curl_mime
#define NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#define NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#ଅନ୍ୟଥା
#define NSCURL_POSTDATA_T ସଂରଚନା curl_httppost
#define NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#define NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#endif

/** Information for a single fetch. */
ସଂରଚନା curl_fetch_info {
	ସଂରଚନା fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool sent_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_headers;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	ସଂରଚନା curl_slist *headers;	/**< List of request headers. */
	ଅକ୍ଷର *location;		/**< Response Location header, or 0. */
	unsigned long content_length;	/**< Response Content-Length, or 0. */
	ଅକ୍ଷର *cookie_string;	/**< Cookie string for this fetch */
	ଅକ୍ଷର *realm;		/**< HTTP Auth Realm */
	ସଂରଚନା fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	long http_code; /**< HTTP result code from cURL. */

	uint64_t last_progress_update;	/**< Time of last progress update */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cert_depth; /**< deepest certificate in use */
	ସଂରଚନା cert_info cert_data[MAX_CERT_DEPTH]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
ସଂରଚନା cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	ସଂରଚନା cache_handle *r_prev; /**< Previous cached handle in ring. */
	ସଂରଚନା cache_handle *r_next; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
ସ୍ଥିର CURL *fetch_blank_curl;

/** Ring of cached handles */
ସ୍ଥିର ସଂରଚନା cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
ସ୍ଥିର bool curl_with_openssl;

/** Error buffer for cURL. */
ସ୍ଥିର ଅକ୍ଷର fetch_error_buffer[CURL_ERROR_SIZE];

/** Proxy authentication details. */
ସ୍ଥିର ଅକ୍ଷର fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
ସ୍ଥିର bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
ସ୍ଥିର bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_data(scheme));
	curl_fetchers_registered++;
	ଫେରନ୍ତୁ true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
ସ୍ଥିର ଶୂନ୍ୟ fetch_curl_finalise(lwc_string *scheme)
{
	ସଂରଚନା cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_data(scheme));
	ଯଦି (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		ଯଦି (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = ଖାଲି;
	}

	/* Free anything remaining in the cached curl handle ring */
	ଯେତେବେଳେ (curl_handle_ring != ଖାଲି) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
		curl_easy_cleanup(h->handle);
		ମୁକ୍ତ(h);
	}
}


/**
 * Check if this fetcher can fetch a url.
 *
 * \param url The url to check.
 * \return true if the fetcher supports the url else false.
 */
ସ୍ଥିର bool fetch_curl_can_fetch(ଅଚଳ nsurl *url)
{
	ଫେରନ୍ତୁ nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
ସ୍ଥିର ସଂରଚନା fetch_postdata *
fetch_curl_alloc_postdata(ଅଚଳ ଅକ୍ଷର *post_urlenc,
			  ଅଚଳ ସଂରଚନା fetch_multipart_data *post_multipart)
{
	ସଂରଚନା fetch_postdata *postdata;
	postdata = calloc(1, ଆକାର(ସଂରଚନା fetch_postdata));
	ଯଦି (postdata != ଖାଲି) {

		ଯଦି (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->data.urlenc = strdup(post_urlenc);
			ଯଦି (postdata->data.urlenc == ଖାଲି) {
				ମୁକ୍ତ(postdata);
				postdata = ଖାଲି;
			}
		} ଅନ୍ୟଥା ଯଦି (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->data.multipart = fetch_multipart_data_clone(post_multipart);
			ଯଦି (postdata->data.multipart == ଖାଲି) {
				ମୁକ୍ତ(postdata);
				postdata = ଖାଲି;
			}
		} ଅନ୍ୟଥା {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	ଫେରନ୍ତୁ postdata;
}

/**
 * free postdata
 */
ସ୍ଥିର ଶୂନ୍ୟ fetch_curl_free_postdata(ସଂରଚନା fetch_postdata *postdata)
{
	ଯଦି (postdata != ଖାଲି) {
		ବାଛ (postdata->type) {
		ସ୍ଥିତି FETCH_POSTDATA_NONE:
			ଭାଙ୍ଗ;
		ସ୍ଥିତି FETCH_POSTDATA_URLENC:
			ମୁକ୍ତ(postdata->data.urlenc);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି FETCH_POSTDATA_MULTIPART:
			fetch_multipart_data_destroy(postdata->data.multipart);
			ଭାଙ୍ଗ;
		}

		ମୁକ୍ତ(postdata);
	}
}

/**
 *construct a new fetch structure
 */
ସ୍ଥିର ସଂରଚନା curl_fetch_info *fetch_alloc(ଶୂନ୍ୟ)
{
	ସଂରଚନା curl_fetch_info *fetch;
	fetch = malloc(ଆକାର (*fetch));
	ଯଦି (fetch == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	fetch->curl_handle = ଖାଲି;
	fetch->sent_ssl_chain = false;
	fetch->had_headers = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->headers = ଖାଲି;
	fetch->url = ଖାଲି;
	fetch->host = ଖାଲି;
	fetch->location = ଖାଲି;
	fetch->content_length = 0;
	fetch->http_code = 0;
	fetch->cookie_string = ଖାଲି;
	fetch->realm = ଖାଲି;
	fetch->last_progress_update = 0;
	fetch->postdata = ଖାଲି;
	fetch->curl_postdata = ଖାଲି;

	/* Clear certificate chain data */
	memset(fetch->cert_data, 0, ଆକାର(fetch->cert_data));
	fetch->cert_depth = -1;

	ଫେରନ୍ତୁ fetch;
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
ସ୍ଥିର ଶୂନ୍ୟ *
fetch_curl_setup(ସଂରଚନା fetch *parent_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ଅଚଳ ଅକ୍ଷର *post_urlenc,
		 ଅଚଳ ସଂରଚନା fetch_multipart_data *post_multipart,
		 ଅଚଳ ଅକ୍ଷର **headers)
{
	ସଂରଚନା curl_fetch_info *fetch;
	ସଂରଚନା curl_slist *slist;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	fetch = fetch_alloc();
	ଯଦି (fetch == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = parent_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	ଯଦି (fetch->host == ଖାଲି) {
		ଯାଅ failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	ଯଦି (fetch->postdata == ଖାଲି) {
		ଯାଅ failed;
	}

#define APPEND(list, ମୂଲ୍ୟ) \
	slist = curl_slist_append(list, ମୂଲ୍ୟ);		\
	ଯଦି (slist == ଖାଲି)				\
		ଯାଅ failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->headers, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->headers, "Expect:");

	ଯଦି ((nsoption_charp(accept_language) != ଖାଲି) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		ଅକ୍ଷର s[80];
		snprintf(s, ଆକାର s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[ଆକାର s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	ଯଦି (nsoption_charp(accept_charset) != ଖାଲି &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		ଅକ୍ଷର s[80];
		snprintf(s, ଆକାର s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[ଆକାର s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	ଯଦି (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->headers, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	ପାଇଁ (i = 0; headers[i] != ଖାଲି; i++) {
		APPEND(fetch->headers, headers[i]);
	}

	ଫେରନ୍ତୁ fetch;

#undef APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->headers);
	ମୁକ୍ତ(fetch);
	ଫେରନ୍ତୁ ଖାଲି;
}


#ifdef WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
ସ୍ଥିର ସଂରଚନା cert_chain *
fetch_curl_get_cached_chain(ସଂରଚନା curl_fetch_info *f)
{
	ସଂରଚନା cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	ଯଦି (chain == ଖାଲି) {
		chain = hashmap_ଭର୍ତ୍ତି(curl_fetch_ssl_hashmap, f->url);
	}

	ଫେରନ୍ତୁ chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
ସ୍ଥିର ଶୂନ୍ୟ
fetch_curl_store_certs_in_cache(ସଂରଚନା curl_fetch_info *f)
{
	size_t depth;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_DEPTH];
	ସଂରଚନା cert_chain chain, *cached_chain;
	ସଂରଚନା cert_info *certs;

	memset(&chain, 0, ଆକାର(chain));

	certs = f->cert_data;
	chain.depth = f->cert_depth + 1; /* 0 indexed certificate depth */

	ପାଇଁ (depth = 0; depth < chain.depth; depth++) {
		ଯଦି (certs[depth].cert == ଖାଲି) {
			/* This certificate is missing, skip it */
			chain.certs[depth].err = SSL_CERT_ERR_CERT_MISSING;
			ଜାରି;
		}

		/* error code (if any) */
		ବାଛ (certs[depth].err) {
		ସ୍ଥିତି X509_V_OK:
			chain.certs[depth].err = SSL_CERT_ERR_OK;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			/* fallthrough */
		ସ୍ଥିତି X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_ଚାବି:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_ISSUER;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		ସ୍ଥିତି X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		ସ୍ଥିତି X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		ସ୍ଥିତି X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_SIG;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		ସ୍ଥିତି X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_YOUNG;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		ସ୍ଥିତି X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_OLD;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			chain.certs[depth].err = SSL_CERT_ERR_SELF_SIGNED;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			chain.certs[depth].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି X509_V_ERR_CERT_REVOKED:
			chain.certs[depth].err = SSL_CERT_ERR_REVOKED;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[depth].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			ଭାଙ୍ଗ;

		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			chain.certs[depth].err = SSL_CERT_ERR_UNKNOWN;
			ଭାଙ୍ଗ;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[depth].cert);
		BIO_get_mem_ptr(mem, &buf[depth]);
		(ଶୂନ୍ୟ) BIO_set_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[depth].der = (uint8_t *)buf[depth]->data;
		chain.certs[depth].der_length = buf[depth]->length;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	ଯଦି (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	ପାଇଁ (depth = 0; depth < chain.depth; depth++) {
		ଯଦି (chain.certs[depth].err == SSL_CERT_ERR_CERT_MISSING) {
			ଜାରି;
		}
		ଯଦି (buf[depth] != ଖାଲି) {
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
fetch_curl_verify_callback(ପୂର୍ଣ୍ଣସଂଖ୍ୟା verify_ok, X509_STORE_CTX *x509_ctx)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth;
	ସଂରଚନା curl_fetch_info *fetch;

	depth = X509_STORE_CTX_get_error_depth(x509_ctx);
	fetch = X509_STORE_CTX_get_app_data(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	ଯଦି (depth >= MAX_CERT_DEPTH) {
		X509_STORE_CTX_set_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		ଫେରନ୍ତୁ 0;
	}

	/* record the max depth */
	ଯଦି (depth > fetch->cert_depth) {
		fetch->cert_depth = depth;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	ଯଦି (!fetch->cert_data[depth].cert) {
		fetch->cert_data[depth].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_up_ref(fetch->cert_data[depth].cert);
		fetch->cert_data[depth].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	ଯଦି (depth > 0) {
		verify_ok = 1;
	} ଅନ୍ୟଥା {
		/* search for deeper certificates in the chain with errors */
		ପାଇଁ (depth = fetch->cert_depth; depth > 0; depth--) {
			ଯଦି (fetch->cert_data[depth].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_set_error(x509_ctx, fetch->cert_data[depth].err);
			}
		}
	}

	ଫେରନ୍ତୁ verify_ok;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, ଶୂନ୍ୟ *parm)
{
	ସଂରଚନା curl_fetch_info *f = (ସଂରଚନା curl_fetch_info *) parm;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_set_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_data(f->host),
					 lwc_string_length(f->host));

	/* Store fetch struct in context for verify callback */
	ଯଦି (ok) {
		ok = X509_STORE_CTX_set_app_data(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	ଯଦି (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_in_cache(f);

	ଫେରନ୍ତୁ ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
ସ୍ଥିର CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, ଶୂନ୍ୟ *_sslctx, ଶୂନ୍ୟ *parm)
{
	ସଂରଚନା curl_fetch_info *f = (ସଂରଚନା curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	long options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_set_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	ଯଦି (f->downgrade_tls) {
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

	ଫେରନ୍ତୁ CURLE_OK;
}


#endif /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
ସ୍ଥିର ଶୂନ୍ୟ
fetch_curl_report_certs_upstream(ସଂରଚନା curl_fetch_info *f)
{
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	ସଂରଚନା cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	ଯଦି (chain != ଖାଲି) {
		ସନ୍ଦେଶ.type = FETCH_CERTS;
		ସନ୍ଦେଶ.data.chain = chain;

		fetch_send_callback(&ସନ୍ଦେଶ, f->fetch_handle);
	}

	f->sent_ssl_chain = true;
}

#ଯଦି LIBCURL_VERSION_ଅଙ୍କ >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
ସଂରଚନା curl_mime_ctx {
	ଅକ୍ଷର *buffer;
	curl_off_t size;
	curl_off_t position;
};

ସ୍ଥିର size_t mime_data_read_callback(ଅକ୍ଷର *buffer, size_t size, size_t nitems, ଶୂନ୍ୟ *arg)
{
	ସଂରଚନା curl_mime_ctx *mctx = (ସଂରଚନା curl_mime_ctx *) arg;
	curl_off_t sz = mctx->size - mctx->position;

	nitems *= size;
	ଯଦି(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	ଯଦି(sz) {
		memcpy(buffer, mctx->buffer + mctx->position, sz);
	}
	mctx->position += sz;
	ଫେରନ୍ତୁ sz;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା mime_data_seek_callback(ଶୂନ୍ୟ *arg, curl_off_t offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା origin)
{
	ସଂରଚନା curl_mime_ctx *mctx = (ସଂରଚନା curl_mime_ctx *) arg;

	ବାଛ(origin) {
	ସ୍ଥିତି SEEK_END:
		offset += mctx->size;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି SEEK_CUR:
		offset += mctx->position;
		ଭାଙ୍ଗ;
	}

	ଯଦି(offset < 0) {
		ଫେରନ୍ତୁ CURL_SEEKFUNC_FAIL;
	}
	mctx->position = offset;
	ଫେରନ୍ତୁ CURL_SEEKFUNC_OK;
}

ସ୍ଥିର ଶୂନ୍ୟ mime_data_free_callback(ଶୂନ୍ୟ *arg)
{
	ସଂରଚନା curl_mime_ctx *mctx = (ସଂରଚନା curl_mime_ctx *) arg;
	ମୁକ୍ତ(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
ସ୍ଥିର curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    ଅଚଳ ସଂରଚନା fetch_multipart_data *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode code = CURLE_OK;
	size_t ମୂଲ୍ୟ_len;

	cmime = curl_mime_init(chandle);
	ଯଦି (cmime == ଖାଲି) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* iterate post data */
	ପାଇଁ (; multipart != ଖାଲି; multipart = multipart->next) {
		part = curl_mime_addpart(cmime);
		ଯଦି (part == ଖାଲି) {
			ଯାଅ convert_failed;
		}

		code = curl_mime_name(part, multipart->name);
		ଯଦି (code != CURLE_OK) {
			ଯାଅ convert_failed;
		}

		ମୂଲ୍ୟ_len = strlen(multipart->ମୂଲ୍ୟ);

		ଯଦି (multipart->file && ମୂଲ୍ୟ_len==0) {
			/* file entries with no filename require special handling */
			code=curl_mime_data(part, multipart->ମୂଲ୍ୟ, ମୂଲ୍ୟ_len);
			ଯଦି (code != CURLE_OK) {
				ଯାଅ convert_failed;
			}

			code = curl_mime_filename(part, "");
			ଯଦି (code != CURLE_OK) {
				ଯାଅ convert_failed;
			}

			code = curl_mime_type(part, "application/octet-stream");
			ଯଦି (code != CURLE_OK) {
				ଯାଅ convert_failed;
			}

		} ଅନ୍ୟଥା ଯଦି(multipart->file) {
			/* file entry */
			nserror ret;
			ଅକ୍ଷର *leafname = ଖାଲି;
			ଅକ୍ଷର *mimetype = ଖାଲି;

			code = curl_mime_filedata(part, multipart->rawfile);
			ଯଦି (code != CURLE_OK) {
				ଯାଅ convert_failed;
			}

			ret = guit->file->basename(multipart->ମୂଲ୍ୟ, &leafname, ଖାଲି);
			ଯଦି (ret != NSERROR_OK) {
				ଯାଅ convert_failed;
			}
			code = curl_mime_filename(part, leafname);
			ମୁକ୍ତ(leafname);
			ଯଦି (code != CURLE_OK) {
				ଯାଅ convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->ମୂଲ୍ୟ);
			ଯଦି (mimetype == ଖାଲି) {
				mimetype=strdup("text/plain");
			}
			ଯଦି (mimetype == ଖାଲି) {
				ଯାଅ convert_failed;
			}
			code = curl_mime_type(part, mimetype);
			ମୁକ୍ତ(mimetype);
			ଯଦି (code != CURLE_OK) {
				ଯାଅ convert_failed;
			}

		} ଅନ୍ୟଥା {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			ସଂରଚନା curl_mime_ctx *cb_ctx;
			cb_ctx = malloc(ଆକାର(ସଂରଚନା curl_mime_ctx));
			ଯଦି (cb_ctx == ଖାଲି) {
				ଯାଅ convert_failed;
			}
			cb_ctx->buffer = multipart->ମୂଲ୍ୟ;
			cb_ctx->size = ମୂଲ୍ୟ_len;
			cb_ctx->position = 0;
			code = curl_mime_data_cb(part,
						 ମୂଲ୍ୟ_len,
						 mime_data_read_callback,
						 mime_data_seek_callback,
						 mime_data_free_callback,
						 cb_ctx);
			ଯଦି (code != CURLE_OK) {
				ମୁକ୍ତ(cb_ctx);
				ଯାଅ convert_failed;
			}
		}
	}

	ଫେରନ୍ତୁ cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", code);
	curl_mime_free(cmime);
	ଫେରନ୍ତୁ ଖାଲି;
}

#ଅନ୍ୟଥା /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
ସ୍ଥିର ସଂରଚନା curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    ଅଚଳ ସଂରଚନା fetch_multipart_data *control)
{
	ସଂରଚନା curl_httppost *post = ଖାଲି, *last = ଖାଲି;
	CURLFORMcode code;
	nserror ret;

	ପାଇଁ (; control; control = control->next) {
		ଯଦି (control->file) {
			ଅକ୍ଷର *leafname = ଖାଲି;
			ret = guit->file->basename(control->ମୂଲ୍ୟ, &leafname, ଖାଲି);
			ଯଦି (ret != NSERROR_OK) {
				ଜାରି;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			ଯଦି (control->ମୂଲ୍ୟ[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				ସ୍ଥିର ଅକ୍ଷର buf;

				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_BUFFER, control->ମୂଲ୍ୟ,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_END);
				ଯଦି (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", code,
					      control->name);
			} ଅନ୍ୟଥା {
				ଅକ୍ଷର *mimetype = guit->fetch->mimetype(control->ମୂଲ୍ୟ);
				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_END);
				ଯଦି (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      code,
					      control->name,
					      control->ମୂଲ୍ୟ);
				ମୁକ୍ତ(mimetype);
			}
			ମୁକ୍ତ(leafname);
		} ଅନ୍ୟଥା {
			code = curl_formadd(&post, &last,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->ମୂଲ୍ୟ,
					    CURLFORM_END);
			ଯଦି (code != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", code,
				      control->name, control->ମୂଲ୍ୟ);
		}
	}

	ଫେରନ୍ତୁ post;
}

#endif  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
ସ୍ଥିର CURLcode fetch_curl_set_postdata(ସଂରଚନା curl_fetch_info *f)
{
	CURLcode code = CURLE_OK;

#undef SETOPT
#define SETOPT(option, ମୂଲ୍ୟ) { \
	code = curl_easy_setopt(f->curl_handle, option, ମୂଲ୍ୟ);	\
	ଯଦି (code != CURLE_OK)					\
		ଫେରନ୍ତୁ code;					\
	}

	ବାଛ (f->postdata->type) {
	ସ୍ଥିତି FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, ଖାଲି);
		SETOPT(NSCURL_POSTDATA_CURLOPT, ଖାଲି);
		SETOPT(CURLOPT_HTTPGET, 1L);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, ଖାଲି);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->data.urlenc);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, ଖାଲି);
		SETOPT(CURLOPT_HTTPGET, 0L);
		ଯଦି (f->curl_postdata == ଖାଲି) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->data.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ code;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
ସ୍ଥିର CURLcode fetch_curl_set_options(ସଂରଚନା curl_fetch_info *f)
{
	CURLcode code;
	ଅଚଳ ଅକ୍ଷର *auth;

#undef SETOPT
#define SETOPT(option, ମୂଲ୍ୟ) { \
	code = curl_easy_setopt(f->curl_handle, option, ମୂଲ୍ୟ);	\
	ଯଦି (code != CURLE_OK)					\
		ଫେରନ୍ତୁ code;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_PRIVATE, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_PROGRESS_DATA, f);
	SETOPT(CURLOPT_HTTPHEADER, f->headers);
	code = fetch_curl_set_postdata(f);
	ଯଦି (code != CURLE_OK) {
		ଫେରନ୍ତୁ code;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	ଯଦି (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} ଅନ୍ୟଥା {
		SETOPT(CURLOPT_COOKIE, ଖାଲି);
	}

	ଯଦି ((auth = urldb_get_auth_details(f->url, ଖାଲି)) != ଖାଲି) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} ଅନ୍ୟଥା {
		SETOPT(CURLOPT_USERPWD, ଖାଲି);
	}

	/* set up proxy options */
	ଯଦି (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != ଖାଲି) &&
	    (strncmp(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (long) nsoption_int(http_proxy_port));

#ଯଦି LIBCURL_VERSION_ଅଙ୍କ >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#endif

		ଯଦି (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(long) CURLAUTH_BASIC :
					(long) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					ଆକାର fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} ଅନ୍ୟଥା {
		SETOPT(CURLOPT_PROXY, ଖାଲି);
	}


	ଯଦି (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	ଯଦି (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		ଯଦି (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, ଖାଲି);
			SETOPT(CURLOPT_SSL_CTX_DATA, ଖାଲି);
		}
	} ଅନ୍ୟଥା {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef WITH_OPENSSL
		ଯଦି (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_DATA, f);
		}
#endif
	}

	ଫେରନ୍ତୁ CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
ସ୍ଥିର bool
fetch_curl_initiate_fetch(ସଂରଚନା curl_fetch_info *fetch, CURL *handle)
{
	CURLcode code;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	code = fetch_curl_set_options(fetch);
	ଯଦି (code != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		ଫେରନ୍ତୁ false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_add_handle(fetch_curl_multi, fetch->curl_handle);
	ପ୍ରତିଜ୍ଞା(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	ଫେରନ୍ତୁ true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
ସ୍ଥିର CURL *fetch_curl_get_handle(lwc_string *host)
{
	ସଂରଚନା cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	ଯଦି (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		ମୁକ୍ତ(h);
	} ଅନ୍ୟଥା {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	ଫେରନ୍ତୁ ret;
}


/**
 * Dispatch a single job
 */
ସ୍ଥିର bool fetch_curl_start(ଶୂନ୍ୟ *vfetch)
{
	ସଂରଚନା curl_fetch_info *fetch = (ସଂରଚନା curl_fetch_info*)vfetch;
	ଯଦି (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		ଫେରନ୍ତୁ false;
	}
	ଫେରନ୍ତୁ fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
ସ୍ଥିର ଶୂନ୍ୟ fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#ଯଦି LIBCURL_VERSION_ଅଙ୍କ >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	ଫେରନ୍ତୁ;
#ଅନ୍ୟଥା
	ସଂରଚନା cache_handle *h = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	ଯଦି (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		ଫେରନ୍ତୁ;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(ସଂରଚନା cache_handle, curl_handle_ring, c);
	ଯଦି (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		ଯଦି (curl_handle_ring != ଖାଲି) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_next;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} ଅନ୍ୟଥା {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		ଫେରନ୍ତୁ;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (ସଂରଚନା cache_handle*)malloc(ଆକାର(ସଂରଚନା cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_ଭର୍ତ୍ତି(curl_handle_ring, h);
#endif
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
ସ୍ଥିର ଶୂନ୍ୟ fetch_curl_stop(ସଂରଚନା curl_fetch_info *f)
{
	CURLMcode codem;

	ପ୍ରତିଜ୍ଞା(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	ଯଦି (f->curl_handle) {
		/* remove from curl multi handle */
		codem = curl_multi_remove_handle(fetch_curl_multi,
				f->curl_handle);
		ପ୍ରତିଜ୍ଞା(codem == CURLM_OK);
		/* Put this curl handle into the cache if wanted. */
		fetch_curl_cache_handle(f->curl_handle, f->host);
		f->curl_handle = 0;
	}

	fetch_remove_from_queues(f->fetch_handle);
}


/**
 * Abort a fetch.
 */
ସ୍ଥିର ଶୂନ୍ୟ fetch_curl_abort(ଶୂନ୍ୟ *vf)
{
	ସଂରଚନା curl_fetch_info *f = (ସଂରଚନା curl_fetch_info *)vf;
	ପ୍ରତିଜ୍ଞା(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	ଯଦି (f->curl_handle) {
		ଯଦି (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->abort = true;
		} ଅନ୍ୟଥା {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} ଅନ୍ୟଥା {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
ସ୍ଥିର ଶୂନ୍ୟ fetch_curl_free(ଶୂନ୍ୟ *vf)
{
	ସଂରଚନା curl_fetch_info *f = (ସଂରଚନା curl_fetch_info *)vf;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	ମୁକ୍ତ(f->location);
	ମୁକ୍ତ(f->cookie_string);
	ମୁକ୍ତ(f->realm);
	ଯଦି (f->headers) {
		curl_slist_free_all(f->headers);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	ପାଇଁ (i = 0; i < MAX_CERT_DEPTH; i++) {
		ଯଦି (f->cert_data[i].cert != ଖାଲି) {
			X509_free(f->cert_data[i].cert);
		}
	}

	ମୁକ୍ତ(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
ସ୍ଥିର bool fetch_curl_process_headers(ସଂରଚନା curl_fetch_info *f)
{
	long http_code;
	CURLcode code;
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;

	f->had_headers = true;

	ଯଦି (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		ପ୍ରତିଜ୍ଞା(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}
	http_code = f->http_code;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_code);

	ଯଦି ((http_code == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		ସନ୍ଦେଶ.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&ସନ୍ଦେଶ, f->fetch_handle);
		ଫେରନ୍ତୁ true;
	}

	/* handle HTTP redirects (3xx response codes) */
	ଯଦି (HTTP_RESPONSE_IS_3XX(http_code) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		ସନ୍ଦେଶ.type = FETCH_REDIRECT;
		ସନ୍ଦେଶ.data.redirect = f->location;
		fetch_send_callback(&ସନ୍ଦେଶ, f->fetch_handle);
		ଫେରନ୍ତୁ true;
	}

	/* handle HTTP 401 (Authentication errors) */
	ଯଦି (http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		ସନ୍ଦେଶ.type = FETCH_AUTH;
		ସନ୍ଦେଶ.data.auth.realm = f->realm;
		fetch_send_callback(&ସନ୍ଦେଶ, f->fetch_handle);
		ଫେରନ୍ତୁ true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	ଯଦି (f->only_2xx &&
	    strncmp(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_code) == false)) {
		ସନ୍ଦେଶ.type = FETCH_ERROR;
		ସନ୍ଦେଶ.data.error = messages_get("Not2xx");
		fetch_send_callback(&ସନ୍ଦେଶ, f->fetch_handle);
		ଫେରନ୍ତୁ true;
	}

	ଯଦି (f->abort)
		ଫେରନ୍ତୁ true;

	ଫେରନ୍ତୁ false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
ସ୍ଥିର ଶୂନ୍ୟ fetch_curl_done(CURL *curl_handle, CURLcode result)
{
	bool finished = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	ସଂରଚନା curl_fetch_info *f;
	ଅକ୍ଷର **_hideous_hack = (ଅକ୍ଷର **) (ଶୂନ୍ୟ *) &f;
	CURLcode code;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	code = curl_easy_getinfo(curl_handle, CURLINFO_PRIVATE, _hideous_hack);
	ପ୍ରତିଜ୍ଞା(code == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	ଯଦି ((abort_fetch == false) &&
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
		ଯଦି (f->stopped ||
		    (!f->had_headers &&	fetch_curl_process_headers(f))) {
			; /* redirect with no body or similar */
		} ଅନ୍ୟଥା {
			finished = true;
		}
	} ଅନ୍ୟଥା ଯଦି (result == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		ଯଦି (!f->had_headers && fetch_curl_process_headers(f))
			; /* redirect with partial body, or similar */
		ଅନ୍ୟଥା {
			finished = true;
		}
	} ଅନ୍ୟଥା ଯଦି (result == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} ଅନ୍ୟଥା ଯଦି (result == CURLE_SSL_PEER_CERTIFICATE ||
		   result == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} ଅନ୍ୟଥା {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", result);
		error = true;
	}

	fetch_curl_stop(f);

	ଯଦି (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	ଯଦି (abort_fetch) {
		; /* fetch was aborted: no callback */
	} ଅନ୍ୟଥା ଯଦି (finished) {
		fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
		ସନ୍ଦେଶ.type = FETCH_FINISHED;
		fetch_send_callback(&ସନ୍ଦେଶ, f->fetch_handle);
	} ଅନ୍ୟଥା ଯଦି (cert) {
		/* user needs to validate certificate with issue */
		fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
		ସନ୍ଦେଶ.type = FETCH_CERT_ERR;
		fetch_send_callback(&ସନ୍ଦେଶ, f->fetch_handle);
	} ଅନ୍ୟଥା ଯଦି (error) {
		fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
		ବାଛ (result) {
		ସ୍ଥିତି CURLE_SSL_CONNECT_ERROR:
			ସନ୍ଦେଶ.type = FETCH_SSL_ERR;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି CURLE_OPERATION_TIMEDOUT:
			ସନ୍ଦେଶ.type = FETCH_TIMEDOUT;
			ସନ୍ଦେଶ.data.error = curl_easy_strerror(result);
			ଭାଙ୍ଗ;

		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ସନ୍ଦେଶ.type = FETCH_ERROR;
			ସନ୍ଦେଶ.data.error = curl_easy_strerror(result);
		}

		fetch_send_callback(&ସନ୍ଦେଶ, f->fetch_handle);
	}

	fetch_free(f->fetch_handle);
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */
ସ୍ଥିର ଶୂନ୍ୟ fetch_curl_poll(lwc_string *scheme_ignored)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା running, queue;
	CURLMcode codem;
	CURLMsg *curl_ସନ୍ଦେଶ;

	ଯଦି (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_set, write_fd_set, exc_fd_set;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_fd = -1;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&exc_fd_set);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_set, &write_fd_set,
				&exc_fd_set, &max_fd);
		ପ୍ରତିଜ୍ଞା(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		ପାଇଁ (i = 0; i <= max_fd; i++) {
			bool ପଠ = false;
			bool ଲିଖ = false;
			bool error = false;

			ଯଦି (FD_ISSET(i, &read_fd_set)) {
				ପଠ = true;
			}
			ଯଦି (FD_ISSET(i, &write_fd_set)) {
				ଲିଖ = true;
			}
			ଯଦି (FD_ISSET(i, &exc_fd_set)) {
				error = true;
			}
			ଯଦି (ପଠ || ଲିଖ || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      ପଠ ? "read" : "    ",
				      ଲିଖ ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	କର {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		ଯଦି (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			ଫେରନ୍ତୁ;
		}
	} ଯେତେବେଳେ (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_ସନ୍ଦେଶ = curl_multi_info_read(fetch_curl_multi, &queue);
	ଯେତେବେଳେ (curl_ସନ୍ଦେଶ) {
		ବାଛ (curl_ସନ୍ଦେଶ->ସନ୍ଦେଶ) {
			ସ୍ଥିତି CURLMSG_DONE:
				fetch_curl_done(curl_ସନ୍ଦେଶ->easy_handle,
						curl_ସନ୍ଦେଶ->data.result);
				ଭାଙ୍ଗ;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				ଭାଙ୍ଗ;
		}
		curl_ସନ୍ଦେଶ = curl_multi_info_read(fetch_curl_multi, &queue);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
fetch_curl_progress(ଶୂନ୍ୟ *clientp,
		    NSCURL_PROGRESS_T dltotal,
		    NSCURL_PROGRESS_T dlnow,
		    NSCURL_PROGRESS_T ultotal,
		    NSCURL_PROGRESS_T ulnow)
{
	ସ୍ଥିର ଅକ୍ଷର fetch_progress_buffer[256]; /**< Progress buffer for cURL */
	ସଂରଚନା curl_fetch_info *f = (ସଂରଚନା curl_fetch_info *) clientp;
	uint64_t time_now_ms;
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;

	ଯଦି (f->abort) {
		ଫେରନ୍ତୁ 0;
        }

	ସନ୍ଦେଶ.type = FETCH_PROGRESS;
	ସନ୍ଦେଶ.data.progress = fetch_progress_buffer;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&time_now_ms);
#define UPDATE_DELAY_MS (1000 / UPDATES_PER_SECOND)
	ଯଦି (time_now_ms - f->last_progress_update < UPDATE_DELAY_MS) {
		ଫେରନ୍ତୁ 0;
        }
#undef UPDATE_DELAY_MS
	f->last_progress_update = time_now_ms;

	ଯଦି (dltotal > 0) {
		snprintf(fetch_progress_buffer, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&ସନ୍ଦେଶ, f->fetch_handle);
	} ଅନ୍ୟଥା {
		snprintf(fetch_progress_buffer, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&ସନ୍ଦେଶ, f->fetch_handle);
	}

	ଫେରନ୍ତୁ 0;
}


/**
 * Format curl debug for nslog
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 ଅକ୍ଷର *data,
		 size_t size,
		 ଶୂନ୍ୟ *userptr)
{
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର s_infotype[CURLINFO_END][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	ବାଛ(type) {
	ସ୍ଥିତି CURLINFO_TEXT:
	ସ୍ଥିତି CURLINFO_HEADER_OUT:
	ସ୍ଥିତି CURLINFO_HEADER_IN:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)size - 1, data);
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର curl_socket_t fetch_curl_socket_open(ଶୂନ୍ୟ *clientp,
		curlsocktype purpose, ସଂରଚନା curl_sockaddr *address)
{
	(ଶୂନ୍ୟ) clientp;
	(ଶୂନ୍ୟ) purpose;

	ଫେରନ୍ତୁ (curl_socket_t) guit->fetch->socket_open(
			address->family, address->socktype,
			address->protocol);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fetch_curl_socket_close(ଶୂନ୍ୟ *clientp, curl_socket_t item)
{
	(ଶୂନ୍ୟ) clientp;

	ଫେରନ୍ତୁ guit->fetch->socket_close((ପୂର୍ଣ୍ଣସଂଖ୍ୟା) item);
}

/**
 * Callback function for cURL.
 */
ସ୍ଥିର size_t fetch_curl_data(ଅକ୍ଷର *data, size_t size, size_t nmemb, ଶୂନ୍ୟ *_f)
{
	ସଂରଚନା curl_fetch_info *f = _f;
	CURLcode code;
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;

	/* ensure we only have to get this information once */
	ଯଦି (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		ପ୍ରତିଜ୍ଞା(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	ଯଦି (f->http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_code = 0;
		ଫେରନ୍ତୁ size * nmemb;
	}

	ଯଦି (f->abort || (!f->had_headers && fetch_curl_process_headers(f))) {
		f->stopped = true;
		ଫେରନ୍ତୁ 0;
	}

	/* send data to the caller */
	ସନ୍ଦେଶ.type = FETCH_DATA;
	ସନ୍ଦେଶ.data.header_or_data.buf = (ଅଚଳ uint8_t *) data;
	ସନ୍ଦେଶ.data.header_or_data.len = size * nmemb;
	fetch_send_callback(&ସନ୍ଦେଶ, f->fetch_handle);

	ଯଦି (f->abort) {
		f->stopped = true;
		ଫେରନ୍ତୁ 0;
	}

	ଫେରନ୍ତୁ size * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
ସ୍ଥିର size_t
fetch_curl_header(ଅକ୍ଷର *data, size_t size, size_t nmemb, ଶୂନ୍ୟ *_f)
{
	ସଂରଚନା curl_fetch_info *f = _f;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	size *= nmemb;

	ଯଦି (f->abort) {
		f->stopped = true;
		ଫେରନ୍ତୁ 0;
	}

	ଯଦି (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	ସନ୍ଦେଶ.type = FETCH_HEADER;
	ସନ୍ଦେଶ.data.header_or_data.buf = (ଅଚଳ uint8_t *) data;
	ସନ୍ଦେଶ.data.header_or_data.len = size;
	fetch_send_callback(&ସନ୍ଦେଶ, f->fetch_handle);

#define SKIP_ST(o) ପାଇଁ (i = (o); i < (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) size && (data[i] == ' ' || data[i] == '\t'); i++)

	ଯଦି (12 < size && strncasecmp(data, "Location:", 9) == 0) {
		/* extract Location header */
		ମୁକ୍ତ(f->location);
		f->location = malloc(size);
		ଯଦି (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			ଫେରନ୍ତୁ size;
		}
		SKIP_ST(9);
		strncpy(f->location, data + i, size - i);
		f->location[size - i] = '\0';
		ପାଇଁ (i = size - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} ଅନ୍ୟଥା ଯଦି (15 < size && strncasecmp(data, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		ଯଦି (i < (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)size && '0' <= data[i] && data[i] <= '9')
			f->content_length = atol(data + i);
	} ଅନ୍ୟଥା ଯଦି (17 < size && strncasecmp(data, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		ଯେତେବେଳେ (i < (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) size - 5 &&
				strncasecmp(data + i, "realm", 5))
			i++;
		ଯେତେବେଳେ (i < (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) size - 1 && data[++i] != '"')
			/* */;
		i++;

		ଯଦି (i < (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) size) {
			size_t end = i;

			ଯେତେବେଳେ (end < size && data[end] != '"')
				++end;

			ଯଦି (end < size) {
				ମୁକ୍ତ(f->realm);
				f->realm = malloc(end - i + 1);
				ଯଦି (f->realm != ଖାଲି) {
					strncpy(f->realm, data + i, end - i);
					f->realm[end - i] = '\0';
				}
			}
		}
	} ଅନ୍ୟଥା ଯଦି (11 < size && strncasecmp(data, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_set_cookie(f->fetch_handle, &data[i]);
	}

	ଫେରନ୍ତୁ size;
#undef SKIP_ST
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fetch_curl_fdset(lwc_string *scheme, fd_set *read_set,
			    fd_set *write_set, fd_set *error_set)
{
	CURLMcode code;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxfd = -1;

	code = curl_multi_fdset(fetch_curl_multi,
				read_set,
				write_set,
				error_set,
				&maxfd);
	ପ୍ରତିଜ୍ଞା(code == CURLM_OK);

	ଫେରନ୍ତୁ maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_register(ଶୂନ୍ୟ)
{
	CURLcode code;
	curl_version_info_data *data;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	lwc_string *scheme;
	ଅଚଳ ସଂରଚନା fetcher_operation_table fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.setup = fetch_curl_setup,
		.start = fetch_curl_start,
		.abort = fetch_curl_abort,
		.ମୁକ୍ତ = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#ଯଦି LIBCURL_VERSION_ଅଙ୍କ >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, ଖାଲି, ଖାଲି);
	ଯଦି (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} ଅନ୍ୟଥା {
		curl_with_openssl = false;
	}
#endif

	NSLOG(netsurf, INFO, "curl_version %s", curl_version());

	code = curl_global_init(CURL_GLOBAL_ALL);
	ଯଦି (code != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		ଫେରନ୍ତୁ NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	ଯଦି (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		ଫେରନ୍ତୁ NSERROR_INIT_FAILED;
	}

#ଯଦି LIBCURL_VERSION_ଅଙ୍କ >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#undef SETOPT
#define SETOPT(option, ମୂଲ୍ୟ) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, ମୂଲ୍ୟ);	\
	ଯଦି (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		ଯାଅ curl_multi_setopt_failed;				\
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
	ଯଦି (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		ଫେରନ୍ତୁ NSERROR_INIT_FAILED;
	}

#undef SETOPT
#define SETOPT(option, ମୂଲ୍ୟ) \
	code = curl_easy_setopt(fetch_blank_curl, option, ମୂଲ୍ୟ);	\
	ଯଦି (code != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		ଯାଅ curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_buffer);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	ଯଦି (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} ଅନ୍ୟଥା {
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
	SETOPT(CURLOPT_ତଳ_SPEED_LIMIT, 1L);
	SETOPT(CURLOPT_ତଳ_SPEED_TIME, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (long)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	ଯଦି (nsoption_charp(ca_bundle) &&
	    strcmp(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	ଯଦି (nsoption_charp(ca_path) && strcmp(nsoption_charp(ca_path), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_path));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_path));
	}

#ଯଦି LIBCURL_VERSION_ଅଙ୍କ < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	code = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, ଖାଲି);
	ଯଦି (code != CURLE_OK) {
		curl_with_openssl = false;
	} ଅନ୍ୟଥା {
		curl_with_openssl = true;
	}
#endif

	ଯଦି (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#ଯଦି LIBCURL_VERSION_ଅଙ୍କ >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		code = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		ଯଦି (code != CURLE_OK && code != CURLE_NOT_BUILT_IN)
			ଯାଅ curl_easy_setopt_failed;
#endif
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	data = curl_version_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	ଯଦି (curl_fetch_ssl_hashmap == ଖାଲି) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ପାଇଁ (i = 0; data->protocols[i]; i++) {
		ଯଦି (strcmp(data->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} ଅନ୍ୟଥା ଯଦି (strcmp(data->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} ଅନ୍ୟଥା {
			/* Ignore non-http(s) protocols */
			ଜାରି;
		}

		ଯଦି (fetcher_add(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      data->protocols[i]);
		}
	}

	ଫେରନ୍ତୁ NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	ଫେରନ୍ତୁ NSERROR_INIT_FAILED;

#ଯଦି LIBCURL_VERSION_ଅଙ୍କ >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	ଫେରନ୍ତୁ NSERROR_INIT_FAILED;
#endif
}
