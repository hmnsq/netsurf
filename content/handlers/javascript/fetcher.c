/*
 * Copyright 2012 Vincent Sanders <vince@kyllikki.org>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
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

/** \file
 * implementation for javascript scheme fetcher
 *
 * This fetcher implements http://www.whatwg.org/specs/web-apps/current-work/multipage/browsers.html#javascript-protocol
 */

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <libwapcaplet/libwapcaplet.h>

#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/ring.h"
#அந்தர்பாவ "content/fetch.h"
#அந்தர்பாவ "content/fetchers.h"

#அந்தர்பாவ "javascript/fetcher.h"

/** Context for an resource fetch */
கட்டமைப்பு fetch_javascript_context {
	கட்டமைப்பு fetch_javascript_context *r_next, *r_prev;

	கட்டமைப்பு fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

நிலையான கட்டமைப்பு fetch_javascript_context *ring = வெற்று;


/** issue fetch callbacks with locking */
நிலையான உள்வரி bool fetch_javascript_send_callback(மாறிலி fetch_செய்தி *செய்தி,
		கட்டமைப்பு fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(செய்தி, ctx->fetchh);
	ctx->locked = false;

	திரும்பு ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
நிலையான bool fetch_javascript_handler(கட்டமைப்பு fetch_javascript_context *ctx)
{
	fetch_செய்தி செய்தி;
	முழு code = 204;

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	செய்தி.type = FETCH_FINISHED;
	fetch_javascript_send_callback(&செய்தி, ctx);

	திரும்பு true;
}



/** callback to initialise the resource fetcher. */
நிலையான bool fetch_javascript_initialise(lwc_string *scheme)
{
	திரும்பு true;
}

/** callback to finalise the resource fetcher. */
நிலையான வெறுமை fetch_javascript_finalise(lwc_string *scheme)
{
}

நிலையான bool fetch_javascript_can_fetch(மாறிலி nsurl *url)
{
	திரும்பு true;
}

/** callback to set up a resource fetch context. */
நிலையான வெறுமை *
fetch_javascript_setup(கட்டமைப்பு fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 மாறிலி எழுத்து *post_urlenc,
		 மாறிலி கட்டமைப்பு fetch_multipart_data *post_multipart,
		 மாறிலி எழுத்து **headers)
{
	கட்டமைப்பு fetch_javascript_context *ctx;

	ctx = calloc(1, அளவு(*ctx));
	எனில் (ctx == வெற்று)
		திரும்பு வெற்று;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_செருகு(ring, ctx);

	திரும்பு ctx;
}

/** callback to free a resource fetch */
நிலையான வெறுமை fetch_javascript_free(வெறுமை *ctx)
{
	கட்டமைப்பு fetch_javascript_context *c = ctx;
	எனில் (c->url != வெற்று) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	விடுவி(ctx);
}

/** callback to start a resource fetch */
நிலையான bool fetch_javascript_start(வெறுமை *ctx)
{
	திரும்பு true;
}

/** callback to abort a resource fetch */
நிலையான வெறுமை fetch_javascript_abort(வெறுமை *ctx)
{
	கட்டமைப்பு fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
நிலையான வெறுமை fetch_javascript_poll(lwc_string *scheme)
{
	கட்டமைப்பு fetch_javascript_context *c, *next;

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
		எனில் (c->aborted == false) {
			/* resource fetches can be processed in one go */
			fetch_javascript_handler(c);
		}

		/* Compute next fetch item at the last possible moment
		 * as processing this item may have added to the ring
		 */
		next = c->r_next;

		fetch_remove_from_queues(c->fetchh);
		fetch_free(c->fetchh);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} போது ( (c = next) != ring && ring != வெற்று);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_register(வெறுமை)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	மாறிலி கட்டமைப்பு fetcher_operation_table fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.setup = fetch_javascript_setup,
		.start = fetch_javascript_start,
		.abort = fetch_javascript_abort,
		.விடுவி = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	திரும்பு fetcher_add(scheme, &fetcher_ops);
}
