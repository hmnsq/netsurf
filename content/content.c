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

#ವ್ಯಾಖ್ಯಾನಿಸು URL_FMT_SPC "%.140s"

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ content_ಸ್ಥಿತಿ_name[] = {
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
ಸ್ಥಿರ ಶೂನ್ಯ content_convert(ರಚನೆ content *c)
{
	ಪ್ರತಿಜ್ಞಾ(c);
	ಪ್ರತಿಜ್ಞಾ(c->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_LOADING ||
	       c->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ERROR);

	ಆದರೆ (c->ಸ್ಥಿತಿ_2 != CONTENT_ಸ್ಥಿತಿ_LOADING)
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (c->locked == true)
		ಹಿಂತಿರುಗಿ;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p)",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)), c);

	ಆದರೆ (c->handler->ದತ್ತಾಂಶ_complete != ಖಾಲಿ) {
		c->locked = true;
		ಆದರೆ (c->handler->ದತ್ತಾಂಶ_complete(c) == false) {
			content_ಗಣ_error(c);
		}
		/* Conversion to the READY state will unlock the content */
	} ಇಲ್ಲದಿದ್ದರೆ {
		content_ಗಣ_ready(c);
		content_ಗಣ_ಮುಗಿದ(c);
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
			 ಸ್ಥಿರಾಂಕ llcache_event *event, ಶೂನ್ಯ *pw)
{
	ರಚನೆ content *c = pw;
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
	nserror error = NSERROR_OK;

	ಆಯ್ಕೆ (event->type) {
	ಸ್ಥಿತಿ LLCACHE_EVENT_GOT_CERTS:
		/* Will never happen: handled in hlcache */
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ LLCACHE_EVENT_HAD_ಶೀರ್ಷಿಕೆಗಳು:
		/* Will never happen: handled in hlcache */
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ LLCACHE_EVENT_HAD_ದತ್ತಾಂಶ:
		ಆದರೆ (c->handler->process_ದತ್ತಾಂಶ != ಖಾಲಿ) {
			ಆದರೆ (c->handler->process_ದತ್ತಾಂಶ(c,
						     (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *) event->ದತ್ತಾಂಶ.ದತ್ತಾಂಶ.buf,
						     event->ದತ್ತಾಂಶ.ದತ್ತಾಂಶ.len) == false) {
				llcache_handle_abort(c->llcache);
				c->ಸ್ಥಿತಿ_2 = CONTENT_ಸ್ಥಿತಿ_ERROR;
				/** \todo It's not clear what error this is */
				error = NSERROR_NOMEM;
			}
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ LLCACHE_EVENT_ಮುಗಿದ:
		{
			size_t ಮೂಲ_ಗಾತ್ರ;

			(ಶೂನ್ಯ) llcache_handle_get_ಮೂಲ_ದತ್ತಾಂಶ(llcache, &ಮೂಲ_ಗಾತ್ರ);

			content_ಗಣ_ಸ್ಥಿತಿ(c, messages_get("Processing"));
			ಸಂದೇಶ_ದತ್ತಾಂಶ.explicit_ಸ್ಥಿತಿ_ಪಠ್ಯ = ಖಾಲಿ;
			content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_ಸ್ಥಿತಿ, &ಸಂದೇಶ_ದತ್ತಾಂಶ);

			content_convert(c);
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ LLCACHE_EVENT_ERROR:
		/** \todo Error page? */
		c->ಸ್ಥಿತಿ_2 = CONTENT_ಸ್ಥಿತಿ_ERROR;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.errordata.errorcode = event->ದತ್ತಾಂಶ.error.ಸಂಕೇತ_2;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.errordata.errormsg = event->ದತ್ತಾಂಶ.error.ಸಂದೇಶ;
		content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_ERROR, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ LLCACHE_EVENT_ಪ್ರಗತಿ:
		content_ಗಣ_ಸ್ಥಿತಿ(c, event->ದತ್ತಾಂಶ.ಪ್ರಗತಿ_ಸಂದೇಶ);
		ಸಂದೇಶ_ದತ್ತಾಂಶ.explicit_ಸ್ಥಿತಿ_ಪಠ್ಯ = ಖಾಲಿ;
		content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_ಸ್ಥಿತಿ, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ LLCACHE_EVENT_REDIRECT:
		ಸಂದೇಶ_ದತ್ತಾಂಶ.redirect.from = event->ದತ್ತಾಂಶ.redirect.from;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.redirect.ಗಮ್ಯ = event->ದತ್ತಾಂಶ.redirect.ಗಮ್ಯ;
		content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_REDIRECT, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ error;
}


/**
 * update content status message
 *
 * \param c the content to update.
 */
ಸ್ಥಿರ ಶೂನ್ಯ content_update_ಸ್ಥಿತಿ(ರಚನೆ content *c)
{
	ಆದರೆ (c->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_LOADING ||
	    c->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY) {
		/* Not done yet */
		snprintf(c->ಸ್ಥಿತಿ_ಸಂದೇಶ, ಗಾತ್ರ (c->ಸ್ಥಿತಿ_ಸಂದೇಶ),
			 "%s%s%s", messages_get("Fetching"),
			 c->sub_ಸ್ಥಿತಿ[0] != '\0' ? ", " : " ",
			 c->sub_ಸ್ಥಿತಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		snprintf(c->ಸ್ಥಿತಿ_ಸಂದೇಶ, ಗಾತ್ರ (c->ಸ್ಥಿತಿ_ಸಂದೇಶ),
			 "%s (%.1fs)", messages_get("Done"),
			 (ದಶಮಾಂಶ) c->time / 1000);
	}
}


/* exported interface documented in content/protected.h */
nserror
content__init(ರಚನೆ content *c,
	      ಸ್ಥಿರಾಂಕ content_handler *handler,
	      lwc_string *imime_type,
	      ಸ್ಥಿರಾಂಕ ರಚನೆ http_parameter *params,
	      llcache_handle *llcache,
	      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
	      bool quirks)
{
	ರಚನೆ content_user *user_sentinel;
	nserror error;

	NSLOG(netsurf, INFO, "url "URL_FMT_SPC" -> %p",
	      nsurl_access_log(llcache_handle_get_url(llcache)), c);

	user_sentinel = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ content_user));
	ಆದರೆ (user_sentinel == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಆದರೆ (fallback_charset != ಖಾಲಿ) {
		c->fallback_charset = strdup(fallback_charset);
		ಆದರೆ (c->fallback_charset == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(user_sentinel);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	}

	c->llcache = llcache;
	c->mime_type = lwc_string_ref(imime_type);
	c->handler = handler;
	c->ಸ್ಥಿತಿ_2 = CONTENT_ಸ್ಥಿತಿ_LOADING;
	c->ಅಗಲ = 0;
	c->ಎತ್ತರ = 0;
	c->ಲಭ್ಯ_ಅಗಲ = 0;
	c->ಲಭ್ಯ_ಎತ್ತರ = 0;
	c->quirks = quirks;
	c->refresh = 0;
	nsu_getmonotonic_ms(&c->time);
	c->ಗಾತ್ರ_2 = 0;
	c->title = ಖಾಲಿ;
	c->ಸಕ್ರಿಯ = 0;
	user_sentinel->callback = ಖಾಲಿ;
	user_sentinel->pw = ಖಾಲಿ;
	user_sentinel->ಮುಂದಿನ = ಖಾಲಿ;
	c->user_list = user_sentinel;
	c->sub_ಸ್ಥಿತಿ[0] = 0;
	c->locked = false;
	c->ಒಟ್ಟು_ಗಾತ್ರ = 0;
	c->http_ಸಂಕೇತ = 0;

	c->textsearch.string = ಖಾಲಿ;
	c->textsearch.context = ಖಾಲಿ;

	content_ಗಣ_ಸ್ಥಿತಿ(c, messages_get("Loading"));

	/* Finally, claim low-level cache events */
	error = llcache_handle_change_callback(llcache,
					       content_llcache_callback, c);
	ಆದರೆ (error != NSERROR_OK) {
		lwc_string_unref(c->mime_type);
		ಹಿಂತಿರುಗಿ error;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in content/content.h */
bool content_can_reformat(hlcache_handle *h)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);

	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ false;

	ಹಿಂತಿರುಗಿ (c->handler->reformat != ಖಾಲಿ);
}


/* exported interface documented in content/protected.h */
ಶೂನ್ಯ content_ಗಣ_ಸ್ಥಿತಿ(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿತಿ_ಸಂದೇಶ)
{
	size_t len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಸ್ಥಿತಿ_ಸಂದೇಶ);

	ಆದರೆ (len >= ಗಾತ್ರ(c->sub_ಸ್ಥಿತಿ)) {
		len = ಗಾತ್ರ(c->sub_ಸ್ಥಿತಿ) - 1;
	}
	ಸ್ಮರಣೆ_ನಕಲು(c->sub_ಸ್ಥಿತಿ, ಸ್ಥಿತಿ_ಸಂದೇಶ, len);
	c->sub_ಸ್ಥಿತಿ[len] = '\0';

	content_update_ಸ್ಥಿತಿ(c);
}


/* exported interface documented in content/protected.h */
ಶೂನ್ಯ content_ಗಣ_ready(ರಚನೆ content *c)
{
	/* The content must be locked at this point, as it can only
	 * become READY after conversion. */
	ಪ್ರತಿಜ್ಞಾ(c->locked);
	c->locked = false;

	c->ಸ್ಥಿತಿ_2 = CONTENT_ಸ್ಥಿತಿ_READY;
	content_update_ಸ್ಥಿತಿ(c);
	content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_READY, ಖಾಲಿ);
}


/* exported interface documented in content/protected.h */
ಶೂನ್ಯ content_ಗಣ_ಮುಗಿದ(ರಚನೆ content *c)
{
	uint64_t now_ms;

	nsu_getmonotonic_ms(&now_ms);

	c->ಸ್ಥಿತಿ_2 = CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ;
	c->time = now_ms - c->time;
	content_update_ಸ್ಥಿತಿ(c);
	content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_ಮುಗಿದ, ಖಾಲಿ);
}


/* exported interface documented in content/protected.h */
ಶೂನ್ಯ content_ಗಣ_error(ರಚನೆ content *c)
{
	c->locked = false;
	c->ಸ್ಥಿತಿ_2 = CONTENT_ಸ್ಥಿತಿ_ERROR;
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ content_reformat(hlcache_handle *h, bool background,
		      ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ)
{
	content__reformat(hlcache_handle_get_content(h), background,
			  ಅಗಲ, ಎತ್ತರ);
}


/* exported interface documented in content/protected.h */
ಶೂನ್ಯ
content__reformat(ರಚನೆ content *c, bool background, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ)
{
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ದತ್ತಾಂಶ;
	ಪ್ರತಿಜ್ಞಾ(c != 0);
	ಪ್ರತಿಜ್ಞಾ(c->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY ||
	       c->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ);
	ಪ್ರತಿಜ್ಞಾ(c->locked == false);

	c->ಲಭ್ಯ_ಅಗಲ = ಅಗಲ;
	c->ಲಭ್ಯ_ಎತ್ತರ = ಎತ್ತರ;
	ಆದರೆ (c->handler->reformat != ಖಾಲಿ) {

		c->locked = true;
		c->handler->reformat(c, ಅಗಲ, ಎತ್ತರ);
		c->locked = false;

		ದತ್ತಾಂಶ.background = background;
		content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_REFORMAT, &ದತ್ತಾಂಶ);
	}
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ content_destroy(ರಚನೆ content *c)
{
	ರಚನೆ content_rfc5988_link *link;

	ಪ್ರತಿಜ್ಞಾ(c);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	ಪ್ರತಿಜ್ಞಾ(c->locked == false);

	ಆದರೆ (c->handler->destroy != ಖಾಲಿ)
		c->handler->destroy(c);

	llcache_handle_release(c->llcache);
	c->llcache = ಖಾಲಿ;

	lwc_string_unref(c->mime_type);

	/* release metadata links */
	link = c->links;
	ಆಗ (link != ಖಾಲಿ) {
		link = content__free_rfc5988_link(link);
	}

	/* free the user list */
	ಆದರೆ (c->user_list != ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(c->user_list);
	}

	/* free the title */
	ಆದರೆ (c->title != ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(c->title);
	}

	/* free the fallback characterset */
	ಆದರೆ (c->fallback_charset != ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(c->fallback_charset);
	}

	ಬಿಡುಗಡೆ(c);
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ
content_mouse_track(hlcache_handle *h,
		    ರಚನೆ browser_ಕಿಟಕಿ *bw,
		    browser_mouse_state mouse,
		    ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಆದರೆ (c->handler->mouse_track != ಖಾಲಿ) {
		c->handler->mouse_track(c, bw, mouse, x, y);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
		ಸಂದೇಶ_ದತ್ತಾಂಶ.ಸೂಚಕ = BROWSER_ಸೂಚಕ_AUTO;
		content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_ಸೂಚಕ, &ಸಂದೇಶ_ದತ್ತಾಂಶ);
	}


	ಹಿಂತಿರುಗಿ;
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ
content_mouse_action(hlcache_handle *h,
		     ರಚನೆ browser_ಕಿಟಕಿ *bw,
		     browser_mouse_state mouse,
		     ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಆದರೆ (c->handler->mouse_action != ಖಾಲಿ)
		c->handler->mouse_action(c, bw, mouse, x, y);

	ಹಿಂತಿರುಗಿ;
}


/* exported interface documented in content/content.h */
bool content_keypress(ರಚನೆ hlcache_handle *h, uint32_t ಕೀಲಿ)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಆದರೆ (c->handler->keypress != ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ c->handler->keypress(c, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ false;
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ content_request_redraw(ರಚನೆ hlcache_handle *h,
			    ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ)
{
	content__request_redraw(hlcache_handle_get_content(h),
				x, y, ಅಗಲ, ಎತ್ತರ);
}


/* exported interface, documented in content/protected.h */
ಶೂನ್ಯ content__request_redraw(ರಚನೆ content *c,
			     ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ)
{
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ದತ್ತಾಂಶ;

	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ;

	ದತ್ತಾಂಶ.redraw.x = x;
	ದತ್ತಾಂಶ.redraw.y = y;
	ದತ್ತಾಂಶ.redraw.ಅಗಲ = ಅಗಲ;
	ದತ್ತಾಂಶ.redraw.ಎತ್ತರ = ಎತ್ತರ;

	content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_REDRAW, &ದತ್ತಾಂಶ);
}


/* exported interface, documented in content/content.h */
bool content_exec(ರಚನೆ hlcache_handle *h, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, size_t srclen)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಆದರೆ (c->locked) {
		/* Not safe to do stuff */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, content locked");
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (c->handler->exec == ಖಾಲಿ) {
		/* Can't exec something on this content */
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no exec function");
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ c->handler->exec(c, src, srclen);
}


/* exported interface, documented in content/content.h */
bool content_saw_insecure_objects(ರಚನೆ hlcache_handle *h)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	ರಚನೆ nsurl *url = hlcache_handle_get_url(h);
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool ಹೊಂದಿಕೆ;

	/* Is this an internal scheme? If so, we trust here and stop */
	ಆದರೆ ((lwc_string_isequal(scheme, corestring_lwc_about,
				&ಹೊಂದಿಕೆ) == lwc_error_ok &&
	     (ಹೊಂದಿಕೆ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_ದತ್ತಾಂಶ,
				&ಹೊಂದಿಕೆ) == lwc_error_ok &&
	     (ಹೊಂದಿಕೆ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&ಹೊಂದಿಕೆ) == lwc_error_ok &&
	     (ಹೊಂದಿಕೆ == true)) ||
	    /* Our internal x-ns-css scheme is secure */
	    (lwc_string_isequal(scheme, corestring_lwc_x_ns_css,
				&ಹೊಂದಿಕೆ) == lwc_error_ok &&
	     (ಹೊಂದಿಕೆ == true)) ||
	    /* We also treat file: as "not insecure" here */
	    (lwc_string_isequal(scheme, corestring_lwc_file,
				&ಹೊಂದಿಕೆ) == lwc_error_ok &&
	     (ಹೊಂದಿಕೆ == true))) {
		/* No insecurity to find */
		lwc_string_unref(scheme);
		ಹಿಂತಿರುಗಿ false;
	}

	/* Okay, not internal, am *I* secure? */
	ಆದರೆ ((lwc_string_isequal(scheme, corestring_lwc_https,
				&ಹೊಂದಿಕೆ) == lwc_error_ok)
	    && (ಹೊಂದಿಕೆ == false)) {
		/* I did see something insecure -- ME! */
		lwc_string_unref(scheme);
		ಹಿಂತಿರುಗಿ true;
	}

	lwc_string_unref(scheme);
	/* I am supposed to be secure, but was I overridden */
	ಆದರೆ (urldb_get_cert_permissions(url)) {
		/* I was https:// but I was overridden, that's no good */
		ಹಿಂತಿರುಗಿ true;
	}

	/* Otherwise try and chain through the handler */
	ಆದರೆ (c != ಖಾಲಿ && c->handler->saw_insecure_objects != ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ c->handler->saw_insecure_objects(c);
	}

	/* If we can't see insecure objects, we can't see them */
	ಹಿಂತಿರುಗಿ false;
}


/* exported interface, documented in content/content.h */
bool
content_redraw(hlcache_handle *h,
	       ರಚನೆ content_redraw_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
	       ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip,
	       ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಆದರೆ (c->locked) {
		/* not safe to attempt redraw */
		ಹಿಂತಿರುಗಿ true;
	}

	/* ensure we have a redrawable content */
	ಆದರೆ (c->handler->redraw == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ true;
	}

	ಹಿಂತಿರುಗಿ c->handler->redraw(c, ದತ್ತಾಂಶ, clip, ctx);
}


/* exported interface, documented in content/content.h */
bool
content_scaled_redraw(ರಚನೆ hlcache_handle *h,
		      ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ,
		      ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	ರಚನೆ redraw_context new_ctx = *ctx;
	ರಚನೆ rect clip;
	ರಚನೆ content_redraw_ದತ್ತಾಂಶ ದತ್ತಾಂಶ;
	bool plot_ok = true;

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	/* ensure it is safe to attempt redraw */
	ಆದರೆ (c->locked) {
		ಹಿಂತಿರುಗಿ true;
	}

	/* ensure we have a redrawable content */
	ಆದರೆ (c->handler->redraw == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ true;
	}

	NSLOG(netsurf, INFO, "Content %p %dx%d ctx:%p", c, ಅಗಲ, ಎತ್ತರ, ctx);

	ಆದರೆ (ctx->plot->option_knockout) {
		knockout_plot_ಆರಂಭ(ctx, &new_ctx);
	}

	/* Set clip rectangle to required thumbnail size */
	clip.x0 = 0;
	clip.y0 = 0;
	clip.x1 = ಅಗಲ;
	clip.y1 = ಎತ್ತರ;

	new_ctx.plot->clip(&new_ctx, &clip);

	/* Plot white background */
	plot_ok &= (new_ctx.plot->rectangle(&new_ctx,
					    plot_style_fill_white,
					    &clip) == NSERROR_OK);

	/* Set up content redraw data */
	ದತ್ತಾಂಶ.x = 0;
	ದತ್ತಾಂಶ.y = 0;
	ದತ್ತಾಂಶ.ಅಗಲ = ಅಗಲ;
	ದತ್ತಾಂಶ.ಎತ್ತರ = ಎತ್ತರ;

	ದತ್ತಾಂಶ.background_ಬಣ್ಣ = 0xFFFFFF;
	ದತ್ತಾಂಶ.repeat_x = false;
	ದತ್ತಾಂಶ.repeat_y = false;

	/* Find the scale factor to use if the content has a width */
	ಆದರೆ (c->ಅಗಲ) {
		ದತ್ತಾಂಶ.scale = (ದಶಮಾಂಶ)ಅಗಲ / (ದಶಮಾಂಶ)c->ಅಗಲ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ದತ್ತಾಂಶ.scale = 1.0;
	}

	/* Render the content */
	plot_ok &= c->handler->redraw(c, &ದತ್ತಾಂಶ, &clip, &new_ctx);

	ಆದರೆ (ctx->plot->option_knockout) {
		knockout_plot_ಕೊನೆ(ctx);
	}

	ಹಿಂತಿರುಗಿ plot_ok;
}


/* exported interface documented in content/content.h */
bool
content_ಕೂಡಿಸು_user(ರಚನೆ content *c,
		 ಶೂನ್ಯ (*callback)(
				  ರಚನೆ content *c,
				  content_ಸಂದೇಶ ಸಂದೇಶ,
				  ಸ್ಥಿರಾಂಕ ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
				  ಶೂನ್ಯ *pw),
		 ಶೂನ್ಯ *pw)
{
	ರಚನೆ content_user *user;

	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);
	user = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ content_user));
	ಆದರೆ (!user)
		ಹಿಂತಿರುಗಿ false;
	user->callback = callback;
	user->pw = pw;
	user->ಮುಂದಿನ = c->user_list->ಮುಂದಿನ;
	c->user_list->ಮುಂದಿನ = user;

	ಆದರೆ (c->handler->ಕೂಡಿಸು_user != ಖಾಲಿ)
		c->handler->ಕೂಡಿಸು_user(c);

	ಹಿಂತಿರುಗಿ true;
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ
content_remove_user(ರಚನೆ content *c,
		    ಶೂನ್ಯ (*callback)(
				     ರಚನೆ content *c,
				     content_ಸಂದೇಶ ಸಂದೇಶ,
				     ಸ್ಥಿರಾಂಕ ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
				     ಶೂನ್ಯ *pw),
		    ಶೂನ್ಯ *pw)
{
	ರಚನೆ content_user *user, *ಮುಂದಿನ;
	NSLOG(netsurf, INFO, "content "URL_FMT_SPC" (%p), user %p %p",
	      nsurl_access_log(llcache_handle_get_url(c->llcache)),
	      c, callback, pw);

	/* user_list starts with a sentinel */
	ಗಾಗಿ (user = c->user_list; user->ಮುಂದಿನ != 0 &&
		     !(user->ಮುಂದಿನ->callback == callback &&
		       user->ಮುಂದಿನ->pw == pw); user = user->ಮುಂದಿನ)
		;
	ಆದರೆ (user->ಮುಂದಿನ == 0) {
		NSLOG(netsurf, INFO, "user not found in list");
		ಪ್ರತಿಜ್ಞಾ(0);
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (c->handler->remove_user != ಖಾಲಿ)
		c->handler->remove_user(c);

	ಮುಂದಿನ = user->ಮುಂದಿನ;
	user->ಮುಂದಿನ = ಮುಂದಿನ->ಮುಂದಿನ;
	ಬಿಡುಗಡೆ(ಮುಂದಿನ);
}


/* exported interface documented in content/content.h */
uint32_t content_ಎಣಿಕೆ_users(ರಚನೆ content *c)
{
	ರಚನೆ content_user *user;
	uint32_t counter = 0;

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಗಾಗಿ (user = c->user_list; user != ಖಾಲಿ; user = user->ಮುಂದಿನ)
		counter += 1;

	ಪ್ರತಿಜ್ಞಾ(counter > 0);

	ಹಿಂತಿರುಗಿ counter - 1; /* Subtract 1 for the sentinel */
}


/* exported interface documented in content/content.h */
bool content_matches_quirks(ರಚನೆ content *c, bool quirks)
{
	ಆದರೆ (c->handler->matches_quirks == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ true;

	ಹಿಂತಿರುಗಿ c->handler->matches_quirks(c, quirks);
}


/* exported interface documented in content/content.h */
bool content_is_shareable(ರಚನೆ content *c)
{
	ಹಿಂತಿರುಗಿ c->handler->no_share == false;
}


/* exported interface documented in content/protected.h */
ಶೂನ್ಯ content_ಪ್ರಸಾರ(ರಚನೆ content *c, content_ಸಂದೇಶ ಸಂದೇಶ,
		       ಸ್ಥಿರಾಂಕ ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ)
{
	ರಚನೆ content_user *user, *ಮುಂದಿನ;
	ಪ್ರತಿಜ್ಞಾ(c);

	NSLOG(netsurf, DEEPDEBUG, "%p -> msg:%d", c, ಸಂದೇಶ);
	ಗಾಗಿ (user = c->user_list->ಮುಂದಿನ; user != 0; user = ಮುಂದಿನ) {
		ಮುಂದಿನ = user->ಮುಂದಿನ;  /* user may be destroyed during callback */
		ಆದರೆ (user->callback != 0)
			user->callback(c, ಸಂದೇಶ, ದತ್ತಾಂಶ, user->pw);
	}
}


/* exported interface documented in content_protected.h */
ಶೂನ್ಯ
content_ಪ್ರಸಾರ_error(ರಚನೆ content *c, nserror errorcode, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ)
{
	ರಚನೆ content_user *user, *ಮುಂದಿನ;
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ದತ್ತಾಂಶ;

	ಪ್ರತಿಜ್ಞಾ(c);

	ದತ್ತಾಂಶ.errordata.errorcode = errorcode;
	ದತ್ತಾಂಶ.errordata.errormsg = ಸಂದೇಶ;

	ಗಾಗಿ (user = c->user_list->ಮುಂದಿನ; user != 0; user = ಮುಂದಿನ) {
		ಮುಂದಿನ = user->ಮುಂದಿನ;  /* user may be destroyed during callback */
		ಆದರೆ (user->callback != 0) {
			user->callback(c, CONTENT_ಸಂದೇಶ_ERROR,
				       &ದತ್ತಾಂಶ, user->pw);
		}
	}
}


/* exported interface, documented in content/content.h */
nserror
content_open(hlcache_handle *h,
	     ರಚನೆ browser_ಕಿಟಕಿ *bw,
	     ರಚನೆ content *ಪುಟ,
	     ರಚನೆ object_params *params)
{
	ರಚನೆ content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);
	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));
	ಆದರೆ (c->handler->open != ಖಾಲಿ) {
		res = c->handler->open(c, bw, ಪುಟ, params);
	} ಇಲ್ಲದಿದ್ದರೆ {
		res = NSERROR_OK;
	}
	ಹಿಂತಿರುಗಿ res;
}


/* exported interface, documented in content/content.h */
nserror content_close(hlcache_handle *h)
{
	ರಚನೆ content *c;
	nserror res;

	c = hlcache_handle_get_content(h);
	ಆದರೆ (c == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_BAD_PARAMETER;
	}

	ಆದರೆ ((c->ಸ್ಥಿತಿ_2 != CONTENT_ಸ್ಥಿತಿ_READY) &&
	    (c->ಸ್ಥಿತಿ_2 != CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ)) {
		/* status is not read or done so nothing to do */
		ಹಿಂತಿರುಗಿ NSERROR_INVALID;
	}

	NSLOG(netsurf, INFO, "content %p %s", c,
	      nsurl_access_log(llcache_handle_get_url(c->llcache)));

	ಆದರೆ (c->textsearch.context != ಖಾಲಿ) {
		content_textsearch_destroy(c->textsearch.context);
		c->textsearch.context = ಖಾಲಿ;
	}

	ಆದರೆ (c->handler->close != ಖಾಲಿ) {
		res = c->handler->close(c);
	} ಇಲ್ಲದಿದ್ದರೆ {
		res = NSERROR_OK;
	}
	ಹಿಂತಿರುಗಿ res;
}


/* exported interface, documented in content/content.h */
ಶೂನ್ಯ content_clear_selection(hlcache_handle *h)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಆದರೆ (c->handler->get_selection != ಖಾಲಿ)
		c->handler->clear_selection(c);
}


