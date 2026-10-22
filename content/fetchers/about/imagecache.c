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

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>

#అన్తర్భావ "netsurf/types.h"

#అన్తర్భావ "image/image_cache.h"

#అన్తర్భావ "private.h"
#అన్తర్భావ "imagecache.h"

/* exported interface documented in about/imagecache.h */
bool fetch_about_imagecache_handler(నిర్మాణం fetch_about_context *ctx)
{
	అక్షరం బఫరు[2048]; /* output buffer */
	పూర్ణాంకం సంకేతం_2 = 200;
	పూర్ణాంకం slen;
	గుర్తులేని పూర్ణాంకం cent_loop = 0;
	పూర్ణాంకం elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_సమితి_http_సంకేతం(ctx, సంకేతం_2);

	/* content type */
	ఒకవేళ (fetch_about_send_శీర్షిక(ctx, "Content-Type: text/html"))
		వెళ్ళు fetch_about_imagecache_handler_aborted;

	/* page head */
	res = fetch_about_ssenddataf(ctx,
		"<html>\n<head>\n"
		"<title>Image Cache Status</title>\n"
		"<link rel=\"stylesheet\" type=\"text/css\" "
		"href=\"resource:internal.css\">\n"
		"</head>\n"
		"<body id =\"cachelist\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
		"<h1 class=\"ns-border\">Image Cache Status</h1>\n");
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_imagecache_handler_aborted;
	}

	/* image cache summary */
	slen = image_cache_snsummaryf(బఫరు, పరిమాణం(బఫరు),
		"<p>Configured limit of %a hysteresis of %b</p>\n"
		"<p>Total bitmap size in use %c (in %d)</p>\n"
		"<p>Age %es</p>\n"
		"<p>Peak size %f (in %g)</p>\n"
		"<p>Peak image count %h (size %i)</p>\n"
		"<p>Cache total/hit/miss/fail (counts) %j/%k/%l/%m "
		"(%pj%%/%pk%%/%pl%%/%pm%%)"
		"<img width=200 height=100 src=\"about:chart?type=pie&width=200&height=100&labels=hit,miss,fail&values=%k,%l,%m\" />"
		"</p>\n");
	ఒకవేళ (slen >= (పూర్ణాంకం) (పరిమాణం(బఫరు))) {
		వెళ్ళు fetch_about_imagecache_handler_aborted; /*అధికప్రవాహం*/
	}

	res = fetch_about_senddata(ctx, (స్థిరరాశి uint8_t *)బఫరు, slen);
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_imagecache_handler_aborted;
	}

	/* image cache summary */
	slen = image_cache_snsummaryf(బఫరు, పరిమాణం(బఫరు),
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
	ఒకవేళ (slen >= (పూర్ణాంకం) (పరిమాణం(బఫరు))) {
		వెళ్ళు fetch_about_imagecache_handler_aborted; /*అధికప్రవాహం*/
	}

	res = fetch_about_senddata(ctx, (స్థిరరాశి uint8_t *)బఫరు, slen);
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_imagecache_handler_aborted;
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
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_imagecache_handler_aborted;
	}

	slen = 0;
	చేయి {
		ఒకవేళ (even) {
			elen = image_cache_snentryf(బఫరు + slen,
						   పరిమాణం బఫరు - slen,
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
		} లేకపోతే {
			elen = image_cache_snentryf(బఫరు + slen,
						   పరిమాణం బఫరు - slen,
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
		ఒకవేళ (elen <= 0)
			ఆపు; /* last option */

		ఒకవేళ (elen >= (పూర్ణాంకం) (పరిమాణం బఫరు - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx,
						   (స్థిరరాశి uint8_t *)బఫరు,
						   slen);
			ఒకవేళ (res != NSERROR_OK) {
				వెళ్ళు fetch_about_imagecache_handler_aborted;
			}

			slen = 0;
		} లేకపోతే {
			/* normal addition */
			slen += elen;
			cent_loop++;
			even = !even;
		}
	} అయితే (elen > 0);

	slen += snprintf(బఫరు + slen, పరిమాణం బఫరు - slen,
			 "</p>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (స్థిరరాశి uint8_t *)బఫరు, slen);
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_imagecache_handler_aborted;
	}

	fetch_about_send_ముగిసిన(ctx);

	తిరిగి true;

fetch_about_imagecache_handler_aborted:
	తిరిగి false;
}
