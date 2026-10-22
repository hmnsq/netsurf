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

#समावेश <stdbool.h>
#समावेश <string.h>
#समावेश <stdarg.h>
#समावेश <stdlib.h>
#समावेश <libwapcaplet/libwapcaplet.h>
#समावेश <nsutils/base64.h>

#समावेश "netsurf/inttypes.h"
#समावेश "utils/url.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/log.h"
#समावेश "utils/utils.h"
#समावेश "utils/ring.h"

#समावेश "content/fetch.h"
#समावेश "content/fetchers.h"
#समावेश "content/fetchers/data.h"

काष्ठा fetch_तथ्य_context {
	काष्ठा fetch *parent_fetch;
	nsurl *url;
	अक्षर *mimetype;
	अक्षर *तथ्य;
	माप_प्रकार datalen;
	bool base64;

	bool aborted;
	bool locked;
	
	काष्ठा fetch_तथ्य_context *r_अग्र, *r_पूर्व;
};

अटल काष्ठा fetch_तथ्य_context *ring = शून्य;

अटल bool fetch_तथ्य_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_initialise called for %s",
	      lwc_string_तथ्य(scheme));

	वापस true;
}

अटल व्योम fetch_तथ्य_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "fetch_data_finalise called for %s",
	      lwc_string_तथ्य(scheme));
}

अटल bool fetch_तथ्य_can_fetch(स्थिर nsurl *url)
{
	वापस true;
}

अटल व्योम fetch_तथ्य_send_callback(स्थिर fetch_संदेश *संदेश,
		काष्ठा fetch_तथ्य_context *c)
{
	c->locked = true;
	fetch_send_callback(संदेश, c->parent_fetch);
	c->locked = false;
}

अटल व्योम fetch_तथ्य_send_header(काष्ठा fetch_तथ्य_context *ctx,
		स्थिर अक्षर *fmt, ...)
{
	अक्षर header[64];
	fetch_संदेश संदेश;
	बहु_सूची ap;
	पूर्णांक len;

	बहु_शुरू(ap, fmt);
	len = vsnprintf(header, माप(header), fmt, ap);
	बहु_पूर्ण(ap);

	अगर (len >= (पूर्णांक)माप(header) || len < 0) {
		वापस;
	}

	संदेश.type = FETCH_HEADER;
	संदेश.तथ्य.header_or_तथ्य.len = len;
	संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *)header;
	fetch_तथ्य_send_callback(&संदेश, ctx);
}

अटल व्योम *fetch_तथ्य_setup(काष्ठा fetch *parent_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, स्थिर अक्षर *post_urlenc,
		 स्थिर काष्ठा fetch_multipart_तथ्य *post_multipart,
		 स्थिर अक्षर **headers)
{
	काष्ठा fetch_तथ्य_context *ctx = सुस्मृति(1, माप(*ctx));
	
	अगर (ctx == शून्य)
		वापस शून्य;
		
	ctx->parent_fetch = parent_fetch;
	ctx->url = nsurl_ref(url);

	RING_जोड़ो(ring, ctx);
	
	वापस ctx;
}

अटल bool fetch_तथ्य_start(व्योम *ctx)
{
	वापस true;
}

अटल व्योम fetch_तथ्य_free(व्योम *ctx)
{
	काष्ठा fetch_तथ्य_context *c = ctx;

	nsurl_unref(c->url);
	मुक्त(c->तथ्य);
	मुक्त(c->mimetype);
	मुक्त(ctx);
}

