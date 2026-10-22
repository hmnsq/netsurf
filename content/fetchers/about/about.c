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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "netsurf/inttypes.h"

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/ring.h"
#অন্তর্ভাব "utils/messages.h"

#অন্তর্ভাব "content/fetch.h"
#অন্তর্ভাব "content/fetchers.h"

#অন্তর্ভাব "private.h"
#অন্তর্ভাব "about.h"
#অন্তর্ভাব "blank.h"
#অন্তর্ভাব "certificate.h"
#অন্তর্ভাব "config.h"
#অন্তর্ভাব "chart.h"
#অন্তর্ভাব "choices.h"
#অন্তর্ভাব "imagecache.h"
#অন্তর্ভাব "nscolours.h"
#অন্তর্ভাব "query.h"
#অন্তর্ভাব "query_auth.h"
#অন্তর্ভাব "query_fetcherror.h"
#অন্তর্ভাব "query_privacy.h"
#অন্তর্ভাব "query_timeout.h"
#অন্তর্ভাব "atestament.h"
#অন্তর্ভাব "websearch.h"

প্রকারনির্দেশ bool (*fetch_about_handler)(সংরচনা fetch_about_context *);

/**
 * Context for an about fetch
 */
সংরচনা fetch_about_context {
	সংরচনা fetch_about_context *r_next, *r_prev;

	সংরচনা fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */

	ধ্রুব সংরচনা fetch_multipart_data *multipart; /**< post data */

	fetch_about_handler handler;
};

স্থির সংরচনা fetch_about_context *ring = রিক্ত;

/**
 * handler info for about scheme
 */
সংরচনা about_handlers {
	ধ্রুব বর্ণ *name; /**< name to match in url */
	পূর্ণাঙ্ক name_len;
	lwc_string *lname; /**< Interned name */
	fetch_about_handler handler; /**< handler for the url */
	bool hidden; /**< If entry should be hidden in listing */
};



/**
 * issue fetch callbacks with locking
 */
