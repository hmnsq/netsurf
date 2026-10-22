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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/types.h"
#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"

#ଅନ୍ତର୍ଭାଵ "private.h"
#ଅନ୍ତର୍ଭାଵ "choices.h"

/**
 * Generate the text of a Choices file which represents the current
 * in use options.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_choices_handler(ସଂରଚନା fetch_about_context *ctx)
{
	ଅକ୍ଷର buffer[1024];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା code = 200;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା slen;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_loop = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 0;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	ଯଦି (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		ଯାଅ fetch_about_choices_handler_aborted;

	slen = snprintf(buffer, ଆକାର buffer,
		 "# Automatically generated current NetSurf browser Choices\n");

	କର {
		res = nsoption_snoptionf(buffer + slen,
				ଆକାର buffer - slen,
				opt_loop,
				"%k:%v\n");
		ଯଦି (res <= 0)
			ଭାଙ୍ଗ; /* last option */

		ଯଦି (res >= (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (ଆକାର buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ଅଚଳ uint8_t *)buffer, slen);
			ଯଦି (res != NSERROR_OK) {
				ଯାଅ fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} ଅନ୍ୟଥା {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} ଯେତେବେଳେ (res > 0);

	res = fetch_about_senddata(ctx, (ଅଚଳ uint8_t *)buffer, slen);
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ fetch_about_choices_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	ଫେରନ୍ତୁ true;

fetch_about_choices_handler_aborted:
	ଫେରନ୍ତୁ false;
}
