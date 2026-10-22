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

#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <string.h>

#شامل "netsurf/types.h"
#شامل "utils/errors.h"

#شامل "private.h"
#شامل "blank.h"

/**
 * Handler to generate about scheme cache page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_blank_handler(ساختار fetch_about_context *ctx)
{
	ثابت نویسه buffer[2] = { ' ', '\0' };

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	اگر (fetch_about_send_header(ctx, "Content-Type: text/html"))
		برو fetch_about_blank_handler_aborted;

	اگر (fetch_about_senddata(ctx, (ثابت uint8_t *) buffer, strlen(buffer)) != NSERROR_OK)
		برو fetch_about_blank_handler_aborted;

	fetch_about_send_finished(ctx);

	بازگشت true;

fetch_about_blank_handler_aborted:
	بازگشت false;
}
