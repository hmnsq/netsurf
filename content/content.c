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

const char * const content_status_name[] = {
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
static void content_convert(struct content *c)
{
	assert(c);
	assert(c->status == CONTENT_STATUS_LOADING ||
	       c->status == CONTENT_STATUS_ERROR);

	если (c->status != CONTENT_STATUS_LOADING)
		вернуть;

	если (c->locked == true)
		вернуть;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	если (c->handler->data_complete != NULL) {
		c->locked = true;
		если (c->handler->data_complete(c) == false) {
			content_set_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} иначе {
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
	struct content *c = pw;
	union content_msg_data msg_data;
	nserror error = NSERROR_OK;

	switch (event->type) {
	case LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		break;
	case LLCACHE_EVENT_HAD_HEADERS:
		/* Will never happen: handled in hlcache */
		break;
	case LLCACHE_EVENT_HAD_DATA:
		если (c->handler->process_data != NULL) {
			если (c->handler->process_data(c,
						     (const char *) event->data.data.buf,
						     event->data.data.len) == false) {
				llcache_handle_abort(c->llcache);
				c->status = CONTENT_STATUS_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		break;
	case LLCACHE_EVENT_DONE:
		{
			size_t source_size;

			(void) llcache_handle_get_source_data(llcache, &source_size);

			content_set_status(c, messages_get("Processing"));
			msg_data.explicit_status_text = NULL;
			content_broadcast(c, CONTENT_MSG_STATUS, &msg_data);

			content_convert(c);
		}
		break;
	case LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->status = CONTENT_STATUS_ERROR;
		msg_data.errordata.errorcode = event->data.error.code;
		msg_data.errordata.errormsg = event->data.error.msg;
		content_broadcast(c, CONTENT_MSG_ERROR, &msg_data);
		break;
	case LLCACHE_EVENT_PROGRESS:
		content_set_status(c, event->data.progress_msg);
		msg_data.explicit_status_text = NULL;
		content_broadcast(c, CONTENT_MSG_STATUS, &msg_data);
		break;
	case LLCACHE_EVENT_REDIRECT:
		msg_data.redirect.from = event->data.redirect.from;
		msg_data.redirect.to = event->data.redirect.to;
		content_broadcast(c, CONTENT_MSG_REDIRECT, &msg_data);
		break;
	}

	вернуть error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
static void content_update_status(struct content *c)
{
	если (c->status == CONTENT_STATUS_LOADING ||
	    c->status == CONTENT_STATUS_READY) {
		/* Not done yet */
		snprintf(c->status_message, sizeof (c->status_message),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_status[0] != '\0' ? ", " : " ",
			 c->sub_status);
	} иначе {
		snprintf(c->status_message, sizeof (c->status_message),
			 "%s (%.1fs)", messages_get("Done"),
			 (float) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(struct content *c,
	      const content_handler *handler,
	      lwc_string *imime_type,
	      const struct http_parameter *params,
	      llcache_handle *llcache,
	      const char *fallback_charset,
	      bool quirks)
{
	struct content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = calloc(1, sizeof(struct content_user));
	если (user_sentinel == NULL) {
		вернуть NSERROR_NOMEM;
	}

	если (fallback_charset != NULL) {
		c->fallback_charset = strdup(fallback_charset);
		если (c->fallback_charset == NULL) {
			free(user_sentinel);
			вернуть NSERROR_NOMEM;
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
	если (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		вернуть error;
	}

	вернуть NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	struct content *c = hlcache_handle_get_content(h);

	если (c == NULL)
		вернуть false;

	вернуть (c->handler->reformat != NULL);
}


/* exported interface documented in content/protected.h */
void content_set_status(struct content *c, const char *status_message)
{
	size_t len = strlen(status_message);

	если (len >= sizeof(c->sub_status)) {
		len = sizeof(c->sub_status) - 1;
	}
	memcpy(c->sub_status, status_message, len);
	c->sub_status[len] = '\0';

	content_update_status(c);
}


/* exported interface documented in content/protected.h */
void content_set_ready(struct content *c)
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
void content_set_done(struct content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->status = CONTENT_STATUS_DONE;
	c->time = now_ms - c->time;
	content_update_status(c);
	content_broadcast(c, CONTENT_MSG_DONE, NULL);
}


/* exported interface documented in content/protected.h */
void content_set_error(struct content *c)
{
	c->locked = false;
	c->status = CONTENT_STATUS_ERROR;
}


/* exported interface documented in content/content.h */
void content_reformat(hlcache_handle *h, bool background,
		      целое width, целое height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  width, height);
}


/* exported interface documented in content/protected.h */
void
content__reformat(struct content *c, bool background, целое width, целое height)
{
	union content_msg_data data;
	assert(c != 0);
	assert(c->status == CONTENT_STATUS_READY ||
	       c->status == CONTENT_STATUS_DONE);
	assert(c->locked == false);

	c->available_width = width;
	c->available_height = height;
	если (c->handler->reformat != NULL) {

		c->locked = true;
		c->handler->reformat(c, width, height);
		c->locked = false;

		data.background = background;
		content_broadcast(c, CONTENT_MSG_REFORMAT, &data);
	}
}


/* exported interface documented in content/content.h */
void content_destroy(struct content *c)
{
	struct content_rfc5988_link *link;

	assert(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	assert(c->locked == false);

	если (c->handler->destroy != NULL)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = NULL;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	пока (link != NULL) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	если (c->user_list != NULL) {
		free(c->user_list);
	}

	/* free the title */
	если (c->title != NULL) {
		free(c->title);
	}

	/* free the fallback characterset */
	если (c->fallback_charset != NULL) {
		free(c->fallback_charset);
	}

	free(c);
}


/* exported interface documented in content/content.h */
void
content_mouse_track(hlcache_handle *h,
		    struct browser_window *bw,
		    browser_mouse_state mouse,
		    целое x, целое y)
{
	struct content *c = hlcache_handle_get_content(h);
	assert(c != NULL);

	если (c->handler->mouse_track != NULL) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} иначе {
		union content_msg_data msg_data;
		msg_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(c, CONTENT_MSG_POINTER, &msg_data);
	}


	вернуть;
}


/* exported interface documented in content/content.h */
void
content_mouse_action(hlcache_handle *h,
		     struct browser_window *bw,
		     browser_mouse_state mouse,
		     целое x, целое y)
{
	struct content *c = hlcache_handle_get_content(h);
	assert(c != NULL);

	если (c->handler->mouse_action != NULL)
		c->handler->mouse_action(c, bw, mouse, x, y);

	вернуть;
}


/* exported interface documented in content/content.h */
bool content_keypress(struct hlcache_handle *h, uint32_t key)
{
	struct content *c = hlcache_handle_get_content(h);
	assert(c != NULL);

	если (c->handler->keypress != NULL)
		вернуть c->handler->keypress(c, key);

	вернуть false;
}


/* exported interface documented in content/content.h */
void content_request_redraw(struct hlcache_handle *h,
			    целое x, целое y, целое width, целое height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, width, height);
}


/* exported interface, documented in content/protected.h */
void content__request_redraw(struct content *c,
			     целое x, целое y, целое width, целое height)
{
	union content_msg_data data;

	если (c == NULL)
		вернуть;

	data.redraw.x = x;
	data.redraw.y = y;
	data.redraw.width = width;
	data.redraw.height = height;

	content_broadcast(c, CONTENT_MSG_REDRAW, &data);
}


/* exported interface, documented in content/content.h */
bool content_exec(struct hlcache_handle *h, const char *src, size_t srclen)
{
	struct content *c = hlcache_handle_get_content(h);

	assert(c != NULL);

	если (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		вернуть false;
	}

	если (c->handler->exec == NULL) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		вернуть false;
	}

	вернуть c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(struct hlcache_handle *h)
{
	struct content *c = hlcache_handle_get_content(h);
	struct nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool match;

	/* Is this an internal scheme? If so, we trust here and stop */
	если ((lwc_string_isequal(scheme, corestring_lwc_about,
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
		вернуть false;
	}

	/* Okay, not internal, am *I* secure? */
	если ((lwc_string_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok)
	    && (match == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		вернуть true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	если (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		вернуть true;
	}

	/* Otherwise try and chain through the handler */
	если (c != NULL && c->handler->saw_insecure_objects != NULL) {
		вернуть c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	вернуть false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       struct content_redraw_data *data,
	       const struct rect *clip,
	       const struct redraw_context *ctx)
{
	struct content *c = hlcache_handle_get_content(h);

	assert(c != NULL);

	если (c->locked) {
		/* not safe to attempt redraw */
		вернуть true;
	}

	/* ensure we have a redrawable content */
	если (c->handler->redraw == NULL) {
		вернуть true;
	}

	вернуть c->handler->redraw(c, data, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(struct hlcache_handle *h,
		      целое width, целое height,
		      const struct redraw_context *ctx)
{
	struct content *c = hlcache_handle_get_content(h);
	struct redraw_context new_ctx = *ctx;
	struct rect clip;
	struct content_redraw_data data;
	bool plot_ok = true;

	assert(c != NULL);

	/* ensure it is safe to attempt redraw */
	если (c->locked) {
		вернуть true;
	}

	/* ensure we have a redrawable content */
	если (c->handler->redraw == NULL) {
		вернуть true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, width, height, ctx);

	если (ctx->plot->option_knockout) {
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
	если (c->width) {
		data.scale = (float)width / (float)c->width;
	} иначе {
		data.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &data, &clip, &new_ctx);

	если (ctx->plot->option_knockout) {
		knockout_plot_end(ctx);
	}

	вернуть plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_add_user(struct content *c,
		 void (*callback)(
				  struct content *c,
				  content_msg msg,
				  const union content_msg_data *data,
				  void *pw),
		 void *pw)
{
	struct content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = malloc(sizeof(struct content_user));
	если (!user)
		вернуть false;
	user->callback = callback;
	user->pw = pw;
	user->next = c->user_list->next;
	c->user_list->next = user;

	если (c->handler->add_user != NULL)
		c->handler->add_user(c);

	вернуть true;
}


/* exported interface documented in content/content.h */
void
content_remove_user(struct content *c,
		    void (*callback)(
				     struct content *c,
				     content_msg msg,
				     const union content_msg_data *data,
				     void *pw),
		    void *pw)
{
	struct content_user *user, *next;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	для (user = c->user_list; user->next != 0 &&
		     !(user->next->callback == callback &&
		       user->next->pw == pw); user = user->next)
		;
	если (user->next == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		assert(0);
		вернуть;
	}

	если (c->handler->remove_user != NULL)
		c->handler->remove_user(c);

	next = user->next;
	user->next = next->next;
	free(next);
}


/* exported interface documented in content/content.h */
uint32_t content_count_users(struct content *c)
{
	struct content_user *user;
	uint32_t counter = 0;

	assert(c != NULL);

	для (user = c->user_list; user != NULL; user = user->next)
		counter += 1;

	assert(counter > 0);

	вернуть counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(struct content *c, bool quirks)
{
	если (c->handler->matches_quirks == NULL)
		вернуть true;

	вернуть c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(struct content *c)
{
	вернуть c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
void content_broadcast(struct content *c, content_msg msg,
		       const union content_msg_data *data)
{
	struct content_user *user, *next;
	assert(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, msg);
	для (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		если (user->callback != 0)
			user->callback(c, msg, data, user->pw);
	}
}


/* exported interface documented in content_protected.h */
void
content_broadcast_error(struct content *c, nserror errorcode, const char *msg)
{
	struct content_user *user, *next;
	union content_msg_data data;

	assert(c);

	data.errordata.errorcode = errorcode;
	data.errordata.errormsg = msg;

	для (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		если (user->callback != 0) {
			user->callback(c, CONTENT_MSG_ERROR,
				       &data, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     struct browser_window *bw,
	     struct content *page,
	     struct object_params *params)
{
	struct content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	assert(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	если (c->handler->open != NULL) {
		res = c->handler->open(c, bw, page, params);
	} иначе {
		res = NSERROR_OK;
	}
	вернуть res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	struct content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	если (c == NULL) {
		вернуть NSERROR_BAD_PARAMETER;
	}

	если ((c->status != CONTENT_STATUS_READY) &&
	    (c->status != CONTENT_STATUS_DONE)) {
		/* status is not read or done so nothing to do */
		вернуть NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	если (c->textsearch.context != NULL) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = NULL;
	}

	если (c->handler->close != NULL) {
		res = c->handler->close(c);
	} иначе {
		res = NSERROR_OK;
	}
	вернуть res;
}


/* exported interface, documented in content/content.h */
void content_clear_selection(hlcache_handle *h)
{
	struct content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	если (c->handler->get_selection != NULL)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
char * content_get_selection(hlcache_handle *h)
{
	struct content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	если (c->handler->get_selection != NULL)
		вернуть c->handler->get_selection(c);
	иначе
		вернуть NULL;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(struct hlcache_handle *h,
			       целое x, целое y,
			       struct browser_window_features *data)
{
	struct content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	если (c->handler->get_contextual_content != NULL) {
		вернуть c->handler->get_contextual_content(c, x, y, data);
	}

	data->object = h;
	вернуть NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_point(struct hlcache_handle *h,
			целое x, целое y,
			целое scrx, целое scry)
{
	struct content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	если (c->handler->scroll_at_point != NULL)
		вернуть c->handler->scroll_at_point(c, x, y, scrx, scry);

	вернуть false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_point(struct hlcache_handle *h,
			   целое x, целое y,
			   char *file)
{
	struct content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	если (c->handler->drop_file_at_point != NULL)
		вернуть c->handler->drop_file_at_point(c, x, y, file);

	вернуть false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(struct hlcache_handle *h, FILE *f, enum content_debug op)
{
	struct content *c = hlcache_handle_get_content(h);
	assert(c != 0);

	если (c->handler->debug_dump == NULL) {
		вернуть NSERROR_NOT_IMPLEMENTED;
	}

	вернуть c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(struct hlcache_handle *h, enum content_debug op)
{
	struct content *c = hlcache_handle_get_content(h);

	если (c == NULL) {
		вернуть NSERROR_BAD_PARAMETER;
	}

	если (c->handler->debug == NULL) {
		вернуть NSERROR_NOT_IMPLEMENTED;
	}

	вернуть c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
struct content_rfc5988_link *
content_find_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	struct content *c = hlcache_handle_get_content(h);
	struct content_rfc5988_link *link = c->links;
	bool rel_match = false;

	пока (link != NULL) {
		если (lwc_string_caseless_isequal(link->rel, rel,
						&rel_match) == lwc_error_ok && rel_match) {
			break;
		}
		link = link->next;
	}
	вернуть link;
}


/* exported interface documented in content/protected.h */
struct content_rfc5988_link *
content__free_rfc5988_link(struct content_rfc5988_link *link)
{
	struct content_rfc5988_link *next;

	next = link->next;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	free(link);

	вернуть next;
}


/* exported interface documented in content/protected.h */
bool
content__add_rfc5988_link(struct content *c,
			  const struct content_rfc5988_link *link)
{
	struct content_rfc5988_link *newlink;
	union content_msg_data msg_data;

	/* a link relation must be present for it to be a link */
	если (link->rel == NULL) {
		вернуть false;
	}

	/* a link href must be present for it to be a link */
	если (link->href == NULL) {
		вернуть false;
	}

	newlink = calloc(1, sizeof(struct content_rfc5988_link));
	если (newlink == NULL) {
		вернуть false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	если (link->hreflang != NULL) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	если (link->type != NULL) {
		newlink->type = lwc_string_ref(link->type);
	}
	если (link->media != NULL) {
		newlink->media = lwc_string_ref(link->media);
	}
	если (link->sizes != NULL) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->next = c->links;
	c->links = newlink;

	/* broadcast the data */
	msg_data.rfc5988_link = newlink;
	content_broadcast(c, CONTENT_MSG_LINK, &msg_data);

	вернуть true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(struct content *c)
{
	если (c == NULL)
		вернуть NULL;

	вернуть llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	struct content *c = hlcache_handle_get_content(h);

	если (c == NULL)
		вернуть CONTENT_NONE;

	вернуть c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	вернуть content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(struct content *c)
{
	если (c == NULL)
		вернуть NULL;

	вернуть lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content__set_title(struct content *c, const char *title)
{
	char *new_title = strdup(title);
	если (new_title == NULL)
		вернуть false;

	если (c->title != NULL)
		free(c->title);

	c->title = new_title;

	вернуть true;
}


/* exported interface documented in content/content.h */
const char *content_get_title(hlcache_handle *h)
{
	вернуть content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
const char *content__get_title(struct content *c)
{
	если (c == NULL)
		вернуть NULL;

	вернуть c->title != NULL ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_status content_get_status(hlcache_handle *h)
{
	вернуть content__get_status(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_status content__get_status(struct content *c)
{
	если (c == NULL)
		вернуть CONTENT_STATUS_ERROR;

	вернуть c->status;
}


/* exported interface documented in content/content.h */
const char *content_get_status_message(hlcache_handle *h)
{
	вернуть content__get_status_message(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
const char *content__get_status_message(struct content *c)
{
	если (c == NULL)
		вернуть NULL;

	вернуть c->status_message;
}


/* exported interface documented in content/content.h */
целое content_get_width(hlcache_handle *h)
{
	вернуть content__get_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
целое content__get_width(struct content *c)
{
	если (c == NULL)
		вернуть 0;

	вернуть c->width;
}


/* exported interface documented in content/content.h */
целое content_get_height(hlcache_handle *h)
{
	вернуть content__get_height(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
целое content__get_height(struct content *c)
{
	если (c == NULL)
		вернуть 0;

	вернуть c->height;
}


/* exported interface documented in content/content.h */
целое content_get_available_width(hlcache_handle *h)
{
	вернуть content__get_available_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
целое content__get_available_width(struct content *c)
{
	если (c == NULL)
		вернуть 0;

	вернуть c->available_width;
}


/* exported interface documented in content/content.h */
const uint8_t *content_get_source_data(hlcache_handle *h, size_t *size)
{
	вернуть content__get_source_data(hlcache_handle_get_content(h), size);
}


/* exported interface documented in content/content_protected.h */
const uint8_t *content__get_source_data(struct content *c, size_t *size)
{
	assert(size != NULL);

	/** \todo check if the content check should be an assert */
	если (c == NULL)
		вернуть NULL;

	вернуть llcache_handle_get_source_data(c->llcache, size);
}


/* exported interface documented in content/content.h */
void content_invalidate_reuse_data(hlcache_handle *h)
{
	content__invalidate_reuse_data(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
void content__invalidate_reuse_data(struct content *c)
{
	если (c == NULL || c->llcache == NULL)
		вернуть;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_data(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	вернуть content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(struct content *c)
{
	если (c == NULL)
		вернуть NULL;

	вернуть c->refresh;
}


/* exported interface documented in content/content.h */
struct bitmap *content_get_bitmap(hlcache_handle *h)
{
	вернуть content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
struct bitmap *content__get_bitmap(struct content *c)
{
	struct bitmap *bitmap = NULL;

	если ((c != NULL) &&
	    (c->handler != NULL) &&
	    (c->handler->type != NULL) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_internal != NULL) ) {
		bitmap = c->handler->get_internal(c, NULL);
	}

	вернуть bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	вернуть content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(struct content *c)
{
	если ((c != NULL) &&
	    (c->handler != NULL) &&
	    (c->handler->is_opaque != NULL)) {
		вернуть c->handler->is_opaque(c);
	}

	вернуть false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	struct content *c = hlcache_handle_get_content(h);

	если (c == NULL)
		вернуть false;

	вернуть c->quirks;
}


/* exported interface documented in content/content.h */
const char *
content_get_encoding(hlcache_handle *h, enum content_encoding_type op)
{
	вернуть content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
const char *
content__get_encoding(struct content *c, enum content_encoding_type op)
{
	const char *encoding = NULL;

	если ((c != NULL) &&
	    (c->handler != NULL) &&
	    (c->handler->get_encoding != NULL) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	вернуть encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	вернуть content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(struct content *c)
{
	вернуть c->locked;
}


/* exported interface documented in content/content.h */
const llcache_handle *content_get_llcache_handle(struct content *c)
{
	если (c == NULL)
		вернуть NULL;

	вернуть c->llcache;
}


/* exported interface documented in content/protected.h */
struct content *content_clone(struct content *c)
{
	struct content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	если (error != NSERROR_OK)
		вернуть NULL;

	вернуть nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(const struct content *c, struct content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	если (error != NSERROR_OK) {
		вернуть error;
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

	если (c->fallback_charset != NULL) {
		nc->fallback_charset = strdup(c->fallback_charset);
		если (nc->fallback_charset == NULL) {
			вернуть NSERROR_NOMEM;
		}
	}

	если (c->refresh != NULL) {
		nc->refresh = nsurl_ref(c->refresh);
		если (nc->refresh == NULL) {
			вернуть NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_time = c->reformat_time;
	nc->size = c->size;

	если (c->title != NULL) {
		nc->title = strdup(c->title);
		если (nc->title == NULL) {
			вернуть NSERROR_NOMEM;
		}
	}

	nc->active = c->active;

	nc->user_list = calloc(1, sizeof(struct content_user));
	если (nc->user_list == NULL) {
		вернуть NSERROR_NOMEM;
	}

	memcpy(&(nc->status_message), &(c->status_message), 120);
	memcpy(&(nc->sub_status), &(c->sub_status), 80);

	nc->locked = c->locked;
	nc->total_size = c->total_size;
	nc->http_code = c->http_code;

	вернуть NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(struct content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	если (c->handler->stop != NULL)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	вернуть llcache_handle_abort(c->llcache);
}
