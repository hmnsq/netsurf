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

#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <nsutils/time.h>

#அந்தர்பாவ "netsurf/inttypes.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "netsurf/browser_window.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "desktop/knockout.h"

#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/textsearch.h"
#அந்தர்பாவ "content/content_debug.h"
#அந்தர்பாவ "content/hlcache.h"
#அந்தர்பாவ "content/urldb.h"

#define URL_FMT_SPC "%.140s"

மாறிலி எழுத்து * மாறிலி content_status_name[] = {
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
நிலையான வெறுமை content_convert(கட்டமைப்பு content *c)
{
	ப்ரதிஜ்ஞா(c);
	ப்ரதிஜ்ஞா(c->status == CONTENT_STATUS_LOADING ||
	       c->status == CONTENT_STATUS_ERROR);

	எனில் (c->status != CONTENT_STATUS_LOADING)
		திரும்பு;

	எனில் (c->locked == true)
		திரும்பு;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	எனில் (c->handler->data_complete != வெற்று) {
		c->locked = true;
		எனில் (c->handler->data_complete(c) == false) {
			content_set_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} இல்லையெனில் {
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
நிலையான nserror
content_llcache_callback(llcache_handle *llcache,
			 மாறிலி llcache_event *event, வெறுமை *pw)
{
	கட்டமைப்பு content *c = pw;
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;
	nserror error = NSERROR_OK;

	தேர்வு (event->type) {
	நிலை LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		நிறுத்து;
	நிலை LLCACHE_EVENT_HAD_HEADERS:
		/* Will never happen: handled in hlcache */
		நிறுத்து;
	நிலை LLCACHE_EVENT_HAD_DATA:
		எனில் (c->handler->process_data != வெற்று) {
			எனில் (c->handler->process_data(c,
						     (மாறிலி எழுத்து *) event->data.data.buf,
						     event->data.data.len) == false) {
				llcache_handle_abort(c->llcache);
				c->status = CONTENT_STATUS_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		நிறுத்து;
	நிலை LLCACHE_EVENT_DONE:
		{
			size_t source_size;

			(வெறுமை) llcache_handle_get_source_data(llcache, &source_size);

			content_set_status(c, messages_get("Processing"));
			செய்தி_data.explicit_status_text = வெற்று;
			content_broadcast(c, CONTENT_செய்தி_STATUS, &செய்தி_data);

			content_convert(c);
		}
		நிறுத்து;
	நிலை LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->status = CONTENT_STATUS_ERROR;
		செய்தி_data.errordata.errorcode = event->data.error.code;
		செய்தி_data.errordata.errormsg = event->data.error.செய்தி;
		content_broadcast(c, CONTENT_செய்தி_ERROR, &செய்தி_data);
		நிறுத்து;
	நிலை LLCACHE_EVENT_PROGRESS:
		content_set_status(c, event->data.progress_செய்தி);
		செய்தி_data.explicit_status_text = வெற்று;
		content_broadcast(c, CONTENT_செய்தி_STATUS, &செய்தி_data);
		நிறுத்து;
	நிலை LLCACHE_EVENT_REDIRECT:
		செய்தி_data.redirect.from = event->data.redirect.from;
		செய்தி_data.redirect.to = event->data.redirect.to;
		content_broadcast(c, CONTENT_செய்தி_REDIRECT, &செய்தி_data);
		நிறுத்து;
	}

	திரும்பு error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
நிலையான வெறுமை content_update_status(கட்டமைப்பு content *c)
{
	எனில் (c->status == CONTENT_STATUS_LOADING ||
	    c->status == CONTENT_STATUS_READY) {
		/* Not done yet */
		snprintf(c->status_message, அளவு (c->status_message),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_status[0] != '\0' ? ", " : " ",
			 c->sub_status);
	} இல்லையெனில் {
		snprintf(c->status_message, அளவு (c->status_message),
			 "%s (%.1fs)", messages_get("Done"),
			 (தசம) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(கட்டமைப்பு content *c,
	      மாறிலி content_handler *handler,
	      lwc_string *imime_type,
	      மாறிலி கட்டமைப்பு http_parameter *params,
	      llcache_handle *llcache,
	      மாறிலி எழுத்து *fallback_charset,
	      bool quirks)
{
	கட்டமைப்பு content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = calloc(1, அளவு(கட்டமைப்பு content_user));
	எனில் (user_sentinel == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	எனில் (fallback_charset != வெற்று) {
		c->fallback_charset = strdup(fallback_charset);
		எனில் (c->fallback_charset == வெற்று) {
			விடுவி(user_sentinel);
			திரும்பு NSERROR_NOMEM;
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
	c->title = வெற்று;
	c->active = 0;
	user_sentinel->callback = வெற்று;
	user_sentinel->pw = வெற்று;
	user_sentinel->next = வெற்று;
	c->user_list = user_sentinel;
	c->sub_status[0] = 0;
	c->locked = false;
	c->total_size = 0;
	c->http_code = 0;

	c->textsearch.string = வெற்று;
	c->textsearch.context = வெற்று;

	content_set_status(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	எனில் (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		திரும்பு error;
	}

	திரும்பு NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);

	எனில் (c == வெற்று)
		திரும்பு false;

	திரும்பு (c->handler->reformat != வெற்று);
}


/* exported interface documented in content/protected.h */
வெறுமை content_set_status(கட்டமைப்பு content *c, மாறிலி எழுத்து *status_message)
{
	size_t len = strlen(status_message);

	எனில் (len >= அளவு(c->sub_status)) {
		len = அளவு(c->sub_status) - 1;
	}
	memcpy(c->sub_status, status_message, len);
	c->sub_status[len] = '\0';

	content_update_status(c);
}


/* exported interface documented in content/protected.h */
வெறுமை content_set_ready(கட்டமைப்பு content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	ப்ரதிஜ்ஞா(c->locked);
	c->locked = false;

	c->status = CONTENT_STATUS_READY;
	content_update_status(c);
	content_broadcast(c, CONTENT_செய்தி_READY, வெற்று);
}


/* exported interface documented in content/protected.h */
வெறுமை content_set_done(கட்டமைப்பு content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->status = CONTENT_STATUS_DONE;
	c->time = now_ms - c->time;
	content_update_status(c);
	content_broadcast(c, CONTENT_செய்தி_DONE, வெற்று);
}


/* exported interface documented in content/protected.h */
வெறுமை content_set_error(கட்டமைப்பு content *c)
{
	c->locked = false;
	c->status = CONTENT_STATUS_ERROR;
}


/* exported interface documented in content/content.h */
வெறுமை content_reformat(hlcache_handle *h, bool background,
		      முழு width, முழு height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  width, height);
}


/* exported interface documented in content/protected.h */
வெறுமை
content__reformat(கட்டமைப்பு content *c, bool background, முழு width, முழு height)
{
	ஒன்றிப்பு content_செய்தி_data data;
	ப்ரதிஜ்ஞா(c != 0);
	ப்ரதிஜ்ஞா(c->status == CONTENT_STATUS_READY ||
	       c->status == CONTENT_STATUS_DONE);
	ப்ரதிஜ்ஞா(c->locked == false);

	c->available_width = width;
	c->available_height = height;
	எனில் (c->handler->reformat != வெற்று) {

		c->locked = true;
		c->handler->reformat(c, width, height);
		c->locked = false;

		data.background = background;
		content_broadcast(c, CONTENT_செய்தி_REFORMAT, &data);
	}
}


/* exported interface documented in content/content.h */
வெறுமை content_destroy(கட்டமைப்பு content *c)
{
	கட்டமைப்பு content_rfc5988_link *link;

	ப்ரதிஜ்ஞா(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	ப்ரதிஜ்ஞா(c->locked == false);

	எனில் (c->handler->destroy != வெற்று)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = வெற்று;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	போது (link != வெற்று) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	எனில் (c->user_list != வெற்று) {
		விடுவி(c->user_list);
	}

	/* free the title */
	எனில் (c->title != வெற்று) {
		விடுவி(c->title);
	}

	/* free the fallback characterset */
	எனில் (c->fallback_charset != வெற்று) {
		விடுவி(c->fallback_charset);
	}

	விடுவி(c);
}


/* exported interface documented in content/content.h */
வெறுமை
content_mouse_track(hlcache_handle *h,
		    கட்டமைப்பு browser_window *bw,
		    browser_mouse_state mouse,
		    முழு x, முழு y)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	ப்ரதிஜ்ஞா(c != வெற்று);

	எனில் (c->handler->mouse_track != வெற்று) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} இல்லையெனில் {
		ஒன்றிப்பு content_செய்தி_data செய்தி_data;
		செய்தி_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(c, CONTENT_செய்தி_POINTER, &செய்தி_data);
	}


	திரும்பு;
}


/* exported interface documented in content/content.h */
வெறுமை
content_mouse_action(hlcache_handle *h,
		     கட்டமைப்பு browser_window *bw,
		     browser_mouse_state mouse,
		     முழு x, முழு y)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	ப்ரதிஜ்ஞா(c != வெற்று);

	எனில் (c->handler->mouse_action != வெற்று)
		c->handler->mouse_action(c, bw, mouse, x, y);

	திரும்பு;
}


/* exported interface documented in content/content.h */
bool content_keypress(கட்டமைப்பு hlcache_handle *h, uint32_t சாவி)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	ப்ரதிஜ்ஞா(c != வெற்று);

	எனில் (c->handler->keypress != வெற்று)
		திரும்பு c->handler->keypress(c, சாவி);

	திரும்பு false;
}


/* exported interface documented in content/content.h */
வெறுமை content_request_redraw(கட்டமைப்பு hlcache_handle *h,
			    முழு x, முழு y, முழு width, முழு height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, width, height);
}


/* exported interface, documented in content/protected.h */
வெறுமை content__request_redraw(கட்டமைப்பு content *c,
			     முழு x, முழு y, முழு width, முழு height)
{
	ஒன்றிப்பு content_செய்தி_data data;

	எனில் (c == வெற்று)
		திரும்பு;

	data.redraw.x = x;
	data.redraw.y = y;
	data.redraw.width = width;
	data.redraw.height = height;

	content_broadcast(c, CONTENT_செய்தி_REDRAW, &data);
}


/* exported interface, documented in content/content.h */
bool content_exec(கட்டமைப்பு hlcache_handle *h, மாறிலி எழுத்து *src, size_t srclen)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);

	ப்ரதிஜ்ஞா(c != வெற்று);

	எனில் (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		திரும்பு false;
	}

	எனில் (c->handler->exec == வெற்று) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		திரும்பு false;
	}

	திரும்பு c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(கட்டமைப்பு hlcache_handle *h)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	கட்டமைப்பு nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool match;

	/* Is this an internal scheme? If so, we trust here and stop */
	எனில் ((lwc_string_isequal(scheme, corestring_lwc_about,
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
		திரும்பு false;
	}

	/* Okay, not internal, am *I* secure? */
	எனில் ((lwc_string_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok)
	    && (match == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		திரும்பு true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	எனில் (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		திரும்பு true;
	}

	/* Otherwise try and chain through the handler */
	எனில் (c != வெற்று && c->handler->saw_insecure_objects != வெற்று) {
		திரும்பு c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	திரும்பு false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       கட்டமைப்பு content_redraw_data *data,
	       மாறிலி கட்டமைப்பு rect *clip,
	       மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);

	ப்ரதிஜ்ஞா(c != வெற்று);

	எனில் (c->locked) {
		/* not safe to attempt redraw */
		திரும்பு true;
	}

	/* ensure we have a redrawable content */
	எனில் (c->handler->redraw == வெற்று) {
		திரும்பு true;
	}

	திரும்பு c->handler->redraw(c, data, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(கட்டமைப்பு hlcache_handle *h,
		      முழு width, முழு height,
		      மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	கட்டமைப்பு redraw_context new_ctx = *ctx;
	கட்டமைப்பு rect clip;
	கட்டமைப்பு content_redraw_data data;
	bool plot_ok = true;

	ப்ரதிஜ்ஞா(c != வெற்று);

	/* ensure it is safe to attempt redraw */
	எனில் (c->locked) {
		திரும்பு true;
	}

	/* ensure we have a redrawable content */
	எனில் (c->handler->redraw == வெற்று) {
		திரும்பு true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, width, height, ctx);

	எனில் (ctx->plot->option_knockout) {
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
	எனில் (c->width) {
		data.scale = (தசம)width / (தசம)c->width;
	} இல்லையெனில் {
		data.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &data, &clip, &new_ctx);

	எனில் (ctx->plot->option_knockout) {
		knockout_plot_end(ctx);
	}

	திரும்பு plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_add_user(கட்டமைப்பு content *c,
		 வெறுமை (*callback)(
				  கட்டமைப்பு content *c,
				  content_செய்தி செய்தி,
				  மாறிலி ஒன்றிப்பு content_செய்தி_data *data,
				  வெறுமை *pw),
		 வெறுமை *pw)
{
	கட்டமைப்பு content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = malloc(அளவு(கட்டமைப்பு content_user));
	எனில் (!user)
		திரும்பு false;
	user->callback = callback;
	user->pw = pw;
	user->next = c->user_list->next;
	c->user_list->next = user;

	எனில் (c->handler->add_user != வெற்று)
		c->handler->add_user(c);

	திரும்பு true;
}


/* exported interface documented in content/content.h */
வெறுமை
content_remove_user(கட்டமைப்பு content *c,
		    வெறுமை (*callback)(
				     கட்டமைப்பு content *c,
				     content_செய்தி செய்தி,
				     மாறிலி ஒன்றிப்பு content_செய்தி_data *data,
				     வெறுமை *pw),
		    வெறுமை *pw)
{
	கட்டமைப்பு content_user *user, *next;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	க்கு (user = c->user_list; user->next != 0 &&
		     !(user->next->callback == callback &&
		       user->next->pw == pw); user = user->next)
		;
	எனில் (user->next == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		ப்ரதிஜ்ஞா(0);
		திரும்பு;
	}

	எனில் (c->handler->remove_user != வெற்று)
		c->handler->remove_user(c);

	next = user->next;
	user->next = next->next;
	விடுவி(next);
}


/* exported interface documented in content/content.h */
uint32_t content_மொத்தம்_users(கட்டமைப்பு content *c)
{
	கட்டமைப்பு content_user *user;
	uint32_t counter = 0;

	ப்ரதிஜ்ஞா(c != வெற்று);

	க்கு (user = c->user_list; user != வெற்று; user = user->next)
		counter += 1;

	ப்ரதிஜ்ஞா(counter > 0);

	திரும்பு counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(கட்டமைப்பு content *c, bool quirks)
{
	எனில் (c->handler->matches_quirks == வெற்று)
		திரும்பு true;

	திரும்பு c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(கட்டமைப்பு content *c)
{
	திரும்பு c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
வெறுமை content_broadcast(கட்டமைப்பு content *c, content_செய்தி செய்தி,
		       மாறிலி ஒன்றிப்பு content_செய்தி_data *data)
{
	கட்டமைப்பு content_user *user, *next;
	ப்ரதிஜ்ஞா(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, செய்தி);
	க்கு (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		எனில் (user->callback != 0)
			user->callback(c, செய்தி, data, user->pw);
	}
}


/* exported interface documented in content_protected.h */
வெறுமை
content_broadcast_error(கட்டமைப்பு content *c, nserror errorcode, மாறிலி எழுத்து *செய்தி)
{
	கட்டமைப்பு content_user *user, *next;
	ஒன்றிப்பு content_செய்தி_data data;

	ப்ரதிஜ்ஞா(c);

	data.errordata.errorcode = errorcode;
	data.errordata.errormsg = செய்தி;

	க்கு (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		எனில் (user->callback != 0) {
			user->callback(c, CONTENT_செய்தி_ERROR,
				       &data, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     கட்டமைப்பு browser_window *bw,
	     கட்டமைப்பு content *page,
	     கட்டமைப்பு object_params *params)
{
	கட்டமைப்பு content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	ப்ரதிஜ்ஞா(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	எனில் (c->handler->open != வெற்று) {
		res = c->handler->open(c, bw, page, params);
	} இல்லையெனில் {
		res = NSERROR_OK;
	}
	திரும்பு res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	கட்டமைப்பு content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	எனில் (c == வெற்று) {
		திரும்பு NSERROR_BAD_PARAMETER;
	}

	எனில் ((c->status != CONTENT_STATUS_READY) &&
	    (c->status != CONTENT_STATUS_DONE)) {
		/* status is not read or done so nothing to do */
		திரும்பு NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	எனில் (c->textsearch.context != வெற்று) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = வெற்று;
	}

	எனில் (c->handler->close != வெற்று) {
		res = c->handler->close(c);
	} இல்லையெனில் {
		res = NSERROR_OK;
	}
	திரும்பு res;
}


/* exported interface, documented in content/content.h */
வெறுமை content_clear_selection(hlcache_handle *h)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	ப்ரதிஜ்ஞா(c != 0);

	எனில் (c->handler->get_selection != வெற்று)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
எழுத்து * content_get_selection(hlcache_handle *h)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	ப்ரதிஜ்ஞா(c != 0);

	எனில் (c->handler->get_selection != வெற்று)
		திரும்பு c->handler->get_selection(c);
	இல்லையெனில்
		திரும்பு வெற்று;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(கட்டமைப்பு hlcache_handle *h,
			       முழு x, முழு y,
			       கட்டமைப்பு browser_window_features *data)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	ப்ரதிஜ்ஞா(c != 0);

	எனில் (c->handler->get_contextual_content != வெற்று) {
		திரும்பு c->handler->get_contextual_content(c, x, y, data);
	}

	data->object = h;
	திரும்பு NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_point(கட்டமைப்பு hlcache_handle *h,
			முழு x, முழு y,
			முழு scrx, முழு scry)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	ப்ரதிஜ்ஞா(c != 0);

	எனில் (c->handler->scroll_at_point != வெற்று)
		திரும்பு c->handler->scroll_at_point(c, x, y, scrx, scry);

	திரும்பு false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_point(கட்டமைப்பு hlcache_handle *h,
			   முழு x, முழு y,
			   எழுத்து *file)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	ப்ரதிஜ்ஞா(c != 0);

	எனில் (c->handler->drop_file_at_point != வெற்று)
		திரும்பு c->handler->drop_file_at_point(c, x, y, file);

	திரும்பு false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(கட்டமைப்பு hlcache_handle *h, FILE *f, எண்ணிக்கை content_debug op)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	ப்ரதிஜ்ஞா(c != 0);

	எனில் (c->handler->debug_dump == வெற்று) {
		திரும்பு NSERROR_NOT_IMPLEMENTED;
	}

	திரும்பு c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(கட்டமைப்பு hlcache_handle *h, எண்ணிக்கை content_debug op)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);

	எனில் (c == வெற்று) {
		திரும்பு NSERROR_BAD_PARAMETER;
	}

	எனில் (c->handler->debug == வெற்று) {
		திரும்பு NSERROR_NOT_IMPLEMENTED;
	}

	திரும்பு c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
கட்டமைப்பு content_rfc5988_link *
content_find_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);
	கட்டமைப்பு content_rfc5988_link *link = c->links;
	bool rel_match = false;

	போது (link != வெற்று) {
		எனில் (lwc_string_caseless_isequal(link->rel, rel,
						&rel_match) == lwc_error_ok && rel_match) {
			நிறுத்து;
		}
		link = link->next;
	}
	திரும்பு link;
}


/* exported interface documented in content/protected.h */
கட்டமைப்பு content_rfc5988_link *
content__free_rfc5988_link(கட்டமைப்பு content_rfc5988_link *link)
{
	கட்டமைப்பு content_rfc5988_link *next;

	next = link->next;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	விடுவி(link);

	திரும்பு next;
}


/* exported interface documented in content/protected.h */
bool
content__add_rfc5988_link(கட்டமைப்பு content *c,
			  மாறிலி கட்டமைப்பு content_rfc5988_link *link)
{
	கட்டமைப்பு content_rfc5988_link *newlink;
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;

	/* a link relation must be present for it to be a link */
	எனில் (link->rel == வெற்று) {
		திரும்பு false;
	}

	/* a link href must be present for it to be a link */
	எனில் (link->href == வெற்று) {
		திரும்பு false;
	}

	newlink = calloc(1, அளவு(கட்டமைப்பு content_rfc5988_link));
	எனில் (newlink == வெற்று) {
		திரும்பு false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	எனில் (link->hreflang != வெற்று) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	எனில் (link->type != வெற்று) {
		newlink->type = lwc_string_ref(link->type);
	}
	எனில் (link->media != வெற்று) {
		newlink->media = lwc_string_ref(link->media);
	}
	எனில் (link->sizes != வெற்று) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->next = c->links;
	c->links = newlink;

	/* broadcast the data */
	செய்தி_data.rfc5988_link = newlink;
	content_broadcast(c, CONTENT_செய்தி_LINK, &செய்தி_data);

	திரும்பு true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(கட்டமைப்பு content *c)
{
	எனில் (c == வெற்று)
		திரும்பு வெற்று;

	திரும்பு llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);

	எனில் (c == வெற்று)
		திரும்பு CONTENT_NONE;

	திரும்பு c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	திரும்பு content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(கட்டமைப்பு content *c)
{
	எனில் (c == வெற்று)
		திரும்பு வெற்று;

	திரும்பு lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content__set_title(கட்டமைப்பு content *c, மாறிலி எழுத்து *title)
{
	எழுத்து *new_title = strdup(title);
	எனில் (new_title == வெற்று)
		திரும்பு false;

	எனில் (c->title != வெற்று)
		விடுவி(c->title);

	c->title = new_title;

	திரும்பு true;
}


/* exported interface documented in content/content.h */
மாறிலி எழுத்து *content_get_title(hlcache_handle *h)
{
	திரும்பு content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
மாறிலி எழுத்து *content__get_title(கட்டமைப்பு content *c)
{
	எனில் (c == வெற்று)
		திரும்பு வெற்று;

	திரும்பு c->title != வெற்று ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_status content_get_status(hlcache_handle *h)
{
	திரும்பு content__get_status(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_status content__get_status(கட்டமைப்பு content *c)
{
	எனில் (c == வெற்று)
		திரும்பு CONTENT_STATUS_ERROR;

	திரும்பு c->status;
}


/* exported interface documented in content/content.h */
மாறிலி எழுத்து *content_get_status_message(hlcache_handle *h)
{
	திரும்பு content__get_status_message(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
மாறிலி எழுத்து *content__get_status_message(கட்டமைப்பு content *c)
{
	எனில் (c == வெற்று)
		திரும்பு வெற்று;

	திரும்பு c->status_message;
}


/* exported interface documented in content/content.h */
முழு content_get_width(hlcache_handle *h)
{
	திரும்பு content__get_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
முழு content__get_width(கட்டமைப்பு content *c)
{
	எனில் (c == வெற்று)
		திரும்பு 0;

	திரும்பு c->width;
}


/* exported interface documented in content/content.h */
முழு content_get_height(hlcache_handle *h)
{
	திரும்பு content__get_height(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
முழு content__get_height(கட்டமைப்பு content *c)
{
	எனில் (c == வெற்று)
		திரும்பு 0;

	திரும்பு c->height;
}


/* exported interface documented in content/content.h */
முழு content_get_available_width(hlcache_handle *h)
{
	திரும்பு content__get_available_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
முழு content__get_available_width(கட்டமைப்பு content *c)
{
	எனில் (c == வெற்று)
		திரும்பு 0;

	திரும்பு c->available_width;
}


/* exported interface documented in content/content.h */
மாறிலி uint8_t *content_get_source_data(hlcache_handle *h, size_t *size)
{
	திரும்பு content__get_source_data(hlcache_handle_get_content(h), size);
}


/* exported interface documented in content/content_protected.h */
மாறிலி uint8_t *content__get_source_data(கட்டமைப்பு content *c, size_t *size)
{
	ப்ரதிஜ்ஞா(size != வெற்று);

	/** \todo check if the content check should be an assert */
	எனில் (c == வெற்று)
		திரும்பு வெற்று;

	திரும்பு llcache_handle_get_source_data(c->llcache, size);
}


/* exported interface documented in content/content.h */
வெறுமை content_invalidate_reuse_data(hlcache_handle *h)
{
	content__invalidate_reuse_data(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
வெறுமை content__invalidate_reuse_data(கட்டமைப்பு content *c)
{
	எனில் (c == வெற்று || c->llcache == வெற்று)
		திரும்பு;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_data(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	திரும்பு content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(கட்டமைப்பு content *c)
{
	எனில் (c == வெற்று)
		திரும்பு வெற்று;

	திரும்பு c->refresh;
}


/* exported interface documented in content/content.h */
கட்டமைப்பு bitmap *content_get_bitmap(hlcache_handle *h)
{
	திரும்பு content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
கட்டமைப்பு bitmap *content__get_bitmap(கட்டமைப்பு content *c)
{
	கட்டமைப்பு bitmap *bitmap = வெற்று;

	எனில் ((c != வெற்று) &&
	    (c->handler != வெற்று) &&
	    (c->handler->type != வெற்று) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_internal != வெற்று) ) {
		bitmap = c->handler->get_internal(c, வெற்று);
	}

	திரும்பு bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	திரும்பு content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(கட்டமைப்பு content *c)
{
	எனில் ((c != வெற்று) &&
	    (c->handler != வெற்று) &&
	    (c->handler->is_opaque != வெற்று)) {
		திரும்பு c->handler->is_opaque(c);
	}

	திரும்பு false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	கட்டமைப்பு content *c = hlcache_handle_get_content(h);

	எனில் (c == வெற்று)
		திரும்பு false;

	திரும்பு c->quirks;
}


/* exported interface documented in content/content.h */
மாறிலி எழுத்து *
content_get_encoding(hlcache_handle *h, எண்ணிக்கை content_encoding_type op)
{
	திரும்பு content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
மாறிலி எழுத்து *
content__get_encoding(கட்டமைப்பு content *c, எண்ணிக்கை content_encoding_type op)
{
	மாறிலி எழுத்து *encoding = வெற்று;

	எனில் ((c != வெற்று) &&
	    (c->handler != வெற்று) &&
	    (c->handler->get_encoding != வெற்று) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	திரும்பு encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	திரும்பு content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(கட்டமைப்பு content *c)
{
	திரும்பு c->locked;
}


/* exported interface documented in content/content.h */
மாறிலி llcache_handle *content_get_llcache_handle(கட்டமைப்பு content *c)
{
	எனில் (c == வெற்று)
		திரும்பு வெற்று;

	திரும்பு c->llcache;
}


/* exported interface documented in content/protected.h */
கட்டமைப்பு content *content_clone(கட்டமைப்பு content *c)
{
	கட்டமைப்பு content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	எனில் (error != NSERROR_OK)
		திரும்பு வெற்று;

	திரும்பு nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(மாறிலி கட்டமைப்பு content *c, கட்டமைப்பு content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	எனில் (error != NSERROR_OK) {
		திரும்பு error;
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

	எனில் (c->fallback_charset != வெற்று) {
		nc->fallback_charset = strdup(c->fallback_charset);
		எனில் (nc->fallback_charset == வெற்று) {
			திரும்பு NSERROR_NOMEM;
		}
	}

	எனில் (c->refresh != வெற்று) {
		nc->refresh = nsurl_ref(c->refresh);
		எனில் (nc->refresh == வெற்று) {
			திரும்பு NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_time = c->reformat_time;
	nc->size = c->size;

	எனில் (c->title != வெற்று) {
		nc->title = strdup(c->title);
		எனில் (nc->title == வெற்று) {
			திரும்பு NSERROR_NOMEM;
		}
	}

	nc->active = c->active;

	nc->user_list = calloc(1, அளவு(கட்டமைப்பு content_user));
	எனில் (nc->user_list == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	memcpy(&(nc->status_message), &(c->status_message), 120);
	memcpy(&(nc->sub_status), &(c->sub_status), 80);

	nc->locked = c->locked;
	nc->total_size = c->total_size;
	nc->http_code = c->http_code;

	திரும்பு NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(கட்டமைப்பு content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	எனில் (c->handler->stop != வெற்று)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	திரும்பு llcache_handle_abort(c->llcache);
}