/* exported interface, documented in content/content.h */
ಅಕ್ಷರ * content_get_selection(hlcache_handle *h)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಆದರೆ (c->handler->get_selection != ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ c->handler->get_selection(c);
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


/* exported interface documented in content/content.h */
nserror
content_get_contextual_content(ರಚನೆ hlcache_handle *h,
			       ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
			       ರಚನೆ browser_ಕಿಟಕಿ_features *ದತ್ತಾಂಶ)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಆದರೆ (c->handler->get_contextual_content != ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ c->handler->get_contextual_content(c, x, y, ದತ್ತಾಂಶ);
	}

	ದತ್ತಾಂಶ->object = h;
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface, documented in content/content.h */
bool
content_scroll_at_ಬಿಂದು(ರಚನೆ hlcache_handle *h,
			ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
			ಪೂರ್ಣಾಂಕ scrx, ಪೂರ್ಣಾಂಕ scry)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಆದರೆ (c->handler->scroll_at_ಬಿಂದು != ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ c->handler->scroll_at_ಬಿಂದು(c, x, y, scrx, scry);

	ಹಿಂತಿರುಗಿ false;
}


/* exported interface, documented in content/content.h */
bool
content_drop_file_at_ಬಿಂದು(ರಚನೆ hlcache_handle *h,
			   ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
			   ಅಕ್ಷರ *file)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಆದರೆ (c->handler->drop_file_at_ಬಿಂದು != ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ c->handler->drop_file_at_ಬಿಂದು(c, x, y, file);

	ಹಿಂತಿರುಗಿ false;
}


/* exported interface documented in content/content.h */
nserror
content_debug_dump(ರಚನೆ hlcache_handle *h, FILE *f, ಎಣಿಕೆ content_debug op)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	ಪ್ರತಿಜ್ಞಾ(c != 0);

	ಆದರೆ (c->handler->debug_dump == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOT_IMPLEMENTED;
	}

	ಹಿಂತಿರುಗಿ c->handler->debug_dump(c, f, op);
}


