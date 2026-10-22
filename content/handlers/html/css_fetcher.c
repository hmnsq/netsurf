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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <dom/dom.h>
#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "utils/config.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/ring.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers.h"

#ಅನ್ತರ್ಭಾವ "html/private.h"

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ html_css_fetcher_ಅಂಶ {
	uint32_t ಕೀಲಿ;
	dom_string *ದತ್ತಾಂಶ;
	nsurl *ತಳ_url;

	ರಚನೆ html_css_fetcher_ಅಂಶ *r_ಮುಂದಿನ, *r_prev;
} html_css_fetcher_ಅಂಶ;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ html_css_fetcher_context {
	ರಚನೆ fetch *ಪೋಷಕ_fetch;

	nsurl *url;
	html_css_fetcher_ಅಂಶ *ಅಂಶ;

	bool aborted;
	bool locked;

	ರಚನೆ html_css_fetcher_context *r_ಮುಂದಿನ, *r_prev;
} html_css_fetcher_context;

ಸ್ಥಿರ uint32_t current_ಕೀಲಿ = 0;
ಸ್ಥಿರ html_css_fetcher_ಅಂಶ *ವಸ್ತುಗಳು = ಖಾಲಿ;
ಸ್ಥಿರ html_css_fetcher_context *ring = ಖಾಲಿ;

ಸ್ಥಿರ bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_ದತ್ತಾಂಶ(scheme));
	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_ದತ್ತಾಂಶ(scheme));
}

