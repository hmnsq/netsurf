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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "about.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "blank.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "certificate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "chart.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "choices.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "imagecache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nscolours.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "query.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "query_auth.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "query_fetcherror.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "query_privacy.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "query_timeout.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "atestament.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "websearch.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ bool (*fetch_about_handler)(ਬਣਤਰ fetch_about_context *);

/**
 * Context for an about fetch
 */
ਬਣਤਰ fetch_about_context {
	ਬਣਤਰ fetch_about_context *r_ਅਗਲਾ, *r_prev;

	ਬਣਤਰ fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */

	ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *multipart; /**< post data */

	fetch_about_handler handler;
};

ਸਥਿਰ ਬਣਤਰ fetch_about_context *ring = ਨਿਰਾ;

/**
 * handler info for about scheme
 */
ਬਣਤਰ about_handlers {
	ਅਚਲ ਅੱਖਰ *name; /**< name to match in url */
	ਪੂਰਨ name_len;
	lwc_string *lname; /**< Interned name */
	fetch_about_handler handler; /**< handler for the url */
	bool hidden; /**< If entry should be hidden in listing */
};



/**
 * issue fetch callbacks with locking
 */
ਸਥਿਰ bool
fetch_about_send_callback(ਅਚਲ fetch_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ, ਬਣਤਰ fetch_about_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ਸੁਨੇਹਾ, ctx->fetchh);
	ctx->locked = false;

	ਵਾਪਸ ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_ਮੁਕੰਮਲ(ਬਣਤਰ fetch_about_context *ctx)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	ਸੁਨੇਹਾ.type = FETCH_ਮੁਕੰਮਲ;
	ਵਾਪਸ fetch_about_send_callback(&ਸੁਨੇਹਾ, ctx);
}

/* exported interface documented in about/private.h */
bool fetch_about_ਸੈੱਟ_http_ਕੋਡ(ਬਣਤਰ fetch_about_context *ctx, ਲੰਮਾ ਕੋਡ)
{
	fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, ਕੋਡ);

	ਵਾਪਸ ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_ਸਿਰਲੇਖ(ਬਣਤਰ fetch_about_context *ctx, ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	ਅੱਖਰ ਸਿਰਲੇਖ[64];
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(ਸਿਰਲੇਖ, ਆਕਾਰ ਸਿਰਲੇਖ, fmt, ap);

	va_end(ap);

	ਸੁਨੇਹਾ.type = FETCH_ਸਿਰਲੇਖ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *) ਸਿਰਲੇਖ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਲੜੀ_ਲੰਬਾਈ(ਸਿਰਲੇਖ);

	ਵਾਪਸ fetch_about_send_callback(&ਸੁਨੇਹਾ, ctx);
}

/* exported interface documented in about/private.h */
nserror
fetch_about_senddata(ਬਣਤਰ fetch_about_context *ctx, ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t ਅੰਕੜੇ_len)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;

	ਸੁਨੇਹਾ.type = FETCH_ਅੰਕੜੇ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = ਅੰਕੜੇ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = ਅੰਕੜੇ_len;

	ਜੇ (fetch_about_send_callback(&ਸੁਨੇਹਾ, ctx)) {
		ਵਾਪਸ NSERROR_INVALID;
	}

	ਵਾਪਸ NSERROR_OK;
}

/* exported interface documented in about/private.h */
nserror
fetch_about_ssenddataf(ਬਣਤਰ fetch_about_context *ctx, ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	ਅੱਖਰ ਬਫਰ[1024];
	ਅੱਖਰ *dbuff;
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;
	va_list ap;
	ਪੂਰਨ slen;

	va_start(ap, fmt);

	slen = vsnprintf(ਬਫਰ, ਆਕਾਰ(ਬਫਰ), fmt, ap);

	va_end(ap);

	ਜੇ (slen < (ਪੂਰਨ)ਆਕਾਰ(ਬਫਰ)) {
		ਸੁਨੇਹਾ.type = FETCH_ਅੰਕੜੇ;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *) ਬਫਰ;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = slen;

		ਜੇ (fetch_about_send_callback(&ਸੁਨੇਹਾ, ctx)) {
			ਵਾਪਸ NSERROR_INVALID;
		}

		ਵਾਪਸ NSERROR_OK;
	}

	dbuff = ਸਮ੍ਰਿਤੀ_ਦਿਓ(slen + 1);
	ਜੇ (dbuff == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOSPACE;
	}

	va_start(ap, fmt);

	slen = vsnprintf(dbuff, slen + 1, fmt, ap);

	va_end(ap);

	ਸੁਨੇਹਾ.type = FETCH_ਅੰਕੜੇ;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf = (ਅਚਲ uint8_t *)dbuff;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len = slen;

	ਜੇ (fetch_about_send_callback(&ਸੁਨੇਹਾ, ctx)) {
		ਮੁਕਤ(dbuff);
		ਵਾਪਸ NSERROR_INVALID;
	}

	ਮੁਕਤ(dbuff);
	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in about/private.h */
