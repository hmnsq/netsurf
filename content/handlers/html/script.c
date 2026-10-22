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

#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <stdint.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <strings.h>
#অন্তর্ভাৱ <stdlib.h>

#অন্তর্ভাৱ "utils/config.h"
#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "netsurf/content.h"
#অন্তর্ভাৱ "javascript/js.h"
#অন্তর্ভাৱ "content/content_protected.h"
#অন্তর্ভাৱ "content/content_factory.h"
#অন্তর্ভাৱ "content/fetch.h"
#অন্তর্ভাৱ "content/hlcache.h"

#অন্তর্ভাৱ "html/html.h"
#অন্তর্ভাৱ "html/private.h"

প্রকারনির্দেশ bool (script_handler_t)(সংরচনা jsthread *jsthread, ধ্রুৱ uint8_t *data, size_t size, ধ্রুৱ ৱর্ণ *name);


স্থির script_handler_t *select_script_handler(content_type ctype)
{
	যদি (ctype == CONTENT_JS) {
		ঘূৰাই js_exec;
	}
	ঘূৰাই রিক্ত;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	unsigned পূর্ণাঙ্ক i;
	সংরচনা html_script *s;
	script_handler_t *script_handler;
	bool have_run_something = false;

	যদি (c->jsthread == রিক্ত) {
		ঘূৰাই NSERROR_BAD_PARAMETER;
	}

	বাবে (i = 0, s = c->scripts; i != c->scripts_গুনতি; i++, s++) {
		যদি (s->already_started) {
			অনুৱর্ত;
		}

		যদি ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			যদি (s->data.handle == রিক্ত)
				অনুৱর্ত;

			/* ensure script content fetch status is not an error */
			যদি (content_get_status(s->data.handle) ==
					CONTENT_STATUS_ERROR)
				অনুৱর্ত;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->data.handle));
			যদি (script_handler == রিক্ত)
				অনুৱর্ত; /* unsupported type */

			যদি (content_get_status(s->data.handle) ==
					CONTENT_STATUS_DONE) {
				/* external script is now available */
				ধ্রুৱ uint8_t *data;
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

	যদি (have_run_something) {
		ঘূৰাই html_proceed_to_done(c);
	}

	ঘূৰাই NSERROR_OK;
}

/* create new html script entry */
স্থির সংরচনা html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			গণনা html_script_type type)
{
	সংরচনা html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  পরিমাণ(সংরচনা html_script) * (c->scripts_গুনতি + 1));
	যদি (nscript == রিক্ত) {
		ঘূৰাই রিক্ত;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_গুনতি];
	c->scripts_গুনতি++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	ঘূৰাই nscript;
}

/**
 * Callback for asyncronous scripts
 */
