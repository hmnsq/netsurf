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

#ಅನ್ತರ್ಭಾವ <stdbool.h>

#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>

#ಅನ್ತರ್ಭಾವ "netsurf/content_type.h"
#ಅನ್ತರ್ಭಾವ "utils/errors.h"

#define CONTENT_FACTORY_REGISTER_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(ಶೂನ್ಯ)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	ಪ್ರತಿ (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_register_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		ಯದಿ (error != NSERROR_OK)				\
			ಭಙ್ಗ;						\
	}								\
									\
	ಪ್ರತ್ಯಾಗಮ error;							\
}

ಸಂರಚನಾ content;
ಸಂರಚನಾ llcache_handle;
ಸಂರಚನಾ content_handler;

ಶೂನ್ಯ content_factory_fini(ಶೂನ್ಯ);

nserror content_factory_register_handler(ಧ್ರುವ ವರ್ಣ *mime_type,
		ಧ್ರುವ ಸಂರಚನಾ content_handler *handler);

ಸಂರಚನಾ content *content_factory_create_content(ಸಂರಚನಾ llcache_handle *llcache, 
		ಧ್ರುವ ವರ್ಣ *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#endif
