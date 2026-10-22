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

#ضمن <assert.h>
#ضمن <ctype.h>
#ضمن <stdint.h>
#ضمن <stdbool.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <stdlib.h>

#ضمن "utils/config.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "netsurf/content.h"
#ضمن "javascript/js.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "content/fetch.h"
#ضمن "content/hlcache.h"

#ضمن "html/html.h"
#ضمن "html/private.h"

تعريفنوع bool (script_handler_t)(بنية jsthread *jsthread, ثابت uint8_t *data, size_t size, ثابت حرف *name);


ساكن script_handler_t *select_script_handler(content_type ctype)
{
	إذا (ctype == CONTENT_JS) {
		أرجع js_exec;
	}
	أرجع عدم;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	unsigned صحيح i;
	بنية html_script *s;
	script_handler_t *script_handler;
	bool have_run_something = false;

	إذا (c->jsthread == عدم) {
		أرجع NSERROR_BAD_PARAMETER;
	}

	لكل (i = 0, s = c->scripts; i != c->scripts_کيراءن; i++, s++) {
		إذا (s->already_started) {
			استمر;
		}

		إذا ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			إذا (s->data.handle == عدم)
				استمر;

			/* ensure script content fetch status is not an error */
			إذا (content_get_status(s->data.handle) ==
					CONTENT_STATUS_ERROR)
				استمر;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->data.handle));
			إذا (script_handler == عدم)
				استمر; /* unsupported type */

			إذا (content_get_status(s->data.handle) ==
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

	إذا (have_run_something) {
		أرجع html_proceed_to_done(c);
	}

	أرجع NSERROR_OK;
}

/* create new html script entry */
ساكن بنية html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			تعداد html_script_type type)
{
	بنية html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  حجم(بنية html_script) * (c->scripts_کيراءن + 1));
	إذا (nscript == عدم) {
		أرجع عدم;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_کيراءن];
	c->scripts_کيراءن++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	أرجع nscript;
}

/**
 * Callback for asyncronous scripts
 */
