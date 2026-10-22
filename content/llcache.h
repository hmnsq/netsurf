/*
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
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
 * Low-level resource cache (interface)
 */

#ifndef NETSURF_CONTENT_LLCACHE_H_
#define NETSURF_CONTENT_LLCACHE_H_

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdint.h>

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/nsurl.h"

ഘടന cert_chain;
ഘടന fetch_multipart_data;

/** Handle for low-level cache object */
തരനിർവചനം ഘടന llcache_handle llcache_handle;

/** POST data object for low-level cache requests */
തരനിർവചനം ഘടന llcache_post_data {
	എണ്ണൽ {
		LLCACHE_POST_URL_ENCODED,
		LLCACHE_POST_MULTIPART
	} type;				/**< Type of POST data */
	സംഘം {
		അക്ഷരം *urlenc;		/**< URL encoded data */
		ഘടന fetch_multipart_data *multipart; /**< Multipart data */
	} data;				/**< POST data content */
} llcache_post_data;

/** Flags for low-level cache object retrieval */
എണ്ണൽ llcache_retrieve_flag {
	/* Note: We're permitted a maximum of 16 flags which must reside in the
	 * bottom 16 bits of the flags word. See hlcache.h for further details.
	 */
	/** Force a new fetch */
	LLCACHE_RETRIEVE_FORCE_FETCH    = (1 << 0),
	/** Requested URL was verified */
	LLCACHE_RETRIEVE_VERIFIABLE     = (1 << 1),
	/**< No error pages */
	LLCACHE_RETRIEVE_NO_ERROR_PAGES = (1 << 2),
	/**< Stream data (implies that object is not cacheable) */
	LLCACHE_RETRIEVE_STREAM_DATA    = (1 << 3)
};

/** Low-level cache event types */
തരനിർവചനം എണ്ണൽ {
	LLCACHE_EVENT_GOT_CERTS,        /**< SSL certificates arrived */
	LLCACHE_EVENT_HAD_HEADERS,	/**< Received all headers */
	LLCACHE_EVENT_HAD_DATA,		/**< Received some data */
	LLCACHE_EVENT_DONE,		/**< Finished fetching data */

	LLCACHE_EVENT_ERROR,		/**< An error occurred during fetch */
	LLCACHE_EVENT_PROGRESS,		/**< Fetch progress update */

	LLCACHE_EVENT_REDIRECT		/**< Fetch URL redirect occured */
} llcache_event_type;

/**
 * Low-level cache events.
 *
 * Lifetime of contained information is only for the duration of the event
 * and must be copied if it is desirable to retain.
 */
തരനിർവചനം ഘടന {
	llcache_event_type type;            /**< Type of event */
	സംഘം {
		ഘടന {
			സ്ഥിരം uint8_t *buf; /**< Buffer of data */
			size_t len;	    /**< Byte length of buffer */
		} data;                     /**< Received data */
		ഘടന {
			nserror code;       /**< The error code */
			സ്ഥിരം അക്ഷരം *സന്ദേശം;    /**< Error message */
		} error;
		സ്ഥിരം അക്ഷരം *progress_സന്ദേശം;   /**< Progress message */
		ഘടന {
			nsurl *from;	    /**< Redirect origin */
			nsurl *to;	    /**< Redirect target */
		} redirect;                 /**< Fetch URL redirect occured */
		സ്ഥിരം ഘടന cert_chain *chain;    /**< Certificate chain */
	} data;				    /**< Event data */
} llcache_event;

/**
 * Client callback for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event   Event data
 * \param pw      Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
തരനിർവചനം nserror (*llcache_handle_callback)(llcache_handle *handle,
		സ്ഥിരം llcache_event *event, ശൂന്യം *pw);

/**
 * Parameters to configure the low level cache backing store.
 */
ഘടന llcache_store_parameters {
	സ്ഥിരം അക്ഷരം *path; /**< The path to the backing store */

	size_t limit; /**< The backing store upper bound target size */
	size_t hysteresis; /**< The hysteresis around the target size */
};

/**
 * Parameters to configure the low level cache.
 */
