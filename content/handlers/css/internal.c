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

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libcss/libcss.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "css/internal.h"

/* exported interface documented in content/handlers/css/internal.h */
css_error nscss_resolve_url(ਖਾਲੀ *pw, ਅਚਲ ਅੱਖਰ *ਅਧਾਰ,
		lwc_string *rel, lwc_string **ਨਿਰਪੇਖ)
{
	lwc_error lerror;
	nserror error;
	nsurl *nsbase;
	nsurl *nsabs;

	/* Create nsurl from base */
	/* TODO: avoid this */
	error = nsurl_create(ਅਧਾਰ, &nsbase);
	ਜੇ (error != NSERROR_OK) {
		ਵਾਪਸ error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	/* Resolve URI */
	error = nsurl_join(nsbase, lwc_string_ਅੰਕੜੇ(rel), &nsabs);
	ਜੇ (error != NSERROR_OK) {
		nsurl_unref(nsbase);
		ਵਾਪਸ error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsbase);

	/* Intern it */
	lerror = lwc_intern_string(nsurl_access(nsabs),
			nsurl_ਲੰਮਾਈ(nsabs), ਨਿਰਪੇਖ);
	ਜੇ (lerror != lwc_error_ok) {
		*ਨਿਰਪੇਖ = ਨਿਰਾ;
		nsurl_unref(nsabs);
		ਵਾਪਸ lerror == lwc_error_oom ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsabs);

	ਵਾਪਸ CSS_OK;
}
