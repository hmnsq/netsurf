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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <dom/dom.h>
#அந்தர்பாவ <libwapcaplet/libwapcaplet.h>

#அந்தர்பாவ "netsurf/inttypes.h"
#அந்தர்பாவ "utils/config.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/ring.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "content/fetch.h"
#அந்தர்பாவ "content/fetchers.h"

#அந்தர்பாவ "html/private.h"

வகைவரை கட்டமைப்பு html_css_fetcher_item {
	uint32_t சாவி;
	dom_string *data;
	nsurl *base_url;

	கட்டமைப்பு html_css_fetcher_item *r_next, *r_prev;
} html_css_fetcher_item;

வகைவரை கட்டமைப்பு html_css_fetcher_context {
	கட்டமைப்பு fetch *parent_fetch;

	nsurl *url;
	html_css_fetcher_item *item;

	bool aborted;
	bool locked;

	கட்டமைப்பு html_css_fetcher_context *r_next, *r_prev;
} html_css_fetcher_context;

நிலையான uint32_t current_சாவி = 0;
நிலையான html_css_fetcher_item *பொருட்கள் = வெற்று;
நிலையான html_css_fetcher_context *ring = வெற்று;

நிலையான bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_data(scheme));
	திரும்பு true;
}

நிலையான வெறுமை html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_data(scheme));
}

நிலையான bool html_css_fetcher_can_fetch(மாறிலி nsurl *url)
{
	திரும்பு true;
}

நிலையான வெறுமை *html_css_fetcher_setup(கட்டமைப்பு fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, மாறிலி எழுத்து *post_urlenc,
		 மாறிலி கட்டமைப்பு fetch_multipart_data *post_multipart,
		 மாறிலி எழுத்து **headers)
{
	html_css_fetcher_context *ctx;
	lwc_string *path;
	uint32_t சாவி;
	html_css_fetcher_item *item, *found = வெற்று;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	path = nsurl_get_component(url, NSURL_PATH);
	/* The path must exist */
	எனில் (path == வெற்று) {
		திரும்பு வெற்று;
	}

	சாவி = strtoul(lwc_string_data(path), வெற்று, 10);

	lwc_string_unref(path);

	/* There must be at least one item */
	எனில் (பொருட்கள் == வெற்று) {
		திரும்பு வெற்று;
	}

	item = பொருட்கள்;
	செய் {
		எனில் (item->சாவி == சாவி) {
			found = item;
			நிறுத்து;
		}

		item = item->r_next;
	} போது (item != பொருட்கள்);

	/* We must have found the item */
	எனில் (found == வெற்று) {
		திரும்பு வெற்று;
	}

	ctx = calloc(1, அளவு(*ctx));
	எனில் (ctx == வெற்று)
		திரும்பு வெற்று;

	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);
	ctx->item = found;

	RING_செருகு(ring, ctx);

	திரும்பு ctx;
}

நிலையான bool html_css_fetcher_start(வெறுமை *ctx)
{
	திரும்பு true;
}

நிலையான வெறுமை html_css_fetcher_free(வெறுமை *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	எனில் (c->item != வெற்று) {
		nsurl_unref(c->item->base_url);
		dom_string_unref(c->item->data);
		RING_REMOVE(பொருட்கள், c->item);
		விடுவி(c->item);
	}
	RING_REMOVE(ring, c);
	விடுவி(ctx);
}

நிலையான வெறுமை html_css_fetcher_abort(வெறுமை *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

நிலையான வெறுமை html_css_fetcher_send_callback(மாறிலி fetch_செய்தி *செய்தி,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(செய்தி, c->parent_fetch);
	c->locked = false;
}

நிலையான வெறுமை html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_செய்தி செய்தி;
	html_css_fetcher_context *c, *next;

	எனில் (ring == வெற்று) திரும்பு;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	செய் {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		எனில் (c->locked == true) {
			next = c->r_next;
			தொடர்;
		}

		/* Only process non-aborted fetches */
		எனில் (c->aborted) {
			/* Nothing to do */
			ப்ரதிஜ்ஞா(c->locked == false);
		} இல்லையெனில் எனில் (c->item != வெற்று) {
			எழுத்து header[4096];

			fetch_set_http_code(c->parent_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(header, அளவு header,
				"Content-Type: text/css; charset=utf-8");
			செய்தி.type = FETCH_HEADER;
			செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *) header;
			செய்தி.data.header_or_data.len = strlen(header);
			html_css_fetcher_send_callback(&செய்தி, c);

			எனில் (c->aborted == false) {
				snprintf(header, அளவு header,
					"Content-Length: %"PRIsizet,
					dom_string_byte_length(c->item->data));
				செய்தி.type = FETCH_HEADER;
				செய்தி.data.header_or_data.buf =
						(மாறிலி uint8_t *) header;
				செய்தி.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&செய்தி, c);
			}

			எனில் (c->aborted == false) {
				snprintf(header, அளவு header,
					"X-NS-Base: %.*s",
					(முழு) nsurl_length(c->item->base_url),
					nsurl_access(c->item->base_url));
				செய்தி.type = FETCH_HEADER;
				செய்தி.data.header_or_data.buf =
						(மாறிலி uint8_t *) header;
				செய்தி.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&செய்தி, c);
			}

			எனில் (c->aborted == false) {
				செய்தி.type = FETCH_DATA;
				செய்தி.data.header_or_data.buf =
						(மாறிலி uint8_t *)
						dom_string_data(c->item->data);
				செய்தி.data.header_or_data.len =
					dom_string_byte_length(c->item->data);
				html_css_fetcher_send_callback(&செய்தி, c);
			}

			எனில் (c->aborted == false) {
				செய்தி.type = FETCH_FINISHED;
				html_css_fetcher_send_callback(&செய்தி, c);
			}
		} இல்லையெனில் {
			NSLOG(netsurf, INFO, "Processing of %s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't,
			 * then html_css_fetcher_process() is broken.
			 */
			ப்ரதிஜ்ஞா(c->locked == false);
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
	} போது ( (c = next) != ring && ring != வெற்று);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_register(வெறுமை)
{
	மாறிலி கட்டமைப்பு fetcher_operation_table html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.setup = html_css_fetcher_setup,
		.start = html_css_fetcher_start,
		.abort = html_css_fetcher_abort,
		.விடுவி = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	திரும்பு fetcher_add(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_add_item(dom_string *data, nsurl *base_url, uint32_t *சாவி)
{
	html_css_fetcher_item *item = malloc(அளவு(*item));

	எனில் (item == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	*சாவி = item->சாவி = current_சாவி++;
	item->data = dom_string_ref(data);
	item->base_url = nsurl_ref(base_url);

	RING_செருகு(பொருட்கள், item);

	திரும்பு NSERROR_OK;
}
