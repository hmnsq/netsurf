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

#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "netsurf/plot_style.h"
#அந்தர்பாவ "css/utils.h"

#அந்தர்பாவ "html/font.h"

/**
 * Map a generic CSS font family to a generic plot font family
 *
 * \param css Generic CSS font family
 * \return Plot font family
 */
நிலையான plot_font_generic_family_t
plot_font_generic_family(எண்ணிக்கை css_font_family_e css)
{
	plot_font_generic_family_t plot;

	தேர்வு (css) {
	நிலை CSS_FONT_FAMILY_SERIF:
		plot = PLOT_FONT_FAMILY_SERIF;
		நிறுத்து;
	நிலை CSS_FONT_FAMILY_MONOSPACE:
		plot = PLOT_FONT_FAMILY_MONOSPACE;
		நிறுத்து;
	நிலை CSS_FONT_FAMILY_CURSIVE:
		plot = PLOT_FONT_FAMILY_CURSIVE;
		நிறுத்து;
	நிலை CSS_FONT_FAMILY_FANTASY:
		plot = PLOT_FONT_FAMILY_FANTASY;
		நிறுத்து;
	நிலை CSS_FONT_FAMILY_SANS_SERIF:
	இயல்பு:
		plot = PLOT_FONT_FAMILY_SANS_SERIF;
		நிறுத்து;
	}

	திரும்பு plot;
}

/**
 * Map a CSS font weight to a plot weight value
 *
 * \param css  CSS font weight
 * \return Plot weight
 */
நிலையான முழு plot_font_weight(எண்ணிக்கை css_font_weight_e css)
{
	முழு weight;

	தேர்வு (css) {
	நிலை CSS_FONT_WEIGHT_100:
		weight = 100;
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_200:
		weight = 200;
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_300:
		weight = 300;
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_400:
	நிலை CSS_FONT_WEIGHT_NORMAL:
	இயல்பு:
		weight = 400;
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_500:
		weight = 500;
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_600:
		weight = 600;
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_700:
	நிலை CSS_FONT_WEIGHT_BOLD:
		weight = 700;
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_800:
		weight = 800;
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_900:
		weight = 900;
		நிறுத்து;
	}

	திரும்பு weight;
}

/**
 * Map a CSS font style and font variant to plot font flags
 *
 * \param style    CSS font style
 * \param variant  CSS font variant
 * \return Computed plot flags
 */
நிலையான plot_font_flags_t plot_font_flags(எண்ணிக்கை css_font_style_e style,
		எண்ணிக்கை css_font_variant_e variant)
{
	plot_font_flags_t flags = FONTF_NONE;

	எனில் (style == CSS_FONT_STYLE_ITALIC)
		flags |= FONTF_ITALIC;
	இல்லையெனில் எனில் (style == CSS_FONT_STYLE_OBLIQUE)
		flags |= FONTF_OBLIQUE;

	எனில் (variant == CSS_FONT_VARIANT_SMALL_CAPS)
		flags |= FONTF_SMALLCAPS;

	திரும்பு flags;
}


/* exported function documented in html/font.h */
வெறுமை font_plot_style_from_css(
		மாறிலி css_unit_ctx *unit_len_ctx,
		மாறிலி css_computed_style *css,
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
	எனில் (fstyle->size < (nsoption_int(font_min_size) * PLOT_STYLE_SCALE) / 10)
		fstyle->size = (nsoption_int(font_min_size) * PLOT_STYLE_SCALE) / 10;

	fstyle->weight = plot_font_weight(css_computed_font_weight(css));
	fstyle->flags = plot_font_flags(css_computed_font_style(css),
			css_computed_font_variant(css));

	css_computed_color(css, &col);
	fstyle->foreground = nscss_color_to_ns(col);
	fstyle->background = 0;
}
