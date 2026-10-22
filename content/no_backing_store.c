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

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/backing_store.h"


/* default to disabled backing store */
ਸਥਿਰ nserror initialise(ਅਚਲ ਬਣਤਰ llcache_store_parameters *parameters)
{
	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ nserror finalise(ਖਾਲੀ)
{
	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ nserror store(nsurl *url,
		     ਗਿਣਤੀ backing_store_flags flags,
		     uint8_t *ਅੰਕੜੇ,
		     ਅਚਲ size_t datalen)
{
	ਵਾਪਸ NSERROR_SAVE_FAILED;
}

ਸਥਿਰ nserror fetch(nsurl *url,
		     ਗਿਣਤੀ backing_store_flags flags,
		     uint8_t **ਅੰਕੜੇ_ਬਾਹਰਲਾ,
		     size_t *datalen_ਬਾਹਰਲਾ)
{
	ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
}

ਸਥਿਰ nserror invalidate(nsurl *url)
{
	ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
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
ਸਥਿਰ nserror release(nsurl *url, ਗਿਣਤੀ backing_store_flags flags)
{
	ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
}

ਸਥਿਰ ਬਣਤਰ gui_llcache_ਸਾਰਣੀ llcache_ਸਾਰਣੀ = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

ਬਣਤਰ gui_llcache_ਸਾਰਣੀ *null_llcache_ਸਾਰਣੀ = &llcache_ਸਾਰਣੀ;
