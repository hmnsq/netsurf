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
#समावेश "utils/inet.h"

#समावेश <assert.h>
#समावेश <errno.h>
#समावेश <inttypes.h>
#समावेश <stdbool.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <time.h>
#समावेश <sys/stat.h>

#समावेश <libwapcaplet/libwapcaplet.h>
#समावेश <nsutils/time.h>

#समावेश "utils/corestrings.h"
#समावेश "utils/hashmap.h"
#समावेश "utils/nsoption.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "utils/utils.h"
#समावेश "utils/ring.h"
#समावेश "utils/useragent.h"
#समावेश "utils/file.h"
#समावेश "utils/string.h"
#समावेश "netsurf/fetch.h"
#समावेश "netsurf/misc.h"
#समावेश "desktop/gui_internal.h"

#समावेश "content/fetch.h"
#समावेश "content/fetchers.h"
#समावेश "content/fetchers/curl.h"
#समावेश "content/urldb.h"

/**
 * maximum number of progress notifications per second
 */
#घोषणा UPDATES_PER_SECOND 2

/**
 * The ciphersuites the browser is prepared to use for TLS1.3
 */
#घोषणा CIPHER_SUITES						\
	"TLS_AES_256_GCM_SHA384:"				\
	"TLS_CHACHA20_POLY1305_SHA256:"				\
	"TLS_AES_128_GCM_SHA256"

/**
 * The ciphersuites the browser is prepared to use for TLS<1.3
 */
#घोषणा CIPHER_LIST						\
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
#घोषणा CIPHER_LIST_LEGACY					\
	/* as above */						\
	CIPHER_LIST":"						\
	/* enable (non-PFS) RSA AES GCM suites */		\
	"RSA+AESGCM:"						\
	/* enable (non-PFS) RSA (mac=SHA1) AES CBC suites */	\
	"RSA+AES+SHA1"

/* Open SSL compatability for certificate handling */
#अगर_घोषित WITH_OPENSSL

#समावेश <openssl/ssl.h>
#समावेश <openssl/x509v3.h>

#अन्यथा /* WITH_OPENSSL */

प्रकार अक्षर X509;

अटल व्योम X509_free(X509 *cert)
{
	मुक्त(cert);
}

#पूर्ण_अगर /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
अटल uint32_t
curl_fetch_ssl_कुंजी_hash(व्योम *कुंजी)
{
	nsurl *url = कुंजी;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *port = nsurl_get_component(url, NSURL_PORT);
	uint32_t hash;

	अगर (port == शून्य)
		port = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_मूल्य(hostname) ^ lwc_string_hash_मूल्य(port);

	lwc_string_unref(hostname);
	lwc_string_unref(port);

	वापस hash;
}