/* exported interface documented in content/content.h */
nserror content_debug(ರಚನೆ hlcache_handle *h, ಎಣಿಕೆ content_debug op)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);

	ಆದರೆ (c == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_BAD_PARAMETER;
	}

	ಆದರೆ (c->handler->debug == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOT_IMPLEMENTED;
	}

	ಹಿಂತಿರುಗಿ c->handler->debug(c, op);
}


/* exported interface documented in content/content.h */
ರಚನೆ content_rfc5988_link *
content_ಕಂಡುಹಿಡಿ_rfc5988_link(hlcache_handle *h, lwc_string *rel)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);
	ರಚನೆ content_rfc5988_link *link = c->links;
	bool rel_ಹೊಂದಿಕೆ = false;

	ಆಗ (link != ಖಾಲಿ) {
		ಆದರೆ (lwc_string_caseless_isequal(link->rel, rel,
						&rel_ಹೊಂದಿಕೆ) == lwc_error_ok && rel_ಹೊಂದಿಕೆ) {
			ನಿಲ್ಲಿಸು;
		}
		link = link->ಮುಂದಿನ;
	}
	ಹಿಂತಿರುಗಿ link;
}


/* exported interface documented in content/protected.h */
ರಚನೆ content_rfc5988_link *
content__free_rfc5988_link(ರಚನೆ content_rfc5988_link *link)
{
	ರಚನೆ content_rfc5988_link *ಮುಂದಿನ;

	ಮುಂದಿನ = link->ಮುಂದಿನ;

	nsurl_unref(link->href);
	lwc_string_unref(link->rel);
	lwc_string_unref(link->hreflang);
	lwc_string_unref(link->type);
	lwc_string_unref(link->media);
	lwc_string_unref(link->sizes);
	ಬಿಡುಗಡೆ(link);

	ಹಿಂತಿರುಗಿ ಮುಂದಿನ;
}


