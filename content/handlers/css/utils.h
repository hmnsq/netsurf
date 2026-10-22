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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_CSS_UTILS_H_
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_CSS_UTILS_H_

#ಅನ್ತರ್ಭಾವ <libcss/libcss.h>

#ಅನ್ತರ್ಭಾವ "netsurf/css.h"

/** DPI of the screen, in fixed point units */
ಬಾಹ್ಯ css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint8_t ns_computed_display(
		ಸ್ಥಿರಾಂಕ css_computed_style *style, bool ಬೇರು)
{
	uint8_t ಮೌಲ್ಯ = css_computed_display(style, ಬೇರು);

	ಆಯ್ಕೆ (ಮೌಲ್ಯ) {
	ಸ್ಥಿತಿ CSS_DISPLAY_GRID:
		ಹಿಂತಿರುಗಿ CSS_DISPLAY_ಖಂಡ;

	ಸ್ಥಿತಿ CSS_DISPLAY_INLINE_GRID:
		ಹಿಂತಿರುಗಿ CSS_DISPLAY_INLINE_ಖಂಡ;

	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint8_t ns_computed_display_static(
		ಸ್ಥಿರಾಂಕ css_computed_style *style)
{
	uint8_t ಮೌಲ್ಯ = css_computed_display_static(style);

	ಆಯ್ಕೆ (ಮೌಲ್ಯ) {
	ಸ್ಥಿತಿ CSS_DISPLAY_GRID:
		ಹಿಂತಿರುಗಿ CSS_DISPLAY_ಖಂಡ;

	ಸ್ಥಿತಿ CSS_DISPLAY_INLINE_GRID:
		ಹಿಂತಿರುಗಿ CSS_DISPLAY_INLINE_ಖಂಡ;

	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint8_t ns_computed_min_ಎತ್ತರ(
		ಸ್ಥಿರಾಂಕ css_computed_style *style,
		css_fixed *ಉದ್ದಳತೆ, css_unit *unit)
{
	uint8_t ಮೌಲ್ಯ = css_computed_min_ಎತ್ತರ(style, ಉದ್ದಳತೆ, unit);

	ಆದರೆ (ಮೌಲ್ಯ == CSS_MIN_ಎತ್ತರ_AUTO) {
		ಮೌಲ್ಯ = CSS_MIN_ಎತ್ತರ_ಗಣ;
		*ಉದ್ದಳತೆ = 0;
		*unit = CSS_UNIT_PX;
	}

	ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
}


ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint8_t ns_computed_min_ಅಗಲ(
		ಸ್ಥಿರಾಂಕ css_computed_style *style,
		css_fixed *ಉದ್ದಳತೆ, css_unit *unit)
{
	uint8_t ಮೌಲ್ಯ = css_computed_min_ಅಗಲ(style, ಉದ್ದಳತೆ, unit);

	ಆದರೆ (ಮೌಲ್ಯ == CSS_MIN_ಅಗಲ_AUTO) {
		ಮೌಲ್ಯ = CSS_MIN_ಅಗಲ_ಗಣ;
		*ಉದ್ದಳತೆ = 0;
		*unit = CSS_UNIT_PX;
	}

	ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
}

#ಆದರೆ_ಅಂತ್ಯ
