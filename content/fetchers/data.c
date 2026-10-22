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

#شامل <stdbool.h>
#شامل <string.h>
#شامل <stdarg.h>
#شامل <stdlib.h>
#شامل <libwapcaplet/libwapcaplet.h>
#شامل <nsutils/base64.h>

#شامل "netsurf/inttypes.h"
#شامل "utils/url.h"
#شامل "utils/nsurl.h"
#شامل "utils/corestrings.h"
#شامل "utils/log.h"
#شامل "utils/utils.h"
#شامل "utils/ring.h"

#شامل "content/fetch.h"
#شامل "content/fetchers.h"
#شامل "content/fetchers/data.h"

ساختار fetch_data_context {
	ساختار fetch *parent_fetch;
	nsurl *url;
	نویسه *mimetype;
	نویسه *data;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	ساختار fetch_data_context *r_next, *r_prev;
};

ایستا ساختار fetch_data_context *ring = پوچ;

ایستا bool fetch_data_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_data(scheme));

	بازگشت true;
}

ایستا تهی fetch_data_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_data(scheme));
}

ایستا bool fetch_data_can_fetch(ثابت nsurl *url)
{
	بازگشت true;
}

ایستا تهی fetch_data_send_callback(ثابت fetch_پیام *پیام,
		ساختار fetch_data_context *c)
{
	c->locked = true;
	fetch_send_callback(پیام, c->parent_fetch);
	c->locked = false;
}

ایستا تهی fetch_data_send_header(ساختار fetch_data_context *ctx,
		ثابت نویسه *fmt, ...)
{
	نویسه header[64];
	fetch_پیام پیام;
	va_list ap;
	صحیح len;

	va_start(ap, fmt);
	len = vsnprintf(header, اندازه(header), fmt, ap);
	va_end(ap);

	اگر (len >= (صحیح)اندازه(header) || len < 0) {
		بازگشت;
	}

	پیام.type = FETCH_HEADER;
	پیام.data.header_or_data.len = len;
	پیام.data.header_or_data.buf = (ثابت uint8_t *)header;
	fetch_data_send_callback(&پیام, ctx);
}

ایستا تهی *fetch_data_setup(ساختار fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ثابت نویسه *post_urlenc,
		 ثابت ساختار fetch_multipart_data *post_multipart,
		 ثابت نویسه **headers)
{
	ساختار fetch_data_context *ctx = calloc(1, اندازه(*ctx));
	
	اگر (ctx == پوچ)
		بازگشت پوچ;
		
	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);

	RING_درج(ring, ctx);
	
	بازگشت ctx;
}

ایستا bool fetch_data_start(تهی *ctx)
{
	بازگشت true;
}

ایستا تهی fetch_data_free(تهی *ctx)
{
	ساختار fetch_data_context *c = ctx;

	nsurl_unref(c->url);
	آزاد(c->data);
	آزاد(c->mimetype);
	آزاد(ctx);
}

ایستا تهی fetch_data_abort(تهی *ctx)
{
	ساختار fetch_data_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ایستا bool fetch_data_process(ساختار fetch_data_context *c)
{
	nserror res;
	fetch_پیام پیام;
	ثابت نویسه *params;
	ثابت نویسه *comma;
	نویسه *unescaped;
	size_t unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	اگر (nsurl_length(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		پیام.type = FETCH_ERROR;
		پیام.data.error = "Malformed data: URL";
		fetch_data_send_callback(&پیام, c);
		بازگشت false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	اگر ( (comma = strchr(params, ',')) == پوچ) {
		پیام.type = FETCH_ERROR;
		پیام.data.error = "Malformed data: URL";
		fetch_data_send_callback(&پیام, c);
		بازگشت false;
	}
	
	اگر (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} وگرنه {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	اگر (c->mimetype == پوچ) {
		پیام.type = FETCH_ERROR;
		پیام.data.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_data_send_callback(&پیام, c);
		بازگشت false;
	}
	
	اگر (strcmp(c->mimetype + strlen(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[strlen(c->mimetype) - 7] = '\0';
	} وگرنه {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	اگر (res != NSERROR_OK) {
		پیام.type = FETCH_ERROR;
		پیام.data.error = "Unable to URL decode data: URL";
		fetch_data_send_callback(&پیام, c);
		بازگشت false;
	}
	
	اگر (c->base64) {
		اگر ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->data,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->data == پوچ)) {
			پیام.type = FETCH_ERROR;
			پیام.data.error = "Unable to Base64 decode data: URL";
			fetch_data_send_callback(&پیام, c);
			آزاد(unescaped);
			بازگشت false;
		}
		آزاد(unescaped);
	} وگرنه {
		c->datalen = unescaped_len;
		c->data = unescaped;
	}
	
	
	بازگشت true;
}

ایستا تهی fetch_data_poll(lwc_string *scheme)
{
	fetch_پیام پیام;
	ساختار fetch_data_context *c, *save_ring = پوچ;
	
	/* Iterate over ring, processing each pending fetch */
	وقتی (ring != پوچ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called 
		 * again.
		 */
		اگر (c->locked == true) {
			RING_درج(save_ring, c);
			ادامه;
		}

		/* Only process non-aborted fetches */
		اگر (c->aborted == false && fetch_data_process(c) == true) {
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

			اگر (c->aborted == false) {
				fetch_data_send_header(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			اگر (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_data_send_header(c, "Cache-Control: "
						"max-age=31536000");
			}

			اگر (c->aborted == false) {
				پیام.type = FETCH_DATA;
				پیام.data.header_or_data.buf = 
						(ثابت uint8_t *) c->data;
				پیام.data.header_or_data.len = c->datalen;
				fetch_data_send_callback(&پیام, c);
			}

			اگر (c->aborted == false) {
				پیام.type = FETCH_FINISHED;
				fetch_data_send_callback(&پیام, c);
			}
		} وگرنه {
			NSLOG(netsurf, INFO, "Processing of %.140s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't, 
			 * then fetch_data_process() is broken.
			 */
			أكد(c->locked == false);
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

nserror fetch_data_register(تهی)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_data);
	ثابت ساختار fetcher_operation_table fetcher_ops = {
		.initialise = fetch_data_initialise,
		.acceptable = fetch_data_can_fetch,
		.setup = fetch_data_setup,
		.start = fetch_data_start,
		.abort = fetch_data_abort,
		.آزاد = fetch_data_free,
		.poll = fetch_data_poll,
		.finalise = fetch_data_finalise
	};

	بازگشت fetcher_add(scheme, &fetcher_ops);
}
