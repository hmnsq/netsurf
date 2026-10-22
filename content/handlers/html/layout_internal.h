/*
 * Copyright 2003 James Bursa <bursa@users.sourceforge.net>
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
 * HTML layout private interface.
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_HTML_LAYOUT_ಆಂತರಿಕ_H
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_HTML_LAYOUT_ಆಂತರಿಕ_H

#ವ್ಯಾಖ್ಯಾನಿಸು AUTO INT_MIN

/**
 * Layout a block formatting context.
 *
 * \param  block            BLOCK, INLINE_BLOCK, or TABLE_CELL to layout
 * \param  viewport_height  Height of viewport in pixels or -ve if unknown
 * \param  content          Memory pool for any new boxes
 * \return  true on success, false on memory exhaustion
 *
 * This function carries out layout of a block and its children, as described
 * in CSS 2.1 9.4.1.
 */
bool layout_ಖಂಡ_context(
		ರಚನೆ box *ಖಂಡ,
		ಪೂರ್ಣಾಂಕ viewport_ಎತ್ತರ,
		html_content *content);

/**
 * Layout a table.
 *
 * \param  table            table to layout
 * \param  available_width  width of containing block
 * \param  content          memory pool for any new boxes
 * \return  true on success, false on memory exhaustion
 */
bool layout_ಕೋಷ್ಟಕ(
		ರಚನೆ box *ಕೋಷ್ಟಕ,
		ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ,
		html_content *content);

/**
 * Layout a flex container.
 *
 * \param[in] flex             table to layout
 * \param[in] available_width  width of containing block
 * \param[in] content          memory pool for any new boxes
 * \return  true on success, false on memory exhaustion
 */
