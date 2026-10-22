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

#اگر_غیر_تعریف_شدہ NETSURF_CONTENT_CONTENT_FACTORY_H_
#تعریف_کرو NETSURF_CONTENT_CONTENT_FACTORY_H_

#شامل <stdbool.h>

#شامل <libwapcaplet/libwapcaplet.h>

#شامل "netsurf/content_type.h"
#شامل "utils/errors.h"

#تعریف_کرو CONTENT_FACTORY_رجسٹر_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(خالی)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	برائے (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_رجسٹر_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		اگر (error != NSERROR_OK)				\
			توڑو;						\
	}								\
									\
	واپس error;							\
}

ساخت content;
ساخت llcache_handle;
ساخت content_handler;

خالی content_factory_fini(خالی);

nserror content_factory_رجسٹر_handler(ثابت حرف *mime_type,
		ثابت ساخت content_handler *handler);

ساخت content *content_factory_create_content(ساخت llcache_handle *llcache, 
		ثابت حرف *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#اگر_ختم
