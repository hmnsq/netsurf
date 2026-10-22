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

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "utils/errors.h"
#include "utils/nsurl.h"
#include "utils/url.h"

#include "content/fetch.h"
#include "desktop/searchweb.h"

#include "private.h"
#include "websearch.h"

static nserror
process_query_section(const var_na *str, size_t len, var_na **term)
{
	නම් (len < 3) {
		ආපසු NSERROR_BAD_PARAMETER;
	}
	නම් (str[0] != 'q' || str[1] != '=') {
		ආපසු NSERROR_BAD_PARAMETER;
	}
	ආපසු url_unescape(str + 2, len - 2, NULL, term);
}

static nserror
searchterm_from_query(sa_nracanaa nsurl *url, var_na **term)
{
	nserror res;
	var_na *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	නම් (res != NSERROR_OK) {
		ආපසු res;
	}

	සඳහා (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		අතරතුර (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, term);
		නම් (res == NSERROR_OK) {
			bha_nga;
		}
		kvlen++; /* account for & separator */
	}
	free(querystr);

	ආපසු res;
}

bool fetch_about_websearch_handler(sa_nracanaa fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	var_na *term;

	res = searchterm_from_query(fetch_about_get_url(ctx), &term);
	නම් (res != NSERROR_OK) {
		ආපසු false;
	}

	res = search_web_omni(term, SEARCH_WEB_OMNI_SEARCHONLY, &url);
	free(term);
	නම් (res != NSERROR_OK) {
		ආපසු false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	ආපසු true;
}
