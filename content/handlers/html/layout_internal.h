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
		સંરચના box *block,
		પૂર્ણાંક viewport_height,
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
		સંરચના box *table,
		પૂર્ણાંક available_width,
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
		સંરચના box *flex,
		પૂર્ણાંક available_width,
		html_content *content);

પ્રકારવ્યાખ્યા uint8_t (*css_len_func)(
		અચલ css_computed_style *style,
		css_fixed *length, css_unit *unit);
પ્રકારવ્યાખ્યા uint8_t (*css_border_style_func)(
		અચલ css_computed_style *style);
પ્રકારવ્યાખ્યા uint8_t (*css_border_color_func)(
		અચલ css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
બાહ્ય અચલ css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
બાહ્ય અચલ css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
બાહ્ય અચલ css_len_func border_width_funcs[4];

/** Array of per-side access functions for computed style border styles. */
બાહ્ય અચલ css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
બાહ્ય અચલ css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
સ્થિર અંતઃરેખ bool lh__box_is_float_box(અચલ સંરચના box *b)
{
	પાછા b->type == BOX_FLOAT_ડાબું ||
	       b->type == BOX_FLOAT_જમણું;
}

/** Layout helper: Check whether box takes part in inline flow. */
સ્થિર અંતઃરેખ bool lh__box_is_inline_flow(અચલ સંરચના box *b)
{
	પાછા b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_TEXT ||
	       b->type == BOX_INLINE_END;
}

/** Layout helper: Check whether box takes part in inline flow. */
સ્થિર અંતઃરેખ bool lh__box_is_flex_container(અચલ સંરચના box *b)
{
	પાછા b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
સ્થિર અંતઃરેખ bool lh__box_is_flex_item(અચલ સંરચના box *b)
{
	પાછા (b->parent != ખાલી) && lh__box_is_flex_container(b->parent);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
સ્થિર અંતઃરેખ bool lh__box_is_inline_level(અચલ સંરચના box *b)
{
	પાછા lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
સ્થિર અંતઃરેખ bool lh__box_is_inline_content(અચલ સંરચના box *b)
{
	પાછા lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
સ્થિર અંતઃરેખ bool lh__box_is_object(અચલ સંરચના box *b)
{
	પાછા b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
સ્થિર અંતઃરેખ bool lh__box_is_replace(અચલ સંરચના box *b)
{
	પાછા b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
સ્થિર અંતઃરેખ bool lh__have_border(
		ગણના box_side side,
		અચલ css_computed_style *style)
{
	પાછા border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

સ્થિર અંતઃરેખ bool lh__box_is_absolute(અચલ સંરચના box *b)
{
	પાછા css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

સ્થિર અંતઃરેખ bool lh__flex_main_is_horizontal(અચલ સંરચના box *flex)
{
	અચલ css_computed_style *style = flex->style;

	પ્રતિજ્ઞા(style != ખાલી);

	પસંદગી (css_computed_flex_direction(style)) {
	મૂળભૂત:                        /* Fallthrough. */
	સ્થિતિ CSS_FLEX_DIRECTION_ROW:    /* Fallthrough. */
	સ્થિતિ CSS_FLEX_DIRECTION_ROW_REVERSE:
		પાછા true;
	સ્થિતિ CSS_FLEX_DIRECTION_COLUMN: /* Fallthrough. */
	સ્થિતિ CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		પાછા false;
	}
}

સ્થિર અંતઃરેખ bool lh__flex_direction_reversed(અચલ સંરચના box *flex)
{
	પસંદગી (css_computed_flex_direction(flex->style)) {
	મૂળભૂત:                             /* Fallthrough. */
	સ્થિતિ CSS_FLEX_DIRECTION_ROW_REVERSE: /* Fallthrough. */
	સ્થિતિ CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		પાછા true;
	સ્થિતિ CSS_FLEX_DIRECTION_ROW: /* Fallthrough. */
	સ્થિતિ CSS_FLEX_DIRECTION_COLUMN:
		પાછા false;
	}
}

સ્થિર અંતઃરેખ પૂર્ણાંક lh__non_auto_margin(અચલ સંરચના box *b, ગણના box_side side)
{
	પાછા (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

સ્થિર અંતઃરેખ પૂર્ણાંક lh__delta_outer_height(અચલ સંરચના box *b)
{
	પાછા b->padding[ટોચ_2] +
	       b->padding[BOTTOM] +
	       b->border[ટોચ_2].width +
	       b->border[BOTTOM].width +
	       lh__non_auto_margin(b, ટોચ_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

સ્થિર અંતઃરેખ પૂર્ણાંક lh__delta_outer_width(અચલ સંરચના box *b)
{
	પાછા b->padding[ડાબું_2] +
	       b->padding[જમણું_2] +
	       b->border[ડાબું_2].width +
	       b->border[જમણું_2].width +
	       lh__non_auto_margin(b, ડાબું_2) +
	       lh__non_auto_margin(b, જમણું_2);
}

સ્થિર અંતઃરેખ પૂર્ણાંક lh__delta_outer_main(
		અચલ સંરચના box *flex,
		અચલ સંરચના box *b)
{
	જો (lh__flex_main_is_horizontal(flex)) {
		પાછા lh__delta_outer_width(b);
	} નહીંતર {
		પાછા lh__delta_outer_height(b);
	}
}

સ્થિર અંતઃરેખ પૂર્ણાંક lh__delta_outer_cross(
		અચલ સંરચના box *flex,
		અચલ સંરચના box *b)
{
	જો (lh__flex_main_is_horizontal(flex) == false) {
		પાછા lh__delta_outer_width(b);
	} નહીંતર {
		પાછા lh__delta_outer_height(b);
	}
}

સ્થિર અંતઃરેખ પૂર્ણાંક *lh__box_size_main_ptr(
		bool horizontal,
		સંરચના box *b)
{
	પાછા horizontal ? &b->width : &b->height;
}

સ્થિર અંતઃરેખ પૂર્ણાંક *lh__box_size_cross_ptr(
		bool horizontal,
		સંરચના box *b)
{
	પાછા horizontal ? &b->height : &b->width;
}

સ્થિર અંતઃરેખ પૂર્ણાંક lh__box_size_main(
		bool horizontal,
		અચલ સંરચના box *b)
{
	પાછા horizontal ? b->width : b->height;
}

સ્થિર અંતઃરેખ પૂર્ણાંક lh__box_size_cross(
		bool horizontal,
		અચલ સંરચના box *b)
{
	પાછા horizontal ? b->height : b->width;
}

સ્થિર અંતઃરેખ bool lh__box_size_cross_is_auto(
		bool horizontal,
		સંરચના box *b)
{
	css_fixed length;
	css_unit unit;

	જો (horizontal) {
		પાછા css_computed_height(b->style,
				&length, &unit) == CSS_HEIGHT_AUTO;
	} નહીંતર {
		પાછા css_computed_width(b->style,
				&length, &unit) == CSS_WIDTH_AUTO;
	}
}

સ્થિર અંતઃરેખ ગણના css_align_self_e lh__box_align_self(
		અચલ સંરચના box *flex,
		અચલ સંરચના box *item)
{
	ગણના css_align_self_e align_self = css_computed_align_self(item->style);

	જો (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_વસ્તુઓ(flex->style);
	}

	પાછા align_self;
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
સ્થિર અંતઃરેખ શૂન્ય calculate_mbp_width(
		અચલ css_unit_ctx *unit_len_ctx,
		અચલ css_computed_style *style,
		unsigned પૂર્ણાંક side,
		bool margin,
		bool border,
		bool padding,
		પૂર્ણાંક *fixed,
		દશાંશ *frac)
{
	css_fixed કિંમત = 0;
	css_unit unit = CSS_UNIT_PX;

	પ્રતિજ્ઞા(style);

	/* margin */
	જો (margin) {
		ગણના css_margin_e type;

		type = margin_funcs[side](style, &કિંમત, &unit);
		જો (type == CSS_MARGIN_SET) {
			જો (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(કિંમત, F_100));
			} નહીંતર {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						કિંમત, unit));
			}
		}
	}

	/* border */
	જો (border) {
		જો (lh__have_border(side, style)) {
			border_width_funcs[side](style, &કિંમત, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					કિંમત, unit));
		}
	}

	/* padding */
	જો (padding) {
		padding_funcs[side](style, &કિંમત, &unit);
		જો (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(કિંમત, F_100));
		} નહીંતર {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					કિંમત, unit));
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
સ્થિર અંતઃરેખ શૂન્ય layout_handle_box_sizing(
		અચલ css_unit_ctx *unit_len_ctx,
		અચલ સંરચના box *box,
		પૂર્ણાંક available_width,
		bool setwidth,
		પૂર્ણાંક *dimension)
{
	ગણના css_box_sizing_e bs;

	પ્રતિજ્ઞા(box && box->style);

	bs = css_computed_box_sizing(box->style);

	જો (bs == CSS_BOX_SIZING_BORDER_BOX) {
		પૂર્ણાંક orig = *dimension;
		પૂર્ણાંક fixed = 0;
		દશાંશ frac = 0;

		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? ડાબું_2 : ટોચ_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? જમણું_2 : BOTTOM,
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
સ્થિર અંતઃરેખ શૂન્ય layout_find_dimensions(
		અચલ css_unit_ctx *unit_len_ctx,
		પૂર્ણાંક available_width,
		પૂર્ણાંક viewport_height,
		અચલ સંરચના box *box,
		અચલ css_computed_style *style,
		પૂર્ણાંક *width,
		પૂર્ણાંક *height,
		પૂર્ણાંક *max_width,
		પૂર્ણાંક *min_width,
		પૂર્ણાંક *max_height,
		પૂર્ણાંક *min_height,
		પૂર્ણાંક margin[4],
		પૂર્ણાંક padding[4],
		સંરચના box_border border[4])
{
	સંરચના box *containing_block = ખાલી;
	unsigned પૂર્ણાંક i;

	જો (width) {
		જો (css_computed_width_px(style, unit_len_ctx,
				available_width, width) == CSS_WIDTH_SET) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, width);
		} નહીંતર {
			*width = AUTO;
		}
	}

	જો (height) {
		ગણના css_height_e htype;
		css_fixed કિંમત = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_height(style, &કિંમત, &unit);

		જો (htype == CSS_HEIGHT_SET) {
			જો (unit == CSS_UNIT_PCT) {
				ગણના css_height_e cbhtype;

				જો (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->parent) {
					/* Box is absolutely positioned */
					પ્રતિજ્ઞા(box->float_container);
					containing_block = box->float_container;
				} નહીંતર જો (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_ડાબું ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_જમણું)) {
					/* Box is a float */
					પ્રતિજ્ઞા(box->parent &&
						box->parent->parent &&
						box->parent->parent->parent);

					containing_block =
						box->parent->parent->parent;
				} નહીંતર જો (box->parent && box->parent->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_block = box->parent;
				} નહીંતર જો (box->parent && box->parent->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					પ્રતિજ્ઞા(box->parent->parent);
					containing_block = box->parent->parent;
				}

				જો (containing_block) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_height(
							containing_block->style,
							&f, &u);
				}

				જો (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*height = FPCT_OF_INT_TOINT(કિંમત,
						containing_block->height);
				} નહીંતર જો ((!box->parent ||
						!box->parent->parent) &&
						viewport_height >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*height = FPCT_OF_INT_TOINT(કિંમત,
							viewport_height);
				} નહીંતર {
					/* precentage height not permissible
					 * treat height as auto */
					*height = AUTO;
				}
			} નહીંતર {
				*height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						કિંમત, unit));
			}
		} નહીંતર {
			*height = AUTO;
		}

		જો (*height != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, false, height);
		}
	}

	જો (max_width) {
		ગણના css_max_width_e type;
		css_fixed કિંમત = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_width(style, &કિંમત, &unit);

		જો (type == CSS_MAX_WIDTH_SET) {
			જો (unit == CSS_UNIT_PCT) {
				*max_width = FPCT_OF_INT_TOINT(કિંમત,
						available_width);
			} નહીંતર {
				*max_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						કિંમત, unit));
			}
		} નહીંતર {
			/* Inadmissible */
			*max_width = -1;
		}

		જો (*max_width != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, max_width);
		}
	}

	જો (min_width) {
		ગણના css_min_width_e type;
		css_fixed કિંમત = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_width(style, &કિંમત, &unit);

		જો (type == CSS_MIN_WIDTH_SET) {
			જો (unit == CSS_UNIT_PCT) {
				*min_width = FPCT_OF_INT_TOINT(કિંમત,
						available_width);
			} નહીંતર {
				*min_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						કિંમત, unit));
			}
		} નહીંતર {
			/* Inadmissible */
			*min_width = 0;
		}

		જો (*min_width != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, min_width);
		}
	}

	જો (max_height) {
		ગણના css_max_height_e type;
		css_fixed કિંમત = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_height(style, &કિંમત, &unit);

		જો (type == CSS_MAX_HEIGHT_SET) {
			જો (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_height = -1;
			} નહીંતર {
				*max_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						કિંમત, unit));
			}
		} નહીંતર {
			/* Inadmissible */
			*max_height = -1;
		}
	}

	જો (min_height) {
		ગણના css_min_height_e type;
		css_fixed કિંમત = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_height(style, &કિંમત, &unit);

		જો (type == CSS_MIN_HEIGHT_SET) {
			જો (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_height = 0;
			} નહીંતર {
				*min_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						કિંમત, unit));
			}
		} નહીંતર {
			/* Inadmissible */
			*min_height = 0;
		}
	}

	માટે (i = 0; i != 4; i++) {
		જો (margin) {
			ગણના css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed કિંમત = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &કિંમત, &unit);

			જો (type == CSS_MARGIN_SET) {
				જો (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_OF_INT_TOINT(કિંમત,
							available_width);
				} નહીંતર {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							કિંમત, unit));
				}
			} નહીંતર {
				margin[i] = AUTO;
			}
		}

		જો (padding) {
			css_fixed કિંમત = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &કિંમત, &unit);

			જો (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_OF_INT_TOINT(કિંમત,
						available_width);
			} નહીંતર {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						કિંમત, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		જો (border && box->type != BOX_TABLE_CELL) {
			ગણના css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed કિંમત = 0;
			css_unit unit = CSS_UNIT_PX;

			border_width_funcs[i](style, &કિંમત, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			જો (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].width = 0;
			નહીંતર
				border[i].width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						કિંમત, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			જો (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_TABLE ||
					 box->type == BOX_TABLE_ROW_GROUP ||
					 box->type == BOX_TABLE_ROW))
				border[i].width = 0;
		}
	}
}

#endif
