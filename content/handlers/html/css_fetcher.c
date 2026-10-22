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

#شامل <assert.h>
#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <dom/dom.h>
#شامل <libwapcaplet/libwapcaplet.h>

#شامل "netsurf/inttypes.h"
#شامل "utils/config.h"
#شامل "utils/corestrings.h"
#شامل "utils/log.h"
#شامل "utils/ring.h"
#شامل "utils/nsurl.h"
#شامل "utils/utils.h"
#شامل "content/fetch.h"
#شامل "content/fetchers.h"

#شامل "html/private.h"

تعریفنوع ساختار html_css_fetcher_item {
	uint32_t کلید;
	dom_string *data;
	nsurl *base_url;

	ساختار html_css_fetcher_item *r_next, *r_prev;
} html_css_fetcher_item;

تعریفنوع ساختار html_css_fetcher_context {
	ساختار fetch *parent_fetch;

	nsurl *url;
	html_css_fetcher_item *item;

	bool aborted;
	bool locked;

	ساختار html_css_fetcher_context *r_next, *r_prev;
} html_css_fetcher_context;

ایستا uint32_t current_کلید = 0;
ایستا html_css_fetcher_item *اقلام = پوچ;
ایستا html_css_fetcher_context *ring = پوچ;

ایستا bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_data(scheme));
	بازگشت true;
}

ایستا تهی html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_data(scheme));
}

ایستا bool html_css_fetcher_can_fetch(ثابت nsurl *url)
{
	بازگشت true;
}

ایستا تهی *html_css_fetcher_setup(ساختار fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ثابت نویسه *post_urlenc,
		 ثابت ساختار fetch_multipart_data *post_multipart,
		 ثابت نویسه **headers)
{
	html_css_fetcher_context *ctx;
	lwc_string *path;
	uint32_t کلید;
	html_css_fetcher_item *item, *found = پوچ;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	path = nsurl_get_component(url, NSURL_PATH);
	/* The path must exist */
	اگر (path == پوچ) {
		بازگشت پوچ;
	}

	کلید = strtoul(lwc_string_data(path), پوچ, 10);

	lwc_string_unref(path);

	/* There must be at least one item */
	اگر (اقلام == پوچ) {
		بازگشت پوچ;
	}

	item = اقلام;
	انجامبده {
		اگر (item->کلید == کلید) {
			found = item;
			بشکن;
		}

		item = item->r_next;
	} وقتی (item != اقلام);

	/* We must have found the item */
	اگر (found == پوچ) {
		بازگشت پوچ;
	}

	ctx = calloc(1, اندازه(*ctx));
	اگر (ctx == پوچ)
		بازگشت پوچ;

	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);
	ctx->item = found;

	RING_درج(ring, ctx);

	بازگشت ctx;
}

ایستا bool html_css_fetcher_start(تهی *ctx)
{
	بازگشت true;
}

ایستا تهی html_css_fetcher_free(تهی *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	اگر (c->item != پوچ) {
		nsurl_unref(c->item->base_url);
		dom_string_unref(c->item->data);
		RING_REMOVE(اقلام, c->item);
		آزاد(c->item);
	}
	RING_REMOVE(ring, c);
	آزاد(ctx);
}

ایستا تهی html_css_fetcher_abort(تهی *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ایستا تهی html_css_fetcher_send_callback(ثابت fetch_پیام *پیام,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(پیام, c->parent_fetch);
	c->locked = false;
}

ایستا تهی html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_پیام پیام;
	html_css_fetcher_context *c, *next;

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
		اگر (c->aborted) {
			/* Nothing to do */
			أكد(c->locked == false);
		} وگرنه اگر (c->item != پوچ) {
			نویسه header[4096];

			fetch_set_http_code(c->parent_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(header, اندازه header,
				"Content-Type: text/css; charset=utf-8");
			پیام.type = FETCH_HEADER;
			پیام.data.header_or_data.buf = (ثابت uint8_t *) header;
			پیام.data.header_or_data.len = strlen(header);
			html_css_fetcher_send_callback(&پیام, c);

			اگر (c->aborted == false) {
				snprintf(header, اندازه header,
					"Content-Length: %"PRIsizet,
					dom_string_byte_length(c->item->data));
				پیام.type = FETCH_HEADER;
				پیام.data.header_or_data.buf =
						(ثابت uint8_t *) header;
				پیام.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&پیام, c);
			}

			اگر (c->aborted == false) {
				snprintf(header, اندازه header,
					"X-NS-Base: %.*s",
					(صحیح) nsurl_length(c->item->base_url),
					nsurl_access(c->item->base_url));
				پیام.type = FETCH_HEADER;
				پیام.data.header_or_data.buf =
						(ثابت uint8_t *) header;
				پیام.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&پیام, c);
			}

			اگر (c->aborted == false) {
				پیام.type = FETCH_DATA;
				پیام.data.header_or_data.buf =
						(ثابت uint8_t *)
						dom_string_data(c->item->data);
				پیام.data.header_or_data.len =
					dom_string_byte_length(c->item->data);
				html_css_fetcher_send_callback(&پیام, c);
			}

			اگر (c->aborted == false) {
				پیام.type = FETCH_FINISHED;
				html_css_fetcher_send_callback(&پیام, c);
			}
		} وگرنه {
			NSLOG(netsurf, INFO, "Processing of %s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't,
			 * then html_css_fetcher_process() is broken.
			 */
			أكد(c->locked == false);
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
	} وقتی ( (c = next) != ring && ring != پوچ);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_register(تهی)
{
	ثابت ساختار fetcher_operation_table html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.setup = html_css_fetcher_setup,
		.start = html_css_fetcher_start,
		.abort = html_css_fetcher_abort,
		.آزاد = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	بازگشت fetcher_add(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_add_item(dom_string *data, nsurl *base_url, uint32_t *کلید)
{
	html_css_fetcher_item *item = malloc(اندازه(*item));

	اگر (item == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	*کلید = item->کلید = current_کلید++;
	item->data = dom_string_ref(data);
	item->base_url = nsurl_ref(base_url);

	RING_درج(اقلام, item);

	بازگشت NSERROR_OK;
}
