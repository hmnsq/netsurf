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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ "utils/config.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "javascript/js.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "content/fetch.h"
#ଅନ୍ତର୍ଭାଵ "content/hlcache.h"

#ଅନ୍ତର୍ଭାଵ "html/html.h"
#ଅନ୍ତର୍ଭାଵ "html/private.h"

ପ୍ରକାରବ୍ୟାଖ୍ୟା bool (script_handler_t)(ସଂରଚନା jsthread *jsthread, ଅଚଳ uint8_t *data, size_t size, ଅଚଳ ଅକ୍ଷର *name);


ସ୍ଥିର script_handler_t *select_script_handler(content_type ctype)
{
	ଯଦି (ctype == CONTENT_JS) {
		ଫେରନ୍ତୁ js_exec;
	}
	ଫେରନ୍ତୁ ଖାଲି;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା html_script *s;
	script_handler_t *script_handler;
	bool have_run_something = false;

	ଯଦି (c->jsthread == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_BAD_PARAMETER;
	}

	ପାଇଁ (i = 0, s = c->scripts; i != c->scripts_ଗଣନା; i++, s++) {
		ଯଦି (s->already_started) {
			ଜାରି;
		}

		ଯଦି ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			ଯଦି (s->data.handle == ଖାଲି)
				ଜାରି;

			/* ensure script content fetch status is not an error */
			ଯଦି (content_get_status(s->data.handle) ==
					CONTENT_STATUS_ERROR)
				ଜାରି;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->data.handle));
			ଯଦି (script_handler == ଖାଲି)
				ଜାରି; /* unsupported type */

			ଯଦି (content_get_status(s->data.handle) ==
					CONTENT_STATUS_DONE) {
				/* external script is now available */
				ଅଚଳ uint8_t *data;
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

	ଯଦି (have_run_something) {
		ଫେରନ୍ତୁ html_proceed_to_done(c);
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}

/* create new html script entry */
ସ୍ଥିର ସଂରଚନା html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			ଗଣନା html_script_type type)
{
	ସଂରଚନା html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  ଆକାର(ସଂରଚନା html_script) * (c->scripts_ଗଣନା + 1));
	ଯଦି (nscript == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_ଗଣନା];
	c->scripts_ଗଣନା++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	ଫେରନ୍ତୁ nscript;
}

/**
 * Callback for asyncronous scripts
 */
