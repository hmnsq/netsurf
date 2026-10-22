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

#অন্তর্ভাৱ <stdlib.h>

#অন্তর্ভাৱ "utils/errors.h"
#অন্তর্ভাৱ "utils/nsurl.h"
#অন্তর্ভাৱ "utils/messages.h"

#অন্তর্ভাৱ "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(সংরচনা nsurl *url,
		      ধ্রুৱ ৱর্ণ *চাবি,
		      ৱর্ণ **out_str)
{
	nserror res;
	ৱর্ণ *url_s;
	size_t url_l;
	ৱর্ণ *str = রিক্ত;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	যদি (res != NSERROR_OK) {
		ঘূৰাই res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(চাবি, url_s);
	যদি (str == রিক্ত) {
		res = NSERROR_NOMEM;
	} অন্যথা {
		*out_str = str;
	}

	মুক্ত(url_s);

	ঘূৰাই res;
}
