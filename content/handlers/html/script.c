/*
 * Copyright 2012 Vincent Sanders <vince@netsurf-browser.org>
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
 * implementation of content handling for text/html scripts.
 */

#समावेश <assert.h>
#समावेश <ctype.h>
#समावेश <stdint.h>
#समावेश <stdbool.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <stdlib.h>

#समावेश "utils/config.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "netsurf/content.h"
#समावेश "javascript/js.h"
#समावेश "content/content_protected.h"
#समावेश "content/content_factory.h"
#समावेश "content/fetch.h"
#समावेश "content/hlcache.h"

#समावेश "html/html.h"
#समावेश "html/private.h"

प्रकार bool (script_handler_t)(काष्ठा jsthread *jsthread, स्थिर uint8_t *तथ्य, माप_प्रकार आकार, स्थिर अक्षर *name);


अटल script_handler_t *select_script_handler(content_type ctype)
{
	अगर (ctype == CONTENT_JS) {
		वापस js_exec;
	}
	वापस शून्य;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	अचिन्हित पूर्णांक i;
	काष्ठा html_script *s;
	script_handler_t *script_handler;
	bool have_run_something = false;

	अगर (c->jsthread == शून्य) {
		वापस NSERROR_BAD_PARAMETER;
	}

	क्रम (i = 0, s = c->scripts; i != c->scripts_गिनती; i++, s++) {
		अगर (s->already_started) {
			जारी;
		}

		अगर ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			अगर (s->तथ्य.handle == शून्य)
				जारी;

			/* ensure script content fetch status is not an error */
			अगर (content_get_status(s->तथ्य.handle) ==
					CONTENT_STATUS_ERROR)
				जारी;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->तथ्य.handle));
			अगर (script_handler == शून्य)
				जारी; /* unsupported type */

			अगर (content_get_status(s->तथ्य.handle) ==
					CONTENT_STATUS_DONE) {
				/* external script is now available */
				स्थिर uint8_t *तथ्य;
				माप_प्रकार आकार;
				तथ्य = content_get_source_तथ्य(
						s->तथ्य.handle, &आकार );
				script_handler(c->jsthread, तथ्य, आकार,
					       nsurl_access(hlcache_handle_get_url(s->तथ्य.handle)));
				have_run_something = true;
				/* We have to re-acquire this here since the
				 * c->scripts array may have been reallocated
				 * as a result of executing this script.
				 */
				s = &(c->scripts[i]);

				s->already_started = true;

			}
		}
	}

	अगर (have_run_something) {
		वापस html_proceed_to_done(c);
	}

	वापस NSERROR_OK;
}

/* create new html script entry */
अटल काष्ठा html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			क्रमागत html_script_type type)
{
	काष्ठा html_script *nscript;
	/* add space for new script entry */
	nscript = पुनः_स्मृति(c->scripts,
			  माप(काष्ठा html_script) * (c->scripts_गिनती + 1));
	अगर (nscript == शून्य) {
		वापस शून्य;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_गिनती];
	c->scripts_गिनती++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	वापस nscript;
}

/**
 * Callback for asyncronous scripts
 */