ساكن nserror
convert_script_async_cb(hlcache_handle *script,
			  ثابت hlcache_event *event,
			  فراغ *pw)
{
	html_content *parent = pw;
	unsigned صحيح i;
	بنية html_script *s;

	/* Find script */
	لكل (i = 0, s = parent->scripts; i != parent->scripts_کيراءن; i++, s++) {
		إذا (s->type == HTML_SCRIPT_ASYNC && s->data.handle == script)
			اكسر;
	}

	أكد(i != parent->scripts_کيراءن);

	بدل (event->type) {
	حالة CONTENT_ڤسنن_LOADING:
		اكسر;

	حالة CONTENT_ڤسنن_READY:
		اكسر;

	حالة CONTENT_ڤسنن_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		اكسر;

	حالة CONTENT_ڤسنن_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = عدم;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		اكسر;

	افتراضي:
		اكسر;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	إذا (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	وإلا إذا (parent->conversion_begun) {
		أرجع html_script_exec(parent, false);
	}

	أرجع NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
ساكن nserror
convert_script_defer_cb(hlcache_handle *script,
			  ثابت hlcache_event *event,
			  فراغ *pw)
{
	html_content *parent = pw;
	unsigned صحيح i;
	بنية html_script *s;

	/* Find script */
	لكل (i = 0, s = parent->scripts; i != parent->scripts_کيراءن; i++, s++) {
		إذا (s->type == HTML_SCRIPT_DEFER && s->data.handle == script)
			اكسر;
	}

	أكد(i != parent->scripts_کيراءن);

	بدل (event->type) {

	حالة CONTENT_ڤسنن_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		اكسر;

	حالة CONTENT_ڤسنن_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = عدم;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		اكسر;

	افتراضي:
		اكسر;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	إذا (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	أرجع NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
ساكن nserror
convert_script_sync_cb(hlcache_handle *script,
			  ثابت hlcache_event *event,
			  فراغ *pw)
{
	html_content *parent = pw;
	unsigned صحيح i;
	بنية html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	unsigned صحيح active_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	لكل (i = 0, s = parent->scripts; i != parent->scripts_کيراءن; i++, s++) {
		إذا (s->type == HTML_SCRIPT_SYNC &&
		    s->data.handle != script && s->already_started == false) {
			active_sync_scripts++;
		}
	}

	/* Find script */
	لكل (i = 0, s = parent->scripts; i != parent->scripts_کيراءن; i++, s++) {
		إذا (s->type == HTML_SCRIPT_SYNC && s->data.handle == script)
			اكسر;
	}

	أكد(i != parent->scripts_کيراءن);

	بدل (event->type) {
	حالة CONTENT_ڤسنن_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->data.handle));
		إذا (script_handler != عدم && parent->jsthread != عدم) {
			/* script has a handler */
			ثابت uint8_t *data;
			size_t size;
			data = content_get_source_data(s->data.handle, &size );
			script_handler(parent->jsthread, data, size,
				       nsurl_access(hlcache_handle_get_url(s->data.handle)));
		}

		/* continue parse */
		إذا (parent->parser != عدم && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			إذا (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		اكسر;

	حالة CONTENT_ڤسنن_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = عدم;
		parent->base.active--;

		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* continue parse */
		إذا (parent->parser != عدم && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			إذا (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		اكسر;

	افتراضي:
		اكسر;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	إذا (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	أرجع NSERROR_OK;
}

/**
 * process a script with a src tag
 */
ساكن dom_hubbub_error
exec_src_script(html_content *c,
		dom_نود *نود,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_child_context child;
	بنية html_script *nscript;
	bool async;
	bool defer;
	تعداد html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->base_url, dom_string_data(src), &joined);
	إذا (ns_error != NSERROR_OK) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, عدم);
		أرجع DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_کيراءن,
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
	exc = dom_element_has_attribute(نود, corestring_dom_async, &async);
	إذا (exc != DOM_NO_ERR) {
		أرجع DOM_HUBBUB_OK; /* dom error */
	}

	إذا (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	إذا (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} وإلا {
		exc = dom_element_has_attribute(نود,
						corestring_dom_defer, &defer);
		إذا (exc != DOM_NO_ERR) {
			أرجع DOM_HUBBUB_OK; /* dom error */
		}

		إذا (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} وإلا {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	إذا (nscript == عدم) {
		nsurl_unref(joined);
		content_broadcast_error(&c->base, NSERROR_NOMEM, عدم);
		أرجع DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->base),
					   عدم,
					   script_cb,
					   c,
					   &child,
					   CONTENT_SCRIPT,
					   &nscript->data.handle);


	nsurl_unref(joined);

	إذا (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} وإلا {
		/* update base content active fetch count */
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		بدل (script_type) {
		حالة HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			اكسر;

		حالة HTML_SCRIPT_ASYNC:
			اكسر;

		حالة HTML_SCRIPT_DEFER:
			اكسر;

		افتراضي:
			أكد(0);
		}
	}

	أرجع ret;
}

ساكن dom_hubbub_error
exec_inline_script(html_content *c, dom_نود *نود, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	بنية lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	بنية html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_نود_get_text_content(نود, &script);
	إذا ((exc != DOM_NO_ERR) || (script == عدم)) {
		أرجع DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	إذا (nscript == عدم) {
		dom_string_unref(script);

		content_broadcast_error(&c->base, NSERROR_NOMEM, عدم);
		أرجع DOM_HUBBUB_NOMEM;

	}

	nscript->data.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	إذا (exc != DOM_NO_ERR) {
		أرجع DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	إذا (script_handler != عدم) {
		script_handler(c->jsthread,
			       (ثابت uint8_t *)dom_string_data(script),
			       dom_string_byte_length(script),
			       "?inline script?");
	}
	أرجع DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(فراغ *ctx, dom_نود *نود)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	إذا (c->jsthread == عدم) {
		اتحاد content_ڤسنن_data ڤسنن_data;

		ڤسنن_data.jsthread = &c->jsthread;
		content_broadcast(&c->base, CONTENT_ڤسنن_GETTHREAD, &ڤسنن_data);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		إذا (c->jsthread == عدم) {
			/* no context and it could not be created, abort */
			أرجع DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      نود);

	exc = dom_element_get_attribute(نود, corestring_dom_type, &mimetype);
	إذا (exc != DOM_NO_ERR || mimetype == عدم) {
		mimetype = dom_string_ref(corestring_dom_text_javascript);
	}

	exc = dom_element_get_attribute(نود, corestring_dom_src, &src);
	إذا (exc != DOM_NO_ERR || src == عدم) {
		err = exec_inline_script(c, نود, mimetype);
	} وإلا {
		err = exec_src_script(c, نود, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	أرجع err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	بنية html_script *s;
	unsigned صحيح i;

	لكل (i = 0, s = htmlc->scripts; i != htmlc->scripts_کيراءن; i++, s++) {
		إذا (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			استمر;
		}
		إذا (s->data.handle == عدم) {
			/* We've not begun loading this? */
			استمر;
		}
		إذا (content_saw_insecure_objects(s->data.handle)) {
			أرجع true;
		}
	}

	أرجع false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	unsigned صحيح i;

	لكل (i = 0; i != html->scripts_کيراءن; i++) {
		إذا (html->scripts[i].mimetype != عدم) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		بدل (html->scripts[i].type) {
		حالة HTML_SCRIPT_INLINE:
			إذا (html->scripts[i].data.string != عدم) {
				dom_string_unref(html->scripts[i].data.string);
			}
			اكسر;
		حالة HTML_SCRIPT_SYNC:
			/* fallthrough */
		حالة HTML_SCRIPT_ASYNC:
			/* fallthrough */
		حالة HTML_SCRIPT_DEFER:
			إذا (html->scripts[i].data.handle != عدم) {
				hlcache_handle_release(html->scripts[i].data.handle);
			}
			اكسر;
		}
	}
	حرر(html->scripts);

	أرجع NSERROR_OK;
}
