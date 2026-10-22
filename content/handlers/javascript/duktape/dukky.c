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

#شامل <stdint.h>
#شامل <nsutils/time.h>

#شامل "netsurf/inttypes.h"
#شامل "utils/utils.h"
#شامل "utils/nsoption.h"
#شامل "utils/log.h"
#شامل "utils/corestrings.h"
#شامل "content/content.h"

#شامل "javascript/js.h"
#شامل "javascript/content.h"

#شامل "duktape/binding.h"
#شامل "duktape/generics.js.inc"
#شامل "duktape/polyfill.js.inc"

#شامل "duktape.h"
#شامل "dukky.h"

#شامل <dom/dom.h>

#define EVENT_MAGIC MAGIC(EVENT_MAP)
#define HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#define HANDLER_MAGIC MAGIC(HANDLER_MAP)
#define EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#define GENERICS_MAGIC MAGIC(GENERICS_TABLE)
#define THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
ساختار jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t next_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	unsigned صحیح live_threads; /**< number of live threads */
	uint64_t exec_start_time;
};

/**
 * dukky javascript thread
 */
ساختار jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	unsigned صحیح in_use; /**< The number of times this thread is in use */
	jsheap *heap; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

ایستا duk_ret_t dukky_populate_object(duk_context *ctx, تهی *udata)
{
	/* ... obj args protoname nargs */
	صحیح nargs = duk_get_int(ctx, -1);
	duk_برداشت(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	اگر (duk_is_undefined(ctx, -1)) {
		NSLOG(dukky, WARNING,
		      "Unable to find dukky prototype for `%s` - falling back to HTMLUnknownElement",
		      duk_get_string(ctx, -3) + 2 /* Skip the two unprintables */);
		duk_برداشت(ctx);
		duk_افزودن_string(ctx, PROTO_NAME(HTMLUNKNOWNELEMENT));
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
	duk_درج(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_برداشت_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	بازگشت 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, ثابت نویسه *name, صحیح args)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, args);
	/* ... args */
	duk_افزودن_object(ctx);
	/* ... args obj */
	duk_افزودن_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
	/* ... args obj */
	duk_افزودن_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
	/* ... args obj */
	duk_درج(ctx, -(args+1));
	/* ... obj args */
	duk_افزودن_string(ctx, name);
	/* ... obj args name */
	duk_افزودن_int(ctx, args);
	/* ... obj args name nargs */
	اگر ((ret = duk_safe_call(ctx, dukky_populate_object, پوچ, args + 3, 1))
	    != DUK_EXEC_SUCCESS)
		بازگشت ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	بازگشت DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_افزودن_گره_stacked(duk_context *ctx)
{
	صحیح بالاترین_at_fail = duk_get_بالاترین(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, گره_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	اگر (duk_is_undefined(ctx, -1)) {
		/* ... nodeptr klass nodes undefined */
		duk_برداشت(ctx);
		/* ... nodeptr klass nodes */
		duk_افزودن_object(ctx);
		/* ... nodeptr klass nodes obj */
		duk_افزودن_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_افزودن_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr klass */
		duk_افزودن_int(ctx, 1);
		/* ... nodeptr klass nodes obj nodeptr klass 1 */
		اگر (duk_safe_call(ctx, dukky_populate_object, پوچ, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_set_بالاترین(ctx, بالاترین_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			بازگشت false;
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
	duk_درج(ctx, -4);
	/* ... node nodeptr klass nodes */
	duk_برداشت_3(ctx);
	/* ... node */
	اگر (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		ثابت نویسه * what = duk_safe_to_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_برداشت(ctx);
	}
	بازگشت true;
}

#define SET_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

ایستا تهی dukky_html_element_class_from_tag_type(dom_html_element_type type,
		ثابت نویسه **html_class, size_t *html_class_len)
{
	انتخاب(type) {
	حالت DOM_HTML_ELEMENT_TYPE_HTML:
		SET_HTML_CLASS(HTML)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_HEAD:
		SET_HTML_CLASS(HEAD)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_META:
		SET_HTML_CLASS(META)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_BASE:
		SET_HTML_CLASS(BASE)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_TITLE:
		SET_HTML_CLASS(TITLE)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_BODY:
		SET_HTML_CLASS(BODY)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_DIV:
		SET_HTML_CLASS(DIV)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_FORM:
		SET_HTML_CLASS(FORM)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_LINK:
		SET_HTML_CLASS(LINK)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_BUTTON:
		SET_HTML_CLASS(BUTTON)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_درون_داد:
		SET_HTML_CLASS(درون_داد_2)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		SET_HTML_CLASS(TEXTAREA)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		SET_HTML_CLASS(OPTGROUP)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_OPTION:
		SET_HTML_CLASS(OPTION)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_SELECT:
		SET_HTML_CLASS(SELECT)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_HR:
		SET_HTML_CLASS(HR)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_DL:
		SET_HTML_CLASS(DLIST)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_DIR:
		SET_HTML_CLASS(DIRECTORY)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_MENU:
		SET_HTML_CLASS(MENU)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_FIELDSET:
		SET_HTML_CLASS(FIELDSET)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_LEGEND:
		SET_HTML_CLASS(LEGEND)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_P:
		SET_HTML_CLASS(PARAGRAPH)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_H1:
	حالت DOM_HTML_ELEMENT_TYPE_H2:
	حالت DOM_HTML_ELEMENT_TYPE_H3:
	حالت DOM_HTML_ELEMENT_TYPE_H4:
	حالت DOM_HTML_ELEMENT_TYPE_H5:
	حالت DOM_HTML_ELEMENT_TYPE_H6:
		SET_HTML_CLASS(HEADING)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	حالت DOM_HTML_ELEMENT_TYPE_Q:
		SET_HTML_CLASS(QUOTE)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_PRE:
		SET_HTML_CLASS(PRE)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_BR:
		SET_HTML_CLASS(BR)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_LABEL:
		SET_HTML_CLASS(LABEL)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_UL:
		SET_HTML_CLASS(ULIST)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_OL:
		SET_HTML_CLASS(OLIST)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_LI:
		SET_HTML_CLASS(LI)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_FONT:
		SET_HTML_CLASS(FONT)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_DEL:
	حالت DOM_HTML_ELEMENT_TYPE_INS:
		SET_HTML_CLASS(MOD)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_A:
		SET_HTML_CLASS(ANCHOR)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_BASEFONT:
		SET_HTML_CLASS(BASEFONT)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_IMG:
		SET_HTML_CLASS(IMAGE)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_OBJECT:
		SET_HTML_CLASS(OBJECT)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_PARAM:
		SET_HTML_CLASS(PARAM)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_APPLET:
		SET_HTML_CLASS(APPLET)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_MAP:
		SET_HTML_CLASS(MAP)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_AREA:
		SET_HTML_CLASS(AREA)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_SCRIPT:
		SET_HTML_CLASS(SCRIPT)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_CAPTION:
		SET_HTML_CLASS(TABLECAPTION)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_TD:
	حالت DOM_HTML_ELEMENT_TYPE_TH:
		SET_HTML_CLASS(TABLECELL)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_COL:
	حالت DOM_HTML_ELEMENT_TYPE_COLGROUP:
		SET_HTML_CLASS(TABLECOL)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_THEAD:
	حالت DOM_HTML_ELEMENT_TYPE_TBODY:
	حالت DOM_HTML_ELEMENT_TYPE_TFOOT:
		SET_HTML_CLASS(TABLESECTION)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_TABLE:
		SET_HTML_CLASS(TABLE)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_TR:
		SET_HTML_CLASS(TABLEROW)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_STYLE:
		SET_HTML_CLASS(STYLE)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_FRAMESET:
		SET_HTML_CLASS(FRAMESET)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_FRAME:
		SET_HTML_CLASS(FRAME)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_IFRAME:
		SET_HTML_CLASS(IFRAME)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_ISINDEX:
		SET_HTML_CLASS(ISINDEX)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_CANVAS:
		SET_HTML_CLASS(CANVAS)
		بشکن;
	حالت DOM_HTML_ELEMENT_TYPE_شمار:
		أكد(type != DOM_HTML_ELEMENT_TYPE_شمار);
		fallthrough;
	حالت DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		SET_HTML_CLASS(UNKNOWN)
		بشکن;
	پیشفرض:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		بشکن;
	}
	بازگشت;
}

#undef SET_HTML_CLASS

ایستا تهی
dukky_افزودن_گره_klass(duk_context *ctx, ساختار dom_گره *گره)
{
	dom_گره_type nodetype;
	dom_exception err;

	err = dom_گره_get_گره_type(گره, &nodetype);
	اگر (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_افزودن_string(ctx, PROTO_NAME(گره_2));
		بازگشت;
	}

	انتخاب(nodetype) {
	حالت DOM_ELEMENT_گره: {
		dom_string *namespace;
		dom_html_element_type type;
		ثابت نویسه *html_class;
		size_t html_class_len;
		err = dom_گره_get_namespace(گره, &namespace);
		اگر (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_افزودن_string(ctx, PROTO_NAME(ELEMENT));
			بشکن;
		}
		اگر (namespace == پوچ) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_افزودن_string(ctx, PROTO_NAME(ELEMENT));
			بشکن;
		}

		اگر (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_افزودن_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			بشکن;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_tag_type(گره, &type);
		اگر (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_tag_type(type,
				&html_class, &html_class_len);

		duk_افزودن_lstring(ctx, html_class, html_class_len);
		بشکن;
	}
	حالت DOM_TEXT_گره:
		duk_افزودن_string(ctx, PROTO_NAME(TEXT));
		بشکن;
	حالت DOM_COMMENT_گره:
		duk_افزودن_string(ctx, PROTO_NAME(COMMENT));
		بشکن;
	حالت DOM_DOCUMENT_گره:
		duk_افزودن_string(ctx, PROTO_NAME(DOCUMENT));
		بشکن;
	حالت DOM_ATTRIBUTE_گره:
	حالت DOM_PROCESSING_INSTRUCTION_گره:
	حالت DOM_DOCUMENT_TYPE_گره:
	حالت DOM_DOCUMENT_FRAGMENT_گره:
	حالت DOM_NOTATION_گره:
	حالت DOM_ENTITY_REFERENCE_گره:
	حالت DOM_ENTITY_گره:
	حالت DOM_CDATA_SECTION_گره:
	پیشفرض:
		/* Oh bum, just node then */
		duk_افزودن_string(ctx, PROTO_NAME(گره_2));
	}
}

duk_bool_t
dukky_افزودن_گره(duk_context *ctx, ساختار dom_گره *گره)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", گره);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, گره_MAGIC);
	/* ... nodes */
	duk_افزودن_pointer(ctx, گره);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	اگر (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_درج(ctx, -2);
		/* ... node nodes */
		duk_برداشت(ctx);
		/* ... node */
		اگر (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			ثابت نویسه * what = duk_safe_to_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_برداشت(ctx);
		}
		بازگشت true;
	}
	/* ... nodes undefined */
	duk_برداشت_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_افزودن_pointer(ctx, گره);
	/* ... nodeptr */
	dukky_افزودن_گره_klass(ctx, گره);
	/* ... nodeptr klass */
	بازگشت dukky_افزودن_گره_stacked(ctx);
}

ایستا duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	بازگشت duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

تهی
dukky_inject_not_ctr(duk_context *ctx, صحیح idx, ثابت نویسه *name)
{
	/* ... p[idx] ... proto */
	duk_افزودن_c_function(ctx, dukky_bad_constructor, 0);
	/* ... p[idx] ... proto cons */
	duk_درج(ctx, -2);
	/* ... p[idx] ... cons proto */
	duk_put_prop_string(ctx, -2, "prototype");
	/* ... p[idx] ... cons[proto] */
	duk_put_prop_string(ctx, idx, name);
	/* ... p ... */
	بازگشت;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

ایستا تهی *dukky_alloc_function(تهی *udata, duk_size_t size)
{
	اگر (size == 0)
		بازگشت پوچ;

	بازگشت malloc(size);
}

ایستا تهی *dukky_realloc_function(تهی *udata, تهی *ptr, duk_size_t size)
{
	اگر (ptr == پوچ && size == 0)
		بازگشت پوچ;

	اگر (size == 0) {
		آزاد(ptr);
		بازگشت پوچ;
	}

	بازگشت realloc(ptr, size);
}


ایستا تهی dukky_free_function(تهی *udata, تهی *ptr)
{
	اگر (ptr != پوچ)
		آزاد(ptr);
}

/* exported interface documented in js.h */
تهی js_initialise(تهی)
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
تهی js_finalise(تهی)
{
	/* NADA for now */
}


/* exported interface documented in js.h */
nserror
js_newheap(صحیح timeout, jsheap **heap)
{
	duk_context *ctx;
	jsheap *ret = calloc(1, اندازه(*ret));
	*heap = پوچ;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	اگر (ret == پوچ) بازگشت NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_heap(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		پوچ);
	اگر (ret->ctx == پوچ) { آزاد(ret); بازگشت NSERROR_NOMEM; }
	/* Create the prototype stuffs */
	duk_افزودن_global_object(ctx);
	duk_افزودن_boolean(ctx, true);
	duk_put_prop_string(ctx, -2, "protos");
	duk_put_global_string(ctx, PROTO_MAGIC);
	/* Create prototypes here */
	dukky_create_prototypes(ctx);
	/* Now create the thread map */
	duk_افزودن_object(ctx);
	duk_put_global_string(ctx, THREAD_MAP);

	*heap = ret;
	بازگشت NSERROR_OK;
}


ایستا تهی dukky_destroyheap(jsheap *heap)
{
	أكد(heap->pending_destroy == true);
	أكد(heap->live_threads == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_heap(heap->ctx);
	آزاد(heap);
}

/* exported interface documented in js.h */
تهی js_destroyheap(jsheap *heap)
{
	heap->pending_destroy = true;
	اگر (heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* Just for here, the CTX is in ret, not thread */
#define CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *heap, تهی *win_priv, تهی *doc_priv, jsthread **thread)
{
	jsthread *ret;
	أكد(heap != پوچ);
	أكد(heap->pending_destroy == false);

	ret = calloc(1, اندازه (*ret));
	اگر (ret == پوچ) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		بازگشت NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_افزودن_thread(heap->ctx); /* ... threads thread */
	ret->heap = heap;
	ret->ctx = duk_require_context(heap->ctx, -1);
	ret->thread_idx = heap->next_thread++;
	duk_put_prop_index(heap->ctx, -2, ret->thread_idx);
	heap->live_threads++;
	duk_برداشت(heap->ctx); /* ... */
	duk_افزودن_int(CTX, 0);
	duk_افزودن_int(CTX, 1);
	duk_افزودن_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_افزودن_pointer(CTX, win_priv);
	duk_افزودن_pointer(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(WINDOW), 2);
	duk_افزودن_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_set_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_افزودن_object(CTX);
	duk_افزودن_pointer(CTX, پوچ);
	duk_افزودن_null(CTX);
	duk_put_prop(CTX, -3);
	duk_put_global_string(CTX, گره_MAGIC);

	/* And now the event mapping table */
	duk_افزودن_object(CTX);
	duk_put_global_string(CTX, EVENT_MAGIC);

	/* Now load the polyfills */
	/* ... */
	duk_افزودن_string(CTX, "polyfill.js");
	/* ..., polyfill.js */
	اگر (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ثابت نویسه *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		بازگشت NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	اگر (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		بازگشت NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_برداشت(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_افزودن_string(CTX, "generics.js");
	/* ..., generics.js */
	اگر (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ثابت نویسه *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		بازگشت NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	اگر (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		بازگشت NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_برداشت(CTX);
	/* ... */
	duk_افزودن_global_object(CTX);
	/* ..., Win */
	duk_get_prop_string(CTX, -1, "NetSurf");
	/* ..., Win, NetSurf */
	duk_put_global_string(CTX, GENERICS_MAGIC);
	/* ..., Win */
	duk_del_prop_string(CTX, -1, "NetSurf");
	duk_برداشت(CTX);
	/* ... */

	dukky_log_پشته_frame(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, heap);
	*thread = ret;

	بازگشت NSERROR_OK;
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
	duk_int_t بالاترین = duk_get_بالاترین(CTX);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, thread->heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Restore whatever stack we had */
	duk_set_بالاترین(CTX, بالاترین);

	بازگشت NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
ایستا تهی dukky_destroythread(jsthread *thread)
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
	duk_برداشت(heap->ctx); /* ... */

	/* We can now free the thread object */
	آزاد(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(heap->ctx, 0);
	duk_gc(heap->ctx, DUK_GC_COMPACT);
	heap->live_threads--;

	/* And if the heap should now go, blow it away */
	اگر (heap->pending_destroy == true && heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* exported interface documented in js.h */
تهی js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	اگر (thread->in_use == 0) {
		dukky_destroythread(thread);
	}
}

ایستا تهی dukky_enter_thread(jsthread *thread)
{
	أكد(thread != پوچ);
	thread->in_use++;
}

ایستا تهی dukky_leave_thread(jsthread *thread)
{
	أكد(thread != پوچ);
	أكد(thread->in_use > 0);

	thread->in_use--;
	اگر (thread->in_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(تهی *udata)
{
#define JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *heap = (jsheap *) udata;
	uint64_t now;

	(تهی) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	بازگشت heap->exec_start_time != 0 &&
			now > (heap->exec_start_time + JS_EXEC_TIMEOUT_MS);
}

ایستا تهی dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_بالاترین(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_safe_to_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_برداشت(ctx);
	/* ..., errobj */
}

ایستا تهی dukky_reset_start_time(duk_context *ctx)
{
	duk_memory_functions funcs;
	jsheap *heap;
	duk_get_memory_functions(ctx, &funcs);
	heap = funcs.udata;
	(تهی) nsu_getmonotonic_ms(&heap->exec_start_time);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_size_t argc, bool reset_timeout)
{
	اگر (reset_timeout) {
		dukky_reset_start_time(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	اگر (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	بازگشت ret;
}


تهی dukky_افزودن_generics(duk_context *ctx, ثابت نویسه *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

ایستا duk_int_t dukky_افزودن_context_dump(duk_context *ctx, تهی *udata)
{
	duk_افزودن_context_dump(ctx);
	بازگشت 1;
}

تهی dukky_log_پشته_frame(duk_context *ctx, ثابت نویسه * reason)
{
	اگر (duk_safe_call(ctx, dukky_افزودن_context_dump, پوچ, 0, 1) != 0) {
		duk_برداشت(ctx);
		duk_افزودن_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_safe_to_string(ctx, -1));
	duk_برداشت(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, ثابت uint8_t *txt, size_t txtlen, ثابت نویسه *name)
{
	bool ret = false;
	أكد(thread);

	اگر (txt == پوچ || txtlen == 0) {
		بازگشت false;
	}

	اگر (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		بازگشت false;
	}

	dukky_enter_thread(thread);

	duk_set_بالاترین(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_start_time(CTX);
	اگر (name != پوچ) {
		duk_افزودن_string(CTX, name);
	} وگرنه {
		duk_افزودن_string(CTX, "?unknown source?");
	}
	اگر (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (ثابت نویسه *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		برو handle_error;
	}

	اگر (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		برو handle_error;
	}

	اگر (duk_get_بالاترین(CTX) == 0) duk_افزودن_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	برو out;

handle_error:
	dukky_dump_error(CTX);
out:
	dukky_leave_thread(thread);
	بازگشت ret;
}

ایستا ثابت نویسه* dukky_event_proto(dom_event *evt)
{
	ثابت نویسه *ret = PROTO_NAME(EVENT);
	dom_string *type = پوچ;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	اگر (err != DOM_NO_ERR) {
		برو out;
	}

	اگر (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		برو out;
	} وگرنه اگر (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		برو out;
	} وگرنه اگر (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		برو out;
	}

out:
	اگر (type != پوچ) {
		dom_string_unref(type);
	}

	بازگشت ret;
}

/*** New style event handling ***/

تهی dukky_افزودن_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_افزودن_pointer(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	اگر (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_برداشت(ctx);
		/* ... events */
		duk_افزودن_pointer(ctx, evt);
		اگر (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_برداشت(ctx);
			/* ... events */
			duk_افزودن_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_افزودن_pointer(ctx, evt);
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

ایستا تهی dukky_افزودن_handler_code(duk_context *ctx, dom_string *name,
				     dom_event_target *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_گره_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	اگر (et == پوچ) {
		duk_افزودن_lstring(ctx, "", 0);
		بازگشت;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_گره_get_گره_type(et, &ntype);
	اگر (exc != DOM_NO_ERR) {
		duk_افزودن_lstring(ctx, "", 0);
		بازگشت;
	}

	اگر (ntype != DOM_ELEMENT_گره) {
		duk_افزودن_lstring(ctx, "", 0);
		بازگشت;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	اگر (exc != DOM_NO_ERR) {
		duk_افزودن_lstring(ctx, "", 0);
		بازگشت;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	اگر ((exc != DOM_NO_ERR) || (val == پوچ)) {
		dom_string_unref(onname);
		duk_افزودن_lstring(ctx, "", 0);
		بازگشت;
	}

	dom_string_unref(onname);
	duk_افزودن_lstring(ctx, dom_string_data(val), dom_string_length(val));
	dom_string_unref(val);
}

bool dukky_get_current_مقدار_of_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_target *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_افزودن_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	اگر (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_برداشت_2(ctx);
		/* ... node */
		dukky_افزودن_handler_code(ctx, name, et);
		/* ... node handlercode? */
		/* TODO: If this is null, clean up and propagate */
		/* ... node handlercode */
		/** @todo This is entirely wrong, but it's hard to get right */
		duk_افزودن_string(ctx, "function (event) {");
		/* ... node handlercode prefix */
		duk_درج(ctx, -2);
		/* ... node prefix handlercode */
		duk_افزودن_string(ctx, "}");
		/* ... node prefix handlercode suffix */
		duk_concat(ctx, 3);
		/* ... node fullhandlersrc */
		duk_افزودن_string(ctx, "internal raw uncompiled handler");
		/* ... node fullhandlersrc filename */
		اگر (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_برداشت_2(ctx);
			بازگشت false;
		}
		/* ... node handler */
		duk_درج(ctx, -2);
		/* ... handler node */
	} وگرنه {
		/* ... node handlers handler */
		duk_درج(ctx, -3);
		/* ... handler node handlers */
		duk_برداشت(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	بازگشت true;
}

ایستا تهی dukky_generic_event_handler(dom_event *evt, تهی *pw)
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
	اگر (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		بازگشت;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (صحیح)dom_string_length(name),
	      dom_string_data(name));
	exc = dom_event_get_event_phase(evt, &phase);
	اگر (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		بازگشت;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_TARGET ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (صحیح)phase);

	exc = dom_event_get_current_target(evt, &targ);
	اگر (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		بازگشت;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	اگر (phase == DOM_CAPTURING_PHASE)
		برو handle_extras;

	/* ... */
	اگر (dukky_افزودن_گره(ctx, (dom_گره *)targ) == false) {
		dom_string_unref(name);
		dom_گره_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		بازگشت;
	}
	/* ... node */
	اگر (dukky_get_current_مقدار_of_event_handler(
		    ctx, name, (dom_event_target *)targ) == false) {
		/* ... */
		برو handle_extras;
	}
	/* ... handler node */
	dukky_افزودن_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_start_time(ctx);
	اگر (duk_pcall_method(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_immediate_propagation(evt);
		اگر (exc != DOM_NO_ERR)
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

		duk_برداشت_n(ctx, 6);
		/* ... */
		برو handle_extras;
	}
	/* ... result */
	اگر (duk_is_boolean(ctx, -1) &&
	    duk_to_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_برداشت(ctx);
handle_extras:
	/* ... */
	duk_افزودن_lstring(ctx, dom_string_data(name), dom_string_length(name));
	dukky_افزودن_گره(ctx, (dom_گره *)targ);
	/* ... type node */
	اگر (dukky_event_target_افزودن_listeners(ctx, true)) {
		/* Nothing to do */
		duk_برداشت(ctx);
		برو out;
	}
	/* ... sublisteners */
	duk_افزودن_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	وقتی (duk_get_prop_index(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_index(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		اگر ((event_listener_flags)duk_to_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_برداشت(ctx);
			/* ... subl copy handler flags */
		}
		duk_برداشت(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_index(ctx, -2, idx);
		/* ... sublisteners copy */
		idx++;
	}
	/* ... sublisteners copy undefined */
	duk_برداشت(ctx);
	/* ... sublisteners copy */
	duk_درج(ctx, -2);
	/* ... copy sublisteners */
	duk_برداشت(ctx);
	/* ... copy */
	idx = 0;
	وقتی (duk_get_prop_index(ctx, -1, idx++)) {
		/* ... copy handler */
		اگر (duk_get_prop_index(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_برداشت_2(ctx);
			ادامه;
		}
		duk_برداشت(ctx);
		duk_get_prop_index(ctx, -1, 0);
		duk_get_prop_index(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_برداشت(ctx);
		/* ... copy handler callback */
		اگر (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_برداشت_2(ctx);
			/* ... copy */
			ادامه;
		}
		/* ... copy handler callback */
		dukky_افزودن_گره(ctx, (dom_گره *)targ);
		/* ... copy handler callback node */
		dukky_افزودن_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_start_time(ctx);
		اگر (duk_pcall_method(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_immediate_propagation(evt);
			اگر (exc != DOM_NO_ERR)
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

			duk_برداشت_n(ctx, 7);
			/* ... copy */
			ادامه;
		}
		/* ... copy handler result */
		اگر (duk_is_boolean(ctx, -1) &&
		    duk_to_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_برداشت_2(ctx);
		/* ... copy */
	}
	duk_برداشت_2(ctx);
out:
	/* ... */
	dom_گره_unref(targ);
	dom_string_unref(name);
}

تهی dukky_register_event_listener_for(duk_context *ctx,
				       ساختار dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = پوچ;
	dom_exception exc;

	/* ... */
	اگر (ele == پوچ) {
		/* A null element is the Window object */
		duk_افزودن_global_object(ctx);
	} وگرنه {
		/* Non null elements must be pushed as a node object */
		اگر (dukky_افزودن_گره(ctx, (ساختار dom_گره *)ele) == false)
			بازگشت;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_افزودن_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	اگر (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_برداشت_2(ctx);
		/* ... */
		بازگشت;
	}
	/* ... node handlers */
	duk_افزودن_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_افزودن_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_برداشت_2(ctx);
	/* ... */
	اگر (ele == پوچ) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		بازگشت;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	اگر (exc != DOM_NO_ERR) بازگشت;
	exc = dom_event_target_add_event_listener(
		ele, name, listen, capture);
	اگر (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (صحیح)dom_string_length(name), dom_string_data(name));
	} وگرنه {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (صحیح)dom_string_length(name), dom_string_data(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_target_افزودن_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	اگر (duk_is_undefined(ctx, -1)) {
		/* ... type this null */
		duk_برداشت(ctx);
		duk_افزودن_object(ctx);
		duk_dup(ctx, -1);
		/* ... type this listeners listeners */
		duk_put_prop_string(ctx, -3, EVENT_LISTENER_JS_MAGIC);
		/* ... type this listeners */
	}
	/* ... type this listeners */
	duk_درج(ctx, -3);
	/* ... listeners type this */
	duk_برداشت(ctx);
	/* ... listeners type */
	duk_dup(ctx, -1);
	/* ... listeners type type */
	duk_get_prop(ctx, -3);
	/* ... listeners type ??? */
	اگر (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		اگر (dont_create == true) {
			duk_برداشت_3(ctx);
			duk_افزودن_undefined(ctx);
			بازگشت true;
		}
		duk_برداشت(ctx);
		duk_افزودن_array(ctx);
		duk_dup(ctx, -2);
		duk_dup(ctx, -2);
		/* ... listeners type sublisteners type sublisteners */
		duk_put_prop(ctx, -5);
		/* ... listeners type sublisteners */
		ret = true;
	}
	duk_درج(ctx, -3);
	/* ... sublisteners listeners type */
	duk_برداشت_2(ctx);
	/* ... sublisteners */
	بازگشت ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
تهی dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	وقتی (duk_get_prop_index(ctx, -1, idx + 1)) {
		duk_put_prop_index(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_index(ctx, -2, idx + 1);
	duk_برداشت(ctx);
}


تهی js_handle_new_element(jsthread *thread, ساختار dom_element *گره)
{
	أكد(thread);
	أكد(گره);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = پوچ;
	dom_string *کلید = پوچ;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_گره_get_گره_name(گره, &nodename);
	اگر (exc != DOM_NO_ERR) بازگشت;

	اگر (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_گره_get_attributes(گره, &map);
	اگر (exc != DOM_NO_ERR) بازگشت;
	اگر (map == پوچ) بازگشت;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_length(map, &siz);
	اگر (exc != DOM_NO_ERR) برو out;

	برای (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_item(map, idx, &attr);
		اگر (exc != DOM_NO_ERR) برو out;
		exc = dom_attr_get_name(attr, &کلید);
		اگر (exc != DOM_NO_ERR) برو out;
		اگر (is_body && (
			    کلید == corestring_dom_onblur ||
			    کلید == corestring_dom_onerror ||
			    کلید == corestring_dom_onfocus ||
			    کلید == corestring_dom_onload ||
			    کلید == corestring_dom_onresize ||
			    کلید == corestring_dom_onscroll)) {
			/* This is a forwarded event, it doesn't matter,
			 * we should skip registering for it and later
			 * we will register it for Window itself
			 */
			برو skip_register;
		}
		اگر (dom_string_length(کلید) > 2) {
			/* Can be on* */
			ثابت uint8_t *data = (ثابت uint8_t *)dom_string_data(کلید);
			اگر (data[0] == 'o' && data[1] == 'n') {
				dom_string *sub = پوچ;
				exc = dom_string_substr(
					کلید, 2, dom_string_length(کلید),
					&sub);
				اگر (exc == DOM_NO_ERR) {
					dukky_register_event_listener_for(
						CTX, گره, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_register:
		dom_string_unref(کلید); کلید = پوچ;
		dom_گره_unref(attr); attr = پوچ;
	}

out:
	اگر (کلید != پوچ)
		dom_string_unref(کلید);

	اگر (attr != پوچ)
		dom_گره_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

تهی js_event_cleanup(jsthread *thread, ساختار dom_event *evt)
{
	أكد(thread);
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, EVENT_MAGIC);
	/* ... EVENT_MAP */
	duk_افزودن_pointer(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_برداشت(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, ثابت نویسه *type, ساختار dom_document *doc, ساختار dom_گره *target)
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

	اگر (target != پوچ)
		/* Swallow non-Window-targetted events quietly */
		بازگشت true;

	اگر (strcmp(type, "load") != 0)
		/* Swallow non-load events quietly */
		بازگشت true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	اگر (exc != DOM_NO_ERR) بازگشت true;
	exc = dom_event_init(evt, corestring_dom_load, false, false);
	اگر (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		بازگشت true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_افزودن_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	اگر (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_برداشت(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		اگر (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			بازگشت true;
		}
		dukky_افزودن_گره(CTX, (ساختار dom_گره *)body);
		/* ... handlers bodynode */
		اگر (dukky_get_current_مقدار_of_event_handler(
			    CTX, corestring_dom_load, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_گره_unref(body);
			/* ... handlers */
			duk_برداشت(CTX);
			dukky_leave_thread(thread);
			بازگشت true;
		}
		/* Unref the body, we don't need it any more */
		dom_گره_unref(body);
		/* ... handlers handler bodynode */
		duk_برداشت(CTX);
	}
	/* ... handlers handler */
	duk_درج(CTX, -2);
	/* ... handler handlers */
	duk_برداشت(CTX);
	/* ... handler */
	duk_افزودن_global_object(CTX);
	/* ... handler Window */
	dukky_افزودن_event(CTX, evt);
	/* ... handler Window event */
	dukky_reset_start_time(CTX);
	اگر (duk_pcall_method(CTX, 1) != 0) {
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

		duk_برداشت_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		بازگشت true;
	}
	/* ... result */
	duk_برداشت(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	بازگشت true;
}
