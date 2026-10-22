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

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "javascript/js.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"

ਖਾਲੀ js_initialise(ਖਾਲੀ)
{
	nsoption_ਸੈੱਟ_bool(enable_javascript, false);
}

ਖਾਲੀ js_finalise(ਖਾਲੀ)
{
}

nserror js_newheap(ਪੂਰਨ timeout, jsheap **ਢੇਰ)
{
	*ਢੇਰ = ਨਿਰਾ;
	ਵਾਪਸ NSERROR_OK;
}

ਖਾਲੀ js_destroyheap(jsheap *ਢੇਰ)
{
}

nserror js_newthread(jsheap *ਢੇਰ, ਖਾਲੀ *win_priv, ਖਾਲੀ *doc_priv, jsthread **thread)
{
	*thread = ਨਿਰਾ;
	ਵਾਪਸ NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	ਵਾਪਸ NSERROR_OK;
}

ਖਾਲੀ js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, ਅਚਲ uint8_t *txt, size_t txtlen, ਅਚਲ ਅੱਖਰ *name)
{
	ਵਾਪਸ true;
}

bool js_fire_event(jsthread *thread, ਅਚਲ ਅੱਖਰ *type, ਬਣਤਰ dom_document *doc, ਬਣਤਰ dom_ਗੰਢ *ਨਿਸ਼ਾਨਾ)
{
	ਵਾਪਸ true;
}

ਖਾਲੀ js_handle_new_element(jsthread *thread, ਬਣਤਰ dom_element *ਗੰਢ)
{
}

ਖਾਲੀ js_event_cleanup(jsthread *thread, ਬਣਤਰ dom_event *evt)
{
}
