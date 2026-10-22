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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "netsurf/inttypes.h"

#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/ring.h"
#अन्तर्भाव "utils/messages.h"

#अन्तर्भाव "content/fetch.h"
#अन्तर्भाव "content/fetchers.h"

#अन्तर्भाव "private.h"
#अन्तर्भाव "about.h"
#अन्तर्भाव "blank.h"
#अन्तर्भाव "certificate.h"
#अन्तर्भाव "config.h"
#अन्तर्भाव "chart.h"
#अन्तर्भाव "choices.h"
#अन्तर्भाव "imagecache.h"
#अन्तर्भाव "nscolours.h"
#अन्तर्भाव "query.h"
#अन्तर्भाव "query_auth.h"
#अन्तर्भाव "query_fetcherror.h"
#अन्तर्भाव "query_privacy.h"
#अन्तर्भाव "query_timeout.h"
#अन्तर्भाव "atestament.h"
#अन्तर्भाव "websearch.h"

प्रकारनिर्देश bool (*fetch_about_handler)(संरचना fetch_about_context *);

/**
 * Context for an about fetch
 */
संरचना fetch_about_context {
	संरचना fetch_about_context *r_अग्र, *r_पूर्व;

	संरचना fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */

	ध्रुव संरचना fetch_multipart_दत्त *multipart; /**< post data */

	fetch_about_handler handler;
};

स्थिर संरचना fetch_about_context *ring = रिक्त;

/**
 * handler info for about scheme
 */
संरचना about_handlers {
	ध्रुव वर्ण *name; /**< name to match in url */
	पूर्णाङ्क name_len;
	lwc_string *lname; /**< Interned name */
	fetch_about_handler handler; /**< handler for the url */
	bool hidden; /**< If entry should be hidden in listing */
};



/**
 * issue fetch callbacks with locking
 */
स्थिर bool
fetch_about_send_callback(ध्रुव fetch_सन्देशः *सन्देशः, संरचना fetch_about_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(सन्देशः, ctx->fetchh);
	ctx->locked = false;

	प्रत्यागम ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_परिसमाप्तम्(संरचना fetch_about_context *ctx)
{
	fetch_सन्देशः सन्देशः;
	सन्देशः.type = FETCH_परिसमाप्तम्;
	प्रत्यागम fetch_about_send_callback(&सन्देशः, ctx);
}

/* exported interface documented in about/private.h */
bool fetch_about_समुच्चयः_http_संहिता(संरचना fetch_about_context *ctx, दीर्घ संहिता)
{
	fetch_समुच्चयः_http_संहिता(ctx->fetchh, संहिता);

	प्रत्यागम ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_शीर्षकम्(संरचना fetch_about_context *ctx, ध्रुव वर्ण *fmt, ...)
{
	वर्ण शीर्षकम्[64];
	fetch_सन्देशः सन्देशः;
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(शीर्षकम्, परिमाण शीर्षकम्, fmt, ap);

	va_end(ap);

	सन्देशः.type = FETCH_शीर्षकम्;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *) शीर्षकम्;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = वाक्यदैर्घ्यम्(शीर्षकम्);

	प्रत्यागम fetch_about_send_callback(&सन्देशः, ctx);
}

/* exported interface documented in about/private.h */
nserror
fetch_about_senddata(संरचना fetch_about_context *ctx, ध्रुव uint8_t *दत्त, size_t दत्त_len)
{
	fetch_सन्देशः सन्देशः;

	सन्देशः.type = FETCH_दत्त;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = दत्त;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = दत्त_len;

	यदि (fetch_about_send_callback(&सन्देशः, ctx)) {
		प्रत्यागम NSERROR_INVALID;
	}

	प्रत्यागम NSERROR_OK;
}

/* exported interface documented in about/private.h */
nserror
fetch_about_ssenddataf(संरचना fetch_about_context *ctx, ध्रुव वर्ण *fmt, ...)
{
	वर्ण बफर[1024];
	वर्ण *dbuff;
	fetch_सन्देशः सन्देशः;
	va_list ap;
	पूर्णाङ्क slen;

	va_start(ap, fmt);

	slen = vsnprintf(बफर, परिमाण(बफर), fmt, ap);

	va_end(ap);

	यदि (slen < (पूर्णाङ्क)परिमाण(बफर)) {
		सन्देशः.type = FETCH_दत्त;
		सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *) बफर;
		सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = slen;

		यदि (fetch_about_send_callback(&सन्देशः, ctx)) {
			प्रत्यागम NSERROR_INVALID;
		}

		प्रत्यागम NSERROR_OK;
	}

	dbuff = स्मृतिं_देहि(slen + 1);
	यदि (dbuff == रिक्त) {
		प्रत्यागम NSERROR_NOSPACE;
	}

	va_start(ap, fmt);

	slen = vsnprintf(dbuff, slen + 1, fmt, ap);

	va_end(ap);

	सन्देशः.type = FETCH_दत्त;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *)dbuff;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = slen;

	यदि (fetch_about_send_callback(&सन्देशः, ctx)) {
		मुक्त(dbuff);
		प्रत्यागम NSERROR_INVALID;
	}

	मुक्त(dbuff);
	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in about/private.h */
