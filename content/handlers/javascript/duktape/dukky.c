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

#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <nsutils/time.h>

#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "content/content.h"

#অন্তর্ভাব "javascript/js.h"
#অন্তর্ভাব "javascript/content.h"

#অন্তর্ভাব "duktape/binding.h"
#অন্তর্ভাব "duktape/generics.js.inc"
#অন্তর্ভাব "duktape/polyfill.js.inc"

#অন্তর্ভাব "duktape.h"
#অন্তর্ভাব "dukky.h"

#অন্তর্ভাব <dom/dom.h>

#define EVENT_MAGIC MAGIC(EVENT_MAP)
#define HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#define HANDLER_MAGIC MAGIC(HANDLER_MAP)
#define EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#define GENERICS_MAGIC MAGIC(GENERICS_TABLE)
#define THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
কাঠামো jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t পরবর্তী_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	unsigned পূর্ণসংখ্যা live_threads; /**< number of live threads */
	uint64_t exec_start_time;
};

/**
 * dukky javascript thread
 */
কাঠামো jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	unsigned পূর্ণসংখ্যা in_use; /**< The number of times this thread is in use */
	jsheap *heap; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

স্থির duk_ret_t dukky_populate_object(duk_context *ctx, শূন্য *udata)
{
	/* ... obj args protoname nargs */
	পূর্ণসংখ্যা nargs = duk_get_int(ctx, -1);
	duk_বের_করো(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	যদি (duk_is_undefined(ctx, -1)) {
		NSLOG(dukky, WARNING,
		      "Unable to find dukky prototype for `%s` - falling back to HTMLUnknownElement",
		      duk_get_string(ctx, -3) + 2 /* Skip the two unprintables */);
		duk_বের_করো(ctx);
		duk_ঠেলো_string(ctx, PROTO_NAME(HTMLUNKNOWNELEMENT));
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
	duk_ঢোকাও(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_বের_করো_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	ফেরত 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা args)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, args);
	/* ... args */
	duk_ঠেলো_object(ctx);
	/* ... args obj */
	duk_ঠেলো_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
	/* ... args obj */
	duk_ঠেলো_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
	/* ... args obj */
	duk_ঢোকাও(ctx, -(args+1));
	/* ... obj args */
	duk_ঠেলো_string(ctx, name);
	/* ... obj args name */
	duk_ঠেলো_int(ctx, args);
	/* ... obj args name nargs */
	যদি ((ret = duk_safe_call(ctx, dukky_populate_object, ফাঁকা, args + 3, 1))
	    != DUK_EXEC_SUCCESS)
		ফেরত ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	ফেরত DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_ঠেলো_গ্রন্থি_stacked(duk_context *ctx)
{
	পূর্ণসংখ্যা শীর্ষ_at_fail = duk_get_শীর্ষ(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, গ্রন্থি_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	যদি (duk_is_undefined(ctx, -1)) {
		/* ... nodeptr klass nodes undefined */
		duk_বের_করো(ctx);
		/* ... nodeptr klass nodes */
		duk_ঠেলো_object(ctx);
		/* ... nodeptr klass nodes obj */
		duk_ঠেলো_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_ঠেলো_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr klass */
		duk_ঠেলো_int(ctx, 1);
		/* ... nodeptr klass nodes obj nodeptr klass 1 */
		যদি (duk_safe_call(ctx, dukky_populate_object, ফাঁকা, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_set_শীর্ষ(ctx, শীর্ষ_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			ফেরত false;
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
	duk_ঢোকাও(ctx, -4);
	/* ... node nodeptr klass nodes */
	duk_বের_করো_3(ctx);
	/* ... node */
	যদি (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		ধ্রুবক অক্ষর * what = duk_safe_to_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_বের_করো(ctx);
	}
	ফেরত true;
}

#define SET_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

স্থির শূন্য dukky_html_element_class_from_tag_type(dom_html_element_type type,
		ধ্রুবক অক্ষর **html_class, size_t *html_class_len)
{
	নির্বাচন(type) {
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_HTML:
		SET_HTML_CLASS(HTML)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_শীর্ষ:
		SET_HTML_CLASS(শীর্ষ_3)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_META:
		SET_HTML_CLASS(META)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_BASE:
		SET_HTML_CLASS(BASE)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_TITLE:
		SET_HTML_CLASS(TITLE)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_BODY:
		SET_HTML_CLASS(BODY)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_DIV:
		SET_HTML_CLASS(DIV)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_FORM:
		SET_HTML_CLASS(FORM)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_LINK:
		SET_HTML_CLASS(LINK)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_BUTTON:
		SET_HTML_CLASS(BUTTON)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_প্রবেশ:
		SET_HTML_CLASS(প্রবেশ_2)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		SET_HTML_CLASS(TEXTAREA)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		SET_HTML_CLASS(OPTGROUP)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_OPTION:
		SET_HTML_CLASS(OPTION)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_SELECT:
		SET_HTML_CLASS(SELECT)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_HR:
		SET_HTML_CLASS(HR)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_DL:
		SET_HTML_CLASS(DLIST)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_DIR:
		SET_HTML_CLASS(DIRECTORY)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_MENU:
		SET_HTML_CLASS(MENU)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_FIELDSET:
		SET_HTML_CLASS(FIELDSET)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_LEGEND:
		SET_HTML_CLASS(LEGEND)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_P:
		SET_HTML_CLASS(PARAGRAPH)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_H1:
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_H2:
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_H3:
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_H4:
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_H5:
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_H6:
		SET_HTML_CLASS(HEADING)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_Q:
		SET_HTML_CLASS(QUOTE)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_PRE:
		SET_HTML_CLASS(PRE)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_BR:
		SET_HTML_CLASS(BR)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_LABEL:
		SET_HTML_CLASS(LABEL)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_UL:
		SET_HTML_CLASS(ULIST)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_OL:
		SET_HTML_CLASS(OLIST)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_LI:
		SET_HTML_CLASS(LI)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_FONT:
		SET_HTML_CLASS(FONT)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_DEL:
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_INS:
		SET_HTML_CLASS(MOD)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_A:
		SET_HTML_CLASS(ANCHOR)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_BASEFONT:
		SET_HTML_CLASS(BASEFONT)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_IMG:
		SET_HTML_CLASS(IMAGE)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_OBJECT:
		SET_HTML_CLASS(OBJECT)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_PARAM:
		SET_HTML_CLASS(PARAM)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_APPLET:
		SET_HTML_CLASS(APPLET)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_MAP:
		SET_HTML_CLASS(MAP)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_AREA:
		SET_HTML_CLASS(AREA)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_SCRIPT:
		SET_HTML_CLASS(SCRIPT)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_CAPTION:
		SET_HTML_CLASS(TABLECAPTION)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_TD:
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_TH:
		SET_HTML_CLASS(TABLECELL)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_COL:
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_COLGROUP:
		SET_HTML_CLASS(TABLECOL)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_THEAD:
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_TBODY:
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_TFOOT:
		SET_HTML_CLASS(TABLESECTION)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_TABLE:
		SET_HTML_CLASS(TABLE)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_TR:
		SET_HTML_CLASS(TABLEROW)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_STYLE:
		SET_HTML_CLASS(STYLE)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_FRAMESET:
		SET_HTML_CLASS(FRAMESET)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_FRAME:
		SET_HTML_CLASS(FRAME)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_IFRAME:
		SET_HTML_CLASS(IFRAME)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_ISINDEX:
		SET_HTML_CLASS(ISINDEX)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_CANVAS:
		SET_HTML_CLASS(CANVAS)
		বিরতি;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE_গুনতি:
		প্রতিজ্ঞা(type != DOM_HTML_ELEMENT_TYPE_গুনতি);
		fallthrough;
	ক্ষেত্র DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		SET_HTML_CLASS(UNKNOWN)
		বিরতি;
	পূর্বনির্ধারিত:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		বিরতি;
	}
	ফেরত;
}

#undef SET_HTML_CLASS

স্থির শূন্য
dukky_ঠেলো_গ্রন্থি_klass(duk_context *ctx, কাঠামো dom_গ্রন্থি *গ্রন্থি)
{
	dom_গ্রন্থি_type nodetype;
	dom_exception err;

	err = dom_গ্রন্থি_get_গ্রন্থি_type(গ্রন্থি, &nodetype);
	যদি (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_ঠেলো_string(ctx, PROTO_NAME(গ্রন্থি_2));
		ফেরত;
	}

	নির্বাচন(nodetype) {
	ক্ষেত্র DOM_ELEMENT_গ্রন্থি: {
		dom_string *namespace;
		dom_html_element_type type;
		ধ্রুবক অক্ষর *html_class;
		size_t html_class_len;
		err = dom_গ্রন্থি_get_namespace(গ্রন্থি, &namespace);
		যদি (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_ঠেলো_string(ctx, PROTO_NAME(ELEMENT));
			বিরতি;
		}
		যদি (namespace == ফাঁকা) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_ঠেলো_string(ctx, PROTO_NAME(ELEMENT));
			বিরতি;
		}

		যদি (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_ঠেলো_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			বিরতি;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_tag_type(গ্রন্থি, &type);
		যদি (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_tag_type(type,
				&html_class, &html_class_len);

		duk_ঠেলো_lstring(ctx, html_class, html_class_len);
		বিরতি;
	}
	ক্ষেত্র DOM_TEXT_গ্রন্থি:
		duk_ঠেলো_string(ctx, PROTO_NAME(TEXT));
		বিরতি;
	ক্ষেত্র DOM_COMMENT_গ্রন্থি:
		duk_ঠেলো_string(ctx, PROTO_NAME(COMMENT));
		বিরতি;
	ক্ষেত্র DOM_DOCUMENT_গ্রন্থি:
		duk_ঠেলো_string(ctx, PROTO_NAME(DOCUMENT));
		বিরতি;
	ক্ষেত্র DOM_ATTRIBUTE_গ্রন্থি:
	ক্ষেত্র DOM_PROCESSING_INSTRUCTION_গ্রন্থি:
	ক্ষেত্র DOM_DOCUMENT_TYPE_গ্রন্থি:
	ক্ষেত্র DOM_DOCUMENT_FRAGMENT_গ্রন্থি:
	ক্ষেত্র DOM_NOTATION_গ্রন্থি:
	ক্ষেত্র DOM_ENTITY_REFERENCE_গ্রন্থি:
	ক্ষেত্র DOM_ENTITY_গ্রন্থি:
	ক্ষেত্র DOM_CDATA_SECTION_গ্রন্থি:
	পূর্বনির্ধারিত:
		/* Oh bum, just node then */
		duk_ঠেলো_string(ctx, PROTO_NAME(গ্রন্থি_2));
	}
}

duk_bool_t
dukky_ঠেলো_গ্রন্থি(duk_context *ctx, কাঠামো dom_গ্রন্থি *গ্রন্থি)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", গ্রন্থি);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, গ্রন্থি_MAGIC);
	/* ... nodes */
	duk_ঠেলো_pointer(ctx, গ্রন্থি);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	যদি (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_ঢোকাও(ctx, -2);
		/* ... node nodes */
		duk_বের_করো(ctx);
		/* ... node */
		যদি (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			ধ্রুবক অক্ষর * what = duk_safe_to_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_বের_করো(ctx);
		}
		ফেরত true;
	}
	/* ... nodes undefined */
	duk_বের_করো_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_ঠেলো_pointer(ctx, গ্রন্থি);
	/* ... nodeptr */
	dukky_ঠেলো_গ্রন্থি_klass(ctx, গ্রন্থি);
	/* ... nodeptr klass */
	ফেরত dukky_ঠেলো_গ্রন্থি_stacked(ctx);
}

স্থির duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	ফেরত duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

শূন্য
dukky_inject_not_ctr(duk_context *ctx, পূর্ণসংখ্যা idx, ধ্রুবক অক্ষর *name)
{
	/* ... p[idx] ... proto */
	duk_ঠেলো_c_function(ctx, dukky_bad_constructor, 0);
	/* ... p[idx] ... proto cons */
	duk_ঢোকাও(ctx, -2);
	/* ... p[idx] ... cons proto */
	duk_put_prop_string(ctx, -2, "prototype");
	/* ... p[idx] ... cons[proto] */
	duk_put_prop_string(ctx, idx, name);
	/* ... p ... */
	ফেরত;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

স্থির শূন্য *dukky_alloc_function(শূন্য *udata, duk_আকার_t আকার_2)
{
	যদি (আকার_2 == 0)
		ফেরত ফাঁকা;

	ফেরত malloc(আকার_2);
}

স্থির শূন্য *dukky_realloc_function(শূন্য *udata, শূন্য *ptr, duk_আকার_t আকার_2)
{
	যদি (ptr == ফাঁকা && আকার_2 == 0)
		ফেরত ফাঁকা;

	যদি (আকার_2 == 0) {
		মুক্ত(ptr);
		ফেরত ফাঁকা;
	}

	ফেরত realloc(ptr, আকার_2);
}


স্থির শূন্য dukky_free_function(শূন্য *udata, শূন্য *ptr)
{
	যদি (ptr != ফাঁকা)
		মুক্ত(ptr);
}

/* exported interface documented in js.h */
শূন্য js_initialise(শূন্য)
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
শূন্য js_finalise(শূন্য)
{
	/* NADA for now */
}


/* exported interface documented in js.h */
nserror
js_newheap(পূর্ণসংখ্যা timeout, jsheap **heap)
{
	duk_context *ctx;
	jsheap *ret = calloc(1, আকার(*ret));
	*heap = ফাঁকা;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	যদি (ret == ফাঁকা) ফেরত NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_heap(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		ফাঁকা);
	যদি (ret->ctx == ফাঁকা) { মুক্ত(ret); ফেরত NSERROR_NOMEM; }
	/* Create the prototype stuffs */
	duk_ঠেলো_global_object(ctx);
	duk_ঠেলো_boolean(ctx, true);
	duk_put_prop_string(ctx, -2, "protos");
	duk_put_global_string(ctx, PROTO_MAGIC);
	/* Create prototypes here */
	dukky_create_prototypes(ctx);
	/* Now create the thread map */
	duk_ঠেলো_object(ctx);
	duk_put_global_string(ctx, THREAD_MAP);

	*heap = ret;
	ফেরত NSERROR_OK;
}


স্থির শূন্য dukky_destroyheap(jsheap *heap)
{
	প্রতিজ্ঞা(heap->pending_destroy == true);
	প্রতিজ্ঞা(heap->live_threads == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_heap(heap->ctx);
	মুক্ত(heap);
}

/* exported interface documented in js.h */
শূন্য js_destroyheap(jsheap *heap)
{
	heap->pending_destroy = true;
	যদি (heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* Just for here, the CTX is in ret, not thread */
#define CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *heap, শূন্য *win_priv, শূন্য *doc_priv, jsthread **thread)
{
	jsthread *ret;
	প্রতিজ্ঞা(heap != ফাঁকা);
	প্রতিজ্ঞা(heap->pending_destroy == false);

	ret = calloc(1, আকার (*ret));
	যদি (ret == ফাঁকা) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		ফেরত NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_ঠেলো_thread(heap->ctx); /* ... threads thread */
	ret->heap = heap;
	ret->ctx = duk_require_context(heap->ctx, -1);
	ret->thread_idx = heap->পরবর্তী_thread++;
	duk_put_prop_সূচক(heap->ctx, -2, ret->thread_idx);
	heap->live_threads++;
	duk_বের_করো(heap->ctx); /* ... */
	duk_ঠেলো_int(CTX, 0);
	duk_ঠেলো_int(CTX, 1);
	duk_ঠেলো_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_ঠেলো_pointer(CTX, win_priv);
	duk_ঠেলো_pointer(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(WINDOW), 2);
	duk_ঠেলো_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_set_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_ঠেলো_object(CTX);
	duk_ঠেলো_pointer(CTX, ফাঁকা);
	duk_ঠেলো_null(CTX);
	duk_put_prop(CTX, -3);
	duk_put_global_string(CTX, গ্রন্থি_MAGIC);

	/* And now the event mapping table */
	duk_ঠেলো_object(CTX);
	duk_put_global_string(CTX, EVENT_MAGIC);

	/* Now load the polyfills */
	/* ... */
	duk_ঠেলো_string(CTX, "polyfill.js");
	/* ..., polyfill.js */
	যদি (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ধ্রুবক অক্ষর *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		ফেরত NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	যদি (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		ফেরত NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_বের_করো(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_ঠেলো_string(CTX, "generics.js");
	/* ..., generics.js */
	যদি (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ধ্রুবক অক্ষর *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		ফেরত NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	যদি (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		ফেরত NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_বের_করো(CTX);
	/* ... */
	duk_ঠেলো_global_object(CTX);
	/* ..., Win */
	duk_get_prop_string(CTX, -1, "NetSurf");
	/* ..., Win, NetSurf */
	duk_put_global_string(CTX, GENERICS_MAGIC);
	/* ..., Win */
	duk_del_prop_string(CTX, -1, "NetSurf");
	duk_বের_করো(CTX);
	/* ... */

	dukky_log_স্তূপ_frame(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, heap);
	*thread = ret;

	ফেরত NSERROR_OK;
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
	duk_int_t শীর্ষ = duk_get_শীর্ষ(CTX);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, thread->heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Restore whatever stack we had */
	duk_set_শীর্ষ(CTX, শীর্ষ);

	ফেরত NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
স্থির শূন্য dukky_destroythread(jsthread *thread)
{
	jsheap *heap = thread->heap;

	প্রতিজ্ঞা(thread->in_use == 0);
	প্রতিজ্ঞা(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_সূচক(heap->ctx, -1, thread->thread_idx);
	duk_বের_করো(heap->ctx); /* ... */

	/* We can now free the thread object */
	মুক্ত(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(heap->ctx, 0);
	duk_gc(heap->ctx, DUK_GC_COMPACT);
	heap->live_threads--;

	/* And if the heap should now go, blow it away */
	যদি (heap->pending_destroy == true && heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* exported interface documented in js.h */
শূন্য js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	যদি (thread->in_use == 0) {
		dukky_destroythread(thread);
	}
}

স্থির শূন্য dukky_enter_thread(jsthread *thread)
{
	প্রতিজ্ঞা(thread != ফাঁকা);
	thread->in_use++;
}

স্থির শূন্য dukky_leave_thread(jsthread *thread)
{
	প্রতিজ্ঞা(thread != ফাঁকা);
	প্রতিজ্ঞা(thread->in_use > 0);

	thread->in_use--;
	যদি (thread->in_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(শূন্য *udata)
{
#define JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *heap = (jsheap *) udata;
	uint64_t now;

	(শূন্য) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	ফেরত heap->exec_start_time != 0 &&
			now > (heap->exec_start_time + JS_EXEC_TIMEOUT_MS);
}

স্থির শূন্য dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_শীর্ষ(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_safe_to_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_বের_করো(ctx);
	/* ..., errobj */
}

স্থির শূন্য dukky_reset_start_time(duk_context *ctx)
{
	duk_memory_functions funcs;
	jsheap *heap;
	duk_get_memory_functions(ctx, &funcs);
	heap = funcs.udata;
	(শূন্য) nsu_getmonotonic_ms(&heap->exec_start_time);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_আকার_t argc, bool reset_timeout)
{
	যদি (reset_timeout) {
		dukky_reset_start_time(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	যদি (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	ফেরত ret;
}


শূন্য dukky_ঠেলো_generics(duk_context *ctx, ধ্রুবক অক্ষর *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

স্থির duk_int_t dukky_ঠেলো_context_dump(duk_context *ctx, শূন্য *udata)
{
	duk_ঠেলো_context_dump(ctx);
	ফেরত 1;
}

শূন্য dukky_log_স্তূপ_frame(duk_context *ctx, ধ্রুবক অক্ষর * reason)
{
	যদি (duk_safe_call(ctx, dukky_ঠেলো_context_dump, ফাঁকা, 0, 1) != 0) {
		duk_বের_করো(ctx);
		duk_ঠেলো_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_safe_to_string(ctx, -1));
	duk_বের_করো(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, ধ্রুবক uint8_t *txt, size_t txtlen, ধ্রুবক অক্ষর *name)
{
	bool ret = false;
	প্রতিজ্ঞা(thread);

	যদি (txt == ফাঁকা || txtlen == 0) {
		ফেরত false;
	}

	যদি (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		ফেরত false;
	}

	dukky_enter_thread(thread);

	duk_set_শীর্ষ(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_start_time(CTX);
	যদি (name != ফাঁকা) {
		duk_ঠেলো_string(CTX, name);
	} অন্যথা {
		duk_ঠেলো_string(CTX, "?unknown source?");
	}
	যদি (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (ধ্রুবক অক্ষর *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		যাও handle_error;
	}

	যদি (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		যাও handle_error;
	}

	যদি (duk_get_শীর্ষ(CTX) == 0) duk_ঠেলো_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	যাও out;

handle_error:
	dukky_dump_error(CTX);
out:
	dukky_leave_thread(thread);
	ফেরত ret;
}

স্থির ধ্রুবক অক্ষর* dukky_event_proto(dom_event *evt)
{
	ধ্রুবক অক্ষর *ret = PROTO_NAME(EVENT);
	dom_string *type = ফাঁকা;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	যদি (err != DOM_NO_ERR) {
		যাও out;
	}

	যদি (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		যাও out;
	} অন্যথা যদি (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		যাও out;
	} অন্যথা যদি (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		যাও out;
	}

out:
	যদি (type != ফাঁকা) {
		dom_string_unref(type);
	}

	ফেরত ret;
}

/*** New style event handling ***/

শূন্য dukky_ঠেলো_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_ঠেলো_pointer(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	যদি (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_বের_করো(ctx);
		/* ... events */
		duk_ঠেলো_pointer(ctx, evt);
		যদি (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_বের_করো(ctx);
			/* ... events */
			duk_ঠেলো_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_ঠেলো_pointer(ctx, evt);
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

স্থির শূন্য dukky_ঠেলো_handler_code(duk_context *ctx, dom_string *name,
				     dom_event_target *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_গ্রন্থি_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	যদি (et == ফাঁকা) {
		duk_ঠেলো_lstring(ctx, "", 0);
		ফেরত;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_গ্রন্থি_get_গ্রন্থি_type(et, &ntype);
	যদি (exc != DOM_NO_ERR) {
		duk_ঠেলো_lstring(ctx, "", 0);
		ফেরত;
	}

	যদি (ntype != DOM_ELEMENT_গ্রন্থি) {
		duk_ঠেলো_lstring(ctx, "", 0);
		ফেরত;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	যদি (exc != DOM_NO_ERR) {
		duk_ঠেলো_lstring(ctx, "", 0);
		ফেরত;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	যদি ((exc != DOM_NO_ERR) || (val == ফাঁকা)) {
		dom_string_unref(onname);
		duk_ঠেলো_lstring(ctx, "", 0);
		ফেরত;
	}

	dom_string_unref(onname);
	duk_ঠেলো_lstring(ctx, dom_string_তথ্য(val), dom_string_দৈর্ঘ্য(val));
	dom_string_unref(val);
}

bool dukky_get_current_মান_of_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_target *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_ঠেলো_lstring(ctx, dom_string_তথ্য(name), dom_string_দৈর্ঘ্য(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	যদি (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_বের_করো_2(ctx);
		/* ... node */
		dukky_ঠেলো_handler_code(ctx, name, et);
		/* ... node handlercode? */
		/* TODO: If this is null, clean up and propagate */
		/* ... node handlercode */
		/** @todo This is entirely wrong, but it's hard to get right */
		duk_ঠেলো_string(ctx, "function (event) {");
		/* ... node handlercode prefix */
		duk_ঢোকাও(ctx, -2);
		/* ... node prefix handlercode */
		duk_ঠেলো_string(ctx, "}");
		/* ... node prefix handlercode suffix */
		duk_concat(ctx, 3);
		/* ... node fullhandlersrc */
		duk_ঠেলো_string(ctx, "internal raw uncompiled handler");
		/* ... node fullhandlersrc filename */
		যদি (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_বের_করো_2(ctx);
			ফেরত false;
		}
		/* ... node handler */
		duk_ঢোকাও(ctx, -2);
		/* ... handler node */
	} অন্যথা {
		/* ... node handlers handler */
		duk_ঢোকাও(ctx, -3);
		/* ... handler node handlers */
		duk_বের_করো(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	ফেরত true;
}

স্থির শূন্য dukky_generic_event_handler(dom_event *evt, শূন্য *pw)
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
	যদি (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		ফেরত;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (পূর্ণসংখ্যা)dom_string_দৈর্ঘ্য(name),
	      dom_string_তথ্য(name));
	exc = dom_event_get_event_phase(evt, &phase);
	যদি (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		ফেরত;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_TARGET ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (পূর্ণসংখ্যা)phase);

	exc = dom_event_get_current_target(evt, &targ);
	যদি (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		ফেরত;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	যদি (phase == DOM_CAPTURING_PHASE)
		যাও handle_extras;

	/* ... */
	যদি (dukky_ঠেলো_গ্রন্থি(ctx, (dom_গ্রন্থি *)targ) == false) {
		dom_string_unref(name);
		dom_গ্রন্থি_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		ফেরত;
	}
	/* ... node */
	যদি (dukky_get_current_মান_of_event_handler(
		    ctx, name, (dom_event_target *)targ) == false) {
		/* ... */
		যাও handle_extras;
	}
	/* ... handler node */
	dukky_ঠেলো_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_start_time(ctx);
	যদি (duk_pcall_method(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_immediate_propagation(evt);
		যদি (exc != DOM_NO_ERR)
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

		duk_বের_করো_n(ctx, 6);
		/* ... */
		যাও handle_extras;
	}
	/* ... result */
	যদি (duk_is_boolean(ctx, -1) &&
	    duk_to_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_বের_করো(ctx);
handle_extras:
	/* ... */
	duk_ঠেলো_lstring(ctx, dom_string_তথ্য(name), dom_string_দৈর্ঘ্য(name));
	dukky_ঠেলো_গ্রন্থি(ctx, (dom_গ্রন্থি *)targ);
	/* ... type node */
	যদি (dukky_event_target_ঠেলো_listeners(ctx, true)) {
		/* Nothing to do */
		duk_বের_করো(ctx);
		যাও out;
	}
	/* ... sublisteners */
	duk_ঠেলো_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	যখন (duk_get_prop_সূচক(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_সূচক(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		যদি ((event_listener_flags)duk_to_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_বের_করো(ctx);
			/* ... subl copy handler flags */
		}
		duk_বের_করো(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_সূচক(ctx, -2, idx);
		/* ... sublisteners copy */
		idx++;
	}
	/* ... sublisteners copy undefined */
	duk_বের_করো(ctx);
	/* ... sublisteners copy */
	duk_ঢোকাও(ctx, -2);
	/* ... copy sublisteners */
	duk_বের_করো(ctx);
	/* ... copy */
	idx = 0;
	যখন (duk_get_prop_সূচক(ctx, -1, idx++)) {
		/* ... copy handler */
		যদি (duk_get_prop_সূচক(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_বের_করো_2(ctx);
			চালু;
		}
		duk_বের_করো(ctx);
		duk_get_prop_সূচক(ctx, -1, 0);
		duk_get_prop_সূচক(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_বের_করো(ctx);
		/* ... copy handler callback */
		যদি (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_বের_করো_2(ctx);
			/* ... copy */
			চালু;
		}
		/* ... copy handler callback */
		dukky_ঠেলো_গ্রন্থি(ctx, (dom_গ্রন্থি *)targ);
		/* ... copy handler callback node */
		dukky_ঠেলো_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_start_time(ctx);
		যদি (duk_pcall_method(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_immediate_propagation(evt);
			যদি (exc != DOM_NO_ERR)
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

			duk_বের_করো_n(ctx, 7);
			/* ... copy */
			চালু;
		}
		/* ... copy handler result */
		যদি (duk_is_boolean(ctx, -1) &&
		    duk_to_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_বের_করো_2(ctx);
		/* ... copy */
	}
	duk_বের_করো_2(ctx);
out:
	/* ... */
	dom_গ্রন্থি_unref(targ);
	dom_string_unref(name);
}

শূন্য dukky_register_event_listener_for(duk_context *ctx,
				       কাঠামো dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = ফাঁকা;
	dom_exception exc;

	/* ... */
	যদি (ele == ফাঁকা) {
		/* A null element is the Window object */
		duk_ঠেলো_global_object(ctx);
	} অন্যথা {
		/* Non null elements must be pushed as a node object */
		যদি (dukky_ঠেলো_গ্রন্থি(ctx, (কাঠামো dom_গ্রন্থি *)ele) == false)
			ফেরত;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_ঠেলো_lstring(ctx, dom_string_তথ্য(name), dom_string_দৈর্ঘ্য(name));
	/* ... node handlers name */
	যদি (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_বের_করো_2(ctx);
		/* ... */
		ফেরত;
	}
	/* ... node handlers */
	duk_ঠেলো_lstring(ctx, dom_string_তথ্য(name), dom_string_দৈর্ঘ্য(name));
	/* ... node handlers name */
	duk_ঠেলো_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_বের_করো_2(ctx);
	/* ... */
	যদি (ele == ফাঁকা) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		ফেরত;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	যদি (exc != DOM_NO_ERR) ফেরত;
	exc = dom_event_target_add_event_listener(
		ele, name, listen, capture);
	যদি (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (পূর্ণসংখ্যা)dom_string_দৈর্ঘ্য(name), dom_string_তথ্য(name));
	} অন্যথা {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (পূর্ণসংখ্যা)dom_string_দৈর্ঘ্য(name), dom_string_তথ্য(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_target_ঠেলো_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	যদি (duk_is_undefined(ctx, -1)) {
		/* ... type this null */
		duk_বের_করো(ctx);
		duk_ঠেলো_object(ctx);
		duk_dup(ctx, -1);
		/* ... type this listeners listeners */
		duk_put_prop_string(ctx, -3, EVENT_LISTENER_JS_MAGIC);
		/* ... type this listeners */
	}
	/* ... type this listeners */
	duk_ঢোকাও(ctx, -3);
	/* ... listeners type this */
	duk_বের_করো(ctx);
	/* ... listeners type */
	duk_dup(ctx, -1);
	/* ... listeners type type */
	duk_get_prop(ctx, -3);
	/* ... listeners type ??? */
	যদি (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		যদি (dont_create == true) {
			duk_বের_করো_3(ctx);
			duk_ঠেলো_undefined(ctx);
			ফেরত true;
		}
		duk_বের_করো(ctx);
		duk_ঠেলো_array(ctx);
		duk_dup(ctx, -2);
		duk_dup(ctx, -2);
		/* ... listeners type sublisteners type sublisteners */
		duk_put_prop(ctx, -5);
		/* ... listeners type sublisteners */
		ret = true;
	}
	duk_ঢোকাও(ctx, -3);
	/* ... sublisteners listeners type */
	duk_বের_করো_2(ctx);
	/* ... sublisteners */
	ফেরত ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
শূন্য dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	যখন (duk_get_prop_সূচক(ctx, -1, idx + 1)) {
		duk_put_prop_সূচক(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_সূচক(ctx, -2, idx + 1);
	duk_বের_করো(ctx);
}


শূন্য js_handle_new_element(jsthread *thread, কাঠামো dom_element *গ্রন্থি)
{
	প্রতিজ্ঞা(thread);
	প্রতিজ্ঞা(গ্রন্থি);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = ফাঁকা;
	dom_string *চাবি = ফাঁকা;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_গ্রন্থি_get_গ্রন্থি_name(গ্রন্থি, &nodename);
	যদি (exc != DOM_NO_ERR) ফেরত;

	যদি (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_গ্রন্থি_get_attributes(গ্রন্থি, &map);
	যদি (exc != DOM_NO_ERR) ফেরত;
	যদি (map == ফাঁকা) ফেরত;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_দৈর্ঘ্য(map, &siz);
	যদি (exc != DOM_NO_ERR) যাও out;

	জন্য (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_item(map, idx, &attr);
		যদি (exc != DOM_NO_ERR) যাও out;
		exc = dom_attr_get_name(attr, &চাবি);
		যদি (exc != DOM_NO_ERR) যাও out;
		যদি (is_body && (
			    চাবি == corestring_dom_onblur ||
			    চাবি == corestring_dom_onerror ||
			    চাবি == corestring_dom_onfocus ||
			    চাবি == corestring_dom_onload ||
			    চাবি == corestring_dom_onresize ||
			    চাবি == corestring_dom_onscroll)) {
			/* This is a forwarded event, it doesn't matter,
			 * we should skip registering for it and later
			 * we will register it for Window itself
			 */
			যাও skip_register;
		}
		যদি (dom_string_দৈর্ঘ্য(চাবি) > 2) {
			/* Can be on* */
			ধ্রুবক uint8_t *তথ্য = (ধ্রুবক uint8_t *)dom_string_তথ্য(চাবি);
			যদি (তথ্য[0] == 'o' && তথ্য[1] == 'n') {
				dom_string *sub = ফাঁকা;
				exc = dom_string_substr(
					চাবি, 2, dom_string_দৈর্ঘ্য(চাবি),
					&sub);
				যদি (exc == DOM_NO_ERR) {
					dukky_register_event_listener_for(
						CTX, গ্রন্থি, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_register:
		dom_string_unref(চাবি); চাবি = ফাঁকা;
		dom_গ্রন্থি_unref(attr); attr = ফাঁকা;
	}

out:
	যদি (চাবি != ফাঁকা)
		dom_string_unref(চাবি);

	যদি (attr != ফাঁকা)
		dom_গ্রন্থি_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

শূন্য js_event_cleanup(jsthread *thread, কাঠামো dom_event *evt)
{
	প্রতিজ্ঞা(thread);
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, EVENT_MAGIC);
	/* ... EVENT_MAP */
	duk_ঠেলো_pointer(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_বের_করো(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, ধ্রুবক অক্ষর *type, কাঠামো dom_document *doc, কাঠামো dom_গ্রন্থি *target)
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

	যদি (target != ফাঁকা)
		/* Swallow non-Window-targetted events quietly */
		ফেরত true;

	যদি (strcmp(type, "load") != 0)
		/* Swallow non-load events quietly */
		ফেরত true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	যদি (exc != DOM_NO_ERR) ফেরত true;
	exc = dom_event_init(evt, corestring_dom_load, false, false);
	যদি (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ফেরত true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_ঠেলো_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	যদি (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_বের_করো(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		যদি (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			ফেরত true;
		}
		dukky_ঠেলো_গ্রন্থি(CTX, (কাঠামো dom_গ্রন্থি *)body);
		/* ... handlers bodynode */
		যদি (dukky_get_current_মান_of_event_handler(
			    CTX, corestring_dom_load, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_গ্রন্থি_unref(body);
			/* ... handlers */
			duk_বের_করো(CTX);
			dukky_leave_thread(thread);
			ফেরত true;
		}
		/* Unref the body, we don't need it any more */
		dom_গ্রন্থি_unref(body);
		/* ... handlers handler bodynode */
		duk_বের_করো(CTX);
	}
	/* ... handlers handler */
	duk_ঢোকাও(CTX, -2);
	/* ... handler handlers */
	duk_বের_করো(CTX);
	/* ... handler */
	duk_ঠেলো_global_object(CTX);
	/* ... handler Window */
	dukky_ঠেলো_event(CTX, evt);
	/* ... handler Window event */
	dukky_reset_start_time(CTX);
	যদি (duk_pcall_method(CTX, 1) != 0) {
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

		duk_বের_করো_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		ফেরত true;
	}
	/* ... result */
	duk_বের_করো(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	ফেরত true;
}
