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
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdio.h>

#অন্তর্ভাব "netsurf/types.h"
#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/nsoption.h"

#অন্তর্ভাব "private.h"
#অন্তর্ভাব "choices.h"

/**
 * Generate the text of a Choices file which represents the current
 * in use options.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_choices_handler(সংরচনা fetch_about_context *ctx)
{
	বর্ণ buffer[1024];
	পূর্ণাঙ্ক code = 200;
	পূর্ণাঙ্ক slen;
	unsigned পূর্ণাঙ্ক opt_loop = 0;
	পূর্ণাঙ্ক res = 0;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	যদি (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		গচ্ছ fetch_about_choices_handler_aborted;

	slen = snprintf(buffer, পরিমাণ buffer,
		 "# Automatically generated current NetSurf browser Choices\n");

	কুরু {
		res = nsoption_snoptionf(buffer + slen,
				পরিমাণ buffer - slen,
				opt_loop,
				"%k:%v\n");
		যদি (res <= 0)
			ভঙ্গ; /* last option */

		যদি (res >= (পূর্ণাঙ্ক) (পরিমাণ buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ধ্রুব uint8_t *)buffer, slen);
			যদি (res != NSERROR_OK) {
				গচ্ছ fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} অন্যথা {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} যাবত্ (res > 0);

	res = fetch_about_senddata(ctx, (ধ্রুব uint8_t *)buffer, slen);
	যদি (res != NSERROR_OK) {
		গচ্ছ fetch_about_choices_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	প্রত্যাগম true;

fetch_about_choices_handler_aborted:
	প্রত্যাগম false;
}
