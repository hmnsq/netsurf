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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "netsurf/plot_style.h"

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/nscolour.h"

#অন্তর্ভাব "private.h"
#অন্তর্ভাব "nscolours.h"

/**
 * Handler to generate the nscolours stylesheet
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_nscolours_handler(কাঠামো fetch_about_context *ctx)
{
	nserror res;
	ধ্রুবক অক্ষর *stylesheet;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	যদি (fetch_about_send_header(ctx, "Content-Type: text/css; charset=utf-8")) {
		যাও aborted;
	}

	res = nscolour_get_stylesheet(&stylesheet);
	যদি (res != NSERROR_OK) {
		যাও aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			"html {\n"
			"\tbackground-color: #%06x;\n"
			"}\n"
			"%s",
			colour_rb_swap(nscolours[NSCOLOUR_WIN_ODD_BG]),
			stylesheet);
	যদি (res != NSERROR_OK) {
		যাও aborted;
	}

	fetch_about_send_finished(ctx);

	ফেরত true;

aborted:

	ফেরত false;
}
