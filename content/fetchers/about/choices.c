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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdio.h>

#અન્તર્ભાવ "netsurf/types.h"
#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "utils/nsoption.h"

#અન્તર્ભાવ "private.h"
#અન્તર્ભાવ "choices.h"

/**
 * Generate the text of a Choices file which represents the current
 * in use options.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_choices_handler(સંરચના fetch_about_context *ctx)
{
	અક્ષર buffer[1024];
	પૂર્ણાંક code = 200;
	પૂર્ણાંક slen;
	unsigned પૂર્ણાંક opt_loop = 0;
	પૂર્ણાંક res = 0;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	જો (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		જાઓ fetch_about_choices_handler_aborted;

	slen = snprintf(buffer, કદ buffer,
		 "# Automatically generated current NetSurf browser Choices\n");

	કરો {
		res = nsoption_snoptionf(buffer + slen,
				કદ buffer - slen,
				opt_loop,
				"%k:%v\n");
		જો (res <= 0)
			તોડો; /* last option */

		જો (res >= (પૂર્ણાંક) (કદ buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (અચલ uint8_t *)buffer, slen);
			જો (res != NSERROR_OK) {
				જાઓ fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} નહીંતર {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} જ્યારે (res > 0);

	res = fetch_about_senddata(ctx, (અચલ uint8_t *)buffer, slen);
	જો (res != NSERROR_OK) {
		જાઓ fetch_about_choices_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	પાછા true;

fetch_about_choices_handler_aborted:
	પાછા false;
}
