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

/**
 * \file
 *
 * HTML internal font handling implementation.
 */

#ضمن "utils/nsoption.h"
#ضمن "netsurf/plot_style.h"
#ضمن "css/utils.h"

#ضمن "html/font.h"

/**
 * Map a generic CSS font family to a generic plot font family
 *
 * \param css Generic CSS font family
 * \return Plot font family
 */
ساكن plot_font_generic_family_t
plot_font_generic_family(تعداد css_font_family_e css)
{
	plot_font_generic_family_t plot;

	بدل (css) {
	حالة CSS_FONT_FAMILY_SERIF:
		plot = PLOT_FONT_FAMILY_SERIF;
		اكسر;
	حالة CSS_FONT_FAMILY_MONOSPACE:
		plot = PLOT_FONT_FAMILY_MONOSPACE;
		اكسر;
	حالة CSS_FONT_FAMILY_CURSIVE:
		plot = PLOT_FONT_FAMILY_CURSIVE;
		اكسر;
	حالة CSS_FONT_FAMILY_FANTASY:
		plot = PLOT_FONT_FAMILY_FANTASY;
		اكسر;
	حالة CSS_FONT_FAMILY_SANS_SERIF:
	افتراضي:
		plot = PLOT_FONT_FAMILY_SANS_SERIF;
		اكسر;
	}

	أرجع plot;
}

/**
 * Map a CSS font weight to a plot weight value
 *
 * \param css  CSS font weight
 * \return Plot weight
 */
ساكن صحيح plot_font_بھار(تعداد css_font_بھار_e css)
{
	صحيح بھار;

	بدل (css) {
	حالة CSS_FONT_بھار_100:
		بھار = 100;
		اكسر;
	حالة CSS_FONT_بھار_200:
		بھار = 200;
		اكسر;
	حالة CSS_FONT_بھار_300:
		بھار = 300;
		اكسر;
	حالة CSS_FONT_بھار_400:
	حالة CSS_FONT_بھار_عام:
	افتراضي:
		بھار = 400;
		اكسر;
	حالة CSS_FONT_بھار_500:
		بھار = 500;
		اكسر;
	حالة CSS_FONT_بھار_600:
		بھار = 600;
		اكسر;
	حالة CSS_FONT_بھار_700:
	حالة CSS_FONT_بھار_BOLD:
		بھار = 700;
		اكسر;
	حالة CSS_FONT_بھار_800:
		بھار = 800;
		اكسر;
	حالة CSS_FONT_بھار_900:
		بھار = 900;
		اكسر;
	}

	أرجع بھار;
}

/**
 * Map a CSS font style and font variant to plot font flags
 *
 * \param style    CSS font style
 * \param variant  CSS font variant
 * \return Computed plot flags
 */
ساكن plot_font_flags_t plot_font_flags(تعداد css_font_style_e style,
		تعداد css_font_variant_e variant)
{
	plot_font_flags_t flags = FONTF_NONE;

	إذا (style == CSS_FONT_STYLE_ITALIC)
		flags |= FONTF_ITALIC;
	وإلا إذا (style == CSS_FONT_STYLE_OBLIQUE)
		flags |= FONTF_OBLIQUE;

	إذا (variant == CSS_FONT_VARIANT_SMALL_CAPS)
		flags |= FONTF_SMALLCAPS;

	أرجع flags;
}


/* exported function documented in html/font.h */
فراغ font_plot_style_from_css(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *css,
		plot_font_style_t *fstyle)
{
	lwc_string **families;
	css_fixed لمائی = 0;
	css_unit unit = CSS_UNIT_PX;
	css_color خانہ;

	fstyle->family = plot_font_generic_family(
			css_computed_font_family(css, &families));
	fstyle->families = families;

	css_computed_font_ناپ(css, &لمائی, &unit);
	fstyle->ناپ = FIXTOINT(FMUL(css_unit_font_ناپ_len2pt(css,
				      unit_len_ctx, لمائی, unit),
				      INTTOFIX(PLOT_STYLE_SCALE)));

	/* Clamp font size to configured minimum */
	إذا (fstyle->ناپ < (nsoption_int(font_min_ناپ) * PLOT_STYLE_SCALE) / 10)
		fstyle->ناپ = (nsoption_int(font_min_ناپ) * PLOT_STYLE_SCALE) / 10;

	fstyle->بھار = plot_font_بھار(css_computed_font_بھار(css));
	fstyle->flags = plot_font_flags(css_computed_font_style(css),
			css_computed_font_variant(css));

	css_computed_color(css, &خانہ);
	fstyle->foreground = nscss_color_منزل_ns(خانہ);
	fstyle->background = 0;
}
