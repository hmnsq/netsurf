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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libwapcaplet/libwapcaplet.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nsutils/base64.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/url.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ring.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers/data.h"

ਬਣਤਰ fetch_ਅੰਕੜੇ_context {
	ਬਣਤਰ fetch *ਮਾਪਾ_fetch;
	nsurl *url;
	ਅੱਖਰ *mimetype;
	ਅੱਖਰ *ਅੰਕੜੇ;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	ਬਣਤਰ fetch_ਅੰਕੜੇ_context *r_ਅਗਲਾ, *r_prev;
};

ਸਥਿਰ ਬਣਤਰ fetch_ਅੰਕੜੇ_context *ring = ਨਿਰਾ;

ਸਥਿਰ bool fetch_ਅੰਕੜੇ_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_ਅੰਕੜੇ(scheme));

	ਵਾਪਸ true;
}

ਸਥਿਰ ਖਾਲੀ fetch_ਅੰਕੜੇ_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_ਅੰਕੜੇ(scheme));
}

ਸਥਿਰ bool fetch_ਅੰਕੜੇ_can_fetch(ਅਚਲ nsurl *url)
{
	ਵਾਪਸ true;
}

ਸਥਿਰ ਖਾਲੀ fetch_ਅੰਕੜੇ_send_callback(ਅਚਲ fetch_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ,
		ਬਣਤਰ fetch_ਅੰਕੜੇ_context *c)
{
	c->locked = true;
	fetch_send_callback(ਸੁਨੇਹਾ, c->ਮਾਪਾ_fetch);
	c->locked = false;
}

ਸਥਿਰ ਖਾਲੀ fetch_ਅੰਕੜੇ_send_ਸਿਰਲੇਖ(ਬਣਤਰ fetch_ਅੰਕੜੇ_context *ctx,
		ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	ਅੱਖਰ ਸਿਰਲੇਖ[64];
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	va_list ap;
	ਪੂਰਨ len;

	va_start(ap, fmt);
	len = vsnprintf(ਸਿਰਲੇਖ, ਆਕਾਰ(ਸਿਰਲੇਖ), fmt, ap);
	va_end(ap);

	ਜੇ (len >= (ਪੂਰਨ)ਆਕਾਰ(ਸਿਰਲੇਖ) || len < 0) {
		ਵਾਪਸ;
	}

	ਸੁਨੇਹਾ.type = FETCH_ਸਿਰਲੇਖ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = len;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *)ਸਿਰਲੇਖ;
	fetch_ਅੰਕੜੇ_send_callback(&ਸੁਨੇਹਾ, ctx);
}

ਸਥਿਰ ਖਾਲੀ *fetch_ਅੰਕੜੇ_ਤਿਆਰੀ(ਬਣਤਰ fetch *ਮਾਪਾ_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ਅਚਲ ਅੱਖਰ *post_urlenc,
		 ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *post_multipart,
		 ਅਚਲ ਅੱਖਰ **ਸਿਰਲੇਖਾਂ)
{
	ਬਣਤਰ fetch_ਅੰਕੜੇ_context *ctx = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*ctx));
	
	ਜੇ (ctx == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;
		
	ctx->ਮਾਪਾ_fetch = ਮਾਪਾ_fetch;
	ctx->url = nsurl_ref(url);

	RING_ਪਾਓ(ring, ctx);
	
	ਵਾਪਸ ctx;
}

ਸਥਿਰ bool fetch_ਅੰਕੜੇ_ਸ਼ੁਰੂ(ਖਾਲੀ *ctx)
{
	ਵਾਪਸ true;
}

ਸਥਿਰ ਖਾਲੀ fetch_ਅੰਕੜੇ_free(ਖਾਲੀ *ctx)
{
	ਬਣਤਰ fetch_ਅੰਕੜੇ_context *c = ctx;

	nsurl_unref(c->url);
	ਮੁਕਤ(c->ਅੰਕੜੇ);
	ਮੁਕਤ(c->mimetype);
	ਮੁਕਤ(ctx);
}

