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

#அந்தர்பாவ <libcss/libcss.h>

#அந்தர்பாவ "netsurf/css.h"

/** DPI of the screen, in fixed point units */
வெளி css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
நிலையான உள்வரி uint8_t ns_computed_display(
		மாறிலி css_computed_style *style, bool வேர்)
{
	uint8_t மதிப்பு = css_computed_display(style, வேர்);

	தேர்வு (மதிப்பு) {
	நிலை CSS_DISPLAY_GRID:
		திரும்பு CSS_DISPLAY_BLOCK;

	நிலை CSS_DISPLAY_INLINE_GRID:
		திரும்பு CSS_DISPLAY_INLINE_BLOCK;

	இயல்பு:
		நிறுத்து;
	}

	திரும்பு மதிப்பு;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
நிலையான உள்வரி uint8_t ns_computed_display_static(
		மாறிலி css_computed_style *style)
{
	uint8_t மதிப்பு = css_computed_display_static(style);

	தேர்வு (மதிப்பு) {
	நிலை CSS_DISPLAY_GRID:
		திரும்பு CSS_DISPLAY_BLOCK;

	நிலை CSS_DISPLAY_INLINE_GRID:
		திரும்பு CSS_DISPLAY_INLINE_BLOCK;

	இயல்பு:
		நிறுத்து;
	}

	திரும்பு மதிப்பு;
}

நிலையான உள்வரி uint8_t ns_computed_min_height(
		மாறிலி css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t மதிப்பு = css_computed_min_height(style, length, unit);

	எனில் (மதிப்பு == CSS_MIN_HEIGHT_AUTO) {
		மதிப்பு = CSS_MIN_HEIGHT_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	திரும்பு மதிப்பு;
}


நிலையான உள்வரி uint8_t ns_computed_min_width(
		மாறிலி css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t மதிப்பு = css_computed_min_width(style, length, unit);

	எனில் (மதிப்பு == CSS_MIN_WIDTH_AUTO) {
		மதிப்பு = CSS_MIN_WIDTH_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	திரும்பு மதிப்பு;
}

#endif
