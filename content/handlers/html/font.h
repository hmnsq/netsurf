/*
 * Copyright 2014 Vincent Sanders <vince@netsurf-browser.org>
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
 * Internal font handling interfaces.
 *
 * These functions provide font related services. They all work on
 * UTF-8 strings with lengths given.
 */

#यदि_अपरिभाषितम् NETSURF_HTML_FONT_H
#परिभाषय NETSURF_HTML_FONT_H

संरचना plot_font_style;

/**
 * Populate a font style using data from a computed CSS style
 *
 * \param unit_len_ctx  Length conversion context
 * \param css      Computed style to consider
 * \param fstyle   Font style to populate
 */
शून्य font_plot_style_from_css(ध्रुव css_unit_ctx *unit_len_ctx,
			      ध्रुव css_computed_style *css,
			      संरचना plot_font_style *fstyle);

#यदि_अन्तः
