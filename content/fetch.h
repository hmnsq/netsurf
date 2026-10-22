/*
 * Copyright 2003 James Bursa <bursa@users.sourceforge.net>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 * Fetching of data from a URL (interface).
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _NETSURF_DESKTOP_FETCH_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _NETSURF_DESKTOP_FETCH_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/inet.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/http/response-codes.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/ssl_certs.h"

ਬਣਤਰ content;
ਬਣਤਰ fetch;
ਬਣਤਰ ssl_cert_info;

/**
 * Fetcher message types
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	FETCH_ਤਰੱਕੀ,
	FETCH_CERTS,
	FETCH_ਸਿਰਲੇਖ,
	FETCH_ਅੰਕੜੇ,
	/* Anything after here is a completed fetch of some kind. */
	FETCH_ਮੁਕੰਮਲ,
	FETCH_TIMEDOUT,
	FETCH_ERROR,
	FETCH_REDIRECT,
	FETCH_NOTMODIFIED,
	FETCH_AUTH,
	FETCH_CERT_ERR,
	FETCH_SSL_ERR
} fetch_ਸੁਨੇਹਾ_type;

/** Minimum finished message type.
 *
 * If a fetch does not progress this far, it's an error and the fetch machinery
 * will send FETCH_ERROR to the llcache on fetch_free()
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FETCH_MIN_ਮੁਕੰਮਲ_ਸੁਨੇਹਾ FETCH_ਮੁਕੰਮਲ

/**
 * This message is actually an internal message used to indicate
 * that a fetch was aborted.  Do not send this, nor expect it.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FETCH_ਅੰਦਰੂਨੀ_ABORTED FETCH_ERROR

/**
 * Fetcher message data
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ fetch_ਸੁਨੇਹਾ {
	fetch_ਸੁਨੇਹਾ_type type;

	ਸੰਘ {
		ਅਚਲ ਅੱਖਰ *ਤਰੱਕੀ;

		ਬਣਤਰ {
			ਅਚਲ uint8_t *buf;
			size_t len;
		} ਸਿਰਲੇਖ_or_ਅੰਕੜੇ;

		ਅਚਲ ਅੱਖਰ *error;

		/** \todo Use nsurl */
		ਅਚਲ ਅੱਖਰ *redirect;

		ਬਣਤਰ {
			ਅਚਲ ਅੱਖਰ *realm;
		} auth;

		ਅਚਲ ਬਣਤਰ cert_chain *chain;
	} ਅੰਕੜੇ;
} fetch_ਸੁਨੇਹਾ;

/**
 * Fetcher post data types
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	FETCH_POSTDATA_NONE,
	FETCH_POSTDATA_URLENC,
	FETCH_POSTDATA_MULTIPART,
} fetch_postdata_type;


/**
 * Fetch POST multipart data
 */
ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ {
	ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *ਅਗਲਾ; /**< Next in linked list */

	ਅੱਖਰ *name; /**< Name of item */
	ਅੱਖਰ *ਮੁੱਲ; /**< Item value */

	ਅੱਖਰ *rawfile; /**< Raw filename if file is true */
	bool file; /**< Item is a file */
};

/**
 * fetch POST data
 */
ਬਣਤਰ fetch_postdata {
	fetch_postdata_type type;
	ਸੰਘ {
		/** Url encoded POST string if type is FETCH_POSTDATA_URLENC */
		ਅੱਖਰ *urlenc;
		/** Multipart post data if type is FETCH_POSTDATA_MULTIPART */
		ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *multipart;
	} ਅੰਕੜੇ;
};


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ (*fetch_callback)(ਅਚਲ fetch_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ, ਖਾਲੀ *p);

/**
 * Start fetching data for the given URL.
 *
 * The function returns immediately. The fetch may be queued for later
 * processing.
 *
 * A pointer to an opaque struct fetch is returned, which can be passed to
 * fetch_abort() to abort the fetch at any time. Returns NULL if memory is
 * exhausted (or some other fatal error occurred).
 *
 * The caller must supply a callback function which is called when anything
 * interesting happens. The callback function is first called with msg
 * FETCH_HEADER, with the header in data, then one or more times
 * with FETCH_DATA with some data for the url, and finally with
 * FETCH_FINISHED. Alternatively, FETCH_ERROR indicates an error occurred:
 * data contains an error message. FETCH_REDIRECT may replace the FETCH_HEADER,
 * FETCH_DATA, FETCH_FINISHED sequence if the server sends a replacement URL.
 *
 * \param url URL to fetch
 * \param referer
 * \param callback
 * \param p
 * \param only_2xx
 * \param post_urlenc
 * \param post_multipart
 * \param verifiable
 * \param downgrade_tls
 * \param headers
 * \param fetch_out ponter to recive new fetch object.
 * \return NSERROR_OK and fetch_out updated else appropriate error code
 */
