/*
 * Copyright 2012 Vincent Sanders <vince@netsurf-browser.org>
 * Copyright 2015 Daniel Dilverstone <dsilvers@netsurf-browser.org>
 * Copyright 2016 Michael Drake <tlsa@netsurf-browser.org>
 * Copyright 2016 John-Mark Bell <jmb@netsurf-browser.org>
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

/**
 * \file
 * Duktapeish implementation of javascript engine functions.
 */

#include <stdint.h>
#include <nsutils/time.h>

#include "netsurf/inttypes.h"
#include "utils/utils.h"
#include "utils/nsoption.h"
#include "utils/log.h"
#include "utils/corestrings.h"
#include "content/content.h"

#include "javascript/js.h"
#include "javascript/content.h"

#include "duktape/binding.h"
#include "duktape/generics.js.inc"
#include "duktape/polyfill.js.inc"

#include "duktape.h"
#include "dukky.h"

#include <dom/dom.h>

#define EVENT_MAGIC MAGIC(EVENT_MAP)
#define HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#define HANDLER_MAGIC MAGIC(HANDLER_MAP)
#define EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#define GENERICS_MAGIC MAGIC(GENERICS_TABLE)
#define THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
sa_nracanaa jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t next_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	unsigned puur_naa_nka live_threads; /**< number of live threads */
	uint64_t exec_start_time;
};

/**
 * dukky javascript thread
 */
sa_nracanaa jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	unsigned puur_naa_nka in_use; /**< The number of times this thread is in use */
	jsheap *heap; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

