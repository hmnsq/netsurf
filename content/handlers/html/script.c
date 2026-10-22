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

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "utils/config.h"
#include "utils/corestrings.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "netsurf/content.h"
#include "javascript/js.h"
#include "content/content_protected.h"
#include "content/content_factory.h"
#include "content/fetch.h"
#include "content/hlcache.h"

#include "html/html.h"
#include "html/private.h"

typedef bool (script_handler_t)(sa_nracanaa jsthread *jsthread, const uint8_t *data, size_t size, const var_na *name);


static script_handler_t *select_script_handler(content_type ctype)
{
	ᨀᨘ (ctype == CONTENT_JS) {
		ᨀᨘᨒ js_exec;
	}
	ᨀᨘᨒ NULL;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	unsigned puur_naa_nka i;
	sa_nracanaa html_script *s;
	script_handler_t *script_handler;
	bool have_run_something = false;

	ᨀᨘ (c->jsthread == NULL) {
		ᨀᨘᨒ NSERROR_BAD_PARAMETER;
	}

	for (i = 0, s = c->scripts; i != c->scripts_count; i++, s++) {
		ᨀᨘ (s->already_started) {
			continue;
		}

		ᨀᨘ ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			ᨀᨘ (s->data.handle == NULL)
				continue;

			/* ensure script content fetch status is not an error */
			ᨀᨘ (content_get_status(s->data.handle) ==
					CONTENT_STATUS_ERROR)
				continue;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->data.handle));
			ᨀᨘ (script_handler == NULL)
				continue; /* unsupported type */

			ᨀᨘ (content_get_status(s->data.handle) ==
					CONTENT_STATUS_DONE) {
				/* external script is now available */
				const uint8_t *data;
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

	ᨀᨘ (have_run_something) {
		ᨀᨘᨒ html_proceed_to_done(c);
	}

	ᨀᨘᨒ NSERROR_OK;
}

/* create new html script entry */
static sa_nracanaa html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			ga_nanaa html_script_type type)
{
	sa_nracanaa html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  parimaa_na(sa_nracanaa html_script) * (c->scripts_count + 1));
	ᨀᨘ (nscript == NULL) {
		ᨀᨘᨒ NULL;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_count];
	c->scripts_count++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	ᨀᨘᨒ nscript;
}

/**
 * Callback for asyncronous scripts
 */
