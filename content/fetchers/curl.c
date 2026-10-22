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
#تعریف_کرو UPDATES_فی_دوسرا 2

/**
 * The ciphersuites the browser is prepared to use for TLS1.3
 */
#تعریف_کرو CIPHER_SUITES						\
	"TLS_AES_256_GCM_SHA384:"				\
	"TLS_CHACHA20_POLY1305_SHA256:"				\
	"TLS_AES_128_GCM_SHA256"

/**
 * The ciphersuites the browser is prepared to use for TLS<1.3
 */
#تعریف_کرو CIPHER_LIST						\
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
#تعریف_کرو CIPHER_LIST_LEGACY					\
	/* as above */						\
	CIPHER_LIST":"						\
	/* enable (non-PFS) RSA AES GCM suites */		\
	"RSA+AESGCM:"						\
	/* enable (non-PFS) RSA (mac=SHA1) AES CBC suites */	\
	"RSA+AES+SHA1"

/* Open SSL compatability for certificate handling */
#اگر_تعریف_شدہ WITH_OPENSSL

#شامل <openssl/ssl.h>
#شامل <openssl/x509v3.h>

#ورنہ /* WITH_OPENSSL */

قسمتعریف حرف X509;

ساکن خالی X509_free(X509 *cert)
{
	آزاد(cert);
}

#اگر_ختم /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
ساکن uint32_t
curl_fetch_ssl_کلید_hash(خالی *کلید)
{
	nsurl *url = کلید;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *پورٹ = nsurl_get_component(url, NSURL_پورٹ);
	uint32_t hash;

	اگر (پورٹ == تہی)
		پورٹ = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_قدر(hostname) ^ lwc_string_hash_قدر(پورٹ);

	lwc_string_unref(hostname);
	lwc_string_unref(پورٹ);

	واپس hash;
}

