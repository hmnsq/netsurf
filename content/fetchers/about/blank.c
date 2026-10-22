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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/types.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "blank.h"

/**
 * Handler to generate about scheme cache page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_blank_handler(ਬਣਤਰ fetch_about_context *ctx)
{
	ਅਚਲ ਅੱਖਰ ਬਫਰ[2] = { ' ', '\0' };

	/* content is going to return ok */
	fetch_about_ਸੈੱਟ_http_ਕੋਡ(ctx, 200);

	/* content type */
	ਜੇ (fetch_about_send_ਸਿਰਲੇਖ(ctx, "Content-Type: text/html"))
		ਜਾਓ fetch_about_blank_handler_aborted;

	ਜੇ (fetch_about_senddata(ctx, (ਅਚਲ uint8_t *) ਬਫਰ, ਲੜੀ_ਲੰਬਾਈ(ਬਫਰ)) != NSERROR_OK)
		ਜਾਓ fetch_about_blank_handler_aborted;

	fetch_about_send_ਮੁਕੰਮਲ(ctx);

	ਵਾਪਸ true;

fetch_about_blank_handler_aborted:
	ਵਾਪਸ false;
}
