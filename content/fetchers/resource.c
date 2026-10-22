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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libwapcaplet/libwapcaplet.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/time.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers/resource.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਿੱਧਾ_ETAG_ਮੁੱਲ 123456

/** Valid resource paths */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *fetch_resource_paths[] = {
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
ਸਥਿਰ ਬਣਤਰ fetch_resource_map_ਇੰਦਰਾਜ {
	lwc_string *ਰਸਤਾ; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	ਅਚਲ uint8_t *ਅੰਕੜੇ; /**< direct pointer to data */
	size_t ਅੰਕੜੇ_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

ਬਣਤਰ fetch_resource_context;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ bool (*fetch_resource_handler)(ਬਣਤਰ fetch_resource_context *);

/** Context for an resource fetch */
ਬਣਤਰ fetch_resource_context {
	ਬਣਤਰ fetch_resource_context *r_ਅਗਲਾ, *r_prev;

	ਬਣਤਰ fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	ਬਣਤਰ fetch_resource_map_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

ਸਥਿਰ ਬਣਤਰ fetch_resource_context *ring = ਨਿਰਾ;

ਸਥਿਰ uint32_t fetch_resource_ਰਸਤਾ_ਗਿਣਤੀ;

/** issue fetch callbacks with locking */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool fetch_resource_send_callback(ਅਚਲ fetch_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ,
		ਬਣਤਰ fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ਸੁਨੇਹਾ, ctx->fetchh);
	ctx->locked = false;

	ਵਾਪਸ ctx->aborted;
}

ਸਥਿਰ bool fetch_resource_send_ਸਿਰਲੇਖ(ਬਣਤਰ fetch_resource_context *ctx,
		ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	ਅੱਖਰ ਸਿਰਲੇਖ[64];
	va_list ap;
	ਪੂਰਨ len;

	va_start(ap, fmt);
	len = vsnprintf(ਸਿਰਲੇਖ, ਆਕਾਰ ਸਿਰਲੇਖ, fmt, ap);
	va_end(ap);

	ਜੇ (len >= (ਪੂਰਨ)ਆਕਾਰ(ਸਿਰਲੇਖ) || len < 0) {
		ਵਾਪਸ false;
	}

	ਸੁਨੇਹਾ.type = FETCH_ਸਿਰਲੇਖ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *) ਸਿਰਲੇਖ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = len;

	ਵਾਪਸ fetch_resource_send_callback(&ਸੁਨੇਹਾ, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
ਸਥਿਰ bool fetch_resource_redirect_handler(ਬਣਤਰ fetch_resource_context *ctx)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;

	/* content is going to return redirect */
	fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, 302);

	ਸੁਨੇਹਾ.type = FETCH_REDIRECT;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.redirect = nsurl_access(ctx->ਇੰਦਰਾਜ->redirect_url);
	fetch_resource_send_callback(&ਸੁਨੇਹਾ, ctx);

	ਵਾਪਸ true;
}

/* resource handler that returns data directly */
ਸਥਿਰ bool fetch_resource_ਅੰਕੜੇ_handler(ਬਣਤਰ fetch_resource_context *ctx)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;

	/* Check if we can just return not modified */
	ਜੇ (ctx->etag != 0 && ctx->etag == ਸਿੱਧਾ_ETAG_ਮੁੱਲ) {
		fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, 304);
		ਸੁਨੇਹਾ.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&ਸੁਨੇਹਾ, ctx);
		ਵਾਪਸ true;
	}

	/* fetch is going to be successful */
	fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ਜੇ (fetch_resource_send_ਸਿਰਲੇਖ(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_ਅੰਕੜੇ(ctx->ਇੰਦਰਾਜ->ਰਸਤਾ)))) {
		ਜਾਓ fetch_resource_ਅੰਕੜੇ_aborted;
	}

	/* content length */
	ਜੇ (fetch_resource_send_ਸਿਰਲੇਖ(ctx, "Content-Length: %" PRIsizet,
				       ctx->ਇੰਦਰਾਜ->ਅੰਕੜੇ_len)) {
		ਜਾਓ fetch_resource_ਅੰਕੜੇ_aborted;
	}

	/* create etag */
	ਜੇ (fetch_resource_send_ਸਿਰਲੇਖ(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) ਸਿੱਧਾ_ETAG_ਮੁੱਲ)) {
		ਜਾਓ fetch_resource_ਅੰਕੜੇ_aborted;
	}

	/* create max-age of 1 year */
	ਜੇ (fetch_resource_send_ਸਿਰਲੇਖ(ctx,
			"Cache-Control: max-age=31536000")) {
		ਜਾਓ fetch_resource_ਅੰਕੜੇ_aborted;
	}

	ਸੁਨੇਹਾ.type = FETCH_ਅੰਕੜੇ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *) ctx->ਇੰਦਰਾਜ->ਅੰਕੜੇ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ctx->ਇੰਦਰਾਜ->ਅੰਕੜੇ_len;
	fetch_resource_send_callback(&ਸੁਨੇਹਾ, ctx);

	ਜੇ (ctx->aborted == false) {
		ਸੁਨੇਹਾ.type = FETCH_ਮੁਕੰਮਲ;
		fetch_resource_send_callback(&ਸੁਨੇਹਾ, ctx);
	}

