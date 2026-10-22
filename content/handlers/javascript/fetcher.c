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

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <libwapcaplet/libwapcaplet.h>

#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/ring.h"
#అన్తర్భావ "content/fetch.h"
#అన్తర్భావ "content/fetchers.h"

#అన్తర్భావ "javascript/fetcher.h"

/** Context for an resource fetch */
నిర్మాణం fetch_javascript_context {
	నిర్మాణం fetch_javascript_context *r_తదుపరి, *r_prev;

	నిర్మాణం fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

స్థిర నిర్మాణం fetch_javascript_context *ring = ఖాళీ;


/** issue fetch callbacks with locking */
స్థిర అంతఃవరుస bool fetch_javascript_send_callback(స్థిరరాశి fetch_సందేశం *సందేశం,
		నిర్మాణం fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(సందేశం, ctx->fetchh);
	ctx->locked = false;

	తిరిగి ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
స్థిర bool fetch_javascript_handler(నిర్మాణం fetch_javascript_context *ctx)
{
	fetch_సందేశం సందేశం;
	పూర్ణాంకం సంకేతం_2 = 204;

	/* content is going to return error code */
	fetch_సమితి_http_సంకేతం(ctx->fetchh, సంకేతం_2);

	సందేశం.type = FETCH_ముగిసిన;
	fetch_javascript_send_callback(&సందేశం, ctx);

	తిరిగి true;
}



/** callback to initialise the resource fetcher. */
స్థిర bool fetch_javascript_initialise(lwc_string *scheme)
{
	తిరిగి true;
}

/** callback to finalise the resource fetcher. */
స్థిర శూన్య fetch_javascript_finalise(lwc_string *scheme)
{
}

స్థిర bool fetch_javascript_can_fetch(స్థిరరాశి nsurl *url)
{
	తిరిగి true;
}

/** callback to set up a resource fetch context. */
స్థిర శూన్య *
fetch_javascript_అమరిక(నిర్మాణం fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 స్థిరరాశి అక్షరం *post_urlenc,
		 స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *post_multipart,
		 స్థిరరాశి అక్షరం **శీర్షికలు)
{
	నిర్మాణం fetch_javascript_context *ctx;

	ctx = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*ctx));
	ఒకవేళ (ctx == ఖాళీ)
		తిరిగి ఖాళీ;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_చొప్పించు(ring, ctx);

	తిరిగి ctx;
}

/** callback to free a resource fetch */
స్థిర శూన్య fetch_javascript_free(శూన్య *ctx)
{
	నిర్మాణం fetch_javascript_context *c = ctx;
	ఒకవేళ (c->url != ఖాళీ) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	విడుదల(ctx);
}

/** callback to start a resource fetch */
స్థిర bool fetch_javascript_ప్రారంభం(శూన్య *ctx)
{
	తిరిగి true;
}

/** callback to abort a resource fetch */
స్థిర శూన్య fetch_javascript_abort(శూన్య *ctx)
{
	నిర్మాణం fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
స్థిర శూన్య fetch_javascript_poll(lwc_string *scheme)
{
	నిర్మాణం fetch_javascript_context *c, *తదుపరి;

	ఒకవేళ (ring == ఖాళీ) తిరిగి;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	చేయి {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ఒకవేళ (c->locked == true) {
			తదుపరి = c->r_తదుపరి;
			కొనసాగు;
		}

		/* Only process non-aborted fetches */
		ఒకవేళ (c->aborted == false) {
			/* resource fetches can be processed in one go */
			fetch_javascript_handler(c);
		}

		/* Compute next fetch item at the last possible moment
		 * as processing this item may have added to the ring
		 */
		తదుపరి = c->r_తదుపరి;

		fetch_remove_from_queues(c->fetchh);
		fetch_free(c->fetchh);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} అయితే ( (c = తదుపరి) != ring && ring != ఖాళీ);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_రిజిస్టరు(శూన్య)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	స్థిరరాశి నిర్మాణం fetcher_operation_పట్టిక fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.అమరిక = fetch_javascript_అమరిక,
		.ప్రారంభం = fetch_javascript_ప్రారంభం,
		.abort = fetch_javascript_abort,
		.విడుదల = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	తిరిగి fetcher_కూడు(scheme, &fetcher_ops);
}
