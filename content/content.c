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

#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <nsutils/time.h>

#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "netsurf/browser_window.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "desktop/knockout.h"

#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/textsearch.h"
#অন্তর্ভাব "content/content_debug.h"
#অন্তর্ভাব "content/hlcache.h"
#অন্তর্ভাব "content/urldb.h"

#define URL_FMT_SPC "%.140s"

ধ্রুবক অক্ষর * ধ্রুবক content_status_name[] = {
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
স্থির শূন্য content_convert(কাঠামো content *c)
{
	প্রতিজ্ঞা(c);
	প্রতিজ্ঞা(c->status == CONTENT_STATUS_LOADING ||
	       c->status == CONTENT_STATUS_ERROR);

	যদি (c->status != CONTENT_STATUS_LOADING)
		ফেরত;

	যদি (c->locked == true)
		ফেরত;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	যদি (c->handler->তথ্য_complete != ফাঁকা) {
		c->locked = true;
		যদি (c->handler->তথ্য_complete(c) == false) {
			content_set_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} অন্যথা {
		content_set_ready(c);
		content_set_done(c);
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
স্থির nserror
content_llcache_callback(llcache_handle *llcache,
			 ধ্রুবক llcache_event *event, শূন্য *pw)
{
	কাঠামো content *c = pw;
	সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
	nserror error = NSERROR_OK;

	নির্বাচন (event->type) {
	ক্ষেত্র LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		বিরতি;
	ক্ষেত্র LLCACHE_EVENT_HAD_HEADERS:
		/* Will never happen: handled in hlcache */
		বিরতি;
	ক্ষেত্র LLCACHE_EVENT_HAD_তথ্য:
		যদি (c->handler->process_তথ্য != ফাঁকা) {
			যদি (c->handler->process_তথ্য(c,
						     (ধ্রুবক অক্ষর *) event->তথ্য.তথ্য.buf,
						     event->তথ্য.তথ্য.len) == false) {
				llcache_handle_abort(c->llcache);
				c->status = CONTENT_STATUS_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		বিরতি;
	ক্ষেত্র LLCACHE_EVENT_DONE:
		{
			size_t source_আকার;

			(শূন্য) llcache_handle_get_source_তথ্য(llcache, &source_আকার);

			content_set_status(c, messages_get("Processing"));
			বার্তা_তথ্য.explicit_status_text = ফাঁকা;
			content_broadcast(c, CONTENT_বার্তা_STATUS, &বার্তা_তথ্য);

			content_convert(c);
		}
		বিরতি;
	ক্ষেত্র LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->status = CONTENT_STATUS_ERROR;
		বার্তা_তথ্য.errordata.errorcode = event->তথ্য.error.code;
		বার্তা_তথ্য.errordata.errormsg = event->তথ্য.error.বার্তা;
		content_broadcast(c, CONTENT_বার্তা_ERROR, &বার্তা_তথ্য);
		বিরতি;
	ক্ষেত্র LLCACHE_EVENT_PROGRESS:
		content_set_status(c, event->তথ্য.progress_বার্তা);
		বার্তা_তথ্য.explicit_status_text = ফাঁকা;
		content_broadcast(c, CONTENT_বার্তা_STATUS, &বার্তা_তথ্য);
		বিরতি;
	ক্ষেত্র LLCACHE_EVENT_REDIRECT:
		বার্তা_তথ্য.redirect.from = event->তথ্য.redirect.from;
		বার্তা_তথ্য.redirect.to = event->তথ্য.redirect.to;
		content_broadcast(c, CONTENT_বার্তা_REDIRECT, &বার্তা_তথ্য);
		বিরতি;
	}

	ফেরত error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
স্থির শূন্য content_update_status(কাঠামো content *c)
{
	যদি (c->status == CONTENT_STATUS_LOADING ||
	    c->status == CONTENT_STATUS_READY) {
		/* Not done yet */
		snprintf(c->status_message, আকার (c->status_message),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_status[0] != '\0' ? ", " : " ",
			 c->sub_status);
	} অন্যথা {
		snprintf(c->status_message, আকার (c->status_message),
			 "%s (%.1fs)", messages_get("Done"),
			 (দশমিক) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(কাঠামো content *c,
	      ধ্রুবক content_handler *handler,
	      lwc_string *imime_type,
	      ধ্রুবক কাঠামো http_parameter *params,
	      llcache_handle *llcache,
	      ধ্রুবক অক্ষর *fallback_charset,
	      bool quirks)
{
	কাঠামো content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = calloc(1, আকার(কাঠামো content_user));
	যদি (user_sentinel == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	যদি (fallback_charset != ফাঁকা) {
		c->fallback_charset = strdup(fallback_charset);
		যদি (c->fallback_charset == ফাঁকা) {
			মুক্ত(user_sentinel);
			ফেরত NSERROR_NOMEM;
		}
	}

	c->llcache = llcache;
	c->mime_type = lwc_string_ref(imime_type);
	c->handler = handler;
	c->status = CONTENT_STATUS_LOADING;
	c->width = 0;
	c->height = 0;
	c->available_width = 0;
	c->available_height = 0;
	c->quirks = quirks;
	c->refresh = 0;
	nsu_getmonotonic_ms(&c->time);
	c->আকার_2 = 0;
	c->title = ফাঁকা;
	c->active = 0;
	user_sentinel->callback = ফাঁকা;
	user_sentinel->pw = ফাঁকা;
	user_sentinel->পরবর্তী = ফাঁকা;
	c->user_list = user_sentinel;
	c->sub_status[0] = 0;
	c->locked = false;
	c->যোগ_আকার = 0;
	c->http_code = 0;

	c->textsearch.string = ফাঁকা;
	c->textsearch.context = ফাঁকা;

	content_set_status(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	যদি (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		ফেরত error;
	}

	ফেরত NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	কাঠামো content *c = hlcache_handle_get_content(h);

	যদি (c == ফাঁকা)
		ফেরত false;

	ফেরত (c->handler->reformat != ফাঁকা);
}


/* exported interface documented in content/protected.h */
শূন্য content_set_status(কাঠামো content *c, ধ্রুবক অক্ষর *status_message)
{
	size_t len = strlen(status_message);

	যদি (len >= আকার(c->sub_status)) {
		len = আকার(c->sub_status) - 1;
	}
	memcpy(c->sub_status, status_message, len);
	c->sub_status[len] = '\0';

	content_update_status(c);
}


/* exported interface documented in content/protected.h */
শূন্য content_set_ready(কাঠামো content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	প্রতিজ্ঞা(c->locked);
	c->locked = false;

	c->status = CONTENT_STATUS_READY;
	content_update_status(c);
	content_broadcast(c, CONTENT_বার্তা_READY, ফাঁকা);
}


/* exported interface documented in content/protected.h */
শূন্য content_set_done(কাঠামো content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->status = CONTENT_STATUS_DONE;
	c->time = now_ms - c->time;
	content_update_status(c);
	content_broadcast(c, CONTENT_বার্তা_DONE, ফাঁকা);
}


/* exported interface documented in content/protected.h */
শূন্য content_set_error(কাঠামো content *c)
{
	c->locked = false;
	c->status = CONTENT_STATUS_ERROR;
}


/* exported interface documented in content/content.h */
শূন্য content_reformat(hlcache_handle *h, bool background,
		      পূর্ণসংখ্যা width, পূর্ণসংখ্যা height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  width, height);
}


/* exported interface documented in content/protected.h */
শূন্য
content__reformat(কাঠামো content *c, bool background, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height)
{
	সংঘ content_বার্তা_তথ্য তথ্য;
	প্রতিজ্ঞা(c != 0);
	প্রতিজ্ঞা(c->status == CONTENT_STATUS_READY ||
	       c->status == CONTENT_STATUS_DONE);
	প্রতিজ্ঞা(c->locked == false);

	c->available_width = width;
	c->available_height = height;
	যদি (c->handler->reformat != ফাঁকা) {

		c->locked = true;
		c->handler->reformat(c, width, height);
		c->locked = false;

		তথ্য.background = background;
		content_broadcast(c, CONTENT_বার্তা_REFORMAT, &তথ্য);
	}
}


/* exported interface documented in content/content.h */
শূন্য content_destroy(কাঠামো content *c)
{
	কাঠামো content_rfc5988_link *link;

	প্রতিজ্ঞা(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	প্রতিজ্ঞা(c->locked == false);

	যদি (c->handler->destroy != ফাঁকা)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = ফাঁকা;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	যখন (link != ফাঁকা) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	যদি (c->user_list != ফাঁকা) {
		মুক্ত(c->user_list);
	}

	/* free the title */
	যদি (c->title != ফাঁকা) {
		মুক্ত(c->title);
	}

	/* free the fallback characterset */
	যদি (c->fallback_charset != ফাঁকা) {
		মুক্ত(c->fallback_charset);
	}

	মুক্ত(c);
}


/* exported interface documented in content/content.h */
শূন্য
content_mouse_track(hlcache_handle *h,
		    কাঠামো browser_window *bw,
		    browser_mouse_state mouse,
		    পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != ফাঁকা);

	যদি (c->handler->mouse_track != ফাঁকা) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} অন্যথা {
		সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;
		বার্তা_তথ্য.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(c, CONTENT_বার্তা_POINTER, &বার্তা_তথ্য);
	}


	ফেরত;
}


/* exported interface documented in content/content.h */
শূন্য
content_mouse_action(hlcache_handle *h,
		     কাঠামো browser_window *bw,
		     browser_mouse_state mouse,
		     পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != ফাঁকা);

	যদি (c->handler->mouse_action != ফাঁকা)
		c->handler->mouse_action(c, bw, mouse, x, y);

	ফেরত;
}


/* exported interface documented in content/content.h */
bool content_keypress(কাঠামো hlcache_handle *h, uint32_t চাবি)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != ফাঁকা);

	যদি (c->handler->keypress != ফাঁকা)
		ফেরত c->handler->keypress(c, চাবি);

	ফেরত false;
}


/* exported interface documented in content/content.h */
শূন্য content_request_redraw(কাঠামো hlcache_handle *h,
			    পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, width, height);
}


/* exported interface, documented in content/protected.h */
শূন্য content__request_redraw(কাঠামো content *c,
			     পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height)
{
	সংঘ content_বার্তা_তথ্য তথ্য;

	যদি (c == ফাঁকা)
		ফেরত;

	তথ্য.redraw.x = x;
	তথ্য.redraw.y = y;
	তথ্য.redraw.width = width;
	তথ্য.redraw.height = height;

	content_broadcast(c, CONTENT_বার্তা_REDRAW, &তথ্য);
}


/* exported interface, documented in content/content.h */
bool content_exec(কাঠামো hlcache_handle *h, ধ্রুবক অক্ষর *src, size_t srclen)
{
	কাঠামো content *c = hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != ফাঁকা);

	যদি (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		ফেরত false;
	}

	যদি (c->handler->exec == ফাঁকা) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		ফেরত false;
	}

	ফেরত c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(কাঠামো hlcache_handle *h)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	কাঠামো nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool match;

	/* Is this an internal scheme? If so, we trust here and stop */
	যদি ((lwc_string_isequal(scheme, corestring_lwc_about,
				&match) == lwc_error_ok &&
	     (match == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_তথ্য,
				&match) == lwc_error_ok &&
	     (match == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&match) == lwc_error_ok &&
	     (match == true)) ||
	    /* Our internal x-ns-css scheme is secure */
	    (lwc_string_isequal(scheme, corestring_lwc_x_ns_css,
				&match) == lwc_error_ok &&
	     (match == true)) ||
	    /* We also treat file: as "not insecure" here */
	    (lwc_string_isequal(scheme, corestring_lwc_file,
				&match) == lwc_error_ok &&
	     (match == true))) {
		/* No insecurity to find */
		lwc_string_unref(scheme);
		ফেরত false;
	}

	/* Okay, not internal, am *I* secure? */
	যদি ((lwc_string_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok)
	    && (match == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		ফেরত true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	যদি (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		ফেরত true;
	}

	/* Otherwise try and chain through the handler */
	যদি (c != ফাঁকা && c->handler->saw_insecure_objects != ফাঁকা) {
		ফেরত c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	ফেরত false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       কাঠামো content_redraw_তথ্য *তথ্য,
	       ধ্রুবক কাঠামো rect *clip,
	       ধ্রুবক কাঠামো redraw_context *ctx)
{
	কাঠামো content *c = hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != ফাঁকা);

	যদি (c->locked) {
		/* not safe to attempt redraw */
		ফেরত true;
	}

	/* ensure we have a redrawable content */
	যদি (c->handler->redraw == ফাঁকা) {
		ফেরত true;
	}

	ফেরত c->handler->redraw(c, তথ্য, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(কাঠামো hlcache_handle *h,
		      পূর্ণসংখ্যা width, পূর্ণসংখ্যা height,
		      ধ্রুবক কাঠামো redraw_context *ctx)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	কাঠামো redraw_context new_ctx = *ctx;
	কাঠামো rect clip;
	কাঠামো content_redraw_তথ্য তথ্য;
	bool plot_ok = true;

	প্রতিজ্ঞা(c != ফাঁকা);

	/* ensure it is safe to attempt redraw */
	যদি (c->locked) {
		ফেরত true;
	}

	/* ensure we have a redrawable content */
	যদি (c->handler->redraw == ফাঁকা) {
		ফেরত true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, width, height, ctx);

	যদি (ctx->plot->option_knockout) {
		knockout_plot_start(ctx, &new_ctx);
	}

	/* Set clip rectangle to required thumbnail size */
	clip.x0 = 0;
	clip.y0 = 0;
	clip.x1 = width;
	clip.y1 = height;

	new_ctx.plot->clip(&new_ctx, &clip);

	/* Plot white background */
	plot_ok &= (new_ctx.plot->rectangle(&new_ctx,
					    plot_style_fill_white,
					    &clip) == NSERROR_OK);

	/* Set up content redraw data */
	তথ্য.x = 0;
	তথ্য.y = 0;
	তথ্য.width = width;
	তথ্য.height = height;

	তথ্য.background_colour = 0xFFFFFF;
	তথ্য.repeat_x = false;
	তথ্য.repeat_y = false;

	/* Find the scale factor to use if the content has a width */
	যদি (c->width) {
		তথ্য.scale = (দশমিক)width / (দশমিক)c->width;
	} অন্যথা {
		তথ্য.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &তথ্য, &clip, &new_ctx);

	যদি (ctx->plot->option_knockout) {
		knockout_plot_end(ctx);
	}

	ফেরত plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_add_user(কাঠামো content *c,
		 শূন্য (*callback)(
				  কাঠামো content *c,
				  content_বার্তা বার্তা,
				  ধ্রুবক সংঘ content_বার্তা_তথ্য *তথ্য,
				  শূন্য *pw),
		 শূন্য *pw)
{
	কাঠামো content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = malloc(আকার(কাঠামো content_user));
	যদি (!user)
		ফেরত false;
	user->callback = callback;
	user->pw = pw;
	user->পরবর্তী = c->user_list->পরবর্তী;
	c->user_list->পরবর্তী = user;

	যদি (c->handler->add_user != ফাঁকা)
		c->handler->add_user(c);

	ফেরত true;
}


/* exported interface documented in content/content.h */
শূন্য
content_remove_user(কাঠামো content *c,
		    শূন্য (*callback)(
				     কাঠামো content *c,
				     content_বার্তা বার্তা,
				     ধ্রুবক সংঘ content_বার্তা_তথ্য *তথ্য,
				     শূন্য *pw),
		    শূন্য *pw)
{
	কাঠামো content_user *user, *পরবর্তী;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	জন্য (user = c->user_list; user->পরবর্তী != 0 &&
		     !(user->পরবর্তী->callback == callback &&
		       user->পরবর্তী->pw == pw); user = user->পরবর্তী)
		;
	যদি (user->পরবর্তী == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		প্রতিজ্ঞা(0);
		ফেরত;
	}

	যদি (c->handler->remove_user != ফাঁকা)
		c->handler->remove_user(c);

	পরবর্তী = user->পরবর্তী;
	user->পরবর্তী = পরবর্তী->পরবর্তী;
	মুক্ত(পরবর্তী);
}


/* exported interface documented in content/content.h */
uint32_t content_গুনতি_users(কাঠামো content *c)
{
	কাঠামো content_user *user;
	uint32_t গণক = 0;

	প্রতিজ্ঞা(c != ফাঁকা);

	জন্য (user = c->user_list; user != ফাঁকা; user = user->পরবর্তী)
		গণক += 1;

	প্রতিজ্ঞা(গণক > 0);

	ফেরত গণক - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(কাঠামো content *c, bool quirks)
{
	যদি (c->handler->matches_quirks == ফাঁকা)
		ফেরত true;

	ফেরত c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(কাঠামো content *c)
{
	ফেরত c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
শূন্য content_broadcast(কাঠামো content *c, content_বার্তা বার্তা,
		       ধ্রুবক সংঘ content_বার্তা_তথ্য *তথ্য)
{
	কাঠামো content_user *user, *পরবর্তী;
	প্রতিজ্ঞা(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, বার্তা);
	জন্য (user = c->user_list->পরবর্তী; user != 0; user = পরবর্তী) {
		পরবর্তী = user->পরবর্তী;  /* user may be destroyed during callback */
		যদি (user->callback != 0)
			user->callback(c, বার্তা, তথ্য, user->pw);
	}
}


/* exported interface documented in content_protected.h */
শূন্য
content_broadcast_error(কাঠামো content *c, nserror errorcode, ধ্রুবক অক্ষর *বার্তা)
{
	কাঠামো content_user *user, *পরবর্তী;
	সংঘ content_বার্তা_তথ্য তথ্য;

	প্রতিজ্ঞা(c);

	তথ্য.errordata.errorcode = errorcode;
	তথ্য.errordata.errormsg = বার্তা;

	জন্য (user = c->user_list->পরবর্তী; user != 0; user = পরবর্তী) {
		পরবর্তী = user->পরবর্তী;  /* user may be destroyed during callback */
		যদি (user->callback != 0) {
			user->callback(c, CONTENT_বার্তা_ERROR,
				       &তথ্য, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     কাঠামো browser_window *bw,
	     কাঠামো content *page,
	     কাঠামো object_params *params)
{
	কাঠামো content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	যদি (c->handler->open != ফাঁকা) {
		res = c->handler->open(c, bw, page, params);
	} অন্যথা {
		res = NSERROR_OK;
	}
	ফেরত res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	কাঠামো content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	যদি (c == ফাঁকা) {
		ফেরত NSERROR_BAD_PARAMETER;
	}

	যদি ((c->status != CONTENT_STATUS_READY) &&
	    (c->status != CONTENT_STATUS_DONE)) {
		/* status is not read or done so nothing to do */
		ফেরত NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	যদি (c->textsearch.context != ফাঁকা) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ফাঁকা;
	}

	যদি (c->handler->close != ফাঁকা) {
		res = c->handler->close(c);
	} অন্যথা {
		res = NSERROR_OK;
	}
	ফেরত res;
}


/* exported interface, documented in content/content.h */
শূন্য content_clear_selection(hlcache_handle *h)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	যদি (c->handler->get_selection != ফাঁকা)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
অক্ষর * content_get_selection(hlcache_handle *h)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	যদি (c->handler->get_selection != ফাঁকা)
		ফেরত c->handler->get_selection(c);
	অন্যথা
		ফেরত ফাঁকা;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(কাঠামো hlcache_handle *h,
			       পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
			       কাঠামো browser_window_features *তথ্য)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	যদি (c->handler->get_contextual_content != ফাঁকা) {
		ফেরত c->handler->get_contextual_content(c, x, y, তথ্য);
	}

	তথ্য->object = h;
	ফেরত NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_point(কাঠামো hlcache_handle *h,
			পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
			পূর্ণসংখ্যা scrx, পূর্ণসংখ্যা scry)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	যদি (c->handler->scroll_at_point != ফাঁকা)
		ফেরত c->handler->scroll_at_point(c, x, y, scrx, scry);

	ফেরত false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_point(কাঠামো hlcache_handle *h,
			   পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
			   অক্ষর *file)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	যদি (c->handler->drop_file_at_point != ফাঁকা)
		ফেরত c->handler->drop_file_at_point(c, x, y, file);

	ফেরত false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(কাঠামো hlcache_handle *h, FILE *f, গণনা content_debug op)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	যদি (c->handler->debug_dump == ফাঁকা) {
		ফেরত NSERROR_NOT_IMPLEMENTED;
	}

	ফেরত c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(কাঠামো hlcache_handle *h, গণনা content_debug op)
{
	কাঠামো content *c = hlcache_handle_get_content(h);

	যদি (c == ফাঁকা) {
		ফেরত NSERROR_BAD_PARAMETER;
	}

	যদি (c->handler->debug == ফাঁকা) {
		ফেরত NSERROR_NOT_IMPLEMENTED;
	}

	ফেরত c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
কাঠামো content_rfc5988_link *
content_find_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	কাঠামো content *c = hlcache_handle_get_content(h);
	কাঠামো content_rfc5988_link *link = c->links;
	bool rel_match = false;

	যখন (link != ফাঁকা) {
		যদি (lwc_string_caseless_isequal(link->rel, rel,
						&rel_match) == lwc_error_ok && rel_match) {
			বিরতি;
		}
		link = link->পরবর্তী;
	}
	ফেরত link;
}


/* exported interface documented in content/protected.h */
কাঠামো content_rfc5988_link *
content__free_rfc5988_link(কাঠামো content_rfc5988_link *link)
{
	কাঠামো content_rfc5988_link *পরবর্তী;

	পরবর্তী = link->পরবর্তী;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	মুক্ত(link);

	ফেরত পরবর্তী;
}


/* exported interface documented in content/protected.h */
bool
content__add_rfc5988_link(কাঠামো content *c,
			  ধ্রুবক কাঠামো content_rfc5988_link *link)
{
	কাঠামো content_rfc5988_link *newlink;
	সংঘ content_বার্তা_তথ্য বার্তা_তথ্য;

	/* a link relation must be present for it to be a link */
	যদি (link->rel == ফাঁকা) {
		ফেরত false;
	}

	/* a link href must be present for it to be a link */
	যদি (link->href == ফাঁকা) {
		ফেরত false;
	}

	newlink = calloc(1, আকার(কাঠামো content_rfc5988_link));
	যদি (newlink == ফাঁকা) {
		ফেরত false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	যদি (link->hreflang != ফাঁকা) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	যদি (link->type != ফাঁকা) {
		newlink->type = lwc_string_ref(link->type);
	}
	যদি (link->media != ফাঁকা) {
		newlink->media = lwc_string_ref(link->media);
	}
	যদি (link->sizes != ফাঁকা) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->পরবর্তী = c->links;
	c->links = newlink;

	/* broadcast the data */
	বার্তা_তথ্য.rfc5988_link = newlink;
	content_broadcast(c, CONTENT_বার্তা_LINK, &বার্তা_তথ্য);

	ফেরত true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(কাঠামো content *c)
{
	যদি (c == ফাঁকা)
		ফেরত ফাঁকা;

	ফেরত llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	কাঠামো content *c = hlcache_handle_get_content(h);

	যদি (c == ফাঁকা)
		ফেরত CONTENT_NONE;

	ফেরত c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	ফেরত content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(কাঠামো content *c)
{
	যদি (c == ফাঁকা)
		ফেরত ফাঁকা;

	ফেরত lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content__set_title(কাঠামো content *c, ধ্রুবক অক্ষর *title)
{
	অক্ষর *new_title = strdup(title);
	যদি (new_title == ফাঁকা)
		ফেরত false;

	যদি (c->title != ফাঁকা)
		মুক্ত(c->title);

	c->title = new_title;

	ফেরত true;
}


/* exported interface documented in content/content.h */
ধ্রুবক অক্ষর *content_get_title(hlcache_handle *h)
{
	ফেরত content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ধ্রুবক অক্ষর *content__get_title(কাঠামো content *c)
{
	যদি (c == ফাঁকা)
		ফেরত ফাঁকা;

	ফেরত c->title != ফাঁকা ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_status content_get_status(hlcache_handle *h)
{
	ফেরত content__get_status(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_status content__get_status(কাঠামো content *c)
{
	যদি (c == ফাঁকা)
		ফেরত CONTENT_STATUS_ERROR;

	ফেরত c->status;
}


/* exported interface documented in content/content.h */
ধ্রুবক অক্ষর *content_get_status_message(hlcache_handle *h)
{
	ফেরত content__get_status_message(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ধ্রুবক অক্ষর *content__get_status_message(কাঠামো content *c)
{
	যদি (c == ফাঁকা)
		ফেরত ফাঁকা;

	ফেরত c->status_message;
}


/* exported interface documented in content/content.h */
পূর্ণসংখ্যা content_get_width(hlcache_handle *h)
{
	ফেরত content__get_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
পূর্ণসংখ্যা content__get_width(কাঠামো content *c)
{
	যদি (c == ফাঁকা)
		ফেরত 0;

	ফেরত c->width;
}


/* exported interface documented in content/content.h */
পূর্ণসংখ্যা content_get_height(hlcache_handle *h)
{
	ফেরত content__get_height(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
পূর্ণসংখ্যা content__get_height(কাঠামো content *c)
{
	যদি (c == ফাঁকা)
		ফেরত 0;

	ফেরত c->height;
}


/* exported interface documented in content/content.h */
পূর্ণসংখ্যা content_get_available_width(hlcache_handle *h)
{
	ফেরত content__get_available_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
পূর্ণসংখ্যা content__get_available_width(কাঠামো content *c)
{
	যদি (c == ফাঁকা)
		ফেরত 0;

	ফেরত c->available_width;
}


/* exported interface documented in content/content.h */
ধ্রুবক uint8_t *content_get_source_তথ্য(hlcache_handle *h, size_t *আকার_2)
{
	ফেরত content_get_source_তথ্য_2(hlcache_handle_get_content(h), আকার_2);
}


/* exported interface documented in content/content_protected.h */
ধ্রুবক uint8_t *content_get_source_তথ্য_2(কাঠামো content *c, size_t *আকার_2)
{
	প্রতিজ্ঞা(আকার_2 != ফাঁকা);

	/** \todo check if the content check should be an assert */
	যদি (c == ফাঁকা)
		ফেরত ফাঁকা;

	ফেরত llcache_handle_get_source_তথ্য(c->llcache, আকার_2);
}


/* exported interface documented in content/content.h */
শূন্য content_invalidate_reuse_তথ্য(hlcache_handle *h)
{
	content_invalidate_reuse_তথ্য_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
শূন্য content_invalidate_reuse_তথ্য_2(কাঠামো content *c)
{
	যদি (c == ফাঁকা || c->llcache == ফাঁকা)
		ফেরত;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_তথ্য(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	ফেরত content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(কাঠামো content *c)
{
	যদি (c == ফাঁকা)
		ফেরত ফাঁকা;

	ফেরত c->refresh;
}


/* exported interface documented in content/content.h */
কাঠামো bitmap *content_get_bitmap(hlcache_handle *h)
{
	ফেরত content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
কাঠামো bitmap *content__get_bitmap(কাঠামো content *c)
{
	কাঠামো bitmap *bitmap = ফাঁকা;

	যদি ((c != ফাঁকা) &&
	    (c->handler != ফাঁকা) &&
	    (c->handler->type != ফাঁকা) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_internal != ফাঁকা) ) {
		bitmap = c->handler->get_internal(c, ফাঁকা);
	}

	ফেরত bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	ফেরত content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(কাঠামো content *c)
{
	যদি ((c != ফাঁকা) &&
	    (c->handler != ফাঁকা) &&
	    (c->handler->is_opaque != ফাঁকা)) {
		ফেরত c->handler->is_opaque(c);
	}

	ফেরত false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	কাঠামো content *c = hlcache_handle_get_content(h);

	যদি (c == ফাঁকা)
		ফেরত false;

	ফেরত c->quirks;
}


/* exported interface documented in content/content.h */
ধ্রুবক অক্ষর *
content_get_encoding(hlcache_handle *h, গণনা content_encoding_type op)
{
	ফেরত content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
ধ্রুবক অক্ষর *
content__get_encoding(কাঠামো content *c, গণনা content_encoding_type op)
{
	ধ্রুবক অক্ষর *encoding = ফাঁকা;

	যদি ((c != ফাঁকা) &&
	    (c->handler != ফাঁকা) &&
	    (c->handler->get_encoding != ফাঁকা) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	ফেরত encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	ফেরত content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(কাঠামো content *c)
{
	ফেরত c->locked;
}


/* exported interface documented in content/content.h */
ধ্রুবক llcache_handle *content_get_llcache_handle(কাঠামো content *c)
{
	যদি (c == ফাঁকা)
		ফেরত ফাঁকা;

	ফেরত c->llcache;
}


/* exported interface documented in content/protected.h */
কাঠামো content *content_clone(কাঠামো content *c)
{
	কাঠামো content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	যদি (error != NSERROR_OK)
		ফেরত ফাঁকা;

	ফেরত nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(ধ্রুবক কাঠামো content *c, কাঠামো content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	যদি (error != NSERROR_OK) {
		ফেরত error;
	}

	llcache_handle_change_callback(nc->llcache,
				       content_llcache_callback, nc);

	nc->mime_type = lwc_string_ref(c->mime_type);
	nc->handler = c->handler;

	nc->status = c->status;

	nc->width = c->width;
	nc->height = c->height;
	nc->available_width = c->available_width;
	nc->quirks = c->quirks;

	যদি (c->fallback_charset != ফাঁকা) {
		nc->fallback_charset = strdup(c->fallback_charset);
		যদি (nc->fallback_charset == ফাঁকা) {
			ফেরত NSERROR_NOMEM;
		}
	}

	যদি (c->refresh != ফাঁকা) {
		nc->refresh = nsurl_ref(c->refresh);
		যদি (nc->refresh == ফাঁকা) {
			ফেরত NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_time = c->reformat_time;
	nc->আকার_2 = c->আকার_2;

	যদি (c->title != ফাঁকা) {
		nc->title = strdup(c->title);
		যদি (nc->title == ফাঁকা) {
			ফেরত NSERROR_NOMEM;
		}
	}

	nc->active = c->active;

	nc->user_list = calloc(1, আকার(কাঠামো content_user));
	যদি (nc->user_list == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	memcpy(&(nc->status_message), &(c->status_message), 120);
	memcpy(&(nc->sub_status), &(c->sub_status), 80);

	nc->locked = c->locked;
	nc->যোগ_আকার = c->যোগ_আকার;
	nc->http_code = c->http_code;

	ফেরত NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(কাঠামো content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	যদি (c->handler->stop != ফাঁকা)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	ফেরত llcache_handle_abort(c->llcache);
}
