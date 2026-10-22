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
ساخت fetch_javascript_context {
	ساخت fetch_javascript_context *r_اگلا, *r_پچھلا;

	ساخت fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

ساکن ساخت fetch_javascript_context *ring = تہی;


/** issue fetch callbacks with locking */
ساکن اندرونی bool fetch_javascript_send_callback(ثابت fetch_پیغام *پیغام,
		ساخت fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پیغام, ctx->fetchh);
	ctx->locked = false;

	واپس ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
ساکن bool fetch_javascript_handler(ساخت fetch_javascript_context *ctx)
{
	fetch_پیغام پیغام;
	صحیح رمز = 204;

	/* content is going to return error code */
	fetch_سیٹ_http_رمز(ctx->fetchh, رمز);

	پیغام.type = FETCH_ختم_شدہ;
	fetch_javascript_send_callback(&پیغام, ctx);

	واپس true;
}



/** callback to initialise the resource fetcher. */
ساکن bool fetch_javascript_initialise(lwc_string *scheme)
{
	واپس true;
}

/** callback to finalise the resource fetcher. */
ساکن خالی fetch_javascript_finalise(lwc_string *scheme)
{
}

ساکن bool fetch_javascript_can_fetch(ثابت nsurl *url)
{
	واپس true;
}

/** callback to set up a resource fetch context. */
ساکن خالی *
fetch_javascript_تیاری(ساخت fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ثابت حرف *post_urlenc,
		 ثابت ساخت fetch_multipart_ڈیٹا *post_multipart,
		 ثابت حرف **سرخیاں)
{
	ساخت fetch_javascript_context *ctx;

	ctx = صفر_حافظہ_دو(1, حجم(*ctx));
	اگر (ctx == تہی)
		واپس تہی;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_داخل_کرو(ring, ctx);

	واپس ctx;
}

/** callback to free a resource fetch */
ساکن خالی fetch_javascript_free(خالی *ctx)
{
	ساخت fetch_javascript_context *c = ctx;
	اگر (c->url != تہی) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	آزاد(ctx);
}

/** callback to start a resource fetch */
ساکن bool fetch_javascript_آغاز(خالی *ctx)
{
	واپس true;
}

/** callback to abort a resource fetch */
ساکن خالی fetch_javascript_abort(خالی *ctx)
{
	ساخت fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ساکن خالی fetch_javascript_poll(lwc_string *scheme)
{
	ساخت fetch_javascript_context *c, *اگلا;

	اگر (ring == تہی) واپس;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	کرو {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		اگر (c->locked == true) {
			اگلا = c->r_اگلا;
			جاری;
		}

		/* Only process non-aborted fetches */
		اگر (c->aborted == false) {
			/* resource fetches can be processed in one go */
			fetch_javascript_handler(c);
		}

		/* Compute next fetch item at the last possible moment
		 * as processing this item may have added to the ring
		 */
		اگلا = c->r_اگلا;

		fetch_remove_from_queues(c->fetchh);
		fetch_free(c->fetchh);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} جب ( (c = اگلا) != ring && ring != تہی);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_رجسٹر(خالی)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	ثابت ساخت fetcher_operation_جدول fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.تیاری = fetch_javascript_تیاری,
		.آغاز = fetch_javascript_آغاز,
		.abort = fetch_javascript_abort,
		.آزاد = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	واپس fetcher_جوڑو(scheme, &fetcher_ops);
}
