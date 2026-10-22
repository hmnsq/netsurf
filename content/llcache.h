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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_CONTENT_LLCACHE_H_
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_CONTENT_LLCACHE_H_

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"

ರಚನೆ cert_chain;
ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ;

/** Handle for low-level cache object */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ llcache_handle llcache_handle;

/** POST data object for low-level cache requests */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ llcache_post_ದತ್ತಾಂಶ {
	ಎಣಿಕೆ {
		LLCACHE_POST_URL_ENCODED,
		LLCACHE_POST_MULTIPART
	} type;				/**< Type of POST data */
	ಒಕ್ಕೂಟ {
		ಅಕ್ಷರ *urlenc;		/**< URL encoded data */
		ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *multipart; /**< Multipart data */
	} ದತ್ತಾಂಶ;				/**< POST data content */
} llcache_post_ದತ್ತಾಂಶ;

/** Flags for low-level cache object retrieval */
ಎಣಿಕೆ llcache_retrieve_flag {
	/* Note: We're permitted a maximum of 16 flags which must reside in the
	 * bottom 16 bits of the flags word. See hlcache.h for further details.
	 */
	/** Force a new fetch */
	LLCACHE_RETRIEVE_FORCE_FETCH    = (1 << 0),
	/** Requested URL was verified */
	LLCACHE_RETRIEVE_VERIFIABLE     = (1 << 1),
	/**< No error pages */
	LLCACHE_RETRIEVE_NO_ERROR_ಪುಟಗಳು = (1 << 2),
	/**< Stream data (implies that object is not cacheable) */
	LLCACHE_RETRIEVE_ಹರಿವು_ದತ್ತಾಂಶ    = (1 << 3)
};

/** Low-level cache event types */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	LLCACHE_EVENT_GOT_CERTS,        /**< SSL certificates arrived */
	LLCACHE_EVENT_HAD_ಶೀರ್ಷಿಕೆಗಳು,	/**< Received all headers */
	LLCACHE_EVENT_HAD_ದತ್ತಾಂಶ,		/**< Received some data */
	LLCACHE_EVENT_ಮುಗಿದ,		/**< Finished fetching data */

	LLCACHE_EVENT_ERROR,		/**< An error occurred during fetch */
	LLCACHE_EVENT_ಪ್ರಗತಿ,		/**< Fetch progress update */

	LLCACHE_EVENT_REDIRECT		/**< Fetch URL redirect occured */
} llcache_event_type;

/**
 * Low-level cache events.
 *
 * Lifetime of contained information is only for the duration of the event
 * and must be copied if it is desirable to retain.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
	llcache_event_type type;            /**< Type of event */
	ಒಕ್ಕೂಟ {
		ರಚನೆ {
			ಸ್ಥಿರಾಂಕ uint8_t *buf; /**< Buffer of data */
			size_t len;	    /**< Byte length of buffer */
		} ದತ್ತಾಂಶ;                     /**< Received data */
		ರಚನೆ {
			nserror ಸಂಕೇತ_2;       /**< The error code */
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ;    /**< Error message */
		} error;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪ್ರಗತಿ_ಸಂದೇಶ;   /**< Progress message */
		ರಚನೆ {
			nsurl *from;	    /**< Redirect origin */
			nsurl *ಗಮ್ಯ;	    /**< Redirect target */
		} redirect;                 /**< Fetch URL redirect occured */
		ಸ್ಥಿರಾಂಕ ರಚನೆ cert_chain *chain;    /**< Certificate chain */
	} ದತ್ತಾಂಶ;				    /**< Event data */
} llcache_event;

/**
 * Client callback for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event   Event data
 * \param pw      Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ nserror (*llcache_handle_callback)(llcache_handle *handle,
		ಸ್ಥಿರಾಂಕ llcache_event *event, ಶೂನ್ಯ *pw);

/**
 * Parameters to configure the low level cache backing store.
 */