স্থির nserror
convert_script_async_cb(hlcache_handle *script,
			  ধ্রুৱ hlcache_event *event,
			  শূন্য *pw)
{
	html_content *parent = pw;
	unsigned পূর্ণাঙ্ক i;
	সংরচনা html_script *s;

	/* Find script */
	বাবে (i = 0, s = parent->scripts; i != parent->scripts_গুনতি; i++, s++) {
		যদি (s->type == HTML_SCRIPT_ASYNC && s->data.handle == script)
			ভঙ্গ;
	}

	প্রতিজ্ঞা(i != parent->scripts_গুনতি);

	ৱিকল্প (event->type) {
	পক্ষ CONTENT_বাৰ্তা_LOADING:
		ভঙ্গ;

	পক্ষ CONTENT_বাৰ্তা_READY:
		ভঙ্গ;

	পক্ষ CONTENT_বাৰ্তা_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		ভঙ্গ;

	পক্ষ CONTENT_বাৰ্তা_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = রিক্ত;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		ভঙ্গ;

	স্ৱভাৱ:
		ভঙ্গ;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	যদি (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	অন্যথা যদি (parent->conversion_begun) {
		ঘূৰাই html_script_exec(parent, false);
	}

	ঘূৰাই NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
স্থির nserror
convert_script_defer_cb(hlcache_handle *script,
			  ধ্রুৱ hlcache_event *event,
			  শূন্য *pw)
{
	html_content *parent = pw;
	unsigned পূর্ণাঙ্ক i;
	সংরচনা html_script *s;

	/* Find script */
	বাবে (i = 0, s = parent->scripts; i != parent->scripts_গুনতি; i++, s++) {
		যদি (s->type == HTML_SCRIPT_DEFER && s->data.handle == script)
			ভঙ্গ;
	}

	প্রতিজ্ঞা(i != parent->scripts_গুনতি);

	ৱিকল্প (event->type) {

	পক্ষ CONTENT_বাৰ্তা_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		ভঙ্গ;

	পক্ষ CONTENT_বাৰ্তা_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = রিক্ত;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		ভঙ্গ;

	স্ৱভাৱ:
		ভঙ্গ;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	যদি (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ঘূৰাই NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
স্থির nserror
convert_script_sync_cb(hlcache_handle *script,
			  ধ্রুৱ hlcache_event *event,
			  শূন্য *pw)
{
	html_content *parent = pw;
	unsigned পূর্ণাঙ্ক i;
	সংরচনা html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	unsigned পূর্ণাঙ্ক active_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	বাবে (i = 0, s = parent->scripts; i != parent->scripts_গুনতি; i++, s++) {
		যদি (s->type == HTML_SCRIPT_SYNC &&
		    s->data.handle != script && s->already_started == false) {
			active_sync_scripts++;
		}
	}

	/* Find script */
	বাবে (i = 0, s = parent->scripts; i != parent->scripts_গুনতি; i++, s++) {
		যদি (s->type == HTML_SCRIPT_SYNC && s->data.handle == script)
			ভঙ্গ;
	}

	প্রতিজ্ঞা(i != parent->scripts_গুনতি);

	ৱিকল্প (event->type) {
	পক্ষ CONTENT_বাৰ্তা_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->data.handle));
		যদি (script_handler != রিক্ত && parent->jsthread != রিক্ত) {
			/* script has a handler */
			ধ্রুৱ uint8_t *data;
			size_t size;
			data = content_get_source_data(s->data.handle, &size );
			script_handler(parent->jsthread, data, size,
				       nsurl_access(hlcache_handle_get_url(s->data.handle)));
		}

		/* continue parse */
		যদি (parent->parser != রিক্ত && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			যদি (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		ভঙ্গ;

	পক্ষ CONTENT_বাৰ্তা_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = রিক্ত;
		parent->base.active--;

		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* continue parse */
		যদি (parent->parser != রিক্ত && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			যদি (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		ভঙ্গ;

	স্ৱভাৱ:
		ভঙ্গ;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	যদি (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ঘূৰাই NSERROR_OK;
}

/**
 * process a script with a src tag
 */
স্থির dom_hubbub_error
exec_src_script(html_content *c,
		dom_গ্ৰন্থি *গ্ৰন্থি,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_child_context child;
	সংরচনা html_script *nscript;
	bool async;
	bool defer;
	গণনা html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->base_url, dom_string_data(src), &joined);
	যদি (ns_error != NSERROR_OK) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, রিক্ত);
		ঘূৰাই DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_গুনতি,
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
	exc = dom_element_has_attribute(গ্ৰন্থি, corestring_dom_async, &async);
	যদি (exc != DOM_NO_ERR) {
		ঘূৰাই DOM_HUBBUB_OK; /* dom error */
	}

	যদি (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	যদি (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} অন্যথা {
		exc = dom_element_has_attribute(গ্ৰন্থি,
						corestring_dom_defer, &defer);
		যদি (exc != DOM_NO_ERR) {
			ঘূৰাই DOM_HUBBUB_OK; /* dom error */
		}

		যদি (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} অন্যথা {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	যদি (nscript == রিক্ত) {
		nsurl_unref(joined);
		content_broadcast_error(&c->base, NSERROR_NOMEM, রিক্ত);
		ঘূৰাই DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->base),
					   রিক্ত,
					   script_cb,
					   c,
					   &child,
					   CONTENT_SCRIPT,
					   &nscript->data.handle);


	nsurl_unref(joined);

	যদি (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} অন্যথা {
		/* update base content active fetch count */
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		ৱিকল্প (script_type) {
		পক্ষ HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			ভঙ্গ;

		পক্ষ HTML_SCRIPT_ASYNC:
			ভঙ্গ;

		পক্ষ HTML_SCRIPT_DEFER:
			ভঙ্গ;

		স্ৱভাৱ:
			প্রতিজ্ঞা(0);
		}
	}

	ঘূৰাই ret;
}

স্থির dom_hubbub_error
exec_inline_script(html_content *c, dom_গ্ৰন্থি *গ্ৰন্থি, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	সংরচনা lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	সংরচনা html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_গ্ৰন্থি_get_text_content(গ্ৰন্থি, &script);
	যদি ((exc != DOM_NO_ERR) || (script == রিক্ত)) {
		ঘূৰাই DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	যদি (nscript == রিক্ত) {
		dom_string_unref(script);

		content_broadcast_error(&c->base, NSERROR_NOMEM, রিক্ত);
		ঘূৰাই DOM_HUBBUB_NOMEM;

	}

	nscript->data.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	যদি (exc != DOM_NO_ERR) {
		ঘূৰাই DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	যদি (script_handler != রিক্ত) {
		script_handler(c->jsthread,
			       (ধ্রুৱ uint8_t *)dom_string_data(script),
			       dom_string_byte_length(script),
			       "?inline script?");
	}
	ঘূৰাই DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(শূন্য *ctx, dom_গ্ৰন্থি *গ্ৰন্থি)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	যদি (c->jsthread == রিক্ত) {
		সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;

		বাৰ্তা_data.jsthread = &c->jsthread;
		content_broadcast(&c->base, CONTENT_বাৰ্তা_GETTHREAD, &বাৰ্তা_data);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		যদি (c->jsthread == রিক্ত) {
			/* no context and it could not be created, abort */
			ঘূৰাই DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      গ্ৰন্থি);

	exc = dom_element_get_attribute(গ্ৰন্থি, corestring_dom_type, &mimetype);
	যদি (exc != DOM_NO_ERR || mimetype == রিক্ত) {
		mimetype = dom_string_ref(corestring_dom_text_javascript);
	}

	exc = dom_element_get_attribute(গ্ৰন্থি, corestring_dom_src, &src);
	যদি (exc != DOM_NO_ERR || src == রিক্ত) {
		err = exec_inline_script(c, গ্ৰন্থি, mimetype);
	} অন্যথা {
		err = exec_src_script(c, গ্ৰন্থি, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	ঘূৰাই err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	সংরচনা html_script *s;
	unsigned পূর্ণাঙ্ক i;

	বাবে (i = 0, s = htmlc->scripts; i != htmlc->scripts_গুনতি; i++, s++) {
		যদি (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			অনুৱর্ত;
		}
		যদি (s->data.handle == রিক্ত) {
			/* We've not begun loading this? */
			অনুৱর্ত;
		}
		যদি (content_saw_insecure_objects(s->data.handle)) {
			ঘূৰাই true;
		}
	}

	ঘূৰাই false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	unsigned পূর্ণাঙ্ক i;

	বাবে (i = 0; i != html->scripts_গুনতি; i++) {
		যদি (html->scripts[i].mimetype != রিক্ত) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		ৱিকল্প (html->scripts[i].type) {
		পক্ষ HTML_SCRIPT_INLINE:
			যদি (html->scripts[i].data.string != রিক্ত) {
				dom_string_unref(html->scripts[i].data.string);
			}
			ভঙ্গ;
		পক্ষ HTML_SCRIPT_SYNC:
			/* fallthrough */
		পক্ষ HTML_SCRIPT_ASYNC:
			/* fallthrough */
		পক্ষ HTML_SCRIPT_DEFER:
			যদি (html->scripts[i].data.handle != রিক্ত) {
				hlcache_handle_release(html->scripts[i].data.handle);
			}
			ভঙ্গ;
		}
	}
	মুক্ত(html->scripts);

	ঘূৰাই NSERROR_OK;
}
