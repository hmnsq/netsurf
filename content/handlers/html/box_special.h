/*
 * Copyright 2020 Vincent Sanders <vince@netsurf-browser.org>
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

/**
 * \file
 * HTML Box tree construction special element conversion interface.
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_HTML_BOX_SPECIAL_H
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_HTML_BOX_SPECIAL_H


/**
 * call an elements special conversion handler
 *
 * \return true if box construction should continue else false on error.
 */
bool convert_special_elements(dom_ಗಂಟು *ಗಂಟು, html_content *content, ರಚನೆ box *box, bool *convert_children);

#ಆದರೆ_ಅಂತ್ಯ
