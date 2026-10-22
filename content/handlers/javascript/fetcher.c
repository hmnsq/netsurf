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

#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <libwapcaplet/libwapcaplet.h>

#ضمن "utils/nsurl.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/ring.h"
#ضمن "content/fetch.h"
#ضمن "content/fetchers.h"

#ضمن "javascript/fetcher.h"

/** Context for an resource fetch */
بنية fetch_javascript_context {
	بنية fetch_javascript_context *r_next, *r_prev;

	بنية fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

ساكن بنية fetch_javascript_context *ring = عدم;


/** issue fetch callbacks with locking */
ساكن مضمن bool fetch_javascript_send_callback(ثابت fetch_msg *msg,
		بنية fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(msg, ctx->fetchh);
	ctx->locked = false;

	أرجع ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
ساكن bool fetch_javascript_handler(بنية fetch_javascript_context *ctx)
{
	fetch_msg msg;
	صحيح code = 204;

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	msg.type = FETCH_FINISHED;
	fetch_javascript_send_callback(&msg, ctx);

	أرجع true;
}



/** callback to initialise the resource fetcher. */
ساكن bool fetch_javascript_initialise(lwc_string *scheme)
{
	أرجع true;
}

/** callback to finalise the resource fetcher. */
ساكن فراغ fetch_javascript_finalise(lwc_string *scheme)
{
}

ساكن bool fetch_javascript_can_fetch(ثابت nsurl *url)
{
	أرجع true;
}

/** callback to set up a resource fetch context. */
ساكن فراغ *
fetch_javascript_setup(بنية fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_data *post_multipart,
		 ثابت حرف **headers)
{
	بنية fetch_javascript_context *ctx;

	ctx = calloc(1, حجم(*ctx));
	𐡄𐡍 (ctx == عدم)
		أرجع عدم;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_INSERT(ring, ctx);

	أرجع ctx;
}

/** callback to free a resource fetch */
ساكن فراغ fetch_javascript_free(فراغ *ctx)
{
	بنية fetch_javascript_context *c = ctx;
	𐡄𐡍 (c->url != عدم) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	حرر(ctx);
}

/** callback to start a resource fetch */
ساكن bool fetch_javascript_start(فراغ *ctx)
{
	أرجع true;
}

/** callback to abort a resource fetch */
ساكن فراغ fetch_javascript_abort(فراغ *ctx)
{
	بنية fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ساكن فراغ fetch_javascript_poll(lwc_string *scheme)
{
	بنية fetch_javascript_context *c, *next;

	𐡄𐡍 (ring == عدم) أرجع;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	𐡏𐡁𐡃 {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		𐡄𐡍 (c->locked == true) {
			next = c->r_next;
			استمر;
		}

		/* Only process non-aborted fetches */
		𐡄𐡍 (c->aborted == false) {
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
	} طالما ( (c = next) != ring && ring != عدم);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_register(فراغ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	ثابت بنية fetcher_operation_table fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.setup = fetch_javascript_setup,
		.start = fetch_javascript_start,
		.abort = fetch_javascript_abort,
		.حرر = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	أرجع fetcher_add(scheme, &fetcher_ops);
}
