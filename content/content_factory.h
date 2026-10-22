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

#அந்தர்பாவ <stdbool.h>

#அந்தர்பாவ <libwapcaplet/libwapcaplet.h>

#அந்தர்பாவ "netsurf/content_type.h"
#அந்தர்பாவ "utils/errors.h"

#define CONTENT_FACTORY_REGISTER_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(வெறுமை)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	க்கு (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_register_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		எனில் (error != NSERROR_OK)				\
			நிறுத்து;						\
	}								\
									\
	திரும்பு error;							\
}

கட்டமைப்பு content;
கட்டமைப்பு llcache_handle;
கட்டமைப்பு content_handler;

வெறுமை content_factory_fini(வெறுமை);

nserror content_factory_register_handler(மாறிலி எழுத்து *mime_type,
		மாறிலி கட்டமைப்பு content_handler *handler);

கட்டமைப்பு content *content_factory_create_content(கட்டமைப்பு llcache_handle *llcache, 
		மாறிலி எழுத்து *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#endif
