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

#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "netsurf/inttypes.h"

#include "utils/errors.h"
#include "utils/nsurl.h"
#include "utils/corestrings.h"
#include "utils/utils.h"
#include "utils/ring.h"
#include "utils/messages.h"

#include "content/fetch.h"
#include "content/fetchers.h"

#include "private.h"
#include "about.h"
#include "blank.h"
#include "certificate.h"
#include "config.h"
#include "chart.h"
#include "choices.h"
#include "imagecache.h"
#include "nscolours.h"
#include "query.h"
#include "query_auth.h"
#include "query_fetcherror.h"
#include "query_privacy.h"
#include "query_timeout.h"
#include "atestament.h"
#include "websearch.h"

typedef bool (*fetch_about_handler)(struct fetch_about_context *);

/**
 * Context for an about fetch
 */
struct fetch_about_context {
	struct fetch_about_context *r_next, *r_prev;

	struct fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */

	const struct fetch_multipart_data *multipart; /**< post data */

	fetch_about_handler handler;
};

static struct fetch_about_context *ring = NULL;

/**
 * handler info for about scheme
 */
struct about_handlers {
	const char *name; /**< name to match in url */
	цяло name_len;
	lwc_string *lname; /**< Interned name */
	fetch_about_handler handler; /**< handler for the url */
	bool hidden; /**< If entry should be hidden in listing */
};



/**
 * issue fetch callbacks with locking
 */
