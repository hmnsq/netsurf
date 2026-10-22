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

#समावेश <stdbool.h>
#समावेश <stddef.h>
#समावेश <stdarg.h>
#समावेश <stdio.h>
#समावेश <string.h>
#समावेश <stdlib.h>

#समावेश "netsurf/inttypes.h"

#समावेश "utils/errors.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/utils.h"
#समावेश "utils/ring.h"
#समावेश "utils/messages.h"

#समावेश "content/fetch.h"
#समावेश "content/fetchers.h"

#समावेश "private.h"
#समावेश "about.h"
#समावेश "blank.h"
#समावेश "certificate.h"
#समावेश "config.h"
#समावेश "chart.h"
#समावेश "choices.h"
#समावेश "imagecache.h"
#समावेश "nscolours.h"
#समावेश "query.h"
#समावेश "query_auth.h"
#समावेश "query_fetcherror.h"
#समावेश "query_privacy.h"
#समावेश "query_timeout.h"
#समावेश "atestament.h"
#समावेश "websearch.h"

प्रकार bool (*fetch_about_handler)(काष्ठा fetch_about_context *);

/**
 * Context for an about fetch
 */
काष्ठा fetch_about_context {
	काष्ठा fetch_about_context *r_अग्र, *r_पूर्व;

	काष्ठा fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< The full url the fetch refers to */

	स्थिर काष्ठा fetch_multipart_तथ्य *multipart; /**< post data */

	fetch_about_handler handler;
};

अटल काष्ठा fetch_about_context *ring = शून्य;

/**
 * handler info for about scheme
 */
काष्ठा about_handlers {
	स्थिर अक्षर *name; /**< name to match in url */
	पूर्णांक name_len;
	lwc_string *lname; /**< Interned name */
	fetch_about_handler handler; /**< handler for the url */
	bool hidden; /**< If entry should be hidden in listing */
};



/**
 * issue fetch callbacks with locking
 */
अटल bool
fetch_about_send_callback(स्थिर fetch_संदेश *संदेश, काष्ठा fetch_about_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(संदेश, ctx->fetchh);
	ctx->locked = false;

	वापस ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_finished(काष्ठा fetch_about_context *ctx)
{
	fetch_संदेश संदेश;
	संदेश.type = FETCH_FINISHED;
	वापस fetch_about_send_callback(&संदेश, ctx);
}

/* exported interface documented in about/private.h */
bool fetch_about_set_http_code(काष्ठा fetch_about_context *ctx, दीर्घ code)
{
	fetch_set_http_code(ctx->fetchh, code);

	वापस ctx->aborted;
}

/* exported interface documented in about/private.h */
bool
fetch_about_send_header(काष्ठा fetch_about_context *ctx, स्थिर अक्षर *fmt, ...)
{
	अक्षर header[64];
	fetch_संदेश संदेश;
	बहु_सूची ap;

	बहु_शुरू(ap, fmt);

	vsnprintf(header, माप header, fmt, ap);

	बहु_पूर्ण(ap);

	संदेश.type = FETCH_HEADER;
	संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *) header;
	संदेश.तथ्य.header_or_तथ्य.len = म_माप(header);

	वापस fetch_about_send_callback(&संदेश, ctx);
}

/* exported interface documented in about/private.h */
nserror
fetch_about_senddata(काष्ठा fetch_about_context *ctx, स्थिर uint8_t *तथ्य, माप_प्रकार तथ्य_len)
{
	fetch_संदेश संदेश;

	संदेश.type = FETCH_तथ्य;
	संदेश.तथ्य.header_or_तथ्य.buf = तथ्य;
	संदेश.तथ्य.header_or_तथ्य.len = तथ्य_len;

	अगर (fetch_about_send_callback(&संदेश, ctx)) {
		वापस NSERROR_INVALID;
	}

	वापस NSERROR_OK;
}

