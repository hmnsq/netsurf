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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "netsurf/types.h"
#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/nsoption.h"

#അന്തര്ഭാവ "private.h"
#അന്തര്ഭാവ "config.h"

/**
 * Handler to generate about scheme config page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_config_handler(ഘടന fetch_about_context *ctx)
{
	അക്ഷരം buffer[1024];
	പൂർണ്ണസംഖ്യ slen = 0;
	unsigned പൂർണ്ണസംഖ്യ opt_loop = 0;
	പൂർണ്ണസംഖ്യ elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	എങ്കിൽ (fetch_about_send_header(ctx, "Content-Type: text/html")) {
		പോകുക fetch_about_config_handler_aborted;
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
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_config_handler_aborted;
	}


	ചെയ്യുക {
		എങ്കിൽ (even) {
			elen = nsoption_snoptionf(buffer + slen,
					വലുപ്പം buffer - slen,
					opt_loop,
					"<tr class=\"ns-even-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		} അല്ലെങ്കിൽ {
			elen = nsoption_snoptionf(buffer + slen,
					വലുപ്പം buffer - slen,
					opt_loop,
					"<tr class=\"ns-odd-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		}
		എങ്കിൽ (elen <= 0)
			നിർത്തുക; /* last option */

		എങ്കിൽ (elen >= (പൂർണ്ണസംഖ്യ) (വലുപ്പം buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (സ്ഥിരം uint8_t *)buffer, slen);
			എങ്കിൽ (res != NSERROR_OK) {
				പോകുക fetch_about_config_handler_aborted;
			}
			slen = 0;
		} അല്ലെങ്കിൽ {
			/* normal addition */
			slen += elen;
			opt_loop++;
			even = !even;
		}
	} ആകുമ്പോൾ (elen > 0);

	slen += snprintf(buffer + slen, വലുപ്പം buffer - slen,
			 "</table>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (സ്ഥിരം uint8_t *)buffer, slen);
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	തിരികെ true;

fetch_about_config_handler_aborted:
	തിരികെ false;
}
