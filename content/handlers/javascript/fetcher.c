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
ಸಂರಚನಾ fetch_javascript_context {
	ಸಂರಚನಾ fetch_javascript_context *r_next, *r_prev;

	ಸಂರಚನಾ fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

ಸ್ಥಿರ ಸಂರಚನಾ fetch_javascript_context *ring = ರಿಕ್ತ;


/** issue fetch callbacks with locking */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool fetch_javascript_send_callback(ಧ್ರುವ fetch_ಸಂದೇಶ *ಸಂದೇಶ,
		ಸಂರಚನಾ fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ಸಂದೇಶ, ctx->fetchh);
	ctx->locked = false;

	ಪ್ರತ್ಯಾಗಮ ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
ಸ್ಥಿರ bool fetch_javascript_handler(ಸಂರಚನಾ fetch_javascript_context *ctx)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಪೂರ್ಣಾಙ್ಕ code = 204;

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	ಸಂದೇಶ.type = FETCH_FINISHED;
	fetch_javascript_send_callback(&ಸಂದೇಶ, ctx);

	ಪ್ರತ್ಯಾಗಮ true;
}



/** callback to initialise the resource fetcher. */
ಸ್ಥಿರ bool fetch_javascript_initialise(lwc_string *scheme)
{
	ಪ್ರತ್ಯಾಗಮ true;
}

/** callback to finalise the resource fetcher. */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_javascript_finalise(lwc_string *scheme)
{
}

ಸ್ಥಿರ bool fetch_javascript_can_fetch(ಧ್ರುವ nsurl *url)
{
	ಪ್ರತ್ಯಾಗಮ true;
}

/** callback to set up a resource fetch context. */
ಸ್ಥಿರ ಶೂನ್ಯ *
fetch_javascript_setup(ಸಂರಚನಾ fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ಧ್ರುವ ವರ್ಣ *post_urlenc,
		 ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *post_multipart,
		 ಧ್ರುವ ವರ್ಣ **headers)
{
	ಸಂರಚನಾ fetch_javascript_context *ctx;

	ctx = calloc(1, ಪರಿಮಾಣ(*ctx));
	ಯದಿ (ctx == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_ಸೇರಿಸು(ring, ctx);

	ಪ್ರತ್ಯಾಗಮ ctx;
}

/** callback to free a resource fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_javascript_free(ಶೂನ್ಯ *ctx)
{
	ಸಂರಚನಾ fetch_javascript_context *c = ctx;
	ಯದಿ (c->url != ರಿಕ್ತ) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	ಮುಕ್ತ(ctx);
}

/** callback to start a resource fetch */
ಸ್ಥಿರ bool fetch_javascript_start(ಶೂನ್ಯ *ctx)
{
	ಪ್ರತ್ಯಾಗಮ true;
}

/** callback to abort a resource fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_javascript_abort(ಶೂನ್ಯ *ctx)
{
	ಸಂರಚನಾ fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_javascript_poll(lwc_string *scheme)
{
	ಸಂರಚನಾ fetch_javascript_context *c, *next;

	ಯದಿ (ring == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	ಕುರು {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ಯದಿ (c->locked == true) {
			next = c->r_next;
			ಅನುವರ್ತ;
		}

		/* Only process non-aborted fetches */
		ಯದಿ (c->aborted == false) {
			/* resource fetches can be processed in one go */
			fetch_javascript_handler(c);
		}

		/* Compute next fetch item at the last possible moment
		 * as processing this item may have added to the ring
		 */
		next = c->r_next;

		fetch_remove_from_queues(c->fetchh);
		fetch_free(c->fetchh);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} ಯಾವತ್ ( (c = next) != ring && ring != ರಿಕ್ತ);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_register(ಶೂನ್ಯ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	ಧ್ರುವ ಸಂರಚನಾ fetcher_operation_table fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.setup = fetch_javascript_setup,
		.start = fetch_javascript_start,
		.abort = fetch_javascript_abort,
		.ಮುಕ್ತ = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	ಪ್ರತ್ಯಾಗಮ fetcher_add(scheme, &fetcher_ops);
}
