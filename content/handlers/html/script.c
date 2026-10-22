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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>
#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "utils/config.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "javascript/js.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "content/fetch.h"
#অন্তর্ভাব "content/hlcache.h"

#অন্তর্ভাব "html/html.h"
#অন্তর্ভাব "html/private.h"

প্রকারলেখ bool (script_handler_t)(কাঠামো jsthread *jsthread, ধ্রুবক uint8_t *তথ্য, size_t আকার_2, ধ্রুবক অক্ষর *name);


স্থির script_handler_t *select_script_handler(content_type ctype)
{
	যদি (ctype == CONTENT_JS) {
		ফেরত js_exec;
	}
	ফেরত ফাঁকা;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	unsigned পূর্ণসংখ্যা i;
	কাঠামো html_script *s;
	script_handler_t *script_handler;
	bool have_run_something = false;

	যদি (c->jsthread == ফাঁকা) {
		ফেরত NSERROR_BAD_PARAMETER;
	}

	জন্য (i = 0, s = c->scripts; i != c->scripts_গুনতি; i++, s++) {
		যদি (s->already_started) {
			চালু;
		}

		যদি ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			যদি (s->তথ্য.handle == ফাঁকা)
				চালু;

			/* ensure script content fetch status is not an error */
			যদি (content_get_status(s->তথ্য.handle) ==
					CONTENT_STATUS_ERROR)
				চালু;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->তথ্য.handle));
			যদি (script_handler == ফাঁকা)
				চালু; /* unsupported type */

			যদি (content_get_status(s->তথ্য.handle) ==
					CONTENT_STATUS_DONE) {
				/* external script is now available */
				ধ্রুবক uint8_t *তথ্য;
				size_t আকার_2;
				তথ্য = content_get_source_তথ্য(
						s->তথ্য.handle, &আকার_2 );
				script_handler(c->jsthread, তথ্য, আকার_2,
					       nsurl_access(hlcache_handle_get_url(s->তথ্য.handle)));
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
		ফেরত html_proceed_to_done(c);
	}

	ফেরত NSERROR_OK;
}

/* create new html script entry */
স্থির কাঠামো html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			গণনা html_script_type type)
{
	কাঠামো html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  আকার(কাঠামো html_script) * (c->scripts_গুনতি + 1));
	যদি (nscript == ফাঁকা) {
		ফেরত ফাঁকা;
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

	ফেরত nscript;
}

/**
 * Callback for asyncronous scripts
 */
স্থির nserror
convert_script_async_cb(hlcache_handle *script,
			  ধ্রুবক hlcache_event *event,
			  শূন্য *pw)
{
	html_content *parent = pw;
	unsigned পূর্ণসংখ্যা i;
	কাঠামো html_script *s;

	/* Find script */
	জন্য (i = 0, s = parent->scripts; i != parent->scripts_গুনতি; i++, s++) {
		যদি (s->type == HTML_SCRIPT_ASYNC && s->তথ্য.handle == script)
			বিরতি;
	}

	প্রতিজ্ঞা(i != parent->scripts_গুনতি);

	নির্বাচন (event->type) {
	ক্ষেত্র CONTENT_বার্তা_LOADING:
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_READY:
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->তথ্য.errordata.errormsg);

		hlcache_handle_release(script);
		s->তথ্য.handle = ফাঁকা;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		বিরতি;

	পূর্বনির্ধারিত:
		বিরতি;
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
		ফেরত html_script_exec(parent, false);
	}

	ফেরত NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
