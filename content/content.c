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

#شامل <stdint.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <nsutils/time.h>

#شامل "netsurf/inttypes.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/corestrings.h"
#شامل "netsurf/browser_window.h"
#شامل "netsurf/bitmap.h"
#شامل "netsurf/content.h"
#شامل "desktop/knockout.h"

#شامل "content/content_protected.h"
#شامل "content/textsearch.h"
#شامل "content/content_debug.h"
#شامل "content/hlcache.h"
#شامل "content/urldb.h"

#define URL_FMT_SPC "%.140s"

ثابت نویسه * ثابت content_status_name[] = {
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
ایستا تهی content_convert(ساختار content *c)
{
	أكد(c);
	أكد(c->status == CONTENT_STATUS_LOADING ||
	       c->status == CONTENT_STATUS_ERROR);

	اگر (c->status != CONTENT_STATUS_LOADING)
		بازگشت;

	اگر (c->locked == true)
		بازگشت;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	اگر (c->handler->data_complete != پوچ) {
		c->locked = true;
		اگر (c->handler->data_complete(c) == false) {
			content_set_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} وگرنه {
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
ایستا nserror
content_llcache_callback(llcache_handle *llcache,
			 ثابت llcache_event *event, تهی *pw)
{
	ساختار content *c = pw;
	اجتماع content_پیام_data پیام_data;
	nserror error = NSERROR_OK;

	انتخاب (event->type) {
	حالت LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		بشکن;
	حالت LLCACHE_EVENT_HAD_HEADERS:
		/* Will never happen: handled in hlcache */
		بشکن;
	حالت LLCACHE_EVENT_HAD_DATA:
		اگر (c->handler->process_data != پوچ) {
			اگر (c->handler->process_data(c,
						     (ثابت نویسه *) event->data.data.buf,
						     event->data.data.len) == false) {
				llcache_handle_abort(c->llcache);
				c->status = CONTENT_STATUS_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		بشکن;
	حالت LLCACHE_EVENT_DONE:
		{
			size_t source_size;

			(تهی) llcache_handle_get_source_data(llcache, &source_size);

			content_set_status(c, messages_get("Processing"));
			پیام_data.explicit_status_text = پوچ;
			content_broadcast(c, CONTENT_پیام_STATUS, &پیام_data);

			content_convert(c);
		}
		بشکن;
	حالت LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->status = CONTENT_STATUS_ERROR;
		پیام_data.errordata.errorcode = event->data.error.code;
		پیام_data.errordata.errormsg = event->data.error.پیام;
		content_broadcast(c, CONTENT_پیام_ERROR, &پیام_data);
		بشکن;
	حالت LLCACHE_EVENT_PROGRESS:
		content_set_status(c, event->data.progress_پیام);
		پیام_data.explicit_status_text = پوچ;
		content_broadcast(c, CONTENT_پیام_STATUS, &پیام_data);
		بشکن;
	حالت LLCACHE_EVENT_REDIRECT:
		پیام_data.redirect.from = event->data.redirect.from;
		پیام_data.redirect.to = event->data.redirect.to;
		content_broadcast(c, CONTENT_پیام_REDIRECT, &پیام_data);
		بشکن;
	}

	بازگشت error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
ایستا تهی content_update_status(ساختار content *c)
{
	اگر (c->status == CONTENT_STATUS_LOADING ||
	    c->status == CONTENT_STATUS_READY) {
		/* Not done yet */
		snprintf(c->status_message, اندازه (c->status_message),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_status[0] != '\0' ? ", " : " ",
			 c->sub_status);
	} وگرنه {
		snprintf(c->status_message, اندازه (c->status_message),
			 "%s (%.1fs)", messages_get("Done"),
			 (اعشاری) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(ساختار content *c,
	      ثابت content_handler *handler,
	      lwc_string *imime_type,
	      ثابت ساختار http_parameter *params,
	      llcache_handle *llcache,
	      ثابت نویسه *fallback_charset,
	      bool quirks)
{
	ساختار content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = calloc(1, اندازه(ساختار content_user));
	اگر (user_sentinel == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	اگر (fallback_charset != پوچ) {
		c->fallback_charset = strdup(fallback_charset);
		اگر (c->fallback_charset == پوچ) {
			آزاد(user_sentinel);
			بازگشت NSERROR_NOMEM;
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
	c->title = پوچ;
	c->active = 0;
	user_sentinel->callback = پوچ;
	user_sentinel->pw = پوچ;
	user_sentinel->next = پوچ;
	c->user_list = user_sentinel;
	c->sub_status[0] = 0;
	c->locked = false;
	c->total_size = 0;
	c->http_code = 0;

	c->textsearch.string = پوچ;
	c->textsearch.context = پوچ;

	content_set_status(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	اگر (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		بازگشت error;
	}

	بازگشت NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	ساختار content *c = hlcache_handle_get_content(h);

	اگر (c == پوچ)
		بازگشت false;

	بازگشت (c->handler->reformat != پوچ);
}


/* exported interface documented in content/protected.h */
تهی content_set_status(ساختار content *c, ثابت نویسه *status_message)
{
	size_t len = strlen(status_message);

	اگر (len >= اندازه(c->sub_status)) {
		len = اندازه(c->sub_status) - 1;
	}
	memcpy(c->sub_status, status_message, len);
	c->sub_status[len] = '\0';

	content_update_status(c);
}


/* exported interface documented in content/protected.h */
تهی content_set_ready(ساختار content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	أكد(c->locked);
	c->locked = false;

	c->status = CONTENT_STATUS_READY;
	content_update_status(c);
	content_broadcast(c, CONTENT_پیام_READY, پوچ);
}


/* exported interface documented in content/protected.h */
تهی content_set_done(ساختار content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->status = CONTENT_STATUS_DONE;
	c->time = now_ms - c->time;
	content_update_status(c);
	content_broadcast(c, CONTENT_پیام_DONE, پوچ);
}


/* exported interface documented in content/protected.h */
تهی content_set_error(ساختار content *c)
{
	c->locked = false;
	c->status = CONTENT_STATUS_ERROR;
}


/* exported interface documented in content/content.h */
تهی content_reformat(hlcache_handle *h, bool background,
		      صحیح width, صحیح height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  width, height);
}


/* exported interface documented in content/protected.h */
تهی
content__reformat(ساختار content *c, bool background, صحیح width, صحیح height)
{
	اجتماع content_پیام_data data;
	أكد(c != 0);
	أكد(c->status == CONTENT_STATUS_READY ||
	       c->status == CONTENT_STATUS_DONE);
	أكد(c->locked == false);

	c->available_width = width;
	c->available_height = height;
	اگر (c->handler->reformat != پوچ) {

		c->locked = true;
		c->handler->reformat(c, width, height);
		c->locked = false;

		data.background = background;
		content_broadcast(c, CONTENT_پیام_REFORMAT, &data);
	}
}


/* exported interface documented in content/content.h */
تهی content_destroy(ساختار content *c)
{
	ساختار content_rfc5988_link *link;

	أكد(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	أكد(c->locked == false);

	اگر (c->handler->destroy != پوچ)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = پوچ;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	وقتی (link != پوچ) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	اگر (c->user_list != پوچ) {
		آزاد(c->user_list);
	}

	/* free the title */
	اگر (c->title != پوچ) {
		آزاد(c->title);
	}

	/* free the fallback characterset */
	اگر (c->fallback_charset != پوچ) {
		آزاد(c->fallback_charset);
	}

	آزاد(c);
}


/* exported interface documented in content/content.h */
تهی
content_mouse_track(hlcache_handle *h,
		    ساختار browser_window *bw,
		    browser_mouse_state mouse,
		    صحیح x, صحیح y)
{
	ساختار content *c = hlcache_handle_get_content(h);
	أكد(c != پوچ);

	اگر (c->handler->mouse_track != پوچ) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} وگرنه {
		اجتماع content_پیام_data پیام_data;
		پیام_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(c, CONTENT_پیام_POINTER, &پیام_data);
	}


	بازگشت;
}


/* exported interface documented in content/content.h */
تهی
content_mouse_action(hlcache_handle *h,
		     ساختار browser_window *bw,
		     browser_mouse_state mouse,
		     صحیح x, صحیح y)
{
	ساختار content *c = hlcache_handle_get_content(h);
	أكد(c != پوچ);

	اگر (c->handler->mouse_action != پوچ)
		c->handler->mouse_action(c, bw, mouse, x, y);

	بازگشت;
}


/* exported interface documented in content/content.h */
bool content_keypress(ساختار hlcache_handle *h, uint32_t کلید)
{
	ساختار content *c = hlcache_handle_get_content(h);
	أكد(c != پوچ);

	اگر (c->handler->keypress != پوچ)
		بازگشت c->handler->keypress(c, کلید);

	بازگشت false;
}


/* exported interface documented in content/content.h */
تهی content_request_redraw(ساختار hlcache_handle *h,
			    صحیح x, صحیح y, صحیح width, صحیح height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, width, height);
}


/* exported interface, documented in content/protected.h */
تهی content__request_redraw(ساختار content *c,
			     صحیح x, صحیح y, صحیح width, صحیح height)
{
	اجتماع content_پیام_data data;

	اگر (c == پوچ)
		بازگشت;

	data.redraw.x = x;
	data.redraw.y = y;
	data.redraw.width = width;
	data.redraw.height = height;

	content_broadcast(c, CONTENT_پیام_REDRAW, &data);
}


/* exported interface, documented in content/content.h */
bool content_exec(ساختار hlcache_handle *h, ثابت نویسه *src, size_t srclen)
{
	ساختار content *c = hlcache_handle_get_content(h);

	أكد(c != پوچ);

	اگر (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		بازگشت false;
	}

	اگر (c->handler->exec == پوچ) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		بازگشت false;
	}

	بازگشت c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(ساختار hlcache_handle *h)
{
	ساختار content *c = hlcache_handle_get_content(h);
	ساختار nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool match;

	/* Is this an internal scheme? If so, we trust here and stop */
	اگر ((lwc_string_isequal(scheme, corestring_lwc_about,
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
		بازگشت false;
	}

	/* Okay, not internal, am *I* secure? */
	اگر ((lwc_string_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok)
	    && (match == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		بازگشت true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	اگر (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		بازگشت true;
	}

	/* Otherwise try and chain through the handler */
	اگر (c != پوچ && c->handler->saw_insecure_objects != پوچ) {
		بازگشت c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	بازگشت false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       ساختار content_redraw_data *data,
	       ثابت ساختار rect *clip,
	       ثابت ساختار redraw_context *ctx)
{
	ساختار content *c = hlcache_handle_get_content(h);

	أكد(c != پوچ);

	اگر (c->locked) {
		/* not safe to attempt redraw */
		بازگشت true;
	}

	/* ensure we have a redrawable content */
	اگر (c->handler->redraw == پوچ) {
		بازگشت true;
	}

	بازگشت c->handler->redraw(c, data, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(ساختار hlcache_handle *h,
		      صحیح width, صحیح height,
		      ثابت ساختار redraw_context *ctx)
{
	ساختار content *c = hlcache_handle_get_content(h);
	ساختار redraw_context new_ctx = *ctx;
	ساختار rect clip;
	ساختار content_redraw_data data;
	bool plot_ok = true;

	أكد(c != پوچ);

	/* ensure it is safe to attempt redraw */
	اگر (c->locked) {
		بازگشت true;
	}

	/* ensure we have a redrawable content */
	اگر (c->handler->redraw == پوچ) {
		بازگشت true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, width, height, ctx);

	اگر (ctx->plot->option_knockout) {
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
	اگر (c->width) {
		data.scale = (اعشاری)width / (اعشاری)c->width;
	} وگرنه {
		data.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &data, &clip, &new_ctx);

	اگر (ctx->plot->option_knockout) {
		knockout_plot_end(ctx);
	}

	بازگشت plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_add_user(ساختار content *c,
		 تهی (*callback)(
				  ساختار content *c,
				  content_پیام پیام,
				  ثابت اجتماع content_پیام_data *data,
				  تهی *pw),
		 تهی *pw)
{
	ساختار content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = malloc(اندازه(ساختار content_user));
	اگر (!user)
		بازگشت false;
	user->callback = callback;
	user->pw = pw;
	user->next = c->user_list->next;
	c->user_list->next = user;

	اگر (c->handler->add_user != پوچ)
		c->handler->add_user(c);

	بازگشت true;
}


/* exported interface documented in content/content.h */
تهی
content_remove_user(ساختار content *c,
		    تهی (*callback)(
				     ساختار content *c,
				     content_پیام پیام,
				     ثابت اجتماع content_پیام_data *data,
				     تهی *pw),
		    تهی *pw)
{
	ساختار content_user *user, *next;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	برای (user = c->user_list; user->next != 0 &&
		     !(user->next->callback == callback &&
		       user->next->pw == pw); user = user->next)
		;
	اگر (user->next == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		أكد(0);
		بازگشت;
	}

	اگر (c->handler->remove_user != پوچ)
		c->handler->remove_user(c);

	next = user->next;
	user->next = next->next;
	آزاد(next);
}


/* exported interface documented in content/content.h */
uint32_t content_شمار_users(ساختار content *c)
{
	ساختار content_user *user;
	uint32_t counter = 0;

	أكد(c != پوچ);

	برای (user = c->user_list; user != پوچ; user = user->next)
		counter += 1;

	أكد(counter > 0);

	بازگشت counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(ساختار content *c, bool quirks)
{
	اگر (c->handler->matches_quirks == پوچ)
		بازگشت true;

	بازگشت c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(ساختار content *c)
{
	بازگشت c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
تهی content_broadcast(ساختار content *c, content_پیام پیام,
		       ثابت اجتماع content_پیام_data *data)
{
	ساختار content_user *user, *next;
	أكد(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, پیام);
	برای (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		اگر (user->callback != 0)
			user->callback(c, پیام, data, user->pw);
	}
}


/* exported interface documented in content_protected.h */
تهی
content_broadcast_error(ساختار content *c, nserror errorcode, ثابت نویسه *پیام)
{
	ساختار content_user *user, *next;
	اجتماع content_پیام_data data;

	أكد(c);

	data.errordata.errorcode = errorcode;
	data.errordata.errormsg = پیام;

	برای (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		اگر (user->callback != 0) {
			user->callback(c, CONTENT_پیام_ERROR,
				       &data, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     ساختار browser_window *bw,
	     ساختار content *page,
	     ساختار object_params *params)
{
	ساختار content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	أكد(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	اگر (c->handler->open != پوچ) {
		res = c->handler->open(c, bw, page, params);
	} وگرنه {
		res = NSERROR_OK;
	}
	بازگشت res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	ساختار content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	اگر (c == پوچ) {
		بازگشت NSERROR_BAD_PARAMETER;
	}

	اگر ((c->status != CONTENT_STATUS_READY) &&
	    (c->status != CONTENT_STATUS_DONE)) {
		/* status is not read or done so nothing to do */
		بازگشت NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	اگر (c->textsearch.context != پوچ) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = پوچ;
	}

	اگر (c->handler->close != پوچ) {
		res = c->handler->close(c);
	} وگرنه {
		res = NSERROR_OK;
	}
	بازگشت res;
}


/* exported interface, documented in content/content.h */
تهی content_clear_selection(hlcache_handle *h)
{
	ساختار content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	اگر (c->handler->get_selection != پوچ)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
نویسه * content_get_selection(hlcache_handle *h)
{
	ساختار content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	اگر (c->handler->get_selection != پوچ)
		بازگشت c->handler->get_selection(c);
	وگرنه
		بازگشت پوچ;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(ساختار hlcache_handle *h,
			       صحیح x, صحیح y,
			       ساختار browser_window_features *data)
{
	ساختار content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	اگر (c->handler->get_contextual_content != پوچ) {
		بازگشت c->handler->get_contextual_content(c, x, y, data);
	}

	data->object = h;
	بازگشت NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_point(ساختار hlcache_handle *h,
			صحیح x, صحیح y,
			صحیح scrx, صحیح scry)
{
	ساختار content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	اگر (c->handler->scroll_at_point != پوچ)
		بازگشت c->handler->scroll_at_point(c, x, y, scrx, scry);

	بازگشت false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_point(ساختار hlcache_handle *h,
			   صحیح x, صحیح y,
			   نویسه *file)
{
	ساختار content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	اگر (c->handler->drop_file_at_point != پوچ)
		بازگشت c->handler->drop_file_at_point(c, x, y, file);

	بازگشت false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(ساختار hlcache_handle *h, FILE *f, شمارش content_debug op)
{
	ساختار content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	اگر (c->handler->debug_dump == پوچ) {
		بازگشت NSERROR_NOT_IMPLEMENTED;
	}

	بازگشت c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(ساختار hlcache_handle *h, شمارش content_debug op)
{
	ساختار content *c = hlcache_handle_get_content(h);

	اگر (c == پوچ) {
		بازگشت NSERROR_BAD_PARAMETER;
	}

	اگر (c->handler->debug == پوچ) {
		بازگشت NSERROR_NOT_IMPLEMENTED;
	}

	بازگشت c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
ساختار content_rfc5988_link *
content_find_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	ساختار content *c = hlcache_handle_get_content(h);
	ساختار content_rfc5988_link *link = c->links;
	bool rel_match = false;

	وقتی (link != پوچ) {
		اگر (lwc_string_caseless_isequal(link->rel, rel,
						&rel_match) == lwc_error_ok && rel_match) {
			بشکن;
		}
		link = link->next;
	}
	بازگشت link;
}


/* exported interface documented in content/protected.h */
ساختار content_rfc5988_link *
content__free_rfc5988_link(ساختار content_rfc5988_link *link)
{
	ساختار content_rfc5988_link *next;

	next = link->next;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	آزاد(link);

	بازگشت next;
}


/* exported interface documented in content/protected.h */
bool
content__add_rfc5988_link(ساختار content *c,
			  ثابت ساختار content_rfc5988_link *link)
{
	ساختار content_rfc5988_link *newlink;
	اجتماع content_پیام_data پیام_data;

	/* a link relation must be present for it to be a link */
	اگر (link->rel == پوچ) {
		بازگشت false;
	}

	/* a link href must be present for it to be a link */
	اگر (link->href == پوچ) {
		بازگشت false;
	}

	newlink = calloc(1, اندازه(ساختار content_rfc5988_link));
	اگر (newlink == پوچ) {
		بازگشت false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	اگر (link->hreflang != پوچ) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	اگر (link->type != پوچ) {
		newlink->type = lwc_string_ref(link->type);
	}
	اگر (link->media != پوچ) {
		newlink->media = lwc_string_ref(link->media);
	}
	اگر (link->sizes != پوچ) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->next = c->links;
	c->links = newlink;

	/* broadcast the data */
	پیام_data.rfc5988_link = newlink;
	content_broadcast(c, CONTENT_پیام_LINK, &پیام_data);

	بازگشت true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(ساختار content *c)
{
	اگر (c == پوچ)
		بازگشت پوچ;

	بازگشت llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	ساختار content *c = hlcache_handle_get_content(h);

	اگر (c == پوچ)
		بازگشت CONTENT_NONE;

	بازگشت c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	بازگشت content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(ساختار content *c)
{
	اگر (c == پوچ)
		بازگشت پوچ;

	بازگشت lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content__set_title(ساختار content *c, ثابت نویسه *title)
{
	نویسه *new_title = strdup(title);
	اگر (new_title == پوچ)
		بازگشت false;

	اگر (c->title != پوچ)
		آزاد(c->title);

	c->title = new_title;

	بازگشت true;
}


/* exported interface documented in content/content.h */
ثابت نویسه *content_get_title(hlcache_handle *h)
{
	بازگشت content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ثابت نویسه *content__get_title(ساختار content *c)
{
	اگر (c == پوچ)
		بازگشت پوچ;

	بازگشت c->title != پوچ ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_status content_get_status(hlcache_handle *h)
{
	بازگشت content__get_status(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_status content__get_status(ساختار content *c)
{
	اگر (c == پوچ)
		بازگشت CONTENT_STATUS_ERROR;

	بازگشت c->status;
}


/* exported interface documented in content/content.h */
ثابت نویسه *content_get_status_message(hlcache_handle *h)
{
	بازگشت content__get_status_message(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ثابت نویسه *content__get_status_message(ساختار content *c)
{
	اگر (c == پوچ)
		بازگشت پوچ;

	بازگشت c->status_message;
}


/* exported interface documented in content/content.h */
صحیح content_get_width(hlcache_handle *h)
{
	بازگشت content__get_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
صحیح content__get_width(ساختار content *c)
{
	اگر (c == پوچ)
		بازگشت 0;

	بازگشت c->width;
}


/* exported interface documented in content/content.h */
صحیح content_get_height(hlcache_handle *h)
{
	بازگشت content__get_height(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
صحیح content__get_height(ساختار content *c)
{
	اگر (c == پوچ)
		بازگشت 0;

	بازگشت c->height;
}


/* exported interface documented in content/content.h */
صحیح content_get_available_width(hlcache_handle *h)
{
	بازگشت content__get_available_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
صحیح content__get_available_width(ساختار content *c)
{
	اگر (c == پوچ)
		بازگشت 0;

	بازگشت c->available_width;
}


/* exported interface documented in content/content.h */
ثابت uint8_t *content_get_source_data(hlcache_handle *h, size_t *size)
{
	بازگشت content__get_source_data(hlcache_handle_get_content(h), size);
}


/* exported interface documented in content/content_protected.h */
ثابت uint8_t *content__get_source_data(ساختار content *c, size_t *size)
{
	أكد(size != پوچ);

	/** \todo check if the content check should be an assert */
	اگر (c == پوچ)
		بازگشت پوچ;

	بازگشت llcache_handle_get_source_data(c->llcache, size);
}


/* exported interface documented in content/content.h */
تهی content_invalidate_reuse_data(hlcache_handle *h)
{
	content__invalidate_reuse_data(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
تهی content__invalidate_reuse_data(ساختار content *c)
{
	اگر (c == پوچ || c->llcache == پوچ)
		بازگشت;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_data(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	بازگشت content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(ساختار content *c)
{
	اگر (c == پوچ)
		بازگشت پوچ;

	بازگشت c->refresh;
}


/* exported interface documented in content/content.h */
ساختار bitmap *content_get_bitmap(hlcache_handle *h)
{
	بازگشت content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ساختار bitmap *content__get_bitmap(ساختار content *c)
{
	ساختار bitmap *bitmap = پوچ;

	اگر ((c != پوچ) &&
	    (c->handler != پوچ) &&
	    (c->handler->type != پوچ) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_internal != پوچ) ) {
		bitmap = c->handler->get_internal(c, پوچ);
	}

	بازگشت bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	بازگشت content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(ساختار content *c)
{
	اگر ((c != پوچ) &&
	    (c->handler != پوچ) &&
	    (c->handler->is_opaque != پوچ)) {
		بازگشت c->handler->is_opaque(c);
	}

	بازگشت false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	ساختار content *c = hlcache_handle_get_content(h);

	اگر (c == پوچ)
		بازگشت false;

	بازگشت c->quirks;
}


/* exported interface documented in content/content.h */
ثابت نویسه *
content_get_encoding(hlcache_handle *h, شمارش content_encoding_type op)
{
	بازگشت content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
ثابت نویسه *
content__get_encoding(ساختار content *c, شمارش content_encoding_type op)
{
	ثابت نویسه *encoding = پوچ;

	اگر ((c != پوچ) &&
	    (c->handler != پوچ) &&
	    (c->handler->get_encoding != پوچ) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	بازگشت encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	بازگشت content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(ساختار content *c)
{
	بازگشت c->locked;
}


/* exported interface documented in content/content.h */
ثابت llcache_handle *content_get_llcache_handle(ساختار content *c)
{
	اگر (c == پوچ)
		بازگشت پوچ;

	بازگشت c->llcache;
}


/* exported interface documented in content/protected.h */
ساختار content *content_clone(ساختار content *c)
{
	ساختار content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	اگر (error != NSERROR_OK)
		بازگشت پوچ;

	بازگشت nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(ثابت ساختار content *c, ساختار content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	اگر (error != NSERROR_OK) {
		بازگشت error;
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

	اگر (c->fallback_charset != پوچ) {
		nc->fallback_charset = strdup(c->fallback_charset);
		اگر (nc->fallback_charset == پوچ) {
			بازگشت NSERROR_NOMEM;
		}
	}

	اگر (c->refresh != پوچ) {
		nc->refresh = nsurl_ref(c->refresh);
		اگر (nc->refresh == پوچ) {
			بازگشت NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_time = c->reformat_time;
	nc->size = c->size;

	اگر (c->title != پوچ) {
		nc->title = strdup(c->title);
		اگر (nc->title == پوچ) {
			بازگشت NSERROR_NOMEM;
		}
	}

	nc->active = c->active;

	nc->user_list = calloc(1, اندازه(ساختار content_user));
	اگر (nc->user_list == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	memcpy(&(nc->status_message), &(c->status_message), 120);
	memcpy(&(nc->sub_status), &(c->sub_status), 80);

	nc->locked = c->locked;
	nc->total_size = c->total_size;
	nc->http_code = c->http_code;

	بازگشت NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(ساختار content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	اگر (c->handler->stop != پوچ)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	بازگشت llcache_handle_abort(c->llcache);
}
