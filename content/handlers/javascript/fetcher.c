/*
 * Copyright 2012 Vincent Sanders <vince@kyllikki.org>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 * implementation for javascript scheme fetcher
 *
 * This fetcher implements http://www.whatwg.org/specs/web-apps/current-work/multipage/browsers.html#javascript-protocol
 */

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>

#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/ring.h"
#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers.h"

#ಅನ್ತರ್ಭಾವ "javascript/fetcher.h"

/** Context for an resource fetch */
ರಚನೆ fetch_javascript_context {
	ರಚನೆ fetch_javascript_context *r_ಮುಂದಿನ, *r_prev;

	ರಚನೆ fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

ಸ್ಥಿರ ರಚನೆ fetch_javascript_context *ring = ಖಾಲಿ;


/** issue fetch callbacks with locking */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool fetch_javascript_send_callback(ಸ್ಥಿರಾಂಕ fetch_ಸಂದೇಶ *ಸಂದೇಶ,
		ರಚನೆ fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ಸಂದೇಶ, ctx->fetchh);
	ctx->locked = false;

	ಹಿಂತಿರುಗಿ ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
ಸ್ಥಿರ bool fetch_javascript_handler(ರಚನೆ fetch_javascript_context *ctx)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2 = 204;

	/* content is going to return error code */
	fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, ಸಂಕೇತ_2);

	ಸಂದೇಶ.type = FETCH_ಪೂರ್ಣಗೊಂಡ;
	fetch_javascript_send_callback(&ಸಂದೇಶ, ctx);

	ಹಿಂತಿರುಗಿ true;
}



/** callback to initialise the resource fetcher. */
ಸ್ಥಿರ bool fetch_javascript_initialise(lwc_string *scheme)
{
	ಹಿಂತಿರುಗಿ true;
}

/** callback to finalise the resource fetcher. */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_javascript_finalise(lwc_string *scheme)
{
}

ಸ್ಥಿರ bool fetch_javascript_can_fetch(ಸ್ಥಿರಾಂಕ nsurl *url)
{
	ಹಿಂತಿರುಗಿ true;
}

/** callback to set up a resource fetch context. */
ಸ್ಥಿರ ಶೂನ್ಯ *
fetch_javascript_ಸಿದ್ಧತೆ(ರಚನೆ fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *post_urlenc,
		 ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *post_multipart,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಶೀರ್ಷಿಕೆಗಳು)
{
	ರಚನೆ fetch_javascript_context *ctx;

	ctx = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*ctx));
	ಆದರೆ (ctx == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_ಸೇರಿಸು(ring, ctx);

	ಹಿಂತಿರುಗಿ ctx;
}

/** callback to free a resource fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_javascript_free(ಶೂನ್ಯ *ctx)
{
	ರಚನೆ fetch_javascript_context *c = ctx;
	ಆದರೆ (c->url != ಖಾಲಿ) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	ಬಿಡುಗಡೆ(ctx);
}

/** callback to start a resource fetch */
ಸ್ಥಿರ bool fetch_javascript_ಆರಂಭ(ಶೂನ್ಯ *ctx)
{
	ಹಿಂತಿರುಗಿ true;
}

/** callback to abort a resource fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_javascript_abort(ಶೂನ್ಯ *ctx)
{
	ರಚನೆ fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_javascript_poll(lwc_string *scheme)
{
	ರಚನೆ fetch_javascript_context *c, *ಮುಂದಿನ;

	ಆದರೆ (ring == ಖಾಲಿ) ಹಿಂತಿರುಗಿ;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	ಮಾಡು {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ಆದರೆ (c->locked == true) {
			ಮುಂದಿನ = c->r_ಮುಂದಿನ;
			ಮುಂದುವರಿ;
		}

		/* Only process non-aborted fetches */
		ಆದರೆ (c->aborted == false) {
			/* resource fetches can be processed in one go */
			fetch_javascript_handler(c);
		}

		/* Compute next fetch item at the last possible moment
		 * as processing this item may have added to the ring
		 */
		ಮುಂದಿನ = c->r_ಮುಂದಿನ;

		fetch_remove_from_queues(c->fetchh);
		fetch_free(c->fetchh);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} ಆಗ ( (c = ಮುಂದಿನ) != ring && ring != ಖಾಲಿ);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_ರಿಜಿಸ್ಟರ್(ಶೂನ್ಯ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	ಸ್ಥಿರಾಂಕ ರಚನೆ fetcher_operation_ಕೋಷ್ಟಕ fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.ಸಿದ್ಧತೆ = fetch_javascript_ಸಿದ್ಧತೆ,
		.ಆರಂಭ = fetch_javascript_ಆರಂಭ,
		.abort = fetch_javascript_abort,
		.ಬಿಡುಗಡೆ = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	ಹಿಂತಿರುಗಿ fetcher_ಕೂಡಿಸು(scheme, &fetcher_ops);
}
