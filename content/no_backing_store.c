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

#अन्तर्भाव "utils/nsurl.h"

#अन्तर्भाव "content/backing_store.h"


/* default to disabled backing store */
स्थिर nserror initialise(ध्रुव संरचना llcache_store_parameters *parameters)
{
	प्रत्यागम NSERROR_OK;
}

स्थिर nserror finalise(शून्य)
{
	प्रत्यागम NSERROR_OK;
}

स्थिर nserror store(nsurl *url,
		     गणना backing_store_flags flags,
		     uint8_t *दत्त,
		     ध्रुव size_t datalen)
{
	प्रत्यागम NSERROR_SAVE_FAILED;
}

स्थिर nserror fetch(nsurl *url,
		     गणना backing_store_flags flags,
		     uint8_t **दत्त_बहिर्गमनम्,
		     size_t *datalen_बहिर्गमनम्)
{
	प्रत्यागम NSERROR_NOT_प्राप्तम्;
}

स्थिर nserror invalidate(nsurl *url)
{
	प्रत्यागम NSERROR_NOT_प्राप्तम्;
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
स्थिर nserror release(nsurl *url, गणना backing_store_flags flags)
{
	प्रत्यागम NSERROR_NOT_प्राप्तम्;
}

स्थिर संरचना gui_llcache_सारणी llcache_सारणी = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

संरचना gui_llcache_सारणी *null_llcache_सारणी = &llcache_सारणी;
