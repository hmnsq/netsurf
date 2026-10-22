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
bool fetch_about_choices_handler(কাঠামো fetch_about_context *ctx)
{
	অক্ষর বাফার[1024];
	পূর্ণসংখ্যা code = 200;
	পূর্ণসংখ্যা slen;
	unsigned পূর্ণসংখ্যা opt_loop = 0;
	পূর্ণসংখ্যা res = 0;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	যদি (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		যাও fetch_about_choices_handler_aborted;

	slen = snprintf(বাফার, আকার বাফার,
		 "# Automatically generated current NetSurf browser Choices\n");

	করো {
		res = nsoption_snoptionf(বাফার + slen,
				আকার বাফার - slen,
				opt_loop,
				"%k:%v\n");
		যদি (res <= 0)
			বিরতি; /* last option */

		যদি (res >= (পূর্ণসংখ্যা) (আকার বাফার - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ধ্রুবক uint8_t *)বাফার, slen);
			যদি (res != NSERROR_OK) {
				যাও fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} অন্যথা {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} যখন (res > 0);

	res = fetch_about_senddata(ctx, (ধ্রুবক uint8_t *)বাফার, slen);
	যদি (res != NSERROR_OK) {
		যাও fetch_about_choices_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	ফেরত true;

fetch_about_choices_handler_aborted:
	ফেরত false;
}
