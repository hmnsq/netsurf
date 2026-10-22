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

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/plot_style.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/utils.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/font.h"

/**
 * Map a generic CSS font family to a generic plot font family
 *
 * \param css Generic CSS font family
 * \return Plot font family
 */
ਸਥਿਰ plot_font_generic_family_t
plot_font_generic_family(ਗਿਣਤੀ css_font_family_e css)
{
	plot_font_generic_family_t plot;

	ਚੋਣ (css) {
	ਹਾਲਤ CSS_FONT_FAMILY_SERIF:
		plot = PLOT_FONT_FAMILY_SERIF;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_FAMILY_MONOSPACE:
		plot = PLOT_FONT_FAMILY_MONOSPACE;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_FAMILY_CURSIVE:
		plot = PLOT_FONT_FAMILY_CURSIVE;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_FAMILY_FANTASY:
		plot = PLOT_FONT_FAMILY_FANTASY;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_FAMILY_SANS_SERIF:
	ਮੂਲ:
		plot = PLOT_FONT_FAMILY_SANS_SERIF;
		ਰੋਕੋ;
	}

	ਵਾਪਸ plot;
}

/**
 * Map a CSS font weight to a plot weight value
 *
 * \param css  CSS font weight
 * \return Plot weight
 */
ਸਥਿਰ ਪੂਰਨ plot_font_ਭਾਰ(ਗਿਣਤੀ css_font_ਭਾਰ_e css)
{
	ਪੂਰਨ ਭਾਰ;

	ਚੋਣ (css) {
	ਹਾਲਤ CSS_FONT_ਭਾਰ_100:
		ਭਾਰ = 100;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_200:
		ਭਾਰ = 200;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_300:
		ਭਾਰ = 300;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_400:
	ਹਾਲਤ CSS_FONT_ਭਾਰ_ਸਾਧਾਰਨ:
	ਮੂਲ:
		ਭਾਰ = 400;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_500:
		ਭਾਰ = 500;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_600:
		ਭਾਰ = 600;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_700:
	ਹਾਲਤ CSS_FONT_ਭਾਰ_BOLD:
		ਭਾਰ = 700;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_800:
		ਭਾਰ = 800;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_900:
		ਭਾਰ = 900;
		ਰੋਕੋ;
	}

	ਵਾਪਸ ਭਾਰ;
}

/**
 * Map a CSS font style and font variant to plot font flags
 *
 * \param style    CSS font style
 * \param variant  CSS font variant
 * \return Computed plot flags
 */
ਸਥਿਰ plot_font_flags_t plot_font_flags(ਗਿਣਤੀ css_font_style_e style,
		ਗਿਣਤੀ css_font_variant_e variant)
{
	plot_font_flags_t flags = FONTF_NONE;

	ਜੇ (style == CSS_FONT_STYLE_ITALIC)
		flags |= FONTF_ITALIC;
	ਨਹੀਂਤਾਂ ਜੇ (style == CSS_FONT_STYLE_OBLIQUE)
		flags |= FONTF_OBLIQUE;

	ਜੇ (variant == CSS_FONT_VARIANT_SMALL_CAPS)
		flags |= FONTF_SMALLCAPS;

	ਵਾਪਸ flags;
}


/* exported function documented in html/font.h */
ਖਾਲੀ font_plot_style_from_css(
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਅਚਲ css_computed_style *css,
		plot_font_style_t *fstyle)
{
	lwc_string **families;
	css_fixed ਲੰਮਾਈ = 0;
	css_unit unit = CSS_UNIT_PX;
	css_color ਖਾਨਾ;

	fstyle->family = plot_font_generic_family(
			css_computed_font_family(css, &families));
	fstyle->families = families;

	css_computed_font_ਮਾਪ(css, &ਲੰਮਾਈ, &unit);
	fstyle->ਮਾਪ = FIXTOINT(FMUL(css_unit_font_ਮਾਪ_len2pt(css,
				      unit_len_ctx, ਲੰਮਾਈ, unit),
				      INTTOFIX(PLOT_STYLE_SCALE)));

	/* Clamp font size to configured minimum */
	ਜੇ (fstyle->ਮਾਪ < (nsoption_int(font_min_ਮਾਪ) * PLOT_STYLE_SCALE) / 10)
		fstyle->ਮਾਪ = (nsoption_int(font_min_ਮਾਪ) * PLOT_STYLE_SCALE) / 10;

	fstyle->ਭਾਰ = plot_font_ਭਾਰ(css_computed_font_ਭਾਰ(css));
	fstyle->flags = plot_font_flags(css_computed_font_style(css),
			css_computed_font_variant(css));

	css_computed_color(css, &ਖਾਨਾ);
	fstyle->foreground = nscss_color_ਮੰਜ਼ਿਲ_ns(ਖਾਨਾ);
	fstyle->background = 0;
}
