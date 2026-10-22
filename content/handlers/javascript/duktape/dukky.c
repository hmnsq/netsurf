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

#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <nsutils/time.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "content/content.h"

#ଅନ୍ତର୍ଭାଵ "javascript/js.h"
#ଅନ୍ତର୍ଭାଵ "javascript/content.h"

#ଅନ୍ତର୍ଭାଵ "duktape/binding.h"
#ଅନ୍ତର୍ଭାଵ "duktape/generics.js.inc"
#ଅନ୍ତର୍ଭାଵ "duktape/polyfill.js.inc"

#ଅନ୍ତର୍ଭାଵ "duktape.h"
#ଅନ୍ତର୍ଭାଵ "dukky.h"

#ଅନ୍ତର୍ଭାଵ <dom/dom.h>

#define EVENT_MAGIC MAGIC(EVENT_MAP)
#define HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#define HANDLER_MAGIC MAGIC(HANDLER_MAP)
#define EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#define GENERICS_MAGIC MAGIC(GENERICS_TABLE)
#define THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
ସଂରଚନା jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t next_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା live_threads; /**< number of live threads */
	uint64_t exec_start_time;
};

/**
 * dukky javascript thread
 */
ସଂରଚନା jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା in_use; /**< The number of times this thread is in use */
	jsheap *heap; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

