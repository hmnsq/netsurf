/*
 * Copyright 2003 James Bursa <bursa@users.sourceforge.net>
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
 * interface to HTML layout.
 *
 * The main interface to the layout code is layout_document(), which takes a
 * normalized box tree and assigns coordinates and dimensions to the boxes, and
 * also adds boxes to the tree (eg. when formatting lines of text).
 */

#यदि_अपरिभाषितम् NETSURF_HTML_LAYOUT_H
#परिभाषय NETSURF_HTML_LAYOUT_H

संरचना box;
संरचना html_content;
संरचना gui_layout_सारणी;

/**
 * Calculate positions of boxes in a document.
 *
 * \param content content of type CONTENT_HTML
 * \param width available width
 * \param height available height
 * \return true on success, false on memory exhaustion
 */
bool layout_document(संरचना html_content *content, पूर्णाङ्क विस्तारः, पूर्णाङ्क height);

#यदि_अन्तः
