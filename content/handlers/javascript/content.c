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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/config.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "content/hlcache.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "javascript/content.h"

రకనిర్వచనం నిర్మాణం javascript_content {
	నిర్మాణం content పునాది;
} javascript_content;

స్థిర nserror javascript_create(స్థిరరాశి content_handler *handler,
		lwc_string *imime_type, స్థిరరాశి నిర్మాణం http_parameter *params,
		llcache_handle *llcache, స్థిరరాశి అక్షరం *fallback_charset,
		bool quirks, నిర్మాణం content **c)
{
	javascript_content *script;
	nserror error;

	script = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(javascript_content));
	ఒకవేళ (script == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__init(&script->పునాది, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(script);
		తిరిగి error;
	}

	*c = (నిర్మాణం content *) script;

	తిరిగి NSERROR_OK;
}

స్థిర bool javascript_convert(నిర్మాణం content *c)
{
	content_సమితి_ready(c);
	content_సమితి_పూర్తి(c);

	తిరిగి true;
}

స్థిర nserror 
javascript_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc)
{
	javascript_content *script;
	nserror error;

	script = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(javascript_content));
	ఒకవేళ (script == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__clone(old, &script->పునాది);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&script->పునాది);
		తిరిగి error;
	}

	*newc = (నిర్మాణం content *) script;

	తిరిగి NSERROR_OK;
}

స్థిర శూన్య javascript_destroy(నిర్మాణం content *c)
{
}

స్థిర content_type javascript_content_type(శూన్య)
{
	తిరిగి CONTENT_JS;
}


స్థిర స్థిరరాశి content_handler javascript_content_handler = {
	.create = javascript_create,
	.దత్తాంశం_complete = javascript_convert,
	.destroy = javascript_destroy,
	.clone = javascript_clone,
	.type = javascript_content_type,
	.no_share = false,
};

స్థిర స్థిరరాశి అక్షరం *javascript_types[] = {
	"application/javascript", /* RFC 4329 */
	"application/ecmascript", /* RFC 4329 */
	"application/x-javascript", /* common usage */
	"text/javascript", /* common usage */
	"text/ecmascript", /* common usage */
};

CONTENT_FACTORY_రిజిస్టరు_TYPES(javascript, javascript_types, javascript_content_handler);
