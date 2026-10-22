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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <dom/dom.h>
#అన్తర్భావ <libwapcaplet/libwapcaplet.h>

#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "utils/config.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/ring.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "content/fetch.h"
#అన్తర్భావ "content/fetchers.h"

#అన్తర్భావ "html/private.h"

రకనిర్వచనం నిర్మాణం html_css_fetcher_అంశం {
	uint32_t కీలకం;
	dom_string *దత్తాంశం;
	nsurl *పునాది_url;

	నిర్మాణం html_css_fetcher_అంశం *r_తదుపరి, *r_prev;
} html_css_fetcher_అంశం;

రకనిర్వచనం నిర్మాణం html_css_fetcher_context {
	నిర్మాణం fetch *మాతృ_fetch;

	nsurl *url;
	html_css_fetcher_అంశం *అంశం;

	bool aborted;
	bool locked;

	నిర్మాణం html_css_fetcher_context *r_తదుపరి, *r_prev;
} html_css_fetcher_context;

స్థిర uint32_t current_కీలకం = 0;
స్థిర html_css_fetcher_అంశం *వస్తువులు = ఖాళీ;
స్థిర html_css_fetcher_context *ring = ఖాళీ;

స్థిర bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_దత్తాంశం(scheme));
	తిరిగి true;
}

స్థిర శూన్య html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_దత్తాంశం(scheme));
}

స్థిర bool html_css_fetcher_can_fetch(స్థిరరాశి nsurl *url)
{
	తిరిగి true;
}

స్థిర శూన్య *html_css_fetcher_అమరిక(నిర్మాణం fetch *మాతృ_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, స్థిరరాశి అక్షరం *post_urlenc,
		 స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *post_multipart,
		 స్థిరరాశి అక్షరం **శీర్షికలు)
{
	html_css_fetcher_context *ctx;
	lwc_string *మార్గం;
	uint32_t కీలకం;
	html_css_fetcher_అంశం *అంశం, *దొరికింది = ఖాళీ;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	మార్గం = nsurl_get_component(url, NSURL_మార్గం);
	/* The path must exist */
	ఒకవేళ (మార్గం == ఖాళీ) {
		తిరిగి ఖాళీ;
	}

	కీలకం = strtoul(lwc_string_దత్తాంశం(మార్గం), ఖాళీ, 10);

	lwc_string_unref(మార్గం);

	/* There must be at least one item */
	ఒకవేళ (వస్తువులు == ఖాళీ) {
		తిరిగి ఖాళీ;
	}

	అంశం = వస్తువులు;
	చేయి {
		ఒకవేళ (అంశం->కీలకం == కీలకం) {
			దొరికింది = అంశం;
			ఆపు;
		}

		అంశం = అంశం->r_తదుపరి;
	} అయితే (అంశం != వస్తువులు);

	/* We must have found the item */
	ఒకవేళ (దొరికింది == ఖాళీ) {
		తిరిగి ఖాళీ;
	}

	ctx = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*ctx));
	ఒకవేళ (ctx == ఖాళీ)
		తిరిగి ఖాళీ;

	ctx->మాతృ_fetch = మాతృ_fetch;
	ctx->url = nsurl_ref(url);
	ctx->అంశం = దొరికింది;

	RING_చొప్పించు(ring, ctx);

	తిరిగి ctx;
}

స్థిర bool html_css_fetcher_ప్రారంభం(శూన్య *ctx)
{
	తిరిగి true;
}

స్థిర శూన్య html_css_fetcher_free(శూన్య *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	ఒకవేళ (c->అంశం != ఖాళీ) {
		nsurl_unref(c->అంశం->పునాది_url);
		dom_string_unref(c->అంశం->దత్తాంశం);
		RING_REMOVE(వస్తువులు, c->అంశం);
		విడుదల(c->అంశం);
	}
	RING_REMOVE(ring, c);
	విడుదల(ctx);
}

