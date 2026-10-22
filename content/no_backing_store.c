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

#അന്തര്ഭാവ "utils/nsurl.h"

#അന്തര്ഭാവ "content/backing_store.h"


/* default to disabled backing store */
സ്ഥിതം nserror initialise(സ്ഥിരം ഘടന llcache_store_parameters *parameters)
{
	തിരികെ NSERROR_OK;
}

സ്ഥിതം nserror finalise(ശൂന്യം)
{
	തിരികെ NSERROR_OK;
}

സ്ഥിതം nserror store(nsurl *url,
		     എണ്ണൽ backing_store_flags flags,
		     uint8_t *data,
		     സ്ഥിരം size_t datalen)
{
	തിരികെ NSERROR_SAVE_FAILED;
}

സ്ഥിതം nserror fetch(nsurl *url,
		     എണ്ണൽ backing_store_flags flags,
		     uint8_t **data_out,
		     size_t *datalen_out)
{
	തിരികെ NSERROR_NOT_FOUND;
}

സ്ഥിതം nserror invalidate(nsurl *url)
{
	തിരികെ NSERROR_NOT_FOUND;
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
സ്ഥിതം nserror release(nsurl *url, എണ്ണൽ backing_store_flags flags)
{
	തിരികെ NSERROR_NOT_FOUND;
}

സ്ഥിതം ഘടന gui_llcache_table llcache_table = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

ഘടന gui_llcache_table *null_llcache_table = &llcache_table;
