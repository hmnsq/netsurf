/*
 * Copyright 2011 Vincent Sanders <vince@netsurf-browser.org>
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
 * resource scheme URL handling. Based on the data fetcher by Rob Kendrick
 */

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <libwapcaplet/libwapcaplet.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/time.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/ring.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/fetch.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"

#ଅନ୍ତର୍ଭାଵ "content/fetch.h"
#ଅନ୍ତର୍ଭାଵ "content/fetchers.h"
#ଅନ୍ତର୍ଭାଵ "content/fetchers/resource.h"

#define DIRECT_ETAG_ମୂଲ୍ୟ 123456

/** Valid resource paths */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *fetch_resource_paths[] = {
	"adblock.css",
	"default.css",
	"internal.css",
	"quirks.css",
	"user.css",
	"credits.html",
	"licence.html",
	"welcome.html",
	"favicon.ico",
	"default.ico",
	"netsurf.png",
	"icons/arrow-l.png",
	"icons/content.png",
	"icons/directory.png",
	"icons/directory2.png",
	"icons/hotlist-add.png",
	"icons/hotlist-rmv.png",
	"icons/search.png"
};

/**
 * map of resource scheme paths to redirect urls
 */
ସ୍ଥିର ସଂରଚନା fetch_resource_map_entry {
	lwc_string *path; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	ଅଚଳ uint8_t *data; /**< direct pointer to data */
	size_t data_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

ସଂରଚନା fetch_resource_context;

ପ୍ରକାରବ୍ୟାଖ୍ୟା bool (*fetch_resource_handler)(ସଂରଚନା fetch_resource_context *);

/** Context for an resource fetch */
ସଂରଚନା fetch_resource_context {
	ସଂରଚନା fetch_resource_context *r_next, *r_prev;

	ସଂରଚନା fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	ସଂରଚନା fetch_resource_map_entry *entry; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

ସ୍ଥିର ସଂରଚନା fetch_resource_context *ring = ଖାଲି;

ସ୍ଥିର uint32_t fetch_resource_path_ଗଣନା;

/** issue fetch callbacks with locking */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool fetch_resource_send_callback(ଅଚଳ fetch_ସନ୍ଦେଶ *ସନ୍ଦେଶ,
		ସଂରଚନା fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ସନ୍ଦେଶ, ctx->fetchh);
	ctx->locked = false;

	ଫେରନ୍ତୁ ctx->aborted;
}

ସ୍ଥିର bool fetch_resource_send_header(ସଂରଚନା fetch_resource_context *ctx,
		ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	ଅକ୍ଷର header[64];
	va_list ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;

	va_start(ap, fmt);
	len = vsnprintf(header, ଆକାର header, fmt, ap);
	va_end(ap);

	ଯଦି (len >= (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଆକାର(header) || len < 0) {
		ଫେରନ୍ତୁ false;
	}

	ସନ୍ଦେଶ.type = FETCH_HEADER;
	ସନ୍ଦେଶ.data.header_or_data.buf = (ଅଚଳ uint8_t *) header;
	ସନ୍ଦେଶ.data.header_or_data.len = len;

	ଫେରନ୍ତୁ fetch_resource_send_callback(&ସନ୍ଦେଶ, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
ସ୍ଥିର bool fetch_resource_redirect_handler(ସଂରଚନା fetch_resource_context *ctx)
{
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, 302);

	ସନ୍ଦେଶ.type = FETCH_REDIRECT;
	ସନ୍ଦେଶ.data.redirect = nsurl_access(ctx->entry->redirect_url);
	fetch_resource_send_callback(&ସନ୍ଦେଶ, ctx);

	ଫେରନ୍ତୁ true;
}

/* resource handler that returns data directly */
ସ୍ଥିର bool fetch_resource_data_handler(ସଂରଚନା fetch_resource_context *ctx)
{
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;

	/* Check if we can just return not modified */
	ଯଦି (ctx->etag != 0 && ctx->etag == DIRECT_ETAG_ମୂଲ୍ୟ) {
		fetch_set_http_code(ctx->fetchh, 304);
		ସନ୍ଦେଶ.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&ସନ୍ଦେଶ, ctx);
		ଫେରନ୍ତୁ true;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ଯଦି (fetch_resource_send_header(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_data(ctx->entry->path)))) {
		ଯାଅ fetch_resource_data_aborted;
	}

	/* content length */
	ଯଦି (fetch_resource_send_header(ctx, "Content-Length: %" PRIsizet,
				       ctx->entry->data_len)) {
		ଯାଅ fetch_resource_data_aborted;
	}

	/* create etag */
	ଯଦି (fetch_resource_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) DIRECT_ETAG_ମୂଲ୍ୟ)) {
		ଯାଅ fetch_resource_data_aborted;
	}

	/* create max-age of 1 year */
	ଯଦି (fetch_resource_send_header(ctx,
			"Cache-Control: max-age=31536000")) {
		ଯାଅ fetch_resource_data_aborted;
	}

	ସନ୍ଦେଶ.type = FETCH_DATA;
	ସନ୍ଦେଶ.data.header_or_data.buf = (ଅଚଳ uint8_t *) ctx->entry->data;
	ସନ୍ଦେଶ.data.header_or_data.len = ctx->entry->data_len;
	fetch_resource_send_callback(&ସନ୍ଦେଶ, ctx);

	ଯଦି (ctx->aborted == false) {
		ସନ୍ଦେଶ.type = FETCH_FINISHED;
		fetch_resource_send_callback(&ସନ୍ଦେଶ, ctx);
	}

fetch_resource_data_aborted:

	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର bool fetch_resource_notfound_handler(ସଂରଚନା fetch_resource_context *ctx)
{
	fetch_ସନ୍ଦେଶ ସନ୍ଦେଶ;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା code = 404;
	ଅକ୍ଷର buffer[1024];
	ଅଚଳ ଅକ୍ଷର *title;
	ଅକ୍ଷର ଚାବି[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	ଯଦି (fetch_resource_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		ଯାଅ fetch_resource_notfound_handler_aborted;

	snprintf(ଚାବି, ଆକାର ଚାବି, "HTTP%03d", code);
	title = messages_get(ଚାବି);

	snprintf(buffer, ଆକାର buffer,
		 "<html><head>"
		 "<title>%s</title>"
		 "<link rel=\"stylesheet\" type=\"text/css\" "
		 "href=\"resource:internal.css\">\n"
		 "</head>"
		 "<body class=\"ns-even-bg ns-even-fg ns-border\" "
		 "id =\"fetcherror\">\n"
		 "<h1 class=\"ns-border ns-odd-fg-bad\">%s</h1>\n"
		 "<p>%s %d %s %s</p>\n"
		 "</body>\n</html>\n",
		 title, title,
		 messages_get("FetchErrorCode"), code,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	ସନ୍ଦେଶ.type = FETCH_DATA;
	ସନ୍ଦେଶ.data.header_or_data.buf = (ଅଚଳ uint8_t *) buffer;
	ସନ୍ଦେଶ.data.header_or_data.len = strlen(buffer);
	ଯଦି (fetch_resource_send_callback(&ସନ୍ଦେଶ, ctx))
		ଯାଅ fetch_resource_notfound_handler_aborted;

	ସନ୍ଦେଶ.type = FETCH_FINISHED;
	fetch_resource_send_callback(&ସନ୍ଦେଶ, ctx);

fetch_resource_notfound_handler_aborted:
	ଫେରନ୍ତୁ false;
}



/** callback to initialise the resource fetcher. */
ସ୍ଥିର bool fetch_resource_initialise(lwc_string *scheme)
{
	ସଂରଚନା fetch_resource_map_entry *e;
	uint32_t i;
	nserror res;

	fetch_resource_path_ଗଣନା = 0;

	ପାଇଁ (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_path_ଗଣନା];

		ଯଦି (lwc_intern_string(fetch_resource_paths[i],
				strlen(fetch_resource_paths[i]),
				&e->path) != lwc_error_ok) {
			ଯେତେବେଳେ (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].path);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->data = ଖାଲି;
		res = guit->fetch->get_resource_data(lwc_string_data(e->path),
						     &e->data,
						     &e->data_len);
		ଯଦି (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_path_ଗଣନା++;
		} ଅନ୍ୟଥା {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			ଯଦି (e->redirect_url == ଖାଲି) {
				lwc_string_unref(e->path);
			} ଅନ୍ୟଥା {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_path_ଗଣନା++;
			}
		}
	}

	ଫେରନ୍ତୁ true;
}

/** callback to finalise the resource fetcher. */
ସ୍ଥିର ଶୂନ୍ୟ fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	ପାଇଁ (i = 0; i < fetch_resource_path_ଗଣନା; i++) {
		lwc_string_unref(fetch_resource_map[i].path);
		ଯଦି (fetch_resource_map[i].data != ଖାଲି) {
			guit->fetch->release_resource_data(fetch_resource_map[i].data);
		} ଅନ୍ୟଥା {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

ସ୍ଥିର bool fetch_resource_can_fetch(ଅଚଳ nsurl *url)
{
	ଫେରନ୍ତୁ true;
}

/**
 * set up a resource fetch context.
 */
ସ୍ଥିର ଶୂନ୍ୟ *
fetch_resource_setup(ସଂରଚନା fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     ଅଚଳ ଅକ୍ଷର *post_urlenc,
		     ଅଚଳ ସଂରଚନା fetch_multipart_data *post_multipart,
		     ଅଚଳ ଅକ୍ଷର **headers)
{
	ସଂରଚନା fetch_resource_context *ctx;
	lwc_string *path;
	nserror ret;
	uint32_t i;

	ctx = calloc(1, ଆକାର(*ctx));
	ଯଦି (ctx == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ctx->handler = fetch_resource_notfound_handler;

	ଯଦି ((path = nsurl_get_component(url, NSURL_PATH)) != ଖାଲି) {
		bool match;

		/* Ensure requested path is valid */
		ପାଇଁ (i = 0; i < fetch_resource_path_ଗଣନା; i++) {
			ଯଦି (lwc_string_isequal(path,
					fetch_resource_map[i].path,
					&match) == lwc_error_ok && match) {
				/* found a url match, select handler */
				ctx->entry = &fetch_resource_map[i];
				ଯଦି (ctx->entry->data != ଖାଲି) {
					ctx->handler = fetch_resource_data_handler;
				} ଅନ୍ୟଥା {
					ctx->handler = fetch_resource_redirect_handler;
				}
				ଭାଙ୍ଗ;
			}
		}

		lwc_string_unref(path);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	ପାଇଁ (i = 0; headers[i] != ଖାଲି; i++) {
		ଯଦି (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			ଜାରି;
		}

		/* If-None-Match: "12345678" */
		ଅଚଳ ଅକ୍ଷର *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		ଯେତେବେଳେ (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		ଯଦି (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->etag);
			ଯଦି (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_ଭର୍ତ୍ତି(ring, ctx);

	ଫେରନ୍ତୁ ctx;
}

/** callback to free a resource fetch */
ସ୍ଥିର ଶୂନ୍ୟ fetch_resource_free(ଶୂନ୍ୟ *ctx)
{
	ସଂରଚନା fetch_resource_context *c = ctx;
	ଯଦି (c->url != ଖାଲି)
		nsurl_unref(c->url);
	ମୁକ୍ତ(ctx);
}

/** callback to start a resource fetch */
ସ୍ଥିର bool fetch_resource_start(ଶୂନ୍ୟ *ctx)
{
	ଫେରନ୍ତୁ true;
}

/** callback to abort a resource fetch */
ସ୍ଥିର ଶୂନ୍ୟ fetch_resource_abort(ଶୂନ୍ୟ *ctx)
{
	ସଂରଚନା fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ସ୍ଥିର ଶୂନ୍ୟ fetch_resource_poll(lwc_string *scheme)
{
	ସଂରଚନା fetch_resource_context *c, *save_ring = ଖାଲି;

	ଯେତେବେଳେ (ring != ଖାଲି) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ଯଦି (c->locked == true) {
			RING_ଭର୍ତ୍ତି(save_ring, c);
			ଜାରି;
		}

		/* Only process non-aborted fetches */
		ଯଦି (c->aborted == false) {
			/* resource fetches can be processed in one go */
			c->handler(c);
		}

		/* And now finish */
		fetch_remove_from_queues(c->fetchh);
		fetch_free(c->fetchh);
	}

	/* Finally, if we saved any fetches which were locked, put them back
	 * into the ring for next time
	 */
	ring = save_ring;
}

nserror fetch_resource_register(ଶୂନ୍ୟ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	ଅଚଳ ସଂରଚନା fetcher_operation_table fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.setup = fetch_resource_setup,
		.start = fetch_resource_start,
		.abort = fetch_resource_abort,
		.ମୁକ୍ତ = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	ଫେରନ୍ତୁ fetcher_add(scheme, &fetcher_ops);
}
