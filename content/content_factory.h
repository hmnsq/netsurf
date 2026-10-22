/*
 * Copyright 2011 John-Mark Bell <jmb@netsurf-browser.org>
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

#ifndef NETSURF_CONTENT_CONTENT_FACTORY_H_
#define NETSURF_CONTENT_CONTENT_FACTORY_H_

#অন্তর্ভাৱ <stdbool.h>

#অন্তর্ভাৱ <libwapcaplet/libwapcaplet.h>

#অন্তর্ভাৱ "netsurf/content_type.h"
#অন্তর্ভাৱ "utils/errors.h"

#define CONTENT_FACTORY_REGISTER_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(শূন্য)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	বাবে (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_register_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		যদি (error != NSERROR_OK)				\
			ভঙ্গ;						\
	}								\
									\
	ঘূৰাই error;							\
}

সংরচনা content;
সংরচনা llcache_handle;
সংরচনা content_handler;

শূন্য content_factory_fini(শূন্য);

nserror content_factory_register_handler(ধ্রুৱ ৱর্ণ *mime_type,
		ধ্রুৱ সংরচনা content_handler *handler);

সংরচনা content *content_factory_create_content(সংরচনা llcache_handle *llcache, 
		ধ্রুৱ ৱর্ণ *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#endif
