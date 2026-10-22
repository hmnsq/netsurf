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

#شامل <stdbool.h>
#شامل <stddef.h>
#شامل <stdarg.h>
#شامل <stdio.h>
#شامل <string.h>
#شامل <stdlib.h>

#شامل "netsurf/inttypes.h"

#شامل "utils/errors.h"
#شامل "utils/nsurl.h"
#شامل "utils/corestrings.h"
#شامل "utils/utils.h"
#شامل "utils/ring.h"
#شامل "utils/messages.h"

#شامل "content/fetch.h"
#شامل "content/fetchers.h"

#شامل "private.h"
#شامل "about.h"
#شامل "blank.h"
#شامل "certificate.h"
#شامل "config.h"
#شامل "chart.h"
#شامل "choices.h"
#شامل "imagecache.h"
#شامل "nscolours.h"
#شامل "query.h"
#شامل "query_auth.h"
#شامل "query_fetcherror.h"
#شامل "query_privacy.h"
#شامل "query_timeout.h"
#شامل "atestament.h"
#شامل "websearch.h"

قسمتعریف bool (*fetch_about_handler)(ساخت fetch_about_context *);

/**
 * Context for an about fetch
 */
ساخت fetch_about_context {
	ساخت fetch_about_context *r_اگلا, *r_پچھلا;

	ساخت fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */

	ثابت ساخت fetch_multipart_ڈیٹا *multipart; /**< post data */

	fetch_about_handler handler;
};

ساکن ساخت fetch_about_context *ring = تہی;

/**
 * handler info for about scheme
 */
ساخت about_handlers {
	ثابت حرف *name; /**< name to match in url */
	صحیح name_len;
	lwc_string *lname; /**< Interned name */
	fetch_about_handler handler; /**< handler for the url */
	bool hidden; /**< If entry should be hidden in listing */
};



/**
 * issue fetch callbacks with locking
 */
ساکن bool
fetch_about_send_callback(ثابت fetch_پیغام *پیغام, ساخت fetch_about_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پیغام, ctx->fetchh);
	ctx->locked = false;

	واپس ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_ختم_شدہ(ساخت fetch_about_context *ctx)
{
	fetch_پیغام پیغام;
	پیغام.type = FETCH_ختم_شدہ;
	واپس fetch_about_send_callback(&پیغام, ctx);
}

/* exported interface documented in about/private.h */
bool fetch_about_سیٹ_http_رمز(ساخت fetch_about_context *ctx, طویل رمز)
{
	fetch_سیٹ_http_رمز(ctx->fetchh, رمز);

	واپس ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_سرخی(ساخت fetch_about_context *ctx, ثابت حرف *fmt, ...)
{
	حرف سرخی[64];
	fetch_پیغام پیغام;
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(سرخی, حجم سرخی, fmt, ap);

	va_end(ap);

	پیغام.type = FETCH_سرخی;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *) سرخی;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = لڑی_لمبائی(سرخی);

	واپس fetch_about_send_callback(&پیغام, ctx);
}

/* exported interface documented in about/private.h */
nserror
fetch_about_senddata(ساخت fetch_about_context *ctx, ثابت uint8_t *ڈیٹا, size_t ڈیٹا_len)
{
	fetch_پیغام پیغام;

	پیغام.type = FETCH_ڈیٹا;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = ڈیٹا;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = ڈیٹا_len;

	اگر (fetch_about_send_callback(&پیغام, ctx)) {
		واپس NSERROR_INVALID;
	}

	واپس NSERROR_OK;
}

