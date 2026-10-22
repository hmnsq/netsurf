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
#અન્તર્ભાવ "utils/inet.h"

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <sys/stat.h>

#અન્તર્ભાવ <libwapcaplet/libwapcaplet.h>
#અન્તર્ભાવ <nsutils/time.h>

#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/hashmap.h"
#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/ring.h"
#અન્તર્ભાવ "utils/useragent.h"
#અન્તર્ભાવ "utils/file.h"
#અન્તર્ભાવ "utils/string.h"
#અન્તર્ભાવ "netsurf/fetch.h"
#અન્તર્ભાવ "netsurf/misc.h"
#અન્તર્ભાવ "desktop/gui_internal.h"

#અન્તર્ભાવ "content/fetch.h"
#અન્તર્ભાવ "content/fetchers.h"
#અન્તર્ભાવ "content/fetchers/curl.h"
#અન્તર્ભાવ "content/urldb.h"

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

#અન્તર્ભાવ <openssl/ssl.h>
#અન્તર્ભાવ <openssl/x509v3.h>

#નહીંતર /* WITH_OPENSSL */

પ્રકારવ્યાખ્યા અક્ષર X509;

સ્થિર શૂન્ય X509_free(X509 *cert)
{
	મુક્ત(cert);
}

#endif /* WITH_OPENSSL */

/* SSL certificate chain cache */

/* We're only interested in the hostname and port */
સ્થિર uint32_t
curl_fetch_ssl_ચાવી_hash(શૂન્ય *ચાવી)
{
	nsurl *url = ચાવી;
	lwc_string *hostname = nsurl_get_component(url, NSURL_HOST);
	lwc_string *port = nsurl_get_component(url, NSURL_PORT);
	uint32_t hash;

	જો (port == ખાલી)
		port = lwc_string_ref(corestring_lwc_443);

	hash = lwc_string_hash_કિંમત(hostname) ^ lwc_string_hash_કિંમત(port);

	lwc_string_unref(hostname);
	lwc_string_unref(port);

	પાછા hash;
}

/* We only compare the hostname and port */
સ્થિર bool
curl_fetch_ssl_ચાવી_eq(શૂન્ય *key1, શૂન્ય *key2)
{
	nsurl *url1 = key1;
	nsurl *url2 = key2;
	lwc_string *hostname1 = nsurl_get_component(url1, NSURL_HOST);
	lwc_string *hostname2 = nsurl_get_component(url2, NSURL_HOST);
	lwc_string *port1 = nsurl_get_component(url1, NSURL_PORT);
	lwc_string *port2 = nsurl_get_component(url2, NSURL_PORT);
	bool iseq = false;

	જો (port1 == ખાલી)
		port1 = lwc_string_ref(corestring_lwc_443);
	જો (port2 == ખાલી)
		port2 = lwc_string_ref(corestring_lwc_443);

	જો (lwc_string_isequal(hostname1, hostname2, &iseq) != lwc_error_ok ||
			iseq == false)
		જાઓ out;

	iseq = false;
	જો (lwc_string_isequal(port1, port2, &iseq) != lwc_error_ok)
		જાઓ out;

out:
	lwc_string_unref(hostname1);
	lwc_string_unref(hostname2);
	lwc_string_unref(port1);
	lwc_string_unref(port2);

	પાછા iseq;
}

સ્થિર શૂન્ય *
curl_fetch_ssl_કિંમત_alloc(શૂન્ય *ચાવી)
{
	સંરચના cert_chain *out;

	જો (cert_chain_alloc(0, &out) != NSERROR_OK) {
		પાછા ખાલી;
	}

	પાછા out;
}

સ્થિર શૂન્ય
curl_fetch_ssl_કિંમત_destroy(શૂન્ય *કિંમત)
{
	સંરચના cert_chain *chain = કિંમત;
	જો (cert_chain_free(chain) != NSERROR_OK) {
		NSLOG(netsurf, WARNING, "Problem freeing SSL certificate chain");
	}
}

સ્થિર hashmap_parameters_t curl_fetch_ssl_hashmap_parameters = {
	.ચાવી_clone = (hashmap_ચાવી_clone_t)nsurl_ref,
	.ચાવી_destroy = (hashmap_ચાવી_destroy_t)nsurl_unref,
	.ચાવી_eq = curl_fetch_ssl_ચાવી_eq,
	.ચાવી_hash = curl_fetch_ssl_ચાવી_hash,
	.કિંમત_alloc = curl_fetch_ssl_કિંમત_alloc,
	.કિંમત_destroy = curl_fetch_ssl_કિંમત_destroy,
};

સ્થિર hashmap_t *curl_fetch_ssl_hashmap = ખાલી;

/** SSL certificate info */
સંરચના cert_info {
	X509 *cert;		/**< Pointer to certificate */
	long err;		/**< OpenSSL error code */
};

#જો LIBCURL_VERSION_અંક >= 0x072000 /* 7.32.0 depricated CURLOPT_PROGRESSFUNCTION*/
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_XFERINFOFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_XFERINFODATA
#define NSCURL_PROGRESS_T curl_off_t
#નહીંતર
#define NSCURLOPT_PROGRESS_FUNCTION CURLOPT_PROGRESSFUNCTION
#define NSCURLOPT_PROGRESS_DATA CURLOPT_PROGRESSDATA
#define NSCURL_PROGRESS_T દ્વિગુણ
#endif

#જો LIBCURL_VERSION_અંક >= 0x073800 /* 7.56.0 depricated curl_formadd */
#define NSCURL_POSTDATA_T curl_mime
#define NSCURL_POSTDATA_CURLOPT CURLOPT_MIMEPOST
#define NSCURL_POSTDATA_FREE(x) curl_mime_free(x)
#નહીંતર
#define NSCURL_POSTDATA_T સંરચના curl_httppost
#define NSCURL_POSTDATA_CURLOPT CURLOPT_HTTPPOST
#define NSCURL_POSTDATA_FREE(x) curl_formfree(x)
#endif

/** Information for a single fetch. */
સંરચના curl_fetch_info {
	સંરચના fetch *fetch_handle; /**< The fetch handle we're parented by. */
	CURL * curl_handle;	/**< cURL handle if being fetched, or 0. */
	bool sent_ssl_chain;	/**< Have we tried to send the SSL chain */
	bool had_headers;	/**< Headers have been processed. */
	bool abort;		/**< Abort requested. */
	bool stopped;		/**< Download stopped on purpose. */
	bool only_2xx;		/**< Only HTTP 2xx responses acceptable. */
	bool downgrade_tls;	/**< Downgrade to TLS 1.2 */
	nsurl *url;		/**< URL of this fetch. */
	lwc_string *host;	/**< The hostname of this fetch. */
	સંરચના curl_slist *headers;	/**< List of request headers. */
	અક્ષર *location;		/**< Response Location header, or 0. */
	unsigned long content_length;	/**< Response Content-Length, or 0. */
	અક્ષર *cookie_string;	/**< Cookie string for this fetch */
	અક્ષર *realm;		/**< HTTP Auth Realm */
	સંરચના fetch_postdata *postdata; /**< POST data */
	NSCURL_POSTDATA_T *curl_postdata; /**< POST data in curl representation */

	long http_code; /**< HTTP result code from cURL. */

	uint64_t last_progress_update;	/**< Time of last progress update */
	પૂર્ણાંક cert_depth; /**< deepest certificate in use */
	સંરચના cert_info cert_data[MAX_CERT_DEPTH]; /**< HTTPS certificate data */
};

/** curl handle cache entry */
સંરચના cache_handle {
	CURL *handle; /**< The cached cURL handle */
	lwc_string *host; /**< The host for which this handle is cached */

	સંરચના cache_handle *r_prev; /**< Previous cached handle in ring. */
	સંરચના cache_handle *r_next; /**< Next cached handle in ring. */
};

/** Global cURL multi handle. */
CURLM *fetch_curl_multi;

/** Curl handle with default options set; not used for transfers. */
સ્થિર CURL *fetch_blank_curl;

/** Ring of cached handles */
સ્થિર સંરચના cache_handle *curl_handle_ring = 0;

/** Count of how many schemes the curl fetcher is handling */
સ્થિર પૂર્ણાંક curl_fetchers_registered = 0;

/** Flag for runtime detection of openssl usage */
સ્થિર bool curl_with_openssl;

/** Error buffer for cURL. */
સ્થિર અક્ષર fetch_error_buffer[CURL_ERROR_SIZE];

/** Proxy authentication details. */
સ્થિર અક્ષર fetch_proxy_userpwd[100];

/** Interlock to prevent initiation during callbacks */
સ્થિર bool inside_curl = false;


