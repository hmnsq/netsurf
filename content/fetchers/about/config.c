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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "netsurf/types.h"
#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/nsoption.h"

#अन्तर्भाव "private.h"
#अन्तर्भाव "config.h"

/**
 * Handler to generate about scheme config page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_config_handler(संरचना fetch_about_context *ctx)
{
	अक्षर buffer[1024];
	पूर्णांक slen = 0;
	अचिन्हित पूर्णांक opt_loop = 0;
	पूर्णांक elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	जर (fetch_about_send_header(ctx, "Content-Type: text/html")) {
		जा fetch_about_config_handler_aborted;
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
	जर (res != NSERROR_OK) {
		जा fetch_about_config_handler_aborted;
	}


	करा {
		जर (even) {
			elen = nsoption_snoptionf(buffer + slen,
					आकार buffer - slen,
					opt_loop,
					"<tr class=\"ns-even-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		} अन्यथा {
			elen = nsoption_snoptionf(buffer + slen,
					आकार buffer - slen,
					opt_loop,
					"<tr class=\"ns-odd-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		}
		जर (elen <= 0)
			थांबा; /* last option */

		जर (elen >= (पूर्णांक) (आकार buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (स्थिरांक uint8_t *)buffer, slen);
			जर (res != NSERROR_OK) {
				जा fetch_about_config_handler_aborted;
			}
			slen = 0;
		} अन्यथा {
			/* normal addition */
			slen += elen;
			opt_loop++;
			even = !even;
		}
	} पर्यंत (elen > 0);

	slen += snprintf(buffer + slen, आकार buffer - slen,
			 "</table>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (स्थिरांक uint8_t *)buffer, slen);
	जर (res != NSERROR_OK) {
		जा fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	परत true;

fetch_about_config_handler_aborted:
	परत false;
}
