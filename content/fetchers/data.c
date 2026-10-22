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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>
#ಅನ್ತರ್ಭಾವ <nsutils/base64.h>

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "utils/url.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/ring.h"

#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers/data.h"

ಸಂರಚನಾ fetch_data_context {
	ಸಂರಚನಾ fetch *parent_fetch;
	nsurl *url;
	ವರ್ಣ *mimetype;
	ವರ್ಣ *data;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	ಸಂರಚನಾ fetch_data_context *r_next, *r_prev;
};

ಸ್ಥಿರ ಸಂರಚನಾ fetch_data_context *ring = ರಿಕ್ತ;

ಸ್ಥಿರ bool fetch_data_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_data(scheme));

	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_data_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_data(scheme));
}

ಸ್ಥಿರ bool fetch_data_can_fetch(ಧ್ರುವ nsurl *url)
{
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_data_send_callback(ಧ್ರುವ fetch_ಸಂದೇಶ *ಸಂದೇಶ,
		ಸಂರಚನಾ fetch_data_context *c)
{
	c->locked = true;
	fetch_send_callback(ಸಂದೇಶ, c->parent_fetch);
	c->locked = false;
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_data_send_header(ಸಂರಚನಾ fetch_data_context *ctx,
		ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	ವರ್ಣ header[64];
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ len;

	va_start(ap, fmt);
	len = vsnprintf(header, ಪರಿಮಾಣ(header), fmt, ap);
	va_end(ap);

	ಯದಿ (len >= (ಪೂರ್ಣಾಙ್ಕ)ಪರಿಮಾಣ(header) || len < 0) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಸಂದೇಶ.type = FETCH_HEADER;
	ಸಂದೇಶ.data.header_or_data.len = len;
	ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *)header;
	fetch_data_send_callback(&ಸಂದೇಶ, ctx);
}

ಸ್ಥಿರ ಶೂನ್ಯ *fetch_data_setup(ಸಂರಚನಾ fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ಧ್ರುವ ವರ್ಣ *post_urlenc,
		 ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *post_multipart,
		 ಧ್ರುವ ವರ್ಣ **headers)
{
	ಸಂರಚನಾ fetch_data_context *ctx = calloc(1, ಪರಿಮಾಣ(*ctx));
	
	ಯದಿ (ctx == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		
	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);

	RING_ಸೇರಿಸು(ring, ctx);
	
	ಪ್ರತ್ಯಾಗಮ ctx;
}

ಸ್ಥಿರ bool fetch_data_start(ಶೂನ್ಯ *ctx)
{
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_data_free(ಶೂನ್ಯ *ctx)
{
	ಸಂರಚನಾ fetch_data_context *c = ctx;

	nsurl_unref(c->url);
	ಮುಕ್ತ(c->data);
	ಮುಕ್ತ(c->mimetype);
	ಮುಕ್ತ(ctx);
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_data_abort(ಶೂನ್ಯ *ctx)
{
	ಸಂರಚನಾ fetch_data_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ಸ್ಥಿರ bool fetch_data_process(ಸಂರಚನಾ fetch_data_context *c)
{
	nserror res;
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಧ್ರುವ ವರ್ಣ *params;
	ಧ್ರುವ ವರ್ಣ *comma;
	ವರ್ಣ *unescaped;
	size_t unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	ಯದಿ (nsurl_length(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.data.error = "Malformed data: URL";
		fetch_data_send_callback(&ಸಂದೇಶ, c);
		ಪ್ರತ್ಯಾಗಮ false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	ಯದಿ ( (comma = strchr(params, ',')) == ರಿಕ್ತ) {
		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.data.error = "Malformed data: URL";
		fetch_data_send_callback(&ಸಂದೇಶ, c);
		ಪ್ರತ್ಯಾಗಮ false;
	}
	
	ಯದಿ (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} ಅನ್ಯಥಾ {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	ಯದಿ (c->mimetype == ರಿಕ್ತ) {
		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.data.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_data_send_callback(&ಸಂದೇಶ, c);
		ಪ್ರತ್ಯಾಗಮ false;
	}
	
	ಯದಿ (strcmp(c->mimetype + strlen(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[strlen(c->mimetype) - 7] = '\0';
	} ಅನ್ಯಥಾ {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	ಯದಿ (res != NSERROR_OK) {
		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.data.error = "Unable to URL decode data: URL";
		fetch_data_send_callback(&ಸಂದೇಶ, c);
		ಪ್ರತ್ಯಾಗಮ false;
	}
	
	ಯದಿ (c->base64) {
		ಯದಿ ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->data,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->data == ರಿಕ್ತ)) {
			ಸಂದೇಶ.type = FETCH_ERROR;
			ಸಂದೇಶ.data.error = "Unable to Base64 decode data: URL";
			fetch_data_send_callback(&ಸಂದೇಶ, c);
			ಮುಕ್ತ(unescaped);
			ಪ್ರತ್ಯಾಗಮ false;
		}
		ಮುಕ್ತ(unescaped);
	} ಅನ್ಯಥಾ {
		c->datalen = unescaped_len;
		c->data = unescaped;
	}
	
	
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_data_poll(lwc_string *scheme)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಸಂರಚನಾ fetch_data_context *c, *save_ring = ರಿಕ್ತ;
	
	/* Iterate over ring, processing each pending fetch */
	ಯಾವತ್ (ring != ರಿಕ್ತ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called 
		 * again.
		 */
		ಯದಿ (c->locked == true) {
			RING_ಸೇರಿಸು(save_ring, c);
			ಅನುವರ್ತ;
		}

		/* Only process non-aborted fetches */
		ಯದಿ (c->aborted == false && fetch_data_process(c) == true) {
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

			ಯದಿ (c->aborted == false) {
				fetch_data_send_header(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			ಯದಿ (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_data_send_header(c, "Cache-Control: "
						"max-age=31536000");
			}

			ಯದಿ (c->aborted == false) {
				ಸಂದೇಶ.type = FETCH_DATA;
				ಸಂದೇಶ.data.header_or_data.buf = 
						(ಧ್ರುವ uint8_t *) c->data;
				ಸಂದೇಶ.data.header_or_data.len = c->datalen;
				fetch_data_send_callback(&ಸಂದೇಶ, c);
			}

			ಯದಿ (c->aborted == false) {
				ಸಂದೇಶ.type = FETCH_FINISHED;
				fetch_data_send_callback(&ಸಂದೇಶ, c);
			}
		} ಅನ್ಯಥಾ {
			NSLOG(netsurf, INFO, "Processing of %.140s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't, 
			 * then fetch_data_process() is broken.
			 */
			ಪ್ರತಿಜ್ಞಾ(c->locked == false);
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

nserror fetch_data_register(ಶೂನ್ಯ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_data);
	ಧ್ರುವ ಸಂರಚನಾ fetcher_operation_table fetcher_ops = {
		.initialise = fetch_data_initialise,
		.acceptable = fetch_data_can_fetch,
		.setup = fetch_data_setup,
		.start = fetch_data_start,
		.abort = fetch_data_abort,
		.ಮುಕ್ತ = fetch_data_free,
		.poll = fetch_data_poll,
		.finalise = fetch_data_finalise
	};

	ಪ್ರತ್ಯಾಗಮ fetcher_add(scheme, &fetcher_ops);
}
