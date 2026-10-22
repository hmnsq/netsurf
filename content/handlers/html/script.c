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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "utils/config.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "javascript/js.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "content/fetch.h"
#అన్తర్భావ "content/hlcache.h"

#అన్తర్భావ "html/html.h"
#అన్తర్భావ "html/private.h"

రకనిర్వచనం bool (script_handler_t)(నిర్మాణం jsthread *jsthread, స్థిరరాశి uint8_t *దత్తాంశం, size_t కొలత, స్థిరరాశి అక్షరం *name);


స్థిర script_handler_t *select_script_handler(content_type ctype)
{
	ఒకవేళ (ctype == CONTENT_JS) {
		తిరిగి js_exec;
	}
	తిరిగి ఖాళీ;
}


/* exported internal interface documented in html/html_internal.h */
nserror html_script_exec(html_content *c, bool allow_defer)
{
	గుర్తులేని పూర్ణాంకం i;
	నిర్మాణం html_script *s;
	script_handler_t *script_handler;
	bool have_నిరంతరం_something = false;

	ఒకవేళ (c->jsthread == ఖాళీ) {
		తిరిగి NSERROR_BAD_PARAMETER;
	}

	కొరకు (i = 0, s = c->scripts; i != c->scripts_లెక్క; i++, s++) {
		ఒకవేళ (s->already_started) {
			కొనసాగు;
		}

		ఒకవేళ ((s->type == HTML_SCRIPT_ASYNC) ||
		    (allow_defer && (s->type == HTML_SCRIPT_DEFER))) {
			/* ensure script content is present */
			ఒకవేళ (s->దత్తాంశం.handle == ఖాళీ)
				కొనసాగు;

			/* ensure script content fetch status is not an error */
			ఒకవేళ (content_get_స్థితి(s->దత్తాంశం.handle) ==
					CONTENT_స్థితి_ERROR)
				కొనసాగు;

			/* ensure script handler for content type */
			script_handler = select_script_handler(
					content_get_type(s->దత్తాంశం.handle));
			ఒకవేళ (script_handler == ఖాళీ)
				కొనసాగు; /* unsupported type */

			ఒకవేళ (content_get_స్థితి(s->దత్తాంశం.handle) ==
					CONTENT_స్థితి_పూర్తి) {
				/* external script is now available */
				స్థిరరాశి uint8_t *దత్తాంశం;
				size_t కొలత;
				దత్తాంశం = content_get_ఆధారం_దత్తాంశం(
						s->దత్తాంశం.handle, &కొలత );
				script_handler(c->jsthread, దత్తాంశం, కొలత,
					       nsurl_access(hlcache_handle_get_url(s->దత్తాంశం.handle)));
				have_నిరంతరం_something = true;
				/* We have to re-acquire this here since the
				 * c->scripts array may have been reallocated
				 * as a result of executing this script.
				 */
				s = &(c->scripts[i]);

				s->already_started = true;

			}
		}
	}

	ఒకవేళ (have_నిరంతరం_something) {
		తిరిగి html_proceed_గమ్యం_పూర్తి(c);
	}

	తిరిగి NSERROR_OK;
}

/* create new html script entry */
స్థిర నిర్మాణం html_script *
html_process_new_script(html_content *c,
			dom_string *mimetype,
			గణన html_script_type type)
{
	నిర్మాణం html_script *nscript;
	/* add space for new script entry */
	nscript = realloc(c->scripts,
			  పరిమాణం(నిర్మాణం html_script) * (c->scripts_లెక్క + 1));
	ఒకవేళ (nscript == ఖాళీ) {
		తిరిగి ఖాళీ;
	}

	c->scripts = nscript;

	/* increment script entry count */
	nscript = &c->scripts[c->scripts_లెక్క];
	c->scripts_లెక్క++;

	nscript->already_started = false;
	nscript->parser_inserted = false;
	nscript->force_async = true;
	nscript->ready_exec = false;
	nscript->async = false;
	nscript->defer = false;

	nscript->type = type;

	nscript->mimetype = dom_string_ref(mimetype); /* reference mimetype */

	తిరిగి nscript;
}

/**
 * Callback for asyncronous scripts
 */
