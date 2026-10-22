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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <libwapcaplet/libwapcaplet.h>

#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/ring.h"
#অন্তর্ভাব "content/fetch.h"
#অন্তর্ভাব "content/fetchers.h"

#অন্তর্ভাব "javascript/fetcher.h"

/** Context for an resource fetch */
কাঠামো fetch_javascript_context {
	কাঠামো fetch_javascript_context *r_পরবর্তী, *r_পূর্ব;

	কাঠামো fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

স্থির কাঠামো fetch_javascript_context *ring = ফাঁকা;


/** issue fetch callbacks with locking */
স্থির অন্তঃরেখ bool fetch_javascript_send_callback(ধ্রুবক fetch_বার্তা *বার্তা,
		কাঠামো fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(বার্তা, ctx->fetchh);
	ctx->locked = false;

	ফেরত ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
স্থির bool fetch_javascript_handler(কাঠামো fetch_javascript_context *ctx)
{
	fetch_বার্তা বার্তা;
	পূর্ণসংখ্যা code = 204;

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	বার্তা.type = FETCH_FINISHED;
	fetch_javascript_send_callback(&বার্তা, ctx);

	ফেরত true;
}



/** callback to initialise the resource fetcher. */
স্থির bool fetch_javascript_initialise(lwc_string *scheme)
{
	ফেরত true;
}

/** callback to finalise the resource fetcher. */
স্থির শূন্য fetch_javascript_finalise(lwc_string *scheme)
{
}

স্থির bool fetch_javascript_can_fetch(ধ্রুবক nsurl *url)
{
	ফেরত true;
}

/** callback to set up a resource fetch context. */
স্থির শূন্য *
fetch_javascript_setup(কাঠামো fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ধ্রুবক অক্ষর *post_urlenc,
		 ধ্রুবক কাঠামো fetch_multipart_তথ্য *post_multipart,
		 ধ্রুবক অক্ষর **headers)
{
	কাঠামো fetch_javascript_context *ctx;

	ctx = calloc(1, আকার(*ctx));
	যদি (ctx == ফাঁকা)
		ফেরত ফাঁকা;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_ঢোকাও(ring, ctx);

	ফেরত ctx;
}

/** callback to free a resource fetch */
স্থির শূন্য fetch_javascript_free(শূন্য *ctx)
{
	কাঠামো fetch_javascript_context *c = ctx;
	যদি (c->url != ফাঁকা) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	মুক্ত(ctx);
}

/** callback to start a resource fetch */
স্থির bool fetch_javascript_start(শূন্য *ctx)
{
	ফেরত true;
}

/** callback to abort a resource fetch */
স্থির শূন্য fetch_javascript_abort(শূন্য *ctx)
{
	কাঠামো fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
স্থির শূন্য fetch_javascript_poll(lwc_string *scheme)
{
	কাঠামো fetch_javascript_context *c, *পরবর্তী;

	যদি (ring == ফাঁকা) ফেরত;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	করো {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		যদি (c->locked == true) {
			পরবর্তী = c->r_পরবর্তী;
			চালু;
		}

		/* Only process non-aborted fetches */
		যদি (c->aborted == false) {
			/* resource fetches can be processed in one go */
			fetch_javascript_handler(c);
		}

		/* Compute next fetch item at the last possible moment
		 * as processing this item may have added to the ring
		 */
		পরবর্তী = c->r_পরবর্তী;

		fetch_remove_from_queues(c->fetchh);
		fetch_free(c->fetchh);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} যখন ( (c = পরবর্তী) != ring && ring != ফাঁকা);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_register(শূন্য)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	ধ্রুবক কাঠামো fetcher_operation_table fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.setup = fetch_javascript_setup,
		.start = fetch_javascript_start,
		.abort = fetch_javascript_abort,
		.মুক্ত = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	ফেরত fetcher_add(scheme, &fetcher_ops);
}
