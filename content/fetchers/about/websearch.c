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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "utils/url.h"

#અન્તર્ભાવ "content/fetch.h"
#અન્તર્ભાવ "desktop/searchweb.h"

#અન્તર્ભાવ "private.h"
#અન્તર્ભાવ "websearch.h"

સ્થિર nserror
process_query_section(અચલ અક્ષર *str, size_t len, અક્ષર **term)
{
	જો (len < 3) {
		પાછા NSERROR_BAD_PARAMETER;
	}
	જો (str[0] != 'q' || str[1] != '=') {
		પાછા NSERROR_BAD_PARAMETER;
	}
	પાછા url_unescape(str + 2, len - 2, ખાલી, term);
}

સ્થિર nserror
searchterm_from_query(સંરચના nsurl *url, અક્ષર **term)
{
	nserror res;
	અક્ષર *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	માટે (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		જ્યારે (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, term);
		જો (res == NSERROR_OK) {
			તોડો;
		}
		kvlen++; /* account for & separator */
	}
	મુક્ત(querystr);

	પાછા res;
}

bool fetch_about_websearch_handler(સંરચના fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	અક્ષર *term;

	res = searchterm_from_query(fetch_about_get_url(ctx), &term);
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	res = શોધ_web_omni(term, શોધ_WEB_OMNI_SEARCHONLY, &url);
	મુક્ત(term);
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	પાછા true;
}
