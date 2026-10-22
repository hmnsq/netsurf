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

ಪ್ರಕಾರನಿರ್ದೇಶ bool (*fetch_about_handler)(ಸಂರಚನಾ fetch_about_context *);

/**
 * Context for an about fetch
 */
ಸಂರಚನಾ fetch_about_context {
	ಸಂರಚನಾ fetch_about_context *r_next, *r_prev;

	ಸಂರಚನಾ fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */

	ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *multipart; /**< post data */

	fetch_about_handler handler;
};

ಸ್ಥಿರ ಸಂರಚನಾ fetch_about_context *ring = ರಿಕ್ತ;

/**
 * handler info for about scheme
 */
ಸಂರಚನಾ about_handlers {
	ಧ್ರುವ ವರ್ಣ *name; /**< name to match in url */
	ಪೂರ್ಣಾಙ್ಕ name_len;
	lwc_string *lname; /**< Interned name */
	fetch_about_handler handler; /**< handler for the url */
	bool hidden; /**< If entry should be hidden in listing */
};



/**
 * issue fetch callbacks with locking
 */
ಸ್ಥಿರ bool
fetch_about_send_callback(ಧ್ರುವ fetch_ಸಂದೇಶ *ಸಂದೇಶ, ಸಂರಚನಾ fetch_about_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(ಸಂದೇಶ, ctx->fetchh);
	ctx->locked = false;

	ಪ್ರತ್ಯಾಗಮ ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_finished(ಸಂರಚನಾ fetch_about_context *ctx)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	ಸಂದೇಶ.type = FETCH_FINISHED;
	ಪ್ರತ್ಯಾಗಮ fetch_about_send_callback(&ಸಂದೇಶ, ctx);
}

/* exported interface documented in about/private.h */
bool fetch_about_set_http_code(ಸಂರಚನಾ fetch_about_context *ctx, long code)
{
	fetch_set_http_code(ctx->fetchh, code);

	ಪ್ರತ್ಯಾಗಮ ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_header(ಸಂರಚನಾ fetch_about_context *ctx, ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	ವರ್ಣ header[64];
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(header, ಪರಿಮಾಣ header, fmt, ap);

	va_end(ap);

	ಸಂದೇಶ.type = FETCH_HEADER;
	ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *) header;
	ಸಂದೇಶ.data.header_or_data.len = strlen(header);

	ಪ್ರತ್ಯಾಗಮ fetch_about_send_callback(&ಸಂದೇಶ, ctx);
}

