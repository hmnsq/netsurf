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

#شامل <assert.h>
#شامل <string.h>
#شامل <stdbool.h>
#شامل <stdlib.h>

#شامل "utils/errors.h"
#شامل "utils/config.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "content/hlcache.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/utils.h"
#شامل "javascript/content.h"

تعریفنوع ساختار javascript_content {
	ساختار content base;
} javascript_content;

ایستا nserror javascript_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت ساختار http_parameter *params,
		llcache_handle *llcache, ثابت نویسه *fallback_charset,
		bool quirks, ساختار content **c)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, اندازه(javascript_content));
	اگر (script == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__init(&script->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(script);
		بازگشت error;
	}

	*c = (ساختار content *) script;

	بازگشت NSERROR_OK;
}

ایستا bool javascript_convert(ساختار content *c)
{
	content_set_ready(c);
	content_set_done(c);

	بازگشت true;
}

ایستا nserror 
javascript_clone(ثابت ساختار content *old, ساختار content **newc)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, اندازه(javascript_content));
	اگر (script == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__clone(old, &script->base);
	اگر (error != NSERROR_OK) {
		content_destroy(&script->base);
		بازگشت error;
	}

	*newc = (ساختار content *) script;

	بازگشت NSERROR_OK;
}

ایستا تهی javascript_destroy(ساختار content *c)
{
}

ایستا content_type javascript_content_type(تهی)
{
	بازگشت CONTENT_JS;
}


ایستا ثابت content_handler javascript_content_handler = {
	.create = javascript_create,
	.data_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

ایستا ثابت نویسه *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_REGISTER_TYPES(javascript, javascript_types, javascript_content_handler);