/**
 * Initialise a cURL fetcher.
 */
સ્થિર bool fetch_curl_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "Initialise cURL fetcher for %s",
	      lwc_string_data(scheme));
	curl_fetchers_registered++;
	પાછા true; /* Always succeeds */
}


/**
 * Finalise a cURL fetcher.
 *
 * \param scheme The scheme to finalise.
 */
સ્થિર શૂન્ય fetch_curl_finalise(lwc_string *scheme)
{
	સંરચના cache_handle *h;

	curl_fetchers_registered--;
	NSLOG(netsurf, INFO, "Finalise cURL fetcher %s",
	      lwc_string_data(scheme));
	જો (curl_fetchers_registered == 0) {
		CURLMcode codem;
		/* All the fetchers have been finalised. */
		NSLOG(netsurf, INFO,
		      "All cURL fetchers finalised, closing down cURL");

		curl_easy_cleanup(fetch_blank_curl);

		codem = curl_multi_cleanup(fetch_curl_multi);
		જો (codem != CURLM_OK)
			NSLOG(netsurf, INFO,
			      "curl_multi_cleanup failed: ignoring");

		curl_global_cleanup();

		NSLOG(netsurf, DEBUG, "Cleaning up SSL cert chain hashmap");
		hashmap_destroy(curl_fetch_ssl_hashmap);
		curl_fetch_ssl_hashmap = ખાલી;
	}

	/* Free anything remaining in the cached curl handle ring */
	જ્યારે (curl_handle_ring != ખાલી) {
		h = curl_handle_ring;
		RING_REMOVE(curl_handle_ring, h);
		lwc_string_unref(h->host);
		curl_easy_cleanup(h->handle);
		મુક્ત(h);
	}
}


/**
 * Check if this fetcher can fetch a url.
 *
 * \param url The url to check.
 * \return true if the fetcher supports the url else false.
 */
સ્થિર bool fetch_curl_can_fetch(અચલ nsurl *url)
{
	પાછા nsurl_has_component(url, NSURL_HOST);
}



/**
 * allocate postdata
 */
સ્થિર સંરચના fetch_postdata *
fetch_curl_alloc_postdata(અચલ અક્ષર *post_urlenc,
			  અચલ સંરચના fetch_multipart_data *post_multipart)
{
	સંરચના fetch_postdata *postdata;
	postdata = calloc(1, કદ(સંરચના fetch_postdata));
	જો (postdata != ખાલી) {

		જો (post_urlenc) {
			postdata->type = FETCH_POSTDATA_URLENC;
			postdata->data.urlenc = strdup(post_urlenc);
			જો (postdata->data.urlenc == ખાલી) {
				મુક્ત(postdata);
				postdata = ખાલી;
			}
		} નહીંતર જો (post_multipart) {
			postdata->type = FETCH_POSTDATA_MULTIPART;
			postdata->data.multipart = fetch_multipart_data_clone(post_multipart);
			જો (postdata->data.multipart == ખાલી) {
				મુક્ત(postdata);
				postdata = ખાલી;
			}
		} નહીંતર {
			postdata->type = FETCH_POSTDATA_NONE;
		}
	}
	પાછા postdata;
}

/**
 * free postdata
 */
સ્થિર શૂન્ય fetch_curl_free_postdata(સંરચના fetch_postdata *postdata)
{
	જો (postdata != ખાલી) {
		પસંદગી (postdata->type) {
		સ્થિતિ FETCH_POSTDATA_NONE:
			તોડો;
		સ્થિતિ FETCH_POSTDATA_URLENC:
			મુક્ત(postdata->data.urlenc);
			તોડો;
		સ્થિતિ FETCH_POSTDATA_MULTIPART:
			fetch_multipart_data_destroy(postdata->data.multipart);
			તોડો;
		}

		મુક્ત(postdata);
	}
}

/**
 *construct a new fetch structure
 */
