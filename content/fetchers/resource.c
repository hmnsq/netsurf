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

#تعریف_کرو براہ_راست_ETAG_قدر 123456

/** Valid resource paths */
ساکن ثابت حرف *fetch_resource_paths[] = {
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
ساکن ساخت fetch_resource_map_اندراج {
	lwc_string *راستہ; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	ثابت uint8_t *ڈیٹا; /**< direct pointer to data */
	size_t ڈیٹا_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

ساخت fetch_resource_context;

قسمتعریف bool (*fetch_resource_handler)(ساخت fetch_resource_context *);

/** Context for an resource fetch */
ساخت fetch_resource_context {
	ساخت fetch_resource_context *r_اگلا, *r_پچھلا;

	ساخت fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	ساخت fetch_resource_map_اندراج *اندراج_2; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

ساکن ساخت fetch_resource_context *ring = تہی;

ساکن uint32_t fetch_resource_راستہ_گنتی;

/** issue fetch callbacks with locking */
ساکن اندرونی bool fetch_resource_send_callback(ثابت fetch_پیغام *پیغام,
		ساخت fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پیغام, ctx->fetchh);
	ctx->locked = false;

	واپس ctx->aborted;
}

ساکن bool fetch_resource_send_سرخی(ساخت fetch_resource_context *ctx,
		ثابت حرف *fmt, ...)
{
	fetch_پیغام پیغام;
	حرف سرخی[64];
	va_list ap;
	صحیح len;

	va_start(ap, fmt);
	len = vsnprintf(سرخی, حجم سرخی, fmt, ap);
	va_end(ap);

	اگر (len >= (صحیح)حجم(سرخی) || len < 0) {
		واپس false;
	}

	پیغام.type = FETCH_سرخی;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *) سرخی;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = len;

	واپس fetch_resource_send_callback(&پیغام, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
ساکن bool fetch_resource_redirect_handler(ساخت fetch_resource_context *ctx)
{
	fetch_پیغام پیغام;

	/* content is going to return redirect */
	fetch_سیٹ_http_رمز(ctx->fetchh, 302);

	پیغام.type = FETCH_REDIRECT;
	پیغام.ڈیٹا.redirect = nsurl_access(ctx->اندراج_2->redirect_url);
	fetch_resource_send_callback(&پیغام, ctx);

	واپس true;
}

/* resource handler that returns data directly */
ساکن bool fetch_resource_ڈیٹا_handler(ساخت fetch_resource_context *ctx)
{
	fetch_پیغام پیغام;

	/* Check if we can just return not modified */
	اگر (ctx->etag != 0 && ctx->etag == براہ_راست_ETAG_قدر) {
		fetch_سیٹ_http_رمز(ctx->fetchh, 304);
		پیغام.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&پیغام, ctx);
		واپس true;
	}

	/* fetch is going to be successful */
	fetch_سیٹ_http_رمز(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	اگر (fetch_resource_send_سرخی(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_ڈیٹا(ctx->اندراج_2->راستہ)))) {
		جاؤ fetch_resource_ڈیٹا_aborted;
	}

	/* content length */
	اگر (fetch_resource_send_سرخی(ctx, "Content-Length: %" PRIsizet,
				       ctx->اندراج_2->ڈیٹا_len)) {
		جاؤ fetch_resource_ڈیٹا_aborted;
	}

	/* create etag */
	اگر (fetch_resource_send_سرخی(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) براہ_راست_ETAG_قدر)) {
		جاؤ fetch_resource_ڈیٹا_aborted;
	}

	/* create max-age of 1 year */
	اگر (fetch_resource_send_سرخی(ctx,
			"Cache-Control: max-age=31536000")) {
		جاؤ fetch_resource_ڈیٹا_aborted;
	}

	پیغام.type = FETCH_ڈیٹا;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *) ctx->اندراج_2->ڈیٹا;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = ctx->اندراج_2->ڈیٹا_len;
	fetch_resource_send_callback(&پیغام, ctx);

	اگر (ctx->aborted == false) {
		پیغام.type = FETCH_ختم_شدہ;
		fetch_resource_send_callback(&پیغام, ctx);
	}

fetch_resource_ڈیٹا_aborted:

	واپس true;
}

ساکن bool fetch_resource_notfound_handler(ساخت fetch_resource_context *ctx)
{
	fetch_پیغام پیغام;
	صحیح رمز = 404;
	حرف بفر[1024];
	ثابت حرف *title;
	حرف کلید[8];

	/* content is going to return error code */
	fetch_سیٹ_http_رمز(ctx->fetchh, رمز);

	/* content type */
	اگر (fetch_resource_send_سرخی(ctx, "Content-Type: text/html; charset=utf-8"))
		جاؤ fetch_resource_notfound_handler_aborted;

	snprintf(کلید, حجم کلید, "HTTP%03d", رمز);
	title = messages_get(کلید);

	snprintf(بفر, حجم بفر,
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
		 messages_get("FetchErrorCode"), رمز,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	پیغام.type = FETCH_ڈیٹا;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *) بفر;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = لڑی_لمبائی(بفر);
	اگر (fetch_resource_send_callback(&پیغام, ctx))
		جاؤ fetch_resource_notfound_handler_aborted;

	پیغام.type = FETCH_ختم_شدہ;
	fetch_resource_send_callback(&پیغام, ctx);

fetch_resource_notfound_handler_aborted:
	واپس false;
}



