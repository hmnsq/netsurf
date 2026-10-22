/*
 * Copyright 2008 Rob Kendrick <rjek@netsurf-browser.org>
 * Copyright 2013 John-Mark Bell <jmb@netsurf-browser.org>
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
 * HTML fetcher for CSS objects
 */

#ضمن <assert.h>
#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <dom/dom.h>
#ضمن <libwapcaplet/libwapcaplet.h>

#ضمن "netsurf/inttypes.h"
#ضمن "utils/config.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"
#ضمن "utils/ring.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/utils.h"
#ضمن "content/fetch.h"
#ضمن "content/fetchers.h"

#ضمن "html/private.h"

تعريفنوع بنية html_css_fetcher_قلم {
	uint32_t کلید;
	dom_string *داده;
	nsurl *پایه_url;

	بنية html_css_fetcher_قلم *r_بعدی, *r_prev;
} html_css_fetcher_قلم;

تعريفنوع بنية html_css_fetcher_context {
	بنية fetch *والد_fetch;

	nsurl *url;
	html_css_fetcher_قلم *قلم;

	bool aborted;
	bool locked;

	بنية html_css_fetcher_context *r_بعدی, *r_prev;
} html_css_fetcher_context;

ساكن uint32_t current_کلید = 0;
ساكن html_css_fetcher_قلم *اقلام = عدم;
ساكن html_css_fetcher_context *ring = عدم;

ساكن bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_داده(scheme));
	أرجع true;
}

ساكن فراغ html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_داده(scheme));
}

ساكن bool html_css_fetcher_can_fetch(ثابت nsurl *url)
{
	أرجع true;
}

ساكن فراغ *html_css_fetcher_راهاندازی(بنية fetch *والد_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_داده *post_multipart,
		 ثابت حرف **سرآیندها)
{
	html_css_fetcher_context *ctx;
	lwc_string *مسیر;
	uint32_t کلید;
	html_css_fetcher_قلم *قلم, *یافت = عدم;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	مسیر = nsurl_get_component(url, NSURL_مسیر);
	/* The path must exist */
	إذا (مسیر == عدم) {
		أرجع عدم;
	}

	کلید = strtoul(lwc_string_داده(مسیر), عدم, 10);

	lwc_string_unref(مسیر);

	/* There must be at least one item */
	إذا (اقلام == عدم) {
		أرجع عدم;
	}

	قلم = اقلام;
	افعل {
		إذا (قلم->کلید == کلید) {
			یافت = قلم;
			اكسر;
		}

		قلم = قلم->r_بعدی;
	} طالما (قلم != اقلام);

	/* We must have found the item */
	إذا (یافت == عدم) {
		أرجع عدم;
	}

	ctx = حافظه_صفر_بده(1, حجم(*ctx));
	إذا (ctx == عدم)
		أرجع عدم;

	ctx->والد_fetch = والد_fetch;
	ctx->url = nsurl_ref(url);
	ctx->قلم = یافت;

	RING_درج(ring, ctx);

	أرجع ctx;
}

ساكن bool html_css_fetcher_شروع(فراغ *ctx)
{
	أرجع true;
}

ساكن فراغ html_css_fetcher_free(فراغ *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	إذا (c->قلم != عدم) {
		nsurl_unref(c->قلم->پایه_url);
		dom_string_unref(c->قلم->داده);
		RING_REMOVE(اقلام, c->قلم);
		حرر(c->قلم);
	}
	RING_REMOVE(ring, c);
	حرر(ctx);
}