अटल व्योम fetch_तथ्य_abort(व्योम *ctx)
{
	काष्ठा fetch_तथ्य_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here. 
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

अटल bool fetch_तथ्य_process(काष्ठा fetch_तथ्य_context *c)
{
	nserror res;
	fetch_संदेश संदेश;
	स्थिर अक्षर *params;
	स्थिर अक्षर *comma;
	अक्षर *unescaped;
	माप_प्रकार unescaped_len;
	
	/* format of a data: URL is:
	 *   data:[<mimetype>][;base64],<data>
	 * The mimetype is optional.  If it is missing, the , before the
	 * data must still be there.
	 */
	
	NSLOG(netsurf, DEEPDEBUG, "url: %.140s", nsurl_access(c->url));
	
	अगर (nsurl_लंबाई(c->url) < 6) {
		/* 6 is the minimum possible length (data:,) */
		संदेश.type = FETCH_ERROR;
		संदेश.तथ्य.error = "Malformed data: URL";
		fetch_तथ्य_send_callback(&संदेश, c);
		वापस false;
	}
	
	/* skip the data: part */
	params = nsurl_access(c->url) + SLEN("data:");
	
	/* find the comma */
	अगर ( (comma = म_अक्षर(params, ',')) == शून्य) {
		संदेश.type = FETCH_ERROR;
		संदेश.तथ्य.error = "Malformed data: URL";
		fetch_तथ्य_send_callback(&संदेश, c);
		वापस false;
	}
	
	अगर (params[0] == ',') {
		/* there is no mimetype here, assume text/plain */
		c->mimetype = strdup("text/plain;charset=US-ASCII");
	} अन्यथा {	
		/* make a copy of everything between data: and the comma */
		c->mimetype = strndup(params, comma - params);
	}
	
	अगर (c->mimetype == शून्य) {
		संदेश.type = FETCH_ERROR;
		संदेश.तथ्य.error = 
			"Unable to allocate memory for mimetype in data: URL";
		fetch_तथ्य_send_callback(&संदेश, c);
		वापस false;
	}
	
	अगर (म_भेद(c->mimetype + म_माप(c->mimetype) - 7, ";base64") == 0) {
		c->base64 = true;
		c->mimetype[म_माप(c->mimetype) - 7] = '\0';
	} अन्यथा {
		c->base64 = false;
	}
	
	/* URL unescape the data first, just incase some insane page
	 * decides to nest URL and base64 encoding.  Like, say, Acid2.
	 */
	res = url_unescape(comma + 1, 0, &unescaped_len, &unescaped);
	अगर (res != NSERROR_OK) {
		संदेश.type = FETCH_ERROR;
		संदेश.तथ्य.error = "Unable to URL decode data: URL";
		fetch_तथ्य_send_callback(&संदेश, c);
		वापस false;
	}
	
	अगर (c->base64) {
		अगर ((nsu_base64_decode_alloc((uint8_t *)unescaped,
					     unescaped_len,
					     (uint8_t **)&c->तथ्य,
					     &c->datalen) != NSUERROR_OK) ||
		    (c->तथ्य == शून्य)) {
			संदेश.type = FETCH_ERROR;
			संदेश.तथ्य.error = "Unable to Base64 decode data: URL";
			fetch_तथ्य_send_callback(&संदेश, c);
			मुक्त(unescaped);
			वापस false;
		}
		मुक्त(unescaped);
	} अन्यथा {
		c->datalen = unescaped_len;
		c->तथ्य = unescaped;
	}
	
	
	वापस true;
}

अटल व्योम fetch_तथ्य_poll(lwc_string *scheme)
{
	fetch_संदेश संदेश;
	काष्ठा fetch_तथ्य_context *c, *save_ring = शून्य;
	
	/* Iterate over ring, processing each pending fetch */
	जबतक (ring != शून्य) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called 
		 * again.
		 */
		अगर (c->locked == true) {
			RING_जोड़ो(save_ring, c);
			जारी;
		}

		/* Only process non-aborted fetches */
		अगर (c->aborted == false && fetch_तथ्य_process(c) == true) {
			fetch_set_http_code(c->parent_fetch, 200);
			NSLOG(netsurf, INFO,
			      "setting data: MIME type to %s, length to %"PRIsizet,
			      c->mimetype,
			      c->datalen);
			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to fetch_data_send_callback().
			 */
			fetch_तथ्य_send_header(c, "Content-Type: %s",
					c->mimetype);

			अगर (c->aborted == false) {
				fetch_तथ्य_send_header(c, "Content-Length: %"
						PRIsizet, c->datalen);
			}

			अगर (c->aborted == false) {
				/* Set max-age to 1 year. */
				fetch_तथ्य_send_header(c, "Cache-Control: "
						"max-age=31536000");
			}

			अगर (c->aborted == false) {
				संदेश.type = FETCH_तथ्य;
				संदेश.तथ्य.header_or_तथ्य.buf = 
						(स्थिर uint8_t *) c->तथ्य;
				संदेश.तथ्य.header_or_तथ्य.len = c->datalen;
				fetch_तथ्य_send_callback(&संदेश, c);
			}

			अगर (c->aborted == false) {
				संदेश.type = FETCH_FINISHED;
				fetch_तथ्य_send_callback(&संदेश, c);
			}
		} अन्यथा {
			NSLOG(netsurf, INFO, "Processing of %.140s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't, 
			 * then fetch_data_process() is broken.
			 */
			निश्चित(c->locked == false);
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

nserror fetch_तथ्य_register(व्योम)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_तथ्य);
	स्थिर काष्ठा fetcher_operation_table fetcher_ops = {
		.initialise = fetch_तथ्य_initialise,
		.acceptable = fetch_तथ्य_can_fetch,
		.setup = fetch_तथ्य_setup,
		.start = fetch_तथ्य_start,
		.पात = fetch_तथ्य_abort,
		.मुक्त = fetch_तथ्य_free,
		.poll = fetch_तथ्य_poll,
		.finalise = fetch_तथ्य_finalise
	};

	वापस fetcher_add(scheme, &fetcher_ops);
}
