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

تعریفنوع bool (*fetch_about_handler)(ساختار fetch_about_context *);

/**
 * Context for an about fetch
 */
ساختار fetch_about_context {
	ساختار fetch_about_context *r_next, *r_prev;

	ساختار fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */

	ثابت ساختار fetch_multipart_data *multipart; /**< post data */

	fetch_about_handler handler;
};

ایستا ساختار fetch_about_context *ring = پوچ;

/**
 * handler info for about scheme
 */
ساختار about_handlers {
	ثابت نویسه *name; /**< name to match in url */
	صحیح name_len;
	lwc_string *lname; /**< Interned name */
	fetch_about_handler handler; /**< handler for the url */
	bool hidden; /**< If entry should be hidden in listing */
};



/**
 * issue fetch callbacks with locking
 */
ایستا bool
fetch_about_send_callback(ثابت fetch_پیام *پیام, ساختار fetch_about_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پیام, ctx->fetchh);
	ctx->locked = false;

	بازگشت ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_finished(ساختار fetch_about_context *ctx)
{
	fetch_پیام پیام;
	پیام.type = FETCH_FINISHED;
	بازگشت fetch_about_send_callback(&پیام, ctx);
}

/* exported interface documented in about/private.h */
bool fetch_about_set_http_code(ساختار fetch_about_context *ctx, long code)
{
	fetch_set_http_code(ctx->fetchh, code);

	بازگشت ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_header(ساختار fetch_about_context *ctx, ثابت نویسه *fmt, ...)
{
	نویسه header[64];
	fetch_پیام پیام;
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(header, اندازه header, fmt, ap);

	va_end(ap);

	پیام.type = FETCH_HEADER;
	پیام.data.header_or_data.buf = (ثابت uint8_t *) header;
	پیام.data.header_or_data.len = strlen(header);

	بازگشت fetch_about_send_callback(&پیام, ctx);
}

/* exported interface documented in about/private.h */
nserror
fetch_about_senddata(ساختار fetch_about_context *ctx, ثابت uint8_t *data, size_t data_len)
{
	fetch_پیام پیام;

	پیام.type = FETCH_DATA;
	پیام.data.header_or_data.buf = data;
	پیام.data.header_or_data.len = data_len;

	اگر (fetch_about_send_callback(&پیام, ctx)) {
		بازگشت NSERROR_INVALID;
	}

	بازگشت NSERROR_OK;
}

/* exported interface documented in about/private.h */
nserror
fetch_about_ssenddataf(ساختار fetch_about_context *ctx, ثابت نویسه *fmt, ...)
{
	نویسه buffer[1024];
	نویسه *dbuff;
	fetch_پیام پیام;
	va_list ap;
	صحیح slen;

	va_start(ap, fmt);

	slen = vsnprintf(buffer, اندازه(buffer), fmt, ap);

	va_end(ap);

	اگر (slen < (صحیح)اندازه(buffer)) {
		پیام.type = FETCH_DATA;
		پیام.data.header_or_data.buf = (ثابت uint8_t *) buffer;
		پیام.data.header_or_data.len = slen;

		اگر (fetch_about_send_callback(&پیام, ctx)) {
			بازگشت NSERROR_INVALID;
		}

		بازگشت NSERROR_OK;
	}

	dbuff = malloc(slen + 1);
	اگر (dbuff == پوچ) {
		بازگشت NSERROR_NOSPACE;
	}

	va_start(ap, fmt);

	slen = vsnprintf(dbuff, slen + 1, fmt, ap);

	va_end(ap);

	پیام.type = FETCH_DATA;
	پیام.data.header_or_data.buf = (ثابت uint8_t *)dbuff;
	پیام.data.header_or_data.len = slen;

	اگر (fetch_about_send_callback(&پیام, ctx)) {
		آزاد(dbuff);
		بازگشت NSERROR_INVALID;
	}

	آزاد(dbuff);
	بازگشت NSERROR_OK;
}


/* exported interface documented in about/private.h */
nsurl *fetch_about_get_url(ساختار fetch_about_context *ctx)
{
	بازگشت ctx->url;
}


/* exported interface documented in about/private.h */
ثابت ساختار fetch_multipart_data *
fetch_about_get_multipart(ساختار fetch_about_context *ctx)
{
	بازگشت ctx->multipart;
}


/* exported interface documented in about/private.h */
bool fetch_about_srverror(ساختار fetch_about_context *ctx)
{
	nserror res;

	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_INTERNAL_SERVER_ERROR);

	/* content type */
	اگر (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		بازگشت false;

	res = fetch_about_ssenddataf(ctx, "%s 500 %s",
				     messages_get("FetchErrorCode"),
				     messages_get("HTTP500"));
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	fetch_about_send_finished(ctx);

	بازگشت true;
}


/* exported interface documented in about/private.h */
bool fetch_about_redirect(ساختار fetch_about_context *ctx, ثابت نویسه *target)
{
	fetch_پیام پیام;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_FOUND);

	پیام.type = FETCH_REDIRECT;
	پیام.data.redirect = target;

	بازگشت fetch_about_send_callback(&پیام, ctx);
}

