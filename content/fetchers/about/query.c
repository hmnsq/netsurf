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

#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "utils/messages.h"

#অন্তর্ভাব "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(কাঠামো nsurl *url,
		      ধ্রুবক অক্ষর *চাবি,
		      অক্ষর **out_str)
{
	nserror res;
	অক্ষর *url_s;
	size_t url_l;
	অক্ষর *str = ফাঁকা;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(চাবি, url_s);
	যদি (str == ফাঁকা) {
		res = NSERROR_NOMEM;
	} অন্যথা {
		*out_str = str;
	}

	মুক্ত(url_s);

	ফেরত res;
}
