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

#ಅನ್ತರ್ಭಾವ <libcss/libcss.h>

#ಅನ್ತರ್ಭಾವ "netsurf/css.h"

/** DPI of the screen, in fixed point units */
ಬಾಹ್ಯ css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ uint8_t ns_computed_display(
		ಧ್ರುವ css_computed_style *style, bool ಬೇರು)
{
	uint8_t ಮೌಲ್ಯ = css_computed_display(style, ಬೇರು);

	ವಿಕಲ್ಪ (ಮೌಲ್ಯ) {
	ಪಕ್ಷ CSS_DISPLAY_GRID:
		ಪ್ರತ್ಯಾಗಮ CSS_DISPLAY_BLOCK;

	ಪಕ್ಷ CSS_DISPLAY_INLINE_GRID:
		ಪ್ರತ್ಯಾಗಮ CSS_DISPLAY_INLINE_BLOCK;

	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ uint8_t ns_computed_display_static(
		ಧ್ರುವ css_computed_style *style)
{
	uint8_t ಮೌಲ್ಯ = css_computed_display_static(style);

	ವಿಕಲ್ಪ (ಮೌಲ್ಯ) {
	ಪಕ್ಷ CSS_DISPLAY_GRID:
		ಪ್ರತ್ಯಾಗಮ CSS_DISPLAY_BLOCK;

	ಪಕ್ಷ CSS_DISPLAY_INLINE_GRID:
		ಪ್ರತ್ಯಾಗಮ CSS_DISPLAY_INLINE_BLOCK;

	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ uint8_t ns_computed_min_height(
		ಧ್ರುವ css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t ಮೌಲ್ಯ = css_computed_min_height(style, length, unit);

	ಯದಿ (ಮೌಲ್ಯ == CSS_MIN_HEIGHT_AUTO) {
		ಮೌಲ್ಯ = CSS_MIN_HEIGHT_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;
}


ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ uint8_t ns_computed_min_width(
		ಧ್ರುವ css_computed_style *style,
		css_fixed *length, css_unit *unit)
{
	uint8_t ಮೌಲ್ಯ = css_computed_min_width(style, length, unit);

	ಯದಿ (ಮೌಲ್ಯ == CSS_MIN_WIDTH_AUTO) {
		ಮೌಲ್ಯ = CSS_MIN_WIDTH_SET;
		*length = 0;
		*unit = CSS_UNIT_PX;
	}

	ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;
}

#endif
