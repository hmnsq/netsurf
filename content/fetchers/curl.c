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
#অন্তর্ভাব "utils/inet.h"

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <inttypes.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <sys/stat.h>

#অন্তর্ভাব <libwapcaplet/libwapcaplet.h>
#অন্তর্ভাব <nsutils/time.h>

#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/hashmap.h"
#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/ring.h"
#অন্তর্ভাব "utils/useragent.h"
#অন্তর্ভাব "utils/file.h"
#অন্তর্ভাব "utils/string.h"
#অন্তর্ভাব "netsurf/fetch.h"
#অন্তর্ভাব "netsurf/misc.h"
#অন্তর্ভাব "desktop/gui_internal.h"

#অন্তর্ভাব "content/fetch.h"
#অন্তর্ভাব "content/fetchers.h"
#অন্তর্ভাব "content/fetchers/curl.h"
#অন্তর্ভাব "content/urldb.h"

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

#অন্তর্ভাব <openssl/ssl.h>
#অন্তর্ভাব <openssl/x509v3.h>

#অন্যথা /* WITH_OPENSSL */

প্রকারলেখ অক্ষর X509;

স্থির শূন্য X509_free(X509 *cert)
{
	মুক্ত(cert);
}

#endif /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
স্থির uint32_t
curl_fetch_ssl_চাবি_hash(শূন্য *চাবি)
{
	nsurl *url = চাবি;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *port = nsurl_get_component(url, NSURL_PORT);
	uint32_t hash;

	যদি (port == ফাঁকা)
		port = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_মান(hostname) ^ lwc_string_hash_মান(port);

	lwc_string_unref(hostname);
	lwc_string_unref(port);

	ফেরত hash;
}

