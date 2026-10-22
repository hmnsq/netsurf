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

#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <nsutils/time.h>

#અન્તર્ભાવ "netsurf/inttypes.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "netsurf/browser_window.h"
#અન્તર્ભાવ "netsurf/bitmap.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "desktop/knockout.h"

#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/textsearch.h"
#અન્તર્ભાવ "content/content_debug.h"
#અન્તર્ભાવ "content/hlcache.h"
#અન્તર્ભાવ "content/urldb.h"

#define URL_FMT_SPC "%.140s"

અચલ અક્ષર * અચલ content_status_name[] = {
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
સ્થિર શૂન્ય content_convert(સંરચના content *c)
{
	પ્રતિજ્ઞા(c);
	પ્રતિજ્ઞા(c->status == CONTENT_STATUS_LOADING ||
	       c->status == CONTENT_STATUS_ERROR);

	જો (c->status != CONTENT_STATUS_LOADING)
		પાછા;

	જો (c->locked == true)
		પાછા;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	જો (c->handler->data_complete != ખાલી) {
		c->locked = true;
		જો (c->handler->data_complete(c) == false) {
			content_set_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} નહીંતર {
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
સ્થિર nserror
content_llcache_callback(llcache_handle *llcache,
			 અચલ llcache_event *event, શૂન્ય *pw)
{
	સંરચના content *c = pw;
	સંઘ content_સંદેશ_data સંદેશ_data;
	nserror error = NSERROR_OK;

	પસંદગી (event->type) {
	સ્થિતિ LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		તોડો;
	સ્થિતિ LLCACHE_EVENT_HAD_HEADERS:
		/* Will never happen: handled in hlcache */
		તોડો;
	સ્થિતિ LLCACHE_EVENT_HAD_DATA:
		જો (c->handler->process_data != ખાલી) {
			જો (c->handler->process_data(c,
						     (અચલ અક્ષર *) event->data.data.buf,
						     event->data.data.len) == false) {
				llcache_handle_abort(c->llcache);
				c->status = CONTENT_STATUS_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		તોડો;
	સ્થિતિ LLCACHE_EVENT_DONE:
		{
			size_t source_size;

			(શૂન્ય) llcache_handle_get_source_data(llcache, &source_size);

			content_set_status(c, messages_get("Processing"));
			સંદેશ_data.explicit_status_text = ખાલી;
			content_broadcast(c, CONTENT_સંદેશ_STATUS, &સંદેશ_data);

			content_convert(c);
		}
		તોડો;
	સ્થિતિ LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->status = CONTENT_STATUS_ERROR;
		સંદેશ_data.errordata.errorcode = event->data.error.code;
		સંદેશ_data.errordata.errormsg = event->data.error.સંદેશ;
		content_broadcast(c, CONTENT_સંદેશ_ERROR, &સંદેશ_data);
		તોડો;
	સ્થિતિ LLCACHE_EVENT_PROGRESS:
		content_set_status(c, event->data.progress_સંદેશ);
		સંદેશ_data.explicit_status_text = ખાલી;
		content_broadcast(c, CONTENT_સંદેશ_STATUS, &સંદેશ_data);
		તોડો;
	સ્થિતિ LLCACHE_EVENT_REDIRECT:
		સંદેશ_data.redirect.from = event->data.redirect.from;
		સંદેશ_data.redirect.to = event->data.redirect.to;
		content_broadcast(c, CONTENT_સંદેશ_REDIRECT, &સંદેશ_data);
		તોડો;
	}

	પાછા error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
સ્થિર શૂન્ય content_update_status(સંરચના content *c)
{
	જો (c->status == CONTENT_STATUS_LOADING ||
	    c->status == CONTENT_STATUS_READY) {
		/* Not done yet */
		snprintf(c->status_message, કદ (c->status_message),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_status[0] != '\0' ? ", " : " ",
			 c->sub_status);
	} નહીંતર {
		snprintf(c->status_message, કદ (c->status_message),
			 "%s (%.1fs)", messages_get("Done"),
			 (દશાંશ) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(સંરચના content *c,
	      અચલ content_handler *handler,
	      lwc_string *imime_type,
	      અચલ સંરચના http_parameter *params,
	      llcache_handle *llcache,
	      અચલ અક્ષર *fallback_charset,
	      bool quirks)
{
	સંરચના content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = calloc(1, કદ(સંરચના content_user));
	જો (user_sentinel == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	જો (fallback_charset != ખાલી) {
		c->fallback_charset = strdup(fallback_charset);
		જો (c->fallback_charset == ખાલી) {
			મુક્ત(user_sentinel);
			પાછા NSERROR_NOMEM;
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
	c->title = ખાલી;
	c->active = 0;
	user_sentinel->callback = ખાલી;
	user_sentinel->pw = ખાલી;
	user_sentinel->next = ખાલી;
	c->user_list = user_sentinel;
	c->sub_status[0] = 0;
	c->locked = false;
	c->total_size = 0;
	c->http_code = 0;

	c->textsearch.string = ખાલી;
	c->textsearch.context = ખાલી;

	content_set_status(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	જો (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		પાછા error;
	}

	પાછા NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	સંરચના content *c = hlcache_handle_get_content(h);

	જો (c == ખાલી)
		પાછા false;

	પાછા (c->handler->reformat != ખાલી);
}


/* exported interface documented in content/protected.h */
શૂન્ય content_set_status(સંરચના content *c, અચલ અક્ષર *status_message)
{
	size_t len = strlen(status_message);

	જો (len >= કદ(c->sub_status)) {
		len = કદ(c->sub_status) - 1;
	}
	memcpy(c->sub_status, status_message, len);
	c->sub_status[len] = '\0';

	content_update_status(c);
}


/* exported interface documented in content/protected.h */
શૂન્ય content_set_ready(સંરચના content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	પ્રતિજ્ઞા(c->locked);
	c->locked = false;

	c->status = CONTENT_STATUS_READY;
	content_update_status(c);
	content_broadcast(c, CONTENT_સંદેશ_READY, ખાલી);
}


/* exported interface documented in content/protected.h */
શૂન્ય content_set_done(સંરચના content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->status = CONTENT_STATUS_DONE;
	c->time = now_ms - c->time;
	content_update_status(c);
	content_broadcast(c, CONTENT_સંદેશ_DONE, ખાલી);
}


/* exported interface documented in content/protected.h */
શૂન્ય content_set_error(સંરચના content *c)
{
	c->locked = false;
	c->status = CONTENT_STATUS_ERROR;
}


/* exported interface documented in content/content.h */
શૂન્ય content_reformat(hlcache_handle *h, bool background,
		      પૂર્ણાંક width, પૂર્ણાંક height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  width, height);
}


/* exported interface documented in content/protected.h */
શૂન્ય
content__reformat(સંરચના content *c, bool background, પૂર્ણાંક width, પૂર્ણાંક height)
{
	સંઘ content_સંદેશ_data data;
	પ્રતિજ્ઞા(c != 0);
	પ્રતિજ્ઞા(c->status == CONTENT_STATUS_READY ||
	       c->status == CONTENT_STATUS_DONE);
	પ્રતિજ્ઞા(c->locked == false);

	c->available_width = width;
	c->available_height = height;
	જો (c->handler->reformat != ખાલી) {

		c->locked = true;
		c->handler->reformat(c, width, height);
		c->locked = false;

		data.background = background;
		content_broadcast(c, CONTENT_સંદેશ_REFORMAT, &data);
	}
}


/* exported interface documented in content/content.h */
શૂન્ય content_destroy(સંરચના content *c)
{
	સંરચના content_rfc5988_link *link;

	પ્રતિજ્ઞા(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	પ્રતિજ્ઞા(c->locked == false);

	જો (c->handler->destroy != ખાલી)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = ખાલી;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	જ્યારે (link != ખાલી) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	જો (c->user_list != ખાલી) {
		મુક્ત(c->user_list);
	}

	/* free the title */
	જો (c->title != ખાલી) {
		મુક્ત(c->title);
	}

	/* free the fallback characterset */
	જો (c->fallback_charset != ખાલી) {
		મુક્ત(c->fallback_charset);
	}

	મુક્ત(c);
}


/* exported interface documented in content/content.h */
શૂન્ય
content_mouse_track(hlcache_handle *h,
		    સંરચના browser_window *bw,
		    browser_mouse_state mouse,
		    પૂર્ણાંક x, પૂર્ણાંક y)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	પ્રતિજ્ઞા(c != ખાલી);

	જો (c->handler->mouse_track != ખાલી) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} નહીંતર {
		સંઘ content_સંદેશ_data સંદેશ_data;
		સંદેશ_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(c, CONTENT_સંદેશ_POINTER, &સંદેશ_data);
	}


	પાછા;
}


/* exported interface documented in content/content.h */
શૂન્ય
content_mouse_action(hlcache_handle *h,
		     સંરચના browser_window *bw,
		     browser_mouse_state mouse,
		     પૂર્ણાંક x, પૂર્ણાંક y)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	પ્રતિજ્ઞા(c != ખાલી);

	જો (c->handler->mouse_action != ખાલી)
		c->handler->mouse_action(c, bw, mouse, x, y);

	પાછા;
}


/* exported interface documented in content/content.h */
bool content_keypress(સંરચના hlcache_handle *h, uint32_t ચાવી)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	પ્રતિજ્ઞા(c != ખાલી);

	જો (c->handler->keypress != ખાલી)
		પાછા c->handler->keypress(c, ચાવી);

	પાછા false;
}


/* exported interface documented in content/content.h */
શૂન્ય content_request_redraw(સંરચના hlcache_handle *h,
			    પૂર્ણાંક x, પૂર્ણાંક y, પૂર્ણાંક width, પૂર્ણાંક height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, width, height);
}


/* exported interface, documented in content/protected.h */
શૂન્ય content__request_redraw(સંરચના content *c,
			     પૂર્ણાંક x, પૂર્ણાંક y, પૂર્ણાંક width, પૂર્ણાંક height)
{
	સંઘ content_સંદેશ_data data;

	જો (c == ખાલી)
		પાછા;

	data.redraw.x = x;
	data.redraw.y = y;
	data.redraw.width = width;
	data.redraw.height = height;

	content_broadcast(c, CONTENT_સંદેશ_REDRAW, &data);
}


/* exported interface, documented in content/content.h */
bool content_exec(સંરચના hlcache_handle *h, અચલ અક્ષર *src, size_t srclen)
{
	સંરચના content *c = hlcache_handle_get_content(h);

	પ્રતિજ્ઞા(c != ખાલી);

	જો (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		પાછા false;
	}

	જો (c->handler->exec == ખાલી) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		પાછા false;
	}

	પાછા c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(સંરચના hlcache_handle *h)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	સંરચના nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool match;

	/* Is this an internal scheme? If so, we trust here and stop */
	જો ((lwc_string_isequal(scheme, corestring_lwc_about,
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
		પાછા false;
	}

	/* Okay, not internal, am *I* secure? */
	જો ((lwc_string_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok)
	    && (match == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		પાછા true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	જો (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		પાછા true;
	}

	/* Otherwise try and chain through the handler */
	જો (c != ખાલી && c->handler->saw_insecure_objects != ખાલી) {
		પાછા c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	પાછા false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       સંરચના content_redraw_data *data,
	       અચલ સંરચના rect *clip,
	       અચલ સંરચના redraw_context *ctx)
{
	સંરચના content *c = hlcache_handle_get_content(h);

	પ્રતિજ્ઞા(c != ખાલી);

	જો (c->locked) {
		/* not safe to attempt redraw */
		પાછા true;
	}

	/* ensure we have a redrawable content */
	જો (c->handler->redraw == ખાલી) {
		પાછા true;
	}

	પાછા c->handler->redraw(c, data, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(સંરચના hlcache_handle *h,
		      પૂર્ણાંક width, પૂર્ણાંક height,
		      અચલ સંરચના redraw_context *ctx)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	સંરચના redraw_context new_ctx = *ctx;
	સંરચના rect clip;
	સંરચના content_redraw_data data;
	bool plot_ok = true;

	પ્રતિજ્ઞા(c != ખાલી);

	/* ensure it is safe to attempt redraw */
	જો (c->locked) {
		પાછા true;
	}

	/* ensure we have a redrawable content */
	જો (c->handler->redraw == ખાલી) {
		પાછા true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, width, height, ctx);

	જો (ctx->plot->option_knockout) {
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
	જો (c->width) {
		data.scale = (દશાંશ)width / (દશાંશ)c->width;
	} નહીંતર {
		data.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &data, &clip, &new_ctx);

	જો (ctx->plot->option_knockout) {
		knockout_plot_end(ctx);
	}

	પાછા plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_add_user(સંરચના content *c,
		 શૂન્ય (*callback)(
				  સંરચના content *c,
				  content_સંદેશ સંદેશ,
				  અચલ સંઘ content_સંદેશ_data *data,
				  શૂન્ય *pw),
		 શૂન્ય *pw)
{
	સંરચના content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = malloc(કદ(સંરચના content_user));
	જો (!user)
		પાછા false;
	user->callback = callback;
	user->pw = pw;
	user->next = c->user_list->next;
	c->user_list->next = user;

	જો (c->handler->add_user != ખાલી)
		c->handler->add_user(c);

	પાછા true;
}


/* exported interface documented in content/content.h */
શૂન્ય
content_remove_user(સંરચના content *c,
		    શૂન્ય (*callback)(
				     સંરચના content *c,
				     content_સંદેશ સંદેશ,
				     અચલ સંઘ content_સંદેશ_data *data,
				     શૂન્ય *pw),
		    શૂન્ય *pw)
{
	સંરચના content_user *user, *next;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	માટે (user = c->user_list; user->next != 0 &&
		     !(user->next->callback == callback &&
		       user->next->pw == pw); user = user->next)
		;
	જો (user->next == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		પ્રતિજ્ઞા(0);
		પાછા;
	}

	જો (c->handler->remove_user != ખાલી)
		c->handler->remove_user(c);

	next = user->next;
	user->next = next->next;
	મુક્ત(next);
}


/* exported interface documented in content/content.h */
uint32_t content_ગણતરી_users(સંરચના content *c)
{
	સંરચના content_user *user;
	uint32_t counter = 0;

	પ્રતિજ્ઞા(c != ખાલી);

	માટે (user = c->user_list; user != ખાલી; user = user->next)
		counter += 1;

	પ્રતિજ્ઞા(counter > 0);

	પાછા counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(સંરચના content *c, bool quirks)
{
	જો (c->handler->matches_quirks == ખાલી)
		પાછા true;

	પાછા c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(સંરચના content *c)
{
	પાછા c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
શૂન્ય content_broadcast(સંરચના content *c, content_સંદેશ સંદેશ,
		       અચલ સંઘ content_સંદેશ_data *data)
{
	સંરચના content_user *user, *next;
	પ્રતિજ્ઞા(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, સંદેશ);
	માટે (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		જો (user->callback != 0)
			user->callback(c, સંદેશ, data, user->pw);
	}
}


/* exported interface documented in content_protected.h */
શૂન્ય
content_broadcast_error(સંરચના content *c, nserror errorcode, અચલ અક્ષર *સંદેશ)
{
	સંરચના content_user *user, *next;
	સંઘ content_સંદેશ_data data;

	પ્રતિજ્ઞા(c);

	data.errordata.errorcode = errorcode;
	data.errordata.errormsg = સંદેશ;

	માટે (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		જો (user->callback != 0) {
			user->callback(c, CONTENT_સંદેશ_ERROR,
				       &data, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     સંરચના browser_window *bw,
	     સંરચના content *page,
	     સંરચના object_params *params)
{
	સંરચના content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	પ્રતિજ્ઞા(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	જો (c->handler->open != ખાલી) {
		res = c->handler->open(c, bw, page, params);
	} નહીંતર {
		res = NSERROR_OK;
	}
	પાછા res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	સંરચના content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	જો (c == ખાલી) {
		પાછા NSERROR_BAD_PARAMETER;
	}

	જો ((c->status != CONTENT_STATUS_READY) &&
	    (c->status != CONTENT_STATUS_DONE)) {
		/* status is not read or done so nothing to do */
		પાછા NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	જો (c->textsearch.context != ખાલી) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ખાલી;
	}

	જો (c->handler->close != ખાલી) {
		res = c->handler->close(c);
	} નહીંતર {
		res = NSERROR_OK;
	}
	પાછા res;
}


/* exported interface, documented in content/content.h */
શૂન્ય content_clear_selection(hlcache_handle *h)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	પ્રતિજ્ઞા(c != 0);

	જો (c->handler->get_selection != ખાલી)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
અક્ષર * content_get_selection(hlcache_handle *h)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	પ્રતિજ્ઞા(c != 0);

	જો (c->handler->get_selection != ખાલી)
		પાછા c->handler->get_selection(c);
	નહીંતર
		પાછા ખાલી;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(સંરચના hlcache_handle *h,
			       પૂર્ણાંક x, પૂર્ણાંક y,
			       સંરચના browser_window_features *data)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	પ્રતિજ્ઞા(c != 0);

	જો (c->handler->get_contextual_content != ખાલી) {
		પાછા c->handler->get_contextual_content(c, x, y, data);
	}

	data->object = h;
	પાછા NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_point(સંરચના hlcache_handle *h,
			પૂર્ણાંક x, પૂર્ણાંક y,
			પૂર્ણાંક scrx, પૂર્ણાંક scry)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	પ્રતિજ્ઞા(c != 0);

	જો (c->handler->scroll_at_point != ખાલી)
		પાછા c->handler->scroll_at_point(c, x, y, scrx, scry);

	પાછા false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_point(સંરચના hlcache_handle *h,
			   પૂર્ણાંક x, પૂર્ણાંક y,
			   અક્ષર *file)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	પ્રતિજ્ઞા(c != 0);

	જો (c->handler->drop_file_at_point != ખાલી)
		પાછા c->handler->drop_file_at_point(c, x, y, file);

	પાછા false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(સંરચના hlcache_handle *h, FILE *f, ગણના content_debug op)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	પ્રતિજ્ઞા(c != 0);

	જો (c->handler->debug_dump == ખાલી) {
		પાછા NSERROR_NOT_IMPLEMENTED;
	}

	પાછા c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(સંરચના hlcache_handle *h, ગણના content_debug op)
{
	સંરચના content *c = hlcache_handle_get_content(h);

	જો (c == ખાલી) {
		પાછા NSERROR_BAD_PARAMETER;
	}

	જો (c->handler->debug == ખાલી) {
		પાછા NSERROR_NOT_IMPLEMENTED;
	}

	પાછા c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
સંરચના content_rfc5988_link *
content_find_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	સંરચના content *c = hlcache_handle_get_content(h);
	સંરચના content_rfc5988_link *link = c->links;
	bool rel_match = false;

	જ્યારે (link != ખાલી) {
		જો (lwc_string_caseless_isequal(link->rel, rel,
						&rel_match) == lwc_error_ok && rel_match) {
			તોડો;
		}
		link = link->next;
	}
	પાછા link;
}


/* exported interface documented in content/protected.h */
સંરચના content_rfc5988_link *
content__free_rfc5988_link(સંરચના content_rfc5988_link *link)
{
	સંરચના content_rfc5988_link *next;

	next = link->next;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	મુક્ત(link);

	પાછા next;
}


/* exported interface documented in content/protected.h */
bool
content__add_rfc5988_link(સંરચના content *c,
			  અચલ સંરચના content_rfc5988_link *link)
{
	સંરચના content_rfc5988_link *newlink;
	સંઘ content_સંદેશ_data સંદેશ_data;

	/* a link relation must be present for it to be a link */
	જો (link->rel == ખાલી) {
		પાછા false;
	}

	/* a link href must be present for it to be a link */
	જો (link->href == ખાલી) {
		પાછા false;
	}

	newlink = calloc(1, કદ(સંરચના content_rfc5988_link));
	જો (newlink == ખાલી) {
		પાછા false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	જો (link->hreflang != ખાલી) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	જો (link->type != ખાલી) {
		newlink->type = lwc_string_ref(link->type);
	}
	જો (link->media != ખાલી) {
		newlink->media = lwc_string_ref(link->media);
	}
	જો (link->sizes != ખાલી) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->next = c->links;
	c->links = newlink;

	/* broadcast the data */
	સંદેશ_data.rfc5988_link = newlink;
	content_broadcast(c, CONTENT_સંદેશ_LINK, &સંદેશ_data);

	પાછા true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(સંરચના content *c)
{
	જો (c == ખાલી)
		પાછા ખાલી;

	પાછા llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	સંરચના content *c = hlcache_handle_get_content(h);

	જો (c == ખાલી)
		પાછા CONTENT_NONE;

	પાછા c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	પાછા content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(સંરચના content *c)
{
	જો (c == ખાલી)
		પાછા ખાલી;

	પાછા lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content__set_title(સંરચના content *c, અચલ અક્ષર *title)
{
	અક્ષર *new_title = strdup(title);
	જો (new_title == ખાલી)
		પાછા false;

	જો (c->title != ખાલી)
		મુક્ત(c->title);

	c->title = new_title;

	પાછા true;
}


/* exported interface documented in content/content.h */
અચલ અક્ષર *content_get_title(hlcache_handle *h)
{
	પાછા content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
અચલ અક્ષર *content__get_title(સંરચના content *c)
{
	જો (c == ખાલી)
		પાછા ખાલી;

	પાછા c->title != ખાલી ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_status content_get_status(hlcache_handle *h)
{
	પાછા content__get_status(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_status content__get_status(સંરચના content *c)
{
	જો (c == ખાલી)
		પાછા CONTENT_STATUS_ERROR;

	પાછા c->status;
}


/* exported interface documented in content/content.h */
અચલ અક્ષર *content_get_status_message(hlcache_handle *h)
{
	પાછા content__get_status_message(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
અચલ અક્ષર *content__get_status_message(સંરચના content *c)
{
	જો (c == ખાલી)
		પાછા ખાલી;

	પાછા c->status_message;
}


/* exported interface documented in content/content.h */
પૂર્ણાંક content_get_width(hlcache_handle *h)
{
	પાછા content__get_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
પૂર્ણાંક content__get_width(સંરચના content *c)
{
	જો (c == ખાલી)
		પાછા 0;

	પાછા c->width;
}


/* exported interface documented in content/content.h */
પૂર્ણાંક content_get_height(hlcache_handle *h)
{
	પાછા content__get_height(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
પૂર્ણાંક content__get_height(સંરચના content *c)
{
	જો (c == ખાલી)
		પાછા 0;

	પાછા c->height;
}


/* exported interface documented in content/content.h */
પૂર્ણાંક content_get_available_width(hlcache_handle *h)
{
	પાછા content__get_available_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
પૂર્ણાંક content__get_available_width(સંરચના content *c)
{
	જો (c == ખાલી)
		પાછા 0;

	પાછા c->available_width;
}


/* exported interface documented in content/content.h */
અચલ uint8_t *content_get_source_data(hlcache_handle *h, size_t *size)
{
	પાછા content__get_source_data(hlcache_handle_get_content(h), size);
}


/* exported interface documented in content/content_protected.h */
અચલ uint8_t *content__get_source_data(સંરચના content *c, size_t *size)
{
	પ્રતિજ્ઞા(size != ખાલી);

	/** \todo check if the content check should be an assert */
	જો (c == ખાલી)
		પાછા ખાલી;

	પાછા llcache_handle_get_source_data(c->llcache, size);
}


/* exported interface documented in content/content.h */
શૂન્ય content_invalidate_reuse_data(hlcache_handle *h)
{
	content__invalidate_reuse_data(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
શૂન્ય content__invalidate_reuse_data(સંરચના content *c)
{
	જો (c == ખાલી || c->llcache == ખાલી)
		પાછા;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_data(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	પાછા content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(સંરચના content *c)
{
	જો (c == ખાલી)
		પાછા ખાલી;

	પાછા c->refresh;
}


/* exported interface documented in content/content.h */
સંરચના bitmap *content_get_bitmap(hlcache_handle *h)
{
	પાછા content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
સંરચના bitmap *content__get_bitmap(સંરચના content *c)
{
	સંરચના bitmap *bitmap = ખાલી;

	જો ((c != ખાલી) &&
	    (c->handler != ખાલી) &&
	    (c->handler->type != ખાલી) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_internal != ખાલી) ) {
		bitmap = c->handler->get_internal(c, ખાલી);
	}

	પાછા bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	પાછા content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(સંરચના content *c)
{
	જો ((c != ખાલી) &&
	    (c->handler != ખાલી) &&
	    (c->handler->is_opaque != ખાલી)) {
		પાછા c->handler->is_opaque(c);
	}

	પાછા false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	સંરચના content *c = hlcache_handle_get_content(h);

	જો (c == ખાલી)
		પાછા false;

	પાછા c->quirks;
}


/* exported interface documented in content/content.h */
અચલ અક્ષર *
content_get_encoding(hlcache_handle *h, ગણના content_encoding_type op)
{
	પાછા content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
અચલ અક્ષર *
content__get_encoding(સંરચના content *c, ગણના content_encoding_type op)
{
	અચલ અક્ષર *encoding = ખાલી;

	જો ((c != ખાલી) &&
	    (c->handler != ખાલી) &&
	    (c->handler->get_encoding != ખાલી) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	પાછા encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	પાછા content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(સંરચના content *c)
{
	પાછા c->locked;
}


/* exported interface documented in content/content.h */
અચલ llcache_handle *content_get_llcache_handle(સંરચના content *c)
{
	જો (c == ખાલી)
		પાછા ખાલી;

	પાછા c->llcache;
}


/* exported interface documented in content/protected.h */
સંરચના content *content_clone(સંરચના content *c)
{
	સંરચના content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	જો (error != NSERROR_OK)
		પાછા ખાલી;

	પાછા nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(અચલ સંરચના content *c, સંરચના content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	જો (error != NSERROR_OK) {
		પાછા error;
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

	જો (c->fallback_charset != ખાલી) {
		nc->fallback_charset = strdup(c->fallback_charset);
		જો (nc->fallback_charset == ખાલી) {
			પાછા NSERROR_NOMEM;
		}
	}

	જો (c->refresh != ખાલી) {
		nc->refresh = nsurl_ref(c->refresh);
		જો (nc->refresh == ખાલી) {
			પાછા NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_time = c->reformat_time;
	nc->size = c->size;

	જો (c->title != ખાલી) {
		nc->title = strdup(c->title);
		જો (nc->title == ખાલી) {
			પાછા NSERROR_NOMEM;
		}
	}

	nc->active = c->active;

	nc->user_list = calloc(1, કદ(સંરચના content_user));
	જો (nc->user_list == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	memcpy(&(nc->status_message), &(c->status_message), 120);
	memcpy(&(nc->sub_status), &(c->sub_status), 80);

	nc->locked = c->locked;
	nc->total_size = c->total_size;
	nc->http_code = c->http_code;

	પાછા NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(સંરચના content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	જો (c->handler->stop != ખાલી)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	પાછા llcache_handle_abort(c->llcache);
}
