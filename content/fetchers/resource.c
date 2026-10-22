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

#تعریف_کړه مستقیم_ETAG_ارزښت 123456

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
ساكن بنية fetch_resource_map_ثبت {
	lwc_string *لار; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	ثابت uint8_t *معلومات; /**< direct pointer to data */
	size_t معلومات_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

بنية fetch_resource_context;

تعريفنوع bool (*fetch_resource_handler)(بنية fetch_resource_context *);

/** Context for an resource fetch */
بنية fetch_resource_context {
	بنية fetch_resource_context *r_بل, *r_prev;

	بنية fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	بنية fetch_resource_map_ثبت *ثبت; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

ساكن بنية fetch_resource_context *ring = عدم;

ساكن uint32_t fetch_resource_لار_شمېر;

/** issue fetch callbacks with locking */
ساكن مضمن bool fetch_resource_send_callback(ثابت fetch_پیغام *پیغام,
		بنية fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پیغام, ctx->fetchh);
	ctx->locked = false;

	أرجع ctx->aborted;
}

ساكن bool fetch_resource_send_سرلیک(بنية fetch_resource_context *ctx,
		ثابت حرف *fmt, ...)
{
	fetch_پیغام پیغام;
	حرف سرلیک[64];
	va_list ap;
	صحيح len;

	va_start(ap, fmt);
	len = vsnprintf(سرلیک, حجم سرلیک, fmt, ap);
	va_end(ap);

	إذا (len >= (صحيح)حجم(سرلیک) || len < 0) {
		أرجع false;
	}

	پیغام.type = FETCH_سرلیک;
	پیغام.معلومات.سرلیک_or_معلومات.buf = (ثابت uint8_t *) سرلیک;
	پیغام.معلومات.سرلیک_or_معلومات.len = len;

	أرجع fetch_resource_send_callback(&پیغام, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
ساكن bool fetch_resource_redirect_handler(بنية fetch_resource_context *ctx)
{
	fetch_پیغام پیغام;

	/* content is going to return redirect */
	fetch_سېټ_http_کوډ(ctx->fetchh, 302);

	پیغام.type = FETCH_REDIRECT;
	پیغام.معلومات.redirect = nsurl_access(ctx->ثبت->redirect_url);
	fetch_resource_send_callback(&پیغام, ctx);

	أرجع true;
}

/* resource handler that returns data directly */
ساكن bool fetch_resource_معلومات_handler(بنية fetch_resource_context *ctx)
{
	fetch_پیغام پیغام;

	/* Check if we can just return not modified */
	إذا (ctx->etag != 0 && ctx->etag == مستقیم_ETAG_ارزښت) {
		fetch_سېټ_http_کوډ(ctx->fetchh, 304);
		پیغام.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&پیغام, ctx);
		أرجع true;
	}

	/* fetch is going to be successful */
	fetch_سېټ_http_کوډ(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	إذا (fetch_resource_send_سرلیک(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_معلومات(ctx->ثبت->لار)))) {
		اذهب fetch_resource_معلومات_aborted;
	}

	/* content length */
	إذا (fetch_resource_send_سرلیک(ctx, "Content-Length: %" PRIsizet,
				       ctx->ثبت->معلومات_len)) {
		اذهب fetch_resource_معلومات_aborted;
	}

	/* create etag */
	إذا (fetch_resource_send_سرلیک(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) مستقیم_ETAG_ارزښت)) {
		اذهب fetch_resource_معلومات_aborted;
	}

	/* create max-age of 1 year */
	إذا (fetch_resource_send_سرلیک(ctx,
			"Cache-Control: max-age=31536000")) {
		اذهب fetch_resource_معلومات_aborted;
	}

	پیغام.type = FETCH_معلومات;
	پیغام.معلومات.سرلیک_or_معلومات.buf = (ثابت uint8_t *) ctx->ثبت->معلومات;
	پیغام.معلومات.سرلیک_or_معلومات.len = ctx->ثبت->معلومات_len;
	fetch_resource_send_callback(&پیغام, ctx);

	إذا (ctx->aborted == false) {
		پیغام.type = FETCH_پای_ته_رسېدلی;
		fetch_resource_send_callback(&پیغام, ctx);
	}

fetch_resource_معلومات_aborted:

	أرجع true;
}

ساكن bool fetch_resource_notfound_handler(بنية fetch_resource_context *ctx)
{
	fetch_پیغام پیغام;
	صحيح کوډ = 404;
	حرف بفر[1024];
	ثابت حرف *title;
	حرف کلی[8];

	/* content is going to return error code */
	fetch_سېټ_http_کوډ(ctx->fetchh, کوډ);

	/* content type */
	إذا (fetch_resource_send_سرلیک(ctx, "Content-Type: text/html; charset=utf-8"))
		اذهب fetch_resource_notfound_handler_aborted;

	snprintf(کلی, حجم کلی, "HTTP%03d", کوډ);
	title = messages_get(کلی);

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
		 messages_get("FetchErrorCode"), کوډ,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	پیغام.type = FETCH_معلومات;
	پیغام.معلومات.سرلیک_or_معلومات.buf = (ثابت uint8_t *) بفر;
	پیغام.معلومات.سرلیک_or_معلومات.len = مزي_اوږدوالی(بفر);
	إذا (fetch_resource_send_callback(&پیغام, ctx))
		اذهب fetch_resource_notfound_handler_aborted;

	پیغام.type = FETCH_پای_ته_رسېدلی;
	fetch_resource_send_callback(&پیغام, ctx);

fetch_resource_notfound_handler_aborted:
	أرجع false;
}



/** callback to initialise the resource fetcher. */
ساكن bool fetch_resource_initialise(lwc_string *scheme)
{
	بنية fetch_resource_map_ثبت *e;
	uint32_t i;
	nserror res;

	fetch_resource_لار_شمېر = 0;

	لكل (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_لار_شمېر];

		إذا (lwc_intern_string(fetch_resource_paths[i],
				مزي_اوږدوالی(fetch_resource_paths[i]),
				&e->لار) != lwc_error_ok) {
			طالما (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].لار);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->معلومات = عدم;
		res = guit->fetch->get_resource_معلومات(lwc_string_معلومات(e->لار),
						     &e->معلومات,
						     &e->معلومات_len);
		إذا (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_لار_شمېر++;
		} وإلا {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			إذا (e->redirect_url == عدم) {
				lwc_string_unref(e->لار);
			} وإلا {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_لار_شمېر++;
			}
		}
	}

	أرجع true;
}

