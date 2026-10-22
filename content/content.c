/*
 * Copyright 2005-2007 James Bursa <bursa@users.sourceforge.net>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 * Content handling implementation.
 */

#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <nsutils/time.h>

#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "netsurf/browser_window.h"
#అన్తర్భావ "netsurf/bitmap.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "desktop/knockout.h"

#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/textsearch.h"
#అన్తర్భావ "content/content_debug.h"
#అన్తర్భావ "content/hlcache.h"
#అన్తర్భావ "content/urldb.h"

#నిర్వచించు URL_FMT_SPC "%.140s"

స్థిరరాశి అక్షరం * స్థిరరాశి content_స్థితి_name[] = {
	"LOADING",
	"READY",
	"DONE",
	"ERROR"
};


/**
 * All data has arrived, convert for display.
 *
 * Calls the convert function for the content.
 *
 * - If the conversion succeeds, but there is still some processing required
 *   (eg. loading images), the content gets status CONTENT_STATUS_READY, and a
 *   CONTENT_MSG_READY is sent to all users.
 * - If the conversion succeeds and is complete, the content gets status
 *   CONTENT_STATUS_DONE, and CONTENT_MSG_READY then CONTENT_MSG_DONE are sent.
 * - If the conversion fails, CONTENT_MSG_ERROR is sent. The content will soon
 *   be destroyed and must no longer be used.
 */
