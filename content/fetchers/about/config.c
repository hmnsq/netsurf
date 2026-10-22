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
 * content generator for the about scheme blank page
 */

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/types.h"
#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"

#ଅନ୍ତର୍ଭାଵ "private.h"
#ଅନ୍ତର୍ଭାଵ "config.h"

/**
 * Handler to generate about scheme config page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_config_handler(ସଂରଚନା fetch_about_context *ctx)
{
	ଅକ୍ଷର buffer[1024];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା slen = 0;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_loop = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	ଯଦି (fetch_about_send_header(ctx, "Content-Type: text/html")) {
		ଯାଅ fetch_about_config_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Config</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body "
				"id =\"configlist\" "
				"class=\"ns-even-bg ns-even-fg ns-border\" "
				"style=\"overflow: hidden;\">\n"
			"<h1 class=\"ns-border\">NetSurf Browser Config</h1>\n"
			"<table class=\"config\">\n"
			"<tr><th>Option</th>"
			"<th>Type</th>"
			"<th>Provenance</th>"
			"<th>Setting</th></tr>\n");
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_config_handler_aborted;
	}


	କର {
		ଯଦି (even) {
			elen = nsoption_snoptionf(buffer + slen,
					ଆକାର buffer - slen,
					opt_loop,
					"<tr class=\"ns-even-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		} ଅନ୍ୟଥା {
			elen = nsoption_snoptionf(buffer + slen,
					ଆକାର buffer - slen,
					opt_loop,
					"<tr class=\"ns-odd-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		}
		ଯଦି (elen <= 0)
			ଭାଙ୍ଗ; /* last option */

		ଯଦି (elen >= (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (ଆକାର buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ଅଚଳ uint8_t *)buffer, slen);
			ଯଦି (res != NSERROR_OK) {
				ଯାଅ fetch_about_config_handler_aborted;
			}
			slen = 0;
		} ଅନ୍ୟଥା {
			/* normal addition */
			slen += elen;
			opt_loop++;
			even = !even;
		}
	} ଯେତେବେଳେ (elen > 0);

	slen += snprintf(buffer + slen, ଆକାର buffer - slen,
			 "</table>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (ଅଚଳ uint8_t *)buffer, slen);
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	ଫେରନ୍ତୁ true;

fetch_about_config_handler_aborted:
	ଫେରନ୍ତୁ false;
}
