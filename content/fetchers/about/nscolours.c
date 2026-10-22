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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "netsurf/plot_style.h"

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/nscolour.h"

#അന്തര്ഭാവ "private.h"
#അന്തര്ഭാവ "nscolours.h"

/**
 * Handler to generate the nscolours stylesheet
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_nscolours_handler(ഘടന fetch_about_context *ctx)
{
	nserror res;
	സ്ഥിരം അക്ഷരം *stylesheet;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	എങ്കിൽ (fetch_about_send_header(ctx, "Content-Type: text/css; charset=utf-8")) {
		പോകുക aborted;
	}

	res = nscolour_get_stylesheet(&stylesheet);
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			"html {\n"
			"\tbackground-color: #%06x;\n"
			"}\n"
			"%s",
			colour_rb_swap(nscolours[NSCOLOUR_WIN_ODD_BG]),
			stylesheet);
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക aborted;
	}

	fetch_about_send_finished(ctx);

	തിരികെ true;

aborted:

	തിരികെ false;
}
