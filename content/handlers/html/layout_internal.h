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

#که_نه_تعریف_شوی NETSURF_HTML_LAYOUT_دننی_H
#تعریف_کړه NETSURF_HTML_LAYOUT_دننی_H

#تعریف_کړه AUTO INT_MIN

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
		صحيح viewport_لوړوالی,
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
		صحيح شته_پلنوالی,
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
		صحيح شته_پلنوالی,
		html_content *content);

تعريفنوع uint8_t (*css_len_func)(
		ثابت css_computed_style *style,
		css_fixed *اوږدتیا, css_unit *unit);
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
خارجي ثابت css_len_func border_پلنوالی_funcs[4];

/** Array of per-side access functions for computed style border styles. */
خارجي ثابت css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
خارجي ثابت css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
ساكن مضمن bool lh__box_is_float_box(ثابت بنية box *b)
{
	أرجع b->type == BOX_FLOAT_کیڼ ||
	       b->type == BOX_FLOAT_ښی;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساكن مضمن bool lh__box_is_inline_flow(ثابت بنية box *b)
{
	أرجع b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_بلاک ||
	       b->type == BOX_متن ||
	       b->type == BOX_INLINE_پای_ټکی;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساكن مضمن bool lh__box_is_flex_container(ثابت بنية box *b)
{
	أرجع b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساكن مضمن bool lh_box_is_flex_توکی(ثابت بنية box *b)
{
	أرجع (b->پلار != عدم) && lh__box_is_flex_container(b->پلار);
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
	حالة CSS_FLEX_DIRECTION_لیکه:    /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_لیکه_سرچپه:
		أرجع true;
	حالة CSS_FLEX_DIRECTION_ستنه: /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_ستنه_سرچپه:
		أرجع false;
	}
}

ساكن مضمن bool lh__flex_direction_reversed(ثابت بنية box *flex)
{
	بدل (css_computed_flex_direction(flex->style)) {
	افتراضي:                             /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_لیکه_سرچپه: /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_ستنه_سرچپه:
		أرجع true;
	حالة CSS_FLEX_DIRECTION_لیکه: /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_ستنه:
		أرجع false;
	}
}

ساكن مضمن صحيح lh__non_auto_margin(ثابت بنية box *b, تعداد box_side side)
{
	أرجع (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

ساكن مضمن صحيح lh_delta_outer_لوړوالی(ثابت بنية box *b)
{
	أرجع b->padding[سر_2] +
	       b->padding[BOTTOM] +
	       b->border[سر_2].پلنوالی +
	       b->border[BOTTOM].پلنوالی +
	       lh__non_auto_margin(b, سر_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

ساكن مضمن صحيح lh_delta_outer_پلنوالی(ثابت بنية box *b)
{
	أرجع b->padding[کیڼ_2] +
	       b->padding[ښی_2] +
	       b->border[کیڼ_2].پلنوالی +
	       b->border[ښی_2].پلنوالی +
	       lh__non_auto_margin(b, کیڼ_2) +
	       lh__non_auto_margin(b, ښی_2);
}

ساكن مضمن صحيح lh__delta_outer_main(
		ثابت بنية box *flex,
		ثابت بنية box *b)
{
	إذا (lh__flex_main_is_horizontal(flex)) {
		أرجع lh_delta_outer_پلنوالی(b);
	} وإلا {
		أرجع lh_delta_outer_لوړوالی(b);
	}
}

ساكن مضمن صحيح lh__delta_outer_cross(
		ثابت بنية box *flex,
		ثابت بنية box *b)
{
	إذا (lh__flex_main_is_horizontal(flex) == false) {
		أرجع lh_delta_outer_پلنوالی(b);
	} وإلا {
		أرجع lh_delta_outer_لوړوالی(b);
	}
}

ساكن مضمن صحيح *lh_box_اندازه_main_ptr(
		bool horizontal,
		بنية box *b)
{
	أرجع horizontal ? &b->پلنوالی : &b->لوړوالی;
}

ساكن مضمن صحيح *lh_box_اندازه_cross_ptr(
		bool horizontal,
		بنية box *b)
{
	أرجع horizontal ? &b->لوړوالی : &b->پلنوالی;
}

ساكن مضمن صحيح lh_box_اندازه_main(
		bool horizontal,
		ثابت بنية box *b)
{
	أرجع horizontal ? b->پلنوالی : b->لوړوالی;
}

ساكن مضمن صحيح lh_box_اندازه_cross(
		bool horizontal,
		ثابت بنية box *b)
{
	أرجع horizontal ? b->لوړوالی : b->پلنوالی;
}

ساكن مضمن bool lh_box_اندازه_cross_is_auto(
		bool horizontal,
		بنية box *b)
{
	css_fixed اوږدتیا;
	css_unit unit;

	إذا (horizontal) {
		أرجع css_computed_لوړوالی(b->style,
				&اوږدتیا, &unit) == CSS_لوړوالی_AUTO;
	} وإلا {
		أرجع css_computed_پلنوالی(b->style,
				&اوږدتیا, &unit) == CSS_پلنوالی_AUTO;
	}
}

ساكن مضمن تعداد css_align_self_e lh__box_align_self(
		ثابت بنية box *flex,
		ثابت بنية box *توکی)
{
	تعداد css_align_self_e align_self = css_computed_align_self(توکی->style);

	إذا (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_توکي(flex->style);
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
ساكن مضمن فراغ calculate_mbp_پلنوالی(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style,
		بینښه صحيح side,
		bool margin,
		bool border,
		bool padding,
		صحيح *fixed,
		عائم *frac)
{
	css_fixed ارزښت = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(style);

	/* margin */
	إذا (margin) {
		تعداد css_margin_e type;

		type = margin_funcs[side](style, &ارزښت, &unit);
		إذا (type == CSS_MARGIN_سېټ) {
			إذا (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(ارزښت, F_100));
			} وإلا {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ارزښت, unit));
			}
		}
	}

	/* border */
	إذا (border) {
		إذا (lh__have_border(side, style)) {
			border_پلنوالی_funcs[side](style, &ارزښت, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					ارزښت, unit));
		}
	}

	/* padding */
	إذا (padding) {
		padding_funcs[side](style, &ارزښت, &unit);
		إذا (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(ارزښت, F_100));
		} وإلا {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					ارزښت, unit));
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
		صحيح شته_پلنوالی,
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

		calculate_mbp_پلنوالی(unit_len_ctx, box->style,
				setwidth ? کیڼ_2 : سر_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_پلنوالی(unit_len_ctx, box->style,
				setwidth ? ښی_2 : BOTTOM,
				false, true, true, &fixed, &frac);
		orig -= frac * شته_پلنوالی + fixed;
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
ساكن مضمن فراغ layout_پیدا_کړه_dimensions(
		ثابت css_unit_ctx *unit_len_ctx,
		صحيح شته_پلنوالی,
		صحيح viewport_لوړوالی,
		ثابت بنية box *box,
		ثابت css_computed_style *style,
		صحيح *پلنوالی,
		صحيح *لوړوالی,
		صحيح *max_پلنوالی,
		صحيح *min_پلنوالی,
		صحيح *max_لوړوالی,
		صحيح *min_لوړوالی,
		صحيح margin[4],
		صحيح padding[4],
		بنية box_border border[4])
{
	بنية box *containing_بلاک = عدم;
	بینښه صحيح i;

	إذا (پلنوالی) {
		إذا (css_computed_پلنوالی_px(style, unit_len_ctx,
				شته_پلنوالی, پلنوالی) == CSS_پلنوالی_سېټ) {
			layout_handle_box_sizing(unit_len_ctx, box,
					شته_پلنوالی, true, پلنوالی);
		} وإلا {
			*پلنوالی = AUTO;
		}
	}

	إذا (لوړوالی) {
		تعداد css_لوړوالی_e htype;
		css_fixed ارزښت = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_لوړوالی(style, &ارزښت, &unit);

		إذا (htype == CSS_لوړوالی_سېټ) {
			إذا (unit == CSS_UNIT_PCT) {
				تعداد css_لوړوالی_e cbhtype;

				إذا (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->پلار) {
					/* Box is absolutely positioned */
					أكد(box->float_container);
					containing_بلاک = box->float_container;
				} وإلا إذا (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_کیڼ ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_ښی)) {
					/* Box is a float */
					أكد(box->پلار &&
						box->پلار->پلار &&
						box->پلار->پلار->پلار);

					containing_بلاک =
						box->پلار->پلار->پلار;
				} وإلا إذا (box->پلار && box->پلار->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_بلاک = box->پلار;
				} وإلا إذا (box->پلار && box->پلار->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					أكد(box->پلار->پلار);
					containing_بلاک = box->پلار->پلار;
				}

				إذا (containing_بلاک) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_لوړوالی(
							containing_بلاک->style,
							&f, &u);
				}

				إذا (containing_بلاک &&
					containing_بلاک->لوړوالی != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_لوړوالی_سېټ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*لوړوالی = FPCT_د_INT_TOINT(ارزښت,
						containing_بلاک->لوړوالی);
				} وإلا إذا ((!box->پلار ||
						!box->پلار->پلار) &&
						viewport_لوړوالی >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*لوړوالی = FPCT_د_INT_TOINT(ارزښت,
							viewport_لوړوالی);
				} وإلا {
					/* precentage height not permissible
					 * treat height as auto */
					*لوړوالی = AUTO;
				}
			} وإلا {
				*لوړوالی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ارزښت, unit));
			}
		} وإلا {
			*لوړوالی = AUTO;
		}

		إذا (*لوړوالی != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					شته_پلنوالی, false, لوړوالی);
		}
	}

	إذا (max_پلنوالی) {
		تعداد css_max_پلنوالی_e type;
		css_fixed ارزښت = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_پلنوالی(style, &ارزښت, &unit);

		إذا (type == CSS_MAX_پلنوالی_سېټ) {
			إذا (unit == CSS_UNIT_PCT) {
				*max_پلنوالی = FPCT_د_INT_TOINT(ارزښت,
						شته_پلنوالی);
			} وإلا {
				*max_پلنوالی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ارزښت, unit));
			}
		} وإلا {
			/* Inadmissible */
			*max_پلنوالی = -1;
		}

		إذا (*max_پلنوالی != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					شته_پلنوالی, true, max_پلنوالی);
		}
	}

	إذا (min_پلنوالی) {
		تعداد css_min_پلنوالی_e type;
		css_fixed ارزښت = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_پلنوالی(style, &ارزښت, &unit);

		إذا (type == CSS_MIN_پلنوالی_سېټ) {
			إذا (unit == CSS_UNIT_PCT) {
				*min_پلنوالی = FPCT_د_INT_TOINT(ارزښت,
						شته_پلنوالی);
			} وإلا {
				*min_پلنوالی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ارزښت, unit));
			}
		} وإلا {
			/* Inadmissible */
			*min_پلنوالی = 0;
		}

		إذا (*min_پلنوالی != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					شته_پلنوالی, true, min_پلنوالی);
		}
	}

	إذا (max_لوړوالی) {
		تعداد css_max_لوړوالی_e type;
		css_fixed ارزښت = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_لوړوالی(style, &ارزښت, &unit);

		إذا (type == CSS_MAX_لوړوالی_سېټ) {
			إذا (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_لوړوالی = -1;
			} وإلا {
				*max_لوړوالی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ارزښت, unit));
			}
		} وإلا {
			/* Inadmissible */
			*max_لوړوالی = -1;
		}
	}

	إذا (min_لوړوالی) {
		تعداد css_min_لوړوالی_e type;
		css_fixed ارزښت = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_لوړوالی(style, &ارزښت, &unit);

		إذا (type == CSS_MIN_لوړوالی_سېټ) {
			إذا (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_لوړوالی = 0;
			} وإلا {
				*min_لوړوالی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ارزښت, unit));
			}
		} وإلا {
			/* Inadmissible */
			*min_لوړوالی = 0;
		}
	}

	لكل (i = 0; i != 4; i++) {
		إذا (margin) {
			تعداد css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed ارزښت = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &ارزښت, &unit);

			إذا (type == CSS_MARGIN_سېټ) {
				إذا (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_د_INT_TOINT(ارزښت,
							شته_پلنوالی);
				} وإلا {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							ارزښت, unit));
				}
			} وإلا {
				margin[i] = AUTO;
			}
		}

		إذا (padding) {
			css_fixed ارزښت = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &ارزښت, &unit);

			إذا (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_د_INT_TOINT(ارزښت,
						شته_پلنوالی);
			} وإلا {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ارزښت, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		إذا (border && box->type != BOX_جدول_CELL) {
			تعداد css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed ارزښت = 0;
			css_unit unit = CSS_UNIT_PX;

			border_پلنوالی_funcs[i](style, &ارزښت, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			إذا (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].پلنوالی = 0;
			وإلا
				border[i].پلنوالی = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ارزښت, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			إذا (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_جدول ||
					 box->type == BOX_جدول_لیکه_ډله ||
					 box->type == BOX_جدول_لیکه))
				border[i].پلنوالی = 0;
		}
	}
}

#د_که_پای
