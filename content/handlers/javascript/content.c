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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "utils/config.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "content/hlcache.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "javascript/content.h"

પ્રકારવ્યાખ્યા સંરચના javascript_content {
	સંરચના content base;
} javascript_content;

સ્થિર nserror javascript_create(અચલ content_handler *handler,
		lwc_string *imime_type, અચલ સંરચના http_parameter *params,
		llcache_handle *llcache, અચલ અક્ષર *fallback_charset,
		bool quirks, સંરચના content **c)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, કદ(javascript_content));
	જો (script == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__init(&script->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	જો (error != NSERROR_OK) {
		મુક્ત(script);
		પાછા error;
	}

	*c = (સંરચના content *) script;

	પાછા NSERROR_OK;
}

સ્થિર bool javascript_convert(સંરચના content *c)
{
	content_set_ready(c);
	content_set_done(c);

	પાછા true;
}

સ્થિર nserror 
javascript_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, કદ(javascript_content));
	જો (script == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__clone(old, &script->base);
	જો (error != NSERROR_OK) {
		content_destroy(&script->base);
		પાછા error;
	}

	*newc = (સંરચના content *) script;

	પાછા NSERROR_OK;
}

સ્થિર શૂન્ય javascript_destroy(સંરચના content *c)
{
}

સ્થિર content_type javascript_content_type(શૂન્ય)
{
	પાછા CONTENT_JS;
}


સ્થિર અચલ content_handler javascript_content_handler = {
	.create = javascript_create,
	.data_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

સ્થિર અચલ અક્ષર *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_REGISTER_TYPES(javascript, javascript_types, javascript_content_handler);
