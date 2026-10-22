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

#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdarg.h>
#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <stdlib.h>

#ضمن "netsurf/inttypes.h"

#ضمن "utils/errors.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/utils.h"
#ضمن "utils/ring.h"
#ضمن "utils/messages.h"

#ضمن "content/fetch.h"
#ضمن "content/fetchers.h"

#ضمن "private.h"
#ضمن "about.h"
#ضمن "blank.h"
#ضمن "certificate.h"
#ضمن "config.h"
#ضمن "chart.h"
#ضمن "choices.h"
#ضمن "imagecache.h"
#ضمن "nscolours.h"
#ضمن "query.h"
#ضمن "query_auth.h"
#ضمن "query_fetcherror.h"
#ضمن "query_privacy.h"
#ضمن "query_timeout.h"
#ضمن "atestament.h"
#ضمن "websearch.h"

تعريفنوع bool (*fetch_about_handler)(بنية fetch_about_context *);

/**
 * Context for an about fetch
 */
بنية fetch_about_context {
	بنية fetch_about_context *r_next, *r_prev;

	بنية fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */

	ثابت بنية fetch_multipart_data *multipart; /**< post data */

	fetch_about_handler handler;
};

ساكن بنية fetch_about_context *ring = عدم;

/**
 * handler info for about scheme
 */
بنية about_handlers {
	ثابت حرف *name; /**< name to match in url */
	صحيح name_len;
	lwc_string *lname; /**< Interned name */
	fetch_about_handler handler; /**< handler for the url */
	bool hidden; /**< If entry should be hidden in listing */
};



/**
 * issue fetch callbacks with locking
 */