/* exported interface documented in about/private.h */
nserror
fetch_about_ssenddataf(काष्ठा fetch_about_context *ctx, स्थिर अक्षर *fmt, ...)
{
	अक्षर बफर[1024];
	अक्षर *dbuff;
	fetch_संदेश संदेश;
	बहु_सूची ap;
	पूर्णांक slen;

	बहु_शुरू(ap, fmt);

	slen = vsnprintf(बफर, माप(बफर), fmt, ap);

	बहु_पूर्ण(ap);

	अगर (slen < (पूर्णांक)माप(बफर)) {
		संदेश.type = FETCH_तथ्य;
		संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *) बफर;
		संदेश.तथ्य.header_or_तथ्य.len = slen;

		अगर (fetch_about_send_callback(&संदेश, ctx)) {
			वापस NSERROR_INVALID;
		}

		वापस NSERROR_OK;
	}

	dbuff = दो_स्मृति(slen + 1);
	अगर (dbuff == शून्य) {
		वापस NSERROR_NOSPACE;
	}

	बहु_शुरू(ap, fmt);

	slen = vsnprintf(dbuff, slen + 1, fmt, ap);

	बहु_पूर्ण(ap);

	संदेश.type = FETCH_तथ्य;
	संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *)dbuff;
	संदेश.तथ्य.header_or_तथ्य.len = slen;

	अगर (fetch_about_send_callback(&संदेश, ctx)) {
		मुक्त(dbuff);
		वापस NSERROR_INVALID;
	}

	मुक्त(dbuff);
	वापस NSERROR_OK;
}


/* exported interface documented in about/private.h */
nsurl *fetch_about_get_url(काष्ठा fetch_about_context *ctx)
{
	वापस ctx->url;
}


/* exported interface documented in about/private.h */
स्थिर काष्ठा fetch_multipart_तथ्य *
fetch_about_get_multipart(काष्ठा fetch_about_context *ctx)
{
	वापस ctx->multipart;
}


/* exported interface documented in about/private.h */
bool fetch_about_srverror(काष्ठा fetch_about_context *ctx)
{
	nserror res;

	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_INTERNAL_SERVER_ERROR);

	/* content type */
	अगर (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		वापस false;

	res = fetch_about_ssenddataf(ctx, "%s 500 %s",
				     messages_get("FetchErrorCode"),
				     messages_get("HTTP500"));
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	fetch_about_send_finished(ctx);

	वापस true;
}


/* exported interface documented in about/private.h */
bool fetch_about_redirect(काष्ठा fetch_about_context *ctx, स्थिर अक्षर *target)
{
	fetch_संदेश संदेश;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_FOUND);

	संदेश.type = FETCH_REDIRECT;
	संदेश.तथ्य.redirect = target;

	वापस fetch_about_send_callback(&संदेश, ctx);
}

/**
 * Handler to generate about scheme credits page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
अटल bool fetch_about_credits_handler(काष्ठा fetch_about_context *ctx)
{
	वापस fetch_about_redirect(ctx, "resource:credits.html");
}


/**
 * Handler to generate about scheme licence page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
अटल bool fetch_about_licence_handler(काष्ठा fetch_about_context *ctx)
{
	वापस fetch_about_redirect(ctx, "resource:licence.html");
}


/**
 * Handler to generate about scheme logo page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
अटल bool fetch_about_logo_handler(काष्ठा fetch_about_context *ctx)
{
	वापस fetch_about_redirect(ctx, "resource:netsurf.png");
}


/**
 * Handler to generate about scheme welcome page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
अटल bool fetch_about_welcome_handler(काष्ठा fetch_about_context *ctx)
{
	वापस fetch_about_redirect(ctx, "resource:welcome.html");
}


/* Forward declaration because this handler requires the handler table. */
अटल bool fetch_about_about_handler(काष्ठा fetch_about_context *ctx);

/**
 * List of about paths and their handlers
 */