ସ୍ଥିର duk_ret_t dukky_populate_object(duk_context *ctx, ଶୂନ୍ୟ *udata)
{
	/* ... obj args protoname nargs */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nargs = duk_get_int(ctx, -1);
	duk_କାଢ଼(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	ଯଦି (duk_is_undefined(ctx, -1)) {
		NSLOG(dukky, WARNING,
		      "Unable to find dukky prototype for `%s` - falling back to HTMLUnknownElement",
		      duk_get_string(ctx, -3) + 2 /* Skip the two unprintables */);
		duk_କାଢ଼(ctx);
		duk_ଠେଲ_string(ctx, PROTO_NAME(HTMLUNKNOWNELEMENT));
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
	duk_ଭର୍ତ୍ତି(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_କାଢ଼_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	ଫେରନ୍ତୁ 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା args)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, args);
	/* ... args */
	duk_ଠେଲ_object(ctx);
	/* ... args obj */
	duk_ଠେଲ_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
	/* ... args obj */
	duk_ଠେଲ_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
	/* ... args obj */
	duk_ଭର୍ତ୍ତି(ctx, -(args+1));
	/* ... obj args */
	duk_ଠେଲ_string(ctx, name);
	/* ... obj args name */
	duk_ଠେଲ_int(ctx, args);
	/* ... obj args name nargs */
	ଯଦି ((ret = duk_safe_call(ctx, dukky_populate_object, ଖାଲି, args + 3, 1))
	    != DUK_EXEC_SUCCESS)
		ଫେରନ୍ତୁ ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	ଫେରନ୍ତୁ DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_ଠେଲ_ଗଣ୍ଠି_stacked(duk_context *ctx)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଶୀର୍ଷ_at_fail = duk_get_ଶୀର୍ଷ(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, ଗଣ୍ଠି_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	ଯଦି (duk_is_undefined(ctx, -1)) {
		/* ... nodeptr klass nodes undefined */
		duk_କାଢ଼(ctx);
		/* ... nodeptr klass nodes */
		duk_ଠେଲ_object(ctx);
		/* ... nodeptr klass nodes obj */
		duk_ଠେଲ_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_ଠେଲ_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr klass */
		duk_ଠେଲ_int(ctx, 1);
		/* ... nodeptr klass nodes obj nodeptr klass 1 */
		ଯଦି (duk_safe_call(ctx, dukky_populate_object, ଖାଲି, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_set_ଶୀର୍ଷ(ctx, ଶୀର୍ଷ_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			ଫେରନ୍ତୁ false;
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
	duk_ଭର୍ତ୍ତି(ctx, -4);
	/* ... node nodeptr klass nodes */
	duk_କାଢ଼_3(ctx);
	/* ... node */
	ଯଦି (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		ଅଚଳ ଅକ୍ଷର * what = duk_safe_to_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_କାଢ଼(ctx);
	}
	ଫେରନ୍ତୁ true;
}

#define SET_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

ସ୍ଥିର ଶୂନ୍ୟ dukky_html_element_class_from_tag_type(dom_html_element_type type,
		ଅଚଳ ଅକ୍ଷର **html_class, size_t *html_class_len)
{
	ବାଛ(type) {
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_HTML:
		SET_HTML_CLASS(HTML)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_HEAD:
		SET_HTML_CLASS(HEAD)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_META:
		SET_HTML_CLASS(META)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_BASE:
		SET_HTML_CLASS(BASE)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TITLE:
		SET_HTML_CLASS(TITLE)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_BODY:
		SET_HTML_CLASS(BODY)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_DIV:
		SET_HTML_CLASS(DIV)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_FORM:
		SET_HTML_CLASS(FORM)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_LINK:
		SET_HTML_CLASS(LINK)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_BUTTON:
		SET_HTML_CLASS(BUTTON)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_ନିବେଶ:
		SET_HTML_CLASS(ନିବେଶ_2)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		SET_HTML_CLASS(TEXTAREA)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		SET_HTML_CLASS(OPTGROUP)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_OPTION:
		SET_HTML_CLASS(OPTION)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_SELECT:
		SET_HTML_CLASS(SELECT)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_HR:
		SET_HTML_CLASS(HR)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_DL:
		SET_HTML_CLASS(DLIST)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_DIR:
		SET_HTML_CLASS(DIRECTORY)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_MENU:
		SET_HTML_CLASS(MENU)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_FIELDSET:
		SET_HTML_CLASS(FIELDSET)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_LEGEND:
		SET_HTML_CLASS(LEGEND)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_P:
		SET_HTML_CLASS(PARAGRAPH)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_H1:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_H2:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_H3:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_H4:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_H5:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_H6:
		SET_HTML_CLASS(HEADING)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_Q:
		SET_HTML_CLASS(QUOTE)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_PRE:
		SET_HTML_CLASS(PRE)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_BR:
		SET_HTML_CLASS(BR)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_LABEL:
		SET_HTML_CLASS(LABEL)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_UL:
		SET_HTML_CLASS(ULIST)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_OL:
		SET_HTML_CLASS(OLIST)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_LI:
		SET_HTML_CLASS(LI)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_FONT:
		SET_HTML_CLASS(FONT)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_DEL:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_INS:
		SET_HTML_CLASS(MOD)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_A:
		SET_HTML_CLASS(ANCHOR)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_BASEFONT:
		SET_HTML_CLASS(BASEFONT)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_IMG:
		SET_HTML_CLASS(IMAGE)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_OBJECT:
		SET_HTML_CLASS(OBJECT)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_PARAM:
		SET_HTML_CLASS(PARAM)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_APPLET:
		SET_HTML_CLASS(APPLET)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_MAP:
		SET_HTML_CLASS(MAP)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_AREA:
		SET_HTML_CLASS(AREA)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_SCRIPT:
		SET_HTML_CLASS(SCRIPT)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_CAPTION:
		SET_HTML_CLASS(TABLECAPTION)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TD:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TH:
		SET_HTML_CLASS(TABLECELL)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_COL:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_COLGROUP:
		SET_HTML_CLASS(TABLECOL)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_THEAD:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TBODY:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TFOOT:
		SET_HTML_CLASS(TABLESECTION)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TABLE:
		SET_HTML_CLASS(TABLE)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TR:
		SET_HTML_CLASS(TABLEROW)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_STYLE:
		SET_HTML_CLASS(STYLE)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_FRAMESET:
		SET_HTML_CLASS(FRAMESET)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_FRAME:
		SET_HTML_CLASS(FRAME)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_IFRAME:
		SET_HTML_CLASS(IFRAME)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_ISINDEX:
		SET_HTML_CLASS(ISINDEX)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_CANVAS:
		SET_HTML_CLASS(CANVAS)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_ଗଣନା:
		ପ୍ରତିଜ୍ଞା(type != DOM_HTML_ELEMENT_TYPE_ଗଣନା);
		fallthrough;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		SET_HTML_CLASS(UNKNOWN)
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ;
}

#undef SET_HTML_CLASS

ସ୍ଥିର ଶୂନ୍ୟ
dukky_ଠେଲ_ଗଣ୍ଠି_klass(duk_context *ctx, ସଂରଚନା dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	dom_ଗଣ୍ଠି_type nodetype;
	dom_exception err;

	err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(ଗଣ୍ଠି, &nodetype);
	ଯଦି (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_ଠେଲ_string(ctx, PROTO_NAME(ଗଣ୍ଠି_2));
		ଫେରନ୍ତୁ;
	}

	ବାଛ(nodetype) {
	ସ୍ଥିତି DOM_ELEMENT_ଗଣ୍ଠି: {
		dom_string *namespace;
		dom_html_element_type type;
		ଅଚଳ ଅକ୍ଷର *html_class;
		size_t html_class_len;
		err = dom_ଗଣ୍ଠି_get_namespace(ଗଣ୍ଠି, &namespace);
		ଯଦି (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_ଠେଲ_string(ctx, PROTO_NAME(ELEMENT));
			ଭାଙ୍ଗ;
		}
		ଯଦି (namespace == ଖାଲି) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_ଠେଲ_string(ctx, PROTO_NAME(ELEMENT));
			ଭାଙ୍ଗ;
		}

		ଯଦି (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_ଠେଲ_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			ଭାଙ୍ଗ;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_tag_type(ଗଣ୍ଠି, &type);
		ଯଦି (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_tag_type(type,
				&html_class, &html_class_len);

		duk_ଠେଲ_lstring(ctx, html_class, html_class_len);
		ଭାଙ୍ଗ;
	}
	ସ୍ଥିତି DOM_TEXT_ଗଣ୍ଠି:
		duk_ଠେଲ_string(ctx, PROTO_NAME(TEXT));
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_COMMENT_ଗଣ୍ଠି:
		duk_ଠେଲ_string(ctx, PROTO_NAME(COMMENT));
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_DOCUMENT_ଗଣ୍ଠି:
		duk_ଠେଲ_string(ctx, PROTO_NAME(DOCUMENT));
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_ATTRIBUTE_ଗଣ୍ଠି:
	ସ୍ଥିତି DOM_PROCESSING_INSTRUCTION_ଗଣ୍ଠି:
	ସ୍ଥିତି DOM_DOCUMENT_TYPE_ଗଣ୍ଠି:
	ସ୍ଥିତି DOM_DOCUMENT_FRAGMENT_ଗଣ୍ଠି:
	ସ୍ଥିତି DOM_NOTATION_ଗଣ୍ଠି:
	ସ୍ଥିତି DOM_ENTITY_REFERENCE_ଗଣ୍ଠି:
	ସ୍ଥିତି DOM_ENTITY_ଗଣ୍ଠି:
	ସ୍ଥିତି DOM_CDATA_SECTION_ଗଣ୍ଠି:
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/* Oh bum, just node then */
		duk_ଠେଲ_string(ctx, PROTO_NAME(ଗଣ୍ଠି_2));
	}
}

duk_bool_t
dukky_ଠେଲ_ଗଣ୍ଠି(duk_context *ctx, ସଂରଚନା dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", ଗଣ୍ଠି);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, ଗଣ୍ଠି_MAGIC);
	/* ... nodes */
	duk_ଠେଲ_pointer(ctx, ଗଣ୍ଠି);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	ଯଦି (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_ଭର୍ତ୍ତି(ctx, -2);
		/* ... node nodes */
		duk_କାଢ଼(ctx);
		/* ... node */
		ଯଦି (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			ଅଚଳ ଅକ୍ଷର * what = duk_safe_to_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_କାଢ଼(ctx);
		}
		ଫେରନ୍ତୁ true;
	}
	/* ... nodes undefined */
	duk_କାଢ଼_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_ଠେଲ_pointer(ctx, ଗଣ୍ଠି);
	/* ... nodeptr */
	dukky_ଠେଲ_ଗଣ୍ଠି_klass(ctx, ଗଣ୍ଠି);
	/* ... nodeptr klass */
	ଫେରନ୍ତୁ dukky_ଠେଲ_ଗଣ୍ଠି_stacked(ctx);
}

ସ୍ଥିର duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	ଫେରନ୍ତୁ duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

ଶୂନ୍ୟ
dukky_inject_not_ctr(duk_context *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, ଅଚଳ ଅକ୍ଷର *name)
{
	/* ... p[idx] ... proto */
	duk_ଠେଲ_c_function(ctx, dukky_bad_constructor, 0);
	/* ... p[idx] ... proto cons */
	duk_ଭର୍ତ୍ତି(ctx, -2);
	/* ... p[idx] ... cons proto */
	duk_put_prop_string(ctx, -2, "prototype");
	/* ... p[idx] ... cons[proto] */
	duk_put_prop_string(ctx, idx, name);
	/* ... p ... */
	ଫେରନ୍ତୁ;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

ସ୍ଥିର ଶୂନ୍ୟ *dukky_alloc_function(ଶୂନ୍ୟ *udata, duk_size_t size)
{
	ଯଦି (size == 0)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ malloc(size);
}

ସ୍ଥିର ଶୂନ୍ୟ *dukky_realloc_function(ଶୂନ୍ୟ *udata, ଶୂନ୍ୟ *ptr, duk_size_t size)
{
	ଯଦି (ptr == ଖାଲି && size == 0)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (size == 0) {
		ମୁକ୍ତ(ptr);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ realloc(ptr, size);
}


ସ୍ଥିର ଶୂନ୍ୟ dukky_free_function(ଶୂନ୍ୟ *udata, ଶୂନ୍ୟ *ptr)
{
	ଯଦି (ptr != ଖାଲି)
		ମୁକ୍ତ(ptr);
}

/* exported interface documented in js.h */
ଶୂନ୍ୟ js_initialise(ଶୂନ୍ୟ)
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
ଶୂନ୍ୟ js_finalise(ଶୂନ୍ୟ)
{
	/* NADA for now */
}


/* exported interface documented in js.h */
nserror
js_newheap(ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout, jsheap **heap)
{
	duk_context *ctx;
	jsheap *ret = calloc(1, ଆକାର(*ret));
	*heap = ଖାଲି;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	ଯଦି (ret == ଖାଲି) ଫେରନ୍ତୁ NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_heap(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		ଖାଲି);
	ଯଦି (ret->ctx == ଖାଲି) { ମୁକ୍ତ(ret); ଫେରନ୍ତୁ NSERROR_NOMEM; }
	/* Create the prototype stuffs */
	duk_ଠେଲ_global_object(ctx);
	duk_ଠେଲ_boolean(ctx, true);
	duk_put_prop_string(ctx, -2, "protos");
	duk_put_global_string(ctx, PROTO_MAGIC);
	/* Create prototypes here */
	dukky_create_prototypes(ctx);
	/* Now create the thread map */
	duk_ଠେଲ_object(ctx);
	duk_put_global_string(ctx, THREAD_MAP);

	*heap = ret;
	ଫେରନ୍ତୁ NSERROR_OK;
}


ସ୍ଥିର ଶୂନ୍ୟ dukky_destroyheap(jsheap *heap)
{
	ପ୍ରତିଜ୍ଞା(heap->pending_destroy == true);
	ପ୍ରତିଜ୍ଞା(heap->live_threads == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_heap(heap->ctx);
	ମୁକ୍ତ(heap);
}

/* exported interface documented in js.h */
ଶୂନ୍ୟ js_destroyheap(jsheap *heap)
{
	heap->pending_destroy = true;
	ଯଦି (heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* Just for here, the CTX is in ret, not thread */
#define CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *heap, ଶୂନ୍ୟ *win_priv, ଶୂନ୍ୟ *doc_priv, jsthread **thread)
{
	jsthread *ret;
	ପ୍ରତିଜ୍ଞା(heap != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(heap->pending_destroy == false);

	ret = calloc(1, ଆକାର (*ret));
	ଯଦି (ret == ଖାଲି) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_ଠେଲ_thread(heap->ctx); /* ... threads thread */
	ret->heap = heap;
	ret->ctx = duk_require_context(heap->ctx, -1);
	ret->thread_idx = heap->next_thread++;
	duk_put_prop_index(heap->ctx, -2, ret->thread_idx);
	heap->live_threads++;
	duk_କାଢ଼(heap->ctx); /* ... */
	duk_ଠେଲ_int(CTX, 0);
	duk_ଠେଲ_int(CTX, 1);
	duk_ଠେଲ_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_ଠେଲ_pointer(CTX, win_priv);
	duk_ଠେଲ_pointer(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(WINDOW), 2);
	duk_ଠେଲ_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_set_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_ଠେଲ_object(CTX);
	duk_ଠେଲ_pointer(CTX, ଖାଲି);
	duk_ଠେଲ_null(CTX);
	duk_put_prop(CTX, -3);
	duk_put_global_string(CTX, ଗଣ୍ଠି_MAGIC);

	/* And now the event mapping table */
	duk_ଠେଲ_object(CTX);
	duk_put_global_string(CTX, EVENT_MAGIC);

	/* Now load the polyfills */
	/* ... */
	duk_ଠେଲ_string(CTX, "polyfill.js");
	/* ..., polyfill.js */
	ଯଦି (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ଅଚଳ ଅକ୍ଷର *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		ଫେରନ୍ତୁ NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	ଯଦି (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		ଫେରନ୍ତୁ NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_କାଢ଼(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_ଠେଲ_string(CTX, "generics.js");
	/* ..., generics.js */
	ଯଦି (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ଅଚଳ ଅକ୍ଷର *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		ଫେରନ୍ତୁ NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	ଯଦି (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		ଫେରନ୍ତୁ NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_କାଢ଼(CTX);
	/* ... */
	duk_ଠେଲ_global_object(CTX);
	/* ..., Win */
	duk_get_prop_string(CTX, -1, "NetSurf");
	/* ..., Win, NetSurf */
	duk_put_global_string(CTX, GENERICS_MAGIC);
	/* ..., Win */
	duk_del_prop_string(CTX, -1, "NetSurf");
	duk_କାଢ଼(CTX);
	/* ... */

	dukky_log_ଗଦା_frame(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, heap);
	*thread = ret;

	ଫେରନ୍ତୁ NSERROR_OK;
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
	duk_int_t ଶୀର୍ଷ = duk_get_ଶୀର୍ଷ(CTX);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, thread->heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Restore whatever stack we had */
	duk_set_ଶୀର୍ଷ(CTX, ଶୀର୍ଷ);

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
ସ୍ଥିର ଶୂନ୍ୟ dukky_destroythread(jsthread *thread)
{
	jsheap *heap = thread->heap;

	ପ୍ରତିଜ୍ଞା(thread->in_use == 0);
	ପ୍ରତିଜ୍ଞା(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_index(heap->ctx, -1, thread->thread_idx);
	duk_କାଢ଼(heap->ctx); /* ... */

	/* We can now free the thread object */
	ମୁକ୍ତ(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(heap->ctx, 0);
	duk_gc(heap->ctx, DUK_GC_COMPACT);
	heap->live_threads--;

	/* And if the heap should now go, blow it away */
	ଯଦି (heap->pending_destroy == true && heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* exported interface documented in js.h */
ଶୂନ୍ୟ js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	ଯଦି (thread->in_use == 0) {
		dukky_destroythread(thread);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ dukky_enter_thread(jsthread *thread)
{
	ପ୍ରତିଜ୍ଞା(thread != ଖାଲି);
	thread->in_use++;
}

ସ୍ଥିର ଶୂନ୍ୟ dukky_leave_thread(jsthread *thread)
{
	ପ୍ରତିଜ୍ଞା(thread != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(thread->in_use > 0);

	thread->in_use--;
	ଯଦି (thread->in_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(ଶୂନ୍ୟ *udata)
{
#define JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *heap = (jsheap *) udata;
	uint64_t now;

	(ଶୂନ୍ୟ) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	ଫେରନ୍ତୁ heap->exec_start_time != 0 &&
			now > (heap->exec_start_time + JS_EXEC_TIMEOUT_MS);
}

ସ୍ଥିର ଶୂନ୍ୟ dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_ଶୀର୍ଷ(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_safe_to_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_କାଢ଼(ctx);
	/* ..., errobj */
}

ସ୍ଥିର ଶୂନ୍ୟ dukky_reset_start_time(duk_context *ctx)
{
	duk_memory_functions funcs;
	jsheap *heap;
	duk_get_memory_functions(ctx, &funcs);
	heap = funcs.udata;
	(ଶୂନ୍ୟ) nsu_getmonotonic_ms(&heap->exec_start_time);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_size_t argc, bool reset_timeout)
{
	ଯଦି (reset_timeout) {
		dukky_reset_start_time(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	ଯଦି (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	ଫେରନ୍ତୁ ret;
}


ଶୂନ୍ୟ dukky_ଠେଲ_generics(duk_context *ctx, ଅଚଳ ଅକ୍ଷର *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

ସ୍ଥିର duk_int_t dukky_ଠେଲ_context_dump(duk_context *ctx, ଶୂନ୍ୟ *udata)
{
	duk_ଠେଲ_context_dump(ctx);
	ଫେରନ୍ତୁ 1;
}

ଶୂନ୍ୟ dukky_log_ଗଦା_frame(duk_context *ctx, ଅଚଳ ଅକ୍ଷର * reason)
{
	ଯଦି (duk_safe_call(ctx, dukky_ଠେଲ_context_dump, ଖାଲି, 0, 1) != 0) {
		duk_କାଢ଼(ctx);
		duk_ଠେଲ_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_safe_to_string(ctx, -1));
	duk_କାଢ଼(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, ଅଚଳ uint8_t *txt, size_t txtlen, ଅଚଳ ଅକ୍ଷର *name)
{
	bool ret = false;
	ପ୍ରତିଜ୍ଞା(thread);

	ଯଦି (txt == ଖାଲି || txtlen == 0) {
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		ଫେରନ୍ତୁ false;
	}

	dukky_enter_thread(thread);

	duk_set_ଶୀର୍ଷ(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_start_time(CTX);
	ଯଦି (name != ଖାଲି) {
		duk_ଠେଲ_string(CTX, name);
	} ଅନ୍ୟଥା {
		duk_ଠେଲ_string(CTX, "?unknown source?");
	}
	ଯଦି (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (ଅଚଳ ଅକ୍ଷର *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		ଯାଅ handle_error;
	}

	ଯଦି (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		ଯାଅ handle_error;
	}

	ଯଦି (duk_get_ଶୀର୍ଷ(CTX) == 0) duk_ଠେଲ_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	ଯାଅ out;

handle_error:
	dukky_dump_error(CTX);
out:
	dukky_leave_thread(thread);
	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର* dukky_event_proto(dom_event *evt)
{
	ଅଚଳ ଅକ୍ଷର *ret = PROTO_NAME(EVENT);
	dom_string *type = ଖାଲି;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	ଯଦି (err != DOM_NO_ERR) {
		ଯାଅ out;
	}

	ଯଦି (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		ଯାଅ out;
	} ଅନ୍ୟଥା ଯଦି (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		ଯାଅ out;
	} ଅନ୍ୟଥା ଯଦି (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		ଯାଅ out;
	}

out:
	ଯଦି (type != ଖାଲି) {
		dom_string_unref(type);
	}

	ଫେରନ୍ତୁ ret;
}

/*** New style event handling ***/

ଶୂନ୍ୟ dukky_ଠେଲ_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_ଠେଲ_pointer(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	ଯଦି (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_କାଢ଼(ctx);
		/* ... events */
		duk_ଠେଲ_pointer(ctx, evt);
		ଯଦି (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_କାଢ଼(ctx);
			/* ... events */
			duk_ଠେଲ_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_ଠେଲ_pointer(ctx, evt);
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

ସ୍ଥିର ଶୂନ୍ୟ dukky_ଠେଲ_handler_code(duk_context *ctx, dom_string *name,
				     dom_event_target *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_ଗଣ୍ଠି_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	ଯଦି (et == ଖାଲି) {
		duk_ଠେଲ_lstring(ctx, "", 0);
		ଫେରନ୍ତୁ;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(et, &ntype);
	ଯଦି (exc != DOM_NO_ERR) {
		duk_ଠେଲ_lstring(ctx, "", 0);
		ଫେରନ୍ତୁ;
	}

	ଯଦି (ntype != DOM_ELEMENT_ଗଣ୍ଠି) {
		duk_ଠେଲ_lstring(ctx, "", 0);
		ଫେରନ୍ତୁ;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	ଯଦି (exc != DOM_NO_ERR) {
		duk_ଠେଲ_lstring(ctx, "", 0);
		ଫେରନ୍ତୁ;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	ଯଦି ((exc != DOM_NO_ERR) || (val == ଖାଲି)) {
		dom_string_unref(onname);
		duk_ଠେଲ_lstring(ctx, "", 0);
		ଫେରନ୍ତୁ;
	}

	dom_string_unref(onname);
	duk_ଠେଲ_lstring(ctx, dom_string_data(val), dom_string_length(val));
	dom_string_unref(val);
}

bool dukky_get_current_ମୂଲ୍ୟ_of_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_target *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_ଠେଲ_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	ଯଦି (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_କାଢ଼_2(ctx);
		/* ... node */
		dukky_ଠେଲ_handler_code(ctx, name, et);
		/* ... node handlercode? */
		/* TODO: If this is null, clean up and propagate */
		/* ... node handlercode */
		/** @todo This is entirely wrong, but it's hard to get right */
		duk_ଠେଲ_string(ctx, "function (event) {");
		/* ... node handlercode prefix */
		duk_ଭର୍ତ୍ତି(ctx, -2);
		/* ... node prefix handlercode */
		duk_ଠେଲ_string(ctx, "}");
		/* ... node prefix handlercode suffix */
		duk_concat(ctx, 3);
		/* ... node fullhandlersrc */
		duk_ଠେଲ_string(ctx, "internal raw uncompiled handler");
		/* ... node fullhandlersrc filename */
		ଯଦି (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_କାଢ଼_2(ctx);
			ଫେରନ୍ତୁ false;
		}
		/* ... node handler */
		duk_ଭର୍ତ୍ତି(ctx, -2);
		/* ... handler node */
	} ଅନ୍ୟଥା {
		/* ... node handlers handler */
		duk_ଭର୍ତ୍ତି(ctx, -3);
		/* ... handler node handlers */
		duk_କାଢ଼(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର ଶୂନ୍ୟ dukky_generic_event_handler(dom_event *evt, ଶୂନ୍ୟ *pw)
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
	ଯଦି (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		ଫେରନ୍ତୁ;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dom_string_length(name),
	      dom_string_data(name));
	exc = dom_event_get_event_phase(evt, &phase);
	ଯଦି (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		ଫେରନ୍ତୁ;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_TARGET ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)phase);

	exc = dom_event_get_current_target(evt, &targ);
	ଯଦି (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		ଫେରନ୍ତୁ;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	ଯଦି (phase == DOM_CAPTURING_PHASE)
		ଯାଅ handle_extras;

	/* ... */
	ଯଦି (dukky_ଠେଲ_ଗଣ୍ଠି(ctx, (dom_ଗଣ୍ଠି *)targ) == false) {
		dom_string_unref(name);
		dom_ଗଣ୍ଠି_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		ଫେରନ୍ତୁ;
	}
	/* ... node */
	ଯଦି (dukky_get_current_ମୂଲ୍ୟ_of_event_handler(
		    ctx, name, (dom_event_target *)targ) == false) {
		/* ... */
		ଯାଅ handle_extras;
	}
	/* ... handler node */
	dukky_ଠେଲ_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_start_time(ctx);
	ଯଦି (duk_pcall_method(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_immediate_propagation(evt);
		ଯଦି (exc != DOM_NO_ERR)
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

		duk_କାଢ଼_n(ctx, 6);
		/* ... */
		ଯାଅ handle_extras;
	}
	/* ... result */
	ଯଦି (duk_is_boolean(ctx, -1) &&
	    duk_to_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_କାଢ଼(ctx);
handle_extras:
	/* ... */
	duk_ଠେଲ_lstring(ctx, dom_string_data(name), dom_string_length(name));
	dukky_ଠେଲ_ଗଣ୍ଠି(ctx, (dom_ଗଣ୍ଠି *)targ);
	/* ... type node */
	ଯଦି (dukky_event_target_ଠେଲ_listeners(ctx, true)) {
		/* Nothing to do */
		duk_କାଢ଼(ctx);
		ଯାଅ out;
	}
	/* ... sublisteners */
	duk_ଠେଲ_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	ଯେତେବେଳେ (duk_get_prop_index(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_index(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		ଯଦି ((event_listener_flags)duk_to_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_କାଢ଼(ctx);
			/* ... subl copy handler flags */
		}
		duk_କାଢ଼(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_index(ctx, -2, idx);
		/* ... sublisteners copy */
		idx++;
	}
	/* ... sublisteners copy undefined */
	duk_କାଢ଼(ctx);
	/* ... sublisteners copy */
	duk_ଭର୍ତ୍ତି(ctx, -2);
	/* ... copy sublisteners */
	duk_କାଢ଼(ctx);
	/* ... copy */
	idx = 0;
	ଯେତେବେଳେ (duk_get_prop_index(ctx, -1, idx++)) {
		/* ... copy handler */
		ଯଦି (duk_get_prop_index(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_କାଢ଼_2(ctx);
			ଜାରି;
		}
		duk_କାଢ଼(ctx);
		duk_get_prop_index(ctx, -1, 0);
		duk_get_prop_index(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_କାଢ଼(ctx);
		/* ... copy handler callback */
		ଯଦି (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_କାଢ଼_2(ctx);
			/* ... copy */
			ଜାରି;
		}
		/* ... copy handler callback */
		dukky_ଠେଲ_ଗଣ୍ଠି(ctx, (dom_ଗଣ୍ଠି *)targ);
		/* ... copy handler callback node */
		dukky_ଠେଲ_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_start_time(ctx);
		ଯଦି (duk_pcall_method(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_immediate_propagation(evt);
			ଯଦି (exc != DOM_NO_ERR)
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

			duk_କାଢ଼_n(ctx, 7);
			/* ... copy */
			ଜାରି;
		}
		/* ... copy handler result */
		ଯଦି (duk_is_boolean(ctx, -1) &&
		    duk_to_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_କାଢ଼_2(ctx);
		/* ... copy */
	}
	duk_କାଢ଼_2(ctx);
out:
	/* ... */
	dom_ଗଣ୍ଠି_unref(targ);
	dom_string_unref(name);
}

ଶୂନ୍ୟ dukky_register_event_listener_for(duk_context *ctx,
				       ସଂରଚନା dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = ଖାଲି;
	dom_exception exc;

	/* ... */
	ଯଦି (ele == ଖାଲି) {
		/* A null element is the Window object */
		duk_ଠେଲ_global_object(ctx);
	} ଅନ୍ୟଥା {
		/* Non null elements must be pushed as a node object */
		ଯଦି (dukky_ଠେଲ_ଗଣ୍ଠି(ctx, (ସଂରଚନା dom_ଗଣ୍ଠି *)ele) == false)
			ଫେରନ୍ତୁ;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_ଠେଲ_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	ଯଦି (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_କାଢ଼_2(ctx);
		/* ... */
		ଫେରନ୍ତୁ;
	}
	/* ... node handlers */
	duk_ଠେଲ_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_ଠେଲ_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_କାଢ଼_2(ctx);
	/* ... */
	ଯଦି (ele == ଖାଲି) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		ଫେରନ୍ତୁ;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	ଯଦି (exc != DOM_NO_ERR) ଫେରନ୍ତୁ;
	exc = dom_event_target_add_event_listener(
		ele, name, listen, capture);
	ଯଦି (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dom_string_length(name), dom_string_data(name));
	} ଅନ୍ୟଥା {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dom_string_length(name), dom_string_data(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_target_ଠେଲ_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	ଯଦି (duk_is_undefined(ctx, -1)) {
		/* ... type this null */
		duk_କାଢ଼(ctx);
		duk_ଠେଲ_object(ctx);
		duk_dup(ctx, -1);
		/* ... type this listeners listeners */
		duk_put_prop_string(ctx, -3, EVENT_LISTENER_JS_MAGIC);
		/* ... type this listeners */
	}
	/* ... type this listeners */
	duk_ଭର୍ତ୍ତି(ctx, -3);
	/* ... listeners type this */
	duk_କାଢ଼(ctx);
	/* ... listeners type */
	duk_dup(ctx, -1);
	/* ... listeners type type */
	duk_get_prop(ctx, -3);
	/* ... listeners type ??? */
	ଯଦି (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		ଯଦି (dont_create == true) {
			duk_କାଢ଼_3(ctx);
			duk_ଠେଲ_undefined(ctx);
			ଫେରନ୍ତୁ true;
		}
		duk_କାଢ଼(ctx);
		duk_ଠେଲ_array(ctx);
		duk_dup(ctx, -2);
		duk_dup(ctx, -2);
		/* ... listeners type sublisteners type sublisteners */
		duk_put_prop(ctx, -5);
		/* ... listeners type sublisteners */
		ret = true;
	}
	duk_ଭର୍ତ୍ତି(ctx, -3);
	/* ... sublisteners listeners type */
	duk_କାଢ଼_2(ctx);
	/* ... sublisteners */
	ଫେରନ୍ତୁ ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
ଶୂନ୍ୟ dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	ଯେତେବେଳେ (duk_get_prop_index(ctx, -1, idx + 1)) {
		duk_put_prop_index(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_index(ctx, -2, idx + 1);
	duk_କାଢ଼(ctx);
}


ଶୂନ୍ୟ js_handle_new_element(jsthread *thread, ସଂରଚନା dom_element *ଗଣ୍ଠି)
{
	ପ୍ରତିଜ୍ଞା(thread);
	ପ୍ରତିଜ୍ଞା(ଗଣ୍ଠି);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = ଖାଲି;
	dom_string *ଚାବି = ଖାଲି;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(ଗଣ୍ଠି, &nodename);
	ଯଦି (exc != DOM_NO_ERR) ଫେରନ୍ତୁ;

	ଯଦି (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_ଗଣ୍ଠି_get_attributes(ଗଣ୍ଠି, &map);
	ଯଦି (exc != DOM_NO_ERR) ଫେରନ୍ତୁ;
	ଯଦି (map == ଖାଲି) ଫେରନ୍ତୁ;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_length(map, &siz);
	ଯଦି (exc != DOM_NO_ERR) ଯାଅ out;

	ପାଇଁ (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_item(map, idx, &attr);
		ଯଦି (exc != DOM_NO_ERR) ଯାଅ out;
		exc = dom_attr_get_name(attr, &ଚାବି);
		ଯଦି (exc != DOM_NO_ERR) ଯାଅ out;
		ଯଦି (is_body && (
			    ଚାବି == corestring_dom_onblur ||
			    ଚାବି == corestring_dom_onerror ||
			    ଚାବି == corestring_dom_onfocus ||
			    ଚାବି == corestring_dom_onload ||
			    ଚାବି == corestring_dom_onresize ||
			    ଚାବି == corestring_dom_onscroll)) {
			/* This is a forwarded event, it doesn't matter,
			 * we should skip registering for it and later
			 * we will register it for Window itself
			 */
			ଯାଅ skip_register;
		}
		ଯଦି (dom_string_length(ଚାବି) > 2) {
			/* Can be on* */
			ଅଚଳ uint8_t *data = (ଅଚଳ uint8_t *)dom_string_data(ଚାବି);
			ଯଦି (data[0] == 'o' && data[1] == 'n') {
				dom_string *sub = ଖାଲି;
				exc = dom_string_substr(
					ଚାବି, 2, dom_string_length(ଚାବି),
					&sub);
				ଯଦି (exc == DOM_NO_ERR) {
					dukky_register_event_listener_for(
						CTX, ଗଣ୍ଠି, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_register:
		dom_string_unref(ଚାବି); ଚାବି = ଖାଲି;
		dom_ଗଣ୍ଠି_unref(attr); attr = ଖାଲି;
	}

out:
	ଯଦି (ଚାବି != ଖାଲି)
		dom_string_unref(ଚାବି);

	ଯଦି (attr != ଖାଲି)
		dom_ଗଣ୍ଠି_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

ଶୂନ୍ୟ js_event_cleanup(jsthread *thread, ସଂରଚନା dom_event *evt)
{
	ପ୍ରତିଜ୍ଞା(thread);
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, EVENT_MAGIC);
	/* ... EVENT_MAP */
	duk_ଠେଲ_pointer(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_କାଢ଼(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, ଅଚଳ ଅକ୍ଷର *type, ସଂରଚନା dom_document *doc, ସଂରଚନା dom_ଗଣ୍ଠି *target)
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

	ଯଦି (target != ଖାଲି)
		/* Swallow non-Window-targetted events quietly */
		ଫେରନ୍ତୁ true;

	ଯଦି (strcmp(type, "load") != 0)
		/* Swallow non-load events quietly */
		ଫେରନ୍ତୁ true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	ଯଦି (exc != DOM_NO_ERR) ଫେରନ୍ତୁ true;
	exc = dom_event_init(evt, corestring_dom_load, false, false);
	ଯଦି (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ଫେରନ୍ତୁ true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_ଠେଲ_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	ଯଦି (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_କାଢ଼(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		ଯଦି (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			ଫେରନ୍ତୁ true;
		}
		dukky_ଠେଲ_ଗଣ୍ଠି(CTX, (ସଂରଚନା dom_ଗଣ୍ଠି *)body);
		/* ... handlers bodynode */
		ଯଦି (dukky_get_current_ମୂଲ୍ୟ_of_event_handler(
			    CTX, corestring_dom_load, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_ଗଣ୍ଠି_unref(body);
			/* ... handlers */
			duk_କାଢ଼(CTX);
			dukky_leave_thread(thread);
			ଫେରନ୍ତୁ true;
		}
		/* Unref the body, we don't need it any more */
		dom_ଗଣ୍ଠି_unref(body);
		/* ... handlers handler bodynode */
		duk_କାଢ଼(CTX);
	}
	/* ... handlers handler */
	duk_ଭର୍ତ୍ତି(CTX, -2);
	/* ... handler handlers */
	duk_କାଢ଼(CTX);
	/* ... handler */
	duk_ଠେଲ_global_object(CTX);
	/* ... handler Window */
	dukky_ଠେଲ_event(CTX, evt);
	/* ... handler Window event */
	dukky_reset_start_time(CTX);
	ଯଦି (duk_pcall_method(CTX, 1) != 0) {
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

		duk_କାଢ଼_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		ଫେରନ୍ତୁ true;
	}
	/* ... result */
	duk_କାଢ଼(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	ଫେରନ୍ତୁ true;
}
