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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/config.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "content/hlcache.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "javascript/content.h"

प्रकारनिर्देश संरचना javascript_content {
	संरचना content base;
} javascript_content;

स्थिर nserror javascript_create(ध्रुव content_handler *handler,
		lwc_string *imime_type, ध्रुव संरचना http_parameter *params,
		llcache_handle *llcache, ध्रुव वर्ण *fallback_charset,
		bool quirks, संरचना content **c)
{
	javascript_content *script;
	nserror error;

	script = सुस्मृति(1, परिमाण(javascript_content));
	यदि (script == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__init(&script->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(script);
		प्रत्यागम error;
	}

	*c = (संरचना content *) script;

	प्रत्यागम NSERROR_OK;
}

स्थिर bool javascript_convert(संरचना content *c)
{
	content_set_ready(c);
	content_set_done(c);

	प्रत्यागम true;
}

स्थिर nserror 
javascript_clone(ध्रुव संरचना content *old, संरचना content **newc)
{
	javascript_content *script;
	nserror error;

	script = सुस्मृति(1, परिमाण(javascript_content));
	यदि (script == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__clone(old, &script->base);
	यदि (error != NSERROR_OK) {
		content_destroy(&script->base);
		प्रत्यागम error;
	}

	*newc = (संरचना content *) script;

	प्रत्यागम NSERROR_OK;
}

स्थिर शून्य javascript_destroy(संरचना content *c)
{
}

स्थिर content_type javascript_content_type(शून्य)
{
	प्रत्यागम CONTENT_JS;
}


स्थिर ध्रुव content_handler javascript_content_handler = {
	.create = javascript_create,
	.data_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

स्थिर ध्रुव वर्ण *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_REGISTER_TYPES(javascript, javascript_types, javascript_content_handler);