ഘടന llcache_parameters {
	size_t limit; /**< The target upper bound for the RAM cache size */
	size_t hysteresis; /**< The hysteresis around the target size */

	/** The minimum lifetime to consider sending objects to backing store.*/
	പൂർണ്ണസംഖ്യ minimum_lifetime;

	/** The minimum bandwidth to allow the backing store to
	 * use in bytes/second
	 */
	size_t minimum_bandwidth;

	/** The maximum bandwidth to allow the backing store to use in
	 * bytes/second
	 */
	size_t maximum_bandwidth;

	/** The time quantum over which to calculate the bandwidth values
	 */
	unsigned long time_quantum;

	/** The number of fetches to attempt when timing out */
	uint32_t fetch_attempts;

	ഘടന llcache_store_parameters store;
};

/**
 * Initialise the low-level cache
 *
 * \param parameters cache configuration parameters.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror llcache_initialise(സ്ഥിരം ഘടന llcache_parameters *parameters);

/**
 * Finalise the low-level cache
 */
ശൂന്യം llcache_finalise(ശൂന്യം);

/**
 * Cause the low-level cache to attempt to perform cleanup.
 *
 * No guarantees are made as to whether or not cleanups will take
 * place and what, if any, space savings will be made.
 *
 * \param purge Any objects held in the cache that are safely removable will
 *              be freed regardless of the configured size limits.
 */
ശൂന്യം llcache_clean(bool purge);

/**
 * Retrieve a handle for a low-level cache object
 *
 * \param url      URL of the object to fetch
 * \param flags    Object retrieval flags
 * \param referer  Referring URL, or NULL if none
 * \param post     POST data, or NULL for a GET request
 * \param cb       Client callback for events
 * \param pw       Pointer to client-specific data
 * \param result   Pointer to location to recieve cache handle
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_retrieve(nsurl *url, uint32_t flags,
		nsurl *referer, സ്ഥിരം llcache_post_data *post,
		llcache_handle_callback cb, ശൂന്യം *pw,
		llcache_handle **result);

/**
 * Change the callback associated with a low-level cache handle
 *
 * \param handle  Handle to change callback of
 * \param cb      New callback
 * \param pw      Client data for new callback
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, ശൂന്യം *pw);

/**
 * Release a low-level cache handle
 *
 * \param handle  Handle to release
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_release(llcache_handle *handle);

/**
 * Clone a low-level cache handle, producing a new handle to
 * the same fetch/content.
 *
 * \param handle  Handle to clone
 * \param result  Pointer to location to receive cloned handle
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **result);

/**
 * Abort a low-level fetch, informing all users of this action.
 *
 * \param handle  Handle to abort
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_abort(llcache_handle *handle);

/**
 * Force a low-level cache handle into streaming mode
 *
 * \param handle  Handle to stream
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_force_stream(llcache_handle *handle);

/**
 * Invalidate cache data for a low-level cache object
 *
 * \param handle  Handle to invalidate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_invalidate_cache_data(llcache_handle *handle);

/**
 * Retrieve the post-redirect URL of a low-level cache object
 *
 * \param handle  Handle to retrieve URL from
 * \return Post-redirect URL of cache object or NULL if no object
 */
nsurl *llcache_handle_get_url(സ്ഥിരം llcache_handle *handle);

/**
 * Retrieve source data of a low-level cache object
 *
 * \param handle  Handle to retrieve source data from
 * \param size    Pointer to location to receive byte length of data
 * \return Pointer to source data
 */
സ്ഥിരം uint8_t *llcache_handle_get_source_data(സ്ഥിരം llcache_handle *handle,
		size_t *size);

/**
 * Retrieve a header value associated with a low-level cache object
 *
 * \param handle  Handle to retrieve header from
 * \param key     Header name
 * \return Header value, or NULL if header does not exist
 *
 * \todo Make the key an enumeration, to avoid needless string comparisons
 * \todo Forcing the client to parse the header value seems wrong.
 *       Better would be to return the actual value part and an array of
 *       key-value pairs for any additional parameters.
 * \todo Deal with multiple headers of the same key (e.g. Set-Cookie)
 */
സ്ഥിരം അക്ഷരം *llcache_handle_get_header(സ്ഥിരം llcache_handle *handle,
		സ്ഥിരം അക്ഷരം *താക്കോൽ);

/**
 * Determine if the same underlying object is referenced by the given handles
 *
 * \param a  First handle
 * \param b  Second handle
 * \return True if handles reference the same object, false otherwise
 */
bool llcache_handle_references_same_object(സ്ഥിരം llcache_handle *a,
		സ്ഥിരം llcache_handle *b);

#endif
