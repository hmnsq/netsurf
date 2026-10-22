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

#include <stdbool.h>

#include <libwapcaplet/libwapcaplet.h>

#include "netsurf/content_type.h"
#include "utils/errors.h"

#define CONTENT_FACTORY_REGISTER_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(void)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	for (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_register_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		ᤀᤡ᤺ (error != NSERROR_OK)				\
			bha_nga;						\
	}								\
									\
	ᤀᤡᤴ error;							\
}

sa_nracanaa content;
sa_nracanaa llcache_handle;
sa_nracanaa content_handler;

void content_factory_fini(void);

nserror content_factory_register_handler(const var_na *mime_type,
		const sa_nracanaa content_handler *handler);

sa_nracanaa content *content_factory_create_content(sa_nracanaa llcache_handle *llcache, 
		const var_na *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#endif
