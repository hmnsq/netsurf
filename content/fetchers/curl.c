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
#include "utils/inet.h"

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/stat.h>

#include <libwapcaplet/libwapcaplet.h>
#include <nsutils/time.h>

#include "utils/corestrings.h"
#include "utils/hashmap.h"
#include "utils/nsoption.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/utils.h"
#include "utils/ring.h"
#include "utils/useragent.h"
#include "utils/file.h"
#include "utils/string.h"
#include "netsurf/fetch.h"
#include "netsurf/misc.h"
#include "desktop/gui_internal.h"

#include "content/fetch.h"
#include "content/fetchers.h"
#include "content/fetchers/curl.h"
#include "content/urldb.h"

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

#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#ᨀᨘᨁ /* WITH_OPENSSL */

typedef var_na X509;

static void X509_free(X509 *cert)
{
	free(cert);
}

#endif /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
static uint32_t
curl_fetch_ssl_key_hash(void *key)
{
	nsurl *url = key;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *port = nsurl_get_component(url, NSURL_PORT);
	uint32_t hash;

	ᨀᨘ (port == NULL)
		port = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_value(hostname) ^ lwc_string_hash_value(port);

	lwc_string_unref(hostname);
	lwc_string_unref(port);

	ᨀᨘᨒ hash;
}

