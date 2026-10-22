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

#ضمن <stdbool.h>
#ضمن <stdint.h>
#ضمن <sys/types.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "utils/errors.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/url.h"

#ضمن "content/fetch.h"
#ضمن "desktop/searchweb.h"

#ضمن "private.h"
#ضمن "websearch.h"

ساكن nserror
process_query_section(ثابت حرف *str, size_t len, حرف **term)
{
	إذا (len < 3) {
		إرجاع NSERROR_BAD_PARAMETER;
	}
	إذا (str[0] != 'q' || str[1] != '=') {
		إرجاع NSERROR_BAD_PARAMETER;
	}
	إرجاع url_unescape(str + 2, len - 2, عدم, term);
}

ساكن nserror
searchterm_from_query(بنية nsurl *url, حرف **term)
{
	nserror res;
	حرف *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	إذا (res != NSERROR_OK) {
		إرجاع res;
	}

	لكل (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		بينما (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, term);
		إذا (res == NSERROR_OK) {
			اكسر;
		}
		kvlen++; /* account for & separator */
	}
	حرر(querystr);

	إرجاع res;
}

bool fetch_about_websearch_handler(بنية fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	حرف *term;

	res = searchterm_from_query(fetch_about_get_url(ctx), &term);
	إذا (res != NSERROR_OK) {
		إرجاع false;
	}

	res = بحث_web_omni(term, بحث_WEB_OMNI_SEARCHONLY, &url);
	حرر(term);
	إذا (res != NSERROR_OK) {
		إرجاع false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	إرجاع true;
}
