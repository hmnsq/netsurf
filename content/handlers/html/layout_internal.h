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
		sa_nracanaa box *block,
		จำนวนเต็ม viewport_height,
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
		sa_nracanaa box *table,
		จำนวนเต็ม available_width,
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
		sa_nracanaa box *flex,
		จำนวนเต็ม available_width,
		html_content *content);

typedef uint8_t (*css_len_func)(
		const css_computed_style *style,
		css_fixed *length, css_unit *unit);
typedef uint8_t (*css_border_style_func)(
		const css_computed_style *style);
typedef uint8_t (*css_border_color_func)(
		const css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
extern const css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
extern const css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
extern const css_len_func border_width_funcs[4];

/** Array of per-side access functions for computed style border styles. */
extern const css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
extern const css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
static anta_hstha bool lh__box_is_float_box(const sa_nracanaa box *b)
{
	คืน b->type == BOX_FLOAT_LEFT ||
	       b->type == BOX_FLOAT_RIGHT;
}

/** Layout helper: Check whether box takes part in inline flow. */
static anta_hstha bool lh__box_is_inline_flow(const sa_nracanaa box *b)
{
	คืน b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_TEXT ||
	       b->type == BOX_INLINE_END;
}

/** Layout helper: Check whether box takes part in inline flow. */
static anta_hstha bool lh__box_is_flex_container(const sa_nracanaa box *b)
{
	คืน b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
static anta_hstha bool lh__box_is_flex_item(const sa_nracanaa box *b)
{
	คืน (b->parent != NULL) && lh__box_is_flex_container(b->parent);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
static anta_hstha bool lh__box_is_inline_level(const sa_nracanaa box *b)
{
	คืน lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
static anta_hstha bool lh__box_is_inline_content(const sa_nracanaa box *b)
{
	คืน lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
static anta_hstha bool lh__box_is_object(const sa_nracanaa box *b)
{
	คืน b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
static anta_hstha bool lh__box_is_replace(const sa_nracanaa box *b)
{
	คืน b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
static anta_hstha bool lh__have_border(
		ga_nanaa box_side side,
		const css_computed_style *style)
{
	คืน border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

static anta_hstha bool lh__box_is_absolute(const sa_nracanaa box *b)
{
	คืน css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

static anta_hstha bool lh__flex_main_is_horizontal(const sa_nracanaa box *flex)
{
	const css_computed_style *style = flex->style;

	assert(style != NULL);

	switch (css_computed_flex_direction(style)) {
	default:                        /* Fallthrough. */
	pak_sa CSS_FLEX_DIRECTION_ROW:    /* Fallthrough. */
	pak_sa CSS_FLEX_DIRECTION_ROW_REVERSE:
		คืน true;
	pak_sa CSS_FLEX_DIRECTION_COLUMN: /* Fallthrough. */
	pak_sa CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		คืน false;
	}
}

static anta_hstha bool lh__flex_direction_reversed(const sa_nracanaa box *flex)
{
	switch (css_computed_flex_direction(flex->style)) {
	default:                             /* Fallthrough. */
	pak_sa CSS_FLEX_DIRECTION_ROW_REVERSE: /* Fallthrough. */
	pak_sa CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		คืน true;
	pak_sa CSS_FLEX_DIRECTION_ROW: /* Fallthrough. */
	pak_sa CSS_FLEX_DIRECTION_COLUMN:
		คืน false;
	}
}

static anta_hstha จำนวนเต็ม lh__non_auto_margin(const sa_nracanaa box *b, ga_nanaa box_side side)
{
	คืน (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

static anta_hstha จำนวนเต็ม lh__delta_outer_height(const sa_nracanaa box *b)
{
	คืน b->padding[TOP] +
	       b->padding[BOTTOM] +
	       b->border[TOP].width +
	       b->border[BOTTOM].width +
	       lh__non_auto_margin(b, TOP) +
	       lh__non_auto_margin(b, BOTTOM);
}

static anta_hstha จำนวนเต็ม lh__delta_outer_width(const sa_nracanaa box *b)
{
	คืน b->padding[LEFT] +
	       b->padding[RIGHT] +
	       b->border[LEFT].width +
	       b->border[RIGHT].width +
	       lh__non_auto_margin(b, LEFT) +
	       lh__non_auto_margin(b, RIGHT);
}

static anta_hstha จำนวนเต็ม lh__delta_outer_main(
		const sa_nracanaa box *flex,
		const sa_nracanaa box *b)
{
	ถ้า (lh__flex_main_is_horizontal(flex)) {
		คืน lh__delta_outer_width(b);
	} มิฉะนั้น {
		คืน lh__delta_outer_height(b);
	}
}

static anta_hstha จำนวนเต็ม lh__delta_outer_cross(
		const sa_nracanaa box *flex,
		const sa_nracanaa box *b)
{
	ถ้า (lh__flex_main_is_horizontal(flex) == false) {
		คืน lh__delta_outer_width(b);
	} มิฉะนั้น {
		คืน lh__delta_outer_height(b);
	}
}

static anta_hstha จำนวนเต็ม *lh__box_size_main_ptr(
		bool horizontal,
		sa_nracanaa box *b)
{
	คืน horizontal ? &b->width : &b->height;
}

static anta_hstha จำนวนเต็ม *lh__box_size_cross_ptr(
		bool horizontal,
		sa_nracanaa box *b)
{
	คืน horizontal ? &b->height : &b->width;
}

static anta_hstha จำนวนเต็ม lh__box_size_main(
		bool horizontal,
		const sa_nracanaa box *b)
{
	คืน horizontal ? b->width : b->height;
}

static anta_hstha จำนวนเต็ม lh__box_size_cross(
		bool horizontal,
		const sa_nracanaa box *b)
{
	คืน horizontal ? b->height : b->width;
}

static anta_hstha bool lh__box_size_cross_is_auto(
		bool horizontal,
		sa_nracanaa box *b)
{
	css_fixed length;
	css_unit unit;

	ถ้า (horizontal) {
		คืน css_computed_height(b->style,
				&length, &unit) == CSS_HEIGHT_AUTO;
	} มิฉะนั้น {
		คืน css_computed_width(b->style,
				&length, &unit) == CSS_WIDTH_AUTO;
	}
}

static anta_hstha ga_nanaa css_align_self_e lh__box_align_self(
		const sa_nracanaa box *flex,
		const sa_nracanaa box *item)
{
	ga_nanaa css_align_self_e align_self = css_computed_align_self(item->style);

	ถ้า (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_items(flex->style);
	}

	คืน align_self;
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
static anta_hstha void calculate_mbp_width(
		const css_unit_ctx *unit_len_ctx,
		const css_computed_style *style,
		unsigned จำนวนเต็ม side,
		bool margin,
		bool border,
		bool padding,
		จำนวนเต็ม *fixed,
		float *frac)
{
	css_fixed value = 0;
	css_unit unit = CSS_UNIT_PX;

	assert(style);

	/* margin */
	ถ้า (margin) {
		ga_nanaa css_margin_e type;

		type = margin_funcs[side](style, &value, &unit);
		ถ้า (type == CSS_MARGIN_SET) {
			ถ้า (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(value, F_100));
			} มิฉะนั้น {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		}
	}

	/* border */
	ถ้า (border) {
		ถ้า (lh__have_border(side, style)) {
			border_width_funcs[side](style, &value, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					value, unit));
		}
	}

	/* padding */
	ถ้า (padding) {
		padding_funcs[side](style, &value, &unit);
		ถ้า (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(value, F_100));
		} มิฉะนั้น {
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
static anta_hstha void layout_handle_box_sizing(
		const css_unit_ctx *unit_len_ctx,
		const sa_nracanaa box *box,
		จำนวนเต็ม available_width,
		bool setwidth,
		จำนวนเต็ม *dimension)
{
	ga_nanaa css_box_sizing_e bs;

	assert(box && box->style);

	bs = css_computed_box_sizing(box->style);

	ถ้า (bs == CSS_BOX_SIZING_BORDER_BOX) {
		จำนวนเต็ม orig = *dimension;
		จำนวนเต็ม fixed = 0;
		float frac = 0;

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
static anta_hstha void layout_find_dimensions(
		const css_unit_ctx *unit_len_ctx,
		จำนวนเต็ม available_width,
		จำนวนเต็ม viewport_height,
		const sa_nracanaa box *box,
		const css_computed_style *style,
		จำนวนเต็ม *width,
		จำนวนเต็ม *height,
		จำนวนเต็ม *max_width,
		จำนวนเต็ม *min_width,
		จำนวนเต็ม *max_height,
		จำนวนเต็ม *min_height,
		จำนวนเต็ม margin[4],
		จำนวนเต็ม padding[4],
		sa_nracanaa box_border border[4])
{
	sa_nracanaa box *containing_block = NULL;
	unsigned จำนวนเต็ม i;

	ถ้า (width) {
		ถ้า (css_computed_width_px(style, unit_len_ctx,
				available_width, width) == CSS_WIDTH_SET) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, width);
		} มิฉะนั้น {
			*width = AUTO;
		}
	}

	ถ้า (height) {
		ga_nanaa css_height_e htype;
		css_fixed value = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_height(style, &value, &unit);

		ถ้า (htype == CSS_HEIGHT_SET) {
			ถ้า (unit == CSS_UNIT_PCT) {
				ga_nanaa css_height_e cbhtype;

				ถ้า (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->parent) {
					/* Box is absolutely positioned */
					assert(box->float_container);
					containing_block = box->float_container;
				} มิฉะนั้น ถ้า (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_LEFT ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_RIGHT)) {
					/* Box is a float */
					assert(box->parent &&
						box->parent->parent &&
						box->parent->parent->parent);

					containing_block =
						box->parent->parent->parent;
				} มิฉะนั้น ถ้า (box->parent && box->parent->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_block = box->parent;
				} มิฉะนั้น ถ้า (box->parent && box->parent->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					assert(box->parent->parent);
					containing_block = box->parent->parent;
				}

				ถ้า (containing_block) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_height(
							containing_block->style,
							&f, &u);
				}

				ถ้า (containing_block &&
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
				} มิฉะนั้น ถ้า ((!box->parent ||
						!box->parent->parent) &&
						viewport_height >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*height = FPCT_OF_INT_TOINT(value,
							viewport_height);
				} มิฉะนั้น {
					/* precentage height not permissible
					 * treat height as auto */
					*height = AUTO;
				}
			} มิฉะนั้น {
				*height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		} มิฉะนั้น {
			*height = AUTO;
		}

		ถ้า (*height != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, false, height);
		}
	}

	ถ้า (max_width) {
		ga_nanaa css_max_width_e type;
		css_fixed value = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_width(style, &value, &unit);

		ถ้า (type == CSS_MAX_WIDTH_SET) {
			ถ้า (unit == CSS_UNIT_PCT) {
				*max_width = FPCT_OF_INT_TOINT(value,
						available_width);
			} มิฉะนั้น {
				*max_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		} มิฉะนั้น {
			/* Inadmissible */
			*max_width = -1;
		}

		ถ้า (*max_width != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, max_width);
		}
	}

	ถ้า (min_width) {
		ga_nanaa css_min_width_e type;
		css_fixed value = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_width(style, &value, &unit);

		ถ้า (type == CSS_MIN_WIDTH_SET) {
			ถ้า (unit == CSS_UNIT_PCT) {
				*min_width = FPCT_OF_INT_TOINT(value,
						available_width);
			} มิฉะนั้น {
				*min_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		} มิฉะนั้น {
			/* Inadmissible */
			*min_width = 0;
		}

		ถ้า (*min_width != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, min_width);
		}
	}

	ถ้า (max_height) {
		ga_nanaa css_max_height_e type;
		css_fixed value = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_height(style, &value, &unit);

		ถ้า (type == CSS_MAX_HEIGHT_SET) {
			ถ้า (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_height = -1;
			} มิฉะนั้น {
				*max_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		} มิฉะนั้น {
			/* Inadmissible */
			*max_height = -1;
		}
	}

	ถ้า (min_height) {
		ga_nanaa css_min_height_e type;
		css_fixed value = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_height(style, &value, &unit);

		ถ้า (type == CSS_MIN_HEIGHT_SET) {
			ถ้า (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_height = 0;
			} มิฉะนั้น {
				*min_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		} มิฉะนั้น {
			/* Inadmissible */
			*min_height = 0;
		}
	}

	สำหรับ (i = 0; i != 4; i++) {
		ถ้า (margin) {
			ga_nanaa css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed value = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &value, &unit);

			ถ้า (type == CSS_MARGIN_SET) {
				ถ้า (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_OF_INT_TOINT(value,
							available_width);
				} มิฉะนั้น {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							value, unit));
				}
			} มิฉะนั้น {
				margin[i] = AUTO;
			}
		}

		ถ้า (padding) {
			css_fixed value = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &value, &unit);

			ถ้า (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_OF_INT_TOINT(value,
						available_width);
			} มิฉะนั้น {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		ถ้า (border && box->type != BOX_TABLE_CELL) {
			ga_nanaa css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed value = 0;
			css_unit unit = CSS_UNIT_PX;

			border_width_funcs[i](style, &value, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			ถ้า (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].width = 0;
			มิฉะนั้น
				border[i].width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						value, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			ถ้า (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_TABLE ||
					 box->type == BOX_TABLE_ROW_GROUP ||
					 box->type == BOX_TABLE_ROW))
				border[i].width = 0;
		}
	}
}

#endif