static nserror
convert_script_async_cb(hlcache_handle *script,
			  const hlcache_event *event,
			  void *pw)
{
	html_content *parent = pw;
	unsigned puur_naa_nka i;
	sa_nracanaa html_script *s;

	/* Find script */
	for (i = 0, s = parent->scripts; i != parent->scripts_count; i++, s++) {
		ᨀᨘ (s->type == HTML_SCRIPT_ASYNC && s->data.handle == script)
			bha_nga;
	}

	assert(i != parent->scripts_count);

	switch (event->type) {
	pak_sa CONTENT_MSG_LOADING:
		bha_nga;

	pak_sa CONTENT_MSG_READY:
		bha_nga;

	pak_sa CONTENT_MSG_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		bha_nga;

	pak_sa CONTENT_MSG_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = NULL;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		bha_nga;

	default:
		bha_nga;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ᨀᨘ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	ᨀᨘᨁ ᨀᨘ (parent->conversion_begun) {
		ᨀᨘᨒ html_script_exec(parent, false);
	}

	ᨀᨘᨒ NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
static nserror
convert_script_defer_cb(hlcache_handle *script,
			  const hlcache_event *event,
			  void *pw)
{
	html_content *parent = pw;
	unsigned puur_naa_nka i;
	sa_nracanaa html_script *s;

	/* Find script */
	for (i = 0, s = parent->scripts; i != parent->scripts_count; i++, s++) {
		ᨀᨘ (s->type == HTML_SCRIPT_DEFER && s->data.handle == script)
			bha_nga;
	}

	assert(i != parent->scripts_count);

	switch (event->type) {

	pak_sa CONTENT_MSG_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		bha_nga;

	pak_sa CONTENT_MSG_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = NULL;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		bha_nga;

	default:
		bha_nga;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ᨀᨘ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ᨀᨘᨒ NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
static nserror
convert_script_sync_cb(hlcache_handle *script,
			  const hlcache_event *event,
			  void *pw)
{
	html_content *parent = pw;
	unsigned puur_naa_nka i;
	sa_nracanaa html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	unsigned puur_naa_nka active_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	for (i = 0, s = parent->scripts; i != parent->scripts_count; i++, s++) {
		ᨀᨘ (s->type == HTML_SCRIPT_SYNC &&
		    s->data.handle != script && s->already_started == false) {
			active_sync_scripts++;
		}
	}

	/* Find script */
	for (i = 0, s = parent->scripts; i != parent->scripts_count; i++, s++) {
		ᨀᨘ (s->type == HTML_SCRIPT_SYNC && s->data.handle == script)
			bha_nga;
	}

	assert(i != parent->scripts_count);

	switch (event->type) {
	pak_sa CONTENT_MSG_DONE:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->data.handle));
		ᨀᨘ (script_handler != NULL && parent->jsthread != NULL) {
			/* script has a handler */
			const uint8_t *data;
			size_t size;
			data = content_get_source_data(s->data.handle, &size );
			script_handler(parent->jsthread, data, size,
				       nsurl_access(hlcache_handle_get_url(s->data.handle)));
		}

		/* continue parse */
		ᨀᨘ (parent->parser != NULL && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			ᨀᨘ (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		bha_nga;

	pak_sa CONTENT_MSG_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(script);
		s->data.handle = NULL;
		parent->base.active--;

		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);

		s->already_started = true;

		/* continue parse */
		ᨀᨘ (parent->parser != NULL && active_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(parent->parser, false);
			ᨀᨘ (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		bha_nga;

	default:
		bha_nga;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ᨀᨘ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ᨀᨘᨒ NSERROR_OK;
}

/**
 * process a script with a src tag
 */
static dom_hubbub_error
exec_src_script(html_content *c,
		dom_node *node,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_child_context child;
	sa_nracanaa html_script *nscript;
	bool async;
	bool defer;
	ga_nanaa html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->base_url, dom_string_data(src), &joined);
	ᨀᨘ (ns_error != NSERROR_OK) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, NULL);
		ᨀᨘᨒ DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_count,
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
	exc = dom_element_has_attribute(node, corestring_dom_async, &async);
	ᨀᨘ (exc != DOM_NO_ERR) {
		ᨀᨘᨒ DOM_HUBBUB_OK; /* dom error */
	}

	ᨀᨘ (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	ᨀᨘ (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} ᨀᨘᨁ {
		exc = dom_element_has_attribute(node,
						corestring_dom_defer, &defer);
		ᨀᨘ (exc != DOM_NO_ERR) {
			ᨀᨘᨒ DOM_HUBBUB_OK; /* dom error */
		}

		ᨀᨘ (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} ᨀᨘᨁ {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	ᨀᨘ (nscript == NULL) {
		nsurl_unref(joined);
		content_broadcast_error(&c->base, NSERROR_NOMEM, NULL);
		ᨀᨘᨒ DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->base),
					   NULL,
					   script_cb,
					   c,
					   &child,
					   CONTENT_SCRIPT,
					   &nscript->data.handle);


	nsurl_unref(joined);

	ᨀᨘ (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} ᨀᨘᨁ {
		/* update base content active fetch count */
		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

		switch (script_type) {
		pak_sa HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			bha_nga;

		pak_sa HTML_SCRIPT_ASYNC:
			bha_nga;

		pak_sa HTML_SCRIPT_DEFER:
			bha_nga;

		default:
			assert(0);
		}
	}

	ᨀᨘᨒ ret;
}

static dom_hubbub_error
exec_inline_script(html_content *c, dom_node *node, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	sa_nracanaa lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	sa_nracanaa html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_node_get_text_content(node, &script);
	ᨀᨘ ((exc != DOM_NO_ERR) || (script == NULL)) {
		ᨀᨘᨒ DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	ᨀᨘ (nscript == NULL) {
		dom_string_unref(script);

		content_broadcast_error(&c->base, NSERROR_NOMEM, NULL);
		ᨀᨘᨒ DOM_HUBBUB_NOMEM;

	}

	nscript->data.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	ᨀᨘ (exc != DOM_NO_ERR) {
		ᨀᨘᨒ DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	ᨀᨘ (script_handler != NULL) {
		script_handler(c->jsthread,
			       (const uint8_t *)dom_string_data(script),
			       dom_string_byte_length(script),
			       "?inline script?");
	}
	ᨀᨘᨒ DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(void *ctx, dom_node *node)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	ᨀᨘ (c->jsthread == NULL) {
		sa_ngha content_msg_data msg_data;

		msg_data.jsthread = &c->jsthread;
		content_broadcast(&c->base, CONTENT_MSG_GETTHREAD, &msg_data);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		ᨀᨘ (c->jsthread == NULL) {
			/* no context and it could not be created, abort */
			ᨀᨘᨒ DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      node);

	exc = dom_element_get_attribute(node, corestring_dom_type, &mimetype);
	ᨀᨘ (exc != DOM_NO_ERR || mimetype == NULL) {
		mimetype = dom_string_ref(corestring_dom_text_javascript);
	}

	exc = dom_element_get_attribute(node, corestring_dom_src, &src);
	ᨀᨘ (exc != DOM_NO_ERR || src == NULL) {
		err = exec_inline_script(c, node, mimetype);
	} ᨀᨘᨁ {
		err = exec_src_script(c, node, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	ᨀᨘᨒ err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	sa_nracanaa html_script *s;
	unsigned puur_naa_nka i;

	for (i = 0, s = htmlc->scripts; i != htmlc->scripts_count; i++, s++) {
		ᨀᨘ (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			continue;
		}
		ᨀᨘ (s->data.handle == NULL) {
			/* We've not begun loading this? */
			continue;
		}
		ᨀᨘ (content_saw_insecure_objects(s->data.handle)) {
			ᨀᨘᨒ true;
		}
	}

	ᨀᨘᨒ false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	unsigned puur_naa_nka i;

	for (i = 0; i != html->scripts_count; i++) {
		ᨀᨘ (html->scripts[i].mimetype != NULL) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		switch (html->scripts[i].type) {
		pak_sa HTML_SCRIPT_INLINE:
			ᨀᨘ (html->scripts[i].data.string != NULL) {
				dom_string_unref(html->scripts[i].data.string);
			}
			bha_nga;
		pak_sa HTML_SCRIPT_SYNC:
			/* fallthrough */
		pak_sa HTML_SCRIPT_ASYNC:
			/* fallthrough */
		pak_sa HTML_SCRIPT_DEFER:
			ᨀᨘ (html->scripts[i].data.handle != NULL) {
				hlcache_handle_release(html->scripts[i].data.handle);
			}
			bha_nga;
		}
	}
	free(html->scripts);

	ᨀᨘᨒ NSERROR_OK;
}
