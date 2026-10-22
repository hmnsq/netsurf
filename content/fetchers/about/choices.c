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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdio.h>

#அந்தர்பாவ "netsurf/types.h"
#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/nsoption.h"

#அந்தர்பாவ "private.h"
#அந்தர்பாவ "choices.h"

/**
 * Generate the text of a Choices file which represents the current
 * in use options.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_choices_handler(கட்டமைப்பு fetch_about_context *ctx)
{
	எழுத்து buffer[1024];
	முழு code = 200;
	முழு slen;
	unsigned முழு opt_loop = 0;
	முழு res = 0;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	எனில் (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		செல் fetch_about_choices_handler_aborted;

	slen = snprintf(buffer, அளவு buffer,
		 "# Automatically generated current NetSurf browser Choices\n");

	செய் {
		res = nsoption_snoptionf(buffer + slen,
				அளவு buffer - slen,
				opt_loop,
				"%k:%v\n");
		எனில் (res <= 0)
			நிறுத்து; /* last option */

		எனில் (res >= (முழு) (அளவு buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (மாறிலி uint8_t *)buffer, slen);
			எனில் (res != NSERROR_OK) {
				செல் fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} இல்லையெனில் {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} போது (res > 0);

	res = fetch_about_senddata(ctx, (மாறிலி uint8_t *)buffer, slen);
	எனில் (res != NSERROR_OK) {
		செல் fetch_about_choices_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	திரும்பு true;

fetch_about_choices_handler_aborted:
	திரும்பு false;
}
