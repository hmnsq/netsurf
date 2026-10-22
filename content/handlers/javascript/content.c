/*
 * Copyright 2012 Vincent Sanders <vince@kyllikki.org>
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

/**
 * \file
 * javascript content implementation
 */

#ضمن <assert.h>
#ضمن <string.h>
#ضمن <stdbool.h>
#ضمن <stdlib.h>

#ضمن "utils/errors.h"
#ضمن "utils/config.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "content/hlcache.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/utils.h"
#ضمن "javascript/content.h"

تعريفنوع بنية javascript_content {
	بنية content base;
} javascript_content;

ساكن nserror javascript_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت بنية http_parameter *params,
		llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, بنية content **c)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, حجم(javascript_content));
	אם (script == عدم)
		החזר NSERROR_NOMEM;

	error = content__init(&script->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	אם (error != NSERROR_OK) {
		حرر(script);
		החזר error;
	}

	*c = (بنية content *) script;

	החזר NSERROR_OK;
}

ساكن bool javascript_convert(بنية content *c)
{
	content_set_ready(c);
	content_set_done(c);

	החזר true;
}

ساكن nserror 
javascript_clone(ثابت بنية content *old, بنية content **newc)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, حجم(javascript_content));
	אם (script == عدم)
		החזר NSERROR_NOMEM;

	error = content__clone(old, &script->base);
	אם (error != NSERROR_OK) {
		content_destroy(&script->base);
		החזר error;
	}

	*newc = (بنية content *) script;

	החזר NSERROR_OK;
}

ساكن فراغ javascript_destroy(بنية content *c)
{
}

ساكن content_type javascript_content_type(فراغ)
{
	החזר CONTENT_JS;
}


ساكن ثابت content_handler javascript_content_handler = {
	.create = javascript_create,
	.data_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

ساكن ثابت حرف *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_REGISTER_TYPES(javascript, javascript_types, javascript_content_handler);