/**
 * Handler to generate about scheme credits page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ایستا bool fetch_about_credits_handler(ساختار fetch_about_context *ctx)
{
	بازگشت fetch_about_redirect(ctx, "resource:credits.html");
}


/**
 * Handler to generate about scheme licence page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ایستا bool fetch_about_licence_handler(ساختار fetch_about_context *ctx)
{
	بازگشت fetch_about_redirect(ctx, "resource:licence.html");
}


/**
 * Handler to generate about scheme logo page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ایستا bool fetch_about_logo_handler(ساختار fetch_about_context *ctx)
{
	بازگشت fetch_about_redirect(ctx, "resource:netsurf.png");
}


/**
 * Handler to generate about scheme welcome page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ایستا bool fetch_about_welcome_handler(ساختار fetch_about_context *ctx)
{
	بازگشت fetch_about_redirect(ctx, "resource:welcome.html");
}


/* Forward declaration because this handler requires the handler table. */
ایستا bool fetch_about_about_handler(ساختار fetch_about_context *ctx);

/**
 * List of about paths and their handlers
 */
ساختار about_handlers about_handler_list[] = {
	{
		"credits",
		SLEN("credits"),
		پوچ,
		fetch_about_credits_handler,
		false
	},
	{
		"licence",
		SLEN("licence"),
		پوچ,
		fetch_about_licence_handler,
		false
	},
	{
		"license",
		SLEN("license"),
		پوچ,
		fetch_about_licence_handler,
		true
	},
	{
		"welcome",
		SLEN("welcome"),
		پوچ,
		fetch_about_welcome_handler,
		false
	},
	{
		"config",
		SLEN("config"),
		پوچ,
		fetch_about_config_handler,
		false
	},
	{
		"Choices",
		SLEN("Choices"),
		پوچ,
		fetch_about_choices_handler,
		false
	},
	{
		"testament",
		SLEN("testament"),
		پوچ,
		fetch_about_testament_handler,
		false
	},
	{
		"about",
		SLEN("about"),
		پوچ,
		fetch_about_about_handler,
		true
	},
	{
		"nscolours.css",
		SLEN("nscolours.css"),
		پوچ,
		fetch_about_nscolours_handler,
		true
	},
	{
		"logo",
		SLEN("logo"),
		پوچ,
		fetch_about_logo_handler,
		true
	},
	{
		/* details about the image cache */
		"imagecache",
		SLEN("imagecache"),
		پوچ,
		fetch_about_imagecache_handler,
		true
	},
	{
		/* The default blank page */
		"blank",
		SLEN("blank"),
		پوچ,
		fetch_about_blank_handler,
		true
	},
	{
		/* details about a certificate */
		"certificate",
		SLEN("certificate"),
		پوچ,
		fetch_about_certificate_handler,
		true
	},
	{
		/* chart generator */
		"chart",
		SLEN("chart"),
		پوچ,
		fetch_about_chart_handler,
		true
	},
	{
		"query/auth",
		SLEN("query/auth"),
		پوچ,
		fetch_about_query_auth_handler,
		true
	},
	{
		"query/ssl",
		SLEN("query/ssl"),
		پوچ,
		fetch_about_query_privacy_handler,
		true
	},
	{
		"query/timeout",
		SLEN("query/timeout"),
		پوچ,
		fetch_about_query_timeout_handler,
		true
	},
	{
		"query/fetcherror",
		SLEN("query/fetcherror"),
		پوچ,
		fetch_about_query_fetcherror_handler,
		true
	},
	{
		/* web search using the configured provider */
		"websearch",
		SLEN("websearch"),
		پوچ,
		fetch_about_websearch_handler,
		true
	}
};

#define about_handler_list_len \
	(اندازه(about_handler_list) / اندازه(ساختار about_handlers))

/**
 * List all the valid about: paths available
 *
 * \param ctx The fetch context.
 * \return true for sucess or false to generate an error.
 */
