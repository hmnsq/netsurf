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

تعريفنوع بنية html_css_fetcher_توکی {
	uint32_t کلی;
	dom_string *معلومات;
	nsurl *بنسټ_url;

	بنية html_css_fetcher_توکی *r_بل, *r_prev;
} html_css_fetcher_توکی;

تعريفنوع بنية html_css_fetcher_context {
	بنية fetch *پلار_fetch;

	nsurl *url;
	html_css_fetcher_توکی *توکی;

	bool aborted;
	bool locked;

	بنية html_css_fetcher_context *r_بل, *r_prev;
} html_css_fetcher_context;

ساكن uint32_t current_کلی = 0;
ساكن html_css_fetcher_توکی *توکي = عدم;
ساكن html_css_fetcher_context *ring = عدم;

ساكن bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_معلومات(scheme));
	أرجع true;
}

ساكن فراغ html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_معلومات(scheme));
}

ساكن bool html_css_fetcher_can_fetch(ثابت nsurl *url)
{
	أرجع true;
}

ساكن فراغ *html_css_fetcher_تیاری(بنية fetch *پلار_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_معلومات *post_multipart,
		 ثابت حرف **سرلیکونه)
{
	html_css_fetcher_context *ctx;
	lwc_string *لار;
	uint32_t کلی;
	html_css_fetcher_توکی *توکی, *وموندل = عدم;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	لار = nsurl_get_component(url, NSURL_لار);
	/* The path must exist */
	إذا (لار == عدم) {
		أرجع عدم;
	}

	کلی = strtoul(lwc_string_معلومات(لار), عدم, 10);

	lwc_string_unref(لار);

	/* There must be at least one item */
	إذا (توکي == عدم) {
		أرجع عدم;
	}

	توکی = توکي;
	افعل {
		إذا (توکی->کلی == کلی) {
			وموندل = توکی;
			اكسر;
		}

		توکی = توکی->r_بل;
	} طالما (توکی != توکي);

	/* We must have found the item */
	إذا (وموندل == عدم) {
		أرجع عدم;
	}

	ctx = حافظه_صفر_ورکړه(1, حجم(*ctx));
	إذا (ctx == عدم)
		أرجع عدم;

	ctx->پلار_fetch = پلار_fetch;
	ctx->url = nsurl_ref(url);
	ctx->توکی = وموندل;

	RING_ورننباسه(ring, ctx);

	أرجع ctx;
}

ساكن bool html_css_fetcher_پیل(فراغ *ctx)
{
	أرجع true;
}

ساكن فراغ html_css_fetcher_free(فراغ *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	إذا (c->توکی != عدم) {
		nsurl_unref(c->توکی->بنسټ_url);
		dom_string_unref(c->توکی->معلومات);
		RING_REMOVE(توکي, c->توکی);
		حرر(c->توکی);
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

ساكن فراغ html_css_fetcher_send_callback(ثابت fetch_پیغام *پیغام,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(پیغام, c->پلار_fetch);
	c->locked = false;
}

ساكن فراغ html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_پیغام پیغام;
	html_css_fetcher_context *c, *بل;

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
			بل = c->r_بل;
			استمر;
		}

		/* Only process non-aborted fetches */
		إذا (c->aborted) {
			/* Nothing to do */
			أكد(c->locked == false);
		} وإلا إذا (c->توکی != عدم) {
			حرف سرلیک[4096];

			fetch_سېټ_http_کوډ(c->پلار_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(سرلیک, حجم سرلیک,
				"Content-Type: text/css; charset=utf-8");
			پیغام.type = FETCH_سرلیک;
			پیغام.معلومات.سرلیک_or_معلومات.buf = (ثابت uint8_t *) سرلیک;
			پیغام.معلومات.سرلیک_or_معلومات.len = مزي_اوږدوالی(سرلیک);
			html_css_fetcher_send_callback(&پیغام, c);

			إذا (c->aborted == false) {
				snprintf(سرلیک, حجم سرلیک,
					"Content-Length: %"PRIsizet,
					dom_string_byte_اوږدتیا(c->توکی->معلومات));
				پیغام.type = FETCH_سرلیک;
				پیغام.معلومات.سرلیک_or_معلومات.buf =
						(ثابت uint8_t *) سرلیک;
				پیغام.معلومات.سرلیک_or_معلومات.len = مزي_اوږدوالی(سرلیک);
				html_css_fetcher_send_callback(&پیغام, c);
			}

			إذا (c->aborted == false) {
				snprintf(سرلیک, حجم سرلیک,
					"X-NS-Base: %.*s",
					(صحيح) nsurl_اوږدتیا(c->توکی->بنسټ_url),
					nsurl_access(c->توکی->بنسټ_url));
				پیغام.type = FETCH_سرلیک;
				پیغام.معلومات.سرلیک_or_معلومات.buf =
						(ثابت uint8_t *) سرلیک;
				پیغام.معلومات.سرلیک_or_معلومات.len = مزي_اوږدوالی(سرلیک);
				html_css_fetcher_send_callback(&پیغام, c);
			}

			إذا (c->aborted == false) {
				پیغام.type = FETCH_معلومات;
				پیغام.معلومات.سرلیک_or_معلومات.buf =
						(ثابت uint8_t *)
						dom_string_معلومات(c->توکی->معلومات);
				پیغام.معلومات.سرلیک_or_معلومات.len =
					dom_string_byte_اوږدتیا(c->توکی->معلومات);
				html_css_fetcher_send_callback(&پیغام, c);
			}

			إذا (c->aborted == false) {
				پیغام.type = FETCH_پای_ته_رسېدلی;
				html_css_fetcher_send_callback(&پیغام, c);
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
		بل = c->r_بل;

		fetch_remove_from_queues(c->پلار_fetch);
		fetch_free(c->پلار_fetch);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} طالما ( (c = بل) != ring && ring != عدم);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_ثبتوونکی(فراغ)
{
	ثابت بنية fetcher_operation_جدول html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.تیاری = html_css_fetcher_تیاری,
		.پیل = html_css_fetcher_پیل,
		.abort = html_css_fetcher_abort,
		.حرر = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	أرجع fetcher_جمع_کړه(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_جمع_کړه_توکی(dom_string *معلومات, nsurl *بنسټ_url, uint32_t *کلی)
{
	html_css_fetcher_توکی *توکی = حافظه_ورکړه(حجم(*توکی));

	إذا (توکی == عدم) {
		أرجع NSERROR_NOMEM;
	}

	*کلی = توکی->کلی = current_کلی++;
	توکی->معلومات = dom_string_ref(معلومات);
	توکی->بنسټ_url = nsurl_ref(بنسټ_url);

	RING_ورننباسه(توکي, توکی);

	أرجع NSERROR_OK;
}
