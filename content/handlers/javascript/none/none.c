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

تهی js_initialise(تهی)
{
	nsoption_set_bool(enable_javascript, false);
}

تهی js_finalise(تهی)
{
}

nserror js_newheap(صحیح timeout, jsheap **heap)
{
	*heap = پوچ;
	بازگشت NSERROR_OK;
}

تهی js_destroyheap(jsheap *heap)
{
}

nserror js_newthread(jsheap *heap, تهی *win_priv, تهی *doc_priv, jsthread **thread)
{
	*thread = پوچ;
	بازگشت NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	بازگشت NSERROR_OK;
}

تهی js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, ثابت uint8_t *txt, size_t txtlen, ثابت نویسه *name)
{
	بازگشت true;
}

bool js_fire_event(jsthread *thread, ثابت نویسه *type, ساختار dom_document *doc, ساختار dom_گره *target)
{
	بازگشت true;
}

تهی js_handle_new_element(jsthread *thread, ساختار dom_element *گره)
{
}

تهی js_event_cleanup(jsthread *thread, ساختار dom_event *evt)
{
}
