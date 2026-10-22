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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "netsurf/inttypes.h"

#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/ring.h"
#અન્તર્ભાવ "utils/messages.h"

#અન્તર્ભાવ "content/fetch.h"
#અન્તર્ભાવ "content/fetchers.h"

#અન્તર્ભાવ "private.h"
#અન્તર્ભાવ "about.h"
#અન્તર્ભાવ "blank.h"
#અન્તર્ભાવ "certificate.h"
#અન્તર્ભાવ "config.h"
#અન્તર્ભાવ "chart.h"
#અન્તર્ભાવ "choices.h"
#અન્તર્ભાવ "imagecache.h"
#અન્તર્ભાવ "nscolours.h"
#અન્તર્ભાવ "query.h"
#અન્તર્ભાવ "query_auth.h"
#અન્તર્ભાવ "query_fetcherror.h"
#અન્તર્ભાવ "query_privacy.h"
#અન્તર્ભાવ "query_timeout.h"
#અન્તર્ભાવ "atestament.h"
#અન્તર્ભાવ "websearch.h"

પ્રકારવ્યાખ્યા bool (*fetch_about_handler)(સંરચના fetch_about_context *);

/**
 * Context for an about fetch
 */
સંરચના fetch_about_context {
	સંરચના fetch_about_context *r_next, *r_prev;

	સંરચના fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */

	અચલ સંરચના fetch_multipart_data *multipart; /**< post data */

	fetch_about_handler handler;
};

સ્થિર સંરચના fetch_about_context *ring = ખાલી;

/**
 * handler info for about scheme
 */
સંરચના about_handlers {
	અચલ અક્ષર *name; /**< name to match in url */
	પૂર્ણાંક name_len;
	lwc_string *lname; /**< Interned name */
	fetch_about_handler handler; /**< handler for the url */
	bool hidden; /**< If entry should be hidden in listing */
};



/**
 * issue fetch callbacks with locking
 */
