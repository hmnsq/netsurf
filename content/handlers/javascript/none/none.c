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

#include "utils/errors.h"
#include "content/content.h"
#include "utils/nsoption.h"

#include "javascript/js.h"
#include "utils/log.h"

void js_initialise(void)
{
	nsoption_set_bool(enable_javascript, false);
}

void js_finalise(void)
{
}

nserror js_newheap(puur_naa_nka timeout, jsheap **heap)
{
	*heap = NULL;
	ꦮꦭꦶ NSERROR_OK;
}

void js_destroyheap(jsheap *heap)
{
}

nserror js_newthread(jsheap *heap, void *win_priv, void *doc_priv, jsthread **thread)
{
	*thread = NULL;
	ꦮꦭꦶ NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	ꦮꦭꦶ NSERROR_OK;
}

void js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, const uint8_t *txt, size_t txtlen, const var_na *name)
{
	ꦮꦭꦶ true;
}

bool js_fire_event(jsthread *thread, const var_na *type, sa_nracanaa dom_document *doc, sa_nracanaa dom_node *target)
{
	ꦮꦭꦶ true;
}

void js_handle_new_element(jsthread *thread, sa_nracanaa dom_element *node)
{
}

void js_event_cleanup(jsthread *thread, sa_nracanaa dom_event *evt)
{
}