/* exported interface documented in about/private.h */
nserror
fetch_about_ssenddataf(ساخت fetch_about_context *ctx, ثابت حرف *fmt, ...)
{
	حرف بفر[1024];
	حرف *dbuff;
	fetch_پیغام پیغام;
	va_list ap;
	صحیح slen;

	va_start(ap, fmt);

	slen = vsnprintf(بفر, حجم(بفر), fmt, ap);

	va_end(ap);

	اگر (slen < (صحیح)حجم(بفر)) {
		پیغام.type = FETCH_ڈیٹا;
		پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *) بفر;
		پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = slen;

		اگر (fetch_about_send_callback(&پیغام, ctx)) {
			واپس NSERROR_INVALID;
		}

		واپس NSERROR_OK;
	}

	dbuff = حافظہ_دو(slen + 1);
	اگر (dbuff == تہی) {
		واپس NSERROR_NOSPACE;
	}

	va_start(ap, fmt);

	slen = vsnprintf(dbuff, slen + 1, fmt, ap);

	va_end(ap);

	پیغام.type = FETCH_ڈیٹا;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *)dbuff;
	پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = slen;

	اگر (fetch_about_send_callback(&پیغام, ctx)) {
		آزاد(dbuff);
		واپس NSERROR_INVALID;
	}

	آزاد(dbuff);
	واپس NSERROR_OK;
}


/* exported interface documented in about/private.h */
nsurl *fetch_about_get_url(ساخت fetch_about_context *ctx)
{
	واپس ctx->url;
}


/* exported interface documented in about/private.h */
ثابت ساخت fetch_multipart_ڈیٹا *
fetch_about_get_multipart(ساخت fetch_about_context *ctx)
{
	واپس ctx->multipart;
}


/* exported interface documented in about/private.h */
bool fetch_about_srverror(ساخت fetch_about_context *ctx)
{
	nserror res;

	fetch_سیٹ_http_رمز(ctx->fetchh, HTTP_RESPONSE_اندرونی_SERVER_ERROR);

	/* content type */
	اگر (fetch_about_send_سرخی(ctx, "Content-Type: text/plain"))
		واپس false;

	res = fetch_about_ssenddataf(ctx, "%s 500 %s",
				     messages_get("FetchErrorCode"),
				     messages_get("HTTP500"));
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	fetch_about_send_ختم_شدہ(ctx);

	واپس true;
}


/* exported interface documented in about/private.h */
bool fetch_about_redirect(ساخت fetch_about_context *ctx, ثابت حرف *ہدف)
{
	fetch_پیغام پیغام;

	/* content is going to return redirect */
	fetch_سیٹ_http_رمز(ctx->fetchh, HTTP_RESPONSE_ملا);

	پیغام.type = FETCH_REDIRECT;
	پیغام.ڈیٹا.redirect = ہدف;

	واپس fetch_about_send_callback(&پیغام, ctx);
}

/**
 * Handler to generate about scheme credits page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ساکن bool fetch_about_credits_handler(ساخت fetch_about_context *ctx)
{
	واپس fetch_about_redirect(ctx, "resource:credits.html");
}


/**
 * Handler to generate about scheme licence page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ساکن bool fetch_about_licence_handler(ساخت fetch_about_context *ctx)
{
	واپس fetch_about_redirect(ctx, "resource:licence.html");
}


/**
 * Handler to generate about scheme logo page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ساکن bool fetch_about_logo_handler(ساخت fetch_about_context *ctx)
{
	واپس fetch_about_redirect(ctx, "resource:netsurf.png");
}


/**
 * Handler to generate about scheme welcome page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ساکن bool fetch_about_welcome_handler(ساخت fetch_about_context *ctx)
{
	واپس fetch_about_redirect(ctx, "resource:welcome.html");
}


/* Forward declaration because this handler requires the handler table. */
ساکن bool fetch_about_about_handler(ساخت fetch_about_context *ctx);

/**
 * List of about paths and their handlers
 */
