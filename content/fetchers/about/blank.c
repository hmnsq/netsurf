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

#ಅನ್ತರ್ಭಾವ "private.h"
#ಅನ್ತರ್ಭಾವ "blank.h"

/**
 * Handler to generate about scheme cache page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_blank_handler(ರಚನೆ fetch_about_context *ctx)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ ಬಫರ್[2] = { ' ', '\0' };

	/* content is going to return ok */
	fetch_about_ಗಣ_http_ಸಂಕೇತ(ctx, 200);

	/* content type */
	ಆದರೆ (fetch_about_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: text/html"))
		ಹೋಗು fetch_about_blank_handler_aborted;

	ಆದರೆ (fetch_about_senddata(ctx, (ಸ್ಥಿರಾಂಕ uint8_t *) ಬಫರ್, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಬಫರ್)) != NSERROR_OK)
		ಹೋಗು fetch_about_blank_handler_aborted;

	fetch_about_send_ಪೂರ್ಣಗೊಂಡ(ctx);

	ಹಿಂತಿರುಗಿ true;

fetch_about_blank_handler_aborted:
	ಹಿಂತಿರುಗಿ false;
}