/* exported interface documented in about/private.h */
nserror
fetch_about_senddata(ಸಂರಚನಾ fetch_about_context *ctx, ಧ್ರುವ uint8_t *data, size_t data_len)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	ಸಂದೇಶ.type = FETCH_DATA;
	ಸಂದೇಶ.data.header_or_data.buf = data;
	ಸಂದೇಶ.data.header_or_data.len = data_len;

	ಯದಿ (fetch_about_send_callback(&ಸಂದೇಶ, ctx)) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_INVALID;
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/* exported interface documented in about/private.h */
nserror
fetch_about_ssenddataf(ಸಂರಚನಾ fetch_about_context *ctx, ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	ವರ್ಣ buffer[1024];
	ವರ್ಣ *dbuff;
	fetch_ಸಂದೇಶ ಸಂದೇಶ;
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ slen;

	va_start(ap, fmt);

	slen = vsnprintf(buffer, ಪರಿಮಾಣ(buffer), fmt, ap);

	va_end(ap);

	ಯದಿ (slen < (ಪೂರ್ಣಾಙ್ಕ)ಪರಿಮಾಣ(buffer)) {
		ಸಂದೇಶ.type = FETCH_DATA;
		ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *) buffer;
		ಸಂದೇಶ.data.header_or_data.len = slen;

		ಯದಿ (fetch_about_send_callback(&ಸಂದೇಶ, ctx)) {
			ಪ್ರತ್ಯಾಗಮ NSERROR_INVALID;
		}

		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	dbuff = malloc(slen + 1);
	ಯದಿ (dbuff == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOSPACE;
	}

	va_start(ap, fmt);

	slen = vsnprintf(dbuff, slen + 1, fmt, ap);

	va_end(ap);

	ಸಂದೇಶ.type = FETCH_DATA;
	ಸಂದೇಶ.data.header_or_data.buf = (ಧ್ರುವ uint8_t *)dbuff;
	ಸಂದೇಶ.data.header_or_data.len = slen;

	ಯದಿ (fetch_about_send_callback(&ಸಂದೇಶ, ctx)) {
		ಮುಕ್ತ(dbuff);
		ಪ್ರತ್ಯಾಗಮ NSERROR_INVALID;
	}

	ಮುಕ್ತ(dbuff);
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in about/private.h */
nsurl *fetch_about_get_url(ಸಂರಚನಾ fetch_about_context *ctx)
{
	ಪ್ರತ್ಯಾಗಮ ctx->url;
}


/* exported interface documented in about/private.h */
ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *
fetch_about_get_multipart(ಸಂರಚನಾ fetch_about_context *ctx)
{
	ಪ್ರತ್ಯಾಗಮ ctx->multipart;
}


/* exported interface documented in about/private.h */
bool fetch_about_srverror(ಸಂರಚನಾ fetch_about_context *ctx)
{
	nserror res;

	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_INTERNAL_SERVER_ERROR);

	/* content type */
	ಯದಿ (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		ಪ್ರತ್ಯಾಗಮ false;

	res = fetch_about_ssenddataf(ctx, "%s 500 %s",
				     messages_get("FetchErrorCode"),
				     messages_get("HTTP500"));
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	fetch_about_send_finished(ctx);

	ಪ್ರತ್ಯಾಗಮ true;
}


/* exported interface documented in about/private.h */
bool fetch_about_redirect(ಸಂರಚನಾ fetch_about_context *ctx, ಧ್ರುವ ವರ್ಣ *target)
{
	fetch_ಸಂದೇಶ ಸಂದೇಶ;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_FOUND);

	ಸಂದೇಶ.type = FETCH_REDIRECT;
	ಸಂದೇಶ.data.redirect = target;

	ಪ್ರತ್ಯಾಗಮ fetch_about_send_callback(&ಸಂದೇಶ, ctx);
}

/**
 * Handler to generate about scheme credits page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ಸ್ಥಿರ bool fetch_about_credits_handler(ಸಂರಚನಾ fetch_about_context *ctx)
{
	ಪ್ರತ್ಯಾಗಮ fetch_about_redirect(ctx, "resource:credits.html");
}


/**
 * Handler to generate about scheme licence page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ಸ್ಥಿರ bool fetch_about_licence_handler(ಸಂರಚನಾ fetch_about_context *ctx)
{
	ಪ್ರತ್ಯಾಗಮ fetch_about_redirect(ctx, "resource:licence.html");
}


/**
 * Handler to generate about scheme logo page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ಸ್ಥಿರ bool fetch_about_logo_handler(ಸಂರಚನಾ fetch_about_context *ctx)
{
	ಪ್ರತ್ಯಾಗಮ fetch_about_redirect(ctx, "resource:netsurf.png");
}


/**
 * Handler to generate about scheme welcome page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ಸ್ಥಿರ bool fetch_about_welcome_handler(ಸಂರಚನಾ fetch_about_context *ctx)
{
	ಪ್ರತ್ಯಾಗಮ fetch_about_redirect(ctx, "resource:welcome.html");
}


/* Forward declaration because this handler requires the handler table. */
ಸ್ಥಿರ bool fetch_about_about_handler(ಸಂರಚನಾ fetch_about_context *ctx);

/**
 * List of about paths and their handlers
 */
