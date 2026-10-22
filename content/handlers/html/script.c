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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "javascript/js.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/hlcache.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/html.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ bool (script_handler_t)(ਬਣਤਰ jsthread *jsthread, ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t ਮਾਪ, ਅਚਲ ਅੱਖਰ *name);


ਸਥਿਰ script_handler_t *select_script_handler(content_type ctype)
{
	ਜੇ (ctype == CONTENT_JS) {
		ਵਾਪਸ js_exec;
	}
	ਵਾਪਸ ਨਿਰਾ;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਬਣਤਰ html_script *s;
	script_handler_t *script_handler;
	bool have_ਲੜੀਵਾਰ_something = false;

	ਜੇ (c->jsthread == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_BAD_PARAMETER;
	}

	ਲਈ (i = 0, s = c->scripts; i != c->scripts_ਗਿਣਤੀ; i++, s++) {
		ਜੇ (s->already_started) {
			ਜਾਰੀ;
		}

		ਜੇ ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			ਜੇ (s->ਅੰਕੜੇ.handle == ਨਿਰਾ)
				ਜਾਰੀ;

			/* ensure script content fetch status is not an error */
			ਜੇ (content_get_ਸਥਿਤੀ(s->ਅੰਕੜੇ.handle) ==
					CONTENT_ਸਥਿਤੀ_ERROR)
				ਜਾਰੀ;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->ਅੰਕੜੇ.handle));
			ਜੇ (script_handler == ਨਿਰਾ)
				ਜਾਰੀ; /* unsupported type */

			ਜੇ (content_get_ਸਥਿਤੀ(s->ਅੰਕੜੇ.handle) ==
					CONTENT_ਸਥਿਤੀ_ਪੂਰਾ) {
				/* external script is now available */
				ਅਚਲ uint8_t *ਅੰਕੜੇ;
				size_t ਮਾਪ;
				ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ(
						s->ਅੰਕੜੇ.handle, &ਮਾਪ );
				script_handler(c->jsthread, ਅੰਕੜੇ, ਮਾਪ,
					       nsurl_access(hlcache_handle_get_url(s->ਅੰਕੜੇ.handle)));
				have_ਲੜੀਵਾਰ_something = true;
				/* We have to re-acquire this here since the
				 * c->scripts array may have been reallocated
				 * as a result of executing this script.
				 */
				s = &(c->scripts[i]);

				s->already_started = true;

			}
		}
	}

	ਜੇ (have_ਲੜੀਵਾਰ_something) {
		ਵਾਪਸ html_proceed_ਮੰਜ਼ਿਲ_ਪੂਰਾ(c);
	}

	ਵਾਪਸ NSERROR_OK;
}

/* create new html script entry */
ਸਥਿਰ ਬਣਤਰ html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			ਗਿਣਤੀ html_script_type type)
{
	ਬਣਤਰ html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  ਆਕਾਰ(ਬਣਤਰ html_script) * (c->scripts_ਗਿਣਤੀ + 1));
	ਜੇ (nscript == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_ਗਿਣਤੀ];
	c->scripts_ਗਿਣਤੀ++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	ਵਾਪਸ nscript;
}

/**
 * Callback for asyncronous scripts
 */
