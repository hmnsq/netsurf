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
 * content generator for the about scheme query privacy page
 */

#include <stdlib.h>

#include "utils/errors.h"
#include "utils/nsurl.h"
#include "utils/messages.h"

#include "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(sa_nracanaa nsurl *url,
		      const var_na *key,
		      var_na **out_str)
{
	nserror res;
	var_na *url_s;
	size_t url_l;
	var_na *str = NULL;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	ถ้า (res != NSERROR_OK) {
		คืน res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(key, url_s);
	ถ้า (str == NULL) {
		res = NSERROR_NOMEM;
	} มิฉะนั้น {
		*out_str = str;
	}

	free(url_s);

	คืน res;
}