ಸಂರಚನಾ about_handlers about_handler_list[] = {
	{
		"credits",
		SLEN("credits"),
		ರಿಕ್ತ,
		fetch_about_credits_handler,
		false
	},
	{
		"licence",
		SLEN("licence"),
		ರಿಕ್ತ,
		fetch_about_licence_handler,
		false
	},
	{
		"license",
		SLEN("license"),
		ರಿಕ್ತ,
		fetch_about_licence_handler,
		true
	},
	{
		"welcome",
		SLEN("welcome"),
		ರಿಕ್ತ,
		fetch_about_welcome_handler,
		false
	},
	{
		"config",
		SLEN("config"),
		ರಿಕ್ತ,
		fetch_about_config_handler,
		false
	},
	{
		"Choices",
		SLEN("Choices"),
		ರಿಕ್ತ,
		fetch_about_choices_handler,
		false
	},
	{
		"testament",
		SLEN("testament"),
		ರಿಕ್ತ,
		fetch_about_testament_handler,
		false
	},
	{
		"about",
		SLEN("about"),
		ರಿಕ್ತ,
		fetch_about_about_handler,
		true
	},
	{
		"nscolours.css",
		SLEN("nscolours.css"),
		ರಿಕ್ತ,
		fetch_about_nscolours_handler,
		true
	},
	{
		"logo",
		SLEN("logo"),
		ರಿಕ್ತ,
		fetch_about_logo_handler,
		true
	},
	{
		/* details about the image cache */
		"imagecache",
		SLEN("imagecache"),
		ರಿಕ್ತ,
		fetch_about_imagecache_handler,
		true
	},
	{
		/* The default blank page */
		"blank",
		SLEN("blank"),
		ರಿಕ್ತ,
		fetch_about_blank_handler,
		true
	},
	{
		/* details about a certificate */
		"certificate",
		SLEN("certificate"),
		ರಿಕ್ತ,
		fetch_about_certificate_handler,
		true
	},
	{
		/* chart generator */
		"chart",
		SLEN("chart"),
		ರಿಕ್ತ,
		fetch_about_chart_handler,
		true
	},
	{
		"query/auth",
		SLEN("query/auth"),
		ರಿಕ್ತ,
		fetch_about_query_auth_handler,
		true
	},
	{
		"query/ssl",
		SLEN("query/ssl"),
		ರಿಕ್ತ,
		fetch_about_query_privacy_handler,
		true
	},
	{
		"query/timeout",
		SLEN("query/timeout"),
		ರಿಕ್ತ,
		fetch_about_query_timeout_handler,
		true
	},
	{
		"query/fetcherror",
		SLEN("query/fetcherror"),
		ರಿಕ್ತ,
		fetch_about_query_fetcherror_handler,
		true
	},
	{
		/* web search using the configured provider */
		"websearch",
		SLEN("websearch"),
		ರಿಕ್ತ,
		fetch_about_websearch_handler,
		true
	}
};

#define about_handler_list_len \
	(ಪರಿಮಾಣ(about_handler_list) / ಪರಿಮಾಣ(ಸಂರಚನಾ about_handlers))

/**
 * List all the valid about: paths available
 *
 * \param ctx The fetch context.
 * \return true for sucess or false to generate an error.
 */
