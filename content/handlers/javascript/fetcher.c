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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libwapcaplet/libwapcaplet.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "javascript/fetcher.h"

/** Context for an resource fetch */
ਬਣਤਰ fetch_javascript_context {
	ਬਣਤਰ fetch_javascript_context *r_ਅਗਲਾ, *r_prev;

	ਬਣਤਰ fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

ਸਥਿਰ ਬਣਤਰ fetch_javascript_context *ring = ਨਿਰਾ;


/** issue fetch callbacks with locking */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool fetch_javascript_send_callback(ਅਚਲ fetch_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ,
		ਬਣਤਰ fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ਸੁਨੇਹਾ, ctx->fetchh);
	ctx->locked = false;

	ਵਾਪਸ ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
ਸਥਿਰ bool fetch_javascript_handler(ਬਣਤਰ fetch_javascript_context *ctx)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	ਪੂਰਨ ਕੋਡ = 204;

	/* content is going to return error code */
	fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, ਕੋਡ);

	ਸੁਨੇਹਾ.type = FETCH_ਮੁਕੰਮਲ;
	fetch_javascript_send_callback(&ਸੁਨੇਹਾ, ctx);

	ਵਾਪਸ true;
}



/** callback to initialise the resource fetcher. */
ਸਥਿਰ bool fetch_javascript_initialise(lwc_string *scheme)
{
	ਵਾਪਸ true;
}

/** callback to finalise the resource fetcher. */
ਸਥਿਰ ਖਾਲੀ fetch_javascript_finalise(lwc_string *scheme)
{
}

ਸਥਿਰ bool fetch_javascript_can_fetch(ਅਚਲ nsurl *url)
{
	ਵਾਪਸ true;
}

/** callback to set up a resource fetch context. */
ਸਥਿਰ ਖਾਲੀ *
fetch_javascript_ਤਿਆਰੀ(ਬਣਤਰ fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ਅਚਲ ਅੱਖਰ *post_urlenc,
		 ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *post_multipart,
		 ਅਚਲ ਅੱਖਰ **ਸਿਰਲੇਖਾਂ)
{
	ਬਣਤਰ fetch_javascript_context *ctx;

	ctx = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*ctx));
	ਜੇ (ctx == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_ਪਾਓ(ring, ctx);

	ਵਾਪਸ ctx;
}

/** callback to free a resource fetch */
ਸਥਿਰ ਖਾਲੀ fetch_javascript_free(ਖਾਲੀ *ctx)
{
	ਬਣਤਰ fetch_javascript_context *c = ctx;
	ਜੇ (c->url != ਨਿਰਾ) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	ਮੁਕਤ(ctx);
}

/** callback to start a resource fetch */
ਸਥਿਰ bool fetch_javascript_ਸ਼ੁਰੂ(ਖਾਲੀ *ctx)
{
	ਵਾਪਸ true;
}

/** callback to abort a resource fetch */
ਸਥਿਰ ਖਾਲੀ fetch_javascript_abort(ਖਾਲੀ *ctx)
{
	ਬਣਤਰ fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ਸਥਿਰ ਖਾਲੀ fetch_javascript_poll(lwc_string *scheme)
{
	ਬਣਤਰ fetch_javascript_context *c, *ਅਗਲਾ;

	ਜੇ (ring == ਨਿਰਾ) ਵਾਪਸ;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	ਕਰੋ {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ਜੇ (c->locked == true) {
			ਅਗਲਾ = c->r_ਅਗਲਾ;
			ਜਾਰੀ;
		}

		/* Only process non-aborted fetches */
		ਜੇ (c->aborted == false) {
			/* resource fetches can be processed in one go */
			fetch_javascript_handler(c);
		}

		/* Compute next fetch item at the last possible moment
		 * as processing this item may have added to the ring
		 */
		ਅਗਲਾ = c->r_ਅਗਲਾ;

		fetch_remove_from_queues(c->fetchh);
		fetch_free(c->fetchh);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} ਜਦੋਂ ( (c = ਅਗਲਾ) != ring && ring != ਨਿਰਾ);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_ਰਜਿਸਟਰ(ਖਾਲੀ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	ਅਚਲ ਬਣਤਰ fetcher_operation_ਸਾਰਣੀ fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.ਤਿਆਰੀ = fetch_javascript_ਤਿਆਰੀ,
		.ਸ਼ੁਰੂ = fetch_javascript_ਸ਼ੁਰੂ,
		.abort = fetch_javascript_abort,
		.ਮੁਕਤ = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	ਵਾਪਸ fetcher_ਜੋੜੋ(scheme, &fetcher_ops);
}
