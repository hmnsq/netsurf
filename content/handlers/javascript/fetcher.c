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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <libwapcaplet/libwapcaplet.h>

#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/ring.h"
#അന്തര്ഭാവ "content/fetch.h"
#അന്തര്ഭാവ "content/fetchers.h"

#അന്തര്ഭാവ "javascript/fetcher.h"

/** Context for an resource fetch */
ഘടന fetch_javascript_context {
	ഘടന fetch_javascript_context *r_next, *r_prev;

	ഘടന fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

സ്ഥിതം ഘടന fetch_javascript_context *ring = ശൂന്യ;


/** issue fetch callbacks with locking */
സ്ഥിതം വരിയിൽ bool fetch_javascript_send_callback(സ്ഥിരം fetch_സന്ദേശം *സന്ദേശം,
		ഘടന fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(സന്ദേശം, ctx->fetchh);
	ctx->locked = false;

	തിരികെ ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
സ്ഥിതം bool fetch_javascript_handler(ഘടന fetch_javascript_context *ctx)
{
	fetch_സന്ദേശം സന്ദേശം;
	പൂർണ്ണസംഖ്യ code = 204;

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	സന്ദേശം.type = FETCH_FINISHED;
	fetch_javascript_send_callback(&സന്ദേശം, ctx);

	തിരികെ true;
}



/** callback to initialise the resource fetcher. */
സ്ഥിതം bool fetch_javascript_initialise(lwc_string *scheme)
{
	തിരികെ true;
}

/** callback to finalise the resource fetcher. */
സ്ഥിതം ശൂന്യം fetch_javascript_finalise(lwc_string *scheme)
{
}

സ്ഥിതം bool fetch_javascript_can_fetch(സ്ഥിരം nsurl *url)
{
	തിരികെ true;
}

/** callback to set up a resource fetch context. */
സ്ഥിതം ശൂന്യം *
fetch_javascript_setup(ഘടന fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 സ്ഥിരം അക്ഷരം *post_urlenc,
		 സ്ഥിരം ഘടന fetch_multipart_data *post_multipart,
		 സ്ഥിരം അക്ഷരം **headers)
{
	ഘടന fetch_javascript_context *ctx;

	ctx = calloc(1, വലുപ്പം(*ctx));
	എങ്കിൽ (ctx == ശൂന്യ)
		തിരികെ ശൂന്യ;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_തിരുകുക(ring, ctx);

	തിരികെ ctx;
}

/** callback to free a resource fetch */
സ്ഥിതം ശൂന്യം fetch_javascript_free(ശൂന്യം *ctx)
{
	ഘടന fetch_javascript_context *c = ctx;
	എങ്കിൽ (c->url != ശൂന്യ) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	സ്വതന്ത്രം(ctx);
}

/** callback to start a resource fetch */
സ്ഥിതം bool fetch_javascript_start(ശൂന്യം *ctx)
{
	തിരികെ true;
}

/** callback to abort a resource fetch */
സ്ഥിതം ശൂന്യം fetch_javascript_abort(ശൂന്യം *ctx)
{
	ഘടന fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
സ്ഥിതം ശൂന്യം fetch_javascript_poll(lwc_string *scheme)
{
	ഘടന fetch_javascript_context *c, *next;

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
		എങ്കിൽ (c->aborted == false) {
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
	} ആകുമ്പോൾ ( (c = next) != ring && ring != ശൂന്യ);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_register(ശൂന്യം)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	സ്ഥിരം ഘടന fetcher_operation_table fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.setup = fetch_javascript_setup,
		.start = fetch_javascript_start,
		.abort = fetch_javascript_abort,
		.സ്വതന്ത്രം = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	തിരികെ fetcher_add(scheme, &fetcher_ops);
}