ساخت about_handlers about_handler_list[] = {
	{
		"credits",
		SLEN("credits"),
		تہی,
		fetch_about_credits_handler,
		false
	},
	{
		"licence",
		SLEN("licence"),
		تہی,
		fetch_about_licence_handler,
		false
	},
	{
		"license",
		SLEN("license"),
		تہی,
		fetch_about_licence_handler,
		true
	},
	{
		"welcome",
		SLEN("welcome"),
		تہی,
		fetch_about_welcome_handler,
		false
	},
	{
		"config",
		SLEN("config"),
		تہی,
		fetch_about_config_handler,
		false
	},
	{
		"Choices",
		SLEN("Choices"),
		تہی,
		fetch_about_choices_handler,
		false
	},
	{
		"testament",
		SLEN("testament"),
		تہی,
		fetch_about_testament_handler,
		false
	},
	{
		"about",
		SLEN("about"),
		تہی,
		fetch_about_about_handler,
		true
	},
	{
		"nscolours.css",
		SLEN("nscolours.css"),
		تہی,
		fetch_about_nscolours_handler,
		true
	},
	{
		"logo",
		SLEN("logo"),
		تہی,
		fetch_about_logo_handler,
		true
	},
	{
		/* details about the image cache */
		"imagecache",
		SLEN("imagecache"),
		تہی,
		fetch_about_imagecache_handler,
		true
	},
	{
		/* The default blank page */
		"blank",
		SLEN("blank"),
		تہی,
		fetch_about_blank_handler,
		true
	},
	{
		/* details about a certificate */
		"certificate",
		SLEN("certificate"),
		تہی,
		fetch_about_certificate_handler,
		true
	},
	{
		/* chart generator */
		"chart",
		SLEN("chart"),
		تہی,
		fetch_about_chart_handler,
		true
	},
	{
		"query/auth",
		SLEN("query/auth"),
		تہی,
		fetch_about_query_auth_handler,
		true
	},
	{
		"query/ssl",
		SLEN("query/ssl"),
		تہی,
		fetch_about_query_privacy_handler,
		true
	},
	{
		"query/timeout",
		SLEN("query/timeout"),
		تہی,
		fetch_about_query_timeout_handler,
		true
	},
	{
		"query/fetcherror",
		SLEN("query/fetcherror"),
		تہی,
		fetch_about_query_fetcherror_handler,
		true
	},
	{
		/* web search using the configured provider */
		"websearch",
		SLEN("websearch"),
		تہی,
		fetch_about_websearch_handler,
		true
	}
};

#تعریف_کرو about_handler_list_len \
	(حجم(about_handler_list) / حجم(ساخت about_handlers))

/**
 * List all the valid about: paths available
 *
 * \param ctx The fetch context.
 * \return true for sucess or false to generate an error.
 */
