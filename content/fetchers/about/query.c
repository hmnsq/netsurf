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

#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/messages.h"

#అన్తర్భావ "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(నిర్మాణం nsurl *url,
		      స్థిరరాశి అక్షరం *కీలకం,
		      అక్షరం **బహిర్గతం_str)
{
	nserror res;
	అక్షరం *url_s;
	size_t url_l;
	అక్షరం *str = ఖాళీ;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(కీలకం, url_s);
	ఒకవేళ (str == ఖాళీ) {
		res = NSERROR_NOMEM;
	} లేకపోతే {
		*బహిర్గతం_str = str;
	}

	విడుదల(url_s);

	తిరిగి res;
}
