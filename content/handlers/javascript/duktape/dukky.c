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

#समावेश <stdint.h>
#समावेश <nsutils/time.h>

#समावेश "netsurf/inttypes.h"
#समावेश "utils/utils.h"
#समावेश "utils/nsoption.h"
#समावेश "utils/log.h"
#समावेश "utils/corestrings.h"
#समावेश "content/content.h"

#समावेश "javascript/js.h"
#समावेश "javascript/content.h"

#समावेश "duktape/binding.h"
#समावेश "duktape/generics.js.inc"
#समावेश "duktape/polyfill.js.inc"

#समावेश "duktape.h"
#समावेश "dukky.h"

#समावेश <dom/dom.h>

#घोषणा EVENT_MAGIC MAGIC(EVENT_MAP)
#घोषणा HANDLER_LISTENER_MAGIC MAGIC(HANDLER_LISTENER_MAP)
#घोषणा HANDLER_MAGIC MAGIC(HANDLER_MAP)
#घोषणा EVENT_LISTENER_JS_MAGIC MAGIC(EVENT_LISTENER_JS_MAP)
#घोषणा GENERICS_MAGIC MAGIC(GENERICS_TABLE)
#घोषणा THREAD_MAP MAGIC(THREAD_MAP)

/**
 * dukky javascript heap
 */
काष्ठा jsheap {
	duk_context *ctx; /**< duktape base context */
	duk_uarridx_t अग्र_thread; /**< monotonic thread counter */
	bool pending_destroy; /**< Whether this heap is pending destruction */
	अचिन्हित पूर्णांक live_threads; /**< number of live threads */
	uint64_t exec_start_time;
};

/**
 * dukky javascript thread
 */
काष्ठा jsthread {
	bool pending_destroy; /**< Whether this thread is pending destruction */
	अचिन्हित पूर्णांक in_use; /**< The number of times this thread is in use */
	jsheap *heap; /**< The heap this thread belongs to */
	duk_context *ctx; /**< The duktape thread context */
	duk_uarridx_t thread_idx; /**< The thread number */
};

