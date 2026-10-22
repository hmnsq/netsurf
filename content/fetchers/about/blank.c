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
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "netsurf/types.h"
#అన్తర్భావ "utils/errors.h"

#అన్తర్భావ "private.h"
#అన్తర్భావ "blank.h"

/**
 * Handler to generate about scheme cache page.
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_blank_handler(నిర్మాణం fetch_about_context *ctx)
{
	స్థిరరాశి అక్షరం బఫరు[2] = { ' ', '\0' };

	/* content is going to return ok */
	fetch_about_సమితి_http_సంకేతం(ctx, 200);

	/* content type */
	ఒకవేళ (fetch_about_send_శీర్షిక(ctx, "Content-Type: text/html"))
		వెళ్ళు fetch_about_blank_handler_aborted;

	ఒకవేళ (fetch_about_senddata(ctx, (స్థిరరాశి uint8_t *) బఫరు, అక్షరశ్రేణి_పొడవు(బఫరు)) != NSERROR_OK)
		వెళ్ళు fetch_about_blank_handler_aborted;

	fetch_about_send_ముగిసిన(ctx);

	తిరిగి true;

fetch_about_blank_handler_aborted:
	తిరిగి false;
}
