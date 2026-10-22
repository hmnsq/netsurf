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
 * HTML text selection handling
 */

#ifndef NETSURF_HTML_TEXTSELECTION_H
#define NETSURF_HTML_TEXTSELECTION_H

ساختار content;
ساختار selection;

nserror html_textselection_redraw(ساختار content *c, unsigned start_idx, unsigned end_idx);

nserror html_textselection_copy(ساختار content *c, unsigned start_idx, unsigned end_idx, ساختار selection_string *selstr);

/**
 * get maximum index of text section.
 *
 * \param[in] c The content to measure
 * \param[out] end_idx pointer to value to recive result
 * \return NSERROR_OK and \a end_idx updated else error code
 */
nserror html_textselection_get_end(ساختار content *c, unsigned *end_idx);

#endif
