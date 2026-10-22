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

#अगर_अघोषित NETSURF_CONTENT_CONTENT_FACTORY_H_
#घोषणा NETSURF_CONTENT_CONTENT_FACTORY_H_

#समावेश <stdbool.h>

#समावेश <libwapcaplet/libwapcaplet.h>

#समावेश "netsurf/content_type.h"
#समावेश "utils/errors.h"

#घोषणा CONTENT_FACTORY_REGISTER_TYPES(HNAME, HTYPELIST, HHANDLER)	\
									\
nserror HNAME##_init(व्योम)						\
{									\
	uint32_t i;							\
	nserror error = NSERROR_OK;					\
									\
	क्रम (i = 0; i < NOF_ELEMENTS(HTYPELIST); i++) {			\
		error = content_factory_register_handler(		\
			HTYPELIST[i],					\
			&HHANDLER);					\
		अगर (error != NSERROR_OK)				\
			अवरोध;						\
	}								\
									\
	वापस error;							\
}

काष्ठा content;
काष्ठा llcache_handle;
काष्ठा content_handler;

व्योम content_factory_fini(व्योम);

nserror content_factory_register_handler(स्थिर अक्षर *mime_type,
		स्थिर काष्ठा content_handler *handler);

काष्ठा content *content_factory_create_content(काष्ठा llcache_handle *llcache, 
		स्थिर अक्षर *fallback_charset, bool quirks,
		lwc_string *effective_type);

content_type content_factory_type_from_mime_type(lwc_string *mime_type);

#पूर्ण_अगर
