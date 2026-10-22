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

#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <stdarg.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <libwapcaplet/libwapcaplet.h>
#অন্তর্ভাৱ <nsutils/base64.h>

#অন্তর্ভাৱ "netsurf/inttypes.h"
#অন্তর্ভাৱ "utils/url.h"
#অন্তর্ভাৱ "utils/nsurl.h"
#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/ring.h"

#অন্তর্ভাৱ "content/fetch.h"
#অন্তর্ভাৱ "content/fetchers.h"
#অন্তর্ভাৱ "content/fetchers/data.h"

সংরচনা fetch_data_context {
	সংরচনা fetch *parent_fetch;
	nsurl *url;
	ৱর্ণ *mimetype;
	ৱর্ণ *data;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	সংরচনা fetch_data_context *r_next, *r_prev;
};

স্থির সংরচনা fetch_data_context *ring = রিক্ত;

স্থির bool fetch_data_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_data(scheme));

	ঘূৰাই true;
}

স্থির শূন্য fetch_data_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_data(scheme));
}

স্থির bool fetch_data_can_fetch(ধ্রুৱ nsurl *url)
{
	ঘূৰাই true;
}

স্থির শূন্য fetch_data_send_callback(ধ্রুৱ fetch_বাৰ্তা *বাৰ্তা,
		সংরচনা fetch_data_context *c)
{
	c->locked = true;
	fetch_send_callback(বাৰ্তা, c->parent_fetch);
	c->locked = false;
}

স্থির শূন্য fetch_data_send_header(সংরচনা fetch_data_context *ctx,
		ধ্রুৱ ৱর্ণ *fmt, ...)
{
	ৱর্ণ header[64];
	fetch_বাৰ্তা বাৰ্তা;
	va_list ap;
	পূর্ণাঙ্ক len;

	va_start(ap, fmt);
	len = vsnprintf(header, পরিমাণ(header), fmt, ap);
	va_end(ap);

	যদি (len >= (পূর্ণাঙ্ক)পরিমাণ(header) || len < 0) {
		ঘূৰাই;
	}

	বাৰ্তা.type = FETCH_HEADER;
	বাৰ্তা.data.header_or_data.len = len;
	বাৰ্তা.data.header_or_data.buf = (ধ্রুৱ uint8_t *)header;
	fetch_data_send_callback(&বাৰ্তা, ctx);
}

স্থির শূন্য *fetch_data_setup(সংরচনা fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ধ্রুৱ ৱর্ণ *post_urlenc,
		 ধ্রুৱ সংরচনা fetch_multipart_data *post_multipart,
		 ধ্রুৱ ৱর্ণ **headers)
{
	সংরচনা fetch_data_context *ctx = calloc(1, পরিমাণ(*ctx));
	
	যদি (ctx == রিক্ত)
		ঘূৰাই রিক্ত;
		
	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);

	RING_ঢোকাও(ring, ctx);
	
	ঘূৰাই ctx;
}

স্থির bool fetch_data_start(শূন্য *ctx)
{
	ঘূৰাই true;
}

স্থির শূন্য fetch_data_free(শূন্য *ctx)
{
	সংরচনা fetch_data_context *c = ctx;

	nsurl_unref(c->url);
	মুক্ত(c->data);
	মুক্ত(c->mimetype);
	মুক্ত(ctx);
}

স্থির শূন্য fetch_data_abort(শূন্য *ctx)
{
	সংরচনা fetch_data_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

স্থির bool fetch_data_process(সংরচনা fetch_data_context *c)
{
	nserror res;
	fetch_বাৰ্তা বাৰ্তা;
	ধ্রুৱ ৱর্ণ *params;
	ধ্রুৱ ৱর্ণ *comma;
	ৱর্ণ *unescaped;
	size_t unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	যদি (nsurl_length(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		বাৰ্তা.type = FETCH_ERROR;
		বাৰ্তা.data.error = "Malformed data: URL";
		fetch_data_send_callback(&বাৰ্তা, c);
		ঘূৰাই false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	যদি ( (comma = strchr(params, ',')) == রিক্ত) {
		বাৰ্তা.type = FETCH_ERROR;
		বাৰ্তা.data.error = "Malformed data: URL";
		fetch_data_send_callback(&বাৰ্তা, c);
		ঘূৰাই false;
	}
	
	যদি (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} অন্যথা {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	যদি (c->mimetype == রিক্ত) {
		বাৰ্তা.type = FETCH_ERROR;
		বাৰ্তা.data.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_data_send_callback(&বাৰ্তা, c);
		ঘূৰাই false;
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
		বাৰ্তা.type = FETCH_ERROR;
		বাৰ্তা.data.error = "Unable to URL decode data: URL";
		fetch_data_send_callback(&বাৰ্তা, c);
		ঘূৰাই false;
	}
	
	যদি (c->base64) {
		যদি ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->data,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->data == রিক্ত)) {
			বাৰ্তা.type = FETCH_ERROR;
			বাৰ্তা.data.error = "Unable to Base64 decode data: URL";
			fetch_data_send_callback(&বাৰ্তা, c);
			মুক্ত(unescaped);
			ঘূৰাই false;
		}
		মুক্ত(unescaped);
	} অন্যথা {
		c->datalen = unescaped_len;
		c->data = unescaped;
	}
	
	
	ঘূৰাই true;
}

স্থির শূন্য fetch_data_poll(lwc_string *scheme)
{
	fetch_বাৰ্তা বাৰ্তা;
	সংরচনা fetch_data_context *c, *save_ring = রিক্ত;
	
	/* Iterate over ring, processing each pending fetch */
	যেতিয়ালৈ (ring != রিক্ত) {
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
			অনুৱর্ত;
		}

		/* Only process non-aborted fetches */
		যদি (c->aborted == false && fetch_data_process(c) == true) {
			fetch_set_http_code(c->parent_fetch, 200);
			NSLOG(netsurf, INFO,
			      "setting data: MIME type to %s, length to %"PRIsizet,
			      c->mimetype,
			      c->datalen);
			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to fetch_data_send_callback().
			 */
			fetch_data_send_header(c, "Content-Type: %s",
					c->mimetype);

			যদি (c->aborted == false) {
				fetch_data_send_header(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			যদি (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_data_send_header(c, "Cache-Control: "
						"max-age=31536000");
			}

			যদি (c->aborted == false) {
				বাৰ্তা.type = FETCH_DATA;
				বাৰ্তা.data.header_or_data.buf = 
						(ধ্রুৱ uint8_t *) c->data;
				বাৰ্তা.data.header_or_data.len = c->datalen;
				fetch_data_send_callback(&বাৰ্তা, c);
			}

			যদি (c->aborted == false) {
				বাৰ্তা.type = FETCH_FINISHED;
				fetch_data_send_callback(&বাৰ্তা, c);
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

nserror fetch_data_register(শূন্য)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_data);
	ধ্রুৱ সংরচনা fetcher_operation_table fetcher_ops = {
		.initialise = fetch_data_initialise,
		.acceptable = fetch_data_can_fetch,
		.setup = fetch_data_setup,
		.start = fetch_data_start,
		.abort = fetch_data_abort,
		.মুক্ত = fetch_data_free,
		.poll = fetch_data_poll,
		.finalise = fetch_data_finalise
	};

	ঘূৰাই fetcher_add(scheme, &fetcher_ops);
}
