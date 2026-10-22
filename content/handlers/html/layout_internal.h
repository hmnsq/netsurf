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

#अगर_अघोषित NETSURF_HTML_LAYOUT_INTERNAL_H
#घोषणा NETSURF_HTML_LAYOUT_INTERNAL_H

#घोषणा AUTO पूर्णांक_न्यून

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
		काष्ठा box *block,
		पूर्णांक viewport_height,
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
		काष्ठा box *table,
		पूर्णांक available_width,
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
		काष्ठा box *flex,
		पूर्णांक available_width,
		html_content *content);

प्रकार uint8_t (*css_len_func)(
		स्थिर css_computed_style *style,
		css_fixed *लंबाई, css_unit *unit);
प्रकार uint8_t (*css_border_style_func)(
		स्थिर css_computed_style *style);
प्रकार uint8_t (*css_border_color_func)(
		स्थिर css_computed_style *style,
		css_color *color);

/** Array of per-side access functions for computed style margins. */
बाह्य स्थिर css_len_func margin_funcs[4];

/** Array of per-side access functions for computed style paddings. */
बाह्य स्थिर css_len_func padding_funcs[4];

/** Array of per-side access functions for computed style border_widths. */
बाह्य स्थिर css_len_func border_width_funcs[4];

/** Array of per-side access functions for computed style border styles. */
बाह्य स्थिर css_border_style_func border_style_funcs[4];

/** Array of per-side access functions for computed style border colors. */
बाह्य स्थिर css_border_color_func border_color_funcs[4];

/** Layout helper: Check whether box is a float. */
अटल अंतरभूत bool lh__box_is_float_box(स्थिर काष्ठा box *b)
{
	वापस b->type == BOX_FLOAT_बायाँ ||
	       b->type == BOX_FLOAT_दायाँ;
}

/** Layout helper: Check whether box takes part in inline flow. */
अटल अंतरभूत bool lh__box_is_inline_flow(स्थिर काष्ठा box *b)
{
	वापस b->type == BOX_INLINE ||
	       b->type == BOX_INLINE_FLEX ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_TEXT ||
	       b->type == BOX_INLINE_END;
}

/** Layout helper: Check whether box takes part in inline flow. */
अटल अंतरभूत bool lh__box_is_flex_container(स्थिर काष्ठा box *b)
{
	वापस b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX;
}

/** Layout helper: Check whether box takes part in inline flow. */
अटल अंतरभूत bool lh__box_is_flex_item(स्थिर काष्ठा box *b)
{
	वापस (b->parent != शून्य) && lh__box_is_flex_container(b->parent);
}

/** Layout helper: Check whether box is inline level. (Includes BR.) */
अटल अंतरभूत bool lh__box_is_inline_level(स्थिर काष्ठा box *b)
{
	वापस lh__box_is_inline_flow(b) ||
	       b->type == BOX_BR;
}

/** Layout helper: Check whether box is inline level. (Includes BR, floats.) */
अटल अंतरभूत bool lh__box_is_inline_content(स्थिर काष्ठा box *b)
{
	वापस lh__box_is_float_box(b) ||
	       lh__box_is_inline_level(b);
}

/** Layout helper: Check whether box is an object. */
अटल अंतरभूत bool lh__box_is_object(स्थिर काष्ठा box *b)
{
	वापस b->object ||
	       (b->flags & (IFRAME | REPLACE_DIM));
}

/** Layout helper: Check whether box is replaced. */
अटल अंतरभूत bool lh__box_is_replace(स्थिर काष्ठा box *b)
{
	वापस b->gadget ||
	       lh__box_is_object(b);
}

/** Layout helper: Check for CSS border on given side. */
अटल अंतरभूत bool lh__have_border(
		क्रमागत box_side side,
		स्थिर css_computed_style *style)
{
	वापस border_style_funcs[side](style) != CSS_BORDER_STYLE_NONE;
}

अटल अंतरभूत bool lh__box_is_absolute(स्थिर काष्ठा box *b)
{
	वापस css_computed_position(b->style) == CSS_POSITION_ABSOLUTE ||
	       css_computed_position(b->style) == CSS_POSITION_FIXED;
}

अटल अंतरभूत bool lh__flex_main_is_horizontal(स्थिर काष्ठा box *flex)
{
	स्थिर css_computed_style *style = flex->style;

	निश्चित(style != शून्य);

	चयन (css_computed_flex_direction(style)) {
	शेष:                        /* Fallthrough. */
	हाल CSS_FLEX_DIRECTION_ROW:    /* Fallthrough. */
	हाल CSS_FLEX_DIRECTION_ROW_REVERSE:
		वापस true;
	हाल CSS_FLEX_DIRECTION_COLUMN: /* Fallthrough. */
	हाल CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		वापस false;
	}
}

