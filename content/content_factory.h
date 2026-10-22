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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_CONTENT_CONTENT_FACTORY_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_CONTENT_CONTENT_FACTORY_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <libwapcaplet/libwapcaplet.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content_type.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CONTENT_FACTORY_ਰਜਿਸਟਰ_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(ਖਾਲੀ)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	ਲਈ (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_ਰਜਿਸਟਰ_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		ਜੇ (error != NSERROR_OK)				\
			ਰੋਕੋ;						\
	}								\
									\
	ਵਾਪਸ error;							\
}

ਬਣਤਰ content;
ਬਣਤਰ llcache_handle;
ਬਣਤਰ content_handler;

ਖਾਲੀ content_factory_fini(ਖਾਲੀ);

nserror content_factory_ਰਜਿਸਟਰ_handler(ਅਚਲ ਅੱਖਰ *mime_type,
		ਅਚਲ ਬਣਤਰ content_handler *handler);

ਬਣਤਰ content *content_factory_create_content(ਬਣਤਰ llcache_handle *llcache, 
		ਅਚਲ ਅੱਖਰ *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#ਜੇ_ਅੰਤ
