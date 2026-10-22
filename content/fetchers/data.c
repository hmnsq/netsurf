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

ساخت fetch_ڈیٹا_context {
	ساخت fetch *والد_fetch;
	nsurl *url;
	حرف *mimetype;
	حرف *ڈیٹا;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	ساخت fetch_ڈیٹا_context *r_اگلا, *r_پچھلا;
};

ساکن ساخت fetch_ڈیٹا_context *ring = تہی;

ساکن bool fetch_ڈیٹا_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_ڈیٹا(scheme));

	واپس true;
}

ساکن خالی fetch_ڈیٹا_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_ڈیٹا(scheme));
}

ساکن bool fetch_ڈیٹا_can_fetch(ثابت nsurl *url)
{
	واپس true;
}

ساکن خالی fetch_ڈیٹا_send_callback(ثابت fetch_پیغام *پیغام,
		ساخت fetch_ڈیٹا_context *c)
{
	c->locked = true;
	fetch_send_callback(پیغام, c->والد_fetch);
	c->locked = false;
}

ساکن خالی fetch_ڈیٹا_send_سرخی(ساخت fetch_ڈیٹا_context *ctx,
		ثابت حرف *fmt, ...)
{
	حرف سرخی[64];
	fetch_پیغام پیغام;
	va_list ap;
	صحیح len;

	va_start(ap, fmt);
	len = vsnprintf(سرخی, حجم(سرخی), fmt, ap);
	va_end(ap);

	اگر (len >= (صحیح)حجم(سرخی) || len < 0) {
		واپس;
	}

	پیغام.type = FETCH_سرخی;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = len;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *)سرخی;
	fetch_ڈیٹا_send_callback(&پیغام, ctx);
}

ساکن خالی *fetch_ڈیٹا_تیاری(ساخت fetch *والد_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ثابت حرف *post_urlenc,
		 ثابت ساخت fetch_multipart_ڈیٹا *post_multipart,
		 ثابت حرف **سرخیاں)
{
	ساخت fetch_ڈیٹا_context *ctx = صفر_حافظہ_دو(1, حجم(*ctx));
	
	اگر (ctx == تہی)
		واپس تہی;
		
	ctx->والد_fetch = والد_fetch;
	ctx->url = nsurl_ref(url);

	RING_داخل_کرو(ring, ctx);
	
	واپس ctx;
}

ساکن bool fetch_ڈیٹا_آغاز(خالی *ctx)
{
	واپس true;
}

ساکن خالی fetch_ڈیٹا_free(خالی *ctx)
{
	ساخت fetch_ڈیٹا_context *c = ctx;

	nsurl_unref(c->url);
	آزاد(c->ڈیٹا);
	آزاد(c->mimetype);
	آزاد(ctx);
}

ساکن خالی fetch_ڈیٹا_abort(خالی *ctx)
{
	ساخت fetch_ڈیٹا_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ساکن bool fetch_ڈیٹا_process(ساخت fetch_ڈیٹا_context *c)
{
	nserror res;
	fetch_پیغام پیغام;
	ثابت حرف *params;
	ثابت حرف *comma;
	حرف *unescaped;
	size_t unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	اگر (nsurl_لمبائی(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		پیغام.type = FETCH_ERROR;
		پیغام.ڈیٹا.error = "Malformed data: URL";
		fetch_ڈیٹا_send_callback(&پیغام, c);
		واپس false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	اگر ( (comma = strchr(params, ',')) == تہی) {
		پیغام.type = FETCH_ERROR;
		پیغام.ڈیٹا.error = "Malformed data: URL";
		fetch_ڈیٹا_send_callback(&پیغام, c);
		واپس false;
	}
	
	اگر (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} ورنہ {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	اگر (c->mimetype == تہی) {
		پیغام.type = FETCH_ERROR;
		پیغام.ڈیٹا.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_ڈیٹا_send_callback(&پیغام, c);
		واپس false;
	}
	
	اگر (strcmp(c->mimetype + لڑی_لمبائی(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[لڑی_لمبائی(c->mimetype) - 7] = '\0';
	} ورنہ {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	اگر (res != NSERROR_OK) {
		پیغام.type = FETCH_ERROR;
		پیغام.ڈیٹا.error = "Unable to URL decode data: URL";
		fetch_ڈیٹا_send_callback(&پیغام, c);
		واپس false;
	}
	
	اگر (c->base64) {
		اگر ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->ڈیٹا,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->ڈیٹا == تہی)) {
			پیغام.type = FETCH_ERROR;
			پیغام.ڈیٹا.error = "Unable to Base64 decode data: URL";
			fetch_ڈیٹا_send_callback(&پیغام, c);
			آزاد(unescaped);
			واپس false;
		}
		آزاد(unescaped);
	} ورنہ {
		c->datalen = unescaped_len;
		c->ڈیٹا = unescaped;
	}
	
	
	واپس true;
}

ساکن خالی fetch_ڈیٹا_poll(lwc_string *scheme)
{
	fetch_پیغام پیغام;
	ساخت fetch_ڈیٹا_context *c, *save_ring = تہی;
	
	/* Iterate over ring, processing each pending fetch */
	جب (ring != تہی) {
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
			RING_داخل_کرو(save_ring, c);
			جاری;
		}

		/* Only process non-aborted fetches */
		اگر (c->aborted == false && fetch_ڈیٹا_process(c) == true) {
			fetch_سیٹ_http_رمز(c->والد_fetch, 200);
			NSLOG(netsurf, INFO,
			      "setting data: MIME type to %s, length to %"PRIsizet,
			      c->mimetype,
			      c->datalen);
			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to fetch_data_send_callback().
			 */
			fetch_ڈیٹا_send_سرخی(c, "Content-Type: %s",
					c->mimetype);

			اگر (c->aborted == false) {
				fetch_ڈیٹا_send_سرخی(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			اگر (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_ڈیٹا_send_سرخی(c, "Cache-Control: "
						"max-age=31536000");
			}

			اگر (c->aborted == false) {
				پیغام.type = FETCH_ڈیٹا;
				پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = 
						(ثابت uint8_t *) c->ڈیٹا;
				پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = c->datalen;
				fetch_ڈیٹا_send_callback(&پیغام, c);
			}

			اگر (c->aborted == false) {
				پیغام.type = FETCH_ختم_شدہ;
				fetch_ڈیٹا_send_callback(&پیغام, c);
			}
		} ورنہ {
			NSLOG(netsurf, INFO, "Processing of %.140s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't, 
			 * then fetch_data_process() is broken.
			 */
			أكد(c->locked == false);
		}

		/* And now finish */
		fetch_remove_from_queues(c->والد_fetch);
		fetch_free(c->والد_fetch);
	}

	/* Finally, if we saved any fetches which were locked, put them back
	 * into the ring for next time
	 */
	ring = save_ring;
}

nserror fetch_ڈیٹا_رجسٹر(خالی)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_ڈیٹا);
	ثابت ساخت fetcher_operation_جدول fetcher_ops = {
		.initialise = fetch_ڈیٹا_initialise,
		.acceptable = fetch_ڈیٹا_can_fetch,
		.تیاری = fetch_ڈیٹا_تیاری,
		.آغاز = fetch_ڈیٹا_آغاز,
		.abort = fetch_ڈیٹا_abort,
		.آزاد = fetch_ڈیٹا_free,
		.poll = fetch_ڈیٹا_poll,
		.finalise = fetch_ڈیٹا_finalise
	};

	واپس fetcher_جوڑو(scheme, &fetcher_ops);
}
