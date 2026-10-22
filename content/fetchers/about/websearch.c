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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/url.h"

#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "desktop/searchweb.h"

#ಅನ್ತರ್ಭಾವ "private.h"
#ಅನ್ತರ್ಭಾವ "websearch.h"

ಸ್ಥಿರ nserror
process_query_section(ಧ್ರುವ ವರ್ಣ *str, size_t len, ವರ್ಣ **term)
{
	ಯದಿ (len < 3) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_BAD_PARAMETER;
	}
	ಯದಿ (str[0] != 'q' || str[1] != '=') {
		ಪ್ರತ್ಯಾಗಮ NSERROR_BAD_PARAMETER;
	}
	ಪ್ರತ್ಯಾಗಮ url_unescape(str + 2, len - 2, ರಿಕ್ತ, term);
}

ಸ್ಥಿರ nserror
searchterm_from_query(ಸಂರಚನಾ nsurl *url, ವರ್ಣ **term)
{
	nserror res;
	ವರ್ಣ *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	ಪ್ರತಿ (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		ಯಾವತ್ (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, term);
		ಯದಿ (res == NSERROR_OK) {
			ಭಙ್ಗ;
		}
		kvlen++; /* account for & separator */
	}
	ಮುಕ್ತ(querystr);

	ಪ್ರತ್ಯಾಗಮ res;
}

bool fetch_about_websearch_handler(ಸಂರಚನಾ fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	ವರ್ಣ *term;

	res = searchterm_from_query(fetch_about_get_url(ctx), &term);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	res = ಹುಡುಕು_web_omni(term, ಹುಡುಕು_WEB_OMNI_SEARCHONLY, &url);
	ಮುಕ್ತ(term);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	ಪ್ರತ್ಯಾಗಮ true;
}
