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
#समावेश <stdio.h>
#समावेश <string.h>

#समावेश "netsurf/types.h"
#समावेश "utils/errors.h"
#समावेश "utils/nsoption.h"

#समावेश "private.h"
#समावेश "config.h"

/**
 * Handler to generate about scheme config page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_config_handler(काष्ठा fetch_about_context *ctx)
{
	अक्षर बफर[1024];
	पूर्णांक slen = 0;
	अचिन्हित पूर्णांक opt_loop = 0;
	पूर्णांक elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	अगर (fetch_about_send_header(ctx, "Content-Type: text/html")) {
		जाओ fetch_about_config_handler_aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			"<html>\n<head>\n"
			"<title>NetSurf Browser Config</title>\n"
			"<link rel=\"stylesheet\" type=\"text/css\" "
			"href=\"resource:internal.css\">\n"
			"</head>\n"
			"<body "
				"id =\"configlist\" "
				"class=\"ns-even-bg ns-even-fg ns-border\" "
				"style=\"overflow: hidden;\">\n"
			"<h1 class=\"ns-border\">NetSurf Browser Config</h1>\n"
			"<table class=\"config\">\n"
			"<tr><th>Option</th>"
			"<th>Type</th>"
			"<th>Provenance</th>"
			"<th>Setting</th></tr>\n");
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_config_handler_aborted;
	}


	करो {
		अगर (even) {
			elen = nsoption_snoptionf(बफर + slen,
					माप बफर - slen,
					opt_loop,
					"<tr class=\"ns-even-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		} अन्यथा {
			elen = nsoption_snoptionf(बफर + slen,
					माप बफर - slen,
					opt_loop,
					"<tr class=\"ns-odd-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		}
		अगर (elen <= 0)
			अवरोध; /* last option */

		अगर (elen >= (पूर्णांक) (माप बफर - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (स्थिर uint8_t *)बफर, slen);
			अगर (res != NSERROR_OK) {
				जाओ fetch_about_config_handler_aborted;
			}
			slen = 0;
		} अन्यथा {
			/* normal addition */
			slen += elen;
			opt_loop++;
			even = !even;
		}
	} जबतक (elen > 0);

	slen += snprintf(बफर + slen, माप बफर - slen,
			 "</table>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (स्थिर uint8_t *)बफर, slen);
	अगर (res != NSERROR_OK) {
		जाओ fetch_about_config_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	वापस true;

fetch_about_config_handler_aborted:
	वापस false;
}