अटल duk_ret_t dukky_populate_object(duk_context *ctx, व्योम *udata)
{
	/* ... obj args protoname nargs */
	पूर्णांक nargs = duk_get_int(ctx, -1);
	duk_निकालो(ctx);
	/* ... obj args protoname */
	duk_get_global_string(ctx, PROTO_MAGIC);
	/* .. obj args protoname prototab */
	duk_dup(ctx, -2);
	/* ... obj args protoname prototab protoname */
	duk_get_prop(ctx, -2);
	/* ... obj args protoname prototab {proto/undefined} */
	अगर (duk_is_undefined(ctx, -1)) {
		NSLOG(dukky, WARNING,
		      "Unable to find dukky prototype for `%s` - falling back to HTMLUnknownElement",
		      duk_get_string(ctx, -3) + 2 /* Skip the two unprintables */);
		duk_निकालो(ctx);
		duk_धकेलो_string(ctx, PROTO_NAME(HTMLUNKNOWNELEMENT));
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
	duk_जोड़ो(ctx, -(nargs+4));
	/* ... initfn obj[proto] args prototab proto */
	duk_निकालो_2(ctx);
	/* ... initfn obj[proto] args */
	NSLOG(dukky, DEEPDEBUG, "Call the init function");
	duk_call(ctx, nargs + 1);
	वापस 1; /* The object */
}

duk_ret_t dukky_create_object(duk_context *ctx, स्थिर अक्षर *name, पूर्णांक args)
{
	duk_ret_t ret;
	NSLOG(dukky, DEEPDEBUG, "name=%s nargs=%d", name + 2, args);
	/* ... args */
	duk_धकेलो_object(ctx);
	/* ... args obj */
	duk_धकेलो_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
	/* ... args obj */
	duk_धकेलो_object(ctx);
	/* ... args obj handlers */
	duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
	/* ... args obj */
	duk_जोड़ो(ctx, -(args+1));
	/* ... obj args */
	duk_धकेलो_string(ctx, name);
	/* ... obj args name */
	duk_धकेलो_int(ctx, args);
	/* ... obj args name nargs */
	अगर ((ret = duk_safe_call(ctx, dukky_populate_object, शून्य, args + 3, 1))
	    != DUK_EXEC_SUCCESS)
		वापस ret;
	NSLOG(dukky, DEEPDEBUG, "created");
	वापस DUK_EXEC_SUCCESS;
}



duk_bool_t
dukky_धकेलो_गाँठ_stacked(duk_context *ctx)
{
	पूर्णांक शीर्ष_at_fail = duk_get_शीर्ष(ctx) - 2;
	/* ... nodeptr klass */
	duk_get_global_string(ctx, गाँठ_MAGIC);
	/* ... nodeptr klass nodes */
	duk_dup(ctx, -3);
	/* ... nodeptr klass nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodeptr klass nodes node/undefined */
	अगर (duk_is_undefined(ctx, -1)) {
		/* ... nodeptr klass nodes undefined */
		duk_निकालो(ctx);
		/* ... nodeptr klass nodes */
		duk_धकेलो_object(ctx);
		/* ... nodeptr klass nodes obj */
		duk_धकेलो_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_LISTENER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_धकेलो_object(ctx);
		/* ... nodeptr klass nodes obj handlers */
		duk_put_prop_string(ctx, -2, HANDLER_MAGIC);
		/* ... nodeptr klass nodes obj */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr */
		duk_dup(ctx, -4);
		/* ... nodeptr klass nodes obj nodeptr klass */
		duk_धकेलो_int(ctx, 1);
		/* ... nodeptr klass nodes obj nodeptr klass 1 */
		अगर (duk_safe_call(ctx, dukky_populate_object, शून्य, 4, 1)
		    != DUK_EXEC_SUCCESS) {
			duk_set_शीर्ष(ctx, शीर्ष_at_fail);
			NSLOG(dukky, ERROR, "Failed to populate object prototype");
			वापस false;
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
	duk_जोड़ो(ctx, -4);
	/* ... node nodeptr klass nodes */
	duk_निकालो_3(ctx);
	/* ... node */
	अगर (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
		duk_dup(ctx, -1);
		स्थिर अक्षर * what = duk_safe_to_string(ctx, -1);
		NSLOG(dukky, DEEPDEBUG, "Created: %s", what);
		duk_निकालो(ctx);
	}
	वापस true;
}

#घोषणा SET_HTML_CLASS(CLASS) \
		*html_class = PROTO_NAME(HTML##CLASS##ELEMENT); \
		*html_class_len = \
				SLEN(PROTO_NAME(HTML)) + \
				SLEN(#CLASS) + \
				SLEN("ELEMENT");

अटल व्योम dukky_html_element_class_from_tag_type(dom_html_element_type type,
		स्थिर अक्षर **html_class, माप_प्रकार *html_class_len)
{
	चयन(type) {
	हाल DOM_HTML_ELEMENT_TYPE_HTML:
		SET_HTML_CLASS(HTML)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_शीर्ष:
		SET_HTML_CLASS(शीर्ष_3)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_META:
		SET_HTML_CLASS(META)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_BASE:
		SET_HTML_CLASS(BASE)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_TITLE:
		SET_HTML_CLASS(TITLE)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_BODY:
		SET_HTML_CLASS(BODY)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_DIV:
		SET_HTML_CLASS(DIV)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_FORM:
		SET_HTML_CLASS(FORM)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_LINK:
		SET_HTML_CLASS(LINK)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_BUTTON:
		SET_HTML_CLASS(BUTTON)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_निवेश:
		SET_HTML_CLASS(निवेश_2)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		SET_HTML_CLASS(TEXTAREA)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_OPTGROUP:
		SET_HTML_CLASS(OPTGROUP)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_OPTION:
		SET_HTML_CLASS(OPTION)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_SELECT:
		SET_HTML_CLASS(SELECT)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_HR:
		SET_HTML_CLASS(HR)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_DL:
		SET_HTML_CLASS(DLIST)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_DIR:
		SET_HTML_CLASS(DIRECTORY)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_MENU:
		SET_HTML_CLASS(MENU)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_FIELDSET:
		SET_HTML_CLASS(FIELDSET)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_LEGEND:
		SET_HTML_CLASS(LEGEND)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_P:
		SET_HTML_CLASS(PARAGRAPH)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_H1:
	हाल DOM_HTML_ELEMENT_TYPE_H2:
	हाल DOM_HTML_ELEMENT_TYPE_H3:
	हाल DOM_HTML_ELEMENT_TYPE_H4:
	हाल DOM_HTML_ELEMENT_TYPE_H5:
	हाल DOM_HTML_ELEMENT_TYPE_H6:
		SET_HTML_CLASS(HEADING)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_BLOCKQUOTE:
	हाल DOM_HTML_ELEMENT_TYPE_Q:
		SET_HTML_CLASS(QUOTE)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_PRE:
		SET_HTML_CLASS(PRE)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_BR:
		SET_HTML_CLASS(BR)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_LABEL:
		SET_HTML_CLASS(LABEL)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_UL:
		SET_HTML_CLASS(ULIST)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_OL:
		SET_HTML_CLASS(OLIST)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_LI:
		SET_HTML_CLASS(LI)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_FONT:
		SET_HTML_CLASS(FONT)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_DEL:
	हाल DOM_HTML_ELEMENT_TYPE_INS:
		SET_HTML_CLASS(MOD)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_A:
		SET_HTML_CLASS(ANCHOR)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_BASEFONT:
		SET_HTML_CLASS(BASEFONT)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_IMG:
		SET_HTML_CLASS(IMAGE)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_OBJECT:
		SET_HTML_CLASS(OBJECT)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_PARAM:
		SET_HTML_CLASS(PARAM)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_APPLET:
		SET_HTML_CLASS(APPLET)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_MAP:
		SET_HTML_CLASS(MAP)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_AREA:
		SET_HTML_CLASS(AREA)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_SCRIPT:
		SET_HTML_CLASS(SCRIPT)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_CAPTION:
		SET_HTML_CLASS(TABLECAPTION)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_TD:
	हाल DOM_HTML_ELEMENT_TYPE_TH:
		SET_HTML_CLASS(TABLECELL)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_COL:
	हाल DOM_HTML_ELEMENT_TYPE_COLGROUP:
		SET_HTML_CLASS(TABLECOL)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_THEAD:
	हाल DOM_HTML_ELEMENT_TYPE_TBODY:
	हाल DOM_HTML_ELEMENT_TYPE_TFOOT:
		SET_HTML_CLASS(TABLESECTION)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_TABLE:
		SET_HTML_CLASS(TABLE)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_TR:
		SET_HTML_CLASS(TABLEROW)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_STYLE:
		SET_HTML_CLASS(STYLE)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_FRAMESET:
		SET_HTML_CLASS(FRAMESET)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_FRAME:
		SET_HTML_CLASS(FRAME)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_IFRAME:
		SET_HTML_CLASS(IFRAME)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_ISINDEX:
		SET_HTML_CLASS(ISINDEX)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_CANVAS:
		SET_HTML_CLASS(CANVAS)
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_गिनती:
		निश्चित(type != DOM_HTML_ELEMENT_TYPE_गिनती);
		fallthrough;
	हाल DOM_HTML_ELEMENT_TYPE__UNKNOWN:
		SET_HTML_CLASS(UNKNOWN)
		अवरोध;
	शेष:
		/* Known HTML element without a specialisation */
		*html_class = PROTO_NAME(HTMLELEMENT);
		*html_class_len =
				SLEN(PROTO_NAME(HTML)) +
				SLEN("ELEMENT");
		अवरोध;
	}
	वापस;
}

#अघोषित SET_HTML_CLASS

अटल व्योम
dukky_धकेलो_गाँठ_klass(duk_context *ctx, काष्ठा dom_गाँठ *गाँठ)
{
	dom_गाँठ_type nodetype;
	dom_exception err;

	err = dom_गाँठ_get_गाँठ_type(गाँठ, &nodetype);
	अगर (err != DOM_NO_ERR) {
		/* Oh bum, just node then */
		duk_धकेलो_string(ctx, PROTO_NAME(गाँठ_2));
		वापस;
	}

	चयन(nodetype) {
	हाल DOM_ELEMENT_गाँठ: {
		dom_string *namespace;
		dom_html_element_type type;
		स्थिर अक्षर *html_class;
		माप_प्रकार html_class_len;
		err = dom_गाँठ_get_namespace(गाँठ, &namespace);
		अगर (err != DOM_NO_ERR) {
			/* Feck it, element */
			NSLOG(dukky, ERROR,
			      "dom_node_get_namespace() failed");
			duk_धकेलो_string(ctx, PROTO_NAME(ELEMENT));
			अवरोध;
		}
		अगर (namespace == शून्य) {
			/* No namespace, -> element */
			NSLOG(dukky, DEBUG, "no namespace");
			duk_धकेलो_string(ctx, PROTO_NAME(ELEMENT));
			अवरोध;
		}

		अगर (dom_string_isequal(namespace, corestring_dom_html_namespace) == false) {
			/* definitely not an HTML element of some kind */
			duk_धकेलो_string(ctx, PROTO_NAME(ELEMENT));
			dom_string_unref(namespace);
			अवरोध;
		}
		dom_string_unref(namespace);

		err = dom_html_element_get_tag_type(गाँठ, &type);
		अगर (err != DOM_NO_ERR) {
			type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		dukky_html_element_class_from_tag_type(type,
				&html_class, &html_class_len);

		duk_धकेलो_lstring(ctx, html_class, html_class_len);
		अवरोध;
	}
	हाल DOM_TEXT_गाँठ:
		duk_धकेलो_string(ctx, PROTO_NAME(TEXT));
		अवरोध;
	हाल DOM_COMMENT_गाँठ:
		duk_धकेलो_string(ctx, PROTO_NAME(COMMENT));
		अवरोध;
	हाल DOM_DOCUMENT_गाँठ:
		duk_धकेलो_string(ctx, PROTO_NAME(DOCUMENT));
		अवरोध;
	हाल DOM_ATTRIBUTE_गाँठ:
	हाल DOM_PROCESSING_INSTRUCTION_गाँठ:
	हाल DOM_DOCUMENT_TYPE_गाँठ:
	हाल DOM_DOCUMENT_FRAGMENT_गाँठ:
	हाल DOM_NOTATION_गाँठ:
	हाल DOM_ENTITY_REFERENCE_गाँठ:
	हाल DOM_ENTITY_गाँठ:
	हाल DOM_CDATA_SECTION_गाँठ:
	शेष:
		/* Oh bum, just node then */
		duk_धकेलो_string(ctx, PROTO_NAME(गाँठ_2));
	}
}

duk_bool_t
dukky_धकेलो_गाँठ(duk_context *ctx, काष्ठा dom_गाँठ *गाँठ)
{
	NSLOG(dukky, DEEPDEBUG, "Pushing node %p", गाँठ);
	/* First check if we can find the node */
	/* ... */
	duk_get_global_string(ctx, गाँठ_MAGIC);
	/* ... nodes */
	duk_धकेलो_pointer(ctx, गाँठ);
	/* ... nodes nodeptr */
	duk_get_prop(ctx, -2);
	/* ... nodes node/undefined */
	अगर (!duk_is_undefined(ctx, -1)) {
		/* ... nodes node */
		duk_जोड़ो(ctx, -2);
		/* ... node nodes */
		duk_निकालो(ctx);
		/* ... node */
		अगर (NSLOG_COMPILED_MIN_LEVEL <= NSLOG_LEVEL_DEEPDEBUG) {
			duk_dup(ctx, -1);
			स्थिर अक्षर * what = duk_safe_to_string(ctx, -1);
			NSLOG(dukky, DEEPDEBUG, "Found it memoised: %s", what);
			duk_निकालो(ctx);
		}
		वापस true;
	}
	/* ... nodes undefined */
	duk_निकालो_2(ctx);
	/* ... */
	/* We couldn't, so now we determine the node type and then
	 * we ask for it to be created
	 */
	duk_धकेलो_pointer(ctx, गाँठ);
	/* ... nodeptr */
	dukky_धकेलो_गाँठ_klass(ctx, गाँठ);
	/* ... nodeptr klass */
	वापस dukky_धकेलो_गाँठ_stacked(ctx);
}

अटल duk_ret_t
dukky_bad_constructor(duk_context *ctx)
{
	वापस duk_error(ctx, DUK_ERR_ERROR, "Bad constructor");
}

व्योम
dukky_inject_not_ctr(duk_context *ctx, पूर्णांक idx, स्थिर अक्षर *name)
{
	/* ... p[idx] ... proto */
	duk_धकेलो_c_function(ctx, dukky_bad_constructor, 0);
	/* ... p[idx] ... proto cons */
	duk_जोड़ो(ctx, -2);
	/* ... p[idx] ... cons proto */
	duk_put_prop_string(ctx, -2, "prototype");
	/* ... p[idx] ... cons[proto] */
	duk_put_prop_string(ctx, idx, name);
	/* ... p ... */
	वापस;
}

/* Duktape heap utility functions */

/* We need to override the defaults because not all platforms are fully ANSI
 * compatible.  E.g. RISC OS gets upset if we malloc or realloc a zero byte
 * block, as do debugging tools such as Electric Fence by Bruce Perens.
 */

अटल व्योम *dukky_alloc_function(व्योम *udata, duk_आकार_t आकार)
{
	अगर (आकार == 0)
		वापस शून्य;

	वापस दो_स्मृति(आकार);
}

अटल व्योम *dukky_realloc_function(व्योम *udata, व्योम *ptr, duk_आकार_t आकार)
{
	अगर (ptr == शून्य && आकार == 0)
		वापस शून्य;

	अगर (आकार == 0) {
		मुक्त(ptr);
		वापस शून्य;
	}

	वापस पुनः_स्मृति(ptr, आकार);
}


अटल व्योम dukky_free_function(व्योम *udata, व्योम *ptr)
{
	अगर (ptr != शून्य)
		मुक्त(ptr);
}

/* exported interface documented in js.h */
व्योम js_initialise(व्योम)
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
व्योम js_finalise(व्योम)
{
	/* NADA for now */
}


/* exported interface documented in js.h */
nserror
js_newheap(पूर्णांक timeout, jsheap **heap)
{
	duk_context *ctx;
	jsheap *ret = सुस्मृति(1, माप(*ret));
	*heap = शून्य;
	NSLOG(dukky, DEBUG, "Creating new duktape javascript heap");
	अगर (ret == शून्य) वापस NSERROR_NOMEM;
	ctx = ret->ctx = duk_create_heap(
		dukky_alloc_function,
		dukky_realloc_function,
		dukky_free_function,
		ret,
		शून्य);
	अगर (ret->ctx == शून्य) { मुक्त(ret); वापस NSERROR_NOMEM; }
	/* Create the prototype stuffs */
	duk_धकेलो_global_object(ctx);
	duk_धकेलो_boolean(ctx, true);
	duk_put_prop_string(ctx, -2, "protos");
	duk_put_global_string(ctx, PROTO_MAGIC);
	/* Create prototypes here */
	dukky_create_prototypes(ctx);
	/* Now create the thread map */
	duk_धकेलो_object(ctx);
	duk_put_global_string(ctx, THREAD_MAP);

	*heap = ret;
	वापस NSERROR_OK;
}


अटल व्योम dukky_destroyheap(jsheap *heap)
{
	निश्चित(heap->pending_destroy == true);
	निश्चित(heap->live_threads == 0);
	NSLOG(dukky, DEBUG, "Destroying duktape javascript context");
	duk_destroy_heap(heap->ctx);
	मुक्त(heap);
}

/* exported interface documented in js.h */
व्योम js_destroyheap(jsheap *heap)
{
	heap->pending_destroy = true;
	अगर (heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* Just for here, the CTX is in ret, not thread */
#घोषणा CTX (ret->ctx)

/* exported interface documented in js.h */
nserror js_newthread(jsheap *heap, व्योम *win_priv, व्योम *doc_priv, jsthread **thread)
{
	jsthread *ret;
	निश्चित(heap != शून्य);
	निश्चित(heap->pending_destroy == false);

	ret = सुस्मृति(1, माप (*ret));
	अगर (ret == शून्य) {
		NSLOG(dukky, ERROR, "Unable to allocate new JS thread structure");
		वापस NSERROR_NOMEM;
	}

	NSLOG(dukky, DEBUG,
	      "New javascript/duktape thread, win_priv=%p, doc_priv=%p",
	      win_priv, doc_priv);

	/* create new thread */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_धकेलो_thread(heap->ctx); /* ... threads thread */
	ret->heap = heap;
	ret->ctx = duk_require_context(heap->ctx, -1);
	ret->thread_idx = heap->अग्र_thread++;
	duk_put_prop_सूचकांक(heap->ctx, -2, ret->thread_idx);
	heap->live_threads++;
	duk_निकालो(heap->ctx); /* ... */
	duk_धकेलो_int(CTX, 0);
	duk_धकेलो_int(CTX, 1);
	duk_धकेलो_int(CTX, 2);
	/* Manufacture a Window object */
	/* win_priv is a browser_window, doc_priv is an html content struct */
	duk_धकेलो_pointer(CTX, win_priv);
	duk_धकेलो_pointer(CTX, doc_priv);
	dukky_create_object(CTX, PROTO_NAME(WINDOW), 2);
	duk_धकेलो_global_object(CTX);
	duk_put_prop_string(CTX, -2, PROTO_MAGIC);
	duk_set_global_object(CTX);

	/* Now we need to prepare our node mapping table */
	duk_धकेलो_object(CTX);
	duk_धकेलो_pointer(CTX, शून्य);
	duk_धकेलो_null(CTX);
	duk_put_prop(CTX, -3);
	duk_put_global_string(CTX, गाँठ_MAGIC);

	/* And now the event mapping table */
	duk_धकेलो_object(CTX);
	duk_put_global_string(CTX, EVENT_MAGIC);

	/* Now load the polyfills */
	/* ... */
	duk_धकेलो_string(CTX, "polyfill.js");
	/* ..., polyfill.js */
	अगर (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (स्थिर अक्षर *)polyfill_js, polyfill_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile polyfill.js, thread aborted");
		js_destroythread(ret);
		वापस NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	अगर (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run polyfill.js, thread aborted");
		js_destroythread(ret);
		वापस NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_निकालो(CTX);
	/* ... */

	/* Now load the NetSurf table in */
	/* ... */
	duk_धकेलो_string(CTX, "generics.js");
	/* ..., generics.js */
	अगर (duk_pcompile_lstring_filename(CTX, DUK_COMPILE_EVAL,
					  (स्थिर अक्षर *)generics_js, generics_js_len) != 0) {
		NSLOG(dukky, CRITICAL, "%s", duk_safe_to_string(CTX, -1));
		NSLOG(dukky, CRITICAL, "Unable to compile generics.js, thread aborted");
		js_destroythread(ret);
		वापस NSERROR_INIT_FAILED;
	}
	/* ..., (generics.js) */
	अगर (dukky_pcall(CTX, 0, true) != 0) {
		NSLOG(dukky, CRITICAL, "Unable to run generics.js, thread aborted");
		js_destroythread(ret);
		वापस NSERROR_INIT_FAILED;
	}
	/* ..., result */
	duk_निकालो(CTX);
	/* ... */
	duk_धकेलो_global_object(CTX);
	/* ..., Win */
	duk_get_prop_string(CTX, -1, "NetSurf");
	/* ..., Win, NetSurf */
	duk_put_global_string(CTX, GENERICS_MAGIC);
	/* ..., Win */
	duk_del_prop_string(CTX, -1, "NetSurf");
	duk_निकालो(CTX);
	/* ... */

	dukky_log_ढेर_frame(CTX, "New thread created");
	NSLOG(dukky, DEBUG, "New thread is %p in heap %p", thread, heap);
	*thread = ret;

	वापस NSERROR_OK;
}

/* Now switch to the long term CTX behaviour */
#अघोषित CTX
#घोषणा CTX (thread->ctx)

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

	वापस NSERROR_OK;
}

/**
 * Destroy a Duktape thread
 */
अटल व्योम dukky_destroythread(jsthread *thread)
{
	jsheap *heap = thread->heap;

	निश्चित(thread->in_use == 0);
	निश्चित(thread->pending_destroy == true);

	/* Closing down the extant thread */
	NSLOG(dukky, DEBUG, "Closing down extant thread %p in heap %p", thread, heap);
	duk_get_global_string(CTX, MAGIC(closedownThread));
	dukky_pcall(CTX, 0, true);

	/* Now delete the thread from the heap */
	duk_get_global_string(heap->ctx, THREAD_MAP); /* ... threads */
	duk_del_prop_सूचकांक(heap->ctx, -1, thread->thread_idx);
	duk_निकालो(heap->ctx); /* ... */

	/* We can now free the thread object */
	मुक्त(thread);

	/* Finally give the heap a chance to clean up */
	duk_gc(heap->ctx, 0);
	duk_gc(heap->ctx, DUK_GC_COMPACT);
	heap->live_threads--;

	/* And if the heap should now go, blow it away */
	अगर (heap->pending_destroy == true && heap->live_threads == 0) {
		dukky_destroyheap(heap);
	}
}

/* exported interface documented in js.h */
व्योम js_destroythread(jsthread *thread)
{
	thread->pending_destroy = true;
	अगर (thread->in_use == 0) {
		dukky_destroythread(thread);
	}
}

अटल व्योम dukky_enter_thread(jsthread *thread)
{
	निश्चित(thread != शून्य);
	thread->in_use++;
}

अटल व्योम dukky_leave_thread(jsthread *thread)
{
	निश्चित(thread != शून्य);
	निश्चित(thread->in_use > 0);

	thread->in_use--;
	अगर (thread->in_use == 0 && thread->pending_destroy == true) {
		dukky_destroythread(thread);
	}
}

duk_bool_t dukky_check_timeout(व्योम *udata)
{
#घोषणा JS_EXEC_TIMEOUT_MS 10000 /* 10 seconds */
	jsheap *heap = (jsheap *) udata;
	uint64_t now;

	(व्योम) nsu_getmonotonic_ms(&now);

	/* This function may be called during duk heap construction,
	 * so only test for execution timeout if we've recorded a
	 * start time.
	 */
	वापस heap->exec_start_time != 0 &&
			now > (heap->exec_start_time + JS_EXEC_TIMEOUT_MS);
}

अटल व्योम dukky_dump_error(duk_context *ctx)
{
	/* stack is ..., errobj */
	duk_dup_शीर्ष(ctx);
	/* ..., errobj, errobj */
	NSLOG(jserrors, WARNING, "Uncaught error in JS: %s", duk_safe_to_stacktrace(ctx, -1));
	/* ..., errobj, errobj.stackstring */
	duk_निकालो(ctx);
	/* ..., errobj */
}

अटल व्योम dukky_reset_start_time(duk_context *ctx)
{
	duk_memory_functions funcs;
	jsheap *heap;
	duk_get_memory_functions(ctx, &funcs);
	heap = funcs.udata;
	(व्योम) nsu_getmonotonic_ms(&heap->exec_start_time);
}

duk_int_t dukky_pcall(duk_context *ctx, duk_आकार_t argc, bool reset_timeout)
{
	अगर (reset_timeout) {
		dukky_reset_start_time(ctx);
	}

	duk_int_t ret = duk_pcall(ctx, argc);
	अगर (ret) {
		/* Something went wrong calling this... */
		dukky_dump_error(ctx);
	}

	वापस ret;
}


व्योम dukky_धकेलो_generics(duk_context *ctx, स्थिर अक्षर *generic)
{
	/* ... */
	duk_get_global_string(ctx, GENERICS_MAGIC);
	/* ..., generics */
	duk_get_prop_string(ctx, -1, generic);
	/* ..., generics, generic */
	duk_remove(ctx, -2);
	/* ..., generic */
}

अटल duk_int_t dukky_धकेलो_context_dump(duk_context *ctx, व्योम *udata)
{
	duk_धकेलो_context_dump(ctx);
	वापस 1;
}

व्योम dukky_log_ढेर_frame(duk_context *ctx, स्थिर अक्षर * reason)
{
	अगर (duk_safe_call(ctx, dukky_धकेलो_context_dump, शून्य, 0, 1) != 0) {
		duk_निकालो(ctx);
		duk_धकेलो_string(ctx, "[???]");
	}
	NSLOG(dukky, DEEPDEBUG, "%s, stack is: %s", reason, duk_safe_to_string(ctx, -1));
	duk_निकालो(ctx);
}


/* exported interface documented in js.h */
bool
js_exec(jsthread *thread, स्थिर uint8_t *txt, माप_प्रकार txtlen, स्थिर अक्षर *name)
{
	bool ret = false;
	निश्चित(thread);

	अगर (txt == शून्य || txtlen == 0) {
		वापस false;
	}

	अगर (thread->pending_destroy) {
		NSLOG(dukky, DEEPDEBUG, "Skipping exec call because thread is dead");
		वापस false;
	}

	dukky_enter_thread(thread);

	duk_set_शीर्ष(CTX, 0);
	NSLOG(dukky, DEEPDEBUG, "Running %"PRIsizet" bytes from %s", txtlen, name);
	/* NSLOG(dukky, DEEPDEBUG, "\n%s\n", txt); */

	dukky_reset_start_time(CTX);
	अगर (name != शून्य) {
		duk_धकेलो_string(CTX, name);
	} अन्यथा {
		duk_धकेलो_string(CTX, "?unknown source?");
	}
	अगर (duk_pcompile_lstring_filename(CTX,
					  DUK_COMPILE_EVAL,
					  (स्थिर अक्षर *)txt,
					  txtlen) != 0) {
		NSLOG(dukky, DEBUG, "Failed to compile JavaScript input");
		जाओ handle_error;
	}

	अगर (duk_pcall(CTX, 0/*nargs*/) == DUK_EXEC_ERROR) {
		NSLOG(dukky, DEBUG, "Failed to execute JavaScript");
		जाओ handle_error;
	}

	अगर (duk_get_शीर्ष(CTX) == 0) duk_धकेलो_boolean(CTX, false);
	NSLOG(dukky, DEEPDEBUG, "Returning %s",
	      duk_get_boolean(CTX, 0) ? "true" : "false");
	ret = duk_get_boolean(CTX, 0);
	जाओ out;

handle_error:
	dukky_dump_error(CTX);
out:
	dukky_leave_thread(thread);
	वापस ret;
}

अटल स्थिर अक्षर* dukky_event_proto(dom_event *evt)
{
	स्थिर अक्षर *ret = PROTO_NAME(EVENT);
	dom_string *type = शून्य;
	dom_exception err;

	err = dom_event_get_type(evt, &type);
	अगर (err != DOM_NO_ERR) {
		जाओ out;
	}

	अगर (dom_string_isequal(type, corestring_dom_keydown)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		जाओ out;
	} अन्यथा अगर (dom_string_isequal(type, corestring_dom_keyup)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		जाओ out;
	} अन्यथा अगर (dom_string_isequal(type, corestring_dom_keypress)) {
		ret = PROTO_NAME(KEYBOARDEVENT);
		जाओ out;
	}

out:
	अगर (type != शून्य) {
		dom_string_unref(type);
	}

	वापस ret;
}

/*** New style event handling ***/

व्योम dukky_धकेलो_event(duk_context *ctx, dom_event *evt)
{
	/* ... */
	duk_get_global_string(ctx, EVENT_MAGIC);
	/* ... events */
	duk_धकेलो_pointer(ctx, evt);
	/* ... events eventptr */
	duk_get_prop(ctx, -2);
	/* ... events event? */
	अगर (duk_is_undefined(ctx, -1)) {
		/* ... events undefined */
		duk_निकालो(ctx);
		/* ... events */
		duk_धकेलो_pointer(ctx, evt);
		अगर (dukky_create_object(ctx, dukky_event_proto(evt), 1) != DUK_EXEC_SUCCESS) {
			/* ... events err */
			duk_निकालो(ctx);
			/* ... events */
			duk_धकेलो_object(ctx);
			/* ... events eobj[meh] */
		}
		/* ... events eobj */
		duk_धकेलो_pointer(ctx, evt);
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

अटल व्योम dukky_धकेलो_handler_code(duk_context *ctx, dom_string *name,
				     dom_event_target *et)
{
	dom_string *onname, *val;
	dom_element *ele = (dom_element *)et;
	dom_exception exc;
	dom_गाँठ_type ntype;

	/* If et is NULL, then we're actually dealing with the Window object
	 * which has no default handlers and no way to assign handlers
	 * which aren't directly stored in the HANDLER_MAGIC
	 */
	अगर (et == शून्य) {
		duk_धकेलो_lstring(ctx, "", 0);
		वापस;
	}

	/* The rest of this assumes et is a proper event target and expands
	 * out from there based on the assumption that all valid event targets
	 * are nodes.
	 */
	exc = dom_गाँठ_get_गाँठ_type(et, &ntype);
	अगर (exc != DOM_NO_ERR) {
		duk_धकेलो_lstring(ctx, "", 0);
		वापस;
	}

	अगर (ntype != DOM_ELEMENT_गाँठ) {
		duk_धकेलो_lstring(ctx, "", 0);
		वापस;
	}

	exc = dom_string_concat(corestring_dom_on, name, &onname);
	अगर (exc != DOM_NO_ERR) {
		duk_धकेलो_lstring(ctx, "", 0);
		वापस;
	}

	exc = dom_element_get_attribute(ele, onname, &val);
	अगर ((exc != DOM_NO_ERR) || (val == शून्य)) {
		dom_string_unref(onname);
		duk_धकेलो_lstring(ctx, "", 0);
		वापस;
	}

	dom_string_unref(onname);
	duk_धकेलो_lstring(ctx, dom_string_तथ्य(val), dom_string_लंबाई(val));
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
	duk_धकेलो_lstring(ctx, dom_string_तथ्य(name), dom_string_लंबाई(name));
	/* ... node handlers name */
	duk_get_prop(ctx, -2);
	/* ... node handlers handler? */
	अगर (duk_is_undefined(ctx, -1)) {
		/* ... node handlers undefined */
		duk_निकालो_2(ctx);
		/* ... node */
		dukky_धकेलो_handler_code(ctx, name, et);
		/* ... node handlercode? */
		/* TODO: If this is null, clean up and propagate */
		/* ... node handlercode */
		/** @todo This is entirely wrong, but it's hard to get right */
		duk_धकेलो_string(ctx, "function (event) {");
		/* ... node handlercode prefix */
		duk_जोड़ो(ctx, -2);
		/* ... node prefix handlercode */
		duk_धकेलो_string(ctx, "}");
		/* ... node prefix handlercode suffix */
		duk_concat(ctx, 3);
		/* ... node fullhandlersrc */
		duk_धकेलो_string(ctx, "internal raw uncompiled handler");
		/* ... node fullhandlersrc filename */
		अगर (duk_pcompile(ctx, DUK_COMPILE_FUNCTION) != 0) {
			/* ... node err */
			NSLOG(dukky, DEBUG,
			      "Unable to proceed with handler, could not compile");
			duk_निकालो_2(ctx);
			वापस false;
		}
		/* ... node handler */
		duk_जोड़ो(ctx, -2);
		/* ... handler node */
	} अन्यथा {
		/* ... node handlers handler */
		duk_जोड़ो(ctx, -3);
		/* ... handler node handlers */
		duk_निकालो(ctx);
		/* ... handler node */
	}
	/* ... handler node */
	वापस true;
}

अटल व्योम dukky_generic_event_handler(dom_event *evt, व्योम *pw)
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
	अगर (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG, "Unable to find the event name");
		वापस;
	}
	NSLOG(dukky, DEBUG, "Event's name is %*s", (पूर्णांक)dom_string_लंबाई(name),
	      dom_string_तथ्य(name));
	exc = dom_event_get_event_phase(evt, &phase);
	अगर (exc != DOM_NO_ERR) {
		NSLOG(dukky, WARNING, "Unable to get event phase");
		वापस;
	}
	NSLOG(dukky, DEBUG, "Event phase is: %s (%d)",
	      phase == DOM_CAPTURING_PHASE ? "capturing" : phase == DOM_AT_TARGET ? "at-target" : phase == DOM_BUBBLING_PHASE ? "bubbling" : "unknown",
	      (पूर्णांक)phase);

	exc = dom_event_get_current_target(evt, &targ);
	अगर (exc != DOM_NO_ERR) {
		dom_string_unref(name);
		NSLOG(dukky, DEBUG, "Unable to find the event target");
		वापस;
	}

	/* If we're capturing right now, we skip the 'event handler'
	 * and go straight to the extras
	 */
	अगर (phase == DOM_CAPTURING_PHASE)
		जाओ handle_extras;

	/* ... */
	अगर (dukky_धकेलो_गाँठ(ctx, (dom_गाँठ *)targ) == false) {
		dom_string_unref(name);
		dom_गाँठ_unref(targ);
		NSLOG(dukky, DEBUG,
		      "Unable to push JS node representation?!");
		वापस;
	}
	/* ... node */
	अगर (dukky_get_current_मूल्य_of_event_handler(
		    ctx, name, (dom_event_target *)targ) == false) {
		/* ... */
		जाओ handle_extras;
	}
	/* ... handler node */
	dukky_धकेलो_event(ctx, evt);
	/* ... handler node event */
	dukky_reset_start_time(ctx);
	अगर (duk_pcall_method(ctx, 1) != 0) {
		/* Failed to run the method */
		/* ... err */
		NSLOG(dukky, DEBUG,
		      "OH NOES! An error running a callback.  Meh.");
		exc = dom_event_stop_immediate_propagation(evt);
		अगर (exc != DOM_NO_ERR)
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

		duk_निकालो_n(ctx, 6);
		/* ... */
		जाओ handle_extras;
	}
	/* ... result */
	अगर (duk_is_boolean(ctx, -1) &&
	    duk_to_boolean(ctx, -1) == 0) {
		dom_event_prevent_default(evt);
	}
	duk_निकालो(ctx);
handle_extras:
	/* ... */
	duk_धकेलो_lstring(ctx, dom_string_तथ्य(name), dom_string_लंबाई(name));
	dukky_धकेलो_गाँठ(ctx, (dom_गाँठ *)targ);
	/* ... type node */
	अगर (dukky_event_target_धकेलो_listeners(ctx, true)) {
		/* Nothing to do */
		duk_निकालो(ctx);
		जाओ out;
	}
	/* ... sublisteners */
	duk_धकेलो_array(ctx);
	/* ... sublisteners copy */
	idx = 0;
	जबतक (duk_get_prop_सूचकांक(ctx, -2, idx)) {
		/* ... sublisteners copy handler */
		duk_get_prop_सूचकांक(ctx, -1, 1);
		/* ... sublisteners copy handler flags */
		अगर ((event_listener_flags)duk_to_int(ctx, -1) & ELF_ONCE) {
			duk_dup(ctx, -4);
			/* ... subl copy handler flags subl */
			dukky_shuffle_array(ctx, idx);
			duk_निकालो(ctx);
			/* ... subl copy handler flags */
		}
		duk_निकालो(ctx);
		/* ... sublisteners copy handler */
		duk_put_prop_सूचकांक(ctx, -2, idx);
		/* ... sublisteners copy */
		idx++;
	}
	/* ... sublisteners copy undefined */
	duk_निकालो(ctx);
	/* ... sublisteners copy */
	duk_जोड़ो(ctx, -2);
	/* ... copy sublisteners */
	duk_निकालो(ctx);
	/* ... copy */
	idx = 0;
	जबतक (duk_get_prop_सूचकांक(ctx, -1, idx++)) {
		/* ... copy handler */
		अगर (duk_get_prop_सूचकांक(ctx, -1, 2)) {
			/* ... copy handler meh */
			duk_निकालो_2(ctx);
			जारी;
		}
		duk_निकालो(ctx);
		duk_get_prop_सूचकांक(ctx, -1, 0);
		duk_get_prop_सूचकांक(ctx, -2, 1);
		/* ... copy handler callback flags */
		flags = (event_listener_flags)duk_get_int(ctx, -1);
		duk_निकालो(ctx);
		/* ... copy handler callback */
		अगर (((phase == DOM_CAPTURING_PHASE) && !(flags & ELF_CAPTURE)) ||
		    ((phase != DOM_CAPTURING_PHASE) && (flags & ELF_CAPTURE))) {
			duk_निकालो_2(ctx);
			/* ... copy */
			जारी;
		}
		/* ... copy handler callback */
		dukky_धकेलो_गाँठ(ctx, (dom_गाँठ *)targ);
		/* ... copy handler callback node */
		dukky_धकेलो_event(ctx, evt);
		/* ... copy handler callback node event */
		dukky_reset_start_time(ctx);
		अगर (duk_pcall_method(ctx, 1) != 0) {
			/* Failed to run the method */
			/* ... copy handler err */
			NSLOG(dukky, DEBUG,
			      "OH NOES! An error running a callback.  Meh.");
			exc = dom_event_stop_immediate_propagation(evt);
			अगर (exc != DOM_NO_ERR)
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

			duk_निकालो_n(ctx, 7);
			/* ... copy */
			जारी;
		}
		/* ... copy handler result */
		अगर (duk_is_boolean(ctx, -1) &&
		    duk_to_boolean(ctx, -1) == 0) {
			dom_event_prevent_default(evt);
		}
		duk_निकालो_2(ctx);
		/* ... copy */
	}
	duk_निकालो_2(ctx);
out:
	/* ... */
	dom_गाँठ_unref(targ);
	dom_string_unref(name);
}

व्योम dukky_register_event_listener_for(duk_context *ctx,
				       काष्ठा dom_element *ele,
				       dom_string *name,
				       bool capture)
{
	dom_event_listener *listen = शून्य;
	dom_exception exc;

	/* ... */
	अगर (ele == शून्य) {
		/* A null element is the Window object */
		duk_धकेलो_global_object(ctx);
	} अन्यथा {
		/* Non null elements must be pushed as a node object */
		अगर (dukky_धकेलो_गाँठ(ctx, (काष्ठा dom_गाँठ *)ele) == false)
			वापस;
	}
	/* ... node */
	duk_get_prop_string(ctx, -1, HANDLER_LISTENER_MAGIC);
	/* ... node handlers */
	duk_धकेलो_lstring(ctx, dom_string_तथ्य(name), dom_string_लंबाई(name));
	/* ... node handlers name */
	अगर (duk_has_prop(ctx, -2)) {
		/* ... node handlers */
		duk_निकालो_2(ctx);
		/* ... */
		वापस;
	}
	/* ... node handlers */
	duk_धकेलो_lstring(ctx, dom_string_तथ्य(name), dom_string_लंबाई(name));
	/* ... node handlers name */
	duk_धकेलो_boolean(ctx, true);
	/* ... node handlers name true */
	duk_put_prop(ctx, -3);
	/* ... node handlers */
	duk_निकालो_2(ctx);
	/* ... */
	अगर (ele == शून्य) {
		/* Nothing more to do, Window doesn't register in the
		 * normal event listener flow
		 */
		वापस;
	}

	/* Otherwise add an event listener to the element */
	exc = dom_event_listener_create(dukky_generic_event_handler, ctx,
					&listen);
	अगर (exc != DOM_NO_ERR) वापस;
	exc = dom_event_target_add_event_listener(
		ele, name, listen, capture);
	अगर (exc != DOM_NO_ERR) {
		NSLOG(dukky, DEBUG,
		      "Unable to register listener for %p.%*s", ele,
		      (पूर्णांक)dom_string_लंबाई(name), dom_string_तथ्य(name));
	} अन्यथा {
		NSLOG(dukky, DEBUG, "have registered listener for %p.%*s",
		      ele, (पूर्णांक)dom_string_लंबाई(name), dom_string_तथ्य(name));
	}
	dom_event_listener_unref(listen);
}

/* The sub-listeners are a list of {callback,flags} tuples */
/* We return true if we created a new sublistener table */
/* If we're told to not create, but we want to, we still return true */
bool dukky_event_target_धकेलो_listeners(duk_context *ctx, bool dont_create)
{
	bool ret = false;
	/* ... type this */
	duk_get_prop_string(ctx, -1, EVENT_LISTENER_JS_MAGIC);
	अगर (duk_is_undefined(ctx, -1)) {
		/* ... type this null */
		duk_निकालो(ctx);
		duk_धकेलो_object(ctx);
		duk_dup(ctx, -1);
		/* ... type this listeners listeners */
		duk_put_prop_string(ctx, -3, EVENT_LISTENER_JS_MAGIC);
		/* ... type this listeners */
	}
	/* ... type this listeners */
	duk_जोड़ो(ctx, -3);
	/* ... listeners type this */
	duk_निकालो(ctx);
	/* ... listeners type */
	duk_dup(ctx, -1);
	/* ... listeners type type */
	duk_get_prop(ctx, -3);
	/* ... listeners type ??? */
	अगर (duk_is_undefined(ctx, -1)) {
		/* ... listeners type ??? */
		अगर (dont_create == true) {
			duk_निकालो_3(ctx);
			duk_धकेलो_undefined(ctx);
			वापस true;
		}
		duk_निकालो(ctx);
		duk_धकेलो_array(ctx);
		duk_dup(ctx, -2);
		duk_dup(ctx, -2);
		/* ... listeners type sublisteners type sublisteners */
		duk_put_prop(ctx, -5);
		/* ... listeners type sublisteners */
		ret = true;
	}
	duk_जोड़ो(ctx, -3);
	/* ... sublisteners listeners type */
	duk_निकालो_2(ctx);
	/* ... sublisteners */
	वापस ret;
}

/* Shuffle a duktape array "down" one.  This involves iterating from
 * the index provided, shuffling elements down, until we reach an
 * undefined
 */
व्योम dukky_shuffle_array(duk_context *ctx, duk_uarridx_t idx)
{
	/* ... somearr */
	जबतक (duk_get_prop_सूचकांक(ctx, -1, idx + 1)) {
		duk_put_prop_सूचकांक(ctx, -2, idx);
		idx++;
	}
	/* ... somearr undefined */
	duk_del_prop_सूचकांक(ctx, -2, idx + 1);
	duk_निकालो(ctx);
}


व्योम js_handle_new_element(jsthread *thread, काष्ठा dom_element *गाँठ)
{
	निश्चित(thread);
	निश्चित(गाँठ);
	dom_namednodemap *map;
	dom_exception exc;
	dom_ulong idx;
	dom_ulong siz;
	dom_attr *attr = शून्य;
	dom_string *कुंजी = शून्य;
	dom_string *nodename;
	duk_bool_t is_body = false;

	exc = dom_गाँठ_get_गाँठ_name(गाँठ, &nodename);
	अगर (exc != DOM_NO_ERR) वापस;

	अगर (nodename == corestring_dom_BODY)
		is_body = true;

	dom_string_unref(nodename);

	exc = dom_गाँठ_get_attributes(गाँठ, &map);
	अगर (exc != DOM_NO_ERR) वापस;
	अगर (map == शून्य) वापस;

	dukky_enter_thread(thread);

	exc = dom_namednodemap_get_लंबाई(map, &siz);
	अगर (exc != DOM_NO_ERR) जाओ out;

	क्रम (idx = 0; idx < siz; idx++) {
		exc = dom_namednodemap_item(map, idx, &attr);
		अगर (exc != DOM_NO_ERR) जाओ out;
		exc = dom_attr_get_name(attr, &कुंजी);
		अगर (exc != DOM_NO_ERR) जाओ out;
		अगर (is_body && (
			    कुंजी == corestring_dom_onblur ||
			    कुंजी == corestring_dom_onerror ||
			    कुंजी == corestring_dom_onfocus ||
			    कुंजी == corestring_dom_onload ||
			    कुंजी == corestring_dom_onresize ||
			    कुंजी == corestring_dom_onscroll)) {
			/* This is a forwarded event, it doesn't matter,
			 * we should skip registering for it and later
			 * we will register it for Window itself
			 */
			जाओ skip_register;
		}
		अगर (dom_string_लंबाई(कुंजी) > 2) {
			/* Can be on* */
			स्थिर uint8_t *तथ्य = (स्थिर uint8_t *)dom_string_तथ्य(कुंजी);
			अगर (तथ्य[0] == 'o' && तथ्य[1] == 'n') {
				dom_string *sub = शून्य;
				exc = dom_string_substr(
					कुंजी, 2, dom_string_लंबाई(कुंजी),
					&sub);
				अगर (exc == DOM_NO_ERR) {
					dukky_register_event_listener_for(
						CTX, गाँठ, sub, false);
					dom_string_unref(sub);
				}
			}
		}
	skip_register:
		dom_string_unref(कुंजी); कुंजी = शून्य;
		dom_गाँठ_unref(attr); attr = शून्य;
	}

out:
	अगर (कुंजी != शून्य)
		dom_string_unref(कुंजी);

	अगर (attr != शून्य)
		dom_गाँठ_unref(attr);

	dom_namednodemap_unref(map);

	dukky_leave_thread(thread);
}

व्योम js_event_cleanup(jsthread *thread, काष्ठा dom_event *evt)
{
	निश्चित(thread);
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, EVENT_MAGIC);
	/* ... EVENT_MAP */
	duk_धकेलो_pointer(CTX, evt);
	/* ... EVENT_MAP eventptr */
	duk_del_prop(CTX, -2);
	/* ... EVENT_MAP */
	duk_निकालो(CTX);
	/* ... */
	dukky_leave_thread(thread);
}

bool js_fire_event(jsthread *thread, स्थिर अक्षर *type, काष्ठा dom_document *doc, काष्ठा dom_गाँठ *target)
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

	अगर (target != शून्य)
		/* Swallow non-Window-targetted events quietly */
		वापस true;

	अगर (म_भेद(type, "load") != 0)
		/* Swallow non-load events quietly */
		वापस true;

	/* Okay, we're processing load, targetted at Window, do the single
	 * thing which gets us there, which is to find the appropriate event
	 * handler and call it.  If we have no event handler on Window then
	 * we divert to the body, and if there's no event handler there
	 * we swallow the event silently
	 */

	exc = dom_event_create(&evt);
	अगर (exc != DOM_NO_ERR) वापस true;
	exc = dom_event_init(evt, corestring_dom_load, false, false);
	अगर (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		वापस true;
	}
	dukky_enter_thread(thread);
	/* ... */
	duk_get_global_string(CTX, HANDLER_MAGIC);
	/* ... handlers */
	duk_धकेलो_lstring(CTX, "load", 4);
	/* ... handlers "load" */
	duk_get_prop(CTX, -2);
	/* ... handlers handler? */
	अगर (duk_is_undefined(CTX, -1)) {
		/* No handler here, *try* and retrieve a handler from
		 * the body
		 */
		duk_निकालो(CTX);
		/* ... handlers */
		exc = dom_html_document_get_body(doc, &body);
		अगर (exc != DOM_NO_ERR) {
			dom_event_unref(evt);
			dukky_leave_thread(thread);
			वापस true;
		}
		dukky_धकेलो_गाँठ(CTX, (काष्ठा dom_गाँठ *)body);
		/* ... handlers bodynode */
		अगर (dukky_get_current_मूल्य_of_event_handler(
			    CTX, corestring_dom_load, body) == false) {
			/* Unref the body, we don't need it any more */
			dom_गाँठ_unref(body);
			/* ... handlers */
			duk_निकालो(CTX);
			dukky_leave_thread(thread);
			वापस true;
		}
		/* Unref the body, we don't need it any more */
		dom_गाँठ_unref(body);
		/* ... handlers handler bodynode */
		duk_निकालो(CTX);
	}
	/* ... handlers handler */
	duk_जोड़ो(CTX, -2);
	/* ... handler handlers */
	duk_निकालो(CTX);
	/* ... handler */
	duk_धकेलो_global_object(CTX);
	/* ... handler Window */
	dukky_धकेलो_event(CTX, evt);
	/* ... handler Window event */
	dukky_reset_start_time(CTX);
	अगर (duk_pcall_method(CTX, 1) != 0) {
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

		duk_निकालो_n(CTX, 6);
		/* ... */
		js_event_cleanup(thread, evt);
		dom_event_unref(evt);
		dukky_leave_thread(thread);
		वापस true;
	}
	/* ... result */
	duk_निकालो(CTX);
	/* ... */
	js_event_cleanup(thread, evt);
	dom_event_unref(evt);
	dukky_leave_thread(thread);
	वापस true;
}
