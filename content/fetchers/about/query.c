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

#ضمن <stdlib.h>

#ضمن "utils/errors.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/messages.h"

#ضمن "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(بنية nsurl *url,
		      ثابت حرف *کلید,
		      حرف **out_str)
{
	nserror res;
	حرف *url_s;
	size_t url_l;
	حرف *str = عدم;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(کلید, url_s);
	إذا (str == عدم) {
		res = NSERROR_NOMEM;
	} وإلا {
		*out_str = str;
	}

	حرر(url_s);

	أرجع res;
}
