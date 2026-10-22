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

#اگر_غیر_تعریف_شدہ NETSURF_HTML_LAYOUT_اندرونی_H
#تعریف_کرو NETSURF_HTML_LAYOUT_اندرونی_H

#تعریف_کرو AUTO INT_MIN

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
bool layout_بلاک_context(
		ساخت box *بلاک,
		صحیح viewport_اونچائی,
		html_content *content);

/**
 * Layout a table.
 *
 * \param  table            table to layout
 * \param  available_width  width of containing block
 * \param  content          memory pool for any new boxes
 * \return  true on success, false on memory exhaustion
 */
bool layout_جدول(
		ساخت box *جدول,
		صحیح دستیاب_چوڑائی,
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
		ساخت box *flex,
		صحیح دستیاب_چوڑائی,
		html_content *content);

قسمتعریف uint8_t (*css_len_func)(
		ثابت css_computed_style *style,
		css_fixed *لمبائی, css_unit *unit);
قسمتعریف uint8_t (*css_border_style_func)(
		ثابت css_computed_style *style);
قسمتعریف uint8_t (*css_border_color_func)(
		ثابت css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
خارجی ثابت css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
خارجی ثابت css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
خارجی ثابت css_len_func border_چوڑائی_funcs[4];

/** Array of per-side access functions for computed style border styles. */
خارجی ثابت css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
خارجی ثابت css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
ساکن اندرونی bool lh__box_is_float_box(ثابت ساخت box *b)
{
	واپس b->type == BOX_FLOAT_بایاں ||
	       b->type == BOX_FLOAT_دایاں;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساکن اندرونی bool lh__box_is_inline_flow(ثابت ساخت box *b)
{
	واپس b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_بلاک ||
	       b->type == BOX_متن ||
	       b->type == BOX_INLINE_انتہا;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساکن اندرونی bool lh__box_is_flex_container(ثابت ساخت box *b)
{
	واپس b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساکن اندرونی bool lh_box_is_flex_شے(ثابت ساخت box *b)
{
	واپس (b->والد != تہی) && lh__box_is_flex_container(b->والد);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
ساکن اندرونی bool lh__box_is_inline_level(ثابت ساخت box *b)
{
	واپس lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
ساکن اندرونی bool lh__box_is_inline_content(ثابت ساخت box *b)
{
	واپس lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
ساکن اندرونی bool lh__box_is_object(ثابت ساخت box *b)
{
	واپس b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
ساکن اندرونی bool lh__box_is_replace(ثابت ساخت box *b)
{
	واپس b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
ساکن اندرونی bool lh__have_border(
		شمار box_side side,
		ثابت css_computed_style *style)
{
	واپس border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

ساکن اندرونی bool lh__box_is_absolute(ثابت ساخت box *b)
{
	واپس css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

ساکن اندرونی bool lh__flex_main_is_horizontal(ثابت ساخت box *flex)
{
	ثابت css_computed_style *style = flex->style;

	أكد(style != تہی);

	انتخاب (css_computed_flex_direction(style)) {
	طےشدہ:                        /* Fallthrough. */
	حالت CSS_FLEX_DIRECTION_صف:    /* Fallthrough. */
	حالت CSS_FLEX_DIRECTION_صف_الٹ:
		واپس true;
	حالت CSS_FLEX_DIRECTION_عمود: /* Fallthrough. */
	حالت CSS_FLEX_DIRECTION_عمود_الٹ:
		واپس false;
	}
}

ساکن اندرونی bool lh__flex_direction_reversed(ثابت ساخت box *flex)
{
	انتخاب (css_computed_flex_direction(flex->style)) {
	طےشدہ:                             /* Fallthrough. */
	حالت CSS_FLEX_DIRECTION_صف_الٹ: /* Fallthrough. */
	حالت CSS_FLEX_DIRECTION_عمود_الٹ:
		واپس true;
	حالت CSS_FLEX_DIRECTION_صف: /* Fallthrough. */
	حالت CSS_FLEX_DIRECTION_عمود:
		واپس false;
	}
}

ساکن اندرونی صحیح lh__non_auto_margin(ثابت ساخت box *b, شمار box_side side)
{
	واپس (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

ساکن اندرونی صحیح lh_delta_outer_اونچائی(ثابت ساخت box *b)
{
	واپس b->padding[چوٹی_2] +
	       b->padding[BOTTOM] +
	       b->border[چوٹی_2].چوڑائی +
	       b->border[BOTTOM].چوڑائی +
	       lh__non_auto_margin(b, چوٹی_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

ساکن اندرونی صحیح lh_delta_outer_چوڑائی(ثابت ساخت box *b)
{
	واپس b->padding[بایاں_2] +
	       b->padding[دایاں_2] +
	       b->border[بایاں_2].چوڑائی +
	       b->border[دایاں_2].چوڑائی +
	       lh__non_auto_margin(b, بایاں_2) +
	       lh__non_auto_margin(b, دایاں_2);
}

ساکن اندرونی صحیح lh__delta_outer_main(
		ثابت ساخت box *flex,
		ثابت ساخت box *b)
{
	اگر (lh__flex_main_is_horizontal(flex)) {
		واپس lh_delta_outer_چوڑائی(b);
	} ورنہ {
		واپس lh_delta_outer_اونچائی(b);
	}
}

ساکن اندرونی صحیح lh__delta_outer_cross(
		ثابت ساخت box *flex,
		ثابت ساخت box *b)
{
	اگر (lh__flex_main_is_horizontal(flex) == false) {
		واپس lh_delta_outer_چوڑائی(b);
	} ورنہ {
		واپس lh_delta_outer_اونچائی(b);
	}
}

ساکن اندرونی صحیح *lh_box_سائز_main_ptr(
		bool horizontal,
		ساخت box *b)
{
	واپس horizontal ? &b->چوڑائی : &b->اونچائی;
}

ساکن اندرونی صحیح *lh_box_سائز_cross_ptr(
		bool horizontal,
		ساخت box *b)
{
	واپس horizontal ? &b->اونچائی : &b->چوڑائی;
}

ساکن اندرونی صحیح lh_box_سائز_main(
		bool horizontal,
		ثابت ساخت box *b)
{
	واپس horizontal ? b->چوڑائی : b->اونچائی;
}

ساکن اندرونی صحیح lh_box_سائز_cross(
		bool horizontal,
		ثابت ساخت box *b)
{
	واپس horizontal ? b->اونچائی : b->چوڑائی;
}

ساکن اندرونی bool lh_box_سائز_cross_is_auto(
		bool horizontal,
		ساخت box *b)
{
	css_fixed لمبائی;
	css_unit unit;

	اگر (horizontal) {
		واپس css_computed_اونچائی(b->style,
				&لمبائی, &unit) == CSS_اونچائی_AUTO;
	} ورنہ {
		واپس css_computed_چوڑائی(b->style,
				&لمبائی, &unit) == CSS_چوڑائی_AUTO;
	}
}

ساکن اندرونی شمار css_align_self_e lh__box_align_self(
		ثابت ساخت box *flex,
		ثابت ساخت box *شے)
{
	شمار css_align_self_e align_self = css_computed_align_self(شے->style);

	اگر (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_اشیاء(flex->style);
	}

	واپس align_self;
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
ساکن اندرونی خالی calculate_mbp_چوڑائی(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style,
		غیرعلامتی صحیح side,
		bool margin,
		bool border,
		bool padding,
		صحیح *fixed,
		اعشاری *frac)
{
	css_fixed قدر = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(style);

	/* margin */
	اگر (margin) {
		شمار css_margin_e type;

		type = margin_funcs[side](style, &قدر, &unit);
		اگر (type == CSS_MARGIN_سیٹ) {
			اگر (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(قدر, F_100));
			} ورنہ {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		}
	}

	/* border */
	اگر (border) {
		اگر (lh__have_border(side, style)) {
			border_چوڑائی_funcs[side](style, &قدر, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					قدر, unit));
		}
	}

	/* padding */
	اگر (padding) {
		padding_funcs[side](style, &قدر, &unit);
		اگر (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(قدر, F_100));
		} ورنہ {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					قدر, unit));
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
ساکن اندرونی خالی layout_handle_box_sizing(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت ساخت box *box,
		صحیح دستیاب_چوڑائی,
		bool setwidth,
		صحیح *dimension)
{
	شمار css_box_sizing_e bs;

	أكد(box && box->style);

	bs = css_computed_box_sizing(box->style);

	اگر (bs == CSS_BOX_SIZING_BORDER_BOX) {
		صحیح orig = *dimension;
		صحیح fixed = 0;
		اعشاری frac = 0;

		calculate_mbp_چوڑائی(unit_len_ctx, box->style,
				setwidth ? بایاں_2 : چوٹی_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_چوڑائی(unit_len_ctx, box->style,
				setwidth ? دایاں_2 : BOTTOM,
				false, true, true, &fixed, &frac);
		orig -= frac * دستیاب_چوڑائی + fixed;
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
ساکن اندرونی خالی layout_ڈھونڈو_dimensions(
		ثابت css_unit_ctx *unit_len_ctx,
		صحیح دستیاب_چوڑائی,
		صحیح viewport_اونچائی,
		ثابت ساخت box *box,
		ثابت css_computed_style *style,
		صحیح *چوڑائی,
		صحیح *اونچائی,
		صحیح *max_چوڑائی,
		صحیح *min_چوڑائی,
		صحیح *max_اونچائی,
		صحیح *min_اونچائی,
		صحیح margin[4],
		صحیح padding[4],
		ساخت box_border border[4])
{
	ساخت box *containing_بلاک = تہی;
	غیرعلامتی صحیح i;

	اگر (چوڑائی) {
		اگر (css_computed_چوڑائی_px(style, unit_len_ctx,
				دستیاب_چوڑائی, چوڑائی) == CSS_چوڑائی_سیٹ) {
			layout_handle_box_sizing(unit_len_ctx, box,
					دستیاب_چوڑائی, true, چوڑائی);
		} ورنہ {
			*چوڑائی = AUTO;
		}
	}

	اگر (اونچائی) {
		شمار css_اونچائی_e htype;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_اونچائی(style, &قدر, &unit);

		اگر (htype == CSS_اونچائی_سیٹ) {
			اگر (unit == CSS_UNIT_PCT) {
				شمار css_اونچائی_e cbhtype;

				اگر (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->والد) {
					/* Box is absolutely positioned */
					أكد(box->float_container);
					containing_بلاک = box->float_container;
				} ورنہ اگر (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_بایاں ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_دایاں)) {
					/* Box is a float */
					أكد(box->والد &&
						box->والد->والد &&
						box->والد->والد->والد);

					containing_بلاک =
						box->والد->والد->والد;
				} ورنہ اگر (box->والد && box->والد->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_بلاک = box->والد;
				} ورنہ اگر (box->والد && box->والد->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					أكد(box->والد->والد);
					containing_بلاک = box->والد->والد;
				}

				اگر (containing_بلاک) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_اونچائی(
							containing_بلاک->style,
							&f, &u);
				}

				اگر (containing_بلاک &&
					containing_بلاک->اونچائی != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_اونچائی_سیٹ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*اونچائی = FPCT_کا_INT_TOINT(قدر,
						containing_بلاک->اونچائی);
				} ورنہ اگر ((!box->والد ||
						!box->والد->والد) &&
						viewport_اونچائی >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*اونچائی = FPCT_کا_INT_TOINT(قدر,
							viewport_اونچائی);
				} ورنہ {
					/* precentage height not permissible
					 * treat height as auto */
					*اونچائی = AUTO;
				}
			} ورنہ {
				*اونچائی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		} ورنہ {
			*اونچائی = AUTO;
		}

		اگر (*اونچائی != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					دستیاب_چوڑائی, false, اونچائی);
		}
	}

	اگر (max_چوڑائی) {
		شمار css_max_چوڑائی_e type;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_چوڑائی(style, &قدر, &unit);

		اگر (type == CSS_MAX_چوڑائی_سیٹ) {
			اگر (unit == CSS_UNIT_PCT) {
				*max_چوڑائی = FPCT_کا_INT_TOINT(قدر,
						دستیاب_چوڑائی);
			} ورنہ {
				*max_چوڑائی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		} ورنہ {
			/* Inadmissible */
			*max_چوڑائی = -1;
		}

		اگر (*max_چوڑائی != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					دستیاب_چوڑائی, true, max_چوڑائی);
		}
	}

	اگر (min_چوڑائی) {
		شمار css_min_چوڑائی_e type;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_چوڑائی(style, &قدر, &unit);

		اگر (type == CSS_MIN_چوڑائی_سیٹ) {
			اگر (unit == CSS_UNIT_PCT) {
				*min_چوڑائی = FPCT_کا_INT_TOINT(قدر,
						دستیاب_چوڑائی);
			} ورنہ {
				*min_چوڑائی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		} ورنہ {
			/* Inadmissible */
			*min_چوڑائی = 0;
		}

		اگر (*min_چوڑائی != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					دستیاب_چوڑائی, true, min_چوڑائی);
		}
	}

	اگر (max_اونچائی) {
		شمار css_max_اونچائی_e type;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_اونچائی(style, &قدر, &unit);

		اگر (type == CSS_MAX_اونچائی_سیٹ) {
			اگر (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_اونچائی = -1;
			} ورنہ {
				*max_اونچائی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		} ورنہ {
			/* Inadmissible */
			*max_اونچائی = -1;
		}
	}

	اگر (min_اونچائی) {
		شمار css_min_اونچائی_e type;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_اونچائی(style, &قدر, &unit);

		اگر (type == CSS_MIN_اونچائی_سیٹ) {
			اگر (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_اونچائی = 0;
			} ورنہ {
				*min_اونچائی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		} ورنہ {
			/* Inadmissible */
			*min_اونچائی = 0;
		}
	}

	برائے (i = 0; i != 4; i++) {
		اگر (margin) {
			شمار css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed قدر = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &قدر, &unit);

			اگر (type == CSS_MARGIN_سیٹ) {
				اگر (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_کا_INT_TOINT(قدر,
							دستیاب_چوڑائی);
				} ورنہ {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							قدر, unit));
				}
			} ورنہ {
				margin[i] = AUTO;
			}
		}

		اگر (padding) {
			css_fixed قدر = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &قدر, &unit);

			اگر (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_کا_INT_TOINT(قدر,
						دستیاب_چوڑائی);
			} ورنہ {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		اگر (border && box->type != BOX_جدول_CELL) {
			شمار css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed قدر = 0;
			css_unit unit = CSS_UNIT_PX;

			border_چوڑائی_funcs[i](style, &قدر, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			اگر (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].چوڑائی = 0;
			ورنہ
				border[i].چوڑائی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			اگر (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_جدول ||
					 box->type == BOX_جدول_صف_گروہ ||
					 box->type == BOX_جدول_صف))
				border[i].چوڑائی = 0;
		}
	}
}

#اگر_ختم
