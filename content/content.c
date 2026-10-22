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

#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <nsutils/time.h>

#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "netsurf/browser_window.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "desktop/knockout.h"

#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/textsearch.h"
#अन्तर्भाव "content/content_debug.h"
#अन्तर्भाव "content/hlcache.h"
#अन्तर्भाव "content/urldb.h"

#define URL_FMT_SPC "%.140s"

ध्रुव वर्ण * ध्रुव content_status_name[] = {
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
स्थिर शून्य content_convert(संरचना content *c)
{
	प्रतिज्ञा(c);
	प्रतिज्ञा(c->status == CONTENT_STATUS_LOADING ||
	       c->status == CONTENT_STATUS_ERROR);

	यदि (c->status != CONTENT_STATUS_LOADING)
		प्रत्यागम;

	यदि (c->locked == true)
		प्रत्यागम;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	यदि (c->handler->data_complete != रिक्त) {
		c->locked = true;
		यदि (c->handler->data_complete(c) == false) {
			content_set_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} अन्यथा {
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
स्थिर nserror
content_llcache_callback(llcache_handle *llcache,
			 ध्रुव llcache_event *event, शून्य *pw)
{
	संरचना content *c = pw;
	सङ्घ content_संदेश_data संदेश_data;
	nserror error = NSERROR_OK;

	विकल्प (event->type) {
	पक्ष LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		भङ्ग;
	पक्ष LLCACHE_EVENT_HAD_HEADERS:
		/* Will never happen: handled in hlcache */
		भङ्ग;
	पक्ष LLCACHE_EVENT_HAD_DATA:
		यदि (c->handler->process_data != रिक्त) {
			यदि (c->handler->process_data(c,
						     (ध्रुव वर्ण *) event->data.data.buf,
						     event->data.data.len) == false) {
				llcache_handle_abort(c->llcache);
				c->status = CONTENT_STATUS_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		भङ्ग;
	पक्ष LLCACHE_EVENT_DONE:
		{
			माप_प्रकार source_size;

			(शून्य) llcache_handle_get_source_data(llcache, &source_size);

			content_set_status(c, messages_get("Processing"));
			संदेश_data.explicit_status_text = रिक्त;
			content_broadcast(c, CONTENT_संदेश_STATUS, &संदेश_data);

			content_convert(c);
		}
		भङ्ग;
	पक्ष LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->status = CONTENT_STATUS_ERROR;
		संदेश_data.errordata.errorcode = event->data.error.code;
		संदेश_data.errordata.errormsg = event->data.error.संदेश;
		content_broadcast(c, CONTENT_संदेश_ERROR, &संदेश_data);
		भङ्ग;
	पक्ष LLCACHE_EVENT_PROGRESS:
		content_set_status(c, event->data.progress_संदेश);
		संदेश_data.explicit_status_text = रिक्त;
		content_broadcast(c, CONTENT_संदेश_STATUS, &संदेश_data);
		भङ्ग;
	पक्ष LLCACHE_EVENT_REDIRECT:
		संदेश_data.redirect.from = event->data.redirect.from;
		संदेश_data.redirect.to = event->data.redirect.to;
		content_broadcast(c, CONTENT_संदेश_REDIRECT, &संदेश_data);
		भङ्ग;
	}

	प्रत्यागम error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
स्थिर शून्य content_update_status(संरचना content *c)
{
	यदि (c->status == CONTENT_STATUS_LOADING ||
	    c->status == CONTENT_STATUS_READY) {
		/* Not done yet */
		snprintf(c->status_message, परिमाण (c->status_message),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_status[0] != '\0' ? ", " : " ",
			 c->sub_status);
	} अन्यथा {
		snprintf(c->status_message, परिमाण (c->status_message),
			 "%s (%.1fs)", messages_get("Done"),
			 (भिन्न) c->समय / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(संरचना content *c,
	      ध्रुव content_handler *handler,
	      lwc_string *imime_type,
	      ध्रुव संरचना http_parameter *params,
	      llcache_handle *llcache,
	      ध्रुव वर्ण *fallback_charset,
	      bool quirks)
{
	संरचना content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = सुस्मृति(1, परिमाण(संरचना content_user));
	यदि (user_sentinel == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	यदि (fallback_charset != रिक्त) {
		c->fallback_charset = strdup(fallback_charset);
		यदि (c->fallback_charset == रिक्त) {
			मुक्त(user_sentinel);
			प्रत्यागम NSERROR_NOMEM;
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
	nsu_getmonotonic_ms(&c->समय);
	c->size = 0;
	c->title = रिक्त;
	c->active = 0;
	user_sentinel->callback = रिक्त;
	user_sentinel->pw = रिक्त;
	user_sentinel->next = रिक्त;
	c->user_list = user_sentinel;
	c->sub_status[0] = 0;
	c->locked = false;
	c->total_size = 0;
	c->http_code = 0;

	c->textsearch.string = रिक्त;
	c->textsearch.context = रिक्त;

	content_set_status(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	यदि (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		प्रत्यागम error;
	}

	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	संरचना content *c = hlcache_handle_get_content(h);

	यदि (c == रिक्त)
		प्रत्यागम false;

	प्रत्यागम (c->handler->reformat != रिक्त);
}


/* exported interface documented in content/protected.h */
शून्य content_set_status(संरचना content *c, ध्रुव वर्ण *status_message)
{
	माप_प्रकार len = म_माप(status_message);

	यदि (len >= परिमाण(c->sub_status)) {
		len = परिमाण(c->sub_status) - 1;
	}
	स_नकल(c->sub_status, status_message, len);
	c->sub_status[len] = '\0';

	content_update_status(c);
}


/* exported interface documented in content/protected.h */
शून्य content_set_ready(संरचना content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	प्रतिज्ञा(c->locked);
	c->locked = false;

	c->status = CONTENT_STATUS_READY;
	content_update_status(c);
	content_broadcast(c, CONTENT_संदेश_READY, रिक्त);
}


/* exported interface documented in content/protected.h */
शून्य content_set_done(संरचना content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->status = CONTENT_STATUS_DONE;
	c->समय = now_ms - c->समय;
	content_update_status(c);
	content_broadcast(c, CONTENT_संदेश_DONE, रिक्त);
}


/* exported interface documented in content/protected.h */
शून्य content_set_error(संरचना content *c)
{
	c->locked = false;
	c->status = CONTENT_STATUS_ERROR;
}


/* exported interface documented in content/content.h */
शून्य content_reformat(hlcache_handle *h, bool background,
		      पूर्णाङ्क width, पूर्णाङ्क height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  width, height);
}


/* exported interface documented in content/protected.h */
शून्य
content__reformat(संरचना content *c, bool background, पूर्णाङ्क width, पूर्णाङ्क height)
{
	सङ्घ content_संदेश_data data;
	प्रतिज्ञा(c != 0);
	प्रतिज्ञा(c->status == CONTENT_STATUS_READY ||
	       c->status == CONTENT_STATUS_DONE);
	प्रतिज्ञा(c->locked == false);

	c->available_width = width;
	c->available_height = height;
	यदि (c->handler->reformat != रिक्त) {

		c->locked = true;
		c->handler->reformat(c, width, height);
		c->locked = false;

		data.background = background;
		content_broadcast(c, CONTENT_संदेश_REFORMAT, &data);
	}
}


/* exported interface documented in content/content.h */
शून्य content_destroy(संरचना content *c)
{
	संरचना content_rfc5988_link *link;

	प्रतिज्ञा(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	प्रतिज्ञा(c->locked == false);

	यदि (c->handler->destroy != रिक्त)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = रिक्त;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	यावत् (link != रिक्त) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	यदि (c->user_list != रिक्त) {
		मुक्त(c->user_list);
	}

	/* free the title */
	यदि (c->title != रिक्त) {
		मुक्त(c->title);
	}

	/* free the fallback characterset */
	यदि (c->fallback_charset != रिक्त) {
		मुक्त(c->fallback_charset);
	}

	मुक्त(c);
}


/* exported interface documented in content/content.h */
शून्य
content_mouse_track(hlcache_handle *h,
		    संरचना browser_window *bw,
		    browser_mouse_state mouse,
		    पूर्णाङ्क x, पूर्णाङ्क y)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != रिक्त);

	यदि (c->handler->mouse_track != रिक्त) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} अन्यथा {
		सङ्घ content_संदेश_data संदेश_data;
		संदेश_data.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(c, CONTENT_संदेश_POINTER, &संदेश_data);
	}


	प्रत्यागम;
}


/* exported interface documented in content/content.h */
शून्य
content_mouse_action(hlcache_handle *h,
		     संरचना browser_window *bw,
		     browser_mouse_state mouse,
		     पूर्णाङ्क x, पूर्णाङ्क y)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != रिक्त);

	यदि (c->handler->mouse_action != रिक्त)
		c->handler->mouse_action(c, bw, mouse, x, y);

	प्रत्यागम;
}


/* exported interface documented in content/content.h */
bool content_keypress(संरचना hlcache_handle *h, uint32_t कुंजी)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != रिक्त);

	यदि (c->handler->keypress != रिक्त)
		प्रत्यागम c->handler->keypress(c, कुंजी);

	प्रत्यागम false;
}


/* exported interface documented in content/content.h */
शून्य content_request_redraw(संरचना hlcache_handle *h,
			    पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क width, पूर्णाङ्क height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, width, height);
}


/* exported interface, documented in content/protected.h */
शून्य content__request_redraw(संरचना content *c,
			     पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क width, पूर्णाङ्क height)
{
	सङ्घ content_संदेश_data data;

	यदि (c == रिक्त)
		प्रत्यागम;

	data.redraw.x = x;
	data.redraw.y = y;
	data.redraw.width = width;
	data.redraw.height = height;

	content_broadcast(c, CONTENT_संदेश_REDRAW, &data);
}


/* exported interface, documented in content/content.h */
bool content_exec(संरचना hlcache_handle *h, ध्रुव वर्ण *src, माप_प्रकार srclen)
{
	संरचना content *c = hlcache_handle_get_content(h);

	प्रतिज्ञा(c != रिक्त);

	यदि (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		प्रत्यागम false;
	}

	यदि (c->handler->exec == रिक्त) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		प्रत्यागम false;
	}

	प्रत्यागम c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(संरचना hlcache_handle *h)
{
	संरचना content *c = hlcache_handle_get_content(h);
	संरचना nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool match;

	/* Is this an internal scheme? If so, we trust here and stop */
	यदि ((lwc_string_isequal(scheme, corestring_lwc_about,
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
		प्रत्यागम false;
	}

	/* Okay, not internal, am *I* secure? */
	यदि ((lwc_string_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok)
	    && (match == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		प्रत्यागम true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	यदि (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		प्रत्यागम true;
	}

	/* Otherwise try and chain through the handler */
	यदि (c != रिक्त && c->handler->saw_insecure_objects != रिक्त) {
		प्रत्यागम c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	प्रत्यागम false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       संरचना content_redraw_data *data,
	       ध्रुव संरचना rect *clip,
	       ध्रुव संरचना redraw_context *ctx)
{
	संरचना content *c = hlcache_handle_get_content(h);

	प्रतिज्ञा(c != रिक्त);

	यदि (c->locked) {
		/* not safe to attempt redraw */
		प्रत्यागम true;
	}

	/* ensure we have a redrawable content */
	यदि (c->handler->redraw == रिक्त) {
		प्रत्यागम true;
	}

	प्रत्यागम c->handler->redraw(c, data, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(संरचना hlcache_handle *h,
		      पूर्णाङ्क width, पूर्णाङ्क height,
		      ध्रुव संरचना redraw_context *ctx)
{
	संरचना content *c = hlcache_handle_get_content(h);
	संरचना redraw_context new_ctx = *ctx;
	संरचना rect clip;
	संरचना content_redraw_data data;
	bool plot_ok = true;

	प्रतिज्ञा(c != रिक्त);

	/* ensure it is safe to attempt redraw */
	यदि (c->locked) {
		प्रत्यागम true;
	}

	/* ensure we have a redrawable content */
	यदि (c->handler->redraw == रिक्त) {
		प्रत्यागम true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, width, height, ctx);

	यदि (ctx->plot->option_knockout) {
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
	यदि (c->width) {
		data.scale = (भिन्न)width / (भिन्न)c->width;
	} अन्यथा {
		data.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &data, &clip, &new_ctx);

	यदि (ctx->plot->option_knockout) {
		knockout_plot_end(ctx);
	}

	प्रत्यागम plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_add_user(संरचना content *c,
		 शून्य (*callback)(
				  संरचना content *c,
				  content_संदेश संदेश,
				  ध्रुव सङ्घ content_संदेश_data *data,
				  शून्य *pw),
		 शून्य *pw)
{
	संरचना content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = दो_स्मृति(परिमाण(संरचना content_user));
	यदि (!user)
		प्रत्यागम false;
	user->callback = callback;
	user->pw = pw;
	user->next = c->user_list->next;
	c->user_list->next = user;

	यदि (c->handler->add_user != रिक्त)
		c->handler->add_user(c);

	प्रत्यागम true;
}


/* exported interface documented in content/content.h */
शून्य
content_remove_user(संरचना content *c,
		    शून्य (*callback)(
				     संरचना content *c,
				     content_संदेश संदेश,
				     ध्रुव सङ्घ content_संदेश_data *data,
				     शून्य *pw),
		    शून्य *pw)
{
	संरचना content_user *user, *next;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	प्रति (user = c->user_list; user->next != 0 &&
		     !(user->next->callback == callback &&
		       user->next->pw == pw); user = user->next)
		;
	यदि (user->next == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		प्रतिज्ञा(0);
		प्रत्यागम;
	}

	यदि (c->handler->remove_user != रिक्त)
		c->handler->remove_user(c);

	next = user->next;
	user->next = next->next;
	मुक्त(next);
}


/* exported interface documented in content/content.h */
uint32_t content_गिनती_users(संरचना content *c)
{
	संरचना content_user *user;
	uint32_t counter = 0;

	प्रतिज्ञा(c != रिक्त);

	प्रति (user = c->user_list; user != रिक्त; user = user->next)
		counter += 1;

	प्रतिज्ञा(counter > 0);

	प्रत्यागम counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(संरचना content *c, bool quirks)
{
	यदि (c->handler->matches_quirks == रिक्त)
		प्रत्यागम true;

	प्रत्यागम c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(संरचना content *c)
{
	प्रत्यागम c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
शून्य content_broadcast(संरचना content *c, content_संदेश संदेश,
		       ध्रुव सङ्घ content_संदेश_data *data)
{
	संरचना content_user *user, *next;
	प्रतिज्ञा(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, संदेश);
	प्रति (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		यदि (user->callback != 0)
			user->callback(c, संदेश, data, user->pw);
	}
}


/* exported interface documented in content_protected.h */
शून्य
content_broadcast_error(संरचना content *c, nserror errorcode, ध्रुव वर्ण *संदेश)
{
	संरचना content_user *user, *next;
	सङ्घ content_संदेश_data data;

	प्रतिज्ञा(c);

	data.errordata.errorcode = errorcode;
	data.errordata.errormsg = संदेश;

	प्रति (user = c->user_list->next; user != 0; user = next) {
		next = user->next;  /* user may be destroyed during callback */
		यदि (user->callback != 0) {
			user->callback(c, CONTENT_संदेश_ERROR,
				       &data, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     संरचना browser_window *bw,
	     संरचना content *page,
	     संरचना object_params *params)
{
	संरचना content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	यदि (c->handler->खोलो != रिक्त) {
		res = c->handler->खोलो(c, bw, page, params);
	} अन्यथा {
		res = NSERROR_OK;
	}
	प्रत्यागम res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	संरचना content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	यदि (c == रिक्त) {
		प्रत्यागम NSERROR_BAD_PARAMETER;
	}

	यदि ((c->status != CONTENT_STATUS_READY) &&
	    (c->status != CONTENT_STATUS_DONE)) {
		/* status is not read or done so nothing to do */
		प्रत्यागम NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	यदि (c->textsearch.context != रिक्त) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = रिक्त;
	}

	यदि (c->handler->बंद != रिक्त) {
		res = c->handler->बंद(c);
	} अन्यथा {
		res = NSERROR_OK;
	}
	प्रत्यागम res;
}


/* exported interface, documented in content/content.h */
शून्य content_clear_selection(hlcache_handle *h)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != 0);

	यदि (c->handler->get_selection != रिक्त)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
वर्ण * content_get_selection(hlcache_handle *h)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != 0);

	यदि (c->handler->get_selection != रिक्त)
		प्रत्यागम c->handler->get_selection(c);
	अन्यथा
		प्रत्यागम रिक्त;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(संरचना hlcache_handle *h,
			       पूर्णाङ्क x, पूर्णाङ्क y,
			       संरचना browser_window_features *data)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != 0);

	यदि (c->handler->get_contextual_content != रिक्त) {
		प्रत्यागम c->handler->get_contextual_content(c, x, y, data);
	}

	data->object = h;
	प्रत्यागम NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_point(संरचना hlcache_handle *h,
			पूर्णाङ्क x, पूर्णाङ्क y,
			पूर्णाङ्क scrx, पूर्णाङ्क scry)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != 0);

	यदि (c->handler->scroll_at_point != रिक्त)
		प्रत्यागम c->handler->scroll_at_point(c, x, y, scrx, scry);

	प्रत्यागम false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_point(संरचना hlcache_handle *h,
			   पूर्णाङ्क x, पूर्णाङ्क y,
			   वर्ण *file)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != 0);

	यदि (c->handler->drop_file_at_point != रिक्त)
		प्रत्यागम c->handler->drop_file_at_point(c, x, y, file);

	प्रत्यागम false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(संरचना hlcache_handle *h, खाता *f, गणना content_debug op)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != 0);

	यदि (c->handler->debug_dump == रिक्त) {
		प्रत्यागम NSERROR_NOT_IMPLEMENTED;
	}

	प्रत्यागम c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(संरचना hlcache_handle *h, गणना content_debug op)
{
	संरचना content *c = hlcache_handle_get_content(h);

	यदि (c == रिक्त) {
		प्रत्यागम NSERROR_BAD_PARAMETER;
	}

	यदि (c->handler->debug == रिक्त) {
		प्रत्यागम NSERROR_NOT_IMPLEMENTED;
	}

	प्रत्यागम c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
संरचना content_rfc5988_link *
content_find_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	संरचना content *c = hlcache_handle_get_content(h);
	संरचना content_rfc5988_link *link = c->links;
	bool rel_match = false;

	यावत् (link != रिक्त) {
		यदि (lwc_string_caseless_isequal(link->rel, rel,
						&rel_match) == lwc_error_ok && rel_match) {
			भङ्ग;
		}
		link = link->next;
	}
	प्रत्यागम link;
}


/* exported interface documented in content/protected.h */
संरचना content_rfc5988_link *
content__free_rfc5988_link(संरचना content_rfc5988_link *link)
{
	संरचना content_rfc5988_link *next;

	next = link->next;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	मुक्त(link);

	प्रत्यागम next;
}


/* exported interface documented in content/protected.h */
bool
content__add_rfc5988_link(संरचना content *c,
			  ध्रुव संरचना content_rfc5988_link *link)
{
	संरचना content_rfc5988_link *newlink;
	सङ्घ content_संदेश_data संदेश_data;

	/* a link relation must be present for it to be a link */
	यदि (link->rel == रिक्त) {
		प्रत्यागम false;
	}

	/* a link href must be present for it to be a link */
	यदि (link->href == रिक्त) {
		प्रत्यागम false;
	}

	newlink = सुस्मृति(1, परिमाण(संरचना content_rfc5988_link));
	यदि (newlink == रिक्त) {
		प्रत्यागम false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	यदि (link->hreflang != रिक्त) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	यदि (link->type != रिक्त) {
		newlink->type = lwc_string_ref(link->type);
	}
	यदि (link->media != रिक्त) {
		newlink->media = lwc_string_ref(link->media);
	}
	यदि (link->sizes != रिक्त) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->next = c->links;
	c->links = newlink;

	/* broadcast the data */
	संदेश_data.rfc5988_link = newlink;
	content_broadcast(c, CONTENT_संदेश_LINK, &संदेश_data);

	प्रत्यागम true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम रिक्त;

	प्रत्यागम llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	संरचना content *c = hlcache_handle_get_content(h);

	यदि (c == रिक्त)
		प्रत्यागम CONTENT_NONE;

	प्रत्यागम c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	प्रत्यागम content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम रिक्त;

	प्रत्यागम lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content__set_title(संरचना content *c, ध्रुव वर्ण *title)
{
	वर्ण *new_title = strdup(title);
	यदि (new_title == रिक्त)
		प्रत्यागम false;

	यदि (c->title != रिक्त)
		मुक्त(c->title);

	c->title = new_title;

	प्रत्यागम true;
}


/* exported interface documented in content/content.h */
ध्रुव वर्ण *content_get_title(hlcache_handle *h)
{
	प्रत्यागम content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ध्रुव वर्ण *content__get_title(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम रिक्त;

	प्रत्यागम c->title != रिक्त ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_status content_get_status(hlcache_handle *h)
{
	प्रत्यागम content__get_status(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_status content__get_status(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम CONTENT_STATUS_ERROR;

	प्रत्यागम c->status;
}


/* exported interface documented in content/content.h */
ध्रुव वर्ण *content_get_status_message(hlcache_handle *h)
{
	प्रत्यागम content__get_status_message(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ध्रुव वर्ण *content__get_status_message(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम रिक्त;

	प्रत्यागम c->status_message;
}


/* exported interface documented in content/content.h */
पूर्णाङ्क content_get_width(hlcache_handle *h)
{
	प्रत्यागम content__get_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
पूर्णाङ्क content__get_width(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम 0;

	प्रत्यागम c->width;
}


/* exported interface documented in content/content.h */
पूर्णाङ्क content_get_height(hlcache_handle *h)
{
	प्रत्यागम content__get_height(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
पूर्णाङ्क content__get_height(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम 0;

	प्रत्यागम c->height;
}


/* exported interface documented in content/content.h */
पूर्णाङ्क content_get_available_width(hlcache_handle *h)
{
	प्रत्यागम content__get_available_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
पूर्णाङ्क content__get_available_width(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम 0;

	प्रत्यागम c->available_width;
}


/* exported interface documented in content/content.h */
ध्रुव uint8_t *content_get_source_data(hlcache_handle *h, माप_प्रकार *size)
{
	प्रत्यागम content__get_source_data(hlcache_handle_get_content(h), size);
}


/* exported interface documented in content/content_protected.h */
ध्रुव uint8_t *content__get_source_data(संरचना content *c, माप_प्रकार *size)
{
	प्रतिज्ञा(size != रिक्त);

	/** \todo check if the content check should be an assert */
	यदि (c == रिक्त)
		प्रत्यागम रिक्त;

	प्रत्यागम llcache_handle_get_source_data(c->llcache, size);
}


/* exported interface documented in content/content.h */
शून्य content_invalidate_reuse_data(hlcache_handle *h)
{
	content__invalidate_reuse_data(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
शून्य content__invalidate_reuse_data(संरचना content *c)
{
	यदि (c == रिक्त || c->llcache == रिक्त)
		प्रत्यागम;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_data(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	प्रत्यागम content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम रिक्त;

	प्रत्यागम c->refresh;
}


/* exported interface documented in content/content.h */
संरचना bitmap *content_get_bitmap(hlcache_handle *h)
{
	प्रत्यागम content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
संरचना bitmap *content__get_bitmap(संरचना content *c)
{
	संरचना bitmap *bitmap = रिक्त;

	यदि ((c != रिक्त) &&
	    (c->handler != रिक्त) &&
	    (c->handler->type != रिक्त) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_internal != रिक्त) ) {
		bitmap = c->handler->get_internal(c, रिक्त);
	}

	प्रत्यागम bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	प्रत्यागम content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(संरचना content *c)
{
	यदि ((c != रिक्त) &&
	    (c->handler != रिक्त) &&
	    (c->handler->is_opaque != रिक्त)) {
		प्रत्यागम c->handler->is_opaque(c);
	}

	प्रत्यागम false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	संरचना content *c = hlcache_handle_get_content(h);

	यदि (c == रिक्त)
		प्रत्यागम false;

	प्रत्यागम c->quirks;
}


/* exported interface documented in content/content.h */
ध्रुव वर्ण *
content_get_encoding(hlcache_handle *h, गणना content_encoding_type op)
{
	प्रत्यागम content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
ध्रुव वर्ण *
content__get_encoding(संरचना content *c, गणना content_encoding_type op)
{
	ध्रुव वर्ण *encoding = रिक्त;

	यदि ((c != रिक्त) &&
	    (c->handler != रिक्त) &&
	    (c->handler->get_encoding != रिक्त) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	प्रत्यागम encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	प्रत्यागम content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(संरचना content *c)
{
	प्रत्यागम c->locked;
}


/* exported interface documented in content/content.h */
ध्रुव llcache_handle *content_get_llcache_handle(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम रिक्त;

	प्रत्यागम c->llcache;
}


/* exported interface documented in content/protected.h */
संरचना content *content_clone(संरचना content *c)
{
	संरचना content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	यदि (error != NSERROR_OK)
		प्रत्यागम रिक्त;

	प्रत्यागम nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(ध्रुव संरचना content *c, संरचना content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	यदि (error != NSERROR_OK) {
		प्रत्यागम error;
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

	यदि (c->fallback_charset != रिक्त) {
		nc->fallback_charset = strdup(c->fallback_charset);
		यदि (nc->fallback_charset == रिक्त) {
			प्रत्यागम NSERROR_NOMEM;
		}
	}

	यदि (c->refresh != रिक्त) {
		nc->refresh = nsurl_ref(c->refresh);
		यदि (nc->refresh == रिक्त) {
			प्रत्यागम NSERROR_NOMEM;
		}
	}

	nc->समय = c->समय;
	nc->reformat_time = c->reformat_time;
	nc->size = c->size;

	यदि (c->title != रिक्त) {
		nc->title = strdup(c->title);
		यदि (nc->title == रिक्त) {
			प्रत्यागम NSERROR_NOMEM;
		}
	}

	nc->active = c->active;

	nc->user_list = सुस्मृति(1, परिमाण(संरचना content_user));
	यदि (nc->user_list == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	स_नकल(&(nc->status_message), &(c->status_message), 120);
	स_नकल(&(nc->sub_status), &(c->sub_status), 80);

	nc->locked = c->locked;
	nc->total_size = c->total_size;
	nc->http_code = c->http_code;

	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(संरचना content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	यदि (c->handler->stop != रिक्त)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	प्रत्यागम llcache_handle_abort(c->llcache);
}