अटल अंतरभूत bool lh__flex_direction_reversed(स्थिर काष्ठा box *flex)
{
	चयन (css_computed_flex_direction(flex->style)) {
	शेष:                             /* Fallthrough. */
	हाल CSS_FLEX_DIRECTION_ROW_REVERSE: /* Fallthrough. */
	हाल CSS_FLEX_DIRECTION_COLUMN_REVERSE:
		वापस true;
	हाल CSS_FLEX_DIRECTION_ROW: /* Fallthrough. */
	हाल CSS_FLEX_DIRECTION_COLUMN:
		वापस false;
	}
}

अटल अंतरभूत पूर्णांक lh__non_auto_margin(स्थिर काष्ठा box *b, क्रमागत box_side side)
{
	वापस (b->margin[side] == AUTO) ? 0 : b->margin[side];
}

अटल अंतरभूत पूर्णांक lh__delta_outer_height(स्थिर काष्ठा box *b)
{
	वापस b->padding[शीर्ष_2] +
	       b->padding[BOTTOM] +
	       b->border[शीर्ष_2].width +
	       b->border[BOTTOM].width +
	       lh__non_auto_margin(b, शीर्ष_2) +
	       lh__non_auto_margin(b, BOTTOM);
}

अटल अंतरभूत पूर्णांक lh__delta_outer_width(स्थिर काष्ठा box *b)
{
	वापस b->padding[बायाँ_2] +
	       b->padding[दायाँ_2] +
	       b->border[बायाँ_2].width +
	       b->border[दायाँ_2].width +
	       lh__non_auto_margin(b, बायाँ_2) +
	       lh__non_auto_margin(b, दायाँ_2);
}

अटल अंतरभूत पूर्णांक lh__delta_outer_main(
		स्थिर काष्ठा box *flex,
		स्थिर काष्ठा box *b)
{
	अगर (lh__flex_main_is_horizontal(flex)) {
		वापस lh__delta_outer_width(b);
	} अन्यथा {
		वापस lh__delta_outer_height(b);
	}
}

अटल अंतरभूत पूर्णांक lh__delta_outer_cross(
		स्थिर काष्ठा box *flex,
		स्थिर काष्ठा box *b)
{
	अगर (lh__flex_main_is_horizontal(flex) == false) {
		वापस lh__delta_outer_width(b);
	} अन्यथा {
		वापस lh__delta_outer_height(b);
	}
}

अटल अंतरभूत पूर्णांक *lh_box_आकार_main_ptr(
		bool horizontal,
		काष्ठा box *b)
{
	वापस horizontal ? &b->width : &b->height;
}

अटल अंतरभूत पूर्णांक *lh_box_आकार_cross_ptr(
		bool horizontal,
		काष्ठा box *b)
{
	वापस horizontal ? &b->height : &b->width;
}

अटल अंतरभूत पूर्णांक lh_box_आकार_main(
		bool horizontal,
		स्थिर काष्ठा box *b)
{
	वापस horizontal ? b->width : b->height;
}

अटल अंतरभूत पूर्णांक lh_box_आकार_cross(
		bool horizontal,
		स्थिर काष्ठा box *b)
{
	वापस horizontal ? b->height : b->width;
}

अटल अंतरभूत bool lh_box_आकार_cross_is_auto(
		bool horizontal,
		काष्ठा box *b)
{
	css_fixed लंबाई;
	css_unit unit;

	अगर (horizontal) {
		वापस css_computed_height(b->style,
				&लंबाई, &unit) == CSS_HEIGHT_AUTO;
	} अन्यथा {
		वापस css_computed_width(b->style,
				&लंबाई, &unit) == CSS_WIDTH_AUTO;
	}
}

