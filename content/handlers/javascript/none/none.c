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

#ضمن "utils/errors.h"
#ضمن "content/content.h"
#ضمن "utils/nsoption.h"

#ضمن "javascript/js.h"
#ضمن "utils/log.h"

فراغ js_initialise(فراغ)
{
	nsoption_set_bool(enable_javascript, false);
}

فراغ js_finalise(فراغ)
{
}

nserror js_newheap(صحيح timeout, jsheap **heap)
{
	*heap = عدم;
	أرجع NSERROR_OK;
}

فراغ js_destroyheap(jsheap *heap)
{
}

nserror js_newthread(jsheap *heap, فراغ *win_priv, فراغ *doc_priv, jsthread **thread)
{
	*thread = عدم;
	أرجع NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	أرجع NSERROR_OK;
}

فراغ js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, ثابت uint8_t *txt, size_t txtlen, ثابت حرف *name)
{
	أرجع true;
}

bool js_fire_event(jsthread *thread, ثابت حرف *type, بنية dom_document *doc, بنية dom_نود *target)
{
	أرجع true;
}

فراغ js_handle_new_element(jsthread *thread, بنية dom_element *نود)
{
}

فراغ js_event_cleanup(jsthread *thread, بنية dom_event *evt)
{
}
