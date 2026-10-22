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

#અન્તર્ભાવ <stdbool.h>

#અન્તર્ભાવ <libwapcaplet/libwapcaplet.h>

#અન્તર્ભાવ "netsurf/content_type.h"
#અન્તર્ભાવ "utils/errors.h"

#define CONTENT_FACTORY_REGISTER_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(શૂન્ય)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	માટે (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_register_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		જો (error != NSERROR_OK)				\
			તોડો;						\
	}								\
									\
	પાછા error;							\
}

સંરચના content;
સંરચના llcache_handle;
સંરચના content_handler;

શૂન્ય content_factory_fini(શૂન્ય);

nserror content_factory_register_handler(અચલ અક્ષર *mime_type,
		અચલ સંરચના content_handler *handler);

સંરચના content *content_factory_create_content(સંરચના llcache_handle *llcache, 
		અચલ અક્ષર *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#endif
