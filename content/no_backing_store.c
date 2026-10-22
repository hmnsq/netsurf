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

/** \file
 * Low-level resource cache null persistant storage implementation.
 */

#అన్తర్భావ "utils/nsurl.h"

#అన్తర్భావ "content/backing_store.h"


/* default to disabled backing store */
స్థిర nserror initialise(స్థిరరాశి నిర్మాణం llcache_store_parameters *parameters)
{
	తిరిగి NSERROR_OK;
}

స్థిర nserror finalise(శూన్య)
{
	తిరిగి NSERROR_OK;
}

స్థిర nserror store(nsurl *url,
		     గణన backing_store_flags flags,
		     uint8_t *దత్తాంశం,
		     స్థిరరాశి size_t datalen)
{
	తిరిగి NSERROR_SAVE_FAILED;
}

స్థిర nserror fetch(nsurl *url,
		     గణన backing_store_flags flags,
		     uint8_t **దత్తాంశం_బహిర్గతం,
		     size_t *datalen_బహిర్గతం)
{
	తిరిగి NSERROR_NOT_దొరికింది;
}

స్థిర nserror invalidate(nsurl *url)
{
	తిరిగి NSERROR_NOT_దొరికింది;
}

/**
 * release a previously fetched or stored memory object.
 *
 * if the BACKING_STORE_ALLOC flag was used with the fetch or
 * store operation for this url the returned storage is
 * unreferenced. When the reference count drops to zero the
 * storage is released.
 *
 * @param url The url is used as the unique primary key to invalidate.
 * @param[in] flags The flags to control how the object data is released.
 * @return NSERROR_OK on success or error code on faliure.
 */
స్థిర nserror release(nsurl *url, గణన backing_store_flags flags)
{
	తిరిగి NSERROR_NOT_దొరికింది;
}

స్థిర నిర్మాణం gui_llcache_పట్టిక llcache_పట్టిక = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

నిర్మాణం gui_llcache_పట్టిక *null_llcache_పట్టిక = &llcache_పట్టిక;
