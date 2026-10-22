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

#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "content/content.h"
#અન્તર્ભાવ "utils/nsoption.h"

#અન્તર્ભાવ "javascript/js.h"
#અન્તર્ભાવ "utils/log.h"

શૂન્ય js_initialise(શૂન્ય)
{
	nsoption_set_bool(enable_javascript, false);
}

શૂન્ય js_finalise(શૂન્ય)
{
}

nserror js_newheap(પૂર્ણાંક timeout, jsheap **heap)
{
	*heap = ખાલી;
	પાછા NSERROR_OK;
}

શૂન્ય js_destroyheap(jsheap *heap)
{
}

nserror js_newthread(jsheap *heap, શૂન્ય *win_priv, શૂન્ય *doc_priv, jsthread **thread)
{
	*thread = ખાલી;
	પાછા NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	પાછા NSERROR_OK;
}

શૂન્ય js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, અચલ uint8_t *txt, size_t txtlen, અચલ અક્ષર *name)
{
	પાછા true;
}

bool js_fire_event(jsthread *thread, અચલ અક્ષર *type, સંરચના dom_document *doc, સંરચના dom_ગાંઠ *target)
{
	પાછા true;
}

શૂન્ય js_handle_new_element(jsthread *thread, સંરચના dom_element *ગાંઠ)
{
}

શૂન્ય js_event_cleanup(jsthread *thread, સંરચના dom_event *evt)
{
}
