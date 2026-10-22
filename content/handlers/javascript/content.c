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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/config.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "javascript/content.h"

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ javascript_content {
	ಸಂರಚನಾ content base;
} javascript_content;

ಸ್ಥಿರ nserror javascript_create(ಧ್ರುವ content_handler *handler,
		lwc_string *imime_type, ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
		llcache_handle *llcache, ಧ್ರುವ ವರ್ಣ *fallback_charset,
		bool quirks, ಸಂರಚನಾ content **c)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, ಪರಿಮಾಣ(javascript_content));
	ಯದಿ (script == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__init(&script->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(script);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	*c = (ಸಂರಚನಾ content *) script;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ bool javascript_convert(ಸಂರಚನಾ content *c)
{
	content_set_ready(c);
	content_set_done(c);

	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ nserror 
javascript_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, ಪರಿಮಾಣ(javascript_content));
	ಯದಿ (script == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__clone(old, &script->base);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&script->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	*newc = (ಸಂರಚನಾ content *) script;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ javascript_destroy(ಸಂರಚನಾ content *c)
{
}

ಸ್ಥಿರ content_type javascript_content_type(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CONTENT_JS;
}


ಸ್ಥಿರ ಧ್ರುವ content_handler javascript_content_handler = {
	.create = javascript_create,
	.data_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_REGISTER_TYPES(javascript, javascript_types, javascript_content_handler);
