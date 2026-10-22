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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <dom/dom.h>
#ଅନ୍ତର୍ଭାଵ <libwapcaplet/libwapcaplet.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "utils/config.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/ring.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "content/fetch.h"
#ଅନ୍ତର୍ଭାଵ "content/fetchers.h"

#ଅନ୍ତର୍ଭାଵ "html/private.h"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା html_css_fetcher_item {
	uint32_t ଚାବି;
	dom_string *data;
	nsurl *base_url;

	ସଂରଚନା html_css_fetcher_item *r_next, *r_prev;
} html_css_fetcher_item;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା html_css_fetcher_context {
	ସଂରଚନା fetch *parent_fetch;

	nsurl *url;
	html_css_fetcher_item *item;

	bool aborted;
	bool locked;

	ସଂରଚନା html_css_fetcher_context *r_next, *r_prev;
} html_css_fetcher_context;

ସ୍ଥିର uint32_t current_ଚାବି = 0;
ସ୍ଥିର html_css_fetcher_item *ବସ୍ତୁଗୁଡ଼ିକ = ଖାଲି;
ସ୍ଥିର html_css_fetcher_context *ring = ଖାଲି;

ସ୍ଥିର bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_data(scheme));
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର ଶୂନ୍ୟ html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_data(scheme));
}

