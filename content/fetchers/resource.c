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

#ವ್ಯಾಖ್ಯಾನಿಸು ನೇರ_ETAG_ಮೌಲ್ಯ 123456

/** Valid resource paths */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fetch_resource_paths[] = {
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
ಸ್ಥಿರ ರಚನೆ fetch_resource_map_ನಮೂದು {
	lwc_string *ಮಾರ್ಗ; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ; /**< direct pointer to data */
	size_t ದತ್ತಾಂಶ_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

ರಚನೆ fetch_resource_context;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ bool (*fetch_resource_handler)(ರಚನೆ fetch_resource_context *);

/** Context for an resource fetch */
ರಚನೆ fetch_resource_context {
	ರಚನೆ fetch_resource_context *r_ಮುಂದಿನ, *r_prev;

	ರಚನೆ fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	ರಚನೆ fetch_resource_map_ನಮೂದು *ನಮೂದು; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

ಸ್ಥಿರ ರಚನೆ fetch_resource_context *ring = ಖಾಲಿ;

ಸ್ಥಿರ uint32_t fetch_resource_ಮಾರ್ಗ_ಎಣಿಕೆ;

/** issue fetch callbacks with locking */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool fetch_resource_send_callback(ಸ್ಥಿರಾಂಕ fetch_ಸಂದೇಶ *ಸಂದೇಶ,
		ರಚನೆ fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ಸಂದೇಶ, ctx->fetchh);
	ctx->locked = false;

	ಹಿಂತಿರುಗಿ ctx->aborted;
}

ಸ್ಥಿರ bool fetch_resource_send_ಶೀರ್ಷಿಕೆ(ರಚನೆ fetch_resource_context *ctx,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಅಕ್ಷರ ಶೀರ್ಷಿಕೆ[64];
	va_list ap;
	ಪೂರ್ಣಾಂಕ len;

	va_start(ap, fmt);
	len = vsnprintf(ಶೀರ್ಷಿಕೆ, ಗಾತ್ರ ಶೀರ್ಷಿಕೆ, fmt, ap);
	va_end(ap);

	ಆದರೆ (len >= (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(ಶೀರ್ಷಿಕೆ) || len < 0) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಸಂದೇಶ.type = FETCH_ಶೀರ್ಷಿಕೆ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *) ಶೀರ್ಷಿಕೆ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = len;

	ಹಿಂತಿರುಗಿ fetch_resource_send_callback(&ಸಂದೇಶ, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
ಸ್ಥಿರ bool fetch_resource_redirect_handler(ರಚನೆ fetch_resource_context *ctx)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	/* content is going to return redirect */
	fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, 302);

	ಸಂದೇಶ.type = FETCH_REDIRECT;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.redirect = nsurl_access(ctx->ನಮೂದು->redirect_url);
	fetch_resource_send_callback(&ಸಂದೇಶ, ctx);

	ಹಿಂತಿರುಗಿ true;
}

/* resource handler that returns data directly */
ಸ್ಥಿರ bool fetch_resource_ದತ್ತಾಂಶ_handler(ರಚನೆ fetch_resource_context *ctx)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	/* Check if we can just return not modified */
	ಆದರೆ (ctx->etag != 0 && ctx->etag == ನೇರ_ETAG_ಮೌಲ್ಯ) {
		fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, 304);
		ಸಂದೇಶ.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&ಸಂದೇಶ, ctx);
		ಹಿಂತಿರುಗಿ true;
	}

	/* fetch is going to be successful */
	fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ಆದರೆ (fetch_resource_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_ದತ್ತಾಂಶ(ctx->ನಮೂದು->ಮಾರ್ಗ)))) {
		ಹೋಗು fetch_resource_ದತ್ತಾಂಶ_aborted;
	}

	/* content length */
	ಆದರೆ (fetch_resource_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Length: %" PRIsizet,
				       ctx->ನಮೂದು->ದತ್ತಾಂಶ_len)) {
		ಹೋಗು fetch_resource_ದತ್ತಾಂಶ_aborted;
	}

	/* create etag */
	ಆದರೆ (fetch_resource_send_ಶೀರ್ಷಿಕೆ(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) ನೇರ_ETAG_ಮೌಲ್ಯ)) {
		ಹೋಗು fetch_resource_ದತ್ತಾಂಶ_aborted;
	}

	/* create max-age of 1 year */
	ಆದರೆ (fetch_resource_send_ಶೀರ್ಷಿಕೆ(ctx,
			"Cache-Control: max-age=31536000")) {
		ಹೋಗು fetch_resource_ದತ್ತಾಂಶ_aborted;
	}

	ಸಂದೇಶ.type = FETCH_ದತ್ತಾಂಶ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *) ctx->ನಮೂದು->ದತ್ತಾಂಶ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ctx->ನಮೂದು->ದತ್ತಾಂಶ_len;
	fetch_resource_send_callback(&ಸಂದೇಶ, ctx);

	ಆದರೆ (ctx->aborted == false) {
		ಸಂದೇಶ.type = FETCH_ಪೂರ್ಣಗೊಂಡ;
		fetch_resource_send_callback(&ಸಂದೇಶ, ctx);
	}

