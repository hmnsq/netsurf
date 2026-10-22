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

#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <string.h>

#ضمن "netsurf/types.h"
#ضمن "utils/errors.h"
#ضمن "utils/nsoption.h"

#ضمن "private.h"
#ضمن "config.h"

/**
 * Handler to generate about scheme config page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_config_handler(بنية fetch_about_context *ctx)
{
	حرف buffer[1024];
	صحيح slen = 0;
	unsigned صحيح opt_loop = 0;
	صحيح elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	إذا (fetch_about_send_header(ctx, "Content-Type: text/html")) {
		اذهب fetch_about_config_handler_aborted;
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
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_config_handler_aborted;
	}


	افعل {
		إذا (even) {
			elen = nsoption_snoptionf(buffer + slen,
					حجم buffer - slen,
					opt_loop,
					"<tr class=\"ns-even-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		} وإلا {
			elen = nsoption_snoptionf(buffer + slen,
					حجم buffer - slen,
					opt_loop,
					"<tr class=\"ns-odd-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		}
		إذا (elen <= 0)
			اكسر; /* last option */

		إذا (elen >= (صحيح) (حجم buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ثابت uint8_t *)buffer, slen);
			إذا (res != NSERROR_OK) {
				اذهب fetch_about_config_handler_aborted;
			}
			slen = 0;
		} وإلا {
			/* normal addition */
			slen += elen;
			opt_loop++;
			even = !even;
		}
	} طالما (elen > 0);

	slen += snprintf(buffer + slen, حجم buffer - slen,
			 "</table>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (ثابت uint8_t *)buffer, slen);
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	أرجع true;

fetch_about_config_handler_aborted:
	أرجع false;
}
