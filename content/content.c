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

#অন্তর্ভাৱ <stdint.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <nsutils/time.h>

#অন্তর্ভাৱ "netsurf/inttypes.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "netsurf/browser_window.h"
#অন্তর্ভাৱ "netsurf/bitmap.h"
#অন্তর্ভাৱ "netsurf/content.h"
#অন্তর্ভাৱ "desktop/knockout.h"

#অন্তর্ভাৱ "content/content_protected.h"
#অন্তর্ভাৱ "content/textsearch.h"
#অন্তর্ভাৱ "content/content_debug.h"
#অন্তর্ভাৱ "content/hlcache.h"
#অন্তর্ভাৱ "content/urldb.h"

#define URL_FMT_SPC "%.140s"

ধ্রুৱ ৱর্ণ * ধ্রুৱ content_status_name[] = {
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
স্থির শূন্য content_convert(সংরচনা content *c)
{
	প্রতিজ্ঞা(c);
	প্রতিজ্ঞা(c->status == CONTENT_STATUS_LOADING ||
	       c->status == CONTENT_STATUS_ERROR);

	যদি (c->status != CONTENT_STATUS_LOADING)
		ঘূৰাই;

	যদি (c->locked == true)
		ঘূৰাই;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	যদি (c->handler->data_complete != রিক্ত) {
		c->locked = true;
		যদি (c->handler->data_complete(c) == false) {
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
			 ধ্রুৱ llcache_event *event, শূন্য *pw)
{
	সংরচনা content *c = pw;
	সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;
	nserror error = NSERROR_OK;

	ৱিকল্প (event->type) {
	পক্ষ LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		ভঙ্গ;
	পক্ষ LLCACHE_EVENT_HAD_HEADERS:
		/* Will never happen: handled in hlcache */
		ভঙ্গ;
	পক্ষ LLCACHE_EVENT_HAD_DATA:
		যদি (c->handler->process_data != রিক্ত) {
			যদি (c->handler->process_data(c,
						     (ধ্রুৱ ৱর্ণ *) event->data.data.buf,
						     event->data.data.len) == false) {
				llcache_handle_abort(c->llcache);
				c->status = CONTENT_STATUS_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		ভঙ্গ;
	পক্ষ LLCACHE_EVENT_DONE:
		{
			size_t source_size;

			(শূন্য) llcache_handle_get_source_data(llcache, &source_size);

			content_set_status(c, messages_get("Processing"));
			বাৰ্তা_data.explicit_status_text = রিক্ত;
			content_broadcast(c, CONTENT_বাৰ্তা_STATUS, &বাৰ্তা_data);

			content_convert(c);
		}
		ভঙ্গ;
	পক্ষ LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->status = CONTENT_STATUS_ERROR;
		বাৰ্তা_data.errordata.errorcode = event->data.error.code;
		বাৰ্তা_data.errordata.errormsg = event->data.error.বাৰ্তা;
		content_broadcast(c, CONTENT_বাৰ্তা_ERROR, &বাৰ্তা_data);
		ভঙ্গ;
	পক্ষ LLCACHE_EVENT_PROGRESS:
		content_set_status(c, event->data.progress_বাৰ্তা);
		বাৰ্তা_data.explicit_status_text = রিক্ত;
		content_broadcast(c, CONTENT_বাৰ্তা_STATUS, &বাৰ্তা_data);
		ভঙ্গ;
	পক্ষ LLCACHE_EVENT_REDIRECT:
		বাৰ্তা_data.redirect.from = event->data.redirect.from;
		বাৰ্তা_data.redirect.to = event->data.redirect.to;
		content_broadcast(c, CONTENT_বাৰ্তা_REDIRECT, &বাৰ্তা_data);
		ভঙ্গ;
	}

	ঘূৰাই error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
স্থির শূন্য content_update_status(সংরচনা content *c)
{
	যদি (c->status == CONTENT_STATUS_LOADING ||
	    c->status == CONTENT_STATUS_READY) {
		/* Not done yet */
		snprintf(c->status_message, পরিমাণ (c->status_message),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_status[0] != '\0' ? ", " : " ",
			 c->sub_status);
	} অন্যথা {
		snprintf(c->status_message, পরিমাণ (c->status_message),
			 "%s (%.1fs)", messages_get("Done"),
			 (ভিন্ন) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(সংরচনা content *c,
	      ধ্রুৱ content_handler *handler,
	      lwc_string *imime_type,
	      ধ্রুৱ সংরচনা http_parameter *params,
	      llcache_handle *llcache,
	      ধ্রুৱ ৱর্ণ *fallback_charset,
	      bool quirks)
{
	সংরচনা content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = calloc(1, পরিমাণ(সংরচনা content_user));
	যদি (user_sentinel == রিক্ত) {
		ঘূৰাই NSERROR_NOMEM;
	}

	যদি (fallback_charset != রিক্ত) {
		c->fallback_charset = strdup(fallback_charset);
		যদি (c->fallback_charset == রিক্ত) {
			মুক্ত(user_sentinel);
			ঘূৰাই NSERROR_NOMEM;
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
	c->size = 0;
	c->title = রিক্ত;
	c->active = 0;
	user_sentinel->callback = রিক্ত;
	user_sentinel->pw = রিক্ত;
	user_sentinel->next = রিক্ত;
	c->user_list = user_sentinel;
	c->sub_status[0] = 0;
	c->locked = false;
	c->total_size = 0;
	c->http_code = 0;

	c->textsearch.string = রিক্ত;
	c->textsearch.context = রিক্ত;

	content_set_status(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	যদি (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		ঘূৰাই error;
	}

	ঘূৰাই NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	সংরচনা content *c = hlcache_handle_get_content(h);

	যদি (c == রিক্ত)
		ঘূৰাই false;

	ঘূৰাই (c->handler->reformat != রিক্ত);
}


/* exported interface documented in content/protected.h */
শূন্য content_set_status(সংরচনা content *c, ধ্রুৱ ৱর্ণ *status_message)
{
	size_t len = strlen(status_message);

	যদি (len >= পরিমাণ(c->sub_status)) {
		len = পরিমাণ(c->sub_status) - 1;
	}
	memcpy(c->sub_status, status_message, len);
	c->sub_status[len] = '\0';

	content_update_status(c);
}


/* exported interface documented in content/protected.h */
শূন্য content_set_ready(সংরচনা content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	প্রতিজ্ঞা(c->locked);
	c->locked = false;

	c->status = CONTENT_STATUS_READY;
	content_update_status(c);
	content_broadcast(c, CONTENT_বাৰ্তা_READY, রিক্ত);
}


/* exported interface documented in content/protected.h */
শূন্য content_set_done(সংরচনা content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->status = CONTENT_STATUS_DONE;
	c->time = now_ms - c->time;
	content_update_status(c);
	content_broadcast(c, CONTENT_বাৰ্তা_DONE, রিক্ত);
}


/* exported interface documented in content/protected.h */
শূন্য content_set_error(সংরচনা content *c)
{
	c->locked = false;
	c->status = CONTENT_STATUS_ERROR;
}


/* exported interface documented in content/content.h */
শূন্য content_reformat(hlcache_handle *h, bool background,
		      পূর্ণাঙ্ক width, পূর্ণাঙ্ক height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  width, height);
}


/* exported interface documented in content/protected.h */
শূন্য
content__reformat(সংরচনা content *c, bool background, পূর্ণাঙ্ক width, পূর্ণাঙ্ক height)
{
	সঙ্ঘ content_বাৰ্তা_data data;
	প্রতিজ্ঞা(c != 0);
	প্রতিজ্ঞা(c->status == CONTENT_STATUS_READY ||
	       c->status == CONTENT_STATUS_DONE);
	প্রতিজ্ঞা(c->locked == false);

	c->available_width = width;
	c->available_height = height;
	যদি (c->handler->reformat != রিক্ত) {

		c->locked = true;
		c->handler->reformat(c, width, height);
		c->locked = false;

		data.background = background;
		content_broadcast(c, CONTENT_বাৰ্তা_REFORMAT, &data);
	}
}


/* exported interface documented in content/content.h */
শূন্য content_destroy(সংরচনা content *c)
{
	সংরচনা content_rfc5988_link *link;

	প্রতিজ্ঞা(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	প্রতিজ্ঞা(c->locked == false);

	যদি (c->handler->destroy != রিক্ত)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = রিক্ত;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	যেতিয়ালৈ (link != রিক্ত) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	যদি (c->user_list != রিক্ত) {
		মুক্ত(c->user_list);
	}

	/* free the title */
	যদি (c->title != রিক্ত) {
		মুক্ত(c->title);
	}

	/* free the fallback characterset */
	যদি (c->fallback_charset != রিক্ত) {
		মুক্ত(c->fallback_charset);
	}

	মুক্ত(c);
}


/* exported interface documented in content/content.h */
শূন্য
content_mouse_track(hlcache_handle *h,
		    সংরচনা browser_window *bw,
		    browser_mouse_state mouse,
		    পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != রিক্ত);

	যদি (c->handler->mouse_track != রিক্ত) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} অন্যথা {
		সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;
		বাৰ্তা_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(c, CONTENT_বাৰ্তা_POINTER, &বাৰ্তা_data);
	}


	ঘূৰাই;
}


/* exported interface documented in content/content.h */
শূন্য
content_mouse_action(hlcache_handle *h,
		     সংরচনা browser_window *bw,
		     browser_mouse_state mouse,
		     পূর্ণাঙ্ক x, পূর্ণাঙ্ক y)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != রিক্ত);

	যদি (c->handler->mouse_action != রিক্ত)
		c->handler->mouse_action(c, bw, mouse, x, y);

	ঘূৰাই;
}


/* exported interface documented in content/content.h */
bool content_keypress(সংরচনা hlcache_handle *h, uint32_t চাবি)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != রিক্ত);

	যদি (c->handler->keypress != রিক্ত)
		ঘূৰাই c->handler->keypress(c, চাবি);

	ঘূৰাই false;
}


/* exported interface documented in content/content.h */
শূন্য content_request_redraw(সংরচনা hlcache_handle *h,
			    পূর্ণাঙ্ক x, পূর্ণাঙ্ক y, পূর্ণাঙ্ক width, পূর্ণাঙ্ক height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, width, height);
}


/* exported interface, documented in content/protected.h */
শূন্য content__request_redraw(সংরচনা content *c,
			     পূর্ণাঙ্ক x, পূর্ণাঙ্ক y, পূর্ণাঙ্ক width, পূর্ণাঙ্ক height)
{
	সঙ্ঘ content_বাৰ্তা_data data;

	যদি (c == রিক্ত)
		ঘূৰাই;

	data.redraw.x = x;
	data.redraw.y = y;
	data.redraw.width = width;
	data.redraw.height = height;

	content_broadcast(c, CONTENT_বাৰ্তা_REDRAW, &data);
}


/* exported interface, documented in content/content.h */
bool content_exec(সংরচনা hlcache_handle *h, ধ্রুৱ ৱর্ণ *src, size_t srclen)
{
	সংরচনা content *c = hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != রিক্ত);

	যদি (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		ঘূৰাই false;
	}

	যদি (c->handler->exec == রিক্ত) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		ঘূৰাই false;
	}

	ঘূৰাই c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(সংরচনা hlcache_handle *h)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	সংরচনা nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool match;

	/* Is this an internal scheme? If so, we trust here and stop */
	যদি ((lwc_string_isequal(scheme, corestring_lwc_about,
				&match) == lwc_error_ok &&
	     (match == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_data,
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
		ঘূৰাই false;
	}

	/* Okay, not internal, am *I* secure? */
	যদি ((lwc_string_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok)
	    && (match == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		ঘূৰাই true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	যদি (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		ঘূৰাই true;
	}

	/* Otherwise try and chain through the handler */
	যদি (c != রিক্ত && c->handler->saw_insecure_objects != রিক্ত) {
		ঘূৰাই c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	ঘূৰাই false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       সংরচনা content_redraw_data *data,
	       ধ্রুৱ সংরচনা rect *clip,
	       ধ্রুৱ সংরচনা redraw_context *ctx)
{
	সংরচনা content *c = hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != রিক্ত);

	যদি (c->locked) {
		/* not safe to attempt redraw */
		ঘূৰাই true;
	}

	/* ensure we have a redrawable content */
	যদি (c->handler->redraw == রিক্ত) {
		ঘূৰাই true;
	}

	ঘূৰাই c->handler->redraw(c, data, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(সংরচনা hlcache_handle *h,
		      পূর্ণাঙ্ক width, পূর্ণাঙ্ক height,
		      ধ্রুৱ সংরচনা redraw_context *ctx)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	সংরচনা redraw_context new_ctx = *ctx;
	সংরচনা rect clip;
	সংরচনা content_redraw_data data;
	bool plot_ok = true;

	প্রতিজ্ঞা(c != রিক্ত);

	/* ensure it is safe to attempt redraw */
	যদি (c->locked) {
		ঘূৰাই true;
	}

	/* ensure we have a redrawable content */
	যদি (c->handler->redraw == রিক্ত) {
		ঘূৰাই true;
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
	data.x = 0;
	data.y = 0;
	data.width = width;
	data.height = height;

	data.background_colour = 0xFFFFFF;
	data.repeat_x = false;
	data.repeat_y = false;

	/* Find the scale factor to use if the content has a width */
	যদি (c->width) {
		data.scale = (ভিন্ন)width / (ভিন্ন)c->width;
	} অন্যথা {
		data.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &data, &clip, &new_ctx);

	যদি (ctx->plot->option_knockout) {
		knockout_plot_end(ctx);
	}

	ঘূৰাই plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_add_user(সংরচনা content *c,
		 শূন্য (*callback)(
				  সংরচনা content *c,
				  content_বাৰ্তা বাৰ্তা,
				  ধ্রুৱ সঙ্ঘ content_বাৰ্তা_data *data,
				  শূন্য *pw),
		 শূন্য *pw)
{
	সংরচনা content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = malloc(পরিমাণ(সংরচনা content_user));
	যদি (!user)
		ঘূৰাই false;
	user->callback = callback;
	user->pw = pw;
	user->next = c->user_list->next;
	c->user_list->next = user;

	যদি (c->handler->add_user != রিক্ত)
		c->handler->add_user(c);

	ঘূৰাই true;
}


/* exported interface documented in content/content.h */
শূন্য
content_remove_user(সংরচনা content *c,
		    শূন্য (*callback)(
				     সংরচনা content *c,
				     content_বাৰ্তা বাৰ্তা,
				     ধ্রুৱ সঙ্ঘ content_বাৰ্তা_data *data,
				     শূন্য *pw),
		    শূন্য *pw)
{
	সংরচনা content_user *user, *next;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	বাবে (user = c->user_list; user->next != 0 &&
		     !(user->next->callback == callback &&
		       user->next->pw == pw); user = user->next)
		;
	যদি (user->next == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		প্রতিজ্ঞা(0);
		ঘূৰাই;
	}

	যদি (c->handler->remove_user != রিক্ত)
		c->handler->remove_user(c);

	next = user->next;
	user->next = next->next;
	মুক্ত(next);
}


/* exported interface documented in content/content.h */
uint32_t content_গুনতি_users(সংরচনা content *c)
{
	সংরচনা content_user *user;
	uint32_t counter = 0;

	প্রতিজ্ঞা(c != রিক্ত);

	বাবে (user = c->user_list; user != রিক্ত; user = user->next)
		counter += 1;

	প্রতিজ্ঞা(counter > 0);

	ঘূৰাই counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(সংরচনা content *c, bool quirks)
{
	যদি (c->handler->matches_quirks == রিক্ত)
		ঘূৰাই true;

	ঘূৰাই c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(সংরচনা content *c)
{
	ঘূৰাই c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
শূন্য content_broadcast(সংরচনা content *c, content_বাৰ্তা বাৰ্তা,
		       ধ্রুৱ সঙ্ঘ content_বাৰ্তা_data *data)
{
	সংরচনা content_user *user, *next;
	প্রতিজ্ঞা(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, বাৰ্তা);
	বাবে (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		যদি (user->callback != 0)
			user->callback(c, বাৰ্তা, data, user->pw);
	}
}


/* exported interface documented in content_protected.h */
শূন্য
content_broadcast_error(সংরচনা content *c, nserror errorcode, ধ্রুৱ ৱর্ণ *বাৰ্তা)
{
	সংরচনা content_user *user, *next;
	সঙ্ঘ content_বাৰ্তা_data data;

	প্রতিজ্ঞা(c);

	data.errordata.errorcode = errorcode;
	data.errordata.errormsg = বাৰ্তা;

	বাবে (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		যদি (user->callback != 0) {
			user->callback(c, CONTENT_বাৰ্তা_ERROR,
				       &data, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     সংরচনা browser_window *bw,
	     সংরচনা content *page,
	     সংরচনা object_params *params)
{
	সংরচনা content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	যদি (c->handler->open != রিক্ত) {
		res = c->handler->open(c, bw, page, params);
	} অন্যথা {
		res = NSERROR_OK;
	}
	ঘূৰাই res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	সংরচনা content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	যদি (c == রিক্ত) {
		ঘূৰাই NSERROR_BAD_PARAMETER;
	}

	যদি ((c->status != CONTENT_STATUS_READY) &&
	    (c->status != CONTENT_STATUS_DONE)) {
		/* status is not read or done so nothing to do */
		ঘূৰাই NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	যদি (c->textsearch.context != রিক্ত) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = রিক্ত;
	}

	যদি (c->handler->close != রিক্ত) {
		res = c->handler->close(c);
	} অন্যথা {
		res = NSERROR_OK;
	}
	ঘূৰাই res;
}


/* exported interface, documented in content/content.h */
শূন্য content_clear_selection(hlcache_handle *h)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	যদি (c->handler->get_selection != রিক্ত)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
ৱর্ণ * content_get_selection(hlcache_handle *h)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	যদি (c->handler->get_selection != রিক্ত)
		ঘূৰাই c->handler->get_selection(c);
	অন্যথা
		ঘূৰাই রিক্ত;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(সংরচনা hlcache_handle *h,
			       পূর্ণাঙ্ক x, পূর্ণাঙ্ক y,
			       সংরচনা browser_window_features *data)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	যদি (c->handler->get_contextual_content != রিক্ত) {
		ঘূৰাই c->handler->get_contextual_content(c, x, y, data);
	}

	data->object = h;
	ঘূৰাই NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_point(সংরচনা hlcache_handle *h,
			পূর্ণাঙ্ক x, পূর্ণাঙ্ক y,
			পূর্ণাঙ্ক scrx, পূর্ণাঙ্ক scry)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	যদি (c->handler->scroll_at_point != রিক্ত)
		ঘূৰাই c->handler->scroll_at_point(c, x, y, scrx, scry);

	ঘূৰাই false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_point(সংরচনা hlcache_handle *h,
			   পূর্ণাঙ্ক x, পূর্ণাঙ্ক y,
			   ৱর্ণ *file)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	যদি (c->handler->drop_file_at_point != রিক্ত)
		ঘূৰাই c->handler->drop_file_at_point(c, x, y, file);

	ঘূৰাই false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(সংরচনা hlcache_handle *h, FILE *f, গণনা content_debug op)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	প্রতিজ্ঞা(c != 0);

	যদি (c->handler->debug_dump == রিক্ত) {
		ঘূৰাই NSERROR_NOT_IMPLEMENTED;
	}

	ঘূৰাই c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(সংরচনা hlcache_handle *h, গণনা content_debug op)
{
	সংরচনা content *c = hlcache_handle_get_content(h);

	যদি (c == রিক্ত) {
		ঘূৰাই NSERROR_BAD_PARAMETER;
	}

	যদি (c->handler->debug == রিক্ত) {
		ঘূৰাই NSERROR_NOT_IMPLEMENTED;
	}

	ঘূৰাই c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
সংরচনা content_rfc5988_link *
content_find_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	সংরচনা content *c = hlcache_handle_get_content(h);
	সংরচনা content_rfc5988_link *link = c->links;
	bool rel_match = false;

	যেতিয়ালৈ (link != রিক্ত) {
		যদি (lwc_string_caseless_isequal(link->rel, rel,
						&rel_match) == lwc_error_ok && rel_match) {
			ভঙ্গ;
		}
		link = link->next;
	}
	ঘূৰাই link;
}


/* exported interface documented in content/protected.h */
সংরচনা content_rfc5988_link *
content__free_rfc5988_link(সংরচনা content_rfc5988_link *link)
{
	সংরচনা content_rfc5988_link *next;

	next = link->next;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	মুক্ত(link);

	ঘূৰাই next;
}


/* exported interface documented in content/protected.h */
bool
content__add_rfc5988_link(সংরচনা content *c,
			  ধ্রুৱ সংরচনা content_rfc5988_link *link)
{
	সংরচনা content_rfc5988_link *newlink;
	সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;

	/* a link relation must be present for it to be a link */
	যদি (link->rel == রিক্ত) {
		ঘূৰাই false;
	}

	/* a link href must be present for it to be a link */
	যদি (link->href == রিক্ত) {
		ঘূৰাই false;
	}

	newlink = calloc(1, পরিমাণ(সংরচনা content_rfc5988_link));
	যদি (newlink == রিক্ত) {
		ঘূৰাই false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	যদি (link->hreflang != রিক্ত) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	যদি (link->type != রিক্ত) {
		newlink->type = lwc_string_ref(link->type);
	}
	যদি (link->media != রিক্ত) {
		newlink->media = lwc_string_ref(link->media);
	}
	যদি (link->sizes != রিক্ত) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->next = c->links;
	c->links = newlink;

	/* broadcast the data */
	বাৰ্তা_data.rfc5988_link = newlink;
	content_broadcast(c, CONTENT_বাৰ্তা_LINK, &বাৰ্তা_data);

	ঘূৰাই true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(সংরচনা content *c)
{
	যদি (c == রিক্ত)
		ঘূৰাই রিক্ত;

	ঘূৰাই llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	সংরচনা content *c = hlcache_handle_get_content(h);

	যদি (c == রিক্ত)
		ঘূৰাই CONTENT_NONE;

	ঘূৰাই c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	ঘূৰাই content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(সংরচনা content *c)
{
	যদি (c == রিক্ত)
		ঘূৰাই রিক্ত;

	ঘূৰাই lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content__set_title(সংরচনা content *c, ধ্রুৱ ৱর্ণ *title)
{
	ৱর্ণ *new_title = strdup(title);
	যদি (new_title == রিক্ত)
		ঘূৰাই false;

	যদি (c->title != রিক্ত)
		মুক্ত(c->title);

	c->title = new_title;

	ঘূৰাই true;
}


/* exported interface documented in content/content.h */
ধ্রুৱ ৱর্ণ *content_get_title(hlcache_handle *h)
{
	ঘূৰাই content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ধ্রুৱ ৱর্ণ *content__get_title(সংরচনা content *c)
{
	যদি (c == রিক্ত)
		ঘূৰাই রিক্ত;

	ঘূৰাই c->title != রিক্ত ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_status content_get_status(hlcache_handle *h)
{
	ঘূৰাই content__get_status(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_status content__get_status(সংরচনা content *c)
{
	যদি (c == রিক্ত)
		ঘূৰাই CONTENT_STATUS_ERROR;

	ঘূৰাই c->status;
}


/* exported interface documented in content/content.h */
ধ্রুৱ ৱর্ণ *content_get_status_message(hlcache_handle *h)
{
	ঘূৰাই content__get_status_message(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ধ্রুৱ ৱর্ণ *content__get_status_message(সংরচনা content *c)
{
	যদি (c == রিক্ত)
		ঘূৰাই রিক্ত;

	ঘূৰাই c->status_message;
}


/* exported interface documented in content/content.h */
পূর্ণাঙ্ক content_get_width(hlcache_handle *h)
{
	ঘূৰাই content__get_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
পূর্ণাঙ্ক content__get_width(সংরচনা content *c)
{
	যদি (c == রিক্ত)
		ঘূৰাই 0;

	ঘূৰাই c->width;
}


/* exported interface documented in content/content.h */
পূর্ণাঙ্ক content_get_height(hlcache_handle *h)
{
	ঘূৰাই content__get_height(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
পূর্ণাঙ্ক content__get_height(সংরচনা content *c)
{
	যদি (c == রিক্ত)
		ঘূৰাই 0;

	ঘূৰাই c->height;
}


/* exported interface documented in content/content.h */
পূর্ণাঙ্ক content_get_available_width(hlcache_handle *h)
{
	ঘূৰাই content__get_available_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
পূর্ণাঙ্ক content__get_available_width(সংরচনা content *c)
{
	যদি (c == রিক্ত)
		ঘূৰাই 0;

	ঘূৰাই c->available_width;
}


/* exported interface documented in content/content.h */
ধ্রুৱ uint8_t *content_get_source_data(hlcache_handle *h, size_t *size)
{
	ঘূৰাই content__get_source_data(hlcache_handle_get_content(h), size);
}


/* exported interface documented in content/content_protected.h */
ধ্রুৱ uint8_t *content__get_source_data(সংরচনা content *c, size_t *size)
{
	প্রতিজ্ঞা(size != রিক্ত);

	/** \todo check if the content check should be an assert */
	যদি (c == রিক্ত)
		ঘূৰাই রিক্ত;

	ঘূৰাই llcache_handle_get_source_data(c->llcache, size);
}


/* exported interface documented in content/content.h */
শূন্য content_invalidate_reuse_data(hlcache_handle *h)
{
	content__invalidate_reuse_data(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
শূন্য content__invalidate_reuse_data(সংরচনা content *c)
{
	যদি (c == রিক্ত || c->llcache == রিক্ত)
		ঘূৰাই;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_data(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	ঘূৰাই content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(সংরচনা content *c)
{
	যদি (c == রিক্ত)
		ঘূৰাই রিক্ত;

	ঘূৰাই c->refresh;
}


/* exported interface documented in content/content.h */
সংরচনা bitmap *content_get_bitmap(hlcache_handle *h)
{
	ঘূৰাই content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
সংরচনা bitmap *content__get_bitmap(সংরচনা content *c)
{
	সংরচনা bitmap *bitmap = রিক্ত;

	যদি ((c != রিক্ত) &&
	    (c->handler != রিক্ত) &&
	    (c->handler->type != রিক্ত) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_internal != রিক্ত) ) {
		bitmap = c->handler->get_internal(c, রিক্ত);
	}

	ঘূৰাই bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	ঘূৰাই content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(সংরচনা content *c)
{
	যদি ((c != রিক্ত) &&
	    (c->handler != রিক্ত) &&
	    (c->handler->is_opaque != রিক্ত)) {
		ঘূৰাই c->handler->is_opaque(c);
	}

	ঘূৰাই false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	সংরচনা content *c = hlcache_handle_get_content(h);

	যদি (c == রিক্ত)
		ঘূৰাই false;

	ঘূৰাই c->quirks;
}


/* exported interface documented in content/content.h */
ধ্রুৱ ৱর্ণ *
content_get_encoding(hlcache_handle *h, গণনা content_encoding_type op)
{
	ঘূৰাই content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
ধ্রুৱ ৱর্ণ *
content__get_encoding(সংরচনা content *c, গণনা content_encoding_type op)
{
	ধ্রুৱ ৱর্ণ *encoding = রিক্ত;

	যদি ((c != রিক্ত) &&
	    (c->handler != রিক্ত) &&
	    (c->handler->get_encoding != রিক্ত) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	ঘূৰাই encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	ঘূৰাই content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(সংরচনা content *c)
{
	ঘূৰাই c->locked;
}


/* exported interface documented in content/content.h */
ধ্রুৱ llcache_handle *content_get_llcache_handle(সংরচনা content *c)
{
	যদি (c == রিক্ত)
		ঘূৰাই রিক্ত;

	ঘূৰাই c->llcache;
}


/* exported interface documented in content/protected.h */
সংরচনা content *content_clone(সংরচনা content *c)
{
	সংরচনা content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	যদি (error != NSERROR_OK)
		ঘূৰাই রিক্ত;

	ঘূৰাই nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(ধ্রুৱ সংরচনা content *c, সংরচনা content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	যদি (error != NSERROR_OK) {
		ঘূৰাই error;
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

	যদি (c->fallback_charset != রিক্ত) {
		nc->fallback_charset = strdup(c->fallback_charset);
		যদি (nc->fallback_charset == রিক্ত) {
			ঘূৰাই NSERROR_NOMEM;
		}
	}

	যদি (c->refresh != রিক্ত) {
		nc->refresh = nsurl_ref(c->refresh);
		যদি (nc->refresh == রিক্ত) {
			ঘূৰাই NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_time = c->reformat_time;
	nc->size = c->size;

	যদি (c->title != রিক্ত) {
		nc->title = strdup(c->title);
		যদি (nc->title == রিক্ত) {
			ঘূৰাই NSERROR_NOMEM;
		}
	}

	nc->active = c->active;

	nc->user_list = calloc(1, পরিমাণ(সংরচনা content_user));
	যদি (nc->user_list == রিক্ত) {
		ঘূৰাই NSERROR_NOMEM;
	}

	memcpy(&(nc->status_message), &(c->status_message), 120);
	memcpy(&(nc->sub_status), &(c->sub_status), 80);

	nc->locked = c->locked;
	nc->total_size = c->total_size;
	nc->http_code = c->http_code;

	ঘূৰাই NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(সংরচনা content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	যদি (c->handler->stop != রিক্ত)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	ঘূৰাই llcache_handle_abort(c->llcache);
}
