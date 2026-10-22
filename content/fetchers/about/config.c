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

#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <string.h>

#شامل "netsurf/types.h"
#شامل "utils/errors.h"
#شامل "utils/nsoption.h"

#شامل "private.h"
#شامل "config.h"

/**
 * Handler to generate about scheme config page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_config_handler(ساختار fetch_about_context *ctx)
{
	نویسه buffer[1024];
	صحیح slen = 0;
	unsigned صحیح opt_loop = 0;
	صحیح elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	اگر (fetch_about_send_header(ctx, "Content-Type: text/html")) {
		برو fetch_about_config_handler_aborted;
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
	اگر (res != NSERROR_OK) {
		برو fetch_about_config_handler_aborted;
	}


	انجامبده {
		اگر (even) {
			elen = nsoption_snoptionf(buffer + slen,
					اندازه buffer - slen,
					opt_loop,
					"<tr class=\"ns-even-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		} وگرنه {
			elen = nsoption_snoptionf(buffer + slen,
					اندازه buffer - slen,
					opt_loop,
					"<tr class=\"ns-odd-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		}
		اگر (elen <= 0)
			بشکن; /* last option */

		اگر (elen >= (صحیح) (اندازه buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ثابت uint8_t *)buffer, slen);
			اگر (res != NSERROR_OK) {
				برو fetch_about_config_handler_aborted;
			}
			slen = 0;
		} وگرنه {
			/* normal addition */
			slen += elen;
			opt_loop++;
			even = !even;
		}
	} وقتی (elen > 0);

	slen += snprintf(buffer + slen, اندازه buffer - slen,
			 "</table>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (ثابت uint8_t *)buffer, slen);
	اگر (res != NSERROR_OK) {
		برو fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	بازگشت true;

fetch_about_config_handler_aborted:
	بازگشت false;
}