ایستا bool fetch_about_about_handler(ساختار fetch_about_context *ctx)
{
	nserror res;
	unsigned صحیح abt_loop = 0;

	/* content is going to return ok */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_OK);

	/* content type */
	اگر (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		برو fetch_about_config_handler_aborted;

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
		برو fetch_about_config_handler_aborted;
	}

	برای (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {

		/* Skip over hidden entries */
		اگر (about_handler_list[abt_loop].hidden)
			ادامه;

		res = fetch_about_ssenddataf(ctx,
			       "<li><a href=\"about:%s\">about:%s</a></li>\n",
			       about_handler_list[abt_loop].name,
			       about_handler_list[abt_loop].name);
		اگر (res != NSERROR_OK) {
			برو fetch_about_config_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</ul>\n</body>\n</html>\n");
	اگر (res != NSERROR_OK) {
		برو fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	بازگشت true;

fetch_about_config_handler_aborted:
	بازگشت false;
}

ایستا bool
fetch_about_404_handler(ساختار fetch_about_context *ctx)
{
	nserror res;
	ثابت نویسه *title;

	/* content is going to return 404 */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_NOT_FOUND);

	/* content type */
	اگر (fetch_about_send_header(ctx,
			"Content-Type: text/html; charset=utf-8")) {
		بازگشت false;
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
		بازگشت false;
	}

	fetch_about_send_finished(ctx);

	بازگشت true;
}

/**
 * callback to initialise the about scheme fetcher.
 */
ایستا bool fetch_about_initialise(lwc_string *scheme)
{
	unsigned صحیح abt_loop = 0;
	lwc_error error;

	برای (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		error = lwc_intern_string(about_handler_list[abt_loop].name,
					about_handler_list[abt_loop].name_len,
					&about_handler_list[abt_loop].lname);
		اگر (error != lwc_error_ok) {
			وقتی (abt_loop-- != 0) {
				lwc_string_unref(about_handler_list[abt_loop].lname);
			}
			بازگشت false;
		}
	}

	بازگشت true;
}


/**
 * callback to finalise the about scheme fetcher.
 */
ایستا تهی fetch_about_finalise(lwc_string *scheme)
{
	unsigned صحیح abt_loop = 0;
	برای (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		lwc_string_unref(about_handler_list[abt_loop].lname);
	}
}


ایستا bool fetch_about_can_fetch(ثابت nsurl *url)
{
	بازگشت true;
}


/**
 * callback to set up a about scheme fetch.
 *
 * \param post_urlenc post data in urlenc format, owned by the llcache object
 *                        hence valid the entire lifetime of the fetch.
 * \param post_multipart post data in multipart format, owned by the llcache
 *                        object hence valid the entire lifetime of the fetch.
 */
ایستا تهی *
fetch_about_setup(ساختار fetch *fetchh,
		  nsurl *url,
		  bool only_2xx,
		  bool downgrade_tls,
		  ثابت نویسه *post_urlenc,
		  ثابت ساختار fetch_multipart_data *post_multipart,
		  ثابت نویسه **headers)
{
	ساختار fetch_about_context *ctx;
	unsigned صحیح handler_loop;
	lwc_string *path;
	bool match;

	ctx = calloc(1, اندازه(*ctx));
	اگر (ctx == پوچ)
		بازگشت پوچ;

	path = nsurl_get_component(url, NSURL_PATH);

	برای (handler_loop = 0;
	     handler_loop < about_handler_list_len;
	     handler_loop++) {
		اگر (lwc_string_isequal(path,
				       about_handler_list[handler_loop].lname,
				       &match) == lwc_error_ok && match) {
			ctx->handler = about_handler_list[handler_loop].handler;
			بشکن;
		}
	}

	lwc_string_unref(path);

	ctx->fetchh = fetchh;
	ctx->url = nsurl_ref(url);
	ctx->multipart = post_multipart;

	RING_درج(ring, ctx);

	بازگشت ctx;
}


/**
 * callback to free a about scheme fetch
 */
ایستا تهی fetch_about_free(تهی *ctx)
{
	ساختار fetch_about_context *c = ctx;
	nsurl_unref(c->url);
	آزاد(ctx);
}


/**
 * callback to start an about scheme fetch
 */
ایستا bool fetch_about_start(تهی *ctx)
{
	بازگشت true;
}


/**
 * callback to abort a about fetch
 */
ایستا تهی fetch_about_abort(تهی *ctx)
{
	ساختار fetch_about_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/**
 * callback to poll for additional about fetch contents
 */
ایستا تهی fetch_about_poll(lwc_string *scheme)
{
	ساختار fetch_about_context *c, *save_ring = پوچ;

	/* Iterate over ring, processing each pending fetch */
	وقتی (ring != پوچ) {
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
			RING_درج(save_ring, c);
			ادامه;
		}

		/* Only process non-aborted fetches */
		اگر (c->aborted == false) {
			/* about fetches can be processed in one go */
			اگر (c->handler == پوچ) {
				fetch_about_404_handler(c);
			} وگرنه {
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


nserror fetch_about_register(تهی)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_about);
	ثابت ساختار fetcher_operation_table fetcher_ops = {
		.initialise = fetch_about_initialise,
		.acceptable = fetch_about_can_fetch,
		.setup = fetch_about_setup,
		.start = fetch_about_start,
		.abort = fetch_about_abort,
		.آزاد = fetch_about_free,
		.poll = fetch_about_poll,
		.finalise = fetch_about_finalise
	};

	بازگشت fetcher_add(scheme, &fetcher_ops);
}
