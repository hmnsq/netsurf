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

#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <string.h>

#समावेश "netsurf/types.h"
#समावेश "utils/errors.h"

#समावेश "private.h"
#समावेश "blank.h"

/**
 * Handler to generate about scheme cache page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_blank_handler(काष्ठा fetch_about_context *ctx)
{
	स्थिर अक्षर बफर[2] = { ' ', '\0' };

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	अगर (fetch_about_send_header(ctx, "Content-Type: text/html"))
		जाओ fetch_about_blank_handler_aborted;

	अगर (fetch_about_senddata(ctx, (स्थिर uint8_t *) बफर, म_माप(बफर)) != NSERROR_OK)
		जाओ fetch_about_blank_handler_aborted;

	fetch_about_send_finished(ctx);

	वापस true;

fetch_about_blank_handler_aborted:
	वापस false;
}
