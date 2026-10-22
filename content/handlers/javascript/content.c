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

প্রকারলেখ কাঠামো javascript_content {
	কাঠামো content base;
} javascript_content;

স্থির nserror javascript_create(ধ্রুবক content_handler *handler,
		lwc_string *imime_type, ধ্রুবক কাঠামো http_parameter *params,
		llcache_handle *llcache, ধ্রুবক অক্ষর *fallback_charset,
		bool quirks, কাঠামো content **c)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, আকার(javascript_content));
	যদি (script == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__init(&script->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(script);
		ফেরত error;
	}

	*c = (কাঠামো content *) script;

	ফেরত NSERROR_OK;
}

স্থির bool javascript_convert(কাঠামো content *c)
{
	content_set_ready(c);
	content_set_done(c);

	ফেরত true;
}

স্থির nserror 
javascript_clone(ধ্রুবক কাঠামো content *old, কাঠামো content **newc)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, আকার(javascript_content));
	যদি (script == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__clone(old, &script->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&script->base);
		ফেরত error;
	}

	*newc = (কাঠামো content *) script;

	ফেরত NSERROR_OK;
}

স্থির শূন্য javascript_destroy(কাঠামো content *c)
{
}

স্থির content_type javascript_content_type(শূন্য)
{
	ফেরত CONTENT_JS;
}


স্থির ধ্রুবক content_handler javascript_content_handler = {
	.create = javascript_create,
	.তথ্য_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

স্থির ধ্রুবক অক্ষর *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_REGISTER_TYPES(javascript, javascript_types, javascript_content_handler);
