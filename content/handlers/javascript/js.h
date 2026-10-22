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

#അന്തര്ഭാവ "utils/errors.h"

ഘടന dom_event;
ഘടന dom_document;
ഘടന dom_കണ്ണി;
ഘടന dom_element;
ഘടന dom_string;

/**
 * JavaScript interpreter heap
 *
 * In order to try and be moderately performant, we create a heap
 * per browser window.  This heap is shared by all browsing contexts
 * we end up creating in that window.
 */
തരനിർവചനം ഘടന jsheap jsheap;

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
തരനിർവചനം ഘടന jsthread jsthread;

/**
 * Initialise javascript interpreter
 */
ശൂന്യം js_initialise(ശൂന്യം);

/**
 * finalise javascript interpreter
 */
ശൂന്യം js_finalise(ശൂന്യം);

/**
 * Create a new javascript heap.
 *
 * There is usually one heap per browser window.
 *
 * \param timeout elapsed wallclock time (in seconds) before \a callback is called
 * \param heap Updated to the created JS heap
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror js_newheap(പൂർണ്ണസംഖ്യ timeout, jsheap **heap);

/**
 * Destroy a previously created heap.
 *
 * \param heap The heap to destroy
 */
ശൂന്യം js_destroyheap(jsheap *heap);

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
nserror js_newthread(jsheap *heap, ശൂന്യം *win_priv, ശൂന്യം *doc_priv, jsthread **thread);

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
ശൂന്യം js_destroythread(jsthread *thread);

/**
 * execute some javascript in a context
 */
bool js_exec(jsthread *thread, സ്ഥിരം uint8_t *txt, size_t txtlen, സ്ഥിരം അക്ഷരം *name);

/**
 * fire an event at a dom node
 */
bool js_fire_event(jsthread *thread, സ്ഥിരം അക്ഷരം *type, ഘടന dom_document *doc, ഘടന dom_കണ്ണി *target);

bool
js_dom_event_add_listener(jsthread *thread,
			  ഘടന dom_document *document,
			  ഘടന dom_കണ്ണി *കണ്ണി,
			  ഘടന dom_string *event_type_dom,
			  ശൂന്യം *js_funcval);

/*** New Events ***/

/**
 * Handle a new element being created.
 *
 * This is called once an element is inserted into the DOM document handled
 * by the context provided.  The JS implementation must then scan the element
 * for on* attributes and register appropriate listeners for those handlers.
 */
ശൂന്യം js_handle_new_element(jsthread *thread, ഘടന dom_element *കണ്ണി);

/**
 * Handle an event propagation finished callback.
 *
 * This is called once an event finishes propagating, no matter how it
 * finishes.  The intent here is that the JS context can perform any cleanups
 * it may need to perform before the DOM finishes and the event may end up
 * freed.
 */
ശൂന്യം js_event_cleanup(jsthread *thread, ഘടന dom_event *evt);

#endif /* NETSURF_JAVASCRIPT_JS_H_ */
