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

#شامل <stdbool.h>
#شامل <stddef.h>
#شامل <stdio.h>
#شامل <string.h>
#شامل <stdint.h>
#شامل <stdlib.h>

#شامل "netsurf/plot_style.h"

#شامل "utils/errors.h"
#شامل "utils/nscolour.h"

#شامل "private.h"
#شامل "nscolours.h"

/**
 * Handler to generate the nscolours stylesheet
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_nscolours_handler(ساخت fetch_about_context *ctx)
{
	nserror res;
	ثابت حرف *stylesheet;

	/* content is going to return ok */
	fetch_about_سیٹ_http_رمز(ctx, 200);

	/* content type */
	اگر (fetch_about_send_سرخی(ctx, "Content-Type: text/css; charset=utf-8")) {
		جاؤ aborted;
	}

	res = nscolour_get_stylesheet(&stylesheet);
	اگر (res != NSERROR_OK) {
		جاؤ aborted;
	}

	res = fetch_about_ssenddataf(ctx,
			"html {\n"
			"\tbackground-color: #%06x;\n"
			"}\n"
			"%s",
			رنگ_rb_تبادلہ(nscolours[NSCOLOUR_WIN_ODD_BG]),
			stylesheet);
	اگر (res != NSERROR_OK) {
		جاؤ aborted;
	}

	fetch_about_send_ختم_شدہ(ctx);

	واپس true;

aborted:

	واپس false;
}
