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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "netsurf/inttypes.h"

#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/ring.h"
#அந்தர்பாவ "utils/messages.h"

#அந்தர்பாவ "content/fetch.h"
#அந்தர்பாவ "content/fetchers.h"

#அந்தர்பாவ "private.h"
#அந்தர்பாவ "about.h"
#அந்தர்பாவ "blank.h"
#அந்தர்பாவ "certificate.h"
#அந்தர்பாவ "config.h"
#அந்தர்பாவ "chart.h"
#அந்தர்பாவ "choices.h"
#அந்தர்பாவ "imagecache.h"
#அந்தர்பாவ "nscolours.h"
#அந்தர்பாவ "query.h"
#அந்தர்பாவ "query_auth.h"
#அந்தர்பாவ "query_fetcherror.h"
#அந்தர்பாவ "query_privacy.h"
#அந்தர்பாவ "query_timeout.h"
#அந்தர்பாவ "atestament.h"
#அந்தர்பாவ "websearch.h"

வகைவரை bool (*fetch_about_handler)(கட்டமைப்பு fetch_about_context *);

/**
 * Context for an about fetch
 */
கட்டமைப்பு fetch_about_context {
	கட்டமைப்பு fetch_about_context *r_next, *r_prev;

	கட்டமைப்பு fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */

	மாறிலி கட்டமைப்பு fetch_multipart_data *multipart; /**< post data */

	fetch_about_handler handler;
};

நிலையான கட்டமைப்பு fetch_about_context *ring = வெற்று;

/**
 * handler info for about scheme
 */
கட்டமைப்பு about_handlers {
	மாறிலி எழுத்து *name; /**< name to match in url */
	முழு name_len;
	lwc_string *lname; /**< Interned name */
	fetch_about_handler handler; /**< handler for the url */
	bool hidden; /**< If entry should be hidden in listing */
};



/**
 * issue fetch callbacks with locking
 */
நிலையான bool
fetch_about_send_callback(மாறிலி fetch_செய்தி *செய்தி, கட்டமைப்பு fetch_about_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(செய்தி, ctx->fetchh);
	ctx->locked = false;

	திரும்பு ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_finished(கட்டமைப்பு fetch_about_context *ctx)
{
	fetch_செய்தி செய்தி;
	செய்தி.type = FETCH_FINISHED;
	திரும்பு fetch_about_send_callback(&செய்தி, ctx);
}

/* exported interface documented in about/private.h */
bool fetch_about_set_http_code(கட்டமைப்பு fetch_about_context *ctx, long code)
{
	fetch_set_http_code(ctx->fetchh, code);

	திரும்பு ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_header(கட்டமைப்பு fetch_about_context *ctx, மாறிலி எழுத்து *fmt, ...)
{
	எழுத்து header[64];
	fetch_செய்தி செய்தி;
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(header, அளவு header, fmt, ap);

	va_end(ap);

	செய்தி.type = FETCH_HEADER;
	செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *) header;
	செய்தி.data.header_or_data.len = strlen(header);

	திரும்பு fetch_about_send_callback(&செய்தி, ctx);
}

/* exported interface documented in about/private.h */
nserror
fetch_about_senddata(கட்டமைப்பு fetch_about_context *ctx, மாறிலி uint8_t *data, size_t data_len)
{
	fetch_செய்தி செய்தி;

	செய்தி.type = FETCH_DATA;
	செய்தி.data.header_or_data.buf = data;
	செய்தி.data.header_or_data.len = data_len;

	எனில் (fetch_about_send_callback(&செய்தி, ctx)) {
		திரும்பு NSERROR_INVALID;
	}

	திரும்பு NSERROR_OK;
}

/* exported interface documented in about/private.h */
nserror
fetch_about_ssenddataf(கட்டமைப்பு fetch_about_context *ctx, மாறிலி எழுத்து *fmt, ...)
{
	எழுத்து buffer[1024];
	எழுத்து *dbuff;
	fetch_செய்தி செய்தி;
	va_list ap;
	முழு slen;

	va_start(ap, fmt);

	slen = vsnprintf(buffer, அளவு(buffer), fmt, ap);

	va_end(ap);

	எனில் (slen < (முழு)அளவு(buffer)) {
		செய்தி.type = FETCH_DATA;
		செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *) buffer;
		செய்தி.data.header_or_data.len = slen;

		எனில் (fetch_about_send_callback(&செய்தி, ctx)) {
			திரும்பு NSERROR_INVALID;
		}

		திரும்பு NSERROR_OK;
	}

	dbuff = malloc(slen + 1);
	எனில் (dbuff == வெற்று) {
		திரும்பு NSERROR_NOSPACE;
	}

	va_start(ap, fmt);

	slen = vsnprintf(dbuff, slen + 1, fmt, ap);

	va_end(ap);

	செய்தி.type = FETCH_DATA;
	செய்தி.data.header_or_data.buf = (மாறிலி uint8_t *)dbuff;
	செய்தி.data.header_or_data.len = slen;

	எனில் (fetch_about_send_callback(&செய்தி, ctx)) {
		விடுவி(dbuff);
		திரும்பு NSERROR_INVALID;
	}

	விடுவி(dbuff);
	திரும்பு NSERROR_OK;
}


