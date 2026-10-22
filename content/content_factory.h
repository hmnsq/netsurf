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

#यदि_अपरिभाषितम् NETSURF_CONTENT_CONTENT_FACTORY_H_
#परिभाषय NETSURF_CONTENT_CONTENT_FACTORY_H_

#अन्तर्भाव <stdbool.h>

#अन्तर्भाव <libwapcaplet/libwapcaplet.h>

#अन्तर्भाव "netsurf/content_type.h"
#अन्तर्भाव "utils/errors.h"

#परिभाषय CONTENT_FACTORY_पञ्जिकाकोशः_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(शून्य)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	प्रति (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_पञ्जिकाकोशः_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		यदि (error != NSERROR_OK)				\
			भङ्ग;						\
	}								\
									\
	प्रत्यागम error;							\
}

संरचना content;
संरचना llcache_handle;
संरचना content_handler;

शून्य content_factory_fini(शून्य);

nserror content_factory_पञ्जिकाकोशः_handler(ध्रुव वर्ण *mime_type,
		ध्रुव संरचना content_handler *handler);

संरचना content *content_factory_create_content(संरचना llcache_handle *llcache, 
		ध्रुव वर्ण *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#यदि_अन्तः
