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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "utils/config.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "javascript/js.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "content/fetch.h"
#அந்தர்பாவ "content/hlcache.h"

#அந்தர்பாவ "html/html.h"
#அந்தர்பாவ "html/private.h"

வகைவரை bool (script_handler_t)(கட்டமைப்பு jsthread *jsthread, மாறிலி uint8_t *data, size_t size, மாறிலி எழுத்து *name);


நிலையான script_handler_t *select_script_handler(content_type ctype)
{
	எனில் (ctype == CONTENT_JS) {
		திரும்பு js_exec;
	}
	திரும்பு வெற்று;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	unsigned முழு i;
	கட்டமைப்பு html_script *s;
	script_handler_t *script_handler;
	bool have_run_something = false;

	எனில் (c->jsthread == வெற்று) {
		திரும்பு NSERROR_BAD_PARAMETER;
	}

	க்கு (i = 0, s = c->scripts; i != c->scripts_மொத்தம்; i++, s++) {
		எனில் (s->already_started) {
			தொடர்;
		}

		எனில் ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			எனில் (s->data.handle == வெற்று)
				தொடர்;

			/* ensure script content fetch status is not an error */
			எனில் (content_get_status(s->data.handle) ==
					CONTENT_STATUS_ERROR)
				தொடர்;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->data.handle));
			எனில் (script_handler == வெற்று)
				தொடர்; /* unsupported type */

			எனில் (content_get_status(s->data.handle) ==
					CONTENT_STATUS_DONE) {
				/* external script is now available */
				மாறிலி uint8_t *data;
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

	எனில் (have_run_something) {
		திரும்பு html_proceed_to_done(c);
	}

	திரும்பு NSERROR_OK;
}

/* create new html script entry */
நிலையான கட்டமைப்பு html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			எண்ணிக்கை html_script_type type)
{
	கட்டமைப்பு html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  அளவு(கட்டமைப்பு html_script) * (c->scripts_மொத்தம் + 1));
	எனில் (nscript == வெற்று) {
		திரும்பு வெற்று;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_மொத்தம்];
	c->scripts_மொத்தம்++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	திரும்பு nscript;
}

/**
 * Callback for asyncronous scripts
 */
