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

#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "netsurf/plot_style.h"
#അന്തര്ഭാവ "css/utils.h"

#അന്തര്ഭാവ "html/font.h"

/**
 * Map a generic CSS font family to a generic plot font family
 *
 * \param css Generic CSS font family
 * \return Plot font family
 */
സ്ഥിതം plot_font_generic_family_t
plot_font_generic_family(എണ്ണൽ css_font_family_e css)
{
	plot_font_generic_family_t plot;

	തിരഞ്ഞെടുപ്പ് (css) {
	അവസ്ഥ CSS_FONT_FAMILY_SERIF:
		plot = PLOT_FONT_FAMILY_SERIF;
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_FAMILY_MONOSPACE:
		plot = PLOT_FONT_FAMILY_MONOSPACE;
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_FAMILY_CURSIVE:
		plot = PLOT_FONT_FAMILY_CURSIVE;
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_FAMILY_FANTASY:
		plot = PLOT_FONT_FAMILY_FANTASY;
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_FAMILY_SANS_SERIF:
	സ്ഥിരസ്ഥിതി:
		plot = PLOT_FONT_FAMILY_SANS_SERIF;
		നിർത്തുക;
	}

	തിരികെ plot;
}

/**
 * Map a CSS font weight to a plot weight value
 *
 * \param css  CSS font weight
 * \return Plot weight
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ plot_font_weight(എണ്ണൽ css_font_weight_e css)
{
	പൂർണ്ണസംഖ്യ weight;

	തിരഞ്ഞെടുപ്പ് (css) {
	അവസ്ഥ CSS_FONT_WEIGHT_100:
		weight = 100;
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_200:
		weight = 200;
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_300:
		weight = 300;
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_400:
	അവസ്ഥ CSS_FONT_WEIGHT_NORMAL:
	സ്ഥിരസ്ഥിതി:
		weight = 400;
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_500:
		weight = 500;
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_600:
		weight = 600;
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_700:
	അവസ്ഥ CSS_FONT_WEIGHT_BOLD:
		weight = 700;
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_800:
		weight = 800;
		നിർത്തുക;
	അവസ്ഥ CSS_FONT_WEIGHT_900:
		weight = 900;
		നിർത്തുക;
	}

	തിരികെ weight;
}

/**
 * Map a CSS font style and font variant to plot font flags
 *
 * \param style    CSS font style
 * \param variant  CSS font variant
 * \return Computed plot flags
 */
സ്ഥിതം plot_font_flags_t plot_font_flags(എണ്ണൽ css_font_style_e style,
		എണ്ണൽ css_font_variant_e variant)
{
	plot_font_flags_t flags = FONTF_NONE;

	എങ്കിൽ (style == CSS_FONT_STYLE_ITALIC)
		flags |= FONTF_ITALIC;
	അല്ലെങ്കിൽ എങ്കിൽ (style == CSS_FONT_STYLE_OBLIQUE)
		flags |= FONTF_OBLIQUE;

	എങ്കിൽ (variant == CSS_FONT_VARIANT_SMALL_CAPS)
		flags |= FONTF_SMALLCAPS;

	തിരികെ flags;
}


/* exported function documented in html/font.h */
ശൂന്യം font_plot_style_from_css(
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		സ്ഥിരം css_computed_style *css,
		plot_font_style_t *fstyle)
{
	lwc_string **families;
	css_fixed length = 0;
	css_unit unit = CSS_UNIT_PX;
	css_color col;

	fstyle->family = plot_font_generic_family(
			css_computed_font_family(css, &families));
	fstyle->families = families;

	css_computed_font_size(css, &length, &unit);
	fstyle->size = FIXTOINT(FMUL(css_unit_font_size_len2pt(css,
				      unit_len_ctx, length, unit),
				      INTTOFIX(PLOT_STYLE_SCALE)));

	/* Clamp font size to configured minimum */
	എങ്കിൽ (fstyle->size < (nsoption_int(font_min_size) * PLOT_STYLE_SCALE) / 10)
		fstyle->size = (nsoption_int(font_min_size) * PLOT_STYLE_SCALE) / 10;

	fstyle->weight = plot_font_weight(css_computed_font_weight(css));
	fstyle->flags = plot_font_flags(css_computed_font_style(css),
			css_computed_font_variant(css));

	css_computed_color(css, &col);
	fstyle->foreground = nscss_color_to_ns(col);
	fstyle->background = 0;
}
