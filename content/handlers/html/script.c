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

قسمتعریف bool (script_handler_t)(ساخت jsthread *jsthread, ثابت uint8_t *ڈیٹا, size_t سائز, ثابت حرف *name);


ساکن script_handler_t *select_script_handler(content_type ctype)
{
	اگر (ctype == CONTENT_JS) {
		واپس js_exec;
	}
	واپس تہی;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	غیرعلامتی صحیح i;
	ساخت html_script *s;
	script_handler_t *script_handler;
	bool have_تسلسل_something = false;

	اگر (c->jsthread == تہی) {
		واپس NSERROR_BAD_PARAMETER;
	}

	برائے (i = 0, s = c->scripts; i != c->scripts_گنتی; i++, s++) {
		اگر (s->already_started) {
			جاری;
		}

		اگر ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			اگر (s->ڈیٹا.handle == تہی)
				جاری;

			/* ensure script content fetch status is not an error */
			اگر (content_get_حالت(s->ڈیٹا.handle) ==
					CONTENT_حالت_ERROR)
				جاری;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->ڈیٹا.handle));
			اگر (script_handler == تہی)
				جاری; /* unsupported type */

			اگر (content_get_حالت(s->ڈیٹا.handle) ==
					CONTENT_حالت_مکمل) {
				/* external script is now available */
				ثابت uint8_t *ڈیٹا;
				size_t سائز;
				ڈیٹا = content_get_ماخذ_ڈیٹا(
						s->ڈیٹا.handle, &سائز );
				script_handler(c->jsthread, ڈیٹا, سائز,
					       nsurl_access(hlcache_handle_get_url(s->ڈیٹا.handle)));
				have_تسلسل_something = true;
				/* We have to re-acquire this here since the
				 * c->scripts array may have been reallocated
				 * as a result of executing this script.
				 */
				s = &(c->scripts[i]);

				s->already_started = true;

			}
		}
	}

	اگر (have_تسلسل_something) {
		واپس html_proceed_مقصد_مکمل(c);
	}

	واپس NSERROR_OK;
}

/* create new html script entry */
ساکن ساخت html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			شمار html_script_type type)
{
	ساخت html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  حجم(ساخت html_script) * (c->scripts_گنتی + 1));
	اگر (nscript == تہی) {
		واپس تہی;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_گنتی];
	c->scripts_گنتی++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	واپس nscript;
}

/**
 * Callback for asyncronous scripts
 */