நிலையான nserror
convert_script_async_cb(hlcache_handle *script,
			  மாறிலி hlcache_event *event,
			  வெறுமை *pw)
{
	html_content *parent = pw;
	unsigned முழு i;
	கட்டமைப்பு html_script *s;

	/* Find script */
	க்கு (i = 0, s = parent->scripts; i != parent->scripts_மொத்தம்; i++, s++) {
		எனில் (s->type == HTML_SCRIPT_ASYNC && s->data.handle == script)
			நிறுத்து;
	}

	ப்ரதிஜ்ஞா(i != parent->scripts_மொத்தம்);

	தேர்வு (event->type) {
	நிலை CONTENT_செய்தி_LOADING:
		நிறுத்து;

	நிலை CONTENT_செய்தி_READY:
		நிறுத்து;

	நிலை CONTENT_செய்தி_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		நிறுத்து;

	நிலை CONTENT_செய்தி_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = வெற்று;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		நிறுத்து;

	இயல்பு:
		நிறுத்து;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	எனில் (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	இல்லையெனில் எனில் (parent->conversion_begun) {
		திரும்பு html_script_exec(parent, false);
	}

	திரும்பு NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
நிலையான nserror
convert_script_defer_cb(hlcache_handle *script,
			  மாறிலி hlcache_event *event,
			  வெறுமை *pw)
{
	html_content *parent = pw;
	unsigned முழு i;
	கட்டமைப்பு html_script *s;

	/* Find script */
	க்கு (i = 0, s = parent->scripts; i != parent->scripts_மொத்தம்; i++, s++) {
		எனில் (s->type == HTML_SCRIPT_DEFER && s->data.handle == script)
			நிறுத்து;
	}

	ப்ரதிஜ்ஞா(i != parent->scripts_மொத்தம்);

	தேர்வு (event->type) {

	நிலை CONTENT_செய்தி_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		நிறுத்து;

	நிலை CONTENT_செய்தி_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = வெற்று;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		நிறுத்து;

	இயல்பு:
		நிறுத்து;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	எனில் (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	திரும்பு NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
நிலையான nserror
convert_script_sync_cb(hlcache_handle *script,
			  மாறிலி hlcache_event *event,
			  வெறுமை *pw)
{
	html_content *parent = pw;
	unsigned முழு i;
	கட்டமைப்பு html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	unsigned முழு active_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	க்கு (i = 0, s = parent->scripts; i != parent->scripts_மொத்தம்; i++, s++) {
		எனில் (s->type == HTML_SCRIPT_SYNC &&
		    s->data.handle != script && s->already_started == false) {
			active_sync_scripts++;
		}
	}

	/* Find script */
	க்கு (i = 0, s = parent->scripts; i != parent->scripts_மொத்தம்; i++, s++) {
		எனில் (s->type == HTML_SCRIPT_SYNC && s->data.handle == script)
			நிறுத்து;
	}

	ப்ரதிஜ்ஞா(i != parent->scripts_மொத்தம்);

	தேர்வு (event->type) {
	நிலை CONTENT_செய்தி_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->data.handle));
		எனில் (script_handler != வெற்று && parent->jsthread != வெற்று) {
			/* script has a handler */
			மாறிலி uint8_t *data;
			size_t size;
			data = content_get_source_data(s->data.handle, &size );
			script_handler(parent->jsthread, data, size,
				       nsurl_access(hlcache_handle_get_url(s->data.handle)));
		}

		/* continue parse */
		எனில் (parent->parser != வெற்று && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			எனில் (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		நிறுத்து;

	நிலை CONTENT_செய்தி_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = வெற்று;
		parent->base.active--;

		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* continue parse */
		எனில் (parent->parser != வெற்று && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			எனில் (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		நிறுத்து;

	இயல்பு:
		நிறுத்து;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	எனில் (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	திரும்பு NSERROR_OK;
}

/**
 * process a script with a src tag
 */
நிலையான dom_hubbub_error
exec_src_script(html_content *c,
		dom_முனை *முனை,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_child_context child;
	கட்டமைப்பு html_script *nscript;
	bool async;
	bool defer;
	எண்ணிக்கை html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->base_url, dom_string_data(src), &joined);
	எனில் (ns_error != NSERROR_OK) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, வெற்று);
		திரும்பு DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_மொத்தம்,
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
	exc = dom_element_has_attribute(முனை, corestring_dom_async, &async);
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு DOM_HUBBUB_OK; /* dom error */
	}

	எனில் (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	எனில் (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} இல்லையெனில் {
		exc = dom_element_has_attribute(முனை,
						corestring_dom_defer, &defer);
		எனில் (exc != DOM_NO_ERR) {
			திரும்பு DOM_HUBBUB_OK; /* dom error */
		}

		எனில் (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} இல்லையெனில் {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	எனில் (nscript == வெற்று) {
		nsurl_unref(joined);
		content_broadcast_error(&c->base, NSERROR_NOMEM, வெற்று);
		திரும்பு DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->base),
					   வெற்று,
					   script_cb,
					   c,
					   &child,
					   CONTENT_SCRIPT,
					   &nscript->data.handle);


	nsurl_unref(joined);

	எனில் (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} இல்லையெனில் {
		/* update base content active fetch count */
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		தேர்வு (script_type) {
		நிலை HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			நிறுத்து;

		நிலை HTML_SCRIPT_ASYNC:
			நிறுத்து;

		நிலை HTML_SCRIPT_DEFER:
			நிறுத்து;

		இயல்பு:
			ப்ரதிஜ்ஞா(0);
		}
	}

	திரும்பு ret;
}

நிலையான dom_hubbub_error
exec_inline_script(html_content *c, dom_முனை *முனை, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	கட்டமைப்பு lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	கட்டமைப்பு html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_முனை_get_text_content(முனை, &script);
	எனில் ((exc != DOM_NO_ERR) || (script == வெற்று)) {
		திரும்பு DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	எனில் (nscript == வெற்று) {
		dom_string_unref(script);

		content_broadcast_error(&c->base, NSERROR_NOMEM, வெற்று);
		திரும்பு DOM_HUBBUB_NOMEM;

	}

	nscript->data.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	எனில் (script_handler != வெற்று) {
		script_handler(c->jsthread,
			       (மாறிலி uint8_t *)dom_string_data(script),
			       dom_string_byte_length(script),
			       "?inline script?");
	}
	திரும்பு DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(வெறுமை *ctx, dom_முனை *முனை)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	எனில் (c->jsthread == வெற்று) {
		ஒன்றிப்பு content_செய்தி_data செய்தி_data;

		செய்தி_data.jsthread = &c->jsthread;
		content_broadcast(&c->base, CONTENT_செய்தி_GETTHREAD, &செய்தி_data);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		எனில் (c->jsthread == வெற்று) {
			/* no context and it could not be created, abort */
			திரும்பு DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      முனை);

	exc = dom_element_get_attribute(முனை, corestring_dom_type, &mimetype);
	எனில் (exc != DOM_NO_ERR || mimetype == வெற்று) {
		mimetype = dom_string_ref(corestring_dom_text_javascript);
	}

	exc = dom_element_get_attribute(முனை, corestring_dom_src, &src);
	எனில் (exc != DOM_NO_ERR || src == வெற்று) {
		err = exec_inline_script(c, முனை, mimetype);
	} இல்லையெனில் {
		err = exec_src_script(c, முனை, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	திரும்பு err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	கட்டமைப்பு html_script *s;
	unsigned முழு i;

	க்கு (i = 0, s = htmlc->scripts; i != htmlc->scripts_மொத்தம்; i++, s++) {
		எனில் (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			தொடர்;
		}
		எனில் (s->data.handle == வெற்று) {
			/* We've not begun loading this? */
			தொடர்;
		}
		எனில் (content_saw_insecure_objects(s->data.handle)) {
			திரும்பு true;
		}
	}

	திரும்பு false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	unsigned முழு i;

	க்கு (i = 0; i != html->scripts_மொத்தம்; i++) {
		எனில் (html->scripts[i].mimetype != வெற்று) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		தேர்வு (html->scripts[i].type) {
		நிலை HTML_SCRIPT_INLINE:
			எனில் (html->scripts[i].data.string != வெற்று) {
				dom_string_unref(html->scripts[i].data.string);
			}
			நிறுத்து;
		நிலை HTML_SCRIPT_SYNC:
			/* fallthrough */
		நிலை HTML_SCRIPT_ASYNC:
			/* fallthrough */
		நிலை HTML_SCRIPT_DEFER:
			எனில் (html->scripts[i].data.handle != வெற்று) {
				hlcache_handle_release(html->scripts[i].data.handle);
			}
			நிறுத்து;
		}
	}
	விடுவி(html->scripts);

	திரும்பு NSERROR_OK;
}