fetch_resource_ਅੰਕੜੇ_aborted:

	ਵਾਪਸ true;
}

ਸਥਿਰ bool fetch_resource_notfound_handler(ਬਣਤਰ fetch_resource_context *ctx)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	ਪੂਰਨ ਕੋਡ = 404;
	ਅੱਖਰ ਬਫਰ[1024];
	ਅਚਲ ਅੱਖਰ *title;
	ਅੱਖਰ ਕੁੰਜੀ[8];

	/* content is going to return error code */
	fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, ਕੋਡ);

	/* content type */
	ਜੇ (fetch_resource_send_ਸਿਰਲੇਖ(ctx, "Content-Type: text/html; charset=utf-8"))
		ਜਾਓ fetch_resource_notfound_handler_aborted;

	snprintf(ਕੁੰਜੀ, ਆਕਾਰ ਕੁੰਜੀ, "HTTP%03d", ਕੋਡ);
	title = messages_get(ਕੁੰਜੀ);

	snprintf(ਬਫਰ, ਆਕਾਰ ਬਫਰ,
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
		 messages_get("FetchErrorCode"), ਕੋਡ,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	ਸੁਨੇਹਾ.type = FETCH_ਅੰਕੜੇ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *) ਬਫਰ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਲੜੀ_ਲੰਬਾਈ(ਬਫਰ);
	ਜੇ (fetch_resource_send_callback(&ਸੁਨੇਹਾ, ctx))
		ਜਾਓ fetch_resource_notfound_handler_aborted;

	ਸੁਨੇਹਾ.type = FETCH_ਮੁਕੰਮਲ;
	fetch_resource_send_callback(&ਸੁਨੇਹਾ, ctx);

fetch_resource_notfound_handler_aborted:
	ਵਾਪਸ false;
}



/** callback to initialise the resource fetcher. */
ਸਥਿਰ bool fetch_resource_initialise(lwc_string *scheme)
{
	ਬਣਤਰ fetch_resource_map_ਇੰਦਰਾਜ *e;
	uint32_t i;
	nserror res;

	fetch_resource_ਰਸਤਾ_ਗਿਣਤੀ = 0;

	ਲਈ (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_ਰਸਤਾ_ਗਿਣਤੀ];

		ਜੇ (lwc_intern_string(fetch_resource_paths[i],
				ਲੜੀ_ਲੰਬਾਈ(fetch_resource_paths[i]),
				&e->ਰਸਤਾ) != lwc_error_ok) {
			ਜਦੋਂ (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].ਰਸਤਾ);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->ਅੰਕੜੇ = ਨਿਰਾ;
		res = guit->fetch->get_resource_ਅੰਕੜੇ(lwc_string_ਅੰਕੜੇ(e->ਰਸਤਾ),
						     &e->ਅੰਕੜੇ,
						     &e->ਅੰਕੜੇ_len);
		ਜੇ (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_ਰਸਤਾ_ਗਿਣਤੀ++;
		} ਨਹੀਂਤਾਂ {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			ਜੇ (e->redirect_url == ਨਿਰਾ) {
				lwc_string_unref(e->ਰਸਤਾ);
			} ਨਹੀਂਤਾਂ {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_ਰਸਤਾ_ਗਿਣਤੀ++;
			}
		}
	}

	ਵਾਪਸ true;
}