/* exported interface documented in content/protected.h */
bool
content_ಕೂಡಿಸು_rfc5988_link(ರಚನೆ content *c,
			  ಸ್ಥಿರಾಂಕ ರಚನೆ content_rfc5988_link *link)
{
	ರಚನೆ content_rfc5988_link *newlink;
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;

	/* a link relation must be present for it to be a link */
	ಆದರೆ (link->rel == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ false;
	}

	/* a link href must be present for it to be a link */
	ಆದರೆ (link->href == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ false;
	}

	newlink = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ content_rfc5988_link));
	ಆದರೆ (newlink == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ false;
	}

	/* copy values */
	newlink->rel = lwc_string_ref(link->rel);
	newlink->href = nsurl_ref(link->href);
	ಆದರೆ (link->hreflang != ಖಾಲಿ) {
		newlink->hreflang = lwc_string_ref(link->hreflang);
	}
	ಆದರೆ (link->type != ಖಾಲಿ) {
		newlink->type = lwc_string_ref(link->type);
	}
	ಆದರೆ (link->media != ಖಾಲಿ) {
		newlink->media = lwc_string_ref(link->media);
	}
	ಆದರೆ (link->sizes != ಖಾಲಿ) {
		newlink->sizes = lwc_string_ref(link->sizes);
	}

	/* add to metadata link to list */
	newlink->ಮುಂದಿನ = c->links;
	c->links = newlink;

	/* broadcast the data */
	ಸಂದೇಶ_ದತ್ತಾಂಶ.rfc5988_link = newlink;
	content_ಪ್ರಸಾರ(c, CONTENT_ಸಂದೇಶ_LINK, &ಸಂದೇಶ_ದತ್ತಾಂಶ);

	ಹಿಂತಿರುಗಿ true;
}


