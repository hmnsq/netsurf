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

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ html_css_fetcher_item {
	uint32_t ಕೀಲಿ;
	dom_string *data;
	nsurl *base_url;

	ಸಂರಚನಾ html_css_fetcher_item *r_next, *r_prev;
} html_css_fetcher_item;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ html_css_fetcher_context {
	ಸಂರಚನಾ fetch *parent_fetch;

	nsurl *url;
	html_css_fetcher_item *item;

	bool aborted;
	bool locked;

	ಸಂರಚನಾ html_css_fetcher_context *r_next, *r_prev;
} html_css_fetcher_context;

ಸ್ಥಿರ uint32_t current_ಕೀಲಿ = 0;
ಸ್ಥಿರ html_css_fetcher_item *ವಸ್ತುಗಳು = ರಿಕ್ತ;
ಸ್ಥಿರ html_css_fetcher_context *ring = ರಿಕ್ತ;

ಸ್ಥಿರ bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_data(scheme));
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_data(scheme));
}

ಸ್ಥಿರ bool html_css_fetcher_can_fetch(ಧ್ರುವ nsurl *url)
{
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ *html_css_fetcher_setup(ಸಂರಚನಾ fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ಧ್ರುವ ವರ್ಣ *post_urlenc,
		 ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *post_multipart,
		 ಧ್ರುವ ವರ್ಣ **headers)
{
	html_css_fetcher_context *ctx;
	lwc_string *path;
	uint32_t ಕೀಲಿ;
	html_css_fetcher_item *item, *found = ರಿಕ್ತ;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	path = nsurl_get_component(url, NSURL_PATH);
	/* The path must exist */
	ಯದಿ (path == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಕೀಲಿ = strtoul(lwc_string_data(path), ರಿಕ್ತ, 10);

	lwc_string_unref(path);

	/* There must be at least one item */
	ಯದಿ (ವಸ್ತುಗಳು == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	item = ವಸ್ತುಗಳು;
	ಕುರು {
		ಯದಿ (item->ಕೀಲಿ == ಕೀಲಿ) {
			found = item;
			ಭಙ್ಗ;
		}

		item = item->r_next;
	} ಯಾವತ್ (item != ವಸ್ತುಗಳು);

	/* We must have found the item */
	ಯದಿ (found == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ctx = calloc(1, ಪರಿಮಾಣ(*ctx));
	ಯದಿ (ctx == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);
	ctx->item = found;

	RING_ಸೇರಿಸು(ring, ctx);

	ಪ್ರತ್ಯಾಗಮ ctx;
}

ಸ್ಥಿರ bool html_css_fetcher_start(ಶೂನ್ಯ *ctx)
{
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ html_css_fetcher_free(ಶೂನ್ಯ *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	ಯದಿ (c->item != ರಿಕ್ತ) {
		nsurl_unref(c->item->base_url);
		dom_string_unref(c->item->data);
		RING_REMOVE(ವಸ್ತುಗಳು, c->item);
		ಮುಕ್ತ(c->item);
	}
	RING_REMOVE(ring, c);
	ಮುಕ್ತ(ctx);
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

ಸ್ಥಿರ ಶೂನ್ಯ html_css_fetcher_send_callback(ಧ್ರುವ fetch_ಸಂದೇಶ *ಸಂದೇಶ,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(ಸಂದೇಶ, c->parent_fetch);
	c->locked = false;
}

ಸ್ಥಿರ ಶೂನ್ಯ html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	html_css_fetcher_context *c, *next;

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
		ಯದಿ (c->aborted) {
			/* Nothing to do */
			ಪ್ರತಿಜ್ಞಾ(c->locked == false);
		} ಅನ್ಯಥಾ ಯದಿ (c->item != ರಿಕ್ತ) {
			ವರ್ಣ header[4096];

			fetch_set_http_code(c->parent_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(header, ಪರಿಮಾಣ header,
				"Content-Type: text/css; charset=utf-8");
			ಸಂದೇಶ.type = FETCH_HEADER;
			ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *) header;
			ಸಂದೇಶ.data.header_or_data.len = strlen(header);
			html_css_fetcher_send_callback(&ಸಂದೇಶ, c);

			ಯದಿ (c->aborted == false) {
				snprintf(header, ಪರಿಮಾಣ header,
					"Content-Length: %"PRIsizet,
					dom_string_byte_length(c->item->data));
				ಸಂದೇಶ.type = FETCH_HEADER;
				ಸಂದೇಶ.data.header_or_data.buf =
						(ಧ್ರುವ uint8_t *) header;
				ಸಂದೇಶ.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&ಸಂದೇಶ, c);
			}

			ಯದಿ (c->aborted == false) {
				snprintf(header, ಪರಿಮಾಣ header,
					"X-NS-Base: %.*s",
					(ಪೂರ್ಣಾಙ್ಕ) nsurl_length(c->item->base_url),
					nsurl_access(c->item->base_url));
				ಸಂದೇಶ.type = FETCH_HEADER;
				ಸಂದೇಶ.data.header_or_data.buf =
						(ಧ್ರುವ uint8_t *) header;
				ಸಂದೇಶ.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&ಸಂದೇಶ, c);
			}

			ಯದಿ (c->aborted == false) {
				ಸಂದೇಶ.type = FETCH_DATA;
				ಸಂದೇಶ.data.header_or_data.buf =
						(ಧ್ರುವ uint8_t *)
						dom_string_data(c->item->data);
				ಸಂದೇಶ.data.header_or_data.len =
					dom_string_byte_length(c->item->data);
				html_css_fetcher_send_callback(&ಸಂದೇಶ, c);
			}

			ಯದಿ (c->aborted == false) {
				ಸಂದೇಶ.type = FETCH_FINISHED;
				html_css_fetcher_send_callback(&ಸಂದೇಶ, c);
			}
		} ಅನ್ಯಥಾ {
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
		next = c->r_next;

		fetch_remove_from_queues(c->parent_fetch);
		fetch_free(c->parent_fetch);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} ಯಾವತ್ ( (c = next) != ring && ring != ರಿಕ್ತ);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_register(ಶೂನ್ಯ)
{
	ಧ್ರುವ ಸಂರಚನಾ fetcher_operation_table html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.setup = html_css_fetcher_setup,
		.start = html_css_fetcher_start,
		.abort = html_css_fetcher_abort,
		.ಮುಕ್ತ = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	ಪ್ರತ್ಯಾಗಮ fetcher_add(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_add_item(dom_string *data, nsurl *base_url, uint32_t *ಕೀಲಿ)
{
	html_css_fetcher_item *item = malloc(ಪರಿಮಾಣ(*item));

	ಯದಿ (item == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	*ಕೀಲಿ = item->ಕೀಲಿ = current_ಕೀಲಿ++;
	item->data = dom_string_ref(data);
	item->base_url = nsurl_ref(base_url);

	RING_ಸೇರಿಸು(ವಸ್ತುಗಳು, item);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}
