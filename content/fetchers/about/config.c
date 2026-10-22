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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "netsurf/types.h"
#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"

#ಅನ್ತರ್ಭಾವ "private.h"
#ಅನ್ತರ್ಭಾವ "config.h"

/**
 * Handler to generate about scheme config page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_config_handler(ರಚನೆ fetch_about_context *ctx)
{
	ಅಕ್ಷರ ಬಫರ್[1024];
	ಪೂರ್ಣಾಂಕ slen = 0;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ opt_loop = 0;
	ಪೂರ್ಣಾಂಕ elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_ಗಣ_http_ಸಂಕೇತ(ctx, 200);

	/* content type */
	ಆದರೆ (fetch_about_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: text/html")) {
		ಹೋಗು fetch_about_config_handler_aborted;
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
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_config_handler_aborted;
	}


	ಮಾಡು {
		ಆದರೆ (even) {
			elen = nsoption_snoptionf(ಬಫರ್ + slen,
					ಗಾತ್ರ ಬಫರ್ - slen,
					opt_loop,
					"<tr class=\"ns-even-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		} ಇಲ್ಲದಿದ್ದರೆ {
			elen = nsoption_snoptionf(ಬಫರ್ + slen,
					ಗಾತ್ರ ಬಫರ್ - slen,
					opt_loop,
					"<tr class=\"ns-odd-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		}
		ಆದರೆ (elen <= 0)
			ನಿಲ್ಲಿಸು; /* last option */

		ಆದರೆ (elen >= (ಪೂರ್ಣಾಂಕ) (ಗಾತ್ರ ಬಫರ್ - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ಸ್ಥಿರಾಂಕ uint8_t *)ಬಫರ್, slen);
			ಆದರೆ (res != NSERROR_OK) {
				ಹೋಗು fetch_about_config_handler_aborted;
			}
			slen = 0;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* normal addition */
			slen += elen;
			opt_loop++;
			even = !even;
		}
	} ಆಗ (elen > 0);

	slen += snprintf(ಬಫರ್ + slen, ಗಾತ್ರ ಬಫರ್ - slen,
			 "</table>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (ಸ್ಥಿರಾಂಕ uint8_t *)ಬಫರ್, slen);
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_config_handler_aborted;
	}

	fetch_about_send_ಪೂರ್ಣಗೊಂಡ(ctx);

	ಹಿಂತಿರುಗಿ true;

fetch_about_config_handler_aborted:
	ಹಿಂತಿರುಗಿ false;
}
