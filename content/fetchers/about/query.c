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
get_query_description(ರಚನೆ nsurl *url,
		      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ,
		      ಅಕ್ಷರ **ಹೊರಹೋಗು_str)
{
	nserror res;
	ಅಕ್ಷರ *url_s;
	size_t url_l;
	ಅಕ್ಷರ *str = ಖಾಲಿ;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(ಕೀಲಿ, url_s);
	ಆದರೆ (str == ಖಾಲಿ) {
		res = NSERROR_NOMEM;
	} ಇಲ್ಲದಿದ್ದರೆ {
		*ಹೊರಹೋಗು_str = str;
	}

	ಬಿಡುಗಡೆ(url_s);

	ಹಿಂತಿರುಗಿ res;
}
