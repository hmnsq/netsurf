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
		கட்டமைப்பு box *block,
		முழு viewport_height,
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
		கட்டமைப்பு box *table,
		முழு available_width,
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
		கட்டமைப்பு box *flex,
		முழு available_width,
		html_content *content);

வகைவரை uint8_t (*css_len_func)(
		மாறிலி css_computed_style *style,
		css_fixed *length, css_unit *unit);
வகைவரை uint8_t (*css_border_style_func)(
		மாறிலி css_computed_style *style);
வகைவரை uint8_t (*css_border_color_func)(
		மாறிலி css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
வெளி மாறிலி css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
வெளி மாறிலி css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
வெளி மாறிலி css_len_func border_width_funcs[4];

/** Array of per-side access functions for computed style border styles. */
வெளி மாறிலி css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
வெளி மாறிலி css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
நிலையான உள்வரி bool lh__box_is_float_box(மாறிலி கட்டமைப்பு box *b)
{
	திரும்பு b->type == BOX_FLOAT_இடது ||
	       b->type == BOX_FLOAT_வலது;
}

/** Layout helper: Check whether box takes part in inline flow. */
நிலையான உள்வரி bool lh__box_is_inline_flow(மாறிலி கட்டமைப்பு box *b)
{
	திரும்பு b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_TEXT ||
	       b->type == BOX_INLINE_END;
}

/** Layout helper: Check whether box takes part in inline flow. */
நிலையான உள்வரி bool lh__box_is_flex_container(மாறிலி கட்டமைப்பு box *b)
{
	திரும்பு b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
நிலையான உள்வரி bool lh__box_is_flex_item(மாறிலி கட்டமைப்பு box *b)
{
	திரும்பு (b->parent != வெற்று) && lh__box_is_flex_container(b->parent);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
நிலையான உள்வரி bool lh__box_is_inline_level(மாறிலி கட்டமைப்பு box *b)
{
	திரும்பு lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
நிலையான உள்வரி bool lh__box_is_inline_content(மாறிலி கட்டமைப்பு box *b)
{
	திரும்பு lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
நிலையான உள்வரி bool lh__box_is_object(மாறிலி கட்டமைப்பு box *b)
{
	திரும்பு b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
நிலையான உள்வரி bool lh__box_is_replace(மாறிலி கட்டமைப்பு box *b)
{
	திரும்பு b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
நிலையான உள்வரி bool lh__have_border(
		எண்ணிக்கை box_side side,
		மாறிலி css_computed_style *style)
{
	திரும்பு border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

நிலையான உள்வரி bool lh__box_is_absolute(மாறிலி கட்டமைப்பு box *b)
{
	திரும்பு css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

நிலையான உள்வரி bool lh__flex_main_is_horizontal(மாறிலி கட்டமைப்பு box *flex)
{
	மாறிலி css_computed_style *style = flex->style;

	ப்ரதிஜ்ஞா(style != வெற்று);

	தேர்வு (css_computed_flex_direction(style)) {
	இயல்பு:                        /* Fallthrough. */
	நிலை CSS_FLEX_DIRECTION_ROW:    /* Fallthrough. */
	நிலை CSS_FLEX_DIRECTION_ROW_REVERSE:
		திரும்பு true;
	நிலை CSS_FLEX_DIRECTION_COLUMN: /* Fallthrough. */
	நிலை CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		திரும்பு false;
	}
}

நிலையான உள்வரி bool lh__flex_direction_reversed(மாறிலி கட்டமைப்பு box *flex)
{
	தேர்வு (css_computed_flex_direction(flex->style)) {
	இயல்பு:                             /* Fallthrough. */
	நிலை CSS_FLEX_DIRECTION_ROW_REVERSE: /* Fallthrough. */
	நிலை CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		திரும்பு true;
	நிலை CSS_FLEX_DIRECTION_ROW: /* Fallthrough. */
	நிலை CSS_FLEX_DIRECTION_COLUMN:
		திரும்பு false;
	}
}

நிலையான உள்வரி முழு lh__non_auto_margin(மாறிலி கட்டமைப்பு box *b, எண்ணிக்கை box_side side)
{
	திரும்பு (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

நிலையான உள்வரி முழு lh__delta_outer_height(மாறிலி கட்டமைப்பு box *b)
{
	திரும்பு b->padding[உச்சி_2] +
	       b->padding[BOTTOM] +
	       b->border[உச்சி_2].width +
	       b->border[BOTTOM].width +
	       lh__non_auto_margin(b, உச்சி_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

நிலையான உள்வரி முழு lh__delta_outer_width(மாறிலி கட்டமைப்பு box *b)
{
	திரும்பு b->padding[இடது_2] +
	       b->padding[வலது_2] +
	       b->border[இடது_2].width +
	       b->border[வலது_2].width +
	       lh__non_auto_margin(b, இடது_2) +
	       lh__non_auto_margin(b, வலது_2);
}

நிலையான உள்வரி முழு lh__delta_outer_main(
		மாறிலி கட்டமைப்பு box *flex,
		மாறிலி கட்டமைப்பு box *b)
{
	எனில் (lh__flex_main_is_horizontal(flex)) {
		திரும்பு lh__delta_outer_width(b);
	} இல்லையெனில் {
		திரும்பு lh__delta_outer_height(b);
	}
}

நிலையான உள்வரி முழு lh__delta_outer_cross(
		மாறிலி கட்டமைப்பு box *flex,
		மாறிலி கட்டமைப்பு box *b)
{
	எனில் (lh__flex_main_is_horizontal(flex) == false) {
		திரும்பு lh__delta_outer_width(b);
	} இல்லையெனில் {
		திரும்பு lh__delta_outer_height(b);
	}
}

நிலையான உள்வரி முழு *lh__box_size_main_ptr(
		bool horizontal,
		கட்டமைப்பு box *b)
{
	திரும்பு horizontal ? &b->width : &b->height;
}

நிலையான உள்வரி முழு *lh__box_size_cross_ptr(
		bool horizontal,
		கட்டமைப்பு box *b)
{
	திரும்பு horizontal ? &b->height : &b->width;
}

நிலையான உள்வரி முழு lh__box_size_main(
		bool horizontal,
		மாறிலி கட்டமைப்பு box *b)
{
	திரும்பு horizontal ? b->width : b->height;
}

நிலையான உள்வரி முழு lh__box_size_cross(
		bool horizontal,
		மாறிலி கட்டமைப்பு box *b)
{
	திரும்பு horizontal ? b->height : b->width;
}

நிலையான உள்வரி bool lh__box_size_cross_is_auto(
		bool horizontal,
		கட்டமைப்பு box *b)
{
	css_fixed length;
	css_unit unit;

	எனில் (horizontal) {
		திரும்பு css_computed_height(b->style,
				&length, &unit) == CSS_HEIGHT_AUTO;
	} இல்லையெனில் {
		திரும்பு css_computed_width(b->style,
				&length, &unit) == CSS_WIDTH_AUTO;
	}
}

நிலையான உள்வரி எண்ணிக்கை css_align_self_e lh__box_align_self(
		மாறிலி கட்டமைப்பு box *flex,
		மாறிலி கட்டமைப்பு box *item)
{
	எண்ணிக்கை css_align_self_e align_self = css_computed_align_self(item->style);

	எனில் (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_பொருட்கள்(flex->style);
	}

	திரும்பு align_self;
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
நிலையான உள்வரி வெறுமை calculate_mbp_width(
		மாறிலி css_unit_ctx *unit_len_ctx,
		மாறிலி css_computed_style *style,
		unsigned முழு side,
		bool margin,
		bool border,
		bool padding,
		முழு *fixed,
		தசம *frac)
{
	css_fixed மதிப்பு = 0;
	css_unit unit = CSS_UNIT_PX;

	ப்ரதிஜ்ஞா(style);

	/* margin */
	எனில் (margin) {
		எண்ணிக்கை css_margin_e type;

		type = margin_funcs[side](style, &மதிப்பு, &unit);
		எனில் (type == CSS_MARGIN_SET) {
			எனில் (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(மதிப்பு, F_100));
			} இல்லையெனில் {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						மதிப்பு, unit));
			}
		}
	}

	/* border */
	எனில் (border) {
		எனில் (lh__have_border(side, style)) {
			border_width_funcs[side](style, &மதிப்பு, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					மதிப்பு, unit));
		}
	}

	/* padding */
	எனில் (padding) {
		padding_funcs[side](style, &மதிப்பு, &unit);
		எனில் (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(மதிப்பு, F_100));
		} இல்லையெனில் {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					மதிப்பு, unit));
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
நிலையான உள்வரி வெறுமை layout_handle_box_sizing(
		மாறிலி css_unit_ctx *unit_len_ctx,
		மாறிலி கட்டமைப்பு box *box,
		முழு available_width,
		bool setwidth,
		முழு *dimension)
{
	எண்ணிக்கை css_box_sizing_e bs;

	ப்ரதிஜ்ஞா(box && box->style);

	bs = css_computed_box_sizing(box->style);

	எனில் (bs == CSS_BOX_SIZING_BORDER_BOX) {
		முழு orig = *dimension;
		முழு fixed = 0;
		தசம frac = 0;

		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? இடது_2 : உச்சி_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? வலது_2 : BOTTOM,
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
நிலையான உள்வரி வெறுமை layout_find_dimensions(
		மாறிலி css_unit_ctx *unit_len_ctx,
		முழு available_width,
		முழு viewport_height,
		மாறிலி கட்டமைப்பு box *box,
		மாறிலி css_computed_style *style,
		முழு *width,
		முழு *height,
		முழு *max_width,
		முழு *min_width,
		முழு *max_height,
		முழு *min_height,
		முழு margin[4],
		முழு padding[4],
		கட்டமைப்பு box_border border[4])
{
	கட்டமைப்பு box *containing_block = வெற்று;
	unsigned முழு i;

	எனில் (width) {
		எனில் (css_computed_width_px(style, unit_len_ctx,
				available_width, width) == CSS_WIDTH_SET) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, width);
		} இல்லையெனில் {
			*width = AUTO;
		}
	}

	எனில் (height) {
		எண்ணிக்கை css_height_e htype;
		css_fixed மதிப்பு = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_height(style, &மதிப்பு, &unit);

		எனில் (htype == CSS_HEIGHT_SET) {
			எனில் (unit == CSS_UNIT_PCT) {
				எண்ணிக்கை css_height_e cbhtype;

				எனில் (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->parent) {
					/* Box is absolutely positioned */
					ப்ரதிஜ்ஞா(box->float_container);
					containing_block = box->float_container;
				} இல்லையெனில் எனில் (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_இடது ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_வலது)) {
					/* Box is a float */
					ப்ரதிஜ்ஞா(box->parent &&
						box->parent->parent &&
						box->parent->parent->parent);

					containing_block =
						box->parent->parent->parent;
				} இல்லையெனில் எனில் (box->parent && box->parent->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_block = box->parent;
				} இல்லையெனில் எனில் (box->parent && box->parent->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					ப்ரதிஜ்ஞா(box->parent->parent);
					containing_block = box->parent->parent;
				}

				எனில் (containing_block) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_height(
							containing_block->style,
							&f, &u);
				}

				எனில் (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*height = FPCT_OF_INT_TOINT(மதிப்பு,
						containing_block->height);
				} இல்லையெனில் எனில் ((!box->parent ||
						!box->parent->parent) &&
						viewport_height >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*height = FPCT_OF_INT_TOINT(மதிப்பு,
							viewport_height);
				} இல்லையெனில் {
					/* precentage height not permissible
					 * treat height as auto */
					*height = AUTO;
				}
			} இல்லையெனில் {
				*height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						மதிப்பு, unit));
			}
		} இல்லையெனில் {
			*height = AUTO;
		}

		எனில் (*height != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, false, height);
		}
	}

	எனில் (max_width) {
		எண்ணிக்கை css_max_width_e type;
		css_fixed மதிப்பு = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_width(style, &மதிப்பு, &unit);

		எனில் (type == CSS_MAX_WIDTH_SET) {
			எனில் (unit == CSS_UNIT_PCT) {
				*max_width = FPCT_OF_INT_TOINT(மதிப்பு,
						available_width);
			} இல்லையெனில் {
				*max_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						மதிப்பு, unit));
			}
		} இல்லையெனில் {
			/* Inadmissible */
			*max_width = -1;
		}

		எனில் (*max_width != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, max_width);
		}
	}

	எனில் (min_width) {
		எண்ணிக்கை css_min_width_e type;
		css_fixed மதிப்பு = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_width(style, &மதிப்பு, &unit);

		எனில் (type == CSS_MIN_WIDTH_SET) {
			எனில் (unit == CSS_UNIT_PCT) {
				*min_width = FPCT_OF_INT_TOINT(மதிப்பு,
						available_width);
			} இல்லையெனில் {
				*min_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						மதிப்பு, unit));
			}
		} இல்லையெனில் {
			/* Inadmissible */
			*min_width = 0;
		}

		எனில் (*min_width != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, min_width);
		}
	}

	எனில் (max_height) {
		எண்ணிக்கை css_max_height_e type;
		css_fixed மதிப்பு = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_height(style, &மதிப்பு, &unit);

		எனில் (type == CSS_MAX_HEIGHT_SET) {
			எனில் (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_height = -1;
			} இல்லையெனில் {
				*max_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						மதிப்பு, unit));
			}
		} இல்லையெனில் {
			/* Inadmissible */
			*max_height = -1;
		}
	}

	எனில் (min_height) {
		எண்ணிக்கை css_min_height_e type;
		css_fixed மதிப்பு = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_height(style, &மதிப்பு, &unit);

		எனில் (type == CSS_MIN_HEIGHT_SET) {
			எனில் (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_height = 0;
			} இல்லையெனில் {
				*min_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						மதிப்பு, unit));
			}
		} இல்லையெனில் {
			/* Inadmissible */
			*min_height = 0;
		}
	}

	க்கு (i = 0; i != 4; i++) {
		எனில் (margin) {
			எண்ணிக்கை css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed மதிப்பு = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &மதிப்பு, &unit);

			எனில் (type == CSS_MARGIN_SET) {
				எனில் (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_OF_INT_TOINT(மதிப்பு,
							available_width);
				} இல்லையெனில் {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							மதிப்பு, unit));
				}
			} இல்லையெனில் {
				margin[i] = AUTO;
			}
		}

		எனில் (padding) {
			css_fixed மதிப்பு = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &மதிப்பு, &unit);

			எனில் (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_OF_INT_TOINT(மதிப்பு,
						available_width);
			} இல்லையெனில் {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						மதிப்பு, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		எனில் (border && box->type != BOX_TABLE_CELL) {
			எண்ணிக்கை css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed மதிப்பு = 0;
			css_unit unit = CSS_UNIT_PX;

			border_width_funcs[i](style, &மதிப்பு, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			எனில் (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].width = 0;
			இல்லையெனில்
				border[i].width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						மதிப்பு, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			எனில் (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_TABLE ||
					 box->type == BOX_TABLE_ROW_GROUP ||
					 box->type == BOX_TABLE_ROW))
				border[i].width = 0;
		}
	}
}

#endif
