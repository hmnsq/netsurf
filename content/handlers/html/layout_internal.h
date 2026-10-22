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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_HTML_LAYOUT_ਅੰਦਰੂਨੀ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_HTML_LAYOUT_ਅੰਦਰੂਨੀ_H

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTO INT_MIN

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
bool layout_ਖੰਡ_context(
		ਬਣਤਰ box *ਖੰਡ,
		ਪੂਰਨ viewport_ਉਚਾਈ,
		html_content *content);

/**
 * Layout a table.
 *
 * \param  table            table to layout
 * \param  available_width  width of containing block
 * \param  content          memory pool for any new boxes
 * \return  true on success, false on memory exhaustion
 */
bool layout_ਸਾਰਣੀ(
		ਬਣਤਰ box *ਸਾਰਣੀ,
		ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ,
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
		ਬਣਤਰ box *flex,
		ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ,
		html_content *content);

ਕਿਸਮਪਰਿਭਾਸ਼ਾ uint8_t (*css_len_func)(
		ਅਚਲ css_computed_style *style,
		css_fixed *ਲੰਮਾਈ, css_unit *unit);
ਕਿਸਮਪਰਿਭਾਸ਼ਾ uint8_t (*css_border_style_func)(
		ਅਚਲ css_computed_style *style);
ਕਿਸਮਪਰਿਭਾਸ਼ਾ uint8_t (*css_border_color_func)(
		ਅਚਲ css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
ਬਾਹਰੀ ਅਚਲ css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
ਬਾਹਰੀ ਅਚਲ css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
ਬਾਹਰੀ ਅਚਲ css_len_func border_ਚੌੜਾਈ_funcs[4];

/** Array of per-side access functions for computed style border styles. */
ਬਾਹਰੀ ਅਚਲ css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
ਬਾਹਰੀ ਅਚਲ css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool lh__box_is_float_box(ਅਚਲ ਬਣਤਰ box *b)
{
	ਵਾਪਸ b->type == BOX_FLOAT_ਖੱਬਾ ||
	       b->type == BOX_FLOAT_ਸੱਜਾ;
}

/** Layout helper: Check whether box takes part in inline flow. */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool lh__box_is_inline_flow(ਅਚਲ ਬਣਤਰ box *b)
{
	ਵਾਪਸ b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_ਖੰਡ ||
	       b->type == BOX_ਪਾਠ ||
	       b->type == BOX_INLINE_ਸਿਰਾ;
}

/** Layout helper: Check whether box takes part in inline flow. */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool lh__box_is_flex_container(ਅਚਲ ਬਣਤਰ box *b)
{
	ਵਾਪਸ b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool lh_box_is_flex_ਵਸਤੂ(ਅਚਲ ਬਣਤਰ box *b)
{
	ਵਾਪਸ (b->ਮਾਪਾ != ਨਿਰਾ) && lh__box_is_flex_container(b->ਮਾਪਾ);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool lh__box_is_inline_level(ਅਚਲ ਬਣਤਰ box *b)
{
	ਵਾਪਸ lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool lh__box_is_inline_content(ਅਚਲ ਬਣਤਰ box *b)
{
	ਵਾਪਸ lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool lh__box_is_object(ਅਚਲ ਬਣਤਰ box *b)
{
	ਵਾਪਸ b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool lh__box_is_replace(ਅਚਲ ਬਣਤਰ box *b)
{
	ਵਾਪਸ b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool lh__have_border(
		ਗਿਣਤੀ box_side side,
		ਅਚਲ css_computed_style *style)
{
	ਵਾਪਸ border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool lh__box_is_absolute(ਅਚਲ ਬਣਤਰ box *b)
{
	ਵਾਪਸ css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool lh__flex_main_is_horizontal(ਅਚਲ ਬਣਤਰ box *flex)
{
	ਅਚਲ css_computed_style *style = flex->style;

	ਪ੍ਰਤਿਜ੍ਞਾ(style != ਨਿਰਾ);

	ਚੋਣ (css_computed_flex_direction(style)) {
	ਮੂਲ:                        /* Fallthrough. */
	ਹਾਲਤ CSS_FLEX_DIRECTION_ਪੰਕਤੀ:    /* Fallthrough. */
	ਹਾਲਤ CSS_FLEX_DIRECTION_ਪੰਕਤੀ_ਉਲਟ:
		ਵਾਪਸ true;
	ਹਾਲਤ CSS_FLEX_DIRECTION_ਕਾਲਮ: /* Fallthrough. */
	ਹਾਲਤ CSS_FLEX_DIRECTION_ਕਾਲਮ_ਉਲਟ:
		ਵਾਪਸ false;
	}
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool lh__flex_direction_reversed(ਅਚਲ ਬਣਤਰ box *flex)
{
	ਚੋਣ (css_computed_flex_direction(flex->style)) {
	ਮੂਲ:                             /* Fallthrough. */
	ਹਾਲਤ CSS_FLEX_DIRECTION_ਪੰਕਤੀ_ਉਲਟ: /* Fallthrough. */
	ਹਾਲਤ CSS_FLEX_DIRECTION_ਕਾਲਮ_ਉਲਟ:
		ਵਾਪਸ true;
	ਹਾਲਤ CSS_FLEX_DIRECTION_ਪੰਕਤੀ: /* Fallthrough. */
	ਹਾਲਤ CSS_FLEX_DIRECTION_ਕਾਲਮ:
		ਵਾਪਸ false;
	}
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ lh__non_auto_margin(ਅਚਲ ਬਣਤਰ box *b, ਗਿਣਤੀ box_side side)
{
	ਵਾਪਸ (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ lh_delta_outer_ਉਚਾਈ(ਅਚਲ ਬਣਤਰ box *b)
{
	ਵਾਪਸ b->padding[ਸਿਖਰ_2] +
	       b->padding[BOTTOM] +
	       b->border[ਸਿਖਰ_2].ਚੌੜਾਈ +
	       b->border[BOTTOM].ਚੌੜਾਈ +
	       lh__non_auto_margin(b, ਸਿਖਰ_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ lh_delta_outer_ਚੌੜਾਈ(ਅਚਲ ਬਣਤਰ box *b)
{
	ਵਾਪਸ b->padding[ਖੱਬਾ_2] +
	       b->padding[ਸੱਜਾ_2] +
	       b->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
	       b->border[ਸੱਜਾ_2].ਚੌੜਾਈ +
	       lh__non_auto_margin(b, ਖੱਬਾ_2) +
	       lh__non_auto_margin(b, ਸੱਜਾ_2);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ lh__delta_outer_main(
		ਅਚਲ ਬਣਤਰ box *flex,
		ਅਚਲ ਬਣਤਰ box *b)
{
	ਜੇ (lh__flex_main_is_horizontal(flex)) {
		ਵਾਪਸ lh_delta_outer_ਚੌੜਾਈ(b);
	} ਨਹੀਂਤਾਂ {
		ਵਾਪਸ lh_delta_outer_ਉਚਾਈ(b);
	}
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ lh__delta_outer_cross(
		ਅਚਲ ਬਣਤਰ box *flex,
		ਅਚਲ ਬਣਤਰ box *b)
{
	ਜੇ (lh__flex_main_is_horizontal(flex) == false) {
		ਵਾਪਸ lh_delta_outer_ਚੌੜਾਈ(b);
	} ਨਹੀਂਤਾਂ {
		ਵਾਪਸ lh_delta_outer_ਉਚਾਈ(b);
	}
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ *lh_box_ਮਾਪ_main_ptr(
		bool horizontal,
		ਬਣਤਰ box *b)
{
	ਵਾਪਸ horizontal ? &b->ਚੌੜਾਈ : &b->ਉਚਾਈ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ *lh_box_ਮਾਪ_cross_ptr(
		bool horizontal,
		ਬਣਤਰ box *b)
{
	ਵਾਪਸ horizontal ? &b->ਉਚਾਈ : &b->ਚੌੜਾਈ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ lh_box_ਮਾਪ_main(
		bool horizontal,
		ਅਚਲ ਬਣਤਰ box *b)
{
	ਵਾਪਸ horizontal ? b->ਚੌੜਾਈ : b->ਉਚਾਈ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ lh_box_ਮਾਪ_cross(
		bool horizontal,
		ਅਚਲ ਬਣਤਰ box *b)
{
	ਵਾਪਸ horizontal ? b->ਉਚਾਈ : b->ਚੌੜਾਈ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool lh_box_ਮਾਪ_cross_is_auto(
		bool horizontal,
		ਬਣਤਰ box *b)
{
	css_fixed ਲੰਮਾਈ;
	css_unit unit;

	ਜੇ (horizontal) {
		ਵਾਪਸ css_computed_ਉਚਾਈ(b->style,
				&ਲੰਮਾਈ, &unit) == CSS_ਉਚਾਈ_AUTO;
	} ਨਹੀਂਤਾਂ {
		ਵਾਪਸ css_computed_ਚੌੜਾਈ(b->style,
				&ਲੰਮਾਈ, &unit) == CSS_ਚੌੜਾਈ_AUTO;
	}
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਗਿਣਤੀ css_align_self_e lh__box_align_self(
		ਅਚਲ ਬਣਤਰ box *flex,
		ਅਚਲ ਬਣਤਰ box *ਵਸਤੂ)
{
	ਗਿਣਤੀ css_align_self_e align_self = css_computed_align_self(ਵਸਤੂ->style);

	ਜੇ (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_ਚੀਜ਼ਾਂ(flex->style);
	}

	ਵਾਪਸ align_self;
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
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ calculate_mbp_ਚੌੜਾਈ(
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਅਚਲ css_computed_style *style,
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ side,
		bool margin,
		bool border,
		bool padding,
		ਪੂਰਨ *fixed,
		ਦਸ਼ਮਲਵ *frac)
{
	css_fixed ਮੁੱਲ = 0;
	css_unit unit = CSS_UNIT_PX;

	ਪ੍ਰਤਿਜ੍ਞਾ(style);

	/* margin */
	ਜੇ (margin) {
		ਗਿਣਤੀ css_margin_e type;

		type = margin_funcs[side](style, &ਮੁੱਲ, &unit);
		ਜੇ (type == CSS_MARGIN_ਸੈੱਟ) {
			ਜੇ (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(ਮੁੱਲ, F_100));
			} ਨਹੀਂਤਾਂ {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ਮੁੱਲ, unit));
			}
		}
	}

	/* border */
	ਜੇ (border) {
		ਜੇ (lh__have_border(side, style)) {
			border_ਚੌੜਾਈ_funcs[side](style, &ਮੁੱਲ, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					ਮੁੱਲ, unit));
		}
	}

	/* padding */
	ਜੇ (padding) {
		padding_funcs[side](style, &ਮੁੱਲ, &unit);
		ਜੇ (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(ਮੁੱਲ, F_100));
		} ਨਹੀਂਤਾਂ {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					ਮੁੱਲ, unit));
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
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ layout_handle_box_sizing(
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਅਚਲ ਬਣਤਰ box *box,
		ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ,
		bool setwidth,
		ਪੂਰਨ *dimension)
{
	ਗਿਣਤੀ css_box_sizing_e bs;

	ਪ੍ਰਤਿਜ੍ਞਾ(box && box->style);

	bs = css_computed_box_sizing(box->style);

	ਜੇ (bs == CSS_BOX_SIZING_BORDER_BOX) {
		ਪੂਰਨ orig = *dimension;
		ਪੂਰਨ fixed = 0;
		ਦਸ਼ਮਲਵ frac = 0;

		calculate_mbp_ਚੌੜਾਈ(unit_len_ctx, box->style,
				setwidth ? ਖੱਬਾ_2 : ਸਿਖਰ_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_ਚੌੜਾਈ(unit_len_ctx, box->style,
				setwidth ? ਸੱਜਾ_2 : BOTTOM,
				false, true, true, &fixed, &frac);
		orig -= frac * ਉਪਲਬਧ_ਚੌੜਾਈ + fixed;
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
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ layout_ਲੱਭੋ_dimensions(
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ,
		ਪੂਰਨ viewport_ਉਚਾਈ,
		ਅਚਲ ਬਣਤਰ box *box,
		ਅਚਲ css_computed_style *style,
		ਪੂਰਨ *ਚੌੜਾਈ,
		ਪੂਰਨ *ਉਚਾਈ,
		ਪੂਰਨ *max_ਚੌੜਾਈ,
		ਪੂਰਨ *min_ਚੌੜਾਈ,
		ਪੂਰਨ *max_ਉਚਾਈ,
		ਪੂਰਨ *min_ਉਚਾਈ,
		ਪੂਰਨ margin[4],
		ਪੂਰਨ padding[4],
		ਬਣਤਰ box_border border[4])
{
	ਬਣਤਰ box *containing_ਖੰਡ = ਨਿਰਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	ਜੇ (ਚੌੜਾਈ) {
		ਜੇ (css_computed_ਚੌੜਾਈ_px(style, unit_len_ctx,
				ਉਪਲਬਧ_ਚੌੜਾਈ, ਚੌੜਾਈ) == CSS_ਚੌੜਾਈ_ਸੈੱਟ) {
			layout_handle_box_sizing(unit_len_ctx, box,
					ਉਪਲਬਧ_ਚੌੜਾਈ, true, ਚੌੜਾਈ);
		} ਨਹੀਂਤਾਂ {
			*ਚੌੜਾਈ = AUTO;
		}
	}

	ਜੇ (ਉਚਾਈ) {
		ਗਿਣਤੀ css_ਉਚਾਈ_e htype;
		css_fixed ਮੁੱਲ = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_ਉਚਾਈ(style, &ਮੁੱਲ, &unit);

		ਜੇ (htype == CSS_ਉਚਾਈ_ਸੈੱਟ) {
			ਜੇ (unit == CSS_UNIT_PCT) {
				ਗਿਣਤੀ css_ਉਚਾਈ_e cbhtype;

				ਜੇ (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->ਮਾਪਾ) {
					/* Box is absolutely positioned */
					ਪ੍ਰਤਿਜ੍ਞਾ(box->float_container);
					containing_ਖੰਡ = box->float_container;
				} ਨਹੀਂਤਾਂ ਜੇ (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_ਖੱਬਾ ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_ਸੱਜਾ)) {
					/* Box is a float */
					ਪ੍ਰਤਿਜ੍ਞਾ(box->ਮਾਪਾ &&
						box->ਮਾਪਾ->ਮਾਪਾ &&
						box->ਮਾਪਾ->ਮਾਪਾ->ਮਾਪਾ);

					containing_ਖੰਡ =
						box->ਮਾਪਾ->ਮਾਪਾ->ਮਾਪਾ;
				} ਨਹੀਂਤਾਂ ਜੇ (box->ਮਾਪਾ && box->ਮਾਪਾ->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_ਖੰਡ = box->ਮਾਪਾ;
				} ਨਹੀਂਤਾਂ ਜੇ (box->ਮਾਪਾ && box->ਮਾਪਾ->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					ਪ੍ਰਤਿਜ੍ਞਾ(box->ਮਾਪਾ->ਮਾਪਾ);
					containing_ਖੰਡ = box->ਮਾਪਾ->ਮਾਪਾ;
				}

				ਜੇ (containing_ਖੰਡ) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_ਉਚਾਈ(
							containing_ਖੰਡ->style,
							&f, &u);
				}

				ਜੇ (containing_ਖੰਡ &&
					containing_ਖੰਡ->ਉਚਾਈ != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_ਉਚਾਈ_ਸੈੱਟ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*ਉਚਾਈ = FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ,
						containing_ਖੰਡ->ਉਚਾਈ);
				} ਨਹੀਂਤਾਂ ਜੇ ((!box->ਮਾਪਾ ||
						!box->ਮਾਪਾ->ਮਾਪਾ) &&
						viewport_ਉਚਾਈ >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*ਉਚਾਈ = FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ,
							viewport_ਉਚਾਈ);
				} ਨਹੀਂਤਾਂ {
					/* precentage height not permissible
					 * treat height as auto */
					*ਉਚਾਈ = AUTO;
				}
			} ਨਹੀਂਤਾਂ {
				*ਉਚਾਈ = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ਮੁੱਲ, unit));
			}
		} ਨਹੀਂਤਾਂ {
			*ਉਚਾਈ = AUTO;
		}

		ਜੇ (*ਉਚਾਈ != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					ਉਪਲਬਧ_ਚੌੜਾਈ, false, ਉਚਾਈ);
		}
	}

	ਜੇ (max_ਚੌੜਾਈ) {
		ਗਿਣਤੀ css_max_ਚੌੜਾਈ_e type;
		css_fixed ਮੁੱਲ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_ਚੌੜਾਈ(style, &ਮੁੱਲ, &unit);

		ਜੇ (type == CSS_MAX_ਚੌੜਾਈ_ਸੈੱਟ) {
			ਜੇ (unit == CSS_UNIT_PCT) {
				*max_ਚੌੜਾਈ = FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ,
						ਉਪਲਬਧ_ਚੌੜਾਈ);
			} ਨਹੀਂਤਾਂ {
				*max_ਚੌੜਾਈ = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ਮੁੱਲ, unit));
			}
		} ਨਹੀਂਤਾਂ {
			/* Inadmissible */
			*max_ਚੌੜਾਈ = -1;
		}

		ਜੇ (*max_ਚੌੜਾਈ != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					ਉਪਲਬਧ_ਚੌੜਾਈ, true, max_ਚੌੜਾਈ);
		}
	}

	ਜੇ (min_ਚੌੜਾਈ) {
		ਗਿਣਤੀ css_min_ਚੌੜਾਈ_e type;
		css_fixed ਮੁੱਲ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_ਚੌੜਾਈ(style, &ਮੁੱਲ, &unit);

		ਜੇ (type == CSS_MIN_ਚੌੜਾਈ_ਸੈੱਟ) {
			ਜੇ (unit == CSS_UNIT_PCT) {
				*min_ਚੌੜਾਈ = FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ,
						ਉਪਲਬਧ_ਚੌੜਾਈ);
			} ਨਹੀਂਤਾਂ {
				*min_ਚੌੜਾਈ = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ਮੁੱਲ, unit));
			}
		} ਨਹੀਂਤਾਂ {
			/* Inadmissible */
			*min_ਚੌੜਾਈ = 0;
		}

		ਜੇ (*min_ਚੌੜਾਈ != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					ਉਪਲਬਧ_ਚੌੜਾਈ, true, min_ਚੌੜਾਈ);
		}
	}

	ਜੇ (max_ਉਚਾਈ) {
		ਗਿਣਤੀ css_max_ਉਚਾਈ_e type;
		css_fixed ਮੁੱਲ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_ਉਚਾਈ(style, &ਮੁੱਲ, &unit);

		ਜੇ (type == CSS_MAX_ਉਚਾਈ_ਸੈੱਟ) {
			ਜੇ (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_ਉਚਾਈ = -1;
			} ਨਹੀਂਤਾਂ {
				*max_ਉਚਾਈ = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ਮੁੱਲ, unit));
			}
		} ਨਹੀਂਤਾਂ {
			/* Inadmissible */
			*max_ਉਚਾਈ = -1;
		}
	}

	ਜੇ (min_ਉਚਾਈ) {
		ਗਿਣਤੀ css_min_ਉਚਾਈ_e type;
		css_fixed ਮੁੱਲ = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_ਉਚਾਈ(style, &ਮੁੱਲ, &unit);

		ਜੇ (type == CSS_MIN_ਉਚਾਈ_ਸੈੱਟ) {
			ਜੇ (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_ਉਚਾਈ = 0;
			} ਨਹੀਂਤਾਂ {
				*min_ਉਚਾਈ = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ਮੁੱਲ, unit));
			}
		} ਨਹੀਂਤਾਂ {
			/* Inadmissible */
			*min_ਉਚਾਈ = 0;
		}
	}

	ਲਈ (i = 0; i != 4; i++) {
		ਜੇ (margin) {
			ਗਿਣਤੀ css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed ਮੁੱਲ = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &ਮੁੱਲ, &unit);

			ਜੇ (type == CSS_MARGIN_ਸੈੱਟ) {
				ਜੇ (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ,
							ਉਪਲਬਧ_ਚੌੜਾਈ);
				} ਨਹੀਂਤਾਂ {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							ਮੁੱਲ, unit));
				}
			} ਨਹੀਂਤਾਂ {
				margin[i] = AUTO;
			}
		}

		ਜੇ (padding) {
			css_fixed ਮੁੱਲ = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &ਮੁੱਲ, &unit);

			ਜੇ (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ,
						ਉਪਲਬਧ_ਚੌੜਾਈ);
			} ਨਹੀਂਤਾਂ {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ਮੁੱਲ, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		ਜੇ (border && box->type != BOX_ਸਾਰਣੀ_CELL) {
			ਗਿਣਤੀ css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed ਮੁੱਲ = 0;
			css_unit unit = CSS_UNIT_PX;

			border_ਚੌੜਾਈ_funcs[i](style, &ਮੁੱਲ, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			ਜੇ (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].ਚੌੜਾਈ = 0;
			ਨਹੀਂਤਾਂ
				border[i].ਚੌੜਾਈ = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						ਮੁੱਲ, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			ਜੇ (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_ਸਾਰਣੀ ||
					 box->type == BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ ||
					 box->type == BOX_ਸਾਰਣੀ_ਪੰਕਤੀ))
				border[i].ਚੌੜਾਈ = 0;
		}
	}
}

#ਜੇ_ਅੰਤ