/* exported interface documented in content/content.h */
nsurl *content_get_url(ರಚನೆ content *c)
{
	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ llcache_handle_get_url(c->llcache);
}


/* exported interface documented in content/content.h */
content_type content_get_type(hlcache_handle *h)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);

	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ CONTENT_NONE;

	ಹಿಂತಿರುಗಿ c->handler->type();
}


/* exported interface documented in content/content.h */
lwc_string *content_get_mime_type(hlcache_handle *h)
{
	ಹಿಂತಿರುಗಿ content__get_mime_type(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
lwc_string *content__get_mime_type(ರಚನೆ content *c)
{
	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ lwc_string_ref(c->mime_type);
}


/* exported interface documented in content/content_protected.h */
bool content_ಗಣ_title(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title)
{
	ಅಕ್ಷರ *new_title = strdup(title);
	ಆದರೆ (new_title == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (c->title != ಖಾಲಿ)
		ಬಿಡುಗಡೆ(c->title);

	c->title = new_title;

	ಹಿಂತಿರುಗಿ true;
}


/* exported interface documented in content/content.h */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *content_get_title(hlcache_handle *h)
{
	ಹಿಂತಿರುಗಿ content__get_title(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *content__get_title(ರಚನೆ content *c)
{
	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ c->title != ಖಾಲಿ ? c->title :
		nsurl_access(llcache_handle_get_url(c->llcache));
}


/* exported interface documented in content/content.h */
content_ಸ್ಥಿತಿ content_get_ಸ್ಥಿತಿ(hlcache_handle *h)
{
	ಹಿಂತಿರುಗಿ content_get_ಸ್ಥಿತಿ_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
content_ಸ್ಥಿತಿ content_get_ಸ್ಥಿತಿ_2(ರಚನೆ content *c)
{
	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ CONTENT_ಸ್ಥಿತಿ_ERROR;

	ಹಿಂತಿರುಗಿ c->ಸ್ಥಿತಿ_2;
}


/* exported interface documented in content/content.h */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *content_get_ಸ್ಥಿತಿ_ಸಂದೇಶ(hlcache_handle *h)
{
	ಹಿಂತಿರುಗಿ content_get_ಸ್ಥಿತಿ_ಸಂದೇಶ_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *content_get_ಸ್ಥಿತಿ_ಸಂದೇಶ_2(ರಚನೆ content *c)
{
	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ c->ಸ್ಥಿತಿ_ಸಂದೇಶ;
}


/* exported interface documented in content/content.h */
ಪೂರ್ಣಾಂಕ content_get_ಅಗಲ(hlcache_handle *h)
{
	ಹಿಂತಿರುಗಿ content_get_ಅಗಲ_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ಪೂರ್ಣಾಂಕ content_get_ಅಗಲ_2(ರಚನೆ content *c)
{
	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ c->ಅಗಲ;
}


/* exported interface documented in content/content.h */
ಪೂರ್ಣಾಂಕ content_get_ಎತ್ತರ(hlcache_handle *h)
{
	ಹಿಂತಿರುಗಿ content_get_ಎತ್ತರ_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ಪೂರ್ಣಾಂಕ content_get_ಎತ್ತರ_2(ರಚನೆ content *c)
{
	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ c->ಎತ್ತರ;
}


/* exported interface documented in content/content.h */
ಪೂರ್ಣಾಂಕ content_get_ಲಭ್ಯ_ಅಗಲ(hlcache_handle *h)
{
	ಹಿಂತಿರುಗಿ content_get_ಲಭ್ಯ_ಅಗಲ_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ಪೂರ್ಣಾಂಕ content_get_ಲಭ್ಯ_ಅಗಲ_2(ರಚನೆ content *c)
{
	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ c->ಲಭ್ಯ_ಅಗಲ;
}


/* exported interface documented in content/content.h */
ಸ್ಥಿರಾಂಕ uint8_t *content_get_ಮೂಲ_ದತ್ತಾಂಶ(hlcache_handle *h, size_t *ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(hlcache_handle_get_content(h), ಗಾತ್ರ_2);
}


/* exported interface documented in content/content_protected.h */
ಸ್ಥಿರಾಂಕ uint8_t *content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(ರಚನೆ content *c, size_t *ಗಾತ್ರ_2)
{
	ಪ್ರತಿಜ್ಞಾ(ಗಾತ್ರ_2 != ಖಾಲಿ);

	/** \todo check if the content check should be an assert */
	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ llcache_handle_get_ಮೂಲ_ದತ್ತಾಂಶ(c->llcache, ಗಾತ್ರ_2);
}


/* exported interface documented in content/content.h */
ಶೂನ್ಯ content_invalidate_reuse_ದತ್ತಾಂಶ(hlcache_handle *h)
{
	content_invalidate_reuse_ದತ್ತಾಂಶ_2(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ಶೂನ್ಯ content_invalidate_reuse_ದತ್ತಾಂಶ_2(ರಚನೆ content *c)
{
	ಆದರೆ (c == ಖಾಲಿ || c->llcache == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ;

	/* Invalidate low-level cache data */
	llcache_handle_invalidate_cache_ದತ್ತಾಂಶ(c->llcache);
}


/* exported interface documented in content/content.h */
nsurl *content_get_refresh_url(hlcache_handle *h)
{
	ಹಿಂತಿರುಗಿ content__get_refresh_url(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
nsurl *content__get_refresh_url(ರಚನೆ content *c)
{
	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ c->refresh;
}


/* exported interface documented in content/content.h */
ರಚನೆ bitmap *content_get_bitmap(hlcache_handle *h)
{
	ಹಿಂತಿರುಗಿ content__get_bitmap(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
ರಚನೆ bitmap *content__get_bitmap(ರಚನೆ content *c)
{
	ರಚನೆ bitmap *bitmap = ಖಾಲಿ;

	ಆದರೆ ((c != ಖಾಲಿ) &&
	    (c->handler != ಖಾಲಿ) &&
	    (c->handler->type != ಖಾಲಿ) &&
	    (c->handler->type() == CONTENT_IMAGE) &&
	    (c->handler->get_ಆಂತರಿಕ != ಖಾಲಿ) ) {
		bitmap = c->handler->get_ಆಂತರಿಕ(c, ಖಾಲಿ);
	}

	ಹಿಂತಿರುಗಿ bitmap;
}


/* exported interface documented in content/content.h */
bool content_get_opaque(hlcache_handle *h)
{
	ಹಿಂತಿರುಗಿ content__get_opaque(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__get_opaque(ರಚನೆ content *c)
{
	ಆದರೆ ((c != ಖಾಲಿ) &&
	    (c->handler != ಖಾಲಿ) &&
	    (c->handler->is_opaque != ಖಾಲಿ)) {
		ಹಿಂತಿರುಗಿ c->handler->is_opaque(c);
	}

	ಹಿಂತಿರುಗಿ false;
}


/* exported interface documented in content/content.h */
bool content_get_quirks(hlcache_handle *h)
{
	ರಚನೆ content *c = hlcache_handle_get_content(h);

	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ false;

	ಹಿಂತಿರುಗಿ c->quirks;
}


/* exported interface documented in content/content.h */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
content_get_encoding(hlcache_handle *h, ಎಣಿಕೆ content_encoding_type op)
{
	ಹಿಂತಿರುಗಿ content__get_encoding(hlcache_handle_get_content(h), op);
}


/* exported interface documented in content/content_protected.h */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
content__get_encoding(ರಚನೆ content *c, ಎಣಿಕೆ content_encoding_type op)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *encoding = ಖಾಲಿ;

	ಆದರೆ ((c != ಖಾಲಿ) &&
	    (c->handler != ಖಾಲಿ) &&
	    (c->handler->get_encoding != ಖಾಲಿ) ) {
		encoding = c->handler->get_encoding(c, op);
	}

	ಹಿಂತಿರುಗಿ encoding;
}


/* exported interface documented in content/content.h */
bool content_is_locked(hlcache_handle *h)
{
	ಹಿಂತಿರುಗಿ content__is_locked(hlcache_handle_get_content(h));
}


/* exported interface documented in content/content_protected.h */
bool content__is_locked(ರಚನೆ content *c)
{
	ಹಿಂತಿರುಗಿ c->locked;
}


/* exported interface documented in content/content.h */
ಸ್ಥಿರಾಂಕ llcache_handle *content_get_llcache_handle(ರಚನೆ content *c)
{
	ಆದರೆ (c == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ c->llcache;
}


/* exported interface documented in content/protected.h */
ರಚನೆ content *content_clone(ರಚನೆ content *c)
{
	ರಚನೆ content *nc;
	nserror error;

	error = c->handler->clone(c, &nc);
	ಆದರೆ (error != NSERROR_OK)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ nc;
};


/* exported interface documented in content/protected.h */
nserror content__clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c, ರಚನೆ content *nc)
{
	nserror error;

	error = llcache_handle_clone(c->llcache, &(nc->llcache));
	ಆದರೆ (error != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ error;
	}

	llcache_handle_change_callback(nc->llcache,
				       content_llcache_callback, nc);

	nc->mime_type = lwc_string_ref(c->mime_type);
	nc->handler = c->handler;

	nc->ಸ್ಥಿತಿ_2 = c->ಸ್ಥಿತಿ_2;

	nc->ಅಗಲ = c->ಅಗಲ;
	nc->ಎತ್ತರ = c->ಎತ್ತರ;
	nc->ಲಭ್ಯ_ಅಗಲ = c->ಲಭ್ಯ_ಅಗಲ;
	nc->quirks = c->quirks;

	ಆದರೆ (c->fallback_charset != ಖಾಲಿ) {
		nc->fallback_charset = strdup(c->fallback_charset);
		ಆದರೆ (nc->fallback_charset == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	}

	ಆದರೆ (c->refresh != ಖಾಲಿ) {
		nc->refresh = nsurl_ref(c->refresh);
		ಆದರೆ (nc->refresh == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	}

	nc->time = c->time;
	nc->reformat_ಸಮಯ = c->reformat_ಸಮಯ;
	nc->ಗಾತ್ರ_2 = c->ಗಾತ್ರ_2;

	ಆದರೆ (c->title != ಖಾಲಿ) {
		nc->title = strdup(c->title);
		ಆದರೆ (nc->title == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	}

	nc->ಸಕ್ರಿಯ = c->ಸಕ್ರಿಯ;

	nc->user_list = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ content_user));
	ಆದರೆ (nc->user_list == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಸ್ಮರಣೆ_ನಕಲು(&(nc->ಸ್ಥಿತಿ_ಸಂದೇಶ), &(c->ಸ್ಥಿತಿ_ಸಂದೇಶ), 120);
	ಸ್ಮರಣೆ_ನಕಲು(&(nc->sub_ಸ್ಥಿತಿ), &(c->sub_ಸ್ಥಿತಿ), 80);

	nc->locked = c->locked;
	nc->ಒಟ್ಟು_ಗಾತ್ರ = c->ಒಟ್ಟು_ಗಾತ್ರ;
	nc->http_ಸಂಕೇತ = c->http_ಸಂಕೇತ;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in content/content.h */
nserror content_abort(ರಚನೆ content *c)
{
	NSLOG(netsurf, INFO, "Aborting %p", c);

	ಆದರೆ (c->handler->stop != ಖಾಲಿ)
		c->handler->stop(c);

	/* And for now, abort our llcache object */
	ಹಿಂತಿರುಗಿ llcache_handle_abort(c->llcache);
}