static bool
fetch_about_send_callback(const fetch_msg *msg, struct fetch_about_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(msg, ctx->fetchh);
	ctx->locked = false;

	върни ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_finished(struct fetch_about_context *ctx)
{
	fetch_msg msg;
	msg.type = FETCH_FINISHED;
	върни fetch_about_send_callback(&msg, ctx);
}

/* exported interface documented in about/private.h */
bool fetch_about_set_http_code(struct fetch_about_context *ctx, long code)
{
	fetch_set_http_code(ctx->fetchh, code);

	върни ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_header(struct fetch_about_context *ctx, const char *fmt, ...)
{
	char header[64];
	fetch_msg msg;
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(header, sizeof header, fmt, ap);

	va_end(ap);

	msg.type = FETCH_HEADER;
	msg.data.header_or_data.buf = (const uint8_t *) header;
	msg.data.header_or_data.len = strlen(header);

	върни fetch_about_send_callback(&msg, ctx);
}

/* exported interface documented in about/private.h */
nserror
fetch_about_senddata(struct fetch_about_context *ctx, const uint8_t *data, size_t data_len)
{
	fetch_msg msg;

	msg.type = FETCH_DATA;
	msg.data.header_or_data.buf = data;
	msg.data.header_or_data.len = data_len;

	ако (fetch_about_send_callback(&msg, ctx)) {
		върни NSERROR_INVALID;
	}

	върни NSERROR_OK;
}

/* exported interface documented in about/private.h */
nserror
fetch_about_ssenddataf(struct fetch_about_context *ctx, const char *fmt, ...)
{
	char buffer[1024];
	char *dbuff;
	fetch_msg msg;
	va_list ap;
	цяло slen;

	va_start(ap, fmt);

	slen = vsnprintf(buffer, sizeof(buffer), fmt, ap);

	va_end(ap);

	ако (slen < (цяло)sizeof(buffer)) {
		msg.type = FETCH_DATA;
		msg.data.header_or_data.buf = (const uint8_t *) buffer;
		msg.data.header_or_data.len = slen;

		ако (fetch_about_send_callback(&msg, ctx)) {
			върни NSERROR_INVALID;
		}

		върни NSERROR_OK;
	}

	dbuff = malloc(slen + 1);
	ако (dbuff == NULL) {
		върни NSERROR_NOSPACE;
	}

	va_start(ap, fmt);

	slen = vsnprintf(dbuff, slen + 1, fmt, ap);

	va_end(ap);

	msg.type = FETCH_DATA;
	msg.data.header_or_data.buf = (const uint8_t *)dbuff;
	msg.data.header_or_data.len = slen;

	ако (fetch_about_send_callback(&msg, ctx)) {
		free(dbuff);
		върни NSERROR_INVALID;
	}

	free(dbuff);
	върни NSERROR_OK;
}


/* exported interface documented in about/private.h */
nsurl *fetch_about_get_url(struct fetch_about_context *ctx)
{
	върни ctx->url;
}


/* exported interface documented in about/private.h */
const struct fetch_multipart_data *
fetch_about_get_multipart(struct fetch_about_context *ctx)
{
	върни ctx->multipart;
}


/* exported interface documented in about/private.h */
bool fetch_about_srverror(struct fetch_about_context *ctx)
{
	nserror res;

	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_INTERNAL_SERVER_ERROR);

	/* content type */
	ако (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		върни false;

	res = fetch_about_ssenddataf(ctx, "%s 500 %s",
				     messages_get("FetchErrorCode"),
				     messages_get("HTTP500"));
	ако (res != NSERROR_OK) {
		върни false;
	}

	fetch_about_send_finished(ctx);

	върни true;
}


/* exported interface documented in about/private.h */
bool fetch_about_redirect(struct fetch_about_context *ctx, const char *target)
{
	fetch_msg msg;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_FOUND);

	msg.type = FETCH_REDIRECT;
	msg.data.redirect = target;

	върни fetch_about_send_callback(&msg, ctx);
}

/**
 * Handler to generate about scheme credits page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
static bool fetch_about_credits_handler(struct fetch_about_context *ctx)
{
	върни fetch_about_redirect(ctx, "resource:credits.html");
}


/**
 * Handler to generate about scheme licence page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
static bool fetch_about_licence_handler(struct fetch_about_context *ctx)
{
	върни fetch_about_redirect(ctx, "resource:licence.html");
}


/**
 * Handler to generate about scheme logo page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
static bool fetch_about_logo_handler(struct fetch_about_context *ctx)
{
	върни fetch_about_redirect(ctx, "resource:netsurf.png");
}


/**
 * Handler to generate about scheme welcome page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
static bool fetch_about_welcome_handler(struct fetch_about_context *ctx)
{
	върни fetch_about_redirect(ctx, "resource:welcome.html");
}


/* Forward declaration because this handler requires the handler table. */
static bool fetch_about_about_handler(struct fetch_about_context *ctx);

/**
 * List of about paths and their handlers
 */
struct about_handlers about_handler_list[] = {
	{
		"credits",
		SLEN("credits"),
		NULL,
		fetch_about_credits_handler,
		false
	},
	{
		"licence",
		SLEN("licence"),
		NULL,
		fetch_about_licence_handler,
		false
	},
	{
		"license",
		SLEN("license"),
		NULL,
		fetch_about_licence_handler,
		true
	},
	{
		"welcome",
		SLEN("welcome"),
		NULL,
		fetch_about_welcome_handler,
		false
	},
	{
		"config",
		SLEN("config"),
		NULL,
		fetch_about_config_handler,
		false
	},
	{
		"Choices",
		SLEN("Choices"),
		NULL,
		fetch_about_choices_handler,
		false
	},
	{
		"testament",
		SLEN("testament"),
		NULL,
		fetch_about_testament_handler,
		false
	},
	{
		"about",
		SLEN("about"),
		NULL,
		fetch_about_about_handler,
		true
	},
	{
		"nscolours.css",
		SLEN("nscolours.css"),
		NULL,
		fetch_about_nscolours_handler,
		true
	},
	{
		"logo",
		SLEN("logo"),
		NULL,
		fetch_about_logo_handler,
		true
	},
	{
		/* details about the image cache */
		"imagecache",
		SLEN("imagecache"),
		NULL,
		fetch_about_imagecache_handler,
		true
	},
	{
		/* The default blank page */
		"blank",
		SLEN("blank"),
		NULL,
		fetch_about_blank_handler,
		true
	},
	{
		/* details about a certificate */
		"certificate",
		SLEN("certificate"),
		NULL,
		fetch_about_certificate_handler,
		true
	},
	{
		/* chart generator */
		"chart",
		SLEN("chart"),
		NULL,
		fetch_about_chart_handler,
		true
	},
	{
		"query/auth",
		SLEN("query/auth"),
		NULL,
		fetch_about_query_auth_handler,
		true
	},
	{
		"query/ssl",
		SLEN("query/ssl"),
		NULL,
		fetch_about_query_privacy_handler,
		true
	},
	{
		"query/timeout",
		SLEN("query/timeout"),
		NULL,
		fetch_about_query_timeout_handler,
		true
	},
	{
		"query/fetcherror",
		SLEN("query/fetcherror"),
		NULL,
		fetch_about_query_fetcherror_handler,
		true
	},
	{
		/* web search using the configured provider */
		"websearch",
		SLEN("websearch"),
		NULL,
		fetch_about_websearch_handler,
		true
	}
};

#define about_handler_list_len \
	(sizeof(about_handler_list) / sizeof(struct about_handlers))

/**
 * List all the valid about: paths available
 *
 * \param ctx The fetch context.
 * \return true for sucess or false to generate an error.
 */
static bool fetch_about_about_handler(struct fetch_about_context *ctx)
{
	nserror res;
	unsigned цяло abt_loop = 0;

	/* content is going to return ok */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_OK);

	/* content type */
	ако (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		goto fetch_about_config_handler_aborted;

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>List of NetSurf pages</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class =\"ns-border\">List of NetSurf pages</h1>\n"
			"<ul>\n");
	ако (res != NSERROR_OK) {
		goto fetch_about_config_handler_aborted;
	}

	за (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {

		/* Skip over hidden entries */
		ако (about_handler_list[abt_loop].hidden)
			continue;

		res = fetch_about_ssenddataf(ctx,
			       "<li><a href=\"about:%s\">about:%s</a></li>\n",
			       about_handler_list[abt_loop].name,
			       about_handler_list[abt_loop].name);
		ако (res != NSERROR_OK) {
			goto fetch_about_config_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</ul>\n</body>\n</html>\n");
	ако (res != NSERROR_OK) {
		goto fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	върни true;

fetch_about_config_handler_aborted:
	върни false;
}

static bool
fetch_about_404_handler(struct fetch_about_context *ctx)
{
	nserror res;
	const char *title;

	/* content is going to return 404 */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_NOT_FOUND);

	/* content type */
	ако (fetch_about_send_header(ctx,
			"Content-Type: text/html; charset=utf-8")) {
		върни false;
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
	ако (res != NSERROR_OK) {
		върни false;
	}

	fetch_about_send_finished(ctx);

	върни true;
}

/**
 * callback to initialise the about scheme fetcher.
 */
static bool fetch_about_initialise(lwc_string *scheme)
{
	unsigned цяло abt_loop = 0;
	lwc_error error;

	за (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		error = lwc_intern_string(about_handler_list[abt_loop].name,
					about_handler_list[abt_loop].name_len,
					&about_handler_list[abt_loop].lname);
		ако (error != lwc_error_ok) {
			докато (abt_loop-- != 0) {
				lwc_string_unref(about_handler_list[abt_loop].lname);
			}
			върни false;
		}
	}

	върни true;
}


/**
 * callback to finalise the about scheme fetcher.
 */
static void fetch_about_finalise(lwc_string *scheme)
{
	unsigned цяло abt_loop = 0;
	за (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		lwc_string_unref(about_handler_list[abt_loop].lname);
	}
}


static bool fetch_about_can_fetch(const nsurl *url)
{
	върни true;
}


/**
 * callback to set up a about scheme fetch.
 *
 * \param post_urlenc post data in urlenc format, owned by the llcache object
 *                        hence valid the entire lifetime of the fetch.
 * \param post_multipart post data in multipart format, owned by the llcache
 *                        object hence valid the entire lifetime of the fetch.
 */
static void *
fetch_about_setup(struct fetch *fetchh,
		  nsurl *url,
		  bool only_2xx,
		  bool downgrade_tls,
		  const char *post_urlenc,
		  const struct fetch_multipart_data *post_multipart,
		  const char **headers)
{
	struct fetch_about_context *ctx;
	unsigned цяло handler_loop;
	lwc_string *path;
	bool match;

	ctx = calloc(1, sizeof(*ctx));
	ако (ctx == NULL)
		върни NULL;

	path = nsurl_get_component(url, NSURL_PATH);

	за (handler_loop = 0;
	     handler_loop < about_handler_list_len;
	     handler_loop++) {
		ако (lwc_string_isequal(path,
				       about_handler_list[handler_loop].lname,
				       &match) == lwc_error_ok && match) {
			ctx->handler = about_handler_list[handler_loop].handler;
			break;
		}
	}

	lwc_string_unref(path);

	ctx->fetchh = fetchh;
	ctx->url = nsurl_ref(url);
	ctx->multipart = post_multipart;

	RING_INSERT(ring, ctx);

	върни ctx;
}


/**
 * callback to free a about scheme fetch
 */
static void fetch_about_free(void *ctx)
{
	struct fetch_about_context *c = ctx;
	nsurl_unref(c->url);
	free(ctx);
}


/**
 * callback to start an about scheme fetch
 */
static bool fetch_about_start(void *ctx)
{
	върни true;
}


/**
 * callback to abort a about fetch
 */
static void fetch_about_abort(void *ctx)
{
	struct fetch_about_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/**
 * callback to poll for additional about fetch contents
 */
static void fetch_about_poll(lwc_string *scheme)
{
	struct fetch_about_context *c, *save_ring = NULL;

	/* Iterate over ring, processing each pending fetch */
	докато (ring != NULL) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		ако (c->locked == true) {
			RING_INSERT(save_ring, c);
			continue;
		}

		/* Only process non-aborted fetches */
		ако (c->aborted == false) {
			/* about fetches can be processed in one go */
			ако (c->handler == NULL) {
				fetch_about_404_handler(c);
			} иначе {
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


nserror fetch_about_register(void)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_about);
	const struct fetcher_operation_table fetcher_ops = {
		.initialise = fetch_about_initialise,
		.acceptable = fetch_about_can_fetch,
		.setup = fetch_about_setup,
		.start = fetch_about_start,
		.abort = fetch_about_abort,
		.free = fetch_about_free,
		.poll = fetch_about_poll,
		.finalise = fetch_about_finalise
	};

	върни fetcher_add(scheme, &fetcher_ops);
}
