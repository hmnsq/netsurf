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

#جے_بنا_تعریف NETSURF_HTML_LAYOUT_اندرونی_H
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
		بنية box *بلاک,
		صحيح viewport_اچائی,
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
		بنية box *جدول,
		صحيح دستیاب_چوڑائی,
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
		صحيح دستیاب_چوڑائی,
		html_content *content);

تعريفنوع uint8_t (*css_len_func)(
		ثابت css_computed_style *style,
		css_fixed *لمائی, css_unit *unit);
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
خارجي ثابت css_len_func border_چوڑائی_funcs[4];

/** Array of per-side access functions for computed style border styles. */
خارجي ثابت css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
خارجي ثابت css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
ساكن مضمن bool lh__box_is_float_box(ثابت بنية box *b)
{
	أرجع b->type == BOX_FLOAT_بایاں ||
	       b->type == BOX_FLOAT_دایاں;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساكن مضمن bool lh__box_is_inline_flow(ثابت بنية box *b)
{
	أرجع b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_بلاک ||
	       b->type == BOX_متن ||
	       b->type == BOX_INLINE_سرا;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساكن مضمن bool lh__box_is_flex_container(ثابت بنية box *b)
{
	أرجع b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساكن مضمن bool lh_box_is_flex_شے(ثابت بنية box *b)
{
	أرجع (b->ماپا != عدم) && lh__box_is_flex_container(b->ماپا);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
ساكن مضمن bool lh__box_is_inline_level(ثابت بنية box *b)
{
	أرجع lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
ساكن مضمن bool lh__box_is_inline_content(ثابت بنية box *b)
{
	أرجع lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
ساكن مضمن bool lh__box_is_object(ثابت بنية box *b)
{
	أرجع b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
ساكن مضمن bool lh__box_is_replace(ثابت بنية box *b)
{
	أرجع b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
ساكن مضمن bool lh__have_border(
		تعداد box_side side,
		ثابت css_computed_style *style)
{
	أرجع border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

ساكن مضمن bool lh__box_is_absolute(ثابت بنية box *b)
{
	أرجع css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

ساكن مضمن bool lh__flex_main_is_horizontal(ثابت بنية box *flex)
{
	ثابت css_computed_style *style = flex->style;

	أكد(style != عدم);

	بدل (css_computed_flex_direction(style)) {
	افتراضي:                        /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_صف:    /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_صف_الٹ:
		أرجع true;
	حالة CSS_FLEX_DIRECTION_کالم: /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_کالم_الٹ:
		أرجع false;
	}
}

ساكن مضمن bool lh__flex_direction_reversed(ثابت بنية box *flex)
{
	بدل (css_computed_flex_direction(flex->style)) {
	افتراضي:                             /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_صف_الٹ: /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_کالم_الٹ:
		أرجع true;
	حالة CSS_FLEX_DIRECTION_صف: /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_کالم:
		أرجع false;
	}
}

ساكن مضمن صحيح lh__non_auto_margin(ثابت بنية box *b, تعداد box_side side)
{
	أرجع (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

ساكن مضمن صحيح lh_delta_outer_اچائی(ثابت بنية box *b)
{
	أرجع b->padding[چوٹی_2] +
	       b->padding[BOTTOM] +
	       b->border[چوٹی_2].چوڑائی +
	       b->border[BOTTOM].چوڑائی +
	       lh__non_auto_margin(b, چوٹی_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

ساكن مضمن صحيح lh_delta_outer_چوڑائی(ثابت بنية box *b)
{
	أرجع b->padding[بایاں_2] +
	       b->padding[دایاں_2] +
	       b->border[بایاں_2].چوڑائی +
	       b->border[دایاں_2].چوڑائی +
	       lh__non_auto_margin(b, بایاں_2) +
	       lh__non_auto_margin(b, دایاں_2);
}

ساكن مضمن صحيح lh__delta_outer_main(
		ثابت بنية box *flex,
		ثابت بنية box *b)
{
	إذا (lh__flex_main_is_horizontal(flex)) {
		أرجع lh_delta_outer_چوڑائی(b);
	} وإلا {
		أرجع lh_delta_outer_اچائی(b);
	}
}

ساكن مضمن صحيح lh__delta_outer_cross(
		ثابت بنية box *flex,
		ثابت بنية box *b)
{
	إذا (lh__flex_main_is_horizontal(flex) == false) {
		أرجع lh_delta_outer_چوڑائی(b);
	} وإلا {
		أرجع lh_delta_outer_اچائی(b);
	}
}

ساكن مضمن صحيح *lh_box_ناپ_main_ptr(
		bool horizontal,
		بنية box *b)
{
	أرجع horizontal ? &b->چوڑائی : &b->اچائی;
}

ساكن مضمن صحيح *lh_box_ناپ_cross_ptr(
		bool horizontal,
		بنية box *b)
{
	أرجع horizontal ? &b->اچائی : &b->چوڑائی;
}

ساكن مضمن صحيح lh_box_ناپ_main(
		bool horizontal,
		ثابت بنية box *b)
{
	أرجع horizontal ? b->چوڑائی : b->اچائی;
}

ساكن مضمن صحيح lh_box_ناپ_cross(
		bool horizontal,
		ثابت بنية box *b)
{
	أرجع horizontal ? b->اچائی : b->چوڑائی;
}

ساكن مضمن bool lh_box_ناپ_cross_is_auto(
		bool horizontal,
		بنية box *b)
{
	css_fixed لمائی;
	css_unit unit;

	إذا (horizontal) {
		أرجع css_computed_اچائی(b->style,
				&لمائی, &unit) == CSS_اچائی_AUTO;
	} وإلا {
		أرجع css_computed_چوڑائی(b->style,
				&لمائی, &unit) == CSS_چوڑائی_AUTO;
	}
}

ساكن مضمن تعداد css_align_self_e lh__box_align_self(
		ثابت بنية box *flex,
		ثابت بنية box *شے)
{
	تعداد css_align_self_e align_self = css_computed_align_self(شے->style);

	إذا (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_اشیاء(flex->style);
	}

	أرجع align_self;
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
ساكن مضمن فراغ calculate_mbp_چوڑائی(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style,
		بنانشانی صحيح side,
		bool margin,
		bool border,
		bool padding,
		صحيح *fixed,
		عائم *frac)
{
	css_fixed قدر = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(style);

	/* margin */
	إذا (margin) {
		تعداد css_margin_e type;

		type = margin_funcs[side](style, &قدر, &unit);
		إذا (type == CSS_MARGIN_سیٹ) {
			إذا (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(قدر, F_100));
			} وإلا {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		}
	}

	/* border */
	إذا (border) {
		إذا (lh__have_border(side, style)) {
			border_چوڑائی_funcs[side](style, &قدر, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					قدر, unit));
		}
	}

	/* padding */
	إذا (padding) {
		padding_funcs[side](style, &قدر, &unit);
		إذا (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(قدر, F_100));
		} وإلا {
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
ساكن مضمن فراغ layout_handle_box_sizing(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت بنية box *box,
		صحيح دستیاب_چوڑائی,
		bool setwidth,
		صحيح *dimension)
{
	تعداد css_box_sizing_e bs;

	أكد(box && box->style);

	bs = css_computed_box_sizing(box->style);

	إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
		صحيح orig = *dimension;
		صحيح fixed = 0;
		عائم frac = 0;

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
ساكن مضمن فراغ layout_لبھو_dimensions(
		ثابت css_unit_ctx *unit_len_ctx,
		صحيح دستیاب_چوڑائی,
		صحيح viewport_اچائی,
		ثابت بنية box *box,
		ثابت css_computed_style *style,
		صحيح *چوڑائی,
		صحيح *اچائی,
		صحيح *max_چوڑائی,
		صحيح *min_چوڑائی,
		صحيح *max_اچائی,
		صحيح *min_اچائی,
		صحيح margin[4],
		صحيح padding[4],
		بنية box_border border[4])
{
	بنية box *containing_بلاک = عدم;
	بنانشانی صحيح i;

	إذا (چوڑائی) {
		إذا (css_computed_چوڑائی_px(style, unit_len_ctx,
				دستیاب_چوڑائی, چوڑائی) == CSS_چوڑائی_سیٹ) {
			layout_handle_box_sizing(unit_len_ctx, box,
					دستیاب_چوڑائی, true, چوڑائی);
		} وإلا {
			*چوڑائی = AUTO;
		}
	}

	إذا (اچائی) {
		تعداد css_اچائی_e htype;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_اچائی(style, &قدر, &unit);

		إذا (htype == CSS_اچائی_سیٹ) {
			إذا (unit == CSS_UNIT_PCT) {
				تعداد css_اچائی_e cbhtype;

				إذا (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->ماپا) {
					/* Box is absolutely positioned */
					أكد(box->float_container);
					containing_بلاک = box->float_container;
				} وإلا إذا (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_بایاں ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_دایاں)) {
					/* Box is a float */
					أكد(box->ماپا &&
						box->ماپا->ماپا &&
						box->ماپا->ماپا->ماپا);

					containing_بلاک =
						box->ماپا->ماپا->ماپا;
				} وإلا إذا (box->ماپا && box->ماپا->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_بلاک = box->ماپا;
				} وإلا إذا (box->ماپا && box->ماپا->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					أكد(box->ماپا->ماپا);
					containing_بلاک = box->ماپا->ماپا;
				}

				إذا (containing_بلاک) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_اچائی(
							containing_بلاک->style,
							&f, &u);
				}

				إذا (containing_بلاک &&
					containing_بلاک->اچائی != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_اچائی_سیٹ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*اچائی = FPCT_دا_INT_TOINT(قدر,
						containing_بلاک->اچائی);
				} وإلا إذا ((!box->ماپا ||
						!box->ماپا->ماپا) &&
						viewport_اچائی >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*اچائی = FPCT_دا_INT_TOINT(قدر,
							viewport_اچائی);
				} وإلا {
					/* precentage height not permissible
					 * treat height as auto */
					*اچائی = AUTO;
				}
			} وإلا {
				*اچائی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		} وإلا {
			*اچائی = AUTO;
		}

		إذا (*اچائی != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					دستیاب_چوڑائی, false, اچائی);
		}
	}

	إذا (max_چوڑائی) {
		تعداد css_max_چوڑائی_e type;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_چوڑائی(style, &قدر, &unit);

		إذا (type == CSS_MAX_چوڑائی_سیٹ) {
			إذا (unit == CSS_UNIT_PCT) {
				*max_چوڑائی = FPCT_دا_INT_TOINT(قدر,
						دستیاب_چوڑائی);
			} وإلا {
				*max_چوڑائی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		} وإلا {
			/* Inadmissible */
			*max_چوڑائی = -1;
		}

		إذا (*max_چوڑائی != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					دستیاب_چوڑائی, true, max_چوڑائی);
		}
	}

	إذا (min_چوڑائی) {
		تعداد css_min_چوڑائی_e type;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_چوڑائی(style, &قدر, &unit);

		إذا (type == CSS_MIN_چوڑائی_سیٹ) {
			إذا (unit == CSS_UNIT_PCT) {
				*min_چوڑائی = FPCT_دا_INT_TOINT(قدر,
						دستیاب_چوڑائی);
			} وإلا {
				*min_چوڑائی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		} وإلا {
			/* Inadmissible */
			*min_چوڑائی = 0;
		}

		إذا (*min_چوڑائی != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					دستیاب_چوڑائی, true, min_چوڑائی);
		}
	}

	إذا (max_اچائی) {
		تعداد css_max_اچائی_e type;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_اچائی(style, &قدر, &unit);

		إذا (type == CSS_MAX_اچائی_سیٹ) {
			إذا (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_اچائی = -1;
			} وإلا {
				*max_اچائی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		} وإلا {
			/* Inadmissible */
			*max_اچائی = -1;
		}
	}

	إذا (min_اچائی) {
		تعداد css_min_اچائی_e type;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_اچائی(style, &قدر, &unit);

		إذا (type == CSS_MIN_اچائی_سیٹ) {
			إذا (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_اچائی = 0;
			} وإلا {
				*min_اچائی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		} وإلا {
			/* Inadmissible */
			*min_اچائی = 0;
		}
	}

	لكل (i = 0; i != 4; i++) {
		إذا (margin) {
			تعداد css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed قدر = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &قدر, &unit);

			إذا (type == CSS_MARGIN_سیٹ) {
				إذا (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_دا_INT_TOINT(قدر,
							دستیاب_چوڑائی);
				} وإلا {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							قدر, unit));
				}
			} وإلا {
				margin[i] = AUTO;
			}
		}

		إذا (padding) {
			css_fixed قدر = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &قدر, &unit);

			إذا (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_دا_INT_TOINT(قدر,
						دستیاب_چوڑائی);
			} وإلا {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		إذا (border && box->type != BOX_جدول_CELL) {
			تعداد css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed قدر = 0;
			css_unit unit = CSS_UNIT_PX;

			border_چوڑائی_funcs[i](style, &قدر, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			إذا (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].چوڑائی = 0;
			وإلا
				border[i].چوڑائی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						قدر, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			إذا (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_جدول ||
					 box->type == BOX_جدول_صف_گروہ ||
					 box->type == BOX_جدول_صف))
				border[i].چوڑائی = 0;
		}
	}
}

#جے_انت
