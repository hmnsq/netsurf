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
ساكن صحيح plot_font_وزن(تعداد css_font_وزن_e css)
{
	صحيح وزن;

	بدل (css) {
	حالة CSS_FONT_وزن_100:
		وزن = 100;
		اكسر;
	حالة CSS_FONT_وزن_200:
		وزن = 200;
		اكسر;
	حالة CSS_FONT_وزن_300:
		وزن = 300;
		اكسر;
	حالة CSS_FONT_وزن_400:
	حالة CSS_FONT_وزن_نرمال:
	افتراضي:
		وزن = 400;
		اكسر;
	حالة CSS_FONT_وزن_500:
		وزن = 500;
		اكسر;
	حالة CSS_FONT_وزن_600:
		وزن = 600;
		اكسر;
	حالة CSS_FONT_وزن_700:
	حالة CSS_FONT_وزن_BOLD:
		وزن = 700;
		اكسر;
	حالة CSS_FONT_وزن_800:
		وزن = 800;
		اكسر;
	حالة CSS_FONT_وزن_900:
		وزن = 900;
		اكسر;
	}

	أرجع وزن;
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
	css_fixed درازی = 0;
	css_unit unit = CSS_UNIT_PX;
	css_color ستون;

	fstyle->family = plot_font_generic_family(
			css_computed_font_family(css, &families));
	fstyle->families = families;

	css_computed_font_اندازه(css, &درازی, &unit);
	fstyle->اندازه = FIXTOINT(FMUL(css_unit_font_اندازه_len2pt(css,
				      unit_len_ctx, درازی, unit),
				      INTTOFIX(PLOT_STYLE_SCALE)));

	/* Clamp font size to configured minimum */
	إذا (fstyle->اندازه < (nsoption_int(font_min_اندازه) * PLOT_STYLE_SCALE) / 10)
		fstyle->اندازه = (nsoption_int(font_min_اندازه) * PLOT_STYLE_SCALE) / 10;

	fstyle->وزن = plot_font_وزن(css_computed_font_وزن(css));
	fstyle->flags = plot_font_flags(css_computed_font_style(css),
			css_computed_font_variant(css));

	css_computed_color(css, &ستون);
	fstyle->foreground = nscss_color_مقصد_ns(ستون);
	fstyle->background = 0;
}
