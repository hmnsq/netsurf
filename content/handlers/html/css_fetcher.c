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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <dom/dom.h>
#અન્તર્ભાવ <libwapcaplet/libwapcaplet.h>

#અન્તર્ભાવ "netsurf/inttypes.h"
#અન્તર્ભાવ "utils/config.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/ring.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "content/fetch.h"
#અન્તર્ભાવ "content/fetchers.h"

#અન્તર્ભાવ "html/private.h"

પ્રકારવ્યાખ્યા સંરચના html_css_fetcher_item {
	uint32_t ચાવી;
	dom_string *data;
	nsurl *base_url;

	સંરચના html_css_fetcher_item *r_next, *r_prev;
} html_css_fetcher_item;

પ્રકારવ્યાખ્યા સંરચના html_css_fetcher_context {
	સંરચના fetch *parent_fetch;

	nsurl *url;
	html_css_fetcher_item *item;

	bool aborted;
	bool locked;

	સંરચના html_css_fetcher_context *r_next, *r_prev;
} html_css_fetcher_context;

સ્થિર uint32_t current_ચાવી = 0;
સ્થિર html_css_fetcher_item *વસ્તુઓ = ખાલી;
સ્થિર html_css_fetcher_context *ring = ખાલી;

સ્થિર bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_data(scheme));
	પાછા true;
}

સ્થિર શૂન્ય html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_data(scheme));
}

સ્થિર bool html_css_fetcher_can_fetch(અચલ nsurl *url)
{
	પાછા true;
}

સ્થિર શૂન્ય *html_css_fetcher_setup(સંરચના fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, અચલ અક્ષર *post_urlenc,
		 અચલ સંરચના fetch_multipart_data *post_multipart,
		 અચલ અક્ષર **headers)
{
	html_css_fetcher_context *ctx;
	lwc_string *path;
	uint32_t ચાવી;
	html_css_fetcher_item *item, *found = ખાલી;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	path = nsurl_get_component(url, NSURL_PATH);
	/* The path must exist */
	જો (path == ખાલી) {
		પાછા ખાલી;
	}

	ચાવી = strtoul(lwc_string_data(path), ખાલી, 10);

	lwc_string_unref(path);

	/* There must be at least one item */
	જો (વસ્તુઓ == ખાલી) {
		પાછા ખાલી;
	}

	item = વસ્તુઓ;
	કરો {
		જો (item->ચાવી == ચાવી) {
			found = item;
			તોડો;
		}

		item = item->r_next;
	} જ્યારે (item != વસ્તુઓ);

	/* We must have found the item */
	જો (found == ખાલી) {
		પાછા ખાલી;
	}

	ctx = calloc(1, કદ(*ctx));
	જો (ctx == ખાલી)
		પાછા ખાલી;

	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);
	ctx->item = found;

	RING_ઉમેરો(ring, ctx);

	પાછા ctx;
}

સ્થિર bool html_css_fetcher_start(શૂન્ય *ctx)
{
	પાછા true;
}

સ્થિર શૂન્ય html_css_fetcher_free(શૂન્ય *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	જો (c->item != ખાલી) {
		nsurl_unref(c->item->base_url);
		dom_string_unref(c->item->data);
		RING_REMOVE(વસ્તુઓ, c->item);
		મુક્ત(c->item);
	}
	RING_REMOVE(ring, c);
	મુક્ત(ctx);
}

સ્થિર શૂન્ય html_css_fetcher_abort(શૂન્ય *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

સ્થિર શૂન્ય html_css_fetcher_send_callback(અચલ fetch_સંદેશ *સંદેશ,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(સંદેશ, c->parent_fetch);
	c->locked = false;
}

સ્થિર શૂન્ય html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_સંદેશ સંદેશ;
	html_css_fetcher_context *c, *next;

	જો (ring == ખાલી) પાછા;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	કરો {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		જો (c->locked == true) {
			next = c->r_next;
			ચાલુ;
		}

		/* Only process non-aborted fetches */
		જો (c->aborted) {
			/* Nothing to do */
			પ્રતિજ્ઞા(c->locked == false);
		} નહીંતર જો (c->item != ખાલી) {
			અક્ષર header[4096];

			fetch_set_http_code(c->parent_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(header, કદ header,
				"Content-Type: text/css; charset=utf-8");
			સંદેશ.type = FETCH_HEADER;
			સંદેશ.data.header_or_data.buf = (અચલ uint8_t *) header;
			સંદેશ.data.header_or_data.len = strlen(header);
			html_css_fetcher_send_callback(&સંદેશ, c);

			જો (c->aborted == false) {
				snprintf(header, કદ header,
					"Content-Length: %"PRIsizet,
					dom_string_byte_length(c->item->data));
				સંદેશ.type = FETCH_HEADER;
				સંદેશ.data.header_or_data.buf =
						(અચલ uint8_t *) header;
				સંદેશ.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&સંદેશ, c);
			}

			જો (c->aborted == false) {
				snprintf(header, કદ header,
					"X-NS-Base: %.*s",
					(પૂર્ણાંક) nsurl_length(c->item->base_url),
					nsurl_access(c->item->base_url));
				સંદેશ.type = FETCH_HEADER;
				સંદેશ.data.header_or_data.buf =
						(અચલ uint8_t *) header;
				સંદેશ.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&સંદેશ, c);
			}

			જો (c->aborted == false) {
				સંદેશ.type = FETCH_DATA;
				સંદેશ.data.header_or_data.buf =
						(અચલ uint8_t *)
						dom_string_data(c->item->data);
				સંદેશ.data.header_or_data.len =
					dom_string_byte_length(c->item->data);
				html_css_fetcher_send_callback(&સંદેશ, c);
			}

			જો (c->aborted == false) {
				સંદેશ.type = FETCH_FINISHED;
				html_css_fetcher_send_callback(&સંદેશ, c);
			}
		} નહીંતર {
			NSLOG(netsurf, INFO, "Processing of %s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't,
			 * then html_css_fetcher_process() is broken.
			 */
			પ્રતિજ્ઞા(c->locked == false);
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
	} જ્યારે ( (c = next) != ring && ring != ખાલી);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_register(શૂન્ય)
{
	અચલ સંરચના fetcher_operation_table html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.setup = html_css_fetcher_setup,
		.start = html_css_fetcher_start,
		.abort = html_css_fetcher_abort,
		.મુક્ત = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	પાછા fetcher_add(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_add_item(dom_string *data, nsurl *base_url, uint32_t *ચાવી)
{
	html_css_fetcher_item *item = malloc(કદ(*item));

	જો (item == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	*ચાવી = item->ચાવી = current_ચાવી++;
	item->data = dom_string_ref(data);
	item->base_url = nsurl_ref(base_url);

	RING_ઉમેરો(વસ્તુઓ, item);

	પાછા NSERROR_OK;
}
