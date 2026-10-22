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

#شامل <stdlib.h>
#شامل <stdbool.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <stdio.h>
#شامل <stdarg.h>
#شامل <libwapcaplet/libwapcaplet.h>

#شامل "netsurf/inttypes.h"
#شامل "utils/nsurl.h"
#شامل "utils/corestrings.h"
#شامل "utils/log.h"
#شامل "utils/time.h"
#شامل "utils/messages.h"
#شامل "utils/utils.h"
#شامل "utils/ring.h"
#شامل "netsurf/fetch.h"
#شامل "desktop/gui_internal.h"

#شامل "content/fetch.h"
#شامل "content/fetchers.h"
#شامل "content/fetchers/resource.h"

#define DIRECT_ETAG_مقدار 123456

/** Valid resource paths */
ایستا ثابت نویسه *fetch_resource_paths[] = {
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
ایستا ساختار fetch_resource_map_entry {
	lwc_string *path; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	ثابت uint8_t *data; /**< direct pointer to data */
	size_t data_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

ساختار fetch_resource_context;

تعریفنوع bool (*fetch_resource_handler)(ساختار fetch_resource_context *);

/** Context for an resource fetch */
ساختار fetch_resource_context {
	ساختار fetch_resource_context *r_next, *r_prev;

	ساختار fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	ساختار fetch_resource_map_entry *entry; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

ایستا ساختار fetch_resource_context *ring = پوچ;

ایستا uint32_t fetch_resource_path_شمار;

/** issue fetch callbacks with locking */
ایستا درونخطی bool fetch_resource_send_callback(ثابت fetch_پیام *پیام,
		ساختار fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پیام, ctx->fetchh);
	ctx->locked = false;

	بازگشت ctx->aborted;
}

ایستا bool fetch_resource_send_header(ساختار fetch_resource_context *ctx,
		ثابت نویسه *fmt, ...)
{
	fetch_پیام پیام;
	نویسه header[64];
	va_list ap;
	صحیح len;

	va_start(ap, fmt);
	len = vsnprintf(header, اندازه header, fmt, ap);
	va_end(ap);

	اگر (len >= (صحیح)اندازه(header) || len < 0) {
		بازگشت false;
	}

	پیام.type = FETCH_HEADER;
	پیام.data.header_or_data.buf = (ثابت uint8_t *) header;
	پیام.data.header_or_data.len = len;

	بازگشت fetch_resource_send_callback(&پیام, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
ایستا bool fetch_resource_redirect_handler(ساختار fetch_resource_context *ctx)
{
	fetch_پیام پیام;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, 302);

	پیام.type = FETCH_REDIRECT;
	پیام.data.redirect = nsurl_access(ctx->entry->redirect_url);
	fetch_resource_send_callback(&پیام, ctx);

	بازگشت true;
}

/* resource handler that returns data directly */
ایستا bool fetch_resource_data_handler(ساختار fetch_resource_context *ctx)
{
	fetch_پیام پیام;

	/* Check if we can just return not modified */
	اگر (ctx->etag != 0 && ctx->etag == DIRECT_ETAG_مقدار) {
		fetch_set_http_code(ctx->fetchh, 304);
		پیام.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&پیام, ctx);
		بازگشت true;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	اگر (fetch_resource_send_header(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_data(ctx->entry->path)))) {
		برو fetch_resource_data_aborted;
	}

	/* content length */
	اگر (fetch_resource_send_header(ctx, "Content-Length: %" PRIsizet,
				       ctx->entry->data_len)) {
		برو fetch_resource_data_aborted;
	}

	/* create etag */
	اگر (fetch_resource_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) DIRECT_ETAG_مقدار)) {
		برو fetch_resource_data_aborted;
	}

	/* create max-age of 1 year */
	اگر (fetch_resource_send_header(ctx,
			"Cache-Control: max-age=31536000")) {
		برو fetch_resource_data_aborted;
	}

	پیام.type = FETCH_DATA;
	پیام.data.header_or_data.buf = (ثابت uint8_t *) ctx->entry->data;
	پیام.data.header_or_data.len = ctx->entry->data_len;
	fetch_resource_send_callback(&پیام, ctx);

	اگر (ctx->aborted == false) {
		پیام.type = FETCH_FINISHED;
		fetch_resource_send_callback(&پیام, ctx);
	}

fetch_resource_data_aborted:

	بازگشت true;
}