nsurl *fetch_about_get_url(संरचना fetch_about_context *ctx)
{
	प्रत्यागम ctx->url;
}


/* exported interface documented in about/private.h */
ध्रुव संरचना fetch_multipart_दत्त *
fetch_about_get_multipart(संरचना fetch_about_context *ctx)
{
	प्रत्यागम ctx->multipart;
}


/* exported interface documented in about/private.h */
bool fetch_about_srverror(संरचना fetch_about_context *ctx)
{
	nserror res;

	fetch_समुच्चयः_http_संहिता(ctx->fetchh, HTTP_RESPONSE_आन्तरिकम्_SERVER_ERROR);

	/* content type */
	यदि (fetch_about_send_शीर्षकम्(ctx, "Content-Type: text/plain"))
		प्रत्यागम false;

	res = fetch_about_ssenddataf(ctx, "%s 500 %s",
				     messages_get("FetchErrorCode"),
				     messages_get("HTTP500"));
	यदि (res != NSERROR_OK) {
		प्रत्यागम false;
	}

	fetch_about_send_परिसमाप्तम्(ctx);

	प्रत्यागम true;
}


/* exported interface documented in about/private.h */
bool fetch_about_redirect(संरचना fetch_about_context *ctx, ध्रुव वर्ण *target)
{
	fetch_सन्देशः सन्देशः;

	/* content is going to return redirect */
	fetch_समुच्चयः_http_संहिता(ctx->fetchh, HTTP_RESPONSE_प्राप्तम्);

	सन्देशः.type = FETCH_REDIRECT;
	सन्देशः.दत्त.redirect = target;

	प्रत्यागम fetch_about_send_callback(&सन्देशः, ctx);
}

/**
 * Handler to generate about scheme credits page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
स्थिर bool fetch_about_credits_handler(संरचना fetch_about_context *ctx)
{
	प्रत्यागम fetch_about_redirect(ctx, "resource:credits.html");
}


/**
 * Handler to generate about scheme licence page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
स्थिर bool fetch_about_licence_handler(संरचना fetch_about_context *ctx)
{
	प्रत्यागम fetch_about_redirect(ctx, "resource:licence.html");
}


/**
 * Handler to generate about scheme logo page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
स्थिर bool fetch_about_logo_handler(संरचना fetch_about_context *ctx)
{
	प्रत्यागम fetch_about_redirect(ctx, "resource:netsurf.png");
}


/**
 * Handler to generate about scheme welcome page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
स्थिर bool fetch_about_welcome_handler(संरचना fetch_about_context *ctx)
{
	प्रत्यागम fetch_about_redirect(ctx, "resource:welcome.html");
}


/* Forward declaration because this handler requires the handler table. */
स्थिर bool fetch_about_about_handler(संरचना fetch_about_context *ctx);

/**
 * List of about paths and their handlers
 */
