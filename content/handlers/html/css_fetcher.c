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

#شامل <assert.h>
#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <dom/dom.h>
#شامل <libwapcaplet/libwapcaplet.h>

#شامل "netsurf/inttypes.h"
#شامل "utils/config.h"
#شامل "utils/corestrings.h"
#شامل "utils/log.h"
#شامل "utils/ring.h"
#شامل "utils/nsurl.h"
#شامل "utils/utils.h"
#شامل "content/fetch.h"
#شامل "content/fetchers.h"

#شامل "html/private.h"

قسمتعریف ساخت html_css_fetcher_شے {
	uint32_t کلید;
	dom_string *ڈیٹا;
	nsurl *بنیاد_url;

	ساخت html_css_fetcher_شے *r_اگلا, *r_پچھلا;
} html_css_fetcher_شے;

قسمتعریف ساخت html_css_fetcher_context {
	ساخت fetch *والد_fetch;

	nsurl *url;
	html_css_fetcher_شے *شے;

	bool aborted;
	bool locked;

	ساخت html_css_fetcher_context *r_اگلا, *r_پچھلا;
} html_css_fetcher_context;

ساکن uint32_t current_کلید = 0;
ساکن html_css_fetcher_شے *اشیاء = تہی;
ساکن html_css_fetcher_context *ring = تہی;

ساکن bool html_css_fetcher_initialise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_initialise called for %s",
	      lwc_string_ڈیٹا(scheme));
	واپس true;
}

ساکن خالی html_css_fetcher_finalise(lwc_string *scheme)
{
	NSLOG(netsurf, INFO, "html_css_fetcher_finalise called for %s",
	      lwc_string_ڈیٹا(scheme));
}

ساکن bool html_css_fetcher_can_fetch(ثابت nsurl *url)
{
	واپس true;
}

ساکن خالی *html_css_fetcher_تیاری(ساخت fetch *والد_fetch, nsurl *url,
		 bool only_2xx, bool downgrade_tls, ثابت حرف *post_urlenc,
		 ثابت ساخت fetch_multipart_ڈیٹا *post_multipart,
		 ثابت حرف **سرخیاں)
{
	html_css_fetcher_context *ctx;
	lwc_string *راستہ;
	uint32_t کلید;
	html_css_fetcher_شے *شے, *ملا = تہی;

	/* format of a x-ns-css URL is:
	 *   x-ns-url:<key>
	 * Where key is an unsigned 32bit integer
	 */

	راستہ = nsurl_get_component(url, NSURL_راستہ);
	/* The path must exist */
	اگر (راستہ == تہی) {
		واپس تہی;
	}

	کلید = strtoul(lwc_string_ڈیٹا(راستہ), تہی, 10);

	lwc_string_unref(راستہ);

	/* There must be at least one item */
	اگر (اشیاء == تہی) {
		واپس تہی;
	}

	شے = اشیاء;
	کرو {
		اگر (شے->کلید == کلید) {
			ملا = شے;
			توڑو;
		}

		شے = شے->r_اگلا;
	} جب (شے != اشیاء);

	/* We must have found the item */
	اگر (ملا == تہی) {
		واپس تہی;
	}

	ctx = صفر_حافظہ_دو(1, حجم(*ctx));
	اگر (ctx == تہی)
		واپس تہی;

	ctx->والد_fetch = والد_fetch;
	ctx->url = nsurl_ref(url);
	ctx->شے = ملا;

	RING_داخل_کرو(ring, ctx);

	واپس ctx;
}

ساکن bool html_css_fetcher_آغاز(خالی *ctx)
{
	واپس true;
}

ساکن خالی html_css_fetcher_free(خالی *ctx)
{
	html_css_fetcher_context *c = ctx;

	nsurl_unref(c->url);
	اگر (c->شے != تہی) {
		nsurl_unref(c->شے->بنیاد_url);
		dom_string_unref(c->شے->ڈیٹا);
		RING_REMOVE(اشیاء, c->شے);
		آزاد(c->شے);
	}
	RING_REMOVE(ring, c);
	آزاد(ctx);
}

ساکن خالی html_css_fetcher_abort(خالی *ctx)
{
	html_css_fetcher_context *c = ctx;

	/* To avoid the poll loop having to deal with the fetch context
	 * disappearing from under it, we simply flag the abort here.
	 * The poll loop itself will perform the appropriate cleanup.
	 */
	c->aborted = true;
}

ساکن خالی html_css_fetcher_send_callback(ثابت fetch_پیغام *پیغام,
		html_css_fetcher_context *c)
{
	c->locked = true;
	fetch_send_callback(پیغام, c->والد_fetch);
	c->locked = false;
}

