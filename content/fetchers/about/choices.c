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

#समावेश <stdbool.h>
#समावेश <stddef.h>
#समावेश <stdio.h>

#समावेश "netsurf/types.h"
#समावेश "utils/errors.h"
#समावेश "utils/nsoption.h"

#समावेश "private.h"
#समावेश "choices.h"

/**
 * Generate the text of a Choices file which represents the current
 * in use options.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_choices_handler(काष्ठा fetch_about_context *ctx)
{
	अक्षर बफर[1024];
	पूर्णांक code = 200;
	पूर्णांक slen;
	अचिन्हित पूर्णांक opt_loop = 0;
	पूर्णांक res = 0;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	अगर (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		जाओ fetch_about_choices_handler_aborted;

	slen = snprintf(बफर, माप बफर,
		 "# Automatically generated current NetSurf browser Choices\n");

	करो {
		res = nsoption_snoptionf(बफर + slen,
				माप बफर - slen,
				opt_loop,
				"%k:%v\n");
		अगर (res <= 0)
			अवरोध; /* last option */

		अगर (res >= (पूर्णांक) (माप बफर - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (स्थिर uint8_t *)बफर, slen);
			अगर (res != NSERROR_OK) {
				जाओ fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} अन्यथा {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} जबतक (res > 0);

	res = fetch_about_senddata(ctx, (स्थिर uint8_t *)बफर, slen);
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_choices_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	वापस true;

fetch_about_choices_handler_aborted:
	वापस false;
}
