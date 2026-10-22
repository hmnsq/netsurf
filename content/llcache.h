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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_CONTENT_LLCACHE_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_CONTENT_LLCACHE_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"

ਬਣਤਰ cert_chain;
ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ;

/** Handle for low-level cache object */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ llcache_handle llcache_handle;

/** POST data object for low-level cache requests */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ llcache_post_ਅੰਕੜੇ {
	ਗਿਣਤੀ {
		LLCACHE_POST_URL_ENCODED,
		LLCACHE_POST_MULTIPART
	} type;				/**< Type of POST data */
	ਸੰਘ {
		ਅੱਖਰ *urlenc;		/**< URL encoded data */
		ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *multipart; /**< Multipart data */
	} ਅੰਕੜੇ;				/**< POST data content */
} llcache_post_ਅੰਕੜੇ;

/** Flags for low-level cache object retrieval */
ਗਿਣਤੀ llcache_retrieve_flag {
	/* Note: We're permitted a maximum of 16 flags which must reside in the
	 * bottom 16 bits of the flags word. See hlcache.h for further details.
	 */
	/** Force a new fetch */
	LLCACHE_RETRIEVE_FORCE_FETCH    = (1 << 0),
	/** Requested URL was verified */
	LLCACHE_RETRIEVE_VERIFIABLE     = (1 << 1),
	/**< No error pages */
	LLCACHE_RETRIEVE_NO_ERROR_ਪੰਨੇ = (1 << 2),
	/**< Stream data (implies that object is not cacheable) */
	LLCACHE_RETRIEVE_ਧਾਰਾ_ਅੰਕੜੇ    = (1 << 3)
};

/** Low-level cache event types */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	LLCACHE_EVENT_GOT_CERTS,        /**< SSL certificates arrived */
	LLCACHE_EVENT_HAD_ਸਿਰਲੇਖਾਂ,	/**< Received all headers */
	LLCACHE_EVENT_HAD_ਅੰਕੜੇ,		/**< Received some data */
	LLCACHE_EVENT_ਪੂਰਾ,		/**< Finished fetching data */

	LLCACHE_EVENT_ERROR,		/**< An error occurred during fetch */
	LLCACHE_EVENT_ਤਰੱਕੀ,		/**< Fetch progress update */

	LLCACHE_EVENT_REDIRECT		/**< Fetch URL redirect occured */
} llcache_event_type;

/**
 * Low-level cache events.
 *
 * Lifetime of contained information is only for the duration of the event
 * and must be copied if it is desirable to retain.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
	llcache_event_type type;            /**< Type of event */
	ਸੰਘ {
		ਬਣਤਰ {
			ਅਚਲ uint8_t *buf; /**< Buffer of data */
			size_t len;	    /**< Byte length of buffer */
		} ਅੰਕੜੇ;                     /**< Received data */
		ਬਣਤਰ {
			nserror ਕੋਡ;       /**< The error code */
			ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ;    /**< Error message */
		} error;
		ਅਚਲ ਅੱਖਰ *ਤਰੱਕੀ_ਸੁਨੇਹਾ;   /**< Progress message */
		ਬਣਤਰ {
			nsurl *from;	    /**< Redirect origin */
			nsurl *ਮੰਜ਼ਿਲ;	    /**< Redirect target */
		} redirect;                 /**< Fetch URL redirect occured */
		ਅਚਲ ਬਣਤਰ cert_chain *chain;    /**< Certificate chain */
	} ਅੰਕੜੇ;				    /**< Event data */
} llcache_event;

/**
 * Client callback for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event   Event data
 * \param pw      Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ nserror (*llcache_handle_callback)(llcache_handle *handle,
		ਅਚਲ llcache_event *event, ਖਾਲੀ *pw);

/**
 * Parameters to configure the low level cache backing store.
 */