fetch_resource_ದತ್ತಾಂಶ_aborted:

	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ bool fetch_resource_notfound_handler(ರಚನೆ fetch_resource_context *ctx)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2 = 404;
	ಅಕ್ಷರ ಬಫರ್[1024];
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title;
	ಅಕ್ಷರ ಕೀಲಿ[8];

	/* content is going to return error code */
	fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, ಸಂಕೇತ_2);

	/* content type */
	ಆದರೆ (fetch_resource_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: text/html; charset=utf-8"))
		ಹೋಗು fetch_resource_notfound_handler_aborted;

	snprintf(ಕೀಲಿ, ಗಾತ್ರ ಕೀಲಿ, "HTTP%03d", ಸಂಕೇತ_2);
	title = messages_get(ಕೀಲಿ);

	snprintf(ಬಫರ್, ಗಾತ್ರ ಬಫರ್,
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
		 messages_get("FetchErrorCode"), ಸಂಕೇತ_2,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	ಸಂದೇಶ.type = FETCH_ದತ್ತಾಂಶ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *) ಬಫರ್;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಬಫರ್);
	ಆದರೆ (fetch_resource_send_callback(&ಸಂದೇಶ, ctx))
		ಹೋಗು fetch_resource_notfound_handler_aborted;

	ಸಂದೇಶ.type = FETCH_ಪೂರ್ಣಗೊಂಡ;
	fetch_resource_send_callback(&ಸಂದೇಶ, ctx);

fetch_resource_notfound_handler_aborted:
	ಹಿಂತಿರುಗಿ false;
}



/** callback to initialise the resource fetcher. */
ಸ್ಥಿರ bool fetch_resource_initialise(lwc_string *scheme)
{
	ರಚನೆ fetch_resource_map_ನಮೂದು *e;
	uint32_t i;
	nserror res;

	fetch_resource_ಮಾರ್ಗ_ಎಣಿಕೆ = 0;

	ಗಾಗಿ (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_ಮಾರ್ಗ_ಎಣಿಕೆ];

		ಆದರೆ (lwc_intern_string(fetch_resource_paths[i],
				ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(fetch_resource_paths[i]),
				&e->ಮಾರ್ಗ) != lwc_error_ok) {
			ಆಗ (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].ಮಾರ್ಗ);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->ದತ್ತಾಂಶ = ಖಾಲಿ;
		res = guit->fetch->get_resource_ದತ್ತಾಂಶ(lwc_string_ದತ್ತಾಂಶ(e->ಮಾರ್ಗ),
						     &e->ದತ್ತಾಂಶ,
						     &e->ದತ್ತಾಂಶ_len);
		ಆದರೆ (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_ಮಾರ್ಗ_ಎಣಿಕೆ++;
		} ಇಲ್ಲದಿದ್ದರೆ {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			ಆದರೆ (e->redirect_url == ಖಾಲಿ) {
				lwc_string_unref(e->ಮಾರ್ಗ);
			} ಇಲ್ಲದಿದ್ದರೆ {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_ಮಾರ್ಗ_ಎಣಿಕೆ++;
			}
		}
	}

	ಹಿಂತಿರುಗಿ true;
}

