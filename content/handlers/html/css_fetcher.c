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

تعريفنوع بنية html_css_fetcher_شے {
	uint32_t کلید;
	dom_string *انکڑے;
	nsurl *ادھار_url;

	بنية html_css_fetcher_شے *r_اگلا, *r_prev;
} html_css_fetcher_شے;

تعريفنوع بنية html_css_fetcher_context {
	بنية fetch *ماپا_fetch;

	nsurl *url;
	html_css_fetcher_شے *شے;

	bool aborted;
	bool locked;

	بنية html_css_fetcher_context *r_اگلا, *r_prev;
} html_css_fetcher_context;

ساكن uint32_t current_کلید = 0;
ساكن html_css_fetcher_شے *اشیاء = عدم;
ساكن html_css_fetcher_context *ring = عدم;

ساكن bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_انکڑے(scheme));
	أرجع true;
}

ساكن فراغ html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_انکڑے(scheme));
}

ساكن bool html_css_fetcher_can_fetch(ثابت nsurl *url)
{
	أرجع true;
}

ساكن فراغ *html_css_fetcher_تیاری(بنية fetch *ماپا_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ثابت حرف *post_urlenc,
		 ثابت بنية fetch_multipart_انکڑے *post_multipart,
		 ثابت حرف **سرخیاں)
{
	html_css_fetcher_context *ctx;
	lwc_string *رستہ;
	uint32_t کلید;
	html_css_fetcher_شے *شے, *لبھیا = عدم;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	رستہ = nsurl_get_component(url, NSURL_رستہ);
	/* The path must exist */
	إذا (رستہ == عدم) {
		أرجع عدم;
	}

	کلید = strtoul(lwc_string_انکڑے(رستہ), عدم, 10);

	lwc_string_unref(رستہ);

	/* There must be at least one item */
	إذا (اشیاء == عدم) {
		أرجع عدم;
	}

	شے = اشیاء;
	افعل {
		إذا (شے->کلید == کلید) {
			لبھیا = شے;
			اكسر;
		}

		شے = شے->r_اگلا;
	} طالما (شے != اشیاء);

	/* We must have found the item */
	إذا (لبھیا == عدم) {
		أرجع عدم;
	}

	ctx = صفر_حافظہ_دیو(1, حجم(*ctx));
	إذا (ctx == عدم)
		أرجع عدم;

	ctx->ماپا_fetch = ماپا_fetch;
	ctx->url = nsurl_ref(url);
	ctx->شے = لبھیا;

	RING_داخل_کرو(ring, ctx);

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
	إذا (c->شے != عدم) {
		nsurl_unref(c->شے->ادھار_url);
		dom_string_unref(c->شے->انکڑے);
		RING_REMOVE(اشیاء, c->شے);
		حرر(c->شے);
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
	fetch_send_callback(پیغام, c->ماپا_fetch);
	c->locked = false;
}

ساكن فراغ html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_پیغام پیغام;
	html_css_fetcher_context *c, *اگلا;

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
			اگلا = c->r_اگلا;
			استمر;
		}

		/* Only process non-aborted fetches */
		إذا (c->aborted) {
			/* Nothing to do */
			أكد(c->locked == false);
		} وإلا إذا (c->شے != عدم) {
			حرف سرخی[4096];

			fetch_سیٹ_http_کوڈ(c->ماپا_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(سرخی, حجم سرخی,
				"Content-Type: text/css; charset=utf-8");
			پیغام.type = FETCH_سرخی;
			پیغام.انکڑے.سرخی_or_انکڑے.buf = (ثابت uint8_t *) سرخی;
			پیغام.انکڑے.سرخی_or_انکڑے.len = لڑی_لمبائی(سرخی);
			html_css_fetcher_send_callback(&پیغام, c);

			إذا (c->aborted == false) {
				snprintf(سرخی, حجم سرخی,
					"Content-Length: %"PRIsizet,
					dom_string_byte_لمائی(c->شے->انکڑے));
				پیغام.type = FETCH_سرخی;
				پیغام.انکڑے.سرخی_or_انکڑے.buf =
						(ثابت uint8_t *) سرخی;
				پیغام.انکڑے.سرخی_or_انکڑے.len = لڑی_لمبائی(سرخی);
				html_css_fetcher_send_callback(&پیغام, c);
			}

			إذا (c->aborted == false) {
				snprintf(سرخی, حجم سرخی,
					"X-NS-Base: %.*s",
					(صحيح) nsurl_لمائی(c->شے->ادھار_url),
					nsurl_access(c->شے->ادھار_url));
				پیغام.type = FETCH_سرخی;
				پیغام.انکڑے.سرخی_or_انکڑے.buf =
						(ثابت uint8_t *) سرخی;
				پیغام.انکڑے.سرخی_or_انکڑے.len = لڑی_لمبائی(سرخی);
				html_css_fetcher_send_callback(&پیغام, c);
			}

			إذا (c->aborted == false) {
				پیغام.type = FETCH_انکڑے;
				پیغام.انکڑے.سرخی_or_انکڑے.buf =
						(ثابت uint8_t *)
						dom_string_انکڑے(c->شے->انکڑے);
				پیغام.انکڑے.سرخی_or_انکڑے.len =
					dom_string_byte_لمائی(c->شے->انکڑے);
				html_css_fetcher_send_callback(&پیغام, c);
			}

			إذا (c->aborted == false) {
				پیغام.type = FETCH_مکمل;
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
		اگلا = c->r_اگلا;

		fetch_remove_from_queues(c->ماپا_fetch);
		fetch_free(c->ماپا_fetch);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} طالما ( (c = اگلا) != ring && ring != عدم);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_رجسٹر(فراغ)
{
	ثابت بنية fetcher_operation_جدول html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.تیاری = html_css_fetcher_تیاری,
		.شروع = html_css_fetcher_شروع,
		.abort = html_css_fetcher_abort,
		.حرر = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	أرجع fetcher_جوڑو(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_جوڑو_شے(dom_string *انکڑے, nsurl *ادھار_url, uint32_t *کلید)
{
	html_css_fetcher_شے *شے = حافظہ_دیو(حجم(*شے));

	إذا (شے == عدم) {
		أرجع NSERROR_NOMEM;
	}

	*کلید = شے->کلید = current_کلید++;
	شے->انکڑے = dom_string_ref(انکڑے);
	شے->ادھار_url = nsurl_ref(ادھار_url);

	RING_داخل_کرو(اشیاء, شے);

	أرجع NSERROR_OK;
}
