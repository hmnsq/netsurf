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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <dom/dom.h>
#अन्तर्भाव <libwapcaplet/libwapcaplet.h>

#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "utils/config.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/ring.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "content/fetch.h"
#अन्तर्भाव "content/fetchers.h"

#अन्तर्भाव "html/private.h"

प्रकारनिर्देश संरचना html_css_fetcher_item {
	uint32_t कुञ्चिका;
	dom_string *data;
	nsurl *base_url;

	संरचना html_css_fetcher_item *r_next, *r_prev;
} html_css_fetcher_item;

प्रकारनिर्देश संरचना html_css_fetcher_context {
	संरचना fetch *parent_fetch;

	nsurl *url;
	html_css_fetcher_item *item;

	bool aborted;
	bool locked;

	संरचना html_css_fetcher_context *r_next, *r_prev;
} html_css_fetcher_context;

स्थिर uint32_t current_कुञ्चिका = 0;
स्थिर html_css_fetcher_item *वस्तूनि = रिक्त;
स्थिर html_css_fetcher_context *ring = रिक्त;

स्थिर bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_data(scheme));
	पटिनिवत्त true;
}

स्थिर शून्य html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_data(scheme));
}

स्थिर bool html_css_fetcher_can_fetch(ध्रुव nsurl *url)
{
	पटिनिवत्त true;
}

स्थिर शून्य *html_css_fetcher_setup(संरचना fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ध्रुव वर्ण *post_urlenc,
		 ध्रुव संरचना fetch_multipart_data *post_multipart,
		 ध्रुव वर्ण **headers)
{
	html_css_fetcher_context *ctx;
	lwc_string *path;
	uint32_t कुञ्चिका;
	html_css_fetcher_item *item, *found = रिक्त;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	path = nsurl_get_component(url, NSURL_PATH);
	/* The path must exist */
	सचे (path == रिक्त) {
		पटिनिवत्त रिक्त;
	}

	कुञ्चिका = strtoul(lwc_string_data(path), रिक्त, 10);

	lwc_string_unref(path);

	/* There must be at least one item */
	सचे (वस्तूनि == रिक्त) {
		पटिनिवत्त रिक्त;
	}

	item = वस्तूनि;
	करोहि {
		सचे (item->कुञ्चिका == कुञ्चिका) {
			found = item;
			भङ्ग;
		}

		item = item->r_next;
	} याव (item != वस्तूनि);

	/* We must have found the item */
	सचे (found == रिक्त) {
		पटिनिवत्त रिक्त;
	}

	ctx = calloc(1, परिमाण(*ctx));
	सचे (ctx == रिक्त)
		पटिनिवत्त रिक्त;

	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);
	ctx->item = found;

	RING_निवेशय(ring, ctx);

	पटिनिवत्त ctx;
}

स्थिर bool html_css_fetcher_start(शून्य *ctx)
{
	पटिनिवत्त true;
}

स्थिर शून्य html_css_fetcher_free(शून्य *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	सचे (c->item != रिक्त) {
		nsurl_unref(c->item->base_url);
		dom_string_unref(c->item->data);
		RING_REMOVE(वस्तूनि, c->item);
		मुक्त(c->item);
	}
	RING_REMOVE(ring, c);
	मुक्त(ctx);
}

स्थिर शून्य html_css_fetcher_abort(शून्य *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

स्थिर शून्य html_css_fetcher_send_callback(ध्रुव fetch_सन्देशः *सन्देशः,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(सन्देशः, c->parent_fetch);
	c->locked = false;
}

स्थिर शून्य html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_सन्देशः सन्देशः;
	html_css_fetcher_context *c, *next;

	सचे (ring == रिक्त) पटिनिवत्त;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	करोहि {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		सचे (c->locked == true) {
			next = c->r_next;
			अनुवर्त;
		}

		/* Only process non-aborted fetches */
		सचे (c->aborted) {
			/* Nothing to do */
			प्रतिज्ञा(c->locked == false);
		} अञ्ञथा सचे (c->item != रिक्त) {
			वर्ण header[4096];

			fetch_set_http_code(c->parent_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(header, परिमाण header,
				"Content-Type: text/css; charset=utf-8");
			सन्देशः.type = FETCH_HEADER;
			सन्देशः.data.header_or_data.buf = (ध्रुव uint8_t *) header;
			सन्देशः.data.header_or_data.len = strlen(header);
			html_css_fetcher_send_callback(&सन्देशः, c);

			सचे (c->aborted == false) {
				snprintf(header, परिमाण header,
					"Content-Length: %"PRIsizet,
					dom_string_byte_length(c->item->data));
				सन्देशः.type = FETCH_HEADER;
				सन्देशः.data.header_or_data.buf =
						(ध्रुव uint8_t *) header;
				सन्देशः.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&सन्देशः, c);
			}

			सचे (c->aborted == false) {
				snprintf(header, परिमाण header,
					"X-NS-Base: %.*s",
					(पूर्णाङ्क) nsurl_length(c->item->base_url),
					nsurl_access(c->item->base_url));
				सन्देशः.type = FETCH_HEADER;
				सन्देशः.data.header_or_data.buf =
						(ध्रुव uint8_t *) header;
				सन्देशः.data.header_or_data.len = strlen(header);
				html_css_fetcher_send_callback(&सन्देशः, c);
			}

			सचे (c->aborted == false) {
				सन्देशः.type = FETCH_DATA;
				सन्देशः.data.header_or_data.buf =
						(ध्रुव uint8_t *)
						dom_string_data(c->item->data);
				सन्देशः.data.header_or_data.len =
					dom_string_byte_length(c->item->data);
				html_css_fetcher_send_callback(&सन्देशः, c);
			}

			सचे (c->aborted == false) {
				सन्देशः.type = FETCH_FINISHED;
				html_css_fetcher_send_callback(&सन्देशः, c);
			}
		} अञ्ञथा {
			NSLOG(netsurf, INFO, "Processing of %s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't,
			 * then html_css_fetcher_process() is broken.
			 */
			प्रतिज्ञा(c->locked == false);
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
	} याव ( (c = next) != ring && ring != रिक्त);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_register(शून्य)
{
	ध्रुव संरचना fetcher_operation_table html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.setup = html_css_fetcher_setup,
		.start = html_css_fetcher_start,
		.abort = html_css_fetcher_abort,
		.मुक्त = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	पटिनिवत्त fetcher_add(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_add_item(dom_string *data, nsurl *base_url, uint32_t *कुञ्चिका)
{
	html_css_fetcher_item *item = malloc(परिमाण(*item));

	सचे (item == रिक्त) {
		पटिनिवत्त NSERROR_NOMEM;
	}

	*कुञ्चिका = item->कुञ्चिका = current_कुञ्चिका++;
	item->data = dom_string_ref(data);
	item->base_url = nsurl_ref(base_url);

	RING_निवेशय(वस्तूनि, item);

	पटिनिवत्त NSERROR_OK;
}
