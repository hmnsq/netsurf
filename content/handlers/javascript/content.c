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

#समावेश <assert.h>
#समावेश <string.h>
#समावेश <stdbool.h>
#समावेश <stdlib.h>

#समावेश "utils/errors.h"
#समावेश "utils/config.h"
#समावेश "content/content_protected.h"
#समावेश "content/content_factory.h"
#समावेश "content/hlcache.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "utils/utils.h"
#समावेश "javascript/content.h"

प्रकार काष्ठा javascript_content {
	काष्ठा content base;
} javascript_content;

अटल nserror javascript_create(स्थिर content_handler *handler,
		lwc_string *imime_type, स्थिर काष्ठा http_parameter *params,
		llcache_handle *llcache, स्थिर अक्षर *fallback_charset,
		bool quirks, काष्ठा content **c)
{
	javascript_content *script;
	nserror error;

	script = सुस्मृति(1, माप(javascript_content));
	अगर (script == शून्य)
		वापस NSERROR_NOMEM;

	error = content__init(&script->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	अगर (error != NSERROR_OK) {
		मुक्त(script);
		वापस error;
	}

	*c = (काष्ठा content *) script;

	वापस NSERROR_OK;
}

अटल bool javascript_convert(काष्ठा content *c)
{
	content_set_ready(c);
	content_set_done(c);

	वापस true;
}

अटल nserror 
javascript_clone(स्थिर काष्ठा content *old, काष्ठा content **newc)
{
	javascript_content *script;
	nserror error;

	script = सुस्मृति(1, माप(javascript_content));
	अगर (script == शून्य)
		वापस NSERROR_NOMEM;

	error = content__clone(old, &script->base);
	अगर (error != NSERROR_OK) {
		content_destroy(&script->base);
		वापस error;
	}

	*newc = (काष्ठा content *) script;

	वापस NSERROR_OK;
}

अटल व्योम javascript_destroy(काष्ठा content *c)
{
}

अटल content_type javascript_content_type(व्योम)
{
	वापस CONTENT_JS;
}


अटल स्थिर content_handler javascript_content_handler = {
	.create = javascript_create,
	.तथ्य_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

अटल स्थिर अक्षर *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_REGISTER_TYPES(javascript, javascript_types, javascript_content_handler);
