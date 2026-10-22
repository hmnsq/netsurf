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

#समावेश <stdlib.h>

#समावेश "utils/errors.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/messages.h"

#समावेश "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(काष्ठा nsurl *url,
		      स्थिर अक्षर *कुंजी,
		      अक्षर **out_str)
{
	nserror res;
	अक्षर *url_s;
	माप_प्रकार url_l;
	अक्षर *str = शून्य;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(कुंजी, url_s);
	अगर (str == शून्य) {
		res = NSERROR_NOMEM;
	} अन्यथा {
		*out_str = str;
	}

	मुक्त(url_s);

	वापस res;
}
