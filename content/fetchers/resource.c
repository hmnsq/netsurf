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

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <libwapcaplet/libwapcaplet.h>

#அந்தர்பாவ "netsurf/inttypes.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/time.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/ring.h"
#அந்தர்பாவ "netsurf/fetch.h"
#அந்தர்பாவ "desktop/gui_internal.h"

#அந்தர்பாவ "content/fetch.h"
#அந்தர்பாவ "content/fetchers.h"
#அந்தர்பாவ "content/fetchers/resource.h"

#define DIRECT_ETAG_மதிப்பு 123456

/** Valid resource paths */
நிலையான மாறிலி எழுத்து *fetch_resource_paths[] = {
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
நிலையான கட்டமைப்பு fetch_resource_map_entry {
	lwc_string *path; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	மாறிலி uint8_t *data; /**< direct pointer to data */
	size_t data_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

கட்டமைப்பு fetch_resource_context;

வகைவரை bool (*fetch_resource_handler)(கட்டமைப்பு fetch_resource_context *);

/** Context for an resource fetch */
கட்டமைப்பு fetch_resource_context {
	கட்டமைப்பு fetch_resource_context *r_next, *r_prev;

	கட்டமைப்பு fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	கட்டமைப்பு fetch_resource_map_entry *entry; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

நிலையான கட்டமைப்பு fetch_resource_context *ring = வெற்று;

நிலையான uint32_t fetch_resource_path_மொத்தம்;

/** issue fetch callbacks with locking */
நிலையான உள்வரி bool fetch_resource_send_callback(மாறிலி fetch_செய்தி *செய்தி,
		கட்டமைப்பு fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(செய்தி, ctx->fetchh);
	ctx->locked = false;

	திரும்பு ctx->aborted;
}

நிலையான bool fetch_resource_send_header(கட்டமைப்பு fetch_resource_context *ctx,
		மாறிலி எழுத்து *fmt, ...)
{
	fetch_செய்தி செய்தி;
	எழுத்து header[64];
	va_list ap;
	முழு len;

	va_start(ap, fmt);
	len = vsnprintf(header, அளவு header, fmt, ap);
	va_end(ap);

	எனில் (len >= (முழு)அளவு(header) || len < 0) {
		திரும்பு false;
	}

	செய்தி.type = FETCH_HEADER;
	செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *) header;
	செய்தி.data.header_or_data.len = len;

	திரும்பு fetch_resource_send_callback(&செய்தி, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
நிலையான bool fetch_resource_redirect_handler(கட்டமைப்பு fetch_resource_context *ctx)
{
	fetch_செய்தி செய்தி;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, 302);

	செய்தி.type = FETCH_REDIRECT;
	செய்தி.data.redirect = nsurl_access(ctx->entry->redirect_url);
	fetch_resource_send_callback(&செய்தி, ctx);

	திரும்பு true;
}

/* resource handler that returns data directly */
நிலையான bool fetch_resource_data_handler(கட்டமைப்பு fetch_resource_context *ctx)
{
	fetch_செய்தி செய்தி;

	/* Check if we can just return not modified */
	எனில் (ctx->etag != 0 && ctx->etag == DIRECT_ETAG_மதிப்பு) {
		fetch_set_http_code(ctx->fetchh, 304);
		செய்தி.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&செய்தி, ctx);
		திரும்பு true;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	எனில் (fetch_resource_send_header(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_data(ctx->entry->path)))) {
		செல் fetch_resource_data_aborted;
	}

	/* content length */
	எனில் (fetch_resource_send_header(ctx, "Content-Length: %" PRIsizet,
				       ctx->entry->data_len)) {
		செல் fetch_resource_data_aborted;
	}

	/* create etag */
	எனில் (fetch_resource_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) DIRECT_ETAG_மதிப்பு)) {
		செல் fetch_resource_data_aborted;
	}

	/* create max-age of 1 year */
	எனில் (fetch_resource_send_header(ctx,
			"Cache-Control: max-age=31536000")) {
		செல் fetch_resource_data_aborted;
	}

	செய்தி.type = FETCH_DATA;
	செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *) ctx->entry->data;
	செய்தி.data.header_or_data.len = ctx->entry->data_len;
	fetch_resource_send_callback(&செய்தி, ctx);

	எனில் (ctx->aborted == false) {
		செய்தி.type = FETCH_FINISHED;
		fetch_resource_send_callback(&செய்தி, ctx);
	}

fetch_resource_data_aborted:

	திரும்பு true;
}

