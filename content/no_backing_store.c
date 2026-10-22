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

#ضمن "utils/nsurl.h"

#ضمن "content/backing_store.h"


/* default to disabled backing store */
ساكن nserror initialise(ثابت بنية llcache_store_parameters *parameters)
{
	أرجع NSERROR_OK;
}

ساكن nserror finalise(فراغ)
{
	أرجع NSERROR_OK;
}

ساكن nserror store(nsurl *url,
		     تعداد backing_store_flags flags,
		     uint8_t *داده,
		     ثابت size_t datalen)
{
	أرجع NSERROR_SAVE_FAILED;
}

ساكن nserror fetch(nsurl *url,
		     تعداد backing_store_flags flags,
		     uint8_t **داده_بیرون,
		     size_t *datalen_بیرون)
{
	أرجع NSERROR_NOT_یافت;
}

ساكن nserror invalidate(nsurl *url)
{
	أرجع NSERROR_NOT_یافت;
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
ساكن nserror release(nsurl *url, تعداد backing_store_flags flags)
{
	أرجع NSERROR_NOT_یافت;
}

ساكن بنية gui_llcache_جدول llcache_جدول = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

بنية gui_llcache_جدول *null_llcache_جدول = &llcache_جدول;
