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
#అన్తర్భావ "utils/inet.h"

#అన్తర్భావ <assert.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <inttypes.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <sys/stat.h>

#అన్తర్భావ <libwapcaplet/libwapcaplet.h>
#అన్తర్భావ <nsutils/time.h>

#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/hashmap.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/ring.h"
#అన్తర్భావ "utils/useragent.h"
#అన్తర్భావ "utils/file.h"
#అన్తర్భావ "utils/string.h"
#అన్తర్భావ "netsurf/fetch.h"
#అన్తర్భావ "netsurf/misc.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "content/fetch.h"
#అన్తర్భావ "content/fetchers.h"
#అన్తర్భావ "content/fetchers/curl.h"
#అన్తర్భావ "content/urldb.h"

/**
 * maximum number of progress notifications per second
 */
#నిర్వచించు UPDATES_చొప్పున_రెండవ 2

/**
 * The ciphersuites the browser is prepared to use for TLS1.3
 */
#నిర్వచించు CIPHER_SUITES						\
	"TLS_AES_256_GCM_SHA384:"				\
	"TLS_CHACHA20_POLY1305_SHA256:"				\
	"TLS_AES_128_GCM_SHA256"

/**
 * The ciphersuites the browser is prepared to use for TLS<1.3
 */
#నిర్వచించు CIPHER_LIST						\
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
#నిర్వచించు CIPHER_LIST_LEGACY					\
	/* as above */						\
	CIPHER_LIST":"						\
	/* enable (non-PFS) RSA AES GCM suites */		\
	"RSA+AESGCM:"						\
	/* enable (non-PFS) RSA (mac=SHA1) AES CBC suites */	\
	"RSA+AES+SHA1"

/* Open SSL compatability for certificate handling */
#నిర్వచితమైతే WITH_OPENSSL

#అన్తర్భావ <openssl/ssl.h>
#అన్తర్భావ <openssl/x509v3.h>

#లేకపోతే /* WITH_OPENSSL */

రకనిర్వచనం అక్షరం X509;

స్థిర శూన్య X509_free(X509 *cert)
{
	విడుదల(cert);
}

#అయితే_ముగింపు /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
స్థిర uint32_t
curl_fetch_ssl_కీలకం_hash(శూన్య *కీలకం)
{
	nsurl *url = కీలకం;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *పోర్టు = nsurl_get_component(url, NSURL_పోర్టు);
	uint32_t hash;

	ఒకవేళ (పోర్టు == ఖాళీ)
		పోర్టు = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_విలువ(hostname) ^ lwc_string_hash_విలువ(పోర్టు);

	lwc_string_unref(hostname);
	lwc_string_unref(పోర్టు);

	తిరిగి hash;
}

