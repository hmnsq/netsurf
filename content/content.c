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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <nsutils/time.h>

#include "netsurf/inttypes.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/corestrings.h"
#include "netsurf/browser_window.h"
#include "netsurf/bitmap.h"
#include "netsurf/content.h"
#include "desktop/knockout.h"

#include "content/content_protected.h"
#include "content/textsearch.h"
#include "content/content_debug.h"
#include "content/hlcache.h"
#include "content/urldb.h"

#define URL_FMT_SPC "%.140s"

const var_na * const content_status_name[] = {
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
static void content_convert(sa_nracanaa content *c)
{
	assert(c);
	assert(c->status == CONTENT_STATUS_LOADING ||
	       c->status == CONTENT_STATUS_ERROR);

	ᮚᮦᮔ᮪ (c->status != CONTENT_STATUS_LOADING)
		ᮝᮜᮤ;

	ᮚᮦᮔ᮪ (c->locked == true)
		ᮝᮜᮤ;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	ᮚᮦᮔ᮪ (c->handler->data_complete != NULL) {
		c->locked = true;
		ᮚᮦᮔ᮪ (c->handler->data_complete(c) == false) {
			content_set_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} ᮜᮤᮚ {
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
static nserror
content_llcache_callback(llcache_handle *llcache,
			 const llcache_event *event, void *pw)
{
	sa_nracanaa content *c = pw;
	sa_ngha content_msg_data msg_data;
	nserror error = NSERROR_OK;

	switch (event->type) {
	pak_sa LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		bha_nga;
	pak_sa LLCACHE_EVENT_HAD_HEADERS:
		/* Will never happen: handled in hlcache */
		bha_nga;
	pak_sa LLCACHE_EVENT_HAD_DATA:
		ᮚᮦᮔ᮪ (c->handler->process_data != NULL) {
			ᮚᮦᮔ᮪ (c->handler->process_data(c,
						     (const var_na *) event->data.data.buf,
						     event->data.data.len) == false) {
				llcache_handle_abort(c->llcache);
				c->status = CONTENT_STATUS_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		bha_nga;
	pak_sa LLCACHE_EVENT_DONE:
		{
			size_t source_size;

			(void) llcache_handle_get_source_data(llcache, &source_size);

			content_set_status(c, messages_get("Processing"));
			msg_data.explicit_status_text = NULL;
			content_broadcast(c, CONTENT_MSG_STATUS, &msg_data);

			content_convert(c);
		}
		bha_nga;
	pak_sa LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->status = CONTENT_STATUS_ERROR;
		msg_data.errordata.errorcode = event->data.error.code;
		msg_data.errordata.errormsg = event->data.error.msg;
		content_broadcast(c, CONTENT_MSG_ERROR, &msg_data);
		bha_nga;
	pak_sa LLCACHE_EVENT_PROGRESS:
		content_set_status(c, event->data.progress_msg);
		msg_data.explicit_status_text = NULL;
		content_broadcast(c, CONTENT_MSG_STATUS, &msg_data);
		bha_nga;
	pak_sa LLCACHE_EVENT_REDIRECT:
		msg_data.redirect.from = event->data.redirect.from;
		msg_data.redirect.to = event->data.redirect.to;
		content_broadcast(c, CONTENT_MSG_REDIRECT, &msg_data);
		bha_nga;
	}

	ᮝᮜᮤ error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
static void content_update_status(sa_nracanaa content *c)
{
	ᮚᮦᮔ᮪ (c->status == CONTENT_STATUS_LOADING ||
	    c->status == CONTENT_STATUS_READY) {
		/* Not done yet */
		snprintf(c->status_message, parimaa_na (c->status_message),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_status[0] != '\0' ? ", " : " ",
			 c->sub_status);
	} ᮜᮤᮚ {
		snprintf(c->status_message, parimaa_na (c->status_message),
			 "%s (%.1fs)", messages_get("Done"),
			 (float) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(sa_nracanaa content *c,
	      const content_handler *handler,
	      lwc_string *imime_type,
	      const sa_nracanaa http_parameter *params,
	      llcache_handle *llcache,
	      const var_na *fallback_charset,
	      bool quirks)
{
	sa_nracanaa content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = calloc(1, parimaa_na(sa_nracanaa content_user));
	ᮚᮦᮔ᮪ (user_sentinel == NULL) {
		ᮝᮜᮤ NSERROR_NOMEM;
	}

	ᮚᮦᮔ᮪ (fallback_charset != NULL) {
		c->fallback_charset = strdup(fallback_charset);
		ᮚᮦᮔ᮪ (c->fallback_charset == NULL) {
			free(user_sentinel);
			ᮝᮜᮤ NSERROR_NOMEM;
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
	c->title = NULL;
	c->active = 0;
	user_sentinel->callback = NULL;
	user_sentinel->pw = NULL;
	user_sentinel->next = NULL;
	c->user_list = user_sentinel;
	c->sub_status[0] = 0;
	c->locked = false;
	c->total_size = 0;
	c->http_code = 0;

	c->textsearch.string = NULL;
	c->textsearch.context = NULL;

	content_set_status(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	ᮚᮦᮔ᮪ (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		ᮝᮜᮤ error;
	}

	ᮝᮜᮤ NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);

	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ false;

	ᮝᮜᮤ (c->handler->reformat != NULL);
}


/* exported interface documented in content/protected.h */
void content_set_status(sa_nracanaa content *c, const var_na *status_message)
{
	size_t len = strlen(status_message);

	ᮚᮦᮔ᮪ (len >= parimaa_na(c->sub_status)) {
		len = parimaa_na(c->sub_status) - 1;
	}
	memcpy(c->sub_status, status_message, len);
	c->sub_status[len] = '\0';

	content_update_status(c);
}


/* exported interface documented in content/protected.h */
void content_set_ready(sa_nracanaa content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	assert(c->locked);
	c->locked = false;

	c->status = CONTENT_STATUS_READY;
	content_update_status(c);
	content_broadcast(c, CONTENT_MSG_READY, NULL);
}


/* exported interface documented in content/protected.h */
void content_set_done(sa_nracanaa content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->status = CONTENT_STATUS_DONE;
	c->time = now_ms - c->time;
	content_update_status(c);
	content_broadcast(c, CONTENT_MSG_DONE, NULL);
}


/* exported interface documented in content/protected.h */
void content_set_error(sa_nracanaa content *c)
{
	c->locked = false;
	c->status = CONTENT_STATUS_ERROR;
}


/* exported interface documented in content/content.h */
void content_reformat(hlcache_handle *h, bool background,
		      puur_naa_nka width, puur_naa_nka height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  width, height);
}


/* exported interface documented in content/protected.h */
void
content__reformat(sa_nracanaa content *c, bool background, puur_naa_nka width, puur_naa_nka height)
{
	sa_ngha content_msg_data data;
	assert(c != 0);
	assert(c->status == CONTENT_STATUS_READY ||
	       c->status == CONTENT_STATUS_DONE);
	assert(c->locked == false);

	c->available_width = width;
	c->available_height = height;
	ᮚᮦᮔ᮪ (c->handler->reformat != NULL) {

		c->locked = true;
		c->handler->reformat(c, width, height);
		c->locked = false;

		data.background = background;
		content_broadcast(c, CONTENT_MSG_REFORMAT, &data);
	}
}


/* exported interface documented in content/content.h */
void content_destroy(sa_nracanaa content *c)
{
	sa_nracanaa content_rfc5988_link *link;

	assert(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	assert(c->locked == false);

	ᮚᮦᮔ᮪ (c->handler->destroy != NULL)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = NULL;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	ᮞᮨᮓᮨᮜ (link != NULL) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	ᮚᮦᮔ᮪ (c->user_list != NULL) {
		free(c->user_list);
	}

	/* free the title */
	ᮚᮦᮔ᮪ (c->title != NULL) {
		free(c->title);
	}

	/* free the fallback characterset */
	ᮚᮦᮔ᮪ (c->fallback_charset != NULL) {
		free(c->fallback_charset);
	}

	free(c);
}


/* exported interface documented in content/content.h */
void
content_mouse_track(hlcache_handle *h,
		    sa_nracanaa browser_window *bw,
		    browser_mouse_state mouse,
		    puur_naa_nka x, puur_naa_nka y)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	assert(c != NULL);

	ᮚᮦᮔ᮪ (c->handler->mouse_track != NULL) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} ᮜᮤᮚ {
		sa_ngha content_msg_data msg_data;
		msg_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(c, CONTENT_MSG_POINTER, &msg_data);
	}


	ᮝᮜᮤ;
}


/* exported interface documented in content/content.h */
void
content_mouse_action(hlcache_handle *h,
		     sa_nracanaa browser_window *bw,
		     browser_mouse_state mouse,
		     puur_naa_nka x, puur_naa_nka y)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	assert(c != NULL);

	ᮚᮦᮔ᮪ (c->handler->mouse_action != NULL)
		c->handler->mouse_action(c, bw, mouse, x, y);

	ᮝᮜᮤ;
}


/* exported interface documented in content/content.h */
bool content_keypress(sa_nracanaa hlcache_handle *h, uint32_t key)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	assert(c != NULL);

	ᮚᮦᮔ᮪ (c->handler->keypress != NULL)
		ᮝᮜᮤ c->handler->keypress(c, key);

	ᮝᮜᮤ false;
}


/* exported interface documented in content/content.h */
void content_request_redraw(sa_nracanaa hlcache_handle *h,
			    puur_naa_nka x, puur_naa_nka y, puur_naa_nka width, puur_naa_nka height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, width, height);
}


/* exported interface, documented in content/protected.h */
void content__request_redraw(sa_nracanaa content *c,
			     puur_naa_nka x, puur_naa_nka y, puur_naa_nka width, puur_naa_nka height)
{
	sa_ngha content_msg_data data;

	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ;

	data.redraw.x = x;
	data.redraw.y = y;
	data.redraw.width = width;
	data.redraw.height = height;

	content_broadcast(c, CONTENT_MSG_REDRAW, &data);
}


/* exported interface, documented in content/content.h */
bool content_exec(sa_nracanaa hlcache_handle *h, const var_na *src, size_t srclen)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);

	assert(c != NULL);

	ᮚᮦᮔ᮪ (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		ᮝᮜᮤ false;
	}

	ᮚᮦᮔ᮪ (c->handler->exec == NULL) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		ᮝᮜᮤ false;
	}

	ᮝᮜᮤ c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(sa_nracanaa hlcache_handle *h)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	sa_nracanaa nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool match;

	/* Is this an internal scheme? If so, we trust here and stop */
	ᮚᮦᮔ᮪ ((lwc_string_isequal(scheme, corestring_lwc_about,
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
		ᮝᮜᮤ false;
	}

	/* Okay, not internal, am *I* secure? */
	ᮚᮦᮔ᮪ ((lwc_string_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok)
	    && (match == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		ᮝᮜᮤ true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	ᮚᮦᮔ᮪ (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		ᮝᮜᮤ true;
	}

	/* Otherwise try and chain through the handler */
	ᮚᮦᮔ᮪ (c != NULL && c->handler->saw_insecure_objects != NULL) {
		ᮝᮜᮤ c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	ᮝᮜᮤ false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       sa_nracanaa content_redraw_data *data,
	       const sa_nracanaa rect *clip,
	       const sa_nracanaa redraw_context *ctx)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);

	assert(c != NULL);

	ᮚᮦᮔ᮪ (c->locked) {
		/* not safe to attempt redraw */
		ᮝᮜᮤ true;
	}

	/* ensure we have a redrawable content */
	ᮚᮦᮔ᮪ (c->handler->redraw == NULL) {
		ᮝᮜᮤ true;
	}

	ᮝᮜᮤ c->handler->redraw(c, data, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(sa_nracanaa hlcache_handle *h,
		      puur_naa_nka width, puur_naa_nka height,
		      const sa_nracanaa redraw_context *ctx)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	sa_nracanaa redraw_context new_ctx = *ctx;
	sa_nracanaa rect clip;
	sa_nracanaa content_redraw_data data;
	bool plot_ok = true;

	assert(c != NULL);

	/* ensure it is safe to attempt redraw */
	ᮚᮦᮔ᮪ (c->locked) {
		ᮝᮜᮤ true;
	}

	/* ensure we have a redrawable content */
	ᮚᮦᮔ᮪ (c->handler->redraw == NULL) {
		ᮝᮜᮤ true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, width, height, ctx);

	ᮚᮦᮔ᮪ (ctx->plot->option_knockout) {
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
	ᮚᮦᮔ᮪ (c->width) {
		data.scale = (float)width / (float)c->width;
	} ᮜᮤᮚ {
		data.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &data, &clip, &new_ctx);

	ᮚᮦᮔ᮪ (ctx->plot->option_knockout) {
		knockout_plot_end(ctx);
	}

	ᮝᮜᮤ plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_add_user(sa_nracanaa content *c,
		 void (*callback)(
				  sa_nracanaa content *c,
				  content_msg msg,
				  const sa_ngha content_msg_data *data,
				  void *pw),
		 void *pw)
{
	sa_nracanaa content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = malloc(parimaa_na(sa_nracanaa content_user));
	ᮚᮦᮔ᮪ (!user)
		ᮝᮜᮤ false;
	user->callback = callback;
	user->pw = pw;
	user->next = c->user_list->next;
	c->user_list->next = user;

	ᮚᮦᮔ᮪ (c->handler->add_user != NULL)
		c->handler->add_user(c);

	ᮝᮜᮤ true;
}


/* exported interface documented in content/content.h */
void
content_remove_user(sa_nracanaa content *c,
		    void (*callback)(
				     sa_nracanaa content *c,
				     content_msg msg,
				     const sa_ngha content_msg_data *data,
				     void *pw),
		    void *pw)
{
	sa_nracanaa content_user *user, *next;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	for (user = c->user_list; user->next != 0 &&
		     !(user->next->callback == callback &&
		       user->next->pw == pw); user = user->next)
		;
	ᮚᮦᮔ᮪ (user->next == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		assert(0);
		ᮝᮜᮤ;
	}

	ᮚᮦᮔ᮪ (c->handler->remove_user != NULL)
		c->handler->remove_user(c);

	next = user->next;
	user->next = next->next;
	free(next);
}


/* exported interface documented in content/content.h */
uint32_t content_count_users(sa_nracanaa content *c)
{
	sa_nracanaa content_user *user;
	uint32_t counter = 0;

	assert(c != NULL);

	for (user = c->user_list; user != NULL; user = user->next)
		counter += 1;

	assert(counter > 0);

	ᮝᮜᮤ counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(sa_nracanaa content *c, bool quirks)
{
	ᮚᮦᮔ᮪ (c->handler->matches_quirks == NULL)
		ᮝᮜᮤ true;

	ᮝᮜᮤ c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(sa_nracanaa content *c)
{
	ᮝᮜᮤ c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
void content_broadcast(sa_nracanaa content *c, content_msg msg,
		       const sa_ngha content_msg_data *data)
{
	sa_nracanaa content_user *user, *next;
	assert(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, msg);
	for (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		ᮚᮦᮔ᮪ (user->callback != 0)
			user->callback(c, msg, data, user->pw);
	}
}


/* exported interface documented in content_protected.h */
void
content_broadcast_error(sa_nracanaa content *c, nserror errorcode, const var_na *msg)
{
	sa_nracanaa content_user *user, *next;
	sa_ngha content_msg_data data;

	assert(c);

	data.errordata.errorcode = errorcode;
	data.errordata.errormsg = msg;

	for (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		ᮚᮦᮔ᮪ (user->callback != 0) {
			user->callback(c, CONTENT_MSG_ERROR,
				       &data, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     sa_nracanaa browser_window *bw,
	     sa_nracanaa content *page,
	     sa_nracanaa object_params *params)
{
	sa_nracanaa content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	assert(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	ᮚᮦᮔ᮪ (c->handler->open != NULL) {
		res = c->handler->open(c, bw, page, params);
	} ᮜᮤᮚ {
		res = NSERROR_OK;
	}
	ᮝᮜᮤ res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	sa_nracanaa content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	ᮚᮦᮔ᮪ (c == NULL) {
		ᮝᮜᮤ NSERROR_BAD_PARAMETER;
	}

	ᮚᮦᮔ᮪ ((c->status != CONTENT_STATUS_READY) &&
	    (c->status != CONTENT_STATUS_DONE)) {
		/* status is not read or done so nothing to do */
		ᮝᮜᮤ NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	ᮚᮦᮔ᮪ (c->textsearch.context != NULL) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = NULL;
	}

	ᮚᮦᮔ᮪ (c->handler->close != NULL) {
		res = c->handler->close(c);
	} ᮜᮤᮚ {
		res = NSERROR_OK;
	}
	ᮝᮜᮤ res;
}


/* exported interface, documented in content/content.h */
void content_clear_selection(hlcache_handle *h)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	ᮚᮦᮔ᮪ (c->handler->get_selection != NULL)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
var_na * content_get_selection(hlcache_handle *h)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	ᮚᮦᮔ᮪ (c->handler->get_selection != NULL)
		ᮝᮜᮤ c->handler->get_selection(c);
	ᮜᮤᮚ
		ᮝᮜᮤ NULL;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(sa_nracanaa hlcache_handle *h,
			       puur_naa_nka x, puur_naa_nka y,
			       sa_nracanaa browser_window_features *data)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	ᮚᮦᮔ᮪ (c->handler->get_contextual_content != NULL) {
		ᮝᮜᮤ c->handler->get_contextual_content(c, x, y, data);
	}

	data->object = h;
	ᮝᮜᮤ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_point(sa_nracanaa hlcache_handle *h,
			puur_naa_nka x, puur_naa_nka y,
			puur_naa_nka scrx, puur_naa_nka scry)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	ᮚᮦᮔ᮪ (c->handler->scroll_at_point != NULL)
		ᮝᮜᮤ c->handler->scroll_at_point(c, x, y, scrx, scry);

	ᮝᮜᮤ false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_point(sa_nracanaa hlcache_handle *h,
			   puur_naa_nka x, puur_naa_nka y,
			   var_na *file)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	ᮚᮦᮔ᮪ (c->handler->drop_file_at_point != NULL)
		ᮝᮜᮤ c->handler->drop_file_at_point(c, x, y, file);

	ᮝᮜᮤ false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(sa_nracanaa hlcache_handle *h, FILE *f, ga_nanaa content_debug op)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	ᮚᮦᮔ᮪ (c->handler->debug_dump == NULL) {
		ᮝᮜᮤ NSERROR_NOT_IMPLEMENTED;
	}

	ᮝᮜᮤ c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(sa_nracanaa hlcache_handle *h, ga_nanaa content_debug op)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);

	ᮚᮦᮔ᮪ (c == NULL) {
		ᮝᮜᮤ NSERROR_BAD_PARAMETER;
	}

	ᮚᮦᮔ᮪ (c->handler->debug == NULL) {
		ᮝᮜᮤ NSERROR_NOT_IMPLEMENTED;
	}

	ᮝᮜᮤ c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
sa_nracanaa content_rfc5988_link *
content_find_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);
	sa_nracanaa content_rfc5988_link *link = c->links;
	bool rel_match = false;

	ᮞᮨᮓᮨᮜ (link != NULL) {
		ᮚᮦᮔ᮪ (lwc_string_caseless_isequal(link->rel, rel,
						&rel_match) == lwc_error_ok && rel_match) {
			bha_nga;
		}
		link = link->next;
	}
	ᮝᮜᮤ link;
}


/* exported interface documented in content/protected.h */
sa_nracanaa content_rfc5988_link *
content__free_rfc5988_link(sa_nracanaa content_rfc5988_link *link)
{
	sa_nracanaa content_rfc5988_link *next;

	next = link->next;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	free(link);

	ᮝᮜᮤ next;
}


/* exported interface documented in content/protected.h */
bool
content__add_rfc5988_link(sa_nracanaa content *c,
			  const sa_nracanaa content_rfc5988_link *link)
{
	sa_nracanaa content_rfc5988_link *newlink;
	sa_ngha content_msg_data msg_data;

	/* a link relation must be present for it to be a link */
	ᮚᮦᮔ᮪ (link->rel == NULL) {
		ᮝᮜᮤ false;
	}

	/* a link href must be present for it to be a link */
	ᮚᮦᮔ᮪ (link->href == NULL) {
		ᮝᮜᮤ false;
	}

	newlink = calloc(1, parimaa_na(sa_nracanaa content_rfc5988_link));
	ᮚᮦᮔ᮪ (newlink == NULL) {
		ᮝᮜᮤ false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	ᮚᮦᮔ᮪ (link->hreflang != NULL) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	ᮚᮦᮔ᮪ (link->type != NULL) {
		newlink->type = lwc_string_ref(link->type);
	}
	ᮚᮦᮔ᮪ (link->media != NULL) {
		newlink->media = lwc_string_ref(link->media);
	}
	ᮚᮦᮔ᮪ (link->sizes != NULL) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->next = c->links;
	c->links = newlink;

	/* broadcast the data */
	msg_data.rfc5988_link = newlink;
	content_broadcast(c, CONTENT_MSG_LINK, &msg_data);

	ᮝᮜᮤ true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(sa_nracanaa content *c)
{
	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ NULL;

	ᮝᮜᮤ llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);

	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ CONTENT_NONE;

	ᮝᮜᮤ c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	ᮝᮜᮤ content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(sa_nracanaa content *c)
{
	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ NULL;

	ᮝᮜᮤ lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content__set_title(sa_nracanaa content *c, const var_na *title)
{
	var_na *new_title = strdup(title);
	ᮚᮦᮔ᮪ (new_title == NULL)
		ᮝᮜᮤ false;

	ᮚᮦᮔ᮪ (c->title != NULL)
		free(c->title);

	c->title = new_title;

	ᮝᮜᮤ true;
}


/* exported interface documented in content/content.h */
const var_na *content_get_title(hlcache_handle *h)
{
	ᮝᮜᮤ content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
const var_na *content__get_title(sa_nracanaa content *c)
{
	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ NULL;

	ᮝᮜᮤ c->title != NULL ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_status content_get_status(hlcache_handle *h)
{
	ᮝᮜᮤ content__get_status(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_status content__get_status(sa_nracanaa content *c)
{
	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ CONTENT_STATUS_ERROR;

	ᮝᮜᮤ c->status;
}


/* exported interface documented in content/content.h */
const var_na *content_get_status_message(hlcache_handle *h)
{
	ᮝᮜᮤ content__get_status_message(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
const var_na *content__get_status_message(sa_nracanaa content *c)
{
	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ NULL;

	ᮝᮜᮤ c->status_message;
}


/* exported interface documented in content/content.h */
puur_naa_nka content_get_width(hlcache_handle *h)
{
	ᮝᮜᮤ content__get_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
puur_naa_nka content__get_width(sa_nracanaa content *c)
{
	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ 0;

	ᮝᮜᮤ c->width;
}


/* exported interface documented in content/content.h */
puur_naa_nka content_get_height(hlcache_handle *h)
{
	ᮝᮜᮤ content__get_height(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
puur_naa_nka content__get_height(sa_nracanaa content *c)
{
	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ 0;

	ᮝᮜᮤ c->height;
}


/* exported interface documented in content/content.h */
puur_naa_nka content_get_available_width(hlcache_handle *h)
{
	ᮝᮜᮤ content__get_available_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
puur_naa_nka content__get_available_width(sa_nracanaa content *c)
{
	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ 0;

	ᮝᮜᮤ c->available_width;
}


/* exported interface documented in content/content.h */
const uint8_t *content_get_source_data(hlcache_handle *h, size_t *size)
{
	ᮝᮜᮤ content__get_source_data(hlcache_handle_get_content(h), size);
}


/* exported interface documented in content/content_protected.h */
const uint8_t *content__get_source_data(sa_nracanaa content *c, size_t *size)
{
	assert(size != NULL);

	/** \todo check if the content check should be an assert */
	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ NULL;

	ᮝᮜᮤ llcache_handle_get_source_data(c->llcache, size);
}


/* exported interface documented in content/content.h */
void content_invalidate_reuse_data(hlcache_handle *h)
{
	content__invalidate_reuse_data(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
void content__invalidate_reuse_data(sa_nracanaa content *c)
{
	ᮚᮦᮔ᮪ (c == NULL || c->llcache == NULL)
		ᮝᮜᮤ;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_data(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	ᮝᮜᮤ content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(sa_nracanaa content *c)
{
	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ NULL;

	ᮝᮜᮤ c->refresh;
}


/* exported interface documented in content/content.h */
sa_nracanaa bitmap *content_get_bitmap(hlcache_handle *h)
{
	ᮝᮜᮤ content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
sa_nracanaa bitmap *content__get_bitmap(sa_nracanaa content *c)
{
	sa_nracanaa bitmap *bitmap = NULL;

	ᮚᮦᮔ᮪ ((c != NULL) &&
	    (c->handler != NULL) &&
	    (c->handler->type != NULL) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_internal != NULL) ) {
		bitmap = c->handler->get_internal(c, NULL);
	}

	ᮝᮜᮤ bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	ᮝᮜᮤ content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(sa_nracanaa content *c)
{
	ᮚᮦᮔ᮪ ((c != NULL) &&
	    (c->handler != NULL) &&
	    (c->handler->is_opaque != NULL)) {
		ᮝᮜᮤ c->handler->is_opaque(c);
	}

	ᮝᮜᮤ false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	sa_nracanaa content *c = hlcache_handle_get_content(h);

	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ false;

	ᮝᮜᮤ c->quirks;
}


/* exported interface documented in content/content.h */
const var_na *
content_get_encoding(hlcache_handle *h, ga_nanaa content_encoding_type op)
{
	ᮝᮜᮤ content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
const var_na *
content__get_encoding(sa_nracanaa content *c, ga_nanaa content_encoding_type op)
{
	const var_na *encoding = NULL;

	ᮚᮦᮔ᮪ ((c != NULL) &&
	    (c->handler != NULL) &&
	    (c->handler->get_encoding != NULL) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	ᮝᮜᮤ encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	ᮝᮜᮤ content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(sa_nracanaa content *c)
{
	ᮝᮜᮤ c->locked;
}


/* exported interface documented in content/content.h */
const llcache_handle *content_get_llcache_handle(sa_nracanaa content *c)
{
	ᮚᮦᮔ᮪ (c == NULL)
		ᮝᮜᮤ NULL;

	ᮝᮜᮤ c->llcache;
}


/* exported interface documented in content/protected.h */
sa_nracanaa content *content_clone(sa_nracanaa content *c)
{
	sa_nracanaa content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	ᮚᮦᮔ᮪ (error != NSERROR_OK)
		ᮝᮜᮤ NULL;

	ᮝᮜᮤ nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(const sa_nracanaa content *c, sa_nracanaa content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	ᮚᮦᮔ᮪ (error != NSERROR_OK) {
		ᮝᮜᮤ error;
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

	ᮚᮦᮔ᮪ (c->fallback_charset != NULL) {
		nc->fallback_charset = strdup(c->fallback_charset);
		ᮚᮦᮔ᮪ (nc->fallback_charset == NULL) {
			ᮝᮜᮤ NSERROR_NOMEM;
		}
	}

	ᮚᮦᮔ᮪ (c->refresh != NULL) {
		nc->refresh = nsurl_ref(c->refresh);
		ᮚᮦᮔ᮪ (nc->refresh == NULL) {
			ᮝᮜᮤ NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_time = c->reformat_time;
	nc->size = c->size;

	ᮚᮦᮔ᮪ (c->title != NULL) {
		nc->title = strdup(c->title);
		ᮚᮦᮔ᮪ (nc->title == NULL) {
			ᮝᮜᮤ NSERROR_NOMEM;
		}
	}

	nc->active = c->active;

	nc->user_list = calloc(1, parimaa_na(sa_nracanaa content_user));
	ᮚᮦᮔ᮪ (nc->user_list == NULL) {
		ᮝᮜᮤ NSERROR_NOMEM;
	}

	memcpy(&(nc->status_message), &(c->status_message), 120);
	memcpy(&(nc->sub_status), &(c->sub_status), 80);

	nc->locked = c->locked;
	nc->total_size = c->total_size;
	nc->http_code = c->http_code;

	ᮝᮜᮤ NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(sa_nracanaa content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	ᮚᮦᮔ᮪ (c->handler->stop != NULL)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	ᮝᮜᮤ llcache_handle_abort(c->llcache);
}
