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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/ring.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"

#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers.h"

#ಅನ್ತರ್ಭಾವ "private.h"
#ಅನ್ತರ್ಭಾವ "about.h"
#ಅನ್ತರ್ಭಾವ "blank.h"
#ಅನ್ತರ್ಭಾವ "certificate.h"
#ಅನ್ತರ್ಭಾವ "config.h"
#ಅನ್ತರ್ಭಾವ "chart.h"
#ಅನ್ತರ್ಭಾವ "choices.h"
#ಅನ್ತರ್ಭಾವ "imagecache.h"
#ಅನ್ತರ್ಭಾವ "nscolours.h"
#ಅನ್ತರ್ಭಾವ "query.h"
#ಅನ್ತರ್ಭಾವ "query_auth.h"
#ಅನ್ತರ್ಭಾವ "query_fetcherror.h"
#ಅನ್ತರ್ಭಾವ "query_privacy.h"
#ಅನ್ತರ್ಭಾವ "query_timeout.h"
#ಅನ್ತರ್ಭಾವ "atestament.h"
#ಅನ್ತರ್ಭಾವ "websearch.h"

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ bool (*fetch_about_handler)(ರಚನೆ fetch_about_context *);

/**
 * Context for an about fetch
 */
ರಚನೆ fetch_about_context {
	ರಚನೆ fetch_about_context *r_ಮುಂದಿನ, *r_prev;

	ರಚನೆ fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */

	ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *multipart; /**< post data */

	fetch_about_handler handler;
};

ಸ್ಥಿರ ರಚನೆ fetch_about_context *ring = ಖಾಲಿ;

/**
 * handler info for about scheme
 */
ರಚನೆ about_handlers {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name; /**< name to match in url */
	ಪೂರ್ಣಾಂಕ name_len;
	lwc_string *lname; /**< Interned name */
	fetch_about_handler handler; /**< handler for the url */
	bool hidden; /**< If entry should be hidden in listing */
};



/**
 * issue fetch callbacks with locking
 */
ಸ್ಥಿರ bool
fetch_about_send_callback(ಸ್ಥಿರಾಂಕ fetch_ಸಂದೇಶ *ಸಂದೇಶ, ರಚನೆ fetch_about_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ಸಂದೇಶ, ctx->fetchh);
	ctx->locked = false;

	ಹಿಂತಿರುಗಿ ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_ಪೂರ್ಣಗೊಂಡ(ರಚನೆ fetch_about_context *ctx)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಸಂದೇಶ.type = FETCH_ಪೂರ್ಣಗೊಂಡ;
	ಹಿಂತಿರುಗಿ fetch_about_send_callback(&ಸಂದೇಶ, ctx);
}

/* exported interface documented in about/private.h */
bool fetch_about_ಗಣ_http_ಸಂಕೇತ(ರಚನೆ fetch_about_context *ctx, ದೀರ್ಘ ಸಂಕೇತ_2)
{
	fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, ಸಂಕೇತ_2);

	ಹಿಂತಿರುಗಿ ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_ಶೀರ್ಷಿಕೆ(ರಚನೆ fetch_about_context *ctx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	ಅಕ್ಷರ ಶೀರ್ಷಿಕೆ[64];
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(ಶೀರ್ಷಿಕೆ, ಗಾತ್ರ ಶೀರ್ಷಿಕೆ, fmt, ap);

	va_end(ap);

	ಸಂದೇಶ.type = FETCH_ಶೀರ್ಷಿಕೆ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *) ಶೀರ್ಷಿಕೆ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಶೀರ್ಷಿಕೆ);

	ಹಿಂತಿರುಗಿ fetch_about_send_callback(&ಸಂದೇಶ, ctx);
}