સ્થિર સંરચના curl_fetch_info *fetch_alloc(શૂન્ય)
{
	સંરચના curl_fetch_info *fetch;
	fetch = malloc(કદ (*fetch));
	જો (fetch == ખાલી)
		પાછા ખાલી;

	fetch->curl_handle = ખાલી;
	fetch->sent_ssl_chain = false;
	fetch->had_headers = false;
	fetch->abort = false;
	fetch->stopped = false;
	fetch->only_2xx = false;
	fetch->downgrade_tls = false;
	fetch->headers = ખાલી;
	fetch->url = ખાલી;
	fetch->host = ખાલી;
	fetch->location = ખાલી;
	fetch->content_length = 0;
	fetch->http_code = 0;
	fetch->cookie_string = ખાલી;
	fetch->realm = ખાલી;
	fetch->last_progress_update = 0;
	fetch->postdata = ખાલી;
	fetch->curl_postdata = ખાલી;

	/* Clear certificate chain data */
	memset(fetch->cert_data, 0, કદ(fetch->cert_data));
	fetch->cert_depth = -1;

	પાછા fetch;
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
સ્થિર શૂન્ય *
fetch_curl_setup(સંરચના fetch *parent_fetch,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 અચલ અક્ષર *post_urlenc,
		 અચલ સંરચના fetch_multipart_data *post_multipart,
		 અચલ અક્ષર **headers)
{
	સંરચના curl_fetch_info *fetch;
	સંરચના curl_slist *slist;
	પૂર્ણાંક i;

	fetch = fetch_alloc();
	જો (fetch == ખાલી)
		પાછા ખાલી;

	NSLOG(netsurf, INFO, "fetch %p, url '%s'", fetch, nsurl_access(url));

	fetch->only_2xx = only_2xx;
	fetch->downgrade_tls = downgrade_tls;
	fetch->fetch_handle = parent_fetch;
	fetch->url = nsurl_ref(url);
	fetch->host = nsurl_get_component(url, NSURL_HOST);
	જો (fetch->host == ખાલી) {
		જાઓ failed;
	}
	fetch->postdata = fetch_curl_alloc_postdata(post_urlenc, post_multipart);
	જો (fetch->postdata == ખાલી) {
		જાઓ failed;
	}

#define APPEND(list, કિંમત) \
	slist = curl_slist_append(list, કિંમત);		\
	જો (slist == ખાલી)				\
		જાઓ failed;				\
	list = slist;

	/* remove curl default headers */
	APPEND(fetch->headers, "Pragma:");

	/* when doing a POST libcurl sends Expect: 100-continue" by default
	 * which fails with lighttpd, so disable it (see bug 1429054) */
	APPEND(fetch->headers, "Expect:");

	જો ((nsoption_charp(accept_language) != ખાલી) &&
	    (nsoption_charp(accept_language)[0] != '\0')) {
		અક્ષર s[80];
		snprintf(s, કદ s, "Accept-Language: %s, *;q=0.1",
			 nsoption_charp(accept_language));
		s[કદ s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	જો (nsoption_charp(accept_charset) != ખાલી &&
	    nsoption_charp(accept_charset)[0] != '\0') {
		અક્ષર s[80];
		snprintf(s, કદ s, "Accept-Charset: %s, *;q=0.1",
			 nsoption_charp(accept_charset));
		s[કદ s - 1] = 0;
		APPEND(fetch->headers, s);
	}

	જો (nsoption_bool(do_not_track) == true) {
		APPEND(fetch->headers, "DNT: 1");
	}

	/* And add any headers specified by the caller */
	માટે (i = 0; headers[i] != ખાલી; i++) {
		APPEND(fetch->headers, headers[i]);
	}

	પાછા fetch;

#undef APPEND

failed:
	lwc_string_unref(fetch->host);
	nsurl_unref(fetch->url);
	fetch_curl_free_postdata(fetch->postdata);
	curl_slist_free_all(fetch->headers);
	મુક્ત(fetch);
	પાછા ખાલી;
}


#ifdef WITH_OPENSSL

/**
 * Retrieve the ssl cert chain for the fetch, creating a blank one if needed
 */
સ્થિર સંરચના cert_chain *
fetch_curl_get_cached_chain(સંરચના curl_fetch_info *f)
{
	સંરચના cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);
	જો (chain == ખાલી) {
		chain = hashmap_ઉમેરો(curl_fetch_ssl_hashmap, f->url);
	}

	પાછા chain;
}

/**
 * Report the certificate information in the fetch to the users
 */
સ્થિર શૂન્ય
fetch_curl_store_certs_in_cache(સંરચના curl_fetch_info *f)
{
	size_t depth;
	BIO *mem;
	BUF_MEM *buf[MAX_CERT_DEPTH];
	સંરચના cert_chain chain, *cached_chain;
	સંરચના cert_info *certs;

	memset(&chain, 0, કદ(chain));

	certs = f->cert_data;
	chain.depth = f->cert_depth + 1; /* 0 indexed certificate depth */

	માટે (depth = 0; depth < chain.depth; depth++) {
		જો (certs[depth].cert == ખાલી) {
			/* This certificate is missing, skip it */
			chain.certs[depth].err = SSL_CERT_ERR_CERT_MISSING;
			ચાલુ;
		}

		/* error code (if any) */
		પસંદગી (certs[depth].err) {
		સ્થિતિ X509_V_OK:
			chain.certs[depth].err = SSL_CERT_ERR_OK;
			તોડો;

		સ્થિતિ X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			/* fallthrough */
		સ્થિતિ X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_ચાવી:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_ISSUER;
			તોડો;

		સ્થિતિ X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			/* fallthrough */
		સ્થિતિ X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			/* fallthrough */
		સ્થિતિ X509_V_ERR_CERT_SIGNATURE_FAILURE:
			/* fallthrough */
		સ્થિતિ X509_V_ERR_CRL_SIGNATURE_FAILURE:
			chain.certs[depth].err = SSL_CERT_ERR_BAD_SIG;
			તોડો;

		સ્થિતિ X509_V_ERR_CERT_NOT_YET_VALID:
			/* fallthrough */
		સ્થિતિ X509_V_ERR_CRL_NOT_YET_VALID:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_YOUNG;
			તોડો;

		સ્થિતિ X509_V_ERR_CERT_HAS_EXPIRED:
			/* fallthrough */
		સ્થિતિ X509_V_ERR_CRL_HAS_EXPIRED:
			chain.certs[depth].err = SSL_CERT_ERR_TOO_OLD;
			તોડો;

		સ્થિતિ X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			chain.certs[depth].err = SSL_CERT_ERR_SELF_SIGNED;
			તોડો;

		સ્થિતિ X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			chain.certs[depth].err = SSL_CERT_ERR_CHAIN_SELF_SIGNED;
			તોડો;

		સ્થિતિ X509_V_ERR_CERT_REVOKED:
			chain.certs[depth].err = SSL_CERT_ERR_REVOKED;
			તોડો;

		સ્થિતિ X509_V_ERR_HOSTNAME_MISMATCH:
			chain.certs[depth].err = SSL_CERT_ERR_HOSTNAME_MISMATCH;
			તોડો;

		મૂળભૂત:
			chain.certs[depth].err = SSL_CERT_ERR_UNKNOWN;
			તોડો;
		}

		/*
		 * get certificate in Distinguished Encoding Rules (DER) format.
		 */
		mem = BIO_new(BIO_s_mem());
		i2d_X509_bio(mem, certs[depth].cert);
		BIO_get_mem_ptr(mem, &buf[depth]);
		(શૂન્ય) BIO_set_close(mem, BIO_NOCLOSE);
		BIO_free(mem);

		chain.certs[depth].der = (uint8_t *)buf[depth]->data;
		chain.certs[depth].der_length = buf[depth]->length;
	}

	/* Now dup that chain into the cache */
	cached_chain = fetch_curl_get_cached_chain(f);
	જો (cert_chain_dup_into(&chain, cached_chain) != NSERROR_OK) {
		/* Something went wrong storing the chain, give up */
		hashmap_remove(curl_fetch_ssl_hashmap, f->url);
	}

	/* release the openssl memory buffer */
	માટે (depth = 0; depth < chain.depth; depth++) {
		જો (chain.certs[depth].err == SSL_CERT_ERR_CERT_MISSING) {
			ચાલુ;
		}
		જો (buf[depth] != ખાલી) {
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
સ્થિર પૂર્ણાંક
fetch_curl_verify_callback(પૂર્ણાંક verify_ok, X509_STORE_CTX *x509_ctx)
{
	પૂર્ણાંક depth;
	સંરચના curl_fetch_info *fetch;

	depth = X509_STORE_CTX_get_error_depth(x509_ctx);
	fetch = X509_STORE_CTX_get_app_data(x509_ctx);

	/* certificate chain is excessively deep so fail verification */
	જો (depth >= MAX_CERT_DEPTH) {
		X509_STORE_CTX_set_error(x509_ctx,
					 X509_V_ERR_CERT_CHAIN_TOO_LONG);
		પાછા 0;
	}

	/* record the max depth */
	જો (depth > fetch->cert_depth) {
		fetch->cert_depth = depth;
	}

	/* save the certificate by incrementing the reference count and
	 * keeping a pointer.
	 */
	જો (!fetch->cert_data[depth].cert) {
		fetch->cert_data[depth].cert = X509_STORE_CTX_get_current_cert(x509_ctx);
		X509_up_ref(fetch->cert_data[depth].cert);
		fetch->cert_data[depth].err = X509_STORE_CTX_get_error(x509_ctx);
	}

	/* allow certificate chain to be completed */
	જો (depth > 0) {
		verify_ok = 1;
	} નહીંતર {
		/* search for deeper certificates in the chain with errors */
		માટે (depth = fetch->cert_depth; depth > 0; depth--) {
			જો (fetch->cert_data[depth].err != 0) {
				/* error in previous certificate so fail verification */
				verify_ok = 0;
				X509_STORE_CTX_set_error(x509_ctx, fetch->cert_data[depth].err);
			}
		}
	}

	પાછા verify_ok;
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
સ્થિર પૂર્ણાંક fetch_curl_cert_verify_callback(X509_STORE_CTX *x509_ctx, શૂન્ય *parm)
{
	સંરચના curl_fetch_info *f = (સંરચના curl_fetch_info *) parm;
	પૂર્ણાંક ok;
	X509_VERIFY_PARAM *vparam;

	/* Configure the verification parameters to include hostname */
	vparam = X509_STORE_CTX_get0_param(x509_ctx);
	X509_VERIFY_PARAM_set_hostflags(vparam, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

	ok = X509_VERIFY_PARAM_set1_host(vparam,
					 lwc_string_data(f->host),
					 lwc_string_length(f->host));

	/* Store fetch struct in context for verify callback */
	જો (ok) {
		ok = X509_STORE_CTX_set_app_data(x509_ctx, parm);
	}

	/* verify the certificate chain using standard call */
	જો (ok) {
		ok = X509_verify_cert(x509_ctx);
	}

	fetch_curl_store_certs_in_cache(f);

	પાછા ok;
}


/**
 * cURL SSL setup callback
 *
 * \param curl_handle The curl handle to perform the ssl operation on.
 * \param _sslctx The ssl context.
 * \param parm The callback context.
 * \return A curl result code.
 */
સ્થિર CURLcode
fetch_curl_sslctxfun(CURL *curl_handle, શૂન્ય *_sslctx, શૂન્ય *parm)
{
	સંરચના curl_fetch_info *f = (સંરચના curl_fetch_info *) parm;
	SSL_CTX *sslctx = _sslctx;
	long options = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;

	/* set verify callback for each certificate in chain */
	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, fetch_curl_verify_callback);

	/* set callback used to verify certificate chain */
	SSL_CTX_set_cert_verify_callback(sslctx,
					 fetch_curl_cert_verify_callback,
					 parm);

	જો (f->downgrade_tls) {
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

	પાછા CURLE_OK;
}


#endif /* WITH_OPENSSL */


/**
 * Report the certificate information in the fetch to the users
 */
સ્થિર શૂન્ય
fetch_curl_report_certs_upstream(સંરચના curl_fetch_info *f)
{
	fetch_સંદેશ સંદેશ;
	સંરચના cert_chain *chain;

	chain = hashmap_lookup(curl_fetch_ssl_hashmap, f->url);

	જો (chain != ખાલી) {
		સંદેશ.type = FETCH_CERTS;
		સંદેશ.data.chain = chain;

		fetch_send_callback(&સંદેશ, f->fetch_handle);
	}

	f->sent_ssl_chain = true;
}

#જો LIBCURL_VERSION_અંક >= 0x073800 /* 7.56.0 depricated curl_formadd */

/**
 * curl mime data context
 */
સંરચના curl_mime_ctx {
	અક્ષર *buffer;
	curl_off_t size;
	curl_off_t position;
};

સ્થિર size_t mime_data_read_callback(અક્ષર *buffer, size_t size, size_t nitems, શૂન્ય *arg)
{
	સંરચના curl_mime_ctx *mctx = (સંરચના curl_mime_ctx *) arg;
	curl_off_t sz = mctx->size - mctx->position;

	nitems *= size;
	જો(sz > (curl_off_t)nitems) {
		sz = nitems;
	}
	જો(sz) {
		memcpy(buffer, mctx->buffer + mctx->position, sz);
	}
	mctx->position += sz;
	પાછા sz;
}

સ્થિર પૂર્ણાંક mime_data_seek_callback(શૂન્ય *arg, curl_off_t offset, પૂર્ણાંક origin)
{
	સંરચના curl_mime_ctx *mctx = (સંરચના curl_mime_ctx *) arg;

	પસંદગી(origin) {
	સ્થિતિ SEEK_END:
		offset += mctx->size;
		તોડો;
	સ્થિતિ SEEK_CUR:
		offset += mctx->position;
		તોડો;
	}

	જો(offset < 0) {
		પાછા CURL_SEEKFUNC_FAIL;
	}
	mctx->position = offset;
	પાછા CURL_SEEKFUNC_OK;
}

સ્થિર શૂન્ય mime_data_free_callback(શૂન્ય *arg)
{
	સંરચના curl_mime_ctx *mctx = (સંરચના curl_mime_ctx *) arg;
	મુક્ત(mctx);
}

/**
 * Convert a POST data list to a libcurl curl_mime.
 *
 * \param chandle curl fetch handle.
 * \param multipart limked list of struct ::fetch_multipart forming post data.
 */
સ્થિર curl_mime *
fetch_curl_postdata_convert(CURL *chandle,
			    અચલ સંરચના fetch_multipart_data *multipart)
{
	curl_mime *cmime;
	curl_mimepart *part;
	CURLcode code = CURLE_OK;
	size_t કિંમત_len;

	cmime = curl_mime_init(chandle);
	જો (cmime == ખાલી) {
		NSLOG(netsurf, WARNING, "postdata conversion failed to curl mime context");
		પાછા ખાલી;
	}

	/* iterate post data */
	માટે (; multipart != ખાલી; multipart = multipart->next) {
		part = curl_mime_addpart(cmime);
		જો (part == ખાલી) {
			જાઓ convert_failed;
		}

		code = curl_mime_name(part, multipart->name);
		જો (code != CURLE_OK) {
			જાઓ convert_failed;
		}

		કિંમત_len = strlen(multipart->કિંમત);

		જો (multipart->file && કિંમત_len==0) {
			/* file entries with no filename require special handling */
			code=curl_mime_data(part, multipart->કિંમત, કિંમત_len);
			જો (code != CURLE_OK) {
				જાઓ convert_failed;
			}

			code = curl_mime_filename(part, "");
			જો (code != CURLE_OK) {
				જાઓ convert_failed;
			}

			code = curl_mime_type(part, "application/octet-stream");
			જો (code != CURLE_OK) {
				જાઓ convert_failed;
			}

		} નહીંતર જો(multipart->file) {
			/* file entry */
			nserror ret;
			અક્ષર *leafname = ખાલી;
			અક્ષર *mimetype = ખાલી;

			code = curl_mime_filedata(part, multipart->rawfile);
			જો (code != CURLE_OK) {
				જાઓ convert_failed;
			}

			ret = guit->file->basename(multipart->કિંમત, &leafname, ખાલી);
			જો (ret != NSERROR_OK) {
				જાઓ convert_failed;
			}
			code = curl_mime_filename(part, leafname);
			મુક્ત(leafname);
			જો (code != CURLE_OK) {
				જાઓ convert_failed;
			}

			mimetype = guit->fetch->mimetype(multipart->કિંમત);
			જો (mimetype == ખાલી) {
				mimetype=strdup("text/plain");
			}
			જો (mimetype == ખાલી) {
				જાઓ convert_failed;
			}
			code = curl_mime_type(part, mimetype);
			મુક્ત(mimetype);
			જો (code != CURLE_OK) {
				જાઓ convert_failed;
			}

		} નહીંતર {
			/* make the curl mime reference the existing multipart
			 * data which requires use of a callback and context.
			 */
			સંરચના curl_mime_ctx *cb_ctx;
			cb_ctx = malloc(કદ(સંરચના curl_mime_ctx));
			જો (cb_ctx == ખાલી) {
				જાઓ convert_failed;
			}
			cb_ctx->buffer = multipart->કિંમત;
			cb_ctx->size = કિંમત_len;
			cb_ctx->position = 0;
			code = curl_mime_data_cb(part,
						 કિંમત_len,
						 mime_data_read_callback,
						 mime_data_seek_callback,
						 mime_data_free_callback,
						 cb_ctx);
			જો (code != CURLE_OK) {
				મુક્ત(cb_ctx);
				જાઓ convert_failed;
			}
		}
	}

	પાછા cmime;

convert_failed:
	NSLOG(netsurf, WARNING, "postdata conversion failed with curl code: %d", code);
	curl_mime_free(cmime);
	પાછા ખાલી;
}

#નહીંતર /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Convert a list of struct ::fetch_multipart_data to a list of
 * struct curl_httppost for libcurl.
 */
સ્થિર સંરચના curl_httppost *
fetch_curl_postdata_convert(CURL *chandle,
			    અચલ સંરચના fetch_multipart_data *control)
{
	સંરચના curl_httppost *post = ખાલી, *last = ખાલી;
	CURLFORMcode code;
	nserror ret;

	માટે (; control; control = control->next) {
		જો (control->file) {
			અક્ષર *leafname = ખાલી;
			ret = guit->file->basename(control->કિંમત, &leafname, ખાલી);
			જો (ret != NSERROR_OK) {
				ચાલુ;
			}

			/* We have to special case filenames of "", so curl
			 * a) actually attempts the fetch and
			 * b) doesn't attempt to open the file ""
			 */
			જો (control->કિંમત[0] == '\0') {
				/* dummy buffer - needs to be static so
				 * pointer's still valid when we go out
				 * of scope (not that libcurl should be
				 * attempting to access it, of course).
				 */
				સ્થિર અક્ષર buf;

				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_BUFFER, control->કિંમત,
						    /* needed, as basename("") == "." */
						    CURLFORM_FILENAME, "",
						    CURLFORM_BUFFERPTR, &buf,
						    CURLFORM_BUFFERLENGTH, 0,
						    CURLFORM_CONTENTTYPE,
						    "application/octet-stream",
						    CURLFORM_END);
				જો (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s)", code,
					      control->name);
			} નહીંતર {
				અક્ષર *mimetype = guit->fetch->mimetype(control->કિંમત);
				code = curl_formadd(&post, &last,
						    CURLFORM_COPYNAME, control->name,
						    CURLFORM_FILE, control->rawfile,
						    CURLFORM_FILENAME, leafname,
						    CURLFORM_CONTENTTYPE,
						    (mimetype != 0 ? mimetype : "text/plain"),
						    CURLFORM_END);
				જો (code != CURL_FORMADD_OK)
					NSLOG(netsurf, INFO,
					      "curl_formadd: %d (%s=%s)",
					      code,
					      control->name,
					      control->કિંમત);
				મુક્ત(mimetype);
			}
			મુક્ત(leafname);
		} નહીંતર {
			code = curl_formadd(&post, &last,
					    CURLFORM_COPYNAME, control->name,
					    CURLFORM_COPYCONTENTS, control->કિંમત,
					    CURLFORM_END);
			જો (code != CURL_FORMADD_OK)
				NSLOG(netsurf, INFO,
				      "curl_formadd: %d (%s=%s)", code,
				      control->name, control->કિંમત);
		}
	}

	પાછા post;
}

#endif  /* LIBCURL_VERSION_NUM >= 0x073800 */

/**
 * Setup multipart post data
 */
સ્થિર CURLcode fetch_curl_set_postdata(સંરચના curl_fetch_info *f)
{
	CURLcode code = CURLE_OK;

#undef SETOPT
#define SETOPT(option, કિંમત) { \
	code = curl_easy_setopt(f->curl_handle, option, કિંમત);	\
	જો (code != CURLE_OK)					\
		પાછા code;					\
	}

	પસંદગી (f->postdata->type) {
	સ્થિતિ FETCH_POSTDATA_NONE:
		SETOPT(CURLOPT_POSTFIELDS, ખાલી);
		SETOPT(NSCURL_POSTDATA_CURLOPT, ખાલી);
		SETOPT(CURLOPT_HTTPGET, 1L);
		તોડો;

	સ્થિતિ FETCH_POSTDATA_URLENC:
		SETOPT(NSCURL_POSTDATA_CURLOPT, ખાલી);
		SETOPT(CURLOPT_HTTPGET, 0L);
		SETOPT(CURLOPT_POSTFIELDS, f->postdata->data.urlenc);
		તોડો;

	સ્થિતિ FETCH_POSTDATA_MULTIPART:
		SETOPT(CURLOPT_POSTFIELDS, ખાલી);
		SETOPT(CURLOPT_HTTPGET, 0L);
		જો (f->curl_postdata == ખાલી) {
			f->curl_postdata =
				fetch_curl_postdata_convert(f->curl_handle,
							    f->postdata->data.multipart);
		}
		SETOPT(NSCURL_POSTDATA_CURLOPT, f->curl_postdata);
		તોડો;
	}
	પાછા code;
}

/**
 * Set options specific for a fetch.
 *
 * \param f The fetch to set options on.
 * \return A curl result code.
 */
સ્થિર CURLcode fetch_curl_set_options(સંરચના curl_fetch_info *f)
{
	CURLcode code;
	અચલ અક્ષર *auth;

#undef SETOPT
#define SETOPT(option, કિંમત) { \
	code = curl_easy_setopt(f->curl_handle, option, કિંમત);	\
	જો (code != CURLE_OK)					\
		પાછા code;					\
	}

	SETOPT(CURLOPT_URL, nsurl_access(f->url));
	SETOPT(CURLOPT_PRIVATE, f);
	SETOPT(CURLOPT_WRITEDATA, f);
	SETOPT(CURLOPT_WRITEHEADER, f);
	SETOPT(NSCURLOPT_PROGRESS_DATA, f);
	SETOPT(CURLOPT_HTTPHEADER, f->headers);
	code = fetch_curl_set_postdata(f);
	જો (code != CURLE_OK) {
		પાછા code;
	}

	f->cookie_string = urldb_get_cookie(f->url, true);
	જો (f->cookie_string) {
		SETOPT(CURLOPT_COOKIE, f->cookie_string);
	} નહીંતર {
		SETOPT(CURLOPT_COOKIE, ખાલી);
	}

	જો ((auth = urldb_get_auth_details(f->url, ખાલી)) != ખાલી) {
		SETOPT(CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		SETOPT(CURLOPT_USERPWD, auth);
	} નહીંતર {
		SETOPT(CURLOPT_USERPWD, ખાલી);
	}

	/* set up proxy options */
	જો (nsoption_bool(http_proxy) &&
	    (nsoption_charp(http_proxy_host) != ખાલી) &&
	    (strncmp(nsurl_access(f->url), "file:", 5) != 0)) {
		SETOPT(CURLOPT_PROXY, nsoption_charp(http_proxy_host));
		SETOPT(CURLOPT_PROXYPORT, (long) nsoption_int(http_proxy_port));

#જો LIBCURL_VERSION_અંક >= 0x071304
		/* Added in 7.19.4 */
		/* setup the omission list */
		SETOPT(CURLOPT_NOPROXY, nsoption_charp(http_proxy_noproxy));
#endif

		જો (nsoption_int(http_proxy_auth) != OPTION_HTTP_PROXY_AUTH_NONE) {
			SETOPT(CURLOPT_PROXYAUTH,
			       nsoption_int(http_proxy_auth) ==
					OPTION_HTTP_PROXY_AUTH_BASIC ?
					(long) CURLAUTH_BASIC :
					(long) CURLAUTH_NTLM);
			snprintf(fetch_proxy_userpwd,
					કદ fetch_proxy_userpwd,
					"%s:%s",
				 nsoption_charp(http_proxy_auth_user),
				 nsoption_charp(http_proxy_auth_pass));
			SETOPT(CURLOPT_PROXYUSERPWD, fetch_proxy_userpwd);
		}
	} નહીંતર {
		SETOPT(CURLOPT_PROXY, ખાલી);
	}


	જો (curl_with_openssl) {
		SETOPT(CURLOPT_SSL_CIPHER_LIST,
			f->downgrade_tls ? CIPHER_LIST_LEGACY : CIPHER_LIST);
	}

	/* Force-enable SSL session ID caching, as some distros are odd. */
	SETOPT(CURLOPT_SSL_SESSIONID_CACHE, 1L);

	જો (urldb_get_cert_permissions(f->url)) {
		/* Disable certificate verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
		જો (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, ખાલી);
			SETOPT(CURLOPT_SSL_CTX_DATA, ખાલી);
		}
	} નહીંતર {
		/* do verification */
		SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
		SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef WITH_OPENSSL
		જો (curl_with_openssl) {
			SETOPT(CURLOPT_SSL_CTX_FUNCTION, fetch_curl_sslctxfun);
			SETOPT(CURLOPT_SSL_CTX_DATA, f);
		}
#endif
	}

	પાછા CURLE_OK;
}

/**
 * Initiate a fetch from the queue.
 *
 * \param fetch fetch to use to fetch content.
 * \param handle CURL handle to be used to fetch the content.
 * \return true if the fetch was successfully initiated else false.
 */
સ્થિર bool
fetch_curl_initiate_fetch(સંરચના curl_fetch_info *fetch, CURL *handle)
{
	CURLcode code;
	CURLMcode codem;

	fetch->curl_handle = handle;

	/* Initialise the handle */
	code = fetch_curl_set_options(fetch);
	જો (code != CURLE_OK) {
		fetch->curl_handle = 0;
		/* The handle maybe went bad, eat it */
		NSLOG(netsurf, WARNING, "cURL handle maybe went bad, retry later");
		curl_easy_cleanup(handle);
		પાછા false;
	}

	/* add to the global curl multi handle */
	codem = curl_multi_add_handle(fetch_curl_multi, fetch->curl_handle);
	પ્રતિજ્ઞા(codem == CURLM_OK || codem == CURLM_CALL_MULTI_PERFORM);

	પાછા true;
}


/**
 * Find a CURL handle to use to dispatch a job
 */
સ્થિર CURL *fetch_curl_get_handle(lwc_string *host)
{
	સંરચના cache_handle *h;
	CURL *ret;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	જો (h) {
		ret = h->handle;
		lwc_string_unref(h->host);
		RING_REMOVE(curl_handle_ring, h);
		મુક્ત(h);
	} નહીંતર {
		ret = curl_easy_duphandle(fetch_blank_curl);
	}
	પાછા ret;
}


/**
 * Dispatch a single job
 */
સ્થિર bool fetch_curl_start(શૂન્ય *vfetch)
{
	સંરચના curl_fetch_info *fetch = (સંરચના curl_fetch_info*)vfetch;
	જો (inside_curl) {
		NSLOG(netsurf, DEBUG, "Deferring fetch because we're inside cURL");
		પાછા false;
	}
	પાછા fetch_curl_initiate_fetch(fetch,
			fetch_curl_get_handle(fetch->host));
}

/**
 * Cache a CURL handle for the provided host (if wanted)
 */
સ્થિર શૂન્ય fetch_curl_cache_handle(CURL *handle, lwc_string *host)
{
#જો LIBCURL_VERSION_અંક >= 0x071e00
	/* 7.30.0 or later has its own connection caching; suppress ours */
	curl_easy_cleanup(handle);
	પાછા;
#નહીંતર
	સંરચના cache_handle *h = 0;
	પૂર્ણાંક c;
	RING_FINDBYLWCHOST(curl_handle_ring, h, host);
	જો (h) {
		/* Already have a handle cached for this hostname */
		curl_easy_cleanup(handle);
		પાછા;
	}
	/* We do not have a handle cached, first up determine if the cache is full */
	RING_GETSIZE(સંરચના cache_handle, curl_handle_ring, c);
	જો (c >= nsoption_int(max_cached_fetch_handles)) {
		/* Cache is full, so, we rotate the ring by one and
		 * replace the oldest handle with this one. We do this
		 * without freeing/allocating memory (except the
		 * hostname) and without removing the entry from the
		 * ring and then re-inserting it, in order to be as
		 * efficient as we can.
		 */
		જો (curl_handle_ring != ખાલી) {
			h = curl_handle_ring;
			curl_handle_ring = h->r_next;
			curl_easy_cleanup(h->handle);
			h->handle = handle;
			lwc_string_unref(h->host);
			h->host = lwc_string_ref(host);
		} નહીંતર {
			/* Actually, we don't want to cache any handles */
			curl_easy_cleanup(handle);
		}

		પાછા;
	}
	/* The table isn't full yet, so make a shiny new handle to add to the ring */
	h = (સંરચના cache_handle*)malloc(કદ(સંરચના cache_handle));
	h->handle = handle;
	h->host = lwc_string_ref(host);
	RING_ઉમેરો(curl_handle_ring, h);
#endif
}


/**
 * Clean up the provided fetch object and free it.
 *
 * Will prod the queue afterwards to allow pending requests to be initiated.
 */
સ્થિર શૂન્ય fetch_curl_stop(સંરચના curl_fetch_info *f)
{
	CURLMcode codem;

	પ્રતિજ્ઞા(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));

	જો (f->curl_handle) {
		/* remove from curl multi handle */
		codem = curl_multi_remove_handle(fetch_curl_multi,
				f->curl_handle);
		પ્રતિજ્ઞા(codem == CURLM_OK);
		/* Put this curl handle into the cache if wanted. */
		fetch_curl_cache_handle(f->curl_handle, f->host);
		f->curl_handle = 0;
	}

	fetch_remove_from_queues(f->fetch_handle);
}


/**
 * Abort a fetch.
 */
સ્થિર શૂન્ય fetch_curl_abort(શૂન્ય *vf)
{
	સંરચના curl_fetch_info *f = (સંરચના curl_fetch_info *)vf;
	પ્રતિજ્ઞા(f);
	NSLOG(netsurf, INFO, "fetch %p, url '%s'", f, nsurl_access(f->url));
	જો (f->curl_handle) {
		જો (inside_curl) {
			NSLOG(netsurf, DEBUG, "Deferring cleanup");
			f->abort = true;
		} નહીંતર {
			NSLOG(netsurf, DEBUG, "Immediate abort");
			fetch_curl_stop(f);
			fetch_free(f->fetch_handle);
		}
	} નહીંતર {
		fetch_remove_from_queues(f->fetch_handle);
		fetch_free(f->fetch_handle);
	}
}


/**
 * Free a fetch structure and associated resources.
 */
સ્થિર શૂન્ય fetch_curl_free(શૂન્ય *vf)
{
	સંરચના curl_fetch_info *f = (સંરચના curl_fetch_info *)vf;
	પૂર્ણાંક i;

	જો (f->curl_handle) {
		curl_easy_cleanup(f->curl_handle);
	}
	nsurl_unref(f->url);
	lwc_string_unref(f->host);
	મુક્ત(f->location);
	મુક્ત(f->cookie_string);
	મુક્ત(f->realm);
	જો (f->headers) {
		curl_slist_free_all(f->headers);
	}
	fetch_curl_free_postdata(f->postdata);
	NSCURL_POSTDATA_FREE(f->curl_postdata);

	/* free certificate data */
	માટે (i = 0; i < MAX_CERT_DEPTH; i++) {
		જો (f->cert_data[i].cert != ખાલી) {
			X509_free(f->cert_data[i].cert);
		}
	}

	મુક્ત(f);
}


/**
 * Find the status code and content type and inform the caller.
 *
 * Return true if the fetch is being aborted.
 */
સ્થિર bool fetch_curl_process_headers(સંરચના curl_fetch_info *f)
{
	long http_code;
	CURLcode code;
	fetch_સંદેશ સંદેશ;

	f->had_headers = true;

	જો (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		પ્રતિજ્ઞા(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}
	http_code = f->http_code;
	NSLOG(netsurf, INFO, "HTTP status code %li", http_code);

	જો ((http_code == HTTP_RESPONSE_NOT_MODIFIED) &&
	    (f->postdata->type==FETCH_POSTDATA_NONE)) {
		/* Not Modified && GET request */
		સંદેશ.type = FETCH_NOTMODIFIED;
		fetch_send_callback(&સંદેશ, f->fetch_handle);
		પાછા true;
	}

	/* handle HTTP redirects (3xx response codes) */
	જો (HTTP_RESPONSE_IS_3XX(http_code) && (f->location != 0)) {
		NSLOG(netsurf, INFO, "FETCH_REDIRECT, '%s'", f->location);
		સંદેશ.type = FETCH_REDIRECT;
		સંદેશ.data.redirect = f->location;
		fetch_send_callback(&સંદેશ, f->fetch_handle);
		પાછા true;
	}

	/* handle HTTP 401 (Authentication errors) */
	જો (http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		સંદેશ.type = FETCH_AUTH;
		સંદેશ.data.auth.realm = f->realm;
		fetch_send_callback(&સંદેશ, f->fetch_handle);
		પાછા true;
	}

	/* handle HTTP errors (non 2xx response codes) */
	જો (f->only_2xx &&
	    strncmp(nsurl_access(f->url), "http", 4) == 0 &&
	    (HTTP_RESPONSE_IS_2XX(http_code) == false)) {
		સંદેશ.type = FETCH_ERROR;
		સંદેશ.data.error = messages_get("Not2xx");
		fetch_send_callback(&સંદેશ, f->fetch_handle);
		પાછા true;
	}

	જો (f->abort)
		પાછા true;

	પાછા false;
}


/**
 * Handle a completed fetch (CURLMSG_DONE from curl_multi_info_read()).
 *
 * \param curl_handle curl easy handle of fetch
 * \param result The result code of the completed fetch.
 */
સ્થિર શૂન્ય fetch_curl_done(CURL *curl_handle, CURLcode result)
{
	bool finished = false;
	bool error = false;
	bool cert = false;
	bool abort_fetch;
	સંરચના curl_fetch_info *f;
	અક્ષર **_hideous_hack = (અક્ષર **) (શૂન્ય *) &f;
	CURLcode code;

	/* find the structure associated with this fetch */
	/* For some reason, cURL thinks CURLINFO_PRIVATE should be a string?! */
	code = curl_easy_getinfo(curl_handle, CURLINFO_PRIVATE, _hideous_hack);
	પ્રતિજ્ઞા(code == CURLE_OK);

	abort_fetch = f->abort;
	NSLOG(netsurf, INFO, "done %s", nsurl_access(f->url));

	જો ((abort_fetch == false) &&
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
		જો (f->stopped ||
		    (!f->had_headers &&	fetch_curl_process_headers(f))) {
			; /* redirect with no body or similar */
		} નહીંતર {
			finished = true;
		}
	} નહીંતર જો (result == CURLE_PARTIAL_FILE) {
		/* CURLE_PARTIAL_FILE occurs if the received body of a
		 * response is smaller than that specified in the
		 * Content-Length header.
		 */
		જો (!f->had_headers && fetch_curl_process_headers(f))
			; /* redirect with partial body, or similar */
		નહીંતર {
			finished = true;
		}
	} નહીંતર જો (result == CURLE_WRITE_ERROR && f->stopped) {
		/* CURLE_WRITE_ERROR occurs when fetch_curl_data
		 * returns 0, which we use to abort intentionally
		 */
		;
	} નહીંતર જો (result == CURLE_SSL_PEER_CERTIFICATE ||
		   result == CURLE_SSL_CACERT) {
		/* Some kind of failure has occurred.  If we don't know
		 * what happened, we'll have reported unknown errors up
		 * to the user already via the certificate chain error fields.
		 */
		cert = true;
	} નહીંતર {
		NSLOG(netsurf, INFO, "Unknown cURL response code %d", result);
		error = true;
	}

	fetch_curl_stop(f);

	જો (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	જો (abort_fetch) {
		; /* fetch was aborted: no callback */
	} નહીંતર જો (finished) {
		fetch_સંદેશ સંદેશ;
		સંદેશ.type = FETCH_FINISHED;
		fetch_send_callback(&સંદેશ, f->fetch_handle);
	} નહીંતર જો (cert) {
		/* user needs to validate certificate with issue */
		fetch_સંદેશ સંદેશ;
		સંદેશ.type = FETCH_CERT_ERR;
		fetch_send_callback(&સંદેશ, f->fetch_handle);
	} નહીંતર જો (error) {
		fetch_સંદેશ સંદેશ;
		પસંદગી (result) {
		સ્થિતિ CURLE_SSL_CONNECT_ERROR:
			સંદેશ.type = FETCH_SSL_ERR;
			તોડો;

		સ્થિતિ CURLE_OPERATION_TIMEDOUT:
			સંદેશ.type = FETCH_TIMEDOUT;
			સંદેશ.data.error = curl_easy_strerror(result);
			તોડો;

		મૂળભૂત:
			સંદેશ.type = FETCH_ERROR;
			સંદેશ.data.error = curl_easy_strerror(result);
		}

		fetch_send_callback(&સંદેશ, f->fetch_handle);
	}

	fetch_free(f->fetch_handle);
}


/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */
સ્થિર શૂન્ય fetch_curl_poll(lwc_string *scheme_ignored)
{
	પૂર્ણાંક running, queue;
	CURLMcode codem;
	CURLMsg *curl_સંદેશ;

	જો (nsoption_bool(suppress_curl_debug) == false) {
		fd_set read_fd_set, write_fd_set, exc_fd_set;
		પૂર્ણાંક max_fd = -1;
		પૂર્ણાંક i;

		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		FD_ZERO(&exc_fd_set);

		codem = curl_multi_fdset(fetch_curl_multi,
				&read_fd_set, &write_fd_set,
				&exc_fd_set, &max_fd);
		પ્રતિજ્ઞા(codem == CURLM_OK);

		NSLOG(netsurf, DEEPDEBUG,
		      "Curl file descriptor states (maxfd=%i):", max_fd);
		માટે (i = 0; i <= max_fd; i++) {
			bool પઠ = false;
			bool લિખ = false;
			bool error = false;

			જો (FD_ISSET(i, &read_fd_set)) {
				પઠ = true;
			}
			જો (FD_ISSET(i, &write_fd_set)) {
				લિખ = true;
			}
			જો (FD_ISSET(i, &exc_fd_set)) {
				error = true;
			}
			જો (પઠ || લિખ || error) {
				NSLOG(netsurf, DEEPDEBUG, "  fd %i: %s %s %s", i,
				      પઠ ? "read" : "    ",
				      લિખ ? "write" : "     ",
				      error ? "error" : "     ");
			}
		}
	}

	/* do any possible work on the current fetches */
	inside_curl = true;
	કરો {
		codem = curl_multi_perform(fetch_curl_multi, &running);
		જો (codem != CURLM_OK && codem != CURLM_CALL_MULTI_PERFORM) {
			NSLOG(netsurf, WARNING,
			      "curl_multi_perform: %i %s",
			      codem, curl_multi_strerror(codem));
			પાછા;
		}
	} જ્યારે (codem == CURLM_CALL_MULTI_PERFORM);

	/* process curl results */
	curl_સંદેશ = curl_multi_info_read(fetch_curl_multi, &queue);
	જ્યારે (curl_સંદેશ) {
		પસંદગી (curl_સંદેશ->સંદેશ) {
			સ્થિતિ CURLMSG_DONE:
				fetch_curl_done(curl_સંદેશ->easy_handle,
						curl_સંદેશ->data.result);
				તોડો;
			મૂળભૂત:
				તોડો;
		}
		curl_સંદેશ = curl_multi_info_read(fetch_curl_multi, &queue);
	}
	inside_curl = false;
}




/**
 * Callback function for fetch progress.
 */
સ્થિર પૂર્ણાંક
fetch_curl_progress(શૂન્ય *clientp,
		    NSCURL_PROGRESS_T dltotal,
		    NSCURL_PROGRESS_T dlnow,
		    NSCURL_PROGRESS_T ultotal,
		    NSCURL_PROGRESS_T ulnow)
{
	સ્થિર અક્ષર fetch_progress_buffer[256]; /**< Progress buffer for cURL */
	સંરચના curl_fetch_info *f = (સંરચના curl_fetch_info *) clientp;
	uint64_t time_now_ms;
	fetch_સંદેશ સંદેશ;

	જો (f->abort) {
		પાછા 0;
        }

	સંદેશ.type = FETCH_PROGRESS;
	સંદેશ.data.progress = fetch_progress_buffer;

	/* Rate limit each fetch's progress notifications */
        nsu_getmonotonic_ms(&time_now_ms);
#define UPDATE_DELAY_MS (1000 / UPDATES_PER_SECOND)
	જો (time_now_ms - f->last_progress_update < UPDATE_DELAY_MS) {
		પાછા 0;
        }
#undef UPDATE_DELAY_MS
	f->last_progress_update = time_now_ms;

	જો (dltotal > 0) {
		snprintf(fetch_progress_buffer, 255,
				messages_get("Progress"),
				human_friendly_bytesize(dlnow),
				human_friendly_bytesize(dltotal));
		fetch_send_callback(&સંદેશ, f->fetch_handle);
	} નહીંતર {
		snprintf(fetch_progress_buffer, 255,
				messages_get("ProgressU"),
				human_friendly_bytesize(dlnow));
		fetch_send_callback(&સંદેશ, f->fetch_handle);
	}

	પાછા 0;
}


/**
 * Format curl debug for nslog
 */
સ્થિર પૂર્ણાંક
fetch_curl_debug(CURL *handle,
		 curl_infotype type,
		 અક્ષર *data,
		 size_t size,
		 શૂન્ય *userptr)
{
	સ્થિર અચલ અક્ષર s_infotype[CURLINFO_END][3] = {
		"* ", "< ", "> ", "{ ", "} ", "{ ", "} "
	};
	પસંદગી(type) {
	સ્થિતિ CURLINFO_TEXT:
	સ્થિતિ CURLINFO_HEADER_OUT:
	સ્થિતિ CURLINFO_HEADER_IN:
		NSLOG(fetch, DEBUG, "%s%.*s", s_infotype[type], (પૂર્ણાંક)size - 1, data);
		તોડો;

	મૂળભૂત:
		તોડો;
	}
	પાછા 0;
}


સ્થિર curl_socket_t fetch_curl_socket_open(શૂન્ય *clientp,
		curlsocktype purpose, સંરચના curl_sockaddr *address)
{
	(શૂન્ય) clientp;
	(શૂન્ય) purpose;

	પાછા (curl_socket_t) guit->fetch->socket_open(
			address->family, address->socktype,
			address->protocol);
}

સ્થિર પૂર્ણાંક fetch_curl_socket_close(શૂન્ય *clientp, curl_socket_t item)
{
	(શૂન્ય) clientp;

	પાછા guit->fetch->socket_close((પૂર્ણાંક) item);
}

/**
 * Callback function for cURL.
 */
સ્થિર size_t fetch_curl_data(અક્ષર *data, size_t size, size_t nmemb, શૂન્ય *_f)
{
	સંરચના curl_fetch_info *f = _f;
	CURLcode code;
	fetch_સંદેશ સંદેશ;

	/* ensure we only have to get this information once */
	જો (!f->http_code) {
		code = curl_easy_getinfo(f->curl_handle, CURLINFO_HTTP_CODE,
					 &f->http_code);
		પ્રતિજ્ઞા(code == CURLE_OK);
		fetch_set_http_code(f->fetch_handle, f->http_code);
	}

	/* ignore body if this is a 401 reply by skipping it and reset
	 * the HTTP response code to enable follow up fetches.
	 */
	જો (f->http_code == HTTP_RESPONSE_UNAUTHORIZED) {
		f->http_code = 0;
		પાછા size * nmemb;
	}

	જો (f->abort || (!f->had_headers && fetch_curl_process_headers(f))) {
		f->stopped = true;
		પાછા 0;
	}

	/* send data to the caller */
	સંદેશ.type = FETCH_DATA;
	સંદેશ.data.header_or_data.buf = (અચલ uint8_t *) data;
	સંદેશ.data.header_or_data.len = size * nmemb;
	fetch_send_callback(&સંદેશ, f->fetch_handle);

	જો (f->abort) {
		f->stopped = true;
		પાછા 0;
	}

	પાછા size * nmemb;
}


/**
 * Callback function for headers.
 *
 * See RFC 2616 4.2.
 */
સ્થિર size_t
fetch_curl_header(અક્ષર *data, size_t size, size_t nmemb, શૂન્ય *_f)
{
	સંરચના curl_fetch_info *f = _f;
	પૂર્ણાંક i;
	fetch_સંદેશ સંદેશ;
	size *= nmemb;

	જો (f->abort) {
		f->stopped = true;
		પાછા 0;
	}

	જો (f->sent_ssl_chain == false) {
		fetch_curl_report_certs_upstream(f);
	}

	સંદેશ.type = FETCH_HEADER;
	સંદેશ.data.header_or_data.buf = (અચલ uint8_t *) data;
	સંદેશ.data.header_or_data.len = size;
	fetch_send_callback(&સંદેશ, f->fetch_handle);

#define SKIP_ST(o) માટે (i = (o); i < (પૂર્ણાંક) size && (data[i] == ' ' || data[i] == '\t'); i++)

	જો (12 < size && strncasecmp(data, "Location:", 9) == 0) {
		/* extract Location header */
		મુક્ત(f->location);
		f->location = malloc(size);
		જો (!f->location) {
			NSLOG(netsurf, INFO, "malloc failed");
			પાછા size;
		}
		SKIP_ST(9);
		strncpy(f->location, data + i, size - i);
		f->location[size - i] = '\0';
		માટે (i = size - i - 1; i >= 0 &&
				(f->location[i] == ' ' ||
				f->location[i] == '\t' ||
				f->location[i] == '\r' ||
				f->location[i] == '\n'); i--)
			f->location[i] = '\0';
	} નહીંતર જો (15 < size && strncasecmp(data, "Content-Length:", 15) == 0) {
		/* extract Content-Length header */
		SKIP_ST(15);
		જો (i < (પૂર્ણાંક)size && '0' <= data[i] && data[i] <= '9')
			f->content_length = atol(data + i);
	} નહીંતર જો (17 < size && strncasecmp(data, "WWW-Authenticate:", 17) == 0) {
		/* extract the first Realm from WWW-Authenticate header */
		SKIP_ST(17);

		જ્યારે (i < (પૂર્ણાંક) size - 5 &&
				strncasecmp(data + i, "realm", 5))
			i++;
		જ્યારે (i < (પૂર્ણાંક) size - 1 && data[++i] != '"')
			/* */;
		i++;

		જો (i < (પૂર્ણાંક) size) {
			size_t end = i;

			જ્યારે (end < size && data[end] != '"')
				++end;

			જો (end < size) {
				મુક્ત(f->realm);
				f->realm = malloc(end - i + 1);
				જો (f->realm != ખાલી) {
					strncpy(f->realm, data + i, end - i);
					f->realm[end - i] = '\0';
				}
			}
		}
	} નહીંતર જો (11 < size && strncasecmp(data, "Set-Cookie:", 11) == 0) {
		/* extract Set-Cookie header */
		SKIP_ST(11);

		fetch_set_cookie(f->fetch_handle, &data[i]);
	}

	પાછા size;
#undef SKIP_ST
}

સ્થિર પૂર્ણાંક fetch_curl_fdset(lwc_string *scheme, fd_set *read_set,
			    fd_set *write_set, fd_set *error_set)
{
	CURLMcode code;
	પૂર્ણાંક maxfd = -1;

	code = curl_multi_fdset(fetch_curl_multi,
				read_set,
				write_set,
				error_set,
				&maxfd);
	પ્રતિજ્ઞા(code == CURLM_OK);

	પાછા maxfd;
}



/* exported function documented in content/fetchers/curl.h */
nserror fetch_curl_register(શૂન્ય)
{
	CURLcode code;
	curl_version_info_data *data;
	પૂર્ણાંક i;
	lwc_string *scheme;
	અચલ સંરચના fetcher_operation_table fetcher_ops = {
		.initialise = fetch_curl_initialise,
		.acceptable = fetch_curl_can_fetch,
		.setup = fetch_curl_setup,
		.start = fetch_curl_start,
		.abort = fetch_curl_abort,
		.મુક્ત = fetch_curl_free,
		.poll = fetch_curl_poll,
		.fdset = fetch_curl_fdset,
		.finalise = fetch_curl_finalise
	};

#જો LIBCURL_VERSION_અંક >= 0x073800
	/* version 7.56.0 can select which SSL backend to use */
	CURLsslset setres;

	setres = curl_global_sslset(CURLSSLBACKEND_OPENSSL, ખાલી, ખાલી);
	જો (setres == CURLSSLSET_OK) {
		curl_with_openssl = true;
	} નહીંતર {
		curl_with_openssl = false;
	}
#endif

	NSLOG(netsurf, INFO, "curl_version %s", curl_version());

	code = curl_global_init(CURL_GLOBAL_ALL);
	જો (code != CURLE_OK) {
		NSLOG(netsurf, INFO, "curl_global_init failed.");
		પાછા NSERROR_INIT_FAILED;
	}

	fetch_curl_multi = curl_multi_init();
	જો (!fetch_curl_multi) {
		NSLOG(netsurf, INFO, "curl_multi_init failed.");
		પાછા NSERROR_INIT_FAILED;
	}

#જો LIBCURL_VERSION_અંક >= 0x071e00
	/* built against 7.30.0 or later: configure caching */
	{
		CURLMcode mcode;
		પૂર્ણાંક maxconnects = nsoption_int(max_fetchers) +
				nsoption_int(max_cached_fetch_handles);

#undef SETOPT
#define SETOPT(option, કિંમત) \
	mcode = curl_multi_setopt(fetch_curl_multi, option, કિંમત);	\
	જો (mcode != CURLM_OK) {					\
		NSLOG(netsurf, ERROR, "attempting curl_multi_setopt(%s, ...)", #option); \
		જાઓ curl_multi_setopt_failed;				\
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
	જો (!fetch_blank_curl) {
		NSLOG(netsurf, INFO, "curl_easy_init failed");
		પાછા NSERROR_INIT_FAILED;
	}

#undef SETOPT
#define SETOPT(option, કિંમત) \
	code = curl_easy_setopt(fetch_blank_curl, option, કિંમત);	\
	જો (code != CURLE_OK) {						\
		NSLOG(netsurf, ERROR, "attempting curl_easy_setopt(%s, ...)", #option); \
		જાઓ curl_easy_setopt_failed;				\
	}

	SETOPT(CURLOPT_ERRORBUFFER, fetch_error_buffer);
	SETOPT(CURLOPT_DEBUGFUNCTION, fetch_curl_debug);
	જો (nsoption_bool(suppress_curl_debug)) {
		SETOPT(CURLOPT_VERBOSE, 0);
	} નહીંતર {
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
	SETOPT(CURLOPT_નીચલું_SPEED_LIMIT, 1L);
	SETOPT(CURLOPT_નીચલું_SPEED_TIME, 180L);
	SETOPT(CURLOPT_NOSIGNAL, 1L);
	SETOPT(CURLOPT_CONNECTTIMEOUT, (long)nsoption_uint(curl_fetch_timeout));
	SETOPT(CURLOPT_OPENSOCKETFUNCTION, fetch_curl_socket_open);
	SETOPT(CURLOPT_CLOSESOCKETFUNCTION, fetch_curl_socket_close);

	જો (nsoption_charp(ca_bundle) &&
	    strcmp(nsoption_charp(ca_bundle), "")) {
		NSLOG(netsurf, INFO, "ca_bundle: '%s'",
		      nsoption_charp(ca_bundle));
		SETOPT(CURLOPT_CAINFO, nsoption_charp(ca_bundle));
	}
	જો (nsoption_charp(ca_path) && strcmp(nsoption_charp(ca_path), "")) {
		NSLOG(netsurf, INFO, "ca_path: '%s'", nsoption_charp(ca_path));
		SETOPT(CURLOPT_CAPATH, nsoption_charp(ca_path));
	}

#જો LIBCURL_VERSION_અંક < 0x073800
	/*
	 * before 7.56.0 Detect openssl from whether the SSL CTX
	 *  function API works
	 */
	code = curl_easy_setopt(fetch_blank_curl, CURLOPT_SSL_CTX_FUNCTION, ખાલી);
	જો (code != CURLE_OK) {
		curl_with_openssl = false;
	} નહીંતર {
		curl_with_openssl = true;
	}
#endif

	જો (curl_with_openssl) {
		/* only set the cipher list with openssl otherwise the
		 *  fetch fails with "Unknown cipher in list"
		 */
#જો LIBCURL_VERSION_અંક >= 0x073d00
		/* Need libcurl 7.61.0 or later built against OpenSSL with
		 * TLS1.3 support */
		code = curl_easy_setopt(fetch_blank_curl,
				CURLOPT_TLS13_CIPHERS, CIPHER_SUITES);
		જો (code != CURLE_OK && code != CURLE_NOT_BUILT_IN)
			જાઓ curl_easy_setopt_failed;
#endif
		SETOPT(CURLOPT_SSL_CIPHER_LIST, CIPHER_LIST);
	}

	NSLOG(netsurf, INFO, "cURL %slinked against openssl",
	      curl_with_openssl ? "" : "not ");

	/* cURL initialised okay, register the fetchers */

	data = curl_version_info(CURLVERSION_NOW);

	curl_fetch_ssl_hashmap = hashmap_create(&curl_fetch_ssl_hashmap_parameters);
	જો (curl_fetch_ssl_hashmap == ખાલી) {
		NSLOG(netsurf, CRITICAL, "Unable to initialise SSL certificate hashmap");
		પાછા NSERROR_NOMEM;
	}

	માટે (i = 0; data->protocols[i]; i++) {
		જો (strcmp(data->protocols[i], "http") == 0) {
			scheme = lwc_string_ref(corestring_lwc_http);

		} નહીંતર જો (strcmp(data->protocols[i], "https") == 0) {
			scheme = lwc_string_ref(corestring_lwc_https);

		} નહીંતર {
			/* Ignore non-http(s) protocols */
			ચાલુ;
		}

		જો (fetcher_add(scheme, &fetcher_ops) != NSERROR_OK) {
			NSLOG(netsurf, INFO,
			      "Unable to register cURL fetcher for %s",
			      data->protocols[i]);
		}
	}

	પાછા NSERROR_OK;

curl_easy_setopt_failed:
	NSLOG(netsurf, INFO, "curl_easy_setopt failed.");
	પાછા NSERROR_INIT_FAILED;

#જો LIBCURL_VERSION_અંક >= 0x071e00
curl_multi_setopt_failed:
	NSLOG(netsurf, INFO, "curl_multi_setopt failed.");
	પાછા NSERROR_INIT_FAILED;
#endif
}
