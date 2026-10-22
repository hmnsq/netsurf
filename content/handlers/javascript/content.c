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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/config.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "content/hlcache.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "javascript/content.h"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା javascript_content {
	ସଂରଚନା content base;
} javascript_content;

ସ୍ଥିର nserror javascript_create(ଅଚଳ content_handler *handler,
		lwc_string *imime_type, ଅଚଳ ସଂରଚନା http_parameter *params,
		llcache_handle *llcache, ଅଚଳ ଅକ୍ଷର *fallback_charset,
		bool quirks, ସଂରଚନା content **c)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, ଆକାର(javascript_content));
	ଯଦି (script == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__init(&script->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(script);
		ଫେରନ୍ତୁ error;
	}

	*c = (ସଂରଚନା content *) script;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର bool javascript_convert(ସଂରଚନା content *c)
{
	content_set_ready(c);
	content_set_done(c);

	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର nserror 
javascript_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, ଆକାର(javascript_content));
	ଯଦି (script == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__clone(old, &script->base);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&script->base);
		ଫେରନ୍ତୁ error;
	}

	*newc = (ସଂରଚନା content *) script;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର ଶୂନ୍ୟ javascript_destroy(ସଂରଚନା content *c)
{
}

ସ୍ଥିର content_type javascript_content_type(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CONTENT_JS;
}


ସ୍ଥିର ଅଚଳ content_handler javascript_content_handler = {
	.create = javascript_create,
	.data_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_REGISTER_TYPES(javascript, javascript_types, javascript_content_handler);
