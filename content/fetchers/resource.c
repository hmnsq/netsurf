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

#परिभाषय प्रत्यक्षम्_ETAG_मानम् 123456

/** Valid resource paths */
स्थिर ध्रुव वर्ण *fetch_resource_paths[] = {
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
स्थिर संरचना fetch_resource_map_प्रविष्टिः {
	lwc_string *मार्गः; /**< resource scheme path */
	nsurl *redirect_url; /**< url to redirect to */
	ध्रुव uint8_t *दत्त; /**< direct pointer to data */
	size_t दत्त_len; /**< length of direct data */
} fetch_resource_map[NOF_ELEMENTS(fetch_resource_paths)];

संरचना fetch_resource_context;

प्रकारनिर्देश bool (*fetch_resource_handler)(संरचना fetch_resource_context *);

/** Context for an resource fetch */
संरचना fetch_resource_context {
	संरचना fetch_resource_context *r_अग्र, *r_पूर्व;

	संरचना fetch *fetchh; /**< Handle for this fetch */

	bool aborted; /**< Flag indicating fetch has been aborted */
	bool locked; /**< Flag indicating entry is already entered */

	nsurl *url; /**< requested url */

	संरचना fetch_resource_map_प्रविष्टिः *प्रविष्टिः; /**< resource map entry */

	fetch_resource_handler handler;

	time_t etag;
};

स्थिर संरचना fetch_resource_context *ring = रिक्त;

स्थिर uint32_t fetch_resource_मार्गः_गणनम्;

/** issue fetch callbacks with locking */
स्थिर अन्तःस्थ bool fetch_resource_send_callback(ध्रुव fetch_सन्देशः *सन्देशः,
		संरचना fetch_resource_context *ctx)
{
	ctx->locked = true;
	fetch_send_callback(सन्देशः, ctx->fetchh);
	ctx->locked = false;

	प्रत्यागम ctx->aborted;
}

स्थिर bool fetch_resource_send_शीर्षकम्(संरचना fetch_resource_context *ctx,
		ध्रुव वर्ण *fmt, ...)
{
	fetch_सन्देशः सन्देशः;
	वर्ण शीर्षकम्[64];
	va_list ap;
	पूर्णाङ्क len;

	va_start(ap, fmt);
	len = vsnprintf(शीर्षकम्, परिमाण शीर्षकम्, fmt, ap);
	va_end(ap);

	यदि (len >= (पूर्णाङ्क)परिमाण(शीर्षकम्) || len < 0) {
		प्रत्यागम false;
	}

	सन्देशः.type = FETCH_शीर्षकम्;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *) शीर्षकम्;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = len;

	प्रत्यागम fetch_resource_send_callback(&सन्देशः, ctx);
}



/**
 * resource handler that results in a redirect to another url.
 */
स्थिर bool fetch_resource_redirect_handler(संरचना fetch_resource_context *ctx)
{
	fetch_सन्देशः सन्देशः;

	/* content is going to return redirect */
	fetch_समुच्चयः_http_संहिता(ctx->fetchh, 302);

	सन्देशः.type = FETCH_REDIRECT;
	सन्देशः.दत्त.redirect = nsurl_access(ctx->प्रविष्टिः->redirect_url);
	fetch_resource_send_callback(&सन्देशः, ctx);

	प्रत्यागम true;
}

