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
 * content generator for the about scheme query privacy page
 */

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "netsurf/plot_style.h"

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/nscolour.h"

#అన్తర్భావ "private.h"
#అన్తర్భావ "nscolours.h"

/**
 * Handler to generate the nscolours stylesheet
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_nscolours_handler(నిర్మాణం fetch_about_context *ctx)
{
	nserror res;
	స్థిరరాశి అక్షరం *stylesheet;

	/* content is going to return ok */
	fetch_about_సమితి_http_సంకేతం(ctx, 200);

	/* content type */
	ఒకవేళ (fetch_about_send_శీర్షిక(ctx, "Content-Type: text/css; charset=utf-8")) {
		వెళ్ళు aborted;
	}

	res = nscolour_get_stylesheet(&stylesheet);
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			"html {\n"
			"\tbackground-color: #%06x;\n"
			"}\n"
			"%s",
			రంగు_rb_మార్పిడి(nscolours[NSCOLOUR_WIN_ODD_BG]),
			stylesheet);
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు aborted;
	}

	fetch_about_send_ముగిసిన(ctx);

	తిరిగి true;

aborted:

	తిరిగి false;
}
