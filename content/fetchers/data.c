/*
 * Copyright 2008 Rob Kendrick <rjek@netsurf-browser.org>
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
 * data scheme handling.  See http://tools.ietf.org/html/rfc2397
 */

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <libwapcaplet/libwapcaplet.h>
#అన్తర్భావ <nsutils/base64.h>

#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "utils/url.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/ring.h"

#అన్తర్భావ "content/fetch.h"
#అన్తర్భావ "content/fetchers.h"
#అన్తర్భావ "content/fetchers/data.h"

నిర్మాణం fetch_దత్తాంశం_context {
	నిర్మాణం fetch *మాతృ_fetch;
	nsurl *url;
	అక్షరం *mimetype;
	అక్షరం *దత్తాంశం;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	నిర్మాణం fetch_దత్తాంశం_context *r_తదుపరి, *r_prev;
};

స్థిర నిర్మాణం fetch_దత్తాంశం_context *ring = ఖాళీ;

స్థిర bool fetch_దత్తాంశం_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_దత్తాంశం(scheme));

	తిరిగి true;
}

స్థిర శూన్య fetch_దత్తాంశం_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_దత్తాంశం(scheme));
}

స్థిర bool fetch_దత్తాంశం_can_fetch(స్థిరరాశి nsurl *url)
{
	తిరిగి true;
}

స్థిర శూన్య fetch_దత్తాంశం_send_callback(స్థిరరాశి fetch_సందేశం *సందేశం,
		నిర్మాణం fetch_దత్తాంశం_context *c)
{
	c->locked = true;
	fetch_send_callback(సందేశం, c->మాతృ_fetch);
	c->locked = false;
}

స్థిర శూన్య fetch_దత్తాంశం_send_శీర్షిక(నిర్మాణం fetch_దత్తాంశం_context *ctx,
		స్థిరరాశి అక్షరం *fmt, ...)
{
	అక్షరం శీర్షిక[64];
	fetch_సందేశం సందేశం;
	va_list ap;
	పూర్ణాంకం len;

	va_start(ap, fmt);
	len = vsnprintf(శీర్షిక, పరిమాణం(శీర్షిక), fmt, ap);
	va_end(ap);

	ఒకవేళ (len >= (పూర్ణాంకం)పరిమాణం(శీర్షిక) || len < 0) {
		తిరిగి;
	}

	సందేశం.type = FETCH_శీర్షిక;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = len;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *)శీర్షిక;
	fetch_దత్తాంశం_send_callback(&సందేశం, ctx);
}

స్థిర శూన్య *fetch_దత్తాంశం_అమరిక(నిర్మాణం fetch *మాతృ_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, స్థిరరాశి అక్షరం *post_urlenc,
		 స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *post_multipart,
		 స్థిరరాశి అక్షరం **శీర్షికలు)
{
	నిర్మాణం fetch_దత్తాంశం_context *ctx = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*ctx));
	
	ఒకవేళ (ctx == ఖాళీ)
		తిరిగి ఖాళీ;
		
	ctx->మాతృ_fetch = మాతృ_fetch;
	ctx->url = nsurl_ref(url);

	RING_చొప్పించు(ring, ctx);
	
	తిరిగి ctx;
}

స్థిర bool fetch_దత్తాంశం_ప్రారంభం(శూన్య *ctx)
{
	తిరిగి true;
}

స్థిర శూన్య fetch_దత్తాంశం_free(శూన్య *ctx)
{
	నిర్మాణం fetch_దత్తాంశం_context *c = ctx;

	nsurl_unref(c->url);
	విడుదల(c->దత్తాంశం);
	విడుదల(c->mimetype);
	విడుదల(ctx);
}

