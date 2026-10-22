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

#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"

#ಅನ್ತರ್ಭಾವ "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(ಸಂರಚನಾ nsurl *url,
		      ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ,
		      ವರ್ಣ **out_str)
{
	nserror res;
	ವರ್ಣ *url_s;
	size_t url_l;
	ವರ್ಣ *str = ರಿಕ್ತ;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(ಕೀಲಿ, url_s);
	ಯದಿ (str == ರಿಕ್ತ) {
		res = NSERROR_NOMEM;
	} ಅನ್ಯಥಾ {
		*out_str = str;
	}

	ಮುಕ್ತ(url_s);

	ಪ್ರತ್ಯಾಗಮ res;
}
