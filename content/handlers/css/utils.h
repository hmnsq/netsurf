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

#که_نه_تعریف_شوی NETSURF_CSS_UTILS_H_
#تعریف_کړه NETSURF_CSS_UTILS_H_

#ضمن <libcss/libcss.h>

#ضمن "netsurf/css.h"

/** DPI of the screen, in fixed point units */
خارجي css_fixed nscss_screen_dpi;

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ساكن مضمن uint8_t ns_computed_display(
		ثابت css_computed_style *style, bool ریښه)
{
	uint8_t ارزښت = css_computed_display(style, ریښه);

	بدل (ارزښت) {
	حالة CSS_DISPLAY_GRID:
		أرجع CSS_DISPLAY_بلاک;

	حالة CSS_DISPLAY_INLINE_GRID:
		أرجع CSS_DISPLAY_INLINE_بلاک;

	افتراضي:
		اكسر;
	}

	أرجع ارزښت;
}

/**
 * Temporary helper wrappers for for libcss computed style getter, while
 * we don't support all values of display.
 */
ساكن مضمن uint8_t ns_computed_display_static(
		ثابت css_computed_style *style)
{
	uint8_t ارزښت = css_computed_display_static(style);

	بدل (ارزښت) {
	حالة CSS_DISPLAY_GRID:
		أرجع CSS_DISPLAY_بلاک;

	حالة CSS_DISPLAY_INLINE_GRID:
		أرجع CSS_DISPLAY_INLINE_بلاک;

	افتراضي:
		اكسر;
	}

	أرجع ارزښت;
}

ساكن مضمن uint8_t ns_computed_min_لوړوالی(
		ثابت css_computed_style *style,
		css_fixed *اوږدتیا, css_unit *unit)
{
	uint8_t ارزښت = css_computed_min_لوړوالی(style, اوږدتیا, unit);

	إذا (ارزښت == CSS_MIN_لوړوالی_AUTO) {
		ارزښت = CSS_MIN_لوړوالی_سېټ;
		*اوږدتیا = 0;
		*unit = CSS_UNIT_PX;
	}

	أرجع ارزښت;
}


ساكن مضمن uint8_t ns_computed_min_پلنوالی(
		ثابت css_computed_style *style,
		css_fixed *اوږدتیا, css_unit *unit)
{
	uint8_t ارزښت = css_computed_min_پلنوالی(style, اوږدتیا, unit);

	إذا (ارزښت == CSS_MIN_پلنوالی_AUTO) {
		ارزښت = CSS_MIN_پلنوالی_سېټ;
		*اوږدتیا = 0;
		*unit = CSS_UNIT_PX;
	}

	أرجع ارزښت;
}

#د_که_پای
