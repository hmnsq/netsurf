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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>

#ಅನ್ತರ್ಭಾವ "netsurf/types.h"

#ಅನ್ತರ್ಭಾವ "image/image_cache.h"

#ಅನ್ತರ್ಭಾವ "private.h"
#ಅನ್ತರ್ಭಾವ "imagecache.h"

/* exported interface documented in about/imagecache.h */
bool fetch_about_imagecache_handler(ಸಂರಚನಾ fetch_about_context *ctx)
{
	ವರ್ಣ buffer[2048]; /* output buffer */
	ಪೂರ್ಣಾಙ್ಕ code = 200;
	ಪೂರ್ಣಾಙ್ಕ slen;
	unsigned ಪೂರ್ಣಾಙ್ಕ cent_loop = 0;
	ಪೂರ್ಣಾಙ್ಕ elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	ಯದಿ (fetch_about_send_header(ctx, "Content-Type: text/html"))
		ಗಚ್ಛ fetch_about_imagecache_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
		"<html>\n<head>\n"
		"<title>Image Cache Status</title>\n"
		"<link rel=\"stylesheet\" type=\"text/css\" "
		"href=\"resource:internal.css\">\n"
		"</head>\n"
		"<body id =\"cachelist\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
		"<h1 class=\"ns-border\">Image Cache Status</h1>\n");
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_imagecache_handler_aborted;
	}

	/* image cache summary */
	slen = image_cache_snsummaryf(buffer, ಪರಿಮಾಣ(buffer),
		"<p>Configured limit of %a hysteresis of %b</p>\n"
		"<p>Total bitmap size in use %c (in %d)</p>\n"
		"<p>Age %es</p>\n"
		"<p>Peak size %f (in %g)</p>\n"
		"<p>Peak image count %h (size %i)</p>\n"
		"<p>Cache total/hit/miss/fail (counts) %j/%k/%l/%m "
		"(%pj%%/%pk%%/%pl%%/%pm%%)"
		"<img width=200 height=100 src=\"about:chart?type=pie&width=200&height=100&labels=hit,miss,fail&values=%k,%l,%m\" />"
		"</p>\n");
	ಯದಿ (slen >= (ಪೂರ್ಣಾಙ್ಕ) (ಪರಿಮಾಣ(buffer))) {
		ಗಚ್ಛ fetch_about_imagecache_handler_aborted; /* overflow */
	}

	res = fetch_about_senddata(ctx, (ಧ್ರುವ uint8_t *)buffer, slen);
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_imagecache_handler_aborted;
	}

	/* image cache summary */
	slen = image_cache_snsummaryf(buffer, ಪರಿಮಾಣ(buffer),
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
	ಯದಿ (slen >= (ಪೂರ್ಣಾಙ್ಕ) (ಪರಿಮಾಣ(buffer))) {
		ಗಚ್ಛ fetch_about_imagecache_handler_aborted; /* overflow */
	}

	res = fetch_about_senddata(ctx, (ಧ್ರುವ uint8_t *)buffer, slen);
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_imagecache_handler_aborted;
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
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_imagecache_handler_aborted;
	}

	slen = 0;
	ಕುರು {
		ಯದಿ (even) {
			elen = image_cache_snentryf(buffer + slen,
						   ಪರಿಮಾಣ buffer - slen,
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
		} ಅನ್ಯಥಾ {
			elen = image_cache_snentryf(buffer + slen,
						   ಪರಿಮಾಣ buffer - slen,
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
		ಯದಿ (elen <= 0)
			ಭಙ್ಗ; /* last option */

		ಯದಿ (elen >= (ಪೂರ್ಣಾಙ್ಕ) (ಪರಿಮಾಣ buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx,
						   (ಧ್ರುವ uint8_t *)buffer,
						   slen);
			ಯದಿ (res != NSERROR_OK) {
				ಗಚ್ಛ fetch_about_imagecache_handler_aborted;
			}

			slen = 0;
		} ಅನ್ಯಥಾ {
			/* normal addition */
			slen += elen;
			cent_loop++;
			even = !even;
		}
	} ಯಾವತ್ (elen > 0);

	slen += snprintf(buffer + slen, ಪರಿಮಾಣ buffer - slen,
			 "</p>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (ಧ್ರುವ uint8_t *)buffer, slen);
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_imagecache_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	ಪ್ರತ್ಯಾಗಮ true;

fetch_about_imagecache_handler_aborted:
	ಪ್ರತ್ಯಾಗಮ false;
}