স্থির bool
fetch_about_send_callback(ধ্রুব fetch_বার্তা *বার্তা, সংরচনা fetch_about_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(বার্তা, ctx->fetchh);
	ctx->locked = false;

	প্রত্যাগম ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_finished(সংরচনা fetch_about_context *ctx)
{
	fetch_বার্তা বার্তা;
	বার্তা.type = FETCH_FINISHED;
	প্রত্যাগম fetch_about_send_callback(&বার্তা, ctx);
}

/* exported interface documented in about/private.h */
bool fetch_about_set_http_code(সংরচনা fetch_about_context *ctx, long code)
{
	fetch_set_http_code(ctx->fetchh, code);

	প্রত্যাগম ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_header(সংরচনা fetch_about_context *ctx, ধ্রুব বর্ণ *fmt, ...)
{
	বর্ণ header[64];
	fetch_বার্তা বার্তা;
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(header, পরিমাণ header, fmt, ap);

	va_end(ap);

	বার্তা.type = FETCH_HEADER;
	বার্তা.data.header_or_data.buf = (ধ্রুব uint8_t *) header;
	বার্তা.data.header_or_data.len = strlen(header);

	প্রত্যাগম fetch_about_send_callback(&বার্তা, ctx);
}

/* exported interface documented in about/private.h */
nserror
fetch_about_senddata(সংরচনা fetch_about_context *ctx, ধ্রুব uint8_t *data, size_t data_len)
{
	fetch_বার্তা বার্তা;

	বার্তা.type = FETCH_DATA;
	বার্তা.data.header_or_data.buf = data;
	বার্তা.data.header_or_data.len = data_len;

	যদি (fetch_about_send_callback(&বার্তা, ctx)) {
		প্রত্যাগম NSERROR_INVALID;
	}

	প্রত্যাগম NSERROR_OK;
}

/* exported interface documented in about/private.h */
nserror
fetch_about_ssenddataf(সংরচনা fetch_about_context *ctx, ধ্রুব বর্ণ *fmt, ...)
{
	বর্ণ buffer[1024];
	বর্ণ *dbuff;
	fetch_বার্তা বার্তা;
	va_list ap;
	পূর্ণাঙ্ক slen;

	va_start(ap, fmt);

	slen = vsnprintf(buffer, পরিমাণ(buffer), fmt, ap);

	va_end(ap);

	যদি (slen < (পূর্ণাঙ্ক)পরিমাণ(buffer)) {
		বার্তা.type = FETCH_DATA;
		বার্তা.data.header_or_data.buf = (ধ্রুব uint8_t *) buffer;
		বার্তা.data.header_or_data.len = slen;

		যদি (fetch_about_send_callback(&বার্তা, ctx)) {
			প্রত্যাগম NSERROR_INVALID;
		}

		প্রত্যাগম NSERROR_OK;
	}

	dbuff = malloc(slen + 1);
	যদি (dbuff == রিক্ত) {
		প্রত্যাগম NSERROR_NOSPACE;
	}

	va_start(ap, fmt);

	slen = vsnprintf(dbuff, slen + 1, fmt, ap);

	va_end(ap);

	বার্তা.type = FETCH_DATA;
	বার্তা.data.header_or_data.buf = (ধ্রুব uint8_t *)dbuff;
	বার্তা.data.header_or_data.len = slen;

	যদি (fetch_about_send_callback(&বার্তা, ctx)) {
		মুক্ত(dbuff);
		প্রত্যাগম NSERROR_INVALID;
	}

	মুক্ত(dbuff);
	প্রত্যাগম NSERROR_OK;
}


/* exported interface documented in about/private.h */
nsurl *fetch_about_get_url(সংরচনা fetch_about_context *ctx)
{
	প্রত্যাগম ctx->url;
}


/* exported interface documented in about/private.h */
ধ্রুব সংরচনা fetch_multipart_data *
fetch_about_get_multipart(সংরচনা fetch_about_context *ctx)
{
	প্রত্যাগম ctx->multipart;
}


/* exported interface documented in about/private.h */
bool fetch_about_srverror(সংরচনা fetch_about_context *ctx)
{
	nserror res;

	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_INTERNAL_SERVER_ERROR);

	/* content type */
	যদি (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		প্রত্যাগম false;

	res = fetch_about_ssenddataf(ctx, "%s 500 %s",
				     messages_get("FetchErrorCode"),
				     messages_get("HTTP500"));
	যদি (res != NSERROR_OK) {
		প্রত্যাগম false;
	}

	fetch_about_send_finished(ctx);

	প্রত্যাগম true;
}


/* exported interface documented in about/private.h */
bool fetch_about_redirect(সংরচনা fetch_about_context *ctx, ধ্রুব বর্ণ *target)
{
	fetch_বার্তা বার্তা;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_FOUND);

	বার্তা.type = FETCH_REDIRECT;
	বার্তা.data.redirect = target;

	প্রত্যাগম fetch_about_send_callback(&বার্তা, ctx);
}

/**
 * Handler to generate about scheme credits page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
স্থির bool fetch_about_credits_handler(সংরচনা fetch_about_context *ctx)
{
	প্রত্যাগম fetch_about_redirect(ctx, "resource:credits.html");
}


/**
 * Handler to generate about scheme licence page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
স্থির bool fetch_about_licence_handler(সংরচনা fetch_about_context *ctx)
{
	প্রত্যাগম fetch_about_redirect(ctx, "resource:licence.html");
}


/**
 * Handler to generate about scheme logo page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
স্থির bool fetch_about_logo_handler(সংরচনা fetch_about_context *ctx)
{
	প্রত্যাগম fetch_about_redirect(ctx, "resource:netsurf.png");
}


/**
 * Handler to generate about scheme welcome page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
স্থির bool fetch_about_welcome_handler(সংরচনা fetch_about_context *ctx)
{
	প্রত্যাগম fetch_about_redirect(ctx, "resource:welcome.html");
}


/* Forward declaration because this handler requires the handler table. */
স্থির bool fetch_about_about_handler(সংরচনা fetch_about_context *ctx);

/**
 * List of about paths and their handlers
 */
সংরচনা about_handlers about_handler_list[] = {
	{
		"credits",
		SLEN("credits"),
		রিক্ত,
		fetch_about_credits_handler,
		false
	},
	{
		"licence",
		SLEN("licence"),
		রিক্ত,
		fetch_about_licence_handler,
		false
	},
	{
		"license",
		SLEN("license"),
		রিক্ত,
		fetch_about_licence_handler,
		true
	},
	{
		"welcome",
		SLEN("welcome"),
		রিক্ত,
		fetch_about_welcome_handler,
		false
	},
	{
		"config",
		SLEN("config"),
		রিক্ত,
		fetch_about_config_handler,
		false
	},
	{
		"Choices",
		SLEN("Choices"),
		রিক্ত,
		fetch_about_choices_handler,
		false
	},
	{
		"testament",
		SLEN("testament"),
		রিক্ত,
		fetch_about_testament_handler,
		false
	},
	{
		"about",
		SLEN("about"),
		রিক্ত,
		fetch_about_about_handler,
		true
	},
	{
		"nscolours.css",
		SLEN("nscolours.css"),
		রিক্ত,
		fetch_about_nscolours_handler,
		true
	},
	{
		"logo",
		SLEN("logo"),
		রিক্ত,
		fetch_about_logo_handler,
		true
	},
	{
		/* details about the image cache */
		"imagecache",
		SLEN("imagecache"),
		রিক্ত,
		fetch_about_imagecache_handler,
		true
	},
	{
		/* The default blank page */
		"blank",
		SLEN("blank"),
		রিক্ত,
		fetch_about_blank_handler,
		true
	},
	{
		/* details about a certificate */
		"certificate",
		SLEN("certificate"),
		রিক্ত,
		fetch_about_certificate_handler,
		true
	},
	{
		/* chart generator */
		"chart",
		SLEN("chart"),
		রিক্ত,
		fetch_about_chart_handler,
		true
	},
	{
		"query/auth",
		SLEN("query/auth"),
		রিক্ত,
		fetch_about_query_auth_handler,
		true
	},
	{
		"query/ssl",
		SLEN("query/ssl"),
		রিক্ত,
		fetch_about_query_privacy_handler,
		true
	},
	{
		"query/timeout",
		SLEN("query/timeout"),
		রিক্ত,
		fetch_about_query_timeout_handler,
		true
	},
	{
		"query/fetcherror",
		SLEN("query/fetcherror"),
		রিক্ত,
		fetch_about_query_fetcherror_handler,
		true
	},
	{
		/* web search using the configured provider */
		"websearch",
		SLEN("websearch"),
		রিক্ত,
		fetch_about_websearch_handler,
		true
	}
};

#define about_handler_list_len \
	(পরিমাণ(about_handler_list) / পরিমাণ(সংরচনা about_handlers))

/**
 * List all the valid about: paths available
 *
 * \param ctx The fetch context.
 * \return true for sucess or false to generate an error.
 */
স্থির bool fetch_about_about_handler(সংরচনা fetch_about_context *ctx)
{
	nserror res;
	unsigned পূর্ণাঙ্ক abt_loop = 0;

	/* content is going to return ok */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_OK);

	/* content type */
	যদি (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		গচ্ছ fetch_about_config_handler_aborted;

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>List of NetSurf pages</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class =\"ns-border\">List of NetSurf pages</h1>\n"
			"<ul>\n");
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_config_handler_aborted;
	}

	প্রতি (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {

		/* Skip over hidden entries */
		যদি (about_handler_list[abt_loop].hidden)
			অনুবর্ত;

		res = fetch_about_ssenddataf(ctx,
			       "<li><a href=\"about:%s\">about:%s</a></li>\n",
			       about_handler_list[abt_loop].name,
			       about_handler_list[abt_loop].name);
		যদি (res != NSERROR_OK) {
			গচ্ছ fetch_about_config_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</ul>\n</body>\n</html>\n");
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	প্রত্যাগম true;

fetch_about_config_handler_aborted:
	প্রত্যাগম false;
}

স্থির bool
fetch_about_404_handler(সংরচনা fetch_about_context *ctx)
{
	nserror res;
	ধ্রুব বর্ণ *title;

	/* content is going to return 404 */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_NOT_FOUND);

	/* content type */
	যদি (fetch_about_send_header(ctx,
			"Content-Type: text/html; charset=utf-8")) {
		প্রত্যাগম false;
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
	যদি (res != NSERROR_OK) {
		প্রত্যাগম false;
	}

	fetch_about_send_finished(ctx);

	প্রত্যাগম true;
}

/**
 * callback to initialise the about scheme fetcher.
 */
স্থির bool fetch_about_initialise(lwc_string *scheme)
{
	unsigned পূর্ণাঙ্ক abt_loop = 0;
	lwc_error error;

	প্রতি (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		error = lwc_intern_string(about_handler_list[abt_loop].name,
					about_handler_list[abt_loop].name_len,
					&about_handler_list[abt_loop].lname);
		যদি (error != lwc_error_ok) {
			যাবত্ (abt_loop-- != 0) {
				lwc_string_unref(about_handler_list[abt_loop].lname);
			}
			প্রত্যাগম false;
		}
	}

	প্রত্যাগম true;
}


/**
 * callback to finalise the about scheme fetcher.
 */
স্থির শূন্য fetch_about_finalise(lwc_string *scheme)
{
	unsigned পূর্ণাঙ্ক abt_loop = 0;
	প্রতি (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		lwc_string_unref(about_handler_list[abt_loop].lname);
	}
}


স্থির bool fetch_about_can_fetch(ধ্রুব nsurl *url)
{
	প্রত্যাগম true;
}


/**
 * callback to set up a about scheme fetch.
 *
 * \param post_urlenc post data in urlenc format, owned by the llcache object
 *                        hence valid the entire lifetime of the fetch.
 * \param post_multipart post data in multipart format, owned by the llcache
 *                        object hence valid the entire lifetime of the fetch.
 */
স্থির শূন্য *
fetch_about_setup(সংরচনা fetch *fetchh,
		  nsurl *url,
		  bool only_2xx,
		  bool downgrade_tls,
		  ধ্রুব বর্ণ *post_urlenc,
		  ধ্রুব সংরচনা fetch_multipart_data *post_multipart,
		  ধ্রুব বর্ণ **headers)
{
	সংরচনা fetch_about_context *ctx;
	unsigned পূর্ণাঙ্ক handler_loop;
	lwc_string *path;
	bool match;

	ctx = calloc(1, পরিমাণ(*ctx));
	যদি (ctx == রিক্ত)
		প্রত্যাগম রিক্ত;

	path = nsurl_get_component(url, NSURL_PATH);

	প্রতি (handler_loop = 0;
	     handler_loop < about_handler_list_len;
	     handler_loop++) {
		যদি (lwc_string_isequal(path,
				       about_handler_list[handler_loop].lname,
				       &match) == lwc_error_ok && match) {
			ctx->handler = about_handler_list[handler_loop].handler;
			ভঙ্গ;
		}
	}

	lwc_string_unref(path);

	ctx->fetchh = fetchh;
	ctx->url = nsurl_ref(url);
	ctx->multipart = post_multipart;

	RING_ঢোকাও(ring, ctx);

	প্রত্যাগম ctx;
}


/**
 * callback to free a about scheme fetch
 */
স্থির শূন্য fetch_about_free(শূন্য *ctx)
{
	সংরচনা fetch_about_context *c = ctx;
	nsurl_unref(c->url);
	মুক্ত(ctx);
}


/**
 * callback to start an about scheme fetch
 */
স্থির bool fetch_about_start(শূন্য *ctx)
{
	প্রত্যাগম true;
}


/**
 * callback to abort a about fetch
 */
স্থির শূন্য fetch_about_abort(শূন্য *ctx)
{
	সংরচনা fetch_about_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/**
 * callback to poll for additional about fetch contents
 */
স্থির শূন্য fetch_about_poll(lwc_string *scheme)
{
	সংরচনা fetch_about_context *c, *save_ring = রিক্ত;

	/* Iterate over ring, processing each pending fetch */
	যাবত্ (ring != রিক্ত) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		যদি (c->locked == true) {
			RING_ঢোকাও(save_ring, c);
			অনুবর্ত;
		}

		/* Only process non-aborted fetches */
		যদি (c->aborted == false) {
			/* about fetches can be processed in one go */
			যদি (c->handler == রিক্ত) {
				fetch_about_404_handler(c);
			} অন্যথা {
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


nserror fetch_about_register(শূন্য)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_about);
	ধ্রুব সংরচনা fetcher_operation_table fetcher_ops = {
		.initialise = fetch_about_initialise,
		.acceptable = fetch_about_can_fetch,
		.setup = fetch_about_setup,
		.start = fetch_about_start,
		.abort = fetch_about_abort,
		.মুক্ত = fetch_about_free,
		.poll = fetch_about_poll,
		.finalise = fetch_about_finalise
	};

	প্রত্যাগম fetcher_add(scheme, &fetcher_ops);
}
