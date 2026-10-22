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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>

#अन्तर्भाव "netsurf/types.h"

#अन्तर्भाव "image/image_cache.h"

#अन्तर्भाव "private.h"
#अन्तर्भाव "imagecache.h"

/* exported interface documented in about/imagecache.h */
bool fetch_about_imagecache_handler(संरचना fetch_about_context *ctx)
{
	वर्ण बफर[2048]; /* output buffer */
	पूर्णाङ्क संहिता = 200;
	पूर्णाङ्क slen;
	अचिह्नित पूर्णाङ्क cent_loop = 0;
	पूर्णाङ्क elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_समुच्चयः_http_संहिता(ctx, संहिता);

	/* content type */
	यदि (fetch_about_send_शीर्षकम्(ctx, "Content-Type: text/html"))
		गच्छ fetch_about_imagecache_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
		"<html>\n<head>\n"
		"<title>Image Cache Status</title>\n"
		"<link rel=\"stylesheet\" type=\"text/css\" "
		"href=\"resource:internal.css\">\n"
		"</head>\n"
		"<body id =\"cachelist\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
		"<h1 class=\"ns-border\">Image Cache Status</h1>\n");
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_imagecache_handler_aborted;
	}

	/* image cache summary */
	slen = image_cache_snsummaryf(बफर, परिमाण(बफर),
		"<p>Configured limit of %a hysteresis of %b</p>\n"
		"<p>Total bitmap size in use %c (in %d)</p>\n"
		"<p>Age %es</p>\n"
		"<p>Peak size %f (in %g)</p>\n"
		"<p>Peak image count %h (size %i)</p>\n"
		"<p>Cache total/hit/miss/fail (counts) %j/%k/%l/%m "
		"(%pj%%/%pk%%/%pl%%/%pm%%)"
		"<img width=200 height=100 src=\"about:chart?type=pie&width=200&height=100&labels=hit,miss,fail&values=%k,%l,%m\" />"
		"</p>\n");
	यदि (slen >= (पूर्णाङ्क) (परिमाण(बफर))) {
		गच्छ fetch_about_imagecache_handler_aborted; /*अतिप्रवाहः*/
	}

	res = fetch_about_senddata(ctx, (ध्रुव uint8_t *)बफर, slen);
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_imagecache_handler_aborted;
	}

	/* image cache summary */
	slen = image_cache_snsummaryf(बफर, परिमाण(बफर),
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
	यदि (slen >= (पूर्णाङ्क) (परिमाण(बफर))) {
		गच्छ fetch_about_imagecache_handler_aborted; /*अतिप्रवाहः*/
	}

	res = fetch_about_senddata(ctx, (ध्रुव uint8_t *)बफर, slen);
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_imagecache_handler_aborted;
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
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_imagecache_handler_aborted;
	}

	slen = 0;
	कुरु {
		यदि (even) {
			elen = image_cache_snentryf(बफर + slen,
						   परिमाण बफर - slen,
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
		} अन्यथा {
			elen = image_cache_snentryf(बफर + slen,
						   परिमाण बफर - slen,
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
		यदि (elen <= 0)
			भङ्ग; /* last option */

		यदि (elen >= (पूर्णाङ्क) (परिमाण बफर - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx,
						   (ध्रुव uint8_t *)बफर,
						   slen);
			यदि (res != NSERROR_OK) {
				गच्छ fetch_about_imagecache_handler_aborted;
			}

			slen = 0;
		} अन्यथा {
			/* normal addition */
			slen += elen;
			cent_loop++;
			even = !even;
		}
	} यावत् (elen > 0);

	slen += snprintf(बफर + slen, परिमाण बफर - slen,
			 "</p>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (ध्रुव uint8_t *)बफर, slen);
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_imagecache_handler_aborted;
	}

	fetch_about_send_परिसमाप्तम्(ctx);

	प्रत्यागम true;

fetch_about_imagecache_handler_aborted:
	प्रत्यागम false;
}
