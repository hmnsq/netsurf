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

#अन्तर्भाव <stdint.h>
#अन्तर्भाव <nsutils/time.h>

#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "content/content.h"

#अन्तर्भाव "javascript/js.h"
#अन्तर्भाव "javascript/content.h"

#अन्तर्भाव "duktape/binding.h"
#अन्तर्भाव "duktape/generics.js.inc"
#अन्तर्भाव "duktape/polyfill.js.inc"

#अन्तर्भाव "duktape.h"
#अन्तर्भाव "dukky.h"

#अन्तर्भाव <dom/dom.h>

#परिभाषय EVENT_MAGIC MAGIC(EVENT_MAP)
#परिभाषय HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#परिभाषय HANDLER_MAGIC MAGIC(HANDLER_MAP)
#परिभाषय EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#परिभाषय GENERICS_MAGIC MAGIC(GENERICS_सारणी)
#परिभाषय THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
संरचना jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t अग्र_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	अचिह्नित पूर्णाङ्क जीवितम्_तन्तवः; /**< number of live threads */
	uint64_t exec_start_time;
};

/**
 * dukky javascript thread
 */
संरचना jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	अचिह्नित पूर्णाङ्क अन्तर्_use; /**< The number of times this thread is in use */
	jsheap *heap; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

