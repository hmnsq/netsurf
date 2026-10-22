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

#جے_بنا_تعریف NETSURF_CONTENT_LLCACHE_H_
#تعریف_کرو NETSURF_CONTENT_LLCACHE_H_

#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdint.h>

#ضمن "utils/errors.h"
#ضمن "utils/nsurl.h"

بنية cert_chain;
بنية fetch_multipart_انکڑے;

/** Handle for low-level cache object */
تعريفنوع بنية llcache_handle llcache_handle;

/** POST data object for low-level cache requests */
تعريفنوع بنية llcache_post_انکڑے {
	تعداد {
		LLCACHE_POST_URL_ENCODED,
		LLCACHE_POST_MULTIPART
	} type;				/**< Type of POST data */
	اتحاد {
		حرف *urlenc;		/**< URL encoded data */
		بنية fetch_multipart_انکڑے *multipart; /**< Multipart data */
	} انکڑے;				/**< POST data content */
} llcache_post_انکڑے;

/** Flags for low-level cache object retrieval */
تعداد llcache_retrieve_flag {
	/* Note: We're permitted a maximum of 16 flags which must reside in the
	 * bottom 16 bits of the flags word. See hlcache.h for further details.
	 */
	/** Force a new fetch */
	LLCACHE_RETRIEVE_FORCE_FETCH    = (1 << 0),
	/** Requested URL was verified */
	LLCACHE_RETRIEVE_VERIFIABLE     = (1 << 1),
	/**< No error pages */
	LLCACHE_RETRIEVE_NO_ERROR_ورقے = (1 << 2),
	/**< Stream data (implies that object is not cacheable) */
	LLCACHE_RETRIEVE_دھارا_انکڑے    = (1 << 3)
};

/** Low-level cache event types */
تعريفنوع تعداد {
	LLCACHE_EVENT_GOT_CERTS,        /**< SSL certificates arrived */
	LLCACHE_EVENT_HAD_سرخیاں,	/**< Received all headers */
	LLCACHE_EVENT_HAD_انکڑے,		/**< Received some data */
	LLCACHE_EVENT_پورا,		/**< Finished fetching data */

	LLCACHE_EVENT_ERROR,		/**< An error occurred during fetch */
	LLCACHE_EVENT_ترقی,		/**< Fetch progress update */

	LLCACHE_EVENT_REDIRECT		/**< Fetch URL redirect occured */
} llcache_event_type;

/**
 * Low-level cache events.
 *
 * Lifetime of contained information is only for the duration of the event
 * and must be copied if it is desirable to retain.
 */
تعريفنوع بنية {
	llcache_event_type type;            /**< Type of event */
	اتحاد {
		بنية {
			ثابت uint8_t *buf; /**< Buffer of data */
			size_t len;	    /**< Byte length of buffer */
		} انکڑے;                     /**< Received data */
		بنية {
			nserror کوڈ;       /**< The error code */
			ثابت حرف *پیغام;    /**< Error message */
		} error;
		ثابت حرف *ترقی_پیغام;   /**< Progress message */
		بنية {
			nsurl *from;	    /**< Redirect origin */
			nsurl *منزل;	    /**< Redirect target */
		} redirect;                 /**< Fetch URL redirect occured */
		ثابت بنية cert_chain *chain;    /**< Certificate chain */
	} انکڑے;				    /**< Event data */
} llcache_event;

/**
 * Client callback for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event   Event data
 * \param pw      Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
تعريفنوع nserror (*llcache_handle_callback)(llcache_handle *handle,
		ثابت llcache_event *event, فراغ *pw);

/**
 * Parameters to configure the low level cache backing store.
 */
بنية llcache_store_parameters {
	ثابت حرف *رستہ; /**< The path to the backing store */

	size_t حد_بندی; /**< The backing store upper bound target size */
	size_t hysteresis; /**< The hysteresis around the target size */
};

/**
 * Parameters to configure the low level cache.
 */
بنية llcache_parameters {
	size_t حد_بندی; /**< The target upper bound for the RAM cache size */
	size_t hysteresis; /**< The hysteresis around the target size */

	/** The minimum lifetime to consider sending objects to backing store.*/
	صحيح minimum_lifetime;

	/** The minimum bandwidth to allow the backing store to
	 * use in bytes/second
	 */
	size_t minimum_bandwidth;

	/** The maximum bandwidth to allow the backing store to use in
	 * bytes/second
	 */
	size_t ودھ_توں_ودھ_bandwidth;

	/** The time quantum over which to calculate the bandwidth values
	 */
	بنانشانی لما ویلا_ویلا_حصہ;

	/** The number of fetches to attempt when timing out */
	uint32_t fetch_attempts;

	بنية llcache_store_parameters store;
};

/**
 * Initialise the low-level cache
 *
 * \param parameters cache configuration parameters.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror llcache_initialise(ثابت بنية llcache_parameters *parameters);

/**
 * Finalise the low-level cache
 */
فراغ llcache_finalise(فراغ);

/**
 * Cause the low-level cache to attempt to perform cleanup.
 *
 * No guarantees are made as to whether or not cleanups will take
 * place and what, if any, space savings will be made.
 *
 * \param purge Any objects held in the cache that are safely removable will
 *              be freed regardless of the configured size limits.
 */
فراغ llcache_clean(bool purge);

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
		nsurl *referer, ثابت llcache_post_انکڑے *post,
		llcache_handle_callback cb, فراغ *pw,
		llcache_handle **نتیجہ);

/**
 * Change the callback associated with a low-level cache handle
 *
 * \param handle  Handle to change callback of
 * \param cb      New callback
 * \param pw      Client data for new callback
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, فراغ *pw);

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
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **نتیجہ);

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
nserror llcache_handle_force_دھارا(llcache_handle *handle);

/**
 * Invalidate cache data for a low-level cache object
 *
 * \param handle  Handle to invalidate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_invalidate_cache_انکڑے(llcache_handle *handle);

/**
 * Retrieve the post-redirect URL of a low-level cache object
 *
 * \param handle  Handle to retrieve URL from
 * \return Post-redirect URL of cache object or NULL if no object
 */
nsurl *llcache_handle_get_url(ثابت llcache_handle *handle);

/**
 * Retrieve source data of a low-level cache object
 *
 * \param handle  Handle to retrieve source data from
 * \param size    Pointer to location to receive byte length of data
 * \return Pointer to source data
 */
ثابت uint8_t *llcache_handle_get_سروت_انکڑے(ثابت llcache_handle *handle,
		size_t *ناپ);

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
ثابت حرف *llcache_handle_get_سرخی(ثابت llcache_handle *handle,
		ثابت حرف *کلید);

/**
 * Determine if the same underlying object is referenced by the given handles
 *
 * \param a  First handle
 * \param b  Second handle
 * \return True if handles reference the same object, false otherwise
 */
bool llcache_handle_references_same_object(ثابت llcache_handle *a,
		ثابت llcache_handle *b);

#جے_انت
