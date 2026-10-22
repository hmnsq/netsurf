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
process_query_section(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, size_t len, ಅಕ್ಷರ **ಪದ)
{
	ಆದರೆ (len < 3) {
		ಹಿಂತಿರುಗಿ NSERROR_BAD_PARAMETER;
	}
	ಆದರೆ (str[0] != 'q' || str[1] != '=') {
		ಹಿಂತಿರುಗಿ NSERROR_BAD_PARAMETER;
	}
	ಹಿಂತಿರುಗಿ url_unescape(str + 2, len - 2, ಖಾಲಿ, ಪದ);
}

ಸ್ಥಿರ nserror
searchterm_from_query(ರಚನೆ nsurl *url, ಅಕ್ಷರ **ಪದ)
{
	nserror res;
	ಅಕ್ಷರ *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	ಗಾಗಿ (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		ಆಗ (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, ಪದ);
		ಆದರೆ (res == NSERROR_OK) {
			ನಿಲ್ಲಿಸು;
		}
		kvlen++; /* account for & separator */
	}
	ಬಿಡುಗಡೆ(querystr);

	ಹಿಂತಿರುಗಿ res;
}

bool fetch_about_websearch_handler(ರಚನೆ fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	ಅಕ್ಷರ *ಪದ;

	res = searchterm_from_query(fetch_about_get_url(ctx), &ಪದ);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	res = ಹುಡುಕು_web_omni(ಪದ, ಹುಡುಕು_WEB_OMNI_SEARCHONLY, &url);
	ಬಿಡುಗಡೆ(ಪದ);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	ಹಿಂತಿರುಗಿ true;
}
