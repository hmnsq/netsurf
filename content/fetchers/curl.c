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
#अन्तर्भाव "utils/inet.h"

#अन्तर्भाव <assert.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <sys/stat.h>

#अन्तर्भाव <libwapcaplet/libwapcaplet.h>
#अन्तर्भाव <nsutils/time.h>

#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/hashmap.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/ring.h"
#अन्तर्भाव "utils/useragent.h"
#अन्तर्भाव "utils/file.h"
#अन्तर्भाव "utils/string.h"
#अन्तर्भाव "netsurf/fetch.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "content/fetch.h"
#अन्तर्भाव "content/fetchers.h"
#अन्तर्भाव "content/fetchers/curl.h"
#अन्तर्भाव "content/urldb.h"

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

#अन्तर्भाव <openssl/ssl.h>
#अन्तर्भाव <openssl/x509v3.h>

#अन्यथा /* WITH_OPENSSL */

प्रकारनिर्देश वर्ण X509;

स्थिर शून्य X509_free(X509 *cert)
{
	मुक्त(cert);
}

#endif /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
स्थिर uint32_t
curl_fetch_ssl_कुंजी_hash(शून्य *कुंजी)
{
	nsurl *url = कुंजी;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *port = nsurl_get_component(url, NSURL_PORT);
	uint32_t hash;

	यदि (port == रिक्त)
		port = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_मूल्य(hostname) ^ lwc_string_hash_मूल्य(port);

	lwc_string_unref(hostname);
	lwc_string_unref(port);

	प्रत्यागम hash;
}

