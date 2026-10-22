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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "utils/config.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "javascript/js.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "content/fetch.h"
#അന്തര്ഭാവ "content/hlcache.h"

#അന്തര്ഭാവ "html/html.h"
#അന്തര്ഭാവ "html/private.h"

തരനിർവചനം bool (script_handler_t)(ഘടന jsthread *jsthread, സ്ഥിരം uint8_t *data, size_t size, സ്ഥിരം അക്ഷരം *name);


സ്ഥിതം script_handler_t *select_script_handler(content_type ctype)
{
	എങ്കിൽ (ctype == CONTENT_JS) {
		തിരികെ js_exec;
	}
	തിരികെ ശൂന്യ;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	unsigned പൂർണ്ണസംഖ്യ i;
	ഘടന html_script *s;
	script_handler_t *script_handler;
	bool have_run_something = false;

	എങ്കിൽ (c->jsthread == ശൂന്യ) {
		തിരികെ NSERROR_BAD_PARAMETER;
	}

	വേണ്ടി (i = 0, s = c->scripts; i != c->scripts_എണ്ണം; i++, s++) {
		എങ്കിൽ (s->already_started) {
			തുടരുക;
		}

		എങ്കിൽ ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			എങ്കിൽ (s->data.handle == ശൂന്യ)
				തുടരുക;

			/* ensure script content fetch status is not an error */
			എങ്കിൽ (content_get_status(s->data.handle) ==
					CONTENT_STATUS_ERROR)
				തുടരുക;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->data.handle));
			എങ്കിൽ (script_handler == ശൂന്യ)
				തുടരുക; /* unsupported type */

			എങ്കിൽ (content_get_status(s->data.handle) ==
					CONTENT_STATUS_DONE) {
				/* external script is now available */
				സ്ഥിരം uint8_t *data;
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

	എങ്കിൽ (have_run_something) {
		തിരികെ html_proceed_to_done(c);
	}

	തിരികെ NSERROR_OK;
}

/* create new html script entry */
സ്ഥിതം ഘടന html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			എണ്ണൽ html_script_type type)
{
	ഘടന html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  വലുപ്പം(ഘടന html_script) * (c->scripts_എണ്ണം + 1));
	എങ്കിൽ (nscript == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_എണ്ണം];
	c->scripts_എണ്ണം++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	തിരികെ nscript;
}

/**
 * Callback for asyncronous scripts
 */
