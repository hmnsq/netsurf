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
 *
 * URL handling for the "about" scheme.
 *
 * Based on the data fetcher by Rob Kendrick
 * This fetcher provides a simple scheme for the user to access
 * information from the browser from a known, fixed URL.
 */

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "netsurf/inttypes.h"

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/ring.h"
#అన్తర్భావ "utils/messages.h"

#అన్తర్భావ "content/fetch.h"
#అన్తర్భావ "content/fetchers.h"

#అన్తర్భావ "private.h"
#అన్తర్భావ "about.h"
#అన్తర్భావ "blank.h"
#అన్తర్భావ "certificate.h"
#అన్తర్భావ "config.h"
#అన్తర్భావ "chart.h"
#అన్తర్భావ "choices.h"
#అన్తర్భావ "imagecache.h"
#అన్తర్భావ "nscolours.h"
#అన్తర్భావ "query.h"
#అన్తర్భావ "query_auth.h"
#అన్తర్భావ "query_fetcherror.h"
#అన్తర్భావ "query_privacy.h"
#అన్తర్భావ "query_timeout.h"
#అన్తర్భావ "atestament.h"
#అన్తర్భావ "websearch.h"

రకనిర్వచనం bool (*fetch_about_handler)(నిర్మాణం fetch_about_context *);

/**
 * Context for an about fetch
 */
నిర్మాణం fetch_about_context {
	నిర్మాణం fetch_about_context *r_తదుపరి, *r_prev;

	నిర్మాణం fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */

	స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *multipart; /**< post data */

	fetch_about_handler handler;
};

స్థిర నిర్మాణం fetch_about_context *ring = ఖాళీ;

/**
 * handler info for about scheme
 */
నిర్మాణం about_handlers {
	స్థిరరాశి అక్షరం *name; /**< name to match in url */
	పూర్ణాంకం name_len;
	lwc_string *lname; /**< Interned name */
	fetch_about_handler handler; /**< handler for the url */
	bool hidden; /**< If entry should be hidden in listing */
};



/**
 * issue fetch callbacks with locking
 */
స్థిర bool
fetch_about_send_callback(స్థిరరాశి fetch_సందేశం *సందేశం, నిర్మాణం fetch_about_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(సందేశం, ctx->fetchh);
	ctx->locked = false;

	తిరిగి ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_ముగిసిన(నిర్మాణం fetch_about_context *ctx)
{
	fetch_సందేశం సందేశం;
	సందేశం.type = FETCH_ముగిసిన;
	తిరిగి fetch_about_send_callback(&సందేశం, ctx);
}

/* exported interface documented in about/private.h */
bool fetch_about_సమితి_http_సంకేతం(నిర్మాణం fetch_about_context *ctx, దీర్ఘ సంకేతం_2)
{
	fetch_సమితి_http_సంకేతం(ctx->fetchh, సంకేతం_2);

	తిరిగి ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_శీర్షిక(నిర్మాణం fetch_about_context *ctx, స్థిరరాశి అక్షరం *fmt, ...)
{
	అక్షరం శీర్షిక[64];
	fetch_సందేశం సందేశం;
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(శీర్షిక, పరిమాణం శీర్షిక, fmt, ap);

	va_end(ap);

	సందేశం.type = FETCH_శీర్షిక;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *) శీర్షిక;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = అక్షరశ్రేణి_పొడవు(శీర్షిక);

	తిరిగి fetch_about_send_callback(&సందేశం, ctx);
}

/* exported interface documented in about/private.h */
nserror
fetch_about_senddata(నిర్మాణం fetch_about_context *ctx, స్థిరరాశి uint8_t *దత్తాంశం, size_t దత్తాంశం_len)
{
	fetch_సందేశం సందేశం;

	సందేశం.type = FETCH_దత్తాంశం;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = దత్తాంశం;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = దత్తాంశం_len;

	ఒకవేళ (fetch_about_send_callback(&సందేశం, ctx)) {
		తిరిగి NSERROR_INVALID;
	}

	తిరిగి NSERROR_OK;
}

/* exported interface documented in about/private.h */
nserror
fetch_about_ssenddataf(నిర్మాణం fetch_about_context *ctx, స్థిరరాశి అక్షరం *fmt, ...)
{
	అక్షరం బఫరు[1024];
	అక్షరం *dbuff;
	fetch_సందేశం సందేశం;
	va_list ap;
	పూర్ణాంకం slen;

	va_start(ap, fmt);

	slen = vsnprintf(బఫరు, పరిమాణం(బఫరు), fmt, ap);

	va_end(ap);

	ఒకవేళ (slen < (పూర్ణాంకం)పరిమాణం(బఫరు)) {
		సందేశం.type = FETCH_దత్తాంశం;
		సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *) బఫరు;
		సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = slen;

		ఒకవేళ (fetch_about_send_callback(&సందేశం, ctx)) {
			తిరిగి NSERROR_INVALID;
		}

		తిరిగి NSERROR_OK;
	}

	dbuff = జ్ఞాపకం_ఇవ్వు(slen + 1);
	ఒకవేళ (dbuff == ఖాళీ) {
		తిరిగి NSERROR_NOSPACE;
	}

	va_start(ap, fmt);

	slen = vsnprintf(dbuff, slen + 1, fmt, ap);

	va_end(ap);

	సందేశం.type = FETCH_దత్తాంశం;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf = (స్థిరరాశి uint8_t *)dbuff;
	సందేశం.దత్తాంశం.శీర్షిక_or_దత్తాంశం.len = slen;

	ఒకవేళ (fetch_about_send_callback(&సందేశం, ctx)) {
		విడుదల(dbuff);
		తిరిగి NSERROR_INVALID;
	}

	విడుదల(dbuff);
	తిరిగి NSERROR_OK;
}


