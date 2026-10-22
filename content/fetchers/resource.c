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

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <libwapcaplet/libwapcaplet.h>

#അന്തര്ഭാവ "netsurf/inttypes.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/time.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/ring.h"
#അന്തര്ഭാവ "netsurf/fetch.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"

#അന്തര്ഭാവ "content/fetch.h"
#അന്തര്ഭാവ "content/fetchers.h"
#അന്തര്ഭാവ "content/fetchers/resource.h"

#define DIRECT_ETAG_മൂല്യം 123456

/** Valid resource paths */
സ്ഥിതം സ്ഥിരം അക്ഷരം *fetch_resource_paths[] = {
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
സ്ഥിതം ഘടന fetch_resource_map_entry {
	lwc_string *path; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	സ്ഥിരം uint8_t *data; /**< direct pointer to data */
	size_t data_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

ഘടന fetch_resource_context;

തരനിർവചനം bool (*fetch_resource_handler)(ഘടന fetch_resource_context *);

/** Context for an resource fetch */
ഘടന fetch_resource_context {
	ഘടന fetch_resource_context *r_next, *r_prev;

	ഘടന fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	ഘടന fetch_resource_map_entry *entry; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

സ്ഥിതം ഘടന fetch_resource_context *ring = ശൂന്യ;

സ്ഥിതം uint32_t fetch_resource_path_എണ്ണം;

/** issue fetch callbacks with locking */
സ്ഥിതം വരിയിൽ bool fetch_resource_send_callback(സ്ഥിരം fetch_സന്ദേശം *സന്ദേശം,
		ഘടന fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(സന്ദേശം, ctx->fetchh);
	ctx->locked = false;

	തിരികെ ctx->aborted;
}

സ്ഥിതം bool fetch_resource_send_header(ഘടന fetch_resource_context *ctx,
		സ്ഥിരം അക്ഷരം *fmt, ...)
{
	fetch_സന്ദേശം സന്ദേശം;
	അക്ഷരം header[64];
	va_list ap;
	പൂർണ്ണസംഖ്യ len;

	va_start(ap, fmt);
	len = vsnprintf(header, വലുപ്പം header, fmt, ap);
	va_end(ap);

	എങ്കിൽ (len >= (പൂർണ്ണസംഖ്യ)വലുപ്പം(header) || len < 0) {
		തിരികെ false;
	}

	സന്ദേശം.type = FETCH_HEADER;
	സന്ദേശം.data.header_or_data.buf = (സ്ഥിരം uint8_t *) header;
	സന്ദേശം.data.header_or_data.len = len;

	തിരികെ fetch_resource_send_callback(&സന്ദേശം, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
സ്ഥിതം bool fetch_resource_redirect_handler(ഘടന fetch_resource_context *ctx)
{
	fetch_സന്ദേശം സന്ദേശം;

	/* content is going to return redirect */
	fetch_set_http_code(ctx->fetchh, 302);

	സന്ദേശം.type = FETCH_REDIRECT;
	സന്ദേശം.data.redirect = nsurl_access(ctx->entry->redirect_url);
	fetch_resource_send_callback(&സന്ദേശം, ctx);

	തിരികെ true;
}

/* resource handler that returns data directly */
സ്ഥിതം bool fetch_resource_data_handler(ഘടന fetch_resource_context *ctx)
{
	fetch_സന്ദേശം സന്ദേശം;

	/* Check if we can just return not modified */
	എങ്കിൽ (ctx->etag != 0 && ctx->etag == DIRECT_ETAG_മൂല്യം) {
		fetch_set_http_code(ctx->fetchh, 304);
		സന്ദേശം.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&സന്ദേശം, ctx);
		തിരികെ true;
	}

	/* fetch is going to be successful */
	fetch_set_http_code(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	എങ്കിൽ (fetch_resource_send_header(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_data(ctx->entry->path)))) {
		പോകുക fetch_resource_data_aborted;
	}

	/* content length */
	എങ്കിൽ (fetch_resource_send_header(ctx, "Content-Length: %" PRIsizet,
				       ctx->entry->data_len)) {
		പോകുക fetch_resource_data_aborted;
	}

	/* create etag */
	എങ്കിൽ (fetch_resource_send_header(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) DIRECT_ETAG_മൂല്യം)) {
		പോകുക fetch_resource_data_aborted;
	}

	/* create max-age of 1 year */
	എങ്കിൽ (fetch_resource_send_header(ctx,
			"Cache-Control: max-age=31536000")) {
		പോകുക fetch_resource_data_aborted;
	}

	സന്ദേശം.type = FETCH_DATA;
	സന്ദേശം.data.header_or_data.buf = (സ്ഥിരം uint8_t *) ctx->entry->data;
	സന്ദേശം.data.header_or_data.len = ctx->entry->data_len;
	fetch_resource_send_callback(&സന്ദേശം, ctx);

	എങ്കിൽ (ctx->aborted == false) {
		സന്ദേശം.type = FETCH_FINISHED;
		fetch_resource_send_callback(&സന്ദേശം, ctx);
	}

fetch_resource_data_aborted:

	തിരികെ true;
}

സ്ഥിതം bool fetch_resource_notfound_handler(ഘടന fetch_resource_context *ctx)
{
	fetch_സന്ദേശം സന്ദേശം;
	പൂർണ്ണസംഖ്യ code = 404;
	അക്ഷരം buffer[1024];
	സ്ഥിരം അക്ഷരം *title;
	അക്ഷരം താക്കോൽ[8];

	/* content is going to return error code */
	fetch_set_http_code(ctx->fetchh, code);

	/* content type */
	എങ്കിൽ (fetch_resource_send_header(ctx, "Content-Type: text/html; charset=utf-8"))
		പോകുക fetch_resource_notfound_handler_aborted;

	snprintf(താക്കോൽ, വലുപ്പം താക്കോൽ, "HTTP%03d", code);
	title = messages_get(താക്കോൽ);

	snprintf(buffer, വലുപ്പം buffer,
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

	സന്ദേശം.type = FETCH_DATA;
	സന്ദേശം.data.header_or_data.buf = (സ്ഥിരം uint8_t *) buffer;
	സന്ദേശം.data.header_or_data.len = strlen(buffer);
	എങ്കിൽ (fetch_resource_send_callback(&സന്ദേശം, ctx))
		പോകുക fetch_resource_notfound_handler_aborted;

	സന്ദേശം.type = FETCH_FINISHED;
	fetch_resource_send_callback(&സന്ദേശം, ctx);

fetch_resource_notfound_handler_aborted:
	തിരികെ false;
}



/** callback to initialise the resource fetcher. */
സ്ഥിതം bool fetch_resource_initialise(lwc_string *scheme)
{
	ഘടന fetch_resource_map_entry *e;
	uint32_t i;
	nserror res;

	fetch_resource_path_എണ്ണം = 0;

	വേണ്ടി (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_path_എണ്ണം];

		എങ്കിൽ (lwc_intern_string(fetch_resource_paths[i],
				strlen(fetch_resource_paths[i]),
				&e->path) != lwc_error_ok) {
			ആകുമ്പോൾ (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].path);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->data = ശൂന്യ;
		res = guit->fetch->get_resource_data(lwc_string_data(e->path),
						     &e->data,
						     &e->data_len);
		എങ്കിൽ (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_path_എണ്ണം++;
		} അല്ലെങ്കിൽ {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			എങ്കിൽ (e->redirect_url == ശൂന്യ) {
				lwc_string_unref(e->path);
			} അല്ലെങ്കിൽ {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_path_എണ്ണം++;
			}
		}
	}

	തിരികെ true;
}

/** callback to finalise the resource fetcher. */
സ്ഥിതം ശൂന്യം fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	വേണ്ടി (i = 0; i < fetch_resource_path_എണ്ണം; i++) {
		lwc_string_unref(fetch_resource_map[i].path);
		എങ്കിൽ (fetch_resource_map[i].data != ശൂന്യ) {
			guit->fetch->release_resource_data(fetch_resource_map[i].data);
		} അല്ലെങ്കിൽ {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

സ്ഥിതം bool fetch_resource_can_fetch(സ്ഥിരം nsurl *url)
{
	തിരികെ true;
}

/**
 * set up a resource fetch context.
 */
സ്ഥിതം ശൂന്യം *
fetch_resource_setup(ഘടന fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     സ്ഥിരം അക്ഷരം *post_urlenc,
		     സ്ഥിരം ഘടന fetch_multipart_data *post_multipart,
		     സ്ഥിരം അക്ഷരം **headers)
{
	ഘടന fetch_resource_context *ctx;
	lwc_string *path;
	nserror ret;
	uint32_t i;

	ctx = calloc(1, വലുപ്പം(*ctx));
	എങ്കിൽ (ctx == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}

	ctx->handler = fetch_resource_notfound_handler;

	എങ്കിൽ ((path = nsurl_get_component(url, NSURL_PATH)) != ശൂന്യ) {
		bool match;

		/* Ensure requested path is valid */
		വേണ്ടി (i = 0; i < fetch_resource_path_എണ്ണം; i++) {
			എങ്കിൽ (lwc_string_isequal(path,
					fetch_resource_map[i].path,
					&match) == lwc_error_ok && match) {
				/* found a url match, select handler */
				ctx->entry = &fetch_resource_map[i];
				എങ്കിൽ (ctx->entry->data != ശൂന്യ) {
					ctx->handler = fetch_resource_data_handler;
				} അല്ലെങ്കിൽ {
					ctx->handler = fetch_resource_redirect_handler;
				}
				നിർത്തുക;
			}
		}

		lwc_string_unref(path);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	വേണ്ടി (i = 0; headers[i] != ശൂന്യ; i++) {
		എങ്കിൽ (strncasecmp(headers[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			തുടരുക;
		}

		/* If-None-Match: "12345678" */
		സ്ഥിരം അക്ഷരം *d = headers[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		ആകുമ്പോൾ (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		എങ്കിൽ (*d != '\0') {
			ret = nsc_snptimet(d, strlen(d), &ctx->etag);
			എങ്കിൽ (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_തിരുകുക(ring, ctx);

	തിരികെ ctx;
}

/** callback to free a resource fetch */
സ്ഥിതം ശൂന്യം fetch_resource_free(ശൂന്യം *ctx)
{
	ഘടന fetch_resource_context *c = ctx;
	എങ്കിൽ (c->url != ശൂന്യ)
		nsurl_unref(c->url);
	സ്വതന്ത്രം(ctx);
}

/** callback to start a resource fetch */
സ്ഥിതം bool fetch_resource_start(ശൂന്യം *ctx)
{
	തിരികെ true;
}

/** callback to abort a resource fetch */
സ്ഥിതം ശൂന്യം fetch_resource_abort(ശൂന്യം *ctx)
{
	ഘടന fetch_resource_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}


/** callback to poll for additional resource fetch contents */
സ്ഥിതം ശൂന്യം fetch_resource_poll(lwc_string *scheme)
{
	ഘടന fetch_resource_context *c, *save_ring = ശൂന്യ;

	ആകുമ്പോൾ (ring != ശൂന്യ) {
		/* Take the first entry from the ring */
		c = ring;
		RING_REMOVE(ring, c);

		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		എങ്കിൽ (c->locked == true) {
			RING_തിരുകുക(save_ring, c);
			തുടരുക;
		}

		/* Only process non-aborted fetches */
		എങ്കിൽ (c->aborted == false) {
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

nserror fetch_resource_register(ശൂന്യം)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	സ്ഥിരം ഘടന fetcher_operation_table fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.setup = fetch_resource_setup,
		.start = fetch_resource_start,
		.abort = fetch_resource_abort,
		.സ്വതന്ത്രം = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	തിരികെ fetcher_add(scheme, &fetcher_ops);
}