സ്ഥിതം nserror
convert_script_async_cb(hlcache_handle *script,
			  സ്ഥിരം hlcache_event *event,
			  ശൂന്യം *pw)
{
	html_content *parent = pw;
	unsigned പൂർണ്ണസംഖ്യ i;
	ഘടന html_script *s;

	/* Find script */
	വേണ്ടി (i = 0, s = parent->scripts; i != parent->scripts_എണ്ണം; i++, s++) {
		എങ്കിൽ (s->type == HTML_SCRIPT_ASYNC && s->data.handle == script)
			നിർത്തുക;
	}

	പ്രതിജ്ഞാ(i != parent->scripts_എണ്ണം);

	തിരഞ്ഞെടുപ്പ് (event->type) {
	അവസ്ഥ CONTENT_സന്ദേശം_LOADING:
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_READY:
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = ശൂന്യ;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	എങ്കിൽ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	അല്ലെങ്കിൽ എങ്കിൽ (parent->conversion_begun) {
		തിരികെ html_script_exec(parent, false);
	}

	തിരികെ NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
സ്ഥിതം nserror
convert_script_defer_cb(hlcache_handle *script,
			  സ്ഥിരം hlcache_event *event,
			  ശൂന്യം *pw)
{
	html_content *parent = pw;
	unsigned പൂർണ്ണസംഖ്യ i;
	ഘടന html_script *s;

	/* Find script */
	വേണ്ടി (i = 0, s = parent->scripts; i != parent->scripts_എണ്ണം; i++, s++) {
		എങ്കിൽ (s->type == HTML_SCRIPT_DEFER && s->data.handle == script)
			നിർത്തുക;
	}

	പ്രതിജ്ഞാ(i != parent->scripts_എണ്ണം);

	തിരഞ്ഞെടുപ്പ് (event->type) {

	അവസ്ഥ CONTENT_സന്ദേശം_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = ശൂന്യ;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	എങ്കിൽ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	തിരികെ NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
സ്ഥിതം nserror
convert_script_sync_cb(hlcache_handle *script,
			  സ്ഥിരം hlcache_event *event,
			  ശൂന്യം *pw)
{
	html_content *parent = pw;
	unsigned പൂർണ്ണസംഖ്യ i;
	ഘടന html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	unsigned പൂർണ്ണസംഖ്യ active_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	വേണ്ടി (i = 0, s = parent->scripts; i != parent->scripts_എണ്ണം; i++, s++) {
		എങ്കിൽ (s->type == HTML_SCRIPT_SYNC &&
		    s->data.handle != script && s->already_started == false) {
			active_sync_scripts++;
		}
	}

	/* Find script */
	വേണ്ടി (i = 0, s = parent->scripts; i != parent->scripts_എണ്ണം; i++, s++) {
		എങ്കിൽ (s->type == HTML_SCRIPT_SYNC && s->data.handle == script)
			നിർത്തുക;
	}

	പ്രതിജ്ഞാ(i != parent->scripts_എണ്ണം);

	തിരഞ്ഞെടുപ്പ് (event->type) {
	അവസ്ഥ CONTENT_സന്ദേശം_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->data.handle));
		എങ്കിൽ (script_handler != ശൂന്യ && parent->jsthread != ശൂന്യ) {
			/* script has a handler */
			സ്ഥിരം uint8_t *data;
			size_t size;
			data = content_get_source_data(s->data.handle, &size );
			script_handler(parent->jsthread, data, size,
				       nsurl_access(hlcache_handle_get_url(s->data.handle)));
		}

		/* continue parse */
		എങ്കിൽ (parent->parser != ശൂന്യ && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			എങ്കിൽ (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = ശൂന്യ;
		parent->base.active--;

		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* continue parse */
		എങ്കിൽ (parent->parser != ശൂന്യ && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			എങ്കിൽ (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	എങ്കിൽ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	തിരികെ NSERROR_OK;
}

/**
 * process a script with a src tag
 */
സ്ഥിതം dom_hubbub_error
exec_src_script(html_content *c,
		dom_കണ്ണി *കണ്ണി,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_child_context child;
	ഘടന html_script *nscript;
	bool async;
	bool defer;
	എണ്ണൽ html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->base_url, dom_string_data(src), &joined);
	എങ്കിൽ (ns_error != NSERROR_OK) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_എണ്ണം,
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
	exc = dom_element_has_attribute(കണ്ണി, corestring_dom_async, &async);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ DOM_HUBBUB_OK; /* dom error */
	}

	എങ്കിൽ (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	എങ്കിൽ (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} അല്ലെങ്കിൽ {
		exc = dom_element_has_attribute(കണ്ണി,
						corestring_dom_defer, &defer);
		എങ്കിൽ (exc != DOM_NO_ERR) {
			തിരികെ DOM_HUBBUB_OK; /* dom error */
		}

		എങ്കിൽ (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} അല്ലെങ്കിൽ {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	എങ്കിൽ (nscript == ശൂന്യ) {
		nsurl_unref(joined);
		content_broadcast_error(&c->base, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->base),
					   ശൂന്യ,
					   script_cb,
					   c,
					   &child,
					   CONTENT_SCRIPT,
					   &nscript->data.handle);


	nsurl_unref(joined);

	എങ്കിൽ (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} അല്ലെങ്കിൽ {
		/* update base content active fetch count */
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		തിരഞ്ഞെടുപ്പ് (script_type) {
		അവസ്ഥ HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			നിർത്തുക;

		അവസ്ഥ HTML_SCRIPT_ASYNC:
			നിർത്തുക;

		അവസ്ഥ HTML_SCRIPT_DEFER:
			നിർത്തുക;

		സ്ഥിരസ്ഥിതി:
			പ്രതിജ്ഞാ(0);
		}
	}

	തിരികെ ret;
}

സ്ഥിതം dom_hubbub_error
exec_inline_script(html_content *c, dom_കണ്ണി *കണ്ണി, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	ഘടന lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	ഘടന html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_കണ്ണി_get_text_content(കണ്ണി, &script);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (script == ശൂന്യ)) {
		തിരികെ DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	എങ്കിൽ (nscript == ശൂന്യ) {
		dom_string_unref(script);

		content_broadcast_error(&c->base, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ DOM_HUBBUB_NOMEM;

	}

	nscript->data.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	എങ്കിൽ (script_handler != ശൂന്യ) {
		script_handler(c->jsthread,
			       (സ്ഥിരം uint8_t *)dom_string_data(script),
			       dom_string_byte_length(script),
			       "?inline script?");
	}
	തിരികെ DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(ശൂന്യം *ctx, dom_കണ്ണി *കണ്ണി)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	എങ്കിൽ (c->jsthread == ശൂന്യ) {
		സംഘം content_സന്ദേശം_data സന്ദേശം_data;

		സന്ദേശം_data.jsthread = &c->jsthread;
		content_broadcast(&c->base, CONTENT_സന്ദേശം_GETTHREAD, &സന്ദേശം_data);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		എങ്കിൽ (c->jsthread == ശൂന്യ) {
			/* no context and it could not be created, abort */
			തിരികെ DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      കണ്ണി);

	exc = dom_element_get_attribute(കണ്ണി, corestring_dom_type, &mimetype);
	എങ്കിൽ (exc != DOM_NO_ERR || mimetype == ശൂന്യ) {
		mimetype = dom_string_ref(corestring_dom_text_javascript);
	}

	exc = dom_element_get_attribute(കണ്ണി, corestring_dom_src, &src);
	എങ്കിൽ (exc != DOM_NO_ERR || src == ശൂന്യ) {
		err = exec_inline_script(c, കണ്ണി, mimetype);
	} അല്ലെങ്കിൽ {
		err = exec_src_script(c, കണ്ണി, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	തിരികെ err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	ഘടന html_script *s;
	unsigned പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0, s = htmlc->scripts; i != htmlc->scripts_എണ്ണം; i++, s++) {
		എങ്കിൽ (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			തുടരുക;
		}
		എങ്കിൽ (s->data.handle == ശൂന്യ) {
			/* We've not begun loading this? */
			തുടരുക;
		}
		എങ്കിൽ (content_saw_insecure_objects(s->data.handle)) {
			തിരികെ true;
		}
	}

	തിരികെ false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	unsigned പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i != html->scripts_എണ്ണം; i++) {
		എങ്കിൽ (html->scripts[i].mimetype != ശൂന്യ) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		തിരഞ്ഞെടുപ്പ് (html->scripts[i].type) {
		അവസ്ഥ HTML_SCRIPT_INLINE:
			എങ്കിൽ (html->scripts[i].data.string != ശൂന്യ) {
				dom_string_unref(html->scripts[i].data.string);
			}
			നിർത്തുക;
		അവസ്ഥ HTML_SCRIPT_SYNC:
			/* fallthrough */
		അവസ്ഥ HTML_SCRIPT_ASYNC:
			/* fallthrough */
		അവസ്ഥ HTML_SCRIPT_DEFER:
			എങ്കിൽ (html->scripts[i].data.handle != ശൂന്യ) {
				hlcache_handle_release(html->scripts[i].data.handle);
			}
			നിർത്തുക;
		}
	}
	സ്വതന്ത്രം(html->scripts);

	തിരികെ NSERROR_OK;
}
