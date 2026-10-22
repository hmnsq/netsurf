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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <libwapcaplet/libwapcaplet.h>
#অন্তর্ভাব <nsutils/base64.h>

#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "utils/url.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/ring.h"

#অন্তর্ভাব "content/fetch.h"
#অন্তর্ভাব "content/fetchers.h"
#অন্তর্ভাব "content/fetchers/data.h"

কাঠামো fetch_তথ্য_context {
	কাঠামো fetch *parent_fetch;
	nsurl *url;
	অক্ষর *mimetype;
	অক্ষর *তথ্য;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	কাঠামো fetch_তথ্য_context *r_পরবর্তী, *r_পূর্ব;
};

স্থির কাঠামো fetch_তথ্য_context *ring = ফাঁকা;

স্থির bool fetch_তথ্য_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_তথ্য(scheme));

	ফেরত true;
}

স্থির শূন্য fetch_তথ্য_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_তথ্য(scheme));
}

স্থির bool fetch_তথ্য_can_fetch(ধ্রুবক nsurl *url)
{
	ফেরত true;
}

স্থির শূন্য fetch_তথ্য_send_callback(ধ্রুবক fetch_বার্তা *বার্তা,
		কাঠামো fetch_তথ্য_context *c)
{
	c->locked = true;
	fetch_send_callback(বার্তা, c->parent_fetch);
	c->locked = false;
}

স্থির শূন্য fetch_তথ্য_send_header(কাঠামো fetch_তথ্য_context *ctx,
		ধ্রুবক অক্ষর *fmt, ...)
{
	অক্ষর header[64];
	fetch_বার্তা বার্তা;
	va_list ap;
	পূর্ণসংখ্যা len;

	va_start(ap, fmt);
	len = vsnprintf(header, আকার(header), fmt, ap);
	va_end(ap);

	যদি (len >= (পূর্ণসংখ্যা)আকার(header) || len < 0) {
		ফেরত;
	}

	বার্তা.type = FETCH_HEADER;
	বার্তা.তথ্য.header_or_তথ্য.len = len;
	বার্তা.তথ্য.header_or_তথ্য.buf = (ধ্রুবক uint8_t *)header;
	fetch_তথ্য_send_callback(&বার্তা, ctx);
}

স্থির শূন্য *fetch_তথ্য_setup(কাঠামো fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ধ্রুবক অক্ষর *post_urlenc,
		 ধ্রুবক কাঠামো fetch_multipart_তথ্য *post_multipart,
		 ধ্রুবক অক্ষর **headers)
{
	কাঠামো fetch_তথ্য_context *ctx = calloc(1, আকার(*ctx));
	
	যদি (ctx == ফাঁকা)
		ফেরত ফাঁকা;
		
	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);

	RING_ঢোকাও(ring, ctx);
	
	ফেরত ctx;
}

স্থির bool fetch_তথ্য_start(শূন্য *ctx)
{
	ফেরত true;
}

স্থির শূন্য fetch_তথ্য_free(শূন্য *ctx)
{
	কাঠামো fetch_তথ্য_context *c = ctx;

	nsurl_unref(c->url);
	মুক্ত(c->তথ্য);
	মুক্ত(c->mimetype);
	মুক্ত(ctx);
}

