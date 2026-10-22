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

#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <nsutils/time.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/browser_window.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "desktop/knockout.h"

#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/textsearch.h"
#ଅନ୍ତର୍ଭାଵ "content/content_debug.h"
#ଅନ୍ତର୍ଭାଵ "content/hlcache.h"
#ଅନ୍ତର୍ଭାଵ "content/urldb.h"

#define URL_FMT_SPC "%.140s"

ଅଚଳ ଅକ୍ଷର * ଅଚଳ content_status_name[] = {
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
ସ୍ଥିର ଶୂନ୍ୟ content_convert(ସଂରଚନା content *c)
{
	ପ୍ରତିଜ୍ଞା(c);
	ପ୍ରତିଜ୍ଞା(c->status == CONTENT_STATUS_LOADING ||
	       c->status == CONTENT_STATUS_ERROR);

	ଯଦି (c->status != CONTENT_STATUS_LOADING)
		ଫେରନ୍ତୁ;

	ଯଦି (c->locked == true)
		ଫେରନ୍ତୁ;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	ଯଦି (c->handler->data_complete != ଖାଲି) {
		c->locked = true;
		ଯଦି (c->handler->data_complete(c) == false) {
			content_set_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} ଅନ୍ୟଥା {
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
ସ୍ଥିର nserror
content_llcache_callback(llcache_handle *llcache,
			 ଅଚଳ llcache_event *event, ଶୂନ୍ୟ *pw)
{
	ସଂରଚନା content *c = pw;
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	nserror error = NSERROR_OK;

	ବାଛ (event->type) {
	ସ୍ଥିତି LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		ଭାଙ୍ଗ;
	ସ୍ଥିତି LLCACHE_EVENT_HAD_HEADERS:
		/* Will never happen: handled in hlcache */
		ଭାଙ୍ଗ;
	ସ୍ଥିତି LLCACHE_EVENT_HAD_DATA:
		ଯଦି (c->handler->process_data != ଖାଲି) {
			ଯଦି (c->handler->process_data(c,
						     (ଅଚଳ ଅକ୍ଷର *) event->data.data.buf,
						     event->data.data.len) == false) {
				llcache_handle_abort(c->llcache);
				c->status = CONTENT_STATUS_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି LLCACHE_EVENT_DONE:
		{
			size_t source_size;

			(ଶୂନ୍ୟ) llcache_handle_get_source_data(llcache, &source_size);

			content_set_status(c, messages_get("Processing"));
			ସନ୍ଦେଶ_data.explicit_status_text = ଖାଲି;
			content_broadcast(c, CONTENT_ସନ୍ଦେଶ_STATUS, &ସନ୍ଦେଶ_data);

			content_convert(c);
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->status = CONTENT_STATUS_ERROR;
		ସନ୍ଦେଶ_data.errordata.errorcode = event->data.error.code;
		ସନ୍ଦେଶ_data.errordata.errormsg = event->data.error.ସନ୍ଦେଶ;
		content_broadcast(c, CONTENT_ସନ୍ଦେଶ_ERROR, &ସନ୍ଦେଶ_data);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି LLCACHE_EVENT_PROGRESS:
		content_set_status(c, event->data.progress_ସନ୍ଦେଶ);
		ସନ୍ଦେଶ_data.explicit_status_text = ଖାଲି;
		content_broadcast(c, CONTENT_ସନ୍ଦେଶ_STATUS, &ସନ୍ଦେଶ_data);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି LLCACHE_EVENT_REDIRECT:
		ସନ୍ଦେଶ_data.redirect.from = event->data.redirect.from;
		ସନ୍ଦେଶ_data.redirect.to = event->data.redirect.to;
		content_broadcast(c, CONTENT_ସନ୍ଦେଶ_REDIRECT, &ସନ୍ଦେଶ_data);
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
ସ୍ଥିର ଶୂନ୍ୟ content_update_status(ସଂରଚନା content *c)
{
	ଯଦି (c->status == CONTENT_STATUS_LOADING ||
	    c->status == CONTENT_STATUS_READY) {
		/* Not done yet */
		snprintf(c->status_message, ଆକାର (c->status_message),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_status[0] != '\0' ? ", " : " ",
			 c->sub_status);
	} ଅନ୍ୟଥା {
		snprintf(c->status_message, ଆକାର (c->status_message),
			 "%s (%.1fs)", messages_get("Done"),
			 (ଦଶମିକ) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(ସଂରଚନା content *c,
	      ଅଚଳ content_handler *handler,
	      lwc_string *imime_type,
	      ଅଚଳ ସଂରଚନା http_parameter *params,
	      llcache_handle *llcache,
	      ଅଚଳ ଅକ୍ଷର *fallback_charset,
	      bool quirks)
{
	ସଂରଚନା content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = calloc(1, ଆକାର(ସଂରଚନା content_user));
	ଯଦି (user_sentinel == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ଯଦି (fallback_charset != ଖାଲି) {
		c->fallback_charset = strdup(fallback_charset);
		ଯଦି (c->fallback_charset == ଖାଲି) {
			ମୁକ୍ତ(user_sentinel);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
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
	c->title = ଖାଲି;
	c->active = 0;
	user_sentinel->callback = ଖାଲି;
	user_sentinel->pw = ଖାଲି;
	user_sentinel->next = ଖାଲି;
	c->user_list = user_sentinel;
	c->sub_status[0] = 0;
	c->locked = false;
	c->total_size = 0;
	c->http_code = 0;

	c->textsearch.string = ଖାଲି;
	c->textsearch.context = ଖାଲି;

	content_set_status(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	ଯଦି (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		ଫେରନ୍ତୁ error;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);

	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ false;

	ଫେରନ୍ତୁ (c->handler->reformat != ଖାଲି);
}


/* exported interface documented in content/protected.h */
ଶୂନ୍ୟ content_set_status(ସଂରଚନା content *c, ଅଚଳ ଅକ୍ଷର *status_message)
{
	size_t len = strlen(status_message);

	ଯଦି (len >= ଆକାର(c->sub_status)) {
		len = ଆକାର(c->sub_status) - 1;
	}
	memcpy(c->sub_status, status_message, len);
	c->sub_status[len] = '\0';

	content_update_status(c);
}


/* exported interface documented in content/protected.h */
ଶୂନ୍ୟ content_set_ready(ସଂରଚନା content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	ପ୍ରତିଜ୍ଞା(c->locked);
	c->locked = false;

	c->status = CONTENT_STATUS_READY;
	content_update_status(c);
	content_broadcast(c, CONTENT_ସନ୍ଦେଶ_READY, ଖାଲି);
}


/* exported interface documented in content/protected.h */
ଶୂନ୍ୟ content_set_done(ସଂରଚନା content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->status = CONTENT_STATUS_DONE;
	c->time = now_ms - c->time;
	content_update_status(c);
	content_broadcast(c, CONTENT_ସନ୍ଦେଶ_DONE, ଖାଲି);
}


/* exported interface documented in content/protected.h */
ଶୂନ୍ୟ content_set_error(ସଂରଚନା content *c)
{
	c->locked = false;
	c->status = CONTENT_STATUS_ERROR;
}


/* exported interface documented in content/content.h */
ଶୂନ୍ୟ content_reformat(hlcache_handle *h, bool background,
		      ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  width, height);
}


/* exported interface documented in content/protected.h */
ଶୂନ୍ୟ
content__reformat(ସଂରଚନା content *c, bool background, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height)
{
	ସଂଘ content_ସନ୍ଦେଶ_data data;
	ପ୍ରତିଜ୍ଞା(c != 0);
	ପ୍ରତିଜ୍ଞା(c->status == CONTENT_STATUS_READY ||
	       c->status == CONTENT_STATUS_DONE);
	ପ୍ରତିଜ୍ଞା(c->locked == false);

	c->available_width = width;
	c->available_height = height;
	ଯଦି (c->handler->reformat != ଖାଲି) {

		c->locked = true;
		c->handler->reformat(c, width, height);
		c->locked = false;

		data.background = background;
		content_broadcast(c, CONTENT_ସନ୍ଦେଶ_REFORMAT, &data);
	}
}


/* exported interface documented in content/content.h */
ଶୂନ୍ୟ content_destroy(ସଂରଚନା content *c)
{
	ସଂରଚନା content_rfc5988_link *link;

	ପ୍ରତିଜ୍ଞା(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	ପ୍ରତିଜ୍ଞା(c->locked == false);

	ଯଦି (c->handler->destroy != ଖାଲି)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = ଖାଲି;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	ଯେତେବେଳେ (link != ଖାଲି) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	ଯଦି (c->user_list != ଖାଲି) {
		ମୁକ୍ତ(c->user_list);
	}

	/* free the title */
	ଯଦି (c->title != ଖାଲି) {
		ମୁକ୍ତ(c->title);
	}

	/* free the fallback characterset */
	ଯଦି (c->fallback_charset != ଖାଲି) {
		ମୁକ୍ତ(c->fallback_charset);
	}

	ମୁକ୍ତ(c);
}


/* exported interface documented in content/content.h */
ଶୂନ୍ୟ
content_mouse_track(hlcache_handle *h,
		    ସଂରଚନା browser_window *bw,
		    browser_mouse_state mouse,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଯଦି (c->handler->mouse_track != ଖାଲି) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} ଅନ୍ୟଥା {
		ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
		ସନ୍ଦେଶ_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(c, CONTENT_ସନ୍ଦେଶ_POINTER, &ସନ୍ଦେଶ_data);
	}


	ଫେରନ୍ତୁ;
}


/* exported interface documented in content/content.h */
ଶୂନ୍ୟ
content_mouse_action(hlcache_handle *h,
		     ସଂରଚନା browser_window *bw,
		     browser_mouse_state mouse,
		     ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଯଦି (c->handler->mouse_action != ଖାଲି)
		c->handler->mouse_action(c, bw, mouse, x, y);

	ଫେରନ୍ତୁ;
}


/* exported interface documented in content/content.h */
bool content_keypress(ସଂରଚନା hlcache_handle *h, uint32_t ଚାବି)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଯଦି (c->handler->keypress != ଖାଲି)
		ଫେରନ୍ତୁ c->handler->keypress(c, ଚାବି);

	ଫେରନ୍ତୁ false;
}


/* exported interface documented in content/content.h */
ଶୂନ୍ୟ content_request_redraw(ସଂରଚନା hlcache_handle *h,
			    ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, width, height);
}


/* exported interface, documented in content/protected.h */
ଶୂନ୍ୟ content__request_redraw(ସଂରଚନା content *c,
			     ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height)
{
	ସଂଘ content_ସନ୍ଦେଶ_data data;

	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ;

	data.redraw.x = x;
	data.redraw.y = y;
	data.redraw.width = width;
	data.redraw.height = height;

	content_broadcast(c, CONTENT_ସନ୍ଦେଶ_REDRAW, &data);
}


/* exported interface, documented in content/content.h */
bool content_exec(ସଂରଚନା hlcache_handle *h, ଅଚଳ ଅକ୍ଷର *src, size_t srclen)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଯଦି (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (c->handler->exec == ଖାଲି) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(ସଂରଚନା hlcache_handle *h)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	ସଂରଚନା nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool match;

	/* Is this an internal scheme? If so, we trust here and stop */
	ଯଦି ((lwc_string_isequal(scheme, corestring_lwc_about,
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
		ଫେରନ୍ତୁ false;
	}

	/* Okay, not internal, am *I* secure? */
	ଯଦି ((lwc_string_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok)
	    && (match == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		ଫେରନ୍ତୁ true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	ଯଦି (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		ଫେରନ୍ତୁ true;
	}

	/* Otherwise try and chain through the handler */
	ଯଦି (c != ଖାଲି && c->handler->saw_insecure_objects != ଖାଲି) {
		ଫେରନ୍ତୁ c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	ଫେରନ୍ତୁ false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       ସଂରଚନା content_redraw_data *data,
	       ଅଚଳ ସଂରଚନା rect *clip,
	       ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଯଦି (c->locked) {
		/* not safe to attempt redraw */
		ଫେରନ୍ତୁ true;
	}

	/* ensure we have a redrawable content */
	ଯଦି (c->handler->redraw == ଖାଲି) {
		ଫେରନ୍ତୁ true;
	}

	ଫେରନ୍ତୁ c->handler->redraw(c, data, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(ସଂରଚନା hlcache_handle *h,
		      ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height,
		      ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	ସଂରଚନା redraw_context new_ctx = *ctx;
	ସଂରଚନା rect clip;
	ସଂରଚନା content_redraw_data data;
	bool plot_ok = true;

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	/* ensure it is safe to attempt redraw */
	ଯଦି (c->locked) {
		ଫେରନ୍ତୁ true;
	}

	/* ensure we have a redrawable content */
	ଯଦି (c->handler->redraw == ଖାଲି) {
		ଫେରନ୍ତୁ true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, width, height, ctx);

	ଯଦି (ctx->plot->option_knockout) {
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
	ଯଦି (c->width) {
		data.scale = (ଦଶମିକ)width / (ଦଶମିକ)c->width;
	} ଅନ୍ୟଥା {
		data.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &data, &clip, &new_ctx);

	ଯଦି (ctx->plot->option_knockout) {
		knockout_plot_end(ctx);
	}

	ଫେରନ୍ତୁ plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_add_user(ସଂରଚନା content *c,
		 ଶୂନ୍ୟ (*callback)(
				  ସଂରଚନା content *c,
				  content_ସନ୍ଦେଶ ସନ୍ଦେଶ,
				  ଅଚଳ ସଂଘ content_ସନ୍ଦେଶ_data *data,
				  ଶୂନ୍ୟ *pw),
		 ଶୂନ୍ୟ *pw)
{
	ସଂରଚନା content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = malloc(ଆକାର(ସଂରଚନା content_user));
	ଯଦି (!user)
		ଫେରନ୍ତୁ false;
	user->callback = callback;
	user->pw = pw;
	user->next = c->user_list->next;
	c->user_list->next = user;

	ଯଦି (c->handler->add_user != ଖାଲି)
		c->handler->add_user(c);

	ଫେରନ୍ତୁ true;
}


/* exported interface documented in content/content.h */
ଶୂନ୍ୟ
content_remove_user(ସଂରଚନା content *c,
		    ଶୂନ୍ୟ (*callback)(
				     ସଂରଚନା content *c,
				     content_ସନ୍ଦେଶ ସନ୍ଦେଶ,
				     ଅଚଳ ସଂଘ content_ସନ୍ଦେଶ_data *data,
				     ଶୂନ୍ୟ *pw),
		    ଶୂନ୍ୟ *pw)
{
	ସଂରଚନା content_user *user, *next;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	ପାଇଁ (user = c->user_list; user->next != 0 &&
		     !(user->next->callback == callback &&
		       user->next->pw == pw); user = user->next)
		;
	ଯଦି (user->next == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		ପ୍ରତିଜ୍ଞା(0);
		ଫେରନ୍ତୁ;
	}

	ଯଦି (c->handler->remove_user != ଖାଲି)
		c->handler->remove_user(c);

	next = user->next;
	user->next = next->next;
	ମୁକ୍ତ(next);
}


/* exported interface documented in content/content.h */
uint32_t content_ଗଣନା_users(ସଂରଚନା content *c)
{
	ସଂରଚନା content_user *user;
	uint32_t counter = 0;

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ପାଇଁ (user = c->user_list; user != ଖାଲି; user = user->next)
		counter += 1;

	ପ୍ରତିଜ୍ଞା(counter > 0);

	ଫେରନ୍ତୁ counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(ସଂରଚନା content *c, bool quirks)
{
	ଯଦି (c->handler->matches_quirks == ଖାଲି)
		ଫେରନ୍ତୁ true;

	ଫେରନ୍ତୁ c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(ସଂରଚନା content *c)
{
	ଫେରନ୍ତୁ c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
ଶୂନ୍ୟ content_broadcast(ସଂରଚନା content *c, content_ସନ୍ଦେଶ ସନ୍ଦେଶ,
		       ଅଚଳ ସଂଘ content_ସନ୍ଦେଶ_data *data)
{
	ସଂରଚନା content_user *user, *next;
	ପ୍ରତିଜ୍ଞା(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, ସନ୍ଦେଶ);
	ପାଇଁ (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		ଯଦି (user->callback != 0)
			user->callback(c, ସନ୍ଦେଶ, data, user->pw);
	}
}


/* exported interface documented in content_protected.h */
ଶୂନ୍ୟ
content_broadcast_error(ସଂରଚନା content *c, nserror errorcode, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ)
{
	ସଂରଚନା content_user *user, *next;
	ସଂଘ content_ସନ୍ଦେଶ_data data;

	ପ୍ରତିଜ୍ଞା(c);

	data.errordata.errorcode = errorcode;
	data.errordata.errormsg = ସନ୍ଦେଶ;

	ପାଇଁ (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		ଯଦି (user->callback != 0) {
			user->callback(c, CONTENT_ସନ୍ଦେଶ_ERROR,
				       &data, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     ସଂରଚନା browser_window *bw,
	     ସଂରଚନା content *page,
	     ସଂରଚନା object_params *params)
{
	ସଂରଚନା content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	ପ୍ରତିଜ୍ଞା(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	ଯଦି (c->handler->open != ଖାଲି) {
		res = c->handler->open(c, bw, page, params);
	} ଅନ୍ୟଥା {
		res = NSERROR_OK;
	}
	ଫେରନ୍ତୁ res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	ସଂରଚନା content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	ଯଦି (c == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_BAD_PARAMETER;
	}

	ଯଦି ((c->status != CONTENT_STATUS_READY) &&
	    (c->status != CONTENT_STATUS_DONE)) {
		/* status is not read or done so nothing to do */
		ଫେରନ୍ତୁ NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	ଯଦି (c->textsearch.context != ଖାଲି) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ଖାଲି;
	}

	ଯଦି (c->handler->close != ଖାଲି) {
		res = c->handler->close(c);
	} ଅନ୍ୟଥା {
		res = NSERROR_OK;
	}
	ଫେରନ୍ତୁ res;
}


/* exported interface, documented in content/content.h */
ଶୂନ୍ୟ content_clear_selection(hlcache_handle *h)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	ପ୍ରତିଜ୍ଞା(c != 0);

	ଯଦି (c->handler->get_selection != ଖାଲି)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
ଅକ୍ଷର * content_get_selection(hlcache_handle *h)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	ପ୍ରତିଜ୍ଞା(c != 0);

	ଯଦି (c->handler->get_selection != ଖାଲି)
		ଫେରନ୍ତୁ c->handler->get_selection(c);
	ଅନ୍ୟଥା
		ଫେରନ୍ତୁ ଖାଲି;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(ସଂରଚନା hlcache_handle *h,
			       ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
			       ସଂରଚନା browser_window_features *data)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	ପ୍ରତିଜ୍ଞା(c != 0);

	ଯଦି (c->handler->get_contextual_content != ଖାଲି) {
		ଫେରନ୍ତୁ c->handler->get_contextual_content(c, x, y, data);
	}

	data->object = h;
	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_point(ସଂରଚନା hlcache_handle *h,
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା scrx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା scry)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	ପ୍ରତିଜ୍ଞା(c != 0);

	ଯଦି (c->handler->scroll_at_point != ଖାଲି)
		ଫେରନ୍ତୁ c->handler->scroll_at_point(c, x, y, scrx, scry);

	ଫେରନ୍ତୁ false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_point(ସଂରଚନା hlcache_handle *h,
			   ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
			   ଅକ୍ଷର *file)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	ପ୍ରତିଜ୍ଞା(c != 0);

	ଯଦି (c->handler->drop_file_at_point != ଖାଲି)
		ଫେରନ୍ତୁ c->handler->drop_file_at_point(c, x, y, file);

	ଫେରନ୍ତୁ false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(ସଂରଚନା hlcache_handle *h, FILE *f, ଗଣନା content_debug op)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	ପ୍ରତିଜ୍ଞା(c != 0);

	ଯଦି (c->handler->debug_dump == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOT_IMPLEMENTED;
	}

	ଫେରନ୍ତୁ c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(ସଂରଚନା hlcache_handle *h, ଗଣନା content_debug op)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);

	ଯଦି (c == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_BAD_PARAMETER;
	}

	ଯଦି (c->handler->debug == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOT_IMPLEMENTED;
	}

	ଫେରନ୍ତୁ c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
ସଂରଚନା content_rfc5988_link *
content_find_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);
	ସଂରଚନା content_rfc5988_link *link = c->links;
	bool rel_match = false;

	ଯେତେବେଳେ (link != ଖାଲି) {
		ଯଦି (lwc_string_caseless_isequal(link->rel, rel,
						&rel_match) == lwc_error_ok && rel_match) {
			ଭାଙ୍ଗ;
		}
		link = link->next;
	}
	ଫେରନ୍ତୁ link;
}


/* exported interface documented in content/protected.h */
ସଂରଚନା content_rfc5988_link *
content__free_rfc5988_link(ସଂରଚନା content_rfc5988_link *link)
{
	ସଂରଚନା content_rfc5988_link *next;

	next = link->next;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	ମୁକ୍ତ(link);

	ଫେରନ୍ତୁ next;
}


/* exported interface documented in content/protected.h */
bool
content__add_rfc5988_link(ସଂରଚନା content *c,
			  ଅଚଳ ସଂରଚନା content_rfc5988_link *link)
{
	ସଂରଚନା content_rfc5988_link *newlink;
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;

	/* a link relation must be present for it to be a link */
	ଯଦି (link->rel == ଖାଲି) {
		ଫେରନ୍ତୁ false;
	}

	/* a link href must be present for it to be a link */
	ଯଦି (link->href == ଖାଲି) {
		ଫେରନ୍ତୁ false;
	}

	newlink = calloc(1, ଆକାର(ସଂରଚନା content_rfc5988_link));
	ଯଦି (newlink == ଖାଲି) {
		ଫେରନ୍ତୁ false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	ଯଦି (link->hreflang != ଖାଲି) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	ଯଦି (link->type != ଖାଲି) {
		newlink->type = lwc_string_ref(link->type);
	}
	ଯଦି (link->media != ଖାଲି) {
		newlink->media = lwc_string_ref(link->media);
	}
	ଯଦି (link->sizes != ଖାଲି) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->next = c->links;
	c->links = newlink;

	/* broadcast the data */
	ସନ୍ଦେଶ_data.rfc5988_link = newlink;
	content_broadcast(c, CONTENT_ସନ୍ଦେଶ_LINK, &ସନ୍ଦେଶ_data);

	ଫେରନ୍ତୁ true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(ସଂରଚନା content *c)
{
	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);

	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ CONTENT_NONE;

	ଫେରନ୍ତୁ c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	ଫେରନ୍ତୁ content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(ସଂରଚନା content *c)
{
	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content__set_title(ସଂରଚନା content *c, ଅଚଳ ଅକ୍ଷର *title)
{
	ଅକ୍ଷର *new_title = strdup(title);
	ଯଦି (new_title == ଖାଲି)
		ଫେରନ୍ତୁ false;

	ଯଦି (c->title != ଖାଲି)
		ମୁକ୍ତ(c->title);

	c->title = new_title;

	ଫେରନ୍ତୁ true;
}


/* exported interface documented in content/content.h */
ଅଚଳ ଅକ୍ଷର *content_get_title(hlcache_handle *h)
{
	ଫେରନ୍ତୁ content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ଅଚଳ ଅକ୍ଷର *content__get_title(ସଂରଚନା content *c)
{
	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ c->title != ଖାଲି ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_status content_get_status(hlcache_handle *h)
{
	ଫେରନ୍ତୁ content__get_status(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_status content__get_status(ସଂରଚନା content *c)
{
	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ CONTENT_STATUS_ERROR;

	ଫେରନ୍ତୁ c->status;
}


/* exported interface documented in content/content.h */
ଅଚଳ ଅକ୍ଷର *content_get_status_message(hlcache_handle *h)
{
	ଫେରନ୍ତୁ content__get_status_message(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ଅଚଳ ଅକ୍ଷର *content__get_status_message(ସଂରଚନା content *c)
{
	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ c->status_message;
}


/* exported interface documented in content/content.h */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା content_get_width(hlcache_handle *h)
{
	ଫେରନ୍ତୁ content__get_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା content__get_width(ସଂରଚନା content *c)
{
	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ c->width;
}


/* exported interface documented in content/content.h */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା content_get_height(hlcache_handle *h)
{
	ଫେରନ୍ତୁ content__get_height(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା content__get_height(ସଂରଚନା content *c)
{
	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ c->height;
}


/* exported interface documented in content/content.h */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା content_get_available_width(hlcache_handle *h)
{
	ଫେରନ୍ତୁ content__get_available_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା content__get_available_width(ସଂରଚନା content *c)
{
	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ c->available_width;
}


/* exported interface documented in content/content.h */
ଅଚଳ uint8_t *content_get_source_data(hlcache_handle *h, size_t *size)
{
	ଫେରନ୍ତୁ content__get_source_data(hlcache_handle_get_content(h), size);
}


/* exported interface documented in content/content_protected.h */
ଅଚଳ uint8_t *content__get_source_data(ସଂରଚନା content *c, size_t *size)
{
	ପ୍ରତିଜ୍ଞା(size != ଖାଲି);

	/** \todo check if the content check should be an assert */
	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ llcache_handle_get_source_data(c->llcache, size);
}


/* exported interface documented in content/content.h */
ଶୂନ୍ୟ content_invalidate_reuse_data(hlcache_handle *h)
{
	content__invalidate_reuse_data(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ଶୂନ୍ୟ content__invalidate_reuse_data(ସଂରଚନା content *c)
{
	ଯଦି (c == ଖାଲି || c->llcache == ଖାଲି)
		ଫେରନ୍ତୁ;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_data(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	ଫେରନ୍ତୁ content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(ସଂରଚନା content *c)
{
	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ c->refresh;
}


/* exported interface documented in content/content.h */
ସଂରଚନା bitmap *content_get_bitmap(hlcache_handle *h)
{
	ଫେରନ୍ତୁ content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ସଂରଚନା bitmap *content__get_bitmap(ସଂରଚନା content *c)
{
	ସଂରଚନା bitmap *bitmap = ଖାଲି;

	ଯଦି ((c != ଖାଲି) &&
	    (c->handler != ଖାଲି) &&
	    (c->handler->type != ଖାଲି) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_internal != ଖାଲି) ) {
		bitmap = c->handler->get_internal(c, ଖାଲି);
	}

	ଫେରନ୍ତୁ bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	ଫେରନ୍ତୁ content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(ସଂରଚନା content *c)
{
	ଯଦି ((c != ଖାଲି) &&
	    (c->handler != ଖାଲି) &&
	    (c->handler->is_opaque != ଖାଲି)) {
		ଫେରନ୍ତୁ c->handler->is_opaque(c);
	}

	ଫେରନ୍ତୁ false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	ସଂରଚନା content *c = hlcache_handle_get_content(h);

	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ false;

	ଫେରନ୍ତୁ c->quirks;
}


/* exported interface documented in content/content.h */
ଅଚଳ ଅକ୍ଷର *
content_get_encoding(hlcache_handle *h, ଗଣନା content_encoding_type op)
{
	ଫେରନ୍ତୁ content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
ଅଚଳ ଅକ୍ଷର *
content__get_encoding(ସଂରଚନା content *c, ଗଣନା content_encoding_type op)
{
	ଅଚଳ ଅକ୍ଷର *encoding = ଖାଲି;

	ଯଦି ((c != ଖାଲି) &&
	    (c->handler != ଖାଲି) &&
	    (c->handler->get_encoding != ଖାଲି) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	ଫେରନ୍ତୁ encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	ଫେରନ୍ତୁ content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(ସଂରଚନା content *c)
{
	ଫେରନ୍ତୁ c->locked;
}


/* exported interface documented in content/content.h */
ଅଚଳ llcache_handle *content_get_llcache_handle(ସଂରଚନା content *c)
{
	ଯଦି (c == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ c->llcache;
}


/* exported interface documented in content/protected.h */
ସଂରଚନା content *content_clone(ସଂରଚନା content *c)
{
	ସଂରଚନା content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(ଅଚଳ ସଂରଚନା content *c, ସଂରଚନା content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	ଯଦି (error != NSERROR_OK) {
		ଫେରନ୍ତୁ error;
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

	ଯଦି (c->fallback_charset != ଖାଲି) {
		nc->fallback_charset = strdup(c->fallback_charset);
		ଯଦି (nc->fallback_charset == ଖାଲି) {
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	}

	ଯଦି (c->refresh != ଖାଲି) {
		nc->refresh = nsurl_ref(c->refresh);
		ଯଦି (nc->refresh == ଖାଲି) {
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_time = c->reformat_time;
	nc->size = c->size;

	ଯଦି (c->title != ଖାଲି) {
		nc->title = strdup(c->title);
		ଯଦି (nc->title == ଖାଲି) {
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	}

	nc->active = c->active;

	nc->user_list = calloc(1, ଆକାର(ସଂରଚନା content_user));
	ଯଦି (nc->user_list == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	memcpy(&(nc->status_message), &(c->status_message), 120);
	memcpy(&(nc->sub_status), &(c->sub_status), 80);

	nc->locked = c->locked;
	nc->total_size = c->total_size;
	nc->http_code = c->http_code;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(ସଂରଚନା content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	ଯଦି (c->handler->stop != ଖାଲି)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	ଫେରନ୍ତୁ llcache_handle_abort(c->llcache);
}
