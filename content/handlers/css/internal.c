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

#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <libcss/libcss.h>

#അന്തര്ഭാവ "utils/nsurl.h"

#അന്തര്ഭാവ "css/internal.h"

/* exported interface documented in content/handlers/css/internal.h */
css_error nscss_resolve_url(ശൂന്യം *pw, സ്ഥിരം അക്ഷരം *base,
		lwc_string *rel, lwc_string **abs)
{
	lwc_error lerror;
	nserror error;
	nsurl *nsbase;
	nsurl *nsabs;

	/* Create nsurl from base */
	/* TODO: avoid this */
	error = nsurl_create(base, &nsbase);
	എങ്കിൽ (error != NSERROR_OK) {
		തിരികെ error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	/* Resolve URI */
	error = nsurl_join(nsbase, lwc_string_data(rel), &nsabs);
	എങ്കിൽ (error != NSERROR_OK) {
		nsurl_unref(nsbase);
		തിരികെ error == NSERROR_NOMEM ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsbase);

	/* Intern it */
	lerror = lwc_intern_string(nsurl_access(nsabs),
			nsurl_length(nsabs), abs);
	എങ്കിൽ (lerror != lwc_error_ok) {
		*abs = ശൂന്യ;
		nsurl_unref(nsabs);
		തിരികെ lerror == lwc_error_oom ? CSS_NOMEM : CSS_INVALID;
	}

	nsurl_unref(nsabs);

	തിരികെ CSS_OK;
}
