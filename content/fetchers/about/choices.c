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
#شامل <stddef.h>
#شامل <stdio.h>

#شامل "netsurf/types.h"
#شامل "utils/errors.h"
#شامل "utils/nsoption.h"

#شامل "private.h"
#شامل "choices.h"

/**
 * Generate the text of a Choices file which represents the current
 * in use options.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_choices_handler(ساختار fetch_about_context *ctx)
{
	نویسه buffer[1024];
	صحیح code = 200;
	صحیح slen;
	unsigned صحیح opt_loop = 0;
	صحیح res = 0;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	اگر (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		برو fetch_about_choices_handler_aborted;

	slen = snprintf(buffer, اندازه buffer,
		 "# Automatically generated current NetSurf browser Choices\n");

	انجامبده {
		res = nsoption_snoptionf(buffer + slen,
				اندازه buffer - slen,
				opt_loop,
				"%k:%v\n");
		اگر (res <= 0)
			بشکن; /* last option */

		اگر (res >= (صحیح) (اندازه buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ثابت uint8_t *)buffer, slen);
			اگر (res != NSERROR_OK) {
				برو fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} وگرنه {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} وقتی (res > 0);

	res = fetch_about_senddata(ctx, (ثابت uint8_t *)buffer, slen);
	اگر (res != NSERROR_OK) {
		برو fetch_about_choices_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	بازگشت true;

fetch_about_choices_handler_aborted:
	بازگشت false;
}
