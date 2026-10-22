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

#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "netsurf/plot_style.h"
#অন্তর্ভাব "css/utils.h"

#অন্তর্ভাব "html/font.h"

/**
 * Map a generic CSS font family to a generic plot font family
 *
 * \param css Generic CSS font family
 * \return Plot font family
 */
স্থির plot_font_generic_family_t
plot_font_generic_family(গণনা css_font_family_e css)
{
	plot_font_generic_family_t plot;

	নির্বাচন (css) {
	ক্ষেত্র CSS_FONT_FAMILY_SERIF:
		plot = PLOT_FONT_FAMILY_SERIF;
		বিরতি;
	ক্ষেত্র CSS_FONT_FAMILY_MONOSPACE:
		plot = PLOT_FONT_FAMILY_MONOSPACE;
		বিরতি;
	ক্ষেত্র CSS_FONT_FAMILY_CURSIVE:
		plot = PLOT_FONT_FAMILY_CURSIVE;
		বিরতি;
	ক্ষেত্র CSS_FONT_FAMILY_FANTASY:
		plot = PLOT_FONT_FAMILY_FANTASY;
		বিরতি;
	ক্ষেত্র CSS_FONT_FAMILY_SANS_SERIF:
	পূর্বনির্ধারিত:
		plot = PLOT_FONT_FAMILY_SANS_SERIF;
		বিরতি;
	}

	ফেরত plot;
}

/**
 * Map a CSS font weight to a plot weight value
 *
 * \param css  CSS font weight
 * \return Plot weight
 */
স্থির পূর্ণসংখ্যা plot_font_weight(গণনা css_font_weight_e css)
{
	পূর্ণসংখ্যা weight;

	নির্বাচন (css) {
	ক্ষেত্র CSS_FONT_WEIGHT_100:
		weight = 100;
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_200:
		weight = 200;
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_300:
		weight = 300;
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_400:
	ক্ষেত্র CSS_FONT_WEIGHT_NORMAL:
	পূর্বনির্ধারিত:
		weight = 400;
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_500:
		weight = 500;
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_600:
		weight = 600;
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_700:
	ক্ষেত্র CSS_FONT_WEIGHT_BOLD:
		weight = 700;
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_800:
		weight = 800;
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_900:
		weight = 900;
		বিরতি;
	}

	ফেরত weight;
}

/**
 * Map a CSS font style and font variant to plot font flags
 *
 * \param style    CSS font style
 * \param variant  CSS font variant
 * \return Computed plot flags
 */
স্থির plot_font_flags_t plot_font_flags(গণনা css_font_style_e style,
		গণনা css_font_variant_e variant)
{
	plot_font_flags_t flags = FONTF_NONE;

	যদি (style == CSS_FONT_STYLE_ITALIC)
		flags |= FONTF_ITALIC;
	অন্যথা যদি (style == CSS_FONT_STYLE_OBLIQUE)
		flags |= FONTF_OBLIQUE;

	যদি (variant == CSS_FONT_VARIANT_SMALL_CAPS)
		flags |= FONTF_SMALLCAPS;

	ফেরত flags;
}


/* exported function documented in html/font.h */
শূন্য font_plot_style_from_css(
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		ধ্রুবক css_computed_style *css,
		plot_font_style_t *fstyle)
{
	lwc_string **families;
	css_fixed দৈর্ঘ্য = 0;
	css_unit unit = CSS_UNIT_PX;
	css_color col;

	fstyle->family = plot_font_generic_family(
			css_computed_font_family(css, &families));
	fstyle->families = families;

	css_computed_font_আকার(css, &দৈর্ঘ্য, &unit);
	fstyle->আকার_2 = FIXTOINT(FMUL(css_unit_font_আকার_len2pt(css,
				      unit_len_ctx, দৈর্ঘ্য, unit),
				      INTTOFIX(PLOT_STYLE_SCALE)));

	/* Clamp font size to configured minimum */
	যদি (fstyle->আকার_2 < (nsoption_int(font_min_আকার) * PLOT_STYLE_SCALE) / 10)
		fstyle->আকার_2 = (nsoption_int(font_min_আকার) * PLOT_STYLE_SCALE) / 10;

	fstyle->weight = plot_font_weight(css_computed_font_weight(css));
	fstyle->flags = plot_font_flags(css_computed_font_style(css),
			css_computed_font_variant(css));

	css_computed_color(css, &col);
	fstyle->foreground = nscss_color_to_ns(col);
	fstyle->background = 0;
}
