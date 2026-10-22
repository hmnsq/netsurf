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

#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <nsutils/time.h>

#அந்தர்பாவ "netsurf/inttypes.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "content/content.h"

#அந்தர்பாவ "javascript/js.h"
#அந்தர்பாவ "javascript/content.h"

#அந்தர்பாவ "duktape/binding.h"
#அந்தர்பாவ "duktape/generics.js.inc"
#அந்தர்பாவ "duktape/polyfill.js.inc"

#அந்தர்பாவ "duktape.h"
#அந்தர்பாவ "dukky.h"

#அந்தர்பாவ <dom/dom.h>

#define EVENT_MAGIC MAGIC(EVENT_MAP)
#define HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#define HANDLER_MAGIC MAGIC(HANDLER_MAP)
#define EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#define GENERICS_MAGIC MAGIC(GENERICS_TABLE)
#define THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
கட்டமைப்பு jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t next_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	unsigned முழு live_threads; /**< number of live threads */
	uint64_t exec_start_time;
};

/**
 * dukky javascript thread
 */
கட்டமைப்பு jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	unsigned முழு in_use; /**< The number of times this thread is in use */
	jsheap *heap; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

நிலையான duk_ret_t dukky_populate_object(duk_context *ctx, வெறுமை *udata)
{
	/* ... obj args protoname nargs */
	முழு nargs = duk_get_int(ctx, -1);
	duk_எடு(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	எனில் (duk_is_undefined(ctx, -1)) {
		NSLOG(dukky, WARNING,
		      "Unable to find dukky prototype for `%s` - falling back to HTMLUnknownElement",
		      duk_get_string(ctx, -3) + 2 /* Skip the two unprintables */);
		duk_எடு(ctx);
		duk_தள்ளு_string(ctx, PROTO_NAME(HTMLUNKNOWNELEMENT));
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
	duk_செருகு(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_எடு_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	திரும்பு 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, மாறிலி எழுத்து *name, முழு args)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, args);
	/* ... args */
	duk_தள்ளு_object(ctx);
	/* ... args obj */
	duk_தள்ளு_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
	/* ... args obj */
	duk_தள்ளு_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
	/* ... args obj */
	duk_செருகு(ctx, -(args+1));
	/* ... obj args */
	duk_தள்ளு_string(ctx, name);
	/* ... obj args name */
	duk_தள்ளு_int(ctx, args);
	/* ... obj args name nargs */
	எனில் ((ret = duk_safe_call(ctx, dukky_populate_object, வெற்று, args + 3, 1))
	    != DUK_EXEC_SUCCESS)
		திரும்பு ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	திரும்பு DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_தள்ளு_முனை_stacked(duk_context *ctx)
{
	முழு உச்சி_at_fail = duk_get_உச்சி(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, முனை_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	எனில் (duk_is_undefined(ctx, -1)) {
		/* ... nodeptr klass nodes undefined */
		duk_எடு(ctx);
		/* ... nodeptr klass nodes */
		duk_தள்ளு_object(ctx);
		/* ... nodeptr klass nodes obj */
		duk_தள்ளு_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_தள்ளு_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr klass */
		duk_தள்ளு_int(ctx, 1);
		/* ... nodeptr klass nodes obj nodeptr klass 1 */
		எனில் (duk_safe_call(ctx, dukky_populate_object, வெற்று, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_set_உச்சி(ctx, உச்சி_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			திரும்பு false;
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
	duk_செருகு(ctx, -4);
	/* ... node nodeptr klass nodes */
	duk_எடு_3(ctx);
	/* ... node */
	எனில் (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		மாறிலி எழுத்து * what = duk_safe_to_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_எடு(ctx);
	}
	திரும்பு true;
}

#define SET_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

நிலையான வெறுமை dukky_html_element_class_from_tag_type(dom_html_element_type type,
		மாறிலி எழுத்து **html_class, size_t *html_class_len)
{
	தேர்வு(type) {
	நிலை DOM_HTML_ELEMENT_TYPE_HTML:
		SET_HTML_CLASS(HTML)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_HEAD:
		SET_HTML_CLASS(HEAD)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_META:
		SET_HTML_CLASS(META)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_BASE:
		SET_HTML_CLASS(BASE)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_TITLE:
		SET_HTML_CLASS(TITLE)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_BODY:
		SET_HTML_CLASS(BODY)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_DIV:
		SET_HTML_CLASS(DIV)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_FORM:
		SET_HTML_CLASS(FORM)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_LINK:
		SET_HTML_CLASS(LINK)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_BUTTON:
		SET_HTML_CLASS(BUTTON)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_உள்ளீடு:
		SET_HTML_CLASS(உள்ளீடு_2)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		SET_HTML_CLASS(TEXTAREA)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		SET_HTML_CLASS(OPTGROUP)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_OPTION:
		SET_HTML_CLASS(OPTION)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_SELECT:
		SET_HTML_CLASS(SELECT)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_HR:
		SET_HTML_CLASS(HR)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_DL:
		SET_HTML_CLASS(DLIST)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_DIR:
		SET_HTML_CLASS(DIRECTORY)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_MENU:
		SET_HTML_CLASS(MENU)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_FIELDSET:
		SET_HTML_CLASS(FIELDSET)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_LEGEND:
		SET_HTML_CLASS(LEGEND)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_P:
		SET_HTML_CLASS(PARAGRAPH)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_H1:
	நிலை DOM_HTML_ELEMENT_TYPE_H2:
	நிலை DOM_HTML_ELEMENT_TYPE_H3:
	நிலை DOM_HTML_ELEMENT_TYPE_H4:
	நிலை DOM_HTML_ELEMENT_TYPE_H5:
	நிலை DOM_HTML_ELEMENT_TYPE_H6:
		SET_HTML_CLASS(HEADING)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	நிலை DOM_HTML_ELEMENT_TYPE_Q:
		SET_HTML_CLASS(QUOTE)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_PRE:
		SET_HTML_CLASS(PRE)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_BR:
		SET_HTML_CLASS(BR)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_LABEL:
		SET_HTML_CLASS(LABEL)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_UL:
		SET_HTML_CLASS(ULIST)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_OL:
		SET_HTML_CLASS(OLIST)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_LI:
		SET_HTML_CLASS(LI)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_FONT:
		SET_HTML_CLASS(FONT)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_DEL:
	நிலை DOM_HTML_ELEMENT_TYPE_INS:
		SET_HTML_CLASS(MOD)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_A:
		SET_HTML_CLASS(ANCHOR)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_BASEFONT:
		SET_HTML_CLASS(BASEFONT)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_IMG:
		SET_HTML_CLASS(IMAGE)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_OBJECT:
		SET_HTML_CLASS(OBJECT)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_PARAM:
		SET_HTML_CLASS(PARAM)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_APPLET:
		SET_HTML_CLASS(APPLET)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_MAP:
		SET_HTML_CLASS(MAP)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_AREA:
		SET_HTML_CLASS(AREA)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_SCRIPT:
		SET_HTML_CLASS(SCRIPT)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_CAPTION:
		SET_HTML_CLASS(TABLECAPTION)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_TD:
	நிலை DOM_HTML_ELEMENT_TYPE_TH:
		SET_HTML_CLASS(TABLECELL)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_COL:
	நிலை DOM_HTML_ELEMENT_TYPE_COLGROUP:
		SET_HTML_CLASS(TABLECOL)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_THEAD:
	நிலை DOM_HTML_ELEMENT_TYPE_TBODY:
	நிலை DOM_HTML_ELEMENT_TYPE_TFOOT:
		SET_HTML_CLASS(TABLESECTION)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_TABLE:
		SET_HTML_CLASS(TABLE)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_TR:
		SET_HTML_CLASS(TABLEROW)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_STYLE:
		SET_HTML_CLASS(STYLE)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_FRAMESET:
		SET_HTML_CLASS(FRAMESET)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_FRAME:
		SET_HTML_CLASS(FRAME)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_IFRAME:
		SET_HTML_CLASS(IFRAME)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_ISINDEX:
		SET_HTML_CLASS(ISINDEX)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_CANVAS:
		SET_HTML_CLASS(CANVAS)
		நிறுத்து;
	நிலை DOM_HTML_ELEMENT_TYPE_மொத்தம்:
		ப்ரதிஜ்ஞா(type != DOM_HTML_ELEMENT_TYPE_மொத்தம்);
		fallthrough;
	நிலை DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		SET_HTML_CLASS(UNKNOWN)
		நிறுத்து;
	இயல்பு:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		நிறுத்து;
	}
	திரும்பு;
}

#undef SET_HTML_CLASS

நிலையான வெறுமை
dukky_தள்ளு_முனை_klass(duk_context *ctx, கட்டமைப்பு dom_முனை *முனை)
{
	dom_முனை_type nodetype;
	dom_exception err;

	err = dom_முனை_get_முனை_type(முனை, &nodetype);
	எனில் (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_தள்ளு_string(ctx, PROTO_NAME(முனை_2));
		திரும்பு;
	}

	தேர்வு(nodetype) {
	நிலை DOM_ELEMENT_முனை: {
		dom_string *namespace;
		dom_html_element_type type;
		மாறிலி எழுத்து *html_class;
		size_t html_class_len;
		err = dom_முனை_get_namespace(முனை, &namespace);
		எனில் (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_தள்ளு_string(ctx, PROTO_NAME(ELEMENT));
			நிறுத்து;
		}
		எனில் (namespace == வெற்று) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_தள்ளு_string(ctx, PROTO_NAME(ELEMENT));
			நிறுத்து;
		}

		எனில் (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_தள்ளு_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			நிறுத்து;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_tag_type(முனை, &type);
		எனில் (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_tag_type(type,
				&html_class, &html_class_len);

		duk_தள்ளு_lstring(ctx, html_class, html_class_len);
		நிறுத்து;
	}
	நிலை DOM_TEXT_முனை:
		duk_தள்ளு_string(ctx, PROTO_NAME(TEXT));
		நிறுத்து;
	நிலை DOM_COMMENT_முனை:
		duk_தள்ளு_string(ctx, PROTO_NAME(COMMENT));
		நிறுத்து;
	நிலை DOM_DOCUMENT_முனை:
		duk_தள்ளு_string(ctx, PROTO_NAME(DOCUMENT));
		நிறுத்து;
	நிலை DOM_ATTRIBUTE_முனை:
	நிலை DOM_PROCESSING_INSTRUCTION_முனை:
	நிலை DOM_DOCUMENT_TYPE_முனை:
	நிலை DOM_DOCUMENT_FRAGMENT_முனை:
	நிலை DOM_NOTATION_முனை:
	நிலை DOM_ENTITY_REFERENCE_முனை:
	நிலை DOM_ENTITY_முனை:
	நிலை DOM_CDATA_SECTION_முனை:
	இயல்பு:
		/* Oh bum, just node then */
		duk_தள்ளு_string(ctx, PROTO_NAME(முனை_2));
	}
}

duk_bool_t
dukky_தள்ளு_முனை(duk_context *ctx, கட்டமைப்பு dom_முனை *முனை)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", முனை);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, முனை_MAGIC);
	/* ... nodes */
	duk_தள்ளு_pointer(ctx, முனை);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	எனில் (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_செருகு(ctx, -2);
		/* ... node nodes */
		duk_எடு(ctx);
		/* ... node */
		எனில் (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			மாறிலி எழுத்து * what = duk_safe_to_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_எடு(ctx);
		}
		திரும்பு true;
	}
	/* ... nodes undefined */
	duk_எடு_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_தள்ளு_pointer(ctx, முனை);
	/* ... nodeptr */
	dukky_தள்ளு_முனை_klass(ctx, முனை);
	/* ... nodeptr klass */
	திரும்பு dukky_தள்ளு_முனை_stacked(ctx);
}

நிலையான duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	திரும்பு duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

வெறுமை
dukky_inject_not_ctr(duk_context *ctx, முழு idx, மாறிலி எழுத்து *name)
{
	/* ... p[idx] ... proto */
	duk_தள்ளு_c_function(ctx, dukky_bad_constructor, 0);
	/* ... p[idx] ... proto cons */
	duk_செருகு(ctx, -2);
	/* ... p[idx] ... cons proto */
	duk_put_prop_string(ctx, -2, "prototype");
	/* ... p[idx] ... cons[proto] */
	duk_put_prop_string(ctx, idx, name);
	/* ... p ... */
	திரும்பு;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

நிலையான வெறுமை *dukky_alloc_function(வெறுமை *udata, duk_size_t size)
{
	எனில் (size == 0)
		திரும்பு வெற்று;

	திரும்பு malloc(size);
}

நிலையான வெறுமை *dukky_realloc_function(வெறுமை *udata, வெறுமை *ptr, duk_size_t size)
{
	எனில் (ptr == வெற்று && size == 0)
		திரும்பு வெற்று;

	எனில் (size == 0) {
		விடுவி(ptr);
		திரும்பு வெற்று;
	}

	திரும்பு realloc(ptr, size);
}


நிலையான வெறுமை dukky_free_function(வெறுமை *udata, வெறுமை *ptr)
{
	எனில் (ptr != வெற்று)
		விடுவி(ptr);
}

/* exported interface documented in js.h */
வெறுமை js_initialise(வெறுமை)
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
வெறுமை js_finalise(வெறுமை)
{
	/* NADA for now */
}


/* exported interface documented in js.h */
nserror
js_newheap(முழு timeout, jsheap **heap)
{
	duk_context *ctx;
	jsheap *ret = calloc(1, அளவு(*ret));
	*heap = வெற்று;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	எனில் (ret == வெற்று) திரும்பு NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_heap(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		வெற்று);
	எனில் (ret->ctx == வெற்று) { விடுவி(ret); திரும்பு NSERROR_NOMEM; }
	/* Create the prototype stuffs */
	duk_தள்ளு_global_object(ctx);
	duk_தள்ளு_boolean(ctx, true);
	duk_put_prop_string(ctx, -2, "protos");
	duk_put_global_string(ctx, PROTO_MAGIC);
	/* Create prototypes here */
	dukky_create_prototypes(ctx);
	/* Now create the thread map */
	duk_தள்ளு_object(ctx);
	duk_put_global_string(ctx, THREAD_MAP);

	*heap = ret;
	திரும்பு NSERROR_OK;
}


நிலையான வெறுமை dukky_destroyheap(jsheap *heap)
{
	ப்ரதிஜ்ஞா(heap->pending_destroy == true);
	ப்ரதிஜ்ஞா(heap->live_threads == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_heap(heap->ctx);
	விடுவி(heap);
}

/* exported interface documented in js.h */
வெறுமை js_destroyheap(jsheap *heap)
{
	heap->pending_destroy = true;
	எனில் (heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* Just for here, the CTX is in ret, not thread */
#define CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *heap, வெறுமை *win_priv, வெறுமை *doc_priv, jsthread **thread)
{
	jsthread *ret;
	ப்ரதிஜ்ஞா(heap != வெற்று);
	ப்ரதிஜ்ஞா(heap->pending_destroy == false);

	ret = calloc(1, அளவு (*ret));
	எனில் (ret == வெற்று) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		திரும்பு NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_தள்ளு_thread(heap->ctx); /* ... threads thread */
	ret->heap = heap;
	ret->ctx = duk_require_context(heap->ctx, -1);
	ret->thread_idx = heap->next_thread++;
	duk_put_prop_index(heap->ctx, -2, ret->thread_idx);
	heap->live_threads++;
	duk_எடு(heap->ctx); /* ... */
	duk_தள்ளு_int(CTX, 0);
	duk_தள்ளு_int(CTX, 1);
	duk_தள்ளு_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_தள்ளு_pointer(CTX, win_priv);
	duk_தள்ளு_pointer(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(WINDOW), 2);
	duk_தள்ளு_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_set_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_தள்ளு_object(CTX);
	duk_தள்ளு_pointer(CTX, வெற்று);
	duk_தள்ளு_null(CTX);
	duk_put_prop(CTX, -3);
	duk_put_global_string(CTX, முனை_MAGIC);

	/* And now the event mapping table */
	duk_தள்ளு_object(CTX);
	duk_put_global_string(CTX, EVENT_MAGIC);

	/* Now load the polyfills */
	/* ... */
	duk_தள்ளு_string(CTX, "polyfill.js");
	/* ..., polyfill.js */
	எனில் (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (மாறிலி எழுத்து *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		திரும்பு NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	எனில் (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		திரும்பு NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_எடு(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_தள்ளு_string(CTX, "generics.js");
	/* ..., generics.js */
	எனில் (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (மாறிலி எழுத்து *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		திரும்பு NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	எனில் (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		திரும்பு NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_எடு(CTX);
	/* ... */
	duk_தள்ளு_global_object(CTX);
	/* ..., Win */
	duk_get_prop_string(CTX, -1, "NetSurf");
	/* ..., Win, NetSurf */
	duk_put_global_string(CTX, GENERICS_MAGIC);
	/* ..., Win */
	duk_del_prop_string(CTX, -1, "NetSurf");
	duk_எடு(CTX);
	/* ... */

	dukky_log_அடுக்கு_frame(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, heap);
	*thread = ret;

	திரும்பு NSERROR_OK;
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
	duk_int_t உச்சி = duk_get_உச்சி(CTX);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, thread->heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Restore whatever stack we had */
	duk_set_உச்சி(CTX, உச்சி);

	திரும்பு NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
நிலையான வெறுமை dukky_destroythread(jsthread *thread)
{
	jsheap *heap = thread->heap;

	ப்ரதிஜ்ஞா(thread->in_use == 0);
	ப்ரதிஜ்ஞா(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_index(heap->ctx, -1, thread->thread_idx);
	duk_எடு(heap->ctx); /* ... */

	/* We can now free the thread object */
	விடுவி(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(heap->ctx, 0);
	duk_gc(heap->ctx, DUK_GC_COMPACT);
	heap->live_threads--;

	/* And if the heap should now go, blow it away */
	எனில் (heap->pending_destroy == true && heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* exported interface documented in js.h */
வெறுமை js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	எனில் (thread->in_use == 0) {
		dukky_destroythread(thread);
	}
}

நிலையான வெறுமை dukky_enter_thread(jsthread *thread)
{
	ப்ரதிஜ்ஞா(thread != வெற்று);
	thread->in_use++;
}

நிலையான வெறுமை dukky_leave_thread(jsthread *thread)
{
	ப்ரதிஜ்ஞா(thread != வெற்று);
	ப்ரதிஜ்ஞா(thread->in_use > 0);

	thread->in_use--;
	எனில் (thread->in_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(வெறுமை *udata)
{
#define JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *heap = (jsheap *) udata;
	uint64_t now;

	(வெறுமை) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	திரும்பு heap->exec_start_time != 0 &&
			now > (heap->exec_start_time + JS_EXEC_TIMEOUT_MS);
}

நிலையான வெறுமை dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_உச்சி(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_safe_to_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_எடு(ctx);
	/* ..., errobj */
}

நிலையான வெறுமை dukky_reset_start_time(duk_context *ctx)
{
	duk_memory_functions funcs;
	jsheap *heap;
	duk_get_memory_functions(ctx, &funcs);
	heap = funcs.udata;
	(வெறுமை) nsu_getmonotonic_ms(&heap->exec_start_time);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_size_t argc, bool reset_timeout)
{
	எனில் (reset_timeout) {
		dukky_reset_start_time(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	எனில் (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	திரும்பு ret;
}


வெறுமை dukky_தள்ளு_generics(duk_context *ctx, மாறிலி எழுத்து *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

நிலையான duk_int_t dukky_தள்ளு_context_dump(duk_context *ctx, வெறுமை *udata)
{
	duk_தள்ளு_context_dump(ctx);
	திரும்பு 1;
}

வெறுமை dukky_log_அடுக்கு_frame(duk_context *ctx, மாறிலி எழுத்து * reason)
{
	எனில் (duk_safe_call(ctx, dukky_தள்ளு_context_dump, வெற்று, 0, 1) != 0) {
		duk_எடு(ctx);
		duk_தள்ளு_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_safe_to_string(ctx, -1));
	duk_எடு(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, மாறிலி uint8_t *txt, size_t txtlen, மாறிலி எழுத்து *name)
{
	bool ret = false;
	ப்ரதிஜ்ஞா(thread);

	எனில் (txt == வெற்று || txtlen == 0) {
		திரும்பு false;
	}

	எனில் (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		திரும்பு false;
	}

	dukky_enter_thread(thread);

	duk_set_உச்சி(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_start_time(CTX);
	எனில் (name != வெற்று) {
		duk_தள்ளு_string(CTX, name);
	} இல்லையெனில் {
		duk_தள்ளு_string(CTX, "?unknown source?");
	}
	எனில் (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (மாறிலி எழுத்து *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		செல் handle_error;
	}

	எனில் (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		செல் handle_error;
	}

	எனில் (duk_get_உச்சி(CTX) == 0) duk_தள்ளு_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	செல் out;

handle_error:
	dukky_dump_error(CTX);
out:
	dukky_leave_thread(thread);
	திரும்பு ret;
}

நிலையான மாறிலி எழுத்து* dukky_event_proto(dom_event *evt)
{
	மாறிலி எழுத்து *ret = PROTO_NAME(EVENT);
	dom_string *type = வெற்று;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	எனில் (err != DOM_NO_ERR) {
		செல் out;
	}

	எனில் (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		செல் out;
	} இல்லையெனில் எனில் (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		செல் out;
	} இல்லையெனில் எனில் (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		செல் out;
	}

out:
	எனில் (type != வெற்று) {
		dom_string_unref(type);
	}

	திரும்பு ret;
}

/*** New style event handling ***/

வெறுமை dukky_தள்ளு_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_தள்ளு_pointer(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	எனில் (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_எடு(ctx);
		/* ... events */
		duk_தள்ளு_pointer(ctx, evt);
		எனில் (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_எடு(ctx);
			/* ... events */
			duk_தள்ளு_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_தள்ளு_pointer(ctx, evt);
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

நிலையான வெறுமை dukky_தள்ளு_handler_code(duk_context *ctx, dom_string *name,
				     dom_event_target *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_முனை_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	எனில் (et == வெற்று) {
		duk_தள்ளு_lstring(ctx, "", 0);
		திரும்பு;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_முனை_get_முனை_type(et, &ntype);
	எனில் (exc != DOM_NO_ERR) {
		duk_தள்ளு_lstring(ctx, "", 0);
		திரும்பு;
	}

	எனில் (ntype != DOM_ELEMENT_முனை) {
		duk_தள்ளு_lstring(ctx, "", 0);
		திரும்பு;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	எனில் (exc != DOM_NO_ERR) {
		duk_தள்ளு_lstring(ctx, "", 0);
		திரும்பு;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	எனில் ((exc != DOM_NO_ERR) || (val == வெற்று)) {
		dom_string_unref(onname);
		duk_தள்ளு_lstring(ctx, "", 0);
		திரும்பு;
	}

	dom_string_unref(onname);
	duk_தள்ளு_lstring(ctx, dom_string_data(val), dom_string_length(val));
	dom_string_unref(val);
}

bool dukky_get_current_மதிப்பு_of_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_target *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_தள்ளு_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	எனில் (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_எடு_2(ctx);
		/* ... node */
		dukky_தள்ளு_handler_code(ctx, name, et);
		/* ... node handlercode? */
		/* TODO: If this is null, clean up and propagate */
		/* ... node handlercode */
		/** @todo This is entirely wrong, but it's hard to get right */
		duk_தள்ளு_string(ctx, "function (event) {");
		/* ... node handlercode prefix */
		duk_செருகு(ctx, -2);
		/* ... node prefix handlercode */
		duk_தள்ளு_string(ctx, "}");
		/* ... node prefix handlercode suffix */
		duk_concat(ctx, 3);
		/* ... node fullhandlersrc */
		duk_தள்ளு_string(ctx, "internal raw uncompiled handler");
		/* ... node fullhandlersrc filename */
		எனில் (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_எடு_2(ctx);
			திரும்பு false;
		}
		/* ... node handler */
		duk_செருகு(ctx, -2);
		/* ... handler node */
	} இல்லையெனில் {
		/* ... node handlers handler */
		duk_செருகு(ctx, -3);
		/* ... handler node handlers */
		duk_எடு(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	திரும்பு true;
}

நிலையான வெறுமை dukky_generic_event_handler(dom_event *evt, வெறுமை *pw)
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
	எனில் (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		திரும்பு;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (முழு)dom_string_length(name),
	      dom_string_data(name));
	exc = dom_event_get_event_phase(evt, &phase);
	எனில் (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		திரும்பு;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_TARGET ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (முழு)phase);

	exc = dom_event_get_current_target(evt, &targ);
	எனில் (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		திரும்பு;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	எனில் (phase == DOM_CAPTURING_PHASE)
		செல் handle_extras;

	/* ... */
	எனில் (dukky_தள்ளு_முனை(ctx, (dom_முனை *)targ) == false) {
		dom_string_unref(name);
		dom_முனை_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		திரும்பு;
	}
	/* ... node */
	எனில் (dukky_get_current_மதிப்பு_of_event_handler(
		    ctx, name, (dom_event_target *)targ) == false) {
		/* ... */
		செல் handle_extras;
	}
	/* ... handler node */
	dukky_தள்ளு_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_start_time(ctx);
	எனில் (duk_pcall_method(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_immediate_propagation(evt);
		எனில் (exc != DOM_NO_ERR)
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

		duk_எடு_n(ctx, 6);
		/* ... */
		செல் handle_extras;
	}
	/* ... result */
	எனில் (duk_is_boolean(ctx, -1) &&
	    duk_to_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_எடு(ctx);
handle_extras:
	/* ... */
	duk_தள்ளு_lstring(ctx, dom_string_data(name), dom_string_length(name));
	dukky_தள்ளு_முனை(ctx, (dom_முனை *)targ);
	/* ... type node */
	எனில் (dukky_event_target_தள்ளு_listeners(ctx, true)) {
		/* Nothing to do */
		duk_எடு(ctx);
		செல் out;
	}
	/* ... sublisteners */
	duk_தள்ளு_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	போது (duk_get_prop_index(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_index(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		எனில் ((event_listener_flags)duk_to_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_எடு(ctx);
			/* ... subl copy handler flags */
		}
		duk_எடு(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_index(ctx, -2, idx);
		/* ... sublisteners copy */
		idx++;
	}
	/* ... sublisteners copy undefined */
	duk_எடு(ctx);
	/* ... sublisteners copy */
	duk_செருகு(ctx, -2);
	/* ... copy sublisteners */
	duk_எடு(ctx);
	/* ... copy */
	idx = 0;
	போது (duk_get_prop_index(ctx, -1, idx++)) {
		/* ... copy handler */
		எனில் (duk_get_prop_index(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_எடு_2(ctx);
			தொடர்;
		}
		duk_எடு(ctx);
		duk_get_prop_index(ctx, -1, 0);
		duk_get_prop_index(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_எடு(ctx);
		/* ... copy handler callback */
		எனில் (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_எடு_2(ctx);
			/* ... copy */
			தொடர்;
		}
		/* ... copy handler callback */
		dukky_தள்ளு_முனை(ctx, (dom_முனை *)targ);
		/* ... copy handler callback node */
		dukky_தள்ளு_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_start_time(ctx);
		எனில் (duk_pcall_method(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_immediate_propagation(evt);
			எனில் (exc != DOM_NO_ERR)
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

			duk_எடு_n(ctx, 7);
			/* ... copy */
			தொடர்;
		}
		/* ... copy handler result */
		எனில் (duk_is_boolean(ctx, -1) &&
		    duk_to_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_எடு_2(ctx);
		/* ... copy */
	}
	duk_எடு_2(ctx);
out:
	/* ... */
	dom_முனை_unref(targ);
	dom_string_unref(name);
}

வெறுமை dukky_register_event_listener_for(duk_context *ctx,
				       கட்டமைப்பு dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = வெற்று;
	dom_exception exc;

	/* ... */
	எனில் (ele == வெற்று) {
		/* A null element is the Window object */
		duk_தள்ளு_global_object(ctx);
	} இல்லையெனில் {
		/* Non null elements must be pushed as a node object */
		எனில் (dukky_தள்ளு_முனை(ctx, (கட்டமைப்பு dom_முனை *)ele) == false)
			திரும்பு;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_தள்ளு_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	எனில் (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_எடு_2(ctx);
		/* ... */
		திரும்பு;
	}
	/* ... node handlers */
	duk_தள்ளு_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_தள்ளு_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_எடு_2(ctx);
	/* ... */
	எனில் (ele == வெற்று) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		திரும்பு;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	எனில் (exc != DOM_NO_ERR) திரும்பு;
	exc = dom_event_target_add_event_listener(
		ele, name, listen, capture);
	எனில் (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (முழு)dom_string_length(name), dom_string_data(name));
	} இல்லையெனில் {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (முழு)dom_string_length(name), dom_string_data(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_target_தள்ளு_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	எனில் (duk_is_undefined(ctx, -1)) {
		/* ... type this null */
		duk_எடு(ctx);
		duk_தள்ளு_object(ctx);
		duk_dup(ctx, -1);
		/* ... type this listeners listeners */
		duk_put_prop_string(ctx, -3, EVENT_LISTENER_JS_MAGIC);
		/* ... type this listeners */
	}
	/* ... type this listeners */
	duk_செருகு(ctx, -3);
	/* ... listeners type this */
	duk_எடு(ctx);
	/* ... listeners type */
	duk_dup(ctx, -1);
	/* ... listeners type type */
	duk_get_prop(ctx, -3);
	/* ... listeners type ??? */
	எனில் (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		எனில் (dont_create == true) {
			duk_எடு_3(ctx);
			duk_தள்ளு_undefined(ctx);
			திரும்பு true;
		}
		duk_எடு(ctx);
		duk_தள்ளு_array(ctx);
		duk_dup(ctx, -2);
		duk_dup(ctx, -2);
		/* ... listeners type sublisteners type sublisteners */
		duk_put_prop(ctx, -5);
		/* ... listeners type sublisteners */
		ret = true;
	}
	duk_செருகு(ctx, -3);
	/* ... sublisteners listeners type */
	duk_எடு_2(ctx);
	/* ... sublisteners */
	திரும்பு ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
வெறுமை dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	போது (duk_get_prop_index(ctx, -1, idx + 1)) {
		duk_put_prop_index(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_index(ctx, -2, idx + 1);
	duk_எடு(ctx);
}


வெறுமை js_handle_new_element(jsthread *thread, கட்டமைப்பு dom_element *முனை)
{
	ப்ரதிஜ்ஞா(thread);
	ப்ரதிஜ்ஞா(முனை);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = வெற்று;
	dom_string *சாவி = வெற்று;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_முனை_get_முனை_name(முனை, &nodename);
	எனில் (exc != DOM_NO_ERR) திரும்பு;

	எனில் (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_முனை_get_attributes(முனை, &map);
	எனில் (exc != DOM_NO_ERR) திரும்பு;
	எனில் (map == வெற்று) திரும்பு;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_length(map, &siz);
	எனில் (exc != DOM_NO_ERR) செல் out;

	க்கு (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_item(map, idx, &attr);
		எனில் (exc != DOM_NO_ERR) செல் out;
		exc = dom_attr_get_name(attr, &சாவி);
		எனில் (exc != DOM_NO_ERR) செல் out;
		எனில் (is_body && (
			    சாவி == corestring_dom_onblur ||
			    சாவி == corestring_dom_onerror ||
			    சாவி == corestring_dom_onfocus ||
			    சாவி == corestring_dom_onload ||
			    சாவி == corestring_dom_onresize ||
			    சாவி == corestring_dom_onscroll)) {
			/* This is a forwarded event, it doesn't matter,
			 * we should skip registering for it and later
			 * we will register it for Window itself
			 */
			செல் skip_register;
		}
		எனில் (dom_string_length(சாவி) > 2) {
			/* Can be on* */
			மாறிலி uint8_t *data = (மாறிலி uint8_t *)dom_string_data(சாவி);
			எனில் (data[0] == 'o' && data[1] == 'n') {
				dom_string *sub = வெற்று;
				exc = dom_string_substr(
					சாவி, 2, dom_string_length(சாவி),
					&sub);
				எனில் (exc == DOM_NO_ERR) {
					dukky_register_event_listener_for(
						CTX, முனை, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_register:
		dom_string_unref(சாவி); சாவி = வெற்று;
		dom_முனை_unref(attr); attr = வெற்று;
	}

out:
	எனில் (சாவி != வெற்று)
		dom_string_unref(சாவி);

	எனில் (attr != வெற்று)
		dom_முனை_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

வெறுமை js_event_cleanup(jsthread *thread, கட்டமைப்பு dom_event *evt)
{
	ப்ரதிஜ்ஞா(thread);
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, EVENT_MAGIC);
	/* ... EVENT_MAP */
	duk_தள்ளு_pointer(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_எடு(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, மாறிலி எழுத்து *type, கட்டமைப்பு dom_document *doc, கட்டமைப்பு dom_முனை *target)
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

	எனில் (target != வெற்று)
		/* Swallow non-Window-targetted events quietly */
		திரும்பு true;

	எனில் (strcmp(type, "load") != 0)
		/* Swallow non-load events quietly */
		திரும்பு true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	எனில் (exc != DOM_NO_ERR) திரும்பு true;
	exc = dom_event_init(evt, corestring_dom_load, false, false);
	எனில் (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		திரும்பு true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_தள்ளு_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	எனில் (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_எடு(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		எனில் (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			திரும்பு true;
		}
		dukky_தள்ளு_முனை(CTX, (கட்டமைப்பு dom_முனை *)body);
		/* ... handlers bodynode */
		எனில் (dukky_get_current_மதிப்பு_of_event_handler(
			    CTX, corestring_dom_load, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_முனை_unref(body);
			/* ... handlers */
			duk_எடு(CTX);
			dukky_leave_thread(thread);
			திரும்பு true;
		}
		/* Unref the body, we don't need it any more */
		dom_முனை_unref(body);
		/* ... handlers handler bodynode */
		duk_எடு(CTX);
	}
	/* ... handlers handler */
	duk_செருகு(CTX, -2);
	/* ... handler handlers */
	duk_எடு(CTX);
	/* ... handler */
	duk_தள்ளு_global_object(CTX);
	/* ... handler Window */
	dukky_தள்ளு_event(CTX, evt);
	/* ... handler Window event */
	dukky_reset_start_time(CTX);
	எனில் (duk_pcall_method(CTX, 1) != 0) {
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

		duk_எடு_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		திரும்பு true;
	}
	/* ... result */
	duk_எடு(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	திரும்பு true;
}
