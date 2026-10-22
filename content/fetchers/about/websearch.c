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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "utils/url.h"

#അന്തര്ഭാവ "content/fetch.h"
#അന്തര്ഭാവ "desktop/searchweb.h"

#അന്തര്ഭാവ "private.h"
#അന്തര്ഭാവ "websearch.h"

സ്ഥിതം nserror
process_query_section(സ്ഥിരം അക്ഷരം *str, size_t len, അക്ഷരം **term)
{
	എങ്കിൽ (len < 3) {
		തിരികെ NSERROR_BAD_PARAMETER;
	}
	എങ്കിൽ (str[0] != 'q' || str[1] != '=') {
		തിരികെ NSERROR_BAD_PARAMETER;
	}
	തിരികെ url_unescape(str + 2, len - 2, ശൂന്യ, term);
}

സ്ഥിതം nserror
searchterm_from_query(ഘടന nsurl *url, അക്ഷരം **term)
{
	nserror res;
	അക്ഷരം *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	വേണ്ടി (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		ആകുമ്പോൾ (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, term);
		എങ്കിൽ (res == NSERROR_OK) {
			നിർത്തുക;
		}
		kvlen++; /* account for & separator */
	}
	സ്വതന്ത്രം(querystr);

	തിരികെ res;
}

bool fetch_about_websearch_handler(ഘടന fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	അക്ഷരം *term;

	res = searchterm_from_query(fetch_about_get_url(ctx), &term);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	res = തിരയൽ_web_omni(term, തിരയൽ_WEB_OMNI_SEARCHONLY, &url);
	സ്വതന്ത്രം(term);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	തിരികെ true;
}
