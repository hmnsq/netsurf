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

#ضمن <assert.h>
#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <dom/dom.h>
#ضمن <libwapcaplet/libwapcaplet.h>

#ضمن "netsurf/inttypes.h"
#ضمن "utils/config.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"
#ضمن "utils/ring.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/utils.h"
#ضمن "content/fetch.h"
#ضمن "content/fetchers.h"

#ضمن "html/private.h"

تعريفنوع بنية html_css_fetcher_item {
	uint32_t key;
	dom_string *data;
	nsurl *base_url;

	بنية html_css_fetcher_item *r_next, *r_prev;
} html_css_fetcher_item;

تعريفنوع بنية html_css_fetcher_context {
	بنية fetch *parent_fetch;

	nsurl *url;
	html_css_fetcher_item *item;

	bool aborted;
	bool locked;

	بنية html_css_fetcher_context *r_next, *r_prev;
} html_css_fetcher_context;

ساكن uint32_t current_key = 0;
ساكن html_css_fetcher_item *items = عدم;
ساكن html_css_fetcher_context *ring = عدم;

ساكن bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_data(scheme));
	ܦܢܐ true;
}

ساكن فراغ html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_data(scheme));
}

ساكن bool html_css_fetcher_can_fetch(ثابت nsurl *url)
{
	ܦܢܐ true;
}

ساكن فراغ *html_css_fetcher_setup(بنية fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_data *post_multipart,
		 ثابت حرف **headers)
{
	html_css_fetcher_context *ctx;
	lwc_string *path;
	uint32_t key;
	html_css_fetcher_item *item, *found = عدم;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	path = nsurl_get_component(url, NSURL_PATH);
	/* The path must exist */
	ܐܢ (path == عدم) {
		ܦܢܐ عدم;
	}

	key = strtoul(lwc_string_data(path), عدم, 10);

	lwc_string_unref(path);

	/* There must be at least one item */
	ܐܢ (items == عدم) {
		ܦܢܐ عدم;
	}

	item = items;
	ܥܒܕ {
		ܐܢ (item->key == key) {
			found = item;
			اكسر;
		}

		item = item->r_next;
	} ܟܡܐ (item != items);

	/* We must have found the item */
	ܐܢ (found == عدم) {
		ܦܢܐ عدم;
	}

	ctx = calloc(1, حجم(*ctx));
	ܐܢ (ctx == عدم)
		ܦܢܐ عدم;

	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);
	ctx->item = found;

	RING_INSERT(ring, ctx);

	ܦܢܐ ctx;
}

ساكن bool html_css_fetcher_start(فراغ *ctx)
{
	ܦܢܐ true;
}

ساكن فراغ html_css_fetcher_free(فراغ *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	ܐܢ (c->item != عدم) {
		nsurl_unref(c->item->base_url);
		dom_string_unref(c->item->data);
		RING_REMOVE(items, c->item);
		حرر(c->item);
	}
	RING_REMOVE(ring, c);
	حرر(ctx);
}

ساكن فراغ html_css_fetcher_abort(فراغ *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ساكن فراغ html_css_fetcher_send_callback(ثابت fetch_msg *msg,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(msg, c->parent_fetch);
	c->locked = false;
}

ساكن فراغ html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_msg msg;
	html_css_fetcher_context *c, *next;

	ܐܢ (ring == عدم) ܦܢܐ;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	ܥܒܕ {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ܐܢ (c->locked == true) {
			next = c->r_next;
			استمر;
		}

		/* Only process non-aborted fetches */
		ܐܢ (c->aborted) {
			/* Nothing to do */
			أكد(c->locked == false);
		} ܐܚܪܢܐ ܐܢ (c->item != عدم) {
			حرف header[4096];

			fetch_set_http_code(c->parent_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(header, حجم header,
				"Content-Type: text/css; charset=utf-8");
			msg.type = FETCH_HEADER;
			msg.data.header_or_data.buf = (ثابت uint8_t *) header;
			msg.data.header_or_data.len = strlen(header);
			html_css_fetcher_send_callback(&msg, c);

			ܐܢ (c->aborted == false) {
				snprintf(header, حجم header,
					"Content-Length: %"PRIsizet,
					dom_string_byte_length(c->item->data));
				msg.type = FETCH_HEADER;
				msg.data.header_or_data.buf =
						(ثابت uint8_t *) header;
				msg.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&msg, c);
			}

			ܐܢ (c->aborted == false) {
				snprintf(header, حجم header,
					"X-NS-Base: %.*s",
					(صحيح) nsurl_length(c->item->base_url),
					nsurl_access(c->item->base_url));
				msg.type = FETCH_HEADER;
				msg.data.header_or_data.buf =
						(ثابت uint8_t *) header;
				msg.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&msg, c);
			}

			ܐܢ (c->aborted == false) {
				msg.type = FETCH_DATA;
				msg.data.header_or_data.buf =
						(ثابت uint8_t *)
						dom_string_data(c->item->data);
				msg.data.header_or_data.len =
					dom_string_byte_length(c->item->data);
				html_css_fetcher_send_callback(&msg, c);
			}

			ܐܢ (c->aborted == false) {
				msg.type = FETCH_FINISHED;
				html_css_fetcher_send_callback(&msg, c);
			}
		} ܐܚܪܢܐ {
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
	} ܟܡܐ ( (c = next) != ring && ring != عدم);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_register(فراغ)
{
	ثابت بنية fetcher_operation_table html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.setup = html_css_fetcher_setup,
		.start = html_css_fetcher_start,
		.abort = html_css_fetcher_abort,
		.حرر = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	ܦܢܐ fetcher_add(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_add_item(dom_string *data, nsurl *base_url, uint32_t *key)
{
	html_css_fetcher_item *item = malloc(حجم(*item));

	ܐܢ (item == عدم) {
		ܦܢܐ NSERROR_NOMEM;
	}

	*key = item->key = current_key++;
	item->data = dom_string_ref(data);
	item->base_url = nsurl_ref(base_url);

	RING_INSERT(items, item);

	ܦܢܐ NSERROR_OK;
}
