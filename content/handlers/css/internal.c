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

#شامل <string.h>
#شامل <libcss/libcss.h>

#شامل "utils/nsurl.h"

#شامل "css/internal.h"

/* exported interface documented in content/handlers/css/internal.h */
css_error nscss_resolve_url(تهی *pw, ثابت نویسه *base,
		lwc_string *rel, lwc_string **abs)
{
	lwc_error lerror;
	nserror error;
	nsurl *nsbase;
	nsurl *nsabs;

	/* Create nsurl from base */
	/* TODO: avoid this */
	error = nsurl_create(base, &nsbase);
	اگر (error != NSERROR_OK) {
		بازگشت error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	/* Resolve URI */
	error = nsurl_join(nsbase, lwc_string_data(rel), &nsabs);
	اگر (error != NSERROR_OK) {
		nsurl_unref(nsbase);
		بازگشت error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsbase);

	/* Intern it */
	lerror = lwc_intern_string(nsurl_access(nsabs),
			nsurl_length(nsabs), abs);
	اگر (lerror != lwc_error_ok) {
		*abs = پوچ;
		nsurl_unref(nsabs);
		بازگشت lerror == lwc_error_oom ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsabs);

	بازگشت CSS_OK;
}
