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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "netsurf/plot_style.h"

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/nscolour.h"

#ಅನ್ತರ್ಭಾವ "private.h"
#ಅನ್ತರ್ಭಾವ "nscolours.h"

/**
 * Handler to generate the nscolours stylesheet
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_nscolours_handler(ರಚನೆ fetch_about_context *ctx)
{
	nserror res;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *stylesheet;

	/* content is going to return ok */
	fetch_about_ಗಣ_http_ಸಂಕೇತ(ctx, 200);

	/* content type */
	ಆದರೆ (fetch_about_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: text/css; charset=utf-8")) {
		ಹೋಗು aborted;
	}

	res = nscolour_get_stylesheet(&stylesheet);
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			"html {\n"
			"\tbackground-color: #%06x;\n"
			"}\n"
			"%s",
			ಬಣ್ಣ_rb_ಅದಲುಬದಲು(nscolours[NSCOLOUR_WIN_ODD_BG]),
			stylesheet);
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು aborted;
	}

	fetch_about_send_ಪೂರ್ಣಗೊಂಡ(ctx);

	ಹಿಂತಿರುಗಿ true;

aborted:

	ಹಿಂತಿರುಗಿ false;
}
