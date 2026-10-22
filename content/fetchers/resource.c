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

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <libwapcaplet/libwapcaplet.h>

#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/time.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/ring.h"
#अन्तर्भाव "netsurf/fetch.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "content/fetch.h"
#अन्तर्भाव "content/fetchers.h"
#अन्तर्भाव "content/fetchers/resource.h"

#define DIRECT_ETAG_मान 123456

/** Valid resource paths */
स्थिर अचर वर्ण *fetch_resource_paths[] = {
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
स्थिर संरचना fetch_resource_map_entry {
	lwc_string *path; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	अचर uint8_t *data; /**< direct pointer to data */
	माप_प्रकार data_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

संरचना fetch_resource_context;

प्रकारनिर्देश bool (*fetch_resource_handler)(संरचना fetch_resource_context *);

/** Context for an resource fetch */
संरचना fetch_resource_context {
	संरचना fetch_resource_context *r_next, *r_prev;

	संरचना fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	संरचना fetch_resource_map_entry *entry; /**< resource map entry */

	fetch_resource_handler handler;

	समय_प्रकार etag;
};

स्थिर संरचना fetch_resource_context *ring = रिक्त;

स्थिर uint32_t fetch_resource_path_गन्ती;

/** issue fetch callbacks with locking */
स्थिर अन्तःस्थ bool fetch_resource_send_callback(अचर fetch_सन्देश *सन्देश,
		संरचना fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(सन्देश, ctx->fetchh);
	ctx->locked = false;

	फर्काऊ ctx->aborted;
}

स्थिर bool fetch_resource_send_header(संरचना fetch_resource_context *ctx,
		अचर वर्ण *fmt, ...)
{
	fetch_सन्देश सन्देश;
	वर्ण header[64];
	बहु_सूची ap;
	पूर्णाङ्क len;

	बहु_शुरू(ap, fmt);
	len = vsnprintf(header, परिमाण header, fmt, ap);
	बहु_पूर्ण(ap);

	यदि (len >= (पूर्णाङ्क)परिमाण(header) || len < 0) {
		फर्काऊ false;
	}

	सन्देश.type = FETCH_HEADER;
	सन्देश.data.header_or_data.buf = (अचर uint8_t *) header;
	सन्देश.data.header_or_data.len = len;

	फर्काऊ fetch_resource_send_callback(&सन्देश, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
स्थिर bool fetch_resource_redirect_handler(संरचना fetch_resource_context *ctx)
{
	fetch_सन्देश सन्देश;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, 302);

	सन्देश.type = FETCH_REDIRECT;
	सन्देश.data.redirect = nsurl_access(ctx->entry->redirect_url);
	fetch_resource_send_callback(&सन्देश, ctx);

	फर्काऊ true;
}

/* resource handler that returns data directly */
स्थिर bool fetch_resource_data_handler(संरचना fetch_resource_context *ctx)
{
	fetch_सन्देश सन्देश;

	/* Check if we can just return not modified */
	यदि (ctx->etag != 0 && ctx->etag == DIRECT_ETAG_मान) {
		fetch_set_http_code(ctx->fetchh, 304);
		सन्देश.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&सन्देश, ctx);
		फर्काऊ true;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	यदि (fetch_resource_send_header(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_data(ctx->entry->path)))) {
		गच्छ fetch_resource_data_aborted;
	}

	/* content length */
	यदि (fetch_resource_send_header(ctx, "Content-Length: %" PRIsizet,
				       ctx->entry->data_len)) {
		गच्छ fetch_resource_data_aborted;
	}

	/* create etag */
	यदि (fetch_resource_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) DIRECT_ETAG_मान)) {
		गच्छ fetch_resource_data_aborted;
	}

	/* create max-age of 1 year */
	यदि (fetch_resource_send_header(ctx,
			"Cache-Control: max-age=31536000")) {
		गच्छ fetch_resource_data_aborted;
	}

	सन्देश.type = FETCH_DATA;
	सन्देश.data.header_or_data.buf = (अचर uint8_t *) ctx->entry->data;
	सन्देश.data.header_or_data.len = ctx->entry->data_len;
	fetch_resource_send_callback(&सन्देश, ctx);

	यदि (ctx->aborted == false) {
		सन्देश.type = FETCH_FINISHED;
		fetch_resource_send_callback(&सन्देश, ctx);
	}

fetch_resource_data_aborted:

	फर्काऊ true;
}

