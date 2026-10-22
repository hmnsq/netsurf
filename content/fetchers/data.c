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

بنية fetch_data_context {
	بنية fetch *parent_fetch;
	nsurl *url;
	حرف *mimetype;
	حرف *data;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	بنية fetch_data_context *r_next, *r_prev;
};

ساكن بنية fetch_data_context *ring = عدم;

ساكن bool fetch_data_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_data(scheme));

	أرجع true;
}

ساكن فراغ fetch_data_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_data(scheme));
}

ساكن bool fetch_data_can_fetch(ثابت nsurl *url)
{
	أرجع true;
}

ساكن فراغ fetch_data_send_callback(ثابت fetch_msg *msg,
		بنية fetch_data_context *c)
{
	c->locked = true;
	fetch_send_callback(msg, c->parent_fetch);
	c->locked = false;
}

ساكن فراغ fetch_data_send_header(بنية fetch_data_context *ctx,
		ثابت حرف *fmt, ...)
{
	حرف header[64];
	fetch_msg msg;
	va_list ap;
	صحيح len;

	va_start(ap, fmt);
	len = vsnprintf(header, حجم(header), fmt, ap);
	va_end(ap);

	𐤀𐤌 (len >= (صحيح)حجم(header) || len < 0) {
		أرجع;
	}

	msg.type = FETCH_HEADER;
	msg.data.header_or_data.len = len;
	msg.data.header_or_data.buf = (ثابت uint8_t *)header;
	fetch_data_send_callback(&msg, ctx);
}

ساكن فراغ *fetch_data_setup(بنية fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_data *post_multipart,
		 ثابت حرف **headers)
{
	بنية fetch_data_context *ctx = calloc(1, حجم(*ctx));
	
	𐤀𐤌 (ctx == عدم)
		أرجع عدم;
		
	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);

	RING_INSERT(ring, ctx);
	
	أرجع ctx;
}

ساكن bool fetch_data_start(فراغ *ctx)
{
	أرجع true;
}

ساكن فراغ fetch_data_free(فراغ *ctx)
{
	بنية fetch_data_context *c = ctx;

	nsurl_unref(c->url);
	حرر(c->data);
	حرر(c->mimetype);
	حرر(ctx);
}

ساكن فراغ fetch_data_abort(فراغ *ctx)
{
	بنية fetch_data_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ساكن bool fetch_data_process(بنية fetch_data_context *c)
{
	nserror res;
	fetch_msg msg;
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
	
	𐤀𐤌 (nsurl_length(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		msg.type = FETCH_ERROR;
		msg.data.error = "Malformed data: URL";
		fetch_data_send_callback(&msg, c);
		أرجع false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	𐤀𐤌 ( (comma = strchr(params, ',')) == عدم) {
		msg.type = FETCH_ERROR;
		msg.data.error = "Malformed data: URL";
		fetch_data_send_callback(&msg, c);
		أرجع false;
	}
	
	𐤀𐤌 (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} وإلا {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	𐤀𐤌 (c->mimetype == عدم) {
		msg.type = FETCH_ERROR;
		msg.data.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_data_send_callback(&msg, c);
		أرجع false;
	}
	
	𐤀𐤌 (strcmp(c->mimetype + strlen(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[strlen(c->mimetype) - 7] = '\0';
	} وإلا {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	𐤀𐤌 (res != NSERROR_OK) {
		msg.type = FETCH_ERROR;
		msg.data.error = "Unable to URL decode data: URL";
		fetch_data_send_callback(&msg, c);
		أرجع false;
	}
	
	𐤀𐤌 (c->base64) {
		𐤀𐤌 ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->data,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->data == عدم)) {
			msg.type = FETCH_ERROR;
			msg.data.error = "Unable to Base64 decode data: URL";
			fetch_data_send_callback(&msg, c);
			حرر(unescaped);
			أرجع false;
		}
		حرر(unescaped);
	} وإلا {
		c->datalen = unescaped_len;
		c->data = unescaped;
	}
	
	
	أرجع true;
}

ساكن فراغ fetch_data_poll(lwc_string *scheme)
{
	fetch_msg msg;
	بنية fetch_data_context *c, *save_ring = عدم;
	
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
		𐤀𐤌 (c->locked == true) {
			RING_INSERT(save_ring, c);
			استمر;
		}

		/* Only process non-aborted fetches */
		𐤀𐤌 (c->aborted == false && fetch_data_process(c) == true) {
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

			𐤀𐤌 (c->aborted == false) {
				fetch_data_send_header(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			𐤀𐤌 (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_data_send_header(c, "Cache-Control: "
						"max-age=31536000");
			}

			𐤀𐤌 (c->aborted == false) {
				msg.type = FETCH_DATA;
				msg.data.header_or_data.buf = 
						(ثابت uint8_t *) c->data;
				msg.data.header_or_data.len = c->datalen;
				fetch_data_send_callback(&msg, c);
			}

			𐤀𐤌 (c->aborted == false) {
				msg.type = FETCH_FINISHED;
				fetch_data_send_callback(&msg, c);
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
		fetch_remove_from_queues(c->parent_fetch);
		fetch_free(c->parent_fetch);
	}

	/* Finally, if we saved any fetches which were locked, put them back
	 * into the ring for next time
	 */
	ring = save_ring;
}

nserror fetch_data_register(فراغ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_data);
	ثابت بنية fetcher_operation_table fetcher_ops = {
		.initialise = fetch_data_initialise,
		.acceptable = fetch_data_can_fetch,
		.setup = fetch_data_setup,
		.start = fetch_data_start,
		.abort = fetch_data_abort,
		.حرر = fetch_data_free,
		.poll = fetch_data_poll,
		.finalise = fetch_data_finalise
	};

	أرجع fetcher_add(scheme, &fetcher_ops);
}
