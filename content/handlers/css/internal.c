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

#अन्तर्भाव <string.h>
#अन्तर्भाव <libcss/libcss.h>

#अन्तर्भाव "utils/nsurl.h"

#अन्तर्भाव "css/internal.h"

/* exported interface documented in content/handlers/css/internal.h */
css_error nscss_resolve_url(शून्य *pw, स्थिरांक अक्षर *base,
		lwc_string *rel, lwc_string **असल)
{
	lwc_error lerror;
	nserror error;
	nsurl *nsbase;
	nsurl *nsabs;

	/* Create nsurl from base */
	/* TODO: avoid this */
	error = nsurl_create(base, &nsbase);
	जर (error != NSERROR_OK) {
		परत error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	/* Resolve URI */
	error = nsurl_join(nsbase, lwc_string_data(rel), &nsabs);
	जर (error != NSERROR_OK) {
		nsurl_unref(nsbase);
		परत error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsbase);

	/* Intern it */
	lerror = lwc_intern_string(nsurl_access(nsabs),
			nsurl_length(nsabs), असल);
	जर (lerror != lwc_error_ok) {
		*असल = रिक्त;
		nsurl_unref(nsabs);
		परत lerror == lwc_error_oom ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsabs);

	परत CSS_OK;
}
