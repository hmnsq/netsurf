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
 * Interface to javascript engine functions.
 */

#ifndef NETSURF_JAVASCRIPT_JS_H_
#define NETSURF_JAVASCRIPT_JS_H_

#অন্তর্ভাব "utils/errors.h"

কাঠামো dom_event;
কাঠামো dom_document;
কাঠামো dom_গ্রন্থি;
কাঠামো dom_element;
কাঠামো dom_string;

/**
 * JavaScript interpreter heap
 *
 * In order to try and be moderately performant, we create a heap
 * per browser window.  This heap is shared by all browsing contexts
 * we end up creating in that window.
 */
প্রকারলেখ কাঠামো jsheap jsheap;

/**
 * JavaScript interpreter thread
 *
 * When we create a browsing context itself (window+content) we have
 * to create a JS thread to attach to the browsing context.
 *
 * JS threads are associated with heaps and will be destroyed when
 * the heap is destroyed.  They can be shut down manually though
 * and should be for object lifetime safety reasons.
 */
প্রকারলেখ কাঠামো jsthread jsthread;

/**
 * Initialise javascript interpreter
 */
শূন্য js_initialise(শূন্য);

/**
 * finalise javascript interpreter
 */
শূন্য js_finalise(শূন্য);

/**
 * Create a new javascript heap.
 *
 * There is usually one heap per browser window.
 *
 * \param timeout elapsed wallclock time (in seconds) before \a callback is called
 * \param heap Updated to the created JS heap
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror js_newheap(পূর্ণসংখ্যা timeout, jsheap **heap);

/**
 * Destroy a previously created heap.
 *
 * \param heap The heap to destroy
 */
শূন্য js_destroyheap(jsheap *heap);

/**
 * Create a new javascript thread
 *
 * This is called once for a page with javascript script tags on
 * it. It constructs a fresh global window object and prepares the JS
 * browsing context.  It's important that threads are shut down cleanly
 * when the browsing context is going to be cleaned up.
 *
 * \param heap The heap to create the thread within
 * \param win_priv The value to give to the Window constructor as the window
 * \param doc_priv The value to give to the Document constructor as the document
 * \param thread Updated to the created thread
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror js_newthread(jsheap *heap, শূন্য *win_priv, শূন্য *doc_priv, jsthread **thread);

/**
 * Close a javascript thread
 *
 * This should be called when the HTML content which owns the thread is
 * being closed.  This is a separate process from destroying the thread
 * and merely disconnects any callbacks and thus hopefully stops
 * additional JS things from triggering.  If any code runs and attempts to
 * register callbacks after closedown, they will fail.
 *
 * \param thread The thread to close down
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror js_closethread(jsthread *thread);

/**
 * Destroy a javascript thread
 *
 * This should be called when the browsing context is done with the thread.
 *
 * This will be called when the HTML content associated with the browsing
 * context is being destroyed.  The thread should have already been closed
 * down during the HTML content close.
 *
 * \param thread The thread to be destroyed
 */
শূন্য js_destroythread(jsthread *thread);

/**
 * execute some javascript in a context
 */
bool js_exec(jsthread *thread, ধ্রুবক uint8_t *txt, size_t txtlen, ধ্রুবক অক্ষর *name);

/**
 * fire an event at a dom node
 */
bool js_fire_event(jsthread *thread, ধ্রুবক অক্ষর *type, কাঠামো dom_document *doc, কাঠামো dom_গ্রন্থি *target);

bool
js_dom_event_add_listener(jsthread *thread,
			  কাঠামো dom_document *document,
			  কাঠামো dom_গ্রন্থি *গ্রন্থি,
			  কাঠামো dom_string *event_type_dom,
			  শূন্য *js_funcval);

/*** New Events ***/

/**
 * Handle a new element being created.
 *
 * This is called once an element is inserted into the DOM document handled
 * by the context provided.  The JS implementation must then scan the element
 * for on* attributes and register appropriate listeners for those handlers.
 */
শূন্য js_handle_new_element(jsthread *thread, কাঠামো dom_element *গ্রন্থি);

/**
 * Handle an event propagation finished callback.
 *
 * This is called once an event finishes propagating, no matter how it
 * finishes.  The intent here is that the JS context can perform any cleanups
 * it may need to perform before the DOM finishes and the event may end up
 * freed.
 */
শূন্য js_event_cleanup(jsthread *thread, কাঠামো dom_event *evt);

#endif /* NETSURF_JAVASCRIPT_JS_H_ */
