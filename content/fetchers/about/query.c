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

#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/messages.h"

#अन्तर्भाव "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(संरचना nsurl *url,
		      ध्रुव वर्ण *कुञ्चिका,
		      वर्ण **out_str)
{
	nserror res;
	वर्ण *url_s;
	size_t url_l;
	वर्ण *str = रिक्त;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(कुञ्चिका, url_s);
	सचे (str == रिक्त) {
		res = NSERROR_NOMEM;
	} अञ्ञथा {
		*out_str = str;
	}

	मुक्त(url_s);

	पटिनिवत्त res;
}
