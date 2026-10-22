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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "utils/config.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "javascript/js.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "content/fetch.h"
#अन्तर्भाव "content/hlcache.h"

#अन्तर्भाव "html/html.h"
#अन्तर्भाव "html/private.h"

प्रकारव्याख्या bool (script_handler_t)(संरचना jsthread *jsthread, स्थिरांक uint8_t *data, माप_प्रकार size, स्थिरांक अक्षर *name);


स्थिर script_handler_t *select_script_handler(content_type ctype)
{
	जर (ctype == CONTENT_JS) {
		परत js_exec;
	}
	परत रिक्त;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	अचिन्हित पूर्णांक i;
	संरचना html_script *s;
	script_handler_t *script_handler;
	bool have_run_something = false;

	जर (c->jsthread == रिक्त) {
		परत NSERROR_BAD_PARAMETER;
	}

	साठी (i = 0, s = c->scripts; i != c->scripts_मोजणी; i++, s++) {
		जर (s->already_started) {
			सुरूठेवा;
		}

		जर ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			जर (s->data.handle == रिक्त)
				सुरूठेवा;

			/* ensure script content fetch status is not an error */
			जर (content_get_status(s->data.handle) ==
					CONTENT_STATUS_ERROR)
				सुरूठेवा;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->data.handle));
			जर (script_handler == रिक्त)
				सुरूठेवा; /* unsupported type */

			जर (content_get_status(s->data.handle) ==
					CONTENT_STATUS_DONE) {
				/* external script is now available */
				स्थिरांक uint8_t *data;
				माप_प्रकार size;
				data = content_get_source_data(
						s->data.handle, &size );
				script_handler(c->jsthread, data, size,
					       nsurl_access(hlcache_handle_get_url(s->data.handle)));
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

	जर (have_run_something) {
		परत html_proceed_to_done(c);
	}

	परत NSERROR_OK;
}

/* create new html script entry */
स्थिर संरचना html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			गणना html_script_type type)
{
	संरचना html_script *nscript;
	/* add space for new script entry */
	nscript = पुनः_स्मृति(c->scripts,
			  आकार(संरचना html_script) * (c->scripts_मोजणी + 1));
	जर (nscript == रिक्त) {
		परत रिक्त;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_मोजणी];
	c->scripts_मोजणी++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	परत nscript;
}

/**
 * Callback for asyncronous scripts
 */
