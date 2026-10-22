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

#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "content/content.h"
#அந்தர்பாவ "utils/nsoption.h"

#அந்தர்பாவ "javascript/js.h"
#அந்தர்பாவ "utils/log.h"

வெறுமை js_initialise(வெறுமை)
{
	nsoption_set_bool(enable_javascript, false);
}

வெறுமை js_finalise(வெறுமை)
{
}

nserror js_newheap(முழு timeout, jsheap **heap)
{
	*heap = வெற்று;
	திரும்பு NSERROR_OK;
}

வெறுமை js_destroyheap(jsheap *heap)
{
}

nserror js_newthread(jsheap *heap, வெறுமை *win_priv, வெறுமை *doc_priv, jsthread **thread)
{
	*thread = வெற்று;
	திரும்பு NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	திரும்பு NSERROR_OK;
}

வெறுமை js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, மாறிலி uint8_t *txt, size_t txtlen, மாறிலி எழுத்து *name)
{
	திரும்பு true;
}

bool js_fire_event(jsthread *thread, மாறிலி எழுத்து *type, கட்டமைப்பு dom_document *doc, கட்டமைப்பு dom_முனை *target)
{
	திரும்பு true;
}

வெறுமை js_handle_new_element(jsthread *thread, கட்டமைப்பு dom_element *முனை)
{
}

வெறுமை js_event_cleanup(jsthread *thread, கட்டமைப்பு dom_event *evt)
{
}
