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

#నిర్వచితం_కాకపోతే NETSURF_CONTENT_LLCACHE_H_
#నిర్వచించు NETSURF_CONTENT_LLCACHE_H_

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdint.h>

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/nsurl.h"

నిర్మాణం cert_chain;
నిర్మాణం fetch_multipart_దత్తాంశం;

/** Handle for low-level cache object */
రకనిర్వచనం నిర్మాణం llcache_handle llcache_handle;

/** POST data object for low-level cache requests */
రకనిర్వచనం నిర్మాణం llcache_post_దత్తాంశం {
	గణన {
		LLCACHE_POST_URL_ENCODED,
		LLCACHE_POST_MULTIPART
	} type;				/**< Type of POST data */
	సంఘం {
		అక్షరం *urlenc;		/**< URL encoded data */
		నిర్మాణం fetch_multipart_దత్తాంశం *multipart; /**< Multipart data */
	} దత్తాంశం;				/**< POST data content */
} llcache_post_దత్తాంశం;

/** Flags for low-level cache object retrieval */
గణన llcache_retrieve_flag {
	/* Note: We're permitted a maximum of 16 flags which must reside in the
	 * bottom 16 bits of the flags word. See hlcache.h for further details.
	 */
	/** Force a new fetch */
	LLCACHE_RETRIEVE_FORCE_FETCH    = (1 << 0),
	/** Requested URL was verified */
	LLCACHE_RETRIEVE_VERIFIABLE     = (1 << 1),
	/**< No error pages */
	LLCACHE_RETRIEVE_NO_ERROR_పుటలు = (1 << 2),
	/**< Stream data (implies that object is not cacheable) */
	LLCACHE_RETRIEVE_ప్రవాహం_దత్తాంశం    = (1 << 3)
};

/** Low-level cache event types */
రకనిర్వచనం గణన {
	LLCACHE_EVENT_GOT_CERTS,        /**< SSL certificates arrived */
	LLCACHE_EVENT_HAD_శీర్షికలు,	/**< Received all headers */
	LLCACHE_EVENT_HAD_దత్తాంశం,		/**< Received some data */
	LLCACHE_EVENT_పూర్తి,		/**< Finished fetching data */

	LLCACHE_EVENT_ERROR,		/**< An error occurred during fetch */
	LLCACHE_EVENT_పురోగతి,		/**< Fetch progress update */

	LLCACHE_EVENT_REDIRECT		/**< Fetch URL redirect occured */
} llcache_event_type;

/**
 * Low-level cache events.
 *
 * Lifetime of contained information is only for the duration of the event
 * and must be copied if it is desirable to retain.
 */
రకనిర్వచనం నిర్మాణం {
	llcache_event_type type;            /**< Type of event */
	సంఘం {
		నిర్మాణం {
			స్థిరరాశి uint8_t *buf; /**< Buffer of data */
			size_t len;	    /**< Byte length of buffer */
		} దత్తాంశం;                     /**< Received data */
		నిర్మాణం {
			nserror సంకేతం_2;       /**< The error code */
			స్థిరరాశి అక్షరం *సందేశం;    /**< Error message */
		} error;
		స్థిరరాశి అక్షరం *పురోగతి_సందేశం;   /**< Progress message */
		నిర్మాణం {
			nsurl *from;	    /**< Redirect origin */
			nsurl *గమ్యం;	    /**< Redirect target */
		} redirect;                 /**< Fetch URL redirect occured */
		స్థిరరాశి నిర్మాణం cert_chain *chain;    /**< Certificate chain */
	} దత్తాంశం;				    /**< Event data */
} llcache_event;

/**
 * Client callback for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event   Event data
 * \param pw      Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
రకనిర్వచనం nserror (*llcache_handle_callback)(llcache_handle *handle,
		స్థిరరాశి llcache_event *event, శూన్య *pw);

/**
 * Parameters to configure the low level cache backing store.
 */
నిర్మాణం llcache_store_parameters {
	స్థిరరాశి అక్షరం *మార్గం; /**< The path to the backing store */

	size_t హద్దు; /**< The backing store upper bound target size */
	size_t hysteresis; /**< The hysteresis around the target size */
};

/**
 * Parameters to configure the low level cache.
 */
నిర్మాణం llcache_parameters {
	size_t హద్దు; /**< The target upper bound for the RAM cache size */
	size_t hysteresis; /**< The hysteresis around the target size */

	/** The minimum lifetime to consider sending objects to backing store.*/
	పూర్ణాంకం minimum_lifetime;

	/** The minimum bandwidth to allow the backing store to
	 * use in bytes/second
	 */
	size_t minimum_bandwidth;

	/** The maximum bandwidth to allow the backing store to use in
	 * bytes/second
	 */
	size_t గరిష్ఠ_bandwidth;

	/** The time quantum over which to calculate the bandwidth values
	 */
	గుర్తులేని దీర్ఘ సమయం_కాలాంశం;

	/** The number of fetches to attempt when timing out */
	uint32_t fetch_attempts;

	నిర్మాణం llcache_store_parameters store;
};

/**
 * Initialise the low-level cache
 *
 * \param parameters cache configuration parameters.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror llcache_initialise(స్థిరరాశి నిర్మాణం llcache_parameters *parameters);

/**
 * Finalise the low-level cache
 */
శూన్య llcache_finalise(శూన్య);

/**
 * Cause the low-level cache to attempt to perform cleanup.
 *
 * No guarantees are made as to whether or not cleanups will take
 * place and what, if any, space savings will be made.
 *
 * \param purge Any objects held in the cache that are safely removable will
 *              be freed regardless of the configured size limits.
 */
శూన్య llcache_clean(bool purge);

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
		nsurl *referer, స్థిరరాశి llcache_post_దత్తాంశం *post,
		llcache_handle_callback cb, శూన్య *pw,
		llcache_handle **ఫలితం);

/**
 * Change the callback associated with a low-level cache handle
 *
 * \param handle  Handle to change callback of
 * \param cb      New callback
 * \param pw      Client data for new callback
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, శూన్య *pw);

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
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **ఫలితం);

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
nserror llcache_handle_force_ప్రవాహం(llcache_handle *handle);

/**
 * Invalidate cache data for a low-level cache object
 *
 * \param handle  Handle to invalidate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror llcache_handle_invalidate_cache_దత్తాంశం(llcache_handle *handle);

/**
 * Retrieve the post-redirect URL of a low-level cache object
 *
 * \param handle  Handle to retrieve URL from
 * \return Post-redirect URL of cache object or NULL if no object
 */
nsurl *llcache_handle_get_url(స్థిరరాశి llcache_handle *handle);

/**
 * Retrieve source data of a low-level cache object
 *
 * \param handle  Handle to retrieve source data from
 * \param size    Pointer to location to receive byte length of data
 * \return Pointer to source data
 */
స్థిరరాశి uint8_t *llcache_handle_get_ఆధారం_దత్తాంశం(స్థిరరాశి llcache_handle *handle,
		size_t *కొలత);

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
స్థిరరాశి అక్షరం *llcache_handle_get_శీర్షిక(స్థిరరాశి llcache_handle *handle,
		స్థిరరాశి అక్షరం *కీలకం);

/**
 * Determine if the same underlying object is referenced by the given handles
 *
 * \param a  First handle
 * \param b  Second handle
 * \return True if handles reference the same object, false otherwise
 */
bool llcache_handle_references_same_object(స్థిరరాశి llcache_handle *a,
		స్థిరరాశి llcache_handle *b);

#అయితే_ముగింపు
