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

#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "utils/messages.h"

#અન્તર્ભાવ "query.h"

/* exported interface documented in about/query.h */
nserror
get_query_description(સંરચના nsurl *url,
		      અચલ અક્ષર *ચાવી,
		      અક્ષર **out_str)
{
	nserror res;
	અક્ષર *url_s;
	size_t url_l;
	અક્ષર *str = ખાલી;

	/* get the host in question */
	res = nsurl_get(url, NSURL_HOST, &url_s, &url_l);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	/* obtain the description with the url substituted */
	str = messages_get_buff(ચાવી, url_s);
	જો (str == ખાલી) {
		res = NSERROR_NOMEM;
	} નહીંતર {
		*out_str = str;
	}

	મુક્ત(url_s);

	પાછા res;
}
