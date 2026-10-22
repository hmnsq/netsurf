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

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <libwapcaplet/libwapcaplet.h>

#અન્તર્ભાવ "netsurf/inttypes.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/time.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/ring.h"
#અન્તર્ભાવ "netsurf/fetch.h"
#અન્તર્ભાવ "desktop/gui_internal.h"

#અન્તર્ભાવ "content/fetch.h"
#અન્તર્ભાવ "content/fetchers.h"
#અન્તર્ભાવ "content/fetchers/resource.h"

#define DIRECT_ETAG_કિંમત 123456

/** Valid resource paths */
સ્થિર અચલ અક્ષર *fetch_resource_paths[] = {
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
સ્થિર સંરચના fetch_resource_map_entry {
	lwc_string *path; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	અચલ uint8_t *data; /**< direct pointer to data */
	size_t data_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

સંરચના fetch_resource_context;

પ્રકારવ્યાખ્યા bool (*fetch_resource_handler)(સંરચના fetch_resource_context *);

/** Context for an resource fetch */
સંરચના fetch_resource_context {
	સંરચના fetch_resource_context *r_next, *r_prev;

	સંરચના fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	સંરચના fetch_resource_map_entry *entry; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

સ્થિર સંરચના fetch_resource_context *ring = ખાલી;

સ્થિર uint32_t fetch_resource_path_ગણતરી;

/** issue fetch callbacks with locking */
સ્થિર અંતઃરેખ bool fetch_resource_send_callback(અચલ fetch_સંદેશ *સંદેશ,
		સંરચના fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(સંદેશ, ctx->fetchh);
	ctx->locked = false;

	પાછા ctx->aborted;
}

સ્થિર bool fetch_resource_send_header(સંરચના fetch_resource_context *ctx,
		અચલ અક્ષર *fmt, ...)
{
	fetch_સંદેશ સંદેશ;
	અક્ષર header[64];
	va_list ap;
	પૂર્ણાંક len;

	va_start(ap, fmt);
	len = vsnprintf(header, કદ header, fmt, ap);
	va_end(ap);

	જો (len >= (પૂર્ણાંક)કદ(header) || len < 0) {
		પાછા false;
	}

	સંદેશ.type = FETCH_HEADER;
	સંદેશ.data.header_or_data.buf = (અચલ uint8_t *) header;
	સંદેશ.data.header_or_data.len = len;

	પાછા fetch_resource_send_callback(&સંદેશ, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
સ્થિર bool fetch_resource_redirect_handler(સંરચના fetch_resource_context *ctx)
{
	fetch_સંદેશ સંદેશ;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, 302);

	સંદેશ.type = FETCH_REDIRECT;
	સંદેશ.data.redirect = nsurl_access(ctx->entry->redirect_url);
	fetch_resource_send_callback(&સંદેશ, ctx);

	પાછા true;
}

/* resource handler that returns data directly */
સ્થિર bool fetch_resource_data_handler(સંરચના fetch_resource_context *ctx)
{
	fetch_સંદેશ સંદેશ;

	/* Check if we can just return not modified */
	જો (ctx->etag != 0 && ctx->etag == DIRECT_ETAG_કિંમત) {
		fetch_set_http_code(ctx->fetchh, 304);
		સંદેશ.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&સંદેશ, ctx);
		પાછા true;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	જો (fetch_resource_send_header(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_data(ctx->entry->path)))) {
		જાઓ fetch_resource_data_aborted;
	}

	/* content length */
	જો (fetch_resource_send_header(ctx, "Content-Length: %" PRIsizet,
				       ctx->entry->data_len)) {
		જાઓ fetch_resource_data_aborted;
	}

	/* create etag */
	જો (fetch_resource_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) DIRECT_ETAG_કિંમત)) {
		જાઓ fetch_resource_data_aborted;
	}

	/* create max-age of 1 year */
	જો (fetch_resource_send_header(ctx,
			"Cache-Control: max-age=31536000")) {
		જાઓ fetch_resource_data_aborted;
	}

	સંદેશ.type = FETCH_DATA;
	સંદેશ.data.header_or_data.buf = (અચલ uint8_t *) ctx->entry->data;
	સંદેશ.data.header_or_data.len = ctx->entry->data_len;
	fetch_resource_send_callback(&સંદેશ, ctx);

	જો (ctx->aborted == false) {
		સંદેશ.type = FETCH_FINISHED;
		fetch_resource_send_callback(&સંદેશ, ctx);
	}

fetch_resource_data_aborted:

	પાછા true;
}

સ્થિર bool fetch_resource_notfound_handler(સંરચના fetch_resource_context *ctx)
{
	fetch_સંદેશ સંદેશ;
	પૂર્ણાંક code = 404;
	અક્ષર buffer[1024];
	અચલ અક્ષર *title;
	અક્ષર ચાવી[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	જો (fetch_resource_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		જાઓ fetch_resource_notfound_handler_aborted;

	snprintf(ચાવી, કદ ચાવી, "HTTP%03d", code);
	title = messages_get(ચાવી);

	snprintf(buffer, કદ buffer,
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

	સંદેશ.type = FETCH_DATA;
	સંદેશ.data.header_or_data.buf = (અચલ uint8_t *) buffer;
	સંદેશ.data.header_or_data.len = strlen(buffer);
	જો (fetch_resource_send_callback(&સંદેશ, ctx))
		જાઓ fetch_resource_notfound_handler_aborted;

	સંદેશ.type = FETCH_FINISHED;
	fetch_resource_send_callback(&સંદેશ, ctx);

fetch_resource_notfound_handler_aborted:
	પાછા false;
}



/** callback to initialise the resource fetcher. */
સ્થિર bool fetch_resource_initialise(lwc_string *scheme)
{
	સંરચના fetch_resource_map_entry *e;
	uint32_t i;
	nserror res;

	fetch_resource_path_ગણતરી = 0;

	માટે (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_path_ગણતરી];

		જો (lwc_intern_string(fetch_resource_paths[i],
				strlen(fetch_resource_paths[i]),
				&e->path) != lwc_error_ok) {
			જ્યારે (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].path);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->data = ખાલી;
		res = guit->fetch->get_resource_data(lwc_string_data(e->path),
						     &e->data,
						     &e->data_len);
		જો (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_path_ગણતરી++;
		} નહીંતર {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			જો (e->redirect_url == ખાલી) {
				lwc_string_unref(e->path);
			} નહીંતર {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_path_ગણતરી++;
			}
		}
	}

	પાછા true;
}

/** callback to finalise the resource fetcher. */
સ્થિર શૂન્ય fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	માટે (i = 0; i < fetch_resource_path_ગણતરી; i++) {
		lwc_string_unref(fetch_resource_map[i].path);
		જો (fetch_resource_map[i].data != ખાલી) {
			guit->fetch->release_resource_data(fetch_resource_map[i].data);
		} નહીંતર {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

સ્થિર bool fetch_resource_can_fetch(અચલ nsurl *url)
{
	પાછા true;
}

/**
 * set up a resource fetch context.
 */
સ્થિર શૂન્ય *
fetch_resource_setup(સંરચના fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     અચલ અક્ષર *post_urlenc,
		     અચલ સંરચના fetch_multipart_data *post_multipart,
		     અચલ અક્ષર **headers)
{
	સંરચના fetch_resource_context *ctx;
	lwc_string *path;
	nserror ret;
	uint32_t i;

	ctx = calloc(1, કદ(*ctx));
	જો (ctx == ખાલી) {
		પાછા ખાલી;
	}

	ctx->handler = fetch_resource_notfound_handler;

	જો ((path = nsurl_get_component(url, NSURL_PATH)) != ખાલી) {
		bool match;

		/* Ensure requested path is valid */
		માટે (i = 0; i < fetch_resource_path_ગણતરી; i++) {
			જો (lwc_string_isequal(path,
					fetch_resource_map[i].path,
					&match) == lwc_error_ok && match) {
				/* found a url match, select handler */
				ctx->entry = &fetch_resource_map[i];
				જો (ctx->entry->data != ખાલી) {
					ctx->handler = fetch_resource_data_handler;
				} નહીંતર {
					ctx->handler = fetch_resource_redirect_handler;
				}
				તોડો;
			}
		}

		lwc_string_unref(path);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	માટે (i = 0; headers[i] != ખાલી; i++) {
		જો (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			ચાલુ;
		}

		/* If-None-Match: "12345678" */
		અચલ અક્ષર *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		જ્યારે (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		જો (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->etag);
			જો (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_ઉમેરો(ring, ctx);

	પાછા ctx;
}

/** callback to free a resource fetch */
સ્થિર શૂન્ય fetch_resource_free(શૂન્ય *ctx)
{
	સંરચના fetch_resource_context *c = ctx;
	જો (c->url != ખાલી)
		nsurl_unref(c->url);
	મુક્ત(ctx);
}

/** callback to start a resource fetch */
સ્થિર bool fetch_resource_start(શૂન્ય *ctx)
{
	પાછા true;
}

/** callback to abort a resource fetch */
સ્થિર શૂન્ય fetch_resource_abort(શૂન્ય *ctx)
{
	સંરચના fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
સ્થિર શૂન્ય fetch_resource_poll(lwc_string *scheme)
{
	સંરચના fetch_resource_context *c, *save_ring = ખાલી;

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
		જો (c->aborted == false) {
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

nserror fetch_resource_register(શૂન્ય)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	અચલ સંરચના fetcher_operation_table fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.setup = fetch_resource_setup,
		.start = fetch_resource_start,
		.abort = fetch_resource_abort,
		.મુક્ત = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	પાછા fetcher_add(scheme, &fetcher_ops);
}
