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

#ifndef NETSURF_CSS_UTILS_H_
#define NETSURF_CSS_UTILS_H_

#અન્તર્ભાવ <libcss/libcss.h>

#અન્તર્ભાવ "netsurf/css.h"

/** DPI of the screen, in fixed point units */
બાહ્ય css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
સ્થિર અંતઃરેખ uint8_t ns_computed_display(
		અચલ css_computed_style *style, bool મૂળ)
{
	uint8_t કિંમત = css_computed_display(style, મૂળ);

	પસંદગી (કિંમત) {
	સ્થિતિ CSS_DISPLAY_GRID:
		પાછા CSS_DISPLAY_BLOCK;

	સ્થિતિ CSS_DISPLAY_INLINE_GRID:
		પાછા CSS_DISPLAY_INLINE_BLOCK;

	મૂળભૂત:
		તોડો;
	}

	પાછા કિંમત;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
સ્થિર અંતઃરેખ uint8_t ns_computed_display_static(
		અચલ css_computed_style *style)
{
	uint8_t કિંમત = css_computed_display_static(style);

	પસંદગી (કિંમત) {
	સ્થિતિ CSS_DISPLAY_GRID:
		પાછા CSS_DISPLAY_BLOCK;

	સ્થિતિ CSS_DISPLAY_INLINE_GRID:
		પાછા CSS_DISPLAY_INLINE_BLOCK;

	મૂળભૂત:
		તોડો;
	}

	પાછા કિંમત;
}

સ્થિર અંતઃરેખ uint8_t ns_computed_min_height(
		અચલ css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t કિંમત = css_computed_min_height(style, length, unit);

	જો (કિંમત == CSS_MIN_HEIGHT_AUTO) {
		કિંમત = CSS_MIN_HEIGHT_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	પાછા કિંમત;
}


સ્થિર અંતઃરેખ uint8_t ns_computed_min_width(
		અચલ css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t કિંમત = css_computed_min_width(style, length, unit);

	જો (કિંમત == CSS_MIN_WIDTH_AUTO) {
		કિંમત = CSS_MIN_WIDTH_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	પાછા કિંમત;
}

#endif
