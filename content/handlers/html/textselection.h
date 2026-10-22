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

#که_نه_تعریف_شوی NETSURF_HTML_TEXTSELECTION_H
#تعریف_کړه NETSURF_HTML_TEXTSELECTION_H

بنية content;
بنية selection;

nserror html_textselection_redraw(بنية content *c, بینښه پیل_idx, بینښه پای_ټکی_idx);

nserror html_textselection_copy(بنية content *c, بینښه پیل_idx, بینښه پای_ټکی_idx, بنية selection_string *selstr);

/**
 * get maximum index of text section.
 *
 * \param[in] c The content to measure
 * \param[out] end_idx pointer to value to recive result
 * \return NSERROR_OK and \a end_idx updated else error code
 */
nserror html_textselection_get_پای_ټکی(بنية content *c, بینښه *پای_ټکی_idx);

#د_که_پای