/* exported interface documented in about/private.h */
nsurl *fetch_about_get_url(கட்டமைப்பு fetch_about_context *ctx)
{
	திரும்பு ctx->url;
}


/* exported interface documented in about/private.h */
மாறிலி கட்டமைப்பு fetch_multipart_data *
fetch_about_get_multipart(கட்டமைப்பு fetch_about_context *ctx)
{
	திரும்பு ctx->multipart;
}


/* exported interface documented in about/private.h */
bool fetch_about_srverror(கட்டமைப்பு fetch_about_context *ctx)
{
	nserror res;

	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_INTERNAL_SERVER_ERROR);

	/* content type */
	எனில் (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		திரும்பு false;

	res = fetch_about_ssenddataf(ctx, "%s 500 %s",
				     messages_get("FetchErrorCode"),
				     messages_get("HTTP500"));
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	fetch_about_send_finished(ctx);

	திரும்பு true;
}


/* exported interface documented in about/private.h */
bool fetch_about_redirect(கட்டமைப்பு fetch_about_context *ctx, மாறிலி எழுத்து *target)
{
	fetch_செய்தி செய்தி;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_FOUND);

	செய்தி.type = FETCH_REDIRECT;
	செய்தி.data.redirect = target;

	திரும்பு fetch_about_send_callback(&செய்தி, ctx);
}

/**
 * Handler to generate about scheme credits page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
நிலையான bool fetch_about_credits_handler(கட்டமைப்பு fetch_about_context *ctx)
{
	திரும்பு fetch_about_redirect(ctx, "resource:credits.html");
}


/**
 * Handler to generate about scheme licence page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
நிலையான bool fetch_about_licence_handler(கட்டமைப்பு fetch_about_context *ctx)
{
	திரும்பு fetch_about_redirect(ctx, "resource:licence.html");
}


/**
 * Handler to generate about scheme logo page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
நிலையான bool fetch_about_logo_handler(கட்டமைப்பு fetch_about_context *ctx)
{
	திரும்பு fetch_about_redirect(ctx, "resource:netsurf.png");
}


/**
 * Handler to generate about scheme welcome page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
நிலையான bool fetch_about_welcome_handler(கட்டமைப்பு fetch_about_context *ctx)
{
	திரும்பு fetch_about_redirect(ctx, "resource:welcome.html");
}


/* Forward declaration because this handler requires the handler table. */
நிலையான bool fetch_about_about_handler(கட்டமைப்பு fetch_about_context *ctx);

/**
 * List of about paths and their handlers
 */
