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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/url.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/searchweb.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "websearch.h"

ਸਥਿਰ nserror
process_query_section(ਅਚਲ ਅੱਖਰ *str, size_t len, ਅੱਖਰ **ਪਦ)
{
	ਜੇ (len < 3) {
		ਵਾਪਸ NSERROR_BAD_PARAMETER;
	}
	ਜੇ (str[0] != 'q' || str[1] != '=') {
		ਵਾਪਸ NSERROR_BAD_PARAMETER;
	}
	ਵਾਪਸ url_unescape(str + 2, len - 2, ਨਿਰਾ, ਪਦ);
}

ਸਥਿਰ nserror
searchterm_from_query(ਬਣਤਰ nsurl *url, ਅੱਖਰ **ਪਦ)
{
	nserror res;
	ਅੱਖਰ *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	ਲਈ (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		ਜਦੋਂ (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, ਪਦ);
		ਜੇ (res == NSERROR_OK) {
			ਰੋਕੋ;
		}
		kvlen++; /* account for & separator */
	}
	ਮੁਕਤ(querystr);

	ਵਾਪਸ res;
}

bool fetch_about_websearch_handler(ਬਣਤਰ fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	ਅੱਖਰ *ਪਦ;

	res = searchterm_from_query(fetch_about_get_url(ctx), &ਪਦ);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	res = ਖੋਜ_web_omni(ਪਦ, ਖੋਜ_WEB_OMNI_SEARCHONLY, &url);
	ਮੁਕਤ(ਪਦ);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	ਵਾਪਸ true;
}
