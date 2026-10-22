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
bool fetch_about_choices_handler(sa_nracanaa fetch_about_context *ctx)
{
	var_na buffer[1024];
	ຈຳນວນເຕັມ code = 200;
	ຈຳນວນເຕັມ slen;
	unsigned ຈຳນວນເຕັມ opt_loop = 0;
	ຈຳນວນເຕັມ res = 0;

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, code);

	/* content type */
	ຖ້າ (fetch_about_send_header(ctx, "Content-Type: text/plain"))
		goto fetch_about_choices_handler_aborted;

	slen = snprintf(buffer, parimaa_na buffer,
		 "# Automatically generated current NetSurf browser Choices\n");

	do {
		res = nsoption_snoptionf(buffer + slen,
				parimaa_na buffer - slen,
				opt_loop,
				"%k:%v\n");
		ຖ້າ (res <= 0)
			bha_nga; /* last option */

		ຖ້າ (res >= (ຈຳນວນເຕັມ) (parimaa_na buffer - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (const uint8_t *)buffer, slen);
			ຖ້າ (res != NSERROR_OK) {
				goto fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} ບໍ່ດັ່ງນັ້ນ {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} ໃນຂະນະທີ່ (res > 0);

	res = fetch_about_senddata(ctx, (const uint8_t *)buffer, slen);
	ຖ້າ (res != NSERROR_OK) {
		goto fetch_about_choices_handler_aborted;
	}

	fetch_about_send_finished(ctx);

	ສົ່ງຄືນ true;

fetch_about_choices_handler_aborted:
	ສົ່ງຄືນ false;
}