ਸਥਿਰ ਖਾਲੀ fetch_ਅੰਕੜੇ_abort(ਖਾਲੀ *ctx)
{
	ਬਣਤਰ fetch_ਅੰਕੜੇ_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ਸਥਿਰ bool fetch_ਅੰਕੜੇ_process(ਬਣਤਰ fetch_ਅੰਕੜੇ_context *c)
{
	nserror res;
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	ਅਚਲ ਅੱਖਰ *params;
	ਅਚਲ ਅੱਖਰ *comma;
	ਅੱਖਰ *unescaped;
	size_t unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	ਜੇ (nsurl_ਲੰਮਾਈ(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		ਸੁਨੇਹਾ.type = FETCH_ERROR;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.error = "Malformed data: URL";
		fetch_ਅੰਕੜੇ_send_callback(&ਸੁਨੇਹਾ, c);
		ਵਾਪਸ false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	ਜੇ ( (comma = strchr(params, ',')) == ਨਿਰਾ) {
		ਸੁਨੇਹਾ.type = FETCH_ERROR;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.error = "Malformed data: URL";
		fetch_ਅੰਕੜੇ_send_callback(&ਸੁਨੇਹਾ, c);
		ਵਾਪਸ false;
	}
	
	ਜੇ (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} ਨਹੀਂਤਾਂ {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	ਜੇ (c->mimetype == ਨਿਰਾ) {
		ਸੁਨੇਹਾ.type = FETCH_ERROR;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_ਅੰਕੜੇ_send_callback(&ਸੁਨੇਹਾ, c);
		ਵਾਪਸ false;
	}
	
	ਜੇ (strcmp(c->mimetype + ਲੜੀ_ਲੰਬਾਈ(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[ਲੜੀ_ਲੰਬਾਈ(c->mimetype) - 7] = '\0';
	} ਨਹੀਂਤਾਂ {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	ਜੇ (res != NSERROR_OK) {
		ਸੁਨੇਹਾ.type = FETCH_ERROR;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.error = "Unable to URL decode data: URL";
		fetch_ਅੰਕੜੇ_send_callback(&ਸੁਨੇਹਾ, c);
		ਵਾਪਸ false;
	}
	
	ਜੇ (c->base64) {
		ਜੇ ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->ਅੰਕੜੇ,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->ਅੰਕੜੇ == ਨਿਰਾ)) {
			ਸੁਨੇਹਾ.type = FETCH_ERROR;
			ਸੁਨੇਹਾ.ਅੰਕੜੇ.error = "Unable to Base64 decode data: URL";
			fetch_ਅੰਕੜੇ_send_callback(&ਸੁਨੇਹਾ, c);
			ਮੁਕਤ(unescaped);
			ਵਾਪਸ false;
		}
		ਮੁਕਤ(unescaped);
	} ਨਹੀਂਤਾਂ {
		c->datalen = unescaped_len;
		c->ਅੰਕੜੇ = unescaped;
	}
	
	
	ਵਾਪਸ true;
}

ਸਥਿਰ ਖਾਲੀ fetch_ਅੰਕੜੇ_poll(lwc_string *scheme)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	ਬਣਤਰ fetch_ਅੰਕੜੇ_context *c, *save_ring = ਨਿਰਾ;
	
	/* Iterate over ring, processing each pending fetch */
	ਜਦੋਂ (ring != ਨਿਰਾ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called 
		 * again.
		 */
		ਜੇ (c->locked == true) {
			RING_ਪਾਓ(save_ring, c);
			ਜਾਰੀ;
		}

		/* Only process non-aborted fetches */
		ਜੇ (c->aborted == false && fetch_ਅੰਕੜੇ_process(c) == true) {
			fetch_ਸੈੱਟ_http_ਕੋਡ(c->ਮਾਪਾ_fetch, 200);
			NSLOG(netsurf, INFO,
			      "setting data: MIME type to %s, length to %"PRIsizet,
			      c->mimetype,
			      c->datalen);
			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to fetch_data_send_callback().
			 */
			fetch_ਅੰਕੜੇ_send_ਸਿਰਲੇਖ(c, "Content-Type: %s",
					c->mimetype);

			ਜੇ (c->aborted == false) {
				fetch_ਅੰਕੜੇ_send_ਸਿਰਲੇਖ(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			ਜੇ (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_ਅੰਕੜੇ_send_ਸਿਰਲੇਖ(c, "Cache-Control: "
						"max-age=31536000");
			}

			ਜੇ (c->aborted == false) {
				ਸੁਨੇਹਾ.type = FETCH_ਅੰਕੜੇ;
				ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = 
						(ਅਚਲ uint8_t *) c->ਅੰਕੜੇ;
				ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = c->datalen;
				fetch_ਅੰਕੜੇ_send_callback(&ਸੁਨੇਹਾ, c);
			}

			ਜੇ (c->aborted == false) {
				ਸੁਨੇਹਾ.type = FETCH_ਮੁਕੰਮਲ;
				fetch_ਅੰਕੜੇ_send_callback(&ਸੁਨੇਹਾ, c);
			}
		} ਨਹੀਂਤਾਂ {
			NSLOG(netsurf, INFO, "Processing of %.140s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't, 
			 * then fetch_data_process() is broken.
			 */
			ਪ੍ਰਤਿਜ੍ਞਾ(c->locked == false);
		}

		/* And now finish */
		fetch_remove_from_queues(c->ਮਾਪਾ_fetch);
		fetch_free(c->ਮਾਪਾ_fetch);
	}

	/* Finally, if we saved any fetches which were locked, put them back
	 * into the ring for next time
	 */
	ring = save_ring;
}

nserror fetch_ਅੰਕੜੇ_ਰਜਿਸਟਰ(ਖਾਲੀ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_ਅੰਕੜੇ);
	ਅਚਲ ਬਣਤਰ fetcher_operation_ਸਾਰਣੀ fetcher_ops = {
		.initialise = fetch_ਅੰਕੜੇ_initialise,
		.acceptable = fetch_ਅੰਕੜੇ_can_fetch,
		.ਤਿਆਰੀ = fetch_ਅੰਕੜੇ_ਤਿਆਰੀ,
		.ਸ਼ੁਰੂ = fetch_ਅੰਕੜੇ_ਸ਼ੁਰੂ,
		.abort = fetch_ਅੰਕੜੇ_abort,
		.ਮੁਕਤ = fetch_ਅੰਕੜੇ_free,
		.poll = fetch_ਅੰਕੜੇ_poll,
		.finalise = fetch_ਅੰਕੜੇ_finalise
	};

	ਵਾਪਸ fetcher_ਜੋੜੋ(scheme, &fetcher_ops);
}
