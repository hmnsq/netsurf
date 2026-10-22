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
		بنية box *block,
		صحيح viewport_height,
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
		بنية box *table,
		صحيح available_width,
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
		بنية box *flex,
		صحيح available_width,
		html_content *content);

تعريفنوع uint8_t (*css_len_func)(
		ثابت css_computed_style *style,
		css_fixed *length, css_unit *unit);
تعريفنوع uint8_t (*css_border_style_func)(
		ثابت css_computed_style *style);
تعريفنوع uint8_t (*css_border_color_func)(
		ثابت css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
خارجي ثابت css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
خارجي ثابت css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
خارجي ثابت css_len_func border_width_funcs[4];

/** Array of per-side access functions for computed style border styles. */
خارجي ثابت css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
خارجي ثابت css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
ساكن مضمن bool lh__box_is_float_box(ثابت بنية box *b)
{
	החזר b->type == BOX_FLOAT_LEFT ||
	       b->type == BOX_FLOAT_RIGHT;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساكن مضمن bool lh__box_is_inline_flow(ثابت بنية box *b)
{
	החזר b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_TEXT ||
	       b->type == BOX_INLINE_END;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساكن مضمن bool lh__box_is_flex_container(ثابت بنية box *b)
{
	החזר b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساكن مضمن bool lh__box_is_flex_item(ثابت بنية box *b)
{
	החזר (b->parent != عدم) && lh__box_is_flex_container(b->parent);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
ساكن مضمن bool lh__box_is_inline_level(ثابت بنية box *b)
{
	החזר lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
ساكن مضمن bool lh__box_is_inline_content(ثابت بنية box *b)
{
	החזר lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
ساكن مضمن bool lh__box_is_object(ثابت بنية box *b)
{
	החזר b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
ساكن مضمن bool lh__box_is_replace(ثابت بنية box *b)
{
	החזר b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
ساكن مضمن bool lh__have_border(
		تعداد box_side side,
		ثابت css_computed_style *style)
{
	החזר border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

ساكن مضمن bool lh__box_is_absolute(ثابت بنية box *b)
{
	החזר css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

ساكن مضمن bool lh__flex_main_is_horizontal(ثابت بنية box *flex)
{
	ثابت css_computed_style *style = flex->style;

	أكد(style != عدم);

	بدل (css_computed_flex_direction(style)) {
	افتراضي:                        /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_ROW:    /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_ROW_REVERSE:
		החזר true;
	حالة CSS_FLEX_DIRECTION_COLUMN: /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		החזר false;
	}
}

ساكن مضمن bool lh__flex_direction_reversed(ثابت بنية box *flex)
{
	بدل (css_computed_flex_direction(flex->style)) {
	افتراضي:                             /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_ROW_REVERSE: /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		החזר true;
	حالة CSS_FLEX_DIRECTION_ROW: /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_COLUMN:
		החזר false;
	}
}

ساكن مضمن صحيح lh__non_auto_margin(ثابت بنية box *b, تعداد box_side side)
{
	החזר (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

ساكن مضمن صحيح lh__delta_outer_height(ثابت بنية box *b)
{
	החזר b->padding[TOP] +
	       b->padding[BOTTOM] +
	       b->border[TOP].width +
	       b->border[BOTTOM].width +
	       lh__non_auto_margin(b, TOP) +
	       lh__non_auto_margin(b, BOTTOM);
}

ساكن مضمن صحيح lh__delta_outer_width(ثابت بنية box *b)
{
	החזר b->padding[LEFT] +
	       b->padding[RIGHT] +
	       b->border[LEFT].width +
	       b->border[RIGHT].width +
	       lh__non_auto_margin(b, LEFT) +
	       lh__non_auto_margin(b, RIGHT);
}

ساكن مضمن صحيح lh__delta_outer_main(
		ثابت بنية box *flex,
		ثابت بنية box *b)
{
	אם (lh__flex_main_is_horizontal(flex)) {
		החזר lh__delta_outer_width(b);
	} אחרת {
		החזר lh__delta_outer_height(b);
	}
}

ساكن مضمن صحيح lh__delta_outer_cross(
		ثابت بنية box *flex,
		ثابت بنية box *b)
{
	אם (lh__flex_main_is_horizontal(flex) == false) {
		החזר lh__delta_outer_width(b);
	} אחרת {
		החזר lh__delta_outer_height(b);
	}
}

ساكن مضمن صحيح *lh__box_size_main_ptr(
		bool horizontal,
		بنية box *b)
{
	החזר horizontal ? &b->width : &b->height;
}

ساكن مضمن صحيح *lh__box_size_cross_ptr(
		bool horizontal,
		بنية box *b)
{
	החזר horizontal ? &b->height : &b->width;
}

ساكن مضمن صحيح lh__box_size_main(
		bool horizontal,
		ثابت بنية box *b)
{
	החזר horizontal ? b->width : b->height;
}

ساكن مضمن صحيح lh__box_size_cross(
		bool horizontal,
		ثابت بنية box *b)
{
	החזר horizontal ? b->height : b->width;
}

ساكن مضمن bool lh__box_size_cross_is_auto(
		bool horizontal,
		بنية box *b)
{
	css_fixed length;
	css_unit unit;

	אם (horizontal) {
		החזר css_computed_height(b->style,
				&length, &unit) == CSS_HEIGHT_AUTO;
	} אחרת {
		החזר css_computed_width(b->style,
				&length, &unit) == CSS_WIDTH_AUTO;
	}
}

ساكن مضمن تعداد css_align_self_e lh__box_align_self(
		ثابت بنية box *flex,
		ثابت بنية box *item)
{
	تعداد css_align_self_e align_self = css_computed_align_self(item->style);

	אם (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_items(flex->style);
	}

	החזר align_self;
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
ساكن مضمن فراغ calculate_mbp_width(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style,
		unsigned صحيح side,
		bool margin,
		bool border,
		bool padding,
		صحيح *fixed,
		عائم *frac)
{
	css_fixed value = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(style);

	/* margin */
	אם (margin) {
		تعداد css_margin_e type;

		type = margin_funcs[side](style, &value, &unit);
		אם (type == CSS_MARGIN_SET) {
			אם (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(value, F_100));
			} אחרת {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		}
	}

	/* border */
	אם (border) {
		אם (lh__have_border(side, style)) {
			border_width_funcs[side](style, &value, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					value, unit));
		}
	}

	/* padding */
	אם (padding) {
		padding_funcs[side](style, &value, &unit);
		אם (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(value, F_100));
		} אחרת {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					value, unit));
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
ساكن مضمن فراغ layout_handle_box_sizing(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت بنية box *box,
		صحيح available_width,
		bool setwidth,
		صحيح *dimension)
{
	تعداد css_box_sizing_e bs;

	أكد(box && box->style);

	bs = css_computed_box_sizing(box->style);

	אם (bs == CSS_BOX_SIZING_BORDER_BOX) {
		صحيح orig = *dimension;
		صحيح fixed = 0;
		عائم frac = 0;

		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? LEFT : TOP,
				false, true, true, &fixed, &frac);
		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? RIGHT : BOTTOM,
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
ساكن مضمن فراغ layout_find_dimensions(
		ثابت css_unit_ctx *unit_len_ctx,
		صحيح available_width,
		صحيح viewport_height,
		ثابت بنية box *box,
		ثابت css_computed_style *style,
		صحيح *width,
		صحيح *height,
		صحيح *max_width,
		صحيح *min_width,
		صحيح *max_height,
		صحيح *min_height,
		صحيح margin[4],
		صحيح padding[4],
		بنية box_border border[4])
{
	بنية box *containing_block = عدم;
	unsigned صحيح i;

	אם (width) {
		אם (css_computed_width_px(style, unit_len_ctx,
				available_width, width) == CSS_WIDTH_SET) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, width);
		} אחרת {
			*width = AUTO;
		}
	}

	אם (height) {
		تعداد css_height_e htype;
		css_fixed value = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_height(style, &value, &unit);

		אם (htype == CSS_HEIGHT_SET) {
			אם (unit == CSS_UNIT_PCT) {
				تعداد css_height_e cbhtype;

				אם (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->parent) {
					/* Box is absolutely positioned */
					أكد(box->float_container);
					containing_block = box->float_container;
				} אחרת אם (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_LEFT ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_RIGHT)) {
					/* Box is a float */
					أكد(box->parent &&
						box->parent->parent &&
						box->parent->parent->parent);

					containing_block =
						box->parent->parent->parent;
				} אחרת אם (box->parent && box->parent->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_block = box->parent;
				} אחרת אם (box->parent && box->parent->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					أكد(box->parent->parent);
					containing_block = box->parent->parent;
				}

				אם (containing_block) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_height(
							containing_block->style,
							&f, &u);
				}

				אם (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*height = FPCT_OF_INT_TOINT(value,
						containing_block->height);
				} אחרת אם ((!box->parent ||
						!box->parent->parent) &&
						viewport_height >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*height = FPCT_OF_INT_TOINT(value,
							viewport_height);
				} אחרת {
					/* precentage height not permissible
					 * treat height as auto */
					*height = AUTO;
				}
			} אחרת {
				*height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		} אחרת {
			*height = AUTO;
		}

		אם (*height != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, false, height);
		}
	}

	אם (max_width) {
		تعداد css_max_width_e type;
		css_fixed value = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_width(style, &value, &unit);

		אם (type == CSS_MAX_WIDTH_SET) {
			אם (unit == CSS_UNIT_PCT) {
				*max_width = FPCT_OF_INT_TOINT(value,
						available_width);
			} אחרת {
				*max_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		} אחרת {
			/* Inadmissible */
			*max_width = -1;
		}

		אם (*max_width != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, max_width);
		}
	}

	אם (min_width) {
		تعداد css_min_width_e type;
		css_fixed value = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_width(style, &value, &unit);

		אם (type == CSS_MIN_WIDTH_SET) {
			אם (unit == CSS_UNIT_PCT) {
				*min_width = FPCT_OF_INT_TOINT(value,
						available_width);
			} אחרת {
				*min_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		} אחרת {
			/* Inadmissible */
			*min_width = 0;
		}

		אם (*min_width != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, min_width);
		}
	}

	אם (max_height) {
		تعداد css_max_height_e type;
		css_fixed value = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_height(style, &value, &unit);

		אם (type == CSS_MAX_HEIGHT_SET) {
			אם (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_height = -1;
			} אחרת {
				*max_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		} אחרת {
			/* Inadmissible */
			*max_height = -1;
		}
	}

	אם (min_height) {
		تعداد css_min_height_e type;
		css_fixed value = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_height(style, &value, &unit);

		אם (type == CSS_MIN_HEIGHT_SET) {
			אם (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_height = 0;
			} אחרת {
				*min_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		} אחרת {
			/* Inadmissible */
			*min_height = 0;
		}
	}

	עבור (i = 0; i != 4; i++) {
		אם (margin) {
			تعداد css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed value = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &value, &unit);

			אם (type == CSS_MARGIN_SET) {
				אם (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_OF_INT_TOINT(value,
							available_width);
				} אחרת {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							value, unit));
				}
			} אחרת {
				margin[i] = AUTO;
			}
		}

		אם (padding) {
			css_fixed value = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &value, &unit);

			אם (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_OF_INT_TOINT(value,
						available_width);
			} אחרת {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		אם (border && box->type != BOX_TABLE_CELL) {
			تعداد css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed value = 0;
			css_unit unit = CSS_UNIT_PX;

			border_width_funcs[i](style, &value, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			אם (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].width = 0;
			אחרת
				border[i].width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			אם (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_TABLE ||
					 box->type == BOX_TABLE_ROW_GROUP ||
					 box->type == BOX_TABLE_ROW))
				border[i].width = 0;
		}
	}
}

#endif
