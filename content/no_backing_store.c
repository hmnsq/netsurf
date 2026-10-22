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

#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"

#ಅನ್ತರ್ಭಾವ "content/backing_store.h"


/* default to disabled backing store */
ಸ್ಥಿರ nserror initialise(ಸ್ಥಿರಾಂಕ ರಚನೆ llcache_store_parameters *parameters)
{
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ nserror finalise(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ nserror store(nsurl *url,
		     ಎಣಿಕೆ backing_store_flags flags,
		     uint8_t *ದತ್ತಾಂಶ,
		     ಸ್ಥಿರಾಂಕ size_t datalen)
{
	ಹಿಂತಿರುಗಿ NSERROR_SAVE_FAILED;
}

ಸ್ಥಿರ nserror fetch(nsurl *url,
		     ಎಣಿಕೆ backing_store_flags flags,
		     uint8_t **ದತ್ತಾಂಶ_ಹೊರಹೋಗು,
		     size_t *datalen_ಹೊರಹೋಗು)
{
	ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
}

ಸ್ಥಿರ nserror invalidate(nsurl *url)
{
	ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
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
ಸ್ಥಿರ nserror release(nsurl *url, ಎಣಿಕೆ backing_store_flags flags)
{
	ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
}

ಸ್ಥಿರ ರಚನೆ gui_llcache_ಕೋಷ್ಟಕ llcache_ಕೋಷ್ಟಕ = {
	.initialise = initialise,
	.finalise = finalise,
	.store = store,
	.fetch = fetch,
	.invalidate = invalidate,
	.release = release,
};

ರಚನೆ gui_llcache_ಕೋಷ್ಟಕ *null_llcache_ಕೋಷ್ಟಕ = &llcache_ಕೋಷ್ಟಕ;