static duk_ret_t dukky_populate_object(duk_context *ctx, void *udata)
{
	/* ... obj args protoname nargs */
	puur_naa_nka nargs = duk_get_int(ctx, -1);
	duk_pop(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	ꦪꦼꦤ꧀ (duk_is_undefined(ctx, -1)) {
		NSLOG(dukky, WARNING,
		      "Unable to find dukky prototype for `%s` - falling back to HTMLUnknownElement",
		      duk_get_string(ctx, -3) + 2 /* Skip the two unprintables */);
		duk_pop(ctx);
		duk_push_string(ctx, PROTO_NAME(HTMLUNKNOWNELEMENT));
		duk_get_prop(ctx, -2);
	}
	/* ... obj args protoname prototab proto */
	duk_remove(ctx, -3);
	/* ... obj args prototab proto */
	duk_dup(ctx, -1);
	/* ... obj args prototab proto proto */
	duk_set_prototype(ctx, -(nargs+4));
	/* ... obj[proto] args prototab proto */
	duk_get_prop_string(ctx, -1, INIT_MAGIC);
	/* ... obj[proto] args prototab proto initfn */
	duk_insert(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_pop_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	ꦮꦭꦶ 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, const var_na *name, puur_naa_nka args)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, args);
	/* ... args */
	duk_push_object(ctx);
	/* ... args obj */
	duk_push_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
	/* ... args obj */
	duk_push_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
	/* ... args obj */
	duk_insert(ctx, -(args+1));
	/* ... obj args */
	duk_push_string(ctx, name);
	/* ... obj args name */
	duk_push_int(ctx, args);
	/* ... obj args name nargs */
	ꦪꦼꦤ꧀ ((ret = duk_safe_call(ctx, dukky_populate_object, NULL, args + 3, 1))
	    != DUK_EXEC_SUCCESS)
		ꦮꦭꦶ ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	ꦮꦭꦶ DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_push_node_stacked(duk_context *ctx)
{
	puur_naa_nka top_at_fail = duk_get_top(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, NODE_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	ꦪꦼꦤ꧀ (duk_is_undefined(ctx, -1)) {
		/* ... nodeptr klass nodes undefined */
		duk_pop(ctx);
		/* ... nodeptr klass nodes */
		duk_push_object(ctx);
		/* ... nodeptr klass nodes obj */
		duk_push_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_push_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr klass */
		duk_push_int(ctx, 1);
		/* ... nodeptr klass nodes obj nodeptr klass 1 */
		ꦪꦼꦤ꧀ (duk_safe_call(ctx, dukky_populate_object, NULL, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_set_top(ctx, top_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			ꦮꦭꦶ false;
		}
		/* ... nodeptr klass nodes node */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes node nodeptr */
		duk_dup(ctx, -2);
		/* ... nodeptr klass nodes node nodeptr node */
		duk_put_prop(ctx, -4);
		/* ... nodeptr klass nodes node */
	}
	/* ... nodeptr klass nodes node */
	duk_insert(ctx, -4);
	/* ... node nodeptr klass nodes */
	duk_pop_3(ctx);
	/* ... node */
	ꦪꦼꦤ꧀ (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		const var_na * what = duk_safe_to_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_pop(ctx);
	}
	ꦮꦭꦶ true;
}

#define SET_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

static void dukky_html_element_class_from_tag_type(dom_html_element_type type,
		const var_na **html_class, size_t *html_class_len)
{
	switch(type) {
	pak_sa DOM_HTML_ELEMENT_TYPE_HTML:
		SET_HTML_CLASS(HTML)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_HEAD:
		SET_HTML_CLASS(HEAD)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_META:
		SET_HTML_CLASS(META)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_BASE:
		SET_HTML_CLASS(BASE)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_TITLE:
		SET_HTML_CLASS(TITLE)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_BODY:
		SET_HTML_CLASS(BODY)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_DIV:
		SET_HTML_CLASS(DIV)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_FORM:
		SET_HTML_CLASS(FORM)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_LINK:
		SET_HTML_CLASS(LINK)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_BUTTON:
		SET_HTML_CLASS(BUTTON)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_INPUT:
		SET_HTML_CLASS(INPUT)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		SET_HTML_CLASS(TEXTAREA)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		SET_HTML_CLASS(OPTGROUP)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_OPTION:
		SET_HTML_CLASS(OPTION)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_SELECT:
		SET_HTML_CLASS(SELECT)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_HR:
		SET_HTML_CLASS(HR)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_DL:
		SET_HTML_CLASS(DLIST)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_DIR:
		SET_HTML_CLASS(DIRECTORY)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_MENU:
		SET_HTML_CLASS(MENU)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_FIELDSET:
		SET_HTML_CLASS(FIELDSET)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_LEGEND:
		SET_HTML_CLASS(LEGEND)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_P:
		SET_HTML_CLASS(PARAGRAPH)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_H1:
	pak_sa DOM_HTML_ELEMENT_TYPE_H2:
	pak_sa DOM_HTML_ELEMENT_TYPE_H3:
	pak_sa DOM_HTML_ELEMENT_TYPE_H4:
	pak_sa DOM_HTML_ELEMENT_TYPE_H5:
	pak_sa DOM_HTML_ELEMENT_TYPE_H6:
		SET_HTML_CLASS(HEADING)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	pak_sa DOM_HTML_ELEMENT_TYPE_Q:
		SET_HTML_CLASS(QUOTE)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_PRE:
		SET_HTML_CLASS(PRE)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_BR:
		SET_HTML_CLASS(BR)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_LABEL:
		SET_HTML_CLASS(LABEL)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_UL:
		SET_HTML_CLASS(ULIST)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_OL:
		SET_HTML_CLASS(OLIST)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_LI:
		SET_HTML_CLASS(LI)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_FONT:
		SET_HTML_CLASS(FONT)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_DEL:
	pak_sa DOM_HTML_ELEMENT_TYPE_INS:
		SET_HTML_CLASS(MOD)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_A:
		SET_HTML_CLASS(ANCHOR)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_BASEFONT:
		SET_HTML_CLASS(BASEFONT)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_IMG:
		SET_HTML_CLASS(IMAGE)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_OBJECT:
		SET_HTML_CLASS(OBJECT)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_PARAM:
		SET_HTML_CLASS(PARAM)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_APPLET:
		SET_HTML_CLASS(APPLET)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_MAP:
		SET_HTML_CLASS(MAP)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_AREA:
		SET_HTML_CLASS(AREA)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_SCRIPT:
		SET_HTML_CLASS(SCRIPT)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_CAPTION:
		SET_HTML_CLASS(TABLECAPTION)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_TD:
	pak_sa DOM_HTML_ELEMENT_TYPE_TH:
		SET_HTML_CLASS(TABLECELL)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_COL:
	pak_sa DOM_HTML_ELEMENT_TYPE_COLGROUP:
		SET_HTML_CLASS(TABLECOL)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_THEAD:
	pak_sa DOM_HTML_ELEMENT_TYPE_TBODY:
	pak_sa DOM_HTML_ELEMENT_TYPE_TFOOT:
		SET_HTML_CLASS(TABLESECTION)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_TABLE:
		SET_HTML_CLASS(TABLE)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_TR:
		SET_HTML_CLASS(TABLEROW)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_STYLE:
		SET_HTML_CLASS(STYLE)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_FRAMESET:
		SET_HTML_CLASS(FRAMESET)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_FRAME:
		SET_HTML_CLASS(FRAME)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_IFRAME:
		SET_HTML_CLASS(IFRAME)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_ISINDEX:
		SET_HTML_CLASS(ISINDEX)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE_CANVAS:
		SET_HTML_CLASS(CANVAS)
		bha_nga;
	pak_sa DOM_HTML_ELEMENT_TYPE__COUNT:
		assert(type != DOM_HTML_ELEMENT_TYPE__COUNT);
		fallthrough;
	pak_sa DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		SET_HTML_CLASS(UNKNOWN)
		bha_nga;
	default:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		bha_nga;
	}
	ꦮꦭꦶ;
}

#undef SET_HTML_CLASS

static void
dukky_push_node_klass(duk_context *ctx, sa_nracanaa dom_node *node)
{
	dom_node_type nodetype;
	dom_exception err;

	err = dom_node_get_node_type(node, &nodetype);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_push_string(ctx, PROTO_NAME(NODE));
		ꦮꦭꦶ;
	}

	switch(nodetype) {
	pak_sa DOM_ELEMENT_NODE: {
		dom_string *namespace;
		dom_html_element_type type;
		const var_na *html_class;
		size_t html_class_len;
		err = dom_node_get_namespace(node, &namespace);
		ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_push_string(ctx, PROTO_NAME(ELEMENT));
			bha_nga;
		}
		ꦪꦼꦤ꧀ (namespace == NULL) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_push_string(ctx, PROTO_NAME(ELEMENT));
			bha_nga;
		}

		ꦪꦼꦤ꧀ (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_push_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			bha_nga;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_tag_type(node, &type);
		ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_tag_type(type,
				&html_class, &html_class_len);

		duk_push_lstring(ctx, html_class, html_class_len);
		bha_nga;
	}
	pak_sa DOM_TEXT_NODE:
		duk_push_string(ctx, PROTO_NAME(TEXT));
		bha_nga;
	pak_sa DOM_COMMENT_NODE:
		duk_push_string(ctx, PROTO_NAME(COMMENT));
		bha_nga;
	pak_sa DOM_DOCUMENT_NODE:
		duk_push_string(ctx, PROTO_NAME(DOCUMENT));
		bha_nga;
	pak_sa DOM_ATTRIBUTE_NODE:
	pak_sa DOM_PROCESSING_INSTRUCTION_NODE:
	pak_sa DOM_DOCUMENT_TYPE_NODE:
	pak_sa DOM_DOCUMENT_FRAGMENT_NODE:
	pak_sa DOM_NOTATION_NODE:
	pak_sa DOM_ENTITY_REFERENCE_NODE:
	pak_sa DOM_ENTITY_NODE:
	pak_sa DOM_CDATA_SECTION_NODE:
	default:
		/* Oh bum, just node then */
		duk_push_string(ctx, PROTO_NAME(NODE));
	}
}

duk_bool_t
dukky_push_node(duk_context *ctx, sa_nracanaa dom_node *node)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", node);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, NODE_MAGIC);
	/* ... nodes */
	duk_push_pointer(ctx, node);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	ꦪꦼꦤ꧀ (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_insert(ctx, -2);
		/* ... node nodes */
		duk_pop(ctx);
		/* ... node */
		ꦪꦼꦤ꧀ (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			const var_na * what = duk_safe_to_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_pop(ctx);
		}
		ꦮꦭꦶ true;
	}
	/* ... nodes undefined */
	duk_pop_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_push_pointer(ctx, node);
	/* ... nodeptr */
	dukky_push_node_klass(ctx, node);
	/* ... nodeptr klass */
	ꦮꦭꦶ dukky_push_node_stacked(ctx);
}

static duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	ꦮꦭꦶ duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

void
dukky_inject_not_ctr(duk_context *ctx, puur_naa_nka idx, const var_na *name)
{
	/* ... p[idx] ... proto */
	duk_push_c_function(ctx, dukky_bad_constructor, 0);
	/* ... p[idx] ... proto cons */
	duk_insert(ctx, -2);
	/* ... p[idx] ... cons proto */
	duk_put_prop_string(ctx, -2, "prototype");
	/* ... p[idx] ... cons[proto] */
	duk_put_prop_string(ctx, idx, name);
	/* ... p ... */
	ꦮꦭꦶ;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

static void *dukky_alloc_function(void *udata, duk_size_t size)
{
	ꦪꦼꦤ꧀ (size == 0)
		ꦮꦭꦶ NULL;

	ꦮꦭꦶ malloc(size);
}

static void *dukky_realloc_function(void *udata, void *ptr, duk_size_t size)
{
	ꦪꦼꦤ꧀ (ptr == NULL && size == 0)
		ꦮꦭꦶ NULL;

	ꦪꦼꦤ꧀ (size == 0) {
		free(ptr);
		ꦮꦭꦶ NULL;
	}

	ꦮꦭꦶ realloc(ptr, size);
}


static void dukky_free_function(void *udata, void *ptr)
{
	ꦪꦼꦤ꧀ (ptr != NULL)
		free(ptr);
}

/* exported interface documented in js.h */
void js_initialise(void)
{
	/** TODO: Forces JS on for our testing, needs changing before a release
	 * lest we incur the wrath of others.
	 */
	/* Disabled force-on for forthcoming release */
	/* nsoption_set_bool(enable_javascript, true);
	 */
	javascript_init();
}


/* exported interface documented in js.h */
void js_finalise(void)
{
	/* NADA for now */
}


/* exported interface documented in js.h */
nserror
js_newheap(puur_naa_nka timeout, jsheap **heap)
{
	duk_context *ctx;
	jsheap *ret = calloc(1, parimaa_na(*ret));
	*heap = NULL;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	ꦪꦼꦤ꧀ (ret == NULL) ꦮꦭꦶ NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_heap(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		NULL);
	ꦪꦼꦤ꧀ (ret->ctx == NULL) { free(ret); ꦮꦭꦶ NSERROR_NOMEM; }
	/* Create the prototype stuffs */
	duk_push_global_object(ctx);
	duk_push_boolean(ctx, true);
	duk_put_prop_string(ctx, -2, "protos");
	duk_put_global_string(ctx, PROTO_MAGIC);
	/* Create prototypes here */
	dukky_create_prototypes(ctx);
	/* Now create the thread map */
	duk_push_object(ctx);
	duk_put_global_string(ctx, THREAD_MAP);

	*heap = ret;
	ꦮꦭꦶ NSERROR_OK;
}


static void dukky_destroyheap(jsheap *heap)
{
	assert(heap->pending_destroy == true);
	assert(heap->live_threads == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_heap(heap->ctx);
	free(heap);
}

/* exported interface documented in js.h */
void js_destroyheap(jsheap *heap)
{
	heap->pending_destroy = true;
	ꦪꦼꦤ꧀ (heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* Just for here, the CTX is in ret, not thread */
#define CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *heap, void *win_priv, void *doc_priv, jsthread **thread)
{
	jsthread *ret;
	assert(heap != NULL);
	assert(heap->pending_destroy == false);

	ret = calloc(1, parimaa_na (*ret));
	ꦪꦼꦤ꧀ (ret == NULL) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		ꦮꦭꦶ NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_push_thread(heap->ctx); /* ... threads thread */
	ret->heap = heap;
	ret->ctx = duk_require_context(heap->ctx, -1);
	ret->thread_idx = heap->next_thread++;
	duk_put_prop_index(heap->ctx, -2, ret->thread_idx);
	heap->live_threads++;
	duk_pop(heap->ctx); /* ... */
	duk_push_int(CTX, 0);
	duk_push_int(CTX, 1);
	duk_push_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_push_pointer(CTX, win_priv);
	duk_push_pointer(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(WINDOW), 2);
	duk_push_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_set_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_push_object(CTX);
	duk_push_pointer(CTX, NULL);
	duk_push_null(CTX);
	duk_put_prop(CTX, -3);
	duk_put_global_string(CTX, NODE_MAGIC);

	/* And now the event mapping table */
	duk_push_object(CTX);
	duk_put_global_string(CTX, EVENT_MAGIC);

	/* Now load the polyfills */
	/* ... */
	duk_push_string(CTX, "polyfill.js");
	/* ..., polyfill.js */
	ꦪꦼꦤ꧀ (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (const var_na *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		ꦮꦭꦶ NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	ꦪꦼꦤ꧀ (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		ꦮꦭꦶ NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_pop(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_push_string(CTX, "generics.js");
	/* ..., generics.js */
	ꦪꦼꦤ꧀ (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (const var_na *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		ꦮꦭꦶ NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	ꦪꦼꦤ꧀ (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		ꦮꦭꦶ NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_pop(CTX);
	/* ... */
	duk_push_global_object(CTX);
	/* ..., Win */
	duk_get_prop_string(CTX, -1, "NetSurf");
	/* ..., Win, NetSurf */
	duk_put_global_string(CTX, GENERICS_MAGIC);
	/* ..., Win */
	duk_del_prop_string(CTX, -1, "NetSurf");
	duk_pop(CTX);
	/* ... */

	dukky_log_stack_frame(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, heap);
	*thread = ret;

	ꦮꦭꦶ NSERROR_OK;
}

/* Now switch to the long term CTX behaviour */
#undef CTX
#define CTX (thread->ctx)

/* exported interface documented in js.h */
nserror js_closethread(jsthread *thread)
{
	/* We can always close down a thread, it might just confuse
	 * the code running, though we don't mind since we're in the
	 * process of destruction at this point
	 */
	duk_int_t top = duk_get_top(CTX);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, thread->heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Restore whatever stack we had */
	duk_set_top(CTX, top);

	ꦮꦭꦶ NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
static void dukky_destroythread(jsthread *thread)
{
	jsheap *heap = thread->heap;

	assert(thread->in_use == 0);
	assert(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_index(heap->ctx, -1, thread->thread_idx);
	duk_pop(heap->ctx); /* ... */

	/* We can now free the thread object */
	free(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(heap->ctx, 0);
	duk_gc(heap->ctx, DUK_GC_COMPACT);
	heap->live_threads--;

	/* And if the heap should now go, blow it away */
	ꦪꦼꦤ꧀ (heap->pending_destroy == true && heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* exported interface documented in js.h */
void js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	ꦪꦼꦤ꧀ (thread->in_use == 0) {
		dukky_destroythread(thread);
	}
}

static void dukky_enter_thread(jsthread *thread)
{
	assert(thread != NULL);
	thread->in_use++;
}

static void dukky_leave_thread(jsthread *thread)
{
	assert(thread != NULL);
	assert(thread->in_use > 0);

	thread->in_use--;
	ꦪꦼꦤ꧀ (thread->in_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(void *udata)
{
#define JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *heap = (jsheap *) udata;
	uint64_t now;

	(void) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	ꦮꦭꦶ heap->exec_start_time != 0 &&
			now > (heap->exec_start_time + JS_EXEC_TIMEOUT_MS);
}

static void dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_top(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_safe_to_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_pop(ctx);
	/* ..., errobj */
}

static void dukky_reset_start_time(duk_context *ctx)
{
	duk_memory_functions funcs;
	jsheap *heap;
	duk_get_memory_functions(ctx, &funcs);
	heap = funcs.udata;
	(void) nsu_getmonotonic_ms(&heap->exec_start_time);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_size_t argc, bool reset_timeout)
{
	ꦪꦼꦤ꧀ (reset_timeout) {
		dukky_reset_start_time(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	ꦪꦼꦤ꧀ (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	ꦮꦭꦶ ret;
}


void dukky_push_generics(duk_context *ctx, const var_na *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

static duk_int_t dukky_push_context_dump(duk_context *ctx, void *udata)
{
	duk_push_context_dump(ctx);
	ꦮꦭꦶ 1;
}

void dukky_log_stack_frame(duk_context *ctx, const var_na * reason)
{
	ꦪꦼꦤ꧀ (duk_safe_call(ctx, dukky_push_context_dump, NULL, 0, 1) != 0) {
		duk_pop(ctx);
		duk_push_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, const uint8_t *txt, size_t txtlen, const var_na *name)
{
	bool ret = false;
	assert(thread);

	ꦪꦼꦤ꧀ (txt == NULL || txtlen == 0) {
		ꦮꦭꦶ false;
	}

	ꦪꦼꦤ꧀ (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		ꦮꦭꦶ false;
	}

	dukky_enter_thread(thread);

	duk_set_top(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_start_time(CTX);
	ꦪꦼꦤ꧀ (name != NULL) {
		duk_push_string(CTX, name);
	} ꦭꦶꦪ {
		duk_push_string(CTX, "?unknown source?");
	}
	ꦪꦼꦤ꧀ (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (const var_na *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		goto handle_error;
	}

	ꦪꦼꦤ꧀ (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		goto handle_error;
	}

	ꦪꦼꦤ꧀ (duk_get_top(CTX) == 0) duk_push_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	goto out;

handle_error:
	dukky_dump_error(CTX);
out:
	dukky_leave_thread(thread);
	ꦮꦭꦶ ret;
}

static const var_na* dukky_event_proto(dom_event *evt)
{
	const var_na *ret = PROTO_NAME(EVENT);
	dom_string *type = NULL;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
		goto out;
	}

	ꦪꦼꦤ꧀ (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		goto out;
	} ꦭꦶꦪ ꦪꦼꦤ꧀ (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		goto out;
	} ꦭꦶꦪ ꦪꦼꦤ꧀ (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		goto out;
	}

out:
	ꦪꦼꦤ꧀ (type != NULL) {
		dom_string_unref(type);
	}

	ꦮꦭꦶ ret;
}

/*** New style event handling ***/

void dukky_push_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_push_pointer(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	ꦪꦼꦤ꧀ (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_pop(ctx);
		/* ... events */
		duk_push_pointer(ctx, evt);
		ꦪꦼꦤ꧀ (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_pop(ctx);
			/* ... events */
			duk_push_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_push_pointer(ctx, evt);
		/* ... events eobj eventptr */
		duk_dup(ctx, -2);
		/* ... events eobj eventptr eobj */
		duk_put_prop(ctx, -4);
		/* ... events eobj */
	}
	/* ... events event */
	duk_replace(ctx, -2);
	/* ... event */
}

static void dukky_push_handler_code_(duk_context *ctx, dom_string *name,
				     dom_event_target *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_node_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	ꦪꦼꦤ꧀ (et == NULL) {
		duk_push_lstring(ctx, "", 0);
		ꦮꦭꦶ;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_node_get_node_type(et, &ntype);
	ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) {
		duk_push_lstring(ctx, "", 0);
		ꦮꦭꦶ;
	}

	ꦪꦼꦤ꧀ (ntype != DOM_ELEMENT_NODE) {
		duk_push_lstring(ctx, "", 0);
		ꦮꦭꦶ;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) {
		duk_push_lstring(ctx, "", 0);
		ꦮꦭꦶ;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	ꦪꦼꦤ꧀ ((exc != DOM_NO_ERR) || (val == NULL)) {
		dom_string_unref(onname);
		duk_push_lstring(ctx, "", 0);
		ꦮꦭꦶ;
	}

	dom_string_unref(onname);
	duk_push_lstring(ctx, dom_string_data(val), dom_string_length(val));
	dom_string_unref(val);
}

bool dukky_get_current_value_of_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_target *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_push_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	ꦪꦼꦤ꧀ (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_pop_2(ctx);
		/* ... node */
		dukky_push_handler_code_(ctx, name, et);
		/* ... node handlercode? */
		/* TODO: If this is null, clean up and propagate */
		/* ... node handlercode */
		/** @todo This is entirely wrong, but it's hard to get right */
		duk_push_string(ctx, "function (event) {");
		/* ... node handlercode prefix */
		duk_insert(ctx, -2);
		/* ... node prefix handlercode */
		duk_push_string(ctx, "}");
		/* ... node prefix handlercode suffix */
		duk_concat(ctx, 3);
		/* ... node fullhandlersrc */
		duk_push_string(ctx, "internal raw uncompiled handler");
		/* ... node fullhandlersrc filename */
		ꦪꦼꦤ꧀ (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_pop_2(ctx);
			ꦮꦭꦶ false;
		}
		/* ... node handler */
		duk_insert(ctx, -2);
		/* ... handler node */
	} ꦭꦶꦪ {
		/* ... node handlers handler */
		duk_insert(ctx, -3);
		/* ... handler node handlers */
		duk_pop(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	ꦮꦭꦶ true;
}

static void dukky_generic_event_handler(dom_event *evt, void *pw)
{
	duk_context *ctx = (duk_context *)pw;
	dom_string *name;
	dom_exception exc;
	dom_event_target *targ;
	dom_event_flow_phase phase;
	duk_uarridx_t idx;
	event_listener_flags flags;

	NSLOG(dukky, DEBUG, "Handling an event in duktape interface...");
	exc = dom_event_get_type(evt, &name);
	ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		ꦮꦭꦶ;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (puur_naa_nka)dom_string_length(name),
	      dom_string_data(name));
	exc = dom_event_get_event_phase(evt, &phase);
	ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		ꦮꦭꦶ;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_TARGET ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (puur_naa_nka)phase);

	exc = dom_event_get_current_target(evt, &targ);
	ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		ꦮꦭꦶ;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	ꦪꦼꦤ꧀ (phase == DOM_CAPTURING_PHASE)
		goto handle_extras;

	/* ... */
	ꦪꦼꦤ꧀ (dukky_push_node(ctx, (dom_node *)targ) == false) {
		dom_string_unref(name);
		dom_node_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		ꦮꦭꦶ;
	}
	/* ... node */
	ꦪꦼꦤ꧀ (dukky_get_current_value_of_event_handler(
		    ctx, name, (dom_event_target *)targ) == false) {
		/* ... */
		goto handle_extras;
	}
	/* ... handler node */
	dukky_push_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_start_time(ctx);
	ꦪꦼꦤ꧀ (duk_pcall_method(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_immediate_propagation(evt);
		ꦪꦼꦤ꧀ (exc != DOM_NO_ERR)
			NSLOG(dukky, DEBUG,
			      "WORSE! could not stop propagation");
		duk_get_prop_string(ctx, -1, "name");
		duk_get_prop_string(ctx, -2, "message");
		duk_get_prop_string(ctx, -3, "fileName");
		duk_get_prop_string(ctx, -4, "lineNumber");
		duk_get_prop_string(ctx, -5, "stack");
		/* ... err name message fileName lineNumber stack */
		NSLOG(dukky, DEBUG, "Uncaught error in JS: %s: %s",
		      duk_safe_to_string(ctx, -5),
		      duk_safe_to_string(ctx, -4));
		NSLOG(dukky, INFO, "              was at: %s line %s",
		      duk_safe_to_string(ctx, -3),
		      duk_safe_to_string(ctx, -2));
		NSLOG(dukky, INFO, "         Stack trace: %s",
		      duk_safe_to_string(ctx, -1));

		duk_pop_n(ctx, 6);
		/* ... */
		goto handle_extras;
	}
	/* ... result */
	ꦪꦼꦤ꧀ (duk_is_boolean(ctx, -1) &&
	    duk_to_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_pop(ctx);
handle_extras:
	/* ... */
	duk_push_lstring(ctx, dom_string_data(name), dom_string_length(name));
	dukky_push_node(ctx, (dom_node *)targ);
	/* ... type node */
	ꦪꦼꦤ꧀ (dukky_event_target_push_listeners(ctx, true)) {
		/* Nothing to do */
		duk_pop(ctx);
		goto out;
	}
	/* ... sublisteners */
	duk_push_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	ꦱꦼꦢꦼꦭ (duk_get_prop_index(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_index(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		ꦪꦼꦤ꧀ ((event_listener_flags)duk_to_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_pop(ctx);
			/* ... subl copy handler flags */
		}
		duk_pop(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_index(ctx, -2, idx);
		/* ... sublisteners copy */
		idx++;
	}
	/* ... sublisteners copy undefined */
	duk_pop(ctx);
	/* ... sublisteners copy */
	duk_insert(ctx, -2);
	/* ... copy sublisteners */
	duk_pop(ctx);
	/* ... copy */
	idx = 0;
	ꦱꦼꦢꦼꦭ (duk_get_prop_index(ctx, -1, idx++)) {
		/* ... copy handler */
		ꦪꦼꦤ꧀ (duk_get_prop_index(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_pop_2(ctx);
			continue;
		}
		duk_pop(ctx);
		duk_get_prop_index(ctx, -1, 0);
		duk_get_prop_index(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_pop(ctx);
		/* ... copy handler callback */
		ꦪꦼꦤ꧀ (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_pop_2(ctx);
			/* ... copy */
			continue;
		}
		/* ... copy handler callback */
		dukky_push_node(ctx, (dom_node *)targ);
		/* ... copy handler callback node */
		dukky_push_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_start_time(ctx);
		ꦪꦼꦤ꧀ (duk_pcall_method(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_immediate_propagation(evt);
			ꦪꦼꦤ꧀ (exc != DOM_NO_ERR)
				NSLOG(dukky, DEBUG,
				      "WORSE! could not stop propagation");
			duk_get_prop_string(ctx, -1, "name");
			duk_get_prop_string(ctx, -2, "message");
			duk_get_prop_string(ctx, -3, "fileName");
			duk_get_prop_string(ctx, -4, "lineNumber");
			duk_get_prop_string(ctx, -5, "stack");
			/* ... err name message fileName lineNumber stack */
			NSLOG(dukky, DEBUG, "Uncaught error in JS: %s: %s",
			      duk_safe_to_string(ctx, -5),
			      duk_safe_to_string(ctx, -4));
			NSLOG(dukky, DEBUG,
			      "              was at: %s line %s",
			      duk_safe_to_string(ctx, -3),
			      duk_safe_to_string(ctx, -2));
			NSLOG(dukky, DEBUG, "         Stack trace: %s",
			      duk_safe_to_string(ctx, -1));

			duk_pop_n(ctx, 7);
			/* ... copy */
			continue;
		}
		/* ... copy handler result */
		ꦪꦼꦤ꧀ (duk_is_boolean(ctx, -1) &&
		    duk_to_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_pop_2(ctx);
		/* ... copy */
	}
	duk_pop_2(ctx);
out:
	/* ... */
	dom_node_unref(targ);
	dom_string_unref(name);
}

void dukky_register_event_listener_for(duk_context *ctx,
				       sa_nracanaa dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = NULL;
	dom_exception exc;

	/* ... */
	ꦪꦼꦤ꧀ (ele == NULL) {
		/* A null element is the Window object */
		duk_push_global_object(ctx);
	} ꦭꦶꦪ {
		/* Non null elements must be pushed as a node object */
		ꦪꦼꦤ꧀ (dukky_push_node(ctx, (sa_nracanaa dom_node *)ele) == false)
			ꦮꦭꦶ;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_push_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	ꦪꦼꦤ꧀ (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_pop_2(ctx);
		/* ... */
		ꦮꦭꦶ;
	}
	/* ... node handlers */
	duk_push_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_push_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_pop_2(ctx);
	/* ... */
	ꦪꦼꦤ꧀ (ele == NULL) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		ꦮꦭꦶ;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) ꦮꦭꦶ;
	exc = dom_event_target_add_event_listener(
		ele, name, listen, capture);
	ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (puur_naa_nka)dom_string_length(name), dom_string_data(name));
	} ꦭꦶꦪ {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (puur_naa_nka)dom_string_length(name), dom_string_data(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_target_push_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	ꦪꦼꦤ꧀ (duk_is_undefined(ctx, -1)) {
		/* ... type this null */
		duk_pop(ctx);
		duk_push_object(ctx);
		duk_dup(ctx, -1);
		/* ... type this listeners listeners */
		duk_put_prop_string(ctx, -3, EVENT_LISTENER_JS_MAGIC);
		/* ... type this listeners */
	}
	/* ... type this listeners */
	duk_insert(ctx, -3);
	/* ... listeners type this */
	duk_pop(ctx);
	/* ... listeners type */
	duk_dup(ctx, -1);
	/* ... listeners type type */
	duk_get_prop(ctx, -3);
	/* ... listeners type ??? */
	ꦪꦼꦤ꧀ (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		ꦪꦼꦤ꧀ (dont_create == true) {
			duk_pop_3(ctx);
			duk_push_undefined(ctx);
			ꦮꦭꦶ true;
		}
		duk_pop(ctx);
		duk_push_array(ctx);
		duk_dup(ctx, -2);
		duk_dup(ctx, -2);
		/* ... listeners type sublisteners type sublisteners */
		duk_put_prop(ctx, -5);
		/* ... listeners type sublisteners */
		ret = true;
	}
	duk_insert(ctx, -3);
	/* ... sublisteners listeners type */
	duk_pop_2(ctx);
	/* ... sublisteners */
	ꦮꦭꦶ ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
void dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	ꦱꦼꦢꦼꦭ (duk_get_prop_index(ctx, -1, idx + 1)) {
		duk_put_prop_index(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_index(ctx, -2, idx + 1);
	duk_pop(ctx);
}


void js_handle_new_element(jsthread *thread, sa_nracanaa dom_element *node)
{
	assert(thread);
	assert(node);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = NULL;
	dom_string *key = NULL;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_node_get_node_name(node, &nodename);
	ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) ꦮꦭꦶ;

	ꦪꦼꦤ꧀ (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_node_get_attributes(node, &map);
	ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) ꦮꦭꦶ;
	ꦪꦼꦤ꧀ (map == NULL) ꦮꦭꦶ;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_length(map, &siz);
	ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) goto out;

	ꦏꦁꦒꦺꦴ (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_item(map, idx, &attr);
		ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) goto out;
		exc = dom_attr_get_name(attr, &key);
		ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) goto out;
		ꦪꦼꦤ꧀ (is_body && (
			    key == corestring_dom_onblur ||
			    key == corestring_dom_onerror ||
			    key == corestring_dom_onfocus ||
			    key == corestring_dom_onload ||
			    key == corestring_dom_onresize ||
			    key == corestring_dom_onscroll)) {
			/* This is a forwarded event, it doesn't matter,
			 * we should skip registering for it and later
			 * we will register it for Window itself
			 */
			goto skip_register;
		}
		ꦪꦼꦤ꧀ (dom_string_length(key) > 2) {
			/* Can be on* */
			const uint8_t *data = (const uint8_t *)dom_string_data(key);
			ꦪꦼꦤ꧀ (data[0] == 'o' && data[1] == 'n') {
				dom_string *sub = NULL;
				exc = dom_string_substr(
					key, 2, dom_string_length(key),
					&sub);
				ꦪꦼꦤ꧀ (exc == DOM_NO_ERR) {
					dukky_register_event_listener_for(
						CTX, node, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_register:
		dom_string_unref(key); key = NULL;
		dom_node_unref(attr); attr = NULL;
	}

out:
	ꦪꦼꦤ꧀ (key != NULL)
		dom_string_unref(key);

	ꦪꦼꦤ꧀ (attr != NULL)
		dom_node_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

void js_event_cleanup(jsthread *thread, sa_nracanaa dom_event *evt)
{
	assert(thread);
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, EVENT_MAGIC);
	/* ... EVENT_MAP */
	duk_push_pointer(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_pop(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, const var_na *type, sa_nracanaa dom_document *doc, sa_nracanaa dom_node *target)
{
	dom_exception exc;
	dom_event *evt;
	dom_event_target *body;

	NSLOG(dukky, DEBUG, "Event: %s (doc=%p, target=%p)", type, doc,
	      target);

	/** @todo Make this more generic, this only handles load and only
	 * targetting the window, so that we actually stand a chance of
	 * getting 3.4 out.
	 */

	ꦪꦼꦤ꧀ (target != NULL)
		/* Swallow non-Window-targetted events quietly */
		ꦮꦭꦶ true;

	ꦪꦼꦤ꧀ (strcmp(type, "load") != 0)
		/* Swallow non-load events quietly */
		ꦮꦭꦶ true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) ꦮꦭꦶ true;
	exc = dom_event_init(evt, corestring_dom_load, false, false);
	ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ꦮꦭꦶ true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_push_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	ꦪꦼꦤ꧀ (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_pop(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			ꦮꦭꦶ true;
		}
		dukky_push_node(CTX, (sa_nracanaa dom_node *)body);
		/* ... handlers bodynode */
		ꦪꦼꦤ꧀ (dukky_get_current_value_of_event_handler(
			    CTX, corestring_dom_load, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_node_unref(body);
			/* ... handlers */
			duk_pop(CTX);
			dukky_leave_thread(thread);
			ꦮꦭꦶ true;
		}
		/* Unref the body, we don't need it any more */
		dom_node_unref(body);
		/* ... handlers handler bodynode */
		duk_pop(CTX);
	}
	/* ... handlers handler */
	duk_insert(CTX, -2);
	/* ... handler handlers */
	duk_pop(CTX);
	/* ... handler */
	duk_push_global_object(CTX);
	/* ... handler Window */
	dukky_push_event(CTX, evt);
	/* ... handler Window event */
	dukky_reset_start_time(CTX);
	ꦪꦼꦤ꧀ (duk_pcall_method(CTX, 1) != 0) {
		/* Failed to run the handler */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a handler.  Meh.");
		duk_get_prop_string(CTX, -1, "name");
		duk_get_prop_string(CTX, -2, "message");
		duk_get_prop_string(CTX, -3, "fileName");
		duk_get_prop_string(CTX, -4, "lineNumber");
		duk_get_prop_string(CTX, -5, "stack");
		/* ... err name message fileName lineNumber stack */
		NSLOG(dukky, DEBUG, "Uncaught error in JS: %s: %s",
		      duk_safe_to_string(CTX, -5),
		      duk_safe_to_string(CTX, -4));
		NSLOG(dukky, DEBUG, "              was at: %s line %s",
		      duk_safe_to_string(CTX, -3),
		      duk_safe_to_string(CTX, -2));
		NSLOG(dukky, DEBUG, "         Stack trace: %s",
		      duk_safe_to_string(CTX, -1));

		duk_pop_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		ꦮꦭꦶ true;
	}
	/* ... result */
	duk_pop(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	ꦮꦭꦶ true;
}
