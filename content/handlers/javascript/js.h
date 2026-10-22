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

#నిర్వచితం_కాకపోతే NETSURF_JAVASCRIPT_JS_H_
#నిర్వచించు NETSURF_JAVASCRIPT_JS_H_

#అన్తర్భావ "utils/errors.h"

నిర్మాణం dom_event;
నిర్మాణం dom_document;
నిర్మాణం dom_కణుపు;
నిర్మాణం dom_element;
నిర్మాణం dom_string;

/**
 * JavaScript interpreter heap
 *
 * In order to try and be moderately performant, we create a heap
 * per browser window.  This heap is shared by all browsing contexts
 * we end up creating in that window.
 */
రకనిర్వచనం నిర్మాణం jsheap jsheap;

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
రకనిర్వచనం నిర్మాణం jsthread jsthread;

/**
 * Initialise javascript interpreter
 */
శూన్య js_initialise(శూన్య);

/**
 * finalise javascript interpreter
 */
శూన్య js_finalise(శూన్య);

/**
 * Create a new javascript heap.
 *
 * There is usually one heap per browser window.
 *
 * \param timeout elapsed wallclock time (in seconds) before \a callback is called
 * \param heap Updated to the created JS heap
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror js_newheap(పూర్ణాంకం timeout, jsheap **కుప్ప);

/**
 * Destroy a previously created heap.
 *
 * \param heap The heap to destroy
 */
శూన్య js_destroyheap(jsheap *కుప్ప);

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
nserror js_newthread(jsheap *కుప్ప, శూన్య *win_priv, శూన్య *doc_priv, jsthread **thread);

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
శూన్య js_destroythread(jsthread *thread);

/**
 * execute some javascript in a context
 */
bool js_exec(jsthread *thread, స్థిరరాశి uint8_t *txt, size_t txtlen, స్థిరరాశి అక్షరం *name);

/**
 * fire an event at a dom node
 */
bool js_fire_event(jsthread *thread, స్థిరరాశి అక్షరం *type, నిర్మాణం dom_document *doc, నిర్మాణం dom_కణుపు *లక్ష్యం);

bool
js_dom_event_కూడు_listener(jsthread *thread,
			  నిర్మాణం dom_document *document,
			  నిర్మాణం dom_కణుపు *కణుపు,
			  నిర్మాణం dom_string *event_type_dom,
			  శూన్య *js_funcval);

/*** New Events ***/

/**
 * Handle a new element being created.
 *
 * This is called once an element is inserted into the DOM document handled
 * by the context provided.  The JS implementation must then scan the element
 * for on* attributes and register appropriate listeners for those handlers.
 */
శూన్య js_handle_new_element(jsthread *thread, నిర్మాణం dom_element *కణుపు);

/**
 * Handle an event propagation finished callback.
 *
 * This is called once an event finishes propagating, no matter how it
 * finishes.  The intent here is that the JS context can perform any cleanups
 * it may need to perform before the DOM finishes and the event may end up
 * freed.
 */
శూన్య js_event_cleanup(jsthread *thread, నిర్మాణం dom_event *evt);

#అయితే_ముగింపు /* NETSURF_JAVASCRIPT_JS_H_ */