స్థిర nserror
convert_script_async_cb(hlcache_handle *script,
			  స్థిరరాశి hlcache_event *event,
			  శూన్య *pw)
{
	html_content *మాతృ = pw;
	గుర్తులేని పూర్ణాంకం i;
	నిర్మాణం html_script *s;

	/* Find script */
	కొరకు (i = 0, s = మాతృ->scripts; i != మాతృ->scripts_లెక్క; i++, s++) {
		ఒకవేళ (s->type == HTML_SCRIPT_ASYNC && s->దత్తాంశం.handle == script)
			ఆపు;
	}

	ప్రతిజ్ఞా(i != మాతృ->scripts_లెక్క);

	ఎంపిక (event->type) {
	స్థితి CONTENT_సందేశం_LOADING:
		ఆపు;

	స్థితి CONTENT_సందేశం_READY:
		ఆపు;

	స్థితి CONTENT_సందేశం_పూర్తి:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		మాతృ->పునాది.చురుకైన--;
		NSLOG(netsurf, INFO, "%d fetches active", మాతృ->పునాది.చురుకైన);

		ఆపు;

	స్థితి CONTENT_సందేశం_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->దత్తాంశం.errordata.errormsg);

		hlcache_handle_release(script);
		s->దత్తాంశం.handle = ఖాళీ;
		మాతృ->పునాది.చురుకైన--;
		NSLOG(netsurf, INFO, "%d fetches active", మాతృ->పునాది.చురుకైన);

		ఆపు;

	అప్రమేయ:
		ఆపు;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ఒకవేళ (html_can_begin_conversion(మాతృ)) {
		html_begin_conversion(మాతృ);
	}

	/* if we have already started converting though, then we can handle the
	 * scripts as they come in.
	 */
	లేకపోతే ఒకవేళ (మాతృ->conversion_begun) {
		తిరిగి html_script_exec(మాతృ, false);
	}

	తిరిగి NSERROR_OK;
}

/**
 * Callback for defer scripts
 */
స్థిర nserror
convert_script_defer_cb(hlcache_handle *script,
			  స్థిరరాశి hlcache_event *event,
			  శూన్య *pw)
{
	html_content *మాతృ = pw;
	గుర్తులేని పూర్ణాంకం i;
	నిర్మాణం html_script *s;

	/* Find script */
	కొరకు (i = 0, s = మాతృ->scripts; i != మాతృ->scripts_లెక్క; i++, s++) {
		ఒకవేళ (s->type == HTML_SCRIPT_DEFER && s->దత్తాంశం.handle == script)
			ఆపు;
	}

	ప్రతిజ్ఞా(i != మాతృ->scripts_లెక్క);

	ఎంపిక (event->type) {

	స్థితి CONTENT_సందేశం_పూర్తి:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		మాతృ->పునాది.చురుకైన--;
		NSLOG(netsurf, INFO, "%d fetches active", మాతృ->పునాది.చురుకైన);

		ఆపు;

	స్థితి CONTENT_సందేశం_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->దత్తాంశం.errordata.errormsg);

		hlcache_handle_release(script);
		s->దత్తాంశం.handle = ఖాళీ;
		మాతృ->పునాది.చురుకైన--;
		NSLOG(netsurf, INFO, "%d fetches active", మాతృ->పునాది.చురుకైన);

		ఆపు;

	అప్రమేయ:
		ఆపు;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ఒకవేళ (html_can_begin_conversion(మాతృ)) {
		html_begin_conversion(మాతృ);
	}

	తిరిగి NSERROR_OK;
}

/**
 * Callback for syncronous scripts
 */
