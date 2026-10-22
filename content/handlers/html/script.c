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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "utils/config.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "javascript/js.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "content/fetch.h"
#અન્તર્ભાવ "content/hlcache.h"

#અન્તર્ભાવ "html/html.h"
#અન્તર્ભાવ "html/private.h"

પ્રકારવ્યાખ્યા bool (script_handler_t)(સંરચના jsthread *jsthread, અચલ uint8_t *data, size_t size, અચલ અક્ષર *name);


સ્થિર script_handler_t *select_script_handler(content_type ctype)
{
	જો (ctype == CONTENT_JS) {
		પાછા js_exec;
	}
	પાછા ખાલી;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	unsigned પૂર્ણાંક i;
	સંરચના html_script *s;
	script_handler_t *script_handler;
	bool have_run_something = false;

	જો (c->jsthread == ખાલી) {
		પાછા NSERROR_BAD_PARAMETER;
	}

	માટે (i = 0, s = c->scripts; i != c->scripts_ગણતરી; i++, s++) {
		જો (s->already_started) {
			ચાલુ;
		}

		જો ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			જો (s->data.handle == ખાલી)
				ચાલુ;

			/* ensure script content fetch status is not an error */
			જો (content_get_status(s->data.handle) ==
					CONTENT_STATUS_ERROR)
				ચાલુ;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->data.handle));
			જો (script_handler == ખાલી)
				ચાલુ; /* unsupported type */

			જો (content_get_status(s->data.handle) ==
					CONTENT_STATUS_DONE) {
				/* external script is now available */
				અચલ uint8_t *data;
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

	જો (have_run_something) {
		પાછા html_proceed_to_done(c);
	}

	પાછા NSERROR_OK;
}

/* create new html script entry */
સ્થિર સંરચના html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			ગણના html_script_type type)
{
	સંરચના html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  કદ(સંરચના html_script) * (c->scripts_ગણતરી + 1));
	જો (nscript == ખાલી) {
		પાછા ખાલી;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_ગણતરી];
	c->scripts_ગણતરી++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	પાછા nscript;
}

/**
 * Callback for asyncronous scripts
 */
