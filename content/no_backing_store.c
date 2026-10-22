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

#અન્તર્ભાવ "utils/nsurl.h"

#અન્તર્ભાવ "content/backing_store.h"


/* default to disabled backing store */
સ્થિર nserror initialise(અચલ સંરચના llcache_store_parameters *parameters)
{
	પાછા NSERROR_OK;
}

સ્થિર nserror finalise(શૂન્ય)
{
	પાછા NSERROR_OK;
}

સ્થિર nserror store(nsurl *url,
		     ગણના backing_store_flags flags,
		     uint8_t *data,
		     અચલ size_t datalen)
{
	પાછા NSERROR_SAVE_FAILED;
}

સ્થિર nserror fetch(nsurl *url,
		     ગણના backing_store_flags flags,
		     uint8_t **data_out,
		     size_t *datalen_out)
{
	પાછા NSERROR_NOT_FOUND;
}

સ્થિર nserror invalidate(nsurl *url)
{
	પાછા NSERROR_NOT_FOUND;
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
સ્થિર nserror release(nsurl *url, ગણના backing_store_flags flags)
{
	પાછા NSERROR_NOT_FOUND;
}

સ્થિર સંરચના gui_llcache_table llcache_table = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

સંરચના gui_llcache_table *null_llcache_table = &llcache_table;
