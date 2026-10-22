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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <libwapcaplet/libwapcaplet.h>
#അന്തര്ഭാവ <nsutils/base64.h>

#അന്തര്ഭാവ "netsurf/inttypes.h"
#അന്തര്ഭാവ "utils/url.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/ring.h"

#അന്തര്ഭാവ "content/fetch.h"
#അന്തര്ഭാവ "content/fetchers.h"
#അന്തര്ഭാവ "content/fetchers/data.h"

ഘടന fetch_data_context {
	ഘടന fetch *parent_fetch;
	nsurl *url;
	അക്ഷരം *mimetype;
	അക്ഷരം *data;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	ഘടന fetch_data_context *r_next, *r_prev;
};

സ്ഥിതം ഘടന fetch_data_context *ring = ശൂന്യ;

സ്ഥിതം bool fetch_data_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_data(scheme));

	തിരികെ true;
}

സ്ഥിതം ശൂന്യം fetch_data_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_data(scheme));
}

സ്ഥിതം bool fetch_data_can_fetch(സ്ഥിരം nsurl *url)
{
	തിരികെ true;
}

സ്ഥിതം ശൂന്യം fetch_data_send_callback(സ്ഥിരം fetch_സന്ദേശം *സന്ദേശം,
		ഘടന fetch_data_context *c)
{
	c->locked = true;
	fetch_send_callback(സന്ദേശം, c->parent_fetch);
	c->locked = false;
}

സ്ഥിതം ശൂന്യം fetch_data_send_header(ഘടന fetch_data_context *ctx,
		സ്ഥിരം അക്ഷരം *fmt, ...)
{
	അക്ഷരം header[64];
	fetch_സന്ദേശം സന്ദേശം;
	va_list ap;
	പൂർണ്ണസംഖ്യ len;

	va_start(ap, fmt);
	len = vsnprintf(header, വലുപ്പം(header), fmt, ap);
	va_end(ap);

	എങ്കിൽ (len >= (പൂർണ്ണസംഖ്യ)വലുപ്പം(header) || len < 0) {
		തിരികെ;
	}

	സന്ദേശം.type = FETCH_HEADER;
	സന്ദേശം.data.header_or_data.len = len;
	സന്ദേശം.data.header_or_data.buf = (സ്ഥിരം uint8_t *)header;
	fetch_data_send_callback(&സന്ദേശം, ctx);
}

സ്ഥിതം ശൂന്യം *fetch_data_setup(ഘടന fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, സ്ഥിരം അക്ഷരം *post_urlenc,
		 സ്ഥിരം ഘടന fetch_multipart_data *post_multipart,
		 സ്ഥിരം അക്ഷരം **headers)
{
	ഘടന fetch_data_context *ctx = calloc(1, വലുപ്പം(*ctx));
	
	എങ്കിൽ (ctx == ശൂന്യ)
		തിരികെ ശൂന്യ;
		
	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);

	RING_തിരുകുക(ring, ctx);
	
	തിരികെ ctx;
}

സ്ഥിതം bool fetch_data_start(ശൂന്യം *ctx)
{
	തിരികെ true;
}

സ്ഥിതം ശൂന്യം fetch_data_free(ശൂന്യം *ctx)
{
	ഘടന fetch_data_context *c = ctx;

	nsurl_unref(c->url);
	സ്വതന്ത്രം(c->data);
	സ്വതന്ത്രം(c->mimetype);
	സ്വതന്ത്രം(ctx);
}

