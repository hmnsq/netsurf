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

#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "netsurf/plot_style.h"
#అన్తర్భావ "css/utils.h"

#అన్తర్భావ "html/font.h"

/**
 * Map a generic CSS font family to a generic plot font family
 *
 * \param css Generic CSS font family
 * \return Plot font family
 */
స్థిర plot_font_generic_family_t
plot_font_generic_family(గణన css_font_family_e css)
{
	plot_font_generic_family_t plot;

	ఎంపిక (css) {
	స్థితి CSS_FONT_FAMILY_SERIF:
		plot = PLOT_FONT_FAMILY_SERIF;
		ఆపు;
	స్థితి CSS_FONT_FAMILY_MONOSPACE:
		plot = PLOT_FONT_FAMILY_MONOSPACE;
		ఆపు;
	స్థితి CSS_FONT_FAMILY_CURSIVE:
		plot = PLOT_FONT_FAMILY_CURSIVE;
		ఆపు;
	స్థితి CSS_FONT_FAMILY_FANTASY:
		plot = PLOT_FONT_FAMILY_FANTASY;
		ఆపు;
	స్థితి CSS_FONT_FAMILY_SANS_SERIF:
	అప్రమేయ:
		plot = PLOT_FONT_FAMILY_SANS_SERIF;
		ఆపు;
	}

	తిరిగి plot;
}

/**
 * Map a CSS font weight to a plot weight value
 *
 * \param css  CSS font weight
 * \return Plot weight
 */
స్థిర పూర్ణాంకం plot_font_బరువు(గణన css_font_బరువు_e css)
{
	పూర్ణాంకం బరువు;

	ఎంపిక (css) {
	స్థితి CSS_FONT_బరువు_100:
		బరువు = 100;
		ఆపు;
	స్థితి CSS_FONT_బరువు_200:
		బరువు = 200;
		ఆపు;
	స్థితి CSS_FONT_బరువు_300:
		బరువు = 300;
		ఆపు;
	స్థితి CSS_FONT_బరువు_400:
	స్థితి CSS_FONT_బరువు_సాధారణ:
	అప్రమేయ:
		బరువు = 400;
		ఆపు;
	స్థితి CSS_FONT_బరువు_500:
		బరువు = 500;
		ఆపు;
	స్థితి CSS_FONT_బరువు_600:
		బరువు = 600;
		ఆపు;
	స్థితి CSS_FONT_బరువు_700:
	స్థితి CSS_FONT_బరువు_BOLD:
		బరువు = 700;
		ఆపు;
	స్థితి CSS_FONT_బరువు_800:
		బరువు = 800;
		ఆపు;
	స్థితి CSS_FONT_బరువు_900:
		బరువు = 900;
		ఆపు;
	}

	తిరిగి బరువు;
}

/**
 * Map a CSS font style and font variant to plot font flags
 *
 * \param style    CSS font style
 * \param variant  CSS font variant
 * \return Computed plot flags
 */
స్థిర plot_font_flags_t plot_font_flags(గణన css_font_style_e style,
		గణన css_font_variant_e variant)
{
	plot_font_flags_t flags = FONTF_NONE;

	ఒకవేళ (style == CSS_FONT_STYLE_ITALIC)
		flags |= FONTF_ITALIC;
	లేకపోతే ఒకవేళ (style == CSS_FONT_STYLE_OBLIQUE)
		flags |= FONTF_OBLIQUE;

	ఒకవేళ (variant == CSS_FONT_VARIANT_SMALL_CAPS)
		flags |= FONTF_SMALLCAPS;

	తిరిగి flags;
}


/* exported function documented in html/font.h */
శూన్య font_plot_style_from_css(
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		స్థిరరాశి css_computed_style *css,
		plot_font_style_t *fstyle)
{
	lwc_string **families;
	css_fixed నిడివి = 0;
	css_unit unit = CSS_UNIT_PX;
	css_color నిలువు;

	fstyle->family = plot_font_generic_family(
			css_computed_font_family(css, &families));
	fstyle->families = families;

	css_computed_font_కొలత(css, &నిడివి, &unit);
	fstyle->కొలత = FIXTOINT(FMUL(css_unit_font_కొలత_len2pt(css,
				      unit_len_ctx, నిడివి, unit),
				      INTTOFIX(PLOT_STYLE_SCALE)));

	/* Clamp font size to configured minimum */
	ఒకవేళ (fstyle->కొలత < (nsoption_int(font_min_కొలత) * PLOT_STYLE_SCALE) / 10)
		fstyle->కొలత = (nsoption_int(font_min_కొలత) * PLOT_STYLE_SCALE) / 10;

	fstyle->బరువు = plot_font_బరువు(css_computed_font_బరువు(css));
	fstyle->flags = plot_font_flags(css_computed_font_style(css),
			css_computed_font_variant(css));

	css_computed_color(css, &నిలువు);
	fstyle->foreground = nscss_color_గమ్యం_ns(నిలువు);
	fstyle->background = 0;
}
