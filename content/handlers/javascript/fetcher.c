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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <libwapcaplet/libwapcaplet.h>

#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/ring.h"
#અન્તર્ભાવ "content/fetch.h"
#અન્તર્ભાવ "content/fetchers.h"

#અન્તર્ભાવ "javascript/fetcher.h"

/** Context for an resource fetch */
સંરચના fetch_javascript_context {
	સંરચના fetch_javascript_context *r_next, *r_prev;

	સંરચના fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

સ્થિર સંરચના fetch_javascript_context *ring = ખાલી;


/** issue fetch callbacks with locking */
સ્થિર અંતઃરેખ bool fetch_javascript_send_callback(અચલ fetch_સંદેશ *સંદેશ,
		સંરચના fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(સંદેશ, ctx->fetchh);
	ctx->locked = false;

	પાછા ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
સ્થિર bool fetch_javascript_handler(સંરચના fetch_javascript_context *ctx)
{
	fetch_સંદેશ સંદેશ;
	પૂર્ણાંક code = 204;

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	સંદેશ.type = FETCH_FINISHED;
	fetch_javascript_send_callback(&સંદેશ, ctx);

	પાછા true;
}



/** callback to initialise the resource fetcher. */
સ્થિર bool fetch_javascript_initialise(lwc_string *scheme)
{
	પાછા true;
}

/** callback to finalise the resource fetcher. */
સ્થિર શૂન્ય fetch_javascript_finalise(lwc_string *scheme)
{
}

સ્થિર bool fetch_javascript_can_fetch(અચલ nsurl *url)
{
	પાછા true;
}

/** callback to set up a resource fetch context. */
સ્થિર શૂન્ય *
fetch_javascript_setup(સંરચના fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 અચલ અક્ષર *post_urlenc,
		 અચલ સંરચના fetch_multipart_data *post_multipart,
		 અચલ અક્ષર **headers)
{
	સંરચના fetch_javascript_context *ctx;

	ctx = calloc(1, કદ(*ctx));
	જો (ctx == ખાલી)
		પાછા ખાલી;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_ઉમેરો(ring, ctx);

	પાછા ctx;
}

/** callback to free a resource fetch */
સ્થિર શૂન્ય fetch_javascript_free(શૂન્ય *ctx)
{
	સંરચના fetch_javascript_context *c = ctx;
	જો (c->url != ખાલી) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	મુક્ત(ctx);
}

/** callback to start a resource fetch */
સ્થિર bool fetch_javascript_start(શૂન્ય *ctx)
{
	પાછા true;
}

/** callback to abort a resource fetch */
સ્થિર શૂન્ય fetch_javascript_abort(શૂન્ય *ctx)
{
	સંરચના fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
સ્થિર શૂન્ય fetch_javascript_poll(lwc_string *scheme)
{
	સંરચના fetch_javascript_context *c, *next;

	જો (ring == ખાલી) પાછા;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	કરો {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		જો (c->locked == true) {
			next = c->r_next;
			ચાલુ;
		}

		/* Only process non-aborted fetches */
		જો (c->aborted == false) {
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
	} જ્યારે ( (c = next) != ring && ring != ખાલી);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_register(શૂન્ય)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	અચલ સંરચના fetcher_operation_table fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.setup = fetch_javascript_setup,
		.start = fetch_javascript_start,
		.abort = fetch_javascript_abort,
		.મુક્ત = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	પાછા fetcher_add(scheme, &fetcher_ops);
}