கட்டமைப்பு about_handlers about_handler_list[] = {
	{
		"credits",
		SLEN("credits"),
		வெற்று,
		fetch_about_credits_handler,
		false
	},
	{
		"licence",
		SLEN("licence"),
		வெற்று,
		fetch_about_licence_handler,
		false
	},
	{
		"license",
		SLEN("license"),
		வெற்று,
		fetch_about_licence_handler,
		true
	},
	{
		"welcome",
		SLEN("welcome"),
		வெற்று,
		fetch_about_welcome_handler,
		false
	},
	{
		"config",
		SLEN("config"),
		வெற்று,
		fetch_about_config_handler,
		false
	},
	{
		"Choices",
		SLEN("Choices"),
		வெற்று,
		fetch_about_choices_handler,
		false
	},
	{
		"testament",
		SLEN("testament"),
		வெற்று,
		fetch_about_testament_handler,
		false
	},
	{
		"about",
		SLEN("about"),
		வெற்று,
		fetch_about_about_handler,
		true
	},
	{
		"nscolours.css",
		SLEN("nscolours.css"),
		வெற்று,
		fetch_about_nscolours_handler,
		true
	},
	{
		"logo",
		SLEN("logo"),
		வெற்று,
		fetch_about_logo_handler,
		true
	},
	{
		/* details about the image cache */
		"imagecache",
		SLEN("imagecache"),
		வெற்று,
		fetch_about_imagecache_handler,
		true
	},
	{
		/* The default blank page */
		"blank",
		SLEN("blank"),
		வெற்று,
		fetch_about_blank_handler,
		true
	},
	{
		/* details about a certificate */
		"certificate",
		SLEN("certificate"),
		வெற்று,
		fetch_about_certificate_handler,
		true
	},
	{
		/* chart generator */
		"chart",
		SLEN("chart"),
		வெற்று,
		fetch_about_chart_handler,
		true
	},
	{
		"query/auth",
		SLEN("query/auth"),
		வெற்று,
		fetch_about_query_auth_handler,
		true
	},
	{
		"query/ssl",
		SLEN("query/ssl"),
		வெற்று,
		fetch_about_query_privacy_handler,
		true
	},
	{
		"query/timeout",
		SLEN("query/timeout"),
		வெற்று,
		fetch_about_query_timeout_handler,
		true
	},
	{
		"query/fetcherror",
		SLEN("query/fetcherror"),
		வெற்று,
		fetch_about_query_fetcherror_handler,
		true
	},
	{
		/* web search using the configured provider */
		"websearch",
		SLEN("websearch"),
		வெற்று,
		fetch_about_websearch_handler,
		true
	}
};

#define about_handler_list_len \
	(அளவு(about_handler_list) / அளவு(கட்டமைப்பு about_handlers))

/**
 * List all the valid about: paths available
 *
 * \param ctx The fetch context.
 * \return true for sucess or false to generate an error.
 */
