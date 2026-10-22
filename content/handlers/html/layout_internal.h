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
		ഘടന box *block,
		പൂർണ്ണസംഖ്യ viewport_height,
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
		ഘടന box *table,
		പൂർണ്ണസംഖ്യ available_width,
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
		ഘടന box *flex,
		പൂർണ്ണസംഖ്യ available_width,
		html_content *content);

തരനിർവചനം uint8_t (*css_len_func)(
		സ്ഥിരം css_computed_style *style,
		css_fixed *length, css_unit *unit);
തരനിർവചനം uint8_t (*css_border_style_func)(
		സ്ഥിരം css_computed_style *style);
തരനിർവചനം uint8_t (*css_border_color_func)(
		സ്ഥിരം css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
ബാഹ്യ സ്ഥിരം css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
ബാഹ്യ സ്ഥിരം css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
ബാഹ്യ സ്ഥിരം css_len_func border_width_funcs[4];

/** Array of per-side access functions for computed style border styles. */
ബാഹ്യ സ്ഥിരം css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
ബാഹ്യ സ്ഥിരം css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
സ്ഥിതം വരിയിൽ bool lh__box_is_float_box(സ്ഥിരം ഘടന box *b)
{
	തിരികെ b->type == BOX_FLOAT_ഇടത് ||
	       b->type == BOX_FLOAT_വലത്;
}

/** Layout helper: Check whether box takes part in inline flow. */
സ്ഥിതം വരിയിൽ bool lh__box_is_inline_flow(സ്ഥിരം ഘടന box *b)
{
	തിരികെ b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_TEXT ||
	       b->type == BOX_INLINE_END;
}

/** Layout helper: Check whether box takes part in inline flow. */
സ്ഥിതം വരിയിൽ bool lh__box_is_flex_container(സ്ഥിരം ഘടന box *b)
{
	തിരികെ b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
സ്ഥിതം വരിയിൽ bool lh__box_is_flex_item(സ്ഥിരം ഘടന box *b)
{
	തിരികെ (b->parent != ശൂന്യ) && lh__box_is_flex_container(b->parent);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
സ്ഥിതം വരിയിൽ bool lh__box_is_inline_level(സ്ഥിരം ഘടന box *b)
{
	തിരികെ lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
സ്ഥിതം വരിയിൽ bool lh__box_is_inline_content(സ്ഥിരം ഘടന box *b)
{
	തിരികെ lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
സ്ഥിതം വരിയിൽ bool lh__box_is_object(സ്ഥിരം ഘടന box *b)
{
	തിരികെ b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
സ്ഥിതം വരിയിൽ bool lh__box_is_replace(സ്ഥിരം ഘടന box *b)
{
	തിരികെ b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
സ്ഥിതം വരിയിൽ bool lh__have_border(
		എണ്ണൽ box_side side,
		സ്ഥിരം css_computed_style *style)
{
	തിരികെ border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

സ്ഥിതം വരിയിൽ bool lh__box_is_absolute(സ്ഥിരം ഘടന box *b)
{
	തിരികെ css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

സ്ഥിതം വരിയിൽ bool lh__flex_main_is_horizontal(സ്ഥിരം ഘടന box *flex)
{
	സ്ഥിരം css_computed_style *style = flex->style;

	പ്രതിജ്ഞാ(style != ശൂന്യ);

	തിരഞ്ഞെടുപ്പ് (css_computed_flex_direction(style)) {
	സ്ഥിരസ്ഥിതി:                        /* Fallthrough. */
	അവസ്ഥ CSS_FLEX_DIRECTION_ROW:    /* Fallthrough. */
	അവസ്ഥ CSS_FLEX_DIRECTION_ROW_REVERSE:
		തിരികെ true;
	അവസ്ഥ CSS_FLEX_DIRECTION_COLUMN: /* Fallthrough. */
	അവസ്ഥ CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		തിരികെ false;
	}
}

സ്ഥിതം വരിയിൽ bool lh__flex_direction_reversed(സ്ഥിരം ഘടന box *flex)
{
	തിരഞ്ഞെടുപ്പ് (css_computed_flex_direction(flex->style)) {
	സ്ഥിരസ്ഥിതി:                             /* Fallthrough. */
	അവസ്ഥ CSS_FLEX_DIRECTION_ROW_REVERSE: /* Fallthrough. */
	അവസ്ഥ CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		തിരികെ true;
	അവസ്ഥ CSS_FLEX_DIRECTION_ROW: /* Fallthrough. */
	അവസ്ഥ CSS_FLEX_DIRECTION_COLUMN:
		തിരികെ false;
	}
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ lh__non_auto_margin(സ്ഥിരം ഘടന box *b, എണ്ണൽ box_side side)
{
	തിരികെ (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ lh__delta_outer_height(സ്ഥിരം ഘടന box *b)
{
	തിരികെ b->padding[മുകൾ_2] +
	       b->padding[BOTTOM] +
	       b->border[മുകൾ_2].width +
	       b->border[BOTTOM].width +
	       lh__non_auto_margin(b, മുകൾ_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ lh__delta_outer_width(സ്ഥിരം ഘടന box *b)
{
	തിരികെ b->padding[ഇടത്_2] +
	       b->padding[വലത്_2] +
	       b->border[ഇടത്_2].width +
	       b->border[വലത്_2].width +
	       lh__non_auto_margin(b, ഇടത്_2) +
	       lh__non_auto_margin(b, വലത്_2);
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ lh__delta_outer_main(
		സ്ഥിരം ഘടന box *flex,
		സ്ഥിരം ഘടന box *b)
{
	എങ്കിൽ (lh__flex_main_is_horizontal(flex)) {
		തിരികെ lh__delta_outer_width(b);
	} അല്ലെങ്കിൽ {
		തിരികെ lh__delta_outer_height(b);
	}
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ lh__delta_outer_cross(
		സ്ഥിരം ഘടന box *flex,
		സ്ഥിരം ഘടന box *b)
{
	എങ്കിൽ (lh__flex_main_is_horizontal(flex) == false) {
		തിരികെ lh__delta_outer_width(b);
	} അല്ലെങ്കിൽ {
		തിരികെ lh__delta_outer_height(b);
	}
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ *lh__box_size_main_ptr(
		bool horizontal,
		ഘടന box *b)
{
	തിരികെ horizontal ? &b->width : &b->height;
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ *lh__box_size_cross_ptr(
		bool horizontal,
		ഘടന box *b)
{
	തിരികെ horizontal ? &b->height : &b->width;
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ lh__box_size_main(
		bool horizontal,
		സ്ഥിരം ഘടന box *b)
{
	തിരികെ horizontal ? b->width : b->height;
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ lh__box_size_cross(
		bool horizontal,
		സ്ഥിരം ഘടന box *b)
{
	തിരികെ horizontal ? b->height : b->width;
}

സ്ഥിതം വരിയിൽ bool lh__box_size_cross_is_auto(
		bool horizontal,
		ഘടന box *b)
{
	css_fixed length;
	css_unit unit;

	എങ്കിൽ (horizontal) {
		തിരികെ css_computed_height(b->style,
				&length, &unit) == CSS_HEIGHT_AUTO;
	} അല്ലെങ്കിൽ {
		തിരികെ css_computed_width(b->style,
				&length, &unit) == CSS_WIDTH_AUTO;
	}
}

സ്ഥിതം വരിയിൽ എണ്ണൽ css_align_self_e lh__box_align_self(
		സ്ഥിരം ഘടന box *flex,
		സ്ഥിരം ഘടന box *item)
{
	എണ്ണൽ css_align_self_e align_self = css_computed_align_self(item->style);

	എങ്കിൽ (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_ഇനങ്ങൾ(flex->style);
	}

	തിരികെ align_self;
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
സ്ഥിതം വരിയിൽ ശൂന്യം calculate_mbp_width(
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		സ്ഥിരം css_computed_style *style,
		unsigned പൂർണ്ണസംഖ്യ side,
		bool margin,
		bool border,
		bool padding,
		പൂർണ്ണസംഖ്യ *fixed,
		ദശാംശം *frac)
{
	css_fixed മൂല്യം = 0;
	css_unit unit = CSS_UNIT_PX;

	പ്രതിജ്ഞാ(style);

	/* margin */
	എങ്കിൽ (margin) {
		എണ്ണൽ css_margin_e type;

		type = margin_funcs[side](style, &മൂല്യം, &unit);
		എങ്കിൽ (type == CSS_MARGIN_SET) {
			എങ്കിൽ (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(മൂല്യം, F_100));
			} അല്ലെങ്കിൽ {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						മൂല്യം, unit));
			}
		}
	}

	/* border */
	എങ്കിൽ (border) {
		എങ്കിൽ (lh__have_border(side, style)) {
			border_width_funcs[side](style, &മൂല്യം, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					മൂല്യം, unit));
		}
	}

	/* padding */
	എങ്കിൽ (padding) {
		padding_funcs[side](style, &മൂല്യം, &unit);
		എങ്കിൽ (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(മൂല്യം, F_100));
		} അല്ലെങ്കിൽ {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					മൂല്യം, unit));
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
സ്ഥിതം വരിയിൽ ശൂന്യം layout_handle_box_sizing(
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		സ്ഥിരം ഘടന box *box,
		പൂർണ്ണസംഖ്യ available_width,
		bool setwidth,
		പൂർണ്ണസംഖ്യ *dimension)
{
	എണ്ണൽ css_box_sizing_e bs;

	പ്രതിജ്ഞാ(box && box->style);

	bs = css_computed_box_sizing(box->style);

	എങ്കിൽ (bs == CSS_BOX_SIZING_BORDER_BOX) {
		പൂർണ്ണസംഖ്യ orig = *dimension;
		പൂർണ്ണസംഖ്യ fixed = 0;
		ദശാംശം frac = 0;

		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? ഇടത്_2 : മുകൾ_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? വലത്_2 : BOTTOM,
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
സ്ഥിതം വരിയിൽ ശൂന്യം layout_find_dimensions(
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		പൂർണ്ണസംഖ്യ available_width,
		പൂർണ്ണസംഖ്യ viewport_height,
		സ്ഥിരം ഘടന box *box,
		സ്ഥിരം css_computed_style *style,
		പൂർണ്ണസംഖ്യ *width,
		പൂർണ്ണസംഖ്യ *height,
		പൂർണ്ണസംഖ്യ *max_width,
		പൂർണ്ണസംഖ്യ *min_width,
		പൂർണ്ണസംഖ്യ *max_height,
		പൂർണ്ണസംഖ്യ *min_height,
		പൂർണ്ണസംഖ്യ margin[4],
		പൂർണ്ണസംഖ്യ padding[4],
		ഘടന box_border border[4])
{
	ഘടന box *containing_block = ശൂന്യ;
	unsigned പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (width) {
		എങ്കിൽ (css_computed_width_px(style, unit_len_ctx,
				available_width, width) == CSS_WIDTH_SET) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, width);
		} അല്ലെങ്കിൽ {
			*width = AUTO;
		}
	}

	എങ്കിൽ (height) {
		എണ്ണൽ css_height_e htype;
		css_fixed മൂല്യം = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_height(style, &മൂല്യം, &unit);

		എങ്കിൽ (htype == CSS_HEIGHT_SET) {
			എങ്കിൽ (unit == CSS_UNIT_PCT) {
				എണ്ണൽ css_height_e cbhtype;

				എങ്കിൽ (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->parent) {
					/* Box is absolutely positioned */
					പ്രതിജ്ഞാ(box->float_container);
					containing_block = box->float_container;
				} അല്ലെങ്കിൽ എങ്കിൽ (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_ഇടത് ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_വലത്)) {
					/* Box is a float */
					പ്രതിജ്ഞാ(box->parent &&
						box->parent->parent &&
						box->parent->parent->parent);

					containing_block =
						box->parent->parent->parent;
				} അല്ലെങ്കിൽ എങ്കിൽ (box->parent && box->parent->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_block = box->parent;
				} അല്ലെങ്കിൽ എങ്കിൽ (box->parent && box->parent->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					പ്രതിജ്ഞാ(box->parent->parent);
					containing_block = box->parent->parent;
				}

				എങ്കിൽ (containing_block) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_height(
							containing_block->style,
							&f, &u);
				}

				എങ്കിൽ (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*height = FPCT_OF_INT_TOINT(മൂല്യം,
						containing_block->height);
				} അല്ലെങ്കിൽ എങ്കിൽ ((!box->parent ||
						!box->parent->parent) &&
						viewport_height >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*height = FPCT_OF_INT_TOINT(മൂല്യം,
							viewport_height);
				} അല്ലെങ്കിൽ {
					/* precentage height not permissible
					 * treat height as auto */
					*height = AUTO;
				}
			} അല്ലെങ്കിൽ {
				*height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						മൂല്യം, unit));
			}
		} അല്ലെങ്കിൽ {
			*height = AUTO;
		}

		എങ്കിൽ (*height != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, false, height);
		}
	}

	എങ്കിൽ (max_width) {
		എണ്ണൽ css_max_width_e type;
		css_fixed മൂല്യം = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_width(style, &മൂല്യം, &unit);

		എങ്കിൽ (type == CSS_MAX_WIDTH_SET) {
			എങ്കിൽ (unit == CSS_UNIT_PCT) {
				*max_width = FPCT_OF_INT_TOINT(മൂല്യം,
						available_width);
			} അല്ലെങ്കിൽ {
				*max_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						മൂല്യം, unit));
			}
		} അല്ലെങ്കിൽ {
			/* Inadmissible */
			*max_width = -1;
		}

		എങ്കിൽ (*max_width != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, max_width);
		}
	}

	എങ്കിൽ (min_width) {
		എണ്ണൽ css_min_width_e type;
		css_fixed മൂല്യം = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_width(style, &മൂല്യം, &unit);

		എങ്കിൽ (type == CSS_MIN_WIDTH_SET) {
			എങ്കിൽ (unit == CSS_UNIT_PCT) {
				*min_width = FPCT_OF_INT_TOINT(മൂല്യം,
						available_width);
			} അല്ലെങ്കിൽ {
				*min_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						മൂല്യം, unit));
			}
		} അല്ലെങ്കിൽ {
			/* Inadmissible */
			*min_width = 0;
		}

		എങ്കിൽ (*min_width != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, min_width);
		}
	}

	എങ്കിൽ (max_height) {
		എണ്ണൽ css_max_height_e type;
		css_fixed മൂല്യം = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_height(style, &മൂല്യം, &unit);

		എങ്കിൽ (type == CSS_MAX_HEIGHT_SET) {
			എങ്കിൽ (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_height = -1;
			} അല്ലെങ്കിൽ {
				*max_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						മൂല്യം, unit));
			}
		} അല്ലെങ്കിൽ {
			/* Inadmissible */
			*max_height = -1;
		}
	}

	എങ്കിൽ (min_height) {
		എണ്ണൽ css_min_height_e type;
		css_fixed മൂല്യം = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_height(style, &മൂല്യം, &unit);

		എങ്കിൽ (type == CSS_MIN_HEIGHT_SET) {
			എങ്കിൽ (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_height = 0;
			} അല്ലെങ്കിൽ {
				*min_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						മൂല്യം, unit));
			}
		} അല്ലെങ്കിൽ {
			/* Inadmissible */
			*min_height = 0;
		}
	}

	വേണ്ടി (i = 0; i != 4; i++) {
		എങ്കിൽ (margin) {
			എണ്ണൽ css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed മൂല്യം = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &മൂല്യം, &unit);

			എങ്കിൽ (type == CSS_MARGIN_SET) {
				എങ്കിൽ (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_OF_INT_TOINT(മൂല്യം,
							available_width);
				} അല്ലെങ്കിൽ {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							മൂല്യം, unit));
				}
			} അല്ലെങ്കിൽ {
				margin[i] = AUTO;
			}
		}

		എങ്കിൽ (padding) {
			css_fixed മൂല്യം = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &മൂല്യം, &unit);

			എങ്കിൽ (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_OF_INT_TOINT(മൂല്യം,
						available_width);
			} അല്ലെങ്കിൽ {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						മൂല്യം, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		എങ്കിൽ (border && box->type != BOX_TABLE_CELL) {
			എണ്ണൽ css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed മൂല്യം = 0;
			css_unit unit = CSS_UNIT_PX;

			border_width_funcs[i](style, &മൂല്യം, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			എങ്കിൽ (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].width = 0;
			അല്ലെങ്കിൽ
				border[i].width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						മൂല്യം, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			എങ്കിൽ (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_TABLE ||
					 box->type == BOX_TABLE_ROW_GROUP ||
					 box->type == BOX_TABLE_ROW))
				border[i].width = 0;
		}
	}
}

#endif