ସ୍ଥିର nserror
convert_script_async_cb(hlcache_handle *script,
			  ଅଚଳ hlcache_event *event,
			  ଶୂନ୍ୟ *pw)
{
	html_content *parent = pw;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା html_script *s;

	/* Find script */
	ପାଇଁ (i = 0, s = parent->scripts; i != parent->scripts_ଗଣନା; i++, s++) {
		ଯଦି (s->type == HTML_SCRIPT_ASYNC && s->data.handle == script)
			ଭାଙ୍ଗ;
	}

	ପ୍ରତିଜ୍ଞା(i != parent->scripts_ଗଣନା);

	ବାଛ (event->type) {
	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_LOADING:
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_READY:
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = ଖାଲି;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ଯଦି (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	ଅନ୍ୟଥା ଯଦି (parent->conversion_begun) {
		ଫେରନ୍ତୁ html_script_exec(parent, false);
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
ସ୍ଥିର nserror
convert_script_defer_cb(hlcache_handle *script,
			  ଅଚଳ hlcache_event *event,
			  ଶୂନ୍ୟ *pw)
{
	html_content *parent = pw;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା html_script *s;

	/* Find script */
	ପାଇଁ (i = 0, s = parent->scripts; i != parent->scripts_ଗଣନା; i++, s++) {
		ଯଦି (s->type == HTML_SCRIPT_DEFER && s->data.handle == script)
			ଭାଙ୍ଗ;
	}

	ପ୍ରତିଜ୍ଞା(i != parent->scripts_ଗଣନା);

	ବାଛ (event->type) {

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = ଖାଲି;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ଯଦି (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
ସ୍ଥିର nserror
convert_script_sync_cb(hlcache_handle *script,
			  ଅଚଳ hlcache_event *event,
			  ଶୂନ୍ୟ *pw)
{
	html_content *parent = pw;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା active_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	ପାଇଁ (i = 0, s = parent->scripts; i != parent->scripts_ଗଣନା; i++, s++) {
		ଯଦି (s->type == HTML_SCRIPT_SYNC &&
		    s->data.handle != script && s->already_started == false) {
			active_sync_scripts++;
		}
	}

	/* Find script */
	ପାଇଁ (i = 0, s = parent->scripts; i != parent->scripts_ଗଣନା; i++, s++) {
		ଯଦି (s->type == HTML_SCRIPT_SYNC && s->data.handle == script)
			ଭାଙ୍ଗ;
	}

	ପ୍ରତିଜ୍ଞା(i != parent->scripts_ଗଣନା);

	ବାଛ (event->type) {
	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->data.handle));
		ଯଦି (script_handler != ଖାଲି && parent->jsthread != ଖାଲି) {
			/* script has a handler */
			ଅଚଳ uint8_t *data;
			size_t size;
			data = content_get_source_data(s->data.handle, &size );
			script_handler(parent->jsthread, data, size,
				       nsurl_access(hlcache_handle_get_url(s->data.handle)));
		}

		/* continue parse */
		ଯଦି (parent->parser != ଖାଲି && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			ଯଦି (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = ଖାଲି;
		parent->base.active--;

		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* continue parse */
		ଯଦି (parent->parser != ଖାଲି && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			ଯଦି (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ଯଦି (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * process a script with a src tag
 */
ସ୍ଥିର dom_hubbub_error
exec_src_script(html_content *c,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_child_context child;
	ସଂରଚନା html_script *nscript;
	bool async;
	bool defer;
	ଗଣନା html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->base_url, dom_string_data(src), &joined);
	ଯଦି (ns_error != NSERROR_OK) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_ଗଣନା,
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
	exc = dom_element_has_attribute(ଗଣ୍ଠି, corestring_dom_async, &async);
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ DOM_HUBBUB_OK; /* dom error */
	}

	ଯଦି (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	ଯଦି (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} ଅନ୍ୟଥା {
		exc = dom_element_has_attribute(ଗଣ୍ଠି,
						corestring_dom_defer, &defer);
		ଯଦି (exc != DOM_NO_ERR) {
			ଫେରନ୍ତୁ DOM_HUBBUB_OK; /* dom error */
		}

		ଯଦି (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} ଅନ୍ୟଥା {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	ଯଦି (nscript == ଖାଲି) {
		nsurl_unref(joined);
		content_broadcast_error(&c->base, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->base),
					   ଖାଲି,
					   script_cb,
					   c,
					   &child,
					   CONTENT_SCRIPT,
					   &nscript->data.handle);


	nsurl_unref(joined);

	ଯଦି (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} ଅନ୍ୟଥା {
		/* update base content active fetch count */
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		ବାଛ (script_type) {
		ସ୍ଥିତି HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି HTML_SCRIPT_ASYNC:
			ଭାଙ୍ଗ;

		ସ୍ଥିତି HTML_SCRIPT_DEFER:
			ଭାଙ୍ଗ;

		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ପ୍ରତିଜ୍ଞା(0);
		}
	}

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର dom_hubbub_error
exec_inline_script(html_content *c, dom_ଗଣ୍ଠି *ଗଣ୍ଠି, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	ସଂରଚନା lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	ସଂରଚନା html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_ଗଣ୍ଠି_get_text_content(ଗଣ୍ଠି, &script);
	ଯଦି ((exc != DOM_NO_ERR) || (script == ଖାଲି)) {
		ଫେରନ୍ତୁ DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	ଯଦି (nscript == ଖାଲି) {
		dom_string_unref(script);

		content_broadcast_error(&c->base, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ DOM_HUBBUB_NOMEM;

	}

	nscript->data.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	ଯଦି (script_handler != ଖାଲି) {
		script_handler(c->jsthread,
			       (ଅଚଳ uint8_t *)dom_string_data(script),
			       dom_string_byte_length(script),
			       "?inline script?");
	}
	ଫେରନ୍ତୁ DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(ଶୂନ୍ୟ *ctx, dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	ଯଦି (c->jsthread == ଖାଲି) {
		ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;

		ସନ୍ଦେଶ_data.jsthread = &c->jsthread;
		content_broadcast(&c->base, CONTENT_ସନ୍ଦେଶ_GETTHREAD, &ସନ୍ଦେଶ_data);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		ଯଦି (c->jsthread == ଖାଲି) {
			/* no context and it could not be created, abort */
			ଫେରନ୍ତୁ DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      ଗଣ୍ଠି);

	exc = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_type, &mimetype);
	ଯଦି (exc != DOM_NO_ERR || mimetype == ଖାଲି) {
		mimetype = dom_string_ref(corestring_dom_text_javascript);
	}

	exc = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_src, &src);
	ଯଦି (exc != DOM_NO_ERR || src == ଖାଲି) {
		err = exec_inline_script(c, ଗଣ୍ଠି, mimetype);
	} ଅନ୍ୟଥା {
		err = exec_src_script(c, ଗଣ୍ଠି, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	ଫେରନ୍ତୁ err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	ସଂରଚନା html_script *s;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0, s = htmlc->scripts; i != htmlc->scripts_ଗଣନା; i++, s++) {
		ଯଦି (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			ଜାରି;
		}
		ଯଦି (s->data.handle == ଖାଲି) {
			/* We've not begun loading this? */
			ଜାରି;
		}
		ଯଦି (content_saw_insecure_objects(s->data.handle)) {
			ଫେରନ୍ତୁ true;
		}
	}

	ଫେରନ୍ତୁ false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; i != html->scripts_ଗଣନା; i++) {
		ଯଦି (html->scripts[i].mimetype != ଖାଲି) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		ବାଛ (html->scripts[i].type) {
		ସ୍ଥିତି HTML_SCRIPT_INLINE:
			ଯଦି (html->scripts[i].data.string != ଖାଲି) {
				dom_string_unref(html->scripts[i].data.string);
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି HTML_SCRIPT_SYNC:
			/* fallthrough */
		ସ୍ଥିତି HTML_SCRIPT_ASYNC:
			/* fallthrough */
		ସ୍ଥିତି HTML_SCRIPT_DEFER:
			ଯଦି (html->scripts[i].data.handle != ଖାଲି) {
				hlcache_handle_release(html->scripts[i].data.handle);
			}
			ଭାଙ୍ଗ;
		}
	}
	ମୁକ୍ତ(html->scripts);

	ଫେରନ୍ତୁ NSERROR_OK;
}