अटल अंतरभूत क्रमागत css_align_self_e lh__box_align_self(
		स्थिर काष्ठा box *flex,
		स्थिर काष्ठा box *item)
{
	क्रमागत css_align_self_e align_self = css_computed_align_self(item->style);

	अगर (align_self == CSS_ALIGN_SELF_AUTO) {
		align_self = css_computed_align_वस्तुएँ(flex->style);
	}

	वापस align_self;
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
अटल अंतरभूत व्योम calculate_mbp_width(
		स्थिर css_unit_ctx *unit_len_ctx,
		स्थिर css_computed_style *style,
		अचिन्हित पूर्णांक side,
		bool margin,
		bool border,
		bool padding,
		पूर्णांक *fixed,
		भग्न *frac)
{
	css_fixed मूल्य = 0;
	css_unit unit = CSS_UNIT_PX;

	निश्चित(style);

	/* margin */
	अगर (margin) {
		क्रमागत css_margin_e type;

		type = margin_funcs[side](style, &मूल्य, &unit);
		अगर (type == CSS_MARGIN_SET) {
			अगर (unit == CSS_UNIT_PCT) {
				*frac += FIXTOFLT(FDIV(मूल्य, F_100));
			} अन्यथा {
				*fixed += FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मूल्य, unit));
			}
		}
	}

	/* border */
	अगर (border) {
		अगर (lh__have_border(side, style)) {
			border_width_funcs[side](style, &मूल्य, &unit);

			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					मूल्य, unit));
		}
	}

	/* padding */
	अगर (padding) {
		padding_funcs[side](style, &मूल्य, &unit);
		अगर (unit == CSS_UNIT_PCT) {
			*frac += FIXTOFLT(FDIV(मूल्य, F_100));
		} अन्यथा {
			*fixed += FIXTOINT(css_unit_len2device_px(
					style, unit_len_ctx,
					मूल्य, unit));
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
अटल अंतरभूत व्योम layout_handle_box_sizing(
		स्थिर css_unit_ctx *unit_len_ctx,
		स्थिर काष्ठा box *box,
		पूर्णांक available_width,
		bool setwidth,
		पूर्णांक *dimension)
{
	क्रमागत css_box_sizing_e bs;

	निश्चित(box && box->style);

	bs = css_computed_box_sizing(box->style);

	अगर (bs == CSS_BOX_SIZING_BORDER_BOX) {
		पूर्णांक orig = *dimension;
		पूर्णांक fixed = 0;
		भग्न frac = 0;

		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? बायाँ_2 : शीर्ष_2,
				false, true, true, &fixed, &frac);
		calculate_mbp_width(unit_len_ctx, box->style,
				setwidth ? दायाँ_2 : BOTTOM,
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
अटल अंतरभूत व्योम layout_find_dimensions(
		स्थिर css_unit_ctx *unit_len_ctx,
		पूर्णांक available_width,
		पूर्णांक viewport_height,
		स्थिर काष्ठा box *box,
		स्थिर css_computed_style *style,
		पूर्णांक *width,
		पूर्णांक *height,
		पूर्णांक *max_width,
		पूर्णांक *min_width,
		पूर्णांक *max_height,
		पूर्णांक *min_height,
		पूर्णांक margin[4],
		पूर्णांक padding[4],
		काष्ठा box_border border[4])
{
	काष्ठा box *containing_block = शून्य;
	अचिन्हित पूर्णांक i;

	अगर (width) {
		अगर (css_computed_width_px(style, unit_len_ctx,
				available_width, width) == CSS_WIDTH_SET) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, width);
		} अन्यथा {
			*width = AUTO;
		}
	}

	अगर (height) {
		क्रमागत css_height_e htype;
		css_fixed मूल्य = 0;
		css_unit unit = CSS_UNIT_PX;

		htype = css_computed_height(style, &मूल्य, &unit);

		अगर (htype == CSS_HEIGHT_SET) {
			अगर (unit == CSS_UNIT_PCT) {
				क्रमागत css_height_e cbhtype;

				अगर (css_computed_position(box->style) ==
						CSS_POSITION_ABSOLUTE &&
						box->parent) {
					/* Box is absolutely positioned */
					निश्चित(box->float_container);
					containing_block = box->float_container;
				} अन्यथा अगर (box->float_container &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(box->style) ==
						CSS_FLOAT_बायाँ ||
					 css_computed_float(box->style) ==
						CSS_FLOAT_दायाँ)) {
					/* Box is a float */
					निश्चित(box->parent &&
						box->parent->parent &&
						box->parent->parent->parent);

					containing_block =
						box->parent->parent->parent;
				} अन्यथा अगर (box->parent && box->parent->type !=
						BOX_INLINE_CONTAINER) {
					/* Box is a block level element */
					containing_block = box->parent;
				} अन्यथा अगर (box->parent && box->parent->type ==
						BOX_INLINE_CONTAINER) {
					/* Box is an inline block */
					निश्चित(box->parent->parent);
					containing_block = box->parent->parent;
				}

				अगर (containing_block) {
					css_fixed f = 0;
					css_unit u = CSS_UNIT_PX;

					cbhtype = css_computed_height(
							containing_block->style,
							&f, &u);
				}

				अगर (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						cbhtype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height.
					 * (CSS 2.1 Section 10.5) */
					*height = FPCT_OF_INT_TOINT(मूल्य,
						containing_block->height);
				} अन्यथा अगर ((!box->parent ||
						!box->parent->parent) &&
						viewport_height >= 0) {
					/* If root element or it's child
					 * (HTML or BODY) */
					*height = FPCT_OF_INT_TOINT(मूल्य,
							viewport_height);
				} अन्यथा {
					/* precentage height not permissible
					 * treat height as auto */
					*height = AUTO;
				}
			} अन्यथा {
				*height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मूल्य, unit));
			}
		} अन्यथा {
			*height = AUTO;
		}

		अगर (*height != AUTO) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, false, height);
		}
	}

	अगर (max_width) {
		क्रमागत css_max_width_e type;
		css_fixed मूल्य = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_width(style, &मूल्य, &unit);

		अगर (type == CSS_MAX_WIDTH_SET) {
			अगर (unit == CSS_UNIT_PCT) {
				*max_width = FPCT_OF_INT_TOINT(मूल्य,
						available_width);
			} अन्यथा {
				*max_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मूल्य, unit));
			}
		} अन्यथा {
			/* Inadmissible */
			*max_width = -1;
		}

		अगर (*max_width != -1) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, max_width);
		}
	}

	अगर (min_width) {
		क्रमागत css_min_width_e type;
		css_fixed मूल्य = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_width(style, &मूल्य, &unit);

		अगर (type == CSS_MIN_WIDTH_SET) {
			अगर (unit == CSS_UNIT_PCT) {
				*min_width = FPCT_OF_INT_TOINT(मूल्य,
						available_width);
			} अन्यथा {
				*min_width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मूल्य, unit));
			}
		} अन्यथा {
			/* Inadmissible */
			*min_width = 0;
		}

		अगर (*min_width != 0) {
			layout_handle_box_sizing(unit_len_ctx, box,
					available_width, true, min_width);
		}
	}

	अगर (max_height) {
		क्रमागत css_max_height_e type;
		css_fixed मूल्य = 0;
		css_unit unit = CSS_UNIT_PX;

		type = css_computed_max_height(style, &मूल्य, &unit);

		अगर (type == CSS_MAX_HEIGHT_SET) {
			अगर (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*max_height = -1;
			} अन्यथा {
				*max_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मूल्य, unit));
			}
		} अन्यथा {
			/* Inadmissible */
			*max_height = -1;
		}
	}

	अगर (min_height) {
		क्रमागत css_min_height_e type;
		css_fixed मूल्य = 0;
		css_unit unit = CSS_UNIT_PX;

		type = ns_computed_min_height(style, &मूल्य, &unit);

		अगर (type == CSS_MIN_HEIGHT_SET) {
			अगर (unit == CSS_UNIT_PCT) {
				/* TODO: handle percentage */
				*min_height = 0;
			} अन्यथा {
				*min_height = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मूल्य, unit));
			}
		} अन्यथा {
			/* Inadmissible */
			*min_height = 0;
		}
	}

	क्रम (i = 0; i != 4; i++) {
		अगर (margin) {
			क्रमागत css_margin_e type = CSS_MARGIN_AUTO;
			css_fixed मूल्य = 0;
			css_unit unit = CSS_UNIT_PX;

			type = margin_funcs[i](style, &मूल्य, &unit);

			अगर (type == CSS_MARGIN_SET) {
				अगर (unit == CSS_UNIT_PCT) {
					margin[i] = FPCT_OF_INT_TOINT(मूल्य,
							available_width);
				} अन्यथा {
					margin[i] = FIXTOINT(css_unit_len2device_px(
							style, unit_len_ctx,
							मूल्य, unit));
				}
			} अन्यथा {
				margin[i] = AUTO;
			}
		}

		अगर (padding) {
			css_fixed मूल्य = 0;
			css_unit unit = CSS_UNIT_PX;

			padding_funcs[i](style, &मूल्य, &unit);

			अगर (unit == CSS_UNIT_PCT) {
				padding[i] = FPCT_OF_INT_TOINT(मूल्य,
						available_width);
			} अन्यथा {
				padding[i] = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मूल्य, unit));
			}
		}

		/* Table cell borders are populated in table.c */
		अगर (border && box->type != BOX_TABLE_CELL) {
			क्रमागत css_border_style_e bstyle = CSS_BORDER_STYLE_NONE;
			css_color color = 0;
			css_fixed मूल्य = 0;
			css_unit unit = CSS_UNIT_PX;

			border_width_funcs[i](style, &मूल्य, &unit);
			bstyle = border_style_funcs[i](style);
			border_color_funcs[i](style, &color);

			border[i].style = bstyle;
			border[i].c = color;

			अगर (bstyle == CSS_BORDER_STYLE_HIDDEN ||
					bstyle == CSS_BORDER_STYLE_NONE)
				/* spec unclear: following Mozilla */
				border[i].width = 0;
			अन्यथा
				border[i].width = FIXTOINT(css_unit_len2device_px(
						style, unit_len_ctx,
						मूल्य, unit));

			/* Special case for border-collapse: make all borders
			 * on table/table-row-group/table-row zero width. */
			अगर (css_computed_border_collapse(style) ==
					CSS_BORDER_COLLAPSE_COLLAPSE &&
					(box->type == BOX_TABLE ||
					 box->type == BOX_TABLE_ROW_GROUP ||
					 box->type == BOX_TABLE_ROW))
				border[i].width = 0;
		}
	}
}

#पूर्ण_अगर
