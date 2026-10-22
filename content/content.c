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

#شامل <stdint.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <nsutils/time.h>

#شامل "netsurf/inttypes.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/corestrings.h"
#شامل "netsurf/browser_window.h"
#شامل "netsurf/bitmap.h"
#شامل "netsurf/content.h"
#شامل "desktop/knockout.h"

#شامل "content/content_protected.h"
#شامل "content/textsearch.h"
#شامل "content/content_debug.h"
#شامل "content/hlcache.h"
#شامل "content/urldb.h"

#تعریف_کرو URL_FMT_SPC "%.140s"

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
ساکن خالی content_convert(ساخت content *c)
{
	أكد(c);
	أكد(c->حالت_2 == CONTENT_حالت_LOADING ||
	       c->حالت_2 == CONTENT_حالت_ERROR);

	اگر (c->حالت_2 != CONTENT_حالت_LOADING)
		واپس;

	اگر (c->locked == true)
		واپس;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	اگر (c->handler->ڈیٹا_complete != تہی) {
		c->locked = true;
		اگر (c->handler->ڈیٹا_complete(c) == false) {
			content_سیٹ_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} ورنہ {
		content_سیٹ_ready(c);
		content_سیٹ_مکمل(c);
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
ساکن nserror
content_llcache_callback(llcache_handle *llcache,
			 ثابت llcache_event *event, خالی *pw)
{
	ساخت content *c = pw;
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
	nserror error = NSERROR_OK;

	انتخاب (event->type) {
	حالت LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		توڑو;
	حالت LLCACHE_EVENT_HAD_سرخیاں:
		/* Will never happen: handled in hlcache */
		توڑو;
	حالت LLCACHE_EVENT_HAD_ڈیٹا:
		اگر (c->handler->process_ڈیٹا != تہی) {
			اگر (c->handler->process_ڈیٹا(c,
						     (ثابت حرف *) event->ڈیٹا.ڈیٹا.buf,
						     event->ڈیٹا.ڈیٹا.len) == false) {
				llcache_handle_abort(c->llcache);
				c->حالت_2 = CONTENT_حالت_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		توڑو;
	حالت LLCACHE_EVENT_مکمل:
		{
			size_t ماخذ_سائز;

			(خالی) llcache_handle_get_ماخذ_ڈیٹا(llcache, &ماخذ_سائز);

			content_سیٹ_حالت(c, messages_get("Processing"));
			پیغام_ڈیٹا.explicit_حالت_متن = تہی;
			content_نشر(c, CONTENT_پیغام_حالت, &پیغام_ڈیٹا);

			content_convert(c);
		}
		توڑو;
	حالت LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->حالت_2 = CONTENT_حالت_ERROR;
		پیغام_ڈیٹا.errordata.errorcode = event->ڈیٹا.error.رمز;
		پیغام_ڈیٹا.errordata.errormsg = event->ڈیٹا.error.پیغام;
		content_نشر(c, CONTENT_پیغام_ERROR, &پیغام_ڈیٹا);
		توڑو;
	حالت LLCACHE_EVENT_پیش_رفت:
		content_سیٹ_حالت(c, event->ڈیٹا.پیش_رفت_پیغام);
		پیغام_ڈیٹا.explicit_حالت_متن = تہی;
		content_نشر(c, CONTENT_پیغام_حالت, &پیغام_ڈیٹا);
		توڑو;
	حالت LLCACHE_EVENT_REDIRECT:
		پیغام_ڈیٹا.redirect.from = event->ڈیٹا.redirect.from;
		پیغام_ڈیٹا.redirect.مقصد = event->ڈیٹا.redirect.مقصد;
		content_نشر(c, CONTENT_پیغام_REDIRECT, &پیغام_ڈیٹا);
		توڑو;
	}

	واپس error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
ساکن خالی content_update_حالت(ساخت content *c)
{
	اگر (c->حالت_2 == CONTENT_حالت_LOADING ||
	    c->حالت_2 == CONTENT_حالت_READY) {
		/* Not done yet */
		snprintf(c->حالت_پیغام, حجم (c->حالت_پیغام),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_حالت[0] != '\0' ? ", " : " ",
			 c->sub_حالت);
	} ورنہ {
		snprintf(c->حالت_پیغام, حجم (c->حالت_پیغام),
			 "%s (%.1fs)", messages_get("Done"),
			 (اعشاری) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(ساخت content *c,
	      ثابت content_handler *handler,
	      lwc_string *imime_type,
	      ثابت ساخت http_parameter *params,
	      llcache_handle *llcache,
	      ثابت حرف *fallback_charset,
	      bool quirks)
{
	ساخت content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = صفر_حافظہ_دو(1, حجم(ساخت content_user));
	اگر (user_sentinel == تہی) {
		واپس NSERROR_NOMEM;
	}

	اگر (fallback_charset != تہی) {
		c->fallback_charset = strdup(fallback_charset);
		اگر (c->fallback_charset == تہی) {
			آزاد(user_sentinel);
			واپس NSERROR_NOMEM;
		}
	}

	c->llcache = llcache;
	c->mime_type = lwc_string_ref(imime_type);
	c->handler = handler;
	c->حالت_2 = CONTENT_حالت_LOADING;
	c->چوڑائی = 0;
	c->اونچائی = 0;
	c->دستیاب_چوڑائی = 0;
	c->دستیاب_اونچائی = 0;
	c->quirks = quirks;
	c->refresh = 0;
	nsu_getmonotonic_ms(&c->time);
	c->سائز = 0;
	c->title = تہی;
	c->فعال = 0;
	user_sentinel->callback = تہی;
	user_sentinel->pw = تہی;
	user_sentinel->اگلا = تہی;
	c->user_list = user_sentinel;
	c->sub_حالت[0] = 0;
	c->locked = false;
	c->کل_سائز = 0;
	c->http_رمز = 0;

	c->textsearch.string = تہی;
	c->textsearch.context = تہی;

	content_سیٹ_حالت(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	اگر (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		واپس error;
	}

	واپس NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	ساخت content *c = hlcache_handle_get_content(h);

	اگر (c == تہی)
		واپس false;

	واپس (c->handler->reformat != تہی);
}


/* exported interface documented in content/protected.h */
خالی content_سیٹ_حالت(ساخت content *c, ثابت حرف *حالت_پیغام)
{
	size_t len = لڑی_لمبائی(حالت_پیغام);

	اگر (len >= حجم(c->sub_حالت)) {
		len = حجم(c->sub_حالت) - 1;
	}
	حافظہ_نقل(c->sub_حالت, حالت_پیغام, len);
	c->sub_حالت[len] = '\0';

	content_update_حالت(c);
}


/* exported interface documented in content/protected.h */
خالی content_سیٹ_ready(ساخت content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	أكد(c->locked);
	c->locked = false;

	c->حالت_2 = CONTENT_حالت_READY;
	content_update_حالت(c);
	content_نشر(c, CONTENT_پیغام_READY, تہی);
}


/* exported interface documented in content/protected.h */
خالی content_سیٹ_مکمل(ساخت content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->حالت_2 = CONTENT_حالت_مکمل;
	c->time = now_ms - c->time;
	content_update_حالت(c);
	content_نشر(c, CONTENT_پیغام_مکمل, تہی);
}


/* exported interface documented in content/protected.h */
خالی content_سیٹ_error(ساخت content *c)
{
	c->locked = false;
	c->حالت_2 = CONTENT_حالت_ERROR;
}


/* exported interface documented in content/content.h */
خالی content_reformat(hlcache_handle *h, bool background,
		      صحیح چوڑائی, صحیح اونچائی)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  چوڑائی, اونچائی);
}


/* exported interface documented in content/protected.h */
خالی
content__reformat(ساخت content *c, bool background, صحیح چوڑائی, صحیح اونچائی)
{
	اتحاد content_پیغام_ڈیٹا ڈیٹا;
	أكد(c != 0);
	أكد(c->حالت_2 == CONTENT_حالت_READY ||
	       c->حالت_2 == CONTENT_حالت_مکمل);
	أكد(c->locked == false);

	c->دستیاب_چوڑائی = چوڑائی;
	c->دستیاب_اونچائی = اونچائی;
	اگر (c->handler->reformat != تہی) {

		c->locked = true;
		c->handler->reformat(c, چوڑائی, اونچائی);
		c->locked = false;

		ڈیٹا.background = background;
		content_نشر(c, CONTENT_پیغام_REFORMAT, &ڈیٹا);
	}
}


/* exported interface documented in content/content.h */
خالی content_destroy(ساخت content *c)
{
	ساخت content_rfc5988_link *link;

	أكد(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	أكد(c->locked == false);

	اگر (c->handler->destroy != تہی)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = تہی;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	جب (link != تہی) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	اگر (c->user_list != تہی) {
		آزاد(c->user_list);
	}

	/* free the title */
	اگر (c->title != تہی) {
		آزاد(c->title);
	}

	/* free the fallback characterset */
	اگر (c->fallback_charset != تہی) {
		آزاد(c->fallback_charset);
	}

	آزاد(c);
}


/* exported interface documented in content/content.h */
خالی
content_mouse_track(hlcache_handle *h,
		    ساخت browser_کھڑکی *bw,
		    browser_mouse_state mouse,
		    صحیح x, صحیح y)
{
	ساخت content *c = hlcache_handle_get_content(h);
	أكد(c != تہی);

	اگر (c->handler->mouse_track != تہی) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} ورنہ {
		اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
		پیغام_ڈیٹا.اشارہ_گر = BROWSER_اشارہ_گر_AUTO;
		content_نشر(c, CONTENT_پیغام_اشارہ_گر, &پیغام_ڈیٹا);
	}


	واپس;
}


/* exported interface documented in content/content.h */
خالی
content_mouse_action(hlcache_handle *h,
		     ساخت browser_کھڑکی *bw,
		     browser_mouse_state mouse,
		     صحیح x, صحیح y)
{
	ساخت content *c = hlcache_handle_get_content(h);
	أكد(c != تہی);

	اگر (c->handler->mouse_action != تہی)
		c->handler->mouse_action(c, bw, mouse, x, y);

	واپس;
}


/* exported interface documented in content/content.h */
bool content_keypress(ساخت hlcache_handle *h, uint32_t کلید)
{
	ساخت content *c = hlcache_handle_get_content(h);
	أكد(c != تہی);

	اگر (c->handler->keypress != تہی)
		واپس c->handler->keypress(c, کلید);

	واپس false;
}


/* exported interface documented in content/content.h */
خالی content_request_redraw(ساخت hlcache_handle *h,
			    صحیح x, صحیح y, صحیح چوڑائی, صحیح اونچائی)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, چوڑائی, اونچائی);
}


/* exported interface, documented in content/protected.h */
خالی content__request_redraw(ساخت content *c,
			     صحیح x, صحیح y, صحیح چوڑائی, صحیح اونچائی)
{
	اتحاد content_پیغام_ڈیٹا ڈیٹا;

	اگر (c == تہی)
		واپس;

	ڈیٹا.redraw.x = x;
	ڈیٹا.redraw.y = y;
	ڈیٹا.redraw.چوڑائی = چوڑائی;
	ڈیٹا.redraw.اونچائی = اونچائی;

	content_نشر(c, CONTENT_پیغام_REDRAW, &ڈیٹا);
}


/* exported interface, documented in content/content.h */
bool content_exec(ساخت hlcache_handle *h, ثابت حرف *src, size_t srclen)
{
	ساخت content *c = hlcache_handle_get_content(h);

	أكد(c != تہی);

	اگر (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		واپس false;
	}

	اگر (c->handler->exec == تہی) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		واپس false;
	}

	واپس c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(ساخت hlcache_handle *h)
{
	ساخت content *c = hlcache_handle_get_content(h);
	ساخت nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool مطابقت;

	/* Is this an internal scheme? If so, we trust here and stop */
	اگر ((lwc_string_isequal(scheme, corestring_lwc_about,
				&مطابقت) == lwc_error_ok &&
	     (مطابقت == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_ڈیٹا,
				&مطابقت) == lwc_error_ok &&
	     (مطابقت == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&مطابقت) == lwc_error_ok &&
	     (مطابقت == true)) ||
	    /* Our internal x-ns-css scheme is secure */
	    (lwc_string_isequal(scheme, corestring_lwc_x_ns_css,
				&مطابقت) == lwc_error_ok &&
	     (مطابقت == true)) ||
	    /* We also treat file: as "not insecure" here */
	    (lwc_string_isequal(scheme, corestring_lwc_file,
				&مطابقت) == lwc_error_ok &&
	     (مطابقت == true))) {
		/* No insecurity to find */
		lwc_string_unref(scheme);
		واپس false;
	}

	/* Okay, not internal, am *I* secure? */
	اگر ((lwc_string_isequal(scheme, corestring_lwc_https,
				&مطابقت) == lwc_error_ok)
	    && (مطابقت == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		واپس true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	اگر (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		واپس true;
	}

	/* Otherwise try and chain through the handler */
	اگر (c != تہی && c->handler->saw_insecure_objects != تہی) {
		واپس c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	واپس false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       ساخت content_redraw_ڈیٹا *ڈیٹا,
	       ثابت ساخت rect *clip,
	       ثابت ساخت redraw_context *ctx)
{
	ساخت content *c = hlcache_handle_get_content(h);

	أكد(c != تہی);

	اگر (c->locked) {
		/* not safe to attempt redraw */
		واپس true;
	}

	/* ensure we have a redrawable content */
	اگر (c->handler->redraw == تہی) {
		واپس true;
	}

	واپس c->handler->redraw(c, ڈیٹا, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(ساخت hlcache_handle *h,
		      صحیح چوڑائی, صحیح اونچائی,
		      ثابت ساخت redraw_context *ctx)
{
	ساخت content *c = hlcache_handle_get_content(h);
	ساخت redraw_context new_ctx = *ctx;
	ساخت rect clip;
	ساخت content_redraw_ڈیٹا ڈیٹا;
	bool plot_ok = true;

	أكد(c != تہی);

	/* ensure it is safe to attempt redraw */
	اگر (c->locked) {
		واپس true;
	}

	/* ensure we have a redrawable content */
	اگر (c->handler->redraw == تہی) {
		واپس true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, چوڑائی, اونچائی, ctx);

	اگر (ctx->plot->option_knockout) {
		knockout_plot_آغاز(ctx, &new_ctx);
	}

	/* Set clip rectangle to required thumbnail size */
	clip.x0 = 0;
	clip.y0 = 0;
	clip.x1 = چوڑائی;
	clip.y1 = اونچائی;

	new_ctx.plot->clip(&new_ctx, &clip);

	/* Plot white background */
	plot_ok &= (new_ctx.plot->rectangle(&new_ctx,
					    plot_style_fill_white,
					    &clip) == NSERROR_OK);

	/* Set up content redraw data */
	ڈیٹا.x = 0;
	ڈیٹا.y = 0;
	ڈیٹا.چوڑائی = چوڑائی;
	ڈیٹا.اونچائی = اونچائی;

	ڈیٹا.background_رنگ = 0xFFFFFF;
	ڈیٹا.repeat_x = false;
	ڈیٹا.repeat_y = false;

	/* Find the scale factor to use if the content has a width */
	اگر (c->چوڑائی) {
		ڈیٹا.scale = (اعشاری)چوڑائی / (اعشاری)c->چوڑائی;
	} ورنہ {
		ڈیٹا.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &ڈیٹا, &clip, &new_ctx);

	اگر (ctx->plot->option_knockout) {
		knockout_plot_انتہا(ctx);
	}

	واپس plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_جوڑو_user(ساخت content *c,
		 خالی (*callback)(
				  ساخت content *c,
				  content_پیغام پیغام,
				  ثابت اتحاد content_پیغام_ڈیٹا *ڈیٹا,
				  خالی *pw),
		 خالی *pw)
{
	ساخت content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = حافظہ_دو(حجم(ساخت content_user));
	اگر (!user)
		واپس false;
	user->callback = callback;
	user->pw = pw;
	user->اگلا = c->user_list->اگلا;
	c->user_list->اگلا = user;

	اگر (c->handler->جوڑو_user != تہی)
		c->handler->جوڑو_user(c);

	واپس true;
}


/* exported interface documented in content/content.h */
خالی
content_remove_user(ساخت content *c,
		    خالی (*callback)(
				     ساخت content *c,
				     content_پیغام پیغام,
				     ثابت اتحاد content_پیغام_ڈیٹا *ڈیٹا,
				     خالی *pw),
		    خالی *pw)
{
	ساخت content_user *user, *اگلا;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	برائے (user = c->user_list; user->اگلا != 0 &&
		     !(user->اگلا->callback == callback &&
		       user->اگلا->pw == pw); user = user->اگلا)
		;
	اگر (user->اگلا == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		أكد(0);
		واپس;
	}

	اگر (c->handler->remove_user != تہی)
		c->handler->remove_user(c);

	اگلا = user->اگلا;
	user->اگلا = اگلا->اگلا;
	آزاد(اگلا);
}


/* exported interface documented in content/content.h */
uint32_t content_گنتی_users(ساخت content *c)
{
	ساخت content_user *user;
	uint32_t شمار_2 = 0;

	أكد(c != تہی);

	برائے (user = c->user_list; user != تہی; user = user->اگلا)
		شمار_2 += 1;

	أكد(شمار_2 > 0);

	واپس شمار_2 - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(ساخت content *c, bool quirks)
{
	اگر (c->handler->matches_quirks == تہی)
		واپس true;

	واپس c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(ساخت content *c)
{
	واپس c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
خالی content_نشر(ساخت content *c, content_پیغام پیغام,
		       ثابت اتحاد content_پیغام_ڈیٹا *ڈیٹا)
{
	ساخت content_user *user, *اگلا;
	أكد(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, پیغام);
	برائے (user = c->user_list->اگلا; user != 0; user = اگلا) {
		اگلا = user->اگلا;  /* user may be destroyed during callback */
		اگر (user->callback != 0)
			user->callback(c, پیغام, ڈیٹا, user->pw);
	}
}


/* exported interface documented in content_protected.h */
خالی
content_نشر_error(ساخت content *c, nserror errorcode, ثابت حرف *پیغام)
{
	ساخت content_user *user, *اگلا;
	اتحاد content_پیغام_ڈیٹا ڈیٹا;

	أكد(c);

	ڈیٹا.errordata.errorcode = errorcode;
	ڈیٹا.errordata.errormsg = پیغام;

	برائے (user = c->user_list->اگلا; user != 0; user = اگلا) {
		اگلا = user->اگلا;  /* user may be destroyed during callback */
		اگر (user->callback != 0) {
			user->callback(c, CONTENT_پیغام_ERROR,
				       &ڈیٹا, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     ساخت browser_کھڑکی *bw,
	     ساخت content *صفحہ,
	     ساخت object_params *params)
{
	ساخت content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	أكد(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	اگر (c->handler->open != تہی) {
		res = c->handler->open(c, bw, صفحہ, params);
	} ورنہ {
		res = NSERROR_OK;
	}
	واپس res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	ساخت content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	اگر (c == تہی) {
		واپس NSERROR_BAD_PARAMETER;
	}

	اگر ((c->حالت_2 != CONTENT_حالت_READY) &&
	    (c->حالت_2 != CONTENT_حالت_مکمل)) {
		/* status is not read or done so nothing to do */
		واپس NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	اگر (c->textsearch.context != تہی) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = تہی;
	}

	اگر (c->handler->close != تہی) {
		res = c->handler->close(c);
	} ورنہ {
		res = NSERROR_OK;
	}
	واپس res;
}


/* exported interface, documented in content/content.h */
خالی content_clear_selection(hlcache_handle *h)
{
	ساخت content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	اگر (c->handler->get_selection != تہی)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
حرف * content_get_selection(hlcache_handle *h)
{
	ساخت content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	اگر (c->handler->get_selection != تہی)
		واپس c->handler->get_selection(c);
	ورنہ
		واپس تہی;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(ساخت hlcache_handle *h,
			       صحیح x, صحیح y,
			       ساخت browser_کھڑکی_features *ڈیٹا)
{
	ساخت content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	اگر (c->handler->get_contextual_content != تہی) {
		واپس c->handler->get_contextual_content(c, x, y, ڈیٹا);
	}

	ڈیٹا->object = h;
	واپس NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_نقطہ(ساخت hlcache_handle *h,
			صحیح x, صحیح y,
			صحیح scrx, صحیح scry)
{
	ساخت content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	اگر (c->handler->scroll_at_نقطہ != تہی)
		واپس c->handler->scroll_at_نقطہ(c, x, y, scrx, scry);

	واپس false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_نقطہ(ساخت hlcache_handle *h,
			   صحیح x, صحیح y,
			   حرف *file)
{
	ساخت content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	اگر (c->handler->drop_file_at_نقطہ != تہی)
		واپس c->handler->drop_file_at_نقطہ(c, x, y, file);

	واپس false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(ساخت hlcache_handle *h, FILE *f, شمار content_debug op)
{
	ساخت content *c = hlcache_handle_get_content(h);
	أكد(c != 0);

	اگر (c->handler->debug_dump == تہی) {
		واپس NSERROR_NOT_IMPLEMENTED;
	}

	واپس c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(ساخت hlcache_handle *h, شمار content_debug op)
{
	ساخت content *c = hlcache_handle_get_content(h);

	اگر (c == تہی) {
		واپس NSERROR_BAD_PARAMETER;
	}

	اگر (c->handler->debug == تہی) {
		واپس NSERROR_NOT_IMPLEMENTED;
	}

	واپس c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
ساخت content_rfc5988_link *
content_ڈھونڈو_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	ساخت content *c = hlcache_handle_get_content(h);
	ساخت content_rfc5988_link *link = c->links;
	bool rel_مطابقت = false;

	جب (link != تہی) {
		اگر (lwc_string_caseless_isequal(link->rel, rel,
						&rel_مطابقت) == lwc_error_ok && rel_مطابقت) {
			توڑو;
		}
		link = link->اگلا;
	}
	واپس link;
}


/* exported interface documented in content/protected.h */
ساخت content_rfc5988_link *
content__free_rfc5988_link(ساخت content_rfc5988_link *link)
{
	ساخت content_rfc5988_link *اگلا;

	اگلا = link->اگلا;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	آزاد(link);

	واپس اگلا;
}


/* exported interface documented in content/protected.h */
bool
content_جوڑو_rfc5988_link(ساخت content *c,
			  ثابت ساخت content_rfc5988_link *link)
{
	ساخت content_rfc5988_link *newlink;
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;

	/* a link relation must be present for it to be a link */
	اگر (link->rel == تہی) {
		واپس false;
	}

	/* a link href must be present for it to be a link */
	اگر (link->href == تہی) {
		واپس false;
	}

	newlink = صفر_حافظہ_دو(1, حجم(ساخت content_rfc5988_link));
	اگر (newlink == تہی) {
		واپس false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	اگر (link->hreflang != تہی) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	اگر (link->type != تہی) {
		newlink->type = lwc_string_ref(link->type);
	}
	اگر (link->media != تہی) {
		newlink->media = lwc_string_ref(link->media);
	}
	اگر (link->sizes != تہی) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->اگلا = c->links;
	c->links = newlink;

	/* broadcast the data */
	پیغام_ڈیٹا.rfc5988_link = newlink;
	content_نشر(c, CONTENT_پیغام_LINK, &پیغام_ڈیٹا);

	واپس true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(ساخت content *c)
{
	اگر (c == تہی)
		واپس تہی;

	واپس llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	ساخت content *c = hlcache_handle_get_content(h);

	اگر (c == تہی)
		واپس CONTENT_NONE;

	واپس c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	واپس content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(ساخت content *c)
{
	اگر (c == تہی)
		واپس تہی;

	واپس lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content_سیٹ_title(ساخت content *c, ثابت حرف *title)
{
	حرف *new_title = strdup(title);
	اگر (new_title == تہی)
		واپس false;

	اگر (c->title != تہی)
		آزاد(c->title);

	c->title = new_title;

	واپس true;
}


/* exported interface documented in content/content.h */
ثابت حرف *content_get_title(hlcache_handle *h)
{
	واپس content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ثابت حرف *content__get_title(ساخت content *c)
{
	اگر (c == تہی)
		واپس تہی;

	واپس c->title != تہی ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_حالت content_get_حالت(hlcache_handle *h)
{
	واپس content_get_حالت_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_حالت content_get_حالت_2(ساخت content *c)
{
	اگر (c == تہی)
		واپس CONTENT_حالت_ERROR;

	واپس c->حالت_2;
}


/* exported interface documented in content/content.h */
ثابت حرف *content_get_حالت_پیغام(hlcache_handle *h)
{
	واپس content_get_حالت_پیغام_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ثابت حرف *content_get_حالت_پیغام_2(ساخت content *c)
{
	اگر (c == تہی)
		واپس تہی;

	واپس c->حالت_پیغام;
}


/* exported interface documented in content/content.h */
صحیح content_get_چوڑائی(hlcache_handle *h)
{
	واپس content_get_چوڑائی_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
صحیح content_get_چوڑائی_2(ساخت content *c)
{
	اگر (c == تہی)
		واپس 0;

	واپس c->چوڑائی;
}


/* exported interface documented in content/content.h */
صحیح content_get_اونچائی(hlcache_handle *h)
{
	واپس content_get_اونچائی_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
صحیح content_get_اونچائی_2(ساخت content *c)
{
	اگر (c == تہی)
		واپس 0;

	واپس c->اونچائی;
}


/* exported interface documented in content/content.h */
صحیح content_get_دستیاب_چوڑائی(hlcache_handle *h)
{
	واپس content_get_دستیاب_چوڑائی_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
صحیح content_get_دستیاب_چوڑائی_2(ساخت content *c)
{
	اگر (c == تہی)
		واپس 0;

	واپس c->دستیاب_چوڑائی;
}


/* exported interface documented in content/content.h */
ثابت uint8_t *content_get_ماخذ_ڈیٹا(hlcache_handle *h, size_t *سائز)
{
	واپس content_get_ماخذ_ڈیٹا_2(hlcache_handle_get_content(h), سائز);
}


/* exported interface documented in content/content_protected.h */
ثابت uint8_t *content_get_ماخذ_ڈیٹا_2(ساخت content *c, size_t *سائز)
{
	أكد(سائز != تہی);

	/** \todo check if the content check should be an assert */
	اگر (c == تہی)
		واپس تہی;

	واپس llcache_handle_get_ماخذ_ڈیٹا(c->llcache, سائز);
}


/* exported interface documented in content/content.h */
خالی content_invalidate_reuse_ڈیٹا(hlcache_handle *h)
{
	content_invalidate_reuse_ڈیٹا_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
خالی content_invalidate_reuse_ڈیٹا_2(ساخت content *c)
{
	اگر (c == تہی || c->llcache == تہی)
		واپس;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_ڈیٹا(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	واپس content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(ساخت content *c)
{
	اگر (c == تہی)
		واپس تہی;

	واپس c->refresh;
}


/* exported interface documented in content/content.h */
ساخت bitmap *content_get_bitmap(hlcache_handle *h)
{
	واپس content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ساخت bitmap *content__get_bitmap(ساخت content *c)
{
	ساخت bitmap *bitmap = تہی;

	اگر ((c != تہی) &&
	    (c->handler != تہی) &&
	    (c->handler->type != تہی) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_اندرونی != تہی) ) {
		bitmap = c->handler->get_اندرونی(c, تہی);
	}

	واپس bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	واپس content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(ساخت content *c)
{
	اگر ((c != تہی) &&
	    (c->handler != تہی) &&
	    (c->handler->is_opaque != تہی)) {
		واپس c->handler->is_opaque(c);
	}

	واپس false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	ساخت content *c = hlcache_handle_get_content(h);

	اگر (c == تہی)
		واپس false;

	واپس c->quirks;
}


/* exported interface documented in content/content.h */
ثابت حرف *
content_get_encoding(hlcache_handle *h, شمار content_encoding_type op)
{
	واپس content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
ثابت حرف *
content__get_encoding(ساخت content *c, شمار content_encoding_type op)
{
	ثابت حرف *encoding = تہی;

	اگر ((c != تہی) &&
	    (c->handler != تہی) &&
	    (c->handler->get_encoding != تہی) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	واپس encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	واپس content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(ساخت content *c)
{
	واپس c->locked;
}


/* exported interface documented in content/content.h */
ثابت llcache_handle *content_get_llcache_handle(ساخت content *c)
{
	اگر (c == تہی)
		واپس تہی;

	واپس c->llcache;
}


/* exported interface documented in content/protected.h */
ساخت content *content_clone(ساخت content *c)
{
	ساخت content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	اگر (error != NSERROR_OK)
		واپس تہی;

	واپس nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(ثابت ساخت content *c, ساخت content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	اگر (error != NSERROR_OK) {
		واپس error;
	}

	llcache_handle_change_callback(nc->llcache,
				       content_llcache_callback, nc);

	nc->mime_type = lwc_string_ref(c->mime_type);
	nc->handler = c->handler;

	nc->حالت_2 = c->حالت_2;

	nc->چوڑائی = c->چوڑائی;
	nc->اونچائی = c->اونچائی;
	nc->دستیاب_چوڑائی = c->دستیاب_چوڑائی;
	nc->quirks = c->quirks;

	اگر (c->fallback_charset != تہی) {
		nc->fallback_charset = strdup(c->fallback_charset);
		اگر (nc->fallback_charset == تہی) {
			واپس NSERROR_NOMEM;
		}
	}

	اگر (c->refresh != تہی) {
		nc->refresh = nsurl_ref(c->refresh);
		اگر (nc->refresh == تہی) {
			واپس NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_وقت = c->reformat_وقت;
	nc->سائز = c->سائز;

	اگر (c->title != تہی) {
		nc->title = strdup(c->title);
		اگر (nc->title == تہی) {
			واپس NSERROR_NOMEM;
		}
	}

	nc->فعال = c->فعال;

	nc->user_list = صفر_حافظہ_دو(1, حجم(ساخت content_user));
	اگر (nc->user_list == تہی) {
		واپس NSERROR_NOMEM;
	}

	حافظہ_نقل(&(nc->حالت_پیغام), &(c->حالت_پیغام), 120);
	حافظہ_نقل(&(nc->sub_حالت), &(c->sub_حالت), 80);

	nc->locked = c->locked;
	nc->کل_سائز = c->کل_سائز;
	nc->http_رمز = c->http_رمز;

	واپس NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(ساخت content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	اگر (c->handler->stop != تہی)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	واپس llcache_handle_abort(c->llcache);
}
