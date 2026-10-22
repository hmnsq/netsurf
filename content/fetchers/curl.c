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
#ضمن "utils/inet.h"

#ضمن <assert.h>
#ضمن <errno.h>
#ضمن <inttypes.h>
#ضمن <stdbool.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <time.h>
#ضمن <sys/stat.h>

#ضمن <libwapcaplet/libwapcaplet.h>
#ضمن <nsutils/time.h>

#ضمن "utils/corestrings.h"
#ضمن "utils/hashmap.h"
#ضمن "utils/nsoption.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/utils.h"
#ضمن "utils/ring.h"
#ضمن "utils/useragent.h"
#ضمن "utils/file.h"
#ضمن "utils/string.h"
#ضمن "netsurf/fetch.h"
#ضمن "netsurf/misc.h"
#ضمن "desktop/gui_internal.h"

#ضمن "content/fetch.h"
#ضمن "content/fetchers.h"
#ضمن "content/fetchers/curl.h"
#ضمن "content/urldb.h"

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

#ضمن <openssl/ssl.h>
#ضمن <openssl/x509v3.h>

#وإلا /* WITH_OPENSSL */

تعريفنوع حرف X509;

ساكن فراغ X509_free(X509 *cert)
{
	حرر(cert);
}

#endif /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
ساكن uint32_t
curl_fetch_ssl_کلید_hash(فراغ *کلید)
{
	nsurl *url = کلید;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *port = nsurl_get_component(url, NSURL_PORT);
	uint32_t hash;

	إذا (port == عدم)
		port = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_قدر(hostname) ^ lwc_string_hash_قدر(port);

	lwc_string_unref(hostname);
	lwc_string_unref(port);

	أرجع hash;
}

