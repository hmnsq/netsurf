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

#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <nsutils/time.h>

#അന്തര്ഭാവ "netsurf/inttypes.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "netsurf/browser_window.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "desktop/knockout.h"

#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/textsearch.h"
#അന്തര്ഭാവ "content/content_debug.h"
#അന്തര്ഭാവ "content/hlcache.h"
#അന്തര്ഭാവ "content/urldb.h"

#define URL_FMT_SPC "%.140s"

സ്ഥിരം അക്ഷരം * സ്ഥിരം content_status_name[] = {
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
സ്ഥിതം ശൂന്യം content_convert(ഘടന content *c)
{
	പ്രതിജ്ഞാ(c);
	പ്രതിജ്ഞാ(c->status == CONTENT_STATUS_LOADING ||
	       c->status == CONTENT_STATUS_ERROR);

	എങ്കിൽ (c->status != CONTENT_STATUS_LOADING)
		തിരികെ;

	എങ്കിൽ (c->locked == true)
		തിരികെ;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	എങ്കിൽ (c->handler->data_complete != ശൂന്യ) {
		c->locked = true;
		എങ്കിൽ (c->handler->data_complete(c) == false) {
			content_set_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} അല്ലെങ്കിൽ {
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
സ്ഥിതം nserror
content_llcache_callback(llcache_handle *llcache,
			 സ്ഥിരം llcache_event *event, ശൂന്യം *pw)
{
	ഘടന content *c = pw;
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;
	nserror error = NSERROR_OK;

	തിരഞ്ഞെടുപ്പ് (event->type) {
	അവസ്ഥ LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		നിർത്തുക;
	അവസ്ഥ LLCACHE_EVENT_HAD_HEADERS:
		/* Will never happen: handled in hlcache */
		നിർത്തുക;
	അവസ്ഥ LLCACHE_EVENT_HAD_DATA:
		എങ്കിൽ (c->handler->process_data != ശൂന്യ) {
			എങ്കിൽ (c->handler->process_data(c,
						     (സ്ഥിരം അക്ഷരം *) event->data.data.buf,
						     event->data.data.len) == false) {
				llcache_handle_abort(c->llcache);
				c->status = CONTENT_STATUS_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		നിർത്തുക;
	അവസ്ഥ LLCACHE_EVENT_DONE:
		{
			size_t source_size;

			(ശൂന്യം) llcache_handle_get_source_data(llcache, &source_size);

			content_set_status(c, messages_get("Processing"));
			സന്ദേശം_data.explicit_status_text = ശൂന്യ;
			content_broadcast(c, CONTENT_സന്ദേശം_STATUS, &സന്ദേശം_data);

			content_convert(c);
		}
		നിർത്തുക;
	അവസ്ഥ LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->status = CONTENT_STATUS_ERROR;
		സന്ദേശം_data.errordata.errorcode = event->data.error.code;
		സന്ദേശം_data.errordata.errormsg = event->data.error.സന്ദേശം;
		content_broadcast(c, CONTENT_സന്ദേശം_ERROR, &സന്ദേശം_data);
		നിർത്തുക;
	അവസ്ഥ LLCACHE_EVENT_PROGRESS:
		content_set_status(c, event->data.progress_സന്ദേശം);
		സന്ദേശം_data.explicit_status_text = ശൂന്യ;
		content_broadcast(c, CONTENT_സന്ദേശം_STATUS, &സന്ദേശം_data);
		നിർത്തുക;
	അവസ്ഥ LLCACHE_EVENT_REDIRECT:
		സന്ദേശം_data.redirect.from = event->data.redirect.from;
		സന്ദേശം_data.redirect.to = event->data.redirect.to;
		content_broadcast(c, CONTENT_സന്ദേശം_REDIRECT, &സന്ദേശം_data);
		നിർത്തുക;
	}

	തിരികെ error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
സ്ഥിതം ശൂന്യം content_update_status(ഘടന content *c)
{
	എങ്കിൽ (c->status == CONTENT_STATUS_LOADING ||
	    c->status == CONTENT_STATUS_READY) {
		/* Not done yet */
		snprintf(c->status_message, വലുപ്പം (c->status_message),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_status[0] != '\0' ? ", " : " ",
			 c->sub_status);
	} അല്ലെങ്കിൽ {
		snprintf(c->status_message, വലുപ്പം (c->status_message),
			 "%s (%.1fs)", messages_get("Done"),
			 (ദശാംശം) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(ഘടന content *c,
	      സ്ഥിരം content_handler *handler,
	      lwc_string *imime_type,
	      സ്ഥിരം ഘടന http_parameter *params,
	      llcache_handle *llcache,
	      സ്ഥിരം അക്ഷരം *fallback_charset,
	      bool quirks)
{
	ഘടന content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = calloc(1, വലുപ്പം(ഘടന content_user));
	എങ്കിൽ (user_sentinel == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	എങ്കിൽ (fallback_charset != ശൂന്യ) {
		c->fallback_charset = strdup(fallback_charset);
		എങ്കിൽ (c->fallback_charset == ശൂന്യ) {
			സ്വതന്ത്രം(user_sentinel);
			തിരികെ NSERROR_NOMEM;
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
	c->title = ശൂന്യ;
	c->active = 0;
	user_sentinel->callback = ശൂന്യ;
	user_sentinel->pw = ശൂന്യ;
	user_sentinel->next = ശൂന്യ;
	c->user_list = user_sentinel;
	c->sub_status[0] = 0;
	c->locked = false;
	c->total_size = 0;
	c->http_code = 0;

	c->textsearch.string = ശൂന്യ;
	c->textsearch.context = ശൂന്യ;

	content_set_status(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	എങ്കിൽ (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		തിരികെ error;
	}

	തിരികെ NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	ഘടന content *c = hlcache_handle_get_content(h);

	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ false;

	തിരികെ (c->handler->reformat != ശൂന്യ);
}


/* exported interface documented in content/protected.h */
ശൂന്യം content_set_status(ഘടന content *c, സ്ഥിരം അക്ഷരം *status_message)
{
	size_t len = strlen(status_message);

	എങ്കിൽ (len >= വലുപ്പം(c->sub_status)) {
		len = വലുപ്പം(c->sub_status) - 1;
	}
	memcpy(c->sub_status, status_message, len);
	c->sub_status[len] = '\0';

	content_update_status(c);
}


/* exported interface documented in content/protected.h */
ശൂന്യം content_set_ready(ഘടന content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	പ്രതിജ്ഞാ(c->locked);
	c->locked = false;

	c->status = CONTENT_STATUS_READY;
	content_update_status(c);
	content_broadcast(c, CONTENT_സന്ദേശം_READY, ശൂന്യ);
}


/* exported interface documented in content/protected.h */
ശൂന്യം content_set_done(ഘടന content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->status = CONTENT_STATUS_DONE;
	c->time = now_ms - c->time;
	content_update_status(c);
	content_broadcast(c, CONTENT_സന്ദേശം_DONE, ശൂന്യ);
}


/* exported interface documented in content/protected.h */
ശൂന്യം content_set_error(ഘടന content *c)
{
	c->locked = false;
	c->status = CONTENT_STATUS_ERROR;
}


/* exported interface documented in content/content.h */
ശൂന്യം content_reformat(hlcache_handle *h, bool background,
		      പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  width, height);
}


/* exported interface documented in content/protected.h */
ശൂന്യം
content__reformat(ഘടന content *c, bool background, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height)
{
	സംഘം content_സന്ദേശം_data data;
	പ്രതിജ്ഞാ(c != 0);
	പ്രതിജ്ഞാ(c->status == CONTENT_STATUS_READY ||
	       c->status == CONTENT_STATUS_DONE);
	പ്രതിജ്ഞാ(c->locked == false);

	c->available_width = width;
	c->available_height = height;
	എങ്കിൽ (c->handler->reformat != ശൂന്യ) {

		c->locked = true;
		c->handler->reformat(c, width, height);
		c->locked = false;

		data.background = background;
		content_broadcast(c, CONTENT_സന്ദേശം_REFORMAT, &data);
	}
}


/* exported interface documented in content/content.h */
ശൂന്യം content_destroy(ഘടന content *c)
{
	ഘടന content_rfc5988_link *link;

	പ്രതിജ്ഞാ(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	പ്രതിജ്ഞാ(c->locked == false);

	എങ്കിൽ (c->handler->destroy != ശൂന്യ)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = ശൂന്യ;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	ആകുമ്പോൾ (link != ശൂന്യ) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	എങ്കിൽ (c->user_list != ശൂന്യ) {
		സ്വതന്ത്രം(c->user_list);
	}

	/* free the title */
	എങ്കിൽ (c->title != ശൂന്യ) {
		സ്വതന്ത്രം(c->title);
	}

	/* free the fallback characterset */
	എങ്കിൽ (c->fallback_charset != ശൂന്യ) {
		സ്വതന്ത്രം(c->fallback_charset);
	}

	സ്വതന്ത്രം(c);
}


/* exported interface documented in content/content.h */
ശൂന്യം
content_mouse_track(hlcache_handle *h,
		    ഘടന browser_window *bw,
		    browser_mouse_state mouse,
		    പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	പ്രതിജ്ഞാ(c != ശൂന്യ);

	എങ്കിൽ (c->handler->mouse_track != ശൂന്യ) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} അല്ലെങ്കിൽ {
		സംഘം content_സന്ദേശം_data സന്ദേശം_data;
		സന്ദേശം_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(c, CONTENT_സന്ദേശം_POINTER, &സന്ദേശം_data);
	}


	തിരികെ;
}


/* exported interface documented in content/content.h */
ശൂന്യം
content_mouse_action(hlcache_handle *h,
		     ഘടന browser_window *bw,
		     browser_mouse_state mouse,
		     പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	പ്രതിജ്ഞാ(c != ശൂന്യ);

	എങ്കിൽ (c->handler->mouse_action != ശൂന്യ)
		c->handler->mouse_action(c, bw, mouse, x, y);

	തിരികെ;
}


/* exported interface documented in content/content.h */
bool content_keypress(ഘടന hlcache_handle *h, uint32_t താക്കോൽ)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	പ്രതിജ്ഞാ(c != ശൂന്യ);

	എങ്കിൽ (c->handler->keypress != ശൂന്യ)
		തിരികെ c->handler->keypress(c, താക്കോൽ);

	തിരികെ false;
}


/* exported interface documented in content/content.h */
ശൂന്യം content_request_redraw(ഘടന hlcache_handle *h,
			    പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, width, height);
}


/* exported interface, documented in content/protected.h */
ശൂന്യം content__request_redraw(ഘടന content *c,
			     പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height)
{
	സംഘം content_സന്ദേശം_data data;

	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ;

	data.redraw.x = x;
	data.redraw.y = y;
	data.redraw.width = width;
	data.redraw.height = height;

	content_broadcast(c, CONTENT_സന്ദേശം_REDRAW, &data);
}


/* exported interface, documented in content/content.h */
bool content_exec(ഘടന hlcache_handle *h, സ്ഥിരം അക്ഷരം *src, size_t srclen)
{
	ഘടന content *c = hlcache_handle_get_content(h);

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	എങ്കിൽ (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		തിരികെ false;
	}

	എങ്കിൽ (c->handler->exec == ശൂന്യ) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		തിരികെ false;
	}

	തിരികെ c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(ഘടന hlcache_handle *h)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	ഘടന nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool match;

	/* Is this an internal scheme? If so, we trust here and stop */
	എങ്കിൽ ((lwc_string_isequal(scheme, corestring_lwc_about,
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
		തിരികെ false;
	}

	/* Okay, not internal, am *I* secure? */
	എങ്കിൽ ((lwc_string_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok)
	    && (match == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		തിരികെ true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	എങ്കിൽ (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		തിരികെ true;
	}

	/* Otherwise try and chain through the handler */
	എങ്കിൽ (c != ശൂന്യ && c->handler->saw_insecure_objects != ശൂന്യ) {
		തിരികെ c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	തിരികെ false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       ഘടന content_redraw_data *data,
	       സ്ഥിരം ഘടന rect *clip,
	       സ്ഥിരം ഘടന redraw_context *ctx)
{
	ഘടന content *c = hlcache_handle_get_content(h);

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	എങ്കിൽ (c->locked) {
		/* not safe to attempt redraw */
		തിരികെ true;
	}

	/* ensure we have a redrawable content */
	എങ്കിൽ (c->handler->redraw == ശൂന്യ) {
		തിരികെ true;
	}

	തിരികെ c->handler->redraw(c, data, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(ഘടന hlcache_handle *h,
		      പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height,
		      സ്ഥിരം ഘടന redraw_context *ctx)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	ഘടന redraw_context new_ctx = *ctx;
	ഘടന rect clip;
	ഘടന content_redraw_data data;
	bool plot_ok = true;

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	/* ensure it is safe to attempt redraw */
	എങ്കിൽ (c->locked) {
		തിരികെ true;
	}

	/* ensure we have a redrawable content */
	എങ്കിൽ (c->handler->redraw == ശൂന്യ) {
		തിരികെ true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, width, height, ctx);

	എങ്കിൽ (ctx->plot->option_knockout) {
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
	എങ്കിൽ (c->width) {
		data.scale = (ദശാംശം)width / (ദശാംശം)c->width;
	} അല്ലെങ്കിൽ {
		data.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &data, &clip, &new_ctx);

	എങ്കിൽ (ctx->plot->option_knockout) {
		knockout_plot_end(ctx);
	}

	തിരികെ plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_add_user(ഘടന content *c,
		 ശൂന്യം (*callback)(
				  ഘടന content *c,
				  content_സന്ദേശം സന്ദേശം,
				  സ്ഥിരം സംഘം content_സന്ദേശം_data *data,
				  ശൂന്യം *pw),
		 ശൂന്യം *pw)
{
	ഘടന content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = malloc(വലുപ്പം(ഘടന content_user));
	എങ്കിൽ (!user)
		തിരികെ false;
	user->callback = callback;
	user->pw = pw;
	user->next = c->user_list->next;
	c->user_list->next = user;

	എങ്കിൽ (c->handler->add_user != ശൂന്യ)
		c->handler->add_user(c);

	തിരികെ true;
}


/* exported interface documented in content/content.h */
ശൂന്യം
content_remove_user(ഘടന content *c,
		    ശൂന്യം (*callback)(
				     ഘടന content *c,
				     content_സന്ദേശം സന്ദേശം,
				     സ്ഥിരം സംഘം content_സന്ദേശം_data *data,
				     ശൂന്യം *pw),
		    ശൂന്യം *pw)
{
	ഘടന content_user *user, *next;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	വേണ്ടി (user = c->user_list; user->next != 0 &&
		     !(user->next->callback == callback &&
		       user->next->pw == pw); user = user->next)
		;
	എങ്കിൽ (user->next == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		പ്രതിജ്ഞാ(0);
		തിരികെ;
	}

	എങ്കിൽ (c->handler->remove_user != ശൂന്യ)
		c->handler->remove_user(c);

	next = user->next;
	user->next = next->next;
	സ്വതന്ത്രം(next);
}


/* exported interface documented in content/content.h */
uint32_t content_എണ്ണം_users(ഘടന content *c)
{
	ഘടന content_user *user;
	uint32_t counter = 0;

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	വേണ്ടി (user = c->user_list; user != ശൂന്യ; user = user->next)
		counter += 1;

	പ്രതിജ്ഞാ(counter > 0);

	തിരികെ counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(ഘടന content *c, bool quirks)
{
	എങ്കിൽ (c->handler->matches_quirks == ശൂന്യ)
		തിരികെ true;

	തിരികെ c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(ഘടന content *c)
{
	തിരികെ c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
ശൂന്യം content_broadcast(ഘടന content *c, content_സന്ദേശം സന്ദേശം,
		       സ്ഥിരം സംഘം content_സന്ദേശം_data *data)
{
	ഘടന content_user *user, *next;
	പ്രതിജ്ഞാ(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, സന്ദേശം);
	വേണ്ടി (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		എങ്കിൽ (user->callback != 0)
			user->callback(c, സന്ദേശം, data, user->pw);
	}
}


/* exported interface documented in content_protected.h */
ശൂന്യം
content_broadcast_error(ഘടന content *c, nserror errorcode, സ്ഥിരം അക്ഷരം *സന്ദേശം)
{
	ഘടന content_user *user, *next;
	സംഘം content_സന്ദേശം_data data;

	പ്രതിജ്ഞാ(c);

	data.errordata.errorcode = errorcode;
	data.errordata.errormsg = സന്ദേശം;

	വേണ്ടി (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		എങ്കിൽ (user->callback != 0) {
			user->callback(c, CONTENT_സന്ദേശം_ERROR,
				       &data, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     ഘടന browser_window *bw,
	     ഘടന content *page,
	     ഘടന object_params *params)
{
	ഘടന content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	പ്രതിജ്ഞാ(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	എങ്കിൽ (c->handler->open != ശൂന്യ) {
		res = c->handler->open(c, bw, page, params);
	} അല്ലെങ്കിൽ {
		res = NSERROR_OK;
	}
	തിരികെ res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	ഘടന content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	എങ്കിൽ (c == ശൂന്യ) {
		തിരികെ NSERROR_BAD_PARAMETER;
	}

	എങ്കിൽ ((c->status != CONTENT_STATUS_READY) &&
	    (c->status != CONTENT_STATUS_DONE)) {
		/* status is not read or done so nothing to do */
		തിരികെ NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	എങ്കിൽ (c->textsearch.context != ശൂന്യ) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ശൂന്യ;
	}

	എങ്കിൽ (c->handler->close != ശൂന്യ) {
		res = c->handler->close(c);
	} അല്ലെങ്കിൽ {
		res = NSERROR_OK;
	}
	തിരികെ res;
}


/* exported interface, documented in content/content.h */
ശൂന്യം content_clear_selection(hlcache_handle *h)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	പ്രതിജ്ഞാ(c != 0);

	എങ്കിൽ (c->handler->get_selection != ശൂന്യ)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
അക്ഷരം * content_get_selection(hlcache_handle *h)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	പ്രതിജ്ഞാ(c != 0);

	എങ്കിൽ (c->handler->get_selection != ശൂന്യ)
		തിരികെ c->handler->get_selection(c);
	അല്ലെങ്കിൽ
		തിരികെ ശൂന്യ;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(ഘടന hlcache_handle *h,
			       പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
			       ഘടന browser_window_features *data)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	പ്രതിജ്ഞാ(c != 0);

	എങ്കിൽ (c->handler->get_contextual_content != ശൂന്യ) {
		തിരികെ c->handler->get_contextual_content(c, x, y, data);
	}

	data->object = h;
	തിരികെ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_point(ഘടന hlcache_handle *h,
			പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
			പൂർണ്ണസംഖ്യ scrx, പൂർണ്ണസംഖ്യ scry)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	പ്രതിജ്ഞാ(c != 0);

	എങ്കിൽ (c->handler->scroll_at_point != ശൂന്യ)
		തിരികെ c->handler->scroll_at_point(c, x, y, scrx, scry);

	തിരികെ false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_point(ഘടന hlcache_handle *h,
			   പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
			   അക്ഷരം *file)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	പ്രതിജ്ഞാ(c != 0);

	എങ്കിൽ (c->handler->drop_file_at_point != ശൂന്യ)
		തിരികെ c->handler->drop_file_at_point(c, x, y, file);

	തിരികെ false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(ഘടന hlcache_handle *h, FILE *f, എണ്ണൽ content_debug op)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	പ്രതിജ്ഞാ(c != 0);

	എങ്കിൽ (c->handler->debug_dump == ശൂന്യ) {
		തിരികെ NSERROR_NOT_IMPLEMENTED;
	}

	തിരികെ c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(ഘടന hlcache_handle *h, എണ്ണൽ content_debug op)
{
	ഘടന content *c = hlcache_handle_get_content(h);

	എങ്കിൽ (c == ശൂന്യ) {
		തിരികെ NSERROR_BAD_PARAMETER;
	}

	എങ്കിൽ (c->handler->debug == ശൂന്യ) {
		തിരികെ NSERROR_NOT_IMPLEMENTED;
	}

	തിരികെ c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
ഘടന content_rfc5988_link *
content_find_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	ഘടന content *c = hlcache_handle_get_content(h);
	ഘടന content_rfc5988_link *link = c->links;
	bool rel_match = false;

	ആകുമ്പോൾ (link != ശൂന്യ) {
		എങ്കിൽ (lwc_string_caseless_isequal(link->rel, rel,
						&rel_match) == lwc_error_ok && rel_match) {
			നിർത്തുക;
		}
		link = link->next;
	}
	തിരികെ link;
}


/* exported interface documented in content/protected.h */
ഘടന content_rfc5988_link *
content__free_rfc5988_link(ഘടന content_rfc5988_link *link)
{
	ഘടന content_rfc5988_link *next;

	next = link->next;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	സ്വതന്ത്രം(link);

	തിരികെ next;
}


/* exported interface documented in content/protected.h */
bool
content__add_rfc5988_link(ഘടന content *c,
			  സ്ഥിരം ഘടന content_rfc5988_link *link)
{
	ഘടന content_rfc5988_link *newlink;
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;

	/* a link relation must be present for it to be a link */
	എങ്കിൽ (link->rel == ശൂന്യ) {
		തിരികെ false;
	}

	/* a link href must be present for it to be a link */
	എങ്കിൽ (link->href == ശൂന്യ) {
		തിരികെ false;
	}

	newlink = calloc(1, വലുപ്പം(ഘടന content_rfc5988_link));
	എങ്കിൽ (newlink == ശൂന്യ) {
		തിരികെ false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	എങ്കിൽ (link->hreflang != ശൂന്യ) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	എങ്കിൽ (link->type != ശൂന്യ) {
		newlink->type = lwc_string_ref(link->type);
	}
	എങ്കിൽ (link->media != ശൂന്യ) {
		newlink->media = lwc_string_ref(link->media);
	}
	എങ്കിൽ (link->sizes != ശൂന്യ) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->next = c->links;
	c->links = newlink;

	/* broadcast the data */
	സന്ദേശം_data.rfc5988_link = newlink;
	content_broadcast(c, CONTENT_സന്ദേശം_LINK, &സന്ദേശം_data);

	തിരികെ true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(ഘടന content *c)
{
	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ ശൂന്യ;

	തിരികെ llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	ഘടന content *c = hlcache_handle_get_content(h);

	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ CONTENT_NONE;

	തിരികെ c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	തിരികെ content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(ഘടന content *c)
{
	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ ശൂന്യ;

	തിരികെ lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content__set_title(ഘടന content *c, സ്ഥിരം അക്ഷരം *title)
{
	അക്ഷരം *new_title = strdup(title);
	എങ്കിൽ (new_title == ശൂന്യ)
		തിരികെ false;

	എങ്കിൽ (c->title != ശൂന്യ)
		സ്വതന്ത്രം(c->title);

	c->title = new_title;

	തിരികെ true;
}


/* exported interface documented in content/content.h */
സ്ഥിരം അക്ഷരം *content_get_title(hlcache_handle *h)
{
	തിരികെ content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
സ്ഥിരം അക്ഷരം *content__get_title(ഘടന content *c)
{
	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ ശൂന്യ;

	തിരികെ c->title != ശൂന്യ ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_status content_get_status(hlcache_handle *h)
{
	തിരികെ content__get_status(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_status content__get_status(ഘടന content *c)
{
	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ CONTENT_STATUS_ERROR;

	തിരികെ c->status;
}


/* exported interface documented in content/content.h */
സ്ഥിരം അക്ഷരം *content_get_status_message(hlcache_handle *h)
{
	തിരികെ content__get_status_message(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
സ്ഥിരം അക്ഷരം *content__get_status_message(ഘടന content *c)
{
	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ ശൂന്യ;

	തിരികെ c->status_message;
}


/* exported interface documented in content/content.h */
പൂർണ്ണസംഖ്യ content_get_width(hlcache_handle *h)
{
	തിരികെ content__get_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
പൂർണ്ണസംഖ്യ content__get_width(ഘടന content *c)
{
	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ 0;

	തിരികെ c->width;
}


/* exported interface documented in content/content.h */
പൂർണ്ണസംഖ്യ content_get_height(hlcache_handle *h)
{
	തിരികെ content__get_height(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
പൂർണ്ണസംഖ്യ content__get_height(ഘടന content *c)
{
	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ 0;

	തിരികെ c->height;
}


/* exported interface documented in content/content.h */
പൂർണ്ണസംഖ്യ content_get_available_width(hlcache_handle *h)
{
	തിരികെ content__get_available_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
പൂർണ്ണസംഖ്യ content__get_available_width(ഘടന content *c)
{
	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ 0;

	തിരികെ c->available_width;
}


/* exported interface documented in content/content.h */
സ്ഥിരം uint8_t *content_get_source_data(hlcache_handle *h, size_t *size)
{
	തിരികെ content__get_source_data(hlcache_handle_get_content(h), size);
}


/* exported interface documented in content/content_protected.h */
സ്ഥിരം uint8_t *content__get_source_data(ഘടന content *c, size_t *size)
{
	പ്രതിജ്ഞാ(size != ശൂന്യ);

	/** \todo check if the content check should be an assert */
	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ ശൂന്യ;

	തിരികെ llcache_handle_get_source_data(c->llcache, size);
}


/* exported interface documented in content/content.h */
ശൂന്യം content_invalidate_reuse_data(hlcache_handle *h)
{
	content__invalidate_reuse_data(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ശൂന്യം content__invalidate_reuse_data(ഘടന content *c)
{
	എങ്കിൽ (c == ശൂന്യ || c->llcache == ശൂന്യ)
		തിരികെ;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_data(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	തിരികെ content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(ഘടന content *c)
{
	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ ശൂന്യ;

	തിരികെ c->refresh;
}


/* exported interface documented in content/content.h */
ഘടന bitmap *content_get_bitmap(hlcache_handle *h)
{
	തിരികെ content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ഘടന bitmap *content__get_bitmap(ഘടന content *c)
{
	ഘടന bitmap *bitmap = ശൂന്യ;

	എങ്കിൽ ((c != ശൂന്യ) &&
	    (c->handler != ശൂന്യ) &&
	    (c->handler->type != ശൂന്യ) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_internal != ശൂന്യ) ) {
		bitmap = c->handler->get_internal(c, ശൂന്യ);
	}

	തിരികെ bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	തിരികെ content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(ഘടന content *c)
{
	എങ്കിൽ ((c != ശൂന്യ) &&
	    (c->handler != ശൂന്യ) &&
	    (c->handler->is_opaque != ശൂന്യ)) {
		തിരികെ c->handler->is_opaque(c);
	}

	തിരികെ false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	ഘടന content *c = hlcache_handle_get_content(h);

	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ false;

	തിരികെ c->quirks;
}


/* exported interface documented in content/content.h */
സ്ഥിരം അക്ഷരം *
content_get_encoding(hlcache_handle *h, എണ്ണൽ content_encoding_type op)
{
	തിരികെ content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
സ്ഥിരം അക്ഷരം *
content__get_encoding(ഘടന content *c, എണ്ണൽ content_encoding_type op)
{
	സ്ഥിരം അക്ഷരം *encoding = ശൂന്യ;

	എങ്കിൽ ((c != ശൂന്യ) &&
	    (c->handler != ശൂന്യ) &&
	    (c->handler->get_encoding != ശൂന്യ) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	തിരികെ encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	തിരികെ content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(ഘടന content *c)
{
	തിരികെ c->locked;
}


/* exported interface documented in content/content.h */
സ്ഥിരം llcache_handle *content_get_llcache_handle(ഘടന content *c)
{
	എങ്കിൽ (c == ശൂന്യ)
		തിരികെ ശൂന്യ;

	തിരികെ c->llcache;
}


/* exported interface documented in content/protected.h */
ഘടന content *content_clone(ഘടന content *c)
{
	ഘടന content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ ശൂന്യ;

	തിരികെ nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(സ്ഥിരം ഘടന content *c, ഘടന content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	എങ്കിൽ (error != NSERROR_OK) {
		തിരികെ error;
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

	എങ്കിൽ (c->fallback_charset != ശൂന്യ) {
		nc->fallback_charset = strdup(c->fallback_charset);
		എങ്കിൽ (nc->fallback_charset == ശൂന്യ) {
			തിരികെ NSERROR_NOMEM;
		}
	}

	എങ്കിൽ (c->refresh != ശൂന്യ) {
		nc->refresh = nsurl_ref(c->refresh);
		എങ്കിൽ (nc->refresh == ശൂന്യ) {
			തിരികെ NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_time = c->reformat_time;
	nc->size = c->size;

	എങ്കിൽ (c->title != ശൂന്യ) {
		nc->title = strdup(c->title);
		എങ്കിൽ (nc->title == ശൂന്യ) {
			തിരികെ NSERROR_NOMEM;
		}
	}

	nc->active = c->active;

	nc->user_list = calloc(1, വലുപ്പം(ഘടന content_user));
	എങ്കിൽ (nc->user_list == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	memcpy(&(nc->status_message), &(c->status_message), 120);
	memcpy(&(nc->sub_status), &(c->sub_status), 80);

	nc->locked = c->locked;
	nc->total_size = c->total_size;
	nc->http_code = c->http_code;

	തിരികെ NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(ഘടന content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	എങ്കിൽ (c->handler->stop != ശൂന്യ)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	തിരികെ llcache_handle_abort(c->llcache);
}