நிலையான bool fetch_resource_notfound_handler(கட்டமைப்பு fetch_resource_context *ctx)
{
	fetch_செய்தி செய்தி;
	முழு code = 404;
	எழுத்து buffer[1024];
	மாறிலி எழுத்து *title;
	எழுத்து சாவி[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	எனில் (fetch_resource_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		செல் fetch_resource_notfound_handler_aborted;

	snprintf(சாவி, அளவு சாவி, "HTTP%03d", code);
	title = messages_get(சாவி);

	snprintf(buffer, அளவு buffer,
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

	செய்தி.type = FETCH_DATA;
	செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *) buffer;
	செய்தி.data.header_or_data.len = strlen(buffer);
	எனில் (fetch_resource_send_callback(&செய்தி, ctx))
		செல் fetch_resource_notfound_handler_aborted;

	செய்தி.type = FETCH_FINISHED;
	fetch_resource_send_callback(&செய்தி, ctx);

fetch_resource_notfound_handler_aborted:
	திரும்பு false;
}



/** callback to initialise the resource fetcher. */
நிலையான bool fetch_resource_initialise(lwc_string *scheme)
{
	கட்டமைப்பு fetch_resource_map_entry *e;
	uint32_t i;
	nserror res;

	fetch_resource_path_மொத்தம் = 0;

	க்கு (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_path_மொத்தம்];

		எனில் (lwc_intern_string(fetch_resource_paths[i],
				strlen(fetch_resource_paths[i]),
				&e->path) != lwc_error_ok) {
			போது (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].path);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->data = வெற்று;
		res = guit->fetch->get_resource_data(lwc_string_data(e->path),
						     &e->data,
						     &e->data_len);
		எனில் (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_path_மொத்தம்++;
		} இல்லையெனில் {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			எனில் (e->redirect_url == வெற்று) {
				lwc_string_unref(e->path);
			} இல்லையெனில் {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_path_மொத்தம்++;
			}
		}
	}

	திரும்பு true;
}

/** callback to finalise the resource fetcher. */
நிலையான வெறுமை fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	க்கு (i = 0; i < fetch_resource_path_மொத்தம்; i++) {
		lwc_string_unref(fetch_resource_map[i].path);
		எனில் (fetch_resource_map[i].data != வெற்று) {
			guit->fetch->release_resource_data(fetch_resource_map[i].data);
		} இல்லையெனில் {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

நிலையான bool fetch_resource_can_fetch(மாறிலி nsurl *url)
{
	திரும்பு true;
}

/**
 * set up a resource fetch context.
 */
நிலையான வெறுமை *
fetch_resource_setup(கட்டமைப்பு fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     மாறிலி எழுத்து *post_urlenc,
		     மாறிலி கட்டமைப்பு fetch_multipart_data *post_multipart,
		     மாறிலி எழுத்து **headers)
{
	கட்டமைப்பு fetch_resource_context *ctx;
	lwc_string *path;
	nserror ret;
	uint32_t i;

	ctx = calloc(1, அளவு(*ctx));
	எனில் (ctx == வெற்று) {
		திரும்பு வெற்று;
	}

	ctx->handler = fetch_resource_notfound_handler;

	எனில் ((path = nsurl_get_component(url, NSURL_PATH)) != வெற்று) {
		bool match;

		/* Ensure requested path is valid */
		க்கு (i = 0; i < fetch_resource_path_மொத்தம்; i++) {
			எனில் (lwc_string_isequal(path,
					fetch_resource_map[i].path,
					&match) == lwc_error_ok && match) {
				/* found a url match, select handler */
				ctx->entry = &fetch_resource_map[i];
				எனில் (ctx->entry->data != வெற்று) {
					ctx->handler = fetch_resource_data_handler;
				} இல்லையெனில் {
					ctx->handler = fetch_resource_redirect_handler;
				}
				நிறுத்து;
			}
		}

		lwc_string_unref(path);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	க்கு (i = 0; headers[i] != வெற்று; i++) {
		எனில் (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			தொடர்;
		}

		/* If-None-Match: "12345678" */
		மாறிலி எழுத்து *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		போது (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		எனில் (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->etag);
			எனில் (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_செருகு(ring, ctx);

	திரும்பு ctx;
}

/** callback to free a resource fetch */
நிலையான வெறுமை fetch_resource_free(வெறுமை *ctx)
{
	கட்டமைப்பு fetch_resource_context *c = ctx;
	எனில் (c->url != வெற்று)
		nsurl_unref(c->url);
	விடுவி(ctx);
}

/** callback to start a resource fetch */
நிலையான bool fetch_resource_start(வெறுமை *ctx)
{
	திரும்பு true;
}

/** callback to abort a resource fetch */
நிலையான வெறுமை fetch_resource_abort(வெறுமை *ctx)
{
	கட்டமைப்பு fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
நிலையான வெறுமை fetch_resource_poll(lwc_string *scheme)
{
	கட்டமைப்பு fetch_resource_context *c, *save_ring = வெற்று;

	போது (ring != வெற்று) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		எனில் (c->locked == true) {
			RING_செருகு(save_ring, c);
			தொடர்;
		}

		/* Only process non-aborted fetches */
		எனில் (c->aborted == false) {
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

nserror fetch_resource_register(வெறுமை)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	மாறிலி கட்டமைப்பு fetcher_operation_table fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.setup = fetch_resource_setup,
		.start = fetch_resource_start,
		.abort = fetch_resource_abort,
		.விடுவி = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	திரும்பு fetcher_add(scheme, &fetcher_ops);
}
