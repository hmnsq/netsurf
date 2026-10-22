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

#অন্তর্ভাব <libcss/libcss.h>

#অন্তর্ভাব "netsurf/css.h"

/** DPI of the screen, in fixed point units */
বাহ্য css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
স্থির অন্তঃস্থ uint8_t ns_computed_display(
		ধ্রুব css_computed_style *style, bool মূল)
{
	uint8_t মান = css_computed_display(style, মূল);

	বিকল্প (মান) {
	পক্ষ CSS_DISPLAY_GRID:
		প্রত্যাগম CSS_DISPLAY_BLOCK;

	পক্ষ CSS_DISPLAY_INLINE_GRID:
		প্রত্যাগম CSS_DISPLAY_INLINE_BLOCK;

	স্বভাব:
		ভঙ্গ;
	}

	প্রত্যাগম মান;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
স্থির অন্তঃস্থ uint8_t ns_computed_display_static(
		ধ্রুব css_computed_style *style)
{
	uint8_t মান = css_computed_display_static(style);

	বিকল্প (মান) {
	পক্ষ CSS_DISPLAY_GRID:
		প্রত্যাগম CSS_DISPLAY_BLOCK;

	পক্ষ CSS_DISPLAY_INLINE_GRID:
		প্রত্যাগম CSS_DISPLAY_INLINE_BLOCK;

	স্বভাব:
		ভঙ্গ;
	}

	প্রত্যাগম মান;
}

স্থির অন্তঃস্থ uint8_t ns_computed_min_height(
		ধ্রুব css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t মান = css_computed_min_height(style, length, unit);

	যদি (মান == CSS_MIN_HEIGHT_AUTO) {
		মান = CSS_MIN_HEIGHT_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	প্রত্যাগম মান;
}


স্থির অন্তঃস্থ uint8_t ns_computed_min_width(
		ধ্রুব css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t মান = css_computed_min_width(style, length, unit);

	যদি (মান == CSS_MIN_WIDTH_AUTO) {
		মান = CSS_MIN_WIDTH_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	প্রত্যাগম মান;
}

#endif
