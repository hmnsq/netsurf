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
plot_font_generic_family(ಗಣನಾ css_font_family_e css)
{
	plot_font_generic_family_t plot;

	ವಿಕಲ್ಪ (css) {
	ಪಕ್ಷ CSS_FONT_FAMILY_SERIF:
		plot = PLOT_FONT_FAMILY_SERIF;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_FAMILY_MONOSPACE:
		plot = PLOT_FONT_FAMILY_MONOSPACE;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_FAMILY_CURSIVE:
		plot = PLOT_FONT_FAMILY_CURSIVE;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_FAMILY_FANTASY:
		plot = PLOT_FONT_FAMILY_FANTASY;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_FAMILY_SANS_SERIF:
	ಸ್ವಭಾವ:
		plot = PLOT_FONT_FAMILY_SANS_SERIF;
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ plot;
}

/**
 * Map a CSS font weight to a plot weight value
 *
 * \param css  CSS font weight
 * \return Plot weight
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ plot_font_weight(ಗಣನಾ css_font_weight_e css)
{
	ಪೂರ್ಣಾಙ್ಕ weight;

	ವಿಕಲ್ಪ (css) {
	ಪಕ್ಷ CSS_FONT_WEIGHT_100:
		weight = 100;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_200:
		weight = 200;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_300:
		weight = 300;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_400:
	ಪಕ್ಷ CSS_FONT_WEIGHT_NORMAL:
	ಸ್ವಭಾವ:
		weight = 400;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_500:
		weight = 500;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_600:
		weight = 600;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_700:
	ಪಕ್ಷ CSS_FONT_WEIGHT_BOLD:
		weight = 700;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_800:
		weight = 800;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_FONT_WEIGHT_900:
		weight = 900;
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ weight;
}

/**
 * Map a CSS font style and font variant to plot font flags
 *
 * \param style    CSS font style
 * \param variant  CSS font variant
 * \return Computed plot flags
 */
ಸ್ಥಿರ plot_font_flags_t plot_font_flags(ಗಣನಾ css_font_style_e style,
		ಗಣನಾ css_font_variant_e variant)
{
	plot_font_flags_t flags = FONTF_NONE;

	ಯದಿ (style == CSS_FONT_STYLE_ITALIC)
		flags |= FONTF_ITALIC;
	ಅನ್ಯಥಾ ಯದಿ (style == CSS_FONT_STYLE_OBLIQUE)
		flags |= FONTF_OBLIQUE;

	ಯದಿ (variant == CSS_FONT_VARIANT_SMALL_CAPS)
		flags |= FONTF_SMALLCAPS;

	ಪ್ರತ್ಯಾಗಮ flags;
}


/* exported function documented in html/font.h */
ಶೂನ್ಯ font_plot_style_from_css(
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಧ್ರುವ css_computed_style *css,
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
	ಯದಿ (fstyle->size < (nsoption_int(font_min_size) * PLOT_STYLE_SCALE) / 10)
		fstyle->size = (nsoption_int(font_min_size) * PLOT_STYLE_SCALE) / 10;

	fstyle->weight = plot_font_weight(css_computed_font_weight(css));
	fstyle->flags = plot_font_flags(css_computed_font_style(css),
			css_computed_font_variant(css));

	css_computed_color(css, &col);
	fstyle->foreground = nscss_color_to_ns(col);
	fstyle->background = 0;
}