/** callback to finalise the resource fetcher. */
ਸਥਿਰ ਖਾਲੀ fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	ਲਈ (i = 0; i < fetch_resource_ਰਸਤਾ_ਗਿਣਤੀ; i++) {
		lwc_string_unref(fetch_resource_map[i].ਰਸਤਾ);
		ਜੇ (fetch_resource_map[i].ਅੰਕੜੇ != ਨਿਰਾ) {
			guit->fetch->release_resource_ਅੰਕੜੇ(fetch_resource_map[i].ਅੰਕੜੇ);
		} ਨਹੀਂਤਾਂ {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

ਸਥਿਰ bool fetch_resource_can_fetch(ਅਚਲ nsurl *url)
{
	ਵਾਪਸ true;
}

/**
 * set up a resource fetch context.
 */
ਸਥਿਰ ਖਾਲੀ *
fetch_resource_ਤਿਆਰੀ(ਬਣਤਰ fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     ਅਚਲ ਅੱਖਰ *post_urlenc,
		     ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *post_multipart,
		     ਅਚਲ ਅੱਖਰ **ਸਿਰਲੇਖਾਂ)
{
	ਬਣਤਰ fetch_resource_context *ctx;
	lwc_string *ਰਸਤਾ;
	nserror ret;
	uint32_t i;

	ctx = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*ctx));
	ਜੇ (ctx == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ctx->handler = fetch_resource_notfound_handler;

	ਜੇ ((ਰਸਤਾ = nsurl_get_component(url, NSURL_ਰਸਤਾ)) != ਨਿਰਾ) {
		bool ਮੇਲ;

		/* Ensure requested path is valid */
		ਲਈ (i = 0; i < fetch_resource_ਰਸਤਾ_ਗਿਣਤੀ; i++) {
			ਜੇ (lwc_string_isequal(ਰਸਤਾ,
					fetch_resource_map[i].ਰਸਤਾ,
					&ਮੇਲ) == lwc_error_ok && ਮੇਲ) {
				/* found a url match, select handler */
				ctx->ਇੰਦਰਾਜ = &fetch_resource_map[i];
				ਜੇ (ctx->ਇੰਦਰਾਜ->ਅੰਕੜੇ != ਨਿਰਾ) {
					ctx->handler = fetch_resource_ਅੰਕੜੇ_handler;
				} ਨਹੀਂਤਾਂ {
					ctx->handler = fetch_resource_redirect_handler;
				}
				ਰੋਕੋ;
			}
		}

		lwc_string_unref(ਰਸਤਾ);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	ਲਈ (i = 0; ਸਿਰਲੇਖਾਂ[i] != ਨਿਰਾ; i++) {
		ਜੇ (strncasecmp(ਸਿਰਲੇਖਾਂ[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			ਜਾਰੀ;
		}

		/* If-None-Match: "12345678" */
		ਅਚਲ ਅੱਖਰ *d = ਸਿਰਲੇਖਾਂ[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		ਜਦੋਂ (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		ਜੇ (*d != '\0') {
			ret = nsc_snptimet(d, ਲੜੀ_ਲੰਬਾਈ(d), &ctx->etag);
			ਜੇ (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_ਪਾਓ(ring, ctx);

	ਵਾਪਸ ctx;
}

/** callback to free a resource fetch */
ਸਥਿਰ ਖਾਲੀ fetch_resource_free(ਖਾਲੀ *ctx)
{
	ਬਣਤਰ fetch_resource_context *c = ctx;
	ਜੇ (c->url != ਨਿਰਾ)
		nsurl_unref(c->url);
	ਮੁਕਤ(ctx);
}

/** callback to start a resource fetch */
ਸਥਿਰ bool fetch_resource_ਸ਼ੁਰੂ(ਖਾਲੀ *ctx)
{
	ਵਾਪਸ true;
}

/** callback to abort a resource fetch */
ਸਥਿਰ ਖਾਲੀ fetch_resource_abort(ਖਾਲੀ *ctx)
{
	ਬਣਤਰ fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ਸਥਿਰ ਖਾਲੀ fetch_resource_poll(lwc_string *scheme)
{
	ਬਣਤਰ fetch_resource_context *c, *save_ring = ਨਿਰਾ;

	ਜਦੋਂ (ring != ਨਿਰਾ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ਜੇ (c->locked == true) {
			RING_ਪਾਓ(save_ring, c);
			ਜਾਰੀ;
		}

		/* Only process non-aborted fetches */
		ਜੇ (c->aborted == false) {
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

nserror fetch_resource_ਰਜਿਸਟਰ(ਖਾਲੀ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	ਅਚਲ ਬਣਤਰ fetcher_operation_ਸਾਰਣੀ fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.ਤਿਆਰੀ = fetch_resource_ਤਿਆਰੀ,
		.ਸ਼ੁਰੂ = fetch_resource_ਸ਼ੁਰੂ,
		.abort = fetch_resource_abort,
		.ਮੁਕਤ = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	ਵਾਪਸ fetcher_ਜੋੜੋ(scheme, &fetcher_ops);
}
