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

#अगर_अघोषित NETSURF_CSS_UTILS_H_
#घोषणा NETSURF_CSS_UTILS_H_

#समावेश <libcss/libcss.h>

#समावेश "netsurf/css.h"

/** DPI of the screen, in fixed point units */
बाह्य css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
अटल अंतरभूत uint8_t ns_computed_display(
		स्थिर css_computed_style *style, bool मूल)
{
	uint8_t मूल्य = css_computed_display(style, मूल);

	चयन (मूल्य) {
	हाल CSS_DISPLAY_GRID:
		वापस CSS_DISPLAY_BLOCK;

	हाल CSS_DISPLAY_INLINE_GRID:
		वापस CSS_DISPLAY_INLINE_BLOCK;

	शेष:
		अवरोध;
	}

	वापस मूल्य;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
अटल अंतरभूत uint8_t ns_computed_display_static(
		स्थिर css_computed_style *style)
{
	uint8_t मूल्य = css_computed_display_static(style);

	चयन (मूल्य) {
	हाल CSS_DISPLAY_GRID:
		वापस CSS_DISPLAY_BLOCK;

	हाल CSS_DISPLAY_INLINE_GRID:
		वापस CSS_DISPLAY_INLINE_BLOCK;

	शेष:
		अवरोध;
	}

	वापस मूल्य;
}

अटल अंतरभूत uint8_t ns_computed_min_height(
		स्थिर css_computed_style *style,
		css_fixed *लंबाई, css_unit *unit)
{
	uint8_t मूल्य = css_computed_min_height(style, लंबाई, unit);

	अगर (मूल्य == CSS_MIN_HEIGHT_AUTO) {
		मूल्य = CSS_MIN_HEIGHT_SET;
		*लंबाई = 0;
		*unit = CSS_UNIT_PX;
	}

	वापस मूल्य;
}


अटल अंतरभूत uint8_t ns_computed_min_width(
		स्थिर css_computed_style *style,
		css_fixed *लंबाई, css_unit *unit)
{
	uint8_t मूल्य = css_computed_min_width(style, लंबाई, unit);

	अगर (मूल्य == CSS_MIN_WIDTH_AUTO) {
		मूल्य = CSS_MIN_WIDTH_SET;
		*लंबाई = 0;
		*unit = CSS_UNIT_PX;
	}

	वापस मूल्य;
}

#पूर्ण_अगर
