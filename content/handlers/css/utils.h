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

#ଅନ୍ତର୍ଭାଵ <libcss/libcss.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/css.h"

/** DPI of the screen, in fixed point units */
ବାହ୍ୟ css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ uint8_t ns_computed_display(
		ଅଚଳ css_computed_style *style, bool ମୂଳ)
{
	uint8_t ମୂଲ୍ୟ = css_computed_display(style, ମୂଳ);

	ବାଛ (ମୂଲ୍ୟ) {
	ସ୍ଥିତି CSS_DISPLAY_GRID:
		ଫେରନ୍ତୁ CSS_DISPLAY_BLOCK;

	ସ୍ଥିତି CSS_DISPLAY_INLINE_GRID:
		ଫେରନ୍ତୁ CSS_DISPLAY_INLINE_BLOCK;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ ମୂଲ୍ୟ;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ uint8_t ns_computed_display_static(
		ଅଚଳ css_computed_style *style)
{
	uint8_t ମୂଲ୍ୟ = css_computed_display_static(style);

	ବାଛ (ମୂଲ୍ୟ) {
	ସ୍ଥିତି CSS_DISPLAY_GRID:
		ଫେରନ୍ତୁ CSS_DISPLAY_BLOCK;

	ସ୍ଥିତି CSS_DISPLAY_INLINE_GRID:
		ଫେରନ୍ତୁ CSS_DISPLAY_INLINE_BLOCK;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ ମୂଲ୍ୟ;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ uint8_t ns_computed_min_height(
		ଅଚଳ css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t ମୂଲ୍ୟ = css_computed_min_height(style, length, unit);

	ଯଦି (ମୂଲ୍ୟ == CSS_MIN_HEIGHT_AUTO) {
		ମୂଲ୍ୟ = CSS_MIN_HEIGHT_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	ଫେରନ୍ତୁ ମୂଲ୍ୟ;
}


ସ୍ଥିର ଧାଡ଼ିଭିତରେ uint8_t ns_computed_min_width(
		ଅଚଳ css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t ମୂଲ୍ୟ = css_computed_min_width(style, length, unit);

	ଯଦି (ମୂଲ୍ୟ == CSS_MIN_WIDTH_AUTO) {
		ମୂଲ୍ୟ = CSS_MIN_WIDTH_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	ଫେରନ୍ତୁ ମୂଲ୍ୟ;
}

#endif