/* We only compare the hostname and port */
ساکن bool
curl_fetch_ssl_کلید_eq(خالی *key1, خالی *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_پورٹ);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_پورٹ);
	bool iseq = false;

	اگر (port1 == تہی)
		port1 = lwc_string_ref(corestring_lwc_443);
	اگر (port2 == تہی)
		port2 = lwc_string_ref(corestring_lwc_443);

	اگر (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		جاؤ برآمد;

	iseq = false;
	اگر (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		جاؤ برآمد;

برآمد:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	واپس iseq;
}

ساکن خالی *
curl_fetch_ssl_قدر_alloc(خالی *کلید)
{
	ساخت cert_chain *برآمد;

	اگر (cert_chain_alloc(0, &برآمد) != NSERROR_OK) {
		واپس تہی;
	}

	واپس برآمد;
}

ساکن خالی
curl_fetch_ssl_قدر_destroy(خالی *قدر)
{
	ساخت cert_chain *chain = قدر;
	اگر (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

ساکن hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.کلید_clone = (hashmap_کلید_clone_t)nsurl_ref,
	.کلید_destroy = (hashmap_کلید_destroy_t)nsurl_unref,
	.کلید_eq = curl_fetch_ssl_کلید_eq,
	.کلید_hash = curl_fetch_ssl_کلید_hash,
	.قدر_alloc = curl_fetch_ssl_قدر_alloc,
	.قدر_destroy = curl_fetch_ssl_قدر_destroy,
};

ساکن hashmap_t *curl_fetch_ssl_hashmap = تہی;

/** SSL certificate info */
ساخت cert_info {
	X509 *cert;		/**< Pointer to certificate */
	طویل err;		/**< OpenSSL error code */
};

#اگر LIBCURL_نسخہ_عدد >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#تعریف_کرو NSCURLOPT_پیش_رفت_FUNCTION CURLOPT_XFERINFOFUNCTION
#تعریف_کرو NSCURLOPT_پیش_رفت_ڈیٹا CURLOPT_XFERINFODATA
#تعریف_کرو NSCURL_پیش_رفت_T curl_off_t
#ورنہ
#تعریف_کرو NSCURLOPT_پیش_رفت_FUNCTION CURLOPT_PROGRESSFUNCTION
#تعریف_کرو NSCURLOPT_پیش_رفت_ڈیٹا CURLOPT_PROGRESSDATA
#تعریف_کرو NSCURL_پیش_رفت_T دوہرا
#اگر_ختم

#اگر LIBCURL_نسخہ_عدد >= 0x073800 /* 7.56.0 depricated curl_formadd */
#تعریف_کرو NSCURL_POSTDATA_T curl_mime
#تعریف_کرو NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#تعریف_کرو NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#ورنہ
#تعریف_کرو NSCURL_POSTDATA_T ساخت curl_httppost
#تعریف_کرو NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#تعریف_کرو NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#اگر_ختم

/** Information for a single fetch. */
ساخت curl_fetch_info {
	ساخت fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool بھیجا_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_سرخیاں;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	ساخت curl_slist *سرخیاں;	/**< List of request headers. */
	حرف *location;		/**< Response Location header, or 0. */
	غیرعلامتی طویل content_لمبائی;	/**< Response Content-Length, or 0. */
	حرف *cookie_string;	/**< Cookie string for this fetch */
	حرف *realm;		/**< HTTP Auth Realm */
	ساخت fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	طویل http_رمز; /**< HTTP result code from cURL. */

	uint64_t آخری_پیش_رفت_update;	/**< Time of last progress update */
	صحیح cert_گہرائی; /**< deepest certificate in use */
	ساخت cert_info cert_ڈیٹا[MAX_CERT_گہرائی]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
ساخت cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	ساخت cache_handle *r_پچھلا; /**< Previous cached handle in ring. */
	ساخت cache_handle *r_اگلا; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
ساکن CURL *fetch_blank_curl;

/** Ring of cached handles */
ساکن ساخت cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
ساکن صحیح curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
ساکن bool curl_with_openssl;

/** Error buffer for cURL. */
ساکن حرف fetch_error_بفر[CURL_ERROR_سائز];

/** Proxy authentication details. */
ساکن حرف fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
ساکن bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
ساکن bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_ڈیٹا(scheme));
	curl_fetchers_registered++;
	واپس true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
ساکن خالی fetch_curl_finalise(lwc_string *scheme)
{
	ساخت cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_ڈیٹا(scheme));
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
		curl_fetch_ssl_hashmap = تہی;
	}

	/* Free anything remaining in the cached curl handle ring */
	جب (curl_handle_ring != تہی) {
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
ساکن bool fetch_curl_can_fetch(ثابت nsurl *url)
{
	واپس nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
ساکن ساخت fetch_postdata *
fetch_curl_alloc_postdata(ثابت حرف *post_urlenc,
			  ثابت ساخت fetch_multipart_ڈیٹا *post_multipart)
{
	ساخت fetch_postdata *postdata;
	postdata = صفر_حافظہ_دو(1, حجم(ساخت fetch_postdata));
	اگر (postdata != تہی) {

		اگر (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->ڈیٹا.urlenc = strdup(post_urlenc);
			اگر (postdata->ڈیٹا.urlenc == تہی) {
				آزاد(postdata);
				postdata = تہی;
			}
		} ورنہ اگر (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->ڈیٹا.multipart = fetch_multipart_ڈیٹا_clone(post_multipart);
			اگر (postdata->ڈیٹا.multipart == تہی) {
				آزاد(postdata);
				postdata = تہی;
			}
		} ورنہ {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	واپس postdata;
}

/**
 * free postdata
 */
ساکن خالی fetch_curl_free_postdata(ساخت fetch_postdata *postdata)
{
	اگر (postdata != تہی) {
		انتخاب (postdata->type) {
		حالت FETCH_POSTDATA_NONE:
			توڑو;
		حالت FETCH_POSTDATA_URLENC:
			آزاد(postdata->ڈیٹا.urlenc);
			توڑو;
		حالت FETCH_POSTDATA_MULTIPART:
			fetch_multipart_ڈیٹا_destroy(postdata->ڈیٹا.multipart);
			توڑو;
		}

		آزاد(postdata);
	}
}

/**
 *construct a new fetch structure
 */
ساکن ساخت curl_fetch_info *fetch_alloc(خالی)
{
	ساخت curl_fetch_info *fetch;
	fetch = حافظہ_دو(حجم (*fetch));
	اگر (fetch == تہی)
		واپس تہی;

	fetch->curl_handle = تہی;
	fetch->بھیجا_ssl_chain = false;
	fetch->had_سرخیاں = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->سرخیاں = تہی;
	fetch->url = تہی;
	fetch->host = تہی;
	fetch->location = تہی;
	fetch->content_لمبائی = 0;
	fetch->http_رمز = 0;
	fetch->cookie_string = تہی;
	fetch->realm = تہی;
	fetch->آخری_پیش_رفت_update = 0;
	fetch->postdata = تہی;
	fetch->curl_postdata = تہی;

	/* Clear certificate chain data */
	memset(fetch->cert_ڈیٹا, 0, حجم(fetch->cert_ڈیٹا));
	fetch->cert_گہرائی = -1;

	واپس fetch;
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
ساکن خالی *
fetch_curl_تیاری(ساخت fetch *والد_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ثابت حرف *post_urlenc,
		 ثابت ساخت fetch_multipart_ڈیٹا *post_multipart,
		 ثابت حرف **سرخیاں)
{
	ساخت curl_fetch_info *fetch;
	ساخت curl_slist *slist;
	صحیح i;

	fetch = fetch_alloc();
	اگر (fetch == تہی)
		واپس تہی;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = والد_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	اگر (fetch->host == تہی) {
		جاؤ failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	اگر (fetch->postdata == تہی) {
		جاؤ failed;
	}

#تعریف_کرو APPEND(list, قدر) \
	slist = curl_slist_append(list, قدر);		\
	اگر (slist == تہی)				\
		جاؤ failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->سرخیاں, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->سرخیاں, "Expect:");

	اگر ((nsoption_charp(accept_language) != تہی) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		حرف s[80];
		snprintf(s, حجم s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[حجم s - 1] = 0;
		APPEND(fetch->سرخیاں, s);
	}

	اگر (nsoption_charp(accept_charset) != تہی &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		حرف s[80];
		snprintf(s, حجم s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[حجم s - 1] = 0;
		APPEND(fetch->سرخیاں, s);
	}

	اگر (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->سرخیاں, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	برائے (i = 0; سرخیاں[i] != تہی; i++) {
		APPEND(fetch->سرخیاں, سرخیاں[i]);
	}

	واپس fetch;

#تعریف_منسوخ APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->سرخیاں);
	آزاد(fetch);
	واپس تہی;
}


#اگر_تعریف_شدہ WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
ساکن ساخت cert_chain *
fetch_curl_get_cached_chain(ساخت curl_fetch_info *f)
{
	ساخت cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	اگر (chain == تہی) {
		chain = hashmap_داخل_کرو(curl_fetch_ssl_hashmap, f->url);
	}

	واپس chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
ساکن خالی
fetch_curl_store_certs_اندر_cache(ساخت curl_fetch_info *f)
{
	size_t گہرائی;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_گہرائی];
	ساخت cert_chain chain, *cached_chain;
	ساخت cert_info *certs;

	memset(&chain, 0, حجم(chain));

	certs = f->cert_ڈیٹا;
	chain.گہرائی = f->cert_گہرائی + 1; /* 0 indexed certificate depth */

	برائے (گہرائی = 0; گہرائی < chain.گہرائی; گہرائی++) {
		اگر (certs[گہرائی].cert == تہی) {
			/* This certificate is missing, skip it */
			chain.certs[گہرائی].err = SSL_CERT_ERR_CERT_MISSING;
			جاری;
		}

		/* error code (if any) */
		انتخاب (certs[گہرائی].err) {
		حالت X509_V_OK:
			chain.certs[گہرائی].err = SSL_CERT_ERR_OK;
			توڑو;

		حالت X509_V_ERR_UNABLE_مقصد_GET_ISSUER_CERT:
			/* fallthrough */
		حالت X509_V_ERR_UNABLE_مقصد_DECODE_ISSUER_عوامی_کلید:
			chain.certs[گہرائی].err = SSL_CERT_ERR_BAD_ISSUER;
			توڑو;

		حالت X509_V_ERR_UNABLE_مقصد_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		حالت X509_V_ERR_UNABLE_مقصد_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		حالت X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		حالت X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[گہرائی].err = SSL_CERT_ERR_BAD_SIG;
			توڑو;

		حالت X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		حالت X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[گہرائی].err = SSL_CERT_ERR_TOO_YOUNG;
			توڑو;

		حالت X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		حالت X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[گہرائی].err = SSL_CERT_ERR_TOO_OLD;
			توڑو;

		حالت X509_V_ERR_گہرائی_ZERO_SELF_SIGNED_CERT:
			chain.certs[گہرائی].err = SSL_CERT_ERR_SELF_SIGNED;
			توڑو;

		حالت X509_V_ERR_SELF_SIGNED_CERT_اندر_CHAIN:
			chain.certs[گہرائی].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			توڑو;

		حالت X509_V_ERR_CERT_REVOKED:
			chain.certs[گہرائی].err = SSL_CERT_ERR_REVOKED;
			توڑو;

		حالت X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[گہرائی].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			توڑو;

		طےشدہ:
			chain.certs[گہرائی].err = SSL_CERT_ERR_UNKNOWN;
			توڑو;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[گہرائی].cert);
		BIO_get_mem_ptr(mem, &buf[گہرائی]);
		(خالی) BIO_سیٹ_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[گہرائی].der = (uint8_t *)buf[گہرائی]->ڈیٹا;
		chain.certs[گہرائی].der_لمبائی = buf[گہرائی]->لمبائی;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	اگر (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	برائے (گہرائی = 0; گہرائی < chain.گہرائی; گہرائی++) {
		اگر (chain.certs[گہرائی].err == SSL_CERT_ERR_CERT_MISSING) {
			جاری;
		}
		اگر (buf[گہرائی] != تہی) {
			BUF_MEM_free(buf[گہرائی]);
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
ساکن صحیح
fetch_curl_verify_callback(صحیح verify_ok, X509_STORE_CTX *x509_ctx)
{
	صحیح گہرائی;
	ساخت curl_fetch_info *fetch;

	گہرائی = X509_STORE_CTX_get_error_گہرائی(x509_ctx);
	fetch = X509_STORE_CTX_get_app_ڈیٹا(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	اگر (گہرائی >= MAX_CERT_گہرائی) {
		X509_STORE_CTX_سیٹ_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		واپس 0;
	}

	/* record the max depth */
	اگر (گہرائی > fetch->cert_گہرائی) {
		fetch->cert_گہرائی = گہرائی;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	اگر (!fetch->cert_ڈیٹا[گہرائی].cert) {
		fetch->cert_ڈیٹا[گہرائی].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_اوپر_ref(fetch->cert_ڈیٹا[گہرائی].cert);
		fetch->cert_ڈیٹا[گہرائی].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	اگر (گہرائی > 0) {
		verify_ok = 1;
	} ورنہ {
		/* search for deeper certificates in the chain with errors */
		برائے (گہرائی = fetch->cert_گہرائی; گہرائی > 0; گہرائی--) {
			اگر (fetch->cert_ڈیٹا[گہرائی].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_سیٹ_error(x509_ctx, fetch->cert_ڈیٹا[گہرائی].err);
			}
		}
	}

	واپس verify_ok;
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
ساکن صحیح fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, خالی *parm)
{
	ساخت curl_fetch_info *f = (ساخت curl_fetch_info *) parm;
	صحیح ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_سیٹ_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_ڈیٹا(f->host),
					 lwc_string_لمبائی(f->host));

	/* Store fetch struct in context for verify callback */
	اگر (ok) {
		ok = X509_STORE_CTX_سیٹ_app_ڈیٹا(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	اگر (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_اندر_cache(f);

	واپس ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
ساکن CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, خالی *_sslctx, خالی *parm)
{
	ساخت curl_fetch_info *f = (ساخت curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	طویل options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_سیٹ_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_سیٹ_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	اگر (f->downgrade_tls) {
		/* Disable TLS 1.3 if the server can't cope with it */
#اگر_تعریف_شدہ SSL_OP_NO_TLSv1_3
		options |= SSL_OP_NO_TLSv1_3;
#اگر_ختم
#اگر_تعریف_شدہ SSL_منوال_SEND_FALLBACK_SCSV
		/* Ensure server rejects the connection if downgraded too far */
		SSL_CTX_سیٹ_منوال(sslctx, SSL_منوال_SEND_FALLBACK_SCSV);
#اگر_ختم
	}

	SSL_CTX_سیٹ_options(sslctx, options);

#اگر_تعریف_شدہ SSL_OP_NO_TICKET
	SSL_CTX_clear_options(sslctx, SSL_OP_NO_TICKET);
#اگر_ختم

	واپس CURLE_OK;
}


#اگر_ختم /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
ساکن خالی
fetch_curl_report_certs_upstream(ساخت curl_fetch_info *f)
{
	fetch_پیغام پیغام;
	ساخت cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	اگر (chain != تہی) {
		پیغام.type = FETCH_CERTS;
		پیغام.ڈیٹا.chain = chain;

		fetch_send_callback(&پیغام, f->fetch_handle);
	}

	f->بھیجا_ssl_chain = true;
}

#اگر LIBCURL_نسخہ_عدد >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
ساخت curl_mime_ctx {
	حرف *بفر;
	curl_off_t سائز;
	curl_off_t position;
};

ساکن size_t mime_ڈیٹا_read_callback(حرف *بفر, size_t سائز, size_t nitems, خالی *arg)
{
	ساخت curl_mime_ctx *mctx = (ساخت curl_mime_ctx *) arg;
	curl_off_t sz = mctx->سائز - mctx->position;

	nitems *= سائز;
	اگر(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	اگر(sz) {
		حافظہ_نقل(بفر, mctx->بفر + mctx->position, sz);
	}
	mctx->position += sz;
	واپس sz;
}

ساکن صحیح mime_ڈیٹا_seek_callback(خالی *arg, curl_off_t انحراف, صحیح origin)
{
	ساخت curl_mime_ctx *mctx = (ساخت curl_mime_ctx *) arg;

	انتخاب(origin) {
	حالت SEEK_END:
		انحراف += mctx->سائز;
		توڑو;
	حالت SEEK_CUR:
		انحراف += mctx->position;
		توڑو;
	}

	اگر(انحراف < 0) {
		واپس CURL_SEEKFUNC_FAIL;
	}
	mctx->position = انحراف;
	واپس CURL_SEEKFUNC_OK;
}

ساکن خالی mime_ڈیٹا_free_callback(خالی *arg)
{
	ساخت curl_mime_ctx *mctx = (ساخت curl_mime_ctx *) arg;
	آزاد(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
ساکن curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    ثابت ساخت fetch_multipart_ڈیٹا *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode رمز = CURLE_OK;
	size_t قدر_len;

	cmime = curl_mime_init(chandle);
	اگر (cmime == تہی) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		واپس تہی;
	}

	/* iterate post data */
	برائے (; multipart != تہی; multipart = multipart->اگلا) {
		part = curl_mime_addpart(cmime);
		اگر (part == تہی) {
			جاؤ convert_failed;
		}

		رمز = curl_mime_name(part, multipart->name);
		اگر (رمز != CURLE_OK) {
			جاؤ convert_failed;
		}

		قدر_len = لڑی_لمبائی(multipart->قدر);

		اگر (multipart->file && قدر_len==0) {
			/* file entries with no filename require special handling */
			رمز=curl_mime_ڈیٹا(part, multipart->قدر, قدر_len);
			اگر (رمز != CURLE_OK) {
				جاؤ convert_failed;
			}

			رمز = curl_mime_filename(part, "");
			اگر (رمز != CURLE_OK) {
				جاؤ convert_failed;
			}

			رمز = curl_mime_type(part, "application/octet-stream");
			اگر (رمز != CURLE_OK) {
				جاؤ convert_failed;
			}

		} ورنہ اگر(multipart->file) {
			/* file entry */
			nserror ret;
			حرف *leafname = تہی;
			حرف *mimetype = تہی;

			رمز = curl_mime_filedata(part, multipart->rawfile);
			اگر (رمز != CURLE_OK) {
				جاؤ convert_failed;
			}

			ret = guit->file->basename(multipart->قدر, &leafname, تہی);
			اگر (ret != NSERROR_OK) {
				جاؤ convert_failed;
			}
			رمز = curl_mime_filename(part, leafname);
			آزاد(leafname);
			اگر (رمز != CURLE_OK) {
				جاؤ convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->قدر);
			اگر (mimetype == تہی) {
				mimetype=strdup("text/plain");
			}
			اگر (mimetype == تہی) {
				جاؤ convert_failed;
			}
			رمز = curl_mime_type(part, mimetype);
			آزاد(mimetype);
			اگر (رمز != CURLE_OK) {
				جاؤ convert_failed;
			}

		} ورنہ {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			ساخت curl_mime_ctx *cb_ctx;
			cb_ctx = حافظہ_دو(حجم(ساخت curl_mime_ctx));
			اگر (cb_ctx == تہی) {
				جاؤ convert_failed;
			}
			cb_ctx->بفر = multipart->قدر;
			cb_ctx->سائز = قدر_len;
			cb_ctx->position = 0;
			رمز = curl_mime_ڈیٹا_cb(part,
						 قدر_len,
						 mime_ڈیٹا_read_callback,
						 mime_ڈیٹا_seek_callback,
						 mime_ڈیٹا_free_callback,
						 cb_ctx);
			اگر (رمز != CURLE_OK) {
				آزاد(cb_ctx);
				جاؤ convert_failed;
			}
		}
	}

	واپس cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", رمز);
	curl_mime_free(cmime);
	واپس تہی;
}

#ورنہ /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
ساکن ساخت curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    ثابت ساخت fetch_multipart_ڈیٹا *control)
{
	ساخت curl_httppost *post = تہی, *آخری = تہی;
	CURLFORMcode رمز;
	nserror ret;

	برائے (; control; control = control->اگلا) {
		اگر (control->file) {
			حرف *leafname = تہی;
			ret = guit->file->basename(control->قدر, &leafname, تہی);
			اگر (ret != NSERROR_OK) {
				جاری;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			اگر (control->قدر[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				ساکن حرف buf;

				رمز = curl_formadd(&post, &آخری,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_بفر, control->قدر,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_انتہا);
				اگر (رمز != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", رمز,
					      control->name);
			} ورنہ {
				حرف *mimetype = guit->fetch->mimetype(control->قدر);
				رمز = curl_formadd(&post, &آخری,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_انتہا);
				اگر (رمز != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      رمز,
					      control->name,
					      control->قدر);
				آزاد(mimetype);
			}
			آزاد(leafname);
		} ورنہ {
			رمز = curl_formadd(&post, &آخری,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->قدر,
					    CURLFORM_انتہا);
			اگر (رمز != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", رمز,
				      control->name, control->قدر);
		}
	}

	واپس post;
}

#اگر_ختم  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
ساکن CURLcode fetch_curl_سیٹ_postdata(ساخت curl_fetch_info *f)
{
	CURLcode رمز = CURLE_OK;

#تعریف_منسوخ SETOPT
#تعریف_کرو SETOPT(option, قدر) { \
	رمز = curl_easy_setopt(f->curl_handle, option, قدر);	\
	اگر (رمز != CURLE_OK)					\
		واپس رمز;					\
	}

	انتخاب (f->postdata->type) {
	حالت FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, تہی);
		SETOPT(NSCURL_POSTDATA_CURLOPT, تہی);
		SETOPT(CURLOPT_HTTPGET, 1L);
		توڑو;

	حالت FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, تہی);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->ڈیٹا.urlenc);
		توڑو;

	حالت FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, تہی);
		SETOPT(CURLOPT_HTTPGET, 0L);
		اگر (f->curl_postdata == تہی) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->ڈیٹا.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		توڑو;
	}
	واپس رمز;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
ساکن CURLcode fetch_curl_سیٹ_options(ساخت curl_fetch_info *f)
{
	CURLcode رمز;
	ثابت حرف *auth;

#تعریف_منسوخ SETOPT
#تعریف_کرو SETOPT(option, قدر) { \
	رمز = curl_easy_setopt(f->curl_handle, option, قدر);	\
	اگر (رمز != CURLE_OK)					\
		واپس رمز;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_نجی, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_پیش_رفت_ڈیٹا, f);
	SETOPT(CURLOPT_HTTPHEADER, f->سرخیاں);
	رمز = fetch_curl_سیٹ_postdata(f);
	اگر (رمز != CURLE_OK) {
		واپس رمز;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	اگر (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} ورنہ {
		SETOPT(CURLOPT_COOKIE, تہی);
	}

	اگر ((auth = urldb_get_auth_details(f->url, تہی)) != تہی) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} ورنہ {
		SETOPT(CURLOPT_USERPWD, تہی);
	}

	/* set up proxy options */
	اگر (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != تہی) &&
	    (لڑی_موازنہ(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (طویل) nsoption_int(http_proxy_پورٹ));

#اگر LIBCURL_نسخہ_عدد >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#اگر_ختم

		اگر (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(طویل) CURLAUTH_BASIC :
					(طویل) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					حجم fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} ورنہ {
		SETOPT(CURLOPT_PROXY, تہی);
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
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, تہی);
			SETOPT(CURLOPT_SSL_CTX_ڈیٹا, تہی);
		}
	} ورنہ {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#اگر_تعریف_شدہ WITH_OPENSSL
		اگر (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_ڈیٹا, f);
		}
#اگر_ختم
	}

	واپس CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
ساکن bool
fetch_curl_initiate_fetch(ساخت curl_fetch_info *fetch, CURL *handle)
{
	CURLcode رمز;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	رمز = fetch_curl_سیٹ_options(fetch);
	اگر (رمز != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		واپس false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_جوڑو_handle(fetch_curl_multi, fetch->curl_handle);
	أكد(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	واپس true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
ساکن CURL *fetch_curl_get_handle(lwc_string *host)
{
	ساخت cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	اگر (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		آزاد(h);
	} ورنہ {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	واپس ret;
}


/**
 * Dispatch a single job
 */
ساکن bool fetch_curl_آغاز(خالی *vfetch)
{
	ساخت curl_fetch_info *fetch = (ساخت curl_fetch_info*)vfetch;
	اگر (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		واپس false;
	}
	واپس fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
ساکن خالی fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#اگر LIBCURL_نسخہ_عدد >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	واپس;
#ورنہ
	ساخت cache_handle *h = 0;
	صحیح c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	اگر (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		واپس;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(ساخت cache_handle, curl_handle_ring, c);
	اگر (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		اگر (curl_handle_ring != تہی) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_اگلا;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} ورنہ {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		واپس;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (ساخت cache_handle*)حافظہ_دو(حجم(ساخت cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_داخل_کرو(curl_handle_ring, h);
#اگر_ختم
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
ساکن خالی fetch_curl_stop(ساخت curl_fetch_info *f)
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
ساکن خالی fetch_curl_abort(خالی *vf)
{
	ساخت curl_fetch_info *f = (ساخت curl_fetch_info *)vf;
	أكد(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	اگر (f->curl_handle) {
		اگر (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->abort = true;
		} ورنہ {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} ورنہ {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
ساکن خالی fetch_curl_free(خالی *vf)
{
	ساخت curl_fetch_info *f = (ساخت curl_fetch_info *)vf;
	صحیح i;

	اگر (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	آزاد(f->location);
	آزاد(f->cookie_string);
	آزاد(f->realm);
	اگر (f->سرخیاں) {
		curl_slist_free_all(f->سرخیاں);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	برائے (i = 0; i < MAX_CERT_گہرائی; i++) {
		اگر (f->cert_ڈیٹا[i].cert != تہی) {
			X509_free(f->cert_ڈیٹا[i].cert);
		}
	}

	آزاد(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
ساکن bool fetch_curl_process_سرخیاں(ساخت curl_fetch_info *f)
{
	طویل http_رمز;
	CURLcode رمز;
	fetch_پیغام پیغام;

	f->had_سرخیاں = true;

	اگر (!f->http_رمز) {
		رمز = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_رمز,
					 &f->http_رمز);
		أكد(رمز == CURLE_OK);
		fetch_سیٹ_http_رمز(f->fetch_handle, f->http_رمز);
	}
	http_رمز = f->http_رمز;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_رمز);

	اگر ((http_رمز == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		پیغام.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&پیغام, f->fetch_handle);
		واپس true;
	}

	/* handle HTTP redirects (3xx response codes) */
	اگر (HTTP_RESPONSE_IS_3XX(http_رمز) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		پیغام.type = FETCH_REDIRECT;
		پیغام.ڈیٹا.redirect = f->location;
		fetch_send_callback(&پیغام, f->fetch_handle);
		واپس true;
	}

	/* handle HTTP 401 (Authentication errors) */
	اگر (http_رمز == HTTP_RESPONSE_UNAUTHORIZED) {
		پیغام.type = FETCH_AUTH;
		پیغام.ڈیٹا.auth.realm = f->realm;
		fetch_send_callback(&پیغام, f->fetch_handle);
		واپس true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	اگر (f->only_2xx &&
	    لڑی_موازنہ(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_رمز) == false)) {
		پیغام.type = FETCH_ERROR;
		پیغام.ڈیٹا.error = messages_get("Not2xx");
		fetch_send_callback(&پیغام, f->fetch_handle);
		واپس true;
	}

	اگر (f->abort)
		واپس true;

	واپس false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
ساکن خالی fetch_curl_مکمل(CURL *curl_handle, CURLcode نتیجہ)
{
	bool ختم_شدہ = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	ساخت curl_fetch_info *f;
	حرف **_hideous_hack = (حرف **) (خالی *) &f;
	CURLcode رمز;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	رمز = curl_easy_getinfo(curl_handle, CURLINFO_نجی, _hideous_hack);
	أكد(رمز == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	اگر ((abort_fetch == false) &&
	    (نتیجہ == CURLE_OK ||
	     ((نتیجہ == CURLE_WRITE_ERROR) && (f->stopped == false)))) {
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
		    (!f->had_سرخیاں &&	fetch_curl_process_سرخیاں(f))) {
			; /* redirect with no body or similar */
		} ورنہ {
			ختم_شدہ = true;
		}
	} ورنہ اگر (نتیجہ == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		اگر (!f->had_سرخیاں && fetch_curl_process_سرخیاں(f))
			; /* redirect with partial body, or similar */
		ورنہ {
			ختم_شدہ = true;
		}
	} ورنہ اگر (نتیجہ == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} ورنہ اگر (نتیجہ == CURLE_SSL_PEER_CERTIFICATE ||
		   نتیجہ == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} ورنہ {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", نتیجہ);
		error = true;
	}

	fetch_curl_stop(f);

	اگر (f->بھیجا_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	اگر (abort_fetch) {
		; /* fetch was aborted: no callback */
	} ورنہ اگر (ختم_شدہ) {
		fetch_پیغام پیغام;
		پیغام.type = FETCH_ختم_شدہ;
		fetch_send_callback(&پیغام, f->fetch_handle);
	} ورنہ اگر (cert) {
		/* user needs to validate certificate with issue */
		fetch_پیغام پیغام;
		پیغام.type = FETCH_CERT_ERR;
		fetch_send_callback(&پیغام, f->fetch_handle);
	} ورنہ اگر (error) {
		fetch_پیغام پیغام;
		انتخاب (نتیجہ) {
		حالت CURLE_SSL_CONNECT_ERROR:
			پیغام.type = FETCH_SSL_ERR;
			توڑو;

		حالت CURLE_OPERATION_TIMEDOUT:
			پیغام.type = FETCH_TIMEDOUT;
			پیغام.ڈیٹا.error = curl_easy_strerror(نتیجہ);
			توڑو;

		طےشدہ:
			پیغام.type = FETCH_ERROR;
			پیغام.ڈیٹا.error = curl_easy_strerror(نتیجہ);
		}

		fetch_send_callback(&پیغام, f->fetch_handle);
	}

	fetch_free(f->fetch_handle);
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */
ساکن خالی fetch_curl_poll(lwc_string *scheme_ignored)
{
	صحیح running, قطار;
	CURLMcode codem;
	CURLMsg *curl_پیغام;

	اگر (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_سیٹ, write_fd_سیٹ, exc_fd_سیٹ;
		صحیح max_fd = -1;
		صحیح i;

		FD_ZERO(&read_fd_سیٹ);
		FD_ZERO(&write_fd_سیٹ);
		FD_ZERO(&exc_fd_سیٹ);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_سیٹ, &write_fd_سیٹ,
				&exc_fd_سیٹ, &max_fd);
		أكد(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		برائے (i = 0; i <= max_fd; i++) {
			bool اقرأ = false;
			bool اكتب = false;
			bool error = false;

			اگر (FD_ISSET(i, &read_fd_سیٹ)) {
				اقرأ = true;
			}
			اگر (FD_ISSET(i, &write_fd_سیٹ)) {
				اكتب = true;
			}
			اگر (FD_ISSET(i, &exc_fd_سیٹ)) {
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
	کرو {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		اگر (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			واپس;
		}
	} جب (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_پیغام = curl_multi_info_read(fetch_curl_multi, &قطار);
	جب (curl_پیغام) {
		انتخاب (curl_پیغام->پیغام) {
			حالت CURLMSG_مکمل:
				fetch_curl_مکمل(curl_پیغام->easy_handle,
						curl_پیغام->ڈیٹا.نتیجہ);
				توڑو;
			طےشدہ:
				توڑو;
		}
		curl_پیغام = curl_multi_info_read(fetch_curl_multi, &قطار);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
ساکن صحیح
fetch_curl_پیش_رفت(خالی *clientp,
		    NSCURL_پیش_رفت_T dltotal,
		    NSCURL_پیش_رفت_T dlnow,
		    NSCURL_پیش_رفت_T ultotal,
		    NSCURL_پیش_رفت_T ulnow)
{
	ساکن حرف fetch_پیش_رفت_بفر[256]; /**< Progress buffer for cURL */
	ساخت curl_fetch_info *f = (ساخت curl_fetch_info *) clientp;
	uint64_t وقت_now_ms;
	fetch_پیغام پیغام;

	اگر (f->abort) {
		واپس 0;
        }

	پیغام.type = FETCH_پیش_رفت;
	پیغام.ڈیٹا.پیش_رفت = fetch_پیش_رفت_بفر;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&وقت_now_ms);
#تعریف_کرو UPDATE_DELAY_MS (1000 / UPDATES_فی_دوسرا)
	اگر (وقت_now_ms - f->آخری_پیش_رفت_update < UPDATE_DELAY_MS) {
		واپس 0;
        }
#تعریف_منسوخ UPDATE_DELAY_MS
	f->آخری_پیش_رفت_update = وقت_now_ms;

	اگر (dltotal > 0) {
		snprintf(fetch_پیش_رفت_بفر, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&پیغام, f->fetch_handle);
	} ورنہ {
		snprintf(fetch_پیش_رفت_بفر, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&پیغام, f->fetch_handle);
	}

	واپس 0;
}


/**
 * Format curl debug for nslog
 */
ساکن صحیح
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 حرف *ڈیٹا,
		 size_t سائز,
		 خالی *userptr)
{
	ساکن ثابت حرف s_infotype[CURLINFO_انتہا][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	انتخاب(type) {
	حالت CURLINFO_متن:
	حالت CURLINFO_سرخی_برآمد:
	حالت CURLINFO_سرخی_اندر:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (صحیح)سائز - 1, ڈیٹا);
		توڑو;

	طےشدہ:
		توڑو;
	}
	واپس 0;
}


ساکن curl_socket_t fetch_curl_socket_open(خالی *clientp,
		curlsocktype purpose, ساخت curl_sockaddr *پتہ)
{
	(خالی) clientp;
	(خالی) purpose;

	واپس (curl_socket_t) guit->fetch->socket_open(
			پتہ->family, پتہ->socktype,
			پتہ->protocol);
}

ساکن صحیح fetch_curl_socket_close(خالی *clientp, curl_socket_t شے)
{
	(خالی) clientp;

	واپس guit->fetch->socket_close((صحیح) شے);
}

/**
 * Callback function for cURL.
 */
ساکن size_t fetch_curl_ڈیٹا(حرف *ڈیٹا, size_t سائز, size_t nmemb, خالی *_f)
{
	ساخت curl_fetch_info *f = _f;
	CURLcode رمز;
	fetch_پیغام پیغام;

	/* ensure we only have to get this information once */
	اگر (!f->http_رمز) {
		رمز = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_رمز,
					 &f->http_رمز);
		أكد(رمز == CURLE_OK);
		fetch_سیٹ_http_رمز(f->fetch_handle, f->http_رمز);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	اگر (f->http_رمز == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_رمز = 0;
		واپس سائز * nmemb;
	}

	اگر (f->abort || (!f->had_سرخیاں && fetch_curl_process_سرخیاں(f))) {
		f->stopped = true;
		واپس 0;
	}

	/* send data to the caller */
	پیغام.type = FETCH_ڈیٹا;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *) ڈیٹا;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = سائز * nmemb;
	fetch_send_callback(&پیغام, f->fetch_handle);

	اگر (f->abort) {
		f->stopped = true;
		واپس 0;
	}

	واپس سائز * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
ساکن size_t
fetch_curl_سرخی(حرف *ڈیٹا, size_t سائز, size_t nmemb, خالی *_f)
{
	ساخت curl_fetch_info *f = _f;
	صحیح i;
	fetch_پیغام پیغام;
	سائز *= nmemb;

	اگر (f->abort) {
		f->stopped = true;
		واپس 0;
	}

	اگر (f->بھیجا_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	پیغام.type = FETCH_سرخی;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *) ڈیٹا;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = سائز;
	fetch_send_callback(&پیغام, f->fetch_handle);

#تعریف_کرو SKIP_ST(o) برائے (i = (o); i < (صحیح) سائز && (ڈیٹا[i] == ' ' || ڈیٹا[i] == '\t'); i++)

	اگر (12 < سائز && strncasecmp(ڈیٹا, "Location:", 9) == 0) {
		/* extract Location header */
		آزاد(f->location);
		f->location = حافظہ_دو(سائز);
		اگر (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			واپس سائز;
		}
		SKIP_ST(9);
		strncpy(f->location, ڈیٹا + i, سائز - i);
		f->location[سائز - i] = '\0';
		برائے (i = سائز - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} ورنہ اگر (15 < سائز && strncasecmp(ڈیٹا, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		اگر (i < (صحیح)سائز && '0' <= ڈیٹا[i] && ڈیٹا[i] <= '9')
			f->content_لمبائی = atol(ڈیٹا + i);
	} ورنہ اگر (17 < سائز && strncasecmp(ڈیٹا, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		جب (i < (صحیح) سائز - 5 &&
				strncasecmp(ڈیٹا + i, "realm", 5))
			i++;
		جب (i < (صحیح) سائز - 1 && ڈیٹا[++i] != '"')
			/* */;
		i++;

		اگر (i < (صحیح) سائز) {
			size_t انتہا = i;

			جب (انتہا < سائز && ڈیٹا[انتہا] != '"')
				++انتہا;

			اگر (انتہا < سائز) {
				آزاد(f->realm);
				f->realm = حافظہ_دو(انتہا - i + 1);
				اگر (f->realm != تہی) {
					strncpy(f->realm, ڈیٹا + i, انتہا - i);
					f->realm[انتہا - i] = '\0';
				}
			}
		}
	} ورنہ اگر (11 < سائز && strncasecmp(ڈیٹا, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_سیٹ_cookie(f->fetch_handle, &ڈیٹا[i]);
	}

	واپس سائز;
#تعریف_منسوخ SKIP_ST
}

ساکن صحیح fetch_curl_fdset(lwc_string *scheme, fd_set *read_سیٹ,
			    fd_set *write_سیٹ, fd_set *error_سیٹ)
{
	CURLMcode رمز;
	صحیح maxfd = -1;

	رمز = curl_multi_fdset(fetch_curl_multi,
				read_سیٹ,
				write_سیٹ,
				error_سیٹ,
				&maxfd);
	أكد(رمز == CURLM_OK);

	واپس maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_رجسٹر(خالی)
{
	CURLcode رمز;
	curl_نسخہ_info_ڈیٹا *ڈیٹا;
	صحیح i;
	lwc_string *scheme;
	ثابت ساخت fetcher_operation_جدول fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.تیاری = fetch_curl_تیاری,
		.آغاز = fetch_curl_آغاز,
		.abort = fetch_curl_abort,
		.آزاد = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#اگر LIBCURL_نسخہ_عدد >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, تہی, تہی);
	اگر (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} ورنہ {
		curl_with_openssl = false;
	}
#اگر_ختم

	NSLOG(netsurf, INFO, "curl_version %s", curl_نسخہ());

	رمز = curl_global_init(CURL_GLOBAL_ALL);
	اگر (رمز != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		واپس NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	اگر (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		واپس NSERROR_INIT_FAILED;
	}

#اگر LIBCURL_نسخہ_عدد >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		صحیح maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#تعریف_منسوخ SETOPT
#تعریف_کرو SETOPT(option, قدر) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, قدر);	\
	اگر (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		جاؤ curl_multi_setopt_failed;				\
	}

		SETOPT(CURLMOPT_MAXCONNECTS, (طویل)maxconnects);
		SETOPT(CURLMOPT_MAX_کل_CONNECTIONS, (طویل)maxconnects);
		SETOPT(CURLMOPT_MAX_HOST_CONNECTIONS, (طویل)nsoption_int(max_fetchers_فی_host));
	}
#اگر_ختم

	/* Create a curl easy handle with the options that are common to all
	 *  fetches.
	 */
	fetch_blank_curl = curl_easy_init();
	اگر (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		واپس NSERROR_INIT_FAILED;
	}

#تعریف_منسوخ SETOPT
#تعریف_کرو SETOPT(option, قدر) \
	رمز = curl_easy_setopt(fetch_blank_curl, option, قدر);	\
	اگر (رمز != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		جاؤ curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_بفر);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	اگر (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} ورنہ {
		SETOPT(CURLOPT_VERBOSE, 1);
	}

	/* Currently we explode if curl uses HTTP2, so force 1.1. */
	SETOPT(CURLOPT_HTTP_نسخہ, CURL_HTTP_نسخہ_1_1);

	SETOPT(CURLOPT_WRITEFUNCTION, fetch_curl_ڈیٹا);
	SETOPT(CURLOPT_HEADERFUNCTION, fetch_curl_سرخی);
	SETOPT(NSCURLOPT_پیش_رفت_FUNCTION, fetch_curl_پیش_رفت);
	SETOPT(CURLOPT_NOPROGRESS, 0L);
	SETOPT(CURLOPT_USERAGENT, user_agent_string());
	SETOPT(CURLOPT_ENCODING, "gzip");
	SETOPT(CURLOPT_کم_رفتار_حد_بندی, 1L);
	SETOPT(CURLOPT_کم_رفتار_وقت, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (طویل)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	اگر (nsoption_charp(ca_bundle) &&
	    strcmp(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	اگر (nsoption_charp(ca_راستہ) && strcmp(nsoption_charp(ca_راستہ), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_راستہ));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_راستہ));
	}

#اگر LIBCURL_نسخہ_عدد < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	رمز = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, تہی);
	اگر (رمز != CURLE_OK) {
		curl_with_openssl = false;
	} ورنہ {
		curl_with_openssl = true;
	}
#اگر_ختم

	اگر (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#اگر LIBCURL_نسخہ_عدد >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		رمز = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		اگر (رمز != CURLE_OK && رمز != CURLE_NOT_BUILT_اندر)
			جاؤ curl_easy_setopt_failed;
#اگر_ختم
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	ڈیٹا = curl_نسخہ_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	اگر (curl_fetch_ssl_hashmap == تہی) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		واپس NSERROR_NOMEM;
	}

	برائے (i = 0; ڈیٹا->protocols[i]; i++) {
		اگر (strcmp(ڈیٹا->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} ورنہ اگر (strcmp(ڈیٹا->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} ورنہ {
			/* Ignore non-http(s) protocols */
			جاری;
		}

		اگر (fetcher_جوڑو(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      ڈیٹا->protocols[i]);
		}
	}

	واپس NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	واپس NSERROR_INIT_FAILED;

#اگر LIBCURL_نسخہ_عدد >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	واپس NSERROR_INIT_FAILED;
#اگر_ختم
}
