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

#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "content/content.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"

#ଅନ୍ତର୍ଭାଵ "javascript/js.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"

ଶୂନ୍ୟ js_initialise(ଶୂନ୍ୟ)
{
	nsoption_set_bool(enable_javascript, false);
}

ଶୂନ୍ୟ js_finalise(ଶୂନ୍ୟ)
{
}

nserror js_newheap(ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout, jsheap **heap)
{
	*heap = ଖାଲି;
	ଫେରନ୍ତୁ NSERROR_OK;
}

ଶୂନ୍ୟ js_destroyheap(jsheap *heap)
{
}

nserror js_newthread(jsheap *heap, ଶୂନ୍ୟ *win_priv, ଶୂନ୍ୟ *doc_priv, jsthread **thread)
{
	*thread = ଖାଲି;
	ଫେରନ୍ତୁ NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	ଫେରନ୍ତୁ NSERROR_OK;
}

ଶୂନ୍ୟ js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, ଅଚଳ uint8_t *txt, size_t txtlen, ଅଚଳ ଅକ୍ଷର *name)
{
	ଫେରନ୍ତୁ true;
}

bool js_fire_event(jsthread *thread, ଅଚଳ ଅକ୍ଷର *type, ସଂରଚନା dom_document *doc, ସଂରଚନା dom_ଗଣ୍ଠି *target)
{
	ଫେରନ୍ତୁ true;
}

ଶୂନ୍ୟ js_handle_new_element(jsthread *thread, ସଂରଚନା dom_element *ଗଣ୍ଠି)
{
}

ଶୂନ୍ୟ js_event_cleanup(jsthread *thread, ସଂରଚନା dom_event *evt)
{
}