స్థిర శూన్య fetch_దత్తాంశం_abort(శూన్య *ctx)
{
	నిర్మాణం fetch_దత్తాంశం_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

స్థిర bool fetch_దత్తాంశం_process(నిర్మాణం fetch_దత్తాంశం_context *c)
{
	nserror res;
	fetch_సందేశం సందేశం;
	స్థిరరాశి అక్షరం *params;
	స్థిరరాశి అక్షరం *comma;
	అక్షరం *unescaped;
	size_t unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	ఒకవేళ (nsurl_నిడివి(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		సందేశం.type = FETCH_ERROR;
		సందేశం.దత్తాంశం.error = "Malformed data: URL";
		fetch_దత్తాంశం_send_callback(&సందేశం, c);
		తిరిగి false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	ఒకవేళ ( (comma = strchr(params, ',')) == ఖాళీ) {
		సందేశం.type = FETCH_ERROR;
		సందేశం.దత్తాంశం.error = "Malformed data: URL";
		fetch_దత్తాంశం_send_callback(&సందేశం, c);
		తిరిగి false;
	}
	
	ఒకవేళ (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} లేకపోతే {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	ఒకవేళ (c->mimetype == ఖాళీ) {
		సందేశం.type = FETCH_ERROR;
		సందేశం.దత్తాంశం.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_దత్తాంశం_send_callback(&సందేశం, c);
		తిరిగి false;
	}
	
	ఒకవేళ (strcmp(c->mimetype + అక్షరశ్రేణి_పొడవు(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[అక్షరశ్రేణి_పొడవు(c->mimetype) - 7] = '\0';
	} లేకపోతే {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	ఒకవేళ (res != NSERROR_OK) {
		సందేశం.type = FETCH_ERROR;
		సందేశం.దత్తాంశం.error = "Unable to URL decode data: URL";
		fetch_దత్తాంశం_send_callback(&సందేశం, c);
		తిరిగి false;
	}
	
	ఒకవేళ (c->base64) {
		ఒకవేళ ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->దత్తాంశం,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->దత్తాంశం == ఖాళీ)) {
			సందేశం.type = FETCH_ERROR;
			సందేశం.దత్తాంశం.error = "Unable to Base64 decode data: URL";
			fetch_దత్తాంశం_send_callback(&సందేశం, c);
			విడుదల(unescaped);
			తిరిగి false;
		}
		విడుదల(unescaped);
	} లేకపోతే {
		c->datalen = unescaped_len;
		c->దత్తాంశం = unescaped;
	}
	
	
	తిరిగి true;
}

స్థిర శూన్య fetch_దత్తాంశం_poll(lwc_string *scheme)
{
	fetch_సందేశం సందేశం;
	నిర్మాణం fetch_దత్తాంశం_context *c, *save_ring = ఖాళీ;
	
	/* Iterate over ring, processing each pending fetch */
	అయితే (ring != ఖాళీ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called 
		 * again.
		 */
		ఒకవేళ (c->locked == true) {
			RING_చొప్పించు(save_ring, c);
			కొనసాగు;
		}

		/* Only process non-aborted fetches */
		ఒకవేళ (c->aborted == false && fetch_దత్తాంశం_process(c) == true) {
			fetch_సమితి_http_సంకేతం(c->మాతృ_fetch, 200);
			NSLOG(netsurf, INFO,
			      "setting data: MIME type to %s, length to %"PRIsizet,
			      c->mimetype,
			      c->datalen);
			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to fetch_data_send_callback().
			 */
			fetch_దత్తాంశం_send_శీర్షిక(c, "Content-Type: %s",
					c->mimetype);

			ఒకవేళ (c->aborted == false) {
				fetch_దత్తాంశం_send_శీర్షిక(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			ఒకవేళ (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_దత్తాంశం_send_శీర్షిక(c, "Cache-Control: "
						"max-age=31536000");
			}

			ఒకవేళ (c->aborted == false) {
				సందేశం.type = FETCH_దత్తాంశం;
				సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = 
						(స్థిరరాశి uint8_t *) c->దత్తాంశం;
				సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = c->datalen;
				fetch_దత్తాంశం_send_callback(&సందేశం, c);
			}

			ఒకవేళ (c->aborted == false) {
				సందేశం.type = FETCH_ముగిసిన;
				fetch_దత్తాంశం_send_callback(&సందేశం, c);
			}
		} లేకపోతే {
			NSLOG(netsurf, INFO, "Processing of %.140s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't, 
			 * then fetch_data_process() is broken.
			 */
			ప్రతిజ్ఞా(c->locked == false);
		}

		/* And now finish */
		fetch_remove_from_queues(c->మాతృ_fetch);
		fetch_free(c->మాతృ_fetch);
	}

	/* Finally, if we saved any fetches which were locked, put them back
	 * into the ring for next time
	 */
	ring = save_ring;
}

nserror fetch_దత్తాంశం_రిజిస్టరు(శూన్య)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_దత్తాంశం);
	స్థిరరాశి నిర్మాణం fetcher_operation_పట్టిక fetcher_ops = {
		.initialise = fetch_దత్తాంశం_initialise,
		.acceptable = fetch_దత్తాంశం_can_fetch,
		.అమరిక = fetch_దత్తాంశం_అమరిక,
		.ప్రారంభం = fetch_దత్తాంశం_ప్రారంభం,
		.abort = fetch_దత్తాంశం_abort,
		.విడుదల = fetch_దత్తాంశం_free,
		.poll = fetch_దత్తాంశం_poll,
		.finalise = fetch_దత్తాంశం_finalise
	};

	తిరిగి fetcher_కూడు(scheme, &fetcher_ops);
}