स्थिर nserror
convert_script_async_cb(hlcache_handle *script,
			  स्थिरांक hlcache_event *event,
			  शून्य *pw)
{
	html_content *parent = pw;
	अचिन्हित पूर्णांक i;
	संरचना html_script *s;

	/* Find script */
	साठी (i = 0, s = parent->scripts; i != parent->scripts_मोजणी; i++, s++) {
		जर (s->type == HTML_SCRIPT_ASYNC && s->data.handle == script)
			थांबा;
	}

	प्रतिज्ञा(i != parent->scripts_मोजणी);

	निवड (event->type) {
	स्थिती CONTENT_संदेश_LOADING:
		थांबा;

	स्थिती CONTENT_संदेश_READY:
		थांबा;

	स्थिती CONTENT_संदेश_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		थांबा;

	स्थिती CONTENT_संदेश_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = रिक्त;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		थांबा;

	पूर्वनिर्धारित:
		थांबा;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	जर (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	अन्यथा जर (parent->conversion_begun) {
		परत html_script_exec(parent, false);
	}

	परत NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
स्थिर nserror
convert_script_defer_cb(hlcache_handle *script,
			  स्थिरांक hlcache_event *event,
			  शून्य *pw)
{
	html_content *parent = pw;
	अचिन्हित पूर्णांक i;
	संरचना html_script *s;

	/* Find script */
	साठी (i = 0, s = parent->scripts; i != parent->scripts_मोजणी; i++, s++) {
		जर (s->type == HTML_SCRIPT_DEFER && s->data.handle == script)
			थांबा;
	}

	प्रतिज्ञा(i != parent->scripts_मोजणी);

	निवड (event->type) {

	स्थिती CONTENT_संदेश_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		थांबा;

	स्थिती CONTENT_संदेश_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = रिक्त;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		थांबा;

	पूर्वनिर्धारित:
		थांबा;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	जर (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	परत NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
स्थिर nserror
convert_script_sync_cb(hlcache_handle *script,
			  स्थिरांक hlcache_event *event,
			  शून्य *pw)
{
	html_content *parent = pw;
	अचिन्हित पूर्णांक i;
	संरचना html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	अचिन्हित पूर्णांक active_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	साठी (i = 0, s = parent->scripts; i != parent->scripts_मोजणी; i++, s++) {
		जर (s->type == HTML_SCRIPT_SYNC &&
		    s->data.handle != script && s->already_started == false) {
			active_sync_scripts++;
		}
	}

	/* Find script */
	साठी (i = 0, s = parent->scripts; i != parent->scripts_मोजणी; i++, s++) {
		जर (s->type == HTML_SCRIPT_SYNC && s->data.handle == script)
			थांबा;
	}

	प्रतिज्ञा(i != parent->scripts_मोजणी);

	निवड (event->type) {
	स्थिती CONTENT_संदेश_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->data.handle));
		जर (script_handler != रिक्त && parent->jsthread != रिक्त) {
			/* script has a handler */
			स्थिरांक uint8_t *data;
			माप_प्रकार size;
			data = content_get_source_data(s->data.handle, &size );
			script_handler(parent->jsthread, data, size,
				       nsurl_access(hlcache_handle_get_url(s->data.handle)));
		}

		/* continue parse */
		जर (parent->parser != रिक्त && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			जर (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		थांबा;

	स्थिती CONTENT_संदेश_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = रिक्त;
		parent->base.active--;

		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* continue parse */
		जर (parent->parser != रिक्त && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			जर (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		थांबा;

	पूर्वनिर्धारित:
		थांबा;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	जर (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	परत NSERROR_OK;
}

/**
 * process a script with a src tag
 */
स्थिर dom_hubbub_error
exec_src_script(html_content *c,
		dom_गाठ *गाठ,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_child_context child;
	संरचना html_script *nscript;
	bool async;
	bool defer;
	गणना html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->base_url, dom_string_data(src), &joined);
	जर (ns_error != NSERROR_OK) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, रिक्त);
		परत DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_मोजणी,
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
	exc = dom_element_has_attribute(गाठ, corestring_dom_async, &async);
	जर (exc != DOM_NO_ERR) {
		परत DOM_HUBBUB_OK; /* dom error */
	}

	जर (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	जर (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} अन्यथा {
		exc = dom_element_has_attribute(गाठ,
						corestring_dom_defer, &defer);
		जर (exc != DOM_NO_ERR) {
			परत DOM_HUBBUB_OK; /* dom error */
		}

		जर (defer) {
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
	जर (nscript == रिक्त) {
		nsurl_unref(joined);
		content_broadcast_error(&c->base, NSERROR_NOMEM, रिक्त);
		परत DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->base),
					   रिक्त,
					   script_cb,
					   c,
					   &child,
					   CONTENT_SCRIPT,
					   &nscript->data.handle);


	nsurl_unref(joined);

	जर (ns_error != NSERROR_OK) {
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

		निवड (script_type) {
		स्थिती HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			थांबा;

		स्थिती HTML_SCRIPT_ASYNC:
			थांबा;

		स्थिती HTML_SCRIPT_DEFER:
			थांबा;

		पूर्वनिर्धारित:
			प्रतिज्ञा(0);
		}
	}

	परत ret;
}

स्थिर dom_hubbub_error
exec_inline_script(html_content *c, dom_गाठ *गाठ, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	संरचना lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	संरचना html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_गाठ_get_text_content(गाठ, &script);
	जर ((exc != DOM_NO_ERR) || (script == रिक्त)) {
		परत DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	जर (nscript == रिक्त) {
		dom_string_unref(script);

		content_broadcast_error(&c->base, NSERROR_NOMEM, रिक्त);
		परत DOM_HUBBUB_NOMEM;

	}

	nscript->data.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	जर (exc != DOM_NO_ERR) {
		परत DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	जर (script_handler != रिक्त) {
		script_handler(c->jsthread,
			       (स्थिरांक uint8_t *)dom_string_data(script),
			       dom_string_byte_length(script),
			       "?inline script?");
	}
	परत DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(शून्य *ctx, dom_गाठ *गाठ)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	जर (c->jsthread == रिक्त) {
		संघ content_संदेश_data संदेश_data;

		संदेश_data.jsthread = &c->jsthread;
		content_broadcast(&c->base, CONTENT_संदेश_GETTHREAD, &संदेश_data);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		जर (c->jsthread == रिक्त) {
			/* no context and it could not be created, abort */
			परत DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      गाठ);

	exc = dom_element_get_attribute(गाठ, corestring_dom_type, &mimetype);
	जर (exc != DOM_NO_ERR || mimetype == रिक्त) {
		mimetype = dom_string_ref(corestring_dom_text_javascript);
	}

	exc = dom_element_get_attribute(गाठ, corestring_dom_src, &src);
	जर (exc != DOM_NO_ERR || src == रिक्त) {
		err = exec_inline_script(c, गाठ, mimetype);
	} अन्यथा {
		err = exec_src_script(c, गाठ, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	परत err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	संरचना html_script *s;
	अचिन्हित पूर्णांक i;

	साठी (i = 0, s = htmlc->scripts; i != htmlc->scripts_मोजणी; i++, s++) {
		जर (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			सुरूठेवा;
		}
		जर (s->data.handle == रिक्त) {
			/* We've not begun loading this? */
			सुरूठेवा;
		}
		जर (content_saw_insecure_objects(s->data.handle)) {
			परत true;
		}
	}

	परत false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	अचिन्हित पूर्णांक i;

	साठी (i = 0; i != html->scripts_मोजणी; i++) {
		जर (html->scripts[i].mimetype != रिक्त) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		निवड (html->scripts[i].type) {
		स्थिती HTML_SCRIPT_INLINE:
			जर (html->scripts[i].data.string != रिक्त) {
				dom_string_unref(html->scripts[i].data.string);
			}
			थांबा;
		स्थिती HTML_SCRIPT_SYNC:
			/* fallthrough */
		स्थिती HTML_SCRIPT_ASYNC:
			/* fallthrough */
		स्थिती HTML_SCRIPT_DEFER:
			जर (html->scripts[i].data.handle != रिक्त) {
				hlcache_handle_release(html->scripts[i].data.handle);
			}
			थांबा;
		}
	}
	मुक्त(html->scripts);

	परत NSERROR_OK;
}