స్థిర శూన్య html_css_fetcher_abort(శూన్య *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

స్థిర శూన్య html_css_fetcher_send_callback(స్థిరరాశి fetch_సందేశం *సందేశం,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(సందేశం, c->మాతృ_fetch);
	c->locked = false;
}

స్థిర శూన్య html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_సందేశం సందేశం;
	html_css_fetcher_context *c, *తదుపరి;

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
		ఒకవేళ (c->aborted) {
			/* Nothing to do */
			ప్రతిజ్ఞా(c->locked == false);
		} లేకపోతే ఒకవేళ (c->అంశం != ఖాళీ) {
			అక్షరం శీర్షిక[4096];

			fetch_సమితి_http_సంకేతం(c->మాతృ_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(శీర్షిక, పరిమాణం శీర్షిక,
				"Content-Type: text/css; charset=utf-8");
			సందేశం.type = FETCH_శీర్షిక;
			సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *) శీర్షిక;
			సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = అక్షరశ్రేణి_పొడవు(శీర్షిక);
			html_css_fetcher_send_callback(&సందేశం, c);

			ఒకవేళ (c->aborted == false) {
				snprintf(శీర్షిక, పరిమాణం శీర్షిక,
					"Content-Length: %"PRIsizet,
					dom_string_byte_నిడివి(c->అంశం->దత్తాంశం));
				సందేశం.type = FETCH_శీర్షిక;
				సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf =
						(స్థిరరాశి uint8_t *) శీర్షిక;
				సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = అక్షరశ్రేణి_పొడవు(శీర్షిక);
				html_css_fetcher_send_callback(&సందేశం, c);
			}

			ఒకవేళ (c->aborted == false) {
				snprintf(శీర్షిక, పరిమాణం శీర్షిక,
					"X-NS-Base: %.*s",
					(పూర్ణాంకం) nsurl_నిడివి(c->అంశం->పునాది_url),
					nsurl_access(c->అంశం->పునాది_url));
				సందేశం.type = FETCH_శీర్షిక;
				సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf =
						(స్థిరరాశి uint8_t *) శీర్షిక;
				సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = అక్షరశ్రేణి_పొడవు(శీర్షిక);
				html_css_fetcher_send_callback(&సందేశం, c);
			}

			ఒకవేళ (c->aborted == false) {
				సందేశం.type = FETCH_దత్తాంశం;
				సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf =
						(స్థిరరాశి uint8_t *)
						dom_string_దత్తాంశం(c->అంశం->దత్తాంశం);
				సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len =
					dom_string_byte_నిడివి(c->అంశం->దత్తాంశం);
				html_css_fetcher_send_callback(&సందేశం, c);
			}

			ఒకవేళ (c->aborted == false) {
				సందేశం.type = FETCH_ముగిసిన;
				html_css_fetcher_send_callback(&సందేశం, c);
			}
		} లేకపోతే {
			NSLOG(netsurf, INFO, "Processing of %s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't,
			 * then html_css_fetcher_process() is broken.
			 */
			ప్రతిజ్ఞా(c->locked == false);
		}

		/* Compute next fetch item at the last possible moment as
		 * processing this item may have added to the ring.
		 */
		తదుపరి = c->r_తదుపరి;

		fetch_remove_from_queues(c->మాతృ_fetch);
		fetch_free(c->మాతృ_fetch);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} అయితే ( (c = తదుపరి) != ring && ring != ఖాళీ);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_రిజిస్టరు(శూన్య)
{
	స్థిరరాశి నిర్మాణం fetcher_operation_పట్టిక html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.అమరిక = html_css_fetcher_అమరిక,
		.ప్రారంభం = html_css_fetcher_ప్రారంభం,
		.abort = html_css_fetcher_abort,
		.విడుదల = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	తిరిగి fetcher_కూడు(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_కూడు_అంశం(dom_string *దత్తాంశం, nsurl *పునాది_url, uint32_t *కీలకం)
{
	html_css_fetcher_అంశం *అంశం = జ్ఞాపకం_ఇవ్వు(పరిమాణం(*అంశం));

	ఒకవేళ (అంశం == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	*కీలకం = అంశం->కీలకం = current_కీలకం++;
	అంశం->దత్తాంశం = dom_string_ref(దత్తాంశం);
	అంశం->పునాది_url = nsurl_ref(పునాది_url);

	RING_చొప్పించు(వస్తువులు, అంశం);

	తిరిగి NSERROR_OK;
}
