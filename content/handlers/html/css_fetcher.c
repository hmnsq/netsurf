/*
 * Copyright 2008 Rob Kendrick <rjek@netsurf-browser.org>
 * Copyright 2013 John-Mark Bell <jmb@netsurf-browser.org>
 *
 * This file is part of NetSurf.
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

/**
 * \file
 * HTML fetcher for CSS objects
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libwapcaplet/libwapcaplet.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ html_css_fetcher_ਵਸਤੂ {
	uint32_t ਕੁੰਜੀ;
	dom_string *ਅੰਕੜੇ;
	nsurl *ਅਧਾਰ_url;

	ਬਣਤਰ html_css_fetcher_ਵਸਤੂ *r_ਅਗਲਾ, *r_prev;
} html_css_fetcher_ਵਸਤੂ;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ html_css_fetcher_context {
	ਬਣਤਰ fetch *ਮਾਪਾ_fetch;

	nsurl *url;
	html_css_fetcher_ਵਸਤੂ *ਵਸਤੂ;

	bool aborted;
	bool locked;

	ਬਣਤਰ html_css_fetcher_context *r_ਅਗਲਾ, *r_prev;
} html_css_fetcher_context;

ਸਥਿਰ uint32_t current_ਕੁੰਜੀ = 0;
ਸਥਿਰ html_css_fetcher_ਵਸਤੂ *ਚੀਜ਼ਾਂ = ਨਿਰਾ;
ਸਥਿਰ html_css_fetcher_context *ring = ਨਿਰਾ;

ਸਥਿਰ bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_ਅੰਕੜੇ(scheme));
	ਵਾਪਸ true;
}

ਸਥਿਰ ਖਾਲੀ html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_ਅੰਕੜੇ(scheme));
}

ਸਥਿਰ bool html_css_fetcher_can_fetch(ਅਚਲ nsurl *url)
{
	ਵਾਪਸ true;
}

ਸਥਿਰ ਖਾਲੀ *html_css_fetcher_ਤਿਆਰੀ(ਬਣਤਰ fetch *ਮਾਪਾ_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ਅਚਲ ਅੱਖਰ *post_urlenc,
		 ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *post_multipart,
		 ਅਚਲ ਅੱਖਰ **ਸਿਰਲੇਖਾਂ)
{
	html_css_fetcher_context *ctx;
	lwc_string *ਰਸਤਾ;
	uint32_t ਕੁੰਜੀ;
	html_css_fetcher_ਵਸਤੂ *ਵਸਤੂ, *ਮਿਲਿਆ = ਨਿਰਾ;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	ਰਸਤਾ = nsurl_get_component(url, NSURL_ਰਸਤਾ);
	/* The path must exist */
	ਜੇ (ਰਸਤਾ == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਕੁੰਜੀ = strtoul(lwc_string_ਅੰਕੜੇ(ਰਸਤਾ), ਨਿਰਾ, 10);

	lwc_string_unref(ਰਸਤਾ);

	/* There must be at least one item */
	ਜੇ (ਚੀਜ਼ਾਂ == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਸਤੂ = ਚੀਜ਼ਾਂ;
	ਕਰੋ {
		ਜੇ (ਵਸਤੂ->ਕੁੰਜੀ == ਕੁੰਜੀ) {
			ਮਿਲਿਆ = ਵਸਤੂ;
			ਰੋਕੋ;
		}

		ਵਸਤੂ = ਵਸਤੂ->r_ਅਗਲਾ;
	} ਜਦੋਂ (ਵਸਤੂ != ਚੀਜ਼ਾਂ);

	/* We must have found the item */
	ਜੇ (ਮਿਲਿਆ == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ctx = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*ctx));
	ਜੇ (ctx == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ctx->ਮਾਪਾ_fetch = ਮਾਪਾ_fetch;
	ctx->url = nsurl_ref(url);
	ctx->ਵਸਤੂ = ਮਿਲਿਆ;

	RING_ਪਾਓ(ring, ctx);

	ਵਾਪਸ ctx;
}

ਸਥਿਰ bool html_css_fetcher_ਸ਼ੁਰੂ(ਖਾਲੀ *ctx)
{
	ਵਾਪਸ true;
}

ਸਥਿਰ ਖਾਲੀ html_css_fetcher_free(ਖਾਲੀ *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	ਜੇ (c->ਵਸਤੂ != ਨਿਰਾ) {
		nsurl_unref(c->ਵਸਤੂ->ਅਧਾਰ_url);
		dom_string_unref(c->ਵਸਤੂ->ਅੰਕੜੇ);
		RING_REMOVE(ਚੀਜ਼ਾਂ, c->ਵਸਤੂ);
		ਮੁਕਤ(c->ਵਸਤੂ);
	}
	RING_REMOVE(ring, c);
	ਮੁਕਤ(ctx);
}

ਸਥਿਰ ਖਾਲੀ html_css_fetcher_abort(ਖਾਲੀ *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ਸਥਿਰ ਖਾਲੀ html_css_fetcher_send_callback(ਅਚਲ fetch_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(ਸੁਨੇਹਾ, c->ਮਾਪਾ_fetch);
	c->locked = false;
}

ਸਥਿਰ ਖਾਲੀ html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	html_css_fetcher_context *c, *ਅਗਲਾ;

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
		ਜੇ (c->aborted) {
			/* Nothing to do */
			ਪ੍ਰਤਿਜ੍ਞਾ(c->locked == false);
		} ਨਹੀਂਤਾਂ ਜੇ (c->ਵਸਤੂ != ਨਿਰਾ) {
			ਅੱਖਰ ਸਿਰਲੇਖ[4096];

			fetch_ਸੈੱਟ_http_ਕੋਡ(c->ਮਾਪਾ_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(ਸਿਰਲੇਖ, ਆਕਾਰ ਸਿਰਲੇਖ,
				"Content-Type: text/css; charset=utf-8");
			ਸੁਨੇਹਾ.type = FETCH_ਸਿਰਲੇਖ;
			ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *) ਸਿਰਲੇਖ;
			ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਲੜੀ_ਲੰਬਾਈ(ਸਿਰਲੇਖ);
			html_css_fetcher_send_callback(&ਸੁਨੇਹਾ, c);

			ਜੇ (c->aborted == false) {
				snprintf(ਸਿਰਲੇਖ, ਆਕਾਰ ਸਿਰਲੇਖ,
					"Content-Length: %"PRIsizet,
					dom_string_byte_ਲੰਮਾਈ(c->ਵਸਤੂ->ਅੰਕੜੇ));
				ਸੁਨੇਹਾ.type = FETCH_ਸਿਰਲੇਖ;
				ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf =
						(ਅਚਲ uint8_t *) ਸਿਰਲੇਖ;
				ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਲੜੀ_ਲੰਬਾਈ(ਸਿਰਲੇਖ);
				html_css_fetcher_send_callback(&ਸੁਨੇਹਾ, c);
			}

			ਜੇ (c->aborted == false) {
				snprintf(ਸਿਰਲੇਖ, ਆਕਾਰ ਸਿਰਲੇਖ,
					"X-NS-Base: %.*s",
					(ਪੂਰਨ) nsurl_ਲੰਮਾਈ(c->ਵਸਤੂ->ਅਧਾਰ_url),
					nsurl_access(c->ਵਸਤੂ->ਅਧਾਰ_url));
				ਸੁਨੇਹਾ.type = FETCH_ਸਿਰਲੇਖ;
				ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf =
						(ਅਚਲ uint8_t *) ਸਿਰਲੇਖ;
				ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਲੜੀ_ਲੰਬਾਈ(ਸਿਰਲੇਖ);
				html_css_fetcher_send_callback(&ਸੁਨੇਹਾ, c);
			}

			ਜੇ (c->aborted == false) {
				ਸੁਨੇਹਾ.type = FETCH_ਅੰਕੜੇ;
				ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf =
						(ਅਚਲ uint8_t *)
						dom_string_ਅੰਕੜੇ(c->ਵਸਤੂ->ਅੰਕੜੇ);
				ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len =
					dom_string_byte_ਲੰਮਾਈ(c->ਵਸਤੂ->ਅੰਕੜੇ);
				html_css_fetcher_send_callback(&ਸੁਨੇਹਾ, c);
			}

			ਜੇ (c->aborted == false) {
				ਸੁਨੇਹਾ.type = FETCH_ਮੁਕੰਮਲ;
				html_css_fetcher_send_callback(&ਸੁਨੇਹਾ, c);
			}
		} ਨਹੀਂਤਾਂ {
			NSLOG(netsurf, INFO, "Processing of %s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't,
			 * then html_css_fetcher_process() is broken.
			 */
			ਪ੍ਰਤਿਜ੍ਞਾ(c->locked == false);
		}

		/* Compute next fetch item at the last possible moment as
		 * processing this item may have added to the ring.
		 */
		ਅਗਲਾ = c->r_ਅਗਲਾ;

		fetch_remove_from_queues(c->ਮਾਪਾ_fetch);
		fetch_free(c->ਮਾਪਾ_fetch);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} ਜਦੋਂ ( (c = ਅਗਲਾ) != ring && ring != ਨਿਰਾ);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_ਰਜਿਸਟਰ(ਖਾਲੀ)
{
	ਅਚਲ ਬਣਤਰ fetcher_operation_ਸਾਰਣੀ html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.ਤਿਆਰੀ = html_css_fetcher_ਤਿਆਰੀ,
		.ਸ਼ੁਰੂ = html_css_fetcher_ਸ਼ੁਰੂ,
		.abort = html_css_fetcher_abort,
		.ਮੁਕਤ = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	ਵਾਪਸ fetcher_ਜੋੜੋ(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_ਜੋੜੋ_ਵਸਤੂ(dom_string *ਅੰਕੜੇ, nsurl *ਅਧਾਰ_url, uint32_t *ਕੁੰਜੀ)
{
	html_css_fetcher_ਵਸਤੂ *ਵਸਤੂ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(*ਵਸਤੂ));

	ਜੇ (ਵਸਤੂ == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	*ਕੁੰਜੀ = ਵਸਤੂ->ਕੁੰਜੀ = current_ਕੁੰਜੀ++;
	ਵਸਤੂ->ਅੰਕੜੇ = dom_string_ref(ਅੰਕੜੇ);
	ਵਸਤੂ->ਅਧਾਰ_url = nsurl_ref(ਅਧਾਰ_url);

	RING_ਪਾਓ(ਚੀਜ਼ਾਂ, ਵਸਤੂ);

	ਵਾਪਸ NSERROR_OK;
}
