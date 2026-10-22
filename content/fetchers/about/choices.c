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
bool fetch_about_choices_handler(ಸಂರಚನಾ fetch_about_context *ctx)
{
	ವರ್ಣ buffer[1024];
	ಪೂರ್ಣಾಙ್ಕ code = 200;
	ಪೂರ್ಣಾಙ್ಕ slen;
	unsigned ಪೂರ್ಣಾಙ್ಕ opt_loop = 0;
	ಪೂರ್ಣಾಙ್ಕ res = 0;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	ಯದಿ (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		ಗಚ್ಛ fetch_about_choices_handler_aborted;

	slen = snprintf(buffer, ಪರಿಮಾಣ buffer,
		 "# Automatically generated current NetSurf browser Choices\n");

	ಕುರು {
		res = nsoption_snoptionf(buffer + slen,
				ಪರಿಮಾಣ buffer - slen,
				opt_loop,
				"%k:%v\n");
		ಯದಿ (res <= 0)
			ಭಙ್ಗ; /* last option */

		ಯದಿ (res >= (ಪೂರ್ಣಾಙ್ಕ) (ಪರಿಮಾಣ buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ಧ್ರುವ uint8_t *)buffer, slen);
			ಯದಿ (res != NSERROR_OK) {
				ಗಚ್ಛ fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} ಅನ್ಯಥಾ {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} ಯಾವತ್ (res > 0);

	res = fetch_about_senddata(ctx, (ಧ್ರುವ uint8_t *)buffer, slen);
	ಯದಿ (res != NSERROR_OK) {
		ಗಚ್ಛ fetch_about_choices_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	ಪ್ರತ್ಯಾಗಮ true;

fetch_about_choices_handler_aborted:
	ಪ್ರತ್ಯಾಗಮ false;
}
