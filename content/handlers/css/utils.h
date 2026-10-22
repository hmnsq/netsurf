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

#اگر_تعریف_نشده NETSURF_CSS_UTILS_H_
#تعریف_کن NETSURF_CSS_UTILS_H_

#ضمن <libcss/libcss.h>

#ضمن "netsurf/css.h"

/** DPI of the screen, in fixed point units */
خارجي css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ساكن مضمن uint8_t ns_computed_display(
		ثابت css_computed_style *style, bool ریشه)
{
	uint8_t مقدار = css_computed_display(style, ریشه);

	بدل (مقدار) {
	حالة CSS_DISPLAY_GRID:
		أرجع CSS_DISPLAY_بلوک;

	حالة CSS_DISPLAY_INLINE_GRID:
		أرجع CSS_DISPLAY_INLINE_بلوک;

	افتراضي:
		اكسر;
	}

	أرجع مقدار;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ساكن مضمن uint8_t ns_computed_display_static(
		ثابت css_computed_style *style)
{
	uint8_t مقدار = css_computed_display_static(style);

	بدل (مقدار) {
	حالة CSS_DISPLAY_GRID:
		أرجع CSS_DISPLAY_بلوک;

	حالة CSS_DISPLAY_INLINE_GRID:
		أرجع CSS_DISPLAY_INLINE_بلوک;

	افتراضي:
		اكسر;
	}

	أرجع مقدار;
}

ساكن مضمن uint8_t ns_computed_min_ارتفاع(
		ثابت css_computed_style *style,
		css_fixed *درازی, css_unit *unit)
{
	uint8_t مقدار = css_computed_min_ارتفاع(style, درازی, unit);

	إذا (مقدار == CSS_MIN_ارتفاع_AUTO) {
		مقدار = CSS_MIN_ارتفاع_مجموعه;
		*درازی = 0;
		*unit = CSS_UNIT_PX;
	}

	أرجع مقدار;
}


ساكن مضمن uint8_t ns_computed_min_عرض(
		ثابت css_computed_style *style,
		css_fixed *درازی, css_unit *unit)
{
	uint8_t مقدار = css_computed_min_عرض(style, درازی, unit);

	إذا (مقدار == CSS_MIN_عرض_AUTO) {
		مقدار = CSS_MIN_عرض_مجموعه;
		*درازی = 0;
		*unit = CSS_UNIT_PX;
	}

	أرجع مقدار;
}

#پایان_اگر
