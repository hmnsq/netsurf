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

#ضمن <stdbool.h>

#ضمن <libwapcaplet/libwapcaplet.h>

#ضمن "netsurf/content_type.h"
#ضمن "utils/errors.h"

#define CONTENT_FACTORY_REGISTER_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(فراغ)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	لكل (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_register_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		ܐܢ (error != NSERROR_OK)				\
			اكسر;						\
	}								\
									\
	ܦܢܐ error;							\
}

بنية content;
بنية llcache_handle;
بنية content_handler;

فراغ content_factory_fini(فراغ);

nserror content_factory_register_handler(ثابت حرف *mime_type,
		ثابت بنية content_handler *handler);

بنية content *content_factory_create_content(بنية llcache_handle *llcache, 
		ثابت حرف *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#endif
