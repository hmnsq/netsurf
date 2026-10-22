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

#ifndef _NETSURF_DESKTOP_FETCH_H_
#define _NETSURF_DESKTOP_FETCH_H_

#அந்தர்பாவ <stdbool.h>

#அந்தர்பாவ "utils/config.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "utils/inet.h"
#அந்தர்பாவ "utils/http/response-codes.h"

#அந்தர்பாவ "netsurf/ssl_certs.h"

கட்டமைப்பு content;
கட்டமைப்பு fetch;
கட்டமைப்பு ssl_cert_info;

/**
 * Fetcher message types
 */
வகைவரை எண்ணிக்கை {
	FETCH_PROGRESS,
	FETCH_CERTS,
	FETCH_HEADER,
	FETCH_DATA,
	/* Anything after here is a completed fetch of some kind. */
	FETCH_FINISHED,
	FETCH_TIMEDOUT,
	FETCH_ERROR,
	FETCH_REDIRECT,
	FETCH_NOTMODIFIED,
	FETCH_AUTH,
	FETCH_CERT_ERR,
	FETCH_SSL_ERR
} fetch_செய்தி_type;

/** Minimum finished message type.
 *
 * If a fetch does not progress this far, it's an error and the fetch machinery
 * will send FETCH_ERROR to the llcache on fetch_free()
 */
#define FETCH_MIN_FINISHED_செய்தி FETCH_FINISHED

/**
 * This message is actually an internal message used to indicate
 * that a fetch was aborted.  Do not send this, nor expect it.
 */
#define FETCH__INTERNAL_ABORTED FETCH_ERROR

/**
 * Fetcher message data
 */
வகைவரை கட்டமைப்பு fetch_செய்தி {
	fetch_செய்தி_type type;

	ஒன்றிப்பு {
		மாறிலி எழுத்து *progress;

		கட்டமைப்பு {
			மாறிலி uint8_t *buf;
			size_t len;
		} header_or_data;

		மாறிலி எழுத்து *error;

		/** \todo Use nsurl */
		மாறிலி எழுத்து *redirect;

		கட்டமைப்பு {
			மாறிலி எழுத்து *realm;
		} auth;

		மாறிலி கட்டமைப்பு cert_chain *chain;
	} data;
} fetch_செய்தி;

/**
 * Fetcher post data types
 */
வகைவரை எண்ணிக்கை {
	FETCH_POSTDATA_NONE,
	FETCH_POSTDATA_URLENC,
	FETCH_POSTDATA_MULTIPART,
} fetch_postdata_type;


/**
 * Fetch POST multipart data
 */
கட்டமைப்பு fetch_multipart_data {
	கட்டமைப்பு fetch_multipart_data *next; /**< Next in linked list */

	எழுத்து *name; /**< Name of item */
	எழுத்து *மதிப்பு; /**< Item value */

	எழுத்து *rawfile; /**< Raw filename if file is true */
	bool file; /**< Item is a file */
};

/**
 * fetch POST data
 */
கட்டமைப்பு fetch_postdata {
	fetch_postdata_type type;
	ஒன்றிப்பு {
		/** Url encoded POST string if type is FETCH_POSTDATA_URLENC */
		எழுத்து *urlenc;
		/** Multipart post data if type is FETCH_POSTDATA_MULTIPART */
		கட்டமைப்பு fetch_multipart_data *multipart;
	} data;
};


வகைவரை வெறுமை (*fetch_callback)(மாறிலி fetch_செய்தி *செய்தி, வெறுமை *p);

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
nserror fetch_start(nsurl *url, nsurl *referer, fetch_callback callback,
		    வெறுமை *p, bool only_2xx, மாறிலி எழுத்து *post_urlenc,
		    மாறிலி கட்டமைப்பு fetch_multipart_data *post_multipart,
		    bool verifiable, bool downgrade_tls,
		    மாறிலி எழுத்து *headers[], கட்டமைப்பு fetch **fetch_out);

/**
 * Abort a fetch.
 */
வெறுமை fetch_abort(கட்டமைப்பு fetch *f);


/**
 * Check if a URL's scheme can be fetched.
 *
 * \param  url  URL to check
 * \return  true if the scheme is supported
 */
bool fetch_can_fetch(மாறிலி nsurl *url);

/**
 * Change the callback function for a fetch.
 */
வெறுமை fetch_change_callback(கட்டமைப்பு fetch *fetch, fetch_callback callback, வெறுமை *p);

/**
 * Get the HTTP response code.
 */
http_response_code fetch_http_code(கட்டமைப்பு fetch *fetch);


/**
 * Free a linked list of fetch_multipart_data.
 *
 * \param list Pointer to head of list to free
 */
வெறுமை fetch_multipart_data_destroy(கட்டமைப்பு fetch_multipart_data *list);

/**
 * Clone a linked list of fetch_multipart_data.
 *
 * \param list  List to clone
 * \return Pointer to head of cloned list, or NULL on failure
 */
கட்டமைப்பு fetch_multipart_data *fetch_multipart_data_clone(மாறிலி கட்டமைப்பு fetch_multipart_data *list);

/**
 * Find an entry in a fetch_multipart_data
 *
 * \param list Pointer to the multipart list
 * \param name The name to look for in the list
 * \return The value found, or NULL if not present
 */
மாறிலி எழுத்து *fetch_multipart_data_find(மாறிலி கட்டமைப்பு fetch_multipart_data *list,
				      மாறிலி எழுத்து *name);

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
nserror fetch_multipart_data_new_kv(கட்டமைப்பு fetch_multipart_data **list,
				    மாறிலி எழுத்து *name,
				    மாறிலி எழுத்து *மதிப்பு);

/**
 * send message to fetch
 */
வெறுமை fetch_send_callback(மாறிலி fetch_செய்தி *செய்தி, கட்டமைப்பு fetch *fetch);

/**
 * remove a queued fetch
 */
வெறுமை fetch_remove_from_queues(கட்டமைப்பு fetch *fetch);

/**
 * Free a fetch structure and associated resources.
 */
வெறுமை fetch_free(கட்டமைப்பு fetch *f);

/**
 * set the http code of a fetch
 */
nserror fetch_set_http_code(கட்டமைப்பு fetch *fetch, http_response_code http_code);

/**
 * set cookie data on a fetch
 */
வெறுமை fetch_set_cookie(கட்டமைப்பு fetch *fetch, மாறிலி எழுத்து *data);

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
nserror fetch_fdset(fd_set *read_fd_set, fd_set *write_fd_set, fd_set *except_fd_set, முழு *maxfd);

#endif
