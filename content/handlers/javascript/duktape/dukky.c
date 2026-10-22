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

#అన్తర్భావ <stdint.h>
#అన్తర్భావ <nsutils/time.h>

#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "content/content.h"

#అన్తర్భావ "javascript/js.h"
#అన్తర్భావ "javascript/content.h"

#అన్తర్భావ "duktape/binding.h"
#అన్తర్భావ "duktape/generics.js.inc"
#అన్తర్భావ "duktape/polyfill.js.inc"

#అన్తర్భావ "duktape.h"
#అన్తర్భావ "dukky.h"

#అన్తర్భావ <dom/dom.h>

#నిర్వచించు EVENT_MAGIC MAGIC(EVENT_MAP)
#నిర్వచించు HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#నిర్వచించు HANDLER_MAGIC MAGIC(HANDLER_MAP)
#నిర్వచించు EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#నిర్వచించు GENERICS_MAGIC MAGIC(GENERICS_పట్టిక)
#నిర్వచించు THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
నిర్మాణం jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t తదుపరి_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	గుర్తులేని పూర్ణాంకం సజీవ_దారాలు; /**< number of live threads */
	uint64_t exec_ప్రారంభం_సమయం;
};

/**
 * dukky javascript thread
 */
నిర్మాణం jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	గుర్తులేని పూర్ణాంకం లోపల_use; /**< The number of times this thread is in use */
	jsheap *కుప్ప; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