/* exported interface documented in about/private.h */
nsurl *fetch_about_get_url(నిర్మాణం fetch_about_context *ctx)
{
	తిరిగి ctx->url;
}


/* exported interface documented in about/private.h */
స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *
fetch_about_get_multipart(నిర్మాణం fetch_about_context *ctx)
{
	తిరిగి ctx->multipart;
}


/* exported interface documented in about/private.h */
bool fetch_about_srverror(నిర్మాణం fetch_about_context *ctx)
{
	nserror res;

	fetch_సమితి_http_సంకేతం(ctx->fetchh, HTTP_RESPONSE_అంతర్గత_SERVER_ERROR);

	/* content type */
	ఒకవేళ (fetch_about_send_శీర్షిక(ctx, "Content-Type: text/plain"))
		తిరిగి false;

	res = fetch_about_ssenddataf(ctx, "%s 500 %s",
				     messages_get("FetchErrorCode"),
				     messages_get("HTTP500"));
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	fetch_about_send_ముగిసిన(ctx);

	తిరిగి true;
}


/* exported interface documented in about/private.h */
bool fetch_about_redirect(నిర్మాణం fetch_about_context *ctx, స్థిరరాశి అక్షరం *లక్ష్యం)
{
	fetch_సందేశం సందేశం;

	/* content is going to return redirect */
	fetch_సమితి_http_సంకేతం(ctx->fetchh, HTTP_RESPONSE_దొరికింది);

	సందేశం.type = FETCH_REDIRECT;
	సందేశం.దత్తాంశం.redirect = లక్ష్యం;

	తిరిగి fetch_about_send_callback(&సందేశం, ctx);
}

/**
 * Handler to generate about scheme credits page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
స్థిర bool fetch_about_credits_handler(నిర్మాణం fetch_about_context *ctx)
{
	తిరిగి fetch_about_redirect(ctx, "resource:credits.html");
}


/**
 * Handler to generate about scheme licence page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
స్థిర bool fetch_about_licence_handler(నిర్మాణం fetch_about_context *ctx)
{
	తిరిగి fetch_about_redirect(ctx, "resource:licence.html");
}


/**
 * Handler to generate about scheme logo page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
స్థిర bool fetch_about_logo_handler(నిర్మాణం fetch_about_context *ctx)
{
	తిరిగి fetch_about_redirect(ctx, "resource:netsurf.png");
}


/**
 * Handler to generate about scheme welcome page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
స్థిర bool fetch_about_welcome_handler(నిర్మాణం fetch_about_context *ctx)
{
	తిరిగి fetch_about_redirect(ctx, "resource:welcome.html");
}


/* Forward declaration because this handler requires the handler table. */
స్థిర bool fetch_about_about_handler(నిర్మాణం fetch_about_context *ctx);

/**
 * List of about paths and their handlers
 */
