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
		কাঠামো box *block,
		পূর্ণসংখ্যা viewport_height,
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
		কাঠামো box *table,
		পূর্ণসংখ্যা available_width,
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
		কাঠামো box *flex,
		পূর্ণসংখ্যা available_width,
		html_content *content);

প্রকারলেখ uint8_t (*css_len_func)(
		ধ্রুবক css_computed_style *style,
		css_fixed *দৈর্ঘ্য, css_unit *unit);
প্রকারলেখ uint8_t (*css_border_style_func)(
		ধ্রুবক css_computed_style *style);
প্রকারলেখ uint8_t (*css_border_color_func)(
		ধ্রুবক css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
বাহ্যিক ধ্রুবক css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
বাহ্যিক ধ্রুবক css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
বাহ্যিক ধ্রুবক css_len_func border_width_funcs[4];

/** Array of per-side access functions for computed style border styles. */
বাহ্যিক ধ্রুবক css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
বাহ্যিক ধ্রুবক css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
স্থির অন্তঃরেখ bool lh__box_is_float_box(ধ্রুবক কাঠামো box *b)
{
	ফেরত b->type == BOX_FLOAT_বাম ||
	       b->type == BOX_FLOAT_ডান;
}

/** Layout helper: Check whether box takes part in inline flow. */
স্থির অন্তঃরেখ bool lh__box_is_inline_flow(ধ্রুবক কাঠামো box *b)
{
	ফেরত b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_TEXT ||
	       b->type == BOX_INLINE_END;
}

/** Layout helper: Check whether box takes part in inline flow. */
স্থির অন্তঃরেখ bool lh__box_is_flex_container(ধ্রুবক কাঠামো box *b)
{
	ফেরত b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
স্থির অন্তঃরেখ bool lh__box_is_flex_item(ধ্রুবক কাঠামো box *b)
{
	ফেরত (b->parent != ফাঁকা) && lh__box_is_flex_container(b->parent);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
স্থির অন্তঃরেখ bool lh__box_is_inline_level(ধ্রুবক কাঠামো box *b)
{
	ফেরত lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
স্থির অন্তঃরেখ bool lh__box_is_inline_content(ধ্রুবক কাঠামো box *b)
{
	ফেরত lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
স্থির অন্তঃরেখ bool lh__box_is_object(ধ্রুবক কাঠামো box *b)
{
	ফেরত b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
স্থির অন্তঃরেখ bool lh__box_is_replace(ধ্রুবক কাঠামো box *b)
{
	ফেরত b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
স্থির অন্তঃরেখ bool lh__have_border(
		গণনা box_side side,
		ধ্রুবক css_computed_style *style)
{
	ফেরত border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

স্থির অন্তঃরেখ bool lh__box_is_absolute(ধ্রুবক কাঠামো box *b)
{
	ফেরত css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

স্থির অন্তঃরেখ bool lh__flex_main_is_horizontal(ধ্রুবক কাঠামো box *flex)
{
	ধ্রুবক css_computed_style *style = flex->style;

	প্রতিজ্ঞা(style != ফাঁকা);

	নির্বাচন (css_computed_flex_direction(style)) {
	পূর্বনির্ধারিত:                        /* Fallthrough. */
	ক্ষেত্র CSS_FLEX_DIRECTION_ROW:    /* Fallthrough. */
	ক্ষেত্র CSS_FLEX_DIRECTION_ROW_REVERSE:
		ফেরত true;
	ক্ষেত্র CSS_FLEX_DIRECTION_COLUMN: /* Fallthrough. */
	ক্ষেত্র CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		ফেরত false;
	}
}

স্থির অন্তঃরেখ bool lh__flex_direction_reversed(ধ্রুবক কাঠামো box *flex)
{
	নির্বাচন (css_computed_flex_direction(flex->style)) {
	পূর্বনির্ধারিত:                             /* Fallthrough. */
	ক্ষেত্র CSS_FLEX_DIRECTION_ROW_REVERSE: /* Fallthrough. */
	ক্ষেত্র CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		ফেরত true;
	ক্ষেত্র CSS_FLEX_DIRECTION_ROW: /* Fallthrough. */
	ক্ষেত্র CSS_FLEX_DIRECTION_COLUMN:
		ফেরত false;
	}
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা lh__non_auto_margin(ধ্রুবক কাঠামো box *b, গণনা box_side side)
{
	ফেরত (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা lh__delta_outer_height(ধ্রুবক কাঠামো box *b)
{
	ফেরত b->padding[শীর্ষ_2] +
	       b->padding[BOTTOM] +
	       b->border[শীর্ষ_2].width +
	       b->border[BOTTOM].width +
	       lh__non_auto_margin(b, শীর্ষ_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা lh__delta_outer_width(ধ্রুবক কাঠামো box *b)
{
	ফেরত b->padding[বাম_2] +
	       b->padding[ডান_2] +
	       b->border[বাম_2].width +
	       b->border[ডান_2].width +
	       lh__non_auto_margin(b, বাম_2) +
	       lh__non_auto_margin(b, ডান_2);
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা lh__delta_outer_main(
		ধ্রুবক কাঠামো box *flex,
		ধ্রুবক কাঠামো box *b)
{
	যদি (lh__flex_main_is_horizontal(flex)) {
		ফেরত lh__delta_outer_width(b);
	} অন্যথা {
		ফেরত lh__delta_outer_height(b);
	}
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা lh__delta_outer_cross(
		ধ্রুবক কাঠামো box *flex,
		ধ্রুবক কাঠামো box *b)
{
	যদি (lh__flex_main_is_horizontal(flex) == false) {
		ফেরত lh__delta_outer_width(b);
	} অন্যথা {
		ফেরত lh__delta_outer_height(b);
	}
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা *lh_box_আকার_main_ptr(
		bool horizontal,
		কাঠামো box *b)
{
	ফেরত horizontal ? &b->width : &b->height;
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা *lh_box_আকার_cross_ptr(
		bool horizontal,
		কাঠামো box *b)
{
	ফেরত horizontal ? &b->height : &b->width;
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা lh_box_আকার_main(
		bool horizontal,
		ধ্রুবক কাঠামো box *b)
{
	ফেরত horizontal ? b->width : b->height;
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা lh_box_আকার_cross(
		bool horizontal,
		ধ্রুবক কাঠামো box *b)
{
	ফেরত horizontal ? b->height : b->width;
}

স্থির অন্তঃরেখ bool lh_box_আকার_cross_is_auto(
		bool horizontal,
		কাঠামো box *b)
{
	css_fixed দৈর্ঘ্য;
	css_unit unit;

	যদি (horizontal) {
		ফেরত css_computed_height(b->style,
				&দৈর্ঘ্য, &unit) == CSS_HEIGHT_AUTO;
	} অন্যথা {
		ফেরত css_computed_width(b->style,
				&দৈর্ঘ্য, &unit) == CSS_WIDTH_AUTO;
	}
}

স্থির অন্তঃরেখ গণনা css_align_self_e lh__box_align_self(
		ধ্রুবক কাঠামো box *flex,
		ধ্রুবক কাঠামো box *item)
{
	গণনা css_align_self_e align_self = css_computed_align_self(item->style);

	যদি (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_বস্তুসমূহ(flex->style);
	}

	ফেরত align_self;
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
স্থির অন্তঃরেখ শূন্য calculate_mbp_width(
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		ধ্রুবক css_computed_style *style,
		unsigned পূর্ণসংখ্যা side,
		bool margin,
		bool border,
		bool padding,
		পূর্ণসংখ্যা *fixed,
		দশমিক *frac)
{
	css_fixed মান = 0;
	css_unit unit = CSS_UNIT_PX;

	প্রতিজ্ঞা(style);

	/* margin */
	যদি (margin) {
		গণনা css_margin_e type;

		type = margin_funcs[side](style, &মান, &unit);
		যদি (type == CSS_MARGIN_SET) {
			যদি (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(মান, F_100));
			} অন্যথা {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						মান, unit));
			}
		}
	}

	/* border */
	যদি (border) {
		যদি (lh__have_border(side, style)) {
			border_width_funcs[side](style, &মান, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					মান, unit));
		}
	}

	/* padding */
	যদি (padding) {
		padding_funcs[side](style, &মান, &unit);
		যদি (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(মান, F_100));
		} অন্যথা {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					মান, unit));
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
স্থির অন্তঃরেখ শূন্য layout_handle_box_sizing(
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		ধ্রুবক কাঠামো box *box,
		পূর্ণসংখ্যা available_width,
		bool setwidth,
		পূর্ণসংখ্যা *dimension)
{
	গণনা css_box_sizing_e bs;

	প্রতিজ্ঞা(box && box->style);

	bs = css_computed_box_sizing(box->style);

	যদি (bs == CSS_BOX_SIZING_BORDER_BOX) {
		পূর্ণসংখ্যা orig = *dimension;
		পূর্ণসংখ্যা fixed = 0;
		দশমিক frac = 0;

		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? বাম_2 : শীর্ষ_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? ডান_2 : BOTTOM,
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
স্থির অন্তঃরেখ শূন্য layout_find_dimensions(
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		পূর্ণসংখ্যা available_width,
		পূর্ণসংখ্যা viewport_height,
		ধ্রুবক কাঠামো box *box,
		ধ্রুবক css_computed_style *style,
		পূর্ণসংখ্যা *width,
		পূর্ণসংখ্যা *height,
		পূর্ণসংখ্যা *max_width,
		পূর্ণসংখ্যা *min_width,
		পূর্ণসংখ্যা *max_height,
		পূর্ণসংখ্যা *min_height,
		পূর্ণসংখ্যা margin[4],
		পূর্ণসংখ্যা padding[4],
		কাঠামো box_border border[4])
{
	কাঠামো box *containing_block = ফাঁকা;
	unsigned পূর্ণসংখ্যা i;

	যদি (width) {
		যদি (css_computed_width_px(style, unit_len_ctx,
				available_width, width) == CSS_WIDTH_SET) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, width);
		} অন্যথা {
			*width = AUTO;
		}
	}

	যদি (height) {
		গণনা css_height_e htype;
		css_fixed মান = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_height(style, &মান, &unit);

		যদি (htype == CSS_HEIGHT_SET) {
			যদি (unit == CSS_UNIT_PCT) {
				গণনা css_height_e cbhtype;

				যদি (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->parent) {
					/* Box is absolutely positioned */
					প্রতিজ্ঞা(box->float_container);
					containing_block = box->float_container;
				} অন্যথা যদি (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_বাম ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_ডান)) {
					/* Box is a float */
					প্রতিজ্ঞা(box->parent &&
						box->parent->parent &&
						box->parent->parent->parent);

					containing_block =
						box->parent->parent->parent;
				} অন্যথা যদি (box->parent && box->parent->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_block = box->parent;
				} অন্যথা যদি (box->parent && box->parent->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					প্রতিজ্ঞা(box->parent->parent);
					containing_block = box->parent->parent;
				}

				যদি (containing_block) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_height(
							containing_block->style,
							&f, &u);
				}

				যদি (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*height = FPCT_OF_INT_TOINT(মান,
						containing_block->height);
				} অন্যথা যদি ((!box->parent ||
						!box->parent->parent) &&
						viewport_height >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*height = FPCT_OF_INT_TOINT(মান,
							viewport_height);
				} অন্যথা {
					/* precentage height not permissible
					 * treat height as auto */
					*height = AUTO;
				}
			} অন্যথা {
				*height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						মান, unit));
			}
		} অন্যথা {
			*height = AUTO;
		}

		যদি (*height != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, false, height);
		}
	}

	যদি (max_width) {
		গণনা css_max_width_e type;
		css_fixed মান = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_width(style, &মান, &unit);

		যদি (type == CSS_MAX_WIDTH_SET) {
			যদি (unit == CSS_UNIT_PCT) {
				*max_width = FPCT_OF_INT_TOINT(মান,
						available_width);
			} অন্যথা {
				*max_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						মান, unit));
			}
		} অন্যথা {
			/* Inadmissible */
			*max_width = -1;
		}

		যদি (*max_width != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, max_width);
		}
	}

	যদি (min_width) {
		গণনা css_min_width_e type;
		css_fixed মান = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_width(style, &মান, &unit);

		যদি (type == CSS_MIN_WIDTH_SET) {
			যদি (unit == CSS_UNIT_PCT) {
				*min_width = FPCT_OF_INT_TOINT(মান,
						available_width);
			} অন্যথা {
				*min_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						মান, unit));
			}
		} অন্যথা {
			/* Inadmissible */
			*min_width = 0;
		}

		যদি (*min_width != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, min_width);
		}
	}

	যদি (max_height) {
		গণনা css_max_height_e type;
		css_fixed মান = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_height(style, &মান, &unit);

		যদি (type == CSS_MAX_HEIGHT_SET) {
			যদি (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_height = -1;
			} অন্যথা {
				*max_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						মান, unit));
			}
		} অন্যথা {
			/* Inadmissible */
			*max_height = -1;
		}
	}

	যদি (min_height) {
		গণনা css_min_height_e type;
		css_fixed মান = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_height(style, &মান, &unit);

		যদি (type == CSS_MIN_HEIGHT_SET) {
			যদি (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_height = 0;
			} অন্যথা {
				*min_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						মান, unit));
			}
		} অন্যথা {
			/* Inadmissible */
			*min_height = 0;
		}
	}

	জন্য (i = 0; i != 4; i++) {
		যদি (margin) {
			গণনা css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed মান = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &মান, &unit);

			যদি (type == CSS_MARGIN_SET) {
				যদি (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_OF_INT_TOINT(মান,
							available_width);
				} অন্যথা {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							মান, unit));
				}
			} অন্যথা {
				margin[i] = AUTO;
			}
		}

		যদি (padding) {
			css_fixed মান = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &মান, &unit);

			যদি (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_OF_INT_TOINT(মান,
						available_width);
			} অন্যথা {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						মান, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		যদি (border && box->type != BOX_TABLE_CELL) {
			গণনা css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed মান = 0;
			css_unit unit = CSS_UNIT_PX;

			border_width_funcs[i](style, &মান, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			যদি (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].width = 0;
			অন্যথা
				border[i].width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						মান, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			যদি (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_TABLE ||
					 box->type == BOX_TABLE_ROW_GROUP ||
					 box->type == BOX_TABLE_ROW))
				border[i].width = 0;
		}
	}
}

#endif