ساكن bool
fetch_about_send_callback(ثابت fetch_پەیام *پەیام, بنية fetch_about_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(پەیام, ctx->fetchh);
	ctx->locked = false;

	أرجع ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_finished(بنية fetch_about_context *ctx)
{
	fetch_پەیام پەیام;
	پەیام.type = FETCH_FINISHED;
	أرجع fetch_about_send_callback(&پەیام, ctx);
}

/* exported interface documented in about/private.h */
bool fetch_about_set_http_code(بنية fetch_about_context *ctx, long code)
{
	fetch_set_http_code(ctx->fetchh, code);

	أرجع ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_header(بنية fetch_about_context *ctx, ثابت حرف *fmt, ...)
{
	حرف header[64];
	fetch_پەیام پەیام;
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(header, حجم header, fmt, ap);

	va_end(ap);

	پەیام.type = FETCH_HEADER;
	پەیام.data.header_or_data.buf = (ثابت uint8_t *) header;
	پەیام.data.header_or_data.len = strlen(header);

	أرجع fetch_about_send_callback(&پەیام, ctx);
}

/* exported interface documented in about/private.h */
nserror
fetch_about_senddata(بنية fetch_about_context *ctx, ثابت uint8_t *data, size_t data_len)
{
	fetch_پەیام پەیام;

	پەیام.type = FETCH_DATA;
	پەیام.data.header_or_data.buf = data;
	پەیام.data.header_or_data.len = data_len;

	إذا (fetch_about_send_callback(&پەیام, ctx)) {
		أرجع NSERROR_INVALID;
	}

	أرجع NSERROR_OK;
}

/* exported interface documented in about/private.h */
nserror
fetch_about_ssenddataf(بنية fetch_about_context *ctx, ثابت حرف *fmt, ...)
{
	حرف buffer[1024];
	حرف *dbuff;
	fetch_پەیام پەیام;
	va_list ap;
	صحيح slen;

	va_start(ap, fmt);

	slen = vsnprintf(buffer, حجم(buffer), fmt, ap);

	va_end(ap);

	إذا (slen < (صحيح)حجم(buffer)) {
		پەیام.type = FETCH_DATA;
		پەیام.data.header_or_data.buf = (ثابت uint8_t *) buffer;
		پەیام.data.header_or_data.len = slen;

		إذا (fetch_about_send_callback(&پەیام, ctx)) {
			أرجع NSERROR_INVALID;
		}

		أرجع NSERROR_OK;
	}

	dbuff = malloc(slen + 1);
	إذا (dbuff == عدم) {
		أرجع NSERROR_NOSPACE;
	}

	va_start(ap, fmt);

	slen = vsnprintf(dbuff, slen + 1, fmt, ap);

	va_end(ap);

	پەیام.type = FETCH_DATA;
	پەیام.data.header_or_data.buf = (ثابت uint8_t *)dbuff;
	پەیام.data.header_or_data.len = slen;

	إذا (fetch_about_send_callback(&پەیام, ctx)) {
		حرر(dbuff);
		أرجع NSERROR_INVALID;
	}

	حرر(dbuff);
	أرجع NSERROR_OK;
}


/* exported interface documented in about/private.h */
nsurl *fetch_about_get_url(بنية fetch_about_context *ctx)
{
	أرجع ctx->url;
}


/* exported interface documented in about/private.h */
ثابت بنية fetch_multipart_data *
fetch_about_get_multipart(بنية fetch_about_context *ctx)
{
	أرجع ctx->multipart;
}


/* exported interface documented in about/private.h */
bool fetch_about_srverror(بنية fetch_about_context *ctx)
{
	nserror res;

	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_INTERNAL_SERVER_ERROR);

	/* content type */
	إذا (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		أرجع false;

	res = fetch_about_ssenddataf(ctx, "%s 500 %s",
				     messages_get("FetchErrorCode"),
				     messages_get("HTTP500"));
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	fetch_about_send_finished(ctx);

	أرجع true;
}


/* exported interface documented in about/private.h */
bool fetch_about_redirect(بنية fetch_about_context *ctx, ثابت حرف *target)
{
	fetch_پەیام پەیام;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_FOUND);

	پەیام.type = FETCH_REDIRECT;
	پەیام.data.redirect = target;

	أرجع fetch_about_send_callback(&پەیام, ctx);
}

/**
 * Handler to generate about scheme credits page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ساكن bool fetch_about_credits_handler(بنية fetch_about_context *ctx)
{
	أرجع fetch_about_redirect(ctx, "resource:credits.html");
}


/**
 * Handler to generate about scheme licence page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ساكن bool fetch_about_licence_handler(بنية fetch_about_context *ctx)
{
	أرجع fetch_about_redirect(ctx, "resource:licence.html");
}


/**
 * Handler to generate about scheme logo page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ساكن bool fetch_about_logo_handler(بنية fetch_about_context *ctx)
{
	أرجع fetch_about_redirect(ctx, "resource:netsurf.png");
}


/**
 * Handler to generate about scheme welcome page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
ساكن bool fetch_about_welcome_handler(بنية fetch_about_context *ctx)
{
	أرجع fetch_about_redirect(ctx, "resource:welcome.html");
}


/* Forward declaration because this handler requires the handler table. */
ساكن bool fetch_about_about_handler(بنية fetch_about_context *ctx);

/**
 * List of about paths and their handlers
 */
بنية about_handlers about_handler_list[] = {
	{
		"credits",
		SLEN("credits"),
		عدم,
		fetch_about_credits_handler,
		false
	},
	{
		"licence",
		SLEN("licence"),
		عدم,
		fetch_about_licence_handler,
		false
	},
	{
		"license",
		SLEN("license"),
		عدم,
		fetch_about_licence_handler,
		true
	},
	{
		"welcome",
		SLEN("welcome"),
		عدم,
		fetch_about_welcome_handler,
		false
	},
	{
		"config",
		SLEN("config"),
		عدم,
		fetch_about_config_handler,
		false
	},
	{
		"Choices",
		SLEN("Choices"),
		عدم,
		fetch_about_choices_handler,
		false
	},
	{
		"testament",
		SLEN("testament"),
		عدم,
		fetch_about_testament_handler,
		false
	},
	{
		"about",
		SLEN("about"),
		عدم,
		fetch_about_about_handler,
		true
	},
	{
		"nscolours.css",
		SLEN("nscolours.css"),
		عدم,
		fetch_about_nscolours_handler,
		true
	},
	{
		"logo",
		SLEN("logo"),
		عدم,
		fetch_about_logo_handler,
		true
	},
	{
		/* details about the image cache */
		"imagecache",
		SLEN("imagecache"),
		عدم,
		fetch_about_imagecache_handler,
		true
	},
	{
		/* The default blank page */
		"blank",
		SLEN("blank"),
		عدم,
		fetch_about_blank_handler,
		true
	},
	{
		/* details about a certificate */
		"certificate",
		SLEN("certificate"),
		عدم,
		fetch_about_certificate_handler,
		true
	},
	{
		/* chart generator */
		"chart",
		SLEN("chart"),
		عدم,
		fetch_about_chart_handler,
		true
	},
	{
		"query/auth",
		SLEN("query/auth"),
		عدم,
		fetch_about_query_auth_handler,
		true
	},
	{
		"query/ssl",
		SLEN("query/ssl"),
		عدم,
		fetch_about_query_privacy_handler,
		true
	},
	{
		"query/timeout",
		SLEN("query/timeout"),
		عدم,
		fetch_about_query_timeout_handler,
		true
	},
	{
		"query/fetcherror",
		SLEN("query/fetcherror"),
		عدم,
		fetch_about_query_fetcherror_handler,
		true
	},
	{
		/* web search using the configured provider */
		"websearch",
		SLEN("websearch"),
		عدم,
		fetch_about_websearch_handler,
		true
	}
};

#define about_handler_list_len \
	(حجم(about_handler_list) / حجم(بنية about_handlers))

/**
 * List all the valid about: paths available
 *
 * \param ctx The fetch context.
 * \return true for sucess or false to generate an error.
 */
ساكن bool fetch_about_about_handler(بنية fetch_about_context *ctx)
{
	nserror res;
	unsigned صحيح abt_loop = 0;

	/* content is going to return ok */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_OK);

	/* content type */
	إذا (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		اذهب fetch_about_config_handler_aborted;

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>List of NetSurf pages</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class =\"ns-border\">List of NetSurf pages</h1>\n"
			"<ul>\n");
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_config_handler_aborted;
	}

	لكل (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {

		/* Skip over hidden entries */
		إذا (about_handler_list[abt_loop].hidden)
			استمر;

		res = fetch_about_ssenddataf(ctx,
			       "<li><a href=\"about:%s\">about:%s</a></li>\n",
			       about_handler_list[abt_loop].name,
			       about_handler_list[abt_loop].name);
		إذا (res != NSERROR_OK) {
			اذهب fetch_about_config_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</ul>\n</body>\n</html>\n");
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	أرجع true;

fetch_about_config_handler_aborted:
	أرجع false;
}

ساكن bool
fetch_about_404_handler(بنية fetch_about_context *ctx)
{
	nserror res;
	ثابت حرف *title;

	/* content is going to return 404 */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_NOT_FOUND);

	/* content type */
	إذا (fetch_about_send_header(ctx,
			"Content-Type: text/html; charset=utf-8")) {
		أرجع false;
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
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	fetch_about_send_finished(ctx);

	أرجع true;
}

/**
 * callback to initialise the about scheme fetcher.
 */
ساكن bool fetch_about_initialise(lwc_string *scheme)
{
	unsigned صحيح abt_loop = 0;
	lwc_error error;

	لكل (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		error = lwc_intern_string(about_handler_list[abt_loop].name,
					about_handler_list[abt_loop].name_len,
					&about_handler_list[abt_loop].lname);
		إذا (error != lwc_error_ok) {
			طالما (abt_loop-- != 0) {
				lwc_string_unref(about_handler_list[abt_loop].lname);
			}
			أرجع false;
		}
	}

	أرجع true;
}


/**
 * callback to finalise the about scheme fetcher.
 */
ساكن فراغ fetch_about_finalise(lwc_string *scheme)
{
	unsigned صحيح abt_loop = 0;
	لكل (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		lwc_string_unref(about_handler_list[abt_loop].lname);
	}
}


ساكن bool fetch_about_can_fetch(ثابت nsurl *url)
{
	أرجع true;
}


/**
 * callback to set up a about scheme fetch.
 *
 * \param post_urlenc post data in urlenc format, owned by the llcache object
 *                        hence valid the entire lifetime of the fetch.
 * \param post_multipart post data in multipart format, owned by the llcache
 *                        object hence valid the entire lifetime of the fetch.
 */
ساكن فراغ *
fetch_about_setup(بنية fetch *fetchh,
		  nsurl *url,
		  bool only_2xx,
		  bool downgrade_tls,
		  ثابت حرف *post_urlenc,
		  ثابت بنية fetch_multipart_data *post_multipart,
		  ثابت حرف **headers)
{
	بنية fetch_about_context *ctx;
	unsigned صحيح handler_loop;
	lwc_string *path;
	bool match;

	ctx = calloc(1, حجم(*ctx));
	إذا (ctx == عدم)
		أرجع عدم;

	path = nsurl_get_component(url, NSURL_PATH);

	لكل (handler_loop = 0;
	     handler_loop < about_handler_list_len;
	     handler_loop++) {
		إذا (lwc_string_isequal(path,
				       about_handler_list[handler_loop].lname,
				       &match) == lwc_error_ok && match) {
			ctx->handler = about_handler_list[handler_loop].handler;
			اكسر;
		}
	}

	lwc_string_unref(path);

	ctx->fetchh = fetchh;
	ctx->url = nsurl_ref(url);
	ctx->multipart = post_multipart;

	RING_زیادکردن(ring, ctx);

	أرجع ctx;
}


/**
 * callback to free a about scheme fetch
 */
ساكن فراغ fetch_about_free(فراغ *ctx)
{
	بنية fetch_about_context *c = ctx;
	nsurl_unref(c->url);
	حرر(ctx);
}


/**
 * callback to start an about scheme fetch
 */
ساكن bool fetch_about_start(فراغ *ctx)
{
	أرجع true;
}


/**
 * callback to abort a about fetch
 */
ساكن فراغ fetch_about_abort(فراغ *ctx)
{
	بنية fetch_about_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/**
 * callback to poll for additional about fetch contents
 */
ساكن فراغ fetch_about_poll(lwc_string *scheme)
{
	بنية fetch_about_context *c, *save_ring = عدم;

	/* Iterate over ring, processing each pending fetch */
	طالما (ring != عدم) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		إذا (c->locked == true) {
			RING_زیادکردن(save_ring, c);
			استمر;
		}

		/* Only process non-aborted fetches */
		إذا (c->aborted == false) {
			/* about fetches can be processed in one go */
			إذا (c->handler == عدم) {
				fetch_about_404_handler(c);
			} وإلا {
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


nserror fetch_about_register(فراغ)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_about);
	ثابت بنية fetcher_operation_table fetcher_ops = {
		.initialise = fetch_about_initialise,
		.acceptable = fetch_about_can_fetch,
		.setup = fetch_about_setup,
		.start = fetch_about_start,
		.abort = fetch_about_abort,
		.حرر = fetch_about_free,
		.poll = fetch_about_poll,
		.finalise = fetch_about_finalise
	};

	أرجع fetcher_add(scheme, &fetcher_ops);
}
