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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdint.h>

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/nsurl.h"

কাঠামো cert_chain;
কাঠামো fetch_multipart_তথ্য;

/** Handle for low-level cache object */
প্রকারলেখ কাঠামো llcache_handle llcache_handle;

/** POST data object for low-level cache requests */
প্রকারলেখ কাঠামো llcache_post_তথ্য {
	গণনা {
		LLCACHE_POST_URL_ENCODED,
		LLCACHE_POST_MULTIPART
	} type;				/**< Type of POST data */
	সংঘ {
		অক্ষর *urlenc;		/**< URL encoded data */
		কাঠামো fetch_multipart_তথ্য *multipart; /**< Multipart data */
	} তথ্য;				/**< POST data content */
} llcache_post_তথ্য;

/** Flags for low-level cache object retrieval */
গণনা llcache_retrieve_flag {
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
	LLCACHE_RETRIEVE_STREAM_তথ্য    = (1 << 3)
};

/** Low-level cache event types */
প্রকারলেখ গণনা {
	LLCACHE_EVENT_GOT_CERTS,        /**< SSL certificates arrived */
	LLCACHE_EVENT_HAD_HEADERS,	/**< Received all headers */
	LLCACHE_EVENT_HAD_তথ্য,		/**< Received some data */
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
প্রকারলেখ কাঠামো {
	llcache_event_type type;            /**< Type of event */
	সংঘ {
		কাঠামো {
			ধ্রুবক uint8_t *buf; /**< Buffer of data */
			size_t len;	    /**< Byte length of buffer */
		} তথ্য;                     /**< Received data */
		কাঠামো {
			nserror code;       /**< The error code */
			ধ্রুবক অক্ষর *বার্তা;    /**< Error message */
		} error;
		ধ্রুবক অক্ষর *progress_বার্তা;   /**< Progress message */
		কাঠামো {
			nsurl *from;	    /**< Redirect origin */
			nsurl *to;	    /**< Redirect target */
		} redirect;                 /**< Fetch URL redirect occured */
		ধ্রুবক কাঠামো cert_chain *chain;    /**< Certificate chain */
	} তথ্য;				    /**< Event data */
} llcache_event;

/**
 * Client callback for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event   Event data
 * \param pw      Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
প্রকারলেখ nserror (*llcache_handle_callback)(llcache_handle *handle,
		ধ্রুবক llcache_event *event, শূন্য *pw);

/**
 * Parameters to configure the low level cache backing store.
 */
কাঠামো llcache_store_parameters {
	ধ্রুবক অক্ষর *path; /**< The path to the backing store */

	size_t limit; /**< The backing store upper bound target size */
	size_t hysteresis; /**< The hysteresis around the target size */
};

/**
 * Parameters to configure the low level cache.
 */
কাঠামো llcache_parameters {
	size_t limit; /**< The target upper bound for the RAM cache size */
	size_t hysteresis; /**< The hysteresis around the target size */

	/** The minimum lifetime to consider sending objects to backing store.*/
	পূর্ণসংখ্যা minimum_lifetime;

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

	কাঠামো llcache_store_parameters store;
};

/**
 * Initialise the low-level cache
 *
 * \param parameters cache configuration parameters.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror llcache_initialise(ধ্রুবক কাঠামো llcache_parameters *parameters);

/**
 * Finalise the low-level cache
 */
শূন্য llcache_finalise(শূন্য);

/**
 * Cause the low-level cache to attempt to perform cleanup.
 *
 * No guarantees are made as to whether or not cleanups will take
 * place and what, if any, space savings will be made.
 *
 * \param purge Any objects held in the cache that are safely removable will
 *              be freed regardless of the configured size limits.
 */
শূন্য llcache_clean(bool purge);

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
		nsurl *referer, ধ্রুবক llcache_post_তথ্য *post,
		llcache_handle_callback cb, শূন্য *pw,
		llcache_handle **ফলাফল);

/**
 * Change the callback associated with a low-level cache handle
 *
 * \param handle  Handle to change callback of
 * \param cb      New callback
 * \param pw      Client data for new callback
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, শূন্য *pw);

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
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **ফলাফল);

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
nserror llcache_handle_invalidate_cache_তথ্য(llcache_handle *handle);

/**
 * Retrieve the post-redirect URL of a low-level cache object
 *
 * \param handle  Handle to retrieve URL from
 * \return Post-redirect URL of cache object or NULL if no object
 */
nsurl *llcache_handle_get_url(ধ্রুবক llcache_handle *handle);

/**
 * Retrieve source data of a low-level cache object
 *
 * \param handle  Handle to retrieve source data from
 * \param size    Pointer to location to receive byte length of data
 * \return Pointer to source data
 */
ধ্রুবক uint8_t *llcache_handle_get_source_তথ্য(ধ্রুবক llcache_handle *handle,
		size_t *আকার_2);

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
ধ্রুবক অক্ষর *llcache_handle_get_header(ধ্রুবক llcache_handle *handle,
		ধ্রুবক অক্ষর *চাবি);

/**
 * Determine if the same underlying object is referenced by the given handles
 *
 * \param a  First handle
 * \param b  Second handle
 * \return True if handles reference the same object, false otherwise
 */
bool llcache_handle_references_same_object(ধ্রুবক llcache_handle *a,
		ধ্রুবক llcache_handle *b);

#endif