ਸਥਿਰ nserror
convert_script_async_cb(hlcache_handle *script,
			  ਅਚਲ hlcache_event *event,
			  ਖਾਲੀ *pw)
{
	html_content *ਮਾਪਾ = pw;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਬਣਤਰ html_script *s;

	/* Find script */
	ਲਈ (i = 0, s = ਮਾਪਾ->scripts; i != ਮਾਪਾ->scripts_ਗਿਣਤੀ; i++, s++) {
		ਜੇ (s->type == HTML_SCRIPT_ASYNC && s->ਅੰਕੜੇ.handle == script)
			ਰੋਕੋ;
	}

	ਪ੍ਰਤਿਜ੍ਞਾ(i != ਮਾਪਾ->scripts_ਗਿਣਤੀ);

	ਚੋਣ (event->type) {
	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_LOADING:
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_READY:
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ਪੂਰਾ:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ--;
		NSLOG(netsurf, INFO, "%d fetches active", ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ);

		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->ਅੰਕੜੇ.errordata.errormsg);

		hlcache_handle_release(script);
		s->ਅੰਕੜੇ.handle = ਨਿਰਾ;
		ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ--;
		NSLOG(netsurf, INFO, "%d fetches active", ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ);

		ਰੋਕੋ;

	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ਜੇ (html_can_begin_conversion(ਮਾਪਾ)) {
		html_begin_conversion(ਮਾਪਾ);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪਾ->conversion_begun) {
		ਵਾਪਸ html_script_exec(ਮਾਪਾ, false);
	}

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
ਸਥਿਰ nserror
convert_script_defer_cb(hlcache_handle *script,
			  ਅਚਲ hlcache_event *event,
			  ਖਾਲੀ *pw)
{
	html_content *ਮਾਪਾ = pw;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਬਣਤਰ html_script *s;

	/* Find script */
	ਲਈ (i = 0, s = ਮਾਪਾ->scripts; i != ਮਾਪਾ->scripts_ਗਿਣਤੀ; i++, s++) {
		ਜੇ (s->type == HTML_SCRIPT_DEFER && s->ਅੰਕੜੇ.handle == script)
			ਰੋਕੋ;
	}

	ਪ੍ਰਤਿਜ੍ਞਾ(i != ਮਾਪਾ->scripts_ਗਿਣਤੀ);

	ਚੋਣ (event->type) {

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ਪੂਰਾ:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ--;
		NSLOG(netsurf, INFO, "%d fetches active", ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ);

		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->ਅੰਕੜੇ.errordata.errormsg);

		hlcache_handle_release(script);
		s->ਅੰਕੜੇ.handle = ਨਿਰਾ;
		ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ--;
		NSLOG(netsurf, INFO, "%d fetches active", ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ);

		ਰੋਕੋ;

	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ਜੇ (html_can_begin_conversion(ਮਾਪਾ)) {
		html_begin_conversion(ਮਾਪਾ);
	}

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
ਸਥਿਰ nserror
convert_script_sync_cb(hlcache_handle *script,
			  ਅਚਲ hlcache_event *event,
			  ਖਾਲੀ *pw)
{
	html_content *ਮਾਪਾ = pw;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਬਣਤਰ html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸਰਗਰਮ_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	ਲਈ (i = 0, s = ਮਾਪਾ->scripts; i != ਮਾਪਾ->scripts_ਗਿਣਤੀ; i++, s++) {
		ਜੇ (s->type == HTML_SCRIPT_SYNC &&
		    s->ਅੰਕੜੇ.handle != script && s->already_started == false) {
			ਸਰਗਰਮ_sync_scripts++;
		}
	}

	/* Find script */
	ਲਈ (i = 0, s = ਮਾਪਾ->scripts; i != ਮਾਪਾ->scripts_ਗਿਣਤੀ; i++, s++) {
		ਜੇ (s->type == HTML_SCRIPT_SYNC && s->ਅੰਕੜੇ.handle == script)
			ਰੋਕੋ;
	}

	ਪ੍ਰਤਿਜ੍ਞਾ(i != ਮਾਪਾ->scripts_ਗਿਣਤੀ);

	ਚੋਣ (event->type) {
	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ਪੂਰਾ:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ--;
		NSLOG(netsurf, INFO, "%d fetches active", ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->ਅੰਕੜੇ.handle));
		ਜੇ (script_handler != ਨਿਰਾ && ਮਾਪਾ->jsthread != ਨਿਰਾ) {
			/* script has a handler */
			ਅਚਲ uint8_t *ਅੰਕੜੇ;
			size_t ਮਾਪ;
			ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ(s->ਅੰਕੜੇ.handle, &ਮਾਪ );
			script_handler(ਮਾਪਾ->jsthread, ਅੰਕੜੇ, ਮਾਪ,
				       nsurl_access(hlcache_handle_get_url(s->ਅੰਕੜੇ.handle)));
		}

		/* continue parse */
		ਜੇ (ਮਾਪਾ->parser != ਨਿਰਾ && ਸਰਗਰਮ_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(ਮਾਪਾ->parser, false);
			ਜੇ (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->ਅੰਕੜੇ.errordata.errormsg);

		hlcache_handle_release(script);
		s->ਅੰਕੜੇ.handle = ਨਿਰਾ;
		ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ--;

		NSLOG(netsurf, INFO, "%d fetches active", ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ);

		s->already_started = true;

		/* continue parse */
		ਜੇ (ਮਾਪਾ->parser != ਨਿਰਾ && ਸਰਗਰਮ_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(ਮਾਪਾ->parser, false);
			ਜੇ (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		ਰੋਕੋ;

	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ਜੇ (html_can_begin_conversion(ਮਾਪਾ)) {
		html_begin_conversion(ਮਾਪਾ);
	}

	ਵਾਪਸ NSERROR_OK;
}

/**
 * process a script with a src tag
 */
ਸਥਿਰ dom_hubbub_error
exec_src_script(html_content *c,
		dom_ਗੰਢ *ਗੰਢ,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_ਬੱਚਾ_context ਬੱਚਾ;
	ਬਣਤਰ html_script *nscript;
	bool async;
	bool defer;
	ਗਿਣਤੀ html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->ਅਧਾਰ_url, dom_string_ਅੰਕੜੇ(src), &joined);
	ਜੇ (ns_error != NSERROR_OK) {
		content_ਪ੍ਰਸਾਰਣ_error(&c->ਅਧਾਰ, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_ਗਿਣਤੀ,
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
	exc = dom_element_has_attribute(ਗੰਢ, corestring_dom_async, &async);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ DOM_HUBBUB_OK; /* dom error */
	}

	ਜੇ (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	ਜੇ (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} ਨਹੀਂਤਾਂ {
		exc = dom_element_has_attribute(ਗੰਢ,
						corestring_dom_defer, &defer);
		ਜੇ (exc != DOM_NO_ERR) {
			ਵਾਪਸ DOM_HUBBUB_OK; /* dom error */
		}

		ਜੇ (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} ਨਹੀਂਤਾਂ {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	ਜੇ (nscript == ਨਿਰਾ) {
		nsurl_unref(joined);
		content_ਪ੍ਰਸਾਰਣ_error(&c->ਅਧਾਰ, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	ਬੱਚਾ.charset = c->encoding;
	ਬੱਚਾ.quirks = c->ਅਧਾਰ.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->ਅਧਾਰ),
					   ਨਿਰਾ,
					   script_cb,
					   c,
					   &ਬੱਚਾ,
					   CONTENT_SCRIPT,
					   &nscript->ਅੰਕੜੇ.handle);


	nsurl_unref(joined);

	ਜੇ (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} ਨਹੀਂਤਾਂ {
		/* update base content active fetch count */
		c->ਅਧਾਰ.ਸਰਗਰਮ++;
		NSLOG(netsurf, INFO, "%d fetches active", c->ਅਧਾਰ.ਸਰਗਰਮ);

		ਚੋਣ (script_type) {
		ਹਾਲਤ HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			ਰੋਕੋ;

		ਹਾਲਤ HTML_SCRIPT_ASYNC:
			ਰੋਕੋ;

		ਹਾਲਤ HTML_SCRIPT_DEFER:
			ਰੋਕੋ;

		ਮੂਲ:
			ਪ੍ਰਤਿਜ੍ਞਾ(0);
		}
	}

	ਵਾਪਸ ret;
}

ਸਥਿਰ dom_hubbub_error
exec_inline_script(html_content *c, dom_ਗੰਢ *ਗੰਢ, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	ਬਣਤਰ lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	ਬਣਤਰ html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_ਗੰਢ_get_ਪਾਠ_content(ਗੰਢ, &script);
	ਜੇ ((exc != DOM_NO_ERR) || (script == ਨਿਰਾ)) {
		ਵਾਪਸ DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	ਜੇ (nscript == ਨਿਰਾ) {
		dom_string_unref(script);

		content_ਪ੍ਰਸਾਰਣ_error(&c->ਅਧਾਰ, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ DOM_HUBBUB_NOMEM;

	}

	nscript->ਅੰਕੜੇ.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	ਜੇ (script_handler != ਨਿਰਾ) {
		script_handler(c->jsthread,
			       (ਅਚਲ uint8_t *)dom_string_ਅੰਕੜੇ(script),
			       dom_string_byte_ਲੰਮਾਈ(script),
			       "?inline script?");
	}
	ਵਾਪਸ DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(ਖਾਲੀ *ctx, dom_ਗੰਢ *ਗੰਢ)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	ਜੇ (c->jsthread == ਨਿਰਾ) {
		ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;

		ਸੁਨੇਹਾ_ਅੰਕੜੇ.jsthread = &c->jsthread;
		content_ਪ੍ਰਸਾਰਣ(&c->ਅਧਾਰ, CONTENT_ਸੁਨੇਹਾ_GETTHREAD, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		ਜੇ (c->jsthread == ਨਿਰਾ) {
			/* no context and it could not be created, abort */
			ਵਾਪਸ DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      ਗੰਢ);

	exc = dom_element_get_attribute(ਗੰਢ, corestring_dom_type, &mimetype);
	ਜੇ (exc != DOM_NO_ERR || mimetype == ਨਿਰਾ) {
		mimetype = dom_string_ref(corestring_dom_ਪਾਠ_javascript);
	}

	exc = dom_element_get_attribute(ਗੰਢ, corestring_dom_src, &src);
	ਜੇ (exc != DOM_NO_ERR || src == ਨਿਰਾ) {
		err = exec_inline_script(c, ਗੰਢ, mimetype);
	} ਨਹੀਂਤਾਂ {
		err = exec_src_script(c, ਗੰਢ, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	ਵਾਪਸ err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	ਬਣਤਰ html_script *s;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	ਲਈ (i = 0, s = htmlc->scripts; i != htmlc->scripts_ਗਿਣਤੀ; i++, s++) {
		ਜੇ (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			ਜਾਰੀ;
		}
		ਜੇ (s->ਅੰਕੜੇ.handle == ਨਿਰਾ) {
			/* We've not begun loading this? */
			ਜਾਰੀ;
		}
		ਜੇ (content_saw_insecure_objects(s->ਅੰਕੜੇ.handle)) {
			ਵਾਪਸ true;
		}
	}

	ਵਾਪਸ false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	ਲਈ (i = 0; i != html->scripts_ਗਿਣਤੀ; i++) {
		ਜੇ (html->scripts[i].mimetype != ਨਿਰਾ) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		ਚੋਣ (html->scripts[i].type) {
		ਹਾਲਤ HTML_SCRIPT_INLINE:
			ਜੇ (html->scripts[i].ਅੰਕੜੇ.string != ਨਿਰਾ) {
				dom_string_unref(html->scripts[i].ਅੰਕੜੇ.string);
			}
			ਰੋਕੋ;
		ਹਾਲਤ HTML_SCRIPT_SYNC:
			/* fallthrough */
		ਹਾਲਤ HTML_SCRIPT_ASYNC:
			/* fallthrough */
		ਹਾਲਤ HTML_SCRIPT_DEFER:
			ਜੇ (html->scripts[i].ਅੰਕੜੇ.handle != ਨਿਰਾ) {
				hlcache_handle_release(html->scripts[i].ਅੰਕੜੇ.handle);
			}
			ਰੋਕੋ;
		}
	}
	ਮੁਕਤ(html->scripts);

	ਵਾਪਸ NSERROR_OK;
}
