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

#ضمن <stdbool.h>
#ضمن <string.h>
#ضمن <stdarg.h>
#ضمن <stdlib.h>
#ضمن <libwapcaplet/libwapcaplet.h>
#ضمن <nsutils/base64.h>

#ضمن "netsurf/inttypes.h"
#ضمن "utils/url.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"
#ضمن "utils/utils.h"
#ضمن "utils/ring.h"

#ضمن "content/fetch.h"
#ضمن "content/fetchers.h"
#ضمن "content/fetchers/data.h"

بنية fetch_داده_context {
	بنية fetch *والد_fetch;
	nsurl *url;
	حرف *mimetype;
	حرف *داده;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	بنية fetch_داده_context *r_بعدی, *r_prev;
};

ساكن بنية fetch_داده_context *ring = عدم;

ساكن bool fetch_داده_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_داده(scheme));

	أرجع true;
}

ساكن فراغ fetch_داده_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_داده(scheme));
}

ساكن bool fetch_داده_can_fetch(ثابت nsurl *url)
{
	أرجع true;
}

ساكن فراغ fetch_داده_send_callback(ثابت fetch_پیام *پیام,
		بنية fetch_داده_context *c)
{
	c->locked = true;
	fetch_send_callback(پیام, c->والد_fetch);
	c->locked = false;
}

ساكن فراغ fetch_داده_send_سرآیند(بنية fetch_داده_context *ctx,
		ثابت حرف *fmt, ...)
{
	حرف سرآیند[64];
	fetch_پیام پیام;
	va_list ap;
	صحيح len;

	va_start(ap, fmt);
	len = vsnprintf(سرآیند, حجم(سرآیند), fmt, ap);
	va_end(ap);

	إذا (len >= (صحيح)حجم(سرآیند) || len < 0) {
		أرجع;
	}

	پیام.type = FETCH_سرآیند;
	پیام.داده.سرآیند_or_داده.len = len;
	پیام.داده.سرآیند_or_داده.buf = (ثابت uint8_t *)سرآیند;
	fetch_داده_send_callback(&پیام, ctx);
}

ساكن فراغ *fetch_داده_راهاندازی(بنية fetch *والد_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_داده *post_multipart,
		 ثابت حرف **سرآیندها)
{
	بنية fetch_داده_context *ctx = حافظه_صفر_بده(1, حجم(*ctx));
	
	إذا (ctx == عدم)
		أرجع عدم;
		
	ctx->والد_fetch = والد_fetch;
	ctx->url = nsurl_ref(url);

	RING_درج(ring, ctx);
	
	أرجع ctx;
}

ساكن bool fetch_داده_شروع(فراغ *ctx)
{
	أرجع true;
}

ساكن فراغ fetch_داده_free(فراغ *ctx)
{
	بنية fetch_داده_context *c = ctx;

	nsurl_unref(c->url);
	حرر(c->داده);
	حرر(c->mimetype);
	حرر(ctx);
}

ساكن فراغ fetch_داده_abort(فراغ *ctx)
{
	بنية fetch_داده_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ساكن bool fetch_داده_process(بنية fetch_داده_context *c)
{
	nserror res;
	fetch_پیام پیام;
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
	
	إذا (nsurl_درازی(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		پیام.type = FETCH_ERROR;
		پیام.داده.error = "Malformed data: URL";
		fetch_داده_send_callback(&پیام, c);
		أرجع false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	إذا ( (comma = strchr(params, ',')) == عدم) {
		پیام.type = FETCH_ERROR;
		پیام.داده.error = "Malformed data: URL";
		fetch_داده_send_callback(&پیام, c);
		أرجع false;
	}
	
	إذا (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} وإلا {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	إذا (c->mimetype == عدم) {
		پیام.type = FETCH_ERROR;
		پیام.داده.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_داده_send_callback(&پیام, c);
		أرجع false;
	}
	
	إذا (strcmp(c->mimetype + طول_رشته(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[طول_رشته(c->mimetype) - 7] = '\0';
	} وإلا {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	إذا (res != NSERROR_OK) {
		پیام.type = FETCH_ERROR;
		پیام.داده.error = "Unable to URL decode data: URL";
		fetch_داده_send_callback(&پیام, c);
		أرجع false;
	}
	
	إذا (c->base64) {
		إذا ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->داده,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->داده == عدم)) {
			پیام.type = FETCH_ERROR;
			پیام.داده.error = "Unable to Base64 decode data: URL";
			fetch_داده_send_callback(&پیام, c);
			حرر(unescaped);
			أرجع false;
		}
		حرر(unescaped);
	} وإلا {
		c->datalen = unescaped_len;
		c->داده = unescaped;
	}
	
	
	أرجع true;
}

ساكن فراغ fetch_داده_poll(lwc_string *scheme)
{
	fetch_پیام پیام;
	بنية fetch_داده_context *c, *save_ring = عدم;
	
	/* Iterate over ring, processing each pending fetch */
	طالما (ring != عدم) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called 
		 * again.
		 */
		إذا (c->locked == true) {
			RING_درج(save_ring, c);
			استمر;
		}

		/* Only process non-aborted fetches */
		إذا (c->aborted == false && fetch_داده_process(c) == true) {
			fetch_مجموعه_http_کد(c->والد_fetch, 200);
			NSLOG(netsurf, INFO,
			      "setting data: MIME type to %s, length to %"PRIsizet,
			      c->mimetype,
			      c->datalen);
			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to fetch_data_send_callback().
			 */
			fetch_داده_send_سرآیند(c, "Content-Type: %s",
					c->mimetype);

			إذا (c->aborted == false) {
				fetch_داده_send_سرآیند(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			إذا (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_داده_send_سرآیند(c, "Cache-Control: "
						"max-age=31536000");
			}

			إذا (c->aborted == false) {
				پیام.type = FETCH_داده;
				پیام.داده.سرآیند_or_داده.buf = 
						(ثابت uint8_t *) c->داده;
				پیام.داده.سرآیند_or_داده.len = c->datalen;
				fetch_داده_send_callback(&پیام, c);
			}

			إذا (c->aborted == false) {
				پیام.type = FETCH_پایانیافته;
				fetch_داده_send_callback(&پیام, c);
			}
		} وإلا {
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

nserror fetch_داده_ثبات(فراغ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_داده);
	ثابت بنية fetcher_operation_جدول fetcher_ops = {
		.initialise = fetch_داده_initialise,
		.acceptable = fetch_داده_can_fetch,
		.راهاندازی = fetch_داده_راهاندازی,
		.شروع = fetch_داده_شروع,
		.abort = fetch_داده_abort,
		.حرر = fetch_داده_free,
		.poll = fetch_داده_poll,
		.finalise = fetch_داده_finalise
	};

	أرجع fetcher_جمع_کن(scheme, &fetcher_ops);
}
