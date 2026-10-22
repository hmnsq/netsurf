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

تعريفنوع بنية html_css_fetcher_شي {
	uint32_t ڪنجي;
	dom_string *معلومات;
	nsurl *بنياد_url;

	بنية html_css_fetcher_شي *r_ايندڙ, *r_prev;
} html_css_fetcher_شي;

تعريفنوع بنية html_css_fetcher_context {
	بنية fetch *والد_fetch;

	nsurl *url;
	html_css_fetcher_شي *شي;

	bool aborted;
	bool locked;

	بنية html_css_fetcher_context *r_ايندڙ, *r_prev;
} html_css_fetcher_context;

ساكن uint32_t current_ڪنجي = 0;
ساكن html_css_fetcher_شي *شيون = عدم;
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

ساكن فراغ *html_css_fetcher_تياري(بنية fetch *والد_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_معلومات *post_multipart,
		 ثابت حرف **سرا)
{
	html_css_fetcher_context *ctx;
	lwc_string *رستو;
	uint32_t ڪنجي;
	html_css_fetcher_شي *شي, *مليو = عدم;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	رستو = nsurl_get_component(url, NSURL_رستو);
	/* The path must exist */
	إذا (رستو == عدم) {
		أرجع عدم;
	}

	ڪنجي = strtoul(lwc_string_معلومات(رستو), عدم, 10);

	lwc_string_unref(رستو);

	/* There must be at least one item */
	إذا (شيون == عدم) {
		أرجع عدم;
	}

	شي = شيون;
	افعل {
		إذا (شي->ڪنجي == ڪنجي) {
			مليو = شي;
			اكسر;
		}

		شي = شي->r_ايندڙ;
	} طالما (شي != شيون);

	/* We must have found the item */
	إذا (مليو == عدم) {
		أرجع عدم;
	}

	ctx = صفر_يادگيري_ڏيو(1, حجم(*ctx));
	إذا (ctx == عدم)
		أرجع عدم;

	ctx->والد_fetch = والد_fetch;
	ctx->url = nsurl_ref(url);
	ctx->شي = مليو;

	RING_وجهو(ring, ctx);

	أرجع ctx;
}

ساكن bool html_css_fetcher_شروعات(فراغ *ctx)
{
	أرجع true;
}

ساكن فراغ html_css_fetcher_free(فراغ *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	إذا (c->شي != عدم) {
		nsurl_unref(c->شي->بنياد_url);
		dom_string_unref(c->شي->معلومات);
		RING_REMOVE(شيون, c->شي);
		حرر(c->شي);
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

ساكن فراغ html_css_fetcher_send_callback(ثابت fetch_نياپو *نياپو,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(نياپو, c->والد_fetch);
	c->locked = false;
}

ساكن فراغ html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_نياپو نياپو;
	html_css_fetcher_context *c, *ايندڙ;

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
			ايندڙ = c->r_ايندڙ;
			استمر;
		}

		/* Only process non-aborted fetches */
		إذا (c->aborted) {
			/* Nothing to do */
			أكد(c->locked == false);
		} وإلا إذا (c->شي != عدم) {
			حرف سرو[4096];

			fetch_سيٽ_http_ڪوڊ(c->والد_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(سرو, حجم سرو,
				"Content-Type: text/css; charset=utf-8");
			نياپو.type = FETCH_سرو;
			نياپو.معلومات.سرو_or_معلومات.buf = (ثابت uint8_t *) سرو;
			نياپو.معلومات.سرو_or_معلومات.len = لڙي_ڊيگهه(سرو);
			html_css_fetcher_send_callback(&نياپو, c);

			إذا (c->aborted == false) {
				snprintf(سرو, حجم سرو,
					"Content-Length: %"PRIsizet,
					dom_string_byte_ڊيگهائي(c->شي->معلومات));
				نياپو.type = FETCH_سرو;
				نياپو.معلومات.سرو_or_معلومات.buf =
						(ثابت uint8_t *) سرو;
				نياپو.معلومات.سرو_or_معلومات.len = لڙي_ڊيگهه(سرو);
				html_css_fetcher_send_callback(&نياپو, c);
			}

			إذا (c->aborted == false) {
				snprintf(سرو, حجم سرو,
					"X-NS-Base: %.*s",
					(صحيح) nsurl_ڊيگهائي(c->شي->بنياد_url),
					nsurl_access(c->شي->بنياد_url));
				نياپو.type = FETCH_سرو;
				نياپو.معلومات.سرو_or_معلومات.buf =
						(ثابت uint8_t *) سرو;
				نياپو.معلومات.سرو_or_معلومات.len = لڙي_ڊيگهه(سرو);
				html_css_fetcher_send_callback(&نياپو, c);
			}

			إذا (c->aborted == false) {
				نياپو.type = FETCH_معلومات;
				نياپو.معلومات.سرو_or_معلومات.buf =
						(ثابت uint8_t *)
						dom_string_معلومات(c->شي->معلومات);
				نياپو.معلومات.سرو_or_معلومات.len =
					dom_string_byte_ڊيگهائي(c->شي->معلومات);
				html_css_fetcher_send_callback(&نياپو, c);
			}

			إذا (c->aborted == false) {
				نياپو.type = FETCH_ختم;
				html_css_fetcher_send_callback(&نياپو, c);
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
		ايندڙ = c->r_ايندڙ;

		fetch_remove_from_queues(c->والد_fetch);
		fetch_free(c->والد_fetch);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} طالما ( (c = ايندڙ) != ring && ring != عدم);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_رجسٽر(فراغ)
{
	ثابت بنية fetcher_operation_جدول html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.تياري = html_css_fetcher_تياري,
		.شروعات = html_css_fetcher_شروعات,
		.abort = html_css_fetcher_abort,
		.حرر = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	أرجع fetcher_جوڙيو(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_جوڙيو_شي(dom_string *معلومات, nsurl *بنياد_url, uint32_t *ڪنجي)
{
	html_css_fetcher_شي *شي = يادگيري_ڏيو(حجم(*شي));

	إذا (شي == عدم) {
		أرجع NSERROR_NOMEM;
	}

	*ڪنجي = شي->ڪنجي = current_ڪنجي++;
	شي->معلومات = dom_string_ref(معلومات);
	شي->بنياد_url = nsurl_ref(بنياد_url);

	RING_وجهو(شيون, شي);

	أرجع NSERROR_OK;
}
