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

#समावेश <stdint.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <nsutils/time.h>

#समावेश "netsurf/inttypes.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "utils/corestrings.h"
#समावेश "netsurf/browser_window.h"
#समावेश "netsurf/bitmap.h"
#समावेश "netsurf/content.h"
#समावेश "desktop/knockout.h"

#समावेश "content/content_protected.h"
#समावेश "content/textsearch.h"
#समावेश "content/content_debug.h"
#समावेश "content/hlcache.h"
#समावेश "content/urldb.h"

#घोषणा URL_FMT_SPC "%.140s"

स्थिर अक्षर * स्थिर content_status_name[] = {
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
अटल व्योम content_convert(काष्ठा content *c)
{
	निश्चित(c);
	निश्चित(c->status == CONTENT_STATUS_LOADING ||
	       c->status == CONTENT_STATUS_ERROR);

	अगर (c->status != CONTENT_STATUS_LOADING)
		वापस;

	अगर (c->locked == true)
		वापस;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	अगर (c->handler->तथ्य_complete != शून्य) {
		c->locked = true;
		अगर (c->handler->तथ्य_complete(c) == false) {
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
अटल nserror
content_llcache_callback(llcache_handle *llcache,
			 स्थिर llcache_event *event, व्योम *pw)
{
	काष्ठा content *c = pw;
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;
	nserror error = NSERROR_OK;

	चयन (event->type) {
	हाल LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		अवरोध;
	हाल LLCACHE_EVENT_HAD_HEADERS:
		/* Will never happen: handled in hlcache */
		अवरोध;
	हाल LLCACHE_EVENT_HAD_तथ्य:
		अगर (c->handler->process_तथ्य != शून्य) {
			अगर (c->handler->process_तथ्य(c,
						     (स्थिर अक्षर *) event->तथ्य.तथ्य.buf,
						     event->तथ्य.तथ्य.len) == false) {
				llcache_handle_abort(c->llcache);
				c->status = CONTENT_STATUS_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		अवरोध;
	हाल LLCACHE_EVENT_DONE:
		{
			माप_प्रकार source_आकार;

			(व्योम) llcache_handle_get_source_तथ्य(llcache, &source_आकार);

			content_set_status(c, messages_get("Processing"));
			संदेश_तथ्य.explicit_status_text = शून्य;
			content_broadcast(c, CONTENT_संदेश_STATUS, &संदेश_तथ्य);

			content_convert(c);
		}
		अवरोध;
	हाल LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->status = CONTENT_STATUS_ERROR;
		संदेश_तथ्य.errordata.errorcode = event->तथ्य.error.code;
		संदेश_तथ्य.errordata.errormsg = event->तथ्य.error.संदेश;
		content_broadcast(c, CONTENT_संदेश_ERROR, &संदेश_तथ्य);
		अवरोध;
	हाल LLCACHE_EVENT_PROGRESS:
		content_set_status(c, event->तथ्य.progress_संदेश);
		संदेश_तथ्य.explicit_status_text = शून्य;
		content_broadcast(c, CONTENT_संदेश_STATUS, &संदेश_तथ्य);
		अवरोध;
	हाल LLCACHE_EVENT_REDIRECT:
		संदेश_तथ्य.redirect.from = event->तथ्य.redirect.from;
		संदेश_तथ्य.redirect.to = event->तथ्य.redirect.to;
		content_broadcast(c, CONTENT_संदेश_REDIRECT, &संदेश_तथ्य);
		अवरोध;
	}

	वापस error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
अटल व्योम content_update_status(काष्ठा content *c)
{
	अगर (c->status == CONTENT_STATUS_LOADING ||
	    c->status == CONTENT_STATUS_READY) {
		/* Not done yet */
		snprintf(c->status_message, माप (c->status_message),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_status[0] != '\0' ? ", " : " ",
			 c->sub_status);
	} अन्यथा {
		snprintf(c->status_message, माप (c->status_message),
			 "%s (%.1fs)", messages_get("Done"),
			 (भग्न) c->समय / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(काष्ठा content *c,
	      स्थिर content_handler *handler,
	      lwc_string *imime_type,
	      स्थिर काष्ठा http_parameter *params,
	      llcache_handle *llcache,
	      स्थिर अक्षर *fallback_charset,
	      bool quirks)
{
	काष्ठा content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = सुस्मृति(1, माप(काष्ठा content_user));
	अगर (user_sentinel == शून्य) {
		वापस NSERROR_NOMEM;
	}

	अगर (fallback_charset != शून्य) {
		c->fallback_charset = strdup(fallback_charset);
		अगर (c->fallback_charset == शून्य) {
			मुक्त(user_sentinel);
			वापस NSERROR_NOMEM;
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
	c->आकार = 0;
	c->title = शून्य;
	c->active = 0;
	user_sentinel->callback = शून्य;
	user_sentinel->pw = शून्य;
	user_sentinel->अग्र = शून्य;
	c->user_list = user_sentinel;
	c->sub_status[0] = 0;
	c->locked = false;
	c->योग_आकार = 0;
	c->http_code = 0;

	c->textsearch.string = शून्य;
	c->textsearch.context = शून्य;

	content_set_status(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	अगर (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		वापस error;
	}

	वापस NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	काष्ठा content *c = hlcache_handle_get_content(h);

	अगर (c == शून्य)
		वापस false;

	वापस (c->handler->reformat != शून्य);
}


/* exported interface documented in content/protected.h */
व्योम content_set_status(काष्ठा content *c, स्थिर अक्षर *status_message)
{
	माप_प्रकार len = म_माप(status_message);

	अगर (len >= माप(c->sub_status)) {
		len = माप(c->sub_status) - 1;
	}
	स_नकल(c->sub_status, status_message, len);
	c->sub_status[len] = '\0';

	content_update_status(c);
}


/* exported interface documented in content/protected.h */
व्योम content_set_ready(काष्ठा content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	निश्चित(c->locked);
	c->locked = false;

	c->status = CONTENT_STATUS_READY;
	content_update_status(c);
	content_broadcast(c, CONTENT_संदेश_READY, शून्य);
}


/* exported interface documented in content/protected.h */
व्योम content_set_done(काष्ठा content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->status = CONTENT_STATUS_DONE;
	c->समय = now_ms - c->समय;
	content_update_status(c);
	content_broadcast(c, CONTENT_संदेश_DONE, शून्य);
}


/* exported interface documented in content/protected.h */
व्योम content_set_error(काष्ठा content *c)
{
	c->locked = false;
	c->status = CONTENT_STATUS_ERROR;
}


/* exported interface documented in content/content.h */
व्योम content_reformat(hlcache_handle *h, bool background,
		      पूर्णांक width, पूर्णांक height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  width, height);
}


/* exported interface documented in content/protected.h */
व्योम
content__reformat(काष्ठा content *c, bool background, पूर्णांक width, पूर्णांक height)
{
	जोड़ content_संदेश_तथ्य तथ्य;
	निश्चित(c != 0);
	निश्चित(c->status == CONTENT_STATUS_READY ||
	       c->status == CONTENT_STATUS_DONE);
	निश्चित(c->locked == false);

	c->available_width = width;
	c->available_height = height;
	अगर (c->handler->reformat != शून्य) {

		c->locked = true;
		c->handler->reformat(c, width, height);
		c->locked = false;

		तथ्य.background = background;
		content_broadcast(c, CONTENT_संदेश_REFORMAT, &तथ्य);
	}
}


/* exported interface documented in content/content.h */
व्योम content_destroy(काष्ठा content *c)
{
	काष्ठा content_rfc5988_link *link;

	निश्चित(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	निश्चित(c->locked == false);

	अगर (c->handler->destroy != शून्य)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = शून्य;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	जबतक (link != शून्य) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	अगर (c->user_list != शून्य) {
		मुक्त(c->user_list);
	}

	/* free the title */
	अगर (c->title != शून्य) {
		मुक्त(c->title);
	}

	/* free the fallback characterset */
	अगर (c->fallback_charset != शून्य) {
		मुक्त(c->fallback_charset);
	}

	मुक्त(c);
}


/* exported interface documented in content/content.h */
व्योम
content_mouse_track(hlcache_handle *h,
		    काष्ठा browser_window *bw,
		    browser_mouse_state mouse,
		    पूर्णांक x, पूर्णांक y)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	निश्चित(c != शून्य);

	अगर (c->handler->mouse_track != शून्य) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} अन्यथा {
		जोड़ content_संदेश_तथ्य संदेश_तथ्य;
		संदेश_तथ्य.pointer = BROWSER_POINTER_AUTO;
		content_broadcast(c, CONTENT_संदेश_POINTER, &संदेश_तथ्य);
	}


	वापस;
}


/* exported interface documented in content/content.h */
व्योम
content_mouse_action(hlcache_handle *h,
		     काष्ठा browser_window *bw,
		     browser_mouse_state mouse,
		     पूर्णांक x, पूर्णांक y)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	निश्चित(c != शून्य);

	अगर (c->handler->mouse_action != शून्य)
		c->handler->mouse_action(c, bw, mouse, x, y);

	वापस;
}


/* exported interface documented in content/content.h */
bool content_keypress(काष्ठा hlcache_handle *h, uint32_t कुंजी)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	निश्चित(c != शून्य);

	अगर (c->handler->keypress != शून्य)
		वापस c->handler->keypress(c, कुंजी);

	वापस false;
}


/* exported interface documented in content/content.h */
व्योम content_request_redraw(काष्ठा hlcache_handle *h,
			    पूर्णांक x, पूर्णांक y, पूर्णांक width, पूर्णांक height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, width, height);
}


/* exported interface, documented in content/protected.h */
व्योम content__request_redraw(काष्ठा content *c,
			     पूर्णांक x, पूर्णांक y, पूर्णांक width, पूर्णांक height)
{
	जोड़ content_संदेश_तथ्य तथ्य;

	अगर (c == शून्य)
		वापस;

	तथ्य.redraw.x = x;
	तथ्य.redraw.y = y;
	तथ्य.redraw.width = width;
	तथ्य.redraw.height = height;

	content_broadcast(c, CONTENT_संदेश_REDRAW, &तथ्य);
}


/* exported interface, documented in content/content.h */
bool content_exec(काष्ठा hlcache_handle *h, स्थिर अक्षर *src, माप_प्रकार srclen)
{
	काष्ठा content *c = hlcache_handle_get_content(h);

	निश्चित(c != शून्य);

	अगर (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		वापस false;
	}

	अगर (c->handler->exec == शून्य) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		वापस false;
	}

	वापस c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(काष्ठा hlcache_handle *h)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	काष्ठा nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool match;

	/* Is this an internal scheme? If so, we trust here and stop */
	अगर ((lwc_string_isequal(scheme, corestring_lwc_about,
				&match) == lwc_error_ok &&
	     (match == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_तथ्य,
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
		वापस false;
	}

	/* Okay, not internal, am *I* secure? */
	अगर ((lwc_string_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok)
	    && (match == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		वापस true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	अगर (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		वापस true;
	}

	/* Otherwise try and chain through the handler */
	अगर (c != शून्य && c->handler->saw_insecure_objects != शून्य) {
		वापस c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	वापस false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       काष्ठा content_redraw_तथ्य *तथ्य,
	       स्थिर काष्ठा rect *clip,
	       स्थिर काष्ठा redraw_context *ctx)
{
	काष्ठा content *c = hlcache_handle_get_content(h);

	निश्चित(c != शून्य);

	अगर (c->locked) {
		/* not safe to attempt redraw */
		वापस true;
	}

	/* ensure we have a redrawable content */
	अगर (c->handler->redraw == शून्य) {
		वापस true;
	}

	वापस c->handler->redraw(c, तथ्य, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(काष्ठा hlcache_handle *h,
		      पूर्णांक width, पूर्णांक height,
		      स्थिर काष्ठा redraw_context *ctx)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	काष्ठा redraw_context new_ctx = *ctx;
	काष्ठा rect clip;
	काष्ठा content_redraw_तथ्य तथ्य;
	bool plot_ok = true;

	निश्चित(c != शून्य);

	/* ensure it is safe to attempt redraw */
	अगर (c->locked) {
		वापस true;
	}

	/* ensure we have a redrawable content */
	अगर (c->handler->redraw == शून्य) {
		वापस true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, width, height, ctx);

	अगर (ctx->plot->option_knockout) {
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
	तथ्य.x = 0;
	तथ्य.y = 0;
	तथ्य.width = width;
	तथ्य.height = height;

	तथ्य.background_colour = 0xFFFFFF;
	तथ्य.repeat_x = false;
	तथ्य.repeat_y = false;

	/* Find the scale factor to use if the content has a width */
	अगर (c->width) {
		तथ्य.scale = (भग्न)width / (भग्न)c->width;
	} अन्यथा {
		तथ्य.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &तथ्य, &clip, &new_ctx);

	अगर (ctx->plot->option_knockout) {
		knockout_plot_end(ctx);
	}

	वापस plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_add_user(काष्ठा content *c,
		 व्योम (*callback)(
				  काष्ठा content *c,
				  content_संदेश संदेश,
				  स्थिर जोड़ content_संदेश_तथ्य *तथ्य,
				  व्योम *pw),
		 व्योम *pw)
{
	काष्ठा content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = दो_स्मृति(माप(काष्ठा content_user));
	अगर (!user)
		वापस false;
	user->callback = callback;
	user->pw = pw;
	user->अग्र = c->user_list->अग्र;
	c->user_list->अग्र = user;

	अगर (c->handler->add_user != शून्य)
		c->handler->add_user(c);

	वापस true;
}


/* exported interface documented in content/content.h */
व्योम
content_remove_user(काष्ठा content *c,
		    व्योम (*callback)(
				     काष्ठा content *c,
				     content_संदेश संदेश,
				     स्थिर जोड़ content_संदेश_तथ्य *तथ्य,
				     व्योम *pw),
		    व्योम *pw)
{
	काष्ठा content_user *user, *अग्र;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	क्रम (user = c->user_list; user->अग्र != 0 &&
		     !(user->अग्र->callback == callback &&
		       user->अग्र->pw == pw); user = user->अग्र)
		;
	अगर (user->अग्र == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		निश्चित(0);
		वापस;
	}

	अगर (c->handler->remove_user != शून्य)
		c->handler->remove_user(c);

	अग्र = user->अग्र;
	user->अग्र = अग्र->अग्र;
	मुक्त(अग्र);
}


/* exported interface documented in content/content.h */
uint32_t content_गिनती_users(काष्ठा content *c)
{
	काष्ठा content_user *user;
	uint32_t गणक = 0;

	निश्चित(c != शून्य);

	क्रम (user = c->user_list; user != शून्य; user = user->अग्र)
		गणक += 1;

	निश्चित(गणक > 0);

	वापस गणक - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(काष्ठा content *c, bool quirks)
{
	अगर (c->handler->matches_quirks == शून्य)
		वापस true;

	वापस c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(काष्ठा content *c)
{
	वापस c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
व्योम content_broadcast(काष्ठा content *c, content_संदेश संदेश,
		       स्थिर जोड़ content_संदेश_तथ्य *तथ्य)
{
	काष्ठा content_user *user, *अग्र;
	निश्चित(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, संदेश);
	क्रम (user = c->user_list->अग्र; user != 0; user = अग्र) {
		अग्र = user->अग्र;  /* user may be destroyed during callback */
		अगर (user->callback != 0)
			user->callback(c, संदेश, तथ्य, user->pw);
	}
}


/* exported interface documented in content_protected.h */
व्योम
content_broadcast_error(काष्ठा content *c, nserror errorcode, स्थिर अक्षर *संदेश)
{
	काष्ठा content_user *user, *अग्र;
	जोड़ content_संदेश_तथ्य तथ्य;

	निश्चित(c);

	तथ्य.errordata.errorcode = errorcode;
	तथ्य.errordata.errormsg = संदेश;

	क्रम (user = c->user_list->अग्र; user != 0; user = अग्र) {
		अग्र = user->अग्र;  /* user may be destroyed during callback */
		अगर (user->callback != 0) {
			user->callback(c, CONTENT_संदेश_ERROR,
				       &तथ्य, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     काष्ठा browser_window *bw,
	     काष्ठा content *page,
	     काष्ठा object_params *params)
{
	काष्ठा content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	निश्चित(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	अगर (c->handler->खोलो != शून्य) {
		res = c->handler->खोलो(c, bw, page, params);
	} अन्यथा {
		res = NSERROR_OK;
	}
	वापस res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	काष्ठा content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	अगर (c == शून्य) {
		वापस NSERROR_BAD_PARAMETER;
	}

	अगर ((c->status != CONTENT_STATUS_READY) &&
	    (c->status != CONTENT_STATUS_DONE)) {
		/* status is not read or done so nothing to do */
		वापस NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	अगर (c->textsearch.context != शून्य) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = शून्य;
	}

	अगर (c->handler->बंद != शून्य) {
		res = c->handler->बंद(c);
	} अन्यथा {
		res = NSERROR_OK;
	}
	वापस res;
}


/* exported interface, documented in content/content.h */
व्योम content_clear_selection(hlcache_handle *h)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	निश्चित(c != 0);

	अगर (c->handler->get_selection != शून्य)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
अक्षर * content_get_selection(hlcache_handle *h)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	निश्चित(c != 0);

	अगर (c->handler->get_selection != शून्य)
		वापस c->handler->get_selection(c);
	अन्यथा
		वापस शून्य;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(काष्ठा hlcache_handle *h,
			       पूर्णांक x, पूर्णांक y,
			       काष्ठा browser_window_features *तथ्य)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	निश्चित(c != 0);

	अगर (c->handler->get_contextual_content != शून्य) {
		वापस c->handler->get_contextual_content(c, x, y, तथ्य);
	}

	तथ्य->object = h;
	वापस NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_point(काष्ठा hlcache_handle *h,
			पूर्णांक x, पूर्णांक y,
			पूर्णांक scrx, पूर्णांक scry)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	निश्चित(c != 0);

	अगर (c->handler->scroll_at_point != शून्य)
		वापस c->handler->scroll_at_point(c, x, y, scrx, scry);

	वापस false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_point(काष्ठा hlcache_handle *h,
			   पूर्णांक x, पूर्णांक y,
			   अक्षर *file)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	निश्चित(c != 0);

	अगर (c->handler->drop_file_at_point != शून्य)
		वापस c->handler->drop_file_at_point(c, x, y, file);

	वापस false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(काष्ठा hlcache_handle *h, खाता *f, क्रमागत content_debug op)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	निश्चित(c != 0);

	अगर (c->handler->debug_dump == शून्य) {
		वापस NSERROR_NOT_IMPLEMENTED;
	}

	वापस c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(काष्ठा hlcache_handle *h, क्रमागत content_debug op)
{
	काष्ठा content *c = hlcache_handle_get_content(h);

	अगर (c == शून्य) {
		वापस NSERROR_BAD_PARAMETER;
	}

	अगर (c->handler->debug == शून्य) {
		वापस NSERROR_NOT_IMPLEMENTED;
	}

	वापस c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
काष्ठा content_rfc5988_link *
content_find_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	काष्ठा content *c = hlcache_handle_get_content(h);
	काष्ठा content_rfc5988_link *link = c->links;
	bool rel_match = false;

	जबतक (link != शून्य) {
		अगर (lwc_string_caseless_isequal(link->rel, rel,
						&rel_match) == lwc_error_ok && rel_match) {
			अवरोध;
		}
		link = link->अग्र;
	}
	वापस link;
}


/* exported interface documented in content/protected.h */
काष्ठा content_rfc5988_link *
content__free_rfc5988_link(काष्ठा content_rfc5988_link *link)
{
	काष्ठा content_rfc5988_link *अग्र;

	अग्र = link->अग्र;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	मुक्त(link);

	वापस अग्र;
}


/* exported interface documented in content/protected.h */
bool
content__add_rfc5988_link(काष्ठा content *c,
			  स्थिर काष्ठा content_rfc5988_link *link)
{
	काष्ठा content_rfc5988_link *newlink;
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;

	/* a link relation must be present for it to be a link */
	अगर (link->rel == शून्य) {
		वापस false;
	}

	/* a link href must be present for it to be a link */
	अगर (link->href == शून्य) {
		वापस false;
	}

	newlink = सुस्मृति(1, माप(काष्ठा content_rfc5988_link));
	अगर (newlink == शून्य) {
		वापस false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	अगर (link->hreflang != शून्य) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	अगर (link->type != शून्य) {
		newlink->type = lwc_string_ref(link->type);
	}
	अगर (link->media != शून्य) {
		newlink->media = lwc_string_ref(link->media);
	}
	अगर (link->sizes != शून्य) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->अग्र = c->links;
	c->links = newlink;

	/* broadcast the data */
	संदेश_तथ्य.rfc5988_link = newlink;
	content_broadcast(c, CONTENT_संदेश_LINK, &संदेश_तथ्य);

	वापस true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(काष्ठा content *c)
{
	अगर (c == शून्य)
		वापस शून्य;

	वापस llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	काष्ठा content *c = hlcache_handle_get_content(h);

	अगर (c == शून्य)
		वापस CONTENT_NONE;

	वापस c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	वापस content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(काष्ठा content *c)
{
	अगर (c == शून्य)
		वापस शून्य;

	वापस lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content__set_title(काष्ठा content *c, स्थिर अक्षर *title)
{
	अक्षर *new_title = strdup(title);
	अगर (new_title == शून्य)
		वापस false;

	अगर (c->title != शून्य)
		मुक्त(c->title);

	c->title = new_title;

	वापस true;
}


/* exported interface documented in content/content.h */
स्थिर अक्षर *content_get_title(hlcache_handle *h)
{
	वापस content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
स्थिर अक्षर *content__get_title(काष्ठा content *c)
{
	अगर (c == शून्य)
		वापस शून्य;

	वापस c->title != शून्य ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_status content_get_status(hlcache_handle *h)
{
	वापस content__get_status(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_status content__get_status(काष्ठा content *c)
{
	अगर (c == शून्य)
		वापस CONTENT_STATUS_ERROR;

	वापस c->status;
}


/* exported interface documented in content/content.h */
स्थिर अक्षर *content_get_status_message(hlcache_handle *h)
{
	वापस content__get_status_message(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
स्थिर अक्षर *content__get_status_message(काष्ठा content *c)
{
	अगर (c == शून्य)
		वापस शून्य;

	वापस c->status_message;
}


/* exported interface documented in content/content.h */
पूर्णांक content_get_width(hlcache_handle *h)
{
	वापस content__get_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
पूर्णांक content__get_width(काष्ठा content *c)
{
	अगर (c == शून्य)
		वापस 0;

	वापस c->width;
}


/* exported interface documented in content/content.h */
पूर्णांक content_get_height(hlcache_handle *h)
{
	वापस content__get_height(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
पूर्णांक content__get_height(काष्ठा content *c)
{
	अगर (c == शून्य)
		वापस 0;

	वापस c->height;
}


/* exported interface documented in content/content.h */
पूर्णांक content_get_available_width(hlcache_handle *h)
{
	वापस content__get_available_width(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
पूर्णांक content__get_available_width(काष्ठा content *c)
{
	अगर (c == शून्य)
		वापस 0;

	वापस c->available_width;
}


/* exported interface documented in content/content.h */
स्थिर uint8_t *content_get_source_तथ्य(hlcache_handle *h, माप_प्रकार *आकार)
{
	वापस content_get_source_तथ्य_2(hlcache_handle_get_content(h), आकार);
}


/* exported interface documented in content/content_protected.h */
स्थिर uint8_t *content_get_source_तथ्य_2(काष्ठा content *c, माप_प्रकार *आकार)
{
	निश्चित(आकार != शून्य);

	/** \todo check if the content check should be an assert */
	अगर (c == शून्य)
		वापस शून्य;

	वापस llcache_handle_get_source_तथ्य(c->llcache, आकार);
}


/* exported interface documented in content/content.h */
व्योम content_invalidate_reuse_तथ्य(hlcache_handle *h)
{
	content_invalidate_reuse_तथ्य_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
व्योम content_invalidate_reuse_तथ्य_2(काष्ठा content *c)
{
	अगर (c == शून्य || c->llcache == शून्य)
		वापस;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_तथ्य(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	वापस content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(काष्ठा content *c)
{
	अगर (c == शून्य)
		वापस शून्य;

	वापस c->refresh;
}


/* exported interface documented in content/content.h */
काष्ठा bitmap *content_get_bitmap(hlcache_handle *h)
{
	वापस content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
काष्ठा bitmap *content__get_bitmap(काष्ठा content *c)
{
	काष्ठा bitmap *bitmap = शून्य;

	अगर ((c != शून्य) &&
	    (c->handler != शून्य) &&
	    (c->handler->type != शून्य) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_internal != शून्य) ) {
		bitmap = c->handler->get_internal(c, शून्य);
	}

	वापस bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	वापस content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(काष्ठा content *c)
{
	अगर ((c != शून्य) &&
	    (c->handler != शून्य) &&
	    (c->handler->is_opaque != शून्य)) {
		वापस c->handler->is_opaque(c);
	}

	वापस false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	काष्ठा content *c = hlcache_handle_get_content(h);

	अगर (c == शून्य)
		वापस false;

	वापस c->quirks;
}


/* exported interface documented in content/content.h */
स्थिर अक्षर *
content_get_encoding(hlcache_handle *h, क्रमागत content_encoding_type op)
{
	वापस content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
स्थिर अक्षर *
content__get_encoding(काष्ठा content *c, क्रमागत content_encoding_type op)
{
	स्थिर अक्षर *encoding = शून्य;

	अगर ((c != शून्य) &&
	    (c->handler != शून्य) &&
	    (c->handler->get_encoding != शून्य) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	वापस encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	वापस content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(काष्ठा content *c)
{
	वापस c->locked;
}


/* exported interface documented in content/content.h */
स्थिर llcache_handle *content_get_llcache_handle(काष्ठा content *c)
{
	अगर (c == शून्य)
		वापस शून्य;

	वापस c->llcache;
}


/* exported interface documented in content/protected.h */
काष्ठा content *content_clone(काष्ठा content *c)
{
	काष्ठा content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	अगर (error != NSERROR_OK)
		वापस शून्य;

	वापस nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(स्थिर काष्ठा content *c, काष्ठा content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	अगर (error != NSERROR_OK) {
		वापस error;
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

	अगर (c->fallback_charset != शून्य) {
		nc->fallback_charset = strdup(c->fallback_charset);
		अगर (nc->fallback_charset == शून्य) {
			वापस NSERROR_NOMEM;
		}
	}

	अगर (c->refresh != शून्य) {
		nc->refresh = nsurl_ref(c->refresh);
		अगर (nc->refresh == शून्य) {
			वापस NSERROR_NOMEM;
		}
	}

	nc->समय = c->समय;
	nc->reformat_time = c->reformat_time;
	nc->आकार = c->आकार;

	अगर (c->title != शून्य) {
		nc->title = strdup(c->title);
		अगर (nc->title == शून्य) {
			वापस NSERROR_NOMEM;
		}
	}

	nc->active = c->active;

	nc->user_list = सुस्मृति(1, माप(काष्ठा content_user));
	अगर (nc->user_list == शून्य) {
		वापस NSERROR_NOMEM;
	}

	स_नकल(&(nc->status_message), &(c->status_message), 120);
	स_नकल(&(nc->sub_status), &(c->sub_status), 80);

	nc->locked = c->locked;
	nc->योग_आकार = c->योग_आकार;
	nc->http_code = c->http_code;

	वापस NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(काष्ठा content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	अगर (c->handler->stop != शून्य)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	वापस llcache_handle_abort(c->llcache);
}