nserror fetch_ਸ਼ੁਰੂ(nsurl *url, nsurl *referer, fetch_callback callback,
		    ਖਾਲੀ *p, bool only_2xx, ਅਚਲ ਅੱਖਰ *post_urlenc,
		    ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *post_multipart,
		    bool verifiable, bool downgrade_tls,
		    ਅਚਲ ਅੱਖਰ *ਸਿਰਲੇਖਾਂ[], ਬਣਤਰ fetch **fetch_ਬਾਹਰਲਾ);

/**
 * Abort a fetch.
 */
ਖਾਲੀ fetch_abort(ਬਣਤਰ fetch *f);


/**
 * Check if a URL's scheme can be fetched.
 *
 * \param  url  URL to check
 * \return  true if the scheme is supported
 */
bool fetch_can_fetch(ਅਚਲ nsurl *url);

/**
 * Change the callback function for a fetch.
 */
ਖਾਲੀ fetch_change_callback(ਬਣਤਰ fetch *fetch, fetch_callback callback, ਖਾਲੀ *p);

/**
 * Get the HTTP response code.
 */
http_response_ਕੋਡ fetch_http_ਕੋਡ(ਬਣਤਰ fetch *fetch);


/**
 * Free a linked list of fetch_multipart_data.
 *
 * \param list Pointer to head of list to free
 */
ਖਾਲੀ fetch_multipart_ਅੰਕੜੇ_destroy(ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *list);

/**
 * Clone a linked list of fetch_multipart_data.
 *
 * \param list  List to clone
 * \return Pointer to head of cloned list, or NULL on failure
 */
ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *fetch_multipart_ਅੰਕੜੇ_clone(ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *list);

/**
 * Find an entry in a fetch_multipart_data
 *
 * \param list Pointer to the multipart list
 * \param name The name to look for in the list
 * \return The value found, or NULL if not present
 */
ਅਚਲ ਅੱਖਰ *fetch_multipart_ਅੰਕੜੇ_ਲੱਭੋ(ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *list,
				      ਅਚਲ ਅੱਖਰ *name);

/**
 * Create an entry for a fetch_multipart_data
 *
 * If an entry exists of the same name, it will *NOT* be overwritten
 *
 * \param list Pointer to the pointer to the current multipart list
 * \param name The name of the entry to create
 * \param value The value of the entry to create
 * \return The result of the attempt
 */
nserror fetch_multipart_ਅੰਕੜੇ_new_kv(ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ **list,
				    ਅਚਲ ਅੱਖਰ *name,
				    ਅਚਲ ਅੱਖਰ *ਮੁੱਲ);

/**
 * send message to fetch
 */
ਖਾਲੀ fetch_send_callback(ਅਚਲ fetch_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ, ਬਣਤਰ fetch *fetch);

/**
 * remove a queued fetch
 */
ਖਾਲੀ fetch_remove_from_queues(ਬਣਤਰ fetch *fetch);

/**
 * Free a fetch structure and associated resources.
 */
ਖਾਲੀ fetch_free(ਬਣਤਰ fetch *f);

/**
 * set the http code of a fetch
 */
nserror fetch_ਸੈੱਟ_http_ਕੋਡ(ਬਣਤਰ fetch *fetch, http_response_ਕੋਡ http_ਕੋਡ);

/**
 * set cookie data on a fetch
 */
ਖਾਲੀ fetch_ਸੈੱਟ_cookie(ਬਣਤਰ fetch *fetch, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ);

/**
 * Get the set of file descriptors the fetchers are currently using.
 *
 * This obtains the file descriptors the fetch system is using to
 * obtain data. It will cause the fetchers to make progress, if
 * possible, potentially completing fetches before requiring activity
 * on file descriptors.
 *
 * If a set of descriptors is returned (maxfd is not -1) The caller is
 * expected to wait on them (with select etc.) and continue to obtain
 * the fdset with this call. This will switch the fetchers from polled
 * mode to waiting for network activity which is much more efficient.
 *
 * \note If the caller does not subsequently obtain the fdset again
 * the fetchers will fall back to the less efficient polled
 * operation. The fallback to polled operation will only occour after
 * a timeout which introduces additional delay.
 *
 * \param[out] read_fd_set The fd set for read.
 * \param[out] write_fd_set The fd set for write.
 * \param[out] except_fd_set The fd set for exceptions.
 * \param[out] maxfd The highest fd number in the set or -1 if no fd available.
 * \return NSERROR_OK on success or appropriate error code.
 */
nserror fetch_fdset(fd_set *read_fd_ਸੈੱਟ, fd_set *write_fd_ਸੈੱਟ, fd_set *except_fd_ਸੈੱਟ, ਪੂਰਨ *maxfd);

#ਜੇ_ਅੰਤ
