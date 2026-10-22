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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <libwapcaplet/libwapcaplet.h>
#અન્તર્ભાવ <nsutils/base64.h>

#અન્તર્ભાવ "netsurf/inttypes.h"
#અન્તર્ભાવ "utils/url.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/ring.h"

#અન્તર્ભાવ "content/fetch.h"
#અન્તર્ભાવ "content/fetchers.h"
#અન્તર્ભાવ "content/fetchers/data.h"

સંરચના fetch_data_context {
	સંરચના fetch *parent_fetch;
	nsurl *url;
	અક્ષર *mimetype;
	અક્ષર *data;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	સંરચના fetch_data_context *r_next, *r_prev;
};

સ્થિર સંરચના fetch_data_context *ring = ખાલી;

સ્થિર bool fetch_data_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_data(scheme));

	પાછા true;
}

સ્થિર શૂન્ય fetch_data_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_data(scheme));
}

સ્થિર bool fetch_data_can_fetch(અચલ nsurl *url)
{
	પાછા true;
}

સ્થિર શૂન્ય fetch_data_send_callback(અચલ fetch_સંદેશ *સંદેશ,
		સંરચના fetch_data_context *c)
{
	c->locked = true;
	fetch_send_callback(સંદેશ, c->parent_fetch);
	c->locked = false;
}

સ્થિર શૂન્ય fetch_data_send_header(સંરચના fetch_data_context *ctx,
		અચલ અક્ષર *fmt, ...)
{
	અક્ષર header[64];
	fetch_સંદેશ સંદેશ;
	va_list ap;
	પૂર્ણાંક len;

	va_start(ap, fmt);
	len = vsnprintf(header, કદ(header), fmt, ap);
	va_end(ap);

	જો (len >= (પૂર્ણાંક)કદ(header) || len < 0) {
		પાછા;
	}

	સંદેશ.type = FETCH_HEADER;
	સંદેશ.data.header_or_data.len = len;
	સંદેશ.data.header_or_data.buf = (અચલ uint8_t *)header;
	fetch_data_send_callback(&સંદેશ, ctx);
}

સ્થિર શૂન્ય *fetch_data_setup(સંરચના fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, અચલ અક્ષર *post_urlenc,
		 અચલ સંરચના fetch_multipart_data *post_multipart,
		 અચલ અક્ષર **headers)
{
	સંરચના fetch_data_context *ctx = calloc(1, કદ(*ctx));
	
	જો (ctx == ખાલી)
		પાછા ખાલી;
		
	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);

	RING_ઉમેરો(ring, ctx);
	
	પાછા ctx;
}

સ્થિર bool fetch_data_start(શૂન્ય *ctx)
{
	પાછા true;
}

સ્થિર શૂન્ય fetch_data_free(શૂન્ય *ctx)
{
	સંરચના fetch_data_context *c = ctx;

	nsurl_unref(c->url);
	મુક્ત(c->data);
	મુક્ત(c->mimetype);
	મુક્ત(ctx);
}

સ્થિર શૂન્ય fetch_data_abort(શૂન્ય *ctx)
{
	સંરચના fetch_data_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

સ્થિર bool fetch_data_process(સંરચના fetch_data_context *c)
{
	nserror res;
	fetch_સંદેશ સંદેશ;
	અચલ અક્ષર *params;
	અચલ અક્ષર *comma;
	અક્ષર *unescaped;
	size_t unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	જો (nsurl_length(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		સંદેશ.type = FETCH_ERROR;
		સંદેશ.data.error = "Malformed data: URL";
		fetch_data_send_callback(&સંદેશ, c);
		પાછા false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	જો ( (comma = strchr(params, ',')) == ખાલી) {
		સંદેશ.type = FETCH_ERROR;
		સંદેશ.data.error = "Malformed data: URL";
		fetch_data_send_callback(&સંદેશ, c);
		પાછા false;
	}
	
	જો (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} નહીંતર {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	જો (c->mimetype == ખાલી) {
		સંદેશ.type = FETCH_ERROR;
		સંદેશ.data.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_data_send_callback(&સંદેશ, c);
		પાછા false;
	}
	
	જો (strcmp(c->mimetype + strlen(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[strlen(c->mimetype) - 7] = '\0';
	} નહીંતર {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	જો (res != NSERROR_OK) {
		સંદેશ.type = FETCH_ERROR;
		સંદેશ.data.error = "Unable to URL decode data: URL";
		fetch_data_send_callback(&સંદેશ, c);
		પાછા false;
	}
	
	જો (c->base64) {
		જો ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->data,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->data == ખાલી)) {
			સંદેશ.type = FETCH_ERROR;
			સંદેશ.data.error = "Unable to Base64 decode data: URL";
			fetch_data_send_callback(&સંદેશ, c);
			મુક્ત(unescaped);
			પાછા false;
		}
		મુક્ત(unescaped);
	} નહીંતર {
		c->datalen = unescaped_len;
		c->data = unescaped;
	}
	
	
	પાછા true;
}

સ્થિર શૂન્ય fetch_data_poll(lwc_string *scheme)
{
	fetch_સંદેશ સંદેશ;
	સંરચના fetch_data_context *c, *save_ring = ખાલી;
	
	/* Iterate over ring, processing each pending fetch */
	જ્યારે (ring != ખાલી) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called 
		 * again.
		 */
		જો (c->locked == true) {
			RING_ઉમેરો(save_ring, c);
			ચાલુ;
		}

		/* Only process non-aborted fetches */
		જો (c->aborted == false && fetch_data_process(c) == true) {
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

			જો (c->aborted == false) {
				fetch_data_send_header(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			જો (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_data_send_header(c, "Cache-Control: "
						"max-age=31536000");
			}

			જો (c->aborted == false) {
				સંદેશ.type = FETCH_DATA;
				સંદેશ.data.header_or_data.buf = 
						(અચલ uint8_t *) c->data;
				સંદેશ.data.header_or_data.len = c->datalen;
				fetch_data_send_callback(&સંદેશ, c);
			}

			જો (c->aborted == false) {
				સંદેશ.type = FETCH_FINISHED;
				fetch_data_send_callback(&સંદેશ, c);
			}
		} નહીંતર {
			NSLOG(netsurf, INFO, "Processing of %.140s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't, 
			 * then fetch_data_process() is broken.
			 */
			પ્રતિજ્ઞા(c->locked == false);
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

nserror fetch_data_register(શૂન્ય)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_data);
	અચલ સંરચના fetcher_operation_table fetcher_ops = {
		.initialise = fetch_data_initialise,
		.acceptable = fetch_data_can_fetch,
		.setup = fetch_data_setup,
		.start = fetch_data_start,
		.abort = fetch_data_abort,
		.મુક્ત = fetch_data_free,
		.poll = fetch_data_poll,
		.finalise = fetch_data_finalise
	};

	પાછા fetcher_add(scheme, &fetcher_ops);
}
