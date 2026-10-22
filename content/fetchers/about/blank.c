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
bool fetch_about_blank_handler(ಸಂರಚನಾ fetch_about_context *ctx)
{
	ಧ್ರುವ ವರ್ಣ buffer[2] = { ' ', '\0' };

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	ಯದಿ (fetch_about_send_header(ctx, "Content-Type: text/html"))
		ಗಚ್ಛ fetch_about_blank_handler_aborted;

	ಯದಿ (fetch_about_senddata(ctx, (ಧ್ರುವ uint8_t *) buffer, strlen(buffer)) != NSERROR_OK)
		ಗಚ್ಛ fetch_about_blank_handler_aborted;

	fetch_about_send_finished(ctx);

	ಪ್ರತ್ಯಾಗಮ true;

fetch_about_blank_handler_aborted:
	ಪ್ರತ್ಯಾಗಮ false;
}
