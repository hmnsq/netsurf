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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "netsurf/types.h"
#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/nsoption.h"

#অন্তর্ভাব "private.h"
#অন্তর্ভাব "config.h"

/**
 * Handler to generate about scheme config page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_config_handler(সংরচনা fetch_about_context *ctx)
{
	বর্ণ buffer[1024];
	পূর্ণাঙ্ক slen = 0;
	unsigned পূর্ণাঙ্ক opt_loop = 0;
	পূর্ণাঙ্ক elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	যদি (fetch_about_send_header(ctx, "Content-Type: text/html")) {
		গচ্ছ fetch_about_config_handler_aborted;
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
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_config_handler_aborted;
	}


	কুরু {
		যদি (even) {
			elen = nsoption_snoptionf(buffer + slen,
					পরিমাণ buffer - slen,
					opt_loop,
					"<tr class=\"ns-even-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		} অন্যথা {
			elen = nsoption_snoptionf(buffer + slen,
					পরিমাণ buffer - slen,
					opt_loop,
					"<tr class=\"ns-odd-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		}
		যদি (elen <= 0)
			ভঙ্গ; /* last option */

		যদি (elen >= (পূর্ণাঙ্ক) (পরিমাণ buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ধ্রুব uint8_t *)buffer, slen);
			যদি (res != NSERROR_OK) {
				গচ্ছ fetch_about_config_handler_aborted;
			}
			slen = 0;
		} অন্যথা {
			/* normal addition */
			slen += elen;
			opt_loop++;
			even = !even;
		}
	} যাবত্ (elen > 0);

	slen += snprintf(buffer + slen, পরিমাণ buffer - slen,
			 "</table>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (ধ্রুব uint8_t *)buffer, slen);
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	প্রত্যাগম true;

fetch_about_config_handler_aborted:
	প্রত্যাগম false;
}
