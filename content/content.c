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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nsutils/time.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/browser_window.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/knockout.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/textsearch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/hlcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/urldb.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ URL_FMT_SPC "%.140s"

ਅਚਲ ਅੱਖਰ * ਅਚਲ content_ਸਥਿਤੀ_name[] = {
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
ਸਥਿਰ ਖਾਲੀ content_convert(ਬਣਤਰ content *c)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(c);
	ਪ੍ਰਤਿਜ੍ਞਾ(c->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_LOADING ||
	       c->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ERROR);

	ਜੇ (c->ਸਥਿਤੀ != CONTENT_ਸਥਿਤੀ_LOADING)
		ਵਾਪਸ;

	ਜੇ (c->locked == true)
		ਵਾਪਸ;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	ਜੇ (c->handler->ਅੰਕੜੇ_complete != ਨਿਰਾ) {
		c->locked = true;
		ਜੇ (c->handler->ਅੰਕੜੇ_complete(c) == false) {
			content_ਸੈੱਟ_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} ਨਹੀਂਤਾਂ {
		content_ਸੈੱਟ_ready(c);
		content_ਸੈੱਟ_ਪੂਰਾ(c);
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
ਸਥਿਰ nserror
content_llcache_callback(llcache_handle *llcache,
			 ਅਚਲ llcache_event *event, ਖਾਲੀ *pw)
{
	ਬਣਤਰ content *c = pw;
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	nserror error = NSERROR_OK;

	ਚੋਣ (event->type) {
	ਹਾਲਤ LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		ਰੋਕੋ;
	ਹਾਲਤ LLCACHE_EVENT_HAD_ਸਿਰਲੇਖਾਂ:
		/* Will never happen: handled in hlcache */
		ਰੋਕੋ;
	ਹਾਲਤ LLCACHE_EVENT_HAD_ਅੰਕੜੇ:
		ਜੇ (c->handler->process_ਅੰਕੜੇ != ਨਿਰਾ) {
			ਜੇ (c->handler->process_ਅੰਕੜੇ(c,
						     (ਅਚਲ ਅੱਖਰ *) event->ਅੰਕੜੇ.ਅੰਕੜੇ.buf,
						     event->ਅੰਕੜੇ.ਅੰਕੜੇ.len) == false) {
				llcache_handle_abort(c->llcache);
				c->ਸਥਿਤੀ = CONTENT_ਸਥਿਤੀ_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		ਰੋਕੋ;
	ਹਾਲਤ LLCACHE_EVENT_ਪੂਰਾ:
		{
			size_t ਸਰੋਤ_ਮਾਪ;

			(ਖਾਲੀ) llcache_handle_get_ਸਰੋਤ_ਅੰਕੜੇ(llcache, &ਸਰੋਤ_ਮਾਪ);

			content_ਸੈੱਟ_ਸਥਿਤੀ(c, messages_get("Processing"));
			ਸੁਨੇਹਾ_ਅੰਕੜੇ.explicit_ਸਥਿਤੀ_ਪਾਠ = ਨਿਰਾ;
			content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_ਸਥਿਤੀ, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);

			content_convert(c);
		}
		ਰੋਕੋ;
	ਹਾਲਤ LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->ਸਥਿਤੀ = CONTENT_ਸਥਿਤੀ_ERROR;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.errordata.errorcode = event->ਅੰਕੜੇ.error.ਕੋਡ;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.errordata.errormsg = event->ਅੰਕੜੇ.error.ਸੁਨੇਹਾ;
		content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_ERROR, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
		ਰੋਕੋ;
	ਹਾਲਤ LLCACHE_EVENT_ਤਰੱਕੀ:
		content_ਸੈੱਟ_ਸਥਿਤੀ(c, event->ਅੰਕੜੇ.ਤਰੱਕੀ_ਸੁਨੇਹਾ);
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.explicit_ਸਥਿਤੀ_ਪਾਠ = ਨਿਰਾ;
		content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_ਸਥਿਤੀ, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
		ਰੋਕੋ;
	ਹਾਲਤ LLCACHE_EVENT_REDIRECT:
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.redirect.from = event->ਅੰਕੜੇ.redirect.from;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.redirect.ਮੰਜ਼ਿਲ = event->ਅੰਕੜੇ.redirect.ਮੰਜ਼ਿਲ;
		content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_REDIRECT, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
		ਰੋਕੋ;
	}

	ਵਾਪਸ error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
ਸਥਿਰ ਖਾਲੀ content_update_ਸਥਿਤੀ(ਬਣਤਰ content *c)
{
	ਜੇ (c->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_LOADING ||
	    c->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY) {
		/* Not done yet */
		snprintf(c->ਸਥਿਤੀ_ਸੁਨੇਹਾ, ਆਕਾਰ (c->ਸਥਿਤੀ_ਸੁਨੇਹਾ),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_ਸਥਿਤੀ[0] != '\0' ? ", " : " ",
			 c->sub_ਸਥਿਤੀ);
	} ਨਹੀਂਤਾਂ {
		snprintf(c->ਸਥਿਤੀ_ਸੁਨੇਹਾ, ਆਕਾਰ (c->ਸਥਿਤੀ_ਸੁਨੇਹਾ),
			 "%s (%.1fs)", messages_get("Done"),
			 (ਦਸ਼ਮਲਵ) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(ਬਣਤਰ content *c,
	      ਅਚਲ content_handler *handler,
	      lwc_string *imime_type,
	      ਅਚਲ ਬਣਤਰ http_parameter *params,
	      llcache_handle *llcache,
	      ਅਚਲ ਅੱਖਰ *fallback_charset,
	      bool quirks)
{
	ਬਣਤਰ content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ content_user));
	ਜੇ (user_sentinel == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਜੇ (fallback_charset != ਨਿਰਾ) {
		c->fallback_charset = strdup(fallback_charset);
		ਜੇ (c->fallback_charset == ਨਿਰਾ) {
			ਮੁਕਤ(user_sentinel);
			ਵਾਪਸ NSERROR_NOMEM;
		}
	}

	c->llcache = llcache;
	c->mime_type = lwc_string_ref(imime_type);
	c->handler = handler;
	c->ਸਥਿਤੀ = CONTENT_ਸਥਿਤੀ_LOADING;
	c->ਚੌੜਾਈ = 0;
	c->ਉਚਾਈ = 0;
	c->ਉਪਲਬਧ_ਚੌੜਾਈ = 0;
	c->ਉਪਲਬਧ_ਉਚਾਈ = 0;
	c->quirks = quirks;
	c->refresh = 0;
	nsu_getmonotonic_ms(&c->time);
	c->ਮਾਪ = 0;
	c->title = ਨਿਰਾ;
	c->ਸਰਗਰਮ = 0;
	user_sentinel->callback = ਨਿਰਾ;
	user_sentinel->pw = ਨਿਰਾ;
	user_sentinel->ਅਗਲਾ = ਨਿਰਾ;
	c->user_list = user_sentinel;
	c->sub_ਸਥਿਤੀ[0] = 0;
	c->locked = false;
	c->ਕੁੱਲ_ਮਾਪ = 0;
	c->http_ਕੋਡ = 0;

	c->textsearch.string = ਨਿਰਾ;
	c->textsearch.context = ਨਿਰਾ;

	content_ਸੈੱਟ_ਸਥਿਤੀ(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	ਜੇ (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		ਵਾਪਸ error;
	}

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);

	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ false;

	ਵਾਪਸ (c->handler->reformat != ਨਿਰਾ);
}


/* exported interface documented in content/protected.h */
ਖਾਲੀ content_ਸੈੱਟ_ਸਥਿਤੀ(ਬਣਤਰ content *c, ਅਚਲ ਅੱਖਰ *ਸਥਿਤੀ_ਸੁਨੇਹਾ)
{
	size_t len = ਲੜੀ_ਲੰਬਾਈ(ਸਥਿਤੀ_ਸੁਨੇਹਾ);

	ਜੇ (len >= ਆਕਾਰ(c->sub_ਸਥਿਤੀ)) {
		len = ਆਕਾਰ(c->sub_ਸਥਿਤੀ) - 1;
	}
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(c->sub_ਸਥਿਤੀ, ਸਥਿਤੀ_ਸੁਨੇਹਾ, len);
	c->sub_ਸਥਿਤੀ[len] = '\0';

	content_update_ਸਥਿਤੀ(c);
}


/* exported interface documented in content/protected.h */
ਖਾਲੀ content_ਸੈੱਟ_ready(ਬਣਤਰ content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	ਪ੍ਰਤਿਜ੍ਞਾ(c->locked);
	c->locked = false;

	c->ਸਥਿਤੀ = CONTENT_ਸਥਿਤੀ_READY;
	content_update_ਸਥਿਤੀ(c);
	content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_READY, ਨਿਰਾ);
}


/* exported interface documented in content/protected.h */
ਖਾਲੀ content_ਸੈੱਟ_ਪੂਰਾ(ਬਣਤਰ content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->ਸਥਿਤੀ = CONTENT_ਸਥਿਤੀ_ਪੂਰਾ;
	c->time = now_ms - c->time;
	content_update_ਸਥਿਤੀ(c);
	content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_ਪੂਰਾ, ਨਿਰਾ);
}


/* exported interface documented in content/protected.h */
ਖਾਲੀ content_ਸੈੱਟ_error(ਬਣਤਰ content *c)
{
	c->locked = false;
	c->ਸਥਿਤੀ = CONTENT_ਸਥਿਤੀ_ERROR;
}


/* exported interface documented in content/content.h */
ਖਾਲੀ content_reformat(hlcache_handle *h, bool background,
		      ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  ਚੌੜਾਈ, ਉਚਾਈ);
}


/* exported interface documented in content/protected.h */
ਖਾਲੀ
content__reformat(ਬਣਤਰ content *c, bool background, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ)
{
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਅੰਕੜੇ;
	ਪ੍ਰਤਿਜ੍ਞਾ(c != 0);
	ਪ੍ਰਤਿਜ੍ਞਾ(c->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY ||
	       c->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(c->locked == false);

	c->ਉਪਲਬਧ_ਚੌੜਾਈ = ਚੌੜਾਈ;
	c->ਉਪਲਬਧ_ਉਚਾਈ = ਉਚਾਈ;
	ਜੇ (c->handler->reformat != ਨਿਰਾ) {

		c->locked = true;
		c->handler->reformat(c, ਚੌੜਾਈ, ਉਚਾਈ);
		c->locked = false;

		ਅੰਕੜੇ.background = background;
		content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_REFORMAT, &ਅੰਕੜੇ);
	}
}


/* exported interface documented in content/content.h */
ਖਾਲੀ content_destroy(ਬਣਤਰ content *c)
{
	ਬਣਤਰ content_rfc5988_link *link;

	ਪ੍ਰਤਿਜ੍ਞਾ(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	ਪ੍ਰਤਿਜ੍ਞਾ(c->locked == false);

	ਜੇ (c->handler->destroy != ਨਿਰਾ)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = ਨਿਰਾ;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	ਜਦੋਂ (link != ਨਿਰਾ) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	ਜੇ (c->user_list != ਨਿਰਾ) {
		ਮੁਕਤ(c->user_list);
	}

	/* free the title */
	ਜੇ (c->title != ਨਿਰਾ) {
		ਮੁਕਤ(c->title);
	}

	/* free the fallback characterset */
	ਜੇ (c->fallback_charset != ਨਿਰਾ) {
		ਮੁਕਤ(c->fallback_charset);
	}

	ਮੁਕਤ(c);
}


/* exported interface documented in content/content.h */
ਖਾਲੀ
content_mouse_track(hlcache_handle *h,
		    ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
		    browser_mouse_state mouse,
		    ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਜੇ (c->handler->mouse_track != ਨਿਰਾ) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} ਨਹੀਂਤਾਂ {
		ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.ਸੂਚਕ = BROWSER_ਸੂਚਕ_AUTO;
		content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_ਸੂਚਕ, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
	}


	ਵਾਪਸ;
}


/* exported interface documented in content/content.h */
ਖਾਲੀ
content_mouse_action(hlcache_handle *h,
		     ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
		     browser_mouse_state mouse,
		     ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਜੇ (c->handler->mouse_action != ਨਿਰਾ)
		c->handler->mouse_action(c, bw, mouse, x, y);

	ਵਾਪਸ;
}


/* exported interface documented in content/content.h */
bool content_keypress(ਬਣਤਰ hlcache_handle *h, uint32_t ਕੁੰਜੀ)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਜੇ (c->handler->keypress != ਨਿਰਾ)
		ਵਾਪਸ c->handler->keypress(c, ਕੁੰਜੀ);

	ਵਾਪਸ false;
}


/* exported interface documented in content/content.h */
ਖਾਲੀ content_request_redraw(ਬਣਤਰ hlcache_handle *h,
			    ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, ਚੌੜਾਈ, ਉਚਾਈ);
}


/* exported interface, documented in content/protected.h */
ਖਾਲੀ content__request_redraw(ਬਣਤਰ content *c,
			     ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ)
{
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਅੰਕੜੇ;

	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ;

	ਅੰਕੜੇ.redraw.x = x;
	ਅੰਕੜੇ.redraw.y = y;
	ਅੰਕੜੇ.redraw.ਚੌੜਾਈ = ਚੌੜਾਈ;
	ਅੰਕੜੇ.redraw.ਉਚਾਈ = ਉਚਾਈ;

	content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_REDRAW, &ਅੰਕੜੇ);
}


/* exported interface, documented in content/content.h */
bool content_exec(ਬਣਤਰ hlcache_handle *h, ਅਚਲ ਅੱਖਰ *src, size_t srclen)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਜੇ (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		ਵਾਪਸ false;
	}

	ਜੇ (c->handler->exec == ਨਿਰਾ) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		ਵਾਪਸ false;
	}

	ਵਾਪਸ c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(ਬਣਤਰ hlcache_handle *h)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	ਬਣਤਰ nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool ਮੇਲ;

	/* Is this an internal scheme? If so, we trust here and stop */
	ਜੇ ((lwc_string_isequal(scheme, corestring_lwc_about,
				&ਮੇਲ) == lwc_error_ok &&
	     (ਮੇਲ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_ਅੰਕੜੇ,
				&ਮੇਲ) == lwc_error_ok &&
	     (ਮੇਲ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&ਮੇਲ) == lwc_error_ok &&
	     (ਮੇਲ == true)) ||
	    /* Our internal x-ns-css scheme is secure */
	    (lwc_string_isequal(scheme, corestring_lwc_x_ns_css,
				&ਮੇਲ) == lwc_error_ok &&
	     (ਮੇਲ == true)) ||
	    /* We also treat file: as "not insecure" here */
	    (lwc_string_isequal(scheme, corestring_lwc_file,
				&ਮੇਲ) == lwc_error_ok &&
	     (ਮੇਲ == true))) {
		/* No insecurity to find */
		lwc_string_unref(scheme);
		ਵਾਪਸ false;
	}

	/* Okay, not internal, am *I* secure? */
	ਜੇ ((lwc_string_isequal(scheme, corestring_lwc_https,
				&ਮੇਲ) == lwc_error_ok)
	    && (ਮੇਲ == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		ਵਾਪਸ true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	ਜੇ (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		ਵਾਪਸ true;
	}

	/* Otherwise try and chain through the handler */
	ਜੇ (c != ਨਿਰਾ && c->handler->saw_insecure_objects != ਨਿਰਾ) {
		ਵਾਪਸ c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	ਵਾਪਸ false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       ਬਣਤਰ content_redraw_ਅੰਕੜੇ *ਅੰਕੜੇ,
	       ਅਚਲ ਬਣਤਰ rect *clip,
	       ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਜੇ (c->locked) {
		/* not safe to attempt redraw */
		ਵਾਪਸ true;
	}

	/* ensure we have a redrawable content */
	ਜੇ (c->handler->redraw == ਨਿਰਾ) {
		ਵਾਪਸ true;
	}

	ਵਾਪਸ c->handler->redraw(c, ਅੰਕੜੇ, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(ਬਣਤਰ hlcache_handle *h,
		      ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ,
		      ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	ਬਣਤਰ redraw_context new_ctx = *ctx;
	ਬਣਤਰ rect clip;
	ਬਣਤਰ content_redraw_ਅੰਕੜੇ ਅੰਕੜੇ;
	bool plot_ok = true;

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	/* ensure it is safe to attempt redraw */
	ਜੇ (c->locked) {
		ਵਾਪਸ true;
	}

	/* ensure we have a redrawable content */
	ਜੇ (c->handler->redraw == ਨਿਰਾ) {
		ਵਾਪਸ true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, ਚੌੜਾਈ, ਉਚਾਈ, ctx);

	ਜੇ (ctx->plot->option_knockout) {
		knockout_plot_ਸ਼ੁਰੂ(ctx, &new_ctx);
	}

	/* Set clip rectangle to required thumbnail size */
	clip.x0 = 0;
	clip.y0 = 0;
	clip.x1 = ਚੌੜਾਈ;
	clip.y1 = ਉਚਾਈ;

	new_ctx.plot->clip(&new_ctx, &clip);

	/* Plot white background */
	plot_ok &= (new_ctx.plot->rectangle(&new_ctx,
					    plot_style_fill_white,
					    &clip) == NSERROR_OK);

	/* Set up content redraw data */
	ਅੰਕੜੇ.x = 0;
	ਅੰਕੜੇ.y = 0;
	ਅੰਕੜੇ.ਚੌੜਾਈ = ਚੌੜਾਈ;
	ਅੰਕੜੇ.ਉਚਾਈ = ਉਚਾਈ;

	ਅੰਕੜੇ.background_ਰੰਗ = 0xFFFFFF;
	ਅੰਕੜੇ.repeat_x = false;
	ਅੰਕੜੇ.repeat_y = false;

	/* Find the scale factor to use if the content has a width */
	ਜੇ (c->ਚੌੜਾਈ) {
		ਅੰਕੜੇ.scale = (ਦਸ਼ਮਲਵ)ਚੌੜਾਈ / (ਦਸ਼ਮਲਵ)c->ਚੌੜਾਈ;
	} ਨਹੀਂਤਾਂ {
		ਅੰਕੜੇ.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &ਅੰਕੜੇ, &clip, &new_ctx);

	ਜੇ (ctx->plot->option_knockout) {
		knockout_plot_ਸਿਰਾ(ctx);
	}

	ਵਾਪਸ plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_ਜੋੜੋ_user(ਬਣਤਰ content *c,
		 ਖਾਲੀ (*callback)(
				  ਬਣਤਰ content *c,
				  content_ਸੁਨੇਹਾ ਸੁਨੇਹਾ,
				  ਅਚਲ ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ *ਅੰਕੜੇ,
				  ਖਾਲੀ *pw),
		 ਖਾਲੀ *pw)
{
	ਬਣਤਰ content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ content_user));
	ਜੇ (!user)
		ਵਾਪਸ false;
	user->callback = callback;
	user->pw = pw;
	user->ਅਗਲਾ = c->user_list->ਅਗਲਾ;
	c->user_list->ਅਗਲਾ = user;

	ਜੇ (c->handler->ਜੋੜੋ_user != ਨਿਰਾ)
		c->handler->ਜੋੜੋ_user(c);

	ਵਾਪਸ true;
}


/* exported interface documented in content/content.h */
ਖਾਲੀ
content_remove_user(ਬਣਤਰ content *c,
		    ਖਾਲੀ (*callback)(
				     ਬਣਤਰ content *c,
				     content_ਸੁਨੇਹਾ ਸੁਨੇਹਾ,
				     ਅਚਲ ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ *ਅੰਕੜੇ,
				     ਖਾਲੀ *pw),
		    ਖਾਲੀ *pw)
{
	ਬਣਤਰ content_user *user, *ਅਗਲਾ;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	ਲਈ (user = c->user_list; user->ਅਗਲਾ != 0 &&
		     !(user->ਅਗਲਾ->callback == callback &&
		       user->ਅਗਲਾ->pw == pw); user = user->ਅਗਲਾ)
		;
	ਜੇ (user->ਅਗਲਾ == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		ਪ੍ਰਤਿਜ੍ਞਾ(0);
		ਵਾਪਸ;
	}

	ਜੇ (c->handler->remove_user != ਨਿਰਾ)
		c->handler->remove_user(c);

	ਅਗਲਾ = user->ਅਗਲਾ;
	user->ਅਗਲਾ = ਅਗਲਾ->ਅਗਲਾ;
	ਮੁਕਤ(ਅਗਲਾ);
}


/* exported interface documented in content/content.h */
uint32_t content_ਗਿਣਤੀ_users(ਬਣਤਰ content *c)
{
	ਬਣਤਰ content_user *user;
	uint32_t counter = 0;

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਲਈ (user = c->user_list; user != ਨਿਰਾ; user = user->ਅਗਲਾ)
		counter += 1;

	ਪ੍ਰਤਿਜ੍ਞਾ(counter > 0);

	ਵਾਪਸ counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(ਬਣਤਰ content *c, bool quirks)
{
	ਜੇ (c->handler->matches_quirks == ਨਿਰਾ)
		ਵਾਪਸ true;

	ਵਾਪਸ c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(ਬਣਤਰ content *c)
{
	ਵਾਪਸ c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
ਖਾਲੀ content_ਪ੍ਰਸਾਰਣ(ਬਣਤਰ content *c, content_ਸੁਨੇਹਾ ਸੁਨੇਹਾ,
		       ਅਚਲ ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ *ਅੰਕੜੇ)
{
	ਬਣਤਰ content_user *user, *ਅਗਲਾ;
	ਪ੍ਰਤਿਜ੍ਞਾ(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, ਸੁਨੇਹਾ);
	ਲਈ (user = c->user_list->ਅਗਲਾ; user != 0; user = ਅਗਲਾ) {
		ਅਗਲਾ = user->ਅਗਲਾ;  /* user may be destroyed during callback */
		ਜੇ (user->callback != 0)
			user->callback(c, ਸੁਨੇਹਾ, ਅੰਕੜੇ, user->pw);
	}
}


/* exported interface documented in content_protected.h */
ਖਾਲੀ
content_ਪ੍ਰਸਾਰਣ_error(ਬਣਤਰ content *c, nserror errorcode, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ)
{
	ਬਣਤਰ content_user *user, *ਅਗਲਾ;
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਅੰਕੜੇ;

	ਪ੍ਰਤਿਜ੍ਞਾ(c);

	ਅੰਕੜੇ.errordata.errorcode = errorcode;
	ਅੰਕੜੇ.errordata.errormsg = ਸੁਨੇਹਾ;

	ਲਈ (user = c->user_list->ਅਗਲਾ; user != 0; user = ਅਗਲਾ) {
		ਅਗਲਾ = user->ਅਗਲਾ;  /* user may be destroyed during callback */
		ਜੇ (user->callback != 0) {
			user->callback(c, CONTENT_ਸੁਨੇਹਾ_ERROR,
				       &ਅੰਕੜੇ, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
	     ਬਣਤਰ content *ਪੰਨਾ,
	     ਬਣਤਰ object_params *params)
{
	ਬਣਤਰ content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	ਪ੍ਰਤਿਜ੍ਞਾ(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	ਜੇ (c->handler->open != ਨਿਰਾ) {
		res = c->handler->open(c, bw, ਪੰਨਾ, params);
	} ਨਹੀਂਤਾਂ {
		res = NSERROR_OK;
	}
	ਵਾਪਸ res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	ਬਣਤਰ content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	ਜੇ (c == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_BAD_PARAMETER;
	}

	ਜੇ ((c->ਸਥਿਤੀ != CONTENT_ਸਥਿਤੀ_READY) &&
	    (c->ਸਥਿਤੀ != CONTENT_ਸਥਿਤੀ_ਪੂਰਾ)) {
		/* status is not read or done so nothing to do */
		ਵਾਪਸ NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	ਜੇ (c->textsearch.context != ਨਿਰਾ) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ਨਿਰਾ;
	}

	ਜੇ (c->handler->close != ਨਿਰਾ) {
		res = c->handler->close(c);
	} ਨਹੀਂਤਾਂ {
		res = NSERROR_OK;
	}
	ਵਾਪਸ res;
}


/* exported interface, documented in content/content.h */
ਖਾਲੀ content_clear_selection(hlcache_handle *h)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	ਪ੍ਰਤਿਜ੍ਞਾ(c != 0);

	ਜੇ (c->handler->get_selection != ਨਿਰਾ)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
ਅੱਖਰ * content_get_selection(hlcache_handle *h)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	ਪ੍ਰਤਿਜ੍ਞਾ(c != 0);

	ਜੇ (c->handler->get_selection != ਨਿਰਾ)
		ਵਾਪਸ c->handler->get_selection(c);
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ ਨਿਰਾ;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(ਬਣਤਰ hlcache_handle *h,
			       ਪੂਰਨ x, ਪੂਰਨ y,
			       ਬਣਤਰ browser_ਖਿੜਕੀ_features *ਅੰਕੜੇ)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	ਪ੍ਰਤਿਜ੍ਞਾ(c != 0);

	ਜੇ (c->handler->get_contextual_content != ਨਿਰਾ) {
		ਵਾਪਸ c->handler->get_contextual_content(c, x, y, ਅੰਕੜੇ);
	}

	ਅੰਕੜੇ->object = h;
	ਵਾਪਸ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_ਬਿੰਦੂ(ਬਣਤਰ hlcache_handle *h,
			ਪੂਰਨ x, ਪੂਰਨ y,
			ਪੂਰਨ scrx, ਪੂਰਨ scry)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	ਪ੍ਰਤਿਜ੍ਞਾ(c != 0);

	ਜੇ (c->handler->scroll_at_ਬਿੰਦੂ != ਨਿਰਾ)
		ਵਾਪਸ c->handler->scroll_at_ਬਿੰਦੂ(c, x, y, scrx, scry);

	ਵਾਪਸ false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_ਬਿੰਦੂ(ਬਣਤਰ hlcache_handle *h,
			   ਪੂਰਨ x, ਪੂਰਨ y,
			   ਅੱਖਰ *file)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	ਪ੍ਰਤਿਜ੍ਞਾ(c != 0);

	ਜੇ (c->handler->drop_file_at_ਬਿੰਦੂ != ਨਿਰਾ)
		ਵਾਪਸ c->handler->drop_file_at_ਬਿੰਦੂ(c, x, y, file);

	ਵਾਪਸ false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(ਬਣਤਰ hlcache_handle *h, FILE *f, ਗਿਣਤੀ content_debug op)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	ਪ੍ਰਤਿਜ੍ਞਾ(c != 0);

	ਜੇ (c->handler->debug_dump == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOT_IMPLEMENTED;
	}

	ਵਾਪਸ c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(ਬਣਤਰ hlcache_handle *h, ਗਿਣਤੀ content_debug op)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);

	ਜੇ (c == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_BAD_PARAMETER;
	}

	ਜੇ (c->handler->debug == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOT_IMPLEMENTED;
	}

	ਵਾਪਸ c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
ਬਣਤਰ content_rfc5988_link *
content_ਲੱਭੋ_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);
	ਬਣਤਰ content_rfc5988_link *link = c->links;
	bool rel_ਮੇਲ = false;

	ਜਦੋਂ (link != ਨਿਰਾ) {
		ਜੇ (lwc_string_caseless_isequal(link->rel, rel,
						&rel_ਮੇਲ) == lwc_error_ok && rel_ਮੇਲ) {
			ਰੋਕੋ;
		}
		link = link->ਅਗਲਾ;
	}
	ਵਾਪਸ link;
}


/* exported interface documented in content/protected.h */
ਬਣਤਰ content_rfc5988_link *
content__free_rfc5988_link(ਬਣਤਰ content_rfc5988_link *link)
{
	ਬਣਤਰ content_rfc5988_link *ਅਗਲਾ;

	ਅਗਲਾ = link->ਅਗਲਾ;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	ਮੁਕਤ(link);

	ਵਾਪਸ ਅਗਲਾ;
}


/* exported interface documented in content/protected.h */
bool
content_ਜੋੜੋ_rfc5988_link(ਬਣਤਰ content *c,
			  ਅਚਲ ਬਣਤਰ content_rfc5988_link *link)
{
	ਬਣਤਰ content_rfc5988_link *newlink;
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;

	/* a link relation must be present for it to be a link */
	ਜੇ (link->rel == ਨਿਰਾ) {
		ਵਾਪਸ false;
	}

	/* a link href must be present for it to be a link */
	ਜੇ (link->href == ਨਿਰਾ) {
		ਵਾਪਸ false;
	}

	newlink = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ content_rfc5988_link));
	ਜੇ (newlink == ਨਿਰਾ) {
		ਵਾਪਸ false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	ਜੇ (link->hreflang != ਨਿਰਾ) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	ਜੇ (link->type != ਨਿਰਾ) {
		newlink->type = lwc_string_ref(link->type);
	}
	ਜੇ (link->media != ਨਿਰਾ) {
		newlink->media = lwc_string_ref(link->media);
	}
	ਜੇ (link->sizes != ਨਿਰਾ) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->ਅਗਲਾ = c->links;
	c->links = newlink;

	/* broadcast the data */
	ਸੁਨੇਹਾ_ਅੰਕੜੇ.rfc5988_link = newlink;
	content_ਪ੍ਰਸਾਰਣ(c, CONTENT_ਸੁਨੇਹਾ_LINK, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);

	ਵਾਪਸ true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(ਬਣਤਰ content *c)
{
	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);

	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ CONTENT_NONE;

	ਵਾਪਸ c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	ਵਾਪਸ content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(ਬਣਤਰ content *c)
{
	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content_ਸੈੱਟ_title(ਬਣਤਰ content *c, ਅਚਲ ਅੱਖਰ *title)
{
	ਅੱਖਰ *new_title = strdup(title);
	ਜੇ (new_title == ਨਿਰਾ)
		ਵਾਪਸ false;

	ਜੇ (c->title != ਨਿਰਾ)
		ਮੁਕਤ(c->title);

	c->title = new_title;

	ਵਾਪਸ true;
}


/* exported interface documented in content/content.h */
ਅਚਲ ਅੱਖਰ *content_get_title(hlcache_handle *h)
{
	ਵਾਪਸ content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ਅਚਲ ਅੱਖਰ *content__get_title(ਬਣਤਰ content *c)
{
	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ c->title != ਨਿਰਾ ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_ਸਥਿਤੀ content_get_ਸਥਿਤੀ(hlcache_handle *h)
{
	ਵਾਪਸ content_get_ਸਥਿਤੀ_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_ਸਥਿਤੀ content_get_ਸਥਿਤੀ_2(ਬਣਤਰ content *c)
{
	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ CONTENT_ਸਥਿਤੀ_ERROR;

	ਵਾਪਸ c->ਸਥਿਤੀ;
}


/* exported interface documented in content/content.h */
ਅਚਲ ਅੱਖਰ *content_get_ਸਥਿਤੀ_ਸੁਨੇਹਾ(hlcache_handle *h)
{
	ਵਾਪਸ content_get_ਸਥਿਤੀ_ਸੁਨੇਹਾ_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ਅਚਲ ਅੱਖਰ *content_get_ਸਥਿਤੀ_ਸੁਨੇਹਾ_2(ਬਣਤਰ content *c)
{
	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ c->ਸਥਿਤੀ_ਸੁਨੇਹਾ;
}


/* exported interface documented in content/content.h */
ਪੂਰਨ content_get_ਚੌੜਾਈ(hlcache_handle *h)
{
	ਵਾਪਸ content_get_ਚੌੜਾਈ_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ਪੂਰਨ content_get_ਚੌੜਾਈ_2(ਬਣਤਰ content *c)
{
	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ 0;

	ਵਾਪਸ c->ਚੌੜਾਈ;
}


/* exported interface documented in content/content.h */
ਪੂਰਨ content_get_ਉਚਾਈ(hlcache_handle *h)
{
	ਵਾਪਸ content_get_ਉਚਾਈ_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ਪੂਰਨ content_get_ਉਚਾਈ_2(ਬਣਤਰ content *c)
{
	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ 0;

	ਵਾਪਸ c->ਉਚਾਈ;
}


/* exported interface documented in content/content.h */
ਪੂਰਨ content_get_ਉਪਲਬਧ_ਚੌੜਾਈ(hlcache_handle *h)
{
	ਵਾਪਸ content_get_ਉਪਲਬਧ_ਚੌੜਾਈ_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ਪੂਰਨ content_get_ਉਪਲਬਧ_ਚੌੜਾਈ_2(ਬਣਤਰ content *c)
{
	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ 0;

	ਵਾਪਸ c->ਉਪਲਬਧ_ਚੌੜਾਈ;
}


/* exported interface documented in content/content.h */
ਅਚਲ uint8_t *content_get_ਸਰੋਤ_ਅੰਕੜੇ(hlcache_handle *h, size_t *ਮਾਪ)
{
	ਵਾਪਸ content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(hlcache_handle_get_content(h), ਮਾਪ);
}


/* exported interface documented in content/content_protected.h */
ਅਚਲ uint8_t *content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(ਬਣਤਰ content *c, size_t *ਮਾਪ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(ਮਾਪ != ਨਿਰਾ);

	/** \todo check if the content check should be an assert */
	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ llcache_handle_get_ਸਰੋਤ_ਅੰਕੜੇ(c->llcache, ਮਾਪ);
}


/* exported interface documented in content/content.h */
ਖਾਲੀ content_invalidate_reuse_ਅੰਕੜੇ(hlcache_handle *h)
{
	content_invalidate_reuse_ਅੰਕੜੇ_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ਖਾਲੀ content_invalidate_reuse_ਅੰਕੜੇ_2(ਬਣਤਰ content *c)
{
	ਜੇ (c == ਨਿਰਾ || c->llcache == ਨਿਰਾ)
		ਵਾਪਸ;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_ਅੰਕੜੇ(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	ਵਾਪਸ content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(ਬਣਤਰ content *c)
{
	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ c->refresh;
}


/* exported interface documented in content/content.h */
ਬਣਤਰ bitmap *content_get_bitmap(hlcache_handle *h)
{
	ਵਾਪਸ content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ਬਣਤਰ bitmap *content__get_bitmap(ਬਣਤਰ content *c)
{
	ਬਣਤਰ bitmap *bitmap = ਨਿਰਾ;

	ਜੇ ((c != ਨਿਰਾ) &&
	    (c->handler != ਨਿਰਾ) &&
	    (c->handler->type != ਨਿਰਾ) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_ਅੰਦਰੂਨੀ != ਨਿਰਾ) ) {
		bitmap = c->handler->get_ਅੰਦਰੂਨੀ(c, ਨਿਰਾ);
	}

	ਵਾਪਸ bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	ਵਾਪਸ content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(ਬਣਤਰ content *c)
{
	ਜੇ ((c != ਨਿਰਾ) &&
	    (c->handler != ਨਿਰਾ) &&
	    (c->handler->is_opaque != ਨਿਰਾ)) {
		ਵਾਪਸ c->handler->is_opaque(c);
	}

	ਵਾਪਸ false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	ਬਣਤਰ content *c = hlcache_handle_get_content(h);

	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ false;

	ਵਾਪਸ c->quirks;
}


/* exported interface documented in content/content.h */
ਅਚਲ ਅੱਖਰ *
content_get_encoding(hlcache_handle *h, ਗਿਣਤੀ content_encoding_type op)
{
	ਵਾਪਸ content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
ਅਚਲ ਅੱਖਰ *
content__get_encoding(ਬਣਤਰ content *c, ਗਿਣਤੀ content_encoding_type op)
{
	ਅਚਲ ਅੱਖਰ *encoding = ਨਿਰਾ;

	ਜੇ ((c != ਨਿਰਾ) &&
	    (c->handler != ਨਿਰਾ) &&
	    (c->handler->get_encoding != ਨਿਰਾ) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	ਵਾਪਸ encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	ਵਾਪਸ content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(ਬਣਤਰ content *c)
{
	ਵਾਪਸ c->locked;
}


/* exported interface documented in content/content.h */
ਅਚਲ llcache_handle *content_get_llcache_handle(ਬਣਤਰ content *c)
{
	ਜੇ (c == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ c->llcache;
}


/* exported interface documented in content/protected.h */
ਬਣਤਰ content *content_clone(ਬਣਤਰ content *c)
{
	ਬਣਤਰ content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(ਅਚਲ ਬਣਤਰ content *c, ਬਣਤਰ content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	ਜੇ (error != NSERROR_OK) {
		ਵਾਪਸ error;
	}

	llcache_handle_change_callback(nc->llcache,
				       content_llcache_callback, nc);

	nc->mime_type = lwc_string_ref(c->mime_type);
	nc->handler = c->handler;

	nc->ਸਥਿਤੀ = c->ਸਥਿਤੀ;

	nc->ਚੌੜਾਈ = c->ਚੌੜਾਈ;
	nc->ਉਚਾਈ = c->ਉਚਾਈ;
	nc->ਉਪਲਬਧ_ਚੌੜਾਈ = c->ਉਪਲਬਧ_ਚੌੜਾਈ;
	nc->quirks = c->quirks;

	ਜੇ (c->fallback_charset != ਨਿਰਾ) {
		nc->fallback_charset = strdup(c->fallback_charset);
		ਜੇ (nc->fallback_charset == ਨਿਰਾ) {
			ਵਾਪਸ NSERROR_NOMEM;
		}
	}

	ਜੇ (c->refresh != ਨਿਰਾ) {
		nc->refresh = nsurl_ref(c->refresh);
		ਜੇ (nc->refresh == ਨਿਰਾ) {
			ਵਾਪਸ NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_ਸਮਾਂ = c->reformat_ਸਮਾਂ;
	nc->ਮਾਪ = c->ਮਾਪ;

	ਜੇ (c->title != ਨਿਰਾ) {
		nc->title = strdup(c->title);
		ਜੇ (nc->title == ਨਿਰਾ) {
			ਵਾਪਸ NSERROR_NOMEM;
		}
	}

	nc->ਸਰਗਰਮ = c->ਸਰਗਰਮ;

	nc->user_list = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ content_user));
	ਜੇ (nc->user_list == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(&(nc->ਸਥਿਤੀ_ਸੁਨੇਹਾ), &(c->ਸਥਿਤੀ_ਸੁਨੇਹਾ), 120);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(&(nc->sub_ਸਥਿਤੀ), &(c->sub_ਸਥਿਤੀ), 80);

	nc->locked = c->locked;
	nc->ਕੁੱਲ_ਮਾਪ = c->ਕੁੱਲ_ਮਾਪ;
	nc->http_ਕੋਡ = c->http_ਕੋਡ;

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(ਬਣਤਰ content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	ਜੇ (c->handler->stop != ਨਿਰਾ)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	ਵਾਪਸ llcache_handle_abort(c->llcache);
}
