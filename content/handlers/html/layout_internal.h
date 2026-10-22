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
		ساختار box *block,
		صحیح viewport_height,
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
		ساختار box *table,
		صحیح available_width,
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
		ساختار box *flex,
		صحیح available_width,
		html_content *content);

تعریفنوع uint8_t (*css_len_func)(
		ثابت css_computed_style *style,
		css_fixed *length, css_unit *unit);
تعریفنوع uint8_t (*css_border_style_func)(
		ثابت css_computed_style *style);
تعریفنوع uint8_t (*css_border_color_func)(
		ثابت css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
بیرونی ثابت css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
بیرونی ثابت css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
بیرونی ثابت css_len_func border_width_funcs[4];

/** Array of per-side access functions for computed style border styles. */
بیرونی ثابت css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
بیرونی ثابت css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
ایستا درونخطی bool lh__box_is_float_box(ثابت ساختار box *b)
{
	بازگشت b->type == BOX_FLOAT_چپ ||
	       b->type == BOX_FLOAT_راست;
}

/** Layout helper: Check whether box takes part in inline flow. */
ایستا درونخطی bool lh__box_is_inline_flow(ثابت ساختار box *b)
{
	بازگشت b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_TEXT ||
	       b->type == BOX_INLINE_END;
}

/** Layout helper: Check whether box takes part in inline flow. */
ایستا درونخطی bool lh__box_is_flex_container(ثابت ساختار box *b)
{
	بازگشت b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
ایستا درونخطی bool lh__box_is_flex_item(ثابت ساختار box *b)
{
	بازگشت (b->parent != پوچ) && lh__box_is_flex_container(b->parent);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
ایستا درونخطی bool lh__box_is_inline_level(ثابت ساختار box *b)
{
	بازگشت lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
ایستا درونخطی bool lh__box_is_inline_content(ثابت ساختار box *b)
{
	بازگشت lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
ایستا درونخطی bool lh__box_is_object(ثابت ساختار box *b)
{
	بازگشت b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
ایستا درونخطی bool lh__box_is_replace(ثابت ساختار box *b)
{
	بازگشت b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
ایستا درونخطی bool lh__have_border(
		شمارش box_side side,
		ثابت css_computed_style *style)
{
	بازگشت border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

ایستا درونخطی bool lh__box_is_absolute(ثابت ساختار box *b)
{
	بازگشت css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

ایستا درونخطی bool lh__flex_main_is_horizontal(ثابت ساختار box *flex)
{
	ثابت css_computed_style *style = flex->style;

	أكد(style != پوچ);

	انتخاب (css_computed_flex_direction(style)) {
	پیشفرض:                        /* Fallthrough. */
	حالت CSS_FLEX_DIRECTION_ROW:    /* Fallthrough. */
	حالت CSS_FLEX_DIRECTION_ROW_REVERSE:
		بازگشت true;
	حالت CSS_FLEX_DIRECTION_COLUMN: /* Fallthrough. */
	حالت CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		بازگشت false;
	}
}

ایستا درونخطی bool lh__flex_direction_reversed(ثابت ساختار box *flex)
{
	انتخاب (css_computed_flex_direction(flex->style)) {
	پیشفرض:                             /* Fallthrough. */
	حالت CSS_FLEX_DIRECTION_ROW_REVERSE: /* Fallthrough. */
	حالت CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		بازگشت true;
	حالت CSS_FLEX_DIRECTION_ROW: /* Fallthrough. */
	حالت CSS_FLEX_DIRECTION_COLUMN:
		بازگشت false;
	}
}

ایستا درونخطی صحیح lh__non_auto_margin(ثابت ساختار box *b, شمارش box_side side)
{
	بازگشت (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

ایستا درونخطی صحیح lh__delta_outer_height(ثابت ساختار box *b)
{
	بازگشت b->padding[بالاترین_2] +
	       b->padding[BOTTOM] +
	       b->border[بالاترین_2].width +
	       b->border[BOTTOM].width +
	       lh__non_auto_margin(b, بالاترین_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

ایستا درونخطی صحیح lh__delta_outer_width(ثابت ساختار box *b)
{
	بازگشت b->padding[چپ_2] +
	       b->padding[راست_2] +
	       b->border[چپ_2].width +
	       b->border[راست_2].width +
	       lh__non_auto_margin(b, چپ_2) +
	       lh__non_auto_margin(b, راست_2);
}

ایستا درونخطی صحیح lh__delta_outer_main(
		ثابت ساختار box *flex,
		ثابت ساختار box *b)
{
	اگر (lh__flex_main_is_horizontal(flex)) {
		بازگشت lh__delta_outer_width(b);
	} وگرنه {
		بازگشت lh__delta_outer_height(b);
	}
}

ایستا درونخطی صحیح lh__delta_outer_cross(
		ثابت ساختار box *flex,
		ثابت ساختار box *b)
{
	اگر (lh__flex_main_is_horizontal(flex) == false) {
		بازگشت lh__delta_outer_width(b);
	} وگرنه {
		بازگشت lh__delta_outer_height(b);
	}
}

ایستا درونخطی صحیح *lh__box_size_main_ptr(
		bool horizontal,
		ساختار box *b)
{
	بازگشت horizontal ? &b->width : &b->height;
}

ایستا درونخطی صحیح *lh__box_size_cross_ptr(
		bool horizontal,
		ساختار box *b)
{
	بازگشت horizontal ? &b->height : &b->width;
}

ایستا درونخطی صحیح lh__box_size_main(
		bool horizontal,
		ثابت ساختار box *b)
{
	بازگشت horizontal ? b->width : b->height;
}

ایستا درونخطی صحیح lh__box_size_cross(
		bool horizontal,
		ثابت ساختار box *b)
{
	بازگشت horizontal ? b->height : b->width;
}

ایستا درونخطی bool lh__box_size_cross_is_auto(
		bool horizontal,
		ساختار box *b)
{
	css_fixed length;
	css_unit unit;

	اگر (horizontal) {
		بازگشت css_computed_height(b->style,
				&length, &unit) == CSS_HEIGHT_AUTO;
	} وگرنه {
		بازگشت css_computed_width(b->style,
				&length, &unit) == CSS_WIDTH_AUTO;
	}
}

ایستا درونخطی شمارش css_align_self_e lh__box_align_self(
		ثابت ساختار box *flex,
		ثابت ساختار box *item)
{
	شمارش css_align_self_e align_self = css_computed_align_self(item->style);

	اگر (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_اقلام(flex->style);
	}

	بازگشت align_self;
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
ایستا درونخطی تهی calculate_mbp_width(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style,
		unsigned صحیح side,
		bool margin,
		bool border,
		bool padding,
		صحیح *fixed,
		اعشاری *frac)
{
	css_fixed مقدار = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(style);

	/* margin */
	اگر (margin) {
		شمارش css_margin_e type;

		type = margin_funcs[side](style, &مقدار, &unit);
		اگر (type == CSS_MARGIN_SET) {
			اگر (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(مقدار, F_100));
			} وگرنه {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		}
	}

	/* border */
	اگر (border) {
		اگر (lh__have_border(side, style)) {
			border_width_funcs[side](style, &مقدار, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					مقدار, unit));
		}
	}

	/* padding */
	اگر (padding) {
		padding_funcs[side](style, &مقدار, &unit);
		اگر (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(مقدار, F_100));
		} وگرنه {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					مقدار, unit));
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
ایستا درونخطی تهی layout_handle_box_sizing(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت ساختار box *box,
		صحیح available_width,
		bool setwidth,
		صحیح *dimension)
{
	شمارش css_box_sizing_e bs;

	أكد(box && box->style);

	bs = css_computed_box_sizing(box->style);

	اگر (bs == CSS_BOX_SIZING_BORDER_BOX) {
		صحیح orig = *dimension;
		صحیح fixed = 0;
		اعشاری frac = 0;

		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? چپ_2 : بالاترین_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? راست_2 : BOTTOM,
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
ایستا درونخطی تهی layout_find_dimensions(
		ثابت css_unit_ctx *unit_len_ctx,
		صحیح available_width,
		صحیح viewport_height,
		ثابت ساختار box *box,
		ثابت css_computed_style *style,
		صحیح *width,
		صحیح *height,
		صحیح *max_width,
		صحیح *min_width,
		صحیح *max_height,
		صحیح *min_height,
		صحیح margin[4],
		صحیح padding[4],
		ساختار box_border border[4])
{
	ساختار box *containing_block = پوچ;
	unsigned صحیح i;

	اگر (width) {
		اگر (css_computed_width_px(style, unit_len_ctx,
				available_width, width) == CSS_WIDTH_SET) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, width);
		} وگرنه {
			*width = AUTO;
		}
	}

	اگر (height) {
		شمارش css_height_e htype;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_height(style, &مقدار, &unit);

		اگر (htype == CSS_HEIGHT_SET) {
			اگر (unit == CSS_UNIT_PCT) {
				شمارش css_height_e cbhtype;

				اگر (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->parent) {
					/* Box is absolutely positioned */
					أكد(box->float_container);
					containing_block = box->float_container;
				} وگرنه اگر (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_چپ ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_راست)) {
					/* Box is a float */
					أكد(box->parent &&
						box->parent->parent &&
						box->parent->parent->parent);

					containing_block =
						box->parent->parent->parent;
				} وگرنه اگر (box->parent && box->parent->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_block = box->parent;
				} وگرنه اگر (box->parent && box->parent->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					أكد(box->parent->parent);
					containing_block = box->parent->parent;
				}

				اگر (containing_block) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_height(
							containing_block->style,
							&f, &u);
				}

				اگر (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*height = FPCT_OF_INT_TOINT(مقدار,
						containing_block->height);
				} وگرنه اگر ((!box->parent ||
						!box->parent->parent) &&
						viewport_height >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*height = FPCT_OF_INT_TOINT(مقدار,
							viewport_height);
				} وگرنه {
					/* precentage height not permissible
					 * treat height as auto */
					*height = AUTO;
				}
			} وگرنه {
				*height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		} وگرنه {
			*height = AUTO;
		}

		اگر (*height != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, false, height);
		}
	}

	اگر (max_width) {
		شمارش css_max_width_e type;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_width(style, &مقدار, &unit);

		اگر (type == CSS_MAX_WIDTH_SET) {
			اگر (unit == CSS_UNIT_PCT) {
				*max_width = FPCT_OF_INT_TOINT(مقدار,
						available_width);
			} وگرنه {
				*max_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		} وگرنه {
			/* Inadmissible */
			*max_width = -1;
		}

		اگر (*max_width != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, max_width);
		}
	}

	اگر (min_width) {
		شمارش css_min_width_e type;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_width(style, &مقدار, &unit);

		اگر (type == CSS_MIN_WIDTH_SET) {
			اگر (unit == CSS_UNIT_PCT) {
				*min_width = FPCT_OF_INT_TOINT(مقدار,
						available_width);
			} وگرنه {
				*min_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		} وگرنه {
			/* Inadmissible */
			*min_width = 0;
		}

		اگر (*min_width != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, min_width);
		}
	}

	اگر (max_height) {
		شمارش css_max_height_e type;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_height(style, &مقدار, &unit);

		اگر (type == CSS_MAX_HEIGHT_SET) {
			اگر (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_height = -1;
			} وگرنه {
				*max_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		} وگرنه {
			/* Inadmissible */
			*max_height = -1;
		}
	}

	اگر (min_height) {
		شمارش css_min_height_e type;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_height(style, &مقدار, &unit);

		اگر (type == CSS_MIN_HEIGHT_SET) {
			اگر (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_height = 0;
			} وگرنه {
				*min_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		} وگرنه {
			/* Inadmissible */
			*min_height = 0;
		}
	}

	برای (i = 0; i != 4; i++) {
		اگر (margin) {
			شمارش css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed مقدار = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &مقدار, &unit);

			اگر (type == CSS_MARGIN_SET) {
				اگر (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_OF_INT_TOINT(مقدار,
							available_width);
				} وگرنه {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							مقدار, unit));
				}
			} وگرنه {
				margin[i] = AUTO;
			}
		}

		اگر (padding) {
			css_fixed مقدار = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &مقدار, &unit);

			اگر (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_OF_INT_TOINT(مقدار,
						available_width);
			} وگرنه {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		اگر (border && box->type != BOX_TABLE_CELL) {
			شمارش css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed مقدار = 0;
			css_unit unit = CSS_UNIT_PX;

			border_width_funcs[i](style, &مقدار, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			اگر (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].width = 0;
			وگرنه
				border[i].width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			اگر (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_TABLE ||
					 box->type == BOX_TABLE_ROW_GROUP ||
					 box->type == BOX_TABLE_ROW))
				border[i].width = 0;
		}
	}
}

#endif
