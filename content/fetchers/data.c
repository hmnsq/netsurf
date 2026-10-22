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

ರಚನೆ fetch_ದತ್ತಾಂಶ_context {
	ರಚನೆ fetch *ಪೋಷಕ_fetch;
	nsurl *url;
	ಅಕ್ಷರ *mimetype;
	ಅಕ್ಷರ *ದತ್ತಾಂಶ;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	ರಚನೆ fetch_ದತ್ತಾಂಶ_context *r_ಮುಂದಿನ, *r_prev;
};

ಸ್ಥಿರ ರಚನೆ fetch_ದತ್ತಾಂಶ_context *ring = ಖಾಲಿ;

ಸ್ಥಿರ bool fetch_ದತ್ತಾಂಶ_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_ದತ್ತಾಂಶ(scheme));

	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_ದತ್ತಾಂಶ_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_ದತ್ತಾಂಶ(scheme));
}

ಸ್ಥಿರ bool fetch_ದತ್ತಾಂಶ_can_fetch(ಸ್ಥಿರಾಂಕ nsurl *url)
{
	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_ದತ್ತಾಂಶ_send_callback(ಸ್ಥಿರಾಂಕ fetch_ಸಂದೇಶ *ಸಂದೇಶ,
		ರಚನೆ fetch_ದತ್ತಾಂಶ_context *c)
{
	c->locked = true;
	fetch_send_callback(ಸಂದೇಶ, c->ಪೋಷಕ_fetch);
	c->locked = false;
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_ದತ್ತಾಂಶ_send_ಶೀರ್ಷಿಕೆ(ರಚನೆ fetch_ದತ್ತಾಂಶ_context *ctx,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	ಅಕ್ಷರ ಶೀರ್ಷಿಕೆ[64];
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	va_list ap;
	ಪೂರ್ಣಾಂಕ len;

	va_start(ap, fmt);
	len = vsnprintf(ಶೀರ್ಷಿಕೆ, ಗಾತ್ರ(ಶೀರ್ಷಿಕೆ), fmt, ap);
	va_end(ap);

	ಆದರೆ (len >= (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(ಶೀರ್ಷಿಕೆ) || len < 0) {
		ಹಿಂತಿರುಗಿ;
	}

	ಸಂದೇಶ.type = FETCH_ಶೀರ್ಷಿಕೆ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = len;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *)ಶೀರ್ಷಿಕೆ;
	fetch_ದತ್ತಾಂಶ_send_callback(&ಸಂದೇಶ, ctx);
}

ಸ್ಥಿರ ಶೂನ್ಯ *fetch_ದತ್ತಾಂಶ_ಸಿದ್ಧತೆ(ರಚನೆ fetch *ಪೋಷಕ_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *post_urlenc,
		 ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *post_multipart,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಶೀರ್ಷಿಕೆಗಳು)
{
	ರಚನೆ fetch_ದತ್ತಾಂಶ_context *ctx = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*ctx));
	
	ಆದರೆ (ctx == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		
	ctx->ಪೋಷಕ_fetch = ಪೋಷಕ_fetch;
	ctx->url = nsurl_ref(url);

	RING_ಸೇರಿಸು(ring, ctx);
	
	ಹಿಂತಿರುಗಿ ctx;
}

ಸ್ಥಿರ bool fetch_ದತ್ತಾಂಶ_ಆರಂಭ(ಶೂನ್ಯ *ctx)
{
	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_ದತ್ತಾಂಶ_free(ಶೂನ್ಯ *ctx)
{
	ರಚನೆ fetch_ದತ್ತಾಂಶ_context *c = ctx;

	nsurl_unref(c->url);
	ಬಿಡುಗಡೆ(c->ದತ್ತಾಂಶ);
	ಬಿಡುಗಡೆ(c->mimetype);
	ಬಿಡುಗಡೆ(ctx);
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_ದತ್ತಾಂಶ_abort(ಶೂನ್ಯ *ctx)
{
	ರಚನೆ fetch_ದತ್ತಾಂಶ_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ಸ್ಥಿರ bool fetch_ದತ್ತಾಂಶ_process(ರಚನೆ fetch_ದತ್ತಾಂಶ_context *c)
{
	nserror res;
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *params;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *comma;
	ಅಕ್ಷರ *unescaped;
	size_t unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	ಆದರೆ (nsurl_ಉದ್ದಳತೆ(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.error = "Malformed data: URL";
		fetch_ದತ್ತಾಂಶ_send_callback(&ಸಂದೇಶ, c);
		ಹಿಂತಿರುಗಿ false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	ಆದರೆ ( (comma = strchr(params, ',')) == ಖಾಲಿ) {
		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.error = "Malformed data: URL";
		fetch_ದತ್ತಾಂಶ_send_callback(&ಸಂದೇಶ, c);
		ಹಿಂತಿರುಗಿ false;
	}
	
	ಆದರೆ (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} ಇಲ್ಲದಿದ್ದರೆ {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	ಆದರೆ (c->mimetype == ಖಾಲಿ) {
		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_ದತ್ತಾಂಶ_send_callback(&ಸಂದೇಶ, c);
		ಹಿಂತಿರುಗಿ false;
	}
	
	ಆದರೆ (strcmp(c->mimetype + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c->mimetype) - 7] = '\0';
	} ಇಲ್ಲದಿದ್ದರೆ {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	ಆದರೆ (res != NSERROR_OK) {
		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.error = "Unable to URL decode data: URL";
		fetch_ದತ್ತಾಂಶ_send_callback(&ಸಂದೇಶ, c);
		ಹಿಂತಿರುಗಿ false;
	}
	
	ಆದರೆ (c->base64) {
		ಆದರೆ ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->ದತ್ತಾಂಶ,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->ದತ್ತಾಂಶ == ಖಾಲಿ)) {
			ಸಂದೇಶ.type = FETCH_ERROR;
			ಸಂದೇಶ.ದತ್ತಾಂಶ.error = "Unable to Base64 decode data: URL";
			fetch_ದತ್ತಾಂಶ_send_callback(&ಸಂದೇಶ, c);
			ಬಿಡುಗಡೆ(unescaped);
			ಹಿಂತಿರುಗಿ false;
		}
		ಬಿಡುಗಡೆ(unescaped);
	} ಇಲ್ಲದಿದ್ದರೆ {
		c->datalen = unescaped_len;
		c->ದತ್ತಾಂಶ = unescaped;
	}
	
	
	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ ಶೂನ್ಯ fetch_ದತ್ತಾಂಶ_poll(lwc_string *scheme)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ರಚನೆ fetch_ದತ್ತಾಂಶ_context *c, *save_ring = ಖಾಲಿ;
	
	/* Iterate over ring, processing each pending fetch */
	ಆಗ (ring != ಖಾಲಿ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called 
		 * again.
		 */
		ಆದರೆ (c->locked == true) {
			RING_ಸೇರಿಸು(save_ring, c);
			ಮುಂದುವರಿ;
		}

		/* Only process non-aborted fetches */
		ಆದರೆ (c->aborted == false && fetch_ದತ್ತಾಂಶ_process(c) == true) {
			fetch_ಗಣ_http_ಸಂಕೇತ(c->ಪೋಷಕ_fetch, 200);
			NSLOG(netsurf, INFO,
			      "setting data: MIME type to %s, length to %"PRIsizet,
			      c->mimetype,
			      c->datalen);
			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to fetch_data_send_callback().
			 */
			fetch_ದತ್ತಾಂಶ_send_ಶೀರ್ಷಿಕೆ(c, "Content-Type: %s",
					c->mimetype);

			ಆದರೆ (c->aborted == false) {
				fetch_ದತ್ತಾಂಶ_send_ಶೀರ್ಷಿಕೆ(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			ಆದರೆ (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_ದತ್ತಾಂಶ_send_ಶೀರ್ಷಿಕೆ(c, "Cache-Control: "
						"max-age=31536000");
			}

			ಆದರೆ (c->aborted == false) {
				ಸಂದೇಶ.type = FETCH_ದತ್ತಾಂಶ;
				ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = 
						(ಸ್ಥಿರಾಂಕ uint8_t *) c->ದತ್ತಾಂಶ;
				ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = c->datalen;
				fetch_ದತ್ತಾಂಶ_send_callback(&ಸಂದೇಶ, c);
			}

			ಆದರೆ (c->aborted == false) {
				ಸಂದೇಶ.type = FETCH_ಪೂರ್ಣಗೊಂಡ;
				fetch_ದತ್ತಾಂಶ_send_callback(&ಸಂದೇಶ, c);
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			NSLOG(netsurf, INFO, "Processing of %.140s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't, 
			 * then fetch_data_process() is broken.
			 */
			ಪ್ರತಿಜ್ಞಾ(c->locked == false);
		}

		/* And now finish */
		fetch_remove_from_queues(c->ಪೋಷಕ_fetch);
		fetch_free(c->ಪೋಷಕ_fetch);
	}

	/* Finally, if we saved any fetches which were locked, put them back
	 * into the ring for next time
	 */
	ring = save_ring;
}

nserror fetch_ದತ್ತಾಂಶ_ರಿಜಿಸ್ಟರ್(ಶೂನ್ಯ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_ದತ್ತಾಂಶ);
	ಸ್ಥಿರಾಂಕ ರಚನೆ fetcher_operation_ಕೋಷ್ಟಕ fetcher_ops = {
		.initialise = fetch_ದತ್ತಾಂಶ_initialise,
		.acceptable = fetch_ದತ್ತಾಂಶ_can_fetch,
		.ಸಿದ್ಧತೆ = fetch_ದತ್ತಾಂಶ_ಸಿದ್ಧತೆ,
		.ಆರಂಭ = fetch_ದತ್ತಾಂಶ_ಆರಂಭ,
		.abort = fetch_ದತ್ತಾಂಶ_abort,
		.ಬಿಡುಗಡೆ = fetch_ದತ್ತಾಂಶ_free,
		.poll = fetch_ದತ್ತಾಂಶ_poll,
		.finalise = fetch_ದತ್ತಾಂಶ_finalise
	};

	ಹಿಂತಿರುಗಿ fetcher_ಕೂಡಿಸು(scheme, &fetcher_ops);
}
