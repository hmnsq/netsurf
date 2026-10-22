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

#অন্তর্ভাৱ <stdbool.h>

#অন্তর্ভাৱ "utils/config.h"
#অন্তর্ভাৱ "utils/nsurl.h"
#অন্তর্ভাৱ "utils/inet.h"
#অন্তর্ভাৱ "utils/http/response-codes.h"

#অন্তর্ভাৱ "netsurf/ssl_certs.h"

সংরচনা content;
সংরচনা fetch;
সংরচনা ssl_cert_info;

/**
 * Fetcher message types
 */
প্রকারনির্দেশ গণনা {
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
} fetch_বাৰ্তা_type;

/** Minimum finished message type.
 *
 * If a fetch does not progress this far, it's an error and the fetch machinery
 * will send FETCH_ERROR to the llcache on fetch_free()
 */
#define FETCH_MIN_FINISHED_বাৰ্তা FETCH_FINISHED

/**
 * This message is actually an internal message used to indicate
 * that a fetch was aborted.  Do not send this, nor expect it.
 */
#define FETCH__INTERNAL_ABORTED FETCH_ERROR

/**
 * Fetcher message data
 */
প্রকারনির্দেশ সংরচনা fetch_বাৰ্তা {
	fetch_বাৰ্তা_type type;

	সঙ্ঘ {
		ধ্রুৱ ৱর্ণ *progress;

		সংরচনা {
			ধ্রুৱ uint8_t *buf;
			size_t len;
		} header_or_data;

		ধ্রুৱ ৱর্ণ *error;

		/** \todo Use nsurl */
		ধ্রুৱ ৱর্ণ *redirect;

		সংরচনা {
			ধ্রুৱ ৱর্ণ *realm;
		} auth;

		ধ্রুৱ সংরচনা cert_chain *chain;
	} data;
} fetch_বাৰ্তা;

/**
 * Fetcher post data types
 */
প্রকারনির্দেশ গণনা {
	FETCH_POSTDATA_NONE,
	FETCH_POSTDATA_URLENC,
	FETCH_POSTDATA_MULTIPART,
} fetch_postdata_type;


/**
 * Fetch POST multipart data
 */
সংরচনা fetch_multipart_data {
	সংরচনা fetch_multipart_data *next; /**< Next in linked list */

	ৱর্ণ *name; /**< Name of item */
	ৱর্ণ *মান; /**< Item value */

	ৱর্ণ *rawfile; /**< Raw filename if file is true */
	bool file; /**< Item is a file */
};

/**
 * fetch POST data
 */
সংরচনা fetch_postdata {
	fetch_postdata_type type;
	সঙ্ঘ {
		/** Url encoded POST string if type is FETCH_POSTDATA_URLENC */
		ৱর্ণ *urlenc;
		/** Multipart post data if type is FETCH_POSTDATA_MULTIPART */
		সংরচনা fetch_multipart_data *multipart;
	} data;
};


প্রকারনির্দেশ শূন্য (*fetch_callback)(ধ্রুৱ fetch_বাৰ্তা *বাৰ্তা, শূন্য *p);

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
		    শূন্য *p, bool only_2xx, ধ্রুৱ ৱর্ণ *post_urlenc,
		    ধ্রুৱ সংরচনা fetch_multipart_data *post_multipart,
		    bool verifiable, bool downgrade_tls,
		    ধ্রুৱ ৱর্ণ *headers[], সংরচনা fetch **fetch_out);

/**
 * Abort a fetch.
 */
শূন্য fetch_abort(সংরচনা fetch *f);


/**
 * Check if a URL's scheme can be fetched.
 *
 * \param  url  URL to check
 * \return  true if the scheme is supported
 */
bool fetch_can_fetch(ধ্রুৱ nsurl *url);

/**
 * Change the callback function for a fetch.
 */
শূন্য fetch_change_callback(সংরচনা fetch *fetch, fetch_callback callback, শূন্য *p);

/**
 * Get the HTTP response code.
 */
http_response_code fetch_http_code(সংরচনা fetch *fetch);


/**
 * Free a linked list of fetch_multipart_data.
 *
 * \param list Pointer to head of list to free
 */
শূন্য fetch_multipart_data_destroy(সংরচনা fetch_multipart_data *list);

/**
 * Clone a linked list of fetch_multipart_data.
 *
 * \param list  List to clone
 * \return Pointer to head of cloned list, or NULL on failure
 */
সংরচনা fetch_multipart_data *fetch_multipart_data_clone(ধ্রুৱ সংরচনা fetch_multipart_data *list);

/**
 * Find an entry in a fetch_multipart_data
 *
 * \param list Pointer to the multipart list
 * \param name The name to look for in the list
 * \return The value found, or NULL if not present
 */
ধ্রুৱ ৱর্ণ *fetch_multipart_data_find(ধ্রুৱ সংরচনা fetch_multipart_data *list,
				      ধ্রুৱ ৱর্ণ *name);

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
nserror fetch_multipart_data_new_kv(সংরচনা fetch_multipart_data **list,
				    ধ্রুৱ ৱর্ণ *name,
				    ধ্রুৱ ৱর্ণ *মান);

/**
 * send message to fetch
 */
শূন্য fetch_send_callback(ধ্রুৱ fetch_বাৰ্তা *বাৰ্তা, সংরচনা fetch *fetch);

/**
 * remove a queued fetch
 */
শূন্য fetch_remove_from_queues(সংরচনা fetch *fetch);

/**
 * Free a fetch structure and associated resources.
 */
শূন্য fetch_free(সংরচনা fetch *f);

/**
 * set the http code of a fetch
 */
nserror fetch_set_http_code(সংরচনা fetch *fetch, http_response_code http_code);

/**
 * set cookie data on a fetch
 */
শূন্য fetch_set_cookie(সংরচনা fetch *fetch, ধ্রুৱ ৱর্ণ *data);

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
nserror fetch_fdset(fd_set *read_fd_set, fd_set *write_fd_set, fd_set *except_fd_set, পূর্ণাঙ্ক *maxfd);

#endif
