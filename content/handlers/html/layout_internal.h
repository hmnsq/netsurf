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

#ifndef NETSURF_HTML_LAYOUT_INTERNAL_H
#define NETSURF_HTML_LAYOUT_INTERNAL_H

#define AUTO INT_MIN

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
bool layout_block_context(
		ಸಂರಚನಾ box *block,
		ಪೂರ್ಣಾಙ್ಕ viewport_height,
		html_content *content);

/**
 * Layout a table.
 *
 * \param  table            table to layout
 * \param  available_width  width of containing block
 * \param  content          memory pool for any new boxes
 * \return  true on success, false on memory exhaustion
 */
bool layout_table(
		ಸಂರಚನಾ box *table,
		ಪೂರ್ಣಾಙ್ಕ available_width,
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
		ಸಂರಚನಾ box *flex,
		ಪೂರ್ಣಾಙ್ಕ available_width,
		html_content *content);

ಪ್ರಕಾರನಿರ್ದೇಶ uint8_t (*css_len_func)(
		ಧ್ರುವ css_computed_style *style,
		css_fixed *length, css_unit *unit);
ಪ್ರಕಾರನಿರ್ದೇಶ uint8_t (*css_border_style_func)(
		ಧ್ರುವ css_computed_style *style);
ಪ್ರಕಾರನಿರ್ದೇಶ uint8_t (*css_border_color_func)(
		ಧ್ರುವ css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
ಬಾಹ್ಯ ಧ್ರುವ css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
ಬಾಹ್ಯ ಧ್ರುವ css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
ಬಾಹ್ಯ ಧ್ರುವ css_len_func border_width_funcs[4];

/** Array of per-side access functions for computed style border styles. */
ಬಾಹ್ಯ ಧ್ರುವ css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
ಬಾಹ್ಯ ಧ್ರುವ css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool lh__box_is_float_box(ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ b->type == BOX_FLOAT_ಎಡ ||
	       b->type == BOX_FLOAT_ಬಲ;
}

/** Layout helper: Check whether box takes part in inline flow. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool lh__box_is_inline_flow(ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_TEXT ||
	       b->type == BOX_INLINE_END;
}

/** Layout helper: Check whether box takes part in inline flow. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool lh__box_is_flex_container(ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool lh__box_is_flex_item(ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ (b->parent != ರಿಕ್ತ) && lh__box_is_flex_container(b->parent);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool lh__box_is_inline_level(ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool lh__box_is_inline_content(ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool lh__box_is_object(ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool lh__box_is_replace(ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool lh__have_border(
		ಗಣನಾ box_side side,
		ಧ್ರುವ css_computed_style *style)
{
	ಪ್ರತ್ಯಾಗಮ border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool lh__box_is_absolute(ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool lh__flex_main_is_horizontal(ಧ್ರುವ ಸಂರಚನಾ box *flex)
{
	ಧ್ರುವ css_computed_style *style = flex->style;

	ಪ್ರತಿಜ್ಞಾ(style != ರಿಕ್ತ);

	ವಿಕಲ್ಪ (css_computed_flex_direction(style)) {
	ಸ್ವಭಾವ:                        /* Fallthrough. */
	ಪಕ್ಷ CSS_FLEX_DIRECTION_ROW:    /* Fallthrough. */
	ಪಕ್ಷ CSS_FLEX_DIRECTION_ROW_REVERSE:
		ಪ್ರತ್ಯಾಗಮ true;
	ಪಕ್ಷ CSS_FLEX_DIRECTION_COLUMN: /* Fallthrough. */
	ಪಕ್ಷ CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		ಪ್ರತ್ಯಾಗಮ false;
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool lh__flex_direction_reversed(ಧ್ರುವ ಸಂರಚನಾ box *flex)
{
	ವಿಕಲ್ಪ (css_computed_flex_direction(flex->style)) {
	ಸ್ವಭಾವ:                             /* Fallthrough. */
	ಪಕ್ಷ CSS_FLEX_DIRECTION_ROW_REVERSE: /* Fallthrough. */
	ಪಕ್ಷ CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		ಪ್ರತ್ಯಾಗಮ true;
	ಪಕ್ಷ CSS_FLEX_DIRECTION_ROW: /* Fallthrough. */
	ಪಕ್ಷ CSS_FLEX_DIRECTION_COLUMN:
		ಪ್ರತ್ಯಾಗಮ false;
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ lh__non_auto_margin(ಧ್ರುವ ಸಂರಚನಾ box *b, ಗಣನಾ box_side side)
{
	ಪ್ರತ್ಯಾಗಮ (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ lh__delta_outer_height(ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ b->padding[ತುದಿ_2] +
	       b->padding[BOTTOM] +
	       b->border[ತುದಿ_2].width +
	       b->border[BOTTOM].width +
	       lh__non_auto_margin(b, ತುದಿ_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ lh__delta_outer_width(ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ b->padding[ಎಡ_2] +
	       b->padding[ಬಲ_2] +
	       b->border[ಎಡ_2].width +
	       b->border[ಬಲ_2].width +
	       lh__non_auto_margin(b, ಎಡ_2) +
	       lh__non_auto_margin(b, ಬಲ_2);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ lh__delta_outer_main(
		ಧ್ರುವ ಸಂರಚನಾ box *flex,
		ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಯದಿ (lh__flex_main_is_horizontal(flex)) {
		ಪ್ರತ್ಯಾಗಮ lh__delta_outer_width(b);
	} ಅನ್ಯಥಾ {
		ಪ್ರತ್ಯಾಗಮ lh__delta_outer_height(b);
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ lh__delta_outer_cross(
		ಧ್ರುವ ಸಂರಚನಾ box *flex,
		ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಯದಿ (lh__flex_main_is_horizontal(flex) == false) {
		ಪ್ರತ್ಯಾಗಮ lh__delta_outer_width(b);
	} ಅನ್ಯಥಾ {
		ಪ್ರತ್ಯಾಗಮ lh__delta_outer_height(b);
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ *lh__box_size_main_ptr(
		bool horizontal,
		ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ horizontal ? &b->width : &b->height;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ *lh__box_size_cross_ptr(
		bool horizontal,
		ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ horizontal ? &b->height : &b->width;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ lh__box_size_main(
		bool horizontal,
		ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ horizontal ? b->width : b->height;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ lh__box_size_cross(
		bool horizontal,
		ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಪ್ರತ್ಯಾಗಮ horizontal ? b->height : b->width;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool lh__box_size_cross_is_auto(
		bool horizontal,
		ಸಂರಚನಾ box *b)
{
	css_fixed length;
	css_unit unit;

	ಯದಿ (horizontal) {
		ಪ್ರತ್ಯಾಗಮ css_computed_height(b->style,
				&length, &unit) == CSS_HEIGHT_AUTO;
	} ಅನ್ಯಥಾ {
		ಪ್ರತ್ಯಾಗಮ css_computed_width(b->style,
				&length, &unit) == CSS_WIDTH_AUTO;
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಗಣನಾ css_align_self_e lh__box_align_self(
		ಧ್ರುವ ಸಂರಚನಾ box *flex,
		ಧ್ರುವ ಸಂರಚನಾ box *item)
{
	ಗಣನಾ css_align_self_e align_self = css_computed_align_self(item->style);

	ಯದಿ (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_ವಸ್ತುಗಳು(flex->style);
	}

	ಪ್ರತ್ಯಾಗಮ align_self;
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
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ calculate_mbp_width(
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಧ್ರುವ css_computed_style *style,
		unsigned ಪೂರ್ಣಾಙ್ಕ side,
		bool margin,
		bool border,
		bool padding,
		ಪೂರ್ಣಾಙ್ಕ *fixed,
		ಭಿನ್ನ *frac)
{
	css_fixed ಮೌಲ್ಯ = 0;
	css_unit unit = CSS_UNIT_PX;

	ಪ್ರತಿಜ್ಞಾ(style);

	/* margin */
	ಯದಿ (margin) {
		ಗಣನಾ css_margin_e type;

		type = margin_funcs[side](style, &ಮೌಲ್ಯ, &unit);
		ಯದಿ (type == CSS_MARGIN_SET) {
			ಯದಿ (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(ಮೌಲ್ಯ, F_100));
			} ಅನ್ಯಥಾ {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		}
	}

	/* border */
	ಯದಿ (border) {
		ಯದಿ (lh__have_border(side, style)) {
			border_width_funcs[side](style, &ಮೌಲ್ಯ, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		}
	}

	/* padding */
	ಯದಿ (padding) {
		padding_funcs[side](style, &ಮೌಲ್ಯ, &unit);
		ಯದಿ (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(ಮೌಲ್ಯ, F_100));
		} ಅನ್ಯಥಾ {
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
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ layout_handle_box_sizing(
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಧ್ರುವ ಸಂರಚನಾ box *box,
		ಪೂರ್ಣಾಙ್ಕ available_width,
		bool setwidth,
		ಪೂರ್ಣಾಙ್ಕ *dimension)
{
	ಗಣನಾ css_box_sizing_e bs;

	ಪ್ರತಿಜ್ಞಾ(box && box->style);

	bs = css_computed_box_sizing(box->style);

	ಯದಿ (bs == CSS_BOX_SIZING_BORDER_BOX) {
		ಪೂರ್ಣಾಙ್ಕ orig = *dimension;
		ಪೂರ್ಣಾಙ್ಕ fixed = 0;
		ಭಿನ್ನ frac = 0;

		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? ಎಡ_2 : ತುದಿ_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? ಬಲ_2 : BOTTOM,
				false, true, true, &fixed, &frac);
		orig -= frac * available_width + fixed;
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
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ layout_find_dimensions(
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಪೂರ್ಣಾಙ್ಕ available_width,
		ಪೂರ್ಣಾಙ್ಕ viewport_height,
		ಧ್ರುವ ಸಂರಚನಾ box *box,
		ಧ್ರುವ css_computed_style *style,
		ಪೂರ್ಣಾಙ್ಕ *width,
		ಪೂರ್ಣಾಙ್ಕ *height,
		ಪೂರ್ಣಾಙ್ಕ *max_width,
		ಪೂರ್ಣಾಙ್ಕ *min_width,
		ಪೂರ್ಣಾಙ್ಕ *max_height,
		ಪೂರ್ಣಾಙ್ಕ *min_height,
		ಪೂರ್ಣಾಙ್ಕ margin[4],
		ಪೂರ್ಣಾಙ್ಕ padding[4],
		ಸಂರಚನಾ box_border border[4])
{
	ಸಂರಚನಾ box *containing_block = ರಿಕ್ತ;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (width) {
		ಯದಿ (css_computed_width_px(style, unit_len_ctx,
				available_width, width) == CSS_WIDTH_SET) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, width);
		} ಅನ್ಯಥಾ {
			*width = AUTO;
		}
	}

	ಯದಿ (height) {
		ಗಣನಾ css_height_e htype;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_height(style, &ಮೌಲ್ಯ, &unit);

		ಯದಿ (htype == CSS_HEIGHT_SET) {
			ಯದಿ (unit == CSS_UNIT_PCT) {
				ಗಣನಾ css_height_e cbhtype;

				ಯದಿ (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->parent) {
					/* Box is absolutely positioned */
					ಪ್ರತಿಜ್ಞಾ(box->float_container);
					containing_block = box->float_container;
				} ಅನ್ಯಥಾ ಯದಿ (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_ಎಡ ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_ಬಲ)) {
					/* Box is a float */
					ಪ್ರತಿಜ್ಞಾ(box->parent &&
						box->parent->parent &&
						box->parent->parent->parent);

					containing_block =
						box->parent->parent->parent;
				} ಅನ್ಯಥಾ ಯದಿ (box->parent && box->parent->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_block = box->parent;
				} ಅನ್ಯಥಾ ಯದಿ (box->parent && box->parent->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					ಪ್ರತಿಜ್ಞಾ(box->parent->parent);
					containing_block = box->parent->parent;
				}

				ಯದಿ (containing_block) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_height(
							containing_block->style,
							&f, &u);
				}

				ಯದಿ (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*height = FPCT_OF_INT_TOINT(ಮೌಲ್ಯ,
						containing_block->height);
				} ಅನ್ಯಥಾ ಯದಿ ((!box->parent ||
						!box->parent->parent) &&
						viewport_height >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*height = FPCT_OF_INT_TOINT(ಮೌಲ್ಯ,
							viewport_height);
				} ಅನ್ಯಥಾ {
					/* precentage height not permissible
					 * treat height as auto */
					*height = AUTO;
				}
			} ಅನ್ಯಥಾ {
				*height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		} ಅನ್ಯಥಾ {
			*height = AUTO;
		}

		ಯದಿ (*height != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, false, height);
		}
	}

	ಯದಿ (max_width) {
		ಗಣನಾ css_max_width_e type;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_width(style, &ಮೌಲ್ಯ, &unit);

		ಯದಿ (type == CSS_MAX_WIDTH_SET) {
			ಯದಿ (unit == CSS_UNIT_PCT) {
				*max_width = FPCT_OF_INT_TOINT(ಮೌಲ್ಯ,
						available_width);
			} ಅನ್ಯಥಾ {
				*max_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		} ಅನ್ಯಥಾ {
			/* Inadmissible */
			*max_width = -1;
		}

		ಯದಿ (*max_width != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, max_width);
		}
	}

	ಯದಿ (min_width) {
		ಗಣನಾ css_min_width_e type;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_width(style, &ಮೌಲ್ಯ, &unit);

		ಯದಿ (type == CSS_MIN_WIDTH_SET) {
			ಯದಿ (unit == CSS_UNIT_PCT) {
				*min_width = FPCT_OF_INT_TOINT(ಮೌಲ್ಯ,
						available_width);
			} ಅನ್ಯಥಾ {
				*min_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		} ಅನ್ಯಥಾ {
			/* Inadmissible */
			*min_width = 0;
		}

		ಯದಿ (*min_width != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, min_width);
		}
	}

	ಯದಿ (max_height) {
		ಗಣನಾ css_max_height_e type;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_height(style, &ಮೌಲ್ಯ, &unit);

		ಯದಿ (type == CSS_MAX_HEIGHT_SET) {
			ಯದಿ (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_height = -1;
			} ಅನ್ಯಥಾ {
				*max_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		} ಅನ್ಯಥಾ {
			/* Inadmissible */
			*max_height = -1;
		}
	}

	ಯದಿ (min_height) {
		ಗಣನಾ css_min_height_e type;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_height(style, &ಮೌಲ್ಯ, &unit);

		ಯದಿ (type == CSS_MIN_HEIGHT_SET) {
			ಯದಿ (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_height = 0;
			} ಅನ್ಯಥಾ {
				*min_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		} ಅನ್ಯಥಾ {
			/* Inadmissible */
			*min_height = 0;
		}
	}

	ಪ್ರತಿ (i = 0; i != 4; i++) {
		ಯದಿ (margin) {
			ಗಣನಾ css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed ಮೌಲ್ಯ = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &ಮೌಲ್ಯ, &unit);

			ಯದಿ (type == CSS_MARGIN_SET) {
				ಯದಿ (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_OF_INT_TOINT(ಮೌಲ್ಯ,
							available_width);
				} ಅನ್ಯಥಾ {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							ಮೌಲ್ಯ, unit));
				}
			} ಅನ್ಯಥಾ {
				margin[i] = AUTO;
			}
		}

		ಯದಿ (padding) {
			css_fixed ಮೌಲ್ಯ = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &ಮೌಲ್ಯ, &unit);

			ಯದಿ (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_OF_INT_TOINT(ಮೌಲ್ಯ,
						available_width);
			} ಅನ್ಯಥಾ {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		ಯದಿ (border && box->type != BOX_TABLE_CELL) {
			ಗಣನಾ css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed ಮೌಲ್ಯ = 0;
			css_unit unit = CSS_UNIT_PX;

			border_width_funcs[i](style, &ಮೌಲ್ಯ, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			ಯದಿ (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].width = 0;
			ಅನ್ಯಥಾ
				border[i].width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			ಯದಿ (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_TABLE ||
					 box->type == BOX_TABLE_ROW_GROUP ||
					 box->type == BOX_TABLE_ROW))
				border[i].width = 0;
		}
	}
}

#endif
