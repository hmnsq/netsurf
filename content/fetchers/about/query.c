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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(ਬਣਤਰ nsurl *url,
		      ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ,
		      ਅੱਖਰ **ਬਾਹਰਲਾ_str)
{
	nserror res;
	ਅੱਖਰ *url_s;
	size_t url_l;
	ਅੱਖਰ *str = ਨਿਰਾ;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(ਕੁੰਜੀ, url_s);
	ਜੇ (str == ਨਿਰਾ) {
		res = NSERROR_NOMEM;
	} ਨਹੀਂਤਾਂ {
		*ਬਾਹਰਲਾ_str = str;
	}

	ਮੁਕਤ(url_s);

	ਵਾਪਸ res;
}