સ્થિર nserror
convert_script_async_cb(hlcache_handle *script,
			  અચલ hlcache_event *event,
			  શૂન્ય *pw)
{
	html_content *parent = pw;
	unsigned પૂર્ણાંક i;
	સંરચના html_script *s;

	/* Find script */
	માટે (i = 0, s = parent->scripts; i != parent->scripts_ગણતરી; i++, s++) {
		જો (s->type == HTML_SCRIPT_ASYNC && s->data.handle == script)
			તોડો;
	}

	પ્રતિજ્ઞા(i != parent->scripts_ગણતરી);

	પસંદગી (event->type) {
	સ્થિતિ CONTENT_સંદેશ_LOADING:
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_READY:
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		તોડો;

	સ્થિતિ CONTENT_સંદેશ_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = ખાલી;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		તોડો;

	મૂળભૂત:
		તોડો;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	જો (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	નહીંતર જો (parent->conversion_begun) {
		પાછા html_script_exec(parent, false);
	}

	પાછા NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
સ્થિર nserror
convert_script_defer_cb(hlcache_handle *script,
			  અચલ hlcache_event *event,
			  શૂન્ય *pw)
{
	html_content *parent = pw;
	unsigned પૂર્ણાંક i;
	સંરચના html_script *s;

	/* Find script */
	માટે (i = 0, s = parent->scripts; i != parent->scripts_ગણતરી; i++, s++) {
		જો (s->type == HTML_SCRIPT_DEFER && s->data.handle == script)
			તોડો;
	}

	પ્રતિજ્ઞા(i != parent->scripts_ગણતરી);

	પસંદગી (event->type) {

	સ્થિતિ CONTENT_સંદેશ_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		તોડો;

	સ્થિતિ CONTENT_સંદેશ_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = ખાલી;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		તોડો;

	મૂળભૂત:
		તોડો;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	જો (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	પાછા NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
સ્થિર nserror
convert_script_sync_cb(hlcache_handle *script,
			  અચલ hlcache_event *event,
			  શૂન્ય *pw)
{
	html_content *parent = pw;
	unsigned પૂર્ણાંક i;
	સંરચના html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	unsigned પૂર્ણાંક active_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	માટે (i = 0, s = parent->scripts; i != parent->scripts_ગણતરી; i++, s++) {
		જો (s->type == HTML_SCRIPT_SYNC &&
		    s->data.handle != script && s->already_started == false) {
			active_sync_scripts++;
		}
	}

	/* Find script */
	માટે (i = 0, s = parent->scripts; i != parent->scripts_ગણતરી; i++, s++) {
		જો (s->type == HTML_SCRIPT_SYNC && s->data.handle == script)
			તોડો;
	}

	પ્રતિજ્ઞા(i != parent->scripts_ગણતરી);

	પસંદગી (event->type) {
	સ્થિતિ CONTENT_સંદેશ_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->data.handle));
		જો (script_handler != ખાલી && parent->jsthread != ખાલી) {
			/* script has a handler */
			અચલ uint8_t *data;
			size_t size;
			data = content_get_source_data(s->data.handle, &size );
			script_handler(parent->jsthread, data, size,
				       nsurl_access(hlcache_handle_get_url(s->data.handle)));
		}

		/* continue parse */
		જો (parent->parser != ખાલી && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			જો (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		તોડો;

	સ્થિતિ CONTENT_સંદેશ_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = ખાલી;
		parent->base.active--;

		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* continue parse */
		જો (parent->parser != ખાલી && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			જો (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		તોડો;

	મૂળભૂત:
		તોડો;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	જો (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	પાછા NSERROR_OK;
}

/**
 * process a script with a src tag
 */
સ્થિર dom_hubbub_error
exec_src_script(html_content *c,
		dom_ગાંઠ *ગાંઠ,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_child_context child;
	સંરચના html_script *nscript;
	bool async;
	bool defer;
	ગણના html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->base_url, dom_string_data(src), &joined);
	જો (ns_error != NSERROR_OK) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, ખાલી);
		પાછા DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_ગણતરી,
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
	exc = dom_element_has_attribute(ગાંઠ, corestring_dom_async, &async);
	જો (exc != DOM_NO_ERR) {
		પાછા DOM_HUBBUB_OK; /* dom error */
	}

	જો (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	જો (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} નહીંતર {
		exc = dom_element_has_attribute(ગાંઠ,
						corestring_dom_defer, &defer);
		જો (exc != DOM_NO_ERR) {
			પાછા DOM_HUBBUB_OK; /* dom error */
		}

		જો (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} નહીંતર {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	જો (nscript == ખાલી) {
		nsurl_unref(joined);
		content_broadcast_error(&c->base, NSERROR_NOMEM, ખાલી);
		પાછા DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->base),
					   ખાલી,
					   script_cb,
					   c,
					   &child,
					   CONTENT_SCRIPT,
					   &nscript->data.handle);


	nsurl_unref(joined);

	જો (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} નહીંતર {
		/* update base content active fetch count */
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		પસંદગી (script_type) {
		સ્થિતિ HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			તોડો;

		સ્થિતિ HTML_SCRIPT_ASYNC:
			તોડો;

		સ્થિતિ HTML_SCRIPT_DEFER:
			તોડો;

		મૂળભૂત:
			પ્રતિજ્ઞા(0);
		}
	}

	પાછા ret;
}

સ્થિર dom_hubbub_error
exec_inline_script(html_content *c, dom_ગાંઠ *ગાંઠ, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	સંરચના lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	સંરચના html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_ગાંઠ_get_text_content(ગાંઠ, &script);
	જો ((exc != DOM_NO_ERR) || (script == ખાલી)) {
		પાછા DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	જો (nscript == ખાલી) {
		dom_string_unref(script);

		content_broadcast_error(&c->base, NSERROR_NOMEM, ખાલી);
		પાછા DOM_HUBBUB_NOMEM;

	}

	nscript->data.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	જો (exc != DOM_NO_ERR) {
		પાછા DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	જો (script_handler != ખાલી) {
		script_handler(c->jsthread,
			       (અચલ uint8_t *)dom_string_data(script),
			       dom_string_byte_length(script),
			       "?inline script?");
	}
	પાછા DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(શૂન્ય *ctx, dom_ગાંઠ *ગાંઠ)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	જો (c->jsthread == ખાલી) {
		સંઘ content_સંદેશ_data સંદેશ_data;

		સંદેશ_data.jsthread = &c->jsthread;
		content_broadcast(&c->base, CONTENT_સંદેશ_GETTHREAD, &સંદેશ_data);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		જો (c->jsthread == ખાલી) {
			/* no context and it could not be created, abort */
			પાછા DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      ગાંઠ);

	exc = dom_element_get_attribute(ગાંઠ, corestring_dom_type, &mimetype);
	જો (exc != DOM_NO_ERR || mimetype == ખાલી) {
		mimetype = dom_string_ref(corestring_dom_text_javascript);
	}

	exc = dom_element_get_attribute(ગાંઠ, corestring_dom_src, &src);
	જો (exc != DOM_NO_ERR || src == ખાલી) {
		err = exec_inline_script(c, ગાંઠ, mimetype);
	} નહીંતર {
		err = exec_src_script(c, ગાંઠ, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	પાછા err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	સંરચના html_script *s;
	unsigned પૂર્ણાંક i;

	માટે (i = 0, s = htmlc->scripts; i != htmlc->scripts_ગણતરી; i++, s++) {
		જો (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			ચાલુ;
		}
		જો (s->data.handle == ખાલી) {
			/* We've not begun loading this? */
			ચાલુ;
		}
		જો (content_saw_insecure_objects(s->data.handle)) {
			પાછા true;
		}
	}

	પાછા false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	unsigned પૂર્ણાંક i;

	માટે (i = 0; i != html->scripts_ગણતરી; i++) {
		જો (html->scripts[i].mimetype != ખાલી) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		પસંદગી (html->scripts[i].type) {
		સ્થિતિ HTML_SCRIPT_INLINE:
			જો (html->scripts[i].data.string != ખાલી) {
				dom_string_unref(html->scripts[i].data.string);
			}
			તોડો;
		સ્થિતિ HTML_SCRIPT_SYNC:
			/* fallthrough */
		સ્થિતિ HTML_SCRIPT_ASYNC:
			/* fallthrough */
		સ્થિતિ HTML_SCRIPT_DEFER:
			જો (html->scripts[i].data.handle != ખાલી) {
				hlcache_handle_release(html->scripts[i].data.handle);
			}
			તોડો;
		}
	}
	મુક્ત(html->scripts);

	પાછા NSERROR_OK;
}
