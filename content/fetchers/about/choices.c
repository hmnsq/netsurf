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
	वर्ण बफर[1024];
	पूर्णाङ्क संहिता = 200;
	पूर्णाङ्क slen;
	अचिह्नित पूर्णाङ्क opt_loop = 0;
	पूर्णाङ्क res = 0;

	/* content is going to return ok */
	fetch_about_समुच्चयः_http_संहिता(ctx, संहिता);

	/* content type */
	यदि (fetch_about_send_शीर्षकम्(ctx, "Content-Type: text/plain"))
		गच्छ fetch_about_choices_handler_aborted;

	slen = snprintf(बफर, परिमाण बफर,
		 "# Automatically generated current NetSurf browser Choices\n");

	कुरु {
		res = nsoption_snoptionf(बफर + slen,
				परिमाण बफर - slen,
				opt_loop,
				"%k:%v\n");
		यदि (res <= 0)
			भङ्ग; /* last option */

		यदि (res >= (पूर्णाङ्क) (परिमाण बफर - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ध्रुव uint8_t *)बफर, slen);
			यदि (res != NSERROR_OK) {
				गच्छ fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} अन्यथा {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} यावत् (res > 0);

	res = fetch_about_senddata(ctx, (ध्रुव uint8_t *)बफर, slen);
	यदि (res != NSERROR_OK) {
		गच्छ fetch_about_choices_handler_aborted;
	}

	fetch_about_send_परिसमाप्तम्(ctx);

	प्रत्यागम true;

fetch_about_choices_handler_aborted:
	प्रत्यागम false;
}