/** callback to initialise the resource fetcher. */
ساکن bool fetch_resource_initialise(lwc_string *scheme)
{
	ساخت fetch_resource_map_اندراج *e;
	uint32_t i;
	nserror res;

	fetch_resource_راستہ_گنتی = 0;

	برائے (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_راستہ_گنتی];

		اگر (lwc_intern_string(fetch_resource_paths[i],
				لڑی_لمبائی(fetch_resource_paths[i]),
				&e->راستہ) != lwc_error_ok) {
			جب (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].راستہ);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->ڈیٹا = تہی;
		res = guit->fetch->get_resource_ڈیٹا(lwc_string_ڈیٹا(e->راستہ),
						     &e->ڈیٹا,
						     &e->ڈیٹا_len);
		اگر (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_راستہ_گنتی++;
		} ورنہ {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			اگر (e->redirect_url == تہی) {
				lwc_string_unref(e->راستہ);
			} ورنہ {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_راستہ_گنتی++;
			}
		}
	}

	واپس true;
}

/** callback to finalise the resource fetcher. */
ساکن خالی fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	برائے (i = 0; i < fetch_resource_راستہ_گنتی; i++) {
		lwc_string_unref(fetch_resource_map[i].راستہ);
		اگر (fetch_resource_map[i].ڈیٹا != تہی) {
			guit->fetch->release_resource_ڈیٹا(fetch_resource_map[i].ڈیٹا);
		} ورنہ {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

ساکن bool fetch_resource_can_fetch(ثابت nsurl *url)
{
	واپس true;
}

/**
 * set up a resource fetch context.
 */
ساکن خالی *
fetch_resource_تیاری(ساخت fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     ثابت حرف *post_urlenc,
		     ثابت ساخت fetch_multipart_ڈیٹا *post_multipart,
		     ثابت حرف **سرخیاں)
{
	ساخت fetch_resource_context *ctx;
	lwc_string *راستہ;
	nserror ret;
	uint32_t i;

	ctx = صفر_حافظہ_دو(1, حجم(*ctx));
	اگر (ctx == تہی) {
		واپس تہی;
	}

	ctx->handler = fetch_resource_notfound_handler;

	اگر ((راستہ = nsurl_get_component(url, NSURL_راستہ)) != تہی) {
		bool مطابقت;

		/* Ensure requested path is valid */
		برائے (i = 0; i < fetch_resource_راستہ_گنتی; i++) {
			اگر (lwc_string_isequal(راستہ,
					fetch_resource_map[i].راستہ,
					&مطابقت) == lwc_error_ok && مطابقت) {
				/* found a url match, select handler */
				ctx->اندراج_2 = &fetch_resource_map[i];
				اگر (ctx->اندراج_2->ڈیٹا != تہی) {
					ctx->handler = fetch_resource_ڈیٹا_handler;
				} ورنہ {
					ctx->handler = fetch_resource_redirect_handler;
				}
				توڑو;
			}
		}

		lwc_string_unref(راستہ);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	برائے (i = 0; سرخیاں[i] != تہی; i++) {
		اگر (strncasecmp(سرخیاں[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			جاری;
		}

		/* If-None-Match: "12345678" */
		ثابت حرف *d = سرخیاں[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		جب (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		اگر (*d != '\0') {
			ret = nsc_snptimet(d, لڑی_لمبائی(d), &ctx->etag);
			اگر (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_داخل_کرو(ring, ctx);

	واپس ctx;
}

/** callback to free a resource fetch */
ساکن خالی fetch_resource_free(خالی *ctx)
{
	ساخت fetch_resource_context *c = ctx;
	اگر (c->url != تہی)
		nsurl_unref(c->url);
	آزاد(ctx);
}

/** callback to start a resource fetch */
ساکن bool fetch_resource_آغاز(خالی *ctx)
{
	واپس true;
}

/** callback to abort a resource fetch */
ساکن خالی fetch_resource_abort(خالی *ctx)
{
	ساخت fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ساکن خالی fetch_resource_poll(lwc_string *scheme)
{
	ساخت fetch_resource_context *c, *save_ring = تہی;

	جب (ring != تہی) {
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
			RING_داخل_کرو(save_ring, c);
			جاری;
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

nserror fetch_resource_رجسٹر(خالی)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	ثابت ساخت fetcher_operation_جدول fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.تیاری = fetch_resource_تیاری,
		.آغاز = fetch_resource_آغاز,
		.abort = fetch_resource_abort,
		.آزاد = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	واپس fetcher_جوڑو(scheme, &fetcher_ops);
}
