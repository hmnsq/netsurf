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
	بنية fetch_javascript_context *r_بل, *r_prev;

	بنية fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

ساكن بنية fetch_javascript_context *ring = عدم;


/** issue fetch callbacks with locking */
ساكن مضمن bool fetch_javascript_send_callback(ثابت fetch_پیغام *پیغام,
		بنية fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پیغام, ctx->fetchh);
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
	fetch_پیغام پیغام;
	صحيح کوډ = 204;

	/* content is going to return error code */
	fetch_سېټ_http_کوډ(ctx->fetchh, کوډ);

	پیغام.type = FETCH_پای_ته_رسېدلی;
	fetch_javascript_send_callback(&پیغام, ctx);

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
fetch_javascript_تیاری(بنية fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_معلومات *post_multipart,
		 ثابت حرف **سرلیکونه)
{
	بنية fetch_javascript_context *ctx;

	ctx = حافظه_صفر_ورکړه(1, حجم(*ctx));
	إذا (ctx == عدم)
		أرجع عدم;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_ورننباسه(ring, ctx);

	أرجع ctx;
}

/** callback to free a resource fetch */
ساكن فراغ fetch_javascript_free(فراغ *ctx)
{
	بنية fetch_javascript_context *c = ctx;
	إذا (c->url != عدم) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	حرر(ctx);
}

/** callback to start a resource fetch */
ساكن bool fetch_javascript_پیل(فراغ *ctx)
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
	بنية fetch_javascript_context *c, *بل;

	إذا (ring == عدم) أرجع;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	افعل {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		إذا (c->locked == true) {
			بل = c->r_بل;
			استمر;
		}

		/* Only process non-aborted fetches */
		إذا (c->aborted == false) {
			/* resource fetches can be processed in one go */
			fetch_javascript_handler(c);
		}

		/* Compute next fetch item at the last possible moment
		 * as processing this item may have added to the ring
		 */
		بل = c->r_بل;

		fetch_remove_from_queues(c->fetchh);
		fetch_free(c->fetchh);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} طالما ( (c = بل) != ring && ring != عدم);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_ثبتوونکی(فراغ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	ثابت بنية fetcher_operation_جدول fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.تیاری = fetch_javascript_تیاری,
		.پیل = fetch_javascript_پیل,
		.abort = fetch_javascript_abort,
		.حرر = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	أرجع fetcher_جمع_کړه(scheme, &fetcher_ops);
}
