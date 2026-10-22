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

#ଅନ୍ତର୍ଭାଵ "utils/errors.h"

ସଂରଚନା dom_event;
ସଂରଚନା dom_document;
ସଂରଚନା dom_ଗଣ୍ଠି;
ସଂରଚନା dom_element;
ସଂରଚନା dom_string;

/**
 * JavaScript interpreter heap
 *
 * In order to try and be moderately performant, we create a heap
 * per browser window.  This heap is shared by all browsing contexts
 * we end up creating in that window.
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା jsheap jsheap;

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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା jsthread jsthread;

/**
 * Initialise javascript interpreter
 */
ଶୂନ୍ୟ js_initialise(ଶୂନ୍ୟ);

/**
 * finalise javascript interpreter
 */
ଶୂନ୍ୟ js_finalise(ଶୂନ୍ୟ);

/**
 * Create a new javascript heap.
 *
 * There is usually one heap per browser window.
 *
 * \param timeout elapsed wallclock time (in seconds) before \a callback is called
 * \param heap Updated to the created JS heap
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror js_newheap(ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout, jsheap **heap);

/**
 * Destroy a previously created heap.
 *
 * \param heap The heap to destroy
 */
ଶୂନ୍ୟ js_destroyheap(jsheap *heap);

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
nserror js_newthread(jsheap *heap, ଶୂନ୍ୟ *win_priv, ଶୂନ୍ୟ *doc_priv, jsthread **thread);

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
ଶୂନ୍ୟ js_destroythread(jsthread *thread);

/**
 * execute some javascript in a context
 */
bool js_exec(jsthread *thread, ଅଚଳ uint8_t *txt, size_t txtlen, ଅଚଳ ଅକ୍ଷର *name);

/**
 * fire an event at a dom node
 */
bool js_fire_event(jsthread *thread, ଅଚଳ ଅକ୍ଷର *type, ସଂରଚନା dom_document *doc, ସଂରଚନା dom_ଗଣ୍ଠି *target);

bool
js_dom_event_add_listener(jsthread *thread,
			  ସଂରଚନା dom_document *document,
			  ସଂରଚନା dom_ଗଣ୍ଠି *ଗଣ୍ଠି,
			  ସଂରଚନା dom_string *event_type_dom,
			  ଶୂନ୍ୟ *js_funcval);

/*** New Events ***/

/**
 * Handle a new element being created.
 *
 * This is called once an element is inserted into the DOM document handled
 * by the context provided.  The JS implementation must then scan the element
 * for on* attributes and register appropriate listeners for those handlers.
 */
ଶୂନ୍ୟ js_handle_new_element(jsthread *thread, ସଂରଚନା dom_element *ଗଣ୍ଠି);

/**
 * Handle an event propagation finished callback.
 *
 * This is called once an event finishes propagating, no matter how it
 * finishes.  The intent here is that the JS context can perform any cleanups
 * it may need to perform before the DOM finishes and the event may end up
 * freed.
 */
ଶୂନ୍ୟ js_event_cleanup(jsthread *thread, ସଂରଚନା dom_event *evt);

#endif /* NETSURF_JAVASCRIPT_JS_H_ */
