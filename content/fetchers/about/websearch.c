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
 * content generator for the about scheme web search
 */

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/url.h"

#అన్తర్భావ "content/fetch.h"
#అన్తర్భావ "desktop/searchweb.h"

#అన్తర్భావ "private.h"
#అన్తర్భావ "websearch.h"

స్థిర nserror
process_query_section(స్థిరరాశి అక్షరం *str, size_t len, అక్షరం **పదం)
{
	ఒకవేళ (len < 3) {
		తిరిగి NSERROR_BAD_PARAMETER;
	}
	ఒకవేళ (str[0] != 'q' || str[1] != '=') {
		తిరిగి NSERROR_BAD_PARAMETER;
	}
	తిరిగి url_unescape(str + 2, len - 2, ఖాళీ, పదం);
}

స్థిర nserror
searchterm_from_query(నిర్మాణం nsurl *url, అక్షరం **పదం)
{
	nserror res;
	అక్షరం *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	కొరకు (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		అయితే (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, పదం);
		ఒకవేళ (res == NSERROR_OK) {
			ఆపు;
		}
		kvlen++; /* account for & separator */
	}
	విడుదల(querystr);

	తిరిగి res;
}

bool fetch_about_websearch_handler(నిర్మాణం fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	అక్షరం *పదం;

	res = searchterm_from_query(fetch_about_get_url(ctx), &పదం);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	res = శోధన_web_omni(పదం, శోధన_WEB_OMNI_SEARCHONLY, &url);
	విడుదల(పదం);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	తిరిగి true;
}
