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

#وصف_ڪريو URL_FMT_SPC "%.140s"

ثابت حرف * ثابت content_حالت_name[] = {
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
	أكد(c->حالت == CONTENT_حالت_LOADING ||
	       c->حالت == CONTENT_حالت_ERROR);

	إذا (c->حالت != CONTENT_حالت_LOADING)
		أرجع;

	إذا (c->locked == true)
		أرجع;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	إذا (c->handler->معلومات_complete != عدم) {
		c->locked = true;
		إذا (c->handler->معلومات_complete(c) == false) {
			content_سيٽ_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} وإلا {
		content_سيٽ_ready(c);
		content_سيٽ_پورو(c);
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
	اتحاد content_نياپو_معلومات نياپو_معلومات;
	nserror error = NSERROR_OK;

	بدل (event->type) {
	حالة LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		اكسر;
	حالة LLCACHE_EVENT_HAD_سرا:
		/* Will never happen: handled in hlcache */
		اكسر;
	حالة LLCACHE_EVENT_HAD_معلومات:
		إذا (c->handler->process_معلومات != عدم) {
			إذا (c->handler->process_معلومات(c,
						     (ثابت حرف *) event->معلومات.معلومات.buf,
						     event->معلومات.معلومات.len) == false) {
				llcache_handle_abort(c->llcache);
				c->حالت = CONTENT_حالت_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		اكسر;
	حالة LLCACHE_EVENT_پورو:
		{
			size_t ذريعو_سائيز;

			(فراغ) llcache_handle_get_ذريعو_معلومات(llcache, &ذريعو_سائيز);

			content_سيٽ_حالت(c, messages_get("Processing"));
			نياپو_معلومات.explicit_حالت_متن = عدم;
			content_نشر(c, CONTENT_نياپو_حالت, &نياپو_معلومات);

			content_convert(c);
		}
		اكسر;
	حالة LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->حالت = CONTENT_حالت_ERROR;
		نياپو_معلومات.errordata.errorcode = event->معلومات.error.ڪوڊ;
		نياپو_معلومات.errordata.errormsg = event->معلومات.error.نياپو;
		content_نشر(c, CONTENT_نياپو_ERROR, &نياپو_معلومات);
		اكسر;
	حالة LLCACHE_EVENT_ترقي:
		content_سيٽ_حالت(c, event->معلومات.ترقي_نياپو);
		نياپو_معلومات.explicit_حالت_متن = عدم;
		content_نشر(c, CONTENT_نياپو_حالت, &نياپو_معلومات);
		اكسر;
	حالة LLCACHE_EVENT_REDIRECT:
		نياپو_معلومات.redirect.from = event->معلومات.redirect.from;
		نياپو_معلومات.redirect.منزل = event->معلومات.redirect.منزل;
		content_نشر(c, CONTENT_نياپو_REDIRECT, &نياپو_معلومات);
		اكسر;
	}

	أرجع error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
ساكن فراغ content_update_حالت(بنية content *c)
{
	إذا (c->حالت == CONTENT_حالت_LOADING ||
	    c->حالت == CONTENT_حالت_READY) {
		/* Not done yet */
		snprintf(c->حالت_پيغام, حجم (c->حالت_پيغام),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_حالت[0] != '\0' ? ", " : " ",
			 c->sub_حالت);
	} وإلا {
		snprintf(c->حالت_پيغام, حجم (c->حالت_پيغام),
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

	user_sentinel = صفر_يادگيري_ڏيو(1, حجم(بنية content_user));
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
	c->حالت = CONTENT_حالت_LOADING;
	c->ويڪر = 0;
	c->اوچائي = 0;
	c->موجود_ويڪر = 0;
	c->موجود_اوچائي = 0;
	c->quirks = quirks;
	c->refresh = 0;
	nsu_getmonotonic_ms(&c->time);
	c->سائيز = 0;
	c->title = عدم;
	c->سرگرم = 0;
	user_sentinel->callback = عدم;
	user_sentinel->pw = عدم;
	user_sentinel->ايندڙ = عدم;
	c->user_list = user_sentinel;
	c->sub_حالت[0] = 0;
	c->locked = false;
	c->ڪل_سائيز = 0;
	c->http_ڪوڊ = 0;

	c->textsearch.string = عدم;
	c->textsearch.context = عدم;

	content_سيٽ_حالت(c, messages_get("Loading"));

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
فراغ content_سيٽ_حالت(بنية content *c, ثابت حرف *حالت_پيغام)
{
	size_t len = لڙي_ڊيگهه(حالت_پيغام);

	إذا (len >= حجم(c->sub_حالت)) {
		len = حجم(c->sub_حالت) - 1;
	}
	يادگيري_نقل(c->sub_حالت, حالت_پيغام, len);
	c->sub_حالت[len] = '\0';

	content_update_حالت(c);
}


/* exported interface documented in content/protected.h */
فراغ content_سيٽ_ready(بنية content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	أكد(c->locked);
	c->locked = false;

	c->حالت = CONTENT_حالت_READY;
	content_update_حالت(c);
	content_نشر(c, CONTENT_نياپو_READY, عدم);
}


/* exported interface documented in content/protected.h */
فراغ content_سيٽ_پورو(بنية content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->حالت = CONTENT_حالت_پورو;
	c->time = now_ms - c->time;
	content_update_حالت(c);
	content_نشر(c, CONTENT_نياپو_پورو, عدم);
}


/* exported interface documented in content/protected.h */
فراغ content_سيٽ_error(بنية content *c)
{
	c->locked = false;
	c->حالت = CONTENT_حالت_ERROR;
}


/* exported interface documented in content/content.h */
فراغ content_reformat(hlcache_handle *h, bool background,
		      صحيح ويڪر, صحيح اوچائي)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  ويڪر, اوچائي);
}


/* exported interface documented in content/protected.h */
فراغ
content__reformat(بنية content *c, bool background, صحيح ويڪر, صحيح اوچائي)
{
	اتحاد content_نياپو_معلومات معلومات;
	أكد(c != 0);
	أكد(c->حالت == CONTENT_حالت_READY ||
	       c->حالت == CONTENT_حالت_پورو);
	أكد(c->locked == false);

	c->موجود_ويڪر = ويڪر;
	c->موجود_اوچائي = اوچائي;
	إذا (c->handler->reformat != عدم) {

		c->locked = true;
		c->handler->reformat(c, ويڪر, اوچائي);
		c->locked = false;

		معلومات.background = background;
		content_نشر(c, CONTENT_نياپو_REFORMAT, &معلومات);
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
		    بنية browser_دري *bw,
		    browser_mouse_state mouse,
		    صحيح x, صحيح y)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != عدم);

	إذا (c->handler->mouse_track != عدم) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} وإلا {
		اتحاد content_نياپو_معلومات نياپو_معلومات;
		نياپو_معلومات.اشارو = BROWSER_اشارو_AUTO;
		content_نشر(c, CONTENT_نياپو_اشارو, &نياپو_معلومات);
	}


	أرجع;
}


/* exported interface documented in content/content.h */
فراغ
content_mouse_action(hlcache_handle *h,
		     بنية browser_دري *bw,
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
bool content_keypress(بنية hlcache_handle *h, uint32_t ڪنجي)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != عدم);

	إذا (c->handler->keypress != عدم)
		أرجع c->handler->keypress(c, ڪنجي);

	أرجع false;
}


/* exported interface documented in content/content.h */
فراغ content_request_redraw(بنية hlcache_handle *h,
			    صحيح x, صحيح y, صحيح ويڪر, صحيح اوچائي)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, ويڪر, اوچائي);
}


/* exported interface, documented in content/protected.h */
فراغ content__request_redraw(بنية content *c,
			     صحيح x, صحيح y, صحيح ويڪر, صحيح اوچائي)
{
	اتحاد content_نياپو_معلومات معلومات;

	إذا (c == عدم)
		أرجع;

	معلومات.redraw.x = x;
	معلومات.redraw.y = y;
	معلومات.redraw.ويڪر = ويڪر;
	معلومات.redraw.اوچائي = اوچائي;

	content_نشر(c, CONTENT_نياپو_REDRAW, &معلومات);
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
	bool ميلاپ;

	/* Is this an internal scheme? If so, we trust here and stop */
	إذا ((lwc_string_isequal(scheme, corestring_lwc_about,
				&ميلاپ) == lwc_error_ok &&
	     (ميلاپ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_معلومات,
				&ميلاپ) == lwc_error_ok &&
	     (ميلاپ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&ميلاپ) == lwc_error_ok &&
	     (ميلاپ == true)) ||
	    /* Our internal x-ns-css scheme is secure */
	    (lwc_string_isequal(scheme, corestring_lwc_x_ns_css,
				&ميلاپ) == lwc_error_ok &&
	     (ميلاپ == true)) ||
	    /* We also treat file: as "not insecure" here */
	    (lwc_string_isequal(scheme, corestring_lwc_file,
				&ميلاپ) == lwc_error_ok &&
	     (ميلاپ == true))) {
		/* No insecurity to find */
		lwc_string_unref(scheme);
		أرجع false;
	}

	/* Okay, not internal, am *I* secure? */
	إذا ((lwc_string_isequal(scheme, corestring_lwc_https,
				&ميلاپ) == lwc_error_ok)
	    && (ميلاپ == false)) {
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
	       بنية content_redraw_معلومات *معلومات,
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

	أرجع c->handler->redraw(c, معلومات, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(بنية hlcache_handle *h,
		      صحيح ويڪر, صحيح اوچائي,
		      ثابت بنية redraw_context *ctx)
{
	بنية content *c = hlcache_handle_get_content(h);
	بنية redraw_context new_ctx = *ctx;
	بنية rect clip;
	بنية content_redraw_معلومات معلومات;
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

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, ويڪر, اوچائي, ctx);

	إذا (ctx->plot->option_knockout) {
		knockout_plot_شروعات(ctx, &new_ctx);
	}

	/* Set clip rectangle to required thumbnail size */
	clip.x0 = 0;
	clip.y0 = 0;
	clip.x1 = ويڪر;
	clip.y1 = اوچائي;

	new_ctx.plot->clip(&new_ctx, &clip);

	/* Plot white background */
	plot_ok &= (new_ctx.plot->rectangle(&new_ctx,
					    plot_style_fill_white,
					    &clip) == NSERROR_OK);

	/* Set up content redraw data */
	معلومات.x = 0;
	معلومات.y = 0;
	معلومات.ويڪر = ويڪر;
	معلومات.اوچائي = اوچائي;

	معلومات.background_رنگ = 0xFFFFFF;
	معلومات.repeat_x = false;
	معلومات.repeat_y = false;

	/* Find the scale factor to use if the content has a width */
	إذا (c->ويڪر) {
		معلومات.scale = (عائم)ويڪر / (عائم)c->ويڪر;
	} وإلا {
		معلومات.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &معلومات, &clip, &new_ctx);

	إذا (ctx->plot->option_knockout) {
		knockout_plot_پڇاڙي(ctx);
	}

	أرجع plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_جوڙيو_user(بنية content *c,
		 فراغ (*callback)(
				  بنية content *c,
				  content_نياپو نياپو,
				  ثابت اتحاد content_نياپو_معلومات *معلومات,
				  فراغ *pw),
		 فراغ *pw)
{
	بنية content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = يادگيري_ڏيو(حجم(بنية content_user));
	إذا (!user)
		أرجع false;
	user->callback = callback;
	user->pw = pw;
	user->ايندڙ = c->user_list->ايندڙ;
	c->user_list->ايندڙ = user;

	إذا (c->handler->جوڙيو_user != عدم)
		c->handler->جوڙيو_user(c);

	أرجع true;
}


/* exported interface documented in content/content.h */
فراغ
content_remove_user(بنية content *c,
		    فراغ (*callback)(
				     بنية content *c,
				     content_نياپو نياپو,
				     ثابت اتحاد content_نياپو_معلومات *معلومات,
				     فراغ *pw),
		    فراغ *pw)
{
	بنية content_user *user, *ايندڙ;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	لكل (user = c->user_list; user->ايندڙ != 0 &&
		     !(user->ايندڙ->callback == callback &&
		       user->ايندڙ->pw == pw); user = user->ايندڙ)
		;
	إذا (user->ايندڙ == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		أكد(0);
		أرجع;
	}

	إذا (c->handler->remove_user != عدم)
		c->handler->remove_user(c);

	ايندڙ = user->ايندڙ;
	user->ايندڙ = ايندڙ->ايندڙ;
	حرر(ايندڙ);
}


/* exported interface documented in content/content.h */
uint32_t content_ڳڻپ_users(بنية content *c)
{
	بنية content_user *user;
	uint32_t counter = 0;

	أكد(c != عدم);

	لكل (user = c->user_list; user != عدم; user = user->ايندڙ)
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
فراغ content_نشر(بنية content *c, content_نياپو نياپو,
		       ثابت اتحاد content_نياپو_معلومات *معلومات)
{
	بنية content_user *user, *ايندڙ;
	أكد(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, نياپو);
	لكل (user = c->user_list->ايندڙ; user != 0; user = ايندڙ) {
		ايندڙ = user->ايندڙ;  /* user may be destroyed during callback */
		إذا (user->callback != 0)
			user->callback(c, نياپو, معلومات, user->pw);
	}
}


/* exported interface documented in content_protected.h */
فراغ
content_نشر_error(بنية content *c, nserror errorcode, ثابت حرف *نياپو)
{
	بنية content_user *user, *ايندڙ;
	اتحاد content_نياپو_معلومات معلومات;

	أكد(c);

	معلومات.errordata.errorcode = errorcode;
	معلومات.errordata.errormsg = نياپو;

	لكل (user = c->user_list->ايندڙ; user != 0; user = ايندڙ) {
		ايندڙ = user->ايندڙ;  /* user may be destroyed during callback */
		إذا (user->callback != 0) {
			user->callback(c, CONTENT_نياپو_ERROR,
				       &معلومات, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     بنية browser_دري *bw,
	     بنية content *صفحو,
	     بنية object_params *params)
{
	بنية content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	أكد(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	إذا (c->handler->open != عدم) {
		res = c->handler->open(c, bw, صفحو, params);
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

	إذا ((c->حالت != CONTENT_حالت_READY) &&
	    (c->حالت != CONTENT_حالت_پورو)) {
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
			       بنية browser_دري_features *معلومات)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	إذا (c->handler->get_contextual_content != عدم) {
		أرجع c->handler->get_contextual_content(c, x, y, معلومات);
	}

	معلومات->object = h;
	أرجع NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_نقطو(بنية hlcache_handle *h,
			صحيح x, صحيح y,
			صحيح scrx, صحيح scry)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	إذا (c->handler->scroll_at_نقطو != عدم)
		أرجع c->handler->scroll_at_نقطو(c, x, y, scrx, scry);

	أرجع false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_نقطو(بنية hlcache_handle *h,
			   صحيح x, صحيح y,
			   حرف *file)
{
	بنية content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	إذا (c->handler->drop_file_at_نقطو != عدم)
		أرجع c->handler->drop_file_at_نقطو(c, x, y, file);

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
content_ڳوليو_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	بنية content *c = hlcache_handle_get_content(h);
	بنية content_rfc5988_link *link = c->links;
	bool rel_ميلاپ = false;

	طالما (link != عدم) {
		إذا (lwc_string_caseless_isequal(link->rel, rel,
						&rel_ميلاپ) == lwc_error_ok && rel_ميلاپ) {
			اكسر;
		}
		link = link->ايندڙ;
	}
	أرجع link;
}


/* exported interface documented in content/protected.h */
بنية content_rfc5988_link *
content__free_rfc5988_link(بنية content_rfc5988_link *link)
{
	بنية content_rfc5988_link *ايندڙ;

	ايندڙ = link->ايندڙ;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	حرر(link);

	أرجع ايندڙ;
}


/* exported interface documented in content/protected.h */
bool
content_جوڙيو_rfc5988_link(بنية content *c,
			  ثابت بنية content_rfc5988_link *link)
{
	بنية content_rfc5988_link *newlink;
	اتحاد content_نياپو_معلومات نياپو_معلومات;

	/* a link relation must be present for it to be a link */
	إذا (link->rel == عدم) {
		أرجع false;
	}

	/* a link href must be present for it to be a link */
	إذا (link->href == عدم) {
		أرجع false;
	}

	newlink = صفر_يادگيري_ڏيو(1, حجم(بنية content_rfc5988_link));
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
	newlink->ايندڙ = c->links;
	c->links = newlink;

	/* broadcast the data */
	نياپو_معلومات.rfc5988_link = newlink;
	content_نشر(c, CONTENT_نياپو_LINK, &نياپو_معلومات);

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
bool content_سيٽ_title(بنية content *c, ثابت حرف *title)
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
content_حالت content_get_حالت(hlcache_handle *h)
{
	أرجع content_get_حالت_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_حالت content_get_حالت_2(بنية content *c)
{
	إذا (c == عدم)
		أرجع CONTENT_حالت_ERROR;

	أرجع c->حالت;
}


/* exported interface documented in content/content.h */
ثابت حرف *content_get_حالت_پيغام(hlcache_handle *h)
{
	أرجع content_get_حالت_پيغام_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ثابت حرف *content_get_حالت_پيغام_2(بنية content *c)
{
	إذا (c == عدم)
		أرجع عدم;

	أرجع c->حالت_پيغام;
}


/* exported interface documented in content/content.h */
صحيح content_get_ويڪر(hlcache_handle *h)
{
	أرجع content_get_ويڪر_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
صحيح content_get_ويڪر_2(بنية content *c)
{
	إذا (c == عدم)
		أرجع 0;

	أرجع c->ويڪر;
}


/* exported interface documented in content/content.h */
صحيح content_get_اوچائي(hlcache_handle *h)
{
	أرجع content_get_اوچائي_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
صحيح content_get_اوچائي_2(بنية content *c)
{
	إذا (c == عدم)
		أرجع 0;

	أرجع c->اوچائي;
}


/* exported interface documented in content/content.h */
صحيح content_get_موجود_ويڪر(hlcache_handle *h)
{
	أرجع content_get_موجود_ويڪر_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
صحيح content_get_موجود_ويڪر_2(بنية content *c)
{
	إذا (c == عدم)
		أرجع 0;

	أرجع c->موجود_ويڪر;
}


/* exported interface documented in content/content.h */
ثابت uint8_t *content_get_ذريعو_معلومات(hlcache_handle *h, size_t *سائيز)
{
	أرجع content_get_ذريعو_معلومات_2(hlcache_handle_get_content(h), سائيز);
}


/* exported interface documented in content/content_protected.h */
ثابت uint8_t *content_get_ذريعو_معلومات_2(بنية content *c, size_t *سائيز)
{
	أكد(سائيز != عدم);

	/** \todo check if the content check should be an assert */
	إذا (c == عدم)
		أرجع عدم;

	أرجع llcache_handle_get_ذريعو_معلومات(c->llcache, سائيز);
}


/* exported interface documented in content/content.h */
فراغ content_invalidate_reuse_معلومات(hlcache_handle *h)
{
	content_invalidate_reuse_معلومات_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
فراغ content_invalidate_reuse_معلومات_2(بنية content *c)
{
	إذا (c == عدم || c->llcache == عدم)
		أرجع;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_معلومات(c->llcache);
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
	    (c->handler->get_اندروني != عدم) ) {
		bitmap = c->handler->get_اندروني(c, عدم);
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

	nc->حالت = c->حالت;

	nc->ويڪر = c->ويڪر;
	nc->اوچائي = c->اوچائي;
	nc->موجود_ويڪر = c->موجود_ويڪر;
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
	nc->reformat_وقت = c->reformat_وقت;
	nc->سائيز = c->سائيز;

	إذا (c->title != عدم) {
		nc->title = strdup(c->title);
		إذا (nc->title == عدم) {
			أرجع NSERROR_NOMEM;
		}
	}

	nc->سرگرم = c->سرگرم;

	nc->user_list = صفر_يادگيري_ڏيو(1, حجم(بنية content_user));
	إذا (nc->user_list == عدم) {
		أرجع NSERROR_NOMEM;
	}

	يادگيري_نقل(&(nc->حالت_پيغام), &(c->حالت_پيغام), 120);
	يادگيري_نقل(&(nc->sub_حالت), &(c->sub_حالت), 80);

	nc->locked = c->locked;
	nc->ڪل_سائيز = c->ڪل_سائيز;
	nc->http_ڪوڊ = c->http_ڪوڊ;

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
