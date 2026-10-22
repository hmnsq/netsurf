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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_CSS_UTILS_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_CSS_UTILS_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <libcss/libcss.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/css.h"

/** DPI of the screen, in fixed point units */
ਬਾਹਰੀ css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ uint8_t ns_computed_display(
		ਅਚਲ css_computed_style *style, bool ਜੜ੍ਹ)
{
	uint8_t ਮੁੱਲ = css_computed_display(style, ਜੜ੍ਹ);

	ਚੋਣ (ਮੁੱਲ) {
	ਹਾਲਤ CSS_DISPLAY_GRID:
		ਵਾਪਸ CSS_DISPLAY_ਖੰਡ;

	ਹਾਲਤ CSS_DISPLAY_INLINE_GRID:
		ਵਾਪਸ CSS_DISPLAY_INLINE_ਖੰਡ;

	ਮੂਲ:
		ਰੋਕੋ;
	}

	ਵਾਪਸ ਮੁੱਲ;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ uint8_t ns_computed_display_static(
		ਅਚਲ css_computed_style *style)
{
	uint8_t ਮੁੱਲ = css_computed_display_static(style);

	ਚੋਣ (ਮੁੱਲ) {
	ਹਾਲਤ CSS_DISPLAY_GRID:
		ਵਾਪਸ CSS_DISPLAY_ਖੰਡ;

	ਹਾਲਤ CSS_DISPLAY_INLINE_GRID:
		ਵਾਪਸ CSS_DISPLAY_INLINE_ਖੰਡ;

	ਮੂਲ:
		ਰੋਕੋ;
	}

	ਵਾਪਸ ਮੁੱਲ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ uint8_t ns_computed_min_ਉਚਾਈ(
		ਅਚਲ css_computed_style *style,
		css_fixed *ਲੰਮਾਈ, css_unit *unit)
{
	uint8_t ਮੁੱਲ = css_computed_min_ਉਚਾਈ(style, ਲੰਮਾਈ, unit);

	ਜੇ (ਮੁੱਲ == CSS_MIN_ਉਚਾਈ_AUTO) {
		ਮੁੱਲ = CSS_MIN_ਉਚਾਈ_ਸੈੱਟ;
		*ਲੰਮਾਈ = 0;
		*unit = CSS_UNIT_PX;
	}

	ਵਾਪਸ ਮੁੱਲ;
}


ਸਥਿਰ ਸਤਰਅੰਦਰ uint8_t ns_computed_min_ਚੌੜਾਈ(
		ਅਚਲ css_computed_style *style,
		css_fixed *ਲੰਮਾਈ, css_unit *unit)
{
	uint8_t ਮੁੱਲ = css_computed_min_ਚੌੜਾਈ(style, ਲੰਮਾਈ, unit);

	ਜੇ (ਮੁੱਲ == CSS_MIN_ਚੌੜਾਈ_AUTO) {
		ਮੁੱਲ = CSS_MIN_ਚੌੜਾਈ_ਸੈੱਟ;
		*ਲੰਮਾਈ = 0;
		*unit = CSS_UNIT_PX;
	}

	ਵਾਪਸ ਮੁੱਲ;
}

#ਜੇ_ਅੰਤ