/* exported interface documented in about/private.h */
nserror
fetch_about_senddata(ರಚನೆ fetch_about_context *ctx, ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t ದತ್ತಾಂಶ_len)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	ಸಂದೇಶ.type = FETCH_ದತ್ತಾಂಶ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = ದತ್ತಾಂಶ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = ದತ್ತಾಂಶ_len;

	ಆದರೆ (fetch_about_send_callback(&ಸಂದೇಶ, ctx)) {
		ಹಿಂತಿರುಗಿ NSERROR_INVALID;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* exported interface documented in about/private.h */
nserror
fetch_about_ssenddataf(ರಚನೆ fetch_about_context *ctx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	ಅಕ್ಷರ ಬಫರ್[1024];
	ಅಕ್ಷರ *dbuff;
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	va_list ap;
	ಪೂರ್ಣಾಂಕ slen;

	va_start(ap, fmt);

	slen = vsnprintf(ಬಫರ್, ಗಾತ್ರ(ಬಫರ್), fmt, ap);

	va_end(ap);

	ಆದರೆ (slen < (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(ಬಫರ್)) {
		ಸಂದೇಶ.type = FETCH_ದತ್ತಾಂಶ;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *) ಬಫರ್;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = slen;

		ಆದರೆ (fetch_about_send_callback(&ಸಂದೇಶ, ctx)) {
			ಹಿಂತಿರುಗಿ NSERROR_INVALID;
		}

		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	dbuff = ಸ್ಮರಣೆ_ಕೊಡು(slen + 1);
	ಆದರೆ (dbuff == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOSPACE;
	}

	va_start(ap, fmt);

	slen = vsnprintf(dbuff, slen + 1, fmt, ap);

	va_end(ap);

	ಸಂದೇಶ.type = FETCH_ದತ್ತಾಂಶ;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf = (ಸ್ಥಿರಾಂಕ uint8_t *)dbuff;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len = slen;

	ಆದರೆ (fetch_about_send_callback(&ಸಂದೇಶ, ctx)) {
		ಬಿಡುಗಡೆ(dbuff);
		ಹಿಂತಿರುಗಿ NSERROR_INVALID;
	}

	ಬಿಡುಗಡೆ(dbuff);
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in about/private.h */
nsurl *fetch_about_get_url(ರಚನೆ fetch_about_context *ctx)
{
	ಹಿಂತಿರುಗಿ ctx->url;
}


/* exported interface documented in about/private.h */
ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *
fetch_about_get_multipart(ರಚನೆ fetch_about_context *ctx)
{
	ಹಿಂತಿರುಗಿ ctx->multipart;
}


/* exported interface documented in about/private.h */
bool fetch_about_srverror(ರಚನೆ fetch_about_context *ctx)
{
	nserror res;

	fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, HTTP_RESPONSE_ಆಂತರಿಕ_SERVER_ERROR);

	/* content type */
	ಆದರೆ (fetch_about_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: text/plain"))
		ಹಿಂತಿರುಗಿ false;

	res = fetch_about_ssenddataf(ctx, "%s 500 %s",
				     messages_get("FetchErrorCode"),
				     messages_get("HTTP500"));
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	fetch_about_send_ಪೂರ್ಣಗೊಂಡ(ctx);

	ಹಿಂತಿರುಗಿ true;
}


/* exported interface documented in about/private.h */
bool fetch_about_redirect(ರಚನೆ fetch_about_context *ctx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	/* content is going to return redirect */
	fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, HTTP_RESPONSE_ಸಿಕ್ಕಿತು);

	ಸಂದೇಶ.type = FETCH_REDIRECT;
	ಸಂದೇಶ.ದತ್ತಾಂಶ.redirect = ಗುರಿ;

	ಹಿಂತಿರುಗಿ fetch_about_send_callback(&ಸಂದೇಶ, ctx);
}

/**
 * Handler to generate about scheme credits page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ಸ್ಥಿರ bool fetch_about_credits_handler(ರಚನೆ fetch_about_context *ctx)
{
	ಹಿಂತಿರುಗಿ fetch_about_redirect(ctx, "resource:credits.html");
}


/**
 * Handler to generate about scheme licence page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ಸ್ಥಿರ bool fetch_about_licence_handler(ರಚನೆ fetch_about_context *ctx)
{
	ಹಿಂತಿರುಗಿ fetch_about_redirect(ctx, "resource:licence.html");
}


/**
 * Handler to generate about scheme logo page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ಸ್ಥಿರ bool fetch_about_logo_handler(ರಚನೆ fetch_about_context *ctx)
{
	ಹಿಂತಿರುಗಿ fetch_about_redirect(ctx, "resource:netsurf.png");
}


/**
 * Handler to generate about scheme welcome page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ಸ್ಥಿರ bool fetch_about_welcome_handler(ರಚನೆ fetch_about_context *ctx)
{
	ಹಿಂತಿರುಗಿ fetch_about_redirect(ctx, "resource:welcome.html");
}


/* Forward declaration because this handler requires the handler table. */
ಸ್ಥಿರ bool fetch_about_about_handler(ರಚನೆ fetch_about_context *ctx);

/**
 * List of about paths and their handlers
 */
ರಚನೆ about_handlers about_handler_list[] = {
	{
		"credits",
		SLEN("credits"),
		ಖಾಲಿ,
		fetch_about_credits_handler,
		false
	},
	{
		"licence",
		SLEN("licence"),
		ಖಾಲಿ,
		fetch_about_licence_handler,
		false
	},
	{
		"license",
		SLEN("license"),
		ಖಾಲಿ,
		fetch_about_licence_handler,
		true
	},
	{
		"welcome",
		SLEN("welcome"),
		ಖಾಲಿ,
		fetch_about_welcome_handler,
		false
	},
	{
		"config",
		SLEN("config"),
		ಖಾಲಿ,
		fetch_about_config_handler,
		false
	},
	{
		"Choices",
		SLEN("Choices"),
		ಖಾಲಿ,
		fetch_about_choices_handler,
		false
	},
	{
		"testament",
		SLEN("testament"),
		ಖಾಲಿ,
		fetch_about_testament_handler,
		false
	},
	{
		"about",
		SLEN("about"),
		ಖಾಲಿ,
		fetch_about_about_handler,
		true
	},
	{
		"nscolours.css",
		SLEN("nscolours.css"),
		ಖಾಲಿ,
		fetch_about_nscolours_handler,
		true
	},
	{
		"logo",
		SLEN("logo"),
		ಖಾಲಿ,
		fetch_about_logo_handler,
		true
	},
	{
		/* details about the image cache */
		"imagecache",
		SLEN("imagecache"),
		ಖಾಲಿ,
		fetch_about_imagecache_handler,
		true
	},
	{
		/* The default blank page */
		"blank",
		SLEN("blank"),
		ಖಾಲಿ,
		fetch_about_blank_handler,
		true
	},
	{
		/* details about a certificate */
		"certificate",
		SLEN("certificate"),
		ಖಾಲಿ,
		fetch_about_certificate_handler,
		true
	},
	{
		/* chart generator */
		"chart",
		SLEN("chart"),
		ಖಾಲಿ,
		fetch_about_chart_handler,
		true
	},
	{
		"query/auth",
		SLEN("query/auth"),
		ಖಾಲಿ,
		fetch_about_query_auth_handler,
		true
	},
	{
		"query/ssl",
		SLEN("query/ssl"),
		ಖಾಲಿ,
		fetch_about_query_privacy_handler,
		true
	},
	{
		"query/timeout",
		SLEN("query/timeout"),
		ಖಾಲಿ,
		fetch_about_query_timeout_handler,
		true
	},
	{
		"query/fetcherror",
		SLEN("query/fetcherror"),
		ಖಾಲಿ,
		fetch_about_query_fetcherror_handler,
		true
	},
	{
		/* web search using the configured provider */
		"websearch",
		SLEN("websearch"),
		ಖಾಲಿ,
		fetch_about_websearch_handler,
		true
	}
};

#ವ್ಯಾಖ್ಯಾನಿಸು about_handler_list_len \
	(ಗಾತ್ರ(about_handler_list) / ಗಾತ್ರ(ರಚನೆ about_handlers))

/**
 * List all the valid about: paths available
 *
 * \param ctx The fetch context.
 * \return true for sucess or false to generate an error.
 */
ಸ್ಥಿರ bool fetch_about_about_handler(ರಚನೆ fetch_about_context *ctx)
{
	nserror res;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ abt_loop = 0;

	/* content is going to return ok */
	fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, HTTP_RESPONSE_OK);

	/* content type */
	ಆದರೆ (fetch_about_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: text/html; charset=utf-8"))
		ಹೋಗು fetch_about_config_handler_aborted;

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>List of NetSurf pages</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class =\"ns-border\">List of NetSurf pages</h1>\n"
			"<ul>\n");
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_config_handler_aborted;
	}

	ಗಾಗಿ (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {

		/* Skip over hidden entries */
		ಆದರೆ (about_handler_list[abt_loop].hidden)
			ಮುಂದುವರಿ;

		res = fetch_about_ssenddataf(ctx,
			       "<li><a href=\"about:%s\">about:%s</a></li>\n",
			       about_handler_list[abt_loop].name,
			       about_handler_list[abt_loop].name);
		ಆದರೆ (res != NSERROR_OK) {
			ಹೋಗು fetch_about_config_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</ul>\n</body>\n</html>\n");
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_config_handler_aborted;
	}

	fetch_about_send_ಪೂರ್ಣಗೊಂಡ(ctx);

	ಹಿಂತಿರುಗಿ true;

fetch_about_config_handler_aborted:
	ಹಿಂತಿರುಗಿ false;
}

ಸ್ಥಿರ bool
fetch_about_404_handler(ರಚನೆ fetch_about_context *ctx)
{
	nserror res;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title;

	/* content is going to return 404 */
	fetch_ಗಣ_http_ಸಂಕೇತ(ctx->fetchh, HTTP_RESPONSE_NOT_ಸಿಕ್ಕಿತು);

	/* content type */
	ಆದರೆ (fetch_about_send_ಶೀರ್ಷಿಕೆ(ctx,
			"Content-Type: text/html; charset=utf-8")) {
		ಹಿಂತಿರುಗಿ false;
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
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	fetch_about_send_ಪೂರ್ಣಗೊಂಡ(ctx);

	ಹಿಂತಿರುಗಿ true;
}

/**
 * callback to initialise the about scheme fetcher.
 */
ಸ್ಥಿರ bool fetch_about_initialise(lwc_string *scheme)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ abt_loop = 0;
	lwc_error error;

	ಗಾಗಿ (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		error = lwc_intern_string(about_handler_list[abt_loop].name,
					about_handler_list[abt_loop].name_len,
					&about_handler_list[abt_loop].lname);
		ಆದರೆ (error != lwc_error_ok) {
			ಆಗ (abt_loop-- != 0) {
				lwc_string_unref(about_handler_list[abt_loop].lname);
			}
			ಹಿಂತಿರುಗಿ false;
		}
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * callback to finalise the about scheme fetcher.
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_about_finalise(lwc_string *scheme)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ abt_loop = 0;
	ಗಾಗಿ (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		lwc_string_unref(about_handler_list[abt_loop].lname);
	}
}


ಸ್ಥಿರ bool fetch_about_can_fetch(ಸ್ಥಿರಾಂಕ nsurl *url)
{
	ಹಿಂತಿರುಗಿ true;
}


/**
 * callback to set up a about scheme fetch.
 *
 * \param post_urlenc post data in urlenc format, owned by the llcache object
 *                        hence valid the entire lifetime of the fetch.
 * \param post_multipart post data in multipart format, owned by the llcache
 *                        object hence valid the entire lifetime of the fetch.
 */
ಸ್ಥಿರ ಶೂನ್ಯ *
fetch_about_ಸಿದ್ಧತೆ(ರಚನೆ fetch *fetchh,
		  nsurl *url,
		  bool only_2xx,
		  bool downgrade_tls,
		  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *post_urlenc,
		  ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *post_multipart,
		  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಶೀರ್ಷಿಕೆಗಳು)
{
	ರಚನೆ fetch_about_context *ctx;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ handler_loop;
	lwc_string *ಮಾರ್ಗ;
	bool ಹೊಂದಿಕೆ;

	ctx = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*ctx));
	ಆದರೆ (ctx == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಮಾರ್ಗ = nsurl_get_component(url, NSURL_ಮಾರ್ಗ);

	ಗಾಗಿ (handler_loop = 0;
	     handler_loop < about_handler_list_len;
	     handler_loop++) {
		ಆದರೆ (lwc_string_isequal(ಮಾರ್ಗ,
				       about_handler_list[handler_loop].lname,
				       &ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ) {
			ctx->handler = about_handler_list[handler_loop].handler;
			ನಿಲ್ಲಿಸು;
		}
	}

	lwc_string_unref(ಮಾರ್ಗ);

	ctx->fetchh = fetchh;
	ctx->url = nsurl_ref(url);
	ctx->multipart = post_multipart;

	RING_ಸೇರಿಸು(ring, ctx);

	ಹಿಂತಿರುಗಿ ctx;
}


/**
 * callback to free a about scheme fetch
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_about_free(ಶೂನ್ಯ *ctx)
{
	ರಚನೆ fetch_about_context *c = ctx;
	nsurl_unref(c->url);
	ಬಿಡುಗಡೆ(ctx);
}


/**
 * callback to start an about scheme fetch
 */
ಸ್ಥಿರ bool fetch_about_ಆರಂಭ(ಶೂನ್ಯ *ctx)
{
	ಹಿಂತಿರುಗಿ true;
}


/**
 * callback to abort a about fetch
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_about_abort(ಶೂನ್ಯ *ctx)
{
	ರಚನೆ fetch_about_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/**
 * callback to poll for additional about fetch contents
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_about_poll(lwc_string *scheme)
{
	ರಚನೆ fetch_about_context *c, *save_ring = ಖಾಲಿ;

	/* Iterate over ring, processing each pending fetch */
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
			/* about fetches can be processed in one go */
			ಆದರೆ (c->handler == ಖಾಲಿ) {
				fetch_about_404_handler(c);
			} ಇಲ್ಲದಿದ್ದರೆ {
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


nserror fetch_about_ರಿಜಿಸ್ಟರ್(ಶೂನ್ಯ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_about);
	ಸ್ಥಿರಾಂಕ ರಚನೆ fetcher_operation_ಕೋಷ್ಟಕ fetcher_ops = {
		.initialise = fetch_about_initialise,
		.acceptable = fetch_about_can_fetch,
		.ಸಿದ್ಧತೆ = fetch_about_ಸಿದ್ಧತೆ,
		.ಆರಂಭ = fetch_about_ಆರಂಭ,
		.abort = fetch_about_abort,
		.ಬಿಡುಗಡೆ = fetch_about_free,
		.poll = fetch_about_poll,
		.finalise = fetch_about_finalise
	};

	ಹಿಂತಿರುಗಿ fetcher_ಕೂಡಿಸು(scheme, &fetcher_ops);
}