ساكن فراغ html_css_fetcher_abort(فراغ *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ساكن فراغ html_css_fetcher_send_callback(ثابت fetch_پیام *پیام,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(پیام, c->والد_fetch);
	c->locked = false;
}

ساكن فراغ html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_پیام پیام;
	html_css_fetcher_context *c, *بعدی;

	إذا (ring == عدم) أرجع;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	افعل {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		إذا (c->locked == true) {
			بعدی = c->r_بعدی;
			استمر;
		}

		/* Only process non-aborted fetches */
		إذا (c->aborted) {
			/* Nothing to do */
			أكد(c->locked == false);
		} وإلا إذا (c->قلم != عدم) {
			حرف سرآیند[4096];

			fetch_مجموعه_http_کد(c->والد_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(سرآیند, حجم سرآیند,
				"Content-Type: text/css; charset=utf-8");
			پیام.type = FETCH_سرآیند;
			پیام.داده.سرآیند_or_داده.buf = (ثابت uint8_t *) سرآیند;
			پیام.داده.سرآیند_or_داده.len = طول_رشته(سرآیند);
			html_css_fetcher_send_callback(&پیام, c);

			إذا (c->aborted == false) {
				snprintf(سرآیند, حجم سرآیند,
					"Content-Length: %"PRIsizet,
					dom_string_byte_درازی(c->قلم->داده));
				پیام.type = FETCH_سرآیند;
				پیام.داده.سرآیند_or_داده.buf =
						(ثابت uint8_t *) سرآیند;
				پیام.داده.سرآیند_or_داده.len = طول_رشته(سرآیند);
				html_css_fetcher_send_callback(&پیام, c);
			}

			إذا (c->aborted == false) {
				snprintf(سرآیند, حجم سرآیند,
					"X-NS-Base: %.*s",
					(صحيح) nsurl_درازی(c->قلم->پایه_url),
					nsurl_access(c->قلم->پایه_url));
				پیام.type = FETCH_سرآیند;
				پیام.داده.سرآیند_or_داده.buf =
						(ثابت uint8_t *) سرآیند;
				پیام.داده.سرآیند_or_داده.len = طول_رشته(سرآیند);
				html_css_fetcher_send_callback(&پیام, c);
			}

			إذا (c->aborted == false) {
				پیام.type = FETCH_داده;
				پیام.داده.سرآیند_or_داده.buf =
						(ثابت uint8_t *)
						dom_string_داده(c->قلم->داده);
				پیام.داده.سرآیند_or_داده.len =
					dom_string_byte_درازی(c->قلم->داده);
				html_css_fetcher_send_callback(&پیام, c);
			}

			إذا (c->aborted == false) {
				پیام.type = FETCH_پایانیافته;
				html_css_fetcher_send_callback(&پیام, c);
			}
		} وإلا {
			NSLOG(netsurf, INFO, "Processing of %s failed!",
			      nsurl_access(c->url));

			/* Ensure that we're unlocked here. If we aren't,
			 * then html_css_fetcher_process() is broken.
			 */
			أكد(c->locked == false);
		}

		/* Compute next fetch item at the last possible moment as
		 * processing this item may have added to the ring.
		 */
		بعدی = c->r_بعدی;

		fetch_remove_from_queues(c->والد_fetch);
		fetch_free(c->والد_fetch);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} طالما ( (c = بعدی) != ring && ring != عدم);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_ثبات(فراغ)
{
	ثابت بنية fetcher_operation_جدول html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.راهاندازی = html_css_fetcher_راهاندازی,
		.شروع = html_css_fetcher_شروع,
		.abort = html_css_fetcher_abort,
		.حرر = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	أرجع fetcher_جمع_کن(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_جمع_کن_قلم(dom_string *داده, nsurl *پایه_url, uint32_t *کلید)
{
	html_css_fetcher_قلم *قلم = حافظه_بده(حجم(*قلم));

	إذا (قلم == عدم) {
		أرجع NSERROR_NOMEM;
	}

	*کلید = قلم->کلید = current_کلید++;
	قلم->داده = dom_string_ref(داده);
	قلم->پایه_url = nsurl_ref(پایه_url);

	RING_درج(اقلام, قلم);

	أرجع NSERROR_OK;
}
