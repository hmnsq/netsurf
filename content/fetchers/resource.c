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

#ضمن <stdlib.h>
#ضمن <stdbool.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <stdio.h>
#ضمن <stdarg.h>
#ضمن <libwapcaplet/libwapcaplet.h>

#ضمن "netsurf/inttypes.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"
#ضمن "utils/time.h"
#ضمن "utils/messages.h"
#ضمن "utils/utils.h"
#ضمن "utils/ring.h"
#ضمن "netsurf/fetch.h"
#ضمن "desktop/gui_internal.h"

#ضمن "content/fetch.h"
#ضمن "content/fetchers.h"
#ضمن "content/fetchers/resource.h"

#تعریف_کن مستقیم_ETAG_مقدار 123456

/** Valid resource paths */
ساكن ثابت حرف *fetch_resource_paths[] = {
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
ساكن بنية fetch_resource_map_مدخل {
	lwc_string *مسیر; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	ثابت uint8_t *داده; /**< direct pointer to data */
	size_t داده_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

بنية fetch_resource_context;

تعريفنوع bool (*fetch_resource_handler)(بنية fetch_resource_context *);

/** Context for an resource fetch */
بنية fetch_resource_context {
	بنية fetch_resource_context *r_بعدی, *r_prev;

	بنية fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	بنية fetch_resource_map_مدخل *مدخل; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

ساكن بنية fetch_resource_context *ring = عدم;

ساكن uint32_t fetch_resource_مسیر_شمار;

/** issue fetch callbacks with locking */
ساكن مضمن bool fetch_resource_send_callback(ثابت fetch_پیام *پیام,
		بنية fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پیام, ctx->fetchh);
	ctx->locked = false;

	أرجع ctx->aborted;
}

ساكن bool fetch_resource_send_سرآیند(بنية fetch_resource_context *ctx,
		ثابت حرف *fmt, ...)
{
	fetch_پیام پیام;
	حرف سرآیند[64];
	va_list ap;
	صحيح len;

	va_start(ap, fmt);
	len = vsnprintf(سرآیند, حجم سرآیند, fmt, ap);
	va_end(ap);

	إذا (len >= (صحيح)حجم(سرآیند) || len < 0) {
		أرجع false;
	}

	پیام.type = FETCH_سرآیند;
	پیام.داده.سرآیند_or_داده.buf = (ثابت uint8_t *) سرآیند;
	پیام.داده.سرآیند_or_داده.len = len;

	أرجع fetch_resource_send_callback(&پیام, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
ساكن bool fetch_resource_redirect_handler(بنية fetch_resource_context *ctx)
{
	fetch_پیام پیام;

	/* content is going to return redirect */
	fetch_مجموعه_http_کد(ctx->fetchh, 302);

	پیام.type = FETCH_REDIRECT;
	پیام.داده.redirect = nsurl_access(ctx->مدخل->redirect_url);
	fetch_resource_send_callback(&پیام, ctx);

	أرجع true;
}

/* resource handler that returns data directly */
ساكن bool fetch_resource_داده_handler(بنية fetch_resource_context *ctx)
{
	fetch_پیام پیام;

	/* Check if we can just return not modified */
	إذا (ctx->etag != 0 && ctx->etag == مستقیم_ETAG_مقدار) {
		fetch_مجموعه_http_کد(ctx->fetchh, 304);
		پیام.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&پیام, ctx);
		أرجع true;
	}

	/* fetch is going to be successful */
	fetch_مجموعه_http_کد(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	إذا (fetch_resource_send_سرآیند(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_داده(ctx->مدخل->مسیر)))) {
		اذهب fetch_resource_داده_aborted;
	}

	/* content length */
	إذا (fetch_resource_send_سرآیند(ctx, "Content-Length: %" PRIsizet,
				       ctx->مدخل->داده_len)) {
		اذهب fetch_resource_داده_aborted;
	}

	/* create etag */
	إذا (fetch_resource_send_سرآیند(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) مستقیم_ETAG_مقدار)) {
		اذهب fetch_resource_داده_aborted;
	}

	/* create max-age of 1 year */
	إذا (fetch_resource_send_سرآیند(ctx,
			"Cache-Control: max-age=31536000")) {
		اذهب fetch_resource_داده_aborted;
	}

	پیام.type = FETCH_داده;
	پیام.داده.سرآیند_or_داده.buf = (ثابت uint8_t *) ctx->مدخل->داده;
	پیام.داده.سرآیند_or_داده.len = ctx->مدخل->داده_len;
	fetch_resource_send_callback(&پیام, ctx);

	إذا (ctx->aborted == false) {
		پیام.type = FETCH_پایانیافته;
		fetch_resource_send_callback(&پیام, ctx);
	}

fetch_resource_داده_aborted:

	أرجع true;
}

ساكن bool fetch_resource_notfound_handler(بنية fetch_resource_context *ctx)
{
	fetch_پیام پیام;
	صحيح کد = 404;
	حرف میانگیر[1024];
	ثابت حرف *title;
	حرف کلید[8];

	/* content is going to return error code */
	fetch_مجموعه_http_کد(ctx->fetchh, کد);

	/* content type */
	إذا (fetch_resource_send_سرآیند(ctx, "Content-Type: text/html; charset=utf-8"))
		اذهب fetch_resource_notfound_handler_aborted;

	snprintf(کلید, حجم کلید, "HTTP%03d", کد);
	title = messages_get(کلید);

	snprintf(میانگیر, حجم میانگیر,
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
		 messages_get("FetchErrorCode"), کد,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	پیام.type = FETCH_داده;
	پیام.داده.سرآیند_or_داده.buf = (ثابت uint8_t *) میانگیر;
	پیام.داده.سرآیند_or_داده.len = طول_رشته(میانگیر);
	إذا (fetch_resource_send_callback(&پیام, ctx))
		اذهب fetch_resource_notfound_handler_aborted;

	پیام.type = FETCH_پایانیافته;
	fetch_resource_send_callback(&پیام, ctx);

fetch_resource_notfound_handler_aborted:
	أرجع false;
}



/** callback to initialise the resource fetcher. */
ساكن bool fetch_resource_initialise(lwc_string *scheme)
{
	بنية fetch_resource_map_مدخل *e;
	uint32_t i;
	nserror res;

	fetch_resource_مسیر_شمار = 0;

	لكل (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_مسیر_شمار];

		إذا (lwc_intern_string(fetch_resource_paths[i],
				طول_رشته(fetch_resource_paths[i]),
				&e->مسیر) != lwc_error_ok) {
			طالما (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].مسیر);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->داده = عدم;
		res = guit->fetch->get_resource_داده(lwc_string_داده(e->مسیر),
						     &e->داده,
						     &e->داده_len);
		إذا (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_مسیر_شمار++;
		} وإلا {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			إذا (e->redirect_url == عدم) {
				lwc_string_unref(e->مسیر);
			} وإلا {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_مسیر_شمار++;
			}
		}
	}

	أرجع true;
}

/** callback to finalise the resource fetcher. */
ساكن فراغ fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	لكل (i = 0; i < fetch_resource_مسیر_شمار; i++) {
		lwc_string_unref(fetch_resource_map[i].مسیر);
		إذا (fetch_resource_map[i].داده != عدم) {
			guit->fetch->release_resource_داده(fetch_resource_map[i].داده);
		} وإلا {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

ساكن bool fetch_resource_can_fetch(ثابت nsurl *url)
{
	أرجع true;
}

/**
 * set up a resource fetch context.
 */
ساكن فراغ *
fetch_resource_راهاندازی(بنية fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     ثابت حرف *post_urlenc,
		     ثابت بنية fetch_multipart_داده *post_multipart,
		     ثابت حرف **سرآیندها)
{
	بنية fetch_resource_context *ctx;
	lwc_string *مسیر;
	nserror ret;
	uint32_t i;

	ctx = حافظه_صفر_بده(1, حجم(*ctx));
	إذا (ctx == عدم) {
		أرجع عدم;
	}

	ctx->handler = fetch_resource_notfound_handler;

	إذا ((مسیر = nsurl_get_component(url, NSURL_مسیر)) != عدم) {
		bool تطابق;

		/* Ensure requested path is valid */
		لكل (i = 0; i < fetch_resource_مسیر_شمار; i++) {
			إذا (lwc_string_isequal(مسیر,
					fetch_resource_map[i].مسیر,
					&تطابق) == lwc_error_ok && تطابق) {
				/* found a url match, select handler */
				ctx->مدخل = &fetch_resource_map[i];
				إذا (ctx->مدخل->داده != عدم) {
					ctx->handler = fetch_resource_داده_handler;
				} وإلا {
					ctx->handler = fetch_resource_redirect_handler;
				}
				اكسر;
			}
		}

		lwc_string_unref(مسیر);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	لكل (i = 0; سرآیندها[i] != عدم; i++) {
		إذا (strncasecmp(سرآیندها[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			استمر;
		}

		/* If-None-Match: "12345678" */
		ثابت حرف *d = سرآیندها[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		طالما (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		إذا (*d != '\0') {
			ret = nsc_snptimet(d, طول_رشته(d), &ctx->etag);
			إذا (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_درج(ring, ctx);

	أرجع ctx;
}

/** callback to free a resource fetch */
ساكن فراغ fetch_resource_free(فراغ *ctx)
{
	بنية fetch_resource_context *c = ctx;
	إذا (c->url != عدم)
		nsurl_unref(c->url);
	حرر(ctx);
}

/** callback to start a resource fetch */
ساكن bool fetch_resource_شروع(فراغ *ctx)
{
	أرجع true;
}

/** callback to abort a resource fetch */
ساكن فراغ fetch_resource_abort(فراغ *ctx)
{
	بنية fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ساكن فراغ fetch_resource_poll(lwc_string *scheme)
{
	بنية fetch_resource_context *c, *save_ring = عدم;

	طالما (ring != عدم) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		إذا (c->locked == true) {
			RING_درج(save_ring, c);
			استمر;
		}

		/* Only process non-aborted fetches */
		إذا (c->aborted == false) {
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

nserror fetch_resource_ثبات(فراغ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	ثابت بنية fetcher_operation_جدول fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.راهاندازی = fetch_resource_راهاندازی,
		.شروع = fetch_resource_شروع,
		.abort = fetch_resource_abort,
		.حرر = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	أرجع fetcher_جمع_کن(scheme, &fetcher_ops);
}
