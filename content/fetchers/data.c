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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <libwapcaplet/libwapcaplet.h>
#अन्तर्भाव <nsutils/base64.h>

#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "utils/url.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/ring.h"

#अन्तर्भाव "content/fetch.h"
#अन्तर्भाव "content/fetchers.h"
#अन्तर्भाव "content/fetchers/data.h"

संरचना fetch_दत्त_context {
	संरचना fetch *parent_fetch;
	nsurl *url;
	वर्ण *mimetype;
	वर्ण *दत्त;
	size_t datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	संरचना fetch_दत्त_context *r_अग्र, *r_पूर्व;
};

स्थिर संरचना fetch_दत्त_context *ring = रिक्त;

स्थिर bool fetch_दत्त_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_दत्त(scheme));

	प्रत्यागम true;
}

स्थिर शून्य fetch_दत्त_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_दत्त(scheme));
}

स्थिर bool fetch_दत्त_can_fetch(ध्रुव nsurl *url)
{
	प्रत्यागम true;
}

स्थिर शून्य fetch_दत्त_send_callback(ध्रुव fetch_सन्देशः *सन्देशः,
		संरचना fetch_दत्त_context *c)
{
	c->locked = true;
	fetch_send_callback(सन्देशः, c->parent_fetch);
	c->locked = false;
}

स्थिर शून्य fetch_दत्त_send_शीर्षकम्(संरचना fetch_दत्त_context *ctx,
		ध्रुव वर्ण *fmt, ...)
{
	वर्ण शीर्षकम्[64];
	fetch_सन्देशः सन्देशः;
	va_list ap;
	पूर्णाङ्क len;

	va_start(ap, fmt);
	len = vsnprintf(शीर्षकम्, परिमाण(शीर्षकम्), fmt, ap);
	va_end(ap);

	यदि (len >= (पूर्णाङ्क)परिमाण(शीर्षकम्) || len < 0) {
		प्रत्यागम;
	}

	सन्देशः.type = FETCH_शीर्षकम्;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = len;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *)शीर्षकम्;
	fetch_दत्त_send_callback(&सन्देशः, ctx);
}

स्थिर शून्य *fetch_दत्त_सज्जता(संरचना fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ध्रुव वर्ण *post_urlenc,
		 ध्रुव संरचना fetch_multipart_दत्त *post_multipart,
		 ध्रुव वर्ण **शीर्षकाणि)
{
	संरचना fetch_दत्त_context *ctx = शून्यस्मृतिं_देहि(1, परिमाण(*ctx));
	
	यदि (ctx == रिक्त)
		प्रत्यागम रिक्त;
		
	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);

	RING_निवेशय(ring, ctx);
	
	प्रत्यागम ctx;
}

स्थिर bool fetch_दत्त_start(शून्य *ctx)
{
	प्रत्यागम true;
}

स्थिर शून्य fetch_दत्त_free(शून्य *ctx)
{
	संरचना fetch_दत्त_context *c = ctx;

	nsurl_unref(c->url);
	मुक्त(c->दत्त);
	मुक्त(c->mimetype);
	मुक्त(ctx);
}