अटल nserror
convert_script_async_cb(hlcache_handle *script,
			  स्थिर hlcache_event *event,
			  व्योम *pw)
{
	html_content *parent = pw;
	अचिन्हित पूर्णांक i;
	काष्ठा html_script *s;

	/* Find script */
	क्रम (i = 0, s = parent->scripts; i != parent->scripts_गिनती; i++, s++) {
		अगर (s->type == HTML_SCRIPT_ASYNC && s->तथ्य.handle == script)
			अवरोध;
	}

	निश्चित(i != parent->scripts_गिनती);

	चयन (event->type) {
	हाल CONTENT_संदेश_LOADING:
		अवरोध;

	हाल CONTENT_संदेश_READY:
		अवरोध;

	हाल CONTENT_संदेश_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		अवरोध;

	हाल CONTENT_संदेश_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->तथ्य.errordata.errormsg);

		hlcache_handle_release(script);
		s->तथ्य.handle = शून्य;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		अवरोध;

	शेष:
		अवरोध;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	अगर (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	अन्यथा अगर (parent->conversion_begun) {
		वापस html_script_exec(parent, false);
	}

	वापस NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
अटल nserror
convert_script_defer_cb(hlcache_handle *script,
			  स्थिर hlcache_event *event,
			  व्योम *pw)
{
	html_content *parent = pw;
	अचिन्हित पूर्णांक i;
	काष्ठा html_script *s;

	/* Find script */
	क्रम (i = 0, s = parent->scripts; i != parent->scripts_गिनती; i++, s++) {
		अगर (s->type == HTML_SCRIPT_DEFER && s->तथ्य.handle == script)
			अवरोध;
	}

	निश्चित(i != parent->scripts_गिनती);

	चयन (event->type) {

	हाल CONTENT_संदेश_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		अवरोध;

	हाल CONTENT_संदेश_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->तथ्य.errordata.errormsg);

		hlcache_handle_release(script);
		s->तथ्य.handle = शून्य;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		अवरोध;

	शेष:
		अवरोध;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	अगर (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	वापस NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
अटल nserror
convert_script_sync_cb(hlcache_handle *script,
			  स्थिर hlcache_event *event,
			  व्योम *pw)
{
	html_content *parent = pw;
	अचिन्हित पूर्णांक i;
	काष्ठा html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	अचिन्हित पूर्णांक active_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	क्रम (i = 0, s = parent->scripts; i != parent->scripts_गिनती; i++, s++) {
		अगर (s->type == HTML_SCRIPT_SYNC &&
		    s->तथ्य.handle != script && s->already_started == false) {
			active_sync_scripts++;
		}
	}

	/* Find script */
	क्रम (i = 0, s = parent->scripts; i != parent->scripts_गिनती; i++, s++) {
		अगर (s->type == HTML_SCRIPT_SYNC && s->तथ्य.handle == script)
			अवरोध;
	}

	निश्चित(i != parent->scripts_गिनती);

	चयन (event->type) {
	हाल CONTENT_संदेश_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->तथ्य.handle));
		अगर (script_handler != शून्य && parent->jsthread != शून्य) {
			/* script has a handler */
			स्थिर uint8_t *तथ्य;
			माप_प्रकार आकार;
			तथ्य = content_get_source_तथ्य(s->तथ्य.handle, &आकार );
			script_handler(parent->jsthread, तथ्य, आकार,
				       nsurl_access(hlcache_handle_get_url(s->तथ्य.handle)));
		}

		/* continue parse */
		अगर (parent->parser != शून्य && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			अगर (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		अवरोध;

	हाल CONTENT_संदेश_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->तथ्य.errordata.errormsg);

		hlcache_handle_release(script);
		s->तथ्य.handle = शून्य;
		parent->base.active--;

		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* continue parse */
		अगर (parent->parser != शून्य && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			अगर (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		अवरोध;

	शेष:
		अवरोध;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	अगर (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	वापस NSERROR_OK;
}

/**
 * process a script with a src tag
 */
अटल dom_hubbub_error
exec_src_script(html_content *c,
		dom_गाँठ *गाँठ,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_child_context child;
	काष्ठा html_script *nscript;
	bool async;
	bool defer;
	क्रमागत html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->base_url, dom_string_तथ्य(src), &joined);
	अगर (ns_error != NSERROR_OK) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, शून्य);
		वापस DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_गिनती,
	      nsurl_access(joined));

	/* there are three ways to process the script tag at this point:
	 *
	 * Syncronously  pause the parent parse and continue after
	 *                 the script has downloaded and executed. (default)
	 * Async         Start the script downloading and execute it when it
	 *                 becomes available.
	 * Defered       Start the script downloading and execute it when
	 *                 the page has completed parsing, may be set along
	 *                 with async where it is ignored.
	 */

	/* we interpret the presence of the async and defer attribute
	 * as true and ignore its value, technically only the empty
	 * value or the attribute name itself are valid. However
	 * various browsers interpret this in various ways the most
	 * compatible approach is to be liberal and accept any
	 * value. Note setting the values to "false" still makes them true!
	 */
	exc = dom_element_has_attribute(गाँठ, corestring_dom_async, &async);
	अगर (exc != DOM_NO_ERR) {
		वापस DOM_HUBBUB_OK; /* dom error */
	}

	अगर (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	अगर (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} अन्यथा {
		exc = dom_element_has_attribute(गाँठ,
						corestring_dom_defer, &defer);
		अगर (exc != DOM_NO_ERR) {
			वापस DOM_HUBBUB_OK; /* dom error */
		}

		अगर (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} अन्यथा {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	अगर (nscript == शून्य) {
		nsurl_unref(joined);
		content_broadcast_error(&c->base, NSERROR_NOMEM, शून्य);
		वापस DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->base),
					   शून्य,
					   script_cb,
					   c,
					   &child,
					   CONTENT_SCRIPT,
					   &nscript->तथ्य.handle);


	nsurl_unref(joined);

	अगर (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} अन्यथा {
		/* update base content active fetch count */
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		चयन (script_type) {
		हाल HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			अवरोध;

		हाल HTML_SCRIPT_ASYNC:
			अवरोध;

		हाल HTML_SCRIPT_DEFER:
			अवरोध;

		शेष:
			निश्चित(0);
		}
	}

	वापस ret;
}

अटल dom_hubbub_error
exec_inline_script(html_content *c, dom_गाँठ *गाँठ, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	काष्ठा lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	काष्ठा html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_गाँठ_get_text_content(गाँठ, &script);
	अगर ((exc != DOM_NO_ERR) || (script == शून्य)) {
		वापस DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	अगर (nscript == शून्य) {
		dom_string_unref(script);

		content_broadcast_error(&c->base, NSERROR_NOMEM, शून्य);
		वापस DOM_HUBBUB_NOMEM;

	}

	nscript->तथ्य.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	अगर (exc != DOM_NO_ERR) {
		वापस DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	अगर (script_handler != शून्य) {
		script_handler(c->jsthread,
			       (स्थिर uint8_t *)dom_string_तथ्य(script),
			       dom_string_byte_लंबाई(script),
			       "?inline script?");
	}
	वापस DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(व्योम *ctx, dom_गाँठ *गाँठ)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	अगर (c->jsthread == शून्य) {
		जोड़ content_संदेश_तथ्य संदेश_तथ्य;

		संदेश_तथ्य.jsthread = &c->jsthread;
		content_broadcast(&c->base, CONTENT_संदेश_GETTHREAD, &संदेश_तथ्य);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		अगर (c->jsthread == शून्य) {
			/* no context and it could not be created, abort */
			वापस DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      गाँठ);

	exc = dom_element_get_attribute(गाँठ, corestring_dom_type, &mimetype);
	अगर (exc != DOM_NO_ERR || mimetype == शून्य) {
		mimetype = dom_string_ref(corestring_dom_text_javascript);
	}

	exc = dom_element_get_attribute(गाँठ, corestring_dom_src, &src);
	अगर (exc != DOM_NO_ERR || src == शून्य) {
		err = exec_inline_script(c, गाँठ, mimetype);
	} अन्यथा {
		err = exec_src_script(c, गाँठ, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	वापस err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	काष्ठा html_script *s;
	अचिन्हित पूर्णांक i;

	क्रम (i = 0, s = htmlc->scripts; i != htmlc->scripts_गिनती; i++, s++) {
		अगर (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			जारी;
		}
		अगर (s->तथ्य.handle == शून्य) {
			/* We've not begun loading this? */
			जारी;
		}
		अगर (content_saw_insecure_objects(s->तथ्य.handle)) {
			वापस true;
		}
	}

	वापस false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	अचिन्हित पूर्णांक i;

	क्रम (i = 0; i != html->scripts_गिनती; i++) {
		अगर (html->scripts[i].mimetype != शून्य) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		चयन (html->scripts[i].type) {
		हाल HTML_SCRIPT_INLINE:
			अगर (html->scripts[i].तथ्य.string != शून्य) {
				dom_string_unref(html->scripts[i].तथ्य.string);
			}
			अवरोध;
		हाल HTML_SCRIPT_SYNC:
			/* fallthrough */
		हाल HTML_SCRIPT_ASYNC:
			/* fallthrough */
		हाल HTML_SCRIPT_DEFER:
			अगर (html->scripts[i].तथ्य.handle != शून्य) {
				hlcache_handle_release(html->scripts[i].तथ्य.handle);
			}
			अवरोध;
		}
	}
	मुक्त(html->scripts);

	वापस NSERROR_OK;
}