స్థిర శూన్య content_convert(నిర్మాణం content *c)
{
	ప్రతిజ్ఞా(c);
	ప్రతిజ్ఞా(c->స్థితి_2 == CONTENT_స్థితి_LOADING ||
	       c->స్థితి_2 == CONTENT_స్థితి_ERROR);

	ఒకవేళ (c->స్థితి_2 != CONTENT_స్థితి_LOADING)
		తిరిగి;

	ఒకవేళ (c->locked == true)
		తిరిగి;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	ఒకవేళ (c->handler->దత్తాంశం_complete != ఖాళీ) {
		c->locked = true;
		ఒకవేళ (c->handler->దత్తాంశం_complete(c) == false) {
			content_సమితి_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} లేకపోతే {
		content_సమితి_ready(c);
		content_సమితి_పూర్తి(c);
	}
}


/**
 * Handler for low-level cache events
 *
 * \param llcache  Low-level cache handle
 * \param event	   Event details
 * \param pw	   Pointer to our context
 * \return NSERROR_OK on success, appropriate error otherwise
 */
స్థిర nserror
content_llcache_callback(llcache_handle *llcache,
			 స్థిరరాశి llcache_event *event, శూన్య *pw)
{
	నిర్మాణం content *c = pw;
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
	nserror error = NSERROR_OK;

	ఎంపిక (event->type) {
	స్థితి LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		ఆపు;
	స్థితి LLCACHE_EVENT_HAD_శీర్షికలు:
		/* Will never happen: handled in hlcache */
		ఆపు;
	స్థితి LLCACHE_EVENT_HAD_దత్తాంశం:
		ఒకవేళ (c->handler->process_దత్తాంశం != ఖాళీ) {
			ఒకవేళ (c->handler->process_దత్తాంశం(c,
						     (స్థిరరాశి అక్షరం *) event->దత్తాంశం.దత్తాంశం.buf,
						     event->దత్తాంశం.దత్తాంశం.len) == false) {
				llcache_handle_abort(c->llcache);
				c->స్థితి_2 = CONTENT_స్థితి_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		ఆపు;
	స్థితి LLCACHE_EVENT_పూర్తి:
		{
			size_t ఆధారం_కొలత;

			(శూన్య) llcache_handle_get_ఆధారం_దత్తాంశం(llcache, &ఆధారం_కొలత);

			content_సమితి_స్థితి(c, messages_get("Processing"));
			సందేశం_దత్తాంశం.explicit_స్థితి_పాఠ్యం = ఖాళీ;
			content_ప్రసారం(c, CONTENT_సందేశం_స్థితి, &సందేశం_దత్తాంశం);

			content_convert(c);
		}
		ఆపు;
	స్థితి LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->స్థితి_2 = CONTENT_స్థితి_ERROR;
		సందేశం_దత్తాంశం.errordata.errorcode = event->దత్తాంశం.error.సంకేతం_2;
		సందేశం_దత్తాంశం.errordata.errormsg = event->దత్తాంశం.error.సందేశం;
		content_ప్రసారం(c, CONTENT_సందేశం_ERROR, &సందేశం_దత్తాంశం);
		ఆపు;
	స్థితి LLCACHE_EVENT_పురోగతి:
		content_సమితి_స్థితి(c, event->దత్తాంశం.పురోగతి_సందేశం);
		సందేశం_దత్తాంశం.explicit_స్థితి_పాఠ్యం = ఖాళీ;
		content_ప్రసారం(c, CONTENT_సందేశం_స్థితి, &సందేశం_దత్తాంశం);
		ఆపు;
	స్థితి LLCACHE_EVENT_REDIRECT:
		సందేశం_దత్తాంశం.redirect.from = event->దత్తాంశం.redirect.from;
		సందేశం_దత్తాంశం.redirect.గమ్యం = event->దత్తాంశం.redirect.గమ్యం;
		content_ప్రసారం(c, CONTENT_సందేశం_REDIRECT, &సందేశం_దత్తాంశం);
		ఆపు;
	}

	తిరిగి error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
స్థిర శూన్య content_update_స్థితి(నిర్మాణం content *c)
{
	ఒకవేళ (c->స్థితి_2 == CONTENT_స్థితి_LOADING ||
	    c->స్థితి_2 == CONTENT_స్థితి_READY) {
		/* Not done yet */
		snprintf(c->స్థితి_సందేశం, పరిమాణం (c->స్థితి_సందేశం),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_స్థితి[0] != '\0' ? ", " : " ",
			 c->sub_స్థితి);
	} లేకపోతే {
		snprintf(c->స్థితి_సందేశం, పరిమాణం (c->స్థితి_సందేశం),
			 "%s (%.1fs)", messages_get("Done"),
			 (దశాంశం) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(నిర్మాణం content *c,
	      స్థిరరాశి content_handler *handler,
	      lwc_string *imime_type,
	      స్థిరరాశి నిర్మాణం http_parameter *params,
	      llcache_handle *llcache,
	      స్థిరరాశి అక్షరం *fallback_charset,
	      bool quirks)
{
	నిర్మాణం content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం content_user));
	ఒకవేళ (user_sentinel == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	ఒకవేళ (fallback_charset != ఖాళీ) {
		c->fallback_charset = strdup(fallback_charset);
		ఒకవేళ (c->fallback_charset == ఖాళీ) {
			విడుదల(user_sentinel);
			తిరిగి NSERROR_NOMEM;
		}
	}

	c->llcache = llcache;
	c->mime_type = lwc_string_ref(imime_type);
	c->handler = handler;
	c->స్థితి_2 = CONTENT_స్థితి_LOADING;
	c->వెడల్పు = 0;
	c->ఎత్తు = 0;
	c->అందుబాటు_వెడల్పు = 0;
	c->అందుబాటు_ఎత్తు = 0;
	c->quirks = quirks;
	c->refresh = 0;
	nsu_getmonotonic_ms(&c->time);
	c->కొలత = 0;
	c->title = ఖాళీ;
	c->చురుకైన = 0;
	user_sentinel->callback = ఖాళీ;
	user_sentinel->pw = ఖాళీ;
	user_sentinel->తదుపరి = ఖాళీ;
	c->user_list = user_sentinel;
	c->sub_స్థితి[0] = 0;
	c->locked = false;
	c->మొత్తం_కొలత = 0;
	c->http_సంకేతం = 0;

	c->textsearch.string = ఖాళీ;
	c->textsearch.context = ఖాళీ;

	content_సమితి_స్థితి(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	ఒకవేళ (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		తిరిగి error;
	}

	తిరిగి NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);

	ఒకవేళ (c == ఖాళీ)
		తిరిగి false;

	తిరిగి (c->handler->reformat != ఖాళీ);
}


/* exported interface documented in content/protected.h */
శూన్య content_సమితి_స్థితి(నిర్మాణం content *c, స్థిరరాశి అక్షరం *స్థితి_సందేశం)
{
	size_t len = అక్షరశ్రేణి_పొడవు(స్థితి_సందేశం);

	ఒకవేళ (len >= పరిమాణం(c->sub_స్థితి)) {
		len = పరిమాణం(c->sub_స్థితి) - 1;
	}
	జ్ఞాపకం_నకలు(c->sub_స్థితి, స్థితి_సందేశం, len);
	c->sub_స్థితి[len] = '\0';

	content_update_స్థితి(c);
}


/* exported interface documented in content/protected.h */
శూన్య content_సమితి_ready(నిర్మాణం content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	ప్రతిజ్ఞా(c->locked);
	c->locked = false;

	c->స్థితి_2 = CONTENT_స్థితి_READY;
	content_update_స్థితి(c);
	content_ప్రసారం(c, CONTENT_సందేశం_READY, ఖాళీ);
}


/* exported interface documented in content/protected.h */
శూన్య content_సమితి_పూర్తి(నిర్మాణం content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->స్థితి_2 = CONTENT_స్థితి_పూర్తి;
	c->time = now_ms - c->time;
	content_update_స్థితి(c);
	content_ప్రసారం(c, CONTENT_సందేశం_పూర్తి, ఖాళీ);
}


/* exported interface documented in content/protected.h */
శూన్య content_సమితి_error(నిర్మాణం content *c)
{
	c->locked = false;
	c->స్థితి_2 = CONTENT_స్థితి_ERROR;
}


/* exported interface documented in content/content.h */
శూన్య content_reformat(hlcache_handle *h, bool background,
		      పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  వెడల్పు, ఎత్తు);
}


/* exported interface documented in content/protected.h */
శూన్య
content__reformat(నిర్మాణం content *c, bool background, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు)
{
	సంఘం content_సందేశం_దత్తాంశం దత్తాంశం;
	ప్రతిజ్ఞా(c != 0);
	ప్రతిజ్ఞా(c->స్థితి_2 == CONTENT_స్థితి_READY ||
	       c->స్థితి_2 == CONTENT_స్థితి_పూర్తి);
	ప్రతిజ్ఞా(c->locked == false);

	c->అందుబాటు_వెడల్పు = వెడల్పు;
	c->అందుబాటు_ఎత్తు = ఎత్తు;
	ఒకవేళ (c->handler->reformat != ఖాళీ) {

		c->locked = true;
		c->handler->reformat(c, వెడల్పు, ఎత్తు);
		c->locked = false;

		దత్తాంశం.background = background;
		content_ప్రసారం(c, CONTENT_సందేశం_REFORMAT, &దత్తాంశం);
	}
}


/* exported interface documented in content/content.h */
శూన్య content_destroy(నిర్మాణం content *c)
{
	నిర్మాణం content_rfc5988_link *link;

	ప్రతిజ్ఞా(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	ప్రతిజ్ఞా(c->locked == false);

	ఒకవేళ (c->handler->destroy != ఖాళీ)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = ఖాళీ;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	అయితే (link != ఖాళీ) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	ఒకవేళ (c->user_list != ఖాళీ) {
		విడుదల(c->user_list);
	}

	/* free the title */
	ఒకవేళ (c->title != ఖాళీ) {
		విడుదల(c->title);
	}

	/* free the fallback characterset */
	ఒకవేళ (c->fallback_charset != ఖాళీ) {
		విడుదల(c->fallback_charset);
	}

	విడుదల(c);
}


/* exported interface documented in content/content.h */
శూన్య
content_mouse_track(hlcache_handle *h,
		    నిర్మాణం browser_కిటికీ *bw,
		    browser_mouse_state mouse,
		    పూర్ణాంకం x, పూర్ణాంకం y)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	ప్రతిజ్ఞా(c != ఖాళీ);

	ఒకవేళ (c->handler->mouse_track != ఖాళీ) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} లేకపోతే {
		సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
		సందేశం_దత్తాంశం.సూచిక = BROWSER_సూచిక_AUTO;
		content_ప్రసారం(c, CONTENT_సందేశం_సూచిక, &సందేశం_దత్తాంశం);
	}


	తిరిగి;
}


/* exported interface documented in content/content.h */
శూన్య
content_mouse_action(hlcache_handle *h,
		     నిర్మాణం browser_కిటికీ *bw,
		     browser_mouse_state mouse,
		     పూర్ణాంకం x, పూర్ణాంకం y)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	ప్రతిజ్ఞా(c != ఖాళీ);

	ఒకవేళ (c->handler->mouse_action != ఖాళీ)
		c->handler->mouse_action(c, bw, mouse, x, y);

	తిరిగి;
}


/* exported interface documented in content/content.h */
bool content_keypress(నిర్మాణం hlcache_handle *h, uint32_t కీలకం)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	ప్రతిజ్ఞా(c != ఖాళీ);

	ఒకవేళ (c->handler->keypress != ఖాళీ)
		తిరిగి c->handler->keypress(c, కీలకం);

	తిరిగి false;
}


/* exported interface documented in content/content.h */
శూన్య content_request_redraw(నిర్మాణం hlcache_handle *h,
			    పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, వెడల్పు, ఎత్తు);
}


/* exported interface, documented in content/protected.h */
శూన్య content__request_redraw(నిర్మాణం content *c,
			     పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు)
{
	సంఘం content_సందేశం_దత్తాంశం దత్తాంశం;

	ఒకవేళ (c == ఖాళీ)
		తిరిగి;

	దత్తాంశం.redraw.x = x;
	దత్తాంశం.redraw.y = y;
	దత్తాంశం.redraw.వెడల్పు = వెడల్పు;
	దత్తాంశం.redraw.ఎత్తు = ఎత్తు;

	content_ప్రసారం(c, CONTENT_సందేశం_REDRAW, &దత్తాంశం);
}


/* exported interface, documented in content/content.h */
bool content_exec(నిర్మాణం hlcache_handle *h, స్థిరరాశి అక్షరం *src, size_t srclen)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);

	ప్రతిజ్ఞా(c != ఖాళీ);

	ఒకవేళ (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		తిరిగి false;
	}

	ఒకవేళ (c->handler->exec == ఖాళీ) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		తిరిగి false;
	}

	తిరిగి c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(నిర్మాణం hlcache_handle *h)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	నిర్మాణం nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool సరిపోలిక;

	/* Is this an internal scheme? If so, we trust here and stop */
	ఒకవేళ ((lwc_string_isequal(scheme, corestring_lwc_about,
				&సరిపోలిక) == lwc_error_ok &&
	     (సరిపోలిక == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_దత్తాంశం,
				&సరిపోలిక) == lwc_error_ok &&
	     (సరిపోలిక == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&సరిపోలిక) == lwc_error_ok &&
	     (సరిపోలిక == true)) ||
	    /* Our internal x-ns-css scheme is secure */
	    (lwc_string_isequal(scheme, corestring_lwc_x_ns_css,
				&సరిపోలిక) == lwc_error_ok &&
	     (సరిపోలిక == true)) ||
	    /* We also treat file: as "not insecure" here */
	    (lwc_string_isequal(scheme, corestring_lwc_file,
				&సరిపోలిక) == lwc_error_ok &&
	     (సరిపోలిక == true))) {
		/* No insecurity to find */
		lwc_string_unref(scheme);
		తిరిగి false;
	}

	/* Okay, not internal, am *I* secure? */
	ఒకవేళ ((lwc_string_isequal(scheme, corestring_lwc_https,
				&సరిపోలిక) == lwc_error_ok)
	    && (సరిపోలిక == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		తిరిగి true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	ఒకవేళ (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		తిరిగి true;
	}

	/* Otherwise try and chain through the handler */
	ఒకవేళ (c != ఖాళీ && c->handler->saw_insecure_objects != ఖాళీ) {
		తిరిగి c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	తిరిగి false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       నిర్మాణం content_redraw_దత్తాంశం *దత్తాంశం,
	       స్థిరరాశి నిర్మాణం rect *clip,
	       స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);

	ప్రతిజ్ఞా(c != ఖాళీ);

	ఒకవేళ (c->locked) {
		/* not safe to attempt redraw */
		తిరిగి true;
	}

	/* ensure we have a redrawable content */
	ఒకవేళ (c->handler->redraw == ఖాళీ) {
		తిరిగి true;
	}

	తిరిగి c->handler->redraw(c, దత్తాంశం, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(నిర్మాణం hlcache_handle *h,
		      పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు,
		      స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	నిర్మాణం redraw_context new_ctx = *ctx;
	నిర్మాణం rect clip;
	నిర్మాణం content_redraw_దత్తాంశం దత్తాంశం;
	bool plot_ok = true;

	ప్రతిజ్ఞా(c != ఖాళీ);

	/* ensure it is safe to attempt redraw */
	ఒకవేళ (c->locked) {
		తిరిగి true;
	}

	/* ensure we have a redrawable content */
	ఒకవేళ (c->handler->redraw == ఖాళీ) {
		తిరిగి true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, వెడల్పు, ఎత్తు, ctx);

	ఒకవేళ (ctx->plot->option_knockout) {
		knockout_plot_ప్రారంభం(ctx, &new_ctx);
	}

	/* Set clip rectangle to required thumbnail size */
	clip.x0 = 0;
	clip.y0 = 0;
	clip.x1 = వెడల్పు;
	clip.y1 = ఎత్తు;

	new_ctx.plot->clip(&new_ctx, &clip);

	/* Plot white background */
	plot_ok &= (new_ctx.plot->rectangle(&new_ctx,
					    plot_style_fill_white,
					    &clip) == NSERROR_OK);

	/* Set up content redraw data */
	దత్తాంశం.x = 0;
	దత్తాంశం.y = 0;
	దత్తాంశం.వెడల్పు = వెడల్పు;
	దత్తాంశం.ఎత్తు = ఎత్తు;

	దత్తాంశం.background_రంగు = 0xFFFFFF;
	దత్తాంశం.repeat_x = false;
	దత్తాంశం.repeat_y = false;

	/* Find the scale factor to use if the content has a width */
	ఒకవేళ (c->వెడల్పు) {
		దత్తాంశం.scale = (దశాంశం)వెడల్పు / (దశాంశం)c->వెడల్పు;
	} లేకపోతే {
		దత్తాంశం.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &దత్తాంశం, &clip, &new_ctx);

	ఒకవేళ (ctx->plot->option_knockout) {
		knockout_plot_చివర(ctx);
	}

	తిరిగి plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_కూడు_user(నిర్మాణం content *c,
		 శూన్య (*callback)(
				  నిర్మాణం content *c,
				  content_సందేశం సందేశం,
				  స్థిరరాశి సంఘం content_సందేశం_దత్తాంశం *దత్తాంశం,
				  శూన్య *pw),
		 శూన్య *pw)
{
	నిర్మాణం content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం content_user));
	ఒకవేళ (!user)
		తిరిగి false;
	user->callback = callback;
	user->pw = pw;
	user->తదుపరి = c->user_list->తదుపరి;
	c->user_list->తదుపరి = user;

	ఒకవేళ (c->handler->కూడు_user != ఖాళీ)
		c->handler->కూడు_user(c);

	తిరిగి true;
}


/* exported interface documented in content/content.h */
శూన్య
content_remove_user(నిర్మాణం content *c,
		    శూన్య (*callback)(
				     నిర్మాణం content *c,
				     content_సందేశం సందేశం,
				     స్థిరరాశి సంఘం content_సందేశం_దత్తాంశం *దత్తాంశం,
				     శూన్య *pw),
		    శూన్య *pw)
{
	నిర్మాణం content_user *user, *తదుపరి;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	కొరకు (user = c->user_list; user->తదుపరి != 0 &&
		     !(user->తదుపరి->callback == callback &&
		       user->తదుపరి->pw == pw); user = user->తదుపరి)
		;
	ఒకవేళ (user->తదుపరి == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		ప్రతిజ్ఞా(0);
		తిరిగి;
	}

	ఒకవేళ (c->handler->remove_user != ఖాళీ)
		c->handler->remove_user(c);

	తదుపరి = user->తదుపరి;
	user->తదుపరి = తదుపరి->తదుపరి;
	విడుదల(తదుపరి);
}


/* exported interface documented in content/content.h */
uint32_t content_లెక్క_users(నిర్మాణం content *c)
{
	నిర్మాణం content_user *user;
	uint32_t counter = 0;

	ప్రతిజ్ఞా(c != ఖాళీ);

	కొరకు (user = c->user_list; user != ఖాళీ; user = user->తదుపరి)
		counter += 1;

	ప్రతిజ్ఞా(counter > 0);

	తిరిగి counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(నిర్మాణం content *c, bool quirks)
{
	ఒకవేళ (c->handler->matches_quirks == ఖాళీ)
		తిరిగి true;

	తిరిగి c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(నిర్మాణం content *c)
{
	తిరిగి c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
శూన్య content_ప్రసారం(నిర్మాణం content *c, content_సందేశం సందేశం,
		       స్థిరరాశి సంఘం content_సందేశం_దత్తాంశం *దత్తాంశం)
{
	నిర్మాణం content_user *user, *తదుపరి;
	ప్రతిజ్ఞా(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, సందేశం);
	కొరకు (user = c->user_list->తదుపరి; user != 0; user = తదుపరి) {
		తదుపరి = user->తదుపరి;  /* user may be destroyed during callback */
		ఒకవేళ (user->callback != 0)
			user->callback(c, సందేశం, దత్తాంశం, user->pw);
	}
}


/* exported interface documented in content_protected.h */
శూన్య
content_ప్రసారం_error(నిర్మాణం content *c, nserror errorcode, స్థిరరాశి అక్షరం *సందేశం)
{
	నిర్మాణం content_user *user, *తదుపరి;
	సంఘం content_సందేశం_దత్తాంశం దత్తాంశం;

	ప్రతిజ్ఞా(c);

	దత్తాంశం.errordata.errorcode = errorcode;
	దత్తాంశం.errordata.errormsg = సందేశం;

	కొరకు (user = c->user_list->తదుపరి; user != 0; user = తదుపరి) {
		తదుపరి = user->తదుపరి;  /* user may be destroyed during callback */
		ఒకవేళ (user->callback != 0) {
			user->callback(c, CONTENT_సందేశం_ERROR,
				       &దత్తాంశం, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     నిర్మాణం browser_కిటికీ *bw,
	     నిర్మాణం content *పుట,
	     నిర్మాణం object_params *params)
{
	నిర్మాణం content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	ప్రతిజ్ఞా(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	ఒకవేళ (c->handler->open != ఖాళీ) {
		res = c->handler->open(c, bw, పుట, params);
	} లేకపోతే {
		res = NSERROR_OK;
	}
	తిరిగి res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	నిర్మాణం content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	ఒకవేళ (c == ఖాళీ) {
		తిరిగి NSERROR_BAD_PARAMETER;
	}

	ఒకవేళ ((c->స్థితి_2 != CONTENT_స్థితి_READY) &&
	    (c->స్థితి_2 != CONTENT_స్థితి_పూర్తి)) {
		/* status is not read or done so nothing to do */
		తిరిగి NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	ఒకవేళ (c->textsearch.context != ఖాళీ) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ఖాళీ;
	}

	ఒకవేళ (c->handler->close != ఖాళీ) {
		res = c->handler->close(c);
	} లేకపోతే {
		res = NSERROR_OK;
	}
	తిరిగి res;
}


/* exported interface, documented in content/content.h */
శూన్య content_clear_selection(hlcache_handle *h)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	ప్రతిజ్ఞా(c != 0);

	ఒకవేళ (c->handler->get_selection != ఖాళీ)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
అక్షరం * content_get_selection(hlcache_handle *h)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	ప్రతిజ్ఞా(c != 0);

	ఒకవేళ (c->handler->get_selection != ఖాళీ)
		తిరిగి c->handler->get_selection(c);
	లేకపోతే
		తిరిగి ఖాళీ;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(నిర్మాణం hlcache_handle *h,
			       పూర్ణాంకం x, పూర్ణాంకం y,
			       నిర్మాణం browser_కిటికీ_features *దత్తాంశం)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	ప్రతిజ్ఞా(c != 0);

	ఒకవేళ (c->handler->get_contextual_content != ఖాళీ) {
		తిరిగి c->handler->get_contextual_content(c, x, y, దత్తాంశం);
	}

	దత్తాంశం->object = h;
	తిరిగి NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_బిందువు(నిర్మాణం hlcache_handle *h,
			పూర్ణాంకం x, పూర్ణాంకం y,
			పూర్ణాంకం scrx, పూర్ణాంకం scry)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	ప్రతిజ్ఞా(c != 0);

	ఒకవేళ (c->handler->scroll_at_బిందువు != ఖాళీ)
		తిరిగి c->handler->scroll_at_బిందువు(c, x, y, scrx, scry);

	తిరిగి false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_బిందువు(నిర్మాణం hlcache_handle *h,
			   పూర్ణాంకం x, పూర్ణాంకం y,
			   అక్షరం *file)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	ప్రతిజ్ఞా(c != 0);

	ఒకవేళ (c->handler->drop_file_at_బిందువు != ఖాళీ)
		తిరిగి c->handler->drop_file_at_బిందువు(c, x, y, file);

	తిరిగి false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(నిర్మాణం hlcache_handle *h, FILE *f, గణన content_debug op)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	ప్రతిజ్ఞా(c != 0);

	ఒకవేళ (c->handler->debug_dump == ఖాళీ) {
		తిరిగి NSERROR_NOT_IMPLEMENTED;
	}

	తిరిగి c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(నిర్మాణం hlcache_handle *h, గణన content_debug op)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);

	ఒకవేళ (c == ఖాళీ) {
		తిరిగి NSERROR_BAD_PARAMETER;
	}

	ఒకవేళ (c->handler->debug == ఖాళీ) {
		తిరిగి NSERROR_NOT_IMPLEMENTED;
	}

	తిరిగి c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
నిర్మాణం content_rfc5988_link *
content_కనుగొను_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);
	నిర్మాణం content_rfc5988_link *link = c->links;
	bool rel_సరిపోలిక = false;

	అయితే (link != ఖాళీ) {
		ఒకవేళ (lwc_string_caseless_isequal(link->rel, rel,
						&rel_సరిపోలిక) == lwc_error_ok && rel_సరిపోలిక) {
			ఆపు;
		}
		link = link->తదుపరి;
	}
	తిరిగి link;
}


/* exported interface documented in content/protected.h */
నిర్మాణం content_rfc5988_link *
content__free_rfc5988_link(నిర్మాణం content_rfc5988_link *link)
{
	నిర్మాణం content_rfc5988_link *తదుపరి;

	తదుపరి = link->తదుపరి;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	విడుదల(link);

	తిరిగి తదుపరి;
}


/* exported interface documented in content/protected.h */
bool
content_కూడు_rfc5988_link(నిర్మాణం content *c,
			  స్థిరరాశి నిర్మాణం content_rfc5988_link *link)
{
	నిర్మాణం content_rfc5988_link *newlink;
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;

	/* a link relation must be present for it to be a link */
	ఒకవేళ (link->rel == ఖాళీ) {
		తిరిగి false;
	}

	/* a link href must be present for it to be a link */
	ఒకవేళ (link->href == ఖాళీ) {
		తిరిగి false;
	}

	newlink = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం content_rfc5988_link));
	ఒకవేళ (newlink == ఖాళీ) {
		తిరిగి false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	ఒకవేళ (link->hreflang != ఖాళీ) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	ఒకవేళ (link->type != ఖాళీ) {
		newlink->type = lwc_string_ref(link->type);
	}
	ఒకవేళ (link->media != ఖాళీ) {
		newlink->media = lwc_string_ref(link->media);
	}
	ఒకవేళ (link->sizes != ఖాళీ) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->తదుపరి = c->links;
	c->links = newlink;

	/* broadcast the data */
	సందేశం_దత్తాంశం.rfc5988_link = newlink;
	content_ప్రసారం(c, CONTENT_సందేశం_LINK, &సందేశం_దత్తాంశం);

	తిరిగి true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(నిర్మాణం content *c)
{
	ఒకవేళ (c == ఖాళీ)
		తిరిగి ఖాళీ;

	తిరిగి llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);

	ఒకవేళ (c == ఖాళీ)
		తిరిగి CONTENT_NONE;

	తిరిగి c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	తిరిగి content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(నిర్మాణం content *c)
{
	ఒకవేళ (c == ఖాళీ)
		తిరిగి ఖాళీ;

	తిరిగి lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content_సమితి_title(నిర్మాణం content *c, స్థిరరాశి అక్షరం *title)
{
	అక్షరం *new_title = strdup(title);
	ఒకవేళ (new_title == ఖాళీ)
		తిరిగి false;

	ఒకవేళ (c->title != ఖాళీ)
		విడుదల(c->title);

	c->title = new_title;

	తిరిగి true;
}


/* exported interface documented in content/content.h */
స్థిరరాశి అక్షరం *content_get_title(hlcache_handle *h)
{
	తిరిగి content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
స్థిరరాశి అక్షరం *content__get_title(నిర్మాణం content *c)
{
	ఒకవేళ (c == ఖాళీ)
		తిరిగి ఖాళీ;

	తిరిగి c->title != ఖాళీ ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_స్థితి content_get_స్థితి(hlcache_handle *h)
{
	తిరిగి content_get_స్థితి_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_స్థితి content_get_స్థితి_2(నిర్మాణం content *c)
{
	ఒకవేళ (c == ఖాళీ)
		తిరిగి CONTENT_స్థితి_ERROR;

	తిరిగి c->స్థితి_2;
}


/* exported interface documented in content/content.h */
స్థిరరాశి అక్షరం *content_get_స్థితి_సందేశం(hlcache_handle *h)
{
	తిరిగి content_get_స్థితి_సందేశం_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
స్థిరరాశి అక్షరం *content_get_స్థితి_సందేశం_2(నిర్మాణం content *c)
{
	ఒకవేళ (c == ఖాళీ)
		తిరిగి ఖాళీ;

	తిరిగి c->స్థితి_సందేశం;
}


/* exported interface documented in content/content.h */
పూర్ణాంకం content_get_వెడల్పు(hlcache_handle *h)
{
	తిరిగి content_get_వెడల్పు_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
పూర్ణాంకం content_get_వెడల్పు_2(నిర్మాణం content *c)
{
	ఒకవేళ (c == ఖాళీ)
		తిరిగి 0;

	తిరిగి c->వెడల్పు;
}


/* exported interface documented in content/content.h */
పూర్ణాంకం content_get_ఎత్తు(hlcache_handle *h)
{
	తిరిగి content_get_ఎత్తు_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
పూర్ణాంకం content_get_ఎత్తు_2(నిర్మాణం content *c)
{
	ఒకవేళ (c == ఖాళీ)
		తిరిగి 0;

	తిరిగి c->ఎత్తు;
}


/* exported interface documented in content/content.h */
పూర్ణాంకం content_get_అందుబాటు_వెడల్పు(hlcache_handle *h)
{
	తిరిగి content_get_అందుబాటు_వెడల్పు_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
పూర్ణాంకం content_get_అందుబాటు_వెడల్పు_2(నిర్మాణం content *c)
{
	ఒకవేళ (c == ఖాళీ)
		తిరిగి 0;

	తిరిగి c->అందుబాటు_వెడల్పు;
}


/* exported interface documented in content/content.h */
స్థిరరాశి uint8_t *content_get_ఆధారం_దత్తాంశం(hlcache_handle *h, size_t *కొలత)
{
	తిరిగి content_get_ఆధారం_దత్తాంశం_2(hlcache_handle_get_content(h), కొలత);
}


/* exported interface documented in content/content_protected.h */
స్థిరరాశి uint8_t *content_get_ఆధారం_దత్తాంశం_2(నిర్మాణం content *c, size_t *కొలత)
{
	ప్రతిజ్ఞా(కొలత != ఖాళీ);

	/** \todo check if the content check should be an assert */
	ఒకవేళ (c == ఖాళీ)
		తిరిగి ఖాళీ;

	తిరిగి llcache_handle_get_ఆధారం_దత్తాంశం(c->llcache, కొలత);
}


/* exported interface documented in content/content.h */
శూన్య content_invalidate_reuse_దత్తాంశం(hlcache_handle *h)
{
	content_invalidate_reuse_దత్తాంశం_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
శూన్య content_invalidate_reuse_దత్తాంశం_2(నిర్మాణం content *c)
{
	ఒకవేళ (c == ఖాళీ || c->llcache == ఖాళీ)
		తిరిగి;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_దత్తాంశం(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	తిరిగి content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(నిర్మాణం content *c)
{
	ఒకవేళ (c == ఖాళీ)
		తిరిగి ఖాళీ;

	తిరిగి c->refresh;
}


/* exported interface documented in content/content.h */
నిర్మాణం bitmap *content_get_bitmap(hlcache_handle *h)
{
	తిరిగి content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
నిర్మాణం bitmap *content__get_bitmap(నిర్మాణం content *c)
{
	నిర్మాణం bitmap *bitmap = ఖాళీ;

	ఒకవేళ ((c != ఖాళీ) &&
	    (c->handler != ఖాళీ) &&
	    (c->handler->type != ఖాళీ) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_అంతర్గత != ఖాళీ) ) {
		bitmap = c->handler->get_అంతర్గత(c, ఖాళీ);
	}

	తిరిగి bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	తిరిగి content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(నిర్మాణం content *c)
{
	ఒకవేళ ((c != ఖాళీ) &&
	    (c->handler != ఖాళీ) &&
	    (c->handler->is_opaque != ఖాళీ)) {
		తిరిగి c->handler->is_opaque(c);
	}

	తిరిగి false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	నిర్మాణం content *c = hlcache_handle_get_content(h);

	ఒకవేళ (c == ఖాళీ)
		తిరిగి false;

	తిరిగి c->quirks;
}


/* exported interface documented in content/content.h */
స్థిరరాశి అక్షరం *
content_get_encoding(hlcache_handle *h, గణన content_encoding_type op)
{
	తిరిగి content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
స్థిరరాశి అక్షరం *
content__get_encoding(నిర్మాణం content *c, గణన content_encoding_type op)
{
	స్థిరరాశి అక్షరం *encoding = ఖాళీ;

	ఒకవేళ ((c != ఖాళీ) &&
	    (c->handler != ఖాళీ) &&
	    (c->handler->get_encoding != ఖాళీ) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	తిరిగి encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	తిరిగి content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(నిర్మాణం content *c)
{
	తిరిగి c->locked;
}


/* exported interface documented in content/content.h */
స్థిరరాశి llcache_handle *content_get_llcache_handle(నిర్మాణం content *c)
{
	ఒకవేళ (c == ఖాళీ)
		తిరిగి ఖాళీ;

	తిరిగి c->llcache;
}


/* exported interface documented in content/protected.h */
నిర్మాణం content *content_clone(నిర్మాణం content *c)
{
	నిర్మాణం content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి ఖాళీ;

	తిరిగి nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(స్థిరరాశి నిర్మాణం content *c, నిర్మాణం content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	ఒకవేళ (error != NSERROR_OK) {
		తిరిగి error;
	}

	llcache_handle_change_callback(nc->llcache,
				       content_llcache_callback, nc);

	nc->mime_type = lwc_string_ref(c->mime_type);
	nc->handler = c->handler;

	nc->స్థితి_2 = c->స్థితి_2;

	nc->వెడల్పు = c->వెడల్పు;
	nc->ఎత్తు = c->ఎత్తు;
	nc->అందుబాటు_వెడల్పు = c->అందుబాటు_వెడల్పు;
	nc->quirks = c->quirks;

	ఒకవేళ (c->fallback_charset != ఖాళీ) {
		nc->fallback_charset = strdup(c->fallback_charset);
		ఒకవేళ (nc->fallback_charset == ఖాళీ) {
			తిరిగి NSERROR_NOMEM;
		}
	}

	ఒకవేళ (c->refresh != ఖాళీ) {
		nc->refresh = nsurl_ref(c->refresh);
		ఒకవేళ (nc->refresh == ఖాళీ) {
			తిరిగి NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_సమయం = c->reformat_సమయం;
	nc->కొలత = c->కొలత;

	ఒకవేళ (c->title != ఖాళీ) {
		nc->title = strdup(c->title);
		ఒకవేళ (nc->title == ఖాళీ) {
			తిరిగి NSERROR_NOMEM;
		}
	}

	nc->చురుకైన = c->చురుకైన;

	nc->user_list = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం content_user));
	ఒకవేళ (nc->user_list == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	జ్ఞాపకం_నకలు(&(nc->స్థితి_సందేశం), &(c->స్థితి_సందేశం), 120);
	జ్ఞాపకం_నకలు(&(nc->sub_స్థితి), &(c->sub_స్థితి), 80);

	nc->locked = c->locked;
	nc->మొత్తం_కొలత = c->మొత్తం_కొలత;
	nc->http_సంకేతం = c->http_సంకేతం;

	తిరిగి NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(నిర్మాణం content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	ఒకవేళ (c->handler->stop != ఖాళీ)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	తిరిగి llcache_handle_abort(c->llcache);
}