ಸ್ಥಿರ bool fetch_about_about_handler(ಸಂರಚನಾ fetch_about_context *ctx)
{
	nserror res;
	unsigned ಪೂರ್ಣಾಙ್ಕ abt_loop = 0;

	/* content is going to return ok */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_OK);

	/* content type */
	ಯದಿ (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		ಗಚ್ಛ fetch_about_config_handler_aborted;

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>List of NetSurf pages</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class =\"ns-border\">List of NetSurf pages</h1>\n"
			"<ul>\n");
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_config_handler_aborted;
	}

	ಪ್ರತಿ (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {

		/* Skip over hidden entries */
		ಯದಿ (about_handler_list[abt_loop].hidden)
			ಅನುವರ್ತ;

		res = fetch_about_ssenddataf(ctx,
			       "<li><a href=\"about:%s\">about:%s</a></li>\n",
			       about_handler_list[abt_loop].name,
			       about_handler_list[abt_loop].name);
		ಯದಿ (res != NSERROR_OK) {
			ಗಚ್ಛ fetch_about_config_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</ul>\n</body>\n</html>\n");
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	ಪ್ರತ್ಯಾಗಮ true;

fetch_about_config_handler_aborted:
	ಪ್ರತ್ಯಾಗಮ false;
}

ಸ್ಥಿರ bool
fetch_about_404_handler(ಸಂರಚನಾ fetch_about_context *ctx)
{
	nserror res;
	ಧ್ರುವ ವರ್ಣ *title;

	/* content is going to return 404 */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_NOT_FOUND);

	/* content type */
	ಯದಿ (fetch_about_send_header(ctx,
			"Content-Type: text/html; charset=utf-8")) {
		ಪ್ರತ್ಯಾಗಮ false;
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
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	fetch_about_send_finished(ctx);

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * callback to initialise the about scheme fetcher.
 */
ಸ್ಥಿರ bool fetch_about_initialise(lwc_string *scheme)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ abt_loop = 0;
	lwc_error error;

	ಪ್ರತಿ (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		error = lwc_intern_string(about_handler_list[abt_loop].name,
					about_handler_list[abt_loop].name_len,
					&about_handler_list[abt_loop].lname);
		ಯದಿ (error != lwc_error_ok) {
			ಯಾವತ್ (abt_loop-- != 0) {
				lwc_string_unref(about_handler_list[abt_loop].lname);
			}
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * callback to finalise the about scheme fetcher.
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_about_finalise(lwc_string *scheme)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ abt_loop = 0;
	ಪ್ರತಿ (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		lwc_string_unref(about_handler_list[abt_loop].lname);
	}
}


ಸ್ಥಿರ bool fetch_about_can_fetch(ಧ್ರುವ nsurl *url)
{
	ಪ್ರತ್ಯಾಗಮ true;
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
fetch_about_setup(ಸಂರಚನಾ fetch *fetchh,
		  nsurl *url,
		  bool only_2xx,
		  bool downgrade_tls,
		  ಧ್ರುವ ವರ್ಣ *post_urlenc,
		  ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *post_multipart,
		  ಧ್ರುವ ವರ್ಣ **headers)
{
	ಸಂರಚನಾ fetch_about_context *ctx;
	unsigned ಪೂರ್ಣಾಙ್ಕ handler_loop;
	lwc_string *path;
	bool match;

	ctx = calloc(1, ಪರಿಮಾಣ(*ctx));
	ಯದಿ (ctx == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	path = nsurl_get_component(url, NSURL_PATH);

	ಪ್ರತಿ (handler_loop = 0;
	     handler_loop < about_handler_list_len;
	     handler_loop++) {
		ಯದಿ (lwc_string_isequal(path,
				       about_handler_list[handler_loop].lname,
				       &match) == lwc_error_ok && match) {
			ctx->handler = about_handler_list[handler_loop].handler;
			ಭಙ್ಗ;
		}
	}

	lwc_string_unref(path);

	ctx->fetchh = fetchh;
	ctx->url = nsurl_ref(url);
	ctx->multipart = post_multipart;

	RING_ಸೇರಿಸು(ring, ctx);

	ಪ್ರತ್ಯಾಗಮ ctx;
}


/**
 * callback to free a about scheme fetch
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_about_free(ಶೂನ್ಯ *ctx)
{
	ಸಂರಚನಾ fetch_about_context *c = ctx;
	nsurl_unref(c->url);
	ಮುಕ್ತ(ctx);
}


/**
 * callback to start an about scheme fetch
 */
ಸ್ಥಿರ bool fetch_about_start(ಶೂನ್ಯ *ctx)
{
	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * callback to abort a about fetch
 */
ಸ್ಥಿರ ಶೂನ್ಯ fetch_about_abort(ಶೂನ್ಯ *ctx)
{
	ಸಂರಚನಾ fetch_about_context *c = ctx;

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
	ಸಂರಚನಾ fetch_about_context *c, *save_ring = ರಿಕ್ತ;

	/* Iterate over ring, processing each pending fetch */
	ಯಾವತ್ (ring != ರಿಕ್ತ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ಯದಿ (c->locked == true) {
			RING_ಸೇರಿಸು(save_ring, c);
			ಅನುವರ್ತ;
		}

		/* Only process non-aborted fetches */
		ಯದಿ (c->aborted == false) {
			/* about fetches can be processed in one go */
			ಯದಿ (c->handler == ರಿಕ್ತ) {
				fetch_about_404_handler(c);
			} ಅನ್ಯಥಾ {
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


nserror fetch_about_register(ಶೂನ್ಯ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_about);
	ಧ್ರುವ ಸಂರಚನಾ fetcher_operation_table fetcher_ops = {
		.initialise = fetch_about_initialise,
		.acceptable = fetch_about_can_fetch,
		.setup = fetch_about_setup,
		.start = fetch_about_start,
		.abort = fetch_about_abort,
		.ಮುಕ್ತ = fetch_about_free,
		.poll = fetch_about_poll,
		.finalise = fetch_about_finalise
	};

	ಪ್ರತ್ಯಾಗಮ fetcher_add(scheme, &fetcher_ops);
}
