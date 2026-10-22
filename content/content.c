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

#ضمن <stdint.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <nsutils/time.h>

#ضمن "netsurf/inttypes.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/corestrings.h"
#ضمن "netsurf/browser_window.h"
#ضمن "netsurf/bitmap.h"
#ضمن "netsurf/content.h"
#ضمن "desktop/knockout.h"

#ضمن "content/content_protected.h"
#ضمن "content/textsearch.h"
#ضمن "content/content_debug.h"
#ضمن "content/hlcache.h"
#ضمن "content/urldb.h"

#define URL_FMT_SPC "%.140s"

ثابت حرف * ثابت content_status_name[] = {
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
ساكن فراغ content_convert(بنية content *c)
{
	أكد(c);
	أكد(c->status == CONTENT_STATUS_LOADING ||
	       c->status == CONTENT_STATUS_ERROR);

	إذا (c->status != CONTENT_STATUS_LOADING)
		أرجع;

	إذا (c->locked == true)
		أرجع;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	إذا (c->handler->data_complete != عدم) {
		c->locked = true;
		إذا (c->handler->data_complete(c) == false) {
			content_set_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} وإلا {
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
ساكن nserror
content_llcache_callback(llcache_handle *llcache,
			 ثابت llcache_event *event, فراغ *pw)
{
	بنية content *c = pw;
	اتحاد content_رسالة_data رسالة_data;
	nserror error = NSERROR_OK;

	بدل (event->type) {
	حالة LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		اكسر;
	حالة LLCACHE_EVENT_HAD_HEADERS:
		/* Will never happen: handled in hlcache */
		اكسر;
	حالة LLCACHE_EVENT_HAD_DATA:
		إذا (c->handler->process_data != عدم) {
			إذا (c->handler->process_data(c,
						     (ثابت حرف *) event->data.data.buf,
						     event->data.data.len) == false) {
				llcache_handle_abort(c->llcache);
				c->status = CONTENT_STATUS_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		اكسر;
	حالة LLCACHE_EVENT_DONE:
		{
			size_t source_size;

			(فراغ) llcache_handle_get_source_data(llcache, &source_size);

			content_set_status(c, messages_get("Processing"));
			رسالة_data.explicit_status_text = عدم;
			content_broadcast(c, CONTENT_رسالة_STATUS, &رسالة_data);

			content_convert(c);
		}
		اكسر;
	حالة LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->status = CONTENT_STATUS_ERROR;
		رسالة_data.errordata.errorcode = event->data.error.code;
		رسالة_data.errordata.errormsg = event->data.error.رسالة;
		content_broadcast(c, CONTENT_رسالة_ERROR, &رسالة_data);
		اكسر;
	حالة LLCACHE_EVENT_PROGRESS:
		content_set_status(c, event->data.progress_رسالة);
		رسالة_data.explicit_status_text = عدم;
		content_broadcast(c, CONTENT_رسالة_STATUS, &رسالة_data);
		اكسر;
	حالة LLCACHE_EVENT_REDIRECT:
		رسالة_data.redirect.from = event->data.redirect.from;
		رسالة_data.redirect.to = event->data.redirect.to;
		content_broadcast(c, CONTENT_رسالة_REDIRECT, &رسالة_data);
		اكسر;
	}

	أرجع error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
ساكن فراغ content_update_status(بنية content *c)
{
	إذا (c->status == CONTENT_STATUS_LOADING ||
	    c->status == CONTENT_STATUS_READY) {
		/* Not done yet */
		snprintf(c->status_message, حجم (c->status_message),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_status[0] != '\0' ? ", " : " ",
			 c->sub_status);
	} وإلا {
		snprintf(c->status_message, حجم (c->status_message),
			 "%s (%.1fs)", messages_get("Done"),
			 (عائم) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(بنية content *c,
	      ثابت content_handler *handler,
	      lwc_string *imime_type,
	      ثابت بنية http_parameter *params,
	      llcache_handle *llcache,
	      ثابت حرف *fallback_charset,
	      bool quirks)
{
	بنية content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = calloc(1, حجم(بنية content_user));
	إذا (user_sentinel == عدم) {
		أرجع NSERROR_NOMEM;
	}

	إذا (fallback_charset != عدم) {
		c->fallback_charset = strdup(fallback_charset);
		إذا (c->fallback_charset == عدم) {
			حرر(user_sentinel);
			أرجع NSERROR_NOMEM;
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
	c->title = عدم;
	c->active = 0;
	user_sentinel->callback = عدم;
	user_sentinel->pw = عدم;
	user_sentinel->next = عدم;
	c->user_list = user_sentinel;
	c->sub_status[0] = 0;
	c->locked = false;
	c->total_size = 0;
	c->http_code = 0;

	c->textsearch.string = عدم;
	c->textsearch.context = عدم;

	content_set_status(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	إذا (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		أرجع error;
	}

	أرجع NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	بنية content *c = hlcache_handle_get_content(h);

	إذا (c == عدم)
		أرجع false;

	أرجع (c->handler->reformat != عدم);
}


/* exported interface documented in content/protected.h */
فراغ content_set_status(بنية content *c, ثابت حرف *status_message)
{
	size_t len = strlen(status_message);

	إذا (len >= حجم(c->sub_status)) {
		len = حجم(c->sub_status) - 1;
	}
	memcpy(c->sub_status, status_message, len);
	c->sub_status[len] = '\0';

	content_update_status(c);
}


/* exported interface documented in content/protected.h */
فراغ content_set_ready(بنية content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	أكد(c->locked);
	c->locked = false;

	c->status = CONTENT_STATUS_READY;
	content_update_status(c);
	content_broadcast(c, CONTENT_رسالة_READY, عدم);
}


/* exported interface documented in content/protected.h */
فراغ content_set_done(بنية content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->status = CONTENT_STATUS_DONE;
	c->time = now_ms - c->time;
	content_update_status(c);
	content_broadcast(c, CONTENT_رسالة_DONE, عدم);
}


/* exported interface documented in content/protected.h */
فراغ content_set_error(بنية content *c)
{
	c->locked = false;
	c->status = CONTENT_STATUS_ERROR;
}


/* exported interface documented in content/content.h */
فراغ content_reformat(hlcache_handle *h, bool background,
		      صحيح width, صحيح height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  width, height);
}


/* exported interface documented in content/protected.h */
فراغ
content__reformat(بنية content *c, bool background, صحيح width, صحيح height)
{
	اتحاد content_رسالة_data data;
	أكد(c != 0);
	أكد(c->status == CONTENT_STATUS_READY ||
	       c->status == CONTENT_STATUS_DONE);
	أكد(c->locked == false);

	c->available_width = width;
	c->available_height = height;
	إذا (c->handler->reformat != عدم) {

		c->locked = true;
		c->handler->reformat(c, width, height);
		c->locked = false;

		data.background = background;
		content_broadcast(c, CONTENT_رسالة_REFORMAT, &data);
	}
}


/* exported interface documented in content/content.h */
فراغ content_destroy(بنية content *c)
{
	بنية content_rfc5988_link *link;

	أكد(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	أكد(c->locked == false);

	إذا (c->handler->destroy != عدم)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = عدم;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	طالما (link != عدم) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	إذا (c->user_list != عدم) {
		حرر(c->user_list);
	}

	/* free the title */
	إذا (c->title != عدم) {
		حرر(c->title);
	}

	/* free the fallback characterset */
	إذا (c->fallback_charset != عدم) {
		حرر(c->fallback_charset);
	}

	حرر(c);
}


/* exported interface documented in content/content.h */
فراغ
content_mouse_track(hlcache_handle *h,
		    بنية browser_window *bw,
		    browser_mouse_state mouse,
		    صحيح x, صحيح y)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != عدم);

	إذا (c->handler->mouse_track != عدم) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} وإلا {
		اتحاد content_رسالة_data رسالة_data;
		رسالة_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(c, CONTENT_رسالة_POINTER, &رسالة_data);
	}


	أرجع;
}


/* exported interface documented in content/content.h */
فراغ
content_mouse_action(hlcache_handle *h,
		     بنية browser_window *bw,
		     browser_mouse_state mouse,
		     صحيح x, صحيح y)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != عدم);

	إذا (c->handler->mouse_action != عدم)
		c->handler->mouse_action(c, bw, mouse, x, y);

	أرجع;
}


/* exported interface documented in content/content.h */
bool content_keypress(بنية hlcache_handle *h, uint32_t مفتاح)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != عدم);

	إذا (c->handler->keypress != عدم)
		أرجع c->handler->keypress(c, مفتاح);

	أرجع false;
}


/* exported interface documented in content/content.h */
فراغ content_request_redraw(بنية hlcache_handle *h,
			    صحيح x, صحيح y, صحيح width, صحيح height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, width, height);
}


/* exported interface, documented in content/protected.h */
فراغ content__request_redraw(بنية content *c,
			     صحيح x, صحيح y, صحيح width, صحيح height)
{
	اتحاد content_رسالة_data data;

	إذا (c == عدم)
		أرجع;

	data.redraw.x = x;
	data.redraw.y = y;
	data.redraw.width = width;
	data.redraw.height = height;

	content_broadcast(c, CONTENT_رسالة_REDRAW, &data);
}


/* exported interface, documented in content/content.h */
bool content_exec(بنية hlcache_handle *h, ثابت حرف *src, size_t srclen)
{
	بنية content *c = hlcache_handle_get_content(h);

	أكد(c != عدم);

	إذا (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		أرجع false;
	}

	إذا (c->handler->exec == عدم) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		أرجع false;
	}

	أرجع c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(بنية hlcache_handle *h)
{
	بنية content *c = hlcache_handle_get_content(h);
	بنية nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool match;

	/* Is this an internal scheme? If so, we trust here and stop */
	إذا ((lwc_string_isequal(scheme, corestring_lwc_about,
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
		أرجع false;
	}

	/* Okay, not internal, am *I* secure? */
	إذا ((lwc_string_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok)
	    && (match == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		أرجع true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	إذا (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		أرجع true;
	}

	/* Otherwise try and chain through the handler */
	إذا (c != عدم && c->handler->saw_insecure_objects != عدم) {
		أرجع c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	أرجع false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       بنية content_redraw_data *data,
	       ثابت بنية rect *clip,
	       ثابت بنية redraw_context *ctx)
{
	بنية content *c = hlcache_handle_get_content(h);

	أكد(c != عدم);

	إذا (c->locked) {
		/* not safe to attempt redraw */
		أرجع true;
	}

	/* ensure we have a redrawable content */
	إذا (c->handler->redraw == عدم) {
		أرجع true;
	}

	أرجع c->handler->redraw(c, data, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(بنية hlcache_handle *h,
		      صحيح width, صحيح height,
		      ثابت بنية redraw_context *ctx)
{
	بنية content *c = hlcache_handle_get_content(h);
	بنية redraw_context new_ctx = *ctx;
	بنية rect clip;
	بنية content_redraw_data data;
	bool plot_ok = true;

	أكد(c != عدم);

	/* ensure it is safe to attempt redraw */
	إذا (c->locked) {
		أرجع true;
	}

	/* ensure we have a redrawable content */
	إذا (c->handler->redraw == عدم) {
		أرجع true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, width, height, ctx);

	إذا (ctx->plot->option_knockout) {
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
	إذا (c->width) {
		data.scale = (عائم)width / (عائم)c->width;
	} وإلا {
		data.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &data, &clip, &new_ctx);

	إذا (ctx->plot->option_knockout) {
		knockout_plot_end(ctx);
	}

	أرجع plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_add_user(بنية content *c,
		 فراغ (*callback)(
				  بنية content *c,
				  content_رسالة رسالة,
				  ثابت اتحاد content_رسالة_data *data,
				  فراغ *pw),
		 فراغ *pw)
{
	بنية content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = malloc(حجم(بنية content_user));
	إذا (!user)
		أرجع false;
	user->callback = callback;
	user->pw = pw;
	user->next = c->user_list->next;
	c->user_list->next = user;

	إذا (c->handler->add_user != عدم)
		c->handler->add_user(c);

	أرجع true;
}


/* exported interface documented in content/content.h */
فراغ
content_remove_user(بنية content *c,
		    فراغ (*callback)(
				     بنية content *c,
				     content_رسالة رسالة,
				     ثابت اتحاد content_رسالة_data *data,
				     فراغ *pw),
		    فراغ *pw)
{
	بنية content_user *user, *next;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	لكل (user = c->user_list; user->next != 0 &&
		     !(user->next->callback == callback &&
		       user->next->pw == pw); user = user->next)
		;
	إذا (user->next == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		أكد(0);
		أرجع;
	}

	إذا (c->handler->remove_user != عدم)
		c->handler->remove_user(c);

	next = user->next;
	user->next = next->next;
	حرر(next);
}


/* exported interface documented in content/content.h */
uint32_t content_عدد_users(بنية content *c)
{
	بنية content_user *user;
	uint32_t counter = 0;

	أكد(c != عدم);

	لكل (user = c->user_list; user != عدم; user = user->next)
		counter += 1;

	أكد(counter > 0);

	أرجع counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(بنية content *c, bool quirks)
{
	إذا (c->handler->matches_quirks == عدم)
		أرجع true;

	أرجع c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(بنية content *c)
{
	أرجع c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
فراغ content_broadcast(بنية content *c, content_رسالة رسالة,
		       ثابت اتحاد content_رسالة_data *data)
{
	بنية content_user *user, *next;
	أكد(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, رسالة);
	لكل (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		إذا (user->callback != 0)
			user->callback(c, رسالة, data, user->pw);
	}
}


/* exported interface documented in content_protected.h */
فراغ
content_broadcast_error(بنية content *c, nserror errorcode, ثابت حرف *رسالة)
{
	بنية content_user *user, *next;
	اتحاد content_رسالة_data data;

	أكد(c);

	data.errordata.errorcode = errorcode;
	data.errordata.errormsg = رسالة;

	لكل (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		إذا (user->callback != 0) {
			user->callback(c, CONTENT_رسالة_ERROR,
				       &data, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     بنية browser_window *bw,
	     بنية content *page,
	     بنية object_params *params)
{
	بنية content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	أكد(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	إذا (c->handler->open != عدم) {
		res = c->handler->open(c, bw, page, params);
	} وإلا {
		res = NSERROR_OK;
	}
	أرجع res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	بنية content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	إذا (c == عدم) {
		أرجع NSERROR_BAD_PARAMETER;
	}

	إذا ((c->status != CONTENT_STATUS_READY) &&
	    (c->status != CONTENT_STATUS_DONE)) {
		/* status is not read or done so nothing to do */
		أرجع NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	إذا (c->textsearch.context != عدم) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = عدم;
	}

	إذا (c->handler->close != عدم) {
		res = c->handler->close(c);
	} وإلا {
		res = NSERROR_OK;
	}
	أرجع res;
}


/* exported interface, documented in content/content.h */
فراغ content_clear_selection(hlcache_handle *h)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	إذا (c->handler->get_selection != عدم)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
حرف * content_get_selection(hlcache_handle *h)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	إذا (c->handler->get_selection != عدم)
		أرجع c->handler->get_selection(c);
	وإلا
		أرجع عدم;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(بنية hlcache_handle *h,
			       صحيح x, صحيح y,
			       بنية browser_window_features *data)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	إذا (c->handler->get_contextual_content != عدم) {
		أرجع c->handler->get_contextual_content(c, x, y, data);
	}

	data->object = h;
	أرجع NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_point(بنية hlcache_handle *h,
			صحيح x, صحيح y,
			صحيح scrx, صحيح scry)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	إذا (c->handler->scroll_at_point != عدم)
		أرجع c->handler->scroll_at_point(c, x, y, scrx, scry);

	أرجع false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_point(بنية hlcache_handle *h,
			   صحيح x, صحيح y,
			   حرف *file)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	إذا (c->handler->drop_file_at_point != عدم)
		أرجع c->handler->drop_file_at_point(c, x, y, file);

	أرجع false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(بنية hlcache_handle *h, FILE *f, تعداد content_debug op)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	إذا (c->handler->debug_dump == عدم) {
		أرجع NSERROR_NOT_IMPLEMENTED;
	}

	أرجع c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(بنية hlcache_handle *h, تعداد content_debug op)
{
	بنية content *c = hlcache_handle_get_content(h);

	إذا (c == عدم) {
		أرجع NSERROR_BAD_PARAMETER;
	}

	إذا (c->handler->debug == عدم) {
		أرجع NSERROR_NOT_IMPLEMENTED;
	}

	أرجع c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
بنية content_rfc5988_link *
content_find_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	بنية content *c = hlcache_handle_get_content(h);
	بنية content_rfc5988_link *link = c->links;
	bool rel_match = false;

	طالما (link != عدم) {
		إذا (lwc_string_caseless_isequal(link->rel, rel,
						&rel_match) == lwc_error_ok && rel_match) {
			اكسر;
		}
		link = link->next;
	}
	أرجع link;
}


/* exported interface documented in content/protected.h */
بنية content_rfc5988_link *
content__free_rfc5988_link(بنية content_rfc5988_link *link)
{
	بنية content_rfc5988_link *next;

	next = link->next;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	حرر(link);

	أرجع next;
}


/* exported interface documented in content/protected.h */
bool
content__add_rfc5988_link(بنية content *c,
			  ثابت بنية content_rfc5988_link *link)
{
	بنية content_rfc5988_link *newlink;
	اتحاد content_رسالة_data رسالة_data;

	/* a link relation must be present for it to be a link */
	إذا (link->rel == عدم) {
		أرجع false;
	}

	/* a link href must be present for it to be a link */
	إذا (link->href == عدم) {
		أرجع false;
	}

	newlink = calloc(1, حجم(بنية content_rfc5988_link));
	إذا (newlink == عدم) {
		أرجع false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	إذا (link->hreflang != عدم) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	إذا (link->type != عدم) {
		newlink->type = lwc_string_ref(link->type);
	}
	إذا (link->media != عدم) {
		newlink->media = lwc_string_ref(link->media);
	}
	إذا (link->sizes != عدم) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->next = c->links;
	c->links = newlink;

	/* broadcast the data */
	رسالة_data.rfc5988_link = newlink;
	content_broadcast(c, CONTENT_رسالة_LINK, &رسالة_data);

	أرجع true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(بنية content *c)
{
	إذا (c == عدم)
		أرجع عدم;

	أرجع llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	بنية content *c = hlcache_handle_get_content(h);

	إذا (c == عدم)
		أرجع CONTENT_NONE;

	أرجع c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	أرجع content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(بنية content *c)
{
	إذا (c == عدم)
		أرجع عدم;

	أرجع lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content__set_title(بنية content *c, ثابت حرف *title)
{
	حرف *new_title = strdup(title);
	إذا (new_title == عدم)
		أرجع false;

	إذا (c->title != عدم)
		حرر(c->title);

	c->title = new_title;

	أرجع true;
}


/* exported interface documented in content/content.h */
ثابت حرف *content_get_title(hlcache_handle *h)
{
	أرجع content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ثابت حرف *content__get_title(بنية content *c)
{
	إذا (c == عدم)
		أرجع عدم;

	أرجع c->title != عدم ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_status content_get_status(hlcache_handle *h)
{
	أرجع content__get_status(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_status content__get_status(بنية content *c)
{
	إذا (c == عدم)
		أرجع CONTENT_STATUS_ERROR;

	أرجع c->status;
}


/* exported interface documented in content/content.h */
ثابت حرف *content_get_status_message(hlcache_handle *h)
{
	أرجع content__get_status_message(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ثابت حرف *content__get_status_message(بنية content *c)
{
	إذا (c == عدم)
		أرجع عدم;

	أرجع c->status_message;
}


/* exported interface documented in content/content.h */
صحيح content_get_width(hlcache_handle *h)
{
	أرجع content__get_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
صحيح content__get_width(بنية content *c)
{
	إذا (c == عدم)
		أرجع 0;

	أرجع c->width;
}


/* exported interface documented in content/content.h */
صحيح content_get_height(hlcache_handle *h)
{
	أرجع content__get_height(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
صحيح content__get_height(بنية content *c)
{
	إذا (c == عدم)
		أرجع 0;

	أرجع c->height;
}


/* exported interface documented in content/content.h */
صحيح content_get_available_width(hlcache_handle *h)
{
	أرجع content__get_available_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
صحيح content__get_available_width(بنية content *c)
{
	إذا (c == عدم)
		أرجع 0;

	أرجع c->available_width;
}


/* exported interface documented in content/content.h */
ثابت uint8_t *content_get_source_data(hlcache_handle *h, size_t *size)
{
	أرجع content__get_source_data(hlcache_handle_get_content(h), size);
}


/* exported interface documented in content/content_protected.h */
ثابت uint8_t *content__get_source_data(بنية content *c, size_t *size)
{
	أكد(size != عدم);

	/** \todo check if the content check should be an assert */
	إذا (c == عدم)
		أرجع عدم;

	أرجع llcache_handle_get_source_data(c->llcache, size);
}


/* exported interface documented in content/content.h */
فراغ content_invalidate_reuse_data(hlcache_handle *h)
{
	content__invalidate_reuse_data(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
فراغ content__invalidate_reuse_data(بنية content *c)
{
	إذا (c == عدم || c->llcache == عدم)
		أرجع;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_data(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	أرجع content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(بنية content *c)
{
	إذا (c == عدم)
		أرجع عدم;

	أرجع c->refresh;
}


/* exported interface documented in content/content.h */
بنية bitmap *content_get_bitmap(hlcache_handle *h)
{
	أرجع content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
بنية bitmap *content__get_bitmap(بنية content *c)
{
	بنية bitmap *bitmap = عدم;

	إذا ((c != عدم) &&
	    (c->handler != عدم) &&
	    (c->handler->type != عدم) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_internal != عدم) ) {
		bitmap = c->handler->get_internal(c, عدم);
	}

	أرجع bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	أرجع content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(بنية content *c)
{
	إذا ((c != عدم) &&
	    (c->handler != عدم) &&
	    (c->handler->is_opaque != عدم)) {
		أرجع c->handler->is_opaque(c);
	}

	أرجع false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	بنية content *c = hlcache_handle_get_content(h);

	إذا (c == عدم)
		أرجع false;

	أرجع c->quirks;
}


/* exported interface documented in content/content.h */
ثابت حرف *
content_get_encoding(hlcache_handle *h, تعداد content_encoding_type op)
{
	أرجع content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
ثابت حرف *
content__get_encoding(بنية content *c, تعداد content_encoding_type op)
{
	ثابت حرف *encoding = عدم;

	إذا ((c != عدم) &&
	    (c->handler != عدم) &&
	    (c->handler->get_encoding != عدم) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	أرجع encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	أرجع content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(بنية content *c)
{
	أرجع c->locked;
}


/* exported interface documented in content/content.h */
ثابت llcache_handle *content_get_llcache_handle(بنية content *c)
{
	إذا (c == عدم)
		أرجع عدم;

	أرجع c->llcache;
}


/* exported interface documented in content/protected.h */
بنية content *content_clone(بنية content *c)
{
	بنية content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	إذا (error != NSERROR_OK)
		أرجع عدم;

	أرجع nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(ثابت بنية content *c, بنية content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	إذا (error != NSERROR_OK) {
		أرجع error;
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

	إذا (c->fallback_charset != عدم) {
		nc->fallback_charset = strdup(c->fallback_charset);
		إذا (nc->fallback_charset == عدم) {
			أرجع NSERROR_NOMEM;
		}
	}

	إذا (c->refresh != عدم) {
		nc->refresh = nsurl_ref(c->refresh);
		إذا (nc->refresh == عدم) {
			أرجع NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_time = c->reformat_time;
	nc->size = c->size;

	إذا (c->title != عدم) {
		nc->title = strdup(c->title);
		إذا (nc->title == عدم) {
			أرجع NSERROR_NOMEM;
		}
	}

	nc->active = c->active;

	nc->user_list = calloc(1, حجم(بنية content_user));
	إذا (nc->user_list == عدم) {
		أرجع NSERROR_NOMEM;
	}

	memcpy(&(nc->status_message), &(c->status_message), 120);
	memcpy(&(nc->sub_status), &(c->sub_status), 80);

	nc->locked = c->locked;
	nc->total_size = c->total_size;
	nc->http_code = c->http_code;

	أرجع NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(بنية content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	إذا (c->handler->stop != عدم)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	أرجع llcache_handle_abort(c->llcache);
}