నిర్మాణం about_handlers about_handler_list[] = {
	{
		"credits",
		SLEN("credits"),
		ఖాళీ,
		fetch_about_credits_handler,
		false
	},
	{
		"licence",
		SLEN("licence"),
		ఖాళీ,
		fetch_about_licence_handler,
		false
	},
	{
		"license",
		SLEN("license"),
		ఖాళీ,
		fetch_about_licence_handler,
		true
	},
	{
		"welcome",
		SLEN("welcome"),
		ఖాళీ,
		fetch_about_welcome_handler,
		false
	},
	{
		"config",
		SLEN("config"),
		ఖాళీ,
		fetch_about_config_handler,
		false
	},
	{
		"Choices",
		SLEN("Choices"),
		ఖాళీ,
		fetch_about_choices_handler,
		false
	},
	{
		"testament",
		SLEN("testament"),
		ఖాళీ,
		fetch_about_testament_handler,
		false
	},
	{
		"about",
		SLEN("about"),
		ఖాళీ,
		fetch_about_about_handler,
		true
	},
	{
		"nscolours.css",
		SLEN("nscolours.css"),
		ఖాళీ,
		fetch_about_nscolours_handler,
		true
	},
	{
		"logo",
		SLEN("logo"),
		ఖాళీ,
		fetch_about_logo_handler,
		true
	},
	{
		/* details about the image cache */
		"imagecache",
		SLEN("imagecache"),
		ఖాళీ,
		fetch_about_imagecache_handler,
		true
	},
	{
		/* The default blank page */
		"blank",
		SLEN("blank"),
		ఖాళీ,
		fetch_about_blank_handler,
		true
	},
	{
		/* details about a certificate */
		"certificate",
		SLEN("certificate"),
		ఖాళీ,
		fetch_about_certificate_handler,
		true
	},
	{
		/* chart generator */
		"chart",
		SLEN("chart"),
		ఖాళీ,
		fetch_about_chart_handler,
		true
	},
	{
		"query/auth",
		SLEN("query/auth"),
		ఖాళీ,
		fetch_about_query_auth_handler,
		true
	},
	{
		"query/ssl",
		SLEN("query/ssl"),
		ఖాళీ,
		fetch_about_query_privacy_handler,
		true
	},
	{
		"query/timeout",
		SLEN("query/timeout"),
		ఖాళీ,
		fetch_about_query_timeout_handler,
		true
	},
	{
		"query/fetcherror",
		SLEN("query/fetcherror"),
		ఖాళీ,
		fetch_about_query_fetcherror_handler,
		true
	},
	{
		/* web search using the configured provider */
		"websearch",
		SLEN("websearch"),
		ఖాళీ,
		fetch_about_websearch_handler,
		true
	}
};

#నిర్వచించు about_handler_list_len \
	(పరిమాణం(about_handler_list) / పరిమాణం(నిర్మాణం about_handlers))

/**
 * List all the valid about: paths available
 *
 * \param ctx The fetch context.
 * \return true for sucess or false to generate an error.
 */
