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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdio.h>

#അന്തര്ഭാവ "netsurf/types.h"
#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/nsoption.h"

#അന്തര്ഭാവ "private.h"
#അന്തര്ഭാവ "choices.h"

/**
 * Generate the text of a Choices file which represents the current
 * in use options.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_choices_handler(ഘടന fetch_about_context *ctx)
{
	അക്ഷരം buffer[1024];
	പൂർണ്ണസംഖ്യ code = 200;
	പൂർണ്ണസംഖ്യ slen;
	unsigned പൂർണ്ണസംഖ്യ opt_loop = 0;
	പൂർണ്ണസംഖ്യ res = 0;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	എങ്കിൽ (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		പോകുക fetch_about_choices_handler_aborted;

	slen = snprintf(buffer, വലുപ്പം buffer,
		 "# Automatically generated current NetSurf browser Choices\n");

	ചെയ്യുക {
		res = nsoption_snoptionf(buffer + slen,
				വലുപ്പം buffer - slen,
				opt_loop,
				"%k:%v\n");
		എങ്കിൽ (res <= 0)
			നിർത്തുക; /* last option */

		എങ്കിൽ (res >= (പൂർണ്ണസംഖ്യ) (വലുപ്പം buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (സ്ഥിരം uint8_t *)buffer, slen);
			എങ്കിൽ (res != NSERROR_OK) {
				പോകുക fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} അല്ലെങ്കിൽ {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} ആകുമ്പോൾ (res > 0);

	res = fetch_about_senddata(ctx, (സ്ഥിരം uint8_t *)buffer, slen);
	എങ്കിൽ (res != NSERROR_OK) {
		പോകുക fetch_about_choices_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	തിരികെ true;

fetch_about_choices_handler_aborted:
	തിരികെ false;
}