স্থির শূন্য fetch_তথ্য_abort(শূন্য *ctx)
{
	কাঠামো fetch_তথ্য_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

স্থির bool fetch_তথ্য_process(কাঠামো fetch_তথ্য_context *c)
{
	nserror res;
	fetch_বার্তা বার্তা;
	ধ্রুবক অক্ষর *params;
	ধ্রুবক অক্ষর *comma;
	অক্ষর *unescaped;
	size_t unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	যদি (nsurl_দৈর্ঘ্য(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		বার্তা.type = FETCH_ERROR;
		বার্তা.তথ্য.error = "Malformed data: URL";
		fetch_তথ্য_send_callback(&বার্তা, c);
		ফেরত false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	যদি ( (comma = strchr(params, ',')) == ফাঁকা) {
		বার্তা.type = FETCH_ERROR;
		বার্তা.তথ্য.error = "Malformed data: URL";
		fetch_তথ্য_send_callback(&বার্তা, c);
		ফেরত false;
	}
	
	যদি (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} অন্যথা {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	যদি (c->mimetype == ফাঁকা) {
		বার্তা.type = FETCH_ERROR;
		বার্তা.তথ্য.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_তথ্য_send_callback(&বার্তা, c);
		ফেরত false;
	}
	
	যদি (strcmp(c->mimetype + strlen(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[strlen(c->mimetype) - 7] = '\0';
	} অন্যথা {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	যদি (res != NSERROR_OK) {
		বার্তা.type = FETCH_ERROR;
		বার্তা.তথ্য.error = "Unable to URL decode data: URL";
		fetch_তথ্য_send_callback(&বার্তা, c);
		ফেরত false;
	}
	
	যদি (c->base64) {
		যদি ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->তথ্য,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->তথ্য == ফাঁকা)) {
			বার্তা.type = FETCH_ERROR;
			বার্তা.তথ্য.error = "Unable to Base64 decode data: URL";
			fetch_তথ্য_send_callback(&বার্তা, c);
			মুক্ত(unescaped);
			ফেরত false;
		}
		মুক্ত(unescaped);
	} অন্যথা {
		c->datalen = unescaped_len;
		c->তথ্য = unescaped;
	}
	
	
	ফেরত true;
}

স্থির শূন্য fetch_তথ্য_poll(lwc_string *scheme)
{
	fetch_বার্তা বার্তা;
	কাঠামো fetch_তথ্য_context *c, *save_ring = ফাঁকা;
	
	/* Iterate over ring, processing each pending fetch */
	যখন (ring != ফাঁকা) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called 
		 * again.
		 */
		যদি (c->locked == true) {
			RING_ঢোকাও(save_ring, c);
			চালু;
		}

		/* Only process non-aborted fetches */
		যদি (c->aborted == false && fetch_তথ্য_process(c) == true) {
			fetch_set_http_code(c->parent_fetch, 200);
			NSLOG(netsurf, INFO,
			      "setting data: MIME type to %s, length to %"PRIsizet,
			      c->mimetype,
			      c->datalen);
			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to fetch_data_send_callback().
			 */
			fetch_তথ্য_send_header(c, "Content-Type: %s",
					c->mimetype);

			যদি (c->aborted == false) {
				fetch_তথ্য_send_header(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			যদি (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_তথ্য_send_header(c, "Cache-Control: "
						"max-age=31536000");
			}

			যদি (c->aborted == false) {
				বার্তা.type = FETCH_তথ্য;
				বার্তা.তথ্য.header_or_তথ্য.buf = 
						(ধ্রুবক uint8_t *) c->তথ্য;
				বার্তা.তথ্য.header_or_তথ্য.len = c->datalen;
				fetch_তথ্য_send_callback(&বার্তা, c);
			}

			যদি (c->aborted == false) {
				বার্তা.type = FETCH_FINISHED;
				fetch_তথ্য_send_callback(&বার্তা, c);
			}
		} অন্যথা {
			NSLOG(netsurf, INFO, "Processing of %.140s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't, 
			 * then fetch_data_process() is broken.
			 */
			প্রতিজ্ঞা(c->locked == false);
		}

		/* And now finish */
		fetch_remove_from_queues(c->parent_fetch);
		fetch_free(c->parent_fetch);
	}

	/* Finally, if we saved any fetches which were locked, put them back
	 * into the ring for next time
	 */
	ring = save_ring;
}

nserror fetch_তথ্য_register(শূন্য)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_তথ্য);
	ধ্রুবক কাঠামো fetcher_operation_table fetcher_ops = {
		.initialise = fetch_তথ্য_initialise,
		.acceptable = fetch_তথ্য_can_fetch,
		.setup = fetch_তথ্য_setup,
		.start = fetch_তথ্য_start,
		.abort = fetch_তথ্য_abort,
		.মুক্ত = fetch_তথ্য_free,
		.poll = fetch_তথ্য_poll,
		.finalise = fetch_তথ্য_finalise
	};

	ফেরত fetcher_add(scheme, &fetcher_ops);
}
