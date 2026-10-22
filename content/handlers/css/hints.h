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

#నిర్వచితం_కాకపోతే NETSURF_CSS_HINTS_H_
#నిర్వచించు NETSURF_CSS_HINTS_H_

#అన్తర్భావ <stdint.h>

#అన్తర్భావ <libcss/libcss.h>

nserror css_hint_init(శూన్య);
శూన్య css_hint_fini(శూన్య);

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
css_error కణుపు_presentational_hint(
		శూన్య *pw,
		శూన్య *కణుపు,
		uint32_t *nhints,
		css_hint **hints);

/**
 * Parser for colours specified in attribute values.
 *
 * \param data    Data to parse (NUL-terminated)
 * \param result  Pointer to location to receive resulting css_color
 * \return true on success, false on invalid input
 */
bool nscss_parse_రంగు(స్థిరరాశి అక్షరం *దత్తాంశం, css_color *ఫలితం);

#అయితే_ముగింపు
