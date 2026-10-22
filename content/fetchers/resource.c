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
 * resource scheme URL handling. Based on the data fetcher by Rob Kendrick
 */

#समावेश <stdlib.h>
#समावेश <stdbool.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <stdio.h>
#समावेश <stdarg.h>
#समावेश <libwapcaplet/libwapcaplet.h>

#समावेश "netsurf/inttypes.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/log.h"
#समावेश "utils/time.h"
#समावेश "utils/messages.h"
#समावेश "utils/utils.h"
#समावेश "utils/ring.h"
#समावेश "netsurf/fetch.h"
#समावेश "desktop/gui_internal.h"

#समावेश "content/fetch.h"
#समावेश "content/fetchers.h"
#समावेश "content/fetchers/resource.h"

#घोषणा DIRECT_ETAG_मूल्य 123456

/** Valid resource paths */
अटल स्थिर अक्षर *fetch_resource_paths[] = {
	"adblock.css",
	"default.css",
	"internal.css",
	"quirks.css",
	"user.css",
	"credits.html",
	"licence.html",
	"welcome.html",
	"favicon.ico",
	"default.ico",
	"netsurf.png",
	"icons/arrow-l.png",
	"icons/content.png",
	"icons/directory.png",
	"icons/directory2.png",
	"icons/hotlist-add.png",
	"icons/hotlist-rmv.png",
	"icons/search.png"
};

/**
 * map of resource scheme paths to redirect urls
 */
अटल काष्ठा fetch_resource_map_entry {
	lwc_string *path; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	स्थिर uint8_t *तथ्य; /**< direct pointer to data */
	माप_प्रकार तथ्य_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

काष्ठा fetch_resource_context;

प्रकार bool (*fetch_resource_handler)(काष्ठा fetch_resource_context *);

/** Context for an resource fetch */
काष्ठा fetch_resource_context {
	काष्ठा fetch_resource_context *r_अग्र, *r_पूर्व;

	काष्ठा fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	काष्ठा fetch_resource_map_entry *entry; /**< resource map entry */

	fetch_resource_handler handler;

	समय_प्रकार etag;
};

अटल काष्ठा fetch_resource_context *ring = शून्य;

अटल uint32_t fetch_resource_path_गिनती;

/** issue fetch callbacks with locking */
अटल अंतरभूत bool fetch_resource_send_callback(स्थिर fetch_संदेश *संदेश,
		काष्ठा fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(संदेश, ctx->fetchh);
	ctx->locked = false;

	वापस ctx->aborted;
}

अटल bool fetch_resource_send_header(काष्ठा fetch_resource_context *ctx,
		स्थिर अक्षर *fmt, ...)
{
	fetch_संदेश संदेश;
	अक्षर header[64];
	बहु_सूची ap;
	पूर्णांक len;

	बहु_शुरू(ap, fmt);
	len = vsnprintf(header, माप header, fmt, ap);
	बहु_पूर्ण(ap);

	अगर (len >= (पूर्णांक)माप(header) || len < 0) {
		वापस false;
	}

	संदेश.type = FETCH_HEADER;
	संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *) header;
	संदेश.तथ्य.header_or_तथ्य.len = len;

	वापस fetch_resource_send_callback(&संदेश, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
अटल bool fetch_resource_redirect_handler(काष्ठा fetch_resource_context *ctx)
{
	fetch_संदेश संदेश;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, 302);

	संदेश.type = FETCH_REDIRECT;
	संदेश.तथ्य.redirect = nsurl_access(ctx->entry->redirect_url);
	fetch_resource_send_callback(&संदेश, ctx);

	वापस true;
}

/* resource handler that returns data directly */
अटल bool fetch_resource_तथ्य_handler(काष्ठा fetch_resource_context *ctx)
{
	fetch_संदेश संदेश;

	/* Check if we can just return not modified */
	अगर (ctx->etag != 0 && ctx->etag == DIRECT_ETAG_मूल्य) {
		fetch_set_http_code(ctx->fetchh, 304);
		संदेश.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&संदेश, ctx);
		वापस true;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	अगर (fetch_resource_send_header(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_तथ्य(ctx->entry->path)))) {
		जाओ fetch_resource_तथ्य_aborted;
	}

	/* content length */
	अगर (fetch_resource_send_header(ctx, "Content-Length: %" PRIsizet,
				       ctx->entry->तथ्य_len)) {
		जाओ fetch_resource_तथ्य_aborted;
	}

	/* create etag */
	अगर (fetch_resource_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) DIRECT_ETAG_मूल्य)) {
		जाओ fetch_resource_तथ्य_aborted;
	}

	/* create max-age of 1 year */
	अगर (fetch_resource_send_header(ctx,
			"Cache-Control: max-age=31536000")) {
		जाओ fetch_resource_तथ्य_aborted;
	}

	संदेश.type = FETCH_तथ्य;
	संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *) ctx->entry->तथ्य;
	संदेश.तथ्य.header_or_तथ्य.len = ctx->entry->तथ्य_len;
	fetch_resource_send_callback(&संदेश, ctx);

	अगर (ctx->aborted == false) {
		संदेश.type = FETCH_FINISHED;
		fetch_resource_send_callback(&संदेश, ctx);
	}

fetch_resource_तथ्य_aborted:

	वापस true;
}

