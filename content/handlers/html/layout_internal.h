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

#اگر_تعریف_نشده NETSURF_HTML_LAYOUT_داخلی_H
#تعریف_کن NETSURF_HTML_LAYOUT_داخلی_H

#تعریف_کن AUTO INT_MIN

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
bool layout_بلوک_context(
		بنية box *بلوک,
		صحيح viewport_ارتفاع,
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
		صحيح موجود_عرض,
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
		صحيح موجود_عرض,
		html_content *content);

تعريفنوع uint8_t (*css_len_func)(
		ثابت css_computed_style *style,
		css_fixed *درازی, css_unit *unit);
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
خارجي ثابت css_len_func border_عرض_funcs[4];

/** Array of per-side access functions for computed style border styles. */
خارجي ثابت css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
خارجي ثابت css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
ساكن مضمن bool lh__box_is_float_box(ثابت بنية box *b)
{
	أرجع b->type == BOX_FLOAT_چپ ||
	       b->type == BOX_FLOAT_راست;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساكن مضمن bool lh__box_is_inline_flow(ثابت بنية box *b)
{
	أرجع b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_بلوک ||
	       b->type == BOX_متن ||
	       b->type == BOX_INLINE_انتها;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساكن مضمن bool lh__box_is_flex_container(ثابت بنية box *b)
{
	أرجع b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
ساكن مضمن bool lh_box_is_flex_قلم(ثابت بنية box *b)
{
	أرجع (b->والد != عدم) && lh__box_is_flex_container(b->والد);
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
	حالة CSS_FLEX_DIRECTION_ردیف:    /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_ردیف_معکوس:
		أرجع true;
	حالة CSS_FLEX_DIRECTION_ستونی: /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_ستونی_معکوس:
		أرجع false;
	}
}

ساكن مضمن bool lh__flex_direction_reversed(ثابت بنية box *flex)
{
	بدل (css_computed_flex_direction(flex->style)) {
	افتراضي:                             /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_ردیف_معکوس: /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_ستونی_معکوس:
		أرجع true;
	حالة CSS_FLEX_DIRECTION_ردیف: /* Fallthrough. */
	حالة CSS_FLEX_DIRECTION_ستونی:
		أرجع false;
	}
}

ساكن مضمن صحيح lh__non_auto_margin(ثابت بنية box *b, تعداد box_side side)
{
	أرجع (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

ساكن مضمن صحيح lh_delta_outer_ارتفاع(ثابت بنية box *b)
{
	أرجع b->padding[بالاترین_2] +
	       b->padding[BOTTOM] +
	       b->border[بالاترین_2].عرض +
	       b->border[BOTTOM].عرض +
	       lh__non_auto_margin(b, بالاترین_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

ساكن مضمن صحيح lh_delta_outer_عرض(ثابت بنية box *b)
{
	أرجع b->padding[چپ_2] +
	       b->padding[راست_2] +
	       b->border[چپ_2].عرض +
	       b->border[راست_2].عرض +
	       lh__non_auto_margin(b, چپ_2) +
	       lh__non_auto_margin(b, راست_2);
}

ساكن مضمن صحيح lh__delta_outer_main(
		ثابت بنية box *flex,
		ثابت بنية box *b)
{
	إذا (lh__flex_main_is_horizontal(flex)) {
		أرجع lh_delta_outer_عرض(b);
	} وإلا {
		أرجع lh_delta_outer_ارتفاع(b);
	}
}

ساكن مضمن صحيح lh__delta_outer_cross(
		ثابت بنية box *flex,
		ثابت بنية box *b)
{
	إذا (lh__flex_main_is_horizontal(flex) == false) {
		أرجع lh_delta_outer_عرض(b);
	} وإلا {
		أرجع lh_delta_outer_ارتفاع(b);
	}
}

ساكن مضمن صحيح *lh_box_اندازه_main_ptr(
		bool horizontal,
		بنية box *b)
{
	أرجع horizontal ? &b->عرض : &b->ارتفاع;
}

ساكن مضمن صحيح *lh_box_اندازه_cross_ptr(
		bool horizontal,
		بنية box *b)
{
	أرجع horizontal ? &b->ارتفاع : &b->عرض;
}

ساكن مضمن صحيح lh_box_اندازه_main(
		bool horizontal,
		ثابت بنية box *b)
{
	أرجع horizontal ? b->عرض : b->ارتفاع;
}

ساكن مضمن صحيح lh_box_اندازه_cross(
		bool horizontal,
		ثابت بنية box *b)
{
	أرجع horizontal ? b->ارتفاع : b->عرض;
}

ساكن مضمن bool lh_box_اندازه_cross_is_auto(
		bool horizontal,
		بنية box *b)
{
	css_fixed درازی;
	css_unit unit;

	إذا (horizontal) {
		أرجع css_computed_ارتفاع(b->style,
				&درازی, &unit) == CSS_ارتفاع_AUTO;
	} وإلا {
		أرجع css_computed_عرض(b->style,
				&درازی, &unit) == CSS_عرض_AUTO;
	}
}

ساكن مضمن تعداد css_align_self_e lh__box_align_self(
		ثابت بنية box *flex,
		ثابت بنية box *قلم)
{
	تعداد css_align_self_e align_self = css_computed_align_self(قلم->style);

	إذا (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_اقلام(flex->style);
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
ساكن مضمن فراغ calculate_mbp_عرض(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style,
		بیعلامت صحيح side,
		bool margin,
		bool border,
		bool padding,
		صحيح *fixed,
		عائم *frac)
{
	css_fixed مقدار = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(style);

	/* margin */
	إذا (margin) {
		تعداد css_margin_e type;

		type = margin_funcs[side](style, &مقدار, &unit);
		إذا (type == CSS_MARGIN_مجموعه) {
			إذا (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(مقدار, F_100));
			} وإلا {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		}
	}

	/* border */
	إذا (border) {
		إذا (lh__have_border(side, style)) {
			border_عرض_funcs[side](style, &مقدار, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					مقدار, unit));
		}
	}

	/* padding */
	إذا (padding) {
		padding_funcs[side](style, &مقدار, &unit);
		إذا (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(مقدار, F_100));
		} وإلا {
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
ساكن مضمن فراغ layout_handle_box_sizing(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت بنية box *box,
		صحيح موجود_عرض,
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

		calculate_mbp_عرض(unit_len_ctx, box->style,
				setwidth ? چپ_2 : بالاترین_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_عرض(unit_len_ctx, box->style,
				setwidth ? راست_2 : BOTTOM,
				false, true, true, &fixed, &frac);
		orig -= frac * موجود_عرض + fixed;
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
ساكن مضمن فراغ layout_پیدا_کن_dimensions(
		ثابت css_unit_ctx *unit_len_ctx,
		صحيح موجود_عرض,
		صحيح viewport_ارتفاع,
		ثابت بنية box *box,
		ثابت css_computed_style *style,
		صحيح *عرض,
		صحيح *ارتفاع,
		صحيح *max_عرض,
		صحيح *min_عرض,
		صحيح *max_ارتفاع,
		صحيح *min_ارتفاع,
		صحيح margin[4],
		صحيح padding[4],
		بنية box_border border[4])
{
	بنية box *containing_بلوک = عدم;
	بیعلامت صحيح i;

	إذا (عرض) {
		إذا (css_computed_عرض_px(style, unit_len_ctx,
				موجود_عرض, عرض) == CSS_عرض_مجموعه) {
			layout_handle_box_sizing(unit_len_ctx, box,
					موجود_عرض, true, عرض);
		} وإلا {
			*عرض = AUTO;
		}
	}

	إذا (ارتفاع) {
		تعداد css_ارتفاع_e htype;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_ارتفاع(style, &مقدار, &unit);

		إذا (htype == CSS_ارتفاع_مجموعه) {
			إذا (unit == CSS_UNIT_PCT) {
				تعداد css_ارتفاع_e cbhtype;

				إذا (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->والد) {
					/* Box is absolutely positioned */
					أكد(box->float_container);
					containing_بلوک = box->float_container;
				} وإلا إذا (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_چپ ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_راست)) {
					/* Box is a float */
					أكد(box->والد &&
						box->والد->والد &&
						box->والد->والد->والد);

					containing_بلوک =
						box->والد->والد->والد;
				} وإلا إذا (box->والد && box->والد->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_بلوک = box->والد;
				} وإلا إذا (box->والد && box->والد->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					أكد(box->والد->والد);
					containing_بلوک = box->والد->والد;
				}

				إذا (containing_بلوک) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_ارتفاع(
							containing_بلوک->style,
							&f, &u);
				}

				إذا (containing_بلوک &&
					containing_بلوک->ارتفاع != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_ارتفاع_مجموعه)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*ارتفاع = FPCT_از_INT_TOINT(مقدار,
						containing_بلوک->ارتفاع);
				} وإلا إذا ((!box->والد ||
						!box->والد->والد) &&
						viewport_ارتفاع >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*ارتفاع = FPCT_از_INT_TOINT(مقدار,
							viewport_ارتفاع);
				} وإلا {
					/* precentage height not permissible
					 * treat height as auto */
					*ارتفاع = AUTO;
				}
			} وإلا {
				*ارتفاع = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		} وإلا {
			*ارتفاع = AUTO;
		}

		إذا (*ارتفاع != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					موجود_عرض, false, ارتفاع);
		}
	}

	إذا (max_عرض) {
		تعداد css_max_عرض_e type;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_عرض(style, &مقدار, &unit);

		إذا (type == CSS_MAX_عرض_مجموعه) {
			إذا (unit == CSS_UNIT_PCT) {
				*max_عرض = FPCT_از_INT_TOINT(مقدار,
						موجود_عرض);
			} وإلا {
				*max_عرض = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		} وإلا {
			/* Inadmissible */
			*max_عرض = -1;
		}

		إذا (*max_عرض != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					موجود_عرض, true, max_عرض);
		}
	}

	إذا (min_عرض) {
		تعداد css_min_عرض_e type;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_عرض(style, &مقدار, &unit);

		إذا (type == CSS_MIN_عرض_مجموعه) {
			إذا (unit == CSS_UNIT_PCT) {
				*min_عرض = FPCT_از_INT_TOINT(مقدار,
						موجود_عرض);
			} وإلا {
				*min_عرض = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		} وإلا {
			/* Inadmissible */
			*min_عرض = 0;
		}

		إذا (*min_عرض != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					موجود_عرض, true, min_عرض);
		}
	}

	إذا (max_ارتفاع) {
		تعداد css_max_ارتفاع_e type;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_ارتفاع(style, &مقدار, &unit);

		إذا (type == CSS_MAX_ارتفاع_مجموعه) {
			إذا (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_ارتفاع = -1;
			} وإلا {
				*max_ارتفاع = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		} وإلا {
			/* Inadmissible */
			*max_ارتفاع = -1;
		}
	}

	إذا (min_ارتفاع) {
		تعداد css_min_ارتفاع_e type;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_ارتفاع(style, &مقدار, &unit);

		إذا (type == CSS_MIN_ارتفاع_مجموعه) {
			إذا (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_ارتفاع = 0;
			} وإلا {
				*min_ارتفاع = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		} وإلا {
			/* Inadmissible */
			*min_ارتفاع = 0;
		}
	}

	لكل (i = 0; i != 4; i++) {
		إذا (margin) {
			تعداد css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed مقدار = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &مقدار, &unit);

			إذا (type == CSS_MARGIN_مجموعه) {
				إذا (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_از_INT_TOINT(مقدار,
							موجود_عرض);
				} وإلا {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							مقدار, unit));
				}
			} وإلا {
				margin[i] = AUTO;
			}
		}

		إذا (padding) {
			css_fixed مقدار = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &مقدار, &unit);

			إذا (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_از_INT_TOINT(مقدار,
						موجود_عرض);
			} وإلا {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		إذا (border && box->type != BOX_جدول_CELL) {
			تعداد css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed مقدار = 0;
			css_unit unit = CSS_UNIT_PX;

			border_عرض_funcs[i](style, &مقدار, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			إذا (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].عرض = 0;
			وإلا
				border[i].عرض = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						مقدار, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			إذا (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_جدول ||
					 box->type == BOX_جدول_ردیف_گروه ||
					 box->type == BOX_جدول_ردیف))
				border[i].عرض = 0;
		}
	}
}

#پایان_اگر