ساکن nserror
convert_script_async_cb(hlcache_handle *script,
			  ثابت hlcache_event *event,
			  خالی *pw)
{
	html_content *والد = pw;
	غیرعلامتی صحیح i;
	ساخت html_script *s;

	/* Find script */
	برائے (i = 0, s = والد->scripts; i != والد->scripts_گنتی; i++, s++) {
		اگر (s->type == HTML_SCRIPT_ASYNC && s->ڈیٹا.handle == script)
			توڑو;
	}

	أكد(i != والد->scripts_گنتی);

	انتخاب (event->type) {
	حالت CONTENT_پیغام_LOADING:
		توڑو;

	حالت CONTENT_پیغام_READY:
		توڑو;

	حالت CONTENT_پیغام_مکمل:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		والد->بنیاد.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", والد->بنیاد.فعال);

		توڑو;

	حالت CONTENT_پیغام_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->ڈیٹا.errordata.errormsg);

		hlcache_handle_release(script);
		s->ڈیٹا.handle = تہی;
		والد->بنیاد.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", والد->بنیاد.فعال);

		توڑو;

	طےشدہ:
		توڑو;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	اگر (html_can_begin_conversion(والد)) {
		html_begin_conversion(والد);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	ورنہ اگر (والد->conversion_begun) {
		واپس html_script_exec(والد, false);
	}

	واپس NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
ساکن nserror
convert_script_defer_cb(hlcache_handle *script,
			  ثابت hlcache_event *event,
			  خالی *pw)
{
	html_content *والد = pw;
	غیرعلامتی صحیح i;
	ساخت html_script *s;

	/* Find script */
	برائے (i = 0, s = والد->scripts; i != والد->scripts_گنتی; i++, s++) {
		اگر (s->type == HTML_SCRIPT_DEFER && s->ڈیٹا.handle == script)
			توڑو;
	}

	أكد(i != والد->scripts_گنتی);

	انتخاب (event->type) {

	حالت CONTENT_پیغام_مکمل:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		والد->بنیاد.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", والد->بنیاد.فعال);

		توڑو;

	حالت CONTENT_پیغام_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->ڈیٹا.errordata.errormsg);

		hlcache_handle_release(script);
		s->ڈیٹا.handle = تہی;
		والد->بنیاد.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", والد->بنیاد.فعال);

		توڑو;

	طےشدہ:
		توڑو;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	اگر (html_can_begin_conversion(والد)) {
		html_begin_conversion(والد);
	}

	واپس NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
ساکن nserror
convert_script_sync_cb(hlcache_handle *script,
			  ثابت hlcache_event *event,
			  خالی *pw)
{
	html_content *والد = pw;
	غیرعلامتی صحیح i;
	ساخت html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	غیرعلامتی صحیح فعال_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	برائے (i = 0, s = والد->scripts; i != والد->scripts_گنتی; i++, s++) {
		اگر (s->type == HTML_SCRIPT_SYNC &&
		    s->ڈیٹا.handle != script && s->already_started == false) {
			فعال_sync_scripts++;
		}
	}

	/* Find script */
	برائے (i = 0, s = والد->scripts; i != والد->scripts_گنتی; i++, s++) {
		اگر (s->type == HTML_SCRIPT_SYNC && s->ڈیٹا.handle == script)
			توڑو;
	}

	أكد(i != والد->scripts_گنتی);

	انتخاب (event->type) {
	حالت CONTENT_پیغام_مکمل:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		والد->بنیاد.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", والد->بنیاد.فعال);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->ڈیٹا.handle));
		اگر (script_handler != تہی && والد->jsthread != تہی) {
			/* script has a handler */
			ثابت uint8_t *ڈیٹا;
			size_t سائز;
			ڈیٹا = content_get_ماخذ_ڈیٹا(s->ڈیٹا.handle, &سائز );
			script_handler(والد->jsthread, ڈیٹا, سائز,
				       nsurl_access(hlcache_handle_get_url(s->ڈیٹا.handle)));
		}

		/* continue parse */
		اگر (والد->parser != تہی && فعال_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(والد->parser, false);
			اگر (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		توڑو;

	حالت CONTENT_پیغام_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->ڈیٹا.errordata.errormsg);

		hlcache_handle_release(script);
		s->ڈیٹا.handle = تہی;
		والد->بنیاد.فعال--;

		NSLOG(netsurf, INFO, "%d fetches active", والد->بنیاد.فعال);

		s->already_started = true;

		/* continue parse */
		اگر (والد->parser != تہی && فعال_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(والد->parser, false);
			اگر (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		توڑو;

	طےشدہ:
		توڑو;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	اگر (html_can_begin_conversion(والد)) {
		html_begin_conversion(والد);
	}

	واپس NSERROR_OK;
}

/**
 * process a script with a src tag
 */
ساکن dom_hubbub_error
exec_src_script(html_content *c,
		dom_گرہ *گرہ,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_بچہ_context بچہ;
	ساخت html_script *nscript;
	bool async;
	bool defer;
	شمار html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->بنیاد_url, dom_string_ڈیٹا(src), &joined);
	اگر (ns_error != NSERROR_OK) {
		content_نشر_error(&c->بنیاد, NSERROR_NOMEM, تہی);
		واپس DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_گنتی,
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
	exc = dom_element_has_attribute(گرہ, corestring_dom_async, &async);
	اگر (exc != DOM_NO_ERR) {
		واپس DOM_HUBBUB_OK; /* dom error */
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

	} ورنہ {
		exc = dom_element_has_attribute(گرہ,
						corestring_dom_defer, &defer);
		اگر (exc != DOM_NO_ERR) {
			واپس DOM_HUBBUB_OK; /* dom error */
		}

		اگر (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} ورنہ {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	اگر (nscript == تہی) {
		nsurl_unref(joined);
		content_نشر_error(&c->بنیاد, NSERROR_NOMEM, تہی);
		واپس DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	بچہ.charset = c->encoding;
	بچہ.quirks = c->بنیاد.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->بنیاد),
					   تہی,
					   script_cb,
					   c,
					   &بچہ,
					   CONTENT_SCRIPT,
					   &nscript->ڈیٹا.handle);


	nsurl_unref(joined);

	اگر (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} ورنہ {
		/* update base content active fetch count */
		c->بنیاد.فعال++;
		NSLOG(netsurf, INFO, "%d fetches active", c->بنیاد.فعال);

		انتخاب (script_type) {
		حالت HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			توڑو;

		حالت HTML_SCRIPT_ASYNC:
			توڑو;

		حالت HTML_SCRIPT_DEFER:
			توڑو;

		طےشدہ:
			أكد(0);
		}
	}

	واپس ret;
}

ساکن dom_hubbub_error
exec_inline_script(html_content *c, dom_گرہ *گرہ, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	ساخت lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	ساخت html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_گرہ_get_متن_content(گرہ, &script);
	اگر ((exc != DOM_NO_ERR) || (script == تہی)) {
		واپس DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	اگر (nscript == تہی) {
		dom_string_unref(script);

		content_نشر_error(&c->بنیاد, NSERROR_NOMEM, تہی);
		واپس DOM_HUBBUB_NOMEM;

	}

	nscript->ڈیٹا.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	اگر (exc != DOM_NO_ERR) {
		واپس DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	اگر (script_handler != تہی) {
		script_handler(c->jsthread,
			       (ثابت uint8_t *)dom_string_ڈیٹا(script),
			       dom_string_byte_لمبائی(script),
			       "?inline script?");
	}
	واپس DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(خالی *ctx, dom_گرہ *گرہ)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	اگر (c->jsthread == تہی) {
		اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;

		پیغام_ڈیٹا.jsthread = &c->jsthread;
		content_نشر(&c->بنیاد, CONTENT_پیغام_GETTHREAD, &پیغام_ڈیٹا);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		اگر (c->jsthread == تہی) {
			/* no context and it could not be created, abort */
			واپس DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      گرہ);

	exc = dom_element_get_attribute(گرہ, corestring_dom_type, &mimetype);
	اگر (exc != DOM_NO_ERR || mimetype == تہی) {
		mimetype = dom_string_ref(corestring_dom_متن_javascript);
	}

	exc = dom_element_get_attribute(گرہ, corestring_dom_src, &src);
	اگر (exc != DOM_NO_ERR || src == تہی) {
		err = exec_inline_script(c, گرہ, mimetype);
	} ورنہ {
		err = exec_src_script(c, گرہ, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	واپس err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	ساخت html_script *s;
	غیرعلامتی صحیح i;

	برائے (i = 0, s = htmlc->scripts; i != htmlc->scripts_گنتی; i++, s++) {
		اگر (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			جاری;
		}
		اگر (s->ڈیٹا.handle == تہی) {
			/* We've not begun loading this? */
			جاری;
		}
		اگر (content_saw_insecure_objects(s->ڈیٹا.handle)) {
			واپس true;
		}
	}

	واپس false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	غیرعلامتی صحیح i;

	برائے (i = 0; i != html->scripts_گنتی; i++) {
		اگر (html->scripts[i].mimetype != تہی) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		انتخاب (html->scripts[i].type) {
		حالت HTML_SCRIPT_INLINE:
			اگر (html->scripts[i].ڈیٹا.string != تہی) {
				dom_string_unref(html->scripts[i].ڈیٹا.string);
			}
			توڑو;
		حالت HTML_SCRIPT_SYNC:
			/* fallthrough */
		حالت HTML_SCRIPT_ASYNC:
			/* fallthrough */
		حالت HTML_SCRIPT_DEFER:
			اگر (html->scripts[i].ڈیٹا.handle != تہی) {
				hlcache_handle_release(html->scripts[i].ڈیٹا.handle);
			}
			توڑو;
		}
	}
	آزاد(html->scripts);

	واپس NSERROR_OK;
}
