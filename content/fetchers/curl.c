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
#परिभाषय UPDATES_प्रत्येकम्_SECOND 2

/**
 * The ciphersuites the browser is prepared to use for TLS1.3
 */
#परिभाषय CIPHER_SUITES						\
	"TLS_AES_256_GCM_SHA384:"				\
	"TLS_CHACHA20_POLY1305_SHA256:"				\
	"TLS_AES_128_GCM_SHA256"

/**
 * The ciphersuites the browser is prepared to use for TLS<1.3
 */
#परिभाषय CIPHER_LIST						\
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
#परिभाषय CIPHER_LIST_LEGACY					\
	/* as above */						\
	CIPHER_LIST":"						\
	/* enable (non-PFS) RSA AES GCM suites */		\
	"RSA+AESGCM:"						\
	/* enable (non-PFS) RSA (mac=SHA1) AES CBC suites */	\
	"RSA+AES+SHA1"

/* Open SSL compatability for certificate handling */
#यदि_परिभाषितम् WITH_OPENSSL

#अन्तर्भाव <openssl/ssl.h>
#अन्तर्भाव <openssl/x509v3.h>

#अन्यथा /* WITH_OPENSSL */

प्रकारनिर्देश वर्ण X509;

स्थिर शून्य X509_free(X509 *cert)
{
	मुक्त(cert);
}

#यदि_अन्तः /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
स्थिर uint32_t
curl_fetch_ssl_कुञ्चिका_hash(शून्य *कुञ्चिका)
{
	nsurl *url = कुञ्चिका;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *पत्तनम् = nsurl_get_component(url, NSURL_पत्तनम्);
	uint32_t hash;

	यदि (पत्तनम् == रिक्त)
		पत्तनम् = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_मानम्(hostname) ^ lwc_string_hash_मानम्(पत्तनम्);

	lwc_string_unref(hostname);
	lwc_string_unref(पत्तनम्);

	प्रत्यागम hash;
}