/* We only compare the hostname and port */
ساكن bool
curl_fetch_ssl_کلید_eq(فراغ *key1, فراغ *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_PORT);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_PORT);
	bool iseq = false;

	إذا (port1 == عدم)
		port1 = lwc_string_ref(corestring_lwc_443);
	إذا (port2 == عدم)
		port2 = lwc_string_ref(corestring_lwc_443);

	إذا (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		اذهب out;

	iseq = false;
	إذا (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		اذهب out;

out:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	أرجع iseq;
}

ساكن فراغ *
curl_fetch_ssl_قدر_alloc(فراغ *کلید)
{
	بنية cert_chain *out;

	إذا (cert_chain_alloc(0, &out) != NSERROR_OK) {
		أرجع عدم;
	}

	أرجع out;
}

ساكن فراغ
curl_fetch_ssl_قدر_destroy(فراغ *قدر)
{
	بنية cert_chain *chain = قدر;
	إذا (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

ساكن hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.کلید_clone = (hashmap_کلید_clone_t)nsurl_ref,
	.کلید_destroy = (hashmap_کلید_destroy_t)nsurl_unref,
	.کلید_eq = curl_fetch_ssl_کلید_eq,
	.کلید_hash = curl_fetch_ssl_کلید_hash,
	.قدر_alloc = curl_fetch_ssl_قدر_alloc,
	.قدر_destroy = curl_fetch_ssl_قدر_destroy,
};

ساكن hashmap_t *curl_fetch_ssl_hashmap = عدم;

/** SSL certificate info */
بنية cert_info {
	X509 *cert;		/**< Pointer to certificate */
	long err;		/**< OpenSSL error code */
};

#إذا LIBCURL_VERSION_عدد >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_XFERINFOFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_XFERINFODATA
#define NSCURL_PROGRESS_T curl_off_t
#وإلا
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_PROGRESSFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_PROGRESSDATA
#define NSCURL_PROGRESS_T مزدوج
#endif

#إذا LIBCURL_VERSION_عدد >= 0x073800 /* 7.56.0 depricated curl_formadd */
#define NSCURL_POSTDATA_T curl_mime
#define NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#define NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#وإلا
#define NSCURL_POSTDATA_T بنية curl_httppost
#define NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#define NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#endif

/** Information for a single fetch. */
بنية curl_fetch_info {
	بنية fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool sent_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_headers;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	بنية curl_slist *headers;	/**< List of request headers. */
	حرف *location;		/**< Response Location header, or 0. */
	unsigned long content_length;	/**< Response Content-Length, or 0. */
	حرف *cookie_string;	/**< Cookie string for this fetch */
	حرف *realm;		/**< HTTP Auth Realm */
	بنية fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	long http_code; /**< HTTP result code from cURL. */

	uint64_t last_progress_update;	/**< Time of last progress update */
	صحيح cert_depth; /**< deepest certificate in use */
	بنية cert_info cert_data[MAX_CERT_DEPTH]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
بنية cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	بنية cache_handle *r_prev; /**< Previous cached handle in ring. */
	بنية cache_handle *r_next; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
ساكن CURL *fetch_blank_curl;

/** Ring of cached handles */
ساكن بنية cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
ساكن صحيح curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
ساكن bool curl_with_openssl;

/** Error buffer for cURL. */
ساكن حرف fetch_error_buffer[CURL_ERROR_SIZE];

/** Proxy authentication details. */
ساكن حرف fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
ساكن bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
ساكن bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_data(scheme));
	curl_fetchers_registered++;
	أرجع true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
ساكن فراغ fetch_curl_finalise(lwc_string *scheme)
{
	بنية cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_data(scheme));
	إذا (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		إذا (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = عدم;
	}

	/* Free anything remaining in the cached curl handle ring */
	طالما (curl_handle_ring != عدم) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
		curl_easy_cleanup(h->handle);
		حرر(h);
	}
}


/**
 * Check if this fetcher can fetch a url.
 *
 * \param url The url to check.
 * \return true if the fetcher supports the url else false.
 */
ساكن bool fetch_curl_can_fetch(ثابت nsurl *url)
{
	أرجع nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
ساكن بنية fetch_postdata *
fetch_curl_alloc_postdata(ثابت حرف *post_urlenc,
			  ثابت بنية fetch_multipart_data *post_multipart)
{
	بنية fetch_postdata *postdata;
	postdata = calloc(1, حجم(بنية fetch_postdata));
	إذا (postdata != عدم) {

		إذا (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->data.urlenc = strdup(post_urlenc);
			إذا (postdata->data.urlenc == عدم) {
				حرر(postdata);
				postdata = عدم;
			}
		} وإلا إذا (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->data.multipart = fetch_multipart_data_clone(post_multipart);
			إذا (postdata->data.multipart == عدم) {
				حرر(postdata);
				postdata = عدم;
			}
		} وإلا {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	أرجع postdata;
}

/**
 * free postdata
 */
ساكن فراغ fetch_curl_free_postdata(بنية fetch_postdata *postdata)
{
	إذا (postdata != عدم) {
		بدل (postdata->type) {
		حالة FETCH_POSTDATA_NONE:
			اكسر;
		حالة FETCH_POSTDATA_URLENC:
			حرر(postdata->data.urlenc);
			اكسر;
		حالة FETCH_POSTDATA_MULTIPART:
			fetch_multipart_data_destroy(postdata->data.multipart);
			اكسر;
		}

		حرر(postdata);
	}
}

/**
 *construct a new fetch structure
 */
ساكن بنية curl_fetch_info *fetch_alloc(فراغ)
{
	بنية curl_fetch_info *fetch;
	fetch = malloc(حجم (*fetch));
	إذا (fetch == عدم)
		أرجع عدم;

	fetch->curl_handle = عدم;
	fetch->sent_ssl_chain = false;
	fetch->had_headers = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->headers = عدم;
	fetch->url = عدم;
	fetch->host = عدم;
	fetch->location = عدم;
	fetch->content_length = 0;
	fetch->http_code = 0;
	fetch->cookie_string = عدم;
	fetch->realm = عدم;
	fetch->last_progress_update = 0;
	fetch->postdata = عدم;
	fetch->curl_postdata = عدم;

	/* Clear certificate chain data */
	memset(fetch->cert_data, 0, حجم(fetch->cert_data));
	fetch->cert_depth = -1;

	أرجع fetch;
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
ساكن فراغ *
fetch_curl_setup(بنية fetch *parent_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_data *post_multipart,
		 ثابت حرف **headers)
{
	بنية curl_fetch_info *fetch;
	بنية curl_slist *slist;
	صحيح i;

	fetch = fetch_alloc();
	إذا (fetch == عدم)
		أرجع عدم;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = parent_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	إذا (fetch->host == عدم) {
		اذهب failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	إذا (fetch->postdata == عدم) {
		اذهب failed;
	}

#define APPEND(list, قدر) \
	slist = curl_slist_append(list, قدر);		\
	إذا (slist == عدم)				\
		اذهب failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->headers, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->headers, "Expect:");

	إذا ((nsoption_charp(accept_language) != عدم) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		حرف s[80];
		snprintf(s, حجم s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[حجم s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	إذا (nsoption_charp(accept_charset) != عدم &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		حرف s[80];
		snprintf(s, حجم s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[حجم s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	إذا (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->headers, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	لكل (i = 0; headers[i] != عدم; i++) {
		APPEND(fetch->headers, headers[i]);
	}

	أرجع fetch;

#undef APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->headers);
	حرر(fetch);
	أرجع عدم;
}


#ifdef WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
ساكن بنية cert_chain *
fetch_curl_get_cached_chain(بنية curl_fetch_info *f)
{
	بنية cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	إذا (chain == عدم) {
		chain = hashmap_داخل_کرو(curl_fetch_ssl_hashmap, f->url);
	}

	أرجع chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
ساكن فراغ
fetch_curl_store_certs_in_cache(بنية curl_fetch_info *f)
{
	size_t depth;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_DEPTH];
	بنية cert_chain chain, *cached_chain;
	بنية cert_info *certs;

	memset(&chain, 0, حجم(chain));

	certs = f->cert_data;
	chain.depth = f->cert_depth + 1; /* 0 indexed certificate depth */

	لكل (depth = 0; depth < chain.depth; depth++) {
		إذا (certs[depth].cert == عدم) {
			/* This certificate is missing, skip it */
			chain.certs[depth].err = SSL_CERT_ERR_CERT_MISSING;
			استمر;
		}

		/* error code (if any) */
		بدل (certs[depth].err) {
		حالة X509_V_OK:
			chain.certs[depth].err = SSL_CERT_ERR_OK;
			اكسر;

		حالة X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			/* fallthrough */
		حالة X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_کلید:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_ISSUER;
			اكسر;

		حالة X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		حالة X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		حالة X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		حالة X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_SIG;
			اكسر;

		حالة X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		حالة X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_YOUNG;
			اكسر;

		حالة X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		حالة X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_OLD;
			اكسر;

		حالة X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			chain.certs[depth].err = SSL_CERT_ERR_SELF_SIGNED;
			اكسر;

		حالة X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			chain.certs[depth].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			اكسر;

		حالة X509_V_ERR_CERT_REVOKED:
			chain.certs[depth].err = SSL_CERT_ERR_REVOKED;
			اكسر;

		حالة X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[depth].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			اكسر;

		افتراضي:
			chain.certs[depth].err = SSL_CERT_ERR_UNKNOWN;
			اكسر;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[depth].cert);
		BIO_get_mem_ptr(mem, &buf[depth]);
		(فراغ) BIO_set_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[depth].der = (uint8_t *)buf[depth]->data;
		chain.certs[depth].der_length = buf[depth]->length;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	إذا (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	لكل (depth = 0; depth < chain.depth; depth++) {
		إذا (chain.certs[depth].err == SSL_CERT_ERR_CERT_MISSING) {
			استمر;
		}
		إذا (buf[depth] != عدم) {
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
ساكن صحيح
fetch_curl_verify_callback(صحيح verify_ok, X509_STORE_CTX *x509_ctx)
{
	صحيح depth;
	بنية curl_fetch_info *fetch;

	depth = X509_STORE_CTX_get_error_depth(x509_ctx);
	fetch = X509_STORE_CTX_get_app_data(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	إذا (depth >= MAX_CERT_DEPTH) {
		X509_STORE_CTX_set_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		أرجع 0;
	}

	/* record the max depth */
	إذا (depth > fetch->cert_depth) {
		fetch->cert_depth = depth;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	إذا (!fetch->cert_data[depth].cert) {
		fetch->cert_data[depth].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_up_ref(fetch->cert_data[depth].cert);
		fetch->cert_data[depth].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	إذا (depth > 0) {
		verify_ok = 1;
	} وإلا {
		/* search for deeper certificates in the chain with errors */
		لكل (depth = fetch->cert_depth; depth > 0; depth--) {
			إذا (fetch->cert_data[depth].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_set_error(x509_ctx, fetch->cert_data[depth].err);
			}
		}
	}

	أرجع verify_ok;
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
ساكن صحيح fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, فراغ *parm)
{
	بنية curl_fetch_info *f = (بنية curl_fetch_info *) parm;
	صحيح ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_set_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_data(f->host),
					 lwc_string_length(f->host));

	/* Store fetch struct in context for verify callback */
	إذا (ok) {
		ok = X509_STORE_CTX_set_app_data(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	إذا (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_in_cache(f);

	أرجع ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
ساكن CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, فراغ *_sslctx, فراغ *parm)
{
	بنية curl_fetch_info *f = (بنية curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	long options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_set_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	إذا (f->downgrade_tls) {
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

	أرجع CURLE_OK;
}


#endif /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
ساكن فراغ
fetch_curl_report_certs_upstream(بنية curl_fetch_info *f)
{
	fetch_پیغام پیغام;
	بنية cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	إذا (chain != عدم) {
		پیغام.type = FETCH_CERTS;
		پیغام.data.chain = chain;

		fetch_send_callback(&پیغام, f->fetch_handle);
	}

	f->sent_ssl_chain = true;
}

#إذا LIBCURL_VERSION_عدد >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
بنية curl_mime_ctx {
	حرف *buffer;
	curl_off_t size;
	curl_off_t position;
};

ساكن size_t mime_data_read_callback(حرف *buffer, size_t size, size_t nitems, فراغ *arg)
{
	بنية curl_mime_ctx *mctx = (بنية curl_mime_ctx *) arg;
	curl_off_t sz = mctx->size - mctx->position;

	nitems *= size;
	إذا(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	إذا(sz) {
		memcpy(buffer, mctx->buffer + mctx->position, sz);
	}
	mctx->position += sz;
	أرجع sz;
}

ساكن صحيح mime_data_seek_callback(فراغ *arg, curl_off_t offset, صحيح origin)
{
	بنية curl_mime_ctx *mctx = (بنية curl_mime_ctx *) arg;

	بدل(origin) {
	حالة SEEK_END:
		offset += mctx->size;
		اكسر;
	حالة SEEK_CUR:
		offset += mctx->position;
		اكسر;
	}

	إذا(offset < 0) {
		أرجع CURL_SEEKFUNC_FAIL;
	}
	mctx->position = offset;
	أرجع CURL_SEEKFUNC_OK;
}

ساكن فراغ mime_data_free_callback(فراغ *arg)
{
	بنية curl_mime_ctx *mctx = (بنية curl_mime_ctx *) arg;
	حرر(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
ساكن curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    ثابت بنية fetch_multipart_data *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode code = CURLE_OK;
	size_t قدر_len;

	cmime = curl_mime_init(chandle);
	إذا (cmime == عدم) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		أرجع عدم;
	}

	/* iterate post data */
	لكل (; multipart != عدم; multipart = multipart->next) {
		part = curl_mime_addpart(cmime);
		إذا (part == عدم) {
			اذهب convert_failed;
		}

		code = curl_mime_name(part, multipart->name);
		إذا (code != CURLE_OK) {
			اذهب convert_failed;
		}

		قدر_len = strlen(multipart->قدر);

		إذا (multipart->file && قدر_len==0) {
			/* file entries with no filename require special handling */
			code=curl_mime_data(part, multipart->قدر, قدر_len);
			إذا (code != CURLE_OK) {
				اذهب convert_failed;
			}

			code = curl_mime_filename(part, "");
			إذا (code != CURLE_OK) {
				اذهب convert_failed;
			}

			code = curl_mime_type(part, "application/octet-stream");
			إذا (code != CURLE_OK) {
				اذهب convert_failed;
			}

		} وإلا إذا(multipart->file) {
			/* file entry */
			nserror ret;
			حرف *leafname = عدم;
			حرف *mimetype = عدم;

			code = curl_mime_filedata(part, multipart->rawfile);
			إذا (code != CURLE_OK) {
				اذهب convert_failed;
			}

			ret = guit->file->basename(multipart->قدر, &leafname, عدم);
			إذا (ret != NSERROR_OK) {
				اذهب convert_failed;
			}
			code = curl_mime_filename(part, leafname);
			حرر(leafname);
			إذا (code != CURLE_OK) {
				اذهب convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->قدر);
			إذا (mimetype == عدم) {
				mimetype=strdup("text/plain");
			}
			إذا (mimetype == عدم) {
				اذهب convert_failed;
			}
			code = curl_mime_type(part, mimetype);
			حرر(mimetype);
			إذا (code != CURLE_OK) {
				اذهب convert_failed;
			}

		} وإلا {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			بنية curl_mime_ctx *cb_ctx;
			cb_ctx = malloc(حجم(بنية curl_mime_ctx));
			إذا (cb_ctx == عدم) {
				اذهب convert_failed;
			}
			cb_ctx->buffer = multipart->قدر;
			cb_ctx->size = قدر_len;
			cb_ctx->position = 0;
			code = curl_mime_data_cb(part,
						 قدر_len,
						 mime_data_read_callback,
						 mime_data_seek_callback,
						 mime_data_free_callback,
						 cb_ctx);
			إذا (code != CURLE_OK) {
				حرر(cb_ctx);
				اذهب convert_failed;
			}
		}
	}

	أرجع cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", code);
	curl_mime_free(cmime);
	أرجع عدم;
}

#وإلا /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
ساكن بنية curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    ثابت بنية fetch_multipart_data *control)
{
	بنية curl_httppost *post = عدم, *last = عدم;
	CURLFORMcode code;
	nserror ret;

	لكل (; control; control = control->next) {
		إذا (control->file) {
			حرف *leafname = عدم;
			ret = guit->file->basename(control->قدر, &leafname, عدم);
			إذا (ret != NSERROR_OK) {
				استمر;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			إذا (control->قدر[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				ساكن حرف buf;

				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_BUFFER, control->قدر,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_END);
				إذا (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", code,
					      control->name);
			} وإلا {
				حرف *mimetype = guit->fetch->mimetype(control->قدر);
				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_END);
				إذا (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      code,
					      control->name,
					      control->قدر);
				حرر(mimetype);
			}
			حرر(leafname);
		} وإلا {
			code = curl_formadd(&post, &last,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->قدر,
					    CURLFORM_END);
			إذا (code != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", code,
				      control->name, control->قدر);
		}
	}

	أرجع post;
}

#endif  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
ساكن CURLcode fetch_curl_set_postdata(بنية curl_fetch_info *f)
{
	CURLcode code = CURLE_OK;

#undef SETOPT
#define SETOPT(option, قدر) { \
	code = curl_easy_setopt(f->curl_handle, option, قدر);	\
	إذا (code != CURLE_OK)					\
		أرجع code;					\
	}

	بدل (f->postdata->type) {
	حالة FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, عدم);
		SETOPT(NSCURL_POSTDATA_CURLOPT, عدم);
		SETOPT(CURLOPT_HTTPGET, 1L);
		اكسر;

	حالة FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, عدم);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->data.urlenc);
		اكسر;

	حالة FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, عدم);
		SETOPT(CURLOPT_HTTPGET, 0L);
		إذا (f->curl_postdata == عدم) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->data.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		اكسر;
	}
	أرجع code;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
ساكن CURLcode fetch_curl_set_options(بنية curl_fetch_info *f)
{
	CURLcode code;
	ثابت حرف *auth;

#undef SETOPT
#define SETOPT(option, قدر) { \
	code = curl_easy_setopt(f->curl_handle, option, قدر);	\
	إذا (code != CURLE_OK)					\
		أرجع code;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_PRIVATE, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_PROGRESS_DATA, f);
	SETOPT(CURLOPT_HTTPHEADER, f->headers);
	code = fetch_curl_set_postdata(f);
	إذا (code != CURLE_OK) {
		أرجع code;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	إذا (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} وإلا {
		SETOPT(CURLOPT_COOKIE, عدم);
	}

	إذا ((auth = urldb_get_auth_details(f->url, عدم)) != عدم) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} وإلا {
		SETOPT(CURLOPT_USERPWD, عدم);
	}

	/* set up proxy options */
	إذا (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != عدم) &&
	    (strncmp(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (long) nsoption_int(http_proxy_port));

#إذا LIBCURL_VERSION_عدد >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#endif

		إذا (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(long) CURLAUTH_BASIC :
					(long) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					حجم fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} وإلا {
		SETOPT(CURLOPT_PROXY, عدم);
	}


	إذا (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	إذا (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		إذا (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, عدم);
			SETOPT(CURLOPT_SSL_CTX_DATA, عدم);
		}
	} وإلا {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef WITH_OPENSSL
		إذا (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_DATA, f);
		}
#endif
	}

	أرجع CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
ساكن bool
fetch_curl_initiate_fetch(بنية curl_fetch_info *fetch, CURL *handle)
{
	CURLcode code;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	code = fetch_curl_set_options(fetch);
	إذا (code != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		أرجع false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_add_handle(fetch_curl_multi, fetch->curl_handle);
	أكد(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	أرجع true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
ساكن CURL *fetch_curl_get_handle(lwc_string *host)
{
	بنية cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	إذا (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		حرر(h);
	} وإلا {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	أرجع ret;
}


/**
 * Dispatch a single job
 */
ساكن bool fetch_curl_start(فراغ *vfetch)
{
	بنية curl_fetch_info *fetch = (بنية curl_fetch_info*)vfetch;
	إذا (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		أرجع false;
	}
	أرجع fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
ساكن فراغ fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#إذا LIBCURL_VERSION_عدد >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	أرجع;
#وإلا
	بنية cache_handle *h = 0;
	صحيح c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	إذا (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		أرجع;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(بنية cache_handle, curl_handle_ring, c);
	إذا (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		إذا (curl_handle_ring != عدم) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_next;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} وإلا {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		أرجع;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (بنية cache_handle*)malloc(حجم(بنية cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_داخل_کرو(curl_handle_ring, h);
#endif
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
ساكن فراغ fetch_curl_stop(بنية curl_fetch_info *f)
{
	CURLMcode codem;

	أكد(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	إذا (f->curl_handle) {
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
ساكن فراغ fetch_curl_abort(فراغ *vf)
{
	بنية curl_fetch_info *f = (بنية curl_fetch_info *)vf;
	أكد(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	إذا (f->curl_handle) {
		إذا (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->abort = true;
		} وإلا {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} وإلا {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
ساكن فراغ fetch_curl_free(فراغ *vf)
{
	بنية curl_fetch_info *f = (بنية curl_fetch_info *)vf;
	صحيح i;

	إذا (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	حرر(f->location);
	حرر(f->cookie_string);
	حرر(f->realm);
	إذا (f->headers) {
		curl_slist_free_all(f->headers);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	لكل (i = 0; i < MAX_CERT_DEPTH; i++) {
		إذا (f->cert_data[i].cert != عدم) {
			X509_free(f->cert_data[i].cert);
		}
	}

	حرر(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
ساكن bool fetch_curl_process_headers(بنية curl_fetch_info *f)
{
	long http_code;
	CURLcode code;
	fetch_پیغام پیغام;

	f->had_headers = true;

	إذا (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		أكد(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}
	http_code = f->http_code;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_code);

	إذا ((http_code == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		پیغام.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&پیغام, f->fetch_handle);
		أرجع true;
	}

	/* handle HTTP redirects (3xx response codes) */
	إذا (HTTP_RESPONSE_IS_3XX(http_code) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		پیغام.type = FETCH_REDIRECT;
		پیغام.data.redirect = f->location;
		fetch_send_callback(&پیغام, f->fetch_handle);
		أرجع true;
	}

	/* handle HTTP 401 (Authentication errors) */
	إذا (http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		پیغام.type = FETCH_AUTH;
		پیغام.data.auth.realm = f->realm;
		fetch_send_callback(&پیغام, f->fetch_handle);
		أرجع true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	إذا (f->only_2xx &&
	    strncmp(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_code) == false)) {
		پیغام.type = FETCH_ERROR;
		پیغام.data.error = messages_get("Not2xx");
		fetch_send_callback(&پیغام, f->fetch_handle);
		أرجع true;
	}

	إذا (f->abort)
		أرجع true;

	أرجع false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
ساكن فراغ fetch_curl_done(CURL *curl_handle, CURLcode result)
{
	bool finished = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	بنية curl_fetch_info *f;
	حرف **_hideous_hack = (حرف **) (فراغ *) &f;
	CURLcode code;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	code = curl_easy_getinfo(curl_handle, CURLINFO_PRIVATE, _hideous_hack);
	أكد(code == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	إذا ((abort_fetch == false) &&
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
		إذا (f->stopped ||
		    (!f->had_headers &&	fetch_curl_process_headers(f))) {
			; /* redirect with no body or similar */
		} وإلا {
			finished = true;
		}
	} وإلا إذا (result == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		إذا (!f->had_headers && fetch_curl_process_headers(f))
			; /* redirect with partial body, or similar */
		وإلا {
			finished = true;
		}
	} وإلا إذا (result == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} وإلا إذا (result == CURLE_SSL_PEER_CERTIFICATE ||
		   result == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} وإلا {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", result);
		error = true;
	}

	fetch_curl_stop(f);

	إذا (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	إذا (abort_fetch) {
		; /* fetch was aborted: no callback */
	} وإلا إذا (finished) {
		fetch_پیغام پیغام;
		پیغام.type = FETCH_FINISHED;
		fetch_send_callback(&پیغام, f->fetch_handle);
	} وإلا إذا (cert) {
		/* user needs to validate certificate with issue */
		fetch_پیغام پیغام;
		پیغام.type = FETCH_CERT_ERR;
		fetch_send_callback(&پیغام, f->fetch_handle);
	} وإلا إذا (error) {
		fetch_پیغام پیغام;
		بدل (result) {
		حالة CURLE_SSL_CONNECT_ERROR:
			پیغام.type = FETCH_SSL_ERR;
			اكسر;

		حالة CURLE_OPERATION_TIMEDOUT:
			پیغام.type = FETCH_TIMEDOUT;
			پیغام.data.error = curl_easy_strerror(result);
			اكسر;

		افتراضي:
			پیغام.type = FETCH_ERROR;
			پیغام.data.error = curl_easy_strerror(result);
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
ساكن فراغ fetch_curl_poll(lwc_string *scheme_ignored)
{
	صحيح running, queue;
	CURLMcode codem;
	CURLMsg *curl_پیغام;

	إذا (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_set, write_fd_set, exc_fd_set;
		صحيح max_fd = -1;
		صحيح i;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&exc_fd_set);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_set, &write_fd_set,
				&exc_fd_set, &max_fd);
		أكد(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		لكل (i = 0; i <= max_fd; i++) {
			bool اقرأ = false;
			bool اكتب = false;
			bool error = false;

			إذا (FD_ISSET(i, &read_fd_set)) {
				اقرأ = true;
			}
			إذا (FD_ISSET(i, &write_fd_set)) {
				اكتب = true;
			}
			إذا (FD_ISSET(i, &exc_fd_set)) {
				error = true;
			}
			إذا (اقرأ || اكتب || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      اقرأ ? "read" : "    ",
				      اكتب ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	افعل {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		إذا (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			أرجع;
		}
	} طالما (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_پیغام = curl_multi_info_read(fetch_curl_multi, &queue);
	طالما (curl_پیغام) {
		بدل (curl_پیغام->پیغام) {
			حالة CURLMSG_DONE:
				fetch_curl_done(curl_پیغام->easy_handle,
						curl_پیغام->data.result);
				اكسر;
			افتراضي:
				اكسر;
		}
		curl_پیغام = curl_multi_info_read(fetch_curl_multi, &queue);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
ساكن صحيح
fetch_curl_progress(فراغ *clientp,
		    NSCURL_PROGRESS_T dltotal,
		    NSCURL_PROGRESS_T dlnow,
		    NSCURL_PROGRESS_T ultotal,
		    NSCURL_PROGRESS_T ulnow)
{
	ساكن حرف fetch_progress_buffer[256]; /**< Progress buffer for cURL */
	بنية curl_fetch_info *f = (بنية curl_fetch_info *) clientp;
	uint64_t time_now_ms;
	fetch_پیغام پیغام;

	إذا (f->abort) {
		أرجع 0;
        }

	پیغام.type = FETCH_PROGRESS;
	پیغام.data.progress = fetch_progress_buffer;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&time_now_ms);
#define UPDATE_DELAY_MS (1000 / UPDATES_PER_SECOND)
	إذا (time_now_ms - f->last_progress_update < UPDATE_DELAY_MS) {
		أرجع 0;
        }
#undef UPDATE_DELAY_MS
	f->last_progress_update = time_now_ms;

	إذا (dltotal > 0) {
		snprintf(fetch_progress_buffer, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&پیغام, f->fetch_handle);
	} وإلا {
		snprintf(fetch_progress_buffer, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&پیغام, f->fetch_handle);
	}

	أرجع 0;
}


/**
 * Format curl debug for nslog
 */
ساكن صحيح
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 حرف *data,
		 size_t size,
		 فراغ *userptr)
{
	ساكن ثابت حرف s_infotype[CURLINFO_END][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	بدل(type) {
	حالة CURLINFO_TEXT:
	حالة CURLINFO_HEADER_OUT:
	حالة CURLINFO_HEADER_IN:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (صحيح)size - 1, data);
		اكسر;

	افتراضي:
		اكسر;
	}
	أرجع 0;
}


ساكن curl_socket_t fetch_curl_socket_open(فراغ *clientp,
		curlsocktype purpose, بنية curl_sockaddr *address)
{
	(فراغ) clientp;
	(فراغ) purpose;

	أرجع (curl_socket_t) guit->fetch->socket_open(
			address->family, address->socktype,
			address->protocol);
}

ساكن صحيح fetch_curl_socket_close(فراغ *clientp, curl_socket_t item)
{
	(فراغ) clientp;

	أرجع guit->fetch->socket_close((صحيح) item);
}

/**
 * Callback function for cURL.
 */
ساكن size_t fetch_curl_data(حرف *data, size_t size, size_t nmemb, فراغ *_f)
{
	بنية curl_fetch_info *f = _f;
	CURLcode code;
	fetch_پیغام پیغام;

	/* ensure we only have to get this information once */
	إذا (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		أكد(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	إذا (f->http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_code = 0;
		أرجع size * nmemb;
	}

	إذا (f->abort || (!f->had_headers && fetch_curl_process_headers(f))) {
		f->stopped = true;
		أرجع 0;
	}

	/* send data to the caller */
	پیغام.type = FETCH_DATA;
	پیغام.data.header_or_data.buf = (ثابت uint8_t *) data;
	پیغام.data.header_or_data.len = size * nmemb;
	fetch_send_callback(&پیغام, f->fetch_handle);

	إذا (f->abort) {
		f->stopped = true;
		أرجع 0;
	}

	أرجع size * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
ساكن size_t
fetch_curl_header(حرف *data, size_t size, size_t nmemb, فراغ *_f)
{
	بنية curl_fetch_info *f = _f;
	صحيح i;
	fetch_پیغام پیغام;
	size *= nmemb;

	إذا (f->abort) {
		f->stopped = true;
		أرجع 0;
	}

	إذا (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	پیغام.type = FETCH_HEADER;
	پیغام.data.header_or_data.buf = (ثابت uint8_t *) data;
	پیغام.data.header_or_data.len = size;
	fetch_send_callback(&پیغام, f->fetch_handle);

#define SKIP_ST(o) لكل (i = (o); i < (صحيح) size && (data[i] == ' ' || data[i] == '\t'); i++)

	إذا (12 < size && strncasecmp(data, "Location:", 9) == 0) {
		/* extract Location header */
		حرر(f->location);
		f->location = malloc(size);
		إذا (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			أرجع size;
		}
		SKIP_ST(9);
		strncpy(f->location, data + i, size - i);
		f->location[size - i] = '\0';
		لكل (i = size - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} وإلا إذا (15 < size && strncasecmp(data, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		إذا (i < (صحيح)size && '0' <= data[i] && data[i] <= '9')
			f->content_length = atol(data + i);
	} وإلا إذا (17 < size && strncasecmp(data, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		طالما (i < (صحيح) size - 5 &&
				strncasecmp(data + i, "realm", 5))
			i++;
		طالما (i < (صحيح) size - 1 && data[++i] != '"')
			/* */;
		i++;

		إذا (i < (صحيح) size) {
			size_t end = i;

			طالما (end < size && data[end] != '"')
				++end;

			إذا (end < size) {
				حرر(f->realm);
				f->realm = malloc(end - i + 1);
				إذا (f->realm != عدم) {
					strncpy(f->realm, data + i, end - i);
					f->realm[end - i] = '\0';
				}
			}
		}
	} وإلا إذا (11 < size && strncasecmp(data, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_set_cookie(f->fetch_handle, &data[i]);
	}

	أرجع size;
#undef SKIP_ST
}

ساكن صحيح fetch_curl_fdset(lwc_string *scheme, fd_set *read_set,
			    fd_set *write_set, fd_set *error_set)
{
	CURLMcode code;
	صحيح maxfd = -1;

	code = curl_multi_fdset(fetch_curl_multi,
				read_set,
				write_set,
				error_set,
				&maxfd);
	أكد(code == CURLM_OK);

	أرجع maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_register(فراغ)
{
	CURLcode code;
	curl_version_info_data *data;
	صحيح i;
	lwc_string *scheme;
	ثابت بنية fetcher_operation_table fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.setup = fetch_curl_setup,
		.start = fetch_curl_start,
		.abort = fetch_curl_abort,
		.حرر = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#إذا LIBCURL_VERSION_عدد >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, عدم, عدم);
	إذا (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} وإلا {
		curl_with_openssl = false;
	}
#endif

	NSLOG(netsurf, INFO, "curl_version %s", curl_version());

	code = curl_global_init(CURL_GLOBAL_ALL);
	إذا (code != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		أرجع NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	إذا (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		أرجع NSERROR_INIT_FAILED;
	}

#إذا LIBCURL_VERSION_عدد >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		صحيح maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#undef SETOPT
#define SETOPT(option, قدر) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, قدر);	\
	إذا (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		اذهب curl_multi_setopt_failed;				\
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
	إذا (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		أرجع NSERROR_INIT_FAILED;
	}

#undef SETOPT
#define SETOPT(option, قدر) \
	code = curl_easy_setopt(fetch_blank_curl, option, قدر);	\
	إذا (code != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		اذهب curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_buffer);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	إذا (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} وإلا {
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

	إذا (nsoption_charp(ca_bundle) &&
	    strcmp(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	إذا (nsoption_charp(ca_path) && strcmp(nsoption_charp(ca_path), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_path));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_path));
	}

#إذا LIBCURL_VERSION_عدد < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	code = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, عدم);
	إذا (code != CURLE_OK) {
		curl_with_openssl = false;
	} وإلا {
		curl_with_openssl = true;
	}
#endif

	إذا (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#إذا LIBCURL_VERSION_عدد >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		code = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		إذا (code != CURLE_OK && code != CURLE_NOT_BUILT_IN)
			اذهب curl_easy_setopt_failed;
#endif
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	data = curl_version_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	إذا (curl_fetch_ssl_hashmap == عدم) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		أرجع NSERROR_NOMEM;
	}

	لكل (i = 0; data->protocols[i]; i++) {
		إذا (strcmp(data->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} وإلا إذا (strcmp(data->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} وإلا {
			/* Ignore non-http(s) protocols */
			استمر;
		}

		إذا (fetcher_add(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      data->protocols[i]);
		}
	}

	أرجع NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	أرجع NSERROR_INIT_FAILED;

#إذا LIBCURL_VERSION_عدد >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	أرجع NSERROR_INIT_FAILED;
#endif
}
