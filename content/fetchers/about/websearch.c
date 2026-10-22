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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "utils/url.h"

#ଅନ୍ତର୍ଭାଵ "content/fetch.h"
#ଅନ୍ତର୍ଭାଵ "desktop/searchweb.h"

#ଅନ୍ତର୍ଭାଵ "private.h"
#ଅନ୍ତର୍ଭାଵ "websearch.h"

ସ୍ଥିର nserror
process_query_section(ଅଚଳ ଅକ୍ଷର *str, size_t len, ଅକ୍ଷର **term)
{
	ଯଦି (len < 3) {
		ଫେରନ୍ତୁ NSERROR_BAD_PARAMETER;
	}
	ଯଦି (str[0] != 'q' || str[1] != '=') {
		ଫେରନ୍ତୁ NSERROR_BAD_PARAMETER;
	}
	ଫେରନ୍ତୁ url_unescape(str + 2, len - 2, ଖାଲି, term);
}

ସ୍ଥିର nserror
searchterm_from_query(ସଂରଚନା nsurl *url, ଅକ୍ଷର **term)
{
	nserror res;
	ଅକ୍ଷର *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	ପାଇଁ (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		ଯେତେବେଳେ (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, term);
		ଯଦି (res == NSERROR_OK) {
			ଭାଙ୍ଗ;
		}
		kvlen++; /* account for & separator */
	}
	ମୁକ୍ତ(querystr);

	ଫେରନ୍ତୁ res;
}

bool fetch_about_websearch_handler(ସଂରଚନା fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	ଅକ୍ଷର *term;

	res = searchterm_from_query(fetch_about_get_url(ctx), &term);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	res = ସନ୍ଧାନ_web_omni(term, ସନ୍ଧାନ_WEB_OMNI_SEARCHONLY, &url);
	ମୁକ୍ତ(term);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	ଫେରନ୍ତୁ true;
}
