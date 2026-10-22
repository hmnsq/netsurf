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

#నిర్వచితం_కాకపోతే NETSURF_CSS_UTILS_H_
#నిర్వచించు NETSURF_CSS_UTILS_H_

#అన్తర్భావ <libcss/libcss.h>

#అన్తర్భావ "netsurf/css.h"

/** DPI of the screen, in fixed point units */
బాహ్య css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
స్థిర అంతఃవరుస uint8_t ns_computed_display(
		స్థిరరాశి css_computed_style *style, bool మూలం)
{
	uint8_t విలువ = css_computed_display(style, మూలం);

	ఎంపిక (విలువ) {
	స్థితి CSS_DISPLAY_GRID:
		తిరిగి CSS_DISPLAY_ఖండం;

	స్థితి CSS_DISPLAY_INLINE_GRID:
		తిరిగి CSS_DISPLAY_INLINE_ఖండం;

	అప్రమేయ:
		ఆపు;
	}

	తిరిగి విలువ;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
స్థిర అంతఃవరుస uint8_t ns_computed_display_static(
		స్థిరరాశి css_computed_style *style)
{
	uint8_t విలువ = css_computed_display_static(style);

	ఎంపిక (విలువ) {
	స్థితి CSS_DISPLAY_GRID:
		తిరిగి CSS_DISPLAY_ఖండం;

	స్థితి CSS_DISPLAY_INLINE_GRID:
		తిరిగి CSS_DISPLAY_INLINE_ఖండం;

	అప్రమేయ:
		ఆపు;
	}

	తిరిగి విలువ;
}

స్థిర అంతఃవరుస uint8_t ns_computed_min_ఎత్తు(
		స్థిరరాశి css_computed_style *style,
		css_fixed *నిడివి, css_unit *unit)
{
	uint8_t విలువ = css_computed_min_ఎత్తు(style, నిడివి, unit);

	ఒకవేళ (విలువ == CSS_MIN_ఎత్తు_AUTO) {
		విలువ = CSS_MIN_ఎత్తు_సమితి;
		*నిడివి = 0;
		*unit = CSS_UNIT_PX;
	}

	తిరిగి విలువ;
}


స్థిర అంతఃవరుస uint8_t ns_computed_min_వెడల్పు(
		స్థిరరాశి css_computed_style *style,
		css_fixed *నిడివి, css_unit *unit)
{
	uint8_t విలువ = css_computed_min_వెడల్పు(style, నిడివి, unit);

	ఒకవేళ (విలువ == CSS_MIN_వెడల్పు_AUTO) {
		విలువ = CSS_MIN_వెడల్పు_సమితి;
		*నిడివి = 0;
		*unit = CSS_UNIT_PX;
	}

	తిరిగి విలువ;
}

#అయితే_ముగింపు
