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

#شامل "utils/errors.h"
#شامل "content/content.h"
#شامل "utils/nsoption.h"

#شامل "javascript/js.h"
#شامل "utils/log.h"

خالی js_initialise(خالی)
{
	nsoption_سیٹ_bool(enable_javascript, false);
}

خالی js_finalise(خالی)
{
}

nserror js_newheap(صحیح timeout, jsheap **ڈھیر)
{
	*ڈھیر = تہی;
	واپس NSERROR_OK;
}

خالی js_destroyheap(jsheap *ڈھیر)
{
}

nserror js_newthread(jsheap *ڈھیر, خالی *win_priv, خالی *doc_priv, jsthread **thread)
{
	*thread = تہی;
	واپس NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	واپس NSERROR_OK;
}

خالی js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, ثابت uint8_t *txt, size_t txtlen, ثابت حرف *name)
{
	واپس true;
}

bool js_fire_event(jsthread *thread, ثابت حرف *type, ساخت dom_document *doc, ساخت dom_گرہ *ہدف)
{
	واپس true;
}

خالی js_handle_new_element(jsthread *thread, ساخت dom_element *گرہ)
{
}

خالی js_event_cleanup(jsthread *thread, ساخت dom_event *evt)
{
}
