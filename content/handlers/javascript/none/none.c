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

#समावेश "utils/errors.h"
#समावेश "content/content.h"
#समावेश "utils/nsoption.h"

#समावेश "javascript/js.h"
#समावेश "utils/log.h"

व्योम js_initialise(व्योम)
{
	nsoption_set_bool(enable_javascript, false);
}

व्योम js_finalise(व्योम)
{
}

nserror js_newheap(पूर्णांक timeout, jsheap **heap)
{
	*heap = शून्य;
	वापस NSERROR_OK;
}

व्योम js_destroyheap(jsheap *heap)
{
}

nserror js_newthread(jsheap *heap, व्योम *win_priv, व्योम *doc_priv, jsthread **thread)
{
	*thread = शून्य;
	वापस NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	वापस NSERROR_OK;
}

व्योम js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, स्थिर uint8_t *txt, माप_प्रकार txtlen, स्थिर अक्षर *name)
{
	वापस true;
}

bool js_fire_event(jsthread *thread, स्थिर अक्षर *type, काष्ठा dom_document *doc, काष्ठा dom_गाँठ *target)
{
	वापस true;
}

व्योम js_handle_new_element(jsthread *thread, काष्ठा dom_element *गाँठ)
{
}

व्योम js_event_cleanup(jsthread *thread, काष्ठा dom_event *evt)
{
}
