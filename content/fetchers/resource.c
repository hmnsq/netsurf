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

#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <strings.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdarg.h>
#অন্তর্ভাৱ <libwapcaplet/libwapcaplet.h>

#অন্তর্ভাৱ "netsurf/inttypes.h"
#অন্তর্ভাৱ "utils/nsurl.h"
#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/time.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/ring.h"
#অন্তর্ভাৱ "netsurf/fetch.h"
#অন্তর্ভাৱ "desktop/gui_internal.h"

#অন্তর্ভাৱ "content/fetch.h"
#অন্তর্ভাৱ "content/fetchers.h"
#অন্তর্ভাৱ "content/fetchers/resource.h"

#define DIRECT_ETAG_মান 123456

/** Valid resource paths */
স্থির ধ্রুৱ ৱর্ণ *fetch_resource_paths[] = {
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
স্থির সংরচনা fetch_resource_map_entry {
	lwc_string *path; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	ধ্রুৱ uint8_t *data; /**< direct pointer to data */
	size_t data_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

সংরচনা fetch_resource_context;

প্রকারনির্দেশ bool (*fetch_resource_handler)(সংরচনা fetch_resource_context *);

/** Context for an resource fetch */
সংরচনা fetch_resource_context {
	সংরচনা fetch_resource_context *r_next, *r_prev;

	সংরচনা fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	সংরচনা fetch_resource_map_entry *entry; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

স্থির সংরচনা fetch_resource_context *ring = রিক্ত;

স্থির uint32_t fetch_resource_path_গুনতি;

/** issue fetch callbacks with locking */
স্থির অন্তঃস্থ bool fetch_resource_send_callback(ধ্রুৱ fetch_বাৰ্তা *বাৰ্তা,
		সংরচনা fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(বাৰ্তা, ctx->fetchh);
	ctx->locked = false;

	ঘূৰাই ctx->aborted;
}

স্থির bool fetch_resource_send_header(সংরচনা fetch_resource_context *ctx,
		ধ্রুৱ ৱর্ণ *fmt, ...)
{
	fetch_বাৰ্তা বাৰ্তা;
	ৱর্ণ header[64];
	va_list ap;
	পূর্ণাঙ্ক len;

	va_start(ap, fmt);
	len = vsnprintf(header, পরিমাণ header, fmt, ap);
	va_end(ap);

	যদি (len >= (পূর্ণাঙ্ক)পরিমাণ(header) || len < 0) {
		ঘূৰাই false;
	}

	বাৰ্তা.type = FETCH_HEADER;
	বাৰ্তা.data.header_or_data.buf = (ধ্রুৱ uint8_t *) header;
	বাৰ্তা.data.header_or_data.len = len;

	ঘূৰাই fetch_resource_send_callback(&বাৰ্তা, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
স্থির bool fetch_resource_redirect_handler(সংরচনা fetch_resource_context *ctx)
{
	fetch_বাৰ্তা বাৰ্তা;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, 302);

	বাৰ্তা.type = FETCH_REDIRECT;
	বাৰ্তা.data.redirect = nsurl_access(ctx->entry->redirect_url);
	fetch_resource_send_callback(&বাৰ্তা, ctx);

	ঘূৰাই true;
}

/* resource handler that returns data directly */
স্থির bool fetch_resource_data_handler(সংরচনা fetch_resource_context *ctx)
{
	fetch_বাৰ্তা বাৰ্তা;

	/* Check if we can just return not modified */
	যদি (ctx->etag != 0 && ctx->etag == DIRECT_ETAG_মান) {
		fetch_set_http_code(ctx->fetchh, 304);
		বাৰ্তা.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&বাৰ্তা, ctx);
		ঘূৰাই true;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	যদি (fetch_resource_send_header(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_data(ctx->entry->path)))) {
		গচ্ছ fetch_resource_data_aborted;
	}

	/* content length */
	যদি (fetch_resource_send_header(ctx, "Content-Length: %" PRIsizet,
				       ctx->entry->data_len)) {
		গচ্ছ fetch_resource_data_aborted;
	}

	/* create etag */
	যদি (fetch_resource_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) DIRECT_ETAG_মান)) {
		গচ্ছ fetch_resource_data_aborted;
	}

	/* create max-age of 1 year */
	যদি (fetch_resource_send_header(ctx,
			"Cache-Control: max-age=31536000")) {
		গচ্ছ fetch_resource_data_aborted;
	}

	বাৰ্তা.type = FETCH_DATA;
	বাৰ্তা.data.header_or_data.buf = (ধ্রুৱ uint8_t *) ctx->entry->data;
	বাৰ্তা.data.header_or_data.len = ctx->entry->data_len;
	fetch_resource_send_callback(&বাৰ্তা, ctx);

	যদি (ctx->aborted == false) {
		বাৰ্তা.type = FETCH_FINISHED;
		fetch_resource_send_callback(&বাৰ্তা, ctx);
	}

fetch_resource_data_aborted:

	ঘূৰাই true;
}

স্থির bool fetch_resource_notfound_handler(সংরচনা fetch_resource_context *ctx)
{
	fetch_বাৰ্তা বাৰ্তা;
	পূর্ণাঙ্ক code = 404;
	ৱর্ণ buffer[1024];
	ধ্রুৱ ৱর্ণ *title;
	ৱর্ণ চাবি[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	যদি (fetch_resource_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		গচ্ছ fetch_resource_notfound_handler_aborted;

	snprintf(চাবি, পরিমাণ চাবি, "HTTP%03d", code);
	title = messages_get(চাবি);

	snprintf(buffer, পরিমাণ buffer,
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

	বাৰ্তা.type = FETCH_DATA;
	বাৰ্তা.data.header_or_data.buf = (ধ্রুৱ uint8_t *) buffer;
	বাৰ্তা.data.header_or_data.len = strlen(buffer);
	যদি (fetch_resource_send_callback(&বাৰ্তা, ctx))
		গচ্ছ fetch_resource_notfound_handler_aborted;

	বাৰ্তা.type = FETCH_FINISHED;
	fetch_resource_send_callback(&বাৰ্তা, ctx);

fetch_resource_notfound_handler_aborted:
	ঘূৰাই false;
}



/** callback to initialise the resource fetcher. */
স্থির bool fetch_resource_initialise(lwc_string *scheme)
{
	সংরচনা fetch_resource_map_entry *e;
	uint32_t i;
	nserror res;

	fetch_resource_path_গুনতি = 0;

	বাবে (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_path_গুনতি];

		যদি (lwc_intern_string(fetch_resource_paths[i],
				strlen(fetch_resource_paths[i]),
				&e->path) != lwc_error_ok) {
			যেতিয়ালৈ (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].path);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->data = রিক্ত;
		res = guit->fetch->get_resource_data(lwc_string_data(e->path),
						     &e->data,
						     &e->data_len);
		যদি (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_path_গুনতি++;
		} অন্যথা {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			যদি (e->redirect_url == রিক্ত) {
				lwc_string_unref(e->path);
			} অন্যথা {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_path_গুনতি++;
			}
		}
	}

	ঘূৰাই true;
}

/** callback to finalise the resource fetcher. */
স্থির শূন্য fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	বাবে (i = 0; i < fetch_resource_path_গুনতি; i++) {
		lwc_string_unref(fetch_resource_map[i].path);
		যদি (fetch_resource_map[i].data != রিক্ত) {
			guit->fetch->release_resource_data(fetch_resource_map[i].data);
		} অন্যথা {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

স্থির bool fetch_resource_can_fetch(ধ্রুৱ nsurl *url)
{
	ঘূৰাই true;
}

/**
 * set up a resource fetch context.
 */
স্থির শূন্য *
fetch_resource_setup(সংরচনা fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     ধ্রুৱ ৱর্ণ *post_urlenc,
		     ধ্রুৱ সংরচনা fetch_multipart_data *post_multipart,
		     ধ্রুৱ ৱর্ণ **headers)
{
	সংরচনা fetch_resource_context *ctx;
	lwc_string *path;
	nserror ret;
	uint32_t i;

	ctx = calloc(1, পরিমাণ(*ctx));
	যদি (ctx == রিক্ত) {
		ঘূৰাই রিক্ত;
	}

	ctx->handler = fetch_resource_notfound_handler;

	যদি ((path = nsurl_get_component(url, NSURL_PATH)) != রিক্ত) {
		bool match;

		/* Ensure requested path is valid */
		বাবে (i = 0; i < fetch_resource_path_গুনতি; i++) {
			যদি (lwc_string_isequal(path,
					fetch_resource_map[i].path,
					&match) == lwc_error_ok && match) {
				/* found a url match, select handler */
				ctx->entry = &fetch_resource_map[i];
				যদি (ctx->entry->data != রিক্ত) {
					ctx->handler = fetch_resource_data_handler;
				} অন্যথা {
					ctx->handler = fetch_resource_redirect_handler;
				}
				ভঙ্গ;
			}
		}

		lwc_string_unref(path);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	বাবে (i = 0; headers[i] != রিক্ত; i++) {
		যদি (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			অনুৱর্ত;
		}

		/* If-None-Match: "12345678" */
		ধ্রুৱ ৱর্ণ *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		যেতিয়ালৈ (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		যদি (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->etag);
			যদি (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_ঢোকাও(ring, ctx);

	ঘূৰাই ctx;
}

/** callback to free a resource fetch */
স্থির শূন্য fetch_resource_free(শূন্য *ctx)
{
	সংরচনা fetch_resource_context *c = ctx;
	যদি (c->url != রিক্ত)
		nsurl_unref(c->url);
	মুক্ত(ctx);
}

/** callback to start a resource fetch */
স্থির bool fetch_resource_start(শূন্য *ctx)
{
	ঘূৰাই true;
}

/** callback to abort a resource fetch */
স্থির শূন্য fetch_resource_abort(শূন্য *ctx)
{
	সংরচনা fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
স্থির শূন্য fetch_resource_poll(lwc_string *scheme)
{
	সংরচনা fetch_resource_context *c, *save_ring = রিক্ত;

	যেতিয়ালৈ (ring != রিক্ত) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		যদি (c->locked == true) {
			RING_ঢোকাও(save_ring, c);
			অনুৱর্ত;
		}

		/* Only process non-aborted fetches */
		যদি (c->aborted == false) {
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

nserror fetch_resource_register(শূন্য)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	ধ্রুৱ সংরচনা fetcher_operation_table fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.setup = fetch_resource_setup,
		.start = fetch_resource_start,
		.abort = fetch_resource_abort,
		.মুক্ত = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	ঘূৰাই fetcher_add(scheme, &fetcher_ops);
}
