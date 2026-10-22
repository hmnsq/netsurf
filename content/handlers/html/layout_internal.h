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
		ସଂରଚନା box *block,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା viewport_height,
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
		ସଂରଚନା box *table,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width,
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
		ସଂରଚନା box *flex,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width,
		html_content *content);

ପ୍ରକାରବ୍ୟାଖ୍ୟା uint8_t (*css_len_func)(
		ଅଚଳ css_computed_style *style,
		css_fixed *length, css_unit *unit);
ପ୍ରକାରବ୍ୟାଖ୍ୟା uint8_t (*css_border_style_func)(
		ଅଚଳ css_computed_style *style);
ପ୍ରକାରବ୍ୟାଖ୍ୟା uint8_t (*css_border_color_func)(
		ଅଚଳ css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
ବାହ୍ୟ ଅଚଳ css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
ବାହ୍ୟ ଅଚଳ css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
ବାହ୍ୟ ଅଚଳ css_len_func border_width_funcs[4];

/** Array of per-side access functions for computed style border styles. */
ବାହ୍ୟ ଅଚଳ css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
ବାହ୍ୟ ଅଚଳ css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool lh__box_is_float_box(ଅଚଳ ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ b->type == BOX_FLOAT_ବାମ ||
	       b->type == BOX_FLOAT_ଡାହାଣ;
}

/** Layout helper: Check whether box takes part in inline flow. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool lh__box_is_inline_flow(ଅଚଳ ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_TEXT ||
	       b->type == BOX_INLINE_END;
}

/** Layout helper: Check whether box takes part in inline flow. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool lh__box_is_flex_container(ଅଚଳ ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool lh__box_is_flex_item(ଅଚଳ ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ (b->parent != ଖାଲି) && lh__box_is_flex_container(b->parent);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool lh__box_is_inline_level(ଅଚଳ ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool lh__box_is_inline_content(ଅଚଳ ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool lh__box_is_object(ଅଚଳ ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool lh__box_is_replace(ଅଚଳ ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool lh__have_border(
		ଗଣନା box_side side,
		ଅଚଳ css_computed_style *style)
{
	ଫେରନ୍ତୁ border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool lh__box_is_absolute(ଅଚଳ ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool lh__flex_main_is_horizontal(ଅଚଳ ସଂରଚନା box *flex)
{
	ଅଚଳ css_computed_style *style = flex->style;

	ପ୍ରତିଜ୍ଞା(style != ଖାଲି);

	ବାଛ (css_computed_flex_direction(style)) {
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:                        /* Fallthrough. */
	ସ୍ଥିତି CSS_FLEX_DIRECTION_ROW:    /* Fallthrough. */
	ସ୍ଥିତି CSS_FLEX_DIRECTION_ROW_REVERSE:
		ଫେରନ୍ତୁ true;
	ସ୍ଥିତି CSS_FLEX_DIRECTION_COLUMN: /* Fallthrough. */
	ସ୍ଥିତି CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		ଫେରନ୍ତୁ false;
	}
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool lh__flex_direction_reversed(ଅଚଳ ସଂରଚନା box *flex)
{
	ବାଛ (css_computed_flex_direction(flex->style)) {
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:                             /* Fallthrough. */
	ସ୍ଥିତି CSS_FLEX_DIRECTION_ROW_REVERSE: /* Fallthrough. */
	ସ୍ଥିତି CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		ଫେରନ୍ତୁ true;
	ସ୍ଥିତି CSS_FLEX_DIRECTION_ROW: /* Fallthrough. */
	ସ୍ଥିତି CSS_FLEX_DIRECTION_COLUMN:
		ଫେରନ୍ତୁ false;
	}
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା lh__non_auto_margin(ଅଚଳ ସଂରଚନା box *b, ଗଣନା box_side side)
{
	ଫେରନ୍ତୁ (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା lh__delta_outer_height(ଅଚଳ ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ b->padding[ଶୀର୍ଷ_2] +
	       b->padding[BOTTOM] +
	       b->border[ଶୀର୍ଷ_2].width +
	       b->border[BOTTOM].width +
	       lh__non_auto_margin(b, ଶୀର୍ଷ_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା lh__delta_outer_width(ଅଚଳ ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ b->padding[ବାମ_2] +
	       b->padding[ଡାହାଣ_2] +
	       b->border[ବାମ_2].width +
	       b->border[ଡାହାଣ_2].width +
	       lh__non_auto_margin(b, ବାମ_2) +
	       lh__non_auto_margin(b, ଡାହାଣ_2);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା lh__delta_outer_main(
		ଅଚଳ ସଂରଚନା box *flex,
		ଅଚଳ ସଂରଚନା box *b)
{
	ଯଦି (lh__flex_main_is_horizontal(flex)) {
		ଫେରନ୍ତୁ lh__delta_outer_width(b);
	} ଅନ୍ୟଥା {
		ଫେରନ୍ତୁ lh__delta_outer_height(b);
	}
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା lh__delta_outer_cross(
		ଅଚଳ ସଂରଚନା box *flex,
		ଅଚଳ ସଂରଚନା box *b)
{
	ଯଦି (lh__flex_main_is_horizontal(flex) == false) {
		ଫେରନ୍ତୁ lh__delta_outer_width(b);
	} ଅନ୍ୟଥା {
		ଫେରନ୍ତୁ lh__delta_outer_height(b);
	}
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା *lh__box_size_main_ptr(
		bool horizontal,
		ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ horizontal ? &b->width : &b->height;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା *lh__box_size_cross_ptr(
		bool horizontal,
		ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ horizontal ? &b->height : &b->width;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା lh__box_size_main(
		bool horizontal,
		ଅଚଳ ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ horizontal ? b->width : b->height;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା lh__box_size_cross(
		bool horizontal,
		ଅଚଳ ସଂରଚନା box *b)
{
	ଫେରନ୍ତୁ horizontal ? b->height : b->width;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool lh__box_size_cross_is_auto(
		bool horizontal,
		ସଂରଚନା box *b)
{
	css_fixed length;
	css_unit unit;

	ଯଦି (horizontal) {
		ଫେରନ୍ତୁ css_computed_height(b->style,
				&length, &unit) == CSS_HEIGHT_AUTO;
	} ଅନ୍ୟଥା {
		ଫେରନ୍ତୁ css_computed_width(b->style,
				&length, &unit) == CSS_WIDTH_AUTO;
	}
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଗଣନା css_align_self_e lh__box_align_self(
		ଅଚଳ ସଂରଚନା box *flex,
		ଅଚଳ ସଂରଚନା box *item)
{
	ଗଣନା css_align_self_e align_self = css_computed_align_self(item->style);

	ଯଦି (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_ବସ୍ତୁଗୁଡ଼ିକ(flex->style);
	}

	ଫେରନ୍ତୁ align_self;
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
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ calculate_mbp_width(
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ଅଚଳ css_computed_style *style,
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା side,
		bool margin,
		bool border,
		bool padding,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *fixed,
		ଦଶମିକ *frac)
{
	css_fixed ମୂଲ୍ୟ = 0;
	css_unit unit = CSS_UNIT_PX;

	ପ୍ରତିଜ୍ଞା(style);

	/* margin */
	ଯଦି (margin) {
		ଗଣନା css_margin_e type;

		type = margin_funcs[side](style, &ମୂଲ୍ୟ, &unit);
		ଯଦି (type == CSS_MARGIN_SET) {
			ଯଦି (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(ମୂଲ୍ୟ, F_100));
			} ଅନ୍ୟଥା {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ମୂଲ୍ୟ, unit));
			}
		}
	}

	/* border */
	ଯଦି (border) {
		ଯଦି (lh__have_border(side, style)) {
			border_width_funcs[side](style, &ମୂଲ୍ୟ, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					ମୂଲ୍ୟ, unit));
		}
	}

	/* padding */
	ଯଦି (padding) {
		padding_funcs[side](style, &ମୂଲ୍ୟ, &unit);
		ଯଦି (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(ମୂଲ୍ୟ, F_100));
		} ଅନ୍ୟଥା {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					ମୂଲ୍ୟ, unit));
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
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ layout_handle_box_sizing(
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ଅଚଳ ସଂରଚନା box *box,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width,
		bool setwidth,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *dimension)
{
	ଗଣନା css_box_sizing_e bs;

	ପ୍ରତିଜ୍ଞା(box && box->style);

	bs = css_computed_box_sizing(box->style);

	ଯଦି (bs == CSS_BOX_SIZING_BORDER_BOX) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା orig = *dimension;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fixed = 0;
		ଦଶମିକ frac = 0;

		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? ବାମ_2 : ଶୀର୍ଷ_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? ଡାହାଣ_2 : BOTTOM,
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
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ layout_find_dimensions(
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା viewport_height,
		ଅଚଳ ସଂରଚନା box *box,
		ଅଚଳ css_computed_style *style,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *width,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *height,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *max_width,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *min_width,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *max_height,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *min_height,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା margin[4],
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା padding[4],
		ସଂରଚନା box_border border[4])
{
	ସଂରଚନା box *containing_block = ଖାଲି;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (width) {
		ଯଦି (css_computed_width_px(style, unit_len_ctx,
				available_width, width) == CSS_WIDTH_SET) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, width);
		} ଅନ୍ୟଥା {
			*width = AUTO;
		}
	}

	ଯଦି (height) {
		ଗଣନା css_height_e htype;
		css_fixed ମୂଲ୍ୟ = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_height(style, &ମୂଲ୍ୟ, &unit);

		ଯଦି (htype == CSS_HEIGHT_SET) {
			ଯଦି (unit == CSS_UNIT_PCT) {
				ଗଣନା css_height_e cbhtype;

				ଯଦି (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->parent) {
					/* Box is absolutely positioned */
					ପ୍ରତିଜ୍ଞା(box->float_container);
					containing_block = box->float_container;
				} ଅନ୍ୟଥା ଯଦି (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_ବାମ ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_ଡାହାଣ)) {
					/* Box is a float */
					ପ୍ରତିଜ୍ଞା(box->parent &&
						box->parent->parent &&
						box->parent->parent->parent);

					containing_block =
						box->parent->parent->parent;
				} ଅନ୍ୟଥା ଯଦି (box->parent && box->parent->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_block = box->parent;
				} ଅନ୍ୟଥା ଯଦି (box->parent && box->parent->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					ପ୍ରତିଜ୍ଞା(box->parent->parent);
					containing_block = box->parent->parent;
				}

				ଯଦି (containing_block) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_height(
							containing_block->style,
							&f, &u);
				}

				ଯଦି (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*height = FPCT_OF_INT_TOINT(ମୂଲ୍ୟ,
						containing_block->height);
				} ଅନ୍ୟଥା ଯଦି ((!box->parent ||
						!box->parent->parent) &&
						viewport_height >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*height = FPCT_OF_INT_TOINT(ମୂଲ୍ୟ,
							viewport_height);
				} ଅନ୍ୟଥା {
					/* precentage height not permissible
					 * treat height as auto */
					*height = AUTO;
				}
			} ଅନ୍ୟଥା {
				*height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ମୂଲ୍ୟ, unit));
			}
		} ଅନ୍ୟଥା {
			*height = AUTO;
		}

		ଯଦି (*height != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, false, height);
		}
	}

	ଯଦି (max_width) {
		ଗଣନା css_max_width_e type;
		css_fixed ମୂଲ୍ୟ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_width(style, &ମୂଲ୍ୟ, &unit);

		ଯଦି (type == CSS_MAX_WIDTH_SET) {
			ଯଦି (unit == CSS_UNIT_PCT) {
				*max_width = FPCT_OF_INT_TOINT(ମୂଲ୍ୟ,
						available_width);
			} ଅନ୍ୟଥା {
				*max_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ମୂଲ୍ୟ, unit));
			}
		} ଅନ୍ୟଥା {
			/* Inadmissible */
			*max_width = -1;
		}

		ଯଦି (*max_width != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, max_width);
		}
	}

	ଯଦି (min_width) {
		ଗଣନା css_min_width_e type;
		css_fixed ମୂଲ୍ୟ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_width(style, &ମୂଲ୍ୟ, &unit);

		ଯଦି (type == CSS_MIN_WIDTH_SET) {
			ଯଦି (unit == CSS_UNIT_PCT) {
				*min_width = FPCT_OF_INT_TOINT(ମୂଲ୍ୟ,
						available_width);
			} ଅନ୍ୟଥା {
				*min_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ମୂଲ୍ୟ, unit));
			}
		} ଅନ୍ୟଥା {
			/* Inadmissible */
			*min_width = 0;
		}

		ଯଦି (*min_width != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, min_width);
		}
	}

	ଯଦି (max_height) {
		ଗଣନା css_max_height_e type;
		css_fixed ମୂଲ୍ୟ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_height(style, &ମୂଲ୍ୟ, &unit);

		ଯଦି (type == CSS_MAX_HEIGHT_SET) {
			ଯଦି (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_height = -1;
			} ଅନ୍ୟଥା {
				*max_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ମୂଲ୍ୟ, unit));
			}
		} ଅନ୍ୟଥା {
			/* Inadmissible */
			*max_height = -1;
		}
	}

	ଯଦି (min_height) {
		ଗଣନା css_min_height_e type;
		css_fixed ମୂଲ୍ୟ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_height(style, &ମୂଲ୍ୟ, &unit);

		ଯଦି (type == CSS_MIN_HEIGHT_SET) {
			ଯଦି (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_height = 0;
			} ଅନ୍ୟଥା {
				*min_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ମୂଲ୍ୟ, unit));
			}
		} ଅନ୍ୟଥା {
			/* Inadmissible */
			*min_height = 0;
		}
	}

	ପାଇଁ (i = 0; i != 4; i++) {
		ଯଦି (margin) {
			ଗଣନା css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed ମୂଲ୍ୟ = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &ମୂଲ୍ୟ, &unit);

			ଯଦି (type == CSS_MARGIN_SET) {
				ଯଦି (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_OF_INT_TOINT(ମୂଲ୍ୟ,
							available_width);
				} ଅନ୍ୟଥା {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							ମୂଲ୍ୟ, unit));
				}
			} ଅନ୍ୟଥା {
				margin[i] = AUTO;
			}
		}

		ଯଦି (padding) {
			css_fixed ମୂଲ୍ୟ = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &ମୂଲ୍ୟ, &unit);

			ଯଦି (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_OF_INT_TOINT(ମୂଲ୍ୟ,
						available_width);
			} ଅନ୍ୟଥା {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ମୂଲ୍ୟ, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		ଯଦି (border && box->type != BOX_TABLE_CELL) {
			ଗଣନା css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed ମୂଲ୍ୟ = 0;
			css_unit unit = CSS_UNIT_PX;

			border_width_funcs[i](style, &ମୂଲ୍ୟ, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			ଯଦି (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].width = 0;
			ଅନ୍ୟଥା
				border[i].width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ମୂଲ୍ୟ, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			ଯଦି (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_TABLE ||
					 box->type == BOX_TABLE_ROW_GROUP ||
					 box->type == BOX_TABLE_ROW))
				border[i].width = 0;
		}
	}
}

#endif