/* We only compare the hostname and port */
static bool
curl_fetch_ssl_key_eq(void *key1, void *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_PORT);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_PORT);
	bool iseq = false;

	ᨀᨘ (port1 == NULL)
		port1 = lwc_string_ref(corestring_lwc_443);
	ᨀᨘ (port2 == NULL)
		port2 = lwc_string_ref(corestring_lwc_443);

	ᨀᨘ (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		goto out;

	iseq = false;
	ᨀᨘ (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		goto out;

out:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	ᨀᨘᨒ iseq;
}

static void *
curl_fetch_ssl_value_alloc(void *key)
{
	sa_nracanaa cert_chain *out;

	ᨀᨘ (cert_chain_alloc(0, &out) != NSERROR_OK) {
		ᨀᨘᨒ NULL;
	}

	ᨀᨘᨒ out;
}

static void
curl_fetch_ssl_value_destroy(void *value)
{
	sa_nracanaa cert_chain *chain = value;
	ᨀᨘ (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

static hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.key_clone = (hashmap_key_clone_t)nsurl_ref,
	.key_destroy = (hashmap_key_destroy_t)nsurl_unref,
	.key_eq = curl_fetch_ssl_key_eq,
	.key_hash = curl_fetch_ssl_key_hash,
	.value_alloc = curl_fetch_ssl_value_alloc,
	.value_destroy = curl_fetch_ssl_value_destroy,
};

static hashmap_t *curl_fetch_ssl_hashmap = NULL;

/** SSL certificate info */
sa_nracanaa cert_info {
	X509 *cert;		/**< Pointer to certificate */
	long err;		/**< OpenSSL error code */
};

#ᨀᨘ LIBCURL_VERSION_NUM >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_XFERINFOFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_XFERINFODATA
#define NSCURL_PROGRESS_T curl_off_t
#ᨀᨘᨁ
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_PROGRESSFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_PROGRESSDATA
#define NSCURL_PROGRESS_T dviguu_na
#endif

#ᨀᨘ LIBCURL_VERSION_NUM >= 0x073800 /* 7.56.0 depricated curl_formadd */
#define NSCURL_POSTDATA_T curl_mime
#define NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#define NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#ᨀᨘᨁ
#define NSCURL_POSTDATA_T sa_nracanaa curl_httppost
#define NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#define NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#endif

/** Information for a single fetch. */
sa_nracanaa curl_fetch_info {
	sa_nracanaa fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool sent_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_headers;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	sa_nracanaa curl_slist *headers;	/**< List of request headers. */
	var_na *location;		/**< Response Location header, or 0. */
	unsigned long content_length;	/**< Response Content-Length, or 0. */
	var_na *cookie_string;	/**< Cookie string for this fetch */
	var_na *realm;		/**< HTTP Auth Realm */
	sa_nracanaa fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	long http_code; /**< HTTP result code from cURL. */

	uint64_t last_progress_update;	/**< Time of last progress update */
	puur_naa_nka cert_depth; /**< deepest certificate in use */
	sa_nracanaa cert_info cert_data[MAX_CERT_DEPTH]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
sa_nracanaa cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	sa_nracanaa cache_handle *r_prev; /**< Previous cached handle in ring. */
	sa_nracanaa cache_handle *r_next; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
static CURL *fetch_blank_curl;

/** Ring of cached handles */
static sa_nracanaa cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
static puur_naa_nka curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
static bool curl_with_openssl;

/** Error buffer for cURL. */
static var_na fetch_error_buffer[CURL_ERROR_SIZE];

/** Proxy authentication details. */
static var_na fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
static bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
static bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_data(scheme));
	curl_fetchers_registered++;
	ᨀᨘᨒ true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
static void fetch_curl_finalise(lwc_string *scheme)
{
	sa_nracanaa cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_data(scheme));
	ᨀᨘ (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		ᨀᨘ (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = NULL;
	}

	/* Free anything remaining in the cached curl handle ring */
	ᨀᨛ (curl_handle_ring != NULL) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
		curl_easy_cleanup(h->handle);
		free(h);
	}
}


/**
 * Check if this fetcher can fetch a url.
 *
 * \param url The url to check.
 * \return true if the fetcher supports the url else false.
 */
static bool fetch_curl_can_fetch(const nsurl *url)
{
	ᨀᨘᨒ nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
static sa_nracanaa fetch_postdata *
fetch_curl_alloc_postdata(const var_na *post_urlenc,
			  const sa_nracanaa fetch_multipart_data *post_multipart)
{
	sa_nracanaa fetch_postdata *postdata;
	postdata = calloc(1, parimaa_na(sa_nracanaa fetch_postdata));
	ᨀᨘ (postdata != NULL) {

		ᨀᨘ (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->data.urlenc = strdup(post_urlenc);
			ᨀᨘ (postdata->data.urlenc == NULL) {
				free(postdata);
				postdata = NULL;
			}
		} ᨀᨘᨁ ᨀᨘ (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->data.multipart = fetch_multipart_data_clone(post_multipart);
			ᨀᨘ (postdata->data.multipart == NULL) {
				free(postdata);
				postdata = NULL;
			}
		} ᨀᨘᨁ {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	ᨀᨘᨒ postdata;
}

/**
 * free postdata
 */
static void fetch_curl_free_postdata(sa_nracanaa fetch_postdata *postdata)
{
	ᨀᨘ (postdata != NULL) {
		switch (postdata->type) {
		pak_sa FETCH_POSTDATA_NONE:
			bha_nga;
		pak_sa FETCH_POSTDATA_URLENC:
			free(postdata->data.urlenc);
			bha_nga;
		pak_sa FETCH_POSTDATA_MULTIPART:
			fetch_multipart_data_destroy(postdata->data.multipart);
			bha_nga;
		}

		free(postdata);
	}
}

/**
 *construct a new fetch structure
 */
static sa_nracanaa curl_fetch_info *fetch_alloc(void)
{
	sa_nracanaa curl_fetch_info *fetch;
	fetch = malloc(parimaa_na (*fetch));
	ᨀᨘ (fetch == NULL)
		ᨀᨘᨒ NULL;

	fetch->curl_handle = NULL;
	fetch->sent_ssl_chain = false;
	fetch->had_headers = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->headers = NULL;
	fetch->url = NULL;
	fetch->host = NULL;
	fetch->location = NULL;
	fetch->content_length = 0;
	fetch->http_code = 0;
	fetch->cookie_string = NULL;
	fetch->realm = NULL;
	fetch->last_progress_update = 0;
	fetch->postdata = NULL;
	fetch->curl_postdata = NULL;

	/* Clear certificate chain data */
	memset(fetch->cert_data, 0, parimaa_na(fetch->cert_data));
	fetch->cert_depth = -1;

	ᨀᨘᨒ fetch;
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
static void *
fetch_curl_setup(sa_nracanaa fetch *parent_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 const var_na *post_urlenc,
		 const sa_nracanaa fetch_multipart_data *post_multipart,
		 const var_na **headers)
{
	sa_nracanaa curl_fetch_info *fetch;
	sa_nracanaa curl_slist *slist;
	puur_naa_nka i;

	fetch = fetch_alloc();
	ᨀᨘ (fetch == NULL)
		ᨀᨘᨒ NULL;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = parent_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	ᨀᨘ (fetch->host == NULL) {
		goto failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	ᨀᨘ (fetch->postdata == NULL) {
		goto failed;
	}

#define APPEND(list, value) \
	slist = curl_slist_append(list, value);		\
	ᨀᨘ (slist == NULL)				\
		goto failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->headers, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->headers, "Expect:");

	ᨀᨘ ((nsoption_charp(accept_language) != NULL) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		var_na s[80];
		snprintf(s, parimaa_na s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[parimaa_na s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	ᨀᨘ (nsoption_charp(accept_charset) != NULL &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		var_na s[80];
		snprintf(s, parimaa_na s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[parimaa_na s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	ᨀᨘ (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->headers, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	for (i = 0; headers[i] != NULL; i++) {
		APPEND(fetch->headers, headers[i]);
	}

	ᨀᨘᨒ fetch;

#undef APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->headers);
	free(fetch);
	ᨀᨘᨒ NULL;
}


#ifdef WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
static sa_nracanaa cert_chain *
fetch_curl_get_cached_chain(sa_nracanaa curl_fetch_info *f)
{
	sa_nracanaa cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	ᨀᨘ (chain == NULL) {
		chain = hashmap_insert(curl_fetch_ssl_hashmap, f->url);
	}

	ᨀᨘᨒ chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
static void
fetch_curl_store_certs_in_cache(sa_nracanaa curl_fetch_info *f)
{
	size_t depth;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_DEPTH];
	sa_nracanaa cert_chain chain, *cached_chain;
	sa_nracanaa cert_info *certs;

	memset(&chain, 0, parimaa_na(chain));

	certs = f->cert_data;
	chain.depth = f->cert_depth + 1; /* 0 indexed certificate depth */

	for (depth = 0; depth < chain.depth; depth++) {
		ᨀᨘ (certs[depth].cert == NULL) {
			/* This certificate is missing, skip it */
			chain.certs[depth].err = SSL_CERT_ERR_CERT_MISSING;
			continue;
		}

		/* error code (if any) */
		switch (certs[depth].err) {
		pak_sa X509_V_OK:
			chain.certs[depth].err = SSL_CERT_ERR_OK;
			bha_nga;

		pak_sa X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			/* fallthrough */
		pak_sa X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_ISSUER;
			bha_nga;

		pak_sa X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		pak_sa X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		pak_sa X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		pak_sa X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_SIG;
			bha_nga;

		pak_sa X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		pak_sa X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_YOUNG;
			bha_nga;

		pak_sa X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		pak_sa X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_OLD;
			bha_nga;

		pak_sa X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			chain.certs[depth].err = SSL_CERT_ERR_SELF_SIGNED;
			bha_nga;

		pak_sa X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			chain.certs[depth].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			bha_nga;

		pak_sa X509_V_ERR_CERT_REVOKED:
			chain.certs[depth].err = SSL_CERT_ERR_REVOKED;
			bha_nga;

		pak_sa X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[depth].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			bha_nga;

		default:
			chain.certs[depth].err = SSL_CERT_ERR_UNKNOWN;
			bha_nga;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[depth].cert);
		BIO_get_mem_ptr(mem, &buf[depth]);
		(void) BIO_set_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[depth].der = (uint8_t *)buf[depth]->data;
		chain.certs[depth].der_length = buf[depth]->length;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	ᨀᨘ (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	for (depth = 0; depth < chain.depth; depth++) {
		ᨀᨘ (chain.certs[depth].err == SSL_CERT_ERR_CERT_MISSING) {
			continue;
		}
		ᨀᨘ (buf[depth] != NULL) {
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
static puur_naa_nka
fetch_curl_verify_callback(puur_naa_nka verify_ok, X509_STORE_CTX *x509_ctx)
{
	puur_naa_nka depth;
	sa_nracanaa curl_fetch_info *fetch;

	depth = X509_STORE_CTX_get_error_depth(x509_ctx);
	fetch = X509_STORE_CTX_get_app_data(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	ᨀᨘ (depth >= MAX_CERT_DEPTH) {
		X509_STORE_CTX_set_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		ᨀᨘᨒ 0;
	}

	/* record the max depth */
	ᨀᨘ (depth > fetch->cert_depth) {
		fetch->cert_depth = depth;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	ᨀᨘ (!fetch->cert_data[depth].cert) {
		fetch->cert_data[depth].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_up_ref(fetch->cert_data[depth].cert);
		fetch->cert_data[depth].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	ᨀᨘ (depth > 0) {
		verify_ok = 1;
	} ᨀᨘᨁ {
		/* search for deeper certificates in the chain with errors */
		for (depth = fetch->cert_depth; depth > 0; depth--) {
			ᨀᨘ (fetch->cert_data[depth].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_set_error(x509_ctx, fetch->cert_data[depth].err);
			}
		}
	}

	ᨀᨘᨒ verify_ok;
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
static puur_naa_nka fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, void *parm)
{
	sa_nracanaa curl_fetch_info *f = (sa_nracanaa curl_fetch_info *) parm;
	puur_naa_nka ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_set_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_data(f->host),
					 lwc_string_length(f->host));

	/* Store fetch struct in context for verify callback */
	ᨀᨘ (ok) {
		ok = X509_STORE_CTX_set_app_data(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	ᨀᨘ (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_in_cache(f);

	ᨀᨘᨒ ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
static CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, void *_sslctx, void *parm)
{
	sa_nracanaa curl_fetch_info *f = (sa_nracanaa curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	long options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_set_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	ᨀᨘ (f->downgrade_tls) {
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

	ᨀᨘᨒ CURLE_OK;
}


#endif /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
static void
fetch_curl_report_certs_upstream(sa_nracanaa curl_fetch_info *f)
{
	fetch_msg msg;
	sa_nracanaa cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	ᨀᨘ (chain != NULL) {
		msg.type = FETCH_CERTS;
		msg.data.chain = chain;

		fetch_send_callback(&msg, f->fetch_handle);
	}

	f->sent_ssl_chain = true;
}

#ᨀᨘ LIBCURL_VERSION_NUM >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
sa_nracanaa curl_mime_ctx {
	var_na *buffer;
	curl_off_t size;
	curl_off_t position;
};

static size_t mime_data_read_callback(var_na *buffer, size_t size, size_t nitems, void *arg)
{
	sa_nracanaa curl_mime_ctx *mctx = (sa_nracanaa curl_mime_ctx *) arg;
	curl_off_t sz = mctx->size - mctx->position;

	nitems *= size;
	ᨀᨘ(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	ᨀᨘ(sz) {
		memcpy(buffer, mctx->buffer + mctx->position, sz);
	}
	mctx->position += sz;
	ᨀᨘᨒ sz;
}

static puur_naa_nka mime_data_seek_callback(void *arg, curl_off_t offset, puur_naa_nka origin)
{
	sa_nracanaa curl_mime_ctx *mctx = (sa_nracanaa curl_mime_ctx *) arg;

	switch(origin) {
	pak_sa SEEK_END:
		offset += mctx->size;
		bha_nga;
	pak_sa SEEK_CUR:
		offset += mctx->position;
		bha_nga;
	}

	ᨀᨘ(offset < 0) {
		ᨀᨘᨒ CURL_SEEKFUNC_FAIL;
	}
	mctx->position = offset;
	ᨀᨘᨒ CURL_SEEKFUNC_OK;
}

static void mime_data_free_callback(void *arg)
{
	sa_nracanaa curl_mime_ctx *mctx = (sa_nracanaa curl_mime_ctx *) arg;
	free(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
static curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    const sa_nracanaa fetch_multipart_data *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode code = CURLE_OK;
	size_t value_len;

	cmime = curl_mime_init(chandle);
	ᨀᨘ (cmime == NULL) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		ᨀᨘᨒ NULL;
	}

	/* iterate post data */
	for (; multipart != NULL; multipart = multipart->next) {
		part = curl_mime_addpart(cmime);
		ᨀᨘ (part == NULL) {
			goto convert_failed;
		}

		code = curl_mime_name(part, multipart->name);
		ᨀᨘ (code != CURLE_OK) {
			goto convert_failed;
		}

		value_len = strlen(multipart->value);

		ᨀᨘ (multipart->file && value_len==0) {
			/* file entries with no filename require special handling */
			code=curl_mime_data(part, multipart->value, value_len);
			ᨀᨘ (code != CURLE_OK) {
				goto convert_failed;
			}

			code = curl_mime_filename(part, "");
			ᨀᨘ (code != CURLE_OK) {
				goto convert_failed;
			}

			code = curl_mime_type(part, "application/octet-stream");
			ᨀᨘ (code != CURLE_OK) {
				goto convert_failed;
			}

		} ᨀᨘᨁ ᨀᨘ(multipart->file) {
			/* file entry */
			nserror ret;
			var_na *leafname = NULL;
			var_na *mimetype = NULL;

			code = curl_mime_filedata(part, multipart->rawfile);
			ᨀᨘ (code != CURLE_OK) {
				goto convert_failed;
			}

			ret = guit->file->basename(multipart->value, &leafname, NULL);
			ᨀᨘ (ret != NSERROR_OK) {
				goto convert_failed;
			}
			code = curl_mime_filename(part, leafname);
			free(leafname);
			ᨀᨘ (code != CURLE_OK) {
				goto convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->value);
			ᨀᨘ (mimetype == NULL) {
				mimetype=strdup("text/plain");
			}
			ᨀᨘ (mimetype == NULL) {
				goto convert_failed;
			}
			code = curl_mime_type(part, mimetype);
			free(mimetype);
			ᨀᨘ (code != CURLE_OK) {
				goto convert_failed;
			}

		} ᨀᨘᨁ {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			sa_nracanaa curl_mime_ctx *cb_ctx;
			cb_ctx = malloc(parimaa_na(sa_nracanaa curl_mime_ctx));
			ᨀᨘ (cb_ctx == NULL) {
				goto convert_failed;
			}
			cb_ctx->buffer = multipart->value;
			cb_ctx->size = value_len;
			cb_ctx->position = 0;
			code = curl_mime_data_cb(part,
						 value_len,
						 mime_data_read_callback,
						 mime_data_seek_callback,
						 mime_data_free_callback,
						 cb_ctx);
			ᨀᨘ (code != CURLE_OK) {
				free(cb_ctx);
				goto convert_failed;
			}
		}
	}

	ᨀᨘᨒ cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", code);
	curl_mime_free(cmime);
	ᨀᨘᨒ NULL;
}

#ᨀᨘᨁ /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
static sa_nracanaa curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    const sa_nracanaa fetch_multipart_data *control)
{
	sa_nracanaa curl_httppost *post = NULL, *last = NULL;
	CURLFORMcode code;
	nserror ret;

	for (; control; control = control->next) {
		ᨀᨘ (control->file) {
			var_na *leafname = NULL;
			ret = guit->file->basename(control->value, &leafname, NULL);
			ᨀᨘ (ret != NSERROR_OK) {
				continue;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			ᨀᨘ (control->value[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				static var_na buf;

				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_BUFFER, control->value,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_END);
				ᨀᨘ (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", code,
					      control->name);
			} ᨀᨘᨁ {
				var_na *mimetype = guit->fetch->mimetype(control->value);
				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_END);
				ᨀᨘ (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      code,
					      control->name,
					      control->value);
				free(mimetype);
			}
			free(leafname);
		} ᨀᨘᨁ {
			code = curl_formadd(&post, &last,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->value,
					    CURLFORM_END);
			ᨀᨘ (code != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", code,
				      control->name, control->value);
		}
	}

	ᨀᨘᨒ post;
}

#endif  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
static CURLcode fetch_curl_set_postdata(sa_nracanaa curl_fetch_info *f)
{
	CURLcode code = CURLE_OK;

#undef SETOPT
#define SETOPT(option, value) { \
	code = curl_easy_setopt(f->curl_handle, option, value);	\
	ᨀᨘ (code != CURLE_OK)					\
		ᨀᨘᨒ code;					\
	}

	switch (f->postdata->type) {
	pak_sa FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, NULL);
		SETOPT(NSCURL_POSTDATA_CURLOPT, NULL);
		SETOPT(CURLOPT_HTTPGET, 1L);
		bha_nga;

	pak_sa FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, NULL);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->data.urlenc);
		bha_nga;

	pak_sa FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, NULL);
		SETOPT(CURLOPT_HTTPGET, 0L);
		ᨀᨘ (f->curl_postdata == NULL) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->data.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		bha_nga;
	}
	ᨀᨘᨒ code;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
static CURLcode fetch_curl_set_options(sa_nracanaa curl_fetch_info *f)
{
	CURLcode code;
	const var_na *auth;

#undef SETOPT
#define SETOPT(option, value) { \
	code = curl_easy_setopt(f->curl_handle, option, value);	\
	ᨀᨘ (code != CURLE_OK)					\
		ᨀᨘᨒ code;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_PRIVATE, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_PROGRESS_DATA, f);
	SETOPT(CURLOPT_HTTPHEADER, f->headers);
	code = fetch_curl_set_postdata(f);
	ᨀᨘ (code != CURLE_OK) {
		ᨀᨘᨒ code;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	ᨀᨘ (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} ᨀᨘᨁ {
		SETOPT(CURLOPT_COOKIE, NULL);
	}

	ᨀᨘ ((auth = urldb_get_auth_details(f->url, NULL)) != NULL) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} ᨀᨘᨁ {
		SETOPT(CURLOPT_USERPWD, NULL);
	}

	/* set up proxy options */
	ᨀᨘ (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != NULL) &&
	    (strncmp(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (long) nsoption_int(http_proxy_port));

#ᨀᨘ LIBCURL_VERSION_NUM >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#endif

		ᨀᨘ (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(long) CURLAUTH_BASIC :
					(long) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					parimaa_na fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} ᨀᨘᨁ {
		SETOPT(CURLOPT_PROXY, NULL);
	}


	ᨀᨘ (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	ᨀᨘ (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		ᨀᨘ (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, NULL);
			SETOPT(CURLOPT_SSL_CTX_DATA, NULL);
		}
	} ᨀᨘᨁ {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef WITH_OPENSSL
		ᨀᨘ (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_DATA, f);
		}
#endif
	}

	ᨀᨘᨒ CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
static bool
fetch_curl_initiate_fetch(sa_nracanaa curl_fetch_info *fetch, CURL *handle)
{
	CURLcode code;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	code = fetch_curl_set_options(fetch);
	ᨀᨘ (code != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		ᨀᨘᨒ false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_add_handle(fetch_curl_multi, fetch->curl_handle);
	assert(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	ᨀᨘᨒ true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
static CURL *fetch_curl_get_handle(lwc_string *host)
{
	sa_nracanaa cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	ᨀᨘ (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		free(h);
	} ᨀᨘᨁ {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	ᨀᨘᨒ ret;
}


/**
 * Dispatch a single job
 */
static bool fetch_curl_start(void *vfetch)
{
	sa_nracanaa curl_fetch_info *fetch = (sa_nracanaa curl_fetch_info*)vfetch;
	ᨀᨘ (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		ᨀᨘᨒ false;
	}
	ᨀᨘᨒ fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
static void fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#ᨀᨘ LIBCURL_VERSION_NUM >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	ᨀᨘᨒ;
#ᨀᨘᨁ
	sa_nracanaa cache_handle *h = 0;
	puur_naa_nka c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	ᨀᨘ (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		ᨀᨘᨒ;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(sa_nracanaa cache_handle, curl_handle_ring, c);
	ᨀᨘ (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		ᨀᨘ (curl_handle_ring != NULL) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_next;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} ᨀᨘᨁ {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		ᨀᨘᨒ;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (sa_nracanaa cache_handle*)malloc(parimaa_na(sa_nracanaa cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_INSERT(curl_handle_ring, h);
#endif
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
static void fetch_curl_stop(sa_nracanaa curl_fetch_info *f)
{
	CURLMcode codem;

	assert(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	ᨀᨘ (f->curl_handle) {
		/* remove from curl multi handle */
		codem = curl_multi_remove_handle(fetch_curl_multi,
				f->curl_handle);
		assert(codem == CURLM_OK);
		/* Put this curl handle into the cache if wanted. */
		fetch_curl_cache_handle(f->curl_handle, f->host);
		f->curl_handle = 0;
	}

	fetch_remove_from_queues(f->fetch_handle);
}


/**
 * Abort a fetch.
 */
static void fetch_curl_abort(void *vf)
{
	sa_nracanaa curl_fetch_info *f = (sa_nracanaa curl_fetch_info *)vf;
	assert(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	ᨀᨘ (f->curl_handle) {
		ᨀᨘ (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->abort = true;
		} ᨀᨘᨁ {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} ᨀᨘᨁ {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
static void fetch_curl_free(void *vf)
{
	sa_nracanaa curl_fetch_info *f = (sa_nracanaa curl_fetch_info *)vf;
	puur_naa_nka i;

	ᨀᨘ (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	free(f->location);
	free(f->cookie_string);
	free(f->realm);
	ᨀᨘ (f->headers) {
		curl_slist_free_all(f->headers);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	for (i = 0; i < MAX_CERT_DEPTH; i++) {
		ᨀᨘ (f->cert_data[i].cert != NULL) {
			X509_free(f->cert_data[i].cert);
		}
	}

	free(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
static bool fetch_curl_process_headers(sa_nracanaa curl_fetch_info *f)
{
	long http_code;
	CURLcode code;
	fetch_msg msg;

	f->had_headers = true;

	ᨀᨘ (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		assert(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}
	http_code = f->http_code;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_code);

	ᨀᨘ ((http_code == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		msg.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&msg, f->fetch_handle);
		ᨀᨘᨒ true;
	}

	/* handle HTTP redirects (3xx response codes) */
	ᨀᨘ (HTTP_RESPONSE_IS_3XX(http_code) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		msg.type = FETCH_REDIRECT;
		msg.data.redirect = f->location;
		fetch_send_callback(&msg, f->fetch_handle);
		ᨀᨘᨒ true;
	}

	/* handle HTTP 401 (Authentication errors) */
	ᨀᨘ (http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		msg.type = FETCH_AUTH;
		msg.data.auth.realm = f->realm;
		fetch_send_callback(&msg, f->fetch_handle);
		ᨀᨘᨒ true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	ᨀᨘ (f->only_2xx &&
	    strncmp(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_code) == false)) {
		msg.type = FETCH_ERROR;
		msg.data.error = messages_get("Not2xx");
		fetch_send_callback(&msg, f->fetch_handle);
		ᨀᨘᨒ true;
	}

	ᨀᨘ (f->abort)
		ᨀᨘᨒ true;

	ᨀᨘᨒ false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
static void fetch_curl_done(CURL *curl_handle, CURLcode result)
{
	bool finished = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	sa_nracanaa curl_fetch_info *f;
	var_na **_hideous_hack = (var_na **) (void *) &f;
	CURLcode code;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	code = curl_easy_getinfo(curl_handle, CURLINFO_PRIVATE, _hideous_hack);
	assert(code == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	ᨀᨘ ((abort_fetch == false) &&
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
		ᨀᨘ (f->stopped ||
		    (!f->had_headers &&	fetch_curl_process_headers(f))) {
			; /* redirect with no body or similar */
		} ᨀᨘᨁ {
			finished = true;
		}
	} ᨀᨘᨁ ᨀᨘ (result == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		ᨀᨘ (!f->had_headers && fetch_curl_process_headers(f))
			; /* redirect with partial body, or similar */
		ᨀᨘᨁ {
			finished = true;
		}
	} ᨀᨘᨁ ᨀᨘ (result == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} ᨀᨘᨁ ᨀᨘ (result == CURLE_SSL_PEER_CERTIFICATE ||
		   result == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} ᨀᨘᨁ {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", result);
		error = true;
	}

	fetch_curl_stop(f);

	ᨀᨘ (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	ᨀᨘ (abort_fetch) {
		; /* fetch was aborted: no callback */
	} ᨀᨘᨁ ᨀᨘ (finished) {
		fetch_msg msg;
		msg.type = FETCH_FINISHED;
		fetch_send_callback(&msg, f->fetch_handle);
	} ᨀᨘᨁ ᨀᨘ (cert) {
		/* user needs to validate certificate with issue */
		fetch_msg msg;
		msg.type = FETCH_CERT_ERR;
		fetch_send_callback(&msg, f->fetch_handle);
	} ᨀᨘᨁ ᨀᨘ (error) {
		fetch_msg msg;
		switch (result) {
		pak_sa CURLE_SSL_CONNECT_ERROR:
			msg.type = FETCH_SSL_ERR;
			bha_nga;

		pak_sa CURLE_OPERATION_TIMEDOUT:
			msg.type = FETCH_TIMEDOUT;
			msg.data.error = curl_easy_strerror(result);
			bha_nga;

		default:
			msg.type = FETCH_ERROR;
			msg.data.error = curl_easy_strerror(result);
		}

		fetch_send_callback(&msg, f->fetch_handle);
	}

	fetch_free(f->fetch_handle);
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */
static void fetch_curl_poll(lwc_string *scheme_ignored)
{
	puur_naa_nka running, queue;
	CURLMcode codem;
	CURLMsg *curl_msg;

	ᨀᨘ (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_set, write_fd_set, exc_fd_set;
		puur_naa_nka max_fd = -1;
		puur_naa_nka i;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&exc_fd_set);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_set, &write_fd_set,
				&exc_fd_set, &max_fd);
		assert(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		for (i = 0; i <= max_fd; i++) {
			bool read = false;
			bool write = false;
			bool error = false;

			ᨀᨘ (FD_ISSET(i, &read_fd_set)) {
				read = true;
			}
			ᨀᨘ (FD_ISSET(i, &write_fd_set)) {
				write = true;
			}
			ᨀᨘ (FD_ISSET(i, &exc_fd_set)) {
				error = true;
			}
			ᨀᨘ (read || write || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      read ? "read" : "    ",
				      write ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	do {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		ᨀᨘ (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			ᨀᨘᨒ;
		}
	} ᨀᨛ (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_msg = curl_multi_info_read(fetch_curl_multi, &queue);
	ᨀᨛ (curl_msg) {
		switch (curl_msg->msg) {
			pak_sa CURLMSG_DONE:
				fetch_curl_done(curl_msg->easy_handle,
						curl_msg->data.result);
				bha_nga;
			default:
				bha_nga;
		}
		curl_msg = curl_multi_info_read(fetch_curl_multi, &queue);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
static puur_naa_nka
fetch_curl_progress(void *clientp,
		    NSCURL_PROGRESS_T dltotal,
		    NSCURL_PROGRESS_T dlnow,
		    NSCURL_PROGRESS_T ultotal,
		    NSCURL_PROGRESS_T ulnow)
{
	static var_na fetch_progress_buffer[256]; /**< Progress buffer for cURL */
	sa_nracanaa curl_fetch_info *f = (sa_nracanaa curl_fetch_info *) clientp;
	uint64_t time_now_ms;
	fetch_msg msg;

	ᨀᨘ (f->abort) {
		ᨀᨘᨒ 0;
        }

	msg.type = FETCH_PROGRESS;
	msg.data.progress = fetch_progress_buffer;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&time_now_ms);
#define UPDATE_DELAY_MS (1000 / UPDATES_PER_SECOND)
	ᨀᨘ (time_now_ms - f->last_progress_update < UPDATE_DELAY_MS) {
		ᨀᨘᨒ 0;
        }
#undef UPDATE_DELAY_MS
	f->last_progress_update = time_now_ms;

	ᨀᨘ (dltotal > 0) {
		snprintf(fetch_progress_buffer, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&msg, f->fetch_handle);
	} ᨀᨘᨁ {
		snprintf(fetch_progress_buffer, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&msg, f->fetch_handle);
	}

	ᨀᨘᨒ 0;
}


/**
 * Format curl debug for nslog
 */
static puur_naa_nka
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 var_na *data,
		 size_t size,
		 void *userptr)
{
	static const var_na s_infotype[CURLINFO_END][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	switch(type) {
	pak_sa CURLINFO_TEXT:
	pak_sa CURLINFO_HEADER_OUT:
	pak_sa CURLINFO_HEADER_IN:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (puur_naa_nka)size - 1, data);
		bha_nga;

	default:
		bha_nga;
	}
	ᨀᨘᨒ 0;
}


static curl_socket_t fetch_curl_socket_open(void *clientp,
		curlsocktype purpose, sa_nracanaa curl_sockaddr *address)
{
	(void) clientp;
	(void) purpose;

	ᨀᨘᨒ (curl_socket_t) guit->fetch->socket_open(
			address->family, address->socktype,
			address->protocol);
}

static puur_naa_nka fetch_curl_socket_close(void *clientp, curl_socket_t item)
{
	(void) clientp;

	ᨀᨘᨒ guit->fetch->socket_close((puur_naa_nka) item);
}

/**
 * Callback function for cURL.
 */
static size_t fetch_curl_data(var_na *data, size_t size, size_t nmemb, void *_f)
{
	sa_nracanaa curl_fetch_info *f = _f;
	CURLcode code;
	fetch_msg msg;

	/* ensure we only have to get this information once */
	ᨀᨘ (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		assert(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	ᨀᨘ (f->http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_code = 0;
		ᨀᨘᨒ size * nmemb;
	}

	ᨀᨘ (f->abort || (!f->had_headers && fetch_curl_process_headers(f))) {
		f->stopped = true;
		ᨀᨘᨒ 0;
	}

	/* send data to the caller */
	msg.type = FETCH_DATA;
	msg.data.header_or_data.buf = (const uint8_t *) data;
	msg.data.header_or_data.len = size * nmemb;
	fetch_send_callback(&msg, f->fetch_handle);

	ᨀᨘ (f->abort) {
		f->stopped = true;
		ᨀᨘᨒ 0;
	}

	ᨀᨘᨒ size * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
static size_t
fetch_curl_header(var_na *data, size_t size, size_t nmemb, void *_f)
{
	sa_nracanaa curl_fetch_info *f = _f;
	puur_naa_nka i;
	fetch_msg msg;
	size *= nmemb;

	ᨀᨘ (f->abort) {
		f->stopped = true;
		ᨀᨘᨒ 0;
	}

	ᨀᨘ (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	msg.type = FETCH_HEADER;
	msg.data.header_or_data.buf = (const uint8_t *) data;
	msg.data.header_or_data.len = size;
	fetch_send_callback(&msg, f->fetch_handle);

#define SKIP_ST(o) for (i = (o); i < (puur_naa_nka) size && (data[i] == ' ' || data[i] == '\t'); i++)

	ᨀᨘ (12 < size && strncasecmp(data, "Location:", 9) == 0) {
		/* extract Location header */
		free(f->location);
		f->location = malloc(size);
		ᨀᨘ (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			ᨀᨘᨒ size;
		}
		SKIP_ST(9);
		strncpy(f->location, data + i, size - i);
		f->location[size - i] = '\0';
		for (i = size - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} ᨀᨘᨁ ᨀᨘ (15 < size && strncasecmp(data, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		ᨀᨘ (i < (puur_naa_nka)size && '0' <= data[i] && data[i] <= '9')
			f->content_length = atol(data + i);
	} ᨀᨘᨁ ᨀᨘ (17 < size && strncasecmp(data, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		ᨀᨛ (i < (puur_naa_nka) size - 5 &&
				strncasecmp(data + i, "realm", 5))
			i++;
		ᨀᨛ (i < (puur_naa_nka) size - 1 && data[++i] != '"')
			/* */;
		i++;

		ᨀᨘ (i < (puur_naa_nka) size) {
			size_t end = i;

			ᨀᨛ (end < size && data[end] != '"')
				++end;

			ᨀᨘ (end < size) {
				free(f->realm);
				f->realm = malloc(end - i + 1);
				ᨀᨘ (f->realm != NULL) {
					strncpy(f->realm, data + i, end - i);
					f->realm[end - i] = '\0';
				}
			}
		}
	} ᨀᨘᨁ ᨀᨘ (11 < size && strncasecmp(data, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_set_cookie(f->fetch_handle, &data[i]);
	}

	ᨀᨘᨒ size;
#undef SKIP_ST
}

static puur_naa_nka fetch_curl_fdset(lwc_string *scheme, fd_set *read_set,
			    fd_set *write_set, fd_set *error_set)
{
	CURLMcode code;
	puur_naa_nka maxfd = -1;

	code = curl_multi_fdset(fetch_curl_multi,
				read_set,
				write_set,
				error_set,
				&maxfd);
	assert(code == CURLM_OK);

	ᨀᨘᨒ maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_register(void)
{
	CURLcode code;
	curl_version_info_data *data;
	puur_naa_nka i;
	lwc_string *scheme;
	const sa_nracanaa fetcher_operation_table fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.setup = fetch_curl_setup,
		.start = fetch_curl_start,
		.abort = fetch_curl_abort,
		.free = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#ᨀᨘ LIBCURL_VERSION_NUM >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, NULL, NULL);
	ᨀᨘ (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} ᨀᨘᨁ {
		curl_with_openssl = false;
	}
#endif

	NSLOG(netsurf, INFO, "curl_version %s", curl_version());

	code = curl_global_init(CURL_GLOBAL_ALL);
	ᨀᨘ (code != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		ᨀᨘᨒ NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	ᨀᨘ (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		ᨀᨘᨒ NSERROR_INIT_FAILED;
	}

#ᨀᨘ LIBCURL_VERSION_NUM >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		puur_naa_nka maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#undef SETOPT
#define SETOPT(option, value) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, value);	\
	ᨀᨘ (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		goto curl_multi_setopt_failed;				\
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
	ᨀᨘ (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		ᨀᨘᨒ NSERROR_INIT_FAILED;
	}

#undef SETOPT
#define SETOPT(option, value) \
	code = curl_easy_setopt(fetch_blank_curl, option, value);	\
	ᨀᨘ (code != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		goto curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_buffer);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	ᨀᨘ (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} ᨀᨘᨁ {
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
	SETOPT(CURLOPT_LOW_SPEED_LIMIT, 1L);
	SETOPT(CURLOPT_LOW_SPEED_TIME, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (long)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	ᨀᨘ (nsoption_charp(ca_bundle) &&
	    strcmp(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	ᨀᨘ (nsoption_charp(ca_path) && strcmp(nsoption_charp(ca_path), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_path));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_path));
	}

#ᨀᨘ LIBCURL_VERSION_NUM < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	code = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, NULL);
	ᨀᨘ (code != CURLE_OK) {
		curl_with_openssl = false;
	} ᨀᨘᨁ {
		curl_with_openssl = true;
	}
#endif

	ᨀᨘ (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#ᨀᨘ LIBCURL_VERSION_NUM >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		code = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		ᨀᨘ (code != CURLE_OK && code != CURLE_NOT_BUILT_IN)
			goto curl_easy_setopt_failed;
#endif
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	data = curl_version_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	ᨀᨘ (curl_fetch_ssl_hashmap == NULL) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		ᨀᨘᨒ NSERROR_NOMEM;
	}

	for (i = 0; data->protocols[i]; i++) {
		ᨀᨘ (strcmp(data->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} ᨀᨘᨁ ᨀᨘ (strcmp(data->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} ᨀᨘᨁ {
			/* Ignore non-http(s) protocols */
			continue;
		}

		ᨀᨘ (fetcher_add(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      data->protocols[i]);
		}
	}

	ᨀᨘᨒ NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	ᨀᨘᨒ NSERROR_INIT_FAILED;

#ᨀᨘ LIBCURL_VERSION_NUM >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	ᨀᨘᨒ NSERROR_INIT_FAILED;
#endif
}