స్థిర duk_ret_t dukky_populate_object(duk_context *ctx, శూన్య *udata)
{
	/* ... obj args protoname nargs */
	పూర్ణాంకం nargs = duk_get_int(ctx, -1);
	duk_తీయి(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	ఒకవేళ (duk_is_undefined(ctx, -1)) {
		NSLOG(dukky, WARNING,
		      "Unable to find dukky prototype for `%s` - falling back to HTMLUnknownElement",
		      duk_get_string(ctx, -3) + 2 /* Skip the two unprintables */);
		duk_తీయి(ctx);
		duk_నెట్టు_string(ctx, PROTO_NAME(HTMLUNKNOWNELEMENT));
		duk_get_prop(ctx, -2);
	}
	/* ... obj args protoname prototab proto */
	duk_remove(ctx, -3);
	/* ... obj args prototab proto */
	duk_dup(ctx, -1);
	/* ... obj args prototab proto proto */
	duk_సమితి_prototype(ctx, -(nargs+4));
	/* ... obj[proto] args prototab proto */
	duk_get_prop_string(ctx, -1, INIT_MAGIC);
	/* ... obj[proto] args prototab proto initfn */
	duk_చొప్పించు(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_తీయి_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	తిరిగి 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, స్థిరరాశి అక్షరం *name, పూర్ణాంకం వాదనలు)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, వాదనలు);
	/* ... args */
	duk_నెట్టు_object(ctx);
	/* ... args obj */
	duk_నెట్టు_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
	/* ... args obj */
	duk_నెట్టు_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
	/* ... args obj */
	duk_చొప్పించు(ctx, -(వాదనలు+1));
	/* ... obj args */
	duk_నెట్టు_string(ctx, name);
	/* ... obj args name */
	duk_నెట్టు_int(ctx, వాదనలు);
	/* ... obj args name nargs */
	ఒకవేళ ((ret = duk_సురక్షిత_call(ctx, dukky_populate_object, ఖాళీ, వాదనలు + 3, 1))
	    != DUK_EXEC_SUCCESS)
		తిరిగి ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	తిరిగి DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_నెట్టు_కణుపు_stacked(duk_context *ctx)
{
	పూర్ణాంకం శిఖరం_at_fail = duk_get_శిఖరం(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, కణుపు_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	ఒకవేళ (duk_is_undefined(ctx, -1)) {
		/* ... nodeptr klass nodes undefined */
		duk_తీయి(ctx);
		/* ... nodeptr klass nodes */
		duk_నెట్టు_object(ctx);
		/* ... nodeptr klass nodes obj */
		duk_నెట్టు_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_నెట్టు_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr klass */
		duk_నెట్టు_int(ctx, 1);
		/* ... nodeptr klass nodes obj nodeptr klass 1 */
		ఒకవేళ (duk_సురక్షిత_call(ctx, dukky_populate_object, ఖాళీ, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_సమితి_శిఖరం(ctx, శిఖరం_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			తిరిగి false;
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
	duk_చొప్పించు(ctx, -4);
	/* ... node nodeptr klass nodes */
	duk_తీయి_3(ctx);
	/* ... node */
	ఒకవేళ (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		స్థిరరాశి అక్షరం * what = duk_సురక్షిత_గమ్యం_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_తీయి(ctx);
	}
	తిరిగి true;
}

#నిర్వచించు సమితి_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

స్థిర శూన్య dukky_html_element_class_from_ట్యాగ్_type(dom_html_element_type type,
		స్థిరరాశి అక్షరం **html_class, size_t *html_class_len)
{
	ఎంపిక(type) {
	స్థితి DOM_HTML_ELEMENT_TYPE_HTML:
		సమితి_HTML_CLASS(HTML)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_తల:
		సమితి_HTML_CLASS(తల)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_META:
		సమితి_HTML_CLASS(META)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_పునాది:
		సమితి_HTML_CLASS(పునాది_2)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_TITLE:
		సమితి_HTML_CLASS(TITLE)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_BODY:
		సమితి_HTML_CLASS(BODY)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_DIV:
		సమితి_HTML_CLASS(DIV)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_FORM:
		సమితి_HTML_CLASS(FORM)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_LINK:
		సమితి_HTML_CLASS(LINK)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_BUTTON:
		సమితి_HTML_CLASS(BUTTON)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_ఆదానం:
		సమితి_HTML_CLASS(ఆదానం_2)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		సమితి_HTML_CLASS(TEXTAREA)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		సమితి_HTML_CLASS(OPTGROUP)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_OPTION:
		సమితి_HTML_CLASS(OPTION)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_SELECT:
		సమితి_HTML_CLASS(SELECT)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_HR:
		సమితి_HTML_CLASS(HR)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_DL:
		సమితి_HTML_CLASS(DLIST)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_DIR:
		సమితి_HTML_CLASS(DIRECTORY)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_MENU:
		సమితి_HTML_CLASS(MENU)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_FIELDSET:
		సమితి_HTML_CLASS(FIELDSET)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_LEGEND:
		సమితి_HTML_CLASS(LEGEND)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_P:
		సమితి_HTML_CLASS(PARAGRAPH)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_H1:
	స్థితి DOM_HTML_ELEMENT_TYPE_H2:
	స్థితి DOM_HTML_ELEMENT_TYPE_H3:
	స్థితి DOM_HTML_ELEMENT_TYPE_H4:
	స్థితి DOM_HTML_ELEMENT_TYPE_H5:
	స్థితి DOM_HTML_ELEMENT_TYPE_H6:
		సమితి_HTML_CLASS(HEADING)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	స్థితి DOM_HTML_ELEMENT_TYPE_Q:
		సమితి_HTML_CLASS(QUOTE)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_PRE:
		సమితి_HTML_CLASS(PRE)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_BR:
		సమితి_HTML_CLASS(BR)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_LABEL:
		సమితి_HTML_CLASS(LABEL)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_UL:
		సమితి_HTML_CLASS(ULIST)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_OL:
		సమితి_HTML_CLASS(OLIST)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_LI:
		సమితి_HTML_CLASS(LI)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_FONT:
		సమితి_HTML_CLASS(FONT)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_DEL:
	స్థితి DOM_HTML_ELEMENT_TYPE_INS:
		సమితి_HTML_CLASS(MOD)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_A:
		సమితి_HTML_CLASS(ANCHOR)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_BASEFONT:
		సమితి_HTML_CLASS(BASEFONT)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_IMG:
		సమితి_HTML_CLASS(IMAGE)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_OBJECT:
		సమితి_HTML_CLASS(OBJECT)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_PARAM:
		సమితి_HTML_CLASS(PARAM)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_APPLET:
		సమితి_HTML_CLASS(APPLET)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_MAP:
		సమితి_HTML_CLASS(MAP)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_AREA:
		సమితి_HTML_CLASS(AREA)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_SCRIPT:
		సమితి_HTML_CLASS(SCRIPT)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_CAPTION:
		సమితి_HTML_CLASS(TABLECAPTION)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_TD:
	స్థితి DOM_HTML_ELEMENT_TYPE_TH:
		సమితి_HTML_CLASS(TABLECELL)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_నిలువు:
	స్థితి DOM_HTML_ELEMENT_TYPE_COLGROUP:
		సమితి_HTML_CLASS(TABLECOL)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_THEAD:
	స్థితి DOM_HTML_ELEMENT_TYPE_TBODY:
	స్థితి DOM_HTML_ELEMENT_TYPE_TFOOT:
		సమితి_HTML_CLASS(TABLESECTION)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_పట్టిక:
		సమితి_HTML_CLASS(పట్టిక_2)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_TR:
		సమితి_HTML_CLASS(TABLEROW)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_STYLE:
		సమితి_HTML_CLASS(STYLE)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_FRAMESET:
		సమితి_HTML_CLASS(FRAMESET)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_చట్రం:
		సమితి_HTML_CLASS(చట్రం_2)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_IFRAME:
		సమితి_HTML_CLASS(IFRAME)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_ISINDEX:
		సమితి_HTML_CLASS(ISINDEX)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_CANVAS:
		సమితి_HTML_CLASS(CANVAS)
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_లెక్క:
		ప్రతిజ్ఞా(type != DOM_HTML_ELEMENT_TYPE_లెక్క);
		fallthrough;
	స్థితి DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		సమితి_HTML_CLASS(UNKNOWN)
		ఆపు;
	అప్రమేయ:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		ఆపు;
	}
	తిరిగి;
}

#నిర్వచనం_రద్దు సమితి_HTML_CLASS

స్థిర శూన్య
dukky_నెట్టు_కణుపు_klass(duk_context *ctx, నిర్మాణం dom_కణుపు *కణుపు)
{
	dom_కణుపు_type nodetype;
	dom_exception err;

	err = dom_కణుపు_get_కణుపు_type(కణుపు, &nodetype);
	ఒకవేళ (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_నెట్టు_string(ctx, PROTO_NAME(కణుపు_2));
		తిరిగి;
	}

	ఎంపిక(nodetype) {
	స్థితి DOM_ELEMENT_కణుపు: {
		dom_string *namespace;
		dom_html_element_type type;
		స్థిరరాశి అక్షరం *html_class;
		size_t html_class_len;
		err = dom_కణుపు_get_namespace(కణుపు, &namespace);
		ఒకవేళ (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_నెట్టు_string(ctx, PROTO_NAME(ELEMENT));
			ఆపు;
		}
		ఒకవేళ (namespace == ఖాళీ) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_నెట్టు_string(ctx, PROTO_NAME(ELEMENT));
			ఆపు;
		}

		ఒకవేళ (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_నెట్టు_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			ఆపు;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_ట్యాగ్_type(కణుపు, &type);
		ఒకవేళ (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_ట్యాగ్_type(type,
				&html_class, &html_class_len);

		duk_నెట్టు_lstring(ctx, html_class, html_class_len);
		ఆపు;
	}
	స్థితి DOM_పాఠ్యం_కణుపు:
		duk_నెట్టు_string(ctx, PROTO_NAME(పాఠ్యం_2));
		ఆపు;
	స్థితి DOM_COMMENT_కణుపు:
		duk_నెట్టు_string(ctx, PROTO_NAME(COMMENT));
		ఆపు;
	స్థితి DOM_DOCUMENT_కణుపు:
		duk_నెట్టు_string(ctx, PROTO_NAME(DOCUMENT));
		ఆపు;
	స్థితి DOM_ATTRIBUTE_కణుపు:
	స్థితి DOM_PROCESSING_INSTRUCTION_కణుపు:
	స్థితి DOM_DOCUMENT_TYPE_కణుపు:
	స్థితి DOM_DOCUMENT_FRAGMENT_కణుపు:
	స్థితి DOM_NOTATION_కణుపు:
	స్థితి DOM_ENTITY_REFERENCE_కణుపు:
	స్థితి DOM_ENTITY_కణుపు:
	స్థితి DOM_CDATA_SECTION_కణుపు:
	అప్రమేయ:
		/* Oh bum, just node then */
		duk_నెట్టు_string(ctx, PROTO_NAME(కణుపు_2));
	}
}

duk_bool_t
dukky_నెట్టు_కణుపు(duk_context *ctx, నిర్మాణం dom_కణుపు *కణుపు)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", కణుపు);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, కణుపు_MAGIC);
	/* ... nodes */
	duk_నెట్టు_సూచిక(ctx, కణుపు);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	ఒకవేళ (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_చొప్పించు(ctx, -2);
		/* ... node nodes */
		duk_తీయి(ctx);
		/* ... node */
		ఒకవేళ (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			స్థిరరాశి అక్షరం * what = duk_సురక్షిత_గమ్యం_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_తీయి(ctx);
		}
		తిరిగి true;
	}
	/* ... nodes undefined */
	duk_తీయి_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_నెట్టు_సూచిక(ctx, కణుపు);
	/* ... nodeptr */
	dukky_నెట్టు_కణుపు_klass(ctx, కణుపు);
	/* ... nodeptr klass */
	తిరిగి dukky_నెట్టు_కణుపు_stacked(ctx);
}

స్థిర duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	తిరిగి duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

శూన్య
dukky_inject_not_ctr(duk_context *ctx, పూర్ణాంకం idx, స్థిరరాశి అక్షరం *name)
{
	/* ... p[idx] ... proto */
	duk_నెట్టు_c_function(ctx, dukky_bad_constructor, 0);
	/* ... p[idx] ... proto cons */
	duk_చొప్పించు(ctx, -2);
	/* ... p[idx] ... cons proto */
	duk_put_prop_string(ctx, -2, "prototype");
	/* ... p[idx] ... cons[proto] */
	duk_put_prop_string(ctx, idx, name);
	/* ... p ... */
	తిరిగి;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

స్థిర శూన్య *dukky_alloc_function(శూన్య *udata, duk_కొలత_t కొలత)
{
	ఒకవేళ (కొలత == 0)
		తిరిగి ఖాళీ;

	తిరిగి జ్ఞాపకం_ఇవ్వు(కొలత);
}

స్థిర శూన్య *dukky_realloc_function(శూన్య *udata, శూన్య *ptr, duk_కొలత_t కొలత)
{
	ఒకవేళ (ptr == ఖాళీ && కొలత == 0)
		తిరిగి ఖాళీ;

	ఒకవేళ (కొలత == 0) {
		విడుదల(ptr);
		తిరిగి ఖాళీ;
	}

	తిరిగి realloc(ptr, కొలత);
}


స్థిర శూన్య dukky_free_function(శూన్య *udata, శూన్య *ptr)
{
	ఒకవేళ (ptr != ఖాళీ)
		విడుదల(ptr);
}

/* exported interface documented in js.h */
శూన్య js_initialise(శూన్య)
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
శూన్య js_finalise(శూన్య)
{
	/* NADA for now */
}


/* exported interface documented in js.h */
nserror
js_newheap(పూర్ణాంకం timeout, jsheap **కుప్ప)
{
	duk_context *ctx;
	jsheap *ret = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*ret));
	*కుప్ప = ఖాళీ;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	ఒకవేళ (ret == ఖాళీ) తిరిగి NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_కుప్ప(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		ఖాళీ);
	ఒకవేళ (ret->ctx == ఖాళీ) { విడుదల(ret); తిరిగి NSERROR_NOMEM; }
	/* Create the prototype stuffs */
	duk_నెట్టు_global_object(ctx);
	duk_నెట్టు_boolean(ctx, true);
	duk_put_prop_string(ctx, -2, "protos");
	duk_put_global_string(ctx, PROTO_MAGIC);
	/* Create prototypes here */
	dukky_create_prototypes(ctx);
	/* Now create the thread map */
	duk_నెట్టు_object(ctx);
	duk_put_global_string(ctx, THREAD_MAP);

	*కుప్ప = ret;
	తిరిగి NSERROR_OK;
}


స్థిర శూన్య dukky_destroyheap(jsheap *కుప్ప)
{
	ప్రతిజ్ఞా(కుప్ప->pending_destroy == true);
	ప్రతిజ్ఞా(కుప్ప->సజీవ_దారాలు == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_కుప్ప(కుప్ప->ctx);
	విడుదల(కుప్ప);
}

/* exported interface documented in js.h */
శూన్య js_destroyheap(jsheap *కుప్ప)
{
	కుప్ప->pending_destroy = true;
	ఒకవేళ (కుప్ప->సజీవ_దారాలు == 0) {
		dukky_destroyheap(కుప్ప);
	}
}

/* Just for here, the CTX is in ret, not thread */
#నిర్వచించు CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *కుప్ప, శూన్య *win_priv, శూన్య *doc_priv, jsthread **thread)
{
	jsthread *ret;
	ప్రతిజ్ఞా(కుప్ప != ఖాళీ);
	ప్రతిజ్ఞా(కుప్ప->pending_destroy == false);

	ret = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం (*ret));
	ఒకవేళ (ret == ఖాళీ) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		తిరిగి NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(కుప్ప->ctx, THREAD_MAP); /* ... threads */
	duk_నెట్టు_thread(కుప్ప->ctx); /* ... threads thread */
	ret->కుప్ప = కుప్ప;
	ret->ctx = duk_require_context(కుప్ప->ctx, -1);
	ret->thread_idx = కుప్ప->తదుపరి_thread++;
	duk_put_prop_సూచీ(కుప్ప->ctx, -2, ret->thread_idx);
	కుప్ప->సజీవ_దారాలు++;
	duk_తీయి(కుప్ప->ctx); /* ... */
	duk_నెట్టు_int(CTX, 0);
	duk_నెట్టు_int(CTX, 1);
	duk_నెట్టు_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_నెట్టు_సూచిక(CTX, win_priv);
	duk_నెట్టు_సూచిక(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(కిటికీ), 2);
	duk_నెట్టు_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_సమితి_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_నెట్టు_object(CTX);
	duk_నెట్టు_సూచిక(CTX, ఖాళీ);
	duk_నెట్టు_null(CTX);
	duk_put_prop(CTX, -3);
	duk_put_global_string(CTX, కణుపు_MAGIC);

	/* And now the event mapping table */
	duk_నెట్టు_object(CTX);
	duk_put_global_string(CTX, EVENT_MAGIC);

	/* Now load the polyfills */
	/* ... */
	duk_నెట్టు_string(CTX, "polyfill.js");
	/* ..., polyfill.js */
	ఒకవేళ (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (స్థిరరాశి అక్షరం *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_సురక్షిత_గమ్యం_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		తిరిగి NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	ఒకవేళ (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		తిరిగి NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_తీయి(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_నెట్టు_string(CTX, "generics.js");
	/* ..., generics.js */
	ఒకవేళ (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (స్థిరరాశి అక్షరం *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_సురక్షిత_గమ్యం_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		తిరిగి NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	ఒకవేళ (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		తిరిగి NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_తీయి(CTX);
	/* ... */
	duk_నెట్టు_global_object(CTX);
	/* ..., Win */
	duk_get_prop_string(CTX, -1, "NetSurf");
	/* ..., Win, NetSurf */
	duk_put_global_string(CTX, GENERICS_MAGIC);
	/* ..., Win */
	duk_del_prop_string(CTX, -1, "NetSurf");
	duk_తీయి(CTX);
	/* ... */

	dukky_log_దొంతర_చట్రం(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, కుప్ప);
	*thread = ret;

	తిరిగి NSERROR_OK;
}

/* Now switch to the long term CTX behaviour */
#నిర్వచనం_రద్దు CTX
#నిర్వచించు CTX (thread->ctx)

/* exported interface documented in js.h */
nserror js_closethread(jsthread *thread)
{
	/* We can always close down a thread, it might just confuse
	 * the code running, though we don't mind since we're in the
	 * process of destruction at this point
	 */
	duk_int_t శిఖరం = duk_get_శిఖరం(CTX);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, thread->కుప్ప);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Restore whatever stack we had */
	duk_సమితి_శిఖరం(CTX, శిఖరం);

	తిరిగి NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
స్థిర శూన్య dukky_destroythread(jsthread *thread)
{
	jsheap *కుప్ప = thread->కుప్ప;

	ప్రతిజ్ఞా(thread->లోపల_use == 0);
	ప్రతిజ్ఞా(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, కుప్ప);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(కుప్ప->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_సూచీ(కుప్ప->ctx, -1, thread->thread_idx);
	duk_తీయి(కుప్ప->ctx); /* ... */

	/* We can now free the thread object */
	విడుదల(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(కుప్ప->ctx, 0);
	duk_gc(కుప్ప->ctx, DUK_GC_COMPACT);
	కుప్ప->సజీవ_దారాలు--;

	/* And if the heap should now go, blow it away */
	ఒకవేళ (కుప్ప->pending_destroy == true && కుప్ప->సజీవ_దారాలు == 0) {
		dukky_destroyheap(కుప్ప);
	}
}

/* exported interface documented in js.h */
శూన్య js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	ఒకవేళ (thread->లోపల_use == 0) {
		dukky_destroythread(thread);
	}
}

స్థిర శూన్య dukky_enter_thread(jsthread *thread)
{
	ప్రతిజ్ఞా(thread != ఖాళీ);
	thread->లోపల_use++;
}

స్థిర శూన్య dukky_leave_thread(jsthread *thread)
{
	ప్రతిజ్ఞా(thread != ఖాళీ);
	ప్రతిజ్ఞా(thread->లోపల_use > 0);

	thread->లోపల_use--;
	ఒకవేళ (thread->లోపల_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(శూన్య *udata)
{
#నిర్వచించు JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *కుప్ప = (jsheap *) udata;
	uint64_t now;

	(శూన్య) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	తిరిగి కుప్ప->exec_ప్రారంభం_సమయం != 0 &&
			now > (కుప్ప->exec_ప్రారంభం_సమయం + JS_EXEC_TIMEOUT_MS);
}

స్థిర శూన్య dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_శిఖరం(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_సురక్షిత_గమ్యం_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_తీయి(ctx);
	/* ..., errobj */
}

స్థిర శూన్య dukky_reset_ప్రారంభం_సమయం(duk_context *ctx)
{
	duk_జ్ఞాపకం_functions funcs;
	jsheap *కుప్ప;
	duk_get_జ్ఞాపకం_functions(ctx, &funcs);
	కుప్ప = funcs.udata;
	(శూన్య) nsu_getmonotonic_ms(&కుప్ప->exec_ప్రారంభం_సమయం);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_కొలత_t argc, bool reset_timeout)
{
	ఒకవేళ (reset_timeout) {
		dukky_reset_ప్రారంభం_సమయం(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	ఒకవేళ (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	తిరిగి ret;
}


శూన్య dukky_నెట్టు_generics(duk_context *ctx, స్థిరరాశి అక్షరం *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

స్థిర duk_int_t dukky_నెట్టు_context_dump(duk_context *ctx, శూన్య *udata)
{
	duk_నెట్టు_context_dump(ctx);
	తిరిగి 1;
}

శూన్య dukky_log_దొంతర_చట్రం(duk_context *ctx, స్థిరరాశి అక్షరం * reason)
{
	ఒకవేళ (duk_సురక్షిత_call(ctx, dukky_నెట్టు_context_dump, ఖాళీ, 0, 1) != 0) {
		duk_తీయి(ctx);
		duk_నెట్టు_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_సురక్షిత_గమ్యం_string(ctx, -1));
	duk_తీయి(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, స్థిరరాశి uint8_t *txt, size_t txtlen, స్థిరరాశి అక్షరం *name)
{
	bool ret = false;
	ప్రతిజ్ఞా(thread);

	ఒకవేళ (txt == ఖాళీ || txtlen == 0) {
		తిరిగి false;
	}

	ఒకవేళ (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		తిరిగి false;
	}

	dukky_enter_thread(thread);

	duk_సమితి_శిఖరం(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_ప్రారంభం_సమయం(CTX);
	ఒకవేళ (name != ఖాళీ) {
		duk_నెట్టు_string(CTX, name);
	} లేకపోతే {
		duk_నెట్టు_string(CTX, "?unknown source?");
	}
	ఒకవేళ (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (స్థిరరాశి అక్షరం *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		వెళ్ళు handle_error;
	}

	ఒకవేళ (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		వెళ్ళు handle_error;
	}

	ఒకవేళ (duk_get_శిఖరం(CTX) == 0) duk_నెట్టు_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	వెళ్ళు బహిర్గతం;

handle_error:
	dukky_dump_error(CTX);
బహిర్గతం:
	dukky_leave_thread(thread);
	తిరిగి ret;
}

స్థిర స్థిరరాశి అక్షరం* dukky_event_proto(dom_event *evt)
{
	స్థిరరాశి అక్షరం *ret = PROTO_NAME(EVENT);
	dom_string *type = ఖాళీ;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	ఒకవేళ (err != DOM_NO_ERR) {
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		వెళ్ళు బహిర్గతం;
	} లేకపోతే ఒకవేళ (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		వెళ్ళు బహిర్గతం;
	} లేకపోతే ఒకవేళ (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		వెళ్ళు బహిర్గతం;
	}

బహిర్గతం:
	ఒకవేళ (type != ఖాళీ) {
		dom_string_unref(type);
	}

	తిరిగి ret;
}

/*** New style event handling ***/

శూన్య dukky_నెట్టు_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_నెట్టు_సూచిక(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	ఒకవేళ (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_తీయి(ctx);
		/* ... events */
		duk_నెట్టు_సూచిక(ctx, evt);
		ఒకవేళ (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_తీయి(ctx);
			/* ... events */
			duk_నెట్టు_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_నెట్టు_సూచిక(ctx, evt);
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

స్థిర శూన్య dukky_నెట్టు_handler_సంకేతం(duk_context *ctx, dom_string *name,
				     dom_event_లక్ష్యం *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_కణుపు_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	ఒకవేళ (et == ఖాళీ) {
		duk_నెట్టు_lstring(ctx, "", 0);
		తిరిగి;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_కణుపు_get_కణుపు_type(et, &ntype);
	ఒకవేళ (exc != DOM_NO_ERR) {
		duk_నెట్టు_lstring(ctx, "", 0);
		తిరిగి;
	}

	ఒకవేళ (ntype != DOM_ELEMENT_కణుపు) {
		duk_నెట్టు_lstring(ctx, "", 0);
		తిరిగి;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	ఒకవేళ (exc != DOM_NO_ERR) {
		duk_నెట్టు_lstring(ctx, "", 0);
		తిరిగి;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	ఒకవేళ ((exc != DOM_NO_ERR) || (val == ఖాళీ)) {
		dom_string_unref(onname);
		duk_నెట్టు_lstring(ctx, "", 0);
		తిరిగి;
	}

	dom_string_unref(onname);
	duk_నెట్టు_lstring(ctx, dom_string_దత్తాంశం(val), dom_string_నిడివి(val));
	dom_string_unref(val);
}

bool dukky_get_current_విలువ_యొక్క_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_లక్ష్యం *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_నెట్టు_lstring(ctx, dom_string_దత్తాంశం(name), dom_string_నిడివి(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	ఒకవేళ (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_తీయి_2(ctx);
		/* ... node */
		dukky_నెట్టు_handler_సంకేతం(ctx, name, et);
		/* ... node handlercode? */
		/* TODO: If this is null, clean up and propagate */
		/* ... node handlercode */
		/** @todo This is entirely wrong, but it's hard to get right */
		duk_నెట్టు_string(ctx, "function (event) {");
		/* ... node handlercode prefix */
		duk_చొప్పించు(ctx, -2);
		/* ... node prefix handlercode */
		duk_నెట్టు_string(ctx, "}");
		/* ... node prefix handlercode suffix */
		duk_concat(ctx, 3);
		/* ... node fullhandlersrc */
		duk_నెట్టు_string(ctx, "internal raw uncompiled handler");
		/* ... node fullhandlersrc filename */
		ఒకవేళ (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_తీయి_2(ctx);
			తిరిగి false;
		}
		/* ... node handler */
		duk_చొప్పించు(ctx, -2);
		/* ... handler node */
	} లేకపోతే {
		/* ... node handlers handler */
		duk_చొప్పించు(ctx, -3);
		/* ... handler node handlers */
		duk_తీయి(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	తిరిగి true;
}

స్థిర శూన్య dukky_generic_event_handler(dom_event *evt, శూన్య *pw)
{
	duk_context *ctx = (duk_context *)pw;
	dom_string *name;
	dom_exception exc;
	dom_event_లక్ష్యం *targ;
	dom_event_flow_phase phase;
	duk_uarridx_t idx;
	event_listener_flags flags;

	NSLOG(dukky, DEBUG, "Handling an event in duktape interface...");
	exc = dom_event_get_type(evt, &name);
	ఒకవేళ (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		తిరిగి;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (పూర్ణాంకం)dom_string_నిడివి(name),
	      dom_string_దత్తాంశం(name));
	exc = dom_event_get_event_phase(evt, &phase);
	ఒకవేళ (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		తిరిగి;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_లక్ష్యం ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (పూర్ణాంకం)phase);

	exc = dom_event_get_current_లక్ష్యం(evt, &targ);
	ఒకవేళ (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		తిరిగి;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	ఒకవేళ (phase == DOM_CAPTURING_PHASE)
		వెళ్ళు handle_extras;

	/* ... */
	ఒకవేళ (dukky_నెట్టు_కణుపు(ctx, (dom_కణుపు *)targ) == false) {
		dom_string_unref(name);
		dom_కణుపు_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		తిరిగి;
	}
	/* ... node */
	ఒకవేళ (dukky_get_current_విలువ_యొక్క_event_handler(
		    ctx, name, (dom_event_లక్ష్యం *)targ) == false) {
		/* ... */
		వెళ్ళు handle_extras;
	}
	/* ... handler node */
	dukky_నెట్టు_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_ప్రారంభం_సమయం(ctx);
	ఒకవేళ (duk_pcall_పద్ధతి(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_తక్షణ_ప్రసరణ(evt);
		ఒకవేళ (exc != DOM_NO_ERR)
			NSLOG(dukky, DEBUG,
			      "WORSE! could not stop propagation");
		duk_get_prop_string(ctx, -1, "name");
		duk_get_prop_string(ctx, -2, "message");
		duk_get_prop_string(ctx, -3, "fileName");
		duk_get_prop_string(ctx, -4, "lineNumber");
		duk_get_prop_string(ctx, -5, "stack");
		/* ... err name message fileName lineNumber stack */
		NSLOG(dukky, DEBUG, "Uncaught error in JS: %s: %s",
		      duk_సురక్షిత_గమ్యం_string(ctx, -5),
		      duk_సురక్షిత_గమ్యం_string(ctx, -4));
		NSLOG(dukky, INFO, "              was at: %s line %s",
		      duk_సురక్షిత_గమ్యం_string(ctx, -3),
		      duk_సురక్షిత_గమ్యం_string(ctx, -2));
		NSLOG(dukky, INFO, "         Stack trace: %s",
		      duk_సురక్షిత_గమ్యం_string(ctx, -1));

		duk_తీయి_n(ctx, 6);
		/* ... */
		వెళ్ళు handle_extras;
	}
	/* ... result */
	ఒకవేళ (duk_is_boolean(ctx, -1) &&
	    duk_గమ్యం_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_తీయి(ctx);
handle_extras:
	/* ... */
	duk_నెట్టు_lstring(ctx, dom_string_దత్తాంశం(name), dom_string_నిడివి(name));
	dukky_నెట్టు_కణుపు(ctx, (dom_కణుపు *)targ);
	/* ... type node */
	ఒకవేళ (dukky_event_లక్ష్యం_నెట్టు_listeners(ctx, true)) {
		/* Nothing to do */
		duk_తీయి(ctx);
		వెళ్ళు బహిర్గతం;
	}
	/* ... sublisteners */
	duk_నెట్టు_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	అయితే (duk_get_prop_సూచీ(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_సూచీ(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		ఒకవేళ ((event_listener_flags)duk_గమ్యం_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_తీయి(ctx);
			/* ... subl copy handler flags */
		}
		duk_తీయి(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_సూచీ(ctx, -2, idx);
		/* ... sublisteners copy */
		idx++;
	}
	/* ... sublisteners copy undefined */
	duk_తీయి(ctx);
	/* ... sublisteners copy */
	duk_చొప్పించు(ctx, -2);
	/* ... copy sublisteners */
	duk_తీయి(ctx);
	/* ... copy */
	idx = 0;
	అయితే (duk_get_prop_సూచీ(ctx, -1, idx++)) {
		/* ... copy handler */
		ఒకవేళ (duk_get_prop_సూచీ(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_తీయి_2(ctx);
			కొనసాగు;
		}
		duk_తీయి(ctx);
		duk_get_prop_సూచీ(ctx, -1, 0);
		duk_get_prop_సూచీ(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_తీయి(ctx);
		/* ... copy handler callback */
		ఒకవేళ (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_తీయి_2(ctx);
			/* ... copy */
			కొనసాగు;
		}
		/* ... copy handler callback */
		dukky_నెట్టు_కణుపు(ctx, (dom_కణుపు *)targ);
		/* ... copy handler callback node */
		dukky_నెట్టు_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_ప్రారంభం_సమయం(ctx);
		ఒకవేళ (duk_pcall_పద్ధతి(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_తక్షణ_ప్రసరణ(evt);
			ఒకవేళ (exc != DOM_NO_ERR)
				NSLOG(dukky, DEBUG,
				      "WORSE! could not stop propagation");
			duk_get_prop_string(ctx, -1, "name");
			duk_get_prop_string(ctx, -2, "message");
			duk_get_prop_string(ctx, -3, "fileName");
			duk_get_prop_string(ctx, -4, "lineNumber");
			duk_get_prop_string(ctx, -5, "stack");
			/* ... err name message fileName lineNumber stack */
			NSLOG(dukky, DEBUG, "Uncaught error in JS: %s: %s",
			      duk_సురక్షిత_గమ్యం_string(ctx, -5),
			      duk_సురక్షిత_గమ్యం_string(ctx, -4));
			NSLOG(dukky, DEBUG,
			      "              was at: %s line %s",
			      duk_సురక్షిత_గమ్యం_string(ctx, -3),
			      duk_సురక్షిత_గమ్యం_string(ctx, -2));
			NSLOG(dukky, DEBUG, "         Stack trace: %s",
			      duk_సురక్షిత_గమ్యం_string(ctx, -1));

			duk_తీయి_n(ctx, 7);
			/* ... copy */
			కొనసాగు;
		}
		/* ... copy handler result */
		ఒకవేళ (duk_is_boolean(ctx, -1) &&
		    duk_గమ్యం_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_తీయి_2(ctx);
		/* ... copy */
	}
	duk_తీయి_2(ctx);
బహిర్గతం:
	/* ... */
	dom_కణుపు_unref(targ);
	dom_string_unref(name);
}

శూన్య dukky_రిజిస్టరు_event_listener_for(duk_context *ctx,
				       నిర్మాణం dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = ఖాళీ;
	dom_exception exc;

	/* ... */
	ఒకవేళ (ele == ఖాళీ) {
		/* A null element is the Window object */
		duk_నెట్టు_global_object(ctx);
	} లేకపోతే {
		/* Non null elements must be pushed as a node object */
		ఒకవేళ (dukky_నెట్టు_కణుపు(ctx, (నిర్మాణం dom_కణుపు *)ele) == false)
			తిరిగి;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_నెట్టు_lstring(ctx, dom_string_దత్తాంశం(name), dom_string_నిడివి(name));
	/* ... node handlers name */
	ఒకవేళ (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_తీయి_2(ctx);
		/* ... */
		తిరిగి;
	}
	/* ... node handlers */
	duk_నెట్టు_lstring(ctx, dom_string_దత్తాంశం(name), dom_string_నిడివి(name));
	/* ... node handlers name */
	duk_నెట్టు_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_తీయి_2(ctx);
	/* ... */
	ఒకవేళ (ele == ఖాళీ) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		తిరిగి;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	ఒకవేళ (exc != DOM_NO_ERR) తిరిగి;
	exc = dom_event_లక్ష్యం_కూడు_event_listener(
		ele, name, listen, capture);
	ఒకవేళ (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (పూర్ణాంకం)dom_string_నిడివి(name), dom_string_దత్తాంశం(name));
	} లేకపోతే {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (పూర్ణాంకం)dom_string_నిడివి(name), dom_string_దత్తాంశం(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_లక్ష్యం_నెట్టు_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	ఒకవేళ (duk_is_undefined(ctx, -1)) {
		/* ... type this null */
		duk_తీయి(ctx);
		duk_నెట్టు_object(ctx);
		duk_dup(ctx, -1);
		/* ... type this listeners listeners */
		duk_put_prop_string(ctx, -3, EVENT_LISTENER_JS_MAGIC);
		/* ... type this listeners */
	}
	/* ... type this listeners */
	duk_చొప్పించు(ctx, -3);
	/* ... listeners type this */
	duk_తీయి(ctx);
	/* ... listeners type */
	duk_dup(ctx, -1);
	/* ... listeners type type */
	duk_get_prop(ctx, -3);
	/* ... listeners type ??? */
	ఒకవేళ (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		ఒకవేళ (dont_create == true) {
			duk_తీయి_3(ctx);
			duk_నెట్టు_undefined(ctx);
			తిరిగి true;
		}
		duk_తీయి(ctx);
		duk_నెట్టు_array(ctx);
		duk_dup(ctx, -2);
		duk_dup(ctx, -2);
		/* ... listeners type sublisteners type sublisteners */
		duk_put_prop(ctx, -5);
		/* ... listeners type sublisteners */
		ret = true;
	}
	duk_చొప్పించు(ctx, -3);
	/* ... sublisteners listeners type */
	duk_తీయి_2(ctx);
	/* ... sublisteners */
	తిరిగి ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
శూన్య dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	అయితే (duk_get_prop_సూచీ(ctx, -1, idx + 1)) {
		duk_put_prop_సూచీ(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_సూచీ(ctx, -2, idx + 1);
	duk_తీయి(ctx);
}


శూన్య js_handle_new_element(jsthread *thread, నిర్మాణం dom_element *కణుపు)
{
	ప్రతిజ్ఞా(thread);
	ప్రతిజ్ఞా(కణుపు);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = ఖాళీ;
	dom_string *కీలకం = ఖాళీ;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_కణుపు_get_కణుపు_name(కణుపు, &nodename);
	ఒకవేళ (exc != DOM_NO_ERR) తిరిగి;

	ఒకవేళ (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_కణుపు_get_లక్షణాలు(కణుపు, &map);
	ఒకవేళ (exc != DOM_NO_ERR) తిరిగి;
	ఒకవేళ (map == ఖాళీ) తిరిగి;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_నిడివి(map, &siz);
	ఒకవేళ (exc != DOM_NO_ERR) వెళ్ళు బహిర్గతం;

	కొరకు (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_అంశం(map, idx, &attr);
		ఒకవేళ (exc != DOM_NO_ERR) వెళ్ళు బహిర్గతం;
		exc = dom_attr_get_name(attr, &కీలకం);
		ఒకవేళ (exc != DOM_NO_ERR) వెళ్ళు బహిర్గతం;
		ఒకవేళ (is_body && (
			    కీలకం == corestring_dom_onblur ||
			    కీలకం == corestring_dom_onerror ||
			    కీలకం == corestring_dom_onfocus ||
			    కీలకం == corestring_dom_onload ||
			    కీలకం == corestring_dom_onresize ||
			    కీలకం == corestring_dom_onscroll)) {
			/* This is a forwarded event, it doesn't matter,
			 * we should skip registering for it and later
			 * we will register it for Window itself
			 */
			వెళ్ళు skip_రిజిస్టరు;
		}
		ఒకవేళ (dom_string_నిడివి(కీలకం) > 2) {
			/* Can be on* */
			స్థిరరాశి uint8_t *దత్తాంశం = (స్థిరరాశి uint8_t *)dom_string_దత్తాంశం(కీలకం);
			ఒకవేళ (దత్తాంశం[0] == 'o' && దత్తాంశం[1] == 'n') {
				dom_string *sub = ఖాళీ;
				exc = dom_string_substr(
					కీలకం, 2, dom_string_నిడివి(కీలకం),
					&sub);
				ఒకవేళ (exc == DOM_NO_ERR) {
					dukky_రిజిస్టరు_event_listener_for(
						CTX, కణుపు, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_రిజిస్టరు:
		dom_string_unref(కీలకం); కీలకం = ఖాళీ;
		dom_కణుపు_unref(attr); attr = ఖాళీ;
	}

బహిర్గతం:
	ఒకవేళ (కీలకం != ఖాళీ)
		dom_string_unref(కీలకం);

	ఒకవేళ (attr != ఖాళీ)
		dom_కణుపు_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

శూన్య js_event_cleanup(jsthread *thread, నిర్మాణం dom_event *evt)
{
	ప్రతిజ్ఞా(thread);
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, EVENT_MAGIC);
	/* ... EVENT_MAP */
	duk_నెట్టు_సూచిక(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_తీయి(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, స్థిరరాశి అక్షరం *type, నిర్మాణం dom_document *doc, నిర్మాణం dom_కణుపు *లక్ష్యం)
{
	dom_exception exc;
	dom_event *evt;
	dom_event_లక్ష్యం *body;

	NSLOG(dukky, DEBUG, "Event: %s (doc=%p, target=%p)", type, doc,
	      లక్ష్యం);

	/** @todo Make this more generic, this only handles load and only
	 * targetting the window, so that we actually stand a chance of
	 * getting 3.4 out.
	 */

	ఒకవేళ (లక్ష్యం != ఖాళీ)
		/* Swallow non-Window-targetted events quietly */
		తిరిగి true;

	ఒకవేళ (strcmp(type, "load") != 0)
		/* Swallow non-load events quietly */
		తిరిగి true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	ఒకవేళ (exc != DOM_NO_ERR) తిరిగి true;
	exc = dom_event_init(evt, corestring_dom_ఎక్కించు, false, false);
	ఒకవేళ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		తిరిగి true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_నెట్టు_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	ఒకవేళ (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_తీయి(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		ఒకవేళ (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			తిరిగి true;
		}
		dukky_నెట్టు_కణుపు(CTX, (నిర్మాణం dom_కణుపు *)body);
		/* ... handlers bodynode */
		ఒకవేళ (dukky_get_current_విలువ_యొక్క_event_handler(
			    CTX, corestring_dom_ఎక్కించు, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_కణుపు_unref(body);
			/* ... handlers */
			duk_తీయి(CTX);
			dukky_leave_thread(thread);
			తిరిగి true;
		}
		/* Unref the body, we don't need it any more */
		dom_కణుపు_unref(body);
		/* ... handlers handler bodynode */
		duk_తీయి(CTX);
	}
	/* ... handlers handler */
	duk_చొప్పించు(CTX, -2);
	/* ... handler handlers */
	duk_తీయి(CTX);
	/* ... handler */
	duk_నెట్టు_global_object(CTX);
	/* ... handler Window */
	dukky_నెట్టు_event(CTX, evt);
	/* ... handler Window event */
	dukky_reset_ప్రారంభం_సమయం(CTX);
	ఒకవేళ (duk_pcall_పద్ధతి(CTX, 1) != 0) {
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
		      duk_సురక్షిత_గమ్యం_string(CTX, -5),
		      duk_సురక్షిత_గమ్యం_string(CTX, -4));
		NSLOG(dukky, DEBUG, "              was at: %s line %s",
		      duk_సురక్షిత_గమ్యం_string(CTX, -3),
		      duk_సురక్షిత_గమ్యం_string(CTX, -2));
		NSLOG(dukky, DEBUG, "         Stack trace: %s",
		      duk_సురక్షిత_గమ్యం_string(CTX, -1));

		duk_తీయి_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		తిరిగి true;
	}
	/* ... result */
	duk_తీయి(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	తిరిగి true;
}
