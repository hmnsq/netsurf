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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/hlcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "javascript/content.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ javascript_content {
	ਬਣਤਰ content ਅਧਾਰ;
} javascript_content;

ਸਥਿਰ nserror javascript_create(ਅਚਲ content_handler *handler,
		lwc_string *imime_type, ਅਚਲ ਬਣਤਰ http_parameter *params,
		llcache_handle *llcache, ਅਚਲ ਅੱਖਰ *fallback_charset,
		bool quirks, ਬਣਤਰ content **c)
{
	javascript_content *script;
	nserror error;

	script = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(javascript_content));
	ਜੇ (script == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__init(&script->ਅਧਾਰ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(script);
		ਵਾਪਸ error;
	}

	*c = (ਬਣਤਰ content *) script;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ bool javascript_convert(ਬਣਤਰ content *c)
{
	content_ਸੈੱਟ_ready(c);
	content_ਸੈੱਟ_ਪੂਰਾ(c);

	ਵਾਪਸ true;
}

ਸਥਿਰ nserror 
javascript_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc)
{
	javascript_content *script;
	nserror error;

	script = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(javascript_content));
	ਜੇ (script == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__clone(old, &script->ਅਧਾਰ);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&script->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	*newc = (ਬਣਤਰ content *) script;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ ਖਾਲੀ javascript_destroy(ਬਣਤਰ content *c)
{
}

ਸਥਿਰ content_type javascript_content_type(ਖਾਲੀ)
{
	ਵਾਪਸ CONTENT_JS;
}


ਸਥਿਰ ਅਚਲ content_handler javascript_content_handler = {
	.create = javascript_create,
	.ਅੰਕੜੇ_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_ਰਜਿਸਟਰ_TYPES(javascript, javascript_types, javascript_content_handler);
