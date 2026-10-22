/*
 * Copyright 2012 Vincent Sanders <vince@kyllikki.org>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
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

/** \file
 * implementation for javascript scheme fetcher
 *
 * This fetcher implements http://www.whatwg.org/specs/web-apps/current-work/multipage/browsers.html#javascript-protocol
 */

#समावेश <stdbool.h>
#समावेश <stdlib.h>
#समावेश <libwapcaplet/libwapcaplet.h>

#समावेश "utils/nsurl.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/ring.h"
#समावेश "content/fetch.h"
#समावेश "content/fetchers.h"

#समावेश "javascript/fetcher.h"

/** Context for an resource fetch */
काष्ठा fetch_javascript_context {
	काष्ठा fetch_javascript_context *r_अग्र, *r_पूर्व;

	काष्ठा fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The URL being fetched */
};

अटल काष्ठा fetch_javascript_context *ring = शून्य;


/** issue fetch callbacks with locking */
अटल अंतरभूत bool fetch_javascript_send_callback(स्थिर fetch_संदेश *संदेश,
		काष्ठा fetch_javascript_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(संदेश, ctx->fetchh);
	ctx->locked = false;

	वापस ctx->aborted;
}


/**
 * called from poll to progress fetch.
 *
 * \todo This is currently completely unimplemented and just returns 204
 */
अटल bool fetch_javascript_handler(काष्ठा fetch_javascript_context *ctx)
{
	fetch_संदेश संदेश;
	पूर्णांक code = 204;

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	संदेश.type = FETCH_FINISHED;
	fetch_javascript_send_callback(&संदेश, ctx);

	वापस true;
}



/** callback to initialise the resource fetcher. */
अटल bool fetch_javascript_initialise(lwc_string *scheme)
{
	वापस true;
}

/** callback to finalise the resource fetcher. */
अटल व्योम fetch_javascript_finalise(lwc_string *scheme)
{
}

अटल bool fetch_javascript_can_fetch(स्थिर nsurl *url)
{
	वापस true;
}

/** callback to set up a resource fetch context. */
अटल व्योम *
fetch_javascript_setup(काष्ठा fetch *fetchh,
		 nsurl *url,
		 bool only_2xx,
		 bool downgrade_tls,
		 स्थिर अक्षर *post_urlenc,
		 स्थिर काष्ठा fetch_multipart_तथ्य *post_multipart,
		 स्थिर अक्षर **headers)
{
	काष्ठा fetch_javascript_context *ctx;

	ctx = सुस्मृति(1, माप(*ctx));
	अगर (ctx == शून्य)
		वापस शून्य;

	ctx->url = nsurl_ref(url);

	ctx->fetchh = fetchh;

	RING_जोड़ो(ring, ctx);

	वापस ctx;
}

/** callback to free a resource fetch */
अटल व्योम fetch_javascript_free(व्योम *ctx)
{
	काष्ठा fetch_javascript_context *c = ctx;
	अगर (c->url != शून्य) {
		nsurl_unref(c->url);
	}
	RING_REMOVE(ring, c);
	मुक्त(ctx);
}

/** callback to start a resource fetch */
अटल bool fetch_javascript_start(व्योम *ctx)
{
	वापस true;
}

/** callback to abort a resource fetch */
अटल व्योम fetch_javascript_abort(व्योम *ctx)
{
	काष्ठा fetch_javascript_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
अटल व्योम fetch_javascript_poll(lwc_string *scheme)
{
	काष्ठा fetch_javascript_context *c, *अग्र;

	अगर (ring == शून्य) वापस;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	करो {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		अगर (c->locked == true) {
			अग्र = c->r_अग्र;
			जारी;
		}

		/* Only process non-aborted fetches */
		अगर (c->aborted == false) {
			/* resource fetches can be processed in one go */
			fetch_javascript_handler(c);
		}

		/* Compute next fetch item at the last possible moment
		 * as processing this item may have added to the ring
		 */
		अग्र = c->r_अग्र;

		fetch_remove_from_queues(c->fetchh);
		fetch_free(c->fetchh);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} जबतक ( (c = अग्र) != ring && ring != शून्य);
}

/**
 * Register javascript scheme fetcher with fetcher factory.
 *
 * \return NSERROR_OK on success or appropriate error code on faliure.
*/
nserror fetch_javascript_register(व्योम)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_javascript);
	स्थिर काष्ठा fetcher_operation_table fetcher_ops = {
		.initialise = fetch_javascript_initialise,
		.acceptable = fetch_javascript_can_fetch,
		.setup = fetch_javascript_setup,
		.start = fetch_javascript_start,
		.पात = fetch_javascript_abort,
		.मुक्त = fetch_javascript_free,
		.poll = fetch_javascript_poll,
		.finalise = fetch_javascript_finalise
	};

	वापस fetcher_add(scheme, &fetcher_ops);
}
