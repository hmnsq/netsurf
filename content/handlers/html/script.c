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

#شامل <assert.h>
#شامل <ctype.h>
#شامل <stdint.h>
#شامل <stdbool.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <stdlib.h>

#شامل "utils/config.h"
#شامل "utils/corestrings.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "netsurf/content.h"
#شامل "javascript/js.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "content/fetch.h"
#شامل "content/hlcache.h"

#شامل "html/html.h"
#شامل "html/private.h"

تعریفنوع bool (script_handler_t)(ساختار jsthread *jsthread, ثابت uint8_t *data, size_t size, ثابت نویسه *name);


ایستا script_handler_t *select_script_handler(content_type ctype)
{
	اگر (ctype == CONTENT_JS) {
		بازگشت js_exec;
	}
	بازگشت پوچ;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	unsigned صحیح i;
	ساختار html_script *s;
	script_handler_t *script_handler;
	bool have_run_something = false;

	اگر (c->jsthread == پوچ) {
		بازگشت NSERROR_BAD_PARAMETER;
	}

	برای (i = 0, s = c->scripts; i != c->scripts_شمار; i++, s++) {
		اگر (s->already_started) {
			ادامه;
		}

		اگر ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			اگر (s->data.handle == پوچ)
				ادامه;

			/* ensure script content fetch status is not an error */
			اگر (content_get_status(s->data.handle) ==
					CONTENT_STATUS_ERROR)
				ادامه;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->data.handle));
			اگر (script_handler == پوچ)
				ادامه; /* unsupported type */

			اگر (content_get_status(s->data.handle) ==
					CONTENT_STATUS_DONE) {
				/* external script is now available */
				ثابت uint8_t *data;
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

	اگر (have_run_something) {
		بازگشت html_proceed_to_done(c);
	}

	بازگشت NSERROR_OK;
}

/* create new html script entry */
ایستا ساختار html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			شمارش html_script_type type)
{
	ساختار html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  اندازه(ساختار html_script) * (c->scripts_شمار + 1));
	اگر (nscript == پوچ) {
		بازگشت پوچ;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_شمار];
	c->scripts_شمار++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	بازگشت nscript;
}

/**
 * Callback for asyncronous scripts
 */