स्थिर शून्य fetch_दत्त_abort(शून्य *ctx)
{
	संरचना fetch_दत्त_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

स्थिर bool fetch_दत्त_process(संरचना fetch_दत्त_context *c)
{
	nserror res;
	fetch_सन्देशः सन्देशः;
	ध्रुव वर्ण *params;
	ध्रुव वर्ण *comma;
	वर्ण *unescaped;
	size_t unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	यदि (nsurl_दीर्घता(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		सन्देशः.type = FETCH_ERROR;
		सन्देशः.दत्त.error = "Malformed data: URL";
		fetch_दत्त_send_callback(&सन्देशः, c);
		प्रत्यागम false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	यदि ( (comma = strchr(params, ',')) == रिक्त) {
		सन्देशः.type = FETCH_ERROR;
		सन्देशः.दत्त.error = "Malformed data: URL";
		fetch_दत्त_send_callback(&सन्देशः, c);
		प्रत्यागम false;
	}
	
	यदि (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} अन्यथा {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	यदि (c->mimetype == रिक्त) {
		सन्देशः.type = FETCH_ERROR;
		सन्देशः.दत्त.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_दत्त_send_callback(&सन्देशः, c);
		प्रत्यागम false;
	}
	
	यदि (वाक्यं_तुलय(c->mimetype + वाक्यदैर्घ्यम्(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[वाक्यदैर्घ्यम्(c->mimetype) - 7] = '\0';
	} अन्यथा {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	यदि (res != NSERROR_OK) {
		सन्देशः.type = FETCH_ERROR;
		सन्देशः.दत्त.error = "Unable to URL decode data: URL";
		fetch_दत्त_send_callback(&सन्देशः, c);
		प्रत्यागम false;
	}
	
	यदि (c->base64) {
		यदि ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->दत्त,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->दत्त == रिक्त)) {
			सन्देशः.type = FETCH_ERROR;
			सन्देशः.दत्त.error = "Unable to Base64 decode data: URL";
			fetch_दत्त_send_callback(&सन्देशः, c);
			मुक्त(unescaped);
			प्रत्यागम false;
		}
		मुक्त(unescaped);
	} अन्यथा {
		c->datalen = unescaped_len;
		c->दत्त = unescaped;
	}
	
	
	प्रत्यागम true;
}

स्थिर शून्य fetch_दत्त_poll(lwc_string *scheme)
{
	fetch_सन्देशः सन्देशः;
	संरचना fetch_दत्त_context *c, *save_ring = रिक्त;
	
	/* Iterate over ring, processing each pending fetch */
	यावत् (ring != रिक्त) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called 
		 * again.
		 */
		यदि (c->locked == true) {
			RING_निवेशय(save_ring, c);
			अनुवर्त;
		}

		/* Only process non-aborted fetches */
		यदि (c->aborted == false && fetch_दत्त_process(c) == true) {
			fetch_समुच्चयः_http_संहिता(c->parent_fetch, 200);
			NSLOG(netsurf, INFO,
			      "setting data: MIME type to %s, length to %"PRIsizet,
			      c->mimetype,
			      c->datalen);
			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to fetch_data_send_callback().
			 */
			fetch_दत्त_send_शीर्षकम्(c, "Content-Type: %s",
					c->mimetype);

			यदि (c->aborted == false) {
				fetch_दत्त_send_शीर्षकम्(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			यदि (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_दत्त_send_शीर्षकम्(c, "Cache-Control: "
						"max-age=31536000");
			}

			यदि (c->aborted == false) {
				सन्देशः.type = FETCH_दत्त;
				सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = 
						(ध्रुव uint8_t *) c->दत्त;
				सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = c->datalen;
				fetch_दत्त_send_callback(&सन्देशः, c);
			}

			यदि (c->aborted == false) {
				सन्देशः.type = FETCH_परिसमाप्तम्;
				fetch_दत्त_send_callback(&सन्देशः, c);
			}
		} अन्यथा {
			NSLOG(netsurf, INFO, "Processing of %.140s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't, 
			 * then fetch_data_process() is broken.
			 */
			प्रतिज्ञा(c->locked == false);
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

nserror fetch_दत्त_पञ्जिकाकोशः(शून्य)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_दत्त);
	ध्रुव संरचना fetcher_operation_सारणी fetcher_ops = {
		.initialise = fetch_दत्त_initialise,
		.acceptable = fetch_दत्त_can_fetch,
		.सज्जता = fetch_दत्त_सज्जता,
		.start = fetch_दत्त_start,
		.abort = fetch_दत्त_abort,
		.मुक्त = fetch_दत्त_free,
		.poll = fetch_दत्त_poll,
		.finalise = fetch_दत्त_finalise
	};

	प्रत्यागम fetcher_सङ्कलय(scheme, &fetcher_ops);
}