/** callback to finalise the resource fetcher. */
ساكن فراغ fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	لكل (i = 0; i < fetch_resource_لار_شمېر; i++) {
		lwc_string_unref(fetch_resource_map[i].لار);
		إذا (fetch_resource_map[i].معلومات != عدم) {
			guit->fetch->release_resource_معلومات(fetch_resource_map[i].معلومات);
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
fetch_resource_تیاری(بنية fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     ثابت حرف *post_urlenc,
		     ثابت بنية fetch_multipart_معلومات *post_multipart,
		     ثابت حرف **سرلیکونه)
{
	بنية fetch_resource_context *ctx;
	lwc_string *لار;
	nserror ret;
	uint32_t i;

	ctx = حافظه_صفر_ورکړه(1, حجم(*ctx));
	إذا (ctx == عدم) {
		أرجع عدم;
	}

	ctx->handler = fetch_resource_notfound_handler;

	إذا ((لار = nsurl_get_component(url, NSURL_لار)) != عدم) {
		bool سمون;

		/* Ensure requested path is valid */
		لكل (i = 0; i < fetch_resource_لار_شمېر; i++) {
			إذا (lwc_string_isequal(لار,
					fetch_resource_map[i].لار,
					&سمون) == lwc_error_ok && سمون) {
				/* found a url match, select handler */
				ctx->ثبت = &fetch_resource_map[i];
				إذا (ctx->ثبت->معلومات != عدم) {
					ctx->handler = fetch_resource_معلومات_handler;
				} وإلا {
					ctx->handler = fetch_resource_redirect_handler;
				}
				اكسر;
			}
		}

		lwc_string_unref(لار);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	لكل (i = 0; سرلیکونه[i] != عدم; i++) {
		إذا (strncasecmp(سرلیکونه[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			استمر;
		}

		/* If-None-Match: "12345678" */
		ثابت حرف *d = سرلیکونه[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		طالما (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		إذا (*d != '\0') {
			ret = nsc_snptimet(d, مزي_اوږدوالی(d), &ctx->etag);
			إذا (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_ورننباسه(ring, ctx);

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
ساكن bool fetch_resource_پیل(فراغ *ctx)
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
			RING_ورننباسه(save_ring, c);
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

nserror fetch_resource_ثبتوونکی(فراغ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	ثابت بنية fetcher_operation_جدول fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.تیاری = fetch_resource_تیاری,
		.پیل = fetch_resource_پیل,
		.abort = fetch_resource_abort,
		.حرر = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	أرجع fetcher_جمع_کړه(scheme, &fetcher_ops);
}
