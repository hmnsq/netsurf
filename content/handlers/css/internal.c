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

#অন্তর্ভাব <string.h>
#অন্তর্ভাব <libcss/libcss.h>

#অন্তর্ভাব "utils/nsurl.h"

#অন্তর্ভাব "css/internal.h"

/* exported interface documented in content/handlers/css/internal.h */
css_error nscss_resolve_url(শূন্য *pw, ধ্রুবক অক্ষর *base,
		lwc_string *rel, lwc_string **abs)
{
	lwc_error lerror;
	nserror error;
	nsurl *nsbase;
	nsurl *nsabs;

	/* Create nsurl from base */
	/* TODO: avoid this */
	error = nsurl_create(base, &nsbase);
	যদি (error != NSERROR_OK) {
		ফেরত error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	/* Resolve URI */
	error = nsurl_join(nsbase, lwc_string_তথ্য(rel), &nsabs);
	যদি (error != NSERROR_OK) {
		nsurl_unref(nsbase);
		ফেরত error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsbase);

	/* Intern it */
	lerror = lwc_intern_string(nsurl_access(nsabs),
			nsurl_দৈর্ঘ্য(nsabs), abs);
	যদি (lerror != lwc_error_ok) {
		*abs = ফাঁকা;
		nsurl_unref(nsabs);
		ফেরত lerror == lwc_error_oom ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsabs);

	ফেরত CSS_OK;
}
