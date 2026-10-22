/*
 * Copyright 2014 Vincent Sanders <vince@netsurf-browser.org>
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

/**
 * \file
 * Low-level source data cache backing store interface.
 */

#నిర్వచితం_కాకపోతే NETSURF_CONTENT_LLCACHE_వ్యక్తిగత_H
#నిర్వచించు NETSURF_CONTENT_LLCACHE_వ్యక్తిగత_H

#అన్తర్భావ "content/llcache.h"

/** storage control flags */
గణన backing_store_flags {
	/** no special processing */
	BACKING_STORE_NONE = 0,
	/** data is metadata */
	BACKING_STORE_META = 1,
};

/**
 * low level cache backing store operation table
 *
 * The low level cache (source objects) has the capability to make
 * objects and their metadata (headers etc) persistent by writing to a
 * backing store using these operations.
 */
నిర్మాణం gui_llcache_పట్టిక {
	/**
	 * Initialise the backing store.
	 *
	 * @param parameters to configure backing store.
	 * @return NSERROR_OK on success or error code on failure.
	 */
	nserror (*initialise)(స్థిరరాశి నిర్మాణం llcache_store_parameters *parameters);

	/**
	 * Finalise the backing store.
	 *
	 * @return NSERROR_OK on success or error code on failure.
	 */
	nserror (*finalise)(శూన్య);

	/**
	 * Place an object in the backing store.
	 *
	 * The object is placed in the persistent store and may be
	 *  retrieved with the fetch method.
	 *
	 * The backing store will take a reference to the
	 *  passed data, subsequently the caller should explicitly
	 *  release the allocation using the release method and not
	 *  free the data itself.
	 *
	 * The caller may not assume that the persistent storage has
	 *  been completely written on return.
	 *
	 * @param[in] url The url is used as the unique primary key for the data.
	 * @param[in] flags The flags to control how the object is stored.
	 * @param[in] data The objects data.
	 * @param[in] datalen The length of the \a data.
	 * @return NSERROR_OK on success or error code on failure.
	 */
	nserror (*store)(నిర్మాణం nsurl *url, గణన backing_store_flags flags,
			 uint8_t *దత్తాంశం, స్థిరరాశి size_t datalen);

	/**
	 * Retrieve an object from the backing store.
	 *
	 * The backing store will manage its own memory and the
	 * allocations returned in \a data *must* not be altered.
	 *
	 * The caller must assume nothing about the backing store
	 *  allocated buffers and the storage and *must* be freed by
	 *  calling the release method.
	 *
	 * @param[in] url The url is used as the unique primary key for the data.
	 * @param[in] flags The flags to control how the object is retrieved.
	 * @param[out] data The retrieved objects data.
	 * @param[out] datalen The length of the \a data retrieved.
	 * @return NSERROR_OK on success or error code on failure.
	 */
	nserror (*fetch)(నిర్మాణం nsurl *url, గణన backing_store_flags flags,
			 uint8_t **దత్తాంశం, size_t *datalen);

	/**
	 * release a previously fetched or stored memory object.
	 *
	 * @param url The url is used as the unique primary key to invalidate.
	 * @param[in] flags The flags to control how the object data is released.
	 * @return NSERROR_OK on success or error code on failure.
	 */
	nserror (*release)(నిర్మాణం nsurl *url, గణన backing_store_flags flags);

	/**
	 * Invalidate a source object from the backing store.
	 *
	 * The entry (if present in the backing store) must no longer
	 * be returned as a result to the fetch or meta operations.
	 *
	 * If the entry had data allocated it will be released.
	 *
	 * @param url The url is used as the unique primary key to invalidate.
	 * @return NSERROR_OK on success or error code on failure.
	 */
	nserror (*invalidate)(నిర్మాణం nsurl *url);

};

బాహ్య నిర్మాణం gui_llcache_పట్టిక* null_llcache_పట్టిక;
బాహ్య నిర్మాణం gui_llcache_పట్టిక* filesystem_llcache_పట్టిక;

#అయితే_ముగింపు
