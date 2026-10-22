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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <dom/dom.h>
#അന്തര്ഭാവ <libwapcaplet/libwapcaplet.h>

#അന്തര്ഭാവ "netsurf/inttypes.h"
#അന്തര്ഭാവ "utils/config.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/ring.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "content/fetch.h"
#അന്തര്ഭാവ "content/fetchers.h"

#അന്തര്ഭാവ "html/private.h"

തരനിർവചനം ഘടന html_css_fetcher_item {
	uint32_t താക്കോൽ;
	dom_string *data;
	nsurl *base_url;

	ഘടന html_css_fetcher_item *r_next, *r_prev;
} html_css_fetcher_item;

തരനിർവചനം ഘടന html_css_fetcher_context {
	ഘടന fetch *parent_fetch;

	nsurl *url;
	html_css_fetcher_item *item;

	bool aborted;
	bool locked;

	ഘടന html_css_fetcher_context *r_next, *r_prev;
} html_css_fetcher_context;

സ്ഥിതം uint32_t current_താക്കോൽ = 0;
സ്ഥിതം html_css_fetcher_item *ഇനങ്ങൾ = ശൂന്യ;
സ്ഥിതം html_css_fetcher_context *ring = ശൂന്യ;

സ്ഥിതം bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_data(scheme));
	തിരികെ true;
}

സ്ഥിതം ശൂന്യം html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_data(scheme));
}

സ്ഥിതം bool html_css_fetcher_can_fetch(സ്ഥിരം nsurl *url)
{
	തിരികെ true;
}

സ്ഥിതം ശൂന്യം *html_css_fetcher_setup(ഘടന fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, സ്ഥിരം അക്ഷരം *post_urlenc,
		 സ്ഥിരം ഘടന fetch_multipart_data *post_multipart,
		 സ്ഥിരം അക്ഷരം **headers)
{
	html_css_fetcher_context *ctx;
	lwc_string *path;
	uint32_t താക്കോൽ;
	html_css_fetcher_item *item, *found = ശൂന്യ;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	path = nsurl_get_component(url, NSURL_PATH);
	/* The path must exist */
	എങ്കിൽ (path == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}

	താക്കോൽ = strtoul(lwc_string_data(path), ശൂന്യ, 10);

	lwc_string_unref(path);

	/* There must be at least one item */
	എങ്കിൽ (ഇനങ്ങൾ == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}

	item = ഇനങ്ങൾ;
	ചെയ്യുക {
		എങ്കിൽ (item->താക്കോൽ == താക്കോൽ) {
			found = item;
			നിർത്തുക;
		}

		item = item->r_next;
	} ആകുമ്പോൾ (item != ഇനങ്ങൾ);

	/* We must have found the item */
	എങ്കിൽ (found == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}

	ctx = calloc(1, വലുപ്പം(*ctx));
	എങ്കിൽ (ctx == ശൂന്യ)
		തിരികെ ശൂന്യ;

	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);
	ctx->item = found;

	RING_തിരുകുക(ring, ctx);

	തിരികെ ctx;
}

സ്ഥിതം bool html_css_fetcher_start(ശൂന്യം *ctx)
{
	തിരികെ true;
}

സ്ഥിതം ശൂന്യം html_css_fetcher_free(ശൂന്യം *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	എങ്കിൽ (c->item != ശൂന്യ) {
		nsurl_unref(c->item->base_url);
		dom_string_unref(c->item->data);
		RING_REMOVE(ഇനങ്ങൾ, c->item);
		സ്വതന്ത്രം(c->item);
	}
	RING_REMOVE(ring, c);
	സ്വതന്ത്രം(ctx);
}