/* resource handler that returns data directly */
स्थिर bool fetch_resource_दत्त_handler(संरचना fetch_resource_context *ctx)
{
	fetch_सन्देशः सन्देशः;

	/* Check if we can just return not modified */
	यदि (ctx->etag != 0 && ctx->etag == प्रत्यक्षम्_ETAG_मानम्) {
		fetch_समुच्चयः_http_संहिता(ctx->fetchh, 304);
		सन्देशः.type = FETCH_NOTMODIFIED;
		fetch_resource_send_callback(&सन्देशः, ctx);
		प्रत्यागम true;
	}

	/* fetch is going to be successful */
	fetch_समुच्चयः_http_संहिता(ctx->fetchh, 200);

	/* Any callback can result in the fetch being aborted.
	 * Therefore, we _must_ check for this after _every_ call to
	 * fetch_file_send_callback().
	 */

	/* content type */
	यदि (fetch_resource_send_शीर्षकम्(ctx, "Content-Type: %s",
				       guit->fetch->filetype(lwc_string_दत्त(ctx->प्रविष्टिः->मार्गः)))) {
		गच्छ fetch_resource_दत्त_aborted;
	}

	/* content length */
	यदि (fetch_resource_send_शीर्षकम्(ctx, "Content-Length: %" PRIsizet,
				       ctx->प्रविष्टिः->दत्त_len)) {
		गच्छ fetch_resource_दत्त_aborted;
	}

	/* create etag */
	यदि (fetch_resource_send_शीर्षकम्(ctx, "ETag: \"%10" PRId64 "\"",
				       (int64_t) प्रत्यक्षम्_ETAG_मानम्)) {
		गच्छ fetch_resource_दत्त_aborted;
	}

	/* create max-age of 1 year */
	यदि (fetch_resource_send_शीर्षकम्(ctx,
			"Cache-Control: max-age=31536000")) {
		गच्छ fetch_resource_दत्त_aborted;
	}

	सन्देशः.type = FETCH_दत्त;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *) ctx->प्रविष्टिः->दत्त;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = ctx->प्रविष्टिः->दत्त_len;
	fetch_resource_send_callback(&सन्देशः, ctx);

	यदि (ctx->aborted == false) {
		सन्देशः.type = FETCH_परिसमाप्तम्;
		fetch_resource_send_callback(&सन्देशः, ctx);
	}

fetch_resource_दत्त_aborted:

	प्रत्यागम true;
}

स्थिर bool fetch_resource_notfound_handler(संरचना fetch_resource_context *ctx)
{
	fetch_सन्देशः सन्देशः;
	पूर्णाङ्क संहिता = 404;
	वर्ण बफर[1024];
	ध्रुव वर्ण *title;
	वर्ण कुञ्चिका[8];

	/* content is going to return error code */
	fetch_समुच्चयः_http_संहिता(ctx->fetchh, संहिता);

	/* content type */
	यदि (fetch_resource_send_शीर्षकम्(ctx, "Content-Type: text/html; charset=utf-8"))
		गच्छ fetch_resource_notfound_handler_aborted;

	snprintf(कुञ्चिका, परिमाण कुञ्चिका, "HTTP%03d", संहिता);
	title = messages_get(कुञ्चिका);

	snprintf(बफर, परिमाण बफर,
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
		 messages_get("FetchErrorCode"), संहिता,
		 messages_get("FetchFile"), nsurl_access(ctx->url));

	सन्देशः.type = FETCH_दत्त;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.buf = (ध्रुव uint8_t *) बफर;
	सन्देशः.दत्त.शीर्षकम्_or_दत्त.len = वाक्यदैर्घ्यम्(बफर);
	यदि (fetch_resource_send_callback(&सन्देशः, ctx))
		गच्छ fetch_resource_notfound_handler_aborted;

	सन्देशः.type = FETCH_परिसमाप्तम्;
	fetch_resource_send_callback(&सन्देशः, ctx);

fetch_resource_notfound_handler_aborted:
	प्रत्यागम false;
}



/** callback to initialise the resource fetcher. */
स्थिर bool fetch_resource_initialise(lwc_string *scheme)
{
	संरचना fetch_resource_map_प्रविष्टिः *e;
	uint32_t i;
	nserror res;

	fetch_resource_मार्गः_गणनम् = 0;

	प्रति (i = 0; i < NOF_ELEMENTS(fetch_resource_paths); i++) {
		e = &fetch_resource_map[fetch_resource_मार्गः_गणनम्];

		यदि (lwc_intern_string(fetch_resource_paths[i],
				वाक्यदैर्घ्यम्(fetch_resource_paths[i]),
				&e->मार्गः) != lwc_error_ok) {
			यावत् (i > 0) {
				i--;
				lwc_string_unref(fetch_resource_map[i].मार्गः);
				nsurl_unref(fetch_resource_map[i].redirect_url);
			}
			/** \todo should this exit with an error condition? */
		}

		e->दत्त = रिक्त;
		res = guit->fetch->get_resource_दत्त(lwc_string_दत्त(e->मार्गः),
						     &e->दत्त,
						     &e->दत्त_len);
		यदि (res == NSERROR_OK) {
			NSLOG(netsurf, INFO, "direct data for %s",
			      fetch_resource_paths[i]);
			fetch_resource_मार्गः_गणनम्++;
		} अन्यथा {
			e->redirect_url = guit->fetch->get_resource_url(fetch_resource_paths[i]);
			यदि (e->redirect_url == रिक्त) {
				lwc_string_unref(e->मार्गः);
			} अन्यथा {
				NSLOG(netsurf, INFO, "redirect url for %s",
				      fetch_resource_paths[i]);
				fetch_resource_मार्गः_गणनम्++;
			}
		}
	}

	प्रत्यागम true;
}

