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
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>

#ಅನ್ತರ್ಭಾವ "netsurf/types.h"
#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"

#ಅನ್ತರ್ಭಾವ "private.h"
#ಅನ್ತರ್ಭಾವ "choices.h"

/**
 * Generate the text of a Choices file which represents the current
 * in use options.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_choices_handler(ರಚನೆ fetch_about_context *ctx)
{
	ಅಕ್ಷರ ಬಫರ್[1024];
	ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2 = 200;
	ಪೂರ್ಣಾಂಕ slen;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ opt_loop = 0;
	ಪೂರ್ಣಾಂಕ res = 0;

	/* content is going to return ok */
	fetch_about_ಗಣ_http_ಸಂಕೇತ(ctx, ಸಂಕೇತ_2);

	/* content type */
	ಆದರೆ (fetch_about_send_ಶೀರ್ಷಿಕೆ(ctx, "Content-Type: text/plain"))
		ಹೋಗು fetch_about_choices_handler_aborted;

	slen = snprintf(ಬಫರ್, ಗಾತ್ರ ಬಫರ್,
		 "# Automatically generated current NetSurf browser Choices\n");

	ಮಾಡು {
		res = nsoption_snoptionf(ಬಫರ್ + slen,
				ಗಾತ್ರ ಬಫರ್ - slen,
				opt_loop,
				"%k:%v\n");
		ಆದರೆ (res <= 0)
			ನಿಲ್ಲಿಸು; /* last option */

		ಆದರೆ (res >= (ಪೂರ್ಣಾಂಕ) (ಗಾತ್ರ ಬಫರ್ - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ಸ್ಥಿರಾಂಕ uint8_t *)ಬಫರ್, slen);
			ಆದರೆ (res != NSERROR_OK) {
				ಹೋಗು fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} ಆಗ (res > 0);

	res = fetch_about_senddata(ctx, (ಸ್ಥಿರಾಂಕ uint8_t *)ಬಫರ್, slen);
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು fetch_about_choices_handler_aborted;
	}

	fetch_about_send_ಪೂರ್ಣಗೊಂಡ(ctx);

	ಹಿಂತಿರುಗಿ true;

fetch_about_choices_handler_aborted:
	ಹಿಂತಿರುಗಿ false;
}
