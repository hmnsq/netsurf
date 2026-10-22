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

#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <nsutils/time.h>

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "content/content.h"

#ಅನ್ತರ್ಭಾವ "javascript/js.h"
#ಅನ್ತರ್ಭಾವ "javascript/content.h"

#ಅನ್ತರ್ಭಾವ "duktape/binding.h"
#ಅನ್ತರ್ಭಾವ "duktape/generics.js.inc"
#ಅನ್ತರ್ಭಾವ "duktape/polyfill.js.inc"

#ಅನ್ತರ್ಭಾವ "duktape.h"
#ಅನ್ತರ್ಭಾವ "dukky.h"

#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#define EVENT_MAGIC MAGIC(EVENT_MAP)
#define HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#define HANDLER_MAGIC MAGIC(HANDLER_MAP)
#define EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#define GENERICS_MAGIC MAGIC(GENERICS_TABLE)
#define THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
ಸಂರಚನಾ jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t next_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	unsigned ಪೂರ್ಣಾಙ್ಕ live_threads; /**< number of live threads */
	uint64_t exec_start_time;
};

/**
 * dukky javascript thread
 */
ಸಂರಚನಾ jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	unsigned ಪೂರ್ಣಾಙ್ಕ in_use; /**< The number of times this thread is in use */
	jsheap *heap; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

