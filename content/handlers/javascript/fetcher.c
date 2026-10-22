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

#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <libwapcaplet/libwapcaplet.h>

#شامل "utils/nsurl.h"
#شامل "utils/corestrings.h"
#شامل "utils/ring.h"
#شامل "content/fetch.h"
#شامل "content/fetchers.h"

#شامل "javascript/fetcher.h"

/** Context for an resource fetch */
ساختار fetch_javascript_context {
	ساختار fetch_javascript_context *r_next, *r_prev;

	ساختار fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

ایستا ساختار fetch_javascript_context *ring = پوچ;


/** issue fetch callbacks with locking */
ایستا درونخطی bool fetch_javascript_send_callback(ثابت fetch_پیام *پیام,
		ساختار fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پیام, ctx->fetchh);
	ctx->locked = false;

	بازگشت ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
ایستا bool fetch_javascript_handler(ساختار fetch_javascript_context *ctx)
{
	fetch_پیام پیام;
	صحیح code = 204;

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	پیام.type = FETCH_FINISHED;
	fetch_javascript_send_callback(&پیام, ctx);

	بازگشت true;
}



/** callback to initialise the resource fetcher. */
ایستا bool fetch_javascript_initialise(lwc_string *scheme)
{
	بازگشت true;
}

/** callback to finalise the resource fetcher. */
ایستا تهی fetch_javascript_finalise(lwc_string *scheme)
{
}

ایستا bool fetch_javascript_can_fetch(ثابت nsurl *url)
{
	بازگشت true;
}

/** callback to set up a resource fetch context. */
ایستا تهی *
fetch_javascript_setup(ساختار fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ثابت نویسه *post_urlenc,
		 ثابت ساختار fetch_multipart_data *post_multipart,
		 ثابت نویسه **headers)
{
	ساختار fetch_javascript_context *ctx;

	ctx = calloc(1, اندازه(*ctx));
	اگر (ctx == پوچ)
		بازگشت پوچ;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_درج(ring, ctx);

	بازگشت ctx;
}

/** callback to free a resource fetch */
ایستا تهی fetch_javascript_free(تهی *ctx)
{
	ساختار fetch_javascript_context *c = ctx;
	اگر (c->url != پوچ) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	آزاد(ctx);
}

/** callback to start a resource fetch */
ایستا bool fetch_javascript_start(تهی *ctx)
{
	بازگشت true;
}

/** callback to abort a resource fetch */
ایستا تهی fetch_javascript_abort(تهی *ctx)
{
	ساختار fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ایستا تهی fetch_javascript_poll(lwc_string *scheme)
{
	ساختار fetch_javascript_context *c, *next;

	اگر (ring == پوچ) بازگشت;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	انجامبده {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		اگر (c->locked == true) {
			next = c->r_next;
			ادامه;
		}

		/* Only process non-aborted fetches */
		اگر (c->aborted == false) {
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
	} وقتی ( (c = next) != ring && ring != پوچ);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_register(تهی)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	ثابت ساختار fetcher_operation_table fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.setup = fetch_javascript_setup,
		.start = fetch_javascript_start,
		.abort = fetch_javascript_abort,
		.آزاد = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	بازگشت fetcher_add(scheme, &fetcher_ops);
}
