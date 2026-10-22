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

#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <nsutils/time.h>

#അന്തര്ഭാവ "netsurf/inttypes.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "content/content.h"

#അന്തര്ഭാവ "javascript/js.h"
#അന്തര്ഭാവ "javascript/content.h"

#അന്തര്ഭാവ "duktape/binding.h"
#അന്തര്ഭാവ "duktape/generics.js.inc"
#അന്തര്ഭാവ "duktape/polyfill.js.inc"

#അന്തര്ഭാവ "duktape.h"
#അന്തര്ഭാവ "dukky.h"

#അന്തര്ഭാവ <dom/dom.h>

#define EVENT_MAGIC MAGIC(EVENT_MAP)
#define HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#define HANDLER_MAGIC MAGIC(HANDLER_MAP)
#define EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#define GENERICS_MAGIC MAGIC(GENERICS_TABLE)
#define THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
ഘടന jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t next_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	unsigned പൂർണ്ണസംഖ്യ live_threads; /**< number of live threads */
	uint64_t exec_start_time;
};

/**
 * dukky javascript thread
 */
ഘടന jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	unsigned പൂർണ്ണസംഖ്യ in_use; /**< The number of times this thread is in use */
	jsheap *heap; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

സ്ഥിതം duk_ret_t dukky_populate_object(duk_context *ctx, ശൂന്യം *udata)
{
	/* ... obj args protoname nargs */
	പൂർണ്ണസംഖ്യ nargs = duk_get_int(ctx, -1);
	duk_എടുക്കുക(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	എങ്കിൽ (duk_is_undefined(ctx, -1)) {
		NSLOG(dukky, WARNING,
		      "Unable to find dukky prototype for `%s` - falling back to HTMLUnknownElement",
		      duk_get_string(ctx, -3) + 2 /* Skip the two unprintables */);
		duk_എടുക്കുക(ctx);
		duk_തള്ളുക_string(ctx, PROTO_NAME(HTMLUNKNOWNELEMENT));
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
	duk_തിരുകുക(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_എടുക്കുക_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	തിരികെ 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ args)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, args);
	/* ... args */
	duk_തള്ളുക_object(ctx);
	/* ... args obj */
	duk_തള്ളുക_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
	/* ... args obj */
	duk_തള്ളുക_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
	/* ... args obj */
	duk_തിരുകുക(ctx, -(args+1));
	/* ... obj args */
	duk_തള്ളുക_string(ctx, name);
	/* ... obj args name */
	duk_തള്ളുക_int(ctx, args);
	/* ... obj args name nargs */
	എങ്കിൽ ((ret = duk_safe_call(ctx, dukky_populate_object, ശൂന്യ, args + 3, 1))
	    != DUK_EXEC_SUCCESS)
		തിരികെ ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	തിരികെ DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_തള്ളുക_കണ്ണി_stacked(duk_context *ctx)
{
	പൂർണ്ണസംഖ്യ മുകൾ_at_fail = duk_get_മുകൾ(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, കണ്ണി_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	എങ്കിൽ (duk_is_undefined(ctx, -1)) {
		/* ... nodeptr klass nodes undefined */
		duk_എടുക്കുക(ctx);
		/* ... nodeptr klass nodes */
		duk_തള്ളുക_object(ctx);
		/* ... nodeptr klass nodes obj */
		duk_തള്ളുക_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_തള്ളുക_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr klass */
		duk_തള്ളുക_int(ctx, 1);
		/* ... nodeptr klass nodes obj nodeptr klass 1 */
		എങ്കിൽ (duk_safe_call(ctx, dukky_populate_object, ശൂന്യ, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_set_മുകൾ(ctx, മുകൾ_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			തിരികെ false;
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
	duk_തിരുകുക(ctx, -4);
	/* ... node nodeptr klass nodes */
	duk_എടുക്കുക_3(ctx);
	/* ... node */
	എങ്കിൽ (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		സ്ഥിരം അക്ഷരം * what = duk_safe_to_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_എടുക്കുക(ctx);
	}
	തിരികെ true;
}

#define SET_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

സ്ഥിതം ശൂന്യം dukky_html_element_class_from_tag_type(dom_html_element_type type,
		സ്ഥിരം അക്ഷരം **html_class, size_t *html_class_len)
{
	തിരഞ്ഞെടുപ്പ്(type) {
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_HTML:
		SET_HTML_CLASS(HTML)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_HEAD:
		SET_HTML_CLASS(HEAD)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_META:
		SET_HTML_CLASS(META)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_BASE:
		SET_HTML_CLASS(BASE)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_TITLE:
		SET_HTML_CLASS(TITLE)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_BODY:
		SET_HTML_CLASS(BODY)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_DIV:
		SET_HTML_CLASS(DIV)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_FORM:
		SET_HTML_CLASS(FORM)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_LINK:
		SET_HTML_CLASS(LINK)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_BUTTON:
		SET_HTML_CLASS(BUTTON)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_നിവേശം:
		SET_HTML_CLASS(നിവേശം_2)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		SET_HTML_CLASS(TEXTAREA)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		SET_HTML_CLASS(OPTGROUP)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_OPTION:
		SET_HTML_CLASS(OPTION)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_SELECT:
		SET_HTML_CLASS(SELECT)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_HR:
		SET_HTML_CLASS(HR)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_DL:
		SET_HTML_CLASS(DLIST)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_DIR:
		SET_HTML_CLASS(DIRECTORY)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_MENU:
		SET_HTML_CLASS(MENU)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_FIELDSET:
		SET_HTML_CLASS(FIELDSET)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_LEGEND:
		SET_HTML_CLASS(LEGEND)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_P:
		SET_HTML_CLASS(PARAGRAPH)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_H1:
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_H2:
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_H3:
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_H4:
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_H5:
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_H6:
		SET_HTML_CLASS(HEADING)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_Q:
		SET_HTML_CLASS(QUOTE)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_PRE:
		SET_HTML_CLASS(PRE)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_BR:
		SET_HTML_CLASS(BR)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_LABEL:
		SET_HTML_CLASS(LABEL)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_UL:
		SET_HTML_CLASS(ULIST)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_OL:
		SET_HTML_CLASS(OLIST)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_LI:
		SET_HTML_CLASS(LI)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_FONT:
		SET_HTML_CLASS(FONT)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_DEL:
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_INS:
		SET_HTML_CLASS(MOD)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_A:
		SET_HTML_CLASS(ANCHOR)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_BASEFONT:
		SET_HTML_CLASS(BASEFONT)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_IMG:
		SET_HTML_CLASS(IMAGE)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_OBJECT:
		SET_HTML_CLASS(OBJECT)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_PARAM:
		SET_HTML_CLASS(PARAM)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_APPLET:
		SET_HTML_CLASS(APPLET)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_MAP:
		SET_HTML_CLASS(MAP)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_AREA:
		SET_HTML_CLASS(AREA)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_SCRIPT:
		SET_HTML_CLASS(SCRIPT)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_CAPTION:
		SET_HTML_CLASS(TABLECAPTION)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_TD:
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_TH:
		SET_HTML_CLASS(TABLECELL)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_COL:
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_COLGROUP:
		SET_HTML_CLASS(TABLECOL)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_THEAD:
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_TBODY:
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_TFOOT:
		SET_HTML_CLASS(TABLESECTION)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_TABLE:
		SET_HTML_CLASS(TABLE)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_TR:
		SET_HTML_CLASS(TABLEROW)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_STYLE:
		SET_HTML_CLASS(STYLE)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_FRAMESET:
		SET_HTML_CLASS(FRAMESET)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_FRAME:
		SET_HTML_CLASS(FRAME)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_IFRAME:
		SET_HTML_CLASS(IFRAME)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_ISINDEX:
		SET_HTML_CLASS(ISINDEX)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_CANVAS:
		SET_HTML_CLASS(CANVAS)
		നിർത്തുക;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_എണ്ണം:
		പ്രതിജ്ഞാ(type != DOM_HTML_ELEMENT_TYPE_എണ്ണം);
		fallthrough;
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		SET_HTML_CLASS(UNKNOWN)
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		നിർത്തുക;
	}
	തിരികെ;
}

#undef SET_HTML_CLASS

സ്ഥിതം ശൂന്യം
dukky_തള്ളുക_കണ്ണി_klass(duk_context *ctx, ഘടന dom_കണ്ണി *കണ്ണി)
{
	dom_കണ്ണി_type nodetype;
	dom_exception err;

	err = dom_കണ്ണി_get_കണ്ണി_type(കണ്ണി, &nodetype);
	എങ്കിൽ (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_തള്ളുക_string(ctx, PROTO_NAME(കണ്ണി_2));
		തിരികെ;
	}

	തിരഞ്ഞെടുപ്പ്(nodetype) {
	അവസ്ഥ DOM_ELEMENT_കണ്ണി: {
		dom_string *namespace;
		dom_html_element_type type;
		സ്ഥിരം അക്ഷരം *html_class;
		size_t html_class_len;
		err = dom_കണ്ണി_get_namespace(കണ്ണി, &namespace);
		എങ്കിൽ (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_തള്ളുക_string(ctx, PROTO_NAME(ELEMENT));
			നിർത്തുക;
		}
		എങ്കിൽ (namespace == ശൂന്യ) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_തള്ളുക_string(ctx, PROTO_NAME(ELEMENT));
			നിർത്തുക;
		}

		എങ്കിൽ (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_തള്ളുക_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			നിർത്തുക;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_tag_type(കണ്ണി, &type);
		എങ്കിൽ (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_tag_type(type,
				&html_class, &html_class_len);

		duk_തള്ളുക_lstring(ctx, html_class, html_class_len);
		നിർത്തുക;
	}
	അവസ്ഥ DOM_TEXT_കണ്ണി:
		duk_തള്ളുക_string(ctx, PROTO_NAME(TEXT));
		നിർത്തുക;
	അവസ്ഥ DOM_COMMENT_കണ്ണി:
		duk_തള്ളുക_string(ctx, PROTO_NAME(COMMENT));
		നിർത്തുക;
	അവസ്ഥ DOM_DOCUMENT_കണ്ണി:
		duk_തള്ളുക_string(ctx, PROTO_NAME(DOCUMENT));
		നിർത്തുക;
	അവസ്ഥ DOM_ATTRIBUTE_കണ്ണി:
	അവസ്ഥ DOM_PROCESSING_INSTRUCTION_കണ്ണി:
	അവസ്ഥ DOM_DOCUMENT_TYPE_കണ്ണി:
	അവസ്ഥ DOM_DOCUMENT_FRAGMENT_കണ്ണി:
	അവസ്ഥ DOM_NOTATION_കണ്ണി:
	അവസ്ഥ DOM_ENTITY_REFERENCE_കണ്ണി:
	അവസ്ഥ DOM_ENTITY_കണ്ണി:
	അവസ്ഥ DOM_CDATA_SECTION_കണ്ണി:
	സ്ഥിരസ്ഥിതി:
		/* Oh bum, just node then */
		duk_തള്ളുക_string(ctx, PROTO_NAME(കണ്ണി_2));
	}
}

duk_bool_t
dukky_തള്ളുക_കണ്ണി(duk_context *ctx, ഘടന dom_കണ്ണി *കണ്ണി)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", കണ്ണി);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, കണ്ണി_MAGIC);
	/* ... nodes */
	duk_തള്ളുക_pointer(ctx, കണ്ണി);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	എങ്കിൽ (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_തിരുകുക(ctx, -2);
		/* ... node nodes */
		duk_എടുക്കുക(ctx);
		/* ... node */
		എങ്കിൽ (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			സ്ഥിരം അക്ഷരം * what = duk_safe_to_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_എടുക്കുക(ctx);
		}
		തിരികെ true;
	}
	/* ... nodes undefined */
	duk_എടുക്കുക_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_തള്ളുക_pointer(ctx, കണ്ണി);
	/* ... nodeptr */
	dukky_തള്ളുക_കണ്ണി_klass(ctx, കണ്ണി);
	/* ... nodeptr klass */
	തിരികെ dukky_തള്ളുക_കണ്ണി_stacked(ctx);
}

സ്ഥിതം duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	തിരികെ duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

ശൂന്യം
dukky_inject_not_ctr(duk_context *ctx, പൂർണ്ണസംഖ്യ idx, സ്ഥിരം അക്ഷരം *name)
{
	/* ... p[idx] ... proto */
	duk_തള്ളുക_c_function(ctx, dukky_bad_constructor, 0);
	/* ... p[idx] ... proto cons */
	duk_തിരുകുക(ctx, -2);
	/* ... p[idx] ... cons proto */
	duk_put_prop_string(ctx, -2, "prototype");
	/* ... p[idx] ... cons[proto] */
	duk_put_prop_string(ctx, idx, name);
	/* ... p ... */
	തിരികെ;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

സ്ഥിതം ശൂന്യം *dukky_alloc_function(ശൂന്യം *udata, duk_size_t size)
{
	എങ്കിൽ (size == 0)
		തിരികെ ശൂന്യ;

	തിരികെ malloc(size);
}

സ്ഥിതം ശൂന്യം *dukky_realloc_function(ശൂന്യം *udata, ശൂന്യം *ptr, duk_size_t size)
{
	എങ്കിൽ (ptr == ശൂന്യ && size == 0)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (size == 0) {
		സ്വതന്ത്രം(ptr);
		തിരികെ ശൂന്യ;
	}

	തിരികെ realloc(ptr, size);
}


സ്ഥിതം ശൂന്യം dukky_free_function(ശൂന്യം *udata, ശൂന്യം *ptr)
{
	എങ്കിൽ (ptr != ശൂന്യ)
		സ്വതന്ത്രം(ptr);
}

/* exported interface documented in js.h */
ശൂന്യം js_initialise(ശൂന്യം)
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
ശൂന്യം js_finalise(ശൂന്യം)
{
	/* NADA for now */
}


/* exported interface documented in js.h */
nserror
js_newheap(പൂർണ്ണസംഖ്യ timeout, jsheap **heap)
{
	duk_context *ctx;
	jsheap *ret = calloc(1, വലുപ്പം(*ret));
	*heap = ശൂന്യ;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	എങ്കിൽ (ret == ശൂന്യ) തിരികെ NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_heap(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		ശൂന്യ);
	എങ്കിൽ (ret->ctx == ശൂന്യ) { സ്വതന്ത്രം(ret); തിരികെ NSERROR_NOMEM; }
	/* Create the prototype stuffs */
	duk_തള്ളുക_global_object(ctx);
	duk_തള്ളുക_boolean(ctx, true);
	duk_put_prop_string(ctx, -2, "protos");
	duk_put_global_string(ctx, PROTO_MAGIC);
	/* Create prototypes here */
	dukky_create_prototypes(ctx);
	/* Now create the thread map */
	duk_തള്ളുക_object(ctx);
	duk_put_global_string(ctx, THREAD_MAP);

	*heap = ret;
	തിരികെ NSERROR_OK;
}


സ്ഥിതം ശൂന്യം dukky_destroyheap(jsheap *heap)
{
	പ്രതിജ്ഞാ(heap->pending_destroy == true);
	പ്രതിജ്ഞാ(heap->live_threads == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_heap(heap->ctx);
	സ്വതന്ത്രം(heap);
}

/* exported interface documented in js.h */
ശൂന്യം js_destroyheap(jsheap *heap)
{
	heap->pending_destroy = true;
	എങ്കിൽ (heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* Just for here, the CTX is in ret, not thread */
#define CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *heap, ശൂന്യം *win_priv, ശൂന്യം *doc_priv, jsthread **thread)
{
	jsthread *ret;
	പ്രതിജ്ഞാ(heap != ശൂന്യ);
	പ്രതിജ്ഞാ(heap->pending_destroy == false);

	ret = calloc(1, വലുപ്പം (*ret));
	എങ്കിൽ (ret == ശൂന്യ) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		തിരികെ NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_തള്ളുക_thread(heap->ctx); /* ... threads thread */
	ret->heap = heap;
	ret->ctx = duk_require_context(heap->ctx, -1);
	ret->thread_idx = heap->next_thread++;
	duk_put_prop_index(heap->ctx, -2, ret->thread_idx);
	heap->live_threads++;
	duk_എടുക്കുക(heap->ctx); /* ... */
	duk_തള്ളുക_int(CTX, 0);
	duk_തള്ളുക_int(CTX, 1);
	duk_തള്ളുക_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_തള്ളുക_pointer(CTX, win_priv);
	duk_തള്ളുക_pointer(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(WINDOW), 2);
	duk_തള്ളുക_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_set_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_തള്ളുക_object(CTX);
	duk_തള്ളുക_pointer(CTX, ശൂന്യ);
	duk_തള്ളുക_null(CTX);
	duk_put_prop(CTX, -3);
	duk_put_global_string(CTX, കണ്ണി_MAGIC);

	/* And now the event mapping table */
	duk_തള്ളുക_object(CTX);
	duk_put_global_string(CTX, EVENT_MAGIC);

	/* Now load the polyfills */
	/* ... */
	duk_തള്ളുക_string(CTX, "polyfill.js");
	/* ..., polyfill.js */
	എങ്കിൽ (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (സ്ഥിരം അക്ഷരം *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		തിരികെ NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	എങ്കിൽ (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		തിരികെ NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_എടുക്കുക(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_തള്ളുക_string(CTX, "generics.js");
	/* ..., generics.js */
	എങ്കിൽ (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (സ്ഥിരം അക്ഷരം *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		തിരികെ NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	എങ്കിൽ (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		തിരികെ NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_എടുക്കുക(CTX);
	/* ... */
	duk_തള്ളുക_global_object(CTX);
	/* ..., Win */
	duk_get_prop_string(CTX, -1, "NetSurf");
	/* ..., Win, NetSurf */
	duk_put_global_string(CTX, GENERICS_MAGIC);
	/* ..., Win */
	duk_del_prop_string(CTX, -1, "NetSurf");
	duk_എടുക്കുക(CTX);
	/* ... */

	dukky_log_അട്ടി_frame(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, heap);
	*thread = ret;

	തിരികെ NSERROR_OK;
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
	duk_int_t മുകൾ = duk_get_മുകൾ(CTX);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, thread->heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Restore whatever stack we had */
	duk_set_മുകൾ(CTX, മുകൾ);

	തിരികെ NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
സ്ഥിതം ശൂന്യം dukky_destroythread(jsthread *thread)
{
	jsheap *heap = thread->heap;

	പ്രതിജ്ഞാ(thread->in_use == 0);
	പ്രതിജ്ഞാ(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_index(heap->ctx, -1, thread->thread_idx);
	duk_എടുക്കുക(heap->ctx); /* ... */

	/* We can now free the thread object */
	സ്വതന്ത്രം(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(heap->ctx, 0);
	duk_gc(heap->ctx, DUK_GC_COMPACT);
	heap->live_threads--;

	/* And if the heap should now go, blow it away */
	എങ്കിൽ (heap->pending_destroy == true && heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* exported interface documented in js.h */
ശൂന്യം js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	എങ്കിൽ (thread->in_use == 0) {
		dukky_destroythread(thread);
	}
}

സ്ഥിതം ശൂന്യം dukky_enter_thread(jsthread *thread)
{
	പ്രതിജ്ഞാ(thread != ശൂന്യ);
	thread->in_use++;
}

സ്ഥിതം ശൂന്യം dukky_leave_thread(jsthread *thread)
{
	പ്രതിജ്ഞാ(thread != ശൂന്യ);
	പ്രതിജ്ഞാ(thread->in_use > 0);

	thread->in_use--;
	എങ്കിൽ (thread->in_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(ശൂന്യം *udata)
{
#define JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *heap = (jsheap *) udata;
	uint64_t now;

	(ശൂന്യം) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	തിരികെ heap->exec_start_time != 0 &&
			now > (heap->exec_start_time + JS_EXEC_TIMEOUT_MS);
}

സ്ഥിതം ശൂന്യം dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_മുകൾ(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_safe_to_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_എടുക്കുക(ctx);
	/* ..., errobj */
}

സ്ഥിതം ശൂന്യം dukky_reset_start_time(duk_context *ctx)
{
	duk_memory_functions funcs;
	jsheap *heap;
	duk_get_memory_functions(ctx, &funcs);
	heap = funcs.udata;
	(ശൂന്യം) nsu_getmonotonic_ms(&heap->exec_start_time);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_size_t argc, bool reset_timeout)
{
	എങ്കിൽ (reset_timeout) {
		dukky_reset_start_time(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	എങ്കിൽ (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	തിരികെ ret;
}


ശൂന്യം dukky_തള്ളുക_generics(duk_context *ctx, സ്ഥിരം അക്ഷരം *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

സ്ഥിതം duk_int_t dukky_തള്ളുക_context_dump(duk_context *ctx, ശൂന്യം *udata)
{
	duk_തള്ളുക_context_dump(ctx);
	തിരികെ 1;
}

ശൂന്യം dukky_log_അട്ടി_frame(duk_context *ctx, സ്ഥിരം അക്ഷരം * reason)
{
	എങ്കിൽ (duk_safe_call(ctx, dukky_തള്ളുക_context_dump, ശൂന്യ, 0, 1) != 0) {
		duk_എടുക്കുക(ctx);
		duk_തള്ളുക_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_safe_to_string(ctx, -1));
	duk_എടുക്കുക(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, സ്ഥിരം uint8_t *txt, size_t txtlen, സ്ഥിരം അക്ഷരം *name)
{
	bool ret = false;
	പ്രതിജ്ഞാ(thread);

	എങ്കിൽ (txt == ശൂന്യ || txtlen == 0) {
		തിരികെ false;
	}

	എങ്കിൽ (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		തിരികെ false;
	}

	dukky_enter_thread(thread);

	duk_set_മുകൾ(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_start_time(CTX);
	എങ്കിൽ (name != ശൂന്യ) {
		duk_തള്ളുക_string(CTX, name);
	} അല്ലെങ്കിൽ {
		duk_തള്ളുക_string(CTX, "?unknown source?");
	}
	എങ്കിൽ (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (സ്ഥിരം അക്ഷരം *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		പോകുക handle_error;
	}

	എങ്കിൽ (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		പോകുക handle_error;
	}

	എങ്കിൽ (duk_get_മുകൾ(CTX) == 0) duk_തള്ളുക_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	പോകുക out;

handle_error:
	dukky_dump_error(CTX);
out:
	dukky_leave_thread(thread);
	തിരികെ ret;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം* dukky_event_proto(dom_event *evt)
{
	സ്ഥിരം അക്ഷരം *ret = PROTO_NAME(EVENT);
	dom_string *type = ശൂന്യ;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	എങ്കിൽ (err != DOM_NO_ERR) {
		പോകുക out;
	}

	എങ്കിൽ (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		പോകുക out;
	} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		പോകുക out;
	} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		പോകുക out;
	}

out:
	എങ്കിൽ (type != ശൂന്യ) {
		dom_string_unref(type);
	}

	തിരികെ ret;
}

/*** New style event handling ***/

ശൂന്യം dukky_തള്ളുക_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_തള്ളുക_pointer(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	എങ്കിൽ (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_എടുക്കുക(ctx);
		/* ... events */
		duk_തള്ളുക_pointer(ctx, evt);
		എങ്കിൽ (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_എടുക്കുക(ctx);
			/* ... events */
			duk_തള്ളുക_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_തള്ളുക_pointer(ctx, evt);
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

സ്ഥിതം ശൂന്യം dukky_തള്ളുക_handler_code(duk_context *ctx, dom_string *name,
				     dom_event_target *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_കണ്ണി_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	എങ്കിൽ (et == ശൂന്യ) {
		duk_തള്ളുക_lstring(ctx, "", 0);
		തിരികെ;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_കണ്ണി_get_കണ്ണി_type(et, &ntype);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		duk_തള്ളുക_lstring(ctx, "", 0);
		തിരികെ;
	}

	എങ്കിൽ (ntype != DOM_ELEMENT_കണ്ണി) {
		duk_തള്ളുക_lstring(ctx, "", 0);
		തിരികെ;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		duk_തള്ളുക_lstring(ctx, "", 0);
		തിരികെ;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (val == ശൂന്യ)) {
		dom_string_unref(onname);
		duk_തള്ളുക_lstring(ctx, "", 0);
		തിരികെ;
	}

	dom_string_unref(onname);
	duk_തള്ളുക_lstring(ctx, dom_string_data(val), dom_string_length(val));
	dom_string_unref(val);
}

bool dukky_get_current_മൂല്യം_of_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_target *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_തള്ളുക_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	എങ്കിൽ (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_എടുക്കുക_2(ctx);
		/* ... node */
		dukky_തള്ളുക_handler_code(ctx, name, et);
		/* ... node handlercode? */
		/* TODO: If this is null, clean up and propagate */
		/* ... node handlercode */
		/** @todo This is entirely wrong, but it's hard to get right */
		duk_തള്ളുക_string(ctx, "function (event) {");
		/* ... node handlercode prefix */
		duk_തിരുകുക(ctx, -2);
		/* ... node prefix handlercode */
		duk_തള്ളുക_string(ctx, "}");
		/* ... node prefix handlercode suffix */
		duk_concat(ctx, 3);
		/* ... node fullhandlersrc */
		duk_തള്ളുക_string(ctx, "internal raw uncompiled handler");
		/* ... node fullhandlersrc filename */
		എങ്കിൽ (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_എടുക്കുക_2(ctx);
			തിരികെ false;
		}
		/* ... node handler */
		duk_തിരുകുക(ctx, -2);
		/* ... handler node */
	} അല്ലെങ്കിൽ {
		/* ... node handlers handler */
		duk_തിരുകുക(ctx, -3);
		/* ... handler node handlers */
		duk_എടുക്കുക(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	തിരികെ true;
}

സ്ഥിതം ശൂന്യം dukky_generic_event_handler(dom_event *evt, ശൂന്യം *pw)
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
	എങ്കിൽ (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		തിരികെ;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (പൂർണ്ണസംഖ്യ)dom_string_length(name),
	      dom_string_data(name));
	exc = dom_event_get_event_phase(evt, &phase);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		തിരികെ;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_TARGET ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (പൂർണ്ണസംഖ്യ)phase);

	exc = dom_event_get_current_target(evt, &targ);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		തിരികെ;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	എങ്കിൽ (phase == DOM_CAPTURING_PHASE)
		പോകുക handle_extras;

	/* ... */
	എങ്കിൽ (dukky_തള്ളുക_കണ്ണി(ctx, (dom_കണ്ണി *)targ) == false) {
		dom_string_unref(name);
		dom_കണ്ണി_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		തിരികെ;
	}
	/* ... node */
	എങ്കിൽ (dukky_get_current_മൂല്യം_of_event_handler(
		    ctx, name, (dom_event_target *)targ) == false) {
		/* ... */
		പോകുക handle_extras;
	}
	/* ... handler node */
	dukky_തള്ളുക_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_start_time(ctx);
	എങ്കിൽ (duk_pcall_method(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_immediate_propagation(evt);
		എങ്കിൽ (exc != DOM_NO_ERR)
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

		duk_എടുക്കുക_n(ctx, 6);
		/* ... */
		പോകുക handle_extras;
	}
	/* ... result */
	എങ്കിൽ (duk_is_boolean(ctx, -1) &&
	    duk_to_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_എടുക്കുക(ctx);
handle_extras:
	/* ... */
	duk_തള്ളുക_lstring(ctx, dom_string_data(name), dom_string_length(name));
	dukky_തള്ളുക_കണ്ണി(ctx, (dom_കണ്ണി *)targ);
	/* ... type node */
	എങ്കിൽ (dukky_event_target_തള്ളുക_listeners(ctx, true)) {
		/* Nothing to do */
		duk_എടുക്കുക(ctx);
		പോകുക out;
	}
	/* ... sublisteners */
	duk_തള്ളുക_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	ആകുമ്പോൾ (duk_get_prop_index(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_index(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		എങ്കിൽ ((event_listener_flags)duk_to_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_എടുക്കുക(ctx);
			/* ... subl copy handler flags */
		}
		duk_എടുക്കുക(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_index(ctx, -2, idx);
		/* ... sublisteners copy */
		idx++;
	}
	/* ... sublisteners copy undefined */
	duk_എടുക്കുക(ctx);
	/* ... sublisteners copy */
	duk_തിരുകുക(ctx, -2);
	/* ... copy sublisteners */
	duk_എടുക്കുക(ctx);
	/* ... copy */
	idx = 0;
	ആകുമ്പോൾ (duk_get_prop_index(ctx, -1, idx++)) {
		/* ... copy handler */
		എങ്കിൽ (duk_get_prop_index(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_എടുക്കുക_2(ctx);
			തുടരുക;
		}
		duk_എടുക്കുക(ctx);
		duk_get_prop_index(ctx, -1, 0);
		duk_get_prop_index(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_എടുക്കുക(ctx);
		/* ... copy handler callback */
		എങ്കിൽ (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_എടുക്കുക_2(ctx);
			/* ... copy */
			തുടരുക;
		}
		/* ... copy handler callback */
		dukky_തള്ളുക_കണ്ണി(ctx, (dom_കണ്ണി *)targ);
		/* ... copy handler callback node */
		dukky_തള്ളുക_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_start_time(ctx);
		എങ്കിൽ (duk_pcall_method(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_immediate_propagation(evt);
			എങ്കിൽ (exc != DOM_NO_ERR)
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

			duk_എടുക്കുക_n(ctx, 7);
			/* ... copy */
			തുടരുക;
		}
		/* ... copy handler result */
		എങ്കിൽ (duk_is_boolean(ctx, -1) &&
		    duk_to_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_എടുക്കുക_2(ctx);
		/* ... copy */
	}
	duk_എടുക്കുക_2(ctx);
out:
	/* ... */
	dom_കണ്ണി_unref(targ);
	dom_string_unref(name);
}

ശൂന്യം dukky_register_event_listener_for(duk_context *ctx,
				       ഘടന dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = ശൂന്യ;
	dom_exception exc;

	/* ... */
	എങ്കിൽ (ele == ശൂന്യ) {
		/* A null element is the Window object */
		duk_തള്ളുക_global_object(ctx);
	} അല്ലെങ്കിൽ {
		/* Non null elements must be pushed as a node object */
		എങ്കിൽ (dukky_തള്ളുക_കണ്ണി(ctx, (ഘടന dom_കണ്ണി *)ele) == false)
			തിരികെ;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_തള്ളുക_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	എങ്കിൽ (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_എടുക്കുക_2(ctx);
		/* ... */
		തിരികെ;
	}
	/* ... node handlers */
	duk_തള്ളുക_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_തള്ളുക_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_എടുക്കുക_2(ctx);
	/* ... */
	എങ്കിൽ (ele == ശൂന്യ) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		തിരികെ;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	എങ്കിൽ (exc != DOM_NO_ERR) തിരികെ;
	exc = dom_event_target_add_event_listener(
		ele, name, listen, capture);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (പൂർണ്ണസംഖ്യ)dom_string_length(name), dom_string_data(name));
	} അല്ലെങ്കിൽ {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (പൂർണ്ണസംഖ്യ)dom_string_length(name), dom_string_data(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_target_തള്ളുക_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	എങ്കിൽ (duk_is_undefined(ctx, -1)) {
		/* ... type this null */
		duk_എടുക്കുക(ctx);
		duk_തള്ളുക_object(ctx);
		duk_dup(ctx, -1);
		/* ... type this listeners listeners */
		duk_put_prop_string(ctx, -3, EVENT_LISTENER_JS_MAGIC);
		/* ... type this listeners */
	}
	/* ... type this listeners */
	duk_തിരുകുക(ctx, -3);
	/* ... listeners type this */
	duk_എടുക്കുക(ctx);
	/* ... listeners type */
	duk_dup(ctx, -1);
	/* ... listeners type type */
	duk_get_prop(ctx, -3);
	/* ... listeners type ??? */
	എങ്കിൽ (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		എങ്കിൽ (dont_create == true) {
			duk_എടുക്കുക_3(ctx);
			duk_തള്ളുക_undefined(ctx);
			തിരികെ true;
		}
		duk_എടുക്കുക(ctx);
		duk_തള്ളുക_array(ctx);
		duk_dup(ctx, -2);
		duk_dup(ctx, -2);
		/* ... listeners type sublisteners type sublisteners */
		duk_put_prop(ctx, -5);
		/* ... listeners type sublisteners */
		ret = true;
	}
	duk_തിരുകുക(ctx, -3);
	/* ... sublisteners listeners type */
	duk_എടുക്കുക_2(ctx);
	/* ... sublisteners */
	തിരികെ ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
ശൂന്യം dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	ആകുമ്പോൾ (duk_get_prop_index(ctx, -1, idx + 1)) {
		duk_put_prop_index(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_index(ctx, -2, idx + 1);
	duk_എടുക്കുക(ctx);
}


ശൂന്യം js_handle_new_element(jsthread *thread, ഘടന dom_element *കണ്ണി)
{
	പ്രതിജ്ഞാ(thread);
	പ്രതിജ്ഞാ(കണ്ണി);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = ശൂന്യ;
	dom_string *താക്കോൽ = ശൂന്യ;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_കണ്ണി_get_കണ്ണി_name(കണ്ണി, &nodename);
	എങ്കിൽ (exc != DOM_NO_ERR) തിരികെ;

	എങ്കിൽ (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_കണ്ണി_get_attributes(കണ്ണി, &map);
	എങ്കിൽ (exc != DOM_NO_ERR) തിരികെ;
	എങ്കിൽ (map == ശൂന്യ) തിരികെ;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_length(map, &siz);
	എങ്കിൽ (exc != DOM_NO_ERR) പോകുക out;

	വേണ്ടി (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_item(map, idx, &attr);
		എങ്കിൽ (exc != DOM_NO_ERR) പോകുക out;
		exc = dom_attr_get_name(attr, &താക്കോൽ);
		എങ്കിൽ (exc != DOM_NO_ERR) പോകുക out;
		എങ്കിൽ (is_body && (
			    താക്കോൽ == corestring_dom_onblur ||
			    താക്കോൽ == corestring_dom_onerror ||
			    താക്കോൽ == corestring_dom_onfocus ||
			    താക്കോൽ == corestring_dom_onload ||
			    താക്കോൽ == corestring_dom_onresize ||
			    താക്കോൽ == corestring_dom_onscroll)) {
			/* This is a forwarded event, it doesn't matter,
			 * we should skip registering for it and later
			 * we will register it for Window itself
			 */
			പോകുക skip_register;
		}
		എങ്കിൽ (dom_string_length(താക്കോൽ) > 2) {
			/* Can be on* */
			സ്ഥിരം uint8_t *data = (സ്ഥിരം uint8_t *)dom_string_data(താക്കോൽ);
			എങ്കിൽ (data[0] == 'o' && data[1] == 'n') {
				dom_string *sub = ശൂന്യ;
				exc = dom_string_substr(
					താക്കോൽ, 2, dom_string_length(താക്കോൽ),
					&sub);
				എങ്കിൽ (exc == DOM_NO_ERR) {
					dukky_register_event_listener_for(
						CTX, കണ്ണി, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_register:
		dom_string_unref(താക്കോൽ); താക്കോൽ = ശൂന്യ;
		dom_കണ്ണി_unref(attr); attr = ശൂന്യ;
	}

out:
	എങ്കിൽ (താക്കോൽ != ശൂന്യ)
		dom_string_unref(താക്കോൽ);

	എങ്കിൽ (attr != ശൂന്യ)
		dom_കണ്ണി_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

ശൂന്യം js_event_cleanup(jsthread *thread, ഘടന dom_event *evt)
{
	പ്രതിജ്ഞാ(thread);
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, EVENT_MAGIC);
	/* ... EVENT_MAP */
	duk_തള്ളുക_pointer(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_എടുക്കുക(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, സ്ഥിരം അക്ഷരം *type, ഘടന dom_document *doc, ഘടന dom_കണ്ണി *target)
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

	എങ്കിൽ (target != ശൂന്യ)
		/* Swallow non-Window-targetted events quietly */
		തിരികെ true;

	എങ്കിൽ (strcmp(type, "load") != 0)
		/* Swallow non-load events quietly */
		തിരികെ true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	എങ്കിൽ (exc != DOM_NO_ERR) തിരികെ true;
	exc = dom_event_init(evt, corestring_dom_load, false, false);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		തിരികെ true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_തള്ളുക_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	എങ്കിൽ (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_എടുക്കുക(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		എങ്കിൽ (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			തിരികെ true;
		}
		dukky_തള്ളുക_കണ്ണി(CTX, (ഘടന dom_കണ്ണി *)body);
		/* ... handlers bodynode */
		എങ്കിൽ (dukky_get_current_മൂല്യം_of_event_handler(
			    CTX, corestring_dom_load, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_കണ്ണി_unref(body);
			/* ... handlers */
			duk_എടുക്കുക(CTX);
			dukky_leave_thread(thread);
			തിരികെ true;
		}
		/* Unref the body, we don't need it any more */
		dom_കണ്ണി_unref(body);
		/* ... handlers handler bodynode */
		duk_എടുക്കുക(CTX);
	}
	/* ... handlers handler */
	duk_തിരുകുക(CTX, -2);
	/* ... handler handlers */
	duk_എടുക്കുക(CTX);
	/* ... handler */
	duk_തള്ളുക_global_object(CTX);
	/* ... handler Window */
	dukky_തള്ളുക_event(CTX, evt);
	/* ... handler Window event */
	dukky_reset_start_time(CTX);
	എങ്കിൽ (duk_pcall_method(CTX, 1) != 0) {
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

		duk_എടുക്കുക_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		തിരികെ true;
	}
	/* ... result */
	duk_എടുക്കുക(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	തിരികെ true;
}
