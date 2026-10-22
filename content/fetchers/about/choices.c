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

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdio.h>

#అన్తర్భావ "netsurf/types.h"
#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/nsoption.h"

#అన్తర్భావ "private.h"
#అన్తర్భావ "choices.h"

/**
 * Generate the text of a Choices file which represents the current
 * in use options.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_choices_handler(నిర్మాణం fetch_about_context *ctx)
{
	అక్షరం బఫరు[1024];
	పూర్ణాంకం సంకేతం_2 = 200;
	పూర్ణాంకం slen;
	గుర్తులేని పూర్ణాంకం opt_loop = 0;
	పూర్ణాంకం res = 0;

	/* content is going to return ok */
	fetch_about_సమితి_http_సంకేతం(ctx, సంకేతం_2);

	/* content type */
	ఒకవేళ (fetch_about_send_శీర్షిక(ctx, "Content-Type: text/plain"))
		వెళ్ళు fetch_about_choices_handler_aborted;

	slen = snprintf(బఫరు, పరిమాణం బఫరు,
		 "# Automatically generated current NetSurf browser Choices\n");

	చేయి {
		res = nsoption_snoptionf(బఫరు + slen,
				పరిమాణం బఫరు - slen,
				opt_loop,
				"%k:%v\n");
		ఒకవేళ (res <= 0)
			ఆపు; /* last option */

		ఒకవేళ (res >= (పూర్ణాంకం) (పరిమాణం బఫరు - slen)) {
			/* last entry would not fit in buffer, submit buffer */
			res = fetch_about_senddata(ctx, (స్థిరరాశి uint8_t *)బఫరు, slen);
			ఒకవేళ (res != NSERROR_OK) {
				వెళ్ళు fetch_about_choices_handler_aborted;
			}
			slen = 0;
		} లేకపోతే {
			/* normal addition */
			slen += res;
			opt_loop++;
		}
	} అయితే (res > 0);

	res = fetch_about_senddata(ctx, (స్థిరరాశి uint8_t *)బఫరు, slen);
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు fetch_about_choices_handler_aborted;
	}

	fetch_about_send_ముగిసిన(ctx);

	తిరిగి true;

fetch_about_choices_handler_aborted:
	తిరిగి false;
}
