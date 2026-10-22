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

#تعریف_کرو EVENT_MAGIC MAGIC(EVENT_MAP)
#تعریف_کرو HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#تعریف_کرو HANDLER_MAGIC MAGIC(HANDLER_MAP)
#تعریف_کرو EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#تعریف_کرو GENERICS_MAGIC MAGIC(GENERICS_جدول)
#تعریف_کرو THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
بنية jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t اگلا_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	بنانشانی صحيح جیوندا_دھاگے; /**< number of live threads */
	uint64_t exec_شروع_ویلا;
};

/**
 * dukky javascript thread
 */
بنية jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	بنانشانی صحيح وچ_use; /**< The number of times this thread is in use */
	jsheap *ڈھیر; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

ساكن duk_ret_t dukky_populate_object(duk_context *ctx, فراغ *udata)
{
	/* ... obj args protoname nargs */
	صحيح nargs = duk_get_int(ctx, -1);
	duk_نکالو(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	إذا (duk_is_undefined(ctx, -1)) {
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
	أرجع 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, ثابت حرف *name, صحيح دلیلاں)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, دلیلاں);
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
	duk_داخل_کرو(ctx, -(دلیلاں+1));
	/* ... obj args */
	duk_ڈالو_string(ctx, name);
	/* ... obj args name */
	duk_ڈالو_int(ctx, دلیلاں);
	/* ... obj args name nargs */
	إذا ((ret = duk_محفوظ_call(ctx, dukky_populate_object, عدم, دلیلاں + 3, 1))
	    != DUK_EXEC_SUCCESS)
		أرجع ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	أرجع DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_ڈالو_گرہ_stacked(duk_context *ctx)
{
	صحيح چوٹی_at_fail = duk_get_چوٹی(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, گرہ_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	إذا (duk_is_undefined(ctx, -1)) {
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
		إذا (duk_محفوظ_call(ctx, dukky_populate_object, عدم, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_سیٹ_چوٹی(ctx, چوٹی_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			أرجع false;
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
	إذا (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		ثابت حرف * what = duk_محفوظ_منزل_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_نکالو(ctx);
	}
	أرجع true;
}

#تعریف_کرو سیٹ_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

ساكن فراغ dukky_html_element_class_from_ٹیگ_type(dom_html_element_type type,
		ثابت حرف **html_class, size_t *html_class_len)
{
	بدل(type) {
	حالة DOM_HTML_ELEMENT_TYPE_HTML:
		سیٹ_HTML_CLASS(HTML)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_سر:
		سیٹ_HTML_CLASS(سر)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_META:
		سیٹ_HTML_CLASS(META)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_ادھار:
		سیٹ_HTML_CLASS(ادھار_2)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_TITLE:
		سیٹ_HTML_CLASS(TITLE)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_BODY:
		سیٹ_HTML_CLASS(BODY)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_DIV:
		سیٹ_HTML_CLASS(DIV)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_FORM:
		سیٹ_HTML_CLASS(FORM)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_LINK:
		سیٹ_HTML_CLASS(LINK)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_BUTTON:
		سیٹ_HTML_CLASS(BUTTON)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_داخلہ:
		سیٹ_HTML_CLASS(داخلہ_2)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		سیٹ_HTML_CLASS(TEXTAREA)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		سیٹ_HTML_CLASS(OPTGROUP)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_OPTION:
		سیٹ_HTML_CLASS(OPTION)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_SELECT:
		سیٹ_HTML_CLASS(SELECT)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_HR:
		سیٹ_HTML_CLASS(HR)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_DL:
		سیٹ_HTML_CLASS(DLIST)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_DIR:
		سیٹ_HTML_CLASS(DIRECTORY)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_MENU:
		سیٹ_HTML_CLASS(MENU)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_FIELDSET:
		سیٹ_HTML_CLASS(FIELDSET)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_LEGEND:
		سیٹ_HTML_CLASS(LEGEND)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_P:
		سیٹ_HTML_CLASS(PARAGRAPH)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_H1:
	حالة DOM_HTML_ELEMENT_TYPE_H2:
	حالة DOM_HTML_ELEMENT_TYPE_H3:
	حالة DOM_HTML_ELEMENT_TYPE_H4:
	حالة DOM_HTML_ELEMENT_TYPE_H5:
	حالة DOM_HTML_ELEMENT_TYPE_H6:
		سیٹ_HTML_CLASS(HEADING)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	حالة DOM_HTML_ELEMENT_TYPE_Q:
		سیٹ_HTML_CLASS(QUOTE)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_PRE:
		سیٹ_HTML_CLASS(PRE)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_BR:
		سیٹ_HTML_CLASS(BR)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_LABEL:
		سیٹ_HTML_CLASS(LABEL)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_UL:
		سیٹ_HTML_CLASS(ULIST)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_OL:
		سیٹ_HTML_CLASS(OLIST)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_LI:
		سیٹ_HTML_CLASS(LI)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_FONT:
		سیٹ_HTML_CLASS(FONT)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_DEL:
	حالة DOM_HTML_ELEMENT_TYPE_INS:
		سیٹ_HTML_CLASS(MOD)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_A:
		سیٹ_HTML_CLASS(ANCHOR)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_BASEFONT:
		سیٹ_HTML_CLASS(BASEFONT)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_IMG:
		سیٹ_HTML_CLASS(IMAGE)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_OBJECT:
		سیٹ_HTML_CLASS(OBJECT)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_PARAM:
		سیٹ_HTML_CLASS(PARAM)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_APPLET:
		سیٹ_HTML_CLASS(APPLET)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_MAP:
		سیٹ_HTML_CLASS(MAP)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_AREA:
		سیٹ_HTML_CLASS(AREA)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_SCRIPT:
		سیٹ_HTML_CLASS(SCRIPT)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_CAPTION:
		سیٹ_HTML_CLASS(TABLECAPTION)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_TD:
	حالة DOM_HTML_ELEMENT_TYPE_TH:
		سیٹ_HTML_CLASS(TABLECELL)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_خانہ:
	حالة DOM_HTML_ELEMENT_TYPE_COLGROUP:
		سیٹ_HTML_CLASS(TABLECOL)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_THEAD:
	حالة DOM_HTML_ELEMENT_TYPE_TBODY:
	حالة DOM_HTML_ELEMENT_TYPE_TFOOT:
		سیٹ_HTML_CLASS(TABLESECTION)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_جدول:
		سیٹ_HTML_CLASS(جدول_2)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_TR:
		سیٹ_HTML_CLASS(TABLEROW)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_STYLE:
		سیٹ_HTML_CLASS(STYLE)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_FRAMESET:
		سیٹ_HTML_CLASS(FRAMESET)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_چوکھٹا:
		سیٹ_HTML_CLASS(چوکھٹا_2)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_IFRAME:
		سیٹ_HTML_CLASS(IFRAME)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_ISINDEX:
		سیٹ_HTML_CLASS(ISINDEX)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_CANVAS:
		سیٹ_HTML_CLASS(CANVAS)
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_گنتی:
		أكد(type != DOM_HTML_ELEMENT_TYPE_گنتی);
		fallthrough;
	حالة DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		سیٹ_HTML_CLASS(UNKNOWN)
		اكسر;
	افتراضي:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		اكسر;
	}
	أرجع;
}

#تعریف_رد سیٹ_HTML_CLASS

ساكن فراغ
dukky_ڈالو_گرہ_klass(duk_context *ctx, بنية dom_گرہ *گرہ)
{
	dom_گرہ_type nodetype;
	dom_exception err;

	err = dom_گرہ_get_گرہ_type(گرہ, &nodetype);
	إذا (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_ڈالو_string(ctx, PROTO_NAME(گرہ_2));
		أرجع;
	}

	بدل(nodetype) {
	حالة DOM_ELEMENT_گرہ: {
		dom_string *namespace;
		dom_html_element_type type;
		ثابت حرف *html_class;
		size_t html_class_len;
		err = dom_گرہ_get_namespace(گرہ, &namespace);
		إذا (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_ڈالو_string(ctx, PROTO_NAME(ELEMENT));
			اكسر;
		}
		إذا (namespace == عدم) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_ڈالو_string(ctx, PROTO_NAME(ELEMENT));
			اكسر;
		}

		إذا (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_ڈالو_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			اكسر;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_ٹیگ_type(گرہ, &type);
		إذا (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_ٹیگ_type(type,
				&html_class, &html_class_len);

		duk_ڈالو_lstring(ctx, html_class, html_class_len);
		اكسر;
	}
	حالة DOM_متن_گرہ:
		duk_ڈالو_string(ctx, PROTO_NAME(متن_2));
		اكسر;
	حالة DOM_COMMENT_گرہ:
		duk_ڈالو_string(ctx, PROTO_NAME(COMMENT));
		اكسر;
	حالة DOM_DOCUMENT_گرہ:
		duk_ڈالو_string(ctx, PROTO_NAME(DOCUMENT));
		اكسر;
	حالة DOM_ATTRIBUTE_گرہ:
	حالة DOM_PROCESSING_INSTRUCTION_گرہ:
	حالة DOM_DOCUMENT_TYPE_گرہ:
	حالة DOM_DOCUMENT_FRAGMENT_گرہ:
	حالة DOM_NOTATION_گرہ:
	حالة DOM_ENTITY_REFERENCE_گرہ:
	حالة DOM_ENTITY_گرہ:
	حالة DOM_CDATA_SECTION_گرہ:
	افتراضي:
		/* Oh bum, just node then */
		duk_ڈالو_string(ctx, PROTO_NAME(گرہ_2));
	}
}

duk_bool_t
dukky_ڈالو_گرہ(duk_context *ctx, بنية dom_گرہ *گرہ)
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
	إذا (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_داخل_کرو(ctx, -2);
		/* ... node nodes */
		duk_نکالو(ctx);
		/* ... node */
		إذا (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			ثابت حرف * what = duk_محفوظ_منزل_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_نکالو(ctx);
		}
		أرجع true;
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
	أرجع dukky_ڈالو_گرہ_stacked(ctx);
}

ساكن duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	أرجع duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

فراغ
dukky_inject_not_ctr(duk_context *ctx, صحيح idx, ثابت حرف *name)
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
	أرجع;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

ساكن فراغ *dukky_alloc_function(فراغ *udata, duk_ناپ_t ناپ)
{
	إذا (ناپ == 0)
		أرجع عدم;

	أرجع حافظہ_دیو(ناپ);
}

ساكن فراغ *dukky_realloc_function(فراغ *udata, فراغ *ptr, duk_ناپ_t ناپ)
{
	إذا (ptr == عدم && ناپ == 0)
		أرجع عدم;

	إذا (ناپ == 0) {
		حرر(ptr);
		أرجع عدم;
	}

	أرجع realloc(ptr, ناپ);
}


ساكن فراغ dukky_free_function(فراغ *udata, فراغ *ptr)
{
	إذا (ptr != عدم)
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
js_newheap(صحيح timeout, jsheap **ڈھیر)
{
	duk_context *ctx;
	jsheap *ret = صفر_حافظہ_دیو(1, حجم(*ret));
	*ڈھیر = عدم;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	إذا (ret == عدم) أرجع NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_ڈھیر(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		عدم);
	إذا (ret->ctx == عدم) { حرر(ret); أرجع NSERROR_NOMEM; }
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
	أرجع NSERROR_OK;
}


ساكن فراغ dukky_destroyheap(jsheap *ڈھیر)
{
	أكد(ڈھیر->pending_destroy == true);
	أكد(ڈھیر->جیوندا_دھاگے == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_ڈھیر(ڈھیر->ctx);
	حرر(ڈھیر);
}

/* exported interface documented in js.h */
فراغ js_destroyheap(jsheap *ڈھیر)
{
	ڈھیر->pending_destroy = true;
	إذا (ڈھیر->جیوندا_دھاگے == 0) {
		dukky_destroyheap(ڈھیر);
	}
}

/* Just for here, the CTX is in ret, not thread */
#تعریف_کرو CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *ڈھیر, فراغ *win_priv, فراغ *doc_priv, jsthread **thread)
{
	jsthread *ret;
	أكد(ڈھیر != عدم);
	أكد(ڈھیر->pending_destroy == false);

	ret = صفر_حافظہ_دیو(1, حجم (*ret));
	إذا (ret == عدم) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		أرجع NSERROR_NOMEM;
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
	ڈھیر->جیوندا_دھاگے++;
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
	duk_ڈالو_اشارہ_گر(CTX, عدم);
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
	إذا (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ثابت حرف *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_محفوظ_منزل_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		أرجع NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	إذا (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		أرجع NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_نکالو(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_ڈالو_string(CTX, "generics.js");
	/* ..., generics.js */
	إذا (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ثابت حرف *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_محفوظ_منزل_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		أرجع NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	إذا (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		أرجع NSERROR_INIT_FAILED;
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

	أرجع NSERROR_OK;
}

/* Now switch to the long term CTX behaviour */
#تعریف_رد CTX
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

	أرجع NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
ساكن فراغ dukky_destroythread(jsthread *thread)
{
	jsheap *ڈھیر = thread->ڈھیر;

	أكد(thread->وچ_use == 0);
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
	حرر(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(ڈھیر->ctx, 0);
	duk_gc(ڈھیر->ctx, DUK_GC_COMPACT);
	ڈھیر->جیوندا_دھاگے--;

	/* And if the heap should now go, blow it away */
	إذا (ڈھیر->pending_destroy == true && ڈھیر->جیوندا_دھاگے == 0) {
		dukky_destroyheap(ڈھیر);
	}
}

/* exported interface documented in js.h */
فراغ js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	إذا (thread->وچ_use == 0) {
		dukky_destroythread(thread);
	}
}

ساكن فراغ dukky_enter_thread(jsthread *thread)
{
	أكد(thread != عدم);
	thread->وچ_use++;
}

ساكن فراغ dukky_leave_thread(jsthread *thread)
{
	أكد(thread != عدم);
	أكد(thread->وچ_use > 0);

	thread->وچ_use--;
	إذا (thread->وچ_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(فراغ *udata)
{
#تعریف_کرو JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *ڈھیر = (jsheap *) udata;
	uint64_t now;

	(فراغ) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	أرجع ڈھیر->exec_شروع_ویلا != 0 &&
			now > (ڈھیر->exec_شروع_ویلا + JS_EXEC_TIMEOUT_MS);
}

ساكن فراغ dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_چوٹی(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_محفوظ_منزل_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_نکالو(ctx);
	/* ..., errobj */
}

ساكن فراغ dukky_reset_شروع_ویلا(duk_context *ctx)
{
	duk_حافظہ_functions funcs;
	jsheap *ڈھیر;
	duk_get_حافظہ_functions(ctx, &funcs);
	ڈھیر = funcs.udata;
	(فراغ) nsu_getmonotonic_ms(&ڈھیر->exec_شروع_ویلا);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_ناپ_t argc, bool reset_timeout)
{
	إذا (reset_timeout) {
		dukky_reset_شروع_ویلا(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	إذا (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	أرجع ret;
}


فراغ dukky_ڈالو_generics(duk_context *ctx, ثابت حرف *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

ساكن duk_int_t dukky_ڈالو_context_dump(duk_context *ctx, فراغ *udata)
{
	duk_ڈالو_context_dump(ctx);
	أرجع 1;
}

فراغ dukky_log_ڈھیر_چوکھٹا(duk_context *ctx, ثابت حرف * reason)
{
	إذا (duk_محفوظ_call(ctx, dukky_ڈالو_context_dump, عدم, 0, 1) != 0) {
		duk_نکالو(ctx);
		duk_ڈالو_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_محفوظ_منزل_string(ctx, -1));
	duk_نکالو(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, ثابت uint8_t *txt, size_t txtlen, ثابت حرف *name)
{
	bool ret = false;
	أكد(thread);

	إذا (txt == عدم || txtlen == 0) {
		أرجع false;
	}

	إذا (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		أرجع false;
	}

	dukky_enter_thread(thread);

	duk_سیٹ_چوٹی(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_شروع_ویلا(CTX);
	إذا (name != عدم) {
		duk_ڈالو_string(CTX, name);
	} وإلا {
		duk_ڈالو_string(CTX, "?unknown source?");
	}
	إذا (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (ثابت حرف *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		اذهب handle_error;
	}

	إذا (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		اذهب handle_error;
	}

	إذا (duk_get_چوٹی(CTX) == 0) duk_ڈالو_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	اذهب باہرلا;

handle_error:
	dukky_dump_error(CTX);
باہرلا:
	dukky_leave_thread(thread);
	أرجع ret;
}

ساكن ثابت حرف* dukky_event_proto(dom_event *evt)
{
	ثابت حرف *ret = PROTO_NAME(EVENT);
	dom_string *type = عدم;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	إذا (err != DOM_NO_ERR) {
		اذهب باہرلا;
	}

	إذا (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		اذهب باہرلا;
	} وإلا إذا (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		اذهب باہرلا;
	} وإلا إذا (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		اذهب باہرلا;
	}

باہرلا:
	إذا (type != عدم) {
		dom_string_unref(type);
	}

	أرجع ret;
}

/*** New style event handling ***/

فراغ dukky_ڈالو_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_ڈالو_اشارہ_گر(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	إذا (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_نکالو(ctx);
		/* ... events */
		duk_ڈالو_اشارہ_گر(ctx, evt);
		إذا (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
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

ساكن فراغ dukky_ڈالو_handler_کوڈ(duk_context *ctx, dom_string *name,
				     dom_event_نشانہ *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_گرہ_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	إذا (et == عدم) {
		duk_ڈالو_lstring(ctx, "", 0);
		أرجع;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_گرہ_get_گرہ_type(et, &ntype);
	إذا (exc != DOM_NO_ERR) {
		duk_ڈالو_lstring(ctx, "", 0);
		أرجع;
	}

	إذا (ntype != DOM_ELEMENT_گرہ) {
		duk_ڈالو_lstring(ctx, "", 0);
		أرجع;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	إذا (exc != DOM_NO_ERR) {
		duk_ڈالو_lstring(ctx, "", 0);
		أرجع;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	إذا ((exc != DOM_NO_ERR) || (val == عدم)) {
		dom_string_unref(onname);
		duk_ڈالو_lstring(ctx, "", 0);
		أرجع;
	}

	dom_string_unref(onname);
	duk_ڈالو_lstring(ctx, dom_string_انکڑے(val), dom_string_لمائی(val));
	dom_string_unref(val);
}

bool dukky_get_current_قدر_دا_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_نشانہ *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_ڈالو_lstring(ctx, dom_string_انکڑے(name), dom_string_لمائی(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	إذا (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_نکالو_2(ctx);
		/* ... node */
		dukky_ڈالو_handler_کوڈ(ctx, name, et);
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
		إذا (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_نکالو_2(ctx);
			أرجع false;
		}
		/* ... node handler */
		duk_داخل_کرو(ctx, -2);
		/* ... handler node */
	} وإلا {
		/* ... node handlers handler */
		duk_داخل_کرو(ctx, -3);
		/* ... handler node handlers */
		duk_نکالو(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	أرجع true;
}

ساكن فراغ dukky_generic_event_handler(dom_event *evt, فراغ *pw)
{
	duk_context *ctx = (duk_context *)pw;
	dom_string *name;
	dom_exception exc;
	dom_event_نشانہ *targ;
	dom_event_flow_phase phase;
	duk_uarridx_t idx;
	event_listener_flags flags;

	NSLOG(dukky, DEBUG, "Handling an event in duktape interface...");
	exc = dom_event_get_type(evt, &name);
	إذا (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		أرجع;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (صحيح)dom_string_لمائی(name),
	      dom_string_انکڑے(name));
	exc = dom_event_get_event_phase(evt, &phase);
	إذا (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		أرجع;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_نشانہ ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (صحيح)phase);

	exc = dom_event_get_current_نشانہ(evt, &targ);
	إذا (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		أرجع;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	إذا (phase == DOM_CAPTURING_PHASE)
		اذهب handle_extras;

	/* ... */
	إذا (dukky_ڈالو_گرہ(ctx, (dom_گرہ *)targ) == false) {
		dom_string_unref(name);
		dom_گرہ_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		أرجع;
	}
	/* ... node */
	إذا (dukky_get_current_قدر_دا_event_handler(
		    ctx, name, (dom_event_نشانہ *)targ) == false) {
		/* ... */
		اذهب handle_extras;
	}
	/* ... handler node */
	dukky_ڈالو_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_شروع_ویلا(ctx);
	إذا (duk_pcall_طریقہ(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_فوری_پسار(evt);
		إذا (exc != DOM_NO_ERR)
			NSLOG(dukky, DEBUG,
			      "WORSE! could not stop propagation");
		duk_get_prop_string(ctx, -1, "name");
		duk_get_prop_string(ctx, -2, "message");
		duk_get_prop_string(ctx, -3, "fileName");
		duk_get_prop_string(ctx, -4, "lineNumber");
		duk_get_prop_string(ctx, -5, "stack");
		/* ... err name message fileName lineNumber stack */
		NSLOG(dukky, DEBUG, "Uncaught error in JS: %s: %s",
		      duk_محفوظ_منزل_string(ctx, -5),
		      duk_محفوظ_منزل_string(ctx, -4));
		NSLOG(dukky, INFO, "              was at: %s line %s",
		      duk_محفوظ_منزل_string(ctx, -3),
		      duk_محفوظ_منزل_string(ctx, -2));
		NSLOG(dukky, INFO, "         Stack trace: %s",
		      duk_محفوظ_منزل_string(ctx, -1));

		duk_نکالو_n(ctx, 6);
		/* ... */
		اذهب handle_extras;
	}
	/* ... result */
	إذا (duk_is_boolean(ctx, -1) &&
	    duk_منزل_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_نکالو(ctx);
handle_extras:
	/* ... */
	duk_ڈالو_lstring(ctx, dom_string_انکڑے(name), dom_string_لمائی(name));
	dukky_ڈالو_گرہ(ctx, (dom_گرہ *)targ);
	/* ... type node */
	إذا (dukky_event_نشانہ_ڈالو_listeners(ctx, true)) {
		/* Nothing to do */
		duk_نکالو(ctx);
		اذهب باہرلا;
	}
	/* ... sublisteners */
	duk_ڈالو_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	طالما (duk_get_prop_اشاریہ(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_اشاریہ(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		إذا ((event_listener_flags)duk_منزل_int(ctx, -1) & ELF_ONCE) {
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
	طالما (duk_get_prop_اشاریہ(ctx, -1, idx++)) {
		/* ... copy handler */
		إذا (duk_get_prop_اشاریہ(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_نکالو_2(ctx);
			استمر;
		}
		duk_نکالو(ctx);
		duk_get_prop_اشاریہ(ctx, -1, 0);
		duk_get_prop_اشاریہ(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_نکالو(ctx);
		/* ... copy handler callback */
		إذا (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_نکالو_2(ctx);
			/* ... copy */
			استمر;
		}
		/* ... copy handler callback */
		dukky_ڈالو_گرہ(ctx, (dom_گرہ *)targ);
		/* ... copy handler callback node */
		dukky_ڈالو_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_شروع_ویلا(ctx);
		إذا (duk_pcall_طریقہ(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_فوری_پسار(evt);
			إذا (exc != DOM_NO_ERR)
				NSLOG(dukky, DEBUG,
				      "WORSE! could not stop propagation");
			duk_get_prop_string(ctx, -1, "name");
			duk_get_prop_string(ctx, -2, "message");
			duk_get_prop_string(ctx, -3, "fileName");
			duk_get_prop_string(ctx, -4, "lineNumber");
			duk_get_prop_string(ctx, -5, "stack");
			/* ... err name message fileName lineNumber stack */
			NSLOG(dukky, DEBUG, "Uncaught error in JS: %s: %s",
			      duk_محفوظ_منزل_string(ctx, -5),
			      duk_محفوظ_منزل_string(ctx, -4));
			NSLOG(dukky, DEBUG,
			      "              was at: %s line %s",
			      duk_محفوظ_منزل_string(ctx, -3),
			      duk_محفوظ_منزل_string(ctx, -2));
			NSLOG(dukky, DEBUG, "         Stack trace: %s",
			      duk_محفوظ_منزل_string(ctx, -1));

			duk_نکالو_n(ctx, 7);
			/* ... copy */
			استمر;
		}
		/* ... copy handler result */
		إذا (duk_is_boolean(ctx, -1) &&
		    duk_منزل_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_نکالو_2(ctx);
		/* ... copy */
	}
	duk_نکالو_2(ctx);
باہرلا:
	/* ... */
	dom_گرہ_unref(targ);
	dom_string_unref(name);
}

فراغ dukky_رجسٹر_event_listener_for(duk_context *ctx,
				       بنية dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = عدم;
	dom_exception exc;

	/* ... */
	إذا (ele == عدم) {
		/* A null element is the Window object */
		duk_ڈالو_global_object(ctx);
	} وإلا {
		/* Non null elements must be pushed as a node object */
		إذا (dukky_ڈالو_گرہ(ctx, (بنية dom_گرہ *)ele) == false)
			أرجع;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_ڈالو_lstring(ctx, dom_string_انکڑے(name), dom_string_لمائی(name));
	/* ... node handlers name */
	إذا (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_نکالو_2(ctx);
		/* ... */
		أرجع;
	}
	/* ... node handlers */
	duk_ڈالو_lstring(ctx, dom_string_انکڑے(name), dom_string_لمائی(name));
	/* ... node handlers name */
	duk_ڈالو_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_نکالو_2(ctx);
	/* ... */
	إذا (ele == عدم) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		أرجع;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	إذا (exc != DOM_NO_ERR) أرجع;
	exc = dom_event_نشانہ_جوڑو_event_listener(
		ele, name, listen, capture);
	إذا (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (صحيح)dom_string_لمائی(name), dom_string_انکڑے(name));
	} وإلا {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (صحيح)dom_string_لمائی(name), dom_string_انکڑے(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_نشانہ_ڈالو_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	إذا (duk_is_undefined(ctx, -1)) {
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
	إذا (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		إذا (dont_create == true) {
			duk_نکالو_3(ctx);
			duk_ڈالو_undefined(ctx);
			أرجع true;
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
	أرجع ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
فراغ dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	طالما (duk_get_prop_اشاریہ(ctx, -1, idx + 1)) {
		duk_put_prop_اشاریہ(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_اشاریہ(ctx, -2, idx + 1);
	duk_نکالو(ctx);
}


فراغ js_handle_new_element(jsthread *thread, بنية dom_element *گرہ)
{
	أكد(thread);
	أكد(گرہ);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = عدم;
	dom_string *کلید = عدم;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_گرہ_get_گرہ_name(گرہ, &nodename);
	إذا (exc != DOM_NO_ERR) أرجع;

	إذا (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_گرہ_get_گن(گرہ, &map);
	إذا (exc != DOM_NO_ERR) أرجع;
	إذا (map == عدم) أرجع;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_لمائی(map, &siz);
	إذا (exc != DOM_NO_ERR) اذهب باہرلا;

	لكل (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_شے(map, idx, &attr);
		إذا (exc != DOM_NO_ERR) اذهب باہرلا;
		exc = dom_attr_get_name(attr, &کلید);
		إذا (exc != DOM_NO_ERR) اذهب باہرلا;
		إذا (is_body && (
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
			اذهب skip_رجسٹر;
		}
		إذا (dom_string_لمائی(کلید) > 2) {
			/* Can be on* */
			ثابت uint8_t *انکڑے = (ثابت uint8_t *)dom_string_انکڑے(کلید);
			إذا (انکڑے[0] == 'o' && انکڑے[1] == 'n') {
				dom_string *sub = عدم;
				exc = dom_string_substr(
					کلید, 2, dom_string_لمائی(کلید),
					&sub);
				إذا (exc == DOM_NO_ERR) {
					dukky_رجسٹر_event_listener_for(
						CTX, گرہ, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_رجسٹر:
		dom_string_unref(کلید); کلید = عدم;
		dom_گرہ_unref(attr); attr = عدم;
	}

باہرلا:
	إذا (کلید != عدم)
		dom_string_unref(کلید);

	إذا (attr != عدم)
		dom_گرہ_unref(attr);

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
	duk_ڈالو_اشارہ_گر(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_نکالو(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, ثابت حرف *type, بنية dom_document *doc, بنية dom_گرہ *نشانہ)
{
	dom_exception exc;
	dom_event *evt;
	dom_event_نشانہ *body;

	NSLOG(dukky, DEBUG, "Event: %s (doc=%p, target=%p)", type, doc,
	      نشانہ);

	/** @todo Make this more generic, this only handles load and only
	 * targetting the window, so that we actually stand a chance of
	 * getting 3.4 out.
	 */

	إذا (نشانہ != عدم)
		/* Swallow non-Window-targetted events quietly */
		أرجع true;

	إذا (strcmp(type, "load") != 0)
		/* Swallow non-load events quietly */
		أرجع true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	إذا (exc != DOM_NO_ERR) أرجع true;
	exc = dom_event_init(evt, corestring_dom_لدو, false, false);
	إذا (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		أرجع true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_ڈالو_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	إذا (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_نکالو(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		إذا (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			أرجع true;
		}
		dukky_ڈالو_گرہ(CTX, (بنية dom_گرہ *)body);
		/* ... handlers bodynode */
		إذا (dukky_get_current_قدر_دا_event_handler(
			    CTX, corestring_dom_لدو, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_گرہ_unref(body);
			/* ... handlers */
			duk_نکالو(CTX);
			dukky_leave_thread(thread);
			أرجع true;
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
	dukky_reset_شروع_ویلا(CTX);
	إذا (duk_pcall_طریقہ(CTX, 1) != 0) {
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
		      duk_محفوظ_منزل_string(CTX, -5),
		      duk_محفوظ_منزل_string(CTX, -4));
		NSLOG(dukky, DEBUG, "              was at: %s line %s",
		      duk_محفوظ_منزل_string(CTX, -3),
		      duk_محفوظ_منزل_string(CTX, -2));
		NSLOG(dukky, DEBUG, "         Stack trace: %s",
		      duk_محفوظ_منزل_string(CTX, -1));

		duk_نکالو_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		أرجع true;
	}
	/* ... result */
	duk_نکالو(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	أرجع true;
}