स्थिर duk_ret_t dukky_populate_object(duk_context *ctx, शून्य *udata)
{
	/* ... obj args protoname nargs */
	पूर्णाङ्क nargs = duk_get_int(ctx, -1);
	duk_निष्कासय(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	यदि (duk_is_undefined(ctx, -1)) {
		NSLOG(dukky, WARNING,
		      "Unable to find dukky prototype for `%s` - falling back to HTMLUnknownElement",
		      duk_get_string(ctx, -3) + 2 /* Skip the two unprintables */);
		duk_निष्कासय(ctx);
		duk_प्रक्षिप_string(ctx, PROTO_NAME(HTMLUNKNOWNELEMENT));
		duk_get_prop(ctx, -2);
	}
	/* ... obj args protoname prototab proto */
	duk_remove(ctx, -3);
	/* ... obj args prototab proto */
	duk_dup(ctx, -1);
	/* ... obj args prototab proto proto */
	duk_समुच्चयः_prototype(ctx, -(nargs+4));
	/* ... obj[proto] args prototab proto */
	duk_get_prop_string(ctx, -1, INIT_MAGIC);
	/* ... obj[proto] args prototab proto initfn */
	duk_निवेशय(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_निष्कासय_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	प्रत्यागम 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, ध्रुव वर्ण *name, पूर्णाङ्क तर्काः)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, तर्काः);
	/* ... args */
	duk_प्रक्षिप_object(ctx);
	/* ... args obj */
	duk_प्रक्षिप_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
	/* ... args obj */
	duk_प्रक्षिप_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
	/* ... args obj */
	duk_निवेशय(ctx, -(तर्काः+1));
	/* ... obj args */
	duk_प्रक्षिप_string(ctx, name);
	/* ... obj args name */
	duk_प्रक्षिप_int(ctx, तर्काः);
	/* ... obj args name nargs */
	यदि ((ret = duk_safe_call(ctx, dukky_populate_object, रिक्त, तर्काः + 3, 1))
	    != DUK_EXEC_SUCCESS)
		प्रत्यागम ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	प्रत्यागम DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_प्रक्षिप_ग्रन्थिः_stacked(duk_context *ctx)
{
	पूर्णाङ्क शिखरम्_at_fail = duk_get_शिखरम्(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, ग्रन्थिः_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	यदि (duk_is_undefined(ctx, -1)) {
		/* ... nodeptr klass nodes undefined */
		duk_निष्कासय(ctx);
		/* ... nodeptr klass nodes */
		duk_प्रक्षिप_object(ctx);
		/* ... nodeptr klass nodes obj */
		duk_प्रक्षिप_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_प्रक्षिप_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr klass */
		duk_प्रक्षिप_int(ctx, 1);
		/* ... nodeptr klass nodes obj nodeptr klass 1 */
		यदि (duk_safe_call(ctx, dukky_populate_object, रिक्त, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_समुच्चयः_शिखरम्(ctx, शिखरम्_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			प्रत्यागम false;
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
	duk_निवेशय(ctx, -4);
	/* ... node nodeptr klass nodes */
	duk_निष्कासय_3(ctx);
	/* ... node */
	यदि (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		ध्रुव वर्ण * what = duk_safe_गन्तव्यम्_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_निष्कासय(ctx);
	}
	प्रत्यागम true;
}

#परिभाषय समुच्चयः_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

स्थिर शून्य dukky_html_element_class_from_चिह्नकम्_type(dom_html_element_type type,
		ध्रुव वर्ण **html_class, size_t *html_class_len)
{
	विकल्प(type) {
	पक्ष DOM_HTML_ELEMENT_TYPE_HTML:
		समुच्चयः_HTML_CLASS(HTML)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_शीर्ष:
		समुच्चयः_HTML_CLASS(शीर्ष)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_META:
		समुच्चयः_HTML_CLASS(META)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_आधारः:
		समुच्चयः_HTML_CLASS(आधारः_2)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_TITLE:
		समुच्चयः_HTML_CLASS(TITLE)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_BODY:
		समुच्चयः_HTML_CLASS(BODY)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_DIV:
		समुच्चयः_HTML_CLASS(DIV)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_FORM:
		समुच्चयः_HTML_CLASS(FORM)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_LINK:
		समुच्चयः_HTML_CLASS(LINK)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_BUTTON:
		समुच्चयः_HTML_CLASS(BUTTON)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_निवेशः:
		समुच्चयः_HTML_CLASS(निवेशः_2)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		समुच्चयः_HTML_CLASS(TEXTAREA)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		समुच्चयः_HTML_CLASS(OPTGROUP)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_OPTION:
		समुच्चयः_HTML_CLASS(OPTION)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_SELECT:
		समुच्चयः_HTML_CLASS(SELECT)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_HR:
		समुच्चयः_HTML_CLASS(HR)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_DL:
		समुच्चयः_HTML_CLASS(DLIST)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_DIR:
		समुच्चयः_HTML_CLASS(DIRECTORY)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_MENU:
		समुच्चयः_HTML_CLASS(MENU)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_FIELDSET:
		समुच्चयः_HTML_CLASS(FIELDSET)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_LEGEND:
		समुच्चयः_HTML_CLASS(LEGEND)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_P:
		समुच्चयः_HTML_CLASS(PARAGRAPH)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_H1:
	पक्ष DOM_HTML_ELEMENT_TYPE_H2:
	पक्ष DOM_HTML_ELEMENT_TYPE_H3:
	पक्ष DOM_HTML_ELEMENT_TYPE_H4:
	पक्ष DOM_HTML_ELEMENT_TYPE_H5:
	पक्ष DOM_HTML_ELEMENT_TYPE_H6:
		समुच्चयः_HTML_CLASS(HEADING)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	पक्ष DOM_HTML_ELEMENT_TYPE_Q:
		समुच्चयः_HTML_CLASS(QUOTE)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_PRE:
		समुच्चयः_HTML_CLASS(PRE)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_BR:
		समुच्चयः_HTML_CLASS(BR)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_LABEL:
		समुच्चयः_HTML_CLASS(LABEL)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_UL:
		समुच्चयः_HTML_CLASS(ULIST)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_OL:
		समुच्चयः_HTML_CLASS(OLIST)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_LI:
		समुच्चयः_HTML_CLASS(LI)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_FONT:
		समुच्चयः_HTML_CLASS(FONT)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_DEL:
	पक्ष DOM_HTML_ELEMENT_TYPE_INS:
		समुच्चयः_HTML_CLASS(MOD)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_A:
		समुच्चयः_HTML_CLASS(ANCHOR)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_BASEFONT:
		समुच्चयः_HTML_CLASS(BASEFONT)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_IMG:
		समुच्चयः_HTML_CLASS(IMAGE)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_OBJECT:
		समुच्चयः_HTML_CLASS(OBJECT)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_PARAM:
		समुच्चयः_HTML_CLASS(PARAM)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_APPLET:
		समुच्चयः_HTML_CLASS(APPLET)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_MAP:
		समुच्चयः_HTML_CLASS(MAP)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_AREA:
		समुच्चयः_HTML_CLASS(AREA)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_SCRIPT:
		समुच्चयः_HTML_CLASS(SCRIPT)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_CAPTION:
		समुच्चयः_HTML_CLASS(TABLECAPTION)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_TD:
	पक्ष DOM_HTML_ELEMENT_TYPE_TH:
		समुच्चयः_HTML_CLASS(TABLECELL)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_COL:
	पक्ष DOM_HTML_ELEMENT_TYPE_COLGROUP:
		समुच्चयः_HTML_CLASS(TABLECOL)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_THEAD:
	पक्ष DOM_HTML_ELEMENT_TYPE_TBODY:
	पक्ष DOM_HTML_ELEMENT_TYPE_TFOOT:
		समुच्चयः_HTML_CLASS(TABLESECTION)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_सारणी:
		समुच्चयः_HTML_CLASS(सारणी_2)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_TR:
		समुच्चयः_HTML_CLASS(TABLEROW)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_STYLE:
		समुच्चयः_HTML_CLASS(STYLE)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_FRAMESET:
		समुच्चयः_HTML_CLASS(FRAMESET)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_FRAME:
		समुच्चयः_HTML_CLASS(FRAME)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_IFRAME:
		समुच्चयः_HTML_CLASS(IFRAME)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_ISINDEX:
		समुच्चयः_HTML_CLASS(ISINDEX)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_CANVAS:
		समुच्चयः_HTML_CLASS(CANVAS)
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_गणनम्:
		प्रतिज्ञा(type != DOM_HTML_ELEMENT_TYPE_गणनम्);
		fallthrough;
	पक्ष DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		समुच्चयः_HTML_CLASS(UNKNOWN)
		भङ्ग;
	स्वभाव:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		भङ्ग;
	}
	प्रत्यागम;
}

#अपरिभाषय समुच्चयः_HTML_CLASS

स्थिर शून्य
dukky_प्रक्षिप_ग्रन्थिः_klass(duk_context *ctx, संरचना dom_ग्रन्थिः *ग्रन्थिः)
{
	dom_ग्रन्थिः_type nodetype;
	dom_exception err;

	err = dom_ग्रन्थिः_get_ग्रन्थिः_type(ग्रन्थिः, &nodetype);
	यदि (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_प्रक्षिप_string(ctx, PROTO_NAME(ग्रन्थिः_2));
		प्रत्यागम;
	}

	विकल्प(nodetype) {
	पक्ष DOM_ELEMENT_ग्रन्थिः: {
		dom_string *namespace;
		dom_html_element_type type;
		ध्रुव वर्ण *html_class;
		size_t html_class_len;
		err = dom_ग्रन्थिः_get_namespace(ग्रन्थिः, &namespace);
		यदि (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_प्रक्षिप_string(ctx, PROTO_NAME(ELEMENT));
			भङ्ग;
		}
		यदि (namespace == रिक्त) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_प्रक्षिप_string(ctx, PROTO_NAME(ELEMENT));
			भङ्ग;
		}

		यदि (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_प्रक्षिप_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			भङ्ग;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_चिह्नकम्_type(ग्रन्थिः, &type);
		यदि (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_चिह्नकम्_type(type,
				&html_class, &html_class_len);

		duk_प्रक्षिप_lstring(ctx, html_class, html_class_len);
		भङ्ग;
	}
	पक्ष DOM_TEXT_ग्रन्थिः:
		duk_प्रक्षिप_string(ctx, PROTO_NAME(TEXT));
		भङ्ग;
	पक्ष DOM_COMMENT_ग्रन्थिः:
		duk_प्रक्षिप_string(ctx, PROTO_NAME(COMMENT));
		भङ्ग;
	पक्ष DOM_DOCUMENT_ग्रन्थिः:
		duk_प्रक्षिप_string(ctx, PROTO_NAME(DOCUMENT));
		भङ्ग;
	पक्ष DOM_ATTRIBUTE_ग्रन्थिः:
	पक्ष DOM_PROCESSING_INSTRUCTION_ग्रन्थिः:
	पक्ष DOM_DOCUMENT_TYPE_ग्रन्थिः:
	पक्ष DOM_DOCUMENT_FRAGMENT_ग्रन्थिः:
	पक्ष DOM_NOTATION_ग्रन्थिः:
	पक्ष DOM_ENTITY_REFERENCE_ग्रन्थिः:
	पक्ष DOM_ENTITY_ग्रन्थिः:
	पक्ष DOM_CDATA_SECTION_ग्रन्थिः:
	स्वभाव:
		/* Oh bum, just node then */
		duk_प्रक्षिप_string(ctx, PROTO_NAME(ग्रन्थिः_2));
	}
}

duk_bool_t
dukky_प्रक्षिप_ग्रन्थिः(duk_context *ctx, संरचना dom_ग्रन्थिः *ग्रन्थिः)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", ग्रन्थिः);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, ग्रन्थिः_MAGIC);
	/* ... nodes */
	duk_प्रक्षिप_pointer(ctx, ग्रन्थिः);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	यदि (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_निवेशय(ctx, -2);
		/* ... node nodes */
		duk_निष्कासय(ctx);
		/* ... node */
		यदि (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			ध्रुव वर्ण * what = duk_safe_गन्तव्यम्_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_निष्कासय(ctx);
		}
		प्रत्यागम true;
	}
	/* ... nodes undefined */
	duk_निष्कासय_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_प्रक्षिप_pointer(ctx, ग्रन्थिः);
	/* ... nodeptr */
	dukky_प्रक्षिप_ग्रन्थिः_klass(ctx, ग्रन्थिः);
	/* ... nodeptr klass */
	प्रत्यागम dukky_प्रक्षिप_ग्रन्थिः_stacked(ctx);
}

स्थिर duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	प्रत्यागम duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

शून्य
dukky_inject_not_ctr(duk_context *ctx, पूर्णाङ्क idx, ध्रुव वर्ण *name)
{
	/* ... p[idx] ... proto */
	duk_प्रक्षिप_c_function(ctx, dukky_bad_constructor, 0);
	/* ... p[idx] ... proto cons */
	duk_निवेशय(ctx, -2);
	/* ... p[idx] ... cons proto */
	duk_put_prop_string(ctx, -2, "prototype");
	/* ... p[idx] ... cons[proto] */
	duk_put_prop_string(ctx, idx, name);
	/* ... p ... */
	प्रत्यागम;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

स्थिर शून्य *dukky_alloc_function(शून्य *udata, duk_आकार_t आकार)
{
	यदि (आकार == 0)
		प्रत्यागम रिक्त;

	प्रत्यागम स्मृतिं_देहि(आकार);
}

स्थिर शून्य *dukky_realloc_function(शून्य *udata, शून्य *ptr, duk_आकार_t आकार)
{
	यदि (ptr == रिक्त && आकार == 0)
		प्रत्यागम रिक्त;

	यदि (आकार == 0) {
		मुक्त(ptr);
		प्रत्यागम रिक्त;
	}

	प्रत्यागम realloc(ptr, आकार);
}


स्थिर शून्य dukky_free_function(शून्य *udata, शून्य *ptr)
{
	यदि (ptr != रिक्त)
		मुक्त(ptr);
}

/* exported interface documented in js.h */
शून्य js_initialise(शून्य)
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
शून्य js_finalise(शून्य)
{
	/* NADA for now */
}


/* exported interface documented in js.h */
nserror
js_newheap(पूर्णाङ्क timeout, jsheap **heap)
{
	duk_context *ctx;
	jsheap *ret = शून्यस्मृतिं_देहि(1, परिमाण(*ret));
	*heap = रिक्त;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	यदि (ret == रिक्त) प्रत्यागम NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_heap(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		रिक्त);
	यदि (ret->ctx == रिक्त) { मुक्त(ret); प्रत्यागम NSERROR_NOMEM; }
	/* Create the prototype stuffs */
	duk_प्रक्षिप_global_object(ctx);
	duk_प्रक्षिप_boolean(ctx, true);
	duk_put_prop_string(ctx, -2, "protos");
	duk_put_global_string(ctx, PROTO_MAGIC);
	/* Create prototypes here */
	dukky_create_prototypes(ctx);
	/* Now create the thread map */
	duk_प्रक्षिप_object(ctx);
	duk_put_global_string(ctx, THREAD_MAP);

	*heap = ret;
	प्रत्यागम NSERROR_OK;
}


स्थिर शून्य dukky_destroyheap(jsheap *heap)
{
	प्रतिज्ञा(heap->pending_destroy == true);
	प्रतिज्ञा(heap->जीवितम्_तन्तवः == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_heap(heap->ctx);
	मुक्त(heap);
}

/* exported interface documented in js.h */
शून्य js_destroyheap(jsheap *heap)
{
	heap->pending_destroy = true;
	यदि (heap->जीवितम्_तन्तवः == 0) {
		dukky_destroyheap(heap);
	}
}

/* Just for here, the CTX is in ret, not thread */
#परिभाषय CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *heap, शून्य *win_priv, शून्य *doc_priv, jsthread **thread)
{
	jsthread *ret;
	प्रतिज्ञा(heap != रिक्त);
	प्रतिज्ञा(heap->pending_destroy == false);

	ret = शून्यस्मृतिं_देहि(1, परिमाण (*ret));
	यदि (ret == रिक्त) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		प्रत्यागम NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_प्रक्षिप_thread(heap->ctx); /* ... threads thread */
	ret->heap = heap;
	ret->ctx = duk_require_context(heap->ctx, -1);
	ret->thread_idx = heap->अग्र_thread++;
	duk_put_prop_सूचकांक(heap->ctx, -2, ret->thread_idx);
	heap->जीवितम्_तन्तवः++;
	duk_निष्कासय(heap->ctx); /* ... */
	duk_प्रक्षिप_int(CTX, 0);
	duk_प्रक्षिप_int(CTX, 1);
	duk_प्रक्षिप_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_प्रक्षिप_pointer(CTX, win_priv);
	duk_प्रक्षिप_pointer(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(गवाक्षः), 2);
	duk_प्रक्षिप_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_समुच्चयः_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_प्रक्षिप_object(CTX);
	duk_प्रक्षिप_pointer(CTX, रिक्त);
	duk_प्रक्षिप_null(CTX);
	duk_put_prop(CTX, -3);
	duk_put_global_string(CTX, ग्रन्थिः_MAGIC);

	/* And now the event mapping table */
	duk_प्रक्षिप_object(CTX);
	duk_put_global_string(CTX, EVENT_MAGIC);

	/* Now load the polyfills */
	/* ... */
	duk_प्रक्षिप_string(CTX, "polyfill.js");
	/* ..., polyfill.js */
	यदि (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ध्रुव वर्ण *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_गन्तव्यम्_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		प्रत्यागम NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	यदि (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		प्रत्यागम NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_निष्कासय(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_प्रक्षिप_string(CTX, "generics.js");
	/* ..., generics.js */
	यदि (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ध्रुव वर्ण *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_गन्तव्यम्_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		प्रत्यागम NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	यदि (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		प्रत्यागम NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_निष्कासय(CTX);
	/* ... */
	duk_प्रक्षिप_global_object(CTX);
	/* ..., Win */
	duk_get_prop_string(CTX, -1, "NetSurf");
	/* ..., Win, NetSurf */
	duk_put_global_string(CTX, GENERICS_MAGIC);
	/* ..., Win */
	duk_del_prop_string(CTX, -1, "NetSurf");
	duk_निष्कासय(CTX);
	/* ... */

	dukky_log_राशिः_frame(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, heap);
	*thread = ret;

	प्रत्यागम NSERROR_OK;
}

/* Now switch to the long term CTX behaviour */
#अपरिभाषय CTX
#परिभाषय CTX (thread->ctx)

/* exported interface documented in js.h */
nserror js_closethread(jsthread *thread)
{
	/* We can always close down a thread, it might just confuse
	 * the code running, though we don't mind since we're in the
	 * process of destruction at this point
	 */
	duk_int_t शिखरम् = duk_get_शिखरम्(CTX);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, thread->heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Restore whatever stack we had */
	duk_समुच्चयः_शिखरम्(CTX, शिखरम्);

	प्रत्यागम NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
स्थिर शून्य dukky_destroythread(jsthread *thread)
{
	jsheap *heap = thread->heap;

	प्रतिज्ञा(thread->अन्तर्_use == 0);
	प्रतिज्ञा(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_सूचकांक(heap->ctx, -1, thread->thread_idx);
	duk_निष्कासय(heap->ctx); /* ... */

	/* We can now free the thread object */
	मुक्त(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(heap->ctx, 0);
	duk_gc(heap->ctx, DUK_GC_COMPACT);
	heap->जीवितम्_तन्तवः--;

	/* And if the heap should now go, blow it away */
	यदि (heap->pending_destroy == true && heap->जीवितम्_तन्तवः == 0) {
		dukky_destroyheap(heap);
	}
}

/* exported interface documented in js.h */
शून्य js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	यदि (thread->अन्तर्_use == 0) {
		dukky_destroythread(thread);
	}
}

स्थिर शून्य dukky_enter_thread(jsthread *thread)
{
	प्रतिज्ञा(thread != रिक्त);
	thread->अन्तर्_use++;
}

स्थिर शून्य dukky_leave_thread(jsthread *thread)
{
	प्रतिज्ञा(thread != रिक्त);
	प्रतिज्ञा(thread->अन्तर्_use > 0);

	thread->अन्तर्_use--;
	यदि (thread->अन्तर्_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(शून्य *udata)
{
#परिभाषय JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *heap = (jsheap *) udata;
	uint64_t now;

	(शून्य) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	प्रत्यागम heap->exec_start_time != 0 &&
			now > (heap->exec_start_time + JS_EXEC_TIMEOUT_MS);
}

स्थिर शून्य dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_शिखरम्(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_safe_गन्तव्यम्_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_निष्कासय(ctx);
	/* ..., errobj */
}

स्थिर शून्य dukky_reset_start_time(duk_context *ctx)
{
	duk_स्मृतिः_functions funcs;
	jsheap *heap;
	duk_get_स्मृतिः_functions(ctx, &funcs);
	heap = funcs.udata;
	(शून्य) nsu_getmonotonic_ms(&heap->exec_start_time);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_आकार_t argc, bool reset_timeout)
{
	यदि (reset_timeout) {
		dukky_reset_start_time(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	यदि (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	प्रत्यागम ret;
}


शून्य dukky_प्रक्षिप_generics(duk_context *ctx, ध्रुव वर्ण *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

स्थिर duk_int_t dukky_प्रक्षिप_context_dump(duk_context *ctx, शून्य *udata)
{
	duk_प्रक्षिप_context_dump(ctx);
	प्रत्यागम 1;
}

शून्य dukky_log_राशिः_frame(duk_context *ctx, ध्रुव वर्ण * reason)
{
	यदि (duk_safe_call(ctx, dukky_प्रक्षिप_context_dump, रिक्त, 0, 1) != 0) {
		duk_निष्कासय(ctx);
		duk_प्रक्षिप_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_safe_गन्तव्यम्_string(ctx, -1));
	duk_निष्कासय(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, ध्रुव uint8_t *txt, size_t txtlen, ध्रुव वर्ण *name)
{
	bool ret = false;
	प्रतिज्ञा(thread);

	यदि (txt == रिक्त || txtlen == 0) {
		प्रत्यागम false;
	}

	यदि (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		प्रत्यागम false;
	}

	dukky_enter_thread(thread);

	duk_समुच्चयः_शिखरम्(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_start_time(CTX);
	यदि (name != रिक्त) {
		duk_प्रक्षिप_string(CTX, name);
	} अन्यथा {
		duk_प्रक्षिप_string(CTX, "?unknown source?");
	}
	यदि (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (ध्रुव वर्ण *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		गच्छ handle_error;
	}

	यदि (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		गच्छ handle_error;
	}

	यदि (duk_get_शिखरम्(CTX) == 0) duk_प्रक्षिप_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	गच्छ बहिर्गमनम्;

handle_error:
	dukky_dump_error(CTX);
बहिर्गमनम्:
	dukky_leave_thread(thread);
	प्रत्यागम ret;
}

स्थिर ध्रुव वर्ण* dukky_event_proto(dom_event *evt)
{
	ध्रुव वर्ण *ret = PROTO_NAME(EVENT);
	dom_string *type = रिक्त;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	यदि (err != DOM_NO_ERR) {
		गच्छ बहिर्गमनम्;
	}

	यदि (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		गच्छ बहिर्गमनम्;
	} अन्यथा यदि (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		गच्छ बहिर्गमनम्;
	} अन्यथा यदि (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		गच्छ बहिर्गमनम्;
	}

बहिर्गमनम्:
	यदि (type != रिक्त) {
		dom_string_unref(type);
	}

	प्रत्यागम ret;
}

/*** New style event handling ***/

शून्य dukky_प्रक्षिप_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_प्रक्षिप_pointer(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	यदि (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_निष्कासय(ctx);
		/* ... events */
		duk_प्रक्षिप_pointer(ctx, evt);
		यदि (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_निष्कासय(ctx);
			/* ... events */
			duk_प्रक्षिप_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_प्रक्षिप_pointer(ctx, evt);
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

स्थिर शून्य dukky_प्रक्षिप_handler_संहिता(duk_context *ctx, dom_string *name,
				     dom_event_target *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_ग्रन्थिः_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	यदि (et == रिक्त) {
		duk_प्रक्षिप_lstring(ctx, "", 0);
		प्रत्यागम;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_ग्रन्थिः_get_ग्रन्थिः_type(et, &ntype);
	यदि (exc != DOM_NO_ERR) {
		duk_प्रक्षिप_lstring(ctx, "", 0);
		प्रत्यागम;
	}

	यदि (ntype != DOM_ELEMENT_ग्रन्थिः) {
		duk_प्रक्षिप_lstring(ctx, "", 0);
		प्रत्यागम;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	यदि (exc != DOM_NO_ERR) {
		duk_प्रक्षिप_lstring(ctx, "", 0);
		प्रत्यागम;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	यदि ((exc != DOM_NO_ERR) || (val == रिक्त)) {
		dom_string_unref(onname);
		duk_प्रक्षिप_lstring(ctx, "", 0);
		प्रत्यागम;
	}

	dom_string_unref(onname);
	duk_प्रक्षिप_lstring(ctx, dom_string_दत्त(val), dom_string_दीर्घता(val));
	dom_string_unref(val);
}

bool dukky_get_current_मानम्_सम्बन्धि_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_target *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_प्रक्षिप_lstring(ctx, dom_string_दत्त(name), dom_string_दीर्घता(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	यदि (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_निष्कासय_2(ctx);
		/* ... node */
		dukky_प्रक्षिप_handler_संहिता(ctx, name, et);
		/* ... node handlercode? */
		/* TODO: If this is null, clean up and propagate */
		/* ... node handlercode */
		/** @todo This is entirely wrong, but it's hard to get right */
		duk_प्रक्षिप_string(ctx, "function (event) {");
		/* ... node handlercode prefix */
		duk_निवेशय(ctx, -2);
		/* ... node prefix handlercode */
		duk_प्रक्षिप_string(ctx, "}");
		/* ... node prefix handlercode suffix */
		duk_concat(ctx, 3);
		/* ... node fullhandlersrc */
		duk_प्रक्षिप_string(ctx, "internal raw uncompiled handler");
		/* ... node fullhandlersrc filename */
		यदि (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_निष्कासय_2(ctx);
			प्रत्यागम false;
		}
		/* ... node handler */
		duk_निवेशय(ctx, -2);
		/* ... handler node */
	} अन्यथा {
		/* ... node handlers handler */
		duk_निवेशय(ctx, -3);
		/* ... handler node handlers */
		duk_निष्कासय(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	प्रत्यागम true;
}

स्थिर शून्य dukky_generic_event_handler(dom_event *evt, शून्य *pw)
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
	यदि (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		प्रत्यागम;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (पूर्णाङ्क)dom_string_दीर्घता(name),
	      dom_string_दत्त(name));
	exc = dom_event_get_event_phase(evt, &phase);
	यदि (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		प्रत्यागम;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_TARGET ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (पूर्णाङ्क)phase);

	exc = dom_event_get_current_target(evt, &targ);
	यदि (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		प्रत्यागम;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	यदि (phase == DOM_CAPTURING_PHASE)
		गच्छ handle_extras;

	/* ... */
	यदि (dukky_प्रक्षिप_ग्रन्थिः(ctx, (dom_ग्रन्थिः *)targ) == false) {
		dom_string_unref(name);
		dom_ग्रन्थिः_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		प्रत्यागम;
	}
	/* ... node */
	यदि (dukky_get_current_मानम्_सम्बन्धि_event_handler(
		    ctx, name, (dom_event_target *)targ) == false) {
		/* ... */
		गच्छ handle_extras;
	}
	/* ... handler node */
	dukky_प्रक्षिप_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_start_time(ctx);
	यदि (duk_pcall_विधानम्(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_तात्कालिकम्_सञ्चरणम्(evt);
		यदि (exc != DOM_NO_ERR)
			NSLOG(dukky, DEBUG,
			      "WORSE! could not stop propagation");
		duk_get_prop_string(ctx, -1, "name");
		duk_get_prop_string(ctx, -2, "message");
		duk_get_prop_string(ctx, -3, "fileName");
		duk_get_prop_string(ctx, -4, "lineNumber");
		duk_get_prop_string(ctx, -5, "stack");
		/* ... err name message fileName lineNumber stack */
		NSLOG(dukky, DEBUG, "Uncaught error in JS: %s: %s",
		      duk_safe_गन्तव्यम्_string(ctx, -5),
		      duk_safe_गन्तव्यम्_string(ctx, -4));
		NSLOG(dukky, INFO, "              was at: %s line %s",
		      duk_safe_गन्तव्यम्_string(ctx, -3),
		      duk_safe_गन्तव्यम्_string(ctx, -2));
		NSLOG(dukky, INFO, "         Stack trace: %s",
		      duk_safe_गन्तव्यम्_string(ctx, -1));

		duk_निष्कासय_n(ctx, 6);
		/* ... */
		गच्छ handle_extras;
	}
	/* ... result */
	यदि (duk_is_boolean(ctx, -1) &&
	    duk_गन्तव्यम्_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_निष्कासय(ctx);
handle_extras:
	/* ... */
	duk_प्रक्षिप_lstring(ctx, dom_string_दत्त(name), dom_string_दीर्घता(name));
	dukky_प्रक्षिप_ग्रन्थिः(ctx, (dom_ग्रन्थिः *)targ);
	/* ... type node */
	यदि (dukky_event_target_प्रक्षिप_listeners(ctx, true)) {
		/* Nothing to do */
		duk_निष्कासय(ctx);
		गच्छ बहिर्गमनम्;
	}
	/* ... sublisteners */
	duk_प्रक्षिप_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	यावत् (duk_get_prop_सूचकांक(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_सूचकांक(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		यदि ((event_listener_flags)duk_गन्तव्यम्_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_निष्कासय(ctx);
			/* ... subl copy handler flags */
		}
		duk_निष्कासय(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_सूचकांक(ctx, -2, idx);
		/* ... sublisteners copy */
		idx++;
	}
	/* ... sublisteners copy undefined */
	duk_निष्कासय(ctx);
	/* ... sublisteners copy */
	duk_निवेशय(ctx, -2);
	/* ... copy sublisteners */
	duk_निष्कासय(ctx);
	/* ... copy */
	idx = 0;
	यावत् (duk_get_prop_सूचकांक(ctx, -1, idx++)) {
		/* ... copy handler */
		यदि (duk_get_prop_सूचकांक(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_निष्कासय_2(ctx);
			अनुवर्त;
		}
		duk_निष्कासय(ctx);
		duk_get_prop_सूचकांक(ctx, -1, 0);
		duk_get_prop_सूचकांक(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_निष्कासय(ctx);
		/* ... copy handler callback */
		यदि (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_निष्कासय_2(ctx);
			/* ... copy */
			अनुवर्त;
		}
		/* ... copy handler callback */
		dukky_प्रक्षिप_ग्रन्थिः(ctx, (dom_ग्रन्थिः *)targ);
		/* ... copy handler callback node */
		dukky_प्रक्षिप_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_start_time(ctx);
		यदि (duk_pcall_विधानम्(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_तात्कालिकम्_सञ्चरणम्(evt);
			यदि (exc != DOM_NO_ERR)
				NSLOG(dukky, DEBUG,
				      "WORSE! could not stop propagation");
			duk_get_prop_string(ctx, -1, "name");
			duk_get_prop_string(ctx, -2, "message");
			duk_get_prop_string(ctx, -3, "fileName");
			duk_get_prop_string(ctx, -4, "lineNumber");
			duk_get_prop_string(ctx, -5, "stack");
			/* ... err name message fileName lineNumber stack */
			NSLOG(dukky, DEBUG, "Uncaught error in JS: %s: %s",
			      duk_safe_गन्तव्यम्_string(ctx, -5),
			      duk_safe_गन्तव्यम्_string(ctx, -4));
			NSLOG(dukky, DEBUG,
			      "              was at: %s line %s",
			      duk_safe_गन्तव्यम्_string(ctx, -3),
			      duk_safe_गन्तव्यम्_string(ctx, -2));
			NSLOG(dukky, DEBUG, "         Stack trace: %s",
			      duk_safe_गन्तव्यम्_string(ctx, -1));

			duk_निष्कासय_n(ctx, 7);
			/* ... copy */
			अनुवर्त;
		}
		/* ... copy handler result */
		यदि (duk_is_boolean(ctx, -1) &&
		    duk_गन्तव्यम्_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_निष्कासय_2(ctx);
		/* ... copy */
	}
	duk_निष्कासय_2(ctx);
बहिर्गमनम्:
	/* ... */
	dom_ग्रन्थिः_unref(targ);
	dom_string_unref(name);
}

शून्य dukky_पञ्जिकाकोशः_event_listener_for(duk_context *ctx,
				       संरचना dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = रिक्त;
	dom_exception exc;

	/* ... */
	यदि (ele == रिक्त) {
		/* A null element is the Window object */
		duk_प्रक्षिप_global_object(ctx);
	} अन्यथा {
		/* Non null elements must be pushed as a node object */
		यदि (dukky_प्रक्षिप_ग्रन्थिः(ctx, (संरचना dom_ग्रन्थिः *)ele) == false)
			प्रत्यागम;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_प्रक्षिप_lstring(ctx, dom_string_दत्त(name), dom_string_दीर्घता(name));
	/* ... node handlers name */
	यदि (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_निष्कासय_2(ctx);
		/* ... */
		प्रत्यागम;
	}
	/* ... node handlers */
	duk_प्रक्षिप_lstring(ctx, dom_string_दत्त(name), dom_string_दीर्घता(name));
	/* ... node handlers name */
	duk_प्रक्षिप_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_निष्कासय_2(ctx);
	/* ... */
	यदि (ele == रिक्त) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		प्रत्यागम;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	यदि (exc != DOM_NO_ERR) प्रत्यागम;
	exc = dom_event_target_सङ्कलय_event_listener(
		ele, name, listen, capture);
	यदि (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (पूर्णाङ्क)dom_string_दीर्घता(name), dom_string_दत्त(name));
	} अन्यथा {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (पूर्णाङ्क)dom_string_दीर्घता(name), dom_string_दत्त(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_target_प्रक्षिप_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	यदि (duk_is_undefined(ctx, -1)) {
		/* ... type this null */
		duk_निष्कासय(ctx);
		duk_प्रक्षिप_object(ctx);
		duk_dup(ctx, -1);
		/* ... type this listeners listeners */
		duk_put_prop_string(ctx, -3, EVENT_LISTENER_JS_MAGIC);
		/* ... type this listeners */
	}
	/* ... type this listeners */
	duk_निवेशय(ctx, -3);
	/* ... listeners type this */
	duk_निष्कासय(ctx);
	/* ... listeners type */
	duk_dup(ctx, -1);
	/* ... listeners type type */
	duk_get_prop(ctx, -3);
	/* ... listeners type ??? */
	यदि (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		यदि (dont_create == true) {
			duk_निष्कासय_3(ctx);
			duk_प्रक्षिप_undefined(ctx);
			प्रत्यागम true;
		}
		duk_निष्कासय(ctx);
		duk_प्रक्षिप_array(ctx);
		duk_dup(ctx, -2);
		duk_dup(ctx, -2);
		/* ... listeners type sublisteners type sublisteners */
		duk_put_prop(ctx, -5);
		/* ... listeners type sublisteners */
		ret = true;
	}
	duk_निवेशय(ctx, -3);
	/* ... sublisteners listeners type */
	duk_निष्कासय_2(ctx);
	/* ... sublisteners */
	प्रत्यागम ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
शून्य dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	यावत् (duk_get_prop_सूचकांक(ctx, -1, idx + 1)) {
		duk_put_prop_सूचकांक(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_सूचकांक(ctx, -2, idx + 1);
	duk_निष्कासय(ctx);
}


शून्य js_handle_new_element(jsthread *thread, संरचना dom_element *ग्रन्थिः)
{
	प्रतिज्ञा(thread);
	प्रतिज्ञा(ग्रन्थिः);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = रिक्त;
	dom_string *कुञ्चिका = रिक्त;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_ग्रन्थिः_get_ग्रन्थिः_name(ग्रन्थिः, &nodename);
	यदि (exc != DOM_NO_ERR) प्रत्यागम;

	यदि (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_ग्रन्थिः_get_गुणाः(ग्रन्थिः, &map);
	यदि (exc != DOM_NO_ERR) प्रत्यागम;
	यदि (map == रिक्त) प्रत्यागम;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_दीर्घता(map, &siz);
	यदि (exc != DOM_NO_ERR) गच्छ बहिर्गमनम्;

	प्रति (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_item(map, idx, &attr);
		यदि (exc != DOM_NO_ERR) गच्छ बहिर्गमनम्;
		exc = dom_attr_get_name(attr, &कुञ्चिका);
		यदि (exc != DOM_NO_ERR) गच्छ बहिर्गमनम्;
		यदि (is_body && (
			    कुञ्चिका == corestring_dom_onblur ||
			    कुञ्चिका == corestring_dom_onerror ||
			    कुञ्चिका == corestring_dom_onfocus ||
			    कुञ्चिका == corestring_dom_onload ||
			    कुञ्चिका == corestring_dom_onresize ||
			    कुञ्चिका == corestring_dom_onscroll)) {
			/* This is a forwarded event, it doesn't matter,
			 * we should skip registering for it and later
			 * we will register it for Window itself
			 */
			गच्छ skip_पञ्जिकाकोशः;
		}
		यदि (dom_string_दीर्घता(कुञ्चिका) > 2) {
			/* Can be on* */
			ध्रुव uint8_t *दत्त = (ध्रुव uint8_t *)dom_string_दत्त(कुञ्चिका);
			यदि (दत्त[0] == 'o' && दत्त[1] == 'n') {
				dom_string *sub = रिक्त;
				exc = dom_string_substr(
					कुञ्चिका, 2, dom_string_दीर्घता(कुञ्चिका),
					&sub);
				यदि (exc == DOM_NO_ERR) {
					dukky_पञ्जिकाकोशः_event_listener_for(
						CTX, ग्रन्थिः, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_पञ्जिकाकोशः:
		dom_string_unref(कुञ्चिका); कुञ्चिका = रिक्त;
		dom_ग्रन्थिः_unref(attr); attr = रिक्त;
	}

बहिर्गमनम्:
	यदि (कुञ्चिका != रिक्त)
		dom_string_unref(कुञ्चिका);

	यदि (attr != रिक्त)
		dom_ग्रन्थिः_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

शून्य js_event_cleanup(jsthread *thread, संरचना dom_event *evt)
{
	प्रतिज्ञा(thread);
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, EVENT_MAGIC);
	/* ... EVENT_MAP */
	duk_प्रक्षिप_pointer(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_निष्कासय(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, ध्रुव वर्ण *type, संरचना dom_document *doc, संरचना dom_ग्रन्थिः *target)
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

	यदि (target != रिक्त)
		/* Swallow non-Window-targetted events quietly */
		प्रत्यागम true;

	यदि (वाक्यं_तुलय(type, "load") != 0)
		/* Swallow non-load events quietly */
		प्रत्यागम true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	यदि (exc != DOM_NO_ERR) प्रत्यागम true;
	exc = dom_event_init(evt, corestring_dom_आरोपय, false, false);
	यदि (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		प्रत्यागम true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_प्रक्षिप_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	यदि (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_निष्कासय(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		यदि (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			प्रत्यागम true;
		}
		dukky_प्रक्षिप_ग्रन्थिः(CTX, (संरचना dom_ग्रन्थिः *)body);
		/* ... handlers bodynode */
		यदि (dukky_get_current_मानम्_सम्बन्धि_event_handler(
			    CTX, corestring_dom_आरोपय, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_ग्रन्थिः_unref(body);
			/* ... handlers */
			duk_निष्कासय(CTX);
			dukky_leave_thread(thread);
			प्रत्यागम true;
		}
		/* Unref the body, we don't need it any more */
		dom_ग्रन्थिः_unref(body);
		/* ... handlers handler bodynode */
		duk_निष्कासय(CTX);
	}
	/* ... handlers handler */
	duk_निवेशय(CTX, -2);
	/* ... handler handlers */
	duk_निष्कासय(CTX);
	/* ... handler */
	duk_प्रक्षिप_global_object(CTX);
	/* ... handler Window */
	dukky_प्रक्षिप_event(CTX, evt);
	/* ... handler Window event */
	dukky_reset_start_time(CTX);
	यदि (duk_pcall_विधानम्(CTX, 1) != 0) {
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
		      duk_safe_गन्तव्यम्_string(CTX, -5),
		      duk_safe_गन्तव्यम्_string(CTX, -4));
		NSLOG(dukky, DEBUG, "              was at: %s line %s",
		      duk_safe_गन्तव्यम्_string(CTX, -3),
		      duk_safe_गन्तव्यम्_string(CTX, -2));
		NSLOG(dukky, DEBUG, "         Stack trace: %s",
		      duk_safe_गन्तव्यम्_string(CTX, -1));

		duk_निष्कासय_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		प्रत्यागम true;
	}
	/* ... result */
	duk_निष्कासय(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	प्रत्यागम true;
}