संरचना about_handlers about_handler_list[] = {
	{
		"credits",
		SLEN("credits"),
		रिक्त,
		fetch_about_credits_handler,
		false
	},
	{
		"licence",
		SLEN("licence"),
		रिक्त,
		fetch_about_licence_handler,
		false
	},
	{
		"license",
		SLEN("license"),
		रिक्त,
		fetch_about_licence_handler,
		true
	},
	{
		"welcome",
		SLEN("welcome"),
		रिक्त,
		fetch_about_welcome_handler,
		false
	},
	{
		"config",
		SLEN("config"),
		रिक्त,
		fetch_about_config_handler,
		false
	},
	{
		"Choices",
		SLEN("Choices"),
		रिक्त,
		fetch_about_choices_handler,
		false
	},
	{
		"testament",
		SLEN("testament"),
		रिक्त,
		fetch_about_testament_handler,
		false
	},
	{
		"about",
		SLEN("about"),
		रिक्त,
		fetch_about_about_handler,
		true
	},
	{
		"nscolours.css",
		SLEN("nscolours.css"),
		रिक्त,
		fetch_about_nscolours_handler,
		true
	},
	{
		"logo",
		SLEN("logo"),
		रिक्त,
		fetch_about_logo_handler,
		true
	},
	{
		/* details about the image cache */
		"imagecache",
		SLEN("imagecache"),
		रिक्त,
		fetch_about_imagecache_handler,
		true
	},
	{
		/* The default blank page */
		"blank",
		SLEN("blank"),
		रिक्त,
		fetch_about_blank_handler,
		true
	},
	{
		/* details about a certificate */
		"certificate",
		SLEN("certificate"),
		रिक्त,
		fetch_about_certificate_handler,
		true
	},
	{
		/* chart generator */
		"chart",
		SLEN("chart"),
		रिक्त,
		fetch_about_chart_handler,
		true
	},
	{
		"query/auth",
		SLEN("query/auth"),
		रिक्त,
		fetch_about_query_auth_handler,
		true
	},
	{
		"query/ssl",
		SLEN("query/ssl"),
		रिक्त,
		fetch_about_query_privacy_handler,
		true
	},
	{
		"query/timeout",
		SLEN("query/timeout"),
		रिक्त,
		fetch_about_query_timeout_handler,
		true
	},
	{
		"query/fetcherror",
		SLEN("query/fetcherror"),
		रिक्त,
		fetch_about_query_fetcherror_handler,
		true
	},
	{
		/* web search using the configured provider */
		"websearch",
		SLEN("websearch"),
		रिक्त,
		fetch_about_websearch_handler,
		true
	}
};

#परिभाषय about_handler_list_len \
	(परिमाण(about_handler_list) / परिमाण(संरचना about_handlers))

/**
 * List all the valid about: paths available
 *
 * \param ctx The fetch context.
 * \return true for sucess or false to generate an error.
 */
