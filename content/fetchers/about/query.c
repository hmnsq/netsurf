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

#شامل <stdlib.h>

#شامل "utils/errors.h"
#شامل "utils/nsurl.h"
#شامل "utils/messages.h"

#شامل "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(ساختار nsurl *url,
		      ثابت نویسه *کلید,
		      نویسه **out_str)
{
	nserror res;
	نویسه *url_s;
	size_t url_l;
	نویسه *str = پوچ;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(کلید, url_s);
	اگر (str == پوچ) {
		res = NSERROR_NOMEM;
	} وگرنه {
		*out_str = str;
	}

	آزاد(url_s);

	بازگشت res;
}