స్థిర bool fetch_about_about_handler(నిర్మాణం fetch_about_context *ctx)
{
	nserror res;
	గుర్తులేని పూర్ణాంకం abt_loop = 0;

	/* content is going to return ok */
	fetch_సమితి_http_సంకేతం(ctx->fetchh, HTTP_RESPONSE_OK);

	/* content type */
	ఒకవేళ (fetch_about_send_శీర్షిక(ctx, "Content-Type: text/html; charset=utf-8"))
		వెళ్ళు fetch_about_config_handler_aborted;

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>List of NetSurf pages</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class =\"ns-border\">List of NetSurf pages</h1>\n"
			"<ul>\n");
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_config_handler_aborted;
	}

	కొరకు (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {

		/* Skip over hidden entries */
		ఒకవేళ (about_handler_list[abt_loop].hidden)
			కొనసాగు;

		res = fetch_about_ssenddataf(ctx,
			       "<li><a href=\"about:%s\">about:%s</a></li>\n",
			       about_handler_list[abt_loop].name,
			       about_handler_list[abt_loop].name);
		ఒకవేళ (res != NSERROR_OK) {
			వెళ్ళు fetch_about_config_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</ul>\n</body>\n</html>\n");
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_config_handler_aborted;
	}

	fetch_about_send_ముగిసిన(ctx);

	తిరిగి true;

fetch_about_config_handler_aborted:
	తిరిగి false;
}

స్థిర bool
fetch_about_404_handler(నిర్మాణం fetch_about_context *ctx)
{
	nserror res;
	స్థిరరాశి అక్షరం *title;

	/* content is going to return 404 */
	fetch_సమితి_http_సంకేతం(ctx->fetchh, HTTP_RESPONSE_NOT_దొరికింది);

	/* content type */
	ఒకవేళ (fetch_about_send_శీర్షిక(ctx,
			"Content-Type: text/html; charset=utf-8")) {
		తిరిగి false;
	}

	title = messages_get("HTTP404");
	res = fetch_about_ssenddataf(ctx,
		"<html>\n<head>\n"
		"<title>%s</title>\n"
		"<link rel=\"stylesheet\" type=\"text/css\" "
		"href=\"resource:internal.css\">\n"
		"</head>\n"
		"<body class=\"ns-even-bg ns-even-fg ns-border\" id =\"fetcherror\">\n"
		"<h1 class=\"ns-border ns-odd-fg-bad\">%s</h1>\n"
		"<p>%s %d %s %s</p>\n"
		"</body>\n</html>\n",
		title, title, messages_get("FetchErrorCode"), 404,
		messages_get("FetchFile"),nsurl_access(ctx->url));
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	fetch_about_send_ముగిసిన(ctx);

	తిరిగి true;
}

/**
 * callback to initialise the about scheme fetcher.
 */
స్థిర bool fetch_about_initialise(lwc_string *scheme)
{
	గుర్తులేని పూర్ణాంకం abt_loop = 0;
	lwc_error error;

	కొరకు (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		error = lwc_intern_string(about_handler_list[abt_loop].name,
					about_handler_list[abt_loop].name_len,
					&about_handler_list[abt_loop].lname);
		ఒకవేళ (error != lwc_error_ok) {
			అయితే (abt_loop-- != 0) {
				lwc_string_unref(about_handler_list[abt_loop].lname);
			}
			తిరిగి false;
		}
	}

	తిరిగి true;
}


/**
 * callback to finalise the about scheme fetcher.
 */
స్థిర శూన్య fetch_about_finalise(lwc_string *scheme)
{
	గుర్తులేని పూర్ణాంకం abt_loop = 0;
	కొరకు (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		lwc_string_unref(about_handler_list[abt_loop].lname);
	}
}


స్థిర bool fetch_about_can_fetch(స్థిరరాశి nsurl *url)
{
	తిరిగి true;
}


/**
 * callback to set up a about scheme fetch.
 *
 * \param post_urlenc post data in urlenc format, owned by the llcache object
 *                        hence valid the entire lifetime of the fetch.
 * \param post_multipart post data in multipart format, owned by the llcache
 *                        object hence valid the entire lifetime of the fetch.
 */
స్థిర శూన్య *
fetch_about_అమరిక(నిర్మాణం fetch *fetchh,
		  nsurl *url,
		  bool only_2xx,
		  bool downgrade_tls,
		  స్థిరరాశి అక్షరం *post_urlenc,
		  స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *post_multipart,
		  స్థిరరాశి అక్షరం **శీర్షికలు)
{
	నిర్మాణం fetch_about_context *ctx;
	గుర్తులేని పూర్ణాంకం handler_loop;
	lwc_string *మార్గం;
	bool సరిపోలిక;

	ctx = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*ctx));
	ఒకవేళ (ctx == ఖాళీ)
		తిరిగి ఖాళీ;

	మార్గం = nsurl_get_component(url, NSURL_మార్గం);

	కొరకు (handler_loop = 0;
	     handler_loop < about_handler_list_len;
	     handler_loop++) {
		ఒకవేళ (lwc_string_isequal(మార్గం,
				       about_handler_list[handler_loop].lname,
				       &సరిపోలిక) == lwc_error_ok && సరిపోలిక) {
			ctx->handler = about_handler_list[handler_loop].handler;
			ఆపు;
		}
	}

	lwc_string_unref(మార్గం);

	ctx->fetchh = fetchh;
	ctx->url = nsurl_ref(url);
	ctx->multipart = post_multipart;

	RING_చొప్పించు(ring, ctx);

	తిరిగి ctx;
}


/**
 * callback to free a about scheme fetch
 */
స్థిర శూన్య fetch_about_free(శూన్య *ctx)
{
	నిర్మాణం fetch_about_context *c = ctx;
	nsurl_unref(c->url);
	విడుదల(ctx);
}


/**
 * callback to start an about scheme fetch
 */
స్థిర bool fetch_about_ప్రారంభం(శూన్య *ctx)
{
	తిరిగి true;
}


/**
 * callback to abort a about fetch
 */
స్థిర శూన్య fetch_about_abort(శూన్య *ctx)
{
	నిర్మాణం fetch_about_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/**
 * callback to poll for additional about fetch contents
 */
స్థిర శూన్య fetch_about_poll(lwc_string *scheme)
{
	నిర్మాణం fetch_about_context *c, *save_ring = ఖాళీ;

	/* Iterate over ring, processing each pending fetch */
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
			/* about fetches can be processed in one go */
			ఒకవేళ (c->handler == ఖాళీ) {
				fetch_about_404_handler(c);
			} లేకపోతే {
				c->handler(c);
			}
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


nserror fetch_about_రిజిస్టరు(శూన్య)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_about);
	స్థిరరాశి నిర్మాణం fetcher_operation_పట్టిక fetcher_ops = {
		.initialise = fetch_about_initialise,
		.acceptable = fetch_about_can_fetch,
		.అమరిక = fetch_about_అమరిక,
		.ప్రారంభం = fetch_about_ప్రారంభం,
		.abort = fetch_about_abort,
		.విడుదల = fetch_about_free,
		.poll = fetch_about_poll,
		.finalise = fetch_about_finalise
	};

	తిరిగి fetcher_కూడు(scheme, &fetcher_ops);
}
