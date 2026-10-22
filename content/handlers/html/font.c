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

#شامل "utils/nsoption.h"
#شامل "netsurf/plot_style.h"
#شامل "css/utils.h"

#شامل "html/font.h"

/**
 * Map a generic CSS font family to a generic plot font family
 *
 * \param css Generic CSS font family
 * \return Plot font family
 */
ساکن plot_font_generic_family_t
plot_font_generic_family(شمار css_font_family_e css)
{
	plot_font_generic_family_t plot;

	انتخاب (css) {
	حالت CSS_FONT_FAMILY_SERIF:
		plot = PLOT_FONT_FAMILY_SERIF;
		توڑو;
	حالت CSS_FONT_FAMILY_MONOSPACE:
		plot = PLOT_FONT_FAMILY_MONOSPACE;
		توڑو;
	حالت CSS_FONT_FAMILY_CURSIVE:
		plot = PLOT_FONT_FAMILY_CURSIVE;
		توڑو;
	حالت CSS_FONT_FAMILY_FANTASY:
		plot = PLOT_FONT_FAMILY_FANTASY;
		توڑو;
	حالت CSS_FONT_FAMILY_SANS_SERIF:
	طےشدہ:
		plot = PLOT_FONT_FAMILY_SANS_SERIF;
		توڑو;
	}

	واپس plot;
}

/**
 * Map a CSS font weight to a plot weight value
 *
 * \param css  CSS font weight
 * \return Plot weight
 */
ساکن صحیح plot_font_وزن(شمار css_font_وزن_e css)
{
	صحیح وزن;

	انتخاب (css) {
	حالت CSS_FONT_وزن_100:
		وزن = 100;
		توڑو;
	حالت CSS_FONT_وزن_200:
		وزن = 200;
		توڑو;
	حالت CSS_FONT_وزن_300:
		وزن = 300;
		توڑو;
	حالت CSS_FONT_وزن_400:
	حالت CSS_FONT_وزن_معمول:
	طےشدہ:
		وزن = 400;
		توڑو;
	حالت CSS_FONT_وزن_500:
		وزن = 500;
		توڑو;
	حالت CSS_FONT_وزن_600:
		وزن = 600;
		توڑو;
	حالت CSS_FONT_وزن_700:
	حالت CSS_FONT_وزن_BOLD:
		وزن = 700;
		توڑو;
	حالت CSS_FONT_وزن_800:
		وزن = 800;
		توڑو;
	حالت CSS_FONT_وزن_900:
		وزن = 900;
		توڑو;
	}

	واپس وزن;
}

/**
 * Map a CSS font style and font variant to plot font flags
 *
 * \param style    CSS font style
 * \param variant  CSS font variant
 * \return Computed plot flags
 */
ساکن plot_font_flags_t plot_font_flags(شمار css_font_style_e style,
		شمار css_font_variant_e variant)
{
	plot_font_flags_t flags = FONTF_NONE;

	اگر (style == CSS_FONT_STYLE_ITALIC)
		flags |= FONTF_ITALIC;
	ورنہ اگر (style == CSS_FONT_STYLE_OBLIQUE)
		flags |= FONTF_OBLIQUE;

	اگر (variant == CSS_FONT_VARIANT_SMALL_CAPS)
		flags |= FONTF_SMALLCAPS;

	واپس flags;
}


/* exported function documented in html/font.h */
خالی font_plot_style_from_css(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *css,
		plot_font_style_t *fstyle)
{
	lwc_string **families;
	css_fixed لمبائی = 0;
	css_unit unit = CSS_UNIT_PX;
	css_color ستون;

	fstyle->family = plot_font_generic_family(
			css_computed_font_family(css, &families));
	fstyle->families = families;

	css_computed_font_سائز(css, &لمبائی, &unit);
	fstyle->سائز = FIXTOINT(FMUL(css_unit_font_سائز_len2pt(css,
				      unit_len_ctx, لمبائی, unit),
				      INTTOFIX(PLOT_STYLE_SCALE)));

	/* Clamp font size to configured minimum */
	اگر (fstyle->سائز < (nsoption_int(font_min_سائز) * PLOT_STYLE_SCALE) / 10)
		fstyle->سائز = (nsoption_int(font_min_سائز) * PLOT_STYLE_SCALE) / 10;

	fstyle->وزن = plot_font_وزن(css_computed_font_وزن(css));
	fstyle->flags = plot_font_flags(css_computed_font_style(css),
			css_computed_font_variant(css));

	css_computed_color(css, &ستون);
	fstyle->foreground = nscss_color_مقصد_ns(ستون);
	fstyle->background = 0;
}