nsurl *fetch_about_get_url(ਬਣਤਰ fetch_about_context *ctx)
{
	ਵਾਪਸ ctx->url;
}


/* exported interface documented in about/private.h */
ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *
fetch_about_get_multipart(ਬਣਤਰ fetch_about_context *ctx)
{
	ਵਾਪਸ ctx->multipart;
}


/* exported interface documented in about/private.h */
bool fetch_about_srverror(ਬਣਤਰ fetch_about_context *ctx)
{
	nserror res;

	fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, HTTP_RESPONSE_ਅੰਦਰੂਨੀ_SERVER_ERROR);

	/* content type */
	ਜੇ (fetch_about_send_ਸਿਰਲੇਖ(ctx, "Content-Type: text/plain"))
		ਵਾਪਸ false;

	res = fetch_about_ssenddataf(ctx, "%s 500 %s",
				     messages_get("FetchErrorCode"),
				     messages_get("HTTP500"));
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	fetch_about_send_ਮੁਕੰਮਲ(ctx);

	ਵਾਪਸ true;
}


/* exported interface documented in about/private.h */
bool fetch_about_redirect(ਬਣਤਰ fetch_about_context *ctx, ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ)
{
	fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;

	/* content is going to return redirect */
	fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, HTTP_RESPONSE_ਮਿਲਿਆ);

	ਸੁਨੇਹਾ.type = FETCH_REDIRECT;
	ਸੁਨੇਹਾ.ਅੰਕੜੇ.redirect = ਨਿਸ਼ਾਨਾ;

	ਵਾਪਸ fetch_about_send_callback(&ਸੁਨੇਹਾ, ctx);
}

/**
 * Handler to generate about scheme credits page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ਸਥਿਰ bool fetch_about_credits_handler(ਬਣਤਰ fetch_about_context *ctx)
{
	ਵਾਪਸ fetch_about_redirect(ctx, "resource:credits.html");
}


/**
 * Handler to generate about scheme licence page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ਸਥਿਰ bool fetch_about_licence_handler(ਬਣਤਰ fetch_about_context *ctx)
{
	ਵਾਪਸ fetch_about_redirect(ctx, "resource:licence.html");
}


/**
 * Handler to generate about scheme logo page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ਸਥਿਰ bool fetch_about_logo_handler(ਬਣਤਰ fetch_about_context *ctx)
{
	ਵਾਪਸ fetch_about_redirect(ctx, "resource:netsurf.png");
}


/**
 * Handler to generate about scheme welcome page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ਸਥਿਰ bool fetch_about_welcome_handler(ਬਣਤਰ fetch_about_context *ctx)
{
	ਵਾਪਸ fetch_about_redirect(ctx, "resource:welcome.html");
}


/* Forward declaration because this handler requires the handler table. */
ਸਥਿਰ bool fetch_about_about_handler(ਬਣਤਰ fetch_about_context *ctx);

/**
 * List of about paths and their handlers
 */