/* We only compare the hostname and port */
స్థిర bool
curl_fetch_ssl_కీలకం_eq(శూన్య *key1, శూన్య *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_పోర్టు);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_పోర్టు);
	bool iseq = false;

	ఒకవేళ (port1 == ఖాళీ)
		port1 = lwc_string_ref(corestring_lwc_443);
	ఒకవేళ (port2 == ఖాళీ)
		port2 = lwc_string_ref(corestring_lwc_443);

	ఒకవేళ (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		వెళ్ళు బహిర్గతం;

	iseq = false;
	ఒకవేళ (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		వెళ్ళు బహిర్గతం;

బహిర్గతం:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	తిరిగి iseq;
}

స్థిర శూన్య *
curl_fetch_ssl_విలువ_alloc(శూన్య *కీలకం)
{
	నిర్మాణం cert_chain *బహిర్గతం;

	ఒకవేళ (cert_chain_alloc(0, &బహిర్గతం) != NSERROR_OK) {
		తిరిగి ఖాళీ;
	}

	తిరిగి బహిర్గతం;
}

స్థిర శూన్య
curl_fetch_ssl_విలువ_destroy(శూన్య *విలువ)
{
	నిర్మాణం cert_chain *chain = విలువ;
	ఒకవేళ (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

స్థిర hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.కీలకం_clone = (hashmap_కీలకం_clone_t)nsurl_ref,
	.కీలకం_destroy = (hashmap_కీలకం_destroy_t)nsurl_unref,
	.కీలకం_eq = curl_fetch_ssl_కీలకం_eq,
	.కీలకం_hash = curl_fetch_ssl_కీలకం_hash,
	.విలువ_alloc = curl_fetch_ssl_విలువ_alloc,
	.విలువ_destroy = curl_fetch_ssl_విలువ_destroy,
};

స్థిర hashmap_t *curl_fetch_ssl_hashmap = ఖాళీ;

/** SSL certificate info */
నిర్మాణం cert_info {
	X509 *cert;		/**< Pointer to certificate */
	దీర్ఘ err;		/**< OpenSSL error code */
};

#ఒకవేళ LIBCURL_సంస్కరణ_అంకె >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#నిర్వచించు NSCURLOPT_పురోగతి_FUNCTION CURLOPT_XFERINFOFUNCTION
#నిర్వచించు NSCURLOPT_పురోగతి_దత్తాంశం CURLOPT_XFERINFODATA
#నిర్వచించు NSCURL_పురోగతి_T curl_off_t
#లేకపోతే
#నిర్వచించు NSCURLOPT_పురోగతి_FUNCTION CURLOPT_PROGRESSFUNCTION
#నిర్వచించు NSCURLOPT_పురోగతి_దత్తాంశం CURLOPT_PROGRESSDATA
#నిర్వచించు NSCURL_పురోగతి_T ద్విగుణ
#అయితే_ముగింపు

#ఒకవేళ LIBCURL_సంస్కరణ_అంకె >= 0x073800 /* 7.56.0 depricated curl_formadd */
#నిర్వచించు NSCURL_POSTDATA_T curl_mime
#నిర్వచించు NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#నిర్వచించు NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#లేకపోతే
#నిర్వచించు NSCURL_POSTDATA_T నిర్మాణం curl_httppost
#నిర్వచించు NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#నిర్వచించు NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#అయితే_ముగింపు

/** Information for a single fetch. */
నిర్మాణం curl_fetch_info {
	నిర్మాణం fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool పంపిన_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_శీర్షికలు;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	నిర్మాణం curl_slist *శీర్షికలు;	/**< List of request headers. */
	అక్షరం *location;		/**< Response Location header, or 0. */
	గుర్తులేని దీర్ఘ content_నిడివి;	/**< Response Content-Length, or 0. */
	అక్షరం *cookie_string;	/**< Cookie string for this fetch */
	అక్షరం *realm;		/**< HTTP Auth Realm */
	నిర్మాణం fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	దీర్ఘ http_సంకేతం; /**< HTTP result code from cURL. */

	uint64_t చివరి_పురోగతి_update;	/**< Time of last progress update */
	పూర్ణాంకం cert_లోతు; /**< deepest certificate in use */
	నిర్మాణం cert_info cert_దత్తాంశం[MAX_CERT_లోతు]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
నిర్మాణం cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	నిర్మాణం cache_handle *r_prev; /**< Previous cached handle in ring. */
	నిర్మాణం cache_handle *r_తదుపరి; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
స్థిర CURL *fetch_blank_curl;

/** Ring of cached handles */
స్థిర నిర్మాణం cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
స్థిర పూర్ణాంకం curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
స్థిర bool curl_with_openssl;

/** Error buffer for cURL. */
స్థిర అక్షరం fetch_error_బఫరు[CURL_ERROR_కొలత];

/** Proxy authentication details. */
స్థిర అక్షరం fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
స్థిర bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
స్థిర bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_దత్తాంశం(scheme));
	curl_fetchers_registered++;
	తిరిగి true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
స్థిర శూన్య fetch_curl_finalise(lwc_string *scheme)
{
	నిర్మాణం cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_దత్తాంశం(scheme));
	ఒకవేళ (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		ఒకవేళ (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = ఖాళీ;
	}

	/* Free anything remaining in the cached curl handle ring */
	అయితే (curl_handle_ring != ఖాళీ) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
		curl_easy_cleanup(h->handle);
		విడుదల(h);
	}
}


/**
 * Check if this fetcher can fetch a url.
 *
 * \param url The url to check.
 * \return true if the fetcher supports the url else false.
 */
స్థిర bool fetch_curl_can_fetch(స్థిరరాశి nsurl *url)
{
	తిరిగి nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
స్థిర నిర్మాణం fetch_postdata *
fetch_curl_alloc_postdata(స్థిరరాశి అక్షరం *post_urlenc,
			  స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *post_multipart)
{
	నిర్మాణం fetch_postdata *postdata;
	postdata = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం fetch_postdata));
	ఒకవేళ (postdata != ఖాళీ) {

		ఒకవేళ (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->దత్తాంశం.urlenc = strdup(post_urlenc);
			ఒకవేళ (postdata->దత్తాంశం.urlenc == ఖాళీ) {
				విడుదల(postdata);
				postdata = ఖాళీ;
			}
		} లేకపోతే ఒకవేళ (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->దత్తాంశం.multipart = fetch_multipart_దత్తాంశం_clone(post_multipart);
			ఒకవేళ (postdata->దత్తాంశం.multipart == ఖాళీ) {
				విడుదల(postdata);
				postdata = ఖాళీ;
			}
		} లేకపోతే {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	తిరిగి postdata;
}

/**
 * free postdata
 */
స్థిర శూన్య fetch_curl_free_postdata(నిర్మాణం fetch_postdata *postdata)
{
	ఒకవేళ (postdata != ఖాళీ) {
		ఎంపిక (postdata->type) {
		స్థితి FETCH_POSTDATA_NONE:
			ఆపు;
		స్థితి FETCH_POSTDATA_URLENC:
			విడుదల(postdata->దత్తాంశం.urlenc);
			ఆపు;
		స్థితి FETCH_POSTDATA_MULTIPART:
			fetch_multipart_దత్తాంశం_destroy(postdata->దత్తాంశం.multipart);
			ఆపు;
		}

		విడుదల(postdata);
	}
}

/**
 *construct a new fetch structure
 */
స్థిర నిర్మాణం curl_fetch_info *fetch_alloc(శూన్య)
{
	నిర్మాణం curl_fetch_info *fetch;
	fetch = జ్ఞాపకం_ఇవ్వు(పరిమాణం (*fetch));
	ఒకవేళ (fetch == ఖాళీ)
		తిరిగి ఖాళీ;

	fetch->curl_handle = ఖాళీ;
	fetch->పంపిన_ssl_chain = false;
	fetch->had_శీర్షికలు = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->శీర్షికలు = ఖాళీ;
	fetch->url = ఖాళీ;
	fetch->host = ఖాళీ;
	fetch->location = ఖాళీ;
	fetch->content_నిడివి = 0;
	fetch->http_సంకేతం = 0;
	fetch->cookie_string = ఖాళీ;
	fetch->realm = ఖాళీ;
	fetch->చివరి_పురోగతి_update = 0;
	fetch->postdata = ఖాళీ;
	fetch->curl_postdata = ఖాళీ;

	/* Clear certificate chain data */
	memset(fetch->cert_దత్తాంశం, 0, పరిమాణం(fetch->cert_దత్తాంశం));
	fetch->cert_లోతు = -1;

	తిరిగి fetch;
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
స్థిర శూన్య *
fetch_curl_అమరిక(నిర్మాణం fetch *మాతృ_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 స్థిరరాశి అక్షరం *post_urlenc,
		 స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *post_multipart,
		 స్థిరరాశి అక్షరం **శీర్షికలు)
{
	నిర్మాణం curl_fetch_info *fetch;
	నిర్మాణం curl_slist *slist;
	పూర్ణాంకం i;

	fetch = fetch_alloc();
	ఒకవేళ (fetch == ఖాళీ)
		తిరిగి ఖాళీ;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = మాతృ_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	ఒకవేళ (fetch->host == ఖాళీ) {
		వెళ్ళు failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	ఒకవేళ (fetch->postdata == ఖాళీ) {
		వెళ్ళు failed;
	}

#నిర్వచించు APPEND(list, విలువ) \
	slist = curl_slist_append(list, విలువ);		\
	ఒకవేళ (slist == ఖాళీ)				\
		వెళ్ళు failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->శీర్షికలు, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->శీర్షికలు, "Expect:");

	ఒకవేళ ((nsoption_charp(accept_language) != ఖాళీ) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		అక్షరం s[80];
		snprintf(s, పరిమాణం s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[పరిమాణం s - 1] = 0;
		APPEND(fetch->శీర్షికలు, s);
	}

	ఒకవేళ (nsoption_charp(accept_charset) != ఖాళీ &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		అక్షరం s[80];
		snprintf(s, పరిమాణం s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[పరిమాణం s - 1] = 0;
		APPEND(fetch->శీర్షికలు, s);
	}

	ఒకవేళ (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->శీర్షికలు, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	కొరకు (i = 0; శీర్షికలు[i] != ఖాళీ; i++) {
		APPEND(fetch->శీర్షికలు, శీర్షికలు[i]);
	}

	తిరిగి fetch;

#నిర్వచనం_రద్దు APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->శీర్షికలు);
	విడుదల(fetch);
	తిరిగి ఖాళీ;
}


#నిర్వచితమైతే WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
స్థిర నిర్మాణం cert_chain *
fetch_curl_get_cached_chain(నిర్మాణం curl_fetch_info *f)
{
	నిర్మాణం cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	ఒకవేళ (chain == ఖాళీ) {
		chain = hashmap_చొప్పించు(curl_fetch_ssl_hashmap, f->url);
	}

	తిరిగి chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
స్థిర శూన్య
fetch_curl_store_certs_లోపల_cache(నిర్మాణం curl_fetch_info *f)
{
	size_t లోతు;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_లోతు];
	నిర్మాణం cert_chain chain, *cached_chain;
	నిర్మాణం cert_info *certs;

	memset(&chain, 0, పరిమాణం(chain));

	certs = f->cert_దత్తాంశం;
	chain.లోతు = f->cert_లోతు + 1; /* 0 indexed certificate depth */

	కొరకు (లోతు = 0; లోతు < chain.లోతు; లోతు++) {
		ఒకవేళ (certs[లోతు].cert == ఖాళీ) {
			/* This certificate is missing, skip it */
			chain.certs[లోతు].err = SSL_CERT_ERR_CERT_MISSING;
			కొనసాగు;
		}

		/* error code (if any) */
		ఎంపిక (certs[లోతు].err) {
		స్థితి X509_V_OK:
			chain.certs[లోతు].err = SSL_CERT_ERR_OK;
			ఆపు;

		స్థితి X509_V_ERR_UNABLE_గమ్యం_GET_ISSUER_CERT:
			/* fallthrough */
		స్థితి X509_V_ERR_UNABLE_గమ్యం_DECODE_ISSUER_ప్రజా_కీలకం:
			chain.certs[లోతు].err = SSL_CERT_ERR_BAD_ISSUER;
			ఆపు;

		స్థితి X509_V_ERR_UNABLE_గమ్యం_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		స్థితి X509_V_ERR_UNABLE_గమ్యం_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		స్థితి X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		స్థితి X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[లోతు].err = SSL_CERT_ERR_BAD_SIG;
			ఆపు;

		స్థితి X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		స్థితి X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[లోతు].err = SSL_CERT_ERR_TOO_YOUNG;
			ఆపు;

		స్థితి X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		స్థితి X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[లోతు].err = SSL_CERT_ERR_TOO_OLD;
			ఆపు;

		స్థితి X509_V_ERR_లోతు_ZERO_SELF_SIGNED_CERT:
			chain.certs[లోతు].err = SSL_CERT_ERR_SELF_SIGNED;
			ఆపు;

		స్థితి X509_V_ERR_SELF_SIGNED_CERT_లోపల_CHAIN:
			chain.certs[లోతు].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			ఆపు;

		స్థితి X509_V_ERR_CERT_REVOKED:
			chain.certs[లోతు].err = SSL_CERT_ERR_REVOKED;
			ఆపు;

		స్థితి X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[లోతు].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			ఆపు;

		అప్రమేయ:
			chain.certs[లోతు].err = SSL_CERT_ERR_UNKNOWN;
			ఆపు;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[లోతు].cert);
		BIO_get_mem_ptr(mem, &buf[లోతు]);
		(శూన్య) BIO_సమితి_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[లోతు].der = (uint8_t *)buf[లోతు]->దత్తాంశం;
		chain.certs[లోతు].der_నిడివి = buf[లోతు]->నిడివి;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	ఒకవేళ (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	కొరకు (లోతు = 0; లోతు < chain.లోతు; లోతు++) {
		ఒకవేళ (chain.certs[లోతు].err == SSL_CERT_ERR_CERT_MISSING) {
			కొనసాగు;
		}
		ఒకవేళ (buf[లోతు] != ఖాళీ) {
			BUF_MEM_free(buf[లోతు]);
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
స్థిర పూర్ణాంకం
fetch_curl_verify_callback(పూర్ణాంకం verify_ok, X509_STORE_CTX *x509_ctx)
{
	పూర్ణాంకం లోతు;
	నిర్మాణం curl_fetch_info *fetch;

	లోతు = X509_STORE_CTX_get_error_లోతు(x509_ctx);
	fetch = X509_STORE_CTX_get_app_దత్తాంశం(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	ఒకవేళ (లోతు >= MAX_CERT_లోతు) {
		X509_STORE_CTX_సమితి_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		తిరిగి 0;
	}

	/* record the max depth */
	ఒకవేళ (లోతు > fetch->cert_లోతు) {
		fetch->cert_లోతు = లోతు;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	ఒకవేళ (!fetch->cert_దత్తాంశం[లోతు].cert) {
		fetch->cert_దత్తాంశం[లోతు].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_పైకి_ref(fetch->cert_దత్తాంశం[లోతు].cert);
		fetch->cert_దత్తాంశం[లోతు].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	ఒకవేళ (లోతు > 0) {
		verify_ok = 1;
	} లేకపోతే {
		/* search for deeper certificates in the chain with errors */
		కొరకు (లోతు = fetch->cert_లోతు; లోతు > 0; లోతు--) {
			ఒకవేళ (fetch->cert_దత్తాంశం[లోతు].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_సమితి_error(x509_ctx, fetch->cert_దత్తాంశం[లోతు].err);
			}
		}
	}

	తిరిగి verify_ok;
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
స్థిర పూర్ణాంకం fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, శూన్య *parm)
{
	నిర్మాణం curl_fetch_info *f = (నిర్మాణం curl_fetch_info *) parm;
	పూర్ణాంకం ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_సమితి_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_దత్తాంశం(f->host),
					 lwc_string_నిడివి(f->host));

	/* Store fetch struct in context for verify callback */
	ఒకవేళ (ok) {
		ok = X509_STORE_CTX_సమితి_app_దత్తాంశం(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	ఒకవేళ (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_లోపల_cache(f);

	తిరిగి ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
స్థిర CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, శూన్య *_sslctx, శూన్య *parm)
{
	నిర్మాణం curl_fetch_info *f = (నిర్మాణం curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	దీర్ఘ options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_సమితి_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_సమితి_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	ఒకవేళ (f->downgrade_tls) {
		/* Disable TLS 1.3 if the server can't cope with it */
#నిర్వచితమైతే SSL_OP_NO_TLSv1_3
		options |= SSL_OP_NO_TLSv1_3;
#అయితే_ముగింపు
#నిర్వచితమైతే SSL_బహుళకం_SEND_FALLBACK_SCSV
		/* Ensure server rejects the connection if downgraded too far */
		SSL_CTX_సమితి_బహుళకం(sslctx, SSL_బహుళకం_SEND_FALLBACK_SCSV);
#అయితే_ముగింపు
	}

	SSL_CTX_సమితి_options(sslctx, options);

#నిర్వచితమైతే SSL_OP_NO_TICKET
	SSL_CTX_clear_options(sslctx, SSL_OP_NO_TICKET);
#అయితే_ముగింపు

	తిరిగి CURLE_OK;
}


#అయితే_ముగింపు /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
స్థిర శూన్య
fetch_curl_report_certs_upstream(నిర్మాణం curl_fetch_info *f)
{
	fetch_సందేశం సందేశం;
	నిర్మాణం cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	ఒకవేళ (chain != ఖాళీ) {
		సందేశం.type = FETCH_CERTS;
		సందేశం.దత్తాంశం.chain = chain;

		fetch_send_callback(&సందేశం, f->fetch_handle);
	}

	f->పంపిన_ssl_chain = true;
}

#ఒకవేళ LIBCURL_సంస్కరణ_అంకె >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
నిర్మాణం curl_mime_ctx {
	అక్షరం *బఫరు;
	curl_off_t కొలత;
	curl_off_t position;
};

స్థిర size_t mime_దత్తాంశం_read_callback(అక్షరం *బఫరు, size_t కొలత, size_t nitems, శూన్య *arg)
{
	నిర్మాణం curl_mime_ctx *mctx = (నిర్మాణం curl_mime_ctx *) arg;
	curl_off_t sz = mctx->కొలత - mctx->position;

	nitems *= కొలత;
	ఒకవేళ(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	ఒకవేళ(sz) {
		జ్ఞాపకం_నకలు(బఫరు, mctx->బఫరు + mctx->position, sz);
	}
	mctx->position += sz;
	తిరిగి sz;
}

స్థిర పూర్ణాంకం mime_దత్తాంశం_seek_callback(శూన్య *arg, curl_off_t స్థానభ్రంశం, పూర్ణాంకం origin)
{
	నిర్మాణం curl_mime_ctx *mctx = (నిర్మాణం curl_mime_ctx *) arg;

	ఎంపిక(origin) {
	స్థితి SEEK_END:
		స్థానభ్రంశం += mctx->కొలత;
		ఆపు;
	స్థితి SEEK_CUR:
		స్థానభ్రంశం += mctx->position;
		ఆపు;
	}

	ఒకవేళ(స్థానభ్రంశం < 0) {
		తిరిగి CURL_SEEKFUNC_FAIL;
	}
	mctx->position = స్థానభ్రంశం;
	తిరిగి CURL_SEEKFUNC_OK;
}

స్థిర శూన్య mime_దత్తాంశం_free_callback(శూన్య *arg)
{
	నిర్మాణం curl_mime_ctx *mctx = (నిర్మాణం curl_mime_ctx *) arg;
	విడుదల(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
స్థిర curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode సంకేతం_2 = CURLE_OK;
	size_t విలువ_len;

	cmime = curl_mime_init(chandle);
	ఒకవేళ (cmime == ఖాళీ) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		తిరిగి ఖాళీ;
	}

	/* iterate post data */
	కొరకు (; multipart != ఖాళీ; multipart = multipart->తదుపరి) {
		part = curl_mime_addpart(cmime);
		ఒకవేళ (part == ఖాళీ) {
			వెళ్ళు convert_failed;
		}

		సంకేతం_2 = curl_mime_name(part, multipart->name);
		ఒకవేళ (సంకేతం_2 != CURLE_OK) {
			వెళ్ళు convert_failed;
		}

		విలువ_len = అక్షరశ్రేణి_పొడవు(multipart->విలువ);

		ఒకవేళ (multipart->file && విలువ_len==0) {
			/* file entries with no filename require special handling */
			సంకేతం_2=curl_mime_దత్తాంశం(part, multipart->విలువ, విలువ_len);
			ఒకవేళ (సంకేతం_2 != CURLE_OK) {
				వెళ్ళు convert_failed;
			}

			సంకేతం_2 = curl_mime_filename(part, "");
			ఒకవేళ (సంకేతం_2 != CURLE_OK) {
				వెళ్ళు convert_failed;
			}

			సంకేతం_2 = curl_mime_type(part, "application/octet-stream");
			ఒకవేళ (సంకేతం_2 != CURLE_OK) {
				వెళ్ళు convert_failed;
			}

		} లేకపోతే ఒకవేళ(multipart->file) {
			/* file entry */
			nserror ret;
			అక్షరం *leafname = ఖాళీ;
			అక్షరం *mimetype = ఖాళీ;

			సంకేతం_2 = curl_mime_filedata(part, multipart->rawfile);
			ఒకవేళ (సంకేతం_2 != CURLE_OK) {
				వెళ్ళు convert_failed;
			}

			ret = guit->file->basename(multipart->విలువ, &leafname, ఖాళీ);
			ఒకవేళ (ret != NSERROR_OK) {
				వెళ్ళు convert_failed;
			}
			సంకేతం_2 = curl_mime_filename(part, leafname);
			విడుదల(leafname);
			ఒకవేళ (సంకేతం_2 != CURLE_OK) {
				వెళ్ళు convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->విలువ);
			ఒకవేళ (mimetype == ఖాళీ) {
				mimetype=strdup("text/plain");
			}
			ఒకవేళ (mimetype == ఖాళీ) {
				వెళ్ళు convert_failed;
			}
			సంకేతం_2 = curl_mime_type(part, mimetype);
			విడుదల(mimetype);
			ఒకవేళ (సంకేతం_2 != CURLE_OK) {
				వెళ్ళు convert_failed;
			}

		} లేకపోతే {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			నిర్మాణం curl_mime_ctx *cb_ctx;
			cb_ctx = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం curl_mime_ctx));
			ఒకవేళ (cb_ctx == ఖాళీ) {
				వెళ్ళు convert_failed;
			}
			cb_ctx->బఫరు = multipart->విలువ;
			cb_ctx->కొలత = విలువ_len;
			cb_ctx->position = 0;
			సంకేతం_2 = curl_mime_దత్తాంశం_cb(part,
						 విలువ_len,
						 mime_దత్తాంశం_read_callback,
						 mime_దత్తాంశం_seek_callback,
						 mime_దత్తాంశం_free_callback,
						 cb_ctx);
			ఒకవేళ (సంకేతం_2 != CURLE_OK) {
				విడుదల(cb_ctx);
				వెళ్ళు convert_failed;
			}
		}
	}

	తిరిగి cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", సంకేతం_2);
	curl_mime_free(cmime);
	తిరిగి ఖాళీ;
}

#లేకపోతే /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
స్థిర నిర్మాణం curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *control)
{
	నిర్మాణం curl_httppost *post = ఖాళీ, *చివరి = ఖాళీ;
	CURLFORMcode సంకేతం_2;
	nserror ret;

	కొరకు (; control; control = control->తదుపరి) {
		ఒకవేళ (control->file) {
			అక్షరం *leafname = ఖాళీ;
			ret = guit->file->basename(control->విలువ, &leafname, ఖాళీ);
			ఒకవేళ (ret != NSERROR_OK) {
				కొనసాగు;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			ఒకవేళ (control->విలువ[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				స్థిర అక్షరం buf;

				సంకేతం_2 = curl_formadd(&post, &చివరి,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_బఫరు, control->విలువ,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_చివర);
				ఒకవేళ (సంకేతం_2 != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", సంకేతం_2,
					      control->name);
			} లేకపోతే {
				అక్షరం *mimetype = guit->fetch->mimetype(control->విలువ);
				సంకేతం_2 = curl_formadd(&post, &చివరి,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_చివర);
				ఒకవేళ (సంకేతం_2 != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      సంకేతం_2,
					      control->name,
					      control->విలువ);
				విడుదల(mimetype);
			}
			విడుదల(leafname);
		} లేకపోతే {
			సంకేతం_2 = curl_formadd(&post, &చివరి,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->విలువ,
					    CURLFORM_చివర);
			ఒకవేళ (సంకేతం_2 != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", సంకేతం_2,
				      control->name, control->విలువ);
		}
	}

	తిరిగి post;
}

#అయితే_ముగింపు  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
స్థిర CURLcode fetch_curl_సమితి_postdata(నిర్మాణం curl_fetch_info *f)
{
	CURLcode సంకేతం_2 = CURLE_OK;

#నిర్వచనం_రద్దు SETOPT
#నిర్వచించు SETOPT(option, విలువ) { \
	సంకేతం_2 = curl_easy_setopt(f->curl_handle, option, విలువ);	\
	ఒకవేళ (సంకేతం_2 != CURLE_OK)					\
		తిరిగి సంకేతం_2;					\
	}

	ఎంపిక (f->postdata->type) {
	స్థితి FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, ఖాళీ);
		SETOPT(NSCURL_POSTDATA_CURLOPT, ఖాళీ);
		SETOPT(CURLOPT_HTTPGET, 1L);
		ఆపు;

	స్థితి FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, ఖాళీ);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->దత్తాంశం.urlenc);
		ఆపు;

	స్థితి FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, ఖాళీ);
		SETOPT(CURLOPT_HTTPGET, 0L);
		ఒకవేళ (f->curl_postdata == ఖాళీ) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->దత్తాంశం.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		ఆపు;
	}
	తిరిగి సంకేతం_2;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
స్థిర CURLcode fetch_curl_సమితి_options(నిర్మాణం curl_fetch_info *f)
{
	CURLcode సంకేతం_2;
	స్థిరరాశి అక్షరం *auth;

#నిర్వచనం_రద్దు SETOPT
#నిర్వచించు SETOPT(option, విలువ) { \
	సంకేతం_2 = curl_easy_setopt(f->curl_handle, option, విలువ);	\
	ఒకవేళ (సంకేతం_2 != CURLE_OK)					\
		తిరిగి సంకేతం_2;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_వ్యక్తిగత, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_పురోగతి_దత్తాంశం, f);
	SETOPT(CURLOPT_HTTPHEADER, f->శీర్షికలు);
	సంకేతం_2 = fetch_curl_సమితి_postdata(f);
	ఒకవేళ (సంకేతం_2 != CURLE_OK) {
		తిరిగి సంకేతం_2;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	ఒకవేళ (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} లేకపోతే {
		SETOPT(CURLOPT_COOKIE, ఖాళీ);
	}

	ఒకవేళ ((auth = urldb_get_auth_details(f->url, ఖాళీ)) != ఖాళీ) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} లేకపోతే {
		SETOPT(CURLOPT_USERPWD, ఖాళీ);
	}

	/* set up proxy options */
	ఒకవేళ (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != ఖాళీ) &&
	    (అక్షరశ్రేణి_పోలిక(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (దీర్ఘ) nsoption_int(http_proxy_పోర్టు));

#ఒకవేళ LIBCURL_సంస్కరణ_అంకె >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#అయితే_ముగింపు

		ఒకవేళ (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(దీర్ఘ) CURLAUTH_BASIC :
					(దీర్ఘ) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					పరిమాణం fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} లేకపోతే {
		SETOPT(CURLOPT_PROXY, ఖాళీ);
	}


	ఒకవేళ (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	ఒకవేళ (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		ఒకవేళ (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, ఖాళీ);
			SETOPT(CURLOPT_SSL_CTX_దత్తాంశం, ఖాళీ);
		}
	} లేకపోతే {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#నిర్వచితమైతే WITH_OPENSSL
		ఒకవేళ (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_దత్తాంశం, f);
		}
#అయితే_ముగింపు
	}

	తిరిగి CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
స్థిర bool
fetch_curl_initiate_fetch(నిర్మాణం curl_fetch_info *fetch, CURL *handle)
{
	CURLcode సంకేతం_2;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	సంకేతం_2 = fetch_curl_సమితి_options(fetch);
	ఒకవేళ (సంకేతం_2 != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		తిరిగి false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_కూడు_handle(fetch_curl_multi, fetch->curl_handle);
	ప్రతిజ్ఞా(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	తిరిగి true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
స్థిర CURL *fetch_curl_get_handle(lwc_string *host)
{
	నిర్మాణం cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	ఒకవేళ (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		విడుదల(h);
	} లేకపోతే {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	తిరిగి ret;
}


/**
 * Dispatch a single job
 */
స్థిర bool fetch_curl_ప్రారంభం(శూన్య *vfetch)
{
	నిర్మాణం curl_fetch_info *fetch = (నిర్మాణం curl_fetch_info*)vfetch;
	ఒకవేళ (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		తిరిగి false;
	}
	తిరిగి fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
స్థిర శూన్య fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#ఒకవేళ LIBCURL_సంస్కరణ_అంకె >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	తిరిగి;
#లేకపోతే
	నిర్మాణం cache_handle *h = 0;
	పూర్ణాంకం c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	ఒకవేళ (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		తిరిగి;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(నిర్మాణం cache_handle, curl_handle_ring, c);
	ఒకవేళ (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		ఒకవేళ (curl_handle_ring != ఖాళీ) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_తదుపరి;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} లేకపోతే {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		తిరిగి;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (నిర్మాణం cache_handle*)జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_చొప్పించు(curl_handle_ring, h);
#అయితే_ముగింపు
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
స్థిర శూన్య fetch_curl_stop(నిర్మాణం curl_fetch_info *f)
{
	CURLMcode codem;

	ప్రతిజ్ఞా(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	ఒకవేళ (f->curl_handle) {
		/* remove from curl multi handle */
		codem = curl_multi_remove_handle(fetch_curl_multi,
				f->curl_handle);
		ప్రతిజ్ఞా(codem == CURLM_OK);
		/* Put this curl handle into the cache if wanted. */
		fetch_curl_cache_handle(f->curl_handle, f->host);
		f->curl_handle = 0;
	}

	fetch_remove_from_queues(f->fetch_handle);
}


/**
 * Abort a fetch.
 */
స్థిర శూన్య fetch_curl_abort(శూన్య *vf)
{
	నిర్మాణం curl_fetch_info *f = (నిర్మాణం curl_fetch_info *)vf;
	ప్రతిజ్ఞా(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	ఒకవేళ (f->curl_handle) {
		ఒకవేళ (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->abort = true;
		} లేకపోతే {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} లేకపోతే {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
స్థిర శూన్య fetch_curl_free(శూన్య *vf)
{
	నిర్మాణం curl_fetch_info *f = (నిర్మాణం curl_fetch_info *)vf;
	పూర్ణాంకం i;

	ఒకవేళ (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	విడుదల(f->location);
	విడుదల(f->cookie_string);
	విడుదల(f->realm);
	ఒకవేళ (f->శీర్షికలు) {
		curl_slist_free_all(f->శీర్షికలు);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	కొరకు (i = 0; i < MAX_CERT_లోతు; i++) {
		ఒకవేళ (f->cert_దత్తాంశం[i].cert != ఖాళీ) {
			X509_free(f->cert_దత్తాంశం[i].cert);
		}
	}

	విడుదల(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
స్థిర bool fetch_curl_process_శీర్షికలు(నిర్మాణం curl_fetch_info *f)
{
	దీర్ఘ http_సంకేతం;
	CURLcode సంకేతం_2;
	fetch_సందేశం సందేశం;

	f->had_శీర్షికలు = true;

	ఒకవేళ (!f->http_సంకేతం) {
		సంకేతం_2 = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_సంకేతం,
					 &f->http_సంకేతం);
		ప్రతిజ్ఞా(సంకేతం_2 == CURLE_OK);
		fetch_సమితి_http_సంకేతం(f->fetch_handle, f->http_సంకేతం);
	}
	http_సంకేతం = f->http_సంకేతం;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_సంకేతం);

	ఒకవేళ ((http_సంకేతం == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		సందేశం.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&సందేశం, f->fetch_handle);
		తిరిగి true;
	}

	/* handle HTTP redirects (3xx response codes) */
	ఒకవేళ (HTTP_RESPONSE_IS_3XX(http_సంకేతం) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		సందేశం.type = FETCH_REDIRECT;
		సందేశం.దత్తాంశం.redirect = f->location;
		fetch_send_callback(&సందేశం, f->fetch_handle);
		తిరిగి true;
	}

	/* handle HTTP 401 (Authentication errors) */
	ఒకవేళ (http_సంకేతం == HTTP_RESPONSE_UNAUTHORIZED) {
		సందేశం.type = FETCH_AUTH;
		సందేశం.దత్తాంశం.auth.realm = f->realm;
		fetch_send_callback(&సందేశం, f->fetch_handle);
		తిరిగి true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	ఒకవేళ (f->only_2xx &&
	    అక్షరశ్రేణి_పోలిక(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_సంకేతం) == false)) {
		సందేశం.type = FETCH_ERROR;
		సందేశం.దత్తాంశం.error = messages_get("Not2xx");
		fetch_send_callback(&సందేశం, f->fetch_handle);
		తిరిగి true;
	}

	ఒకవేళ (f->abort)
		తిరిగి true;

	తిరిగి false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
స్థిర శూన్య fetch_curl_పూర్తి(CURL *curl_handle, CURLcode ఫలితం)
{
	bool ముగిసిన = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	నిర్మాణం curl_fetch_info *f;
	అక్షరం **_hideous_hack = (అక్షరం **) (శూన్య *) &f;
	CURLcode సంకేతం_2;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	సంకేతం_2 = curl_easy_getinfo(curl_handle, CURLINFO_వ్యక్తిగత, _hideous_hack);
	ప్రతిజ్ఞా(సంకేతం_2 == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	ఒకవేళ ((abort_fetch == false) &&
	    (ఫలితం == CURLE_OK ||
	     ((ఫలితం == CURLE_WRITE_ERROR) && (f->stopped == false)))) {
		/* fetch completed normally or the server fed us a junk gzip
		 * stream (usually in the form of garbage at the end of the
		 * stream). Curl will have fed us all but the last chunk of
		 * decoded data, which is sad as, if we'd received the last
		 * chunk, too, we'd be able to render the whole object.
		 * As is, we'll just have to accept that the end of the
		 * object will be truncated in this case and leave it to
		 * the content handlers to cope.
		 */
		ఒకవేళ (f->stopped ||
		    (!f->had_శీర్షికలు &&	fetch_curl_process_శీర్షికలు(f))) {
			; /* redirect with no body or similar */
		} లేకపోతే {
			ముగిసిన = true;
		}
	} లేకపోతే ఒకవేళ (ఫలితం == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		ఒకవేళ (!f->had_శీర్షికలు && fetch_curl_process_శీర్షికలు(f))
			; /* redirect with partial body, or similar */
		లేకపోతే {
			ముగిసిన = true;
		}
	} లేకపోతే ఒకవేళ (ఫలితం == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} లేకపోతే ఒకవేళ (ఫలితం == CURLE_SSL_PEER_CERTIFICATE ||
		   ఫలితం == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} లేకపోతే {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", ఫలితం);
		error = true;
	}

	fetch_curl_stop(f);

	ఒకవేళ (f->పంపిన_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	ఒకవేళ (abort_fetch) {
		; /* fetch was aborted: no callback */
	} లేకపోతే ఒకవేళ (ముగిసిన) {
		fetch_సందేశం సందేశం;
		సందేశం.type = FETCH_ముగిసిన;
		fetch_send_callback(&సందేశం, f->fetch_handle);
	} లేకపోతే ఒకవేళ (cert) {
		/* user needs to validate certificate with issue */
		fetch_సందేశం సందేశం;
		సందేశం.type = FETCH_CERT_ERR;
		fetch_send_callback(&సందేశం, f->fetch_handle);
	} లేకపోతే ఒకవేళ (error) {
		fetch_సందేశం సందేశం;
		ఎంపిక (ఫలితం) {
		స్థితి CURLE_SSL_CONNECT_ERROR:
			సందేశం.type = FETCH_SSL_ERR;
			ఆపు;

		స్థితి CURLE_OPERATION_TIMEDOUT:
			సందేశం.type = FETCH_TIMEDOUT;
			సందేశం.దత్తాంశం.error = curl_easy_strerror(ఫలితం);
			ఆపు;

		అప్రమేయ:
			సందేశం.type = FETCH_ERROR;
			సందేశం.దత్తాంశం.error = curl_easy_strerror(ఫలితం);
		}

		fetch_send_callback(&సందేశం, f->fetch_handle);
	}

	fetch_free(f->fetch_handle);
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */
స్థిర శూన్య fetch_curl_poll(lwc_string *scheme_ignored)
{
	పూర్ణాంకం running, వరుస;
	CURLMcode codem;
	CURLMsg *curl_సందేశం;

	ఒకవేళ (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_సమితి, write_fd_సమితి, exc_fd_సమితి;
		పూర్ణాంకం max_fd = -1;
		పూర్ణాంకం i;

		FD_ZERO(&read_fd_సమితి);
		FD_ZERO(&write_fd_సమితి);
		FD_ZERO(&exc_fd_సమితి);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_సమితి, &write_fd_సమితి,
				&exc_fd_సమితి, &max_fd);
		ప్రతిజ్ఞా(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		కొరకు (i = 0; i <= max_fd; i++) {
			bool పఠ = false;
			bool లిఖ = false;
			bool error = false;

			ఒకవేళ (FD_ISSET(i, &read_fd_సమితి)) {
				పఠ = true;
			}
			ఒకవేళ (FD_ISSET(i, &write_fd_సమితి)) {
				లిఖ = true;
			}
			ఒకవేళ (FD_ISSET(i, &exc_fd_సమితి)) {
				error = true;
			}
			ఒకవేళ (పఠ || లిఖ || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      పఠ ? "read" : "    ",
				      లిఖ ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	చేయి {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		ఒకవేళ (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			తిరిగి;
		}
	} అయితే (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_సందేశం = curl_multi_info_read(fetch_curl_multi, &వరుస);
	అయితే (curl_సందేశం) {
		ఎంపిక (curl_సందేశం->సందేశం) {
			స్థితి CURLMSG_పూర్తి:
				fetch_curl_పూర్తి(curl_సందేశం->easy_handle,
						curl_సందేశం->దత్తాంశం.ఫలితం);
				ఆపు;
			అప్రమేయ:
				ఆపు;
		}
		curl_సందేశం = curl_multi_info_read(fetch_curl_multi, &వరుస);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
స్థిర పూర్ణాంకం
fetch_curl_పురోగతి(శూన్య *clientp,
		    NSCURL_పురోగతి_T dltotal,
		    NSCURL_పురోగతి_T dlnow,
		    NSCURL_పురోగతి_T ultotal,
		    NSCURL_పురోగతి_T ulnow)
{
	స్థిర అక్షరం fetch_పురోగతి_బఫరు[256]; /**< Progress buffer for cURL */
	నిర్మాణం curl_fetch_info *f = (నిర్మాణం curl_fetch_info *) clientp;
	uint64_t సమయం_now_ms;
	fetch_సందేశం సందేశం;

	ఒకవేళ (f->abort) {
		తిరిగి 0;
        }

	సందేశం.type = FETCH_పురోగతి;
	సందేశం.దత్తాంశం.పురోగతి = fetch_పురోగతి_బఫరు;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&సమయం_now_ms);
#నిర్వచించు UPDATE_DELAY_MS (1000 / UPDATES_చొప్పున_రెండవ)
	ఒకవేళ (సమయం_now_ms - f->చివరి_పురోగతి_update < UPDATE_DELAY_MS) {
		తిరిగి 0;
        }
#నిర్వచనం_రద్దు UPDATE_DELAY_MS
	f->చివరి_పురోగతి_update = సమయం_now_ms;

	ఒకవేళ (dltotal > 0) {
		snprintf(fetch_పురోగతి_బఫరు, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&సందేశం, f->fetch_handle);
	} లేకపోతే {
		snprintf(fetch_పురోగతి_బఫరు, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&సందేశం, f->fetch_handle);
	}

	తిరిగి 0;
}


/**
 * Format curl debug for nslog
 */
స్థిర పూర్ణాంకం
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 అక్షరం *దత్తాంశం,
		 size_t కొలత,
		 శూన్య *userptr)
{
	స్థిర స్థిరరాశి అక్షరం s_infotype[CURLINFO_చివర][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	ఎంపిక(type) {
	స్థితి CURLINFO_పాఠ్యం:
	స్థితి CURLINFO_శీర్షిక_బహిర్గతం:
	స్థితి CURLINFO_శీర్షిక_లోపల:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (పూర్ణాంకం)కొలత - 1, దత్తాంశం);
		ఆపు;

	అప్రమేయ:
		ఆపు;
	}
	తిరిగి 0;
}


స్థిర curl_socket_t fetch_curl_socket_open(శూన్య *clientp,
		curlsocktype purpose, నిర్మాణం curl_sockaddr *చిరునామా)
{
	(శూన్య) clientp;
	(శూన్య) purpose;

	తిరిగి (curl_socket_t) guit->fetch->socket_open(
			చిరునామా->family, చిరునామా->socktype,
			చిరునామా->protocol);
}

స్థిర పూర్ణాంకం fetch_curl_socket_close(శూన్య *clientp, curl_socket_t అంశం)
{
	(శూన్య) clientp;

	తిరిగి guit->fetch->socket_close((పూర్ణాంకం) అంశం);
}

/**
 * Callback function for cURL.
 */
స్థిర size_t fetch_curl_దత్తాంశం(అక్షరం *దత్తాంశం, size_t కొలత, size_t nmemb, శూన్య *_f)
{
	నిర్మాణం curl_fetch_info *f = _f;
	CURLcode సంకేతం_2;
	fetch_సందేశం సందేశం;

	/* ensure we only have to get this information once */
	ఒకవేళ (!f->http_సంకేతం) {
		సంకేతం_2 = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_సంకేతం,
					 &f->http_సంకేతం);
		ప్రతిజ్ఞా(సంకేతం_2 == CURLE_OK);
		fetch_సమితి_http_సంకేతం(f->fetch_handle, f->http_సంకేతం);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	ఒకవేళ (f->http_సంకేతం == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_సంకేతం = 0;
		తిరిగి కొలత * nmemb;
	}

	ఒకవేళ (f->abort || (!f->had_శీర్షికలు && fetch_curl_process_శీర్షికలు(f))) {
		f->stopped = true;
		తిరిగి 0;
	}

	/* send data to the caller */
	సందేశం.type = FETCH_దత్తాంశం;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *) దత్తాంశం;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = కొలత * nmemb;
	fetch_send_callback(&సందేశం, f->fetch_handle);

	ఒకవేళ (f->abort) {
		f->stopped = true;
		తిరిగి 0;
	}

	తిరిగి కొలత * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
స్థిర size_t
fetch_curl_శీర్షిక(అక్షరం *దత్తాంశం, size_t కొలత, size_t nmemb, శూన్య *_f)
{
	నిర్మాణం curl_fetch_info *f = _f;
	పూర్ణాంకం i;
	fetch_సందేశం సందేశం;
	కొలత *= nmemb;

	ఒకవేళ (f->abort) {
		f->stopped = true;
		తిరిగి 0;
	}

	ఒకవేళ (f->పంపిన_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	సందేశం.type = FETCH_శీర్షిక;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *) దత్తాంశం;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = కొలత;
	fetch_send_callback(&సందేశం, f->fetch_handle);

#నిర్వచించు SKIP_ST(o) కొరకు (i = (o); i < (పూర్ణాంకం) కొలత && (దత్తాంశం[i] == ' ' || దత్తాంశం[i] == '\t'); i++)

	ఒకవేళ (12 < కొలత && strncasecmp(దత్తాంశం, "Location:", 9) == 0) {
		/* extract Location header */
		విడుదల(f->location);
		f->location = జ్ఞాపకం_ఇవ్వు(కొలత);
		ఒకవేళ (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			తిరిగి కొలత;
		}
		SKIP_ST(9);
		strncpy(f->location, దత్తాంశం + i, కొలత - i);
		f->location[కొలత - i] = '\0';
		కొరకు (i = కొలత - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} లేకపోతే ఒకవేళ (15 < కొలత && strncasecmp(దత్తాంశం, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		ఒకవేళ (i < (పూర్ణాంకం)కొలత && '0' <= దత్తాంశం[i] && దత్తాంశం[i] <= '9')
			f->content_నిడివి = atol(దత్తాంశం + i);
	} లేకపోతే ఒకవేళ (17 < కొలత && strncasecmp(దత్తాంశం, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		అయితే (i < (పూర్ణాంకం) కొలత - 5 &&
				strncasecmp(దత్తాంశం + i, "realm", 5))
			i++;
		అయితే (i < (పూర్ణాంకం) కొలత - 1 && దత్తాంశం[++i] != '"')
			/* */;
		i++;

		ఒకవేళ (i < (పూర్ణాంకం) కొలత) {
			size_t చివర = i;

			అయితే (చివర < కొలత && దత్తాంశం[చివర] != '"')
				++చివర;

			ఒకవేళ (చివర < కొలత) {
				విడుదల(f->realm);
				f->realm = జ్ఞాపకం_ఇవ్వు(చివర - i + 1);
				ఒకవేళ (f->realm != ఖాళీ) {
					strncpy(f->realm, దత్తాంశం + i, చివర - i);
					f->realm[చివర - i] = '\0';
				}
			}
		}
	} లేకపోతే ఒకవేళ (11 < కొలత && strncasecmp(దత్తాంశం, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_సమితి_cookie(f->fetch_handle, &దత్తాంశం[i]);
	}

	తిరిగి కొలత;
#నిర్వచనం_రద్దు SKIP_ST
}

స్థిర పూర్ణాంకం fetch_curl_fdset(lwc_string *scheme, fd_set *read_సమితి,
			    fd_set *write_సమితి, fd_set *error_సమితి)
{
	CURLMcode సంకేతం_2;
	పూర్ణాంకం maxfd = -1;

	సంకేతం_2 = curl_multi_fdset(fetch_curl_multi,
				read_సమితి,
				write_సమితి,
				error_సమితి,
				&maxfd);
	ప్రతిజ్ఞా(సంకేతం_2 == CURLM_OK);

	తిరిగి maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_రిజిస్టరు(శూన్య)
{
	CURLcode సంకేతం_2;
	curl_సంస్కరణ_info_దత్తాంశం *దత్తాంశం;
	పూర్ణాంకం i;
	lwc_string *scheme;
	స్థిరరాశి నిర్మాణం fetcher_operation_పట్టిక fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.అమరిక = fetch_curl_అమరిక,
		.ప్రారంభం = fetch_curl_ప్రారంభం,
		.abort = fetch_curl_abort,
		.విడుదల = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#ఒకవేళ LIBCURL_సంస్కరణ_అంకె >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, ఖాళీ, ఖాళీ);
	ఒకవేళ (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} లేకపోతే {
		curl_with_openssl = false;
	}
#అయితే_ముగింపు

	NSLOG(netsurf, INFO, "curl_version %s", curl_సంస్కరణ());

	సంకేతం_2 = curl_global_init(CURL_GLOBAL_ALL);
	ఒకవేళ (సంకేతం_2 != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		తిరిగి NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	ఒకవేళ (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		తిరిగి NSERROR_INIT_FAILED;
	}

#ఒకవేళ LIBCURL_సంస్కరణ_అంకె >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		పూర్ణాంకం maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#నిర్వచనం_రద్దు SETOPT
#నిర్వచించు SETOPT(option, విలువ) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, విలువ);	\
	ఒకవేళ (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		వెళ్ళు curl_multi_setopt_failed;				\
	}

		SETOPT(CURLMOPT_MAXCONNECTS, (దీర్ఘ)maxconnects);
		SETOPT(CURLMOPT_MAX_మొత్తం_CONNECTIONS, (దీర్ఘ)maxconnects);
		SETOPT(CURLMOPT_MAX_HOST_CONNECTIONS, (దీర్ఘ)nsoption_int(max_fetchers_చొప్పున_host));
	}
#అయితే_ముగింపు

	/* Create a curl easy handle with the options that are common to all
	 *  fetches.
	 */
	fetch_blank_curl = curl_easy_init();
	ఒకవేళ (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		తిరిగి NSERROR_INIT_FAILED;
	}

#నిర్వచనం_రద్దు SETOPT
#నిర్వచించు SETOPT(option, విలువ) \
	సంకేతం_2 = curl_easy_setopt(fetch_blank_curl, option, విలువ);	\
	ఒకవేళ (సంకేతం_2 != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		వెళ్ళు curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_బఫరు);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	ఒకవేళ (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} లేకపోతే {
		SETOPT(CURLOPT_VERBOSE, 1);
	}

	/* Currently we explode if curl uses HTTP2, so force 1.1. */
	SETOPT(CURLOPT_HTTP_సంస్కరణ, CURL_HTTP_సంస్కరణ_1_1);

	SETOPT(CURLOPT_WRITEFUNCTION, fetch_curl_దత్తాంశం);
	SETOPT(CURLOPT_HEADERFUNCTION, fetch_curl_శీర్షిక);
	SETOPT(NSCURLOPT_పురోగతి_FUNCTION, fetch_curl_పురోగతి);
	SETOPT(CURLOPT_NOPROGRESS, 0L);
	SETOPT(CURLOPT_USERAGENT, user_agent_string());
	SETOPT(CURLOPT_ENCODING, "gzip");
	SETOPT(CURLOPT_దిగువ_వేగం_హద్దు, 1L);
	SETOPT(CURLOPT_దిగువ_వేగం_సమయం, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (దీర్ఘ)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	ఒకవేళ (nsoption_charp(ca_bundle) &&
	    strcmp(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	ఒకవేళ (nsoption_charp(ca_మార్గం) && strcmp(nsoption_charp(ca_మార్గం), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_మార్గం));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_మార్గం));
	}

#ఒకవేళ LIBCURL_సంస్కరణ_అంకె < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	సంకేతం_2 = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, ఖాళీ);
	ఒకవేళ (సంకేతం_2 != CURLE_OK) {
		curl_with_openssl = false;
	} లేకపోతే {
		curl_with_openssl = true;
	}
#అయితే_ముగింపు

	ఒకవేళ (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#ఒకవేళ LIBCURL_సంస్కరణ_అంకె >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		సంకేతం_2 = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		ఒకవేళ (సంకేతం_2 != CURLE_OK && సంకేతం_2 != CURLE_NOT_BUILT_లోపల)
			వెళ్ళు curl_easy_setopt_failed;
#అయితే_ముగింపు
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	దత్తాంశం = curl_సంస్కరణ_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	ఒకవేళ (curl_fetch_ssl_hashmap == ఖాళీ) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		తిరిగి NSERROR_NOMEM;
	}

	కొరకు (i = 0; దత్తాంశం->protocols[i]; i++) {
		ఒకవేళ (strcmp(దత్తాంశం->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} లేకపోతే ఒకవేళ (strcmp(దత్తాంశం->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} లేకపోతే {
			/* Ignore non-http(s) protocols */
			కొనసాగు;
		}

		ఒకవేళ (fetcher_కూడు(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      దత్తాంశం->protocols[i]);
		}
	}

	తిరిగి NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	తిరిగి NSERROR_INIT_FAILED;

#ఒకవేళ LIBCURL_సంస్కరణ_అంకె >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	తిరిగి NSERROR_INIT_FAILED;
#అయితే_ముగింపు
}
