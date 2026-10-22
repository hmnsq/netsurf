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

#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "utils/messages.h"

#അന്തര്ഭാവ "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(ഘടന nsurl *url,
		      സ്ഥിരം അക്ഷരം *താക്കോൽ,
		      അക്ഷരം **out_str)
{
	nserror res;
	അക്ഷരം *url_s;
	size_t url_l;
	അക്ഷരം *str = ശൂന്യ;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(താക്കോൽ, url_s);
	എങ്കിൽ (str == ശൂന്യ) {
		res = NSERROR_NOMEM;
	} അല്ലെങ്കിൽ {
		*out_str = str;
	}

	സ്വതന്ത്രം(url_s);

	തിരികെ res;
}
