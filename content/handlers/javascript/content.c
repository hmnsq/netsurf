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

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ javascript_content {
	ರಚನೆ content ತಳ;
} javascript_content;

ಸ್ಥಿರ nserror javascript_create(ಸ್ಥಿರಾಂಕ content_handler *handler,
		lwc_string *imime_type, ಸ್ಥಿರಾಂಕ ರಚನೆ http_parameter *params,
		llcache_handle *llcache, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
		bool quirks, ರಚನೆ content **c)
{
	javascript_content *script;
	nserror error;

	script = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(javascript_content));
	ಆದರೆ (script == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__init(&script->ತಳ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(script);
		ಹಿಂತಿರುಗಿ error;
	}

	*c = (ರಚನೆ content *) script;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ bool javascript_convert(ರಚನೆ content *c)
{
	content_ಗಣ_ready(c);
	content_ಗಣ_ಮುಗಿದ(c);

	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ nserror 
javascript_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old, ರಚನೆ content **newc)
{
	javascript_content *script;
	nserror error;

	script = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(javascript_content));
	ಆದರೆ (script == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__clone(old, &script->ತಳ);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&script->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	*newc = (ರಚನೆ content *) script;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ javascript_destroy(ರಚನೆ content *c)
{
}

ಸ್ಥಿರ content_type javascript_content_type(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ CONTENT_JS;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_handler javascript_content_handler = {
	.create = javascript_create,
	.ದತ್ತಾಂಶ_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_ರಿಜಿಸ್ಟರ್_TYPES(javascript, javascript_types, javascript_content_handler);
