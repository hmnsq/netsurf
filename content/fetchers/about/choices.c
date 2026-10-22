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
 * content generator for the about scheme blank page
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "netsurf/types.h"
#include "utils/errors.h"
#include "utils/nsoption.h"

#include "private.h"
#include "choices.h"

/**
 * Generate the text of a Choices file which represents the current
 * in use options.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_choices_handler(struct fetch_about_context *ctx)
{
	char buffer[1024];
	整数 code = 200;
	整数 slen;
	unsigned 整数 opt_loop = 0;
	整数 res = 0;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	如果 (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		goto fetch_about_choices_handler_aborted;

	slen = snprintf(buffer, sizeof buffer,
		 "# Automatically generated current NetSurf browser Choices\n");

	do {
		res = nsoption_snoptionf(buffer + slen,
				sizeof buffer - slen,
				opt_loop,
				"%k:%v\n");
		如果 (res <= 0)
			break; /* last option */

		如果 (res >= (整数) (sizeof buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (const uint8_t *)buffer, slen);
			如果 (res != NSERROR_OK) {
				goto fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} 否则 {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} 当 (res > 0);

	res = fetch_about_senddata(ctx, (const uint8_t *)buffer, slen);
	如果 (res != NSERROR_OK) {
		goto fetch_about_choices_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	返回 true;

fetch_about_choices_handler_aborted:
	返回 false;
}