ਬਣਤਰ about_handlers about_handler_list[] = {
	{
		"credits",
		SLEN("credits"),
		ਨਿਰਾ,
		fetch_about_credits_handler,
		false
	},
	{
		"licence",
		SLEN("licence"),
		ਨਿਰਾ,
		fetch_about_licence_handler,
		false
	},
	{
		"license",
		SLEN("license"),
		ਨਿਰਾ,
		fetch_about_licence_handler,
		true
	},
	{
		"welcome",
		SLEN("welcome"),
		ਨਿਰਾ,
		fetch_about_welcome_handler,
		false
	},
	{
		"config",
		SLEN("config"),
		ਨਿਰਾ,
		fetch_about_config_handler,
		false
	},
	{
		"Choices",
		SLEN("Choices"),
		ਨਿਰਾ,
		fetch_about_choices_handler,
		false
	},
	{
		"testament",
		SLEN("testament"),
		ਨਿਰਾ,
		fetch_about_testament_handler,
		false
	},
	{
		"about",
		SLEN("about"),
		ਨਿਰਾ,
		fetch_about_about_handler,
		true
	},
	{
		"nscolours.css",
		SLEN("nscolours.css"),
		ਨਿਰਾ,
		fetch_about_nscolours_handler,
		true
	},
	{
		"logo",
		SLEN("logo"),
		ਨਿਰਾ,
		fetch_about_logo_handler,
		true
	},
	{
		/* details about the image cache */
		"imagecache",
		SLEN("imagecache"),
		ਨਿਰਾ,
		fetch_about_imagecache_handler,
		true
	},
	{
		/* The default blank page */
		"blank",
		SLEN("blank"),
		ਨਿਰਾ,
		fetch_about_blank_handler,
		true
	},
	{
		/* details about a certificate */
		"certificate",
		SLEN("certificate"),
		ਨਿਰਾ,
		fetch_about_certificate_handler,
		true
	},
	{
		/* chart generator */
		"chart",
		SLEN("chart"),
		ਨਿਰਾ,
		fetch_about_chart_handler,
		true
	},
	{
		"query/auth",
		SLEN("query/auth"),
		ਨਿਰਾ,
		fetch_about_query_auth_handler,
		true
	},
	{
		"query/ssl",
		SLEN("query/ssl"),
		ਨਿਰਾ,
		fetch_about_query_privacy_handler,
		true
	},
	{
		"query/timeout",
		SLEN("query/timeout"),
		ਨਿਰਾ,
		fetch_about_query_timeout_handler,
		true
	},
	{
		"query/fetcherror",
		SLEN("query/fetcherror"),
		ਨਿਰਾ,
		fetch_about_query_fetcherror_handler,
		true
	},
	{
		/* web search using the configured provider */
		"websearch",
		SLEN("websearch"),
		ਨਿਰਾ,
		fetch_about_websearch_handler,
		true
	}
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ about_handler_list_len \
	(ਆਕਾਰ(about_handler_list) / ਆਕਾਰ(ਬਣਤਰ about_handlers))

/**
 * List all the valid about: paths available
 *
 * \param ctx The fetch context.
 * \return true for sucess or false to generate an error.
 */
ਸਥਿਰ bool fetch_about_about_handler(ਬਣਤਰ fetch_about_context *ctx)
{
	nserror res;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ abt_loop = 0;

	/* content is going to return ok */
	fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, HTTP_RESPONSE_OK);

	/* content type */
	ਜੇ (fetch_about_send_ਸਿਰਲੇਖ(ctx, "Content-Type: text/html; charset=utf-8"))
		ਜਾਓ fetch_about_config_handler_aborted;

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>List of NetSurf pages</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class =\"ns-border\">List of NetSurf pages</h1>\n"
			"<ul>\n");
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ fetch_about_config_handler_aborted;
	}

	ਲਈ (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {

		/* Skip over hidden entries */
		ਜੇ (about_handler_list[abt_loop].hidden)
			ਜਾਰੀ;

		res = fetch_about_ssenddataf(ctx,
			       "<li><a href=\"about:%s\">about:%s</a></li>\n",
			       about_handler_list[abt_loop].name,
			       about_handler_list[abt_loop].name);
		ਜੇ (res != NSERROR_OK) {
			ਜਾਓ fetch_about_config_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</ul>\n</body>\n</html>\n");
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ fetch_about_config_handler_aborted;
	}

	fetch_about_send_ਮੁਕੰਮਲ(ctx);

	ਵਾਪਸ true;

fetch_about_config_handler_aborted:
	ਵਾਪਸ false;
}

ਸਥਿਰ bool
fetch_about_404_handler(ਬਣਤਰ fetch_about_context *ctx)
{
	nserror res;
	ਅਚਲ ਅੱਖਰ *title;

	/* content is going to return 404 */
	fetch_ਸੈੱਟ_http_ਕੋਡ(ctx->fetchh, HTTP_RESPONSE_NOT_ਮਿਲਿਆ);

	/* content type */
	ਜੇ (fetch_about_send_ਸਿਰਲੇਖ(ctx,
			"Content-Type: text/html; charset=utf-8")) {
		ਵਾਪਸ false;
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
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	fetch_about_send_ਮੁਕੰਮਲ(ctx);

	ਵਾਪਸ true;
}

/**
 * callback to initialise the about scheme fetcher.
 */
ਸਥਿਰ bool fetch_about_initialise(lwc_string *scheme)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ abt_loop = 0;
	lwc_error error;

	ਲਈ (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		error = lwc_intern_string(about_handler_list[abt_loop].name,
					about_handler_list[abt_loop].name_len,
					&about_handler_list[abt_loop].lname);
		ਜੇ (error != lwc_error_ok) {
			ਜਦੋਂ (abt_loop-- != 0) {
				lwc_string_unref(about_handler_list[abt_loop].lname);
			}
			ਵਾਪਸ false;
		}
	}

	ਵਾਪਸ true;
}


/**
 * callback to finalise the about scheme fetcher.
 */
ਸਥਿਰ ਖਾਲੀ fetch_about_finalise(lwc_string *scheme)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ abt_loop = 0;
	ਲਈ (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		lwc_string_unref(about_handler_list[abt_loop].lname);
	}
}


