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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "netsurf/types.h"
#அந்தர்பாவ "utils/errors.h"

#அந்தர்பாவ "private.h"
#அந்தர்பாவ "blank.h"

/**
 * Handler to generate about scheme cache page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_blank_handler(கட்டமைப்பு fetch_about_context *ctx)
{
	மாறிலி எழுத்து buffer[2] = { ' ', '\0' };

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	எனில் (fetch_about_send_header(ctx, "Content-Type: text/html"))
		செல் fetch_about_blank_handler_aborted;

	எனில் (fetch_about_senddata(ctx, (மாறிலி uint8_t *) buffer, strlen(buffer)) != NSERROR_OK)
		செல் fetch_about_blank_handler_aborted;

	fetch_about_send_finished(ctx);

	திரும்பு true;

fetch_about_blank_handler_aborted:
	திரும்பு false;
}