ರಚನೆ llcache_store_parameters {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ; /**< The path to the backing store */

	size_t ಎಲ್ಲೆ; /**< The backing store upper bound target size */
	size_t hysteresis; /**< The hysteresis around the target size */
};

/**
 * Parameters to configure the low level cache.
 */
ರಚನೆ llcache_parameters {
	size_t ಎಲ್ಲೆ; /**< The target upper bound for the RAM cache size */
	size_t hysteresis; /**< The hysteresis around the target size */

	/** The minimum lifetime to consider sending objects to backing store.*/
	ಪೂರ್ಣಾಂಕ minimum_lifetime;

	/** The minimum bandwidth to allow the backing store to
	 * use in bytes/second
	 */
	size_t minimum_bandwidth;

	/** The maximum bandwidth to allow the backing store to use in
	 * bytes/second
	 */
	size_t ಗರಿಷ್ಠ_bandwidth;

	/** The time quantum over which to calculate the bandwidth values
	 */
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಸಮಯ_ಕಾಲಾಂಶ;

	/** The number of fetches to attempt when timing out */
	uint32_t fetch_attempts;

	ರಚನೆ llcache_store_parameters store;
};

/**
 * Initialise the low-level cache
 *
 * \param parameters cache configuration parameters.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror llcache_initialise(ಸ್ಥಿರಾಂಕ ರಚನೆ llcache_parameters *parameters);

/**
 * Finalise the low-level cache
 */
ಶೂನ್ಯ llcache_finalise(ಶೂನ್ಯ);

/**
 * Cause the low-level cache to attempt to perform cleanup.
 *
 * No guarantees are made as to whether or not cleanups will take
 * place and what, if any, space savings will be made.
 *
 * \param purge Any objects held in the cache that are safely removable will
 *              be freed regardless of the configured size limits.
 */
ಶೂನ್ಯ llcache_clean(bool purge);

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
		nsurl *referer, ಸ್ಥಿರಾಂಕ llcache_post_ದತ್ತಾಂಶ *post,
		llcache_handle_callback cb, ಶೂನ್ಯ *pw,
		llcache_handle **ಫಲಿತಾಂಶ);

/**
 * Change the callback associated with a low-level cache handle
 *
 * \param handle  Handle to change callback of
 * \param cb      New callback
 * \param pw      Client data for new callback
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, ಶೂನ್ಯ *pw);

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
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **ಫಲಿತಾಂಶ);

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
nserror llcache_handle_force_ಹರಿವು(llcache_handle *handle);

/**
 * Invalidate cache data for a low-level cache object
 *
 * \param handle  Handle to invalidate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_invalidate_cache_ದತ್ತಾಂಶ(llcache_handle *handle);

/**
 * Retrieve the post-redirect URL of a low-level cache object
 *
 * \param handle  Handle to retrieve URL from
 * \return Post-redirect URL of cache object or NULL if no object
 */
nsurl *llcache_handle_get_url(ಸ್ಥಿರಾಂಕ llcache_handle *handle);

/**
 * Retrieve source data of a low-level cache object
 *
 * \param handle  Handle to retrieve source data from
 * \param size    Pointer to location to receive byte length of data
 * \return Pointer to source data
 */
ಸ್ಥಿರಾಂಕ uint8_t *llcache_handle_get_ಮೂಲ_ದತ್ತಾಂಶ(ಸ್ಥಿರಾಂಕ llcache_handle *handle,
		size_t *ಗಾತ್ರ_2);

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
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *llcache_handle_get_ಶೀರ್ಷಿಕೆ(ಸ್ಥಿರಾಂಕ llcache_handle *handle,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ);

/**
 * Determine if the same underlying object is referenced by the given handles
 *
 * \param a  First handle
 * \param b  Second handle
 * \return True if handles reference the same object, false otherwise
 */
bool llcache_handle_references_same_object(ಸ್ಥಿರಾಂಕ llcache_handle *a,
		ಸ್ಥಿರಾಂಕ llcache_handle *b);

#ಆದರೆ_ಅಂತ್ಯ