bool layout_flex(
		ರಚನೆ box *flex,
		ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ,
		html_content *content);

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uint8_t (*css_len_func)(
		ಸ್ಥಿರಾಂಕ css_computed_style *style,
		css_fixed *ಉದ್ದಳತೆ, css_unit *unit);
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uint8_t (*css_border_style_func)(
		ಸ್ಥಿರಾಂಕ css_computed_style *style);
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uint8_t (*css_border_color_func)(
		ಸ್ಥಿರಾಂಕ css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ css_len_func border_ಅಗಲ_funcs[4];

/** Array of per-side access functions for computed style border styles. */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool lh__box_is_float_box(ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ b->type == BOX_FLOAT_ಎಡ ||
	       b->type == BOX_FLOAT_ಬಲ;
}

/** Layout helper: Check whether box takes part in inline flow. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool lh__box_is_inline_flow(ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_ಖಂಡ ||
	       b->type == BOX_ಪಠ್ಯ ||
	       b->type == BOX_INLINE_ಕೊನೆ;
}

/** Layout helper: Check whether box takes part in inline flow. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool lh__box_is_flex_container(ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool lh_box_is_flex_ಅಂಶ(ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ (b->ಪೋಷಕ != ಖಾಲಿ) && lh__box_is_flex_container(b->ಪೋಷಕ);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool lh__box_is_inline_level(ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool lh__box_is_inline_content(ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool lh__box_is_object(ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool lh__box_is_replace(ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool lh__have_border(
		ಎಣಿಕೆ box_side side,
		ಸ್ಥಿರಾಂಕ css_computed_style *style)
{
	ಹಿಂತಿರುಗಿ border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool lh__box_is_absolute(ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool lh__flex_main_is_horizontal(ಸ್ಥಿರಾಂಕ ರಚನೆ box *flex)
{
	ಸ್ಥಿರಾಂಕ css_computed_style *style = flex->style;

	ಪ್ರತಿಜ್ಞಾ(style != ಖಾಲಿ);

	ಆಯ್ಕೆ (css_computed_flex_direction(style)) {
	ಪೂರ್ವನಿಯೋಜಿತ:                        /* Fallthrough. */
	ಸ್ಥಿತಿ CSS_FLEX_DIRECTION_ಅಡ್ಡಸಾಲು:    /* Fallthrough. */
	ಸ್ಥಿತಿ CSS_FLEX_DIRECTION_ಅಡ್ಡಸಾಲು_ಹಿಮ್ಮುಖ:
		ಹಿಂತಿರುಗಿ true;
	ಸ್ಥಿತಿ CSS_FLEX_DIRECTION_ಸ್ತಂಭ: /* Fallthrough. */
	ಸ್ಥಿತಿ CSS_FLEX_DIRECTION_ಸ್ತಂಭ_ಹಿಮ್ಮುಖ:
		ಹಿಂತಿರುಗಿ false;
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool lh__flex_direction_reversed(ಸ್ಥಿರಾಂಕ ರಚನೆ box *flex)
{
	ಆಯ್ಕೆ (css_computed_flex_direction(flex->style)) {
	ಪೂರ್ವನಿಯೋಜಿತ:                             /* Fallthrough. */
	ಸ್ಥಿತಿ CSS_FLEX_DIRECTION_ಅಡ್ಡಸಾಲು_ಹಿಮ್ಮುಖ: /* Fallthrough. */
	ಸ್ಥಿತಿ CSS_FLEX_DIRECTION_ಸ್ತಂಭ_ಹಿಮ್ಮುಖ:
		ಹಿಂತಿರುಗಿ true;
	ಸ್ಥಿತಿ CSS_FLEX_DIRECTION_ಅಡ್ಡಸಾಲು: /* Fallthrough. */
	ಸ್ಥಿತಿ CSS_FLEX_DIRECTION_ಸ್ತಂಭ:
		ಹಿಂತಿರುಗಿ false;
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ lh__non_auto_margin(ಸ್ಥಿರಾಂಕ ರಚನೆ box *b, ಎಣಿಕೆ box_side side)
{
	ಹಿಂತಿರುಗಿ (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ lh_delta_outer_ಎತ್ತರ(ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ b->padding[ತುದಿ_2] +
	       b->padding[BOTTOM] +
	       b->border[ತುದಿ_2].ಅಗಲ +
	       b->border[BOTTOM].ಅಗಲ +
	       lh__non_auto_margin(b, ತುದಿ_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ lh_delta_outer_ಅಗಲ(ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ b->padding[ಎಡ_2] +
	       b->padding[ಬಲ_2] +
	       b->border[ಎಡ_2].ಅಗಲ +
	       b->border[ಬಲ_2].ಅಗಲ +
	       lh__non_auto_margin(b, ಎಡ_2) +
	       lh__non_auto_margin(b, ಬಲ_2);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ lh__delta_outer_main(
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *flex,
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಆದರೆ (lh__flex_main_is_horizontal(flex)) {
		ಹಿಂತಿರುಗಿ lh_delta_outer_ಅಗಲ(b);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಹಿಂತಿರುಗಿ lh_delta_outer_ಎತ್ತರ(b);
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ lh__delta_outer_cross(
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *flex,
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಆದರೆ (lh__flex_main_is_horizontal(flex) == false) {
		ಹಿಂತಿರುಗಿ lh_delta_outer_ಅಗಲ(b);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಹಿಂತಿರುಗಿ lh_delta_outer_ಎತ್ತರ(b);
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ *lh_box_ಗಾತ್ರ_main_ptr(
		bool horizontal,
		ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ horizontal ? &b->ಅಗಲ : &b->ಎತ್ತರ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ *lh_box_ಗಾತ್ರ_cross_ptr(
		bool horizontal,
		ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ horizontal ? &b->ಎತ್ತರ : &b->ಅಗಲ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ lh_box_ಗಾತ್ರ_main(
		bool horizontal,
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ horizontal ? b->ಅಗಲ : b->ಎತ್ತರ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ lh_box_ಗಾತ್ರ_cross(
		bool horizontal,
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಹಿಂತಿರುಗಿ horizontal ? b->ಎತ್ತರ : b->ಅಗಲ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool lh_box_ಗಾತ್ರ_cross_is_auto(
		bool horizontal,
		ರಚನೆ box *b)
{
	css_fixed ಉದ್ದಳತೆ;
	css_unit unit;

	ಆದರೆ (horizontal) {
		ಹಿಂತಿರುಗಿ css_computed_ಎತ್ತರ(b->style,
				&ಉದ್ದಳತೆ, &unit) == CSS_ಎತ್ತರ_AUTO;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಹಿಂತಿರುಗಿ css_computed_ಅಗಲ(b->style,
				&ಉದ್ದಳತೆ, &unit) == CSS_ಅಗಲ_AUTO;
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಎಣಿಕೆ css_align_self_e lh__box_align_self(
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *flex,
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *ಅಂಶ)
{
	ಎಣಿಕೆ css_align_self_e align_self = css_computed_align_self(ಅಂಶ->style);

	ಆದರೆ (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_ವಸ್ತುಗಳು(flex->style);
	}

	ಹಿಂತಿರುಗಿ align_self;
}

/**
 * Determine width of margin, borders, and padding on one side of a box.
 *
 * \param unit_len_ctx  CSS length conversion context for document
 * \param style    style to measure
 * \param side     side of box to measure
 * \param margin   whether margin width is required
 * \param border   whether border width is required
 * \param padding  whether padding width is required
 * \param fixed    increased by sum of fixed margin, border, and padding
 * \param frac     increased by sum of fractional margin and padding
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ calculate_mbp_ಅಗಲ(
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಸ್ಥಿರಾಂಕ css_computed_style *style,
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ side,
		bool margin,
		bool border,
		bool padding,
		ಪೂರ್ಣಾಂಕ *fixed,
		ದಶಮಾಂಶ *frac)
{
	css_fixed ಮೌಲ್ಯ = 0;
	css_unit unit = CSS_UNIT_PX;

	ಪ್ರತಿಜ್ಞಾ(style);

	/* margin */
	ಆದರೆ (margin) {
		ಎಣಿಕೆ css_margin_e type;

		type = margin_funcs[side](style, &ಮೌಲ್ಯ, &unit);
		ಆದರೆ (type == CSS_MARGIN_ಗಣ) {
			ಆದರೆ (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(ಮೌಲ್ಯ, F_100));
			} ಇಲ್ಲದಿದ್ದರೆ {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		}
	}

	/* border */
	ಆದರೆ (border) {
		ಆದರೆ (lh__have_border(side, style)) {
			border_ಅಗಲ_funcs[side](style, &ಮೌಲ್ಯ, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		}
	}

	/* padding */
	ಆದರೆ (padding) {
		padding_funcs[side](style, &ಮೌಲ್ಯ, &unit);
		ಆದರೆ (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(ಮೌಲ್ಯ, F_100));
		} ಇಲ್ಲದಿದ್ದರೆ {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		}
	}
}

/**
 * Adjust a specified width or height for the box-sizing property.
 *
 * This turns the specified dimension into a content-box dimension.
 *
 * \param  unit_len_ctx          Length conversion context
 * \param  box		    gadget to adjust dimensions of
 * \param  available_width  width of containing block
 * \param  setwidth	    set true if the dimension to be tweaked is a width,
 *				else set false for a height
 * \param  dimension	    current value for given width/height dimension.
 *				updated to new value after consideration of
 *				gadget properties.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ layout_handle_box_sizing(
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *box,
		ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ,
		bool setwidth,
		ಪೂರ್ಣಾಂಕ *dimension)
{
	ಎಣಿಕೆ css_box_sizing_e bs;

	ಪ್ರತಿಜ್ಞಾ(box && box->style);

	bs = css_computed_box_sizing(box->style);

	ಆದರೆ (bs == CSS_BOX_SIZING_BORDER_BOX) {
		ಪೂರ್ಣಾಂಕ orig = *dimension;
		ಪೂರ್ಣಾಂಕ fixed = 0;
		ದಶಮಾಂಶ frac = 0;

		calculate_mbp_ಅಗಲ(unit_len_ctx, box->style,
				setwidth ? ಎಡ_2 : ತುದಿ_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_ಅಗಲ(unit_len_ctx, box->style,
				setwidth ? ಬಲ_2 : BOTTOM,
				false, true, true, &fixed, &frac);
		orig -= frac * ಲಭ್ಯ_ಅಗಲ + fixed;
		*dimension = orig > 0 ? orig : 0;
	}
}

/**
 * Calculate width, height, and thickness of margins, paddings, and borders.
 *
 * \param  unit_len_ctx     Length conversion context
 * \param  available_width  width of containing block
 * \param  viewport_height  height of viewport in pixels or -ve if unknown
 * \param  box              current box
 * \param  style            style giving width, height, margins, paddings,
 *                          and borders
 * \param  width            updated to width, may be NULL
 * \param  height           updated to height, may be NULL
 * \param  max_width        updated to max-width, may be NULL
 * \param  min_width        updated to min-width, may be NULL
 * \param  max_height       updated to max-height, may be NULL
 * \param  min_height       updated to min-height, may be NULL
 * \param  margin           filled with margins, may be NULL
 * \param  padding          filled with paddings, may be NULL
 * \param  border           filled with border widths, may be NULL
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ layout_ಕಂಡುಹಿಡಿ_dimensions(
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ,
		ಪೂರ್ಣಾಂಕ viewport_ಎತ್ತರ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *box,
		ಸ್ಥಿರಾಂಕ css_computed_style *style,
		ಪೂರ್ಣಾಂಕ *ಅಗಲ,
		ಪೂರ್ಣಾಂಕ *ಎತ್ತರ,
		ಪೂರ್ಣಾಂಕ *max_ಅಗಲ,
		ಪೂರ್ಣಾಂಕ *min_ಅಗಲ,
		ಪೂರ್ಣಾಂಕ *max_ಎತ್ತರ,
		ಪೂರ್ಣಾಂಕ *min_ಎತ್ತರ,
		ಪೂರ್ಣಾಂಕ margin[4],
		ಪೂರ್ಣಾಂಕ padding[4],
		ರಚನೆ box_border border[4])
{
	ರಚನೆ box *containing_ಖಂಡ = ಖಾಲಿ;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (ಅಗಲ) {
		ಆದರೆ (css_computed_ಅಗಲ_px(style, unit_len_ctx,
				ಲಭ್ಯ_ಅಗಲ, ಅಗಲ) == CSS_ಅಗಲ_ಗಣ) {
			layout_handle_box_sizing(unit_len_ctx, box,
					ಲಭ್ಯ_ಅಗಲ, true, ಅಗಲ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			*ಅಗಲ = AUTO;
		}
	}

	ಆದರೆ (ಎತ್ತರ) {
		ಎಣಿಕೆ css_ಎತ್ತರ_e htype;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_ಎತ್ತರ(style, &ಮೌಲ್ಯ, &unit);

		ಆದರೆ (htype == CSS_ಎತ್ತರ_ಗಣ) {
			ಆದರೆ (unit == CSS_UNIT_PCT) {
				ಎಣಿಕೆ css_ಎತ್ತರ_e cbhtype;

				ಆದರೆ (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->ಪೋಷಕ) {
					/* Box is absolutely positioned */
					ಪ್ರತಿಜ್ಞಾ(box->float_container);
					containing_ಖಂಡ = box->float_container;
				} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_ಎಡ ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_ಬಲ)) {
					/* Box is a float */
					ಪ್ರತಿಜ್ಞಾ(box->ಪೋಷಕ &&
						box->ಪೋಷಕ->ಪೋಷಕ &&
						box->ಪೋಷಕ->ಪೋಷಕ->ಪೋಷಕ);

					containing_ಖಂಡ =
						box->ಪೋಷಕ->ಪೋಷಕ->ಪೋಷಕ;
				} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->ಪೋಷಕ && box->ಪೋಷಕ->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_ಖಂಡ = box->ಪೋಷಕ;
				} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->ಪೋಷಕ && box->ಪೋಷಕ->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					ಪ್ರತಿಜ್ಞಾ(box->ಪೋಷಕ->ಪೋಷಕ);
					containing_ಖಂಡ = box->ಪೋಷಕ->ಪೋಷಕ;
				}

				ಆದರೆ (containing_ಖಂಡ) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_ಎತ್ತರ(
							containing_ಖಂಡ->style,
							&f, &u);
				}

				ಆದರೆ (containing_ಖಂಡ &&
					containing_ಖಂಡ->ಎತ್ತರ != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_ಎತ್ತರ_ಗಣ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*ಎತ್ತರ = FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ,
						containing_ಖಂಡ->ಎತ್ತರ);
				} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((!box->ಪೋಷಕ ||
						!box->ಪೋಷಕ->ಪೋಷಕ) &&
						viewport_ಎತ್ತರ >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*ಎತ್ತರ = FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ,
							viewport_ಎತ್ತರ);
				} ಇಲ್ಲದಿದ್ದರೆ {
					/* precentage height not permissible
					 * treat height as auto */
					*ಎತ್ತರ = AUTO;
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				*ಎತ್ತರ = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			*ಎತ್ತರ = AUTO;
		}

		ಆದರೆ (*ಎತ್ತರ != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					ಲಭ್ಯ_ಅಗಲ, false, ಎತ್ತರ);
		}
	}

	ಆದರೆ (max_ಅಗಲ) {
		ಎಣಿಕೆ css_max_ಅಗಲ_e type;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_ಅಗಲ(style, &ಮೌಲ್ಯ, &unit);

		ಆದರೆ (type == CSS_MAX_ಅಗಲ_ಗಣ) {
			ಆದರೆ (unit == CSS_UNIT_PCT) {
				*max_ಅಗಲ = FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ,
						ಲಭ್ಯ_ಅಗಲ);
			} ಇಲ್ಲದಿದ್ದರೆ {
				*max_ಅಗಲ = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Inadmissible */
			*max_ಅಗಲ = -1;
		}

		ಆದರೆ (*max_ಅಗಲ != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					ಲಭ್ಯ_ಅಗಲ, true, max_ಅಗಲ);
		}
	}

	ಆದರೆ (min_ಅಗಲ) {
		ಎಣಿಕೆ css_min_ಅಗಲ_e type;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_ಅಗಲ(style, &ಮೌಲ್ಯ, &unit);

		ಆದರೆ (type == CSS_MIN_ಅಗಲ_ಗಣ) {
			ಆದರೆ (unit == CSS_UNIT_PCT) {
				*min_ಅಗಲ = FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ,
						ಲಭ್ಯ_ಅಗಲ);
			} ಇಲ್ಲದಿದ್ದರೆ {
				*min_ಅಗಲ = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Inadmissible */
			*min_ಅಗಲ = 0;
		}

		ಆದರೆ (*min_ಅಗಲ != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					ಲಭ್ಯ_ಅಗಲ, true, min_ಅಗಲ);
		}
	}

	ಆದರೆ (max_ಎತ್ತರ) {
		ಎಣಿಕೆ css_max_ಎತ್ತರ_e type;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_ಎತ್ತರ(style, &ಮೌಲ್ಯ, &unit);

		ಆದರೆ (type == CSS_MAX_ಎತ್ತರ_ಗಣ) {
			ಆದರೆ (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_ಎತ್ತರ = -1;
			} ಇಲ್ಲದಿದ್ದರೆ {
				*max_ಎತ್ತರ = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Inadmissible */
			*max_ಎತ್ತರ = -1;
		}
	}

	ಆದರೆ (min_ಎತ್ತರ) {
		ಎಣಿಕೆ css_min_ಎತ್ತರ_e type;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_ಎತ್ತರ(style, &ಮೌಲ್ಯ, &unit);

		ಆದರೆ (type == CSS_MIN_ಎತ್ತರ_ಗಣ) {
			ಆದರೆ (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_ಎತ್ತರ = 0;
			} ಇಲ್ಲದಿದ್ದರೆ {
				*min_ಎತ್ತರ = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Inadmissible */
			*min_ಎತ್ತರ = 0;
		}
	}

	ಗಾಗಿ (i = 0; i != 4; i++) {
		ಆದರೆ (margin) {
			ಎಣಿಕೆ css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed ಮೌಲ್ಯ = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &ಮೌಲ್ಯ, &unit);

			ಆದರೆ (type == CSS_MARGIN_ಗಣ) {
				ಆದರೆ (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ,
							ಲಭ್ಯ_ಅಗಲ);
				} ಇಲ್ಲದಿದ್ದರೆ {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							ಮೌಲ್ಯ, unit));
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				margin[i] = AUTO;
			}
		}

		ಆದರೆ (padding) {
			css_fixed ಮೌಲ್ಯ = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &ಮೌಲ್ಯ, &unit);

			ಆದರೆ (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ,
						ಲಭ್ಯ_ಅಗಲ);
			} ಇಲ್ಲದಿದ್ದರೆ {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		ಆದರೆ (border && box->type != BOX_ಕೋಷ್ಟಕ_CELL) {
			ಎಣಿಕೆ css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed ಮೌಲ್ಯ = 0;
			css_unit unit = CSS_UNIT_PX;

			border_ಅಗಲ_funcs[i](style, &ಮೌಲ್ಯ, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			ಆದರೆ (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].ಅಗಲ = 0;
			ಇಲ್ಲದಿದ್ದರೆ
				border[i].ಅಗಲ = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			ಆದರೆ (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_ಕೋಷ್ಟಕ ||
					 box->type == BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು ||
					 box->type == BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು))
				border[i].ಅಗಲ = 0;
		}
	}
}

#ಆದರೆ_ಅಂತ್ಯ
