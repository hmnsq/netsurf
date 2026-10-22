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

#నిర్వచితం_కాకపోతే NETSURF_HTML_LAYOUT_అంతర్గత_H
#నిర్వచించు NETSURF_HTML_LAYOUT_అంతర్గత_H

#నిర్వచించు AUTO INT_MIN

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
bool layout_ఖండం_context(
		నిర్మాణం box *ఖండం,
		పూర్ణాంకం viewport_ఎత్తు,
		html_content *content);

/**
 * Layout a table.
 *
 * \param  table            table to layout
 * \param  available_width  width of containing block
 * \param  content          memory pool for any new boxes
 * \return  true on success, false on memory exhaustion
 */
bool layout_పట్టిక(
		నిర్మాణం box *పట్టిక,
		పూర్ణాంకం అందుబాటు_వెడల్పు,
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
		నిర్మాణం box *flex,
		పూర్ణాంకం అందుబాటు_వెడల్పు,
		html_content *content);

రకనిర్వచనం uint8_t (*css_len_func)(
		స్థిరరాశి css_computed_style *style,
		css_fixed *నిడివి, css_unit *unit);
రకనిర్వచనం uint8_t (*css_border_style_func)(
		స్థిరరాశి css_computed_style *style);
రకనిర్వచనం uint8_t (*css_border_color_func)(
		స్థిరరాశి css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
బాహ్య స్థిరరాశి css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
బాహ్య స్థిరరాశి css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
బాహ్య స్థిరరాశి css_len_func border_వెడల్పు_funcs[4];

/** Array of per-side access functions for computed style border styles. */
బాహ్య స్థిరరాశి css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
బాహ్య స్థిరరాశి css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
స్థిర అంతఃవరుస bool lh__box_is_float_box(స్థిరరాశి నిర్మాణం box *b)
{
	తిరిగి b->type == BOX_FLOAT_ఎడమ ||
	       b->type == BOX_FLOAT_కుడి;
}

/** Layout helper: Check whether box takes part in inline flow. */
స్థిర అంతఃవరుస bool lh__box_is_inline_flow(స్థిరరాశి నిర్మాణం box *b)
{
	తిరిగి b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_ఖండం ||
	       b->type == BOX_పాఠ్యం ||
	       b->type == BOX_INLINE_చివర;
}

/** Layout helper: Check whether box takes part in inline flow. */
స్థిర అంతఃవరుస bool lh__box_is_flex_container(స్థిరరాశి నిర్మాణం box *b)
{
	తిరిగి b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
స్థిర అంతఃవరుస bool lh_box_is_flex_అంశం(స్థిరరాశి నిర్మాణం box *b)
{
	తిరిగి (b->మాతృ != ఖాళీ) && lh__box_is_flex_container(b->మాతృ);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
స్థిర అంతఃవరుస bool lh__box_is_inline_level(స్థిరరాశి నిర్మాణం box *b)
{
	తిరిగి lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
స్థిర అంతఃవరుస bool lh__box_is_inline_content(స్థిరరాశి నిర్మాణం box *b)
{
	తిరిగి lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
స్థిర అంతఃవరుస bool lh__box_is_object(స్థిరరాశి నిర్మాణం box *b)
{
	తిరిగి b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
స్థిర అంతఃవరుస bool lh__box_is_replace(స్థిరరాశి నిర్మాణం box *b)
{
	తిరిగి b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
స్థిర అంతఃవరుస bool lh__have_border(
		గణన box_side side,
		స్థిరరాశి css_computed_style *style)
{
	తిరిగి border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

స్థిర అంతఃవరుస bool lh__box_is_absolute(స్థిరరాశి నిర్మాణం box *b)
{
	తిరిగి css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

స్థిర అంతఃవరుస bool lh__flex_main_is_horizontal(స్థిరరాశి నిర్మాణం box *flex)
{
	స్థిరరాశి css_computed_style *style = flex->style;

	ప్రతిజ్ఞా(style != ఖాళీ);

	ఎంపిక (css_computed_flex_direction(style)) {
	అప్రమేయ:                        /* Fallthrough. */
	స్థితి CSS_FLEX_DIRECTION_అడ్డువరుస:    /* Fallthrough. */
	స్థితి CSS_FLEX_DIRECTION_అడ్డువరుస_తిరగేయి:
		తిరిగి true;
	స్థితి CSS_FLEX_DIRECTION_స్తంభం: /* Fallthrough. */
	స్థితి CSS_FLEX_DIRECTION_స్తంభం_తిరగేయి:
		తిరిగి false;
	}
}

స్థిర అంతఃవరుస bool lh__flex_direction_reversed(స్థిరరాశి నిర్మాణం box *flex)
{
	ఎంపిక (css_computed_flex_direction(flex->style)) {
	అప్రమేయ:                             /* Fallthrough. */
	స్థితి CSS_FLEX_DIRECTION_అడ్డువరుస_తిరగేయి: /* Fallthrough. */
	స్థితి CSS_FLEX_DIRECTION_స్తంభం_తిరగేయి:
		తిరిగి true;
	స్థితి CSS_FLEX_DIRECTION_అడ్డువరుస: /* Fallthrough. */
	స్థితి CSS_FLEX_DIRECTION_స్తంభం:
		తిరిగి false;
	}
}

స్థిర అంతఃవరుస పూర్ణాంకం lh__non_auto_margin(స్థిరరాశి నిర్మాణం box *b, గణన box_side side)
{
	తిరిగి (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

స్థిర అంతఃవరుస పూర్ణాంకం lh_delta_outer_ఎత్తు(స్థిరరాశి నిర్మాణం box *b)
{
	తిరిగి b->padding[శిఖరం_2] +
	       b->padding[BOTTOM] +
	       b->border[శిఖరం_2].వెడల్పు +
	       b->border[BOTTOM].వెడల్పు +
	       lh__non_auto_margin(b, శిఖరం_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

స్థిర అంతఃవరుస పూర్ణాంకం lh_delta_outer_వెడల్పు(స్థిరరాశి నిర్మాణం box *b)
{
	తిరిగి b->padding[ఎడమ_2] +
	       b->padding[కుడి_2] +
	       b->border[ఎడమ_2].వెడల్పు +
	       b->border[కుడి_2].వెడల్పు +
	       lh__non_auto_margin(b, ఎడమ_2) +
	       lh__non_auto_margin(b, కుడి_2);
}

స్థిర అంతఃవరుస పూర్ణాంకం lh__delta_outer_main(
		స్థిరరాశి నిర్మాణం box *flex,
		స్థిరరాశి నిర్మాణం box *b)
{
	ఒకవేళ (lh__flex_main_is_horizontal(flex)) {
		తిరిగి lh_delta_outer_వెడల్పు(b);
	} లేకపోతే {
		తిరిగి lh_delta_outer_ఎత్తు(b);
	}
}

స్థిర అంతఃవరుస పూర్ణాంకం lh__delta_outer_cross(
		స్థిరరాశి నిర్మాణం box *flex,
		స్థిరరాశి నిర్మాణం box *b)
{
	ఒకవేళ (lh__flex_main_is_horizontal(flex) == false) {
		తిరిగి lh_delta_outer_వెడల్పు(b);
	} లేకపోతే {
		తిరిగి lh_delta_outer_ఎత్తు(b);
	}
}

స్థిర అంతఃవరుస పూర్ణాంకం *lh_box_కొలత_main_ptr(
		bool horizontal,
		నిర్మాణం box *b)
{
	తిరిగి horizontal ? &b->వెడల్పు : &b->ఎత్తు;
}

స్థిర అంతఃవరుస పూర్ణాంకం *lh_box_కొలత_cross_ptr(
		bool horizontal,
		నిర్మాణం box *b)
{
	తిరిగి horizontal ? &b->ఎత్తు : &b->వెడల్పు;
}

స్థిర అంతఃవరుస పూర్ణాంకం lh_box_కొలత_main(
		bool horizontal,
		స్థిరరాశి నిర్మాణం box *b)
{
	తిరిగి horizontal ? b->వెడల్పు : b->ఎత్తు;
}

స్థిర అంతఃవరుస పూర్ణాంకం lh_box_కొలత_cross(
		bool horizontal,
		స్థిరరాశి నిర్మాణం box *b)
{
	తిరిగి horizontal ? b->ఎత్తు : b->వెడల్పు;
}

స్థిర అంతఃవరుస bool lh_box_కొలత_cross_is_auto(
		bool horizontal,
		నిర్మాణం box *b)
{
	css_fixed నిడివి;
	css_unit unit;

	ఒకవేళ (horizontal) {
		తిరిగి css_computed_ఎత్తు(b->style,
				&నిడివి, &unit) == CSS_ఎత్తు_AUTO;
	} లేకపోతే {
		తిరిగి css_computed_వెడల్పు(b->style,
				&నిడివి, &unit) == CSS_వెడల్పు_AUTO;
	}
}

స్థిర అంతఃవరుస గణన css_align_self_e lh__box_align_self(
		స్థిరరాశి నిర్మాణం box *flex,
		స్థిరరాశి నిర్మాణం box *అంశం)
{
	గణన css_align_self_e align_self = css_computed_align_self(అంశం->style);

	ఒకవేళ (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_వస్తువులు(flex->style);
	}

	తిరిగి align_self;
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
స్థిర అంతఃవరుస శూన్య calculate_mbp_వెడల్పు(
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		స్థిరరాశి css_computed_style *style,
		గుర్తులేని పూర్ణాంకం side,
		bool margin,
		bool border,
		bool padding,
		పూర్ణాంకం *fixed,
		దశాంశం *frac)
{
	css_fixed విలువ = 0;
	css_unit unit = CSS_UNIT_PX;

	ప్రతిజ్ఞా(style);

	/* margin */
	ఒకవేళ (margin) {
		గణన css_margin_e type;

		type = margin_funcs[side](style, &విలువ, &unit);
		ఒకవేళ (type == CSS_MARGIN_సమితి) {
			ఒకవేళ (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(విలువ, F_100));
			} లేకపోతే {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						విలువ, unit));
			}
		}
	}

	/* border */
	ఒకవేళ (border) {
		ఒకవేళ (lh__have_border(side, style)) {
			border_వెడల్పు_funcs[side](style, &విలువ, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					విలువ, unit));
		}
	}

	/* padding */
	ఒకవేళ (padding) {
		padding_funcs[side](style, &విలువ, &unit);
		ఒకవేళ (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(విలువ, F_100));
		} లేకపోతే {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					విలువ, unit));
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
స్థిర అంతఃవరుస శూన్య layout_handle_box_sizing(
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		స్థిరరాశి నిర్మాణం box *box,
		పూర్ణాంకం అందుబాటు_వెడల్పు,
		bool setwidth,
		పూర్ణాంకం *dimension)
{
	గణన css_box_sizing_e bs;

	ప్రతిజ్ఞా(box && box->style);

	bs = css_computed_box_sizing(box->style);

	ఒకవేళ (bs == CSS_BOX_SIZING_BORDER_BOX) {
		పూర్ణాంకం orig = *dimension;
		పూర్ణాంకం fixed = 0;
		దశాంశం frac = 0;

		calculate_mbp_వెడల్పు(unit_len_ctx, box->style,
				setwidth ? ఎడమ_2 : శిఖరం_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_వెడల్పు(unit_len_ctx, box->style,
				setwidth ? కుడి_2 : BOTTOM,
				false, true, true, &fixed, &frac);
		orig -= frac * అందుబాటు_వెడల్పు + fixed;
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
స్థిర అంతఃవరుస శూన్య layout_కనుగొను_dimensions(
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		పూర్ణాంకం అందుబాటు_వెడల్పు,
		పూర్ణాంకం viewport_ఎత్తు,
		స్థిరరాశి నిర్మాణం box *box,
		స్థిరరాశి css_computed_style *style,
		పూర్ణాంకం *వెడల్పు,
		పూర్ణాంకం *ఎత్తు,
		పూర్ణాంకం *max_వెడల్పు,
		పూర్ణాంకం *min_వెడల్పు,
		పూర్ణాంకం *max_ఎత్తు,
		పూర్ణాంకం *min_ఎత్తు,
		పూర్ణాంకం margin[4],
		పూర్ణాంకం padding[4],
		నిర్మాణం box_border border[4])
{
	నిర్మాణం box *containing_ఖండం = ఖాళీ;
	గుర్తులేని పూర్ణాంకం i;

	ఒకవేళ (వెడల్పు) {
		ఒకవేళ (css_computed_వెడల్పు_px(style, unit_len_ctx,
				అందుబాటు_వెడల్పు, వెడల్పు) == CSS_వెడల్పు_సమితి) {
			layout_handle_box_sizing(unit_len_ctx, box,
					అందుబాటు_వెడల్పు, true, వెడల్పు);
		} లేకపోతే {
			*వెడల్పు = AUTO;
		}
	}

	ఒకవేళ (ఎత్తు) {
		గణన css_ఎత్తు_e htype;
		css_fixed విలువ = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_ఎత్తు(style, &విలువ, &unit);

		ఒకవేళ (htype == CSS_ఎత్తు_సమితి) {
			ఒకవేళ (unit == CSS_UNIT_PCT) {
				గణన css_ఎత్తు_e cbhtype;

				ఒకవేళ (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->మాతృ) {
					/* Box is absolutely positioned */
					ప్రతిజ్ఞా(box->float_container);
					containing_ఖండం = box->float_container;
				} లేకపోతే ఒకవేళ (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_ఎడమ ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_కుడి)) {
					/* Box is a float */
					ప్రతిజ్ఞా(box->మాతృ &&
						box->మాతృ->మాతృ &&
						box->మాతృ->మాతృ->మాతృ);

					containing_ఖండం =
						box->మాతృ->మాతృ->మాతృ;
				} లేకపోతే ఒకవేళ (box->మాతృ && box->మాతృ->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_ఖండం = box->మాతృ;
				} లేకపోతే ఒకవేళ (box->మాతృ && box->మాతృ->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					ప్రతిజ్ఞా(box->మాతృ->మాతృ);
					containing_ఖండం = box->మాతృ->మాతృ;
				}

				ఒకవేళ (containing_ఖండం) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_ఎత్తు(
							containing_ఖండం->style,
							&f, &u);
				}

				ఒకవేళ (containing_ఖండం &&
					containing_ఖండం->ఎత్తు != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_ఎత్తు_సమితి)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*ఎత్తు = FPCT_యొక్క_INT_TOINT(విలువ,
						containing_ఖండం->ఎత్తు);
				} లేకపోతే ఒకవేళ ((!box->మాతృ ||
						!box->మాతృ->మాతృ) &&
						viewport_ఎత్తు >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*ఎత్తు = FPCT_యొక్క_INT_TOINT(విలువ,
							viewport_ఎత్తు);
				} లేకపోతే {
					/* precentage height not permissible
					 * treat height as auto */
					*ఎత్తు = AUTO;
				}
			} లేకపోతే {
				*ఎత్తు = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						విలువ, unit));
			}
		} లేకపోతే {
			*ఎత్తు = AUTO;
		}

		ఒకవేళ (*ఎత్తు != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					అందుబాటు_వెడల్పు, false, ఎత్తు);
		}
	}

	ఒకవేళ (max_వెడల్పు) {
		గణన css_max_వెడల్పు_e type;
		css_fixed విలువ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_వెడల్పు(style, &విలువ, &unit);

		ఒకవేళ (type == CSS_MAX_వెడల్పు_సమితి) {
			ఒకవేళ (unit == CSS_UNIT_PCT) {
				*max_వెడల్పు = FPCT_యొక్క_INT_TOINT(విలువ,
						అందుబాటు_వెడల్పు);
			} లేకపోతే {
				*max_వెడల్పు = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						విలువ, unit));
			}
		} లేకపోతే {
			/* Inadmissible */
			*max_వెడల్పు = -1;
		}

		ఒకవేళ (*max_వెడల్పు != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					అందుబాటు_వెడల్పు, true, max_వెడల్పు);
		}
	}

	ఒకవేళ (min_వెడల్పు) {
		గణన css_min_వెడల్పు_e type;
		css_fixed విలువ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_వెడల్పు(style, &విలువ, &unit);

		ఒకవేళ (type == CSS_MIN_వెడల్పు_సమితి) {
			ఒకవేళ (unit == CSS_UNIT_PCT) {
				*min_వెడల్పు = FPCT_యొక్క_INT_TOINT(విలువ,
						అందుబాటు_వెడల్పు);
			} లేకపోతే {
				*min_వెడల్పు = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						విలువ, unit));
			}
		} లేకపోతే {
			/* Inadmissible */
			*min_వెడల్పు = 0;
		}

		ఒకవేళ (*min_వెడల్పు != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					అందుబాటు_వెడల్పు, true, min_వెడల్పు);
		}
	}

	ఒకవేళ (max_ఎత్తు) {
		గణన css_max_ఎత్తు_e type;
		css_fixed విలువ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_ఎత్తు(style, &విలువ, &unit);

		ఒకవేళ (type == CSS_MAX_ఎత్తు_సమితి) {
			ఒకవేళ (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_ఎత్తు = -1;
			} లేకపోతే {
				*max_ఎత్తు = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						విలువ, unit));
			}
		} లేకపోతే {
			/* Inadmissible */
			*max_ఎత్తు = -1;
		}
	}

	ఒకవేళ (min_ఎత్తు) {
		గణన css_min_ఎత్తు_e type;
		css_fixed విలువ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_ఎత్తు(style, &విలువ, &unit);

		ఒకవేళ (type == CSS_MIN_ఎత్తు_సమితి) {
			ఒకవేళ (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_ఎత్తు = 0;
			} లేకపోతే {
				*min_ఎత్తు = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						విలువ, unit));
			}
		} లేకపోతే {
			/* Inadmissible */
			*min_ఎత్తు = 0;
		}
	}

	కొరకు (i = 0; i != 4; i++) {
		ఒకవేళ (margin) {
			గణన css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed విలువ = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &విలువ, &unit);

			ఒకవేళ (type == CSS_MARGIN_సమితి) {
				ఒకవేళ (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_యొక్క_INT_TOINT(విలువ,
							అందుబాటు_వెడల్పు);
				} లేకపోతే {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							విలువ, unit));
				}
			} లేకపోతే {
				margin[i] = AUTO;
			}
		}

		ఒకవేళ (padding) {
			css_fixed విలువ = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &విలువ, &unit);

			ఒకవేళ (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_యొక్క_INT_TOINT(విలువ,
						అందుబాటు_వెడల్పు);
			} లేకపోతే {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						విలువ, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		ఒకవేళ (border && box->type != BOX_పట్టిక_CELL) {
			గణన css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed విలువ = 0;
			css_unit unit = CSS_UNIT_PX;

			border_వెడల్పు_funcs[i](style, &విలువ, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			ఒకవేళ (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].వెడల్పు = 0;
			లేకపోతే
				border[i].వెడల్పు = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						విలువ, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			ఒకవేళ (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_పట్టిక ||
					 box->type == BOX_పట్టిక_అడ్డువరుస_సమూహం ||
					 box->type == BOX_పట్టిక_అడ్డువరుస))
				border[i].వెడల్పు = 0;
		}
	}
}

#అయితే_ముగింపు
