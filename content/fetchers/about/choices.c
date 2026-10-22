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

#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdio.h>

#ضمن "netsurf/types.h"
#ضمن "utils/errors.h"
#ضمن "utils/nsoption.h"

#ضمن "private.h"
#ضمن "choices.h"

/**
 * Generate the text of a Choices file which represents the current
 * in use options.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_choices_handler(بنية fetch_about_context *ctx)
{
	حرف buffer[1024];
	صحيح code = 200;
	صحيح slen;
	unsigned صحيح opt_loop = 0;
	صحيح res = 0;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	إذا (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		اذهب fetch_about_choices_handler_aborted;

	slen = snprintf(buffer, حجم buffer,
		 "# Automatically generated current NetSurf browser Choices\n");

	افعل {
		res = nsoption_snoptionf(buffer + slen,
				حجم buffer - slen,
				opt_loop,
				"%k:%v\n");
		إذا (res <= 0)
			اكسر; /* last option */

		إذا (res >= (صحيح) (حجم buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ثابت uint8_t *)buffer, slen);
			إذا (res != NSERROR_OK) {
				اذهب fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} وإلا {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} بينما (res > 0);

	res = fetch_about_senddata(ctx, (ثابت uint8_t *)buffer, slen);
	إذا (res != NSERROR_OK) {
		اذهب fetch_about_choices_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	إرجاع true;

fetch_about_choices_handler_aborted:
	إرجاع false;
}
