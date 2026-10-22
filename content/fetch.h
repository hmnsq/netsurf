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

#నిర్వచితం_కాకపోతే _NETSURF_DESKTOP_FETCH_H_
#నిర్వచించు _NETSURF_DESKTOP_FETCH_H_

#అన్తర్భావ <stdbool.h>

#అన్తర్భావ "utils/config.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/inet.h"
#అన్తర్భావ "utils/http/response-codes.h"

#అన్తర్భావ "netsurf/ssl_certs.h"

నిర్మాణం content;
నిర్మాణం fetch;
నిర్మాణం ssl_cert_info;

/**
 * Fetcher message types
 */
రకనిర్వచనం గణన {
	FETCH_పురోగతి,
	FETCH_CERTS,
	FETCH_శీర్షిక,
	FETCH_దత్తాంశం,
	/* Anything after here is a completed fetch of some kind. */
	FETCH_ముగిసిన,
	FETCH_TIMEDOUT,
	FETCH_ERROR,
	FETCH_REDIRECT,
	FETCH_NOTMODIFIED,
	FETCH_AUTH,
	FETCH_CERT_ERR,
	FETCH_SSL_ERR
} fetch_సందేశం_type;

/** Minimum finished message type.
 *
 * If a fetch does not progress this far, it's an error and the fetch machinery
 * will send FETCH_ERROR to the llcache on fetch_free()
 */
#నిర్వచించు FETCH_MIN_ముగిసిన_సందేశం FETCH_ముగిసిన

/**
 * This message is actually an internal message used to indicate
 * that a fetch was aborted.  Do not send this, nor expect it.
 */
#నిర్వచించు FETCH_అంతర్గత_ABORTED FETCH_ERROR

/**
 * Fetcher message data
 */
రకనిర్వచనం నిర్మాణం fetch_సందేశం {
	fetch_సందేశం_type type;

	సంఘం {
		స్థిరరాశి అక్షరం *పురోగతి;

		నిర్మాణం {
			స్థిరరాశి uint8_t *buf;
			size_t len;
		} శీర్షిక_or_దత్తాంశం;

		స్థిరరాశి అక్షరం *error;

		/** \todo Use nsurl */
		స్థిరరాశి అక్షరం *redirect;

		నిర్మాణం {
			స్థిరరాశి అక్షరం *realm;
		} auth;

		స్థిరరాశి నిర్మాణం cert_chain *chain;
	} దత్తాంశం;
} fetch_సందేశం;

/**
 * Fetcher post data types
 */
రకనిర్వచనం గణన {
	FETCH_POSTDATA_NONE,
	FETCH_POSTDATA_URLENC,
	FETCH_POSTDATA_MULTIPART,
} fetch_postdata_type;


/**
 * Fetch POST multipart data
 */
నిర్మాణం fetch_multipart_దత్తాంశం {
	నిర్మాణం fetch_multipart_దత్తాంశం *తదుపరి; /**< Next in linked list */

	అక్షరం *name; /**< Name of item */
	అక్షరం *విలువ; /**< Item value */

	అక్షరం *rawfile; /**< Raw filename if file is true */
	bool file; /**< Item is a file */
};

/**
 * fetch POST data
 */
నిర్మాణం fetch_postdata {
	fetch_postdata_type type;
	సంఘం {
		/** Url encoded POST string if type is FETCH_POSTDATA_URLENC */
		అక్షరం *urlenc;
		/** Multipart post data if type is FETCH_POSTDATA_MULTIPART */
		నిర్మాణం fetch_multipart_దత్తాంశం *multipart;
	} దత్తాంశం;
};


రకనిర్వచనం శూన్య (*fetch_callback)(స్థిరరాశి fetch_సందేశం *సందేశం, శూన్య *p);

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
nserror fetch_ప్రారంభం(nsurl *url, nsurl *referer, fetch_callback callback,
		    శూన్య *p, bool only_2xx, స్థిరరాశి అక్షరం *post_urlenc,
		    స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *post_multipart,
		    bool verifiable, bool downgrade_tls,
		    స్థిరరాశి అక్షరం *శీర్షికలు[], నిర్మాణం fetch **fetch_బహిర్గతం);

/**
 * Abort a fetch.
 */
శూన్య fetch_abort(నిర్మాణం fetch *f);


/**
 * Check if a URL's scheme can be fetched.
 *
 * \param  url  URL to check
 * \return  true if the scheme is supported
 */
bool fetch_can_fetch(స్థిరరాశి nsurl *url);

/**
 * Change the callback function for a fetch.
 */
శూన్య fetch_change_callback(నిర్మాణం fetch *fetch, fetch_callback callback, శూన్య *p);

/**
 * Get the HTTP response code.
 */
http_response_సంకేతం fetch_http_సంకేతం(నిర్మాణం fetch *fetch);


/**
 * Free a linked list of fetch_multipart_data.
 *
 * \param list Pointer to head of list to free
 */
శూన్య fetch_multipart_దత్తాంశం_destroy(నిర్మాణం fetch_multipart_దత్తాంశం *list);

/**
 * Clone a linked list of fetch_multipart_data.
 *
 * \param list  List to clone
 * \return Pointer to head of cloned list, or NULL on failure
 */
నిర్మాణం fetch_multipart_దత్తాంశం *fetch_multipart_దత్తాంశం_clone(స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *list);

/**
 * Find an entry in a fetch_multipart_data
 *
 * \param list Pointer to the multipart list
 * \param name The name to look for in the list
 * \return The value found, or NULL if not present
 */
స్థిరరాశి అక్షరం *fetch_multipart_దత్తాంశం_కనుగొను(స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *list,
				      స్థిరరాశి అక్షరం *name);

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
nserror fetch_multipart_దత్తాంశం_new_kv(నిర్మాణం fetch_multipart_దత్తాంశం **list,
				    స్థిరరాశి అక్షరం *name,
				    స్థిరరాశి అక్షరం *విలువ);

/**
 * send message to fetch
 */
శూన్య fetch_send_callback(స్థిరరాశి fetch_సందేశం *సందేశం, నిర్మాణం fetch *fetch);

/**
 * remove a queued fetch
 */
శూన్య fetch_remove_from_queues(నిర్మాణం fetch *fetch);

/**
 * Free a fetch structure and associated resources.
 */
శూన్య fetch_free(నిర్మాణం fetch *f);

/**
 * set the http code of a fetch
 */
nserror fetch_సమితి_http_సంకేతం(నిర్మాణం fetch *fetch, http_response_సంకేతం http_సంకేతం);

/**
 * set cookie data on a fetch
 */
శూన్య fetch_సమితి_cookie(నిర్మాణం fetch *fetch, స్థిరరాశి అక్షరం *దత్తాంశం);

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
nserror fetch_fdset(fd_set *read_fd_సమితి, fd_set *write_fd_సమితి, fd_set *except_fd_సమితి, పూర్ణాంకం *maxfd);

#అయితే_ముగింపు
