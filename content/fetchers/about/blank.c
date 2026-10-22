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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "netsurf/types.h"
#অন্তর্ভাব "utils/errors.h"

#অন্তর্ভাব "private.h"
#অন্তর্ভাব "blank.h"

/**
 * Handler to generate about scheme cache page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_blank_handler(সংরচনা fetch_about_context *ctx)
{
	ধ্রুব বর্ণ buffer[2] = { ' ', '\0' };

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	যদি (fetch_about_send_header(ctx, "Content-Type: text/html"))
		গচ্ছ fetch_about_blank_handler_aborted;

	যদি (fetch_about_senddata(ctx, (ধ্রুব uint8_t *) buffer, strlen(buffer)) != NSERROR_OK)
		গচ্ছ fetch_about_blank_handler_aborted;

	fetch_about_send_finished(ctx);

	প্রত্যাগম true;

fetch_about_blank_handler_aborted:
	প্রত্যাগম false;
}