સ્થિર bool
fetch_about_send_callback(અચલ fetch_સંદેશ *સંદેશ, સંરચના fetch_about_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(સંદેશ, ctx->fetchh);
	ctx->locked = false;

	પાછા ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_finished(સંરચના fetch_about_context *ctx)
{
	fetch_સંદેશ સંદેશ;
	સંદેશ.type = FETCH_FINISHED;
	પાછા fetch_about_send_callback(&સંદેશ, ctx);
}

/* exported interface documented in about/private.h */
bool fetch_about_set_http_code(સંરચના fetch_about_context *ctx, long code)
{
	fetch_set_http_code(ctx->fetchh, code);

	પાછા ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_header(સંરચના fetch_about_context *ctx, અચલ અક્ષર *fmt, ...)
{
	અક્ષર header[64];
	fetch_સંદેશ સંદેશ;
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(header, કદ header, fmt, ap);

	va_end(ap);

	સંદેશ.type = FETCH_HEADER;
	સંદેશ.data.header_or_data.buf = (અચલ uint8_t *) header;
	સંદેશ.data.header_or_data.len = strlen(header);

	પાછા fetch_about_send_callback(&સંદેશ, ctx);
}

/* exported interface documented in about/private.h */
nserror
fetch_about_senddata(સંરચના fetch_about_context *ctx, અચલ uint8_t *data, size_t data_len)
{
	fetch_સંદેશ સંદેશ;

	સંદેશ.type = FETCH_DATA;
	સંદેશ.data.header_or_data.buf = data;
	સંદેશ.data.header_or_data.len = data_len;

	જો (fetch_about_send_callback(&સંદેશ, ctx)) {
		પાછા NSERROR_INVALID;
	}

	પાછા NSERROR_OK;
}

/* exported interface documented in about/private.h */
nserror
fetch_about_ssenddataf(સંરચના fetch_about_context *ctx, અચલ અક્ષર *fmt, ...)
{
	અક્ષર buffer[1024];
	અક્ષર *dbuff;
	fetch_સંદેશ સંદેશ;
	va_list ap;
	પૂર્ણાંક slen;

	va_start(ap, fmt);

	slen = vsnprintf(buffer, કદ(buffer), fmt, ap);

	va_end(ap);

	જો (slen < (પૂર્ણાંક)કદ(buffer)) {
		સંદેશ.type = FETCH_DATA;
		સંદેશ.data.header_or_data.buf = (અચલ uint8_t *) buffer;
		સંદેશ.data.header_or_data.len = slen;

		જો (fetch_about_send_callback(&સંદેશ, ctx)) {
			પાછા NSERROR_INVALID;
		}

		પાછા NSERROR_OK;
	}

	dbuff = malloc(slen + 1);
	જો (dbuff == ખાલી) {
		પાછા NSERROR_NOSPACE;
	}

	va_start(ap, fmt);

	slen = vsnprintf(dbuff, slen + 1, fmt, ap);

	va_end(ap);

	સંદેશ.type = FETCH_DATA;
	સંદેશ.data.header_or_data.buf = (અચલ uint8_t *)dbuff;
	સંદેશ.data.header_or_data.len = slen;

	જો (fetch_about_send_callback(&સંદેશ, ctx)) {
		મુક્ત(dbuff);
		પાછા NSERROR_INVALID;
	}

	મુક્ત(dbuff);
	પાછા NSERROR_OK;
}


/* exported interface documented in about/private.h */
nsurl *fetch_about_get_url(સંરચના fetch_about_context *ctx)
{
	પાછા ctx->url;
}


/* exported interface documented in about/private.h */
અચલ સંરચના fetch_multipart_data *
fetch_about_get_multipart(સંરચના fetch_about_context *ctx)
{
	પાછા ctx->multipart;
}


/* exported interface documented in about/private.h */
bool fetch_about_srverror(સંરચના fetch_about_context *ctx)
{
	nserror res;

	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_INTERNAL_SERVER_ERROR);

	/* content type */
	જો (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		પાછા false;

	res = fetch_about_ssenddataf(ctx, "%s 500 %s",
				     messages_get("FetchErrorCode"),
				     messages_get("HTTP500"));
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	fetch_about_send_finished(ctx);

	પાછા true;
}


/* exported interface documented in about/private.h */
bool fetch_about_redirect(સંરચના fetch_about_context *ctx, અચલ અક્ષર *target)
{
	fetch_સંદેશ સંદેશ;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_FOUND);

	સંદેશ.type = FETCH_REDIRECT;
	સંદેશ.data.redirect = target;

	પાછા fetch_about_send_callback(&સંદેશ, ctx);
}

/**
 * Handler to generate about scheme credits page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
સ્થિર bool fetch_about_credits_handler(સંરચના fetch_about_context *ctx)
{
	પાછા fetch_about_redirect(ctx, "resource:credits.html");
}


/**
 * Handler to generate about scheme licence page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
સ્થિર bool fetch_about_licence_handler(સંરચના fetch_about_context *ctx)
{
	પાછા fetch_about_redirect(ctx, "resource:licence.html");
}


/**
 * Handler to generate about scheme logo page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
સ્થિર bool fetch_about_logo_handler(સંરચના fetch_about_context *ctx)
{
	પાછા fetch_about_redirect(ctx, "resource:netsurf.png");
}


/**
 * Handler to generate about scheme welcome page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
સ્થિર bool fetch_about_welcome_handler(સંરચના fetch_about_context *ctx)
{
	પાછા fetch_about_redirect(ctx, "resource:welcome.html");
}


/* Forward declaration because this handler requires the handler table. */
સ્થિર bool fetch_about_about_handler(સંરચના fetch_about_context *ctx);

/**
 * List of about paths and their handlers
 */
સંરચના about_handlers about_handler_list[] = {
	{
		"credits",
		SLEN("credits"),
		ખાલી,
		fetch_about_credits_handler,
		false
	},
	{
		"licence",
		SLEN("licence"),
		ખાલી,
		fetch_about_licence_handler,
		false
	},
	{
		"license",
		SLEN("license"),
		ખાલી,
		fetch_about_licence_handler,
		true
	},
	{
		"welcome",
		SLEN("welcome"),
		ખાલી,
		fetch_about_welcome_handler,
		false
	},
	{
		"config",
		SLEN("config"),
		ખાલી,
		fetch_about_config_handler,
		false
	},
	{
		"Choices",
		SLEN("Choices"),
		ખાલી,
		fetch_about_choices_handler,
		false
	},
	{
		"testament",
		SLEN("testament"),
		ખાલી,
		fetch_about_testament_handler,
		false
	},
	{
		"about",
		SLEN("about"),
		ખાલી,
		fetch_about_about_handler,
		true
	},
	{
		"nscolours.css",
		SLEN("nscolours.css"),
		ખાલી,
		fetch_about_nscolours_handler,
		true
	},
	{
		"logo",
		SLEN("logo"),
		ખાલી,
		fetch_about_logo_handler,
		true
	},
	{
		/* details about the image cache */
		"imagecache",
		SLEN("imagecache"),
		ખાલી,
		fetch_about_imagecache_handler,
		true
	},
	{
		/* The default blank page */
		"blank",
		SLEN("blank"),
		ખાલી,
		fetch_about_blank_handler,
		true
	},
	{
		/* details about a certificate */
		"certificate",
		SLEN("certificate"),
		ખાલી,
		fetch_about_certificate_handler,
		true
	},
	{
		/* chart generator */
		"chart",
		SLEN("chart"),
		ખાલી,
		fetch_about_chart_handler,
		true
	},
	{
		"query/auth",
		SLEN("query/auth"),
		ખાલી,
		fetch_about_query_auth_handler,
		true
	},
	{
		"query/ssl",
		SLEN("query/ssl"),
		ખાલી,
		fetch_about_query_privacy_handler,
		true
	},
	{
		"query/timeout",
		SLEN("query/timeout"),
		ખાલી,
		fetch_about_query_timeout_handler,
		true
	},
	{
		"query/fetcherror",
		SLEN("query/fetcherror"),
		ખાલી,
		fetch_about_query_fetcherror_handler,
		true
	},
	{
		/* web search using the configured provider */
		"websearch",
		SLEN("websearch"),
		ખાલી,
		fetch_about_websearch_handler,
		true
	}
};

#define about_handler_list_len \
	(કદ(about_handler_list) / કદ(સંરચના about_handlers))

/**
 * List all the valid about: paths available
 *
 * \param ctx The fetch context.
 * \return true for sucess or false to generate an error.
 */
સ્થિર bool fetch_about_about_handler(સંરચના fetch_about_context *ctx)
{
	nserror res;
	unsigned પૂર્ણાંક abt_loop = 0;

	/* content is going to return ok */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_OK);

	/* content type */
	જો (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		જાઓ fetch_about_config_handler_aborted;

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>List of NetSurf pages</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class =\"ns-border\">List of NetSurf pages</h1>\n"
			"<ul>\n");
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_config_handler_aborted;
	}

	માટે (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {

		/* Skip over hidden entries */
		જો (about_handler_list[abt_loop].hidden)
			ચાલુ;

		res = fetch_about_ssenddataf(ctx,
			       "<li><a href=\"about:%s\">about:%s</a></li>\n",
			       about_handler_list[abt_loop].name,
			       about_handler_list[abt_loop].name);
		જો (res != NSERROR_OK) {
			જાઓ fetch_about_config_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</ul>\n</body>\n</html>\n");
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	પાછા true;

fetch_about_config_handler_aborted:
	પાછા false;
}

સ્થિર bool
fetch_about_404_handler(સંરચના fetch_about_context *ctx)
{
	nserror res;
	અચલ અક્ષર *title;

	/* content is going to return 404 */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_NOT_FOUND);

	/* content type */
	જો (fetch_about_send_header(ctx,
			"Content-Type: text/html; charset=utf-8")) {
		પાછા false;
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
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	fetch_about_send_finished(ctx);

	પાછા true;
}

/**
 * callback to initialise the about scheme fetcher.
 */
સ્થિર bool fetch_about_initialise(lwc_string *scheme)
{
	unsigned પૂર્ણાંક abt_loop = 0;
	lwc_error error;

	માટે (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		error = lwc_intern_string(about_handler_list[abt_loop].name,
					about_handler_list[abt_loop].name_len,
					&about_handler_list[abt_loop].lname);
		જો (error != lwc_error_ok) {
			જ્યારે (abt_loop-- != 0) {
				lwc_string_unref(about_handler_list[abt_loop].lname);
			}
			પાછા false;
		}
	}

	પાછા true;
}


/**
 * callback to finalise the about scheme fetcher.
 */
સ્થિર શૂન્ય fetch_about_finalise(lwc_string *scheme)
{
	unsigned પૂર્ણાંક abt_loop = 0;
	માટે (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		lwc_string_unref(about_handler_list[abt_loop].lname);
	}
}


સ્થિર bool fetch_about_can_fetch(અચલ nsurl *url)
{
	પાછા true;
}


/**
 * callback to set up a about scheme fetch.
 *
 * \param post_urlenc post data in urlenc format, owned by the llcache object
 *                        hence valid the entire lifetime of the fetch.
 * \param post_multipart post data in multipart format, owned by the llcache
 *                        object hence valid the entire lifetime of the fetch.
 */
સ્થિર શૂન્ય *
fetch_about_setup(સંરચના fetch *fetchh,
		  nsurl *url,
		  bool only_2xx,
		  bool downgrade_tls,
		  અચલ અક્ષર *post_urlenc,
		  અચલ સંરચના fetch_multipart_data *post_multipart,
		  અચલ અક્ષર **headers)
{
	સંરચના fetch_about_context *ctx;
	unsigned પૂર્ણાંક handler_loop;
	lwc_string *path;
	bool match;

	ctx = calloc(1, કદ(*ctx));
	જો (ctx == ખાલી)
		પાછા ખાલી;

	path = nsurl_get_component(url, NSURL_PATH);

	માટે (handler_loop = 0;
	     handler_loop < about_handler_list_len;
	     handler_loop++) {
		જો (lwc_string_isequal(path,
				       about_handler_list[handler_loop].lname,
				       &match) == lwc_error_ok && match) {
			ctx->handler = about_handler_list[handler_loop].handler;
			તોડો;
		}
	}

	lwc_string_unref(path);

	ctx->fetchh = fetchh;
	ctx->url = nsurl_ref(url);
	ctx->multipart = post_multipart;

	RING_ઉમેરો(ring, ctx);

	પાછા ctx;
}


/**
 * callback to free a about scheme fetch
 */
સ્થિર શૂન્ય fetch_about_free(શૂન્ય *ctx)
{
	સંરચના fetch_about_context *c = ctx;
	nsurl_unref(c->url);
	મુક્ત(ctx);
}


/**
 * callback to start an about scheme fetch
 */
સ્થિર bool fetch_about_start(શૂન્ય *ctx)
{
	પાછા true;
}


/**
 * callback to abort a about fetch
 */
સ્થિર શૂન્ય fetch_about_abort(શૂન્ય *ctx)
{
	સંરચના fetch_about_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/**
 * callback to poll for additional about fetch contents
 */
સ્થિર શૂન્ય fetch_about_poll(lwc_string *scheme)
{
	સંરચના fetch_about_context *c, *save_ring = ખાલી;

	/* Iterate over ring, processing each pending fetch */
	જ્યારે (ring != ખાલી) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		જો (c->locked == true) {
			RING_ઉમેરો(save_ring, c);
			ચાલુ;
		}

		/* Only process non-aborted fetches */
		જો (c->aborted == false) {
			/* about fetches can be processed in one go */
			જો (c->handler == ખાલી) {
				fetch_about_404_handler(c);
			} નહીંતર {
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


nserror fetch_about_register(શૂન્ય)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_about);
	અચલ સંરચના fetcher_operation_table fetcher_ops = {
		.initialise = fetch_about_initialise,
		.acceptable = fetch_about_can_fetch,
		.setup = fetch_about_setup,
		.start = fetch_about_start,
		.abort = fetch_about_abort,
		.મુક્ત = fetch_about_free,
		.poll = fetch_about_poll,
		.finalise = fetch_about_finalise
	};

	પાછા fetcher_add(scheme, &fetcher_ops);
}
