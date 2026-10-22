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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "netsurf/types.h"
#അന്തര്ഭാവ "utils/errors.h"

#അന്തര്ഭാവ "private.h"
#അന്തര്ഭാവ "blank.h"

/**
 * Handler to generate about scheme cache page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_blank_handler(ഘടന fetch_about_context *ctx)
{
	സ്ഥിരം അക്ഷരം buffer[2] = { ' ', '\0' };

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	എങ്കിൽ (fetch_about_send_header(ctx, "Content-Type: text/html"))
		പോകുക fetch_about_blank_handler_aborted;

	എങ്കിൽ (fetch_about_senddata(ctx, (സ്ഥിരം uint8_t *) buffer, strlen(buffer)) != NSERROR_OK)
		പോകുക fetch_about_blank_handler_aborted;

	fetch_about_send_finished(ctx);

	തിരികെ true;

fetch_about_blank_handler_aborted:
	തിരികെ false;
}
