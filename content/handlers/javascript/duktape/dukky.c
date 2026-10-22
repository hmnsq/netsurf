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

#define EVENT_MAGIC MAGIC(EVENT_MAP)
#define HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#define HANDLER_MAGIC MAGIC(HANDLER_MAP)
#define EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#define GENERICS_MAGIC MAGIC(GENERICS_TABLE)
#define THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
संरचना jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t next_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	अचिन्हित पूर्णांक live_threads; /**< number of live threads */
	uint64_t exec_start_time;
};

/**
 * dukky javascript thread
 */
संरचना jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	अचिन्हित पूर्णांक in_use; /**< The number of times this thread is in use */
	jsheap *heap; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

स्थिर duk_ret_t dukky_populate_object(duk_context *ctx, शून्य *udata)
{
	/* ... obj args protoname nargs */
	पूर्णांक nargs = duk_get_int(ctx, -1);
	duk_काढा(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	जर (duk_is_undefined(ctx, -1)) {
		NSLOG(dukky, WARNING,
		      "Unable to find dukky prototype for `%s` - falling back to HTMLUnknownElement",
		      duk_get_string(ctx, -3) + 2 /* Skip the two unprintables */);
		duk_काढा(ctx);
		duk_ढकला_string(ctx, PROTO_NAME(HTMLUNKNOWNELEMENT));
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
	duk_घाला(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_काढा_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	परत 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, स्थिरांक अक्षर *name, पूर्णांक args)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, args);
	/* ... args */
	duk_ढकला_object(ctx);
	/* ... args obj */
	duk_ढकला_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
	/* ... args obj */
	duk_ढकला_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
	/* ... args obj */
	duk_घाला(ctx, -(args+1));
	/* ... obj args */
	duk_ढकला_string(ctx, name);
	/* ... obj args name */
	duk_ढकला_int(ctx, args);
	/* ... obj args name nargs */
	जर ((ret = duk_safe_call(ctx, dukky_populate_object, रिक्त, args + 3, 1))
	    != DUK_EXEC_SUCCESS)
		परत ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	परत DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_ढकला_गाठ_stacked(duk_context *ctx)
{
	पूर्णांक शीर्ष_at_fail = duk_get_शीर्ष(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, गाठ_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	जर (duk_is_undefined(ctx, -1)) {
		/* ... nodeptr klass nodes undefined */
		duk_काढा(ctx);
		/* ... nodeptr klass nodes */
		duk_ढकला_object(ctx);
		/* ... nodeptr klass nodes obj */
		duk_ढकला_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_ढकला_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr klass */
		duk_ढकला_int(ctx, 1);
		/* ... nodeptr klass nodes obj nodeptr klass 1 */
		जर (duk_safe_call(ctx, dukky_populate_object, रिक्त, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_set_शीर्ष(ctx, शीर्ष_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			परत false;
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
	duk_घाला(ctx, -4);
	/* ... node nodeptr klass nodes */
	duk_काढा_3(ctx);
	/* ... node */
	जर (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		स्थिरांक अक्षर * what = duk_safe_to_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_काढा(ctx);
	}
	परत true;
}

#define SET_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

स्थिर शून्य dukky_html_element_class_from_tag_type(dom_html_element_type type,
		स्थिरांक अक्षर **html_class, माप_प्रकार *html_class_len)
{
	निवड(type) {
	स्थिती DOM_HTML_ELEMENT_TYPE_HTML:
		SET_HTML_CLASS(HTML)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_HEAD:
		SET_HTML_CLASS(HEAD)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_META:
		SET_HTML_CLASS(META)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_BASE:
		SET_HTML_CLASS(BASE)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_TITLE:
		SET_HTML_CLASS(TITLE)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_BODY:
		SET_HTML_CLASS(BODY)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_DIV:
		SET_HTML_CLASS(DIV)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_FORM:
		SET_HTML_CLASS(FORM)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_LINK:
		SET_HTML_CLASS(LINK)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_BUTTON:
		SET_HTML_CLASS(BUTTON)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_आदान:
		SET_HTML_CLASS(आदान_2)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		SET_HTML_CLASS(TEXTAREA)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		SET_HTML_CLASS(OPTGROUP)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_OPTION:
		SET_HTML_CLASS(OPTION)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_SELECT:
		SET_HTML_CLASS(SELECT)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_HR:
		SET_HTML_CLASS(HR)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_DL:
		SET_HTML_CLASS(DLIST)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_DIR:
		SET_HTML_CLASS(DIRECTORY)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_MENU:
		SET_HTML_CLASS(MENU)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_FIELDSET:
		SET_HTML_CLASS(FIELDSET)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_LEGEND:
		SET_HTML_CLASS(LEGEND)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_P:
		SET_HTML_CLASS(PARAGRAPH)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_H1:
	स्थिती DOM_HTML_ELEMENT_TYPE_H2:
	स्थिती DOM_HTML_ELEMENT_TYPE_H3:
	स्थिती DOM_HTML_ELEMENT_TYPE_H4:
	स्थिती DOM_HTML_ELEMENT_TYPE_H5:
	स्थिती DOM_HTML_ELEMENT_TYPE_H6:
		SET_HTML_CLASS(HEADING)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	स्थिती DOM_HTML_ELEMENT_TYPE_Q:
		SET_HTML_CLASS(QUOTE)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_PRE:
		SET_HTML_CLASS(PRE)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_BR:
		SET_HTML_CLASS(BR)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_LABEL:
		SET_HTML_CLASS(LABEL)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_UL:
		SET_HTML_CLASS(ULIST)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_OL:
		SET_HTML_CLASS(OLIST)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_LI:
		SET_HTML_CLASS(LI)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_FONT:
		SET_HTML_CLASS(FONT)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_DEL:
	स्थिती DOM_HTML_ELEMENT_TYPE_INS:
		SET_HTML_CLASS(MOD)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_A:
		SET_HTML_CLASS(ANCHOR)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_BASEFONT:
		SET_HTML_CLASS(BASEFONT)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_IMG:
		SET_HTML_CLASS(IMAGE)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_OBJECT:
		SET_HTML_CLASS(OBJECT)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_PARAM:
		SET_HTML_CLASS(PARAM)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_APPLET:
		SET_HTML_CLASS(APPLET)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_MAP:
		SET_HTML_CLASS(MAP)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_AREA:
		SET_HTML_CLASS(AREA)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_SCRIPT:
		SET_HTML_CLASS(SCRIPT)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_CAPTION:
		SET_HTML_CLASS(TABLECAPTION)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_TD:
	स्थिती DOM_HTML_ELEMENT_TYPE_TH:
		SET_HTML_CLASS(TABLECELL)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_COL:
	स्थिती DOM_HTML_ELEMENT_TYPE_COLGROUP:
		SET_HTML_CLASS(TABLECOL)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_THEAD:
	स्थिती DOM_HTML_ELEMENT_TYPE_TBODY:
	स्थिती DOM_HTML_ELEMENT_TYPE_TFOOT:
		SET_HTML_CLASS(TABLESECTION)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_TABLE:
		SET_HTML_CLASS(TABLE)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_TR:
		SET_HTML_CLASS(TABLEROW)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_STYLE:
		SET_HTML_CLASS(STYLE)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_FRAMESET:
		SET_HTML_CLASS(FRAMESET)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_FRAME:
		SET_HTML_CLASS(FRAME)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_IFRAME:
		SET_HTML_CLASS(IFRAME)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_ISINDEX:
		SET_HTML_CLASS(ISINDEX)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_CANVAS:
		SET_HTML_CLASS(CANVAS)
		थांबा;
	स्थिती DOM_HTML_ELEMENT_TYPE_मोजणी:
		प्रतिज्ञा(type != DOM_HTML_ELEMENT_TYPE_मोजणी);
		fallthrough;
	स्थिती DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		SET_HTML_CLASS(UNKNOWN)
		थांबा;
	पूर्वनिर्धारित:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		थांबा;
	}
	परत;
}

#undef SET_HTML_CLASS

स्थिर शून्य
dukky_ढकला_गाठ_klass(duk_context *ctx, संरचना dom_गाठ *गाठ)
{
	dom_गाठ_type nodetype;
	dom_exception err;

	err = dom_गाठ_get_गाठ_type(गाठ, &nodetype);
	जर (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_ढकला_string(ctx, PROTO_NAME(गाठ_2));
		परत;
	}

	निवड(nodetype) {
	स्थिती DOM_ELEMENT_गाठ: {
		dom_string *namespace;
		dom_html_element_type type;
		स्थिरांक अक्षर *html_class;
		माप_प्रकार html_class_len;
		err = dom_गाठ_get_namespace(गाठ, &namespace);
		जर (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_ढकला_string(ctx, PROTO_NAME(ELEMENT));
			थांबा;
		}
		जर (namespace == रिक्त) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_ढकला_string(ctx, PROTO_NAME(ELEMENT));
			थांबा;
		}

		जर (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_ढकला_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			थांबा;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_tag_type(गाठ, &type);
		जर (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_tag_type(type,
				&html_class, &html_class_len);

		duk_ढकला_lstring(ctx, html_class, html_class_len);
		थांबा;
	}
	स्थिती DOM_TEXT_गाठ:
		duk_ढकला_string(ctx, PROTO_NAME(TEXT));
		थांबा;
	स्थिती DOM_COMMENT_गाठ:
		duk_ढकला_string(ctx, PROTO_NAME(COMMENT));
		थांबा;
	स्थिती DOM_DOCUMENT_गाठ:
		duk_ढकला_string(ctx, PROTO_NAME(DOCUMENT));
		थांबा;
	स्थिती DOM_ATTRIBUTE_गाठ:
	स्थिती DOM_PROCESSING_INSTRUCTION_गाठ:
	स्थिती DOM_DOCUMENT_TYPE_गाठ:
	स्थिती DOM_DOCUMENT_FRAGMENT_गाठ:
	स्थिती DOM_NOTATION_गाठ:
	स्थिती DOM_ENTITY_REFERENCE_गाठ:
	स्थिती DOM_ENTITY_गाठ:
	स्थिती DOM_CDATA_SECTION_गाठ:
	पूर्वनिर्धारित:
		/* Oh bum, just node then */
		duk_ढकला_string(ctx, PROTO_NAME(गाठ_2));
	}
}

duk_bool_t
dukky_ढकला_गाठ(duk_context *ctx, संरचना dom_गाठ *गाठ)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", गाठ);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, गाठ_MAGIC);
	/* ... nodes */
	duk_ढकला_pointer(ctx, गाठ);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	जर (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_घाला(ctx, -2);
		/* ... node nodes */
		duk_काढा(ctx);
		/* ... node */
		जर (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			स्थिरांक अक्षर * what = duk_safe_to_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_काढा(ctx);
		}
		परत true;
	}
	/* ... nodes undefined */
	duk_काढा_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_ढकला_pointer(ctx, गाठ);
	/* ... nodeptr */
	dukky_ढकला_गाठ_klass(ctx, गाठ);
	/* ... nodeptr klass */
	परत dukky_ढकला_गाठ_stacked(ctx);
}

स्थिर duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	परत duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

शून्य
dukky_inject_not_ctr(duk_context *ctx, पूर्णांक idx, स्थिरांक अक्षर *name)
{
	/* ... p[idx] ... proto */
	duk_ढकला_c_function(ctx, dukky_bad_constructor, 0);
	/* ... p[idx] ... proto cons */
	duk_घाला(ctx, -2);
	/* ... p[idx] ... cons proto */
	duk_put_prop_string(ctx, -2, "prototype");
	/* ... p[idx] ... cons[proto] */
	duk_put_prop_string(ctx, idx, name);
	/* ... p ... */
	परत;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

स्थिर शून्य *dukky_alloc_function(शून्य *udata, duk_size_t size)
{
	जर (size == 0)
		परत रिक्त;

	परत दो_स्मृति(size);
}

स्थिर शून्य *dukky_realloc_function(शून्य *udata, शून्य *ptr, duk_size_t size)
{
	जर (ptr == रिक्त && size == 0)
		परत रिक्त;

	जर (size == 0) {
		मुक्त(ptr);
		परत रिक्त;
	}

	परत पुनः_स्मृति(ptr, size);
}


स्थिर शून्य dukky_free_function(शून्य *udata, शून्य *ptr)
{
	जर (ptr != रिक्त)
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
js_newheap(पूर्णांक timeout, jsheap **heap)
{
	duk_context *ctx;
	jsheap *ret = सुस्मृति(1, आकार(*ret));
	*heap = रिक्त;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	जर (ret == रिक्त) परत NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_heap(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		रिक्त);
	जर (ret->ctx == रिक्त) { मुक्त(ret); परत NSERROR_NOMEM; }
	/* Create the prototype stuffs */
	duk_ढकला_global_object(ctx);
	duk_ढकला_boolean(ctx, true);
	duk_put_prop_string(ctx, -2, "protos");
	duk_put_global_string(ctx, PROTO_MAGIC);
	/* Create prototypes here */
	dukky_create_prototypes(ctx);
	/* Now create the thread map */
	duk_ढकला_object(ctx);
	duk_put_global_string(ctx, THREAD_MAP);

	*heap = ret;
	परत NSERROR_OK;
}


स्थिर शून्य dukky_destroyheap(jsheap *heap)
{
	प्रतिज्ञा(heap->pending_destroy == true);
	प्रतिज्ञा(heap->live_threads == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_heap(heap->ctx);
	मुक्त(heap);
}

/* exported interface documented in js.h */
शून्य js_destroyheap(jsheap *heap)
{
	heap->pending_destroy = true;
	जर (heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* Just for here, the CTX is in ret, not thread */
#define CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *heap, शून्य *win_priv, शून्य *doc_priv, jsthread **thread)
{
	jsthread *ret;
	प्रतिज्ञा(heap != रिक्त);
	प्रतिज्ञा(heap->pending_destroy == false);

	ret = सुस्मृति(1, आकार (*ret));
	जर (ret == रिक्त) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		परत NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_ढकला_thread(heap->ctx); /* ... threads thread */
	ret->heap = heap;
	ret->ctx = duk_require_context(heap->ctx, -1);
	ret->thread_idx = heap->next_thread++;
	duk_put_prop_index(heap->ctx, -2, ret->thread_idx);
	heap->live_threads++;
	duk_काढा(heap->ctx); /* ... */
	duk_ढकला_int(CTX, 0);
	duk_ढकला_int(CTX, 1);
	duk_ढकला_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_ढकला_pointer(CTX, win_priv);
	duk_ढकला_pointer(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(WINDOW), 2);
	duk_ढकला_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_set_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_ढकला_object(CTX);
	duk_ढकला_pointer(CTX, रिक्त);
	duk_ढकला_null(CTX);
	duk_put_prop(CTX, -3);
	duk_put_global_string(CTX, गाठ_MAGIC);

	/* And now the event mapping table */
	duk_ढकला_object(CTX);
	duk_put_global_string(CTX, EVENT_MAGIC);

	/* Now load the polyfills */
	/* ... */
	duk_ढकला_string(CTX, "polyfill.js");
	/* ..., polyfill.js */
	जर (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (स्थिरांक अक्षर *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		परत NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	जर (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		परत NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_काढा(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_ढकला_string(CTX, "generics.js");
	/* ..., generics.js */
	जर (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (स्थिरांक अक्षर *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		परत NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	जर (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		परत NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_काढा(CTX);
	/* ... */
	duk_ढकला_global_object(CTX);
	/* ..., Win */
	duk_get_prop_string(CTX, -1, "NetSurf");
	/* ..., Win, NetSurf */
	duk_put_global_string(CTX, GENERICS_MAGIC);
	/* ..., Win */
	duk_del_prop_string(CTX, -1, "NetSurf");
	duk_काढा(CTX);
	/* ... */

	dukky_log_चळत_frame(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, heap);
	*thread = ret;

	परत NSERROR_OK;
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
	duk_int_t शीर्ष = duk_get_शीर्ष(CTX);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, thread->heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Restore whatever stack we had */
	duk_set_शीर्ष(CTX, शीर्ष);

	परत NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
स्थिर शून्य dukky_destroythread(jsthread *thread)
{
	jsheap *heap = thread->heap;

	प्रतिज्ञा(thread->in_use == 0);
	प्रतिज्ञा(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_index(heap->ctx, -1, thread->thread_idx);
	duk_काढा(heap->ctx); /* ... */

	/* We can now free the thread object */
	मुक्त(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(heap->ctx, 0);
	duk_gc(heap->ctx, DUK_GC_COMPACT);
	heap->live_threads--;

	/* And if the heap should now go, blow it away */
	जर (heap->pending_destroy == true && heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* exported interface documented in js.h */
शून्य js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	जर (thread->in_use == 0) {
		dukky_destroythread(thread);
	}
}

स्थिर शून्य dukky_enter_thread(jsthread *thread)
{
	प्रतिज्ञा(thread != रिक्त);
	thread->in_use++;
}

स्थिर शून्य dukky_leave_thread(jsthread *thread)
{
	प्रतिज्ञा(thread != रिक्त);
	प्रतिज्ञा(thread->in_use > 0);

	thread->in_use--;
	जर (thread->in_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(शून्य *udata)
{
#define JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *heap = (jsheap *) udata;
	uint64_t now;

	(शून्य) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	परत heap->exec_start_time != 0 &&
			now > (heap->exec_start_time + JS_EXEC_TIMEOUT_MS);
}

स्थिर शून्य dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_शीर्ष(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_safe_to_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_काढा(ctx);
	/* ..., errobj */
}

स्थिर शून्य dukky_reset_start_time(duk_context *ctx)
{
	duk_memory_functions funcs;
	jsheap *heap;
	duk_get_memory_functions(ctx, &funcs);
	heap = funcs.udata;
	(शून्य) nsu_getmonotonic_ms(&heap->exec_start_time);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_size_t argc, bool reset_timeout)
{
	जर (reset_timeout) {
		dukky_reset_start_time(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	जर (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	परत ret;
}


शून्य dukky_ढकला_generics(duk_context *ctx, स्थिरांक अक्षर *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

स्थिर duk_int_t dukky_ढकला_context_dump(duk_context *ctx, शून्य *udata)
{
	duk_ढकला_context_dump(ctx);
	परत 1;
}

शून्य dukky_log_चळत_frame(duk_context *ctx, स्थिरांक अक्षर * reason)
{
	जर (duk_safe_call(ctx, dukky_ढकला_context_dump, रिक्त, 0, 1) != 0) {
		duk_काढा(ctx);
		duk_ढकला_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_safe_to_string(ctx, -1));
	duk_काढा(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, स्थिरांक uint8_t *txt, माप_प्रकार txtlen, स्थिरांक अक्षर *name)
{
	bool ret = false;
	प्रतिज्ञा(thread);

	जर (txt == रिक्त || txtlen == 0) {
		परत false;
	}

	जर (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		परत false;
	}

	dukky_enter_thread(thread);

	duk_set_शीर्ष(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_start_time(CTX);
	जर (name != रिक्त) {
		duk_ढकला_string(CTX, name);
	} अन्यथा {
		duk_ढकला_string(CTX, "?unknown source?");
	}
	जर (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (स्थिरांक अक्षर *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		जा handle_error;
	}

	जर (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		जा handle_error;
	}

	जर (duk_get_शीर्ष(CTX) == 0) duk_ढकला_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	जा out;

handle_error:
	dukky_dump_error(CTX);
out:
	dukky_leave_thread(thread);
	परत ret;
}

स्थिर स्थिरांक अक्षर* dukky_event_proto(dom_event *evt)
{
	स्थिरांक अक्षर *ret = PROTO_NAME(EVENT);
	dom_string *type = रिक्त;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	जर (err != DOM_NO_ERR) {
		जा out;
	}

	जर (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		जा out;
	} अन्यथा जर (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		जा out;
	} अन्यथा जर (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		जा out;
	}

out:
	जर (type != रिक्त) {
		dom_string_unref(type);
	}

	परत ret;
}

/*** New style event handling ***/

शून्य dukky_ढकला_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_ढकला_pointer(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	जर (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_काढा(ctx);
		/* ... events */
		duk_ढकला_pointer(ctx, evt);
		जर (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_काढा(ctx);
			/* ... events */
			duk_ढकला_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_ढकला_pointer(ctx, evt);
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

स्थिर शून्य dukky_ढकला_handler_code(duk_context *ctx, dom_string *name,
				     dom_event_target *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_गाठ_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	जर (et == रिक्त) {
		duk_ढकला_lstring(ctx, "", 0);
		परत;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_गाठ_get_गाठ_type(et, &ntype);
	जर (exc != DOM_NO_ERR) {
		duk_ढकला_lstring(ctx, "", 0);
		परत;
	}

	जर (ntype != DOM_ELEMENT_गाठ) {
		duk_ढकला_lstring(ctx, "", 0);
		परत;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	जर (exc != DOM_NO_ERR) {
		duk_ढकला_lstring(ctx, "", 0);
		परत;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	जर ((exc != DOM_NO_ERR) || (val == रिक्त)) {
		dom_string_unref(onname);
		duk_ढकला_lstring(ctx, "", 0);
		परत;
	}

	dom_string_unref(onname);
	duk_ढकला_lstring(ctx, dom_string_data(val), dom_string_length(val));
	dom_string_unref(val);
}

bool dukky_get_current_मूल्य_of_event_handler(duk_context *ctx,
					      dom_string *name,
					      dom_event_target *et)
{
	/* Must be entered as:
	 * ... node(et)
	 */
	duk_get_prop_string(ctx, -1, HANDLER_MAGIC);
	/* ... node handlers */
	duk_ढकला_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	जर (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_काढा_2(ctx);
		/* ... node */
		dukky_ढकला_handler_code(ctx, name, et);
		/* ... node handlercode? */
		/* TODO: If this is null, clean up and propagate */
		/* ... node handlercode */
		/** @todo This is entirely wrong, but it's hard to get right */
		duk_ढकला_string(ctx, "function (event) {");
		/* ... node handlercode prefix */
		duk_घाला(ctx, -2);
		/* ... node prefix handlercode */
		duk_ढकला_string(ctx, "}");
		/* ... node prefix handlercode suffix */
		duk_concat(ctx, 3);
		/* ... node fullhandlersrc */
		duk_ढकला_string(ctx, "internal raw uncompiled handler");
		/* ... node fullhandlersrc filename */
		जर (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_काढा_2(ctx);
			परत false;
		}
		/* ... node handler */
		duk_घाला(ctx, -2);
		/* ... handler node */
	} अन्यथा {
		/* ... node handlers handler */
		duk_घाला(ctx, -3);
		/* ... handler node handlers */
		duk_काढा(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	परत true;
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
	जर (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		परत;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (पूर्णांक)dom_string_length(name),
	      dom_string_data(name));
	exc = dom_event_get_event_phase(evt, &phase);
	जर (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		परत;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_TARGET ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (पूर्णांक)phase);

	exc = dom_event_get_current_target(evt, &targ);
	जर (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		परत;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	जर (phase == DOM_CAPTURING_PHASE)
		जा handle_extras;

	/* ... */
	जर (dukky_ढकला_गाठ(ctx, (dom_गाठ *)targ) == false) {
		dom_string_unref(name);
		dom_गाठ_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		परत;
	}
	/* ... node */
	जर (dukky_get_current_मूल्य_of_event_handler(
		    ctx, name, (dom_event_target *)targ) == false) {
		/* ... */
		जा handle_extras;
	}
	/* ... handler node */
	dukky_ढकला_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_start_time(ctx);
	जर (duk_pcall_method(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_immediate_propagation(evt);
		जर (exc != DOM_NO_ERR)
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

		duk_काढा_n(ctx, 6);
		/* ... */
		जा handle_extras;
	}
	/* ... result */
	जर (duk_is_boolean(ctx, -1) &&
	    duk_to_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_काढा(ctx);
handle_extras:
	/* ... */
	duk_ढकला_lstring(ctx, dom_string_data(name), dom_string_length(name));
	dukky_ढकला_गाठ(ctx, (dom_गाठ *)targ);
	/* ... type node */
	जर (dukky_event_target_ढकला_listeners(ctx, true)) {
		/* Nothing to do */
		duk_काढा(ctx);
		जा out;
	}
	/* ... sublisteners */
	duk_ढकला_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	पर्यंत (duk_get_prop_index(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_index(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		जर ((event_listener_flags)duk_to_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_काढा(ctx);
			/* ... subl copy handler flags */
		}
		duk_काढा(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_index(ctx, -2, idx);
		/* ... sublisteners copy */
		idx++;
	}
	/* ... sublisteners copy undefined */
	duk_काढा(ctx);
	/* ... sublisteners copy */
	duk_घाला(ctx, -2);
	/* ... copy sublisteners */
	duk_काढा(ctx);
	/* ... copy */
	idx = 0;
	पर्यंत (duk_get_prop_index(ctx, -1, idx++)) {
		/* ... copy handler */
		जर (duk_get_prop_index(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_काढा_2(ctx);
			सुरूठेवा;
		}
		duk_काढा(ctx);
		duk_get_prop_index(ctx, -1, 0);
		duk_get_prop_index(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_काढा(ctx);
		/* ... copy handler callback */
		जर (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_काढा_2(ctx);
			/* ... copy */
			सुरूठेवा;
		}
		/* ... copy handler callback */
		dukky_ढकला_गाठ(ctx, (dom_गाठ *)targ);
		/* ... copy handler callback node */
		dukky_ढकला_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_start_time(ctx);
		जर (duk_pcall_method(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_immediate_propagation(evt);
			जर (exc != DOM_NO_ERR)
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

			duk_काढा_n(ctx, 7);
			/* ... copy */
			सुरूठेवा;
		}
		/* ... copy handler result */
		जर (duk_is_boolean(ctx, -1) &&
		    duk_to_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_काढा_2(ctx);
		/* ... copy */
	}
	duk_काढा_2(ctx);
out:
	/* ... */
	dom_गाठ_unref(targ);
	dom_string_unref(name);
}

शून्य dukky_register_event_listener_for(duk_context *ctx,
				       संरचना dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = रिक्त;
	dom_exception exc;

	/* ... */
	जर (ele == रिक्त) {
		/* A null element is the Window object */
		duk_ढकला_global_object(ctx);
	} अन्यथा {
		/* Non null elements must be pushed as a node object */
		जर (dukky_ढकला_गाठ(ctx, (संरचना dom_गाठ *)ele) == false)
			परत;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_ढकला_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	जर (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_काढा_2(ctx);
		/* ... */
		परत;
	}
	/* ... node handlers */
	duk_ढकला_lstring(ctx, dom_string_data(name), dom_string_length(name));
	/* ... node handlers name */
	duk_ढकला_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_काढा_2(ctx);
	/* ... */
	जर (ele == रिक्त) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		परत;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	जर (exc != DOM_NO_ERR) परत;
	exc = dom_event_target_add_event_listener(
		ele, name, listen, capture);
	जर (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (पूर्णांक)dom_string_length(name), dom_string_data(name));
	} अन्यथा {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (पूर्णांक)dom_string_length(name), dom_string_data(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_target_ढकला_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	जर (duk_is_undefined(ctx, -1)) {
		/* ... type this null */
		duk_काढा(ctx);
		duk_ढकला_object(ctx);
		duk_dup(ctx, -1);
		/* ... type this listeners listeners */
		duk_put_prop_string(ctx, -3, EVENT_LISTENER_JS_MAGIC);
		/* ... type this listeners */
	}
	/* ... type this listeners */
	duk_घाला(ctx, -3);
	/* ... listeners type this */
	duk_काढा(ctx);
	/* ... listeners type */
	duk_dup(ctx, -1);
	/* ... listeners type type */
	duk_get_prop(ctx, -3);
	/* ... listeners type ??? */
	जर (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		जर (dont_create == true) {
			duk_काढा_3(ctx);
			duk_ढकला_undefined(ctx);
			परत true;
		}
		duk_काढा(ctx);
		duk_ढकला_array(ctx);
		duk_dup(ctx, -2);
		duk_dup(ctx, -2);
		/* ... listeners type sublisteners type sublisteners */
		duk_put_prop(ctx, -5);
		/* ... listeners type sublisteners */
		ret = true;
	}
	duk_घाला(ctx, -3);
	/* ... sublisteners listeners type */
	duk_काढा_2(ctx);
	/* ... sublisteners */
	परत ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
शून्य dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	पर्यंत (duk_get_prop_index(ctx, -1, idx + 1)) {
		duk_put_prop_index(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_index(ctx, -2, idx + 1);
	duk_काढा(ctx);
}


शून्य js_handle_new_element(jsthread *thread, संरचना dom_element *गाठ)
{
	प्रतिज्ञा(thread);
	प्रतिज्ञा(गाठ);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = रिक्त;
	dom_string *किल्ली = रिक्त;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_गाठ_get_गाठ_name(गाठ, &nodename);
	जर (exc != DOM_NO_ERR) परत;

	जर (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_गाठ_get_attributes(गाठ, &map);
	जर (exc != DOM_NO_ERR) परत;
	जर (map == रिक्त) परत;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_length(map, &siz);
	जर (exc != DOM_NO_ERR) जा out;

	साठी (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_item(map, idx, &attr);
		जर (exc != DOM_NO_ERR) जा out;
		exc = dom_attr_get_name(attr, &किल्ली);
		जर (exc != DOM_NO_ERR) जा out;
		जर (is_body && (
			    किल्ली == corestring_dom_onblur ||
			    किल्ली == corestring_dom_onerror ||
			    किल्ली == corestring_dom_onfocus ||
			    किल्ली == corestring_dom_onload ||
			    किल्ली == corestring_dom_onresize ||
			    किल्ली == corestring_dom_onscroll)) {
			/* This is a forwarded event, it doesn't matter,
			 * we should skip registering for it and later
			 * we will register it for Window itself
			 */
			जा skip_register;
		}
		जर (dom_string_length(किल्ली) > 2) {
			/* Can be on* */
			स्थिरांक uint8_t *data = (स्थिरांक uint8_t *)dom_string_data(किल्ली);
			जर (data[0] == 'o' && data[1] == 'n') {
				dom_string *sub = रिक्त;
				exc = dom_string_substr(
					किल्ली, 2, dom_string_length(किल्ली),
					&sub);
				जर (exc == DOM_NO_ERR) {
					dukky_register_event_listener_for(
						CTX, गाठ, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_register:
		dom_string_unref(किल्ली); किल्ली = रिक्त;
		dom_गाठ_unref(attr); attr = रिक्त;
	}

out:
	जर (किल्ली != रिक्त)
		dom_string_unref(किल्ली);

	जर (attr != रिक्त)
		dom_गाठ_unref(attr);

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
	duk_ढकला_pointer(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_काढा(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, स्थिरांक अक्षर *type, संरचना dom_document *doc, संरचना dom_गाठ *target)
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

	जर (target != रिक्त)
		/* Swallow non-Window-targetted events quietly */
		परत true;

	जर (म_भेद(type, "load") != 0)
		/* Swallow non-load events quietly */
		परत true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	जर (exc != DOM_NO_ERR) परत true;
	exc = dom_event_init(evt, corestring_dom_load, false, false);
	जर (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		परत true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_ढकला_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	जर (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_काढा(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		जर (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			परत true;
		}
		dukky_ढकला_गाठ(CTX, (संरचना dom_गाठ *)body);
		/* ... handlers bodynode */
		जर (dukky_get_current_मूल्य_of_event_handler(
			    CTX, corestring_dom_load, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_गाठ_unref(body);
			/* ... handlers */
			duk_काढा(CTX);
			dukky_leave_thread(thread);
			परत true;
		}
		/* Unref the body, we don't need it any more */
		dom_गाठ_unref(body);
		/* ... handlers handler bodynode */
		duk_काढा(CTX);
	}
	/* ... handlers handler */
	duk_घाला(CTX, -2);
	/* ... handler handlers */
	duk_काढा(CTX);
	/* ... handler */
	duk_ढकला_global_object(CTX);
	/* ... handler Window */
	dukky_ढकला_event(CTX, evt);
	/* ... handler Window event */
	dukky_reset_start_time(CTX);
	जर (duk_pcall_method(CTX, 1) != 0) {
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

		duk_काढा_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		परत true;
	}
	/* ... result */
	duk_काढा(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	परत true;
}