काष्ठा about_handlers about_handler_list[] = {
	{
		"credits",
		SLEN("credits"),
		शून्य,
		fetch_about_credits_handler,
		false
	},
	{
		"licence",
		SLEN("licence"),
		शून्य,
		fetch_about_licence_handler,
		false
	},
	{
		"license",
		SLEN("license"),
		शून्य,
		fetch_about_licence_handler,
		true
	},
	{
		"welcome",
		SLEN("welcome"),
		शून्य,
		fetch_about_welcome_handler,
		false
	},
	{
		"config",
		SLEN("config"),
		शून्य,
		fetch_about_config_handler,
		false
	},
	{
		"Choices",
		SLEN("Choices"),
		शून्य,
		fetch_about_choices_handler,
		false
	},
	{
		"testament",
		SLEN("testament"),
		शून्य,
		fetch_about_testament_handler,
		false
	},
	{
		"about",
		SLEN("about"),
		शून्य,
		fetch_about_about_handler,
		true
	},
	{
		"nscolours.css",
		SLEN("nscolours.css"),
		शून्य,
		fetch_about_nscolours_handler,
		true
	},
	{
		"logo",
		SLEN("logo"),
		शून्य,
		fetch_about_logo_handler,
		true
	},
	{
		/* details about the image cache */
		"imagecache",
		SLEN("imagecache"),
		शून्य,
		fetch_about_imagecache_handler,
		true
	},
	{
		/* The default blank page */
		"blank",
		SLEN("blank"),
		शून्य,
		fetch_about_blank_handler,
		true
	},
	{
		/* details about a certificate */
		"certificate",
		SLEN("certificate"),
		शून्य,
		fetch_about_certificate_handler,
		true
	},
	{
		/* chart generator */
		"chart",
		SLEN("chart"),
		शून्य,
		fetch_about_chart_handler,
		true
	},
	{
		"query/auth",
		SLEN("query/auth"),
		शून्य,
		fetch_about_query_auth_handler,
		true
	},
	{
		"query/ssl",
		SLEN("query/ssl"),
		शून्य,
		fetch_about_query_privacy_handler,
		true
	},
	{
		"query/timeout",
		SLEN("query/timeout"),
		शून्य,
		fetch_about_query_timeout_handler,
		true
	},
	{
		"query/fetcherror",
		SLEN("query/fetcherror"),
		शून्य,
		fetch_about_query_fetcherror_handler,
		true
	},
	{
		/* web search using the configured provider */
		"websearch",
		SLEN("websearch"),
		शून्य,
		fetch_about_websearch_handler,
		true
	}
};

#घोषणा about_handler_list_len \
	(माप(about_handler_list) / माप(काष्ठा about_handlers))

/**
 * List all the valid about: paths available
 *
 * \param ctx The fetch context.
 * \return true for sucess or false to generate an error.
 */
अटल bool fetch_about_about_handler(काष्ठा fetch_about_context *ctx)
{
	nserror res;
	अचिन्हित पूर्णांक abt_loop = 0;

	/* content is going to return ok */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_OK);

	/* content type */
	अगर (fetch_about_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		जाओ fetch_about_config_handler_aborted;

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>List of NetSurf pages</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class =\"ns-border\">List of NetSurf pages</h1>\n"
			"<ul>\n");
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_config_handler_aborted;
	}

	क्रम (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {

		/* Skip over hidden entries */
		अगर (about_handler_list[abt_loop].hidden)
			जारी;

		res = fetch_about_ssenddataf(ctx,
			       "<li><a href=\"about:%s\">about:%s</a></li>\n",
			       about_handler_list[abt_loop].name,
			       about_handler_list[abt_loop].name);
		अगर (res != NSERROR_OK) {
			जाओ fetch_about_config_handler_aborted;
		}
	}

	res = fetch_about_ssenddataf(ctx, "</ul>\n</body>\n</html>\n");
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	वापस true;

fetch_about_config_handler_aborted:
	वापस false;
}