நிலையான bool fetch_about_about_handler(கட்டமைப்பு fetch_about_context *ctx)
{
	nserror res;
	unsigned முழு abt_loop = 0;

	/* content is going to return ok */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_OK);

	/* content type */
	எனில் (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		செல் fetch_about_config_handler_aborted;

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>List of NetSurf pages</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class =\"ns-border\">List of NetSurf pages</h1>\n"
			"<ul>\n");
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_config_handler_aborted;
	}

	க்கு (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {

		/* Skip over hidden entries */
		எனில் (about_handler_list[abt_loop].hidden)
			தொடர்;

		res = fetch_about_ssenddataf(ctx,
			       "<li><a href=\"about:%s\">about:%s</a></li>\n",
			       about_handler_list[abt_loop].name,
			       about_handler_list[abt_loop].name);
		எனில் (res != NSERROR_OK) {
			செல் fetch_about_config_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</ul>\n</body>\n</html>\n");
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	திரும்பு true;

fetch_about_config_handler_aborted:
	திரும்பு false;
}

நிலையான bool
fetch_about_404_handler(கட்டமைப்பு fetch_about_context *ctx)
{
	nserror res;
	மாறிலி எழுத்து *title;

	/* content is going to return 404 */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_NOT_FOUND);

	/* content type */
	எனில் (fetch_about_send_header(ctx,
			"Content-Type: text/html; charset=utf-8")) {
		திரும்பு false;
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
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	fetch_about_send_finished(ctx);

	திரும்பு true;
}

/**
 * callback to initialise the about scheme fetcher.
 */
நிலையான bool fetch_about_initialise(lwc_string *scheme)
{
	unsigned முழு abt_loop = 0;
	lwc_error error;

	க்கு (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		error = lwc_intern_string(about_handler_list[abt_loop].name,
					about_handler_list[abt_loop].name_len,
					&about_handler_list[abt_loop].lname);
		எனில் (error != lwc_error_ok) {
			போது (abt_loop-- != 0) {
				lwc_string_unref(about_handler_list[abt_loop].lname);
			}
			திரும்பு false;
		}
	}

	திரும்பு true;
}


/**
 * callback to finalise the about scheme fetcher.
 */
நிலையான வெறுமை fetch_about_finalise(lwc_string *scheme)
{
	unsigned முழு abt_loop = 0;
	க்கு (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		lwc_string_unref(about_handler_list[abt_loop].lname);
	}
}


நிலையான bool fetch_about_can_fetch(மாறிலி nsurl *url)
{
	திரும்பு true;
}


/**
 * callback to set up a about scheme fetch.
 *
 * \param post_urlenc post data in urlenc format, owned by the llcache object
 *                        hence valid the entire lifetime of the fetch.
 * \param post_multipart post data in multipart format, owned by the llcache
 *                        object hence valid the entire lifetime of the fetch.
 */
நிலையான வெறுமை *
fetch_about_setup(கட்டமைப்பு fetch *fetchh,
		  nsurl *url,
		  bool only_2xx,
		  bool downgrade_tls,
		  மாறிலி எழுத்து *post_urlenc,
		  மாறிலி கட்டமைப்பு fetch_multipart_data *post_multipart,
		  மாறிலி எழுத்து **headers)
{
	கட்டமைப்பு fetch_about_context *ctx;
	unsigned முழு handler_loop;
	lwc_string *path;
	bool match;

	ctx = calloc(1, அளவு(*ctx));
	எனில் (ctx == வெற்று)
		திரும்பு வெற்று;

	path = nsurl_get_component(url, NSURL_PATH);

	க்கு (handler_loop = 0;
	     handler_loop < about_handler_list_len;
	     handler_loop++) {
		எனில் (lwc_string_isequal(path,
				       about_handler_list[handler_loop].lname,
				       &match) == lwc_error_ok && match) {
			ctx->handler = about_handler_list[handler_loop].handler;
			நிறுத்து;
		}
	}

	lwc_string_unref(path);

	ctx->fetchh = fetchh;
	ctx->url = nsurl_ref(url);
	ctx->multipart = post_multipart;

	RING_செருகு(ring, ctx);

	திரும்பு ctx;
}


/**
 * callback to free a about scheme fetch
 */
நிலையான வெறுமை fetch_about_free(வெறுமை *ctx)
{
	கட்டமைப்பு fetch_about_context *c = ctx;
	nsurl_unref(c->url);
	விடுவி(ctx);
}


/**
 * callback to start an about scheme fetch
 */
நிலையான bool fetch_about_start(வெறுமை *ctx)
{
	திரும்பு true;
}


/**
 * callback to abort a about fetch
 */
நிலையான வெறுமை fetch_about_abort(வெறுமை *ctx)
{
	கட்டமைப்பு fetch_about_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/**
 * callback to poll for additional about fetch contents
 */
நிலையான வெறுமை fetch_about_poll(lwc_string *scheme)
{
	கட்டமைப்பு fetch_about_context *c, *save_ring = வெற்று;

	/* Iterate over ring, processing each pending fetch */
	போது (ring != வெற்று) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		எனில் (c->locked == true) {
			RING_செருகு(save_ring, c);
			தொடர்;
		}

		/* Only process non-aborted fetches */
		எனில் (c->aborted == false) {
			/* about fetches can be processed in one go */
			எனில் (c->handler == வெற்று) {
				fetch_about_404_handler(c);
			} இல்லையெனில் {
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


nserror fetch_about_register(வெறுமை)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_about);
	மாறிலி கட்டமைப்பு fetcher_operation_table fetcher_ops = {
		.initialise = fetch_about_initialise,
		.acceptable = fetch_about_can_fetch,
		.setup = fetch_about_setup,
		.start = fetch_about_start,
		.abort = fetch_about_abort,
		.விடுவி = fetch_about_free,
		.poll = fetch_about_poll,
		.finalise = fetch_about_finalise
	};

	திரும்பு fetcher_add(scheme, &fetcher_ops);
}
