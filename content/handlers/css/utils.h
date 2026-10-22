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

#അന്തര്ഭാവ <libcss/libcss.h>

#അന്തര്ഭാവ "netsurf/css.h"

/** DPI of the screen, in fixed point units */
ബാഹ്യ css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
സ്ഥിതം വരിയിൽ uint8_t ns_computed_display(
		സ്ഥിരം css_computed_style *style, bool വേര്)
{
	uint8_t മൂല്യം = css_computed_display(style, വേര്);

	തിരഞ്ഞെടുപ്പ് (മൂല്യം) {
	അവസ്ഥ CSS_DISPLAY_GRID:
		തിരികെ CSS_DISPLAY_BLOCK;

	അവസ്ഥ CSS_DISPLAY_INLINE_GRID:
		തിരികെ CSS_DISPLAY_INLINE_BLOCK;

	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	തിരികെ മൂല്യം;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
സ്ഥിതം വരിയിൽ uint8_t ns_computed_display_static(
		സ്ഥിരം css_computed_style *style)
{
	uint8_t മൂല്യം = css_computed_display_static(style);

	തിരഞ്ഞെടുപ്പ് (മൂല്യം) {
	അവസ്ഥ CSS_DISPLAY_GRID:
		തിരികെ CSS_DISPLAY_BLOCK;

	അവസ്ഥ CSS_DISPLAY_INLINE_GRID:
		തിരികെ CSS_DISPLAY_INLINE_BLOCK;

	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	തിരികെ മൂല്യം;
}

സ്ഥിതം വരിയിൽ uint8_t ns_computed_min_height(
		സ്ഥിരം css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t മൂല്യം = css_computed_min_height(style, length, unit);

	എങ്കിൽ (മൂല്യം == CSS_MIN_HEIGHT_AUTO) {
		മൂല്യം = CSS_MIN_HEIGHT_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	തിരികെ മൂല്യം;
}


സ്ഥിതം വരിയിൽ uint8_t ns_computed_min_width(
		സ്ഥിരം css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t മൂല്യം = css_computed_min_width(style, length, unit);

	എങ്കിൽ (മൂല്യം == CSS_MIN_WIDTH_AUTO) {
		മൂല്യം = CSS_MIN_WIDTH_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	തിരികെ മൂല്യം;
}

#endif
