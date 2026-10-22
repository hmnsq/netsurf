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
ایستا plot_font_generic_family_t
plot_font_generic_family(شمارش css_font_family_e css)
{
	plot_font_generic_family_t plot;

	انتخاب (css) {
	حالت CSS_FONT_FAMILY_SERIF:
		plot = PLOT_FONT_FAMILY_SERIF;
		بشکن;
	حالت CSS_FONT_FAMILY_MONOSPACE:
		plot = PLOT_FONT_FAMILY_MONOSPACE;
		بشکن;
	حالت CSS_FONT_FAMILY_CURSIVE:
		plot = PLOT_FONT_FAMILY_CURSIVE;
		بشکن;
	حالت CSS_FONT_FAMILY_FANTASY:
		plot = PLOT_FONT_FAMILY_FANTASY;
		بشکن;
	حالت CSS_FONT_FAMILY_SANS_SERIF:
	پیشفرض:
		plot = PLOT_FONT_FAMILY_SANS_SERIF;
		بشکن;
	}

	بازگشت plot;
}

/**
 * Map a CSS font weight to a plot weight value
 *
 * \param css  CSS font weight
 * \return Plot weight
 */
ایستا صحیح plot_font_weight(شمارش css_font_weight_e css)
{
	صحیح weight;

	انتخاب (css) {
	حالت CSS_FONT_WEIGHT_100:
		weight = 100;
		بشکن;
	حالت CSS_FONT_WEIGHT_200:
		weight = 200;
		بشکن;
	حالت CSS_FONT_WEIGHT_300:
		weight = 300;
		بشکن;
	حالت CSS_FONT_WEIGHT_400:
	حالت CSS_FONT_WEIGHT_NORMAL:
	پیشفرض:
		weight = 400;
		بشکن;
	حالت CSS_FONT_WEIGHT_500:
		weight = 500;
		بشکن;
	حالت CSS_FONT_WEIGHT_600:
		weight = 600;
		بشکن;
	حالت CSS_FONT_WEIGHT_700:
	حالت CSS_FONT_WEIGHT_BOLD:
		weight = 700;
		بشکن;
	حالت CSS_FONT_WEIGHT_800:
		weight = 800;
		بشکن;
	حالت CSS_FONT_WEIGHT_900:
		weight = 900;
		بشکن;
	}

	بازگشت weight;
}

/**
 * Map a CSS font style and font variant to plot font flags
 *
 * \param style    CSS font style
 * \param variant  CSS font variant
 * \return Computed plot flags
 */
ایستا plot_font_flags_t plot_font_flags(شمارش css_font_style_e style,
		شمارش css_font_variant_e variant)
{
	plot_font_flags_t flags = FONTF_NONE;

	اگر (style == CSS_FONT_STYLE_ITALIC)
		flags |= FONTF_ITALIC;
	وگرنه اگر (style == CSS_FONT_STYLE_OBLIQUE)
		flags |= FONTF_OBLIQUE;

	اگر (variant == CSS_FONT_VARIANT_SMALL_CAPS)
		flags |= FONTF_SMALLCAPS;

	بازگشت flags;
}


/* exported function documented in html/font.h */
تهی font_plot_style_from_css(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *css,
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
	اگر (fstyle->size < (nsoption_int(font_min_size) * PLOT_STYLE_SCALE) / 10)
		fstyle->size = (nsoption_int(font_min_size) * PLOT_STYLE_SCALE) / 10;

	fstyle->weight = plot_font_weight(css_computed_font_weight(css));
	fstyle->flags = plot_font_flags(css_computed_font_style(css),
			css_computed_font_variant(css));

	css_computed_color(css, &col);
	fstyle->foreground = nscss_color_to_ns(col);
	fstyle->background = 0;
}
