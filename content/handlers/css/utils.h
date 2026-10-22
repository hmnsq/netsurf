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

#ضمن <libcss/libcss.h>

#ضمن "netsurf/css.h"

/** DPI of the screen, in fixed point units */
خارجي css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ساكن مضمن uint8_t ns_computed_display(
		ثابت css_computed_style *style, bool root)
{
	uint8_t value = css_computed_display(style, root);

	بدل (value) {
	حالة CSS_DISPLAY_GRID:
		أرجع CSS_DISPLAY_BLOCK;

	حالة CSS_DISPLAY_INLINE_GRID:
		أرجع CSS_DISPLAY_INLINE_BLOCK;

	افتراضي:
		اكسر;
	}

	أرجع value;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ساكن مضمن uint8_t ns_computed_display_static(
		ثابت css_computed_style *style)
{
	uint8_t value = css_computed_display_static(style);

	بدل (value) {
	حالة CSS_DISPLAY_GRID:
		أرجع CSS_DISPLAY_BLOCK;

	حالة CSS_DISPLAY_INLINE_GRID:
		أرجع CSS_DISPLAY_INLINE_BLOCK;

	افتراضي:
		اكسر;
	}

	أرجع value;
}

ساكن مضمن uint8_t ns_computed_min_height(
		ثابت css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t value = css_computed_min_height(style, length, unit);

	𐤀𐤌 (value == CSS_MIN_HEIGHT_AUTO) {
		value = CSS_MIN_HEIGHT_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	أرجع value;
}


ساكن مضمن uint8_t ns_computed_min_width(
		ثابت css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t value = css_computed_min_width(style, length, unit);

	𐤀𐤌 (value == CSS_MIN_WIDTH_AUTO) {
		value = CSS_MIN_WIDTH_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	أرجع value;
}

#endif
