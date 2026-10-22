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

#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <stdint.h>
#ضمن <stdlib.h>

#ضمن "netsurf/plot_style.h"

#ضمن "utils/errors.h"
#ضمن "utils/nscolour.h"

#ضمن "private.h"
#ضمن "nscolours.h"

/**
 * Handler to generate the nscolours stylesheet
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_nscolours_handler(بنية fetch_about_context *ctx)
{
	nserror res;
	ثابت حرف *stylesheet;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	إذا (fetch_about_send_header(ctx, "Content-Type: text/css; charset=utf-8")) {
		اذهب aborted;
	}

	res = nscolour_get_stylesheet(&stylesheet);
	إذا (res != NSERROR_OK) {
		اذهب aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			"html {\n"
			"\tbackground-color: #%06x;\n"
			"}\n"
			"%s",
			colour_rb_swap(nscolours[NSCOLOUR_WIN_ODD_BG]),
			stylesheet);
	إذا (res != NSERROR_OK) {
		اذهب aborted;
	}

	fetch_about_send_finished(ctx);

	إرجاع true;

aborted:

	إرجاع false;
}
