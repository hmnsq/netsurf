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

#अन्तर्भाव <stdbool.h>

#अन्तर्भाव <libwapcaplet/libwapcaplet.h>

#अन्तर्भाव "netsurf/content_type.h"
#अन्तर्भाव "utils/errors.h"

#define CONTENT_FACTORY_REGISTER_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(शून्य)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	साठी (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_register_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		जर (error != NSERROR_OK)				\
			थांबा;						\
	}								\
									\
	परत error;							\
}

संरचना content;
संरचना llcache_handle;
संरचना content_handler;

शून्य content_factory_fini(शून्य);

nserror content_factory_register_handler(स्थिरांक अक्षर *mime_type,
		स्थिरांक संरचना content_handler *handler);

संरचना content *content_factory_create_content(संरचना llcache_handle *llcache, 
		स्थिरांक अक्षर *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#endif
