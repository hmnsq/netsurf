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

#परिभाषय URL_FMT_SPC "%.140s"

ध्रुव वर्ण * ध्रुव content_स्थितिः_name[] = {
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
	प्रतिज्ञा(c->स्थितिः == CONTENT_स्थितिः_LOADING ||
	       c->स्थितिः == CONTENT_स्थितिः_ERROR);

	यदि (c->स्थितिः != CONTENT_स्थितिः_LOADING)
		प्रत्यागम;

	यदि (c->locked == true)
		प्रत्यागम;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	यदि (c->handler->दत्त_complete != रिक्त) {
		c->locked = true;
		यदि (c->handler->दत्त_complete(c) == false) {
			content_समुच्चयः_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} अन्यथा {
		content_समुच्चयः_ready(c);
		content_समुच्चयः_समाप्तम्(c);
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
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
	nserror error = NSERROR_OK;

	विकल्प (event->type) {
	पक्ष LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		भङ्ग;
	पक्ष LLCACHE_EVENT_HAD_शीर्षकाणि:
		/* Will never happen: handled in hlcache */
		भङ्ग;
	पक्ष LLCACHE_EVENT_HAD_दत्त:
		यदि (c->handler->process_दत्त != रिक्त) {
			यदि (c->handler->process_दत्त(c,
						     (ध्रुव वर्ण *) event->दत्त.दत्त.buf,
						     event->दत्त.दत्त.len) == false) {
				llcache_handle_abort(c->llcache);
				c->स्थितिः = CONTENT_स्थितिः_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		भङ्ग;
	पक्ष LLCACHE_EVENT_समाप्तम्:
		{
			size_t source_आकार;

			(शून्य) llcache_handle_get_source_दत्त(llcache, &source_आकार);

			content_समुच्चयः_स्थितिः(c, messages_get("Processing"));
			सन्देशः_दत्त.explicit_स्थितिः_text = रिक्त;
			content_प्रसारणम्(c, CONTENT_सन्देशः_स्थितिः, &सन्देशः_दत्त);

			content_convert(c);
		}
		भङ्ग;
	पक्ष LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->स्थितिः = CONTENT_स्थितिः_ERROR;
		सन्देशः_दत्त.errordata.errorcode = event->दत्त.error.संहिता;
		सन्देशः_दत्त.errordata.errormsg = event->दत्त.error.सन्देशः;
		content_प्रसारणम्(c, CONTENT_सन्देशः_ERROR, &सन्देशः_दत्त);
		भङ्ग;
	पक्ष LLCACHE_EVENT_प्रगतिः:
		content_समुच्चयः_स्थितिः(c, event->दत्त.प्रगतिः_सन्देशः);
		सन्देशः_दत्त.explicit_स्थितिः_text = रिक्त;
		content_प्रसारणम्(c, CONTENT_सन्देशः_स्थितिः, &सन्देशः_दत्त);
		भङ्ग;
	पक्ष LLCACHE_EVENT_REDIRECT:
		सन्देशः_दत्त.redirect.from = event->दत्त.redirect.from;
		सन्देशः_दत्त.redirect.गन्तव्यम् = event->दत्त.redirect.गन्तव्यम्;
		content_प्रसारणम्(c, CONTENT_सन्देशः_REDIRECT, &सन्देशः_दत्त);
		भङ्ग;
	}

	प्रत्यागम error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
स्थिर शून्य content_update_स्थितिः(संरचना content *c)
{
	यदि (c->स्थितिः == CONTENT_स्थितिः_LOADING ||
	    c->स्थितिः == CONTENT_स्थितिः_READY) {
		/* Not done yet */
		snprintf(c->स्थितिः_सन्देशः, परिमाण (c->स्थितिः_सन्देशः),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_स्थितिः[0] != '\0' ? ", " : " ",
			 c->sub_स्थितिः);
	} अन्यथा {
		snprintf(c->स्थितिः_सन्देशः, परिमाण (c->स्थितिः_सन्देशः),
			 "%s (%.1fs)", messages_get("Done"),
			 (भिन्न) c->time / 1000);
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

	user_sentinel = शून्यस्मृतिं_देहि(1, परिमाण(संरचना content_user));
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
	c->स्थितिः = CONTENT_स्थितिः_LOADING;
	c->विस्तारः = 0;
	c->height = 0;
	c->उपलब्धम्_विस्तारः = 0;
	c->उपलब्धम्_height = 0;
	c->quirks = quirks;
	c->refresh = 0;
	nsu_getmonotonic_ms(&c->time);
	c->आकार = 0;
	c->title = रिक्त;
	c->सक्रिय = 0;
	user_sentinel->callback = रिक्त;
	user_sentinel->pw = रिक्त;
	user_sentinel->अग्र = रिक्त;
	c->user_list = user_sentinel;
	c->sub_स्थितिः[0] = 0;
	c->locked = false;
	c->योग_आकार = 0;
	c->http_संहिता = 0;

	c->textsearch.string = रिक्त;
	c->textsearch.context = रिक्त;

	content_समुच्चयः_स्थितिः(c, messages_get("Loading"));

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
शून्य content_समुच्चयः_स्थितिः(संरचना content *c, ध्रुव वर्ण *स्थितिः_सन्देशः)
{
	size_t len = वाक्यदैर्घ्यम्(स्थितिः_सन्देशः);

	यदि (len >= परिमाण(c->sub_स्थितिः)) {
		len = परिमाण(c->sub_स्थितिः) - 1;
	}
	स्मृतिं_प्रतिलिख(c->sub_स्थितिः, स्थितिः_सन्देशः, len);
	c->sub_स्थितिः[len] = '\0';

	content_update_स्थितिः(c);
}


/* exported interface documented in content/protected.h */
शून्य content_समुच्चयः_ready(संरचना content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	प्रतिज्ञा(c->locked);
	c->locked = false;

	c->स्थितिः = CONTENT_स्थितिः_READY;
	content_update_स्थितिः(c);
	content_प्रसारणम्(c, CONTENT_सन्देशः_READY, रिक्त);
}


/* exported interface documented in content/protected.h */
शून्य content_समुच्चयः_समाप्तम्(संरचना content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->स्थितिः = CONTENT_स्थितिः_समाप्तम्;
	c->time = now_ms - c->time;
	content_update_स्थितिः(c);
	content_प्रसारणम्(c, CONTENT_सन्देशः_समाप्तम्, रिक्त);
}


/* exported interface documented in content/protected.h */
शून्य content_समुच्चयः_error(संरचना content *c)
{
	c->locked = false;
	c->स्थितिः = CONTENT_स्थितिः_ERROR;
}


/* exported interface documented in content/content.h */
शून्य content_reformat(hlcache_handle *h, bool background,
		      पूर्णाङ्क विस्तारः, पूर्णाङ्क height)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  विस्तारः, height);
}


/* exported interface documented in content/protected.h */
शून्य
content__reformat(संरचना content *c, bool background, पूर्णाङ्क विस्तारः, पूर्णाङ्क height)
{
	सङ्घ content_सन्देशः_दत्त दत्त;
	प्रतिज्ञा(c != 0);
	प्रतिज्ञा(c->स्थितिः == CONTENT_स्थितिः_READY ||
	       c->स्थितिः == CONTENT_स्थितिः_समाप्तम्);
	प्रतिज्ञा(c->locked == false);

	c->उपलब्धम्_विस्तारः = विस्तारः;
	c->उपलब्धम्_height = height;
	यदि (c->handler->reformat != रिक्त) {

		c->locked = true;
		c->handler->reformat(c, विस्तारः, height);
		c->locked = false;

		दत्त.background = background;
		content_प्रसारणम्(c, CONTENT_सन्देशः_REFORMAT, &दत्त);
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
		    संरचना browser_गवाक्षः *bw,
		    browser_mouse_state mouse,
		    पूर्णाङ्क x, पूर्णाङ्क y)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != रिक्त);

	यदि (c->handler->mouse_track != रिक्त) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} अन्यथा {
		सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;
		सन्देशः_दत्त.pointer = BROWSER_POINTER_AUTO;
		content_प्रसारणम्(c, CONTENT_सन्देशः_POINTER, &सन्देशः_दत्त);
	}


	प्रत्यागम;
}


/* exported interface documented in content/content.h */
शून्य
content_mouse_action(hlcache_handle *h,
		     संरचना browser_गवाक्षः *bw,
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
bool content_keypress(संरचना hlcache_handle *h, uint32_t कुञ्चिका)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != रिक्त);

	यदि (c->handler->keypress != रिक्त)
		प्रत्यागम c->handler->keypress(c, कुञ्चिका);

	प्रत्यागम false;
}


/* exported interface documented in content/content.h */
शून्य content_request_redraw(संरचना hlcache_handle *h,
			    पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क विस्तारः, पूर्णाङ्क height)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, विस्तारः, height);
}


