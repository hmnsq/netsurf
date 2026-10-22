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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <libwapcaplet/libwapcaplet.h>
#அந்தர்பாவ <nsutils/base64.h>

#அந்தர்பாவ "netsurf/inttypes.h"
#அந்தர்பாவ "utils/url.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/ring.h"

#அந்தர்பாவ "content/fetch.h"
#அந்தர்பாவ "content/fetchers.h"
#அந்தர்பாவ "content/fetchers/data.h"

கட்டமைப்பு fetch_data_context {
	கட்டமைப்பு fetch *parent_fetch;
	nsurl *url;
	எழுத்து *mimetype;
	எழுத்து *data;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	கட்டமைப்பு fetch_data_context *r_next, *r_prev;
};

நிலையான கட்டமைப்பு fetch_data_context *ring = வெற்று;

நிலையான bool fetch_data_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_data(scheme));

	திரும்பு true;
}

நிலையான வெறுமை fetch_data_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_data(scheme));
}

நிலையான bool fetch_data_can_fetch(மாறிலி nsurl *url)
{
	திரும்பு true;
}

நிலையான வெறுமை fetch_data_send_callback(மாறிலி fetch_செய்தி *செய்தி,
		கட்டமைப்பு fetch_data_context *c)
{
	c->locked = true;
	fetch_send_callback(செய்தி, c->parent_fetch);
	c->locked = false;
}

நிலையான வெறுமை fetch_data_send_header(கட்டமைப்பு fetch_data_context *ctx,
		மாறிலி எழுத்து *fmt, ...)
{
	எழுத்து header[64];
	fetch_செய்தி செய்தி;
	va_list ap;
	முழு len;

	va_start(ap, fmt);
	len = vsnprintf(header, அளவு(header), fmt, ap);
	va_end(ap);

	எனில் (len >= (முழு)அளவு(header) || len < 0) {
		திரும்பு;
	}

	செய்தி.type = FETCH_HEADER;
	செய்தி.data.header_or_data.len = len;
	செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *)header;
	fetch_data_send_callback(&செய்தி, ctx);
}

நிலையான வெறுமை *fetch_data_setup(கட்டமைப்பு fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, மாறிலி எழுத்து *post_urlenc,
		 மாறிலி கட்டமைப்பு fetch_multipart_data *post_multipart,
		 மாறிலி எழுத்து **headers)
{
	கட்டமைப்பு fetch_data_context *ctx = calloc(1, அளவு(*ctx));
	
	எனில் (ctx == வெற்று)
		திரும்பு வெற்று;
		
	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);

	RING_செருகு(ring, ctx);
	
	திரும்பு ctx;
}

நிலையான bool fetch_data_start(வெறுமை *ctx)
{
	திரும்பு true;
}

நிலையான வெறுமை fetch_data_free(வெறுமை *ctx)
{
	கட்டமைப்பு fetch_data_context *c = ctx;

	nsurl_unref(c->url);
	விடுவி(c->data);
	விடுவி(c->mimetype);
	விடுவி(ctx);
}

நிலையான வெறுமை fetch_data_abort(வெறுமை *ctx)
{
	கட்டமைப்பு fetch_data_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

நிலையான bool fetch_data_process(கட்டமைப்பு fetch_data_context *c)
{
	nserror res;
	fetch_செய்தி செய்தி;
	மாறிலி எழுத்து *params;
	மாறிலி எழுத்து *comma;
	எழுத்து *unescaped;
	size_t unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	எனில் (nsurl_length(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		செய்தி.type = FETCH_ERROR;
		செய்தி.data.error = "Malformed data: URL";
		fetch_data_send_callback(&செய்தி, c);
		திரும்பு false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	எனில் ( (comma = strchr(params, ',')) == வெற்று) {
		செய்தி.type = FETCH_ERROR;
		செய்தி.data.error = "Malformed data: URL";
		fetch_data_send_callback(&செய்தி, c);
		திரும்பு false;
	}
	
	எனில் (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} இல்லையெனில் {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	எனில் (c->mimetype == வெற்று) {
		செய்தி.type = FETCH_ERROR;
		செய்தி.data.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_data_send_callback(&செய்தி, c);
		திரும்பு false;
	}
	
	எனில் (strcmp(c->mimetype + strlen(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[strlen(c->mimetype) - 7] = '\0';
	} இல்லையெனில் {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	எனில் (res != NSERROR_OK) {
		செய்தி.type = FETCH_ERROR;
		செய்தி.data.error = "Unable to URL decode data: URL";
		fetch_data_send_callback(&செய்தி, c);
		திரும்பு false;
	}
	
	எனில் (c->base64) {
		எனில் ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->data,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->data == வெற்று)) {
			செய்தி.type = FETCH_ERROR;
			செய்தி.data.error = "Unable to Base64 decode data: URL";
			fetch_data_send_callback(&செய்தி, c);
			விடுவி(unescaped);
			திரும்பு false;
		}
		விடுவி(unescaped);
	} இல்லையெனில் {
		c->datalen = unescaped_len;
		c->data = unescaped;
	}
	
	
	திரும்பு true;
}

நிலையான வெறுமை fetch_data_poll(lwc_string *scheme)
{
	fetch_செய்தி செய்தி;
	கட்டமைப்பு fetch_data_context *c, *save_ring = வெற்று;
	
	/* Iterate over ring, processing each pending fetch */
	போது (ring != வெற்று) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called 
		 * again.
		 */
		எனில் (c->locked == true) {
			RING_செருகு(save_ring, c);
			தொடர்;
		}

		/* Only process non-aborted fetches */
		எனில் (c->aborted == false && fetch_data_process(c) == true) {
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

			எனில் (c->aborted == false) {
				fetch_data_send_header(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			எனில் (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_data_send_header(c, "Cache-Control: "
						"max-age=31536000");
			}

			எனில் (c->aborted == false) {
				செய்தி.type = FETCH_DATA;
				செய்தி.data.header_or_data.buf = 
						(மாறிலி uint8_t *) c->data;
				செய்தி.data.header_or_data.len = c->datalen;
				fetch_data_send_callback(&செய்தி, c);
			}

			எனில் (c->aborted == false) {
				செய்தி.type = FETCH_FINISHED;
				fetch_data_send_callback(&செய்தி, c);
			}
		} இல்லையெனில் {
			NSLOG(netsurf, INFO, "Processing of %.140s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't, 
			 * then fetch_data_process() is broken.
			 */
			ப்ரதிஜ்ஞா(c->locked == false);
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

nserror fetch_data_register(வெறுமை)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_data);
	மாறிலி கட்டமைப்பு fetcher_operation_table fetcher_ops = {
		.initialise = fetch_data_initialise,
		.acceptable = fetch_data_can_fetch,
		.setup = fetch_data_setup,
		.start = fetch_data_start,
		.abort = fetch_data_abort,
		.விடுவி = fetch_data_free,
		.poll = fetch_data_poll,
		.finalise = fetch_data_finalise
	};

	திரும்பு fetcher_add(scheme, &fetcher_ops);
}
