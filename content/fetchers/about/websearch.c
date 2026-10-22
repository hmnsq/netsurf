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
process_query_section(const char *str, size_t len, char **term)
{
	如果 (len < 3) {
		返回 NSERROR_BAD_PARAMETER;
	}
	如果 (str[0] != 'q' || str[1] != '=') {
		返回 NSERROR_BAD_PARAMETER;
	}
	返回 url_unescape(str + 2, len - 2, NULL, term);
}

static nserror
searchterm_from_query(struct nsurl *url, char **term)
{
	nserror res;
	char *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	如果 (res != NSERROR_OK) {
		返回 res;
	}

	为 (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		当 (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, term);
		如果 (res == NSERROR_OK) {
			break;
		}
		kvlen++; /* account for & separator */
	}
	free(querystr);

	返回 res;
}

bool fetch_about_websearch_handler(struct fetch_about_context *ctx)
{
	nserror res;
	nsurl *url;
	char *term;

	res = searchterm_from_query(fetch_about_get_url(ctx), &term);
	如果 (res != NSERROR_OK) {
		返回 false;
	}

	res = search_web_omni(term, SEARCH_WEB_OMNI_SEARCHONLY, &url);
	free(term);
	如果 (res != NSERROR_OK) {
		返回 false;
	}

	fetch_about_redirect(ctx, nsurl_access(url));
	nsurl_unref(url);

	返回 true;
}