ਸਥਿਰ bool fetch_about_can_fetch(ਅਚਲ nsurl *url)
{
	ਵਾਪਸ true;
}


/**
 * callback to set up a about scheme fetch.
 *
 * \param post_urlenc post data in urlenc format, owned by the llcache object
 *                        hence valid the entire lifetime of the fetch.
 * \param post_multipart post data in multipart format, owned by the llcache
 *                        object hence valid the entire lifetime of the fetch.
 */
ਸਥਿਰ ਖਾਲੀ *
fetch_about_ਤਿਆਰੀ(ਬਣਤਰ fetch *fetchh,
		  nsurl *url,
		  bool only_2xx,
		  bool downgrade_tls,
		  ਅਚਲ ਅੱਖਰ *post_urlenc,
		  ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *post_multipart,
		  ਅਚਲ ਅੱਖਰ **ਸਿਰਲੇਖਾਂ)
{
	ਬਣਤਰ fetch_about_context *ctx;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ handler_loop;
	lwc_string *ਰਸਤਾ;
	bool ਮੇਲ;

	ctx = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*ctx));
	ਜੇ (ctx == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਰਸਤਾ = nsurl_get_component(url, NSURL_ਰਸਤਾ);

	ਲਈ (handler_loop = 0;
	     handler_loop < about_handler_list_len;
	     handler_loop++) {
		ਜੇ (lwc_string_isequal(ਰਸਤਾ,
				       about_handler_list[handler_loop].lname,
				       &ਮੇਲ) == lwc_error_ok && ਮੇਲ) {
			ctx->handler = about_handler_list[handler_loop].handler;
			ਰੋਕੋ;
		}
	}

	lwc_string_unref(ਰਸਤਾ);

	ctx->fetchh = fetchh;
	ctx->url = nsurl_ref(url);
	ctx->multipart = post_multipart;

	RING_ਪਾਓ(ring, ctx);

	ਵਾਪਸ ctx;
}


/**
 * callback to free a about scheme fetch
 */
ਸਥਿਰ ਖਾਲੀ fetch_about_free(ਖਾਲੀ *ctx)
{
	ਬਣਤਰ fetch_about_context *c = ctx;
	nsurl_unref(c->url);
	ਮੁਕਤ(ctx);
}


/**
 * callback to start an about scheme fetch
 */
ਸਥਿਰ bool fetch_about_ਸ਼ੁਰੂ(ਖਾਲੀ *ctx)
{
	ਵਾਪਸ true;
}


/**
 * callback to abort a about fetch
 */
ਸਥਿਰ ਖਾਲੀ fetch_about_abort(ਖਾਲੀ *ctx)
{
	ਬਣਤਰ fetch_about_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/**
 * callback to poll for additional about fetch contents
 */
ਸਥਿਰ ਖਾਲੀ fetch_about_poll(lwc_string *scheme)
{
	ਬਣਤਰ fetch_about_context *c, *save_ring = ਨਿਰਾ;

	/* Iterate over ring, processing each pending fetch */
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
			/* about fetches can be processed in one go */
			ਜੇ (c->handler == ਨਿਰਾ) {
				fetch_about_404_handler(c);
			} ਨਹੀਂਤਾਂ {
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


nserror fetch_about_ਰਜਿਸਟਰ(ਖਾਲੀ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_about);
	ਅਚਲ ਬਣਤਰ fetcher_operation_ਸਾਰਣੀ fetcher_ops = {
		.initialise = fetch_about_initialise,
		.acceptable = fetch_about_can_fetch,
		.ਤਿਆਰੀ = fetch_about_ਤਿਆਰੀ,
		.ਸ਼ੁਰੂ = fetch_about_ਸ਼ੁਰੂ,
		.abort = fetch_about_abort,
		.ਮੁਕਤ = fetch_about_free,
		.poll = fetch_about_poll,
		.finalise = fetch_about_finalise
	};

	ਵਾਪਸ fetcher_ਜੋੜੋ(scheme, &fetcher_ops);
}
