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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <libwapcaplet/libwapcaplet.h>

#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/ring.h"
#अन्तर्भाव "content/fetch.h"
#अन्तर्भाव "content/fetchers.h"

#अन्तर्भाव "javascript/fetcher.h"

/** Context for an resource fetch */
संरचना fetch_javascript_context {
	संरचना fetch_javascript_context *r_अग्र, *r_पूर्व;

	संरचना fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

स्थिर संरचना fetch_javascript_context *ring = रिक्त;


/** issue fetch callbacks with locking */
स्थिर अन्तःस्थ bool fetch_javascript_send_callback(ध्रुव fetch_सन्देशः *सन्देशः,
		संरचना fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(सन्देशः, ctx->fetchh);
	ctx->locked = false;

	प्रत्यागम ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
स्थिर bool fetch_javascript_handler(संरचना fetch_javascript_context *ctx)
{
	fetch_सन्देशः सन्देशः;
	पूर्णाङ्क संहिता = 204;

	/* content is going to return error code */
	fetch_समुच्चयः_http_संहिता(ctx->fetchh, संहिता);

	सन्देशः.type = FETCH_परिसमाप्तम्;
	fetch_javascript_send_callback(&सन्देशः, ctx);

	प्रत्यागम true;
}



/** callback to initialise the resource fetcher. */
स्थिर bool fetch_javascript_initialise(lwc_string *scheme)
{
	प्रत्यागम true;
}

/** callback to finalise the resource fetcher. */
स्थिर शून्य fetch_javascript_finalise(lwc_string *scheme)
{
}

स्थिर bool fetch_javascript_can_fetch(ध्रुव nsurl *url)
{
	प्रत्यागम true;
}

/** callback to set up a resource fetch context. */
स्थिर शून्य *
fetch_javascript_सज्जता(संरचना fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 ध्रुव वर्ण *post_urlenc,
		 ध्रुव संरचना fetch_multipart_दत्त *post_multipart,
		 ध्रुव वर्ण **शीर्षकाणि)
{
	संरचना fetch_javascript_context *ctx;

	ctx = शून्यस्मृतिं_देहि(1, परिमाण(*ctx));
	यदि (ctx == रिक्त)
		प्रत्यागम रिक्त;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_निवेशय(ring, ctx);

	प्रत्यागम ctx;
}

/** callback to free a resource fetch */
स्थिर शून्य fetch_javascript_free(शून्य *ctx)
{
	संरचना fetch_javascript_context *c = ctx;
	यदि (c->url != रिक्त) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	मुक्त(ctx);
}

/** callback to start a resource fetch */
स्थिर bool fetch_javascript_start(शून्य *ctx)
{
	प्रत्यागम true;
}

/** callback to abort a resource fetch */
स्थिर शून्य fetch_javascript_abort(शून्य *ctx)
{
	संरचना fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
स्थिर शून्य fetch_javascript_poll(lwc_string *scheme)
{
	संरचना fetch_javascript_context *c, *अग्र;

	यदि (ring == रिक्त) प्रत्यागम;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	कुरु {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		यदि (c->locked == true) {
			अग्र = c->r_अग्र;
			अनुवर्त;
		}

		/* Only process non-aborted fetches */
		यदि (c->aborted == false) {
			/* resource fetches can be processed in one go */
			fetch_javascript_handler(c);
		}

		/* Compute next fetch item at the last possible moment
		 * as processing this item may have added to the ring
		 */
		अग्र = c->r_अग्र;

		fetch_remove_from_queues(c->fetchh);
		fetch_free(c->fetchh);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} यावत् ( (c = अग्र) != ring && ring != रिक्त);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_पञ्जिकाकोशः(शून्य)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	ध्रुव संरचना fetcher_operation_सारणी fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.सज्जता = fetch_javascript_सज्जता,
		.start = fetch_javascript_start,
		.abort = fetch_javascript_abort,
		.मुक्त = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	प्रत्यागम fetcher_सङ्कलय(scheme, &fetcher_ops);
}