ಸ್ಥಿರ duk_ret_t dukky_populate_object(duk_context *ctx, ಶೂನ್ಯ *udata)
{
	/* ... obj args protoname nargs */
	ಪೂರ್ಣಾಙ್ಕ nargs = duk_get_int(ctx, -1);
	duk_ತೆಗೆ(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	ಯದಿ (duk_is_undefined(ctx, -1)) {
		NSLOG(dukky, WARNING,
		      "Unable to find dukky prototype for `%s` - falling back to HTMLUnknownElement",
		      duk_get_string(ctx, -3) + 2 /* Skip the two unprintables */);
		duk_ತೆಗೆ(ctx);
		duk_ತಳ್ಳು_string(ctx, PROTO_NAME(HTMLUNKNOWNELEMENT));
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
	duk_ಸೇರಿಸು(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_ತೆಗೆ_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	ಪ್ರತ್ಯಾಗಮ 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ args)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, args);
	/* ... args */
	duk_ತಳ್ಳು_object(ctx);
	/* ... args obj */
	duk_ತಳ್ಳು_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
	/* ... args obj */
	duk_ತಳ್ಳು_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
	/* ... args obj */
	duk_ಸೇರಿಸು(ctx, -(args+1));
	/* ... obj args */
	duk_ತಳ್ಳು_string(ctx, name);
	/* ... obj args name */
	duk_ತಳ್ಳು_int(ctx, args);
	/* ... obj args name nargs */
	ಯದಿ ((ret = duk_safe_call(ctx, dukky_populate_object, ರಿಕ್ತ, args + 3, 1))
	    != DUK_EXEC_SUCCESS)
		ಪ್ರತ್ಯಾಗಮ ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	ಪ್ರತ್ಯಾಗಮ DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_ತಳ್ಳು_ಗಂಟು_stacked(duk_context *ctx)
{
	ಪೂರ್ಣಾಙ್ಕ ತುದಿ_at_fail = duk_get_ತುದಿ(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, ಗಂಟು_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	ಯದಿ (duk_is_undefined(ctx, -1)) {
		/* ... nodeptr klass nodes undefined */
		duk_ತೆಗೆ(ctx);
		/* ... nodeptr klass nodes */
		duk_ತಳ್ಳು_object(ctx);
		/* ... nodeptr klass nodes obj */
		duk_ತಳ್ಳು_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_ತಳ್ಳು_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr klass */
		duk_ತಳ್ಳು_int(ctx, 1);
		/* ... nodeptr klass nodes obj nodeptr klass 1 */
		ಯದಿ (duk_safe_call(ctx, dukky_populate_object, ರಿಕ್ತ, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_set_ತುದಿ(ctx, ತುದಿ_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			ಪ್ರತ್ಯಾಗಮ false;
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
	duk_ಸೇರಿಸು(ctx, -4);
	/* ... node nodeptr klass nodes */
	duk_ತೆಗೆ_3(ctx);
	/* ... node */
	ಯದಿ (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		ಧ್ರುವ ವರ್ಣ * what = duk_safe_to_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_ತೆಗೆ(ctx);
	}
	ಪ್ರತ್ಯಾಗಮ true;
}

#define SET_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

ಸ್ಥಿರ ಶೂನ್ಯ dukky_html_element_class_from_tag_type(dom_html_element_type type,
		ಧ್ರುವ ವರ್ಣ **html_class, size_t *html_class_len)
{
	ವಿಕಲ್ಪ(type) {
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_HTML:
		SET_HTML_CLASS(HTML)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_HEAD:
		SET_HTML_CLASS(HEAD)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_META:
		SET_HTML_CLASS(META)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_BASE:
		SET_HTML_CLASS(BASE)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_TITLE:
		SET_HTML_CLASS(TITLE)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_BODY:
		SET_HTML_CLASS(BODY)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_DIV:
		SET_HTML_CLASS(DIV)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_FORM:
		SET_HTML_CLASS(FORM)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_LINK:
		SET_HTML_CLASS(LINK)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_BUTTON:
		SET_HTML_CLASS(BUTTON)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_ಒಳಹರಿವು:
		SET_HTML_CLASS(ಒಳಹರಿವು_2)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		SET_HTML_CLASS(TEXTAREA)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		SET_HTML_CLASS(OPTGROUP)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_OPTION:
		SET_HTML_CLASS(OPTION)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_SELECT:
		SET_HTML_CLASS(SELECT)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_HR:
		SET_HTML_CLASS(HR)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_DL:
		SET_HTML_CLASS(DLIST)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_DIR:
		SET_HTML_CLASS(DIRECTORY)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_MENU:
		SET_HTML_CLASS(MENU)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_FIELDSET:
		SET_HTML_CLASS(FIELDSET)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_LEGEND:
		SET_HTML_CLASS(LEGEND)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_P:
		SET_HTML_CLASS(PARAGRAPH)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_H1:
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_H2:
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_H3:
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_H4:
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_H5:
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_H6:
		SET_HTML_CLASS(HEADING)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_Q:
		SET_HTML_CLASS(QUOTE)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_PRE:
		SET_HTML_CLASS(PRE)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_BR:
		SET_HTML_CLASS(BR)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_LABEL:
		SET_HTML_CLASS(LABEL)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_UL:
		SET_HTML_CLASS(ULIST)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_OL:
		SET_HTML_CLASS(OLIST)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_LI:
		SET_HTML_CLASS(LI)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_FONT:
		SET_HTML_CLASS(FONT)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_DEL:
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_INS:
		SET_HTML_CLASS(MOD)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_A:
		SET_HTML_CLASS(ANCHOR)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_BASEFONT:
		SET_HTML_CLASS(BASEFONT)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_IMG:
		SET_HTML_CLASS(IMAGE)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_OBJECT:
		SET_HTML_CLASS(OBJECT)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_PARAM:
		SET_HTML_CLASS(PARAM)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_APPLET:
		SET_HTML_CLASS(APPLET)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_MAP:
		SET_HTML_CLASS(MAP)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_AREA:
		SET_HTML_CLASS(AREA)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_SCRIPT:
		SET_HTML_CLASS(SCRIPT)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_CAPTION:
		SET_HTML_CLASS(TABLECAPTION)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_TD:
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_TH:
		SET_HTML_CLASS(TABLECELL)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_COL:
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_COLGROUP:
		SET_HTML_CLASS(TABLECOL)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_THEAD:
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_TBODY:
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_TFOOT:
		SET_HTML_CLASS(TABLESECTION)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_TABLE:
		SET_HTML_CLASS(TABLE)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_TR:
		SET_HTML_CLASS(TABLEROW)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_STYLE:
		SET_HTML_CLASS(STYLE)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_FRAMESET:
		SET_HTML_CLASS(FRAMESET)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_FRAME:
		SET_HTML_CLASS(FRAME)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_IFRAME:
		SET_HTML_CLASS(IFRAME)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_ISINDEX:
		SET_HTML_CLASS(ISINDEX)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_CANVAS:
		SET_HTML_CLASS(CANVAS)
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_ಎಣಿಕೆ:
		ಪ್ರತಿಜ್ಞಾ(type != DOM_HTML_ELEMENT_TYPE_ಎಣಿಕೆ);
		fallthrough;
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		SET_HTML_CLASS(UNKNOWN)
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ;
}

#undef SET_HTML_CLASS

ಸ್ಥಿರ ಶೂನ್ಯ
dukky_ತಳ್ಳು_ಗಂಟು_klass(duk_context *ctx, ಸಂರಚನಾ dom_ಗಂಟು *ಗಂಟು)
{
	dom_ಗಂಟು_type nodetype;
	dom_exception err;

	err = dom_ಗಂಟು_get_ಗಂಟು_type(ಗಂಟು, &nodetype);
	ಯದಿ (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_ತಳ್ಳು_string(ctx, PROTO_NAME(ಗಂಟು_2));
		ಪ್ರತ್ಯಾಗಮ;
	}

	ವಿಕಲ್ಪ(nodetype) {
	ಪಕ್ಷ DOM_ELEMENT_ಗಂಟು: {
		dom_string *namespace;
		dom_html_element_type type;
		ಧ್ರುವ ವರ್ಣ *html_class;
		size_t html_class_len;
		err = dom_ಗಂಟು_get_namespace(ಗಂಟು, &namespace);
		ಯದಿ (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_ತಳ್ಳು_string(ctx, PROTO_NAME(ELEMENT));
			ಭಙ್ಗ;
		}
		ಯದಿ (namespace == ರಿಕ್ತ) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_ತಳ್ಳು_string(ctx, PROTO_NAME(ELEMENT));
			ಭಙ್ಗ;
		}

		ಯದಿ (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_ತಳ್ಳು_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			ಭಙ್ಗ;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_tag_type(ಗಂಟು, &type);
		ಯದಿ (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_tag_type(type,
				&html_class, &html_class_len);

		duk_ತಳ್ಳು_lstring(ctx, html_class, html_class_len);
		ಭಙ್ಗ;
	}
	ಪಕ್ಷ DOM_TEXT_ಗಂಟು:
		duk_ತಳ್ಳು_string(ctx, PROTO_NAME(TEXT));
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_COMMENT_ಗಂಟು:
		duk_ತಳ್ಳು_string(ctx, PROTO_NAME(COMMENT));
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_DOCUMENT_ಗಂಟು:
		duk_ತಳ್ಳು_string(ctx, PROTO_NAME(DOCUMENT));
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_ATTRIBUTE_ಗಂಟು:
	ಪಕ್ಷ DOM_PROCESSING_INSTRUCTION_ಗಂಟು:
	ಪಕ್ಷ DOM_DOCUMENT_TYPE_ಗಂಟು:
	ಪಕ್ಷ DOM_DOCUMENT_FRAGMENT_ಗಂಟು:
	ಪಕ್ಷ DOM_NOTATION_ಗಂಟು:
	ಪಕ್ಷ DOM_ENTITY_REFERENCE_ಗಂಟು:
	ಪಕ್ಷ DOM_ENTITY_ಗಂಟು:
	ಪಕ್ಷ DOM_CDATA_SECTION_ಗಂಟು:
	ಸ್ವಭಾವ:
		/* Oh bum, just node then */
		duk_ತಳ್ಳು_string(ctx, PROTO_NAME(ಗಂಟು_2));
	}
}

duk_bool_t
dukky_ತಳ್ಳು_ಗಂಟು(duk_context *ctx, ಸಂರಚನಾ dom_ಗಂಟು *ಗಂಟು)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", ಗಂಟು);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, ಗಂಟು_MAGIC);
	/* ... nodes */
	duk_ತಳ್ಳು_pointer(ctx, ಗಂಟು);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	ಯದಿ (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_ಸೇರಿಸು(ctx, -2);
		/* ... node nodes */
		duk_ತೆಗೆ(ctx);
		/* ... node */
		ಯದಿ (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			ಧ್ರುವ ವರ್ಣ * what = duk_safe_to_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_ತೆಗೆ(ctx);
		}
		ಪ್ರತ್ಯಾಗಮ true;
	}
	/* ... nodes undefined */
	duk_ತೆಗೆ_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_ತಳ್ಳು_pointer(ctx, ಗಂಟು);
	/* ... nodeptr */
	dukky_ತಳ್ಳು_ಗಂಟು_klass(ctx, ಗಂಟು);
	/* ... nodeptr klass */
	ಪ್ರತ್ಯಾಗಮ dukky_ತಳ್ಳು_ಗಂಟು_stacked(ctx);
}

ಸ್ಥಿರ duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	ಪ್ರತ್ಯಾಗಮ duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

ಶೂನ್ಯ
dukky_inject_not_ctr(duk_context *ctx, ಪೂರ್ಣಾಙ್ಕ idx, ಧ್ರುವ ವರ್ಣ *name)
{
	/* ... p[idx] ... proto */
	duk_ತಳ್ಳು_c_function(ctx, dukky_bad_constructor, 0);
	/* ... p[idx] ... proto cons */
	duk_ಸೇರಿಸು(ctx, -2);
	/* ... p[idx] ... cons proto */
	duk_put_prop_string(ctx, -2, "prototype");
	/* ... p[idx] ... cons[proto] */
	duk_put_prop_string(ctx, idx, name);
	/* ... p ... */
	ಪ್ರತ್ಯಾಗಮ;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

ಸ್ಥಿರ ಶೂನ್ಯ *dukky_alloc_function(ಶೂನ್ಯ *udata, duk_size_t size)
{
	ಯದಿ (size == 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ malloc(size);
}

ಸ್ಥಿರ ಶೂನ್ಯ *dukky_realloc_function(ಶೂನ್ಯ *udata, ಶೂನ್ಯ *ptr, duk_size_t size)
{
	ಯದಿ (ptr == ರಿಕ್ತ && size == 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (size == 0) {
		ಮುಕ್ತ(ptr);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ realloc(ptr, size);
}


ಸ್ಥಿರ ಶೂನ್ಯ dukky_free_function(ಶೂನ್ಯ *udata, ಶೂನ್ಯ *ptr)
{
	ಯದಿ (ptr != ರಿಕ್ತ)
		ಮುಕ್ತ(ptr);
}

/* exported interface documented in js.h */
ಶೂನ್ಯ js_initialise(ಶೂನ್ಯ)
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
ಶೂನ್ಯ js_finalise(ಶೂನ್ಯ)
{
	/* NADA for now */
}


/* exported interface documented in js.h */
nserror
js_newheap(ಪೂರ್ಣಾಙ್ಕ timeout, jsheap **heap)
{
	duk_context *ctx;
	jsheap *ret = calloc(1, ಪರಿಮಾಣ(*ret));
	*heap = ರಿಕ್ತ;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	ಯದಿ (ret == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_heap(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		ರಿಕ್ತ);
	ಯದಿ (ret->ctx == ರಿಕ್ತ) { ಮುಕ್ತ(ret); ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM; }
	/* Create the prototype stuffs */
	duk_ತಳ್ಳು_global_object(ctx);
	duk_ತಳ್ಳು_boolean(ctx, true);
	duk_put_prop_string(ctx, -2, "protos");
	duk_put_global_string(ctx, PROTO_MAGIC);
	/* Create prototypes here */
	dukky_create_prototypes(ctx);
	/* Now create the thread map */
	duk_ತಳ್ಳು_object(ctx);
	duk_put_global_string(ctx, THREAD_MAP);

	*heap = ret;
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


ಸ್ಥಿರ ಶೂನ್ಯ dukky_destroyheap(jsheap *heap)
{
	ಪ್ರತಿಜ್ಞಾ(heap->pending_destroy == true);
	ಪ್ರತಿಜ್ಞಾ(heap->live_threads == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_heap(heap->ctx);
	ಮುಕ್ತ(heap);
}

/* exported interface documented in js.h */
ಶೂನ್ಯ js_destroyheap(jsheap *heap)
{
	heap->pending_destroy = true;
	ಯದಿ (heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* Just for here, the CTX is in ret, not thread */
#define CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *heap, ಶೂನ್ಯ *win_priv, ಶೂನ್ಯ *doc_priv, jsthread **thread)
{
	jsthread *ret;
	ಪ್ರತಿಜ್ಞಾ(heap != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(heap->pending_destroy == false);

	ret = calloc(1, ಪರಿಮಾಣ (*ret));
	ಯದಿ (ret == ರಿಕ್ತ) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_ತಳ್ಳು_thread(heap->ctx); /* ... threads thread */
	ret->heap = heap;
	ret->ctx = duk_require_context(heap->ctx, -1);
	ret->thread_idx = heap->next_thread++;
	duk_put_prop_index(heap->ctx, -2, ret->thread_idx);
	heap->live_threads++;
	duk_ತೆಗೆ(heap->ctx); /* ... */
	duk_ತಳ್ಳು_int(CTX, 0);
	duk_ತಳ್ಳು_int(CTX, 1);
	duk_ತಳ್ಳು_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_ತಳ್ಳು_pointer(CTX, win_priv);
	duk_ತಳ್ಳು_pointer(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(WINDOW), 2);
	duk_ತಳ್ಳು_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_set_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_ತಳ್ಳು_object(CTX);
	duk_ತಳ್ಳು_pointer(CTX, ರಿಕ್ತ);
	duk_ತಳ್ಳು_null(CTX);
	duk_put_prop(CTX, -3);
	duk_put_global_string(CTX, ಗಂಟು_MAGIC);

	/* And now the event mapping table */
	duk_ತಳ್ಳು_object(CTX);
	duk_put_global_string(CTX, EVENT_MAGIC);

	/* Now load the polyfills */
	/* ... */
	duk_ತಳ್ಳು_string(CTX, "polyfill.js");
	/* ..., polyfill.js */
	ಯದಿ (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ಧ್ರುವ ವರ್ಣ *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		ಪ್ರತ್ಯಾಗಮ NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	ಯದಿ (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		ಪ್ರತ್ಯಾಗಮ NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_ತೆಗೆ(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_ತಳ್ಳು_string(CTX, "generics.js");
	/* ..., generics.js */
	ಯದಿ (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ಧ್ರುವ ವರ್ಣ *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		ಪ್ರತ್ಯಾಗಮ NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	ಯದಿ (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		ಪ್ರತ್ಯಾಗಮ NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_ತೆಗೆ(CTX);
	/* ... */
	duk_ತಳ್ಳು_global_object(CTX);
	/* ..., Win */
	duk_get_prop_string(CTX, -1, "NetSurf");
	/* ..., Win, NetSurf */
	duk_put_global_string(CTX, GENERICS_MAGIC);
	/* ..., Win */
	duk_del_prop_string(CTX, -1, "NetSurf");
	duk_ತೆಗೆ(CTX);
	/* ... */

	dukky_log_ರಾಶಿ_frame(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, heap);
	*thread = ret;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
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
	duk_int_t ತುದಿ = duk_get_ತುದಿ(CTX);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, thread->heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Restore whatever stack we had */
	duk_set_ತುದಿ(CTX, ತುದಿ);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
ಸ್ಥಿರ ಶೂನ್ಯ dukky_destroythread(jsthread *thread)
{
	jsheap *heap = thread->heap;

	ಪ್ರತಿಜ್ಞಾ(thread->in_use == 0);
	ಪ್ರತಿಜ್ಞಾ(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_index(heap->ctx, -1, thread->thread_idx);
	duk_ತೆಗೆ(heap->ctx); /* ... */

	/* We can now free the thread object */
	ಮುಕ್ತ(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(heap->ctx, 0);
	duk_gc(heap->ctx, DUK_GC_COMPACT);
	heap->live_threads--;

	/* And if the heap should now go, blow it away */
	ಯದಿ (heap->pending_destroy == true && heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* exported interface documented in js.h */
ಶೂನ್ಯ js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	ಯದಿ (thread->in_use == 0) {
		dukky_destroythread(thread);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ dukky_enter_thread(jsthread *thread)
{
	ಪ್ರತಿಜ್ಞಾ(thread != ರಿಕ್ತ);
	thread->in_use++;
}

ಸ್ಥಿರ ಶೂನ್ಯ dukky_leave_thread(jsthread *thread)
{
	ಪ್ರತಿಜ್ಞಾ(thread != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(thread->in_use > 0);

	thread->in_use--;
	ಯದಿ (thread->in_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(ಶೂನ್ಯ *udata)
{
#define JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *heap = (jsheap *) udata;
	uint64_t now;

	(ಶೂನ್ಯ) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	ಪ್ರತ್ಯಾಗಮ heap->exec_start_time != 0 &&
			now > (heap->exec_start_time + JS_EXEC_TIMEOUT_MS);
}

ಸ್ಥಿರ ಶೂನ್ಯ dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_ತುದಿ(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_safe_to_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_ತೆಗೆ(ctx);
	/* ..., errobj */
}

ಸ್ಥಿರ ಶೂನ್ಯ dukky_reset_start_time(duk_context *ctx)
{
	duk_memory_functions funcs;
	jsheap *heap;
	duk_get_memory_functions(ctx, &funcs);
	heap = funcs.udata;
	(ಶೂನ್ಯ) nsu_getmonotonic_ms(&heap->exec_start_time);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_size_t argc, bool reset_timeout)
{
	ಯದಿ (reset_timeout) {
		dukky_reset_start_time(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	ಯದಿ (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}


ಶೂನ್ಯ dukky_ತಳ್ಳು_generics(duk_context *ctx, ಧ್ರುವ ವರ್ಣ *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

ಸ್ಥಿರ duk_int_t dukky_ತಳ್ಳು_context_dump(duk_context *ctx, ಶೂನ್ಯ *udata)
{
	duk_ತಳ್ಳು_context_dump(ctx);
	ಪ್ರತ್ಯಾಗಮ 1;
}

ಶೂನ್ಯ dukky_log_ರಾಶಿ_frame(duk_context *ctx, ಧ್ರುವ ವರ್ಣ * reason)
{
	ಯದಿ (duk_safe_call(ctx, dukky_ತಳ್ಳು_context_dump, ರಿಕ್ತ, 0, 1) != 0) {
		duk_ತೆಗೆ(ctx);
		duk_ತಳ್ಳು_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_safe_to_string(ctx, -1));
	duk_ತೆಗೆ(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, ಧ್ರುವ uint8_t *txt, size_t txtlen, ಧ್ರುವ ವರ್ಣ *name)
{
	bool ret = false;
	ಪ್ರತಿಜ್ಞಾ(thread);

	ಯದಿ (txt == ರಿಕ್ತ || txtlen == 0) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		ಪ್ರತ್ಯಾಗಮ false;
	}

	dukky_enter_thread(thread);

	duk_set_ತುದಿ(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_start_time(CTX);
	ಯದಿ (name != ರಿಕ್ತ) {
		duk_ತಳ್ಳು_string(CTX, name);
	} ಅನ್ಯಥಾ {
		duk_ತಳ್ಳು_string(CTX, "?unknown source?");
	}
	ಯದಿ (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (ಧ್ರುವ ವರ್ಣ *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		ಗಚ್ಛ handle_error;
	}

	ಯದಿ (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		ಗಚ್ಛ handle_error;
	}

	ಯದಿ (duk_get_ತುದಿ(CTX) == 0) duk_ತಳ್ಳು_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	ಗಚ್ಛ out;

handle_error:
	dukky_dump_error(CTX);
out:
	dukky_leave_thread(thread);
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ* dukky_event_proto(dom_event *evt)
{
	ಧ್ರುವ ವರ್ಣ *ret = PROTO_NAME(EVENT);
	dom_string *type = ರಿಕ್ತ;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	ಯದಿ (err != DOM_NO_ERR) {
		ಗಚ್ಛ out;
	}

	ಯದಿ (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		ಗಚ್ಛ out;
	} ಅನ್ಯಥಾ ಯದಿ (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		ಗಚ್ಛ out;
	} ಅನ್ಯಥಾ ಯದಿ (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		ಗಚ್ಛ out;
	}

out:
	ಯದಿ (type != ರಿಕ್ತ) {
		dom_string_unref(type);
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

/*** New style event handling ***/

ಶೂನ್ಯ dukky_ತಳ್ಳು_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_ತಳ್ಳು_pointer(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	ಯದಿ (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_ತೆಗೆ(ctx);
		/* ... events */
		duk_ತಳ್ಳು_pointer(ctx, evt);
		ಯದಿ (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_ತೆಗೆ(ctx);
			/* ... events */
			duk_ತಳ್ಳು_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_ತಳ್ಳು_pointer(ctx, evt);
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

ಸ್ಥಿರ ಶೂನ್ಯ dukky_ತಳ್ಳು_handler_code(duk_context *ctx, dom_string *name,
				     dom_event_target *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_ಗಂಟು_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	ಯದಿ (et == ರಿಕ್ತ) {
		duk_ತಳ್ಳು_lstring(ctx, "", 0);
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_ಗಂಟು_get_ಗಂಟು_type(et, &ntype);
	ಯದಿ (exc != DOM_NO_ERR) {
		duk_ತಳ್ಳು_lstring(ctx, "", 0);
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (ntype != DOM_ELEMENT_ಗಂಟು) {
		duk_ತಳ್ಳು_lstring(ctx, "", 0);
		ಪ್ರತ್ಯಾಗಮ;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	ಯದಿ (exc != DOM_NO_ERR) {
		duk_ತಳ್ಳು_lstring(ctx, "", 0);
		ಪ್ರತ್ಯಾಗಮ;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	ಯದಿ ((exc != DOM_NO_ERR) || (val == ರಿಕ್ತ)) {
		dom_string_unref(onname);
		duk_ತಳ್ಳು_lstring(ctx, "", 0);
		ಪ್ರತ್ಯಾಗಮ;
	}

	dom_string_unref(onname);
	duk_ತಳ್ಳು_lstring(ctx, dom_string_data(val), dom_string_length(val));
	dom_string_unref(val);
}

bool dukky_get_current_ಮೌಲ್ಯ_of_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_target *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_ತಳ್ಳು_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	ಯದಿ (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_ತೆಗೆ_2(ctx);
		/* ... node */
		dukky_ತಳ್ಳು_handler_code(ctx, name, et);
		/* ... node handlercode? */
		/* TODO: If this is null, clean up and propagate */
		/* ... node handlercode */
		/** @todo This is entirely wrong, but it's hard to get right */
		duk_ತಳ್ಳು_string(ctx, "function (event) {");
		/* ... node handlercode prefix */
		duk_ಸೇರಿಸು(ctx, -2);
		/* ... node prefix handlercode */
		duk_ತಳ್ಳು_string(ctx, "}");
		/* ... node prefix handlercode suffix */
		duk_concat(ctx, 3);
		/* ... node fullhandlersrc */
		duk_ತಳ್ಳು_string(ctx, "internal raw uncompiled handler");
		/* ... node fullhandlersrc filename */
		ಯದಿ (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_ತೆಗೆ_2(ctx);
			ಪ್ರತ್ಯಾಗಮ false;
		}
		/* ... node handler */
		duk_ಸೇರಿಸು(ctx, -2);
		/* ... handler node */
	} ಅನ್ಯಥಾ {
		/* ... node handlers handler */
		duk_ಸೇರಿಸು(ctx, -3);
		/* ... handler node handlers */
		duk_ತೆಗೆ(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ dukky_generic_event_handler(dom_event *evt, ಶೂನ್ಯ *pw)
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
	ಯದಿ (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		ಪ್ರತ್ಯಾಗಮ;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (ಪೂರ್ಣಾಙ್ಕ)dom_string_length(name),
	      dom_string_data(name));
	exc = dom_event_get_event_phase(evt, &phase);
	ಯದಿ (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		ಪ್ರತ್ಯಾಗಮ;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_TARGET ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (ಪೂರ್ಣಾಙ್ಕ)phase);

	exc = dom_event_get_current_target(evt, &targ);
	ಯದಿ (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	ಯದಿ (phase == DOM_CAPTURING_PHASE)
		ಗಚ್ಛ handle_extras;

	/* ... */
	ಯದಿ (dukky_ತಳ್ಳು_ಗಂಟು(ctx, (dom_ಗಂಟು *)targ) == false) {
		dom_string_unref(name);
		dom_ಗಂಟು_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		ಪ್ರತ್ಯಾಗಮ;
	}
	/* ... node */
	ಯದಿ (dukky_get_current_ಮೌಲ್ಯ_of_event_handler(
		    ctx, name, (dom_event_target *)targ) == false) {
		/* ... */
		ಗಚ್ಛ handle_extras;
	}
	/* ... handler node */
	dukky_ತಳ್ಳು_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_start_time(ctx);
	ಯದಿ (duk_pcall_method(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_immediate_propagation(evt);
		ಯದಿ (exc != DOM_NO_ERR)
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

		duk_ತೆಗೆ_n(ctx, 6);
		/* ... */
		ಗಚ್ಛ handle_extras;
	}
	/* ... result */
	ಯದಿ (duk_is_boolean(ctx, -1) &&
	    duk_to_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_ತೆಗೆ(ctx);
handle_extras:
	/* ... */
	duk_ತಳ್ಳು_lstring(ctx, dom_string_data(name), dom_string_length(name));
	dukky_ತಳ್ಳು_ಗಂಟು(ctx, (dom_ಗಂಟು *)targ);
	/* ... type node */
	ಯದಿ (dukky_event_target_ತಳ್ಳು_listeners(ctx, true)) {
		/* Nothing to do */
		duk_ತೆಗೆ(ctx);
		ಗಚ್ಛ out;
	}
	/* ... sublisteners */
	duk_ತಳ್ಳು_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	ಯಾವತ್ (duk_get_prop_index(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_index(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		ಯದಿ ((event_listener_flags)duk_to_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_ತೆಗೆ(ctx);
			/* ... subl copy handler flags */
		}
		duk_ತೆಗೆ(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_index(ctx, -2, idx);
		/* ... sublisteners copy */
		idx++;
	}
	/* ... sublisteners copy undefined */
	duk_ತೆಗೆ(ctx);
	/* ... sublisteners copy */
	duk_ಸೇರಿಸು(ctx, -2);
	/* ... copy sublisteners */
	duk_ತೆಗೆ(ctx);
	/* ... copy */
	idx = 0;
	ಯಾವತ್ (duk_get_prop_index(ctx, -1, idx++)) {
		/* ... copy handler */
		ಯದಿ (duk_get_prop_index(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_ತೆಗೆ_2(ctx);
			ಅನುವರ್ತ;
		}
		duk_ತೆಗೆ(ctx);
		duk_get_prop_index(ctx, -1, 0);
		duk_get_prop_index(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_ತೆಗೆ(ctx);
		/* ... copy handler callback */
		ಯದಿ (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_ತೆಗೆ_2(ctx);
			/* ... copy */
			ಅನುವರ್ತ;
		}
		/* ... copy handler callback */
		dukky_ತಳ್ಳು_ಗಂಟು(ctx, (dom_ಗಂಟು *)targ);
		/* ... copy handler callback node */
		dukky_ತಳ್ಳು_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_start_time(ctx);
		ಯದಿ (duk_pcall_method(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_immediate_propagation(evt);
			ಯದಿ (exc != DOM_NO_ERR)
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

			duk_ತೆಗೆ_n(ctx, 7);
			/* ... copy */
			ಅನುವರ್ತ;
		}
		/* ... copy handler result */
		ಯದಿ (duk_is_boolean(ctx, -1) &&
		    duk_to_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_ತೆಗೆ_2(ctx);
		/* ... copy */
	}
	duk_ತೆಗೆ_2(ctx);
out:
	/* ... */
	dom_ಗಂಟು_unref(targ);
	dom_string_unref(name);
}

ಶೂನ್ಯ dukky_register_event_listener_for(duk_context *ctx,
				       ಸಂರಚನಾ dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = ರಿಕ್ತ;
	dom_exception exc;

	/* ... */
	ಯದಿ (ele == ರಿಕ್ತ) {
		/* A null element is the Window object */
		duk_ತಳ್ಳು_global_object(ctx);
	} ಅನ್ಯಥಾ {
		/* Non null elements must be pushed as a node object */
		ಯದಿ (dukky_ತಳ್ಳು_ಗಂಟು(ctx, (ಸಂರಚನಾ dom_ಗಂಟು *)ele) == false)
			ಪ್ರತ್ಯಾಗಮ;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_ತಳ್ಳು_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	ಯದಿ (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_ತೆಗೆ_2(ctx);
		/* ... */
		ಪ್ರತ್ಯಾಗಮ;
	}
	/* ... node handlers */
	duk_ತಳ್ಳು_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_ತಳ್ಳು_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_ತೆಗೆ_2(ctx);
	/* ... */
	ಯದಿ (ele == ರಿಕ್ತ) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	ಯದಿ (exc != DOM_NO_ERR) ಪ್ರತ್ಯಾಗಮ;
	exc = dom_event_target_add_event_listener(
		ele, name, listen, capture);
	ಯದಿ (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (ಪೂರ್ಣಾಙ್ಕ)dom_string_length(name), dom_string_data(name));
	} ಅನ್ಯಥಾ {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (ಪೂರ್ಣಾಙ್ಕ)dom_string_length(name), dom_string_data(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_target_ತಳ್ಳು_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	ಯದಿ (duk_is_undefined(ctx, -1)) {
		/* ... type this null */
		duk_ತೆಗೆ(ctx);
		duk_ತಳ್ಳು_object(ctx);
		duk_dup(ctx, -1);
		/* ... type this listeners listeners */
		duk_put_prop_string(ctx, -3, EVENT_LISTENER_JS_MAGIC);
		/* ... type this listeners */
	}
	/* ... type this listeners */
	duk_ಸೇರಿಸು(ctx, -3);
	/* ... listeners type this */
	duk_ತೆಗೆ(ctx);
	/* ... listeners type */
	duk_dup(ctx, -1);
	/* ... listeners type type */
	duk_get_prop(ctx, -3);
	/* ... listeners type ??? */
	ಯದಿ (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		ಯದಿ (dont_create == true) {
			duk_ತೆಗೆ_3(ctx);
			duk_ತಳ್ಳು_undefined(ctx);
			ಪ್ರತ್ಯಾಗಮ true;
		}
		duk_ತೆಗೆ(ctx);
		duk_ತಳ್ಳು_array(ctx);
		duk_dup(ctx, -2);
		duk_dup(ctx, -2);
		/* ... listeners type sublisteners type sublisteners */
		duk_put_prop(ctx, -5);
		/* ... listeners type sublisteners */
		ret = true;
	}
	duk_ಸೇರಿಸು(ctx, -3);
	/* ... sublisteners listeners type */
	duk_ತೆಗೆ_2(ctx);
	/* ... sublisteners */
	ಪ್ರತ್ಯಾಗಮ ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
ಶೂನ್ಯ dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	ಯಾವತ್ (duk_get_prop_index(ctx, -1, idx + 1)) {
		duk_put_prop_index(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_index(ctx, -2, idx + 1);
	duk_ತೆಗೆ(ctx);
}


ಶೂನ್ಯ js_handle_new_element(jsthread *thread, ಸಂರಚನಾ dom_element *ಗಂಟು)
{
	ಪ್ರತಿಜ್ಞಾ(thread);
	ಪ್ರತಿಜ್ಞಾ(ಗಂಟು);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = ರಿಕ್ತ;
	dom_string *ಕೀಲಿ = ರಿಕ್ತ;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_ಗಂಟು_get_ಗಂಟು_name(ಗಂಟು, &nodename);
	ಯದಿ (exc != DOM_NO_ERR) ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_ಗಂಟು_get_attributes(ಗಂಟು, &map);
	ಯದಿ (exc != DOM_NO_ERR) ಪ್ರತ್ಯಾಗಮ;
	ಯದಿ (map == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_length(map, &siz);
	ಯದಿ (exc != DOM_NO_ERR) ಗಚ್ಛ out;

	ಪ್ರತಿ (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_item(map, idx, &attr);
		ಯದಿ (exc != DOM_NO_ERR) ಗಚ್ಛ out;
		exc = dom_attr_get_name(attr, &ಕೀಲಿ);
		ಯದಿ (exc != DOM_NO_ERR) ಗಚ್ಛ out;
		ಯದಿ (is_body && (
			    ಕೀಲಿ == corestring_dom_onblur ||
			    ಕೀಲಿ == corestring_dom_onerror ||
			    ಕೀಲಿ == corestring_dom_onfocus ||
			    ಕೀಲಿ == corestring_dom_onload ||
			    ಕೀಲಿ == corestring_dom_onresize ||
			    ಕೀಲಿ == corestring_dom_onscroll)) {
			/* This is a forwarded event, it doesn't matter,
			 * we should skip registering for it and later
			 * we will register it for Window itself
			 */
			ಗಚ್ಛ skip_register;
		}
		ಯದಿ (dom_string_length(ಕೀಲಿ) > 2) {
			/* Can be on* */
			ಧ್ರುವ uint8_t *data = (ಧ್ರುವ uint8_t *)dom_string_data(ಕೀಲಿ);
			ಯದಿ (data[0] == 'o' && data[1] == 'n') {
				dom_string *sub = ರಿಕ್ತ;
				exc = dom_string_substr(
					ಕೀಲಿ, 2, dom_string_length(ಕೀಲಿ),
					&sub);
				ಯದಿ (exc == DOM_NO_ERR) {
					dukky_register_event_listener_for(
						CTX, ಗಂಟು, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_register:
		dom_string_unref(ಕೀಲಿ); ಕೀಲಿ = ರಿಕ್ತ;
		dom_ಗಂಟು_unref(attr); attr = ರಿಕ್ತ;
	}

out:
	ಯದಿ (ಕೀಲಿ != ರಿಕ್ತ)
		dom_string_unref(ಕೀಲಿ);

	ಯದಿ (attr != ರಿಕ್ತ)
		dom_ಗಂಟು_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

ಶೂನ್ಯ js_event_cleanup(jsthread *thread, ಸಂರಚನಾ dom_event *evt)
{
	ಪ್ರತಿಜ್ಞಾ(thread);
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, EVENT_MAGIC);
	/* ... EVENT_MAP */
	duk_ತಳ್ಳು_pointer(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_ತೆಗೆ(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, ಧ್ರುವ ವರ್ಣ *type, ಸಂರಚನಾ dom_document *doc, ಸಂರಚನಾ dom_ಗಂಟು *target)
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

	ಯದಿ (target != ರಿಕ್ತ)
		/* Swallow non-Window-targetted events quietly */
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (strcmp(type, "load") != 0)
		/* Swallow non-load events quietly */
		ಪ್ರತ್ಯಾಗಮ true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	ಯದಿ (exc != DOM_NO_ERR) ಪ್ರತ್ಯಾಗಮ true;
	exc = dom_event_init(evt, corestring_dom_load, false, false);
	ಯದಿ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ಪ್ರತ್ಯಾಗಮ true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_ತಳ್ಳು_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	ಯದಿ (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_ತೆಗೆ(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		ಯದಿ (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			ಪ್ರತ್ಯಾಗಮ true;
		}
		dukky_ತಳ್ಳು_ಗಂಟು(CTX, (ಸಂರಚನಾ dom_ಗಂಟು *)body);
		/* ... handlers bodynode */
		ಯದಿ (dukky_get_current_ಮೌಲ್ಯ_of_event_handler(
			    CTX, corestring_dom_load, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_ಗಂಟು_unref(body);
			/* ... handlers */
			duk_ತೆಗೆ(CTX);
			dukky_leave_thread(thread);
			ಪ್ರತ್ಯಾಗಮ true;
		}
		/* Unref the body, we don't need it any more */
		dom_ಗಂಟು_unref(body);
		/* ... handlers handler bodynode */
		duk_ತೆಗೆ(CTX);
	}
	/* ... handlers handler */
	duk_ಸೇರಿಸು(CTX, -2);
	/* ... handler handlers */
	duk_ತೆಗೆ(CTX);
	/* ... handler */
	duk_ತಳ್ಳು_global_object(CTX);
	/* ... handler Window */
	dukky_ತಳ್ಳು_event(CTX, evt);
	/* ... handler Window event */
	dukky_reset_start_time(CTX);
	ಯದಿ (duk_pcall_method(CTX, 1) != 0) {
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

		duk_ತೆಗೆ_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		ಪ್ರತ್ಯಾಗಮ true;
	}
	/* ... result */
	duk_ತೆಗೆ(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	ಪ್ರತ್ಯಾಗಮ true;
}