ਬਣਤਰ llcache_store_parameters {
	ਅਚਲ ਅੱਖਰ *ਰਸਤਾ; /**< The path to the backing store */

	size_t ਹੱਦਬੰਦੀ; /**< The backing store upper bound target size */
	size_t hysteresis; /**< The hysteresis around the target size */
};

/**
 * Parameters to configure the low level cache.
 */
ਬਣਤਰ llcache_parameters {
	size_t ਹੱਦਬੰਦੀ; /**< The target upper bound for the RAM cache size */
	size_t hysteresis; /**< The hysteresis around the target size */

	/** The minimum lifetime to consider sending objects to backing store.*/
	ਪੂਰਨ minimum_lifetime;

	/** The minimum bandwidth to allow the backing store to
	 * use in bytes/second
	 */
	size_t minimum_bandwidth;

	/** The maximum bandwidth to allow the backing store to use in
	 * bytes/second
	 */
	size_t ਵੱਧ_ਤੋਂ_ਵੱਧ_bandwidth;

	/** The time quantum over which to calculate the bandwidth values
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸਮਾਂ_ਸਮਾਂ_ਹਿੱਸਾ;

	/** The number of fetches to attempt when timing out */
	uint32_t fetch_attempts;

	ਬਣਤਰ llcache_store_parameters store;
};

/**
 * Initialise the low-level cache
 *
 * \param parameters cache configuration parameters.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror llcache_initialise(ਅਚਲ ਬਣਤਰ llcache_parameters *parameters);

/**
 * Finalise the low-level cache
 */
ਖਾਲੀ llcache_finalise(ਖਾਲੀ);

/**
 * Cause the low-level cache to attempt to perform cleanup.
 *
 * No guarantees are made as to whether or not cleanups will take
 * place and what, if any, space savings will be made.
 *
 * \param purge Any objects held in the cache that are safely removable will
 *              be freed regardless of the configured size limits.
 */
ਖਾਲੀ llcache_clean(bool purge);

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
		nsurl *referer, ਅਚਲ llcache_post_ਅੰਕੜੇ *post,
		llcache_handle_callback cb, ਖਾਲੀ *pw,
		llcache_handle **ਨਤੀਜਾ);

/**
 * Change the callback associated with a low-level cache handle
 *
 * \param handle  Handle to change callback of
 * \param cb      New callback
 * \param pw      Client data for new callback
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, ਖਾਲੀ *pw);

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
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **ਨਤੀਜਾ);

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
nserror llcache_handle_force_ਧਾਰਾ(llcache_handle *handle);

/**
 * Invalidate cache data for a low-level cache object
 *
 * \param handle  Handle to invalidate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_invalidate_cache_ਅੰਕੜੇ(llcache_handle *handle);

/**
 * Retrieve the post-redirect URL of a low-level cache object
 *
 * \param handle  Handle to retrieve URL from
 * \return Post-redirect URL of cache object or NULL if no object
 */
nsurl *llcache_handle_get_url(ਅਚਲ llcache_handle *handle);

/**
 * Retrieve source data of a low-level cache object
 *
 * \param handle  Handle to retrieve source data from
 * \param size    Pointer to location to receive byte length of data
 * \return Pointer to source data
 */
ਅਚਲ uint8_t *llcache_handle_get_ਸਰੋਤ_ਅੰਕੜੇ(ਅਚਲ llcache_handle *handle,
		size_t *ਮਾਪ);

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
ਅਚਲ ਅੱਖਰ *llcache_handle_get_ਸਿਰਲੇਖ(ਅਚਲ llcache_handle *handle,
		ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ);

/**
 * Determine if the same underlying object is referenced by the given handles
 *
 * \param a  First handle
 * \param b  Second handle
 * \return True if handles reference the same object, false otherwise
 */
bool llcache_handle_references_same_object(ਅਚਲ llcache_handle *a,
		ਅਚਲ llcache_handle *b);

#ਜੇ_ਅੰਤ
