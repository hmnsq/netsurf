/*
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
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

#అన్తర్భావ <string.h>
#అన్తర్భావ <libcss/libcss.h>

#అన్తర్భావ "utils/nsurl.h"

#అన్తర్భావ "css/internal.h"

/* exported interface documented in content/handlers/css/internal.h */
css_error nscss_resolve_url(శూన్య *pw, స్థిరరాశి అక్షరం *పునాది,
		lwc_string *rel, lwc_string **నిరపేక్ష)
{
	lwc_error lerror;
	nserror error;
	nsurl *nsbase;
	nsurl *nsabs;

	/* Create nsurl from base */
	/* TODO: avoid this */
	error = nsurl_create(పునాది, &nsbase);
	ఒకవేళ (error != NSERROR_OK) {
		తిరిగి error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	/* Resolve URI */
	error = nsurl_join(nsbase, lwc_string_దత్తాంశం(rel), &nsabs);
	ఒకవేళ (error != NSERROR_OK) {
		nsurl_unref(nsbase);
		తిరిగి error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsbase);

	/* Intern it */
	lerror = lwc_intern_string(nsurl_access(nsabs),
			nsurl_నిడివి(nsabs), నిరపేక్ష);
	ఒకవేళ (lerror != lwc_error_ok) {
		*నిరపేక్ష = ఖాళీ;
		nsurl_unref(nsabs);
		తిరిగి lerror == lwc_error_oom ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsabs);

	తిరిగి CSS_OK;
}
