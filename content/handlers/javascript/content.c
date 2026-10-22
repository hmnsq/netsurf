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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/config.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "content/hlcache.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "javascript/content.h"

வகைவரை கட்டமைப்பு javascript_content {
	கட்டமைப்பு content base;
} javascript_content;

நிலையான nserror javascript_create(மாறிலி content_handler *handler,
		lwc_string *imime_type, மாறிலி கட்டமைப்பு http_parameter *params,
		llcache_handle *llcache, மாறிலி எழுத்து *fallback_charset,
		bool quirks, கட்டமைப்பு content **c)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, அளவு(javascript_content));
	எனில் (script == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__init(&script->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	எனில் (error != NSERROR_OK) {
		விடுவி(script);
		திரும்பு error;
	}

	*c = (கட்டமைப்பு content *) script;

	திரும்பு NSERROR_OK;
}

நிலையான bool javascript_convert(கட்டமைப்பு content *c)
{
	content_set_ready(c);
	content_set_done(c);

	திரும்பு true;
}

நிலையான nserror 
javascript_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, அளவு(javascript_content));
	எனில் (script == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__clone(old, &script->base);
	எனில் (error != NSERROR_OK) {
		content_destroy(&script->base);
		திரும்பு error;
	}

	*newc = (கட்டமைப்பு content *) script;

	திரும்பு NSERROR_OK;
}

நிலையான வெறுமை javascript_destroy(கட்டமைப்பு content *c)
{
}

நிலையான content_type javascript_content_type(வெறுமை)
{
	திரும்பு CONTENT_JS;
}


நிலையான மாறிலி content_handler javascript_content_handler = {
	.create = javascript_create,
	.data_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

நிலையான மாறிலி எழுத்து *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_REGISTER_TYPES(javascript, javascript_types, javascript_content_handler);
