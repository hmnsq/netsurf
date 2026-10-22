/*
 * Copyright 2013 Michael Drake <tlsa@netsurf-browser.org>
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
 * Box tree treeview box replacement (interface).
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_HTML_BOX_TEXTAREA_H
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_HTML_BOX_TEXTAREA_H

ರಚನೆ dom_ಗಂಟು;
ರಚನೆ html_content;
ರಚನೆ box;

/**
 * Create textarea widget for a form element
 *
 * \param html    html content object
 * \param box     box with gadget to be given textarea widget
 * \param node    DOM node for form element
 */
bool box_textarea_create_textarea(ರಚನೆ html_content *html,
		ರಚನೆ box *box, ರಚನೆ dom_ಗಂಟು *ಗಂಟು);


/**
 * Handle form textarea keypress input
 *
 * \param html    html content object
 * \param box     box with textarea widget
 * \param key     keypress
 * \return NSERROR_OK iff keypress handled
 */
nserror box_textarea_keypress(ರಚನೆ html_content *html, ರಚನೆ box *box, uint32_t ಕೀಲಿ);

#ಆದರೆ_ಅಂತ್ಯ
