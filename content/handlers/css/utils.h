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

#अन्तर्भाव <libcss/libcss.h>

#अन्तर्भाव "netsurf/css.h"

/** DPI of the screen, in fixed point units */
बाह्य css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
स्थिर ओळीत uint8_t ns_computed_display(
		स्थिरांक css_computed_style *style, bool मूळ)
{
	uint8_t मूल्य = css_computed_display(style, मूळ);

	निवड (मूल्य) {
	स्थिती CSS_DISPLAY_GRID:
		परत CSS_DISPLAY_BLOCK;

	स्थिती CSS_DISPLAY_INLINE_GRID:
		परत CSS_DISPLAY_INLINE_BLOCK;

	पूर्वनिर्धारित:
		थांबा;
	}

	परत मूल्य;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
स्थिर ओळीत uint8_t ns_computed_display_static(
		स्थिरांक css_computed_style *style)
{
	uint8_t मूल्य = css_computed_display_static(style);

	निवड (मूल्य) {
	स्थिती CSS_DISPLAY_GRID:
		परत CSS_DISPLAY_BLOCK;

	स्थिती CSS_DISPLAY_INLINE_GRID:
		परत CSS_DISPLAY_INLINE_BLOCK;

	पूर्वनिर्धारित:
		थांबा;
	}

	परत मूल्य;
}

स्थिर ओळीत uint8_t ns_computed_min_height(
		स्थिरांक css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t मूल्य = css_computed_min_height(style, length, unit);

	जर (मूल्य == CSS_MIN_HEIGHT_AUTO) {
		मूल्य = CSS_MIN_HEIGHT_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	परत मूल्य;
}


स्थिर ओळीत uint8_t ns_computed_min_width(
		स्थिरांक css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t मूल्य = css_computed_min_width(style, length, unit);

	जर (मूल्य == CSS_MIN_WIDTH_AUTO) {
		मूल्य = CSS_MIN_WIDTH_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	परत मूल्य;
}

#endif
