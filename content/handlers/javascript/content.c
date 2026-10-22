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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/config.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "content/hlcache.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "javascript/content.h"

প্রকারনির্দেশ সংরচনা javascript_content {
	সংরচনা content base;
} javascript_content;

স্থির nserror javascript_create(ধ্রুব content_handler *handler,
		lwc_string *imime_type, ধ্রুব সংরচনা http_parameter *params,
		llcache_handle *llcache, ধ্রুব বর্ণ *fallback_charset,
		bool quirks, সংরচনা content **c)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, পরিমাণ(javascript_content));
	যদি (script == রিক্ত)
		প্রত্যাগম NSERROR_NOMEM;

	error = content__init(&script->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(script);
		প্রত্যাগম error;
	}

	*c = (সংরচনা content *) script;

	প্রত্যাগম NSERROR_OK;
}

স্থির bool javascript_convert(সংরচনা content *c)
{
	content_set_ready(c);
	content_set_done(c);

	প্রত্যাগম true;
}

স্থির nserror 
javascript_clone(ধ্রুব সংরচনা content *old, সংরচনা content **newc)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, পরিমাণ(javascript_content));
	যদি (script == রিক্ত)
		প্রত্যাগম NSERROR_NOMEM;

	error = content__clone(old, &script->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&script->base);
		প্রত্যাগম error;
	}

	*newc = (সংরচনা content *) script;

	প্রত্যাগম NSERROR_OK;
}

স্থির শূন্য javascript_destroy(সংরচনা content *c)
{
}

স্থির content_type javascript_content_type(শূন্য)
{
	প্রত্যাগম CONTENT_JS;
}


স্থির ধ্রুব content_handler javascript_content_handler = {
	.create = javascript_create,
	.data_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

স্থির ধ্রুব বর্ণ *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_REGISTER_TYPES(javascript, javascript_types, javascript_content_handler);
