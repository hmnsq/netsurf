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

#اگر_غیر_تعریف_شدہ NETSURF_CSS_UTILS_H_
#تعریف_کرو NETSURF_CSS_UTILS_H_

#شامل <libcss/libcss.h>

#شامل "netsurf/css.h"

/** DPI of the screen, in fixed point units */
خارجی css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ساکن اندرونی uint8_t ns_computed_display(
		ثابت css_computed_style *style, bool جڑ)
{
	uint8_t قدر = css_computed_display(style, جڑ);

	انتخاب (قدر) {
	حالت CSS_DISPLAY_GRID:
		واپس CSS_DISPLAY_بلاک;

	حالت CSS_DISPLAY_INLINE_GRID:
		واپس CSS_DISPLAY_INLINE_بلاک;

	طےشدہ:
		توڑو;
	}

	واپس قدر;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ساکن اندرونی uint8_t ns_computed_display_static(
		ثابت css_computed_style *style)
{
	uint8_t قدر = css_computed_display_static(style);

	انتخاب (قدر) {
	حالت CSS_DISPLAY_GRID:
		واپس CSS_DISPLAY_بلاک;

	حالت CSS_DISPLAY_INLINE_GRID:
		واپس CSS_DISPLAY_INLINE_بلاک;

	طےشدہ:
		توڑو;
	}

	واپس قدر;
}

ساکن اندرونی uint8_t ns_computed_min_اونچائی(
		ثابت css_computed_style *style,
		css_fixed *لمبائی, css_unit *unit)
{
	uint8_t قدر = css_computed_min_اونچائی(style, لمبائی, unit);

	اگر (قدر == CSS_MIN_اونچائی_AUTO) {
		قدر = CSS_MIN_اونچائی_سیٹ;
		*لمبائی = 0;
		*unit = CSS_UNIT_PX;
	}

	واپس قدر;
}


ساکن اندرونی uint8_t ns_computed_min_چوڑائی(
		ثابت css_computed_style *style,
		css_fixed *لمبائی, css_unit *unit)
{
	uint8_t قدر = css_computed_min_چوڑائی(style, لمبائی, unit);

	اگر (قدر == CSS_MIN_چوڑائی_AUTO) {
		قدر = CSS_MIN_چوڑائی_سیٹ;
		*لمبائی = 0;
		*unit = CSS_UNIT_PX;
	}

	واپس قدر;
}

#اگر_ختم
