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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdio.h>

#अन्तर्भाव "netsurf/types.h"
#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/nsoption.h"

#अन्तर्भाव "private.h"
#अन्तर्भाव "choices.h"

/**
 * Generate the text of a Choices file which represents the current
 * in use options.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_choices_handler(संरचना fetch_about_context *ctx)
{
	अक्षर buffer[1024];
	पूर्णांक code = 200;
	पूर्णांक slen;
	अचिन्हित पूर्णांक opt_loop = 0;
	पूर्णांक res = 0;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	जर (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		जा fetch_about_choices_handler_aborted;

	slen = snprintf(buffer, आकार buffer,
		 "# Automatically generated current NetSurf browser Choices\n");

	करा {
		res = nsoption_snoptionf(buffer + slen,
				आकार buffer - slen,
				opt_loop,
				"%k:%v\n");
		जर (res <= 0)
			थांबा; /* last option */

		जर (res >= (पूर्णांक) (आकार buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (स्थिरांक uint8_t *)buffer, slen);
			जर (res != NSERROR_OK) {
				जा fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} अन्यथा {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} पर्यंत (res > 0);

	res = fetch_about_senddata(ctx, (स्थिरांक uint8_t *)buffer, slen);
	जर (res != NSERROR_OK) {
		जा fetch_about_choices_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	परत true;

fetch_about_choices_handler_aborted:
	परत false;
}
