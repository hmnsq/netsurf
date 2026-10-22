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

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <libwapcaplet/libwapcaplet.h>

#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/time.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/ring.h"
#అన్తర్భావ "netsurf/fetch.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "content/fetch.h"
#అన్తర్భావ "content/fetchers.h"
#అన్తర్భావ "content/fetchers/resource.h"

#నిర్వచించు ప్రత్యక్ష_ETAG_విలువ 123456

/** Valid resource paths */
స్థిర స్థిరరాశి అక్షరం *fetch_resource_paths[] = {
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
స్థిర నిర్మాణం fetch_resource_map_నమోదు {
	lwc_string *మార్గం; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	స్థిరరాశి uint8_t *దత్తాంశం; /**< direct pointer to data */
	size_t దత్తాంశం_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

నిర్మాణం fetch_resource_context;

రకనిర్వచనం bool (*fetch_resource_handler)(నిర్మాణం fetch_resource_context *);

/** Context for an resource fetch */
నిర్మాణం fetch_resource_context {
	నిర్మాణం fetch_resource_context *r_తదుపరి, *r_prev;

	నిర్మాణం fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	నిర్మాణం fetch_resource_map_నమోదు *నమోదు_2; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

స్థిర నిర్మాణం fetch_resource_context *ring = ఖాళీ;

స్థిర uint32_t fetch_resource_మార్గం_లెక్క;

/** issue fetch callbacks with locking */
స్థిర అంతఃవరుస bool fetch_resource_send_callback(స్థిరరాశి fetch_సందేశం *సందేశం,
		నిర్మాణం fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(సందేశం, ctx->fetchh);
	ctx->locked = false;

	తిరిగి ctx->aborted;
}

స్థిర bool fetch_resource_send_శీర్షిక(నిర్మాణం fetch_resource_context *ctx,
		స్థిరరాశి అక్షరం *fmt, ...)
{
	fetch_సందేశం సందేశం;
	అక్షరం శీర్షిక[64];
	va_list ap;
	పూర్ణాంకం len;

	va_start(ap, fmt);
	len = vsnprintf(శీర్షిక, పరిమాణం శీర్షిక, fmt, ap);
	va_end(ap);

	ఒకవేళ (len >= (పూర్ణాంకం)పరిమాణం(శీర్షిక) || len < 0) {
		తిరిగి false;
	}

	సందేశం.type = FETCH_శీర్షిక;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *) శీర్షిక;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = len;

	తిరిగి fetch_resource_send_callback(&సందేశం, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
స్థిర bool fetch_resource_redirect_handler(నిర్మాణం fetch_resource_context *ctx)
{
	fetch_సందేశం సందేశం;

	/* content is going to return redirect */
	fetch_సమితి_http_సంకేతం(ctx->fetchh, 302);

	సందేశం.type = FETCH_REDIRECT;
	సందేశం.దత్తాంశం.redirect = nsurl_access(ctx->నమోదు_2->redirect_url);
	fetch_resource_send_callback(&సందేశం, ctx);

	తిరిగి true;
}

/* resource handler that returns data directly */
స్థిర bool fetch_resource_దత్తాంశం_handler(నిర్మాణం fetch_resource_context *ctx)
{
	fetch_సందేశం సందేశం;

	/* Check if we can just return not modified */
	ఒకవేళ (ctx->etag != 0 && ctx->etag == ప్రత్యక్ష_ETAG_విలువ) {
		fetch_సమితి_http_సంకేతం(ctx->fetchh, 304);
		సందేశం.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&సందేశం, ctx);
		తిరిగి true;
	}

	/* fetch is going to be successful */
	fetch_సమితి_http_సంకేతం(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	ఒకవేళ (fetch_resource_send_శీర్షిక(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_దత్తాంశం(ctx->నమోదు_2->మార్గం)))) {
		వెళ్ళు fetch_resource_దత్తాంశం_aborted;
	}

	/* content length */
	ఒకవేళ (fetch_resource_send_శీర్షిక(ctx, "Content-Length: %" PRIsizet,
				       ctx->నమోదు_2->దత్తాంశం_len)) {
		వెళ్ళు fetch_resource_దత్తాంశం_aborted;
	}

	/* create etag */
	ఒకవేళ (fetch_resource_send_శీర్షిక(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) ప్రత్యక్ష_ETAG_విలువ)) {
		వెళ్ళు fetch_resource_దత్తాంశం_aborted;
	}

	/* create max-age of 1 year */
	ఒకవేళ (fetch_resource_send_శీర్షిక(ctx,
			"Cache-Control: max-age=31536000")) {
		వెళ్ళు fetch_resource_దత్తాంశం_aborted;
	}

	సందేశం.type = FETCH_దత్తాంశం;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *) ctx->నమోదు_2->దత్తాంశం;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = ctx->నమోదు_2->దత్తాంశం_len;
	fetch_resource_send_callback(&సందేశం, ctx);

	ఒకవేళ (ctx->aborted == false) {
		సందేశం.type = FETCH_ముగిసిన;
		fetch_resource_send_callback(&సందేశం, ctx);
	}

fetch_resource_దత్తాంశం_aborted:

	తిరిగి true;
}

స్థిర bool fetch_resource_notfound_handler(నిర్మాణం fetch_resource_context *ctx)
{
	fetch_సందేశం సందేశం;
	పూర్ణాంకం సంకేతం_2 = 404;
	అక్షరం బఫరు[1024];
	స్థిరరాశి అక్షరం *title;
	అక్షరం కీలకం[8];

	/* content is going to return error code */
	fetch_సమితి_http_సంకేతం(ctx->fetchh, సంకేతం_2);

	/* content type */
	ఒకవేళ (fetch_resource_send_శీర్షిక(ctx, "Content-Type: text/html; charset=utf-8"))
		వెళ్ళు fetch_resource_notfound_handler_aborted;

	snprintf(కీలకం, పరిమాణం కీలకం, "HTTP%03d", సంకేతం_2);
	title = messages_get(కీలకం);

	snprintf(బఫరు, పరిమాణం బఫరు,
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
		 messages_get("FetchErrorCode"), సంకేతం_2,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	సందేశం.type = FETCH_దత్తాంశం;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *) బఫరు;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = అక్షరశ్రేణి_పొడవు(బఫరు);
	ఒకవేళ (fetch_resource_send_callback(&సందేశం, ctx))
		వెళ్ళు fetch_resource_notfound_handler_aborted;

	సందేశం.type = FETCH_ముగిసిన;
	fetch_resource_send_callback(&సందేశం, ctx);

fetch_resource_notfound_handler_aborted:
	తిరిగి false;
}



/** callback to initialise the resource fetcher. */
స్థిర bool fetch_resource_initialise(lwc_string *scheme)
{
	నిర్మాణం fetch_resource_map_నమోదు *e;
	uint32_t i;
	nserror res;

	fetch_resource_మార్గం_లెక్క = 0;

	కొరకు (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_మార్గం_లెక్క];

		ఒకవేళ (lwc_intern_string(fetch_resource_paths[i],
				అక్షరశ్రేణి_పొడవు(fetch_resource_paths[i]),
				&e->మార్గం) != lwc_error_ok) {
			అయితే (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].మార్గం);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->దత్తాంశం = ఖాళీ;
		res = guit->fetch->get_resource_దత్తాంశం(lwc_string_దత్తాంశం(e->మార్గం),
						     &e->దత్తాంశం,
						     &e->దత్తాంశం_len);
		ఒకవేళ (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_మార్గం_లెక్క++;
		} లేకపోతే {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			ఒకవేళ (e->redirect_url == ఖాళీ) {
				lwc_string_unref(e->మార్గం);
			} లేకపోతే {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_మార్గం_లెక్క++;
			}
		}
	}

	తిరిగి true;
}

