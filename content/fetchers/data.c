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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <libwapcaplet/libwapcaplet.h>
#ଅନ୍ତର୍ଭାଵ <nsutils/base64.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "utils/url.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/ring.h"

#ଅନ୍ତର୍ଭାଵ "content/fetch.h"
#ଅନ୍ତର୍ଭାଵ "content/fetchers.h"
#ଅନ୍ତର୍ଭାଵ "content/fetchers/data.h"

ସଂରଚନା fetch_data_context {
	ସଂରଚନା fetch *parent_fetch;
	nsurl *url;
	ଅକ୍ଷର *mimetype;
	ଅକ୍ଷର *data;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	ସଂରଚନା fetch_data_context *r_next, *r_prev;
};

ସ୍ଥିର ସଂରଚନା fetch_data_context *ring = ଖାଲି;

ସ୍ଥିର bool fetch_data_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_data(scheme));

	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର ଶୂନ୍ୟ fetch_data_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_data(scheme));
}

ସ୍ଥିର bool fetch_data_can_fetch(ଅଚଳ nsurl *url)
{
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର ଶୂନ୍ୟ fetch_data_send_callback(ଅଚଳ fetch_ସନ୍ଦେଶ *ସନ୍ଦେଶ,
		ସଂରଚନା fetch_data_context *c)
{
	c->locked = true;
	fetch_send_callback(ସନ୍ଦେଶ, c->parent_fetch);
	c->locked = false;
}

ସ୍ଥିର ଶୂନ୍ୟ fetch_data_send_header(ସଂରଚନା fetch_data_context *ctx,
		ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	ଅକ୍ଷର header[64];
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	va_list ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;

	va_start(ap, fmt);
	len = vsnprintf(header, ଆକାର(header), fmt, ap);
	va_end(ap);

	ଯଦି (len >= (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଆକାର(header) || len < 0) {
		ଫେରନ୍ତୁ;
	}

	ସନ୍ଦେଶ.type = FETCH_HEADER;
	ସନ୍ଦେଶ.data.header_or_data.len = len;
	ସନ୍ଦେଶ.data.header_or_data.buf = (ଅଚଳ uint8_t *)header;
	fetch_data_send_callback(&ସନ୍ଦେଶ, ctx);
}

ସ୍ଥିର ଶୂନ୍ୟ *fetch_data_setup(ସଂରଚନା fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ଅଚଳ ଅକ୍ଷର *post_urlenc,
		 ଅଚଳ ସଂରଚନା fetch_multipart_data *post_multipart,
		 ଅଚଳ ଅକ୍ଷର **headers)
{
	ସଂରଚନା fetch_data_context *ctx = calloc(1, ଆକାର(*ctx));
	
	ଯଦି (ctx == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;
		
	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);

	RING_ଭର୍ତ୍ତି(ring, ctx);
	
	ଫେରନ୍ତୁ ctx;
}

ସ୍ଥିର bool fetch_data_start(ଶୂନ୍ୟ *ctx)
{
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର ଶୂନ୍ୟ fetch_data_free(ଶୂନ୍ୟ *ctx)
{
	ସଂରଚନା fetch_data_context *c = ctx;

	nsurl_unref(c->url);
	ମୁକ୍ତ(c->data);
	ମୁକ୍ତ(c->mimetype);
	ମୁକ୍ତ(ctx);
}

ସ୍ଥିର ଶୂନ୍ୟ fetch_data_abort(ଶୂନ୍ୟ *ctx)
{
	ସଂରଚନା fetch_data_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ସ୍ଥିର bool fetch_data_process(ସଂରଚନା fetch_data_context *c)
{
	nserror res;
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	ଅଚଳ ଅକ୍ଷର *params;
	ଅଚଳ ଅକ୍ଷର *comma;
	ଅକ୍ଷର *unescaped;
	size_t unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	ଯଦି (nsurl_length(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		ସନ୍ଦେଶ.type = FETCH_ERROR;
		ସନ୍ଦେଶ.data.error = "Malformed data: URL";
		fetch_data_send_callback(&ସନ୍ଦେଶ, c);
		ଫେରନ୍ତୁ false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	ଯଦି ( (comma = strchr(params, ',')) == ଖାଲି) {
		ସନ୍ଦେଶ.type = FETCH_ERROR;
		ସନ୍ଦେଶ.data.error = "Malformed data: URL";
		fetch_data_send_callback(&ସନ୍ଦେଶ, c);
		ଫେରନ୍ତୁ false;
	}
	
	ଯଦି (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} ଅନ୍ୟଥା {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	ଯଦି (c->mimetype == ଖାଲି) {
		ସନ୍ଦେଶ.type = FETCH_ERROR;
		ସନ୍ଦେଶ.data.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_data_send_callback(&ସନ୍ଦେଶ, c);
		ଫେରନ୍ତୁ false;
	}
	
	ଯଦି (strcmp(c->mimetype + strlen(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[strlen(c->mimetype) - 7] = '\0';
	} ଅନ୍ୟଥା {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	ଯଦି (res != NSERROR_OK) {
		ସନ୍ଦେଶ.type = FETCH_ERROR;
		ସନ୍ଦେଶ.data.error = "Unable to URL decode data: URL";
		fetch_data_send_callback(&ସନ୍ଦେଶ, c);
		ଫେରନ୍ତୁ false;
	}
	
	ଯଦି (c->base64) {
		ଯଦି ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->data,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->data == ଖାଲି)) {
			ସନ୍ଦେଶ.type = FETCH_ERROR;
			ସନ୍ଦେଶ.data.error = "Unable to Base64 decode data: URL";
			fetch_data_send_callback(&ସନ୍ଦେଶ, c);
			ମୁକ୍ତ(unescaped);
			ଫେରନ୍ତୁ false;
		}
		ମୁକ୍ତ(unescaped);
	} ଅନ୍ୟଥା {
		c->datalen = unescaped_len;
		c->data = unescaped;
	}
	
	
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର ଶୂନ୍ୟ fetch_data_poll(lwc_string *scheme)
{
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	ସଂରଚନା fetch_data_context *c, *save_ring = ଖାଲି;
	
	/* Iterate over ring, processing each pending fetch */
	ଯେତେବେଳେ (ring != ଖାଲି) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called 
		 * again.
		 */
		ଯଦି (c->locked == true) {
			RING_ଭର୍ତ୍ତି(save_ring, c);
			ଜାରି;
		}

		/* Only process non-aborted fetches */
		ଯଦି (c->aborted == false && fetch_data_process(c) == true) {
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

			ଯଦି (c->aborted == false) {
				fetch_data_send_header(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			ଯଦି (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_data_send_header(c, "Cache-Control: "
						"max-age=31536000");
			}

			ଯଦି (c->aborted == false) {
				ସନ୍ଦେଶ.type = FETCH_DATA;
				ସନ୍ଦେଶ.data.header_or_data.buf = 
						(ଅଚଳ uint8_t *) c->data;
				ସନ୍ଦେଶ.data.header_or_data.len = c->datalen;
				fetch_data_send_callback(&ସନ୍ଦେଶ, c);
			}

			ଯଦି (c->aborted == false) {
				ସନ୍ଦେଶ.type = FETCH_FINISHED;
				fetch_data_send_callback(&ସନ୍ଦେଶ, c);
			}
		} ଅନ୍ୟଥା {
			NSLOG(netsurf, INFO, "Processing of %.140s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't, 
			 * then fetch_data_process() is broken.
			 */
			ପ୍ରତିଜ୍ଞା(c->locked == false);
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

nserror fetch_data_register(ଶୂନ୍ୟ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_data);
	ଅଚଳ ସଂରଚନା fetcher_operation_table fetcher_ops = {
		.initialise = fetch_data_initialise,
		.acceptable = fetch_data_can_fetch,
		.setup = fetch_data_setup,
		.start = fetch_data_start,
		.abort = fetch_data_abort,
		.ମୁକ୍ତ = fetch_data_free,
		.poll = fetch_data_poll,
		.finalise = fetch_data_finalise
	};

	ଫେରନ୍ତୁ fetcher_add(scheme, &fetcher_ops);
}
