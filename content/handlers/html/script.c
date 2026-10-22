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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "utils/config.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "javascript/js.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"

#ಅನ್ತರ್ಭಾವ "html/html.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"

ಪ್ರಕಾರನಿರ್ದೇಶ bool (script_handler_t)(ಸಂರಚನಾ jsthread *jsthread, ಧ್ರುವ uint8_t *data, size_t size, ಧ್ರುವ ವರ್ಣ *name);


ಸ್ಥಿರ script_handler_t *select_script_handler(content_type ctype)
{
	ಯದಿ (ctype == CONTENT_JS) {
		ಪ್ರತ್ಯಾಗಮ js_exec;
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ html_script *s;
	script_handler_t *script_handler;
	bool have_run_something = false;

	ಯದಿ (c->jsthread == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_BAD_PARAMETER;
	}

	ಪ್ರತಿ (i = 0, s = c->scripts; i != c->scripts_ಎಣಿಕೆ; i++, s++) {
		ಯದಿ (s->already_started) {
			ಅನುವರ್ತ;
		}

		ಯದಿ ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			ಯದಿ (s->data.handle == ರಿಕ್ತ)
				ಅನುವರ್ತ;

			/* ensure script content fetch status is not an error */
			ಯದಿ (content_get_status(s->data.handle) ==
					CONTENT_STATUS_ERROR)
				ಅನುವರ್ತ;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->data.handle));
			ಯದಿ (script_handler == ರಿಕ್ತ)
				ಅನುವರ್ತ; /* unsupported type */

			ಯದಿ (content_get_status(s->data.handle) ==
					CONTENT_STATUS_DONE) {
				/* external script is now available */
				ಧ್ರುವ uint8_t *data;
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

	ಯದಿ (have_run_something) {
		ಪ್ರತ್ಯಾಗಮ html_proceed_to_done(c);
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/* create new html script entry */
ಸ್ಥಿರ ಸಂರಚನಾ html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			ಗಣನಾ html_script_type type)
{
	ಸಂರಚನಾ html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  ಪರಿಮಾಣ(ಸಂರಚನಾ html_script) * (c->scripts_ಎಣಿಕೆ + 1));
	ಯದಿ (nscript == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_ಎಣಿಕೆ];
	c->scripts_ಎಣಿಕೆ++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	ಪ್ರತ್ಯಾಗಮ nscript;
}

/**
 * Callback for asyncronous scripts
 */
ಸ್ಥಿರ nserror
convert_script_async_cb(hlcache_handle *script,
			  ಧ್ರುವ hlcache_event *event,
			  ಶೂನ್ಯ *pw)
{
	html_content *parent = pw;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ html_script *s;

	/* Find script */
	ಪ್ರತಿ (i = 0, s = parent->scripts; i != parent->scripts_ಎಣಿಕೆ; i++, s++) {
		ಯದಿ (s->type == HTML_SCRIPT_ASYNC && s->data.handle == script)
			ಭಙ್ಗ;
	}

	ಪ್ರತಿಜ್ಞಾ(i != parent->scripts_ಎಣಿಕೆ);

	ವಿಕಲ್ಪ (event->type) {
	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_LOADING:
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_READY:
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = ರಿಕ್ತ;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ಯದಿ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	ಅನ್ಯಥಾ ಯದಿ (parent->conversion_begun) {
		ಪ್ರತ್ಯಾಗಮ html_script_exec(parent, false);
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
ಸ್ಥಿರ nserror
convert_script_defer_cb(hlcache_handle *script,
			  ಧ್ರುವ hlcache_event *event,
			  ಶೂನ್ಯ *pw)
{
	html_content *parent = pw;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ html_script *s;

	/* Find script */
	ಪ್ರತಿ (i = 0, s = parent->scripts; i != parent->scripts_ಎಣಿಕೆ; i++, s++) {
		ಯದಿ (s->type == HTML_SCRIPT_DEFER && s->data.handle == script)
			ಭಙ್ಗ;
	}

	ಪ್ರತಿಜ್ಞಾ(i != parent->scripts_ಎಣಿಕೆ);

	ವಿಕಲ್ಪ (event->type) {

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = ರಿಕ್ತ;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ಯದಿ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
ಸ್ಥಿರ nserror
convert_script_sync_cb(hlcache_handle *script,
			  ಧ್ರುವ hlcache_event *event,
			  ಶೂನ್ಯ *pw)
{
	html_content *parent = pw;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	unsigned ಪೂರ್ಣಾಙ್ಕ active_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	ಪ್ರತಿ (i = 0, s = parent->scripts; i != parent->scripts_ಎಣಿಕೆ; i++, s++) {
		ಯದಿ (s->type == HTML_SCRIPT_SYNC &&
		    s->data.handle != script && s->already_started == false) {
			active_sync_scripts++;
		}
	}

	/* Find script */
	ಪ್ರತಿ (i = 0, s = parent->scripts; i != parent->scripts_ಎಣಿಕೆ; i++, s++) {
		ಯದಿ (s->type == HTML_SCRIPT_SYNC && s->data.handle == script)
			ಭಙ್ಗ;
	}

	ಪ್ರತಿಜ್ಞಾ(i != parent->scripts_ಎಣಿಕೆ);

	ವಿಕಲ್ಪ (event->type) {
	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->data.handle));
		ಯದಿ (script_handler != ರಿಕ್ತ && parent->jsthread != ರಿಕ್ತ) {
			/* script has a handler */
			ಧ್ರುವ uint8_t *data;
			size_t size;
			data = content_get_source_data(s->data.handle, &size );
			script_handler(parent->jsthread, data, size,
				       nsurl_access(hlcache_handle_get_url(s->data.handle)));
		}

		/* continue parse */
		ಯದಿ (parent->parser != ರಿಕ್ತ && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			ಯದಿ (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = ರಿಕ್ತ;
		parent->base.active--;

		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* continue parse */
		ಯದಿ (parent->parser != ರಿಕ್ತ && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			ಯದಿ (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ಯದಿ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/**
 * process a script with a src tag
 */
ಸ್ಥಿರ dom_hubbub_error
exec_src_script(html_content *c,
		dom_ಗಂಟು *ಗಂಟು,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_child_context child;
	ಸಂರಚನಾ html_script *nscript;
	bool async;
	bool defer;
	ಗಣನಾ html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->base_url, dom_string_data(src), &joined);
	ಯದಿ (ns_error != NSERROR_OK) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_ಎಣಿಕೆ,
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
	exc = dom_element_has_attribute(ಗಂಟು, corestring_dom_async, &async);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ DOM_HUBBUB_OK; /* dom error */
	}

	ಯದಿ (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	ಯದಿ (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} ಅನ್ಯಥಾ {
		exc = dom_element_has_attribute(ಗಂಟು,
						corestring_dom_defer, &defer);
		ಯದಿ (exc != DOM_NO_ERR) {
			ಪ್ರತ್ಯಾಗಮ DOM_HUBBUB_OK; /* dom error */
		}

		ಯದಿ (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} ಅನ್ಯಥಾ {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	ಯದಿ (nscript == ರಿಕ್ತ) {
		nsurl_unref(joined);
		content_broadcast_error(&c->base, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->base),
					   ರಿಕ್ತ,
					   script_cb,
					   c,
					   &child,
					   CONTENT_SCRIPT,
					   &nscript->data.handle);


	nsurl_unref(joined);

	ಯದಿ (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} ಅನ್ಯಥಾ {
		/* update base content active fetch count */
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		ವಿಕಲ್ಪ (script_type) {
		ಪಕ್ಷ HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			ಭಙ್ಗ;

		ಪಕ್ಷ HTML_SCRIPT_ASYNC:
			ಭಙ್ಗ;

		ಪಕ್ಷ HTML_SCRIPT_DEFER:
			ಭಙ್ಗ;

		ಸ್ವಭಾವ:
			ಪ್ರತಿಜ್ಞಾ(0);
		}
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ dom_hubbub_error
exec_inline_script(html_content *c, dom_ಗಂಟು *ಗಂಟು, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	ಸಂರಚನಾ lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	ಸಂರಚನಾ html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_ಗಂಟು_get_text_content(ಗಂಟು, &script);
	ಯದಿ ((exc != DOM_NO_ERR) || (script == ರಿಕ್ತ)) {
		ಪ್ರತ್ಯಾಗಮ DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	ಯದಿ (nscript == ರಿಕ್ತ) {
		dom_string_unref(script);

		content_broadcast_error(&c->base, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ DOM_HUBBUB_NOMEM;

	}

	nscript->data.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	ಯದಿ (script_handler != ರಿಕ್ತ) {
		script_handler(c->jsthread,
			       (ಧ್ರುವ uint8_t *)dom_string_data(script),
			       dom_string_byte_length(script),
			       "?inline script?");
	}
	ಪ್ರತ್ಯಾಗಮ DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(ಶೂನ್ಯ *ctx, dom_ಗಂಟು *ಗಂಟು)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	ಯದಿ (c->jsthread == ರಿಕ್ತ) {
		ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;

		ಸಂದೇಶ_data.jsthread = &c->jsthread;
		content_broadcast(&c->base, CONTENT_ಸಂದೇಶ_GETTHREAD, &ಸಂದೇಶ_data);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		ಯದಿ (c->jsthread == ರಿಕ್ತ) {
			/* no context and it could not be created, abort */
			ಪ್ರತ್ಯಾಗಮ DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      ಗಂಟು);

	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_type, &mimetype);
	ಯದಿ (exc != DOM_NO_ERR || mimetype == ರಿಕ್ತ) {
		mimetype = dom_string_ref(corestring_dom_text_javascript);
	}

	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_src, &src);
	ಯದಿ (exc != DOM_NO_ERR || src == ರಿಕ್ತ) {
		err = exec_inline_script(c, ಗಂಟು, mimetype);
	} ಅನ್ಯಥಾ {
		err = exec_src_script(c, ಗಂಟು, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	ಪ್ರತ್ಯಾಗಮ err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	ಸಂರಚನಾ html_script *s;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0, s = htmlc->scripts; i != htmlc->scripts_ಎಣಿಕೆ; i++, s++) {
		ಯದಿ (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			ಅನುವರ್ತ;
		}
		ಯದಿ (s->data.handle == ರಿಕ್ತ) {
			/* We've not begun loading this? */
			ಅನುವರ್ತ;
		}
		ಯದಿ (content_saw_insecure_objects(s->data.handle)) {
			ಪ್ರತ್ಯಾಗಮ true;
		}
	}

	ಪ್ರತ್ಯಾಗಮ false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i != html->scripts_ಎಣಿಕೆ; i++) {
		ಯದಿ (html->scripts[i].mimetype != ರಿಕ್ತ) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		ವಿಕಲ್ಪ (html->scripts[i].type) {
		ಪಕ್ಷ HTML_SCRIPT_INLINE:
			ಯದಿ (html->scripts[i].data.string != ರಿಕ್ತ) {
				dom_string_unref(html->scripts[i].data.string);
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ HTML_SCRIPT_SYNC:
			/* fallthrough */
		ಪಕ್ಷ HTML_SCRIPT_ASYNC:
			/* fallthrough */
		ಪಕ್ಷ HTML_SCRIPT_DEFER:
			ಯದಿ (html->scripts[i].data.handle != ರಿಕ್ತ) {
				hlcache_handle_release(html->scripts[i].data.handle);
			}
			ಭಙ್ಗ;
		}
	}
	ಮುಕ್ತ(html->scripts);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}