/** callback to finalise the resource fetcher. */
స్థిర శూన్య fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	కొరకు (i = 0; i < fetch_resource_మార్గం_లెక్క; i++) {
		lwc_string_unref(fetch_resource_map[i].మార్గం);
		ఒకవేళ (fetch_resource_map[i].దత్తాంశం != ఖాళీ) {
			guit->fetch->release_resource_దత్తాంశం(fetch_resource_map[i].దత్తాంశం);
		} లేకపోతే {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

స్థిర bool fetch_resource_can_fetch(స్థిరరాశి nsurl *url)
{
	తిరిగి true;
}

/**
 * set up a resource fetch context.
 */
స్థిర శూన్య *
fetch_resource_అమరిక(నిర్మాణం fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     స్థిరరాశి అక్షరం *post_urlenc,
		     స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *post_multipart,
		     స్థిరరాశి అక్షరం **శీర్షికలు)
{
	నిర్మాణం fetch_resource_context *ctx;
	lwc_string *మార్గం;
	nserror ret;
	uint32_t i;

	ctx = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*ctx));
	ఒకవేళ (ctx == ఖాళీ) {
		తిరిగి ఖాళీ;
	}

	ctx->handler = fetch_resource_notfound_handler;

	ఒకవేళ ((మార్గం = nsurl_get_component(url, NSURL_మార్గం)) != ఖాళీ) {
		bool సరిపోలిక;

		/* Ensure requested path is valid */
		కొరకు (i = 0; i < fetch_resource_మార్గం_లెక్క; i++) {
			ఒకవేళ (lwc_string_isequal(మార్గం,
					fetch_resource_map[i].మార్గం,
					&సరిపోలిక) == lwc_error_ok && సరిపోలిక) {
				/* found a url match, select handler */
				ctx->నమోదు_2 = &fetch_resource_map[i];
				ఒకవేళ (ctx->నమోదు_2->దత్తాంశం != ఖాళీ) {
					ctx->handler = fetch_resource_దత్తాంశం_handler;
				} లేకపోతే {
					ctx->handler = fetch_resource_redirect_handler;
				}
				ఆపు;
			}
		}

		lwc_string_unref(మార్గం);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	కొరకు (i = 0; శీర్షికలు[i] != ఖాళీ; i++) {
		ఒకవేళ (strncasecmp(శీర్షికలు[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			కొనసాగు;
		}

		/* If-None-Match: "12345678" */
		స్థిరరాశి అక్షరం *d = శీర్షికలు[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		అయితే (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		ఒకవేళ (*d != '\0') {
			ret = nsc_snptimet(d, అక్షరశ్రేణి_పొడవు(d), &ctx->etag);
			ఒకవేళ (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_చొప్పించు(ring, ctx);

	తిరిగి ctx;
}

/** callback to free a resource fetch */
స్థిర శూన్య fetch_resource_free(శూన్య *ctx)
{
	నిర్మాణం fetch_resource_context *c = ctx;
	ఒకవేళ (c->url != ఖాళీ)
		nsurl_unref(c->url);
	విడుదల(ctx);
}

/** callback to start a resource fetch */
స్థిర bool fetch_resource_ప్రారంభం(శూన్య *ctx)
{
	తిరిగి true;
}

/** callback to abort a resource fetch */
స్థిర శూన్య fetch_resource_abort(శూన్య *ctx)
{
	నిర్మాణం fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
స్థిర శూన్య fetch_resource_poll(lwc_string *scheme)
{
	నిర్మాణం fetch_resource_context *c, *save_ring = ఖాళీ;

	అయితే (ring != ఖాళీ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ఒకవేళ (c->locked == true) {
			RING_చొప్పించు(save_ring, c);
			కొనసాగు;
		}

		/* Only process non-aborted fetches */
		ఒకవేళ (c->aborted == false) {
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

nserror fetch_resource_రిజిస్టరు(శూన్య)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	స్థిరరాశి నిర్మాణం fetcher_operation_పట్టిక fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.అమరిక = fetch_resource_అమరిక,
		.ప్రారంభం = fetch_resource_ప్రారంభం,
		.abort = fetch_resource_abort,
		.విడుదల = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	తిరిగి fetcher_కూడు(scheme, &fetcher_ops);
}
