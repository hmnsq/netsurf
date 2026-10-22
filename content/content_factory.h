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

#నిర్వచితం_కాకపోతే NETSURF_CONTENT_CONTENT_FACTORY_H_
#నిర్వచించు NETSURF_CONTENT_CONTENT_FACTORY_H_

#అన్తర్భావ <stdbool.h>

#అన్తర్భావ <libwapcaplet/libwapcaplet.h>

#అన్తర్భావ "netsurf/content_type.h"
#అన్తర్భావ "utils/errors.h"

#నిర్వచించు CONTENT_FACTORY_రిజిస్టరు_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(శూన్య)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	కొరకు (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_రిజిస్టరు_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		ఒకవేళ (error != NSERROR_OK)				\
			ఆపు;						\
	}								\
									\
	తిరిగి error;							\
}

నిర్మాణం content;
నిర్మాణం llcache_handle;
నిర్మాణం content_handler;

శూన్య content_factory_fini(శూన్య);

nserror content_factory_రిజిస్టరు_handler(స్థిరరాశి అక్షరం *mime_type,
		స్థిరరాశి నిర్మాణం content_handler *handler);

నిర్మాణం content *content_factory_create_content(నిర్మాణం llcache_handle *llcache, 
		స్థిరరాశి అక్షరం *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#అయితే_ముగింపు
