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

#تعریف_کرو EVENT_MAGIC MAGIC(EVENT_MAP)
#تعریف_کرو HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#تعریف_کرو HANDLER_MAGIC MAGIC(HANDLER_MAP)
#تعریف_کرو EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#تعریف_کرو GENERICS_MAGIC MAGIC(GENERICS_جدول)
#تعریف_کرو THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
ساخت jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t اگلا_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	غیرعلامتی صحیح زندہ_دھاگے; /**< number of live threads */
	uint64_t exec_آغاز_وقت;
};

/**
 * dukky javascript thread
 */
ساخت jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	غیرعلامتی صحیح اندر_use; /**< The number of times this thread is in use */
	jsheap *ڈھیر; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

ساکن duk_ret_t dukky_populate_object(duk_context *ctx, خالی *udata)
{
	/* ... obj args protoname nargs */
	صحیح nargs = duk_get_int(ctx, -1);
	duk_نکالو(ctx);
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
		duk_نکالو(ctx);
		duk_ڈالو_string(ctx, PROTO_NAME(HTMLUNKNOWNELEMENT));
		duk_get_prop(ctx, -2);
	}
	/* ... obj args protoname prototab proto */
	duk_remove(ctx, -3);
	/* ... obj args prototab proto */
	duk_dup(ctx, -1);
	/* ... obj args prototab proto proto */
	duk_سیٹ_prototype(ctx, -(nargs+4));
	/* ... obj[proto] args prototab proto */
	duk_get_prop_string(ctx, -1, INIT_MAGIC);
	/* ... obj[proto] args prototab proto initfn */
	duk_داخل_کرو(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_نکالو_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	واپس 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, ثابت حرف *name, صحیح دلائل)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, دلائل);
	/* ... args */
	duk_ڈالو_object(ctx);
	/* ... args obj */
	duk_ڈالو_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
	/* ... args obj */
	duk_ڈالو_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
	/* ... args obj */
	duk_داخل_کرو(ctx, -(دلائل+1));
	/* ... obj args */
	duk_ڈالو_string(ctx, name);
	/* ... obj args name */
	duk_ڈالو_int(ctx, دلائل);
	/* ... obj args name nargs */
	اگر ((ret = duk_محفوظ_call(ctx, dukky_populate_object, تہی, دلائل + 3, 1))
	    != DUK_EXEC_SUCCESS)
		واپس ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	واپس DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_ڈالو_گرہ_stacked(duk_context *ctx)
{
	صحیح چوٹی_at_fail = duk_get_چوٹی(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, گرہ_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	اگر (duk_is_undefined(ctx, -1)) {
		/* ... nodeptr klass nodes undefined */
		duk_نکالو(ctx);
		/* ... nodeptr klass nodes */
		duk_ڈالو_object(ctx);
		/* ... nodeptr klass nodes obj */
		duk_ڈالو_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_ڈالو_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr klass */
		duk_ڈالو_int(ctx, 1);
		/* ... nodeptr klass nodes obj nodeptr klass 1 */
		اگر (duk_محفوظ_call(ctx, dukky_populate_object, تہی, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_سیٹ_چوٹی(ctx, چوٹی_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			واپس false;
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
	duk_داخل_کرو(ctx, -4);
	/* ... node nodeptr klass nodes */
	duk_نکالو_3(ctx);
	/* ... node */
	اگر (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		ثابت حرف * what = duk_محفوظ_مقصد_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_نکالو(ctx);
	}
	واپس true;
}

#تعریف_کرو سیٹ_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

ساکن خالی dukky_html_element_class_from_لیبل_type(dom_html_element_type type,
		ثابت حرف **html_class, size_t *html_class_len)
{
	انتخاب(type) {
	حالت DOM_HTML_ELEMENT_TYPE_HTML:
		سیٹ_HTML_CLASS(HTML)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_سر:
		سیٹ_HTML_CLASS(سر)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_META:
		سیٹ_HTML_CLASS(META)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_بنیاد:
		سیٹ_HTML_CLASS(بنیاد_2)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_TITLE:
		سیٹ_HTML_CLASS(TITLE)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_BODY:
		سیٹ_HTML_CLASS(BODY)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_DIV:
		سیٹ_HTML_CLASS(DIV)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_FORM:
		سیٹ_HTML_CLASS(FORM)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_LINK:
		سیٹ_HTML_CLASS(LINK)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_BUTTON:
		سیٹ_HTML_CLASS(BUTTON)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_داخلہ:
		سیٹ_HTML_CLASS(داخلہ_2)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		سیٹ_HTML_CLASS(TEXTAREA)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		سیٹ_HTML_CLASS(OPTGROUP)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_OPTION:
		سیٹ_HTML_CLASS(OPTION)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_SELECT:
		سیٹ_HTML_CLASS(SELECT)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_HR:
		سیٹ_HTML_CLASS(HR)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_DL:
		سیٹ_HTML_CLASS(DLIST)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_DIR:
		سیٹ_HTML_CLASS(DIRECTORY)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_MENU:
		سیٹ_HTML_CLASS(MENU)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_FIELDSET:
		سیٹ_HTML_CLASS(FIELDSET)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_LEGEND:
		سیٹ_HTML_CLASS(LEGEND)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_P:
		سیٹ_HTML_CLASS(PARAGRAPH)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_H1:
	حالت DOM_HTML_ELEMENT_TYPE_H2:
	حالت DOM_HTML_ELEMENT_TYPE_H3:
	حالت DOM_HTML_ELEMENT_TYPE_H4:
	حالت DOM_HTML_ELEMENT_TYPE_H5:
	حالت DOM_HTML_ELEMENT_TYPE_H6:
		سیٹ_HTML_CLASS(HEADING)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	حالت DOM_HTML_ELEMENT_TYPE_Q:
		سیٹ_HTML_CLASS(QUOTE)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_PRE:
		سیٹ_HTML_CLASS(PRE)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_BR:
		سیٹ_HTML_CLASS(BR)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_LABEL:
		سیٹ_HTML_CLASS(LABEL)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_UL:
		سیٹ_HTML_CLASS(ULIST)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_OL:
		سیٹ_HTML_CLASS(OLIST)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_LI:
		سیٹ_HTML_CLASS(LI)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_FONT:
		سیٹ_HTML_CLASS(FONT)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_DEL:
	حالت DOM_HTML_ELEMENT_TYPE_INS:
		سیٹ_HTML_CLASS(MOD)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_A:
		سیٹ_HTML_CLASS(ANCHOR)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_BASEFONT:
		سیٹ_HTML_CLASS(BASEFONT)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_IMG:
		سیٹ_HTML_CLASS(IMAGE)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_OBJECT:
		سیٹ_HTML_CLASS(OBJECT)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_PARAM:
		سیٹ_HTML_CLASS(PARAM)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_APPLET:
		سیٹ_HTML_CLASS(APPLET)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_MAP:
		سیٹ_HTML_CLASS(MAP)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_AREA:
		سیٹ_HTML_CLASS(AREA)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_SCRIPT:
		سیٹ_HTML_CLASS(SCRIPT)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_CAPTION:
		سیٹ_HTML_CLASS(TABLECAPTION)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_TD:
	حالت DOM_HTML_ELEMENT_TYPE_TH:
		سیٹ_HTML_CLASS(TABLECELL)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_ستون:
	حالت DOM_HTML_ELEMENT_TYPE_COLGROUP:
		سیٹ_HTML_CLASS(TABLECOL)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_THEAD:
	حالت DOM_HTML_ELEMENT_TYPE_TBODY:
	حالت DOM_HTML_ELEMENT_TYPE_TFOOT:
		سیٹ_HTML_CLASS(TABLESECTION)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_جدول:
		سیٹ_HTML_CLASS(جدول_2)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_TR:
		سیٹ_HTML_CLASS(TABLEROW)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_STYLE:
		سیٹ_HTML_CLASS(STYLE)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_FRAMESET:
		سیٹ_HTML_CLASS(FRAMESET)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_چوکھٹا:
		سیٹ_HTML_CLASS(چوکھٹا_2)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_IFRAME:
		سیٹ_HTML_CLASS(IFRAME)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_ISINDEX:
		سیٹ_HTML_CLASS(ISINDEX)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_CANVAS:
		سیٹ_HTML_CLASS(CANVAS)
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_گنتی:
		أكد(type != DOM_HTML_ELEMENT_TYPE_گنتی);
		fallthrough;
	حالت DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		سیٹ_HTML_CLASS(UNKNOWN)
		توڑو;
	طےشدہ:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		توڑو;
	}
	واپس;
}

#تعریف_منسوخ سیٹ_HTML_CLASS

ساکن خالی
dukky_ڈالو_گرہ_klass(duk_context *ctx, ساخت dom_گرہ *گرہ)
{
	dom_گرہ_type nodetype;
	dom_exception err;

	err = dom_گرہ_get_گرہ_type(گرہ, &nodetype);
	اگر (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_ڈالو_string(ctx, PROTO_NAME(گرہ_2));
		واپس;
	}

	انتخاب(nodetype) {
	حالت DOM_ELEMENT_گرہ: {
		dom_string *namespace;
		dom_html_element_type type;
		ثابت حرف *html_class;
		size_t html_class_len;
		err = dom_گرہ_get_namespace(گرہ, &namespace);
		اگر (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_ڈالو_string(ctx, PROTO_NAME(ELEMENT));
			توڑو;
		}
		اگر (namespace == تہی) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_ڈالو_string(ctx, PROTO_NAME(ELEMENT));
			توڑو;
		}

		اگر (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_ڈالو_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			توڑو;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_لیبل_type(گرہ, &type);
		اگر (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_لیبل_type(type,
				&html_class, &html_class_len);

		duk_ڈالو_lstring(ctx, html_class, html_class_len);
		توڑو;
	}
	حالت DOM_متن_گرہ:
		duk_ڈالو_string(ctx, PROTO_NAME(متن_2));
		توڑو;
	حالت DOM_COMMENT_گرہ:
		duk_ڈالو_string(ctx, PROTO_NAME(COMMENT));
		توڑو;
	حالت DOM_DOCUMENT_گرہ:
		duk_ڈالو_string(ctx, PROTO_NAME(DOCUMENT));
		توڑو;
	حالت DOM_ATTRIBUTE_گرہ:
	حالت DOM_PROCESSING_INSTRUCTION_گرہ:
	حالت DOM_DOCUMENT_TYPE_گرہ:
	حالت DOM_DOCUMENT_FRAGMENT_گرہ:
	حالت DOM_NOTATION_گرہ:
	حالت DOM_ENTITY_REFERENCE_گرہ:
	حالت DOM_ENTITY_گرہ:
	حالت DOM_CDATA_SECTION_گرہ:
	طےشدہ:
		/* Oh bum, just node then */
		duk_ڈالو_string(ctx, PROTO_NAME(گرہ_2));
	}
}

duk_bool_t
dukky_ڈالو_گرہ(duk_context *ctx, ساخت dom_گرہ *گرہ)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", گرہ);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, گرہ_MAGIC);
	/* ... nodes */
	duk_ڈالو_اشارہ_گر(ctx, گرہ);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	اگر (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_داخل_کرو(ctx, -2);
		/* ... node nodes */
		duk_نکالو(ctx);
		/* ... node */
		اگر (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			ثابت حرف * what = duk_محفوظ_مقصد_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_نکالو(ctx);
		}
		واپس true;
	}
	/* ... nodes undefined */
	duk_نکالو_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_ڈالو_اشارہ_گر(ctx, گرہ);
	/* ... nodeptr */
	dukky_ڈالو_گرہ_klass(ctx, گرہ);
	/* ... nodeptr klass */
	واپس dukky_ڈالو_گرہ_stacked(ctx);
}

ساکن duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	واپس duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

خالی
dukky_inject_not_ctr(duk_context *ctx, صحیح idx, ثابت حرف *name)
{
	/* ... p[idx] ... proto */
	duk_ڈالو_c_function(ctx, dukky_bad_constructor, 0);
	/* ... p[idx] ... proto cons */
	duk_داخل_کرو(ctx, -2);
	/* ... p[idx] ... cons proto */
	duk_put_prop_string(ctx, -2, "prototype");
	/* ... p[idx] ... cons[proto] */
	duk_put_prop_string(ctx, idx, name);
	/* ... p ... */
	واپس;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

ساکن خالی *dukky_alloc_function(خالی *udata, duk_سائز_t سائز)
{
	اگر (سائز == 0)
		واپس تہی;

	واپس حافظہ_دو(سائز);
}

ساکن خالی *dukky_realloc_function(خالی *udata, خالی *ptr, duk_سائز_t سائز)
{
	اگر (ptr == تہی && سائز == 0)
		واپس تہی;

	اگر (سائز == 0) {
		آزاد(ptr);
		واپس تہی;
	}

	واپس realloc(ptr, سائز);
}


ساکن خالی dukky_free_function(خالی *udata, خالی *ptr)
{
	اگر (ptr != تہی)
		آزاد(ptr);
}

/* exported interface documented in js.h */
خالی js_initialise(خالی)
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
خالی js_finalise(خالی)
{
	/* NADA for now */
}


/* exported interface documented in js.h */
nserror
js_newheap(صحیح timeout, jsheap **ڈھیر)
{
	duk_context *ctx;
	jsheap *ret = صفر_حافظہ_دو(1, حجم(*ret));
	*ڈھیر = تہی;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	اگر (ret == تہی) واپس NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_ڈھیر(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		تہی);
	اگر (ret->ctx == تہی) { آزاد(ret); واپس NSERROR_NOMEM; }
	/* Create the prototype stuffs */
	duk_ڈالو_global_object(ctx);
	duk_ڈالو_boolean(ctx, true);
	duk_put_prop_string(ctx, -2, "protos");
	duk_put_global_string(ctx, PROTO_MAGIC);
	/* Create prototypes here */
	dukky_create_prototypes(ctx);
	/* Now create the thread map */
	duk_ڈالو_object(ctx);
	duk_put_global_string(ctx, THREAD_MAP);

	*ڈھیر = ret;
	واپس NSERROR_OK;
}


ساکن خالی dukky_destroyheap(jsheap *ڈھیر)
{
	أكد(ڈھیر->pending_destroy == true);
	أكد(ڈھیر->زندہ_دھاگے == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_ڈھیر(ڈھیر->ctx);
	آزاد(ڈھیر);
}

/* exported interface documented in js.h */
خالی js_destroyheap(jsheap *ڈھیر)
{
	ڈھیر->pending_destroy = true;
	اگر (ڈھیر->زندہ_دھاگے == 0) {
		dukky_destroyheap(ڈھیر);
	}
}

/* Just for here, the CTX is in ret, not thread */
#تعریف_کرو CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *ڈھیر, خالی *win_priv, خالی *doc_priv, jsthread **thread)
{
	jsthread *ret;
	أكد(ڈھیر != تہی);
	أكد(ڈھیر->pending_destroy == false);

	ret = صفر_حافظہ_دو(1, حجم (*ret));
	اگر (ret == تہی) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		واپس NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(ڈھیر->ctx, THREAD_MAP); /* ... threads */
	duk_ڈالو_thread(ڈھیر->ctx); /* ... threads thread */
	ret->ڈھیر = ڈھیر;
	ret->ctx = duk_require_context(ڈھیر->ctx, -1);
	ret->thread_idx = ڈھیر->اگلا_thread++;
	duk_put_prop_اشاریہ(ڈھیر->ctx, -2, ret->thread_idx);
	ڈھیر->زندہ_دھاگے++;
	duk_نکالو(ڈھیر->ctx); /* ... */
	duk_ڈالو_int(CTX, 0);
	duk_ڈالو_int(CTX, 1);
	duk_ڈالو_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_ڈالو_اشارہ_گر(CTX, win_priv);
	duk_ڈالو_اشارہ_گر(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(کھڑکی), 2);
	duk_ڈالو_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_سیٹ_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_ڈالو_object(CTX);
	duk_ڈالو_اشارہ_گر(CTX, تہی);
	duk_ڈالو_null(CTX);
	duk_put_prop(CTX, -3);
	duk_put_global_string(CTX, گرہ_MAGIC);

	/* And now the event mapping table */
	duk_ڈالو_object(CTX);
	duk_put_global_string(CTX, EVENT_MAGIC);

	/* Now load the polyfills */
	/* ... */
	duk_ڈالو_string(CTX, "polyfill.js");
	/* ..., polyfill.js */
	اگر (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ثابت حرف *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_محفوظ_مقصد_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		واپس NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	اگر (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		واپس NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_نکالو(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_ڈالو_string(CTX, "generics.js");
	/* ..., generics.js */
	اگر (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ثابت حرف *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_محفوظ_مقصد_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		واپس NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	اگر (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		واپس NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_نکالو(CTX);
	/* ... */
	duk_ڈالو_global_object(CTX);
	/* ..., Win */
	duk_get_prop_string(CTX, -1, "NetSurf");
	/* ..., Win, NetSurf */
	duk_put_global_string(CTX, GENERICS_MAGIC);
	/* ..., Win */
	duk_del_prop_string(CTX, -1, "NetSurf");
	duk_نکالو(CTX);
	/* ... */

	dukky_log_ڈھیر_چوکھٹا(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, ڈھیر);
	*thread = ret;

	واپس NSERROR_OK;
}

/* Now switch to the long term CTX behaviour */
#تعریف_منسوخ CTX
#تعریف_کرو CTX (thread->ctx)

/* exported interface documented in js.h */
nserror js_closethread(jsthread *thread)
{
	/* We can always close down a thread, it might just confuse
	 * the code running, though we don't mind since we're in the
	 * process of destruction at this point
	 */
	duk_int_t چوٹی = duk_get_چوٹی(CTX);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, thread->ڈھیر);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Restore whatever stack we had */
	duk_سیٹ_چوٹی(CTX, چوٹی);

	واپس NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
ساکن خالی dukky_destroythread(jsthread *thread)
{
	jsheap *ڈھیر = thread->ڈھیر;

	أكد(thread->اندر_use == 0);
	أكد(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, ڈھیر);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(ڈھیر->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_اشاریہ(ڈھیر->ctx, -1, thread->thread_idx);
	duk_نکالو(ڈھیر->ctx); /* ... */

	/* We can now free the thread object */
	آزاد(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(ڈھیر->ctx, 0);
	duk_gc(ڈھیر->ctx, DUK_GC_COMPACT);
	ڈھیر->زندہ_دھاگے--;

	/* And if the heap should now go, blow it away */
	اگر (ڈھیر->pending_destroy == true && ڈھیر->زندہ_دھاگے == 0) {
		dukky_destroyheap(ڈھیر);
	}
}

/* exported interface documented in js.h */
خالی js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	اگر (thread->اندر_use == 0) {
		dukky_destroythread(thread);
	}
}

ساکن خالی dukky_enter_thread(jsthread *thread)
{
	أكد(thread != تہی);
	thread->اندر_use++;
}

ساکن خالی dukky_leave_thread(jsthread *thread)
{
	أكد(thread != تہی);
	أكد(thread->اندر_use > 0);

	thread->اندر_use--;
	اگر (thread->اندر_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(خالی *udata)
{
#تعریف_کرو JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *ڈھیر = (jsheap *) udata;
	uint64_t now;

	(خالی) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	واپس ڈھیر->exec_آغاز_وقت != 0 &&
			now > (ڈھیر->exec_آغاز_وقت + JS_EXEC_TIMEOUT_MS);
}

ساکن خالی dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_چوٹی(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_محفوظ_مقصد_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_نکالو(ctx);
	/* ..., errobj */
}

ساکن خالی dukky_reset_آغاز_وقت(duk_context *ctx)
{
	duk_حافظہ_functions funcs;
	jsheap *ڈھیر;
	duk_get_حافظہ_functions(ctx, &funcs);
	ڈھیر = funcs.udata;
	(خالی) nsu_getmonotonic_ms(&ڈھیر->exec_آغاز_وقت);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_سائز_t argc, bool reset_timeout)
{
	اگر (reset_timeout) {
		dukky_reset_آغاز_وقت(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	اگر (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	واپس ret;
}


خالی dukky_ڈالو_generics(duk_context *ctx, ثابت حرف *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

ساکن duk_int_t dukky_ڈالو_context_dump(duk_context *ctx, خالی *udata)
{
	duk_ڈالو_context_dump(ctx);
	واپس 1;
}

خالی dukky_log_ڈھیر_چوکھٹا(duk_context *ctx, ثابت حرف * reason)
{
	اگر (duk_محفوظ_call(ctx, dukky_ڈالو_context_dump, تہی, 0, 1) != 0) {
		duk_نکالو(ctx);
		duk_ڈالو_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_محفوظ_مقصد_string(ctx, -1));
	duk_نکالو(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, ثابت uint8_t *txt, size_t txtlen, ثابت حرف *name)
{
	bool ret = false;
	أكد(thread);

	اگر (txt == تہی || txtlen == 0) {
		واپس false;
	}

	اگر (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		واپس false;
	}

	dukky_enter_thread(thread);

	duk_سیٹ_چوٹی(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_آغاز_وقت(CTX);
	اگر (name != تہی) {
		duk_ڈالو_string(CTX, name);
	} ورنہ {
		duk_ڈالو_string(CTX, "?unknown source?");
	}
	اگر (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (ثابت حرف *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		جاؤ handle_error;
	}

	اگر (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		جاؤ handle_error;
	}

	اگر (duk_get_چوٹی(CTX) == 0) duk_ڈالو_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	جاؤ برآمد;

handle_error:
	dukky_dump_error(CTX);
برآمد:
	dukky_leave_thread(thread);
	واپس ret;
}

ساکن ثابت حرف* dukky_event_proto(dom_event *evt)
{
	ثابت حرف *ret = PROTO_NAME(EVENT);
	dom_string *type = تہی;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	اگر (err != DOM_NO_ERR) {
		جاؤ برآمد;
	}

	اگر (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		جاؤ برآمد;
	} ورنہ اگر (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		جاؤ برآمد;
	} ورنہ اگر (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		جاؤ برآمد;
	}

برآمد:
	اگر (type != تہی) {
		dom_string_unref(type);
	}

	واپس ret;
}

/*** New style event handling ***/

خالی dukky_ڈالو_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_ڈالو_اشارہ_گر(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	اگر (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_نکالو(ctx);
		/* ... events */
		duk_ڈالو_اشارہ_گر(ctx, evt);
		اگر (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_نکالو(ctx);
			/* ... events */
			duk_ڈالو_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_ڈالو_اشارہ_گر(ctx, evt);
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

ساکن خالی dukky_ڈالو_handler_رمز(duk_context *ctx, dom_string *name,
				     dom_event_ہدف *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_گرہ_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	اگر (et == تہی) {
		duk_ڈالو_lstring(ctx, "", 0);
		واپس;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_گرہ_get_گرہ_type(et, &ntype);
	اگر (exc != DOM_NO_ERR) {
		duk_ڈالو_lstring(ctx, "", 0);
		واپس;
	}

	اگر (ntype != DOM_ELEMENT_گرہ) {
		duk_ڈالو_lstring(ctx, "", 0);
		واپس;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	اگر (exc != DOM_NO_ERR) {
		duk_ڈالو_lstring(ctx, "", 0);
		واپس;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	اگر ((exc != DOM_NO_ERR) || (val == تہی)) {
		dom_string_unref(onname);
		duk_ڈالو_lstring(ctx, "", 0);
		واپس;
	}

	dom_string_unref(onname);
	duk_ڈالو_lstring(ctx, dom_string_ڈیٹا(val), dom_string_لمبائی(val));
	dom_string_unref(val);
}

bool dukky_get_current_قدر_کا_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_ہدف *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_ڈالو_lstring(ctx, dom_string_ڈیٹا(name), dom_string_لمبائی(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	اگر (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_نکالو_2(ctx);
		/* ... node */
		dukky_ڈالو_handler_رمز(ctx, name, et);
		/* ... node handlercode? */
		/* TODO: If this is null, clean up and propagate */
		/* ... node handlercode */
		/** @todo This is entirely wrong, but it's hard to get right */
		duk_ڈالو_string(ctx, "function (event) {");
		/* ... node handlercode prefix */
		duk_داخل_کرو(ctx, -2);
		/* ... node prefix handlercode */
		duk_ڈالو_string(ctx, "}");
		/* ... node prefix handlercode suffix */
		duk_concat(ctx, 3);
		/* ... node fullhandlersrc */
		duk_ڈالو_string(ctx, "internal raw uncompiled handler");
		/* ... node fullhandlersrc filename */
		اگر (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_نکالو_2(ctx);
			واپس false;
		}
		/* ... node handler */
		duk_داخل_کرو(ctx, -2);
		/* ... handler node */
	} ورنہ {
		/* ... node handlers handler */
		duk_داخل_کرو(ctx, -3);
		/* ... handler node handlers */
		duk_نکالو(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	واپس true;
}

ساکن خالی dukky_generic_event_handler(dom_event *evt, خالی *pw)
{
	duk_context *ctx = (duk_context *)pw;
	dom_string *name;
	dom_exception exc;
	dom_event_ہدف *targ;
	dom_event_flow_phase phase;
	duk_uarridx_t idx;
	event_listener_flags flags;

	NSLOG(dukky, DEBUG, "Handling an event in duktape interface...");
	exc = dom_event_get_type(evt, &name);
	اگر (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		واپس;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (صحیح)dom_string_لمبائی(name),
	      dom_string_ڈیٹا(name));
	exc = dom_event_get_event_phase(evt, &phase);
	اگر (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		واپس;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_ہدف ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (صحیح)phase);

	exc = dom_event_get_current_ہدف(evt, &targ);
	اگر (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		واپس;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	اگر (phase == DOM_CAPTURING_PHASE)
		جاؤ handle_extras;

	/* ... */
	اگر (dukky_ڈالو_گرہ(ctx, (dom_گرہ *)targ) == false) {
		dom_string_unref(name);
		dom_گرہ_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		واپس;
	}
	/* ... node */
	اگر (dukky_get_current_قدر_کا_event_handler(
		    ctx, name, (dom_event_ہدف *)targ) == false) {
		/* ... */
		جاؤ handle_extras;
	}
	/* ... handler node */
	dukky_ڈالو_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_آغاز_وقت(ctx);
	اگر (duk_pcall_طریقہ(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_فوری_اشاعت(evt);
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
		      duk_محفوظ_مقصد_string(ctx, -5),
		      duk_محفوظ_مقصد_string(ctx, -4));
		NSLOG(dukky, INFO, "              was at: %s line %s",
		      duk_محفوظ_مقصد_string(ctx, -3),
		      duk_محفوظ_مقصد_string(ctx, -2));
		NSLOG(dukky, INFO, "         Stack trace: %s",
		      duk_محفوظ_مقصد_string(ctx, -1));

		duk_نکالو_n(ctx, 6);
		/* ... */
		جاؤ handle_extras;
	}
	/* ... result */
	اگر (duk_is_boolean(ctx, -1) &&
	    duk_مقصد_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_نکالو(ctx);
handle_extras:
	/* ... */
	duk_ڈالو_lstring(ctx, dom_string_ڈیٹا(name), dom_string_لمبائی(name));
	dukky_ڈالو_گرہ(ctx, (dom_گرہ *)targ);
	/* ... type node */
	اگر (dukky_event_ہدف_ڈالو_listeners(ctx, true)) {
		/* Nothing to do */
		duk_نکالو(ctx);
		جاؤ برآمد;
	}
	/* ... sublisteners */
	duk_ڈالو_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	جب (duk_get_prop_اشاریہ(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_اشاریہ(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		اگر ((event_listener_flags)duk_مقصد_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_نکالو(ctx);
			/* ... subl copy handler flags */
		}
		duk_نکالو(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_اشاریہ(ctx, -2, idx);
		/* ... sublisteners copy */
		idx++;
	}
	/* ... sublisteners copy undefined */
	duk_نکالو(ctx);
	/* ... sublisteners copy */
	duk_داخل_کرو(ctx, -2);
	/* ... copy sublisteners */
	duk_نکالو(ctx);
	/* ... copy */
	idx = 0;
	جب (duk_get_prop_اشاریہ(ctx, -1, idx++)) {
		/* ... copy handler */
		اگر (duk_get_prop_اشاریہ(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_نکالو_2(ctx);
			جاری;
		}
		duk_نکالو(ctx);
		duk_get_prop_اشاریہ(ctx, -1, 0);
		duk_get_prop_اشاریہ(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_نکالو(ctx);
		/* ... copy handler callback */
		اگر (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_نکالو_2(ctx);
			/* ... copy */
			جاری;
		}
		/* ... copy handler callback */
		dukky_ڈالو_گرہ(ctx, (dom_گرہ *)targ);
		/* ... copy handler callback node */
		dukky_ڈالو_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_آغاز_وقت(ctx);
		اگر (duk_pcall_طریقہ(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_فوری_اشاعت(evt);
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
			      duk_محفوظ_مقصد_string(ctx, -5),
			      duk_محفوظ_مقصد_string(ctx, -4));
			NSLOG(dukky, DEBUG,
			      "              was at: %s line %s",
			      duk_محفوظ_مقصد_string(ctx, -3),
			      duk_محفوظ_مقصد_string(ctx, -2));
			NSLOG(dukky, DEBUG, "         Stack trace: %s",
			      duk_محفوظ_مقصد_string(ctx, -1));

			duk_نکالو_n(ctx, 7);
			/* ... copy */
			جاری;
		}
		/* ... copy handler result */
		اگر (duk_is_boolean(ctx, -1) &&
		    duk_مقصد_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_نکالو_2(ctx);
		/* ... copy */
	}
	duk_نکالو_2(ctx);
برآمد:
	/* ... */
	dom_گرہ_unref(targ);
	dom_string_unref(name);
}

خالی dukky_رجسٹر_event_listener_for(duk_context *ctx,
				       ساخت dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = تہی;
	dom_exception exc;

	/* ... */
	اگر (ele == تہی) {
		/* A null element is the Window object */
		duk_ڈالو_global_object(ctx);
	} ورنہ {
		/* Non null elements must be pushed as a node object */
		اگر (dukky_ڈالو_گرہ(ctx, (ساخت dom_گرہ *)ele) == false)
			واپس;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_ڈالو_lstring(ctx, dom_string_ڈیٹا(name), dom_string_لمبائی(name));
	/* ... node handlers name */
	اگر (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_نکالو_2(ctx);
		/* ... */
		واپس;
	}
	/* ... node handlers */
	duk_ڈالو_lstring(ctx, dom_string_ڈیٹا(name), dom_string_لمبائی(name));
	/* ... node handlers name */
	duk_ڈالو_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_نکالو_2(ctx);
	/* ... */
	اگر (ele == تہی) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		واپس;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	اگر (exc != DOM_NO_ERR) واپس;
	exc = dom_event_ہدف_جوڑو_event_listener(
		ele, name, listen, capture);
	اگر (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (صحیح)dom_string_لمبائی(name), dom_string_ڈیٹا(name));
	} ورنہ {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (صحیح)dom_string_لمبائی(name), dom_string_ڈیٹا(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_ہدف_ڈالو_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	اگر (duk_is_undefined(ctx, -1)) {
		/* ... type this null */
		duk_نکالو(ctx);
		duk_ڈالو_object(ctx);
		duk_dup(ctx, -1);
		/* ... type this listeners listeners */
		duk_put_prop_string(ctx, -3, EVENT_LISTENER_JS_MAGIC);
		/* ... type this listeners */
	}
	/* ... type this listeners */
	duk_داخل_کرو(ctx, -3);
	/* ... listeners type this */
	duk_نکالو(ctx);
	/* ... listeners type */
	duk_dup(ctx, -1);
	/* ... listeners type type */
	duk_get_prop(ctx, -3);
	/* ... listeners type ??? */
	اگر (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		اگر (dont_create == true) {
			duk_نکالو_3(ctx);
			duk_ڈالو_undefined(ctx);
			واپس true;
		}
		duk_نکالو(ctx);
		duk_ڈالو_array(ctx);
		duk_dup(ctx, -2);
		duk_dup(ctx, -2);
		/* ... listeners type sublisteners type sublisteners */
		duk_put_prop(ctx, -5);
		/* ... listeners type sublisteners */
		ret = true;
	}
	duk_داخل_کرو(ctx, -3);
	/* ... sublisteners listeners type */
	duk_نکالو_2(ctx);
	/* ... sublisteners */
	واپس ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
خالی dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	جب (duk_get_prop_اشاریہ(ctx, -1, idx + 1)) {
		duk_put_prop_اشاریہ(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_اشاریہ(ctx, -2, idx + 1);
	duk_نکالو(ctx);
}


خالی js_handle_new_element(jsthread *thread, ساخت dom_element *گرہ)
{
	أكد(thread);
	أكد(گرہ);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = تہی;
	dom_string *کلید = تہی;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_گرہ_get_گرہ_name(گرہ, &nodename);
	اگر (exc != DOM_NO_ERR) واپس;

	اگر (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_گرہ_get_اوصاف(گرہ, &map);
	اگر (exc != DOM_NO_ERR) واپس;
	اگر (map == تہی) واپس;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_لمبائی(map, &siz);
	اگر (exc != DOM_NO_ERR) جاؤ برآمد;

	برائے (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_شے(map, idx, &attr);
		اگر (exc != DOM_NO_ERR) جاؤ برآمد;
		exc = dom_attr_get_name(attr, &کلید);
		اگر (exc != DOM_NO_ERR) جاؤ برآمد;
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
			جاؤ skip_رجسٹر;
		}
		اگر (dom_string_لمبائی(کلید) > 2) {
			/* Can be on* */
			ثابت uint8_t *ڈیٹا = (ثابت uint8_t *)dom_string_ڈیٹا(کلید);
			اگر (ڈیٹا[0] == 'o' && ڈیٹا[1] == 'n') {
				dom_string *sub = تہی;
				exc = dom_string_substr(
					کلید, 2, dom_string_لمبائی(کلید),
					&sub);
				اگر (exc == DOM_NO_ERR) {
					dukky_رجسٹر_event_listener_for(
						CTX, گرہ, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_رجسٹر:
		dom_string_unref(کلید); کلید = تہی;
		dom_گرہ_unref(attr); attr = تہی;
	}

برآمد:
	اگر (کلید != تہی)
		dom_string_unref(کلید);

	اگر (attr != تہی)
		dom_گرہ_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

خالی js_event_cleanup(jsthread *thread, ساخت dom_event *evt)
{
	أكد(thread);
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, EVENT_MAGIC);
	/* ... EVENT_MAP */
	duk_ڈالو_اشارہ_گر(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_نکالو(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, ثابت حرف *type, ساخت dom_document *doc, ساخت dom_گرہ *ہدف)
{
	dom_exception exc;
	dom_event *evt;
	dom_event_ہدف *body;

	NSLOG(dukky, DEBUG, "Event: %s (doc=%p, target=%p)", type, doc,
	      ہدف);

	/** @todo Make this more generic, this only handles load and only
	 * targetting the window, so that we actually stand a chance of
	 * getting 3.4 out.
	 */

	اگر (ہدف != تہی)
		/* Swallow non-Window-targetted events quietly */
		واپس true;

	اگر (strcmp(type, "load") != 0)
		/* Swallow non-load events quietly */
		واپس true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	اگر (exc != DOM_NO_ERR) واپس true;
	exc = dom_event_init(evt, corestring_dom_بھرو, false, false);
	اگر (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		واپس true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_ڈالو_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	اگر (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_نکالو(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		اگر (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			واپس true;
		}
		dukky_ڈالو_گرہ(CTX, (ساخت dom_گرہ *)body);
		/* ... handlers bodynode */
		اگر (dukky_get_current_قدر_کا_event_handler(
			    CTX, corestring_dom_بھرو, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_گرہ_unref(body);
			/* ... handlers */
			duk_نکالو(CTX);
			dukky_leave_thread(thread);
			واپس true;
		}
		/* Unref the body, we don't need it any more */
		dom_گرہ_unref(body);
		/* ... handlers handler bodynode */
		duk_نکالو(CTX);
	}
	/* ... handlers handler */
	duk_داخل_کرو(CTX, -2);
	/* ... handler handlers */
	duk_نکالو(CTX);
	/* ... handler */
	duk_ڈالو_global_object(CTX);
	/* ... handler Window */
	dukky_ڈالو_event(CTX, evt);
	/* ... handler Window event */
	dukky_reset_آغاز_وقت(CTX);
	اگر (duk_pcall_طریقہ(CTX, 1) != 0) {
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
		      duk_محفوظ_مقصد_string(CTX, -5),
		      duk_محفوظ_مقصد_string(CTX, -4));
		NSLOG(dukky, DEBUG, "              was at: %s line %s",
		      duk_محفوظ_مقصد_string(CTX, -3),
		      duk_محفوظ_مقصد_string(CTX, -2));
		NSLOG(dukky, DEBUG, "         Stack trace: %s",
		      duk_محفوظ_مقصد_string(CTX, -1));

		duk_نکالو_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		واپس true;
	}
	/* ... result */
	duk_نکالو(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	واپس true;
}
