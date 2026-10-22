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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nsutils/time.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "javascript/js.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "javascript/content.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "duktape/binding.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "duktape/generics.js.inc"
#ਅਨ੍ਤਰ੍ਭਾਵ "duktape/polyfill.js.inc"

#ਅਨ੍ਤਰ੍ਭਾਵ "duktape.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "dukky.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EVENT_MAGIC MAGIC(EVENT_MAP)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HANDLER_MAGIC MAGIC(HANDLER_MAP)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GENERICS_MAGIC MAGIC(GENERICS_ਸਾਰਣੀ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
ਬਣਤਰ jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t ਅਗਲਾ_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਜੀਵਤ_ਧਾਗੇ; /**< number of live threads */
	uint64_t exec_ਸ਼ੁਰੂ_ਸਮਾਂ;
};

/**
 * dukky javascript thread
 */
ਬਣਤਰ jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਵਿੱਚ_use; /**< The number of times this thread is in use */
	jsheap *ਢੇਰ; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

ਸਥਿਰ duk_ret_t dukky_populate_object(duk_context *ctx, ਖਾਲੀ *udata)
{
	/* ... obj args protoname nargs */
	ਪੂਰਨ nargs = duk_get_int(ctx, -1);
	duk_ਕੱਢੋ(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	ਜੇ (duk_is_undefined(ctx, -1)) {
		NSLOG(dukky, WARNING,
		      "Unable to find dukky prototype for `%s` - falling back to HTMLUnknownElement",
		      duk_get_string(ctx, -3) + 2 /* Skip the two unprintables */);
		duk_ਕੱਢੋ(ctx);
		duk_ਧੱਕੋ_string(ctx, PROTO_NAME(HTMLUNKNOWNELEMENT));
		duk_get_prop(ctx, -2);
	}
	/* ... obj args protoname prototab proto */
	duk_remove(ctx, -3);
	/* ... obj args prototab proto */
	duk_dup(ctx, -1);
	/* ... obj args prototab proto proto */
	duk_ਸੈੱਟ_prototype(ctx, -(nargs+4));
	/* ... obj[proto] args prototab proto */
	duk_get_prop_string(ctx, -1, INIT_MAGIC);
	/* ... obj[proto] args prototab proto initfn */
	duk_ਪਾਓ(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_ਕੱਢੋ_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	ਵਾਪਸ 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ ਦਲੀਲਾਂ)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, ਦਲੀਲਾਂ);
	/* ... args */
	duk_ਧੱਕੋ_object(ctx);
	/* ... args obj */
	duk_ਧੱਕੋ_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
	/* ... args obj */
	duk_ਧੱਕੋ_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
	/* ... args obj */
	duk_ਪਾਓ(ctx, -(ਦਲੀਲਾਂ+1));
	/* ... obj args */
	duk_ਧੱਕੋ_string(ctx, name);
	/* ... obj args name */
	duk_ਧੱਕੋ_int(ctx, ਦਲੀਲਾਂ);
	/* ... obj args name nargs */
	ਜੇ ((ret = duk_ਸੁਰੱਖਿਅਤ_call(ctx, dukky_populate_object, ਨਿਰਾ, ਦਲੀਲਾਂ + 3, 1))
	    != DUK_EXEC_SUCCESS)
		ਵਾਪਸ ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	ਵਾਪਸ DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_ਧੱਕੋ_ਗੰਢ_stacked(duk_context *ctx)
{
	ਪੂਰਨ ਸਿਖਰ_at_fail = duk_get_ਸਿਖਰ(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, ਗੰਢ_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	ਜੇ (duk_is_undefined(ctx, -1)) {
		/* ... nodeptr klass nodes undefined */
		duk_ਕੱਢੋ(ctx);
		/* ... nodeptr klass nodes */
		duk_ਧੱਕੋ_object(ctx);
		/* ... nodeptr klass nodes obj */
		duk_ਧੱਕੋ_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_ਧੱਕੋ_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr klass */
		duk_ਧੱਕੋ_int(ctx, 1);
		/* ... nodeptr klass nodes obj nodeptr klass 1 */
		ਜੇ (duk_ਸੁਰੱਖਿਅਤ_call(ctx, dukky_populate_object, ਨਿਰਾ, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_ਸੈੱਟ_ਸਿਖਰ(ctx, ਸਿਖਰ_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			ਵਾਪਸ false;
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
	duk_ਪਾਓ(ctx, -4);
	/* ... node nodeptr klass nodes */
	duk_ਕੱਢੋ_3(ctx);
	/* ... node */
	ਜੇ (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		ਅਚਲ ਅੱਖਰ * what = duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_ਕੱਢੋ(ctx);
	}
	ਵਾਪਸ true;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

ਸਥਿਰ ਖਾਲੀ dukky_html_element_class_from_ਟੈਗ_type(dom_html_element_type type,
		ਅਚਲ ਅੱਖਰ **html_class, size_t *html_class_len)
{
	ਚੋਣ(type) {
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_HTML:
		ਸੈੱਟ_HTML_CLASS(HTML)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ਸਿਰ:
		ਸੈੱਟ_HTML_CLASS(ਸਿਰ)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_META:
		ਸੈੱਟ_HTML_CLASS(META)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ਅਧਾਰ:
		ਸੈੱਟ_HTML_CLASS(ਅਧਾਰ_2)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TITLE:
		ਸੈੱਟ_HTML_CLASS(TITLE)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_BODY:
		ਸੈੱਟ_HTML_CLASS(BODY)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_DIV:
		ਸੈੱਟ_HTML_CLASS(DIV)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_FORM:
		ਸੈੱਟ_HTML_CLASS(FORM)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_LINK:
		ਸੈੱਟ_HTML_CLASS(LINK)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_BUTTON:
		ਸੈੱਟ_HTML_CLASS(BUTTON)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ਨਿਵੇਸ਼:
		ਸੈੱਟ_HTML_CLASS(ਨਿਵੇਸ਼_2)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		ਸੈੱਟ_HTML_CLASS(TEXTAREA)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		ਸੈੱਟ_HTML_CLASS(OPTGROUP)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_OPTION:
		ਸੈੱਟ_HTML_CLASS(OPTION)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_SELECT:
		ਸੈੱਟ_HTML_CLASS(SELECT)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_HR:
		ਸੈੱਟ_HTML_CLASS(HR)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_DL:
		ਸੈੱਟ_HTML_CLASS(DLIST)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_DIR:
		ਸੈੱਟ_HTML_CLASS(DIRECTORY)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_MENU:
		ਸੈੱਟ_HTML_CLASS(MENU)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_FIELDSET:
		ਸੈੱਟ_HTML_CLASS(FIELDSET)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_LEGEND:
		ਸੈੱਟ_HTML_CLASS(LEGEND)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_P:
		ਸੈੱਟ_HTML_CLASS(PARAGRAPH)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_H1:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_H2:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_H3:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_H4:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_H5:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_H6:
		ਸੈੱਟ_HTML_CLASS(HEADING)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_Q:
		ਸੈੱਟ_HTML_CLASS(QUOTE)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_PRE:
		ਸੈੱਟ_HTML_CLASS(PRE)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_BR:
		ਸੈੱਟ_HTML_CLASS(BR)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_LABEL:
		ਸੈੱਟ_HTML_CLASS(LABEL)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_UL:
		ਸੈੱਟ_HTML_CLASS(ULIST)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_OL:
		ਸੈੱਟ_HTML_CLASS(OLIST)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_LI:
		ਸੈੱਟ_HTML_CLASS(LI)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_FONT:
		ਸੈੱਟ_HTML_CLASS(FONT)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_DEL:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_INS:
		ਸੈੱਟ_HTML_CLASS(MOD)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_A:
		ਸੈੱਟ_HTML_CLASS(ANCHOR)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_BASEFONT:
		ਸੈੱਟ_HTML_CLASS(BASEFONT)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_IMG:
		ਸੈੱਟ_HTML_CLASS(IMAGE)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_OBJECT:
		ਸੈੱਟ_HTML_CLASS(OBJECT)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_PARAM:
		ਸੈੱਟ_HTML_CLASS(PARAM)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_APPLET:
		ਸੈੱਟ_HTML_CLASS(APPLET)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_MAP:
		ਸੈੱਟ_HTML_CLASS(MAP)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_AREA:
		ਸੈੱਟ_HTML_CLASS(AREA)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_SCRIPT:
		ਸੈੱਟ_HTML_CLASS(SCRIPT)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_CAPTION:
		ਸੈੱਟ_HTML_CLASS(TABLECAPTION)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TD:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TH:
		ਸੈੱਟ_HTML_CLASS(TABLECELL)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ਖਾਨਾ:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_COLGROUP:
		ਸੈੱਟ_HTML_CLASS(TABLECOL)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_THEAD:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TBODY:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TFOOT:
		ਸੈੱਟ_HTML_CLASS(TABLESECTION)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ਸਾਰਣੀ:
		ਸੈੱਟ_HTML_CLASS(ਸਾਰਣੀ_2)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TR:
		ਸੈੱਟ_HTML_CLASS(TABLEROW)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_STYLE:
		ਸੈੱਟ_HTML_CLASS(STYLE)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_FRAMESET:
		ਸੈੱਟ_HTML_CLASS(FRAMESET)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ਚੌਖਟਾ:
		ਸੈੱਟ_HTML_CLASS(ਚੌਖਟਾ_2)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_IFRAME:
		ਸੈੱਟ_HTML_CLASS(IFRAME)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ISINDEX:
		ਸੈੱਟ_HTML_CLASS(ISINDEX)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_CANVAS:
		ਸੈੱਟ_HTML_CLASS(CANVAS)
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ਗਿਣਤੀ:
		ਪ੍ਰਤਿਜ੍ਞਾ(type != DOM_HTML_ELEMENT_TYPE_ਗਿਣਤੀ);
		fallthrough;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		ਸੈੱਟ_HTML_CLASS(UNKNOWN)
		ਰੋਕੋ;
	ਮੂਲ:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		ਰੋਕੋ;
	}
	ਵਾਪਸ;
}

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਸੈੱਟ_HTML_CLASS

ਸਥਿਰ ਖਾਲੀ
dukky_ਧੱਕੋ_ਗੰਢ_klass(duk_context *ctx, ਬਣਤਰ dom_ਗੰਢ *ਗੰਢ)
{
	dom_ਗੰਢ_type nodetype;
	dom_exception err;

	err = dom_ਗੰਢ_get_ਗੰਢ_type(ਗੰਢ, &nodetype);
	ਜੇ (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_ਧੱਕੋ_string(ctx, PROTO_NAME(ਗੰਢ_2));
		ਵਾਪਸ;
	}

	ਚੋਣ(nodetype) {
	ਹਾਲਤ DOM_ELEMENT_ਗੰਢ: {
		dom_string *namespace;
		dom_html_element_type type;
		ਅਚਲ ਅੱਖਰ *html_class;
		size_t html_class_len;
		err = dom_ਗੰਢ_get_namespace(ਗੰਢ, &namespace);
		ਜੇ (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_ਧੱਕੋ_string(ctx, PROTO_NAME(ELEMENT));
			ਰੋਕੋ;
		}
		ਜੇ (namespace == ਨਿਰਾ) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_ਧੱਕੋ_string(ctx, PROTO_NAME(ELEMENT));
			ਰੋਕੋ;
		}

		ਜੇ (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_ਧੱਕੋ_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			ਰੋਕੋ;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_ਟੈਗ_type(ਗੰਢ, &type);
		ਜੇ (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_ਟੈਗ_type(type,
				&html_class, &html_class_len);

		duk_ਧੱਕੋ_lstring(ctx, html_class, html_class_len);
		ਰੋਕੋ;
	}
	ਹਾਲਤ DOM_ਪਾਠ_ਗੰਢ:
		duk_ਧੱਕੋ_string(ctx, PROTO_NAME(ਪਾਠ_2));
		ਰੋਕੋ;
	ਹਾਲਤ DOM_COMMENT_ਗੰਢ:
		duk_ਧੱਕੋ_string(ctx, PROTO_NAME(COMMENT));
		ਰੋਕੋ;
	ਹਾਲਤ DOM_DOCUMENT_ਗੰਢ:
		duk_ਧੱਕੋ_string(ctx, PROTO_NAME(DOCUMENT));
		ਰੋਕੋ;
	ਹਾਲਤ DOM_ATTRIBUTE_ਗੰਢ:
	ਹਾਲਤ DOM_PROCESSING_INSTRUCTION_ਗੰਢ:
	ਹਾਲਤ DOM_DOCUMENT_TYPE_ਗੰਢ:
	ਹਾਲਤ DOM_DOCUMENT_FRAGMENT_ਗੰਢ:
	ਹਾਲਤ DOM_NOTATION_ਗੰਢ:
	ਹਾਲਤ DOM_ENTITY_REFERENCE_ਗੰਢ:
	ਹਾਲਤ DOM_ENTITY_ਗੰਢ:
	ਹਾਲਤ DOM_CDATA_SECTION_ਗੰਢ:
	ਮੂਲ:
		/* Oh bum, just node then */
		duk_ਧੱਕੋ_string(ctx, PROTO_NAME(ਗੰਢ_2));
	}
}

duk_bool_t
dukky_ਧੱਕੋ_ਗੰਢ(duk_context *ctx, ਬਣਤਰ dom_ਗੰਢ *ਗੰਢ)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", ਗੰਢ);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, ਗੰਢ_MAGIC);
	/* ... nodes */
	duk_ਧੱਕੋ_ਸੂਚਕ(ctx, ਗੰਢ);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	ਜੇ (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_ਪਾਓ(ctx, -2);
		/* ... node nodes */
		duk_ਕੱਢੋ(ctx);
		/* ... node */
		ਜੇ (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			ਅਚਲ ਅੱਖਰ * what = duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_ਕੱਢੋ(ctx);
		}
		ਵਾਪਸ true;
	}
	/* ... nodes undefined */
	duk_ਕੱਢੋ_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_ਧੱਕੋ_ਸੂਚਕ(ctx, ਗੰਢ);
	/* ... nodeptr */
	dukky_ਧੱਕੋ_ਗੰਢ_klass(ctx, ਗੰਢ);
	/* ... nodeptr klass */
	ਵਾਪਸ dukky_ਧੱਕੋ_ਗੰਢ_stacked(ctx);
}

ਸਥਿਰ duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	ਵਾਪਸ duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

ਖਾਲੀ
dukky_inject_not_ctr(duk_context *ctx, ਪੂਰਨ idx, ਅਚਲ ਅੱਖਰ *name)
{
	/* ... p[idx] ... proto */
	duk_ਧੱਕੋ_c_function(ctx, dukky_bad_constructor, 0);
	/* ... p[idx] ... proto cons */
	duk_ਪਾਓ(ctx, -2);
	/* ... p[idx] ... cons proto */
	duk_put_prop_string(ctx, -2, "prototype");
	/* ... p[idx] ... cons[proto] */
	duk_put_prop_string(ctx, idx, name);
	/* ... p ... */
	ਵਾਪਸ;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

ਸਥਿਰ ਖਾਲੀ *dukky_alloc_function(ਖਾਲੀ *udata, duk_ਮਾਪ_t ਮਾਪ)
{
	ਜੇ (ਮਾਪ == 0)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ);
}

ਸਥਿਰ ਖਾਲੀ *dukky_realloc_function(ਖਾਲੀ *udata, ਖਾਲੀ *ptr, duk_ਮਾਪ_t ਮਾਪ)
{
	ਜੇ (ptr == ਨਿਰਾ && ਮਾਪ == 0)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (ਮਾਪ == 0) {
		ਮੁਕਤ(ptr);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ realloc(ptr, ਮਾਪ);
}


ਸਥਿਰ ਖਾਲੀ dukky_free_function(ਖਾਲੀ *udata, ਖਾਲੀ *ptr)
{
	ਜੇ (ptr != ਨਿਰਾ)
		ਮੁਕਤ(ptr);
}

/* exported interface documented in js.h */
ਖਾਲੀ js_initialise(ਖਾਲੀ)
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
ਖਾਲੀ js_finalise(ਖਾਲੀ)
{
	/* NADA for now */
}


/* exported interface documented in js.h */
nserror
js_newheap(ਪੂਰਨ timeout, jsheap **ਢੇਰ)
{
	duk_context *ctx;
	jsheap *ret = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*ret));
	*ਢੇਰ = ਨਿਰਾ;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	ਜੇ (ret == ਨਿਰਾ) ਵਾਪਸ NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_ਢੇਰ(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		ਨਿਰਾ);
	ਜੇ (ret->ctx == ਨਿਰਾ) { ਮੁਕਤ(ret); ਵਾਪਸ NSERROR_NOMEM; }
	/* Create the prototype stuffs */
	duk_ਧੱਕੋ_global_object(ctx);
	duk_ਧੱਕੋ_boolean(ctx, true);
	duk_put_prop_string(ctx, -2, "protos");
	duk_put_global_string(ctx, PROTO_MAGIC);
	/* Create prototypes here */
	dukky_create_prototypes(ctx);
	/* Now create the thread map */
	duk_ਧੱਕੋ_object(ctx);
	duk_put_global_string(ctx, THREAD_MAP);

	*ਢੇਰ = ret;
	ਵਾਪਸ NSERROR_OK;
}


ਸਥਿਰ ਖਾਲੀ dukky_destroyheap(jsheap *ਢੇਰ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ->pending_destroy == true);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ->ਜੀਵਤ_ਧਾਗੇ == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_ਢੇਰ(ਢੇਰ->ctx);
	ਮੁਕਤ(ਢੇਰ);
}

/* exported interface documented in js.h */
ਖਾਲੀ js_destroyheap(jsheap *ਢੇਰ)
{
	ਢੇਰ->pending_destroy = true;
	ਜੇ (ਢੇਰ->ਜੀਵਤ_ਧਾਗੇ == 0) {
		dukky_destroyheap(ਢੇਰ);
	}
}

/* Just for here, the CTX is in ret, not thread */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *ਢੇਰ, ਖਾਲੀ *win_priv, ਖਾਲੀ *doc_priv, jsthread **thread)
{
	jsthread *ret;
	ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਢੇਰ->pending_destroy == false);

	ret = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ (*ret));
	ਜੇ (ret == ਨਿਰਾ) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		ਵਾਪਸ NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(ਢੇਰ->ctx, THREAD_MAP); /* ... threads */
	duk_ਧੱਕੋ_thread(ਢੇਰ->ctx); /* ... threads thread */
	ret->ਢੇਰ = ਢੇਰ;
	ret->ctx = duk_require_context(ਢੇਰ->ctx, -1);
	ret->thread_idx = ਢੇਰ->ਅਗਲਾ_thread++;
	duk_put_prop_ਸੂਚਕਾਂਕ(ਢੇਰ->ctx, -2, ret->thread_idx);
	ਢੇਰ->ਜੀਵਤ_ਧਾਗੇ++;
	duk_ਕੱਢੋ(ਢੇਰ->ctx); /* ... */
	duk_ਧੱਕੋ_int(CTX, 0);
	duk_ਧੱਕੋ_int(CTX, 1);
	duk_ਧੱਕੋ_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_ਧੱਕੋ_ਸੂਚਕ(CTX, win_priv);
	duk_ਧੱਕੋ_ਸੂਚਕ(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(ਖਿੜਕੀ), 2);
	duk_ਧੱਕੋ_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_ਸੈੱਟ_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_ਧੱਕੋ_object(CTX);
	duk_ਧੱਕੋ_ਸੂਚਕ(CTX, ਨਿਰਾ);
	duk_ਧੱਕੋ_null(CTX);
	duk_put_prop(CTX, -3);
	duk_put_global_string(CTX, ਗੰਢ_MAGIC);

	/* And now the event mapping table */
	duk_ਧੱਕੋ_object(CTX);
	duk_put_global_string(CTX, EVENT_MAGIC);

	/* Now load the polyfills */
	/* ... */
	duk_ਧੱਕੋ_string(CTX, "polyfill.js");
	/* ..., polyfill.js */
	ਜੇ (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ਅਚਲ ਅੱਖਰ *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	ਜੇ (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_ਕੱਢੋ(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_ਧੱਕੋ_string(CTX, "generics.js");
	/* ..., generics.js */
	ਜੇ (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ਅਚਲ ਅੱਖਰ *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	ਜੇ (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_ਕੱਢੋ(CTX);
	/* ... */
	duk_ਧੱਕੋ_global_object(CTX);
	/* ..., Win */
	duk_get_prop_string(CTX, -1, "NetSurf");
	/* ..., Win, NetSurf */
	duk_put_global_string(CTX, GENERICS_MAGIC);
	/* ..., Win */
	duk_del_prop_string(CTX, -1, "NetSurf");
	duk_ਕੱਢੋ(CTX);
	/* ... */

	dukky_log_ਢੇਰ_ਚੌਖਟਾ(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, ਢੇਰ);
	*thread = ret;

	ਵਾਪਸ NSERROR_OK;
}

/* Now switch to the long term CTX behaviour */
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ CTX
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CTX (thread->ctx)

/* exported interface documented in js.h */
nserror js_closethread(jsthread *thread)
{
	/* We can always close down a thread, it might just confuse
	 * the code running, though we don't mind since we're in the
	 * process of destruction at this point
	 */
	duk_int_t ਸਿਖਰ = duk_get_ਸਿਖਰ(CTX);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, thread->ਢੇਰ);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Restore whatever stack we had */
	duk_ਸੈੱਟ_ਸਿਖਰ(CTX, ਸਿਖਰ);

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
ਸਥਿਰ ਖਾਲੀ dukky_destroythread(jsthread *thread)
{
	jsheap *ਢੇਰ = thread->ਢੇਰ;

	ਪ੍ਰਤਿਜ੍ਞਾ(thread->ਵਿੱਚ_use == 0);
	ਪ੍ਰਤਿਜ੍ਞਾ(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, ਢੇਰ);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(ਢੇਰ->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_ਸੂਚਕਾਂਕ(ਢੇਰ->ctx, -1, thread->thread_idx);
	duk_ਕੱਢੋ(ਢੇਰ->ctx); /* ... */

	/* We can now free the thread object */
	ਮੁਕਤ(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(ਢੇਰ->ctx, 0);
	duk_gc(ਢੇਰ->ctx, DUK_GC_COMPACT);
	ਢੇਰ->ਜੀਵਤ_ਧਾਗੇ--;

	/* And if the heap should now go, blow it away */
	ਜੇ (ਢੇਰ->pending_destroy == true && ਢੇਰ->ਜੀਵਤ_ਧਾਗੇ == 0) {
		dukky_destroyheap(ਢੇਰ);
	}
}

/* exported interface documented in js.h */
ਖਾਲੀ js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	ਜੇ (thread->ਵਿੱਚ_use == 0) {
		dukky_destroythread(thread);
	}
}

ਸਥਿਰ ਖਾਲੀ dukky_enter_thread(jsthread *thread)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(thread != ਨਿਰਾ);
	thread->ਵਿੱਚ_use++;
}

ਸਥਿਰ ਖਾਲੀ dukky_leave_thread(jsthread *thread)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(thread != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(thread->ਵਿੱਚ_use > 0);

	thread->ਵਿੱਚ_use--;
	ਜੇ (thread->ਵਿੱਚ_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(ਖਾਲੀ *udata)
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *ਢੇਰ = (jsheap *) udata;
	uint64_t now;

	(ਖਾਲੀ) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	ਵਾਪਸ ਢੇਰ->exec_ਸ਼ੁਰੂ_ਸਮਾਂ != 0 &&
			now > (ਢੇਰ->exec_ਸ਼ੁਰੂ_ਸਮਾਂ + JS_EXEC_TIMEOUT_MS);
}

ਸਥਿਰ ਖਾਲੀ dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_ਸਿਖਰ(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_ਕੱਢੋ(ctx);
	/* ..., errobj */
}

ਸਥਿਰ ਖਾਲੀ dukky_reset_ਸ਼ੁਰੂ_ਸਮਾਂ(duk_context *ctx)
{
	duk_ਸਮ੍ਰਿਤੀ_functions funcs;
	jsheap *ਢੇਰ;
	duk_get_ਸਮ੍ਰਿਤੀ_functions(ctx, &funcs);
	ਢੇਰ = funcs.udata;
	(ਖਾਲੀ) nsu_getmonotonic_ms(&ਢੇਰ->exec_ਸ਼ੁਰੂ_ਸਮਾਂ);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_ਮਾਪ_t argc, bool reset_timeout)
{
	ਜੇ (reset_timeout) {
		dukky_reset_ਸ਼ੁਰੂ_ਸਮਾਂ(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	ਜੇ (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	ਵਾਪਸ ret;
}


ਖਾਲੀ dukky_ਧੱਕੋ_generics(duk_context *ctx, ਅਚਲ ਅੱਖਰ *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

ਸਥਿਰ duk_int_t dukky_ਧੱਕੋ_context_dump(duk_context *ctx, ਖਾਲੀ *udata)
{
	duk_ਧੱਕੋ_context_dump(ctx);
	ਵਾਪਸ 1;
}

ਖਾਲੀ dukky_log_ਢੇਰ_ਚੌਖਟਾ(duk_context *ctx, ਅਚਲ ਅੱਖਰ * reason)
{
	ਜੇ (duk_ਸੁਰੱਖਿਅਤ_call(ctx, dukky_ਧੱਕੋ_context_dump, ਨਿਰਾ, 0, 1) != 0) {
		duk_ਕੱਢੋ(ctx);
		duk_ਧੱਕੋ_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(ctx, -1));
	duk_ਕੱਢੋ(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, ਅਚਲ uint8_t *txt, size_t txtlen, ਅਚਲ ਅੱਖਰ *name)
{
	bool ret = false;
	ਪ੍ਰਤਿਜ੍ਞਾ(thread);

	ਜੇ (txt == ਨਿਰਾ || txtlen == 0) {
		ਵਾਪਸ false;
	}

	ਜੇ (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		ਵਾਪਸ false;
	}

	dukky_enter_thread(thread);

	duk_ਸੈੱਟ_ਸਿਖਰ(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_ਸ਼ੁਰੂ_ਸਮਾਂ(CTX);
	ਜੇ (name != ਨਿਰਾ) {
		duk_ਧੱਕੋ_string(CTX, name);
	} ਨਹੀਂਤਾਂ {
		duk_ਧੱਕੋ_string(CTX, "?unknown source?");
	}
	ਜੇ (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (ਅਚਲ ਅੱਖਰ *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		ਜਾਓ handle_error;
	}

	ਜੇ (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		ਜਾਓ handle_error;
	}

	ਜੇ (duk_get_ਸਿਖਰ(CTX) == 0) duk_ਧੱਕੋ_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	ਜਾਓ ਬਾਹਰਲਾ;

handle_error:
	dukky_dump_error(CTX);
ਬਾਹਰਲਾ:
	dukky_leave_thread(thread);
	ਵਾਪਸ ret;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ* dukky_event_proto(dom_event *evt)
{
	ਅਚਲ ਅੱਖਰ *ret = PROTO_NAME(EVENT);
	dom_string *type = ਨਿਰਾ;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	ਜੇ (err != DOM_NO_ERR) {
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		ਜਾਓ ਬਾਹਰਲਾ;
	} ਨਹੀਂਤਾਂ ਜੇ (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		ਜਾਓ ਬਾਹਰਲਾ;
	} ਨਹੀਂਤਾਂ ਜੇ (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		ਜਾਓ ਬਾਹਰਲਾ;
	}

ਬਾਹਰਲਾ:
	ਜੇ (type != ਨਿਰਾ) {
		dom_string_unref(type);
	}

	ਵਾਪਸ ret;
}

/*** New style event handling ***/

ਖਾਲੀ dukky_ਧੱਕੋ_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_ਧੱਕੋ_ਸੂਚਕ(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	ਜੇ (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_ਕੱਢੋ(ctx);
		/* ... events */
		duk_ਧੱਕੋ_ਸੂਚਕ(ctx, evt);
		ਜੇ (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_ਕੱਢੋ(ctx);
			/* ... events */
			duk_ਧੱਕੋ_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_ਧੱਕੋ_ਸੂਚਕ(ctx, evt);
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

ਸਥਿਰ ਖਾਲੀ dukky_ਧੱਕੋ_handler_ਕੋਡ(duk_context *ctx, dom_string *name,
				     dom_event_ਨਿਸ਼ਾਨਾ *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_ਗੰਢ_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	ਜੇ (et == ਨਿਰਾ) {
		duk_ਧੱਕੋ_lstring(ctx, "", 0);
		ਵਾਪਸ;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_ਗੰਢ_get_ਗੰਢ_type(et, &ntype);
	ਜੇ (exc != DOM_NO_ERR) {
		duk_ਧੱਕੋ_lstring(ctx, "", 0);
		ਵਾਪਸ;
	}

	ਜੇ (ntype != DOM_ELEMENT_ਗੰਢ) {
		duk_ਧੱਕੋ_lstring(ctx, "", 0);
		ਵਾਪਸ;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	ਜੇ (exc != DOM_NO_ERR) {
		duk_ਧੱਕੋ_lstring(ctx, "", 0);
		ਵਾਪਸ;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	ਜੇ ((exc != DOM_NO_ERR) || (val == ਨਿਰਾ)) {
		dom_string_unref(onname);
		duk_ਧੱਕੋ_lstring(ctx, "", 0);
		ਵਾਪਸ;
	}

	dom_string_unref(onname);
	duk_ਧੱਕੋ_lstring(ctx, dom_string_ਅੰਕੜੇ(val), dom_string_ਲੰਮਾਈ(val));
	dom_string_unref(val);
}

bool dukky_get_current_ਮੁੱਲ_ਦਾ_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_ਨਿਸ਼ਾਨਾ *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_ਧੱਕੋ_lstring(ctx, dom_string_ਅੰਕੜੇ(name), dom_string_ਲੰਮਾਈ(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	ਜੇ (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_ਕੱਢੋ_2(ctx);
		/* ... node */
		dukky_ਧੱਕੋ_handler_ਕੋਡ(ctx, name, et);
		/* ... node handlercode? */
		/* TODO: If this is null, clean up and propagate */
		/* ... node handlercode */
		/** @todo This is entirely wrong, but it's hard to get right */
		duk_ਧੱਕੋ_string(ctx, "function (event) {");
		/* ... node handlercode prefix */
		duk_ਪਾਓ(ctx, -2);
		/* ... node prefix handlercode */
		duk_ਧੱਕੋ_string(ctx, "}");
		/* ... node prefix handlercode suffix */
		duk_concat(ctx, 3);
		/* ... node fullhandlersrc */
		duk_ਧੱਕੋ_string(ctx, "internal raw uncompiled handler");
		/* ... node fullhandlersrc filename */
		ਜੇ (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_ਕੱਢੋ_2(ctx);
			ਵਾਪਸ false;
		}
		/* ... node handler */
		duk_ਪਾਓ(ctx, -2);
		/* ... handler node */
	} ਨਹੀਂਤਾਂ {
		/* ... node handlers handler */
		duk_ਪਾਓ(ctx, -3);
		/* ... handler node handlers */
		duk_ਕੱਢੋ(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	ਵਾਪਸ true;
}

ਸਥਿਰ ਖਾਲੀ dukky_generic_event_handler(dom_event *evt, ਖਾਲੀ *pw)
{
	duk_context *ctx = (duk_context *)pw;
	dom_string *name;
	dom_exception exc;
	dom_event_ਨਿਸ਼ਾਨਾ *targ;
	dom_event_flow_phase phase;
	duk_uarridx_t idx;
	event_listener_flags flags;

	NSLOG(dukky, DEBUG, "Handling an event in duktape interface...");
	exc = dom_event_get_type(evt, &name);
	ਜੇ (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		ਵਾਪਸ;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (ਪੂਰਨ)dom_string_ਲੰਮਾਈ(name),
	      dom_string_ਅੰਕੜੇ(name));
	exc = dom_event_get_event_phase(evt, &phase);
	ਜੇ (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		ਵਾਪਸ;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_ਨਿਸ਼ਾਨਾ ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (ਪੂਰਨ)phase);

	exc = dom_event_get_current_ਨਿਸ਼ਾਨਾ(evt, &targ);
	ਜੇ (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		ਵਾਪਸ;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	ਜੇ (phase == DOM_CAPTURING_PHASE)
		ਜਾਓ handle_extras;

	/* ... */
	ਜੇ (dukky_ਧੱਕੋ_ਗੰਢ(ctx, (dom_ਗੰਢ *)targ) == false) {
		dom_string_unref(name);
		dom_ਗੰਢ_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		ਵਾਪਸ;
	}
	/* ... node */
	ਜੇ (dukky_get_current_ਮੁੱਲ_ਦਾ_event_handler(
		    ctx, name, (dom_event_ਨਿਸ਼ਾਨਾ *)targ) == false) {
		/* ... */
		ਜਾਓ handle_extras;
	}
	/* ... handler node */
	dukky_ਧੱਕੋ_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_ਸ਼ੁਰੂ_ਸਮਾਂ(ctx);
	ਜੇ (duk_pcall_ਵਿਧੀ(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_ਤੁਰੰਤ_ਪ੍ਰਸਾਰ(evt);
		ਜੇ (exc != DOM_NO_ERR)
			NSLOG(dukky, DEBUG,
			      "WORSE! could not stop propagation");
		duk_get_prop_string(ctx, -1, "name");
		duk_get_prop_string(ctx, -2, "message");
		duk_get_prop_string(ctx, -3, "fileName");
		duk_get_prop_string(ctx, -4, "lineNumber");
		duk_get_prop_string(ctx, -5, "stack");
		/* ... err name message fileName lineNumber stack */
		NSLOG(dukky, DEBUG, "Uncaught error in JS: %s: %s",
		      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(ctx, -5),
		      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(ctx, -4));
		NSLOG(dukky, INFO, "              was at: %s line %s",
		      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(ctx, -3),
		      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(ctx, -2));
		NSLOG(dukky, INFO, "         Stack trace: %s",
		      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(ctx, -1));

		duk_ਕੱਢੋ_n(ctx, 6);
		/* ... */
		ਜਾਓ handle_extras;
	}
	/* ... result */
	ਜੇ (duk_is_boolean(ctx, -1) &&
	    duk_ਮੰਜ਼ਿਲ_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_ਕੱਢੋ(ctx);
handle_extras:
	/* ... */
	duk_ਧੱਕੋ_lstring(ctx, dom_string_ਅੰਕੜੇ(name), dom_string_ਲੰਮਾਈ(name));
	dukky_ਧੱਕੋ_ਗੰਢ(ctx, (dom_ਗੰਢ *)targ);
	/* ... type node */
	ਜੇ (dukky_event_ਨਿਸ਼ਾਨਾ_ਧੱਕੋ_listeners(ctx, true)) {
		/* Nothing to do */
		duk_ਕੱਢੋ(ctx);
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	/* ... sublisteners */
	duk_ਧੱਕੋ_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	ਜਦੋਂ (duk_get_prop_ਸੂਚਕਾਂਕ(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_ਸੂਚਕਾਂਕ(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		ਜੇ ((event_listener_flags)duk_ਮੰਜ਼ਿਲ_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_ਕੱਢੋ(ctx);
			/* ... subl copy handler flags */
		}
		duk_ਕੱਢੋ(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_ਸੂਚਕਾਂਕ(ctx, -2, idx);
		/* ... sublisteners copy */
		idx++;
	}
	/* ... sublisteners copy undefined */
	duk_ਕੱਢੋ(ctx);
	/* ... sublisteners copy */
	duk_ਪਾਓ(ctx, -2);
	/* ... copy sublisteners */
	duk_ਕੱਢੋ(ctx);
	/* ... copy */
	idx = 0;
	ਜਦੋਂ (duk_get_prop_ਸੂਚਕਾਂਕ(ctx, -1, idx++)) {
		/* ... copy handler */
		ਜੇ (duk_get_prop_ਸੂਚਕਾਂਕ(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_ਕੱਢੋ_2(ctx);
			ਜਾਰੀ;
		}
		duk_ਕੱਢੋ(ctx);
		duk_get_prop_ਸੂਚਕਾਂਕ(ctx, -1, 0);
		duk_get_prop_ਸੂਚਕਾਂਕ(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_ਕੱਢੋ(ctx);
		/* ... copy handler callback */
		ਜੇ (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_ਕੱਢੋ_2(ctx);
			/* ... copy */
			ਜਾਰੀ;
		}
		/* ... copy handler callback */
		dukky_ਧੱਕੋ_ਗੰਢ(ctx, (dom_ਗੰਢ *)targ);
		/* ... copy handler callback node */
		dukky_ਧੱਕੋ_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_ਸ਼ੁਰੂ_ਸਮਾਂ(ctx);
		ਜੇ (duk_pcall_ਵਿਧੀ(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_ਤੁਰੰਤ_ਪ੍ਰਸਾਰ(evt);
			ਜੇ (exc != DOM_NO_ERR)
				NSLOG(dukky, DEBUG,
				      "WORSE! could not stop propagation");
			duk_get_prop_string(ctx, -1, "name");
			duk_get_prop_string(ctx, -2, "message");
			duk_get_prop_string(ctx, -3, "fileName");
			duk_get_prop_string(ctx, -4, "lineNumber");
			duk_get_prop_string(ctx, -5, "stack");
			/* ... err name message fileName lineNumber stack */
			NSLOG(dukky, DEBUG, "Uncaught error in JS: %s: %s",
			      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(ctx, -5),
			      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(ctx, -4));
			NSLOG(dukky, DEBUG,
			      "              was at: %s line %s",
			      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(ctx, -3),
			      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(ctx, -2));
			NSLOG(dukky, DEBUG, "         Stack trace: %s",
			      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(ctx, -1));

			duk_ਕੱਢੋ_n(ctx, 7);
			/* ... copy */
			ਜਾਰੀ;
		}
		/* ... copy handler result */
		ਜੇ (duk_is_boolean(ctx, -1) &&
		    duk_ਮੰਜ਼ਿਲ_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_ਕੱਢੋ_2(ctx);
		/* ... copy */
	}
	duk_ਕੱਢੋ_2(ctx);
ਬਾਹਰਲਾ:
	/* ... */
	dom_ਗੰਢ_unref(targ);
	dom_string_unref(name);
}

ਖਾਲੀ dukky_ਰਜਿਸਟਰ_event_listener_for(duk_context *ctx,
				       ਬਣਤਰ dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = ਨਿਰਾ;
	dom_exception exc;

	/* ... */
	ਜੇ (ele == ਨਿਰਾ) {
		/* A null element is the Window object */
		duk_ਧੱਕੋ_global_object(ctx);
	} ਨਹੀਂਤਾਂ {
		/* Non null elements must be pushed as a node object */
		ਜੇ (dukky_ਧੱਕੋ_ਗੰਢ(ctx, (ਬਣਤਰ dom_ਗੰਢ *)ele) == false)
			ਵਾਪਸ;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_ਧੱਕੋ_lstring(ctx, dom_string_ਅੰਕੜੇ(name), dom_string_ਲੰਮਾਈ(name));
	/* ... node handlers name */
	ਜੇ (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_ਕੱਢੋ_2(ctx);
		/* ... */
		ਵਾਪਸ;
	}
	/* ... node handlers */
	duk_ਧੱਕੋ_lstring(ctx, dom_string_ਅੰਕੜੇ(name), dom_string_ਲੰਮਾਈ(name));
	/* ... node handlers name */
	duk_ਧੱਕੋ_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_ਕੱਢੋ_2(ctx);
	/* ... */
	ਜੇ (ele == ਨਿਰਾ) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		ਵਾਪਸ;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	ਜੇ (exc != DOM_NO_ERR) ਵਾਪਸ;
	exc = dom_event_ਨਿਸ਼ਾਨਾ_ਜੋੜੋ_event_listener(
		ele, name, listen, capture);
	ਜੇ (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (ਪੂਰਨ)dom_string_ਲੰਮਾਈ(name), dom_string_ਅੰਕੜੇ(name));
	} ਨਹੀਂਤਾਂ {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (ਪੂਰਨ)dom_string_ਲੰਮਾਈ(name), dom_string_ਅੰਕੜੇ(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_ਨਿਸ਼ਾਨਾ_ਧੱਕੋ_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	ਜੇ (duk_is_undefined(ctx, -1)) {
		/* ... type this null */
		duk_ਕੱਢੋ(ctx);
		duk_ਧੱਕੋ_object(ctx);
		duk_dup(ctx, -1);
		/* ... type this listeners listeners */
		duk_put_prop_string(ctx, -3, EVENT_LISTENER_JS_MAGIC);
		/* ... type this listeners */
	}
	/* ... type this listeners */
	duk_ਪਾਓ(ctx, -3);
	/* ... listeners type this */
	duk_ਕੱਢੋ(ctx);
	/* ... listeners type */
	duk_dup(ctx, -1);
	/* ... listeners type type */
	duk_get_prop(ctx, -3);
	/* ... listeners type ??? */
	ਜੇ (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		ਜੇ (dont_create == true) {
			duk_ਕੱਢੋ_3(ctx);
			duk_ਧੱਕੋ_undefined(ctx);
			ਵਾਪਸ true;
		}
		duk_ਕੱਢੋ(ctx);
		duk_ਧੱਕੋ_array(ctx);
		duk_dup(ctx, -2);
		duk_dup(ctx, -2);
		/* ... listeners type sublisteners type sublisteners */
		duk_put_prop(ctx, -5);
		/* ... listeners type sublisteners */
		ret = true;
	}
	duk_ਪਾਓ(ctx, -3);
	/* ... sublisteners listeners type */
	duk_ਕੱਢੋ_2(ctx);
	/* ... sublisteners */
	ਵਾਪਸ ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
ਖਾਲੀ dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	ਜਦੋਂ (duk_get_prop_ਸੂਚਕਾਂਕ(ctx, -1, idx + 1)) {
		duk_put_prop_ਸੂਚਕਾਂਕ(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_ਸੂਚਕਾਂਕ(ctx, -2, idx + 1);
	duk_ਕੱਢੋ(ctx);
}


ਖਾਲੀ js_handle_new_element(jsthread *thread, ਬਣਤਰ dom_element *ਗੰਢ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(thread);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਗੰਢ);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = ਨਿਰਾ;
	dom_string *ਕੁੰਜੀ = ਨਿਰਾ;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_ਗੰਢ_get_ਗੰਢ_name(ਗੰਢ, &nodename);
	ਜੇ (exc != DOM_NO_ERR) ਵਾਪਸ;

	ਜੇ (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_ਗੰਢ_get_ਗੁਣ(ਗੰਢ, &map);
	ਜੇ (exc != DOM_NO_ERR) ਵਾਪਸ;
	ਜੇ (map == ਨਿਰਾ) ਵਾਪਸ;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_ਲੰਮਾਈ(map, &siz);
	ਜੇ (exc != DOM_NO_ERR) ਜਾਓ ਬਾਹਰਲਾ;

	ਲਈ (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_ਵਸਤੂ(map, idx, &attr);
		ਜੇ (exc != DOM_NO_ERR) ਜਾਓ ਬਾਹਰਲਾ;
		exc = dom_attr_get_name(attr, &ਕੁੰਜੀ);
		ਜੇ (exc != DOM_NO_ERR) ਜਾਓ ਬਾਹਰਲਾ;
		ਜੇ (is_body && (
			    ਕੁੰਜੀ == corestring_dom_onblur ||
			    ਕੁੰਜੀ == corestring_dom_onerror ||
			    ਕੁੰਜੀ == corestring_dom_onfocus ||
			    ਕੁੰਜੀ == corestring_dom_onload ||
			    ਕੁੰਜੀ == corestring_dom_onresize ||
			    ਕੁੰਜੀ == corestring_dom_onscroll)) {
			/* This is a forwarded event, it doesn't matter,
			 * we should skip registering for it and later
			 * we will register it for Window itself
			 */
			ਜਾਓ skip_ਰਜਿਸਟਰ;
		}
		ਜੇ (dom_string_ਲੰਮਾਈ(ਕੁੰਜੀ) > 2) {
			/* Can be on* */
			ਅਚਲ uint8_t *ਅੰਕੜੇ = (ਅਚਲ uint8_t *)dom_string_ਅੰਕੜੇ(ਕੁੰਜੀ);
			ਜੇ (ਅੰਕੜੇ[0] == 'o' && ਅੰਕੜੇ[1] == 'n') {
				dom_string *sub = ਨਿਰਾ;
				exc = dom_string_substr(
					ਕੁੰਜੀ, 2, dom_string_ਲੰਮਾਈ(ਕੁੰਜੀ),
					&sub);
				ਜੇ (exc == DOM_NO_ERR) {
					dukky_ਰਜਿਸਟਰ_event_listener_for(
						CTX, ਗੰਢ, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_ਰਜਿਸਟਰ:
		dom_string_unref(ਕੁੰਜੀ); ਕੁੰਜੀ = ਨਿਰਾ;
		dom_ਗੰਢ_unref(attr); attr = ਨਿਰਾ;
	}

ਬਾਹਰਲਾ:
	ਜੇ (ਕੁੰਜੀ != ਨਿਰਾ)
		dom_string_unref(ਕੁੰਜੀ);

	ਜੇ (attr != ਨਿਰਾ)
		dom_ਗੰਢ_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

ਖਾਲੀ js_event_cleanup(jsthread *thread, ਬਣਤਰ dom_event *evt)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(thread);
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, EVENT_MAGIC);
	/* ... EVENT_MAP */
	duk_ਧੱਕੋ_ਸੂਚਕ(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_ਕੱਢੋ(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, ਅਚਲ ਅੱਖਰ *type, ਬਣਤਰ dom_document *doc, ਬਣਤਰ dom_ਗੰਢ *ਨਿਸ਼ਾਨਾ)
{
	dom_exception exc;
	dom_event *evt;
	dom_event_ਨਿਸ਼ਾਨਾ *body;

	NSLOG(dukky, DEBUG, "Event: %s (doc=%p, target=%p)", type, doc,
	      ਨਿਸ਼ਾਨਾ);

	/** @todo Make this more generic, this only handles load and only
	 * targetting the window, so that we actually stand a chance of
	 * getting 3.4 out.
	 */

	ਜੇ (ਨਿਸ਼ਾਨਾ != ਨਿਰਾ)
		/* Swallow non-Window-targetted events quietly */
		ਵਾਪਸ true;

	ਜੇ (strcmp(type, "load") != 0)
		/* Swallow non-load events quietly */
		ਵਾਪਸ true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	ਜੇ (exc != DOM_NO_ERR) ਵਾਪਸ true;
	exc = dom_event_init(evt, corestring_dom_ਲੱਦੋ, false, false);
	ਜੇ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ਵਾਪਸ true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_ਧੱਕੋ_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	ਜੇ (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_ਕੱਢੋ(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		ਜੇ (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			ਵਾਪਸ true;
		}
		dukky_ਧੱਕੋ_ਗੰਢ(CTX, (ਬਣਤਰ dom_ਗੰਢ *)body);
		/* ... handlers bodynode */
		ਜੇ (dukky_get_current_ਮੁੱਲ_ਦਾ_event_handler(
			    CTX, corestring_dom_ਲੱਦੋ, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_ਗੰਢ_unref(body);
			/* ... handlers */
			duk_ਕੱਢੋ(CTX);
			dukky_leave_thread(thread);
			ਵਾਪਸ true;
		}
		/* Unref the body, we don't need it any more */
		dom_ਗੰਢ_unref(body);
		/* ... handlers handler bodynode */
		duk_ਕੱਢੋ(CTX);
	}
	/* ... handlers handler */
	duk_ਪਾਓ(CTX, -2);
	/* ... handler handlers */
	duk_ਕੱਢੋ(CTX);
	/* ... handler */
	duk_ਧੱਕੋ_global_object(CTX);
	/* ... handler Window */
	dukky_ਧੱਕੋ_event(CTX, evt);
	/* ... handler Window event */
	dukky_reset_ਸ਼ੁਰੂ_ਸਮਾਂ(CTX);
	ਜੇ (duk_pcall_ਵਿਧੀ(CTX, 1) != 0) {
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
		      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(CTX, -5),
		      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(CTX, -4));
		NSLOG(dukky, DEBUG, "              was at: %s line %s",
		      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(CTX, -3),
		      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(CTX, -2));
		NSLOG(dukky, DEBUG, "         Stack trace: %s",
		      duk_ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_string(CTX, -1));

		duk_ਕੱਢੋ_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		ਵਾਪਸ true;
	}
	/* ... result */
	duk_ਕੱਢੋ(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	ਵਾਪਸ true;
}
