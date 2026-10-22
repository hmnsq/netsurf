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

प्रकारनिर्देश bool (script_handler_t)(संरचना jsthread *jsthread, ध्रुव uint8_t *data, size_t size, ध्रुव वर्ण *name);


स्थिर script_handler_t *select_script_handler(content_type ctype)
{
	जइ (ctype == CONTENT_JS) {
		प्रत्यागम js_exec;
	}
	प्रत्यागम रिक्त;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	unsigned पूर्णाङ्क i;
	संरचना html_script *s;
	script_handler_t *script_handler;
	bool have_run_something = false;

	जइ (c->jsthread == रिक्त) {
		प्रत्यागम NSERROR_BAD_PARAMETER;
	}

	प्रति (i = 0, s = c->scripts; i != c->scripts_गणनम्; i++, s++) {
		जइ (s->already_started) {
			अनुवर्त;
		}

		जइ ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			जइ (s->data.handle == रिक्त)
				अनुवर्त;

			/* ensure script content fetch status is not an error */
			जइ (content_get_status(s->data.handle) ==
					CONTENT_STATUS_ERROR)
				अनुवर्त;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->data.handle));
			जइ (script_handler == रिक्त)
				अनुवर्त; /* unsupported type */

			जइ (content_get_status(s->data.handle) ==
					CONTENT_STATUS_DONE) {
				/* external script is now available */
				ध्रुव uint8_t *data;
				size_t size;
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

	जइ (have_run_something) {
		प्रत्यागम html_proceed_to_done(c);
	}

	प्रत्यागम NSERROR_OK;
}

/* create new html script entry */
स्थिर संरचना html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			गणना html_script_type type)
{
	संरचना html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  परिमाण(संरचना html_script) * (c->scripts_गणनम् + 1));
	जइ (nscript == रिक्त) {
		प्रत्यागम रिक्त;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_गणनम्];
	c->scripts_गणनम्++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	प्रत्यागम nscript;
}

/**
 * Callback for asyncronous scripts
 */
