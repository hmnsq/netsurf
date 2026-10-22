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

قسمتعریف ساخت javascript_content {
	ساخت content بنیاد;
} javascript_content;

ساکن nserror javascript_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت ساخت http_parameter *params,
		llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, ساخت content **c)
{
	javascript_content *script;
	nserror error;

	script = صفر_حافظہ_دو(1, حجم(javascript_content));
	اگر (script == تہی)
		واپس NSERROR_NOMEM;

	error = content__init(&script->بنیاد, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(script);
		واپس error;
	}

	*c = (ساخت content *) script;

	واپس NSERROR_OK;
}

ساکن bool javascript_convert(ساخت content *c)
{
	content_سیٹ_ready(c);
	content_سیٹ_مکمل(c);

	واپس true;
}

ساکن nserror 
javascript_clone(ثابت ساخت content *old, ساخت content **newc)
{
	javascript_content *script;
	nserror error;

	script = صفر_حافظہ_دو(1, حجم(javascript_content));
	اگر (script == تہی)
		واپس NSERROR_NOMEM;

	error = content__clone(old, &script->بنیاد);
	اگر (error != NSERROR_OK) {
		content_destroy(&script->بنیاد);
		واپس error;
	}

	*newc = (ساخت content *) script;

	واپس NSERROR_OK;
}

ساکن خالی javascript_destroy(ساخت content *c)
{
}

ساکن content_type javascript_content_type(خالی)
{
	واپس CONTENT_JS;
}


ساکن ثابت content_handler javascript_content_handler = {
	.create = javascript_create,
	.ڈیٹا_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

ساکن ثابت حرف *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_رجسٹر_TYPES(javascript, javascript_types, javascript_content_handler);