अटल bool fetch_resource_notfound_handler(काष्ठा fetch_resource_context *ctx)
{
	fetch_संदेश संदेश;
	पूर्णांक code = 404;
	अक्षर बफर[1024];
	स्थिर अक्षर *title;
	अक्षर कुंजी[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	अगर (fetch_resource_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		जाओ fetch_resource_notfound_handler_aborted;

	snprintf(कुंजी, माप कुंजी, "HTTP%03d", code);
	title = messages_get(कुंजी);

	snprintf(बफर, माप बफर,
		 "<html><head>"
		 "<title>%s</title>"
		 "<link rel=\"stylesheet\" type=\"text/css\" "
		 "href=\"resource:internal.css\">\n"
		 "</head>"
		 "<body class=\"ns-even-bg ns-even-fg ns-border\" "
		 "id =\"fetcherror\">\n"
		 "<h1 class=\"ns-border ns-odd-fg-bad\">%s</h1>\n"
		 "<p>%s %d %s %s</p>\n"
		 "</body>\n</html>\n",
		 title, title,
		 messages_get("FetchErrorCode"), code,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	संदेश.type = FETCH_तथ्य;
	संदेश.तथ्य.header_or_तथ्य.buf = (स्थिर uint8_t *) बफर;
	संदेश.तथ्य.header_or_तथ्य.len = म_माप(बफर);
	अगर (fetch_resource_send_callback(&संदेश, ctx))
		जाओ fetch_resource_notfound_handler_aborted;

	संदेश.type = FETCH_FINISHED;
	fetch_resource_send_callback(&संदेश, ctx);

fetch_resource_notfound_handler_aborted:
	वापस false;
}



/** callback to initialise the resource fetcher. */
अटल bool fetch_resource_initialise(lwc_string *scheme)
{
	काष्ठा fetch_resource_map_entry *e;
	uint32_t i;
	nserror res;

	fetch_resource_path_गिनती = 0;

	क्रम (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_path_गिनती];

		अगर (lwc_intern_string(fetch_resource_paths[i],
				म_माप(fetch_resource_paths[i]),
				&e->path) != lwc_error_ok) {
			जबतक (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].path);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->तथ्य = शून्य;
		res = guit->fetch->get_resource_तथ्य(lwc_string_तथ्य(e->path),
						     &e->तथ्य,
						     &e->तथ्य_len);
		अगर (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_path_गिनती++;
		} अन्यथा {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			अगर (e->redirect_url == शून्य) {
				lwc_string_unref(e->path);
			} अन्यथा {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_path_गिनती++;
			}
		}
	}

	वापस true;
}

/** callback to finalise the resource fetcher. */
अटल व्योम fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	क्रम (i = 0; i < fetch_resource_path_गिनती; i++) {
		lwc_string_unref(fetch_resource_map[i].path);
		अगर (fetch_resource_map[i].तथ्य != शून्य) {
			guit->fetch->release_resource_तथ्य(fetch_resource_map[i].तथ्य);
		} अन्यथा {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

अटल bool fetch_resource_can_fetch(स्थिर nsurl *url)
{
	वापस true;
}

/**
 * set up a resource fetch context.
 */
अटल व्योम *
fetch_resource_setup(काष्ठा fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     स्थिर अक्षर *post_urlenc,
		     स्थिर काष्ठा fetch_multipart_तथ्य *post_multipart,
		     स्थिर अक्षर **headers)
{
	काष्ठा fetch_resource_context *ctx;
	lwc_string *path;
	nserror ret;
	uint32_t i;

	ctx = सुस्मृति(1, माप(*ctx));
	अगर (ctx == शून्य) {
		वापस शून्य;
	}

	ctx->handler = fetch_resource_notfound_handler;

	अगर ((path = nsurl_get_component(url, NSURL_PATH)) != शून्य) {
		bool match;

		/* Ensure requested path is valid */
		क्रम (i = 0; i < fetch_resource_path_गिनती; i++) {
			अगर (lwc_string_isequal(path,
					fetch_resource_map[i].path,
					&match) == lwc_error_ok && match) {
				/* found a url match, select handler */
				ctx->entry = &fetch_resource_map[i];
				अगर (ctx->entry->तथ्य != शून्य) {
					ctx->handler = fetch_resource_तथ्य_handler;
				} अन्यथा {
					ctx->handler = fetch_resource_redirect_handler;
				}
				अवरोध;
			}
		}

		lwc_string_unref(path);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	क्रम (i = 0; headers[i] != शून्य; i++) {
		अगर (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			जारी;
		}

		/* If-None-Match: "12345678" */
		स्थिर अक्षर *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		जबतक (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		अगर (*d != '\0') {
			ret = nsc_snptimet(d, म_माप(d), &ctx->etag);
			अगर (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_जोड़ो(ring, ctx);

	वापस ctx;
}

/** callback to free a resource fetch */
अटल व्योम fetch_resource_free(व्योम *ctx)
{
	काष्ठा fetch_resource_context *c = ctx;
	अगर (c->url != शून्य)
		nsurl_unref(c->url);
	मुक्त(ctx);
}

/** callback to start a resource fetch */
अटल bool fetch_resource_start(व्योम *ctx)
{
	वापस true;
}

/** callback to abort a resource fetch */
अटल व्योम fetch_resource_abort(व्योम *ctx)
{
	काष्ठा fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
अटल व्योम fetch_resource_poll(lwc_string *scheme)
{
	काष्ठा fetch_resource_context *c, *save_ring = शून्य;

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
			/* resource fetches can be processed in one go */
			c->handler(c);
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

nserror fetch_resource_register(व्योम)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	स्थिर काष्ठा fetcher_operation_table fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.setup = fetch_resource_setup,
		.start = fetch_resource_start,
		.पात = fetch_resource_abort,
		.मुक्त = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	वापस fetcher_add(scheme, &fetcher_ops);
}