स्थिर bool fetch_resource_notfound_handler(संरचना fetch_resource_context *ctx)
{
	fetch_सन्देश सन्देश;
	पूर्णाङ्क code = 404;
	वर्ण buffer[1024];
	अचर वर्ण *title;
	वर्ण साँचो[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	यदि (fetch_resource_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		गच्छ fetch_resource_notfound_handler_aborted;

	snprintf(साँचो, परिमाण साँचो, "HTTP%03d", code);
	title = messages_get(साँचो);

	snprintf(buffer, परिमाण buffer,
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

	सन्देश.type = FETCH_DATA;
	सन्देश.data.header_or_data.buf = (अचर uint8_t *) buffer;
	सन्देश.data.header_or_data.len = म_माप(buffer);
	यदि (fetch_resource_send_callback(&सन्देश, ctx))
		गच्छ fetch_resource_notfound_handler_aborted;

	सन्देश.type = FETCH_FINISHED;
	fetch_resource_send_callback(&सन्देश, ctx);

fetch_resource_notfound_handler_aborted:
	फर्काऊ false;
}



/** callback to initialise the resource fetcher. */
स्थिर bool fetch_resource_initialise(lwc_string *scheme)
{
	संरचना fetch_resource_map_entry *e;
	uint32_t i;
	nserror res;

	fetch_resource_path_गन्ती = 0;

	लागि (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_path_गन्ती];

		यदि (lwc_intern_string(fetch_resource_paths[i],
				म_माप(fetch_resource_paths[i]),
				&e->path) != lwc_error_ok) {
			जबसम्म (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].path);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->data = रिक्त;
		res = guit->fetch->get_resource_data(lwc_string_data(e->path),
						     &e->data,
						     &e->data_len);
		यदि (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_path_गन्ती++;
		} अन्यथा {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			यदि (e->redirect_url == रिक्त) {
				lwc_string_unref(e->path);
			} अन्यथा {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_path_गन्ती++;
			}
		}
	}

	फर्काऊ true;
}

/** callback to finalise the resource fetcher. */
स्थिर शून्य fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	लागि (i = 0; i < fetch_resource_path_गन्ती; i++) {
		lwc_string_unref(fetch_resource_map[i].path);
		यदि (fetch_resource_map[i].data != रिक्त) {
			guit->fetch->release_resource_data(fetch_resource_map[i].data);
		} अन्यथा {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

स्थिर bool fetch_resource_can_fetch(अचर nsurl *url)
{
	फर्काऊ true;
}

/**
 * set up a resource fetch context.
 */
स्थिर शून्य *
fetch_resource_setup(संरचना fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     अचर वर्ण *post_urlenc,
		     अचर संरचना fetch_multipart_data *post_multipart,
		     अचर वर्ण **headers)
{
	संरचना fetch_resource_context *ctx;
	lwc_string *path;
	nserror ret;
	uint32_t i;

	ctx = सुस्मृति(1, परिमाण(*ctx));
	यदि (ctx == रिक्त) {
		फर्काऊ रिक्त;
	}

	ctx->handler = fetch_resource_notfound_handler;

	यदि ((path = nsurl_get_component(url, NSURL_PATH)) != रिक्त) {
		bool match;

		/* Ensure requested path is valid */
		लागि (i = 0; i < fetch_resource_path_गन्ती; i++) {
			यदि (lwc_string_isequal(path,
					fetch_resource_map[i].path,
					&match) == lwc_error_ok && match) {
				/* found a url match, select handler */
				ctx->entry = &fetch_resource_map[i];
				यदि (ctx->entry->data != रिक्त) {
					ctx->handler = fetch_resource_data_handler;
				} अन्यथा {
					ctx->handler = fetch_resource_redirect_handler;
				}
				रोक;
			}
		}

		lwc_string_unref(path);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	लागि (i = 0; headers[i] != रिक्त; i++) {
		यदि (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			जारी;
		}

		/* If-None-Match: "12345678" */
		अचर वर्ण *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		जबसम्म (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		यदि (*d != '\0') {
			ret = nsc_snptimet(d, म_माप(d), &ctx->etag);
			यदि (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_घुसाउ(ring, ctx);

	फर्काऊ ctx;
}

/** callback to free a resource fetch */
स्थिर शून्य fetch_resource_free(शून्य *ctx)
{
	संरचना fetch_resource_context *c = ctx;
	यदि (c->url != रिक्त)
		nsurl_unref(c->url);
	मुक्त(ctx);
}

/** callback to start a resource fetch */
स्थिर bool fetch_resource_start(शून्य *ctx)
{
	फर्काऊ true;
}

/** callback to abort a resource fetch */
स्थिर शून्य fetch_resource_abort(शून्य *ctx)
{
	संरचना fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
स्थिर शून्य fetch_resource_poll(lwc_string *scheme)
{
	संरचना fetch_resource_context *c, *save_ring = रिक्त;

	जबसम्म (ring != रिक्त) {
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
			RING_घुसाउ(save_ring, c);
			जारी;
		}

		/* Only process non-aborted fetches */
		यदि (c->aborted == false) {
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

nserror fetch_resource_register(शून्य)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	अचर संरचना fetcher_operation_table fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.setup = fetch_resource_setup,
		.start = fetch_resource_start,
		.पात = fetch_resource_abort,
		.मुक्त = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	फर्काऊ fetcher_add(scheme, &fetcher_ops);
}
