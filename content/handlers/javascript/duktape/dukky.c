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

#ವ್ಯಾಖ್ಯಾನಿಸು EVENT_MAGIC MAGIC(EVENT_MAP)
#ವ್ಯಾಖ್ಯಾನಿಸು HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#ವ್ಯಾಖ್ಯಾನಿಸು HANDLER_MAGIC MAGIC(HANDLER_MAP)
#ವ್ಯಾಖ್ಯಾನಿಸು EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#ವ್ಯಾಖ್ಯಾನಿಸು GENERICS_MAGIC MAGIC(GENERICS_ಕೋಷ್ಟಕ)
#ವ್ಯಾಖ್ಯಾನಿಸು THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
ರಚನೆ jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t ಮುಂದಿನ_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಜೀವಂತ_ಎಳೆಗಳು; /**< number of live threads */
	uint64_t exec_ಆರಂಭ_ಸಮಯ;
};

/**
 * dukky javascript thread
 */
ರಚನೆ jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಒಳ_use; /**< The number of times this thread is in use */
	jsheap *ರಾಶಿ; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

ಸ್ಥಿರ duk_ret_t dukky_populate_object(duk_context *ctx, ಶೂನ್ಯ *udata)
{
	/* ... obj args protoname nargs */
	ಪೂರ್ಣಾಂಕ nargs = duk_get_int(ctx, -1);
	duk_ತೆಗೆ(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	ಆದರೆ (duk_is_undefined(ctx, -1)) {
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
	duk_ಗಣ_prototype(ctx, -(nargs+4));
	/* ... obj[proto] args prototab proto */
	duk_get_prop_string(ctx, -1, INIT_MAGIC);
	/* ... obj[proto] args prototab proto initfn */
	duk_ಸೇರಿಸು(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_ತೆಗೆ_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	ಹಿಂತಿರುಗಿ 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ ವಾದಗಳು)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, ವಾದಗಳು);
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
	duk_ಸೇರಿಸು(ctx, -(ವಾದಗಳು+1));
	/* ... obj args */
	duk_ತಳ್ಳು_string(ctx, name);
	/* ... obj args name */
	duk_ತಳ್ಳು_int(ctx, ವಾದಗಳು);
	/* ... obj args name nargs */
	ಆದರೆ ((ret = duk_ಸುರಕ್ಷಿತ_call(ctx, dukky_populate_object, ಖಾಲಿ, ವಾದಗಳು + 3, 1))
	    != DUK_EXEC_SUCCESS)
		ಹಿಂತಿರುಗಿ ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	ಹಿಂತಿರುಗಿ DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_ತಳ್ಳು_ಗಂಟು_stacked(duk_context *ctx)
{
	ಪೂರ್ಣಾಂಕ ತುದಿ_at_fail = duk_get_ತುದಿ(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, ಗಂಟು_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	ಆದರೆ (duk_is_undefined(ctx, -1)) {
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
		ಆದರೆ (duk_ಸುರಕ್ಷಿತ_call(ctx, dukky_populate_object, ಖಾಲಿ, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_ಗಣ_ತುದಿ(ctx, ತುದಿ_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			ಹಿಂತಿರುಗಿ false;
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
	ಆದರೆ (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * what = duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_ತೆಗೆ(ctx);
	}
	ಹಿಂತಿರುಗಿ true;
}

#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

ಸ್ಥಿರ ಶೂನ್ಯ dukky_html_element_class_from_ಟ್ಯಾಗ್_type(dom_html_element_type type,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **html_class, size_t *html_class_len)
{
	ಆಯ್ಕೆ(type) {
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_HTML:
		ಗಣ_HTML_CLASS(HTML)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ತಲೆ:
		ಗಣ_HTML_CLASS(ತಲೆ)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_META:
		ಗಣ_HTML_CLASS(META)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ತಳ:
		ಗಣ_HTML_CLASS(ತಳ_2)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TITLE:
		ಗಣ_HTML_CLASS(TITLE)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_BODY:
		ಗಣ_HTML_CLASS(BODY)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_DIV:
		ಗಣ_HTML_CLASS(DIV)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_FORM:
		ಗಣ_HTML_CLASS(FORM)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_LINK:
		ಗಣ_HTML_CLASS(LINK)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_BUTTON:
		ಗಣ_HTML_CLASS(BUTTON)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ಒಳಹರಿವು:
		ಗಣ_HTML_CLASS(ಒಳಹರಿವು_2)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		ಗಣ_HTML_CLASS(TEXTAREA)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		ಗಣ_HTML_CLASS(OPTGROUP)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_OPTION:
		ಗಣ_HTML_CLASS(OPTION)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_SELECT:
		ಗಣ_HTML_CLASS(SELECT)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_HR:
		ಗಣ_HTML_CLASS(HR)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_DL:
		ಗಣ_HTML_CLASS(DLIST)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_DIR:
		ಗಣ_HTML_CLASS(DIRECTORY)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_MENU:
		ಗಣ_HTML_CLASS(MENU)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_FIELDSET:
		ಗಣ_HTML_CLASS(FIELDSET)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_LEGEND:
		ಗಣ_HTML_CLASS(LEGEND)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_P:
		ಗಣ_HTML_CLASS(PARAGRAPH)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_H1:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_H2:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_H3:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_H4:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_H5:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_H6:
		ಗಣ_HTML_CLASS(HEADING)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_Q:
		ಗಣ_HTML_CLASS(QUOTE)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_PRE:
		ಗಣ_HTML_CLASS(PRE)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_BR:
		ಗಣ_HTML_CLASS(BR)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_LABEL:
		ಗಣ_HTML_CLASS(LABEL)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_UL:
		ಗಣ_HTML_CLASS(ULIST)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_OL:
		ಗಣ_HTML_CLASS(OLIST)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_LI:
		ಗಣ_HTML_CLASS(LI)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_FONT:
		ಗಣ_HTML_CLASS(FONT)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_DEL:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_INS:
		ಗಣ_HTML_CLASS(MOD)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_A:
		ಗಣ_HTML_CLASS(ANCHOR)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_BASEFONT:
		ಗಣ_HTML_CLASS(BASEFONT)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_IMG:
		ಗಣ_HTML_CLASS(IMAGE)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_OBJECT:
		ಗಣ_HTML_CLASS(OBJECT)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_PARAM:
		ಗಣ_HTML_CLASS(PARAM)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_APPLET:
		ಗಣ_HTML_CLASS(APPLET)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_MAP:
		ಗಣ_HTML_CLASS(MAP)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_AREA:
		ಗಣ_HTML_CLASS(AREA)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_SCRIPT:
		ಗಣ_HTML_CLASS(SCRIPT)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_CAPTION:
		ಗಣ_HTML_CLASS(TABLECAPTION)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TD:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TH:
		ಗಣ_HTML_CLASS(TABLECELL)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ಲಂಬಸಾಲು:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_COLGROUP:
		ಗಣ_HTML_CLASS(TABLECOL)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_THEAD:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TBODY:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TFOOT:
		ಗಣ_HTML_CLASS(TABLESECTION)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ಕೋಷ್ಟಕ:
		ಗಣ_HTML_CLASS(ಕೋಷ್ಟಕ_2)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TR:
		ಗಣ_HTML_CLASS(TABLEROW)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_STYLE:
		ಗಣ_HTML_CLASS(STYLE)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_FRAMESET:
		ಗಣ_HTML_CLASS(FRAMESET)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ಚೌಕಟ್ಟು:
		ಗಣ_HTML_CLASS(ಚೌಕಟ್ಟು_2)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_IFRAME:
		ಗಣ_HTML_CLASS(IFRAME)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ISINDEX:
		ಗಣ_HTML_CLASS(ISINDEX)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_CANVAS:
		ಗಣ_HTML_CLASS(CANVAS)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ಎಣಿಕೆ:
		ಪ್ರತಿಜ್ಞಾ(type != DOM_HTML_ELEMENT_TYPE_ಎಣಿಕೆ);
		fallthrough;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		ಗಣ_HTML_CLASS(UNKNOWN)
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ;
}

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಗಣ_HTML_CLASS

ಸ್ಥಿರ ಶೂನ್ಯ
dukky_ತಳ್ಳು_ಗಂಟು_klass(duk_context *ctx, ರಚನೆ dom_ಗಂಟು *ಗಂಟು)
{
	dom_ಗಂಟು_type nodetype;
	dom_exception err;

	err = dom_ಗಂಟು_get_ಗಂಟು_type(ಗಂಟು, &nodetype);
	ಆದರೆ (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_ತಳ್ಳು_string(ctx, PROTO_NAME(ಗಂಟು_2));
		ಹಿಂತಿರುಗಿ;
	}

	ಆಯ್ಕೆ(nodetype) {
	ಸ್ಥಿತಿ DOM_ELEMENT_ಗಂಟು: {
		dom_string *namespace;
		dom_html_element_type type;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *html_class;
		size_t html_class_len;
		err = dom_ಗಂಟು_get_namespace(ಗಂಟು, &namespace);
		ಆದರೆ (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_ತಳ್ಳು_string(ctx, PROTO_NAME(ELEMENT));
			ನಿಲ್ಲಿಸು;
		}
		ಆದರೆ (namespace == ಖಾಲಿ) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_ತಳ್ಳು_string(ctx, PROTO_NAME(ELEMENT));
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_ತಳ್ಳು_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			ನಿಲ್ಲಿಸು;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_ಟ್ಯಾಗ್_type(ಗಂಟು, &type);
		ಆದರೆ (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_ಟ್ಯಾಗ್_type(type,
				&html_class, &html_class_len);

		duk_ತಳ್ಳು_lstring(ctx, html_class, html_class_len);
		ನಿಲ್ಲಿಸು;
	}
	ಸ್ಥಿತಿ DOM_ಪಠ್ಯ_ಗಂಟು:
		duk_ತಳ್ಳು_string(ctx, PROTO_NAME(ಪಠ್ಯ_2));
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_COMMENT_ಗಂಟು:
		duk_ತಳ್ಳು_string(ctx, PROTO_NAME(COMMENT));
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_DOCUMENT_ಗಂಟು:
		duk_ತಳ್ಳು_string(ctx, PROTO_NAME(DOCUMENT));
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_ATTRIBUTE_ಗಂಟು:
	ಸ್ಥಿತಿ DOM_PROCESSING_INSTRUCTION_ಗಂಟು:
	ಸ್ಥಿತಿ DOM_DOCUMENT_TYPE_ಗಂಟು:
	ಸ್ಥಿತಿ DOM_DOCUMENT_FRAGMENT_ಗಂಟು:
	ಸ್ಥಿತಿ DOM_NOTATION_ಗಂಟು:
	ಸ್ಥಿತಿ DOM_ENTITY_REFERENCE_ಗಂಟು:
	ಸ್ಥಿತಿ DOM_ENTITY_ಗಂಟು:
	ಸ್ಥಿತಿ DOM_CDATA_SECTION_ಗಂಟು:
	ಪೂರ್ವನಿಯೋಜಿತ:
		/* Oh bum, just node then */
		duk_ತಳ್ಳು_string(ctx, PROTO_NAME(ಗಂಟು_2));
	}
}

duk_bool_t
dukky_ತಳ್ಳು_ಗಂಟು(duk_context *ctx, ರಚನೆ dom_ಗಂಟು *ಗಂಟು)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", ಗಂಟು);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, ಗಂಟು_MAGIC);
	/* ... nodes */
	duk_ತಳ್ಳು_ಸೂಚಕ(ctx, ಗಂಟು);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	ಆದರೆ (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_ಸೇರಿಸು(ctx, -2);
		/* ... node nodes */
		duk_ತೆಗೆ(ctx);
		/* ... node */
		ಆದರೆ (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * what = duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_ತೆಗೆ(ctx);
		}
		ಹಿಂತಿರುಗಿ true;
	}
	/* ... nodes undefined */
	duk_ತೆಗೆ_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_ತಳ್ಳು_ಸೂಚಕ(ctx, ಗಂಟು);
	/* ... nodeptr */
	dukky_ತಳ್ಳು_ಗಂಟು_klass(ctx, ಗಂಟು);
	/* ... nodeptr klass */
	ಹಿಂತಿರುಗಿ dukky_ತಳ್ಳು_ಗಂಟು_stacked(ctx);
}

ಸ್ಥಿರ duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	ಹಿಂತಿರುಗಿ duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

ಶೂನ್ಯ
dukky_inject_not_ctr(duk_context *ctx, ಪೂರ್ಣಾಂಕ idx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
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
	ಹಿಂತಿರುಗಿ;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

ಸ್ಥಿರ ಶೂನ್ಯ *dukky_alloc_function(ಶೂನ್ಯ *udata, duk_ಗಾತ್ರ_t ಗಾತ್ರ_2)
{
	ಆದರೆ (ಗಾತ್ರ_2 == 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2);
}

ಸ್ಥಿರ ಶೂನ್ಯ *dukky_realloc_function(ಶೂನ್ಯ *udata, ಶೂನ್ಯ *ptr, duk_ಗಾತ್ರ_t ಗಾತ್ರ_2)
{
	ಆದರೆ (ptr == ಖಾಲಿ && ಗಾತ್ರ_2 == 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (ಗಾತ್ರ_2 == 0) {
		ಬಿಡುಗಡೆ(ptr);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ realloc(ptr, ಗಾತ್ರ_2);
}


ಸ್ಥಿರ ಶೂನ್ಯ dukky_free_function(ಶೂನ್ಯ *udata, ಶೂನ್ಯ *ptr)
{
	ಆದರೆ (ptr != ಖಾಲಿ)
		ಬಿಡುಗಡೆ(ptr);
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
js_newheap(ಪೂರ್ಣಾಂಕ timeout, jsheap **ರಾಶಿ)
{
	duk_context *ctx;
	jsheap *ret = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*ret));
	*ರಾಶಿ = ಖಾಲಿ;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	ಆದರೆ (ret == ಖಾಲಿ) ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_ರಾಶಿ(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		ಖಾಲಿ);
	ಆದರೆ (ret->ctx == ಖಾಲಿ) { ಬಿಡುಗಡೆ(ret); ಹಿಂತಿರುಗಿ NSERROR_NOMEM; }
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

	*ರಾಶಿ = ret;
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


ಸ್ಥಿರ ಶೂನ್ಯ dukky_destroyheap(jsheap *ರಾಶಿ)
{
	ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ->pending_destroy == true);
	ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ->ಜೀವಂತ_ಎಳೆಗಳು == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_ರಾಶಿ(ರಾಶಿ->ctx);
	ಬಿಡುಗಡೆ(ರಾಶಿ);
}

/* exported interface documented in js.h */
ಶೂನ್ಯ js_destroyheap(jsheap *ರಾಶಿ)
{
	ರಾಶಿ->pending_destroy = true;
	ಆದರೆ (ರಾಶಿ->ಜೀವಂತ_ಎಳೆಗಳು == 0) {
		dukky_destroyheap(ರಾಶಿ);
	}
}

/* Just for here, the CTX is in ret, not thread */
#ವ್ಯಾಖ್ಯಾನಿಸು CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *ರಾಶಿ, ಶೂನ್ಯ *win_priv, ಶೂನ್ಯ *doc_priv, jsthread **thread)
{
	jsthread *ret;
	ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ->pending_destroy == false);

	ret = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ (*ret));
	ಆದರೆ (ret == ಖಾಲಿ) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(ರಾಶಿ->ctx, THREAD_MAP); /* ... threads */
	duk_ತಳ್ಳು_thread(ರಾಶಿ->ctx); /* ... threads thread */
	ret->ರಾಶಿ = ರಾಶಿ;
	ret->ctx = duk_require_context(ರಾಶಿ->ctx, -1);
	ret->thread_idx = ರಾಶಿ->ಮುಂದಿನ_thread++;
	duk_put_prop_ಸೂಚ್ಯಂಕ(ರಾಶಿ->ctx, -2, ret->thread_idx);
	ರಾಶಿ->ಜೀವಂತ_ಎಳೆಗಳು++;
	duk_ತೆಗೆ(ರಾಶಿ->ctx); /* ... */
	duk_ತಳ್ಳು_int(CTX, 0);
	duk_ತಳ್ಳು_int(CTX, 1);
	duk_ತಳ್ಳು_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_ತಳ್ಳು_ಸೂಚಕ(CTX, win_priv);
	duk_ತಳ್ಳು_ಸೂಚಕ(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(ಕಿಟಕಿ), 2);
	duk_ತಳ್ಳು_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_ಗಣ_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_ತಳ್ಳು_object(CTX);
	duk_ತಳ್ಳು_ಸೂಚಕ(CTX, ಖಾಲಿ);
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
	ಆದರೆ (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	ಆದರೆ (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_ತೆಗೆ(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_ತಳ್ಳು_string(CTX, "generics.js");
	/* ..., generics.js */
	ಆದರೆ (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	ಆದರೆ (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
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

	dukky_log_ರಾಶಿ_ಚೌಕಟ್ಟು(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, ರಾಶಿ);
	*thread = ret;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* Now switch to the long term CTX behaviour */
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು CTX
#ವ್ಯಾಖ್ಯಾನಿಸು CTX (thread->ctx)

/* exported interface documented in js.h */
nserror js_closethread(jsthread *thread)
{
	/* We can always close down a thread, it might just confuse
	 * the code running, though we don't mind since we're in the
	 * process of destruction at this point
	 */
	duk_int_t ತುದಿ = duk_get_ತುದಿ(CTX);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, thread->ರಾಶಿ);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Restore whatever stack we had */
	duk_ಗಣ_ತುದಿ(CTX, ತುದಿ);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
ಸ್ಥಿರ ಶೂನ್ಯ dukky_destroythread(jsthread *thread)
{
	jsheap *ರಾಶಿ = thread->ರಾಶಿ;

	ಪ್ರತಿಜ್ಞಾ(thread->ಒಳ_use == 0);
	ಪ್ರತಿಜ್ಞಾ(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, ರಾಶಿ);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(ರಾಶಿ->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_ಸೂಚ್ಯಂಕ(ರಾಶಿ->ctx, -1, thread->thread_idx);
	duk_ತೆಗೆ(ರಾಶಿ->ctx); /* ... */

	/* We can now free the thread object */
	ಬಿಡುಗಡೆ(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(ರಾಶಿ->ctx, 0);
	duk_gc(ರಾಶಿ->ctx, DUK_GC_COMPACT);
	ರಾಶಿ->ಜೀವಂತ_ಎಳೆಗಳು--;

	/* And if the heap should now go, blow it away */
	ಆದರೆ (ರಾಶಿ->pending_destroy == true && ರಾಶಿ->ಜೀವಂತ_ಎಳೆಗಳು == 0) {
		dukky_destroyheap(ರಾಶಿ);
	}
}

/* exported interface documented in js.h */
ಶೂನ್ಯ js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	ಆದರೆ (thread->ಒಳ_use == 0) {
		dukky_destroythread(thread);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ dukky_enter_thread(jsthread *thread)
{
	ಪ್ರತಿಜ್ಞಾ(thread != ಖಾಲಿ);
	thread->ಒಳ_use++;
}

ಸ್ಥಿರ ಶೂನ್ಯ dukky_leave_thread(jsthread *thread)
{
	ಪ್ರತಿಜ್ಞಾ(thread != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(thread->ಒಳ_use > 0);

	thread->ಒಳ_use--;
	ಆದರೆ (thread->ಒಳ_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(ಶೂನ್ಯ *udata)
{
#ವ್ಯಾಖ್ಯಾನಿಸು JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *ರಾಶಿ = (jsheap *) udata;
	uint64_t now;

	(ಶೂನ್ಯ) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	ಹಿಂತಿರುಗಿ ರಾಶಿ->exec_ಆರಂಭ_ಸಮಯ != 0 &&
			now > (ರಾಶಿ->exec_ಆರಂಭ_ಸಮಯ + JS_EXEC_TIMEOUT_MS);
}

ಸ್ಥಿರ ಶೂನ್ಯ dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_ತುದಿ(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_ತೆಗೆ(ctx);
	/* ..., errobj */
}

ಸ್ಥಿರ ಶೂನ್ಯ dukky_reset_ಆರಂಭ_ಸಮಯ(duk_context *ctx)
{
	duk_ಸ್ಮರಣೆ_functions funcs;
	jsheap *ರಾಶಿ;
	duk_get_ಸ್ಮರಣೆ_functions(ctx, &funcs);
	ರಾಶಿ = funcs.udata;
	(ಶೂನ್ಯ) nsu_getmonotonic_ms(&ರಾಶಿ->exec_ಆರಂಭ_ಸಮಯ);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_ಗಾತ್ರ_t argc, bool reset_timeout)
{
	ಆದರೆ (reset_timeout) {
		dukky_reset_ಆರಂಭ_ಸಮಯ(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	ಆದರೆ (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	ಹಿಂತಿರುಗಿ ret;
}


ಶೂನ್ಯ dukky_ತಳ್ಳು_generics(duk_context *ctx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *generic)
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
	ಹಿಂತಿರುಗಿ 1;
}

ಶೂನ್ಯ dukky_log_ರಾಶಿ_ಚೌಕಟ್ಟು(duk_context *ctx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * reason)
{
	ಆದರೆ (duk_ಸುರಕ್ಷಿತ_call(ctx, dukky_ತಳ್ಳು_context_dump, ಖಾಲಿ, 0, 1) != 0) {
		duk_ತೆಗೆ(ctx);
		duk_ತಳ್ಳು_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(ctx, -1));
	duk_ತೆಗೆ(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, ಸ್ಥಿರಾಂಕ uint8_t *txt, size_t txtlen, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	bool ret = false;
	ಪ್ರತಿಜ್ಞಾ(thread);

	ಆದರೆ (txt == ಖಾಲಿ || txtlen == 0) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		ಹಿಂತಿರುಗಿ false;
	}

	dukky_enter_thread(thread);

	duk_ಗಣ_ತುದಿ(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_ಆರಂಭ_ಸಮಯ(CTX);
	ಆದರೆ (name != ಖಾಲಿ) {
		duk_ತಳ್ಳು_string(CTX, name);
	} ಇಲ್ಲದಿದ್ದರೆ {
		duk_ತಳ್ಳು_string(CTX, "?unknown source?");
	}
	ಆದರೆ (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		ಹೋಗು handle_error;
	}

	ಆದರೆ (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		ಹೋಗು handle_error;
	}

	ಆದರೆ (duk_get_ತುದಿ(CTX) == 0) duk_ತಳ್ಳು_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	ಹೋಗು ಹೊರಹೋಗು;

handle_error:
	dukky_dump_error(CTX);
ಹೊರಹೋಗು:
	dukky_leave_thread(thread);
	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* dukky_event_proto(dom_event *evt)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ret = PROTO_NAME(EVENT);
	dom_string *type = ಖಾಲಿ;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	ಆದರೆ (err != DOM_NO_ERR) {
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		ಹೋಗು ಹೊರಹೋಗು;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		ಹೋಗು ಹೊರಹೋಗು;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		ಹೋಗು ಹೊರಹೋಗು;
	}

ಹೊರಹೋಗು:
	ಆದರೆ (type != ಖಾಲಿ) {
		dom_string_unref(type);
	}

	ಹಿಂತಿರುಗಿ ret;
}

/*** New style event handling ***/

ಶೂನ್ಯ dukky_ತಳ್ಳು_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_ತಳ್ಳು_ಸೂಚಕ(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	ಆದರೆ (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_ತೆಗೆ(ctx);
		/* ... events */
		duk_ತಳ್ಳು_ಸೂಚಕ(ctx, evt);
		ಆದರೆ (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_ತೆಗೆ(ctx);
			/* ... events */
			duk_ತಳ್ಳು_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_ತಳ್ಳು_ಸೂಚಕ(ctx, evt);
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

ಸ್ಥಿರ ಶೂನ್ಯ dukky_ತಳ್ಳು_handler_ಸಂಕೇತ(duk_context *ctx, dom_string *name,
				     dom_event_ಗುರಿ *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_ಗಂಟು_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	ಆದರೆ (et == ಖಾಲಿ) {
		duk_ತಳ್ಳು_lstring(ctx, "", 0);
		ಹಿಂತಿರುಗಿ;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_ಗಂಟು_get_ಗಂಟು_type(et, &ntype);
	ಆದರೆ (exc != DOM_NO_ERR) {
		duk_ತಳ್ಳು_lstring(ctx, "", 0);
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (ntype != DOM_ELEMENT_ಗಂಟು) {
		duk_ತಳ್ಳು_lstring(ctx, "", 0);
		ಹಿಂತಿರುಗಿ;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	ಆದರೆ (exc != DOM_NO_ERR) {
		duk_ತಳ್ಳು_lstring(ctx, "", 0);
		ಹಿಂತಿರುಗಿ;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	ಆದರೆ ((exc != DOM_NO_ERR) || (val == ಖಾಲಿ)) {
		dom_string_unref(onname);
		duk_ತಳ್ಳು_lstring(ctx, "", 0);
		ಹಿಂತಿರುಗಿ;
	}

	dom_string_unref(onname);
	duk_ತಳ್ಳು_lstring(ctx, dom_string_ದತ್ತಾಂಶ(val), dom_string_ಉದ್ದಳತೆ(val));
	dom_string_unref(val);
}

bool dukky_get_current_ಮೌಲ್ಯ_ದ_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_ಗುರಿ *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_ತಳ್ಳು_lstring(ctx, dom_string_ದತ್ತಾಂಶ(name), dom_string_ಉದ್ದಳತೆ(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	ಆದರೆ (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_ತೆಗೆ_2(ctx);
		/* ... node */
		dukky_ತಳ್ಳು_handler_ಸಂಕೇತ(ctx, name, et);
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
		ಆದರೆ (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_ತೆಗೆ_2(ctx);
			ಹಿಂತಿರುಗಿ false;
		}
		/* ... node handler */
		duk_ಸೇರಿಸು(ctx, -2);
		/* ... handler node */
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* ... node handlers handler */
		duk_ಸೇರಿಸು(ctx, -3);
		/* ... handler node handlers */
		duk_ತೆಗೆ(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ dukky_generic_event_handler(dom_event *evt, ಶೂನ್ಯ *pw)
{
	duk_context *ctx = (duk_context *)pw;
	dom_string *name;
	dom_exception exc;
	dom_event_ಗುರಿ *targ;
	dom_event_flow_phase phase;
	duk_uarridx_t idx;
	event_listener_flags flags;

	NSLOG(dukky, DEBUG, "Handling an event in duktape interface...");
	exc = dom_event_get_type(evt, &name);
	ಆದರೆ (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		ಹಿಂತಿರುಗಿ;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (ಪೂರ್ಣಾಂಕ)dom_string_ಉದ್ದಳತೆ(name),
	      dom_string_ದತ್ತಾಂಶ(name));
	exc = dom_event_get_event_phase(evt, &phase);
	ಆದರೆ (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		ಹಿಂತಿರುಗಿ;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_ಗುರಿ ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (ಪೂರ್ಣಾಂಕ)phase);

	exc = dom_event_get_current_ಗುರಿ(evt, &targ);
	ಆದರೆ (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		ಹಿಂತಿರುಗಿ;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	ಆದರೆ (phase == DOM_CAPTURING_PHASE)
		ಹೋಗು handle_extras;

	/* ... */
	ಆದರೆ (dukky_ತಳ್ಳು_ಗಂಟು(ctx, (dom_ಗಂಟು *)targ) == false) {
		dom_string_unref(name);
		dom_ಗಂಟು_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		ಹಿಂತಿರುಗಿ;
	}
	/* ... node */
	ಆದರೆ (dukky_get_current_ಮೌಲ್ಯ_ದ_event_handler(
		    ctx, name, (dom_event_ಗುರಿ *)targ) == false) {
		/* ... */
		ಹೋಗು handle_extras;
	}
	/* ... handler node */
	dukky_ತಳ್ಳು_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_ಆರಂಭ_ಸಮಯ(ctx);
	ಆದರೆ (duk_pcall_ವಿಧಾನ(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_ತಕ್ಷಣ_ಪ್ರಸರಣ(evt);
		ಆದರೆ (exc != DOM_NO_ERR)
			NSLOG(dukky, DEBUG,
			      "WORSE! could not stop propagation");
		duk_get_prop_string(ctx, -1, "name");
		duk_get_prop_string(ctx, -2, "message");
		duk_get_prop_string(ctx, -3, "fileName");
		duk_get_prop_string(ctx, -4, "lineNumber");
		duk_get_prop_string(ctx, -5, "stack");
		/* ... err name message fileName lineNumber stack */
		NSLOG(dukky, DEBUG, "Uncaught error in JS: %s: %s",
		      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(ctx, -5),
		      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(ctx, -4));
		NSLOG(dukky, INFO, "              was at: %s line %s",
		      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(ctx, -3),
		      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(ctx, -2));
		NSLOG(dukky, INFO, "         Stack trace: %s",
		      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(ctx, -1));

		duk_ತೆಗೆ_n(ctx, 6);
		/* ... */
		ಹೋಗು handle_extras;
	}
	/* ... result */
	ಆದರೆ (duk_is_boolean(ctx, -1) &&
	    duk_ಗಮ್ಯ_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_ತೆಗೆ(ctx);
handle_extras:
	/* ... */
	duk_ತಳ್ಳು_lstring(ctx, dom_string_ದತ್ತಾಂಶ(name), dom_string_ಉದ್ದಳತೆ(name));
	dukky_ತಳ್ಳು_ಗಂಟು(ctx, (dom_ಗಂಟು *)targ);
	/* ... type node */
	ಆದರೆ (dukky_event_ಗುರಿ_ತಳ್ಳು_listeners(ctx, true)) {
		/* Nothing to do */
		duk_ತೆಗೆ(ctx);
		ಹೋಗು ಹೊರಹೋಗು;
	}
	/* ... sublisteners */
	duk_ತಳ್ಳು_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	ಆಗ (duk_get_prop_ಸೂಚ್ಯಂಕ(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_ಸೂಚ್ಯಂಕ(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		ಆದರೆ ((event_listener_flags)duk_ಗಮ್ಯ_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_ತೆಗೆ(ctx);
			/* ... subl copy handler flags */
		}
		duk_ತೆಗೆ(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_ಸೂಚ್ಯಂಕ(ctx, -2, idx);
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
	ಆಗ (duk_get_prop_ಸೂಚ್ಯಂಕ(ctx, -1, idx++)) {
		/* ... copy handler */
		ಆದರೆ (duk_get_prop_ಸೂಚ್ಯಂಕ(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_ತೆಗೆ_2(ctx);
			ಮುಂದುವರಿ;
		}
		duk_ತೆಗೆ(ctx);
		duk_get_prop_ಸೂಚ್ಯಂಕ(ctx, -1, 0);
		duk_get_prop_ಸೂಚ್ಯಂಕ(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_ತೆಗೆ(ctx);
		/* ... copy handler callback */
		ಆದರೆ (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_ತೆಗೆ_2(ctx);
			/* ... copy */
			ಮುಂದುವರಿ;
		}
		/* ... copy handler callback */
		dukky_ತಳ್ಳು_ಗಂಟು(ctx, (dom_ಗಂಟು *)targ);
		/* ... copy handler callback node */
		dukky_ತಳ್ಳು_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_ಆರಂಭ_ಸಮಯ(ctx);
		ಆದರೆ (duk_pcall_ವಿಧಾನ(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_ತಕ್ಷಣ_ಪ್ರಸರಣ(evt);
			ಆದರೆ (exc != DOM_NO_ERR)
				NSLOG(dukky, DEBUG,
				      "WORSE! could not stop propagation");
			duk_get_prop_string(ctx, -1, "name");
			duk_get_prop_string(ctx, -2, "message");
			duk_get_prop_string(ctx, -3, "fileName");
			duk_get_prop_string(ctx, -4, "lineNumber");
			duk_get_prop_string(ctx, -5, "stack");
			/* ... err name message fileName lineNumber stack */
			NSLOG(dukky, DEBUG, "Uncaught error in JS: %s: %s",
			      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(ctx, -5),
			      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(ctx, -4));
			NSLOG(dukky, DEBUG,
			      "              was at: %s line %s",
			      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(ctx, -3),
			      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(ctx, -2));
			NSLOG(dukky, DEBUG, "         Stack trace: %s",
			      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(ctx, -1));

			duk_ತೆಗೆ_n(ctx, 7);
			/* ... copy */
			ಮುಂದುವರಿ;
		}
		/* ... copy handler result */
		ಆದರೆ (duk_is_boolean(ctx, -1) &&
		    duk_ಗಮ್ಯ_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_ತೆಗೆ_2(ctx);
		/* ... copy */
	}
	duk_ತೆಗೆ_2(ctx);
ಹೊರಹೋಗು:
	/* ... */
	dom_ಗಂಟು_unref(targ);
	dom_string_unref(name);
}

ಶೂನ್ಯ dukky_ರಿಜಿಸ್ಟರ್_event_listener_for(duk_context *ctx,
				       ರಚನೆ dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = ಖಾಲಿ;
	dom_exception exc;

	/* ... */
	ಆದರೆ (ele == ಖಾಲಿ) {
		/* A null element is the Window object */
		duk_ತಳ್ಳು_global_object(ctx);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Non null elements must be pushed as a node object */
		ಆದರೆ (dukky_ತಳ್ಳು_ಗಂಟು(ctx, (ರಚನೆ dom_ಗಂಟು *)ele) == false)
			ಹಿಂತಿರುಗಿ;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_ತಳ್ಳು_lstring(ctx, dom_string_ದತ್ತಾಂಶ(name), dom_string_ಉದ್ದಳತೆ(name));
	/* ... node handlers name */
	ಆದರೆ (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_ತೆಗೆ_2(ctx);
		/* ... */
		ಹಿಂತಿರುಗಿ;
	}
	/* ... node handlers */
	duk_ತಳ್ಳು_lstring(ctx, dom_string_ದತ್ತಾಂಶ(name), dom_string_ಉದ್ದಳತೆ(name));
	/* ... node handlers name */
	duk_ತಳ್ಳು_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_ತೆಗೆ_2(ctx);
	/* ... */
	ಆದರೆ (ele == ಖಾಲಿ) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		ಹಿಂತಿರುಗಿ;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	ಆದರೆ (exc != DOM_NO_ERR) ಹಿಂತಿರುಗಿ;
	exc = dom_event_ಗುರಿ_ಕೂಡಿಸು_event_listener(
		ele, name, listen, capture);
	ಆದರೆ (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (ಪೂರ್ಣಾಂಕ)dom_string_ಉದ್ದಳತೆ(name), dom_string_ದತ್ತಾಂಶ(name));
	} ಇಲ್ಲದಿದ್ದರೆ {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (ಪೂರ್ಣಾಂಕ)dom_string_ಉದ್ದಳತೆ(name), dom_string_ದತ್ತಾಂಶ(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_ಗುರಿ_ತಳ್ಳು_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	ಆದರೆ (duk_is_undefined(ctx, -1)) {
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
	ಆದರೆ (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		ಆದರೆ (dont_create == true) {
			duk_ತೆಗೆ_3(ctx);
			duk_ತಳ್ಳು_undefined(ctx);
			ಹಿಂತಿರುಗಿ true;
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
	ಹಿಂತಿರುಗಿ ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
ಶೂನ್ಯ dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	ಆಗ (duk_get_prop_ಸೂಚ್ಯಂಕ(ctx, -1, idx + 1)) {
		duk_put_prop_ಸೂಚ್ಯಂಕ(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_ಸೂಚ್ಯಂಕ(ctx, -2, idx + 1);
	duk_ತೆಗೆ(ctx);
}


ಶೂನ್ಯ js_handle_new_element(jsthread *thread, ರಚನೆ dom_element *ಗಂಟು)
{
	ಪ್ರತಿಜ್ಞಾ(thread);
	ಪ್ರತಿಜ್ಞಾ(ಗಂಟು);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = ಖಾಲಿ;
	dom_string *ಕೀಲಿ = ಖಾಲಿ;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_ಗಂಟು_get_ಗಂಟು_name(ಗಂಟು, &nodename);
	ಆದರೆ (exc != DOM_NO_ERR) ಹಿಂತಿರುಗಿ;

	ಆದರೆ (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_ಗಂಟು_get_ಗುಣಲಕ್ಷಣಗಳು(ಗಂಟು, &map);
	ಆದರೆ (exc != DOM_NO_ERR) ಹಿಂತಿರುಗಿ;
	ಆದರೆ (map == ಖಾಲಿ) ಹಿಂತಿರುಗಿ;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_ಉದ್ದಳತೆ(map, &siz);
	ಆದರೆ (exc != DOM_NO_ERR) ಹೋಗು ಹೊರಹೋಗು;

	ಗಾಗಿ (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_ಅಂಶ(map, idx, &attr);
		ಆದರೆ (exc != DOM_NO_ERR) ಹೋಗು ಹೊರಹೋಗು;
		exc = dom_attr_get_name(attr, &ಕೀಲಿ);
		ಆದರೆ (exc != DOM_NO_ERR) ಹೋಗು ಹೊರಹೋಗು;
		ಆದರೆ (is_body && (
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
			ಹೋಗು skip_ರಿಜಿಸ್ಟರ್;
		}
		ಆದರೆ (dom_string_ಉದ್ದಳತೆ(ಕೀಲಿ) > 2) {
			/* Can be on* */
			ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ = (ಸ್ಥಿರಾಂಕ uint8_t *)dom_string_ದತ್ತಾಂಶ(ಕೀಲಿ);
			ಆದರೆ (ದತ್ತಾಂಶ[0] == 'o' && ದತ್ತಾಂಶ[1] == 'n') {
				dom_string *sub = ಖಾಲಿ;
				exc = dom_string_substr(
					ಕೀಲಿ, 2, dom_string_ಉದ್ದಳತೆ(ಕೀಲಿ),
					&sub);
				ಆದರೆ (exc == DOM_NO_ERR) {
					dukky_ರಿಜಿಸ್ಟರ್_event_listener_for(
						CTX, ಗಂಟು, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_ರಿಜಿಸ್ಟರ್:
		dom_string_unref(ಕೀಲಿ); ಕೀಲಿ = ಖಾಲಿ;
		dom_ಗಂಟು_unref(attr); attr = ಖಾಲಿ;
	}

ಹೊರಹೋಗು:
	ಆದರೆ (ಕೀಲಿ != ಖಾಲಿ)
		dom_string_unref(ಕೀಲಿ);

	ಆದರೆ (attr != ಖಾಲಿ)
		dom_ಗಂಟು_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

ಶೂನ್ಯ js_event_cleanup(jsthread *thread, ರಚನೆ dom_event *evt)
{
	ಪ್ರತಿಜ್ಞಾ(thread);
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, EVENT_MAGIC);
	/* ... EVENT_MAP */
	duk_ತಳ್ಳು_ಸೂಚಕ(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_ತೆಗೆ(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type, ರಚನೆ dom_document *doc, ರಚನೆ dom_ಗಂಟು *ಗುರಿ)
{
	dom_exception exc;
	dom_event *evt;
	dom_event_ಗುರಿ *body;

	NSLOG(dukky, DEBUG, "Event: %s (doc=%p, target=%p)", type, doc,
	      ಗುರಿ);

	/** @todo Make this more generic, this only handles load and only
	 * targetting the window, so that we actually stand a chance of
	 * getting 3.4 out.
	 */

	ಆದರೆ (ಗುರಿ != ಖಾಲಿ)
		/* Swallow non-Window-targetted events quietly */
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (strcmp(type, "load") != 0)
		/* Swallow non-load events quietly */
		ಹಿಂತಿರುಗಿ true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	ಆದರೆ (exc != DOM_NO_ERR) ಹಿಂತಿರುಗಿ true;
	exc = dom_event_init(evt, corestring_dom_ತುಂಬು, false, false);
	ಆದರೆ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ಹಿಂತಿರುಗಿ true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_ತಳ್ಳು_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	ಆದರೆ (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_ತೆಗೆ(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		ಆದರೆ (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			ಹಿಂತಿರುಗಿ true;
		}
		dukky_ತಳ್ಳು_ಗಂಟು(CTX, (ರಚನೆ dom_ಗಂಟು *)body);
		/* ... handlers bodynode */
		ಆದರೆ (dukky_get_current_ಮೌಲ್ಯ_ದ_event_handler(
			    CTX, corestring_dom_ತುಂಬು, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_ಗಂಟು_unref(body);
			/* ... handlers */
			duk_ತೆಗೆ(CTX);
			dukky_leave_thread(thread);
			ಹಿಂತಿರುಗಿ true;
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
	dukky_reset_ಆರಂಭ_ಸಮಯ(CTX);
	ಆದರೆ (duk_pcall_ವಿಧಾನ(CTX, 1) != 0) {
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
		      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(CTX, -5),
		      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(CTX, -4));
		NSLOG(dukky, DEBUG, "              was at: %s line %s",
		      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(CTX, -3),
		      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(CTX, -2));
		NSLOG(dukky, DEBUG, "         Stack trace: %s",
		      duk_ಸುರಕ್ಷಿತ_ಗಮ್ಯ_string(CTX, -1));

		duk_ತೆಗೆ_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		ಹಿಂತಿರುಗಿ true;
	}
	/* ... result */
	duk_ತೆಗೆ(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	ಹಿಂತಿರುಗಿ true;
}
