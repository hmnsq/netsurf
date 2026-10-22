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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/types.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "choices.h"

/**
 * Generate the text of a Choices file which represents the current
 * in use options.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_choices_handler(ਬਣਤਰ fetch_about_context *ctx)
{
	ਅੱਖਰ ਬਫਰ[1024];
	ਪੂਰਨ ਕੋਡ = 200;
	ਪੂਰਨ slen;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ opt_loop = 0;
	ਪੂਰਨ res = 0;

	/* content is going to return ok */
	fetch_about_ਸੈੱਟ_http_ਕੋਡ(ctx, ਕੋਡ);

	/* content type */
	ਜੇ (fetch_about_send_ਸਿਰਲੇਖ(ctx, "Content-Type: text/plain"))
		ਜਾਓ fetch_about_choices_handler_aborted;

	slen = snprintf(ਬਫਰ, ਆਕਾਰ ਬਫਰ,
		 "# Automatically generated current NetSurf browser Choices\n");

	ਕਰੋ {
		res = nsoption_snoptionf(ਬਫਰ + slen,
				ਆਕਾਰ ਬਫਰ - slen,
				opt_loop,
				"%k:%v\n");
		ਜੇ (res <= 0)
			ਰੋਕੋ; /* last option */

		ਜੇ (res >= (ਪੂਰਨ) (ਆਕਾਰ ਬਫਰ - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (ਅਚਲ uint8_t *)ਬਫਰ, slen);
			ਜੇ (res != NSERROR_OK) {
				ਜਾਓ fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} ਨਹੀਂਤਾਂ {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} ਜਦੋਂ (res > 0);

	res = fetch_about_senddata(ctx, (ਅਚਲ uint8_t *)ਬਫਰ, slen);
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ fetch_about_choices_handler_aborted;
	}

	fetch_about_send_ਮੁਕੰਮਲ(ctx);

	ਵਾਪਸ true;

fetch_about_choices_handler_aborted:
	ਵਾਪਸ false;
}
