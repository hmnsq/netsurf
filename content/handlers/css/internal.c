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

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <libcss/libcss.h>

#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"

#ಅನ್ತರ್ಭಾವ "css/internal.h"

/* exported interface documented in content/handlers/css/internal.h */
css_error nscss_resolve_url(ಶೂನ್ಯ *pw, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ತಳ,
		lwc_string *rel, lwc_string **ನಿರಪೇಕ್ಷ)
{
	lwc_error lerror;
	nserror error;
	nsurl *nsbase;
	nsurl *nsabs;

	/* Create nsurl from base */
	/* TODO: avoid this */
	error = nsurl_create(ತಳ, &nsbase);
	ಆದರೆ (error != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	/* Resolve URI */
	error = nsurl_join(nsbase, lwc_string_ದತ್ತಾಂಶ(rel), &nsabs);
	ಆದರೆ (error != NSERROR_OK) {
		nsurl_unref(nsbase);
		ಹಿಂತಿರುಗಿ error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsbase);

	/* Intern it */
	lerror = lwc_intern_string(nsurl_access(nsabs),
			nsurl_ಉದ್ದಳತೆ(nsabs), ನಿರಪೇಕ್ಷ);
	ಆದರೆ (lerror != lwc_error_ok) {
		*ನಿರಪೇಕ್ಷ = ಖಾಲಿ;
		nsurl_unref(nsabs);
		ಹಿಂತಿರುಗಿ lerror == lwc_error_oom ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsabs);

	ಹಿಂತಿರುಗಿ CSS_OK;
}
