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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/config.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "content/hlcache.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "javascript/content.h"

തരനിർവചനം ഘടന javascript_content {
	ഘടന content base;
} javascript_content;

സ്ഥിതം nserror javascript_create(സ്ഥിരം content_handler *handler,
		lwc_string *imime_type, സ്ഥിരം ഘടന http_parameter *params,
		llcache_handle *llcache, സ്ഥിരം അക്ഷരം *fallback_charset,
		bool quirks, ഘടന content **c)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, വലുപ്പം(javascript_content));
	എങ്കിൽ (script == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__init(&script->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(script);
		തിരികെ error;
	}

	*c = (ഘടന content *) script;

	തിരികെ NSERROR_OK;
}

സ്ഥിതം bool javascript_convert(ഘടന content *c)
{
	content_set_ready(c);
	content_set_done(c);

	തിരികെ true;
}

സ്ഥിതം nserror 
javascript_clone(സ്ഥിരം ഘടന content *old, ഘടന content **newc)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, വലുപ്പം(javascript_content));
	എങ്കിൽ (script == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__clone(old, &script->base);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&script->base);
		തിരികെ error;
	}

	*newc = (ഘടന content *) script;

	തിരികെ NSERROR_OK;
}

സ്ഥിതം ശൂന്യം javascript_destroy(ഘടന content *c)
{
}

സ്ഥിതം content_type javascript_content_type(ശൂന്യം)
{
	തിരികെ CONTENT_JS;
}


സ്ഥിതം സ്ഥിരം content_handler javascript_content_handler = {
	.create = javascript_create,
	.data_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

സ്ഥിതം സ്ഥിരം അക്ഷരം *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_REGISTER_TYPES(javascript, javascript_types, javascript_content_handler);