స్థిర nserror
convert_script_sync_cb(hlcache_handle *script,
			  స్థిరరాశి hlcache_event *event,
			  శూన్య *pw)
{
	html_content *మాతృ = pw;
	గుర్తులేని పూర్ణాంకం i;
	నిర్మాణం html_script *s;
	script_handler_t *script_handler;
	dom_hubbub_error err;
	గుర్తులేని పూర్ణాంకం చురుకైన_sync_scripts = 0;

	/* Count sync scripts which have yet to complete (other than us) */
	కొరకు (i = 0, s = మాతృ->scripts; i != మాతృ->scripts_లెక్క; i++, s++) {
		ఒకవేళ (s->type == HTML_SCRIPT_SYNC &&
		    s->దత్తాంశం.handle != script && s->already_started == false) {
			చురుకైన_sync_scripts++;
		}
	}

	/* Find script */
	కొరకు (i = 0, s = మాతృ->scripts; i != మాతృ->scripts_లెక్క; i++, s++) {
		ఒకవేళ (s->type == HTML_SCRIPT_SYNC && s->దత్తాంశం.handle == script)
			ఆపు;
	}

	ప్రతిజ్ఞా(i != మాతృ->scripts_లెక్క);

	ఎంపిక (event->type) {
	స్థితి CONTENT_సందేశం_పూర్తి:
		NSLOG(netsurf, INFO, "script %d done '%s'", i,
		      nsurl_access(hlcache_handle_get_url(script)));
		మాతృ->పునాది.చురుకైన--;
		NSLOG(netsurf, INFO, "%d fetches active", మాతృ->పునాది.చురుకైన);

		s->already_started = true;

		/* attempt to execute script */
		script_handler = select_script_handler(content_get_type(s->దత్తాంశం.handle));
		ఒకవేళ (script_handler != ఖాళీ && మాతృ->jsthread != ఖాళీ) {
			/* script has a handler */
			స్థిరరాశి uint8_t *దత్తాంశం;
			size_t కొలత;
			దత్తాంశం = content_get_ఆధారం_దత్తాంశం(s->దత్తాంశం.handle, &కొలత );
			script_handler(మాతృ->jsthread, దత్తాంశం, కొలత,
				       nsurl_access(hlcache_handle_get_url(s->దత్తాంశం.handle)));
		}

		/* continue parse */
		ఒకవేళ (మాతృ->parser != ఖాళీ && చురుకైన_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(మాతృ->parser, false);
			ఒకవేళ (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		ఆపు;

	స్థితి CONTENT_సందేశం_ERROR:
		NSLOG(netsurf, INFO, "script %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(script)),
		      event->దత్తాంశం.errordata.errormsg);

		hlcache_handle_release(script);
		s->దత్తాంశం.handle = ఖాళీ;
		మాతృ->పునాది.చురుకైన--;

		NSLOG(netsurf, INFO, "%d fetches active", మాతృ->పునాది.చురుకైన);

		s->already_started = true;

		/* continue parse */
		ఒకవేళ (మాతృ->parser != ఖాళీ && చురుకైన_sync_scripts == 0) {
			err = dom_hubbub_parser_pause(మాతృ->parser, false);
			ఒకవేళ (err != DOM_HUBBUB_OK) {
				NSLOG(netsurf, INFO, "unpause returned 0x%x", err);
			}
		}

		ఆపు;

	అప్రమేయ:
		ఆపు;
	}

	/* if there are no active fetches remaining begin post parse
	 * conversion
	 */
	ఒకవేళ (html_can_begin_conversion(మాతృ)) {
		html_begin_conversion(మాతృ);
	}

	తిరిగి NSERROR_OK;
}

/**
 * process a script with a src tag
 */
స్థిర dom_hubbub_error
exec_src_script(html_content *c,
		dom_కణుపు *కణుపు,
		dom_string *mimetype,
		dom_string *src)
{
	nserror ns_error;
	nsurl *joined;
	hlcache_బిడ్డ_context బిడ్డ;
	నిర్మాణం html_script *nscript;
	bool async;
	bool defer;
	గణన html_script_type script_type;
	hlcache_handle_callback script_cb;
	dom_hubbub_error ret = DOM_HUBBUB_OK;
	dom_exception exc; /* returned by libdom functions */

	/* src url */
	ns_error = nsurl_join(c->పునాది_url, dom_string_దత్తాంశం(src), &joined);
	ఒకవేళ (ns_error != NSERROR_OK) {
		content_ప్రసారం_error(&c->పునాది, NSERROR_NOMEM, ఖాళీ);
		తిరిగి DOM_HUBBUB_NOMEM;
	}

	NSLOG(netsurf, INFO, "script %i '%s'", c->scripts_లెక్క,
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
	exc = dom_element_has_attribute(కణుపు, corestring_dom_async, &async);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి DOM_HUBBUB_OK; /* dom error */
	}

	ఒకవేళ (c->parse_completed) {
		/* After parse completed, all scripts are essentially async */
		async = true;
		defer = false;
	}

	ఒకవేళ (async) {
		/* asyncronous script */
		script_type = HTML_SCRIPT_ASYNC;
		script_cb = convert_script_async_cb;

	} లేకపోతే {
		exc = dom_element_has_attribute(కణుపు,
						corestring_dom_defer, &defer);
		ఒకవేళ (exc != DOM_NO_ERR) {
			తిరిగి DOM_HUBBUB_OK; /* dom error */
		}

		ఒకవేళ (defer) {
			/* defered script */
			script_type = HTML_SCRIPT_DEFER;
			script_cb = convert_script_defer_cb;
		} లేకపోతే {
			/* syncronous script */
			script_type = HTML_SCRIPT_SYNC;
			script_cb = convert_script_sync_cb;
		}
	}

	nscript = html_process_new_script(c, mimetype, script_type);
	ఒకవేళ (nscript == ఖాళీ) {
		nsurl_unref(joined);
		content_ప్రసారం_error(&c->పునాది, NSERROR_NOMEM, ఖాళీ);
		తిరిగి DOM_HUBBUB_NOMEM;
	}

	/* set up child fetch encoding and quirks */
	బిడ్డ.charset = c->encoding;
	బిడ్డ.quirks = c->పునాది.quirks;

	ns_error = hlcache_handle_retrieve(joined,
					   0,
					   content_get_url(&c->పునాది),
					   ఖాళీ,
					   script_cb,
					   c,
					   &బిడ్డ,
					   CONTENT_SCRIPT,
					   &nscript->దత్తాంశం.handle);


	nsurl_unref(joined);

	ఒకవేళ (ns_error != NSERROR_OK) {
		/* @todo Deal with fetch error better. currently assume
		 * fetch never became active
		 */
		/* mark duff script fetch as already started */
		nscript->already_started = true;
		NSLOG(netsurf, INFO, "Fetch failed with error %d", ns_error);
	} లేకపోతే {
		/* update base content active fetch count */
		c->పునాది.చురుకైన++;
		NSLOG(netsurf, INFO, "%d fetches active", c->పునాది.చురుకైన);

		ఎంపిక (script_type) {
		స్థితి HTML_SCRIPT_SYNC:
			ret =  DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED;
			ఆపు;

		స్థితి HTML_SCRIPT_ASYNC:
			ఆపు;

		స్థితి HTML_SCRIPT_DEFER:
			ఆపు;

		అప్రమేయ:
			ప్రతిజ్ఞా(0);
		}
	}

	తిరిగి ret;
}

స్థిర dom_hubbub_error
exec_inline_script(html_content *c, dom_కణుపు *కణుపు, dom_string *mimetype)
{
	dom_string *script;
	dom_exception exc; /* returned by libdom functions */
	నిర్మాణం lwc_string_s *lwcmimetype;
	script_handler_t *script_handler;
	నిర్మాణం html_script *nscript;

	/* does not appear to be a src so script is inline content */
	exc = dom_కణుపు_get_పాఠ్యం_content(కణుపు, &script);
	ఒకవేళ ((exc != DOM_NO_ERR) || (script == ఖాళీ)) {
		తిరిగి DOM_HUBBUB_OK; /* no contents, skip */
	}

	nscript = html_process_new_script(c, mimetype, HTML_SCRIPT_INLINE);
	ఒకవేళ (nscript == ఖాళీ) {
		dom_string_unref(script);

		content_ప్రసారం_error(&c->పునాది, NSERROR_NOMEM, ఖాళీ);
		తిరిగి DOM_HUBBUB_NOMEM;

	}

	nscript->దత్తాంశం.string = script;
	nscript->already_started = true;

	/* ensure script handler for content type */
	exc = dom_string_intern(mimetype, &lwcmimetype);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి DOM_HUBBUB_DOM;
	}

	script_handler = select_script_handler(content_factory_type_from_mime_type(lwcmimetype));
	lwc_string_unref(lwcmimetype);

	ఒకవేళ (script_handler != ఖాళీ) {
		script_handler(c->jsthread,
			       (స్థిరరాశి uint8_t *)dom_string_దత్తాంశం(script),
			       dom_string_byte_నిడివి(script),
			       "?inline script?");
	}
	తిరిగి DOM_HUBBUB_OK;
}


/**
 * process script node parser callback
 *
 *
 */
dom_hubbub_error
html_process_script(శూన్య *ctx, dom_కణుపు *కణుపు)
{
	html_content *c = (html_content *)ctx;
	dom_exception exc; /* returned by libdom functions */
	dom_string *src, *mimetype;
	dom_hubbub_error err = DOM_HUBBUB_OK;

	/* ensure javascript context is available */
	/* We should only ever be here if scripting was enabled for this
	 * content so it's correct to make a javascript context if there
	 * isn't one already. */
	ఒకవేళ (c->jsthread == ఖాళీ) {
		సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;

		సందేశం_దత్తాంశం.jsthread = &c->jsthread;
		content_ప్రసారం(&c->పునాది, CONTENT_సందేశం_GETTHREAD, &సందేశం_దత్తాంశం);
		NSLOG(netsurf, INFO, "javascript context %p ", c->jsthread);
		ఒకవేళ (c->jsthread == ఖాళీ) {
			/* no context and it could not be created, abort */
			తిరిగి DOM_HUBBUB_OK;
		}
	}

	NSLOG(netsurf, INFO, "content %p parser %p node %p", c, c->parser,
	      కణుపు);

	exc = dom_element_get_attribute(కణుపు, corestring_dom_type, &mimetype);
	ఒకవేళ (exc != DOM_NO_ERR || mimetype == ఖాళీ) {
		mimetype = dom_string_ref(corestring_dom_పాఠ్యం_javascript);
	}

	exc = dom_element_get_attribute(కణుపు, corestring_dom_src, &src);
	ఒకవేళ (exc != DOM_NO_ERR || src == ఖాళీ) {
		err = exec_inline_script(c, కణుపు, mimetype);
	} లేకపోతే {
		err = exec_src_script(c, కణుపు, mimetype, src);
		dom_string_unref(src);
	}

	dom_string_unref(mimetype);

	తిరిగి err;
}

/* exported internal interface documented in html/html_internal.h */
bool html_saw_insecure_scripts(html_content *htmlc)
{
	నిర్మాణం html_script *s;
	గుర్తులేని పూర్ణాంకం i;

	కొరకు (i = 0, s = htmlc->scripts; i != htmlc->scripts_లెక్క; i++, s++) {
		ఒకవేళ (s->type == HTML_SCRIPT_INLINE) {
			/* Inline scripts are no less secure than their
			 * containing HTML content
			 */
			కొనసాగు;
		}
		ఒకవేళ (s->దత్తాంశం.handle == ఖాళీ) {
			/* We've not begun loading this? */
			కొనసాగు;
		}
		ఒకవేళ (content_saw_insecure_objects(s->దత్తాంశం.handle)) {
			తిరిగి true;
		}
	}

	తిరిగి false;
}

/* exported internal interface documented in html/html_internal.h */
nserror html_script_free(html_content *html)
{
	గుర్తులేని పూర్ణాంకం i;

	కొరకు (i = 0; i != html->scripts_లెక్క; i++) {
		ఒకవేళ (html->scripts[i].mimetype != ఖాళీ) {
			dom_string_unref(html->scripts[i].mimetype);
		}

		ఎంపిక (html->scripts[i].type) {
		స్థితి HTML_SCRIPT_INLINE:
			ఒకవేళ (html->scripts[i].దత్తాంశం.string != ఖాళీ) {
				dom_string_unref(html->scripts[i].దత్తాంశం.string);
			}
			ఆపు;
		స్థితి HTML_SCRIPT_SYNC:
			/* fallthrough */
		స్థితి HTML_SCRIPT_ASYNC:
			/* fallthrough */
		స్థితి HTML_SCRIPT_DEFER:
			ఒకవేళ (html->scripts[i].దత్తాంశం.handle != ఖాళీ) {
				hlcache_handle_release(html->scripts[i].దత్తాంశం.handle);
			}
			ఆపు;
		}
	}
	విడుదల(html->scripts);

	తిరిగి NSERROR_OK;
}