ساکن bool fetch_about_about_handler(ساخت fetch_about_context *ctx)
{
	nserror res;
	غیرعلامتی صحیح abt_loop = 0;

	/* content is going to return ok */
	fetch_سیٹ_http_رمز(ctx->fetchh, HTTP_RESPONSE_OK);

	/* content type */
	اگر (fetch_about_send_سرخی(ctx, "Content-Type: text/html; charset=utf-8"))
		جاؤ fetch_about_config_handler_aborted;

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>List of NetSurf pages</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class =\"ns-border\">List of NetSurf pages</h1>\n"
			"<ul>\n");
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_config_handler_aborted;
	}

	برائے (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {

		/* Skip over hidden entries */
		اگر (about_handler_list[abt_loop].hidden)
			جاری;

		res = fetch_about_ssenddataf(ctx,
			       "<li><a href=\"about:%s\">about:%s</a></li>\n",
			       about_handler_list[abt_loop].name,
			       about_handler_list[abt_loop].name);
		اگر (res != NSERROR_OK) {
			جاؤ fetch_about_config_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</ul>\n</body>\n</html>\n");
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_config_handler_aborted;
	}

	fetch_about_send_ختم_شدہ(ctx);

	واپس true;

fetch_about_config_handler_aborted:
	واپس false;
}

ساکن bool
fetch_about_404_handler(ساخت fetch_about_context *ctx)
{
	nserror res;
	ثابت حرف *title;

	/* content is going to return 404 */
	fetch_سیٹ_http_رمز(ctx->fetchh, HTTP_RESPONSE_NOT_ملا);

	/* content type */
	اگر (fetch_about_send_سرخی(ctx,
			"Content-Type: text/html; charset=utf-8")) {
		واپس false;
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
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	fetch_about_send_ختم_شدہ(ctx);

	واپس true;
}

/**
 * callback to initialise the about scheme fetcher.
 */
ساکن bool fetch_about_initialise(lwc_string *scheme)
{
	غیرعلامتی صحیح abt_loop = 0;
	lwc_error error;

	برائے (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		error = lwc_intern_string(about_handler_list[abt_loop].name,
					about_handler_list[abt_loop].name_len,
					&about_handler_list[abt_loop].lname);
		اگر (error != lwc_error_ok) {
			جب (abt_loop-- != 0) {
				lwc_string_unref(about_handler_list[abt_loop].lname);
			}
			واپس false;
		}
	}

	واپس true;
}


/**
 * callback to finalise the about scheme fetcher.
 */
ساکن خالی fetch_about_finalise(lwc_string *scheme)
{
	غیرعلامتی صحیح abt_loop = 0;
	برائے (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		lwc_string_unref(about_handler_list[abt_loop].lname);
	}
}


ساکن bool fetch_about_can_fetch(ثابت nsurl *url)
{
	واپس true;
}


/**
 * callback to set up a about scheme fetch.
 *
 * \param post_urlenc post data in urlenc format, owned by the llcache object
 *                        hence valid the entire lifetime of the fetch.
 * \param post_multipart post data in multipart format, owned by the llcache
 *                        object hence valid the entire lifetime of the fetch.
 */
ساکن خالی *
fetch_about_تیاری(ساخت fetch *fetchh,
		  nsurl *url,
		  bool only_2xx,
		  bool downgrade_tls,
		  ثابت حرف *post_urlenc,
		  ثابت ساخت fetch_multipart_ڈیٹا *post_multipart,
		  ثابت حرف **سرخیاں)
{
	ساخت fetch_about_context *ctx;
	غیرعلامتی صحیح handler_loop;
	lwc_string *راستہ;
	bool مطابقت;

	ctx = صفر_حافظہ_دو(1, حجم(*ctx));
	اگر (ctx == تہی)
		واپس تہی;

	راستہ = nsurl_get_component(url, NSURL_راستہ);

	برائے (handler_loop = 0;
	     handler_loop < about_handler_list_len;
	     handler_loop++) {
		اگر (lwc_string_isequal(راستہ,
				       about_handler_list[handler_loop].lname,
				       &مطابقت) == lwc_error_ok && مطابقت) {
			ctx->handler = about_handler_list[handler_loop].handler;
			توڑو;
		}
	}

	lwc_string_unref(راستہ);

	ctx->fetchh = fetchh;
	ctx->url = nsurl_ref(url);
	ctx->multipart = post_multipart;

	RING_داخل_کرو(ring, ctx);

	واپس ctx;
}


/**
 * callback to free a about scheme fetch
 */
ساکن خالی fetch_about_free(خالی *ctx)
{
	ساخت fetch_about_context *c = ctx;
	nsurl_unref(c->url);
	آزاد(ctx);
}


/**
 * callback to start an about scheme fetch
 */
ساکن bool fetch_about_آغاز(خالی *ctx)
{
	واپس true;
}


/**
 * callback to abort a about fetch
 */
ساکن خالی fetch_about_abort(خالی *ctx)
{
	ساخت fetch_about_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/**
 * callback to poll for additional about fetch contents
 */
ساکن خالی fetch_about_poll(lwc_string *scheme)
{
	ساخت fetch_about_context *c, *save_ring = تہی;

	/* Iterate over ring, processing each pending fetch */
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
			/* about fetches can be processed in one go */
			اگر (c->handler == تہی) {
				fetch_about_404_handler(c);
			} ورنہ {
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


nserror fetch_about_رجسٹر(خالی)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_about);
	ثابت ساخت fetcher_operation_جدول fetcher_ops = {
		.initialise = fetch_about_initialise,
		.acceptable = fetch_about_can_fetch,
		.تیاری = fetch_about_تیاری,
		.آغاز = fetch_about_آغاز,
		.abort = fetch_about_abort,
		.آزاد = fetch_about_free,
		.poll = fetch_about_poll,
		.finalise = fetch_about_finalise
	};

	واپس fetcher_جوڑو(scheme, &fetcher_ops);
}
