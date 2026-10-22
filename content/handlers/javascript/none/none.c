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

#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "content/content.h"
#अन्तर्भाव "utils/nsoption.h"

#अन्तर्भाव "javascript/js.h"
#अन्तर्भाव "utils/log.h"

शून्य js_initialise(शून्य)
{
	nsoption_set_bool(enable_javascript, false);
}

शून्य js_finalise(शून्य)
{
}

nserror js_newheap(पूर्णांक timeout, jsheap **heap)
{
	*heap = रिक्त;
	परत NSERROR_OK;
}

शून्य js_destroyheap(jsheap *heap)
{
}

nserror js_newthread(jsheap *heap, शून्य *win_priv, शून्य *doc_priv, jsthread **thread)
{
	*thread = रिक्त;
	परत NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	परत NSERROR_OK;
}

शून्य js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, स्थिरांक uint8_t *txt, माप_प्रकार txtlen, स्थिरांक अक्षर *name)
{
	परत true;
}

bool js_fire_event(jsthread *thread, स्थिरांक अक्षर *type, संरचना dom_document *doc, संरचना dom_गाठ *target)
{
	परत true;
}

शून्य js_handle_new_element(jsthread *thread, संरचना dom_element *गाठ)
{
}

शून्य js_event_cleanup(jsthread *thread, संरचना dom_event *evt)
{
}
