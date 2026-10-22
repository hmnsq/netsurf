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

#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"

#ଅନ୍ତର୍ଭାଵ "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(ସଂରଚନା nsurl *url,
		      ଅଚଳ ଅକ୍ଷର *ଚାବି,
		      ଅକ୍ଷର **out_str)
{
	nserror res;
	ଅକ୍ଷର *url_s;
	size_t url_l;
	ଅକ୍ଷର *str = ଖାଲି;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(ଚାବି, url_s);
	ଯଦି (str == ଖାଲି) {
		res = NSERROR_NOMEM;
	} ଅନ୍ୟଥା {
		*out_str = str;
	}

	ମୁକ୍ତ(url_s);

	ଫେରନ୍ତୁ res;
}
