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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "utils/url.h"

#অন্তর্ভাব "content/fetch.h"
#অন্তর্ভাব "desktop/searchweb.h"

#অন্তর্ভাব "private.h"
#অন্তর্ভাব "websearch.h"

স্থির nserror
process_query_section(ধ্রুবক অক্ষর *str, size_t len, অক্ষর **term)
{
	যদি (len < 3) {
		ফেরত NSERROR_BAD_PARAMETER;
	}
	যদি (str[0] != 'q' || str[1] != '=') {
		ফেরত NSERROR_BAD_PARAMETER;
	}
	ফেরত url_unescape(str + 2, len - 2, ফাঁকা, term);
}

স্থির nserror
searchterm_from_query(কাঠামো nsurl *url, অক্ষর **term)
{
	nserror res;
	অক্ষর *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	জন্য (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		যখন (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, term);
		যদি (res == NSERROR_OK) {
			বিরতি;
		}
		kvlen++; /* account for & separator */
	}
	মুক্ত(querystr);

	ফেরত res;
}

bool fetch_about_websearch_handler(কাঠামো fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	অক্ষর *term;

	res = searchterm_from_query(fetch_about_get_url(ctx), &term);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	res = অনুসন্ধান_web_omni(term, অনুসন্ধান_WEB_OMNI_SEARCHONLY, &url);
	মুক্ত(term);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	ফেরত true;
}