/* We only compare the hostname and port */
स्थिर bool
curl_fetch_ssl_कुंजी_eq(शून्य *key1, शून्य *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_PORT);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_PORT);
	bool iseq = false;

	यदि (port1 == रिक्त)
		port1 = lwc_string_ref(corestring_lwc_443);
	यदि (port2 == रिक्त)
		port2 = lwc_string_ref(corestring_lwc_443);

	यदि (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		गच्छ out;

	iseq = false;
	यदि (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		गच्छ out;

out:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	प्रत्यागम iseq;
}

स्थिर शून्य *
curl_fetch_ssl_मूल्य_alloc(शून्य *कुंजी)
{
	संरचना cert_chain *out;

	यदि (cert_chain_alloc(0, &out) != NSERROR_OK) {
		प्रत्यागम रिक्त;
	}

	प्रत्यागम out;
}

स्थिर शून्य
curl_fetch_ssl_मूल्य_destroy(शून्य *मूल्य)
{
	संरचना cert_chain *chain = मूल्य;
	यदि (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

स्थिर hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.कुंजी_clone = (hashmap_कुंजी_clone_t)nsurl_ref,
	.कुंजी_destroy = (hashmap_कुंजी_destroy_t)nsurl_unref,
	.कुंजी_eq = curl_fetch_ssl_कुंजी_eq,
	.कुंजी_hash = curl_fetch_ssl_कुंजी_hash,
	.मूल्य_alloc = curl_fetch_ssl_मूल्य_alloc,
	.मूल्य_destroy = curl_fetch_ssl_मूल्य_destroy,
};

स्थिर hashmap_t *curl_fetch_ssl_hashmap = रिक्त;

/** SSL certificate info */
संरचना cert_info {
	X509 *cert;		/**< Pointer to certificate */
	दीर्घ err;		/**< OpenSSL error code */
};

#यदि LIBCURL_VERSION_अंक >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_XFERINFOFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_XFERINFODATA
#define NSCURL_PROGRESS_T curl_off_t
#अन्यथा
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_PROGRESSFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_PROGRESSDATA
#define NSCURL_PROGRESS_T द्विगुण
#endif

#यदि LIBCURL_VERSION_अंक >= 0x073800 /* 7.56.0 depricated curl_formadd */
#define NSCURL_POSTDATA_T curl_mime
#define NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#define NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#अन्यथा
#define NSCURL_POSTDATA_T संरचना curl_httppost
#define NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#define NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#endif

/** Information for a single fetch. */
संरचना curl_fetch_info {
	संरचना fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool sent_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_headers;	/**< Headers have been processed. */
	bool पात;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	संरचना curl_slist *headers;	/**< List of request headers. */
	वर्ण *location;		/**< Response Location header, or 0. */
	अचिन्हित दीर्घ content_length;	/**< Response Content-Length, or 0. */
	वर्ण *cookie_string;	/**< Cookie string for this fetch */
	वर्ण *realm;		/**< HTTP Auth Realm */
	संरचना fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	दीर्घ http_code; /**< HTTP result code from cURL. */

	uint64_t last_progress_update;	/**< Time of last progress update */
	पूर्णाङ्क cert_depth; /**< deepest certificate in use */
	संरचना cert_info cert_data[MAX_CERT_DEPTH]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
संरचना cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	संरचना cache_handle *r_prev; /**< Previous cached handle in ring. */
	संरचना cache_handle *r_next; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
स्थिर CURL *fetch_blank_curl;

/** Ring of cached handles */
स्थिर संरचना cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
स्थिर पूर्णाङ्क curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
स्थिर bool curl_with_openssl;

/** Error buffer for cURL. */
स्थिर वर्ण fetch_error_buffer[CURL_ERROR_SIZE];

/** Proxy authentication details. */
स्थिर वर्ण fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
स्थिर bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
स्थिर bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_data(scheme));
	curl_fetchers_registered++;
	प्रत्यागम true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
स्थिर शून्य fetch_curl_finalise(lwc_string *scheme)
{
	संरचना cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_data(scheme));
	यदि (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		यदि (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = रिक्त;
	}

	/* Free anything remaining in the cached curl handle ring */
	यावत् (curl_handle_ring != रिक्त) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
		curl_easy_cleanup(h->handle);
		मुक्त(h);
	}
}


/**
 * Check if this fetcher can fetch a url.
 *
 * \param url The url to check.
 * \return true if the fetcher supports the url else false.
 */
स्थिर bool fetch_curl_can_fetch(ध्रुव nsurl *url)
{
	प्रत्यागम nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
स्थिर संरचना fetch_postdata *
fetch_curl_alloc_postdata(ध्रुव वर्ण *post_urlenc,
			  ध्रुव संरचना fetch_multipart_data *post_multipart)
{
	संरचना fetch_postdata *postdata;
	postdata = सुस्मृति(1, परिमाण(संरचना fetch_postdata));
	यदि (postdata != रिक्त) {

		यदि (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->data.urlenc = strdup(post_urlenc);
			यदि (postdata->data.urlenc == रिक्त) {
				मुक्त(postdata);
				postdata = रिक्त;
			}
		} अन्यथा यदि (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->data.multipart = fetch_multipart_data_clone(post_multipart);
			यदि (postdata->data.multipart == रिक्त) {
				मुक्त(postdata);
				postdata = रिक्त;
			}
		} अन्यथा {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	प्रत्यागम postdata;
}

/**
 * free postdata
 */
स्थिर शून्य fetch_curl_free_postdata(संरचना fetch_postdata *postdata)
{
	यदि (postdata != रिक्त) {
		विकल्प (postdata->type) {
		पक्ष FETCH_POSTDATA_NONE:
			भङ्ग;
		पक्ष FETCH_POSTDATA_URLENC:
			मुक्त(postdata->data.urlenc);
			भङ्ग;
		पक्ष FETCH_POSTDATA_MULTIPART:
			fetch_multipart_data_destroy(postdata->data.multipart);
			भङ्ग;
		}

		मुक्त(postdata);
	}
}

/**
 *construct a new fetch structure
 */
स्थिर संरचना curl_fetch_info *fetch_alloc(शून्य)
{
	संरचना curl_fetch_info *fetch;
	fetch = दो_स्मृति(परिमाण (*fetch));
	यदि (fetch == रिक्त)
		प्रत्यागम रिक्त;

	fetch->curl_handle = रिक्त;
	fetch->sent_ssl_chain = false;
	fetch->had_headers = false;
	fetch->पात = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->headers = रिक्त;
	fetch->url = रिक्त;
	fetch->host = रिक्त;
	fetch->location = रिक्त;
	fetch->content_length = 0;
	fetch->http_code = 0;
	fetch->cookie_string = रिक्त;
	fetch->realm = रिक्त;
	fetch->last_progress_update = 0;
	fetch->postdata = रिक्त;
	fetch->curl_postdata = रिक्त;

	/* Clear certificate chain data */
	स_रखो(fetch->cert_data, 0, परिमाण(fetch->cert_data));
	fetch->cert_depth = -1;

	प्रत्यागम fetch;
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
स्थिर शून्य *
fetch_curl_setup(संरचना fetch *parent_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ध्रुव वर्ण *post_urlenc,
		 ध्रुव संरचना fetch_multipart_data *post_multipart,
		 ध्रुव वर्ण **headers)
{
	संरचना curl_fetch_info *fetch;
	संरचना curl_slist *slist;
	पूर्णाङ्क i;

	fetch = fetch_alloc();
	यदि (fetch == रिक्त)
		प्रत्यागम रिक्त;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = parent_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	यदि (fetch->host == रिक्त) {
		गच्छ failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	यदि (fetch->postdata == रिक्त) {
		गच्छ failed;
	}

#define APPEND(list, मूल्य) \
	slist = curl_slist_append(list, मूल्य);		\
	यदि (slist == रिक्त)				\
		गच्छ failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->headers, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->headers, "Expect:");

	यदि ((nsoption_charp(accept_language) != रिक्त) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		वर्ण s[80];
		snprintf(s, परिमाण s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[परिमाण s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	यदि (nsoption_charp(accept_charset) != रिक्त &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		वर्ण s[80];
		snprintf(s, परिमाण s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[परिमाण s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	यदि (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->headers, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	प्रति (i = 0; headers[i] != रिक्त; i++) {
		APPEND(fetch->headers, headers[i]);
	}

	प्रत्यागम fetch;

#undef APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->headers);
	मुक्त(fetch);
	प्रत्यागम रिक्त;
}


#ifdef WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
स्थिर संरचना cert_chain *
fetch_curl_get_cached_chain(संरचना curl_fetch_info *f)
{
	संरचना cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	यदि (chain == रिक्त) {
		chain = hashmap_जोड़ो(curl_fetch_ssl_hashmap, f->url);
	}

	प्रत्यागम chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
स्थिर शून्य
fetch_curl_store_certs_in_cache(संरचना curl_fetch_info *f)
{
	माप_प्रकार depth;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_DEPTH];
	संरचना cert_chain chain, *cached_chain;
	संरचना cert_info *certs;

	स_रखो(&chain, 0, परिमाण(chain));

	certs = f->cert_data;
	chain.depth = f->cert_depth + 1; /* 0 indexed certificate depth */

	प्रति (depth = 0; depth < chain.depth; depth++) {
		यदि (certs[depth].cert == रिक्त) {
			/* This certificate is missing, skip it */
			chain.certs[depth].err = SSL_CERT_ERR_CERT_MISSING;
			अनुवर्त;
		}

		/* error code (if any) */
		विकल्प (certs[depth].err) {
		पक्ष X509_V_OK:
			chain.certs[depth].err = SSL_CERT_ERR_OK;
			भङ्ग;

		पक्ष X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			/* fallthrough */
		पक्ष X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_कुंजी:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_ISSUER;
			भङ्ग;

		पक्ष X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		पक्ष X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		पक्ष X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		पक्ष X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_SIG;
			भङ्ग;

		पक्ष X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		पक्ष X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_YOUNG;
			भङ्ग;

		पक्ष X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		पक्ष X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_OLD;
			भङ्ग;

		पक्ष X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			chain.certs[depth].err = SSL_CERT_ERR_SELF_SIGNED;
			भङ्ग;

		पक्ष X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			chain.certs[depth].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			भङ्ग;

		पक्ष X509_V_ERR_CERT_REVOKED:
			chain.certs[depth].err = SSL_CERT_ERR_REVOKED;
			भङ्ग;

		पक्ष X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[depth].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			भङ्ग;

		स्वभाव:
			chain.certs[depth].err = SSL_CERT_ERR_UNKNOWN;
			भङ्ग;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[depth].cert);
		BIO_get_mem_ptr(mem, &buf[depth]);
		(शून्य) BIO_set_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[depth].der = (uint8_t *)buf[depth]->data;
		chain.certs[depth].der_length = buf[depth]->length;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	यदि (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	प्रति (depth = 0; depth < chain.depth; depth++) {
		यदि (chain.certs[depth].err == SSL_CERT_ERR_CERT_MISSING) {
			अनुवर्त;
		}
		यदि (buf[depth] != रिक्त) {
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
स्थिर पूर्णाङ्क
fetch_curl_verify_callback(पूर्णाङ्क verify_ok, X509_STORE_CTX *x509_ctx)
{
	पूर्णाङ्क depth;
	संरचना curl_fetch_info *fetch;

	depth = X509_STORE_CTX_get_error_depth(x509_ctx);
	fetch = X509_STORE_CTX_get_app_data(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	यदि (depth >= MAX_CERT_DEPTH) {
		X509_STORE_CTX_set_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		प्रत्यागम 0;
	}

	/* record the max depth */
	यदि (depth > fetch->cert_depth) {
		fetch->cert_depth = depth;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	यदि (!fetch->cert_data[depth].cert) {
		fetch->cert_data[depth].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_up_ref(fetch->cert_data[depth].cert);
		fetch->cert_data[depth].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	यदि (depth > 0) {
		verify_ok = 1;
	} अन्यथा {
		/* search for deeper certificates in the chain with errors */
		प्रति (depth = fetch->cert_depth; depth > 0; depth--) {
			यदि (fetch->cert_data[depth].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_set_error(x509_ctx, fetch->cert_data[depth].err);
			}
		}
	}

	प्रत्यागम verify_ok;
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
स्थिर पूर्णाङ्क fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, शून्य *parm)
{
	संरचना curl_fetch_info *f = (संरचना curl_fetch_info *) parm;
	पूर्णाङ्क ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_set_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_data(f->host),
					 lwc_string_length(f->host));

	/* Store fetch struct in context for verify callback */
	यदि (ok) {
		ok = X509_STORE_CTX_set_app_data(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	यदि (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_in_cache(f);

	प्रत्यागम ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
स्थिर CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, शून्य *_sslctx, शून्य *parm)
{
	संरचना curl_fetch_info *f = (संरचना curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	दीर्घ options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_set_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	यदि (f->downgrade_tls) {
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

	प्रत्यागम CURLE_OK;
}


#endif /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
स्थिर शून्य
fetch_curl_report_certs_upstream(संरचना curl_fetch_info *f)
{
	fetch_संदेश संदेश;
	संरचना cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	यदि (chain != रिक्त) {
		संदेश.type = FETCH_CERTS;
		संदेश.data.chain = chain;

		fetch_send_callback(&संदेश, f->fetch_handle);
	}

	f->sent_ssl_chain = true;
}

#यदि LIBCURL_VERSION_अंक >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
संरचना curl_mime_ctx {
	वर्ण *buffer;
	curl_off_t size;
	curl_off_t position;
};

स्थिर माप_प्रकार mime_data_read_callback(वर्ण *buffer, माप_प्रकार size, माप_प्रकार nitems, शून्य *arg)
{
	संरचना curl_mime_ctx *mctx = (संरचना curl_mime_ctx *) arg;
	curl_off_t sz = mctx->size - mctx->position;

	nitems *= size;
	यदि(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	यदि(sz) {
		स_नकल(buffer, mctx->buffer + mctx->position, sz);
	}
	mctx->position += sz;
	प्रत्यागम sz;
}

स्थिर पूर्णाङ्क mime_data_seek_callback(शून्य *arg, curl_off_t offset, पूर्णाङ्क origin)
{
	संरचना curl_mime_ctx *mctx = (संरचना curl_mime_ctx *) arg;

	विकल्प(origin) {
	पक्ष अंत_से:
		offset += mctx->size;
		भङ्ग;
	पक्ष प्रस्तुत_से:
		offset += mctx->position;
		भङ्ग;
	}

	यदि(offset < 0) {
		प्रत्यागम CURL_SEEKFUNC_FAIL;
	}
	mctx->position = offset;
	प्रत्यागम CURL_SEEKFUNC_OK;
}

स्थिर शून्य mime_data_free_callback(शून्य *arg)
{
	संरचना curl_mime_ctx *mctx = (संरचना curl_mime_ctx *) arg;
	मुक्त(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
स्थिर curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    ध्रुव संरचना fetch_multipart_data *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode code = CURLE_OK;
	माप_प्रकार मूल्य_len;

	cmime = curl_mime_init(chandle);
	यदि (cmime == रिक्त) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		प्रत्यागम रिक्त;
	}

	/* iterate post data */
	प्रति (; multipart != रिक्त; multipart = multipart->next) {
		part = curl_mime_addpart(cmime);
		यदि (part == रिक्त) {
			गच्छ convert_failed;
		}

		code = curl_mime_name(part, multipart->name);
		यदि (code != CURLE_OK) {
			गच्छ convert_failed;
		}

		मूल्य_len = म_माप(multipart->मूल्य);

		यदि (multipart->file && मूल्य_len==0) {
			/* file entries with no filename require special handling */
			code=curl_mime_data(part, multipart->मूल्य, मूल्य_len);
			यदि (code != CURLE_OK) {
				गच्छ convert_failed;
			}

			code = curl_mime_filename(part, "");
			यदि (code != CURLE_OK) {
				गच्छ convert_failed;
			}

			code = curl_mime_type(part, "application/octet-stream");
			यदि (code != CURLE_OK) {
				गच्छ convert_failed;
			}

		} अन्यथा यदि(multipart->file) {
			/* file entry */
			nserror ret;
			वर्ण *leafname = रिक्त;
			वर्ण *mimetype = रिक्त;

			code = curl_mime_filedata(part, multipart->rawfile);
			यदि (code != CURLE_OK) {
				गच्छ convert_failed;
			}

			ret = guit->file->basename(multipart->मूल्य, &leafname, रिक्त);
			यदि (ret != NSERROR_OK) {
				गच्छ convert_failed;
			}
			code = curl_mime_filename(part, leafname);
			मुक्त(leafname);
			यदि (code != CURLE_OK) {
				गच्छ convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->मूल्य);
			यदि (mimetype == रिक्त) {
				mimetype=strdup("text/plain");
			}
			यदि (mimetype == रिक्त) {
				गच्छ convert_failed;
			}
			code = curl_mime_type(part, mimetype);
			मुक्त(mimetype);
			यदि (code != CURLE_OK) {
				गच्छ convert_failed;
			}

		} अन्यथा {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			संरचना curl_mime_ctx *cb_ctx;
			cb_ctx = दो_स्मृति(परिमाण(संरचना curl_mime_ctx));
			यदि (cb_ctx == रिक्त) {
				गच्छ convert_failed;
			}
			cb_ctx->buffer = multipart->मूल्य;
			cb_ctx->size = मूल्य_len;
			cb_ctx->position = 0;
			code = curl_mime_data_cb(part,
						 मूल्य_len,
						 mime_data_read_callback,
						 mime_data_seek_callback,
						 mime_data_free_callback,
						 cb_ctx);
			यदि (code != CURLE_OK) {
				मुक्त(cb_ctx);
				गच्छ convert_failed;
			}
		}
	}

	प्रत्यागम cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", code);
	curl_mime_free(cmime);
	प्रत्यागम रिक्त;
}

#अन्यथा /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
स्थिर संरचना curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    ध्रुव संरचना fetch_multipart_data *control)
{
	संरचना curl_httppost *post = रिक्त, *last = रिक्त;
	CURLFORMcode code;
	nserror ret;

	प्रति (; control; control = control->next) {
		यदि (control->file) {
			वर्ण *leafname = रिक्त;
			ret = guit->file->basename(control->मूल्य, &leafname, रिक्त);
			यदि (ret != NSERROR_OK) {
				अनुवर्त;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			यदि (control->मूल्य[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				स्थिर वर्ण buf;

				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_BUFFER, control->मूल्य,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_END);
				यदि (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", code,
					      control->name);
			} अन्यथा {
				वर्ण *mimetype = guit->fetch->mimetype(control->मूल्य);
				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_END);
				यदि (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      code,
					      control->name,
					      control->मूल्य);
				मुक्त(mimetype);
			}
			मुक्त(leafname);
		} अन्यथा {
			code = curl_formadd(&post, &last,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->मूल्य,
					    CURLFORM_END);
			यदि (code != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", code,
				      control->name, control->मूल्य);
		}
	}

	प्रत्यागम post;
}

#endif  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
स्थिर CURLcode fetch_curl_set_postdata(संरचना curl_fetch_info *f)
{
	CURLcode code = CURLE_OK;

#undef SETOPT
#define SETOPT(option, मूल्य) { \
	code = curl_easy_setopt(f->curl_handle, option, मूल्य);	\
	यदि (code != CURLE_OK)					\
		प्रत्यागम code;					\
	}

	विकल्प (f->postdata->type) {
	पक्ष FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, रिक्त);
		SETOPT(NSCURL_POSTDATA_CURLOPT, रिक्त);
		SETOPT(CURLOPT_HTTPGET, 1L);
		भङ्ग;

	पक्ष FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, रिक्त);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->data.urlenc);
		भङ्ग;

	पक्ष FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, रिक्त);
		SETOPT(CURLOPT_HTTPGET, 0L);
		यदि (f->curl_postdata == रिक्त) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->data.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		भङ्ग;
	}
	प्रत्यागम code;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
स्थिर CURLcode fetch_curl_set_options(संरचना curl_fetch_info *f)
{
	CURLcode code;
	ध्रुव वर्ण *auth;

#undef SETOPT
#define SETOPT(option, मूल्य) { \
	code = curl_easy_setopt(f->curl_handle, option, मूल्य);	\
	यदि (code != CURLE_OK)					\
		प्रत्यागम code;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_PRIVATE, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_PROGRESS_DATA, f);
	SETOPT(CURLOPT_HTTPHEADER, f->headers);
	code = fetch_curl_set_postdata(f);
	यदि (code != CURLE_OK) {
		प्रत्यागम code;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	यदि (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} अन्यथा {
		SETOPT(CURLOPT_COOKIE, रिक्त);
	}

	यदि ((auth = urldb_get_auth_details(f->url, रिक्त)) != रिक्त) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} अन्यथा {
		SETOPT(CURLOPT_USERPWD, रिक्त);
	}

	/* set up proxy options */
	यदि (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != रिक्त) &&
	    (म_भेदन(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (दीर्घ) nsoption_int(http_proxy_port));

#यदि LIBCURL_VERSION_अंक >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#endif

		यदि (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(दीर्घ) CURLAUTH_BASIC :
					(दीर्घ) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					परिमाण fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} अन्यथा {
		SETOPT(CURLOPT_PROXY, रिक्त);
	}


	यदि (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	यदि (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		यदि (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, रिक्त);
			SETOPT(CURLOPT_SSL_CTX_DATA, रिक्त);
		}
	} अन्यथा {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef WITH_OPENSSL
		यदि (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_DATA, f);
		}
#endif
	}

	प्रत्यागम CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
स्थिर bool
fetch_curl_initiate_fetch(संरचना curl_fetch_info *fetch, CURL *handle)
{
	CURLcode code;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	code = fetch_curl_set_options(fetch);
	यदि (code != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		प्रत्यागम false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_add_handle(fetch_curl_multi, fetch->curl_handle);
	प्रतिज्ञा(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	प्रत्यागम true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
स्थिर CURL *fetch_curl_get_handle(lwc_string *host)
{
	संरचना cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	यदि (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		मुक्त(h);
	} अन्यथा {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	प्रत्यागम ret;
}


/**
 * Dispatch a single job
 */
स्थिर bool fetch_curl_start(शून्य *vfetch)
{
	संरचना curl_fetch_info *fetch = (संरचना curl_fetch_info*)vfetch;
	यदि (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		प्रत्यागम false;
	}
	प्रत्यागम fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
स्थिर शून्य fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#यदि LIBCURL_VERSION_अंक >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	प्रत्यागम;
#अन्यथा
	संरचना cache_handle *h = 0;
	पूर्णाङ्क c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	यदि (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		प्रत्यागम;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(संरचना cache_handle, curl_handle_ring, c);
	यदि (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		यदि (curl_handle_ring != रिक्त) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_next;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} अन्यथा {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		प्रत्यागम;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (संरचना cache_handle*)दो_स्मृति(परिमाण(संरचना cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_जोड़ो(curl_handle_ring, h);
#endif
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
स्थिर शून्य fetch_curl_stop(संरचना curl_fetch_info *f)
{
	CURLMcode codem;

	प्रतिज्ञा(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	यदि (f->curl_handle) {
		/* remove from curl multi handle */
		codem = curl_multi_remove_handle(fetch_curl_multi,
				f->curl_handle);
		प्रतिज्ञा(codem == CURLM_OK);
		/* Put this curl handle into the cache if wanted. */
		fetch_curl_cache_handle(f->curl_handle, f->host);
		f->curl_handle = 0;
	}

	fetch_remove_from_queues(f->fetch_handle);
}


/**
 * Abort a fetch.
 */
स्थिर शून्य fetch_curl_abort(शून्य *vf)
{
	संरचना curl_fetch_info *f = (संरचना curl_fetch_info *)vf;
	प्रतिज्ञा(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	यदि (f->curl_handle) {
		यदि (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->पात = true;
		} अन्यथा {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} अन्यथा {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
स्थिर शून्य fetch_curl_free(शून्य *vf)
{
	संरचना curl_fetch_info *f = (संरचना curl_fetch_info *)vf;
	पूर्णाङ्क i;

	यदि (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	मुक्त(f->location);
	मुक्त(f->cookie_string);
	मुक्त(f->realm);
	यदि (f->headers) {
		curl_slist_free_all(f->headers);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	प्रति (i = 0; i < MAX_CERT_DEPTH; i++) {
		यदि (f->cert_data[i].cert != रिक्त) {
			X509_free(f->cert_data[i].cert);
		}
	}

	मुक्त(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
स्थिर bool fetch_curl_process_headers(संरचना curl_fetch_info *f)
{
	दीर्घ http_code;
	CURLcode code;
	fetch_संदेश संदेश;

	f->had_headers = true;

	यदि (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		प्रतिज्ञा(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}
	http_code = f->http_code;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_code);

	यदि ((http_code == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		संदेश.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&संदेश, f->fetch_handle);
		प्रत्यागम true;
	}

	/* handle HTTP redirects (3xx response codes) */
	यदि (HTTP_RESPONSE_IS_3XX(http_code) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		संदेश.type = FETCH_REDIRECT;
		संदेश.data.redirect = f->location;
		fetch_send_callback(&संदेश, f->fetch_handle);
		प्रत्यागम true;
	}

	/* handle HTTP 401 (Authentication errors) */
	यदि (http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		संदेश.type = FETCH_AUTH;
		संदेश.data.auth.realm = f->realm;
		fetch_send_callback(&संदेश, f->fetch_handle);
		प्रत्यागम true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	यदि (f->only_2xx &&
	    म_भेदन(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_code) == false)) {
		संदेश.type = FETCH_ERROR;
		संदेश.data.error = messages_get("Not2xx");
		fetch_send_callback(&संदेश, f->fetch_handle);
		प्रत्यागम true;
	}

	यदि (f->पात)
		प्रत्यागम true;

	प्रत्यागम false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
स्थिर शून्य fetch_curl_done(CURL *curl_handle, CURLcode result)
{
	bool finished = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	संरचना curl_fetch_info *f;
	वर्ण **_hideous_hack = (वर्ण **) (शून्य *) &f;
	CURLcode code;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	code = curl_easy_getinfo(curl_handle, CURLINFO_PRIVATE, _hideous_hack);
	प्रतिज्ञा(code == CURLE_OK);

	abort_fetch = f->पात;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	यदि ((abort_fetch == false) &&
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
		यदि (f->stopped ||
		    (!f->had_headers &&	fetch_curl_process_headers(f))) {
			; /* redirect with no body or similar */
		} अन्यथा {
			finished = true;
		}
	} अन्यथा यदि (result == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		यदि (!f->had_headers && fetch_curl_process_headers(f))
			; /* redirect with partial body, or similar */
		अन्यथा {
			finished = true;
		}
	} अन्यथा यदि (result == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} अन्यथा यदि (result == CURLE_SSL_PEER_CERTIFICATE ||
		   result == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} अन्यथा {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", result);
		error = true;
	}

	fetch_curl_stop(f);

	यदि (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	यदि (abort_fetch) {
		; /* fetch was aborted: no callback */
	} अन्यथा यदि (finished) {
		fetch_संदेश संदेश;
		संदेश.type = FETCH_FINISHED;
		fetch_send_callback(&संदेश, f->fetch_handle);
	} अन्यथा यदि (cert) {
		/* user needs to validate certificate with issue */
		fetch_संदेश संदेश;
		संदेश.type = FETCH_CERT_ERR;
		fetch_send_callback(&संदेश, f->fetch_handle);
	} अन्यथा यदि (error) {
		fetch_संदेश संदेश;
		विकल्प (result) {
		पक्ष CURLE_SSL_CONNECT_ERROR:
			संदेश.type = FETCH_SSL_ERR;
			भङ्ग;

		पक्ष CURLE_OPERATION_TIMEDOUT:
			संदेश.type = FETCH_TIMEDOUT;
			संदेश.data.error = curl_easy_strerror(result);
			भङ्ग;

		स्वभाव:
			संदेश.type = FETCH_ERROR;
			संदेश.data.error = curl_easy_strerror(result);
		}

		fetch_send_callback(&संदेश, f->fetch_handle);
	}

	fetch_free(f->fetch_handle);
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */
स्थिर शून्य fetch_curl_poll(lwc_string *scheme_ignored)
{
	पूर्णाङ्क running, queue;
	CURLMcode codem;
	CURLMsg *curl_संदेश;

	यदि (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_set, write_fd_set, exc_fd_set;
		पूर्णाङ्क max_fd = -1;
		पूर्णाङ्क i;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&exc_fd_set);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_set, &write_fd_set,
				&exc_fd_set, &max_fd);
		प्रतिज्ञा(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		प्रति (i = 0; i <= max_fd; i++) {
			bool पठ = false;
			bool लिख = false;
			bool error = false;

			यदि (FD_ISSET(i, &read_fd_set)) {
				पठ = true;
			}
			यदि (FD_ISSET(i, &write_fd_set)) {
				लिख = true;
			}
			यदि (FD_ISSET(i, &exc_fd_set)) {
				error = true;
			}
			यदि (पठ || लिख || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      पठ ? "read" : "    ",
				      लिख ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	कुरु {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		यदि (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			प्रत्यागम;
		}
	} यावत् (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_संदेश = curl_multi_info_read(fetch_curl_multi, &queue);
	यावत् (curl_संदेश) {
		विकल्प (curl_संदेश->संदेश) {
			पक्ष CURLMSG_DONE:
				fetch_curl_done(curl_संदेश->easy_handle,
						curl_संदेश->data.result);
				भङ्ग;
			स्वभाव:
				भङ्ग;
		}
		curl_संदेश = curl_multi_info_read(fetch_curl_multi, &queue);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
स्थिर पूर्णाङ्क
fetch_curl_progress(शून्य *clientp,
		    NSCURL_PROGRESS_T dltotal,
		    NSCURL_PROGRESS_T dlnow,
		    NSCURL_PROGRESS_T ultotal,
		    NSCURL_PROGRESS_T ulnow)
{
	स्थिर वर्ण fetch_progress_buffer[256]; /**< Progress buffer for cURL */
	संरचना curl_fetch_info *f = (संरचना curl_fetch_info *) clientp;
	uint64_t time_now_ms;
	fetch_संदेश संदेश;

	यदि (f->पात) {
		प्रत्यागम 0;
        }

	संदेश.type = FETCH_PROGRESS;
	संदेश.data.progress = fetch_progress_buffer;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&time_now_ms);
#define UPDATE_DELAY_MS (1000 / UPDATES_PER_SECOND)
	यदि (time_now_ms - f->last_progress_update < UPDATE_DELAY_MS) {
		प्रत्यागम 0;
        }
#undef UPDATE_DELAY_MS
	f->last_progress_update = time_now_ms;

	यदि (dltotal > 0) {
		snprintf(fetch_progress_buffer, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&संदेश, f->fetch_handle);
	} अन्यथा {
		snprintf(fetch_progress_buffer, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&संदेश, f->fetch_handle);
	}

	प्रत्यागम 0;
}


/**
 * Format curl debug for nslog
 */
स्थिर पूर्णाङ्क
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 वर्ण *data,
		 माप_प्रकार size,
		 शून्य *userptr)
{
	स्थिर ध्रुव वर्ण s_infotype[CURLINFO_END][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	विकल्प(type) {
	पक्ष CURLINFO_TEXT:
	पक्ष CURLINFO_HEADER_OUT:
	पक्ष CURLINFO_HEADER_IN:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (पूर्णाङ्क)size - 1, data);
		भङ्ग;

	स्वभाव:
		भङ्ग;
	}
	प्रत्यागम 0;
}


स्थिर curl_socket_t fetch_curl_socket_open(शून्य *clientp,
		curlsocktype purpose, संरचना curl_sockaddr *address)
{
	(शून्य) clientp;
	(शून्य) purpose;

	प्रत्यागम (curl_socket_t) guit->fetch->socket_open(
			address->family, address->socktype,
			address->protocol);
}

स्थिर पूर्णाङ्क fetch_curl_socket_close(शून्य *clientp, curl_socket_t item)
{
	(शून्य) clientp;

	प्रत्यागम guit->fetch->socket_close((पूर्णाङ्क) item);
}

/**
 * Callback function for cURL.
 */
स्थिर माप_प्रकार fetch_curl_data(वर्ण *data, माप_प्रकार size, माप_प्रकार nmemb, शून्य *_f)
{
	संरचना curl_fetch_info *f = _f;
	CURLcode code;
	fetch_संदेश संदेश;

	/* ensure we only have to get this information once */
	यदि (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		प्रतिज्ञा(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	यदि (f->http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_code = 0;
		प्रत्यागम size * nmemb;
	}

	यदि (f->पात || (!f->had_headers && fetch_curl_process_headers(f))) {
		f->stopped = true;
		प्रत्यागम 0;
	}

	/* send data to the caller */
	संदेश.type = FETCH_DATA;
	संदेश.data.header_or_data.buf = (ध्रुव uint8_t *) data;
	संदेश.data.header_or_data.len = size * nmemb;
	fetch_send_callback(&संदेश, f->fetch_handle);

	यदि (f->पात) {
		f->stopped = true;
		प्रत्यागम 0;
	}

	प्रत्यागम size * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
स्थिर माप_प्रकार
fetch_curl_header(वर्ण *data, माप_प्रकार size, माप_प्रकार nmemb, शून्य *_f)
{
	संरचना curl_fetch_info *f = _f;
	पूर्णाङ्क i;
	fetch_संदेश संदेश;
	size *= nmemb;

	यदि (f->पात) {
		f->stopped = true;
		प्रत्यागम 0;
	}

	यदि (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	संदेश.type = FETCH_HEADER;
	संदेश.data.header_or_data.buf = (ध्रुव uint8_t *) data;
	संदेश.data.header_or_data.len = size;
	fetch_send_callback(&संदेश, f->fetch_handle);

#define SKIP_ST(o) प्रति (i = (o); i < (पूर्णाङ्क) size && (data[i] == ' ' || data[i] == '\t'); i++)

	यदि (12 < size && strncasecmp(data, "Location:", 9) == 0) {
		/* extract Location header */
		मुक्त(f->location);
		f->location = दो_स्मृति(size);
		यदि (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			प्रत्यागम size;
		}
		SKIP_ST(9);
		म_नकलन(f->location, data + i, size - i);
		f->location[size - i] = '\0';
		प्रति (i = size - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} अन्यथा यदि (15 < size && strncasecmp(data, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		यदि (i < (पूर्णाङ्क)size && '0' <= data[i] && data[i] <= '9')
			f->content_length = म_से_द(data + i);
	} अन्यथा यदि (17 < size && strncasecmp(data, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		यावत् (i < (पूर्णाङ्क) size - 5 &&
				strncasecmp(data + i, "realm", 5))
			i++;
		यावत् (i < (पूर्णाङ्क) size - 1 && data[++i] != '"')
			/* */;
		i++;

		यदि (i < (पूर्णाङ्क) size) {
			माप_प्रकार end = i;

			यावत् (end < size && data[end] != '"')
				++end;

			यदि (end < size) {
				मुक्त(f->realm);
				f->realm = दो_स्मृति(end - i + 1);
				यदि (f->realm != रिक्त) {
					म_नकलन(f->realm, data + i, end - i);
					f->realm[end - i] = '\0';
				}
			}
		}
	} अन्यथा यदि (11 < size && strncasecmp(data, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_set_cookie(f->fetch_handle, &data[i]);
	}

	प्रत्यागम size;
#undef SKIP_ST
}

स्थिर पूर्णाङ्क fetch_curl_fdset(lwc_string *scheme, fd_set *read_set,
			    fd_set *write_set, fd_set *error_set)
{
	CURLMcode code;
	पूर्णाङ्क maxfd = -1;

	code = curl_multi_fdset(fetch_curl_multi,
				read_set,
				write_set,
				error_set,
				&maxfd);
	प्रतिज्ञा(code == CURLM_OK);

	प्रत्यागम maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_register(शून्य)
{
	CURLcode code;
	curl_version_info_data *data;
	पूर्णाङ्क i;
	lwc_string *scheme;
	ध्रुव संरचना fetcher_operation_table fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.setup = fetch_curl_setup,
		.start = fetch_curl_start,
		.पात = fetch_curl_abort,
		.मुक्त = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#यदि LIBCURL_VERSION_अंक >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, रिक्त, रिक्त);
	यदि (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} अन्यथा {
		curl_with_openssl = false;
	}
#endif

	NSLOG(netsurf, INFO, "curl_version %s", curl_version());

	code = curl_global_init(CURL_GLOBAL_ALL);
	यदि (code != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		प्रत्यागम NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	यदि (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		प्रत्यागम NSERROR_INIT_FAILED;
	}

#यदि LIBCURL_VERSION_अंक >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		पूर्णाङ्क maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#undef SETOPT
#define SETOPT(option, मूल्य) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, मूल्य);	\
	यदि (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		गच्छ curl_multi_setopt_failed;				\
	}

		SETOPT(CURLMOPT_MAXCONNECTS, (दीर्घ)maxconnects);
		SETOPT(CURLMOPT_MAX_TOTAL_CONNECTIONS, (दीर्घ)maxconnects);
		SETOPT(CURLMOPT_MAX_HOST_CONNECTIONS, (दीर्घ)nsoption_int(max_fetchers_per_host));
	}
#endif

	/* Create a curl easy handle with the options that are common to all
	 *  fetches.
	 */
	fetch_blank_curl = curl_easy_init();
	यदि (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		प्रत्यागम NSERROR_INIT_FAILED;
	}

#undef SETOPT
#define SETOPT(option, मूल्य) \
	code = curl_easy_setopt(fetch_blank_curl, option, मूल्य);	\
	यदि (code != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		गच्छ curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_buffer);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	यदि (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} अन्यथा {
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
	SETOPT(CURLOPT_निचला_SPEED_LIMIT, 1L);
	SETOPT(CURLOPT_निचला_SPEED_TIME, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (दीर्घ)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	यदि (nsoption_charp(ca_bundle) &&
	    म_भेद(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	यदि (nsoption_charp(ca_path) && म_भेद(nsoption_charp(ca_path), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_path));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_path));
	}

#यदि LIBCURL_VERSION_अंक < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	code = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, रिक्त);
	यदि (code != CURLE_OK) {
		curl_with_openssl = false;
	} अन्यथा {
		curl_with_openssl = true;
	}
#endif

	यदि (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#यदि LIBCURL_VERSION_अंक >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		code = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		यदि (code != CURLE_OK && code != CURLE_NOT_BUILT_IN)
			गच्छ curl_easy_setopt_failed;
#endif
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	data = curl_version_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	यदि (curl_fetch_ssl_hashmap == रिक्त) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		प्रत्यागम NSERROR_NOMEM;
	}

	प्रति (i = 0; data->protocols[i]; i++) {
		यदि (म_भेद(data->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} अन्यथा यदि (म_भेद(data->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} अन्यथा {
			/* Ignore non-http(s) protocols */
			अनुवर्त;
		}

		यदि (fetcher_add(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      data->protocols[i]);
		}
	}

	प्रत्यागम NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	प्रत्यागम NSERROR_INIT_FAILED;

#यदि LIBCURL_VERSION_अंक >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	प्रत्यागम NSERROR_INIT_FAILED;
#endif
}