स्थिर bool fetch_about_about_handler(संरचना fetch_about_context *ctx)
{
	nserror res;
	अचिह्नित पूर्णाङ्क abt_loop = 0;

	/* content is going to return ok */
	fetch_समुच्चयः_http_संहिता(ctx->fetchh, HTTP_RESPONSE_OK);

	/* content type */
	यदि (fetch_about_send_शीर्षकम्(ctx, "Content-Type: text/html; charset=utf-8"))
		गच्छ fetch_about_config_handler_aborted;

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>List of NetSurf pages</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class =\"ns-border\">List of NetSurf pages</h1>\n"
			"<ul>\n");
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_config_handler_aborted;
	}

	प्रति (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {

		/* Skip over hidden entries */
		यदि (about_handler_list[abt_loop].hidden)
			अनुवर्त;

		res = fetch_about_ssenddataf(ctx,
			       "<li><a href=\"about:%s\">about:%s</a></li>\n",
			       about_handler_list[abt_loop].name,
			       about_handler_list[abt_loop].name);
		यदि (res != NSERROR_OK) {
			गच्छ fetch_about_config_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</ul>\n</body>\n</html>\n");
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_config_handler_aborted;
	}

	fetch_about_send_परिसमाप्तम्(ctx);

	प्रत्यागम true;

fetch_about_config_handler_aborted:
	प्रत्यागम false;
}

स्थिर bool
fetch_about_404_handler(संरचना fetch_about_context *ctx)
{
	nserror res;
	ध्रुव वर्ण *title;

	/* content is going to return 404 */
	fetch_समुच्चयः_http_संहिता(ctx->fetchh, HTTP_RESPONSE_NOT_प्राप्तम्);

	/* content type */
	यदि (fetch_about_send_शीर्षकम्(ctx,
			"Content-Type: text/html; charset=utf-8")) {
		प्रत्यागम false;
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
	यदि (res != NSERROR_OK) {
		प्रत्यागम false;
	}

	fetch_about_send_परिसमाप्तम्(ctx);

	प्रत्यागम true;
}

/**
 * callback to initialise the about scheme fetcher.
 */
स्थिर bool fetch_about_initialise(lwc_string *scheme)
{
	अचिह्नित पूर्णाङ्क abt_loop = 0;
	lwc_error error;

	प्रति (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		error = lwc_intern_string(about_handler_list[abt_loop].name,
					about_handler_list[abt_loop].name_len,
					&about_handler_list[abt_loop].lname);
		यदि (error != lwc_error_ok) {
			यावत् (abt_loop-- != 0) {
				lwc_string_unref(about_handler_list[abt_loop].lname);
			}
			प्रत्यागम false;
		}
	}

	प्रत्यागम true;
}


/**
 * callback to finalise the about scheme fetcher.
 */
स्थिर शून्य fetch_about_finalise(lwc_string *scheme)
{
	अचिह्नित पूर्णाङ्क abt_loop = 0;
	प्रति (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		lwc_string_unref(about_handler_list[abt_loop].lname);
	}
}


स्थिर bool fetch_about_can_fetch(ध्रुव nsurl *url)
{
	प्रत्यागम true;
}


/**
 * callback to set up a about scheme fetch.
 *
 * \param post_urlenc post data in urlenc format, owned by the llcache object
 *                        hence valid the entire lifetime of the fetch.
 * \param post_multipart post data in multipart format, owned by the llcache
 *                        object hence valid the entire lifetime of the fetch.
 */
स्थिर शून्य *
fetch_about_सज्जता(संरचना fetch *fetchh,
		  nsurl *url,
		  bool only_2xx,
		  bool downgrade_tls,
		  ध्रुव वर्ण *post_urlenc,
		  ध्रुव संरचना fetch_multipart_दत्त *post_multipart,
		  ध्रुव वर्ण **शीर्षकाणि)
{
	संरचना fetch_about_context *ctx;
	अचिह्नित पूर्णाङ्क handler_loop;
	lwc_string *मार्गः;
	bool मेलनम्;

	ctx = शून्यस्मृतिं_देहि(1, परिमाण(*ctx));
	यदि (ctx == रिक्त)
		प्रत्यागम रिक्त;

	मार्गः = nsurl_get_component(url, NSURL_मार्गः);

	प्रति (handler_loop = 0;
	     handler_loop < about_handler_list_len;
	     handler_loop++) {
		यदि (lwc_string_isequal(मार्गः,
				       about_handler_list[handler_loop].lname,
				       &मेलनम्) == lwc_error_ok && मेलनम्) {
			ctx->handler = about_handler_list[handler_loop].handler;
			भङ्ग;
		}
	}

	lwc_string_unref(मार्गः);

	ctx->fetchh = fetchh;
	ctx->url = nsurl_ref(url);
	ctx->multipart = post_multipart;

	RING_निवेशय(ring, ctx);

	प्रत्यागम ctx;
}


/**
 * callback to free a about scheme fetch
 */
स्थिर शून्य fetch_about_free(शून्य *ctx)
{
	संरचना fetch_about_context *c = ctx;
	nsurl_unref(c->url);
	मुक्त(ctx);
}


/**
 * callback to start an about scheme fetch
 */
स्थिर bool fetch_about_start(शून्य *ctx)
{
	प्रत्यागम true;
}


/**
 * callback to abort a about fetch
 */
स्थिर शून्य fetch_about_abort(शून्य *ctx)
{
	संरचना fetch_about_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/**
 * callback to poll for additional about fetch contents
 */
स्थिर शून्य fetch_about_poll(lwc_string *scheme)
{
	संरचना fetch_about_context *c, *save_ring = रिक्त;

	/* Iterate over ring, processing each pending fetch */
	यावत् (ring != रिक्त) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		यदि (c->locked == true) {
			RING_निवेशय(save_ring, c);
			अनुवर्त;
		}

		/* Only process non-aborted fetches */
		यदि (c->aborted == false) {
			/* about fetches can be processed in one go */
			यदि (c->handler == रिक्त) {
				fetch_about_404_handler(c);
			} अन्यथा {
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


nserror fetch_about_पञ्जिकाकोशः(शून्य)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_about);
	ध्रुव संरचना fetcher_operation_सारणी fetcher_ops = {
		.initialise = fetch_about_initialise,
		.acceptable = fetch_about_can_fetch,
		.सज्जता = fetch_about_सज्जता,
		.start = fetch_about_start,
		.abort = fetch_about_abort,
		.मुक्त = fetch_about_free,
		.poll = fetch_about_poll,
		.finalise = fetch_about_finalise
	};

	प्रत्यागम fetcher_सङ्कलय(scheme, &fetcher_ops);
}
