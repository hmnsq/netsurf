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

#समावेश <assert.h>
#समावेश <stdbool.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <dom/dom.h>
#समावेश <libwapcaplet/libwapcaplet.h>

#समावेश "netsurf/inttypes.h"
#समावेश "utils/config.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/log.h"
#समावेश "utils/ring.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/utils.h"
#समावेश "content/fetch.h"
#समावेश "content/fetchers.h"

#समावेश "html/private.h"

प्रकार काष्ठा html_css_fetcher_item {
	uint32_t कुंजी;
	dom_string *तथ्य;
	nsurl *base_url;

	काष्ठा html_css_fetcher_item *r_अग्र, *r_पूर्व;
} html_css_fetcher_item;

प्रकार काष्ठा html_css_fetcher_context {
	काष्ठा fetch *parent_fetch;

	nsurl *url;
	html_css_fetcher_item *item;

	bool aborted;
	bool locked;

	काष्ठा html_css_fetcher_context *r_अग्र, *r_पूर्व;
} html_css_fetcher_context;

अटल uint32_t current_कुंजी = 0;
अटल html_css_fetcher_item *वस्तुएँ = शून्य;
अटल html_css_fetcher_context *ring = शून्य;

अटल bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_तथ्य(scheme));
	वापस true;
}

अटल व्योम html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_तथ्य(scheme));
}

अटल bool html_css_fetcher_can_fetch(स्थिर nsurl *url)
{
	वापस true;
}

अटल व्योम *html_css_fetcher_setup(काष्ठा fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, स्थिर अक्षर *post_urlenc,
		 स्थिर काष्ठा fetch_multipart_तथ्य *post_multipart,
		 स्थिर अक्षर **headers)
{
	html_css_fetcher_context *ctx;
	lwc_string *path;
	uint32_t कुंजी;
	html_css_fetcher_item *item, *found = शून्य;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	path = nsurl_get_component(url, NSURL_PATH);
	/* The path must exist */
	अगर (path == शून्य) {
		वापस शून्य;
	}

	कुंजी = म_से_अदीर्घ(lwc_string_तथ्य(path), शून्य, 10);

	lwc_string_unref(path);

	/* There must be at least one item */
	अगर (वस्तुएँ == शून्य) {
		वापस शून्य;
	}

	item = वस्तुएँ;
	करो {
		अगर (item->कुंजी == कुंजी) {
			found = item;
			अवरोध;
		}

		item = item->r_अग्र;
	} जबतक (item != वस्तुएँ);

	/* We must have found the item */
	अगर (found == शून्य) {
		वापस शून्य;
	}

	ctx = सुस्मृति(1, माप(*ctx));
	अगर (ctx == शून्य)
		वापस शून्य;

	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);
	ctx->item = found;

	RING_जोड़ो(ring, ctx);

	वापस ctx;
}

अटल bool html_css_fetcher_start(व्योम *ctx)
{
	वापस true;
}

अटल व्योम html_css_fetcher_free(व्योम *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	अगर (c->item != शून्य) {
		nsurl_unref(c->item->base_url);
		dom_string_unref(c->item->तथ्य);
		RING_REMOVE(वस्तुएँ, c->item);
		मुक्त(c->item);
	}
	RING_REMOVE(ring, c);
	मुक्त(ctx);
}

अटल व्योम html_css_fetcher_abort(व्योम *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

अटल व्योम html_css_fetcher_send_callback(स्थिर fetch_संदेश *संदेश,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(संदेश, c->parent_fetch);
	c->locked = false;
}

अटल व्योम html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_संदेश संदेश;
	html_css_fetcher_context *c, *अग्र;

	अगर (ring == शून्य) वापस;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	करो {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		अगर (c->locked == true) {
			अग्र = c->r_अग्र;
			जारी;
		}

		/* Only process non-aborted fetches */
		अगर (c->aborted) {
			/* Nothing to do */
			निश्चित(c->locked == false);
		} अन्यथा अगर (c->item != शून्य) {
			अक्षर header[4096];

			fetch_set_http_code(c->parent_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(header, माप header,
				"Content-Type: text/css; charset=utf-8");
			संदेश.type = FETCH_HEADER;
			संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *) header;
			संदेश.तथ्य.header_or_तथ्य.len = म_माप(header);
			html_css_fetcher_send_callback(&संदेश, c);

			अगर (c->aborted == false) {
				snprintf(header, माप header,
					"Content-Length: %"PRIsizet,
					dom_string_byte_लंबाई(c->item->तथ्य));
				संदेश.type = FETCH_HEADER;
				संदेश.तथ्य.header_or_तथ्य.buf =
						(स्थिर uint8_t *) header;
				संदेश.तथ्य.header_or_तथ्य.len = म_माप(header);
				html_css_fetcher_send_callback(&संदेश, c);
			}

			अगर (c->aborted == false) {
				snprintf(header, माप header,
					"X-NS-Base: %.*s",
					(पूर्णांक) nsurl_लंबाई(c->item->base_url),
					nsurl_access(c->item->base_url));
				संदेश.type = FETCH_HEADER;
				संदेश.तथ्य.header_or_तथ्य.buf =
						(स्थिर uint8_t *) header;
				संदेश.तथ्य.header_or_तथ्य.len = म_माप(header);
				html_css_fetcher_send_callback(&संदेश, c);
			}

			अगर (c->aborted == false) {
				संदेश.type = FETCH_तथ्य;
				संदेश.तथ्य.header_or_तथ्य.buf =
						(स्थिर uint8_t *)
						dom_string_तथ्य(c->item->तथ्य);
				संदेश.तथ्य.header_or_तथ्य.len =
					dom_string_byte_लंबाई(c->item->तथ्य);
				html_css_fetcher_send_callback(&संदेश, c);
			}

			अगर (c->aborted == false) {
				संदेश.type = FETCH_FINISHED;
				html_css_fetcher_send_callback(&संदेश, c);
			}
		} अन्यथा {
			NSLOG(netsurf, INFO, "Processing of %s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't,
			 * then html_css_fetcher_process() is broken.
			 */
			निश्चित(c->locked == false);
		}

		/* Compute next fetch item at the last possible moment as
		 * processing this item may have added to the ring.
		 */
		अग्र = c->r_अग्र;

		fetch_remove_from_queues(c->parent_fetch);
		fetch_free(c->parent_fetch);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} जबतक ( (c = अग्र) != ring && ring != शून्य);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_register(व्योम)
{
	स्थिर काष्ठा fetcher_operation_table html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.setup = html_css_fetcher_setup,
		.start = html_css_fetcher_start,
		.पात = html_css_fetcher_abort,
		.मुक्त = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	वापस fetcher_add(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_add_item(dom_string *तथ्य, nsurl *base_url, uint32_t *कुंजी)
{
	html_css_fetcher_item *item = दो_स्मृति(माप(*item));

	अगर (item == शून्य) {
		वापस NSERROR_NOMEM;
	}

	*कुंजी = item->कुंजी = current_कुंजी++;
	item->तथ्य = dom_string_ref(तथ्य);
	item->base_url = nsurl_ref(base_url);

	RING_जोड़ो(वस्तुएँ, item);

	वापस NSERROR_OK;
}