/* We only compare the hostname and port */
अटल bool
curl_fetch_ssl_कुंजी_eq(व्योम *key1, व्योम *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_PORT);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_PORT);
	bool iseq = false;

	अगर (port1 == शून्य)
		port1 = lwc_string_ref(corestring_lwc_443);
	अगर (port2 == शून्य)
		port2 = lwc_string_ref(corestring_lwc_443);

	अगर (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		जाओ out;

	iseq = false;
	अगर (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		जाओ out;

out:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	वापस iseq;
}

अटल व्योम *
curl_fetch_ssl_मूल्य_alloc(व्योम *कुंजी)
{
	काष्ठा cert_chain *out;

	अगर (cert_chain_alloc(0, &out) != NSERROR_OK) {
		वापस शून्य;
	}

	वापस out;
}

अटल व्योम
curl_fetch_ssl_मूल्य_destroy(व्योम *मूल्य)
{
	काष्ठा cert_chain *chain = मूल्य;
	अगर (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

अटल hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.कुंजी_clone = (hashmap_कुंजी_clone_t)nsurl_ref,
	.कुंजी_destroy = (hashmap_कुंजी_destroy_t)nsurl_unref,
	.कुंजी_eq = curl_fetch_ssl_कुंजी_eq,
	.कुंजी_hash = curl_fetch_ssl_कुंजी_hash,
	.मूल्य_alloc = curl_fetch_ssl_मूल्य_alloc,
	.मूल्य_destroy = curl_fetch_ssl_मूल्य_destroy,
};

अटल hashmap_t *curl_fetch_ssl_hashmap = शून्य;

/** SSL certificate info */
काष्ठा cert_info {
	X509 *cert;		/**< Pointer to certificate */
	दीर्घ err;		/**< OpenSSL error code */
};

#अगर LIBCURL_VERSION_अंक >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#घोषणा NSCURLOPT_PROGRESS_FUNCTION CURLOPT_XFERINFOFUNCTION
#घोषणा NSCURLOPT_PROGRESS_तथ्य CURLOPT_XFERINFODATA
#घोषणा NSCURL_PROGRESS_T curl_off_t
#अन्यथा
#घोषणा NSCURLOPT_PROGRESS_FUNCTION CURLOPT_PROGRESSFUNCTION
#घोषणा NSCURLOPT_PROGRESS_तथ्य CURLOPT_PROGRESSDATA
#घोषणा NSCURL_PROGRESS_T द्विगुन
#पूर्ण_अगर

#अगर LIBCURL_VERSION_अंक >= 0x073800 /* 7.56.0 depricated curl_formadd */
#घोषणा NSCURL_POSTDATA_T curl_mime
#घोषणा NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#घोषणा NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#अन्यथा
#घोषणा NSCURL_POSTDATA_T काष्ठा curl_httppost
#घोषणा NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#घोषणा NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#पूर्ण_अगर

/** Information for a single fetch. */
काष्ठा curl_fetch_info {
	काष्ठा fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool sent_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_headers;	/**< Headers have been processed. */
	bool पात;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	काष्ठा curl_slist *headers;	/**< List of request headers. */
	अक्षर *location;		/**< Response Location header, or 0. */
	अचिन्हित दीर्घ content_लंबाई;	/**< Response Content-Length, or 0. */
	अक्षर *cookie_string;	/**< Cookie string for this fetch */
	अक्षर *realm;		/**< HTTP Auth Realm */
	काष्ठा fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	दीर्घ http_code; /**< HTTP result code from cURL. */

	uint64_t last_progress_update;	/**< Time of last progress update */
	पूर्णांक cert_depth; /**< deepest certificate in use */
	काष्ठा cert_info cert_तथ्य[MAX_CERT_DEPTH]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
काष्ठा cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	काष्ठा cache_handle *r_पूर्व; /**< Previous cached handle in ring. */
	काष्ठा cache_handle *r_अग्र; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
अटल CURL *fetch_blank_curl;

/** Ring of cached handles */
अटल काष्ठा cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
अटल पूर्णांक curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
अटल bool curl_with_openssl;

/** Error buffer for cURL. */
अटल अक्षर fetch_error_बफर[CURL_ERROR_आकार];

/** Proxy authentication details. */
अटल अक्षर fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
अटल bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
अटल bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_तथ्य(scheme));
	curl_fetchers_registered++;
	वापस true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
अटल व्योम fetch_curl_finalise(lwc_string *scheme)
{
	काष्ठा cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_तथ्य(scheme));
	अगर (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		अगर (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = शून्य;
	}

	/* Free anything remaining in the cached curl handle ring */
	जबतक (curl_handle_ring != शून्य) {
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
अटल bool fetch_curl_can_fetch(स्थिर nsurl *url)
{
	वापस nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
अटल काष्ठा fetch_postdata *
fetch_curl_alloc_postdata(स्थिर अक्षर *post_urlenc,
			  स्थिर काष्ठा fetch_multipart_तथ्य *post_multipart)
{
	काष्ठा fetch_postdata *postdata;
	postdata = सुस्मृति(1, माप(काष्ठा fetch_postdata));
	अगर (postdata != शून्य) {

		अगर (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->तथ्य.urlenc = strdup(post_urlenc);
			अगर (postdata->तथ्य.urlenc == शून्य) {
				मुक्त(postdata);
				postdata = शून्य;
			}
		} अन्यथा अगर (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->तथ्य.multipart = fetch_multipart_तथ्य_clone(post_multipart);
			अगर (postdata->तथ्य.multipart == शून्य) {
				मुक्त(postdata);
				postdata = शून्य;
			}
		} अन्यथा {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	वापस postdata;
}

/**
 * free postdata
 */
अटल व्योम fetch_curl_free_postdata(काष्ठा fetch_postdata *postdata)
{
	अगर (postdata != शून्य) {
		चयन (postdata->type) {
		हाल FETCH_POSTDATA_NONE:
			अवरोध;
		हाल FETCH_POSTDATA_URLENC:
			मुक्त(postdata->तथ्य.urlenc);
			अवरोध;
		हाल FETCH_POSTDATA_MULTIPART:
			fetch_multipart_तथ्य_destroy(postdata->तथ्य.multipart);
			अवरोध;
		}

		मुक्त(postdata);
	}
}

/**
 *construct a new fetch structure
 */
अटल काष्ठा curl_fetch_info *fetch_alloc(व्योम)
{
	काष्ठा curl_fetch_info *fetch;
	fetch = दो_स्मृति(माप (*fetch));
	अगर (fetch == शून्य)
		वापस शून्य;

	fetch->curl_handle = शून्य;
	fetch->sent_ssl_chain = false;
	fetch->had_headers = false;
	fetch->पात = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->headers = शून्य;
	fetch->url = शून्य;
	fetch->host = शून्य;
	fetch->location = शून्य;
	fetch->content_लंबाई = 0;
	fetch->http_code = 0;
	fetch->cookie_string = शून्य;
	fetch->realm = शून्य;
	fetch->last_progress_update = 0;
	fetch->postdata = शून्य;
	fetch->curl_postdata = शून्य;

	/* Clear certificate chain data */
	स_रखो(fetch->cert_तथ्य, 0, माप(fetch->cert_तथ्य));
	fetch->cert_depth = -1;

	वापस fetch;
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
अटल व्योम *
fetch_curl_setup(काष्ठा fetch *parent_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 स्थिर अक्षर *post_urlenc,
		 स्थिर काष्ठा fetch_multipart_तथ्य *post_multipart,
		 स्थिर अक्षर **headers)
{
	काष्ठा curl_fetch_info *fetch;
	काष्ठा curl_slist *slist;
	पूर्णांक i;

	fetch = fetch_alloc();
	अगर (fetch == शून्य)
		वापस शून्य;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = parent_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	अगर (fetch->host == शून्य) {
		जाओ failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	अगर (fetch->postdata == शून्य) {
		जाओ failed;
	}

#घोषणा APPEND(list, मूल्य) \
	slist = curl_slist_append(list, मूल्य);		\
	अगर (slist == शून्य)				\
		जाओ failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->headers, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->headers, "Expect:");

	अगर ((nsoption_charp(accept_language) != शून्य) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		अक्षर s[80];
		snprintf(s, माप s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[माप s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	अगर (nsoption_charp(accept_charset) != शून्य &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		अक्षर s[80];
		snprintf(s, माप s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[माप s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	अगर (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->headers, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	क्रम (i = 0; headers[i] != शून्य; i++) {
		APPEND(fetch->headers, headers[i]);
	}

	वापस fetch;

#अघोषित APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->headers);
	मुक्त(fetch);
	वापस शून्य;
}


#अगर_घोषित WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
अटल काष्ठा cert_chain *
fetch_curl_get_cached_chain(काष्ठा curl_fetch_info *f)
{
	काष्ठा cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	अगर (chain == शून्य) {
		chain = hashmap_जोड़ो(curl_fetch_ssl_hashmap, f->url);
	}

	वापस chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
अटल व्योम
fetch_curl_store_certs_in_cache(काष्ठा curl_fetch_info *f)
{
	माप_प्रकार depth;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_DEPTH];
	काष्ठा cert_chain chain, *cached_chain;
	काष्ठा cert_info *certs;

	स_रखो(&chain, 0, माप(chain));

	certs = f->cert_तथ्य;
	chain.depth = f->cert_depth + 1; /* 0 indexed certificate depth */

	क्रम (depth = 0; depth < chain.depth; depth++) {
		अगर (certs[depth].cert == शून्य) {
			/* This certificate is missing, skip it */
			chain.certs[depth].err = SSL_CERT_ERR_CERT_MISSING;
			जारी;
		}

		/* error code (if any) */
		चयन (certs[depth].err) {
		हाल X509_V_OK:
			chain.certs[depth].err = SSL_CERT_ERR_OK;
			अवरोध;

		हाल X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			/* fallthrough */
		हाल X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_कुंजी:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_ISSUER;
			अवरोध;

		हाल X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		हाल X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		हाल X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		हाल X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_SIG;
			अवरोध;

		हाल X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		हाल X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_YOUNG;
			अवरोध;

		हाल X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		हाल X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_OLD;
			अवरोध;

		हाल X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			chain.certs[depth].err = SSL_CERT_ERR_SELF_SIGNED;
			अवरोध;

		हाल X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			chain.certs[depth].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			अवरोध;

		हाल X509_V_ERR_CERT_REVOKED:
			chain.certs[depth].err = SSL_CERT_ERR_REVOKED;
			अवरोध;

		हाल X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[depth].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			अवरोध;

		शेष:
			chain.certs[depth].err = SSL_CERT_ERR_UNKNOWN;
			अवरोध;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[depth].cert);
		BIO_get_mem_ptr(mem, &buf[depth]);
		(व्योम) BIO_set_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[depth].der = (uint8_t *)buf[depth]->तथ्य;
		chain.certs[depth].der_लंबाई = buf[depth]->लंबाई;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	अगर (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	क्रम (depth = 0; depth < chain.depth; depth++) {
		अगर (chain.certs[depth].err == SSL_CERT_ERR_CERT_MISSING) {
			जारी;
		}
		अगर (buf[depth] != शून्य) {
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
अटल पूर्णांक
fetch_curl_verify_callback(पूर्णांक verify_ok, X509_STORE_CTX *x509_ctx)
{
	पूर्णांक depth;
	काष्ठा curl_fetch_info *fetch;

	depth = X509_STORE_CTX_get_error_depth(x509_ctx);
	fetch = X509_STORE_CTX_get_app_तथ्य(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	अगर (depth >= MAX_CERT_DEPTH) {
		X509_STORE_CTX_set_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		वापस 0;
	}

	/* record the max depth */
	अगर (depth > fetch->cert_depth) {
		fetch->cert_depth = depth;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	अगर (!fetch->cert_तथ्य[depth].cert) {
		fetch->cert_तथ्य[depth].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_up_ref(fetch->cert_तथ्य[depth].cert);
		fetch->cert_तथ्य[depth].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	अगर (depth > 0) {
		verify_ok = 1;
	} अन्यथा {
		/* search for deeper certificates in the chain with errors */
		क्रम (depth = fetch->cert_depth; depth > 0; depth--) {
			अगर (fetch->cert_तथ्य[depth].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_set_error(x509_ctx, fetch->cert_तथ्य[depth].err);
			}
		}
	}

	वापस verify_ok;
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
अटल पूर्णांक fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, व्योम *parm)
{
	काष्ठा curl_fetch_info *f = (काष्ठा curl_fetch_info *) parm;
	पूर्णांक ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_set_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_तथ्य(f->host),
					 lwc_string_लंबाई(f->host));

	/* Store fetch struct in context for verify callback */
	अगर (ok) {
		ok = X509_STORE_CTX_set_app_तथ्य(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	अगर (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_in_cache(f);

	वापस ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
अटल CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, व्योम *_sslctx, व्योम *parm)
{
	काष्ठा curl_fetch_info *f = (काष्ठा curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	दीर्घ options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_set_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	अगर (f->downgrade_tls) {
		/* Disable TLS 1.3 if the server can't cope with it */
#अगर_घोषित SSL_OP_NO_TLSv1_3
		options |= SSL_OP_NO_TLSv1_3;
#पूर्ण_अगर
#अगर_घोषित SSL_MODE_SEND_FALLBACK_SCSV
		/* Ensure server rejects the connection if downgraded too far */
		SSL_CTX_set_mode(sslctx, SSL_MODE_SEND_FALLBACK_SCSV);
#पूर्ण_अगर
	}

	SSL_CTX_set_options(sslctx, options);

#अगर_घोषित SSL_OP_NO_TICKET
	SSL_CTX_clear_options(sslctx, SSL_OP_NO_TICKET);
#पूर्ण_अगर

	वापस CURLE_OK;
}


#पूर्ण_अगर /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
अटल व्योम
fetch_curl_report_certs_upstream(काष्ठा curl_fetch_info *f)
{
	fetch_संदेश संदेश;
	काष्ठा cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	अगर (chain != शून्य) {
		संदेश.type = FETCH_CERTS;
		संदेश.तथ्य.chain = chain;

		fetch_send_callback(&संदेश, f->fetch_handle);
	}

	f->sent_ssl_chain = true;
}

#अगर LIBCURL_VERSION_अंक >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
काष्ठा curl_mime_ctx {
	अक्षर *बफर;
	curl_off_t आकार;
	curl_off_t position;
};

अटल माप_प्रकार mime_तथ्य_read_callback(अक्षर *बफर, माप_प्रकार आकार, माप_प्रकार nitems, व्योम *arg)
{
	काष्ठा curl_mime_ctx *mctx = (काष्ठा curl_mime_ctx *) arg;
	curl_off_t sz = mctx->आकार - mctx->position;

	nitems *= आकार;
	अगर(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	अगर(sz) {
		स_नकल(बफर, mctx->बफर + mctx->position, sz);
	}
	mctx->position += sz;
	वापस sz;
}

अटल पूर्णांक mime_तथ्य_seek_callback(व्योम *arg, curl_off_t offset, पूर्णांक origin)
{
	काष्ठा curl_mime_ctx *mctx = (काष्ठा curl_mime_ctx *) arg;

	चयन(origin) {
	हाल अंत_से:
		offset += mctx->आकार;
		अवरोध;
	हाल प्रस्तुत_से:
		offset += mctx->position;
		अवरोध;
	}

	अगर(offset < 0) {
		वापस CURL_SEEKFUNC_FAIL;
	}
	mctx->position = offset;
	वापस CURL_SEEKFUNC_OK;
}

अटल व्योम mime_तथ्य_free_callback(व्योम *arg)
{
	काष्ठा curl_mime_ctx *mctx = (काष्ठा curl_mime_ctx *) arg;
	मुक्त(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
अटल curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    स्थिर काष्ठा fetch_multipart_तथ्य *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode code = CURLE_OK;
	माप_प्रकार मूल्य_len;

	cmime = curl_mime_init(chandle);
	अगर (cmime == शून्य) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		वापस शून्य;
	}

	/* iterate post data */
	क्रम (; multipart != शून्य; multipart = multipart->अग्र) {
		part = curl_mime_addpart(cmime);
		अगर (part == शून्य) {
			जाओ convert_failed;
		}

		code = curl_mime_name(part, multipart->name);
		अगर (code != CURLE_OK) {
			जाओ convert_failed;
		}

		मूल्य_len = म_माप(multipart->मूल्य);

		अगर (multipart->file && मूल्य_len==0) {
			/* file entries with no filename require special handling */
			code=curl_mime_तथ्य(part, multipart->मूल्य, मूल्य_len);
			अगर (code != CURLE_OK) {
				जाओ convert_failed;
			}

			code = curl_mime_filename(part, "");
			अगर (code != CURLE_OK) {
				जाओ convert_failed;
			}

			code = curl_mime_type(part, "application/octet-stream");
			अगर (code != CURLE_OK) {
				जाओ convert_failed;
			}

		} अन्यथा अगर(multipart->file) {
			/* file entry */
			nserror ret;
			अक्षर *leafname = शून्य;
			अक्षर *mimetype = शून्य;

			code = curl_mime_filedata(part, multipart->rawfile);
			अगर (code != CURLE_OK) {
				जाओ convert_failed;
			}

			ret = guit->file->basename(multipart->मूल्य, &leafname, शून्य);
			अगर (ret != NSERROR_OK) {
				जाओ convert_failed;
			}
			code = curl_mime_filename(part, leafname);
			मुक्त(leafname);
			अगर (code != CURLE_OK) {
				जाओ convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->मूल्य);
			अगर (mimetype == शून्य) {
				mimetype=strdup("text/plain");
			}
			अगर (mimetype == शून्य) {
				जाओ convert_failed;
			}
			code = curl_mime_type(part, mimetype);
			मुक्त(mimetype);
			अगर (code != CURLE_OK) {
				जाओ convert_failed;
			}

		} अन्यथा {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			काष्ठा curl_mime_ctx *cb_ctx;
			cb_ctx = दो_स्मृति(माप(काष्ठा curl_mime_ctx));
			अगर (cb_ctx == शून्य) {
				जाओ convert_failed;
			}
			cb_ctx->बफर = multipart->मूल्य;
			cb_ctx->आकार = मूल्य_len;
			cb_ctx->position = 0;
			code = curl_mime_तथ्य_cb(part,
						 मूल्य_len,
						 mime_तथ्य_read_callback,
						 mime_तथ्य_seek_callback,
						 mime_तथ्य_free_callback,
						 cb_ctx);
			अगर (code != CURLE_OK) {
				मुक्त(cb_ctx);
				जाओ convert_failed;
			}
		}
	}

	वापस cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", code);
	curl_mime_free(cmime);
	वापस शून्य;
}

#अन्यथा /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
अटल काष्ठा curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    स्थिर काष्ठा fetch_multipart_तथ्य *control)
{
	काष्ठा curl_httppost *post = शून्य, *last = शून्य;
	CURLFORMcode code;
	nserror ret;

	क्रम (; control; control = control->अग्र) {
		अगर (control->file) {
			अक्षर *leafname = शून्य;
			ret = guit->file->basename(control->मूल्य, &leafname, शून्य);
			अगर (ret != NSERROR_OK) {
				जारी;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			अगर (control->मूल्य[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				अटल अक्षर buf;

				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_बफर, control->मूल्य,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_END);
				अगर (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", code,
					      control->name);
			} अन्यथा {
				अक्षर *mimetype = guit->fetch->mimetype(control->मूल्य);
				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_END);
				अगर (code != CURL_FORMADD_OK)
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
			अगर (code != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", code,
				      control->name, control->मूल्य);
		}
	}

	वापस post;
}

#पूर्ण_अगर  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
अटल CURLcode fetch_curl_set_postdata(काष्ठा curl_fetch_info *f)
{
	CURLcode code = CURLE_OK;

#अघोषित SETOPT
#घोषणा SETOPT(option, मूल्य) { \
	code = curl_easy_setopt(f->curl_handle, option, मूल्य);	\
	अगर (code != CURLE_OK)					\
		वापस code;					\
	}

	चयन (f->postdata->type) {
	हाल FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, शून्य);
		SETOPT(NSCURL_POSTDATA_CURLOPT, शून्य);
		SETOPT(CURLOPT_HTTPGET, 1L);
		अवरोध;

	हाल FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, शून्य);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->तथ्य.urlenc);
		अवरोध;

	हाल FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, शून्य);
		SETOPT(CURLOPT_HTTPGET, 0L);
		अगर (f->curl_postdata == शून्य) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->तथ्य.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		अवरोध;
	}
	वापस code;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
अटल CURLcode fetch_curl_set_options(काष्ठा curl_fetch_info *f)
{
	CURLcode code;
	स्थिर अक्षर *auth;

#अघोषित SETOPT
#घोषणा SETOPT(option, मूल्य) { \
	code = curl_easy_setopt(f->curl_handle, option, मूल्य);	\
	अगर (code != CURLE_OK)					\
		वापस code;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_PRIVATE, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_PROGRESS_तथ्य, f);
	SETOPT(CURLOPT_HTTPHEADER, f->headers);
	code = fetch_curl_set_postdata(f);
	अगर (code != CURLE_OK) {
		वापस code;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	अगर (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} अन्यथा {
		SETOPT(CURLOPT_COOKIE, शून्य);
	}

	अगर ((auth = urldb_get_auth_details(f->url, शून्य)) != शून्य) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} अन्यथा {
		SETOPT(CURLOPT_USERPWD, शून्य);
	}

	/* set up proxy options */
	अगर (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != शून्य) &&
	    (म_भेदन(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (दीर्घ) nsoption_int(http_proxy_port));

#अगर LIBCURL_VERSION_अंक >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#पूर्ण_अगर

		अगर (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(दीर्घ) CURLAUTH_BASIC :
					(दीर्घ) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					माप fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} अन्यथा {
		SETOPT(CURLOPT_PROXY, शून्य);
	}


	अगर (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	अगर (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		अगर (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, शून्य);
			SETOPT(CURLOPT_SSL_CTX_तथ्य, शून्य);
		}
	} अन्यथा {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#अगर_घोषित WITH_OPENSSL
		अगर (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_तथ्य, f);
		}
#पूर्ण_अगर
	}

	वापस CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
अटल bool
fetch_curl_initiate_fetch(काष्ठा curl_fetch_info *fetch, CURL *handle)
{
	CURLcode code;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	code = fetch_curl_set_options(fetch);
	अगर (code != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		वापस false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_add_handle(fetch_curl_multi, fetch->curl_handle);
	निश्चित(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	वापस true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
अटल CURL *fetch_curl_get_handle(lwc_string *host)
{
	काष्ठा cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	अगर (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		मुक्त(h);
	} अन्यथा {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	वापस ret;
}


/**
 * Dispatch a single job
 */
अटल bool fetch_curl_start(व्योम *vfetch)
{
	काष्ठा curl_fetch_info *fetch = (काष्ठा curl_fetch_info*)vfetch;
	अगर (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		वापस false;
	}
	वापस fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
अटल व्योम fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#अगर LIBCURL_VERSION_अंक >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	वापस;
#अन्यथा
	काष्ठा cache_handle *h = 0;
	पूर्णांक c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	अगर (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		वापस;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(काष्ठा cache_handle, curl_handle_ring, c);
	अगर (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		अगर (curl_handle_ring != शून्य) {
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

		वापस;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (काष्ठा cache_handle*)दो_स्मृति(माप(काष्ठा cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_जोड़ो(curl_handle_ring, h);
#पूर्ण_अगर
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
अटल व्योम fetch_curl_stop(काष्ठा curl_fetch_info *f)
{
	CURLMcode codem;

	निश्चित(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	अगर (f->curl_handle) {
		/* remove from curl multi handle */
		codem = curl_multi_remove_handle(fetch_curl_multi,
				f->curl_handle);
		निश्चित(codem == CURLM_OK);
		/* Put this curl handle into the cache if wanted. */
		fetch_curl_cache_handle(f->curl_handle, f->host);
		f->curl_handle = 0;
	}

	fetch_remove_from_queues(f->fetch_handle);
}


/**
 * Abort a fetch.
 */
अटल व्योम fetch_curl_abort(व्योम *vf)
{
	काष्ठा curl_fetch_info *f = (काष्ठा curl_fetch_info *)vf;
	निश्चित(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	अगर (f->curl_handle) {
		अगर (inside_curl) {
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
अटल व्योम fetch_curl_free(व्योम *vf)
{
	काष्ठा curl_fetch_info *f = (काष्ठा curl_fetch_info *)vf;
	पूर्णांक i;

	अगर (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	मुक्त(f->location);
	मुक्त(f->cookie_string);
	मुक्त(f->realm);
	अगर (f->headers) {
		curl_slist_free_all(f->headers);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	क्रम (i = 0; i < MAX_CERT_DEPTH; i++) {
		अगर (f->cert_तथ्य[i].cert != शून्य) {
			X509_free(f->cert_तथ्य[i].cert);
		}
	}

	मुक्त(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
अटल bool fetch_curl_process_headers(काष्ठा curl_fetch_info *f)
{
	दीर्घ http_code;
	CURLcode code;
	fetch_संदेश संदेश;

	f->had_headers = true;

	अगर (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		निश्चित(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}
	http_code = f->http_code;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_code);

	अगर ((http_code == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		संदेश.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&संदेश, f->fetch_handle);
		वापस true;
	}

	/* handle HTTP redirects (3xx response codes) */
	अगर (HTTP_RESPONSE_IS_3XX(http_code) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		संदेश.type = FETCH_REDIRECT;
		संदेश.तथ्य.redirect = f->location;
		fetch_send_callback(&संदेश, f->fetch_handle);
		वापस true;
	}

	/* handle HTTP 401 (Authentication errors) */
	अगर (http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		संदेश.type = FETCH_AUTH;
		संदेश.तथ्य.auth.realm = f->realm;
		fetch_send_callback(&संदेश, f->fetch_handle);
		वापस true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	अगर (f->only_2xx &&
	    म_भेदन(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_code) == false)) {
		संदेश.type = FETCH_ERROR;
		संदेश.तथ्य.error = messages_get("Not2xx");
		fetch_send_callback(&संदेश, f->fetch_handle);
		वापस true;
	}

	अगर (f->पात)
		वापस true;

	वापस false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
अटल व्योम fetch_curl_done(CURL *curl_handle, CURLcode परिणाम)
{
	bool finished = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	काष्ठा curl_fetch_info *f;
	अक्षर **_hideous_hack = (अक्षर **) (व्योम *) &f;
	CURLcode code;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	code = curl_easy_getinfo(curl_handle, CURLINFO_PRIVATE, _hideous_hack);
	निश्चित(code == CURLE_OK);

	abort_fetch = f->पात;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	अगर ((abort_fetch == false) &&
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
		अगर (f->stopped ||
		    (!f->had_headers &&	fetch_curl_process_headers(f))) {
			; /* redirect with no body or similar */
		} अन्यथा {
			finished = true;
		}
	} अन्यथा अगर (परिणाम == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		अगर (!f->had_headers && fetch_curl_process_headers(f))
			; /* redirect with partial body, or similar */
		अन्यथा {
			finished = true;
		}
	} अन्यथा अगर (परिणाम == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} अन्यथा अगर (परिणाम == CURLE_SSL_PEER_CERTIFICATE ||
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

	अगर (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	अगर (abort_fetch) {
		; /* fetch was aborted: no callback */
	} अन्यथा अगर (finished) {
		fetch_संदेश संदेश;
		संदेश.type = FETCH_FINISHED;
		fetch_send_callback(&संदेश, f->fetch_handle);
	} अन्यथा अगर (cert) {
		/* user needs to validate certificate with issue */
		fetch_संदेश संदेश;
		संदेश.type = FETCH_CERT_ERR;
		fetch_send_callback(&संदेश, f->fetch_handle);
	} अन्यथा अगर (error) {
		fetch_संदेश संदेश;
		चयन (परिणाम) {
		हाल CURLE_SSL_CONNECT_ERROR:
			संदेश.type = FETCH_SSL_ERR;
			अवरोध;

		हाल CURLE_OPERATION_TIMEDOUT:
			संदेश.type = FETCH_TIMEDOUT;
			संदेश.तथ्य.error = curl_easy_strerror(परिणाम);
			अवरोध;

		शेष:
			संदेश.type = FETCH_ERROR;
			संदेश.तथ्य.error = curl_easy_strerror(परिणाम);
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
अटल व्योम fetch_curl_poll(lwc_string *scheme_ignored)
{
	पूर्णांक running, queue;
	CURLMcode codem;
	CURLMsg *curl_संदेश;

	अगर (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_set, write_fd_set, exc_fd_set;
		पूर्णांक max_fd = -1;
		पूर्णांक i;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&exc_fd_set);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_set, &write_fd_set,
				&exc_fd_set, &max_fd);
		निश्चित(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		क्रम (i = 0; i <= max_fd; i++) {
			bool पढ़ो = false;
			bool ग_लिखो = false;
			bool error = false;

			अगर (FD_ISSET(i, &read_fd_set)) {
				पढ़ो = true;
			}
			अगर (FD_ISSET(i, &write_fd_set)) {
				ग_लिखो = true;
			}
			अगर (FD_ISSET(i, &exc_fd_set)) {
				error = true;
			}
			अगर (पढ़ो || ग_लिखो || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      पढ़ो ? "read" : "    ",
				      ग_लिखो ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	करो {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		अगर (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			वापस;
		}
	} जबतक (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_संदेश = curl_multi_info_read(fetch_curl_multi, &queue);
	जबतक (curl_संदेश) {
		चयन (curl_संदेश->संदेश) {
			हाल CURLMSG_DONE:
				fetch_curl_done(curl_संदेश->easy_handle,
						curl_संदेश->तथ्य.परिणाम);
				अवरोध;
			शेष:
				अवरोध;
		}
		curl_संदेश = curl_multi_info_read(fetch_curl_multi, &queue);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
अटल पूर्णांक
fetch_curl_progress(व्योम *clientp,
		    NSCURL_PROGRESS_T dltotal,
		    NSCURL_PROGRESS_T dlnow,
		    NSCURL_PROGRESS_T ultotal,
		    NSCURL_PROGRESS_T ulnow)
{
	अटल अक्षर fetch_progress_बफर[256]; /**< Progress buffer for cURL */
	काष्ठा curl_fetch_info *f = (काष्ठा curl_fetch_info *) clientp;
	uint64_t time_now_ms;
	fetch_संदेश संदेश;

	अगर (f->पात) {
		वापस 0;
        }

	संदेश.type = FETCH_PROGRESS;
	संदेश.तथ्य.progress = fetch_progress_बफर;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&time_now_ms);
#घोषणा UPDATE_DELAY_MS (1000 / UPDATES_PER_SECOND)
	अगर (time_now_ms - f->last_progress_update < UPDATE_DELAY_MS) {
		वापस 0;
        }
#अघोषित UPDATE_DELAY_MS
	f->last_progress_update = time_now_ms;

	अगर (dltotal > 0) {
		snprintf(fetch_progress_बफर, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&संदेश, f->fetch_handle);
	} अन्यथा {
		snprintf(fetch_progress_बफर, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&संदेश, f->fetch_handle);
	}

	वापस 0;
}


/**
 * Format curl debug for nslog
 */
अटल पूर्णांक
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 अक्षर *तथ्य,
		 माप_प्रकार आकार,
		 व्योम *userptr)
{
	अटल स्थिर अक्षर s_infotype[CURLINFO_END][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	चयन(type) {
	हाल CURLINFO_TEXT:
	हाल CURLINFO_HEADER_OUT:
	हाल CURLINFO_HEADER_IN:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (पूर्णांक)आकार - 1, तथ्य);
		अवरोध;

	शेष:
		अवरोध;
	}
	वापस 0;
}


अटल curl_socket_t fetch_curl_socket_open(व्योम *clientp,
		curlsocktype purpose, काष्ठा curl_sockaddr *address)
{
	(व्योम) clientp;
	(व्योम) purpose;

	वापस (curl_socket_t) guit->fetch->socket_open(
			address->family, address->socktype,
			address->protocol);
}

अटल पूर्णांक fetch_curl_socket_close(व्योम *clientp, curl_socket_t item)
{
	(व्योम) clientp;

	वापस guit->fetch->socket_close((पूर्णांक) item);
}

/**
 * Callback function for cURL.
 */
अटल माप_प्रकार fetch_curl_तथ्य(अक्षर *तथ्य, माप_प्रकार आकार, माप_प्रकार nmemb, व्योम *_f)
{
	काष्ठा curl_fetch_info *f = _f;
	CURLcode code;
	fetch_संदेश संदेश;

	/* ensure we only have to get this information once */
	अगर (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		निश्चित(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	अगर (f->http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_code = 0;
		वापस आकार * nmemb;
	}

	अगर (f->पात || (!f->had_headers && fetch_curl_process_headers(f))) {
		f->stopped = true;
		वापस 0;
	}

	/* send data to the caller */
	संदेश.type = FETCH_तथ्य;
	संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *) तथ्य;
	संदेश.तथ्य.header_or_तथ्य.len = आकार * nmemb;
	fetch_send_callback(&संदेश, f->fetch_handle);

	अगर (f->पात) {
		f->stopped = true;
		वापस 0;
	}

	वापस आकार * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
अटल माप_प्रकार
fetch_curl_header(अक्षर *तथ्य, माप_प्रकार आकार, माप_प्रकार nmemb, व्योम *_f)
{
	काष्ठा curl_fetch_info *f = _f;
	पूर्णांक i;
	fetch_संदेश संदेश;
	आकार *= nmemb;

	अगर (f->पात) {
		f->stopped = true;
		वापस 0;
	}

	अगर (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	संदेश.type = FETCH_HEADER;
	संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *) तथ्य;
	संदेश.तथ्य.header_or_तथ्य.len = आकार;
	fetch_send_callback(&संदेश, f->fetch_handle);

#घोषणा SKIP_ST(o) क्रम (i = (o); i < (पूर्णांक) आकार && (तथ्य[i] == ' ' || तथ्य[i] == '\t'); i++)

	अगर (12 < आकार && strncasecmp(तथ्य, "Location:", 9) == 0) {
		/* extract Location header */
		मुक्त(f->location);
		f->location = दो_स्मृति(आकार);
		अगर (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			वापस आकार;
		}
		SKIP_ST(9);
		म_नकलन(f->location, तथ्य + i, आकार - i);
		f->location[आकार - i] = '\0';
		क्रम (i = आकार - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} अन्यथा अगर (15 < आकार && strncasecmp(तथ्य, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		अगर (i < (पूर्णांक)आकार && '0' <= तथ्य[i] && तथ्य[i] <= '9')
			f->content_लंबाई = म_से_द(तथ्य + i);
	} अन्यथा अगर (17 < आकार && strncasecmp(तथ्य, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		जबतक (i < (पूर्णांक) आकार - 5 &&
				strncasecmp(तथ्य + i, "realm", 5))
			i++;
		जबतक (i < (पूर्णांक) आकार - 1 && तथ्य[++i] != '"')
			/* */;
		i++;

		अगर (i < (पूर्णांक) आकार) {
			माप_प्रकार end = i;

			जबतक (end < आकार && तथ्य[end] != '"')
				++end;

			अगर (end < आकार) {
				मुक्त(f->realm);
				f->realm = दो_स्मृति(end - i + 1);
				अगर (f->realm != शून्य) {
					म_नकलन(f->realm, तथ्य + i, end - i);
					f->realm[end - i] = '\0';
				}
			}
		}
	} अन्यथा अगर (11 < आकार && strncasecmp(तथ्य, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_set_cookie(f->fetch_handle, &तथ्य[i]);
	}

	वापस आकार;
#अघोषित SKIP_ST
}

अटल पूर्णांक fetch_curl_fdset(lwc_string *scheme, fd_set *read_set,
			    fd_set *write_set, fd_set *error_set)
{
	CURLMcode code;
	पूर्णांक maxfd = -1;

	code = curl_multi_fdset(fetch_curl_multi,
				read_set,
				write_set,
				error_set,
				&maxfd);
	निश्चित(code == CURLM_OK);

	वापस maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_register(व्योम)
{
	CURLcode code;
	curl_version_info_तथ्य *तथ्य;
	पूर्णांक i;
	lwc_string *scheme;
	स्थिर काष्ठा fetcher_operation_table fetcher_ops = {
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

#अगर LIBCURL_VERSION_अंक >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, शून्य, शून्य);
	अगर (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} अन्यथा {
		curl_with_openssl = false;
	}
#पूर्ण_अगर

	NSLOG(netsurf, INFO, "curl_version %s", curl_version());

	code = curl_global_init(CURL_GLOBAL_ALL);
	अगर (code != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		वापस NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	अगर (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		वापस NSERROR_INIT_FAILED;
	}

#अगर LIBCURL_VERSION_अंक >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		पूर्णांक maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#अघोषित SETOPT
#घोषणा SETOPT(option, मूल्य) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, मूल्य);	\
	अगर (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		जाओ curl_multi_setopt_failed;				\
	}

		SETOPT(CURLMOPT_MAXCONNECTS, (दीर्घ)maxconnects);
		SETOPT(CURLMOPT_MAX_योग_CONNECTIONS, (दीर्घ)maxconnects);
		SETOPT(CURLMOPT_MAX_HOST_CONNECTIONS, (दीर्घ)nsoption_int(max_fetchers_per_host));
	}
#पूर्ण_अगर

	/* Create a curl easy handle with the options that are common to all
	 *  fetches.
	 */
	fetch_blank_curl = curl_easy_init();
	अगर (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		वापस NSERROR_INIT_FAILED;
	}

#अघोषित SETOPT
#घोषणा SETOPT(option, मूल्य) \
	code = curl_easy_setopt(fetch_blank_curl, option, मूल्य);	\
	अगर (code != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		जाओ curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_बफर);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	अगर (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} अन्यथा {
		SETOPT(CURLOPT_VERBOSE, 1);
	}

	/* Currently we explode if curl uses HTTP2, so force 1.1. */
	SETOPT(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

	SETOPT(CURLOPT_WRITEFUNCTION, fetch_curl_तथ्य);
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

	अगर (nsoption_charp(ca_bundle) &&
	    म_भेद(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	अगर (nsoption_charp(ca_path) && म_भेद(nsoption_charp(ca_path), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_path));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_path));
	}

#अगर LIBCURL_VERSION_अंक < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	code = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, शून्य);
	अगर (code != CURLE_OK) {
		curl_with_openssl = false;
	} अन्यथा {
		curl_with_openssl = true;
	}
#पूर्ण_अगर

	अगर (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#अगर LIBCURL_VERSION_अंक >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		code = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		अगर (code != CURLE_OK && code != CURLE_NOT_BUILT_IN)
			जाओ curl_easy_setopt_failed;
#पूर्ण_अगर
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	तथ्य = curl_version_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	अगर (curl_fetch_ssl_hashmap == शून्य) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		वापस NSERROR_NOMEM;
	}

	क्रम (i = 0; तथ्य->protocols[i]; i++) {
		अगर (म_भेद(तथ्य->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} अन्यथा अगर (म_भेद(तथ्य->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} अन्यथा {
			/* Ignore non-http(s) protocols */
			जारी;
		}

		अगर (fetcher_add(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      तथ्य->protocols[i]);
		}
	}

	वापस NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	वापस NSERROR_INIT_FAILED;

#अगर LIBCURL_VERSION_अंक >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	वापस NSERROR_INIT_FAILED;
#पूर्ण_अगर
}
