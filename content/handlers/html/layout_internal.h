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
		संरचना box *block,
		पूर्णाङ्क viewport_height,
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
		संरचना box *table,
		पूर्णाङ्क available_width,
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
		संरचना box *flex,
		पूर्णाङ्क available_width,
		html_content *content);

प्रकारनिर्देश uint8_t (*css_len_func)(
		ध्रुव css_computed_style *style,
		css_fixed *length, css_unit *unit);
प्रकारनिर्देश uint8_t (*css_border_style_func)(
		ध्रुव css_computed_style *style);
प्रकारनिर्देश uint8_t (*css_border_color_func)(
		ध्रुव css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
बाह्य ध्रुव css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
बाह्य ध्रुव css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
बाह्य ध्रुव css_len_func border_width_funcs[4];

/** Array of per-side access functions for computed style border styles. */
बाह्य ध्रुव css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
बाह्य ध्रुव css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
स्थिर अन्तःस्थ bool lh__box_is_float_box(ध्रुव संरचना box *b)
{
	पटिनिवत्त b->type == BOX_FLOAT_वामः ||
	       b->type == BOX_FLOAT_दक्षिणः;
}

/** Layout helper: Check whether box takes part in inline flow. */
स्थिर अन्तःस्थ bool lh__box_is_inline_flow(ध्रुव संरचना box *b)
{
	पटिनिवत्त b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_TEXT ||
	       b->type == BOX_INLINE_END;
}

/** Layout helper: Check whether box takes part in inline flow. */
स्थिर अन्तःस्थ bool lh__box_is_flex_container(ध्रुव संरचना box *b)
{
	पटिनिवत्त b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
स्थिर अन्तःस्थ bool lh__box_is_flex_item(ध्रुव संरचना box *b)
{
	पटिनिवत्त (b->parent != रिक्त) && lh__box_is_flex_container(b->parent);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
स्थिर अन्तःस्थ bool lh__box_is_inline_level(ध्रुव संरचना box *b)
{
	पटिनिवत्त lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
स्थिर अन्तःस्थ bool lh__box_is_inline_content(ध्रुव संरचना box *b)
{
	पटिनिवत्त lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
स्थिर अन्तःस्थ bool lh__box_is_object(ध्रुव संरचना box *b)
{
	पटिनिवत्त b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
स्थिर अन्तःस्थ bool lh__box_is_replace(ध्रुव संरचना box *b)
{
	पटिनिवत्त b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
स्थिर अन्तःस्थ bool lh__have_border(
		गणना box_side side,
		ध्रुव css_computed_style *style)
{
	पटिनिवत्त border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

स्थिर अन्तःस्थ bool lh__box_is_absolute(ध्रुव संरचना box *b)
{
	पटिनिवत्त css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

स्थिर अन्तःस्थ bool lh__flex_main_is_horizontal(ध्रुव संरचना box *flex)
{
	ध्रुव css_computed_style *style = flex->style;

	प्रतिज्ञा(style != रिक्त);

	विकल्प (css_computed_flex_direction(style)) {
	स्वभाव:                        /* Fallthrough. */
	पक्ष CSS_FLEX_DIRECTION_ROW:    /* Fallthrough. */
	पक्ष CSS_FLEX_DIRECTION_ROW_REVERSE:
		पटिनिवत्त true;
	पक्ष CSS_FLEX_DIRECTION_COLUMN: /* Fallthrough. */
	पक्ष CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		पटिनिवत्त false;
	}
}

स्थिर अन्तःस्थ bool lh__flex_direction_reversed(ध्रुव संरचना box *flex)
{
	विकल्प (css_computed_flex_direction(flex->style)) {
	स्वभाव:                             /* Fallthrough. */
	पक्ष CSS_FLEX_DIRECTION_ROW_REVERSE: /* Fallthrough. */
	पक्ष CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		पटिनिवत्त true;
	पक्ष CSS_FLEX_DIRECTION_ROW: /* Fallthrough. */
	पक्ष CSS_FLEX_DIRECTION_COLUMN:
		पटिनिवत्त false;
	}
}

स्थिर अन्तःस्थ पूर्णाङ्क lh__non_auto_margin(ध्रुव संरचना box *b, गणना box_side side)
{
	पटिनिवत्त (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

स्थिर अन्तःस्थ पूर्णाङ्क lh__delta_outer_height(ध्रुव संरचना box *b)
{
	पटिनिवत्त b->padding[शिखरम्_2] +
	       b->padding[BOTTOM] +
	       b->border[शिखरम्_2].width +
	       b->border[BOTTOM].width +
	       lh__non_auto_margin(b, शिखरम्_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

स्थिर अन्तःस्थ पूर्णाङ्क lh__delta_outer_width(ध्रुव संरचना box *b)
{
	पटिनिवत्त b->padding[वामः_2] +
	       b->padding[दक्षिणः_2] +
	       b->border[वामः_2].width +
	       b->border[दक्षिणः_2].width +
	       lh__non_auto_margin(b, वामः_2) +
	       lh__non_auto_margin(b, दक्षिणः_2);
}

स्थिर अन्तःस्थ पूर्णाङ्क lh__delta_outer_main(
		ध्रुव संरचना box *flex,
		ध्रुव संरचना box *b)
{
	सचे (lh__flex_main_is_horizontal(flex)) {
		पटिनिवत्त lh__delta_outer_width(b);
	} अञ्ञथा {
		पटिनिवत्त lh__delta_outer_height(b);
	}
}

स्थिर अन्तःस्थ पूर्णाङ्क lh__delta_outer_cross(
		ध्रुव संरचना box *flex,
		ध्रुव संरचना box *b)
{
	सचे (lh__flex_main_is_horizontal(flex) == false) {
		पटिनिवत्त lh__delta_outer_width(b);
	} अञ्ञथा {
		पटिनिवत्त lh__delta_outer_height(b);
	}
}

स्थिर अन्तःस्थ पूर्णाङ्क *lh__box_size_main_ptr(
		bool horizontal,
		संरचना box *b)
{
	पटिनिवत्त horizontal ? &b->width : &b->height;
}

स्थिर अन्तःस्थ पूर्णाङ्क *lh__box_size_cross_ptr(
		bool horizontal,
		संरचना box *b)
{
	पटिनिवत्त horizontal ? &b->height : &b->width;
}

स्थिर अन्तःस्थ पूर्णाङ्क lh__box_size_main(
		bool horizontal,
		ध्रुव संरचना box *b)
{
	पटिनिवत्त horizontal ? b->width : b->height;
}

स्थिर अन्तःस्थ पूर्णाङ्क lh__box_size_cross(
		bool horizontal,
		ध्रुव संरचना box *b)
{
	पटिनिवत्त horizontal ? b->height : b->width;
}

स्थिर अन्तःस्थ bool lh__box_size_cross_is_auto(
		bool horizontal,
		संरचना box *b)
{
	css_fixed length;
	css_unit unit;

	सचे (horizontal) {
		पटिनिवत्त css_computed_height(b->style,
				&length, &unit) == CSS_HEIGHT_AUTO;
	} अञ्ञथा {
		पटिनिवत्त css_computed_width(b->style,
				&length, &unit) == CSS_WIDTH_AUTO;
	}
}

स्थिर अन्तःस्थ गणना css_align_self_e lh__box_align_self(
		ध्रुव संरचना box *flex,
		ध्रुव संरचना box *item)
{
	गणना css_align_self_e align_self = css_computed_align_self(item->style);

	सचे (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_वस्तूनि(flex->style);
	}

	पटिनिवत्त align_self;
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
स्थिर अन्तःस्थ शून्य calculate_mbp_width(
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव css_computed_style *style,
		unsigned पूर्णाङ्क side,
		bool margin,
		bool border,
		bool padding,
		पूर्णाङ्क *fixed,
		भिन्न *frac)
{
	css_fixed मानम् = 0;
	css_unit unit = CSS_UNIT_PX;

	प्रतिज्ञा(style);

	/* margin */
	सचे (margin) {
		गणना css_margin_e type;

		type = margin_funcs[side](style, &मानम्, &unit);
		सचे (type == CSS_MARGIN_SET) {
			सचे (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(मानम्, F_100));
			} अञ्ञथा {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मानम्, unit));
			}
		}
	}

	/* border */
	सचे (border) {
		सचे (lh__have_border(side, style)) {
			border_width_funcs[side](style, &मानम्, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					मानम्, unit));
		}
	}

	/* padding */
	सचे (padding) {
		padding_funcs[side](style, &मानम्, &unit);
		सचे (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(मानम्, F_100));
		} अञ्ञथा {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					मानम्, unit));
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
स्थिर अन्तःस्थ शून्य layout_handle_box_sizing(
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव संरचना box *box,
		पूर्णाङ्क available_width,
		bool setwidth,
		पूर्णाङ्क *dimension)
{
	गणना css_box_sizing_e bs;

	प्रतिज्ञा(box && box->style);

	bs = css_computed_box_sizing(box->style);

	सचे (bs == CSS_BOX_SIZING_BORDER_BOX) {
		पूर्णाङ्क orig = *dimension;
		पूर्णाङ्क fixed = 0;
		भिन्न frac = 0;

		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? वामः_2 : शिखरम्_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? दक्षिणः_2 : BOTTOM,
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
स्थिर अन्तःस्थ शून्य layout_find_dimensions(
		ध्रुव css_unit_ctx *unit_len_ctx,
		पूर्णाङ्क available_width,
		पूर्णाङ्क viewport_height,
		ध्रुव संरचना box *box,
		ध्रुव css_computed_style *style,
		पूर्णाङ्क *width,
		पूर्णाङ्क *height,
		पूर्णाङ्क *max_width,
		पूर्णाङ्क *min_width,
		पूर्णाङ्क *max_height,
		पूर्णाङ्क *min_height,
		पूर्णाङ्क margin[4],
		पूर्णाङ्क padding[4],
		संरचना box_border border[4])
{
	संरचना box *containing_block = रिक्त;
	unsigned पूर्णाङ्क i;

	सचे (width) {
		सचे (css_computed_width_px(style, unit_len_ctx,
				available_width, width) == CSS_WIDTH_SET) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, width);
		} अञ्ञथा {
			*width = AUTO;
		}
	}

	सचे (height) {
		गणना css_height_e htype;
		css_fixed मानम् = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_height(style, &मानम्, &unit);

		सचे (htype == CSS_HEIGHT_SET) {
			सचे (unit == CSS_UNIT_PCT) {
				गणना css_height_e cbhtype;

				सचे (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->parent) {
					/* Box is absolutely positioned */
					प्रतिज्ञा(box->float_container);
					containing_block = box->float_container;
				} अञ्ञथा सचे (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_वामः ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_दक्षिणः)) {
					/* Box is a float */
					प्रतिज्ञा(box->parent &&
						box->parent->parent &&
						box->parent->parent->parent);

					containing_block =
						box->parent->parent->parent;
				} अञ्ञथा सचे (box->parent && box->parent->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_block = box->parent;
				} अञ्ञथा सचे (box->parent && box->parent->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					प्रतिज्ञा(box->parent->parent);
					containing_block = box->parent->parent;
				}

				सचे (containing_block) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_height(
							containing_block->style,
							&f, &u);
				}

				सचे (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*height = FPCT_OF_INT_TOINT(मानम्,
						containing_block->height);
				} अञ्ञथा सचे ((!box->parent ||
						!box->parent->parent) &&
						viewport_height >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*height = FPCT_OF_INT_TOINT(मानम्,
							viewport_height);
				} अञ्ञथा {
					/* precentage height not permissible
					 * treat height as auto */
					*height = AUTO;
				}
			} अञ्ञथा {
				*height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मानम्, unit));
			}
		} अञ्ञथा {
			*height = AUTO;
		}

		सचे (*height != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, false, height);
		}
	}

	सचे (max_width) {
		गणना css_max_width_e type;
		css_fixed मानम् = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_width(style, &मानम्, &unit);

		सचे (type == CSS_MAX_WIDTH_SET) {
			सचे (unit == CSS_UNIT_PCT) {
				*max_width = FPCT_OF_INT_TOINT(मानम्,
						available_width);
			} अञ्ञथा {
				*max_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मानम्, unit));
			}
		} अञ्ञथा {
			/* Inadmissible */
			*max_width = -1;
		}

		सचे (*max_width != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, max_width);
		}
	}

	सचे (min_width) {
		गणना css_min_width_e type;
		css_fixed मानम् = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_width(style, &मानम्, &unit);

		सचे (type == CSS_MIN_WIDTH_SET) {
			सचे (unit == CSS_UNIT_PCT) {
				*min_width = FPCT_OF_INT_TOINT(मानम्,
						available_width);
			} अञ्ञथा {
				*min_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मानम्, unit));
			}
		} अञ्ञथा {
			/* Inadmissible */
			*min_width = 0;
		}

		सचे (*min_width != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, min_width);
		}
	}

	सचे (max_height) {
		गणना css_max_height_e type;
		css_fixed मानम् = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_height(style, &मानम्, &unit);

		सचे (type == CSS_MAX_HEIGHT_SET) {
			सचे (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_height = -1;
			} अञ्ञथा {
				*max_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मानम्, unit));
			}
		} अञ्ञथा {
			/* Inadmissible */
			*max_height = -1;
		}
	}

	सचे (min_height) {
		गणना css_min_height_e type;
		css_fixed मानम् = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_height(style, &मानम्, &unit);

		सचे (type == CSS_MIN_HEIGHT_SET) {
			सचे (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_height = 0;
			} अञ्ञथा {
				*min_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मानम्, unit));
			}
		} अञ्ञथा {
			/* Inadmissible */
			*min_height = 0;
		}
	}

	अत्थाय (i = 0; i != 4; i++) {
		सचे (margin) {
			गणना css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed मानम् = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &मानम्, &unit);

			सचे (type == CSS_MARGIN_SET) {
				सचे (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_OF_INT_TOINT(मानम्,
							available_width);
				} अञ्ञथा {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							मानम्, unit));
				}
			} अञ्ञथा {
				margin[i] = AUTO;
			}
		}

		सचे (padding) {
			css_fixed मानम् = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &मानम्, &unit);

			सचे (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_OF_INT_TOINT(मानम्,
						available_width);
			} अञ्ञथा {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मानम्, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		सचे (border && box->type != BOX_TABLE_CELL) {
			गणना css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed मानम् = 0;
			css_unit unit = CSS_UNIT_PX;

			border_width_funcs[i](style, &मानम्, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			सचे (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].width = 0;
			अञ्ञथा
				border[i].width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मानम्, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			सचे (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_TABLE ||
					 box->type == BOX_TABLE_ROW_GROUP ||
					 box->type == BOX_TABLE_ROW))
				border[i].width = 0;
		}
	}
}

#endif
