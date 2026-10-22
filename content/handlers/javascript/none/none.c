/*
 * Copyright 2012 Vincent Sanders <vince@netsurf-browser.org>
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

/** \file
 * Dummy implementation of javascript engine functions.
 */

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "content/content.h"
#అన్తర్భావ "utils/nsoption.h"

#అన్తర్భావ "javascript/js.h"
#అన్తర్భావ "utils/log.h"

శూన్య js_initialise(శూన్య)
{
	nsoption_సమితి_bool(enable_javascript, false);
}

శూన్య js_finalise(శూన్య)
{
}

nserror js_newheap(పూర్ణాంకం timeout, jsheap **కుప్ప)
{
	*కుప్ప = ఖాళీ;
	తిరిగి NSERROR_OK;
}

శూన్య js_destroyheap(jsheap *కుప్ప)
{
}

nserror js_newthread(jsheap *కుప్ప, శూన్య *win_priv, శూన్య *doc_priv, jsthread **thread)
{
	*thread = ఖాళీ;
	తిరిగి NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	తిరిగి NSERROR_OK;
}

శూన్య js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, స్థిరరాశి uint8_t *txt, size_t txtlen, స్థిరరాశి అక్షరం *name)
{
	తిరిగి true;
}

bool js_fire_event(jsthread *thread, స్థిరరాశి అక్షరం *type, నిర్మాణం dom_document *doc, నిర్మాణం dom_కణుపు *లక్ష్యం)
{
	తిరిగి true;
}

శూన్య js_handle_new_element(jsthread *thread, నిర్మాణం dom_element *కణుపు)
{
}

శూన్య js_event_cleanup(jsthread *thread, నిర్మాణం dom_event *evt)
{
}
