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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/url.h"

#अन्तर्भाव "content/fetch.h"
#अन्तर्भाव "desktop/searchweb.h"

#अन्तर्भाव "private.h"
#अन्तर्भाव "websearch.h"

स्थिर nserror
process_query_section(ध्रुव वर्ण *str, size_t len, वर्ण **term)
{
	यदि (len < 3) {
		प्रत्यागम NSERROR_BAD_PARAMETER;
	}
	यदि (str[0] != 'q' || str[1] != '=') {
		प्रत्यागम NSERROR_BAD_PARAMETER;
	}
	प्रत्यागम url_unescape(str + 2, len - 2, रिक्त, term);
}

स्थिर nserror
searchterm_from_query(संरचना nsurl *url, वर्ण **term)
{
	nserror res;
	वर्ण *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	प्रति (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		यावत् (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, term);
		यदि (res == NSERROR_OK) {
			भङ्ग;
		}
		kvlen++; /* account for & separator */
	}
	मुक्त(querystr);

	प्रत्यागम res;
}

bool fetch_about_websearch_handler(संरचना fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	वर्ण *term;

	res = searchterm_from_query(fetch_about_get_url(ctx), &term);
	यदि (res != NSERROR_OK) {
		प्रत्यागम false;
	}

	res = शोध_web_omni(term, शोध_WEB_OMNI_SEARCHONLY, &url);
	मुक्त(term);
	यदि (res != NSERROR_OK) {
		प्रत्यागम false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	प्रत्यागम true;
}
