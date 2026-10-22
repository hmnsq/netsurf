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

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/time.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/ring.h"
#ಅನ್ತರ್ಭಾವ "netsurf/fetch.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers/resource.h"

#define DIRECT_ETAG_ಮೌಲ್ಯ 123456

/** Valid resource paths */
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *fetch_resource_paths[] = {
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
ಸ್ಥಿರ ಸಂರಚನಾ fetch_resource_map_entry {
	lwc_string *path; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	ಧ್ರುವ uint8_t *data; /**< direct pointer to data */
	size_t data_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

ಸಂರಚನಾ fetch_resource_context;

ಪ್ರಕಾರನಿರ್ದೇಶ bool (*fetch_resource_handler)(ಸಂರಚನಾ fetch_resource_context *);

/** Context for an resource fetch */
ಸಂರಚನಾ fetch_resource_context {
	ಸಂರಚನಾ fetch_resource_context *r_next, *r_prev;

	ಸಂರಚನಾ fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	ಸಂರಚನಾ fetch_resource_map_entry *entry; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

ಸ್ಥಿರ ಸಂರಚನಾ fetch_resource_context *ring = ರಿಕ್ತ;

ಸ್ಥಿರ uint32_t fetch_resource_path_ಎಣಿಕೆ;

/** issue fetch callbacks with locking */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool fetch_resource_send_callback(ಧ್ರುವ fetch_ಸಂದೇಶ *ಸಂದೇಶ,
		ಸಂರಚನಾ fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ಸಂದೇಶ, ctx->fetchh);
	ctx->locked = false;

	ಪ್ರತ್ಯಾಗಮ ctx->aborted;
}

ಸ್ಥಿರ bool fetch_resource_send_header(ಸಂರಚನಾ fetch_resource_context *ctx,
		ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ವರ್ಣ header[64];
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ len;

	va_start(ap, fmt);
	len = vsnprintf(header, ಪರಿಮಾಣ header, fmt, ap);
	va_end(ap);

	ಯದಿ (len >= (ಪೂರ್ಣಾಙ್ಕ)ಪರಿಮಾಣ(header) || len < 0) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಸಂದೇಶ.type = FETCH_HEADER;
	ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *) header;
	ಸಂದೇಶ.data.header_or_data.len = len;

	ಪ್ರತ್ಯಾಗಮ fetch_resource_send_callback(&ಸಂದೇಶ, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
ಸ್ಥಿರ bool fetch_resource_redirect_handler(ಸಂರಚನಾ fetch_resource_context *ctx)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, 302);

	ಸಂದೇಶ.type = FETCH_REDIRECT;
	ಸಂದೇಶ.data.redirect = nsurl_access(ctx->entry->redirect_url);
	fetch_resource_send_callback(&ಸಂದೇಶ, ctx);

	ಪ್ರತ್ಯಾಗಮ true;
}

/* resource handler that returns data directly */
ಸ್ಥಿರ bool fetch_resource_data_handler(ಸಂರಚನಾ fetch_resource_context *ctx)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	/* Check if we can just return not modified */
	ಯದಿ (ctx->etag != 0 && ctx->etag == DIRECT_ETAG_ಮೌಲ್ಯ) {
		fetch_set_http_code(ctx->fetchh, 304);
		ಸಂದೇಶ.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&ಸಂದೇಶ, ctx);
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ಯದಿ (fetch_resource_send_header(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_data(ctx->entry->path)))) {
		ಗಚ್ಛ fetch_resource_data_aborted;
	}

	/* content length */
	ಯದಿ (fetch_resource_send_header(ctx, "Content-Length: %" PRIsizet,
				       ctx->entry->data_len)) {
		ಗಚ್ಛ fetch_resource_data_aborted;
	}

	/* create etag */
	ಯದಿ (fetch_resource_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) DIRECT_ETAG_ಮೌಲ್ಯ)) {
		ಗಚ್ಛ fetch_resource_data_aborted;
	}

	/* create max-age of 1 year */
	ಯದಿ (fetch_resource_send_header(ctx,
			"Cache-Control: max-age=31536000")) {
		ಗಚ್ಛ fetch_resource_data_aborted;
	}

	ಸಂದೇಶ.type = FETCH_DATA;
	ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *) ctx->entry->data;
	ಸಂದೇಶ.data.header_or_data.len = ctx->entry->data_len;
	fetch_resource_send_callback(&ಸಂದೇಶ, ctx);

	ಯದಿ (ctx->aborted == false) {
		ಸಂದೇಶ.type = FETCH_FINISHED;
		fetch_resource_send_callback(&ಸಂದೇಶ, ctx);
	}

