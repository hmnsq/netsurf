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

#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <nsutils/time.h>

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "netsurf/browser_window.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "desktop/knockout.h"

#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/textsearch.h"
#ಅನ್ತರ್ಭಾವ "content/content_debug.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
#ಅನ್ತರ್ಭಾವ "content/urldb.h"

#define URL_FMT_SPC "%.140s"

ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ content_status_name[] = {
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
ಸ್ಥಿರ ಶೂನ್ಯ content_convert(ಸಂರಚನಾ content *c)
{
	ಪ್ರತಿಜ್ಞಾ(c);
	ಪ್ರತಿಜ್ಞಾ(c->status == CONTENT_STATUS_LOADING ||
	       c->status == CONTENT_STATUS_ERROR);

	ಯದಿ (c->status != CONTENT_STATUS_LOADING)
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (c->locked == true)
		ಪ್ರತ್ಯಾಗಮ;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	ಯದಿ (c->handler->data_complete != ರಿಕ್ತ) {
		c->locked = true;
		ಯದಿ (c->handler->data_complete(c) == false) {
			content_set_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} ಅನ್ಯಥಾ {
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
ಸ್ಥಿರ nserror
content_llcache_callback(llcache_handle *llcache,
			 ಧ್ರುವ llcache_event *event, ಶೂನ್ಯ *pw)
{
	ಸಂರಚನಾ content *c = pw;
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
	nserror error = NSERROR_OK;

	ವಿಕಲ್ಪ (event->type) {
	ಪಕ್ಷ LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		ಭಙ್ಗ;
	ಪಕ್ಷ LLCACHE_EVENT_HAD_HEADERS:
		/* Will never happen: handled in hlcache */
		ಭಙ್ಗ;
	ಪಕ್ಷ LLCACHE_EVENT_HAD_DATA:
		ಯದಿ (c->handler->process_data != ರಿಕ್ತ) {
			ಯದಿ (c->handler->process_data(c,
						     (ಧ್ರುವ ವರ್ಣ *) event->data.data.buf,
						     event->data.data.len) == false) {
				llcache_handle_abort(c->llcache);
				c->status = CONTENT_STATUS_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ LLCACHE_EVENT_DONE:
		{
			size_t source_size;

			(ಶೂನ್ಯ) llcache_handle_get_source_data(llcache, &source_size);

			content_set_status(c, messages_get("Processing"));
			ಸಂದೇಶ_data.explicit_status_text = ರಿಕ್ತ;
			content_broadcast(c, CONTENT_ಸಂದೇಶ_STATUS, &ಸಂದೇಶ_data);

			content_convert(c);
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->status = CONTENT_STATUS_ERROR;
		ಸಂದೇಶ_data.errordata.errorcode = event->data.error.code;
		ಸಂದೇಶ_data.errordata.errormsg = event->data.error.ಸಂದೇಶ;
		content_broadcast(c, CONTENT_ಸಂದೇಶ_ERROR, &ಸಂದೇಶ_data);
		ಭಙ್ಗ;
	ಪಕ್ಷ LLCACHE_EVENT_PROGRESS:
		content_set_status(c, event->data.progress_ಸಂದೇಶ);
		ಸಂದೇಶ_data.explicit_status_text = ರಿಕ್ತ;
		content_broadcast(c, CONTENT_ಸಂದೇಶ_STATUS, &ಸಂದೇಶ_data);
		ಭಙ್ಗ;
	ಪಕ್ಷ LLCACHE_EVENT_REDIRECT:
		ಸಂದೇಶ_data.redirect.from = event->data.redirect.from;
		ಸಂದೇಶ_data.redirect.to = event->data.redirect.to;
		content_broadcast(c, CONTENT_ಸಂದೇಶ_REDIRECT, &ಸಂದೇಶ_data);
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
ಸ್ಥಿರ ಶೂನ್ಯ content_update_status(ಸಂರಚನಾ content *c)
{
	ಯದಿ (c->status == CONTENT_STATUS_LOADING ||
	    c->status == CONTENT_STATUS_READY) {
		/* Not done yet */
		snprintf(c->status_message, ಪರಿಮಾಣ (c->status_message),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_status[0] != '\0' ? ", " : " ",
			 c->sub_status);
	} ಅನ್ಯಥಾ {
		snprintf(c->status_message, ಪರಿಮಾಣ (c->status_message),
			 "%s (%.1fs)", messages_get("Done"),
			 (ಭಿನ್ನ) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(ಸಂರಚನಾ content *c,
	      ಧ್ರುವ content_handler *handler,
	      lwc_string *imime_type,
	      ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
	      llcache_handle *llcache,
	      ಧ್ರುವ ವರ್ಣ *fallback_charset,
	      bool quirks)
{
	ಸಂರಚನಾ content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ content_user));
	ಯದಿ (user_sentinel == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ಯದಿ (fallback_charset != ರಿಕ್ತ) {
		c->fallback_charset = strdup(fallback_charset);
		ಯದಿ (c->fallback_charset == ರಿಕ್ತ) {
			ಮುಕ್ತ(user_sentinel);
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
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
	c->title = ರಿಕ್ತ;
	c->active = 0;
	user_sentinel->callback = ರಿಕ್ತ;
	user_sentinel->pw = ರಿಕ್ತ;
	user_sentinel->next = ರಿಕ್ತ;
	c->user_list = user_sentinel;
	c->sub_status[0] = 0;
	c->locked = false;
	c->total_size = 0;
	c->http_code = 0;

	c->textsearch.string = ರಿಕ್ತ;
	c->textsearch.context = ರಿಕ್ತ;

	content_set_status(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	ಯದಿ (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);

	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ false;

	ಪ್ರತ್ಯಾಗಮ (c->handler->reformat != ರಿಕ್ತ);
}


/* exported interface documented in content/protected.h */
ಶೂನ್ಯ content_set_status(ಸಂರಚನಾ content *c, ಧ್ರುವ ವರ್ಣ *status_message)
{
	size_t len = strlen(status_message);

	ಯದಿ (len >= ಪರಿಮಾಣ(c->sub_status)) {
		len = ಪರಿಮಾಣ(c->sub_status) - 1;
	}
	memcpy(c->sub_status, status_message, len);
	c->sub_status[len] = '\0';

	content_update_status(c);
}


/* exported interface documented in content/protected.h */
ಶೂನ್ಯ content_set_ready(ಸಂರಚನಾ content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	ಪ್ರತಿಜ್ಞಾ(c->locked);
	c->locked = false;

	c->status = CONTENT_STATUS_READY;
	content_update_status(c);
	content_broadcast(c, CONTENT_ಸಂದೇಶ_READY, ರಿಕ್ತ);
}


/* exported interface documented in content/protected.h */
ಶೂನ್ಯ content_set_done(ಸಂರಚನಾ content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->status = CONTENT_STATUS_DONE;
	c->time = now_ms - c->time;
	content_update_status(c);
	content_broadcast(c, CONTENT_ಸಂದೇಶ_DONE, ರಿಕ್ತ);
}


/* exported interface documented in content/protected.h */
ಶೂನ್ಯ content_set_error(ಸಂರಚನಾ content *c)
{
	c->locked = false;
	c->status = CONTENT_STATUS_ERROR;
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ content_reformat(hlcache_handle *h, bool background,
		      ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  width, height);
}


/* exported interface documented in content/protected.h */
ಶೂನ್ಯ
content__reformat(ಸಂರಚನಾ content *c, bool background, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height)
{
	ಸಙ್ಘ content_ಸಂದೇಶ_data data;
	ಪ್ರತಿಜ್ಞಾ(c != 0);
	ಪ್ರತಿಜ್ಞಾ(c->status == CONTENT_STATUS_READY ||
	       c->status == CONTENT_STATUS_DONE);
	ಪ್ರತಿಜ್ಞಾ(c->locked == false);

	c->available_width = width;
	c->available_height = height;
	ಯದಿ (c->handler->reformat != ರಿಕ್ತ) {

		c->locked = true;
		c->handler->reformat(c, width, height);
		c->locked = false;

		data.background = background;
		content_broadcast(c, CONTENT_ಸಂದೇಶ_REFORMAT, &data);
	}
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ content_destroy(ಸಂರಚನಾ content *c)
{
	ಸಂರಚನಾ content_rfc5988_link *link;

	ಪ್ರತಿಜ್ಞಾ(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	ಪ್ರತಿಜ್ಞಾ(c->locked == false);

	ಯದಿ (c->handler->destroy != ರಿಕ್ತ)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = ರಿಕ್ತ;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	ಯಾವತ್ (link != ರಿಕ್ತ) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	ಯದಿ (c->user_list != ರಿಕ್ತ) {
		ಮುಕ್ತ(c->user_list);
	}

	/* free the title */
	ಯದಿ (c->title != ರಿಕ್ತ) {
		ಮುಕ್ತ(c->title);
	}

	/* free the fallback characterset */
	ಯದಿ (c->fallback_charset != ರಿಕ್ತ) {
		ಮುಕ್ತ(c->fallback_charset);
	}

	ಮುಕ್ತ(c);
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ
content_mouse_track(hlcache_handle *h,
		    ಸಂರಚನಾ browser_window *bw,
		    browser_mouse_state mouse,
		    ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಯದಿ (c->handler->mouse_track != ರಿಕ್ತ) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} ಅನ್ಯಥಾ {
		ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
		ಸಂದೇಶ_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(c, CONTENT_ಸಂದೇಶ_POINTER, &ಸಂದೇಶ_data);
	}


	ಪ್ರತ್ಯಾಗಮ;
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ
content_mouse_action(hlcache_handle *h,
		     ಸಂರಚನಾ browser_window *bw,
		     browser_mouse_state mouse,
		     ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಯದಿ (c->handler->mouse_action != ರಿಕ್ತ)
		c->handler->mouse_action(c, bw, mouse, x, y);

	ಪ್ರತ್ಯಾಗಮ;
}


/* exported interface documented in content/content.h */
bool content_keypress(ಸಂರಚನಾ hlcache_handle *h, uint32_t ಕೀಲಿ)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಯದಿ (c->handler->keypress != ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ c->handler->keypress(c, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ false;
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ content_request_redraw(ಸಂರಚನಾ hlcache_handle *h,
			    ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, width, height);
}


/* exported interface, documented in content/protected.h */
ಶೂನ್ಯ content__request_redraw(ಸಂರಚನಾ content *c,
			     ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height)
{
	ಸಙ್ಘ content_ಸಂದೇಶ_data data;

	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ;

	data.redraw.x = x;
	data.redraw.y = y;
	data.redraw.width = width;
	data.redraw.height = height;

	content_broadcast(c, CONTENT_ಸಂದೇಶ_REDRAW, &data);
}


/* exported interface, documented in content/content.h */
bool content_exec(ಸಂರಚನಾ hlcache_handle *h, ಧ್ರುವ ವರ್ಣ *src, size_t srclen)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಯದಿ (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (c->handler->exec == ರಿಕ್ತ) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(ಸಂರಚನಾ hlcache_handle *h)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	ಸಂರಚನಾ nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool match;

	/* Is this an internal scheme? If so, we trust here and stop */
	ಯದಿ ((lwc_string_isequal(scheme, corestring_lwc_about,
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
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* Okay, not internal, am *I* secure? */
	ಯದಿ ((lwc_string_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok)
	    && (match == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		ಪ್ರತ್ಯಾಗಮ true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	ಯದಿ (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* Otherwise try and chain through the handler */
	ಯದಿ (c != ರಿಕ್ತ && c->handler->saw_insecure_objects != ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	ಪ್ರತ್ಯಾಗಮ false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       ಸಂರಚನಾ content_redraw_data *data,
	       ಧ್ರುವ ಸಂರಚನಾ rect *clip,
	       ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಯದಿ (c->locked) {
		/* not safe to attempt redraw */
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* ensure we have a redrawable content */
	ಯದಿ (c->handler->redraw == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	ಪ್ರತ್ಯಾಗಮ c->handler->redraw(c, data, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(ಸಂರಚನಾ hlcache_handle *h,
		      ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height,
		      ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	ಸಂರಚನಾ redraw_context new_ctx = *ctx;
	ಸಂರಚನಾ rect clip;
	ಸಂರಚನಾ content_redraw_data data;
	bool plot_ok = true;

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	/* ensure it is safe to attempt redraw */
	ಯದಿ (c->locked) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* ensure we have a redrawable content */
	ಯದಿ (c->handler->redraw == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, width, height, ctx);

	ಯದಿ (ctx->plot->option_knockout) {
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
	ಯದಿ (c->width) {
		data.scale = (ಭಿನ್ನ)width / (ಭಿನ್ನ)c->width;
	} ಅನ್ಯಥಾ {
		data.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &data, &clip, &new_ctx);

	ಯದಿ (ctx->plot->option_knockout) {
		knockout_plot_end(ctx);
	}

	ಪ್ರತ್ಯಾಗಮ plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_add_user(ಸಂರಚನಾ content *c,
		 ಶೂನ್ಯ (*callback)(
				  ಸಂರಚನಾ content *c,
				  content_ಸಂದೇಶ ಸಂದೇಶ,
				  ಧ್ರುವ ಸಙ್ಘ content_ಸಂದೇಶ_data *data,
				  ಶೂನ್ಯ *pw),
		 ಶೂನ್ಯ *pw)
{
	ಸಂರಚನಾ content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ content_user));
	ಯದಿ (!user)
		ಪ್ರತ್ಯಾಗಮ false;
	user->callback = callback;
	user->pw = pw;
	user->next = c->user_list->next;
	c->user_list->next = user;

	ಯದಿ (c->handler->add_user != ರಿಕ್ತ)
		c->handler->add_user(c);

	ಪ್ರತ್ಯಾಗಮ true;
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ
content_remove_user(ಸಂರಚನಾ content *c,
		    ಶೂನ್ಯ (*callback)(
				     ಸಂರಚನಾ content *c,
				     content_ಸಂದೇಶ ಸಂದೇಶ,
				     ಧ್ರುವ ಸಙ್ಘ content_ಸಂದೇಶ_data *data,
				     ಶೂನ್ಯ *pw),
		    ಶೂನ್ಯ *pw)
{
	ಸಂರಚನಾ content_user *user, *next;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	ಪ್ರತಿ (user = c->user_list; user->next != 0 &&
		     !(user->next->callback == callback &&
		       user->next->pw == pw); user = user->next)
		;
	ಯದಿ (user->next == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		ಪ್ರತಿಜ್ಞಾ(0);
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (c->handler->remove_user != ರಿಕ್ತ)
		c->handler->remove_user(c);

	next = user->next;
	user->next = next->next;
	ಮುಕ್ತ(next);
}


/* exported interface documented in content/content.h */
uint32_t content_ಎಣಿಕೆ_users(ಸಂರಚನಾ content *c)
{
	ಸಂರಚನಾ content_user *user;
	uint32_t counter = 0;

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಪ್ರತಿ (user = c->user_list; user != ರಿಕ್ತ; user = user->next)
		counter += 1;

	ಪ್ರತಿಜ್ಞಾ(counter > 0);

	ಪ್ರತ್ಯಾಗಮ counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(ಸಂರಚನಾ content *c, bool quirks)
{
	ಯದಿ (c->handler->matches_quirks == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ true;

	ಪ್ರತ್ಯಾಗಮ c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(ಸಂರಚನಾ content *c)
{
	ಪ್ರತ್ಯಾಗಮ c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
ಶೂನ್ಯ content_broadcast(ಸಂರಚನಾ content *c, content_ಸಂದೇಶ ಸಂದೇಶ,
		       ಧ್ರುವ ಸಙ್ಘ content_ಸಂದೇಶ_data *data)
{
	ಸಂರಚನಾ content_user *user, *next;
	ಪ್ರತಿಜ್ಞಾ(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, ಸಂದೇಶ);
	ಪ್ರತಿ (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		ಯದಿ (user->callback != 0)
			user->callback(c, ಸಂದೇಶ, data, user->pw);
	}
}


/* exported interface documented in content_protected.h */
ಶೂನ್ಯ
content_broadcast_error(ಸಂರಚನಾ content *c, nserror errorcode, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ)
{
	ಸಂರಚನಾ content_user *user, *next;
	ಸಙ್ಘ content_ಸಂದೇಶ_data data;

	ಪ್ರತಿಜ್ಞಾ(c);

	data.errordata.errorcode = errorcode;
	data.errordata.errormsg = ಸಂದೇಶ;

	ಪ್ರತಿ (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		ಯದಿ (user->callback != 0) {
			user->callback(c, CONTENT_ಸಂದೇಶ_ERROR,
				       &data, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     ಸಂರಚನಾ browser_window *bw,
	     ಸಂರಚನಾ content *page,
	     ಸಂರಚನಾ object_params *params)
{
	ಸಂರಚನಾ content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	ಯದಿ (c->handler->open != ರಿಕ್ತ) {
		res = c->handler->open(c, bw, page, params);
	} ಅನ್ಯಥಾ {
		res = NSERROR_OK;
	}
	ಪ್ರತ್ಯಾಗಮ res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	ಸಂರಚನಾ content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	ಯದಿ (c == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_BAD_PARAMETER;
	}

	ಯದಿ ((c->status != CONTENT_STATUS_READY) &&
	    (c->status != CONTENT_STATUS_DONE)) {
		/* status is not read or done so nothing to do */
		ಪ್ರತ್ಯಾಗಮ NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	ಯದಿ (c->textsearch.context != ರಿಕ್ತ) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ರಿಕ್ತ;
	}

	ಯದಿ (c->handler->close != ರಿಕ್ತ) {
		res = c->handler->close(c);
	} ಅನ್ಯಥಾ {
		res = NSERROR_OK;
	}
	ಪ್ರತ್ಯಾಗಮ res;
}


/* exported interface, documented in content/content.h */
ಶೂನ್ಯ content_clear_selection(hlcache_handle *h)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಯದಿ (c->handler->get_selection != ರಿಕ್ತ)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
ವರ್ಣ * content_get_selection(hlcache_handle *h)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಯದಿ (c->handler->get_selection != ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ c->handler->get_selection(c);
	ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(ಸಂರಚನಾ hlcache_handle *h,
			       ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
			       ಸಂರಚನಾ browser_window_features *data)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಯದಿ (c->handler->get_contextual_content != ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ c->handler->get_contextual_content(c, x, y, data);
	}

	data->object = h;
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_point(ಸಂರಚನಾ hlcache_handle *h,
			ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
			ಪೂರ್ಣಾಙ್ಕ scrx, ಪೂರ್ಣಾಙ್ಕ scry)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಯದಿ (c->handler->scroll_at_point != ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ c->handler->scroll_at_point(c, x, y, scrx, scry);

	ಪ್ರತ್ಯಾಗಮ false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_point(ಸಂರಚನಾ hlcache_handle *h,
			   ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
			   ವರ್ಣ *file)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಯದಿ (c->handler->drop_file_at_point != ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ c->handler->drop_file_at_point(c, x, y, file);

	ಪ್ರತ್ಯಾಗಮ false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(ಸಂರಚನಾ hlcache_handle *h, FILE *f, ಗಣನಾ content_debug op)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಯದಿ (c->handler->debug_dump == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOT_IMPLEMENTED;
	}

	ಪ್ರತ್ಯಾಗಮ c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(ಸಂರಚನಾ hlcache_handle *h, ಗಣನಾ content_debug op)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);

	ಯದಿ (c == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_BAD_PARAMETER;
	}

	ಯದಿ (c->handler->debug == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOT_IMPLEMENTED;
	}

	ಪ್ರತ್ಯಾಗಮ c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
ಸಂರಚನಾ content_rfc5988_link *
content_find_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);
	ಸಂರಚನಾ content_rfc5988_link *link = c->links;
	bool rel_match = false;

	ಯಾವತ್ (link != ರಿಕ್ತ) {
		ಯದಿ (lwc_string_caseless_isequal(link->rel, rel,
						&rel_match) == lwc_error_ok && rel_match) {
			ಭಙ್ಗ;
		}
		link = link->next;
	}
	ಪ್ರತ್ಯಾಗಮ link;
}


/* exported interface documented in content/protected.h */
ಸಂರಚನಾ content_rfc5988_link *
content__free_rfc5988_link(ಸಂರಚನಾ content_rfc5988_link *link)
{
	ಸಂರಚನಾ content_rfc5988_link *next;

	next = link->next;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	ಮುಕ್ತ(link);

	ಪ್ರತ್ಯಾಗಮ next;
}


/* exported interface documented in content/protected.h */
bool
content__add_rfc5988_link(ಸಂರಚನಾ content *c,
			  ಧ್ರುವ ಸಂರಚನಾ content_rfc5988_link *link)
{
	ಸಂರಚನಾ content_rfc5988_link *newlink;
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;

	/* a link relation must be present for it to be a link */
	ಯದಿ (link->rel == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* a link href must be present for it to be a link */
	ಯದಿ (link->href == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	newlink = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ content_rfc5988_link));
	ಯದಿ (newlink == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	ಯದಿ (link->hreflang != ರಿಕ್ತ) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	ಯದಿ (link->type != ರಿಕ್ತ) {
		newlink->type = lwc_string_ref(link->type);
	}
	ಯದಿ (link->media != ರಿಕ್ತ) {
		newlink->media = lwc_string_ref(link->media);
	}
	ಯದಿ (link->sizes != ರಿಕ್ತ) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->next = c->links;
	c->links = newlink;

	/* broadcast the data */
	ಸಂದೇಶ_data.rfc5988_link = newlink;
	content_broadcast(c, CONTENT_ಸಂದೇಶ_LINK, &ಸಂದೇಶ_data);

	ಪ್ರತ್ಯಾಗಮ true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(ಸಂರಚನಾ content *c)
{
	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);

	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ CONTENT_NONE;

	ಪ್ರತ್ಯಾಗಮ c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	ಪ್ರತ್ಯಾಗಮ content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(ಸಂರಚನಾ content *c)
{
	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content__set_title(ಸಂರಚನಾ content *c, ಧ್ರುವ ವರ್ಣ *title)
{
	ವರ್ಣ *new_title = strdup(title);
	ಯದಿ (new_title == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (c->title != ರಿಕ್ತ)
		ಮುಕ್ತ(c->title);

	c->title = new_title;

	ಪ್ರತ್ಯಾಗಮ true;
}


/* exported interface documented in content/content.h */
ಧ್ರುವ ವರ್ಣ *content_get_title(hlcache_handle *h)
{
	ಪ್ರತ್ಯಾಗಮ content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ಧ್ರುವ ವರ್ಣ *content__get_title(ಸಂರಚನಾ content *c)
{
	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ c->title != ರಿಕ್ತ ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_status content_get_status(hlcache_handle *h)
{
	ಪ್ರತ್ಯಾಗಮ content__get_status(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_status content__get_status(ಸಂರಚನಾ content *c)
{
	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ CONTENT_STATUS_ERROR;

	ಪ್ರತ್ಯಾಗಮ c->status;
}


/* exported interface documented in content/content.h */
ಧ್ರುವ ವರ್ಣ *content_get_status_message(hlcache_handle *h)
{
	ಪ್ರತ್ಯಾಗಮ content__get_status_message(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ಧ್ರುವ ವರ್ಣ *content__get_status_message(ಸಂರಚನಾ content *c)
{
	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ c->status_message;
}


/* exported interface documented in content/content.h */
ಪೂರ್ಣಾಙ್ಕ content_get_width(hlcache_handle *h)
{
	ಪ್ರತ್ಯಾಗಮ content__get_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ಪೂರ್ಣಾಙ್ಕ content__get_width(ಸಂರಚನಾ content *c)
{
	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ c->width;
}


/* exported interface documented in content/content.h */
ಪೂರ್ಣಾಙ್ಕ content_get_height(hlcache_handle *h)
{
	ಪ್ರತ್ಯಾಗಮ content__get_height(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ಪೂರ್ಣಾಙ್ಕ content__get_height(ಸಂರಚನಾ content *c)
{
	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ c->height;
}


/* exported interface documented in content/content.h */
ಪೂರ್ಣಾಙ್ಕ content_get_available_width(hlcache_handle *h)
{
	ಪ್ರತ್ಯಾಗಮ content__get_available_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ಪೂರ್ಣಾಙ್ಕ content__get_available_width(ಸಂರಚನಾ content *c)
{
	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ c->available_width;
}


/* exported interface documented in content/content.h */
ಧ್ರುವ uint8_t *content_get_source_data(hlcache_handle *h, size_t *size)
{
	ಪ್ರತ್ಯಾಗಮ content__get_source_data(hlcache_handle_get_content(h), size);
}


/* exported interface documented in content/content_protected.h */
ಧ್ರುವ uint8_t *content__get_source_data(ಸಂರಚನಾ content *c, size_t *size)
{
	ಪ್ರತಿಜ್ಞಾ(size != ರಿಕ್ತ);

	/** \todo check if the content check should be an assert */
	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ llcache_handle_get_source_data(c->llcache, size);
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ content_invalidate_reuse_data(hlcache_handle *h)
{
	content__invalidate_reuse_data(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ಶೂನ್ಯ content__invalidate_reuse_data(ಸಂರಚನಾ content *c)
{
	ಯದಿ (c == ರಿಕ್ತ || c->llcache == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_data(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	ಪ್ರತ್ಯಾಗಮ content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(ಸಂರಚನಾ content *c)
{
	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ c->refresh;
}


/* exported interface documented in content/content.h */
ಸಂರಚನಾ bitmap *content_get_bitmap(hlcache_handle *h)
{
	ಪ್ರತ್ಯಾಗಮ content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ಸಂರಚನಾ bitmap *content__get_bitmap(ಸಂರಚನಾ content *c)
{
	ಸಂರಚನಾ bitmap *bitmap = ರಿಕ್ತ;

	ಯದಿ ((c != ರಿಕ್ತ) &&
	    (c->handler != ರಿಕ್ತ) &&
	    (c->handler->type != ರಿಕ್ತ) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_internal != ರಿಕ್ತ) ) {
		bitmap = c->handler->get_internal(c, ರಿಕ್ತ);
	}

	ಪ್ರತ್ಯಾಗಮ bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	ಪ್ರತ್ಯಾಗಮ content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(ಸಂರಚನಾ content *c)
{
	ಯದಿ ((c != ರಿಕ್ತ) &&
	    (c->handler != ರಿಕ್ತ) &&
	    (c->handler->is_opaque != ರಿಕ್ತ)) {
		ಪ್ರತ್ಯಾಗಮ c->handler->is_opaque(c);
	}

	ಪ್ರತ್ಯಾಗಮ false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	ಸಂರಚನಾ content *c = hlcache_handle_get_content(h);

	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ false;

	ಪ್ರತ್ಯಾಗಮ c->quirks;
}


/* exported interface documented in content/content.h */
ಧ್ರುವ ವರ್ಣ *
content_get_encoding(hlcache_handle *h, ಗಣನಾ content_encoding_type op)
{
	ಪ್ರತ್ಯಾಗಮ content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
ಧ್ರುವ ವರ್ಣ *
content__get_encoding(ಸಂರಚನಾ content *c, ಗಣನಾ content_encoding_type op)
{
	ಧ್ರುವ ವರ್ಣ *encoding = ರಿಕ್ತ;

	ಯದಿ ((c != ರಿಕ್ತ) &&
	    (c->handler != ರಿಕ್ತ) &&
	    (c->handler->get_encoding != ರಿಕ್ತ) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	ಪ್ರತ್ಯಾಗಮ encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	ಪ್ರತ್ಯಾಗಮ content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(ಸಂರಚನಾ content *c)
{
	ಪ್ರತ್ಯಾಗಮ c->locked;
}


/* exported interface documented in content/content.h */
ಧ್ರುವ llcache_handle *content_get_llcache_handle(ಸಂರಚನಾ content *c)
{
	ಯದಿ (c == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ c->llcache;
}


/* exported interface documented in content/protected.h */
ಸಂರಚನಾ content *content_clone(ಸಂರಚನಾ content *c)
{
	ಸಂರಚನಾ content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	ಯದಿ (error != NSERROR_OK)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(ಧ್ರುವ ಸಂರಚನಾ content *c, ಸಂರಚನಾ content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	ಯದಿ (error != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ error;
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

	ಯದಿ (c->fallback_charset != ರಿಕ್ತ) {
		nc->fallback_charset = strdup(c->fallback_charset);
		ಯದಿ (nc->fallback_charset == ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}
	}

	ಯದಿ (c->refresh != ರಿಕ್ತ) {
		nc->refresh = nsurl_ref(c->refresh);
		ಯದಿ (nc->refresh == ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_time = c->reformat_time;
	nc->size = c->size;

	ಯದಿ (c->title != ರಿಕ್ತ) {
		nc->title = strdup(c->title);
		ಯದಿ (nc->title == ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}
	}

	nc->active = c->active;

	nc->user_list = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ content_user));
	ಯದಿ (nc->user_list == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	memcpy(&(nc->status_message), &(c->status_message), 120);
	memcpy(&(nc->sub_status), &(c->sub_status), 80);

	nc->locked = c->locked;
	nc->total_size = c->total_size;
	nc->http_code = c->http_code;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(ಸಂರಚನಾ content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	ಯದಿ (c->handler->stop != ರಿಕ್ತ)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	ಪ್ರತ್ಯಾಗಮ llcache_handle_abort(c->llcache);
}
