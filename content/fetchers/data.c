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

بنية fetch_معلومات_context {
	بنية fetch *والد_fetch;
	nsurl *url;
	حرف *mimetype;
	حرف *معلومات;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	بنية fetch_معلومات_context *r_ايندڙ, *r_prev;
};

ساكن بنية fetch_معلومات_context *ring = عدم;

ساكن bool fetch_معلومات_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_معلومات(scheme));

	أرجع true;
}

ساكن فراغ fetch_معلومات_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_معلومات(scheme));
}

ساكن bool fetch_معلومات_can_fetch(ثابت nsurl *url)
{
	أرجع true;
}

ساكن فراغ fetch_معلومات_send_callback(ثابت fetch_نياپو *نياپو,
		بنية fetch_معلومات_context *c)
{
	c->locked = true;
	fetch_send_callback(نياپو, c->والد_fetch);
	c->locked = false;
}

ساكن فراغ fetch_معلومات_send_سرو(بنية fetch_معلومات_context *ctx,
		ثابت حرف *fmt, ...)
{
	حرف سرو[64];
	fetch_نياپو نياپو;
	va_list ap;
	صحيح len;

	va_start(ap, fmt);
	len = vsnprintf(سرو, حجم(سرو), fmt, ap);
	va_end(ap);

	إذا (len >= (صحيح)حجم(سرو) || len < 0) {
		أرجع;
	}

	نياپو.type = FETCH_سرو;
	نياپو.معلومات.سرو_or_معلومات.len = len;
	نياپو.معلومات.سرو_or_معلومات.buf = (ثابت uint8_t *)سرو;
	fetch_معلومات_send_callback(&نياپو, ctx);
}

ساكن فراغ *fetch_معلومات_تياري(بنية fetch *والد_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_معلومات *post_multipart,
		 ثابت حرف **سرا)
{
	بنية fetch_معلومات_context *ctx = صفر_يادگيري_ڏيو(1, حجم(*ctx));
	
	إذا (ctx == عدم)
		أرجع عدم;
		
	ctx->والد_fetch = والد_fetch;
	ctx->url = nsurl_ref(url);

	RING_وجهو(ring, ctx);
	
	أرجع ctx;
}

ساكن bool fetch_معلومات_شروعات(فراغ *ctx)
{
	أرجع true;
}

ساكن فراغ fetch_معلومات_free(فراغ *ctx)
{
	بنية fetch_معلومات_context *c = ctx;

	nsurl_unref(c->url);
	حرر(c->معلومات);
	حرر(c->mimetype);
	حرر(ctx);
}

ساكن فراغ fetch_معلومات_abort(فراغ *ctx)
{
	بنية fetch_معلومات_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ساكن bool fetch_معلومات_process(بنية fetch_معلومات_context *c)
{
	nserror res;
	fetch_نياپو نياپو;
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
	
	إذا (nsurl_ڊيگهائي(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		نياپو.type = FETCH_ERROR;
		نياپو.معلومات.error = "Malformed data: URL";
		fetch_معلومات_send_callback(&نياپو, c);
		أرجع false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	إذا ( (comma = strchr(params, ',')) == عدم) {
		نياپو.type = FETCH_ERROR;
		نياپو.معلومات.error = "Malformed data: URL";
		fetch_معلومات_send_callback(&نياپو, c);
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
		نياپو.type = FETCH_ERROR;
		نياپو.معلومات.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_معلومات_send_callback(&نياپو, c);
		أرجع false;
	}
	
	إذا (strcmp(c->mimetype + لڙي_ڊيگهه(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[لڙي_ڊيگهه(c->mimetype) - 7] = '\0';
	} وإلا {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	إذا (res != NSERROR_OK) {
		نياپو.type = FETCH_ERROR;
		نياپو.معلومات.error = "Unable to URL decode data: URL";
		fetch_معلومات_send_callback(&نياپو, c);
		أرجع false;
	}
	
	إذا (c->base64) {
		إذا ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->معلومات,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->معلومات == عدم)) {
			نياپو.type = FETCH_ERROR;
			نياپو.معلومات.error = "Unable to Base64 decode data: URL";
			fetch_معلومات_send_callback(&نياپو, c);
			حرر(unescaped);
			أرجع false;
		}
		حرر(unescaped);
	} وإلا {
		c->datalen = unescaped_len;
		c->معلومات = unescaped;
	}
	
	
	أرجع true;
}

ساكن فراغ fetch_معلومات_poll(lwc_string *scheme)
{
	fetch_نياپو نياپو;
	بنية fetch_معلومات_context *c, *save_ring = عدم;
	
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
			RING_وجهو(save_ring, c);
			استمر;
		}

		/* Only process non-aborted fetches */
		إذا (c->aborted == false && fetch_معلومات_process(c) == true) {
			fetch_سيٽ_http_ڪوڊ(c->والد_fetch, 200);
			NSLOG(netsurf, INFO,
			      "setting data: MIME type to %s, length to %"PRIsizet,
			      c->mimetype,
			      c->datalen);
			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to fetch_data_send_callback().
			 */
			fetch_معلومات_send_سرو(c, "Content-Type: %s",
					c->mimetype);

			إذا (c->aborted == false) {
				fetch_معلومات_send_سرو(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			إذا (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_معلومات_send_سرو(c, "Cache-Control: "
						"max-age=31536000");
			}

			إذا (c->aborted == false) {
				نياپو.type = FETCH_معلومات;
				نياپو.معلومات.سرو_or_معلومات.buf = 
						(ثابت uint8_t *) c->معلومات;
				نياپو.معلومات.سرو_or_معلومات.len = c->datalen;
				fetch_معلومات_send_callback(&نياپو, c);
			}

			إذا (c->aborted == false) {
				نياپو.type = FETCH_ختم;
				fetch_معلومات_send_callback(&نياپو, c);
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

nserror fetch_معلومات_رجسٽر(فراغ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_معلومات);
	ثابت بنية fetcher_operation_جدول fetcher_ops = {
		.initialise = fetch_معلومات_initialise,
		.acceptable = fetch_معلومات_can_fetch,
		.تياري = fetch_معلومات_تياري,
		.شروعات = fetch_معلومات_شروعات,
		.abort = fetch_معلومات_abort,
		.حرر = fetch_معلومات_free,
		.poll = fetch_معلومات_poll,
		.finalise = fetch_معلومات_finalise
	};

	أرجع fetcher_جوڙيو(scheme, &fetcher_ops);
}