സ്ഥിതം ശൂന്യം html_css_fetcher_abort(ശൂന്യം *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

സ്ഥിതം ശൂന്യം html_css_fetcher_send_callback(സ്ഥിരം fetch_സന്ദേശം *സന്ദേശം,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(സന്ദേശം, c->parent_fetch);
	c->locked = false;
}

സ്ഥിതം ശൂന്യം html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_സന്ദേശം സന്ദേശം;
	html_css_fetcher_context *c, *next;

	എങ്കിൽ (ring == ശൂന്യ) തിരികെ;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	ചെയ്യുക {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		എങ്കിൽ (c->locked == true) {
			next = c->r_next;
			തുടരുക;
		}

		/* Only process non-aborted fetches */
		എങ്കിൽ (c->aborted) {
			/* Nothing to do */
			പ്രതിജ്ഞാ(c->locked == false);
		} അല്ലെങ്കിൽ എങ്കിൽ (c->item != ശൂന്യ) {
			അക്ഷരം header[4096];

			fetch_set_http_code(c->parent_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(header, വലുപ്പം header,
				"Content-Type: text/css; charset=utf-8");
			സന്ദേശം.type = FETCH_HEADER;
			സന്ദേശം.data.header_or_data.buf = (സ്ഥിരം uint8_t *) header;
			സന്ദേശം.data.header_or_data.len = strlen(header);
			html_css_fetcher_send_callback(&സന്ദേശം, c);

			എങ്കിൽ (c->aborted == false) {
				snprintf(header, വലുപ്പം header,
					"Content-Length: %"PRIsizet,
					dom_string_byte_length(c->item->data));
				സന്ദേശം.type = FETCH_HEADER;
				സന്ദേശം.data.header_or_data.buf =
						(സ്ഥിരം uint8_t *) header;
				സന്ദേശം.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&സന്ദേശം, c);
			}

			എങ്കിൽ (c->aborted == false) {
				snprintf(header, വലുപ്പം header,
					"X-NS-Base: %.*s",
					(പൂർണ്ണസംഖ്യ) nsurl_length(c->item->base_url),
					nsurl_access(c->item->base_url));
				സന്ദേശം.type = FETCH_HEADER;
				സന്ദേശം.data.header_or_data.buf =
						(സ്ഥിരം uint8_t *) header;
				സന്ദേശം.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&സന്ദേശം, c);
			}

			എങ്കിൽ (c->aborted == false) {
				സന്ദേശം.type = FETCH_DATA;
				സന്ദേശം.data.header_or_data.buf =
						(സ്ഥിരം uint8_t *)
						dom_string_data(c->item->data);
				സന്ദേശം.data.header_or_data.len =
					dom_string_byte_length(c->item->data);
				html_css_fetcher_send_callback(&സന്ദേശം, c);
			}

			എങ്കിൽ (c->aborted == false) {
				സന്ദേശം.type = FETCH_FINISHED;
				html_css_fetcher_send_callback(&സന്ദേശം, c);
			}
		} അല്ലെങ്കിൽ {
			NSLOG(netsurf, INFO, "Processing of %s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't,
			 * then html_css_fetcher_process() is broken.
			 */
			പ്രതിജ്ഞാ(c->locked == false);
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
	} ആകുമ്പോൾ ( (c = next) != ring && ring != ശൂന്യ);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_register(ശൂന്യം)
{
	സ്ഥിരം ഘടന fetcher_operation_table html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.setup = html_css_fetcher_setup,
		.start = html_css_fetcher_start,
		.abort = html_css_fetcher_abort,
		.സ്വതന്ത്രം = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	തിരികെ fetcher_add(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_add_item(dom_string *data, nsurl *base_url, uint32_t *താക്കോൽ)
{
	html_css_fetcher_item *item = malloc(വലുപ്പം(*item));

	എങ്കിൽ (item == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	*താക്കോൽ = item->താക്കോൽ = current_താക്കോൽ++;
	item->data = dom_string_ref(data);
	item->base_url = nsurl_ref(base_url);

	RING_തിരുകുക(ഇനങ്ങൾ, item);

	തിരികെ NSERROR_OK;
}
