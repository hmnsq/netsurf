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
bool fetch_about_choices_handler(ساخت fetch_about_context *ctx)
{
	حرف بفر[1024];
	صحیح رمز = 200;
	صحیح slen;
	غیرعلامتی صحیح opt_loop = 0;
	صحیح res = 0;

	/* content is going to return ok */
	fetch_about_سیٹ_http_رمز(ctx, رمز);

	/* content type */
	اگر (fetch_about_send_سرخی(ctx, "Content-Type: text/plain"))
		جاؤ fetch_about_choices_handler_aborted;

	slen = snprintf(بفر, حجم بفر,
		 "# Automatically generated current NetSurf browser Choices\n");

	کرو {
		res = nsoption_snoptionf(بفر + slen,
				حجم بفر - slen,
				opt_loop,
				"%k:%v\n");
		اگر (res <= 0)
			توڑو; /* last option */

		اگر (res >= (صحیح) (حجم بفر - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ثابت uint8_t *)بفر, slen);
			اگر (res != NSERROR_OK) {
				جاؤ fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} ورنہ {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} جب (res > 0);

	res = fetch_about_senddata(ctx, (ثابت uint8_t *)بفر, slen);
	اگر (res != NSERROR_OK) {
		جاؤ fetch_about_choices_handler_aborted;
	}

	fetch_about_send_ختم_شدہ(ctx);

	واپس true;

fetch_about_choices_handler_aborted:
	واپس false;
}
