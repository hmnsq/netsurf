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

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "netsurf/types.h"
#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/nsoption.h"

#అన్తర్భావ "private.h"
#అన్తర్భావ "config.h"

/**
 * Handler to generate about scheme config page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_config_handler(నిర్మాణం fetch_about_context *ctx)
{
	అక్షరం బఫరు[1024];
	పూర్ణాంకం slen = 0;
	గుర్తులేని పూర్ణాంకం opt_loop = 0;
	పూర్ణాంకం elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_సమితి_http_సంకేతం(ctx, 200);

	/* content type */
	ఒకవేళ (fetch_about_send_శీర్షిక(ctx, "Content-Type: text/html")) {
		వెళ్ళు fetch_about_config_handler_aborted;
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
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_config_handler_aborted;
	}


	చేయి {
		ఒకవేళ (even) {
			elen = nsoption_snoptionf(బఫరు + slen,
					పరిమాణం బఫరు - slen,
					opt_loop,
					"<tr class=\"ns-even-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		} లేకపోతే {
			elen = nsoption_snoptionf(బఫరు + slen,
					పరిమాణం బఫరు - slen,
					opt_loop,
					"<tr class=\"ns-odd-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		}
		ఒకవేళ (elen <= 0)
			ఆపు; /* last option */

		ఒకవేళ (elen >= (పూర్ణాంకం) (పరిమాణం బఫరు - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (స్థిరరాశి uint8_t *)బఫరు, slen);
			ఒకవేళ (res != NSERROR_OK) {
				వెళ్ళు fetch_about_config_handler_aborted;
			}
			slen = 0;
		} లేకపోతే {
			/* normal addition */
			slen += elen;
			opt_loop++;
			even = !even;
		}
	} అయితే (elen > 0);

	slen += snprintf(బఫరు + slen, పరిమాణం బఫరు - slen,
			 "</table>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (స్థిరరాశి uint8_t *)బఫరు, slen);
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_config_handler_aborted;
	}

	fetch_about_send_ముగిసిన(ctx);

	తిరిగి true;

fetch_about_config_handler_aborted:
	తిరిగి false;
}