fetch_resource_data_aborted:

	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ bool fetch_resource_notfound_handler(ಸಂರಚನಾ fetch_resource_context *ctx)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಪೂರ್ಣಾಙ್ಕ code = 404;
	ವರ್ಣ buffer[1024];
	ಧ್ರುವ ವರ್ಣ *title;
	ವರ್ಣ ಕೀಲಿ[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	ಯದಿ (fetch_resource_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		ಗಚ್ಛ fetch_resource_notfound_handler_aborted;

	snprintf(ಕೀಲಿ, ಪರಿಮಾಣ ಕೀಲಿ, "HTTP%03d", code);
	title = messages_get(ಕೀಲಿ);

	snprintf(buffer, ಪರಿಮಾಣ buffer,
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

	ಸಂದೇಶ.type = FETCH_DATA;
	ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *) buffer;
	ಸಂದೇಶ.data.header_or_data.len = strlen(buffer);
	ಯದಿ (fetch_resource_send_callback(&ಸಂದೇಶ, ctx))
		ಗಚ್ಛ fetch_resource_notfound_handler_aborted;

	ಸಂದೇಶ.type = FETCH_FINISHED;
	fetch_resource_send_callback(&ಸಂದೇಶ, ctx);

fetch_resource_notfound_handler_aborted:
	ಪ್ರತ್ಯಾಗಮ false;
}



/** callback to initialise the resource fetcher. */
ಸ್ಥಿರ bool fetch_resource_initialise(lwc_string *scheme)
{
	ಸಂರಚನಾ fetch_resource_map_entry *e;
	uint32_t i;
	nserror res;

	fetch_resource_path_ಎಣಿಕೆ = 0;

	ಪ್ರತಿ (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_path_ಎಣಿಕೆ];

		ಯದಿ (lwc_intern_string(fetch_resource_paths[i],
				strlen(fetch_resource_paths[i]),
				&e->path) != lwc_error_ok) {
			ಯಾವತ್ (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].path);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->data = ರಿಕ್ತ;
		res = guit->fetch->get_resource_data(lwc_string_data(e->path),
						     &e->data,
						     &e->data_len);
		ಯದಿ (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_path_ಎಣಿಕೆ++;
		} ಅನ್ಯಥಾ {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			ಯದಿ (e->redirect_url == ರಿಕ್ತ) {
				lwc_string_unref(e->path);
			} ಅನ್ಯಥಾ {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_path_ಎಣಿಕೆ++;
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

/** callback to finalise the resource fetcher. */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	ಪ್ರತಿ (i = 0; i < fetch_resource_path_ಎಣಿಕೆ; i++) {
		lwc_string_unref(fetch_resource_map[i].path);
		ಯದಿ (fetch_resource_map[i].data != ರಿಕ್ತ) {
			guit->fetch->release_resource_data(fetch_resource_map[i].data);
		} ಅನ್ಯಥಾ {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

ಸ್ಥಿರ bool fetch_resource_can_fetch(ಧ್ರುವ nsurl *url)
{
	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * set up a resource fetch context.
 */
ಸ್ಥಿರ ಶೂನ್ಯ *
fetch_resource_setup(ಸಂರಚನಾ fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     ಧ್ರುವ ವರ್ಣ *post_urlenc,
		     ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *post_multipart,
		     ಧ್ರುವ ವರ್ಣ **headers)
{
	ಸಂರಚನಾ fetch_resource_context *ctx;
	lwc_string *path;
	nserror ret;
	uint32_t i;

	ctx = calloc(1, ಪರಿಮಾಣ(*ctx));
	ಯದಿ (ctx == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ctx->handler = fetch_resource_notfound_handler;

	ಯದಿ ((path = nsurl_get_component(url, NSURL_PATH)) != ರಿಕ್ತ) {
		bool match;

		/* Ensure requested path is valid */
		ಪ್ರತಿ (i = 0; i < fetch_resource_path_ಎಣಿಕೆ; i++) {
			ಯದಿ (lwc_string_isequal(path,
					fetch_resource_map[i].path,
					&match) == lwc_error_ok && match) {
				/* found a url match, select handler */
				ctx->entry = &fetch_resource_map[i];
				ಯದಿ (ctx->entry->data != ರಿಕ್ತ) {
					ctx->handler = fetch_resource_data_handler;
				} ಅನ್ಯಥಾ {
					ctx->handler = fetch_resource_redirect_handler;
				}
				ಭಙ್ಗ;
			}
		}

		lwc_string_unref(path);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	ಪ್ರತಿ (i = 0; headers[i] != ರಿಕ್ತ; i++) {
		ಯದಿ (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			ಅನುವರ್ತ;
		}

		/* If-None-Match: "12345678" */
		ಧ್ರುವ ವರ್ಣ *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		ಯಾವತ್ (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		ಯದಿ (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->etag);
			ಯದಿ (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_ಸೇರಿಸು(ring, ctx);

	ಪ್ರತ್ಯಾಗಮ ctx;
}

/** callback to free a resource fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_resource_free(ಶೂನ್ಯ *ctx)
{
	ಸಂರಚನಾ fetch_resource_context *c = ctx;
	ಯದಿ (c->url != ರಿಕ್ತ)
		nsurl_unref(c->url);
	ಮುಕ್ತ(ctx);
}

/** callback to start a resource fetch */
ಸ್ಥಿರ bool fetch_resource_start(ಶೂನ್ಯ *ctx)
{
	ಪ್ರತ್ಯಾಗಮ true;
}

/** callback to abort a resource fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_resource_abort(ಶೂನ್ಯ *ctx)
{
	ಸಂರಚನಾ fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_resource_poll(lwc_string *scheme)
{
	ಸಂರಚನಾ fetch_resource_context *c, *save_ring = ರಿಕ್ತ;

	ಯಾವತ್ (ring != ರಿಕ್ತ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ಯದಿ (c->locked == true) {
			RING_ಸೇರಿಸು(save_ring, c);
			ಅನುವರ್ತ;
		}

		/* Only process non-aborted fetches */
		ಯದಿ (c->aborted == false) {
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

nserror fetch_resource_register(ಶೂನ್ಯ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	ಧ್ರುವ ಸಂರಚನಾ fetcher_operation_table fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.setup = fetch_resource_setup,
		.start = fetch_resource_start,
		.abort = fetch_resource_abort,
		.ಮುಕ್ತ = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	ಪ್ರತ್ಯಾಗಮ fetcher_add(scheme, &fetcher_ops);
}