ایستا nserror
convert_script_async_cb(hlcache_handle *script,
			  ثابت hlcache_event *event,
			  تهی *pw)
{
	html_content *parent = pw;
	unsigned صحیح i;
	ساختار html_script *s;

	/* Find script */
	برای (i = 0, s = parent->scripts; i != parent->scripts_شمار; i++, s++) {
		اگر (s->type == HTML_SCRIPT_ASYNC && s->data.handle == script)
			بشکن;
	}

	أكد(i != parent->scripts_شمار);

	انتخاب (event->type) {
	حالت CONTENT_پیام_LOADING:
		بشکن;

	حالت CONTENT_پیام_READY:
		بشکن;

	حالت CONTENT_پیام_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		بشکن;

	حالت CONTENT_پیام_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = پوچ;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		بشکن;

	پیشفرض:
		بشکن;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	اگر (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	وگرنه اگر (parent->conversion_begun) {
		بازگشت html_script_exec(parent, false);
	}

	بازگشت NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
ایستا nserror
convert_script_defer_cb(hlcache_handle *script,
			  ثابت hlcache_event *event,
			  تهی *pw)
{
	html_content *parent = pw;
	unsigned صحیح i;
	ساختار html_script *s;

	/* Find script */
	برای (i = 0, s = parent->scripts; i != parent->scripts_شمار; i++, s++) {
		اگر (s->type == HTML_SCRIPT_DEFER && s->data.handle == script)
			بشکن;
	}

	أكد(i != parent->scripts_شمار);

	انتخاب (event->type) {

	حالت CONTENT_پیام_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		بشکن;

	حالت CONTENT_پیام_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = پوچ;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		بشکن;

	پیشفرض:
		بشکن;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	اگر (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	بازگشت NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
ایستا nserror
convert_script_sync_cb(hlcache_handle *script,
			  ثابت hlcache_event *event,
			  تهی *pw)
{
	html_content *parent = pw;
	unsigned صحیح i;
	ساختار html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	unsigned صحیح active_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	برای (i = 0, s = parent->scripts; i != parent->scripts_شمار; i++, s++) {
		اگر (s->type == HTML_SCRIPT_SYNC &&
		    s->data.handle != script && s->already_started == false) {
			active_sync_scripts++;
		}
	}

	/* Find script */
	برای (i = 0, s = parent->scripts; i != parent->scripts_شمار; i++, s++) {
		اگر (s->type == HTML_SCRIPT_SYNC && s->data.handle == script)
			بشکن;
	}

	أكد(i != parent->scripts_شمار);

	انتخاب (event->type) {
	حالت CONTENT_پیام_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->data.handle));
		اگر (script_handler != پوچ && parent->jsthread != پوچ) {
			/* script has a handler */
			ثابت uint8_t *data;
			size_t size;
			data = content_get_source_data(s->data.handle, &size );
			script_handler(parent->jsthread, data, size,
				       nsurl_access(hlcache_handle_get_url(s->data.handle)));
		}

		/* continue parse */
		اگر (parent->parser != پوچ && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			اگر (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		بشکن;

	حالت CONTENT_پیام_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = پوچ;
		parent->base.active--;

		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* continue parse */
		اگر (parent->parser != پوچ && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			اگر (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		بشکن;

	پیشفرض:
		بشکن;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	اگر (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	بازگشت NSERROR_OK;
}

/**
 * process a script with a src tag
 */
ایستا dom_hubbub_error
exec_src_script(html_content *c,
		dom_گره *گره,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_child_context child;
	ساختار html_script *nscript;
	bool async;
	bool defer;
	شمارش html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->base_url, dom_string_data(src), &joined);
	اگر (ns_error != NSERROR_OK) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, پوچ);
		بازگشت DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_شمار,
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
	exc = dom_element_has_attribute(گره, corestring_dom_async, &async);
	اگر (exc != DOM_NO_ERR) {
		بازگشت DOM_HUBBUB_OK; /* dom error */
	}

	اگر (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	اگر (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} وگرنه {
		exc = dom_element_has_attribute(گره,
						corestring_dom_defer, &defer);
		اگر (exc != DOM_NO_ERR) {
			بازگشت DOM_HUBBUB_OK; /* dom error */
		}

		اگر (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} وگرنه {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	اگر (nscript == پوچ) {
		nsurl_unref(joined);
		content_broadcast_error(&c->base, NSERROR_NOMEM, پوچ);
		بازگشت DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->base),
					   پوچ,
					   script_cb,
					   c,
					   &child,
					   CONTENT_SCRIPT,
					   &nscript->data.handle);


	nsurl_unref(joined);

	اگر (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} وگرنه {
		/* update base content active fetch count */
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		انتخاب (script_type) {
		حالت HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			بشکن;

		حالت HTML_SCRIPT_ASYNC:
			بشکن;

		حالت HTML_SCRIPT_DEFER:
			بشکن;

		پیشفرض:
			أكد(0);
		}
	}

	بازگشت ret;
}

ایستا dom_hubbub_error
exec_inline_script(html_content *c, dom_گره *گره, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	ساختار lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	ساختار html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_گره_get_text_content(گره, &script);
	اگر ((exc != DOM_NO_ERR) || (script == پوچ)) {
		بازگشت DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	اگر (nscript == پوچ) {
		dom_string_unref(script);

		content_broadcast_error(&c->base, NSERROR_NOMEM, پوچ);
		بازگشت DOM_HUBBUB_NOMEM;

	}

	nscript->data.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	اگر (exc != DOM_NO_ERR) {
		بازگشت DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	اگر (script_handler != پوچ) {
		script_handler(c->jsthread,
			       (ثابت uint8_t *)dom_string_data(script),
			       dom_string_byte_length(script),
			       "?inline script?");
	}
	بازگشت DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(تهی *ctx, dom_گره *گره)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	اگر (c->jsthread == پوچ) {
		اجتماع content_پیام_data پیام_data;

		پیام_data.jsthread = &c->jsthread;
		content_broadcast(&c->base, CONTENT_پیام_GETTHREAD, &پیام_data);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		اگر (c->jsthread == پوچ) {
			/* no context and it could not be created, abort */
			بازگشت DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      گره);

	exc = dom_element_get_attribute(گره, corestring_dom_type, &mimetype);
	اگر (exc != DOM_NO_ERR || mimetype == پوچ) {
		mimetype = dom_string_ref(corestring_dom_text_javascript);
	}

	exc = dom_element_get_attribute(گره, corestring_dom_src, &src);
	اگر (exc != DOM_NO_ERR || src == پوچ) {
		err = exec_inline_script(c, گره, mimetype);
	} وگرنه {
		err = exec_src_script(c, گره, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	بازگشت err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	ساختار html_script *s;
	unsigned صحیح i;

	برای (i = 0, s = htmlc->scripts; i != htmlc->scripts_شمار; i++, s++) {
		اگر (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			ادامه;
		}
		اگر (s->data.handle == پوچ) {
			/* We've not begun loading this? */
			ادامه;
		}
		اگر (content_saw_insecure_objects(s->data.handle)) {
			بازگشت true;
		}
	}

	بازگشت false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	unsigned صحیح i;

	برای (i = 0; i != html->scripts_شمار; i++) {
		اگر (html->scripts[i].mimetype != پوچ) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		انتخاب (html->scripts[i].type) {
		حالت HTML_SCRIPT_INLINE:
			اگر (html->scripts[i].data.string != پوچ) {
				dom_string_unref(html->scripts[i].data.string);
			}
			بشکن;
		حالت HTML_SCRIPT_SYNC:
			/* fallthrough */
		حالت HTML_SCRIPT_ASYNC:
			/* fallthrough */
		حالت HTML_SCRIPT_DEFER:
			اگر (html->scripts[i].data.handle != پوچ) {
				hlcache_handle_release(html->scripts[i].data.handle);
			}
			بشکن;
		}
	}
	آزاد(html->scripts);

	بازگشت NSERROR_OK;
}
