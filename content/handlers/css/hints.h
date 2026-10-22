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

#अगर_अघोषित NETSURF_CSS_HINTS_H_
#घोषणा NETSURF_CSS_HINTS_H_

#समावेश <stdint.h>

#समावेश <libcss/libcss.h>

nserror css_hint_init(व्योम);
व्योम css_hint_fini(व्योम);

/**
 * Callback to retrieve presentational hints for a node
 *
 * \param[in] pw HTML document
 * \param[in] node DOM node
 * \param[out] nhints number of hints retrieved
 * \param[out] hints retrieved hints
 * \return CSS_OK               on success,
 *         CSS_PROPERTY_NOT_SET if there is no hint for the requested property,
 *         CSS_NOMEM            on memory exhaustion.
 */
css_error गाँठ_presentational_hint(
		व्योम *pw,
		व्योम *गाँठ,
		uint32_t *nhints,
		css_hint **hints);

/**
 * Parser for colours specified in attribute values.
 *
 * \param data    Data to parse (NUL-terminated)
 * \param result  Pointer to location to receive resulting css_color
 * \return true on success, false on invalid input
 */
bool nscss_parse_colour(स्थिर अक्षर *तथ्य, css_color *परिणाम);

#पूर्ण_अगर