ساکن خالی html_css_fetcher_poll(lwc_string *scheme)
{
	fetch_پیغام پیغام;
	html_css_fetcher_context *c, *اگلا;

	اگر (ring == تہی) واپس;

	/* Iterate over ring, processing each pending fetch */
	c = ring;
	کرو {
		/* Ignore fetches that have been flagged as locked.
		 * This allows safe re-entrant calls to this function.
		 * Re-entrancy can occur if, as a result of a callback,
		 * the interested party causes fetch_poll() to be called
		 * again.
		 */
		اگر (c->locked == true) {
			اگلا = c->r_اگلا;
			جاری;
		}

		/* Only process non-aborted fetches */
		اگر (c->aborted) {
			/* Nothing to do */
			أكد(c->locked == false);
		} ورنہ اگر (c->شے != تہی) {
			حرف سرخی[4096];

			fetch_سیٹ_http_رمز(c->والد_fetch, 200);

			/* Any callback can result in the fetch being aborted.
			 * Therefore, we _must_ check for this after _every_
			 * call to html_css_fetcher_send_callback().
			 */
			snprintf(سرخی, حجم سرخی,
				"Content-Type: text/css; charset=utf-8");
			پیغام.type = FETCH_سرخی;
			پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf = (ثابت uint8_t *) سرخی;
			پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = لڑی_لمبائی(سرخی);
			html_css_fetcher_send_callback(&پیغام, c);

			اگر (c->aborted == false) {
				snprintf(سرخی, حجم سرخی,
					"Content-Length: %"PRIsizet,
					dom_string_byte_لمبائی(c->شے->ڈیٹا));
				پیغام.type = FETCH_سرخی;
				پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf =
						(ثابت uint8_t *) سرخی;
				پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = لڑی_لمبائی(سرخی);
				html_css_fetcher_send_callback(&پیغام, c);
			}

			اگر (c->aborted == false) {
				snprintf(سرخی, حجم سرخی,
					"X-NS-Base: %.*s",
					(صحیح) nsurl_لمبائی(c->شے->بنیاد_url),
					nsurl_access(c->شے->بنیاد_url));
				پیغام.type = FETCH_سرخی;
				پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf =
						(ثابت uint8_t *) سرخی;
				پیغام.ڈیٹا.سرخی_or_ڈیٹا.len = لڑی_لمبائی(سرخی);
				html_css_fetcher_send_callback(&پیغام, c);
			}

			اگر (c->aborted == false) {
				پیغام.type = FETCH_ڈیٹا;
				پیغام.ڈیٹا.سرخی_or_ڈیٹا.buf =
						(ثابت uint8_t *)
						dom_string_ڈیٹا(c->شے->ڈیٹا);
				پیغام.ڈیٹا.سرخی_or_ڈیٹا.len =
					dom_string_byte_لمبائی(c->شے->ڈیٹا);
				html_css_fetcher_send_callback(&پیغام, c);
			}

			اگر (c->aborted == false) {
				پیغام.type = FETCH_ختم_شدہ;
				html_css_fetcher_send_callback(&پیغام, c);
			}
		} ورنہ {
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

		fetch_remove_from_queues(c->والد_fetch);
		fetch_free(c->والد_fetch);

		/* Advance to next ring entry, exiting if we've reached
		 * the start of the ring or the ring has become empty
		 */
	} جب ( (c = اگلا) != ring && ring != تہی);
}

/* exported interface documented in html_internal.h */
nserror html_css_fetcher_رجسٹر(خالی)
{
	ثابت ساخت fetcher_operation_جدول html_css_fetcher_ops = {
		.initialise = html_css_fetcher_initialise,
		.acceptable = html_css_fetcher_can_fetch,
		.تیاری = html_css_fetcher_تیاری,
		.آغاز = html_css_fetcher_آغاز,
		.abort = html_css_fetcher_abort,
		.آزاد = html_css_fetcher_free,
		.poll = html_css_fetcher_poll,
		.finalise = html_css_fetcher_finalise
	};

	واپس fetcher_جوڑو(lwc_string_ref(corestring_lwc_x_ns_css),
			&html_css_fetcher_ops);
}

/* exported interface documented in html_internal.h */
nserror
html_css_fetcher_جوڑو_شے(dom_string *ڈیٹا, nsurl *بنیاد_url, uint32_t *کلید)
{
	html_css_fetcher_شے *شے = حافظہ_دو(حجم(*شے));

	اگر (شے == تہی) {
		واپس NSERROR_NOMEM;
	}

	*کلید = شے->کلید = current_کلید++;
	شے->ڈیٹا = dom_string_ref(ڈیٹا);
	شے->بنیاد_url = nsurl_ref(بنیاد_url);

	RING_داخل_کرو(اشیاء, شے);

	واپس NSERROR_OK;
}
