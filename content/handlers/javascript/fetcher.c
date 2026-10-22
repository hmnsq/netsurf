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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <libwapcaplet/libwapcaplet.h>

#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/ring.h"
#ଅନ୍ତର୍ଭାଵ "content/fetch.h"
#ଅନ୍ତର୍ଭାଵ "content/fetchers.h"

#ଅନ୍ତର୍ଭାଵ "javascript/fetcher.h"

/** Context for an resource fetch */
ସଂରଚନା fetch_javascript_context {
	ସଂରଚନା fetch_javascript_context *r_next, *r_prev;

	ସଂରଚନା fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

ସ୍ଥିର ସଂରଚନା fetch_javascript_context *ring = ଖାଲି;


/** issue fetch callbacks with locking */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool fetch_javascript_send_callback(ଅଚଳ fetch_ସନ୍ଦେଶ *ସନ୍ଦେଶ,
		ସଂରଚନା fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ସନ୍ଦେଶ, ctx->fetchh);
	ctx->locked = false;

	ଫେରନ୍ତୁ ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
ସ୍ଥିର bool fetch_javascript_handler(ସଂରଚନା fetch_javascript_context *ctx)
{
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା code = 204;

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	ସନ୍ଦେଶ.type = FETCH_FINISHED;
	fetch_javascript_send_callback(&ସନ୍ଦେଶ, ctx);

	ଫେରନ୍ତୁ true;
}



/** callback to initialise the resource fetcher. */
ସ୍ଥିର bool fetch_javascript_initialise(lwc_string *scheme)
{
	ଫେରନ୍ତୁ true;
}

/** callback to finalise the resource fetcher. */
ସ୍ଥିର ଶୂନ୍ୟ fetch_javascript_finalise(lwc_string *scheme)
{
}

ସ୍ଥିର bool fetch_javascript_can_fetch(ଅଚଳ nsurl *url)
{
	ଫେରନ୍ତୁ true;
}

/** callback to set up a resource fetch context. */
ସ୍ଥିର ଶୂନ୍ୟ *
fetch_javascript_setup(ସଂରଚନା fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ଅଚଳ ଅକ୍ଷର *post_urlenc,
		 ଅଚଳ ସଂରଚନା fetch_multipart_data *post_multipart,
		 ଅଚଳ ଅକ୍ଷର **headers)
{
	ସଂରଚନା fetch_javascript_context *ctx;

	ctx = calloc(1, ଆକାର(*ctx));
	ଯଦି (ctx == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_ଭର୍ତ୍ତି(ring, ctx);

	ଫେରନ୍ତୁ ctx;
}

/** callback to free a resource fetch */
ସ୍ଥିର ଶୂନ୍ୟ fetch_javascript_free(ଶୂନ୍ୟ *ctx)
{
	ସଂରଚନା fetch_javascript_context *c = ctx;
	ଯଦି (c->url != ଖାଲି) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	ମୁକ୍ତ(ctx);
}

/** callback to start a resource fetch */
ସ୍ଥିର bool fetch_javascript_start(ଶୂନ୍ୟ *ctx)
{
	ଫେରନ୍ତୁ true;
}

/** callback to abort a resource fetch */
ସ୍ଥିର ଶୂନ୍ୟ fetch_javascript_abort(ଶୂନ୍ୟ *ctx)
{
	ସଂରଚନା fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ସ୍ଥିର ଶୂନ୍ୟ fetch_javascript_poll(lwc_string *scheme)
{
	ସଂରଚନା fetch_javascript_context *c, *next;

	ଯଦି (ring == ଖାଲି) ଫେରନ୍ତୁ;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	କର {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ଯଦି (c->locked == true) {
			next = c->r_next;
			ଜାରି;
		}

		/* Only process non-aborted fetches */
		ଯଦି (c->aborted == false) {
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
	} ଯେତେବେଳେ ( (c = next) != ring && ring != ଖାଲି);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_register(ଶୂନ୍ୟ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	ଅଚଳ ସଂରଚନା fetcher_operation_table fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.setup = fetch_javascript_setup,
		.start = fetch_javascript_start,
		.abort = fetch_javascript_abort,
		.ମୁକ୍ତ = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	ଫେରନ୍ତୁ fetcher_add(scheme, &fetcher_ops);
}
