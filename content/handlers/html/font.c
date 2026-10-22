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

#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plot_style.h"
#ಅನ್ತರ್ಭಾವ "css/utils.h"

#ಅನ್ತರ್ಭಾವ "html/font.h"

/**
 * Map a generic CSS font family to a generic plot font family
 *
 * \param css Generic CSS font family
 * \return Plot font family
 */
ಸ್ಥಿರ plot_font_generic_family_t
plot_font_generic_family(ಎಣಿಕೆ css_font_family_e css)
{
	plot_font_generic_family_t plot;

	ಆಯ್ಕೆ (css) {
	ಸ್ಥಿತಿ CSS_FONT_FAMILY_SERIF:
		plot = PLOT_FONT_FAMILY_SERIF;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_FAMILY_MONOSPACE:
		plot = PLOT_FONT_FAMILY_MONOSPACE;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_FAMILY_CURSIVE:
		plot = PLOT_FONT_FAMILY_CURSIVE;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_FAMILY_FANTASY:
		plot = PLOT_FONT_FAMILY_FANTASY;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_FAMILY_SANS_SERIF:
	ಪೂರ್ವನಿಯೋಜಿತ:
		plot = PLOT_FONT_FAMILY_SANS_SERIF;
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ plot;
}

/**
 * Map a CSS font weight to a plot weight value
 *
 * \param css  CSS font weight
 * \return Plot weight
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ plot_font_ತೂಕ(ಎಣಿಕೆ css_font_ತೂಕ_e css)
{
	ಪೂರ್ಣಾಂಕ ತೂಕ;

	ಆಯ್ಕೆ (css) {
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_100:
		ತೂಕ = 100;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_200:
		ತೂಕ = 200;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_300:
		ತೂಕ = 300;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_400:
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_ಸಾಮಾನ್ಯ:
	ಪೂರ್ವನಿಯೋಜಿತ:
		ತೂಕ = 400;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_500:
		ತೂಕ = 500;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_600:
		ತೂಕ = 600;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_700:
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_BOLD:
		ತೂಕ = 700;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_800:
		ತೂಕ = 800;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_FONT_ತೂಕ_900:
		ತೂಕ = 900;
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ ತೂಕ;
}

/**
 * Map a CSS font style and font variant to plot font flags
 *
 * \param style    CSS font style
 * \param variant  CSS font variant
 * \return Computed plot flags
 */
ಸ್ಥಿರ plot_font_flags_t plot_font_flags(ಎಣಿಕೆ css_font_style_e style,
		ಎಣಿಕೆ css_font_variant_e variant)
{
	plot_font_flags_t flags = FONTF_NONE;

	ಆದರೆ (style == CSS_FONT_STYLE_ITALIC)
		flags |= FONTF_ITALIC;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (style == CSS_FONT_STYLE_OBLIQUE)
		flags |= FONTF_OBLIQUE;

	ಆದರೆ (variant == CSS_FONT_VARIANT_SMALL_CAPS)
		flags |= FONTF_SMALLCAPS;

	ಹಿಂತಿರುಗಿ flags;
}


/* exported function documented in html/font.h */
ಶೂನ್ಯ font_plot_style_from_css(
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಸ್ಥಿರಾಂಕ css_computed_style *css,
		plot_font_style_t *fstyle)
{
	lwc_string **families;
	css_fixed ಉದ್ದಳತೆ = 0;
	css_unit unit = CSS_UNIT_PX;
	css_color ಲಂಬಸಾಲು;

	fstyle->family = plot_font_generic_family(
			css_computed_font_family(css, &families));
	fstyle->families = families;

	css_computed_font_ಗಾತ್ರ(css, &ಉದ್ದಳತೆ, &unit);
	fstyle->ಗಾತ್ರ_2 = FIXTOINT(FMUL(css_unit_font_ಗಾತ್ರ_len2pt(css,
				      unit_len_ctx, ಉದ್ದಳತೆ, unit),
				      INTTOFIX(PLOT_STYLE_SCALE)));

	/* Clamp font size to configured minimum */
	ಆದರೆ (fstyle->ಗಾತ್ರ_2 < (nsoption_int(font_min_ಗಾತ್ರ) * PLOT_STYLE_SCALE) / 10)
		fstyle->ಗಾತ್ರ_2 = (nsoption_int(font_min_ಗಾತ್ರ) * PLOT_STYLE_SCALE) / 10;

	fstyle->ತೂಕ = plot_font_ತೂಕ(css_computed_font_ತೂಕ(css));
	fstyle->flags = plot_font_flags(css_computed_font_style(css),
			css_computed_font_variant(css));

	css_computed_color(css, &ಲಂಬಸಾಲು);
	fstyle->foreground = nscss_color_ಗಮ್ಯ_ns(ಲಂಬಸಾಲು);
	fstyle->background = 0;
}
