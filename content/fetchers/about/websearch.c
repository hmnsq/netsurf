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

#شامل <stdbool.h>
#شامل <stdint.h>
#شامل <sys/types.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "utils/errors.h"
#شامل "utils/nsurl.h"
#شامل "utils/url.h"

#شامل "content/fetch.h"
#شامل "desktop/searchweb.h"

#شامل "private.h"
#شامل "websearch.h"

ساکن nserror
process_query_section(ثابت حرف *str, size_t len, حرف **رکن)
{
	اگر (len < 3) {
		واپس NSERROR_BAD_PARAMETER;
	}
	اگر (str[0] != 'q' || str[1] != '=') {
		واپس NSERROR_BAD_PARAMETER;
	}
	واپس url_unescape(str + 2, len - 2, تہی, رکن);
}

ساکن nserror
searchterm_from_query(ساخت nsurl *url, حرف **رکن)
{
	nserror res;
	حرف *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	برائے (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		جب (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, رکن);
		اگر (res == NSERROR_OK) {
			توڑو;
		}
		kvlen++; /* account for & separator */
	}
	آزاد(querystr);

	واپس res;
}

bool fetch_about_websearch_handler(ساخت fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	حرف *رکن;

	res = searchterm_from_query(fetch_about_get_url(ctx), &رکن);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	res = تلاش_web_omni(رکن, تلاش_WEB_OMNI_SEARCHONLY, &url);
	آزاد(رکن);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	واپس true;
}