/** callback to finalise the resource fetcher. */
स्थिर शून्य fetch_resource_finalise(lwc_string *scheme)
{
	uint32_t i;

	प्रति (i = 0; i < fetch_resource_मार्गः_गणनम्; i++) {
		lwc_string_unref(fetch_resource_map[i].मार्गः);
		यदि (fetch_resource_map[i].दत्त != रिक्त) {
			guit->fetch->release_resource_दत्त(fetch_resource_map[i].दत्त);
		} अन्यथा {
			nsurl_unref(fetch_resource_map[i].redirect_url);
		}
	}
}

स्थिर bool fetch_resource_can_fetch(ध्रुव nsurl *url)
{
	प्रत्यागम true;
}

/**
 * set up a resource fetch context.
 */
स्थिर शून्य *
fetch_resource_सज्जता(संरचना fetch *fetchh,
		     nsurl *url,
		     bool only_2xx,
		     bool downgrade_tls,
		     ध्रुव वर्ण *post_urlenc,
		     ध्रुव संरचना fetch_multipart_दत्त *post_multipart,
		     ध्रुव वर्ण **शीर्षकाणि)
{
	संरचना fetch_resource_context *ctx;
	lwc_string *मार्गः;
	nserror ret;
	uint32_t i;

	ctx = शून्यस्मृतिं_देहि(1, परिमाण(*ctx));
	यदि (ctx == रिक्त) {
		प्रत्यागम रिक्त;
	}

	ctx->handler = fetch_resource_notfound_handler;

	यदि ((मार्गः = nsurl_get_component(url, NSURL_मार्गः)) != रिक्त) {
		bool मेलनम्;

		/* Ensure requested path is valid */
		प्रति (i = 0; i < fetch_resource_मार्गः_गणनम्; i++) {
			यदि (lwc_string_isequal(मार्गः,
					fetch_resource_map[i].मार्गः,
					&मेलनम्) == lwc_error_ok && मेलनम्) {
				/* found a url match, select handler */
				ctx->प्रविष्टिः = &fetch_resource_map[i];
				यदि (ctx->प्रविष्टिः->दत्त != रिक्त) {
					ctx->handler = fetch_resource_दत्त_handler;
				} अन्यथा {
					ctx->handler = fetch_resource_redirect_handler;
				}
				भङ्ग;
			}
		}

		lwc_string_unref(मार्गः);
	}

	ctx->url = nsurl_ref(url);

	/* Scan request headers looking for If-None-Match */
	प्रति (i = 0; शीर्षकाणि[i] != रिक्त; i++) {
		यदि (strncasecmp(शीर्षकाणि[i], "If-None-Match:",
				SLEN("If-None-Match:")) != 0) {
			अनुवर्त;
		}

		/* If-None-Match: "12345678" */
		ध्रुव वर्ण *d = शीर्षकाणि[i] + SLEN("If-None-Match:");

		/* Scan to first digit, if any */
		यावत् (*d != '\0' && (*d < '0' || '9' < *d))
			d++;

		/* Convert to time_t */
		यदि (*d != '\0') {
			ret = nsc_snptimet(d, वाक्यदैर्घ्यम्(d), &ctx->etag);
			यदि (ret != NSERROR_OK) {
				NSLOG(fetch, WARNING,
						"Bad If-None-Match value");
			}
		}
	}

	ctx->fetchh = fetchh;

	RING_निवेशय(ring, ctx);

	प्रत्यागम ctx;
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
	प्रत्यागम true;
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

nserror fetch_resource_पञ्जिकाकोशः(शून्य)
{
	lwc_string *scheme = lwc_string_ref(corestring_lwc_resource);
	ध्रुव संरचना fetcher_operation_सारणी fetcher_ops = {
		.initialise = fetch_resource_initialise,
		.acceptable = fetch_resource_can_fetch,
		.सज्जता = fetch_resource_सज्जता,
		.start = fetch_resource_start,
		.abort = fetch_resource_abort,
		.मुक्त = fetch_resource_free,
		.poll = fetch_resource_poll,
		.finalise = fetch_resource_finalise
	};

	प्रत्यागम fetcher_सङ्कलय(scheme, &fetcher_ops);
}
