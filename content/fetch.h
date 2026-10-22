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

#অন্তর্ভাব <stdbool.h>

#অন্তর্ভাব "utils/config.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "utils/inet.h"
#অন্তর্ভাব "utils/http/response-codes.h"

#অন্তর্ভাব "netsurf/ssl_certs.h"

কাঠামো content;
কাঠামো fetch;
কাঠামো ssl_cert_info;

/**
 * Fetcher message types
 */
প্রকারলেখ গণনা {
	FETCH_PROGRESS,
	FETCH_CERTS,
	FETCH_HEADER,
	FETCH_তথ্য,
	/* Anything after here is a completed fetch of some kind. */
	FETCH_FINISHED,
	FETCH_TIMEDOUT,
	FETCH_ERROR,
	FETCH_REDIRECT,
	FETCH_NOTMODIFIED,
	FETCH_AUTH,
	FETCH_CERT_ERR,
	FETCH_SSL_ERR
} fetch_বার্তা_type;

/** Minimum finished message type.
 *
 * If a fetch does not progress this far, it's an error and the fetch machinery
 * will send FETCH_ERROR to the llcache on fetch_free()
 */
#define FETCH_MIN_FINISHED_বার্তা FETCH_FINISHED

/**
 * This message is actually an internal message used to indicate
 * that a fetch was aborted.  Do not send this, nor expect it.
 */
#define FETCH__INTERNAL_ABORTED FETCH_ERROR

/**
 * Fetcher message data
 */
প্রকারলেখ কাঠামো fetch_বার্তা {
	fetch_বার্তা_type type;

	সংঘ {
		ধ্রুবক অক্ষর *progress;

		কাঠামো {
			ধ্রুবক uint8_t *buf;
			size_t len;
		} header_or_তথ্য;

		ধ্রুবক অক্ষর *error;

		/** \todo Use nsurl */
		ধ্রুবক অক্ষর *redirect;

		কাঠামো {
			ধ্রুবক অক্ষর *realm;
		} auth;

		ধ্রুবক কাঠামো cert_chain *chain;
	} তথ্য;
} fetch_বার্তা;

/**
 * Fetcher post data types
 */
প্রকারলেখ গণনা {
	FETCH_POSTDATA_NONE,
	FETCH_POSTDATA_URLENC,
	FETCH_POSTDATA_MULTIPART,
} fetch_postdata_type;


/**
 * Fetch POST multipart data
 */
কাঠামো fetch_multipart_তথ্য {
	কাঠামো fetch_multipart_তথ্য *পরবর্তী; /**< Next in linked list */

	অক্ষর *name; /**< Name of item */
	অক্ষর *মান; /**< Item value */

	অক্ষর *rawfile; /**< Raw filename if file is true */
	bool file; /**< Item is a file */
};

/**
 * fetch POST data
 */
কাঠামো fetch_postdata {
	fetch_postdata_type type;
	সংঘ {
		/** Url encoded POST string if type is FETCH_POSTDATA_URLENC */
		অক্ষর *urlenc;
		/** Multipart post data if type is FETCH_POSTDATA_MULTIPART */
		কাঠামো fetch_multipart_তথ্য *multipart;
	} তথ্য;
};


প্রকারলেখ শূন্য (*fetch_callback)(ধ্রুবক fetch_বার্তা *বার্তা, শূন্য *p);

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
		    শূন্য *p, bool only_2xx, ধ্রুবক অক্ষর *post_urlenc,
		    ধ্রুবক কাঠামো fetch_multipart_তথ্য *post_multipart,
		    bool verifiable, bool downgrade_tls,
		    ধ্রুবক অক্ষর *headers[], কাঠামো fetch **fetch_out);

/**
 * Abort a fetch.
 */
শূন্য fetch_abort(কাঠামো fetch *f);


/**
 * Check if a URL's scheme can be fetched.
 *
 * \param  url  URL to check
 * \return  true if the scheme is supported
 */
bool fetch_can_fetch(ধ্রুবক nsurl *url);

/**
 * Change the callback function for a fetch.
 */
শূন্য fetch_change_callback(কাঠামো fetch *fetch, fetch_callback callback, শূন্য *p);

/**
 * Get the HTTP response code.
 */
http_response_code fetch_http_code(কাঠামো fetch *fetch);


/**
 * Free a linked list of fetch_multipart_data.
 *
 * \param list Pointer to head of list to free
 */
শূন্য fetch_multipart_তথ্য_destroy(কাঠামো fetch_multipart_তথ্য *list);

/**
 * Clone a linked list of fetch_multipart_data.
 *
 * \param list  List to clone
 * \return Pointer to head of cloned list, or NULL on failure
 */
কাঠামো fetch_multipart_তথ্য *fetch_multipart_তথ্য_clone(ধ্রুবক কাঠামো fetch_multipart_তথ্য *list);

/**
 * Find an entry in a fetch_multipart_data
 *
 * \param list Pointer to the multipart list
 * \param name The name to look for in the list
 * \return The value found, or NULL if not present
 */
ধ্রুবক অক্ষর *fetch_multipart_তথ্য_find(ধ্রুবক কাঠামো fetch_multipart_তথ্য *list,
				      ধ্রুবক অক্ষর *name);

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
nserror fetch_multipart_তথ্য_new_kv(কাঠামো fetch_multipart_তথ্য **list,
				    ধ্রুবক অক্ষর *name,
				    ধ্রুবক অক্ষর *মান);

/**
 * send message to fetch
 */
শূন্য fetch_send_callback(ধ্রুবক fetch_বার্তা *বার্তা, কাঠামো fetch *fetch);

/**
 * remove a queued fetch
 */
শূন্য fetch_remove_from_queues(কাঠামো fetch *fetch);

/**
 * Free a fetch structure and associated resources.
 */
শূন্য fetch_free(কাঠামো fetch *f);

/**
 * set the http code of a fetch
 */
nserror fetch_set_http_code(কাঠামো fetch *fetch, http_response_code http_code);

/**
 * set cookie data on a fetch
 */
শূন্য fetch_set_cookie(কাঠামো fetch *fetch, ধ্রুবক অক্ষর *তথ্য);

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
nserror fetch_fdset(fd_set *read_fd_set, fd_set *write_fd_set, fd_set *except_fd_set, পূর্ণসংখ্যা *maxfd);

#endif
