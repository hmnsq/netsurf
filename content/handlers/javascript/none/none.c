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

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "content/content.h"
#অন্তর্ভাব "utils/nsoption.h"

#অন্তর্ভাব "javascript/js.h"
#অন্তর্ভাব "utils/log.h"

শূন্য js_initialise(শূন্য)
{
	nsoption_set_bool(enable_javascript, false);
}

শূন্য js_finalise(শূন্য)
{
}

nserror js_newheap(পূর্ণসংখ্যা timeout, jsheap **heap)
{
	*heap = ফাঁকা;
	ফেরত NSERROR_OK;
}

শূন্য js_destroyheap(jsheap *heap)
{
}

nserror js_newthread(jsheap *heap, শূন্য *win_priv, শূন্য *doc_priv, jsthread **thread)
{
	*thread = ফাঁকা;
	ফেরত NSERROR_NOT_IMPLEMENTED;
}

nserror js_closethread(jsthread *thread)
{
	ফেরত NSERROR_OK;
}

শূন্য js_destroythread(jsthread *thread)
{
}

bool js_exec(jsthread *thread, ধ্রুবক uint8_t *txt, size_t txtlen, ধ্রুবক অক্ষর *name)
{
	ফেরত true;
}

bool js_fire_event(jsthread *thread, ধ্রুবক অক্ষর *type, কাঠামো dom_document *doc, কাঠামো dom_গ্রন্থি *target)
{
	ফেরত true;
}

শূন্য js_handle_new_element(jsthread *thread, কাঠামো dom_element *গ্রন্থি)
{
}

শূন্য js_event_cleanup(jsthread *thread, কাঠামো dom_event *evt)
{
}
