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

#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stdint.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "utils/errors.h"
#অন্তর্ভাৱ "utils/nsurl.h"
#অন্তর্ভাৱ "utils/url.h"

#অন্তর্ভাৱ "content/fetch.h"
#অন্তর্ভাৱ "desktop/searchweb.h"

#অন্তর্ভাৱ "private.h"
#অন্তর্ভাৱ "websearch.h"

স্থির nserror
process_query_section(ধ্রুৱ ৱর্ণ *str, size_t len, ৱর্ণ **term)
{
	যদি (len < 3) {
		ঘূৰাই NSERROR_BAD_PARAMETER;
	}
	যদি (str[0] != 'q' || str[1] != '=') {
		ঘূৰাই NSERROR_BAD_PARAMETER;
	}
	ঘূৰাই url_unescape(str + 2, len - 2, রিক্ত, term);
}

স্থির nserror
searchterm_from_query(সংরচনা nsurl *url, ৱর্ণ **term)
{
	nserror res;
	ৱর্ণ *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	বাবে (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		যেতিয়ালৈ (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, term);
		যদি (res == NSERROR_OK) {
			ভঙ্গ;
		}
		kvlen++; /* account for & separator */
	}
	মুক্ত(querystr);

	ঘূৰাই res;
}

bool fetch_about_websearch_handler(সংরচনা fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	ৱর্ণ *term;

	res = searchterm_from_query(fetch_about_get_url(ctx), &term);
	যদি (res != NSERROR_OK) {
		ঘূৰাই false;
	}

	res = অনুসন্ধান_web_omni(term, অনুসন্ধান_WEB_OMNI_SEARCHONLY, &url);
	মুক্ত(term);
	যদি (res != NSERROR_OK) {
		ঘূৰাই false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	ঘূৰাই true;
}
