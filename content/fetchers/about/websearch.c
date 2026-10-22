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

ایستا nserror
process_query_section(ثابت نویسه *str, size_t len, نویسه **term)
{
	اگر (len < 3) {
		بازگشت NSERROR_BAD_PARAMETER;
	}
	اگر (str[0] != 'q' || str[1] != '=') {
		بازگشت NSERROR_BAD_PARAMETER;
	}
	بازگشت url_unescape(str + 2, len - 2, پوچ, term);
}

ایستا nserror
searchterm_from_query(ساختار nsurl *url, نویسه **term)
{
	nserror res;
	نویسه *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	برای (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		وقتی (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, term);
		اگر (res == NSERROR_OK) {
			بشکن;
		}
		kvlen++; /* account for & separator */
	}
	آزاد(querystr);

	بازگشت res;
}

bool fetch_about_websearch_handler(ساختار fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	نویسه *term;

	res = searchterm_from_query(fetch_about_get_url(ctx), &term);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	res = جستجو_web_omni(term, جستجو_WEB_OMNI_SEARCHONLY, &url);
	آزاد(term);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	بازگشت true;
}
