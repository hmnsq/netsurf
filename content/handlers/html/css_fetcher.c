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

#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <dom/dom.h>
#অন্তর্ভাৱ <libwapcaplet/libwapcaplet.h>

#অন্তর্ভাৱ "netsurf/inttypes.h"
#অন্তর্ভাৱ "utils/config.h"
#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/ring.h"
#অন্তর্ভাৱ "utils/nsurl.h"
#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "content/fetch.h"
#অন্তর্ভাৱ "content/fetchers.h"

#অন্তর্ভাৱ "html/private.h"

প্রকারনির্দেশ সংরচনা html_css_fetcher_item {
	uint32_t চাবি;
	dom_string *data;
	nsurl *base_url;

	সংরচনা html_css_fetcher_item *r_next, *r_prev;
} html_css_fetcher_item;

প্রকারনির্দেশ সংরচনা html_css_fetcher_context {
	সংরচনা fetch *parent_fetch;

	nsurl *url;
	html_css_fetcher_item *item;

	bool aborted;
	bool locked;

	সংরচনা html_css_fetcher_context *r_next, *r_prev;
} html_css_fetcher_context;

স্থির uint32_t current_চাবি = 0;
স্থির html_css_fetcher_item *বস্তুসমূহ = রিক্ত;
স্থির html_css_fetcher_context *ring = রিক্ত;

স্থির bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_data(scheme));
	ঘূৰাই true;
}

স্থির শূন্য html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_data(scheme));
}

স্থির bool html_css_fetcher_can_fetch(ধ্রুৱ nsurl *url)
{
	ঘূৰাই true;
}

স্থির শূন্য *html_css_fetcher_setup(সংরচনা fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ধ্রুৱ ৱর্ণ *post_urlenc,
		 ধ্রুৱ সংরচনা fetch_multipart_data *post_multipart,
		 ধ্রুৱ ৱর্ণ **headers)
{
	html_css_fetcher_context *ctx;
	lwc_string *path;
	uint32_t চাবি;
	html_css_fetcher_item *item, *found = রিক্ত;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	path = nsurl_get_component(url, NSURL_PATH);
	/* The path must exist */
	যদি (path == রিক্ত) {
		ঘূৰাই রিক্ত;
	}

	চাবি = strtoul(lwc_string_data(path), রিক্ত, 10);

	lwc_string_unref(path);

	/* There must be at least one item */
	যদি (বস্তুসমূহ == রিক্ত) {
		ঘূৰাই রিক্ত;
	}

	item = বস্তুসমূহ;
	কৰা {
		যদি (item->চাবি == চাবি) {
			found = item;
			ভঙ্গ;
		}

		item = item->r_next;
	} যেতিয়ালৈ (item != বস্তুসমূহ);

	/* We must have found the item */
	যদি (found == রিক্ত) {
		ঘূৰাই রিক্ত;
	}

	ctx = calloc(1, পরিমাণ(*ctx));
	যদি (ctx == রিক্ত)
		ঘূৰাই রিক্ত;

	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);
	ctx->item = found;

	RING_ঢোকাও(ring, ctx);

	ঘূৰাই ctx;
}

স্থির bool html_css_fetcher_start(শূন্য *ctx)
{
	ঘূৰাই true;
}

স্থির শূন্য html_css_fetcher_free(শূন্য *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	যদি (c->item != রিক্ত) {
		nsurl_unref(c->item->base_url);
		dom_string_unref(c->item->data);
		RING_REMOVE(বস্তুসমূহ, c->item);
		মুক্ত(c->item);
	}
	RING_REMOVE(ring, c);
	মুক্ত(ctx);
}

স্থির শূন্য html_css_fetcher_abort(শূন্য *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

স্থির শূন্য html_css_fetcher_send_callback(ধ্রুৱ fetch_বাৰ্তা *বাৰ্তা,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(বাৰ্তা, c->parent_fetch);
	c->locked = false;
}

স্থির শূন্য html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_বাৰ্তা বাৰ্তা;
	html_css_fetcher_context *c, *next;

	যদি (ring == রিক্ত) ঘূৰাই;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	কৰা {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		যদি (c->locked == true) {
			next = c->r_next;
			অনুৱর্ত;
		}

		/* Only process non-aborted fetches */
		যদি (c->aborted) {
			/* Nothing to do */
			প্রতিজ্ঞা(c->locked == false);
		} অন্যথা যদি (c->item != রিক্ত) {
			ৱর্ণ header[4096];

			fetch_set_http_code(c->parent_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(header, পরিমাণ header,
				"Content-Type: text/css; charset=utf-8");
			বাৰ্তা.type = FETCH_HEADER;
			বাৰ্তা.data.header_or_data.buf = (ধ্রুৱ uint8_t *) header;
			বাৰ্তা.data.header_or_data.len = strlen(header);
			html_css_fetcher_send_callback(&বাৰ্তা, c);

			যদি (c->aborted == false) {
				snprintf(header, পরিমাণ header,
					"Content-Length: %"PRIsizet,
					dom_string_byte_length(c->item->data));
				বাৰ্তা.type = FETCH_HEADER;
				বাৰ্তা.data.header_or_data.buf =
						(ধ্রুৱ uint8_t *) header;
				বাৰ্তা.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&বাৰ্তা, c);
			}

			যদি (c->aborted == false) {
				snprintf(header, পরিমাণ header,
					"X-NS-Base: %.*s",
					(পূর্ণাঙ্ক) nsurl_length(c->item->base_url),
					nsurl_access(c->item->base_url));
				বাৰ্তা.type = FETCH_HEADER;
				বাৰ্তা.data.header_or_data.buf =
						(ধ্রুৱ uint8_t *) header;
				বাৰ্তা.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&বাৰ্তা, c);
			}

			যদি (c->aborted == false) {
				বাৰ্তা.type = FETCH_DATA;
				বাৰ্তা.data.header_or_data.buf =
						(ধ্রুৱ uint8_t *)
						dom_string_data(c->item->data);
				বাৰ্তা.data.header_or_data.len =
					dom_string_byte_length(c->item->data);
				html_css_fetcher_send_callback(&বাৰ্তা, c);
			}

			যদি (c->aborted == false) {
				বাৰ্তা.type = FETCH_FINISHED;
				html_css_fetcher_send_callback(&বাৰ্তা, c);
			}
		} অন্যথা {
			NSLOG(netsurf, INFO, "Processing of %s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't,
			 * then html_css_fetcher_process() is broken.
			 */
			প্রতিজ্ঞা(c->locked == false);
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
	} যেতিয়ালৈ ( (c = next) != ring && ring != রিক্ত);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_register(শূন্য)
{
	ধ্রুৱ সংরচনা fetcher_operation_table html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.setup = html_css_fetcher_setup,
		.start = html_css_fetcher_start,
		.abort = html_css_fetcher_abort,
		.মুক্ত = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	ঘূৰাই fetcher_add(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_add_item(dom_string *data, nsurl *base_url, uint32_t *চাবি)
{
	html_css_fetcher_item *item = malloc(পরিমাণ(*item));

	যদি (item == রিক্ত) {
		ঘূৰাই NSERROR_NOMEM;
	}

	*চাবি = item->চাবি = current_চাবি++;
	item->data = dom_string_ref(data);
	item->base_url = nsurl_ref(base_url);

	RING_ঢোকাও(বস্তুসমূহ, item);

	ঘূৰাই NSERROR_OK;
}