স্থির nserror
convert_script_defer_cb(hlcache_handle *script,
			  ধ্রুবক hlcache_event *event,
			  শূন্য *pw)
{
	html_content *parent = pw;
	unsigned পূর্ণসংখ্যা i;
	কাঠামো html_script *s;

	/* Find script */
	জন্য (i = 0, s = parent->scripts; i != parent->scripts_গুনতি; i++, s++) {
		যদি (s->type == HTML_SCRIPT_DEFER && s->তথ্য.handle == script)
			বিরতি;
	}

	প্রতিজ্ঞা(i != parent->scripts_গুনতি);

	নির্বাচন (event->type) {

	ক্ষেত্র CONTENT_বার্তা_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->তথ্য.errordata.errormsg);

		hlcache_handle_release(script);
		s->তথ্য.handle = ফাঁকা;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		বিরতি;

	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	যদি (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ফেরত NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
স্থির nserror
convert_script_sync_cb(hlcache_handle *script,
			  ধ্রুবক hlcache_event *event,
			  শূন্য *pw)
{
	html_content *parent = pw;
	unsigned পূর্ণসংখ্যা i;
	কাঠামো html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	unsigned পূর্ণসংখ্যা active_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	জন্য (i = 0, s = parent->scripts; i != parent->scripts_গুনতি; i++, s++) {
		যদি (s->type == HTML_SCRIPT_SYNC &&
		    s->তথ্য.handle != script && s->already_started == false) {
			active_sync_scripts++;
		}
	}

	/* Find script */
	জন্য (i = 0, s = parent->scripts; i != parent->scripts_গুনতি; i++, s++) {
		যদি (s->type == HTML_SCRIPT_SYNC && s->তথ্য.handle == script)
			বিরতি;
	}

	প্রতিজ্ঞা(i != parent->scripts_গুনতি);

	নির্বাচন (event->type) {
	ক্ষেত্র CONTENT_বার্তা_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->তথ্য.handle));
		যদি (script_handler != ফাঁকা && parent->jsthread != ফাঁকা) {
			/* script has a handler */
			ধ্রুবক uint8_t *তথ্য;
			size_t আকার_2;
			তথ্য = content_get_source_তথ্য(s->তথ্য.handle, &আকার_2 );
			script_handler(parent->jsthread, তথ্য, আকার_2,
				       nsurl_access(hlcache_handle_get_url(s->তথ্য.handle)));
		}

		/* continue parse */
		যদি (parent->parser != ফাঁকা && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			যদি (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->তথ্য.errordata.errormsg);

		hlcache_handle_release(script);
		s->তথ্য.handle = ফাঁকা;
		parent->base.active--;

		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* continue parse */
		যদি (parent->parser != ফাঁকা && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			যদি (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		বিরতি;

	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	যদি (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ফেরত NSERROR_OK;
}

/**
 * process a script with a src tag
 */
স্থির dom_hubbub_error
exec_src_script(html_content *c,
		dom_গ্রন্থি *গ্রন্থি,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_child_context child;
	কাঠামো html_script *nscript;
	bool async;
	bool defer;
	গণনা html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->base_url, dom_string_তথ্য(src), &joined);
	যদি (ns_error != NSERROR_OK) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, ফাঁকা);
		ফেরত DOM_HUBBUB_NOMEM;
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
	exc = dom_element_has_attribute(গ্রন্থি, corestring_dom_async, &async);
	যদি (exc != DOM_NO_ERR) {
		ফেরত DOM_HUBBUB_OK; /* dom error */
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
		exc = dom_element_has_attribute(গ্রন্থি,
						corestring_dom_defer, &defer);
		যদি (exc != DOM_NO_ERR) {
			ফেরত DOM_HUBBUB_OK; /* dom error */
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
	যদি (nscript == ফাঁকা) {
		nsurl_unref(joined);
		content_broadcast_error(&c->base, NSERROR_NOMEM, ফাঁকা);
		ফেরত DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->base),
					   ফাঁকা,
					   script_cb,
					   c,
					   &child,
					   CONTENT_SCRIPT,
					   &nscript->তথ্য.handle);


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

		নির্বাচন (script_type) {
		ক্ষেত্র HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			বিরতি;

		ক্ষেত্র HTML_SCRIPT_ASYNC:
			বিরতি;

		ক্ষেত্র HTML_SCRIPT_DEFER:
			বিরতি;

		পূর্বনির্ধারিত:
			প্রতিজ্ঞা(0);
		}
	}

	ফেরত ret;
}

স্থির dom_hubbub_error
exec_inline_script(html_content *c, dom_গ্রন্থি *গ্রন্থি, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	কাঠামো lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	কাঠামো html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_গ্রন্থি_get_text_content(গ্রন্থি, &script);
	যদি ((exc != DOM_NO_ERR) || (script == ফাঁকা)) {
		ফেরত DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	যদি (nscript == ফাঁকা) {
		dom_string_unref(script);

		content_broadcast_error(&c->base, NSERROR_NOMEM, ফাঁকা);
		ফেরত DOM_HUBBUB_NOMEM;

	}

	nscript->তথ্য.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	যদি (exc != DOM_NO_ERR) {
		ফেরত DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	যদি (script_handler != ফাঁকা) {
		script_handler(c->jsthread,
			       (ধ্রুবক uint8_t *)dom_string_তথ্য(script),
			       dom_string_byte_দৈর্ঘ্য(script),
			       "?inline script?");
	}
	ফেরত DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(শূন্য *ctx, dom_গ্রন্থি *গ্রন্থি)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	যদি (c->jsthread == ফাঁকা) {
		সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;

		বার্তা_তথ্য.jsthread = &c->jsthread;
		content_broadcast(&c->base, CONTENT_বার্তা_GETTHREAD, &বার্তা_তথ্য);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		যদি (c->jsthread == ফাঁকা) {
			/* no context and it could not be created, abort */
			ফেরত DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      গ্রন্থি);

	exc = dom_element_get_attribute(গ্রন্থি, corestring_dom_type, &mimetype);
	যদি (exc != DOM_NO_ERR || mimetype == ফাঁকা) {
		mimetype = dom_string_ref(corestring_dom_text_javascript);
	}

	exc = dom_element_get_attribute(গ্রন্থি, corestring_dom_src, &src);
	যদি (exc != DOM_NO_ERR || src == ফাঁকা) {
		err = exec_inline_script(c, গ্রন্থি, mimetype);
	} অন্যথা {
		err = exec_src_script(c, গ্রন্থি, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	ফেরত err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	কাঠামো html_script *s;
	unsigned পূর্ণসংখ্যা i;

	জন্য (i = 0, s = htmlc->scripts; i != htmlc->scripts_গুনতি; i++, s++) {
		যদি (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			চালু;
		}
		যদি (s->তথ্য.handle == ফাঁকা) {
			/* We've not begun loading this? */
			চালু;
		}
		যদি (content_saw_insecure_objects(s->তথ্য.handle)) {
			ফেরত true;
		}
	}

	ফেরত false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	unsigned পূর্ণসংখ্যা i;

	জন্য (i = 0; i != html->scripts_গুনতি; i++) {
		যদি (html->scripts[i].mimetype != ফাঁকা) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		নির্বাচন (html->scripts[i].type) {
		ক্ষেত্র HTML_SCRIPT_INLINE:
			যদি (html->scripts[i].তথ্য.string != ফাঁকা) {
				dom_string_unref(html->scripts[i].তথ্য.string);
			}
			বিরতি;
		ক্ষেত্র HTML_SCRIPT_SYNC:
			/* fallthrough */
		ক্ষেত্র HTML_SCRIPT_ASYNC:
			/* fallthrough */
		ক্ষেত্র HTML_SCRIPT_DEFER:
			যদি (html->scripts[i].তথ্য.handle != ফাঁকা) {
				hlcache_handle_release(html->scripts[i].তথ্য.handle);
			}
			বিরতি;
		}
	}
	মুক্ত(html->scripts);

	ফেরত NSERROR_OK;
}
