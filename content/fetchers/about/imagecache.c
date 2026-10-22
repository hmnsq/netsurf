/*
 * Copyright 2020 Vincent Sanders <vince@netsurf-browser.org>
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
 * content generator for the about scheme imagecache page
 */

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/types.h"

#ଅନ୍ତର୍ଭାଵ "image/image_cache.h"

#ଅନ୍ତର୍ଭାଵ "private.h"
#ଅନ୍ତର୍ଭାଵ "imagecache.h"

/* exported interface documented in about/imagecache.h */
bool fetch_about_imagecache_handler(ସଂରଚନା fetch_about_context *ctx)
{
	ଅକ୍ଷର buffer[2048]; /* output buffer */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା code = 200;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା slen;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cent_loop = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	ଯଦି (fetch_about_send_header(ctx, "Content-Type: text/html"))
		ଯାଅ fetch_about_imagecache_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
		"<html>\n<head>\n"
		"<title>Image Cache Status</title>\n"
		"<link rel=\"stylesheet\" type=\"text/css\" "
		"href=\"resource:internal.css\">\n"
		"</head>\n"
		"<body id =\"cachelist\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
		"<h1 class=\"ns-border\">Image Cache Status</h1>\n");
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_imagecache_handler_aborted;
	}

	/* image cache summary */
	slen = image_cache_snsummaryf(buffer, ଆକାର(buffer),
		"<p>Configured limit of %a hysteresis of %b</p>\n"
		"<p>Total bitmap size in use %c (in %d)</p>\n"
		"<p>Age %es</p>\n"
		"<p>Peak size %f (in %g)</p>\n"
		"<p>Peak image count %h (size %i)</p>\n"
		"<p>Cache total/hit/miss/fail (counts) %j/%k/%l/%m "
		"(%pj%%/%pk%%/%pl%%/%pm%%)"
		"<img width=200 height=100 src=\"about:chart?type=pie&width=200&height=100&labels=hit,miss,fail&values=%k,%l,%m\" />"
		"</p>\n");
	ଯଦି (slen >= (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (ଆକାର(buffer))) {
		ଯାଅ fetch_about_imagecache_handler_aborted; /* overflow */
	}

	res = fetch_about_senddata(ctx, (ଅଚଳ uint8_t *)buffer, slen);
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_imagecache_handler_aborted;
	}

	/* image cache summary */
	slen = image_cache_snsummaryf(buffer, ଆକାର(buffer),
		"<p>Cache total/hit/miss/fail (size) %n/%o/%q/%r "
				"(%pn%%/%po%%/%pq%%/%pr%%)"
		"<img width=200 height=100 src=\"about:chart?type=pie&width=200&height=100&labels=hit,miss,fail&values=%o,%q,%r\" /></p>\n"
		"<p>Total images never rendered: %s "
				"(includes %t that were converted)</p>\n"
		"<p>Total number of excessive conversions: %u "
				"(from %v images converted more than once)"
				"</p>\n"
		"<p>Bitmap of size %w had most (%x) conversions</p>\n"
		"<h2 class=\"ns-border\">Current contents</h2>\n");
	ଯଦି (slen >= (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (ଆକାର(buffer))) {
		ଯାଅ fetch_about_imagecache_handler_aborted; /* overflow */
	}

	res = fetch_about_senddata(ctx, (ଅଚଳ uint8_t *)buffer, slen);
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_imagecache_handler_aborted;
	}

	/* image cache entry table */
	res = fetch_about_ssenddataf(ctx, "<p class=\"imagecachelist\">\n"
			"<strong>"
			"<span>Entry</span>"
			"<span>Content Key</span>"
			"<span>Redraw Count</span>"
			"<span>Conversion Count</span>"
			"<span>Last Redraw</span>"
			"<span>Bitmap Age</span>"
			"<span>Bitmap Size</span>"
			"<span>Source</span>"
			"</strong>\n");
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_imagecache_handler_aborted;
	}

	slen = 0;
	କର {
		ଯଦି (even) {
			elen = image_cache_snentryf(buffer + slen,
						   ଆକାର buffer - slen,
					cent_loop,
					"<a href=\"%U\">"
					"<span class=\"ns-border\">%e</span>"
					"<span class=\"ns-border\">%k</span>"
					"<span class=\"ns-border\">%r</span>"
					"<span class=\"ns-border\">%c</span>"
					"<span class=\"ns-border\">%a</span>"
					"<span class=\"ns-border\">%g</span>"
					"<span class=\"ns-border\">%s</span>"
					"<span class=\"ns-border\">%o</span>"
					"</a>\n");
		} ଅନ୍ୟଥା {
			elen = image_cache_snentryf(buffer + slen,
						   ଆକାର buffer - slen,
					cent_loop,
					"<a class=\"ns-odd-bg\" href=\"%U\">"
					"<span class=\"ns-border\">%e</span>"
					"<span class=\"ns-border\">%k</span>"
					"<span class=\"ns-border\">%r</span>"
					"<span class=\"ns-border\">%c</span>"
					"<span class=\"ns-border\">%a</span>"
					"<span class=\"ns-border\">%g</span>"
					"<span class=\"ns-border\">%s</span>"
					"<span class=\"ns-border\">%o</span>"
					"</a>\n");
		}
		ଯଦି (elen <= 0)
			ଭାଙ୍ଗ; /* last option */

		ଯଦି (elen >= (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (ଆକାର buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx,
						   (ଅଚଳ uint8_t *)buffer,
						   slen);
			ଯଦି (res != NSERROR_OK) {
				ଯାଅ fetch_about_imagecache_handler_aborted;
			}

			slen = 0;
		} ଅନ୍ୟଥା {
			/* normal addition */
			slen += elen;
			cent_loop++;
			even = !even;
		}
	} ଯେତେବେଳେ (elen > 0);

	slen += snprintf(buffer + slen, ଆକାର buffer - slen,
			 "</p>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (ଅଚଳ uint8_t *)buffer, slen);
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_imagecache_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	ଫେରନ୍ତୁ true;

fetch_about_imagecache_handler_aborted:
	ଫେରନ୍ତୁ false;
}
