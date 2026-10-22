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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/types.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h"

/**
 * Handler to generate about scheme config page
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_config_handler(ਬਣਤਰ fetch_about_context *ctx)
{
	ਅੱਖਰ ਬਫਰ[1024];
	ਪੂਰਨ slen = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ opt_loop = 0;
	ਪੂਰਨ elen = 0; /* entry length */
	nserror res;
	bool even = false;

	/* content is going to return ok */
	fetch_about_ਸੈੱਟ_http_ਕੋਡ(ctx, 200);

	/* content type */
	ਜੇ (fetch_about_send_ਸਿਰਲੇਖ(ctx, "Content-Type: text/html")) {
		ਜਾਓ fetch_about_config_handler_aborted;
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
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ fetch_about_config_handler_aborted;
	}


	ਕਰੋ {
		ਜੇ (even) {
			elen = nsoption_snoptionf(ਬਫਰ + slen,
					ਆਕਾਰ ਬਫਰ - slen,
					opt_loop,
					"<tr class=\"ns-even-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		} ਨਹੀਂਤਾਂ {
			elen = nsoption_snoptionf(ਬਫਰ + slen,
					ਆਕਾਰ ਬਫਰ - slen,
					opt_loop,
					"<tr class=\"ns-odd-bg\">"
						"<th class=\"ns-border\">%k</th>"
						"<td class=\"ns-border\">%t</td>"
						"<td class=\"ns-border\">%p</td>"
						"<td class=\"ns-border\">%V</td>"
					"</tr>\n");
		}
		ਜੇ (elen <= 0)
			ਰੋਕੋ; /* last option */

		ਜੇ (elen >= (ਪੂਰਨ) (ਆਕਾਰ ਬਫਰ - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ਅਚਲ uint8_t *)ਬਫਰ, slen);
			ਜੇ (res != NSERROR_OK) {
				ਜਾਓ fetch_about_config_handler_aborted;
			}
			slen = 0;
		} ਨਹੀਂਤਾਂ {
			/* normal addition */
			slen += elen;
			opt_loop++;
			even = !even;
		}
	} ਜਦੋਂ (elen > 0);

	slen += snprintf(ਬਫਰ + slen, ਆਕਾਰ ਬਫਰ - slen,
			 "</table>\n</body>\n</html>\n");

	res = fetch_about_senddata(ctx, (ਅਚਲ uint8_t *)ਬਫਰ, slen);
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ fetch_about_config_handler_aborted;
	}

	fetch_about_send_ਮੁਕੰਮਲ(ctx);

	ਵਾਪਸ true;

fetch_about_config_handler_aborted:
	ਵਾਪਸ false;
}
