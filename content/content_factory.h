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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>

#ଅନ୍ତର୍ଭାଵ <libwapcaplet/libwapcaplet.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/content_type.h"
#ଅନ୍ତର୍ଭାଵ "utils/errors.h"

#define CONTENT_FACTORY_REGISTER_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(ଶୂନ୍ୟ)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	ପାଇଁ (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_register_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		ଯଦି (error != NSERROR_OK)				\
			ଭାଙ୍ଗ;						\
	}								\
									\
	ଫେରନ୍ତୁ error;							\
}

ସଂରଚନା content;
ସଂରଚନା llcache_handle;
ସଂରଚନା content_handler;

ଶୂନ୍ୟ content_factory_fini(ଶୂନ୍ୟ);

nserror content_factory_register_handler(ଅଚଳ ଅକ୍ଷର *mime_type,
		ଅଚଳ ସଂରଚନା content_handler *handler);

ସଂରଚନା content *content_factory_create_content(ସଂରଚନା llcache_handle *llcache, 
		ଅଚଳ ଅକ୍ଷର *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#endif
