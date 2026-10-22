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

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <libwapcaplet/libwapcaplet.h>

#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/time.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/ring.h"
#অন্তর্ভাব "netsurf/fetch.h"
#অন্তর্ভাব "desktop/gui_internal.h"

#অন্তর্ভাব "content/fetch.h"
#অন্তর্ভাব "content/fetchers.h"
#অন্তর্ভাব "content/fetchers/resource.h"

#define DIRECT_ETAG_মান 123456

/** Valid resource paths */
স্থির ধ্রুবক অক্ষর *fetch_resource_paths[] = {
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
স্থির কাঠামো fetch_resource_map_entry {
	lwc_string *path; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	ধ্রুবক uint8_t *তথ্য; /**< direct pointer to data */
	size_t তথ্য_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

কাঠামো fetch_resource_context;

প্রকারলেখ bool (*fetch_resource_handler)(কাঠামো fetch_resource_context *);

/** Context for an resource fetch */
কাঠামো fetch_resource_context {
	কাঠামো fetch_resource_context *r_পরবর্তী, *r_পূর্ব;

	কাঠামো fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	কাঠামো fetch_resource_map_entry *entry; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

স্থির কাঠামো fetch_resource_context *ring = ফাঁকা;

স্থির uint32_t fetch_resource_path_গুনতি;

/** issue fetch callbacks with locking */
স্থির অন্তঃরেখ bool fetch_resource_send_callback(ধ্রুবক fetch_বার্তা *বার্তা,
		কাঠামো fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(বার্তা, ctx->fetchh);
	ctx->locked = false;

	ফেরত ctx->aborted;
}

স্থির bool fetch_resource_send_header(কাঠামো fetch_resource_context *ctx,
		ধ্রুবক অক্ষর *fmt, ...)
{
	fetch_বার্তা বার্তা;
	অক্ষর header[64];
	va_list ap;
	পূর্ণসংখ্যা len;

	va_start(ap, fmt);
	len = vsnprintf(header, আকার header, fmt, ap);
	va_end(ap);

	যদি (len >= (পূর্ণসংখ্যা)আকার(header) || len < 0) {
		ফেরত false;
	}

	বার্তা.type = FETCH_HEADER;
	বার্তা.তথ্য.header_or_তথ্য.buf = (ধ্রুবক uint8_t *) header;
	বার্তা.তথ্য.header_or_তথ্য.len = len;

	ফেরত fetch_resource_send_callback(&বার্তা, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
স্থির bool fetch_resource_redirect_handler(কাঠামো fetch_resource_context *ctx)
{
	fetch_বার্তা বার্তা;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, 302);

	বার্তা.type = FETCH_REDIRECT;
	বার্তা.তথ্য.redirect = nsurl_access(ctx->entry->redirect_url);
	fetch_resource_send_callback(&বার্তা, ctx);

	ফেরত true;
}

/* resource handler that returns data directly */
স্থির bool fetch_resource_তথ্য_handler(কাঠামো fetch_resource_context *ctx)
{
	fetch_বার্তা বার্তা;

	/* Check if we can just return not modified */
	যদি (ctx->etag != 0 && ctx->etag == DIRECT_ETAG_মান) {
		fetch_set_http_code(ctx->fetchh, 304);
		বার্তা.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&বার্তা, ctx);
		ফেরত true;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	যদি (fetch_resource_send_header(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_তথ্য(ctx->entry->path)))) {
		যাও fetch_resource_তথ্য_aborted;
	}

	/* content length */
	যদি (fetch_resource_send_header(ctx, "Content-Length: %" PRIsizet,
				       ctx->entry->তথ্য_len)) {
		যাও fetch_resource_তথ্য_aborted;
	}

	/* create etag */
	যদি (fetch_resource_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) DIRECT_ETAG_মান)) {
		যাও fetch_resource_তথ্য_aborted;
	}

	/* create max-age of 1 year */
	যদি (fetch_resource_send_header(ctx,
			"Cache-Control: max-age=31536000")) {
		যাও fetch_resource_তথ্য_aborted;
	}

	বার্তা.type = FETCH_তথ্য;
	বার্তা.তথ্য.header_or_তথ্য.buf = (ধ্রুবক uint8_t *) ctx->entry->তথ্য;
	বার্তা.তথ্য.header_or_তথ্য.len = ctx->entry->তথ্য_len;
	fetch_resource_send_callback(&বার্তা, ctx);

	যদি (ctx->aborted == false) {
		বার্তা.type = FETCH_FINISHED;
		fetch_resource_send_callback(&বার্তা, ctx);
	}

fetch_resource_তথ্য_aborted:

	ফেরত true;
}

স্থির bool fetch_resource_notfound_handler(কাঠামো fetch_resource_context *ctx)
{
	fetch_বার্তা বার্তা;
	পূর্ণসংখ্যা code = 404;
	অক্ষর বাফার[1024];
	ধ্রুবক অক্ষর *title;
	অক্ষর চাবি[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	যদি (fetch_resource_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		যাও fetch_resource_notfound_handler_aborted;

	snprintf(চাবি, আকার চাবি, "HTTP%03d", code);
	title = messages_get(চাবি);

	snprintf(বাফার, আকার বাফার,
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

	বার্তা.type = FETCH_তথ্য;
	বার্তা.তথ্য.header_or_তথ্য.buf = (ধ্রুবক uint8_t *) বাফার;
	বার্তা.তথ্য.header_or_তথ্য.len = strlen(বাফার);
	যদি (fetch_resource_send_callback(&বার্তা, ctx))
		যাও fetch_resource_notfound_handler_aborted;

	বার্তা.type = FETCH_FINISHED;
	fetch_resource_send_callback(&বার্তা, ctx);

fetch_resource_notfound_handler_aborted:
	ফেরত false;
}



/** callback to initialise the resource fetcher. */
স্থির bool fetch_resource_initialise(lwc_string *scheme)
{
	কাঠামো fetch_resource_map_entry *e;
	uint32_t i;
	nserror res;

	fetch_resource_path_গুনতি = 0;

	জন্য (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_path_গুনতি];

		যদি (lwc_intern_string(fetch_resource_paths[i],
				strlen(fetch_resource_paths[i]),
				&e->path) != lwc_error_ok) {
			যখন (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].path);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->তথ্য = ফাঁকা;
		res = guit->fetch->get_resource_তথ্য(lwc_string_তথ্য(e->path),
						     &e->তথ্য,
						     &e->তথ্য_len);
		যদি (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_path_গুনতি++;
		} অন্যথা {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			যদি (e->redirect_url == ফাঁকা) {
				lwc_string_unref(e->path);
			} অন্যথা {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_path_গুনতি++;
			}
		}
	}

	ফেরত true;
}

/** callback to finalise the resource fetcher. */
স্থির শূন্য fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	জন্য (i = 0; i < fetch_resource_path_গুনতি; i++) {
		lwc_string_unref(fetch_resource_map[i].path);
		যদি (fetch_resource_map[i].তথ্য != ফাঁকা) {
			guit->fetch->release_resource_তথ্য(fetch_resource_map[i].তথ্য);
		} অন্যথা {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

স্থির bool fetch_resource_can_fetch(ধ্রুবক nsurl *url)
{
	ফেরত true;
}

/**
 * set up a resource fetch context.
 */
স্থির শূন্য *
fetch_resource_setup(কাঠামো fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     ধ্রুবক অক্ষর *post_urlenc,
		     ধ্রুবক কাঠামো fetch_multipart_তথ্য *post_multipart,
		     ধ্রুবক অক্ষর **headers)
{
	কাঠামো fetch_resource_context *ctx;
	lwc_string *path;
	nserror ret;
	uint32_t i;

	ctx = calloc(1, আকার(*ctx));
	যদি (ctx == ফাঁকা) {
		ফেরত ফাঁকা;
	}

	ctx->handler = fetch_resource_notfound_handler;

	যদি ((path = nsurl_get_component(url, NSURL_PATH)) != ফাঁকা) {
		bool match;

		/* Ensure requested path is valid */
		জন্য (i = 0; i < fetch_resource_path_গুনতি; i++) {
			যদি (lwc_string_isequal(path,
					fetch_resource_map[i].path,
					&match) == lwc_error_ok && match) {
				/* found a url match, select handler */
				ctx->entry = &fetch_resource_map[i];
				যদি (ctx->entry->তথ্য != ফাঁকা) {
					ctx->handler = fetch_resource_তথ্য_handler;
				} অন্যথা {
					ctx->handler = fetch_resource_redirect_handler;
				}
				বিরতি;
			}
		}

		lwc_string_unref(path);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	জন্য (i = 0; headers[i] != ফাঁকা; i++) {
		যদি (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			চালু;
		}

		/* If-None-Match: "12345678" */
		ধ্রুবক অক্ষর *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		যখন (*d != '\0' && (*d < '0' || '9' < *d))
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

	ফেরত ctx;
}

/** callback to free a resource fetch */
স্থির শূন্য fetch_resource_free(শূন্য *ctx)
{
	কাঠামো fetch_resource_context *c = ctx;
	যদি (c->url != ফাঁকা)
		nsurl_unref(c->url);
	মুক্ত(ctx);
}

/** callback to start a resource fetch */
স্থির bool fetch_resource_start(শূন্য *ctx)
{
	ফেরত true;
}

/** callback to abort a resource fetch */
স্থির শূন্য fetch_resource_abort(শূন্য *ctx)
{
	কাঠামো fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
স্থির শূন্য fetch_resource_poll(lwc_string *scheme)
{
	কাঠামো fetch_resource_context *c, *save_ring = ফাঁকা;

	যখন (ring != ফাঁকা) {
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
			চালু;
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
	ধ্রুবক কাঠামো fetcher_operation_table fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.setup = fetch_resource_setup,
		.start = fetch_resource_start,
		.abort = fetch_resource_abort,
		.মুক্ত = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	ফেরত fetcher_add(scheme, &fetcher_ops);
}