സ്ഥിതം ശൂന്യം fetch_data_abort(ശൂന്യം *ctx)
{
	ഘടന fetch_data_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

സ്ഥിതം bool fetch_data_process(ഘടന fetch_data_context *c)
{
	nserror res;
	fetch_സന്ദേശം സന്ദേശം;
	സ്ഥിരം അക്ഷരം *params;
	സ്ഥിരം അക്ഷരം *comma;
	അക്ഷരം *unescaped;
	size_t unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	എങ്കിൽ (nsurl_length(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		സന്ദേശം.type = FETCH_ERROR;
		സന്ദേശം.data.error = "Malformed data: URL";
		fetch_data_send_callback(&സന്ദേശം, c);
		തിരികെ false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	എങ്കിൽ ( (comma = strchr(params, ',')) == ശൂന്യ) {
		സന്ദേശം.type = FETCH_ERROR;
		സന്ദേശം.data.error = "Malformed data: URL";
		fetch_data_send_callback(&സന്ദേശം, c);
		തിരികെ false;
	}
	
	എങ്കിൽ (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} അല്ലെങ്കിൽ {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	എങ്കിൽ (c->mimetype == ശൂന്യ) {
		സന്ദേശം.type = FETCH_ERROR;
		സന്ദേശം.data.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_data_send_callback(&സന്ദേശം, c);
		തിരികെ false;
	}
	
	എങ്കിൽ (strcmp(c->mimetype + strlen(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[strlen(c->mimetype) - 7] = '\0';
	} അല്ലെങ്കിൽ {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	എങ്കിൽ (res != NSERROR_OK) {
		സന്ദേശം.type = FETCH_ERROR;
		സന്ദേശം.data.error = "Unable to URL decode data: URL";
		fetch_data_send_callback(&സന്ദേശം, c);
		തിരികെ false;
	}
	
	എങ്കിൽ (c->base64) {
		എങ്കിൽ ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->data,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->data == ശൂന്യ)) {
			സന്ദേശം.type = FETCH_ERROR;
			സന്ദേശം.data.error = "Unable to Base64 decode data: URL";
			fetch_data_send_callback(&സന്ദേശം, c);
			സ്വതന്ത്രം(unescaped);
			തിരികെ false;
		}
		സ്വതന്ത്രം(unescaped);
	} അല്ലെങ്കിൽ {
		c->datalen = unescaped_len;
		c->data = unescaped;
	}
	
	
	തിരികെ true;
}

സ്ഥിതം ശൂന്യം fetch_data_poll(lwc_string *scheme)
{
	fetch_സന്ദേശം സന്ദേശം;
	ഘടന fetch_data_context *c, *save_ring = ശൂന്യ;
	
	/* Iterate over ring, processing each pending fetch */
	ആകുമ്പോൾ (ring != ശൂന്യ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called 
		 * again.
		 */
		എങ്കിൽ (c->locked == true) {
			RING_തിരുകുക(save_ring, c);
			തുടരുക;
		}

		/* Only process non-aborted fetches */
		എങ്കിൽ (c->aborted == false && fetch_data_process(c) == true) {
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

			എങ്കിൽ (c->aborted == false) {
				fetch_data_send_header(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			എങ്കിൽ (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_data_send_header(c, "Cache-Control: "
						"max-age=31536000");
			}

			എങ്കിൽ (c->aborted == false) {
				സന്ദേശം.type = FETCH_DATA;
				സന്ദേശം.data.header_or_data.buf = 
						(സ്ഥിരം uint8_t *) c->data;
				സന്ദേശം.data.header_or_data.len = c->datalen;
				fetch_data_send_callback(&സന്ദേശം, c);
			}

			എങ്കിൽ (c->aborted == false) {
				സന്ദേശം.type = FETCH_FINISHED;
				fetch_data_send_callback(&സന്ദേശം, c);
			}
		} അല്ലെങ്കിൽ {
			NSLOG(netsurf, INFO, "Processing of %.140s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't, 
			 * then fetch_data_process() is broken.
			 */
			പ്രതിജ്ഞാ(c->locked == false);
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

nserror fetch_data_register(ശൂന്യം)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_data);
	സ്ഥിരം ഘടന fetcher_operation_table fetcher_ops = {
		.initialise = fetch_data_initialise,
		.acceptable = fetch_data_can_fetch,
		.setup = fetch_data_setup,
		.start = fetch_data_start,
		.abort = fetch_data_abort,
		.സ്വതന്ത്രം = fetch_data_free,
		.poll = fetch_data_poll,
		.finalise = fetch_data_finalise
	};

	തിരികെ fetcher_add(scheme, &fetcher_ops);
}
