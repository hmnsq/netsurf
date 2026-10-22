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

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "content/content.h"
#അന്തര്ഭാവ "utils/nsoption.h"

#അന്തര്ഭാവ "javascript/js.h"
#അന്തര്ഭാവ "utils/log.h"

ശൂന്യം js_initialise(ശൂന്യം)
{
	nsoption_set_bool(enable_javascript, false);
}

ശൂന്യം js_finalise(ശൂന്യം)
{
}

nserror js_newheap(പൂർണ്ണസംഖ്യ timeout, jsheap **heap)
{
	*heap = ശൂന്യ;
	തിരികെ NSERROR_OK;
}

ശൂന്യം js_destroyheap(jsheap *heap)
{
}

nserror js_newthread(jsheap *heap, ശൂന്യം *win_priv, ശൂന്യം *doc_priv, jsthread **thread)
{
	*thread = ശൂന്യ;
	തിരികെ NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	തിരികെ NSERROR_OK;
}

ശൂന്യം js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, സ്ഥിരം uint8_t *txt, size_t txtlen, സ്ഥിരം അക്ഷരം *name)
{
	തിരികെ true;
}

bool js_fire_event(jsthread *thread, സ്ഥിരം അക്ഷരം *type, ഘടന dom_document *doc, ഘടന dom_കണ്ണി *target)
{
	തിരികെ true;
}

ശൂന്യം js_handle_new_element(jsthread *thread, ഘടന dom_element *കണ്ണി)
{
}

ശൂന്യം js_event_cleanup(jsthread *thread, ഘടന dom_event *evt)
{
}
