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
#অন্তর্ভাৱ "utils/inet.h"

#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <inttypes.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <strings.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <sys/stat.h>

#অন্তর্ভাৱ <libwapcaplet/libwapcaplet.h>
#অন্তর্ভাৱ <nsutils/time.h>

#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "utils/hashmap.h"
#অন্তর্ভাৱ "utils/nsoption.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/ring.h"
#অন্তর্ভাৱ "utils/useragent.h"
#অন্তর্ভাৱ "utils/file.h"
#অন্তর্ভাৱ "utils/string.h"
#অন্তর্ভাৱ "netsurf/fetch.h"
#অন্তর্ভাৱ "netsurf/misc.h"
#অন্তর্ভাৱ "desktop/gui_internal.h"

#অন্তর্ভাৱ "content/fetch.h"
#অন্তর্ভাৱ "content/fetchers.h"
#অন্তর্ভাৱ "content/fetchers/curl.h"
#অন্তর্ভাৱ "content/urldb.h"

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

#অন্তর্ভাৱ <openssl/ssl.h>
#অন্তর্ভাৱ <openssl/x509v3.h>

#অন্যথা /* WITH_OPENSSL */

প্রকারনির্দেশ ৱর্ণ X509;

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

	যদি (port == রিক্ত)
		port = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_মান(hostname) ^ lwc_string_hash_মান(port);

	lwc_string_unref(hostname);
	lwc_string_unref(port);

	ঘূৰাই hash;
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

	যদি (port1 == রিক্ত)
		port1 = lwc_string_ref(corestring_lwc_443);
	যদি (port2 == রিক্ত)
		port2 = lwc_string_ref(corestring_lwc_443);

	যদি (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		গচ্ছ out;

	iseq = false;
	যদি (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		গচ্ছ out;

out:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	ঘূৰাই iseq;
}

স্থির শূন্য *
curl_fetch_ssl_মান_alloc(শূন্য *চাবি)
{
	সংরচনা cert_chain *out;

	যদি (cert_chain_alloc(0, &out) != NSERROR_OK) {
		ঘূৰাই রিক্ত;
	}

	ঘূৰাই out;
}

স্থির শূন্য
curl_fetch_ssl_মান_destroy(শূন্য *মান)
{
	সংরচনা cert_chain *chain = মান;
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

স্থির hashmap_t *curl_fetch_ssl_hashmap = রিক্ত;

/** SSL certificate info */
সংরচনা cert_info {
	X509 *cert;		/**< Pointer to certificate */
	long err;		/**< OpenSSL error code */
};

#যদি LIBCURL_VERSION_অঙ্ক >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_XFERINFOFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_XFERINFODATA
#define NSCURL_PROGRESS_T curl_off_t
#অন্যথা
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_PROGRESSFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_PROGRESSDATA
#define NSCURL_PROGRESS_T দ্ৱিগুণ
#endif

#যদি LIBCURL_VERSION_অঙ্ক >= 0x073800 /* 7.56.0 depricated curl_formadd */
#define NSCURL_POSTDATA_T curl_mime
#define NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#define NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#অন্যথা
#define NSCURL_POSTDATA_T সংরচনা curl_httppost
#define NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#define NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#endif

/** Information for a single fetch. */
সংরচনা curl_fetch_info {
	সংরচনা fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool sent_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_headers;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	সংরচনা curl_slist *headers;	/**< List of request headers. */
	ৱর্ণ *location;		/**< Response Location header, or 0. */
	unsigned long content_length;	/**< Response Content-Length, or 0. */
	ৱর্ণ *cookie_string;	/**< Cookie string for this fetch */
	ৱর্ণ *realm;		/**< HTTP Auth Realm */
	সংরচনা fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	long http_code; /**< HTTP result code from cURL. */

	uint64_t last_progress_update;	/**< Time of last progress update */
	পূর্ণাঙ্ক cert_depth; /**< deepest certificate in use */
	সংরচনা cert_info cert_data[MAX_CERT_DEPTH]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
সংরচনা cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	সংরচনা cache_handle *r_prev; /**< Previous cached handle in ring. */
	সংরচনা cache_handle *r_next; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
স্থির CURL *fetch_blank_curl;

/** Ring of cached handles */
স্থির সংরচনা cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
স্থির পূর্ণাঙ্ক curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
স্থির bool curl_with_openssl;

/** Error buffer for cURL. */
স্থির ৱর্ণ fetch_error_buffer[CURL_ERROR_SIZE];

/** Proxy authentication details. */
স্থির ৱর্ণ fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
স্থির bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
স্থির bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_data(scheme));
	curl_fetchers_registered++;
	ঘূৰাই true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
স্থির শূন্য fetch_curl_finalise(lwc_string *scheme)
{
	সংরচনা cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_data(scheme));
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
		curl_fetch_ssl_hashmap = রিক্ত;
	}

	/* Free anything remaining in the cached curl handle ring */
	যেতিয়ালৈ (curl_handle_ring != রিক্ত) {
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
স্থির bool fetch_curl_can_fetch(ধ্রুৱ nsurl *url)
{
	ঘূৰাই nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
স্থির সংরচনা fetch_postdata *
fetch_curl_alloc_postdata(ধ্রুৱ ৱর্ণ *post_urlenc,
			  ধ্রুৱ সংরচনা fetch_multipart_data *post_multipart)
{
	সংরচনা fetch_postdata *postdata;
	postdata = calloc(1, পরিমাণ(সংরচনা fetch_postdata));
	যদি (postdata != রিক্ত) {

		যদি (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->data.urlenc = strdup(post_urlenc);
			যদি (postdata->data.urlenc == রিক্ত) {
				মুক্ত(postdata);
				postdata = রিক্ত;
			}
		} অন্যথা যদি (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->data.multipart = fetch_multipart_data_clone(post_multipart);
			যদি (postdata->data.multipart == রিক্ত) {
				মুক্ত(postdata);
				postdata = রিক্ত;
			}
		} অন্যথা {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	ঘূৰাই postdata;
}

/**
 * free postdata
 */
স্থির শূন্য fetch_curl_free_postdata(সংরচনা fetch_postdata *postdata)
{
	যদি (postdata != রিক্ত) {
		ৱিকল্প (postdata->type) {
		পক্ষ FETCH_POSTDATA_NONE:
			ভঙ্গ;
		পক্ষ FETCH_POSTDATA_URLENC:
			মুক্ত(postdata->data.urlenc);
			ভঙ্গ;
		পক্ষ FETCH_POSTDATA_MULTIPART:
			fetch_multipart_data_destroy(postdata->data.multipart);
			ভঙ্গ;
		}

		মুক্ত(postdata);
	}
}

/**
 *construct a new fetch structure
 */
স্থির সংরচনা curl_fetch_info *fetch_alloc(শূন্য)
{
	সংরচনা curl_fetch_info *fetch;
	fetch = malloc(পরিমাণ (*fetch));
	যদি (fetch == রিক্ত)
		ঘূৰাই রিক্ত;

	fetch->curl_handle = রিক্ত;
	fetch->sent_ssl_chain = false;
	fetch->had_headers = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->headers = রিক্ত;
	fetch->url = রিক্ত;
	fetch->host = রিক্ত;
	fetch->location = রিক্ত;
	fetch->content_length = 0;
	fetch->http_code = 0;
	fetch->cookie_string = রিক্ত;
	fetch->realm = রিক্ত;
	fetch->last_progress_update = 0;
	fetch->postdata = রিক্ত;
	fetch->curl_postdata = রিক্ত;

	/* Clear certificate chain data */
	memset(fetch->cert_data, 0, পরিমাণ(fetch->cert_data));
	fetch->cert_depth = -1;

	ঘূৰাই fetch;
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
fetch_curl_setup(সংরচনা fetch *parent_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ধ্রুৱ ৱর্ণ *post_urlenc,
		 ধ্রুৱ সংরচনা fetch_multipart_data *post_multipart,
		 ধ্রুৱ ৱর্ণ **headers)
{
	সংরচনা curl_fetch_info *fetch;
	সংরচনা curl_slist *slist;
	পূর্ণাঙ্ক i;

	fetch = fetch_alloc();
	যদি (fetch == রিক্ত)
		ঘূৰাই রিক্ত;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = parent_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	যদি (fetch->host == রিক্ত) {
		গচ্ছ failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	যদি (fetch->postdata == রিক্ত) {
		গচ্ছ failed;
	}

#define APPEND(list, মান) \
	slist = curl_slist_append(list, মান);		\
	যদি (slist == রিক্ত)				\
		গচ্ছ failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->headers, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->headers, "Expect:");

	যদি ((nsoption_charp(accept_language) != রিক্ত) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		ৱর্ণ s[80];
		snprintf(s, পরিমাণ s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[পরিমাণ s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	যদি (nsoption_charp(accept_charset) != রিক্ত &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		ৱর্ণ s[80];
		snprintf(s, পরিমাণ s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[পরিমাণ s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	যদি (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->headers, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	বাবে (i = 0; headers[i] != রিক্ত; i++) {
		APPEND(fetch->headers, headers[i]);
	}

	ঘূৰাই fetch;

#undef APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->headers);
	মুক্ত(fetch);
	ঘূৰাই রিক্ত;
}


#ifdef WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
স্থির সংরচনা cert_chain *
fetch_curl_get_cached_chain(সংরচনা curl_fetch_info *f)
{
	সংরচনা cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	যদি (chain == রিক্ত) {
		chain = hashmap_ঢোকাও(curl_fetch_ssl_hashmap, f->url);
	}

	ঘূৰাই chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
স্থির শূন্য
fetch_curl_store_certs_in_cache(সংরচনা curl_fetch_info *f)
{
	size_t depth;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_DEPTH];
	সংরচনা cert_chain chain, *cached_chain;
	সংরচনা cert_info *certs;

	memset(&chain, 0, পরিমাণ(chain));

	certs = f->cert_data;
	chain.depth = f->cert_depth + 1; /* 0 indexed certificate depth */

	বাবে (depth = 0; depth < chain.depth; depth++) {
		যদি (certs[depth].cert == রিক্ত) {
			/* This certificate is missing, skip it */
			chain.certs[depth].err = SSL_CERT_ERR_CERT_MISSING;
			অনুৱর্ত;
		}

		/* error code (if any) */
		ৱিকল্প (certs[depth].err) {
		পক্ষ X509_V_OK:
			chain.certs[depth].err = SSL_CERT_ERR_OK;
			ভঙ্গ;

		পক্ষ X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			/* fallthrough */
		পক্ষ X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_চাবি:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_ISSUER;
			ভঙ্গ;

		পক্ষ X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		পক্ষ X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		পক্ষ X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		পক্ষ X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_SIG;
			ভঙ্গ;

		পক্ষ X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		পক্ষ X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_YOUNG;
			ভঙ্গ;

		পক্ষ X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		পক্ষ X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_OLD;
			ভঙ্গ;

		পক্ষ X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			chain.certs[depth].err = SSL_CERT_ERR_SELF_SIGNED;
			ভঙ্গ;

		পক্ষ X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			chain.certs[depth].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			ভঙ্গ;

		পক্ষ X509_V_ERR_CERT_REVOKED:
			chain.certs[depth].err = SSL_CERT_ERR_REVOKED;
			ভঙ্গ;

		পক্ষ X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[depth].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			ভঙ্গ;

		স্ৱভাৱ:
			chain.certs[depth].err = SSL_CERT_ERR_UNKNOWN;
			ভঙ্গ;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[depth].cert);
		BIO_get_mem_ptr(mem, &buf[depth]);
		(শূন্য) BIO_set_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[depth].der = (uint8_t *)buf[depth]->data;
		chain.certs[depth].der_length = buf[depth]->length;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	যদি (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	বাবে (depth = 0; depth < chain.depth; depth++) {
		যদি (chain.certs[depth].err == SSL_CERT_ERR_CERT_MISSING) {
			অনুৱর্ত;
		}
		যদি (buf[depth] != রিক্ত) {
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
স্থির পূর্ণাঙ্ক
fetch_curl_verify_callback(পূর্ণাঙ্ক verify_ok, X509_STORE_CTX *x509_ctx)
{
	পূর্ণাঙ্ক depth;
	সংরচনা curl_fetch_info *fetch;

	depth = X509_STORE_CTX_get_error_depth(x509_ctx);
	fetch = X509_STORE_CTX_get_app_data(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	যদি (depth >= MAX_CERT_DEPTH) {
		X509_STORE_CTX_set_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		ঘূৰাই 0;
	}

	/* record the max depth */
	যদি (depth > fetch->cert_depth) {
		fetch->cert_depth = depth;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	যদি (!fetch->cert_data[depth].cert) {
		fetch->cert_data[depth].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_up_ref(fetch->cert_data[depth].cert);
		fetch->cert_data[depth].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	যদি (depth > 0) {
		verify_ok = 1;
	} অন্যথা {
		/* search for deeper certificates in the chain with errors */
		বাবে (depth = fetch->cert_depth; depth > 0; depth--) {
			যদি (fetch->cert_data[depth].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_set_error(x509_ctx, fetch->cert_data[depth].err);
			}
		}
	}

	ঘূৰাই verify_ok;
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
স্থির পূর্ণাঙ্ক fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, শূন্য *parm)
{
	সংরচনা curl_fetch_info *f = (সংরচনা curl_fetch_info *) parm;
	পূর্ণাঙ্ক ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_set_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_data(f->host),
					 lwc_string_length(f->host));

	/* Store fetch struct in context for verify callback */
	যদি (ok) {
		ok = X509_STORE_CTX_set_app_data(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	যদি (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_in_cache(f);

	ঘূৰাই ok;
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
	সংরচনা curl_fetch_info *f = (সংরচনা curl_fetch_info *) parm;
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

	ঘূৰাই CURLE_OK;
}


#endif /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
স্থির শূন্য
fetch_curl_report_certs_upstream(সংরচনা curl_fetch_info *f)
{
	fetch_বাৰ্তা বাৰ্তা;
	সংরচনা cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	যদি (chain != রিক্ত) {
		বাৰ্তা.type = FETCH_CERTS;
		বাৰ্তা.data.chain = chain;

		fetch_send_callback(&বাৰ্তা, f->fetch_handle);
	}

	f->sent_ssl_chain = true;
}

#যদি LIBCURL_VERSION_অঙ্ক >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
সংরচনা curl_mime_ctx {
	ৱর্ণ *buffer;
	curl_off_t size;
	curl_off_t position;
};

স্থির size_t mime_data_read_callback(ৱর্ণ *buffer, size_t size, size_t nitems, শূন্য *arg)
{
	সংরচনা curl_mime_ctx *mctx = (সংরচনা curl_mime_ctx *) arg;
	curl_off_t sz = mctx->size - mctx->position;

	nitems *= size;
	যদি(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	যদি(sz) {
		memcpy(buffer, mctx->buffer + mctx->position, sz);
	}
	mctx->position += sz;
	ঘূৰাই sz;
}

স্থির পূর্ণাঙ্ক mime_data_seek_callback(শূন্য *arg, curl_off_t offset, পূর্ণাঙ্ক origin)
{
	সংরচনা curl_mime_ctx *mctx = (সংরচনা curl_mime_ctx *) arg;

	ৱিকল্প(origin) {
	পক্ষ SEEK_END:
		offset += mctx->size;
		ভঙ্গ;
	পক্ষ SEEK_CUR:
		offset += mctx->position;
		ভঙ্গ;
	}

	যদি(offset < 0) {
		ঘূৰাই CURL_SEEKFUNC_FAIL;
	}
	mctx->position = offset;
	ঘূৰাই CURL_SEEKFUNC_OK;
}

স্থির শূন্য mime_data_free_callback(শূন্য *arg)
{
	সংরচনা curl_mime_ctx *mctx = (সংরচনা curl_mime_ctx *) arg;
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
			    ধ্রুৱ সংরচনা fetch_multipart_data *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode code = CURLE_OK;
	size_t মান_len;

	cmime = curl_mime_init(chandle);
	যদি (cmime == রিক্ত) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		ঘূৰাই রিক্ত;
	}

	/* iterate post data */
	বাবে (; multipart != রিক্ত; multipart = multipart->next) {
		part = curl_mime_addpart(cmime);
		যদি (part == রিক্ত) {
			গচ্ছ convert_failed;
		}

		code = curl_mime_name(part, multipart->name);
		যদি (code != CURLE_OK) {
			গচ্ছ convert_failed;
		}

		মান_len = strlen(multipart->মান);

		যদি (multipart->file && মান_len==0) {
			/* file entries with no filename require special handling */
			code=curl_mime_data(part, multipart->মান, মান_len);
			যদি (code != CURLE_OK) {
				গচ্ছ convert_failed;
			}

			code = curl_mime_filename(part, "");
			যদি (code != CURLE_OK) {
				গচ্ছ convert_failed;
			}

			code = curl_mime_type(part, "application/octet-stream");
			যদি (code != CURLE_OK) {
				গচ্ছ convert_failed;
			}

		} অন্যথা যদি(multipart->file) {
			/* file entry */
			nserror ret;
			ৱর্ণ *leafname = রিক্ত;
			ৱর্ণ *mimetype = রিক্ত;

			code = curl_mime_filedata(part, multipart->rawfile);
			যদি (code != CURLE_OK) {
				গচ্ছ convert_failed;
			}

			ret = guit->file->basename(multipart->মান, &leafname, রিক্ত);
			যদি (ret != NSERROR_OK) {
				গচ্ছ convert_failed;
			}
			code = curl_mime_filename(part, leafname);
			মুক্ত(leafname);
			যদি (code != CURLE_OK) {
				গচ্ছ convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->মান);
			যদি (mimetype == রিক্ত) {
				mimetype=strdup("text/plain");
			}
			যদি (mimetype == রিক্ত) {
				গচ্ছ convert_failed;
			}
			code = curl_mime_type(part, mimetype);
			মুক্ত(mimetype);
			যদি (code != CURLE_OK) {
				গচ্ছ convert_failed;
			}

		} অন্যথা {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			সংরচনা curl_mime_ctx *cb_ctx;
			cb_ctx = malloc(পরিমাণ(সংরচনা curl_mime_ctx));
			যদি (cb_ctx == রিক্ত) {
				গচ্ছ convert_failed;
			}
			cb_ctx->buffer = multipart->মান;
			cb_ctx->size = মান_len;
			cb_ctx->position = 0;
			code = curl_mime_data_cb(part,
						 মান_len,
						 mime_data_read_callback,
						 mime_data_seek_callback,
						 mime_data_free_callback,
						 cb_ctx);
			যদি (code != CURLE_OK) {
				মুক্ত(cb_ctx);
				গচ্ছ convert_failed;
			}
		}
	}

	ঘূৰাই cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", code);
	curl_mime_free(cmime);
	ঘূৰাই রিক্ত;
}

#অন্যথা /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
স্থির সংরচনা curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    ধ্রুৱ সংরচনা fetch_multipart_data *control)
{
	সংরচনা curl_httppost *post = রিক্ত, *last = রিক্ত;
	CURLFORMcode code;
	nserror ret;

	বাবে (; control; control = control->next) {
		যদি (control->file) {
			ৱর্ণ *leafname = রিক্ত;
			ret = guit->file->basename(control->মান, &leafname, রিক্ত);
			যদি (ret != NSERROR_OK) {
				অনুৱর্ত;
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
				স্থির ৱর্ণ buf;

				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_BUFFER, control->মান,
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
				ৱর্ণ *mimetype = guit->fetch->mimetype(control->মান);
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

	ঘূৰাই post;
}

#endif  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
স্থির CURLcode fetch_curl_set_postdata(সংরচনা curl_fetch_info *f)
{
	CURLcode code = CURLE_OK;

#undef SETOPT
#define SETOPT(option, মান) { \
	code = curl_easy_setopt(f->curl_handle, option, মান);	\
	যদি (code != CURLE_OK)					\
		ঘূৰাই code;					\
	}

	ৱিকল্প (f->postdata->type) {
	পক্ষ FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, রিক্ত);
		SETOPT(NSCURL_POSTDATA_CURLOPT, রিক্ত);
		SETOPT(CURLOPT_HTTPGET, 1L);
		ভঙ্গ;

	পক্ষ FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, রিক্ত);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->data.urlenc);
		ভঙ্গ;

	পক্ষ FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, রিক্ত);
		SETOPT(CURLOPT_HTTPGET, 0L);
		যদি (f->curl_postdata == রিক্ত) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->data.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		ভঙ্গ;
	}
	ঘূৰাই code;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
স্থির CURLcode fetch_curl_set_options(সংরচনা curl_fetch_info *f)
{
	CURLcode code;
	ধ্রুৱ ৱর্ণ *auth;

#undef SETOPT
#define SETOPT(option, মান) { \
	code = curl_easy_setopt(f->curl_handle, option, মান);	\
	যদি (code != CURLE_OK)					\
		ঘূৰাই code;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_PRIVATE, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_PROGRESS_DATA, f);
	SETOPT(CURLOPT_HTTPHEADER, f->headers);
	code = fetch_curl_set_postdata(f);
	যদি (code != CURLE_OK) {
		ঘূৰাই code;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	যদি (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} অন্যথা {
		SETOPT(CURLOPT_COOKIE, রিক্ত);
	}

	যদি ((auth = urldb_get_auth_details(f->url, রিক্ত)) != রিক্ত) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} অন্যথা {
		SETOPT(CURLOPT_USERPWD, রিক্ত);
	}

	/* set up proxy options */
	যদি (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != রিক্ত) &&
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
					পরিমাণ fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} অন্যথা {
		SETOPT(CURLOPT_PROXY, রিক্ত);
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
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, রিক্ত);
			SETOPT(CURLOPT_SSL_CTX_DATA, রিক্ত);
		}
	} অন্যথা {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef WITH_OPENSSL
		যদি (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_DATA, f);
		}
#endif
	}

	ঘূৰাই CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
স্থির bool
fetch_curl_initiate_fetch(সংরচনা curl_fetch_info *fetch, CURL *handle)
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
		ঘূৰাই false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_add_handle(fetch_curl_multi, fetch->curl_handle);
	প্রতিজ্ঞা(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	ঘূৰাই true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
স্থির CURL *fetch_curl_get_handle(lwc_string *host)
{
	সংরচনা cache_handle *h;
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
	ঘূৰাই ret;
}


/**
 * Dispatch a single job
 */
স্থির bool fetch_curl_start(শূন্য *vfetch)
{
	সংরচনা curl_fetch_info *fetch = (সংরচনা curl_fetch_info*)vfetch;
	যদি (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		ঘূৰাই false;
	}
	ঘূৰাই fetch_curl_initiate_fetch(fetch,
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
	ঘূৰাই;
#অন্যথা
	সংরচনা cache_handle *h = 0;
	পূর্ণাঙ্ক c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	যদি (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		ঘূৰাই;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(সংরচনা cache_handle, curl_handle_ring, c);
	যদি (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		যদি (curl_handle_ring != রিক্ত) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_next;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} অন্যথা {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		ঘূৰাই;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (সংরচনা cache_handle*)malloc(পরিমাণ(সংরচনা cache_handle));
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
স্থির শূন্য fetch_curl_stop(সংরচনা curl_fetch_info *f)
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
	সংরচনা curl_fetch_info *f = (সংরচনা curl_fetch_info *)vf;
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
	সংরচনা curl_fetch_info *f = (সংরচনা curl_fetch_info *)vf;
	পূর্ণাঙ্ক i;

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
	বাবে (i = 0; i < MAX_CERT_DEPTH; i++) {
		যদি (f->cert_data[i].cert != রিক্ত) {
			X509_free(f->cert_data[i].cert);
		}
	}

	মুক্ত(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
স্থির bool fetch_curl_process_headers(সংরচনা curl_fetch_info *f)
{
	long http_code;
	CURLcode code;
	fetch_বাৰ্তা বাৰ্তা;

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
		বাৰ্তা.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&বাৰ্তা, f->fetch_handle);
		ঘূৰাই true;
	}

	/* handle HTTP redirects (3xx response codes) */
	যদি (HTTP_RESPONSE_IS_3XX(http_code) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		বাৰ্তা.type = FETCH_REDIRECT;
		বাৰ্তা.data.redirect = f->location;
		fetch_send_callback(&বাৰ্তা, f->fetch_handle);
		ঘূৰাই true;
	}

	/* handle HTTP 401 (Authentication errors) */
	যদি (http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		বাৰ্তা.type = FETCH_AUTH;
		বাৰ্তা.data.auth.realm = f->realm;
		fetch_send_callback(&বাৰ্তা, f->fetch_handle);
		ঘূৰাই true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	যদি (f->only_2xx &&
	    strncmp(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_code) == false)) {
		বাৰ্তা.type = FETCH_ERROR;
		বাৰ্তা.data.error = messages_get("Not2xx");
		fetch_send_callback(&বাৰ্তা, f->fetch_handle);
		ঘূৰাই true;
	}

	যদি (f->abort)
		ঘূৰাই true;

	ঘূৰাই false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
স্থির শূন্য fetch_curl_done(CURL *curl_handle, CURLcode result)
{
	bool finished = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	সংরচনা curl_fetch_info *f;
	ৱর্ণ **_hideous_hack = (ৱর্ণ **) (শূন্য *) &f;
	CURLcode code;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	code = curl_easy_getinfo(curl_handle, CURLINFO_PRIVATE, _hideous_hack);
	প্রতিজ্ঞা(code == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	যদি ((abort_fetch == false) &&
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
		যদি (f->stopped ||
		    (!f->had_headers &&	fetch_curl_process_headers(f))) {
			; /* redirect with no body or similar */
		} অন্যথা {
			finished = true;
		}
	} অন্যথা যদি (result == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		যদি (!f->had_headers && fetch_curl_process_headers(f))
			; /* redirect with partial body, or similar */
		অন্যথা {
			finished = true;
		}
	} অন্যথা যদি (result == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} অন্যথা যদি (result == CURLE_SSL_PEER_CERTIFICATE ||
		   result == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} অন্যথা {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", result);
		error = true;
	}

	fetch_curl_stop(f);

	যদি (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	যদি (abort_fetch) {
		; /* fetch was aborted: no callback */
	} অন্যথা যদি (finished) {
		fetch_বাৰ্তা বাৰ্তা;
		বাৰ্তা.type = FETCH_FINISHED;
		fetch_send_callback(&বাৰ্তা, f->fetch_handle);
	} অন্যথা যদি (cert) {
		/* user needs to validate certificate with issue */
		fetch_বাৰ্তা বাৰ্তা;
		বাৰ্তা.type = FETCH_CERT_ERR;
		fetch_send_callback(&বাৰ্তা, f->fetch_handle);
	} অন্যথা যদি (error) {
		fetch_বাৰ্তা বাৰ্তা;
		ৱিকল্প (result) {
		পক্ষ CURLE_SSL_CONNECT_ERROR:
			বাৰ্তা.type = FETCH_SSL_ERR;
			ভঙ্গ;

		পক্ষ CURLE_OPERATION_TIMEDOUT:
			বাৰ্তা.type = FETCH_TIMEDOUT;
			বাৰ্তা.data.error = curl_easy_strerror(result);
			ভঙ্গ;

		স্ৱভাৱ:
			বাৰ্তা.type = FETCH_ERROR;
			বাৰ্তা.data.error = curl_easy_strerror(result);
		}

		fetch_send_callback(&বাৰ্তা, f->fetch_handle);
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
	পূর্ণাঙ্ক running, queue;
	CURLMcode codem;
	CURLMsg *curl_বাৰ্তা;

	যদি (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_set, write_fd_set, exc_fd_set;
		পূর্ণাঙ্ক max_fd = -1;
		পূর্ণাঙ্ক i;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&exc_fd_set);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_set, &write_fd_set,
				&exc_fd_set, &max_fd);
		প্রতিজ্ঞা(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		বাবে (i = 0; i <= max_fd; i++) {
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
	কৰা {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		যদি (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			ঘূৰাই;
		}
	} যেতিয়ালৈ (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_বাৰ্তা = curl_multi_info_read(fetch_curl_multi, &queue);
	যেতিয়ালৈ (curl_বাৰ্তা) {
		ৱিকল্প (curl_বাৰ্তা->বাৰ্তা) {
			পক্ষ CURLMSG_DONE:
				fetch_curl_done(curl_বাৰ্তা->easy_handle,
						curl_বাৰ্তা->data.result);
				ভঙ্গ;
			স্ৱভাৱ:
				ভঙ্গ;
		}
		curl_বাৰ্তা = curl_multi_info_read(fetch_curl_multi, &queue);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
স্থির পূর্ণাঙ্ক
fetch_curl_progress(শূন্য *clientp,
		    NSCURL_PROGRESS_T dltotal,
		    NSCURL_PROGRESS_T dlnow,
		    NSCURL_PROGRESS_T ultotal,
		    NSCURL_PROGRESS_T ulnow)
{
	স্থির ৱর্ণ fetch_progress_buffer[256]; /**< Progress buffer for cURL */
	সংরচনা curl_fetch_info *f = (সংরচনা curl_fetch_info *) clientp;
	uint64_t time_now_ms;
	fetch_বাৰ্তা বাৰ্তা;

	যদি (f->abort) {
		ঘূৰাই 0;
        }

	বাৰ্তা.type = FETCH_PROGRESS;
	বাৰ্তা.data.progress = fetch_progress_buffer;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&time_now_ms);
#define UPDATE_DELAY_MS (1000 / UPDATES_PER_SECOND)
	যদি (time_now_ms - f->last_progress_update < UPDATE_DELAY_MS) {
		ঘূৰাই 0;
        }
#undef UPDATE_DELAY_MS
	f->last_progress_update = time_now_ms;

	যদি (dltotal > 0) {
		snprintf(fetch_progress_buffer, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&বাৰ্তা, f->fetch_handle);
	} অন্যথা {
		snprintf(fetch_progress_buffer, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&বাৰ্তা, f->fetch_handle);
	}

	ঘূৰাই 0;
}


/**
 * Format curl debug for nslog
 */
স্থির পূর্ণাঙ্ক
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 ৱর্ণ *data,
		 size_t size,
		 শূন্য *userptr)
{
	স্থির ধ্রুৱ ৱর্ণ s_infotype[CURLINFO_END][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	ৱিকল্প(type) {
	পক্ষ CURLINFO_TEXT:
	পক্ষ CURLINFO_HEADER_OUT:
	পক্ষ CURLINFO_HEADER_IN:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (পূর্ণাঙ্ক)size - 1, data);
		ভঙ্গ;

	স্ৱভাৱ:
		ভঙ্গ;
	}
	ঘূৰাই 0;
}


স্থির curl_socket_t fetch_curl_socket_open(শূন্য *clientp,
		curlsocktype purpose, সংরচনা curl_sockaddr *address)
{
	(শূন্য) clientp;
	(শূন্য) purpose;

	ঘূৰাই (curl_socket_t) guit->fetch->socket_open(
			address->family, address->socktype,
			address->protocol);
}

স্থির পূর্ণাঙ্ক fetch_curl_socket_close(শূন্য *clientp, curl_socket_t item)
{
	(শূন্য) clientp;

	ঘূৰাই guit->fetch->socket_close((পূর্ণাঙ্ক) item);
}

/**
 * Callback function for cURL.
 */
স্থির size_t fetch_curl_data(ৱর্ণ *data, size_t size, size_t nmemb, শূন্য *_f)
{
	সংরচনা curl_fetch_info *f = _f;
	CURLcode code;
	fetch_বাৰ্তা বাৰ্তা;

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
		ঘূৰাই size * nmemb;
	}

	যদি (f->abort || (!f->had_headers && fetch_curl_process_headers(f))) {
		f->stopped = true;
		ঘূৰাই 0;
	}

	/* send data to the caller */
	বাৰ্তা.type = FETCH_DATA;
	বাৰ্তা.data.header_or_data.buf = (ধ্রুৱ uint8_t *) data;
	বাৰ্তা.data.header_or_data.len = size * nmemb;
	fetch_send_callback(&বাৰ্তা, f->fetch_handle);

	যদি (f->abort) {
		f->stopped = true;
		ঘূৰাই 0;
	}

	ঘূৰাই size * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
স্থির size_t
fetch_curl_header(ৱর্ণ *data, size_t size, size_t nmemb, শূন্য *_f)
{
	সংরচনা curl_fetch_info *f = _f;
	পূর্ণাঙ্ক i;
	fetch_বাৰ্তা বাৰ্তা;
	size *= nmemb;

	যদি (f->abort) {
		f->stopped = true;
		ঘূৰাই 0;
	}

	যদি (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	বাৰ্তা.type = FETCH_HEADER;
	বাৰ্তা.data.header_or_data.buf = (ধ্রুৱ uint8_t *) data;
	বাৰ্তা.data.header_or_data.len = size;
	fetch_send_callback(&বাৰ্তা, f->fetch_handle);

#define SKIP_ST(o) বাবে (i = (o); i < (পূর্ণাঙ্ক) size && (data[i] == ' ' || data[i] == '\t'); i++)

	যদি (12 < size && strncasecmp(data, "Location:", 9) == 0) {
		/* extract Location header */
		মুক্ত(f->location);
		f->location = malloc(size);
		যদি (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			ঘূৰাই size;
		}
		SKIP_ST(9);
		strncpy(f->location, data + i, size - i);
		f->location[size - i] = '\0';
		বাবে (i = size - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} অন্যথা যদি (15 < size && strncasecmp(data, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		যদি (i < (পূর্ণাঙ্ক)size && '0' <= data[i] && data[i] <= '9')
			f->content_length = atol(data + i);
	} অন্যথা যদি (17 < size && strncasecmp(data, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		যেতিয়ালৈ (i < (পূর্ণাঙ্ক) size - 5 &&
				strncasecmp(data + i, "realm", 5))
			i++;
		যেতিয়ালৈ (i < (পূর্ণাঙ্ক) size - 1 && data[++i] != '"')
			/* */;
		i++;

		যদি (i < (পূর্ণাঙ্ক) size) {
			size_t end = i;

			যেতিয়ালৈ (end < size && data[end] != '"')
				++end;

			যদি (end < size) {
				মুক্ত(f->realm);
				f->realm = malloc(end - i + 1);
				যদি (f->realm != রিক্ত) {
					strncpy(f->realm, data + i, end - i);
					f->realm[end - i] = '\0';
				}
			}
		}
	} অন্যথা যদি (11 < size && strncasecmp(data, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_set_cookie(f->fetch_handle, &data[i]);
	}

	ঘূৰাই size;
#undef SKIP_ST
}

স্থির পূর্ণাঙ্ক fetch_curl_fdset(lwc_string *scheme, fd_set *read_set,
			    fd_set *write_set, fd_set *error_set)
{
	CURLMcode code;
	পূর্ণাঙ্ক maxfd = -1;

	code = curl_multi_fdset(fetch_curl_multi,
				read_set,
				write_set,
				error_set,
				&maxfd);
	প্রতিজ্ঞা(code == CURLM_OK);

	ঘূৰাই maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_register(শূন্য)
{
	CURLcode code;
	curl_version_info_data *data;
	পূর্ণাঙ্ক i;
	lwc_string *scheme;
	ধ্রুৱ সংরচনা fetcher_operation_table fetcher_ops = {
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

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, রিক্ত, রিক্ত);
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
		ঘূৰাই NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	যদি (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		ঘূৰাই NSERROR_INIT_FAILED;
	}

#যদি LIBCURL_VERSION_অঙ্ক >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		পূর্ণাঙ্ক maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#undef SETOPT
#define SETOPT(option, মান) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, মান);	\
	যদি (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		গচ্ছ curl_multi_setopt_failed;				\
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
	যদি (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		ঘূৰাই NSERROR_INIT_FAILED;
	}

#undef SETOPT
#define SETOPT(option, মান) \
	code = curl_easy_setopt(fetch_blank_curl, option, মান);	\
	যদি (code != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		গচ্ছ curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_buffer);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	যদি (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} অন্যথা {
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
	SETOPT(CURLOPT_নিচেৰ_SPEED_LIMIT, 1L);
	SETOPT(CURLOPT_নিচেৰ_SPEED_TIME, 180L);
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
	code = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, রিক্ত);
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
			গচ্ছ curl_easy_setopt_failed;
#endif
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	data = curl_version_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	যদি (curl_fetch_ssl_hashmap == রিক্ত) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		ঘূৰাই NSERROR_NOMEM;
	}

	বাবে (i = 0; data->protocols[i]; i++) {
		যদি (strcmp(data->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} অন্যথা যদি (strcmp(data->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} অন্যথা {
			/* Ignore non-http(s) protocols */
			অনুৱর্ত;
		}

		যদি (fetcher_add(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      data->protocols[i]);
		}
	}

	ঘূৰাই NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	ঘূৰাই NSERROR_INIT_FAILED;

#যদি LIBCURL_VERSION_অঙ্ক >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	ঘূৰাই NSERROR_INIT_FAILED;
#endif
}