अटल bool
fetch_about_404_handler(काष्ठा fetch_about_context *ctx)
{
	nserror res;
	स्थिर अक्षर *title;

	/* content is going to return 404 */
	fetch_set_http_code(ctx->fetchh, HTTP_RESPONSE_NOT_FOUND);

	/* content type */
	अगर (fetch_about_send_header(ctx,
			"Content-Type: text/html; charset=utf-8")) {
		वापस false;
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
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	fetch_about_send_finished(ctx);

	वापस true;
}

/**
 * callback to initialise the about scheme fetcher.
 */
अटल bool fetch_about_initialise(lwc_string *scheme)
{
	अचिन्हित पूर्णांक abt_loop = 0;
	lwc_error error;

	क्रम (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		error = lwc_intern_string(about_handler_list[abt_loop].name,
					about_handler_list[abt_loop].name_len,
					&about_handler_list[abt_loop].lname);
		अगर (error != lwc_error_ok) {
			जबतक (abt_loop-- != 0) {
				lwc_string_unref(about_handler_list[abt_loop].lname);
			}
			वापस false;
		}
	}

	वापस true;
}


/**
 * callback to finalise the about scheme fetcher.
 */
अटल व्योम fetch_about_finalise(lwc_string *scheme)
{
	अचिन्हित पूर्णांक abt_loop = 0;
	क्रम (abt_loop = 0; abt_loop < about_handler_list_len; abt_loop++) {
		lwc_string_unref(about_handler_list[abt_loop].lname);
	}
}


अटल bool fetch_about_can_fetch(स्थिर nsurl *url)
{
	वापस true;
}


/**
 * callback to set up a about scheme fetch.
 *
 * \param post_urlenc post data in urlenc format, owned by the llcache object
 *                        hence valid the entire lifetime of the fetch.
 * \param post_multipart post data in multipart format, owned by the llcache
 *                        object hence valid the entire lifetime of the fetch.
 */
अटल व्योम *
fetch_about_setup(काष्ठा fetch *fetchh,
		  nsurl *url,
		  bool only_2xx,
		  bool downgrade_tls,
		  स्थिर अक्षर *post_urlenc,
		  स्थिर काष्ठा fetch_multipart_तथ्य *post_multipart,
		  स्थिर अक्षर **headers)
{
	काष्ठा fetch_about_context *ctx;
	अचिन्हित पूर्णांक handler_loop;
	lwc_string *path;
	bool match;

	ctx = सुस्मृति(1, माप(*ctx));
	अगर (ctx == शून्य)
		वापस शून्य;

	path = nsurl_get_component(url, NSURL_PATH);

	क्रम (handler_loop = 0;
	     handler_loop < about_handler_list_len;
	     handler_loop++) {
		अगर (lwc_string_isequal(path,
				       about_handler_list[handler_loop].lname,
				       &match) == lwc_error_ok && match) {
			ctx->handler = about_handler_list[handler_loop].handler;
			अवरोध;
		}
	}

	lwc_string_unref(path);

	ctx->fetchh = fetchh;
	ctx->url = nsurl_ref(url);
	ctx->multipart = post_multipart;

	RING_जोड़ो(ring, ctx);

	वापस ctx;
}


/**
 * callback to free a about scheme fetch
 */
अटल व्योम fetch_about_free(व्योम *ctx)
{
	काष्ठा fetch_about_context *c = ctx;
	nsurl_unref(c->url);
	मुक्त(ctx);
}


/**
 * callback to start an about scheme fetch
 */
अटल bool fetch_about_start(व्योम *ctx)
{
	वापस true;
}


/**
 * callback to abort a about fetch
 */
अटल व्योम fetch_about_abort(व्योम *ctx)
{
	काष्ठा fetch_about_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/**
 * callback to poll for additional about fetch contents
 */
अटल व्योम fetch_about_poll(lwc_string *scheme)
{
	काष्ठा fetch_about_context *c, *save_ring = शून्य;

	/* Iterate over ring, processing each pending fetch */
	जबतक (ring != शून्य) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		अगर (c->locked == true) {
			RING_जोड़ो(save_ring, c);
			जारी;
		}

		/* Only process non-aborted fetches */
		अगर (c->aborted == false) {
			/* about fetches can be processed in one go */
			अगर (c->handler == शून्य) {
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


nserror fetch_about_register(व्योम)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_about);
	स्थिर काष्ठा fetcher_operation_table fetcher_ops = {
		.initialise = fetch_about_initialise,
		.acceptable = fetch_about_can_fetch,
		.setup = fetch_about_setup,
		.start = fetch_about_start,
		.पात = fetch_about_abort,
		.मुक्त = fetch_about_free,
		.poll = fetch_about_poll,
		.finalise = fetch_about_finalise
	};

	वापस fetcher_add(scheme, &fetcher_ops);
}