स्थिर nserror
convert_script_async_cb(hlcache_handle *script,
			  ध्रुव hlcache_event *event,
			  शून्य *pw)
{
	html_content *parent = pw;
	unsigned पूर्णाङ्क i;
	संरचना html_script *s;

	/* Find script */
	प्रति (i = 0, s = parent->scripts; i != parent->scripts_गणनम्; i++, s++) {
		जइ (s->type == HTML_SCRIPT_ASYNC && s->data.handle == script)
			भङ्ग;
	}

	प्रतिज्ञा(i != parent->scripts_गणनम्);

	विकल्प (event->type) {
	पक्ष CONTENT_सन्देशः_LOADING:
		भङ्ग;

	पक्ष CONTENT_सन्देशः_READY:
		भङ्ग;

	पक्ष CONTENT_सन्देशः_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		भङ्ग;

	पक्ष CONTENT_सन्देशः_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = रिक्त;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		भङ्ग;

	स्वभाव:
		भङ्ग;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	जइ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	अण्णहा जइ (parent->conversion_begun) {
		प्रत्यागम html_script_exec(parent, false);
	}

	प्रत्यागम NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
स्थिर nserror
convert_script_defer_cb(hlcache_handle *script,
			  ध्रुव hlcache_event *event,
			  शून्य *pw)
{
	html_content *parent = pw;
	unsigned पूर्णाङ्क i;
	संरचना html_script *s;

	/* Find script */
	प्रति (i = 0, s = parent->scripts; i != parent->scripts_गणनम्; i++, s++) {
		जइ (s->type == HTML_SCRIPT_DEFER && s->data.handle == script)
			भङ्ग;
	}

	प्रतिज्ञा(i != parent->scripts_गणनम्);

	विकल्प (event->type) {

	पक्ष CONTENT_सन्देशः_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		भङ्ग;

	पक्ष CONTENT_सन्देशः_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = रिक्त;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		भङ्ग;

	स्वभाव:
		भङ्ग;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	जइ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	प्रत्यागम NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
स्थिर nserror
convert_script_sync_cb(hlcache_handle *script,
			  ध्रुव hlcache_event *event,
			  शून्य *pw)
{
	html_content *parent = pw;
	unsigned पूर्णाङ्क i;
	संरचना html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	unsigned पूर्णाङ्क active_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	प्रति (i = 0, s = parent->scripts; i != parent->scripts_गणनम्; i++, s++) {
		जइ (s->type == HTML_SCRIPT_SYNC &&
		    s->data.handle != script && s->already_started == false) {
			active_sync_scripts++;
		}
	}

	/* Find script */
	प्रति (i = 0, s = parent->scripts; i != parent->scripts_गणनम्; i++, s++) {
		जइ (s->type == HTML_SCRIPT_SYNC && s->data.handle == script)
			भङ्ग;
	}

	प्रतिज्ञा(i != parent->scripts_गणनम्);

	विकल्प (event->type) {
	पक्ष CONTENT_सन्देशः_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->data.handle));
		जइ (script_handler != रिक्त && parent->jsthread != रिक्त) {
			/* script has a handler */
			ध्रुव uint8_t *data;
			size_t size;
			data = content_get_source_data(s->data.handle, &size );
			script_handler(parent->jsthread, data, size,
				       nsurl_access(hlcache_handle_get_url(s->data.handle)));
		}

		/* continue parse */
		जइ (parent->parser != रिक्त && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			जइ (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		भङ्ग;

	पक्ष CONTENT_सन्देशः_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = रिक्त;
		parent->base.active--;

		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* continue parse */
		जइ (parent->parser != रिक्त && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			जइ (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		भङ्ग;

	स्वभाव:
		भङ्ग;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	जइ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	प्रत्यागम NSERROR_OK;
}

/**
 * process a script with a src tag
 */
स्थिर dom_hubbub_error
exec_src_script(html_content *c,
		dom_ग्रन्थिः *ग्रन्थिः,
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
	जइ (ns_error != NSERROR_OK) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, रिक्त);
		प्रत्यागम DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_गणनम्,
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
	exc = dom_element_has_attribute(ग्रन्थिः, corestring_dom_async, &async);
	जइ (exc != DOM_NO_ERR) {
		प्रत्यागम DOM_HUBBUB_OK; /* dom error */
	}

	जइ (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	जइ (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} अण्णहा {
		exc = dom_element_has_attribute(ग्रन्थिः,
						corestring_dom_defer, &defer);
		जइ (exc != DOM_NO_ERR) {
			प्रत्यागम DOM_HUBBUB_OK; /* dom error */
		}

		जइ (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} अण्णहा {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	जइ (nscript == रिक्त) {
		nsurl_unref(joined);
		content_broadcast_error(&c->base, NSERROR_NOMEM, रिक्त);
		प्रत्यागम DOM_HUBBUB_NOMEM;
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

	जइ (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} अण्णहा {
		/* update base content active fetch count */
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		विकल्प (script_type) {
		पक्ष HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			भङ्ग;

		पक्ष HTML_SCRIPT_ASYNC:
			भङ्ग;

		पक्ष HTML_SCRIPT_DEFER:
			भङ्ग;

		स्वभाव:
			प्रतिज्ञा(0);
		}
	}

	प्रत्यागम ret;
}

स्थिर dom_hubbub_error
exec_inline_script(html_content *c, dom_ग्रन्थिः *ग्रन्थिः, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	संरचना lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	संरचना html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_ग्रन्थिः_get_text_content(ग्रन्थिः, &script);
	जइ ((exc != DOM_NO_ERR) || (script == रिक्त)) {
		प्रत्यागम DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	जइ (nscript == रिक्त) {
		dom_string_unref(script);

		content_broadcast_error(&c->base, NSERROR_NOMEM, रिक्त);
		प्रत्यागम DOM_HUBBUB_NOMEM;

	}

	nscript->data.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	जइ (exc != DOM_NO_ERR) {
		प्रत्यागम DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	जइ (script_handler != रिक्त) {
		script_handler(c->jsthread,
			       (ध्रुव uint8_t *)dom_string_data(script),
			       dom_string_byte_length(script),
			       "?inline script?");
	}
	प्रत्यागम DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(शून्य *ctx, dom_ग्रन्थिः *ग्रन्थिः)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	जइ (c->jsthread == रिक्त) {
		सङ्घ content_सन्देशः_data सन्देशः_data;

		सन्देशः_data.jsthread = &c->jsthread;
		content_broadcast(&c->base, CONTENT_सन्देशः_GETTHREAD, &सन्देशः_data);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		जइ (c->jsthread == रिक्त) {
			/* no context and it could not be created, abort */
			प्रत्यागम DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      ग्रन्थिः);

	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_type, &mimetype);
	जइ (exc != DOM_NO_ERR || mimetype == रिक्त) {
		mimetype = dom_string_ref(corestring_dom_text_javascript);
	}

	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_src, &src);
	जइ (exc != DOM_NO_ERR || src == रिक्त) {
		err = exec_inline_script(c, ग्रन्थिः, mimetype);
	} अण्णहा {
		err = exec_src_script(c, ग्रन्थिः, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	प्रत्यागम err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	संरचना html_script *s;
	unsigned पूर्णाङ्क i;

	प्रति (i = 0, s = htmlc->scripts; i != htmlc->scripts_गणनम्; i++, s++) {
		जइ (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			अनुवर्त;
		}
		जइ (s->data.handle == रिक्त) {
			/* We've not begun loading this? */
			अनुवर्त;
		}
		जइ (content_saw_insecure_objects(s->data.handle)) {
			प्रत्यागम true;
		}
	}

	प्रत्यागम false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	unsigned पूर्णाङ्क i;

	प्रति (i = 0; i != html->scripts_गणनम्; i++) {
		जइ (html->scripts[i].mimetype != रिक्त) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		विकल्प (html->scripts[i].type) {
		पक्ष HTML_SCRIPT_INLINE:
			जइ (html->scripts[i].data.string != रिक्त) {
				dom_string_unref(html->scripts[i].data.string);
			}
			भङ्ग;
		पक्ष HTML_SCRIPT_SYNC:
			/* fallthrough */
		पक्ष HTML_SCRIPT_ASYNC:
			/* fallthrough */
		पक्ष HTML_SCRIPT_DEFER:
			जइ (html->scripts[i].data.handle != रिक्त) {
				hlcache_handle_release(html->scripts[i].data.handle);
			}
			भङ्ग;
		}
	}
	मुक्त(html->scripts);

	प्रत्यागम NSERROR_OK;
}
