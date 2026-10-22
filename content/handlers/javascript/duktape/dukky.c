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

#ضمن <stdint.h>
#ضمن <nsutils/time.h>

#ضمن "netsurf/inttypes.h"
#ضمن "utils/utils.h"
#ضمن "utils/nsoption.h"
#ضمن "utils/log.h"
#ضمن "utils/corestrings.h"
#ضمن "content/content.h"

#ضمن "javascript/js.h"
#ضمن "javascript/content.h"

#ضمن "duktape/binding.h"
#ضمن "duktape/generics.js.inc"
#ضمن "duktape/polyfill.js.inc"

#ضمن "duktape.h"
#ضمن "dukky.h"

#ضمن <dom/dom.h>

#define EVENT_MAGIC MAGIC(EVENT_MAP)
#define HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#define HANDLER_MAGIC MAGIC(HANDLER_MAP)
#define EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#define GENERICS_MAGIC MAGIC(GENERICS_TABLE)
#define THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
بنية jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t next_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	unsigned صحيح live_threads; /**< number of live threads */
	uint64_t exec_start_time;
};

/**
 * dukky javascript thread
 */
بنية jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	unsigned صحيح in_use; /**< The number of times this thread is in use */
	jsheap *heap; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

ساكن duk_ret_t dukky_populate_object(duk_context *ctx, فراغ *udata)
{
	/* ... obj args protoname nargs */
	صحيح nargs = duk_get_int(ctx, -1);
	duk_pop(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	אם (duk_is_undefined(ctx, -1)) {
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
	החזר 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, ثابت حرف *name, صحيح args)
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
	אם ((ret = duk_safe_call(ctx, dukky_populate_object, عدم, args + 3, 1))
	    != DUK_EXEC_SUCCESS)
		החזר ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	החזר DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_push_node_stacked(duk_context *ctx)
{
	صحيح top_at_fail = duk_get_top(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, NODE_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	אם (duk_is_undefined(ctx, -1)) {
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
		אם (duk_safe_call(ctx, dukky_populate_object, عدم, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_set_top(ctx, top_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			החזר false;
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
	אם (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		ثابت حرف * what = duk_safe_to_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_pop(ctx);
	}
	החזר true;
}

#define SET_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

ساكن فراغ dukky_html_element_class_from_tag_type(dom_html_element_type type,
		ثابت حرف **html_class, size_t *html_class_len)
{
	بدل(type) {
	حالة DOM_HTML_ELEMENT_TYPE_HTML:
		SET_HTML_CLASS(HTML)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_HEAD:
		SET_HTML_CLASS(HEAD)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_META:
		SET_HTML_CLASS(META)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_BASE:
		SET_HTML_CLASS(BASE)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_TITLE:
		SET_HTML_CLASS(TITLE)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_BODY:
		SET_HTML_CLASS(BODY)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_DIV:
		SET_HTML_CLASS(DIV)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_FORM:
		SET_HTML_CLASS(FORM)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_LINK:
		SET_HTML_CLASS(LINK)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_BUTTON:
		SET_HTML_CLASS(BUTTON)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_INPUT:
		SET_HTML_CLASS(INPUT)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		SET_HTML_CLASS(TEXTAREA)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		SET_HTML_CLASS(OPTGROUP)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_OPTION:
		SET_HTML_CLASS(OPTION)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_SELECT:
		SET_HTML_CLASS(SELECT)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_HR:
		SET_HTML_CLASS(HR)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_DL:
		SET_HTML_CLASS(DLIST)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_DIR:
		SET_HTML_CLASS(DIRECTORY)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_MENU:
		SET_HTML_CLASS(MENU)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_FIELDSET:
		SET_HTML_CLASS(FIELDSET)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_LEGEND:
		SET_HTML_CLASS(LEGEND)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_P:
		SET_HTML_CLASS(PARAGRAPH)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_H1:
	حالة DOM_HTML_ELEMENT_TYPE_H2:
	حالة DOM_HTML_ELEMENT_TYPE_H3:
	حالة DOM_HTML_ELEMENT_TYPE_H4:
	حالة DOM_HTML_ELEMENT_TYPE_H5:
	حالة DOM_HTML_ELEMENT_TYPE_H6:
		SET_HTML_CLASS(HEADING)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	حالة DOM_HTML_ELEMENT_TYPE_Q:
		SET_HTML_CLASS(QUOTE)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_PRE:
		SET_HTML_CLASS(PRE)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_BR:
		SET_HTML_CLASS(BR)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_LABEL:
		SET_HTML_CLASS(LABEL)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_UL:
		SET_HTML_CLASS(ULIST)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_OL:
		SET_HTML_CLASS(OLIST)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_LI:
		SET_HTML_CLASS(LI)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_FONT:
		SET_HTML_CLASS(FONT)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_DEL:
	حالة DOM_HTML_ELEMENT_TYPE_INS:
		SET_HTML_CLASS(MOD)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_A:
		SET_HTML_CLASS(ANCHOR)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_BASEFONT:
		SET_HTML_CLASS(BASEFONT)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_IMG:
		SET_HTML_CLASS(IMAGE)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_OBJECT:
		SET_HTML_CLASS(OBJECT)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_PARAM:
		SET_HTML_CLASS(PARAM)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_APPLET:
		SET_HTML_CLASS(APPLET)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_MAP:
		SET_HTML_CLASS(MAP)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_AREA:
		SET_HTML_CLASS(AREA)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_SCRIPT:
		SET_HTML_CLASS(SCRIPT)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_CAPTION:
		SET_HTML_CLASS(TABLECAPTION)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_TD:
	حالة DOM_HTML_ELEMENT_TYPE_TH:
		SET_HTML_CLASS(TABLECELL)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_COL:
	حالة DOM_HTML_ELEMENT_TYPE_COLGROUP:
		SET_HTML_CLASS(TABLECOL)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_THEAD:
	حالة DOM_HTML_ELEMENT_TYPE_TBODY:
	حالة DOM_HTML_ELEMENT_TYPE_TFOOT:
		SET_HTML_CLASS(TABLESECTION)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_TABLE:
		SET_HTML_CLASS(TABLE)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_TR:
		SET_HTML_CLASS(TABLEROW)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_STYLE:
		SET_HTML_CLASS(STYLE)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_FRAMESET:
		SET_HTML_CLASS(FRAMESET)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_FRAME:
		SET_HTML_CLASS(FRAME)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_IFRAME:
		SET_HTML_CLASS(IFRAME)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_ISINDEX:
		SET_HTML_CLASS(ISINDEX)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_CANVAS:
		SET_HTML_CLASS(CANVAS)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE__COUNT:
		أكد(type != DOM_HTML_ELEMENT_TYPE__COUNT);
		fallthrough;
	حالة DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		SET_HTML_CLASS(UNKNOWN)
		اكسر;
	افتراضي:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		اكسر;
	}
	החזר;
}

#undef SET_HTML_CLASS

ساكن فراغ
dukky_push_node_klass(duk_context *ctx, بنية dom_node *node)
{
	dom_node_type nodetype;
	dom_exception err;

	err = dom_node_get_node_type(node, &nodetype);
	אם (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_push_string(ctx, PROTO_NAME(NODE));
		החזר;
	}

	بدل(nodetype) {
	حالة DOM_ELEMENT_NODE: {
		dom_string *namespace;
		dom_html_element_type type;
		ثابت حرف *html_class;
		size_t html_class_len;
		err = dom_node_get_namespace(node, &namespace);
		אם (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_push_string(ctx, PROTO_NAME(ELEMENT));
			اكسر;
		}
		אם (namespace == عدم) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_push_string(ctx, PROTO_NAME(ELEMENT));
			اكسر;
		}

		אם (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_push_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			اكسر;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_tag_type(node, &type);
		אם (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_tag_type(type,
				&html_class, &html_class_len);

		duk_push_lstring(ctx, html_class, html_class_len);
		اكسر;
	}
	حالة DOM_TEXT_NODE:
		duk_push_string(ctx, PROTO_NAME(TEXT));
		اكسر;
	حالة DOM_COMMENT_NODE:
		duk_push_string(ctx, PROTO_NAME(COMMENT));
		اكسر;
	حالة DOM_DOCUMENT_NODE:
		duk_push_string(ctx, PROTO_NAME(DOCUMENT));
		اكسر;
	حالة DOM_ATTRIBUTE_NODE:
	حالة DOM_PROCESSING_INSTRUCTION_NODE:
	حالة DOM_DOCUMENT_TYPE_NODE:
	حالة DOM_DOCUMENT_FRAGMENT_NODE:
	حالة DOM_NOTATION_NODE:
	حالة DOM_ENTITY_REFERENCE_NODE:
	حالة DOM_ENTITY_NODE:
	حالة DOM_CDATA_SECTION_NODE:
	افتراضي:
		/* Oh bum, just node then */
		duk_push_string(ctx, PROTO_NAME(NODE));
	}
}

duk_bool_t
dukky_push_node(duk_context *ctx, بنية dom_node *node)
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
	אם (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_insert(ctx, -2);
		/* ... node nodes */
		duk_pop(ctx);
		/* ... node */
		אם (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			ثابت حرف * what = duk_safe_to_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_pop(ctx);
		}
		החזר true;
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
	החזר dukky_push_node_stacked(ctx);
}

ساكن duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	החזר duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

فراغ
dukky_inject_not_ctr(duk_context *ctx, صحيح idx, ثابت حرف *name)
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
	החזר;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

ساكن فراغ *dukky_alloc_function(فراغ *udata, duk_size_t size)
{
	אם (size == 0)
		החזר عدم;

	החזר malloc(size);
}

ساكن فراغ *dukky_realloc_function(فراغ *udata, فراغ *ptr, duk_size_t size)
{
	אם (ptr == عدم && size == 0)
		החזר عدم;

	אם (size == 0) {
		حرر(ptr);
		החזר عدم;
	}

	החזר realloc(ptr, size);
}


ساكن فراغ dukky_free_function(فراغ *udata, فراغ *ptr)
{
	אם (ptr != عدم)
		حرر(ptr);
}

/* exported interface documented in js.h */
فراغ js_initialise(فراغ)
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
فراغ js_finalise(فراغ)
{
	/* NADA for now */
}


/* exported interface documented in js.h */
nserror
js_newheap(صحيح timeout, jsheap **heap)
{
	duk_context *ctx;
	jsheap *ret = calloc(1, حجم(*ret));
	*heap = عدم;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	אם (ret == عدم) החזר NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_heap(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		عدم);
	אם (ret->ctx == عدم) { حرر(ret); החזר NSERROR_NOMEM; }
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
	החזר NSERROR_OK;
}


ساكن فراغ dukky_destroyheap(jsheap *heap)
{
	أكد(heap->pending_destroy == true);
	أكد(heap->live_threads == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_heap(heap->ctx);
	حرر(heap);
}

/* exported interface documented in js.h */
فراغ js_destroyheap(jsheap *heap)
{
	heap->pending_destroy = true;
	אם (heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* Just for here, the CTX is in ret, not thread */
#define CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *heap, فراغ *win_priv, فراغ *doc_priv, jsthread **thread)
{
	jsthread *ret;
	أكد(heap != عدم);
	أكد(heap->pending_destroy == false);

	ret = calloc(1, حجم (*ret));
	אם (ret == عدم) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		החזר NSERROR_NOMEM;
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
	duk_push_pointer(CTX, عدم);
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
	אם (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ثابت حرف *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		החזר NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	אם (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		החזר NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_pop(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_push_string(CTX, "generics.js");
	/* ..., generics.js */
	אם (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ثابت حرف *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		החזר NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	אם (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		החזר NSERROR_INIT_FAILED;
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

	החזר NSERROR_OK;
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

	החזר NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
ساكن فراغ dukky_destroythread(jsthread *thread)
{
	jsheap *heap = thread->heap;

	أكد(thread->in_use == 0);
	أكد(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_index(heap->ctx, -1, thread->thread_idx);
	duk_pop(heap->ctx); /* ... */

	/* We can now free the thread object */
	حرر(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(heap->ctx, 0);
	duk_gc(heap->ctx, DUK_GC_COMPACT);
	heap->live_threads--;

	/* And if the heap should now go, blow it away */
	אם (heap->pending_destroy == true && heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* exported interface documented in js.h */
فراغ js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	אם (thread->in_use == 0) {
		dukky_destroythread(thread);
	}
}

ساكن فراغ dukky_enter_thread(jsthread *thread)
{
	أكد(thread != عدم);
	thread->in_use++;
}

ساكن فراغ dukky_leave_thread(jsthread *thread)
{
	أكد(thread != عدم);
	أكد(thread->in_use > 0);

	thread->in_use--;
	אם (thread->in_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(فراغ *udata)
{
#define JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *heap = (jsheap *) udata;
	uint64_t now;

	(فراغ) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	החזר heap->exec_start_time != 0 &&
			now > (heap->exec_start_time + JS_EXEC_TIMEOUT_MS);
}

ساكن فراغ dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_top(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_safe_to_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_pop(ctx);
	/* ..., errobj */
}

ساكن فراغ dukky_reset_start_time(duk_context *ctx)
{
	duk_memory_functions funcs;
	jsheap *heap;
	duk_get_memory_functions(ctx, &funcs);
	heap = funcs.udata;
	(فراغ) nsu_getmonotonic_ms(&heap->exec_start_time);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_size_t argc, bool reset_timeout)
{
	אם (reset_timeout) {
		dukky_reset_start_time(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	אם (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	החזר ret;
}


فراغ dukky_push_generics(duk_context *ctx, ثابت حرف *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

ساكن duk_int_t dukky_push_context_dump(duk_context *ctx, فراغ *udata)
{
	duk_push_context_dump(ctx);
	החזר 1;
}

فراغ dukky_log_stack_frame(duk_context *ctx, ثابت حرف * reason)
{
	אם (duk_safe_call(ctx, dukky_push_context_dump, عدم, 0, 1) != 0) {
		duk_pop(ctx);
		duk_push_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, ثابت uint8_t *txt, size_t txtlen, ثابت حرف *name)
{
	bool ret = false;
	أكد(thread);

	אם (txt == عدم || txtlen == 0) {
		החזר false;
	}

	אם (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		החזר false;
	}

	dukky_enter_thread(thread);

	duk_set_top(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_start_time(CTX);
	אם (name != عدم) {
		duk_push_string(CTX, name);
	} אחרת {
		duk_push_string(CTX, "?unknown source?");
	}
	אם (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (ثابت حرف *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		اذهب handle_error;
	}

	אם (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		اذهب handle_error;
	}

	אם (duk_get_top(CTX) == 0) duk_push_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	اذهب out;

handle_error:
	dukky_dump_error(CTX);
out:
	dukky_leave_thread(thread);
	החזר ret;
}

ساكن ثابت حرف* dukky_event_proto(dom_event *evt)
{
	ثابت حرف *ret = PROTO_NAME(EVENT);
	dom_string *type = عدم;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	אם (err != DOM_NO_ERR) {
		اذهب out;
	}

	אם (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		اذهب out;
	} אחרת אם (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		اذهب out;
	} אחרת אם (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		اذهب out;
	}

out:
	אם (type != عدم) {
		dom_string_unref(type);
	}

	החזר ret;
}

/*** New style event handling ***/

فراغ dukky_push_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_push_pointer(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	אם (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_pop(ctx);
		/* ... events */
		duk_push_pointer(ctx, evt);
		אם (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
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

ساكن فراغ dukky_push_handler_code_(duk_context *ctx, dom_string *name,
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
	אם (et == عدم) {
		duk_push_lstring(ctx, "", 0);
		החזר;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_node_get_node_type(et, &ntype);
	אם (exc != DOM_NO_ERR) {
		duk_push_lstring(ctx, "", 0);
		החזר;
	}

	אם (ntype != DOM_ELEMENT_NODE) {
		duk_push_lstring(ctx, "", 0);
		החזר;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	אם (exc != DOM_NO_ERR) {
		duk_push_lstring(ctx, "", 0);
		החזר;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	אם ((exc != DOM_NO_ERR) || (val == عدم)) {
		dom_string_unref(onname);
		duk_push_lstring(ctx, "", 0);
		החזר;
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
	אם (duk_is_undefined(ctx, -1)) {
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
		אם (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_pop_2(ctx);
			החזר false;
		}
		/* ... node handler */
		duk_insert(ctx, -2);
		/* ... handler node */
	} אחרת {
		/* ... node handlers handler */
		duk_insert(ctx, -3);
		/* ... handler node handlers */
		duk_pop(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	החזר true;
}

ساكن فراغ dukky_generic_event_handler(dom_event *evt, فراغ *pw)
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
	אם (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		החזר;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (صحيح)dom_string_length(name),
	      dom_string_data(name));
	exc = dom_event_get_event_phase(evt, &phase);
	אם (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		החזר;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_TARGET ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (صحيح)phase);

	exc = dom_event_get_current_target(evt, &targ);
	אם (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		החזר;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	אם (phase == DOM_CAPTURING_PHASE)
		اذهب handle_extras;

	/* ... */
	אם (dukky_push_node(ctx, (dom_node *)targ) == false) {
		dom_string_unref(name);
		dom_node_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		החזר;
	}
	/* ... node */
	אם (dukky_get_current_value_of_event_handler(
		    ctx, name, (dom_event_target *)targ) == false) {
		/* ... */
		اذهب handle_extras;
	}
	/* ... handler node */
	dukky_push_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_start_time(ctx);
	אם (duk_pcall_method(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_immediate_propagation(evt);
		אם (exc != DOM_NO_ERR)
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
		اذهب handle_extras;
	}
	/* ... result */
	אם (duk_is_boolean(ctx, -1) &&
	    duk_to_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_pop(ctx);
handle_extras:
	/* ... */
	duk_push_lstring(ctx, dom_string_data(name), dom_string_length(name));
	dukky_push_node(ctx, (dom_node *)targ);
	/* ... type node */
	אם (dukky_event_target_push_listeners(ctx, true)) {
		/* Nothing to do */
		duk_pop(ctx);
		اذهب out;
	}
	/* ... sublisteners */
	duk_push_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	כלעוד (duk_get_prop_index(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_index(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		אם ((event_listener_flags)duk_to_int(ctx, -1) & ELF_ONCE) {
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
	כלעוד (duk_get_prop_index(ctx, -1, idx++)) {
		/* ... copy handler */
		אם (duk_get_prop_index(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_pop_2(ctx);
			استمر;
		}
		duk_pop(ctx);
		duk_get_prop_index(ctx, -1, 0);
		duk_get_prop_index(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_pop(ctx);
		/* ... copy handler callback */
		אם (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_pop_2(ctx);
			/* ... copy */
			استمر;
		}
		/* ... copy handler callback */
		dukky_push_node(ctx, (dom_node *)targ);
		/* ... copy handler callback node */
		dukky_push_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_start_time(ctx);
		אם (duk_pcall_method(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_immediate_propagation(evt);
			אם (exc != DOM_NO_ERR)
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
			استمر;
		}
		/* ... copy handler result */
		אם (duk_is_boolean(ctx, -1) &&
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

فراغ dukky_register_event_listener_for(duk_context *ctx,
				       بنية dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = عدم;
	dom_exception exc;

	/* ... */
	אם (ele == عدم) {
		/* A null element is the Window object */
		duk_push_global_object(ctx);
	} אחרת {
		/* Non null elements must be pushed as a node object */
		אם (dukky_push_node(ctx, (بنية dom_node *)ele) == false)
			החזר;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_push_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	אם (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_pop_2(ctx);
		/* ... */
		החזר;
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
	אם (ele == عدم) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		החזר;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	אם (exc != DOM_NO_ERR) החזר;
	exc = dom_event_target_add_event_listener(
		ele, name, listen, capture);
	אם (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (صحيح)dom_string_length(name), dom_string_data(name));
	} אחרת {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (صحيح)dom_string_length(name), dom_string_data(name));
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
	אם (duk_is_undefined(ctx, -1)) {
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
	אם (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		אם (dont_create == true) {
			duk_pop_3(ctx);
			duk_push_undefined(ctx);
			החזר true;
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
	החזר ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
فراغ dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	כלעוד (duk_get_prop_index(ctx, -1, idx + 1)) {
		duk_put_prop_index(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_index(ctx, -2, idx + 1);
	duk_pop(ctx);
}


فراغ js_handle_new_element(jsthread *thread, بنية dom_element *node)
{
	أكد(thread);
	أكد(node);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = عدم;
	dom_string *key = عدم;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_node_get_node_name(node, &nodename);
	אם (exc != DOM_NO_ERR) החזר;

	אם (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_node_get_attributes(node, &map);
	אם (exc != DOM_NO_ERR) החזר;
	אם (map == عدم) החזר;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_length(map, &siz);
	אם (exc != DOM_NO_ERR) اذهب out;

	עבור (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_item(map, idx, &attr);
		אם (exc != DOM_NO_ERR) اذهب out;
		exc = dom_attr_get_name(attr, &key);
		אם (exc != DOM_NO_ERR) اذهب out;
		אם (is_body && (
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
			اذهب skip_register;
		}
		אם (dom_string_length(key) > 2) {
			/* Can be on* */
			ثابت uint8_t *data = (ثابت uint8_t *)dom_string_data(key);
			אם (data[0] == 'o' && data[1] == 'n') {
				dom_string *sub = عدم;
				exc = dom_string_substr(
					key, 2, dom_string_length(key),
					&sub);
				אם (exc == DOM_NO_ERR) {
					dukky_register_event_listener_for(
						CTX, node, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_register:
		dom_string_unref(key); key = عدم;
		dom_node_unref(attr); attr = عدم;
	}

out:
	אם (key != عدم)
		dom_string_unref(key);

	אם (attr != عدم)
		dom_node_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

فراغ js_event_cleanup(jsthread *thread, بنية dom_event *evt)
{
	أكد(thread);
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

bool js_fire_event(jsthread *thread, ثابت حرف *type, بنية dom_document *doc, بنية dom_node *target)
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

	אם (target != عدم)
		/* Swallow non-Window-targetted events quietly */
		החזר true;

	אם (strcmp(type, "load") != 0)
		/* Swallow non-load events quietly */
		החזר true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	אם (exc != DOM_NO_ERR) החזר true;
	exc = dom_event_init(evt, corestring_dom_load, false, false);
	אם (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		החזר true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_push_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	אם (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_pop(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		אם (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			החזר true;
		}
		dukky_push_node(CTX, (بنية dom_node *)body);
		/* ... handlers bodynode */
		אם (dukky_get_current_value_of_event_handler(
			    CTX, corestring_dom_load, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_node_unref(body);
			/* ... handlers */
			duk_pop(CTX);
			dukky_leave_thread(thread);
			החזר true;
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
	אם (duk_pcall_method(CTX, 1) != 0) {
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
		החזר true;
	}
	/* ... result */
	duk_pop(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	החזר true;
}