/* We only compare the hostname and port */
स्थिर bool
curl_fetch_ssl_कुञ्चिका_eq(शून्य *key1, शून्य *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_पत्तनम्);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_पत्तनम्);
	bool iseq = false;

	यदि (port1 == रिक्त)
		port1 = lwc_string_ref(corestring_lwc_443);
	यदि (port2 == रिक्त)
		port2 = lwc_string_ref(corestring_lwc_443);

	यदि (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		गच्छ बहिर्गमनम्;

	iseq = false;
	यदि (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		गच्छ बहिर्गमनम्;

बहिर्गमनम्:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	प्रत्यागम iseq;
}

स्थिर शून्य *
curl_fetch_ssl_मानम्_alloc(शून्य *कुञ्चिका)
{
	संरचना cert_chain *बहिर्गमनम्;

	यदि (cert_chain_alloc(0, &बहिर्गमनम्) != NSERROR_OK) {
		प्रत्यागम रिक्त;
	}

	प्रत्यागम बहिर्गमनम्;
}

स्थिर शून्य
curl_fetch_ssl_मानम्_destroy(शून्य *मानम्)
{
	संरचना cert_chain *chain = मानम्;
	यदि (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

स्थिर hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.कुञ्चिका_clone = (hashmap_कुञ्चिका_clone_t)nsurl_ref,
	.कुञ्चिका_destroy = (hashmap_कुञ्चिका_destroy_t)nsurl_unref,
	.कुञ्चिका_eq = curl_fetch_ssl_कुञ्चिका_eq,
	.कुञ्चिका_hash = curl_fetch_ssl_कुञ्चिका_hash,
	.मानम्_alloc = curl_fetch_ssl_मानम्_alloc,
	.मानम्_destroy = curl_fetch_ssl_मानम्_destroy,
};

स्थिर hashmap_t *curl_fetch_ssl_hashmap = रिक्त;

/** SSL certificate info */
संरचना cert_info {
	X509 *cert;		/**< Pointer to certificate */
	दीर्घ err;		/**< OpenSSL error code */
};

#यदि LIBCURL_संस्करणम्_अङ्कः >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#परिभाषय NSCURLOPT_प्रगतिः_FUNCTION CURLOPT_XFERINFOFUNCTION
#परिभाषय NSCURLOPT_प्रगतिः_दत्त CURLOPT_XFERINFODATA
#परिभाषय NSCURL_प्रगतिः_T curl_off_t
#अन्यथा
#परिभाषय NSCURLOPT_प्रगतिः_FUNCTION CURLOPT_PROGRESSFUNCTION
#परिभाषय NSCURLOPT_प्रगतिः_दत्त CURLOPT_PROGRESSDATA
#परिभाषय NSCURL_प्रगतिः_T द्विगुण
#यदि_अन्तः

#यदि LIBCURL_संस्करणम्_अङ्कः >= 0x073800 /* 7.56.0 depricated curl_formadd */
#परिभाषय NSCURL_POSTDATA_T curl_mime
#परिभाषय NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#परिभाषय NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#अन्यथा
#परिभाषय NSCURL_POSTDATA_T संरचना curl_httppost
#परिभाषय NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#परिभाषय NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#यदि_अन्तः

/** Information for a single fetch. */
संरचना curl_fetch_info {
	संरचना fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool प्रेषितम्_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_शीर्षकाणि;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	संरचना curl_slist *शीर्षकाणि;	/**< List of request headers. */
	वर्ण *location;		/**< Response Location header, or 0. */
	अचिह्नित दीर्घ content_दीर्घता;	/**< Response Content-Length, or 0. */
	वर्ण *cookie_string;	/**< Cookie string for this fetch */
	वर्ण *realm;		/**< HTTP Auth Realm */
	संरचना fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	दीर्घ http_संहिता; /**< HTTP result code from cURL. */

	uint64_t चरमः_प्रगतिः_update;	/**< Time of last progress update */
	पूर्णाङ्क cert_गभीरता; /**< deepest certificate in use */
	संरचना cert_info cert_दत्त[MAX_CERT_गभीरता]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
संरचना cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	संरचना cache_handle *r_पूर्व; /**< Previous cached handle in ring. */
	संरचना cache_handle *r_अग्र; /**< Next cached handle in ring. */
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
स्थिर वर्ण fetch_error_बफर[CURL_ERROR_आकार];

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
	      lwc_string_दत्त(scheme));
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
	      lwc_string_दत्त(scheme));
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
			  ध्रुव संरचना fetch_multipart_दत्त *post_multipart)
{
	संरचना fetch_postdata *postdata;
	postdata = शून्यस्मृतिं_देहि(1, परिमाण(संरचना fetch_postdata));
	यदि (postdata != रिक्त) {

		यदि (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->दत्त.urlenc = strdup(post_urlenc);
			यदि (postdata->दत्त.urlenc == रिक्त) {
				मुक्त(postdata);
				postdata = रिक्त;
			}
		} अन्यथा यदि (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->दत्त.multipart = fetch_multipart_दत्त_clone(post_multipart);
			यदि (postdata->दत्त.multipart == रिक्त) {
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
			मुक्त(postdata->दत्त.urlenc);
			भङ्ग;
		पक्ष FETCH_POSTDATA_MULTIPART:
			fetch_multipart_दत्त_destroy(postdata->दत्त.multipart);
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
	fetch = स्मृतिं_देहि(परिमाण (*fetch));
	यदि (fetch == रिक्त)
		प्रत्यागम रिक्त;

	fetch->curl_handle = रिक्त;
	fetch->प्रेषितम्_ssl_chain = false;
	fetch->had_शीर्षकाणि = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->शीर्षकाणि = रिक्त;
	fetch->url = रिक्त;
	fetch->host = रिक्त;
	fetch->location = रिक्त;
	fetch->content_दीर्घता = 0;
	fetch->http_संहिता = 0;
	fetch->cookie_string = रिक्त;
	fetch->realm = रिक्त;
	fetch->चरमः_प्रगतिः_update = 0;
	fetch->postdata = रिक्त;
	fetch->curl_postdata = रिक्त;

	/* Clear certificate chain data */
	स्मृतिं_पूरय(fetch->cert_दत्त, 0, परिमाण(fetch->cert_दत्त));
	fetch->cert_गभीरता = -1;

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
fetch_curl_सज्जता(संरचना fetch *parent_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ध्रुव वर्ण *post_urlenc,
		 ध्रुव संरचना fetch_multipart_दत्त *post_multipart,
		 ध्रुव वर्ण **शीर्षकाणि)
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

#परिभाषय APPEND(list, मानम्) \
	slist = curl_slist_append(list, मानम्);		\
	यदि (slist == रिक्त)				\
		गच्छ failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->शीर्षकाणि, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->शीर्षकाणि, "Expect:");

	यदि ((nsoption_charp(accept_language) != रिक्त) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		वर्ण s[80];
		snprintf(s, परिमाण s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[परिमाण s - 1] = 0;
		APPEND(fetch->शीर्षकाणि, s);
	}

	यदि (nsoption_charp(accept_charset) != रिक्त &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		वर्ण s[80];
		snprintf(s, परिमाण s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[परिमाण s - 1] = 0;
		APPEND(fetch->शीर्षकाणि, s);
	}

	यदि (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->शीर्षकाणि, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	प्रति (i = 0; शीर्षकाणि[i] != रिक्त; i++) {
		APPEND(fetch->शीर्षकाणि, शीर्षकाणि[i]);
	}

	प्रत्यागम fetch;

#अपरिभाषय APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->शीर्षकाणि);
	मुक्त(fetch);
	प्रत्यागम रिक्त;
}


#यदि_परिभाषितम् WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
स्थिर संरचना cert_chain *
fetch_curl_get_cached_chain(संरचना curl_fetch_info *f)
{
	संरचना cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	यदि (chain == रिक्त) {
		chain = hashmap_निवेशय(curl_fetch_ssl_hashmap, f->url);
	}

	प्रत्यागम chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
स्थिर शून्य
fetch_curl_store_certs_अन्तर्_cache(संरचना curl_fetch_info *f)
{
	size_t गभीरता;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_गभीरता];
	संरचना cert_chain chain, *cached_chain;
	संरचना cert_info *certs;

	स्मृतिं_पूरय(&chain, 0, परिमाण(chain));

	certs = f->cert_दत्त;
	chain.गभीरता = f->cert_गभीरता + 1; /* 0 indexed certificate depth */

	प्रति (गभीरता = 0; गभीरता < chain.गभीरता; गभीरता++) {
		यदि (certs[गभीरता].cert == रिक्त) {
			/* This certificate is missing, skip it */
			chain.certs[गभीरता].err = SSL_CERT_ERR_CERT_MISSING;
			अनुवर्त;
		}

		/* error code (if any) */
		विकल्प (certs[गभीरता].err) {
		पक्ष X509_V_OK:
			chain.certs[गभीरता].err = SSL_CERT_ERR_OK;
			भङ्ग;

		पक्ष X509_V_ERR_UNABLE_गन्तव्यम्_GET_ISSUER_CERT:
			/* fallthrough */
		पक्ष X509_V_ERR_UNABLE_गन्तव्यम्_DECODE_ISSUER_लौकिकम्_कुञ्चिका:
			chain.certs[गभीरता].err = SSL_CERT_ERR_BAD_ISSUER;
			भङ्ग;

		पक्ष X509_V_ERR_UNABLE_गन्तव्यम्_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		पक्ष X509_V_ERR_UNABLE_गन्तव्यम्_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		पक्ष X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		पक्ष X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[गभीरता].err = SSL_CERT_ERR_BAD_SIG;
			भङ्ग;

		पक्ष X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		पक्ष X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[गभीरता].err = SSL_CERT_ERR_TOO_YOUNG;
			भङ्ग;

		पक्ष X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		पक्ष X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[गभीरता].err = SSL_CERT_ERR_TOO_OLD;
			भङ्ग;

		पक्ष X509_V_ERR_गभीरता_ZERO_SELF_SIGNED_CERT:
			chain.certs[गभीरता].err = SSL_CERT_ERR_SELF_SIGNED;
			भङ्ग;

		पक्ष X509_V_ERR_SELF_SIGNED_CERT_अन्तर्_CHAIN:
			chain.certs[गभीरता].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			भङ्ग;

		पक्ष X509_V_ERR_CERT_REVOKED:
			chain.certs[गभीरता].err = SSL_CERT_ERR_REVOKED;
			भङ्ग;

		पक्ष X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[गभीरता].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			भङ्ग;

		स्वभाव:
			chain.certs[गभीरता].err = SSL_CERT_ERR_UNKNOWN;
			भङ्ग;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[गभीरता].cert);
		BIO_get_mem_ptr(mem, &buf[गभीरता]);
		(शून्य) BIO_समुच्चयः_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[गभीरता].der = (uint8_t *)buf[गभीरता]->दत्त;
		chain.certs[गभीरता].der_दीर्घता = buf[गभीरता]->दीर्घता;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	यदि (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	प्रति (गभीरता = 0; गभीरता < chain.गभीरता; गभीरता++) {
		यदि (chain.certs[गभीरता].err == SSL_CERT_ERR_CERT_MISSING) {
			अनुवर्त;
		}
		यदि (buf[गभीरता] != रिक्त) {
			BUF_MEM_free(buf[गभीरता]);
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
	पूर्णाङ्क गभीरता;
	संरचना curl_fetch_info *fetch;

	गभीरता = X509_STORE_CTX_get_error_गभीरता(x509_ctx);
	fetch = X509_STORE_CTX_get_app_दत्त(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	यदि (गभीरता >= MAX_CERT_गभीरता) {
		X509_STORE_CTX_समुच्चयः_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		प्रत्यागम 0;
	}

	/* record the max depth */
	यदि (गभीरता > fetch->cert_गभीरता) {
		fetch->cert_गभीरता = गभीरता;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	यदि (!fetch->cert_दत्त[गभीरता].cert) {
		fetch->cert_दत्त[गभीरता].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_ऊर्ध्वम्_ref(fetch->cert_दत्त[गभीरता].cert);
		fetch->cert_दत्त[गभीरता].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	यदि (गभीरता > 0) {
		verify_ok = 1;
	} अन्यथा {
		/* search for deeper certificates in the chain with errors */
		प्रति (गभीरता = fetch->cert_गभीरता; गभीरता > 0; गभीरता--) {
			यदि (fetch->cert_दत्त[गभीरता].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_समुच्चयः_error(x509_ctx, fetch->cert_दत्त[गभीरता].err);
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
	X509_VERIFY_PARAM_समुच्चयः_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_दत्त(f->host),
					 lwc_string_दीर्घता(f->host));

	/* Store fetch struct in context for verify callback */
	यदि (ok) {
		ok = X509_STORE_CTX_समुच्चयः_app_दत्त(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	यदि (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_अन्तर्_cache(f);

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
	SSL_CTX_समुच्चयः_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_समुच्चयः_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	यदि (f->downgrade_tls) {
		/* Disable TLS 1.3 if the server can't cope with it */
#यदि_परिभाषितम् SSL_OP_NO_TLSv1_3
		options |= SSL_OP_NO_TLSv1_3;
#यदि_अन्तः
#यदि_परिभाषितम् SSL_बहुलकम्_SEND_FALLBACK_SCSV
		/* Ensure server rejects the connection if downgraded too far */
		SSL_CTX_समुच्चयः_बहुलकम्(sslctx, SSL_बहुलकम्_SEND_FALLBACK_SCSV);
#यदि_अन्तः
	}

	SSL_CTX_समुच्चयः_options(sslctx, options);

#यदि_परिभाषितम् SSL_OP_NO_TICKET
	SSL_CTX_clear_options(sslctx, SSL_OP_NO_TICKET);
#यदि_अन्तः

	प्रत्यागम CURLE_OK;
}


#यदि_अन्तः /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
स्थिर शून्य
fetch_curl_report_certs_upstream(संरचना curl_fetch_info *f)
{
	fetch_सन्देशः सन्देशः;
	संरचना cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	यदि (chain != रिक्त) {
		सन्देशः.type = FETCH_CERTS;
		सन्देशः.दत्त.chain = chain;

		fetch_send_callback(&सन्देशः, f->fetch_handle);
	}

	f->प्रेषितम्_ssl_chain = true;
}

#यदि LIBCURL_संस्करणम्_अङ्कः >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
संरचना curl_mime_ctx {
	वर्ण *बफर;
	curl_off_t आकार;
	curl_off_t position;
};

स्थिर size_t mime_दत्त_read_callback(वर्ण *बफर, size_t आकार, size_t nitems, शून्य *arg)
{
	संरचना curl_mime_ctx *mctx = (संरचना curl_mime_ctx *) arg;
	curl_off_t sz = mctx->आकार - mctx->position;

	nitems *= आकार;
	यदि(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	यदि(sz) {
		स्मृतिं_प्रतिलिख(बफर, mctx->बफर + mctx->position, sz);
	}
	mctx->position += sz;
	प्रत्यागम sz;
}

स्थिर पूर्णाङ्क mime_दत्त_seek_callback(शून्य *arg, curl_off_t विस्थापनम्, पूर्णाङ्क origin)
{
	संरचना curl_mime_ctx *mctx = (संरचना curl_mime_ctx *) arg;

	विकल्प(origin) {
	पक्ष SEEK_END:
		विस्थापनम् += mctx->आकार;
		भङ्ग;
	पक्ष SEEK_CUR:
		विस्थापनम् += mctx->position;
		भङ्ग;
	}

	यदि(विस्थापनम् < 0) {
		प्रत्यागम CURL_SEEKFUNC_FAIL;
	}
	mctx->position = विस्थापनम्;
	प्रत्यागम CURL_SEEKFUNC_OK;
}

स्थिर शून्य mime_दत्त_free_callback(शून्य *arg)
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
			    ध्रुव संरचना fetch_multipart_दत्त *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode संहिता = CURLE_OK;
	size_t मानम्_len;

	cmime = curl_mime_init(chandle);
	यदि (cmime == रिक्त) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		प्रत्यागम रिक्त;
	}

	/* iterate post data */
	प्रति (; multipart != रिक्त; multipart = multipart->अग्र) {
		part = curl_mime_addpart(cmime);
		यदि (part == रिक्त) {
			गच्छ convert_failed;
		}

		संहिता = curl_mime_name(part, multipart->name);
		यदि (संहिता != CURLE_OK) {
			गच्छ convert_failed;
		}

		मानम्_len = वाक्यदैर्घ्यम्(multipart->मानम्);

		यदि (multipart->file && मानम्_len==0) {
			/* file entries with no filename require special handling */
			संहिता=curl_mime_दत्त(part, multipart->मानम्, मानम्_len);
			यदि (संहिता != CURLE_OK) {
				गच्छ convert_failed;
			}

			संहिता = curl_mime_filename(part, "");
			यदि (संहिता != CURLE_OK) {
				गच्छ convert_failed;
			}

			संहिता = curl_mime_type(part, "application/octet-stream");
			यदि (संहिता != CURLE_OK) {
				गच्छ convert_failed;
			}

		} अन्यथा यदि(multipart->file) {
			/* file entry */
			nserror ret;
			वर्ण *leafname = रिक्त;
			वर्ण *mimetype = रिक्त;

			संहिता = curl_mime_filedata(part, multipart->rawfile);
			यदि (संहिता != CURLE_OK) {
				गच्छ convert_failed;
			}

			ret = guit->file->basename(multipart->मानम्, &leafname, रिक्त);
			यदि (ret != NSERROR_OK) {
				गच्छ convert_failed;
			}
			संहिता = curl_mime_filename(part, leafname);
			मुक्त(leafname);
			यदि (संहिता != CURLE_OK) {
				गच्छ convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->मानम्);
			यदि (mimetype == रिक्त) {
				mimetype=strdup("text/plain");
			}
			यदि (mimetype == रिक्त) {
				गच्छ convert_failed;
			}
			संहिता = curl_mime_type(part, mimetype);
			मुक्त(mimetype);
			यदि (संहिता != CURLE_OK) {
				गच्छ convert_failed;
			}

		} अन्यथा {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			संरचना curl_mime_ctx *cb_ctx;
			cb_ctx = स्मृतिं_देहि(परिमाण(संरचना curl_mime_ctx));
			यदि (cb_ctx == रिक्त) {
				गच्छ convert_failed;
			}
			cb_ctx->बफर = multipart->मानम्;
			cb_ctx->आकार = मानम्_len;
			cb_ctx->position = 0;
			संहिता = curl_mime_दत्त_cb(part,
						 मानम्_len,
						 mime_दत्त_read_callback,
						 mime_दत्त_seek_callback,
						 mime_दत्त_free_callback,
						 cb_ctx);
			यदि (संहिता != CURLE_OK) {
				मुक्त(cb_ctx);
				गच्छ convert_failed;
			}
		}
	}

	प्रत्यागम cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", संहिता);
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
			    ध्रुव संरचना fetch_multipart_दत्त *control)
{
	संरचना curl_httppost *post = रिक्त, *चरमः = रिक्त;
	CURLFORMcode संहिता;
	nserror ret;

	प्रति (; control; control = control->अग्र) {
		यदि (control->file) {
			वर्ण *leafname = रिक्त;
			ret = guit->file->basename(control->मानम्, &leafname, रिक्त);
			यदि (ret != NSERROR_OK) {
				अनुवर्त;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			यदि (control->मानम्[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				स्थिर वर्ण buf;

				संहिता = curl_formadd(&post, &चरमः,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_बफर, control->मानम्,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_अन्तः);
				यदि (संहिता != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", संहिता,
					      control->name);
			} अन्यथा {
				वर्ण *mimetype = guit->fetch->mimetype(control->मानम्);
				संहिता = curl_formadd(&post, &चरमः,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_अन्तः);
				यदि (संहिता != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      संहिता,
					      control->name,
					      control->मानम्);
				मुक्त(mimetype);
			}
			मुक्त(leafname);
		} अन्यथा {
			संहिता = curl_formadd(&post, &चरमः,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->मानम्,
					    CURLFORM_अन्तः);
			यदि (संहिता != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", संहिता,
				      control->name, control->मानम्);
		}
	}

	प्रत्यागम post;
}

#यदि_अन्तः  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
स्थिर CURLcode fetch_curl_समुच्चयः_postdata(संरचना curl_fetch_info *f)
{
	CURLcode संहिता = CURLE_OK;

#अपरिभाषय SETOPT
#परिभाषय SETOPT(option, मानम्) { \
	संहिता = curl_easy_setopt(f->curl_handle, option, मानम्);	\
	यदि (संहिता != CURLE_OK)					\
		प्रत्यागम संहिता;					\
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
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->दत्त.urlenc);
		भङ्ग;

	पक्ष FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, रिक्त);
		SETOPT(CURLOPT_HTTPGET, 0L);
		यदि (f->curl_postdata == रिक्त) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->दत्त.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		भङ्ग;
	}
	प्रत्यागम संहिता;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
स्थिर CURLcode fetch_curl_समुच्चयः_options(संरचना curl_fetch_info *f)
{
	CURLcode संहिता;
	ध्रुव वर्ण *auth;

#अपरिभाषय SETOPT
#परिभाषय SETOPT(option, मानम्) { \
	संहिता = curl_easy_setopt(f->curl_handle, option, मानम्);	\
	यदि (संहिता != CURLE_OK)					\
		प्रत्यागम संहिता;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_वैयक्तिकम्, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_प्रगतिः_दत्त, f);
	SETOPT(CURLOPT_HTTPHEADER, f->शीर्षकाणि);
	संहिता = fetch_curl_समुच्चयः_postdata(f);
	यदि (संहिता != CURLE_OK) {
		प्रत्यागम संहिता;
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
	    (वाक्ये_तुलय(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (दीर्घ) nsoption_int(http_proxy_पत्तनम्));

#यदि LIBCURL_संस्करणम्_अङ्कः >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#यदि_अन्तः

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
			SETOPT(CURLOPT_SSL_CTX_दत्त, रिक्त);
		}
	} अन्यथा {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#यदि_परिभाषितम् WITH_OPENSSL
		यदि (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_दत्त, f);
		}
#यदि_अन्तः
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
	CURLcode संहिता;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	संहिता = fetch_curl_समुच्चयः_options(fetch);
	यदि (संहिता != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		प्रत्यागम false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_सङ्कलय_handle(fetch_curl_multi, fetch->curl_handle);
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
#यदि LIBCURL_संस्करणम्_अङ्कः >= 0x071e00
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
			curl_handle_ring = h->r_अग्र;
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
	h = (संरचना cache_handle*)स्मृतिं_देहि(परिमाण(संरचना cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_निवेशय(curl_handle_ring, h);
#यदि_अन्तः
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
			f->abort = true;
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
	यदि (f->शीर्षकाणि) {
		curl_slist_free_all(f->शीर्षकाणि);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	प्रति (i = 0; i < MAX_CERT_गभीरता; i++) {
		यदि (f->cert_दत्त[i].cert != रिक्त) {
			X509_free(f->cert_दत्त[i].cert);
		}
	}

	मुक्त(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
स्थिर bool fetch_curl_process_शीर्षकाणि(संरचना curl_fetch_info *f)
{
	दीर्घ http_संहिता;
	CURLcode संहिता;
	fetch_सन्देशः सन्देशः;

	f->had_शीर्षकाणि = true;

	यदि (!f->http_संहिता) {
		संहिता = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_संहिता,
					 &f->http_संहिता);
		प्रतिज्ञा(संहिता == CURLE_OK);
		fetch_समुच्चयः_http_संहिता(f->fetch_handle, f->http_संहिता);
	}
	http_संहिता = f->http_संहिता;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_संहिता);

	यदि ((http_संहिता == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		सन्देशः.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&सन्देशः, f->fetch_handle);
		प्रत्यागम true;
	}

	/* handle HTTP redirects (3xx response codes) */
	यदि (HTTP_RESPONSE_IS_3XX(http_संहिता) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		सन्देशः.type = FETCH_REDIRECT;
		सन्देशः.दत्त.redirect = f->location;
		fetch_send_callback(&सन्देशः, f->fetch_handle);
		प्रत्यागम true;
	}

	/* handle HTTP 401 (Authentication errors) */
	यदि (http_संहिता == HTTP_RESPONSE_UNAUTHORIZED) {
		सन्देशः.type = FETCH_AUTH;
		सन्देशः.दत्त.auth.realm = f->realm;
		fetch_send_callback(&सन्देशः, f->fetch_handle);
		प्रत्यागम true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	यदि (f->only_2xx &&
	    वाक्ये_तुलय(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_संहिता) == false)) {
		सन्देशः.type = FETCH_ERROR;
		सन्देशः.दत्त.error = messages_get("Not2xx");
		fetch_send_callback(&सन्देशः, f->fetch_handle);
		प्रत्यागम true;
	}

	यदि (f->abort)
		प्रत्यागम true;

	प्रत्यागम false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
स्थिर शून्य fetch_curl_समाप्तम्(CURL *curl_handle, CURLcode परिणाम)
{
	bool परिसमाप्तम् = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	संरचना curl_fetch_info *f;
	वर्ण **_hideous_hack = (वर्ण **) (शून्य *) &f;
	CURLcode संहिता;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	संहिता = curl_easy_getinfo(curl_handle, CURLINFO_वैयक्तिकम्, _hideous_hack);
	प्रतिज्ञा(संहिता == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	यदि ((abort_fetch == false) &&
	    (परिणाम == CURLE_OK ||
	     ((परिणाम == CURLE_WRITE_ERROR) && (f->stopped == false)))) {
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
		    (!f->had_शीर्षकाणि &&	fetch_curl_process_शीर्षकाणि(f))) {
			; /* redirect with no body or similar */
		} अन्यथा {
			परिसमाप्तम् = true;
		}
	} अन्यथा यदि (परिणाम == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		यदि (!f->had_शीर्षकाणि && fetch_curl_process_शीर्षकाणि(f))
			; /* redirect with partial body, or similar */
		अन्यथा {
			परिसमाप्तम् = true;
		}
	} अन्यथा यदि (परिणाम == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} अन्यथा यदि (परिणाम == CURLE_SSL_PEER_CERTIFICATE ||
		   परिणाम == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} अन्यथा {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", परिणाम);
		error = true;
	}

	fetch_curl_stop(f);

	यदि (f->प्रेषितम्_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	यदि (abort_fetch) {
		; /* fetch was aborted: no callback */
	} अन्यथा यदि (परिसमाप्तम्) {
		fetch_सन्देशः सन्देशः;
		सन्देशः.type = FETCH_परिसमाप्तम्;
		fetch_send_callback(&सन्देशः, f->fetch_handle);
	} अन्यथा यदि (cert) {
		/* user needs to validate certificate with issue */
		fetch_सन्देशः सन्देशः;
		सन्देशः.type = FETCH_CERT_ERR;
		fetch_send_callback(&सन्देशः, f->fetch_handle);
	} अन्यथा यदि (error) {
		fetch_सन्देशः सन्देशः;
		विकल्प (परिणाम) {
		पक्ष CURLE_SSL_CONNECT_ERROR:
			सन्देशः.type = FETCH_SSL_ERR;
			भङ्ग;

		पक्ष CURLE_OPERATION_TIMEDOUT:
			सन्देशः.type = FETCH_TIMEDOUT;
			सन्देशः.दत्त.error = curl_easy_strerror(परिणाम);
			भङ्ग;

		स्वभाव:
			सन्देशः.type = FETCH_ERROR;
			सन्देशः.दत्त.error = curl_easy_strerror(परिणाम);
		}

		fetch_send_callback(&सन्देशः, f->fetch_handle);
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
	CURLMsg *curl_सन्देशः;

	यदि (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_समुच्चयः, write_fd_समुच्चयः, exc_fd_समुच्चयः;
		पूर्णाङ्क max_fd = -1;
		पूर्णाङ्क i;

		FD_ZERO(&read_fd_समुच्चयः);
		FD_ZERO(&write_fd_समुच्चयः);
		FD_ZERO(&exc_fd_समुच्चयः);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_समुच्चयः, &write_fd_समुच्चयः,
				&exc_fd_समुच्चयः, &max_fd);
		प्रतिज्ञा(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		प्रति (i = 0; i <= max_fd; i++) {
			bool पठ = false;
			bool लिख = false;
			bool error = false;

			यदि (FD_ISSET(i, &read_fd_समुच्चयः)) {
				पठ = true;
			}
			यदि (FD_ISSET(i, &write_fd_समुच्चयः)) {
				लिख = true;
			}
			यदि (FD_ISSET(i, &exc_fd_समुच्चयः)) {
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
	curl_सन्देशः = curl_multi_info_read(fetch_curl_multi, &queue);
	यावत् (curl_सन्देशः) {
		विकल्प (curl_सन्देशः->सन्देशः) {
			पक्ष CURLMSG_समाप्तम्:
				fetch_curl_समाप्तम्(curl_सन्देशः->easy_handle,
						curl_सन्देशः->दत्त.परिणाम);
				भङ्ग;
			स्वभाव:
				भङ्ग;
		}
		curl_सन्देशः = curl_multi_info_read(fetch_curl_multi, &queue);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
स्थिर पूर्णाङ्क
fetch_curl_प्रगतिः(शून्य *clientp,
		    NSCURL_प्रगतिः_T dltotal,
		    NSCURL_प्रगतिः_T dlnow,
		    NSCURL_प्रगतिः_T ultotal,
		    NSCURL_प्रगतिः_T ulnow)
{
	स्थिर वर्ण fetch_प्रगतिः_बफर[256]; /**< Progress buffer for cURL */
	संरचना curl_fetch_info *f = (संरचना curl_fetch_info *) clientp;
	uint64_t time_now_ms;
	fetch_सन्देशः सन्देशः;

	यदि (f->abort) {
		प्रत्यागम 0;
        }

	सन्देशः.type = FETCH_प्रगतिः;
	सन्देशः.दत्त.प्रगतिः = fetch_प्रगतिः_बफर;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&time_now_ms);
#परिभाषय UPDATE_DELAY_MS (1000 / UPDATES_प्रत्येकम्_SECOND)
	यदि (time_now_ms - f->चरमः_प्रगतिः_update < UPDATE_DELAY_MS) {
		प्रत्यागम 0;
        }
#अपरिभाषय UPDATE_DELAY_MS
	f->चरमः_प्रगतिः_update = time_now_ms;

	यदि (dltotal > 0) {
		snprintf(fetch_प्रगतिः_बफर, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&सन्देशः, f->fetch_handle);
	} अन्यथा {
		snprintf(fetch_प्रगतिः_बफर, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&सन्देशः, f->fetch_handle);
	}

	प्रत्यागम 0;
}


/**
 * Format curl debug for nslog
 */
स्थिर पूर्णाङ्क
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 वर्ण *दत्त,
		 size_t आकार,
		 शून्य *userptr)
{
	स्थिर ध्रुव वर्ण s_infotype[CURLINFO_अन्तः][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	विकल्प(type) {
	पक्ष CURLINFO_TEXT:
	पक्ष CURLINFO_शीर्षकम्_बहिर्गमनम्:
	पक्ष CURLINFO_शीर्षकम्_अन्तर्:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (पूर्णाङ्क)आकार - 1, दत्त);
		भङ्ग;

	स्वभाव:
		भङ्ग;
	}
	प्रत्यागम 0;
}


स्थिर curl_socket_t fetch_curl_socket_open(शून्य *clientp,
		curlsocktype purpose, संरचना curl_sockaddr *स्थानाङ्कः)
{
	(शून्य) clientp;
	(शून्य) purpose;

	प्रत्यागम (curl_socket_t) guit->fetch->socket_open(
			स्थानाङ्कः->family, स्थानाङ्कः->socktype,
			स्थानाङ्कः->protocol);
}

स्थिर पूर्णाङ्क fetch_curl_socket_close(शून्य *clientp, curl_socket_t item)
{
	(शून्य) clientp;

	प्रत्यागम guit->fetch->socket_close((पूर्णाङ्क) item);
}

/**
 * Callback function for cURL.
 */
स्थिर size_t fetch_curl_दत्त(वर्ण *दत्त, size_t आकार, size_t nmemb, शून्य *_f)
{
	संरचना curl_fetch_info *f = _f;
	CURLcode संहिता;
	fetch_सन्देशः सन्देशः;

	/* ensure we only have to get this information once */
	यदि (!f->http_संहिता) {
		संहिता = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_संहिता,
					 &f->http_संहिता);
		प्रतिज्ञा(संहिता == CURLE_OK);
		fetch_समुच्चयः_http_संहिता(f->fetch_handle, f->http_संहिता);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	यदि (f->http_संहिता == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_संहिता = 0;
		प्रत्यागम आकार * nmemb;
	}

	यदि (f->abort || (!f->had_शीर्षकाणि && fetch_curl_process_शीर्षकाणि(f))) {
		f->stopped = true;
		प्रत्यागम 0;
	}

	/* send data to the caller */
	सन्देशः.type = FETCH_दत्त;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *) दत्त;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = आकार * nmemb;
	fetch_send_callback(&सन्देशः, f->fetch_handle);

	यदि (f->abort) {
		f->stopped = true;
		प्रत्यागम 0;
	}

	प्रत्यागम आकार * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
स्थिर size_t
fetch_curl_शीर्षकम्(वर्ण *दत्त, size_t आकार, size_t nmemb, शून्य *_f)
{
	संरचना curl_fetch_info *f = _f;
	पूर्णाङ्क i;
	fetch_सन्देशः सन्देशः;
	आकार *= nmemb;

	यदि (f->abort) {
		f->stopped = true;
		प्रत्यागम 0;
	}

	यदि (f->प्रेषितम्_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	सन्देशः.type = FETCH_शीर्षकम्;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *) दत्त;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = आकार;
	fetch_send_callback(&सन्देशः, f->fetch_handle);

#परिभाषय SKIP_ST(o) प्रति (i = (o); i < (पूर्णाङ्क) आकार && (दत्त[i] == ' ' || दत्त[i] == '\t'); i++)

	यदि (12 < आकार && strncasecmp(दत्त, "Location:", 9) == 0) {
		/* extract Location header */
		मुक्त(f->location);
		f->location = स्मृतिं_देहि(आकार);
		यदि (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			प्रत्यागम आकार;
		}
		SKIP_ST(9);
		strncpy(f->location, दत्त + i, आकार - i);
		f->location[आकार - i] = '\0';
		प्रति (i = आकार - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} अन्यथा यदि (15 < आकार && strncasecmp(दत्त, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		यदि (i < (पूर्णाङ्क)आकार && '0' <= दत्त[i] && दत्त[i] <= '9')
			f->content_दीर्घता = atol(दत्त + i);
	} अन्यथा यदि (17 < आकार && strncasecmp(दत्त, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		यावत् (i < (पूर्णाङ्क) आकार - 5 &&
				strncasecmp(दत्त + i, "realm", 5))
			i++;
		यावत् (i < (पूर्णाङ्क) आकार - 1 && दत्त[++i] != '"')
			/* */;
		i++;

		यदि (i < (पूर्णाङ्क) आकार) {
			size_t अन्तः = i;

			यावत् (अन्तः < आकार && दत्त[अन्तः] != '"')
				++अन्तः;

			यदि (अन्तः < आकार) {
				मुक्त(f->realm);
				f->realm = स्मृतिं_देहि(अन्तः - i + 1);
				यदि (f->realm != रिक्त) {
					strncpy(f->realm, दत्त + i, अन्तः - i);
					f->realm[अन्तः - i] = '\0';
				}
			}
		}
	} अन्यथा यदि (11 < आकार && strncasecmp(दत्त, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_समुच्चयः_cookie(f->fetch_handle, &दत्त[i]);
	}

	प्रत्यागम आकार;
#अपरिभाषय SKIP_ST
}

स्थिर पूर्णाङ्क fetch_curl_fdset(lwc_string *scheme, fd_set *read_समुच्चयः,
			    fd_set *write_समुच्चयः, fd_set *error_समुच्चयः)
{
	CURLMcode संहिता;
	पूर्णाङ्क maxfd = -1;

	संहिता = curl_multi_fdset(fetch_curl_multi,
				read_समुच्चयः,
				write_समुच्चयः,
				error_समुच्चयः,
				&maxfd);
	प्रतिज्ञा(संहिता == CURLM_OK);

	प्रत्यागम maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_पञ्जिकाकोशः(शून्य)
{
	CURLcode संहिता;
	curl_संस्करणम्_info_दत्त *दत्त;
	पूर्णाङ्क i;
	lwc_string *scheme;
	ध्रुव संरचना fetcher_operation_सारणी fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.सज्जता = fetch_curl_सज्जता,
		.start = fetch_curl_start,
		.abort = fetch_curl_abort,
		.मुक्त = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#यदि LIBCURL_संस्करणम्_अङ्कः >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, रिक्त, रिक्त);
	यदि (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} अन्यथा {
		curl_with_openssl = false;
	}
#यदि_अन्तः

	NSLOG(netsurf, INFO, "curl_version %s", curl_संस्करणम्());

	संहिता = curl_global_init(CURL_GLOBAL_ALL);
	यदि (संहिता != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		प्रत्यागम NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	यदि (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		प्रत्यागम NSERROR_INIT_FAILED;
	}

#यदि LIBCURL_संस्करणम्_अङ्कः >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		पूर्णाङ्क maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#अपरिभाषय SETOPT
#परिभाषय SETOPT(option, मानम्) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, मानम्);	\
	यदि (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		गच्छ curl_multi_setopt_failed;				\
	}

		SETOPT(CURLMOPT_MAXCONNECTS, (दीर्घ)maxconnects);
		SETOPT(CURLMOPT_MAX_योग_CONNECTIONS, (दीर्घ)maxconnects);
		SETOPT(CURLMOPT_MAX_HOST_CONNECTIONS, (दीर्घ)nsoption_int(max_fetchers_प्रत्येकम्_host));
	}
#यदि_अन्तः

	/* Create a curl easy handle with the options that are common to all
	 *  fetches.
	 */
	fetch_blank_curl = curl_easy_init();
	यदि (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		प्रत्यागम NSERROR_INIT_FAILED;
	}

#अपरिभाषय SETOPT
#परिभाषय SETOPT(option, मानम्) \
	संहिता = curl_easy_setopt(fetch_blank_curl, option, मानम्);	\
	यदि (संहिता != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		गच्छ curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_बफर);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	यदि (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} अन्यथा {
		SETOPT(CURLOPT_VERBOSE, 1);
	}

	/* Currently we explode if curl uses HTTP2, so force 1.1. */
	SETOPT(CURLOPT_HTTP_संस्करणम्, CURL_HTTP_संस्करणम्_1_1);

	SETOPT(CURLOPT_WRITEFUNCTION, fetch_curl_दत्त);
	SETOPT(CURLOPT_HEADERFUNCTION, fetch_curl_शीर्षकम्);
	SETOPT(NSCURLOPT_प्रगतिः_FUNCTION, fetch_curl_प्रगतिः);
	SETOPT(CURLOPT_NOPROGRESS, 0L);
	SETOPT(CURLOPT_USERAGENT, user_agent_string());
	SETOPT(CURLOPT_ENCODING, "gzip");
	SETOPT(CURLOPT_नीचः_जवः_सीमा, 1L);
	SETOPT(CURLOPT_नीचः_जवः_TIME, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (दीर्घ)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	यदि (nsoption_charp(ca_bundle) &&
	    वाक्यं_तुलय(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	यदि (nsoption_charp(ca_मार्गः) && वाक्यं_तुलय(nsoption_charp(ca_मार्गः), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_मार्गः));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_मार्गः));
	}

#यदि LIBCURL_संस्करणम्_अङ्कः < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	संहिता = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, रिक्त);
	यदि (संहिता != CURLE_OK) {
		curl_with_openssl = false;
	} अन्यथा {
		curl_with_openssl = true;
	}
#यदि_अन्तः

	यदि (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#यदि LIBCURL_संस्करणम्_अङ्कः >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		संहिता = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		यदि (संहिता != CURLE_OK && संहिता != CURLE_NOT_BUILT_अन्तर्)
			गच्छ curl_easy_setopt_failed;
#यदि_अन्तः
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	दत्त = curl_संस्करणम्_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	यदि (curl_fetch_ssl_hashmap == रिक्त) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		प्रत्यागम NSERROR_NOMEM;
	}

	प्रति (i = 0; दत्त->protocols[i]; i++) {
		यदि (वाक्यं_तुलय(दत्त->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} अन्यथा यदि (वाक्यं_तुलय(दत्त->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} अन्यथा {
			/* Ignore non-http(s) protocols */
			अनुवर्त;
		}

		यदि (fetcher_सङ्कलय(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      दत्त->protocols[i]);
		}
	}

	प्रत्यागम NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	प्रत्यागम NSERROR_INIT_FAILED;

#यदि LIBCURL_संस्करणम्_अङ्कः >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	प्रत्यागम NSERROR_INIT_FAILED;
#यदि_अन्तः
}