ಸ್ಥಿರ bool html_css_fetcher_can_fetch(ಸ್ಥಿರಾಂಕ nsurl *url)
{
	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ *html_css_fetcher_ಸಿದ್ಧತೆ(ರಚನೆ fetch *ಪೋಷಕ_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *post_urlenc,
		 ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *post_multipart,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಶೀರ್ಷಿಕೆಗಳು)
{
	html_css_fetcher_context *ctx;
	lwc_string *ಮಾರ್ಗ;
	uint32_t ಕೀಲಿ;
	html_css_fetcher_ಅಂಶ *ಅಂಶ, *ಸಿಕ್ಕಿತು = ಖಾಲಿ;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	ಮಾರ್ಗ = nsurl_get_component(url, NSURL_ಮಾರ್ಗ);
	/* The path must exist */
	ಆದರೆ (ಮಾರ್ಗ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಕೀಲಿ = strtoul(lwc_string_ದತ್ತಾಂಶ(ಮಾರ್ಗ), ಖಾಲಿ, 10);

	lwc_string_unref(ಮಾರ್ಗ);

	/* There must be at least one item */
	ಆದರೆ (ವಸ್ತುಗಳು == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಅಂಶ = ವಸ್ತುಗಳು;
	ಮಾಡು {
		ಆದರೆ (ಅಂಶ->ಕೀಲಿ == ಕೀಲಿ) {
			ಸಿಕ್ಕಿತು = ಅಂಶ;
			ನಿಲ್ಲಿಸು;
		}

		ಅಂಶ = ಅಂಶ->r_ಮುಂದಿನ;
	} ಆಗ (ಅಂಶ != ವಸ್ತುಗಳು);

	/* We must have found the item */
	ಆದರೆ (ಸಿಕ್ಕಿತು == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ctx = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*ctx));
	ಆದರೆ (ctx == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ctx->ಪೋಷಕ_fetch = ಪೋಷಕ_fetch;
	ctx->url = nsurl_ref(url);
	ctx->ಅಂಶ = ಸಿಕ್ಕಿತು;

	RING_ಸೇರಿಸು(ring, ctx);

	ಹಿಂತಿರುಗಿ ctx;
}

ಸ್ಥಿರ bool html_css_fetcher_ಆರಂಭ(ಶೂನ್ಯ *ctx)
{
	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ html_css_fetcher_free(ಶೂನ್ಯ *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	ಆದರೆ (c->ಅಂಶ != ಖಾಲಿ) {
		nsurl_unref(c->ಅಂಶ->ತಳ_url);
		dom_string_unref(c->ಅಂಶ->ದತ್ತಾಂಶ);
		RING_REMOVE(ವಸ್ತುಗಳು, c->ಅಂಶ);
		ಬಿಡುಗಡೆ(c->ಅಂಶ);
	}
	RING_REMOVE(ring, c);
	ಬಿಡುಗಡೆ(ctx);
}

ಸ್ಥಿರ ಶೂನ್ಯ html_css_fetcher_abort(ಶೂನ್ಯ *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ಸ್ಥಿರ ಶೂನ್ಯ html_css_fetcher_send_callback(ಸ್ಥಿರಾಂಕ fetch_ಸಂದೇಶ *ಸಂದೇಶ,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(ಸಂದೇಶ, c->ಪೋಷಕ_fetch);
	c->locked = false;
}

ಸ್ಥಿರ ಶೂನ್ಯ html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	html_css_fetcher_context *c, *ಮುಂದಿನ;

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
		ಆದರೆ (c->aborted) {
			/* Nothing to do */
			ಪ್ರತಿಜ್ಞಾ(c->locked == false);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c->ಅಂಶ != ಖಾಲಿ) {
			ಅಕ್ಷರ ಶೀರ್ಷಿಕೆ[4096];

			fetch_ಗಣ_http_ಸಂಕೇತ(c->ಪೋಷಕ_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(ಶೀರ್ಷಿಕೆ, ಗಾತ್ರ ಶೀರ್ಷಿಕೆ,
				"Content-Type: text/css; charset=utf-8");
			ಸಂದೇಶ.type = FETCH_ಶೀರ್ಷಿಕೆ;
			ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *) ಶೀರ್ಷಿಕೆ;
			ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಶೀರ್ಷಿಕೆ);
			html_css_fetcher_send_callback(&ಸಂದೇಶ, c);

			ಆದರೆ (c->aborted == false) {
				snprintf(ಶೀರ್ಷಿಕೆ, ಗಾತ್ರ ಶೀರ್ಷಿಕೆ,
					"Content-Length: %"PRIsizet,
					dom_string_byte_ಉದ್ದಳತೆ(c->ಅಂಶ->ದತ್ತಾಂಶ));
				ಸಂದೇಶ.type = FETCH_ಶೀರ್ಷಿಕೆ;
				ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf =
						(ಸ್ಥಿರಾಂಕ uint8_t *) ಶೀರ್ಷಿಕೆ;
				ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಶೀರ್ಷಿಕೆ);
				html_css_fetcher_send_callback(&ಸಂದೇಶ, c);
			}

			ಆದರೆ (c->aborted == false) {
				snprintf(ಶೀರ್ಷಿಕೆ, ಗಾತ್ರ ಶೀರ್ಷಿಕೆ,
					"X-NS-Base: %.*s",
					(ಪೂರ್ಣಾಂಕ) nsurl_ಉದ್ದಳತೆ(c->ಅಂಶ->ತಳ_url),
					nsurl_access(c->ಅಂಶ->ತಳ_url));
				ಸಂದೇಶ.type = FETCH_ಶೀರ್ಷಿಕೆ;
				ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf =
						(ಸ್ಥಿರಾಂಕ uint8_t *) ಶೀರ್ಷಿಕೆ;
				ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಶೀರ್ಷಿಕೆ);
				html_css_fetcher_send_callback(&ಸಂದೇಶ, c);
			}

			ಆದರೆ (c->aborted == false) {
				ಸಂದೇಶ.type = FETCH_ದತ್ತಾಂಶ;
				ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf =
						(ಸ್ಥಿರಾಂಕ uint8_t *)
						dom_string_ದತ್ತಾಂಶ(c->ಅಂಶ->ದತ್ತಾಂಶ);
				ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len =
					dom_string_byte_ಉದ್ದಳತೆ(c->ಅಂಶ->ದತ್ತಾಂಶ);
				html_css_fetcher_send_callback(&ಸಂದೇಶ, c);
			}

			ಆದರೆ (c->aborted == false) {
				ಸಂದೇಶ.type = FETCH_ಪೂರ್ಣಗೊಂಡ;
				html_css_fetcher_send_callback(&ಸಂದೇಶ, c);
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			NSLOG(netsurf, INFO, "Processing of %s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't,
			 * then html_css_fetcher_process() is broken.
			 */
			ಪ್ರತಿಜ್ಞಾ(c->locked == false);
		}

		/* Compute next fetch item at the last possible moment as
		 * processing this item may have added to the ring.
		 */
		ಮುಂದಿನ = c->r_ಮುಂದಿನ;

		fetch_remove_from_queues(c->ಪೋಷಕ_fetch);
		fetch_free(c->ಪೋಷಕ_fetch);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} ಆಗ ( (c = ಮುಂದಿನ) != ring && ring != ಖಾಲಿ);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_ರಿಜಿಸ್ಟರ್(ಶೂನ್ಯ)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ fetcher_operation_ಕೋಷ್ಟಕ html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.ಸಿದ್ಧತೆ = html_css_fetcher_ಸಿದ್ಧತೆ,
		.ಆರಂಭ = html_css_fetcher_ಆರಂಭ,
		.abort = html_css_fetcher_abort,
		.ಬಿಡುಗಡೆ = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	ಹಿಂತಿರುಗಿ fetcher_ಕೂಡಿಸು(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_ಕೂಡಿಸು_ಅಂಶ(dom_string *ದತ್ತಾಂಶ, nsurl *ತಳ_url, uint32_t *ಕೀಲಿ)
{
	html_css_fetcher_ಅಂಶ *ಅಂಶ = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(*ಅಂಶ));

	ಆದರೆ (ಅಂಶ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	*ಕೀಲಿ = ಅಂಶ->ಕೀಲಿ = current_ಕೀಲಿ++;
	ಅಂಶ->ದತ್ತಾಂಶ = dom_string_ref(ದತ್ತಾಂಶ);
	ಅಂಶ->ತಳ_url = nsurl_ref(ತಳ_url);

	RING_ಸೇರಿಸು(ವಸ್ತುಗಳು, ಅಂಶ);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}