/** callback to finalise the resource fetcher. */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	ಗಾಗಿ (i = 0; i < fetch_resource_ಮಾರ್ಗ_ಎಣಿಕೆ; i++) {
		lwc_string_unref(fetch_resource_map[i].ಮಾರ್ಗ);
		ಆದರೆ (fetch_resource_map[i].ದತ್ತಾಂಶ != ಖಾಲಿ) {
			guit->fetch->release_resource_ದತ್ತಾಂಶ(fetch_resource_map[i].ದತ್ತಾಂಶ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

ಸ್ಥಿರ bool fetch_resource_can_fetch(ಸ್ಥಿರಾಂಕ nsurl *url)
{
	ಹಿಂತಿರುಗಿ true;
}

/**
 * set up a resource fetch context.
 */
ಸ್ಥಿರ ಶೂನ್ಯ *
fetch_resource_ಸಿದ್ಧತೆ(ರಚನೆ fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *post_urlenc,
		     ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *post_multipart,
		     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಶೀರ್ಷಿಕೆಗಳು)
{
	ರಚನೆ fetch_resource_context *ctx;
	lwc_string *ಮಾರ್ಗ;
	nserror ret;
	uint32_t i;

	ctx = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*ctx));
	ಆದರೆ (ctx == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ctx->handler = fetch_resource_notfound_handler;

	ಆದರೆ ((ಮಾರ್ಗ = nsurl_get_component(url, NSURL_ಮಾರ್ಗ)) != ಖಾಲಿ) {
		bool ಹೊಂದಿಕೆ;

		/* Ensure requested path is valid */
		ಗಾಗಿ (i = 0; i < fetch_resource_ಮಾರ್ಗ_ಎಣಿಕೆ; i++) {
			ಆದರೆ (lwc_string_isequal(ಮಾರ್ಗ,
					fetch_resource_map[i].ಮಾರ್ಗ,
					&ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ) {
				/* found a url match, select handler */
				ctx->ನಮೂದು = &fetch_resource_map[i];
				ಆದರೆ (ctx->ನಮೂದು->ದತ್ತಾಂಶ != ಖಾಲಿ) {
					ctx->handler = fetch_resource_ದತ್ತಾಂಶ_handler;
				} ಇಲ್ಲದಿದ್ದರೆ {
					ctx->handler = fetch_resource_redirect_handler;
				}
				ನಿಲ್ಲಿಸು;
			}
		}

		lwc_string_unref(ಮಾರ್ಗ);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	ಗಾಗಿ (i = 0; ಶೀರ್ಷಿಕೆಗಳು[i] != ಖಾಲಿ; i++) {
		ಆದರೆ (strncasecmp(ಶೀರ್ಷಿಕೆಗಳು[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			ಮುಂದುವರಿ;
		}

		/* If-None-Match: "12345678" */
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *d = ಶೀರ್ಷಿಕೆಗಳು[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		ಆಗ (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		ಆದರೆ (*d != '\0') {
			ret = nsc_snptimet(d, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(d), &ctx->etag);
			ಆದರೆ (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_ಸೇರಿಸು(ring, ctx);

	ಹಿಂತಿರುಗಿ ctx;
}

/** callback to free a resource fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_resource_free(ಶೂನ್ಯ *ctx)
{
	ರಚನೆ fetch_resource_context *c = ctx;
	ಆದರೆ (c->url != ಖಾಲಿ)
		nsurl_unref(c->url);
	ಬಿಡುಗಡೆ(ctx);
}

/** callback to start a resource fetch */
ಸ್ಥಿರ bool fetch_resource_ಆರಂಭ(ಶೂನ್ಯ *ctx)
{
	ಹಿಂತಿರುಗಿ true;
}

/** callback to abort a resource fetch */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_resource_abort(ಶೂನ್ಯ *ctx)
{
	ರಚನೆ fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_resource_poll(lwc_string *scheme)
{
	ರಚನೆ fetch_resource_context *c, *save_ring = ಖಾಲಿ;

	ಆಗ (ring != ಖಾಲಿ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ಆದರೆ (c->locked == true) {
			RING_ಸೇರಿಸು(save_ring, c);
			ಮುಂದುವರಿ;
		}

		/* Only process non-aborted fetches */
		ಆದರೆ (c->aborted == false) {
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

nserror fetch_resource_ರಿಜಿಸ್ಟರ್(ಶೂನ್ಯ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	ಸ್ಥಿರಾಂಕ ರಚನೆ fetcher_operation_ಕೋಷ್ಟಕ fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.ಸಿದ್ಧತೆ = fetch_resource_ಸಿದ್ಧತೆ,
		.ಆರಂಭ = fetch_resource_ಆರಂಭ,
		.abort = fetch_resource_abort,
		.ಬಿಡುಗಡೆ = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	ಹಿಂತಿರುಗಿ fetcher_ಕೂಡಿಸು(scheme, &fetcher_ops);
}
