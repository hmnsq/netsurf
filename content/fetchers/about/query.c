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

#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "utils/messages.h"

#அந்தர்பாவ "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(கட்டமைப்பு nsurl *url,
		      மாறிலி எழுத்து *சாவி,
		      எழுத்து **out_str)
{
	nserror res;
	எழுத்து *url_s;
	size_t url_l;
	எழுத்து *str = வெற்று;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(சாவி, url_s);
	எனில் (str == வெற்று) {
		res = NSERROR_NOMEM;
	} இல்லையெனில் {
		*out_str = str;
	}

	விடுவி(url_s);

	திரும்பு res;
}
