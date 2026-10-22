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

#അന്തര്ഭാവ <stdbool.h>

#അന്തര്ഭാവ <libwapcaplet/libwapcaplet.h>

#അന്തര്ഭാവ "netsurf/content_type.h"
#അന്തര്ഭാവ "utils/errors.h"

#define CONTENT_FACTORY_REGISTER_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(ശൂന്യം)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	വേണ്ടി (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_register_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		എങ്കിൽ (error != NSERROR_OK)				\
			നിർത്തുക;						\
	}								\
									\
	തിരികെ error;							\
}

ഘടന content;
ഘടന llcache_handle;
ഘടന content_handler;

ശൂന്യം content_factory_fini(ശൂന്യം);

nserror content_factory_register_handler(സ്ഥിരം അക്ഷരം *mime_type,
		സ്ഥിരം ഘടന content_handler *handler);

ഘടന content *content_factory_create_content(ഘടന llcache_handle *llcache, 
		സ്ഥിരം അക്ഷരം *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#endif
