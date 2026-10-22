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

بنية fetch_انکڑے_context {
	بنية fetch *ماپا_fetch;
	nsurl *url;
	حرف *mimetype;
	حرف *انکڑے;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	بنية fetch_انکڑے_context *r_اگلا, *r_prev;
};

ساكن بنية fetch_انکڑے_context *ring = عدم;

ساكن bool fetch_انکڑے_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_انکڑے(scheme));

	أرجع true;
}

ساكن فراغ fetch_انکڑے_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_انکڑے(scheme));
}

ساكن bool fetch_انکڑے_can_fetch(ثابت nsurl *url)
{
	أرجع true;
}

ساكن فراغ fetch_انکڑے_send_callback(ثابت fetch_پیغام *پیغام,
		بنية fetch_انکڑے_context *c)
{
	c->locked = true;
	fetch_send_callback(پیغام, c->ماپا_fetch);
	c->locked = false;
}

ساكن فراغ fetch_انکڑے_send_سرخی(بنية fetch_انکڑے_context *ctx,
		ثابت حرف *fmt, ...)
{
	حرف سرخی[64];
	fetch_پیغام پیغام;
	va_list ap;
	صحيح len;

	va_start(ap, fmt);
	len = vsnprintf(سرخی, حجم(سرخی), fmt, ap);
	va_end(ap);

	إذا (len >= (صحيح)حجم(سرخی) || len < 0) {
		أرجع;
	}

	پیغام.type = FETCH_سرخی;
	پیغام.انکڑے.سرخی_or_انکڑے.len = len;
	پیغام.انکڑے.سرخی_or_انکڑے.buf = (ثابت uint8_t *)سرخی;
	fetch_انکڑے_send_callback(&پیغام, ctx);
}

ساكن فراغ *fetch_انکڑے_تیاری(بنية fetch *ماپا_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_انکڑے *post_multipart,
		 ثابت حرف **سرخیاں)
{
	بنية fetch_انکڑے_context *ctx = صفر_حافظہ_دیو(1, حجم(*ctx));
	
	إذا (ctx == عدم)
		أرجع عدم;
		
	ctx->ماپا_fetch = ماپا_fetch;
	ctx->url = nsurl_ref(url);

	RING_داخل_کرو(ring, ctx);
	
	أرجع ctx;
}

ساكن bool fetch_انکڑے_شروع(فراغ *ctx)
{
	أرجع true;
}

ساكن فراغ fetch_انکڑے_free(فراغ *ctx)
{
	بنية fetch_انکڑے_context *c = ctx;

	nsurl_unref(c->url);
	حرر(c->انکڑے);
	حرر(c->mimetype);
	حرر(ctx);
}

ساكن فراغ fetch_انکڑے_abort(فراغ *ctx)
{
	بنية fetch_انکڑے_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ساكن bool fetch_انکڑے_process(بنية fetch_انکڑے_context *c)
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
	
	إذا (nsurl_لمائی(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		پیغام.type = FETCH_ERROR;
		پیغام.انکڑے.error = "Malformed data: URL";
		fetch_انکڑے_send_callback(&پیغام, c);
		أرجع false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	إذا ( (comma = strchr(params, ',')) == عدم) {
		پیغام.type = FETCH_ERROR;
		پیغام.انکڑے.error = "Malformed data: URL";
		fetch_انکڑے_send_callback(&پیغام, c);
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
		پیغام.type = FETCH_ERROR;
		پیغام.انکڑے.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_انکڑے_send_callback(&پیغام, c);
		أرجع false;
	}
	
	إذا (strcmp(c->mimetype + لڑی_لمبائی(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[لڑی_لمبائی(c->mimetype) - 7] = '\0';
	} وإلا {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	إذا (res != NSERROR_OK) {
		پیغام.type = FETCH_ERROR;
		پیغام.انکڑے.error = "Unable to URL decode data: URL";
		fetch_انکڑے_send_callback(&پیغام, c);
		أرجع false;
	}
	
	إذا (c->base64) {
		إذا ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->انکڑے,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->انکڑے == عدم)) {
			پیغام.type = FETCH_ERROR;
			پیغام.انکڑے.error = "Unable to Base64 decode data: URL";
			fetch_انکڑے_send_callback(&پیغام, c);
			حرر(unescaped);
			أرجع false;
		}
		حرر(unescaped);
	} وإلا {
		c->datalen = unescaped_len;
		c->انکڑے = unescaped;
	}
	
	
	أرجع true;
}

ساكن فراغ fetch_انکڑے_poll(lwc_string *scheme)
{
	fetch_پیغام پیغام;
	بنية fetch_انکڑے_context *c, *save_ring = عدم;
	
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
			RING_داخل_کرو(save_ring, c);
			استمر;
		}

		/* Only process non-aborted fetches */
		إذا (c->aborted == false && fetch_انکڑے_process(c) == true) {
			fetch_سیٹ_http_کوڈ(c->ماپا_fetch, 200);
			NSLOG(netsurf, INFO,
			      "setting data: MIME type to %s, length to %"PRIsizet,
			      c->mimetype,
			      c->datalen);
			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to fetch_data_send_callback().
			 */
			fetch_انکڑے_send_سرخی(c, "Content-Type: %s",
					c->mimetype);

			إذا (c->aborted == false) {
				fetch_انکڑے_send_سرخی(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			إذا (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_انکڑے_send_سرخی(c, "Cache-Control: "
						"max-age=31536000");
			}

			إذا (c->aborted == false) {
				پیغام.type = FETCH_انکڑے;
				پیغام.انکڑے.سرخی_or_انکڑے.buf = 
						(ثابت uint8_t *) c->انکڑے;
				پیغام.انکڑے.سرخی_or_انکڑے.len = c->datalen;
				fetch_انکڑے_send_callback(&پیغام, c);
			}

			إذا (c->aborted == false) {
				پیغام.type = FETCH_مکمل;
				fetch_انکڑے_send_callback(&پیغام, c);
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
		fetch_remove_from_queues(c->ماپا_fetch);
		fetch_free(c->ماپا_fetch);
	}

	/* Finally, if we saved any fetches which were locked, put them back
	 * into the ring for next time
	 */
	ring = save_ring;
}

nserror fetch_انکڑے_رجسٹر(فراغ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_انکڑے);
	ثابت بنية fetcher_operation_جدول fetcher_ops = {
		.initialise = fetch_انکڑے_initialise,
		.acceptable = fetch_انکڑے_can_fetch,
		.تیاری = fetch_انکڑے_تیاری,
		.شروع = fetch_انکڑے_شروع,
		.abort = fetch_انکڑے_abort,
		.حرر = fetch_انکڑے_free,
		.poll = fetch_انکڑے_poll,
		.finalise = fetch_انکڑے_finalise
	};

	أرجع fetcher_جوڑو(scheme, &fetcher_ops);
}