ସ୍ଥିର bool html_css_fetcher_can_fetch(ଅଚଳ nsurl *url)
{
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର ଶୂନ୍ୟ *html_css_fetcher_setup(ସଂରଚନା fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ଅଚଳ ଅକ୍ଷର *post_urlenc,
		 ଅଚଳ ସଂରଚନା fetch_multipart_data *post_multipart,
		 ଅଚଳ ଅକ୍ଷର **headers)
{
	html_css_fetcher_context *ctx;
	lwc_string *path;
	uint32_t ଚାବି;
	html_css_fetcher_item *item, *found = ଖାଲି;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	path = nsurl_get_component(url, NSURL_PATH);
	/* The path must exist */
	ଯଦି (path == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଚାବି = strtoul(lwc_string_data(path), ଖାଲି, 10);

	lwc_string_unref(path);

	/* There must be at least one item */
	ଯଦି (ବସ୍ତୁଗୁଡ଼ିକ == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	item = ବସ୍ତୁଗୁଡ଼ିକ;
	କର {
		ଯଦି (item->ଚାବି == ଚାବି) {
			found = item;
			ଭାଙ୍ଗ;
		}

		item = item->r_next;
	} ଯେତେବେଳେ (item != ବସ୍ତୁଗୁଡ଼ିକ);

	/* We must have found the item */
	ଯଦି (found == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ctx = calloc(1, ଆକାର(*ctx));
	ଯଦି (ctx == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);
	ctx->item = found;

	RING_ଭର୍ତ୍ତି(ring, ctx);

	ଫେରନ୍ତୁ ctx;
}

ସ୍ଥିର bool html_css_fetcher_start(ଶୂନ୍ୟ *ctx)
{
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର ଶୂନ୍ୟ html_css_fetcher_free(ଶୂନ୍ୟ *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	ଯଦି (c->item != ଖାଲି) {
		nsurl_unref(c->item->base_url);
		dom_string_unref(c->item->data);
		RING_REMOVE(ବସ୍ତୁଗୁଡ଼ିକ, c->item);
		ମୁକ୍ତ(c->item);
	}
	RING_REMOVE(ring, c);
	ମୁକ୍ତ(ctx);
}

ସ୍ଥିର ଶୂନ୍ୟ html_css_fetcher_abort(ଶୂନ୍ୟ *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ସ୍ଥିର ଶୂନ୍ୟ html_css_fetcher_send_callback(ଅଚଳ fetch_ସନ୍ଦେଶ *ସନ୍ଦେଶ,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(ସନ୍ଦେଶ, c->parent_fetch);
	c->locked = false;
}

ସ୍ଥିର ଶୂନ୍ୟ html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	html_css_fetcher_context *c, *next;

	ଯଦି (ring == ଖାଲି) ଫେରନ୍ତୁ;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	କର {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ଯଦି (c->locked == true) {
			next = c->r_next;
			ଜାରି;
		}

		/* Only process non-aborted fetches */
		ଯଦି (c->aborted) {
			/* Nothing to do */
			ପ୍ରତିଜ୍ଞା(c->locked == false);
		} ଅନ୍ୟଥା ଯଦି (c->item != ଖାଲି) {
			ଅକ୍ଷର header[4096];

			fetch_set_http_code(c->parent_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(header, ଆକାର header,
				"Content-Type: text/css; charset=utf-8");
			ସନ୍ଦେଶ.type = FETCH_HEADER;
			ସନ୍ଦେଶ.data.header_or_data.buf = (ଅଚଳ uint8_t *) header;
			ସନ୍ଦେଶ.data.header_or_data.len = strlen(header);
			html_css_fetcher_send_callback(&ସନ୍ଦେଶ, c);

			ଯଦି (c->aborted == false) {
				snprintf(header, ଆକାର header,
					"Content-Length: %"PRIsizet,
					dom_string_byte_length(c->item->data));
				ସନ୍ଦେଶ.type = FETCH_HEADER;
				ସନ୍ଦେଶ.data.header_or_data.buf =
						(ଅଚଳ uint8_t *) header;
				ସନ୍ଦେଶ.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&ସନ୍ଦେଶ, c);
			}

			ଯଦି (c->aborted == false) {
				snprintf(header, ଆକାର header,
					"X-NS-Base: %.*s",
					(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) nsurl_length(c->item->base_url),
					nsurl_access(c->item->base_url));
				ସନ୍ଦେଶ.type = FETCH_HEADER;
				ସନ୍ଦେଶ.data.header_or_data.buf =
						(ଅଚଳ uint8_t *) header;
				ସନ୍ଦେଶ.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&ସନ୍ଦେଶ, c);
			}

			ଯଦି (c->aborted == false) {
				ସନ୍ଦେଶ.type = FETCH_DATA;
				ସନ୍ଦେଶ.data.header_or_data.buf =
						(ଅଚଳ uint8_t *)
						dom_string_data(c->item->data);
				ସନ୍ଦେଶ.data.header_or_data.len =
					dom_string_byte_length(c->item->data);
				html_css_fetcher_send_callback(&ସନ୍ଦେଶ, c);
			}

			ଯଦି (c->aborted == false) {
				ସନ୍ଦେଶ.type = FETCH_FINISHED;
				html_css_fetcher_send_callback(&ସନ୍ଦେଶ, c);
			}
		} ଅନ୍ୟଥା {
			NSLOG(netsurf, INFO, "Processing of %s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't,
			 * then html_css_fetcher_process() is broken.
			 */
			ପ୍ରତିଜ୍ଞା(c->locked == false);
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
	} ଯେତେବେଳେ ( (c = next) != ring && ring != ଖାଲି);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_register(ଶୂନ୍ୟ)
{
	ଅଚଳ ସଂରଚନା fetcher_operation_table html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.setup = html_css_fetcher_setup,
		.start = html_css_fetcher_start,
		.abort = html_css_fetcher_abort,
		.ମୁକ୍ତ = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	ଫେରନ୍ତୁ fetcher_add(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_add_item(dom_string *data, nsurl *base_url, uint32_t *ଚାବି)
{
	html_css_fetcher_item *item = malloc(ଆକାର(*item));

	ଯଦି (item == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	*ଚାବି = item->ଚାବି = current_ଚାବି++;
	item->data = dom_string_ref(data);
	item->base_url = nsurl_ref(base_url);

	RING_ଭର୍ତ୍ତି(ବସ୍ତୁଗୁଡ଼ିକ, item);

	ଫେରନ୍ତୁ NSERROR_OK;
}
