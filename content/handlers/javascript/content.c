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

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "utils/errors.h"
#include "utils/config.h"
#include "content/content_protected.h"
#include "content/content_factory.h"
#include "content/hlcache.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/utils.h"
#include "javascript/content.h"

typedef sa_nracanaa javascript_content {
	sa_nracanaa content base;
} javascript_content;

static nserror javascript_create(const content_handler *handler,
		lwc_string *imime_type, const sa_nracanaa http_parameter *params,
		llcache_handle *llcache, const var_na *fallback_charset,
		bool quirks, sa_nracanaa content **c)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, parimaa_na(javascript_content));
	ຖ້າ (script == NULL)
		ສົ່ງຄືນ NSERROR_NOMEM;

	error = content__init(&script->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ຖ້າ (error != NSERROR_OK) {
		free(script);
		ສົ່ງຄືນ error;
	}

	*c = (sa_nracanaa content *) script;

	ສົ່ງຄືນ NSERROR_OK;
}

static bool javascript_convert(sa_nracanaa content *c)
{
	content_set_ready(c);
	content_set_done(c);

	ສົ່ງຄືນ true;
}

static nserror 
javascript_clone(const sa_nracanaa content *old, sa_nracanaa content **newc)
{
	javascript_content *script;
	nserror error;

	script = calloc(1, parimaa_na(javascript_content));
	ຖ້າ (script == NULL)
		ສົ່ງຄືນ NSERROR_NOMEM;

	error = content__clone(old, &script->base);
	ຖ້າ (error != NSERROR_OK) {
		content_destroy(&script->base);
		ສົ່ງຄືນ error;
	}

	*newc = (sa_nracanaa content *) script;

	ສົ່ງຄືນ NSERROR_OK;
}

static void javascript_destroy(sa_nracanaa content *c)
{
}

static content_type javascript_content_type(void)
{
	ສົ່ງຄືນ CONTENT_JS;
}


static const content_handler javascript_content_handler = {
	.create = javascript_create,
	.data_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

static const var_na *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_REGISTER_TYPES(javascript, javascript_types, javascript_content_handler);
