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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "utils/url.h"

#அந்தர்பாவ "content/fetch.h"
#அந்தர்பாவ "desktop/searchweb.h"

#அந்தர்பாவ "private.h"
#அந்தர்பாவ "websearch.h"

நிலையான nserror
process_query_section(மாறிலி எழுத்து *str, size_t len, எழுத்து **term)
{
	எனில் (len < 3) {
		திரும்பு NSERROR_BAD_PARAMETER;
	}
	எனில் (str[0] != 'q' || str[1] != '=') {
		திரும்பு NSERROR_BAD_PARAMETER;
	}
	திரும்பு url_unescape(str + 2, len - 2, வெற்று, term);
}

நிலையான nserror
searchterm_from_query(கட்டமைப்பு nsurl *url, எழுத்து **term)
{
	nserror res;
	எழுத்து *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	க்கு (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		போது (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, term);
		எனில் (res == NSERROR_OK) {
			நிறுத்து;
		}
		kvlen++; /* account for & separator */
	}
	விடுவி(querystr);

	திரும்பு res;
}

bool fetch_about_websearch_handler(கட்டமைப்பு fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	எழுத்து *term;

	res = searchterm_from_query(fetch_about_get_url(ctx), &term);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	res = தேடல்_web_omni(term, தேடல்_WEB_OMNI_SEARCHONLY, &url);
	விடுவி(term);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	திரும்பு true;
}
