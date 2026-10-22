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

#समावेश <stdbool.h>
#समावेश <stdint.h>
#समावेश <sys/types.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "utils/errors.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/url.h"

#समावेश "content/fetch.h"
#समावेश "desktop/searchweb.h"

#समावेश "private.h"
#समावेश "websearch.h"

अटल nserror
process_query_section(स्थिर अक्षर *str, माप_प्रकार len, अक्षर **term)
{
	अगर (len < 3) {
		वापस NSERROR_BAD_PARAMETER;
	}
	अगर (str[0] != 'q' || str[1] != '=') {
		वापस NSERROR_BAD_PARAMETER;
	}
	वापस url_unescape(str + 2, len - 2, शून्य, term);
}

अटल nserror
searchterm_from_query(काष्ठा nsurl *url, अक्षर **term)
{
	nserror res;
	अक्षर *querystr;
	माप_प्रकार querylen;
	माप_प्रकार kvstart;/* key value start */
	माप_प्रकार kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	क्रम (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		जबतक (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, term);
		अगर (res == NSERROR_OK) {
			अवरोध;
		}
		kvlen++; /* account for & separator */
	}
	मुक्त(querystr);

	वापस res;
}

bool fetch_about_websearch_handler(काष्ठा fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	अक्षर *term;

	res = searchterm_from_query(fetch_about_get_url(ctx), &term);
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	res = खोज_web_omni(term, खोज_WEB_OMNI_SEARCHONLY, &url);
	मुक्त(term);
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	वापस true;
}
