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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "netsurf/types.h"
#अन्तर्भाव "utils/errors.h"

#अन्तर्भाव "private.h"
#अन्तर्भाव "blank.h"

/**
 * Handler to generate about scheme cache page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_blank_handler(संरचना fetch_about_context *ctx)
{
	स्थिरांक अक्षर buffer[2] = { ' ', '\0' };

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	जर (fetch_about_send_header(ctx, "Content-Type: text/html"))
		जा fetch_about_blank_handler_aborted;

	जर (fetch_about_senddata(ctx, (स्थिरांक uint8_t *) buffer, म_माप(buffer)) != NSERROR_OK)
		जा fetch_about_blank_handler_aborted;

	fetch_about_send_finished(ctx);

	परत true;

fetch_about_blank_handler_aborted:
	परत false;
}