ایستا bool fetch_resource_notfound_handler(ساختار fetch_resource_context *ctx)
{
	fetch_پیام پیام;
	صحیح code = 404;
	نویسه buffer[1024];
	ثابت نویسه *title;
	نویسه کلید[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	اگر (fetch_resource_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		برو fetch_resource_notfound_handler_aborted;

	snprintf(کلید, اندازه کلید, "HTTP%03d", code);
	title = messages_get(کلید);

	snprintf(buffer, اندازه buffer,
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

	پیام.type = FETCH_DATA;
	پیام.data.header_or_data.buf = (ثابت uint8_t *) buffer;
	پیام.data.header_or_data.len = strlen(buffer);
	اگر (fetch_resource_send_callback(&پیام, ctx))
		برو fetch_resource_notfound_handler_aborted;

	پیام.type = FETCH_FINISHED;
	fetch_resource_send_callback(&پیام, ctx);

fetch_resource_notfound_handler_aborted:
	بازگشت false;
}



/** callback to initialise the resource fetcher. */
ایستا bool fetch_resource_initialise(lwc_string *scheme)
{
	ساختار fetch_resource_map_entry *e;
	uint32_t i;
	nserror res;

	fetch_resource_path_شمار = 0;

	برای (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_path_شمار];

		اگر (lwc_intern_string(fetch_resource_paths[i],
				strlen(fetch_resource_paths[i]),
				&e->path) != lwc_error_ok) {
			وقتی (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].path);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->data = پوچ;
		res = guit->fetch->get_resource_data(lwc_string_data(e->path),
						     &e->data,
						     &e->data_len);
		اگر (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_path_شمار++;
		} وگرنه {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			اگر (e->redirect_url == پوچ) {
				lwc_string_unref(e->path);
			} وگرنه {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_path_شمار++;
			}
		}
	}

	بازگشت true;
}

/** callback to finalise the resource fetcher. */
ایستا تهی fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	برای (i = 0; i < fetch_resource_path_شمار; i++) {
		lwc_string_unref(fetch_resource_map[i].path);
		اگر (fetch_resource_map[i].data != پوچ) {
			guit->fetch->release_resource_data(fetch_resource_map[i].data);
		} وگرنه {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

ایستا bool fetch_resource_can_fetch(ثابت nsurl *url)
{
	بازگشت true;
}

/**
 * set up a resource fetch context.
 */
ایستا تهی *
fetch_resource_setup(ساختار fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     ثابت نویسه *post_urlenc,
		     ثابت ساختار fetch_multipart_data *post_multipart,
		     ثابت نویسه **headers)
{
	ساختار fetch_resource_context *ctx;
	lwc_string *path;
	nserror ret;
	uint32_t i;

	ctx = calloc(1, اندازه(*ctx));
	اگر (ctx == پوچ) {
		بازگشت پوچ;
	}

	ctx->handler = fetch_resource_notfound_handler;

	اگر ((path = nsurl_get_component(url, NSURL_PATH)) != پوچ) {
		bool match;

		/* Ensure requested path is valid */
		برای (i = 0; i < fetch_resource_path_شمار; i++) {
			اگر (lwc_string_isequal(path,
					fetch_resource_map[i].path,
					&match) == lwc_error_ok && match) {
				/* found a url match, select handler */
				ctx->entry = &fetch_resource_map[i];
				اگر (ctx->entry->data != پوچ) {
					ctx->handler = fetch_resource_data_handler;
				} وگرنه {
					ctx->handler = fetch_resource_redirect_handler;
				}
				بشکن;
			}
		}

		lwc_string_unref(path);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	برای (i = 0; headers[i] != پوچ; i++) {
		اگر (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			ادامه;
		}

		/* If-None-Match: "12345678" */
		ثابت نویسه *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		وقتی (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		اگر (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->etag);
			اگر (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_درج(ring, ctx);

	بازگشت ctx;
}

/** callback to free a resource fetch */
ایستا تهی fetch_resource_free(تهی *ctx)
{
	ساختار fetch_resource_context *c = ctx;
	اگر (c->url != پوچ)
		nsurl_unref(c->url);
	آزاد(ctx);
}

/** callback to start a resource fetch */
ایستا bool fetch_resource_start(تهی *ctx)
{
	بازگشت true;
}

/** callback to abort a resource fetch */
ایستا تهی fetch_resource_abort(تهی *ctx)
{
	ساختار fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ایستا تهی fetch_resource_poll(lwc_string *scheme)
{
	ساختار fetch_resource_context *c, *save_ring = پوچ;

	وقتی (ring != پوچ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		اگر (c->locked == true) {
			RING_درج(save_ring, c);
			ادامه;
		}

		/* Only process non-aborted fetches */
		اگر (c->aborted == false) {
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

nserror fetch_resource_register(تهی)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	ثابت ساختار fetcher_operation_table fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.setup = fetch_resource_setup,
		.start = fetch_resource_start,
		.abort = fetch_resource_abort,
		.آزاد = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	بازگشت fetcher_add(scheme, &fetcher_ops);
}
