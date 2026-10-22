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

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dom/dom.h>
#include <libwapcaplet/libwapcaplet.h>

#include "netsurf/inttypes.h"
#include "utils/config.h"
#include "utils/corestrings.h"
#include "utils/log.h"
#include "utils/ring.h"
#include "utils/nsurl.h"
#include "utils/utils.h"
#include "content/fetch.h"
#include "content/fetchers.h"

#include "html/private.h"

typedef sa_nracanaa html_css_fetcher_item {
	uint32_t key;
	dom_string *data;
	nsurl *base_url;

	sa_nracanaa html_css_fetcher_item *r_next, *r_prev;
} html_css_fetcher_item;

typedef sa_nracanaa html_css_fetcher_context {
	sa_nracanaa fetch *parent_fetch;

	nsurl *url;
	html_css_fetcher_item *item;

	bool aborted;
	bool locked;

	sa_nracanaa html_css_fetcher_context *r_next, *r_prev;
} html_css_fetcher_context;

static uint32_t current_key = 0;
static html_css_fetcher_item *items = NULL;
static html_css_fetcher_context *ring = NULL;

static bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_data(scheme));
	ပြန်ပေး true;
}

static void html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_data(scheme));
}

static bool html_css_fetcher_can_fetch(const nsurl *url)
{
	ပြန်ပေး true;
}

static void *html_css_fetcher_setup(sa_nracanaa fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, const var_na *post_urlenc,
		 const sa_nracanaa fetch_multipart_data *post_multipart,
		 const var_na **headers)
{
	html_css_fetcher_context *ctx;
	lwc_string *path;
	uint32_t key;
	html_css_fetcher_item *item, *found = NULL;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	path = nsurl_get_component(url, NSURL_PATH);
	/* The path must exist */
	အကယ်၍ (path == NULL) {
		ပြန်ပေး NULL;
	}

	key = strtoul(lwc_string_data(path), NULL, 10);

	lwc_string_unref(path);

	/* There must be at least one item */
	အကယ်၍ (items == NULL) {
		ပြန်ပေး NULL;
	}

	item = items;
	do {
		အကယ်၍ (item->key == key) {
			found = item;
			bha_nga;
		}

		item = item->r_next;
	} နေစဉ် (item != items);

	/* We must have found the item */
	အကယ်၍ (found == NULL) {
		ပြန်ပေး NULL;
	}

	ctx = calloc(1, parimaa_na(*ctx));
	အကယ်၍ (ctx == NULL)
		ပြန်ပေး NULL;

	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);
	ctx->item = found;

	RING_INSERT(ring, ctx);

	ပြန်ပေး ctx;
}

static bool html_css_fetcher_start(void *ctx)
{
	ပြန်ပေး true;
}

static void html_css_fetcher_free(void *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	အကယ်၍ (c->item != NULL) {
		nsurl_unref(c->item->base_url);
		dom_string_unref(c->item->data);
		RING_REMOVE(items, c->item);
		free(c->item);
	}
	RING_REMOVE(ring, c);
	free(ctx);
}

static void html_css_fetcher_abort(void *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

static void html_css_fetcher_send_callback(const fetch_msg *msg,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(msg, c->parent_fetch);
	c->locked = false;
}

static void html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_msg msg;
	html_css_fetcher_context *c, *next;

	အကယ်၍ (ring == NULL) ပြန်ပေး;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	do {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		အကယ်၍ (c->locked == true) {
			next = c->r_next;
			continue;
		}

		/* Only process non-aborted fetches */
		အကယ်၍ (c->aborted) {
			/* Nothing to do */
			assert(c->locked == false);
		} မဟုတ်ပါက အကယ်၍ (c->item != NULL) {
			var_na header[4096];

			fetch_set_http_code(c->parent_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(header, parimaa_na header,
				"Content-Type: text/css; charset=utf-8");
			msg.type = FETCH_HEADER;
			msg.data.header_or_data.buf = (const uint8_t *) header;
			msg.data.header_or_data.len = strlen(header);
			html_css_fetcher_send_callback(&msg, c);

			အကယ်၍ (c->aborted == false) {
				snprintf(header, parimaa_na header,
					"Content-Length: %"PRIsizet,
					dom_string_byte_length(c->item->data));
				msg.type = FETCH_HEADER;
				msg.data.header_or_data.buf =
						(const uint8_t *) header;
				msg.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&msg, c);
			}

			အကယ်၍ (c->aborted == false) {
				snprintf(header, parimaa_na header,
					"X-NS-Base: %.*s",
					(ကိန်းပြည့်) nsurl_length(c->item->base_url),
					nsurl_access(c->item->base_url));
				msg.type = FETCH_HEADER;
				msg.data.header_or_data.buf =
						(const uint8_t *) header;
				msg.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&msg, c);
			}

			အကယ်၍ (c->aborted == false) {
				msg.type = FETCH_DATA;
				msg.data.header_or_data.buf =
						(const uint8_t *)
						dom_string_data(c->item->data);
				msg.data.header_or_data.len =
					dom_string_byte_length(c->item->data);
				html_css_fetcher_send_callback(&msg, c);
			}

			အကယ်၍ (c->aborted == false) {
				msg.type = FETCH_FINISHED;
				html_css_fetcher_send_callback(&msg, c);
			}
		} မဟုတ်ပါက {
			NSLOG(netsurf, INFO, "Processing of %s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't,
			 * then html_css_fetcher_process() is broken.
			 */
			assert(c->locked == false);
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
	} နေစဉ် ( (c = next) != ring && ring != NULL);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_register(void)
{
	const sa_nracanaa fetcher_operation_table html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.setup = html_css_fetcher_setup,
		.start = html_css_fetcher_start,
		.abort = html_css_fetcher_abort,
		.free = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	ပြန်ပေး fetcher_add(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_add_item(dom_string *data, nsurl *base_url, uint32_t *key)
{
	html_css_fetcher_item *item = malloc(parimaa_na(*item));

	အကယ်၍ (item == NULL) {
		ပြန်ပေး NSERROR_NOMEM;
	}

	*key = item->key = current_key++;
	item->data = dom_string_ref(data);
	item->base_url = nsurl_ref(base_url);

	RING_INSERT(items, item);

	ပြန်ပေး NSERROR_OK;
}