/* We only compare the hostname and port */
স্থির bool
curl_fetch_ssl_চাবি_eq(শূন্য *key1, শূন্য *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_PORT);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_PORT);
	bool iseq = false;

	যদি (port1 == ফাঁকা)
		port1 = lwc_string_ref(corestring_lwc_443);
	যদি (port2 == ফাঁকা)
		port2 = lwc_string_ref(corestring_lwc_443);

	যদি (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		যাও out;

	iseq = false;
	যদি (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		যাও out;

out:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	ফেরত iseq;
}

স্থির শূন্য *
curl_fetch_ssl_মান_alloc(শূন্য *চাবি)
{
	কাঠামো cert_chain *out;

	যদি (cert_chain_alloc(0, &out) != NSERROR_OK) {
		ফেরত ফাঁকা;
	}

	ফেরত out;
}

স্থির শূন্য
curl_fetch_ssl_মান_destroy(শূন্য *মান)
{
	কাঠামো cert_chain *chain = মান;
	যদি (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

স্থির hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.চাবি_clone = (hashmap_চাবি_clone_t)nsurl_ref,
	.চাবি_destroy = (hashmap_চাবি_destroy_t)nsurl_unref,
	.চাবি_eq = curl_fetch_ssl_চাবি_eq,
	.চাবি_hash = curl_fetch_ssl_চাবি_hash,
	.মান_alloc = curl_fetch_ssl_মান_alloc,
	.মান_destroy = curl_fetch_ssl_মান_destroy,
};

স্থির hashmap_t *curl_fetch_ssl_hashmap = ফাঁকা;

/** SSL certificate info */
কাঠামো cert_info {
	X509 *cert;		/**< Pointer to certificate */
	long err;		/**< OpenSSL error code */
};

#যদি LIBCURL_VERSION_অঙ্ক >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_XFERINFOFUNCTION
#define NSCURLOPT_PROGRESS_তথ্য CURLOPT_XFERINFODATA
#define NSCURL_PROGRESS_T curl_off_t
#অন্যথা
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_PROGRESSFUNCTION
#define NSCURLOPT_PROGRESS_তথ্য CURLOPT_PROGRESSDATA
#define NSCURL_PROGRESS_T দ্বিগুণ
#endif

#যদি LIBCURL_VERSION_অঙ্ক >= 0x073800 /* 7.56.0 depricated curl_formadd */
#define NSCURL_POSTDATA_T curl_mime
#define NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#define NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#অন্যথা
#define NSCURL_POSTDATA_T কাঠামো curl_httppost
#define NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#define NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#endif

/** Information for a single fetch. */
কাঠামো curl_fetch_info {
	কাঠামো fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool sent_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_headers;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	কাঠামো curl_slist *headers;	/**< List of request headers. */
	অক্ষর *location;		/**< Response Location header, or 0. */
	unsigned long content_দৈর্ঘ্য;	/**< Response Content-Length, or 0. */
	অক্ষর *cookie_string;	/**< Cookie string for this fetch */
	অক্ষর *realm;		/**< HTTP Auth Realm */
	কাঠামো fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	long http_code; /**< HTTP result code from cURL. */

	uint64_t last_progress_update;	/**< Time of last progress update */
	পূর্ণসংখ্যা cert_depth; /**< deepest certificate in use */
	কাঠামো cert_info cert_তথ্য[MAX_CERT_DEPTH]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
কাঠামো cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	কাঠামো cache_handle *r_পূর্ব; /**< Previous cached handle in ring. */
	কাঠামো cache_handle *r_পরবর্তী; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
স্থির CURL *fetch_blank_curl;

/** Ring of cached handles */
স্থির কাঠামো cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
স্থির পূর্ণসংখ্যা curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
স্থির bool curl_with_openssl;

/** Error buffer for cURL. */
স্থির অক্ষর fetch_error_বাফার[CURL_ERROR_আকার];

/** Proxy authentication details. */
স্থির অক্ষর fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
স্থির bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
স্থির bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_তথ্য(scheme));
	curl_fetchers_registered++;
	ফেরত true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
স্থির শূন্য fetch_curl_finalise(lwc_string *scheme)
{
	কাঠামো cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_তথ্য(scheme));
	যদি (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		যদি (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = ফাঁকা;
	}

	/* Free anything remaining in the cached curl handle ring */
	যখন (curl_handle_ring != ফাঁকা) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
		curl_easy_cleanup(h->handle);
		মুক্ত(h);
	}
}


/**
 * Check if this fetcher can fetch a url.
 *
 * \param url The url to check.
 * \return true if the fetcher supports the url else false.
 */
স্থির bool fetch_curl_can_fetch(ধ্রুবক nsurl *url)
{
	ফেরত nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
স্থির কাঠামো fetch_postdata *
fetch_curl_alloc_postdata(ধ্রুবক অক্ষর *post_urlenc,
			  ধ্রুবক কাঠামো fetch_multipart_তথ্য *post_multipart)
{
	কাঠামো fetch_postdata *postdata;
	postdata = calloc(1, আকার(কাঠামো fetch_postdata));
	যদি (postdata != ফাঁকা) {

		যদি (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->তথ্য.urlenc = strdup(post_urlenc);
			যদি (postdata->তথ্য.urlenc == ফাঁকা) {
				মুক্ত(postdata);
				postdata = ফাঁকা;
			}
		} অন্যথা যদি (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->তথ্য.multipart = fetch_multipart_তথ্য_clone(post_multipart);
			যদি (postdata->তথ্য.multipart == ফাঁকা) {
				মুক্ত(postdata);
				postdata = ফাঁকা;
			}
		} অন্যথা {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	ফেরত postdata;
}

/**
 * free postdata
 */
স্থির শূন্য fetch_curl_free_postdata(কাঠামো fetch_postdata *postdata)
{
	যদি (postdata != ফাঁকা) {
		নির্বাচন (postdata->type) {
		ক্ষেত্র FETCH_POSTDATA_NONE:
			বিরতি;
		ক্ষেত্র FETCH_POSTDATA_URLENC:
			মুক্ত(postdata->তথ্য.urlenc);
			বিরতি;
		ক্ষেত্র FETCH_POSTDATA_MULTIPART:
			fetch_multipart_তথ্য_destroy(postdata->তথ্য.multipart);
			বিরতি;
		}

		মুক্ত(postdata);
	}
}

/**
 *construct a new fetch structure
 */
স্থির কাঠামো curl_fetch_info *fetch_alloc(শূন্য)
{
	কাঠামো curl_fetch_info *fetch;
	fetch = malloc(আকার (*fetch));
	যদি (fetch == ফাঁকা)
		ফেরত ফাঁকা;

	fetch->curl_handle = ফাঁকা;
	fetch->sent_ssl_chain = false;
	fetch->had_headers = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->headers = ফাঁকা;
	fetch->url = ফাঁকা;
	fetch->host = ফাঁকা;
	fetch->location = ফাঁকা;
	fetch->content_দৈর্ঘ্য = 0;
	fetch->http_code = 0;
	fetch->cookie_string = ফাঁকা;
	fetch->realm = ফাঁকা;
	fetch->last_progress_update = 0;
	fetch->postdata = ফাঁকা;
	fetch->curl_postdata = ফাঁকা;

	/* Clear certificate chain data */
	memset(fetch->cert_তথ্য, 0, আকার(fetch->cert_তথ্য));
	fetch->cert_depth = -1;

	ফেরত fetch;
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
স্থির শূন্য *
fetch_curl_setup(কাঠামো fetch *parent_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ধ্রুবক অক্ষর *post_urlenc,
		 ধ্রুবক কাঠামো fetch_multipart_তথ্য *post_multipart,
		 ধ্রুবক অক্ষর **headers)
{
	কাঠামো curl_fetch_info *fetch;
	কাঠামো curl_slist *slist;
	পূর্ণসংখ্যা i;

	fetch = fetch_alloc();
	যদি (fetch == ফাঁকা)
		ফেরত ফাঁকা;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = parent_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	যদি (fetch->host == ফাঁকা) {
		যাও failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	যদি (fetch->postdata == ফাঁকা) {
		যাও failed;
	}

#define APPEND(list, মান) \
	slist = curl_slist_append(list, মান);		\
	যদি (slist == ফাঁকা)				\
		যাও failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->headers, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->headers, "Expect:");

	যদি ((nsoption_charp(accept_language) != ফাঁকা) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		অক্ষর s[80];
		snprintf(s, আকার s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[আকার s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	যদি (nsoption_charp(accept_charset) != ফাঁকা &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		অক্ষর s[80];
		snprintf(s, আকার s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[আকার s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	যদি (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->headers, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	জন্য (i = 0; headers[i] != ফাঁকা; i++) {
		APPEND(fetch->headers, headers[i]);
	}

	ফেরত fetch;

#undef APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->headers);
	মুক্ত(fetch);
	ফেরত ফাঁকা;
}


#ifdef WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
স্থির কাঠামো cert_chain *
fetch_curl_get_cached_chain(কাঠামো curl_fetch_info *f)
{
	কাঠামো cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	যদি (chain == ফাঁকা) {
		chain = hashmap_ঢোকাও(curl_fetch_ssl_hashmap, f->url);
	}

	ফেরত chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
স্থির শূন্য
fetch_curl_store_certs_in_cache(কাঠামো curl_fetch_info *f)
{
	size_t depth;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_DEPTH];
	কাঠামো cert_chain chain, *cached_chain;
	কাঠামো cert_info *certs;

	memset(&chain, 0, আকার(chain));

	certs = f->cert_তথ্য;
	chain.depth = f->cert_depth + 1; /* 0 indexed certificate depth */

	জন্য (depth = 0; depth < chain.depth; depth++) {
		যদি (certs[depth].cert == ফাঁকা) {
			/* This certificate is missing, skip it */
			chain.certs[depth].err = SSL_CERT_ERR_CERT_MISSING;
			চালু;
		}

		/* error code (if any) */
		নির্বাচন (certs[depth].err) {
		ক্ষেত্র X509_V_OK:
			chain.certs[depth].err = SSL_CERT_ERR_OK;
			বিরতি;

		ক্ষেত্র X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			/* fallthrough */
		ক্ষেত্র X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_চাবি:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_ISSUER;
			বিরতি;

		ক্ষেত্র X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		ক্ষেত্র X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		ক্ষেত্র X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		ক্ষেত্র X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_SIG;
			বিরতি;

		ক্ষেত্র X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		ক্ষেত্র X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_YOUNG;
			বিরতি;

		ক্ষেত্র X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		ক্ষেত্র X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_OLD;
			বিরতি;

		ক্ষেত্র X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			chain.certs[depth].err = SSL_CERT_ERR_SELF_SIGNED;
			বিরতি;

		ক্ষেত্র X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			chain.certs[depth].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			বিরতি;

		ক্ষেত্র X509_V_ERR_CERT_REVOKED:
			chain.certs[depth].err = SSL_CERT_ERR_REVOKED;
			বিরতি;

		ক্ষেত্র X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[depth].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			বিরতি;

		পূর্বনির্ধারিত:
			chain.certs[depth].err = SSL_CERT_ERR_UNKNOWN;
			বিরতি;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[depth].cert);
		BIO_get_mem_ptr(mem, &buf[depth]);
		(শূন্য) BIO_set_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[depth].der = (uint8_t *)buf[depth]->তথ্য;
		chain.certs[depth].der_দৈর্ঘ্য = buf[depth]->দৈর্ঘ্য;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	যদি (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	জন্য (depth = 0; depth < chain.depth; depth++) {
		যদি (chain.certs[depth].err == SSL_CERT_ERR_CERT_MISSING) {
			চালু;
		}
		যদি (buf[depth] != ফাঁকা) {
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
স্থির পূর্ণসংখ্যা
fetch_curl_verify_callback(পূর্ণসংখ্যা verify_ok, X509_STORE_CTX *x509_ctx)
{
	পূর্ণসংখ্যা depth;
	কাঠামো curl_fetch_info *fetch;

	depth = X509_STORE_CTX_get_error_depth(x509_ctx);
	fetch = X509_STORE_CTX_get_app_তথ্য(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	যদি (depth >= MAX_CERT_DEPTH) {
		X509_STORE_CTX_set_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		ফেরত 0;
	}

	/* record the max depth */
	যদি (depth > fetch->cert_depth) {
		fetch->cert_depth = depth;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	যদি (!fetch->cert_তথ্য[depth].cert) {
		fetch->cert_তথ্য[depth].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_up_ref(fetch->cert_তথ্য[depth].cert);
		fetch->cert_তথ্য[depth].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	যদি (depth > 0) {
		verify_ok = 1;
	} অন্যথা {
		/* search for deeper certificates in the chain with errors */
		জন্য (depth = fetch->cert_depth; depth > 0; depth--) {
			যদি (fetch->cert_তথ্য[depth].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_set_error(x509_ctx, fetch->cert_তথ্য[depth].err);
			}
		}
	}

	ফেরত verify_ok;
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
স্থির পূর্ণসংখ্যা fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, শূন্য *parm)
{
	কাঠামো curl_fetch_info *f = (কাঠামো curl_fetch_info *) parm;
	পূর্ণসংখ্যা ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_set_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_তথ্য(f->host),
					 lwc_string_দৈর্ঘ্য(f->host));

	/* Store fetch struct in context for verify callback */
	যদি (ok) {
		ok = X509_STORE_CTX_set_app_তথ্য(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	যদি (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_in_cache(f);

	ফেরত ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
স্থির CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, শূন্য *_sslctx, শূন্য *parm)
{
	কাঠামো curl_fetch_info *f = (কাঠামো curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	long options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_set_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	যদি (f->downgrade_tls) {
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

	ফেরত CURLE_OK;
}


#endif /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
স্থির শূন্য
fetch_curl_report_certs_upstream(কাঠামো curl_fetch_info *f)
{
	fetch_বার্তা বার্তা;
	কাঠামো cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	যদি (chain != ফাঁকা) {
		বার্তা.type = FETCH_CERTS;
		বার্তা.তথ্য.chain = chain;

		fetch_send_callback(&বার্তা, f->fetch_handle);
	}

	f->sent_ssl_chain = true;
}

#যদি LIBCURL_VERSION_অঙ্ক >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
কাঠামো curl_mime_ctx {
	অক্ষর *বাফার;
	curl_off_t আকার_2;
	curl_off_t position;
};

স্থির size_t mime_তথ্য_read_callback(অক্ষর *বাফার, size_t আকার_2, size_t nitems, শূন্য *arg)
{
	কাঠামো curl_mime_ctx *mctx = (কাঠামো curl_mime_ctx *) arg;
	curl_off_t sz = mctx->আকার_2 - mctx->position;

	nitems *= আকার_2;
	যদি(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	যদি(sz) {
		memcpy(বাফার, mctx->বাফার + mctx->position, sz);
	}
	mctx->position += sz;
	ফেরত sz;
}

স্থির পূর্ণসংখ্যা mime_তথ্য_seek_callback(শূন্য *arg, curl_off_t offset, পূর্ণসংখ্যা origin)
{
	কাঠামো curl_mime_ctx *mctx = (কাঠামো curl_mime_ctx *) arg;

	নির্বাচন(origin) {
	ক্ষেত্র SEEK_END:
		offset += mctx->আকার_2;
		বিরতি;
	ক্ষেত্র SEEK_CUR:
		offset += mctx->position;
		বিরতি;
	}

	যদি(offset < 0) {
		ফেরত CURL_SEEKFUNC_FAIL;
	}
	mctx->position = offset;
	ফেরত CURL_SEEKFUNC_OK;
}

স্থির শূন্য mime_তথ্য_free_callback(শূন্য *arg)
{
	কাঠামো curl_mime_ctx *mctx = (কাঠামো curl_mime_ctx *) arg;
	মুক্ত(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
স্থির curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    ধ্রুবক কাঠামো fetch_multipart_তথ্য *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode code = CURLE_OK;
	size_t মান_len;

	cmime = curl_mime_init(chandle);
	যদি (cmime == ফাঁকা) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		ফেরত ফাঁকা;
	}

	/* iterate post data */
	জন্য (; multipart != ফাঁকা; multipart = multipart->পরবর্তী) {
		part = curl_mime_addpart(cmime);
		যদি (part == ফাঁকা) {
			যাও convert_failed;
		}

		code = curl_mime_name(part, multipart->name);
		যদি (code != CURLE_OK) {
			যাও convert_failed;
		}

		মান_len = strlen(multipart->মান);

		যদি (multipart->file && মান_len==0) {
			/* file entries with no filename require special handling */
			code=curl_mime_তথ্য(part, multipart->মান, মান_len);
			যদি (code != CURLE_OK) {
				যাও convert_failed;
			}

			code = curl_mime_filename(part, "");
			যদি (code != CURLE_OK) {
				যাও convert_failed;
			}

			code = curl_mime_type(part, "application/octet-stream");
			যদি (code != CURLE_OK) {
				যাও convert_failed;
			}

		} অন্যথা যদি(multipart->file) {
			/* file entry */
			nserror ret;
			অক্ষর *leafname = ফাঁকা;
			অক্ষর *mimetype = ফাঁকা;

			code = curl_mime_filedata(part, multipart->rawfile);
			যদি (code != CURLE_OK) {
				যাও convert_failed;
			}

			ret = guit->file->basename(multipart->মান, &leafname, ফাঁকা);
			যদি (ret != NSERROR_OK) {
				যাও convert_failed;
			}
			code = curl_mime_filename(part, leafname);
			মুক্ত(leafname);
			যদি (code != CURLE_OK) {
				যাও convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->মান);
			যদি (mimetype == ফাঁকা) {
				mimetype=strdup("text/plain");
			}
			যদি (mimetype == ফাঁকা) {
				যাও convert_failed;
			}
			code = curl_mime_type(part, mimetype);
			মুক্ত(mimetype);
			যদি (code != CURLE_OK) {
				যাও convert_failed;
			}

		} অন্যথা {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			কাঠামো curl_mime_ctx *cb_ctx;
			cb_ctx = malloc(আকার(কাঠামো curl_mime_ctx));
			যদি (cb_ctx == ফাঁকা) {
				যাও convert_failed;
			}
			cb_ctx->বাফার = multipart->মান;
			cb_ctx->আকার_2 = মান_len;
			cb_ctx->position = 0;
			code = curl_mime_তথ্য_cb(part,
						 মান_len,
						 mime_তথ্য_read_callback,
						 mime_তথ্য_seek_callback,
						 mime_তথ্য_free_callback,
						 cb_ctx);
			যদি (code != CURLE_OK) {
				মুক্ত(cb_ctx);
				যাও convert_failed;
			}
		}
	}

	ফেরত cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", code);
	curl_mime_free(cmime);
	ফেরত ফাঁকা;
}

#অন্যথা /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
স্থির কাঠামো curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    ধ্রুবক কাঠামো fetch_multipart_তথ্য *control)
{
	কাঠামো curl_httppost *post = ফাঁকা, *last = ফাঁকা;
	CURLFORMcode code;
	nserror ret;

	জন্য (; control; control = control->পরবর্তী) {
		যদি (control->file) {
			অক্ষর *leafname = ফাঁকা;
			ret = guit->file->basename(control->মান, &leafname, ফাঁকা);
			যদি (ret != NSERROR_OK) {
				চালু;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			যদি (control->মান[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				স্থির অক্ষর buf;

				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_বাফার, control->মান,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_END);
				যদি (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", code,
					      control->name);
			} অন্যথা {
				অক্ষর *mimetype = guit->fetch->mimetype(control->মান);
				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_END);
				যদি (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      code,
					      control->name,
					      control->মান);
				মুক্ত(mimetype);
			}
			মুক্ত(leafname);
		} অন্যথা {
			code = curl_formadd(&post, &last,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->মান,
					    CURLFORM_END);
			যদি (code != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", code,
				      control->name, control->মান);
		}
	}

	ফেরত post;
}

#endif  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
স্থির CURLcode fetch_curl_set_postdata(কাঠামো curl_fetch_info *f)
{
	CURLcode code = CURLE_OK;

#undef SETOPT
#define SETOPT(option, মান) { \
	code = curl_easy_setopt(f->curl_handle, option, মান);	\
	যদি (code != CURLE_OK)					\
		ফেরত code;					\
	}

	নির্বাচন (f->postdata->type) {
	ক্ষেত্র FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, ফাঁকা);
		SETOPT(NSCURL_POSTDATA_CURLOPT, ফাঁকা);
		SETOPT(CURLOPT_HTTPGET, 1L);
		বিরতি;

	ক্ষেত্র FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, ফাঁকা);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->তথ্য.urlenc);
		বিরতি;

	ক্ষেত্র FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, ফাঁকা);
		SETOPT(CURLOPT_HTTPGET, 0L);
		যদি (f->curl_postdata == ফাঁকা) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->তথ্য.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		বিরতি;
	}
	ফেরত code;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
স্থির CURLcode fetch_curl_set_options(কাঠামো curl_fetch_info *f)
{
	CURLcode code;
	ধ্রুবক অক্ষর *auth;

#undef SETOPT
#define SETOPT(option, মান) { \
	code = curl_easy_setopt(f->curl_handle, option, মান);	\
	যদি (code != CURLE_OK)					\
		ফেরত code;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_PRIVATE, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_PROGRESS_তথ্য, f);
	SETOPT(CURLOPT_HTTPHEADER, f->headers);
	code = fetch_curl_set_postdata(f);
	যদি (code != CURLE_OK) {
		ফেরত code;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	যদি (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} অন্যথা {
		SETOPT(CURLOPT_COOKIE, ফাঁকা);
	}

	যদি ((auth = urldb_get_auth_details(f->url, ফাঁকা)) != ফাঁকা) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} অন্যথা {
		SETOPT(CURLOPT_USERPWD, ফাঁকা);
	}

	/* set up proxy options */
	যদি (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != ফাঁকা) &&
	    (strncmp(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (long) nsoption_int(http_proxy_port));

#যদি LIBCURL_VERSION_অঙ্ক >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#endif

		যদি (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(long) CURLAUTH_BASIC :
					(long) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					আকার fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} অন্যথা {
		SETOPT(CURLOPT_PROXY, ফাঁকা);
	}


	যদি (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	যদি (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		যদি (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, ফাঁকা);
			SETOPT(CURLOPT_SSL_CTX_তথ্য, ফাঁকা);
		}
	} অন্যথা {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef WITH_OPENSSL
		যদি (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_তথ্য, f);
		}
#endif
	}

	ফেরত CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
স্থির bool
fetch_curl_initiate_fetch(কাঠামো curl_fetch_info *fetch, CURL *handle)
{
	CURLcode code;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	code = fetch_curl_set_options(fetch);
	যদি (code != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		ফেরত false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_add_handle(fetch_curl_multi, fetch->curl_handle);
	প্রতিজ্ঞা(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	ফেরত true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
স্থির CURL *fetch_curl_get_handle(lwc_string *host)
{
	কাঠামো cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	যদি (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		মুক্ত(h);
	} অন্যথা {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	ফেরত ret;
}


/**
 * Dispatch a single job
 */
স্থির bool fetch_curl_start(শূন্য *vfetch)
{
	কাঠামো curl_fetch_info *fetch = (কাঠামো curl_fetch_info*)vfetch;
	যদি (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		ফেরত false;
	}
	ফেরত fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
স্থির শূন্য fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#যদি LIBCURL_VERSION_অঙ্ক >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	ফেরত;
#অন্যথা
	কাঠামো cache_handle *h = 0;
	পূর্ণসংখ্যা c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	যদি (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		ফেরত;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(কাঠামো cache_handle, curl_handle_ring, c);
	যদি (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		যদি (curl_handle_ring != ফাঁকা) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_পরবর্তী;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} অন্যথা {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		ফেরত;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (কাঠামো cache_handle*)malloc(আকার(কাঠামো cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_ঢোকাও(curl_handle_ring, h);
#endif
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
স্থির শূন্য fetch_curl_stop(কাঠামো curl_fetch_info *f)
{
	CURLMcode codem;

	প্রতিজ্ঞা(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	যদি (f->curl_handle) {
		/* remove from curl multi handle */
		codem = curl_multi_remove_handle(fetch_curl_multi,
				f->curl_handle);
		প্রতিজ্ঞা(codem == CURLM_OK);
		/* Put this curl handle into the cache if wanted. */
		fetch_curl_cache_handle(f->curl_handle, f->host);
		f->curl_handle = 0;
	}

	fetch_remove_from_queues(f->fetch_handle);
}


/**
 * Abort a fetch.
 */
স্থির শূন্য fetch_curl_abort(শূন্য *vf)
{
	কাঠামো curl_fetch_info *f = (কাঠামো curl_fetch_info *)vf;
	প্রতিজ্ঞা(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	যদি (f->curl_handle) {
		যদি (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->abort = true;
		} অন্যথা {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} অন্যথা {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
স্থির শূন্য fetch_curl_free(শূন্য *vf)
{
	কাঠামো curl_fetch_info *f = (কাঠামো curl_fetch_info *)vf;
	পূর্ণসংখ্যা i;

	যদি (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	মুক্ত(f->location);
	মুক্ত(f->cookie_string);
	মুক্ত(f->realm);
	যদি (f->headers) {
		curl_slist_free_all(f->headers);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	জন্য (i = 0; i < MAX_CERT_DEPTH; i++) {
		যদি (f->cert_তথ্য[i].cert != ফাঁকা) {
			X509_free(f->cert_তথ্য[i].cert);
		}
	}

	মুক্ত(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
স্থির bool fetch_curl_process_headers(কাঠামো curl_fetch_info *f)
{
	long http_code;
	CURLcode code;
	fetch_বার্তা বার্তা;

	f->had_headers = true;

	যদি (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		প্রতিজ্ঞা(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}
	http_code = f->http_code;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_code);

	যদি ((http_code == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		বার্তা.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&বার্তা, f->fetch_handle);
		ফেরত true;
	}

	/* handle HTTP redirects (3xx response codes) */
	যদি (HTTP_RESPONSE_IS_3XX(http_code) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		বার্তা.type = FETCH_REDIRECT;
		বার্তা.তথ্য.redirect = f->location;
		fetch_send_callback(&বার্তা, f->fetch_handle);
		ফেরত true;
	}

	/* handle HTTP 401 (Authentication errors) */
	যদি (http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		বার্তা.type = FETCH_AUTH;
		বার্তা.তথ্য.auth.realm = f->realm;
		fetch_send_callback(&বার্তা, f->fetch_handle);
		ফেরত true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	যদি (f->only_2xx &&
	    strncmp(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_code) == false)) {
		বার্তা.type = FETCH_ERROR;
		বার্তা.তথ্য.error = messages_get("Not2xx");
		fetch_send_callback(&বার্তা, f->fetch_handle);
		ফেরত true;
	}

	যদি (f->abort)
		ফেরত true;

	ফেরত false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
স্থির শূন্য fetch_curl_done(CURL *curl_handle, CURLcode ফলাফল)
{
	bool finished = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	কাঠামো curl_fetch_info *f;
	অক্ষর **_hideous_hack = (অক্ষর **) (শূন্য *) &f;
	CURLcode code;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	code = curl_easy_getinfo(curl_handle, CURLINFO_PRIVATE, _hideous_hack);
	প্রতিজ্ঞা(code == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	যদি ((abort_fetch == false) &&
	    (ফলাফল == CURLE_OK ||
	     ((ফলাফল == CURLE_WRITE_ERROR) && (f->stopped == false)))) {
		/* fetch completed normally or the server fed us a junk gzip
		 * stream (usually in the form of garbage at the end of the
		 * stream). Curl will have fed us all but the last chunk of
		 * decoded data, which is sad as, if we'd received the last
		 * chunk, too, we'd be able to render the whole object.
		 * As is, we'll just have to accept that the end of the
		 * object will be truncated in this case and leave it to
		 * the content handlers to cope.
		 */
		যদি (f->stopped ||
		    (!f->had_headers &&	fetch_curl_process_headers(f))) {
			; /* redirect with no body or similar */
		} অন্যথা {
			finished = true;
		}
	} অন্যথা যদি (ফলাফল == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		যদি (!f->had_headers && fetch_curl_process_headers(f))
			; /* redirect with partial body, or similar */
		অন্যথা {
			finished = true;
		}
	} অন্যথা যদি (ফলাফল == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} অন্যথা যদি (ফলাফল == CURLE_SSL_PEER_CERTIFICATE ||
		   ফলাফল == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} অন্যথা {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", ফলাফল);
		error = true;
	}

	fetch_curl_stop(f);

	যদি (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	যদি (abort_fetch) {
		; /* fetch was aborted: no callback */
	} অন্যথা যদি (finished) {
		fetch_বার্তা বার্তা;
		বার্তা.type = FETCH_FINISHED;
		fetch_send_callback(&বার্তা, f->fetch_handle);
	} অন্যথা যদি (cert) {
		/* user needs to validate certificate with issue */
		fetch_বার্তা বার্তা;
		বার্তা.type = FETCH_CERT_ERR;
		fetch_send_callback(&বার্তা, f->fetch_handle);
	} অন্যথা যদি (error) {
		fetch_বার্তা বার্তা;
		নির্বাচন (ফলাফল) {
		ক্ষেত্র CURLE_SSL_CONNECT_ERROR:
			বার্তা.type = FETCH_SSL_ERR;
			বিরতি;

		ক্ষেত্র CURLE_OPERATION_TIMEDOUT:
			বার্তা.type = FETCH_TIMEDOUT;
			বার্তা.তথ্য.error = curl_easy_strerror(ফলাফল);
			বিরতি;

		পূর্বনির্ধারিত:
			বার্তা.type = FETCH_ERROR;
			বার্তা.তথ্য.error = curl_easy_strerror(ফলাফল);
		}

		fetch_send_callback(&বার্তা, f->fetch_handle);
	}

	fetch_free(f->fetch_handle);
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */
স্থির শূন্য fetch_curl_poll(lwc_string *scheme_ignored)
{
	পূর্ণসংখ্যা running, queue;
	CURLMcode codem;
	CURLMsg *curl_বার্তা;

	যদি (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_set, write_fd_set, exc_fd_set;
		পূর্ণসংখ্যা max_fd = -1;
		পূর্ণসংখ্যা i;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&exc_fd_set);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_set, &write_fd_set,
				&exc_fd_set, &max_fd);
		প্রতিজ্ঞা(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		জন্য (i = 0; i <= max_fd; i++) {
			bool পঠ = false;
			bool লিখ = false;
			bool error = false;

			যদি (FD_ISSET(i, &read_fd_set)) {
				পঠ = true;
			}
			যদি (FD_ISSET(i, &write_fd_set)) {
				লিখ = true;
			}
			যদি (FD_ISSET(i, &exc_fd_set)) {
				error = true;
			}
			যদি (পঠ || লিখ || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      পঠ ? "read" : "    ",
				      লিখ ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	করো {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		যদি (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			ফেরত;
		}
	} যখন (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_বার্তা = curl_multi_info_read(fetch_curl_multi, &queue);
	যখন (curl_বার্তা) {
		নির্বাচন (curl_বার্তা->বার্তা) {
			ক্ষেত্র CURLMSG_DONE:
				fetch_curl_done(curl_বার্তা->easy_handle,
						curl_বার্তা->তথ্য.ফলাফল);
				বিরতি;
			পূর্বনির্ধারিত:
				বিরতি;
		}
		curl_বার্তা = curl_multi_info_read(fetch_curl_multi, &queue);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
স্থির পূর্ণসংখ্যা
fetch_curl_progress(শূন্য *clientp,
		    NSCURL_PROGRESS_T dltotal,
		    NSCURL_PROGRESS_T dlnow,
		    NSCURL_PROGRESS_T ultotal,
		    NSCURL_PROGRESS_T ulnow)
{
	স্থির অক্ষর fetch_progress_বাফার[256]; /**< Progress buffer for cURL */
	কাঠামো curl_fetch_info *f = (কাঠামো curl_fetch_info *) clientp;
	uint64_t time_now_ms;
	fetch_বার্তা বার্তা;

	যদি (f->abort) {
		ফেরত 0;
        }

	বার্তা.type = FETCH_PROGRESS;
	বার্তা.তথ্য.progress = fetch_progress_বাফার;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&time_now_ms);
#define UPDATE_DELAY_MS (1000 / UPDATES_PER_SECOND)
	যদি (time_now_ms - f->last_progress_update < UPDATE_DELAY_MS) {
		ফেরত 0;
        }
#undef UPDATE_DELAY_MS
	f->last_progress_update = time_now_ms;

	যদি (dltotal > 0) {
		snprintf(fetch_progress_বাফার, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&বার্তা, f->fetch_handle);
	} অন্যথা {
		snprintf(fetch_progress_বাফার, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&বার্তা, f->fetch_handle);
	}

	ফেরত 0;
}


/**
 * Format curl debug for nslog
 */
স্থির পূর্ণসংখ্যা
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 অক্ষর *তথ্য,
		 size_t আকার_2,
		 শূন্য *userptr)
{
	স্থির ধ্রুবক অক্ষর s_infotype[CURLINFO_END][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	নির্বাচন(type) {
	ক্ষেত্র CURLINFO_TEXT:
	ক্ষেত্র CURLINFO_HEADER_OUT:
	ক্ষেত্র CURLINFO_HEADER_IN:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (পূর্ণসংখ্যা)আকার_2 - 1, তথ্য);
		বিরতি;

	পূর্বনির্ধারিত:
		বিরতি;
	}
	ফেরত 0;
}


স্থির curl_socket_t fetch_curl_socket_open(শূন্য *clientp,
		curlsocktype purpose, কাঠামো curl_sockaddr *address)
{
	(শূন্য) clientp;
	(শূন্য) purpose;

	ফেরত (curl_socket_t) guit->fetch->socket_open(
			address->family, address->socktype,
			address->protocol);
}

স্থির পূর্ণসংখ্যা fetch_curl_socket_close(শূন্য *clientp, curl_socket_t item)
{
	(শূন্য) clientp;

	ফেরত guit->fetch->socket_close((পূর্ণসংখ্যা) item);
}

/**
 * Callback function for cURL.
 */
স্থির size_t fetch_curl_তথ্য(অক্ষর *তথ্য, size_t আকার_2, size_t nmemb, শূন্য *_f)
{
	কাঠামো curl_fetch_info *f = _f;
	CURLcode code;
	fetch_বার্তা বার্তা;

	/* ensure we only have to get this information once */
	যদি (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		প্রতিজ্ঞা(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	যদি (f->http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_code = 0;
		ফেরত আকার_2 * nmemb;
	}

	যদি (f->abort || (!f->had_headers && fetch_curl_process_headers(f))) {
		f->stopped = true;
		ফেরত 0;
	}

	/* send data to the caller */
	বার্তা.type = FETCH_তথ্য;
	বার্তা.তথ্য.header_or_তথ্য.buf = (ধ্রুবক uint8_t *) তথ্য;
	বার্তা.তথ্য.header_or_তথ্য.len = আকার_2 * nmemb;
	fetch_send_callback(&বার্তা, f->fetch_handle);

	যদি (f->abort) {
		f->stopped = true;
		ফেরত 0;
	}

	ফেরত আকার_2 * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
স্থির size_t
fetch_curl_header(অক্ষর *তথ্য, size_t আকার_2, size_t nmemb, শূন্য *_f)
{
	কাঠামো curl_fetch_info *f = _f;
	পূর্ণসংখ্যা i;
	fetch_বার্তা বার্তা;
	আকার_2 *= nmemb;

	যদি (f->abort) {
		f->stopped = true;
		ফেরত 0;
	}

	যদি (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	বার্তা.type = FETCH_HEADER;
	বার্তা.তথ্য.header_or_তথ্য.buf = (ধ্রুবক uint8_t *) তথ্য;
	বার্তা.তথ্য.header_or_তথ্য.len = আকার_2;
	fetch_send_callback(&বার্তা, f->fetch_handle);

#define SKIP_ST(o) জন্য (i = (o); i < (পূর্ণসংখ্যা) আকার_2 && (তথ্য[i] == ' ' || তথ্য[i] == '\t'); i++)

	যদি (12 < আকার_2 && strncasecmp(তথ্য, "Location:", 9) == 0) {
		/* extract Location header */
		মুক্ত(f->location);
		f->location = malloc(আকার_2);
		যদি (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			ফেরত আকার_2;
		}
		SKIP_ST(9);
		strncpy(f->location, তথ্য + i, আকার_2 - i);
		f->location[আকার_2 - i] = '\0';
		জন্য (i = আকার_2 - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} অন্যথা যদি (15 < আকার_2 && strncasecmp(তথ্য, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		যদি (i < (পূর্ণসংখ্যা)আকার_2 && '0' <= তথ্য[i] && তথ্য[i] <= '9')
			f->content_দৈর্ঘ্য = atol(তথ্য + i);
	} অন্যথা যদি (17 < আকার_2 && strncasecmp(তথ্য, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		যখন (i < (পূর্ণসংখ্যা) আকার_2 - 5 &&
				strncasecmp(তথ্য + i, "realm", 5))
			i++;
		যখন (i < (পূর্ণসংখ্যা) আকার_2 - 1 && তথ্য[++i] != '"')
			/* */;
		i++;

		যদি (i < (পূর্ণসংখ্যা) আকার_2) {
			size_t end = i;

			যখন (end < আকার_2 && তথ্য[end] != '"')
				++end;

			যদি (end < আকার_2) {
				মুক্ত(f->realm);
				f->realm = malloc(end - i + 1);
				যদি (f->realm != ফাঁকা) {
					strncpy(f->realm, তথ্য + i, end - i);
					f->realm[end - i] = '\0';
				}
			}
		}
	} অন্যথা যদি (11 < আকার_2 && strncasecmp(তথ্য, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_set_cookie(f->fetch_handle, &তথ্য[i]);
	}

	ফেরত আকার_2;
#undef SKIP_ST
}

স্থির পূর্ণসংখ্যা fetch_curl_fdset(lwc_string *scheme, fd_set *read_set,
			    fd_set *write_set, fd_set *error_set)
{
	CURLMcode code;
	পূর্ণসংখ্যা maxfd = -1;

	code = curl_multi_fdset(fetch_curl_multi,
				read_set,
				write_set,
				error_set,
				&maxfd);
	প্রতিজ্ঞা(code == CURLM_OK);

	ফেরত maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_register(শূন্য)
{
	CURLcode code;
	curl_version_info_তথ্য *তথ্য;
	পূর্ণসংখ্যা i;
	lwc_string *scheme;
	ধ্রুবক কাঠামো fetcher_operation_table fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.setup = fetch_curl_setup,
		.start = fetch_curl_start,
		.abort = fetch_curl_abort,
		.মুক্ত = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#যদি LIBCURL_VERSION_অঙ্ক >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, ফাঁকা, ফাঁকা);
	যদি (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} অন্যথা {
		curl_with_openssl = false;
	}
#endif

	NSLOG(netsurf, INFO, "curl_version %s", curl_version());

	code = curl_global_init(CURL_GLOBAL_ALL);
	যদি (code != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		ফেরত NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	যদি (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		ফেরত NSERROR_INIT_FAILED;
	}

#যদি LIBCURL_VERSION_অঙ্ক >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		পূর্ণসংখ্যা maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#undef SETOPT
#define SETOPT(option, মান) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, মান);	\
	যদি (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		যাও curl_multi_setopt_failed;				\
	}

		SETOPT(CURLMOPT_MAXCONNECTS, (long)maxconnects);
		SETOPT(CURLMOPT_MAX_যোগ_CONNECTIONS, (long)maxconnects);
		SETOPT(CURLMOPT_MAX_HOST_CONNECTIONS, (long)nsoption_int(max_fetchers_per_host));
	}
#endif

	/* Create a curl easy handle with the options that are common to all
	 *  fetches.
	 */
	fetch_blank_curl = curl_easy_init();
	যদি (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		ফেরত NSERROR_INIT_FAILED;
	}

#undef SETOPT
#define SETOPT(option, মান) \
	code = curl_easy_setopt(fetch_blank_curl, option, মান);	\
	যদি (code != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		যাও curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_বাফার);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	যদি (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} অন্যথা {
		SETOPT(CURLOPT_VERBOSE, 1);
	}

	/* Currently we explode if curl uses HTTP2, so force 1.1. */
	SETOPT(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

	SETOPT(CURLOPT_WRITEFUNCTION, fetch_curl_তথ্য);
	SETOPT(CURLOPT_HEADERFUNCTION, fetch_curl_header);
	SETOPT(NSCURLOPT_PROGRESS_FUNCTION, fetch_curl_progress);
	SETOPT(CURLOPT_NOPROGRESS, 0L);
	SETOPT(CURLOPT_USERAGENT, user_agent_string());
	SETOPT(CURLOPT_ENCODING, "gzip");
	SETOPT(CURLOPT_নিচের_SPEED_LIMIT, 1L);
	SETOPT(CURLOPT_নিচের_SPEED_TIME, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (long)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	যদি (nsoption_charp(ca_bundle) &&
	    strcmp(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	যদি (nsoption_charp(ca_path) && strcmp(nsoption_charp(ca_path), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_path));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_path));
	}

#যদি LIBCURL_VERSION_অঙ্ক < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	code = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, ফাঁকা);
	যদি (code != CURLE_OK) {
		curl_with_openssl = false;
	} অন্যথা {
		curl_with_openssl = true;
	}
#endif

	যদি (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#যদি LIBCURL_VERSION_অঙ্ক >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		code = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		যদি (code != CURLE_OK && code != CURLE_NOT_BUILT_IN)
			যাও curl_easy_setopt_failed;
#endif
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	তথ্য = curl_version_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	যদি (curl_fetch_ssl_hashmap == ফাঁকা) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		ফেরত NSERROR_NOMEM;
	}

	জন্য (i = 0; তথ্য->protocols[i]; i++) {
		যদি (strcmp(তথ্য->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} অন্যথা যদি (strcmp(তথ্য->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} অন্যথা {
			/* Ignore non-http(s) protocols */
			চালু;
		}

		যদি (fetcher_add(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      তথ্য->protocols[i]);
		}
	}

	ফেরত NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	ফেরত NSERROR_INIT_FAILED;

#যদি LIBCURL_VERSION_অঙ্ক >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	ফেরত NSERROR_INIT_FAILED;
#endif
}