/* exported interface, documented in content/protected.h */
शून्य content__request_redraw(संरचना content *c,
			     पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क विस्तारः, पूर्णाङ्क height)
{
	सङ्घ content_सन्देशः_दत्त दत्त;

	यदि (c == रिक्त)
		प्रत्यागम;

	दत्त.redraw.x = x;
	दत्त.redraw.y = y;
	दत्त.redraw.विस्तारः = विस्तारः;
	दत्त.redraw.height = height;

	content_प्रसारणम्(c, CONTENT_सन्देशः_REDRAW, &दत्त);
}


/* exported interface, documented in content/content.h */
bool content_exec(संरचना hlcache_handle *h, ध्रुव वर्ण *src, size_t srclen)
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
	bool मेलनम्;

	/* Is this an internal scheme? If so, we trust here and stop */
	यदि ((lwc_string_isequal(scheme, corestring_lwc_about,
				&मेलनम्) == lwc_error_ok &&
	     (मेलनम् == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_दत्त,
				&मेलनम्) == lwc_error_ok &&
	     (मेलनम् == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&मेलनम्) == lwc_error_ok &&
	     (मेलनम् == true)) ||
	    /* Our internal x-ns-css scheme is secure */
	    (lwc_string_isequal(scheme, corestring_lwc_x_ns_css,
				&मेलनम्) == lwc_error_ok &&
	     (मेलनम् == true)) ||
	    /* We also treat file: as "not insecure" here */
	    (lwc_string_isequal(scheme, corestring_lwc_file,
				&मेलनम्) == lwc_error_ok &&
	     (मेलनम् == true))) {
		/* No insecurity to find */
		lwc_string_unref(scheme);
		प्रत्यागम false;
	}

	/* Okay, not internal, am *I* secure? */
	यदि ((lwc_string_isequal(scheme, corestring_lwc_https,
				&मेलनम्) == lwc_error_ok)
	    && (मेलनम् == false)) {
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
	       संरचना content_redraw_दत्त *दत्त,
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

	प्रत्यागम c->handler->redraw(c, दत्त, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(संरचना hlcache_handle *h,
		      पूर्णाङ्क विस्तारः, पूर्णाङ्क height,
		      ध्रुव संरचना redraw_context *ctx)
{
	संरचना content *c = hlcache_handle_get_content(h);
	संरचना redraw_context new_ctx = *ctx;
	संरचना rect clip;
	संरचना content_redraw_दत्त दत्त;
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

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, विस्तारः, height, ctx);

	यदि (ctx->plot->option_knockout) {
		knockout_plot_start(ctx, &new_ctx);
	}

	/* Set clip rectangle to required thumbnail size */
	clip.x0 = 0;
	clip.y0 = 0;
	clip.x1 = विस्तारः;
	clip.y1 = height;

	new_ctx.plot->clip(&new_ctx, &clip);

	/* Plot white background */
	plot_ok &= (new_ctx.plot->rectangle(&new_ctx,
					    plot_style_fill_white,
					    &clip) == NSERROR_OK);

	/* Set up content redraw data */
	दत्त.x = 0;
	दत्त.y = 0;
	दत्त.विस्तारः = विस्तारः;
	दत्त.height = height;

	दत्त.background_रङ्गः = 0xFFFFFF;
	दत्त.repeat_x = false;
	दत्त.repeat_y = false;

	/* Find the scale factor to use if the content has a width */
	यदि (c->विस्तारः) {
		दत्त.scale = (भिन्न)विस्तारः / (भिन्न)c->विस्तारः;
	} अन्यथा {
		दत्त.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &दत्त, &clip, &new_ctx);

	यदि (ctx->plot->option_knockout) {
		knockout_plot_अन्तः(ctx);
	}

	प्रत्यागम plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_सङ्कलय_user(संरचना content *c,
		 शून्य (*callback)(
				  संरचना content *c,
				  content_सन्देशः सन्देशः,
				  ध्रुव सङ्घ content_सन्देशः_दत्त *दत्त,
				  शून्य *pw),
		 शून्य *pw)
{
	संरचना content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = स्मृतिं_देहि(परिमाण(संरचना content_user));
	यदि (!user)
		प्रत्यागम false;
	user->callback = callback;
	user->pw = pw;
	user->अग्र = c->user_list->अग्र;
	c->user_list->अग्र = user;

	यदि (c->handler->सङ्कलय_user != रिक्त)
		c->handler->सङ्कलय_user(c);

	प्रत्यागम true;
}


/* exported interface documented in content/content.h */
शून्य
content_remove_user(संरचना content *c,
		    शून्य (*callback)(
				     संरचना content *c,
				     content_सन्देशः सन्देशः,
				     ध्रुव सङ्घ content_सन्देशः_दत्त *दत्त,
				     शून्य *pw),
		    शून्य *pw)
{
	संरचना content_user *user, *अग्र;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	प्रति (user = c->user_list; user->अग्र != 0 &&
		     !(user->अग्र->callback == callback &&
		       user->अग्र->pw == pw); user = user->अग्र)
		;
	यदि (user->अग्र == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		प्रतिज्ञा(0);
		प्रत्यागम;
	}

	यदि (c->handler->remove_user != रिक्त)
		c->handler->remove_user(c);

	अग्र = user->अग्र;
	user->अग्र = अग्र->अग्र;
	मुक्त(अग्र);
}


/* exported interface documented in content/content.h */
uint32_t content_गणनम्_users(संरचना content *c)
{
	संरचना content_user *user;
	uint32_t गणक = 0;

	प्रतिज्ञा(c != रिक्त);

	प्रति (user = c->user_list; user != रिक्त; user = user->अग्र)
		गणक += 1;

	प्रतिज्ञा(गणक > 0);

	प्रत्यागम गणक - 1; /* Subtract 1 for the sentinel */
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
शून्य content_प्रसारणम्(संरचना content *c, content_सन्देशः सन्देशः,
		       ध्रुव सङ्घ content_सन्देशः_दत्त *दत्त)
{
	संरचना content_user *user, *अग्र;
	प्रतिज्ञा(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, सन्देशः);
	प्रति (user = c->user_list->अग्र; user != 0; user = अग्र) {
		अग्र = user->अग्र;  /* user may be destroyed during callback */
		यदि (user->callback != 0)
			user->callback(c, सन्देशः, दत्त, user->pw);
	}
}


/* exported interface documented in content_protected.h */
शून्य
content_प्रसारणम्_error(संरचना content *c, nserror errorcode, ध्रुव वर्ण *सन्देशः)
{
	संरचना content_user *user, *अग्र;
	सङ्घ content_सन्देशः_दत्त दत्त;

	प्रतिज्ञा(c);

	दत्त.errordata.errorcode = errorcode;
	दत्त.errordata.errormsg = सन्देशः;

	प्रति (user = c->user_list->अग्र; user != 0; user = अग्र) {
		अग्र = user->अग्र;  /* user may be destroyed during callback */
		यदि (user->callback != 0) {
			user->callback(c, CONTENT_सन्देशः_ERROR,
				       &दत्त, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     संरचना browser_गवाक्षः *bw,
	     संरचना content *page,
	     संरचना object_params *params)
{
	संरचना content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	यदि (c->handler->उद्घाटय != रिक्त) {
		res = c->handler->उद्घाटय(c, bw, page, params);
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

	यदि ((c->स्थितिः != CONTENT_स्थितिः_READY) &&
	    (c->स्थितिः != CONTENT_स्थितिः_समाप्तम्)) {
		/* status is not read or done so nothing to do */
		प्रत्यागम NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	यदि (c->textsearch.context != रिक्त) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = रिक्त;
	}

	यदि (c->handler->close != रिक्त) {
		res = c->handler->close(c);
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
			       संरचना browser_गवाक्षः_features *दत्त)
{
	संरचना content *c = hlcache_handle_get_content(h);
	प्रतिज्ञा(c != 0);

	यदि (c->handler->get_contextual_content != रिक्त) {
		प्रत्यागम c->handler->get_contextual_content(c, x, y, दत्त);
	}

	दत्त->object = h;
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
content_debug_dump(संरचना hlcache_handle *h, FILE *f, गणना content_debug op)
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
	bool rel_मेलनम् = false;

	यावत् (link != रिक्त) {
		यदि (lwc_string_caseless_isequal(link->rel, rel,
						&rel_मेलनम्) == lwc_error_ok && rel_मेलनम्) {
			भङ्ग;
		}
		link = link->अग्र;
	}
	प्रत्यागम link;
}


/* exported interface documented in content/protected.h */
संरचना content_rfc5988_link *
content__free_rfc5988_link(संरचना content_rfc5988_link *link)
{
	संरचना content_rfc5988_link *अग्र;

	अग्र = link->अग्र;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	मुक्त(link);

	प्रत्यागम अग्र;
}


/* exported interface documented in content/protected.h */
bool
content_सङ्कलय_rfc5988_link(संरचना content *c,
			  ध्रुव संरचना content_rfc5988_link *link)
{
	संरचना content_rfc5988_link *newlink;
	सङ्घ content_सन्देशः_दत्त सन्देशः_दत्त;

	/* a link relation must be present for it to be a link */
	यदि (link->rel == रिक्त) {
		प्रत्यागम false;
	}

	/* a link href must be present for it to be a link */
	यदि (link->href == रिक्त) {
		प्रत्यागम false;
	}

	newlink = शून्यस्मृतिं_देहि(1, परिमाण(संरचना content_rfc5988_link));
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
	newlink->अग्र = c->links;
	c->links = newlink;

	/* broadcast the data */
	सन्देशः_दत्त.rfc5988_link = newlink;
	content_प्रसारणम्(c, CONTENT_सन्देशः_LINK, &सन्देशः_दत्त);

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
bool content_समुच्चयः_title(संरचना content *c, ध्रुव वर्ण *title)
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
content_स्थितिः content_get_स्थितिः(hlcache_handle *h)
{
	प्रत्यागम content_get_स्थितिः_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_स्थितिः content_get_स्थितिः_2(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम CONTENT_स्थितिः_ERROR;

	प्रत्यागम c->स्थितिः;
}


/* exported interface documented in content/content.h */
ध्रुव वर्ण *content_get_स्थितिः_सन्देशः(hlcache_handle *h)
{
	प्रत्यागम content_get_स्थितिः_सन्देशः_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ध्रुव वर्ण *content_get_स्थितिः_सन्देशः_2(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम रिक्त;

	प्रत्यागम c->स्थितिः_सन्देशः;
}


/* exported interface documented in content/content.h */
पूर्णाङ्क content_get_विस्तारः(hlcache_handle *h)
{
	प्रत्यागम content_get_विस्तारः_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
पूर्णाङ्क content_get_विस्तारः_2(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम 0;

	प्रत्यागम c->विस्तारः;
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
पूर्णाङ्क content_get_उपलब्धम्_विस्तारः(hlcache_handle *h)
{
	प्रत्यागम content_get_उपलब्धम्_विस्तारः_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
पूर्णाङ्क content_get_उपलब्धम्_विस्तारः_2(संरचना content *c)
{
	यदि (c == रिक्त)
		प्रत्यागम 0;

	प्रत्यागम c->उपलब्धम्_विस्तारः;
}


/* exported interface documented in content/content.h */
ध्रुव uint8_t *content_get_source_दत्त(hlcache_handle *h, size_t *आकार)
{
	प्रत्यागम content_get_source_दत्त_2(hlcache_handle_get_content(h), आकार);
}


/* exported interface documented in content/content_protected.h */
ध्रुव uint8_t *content_get_source_दत्त_2(संरचना content *c, size_t *आकार)
{
	प्रतिज्ञा(आकार != रिक्त);

	/** \todo check if the content check should be an assert */
	यदि (c == रिक्त)
		प्रत्यागम रिक्त;

	प्रत्यागम llcache_handle_get_source_दत्त(c->llcache, आकार);
}


/* exported interface documented in content/content.h */
शून्य content_invalidate_reuse_दत्त(hlcache_handle *h)
{
	content_invalidate_reuse_दत्त_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
शून्य content_invalidate_reuse_दत्त_2(संरचना content *c)
{
	यदि (c == रिक्त || c->llcache == रिक्त)
		प्रत्यागम;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_दत्त(c->llcache);
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
	    (c->handler->get_आन्तरिकम् != रिक्त) ) {
		bitmap = c->handler->get_आन्तरिकम्(c, रिक्त);
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

	nc->स्थितिः = c->स्थितिः;

	nc->विस्तारः = c->विस्तारः;
	nc->height = c->height;
	nc->उपलब्धम्_विस्तारः = c->उपलब्धम्_विस्तारः;
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

	nc->time = c->time;
	nc->reformat_time = c->reformat_time;
	nc->आकार = c->आकार;

	यदि (c->title != रिक्त) {
		nc->title = strdup(c->title);
		यदि (nc->title == रिक्त) {
			प्रत्यागम NSERROR_NOMEM;
		}
	}

	nc->सक्रिय = c->सक्रिय;

	nc->user_list = शून्यस्मृतिं_देहि(1, परिमाण(संरचना content_user));
	यदि (nc->user_list == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	स्मृतिं_प्रतिलिख(&(nc->स्थितिः_सन्देशः), &(c->स्थितिः_सन्देशः), 120);
	स्मृतिं_प्रतिलिख(&(nc->sub_स्थितिः), &(c->sub_स्थितिः), 80);

	nc->locked = c->locked;
	nc->योग_आकार = c->योग_आकार;
	nc->http_संहिता = c->http_संहिता;

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
