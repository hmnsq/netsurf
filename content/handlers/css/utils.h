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

#यदि_अपरिभाषितम् NETSURF_CSS_UTILS_H_
#परिभाषय NETSURF_CSS_UTILS_H_

#अन्तर्भाव <libcss/libcss.h>

#अन्तर्भाव "netsurf/css.h"

/** DPI of the screen, in fixed point units */
बाह्य css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
स्थिर अन्तःस्थ uint8_t ns_computed_display(
		ध्रुव css_computed_style *style, bool मूलम्)
{
	uint8_t मानम् = css_computed_display(style, मूलम्);

	विकल्प (मानम्) {
	पक्ष CSS_DISPLAY_GRID:
		प्रत्यागम CSS_DISPLAY_खण्डम्;

	पक्ष CSS_DISPLAY_INLINE_GRID:
		प्रत्यागम CSS_DISPLAY_INLINE_खण्डम्;

	स्वभाव:
		भङ्ग;
	}

	प्रत्यागम मानम्;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
स्थिर अन्तःस्थ uint8_t ns_computed_display_static(
		ध्रुव css_computed_style *style)
{
	uint8_t मानम् = css_computed_display_static(style);

	विकल्प (मानम्) {
	पक्ष CSS_DISPLAY_GRID:
		प्रत्यागम CSS_DISPLAY_खण्डम्;

	पक्ष CSS_DISPLAY_INLINE_GRID:
		प्रत्यागम CSS_DISPLAY_INLINE_खण्डम्;

	स्वभाव:
		भङ्ग;
	}

	प्रत्यागम मानम्;
}

स्थिर अन्तःस्थ uint8_t ns_computed_min_height(
		ध्रुव css_computed_style *style,
		css_fixed *दीर्घता, css_unit *unit)
{
	uint8_t मानम् = css_computed_min_height(style, दीर्घता, unit);

	यदि (मानम् == CSS_MIN_HEIGHT_AUTO) {
		मानम् = CSS_MIN_HEIGHT_समुच्चयः;
		*दीर्घता = 0;
		*unit = CSS_UNIT_PX;
	}

	प्रत्यागम मानम्;
}


स्थिर अन्तःस्थ uint8_t ns_computed_min_विस्तारः(
		ध्रुव css_computed_style *style,
		css_fixed *दीर्घता, css_unit *unit)
{
	uint8_t मानम् = css_computed_min_विस्तारः(style, दीर्घता, unit);

	यदि (मानम् == CSS_MIN_विस्तारः_AUTO) {
		मानम् = CSS_MIN_विस्तारः_समुच्चयः;
		*दीर्घता = 0;
		*unit = CSS_UNIT_PX;
	}

	प्रत्यागम मानम्;
}

#यदि_अन्तः
