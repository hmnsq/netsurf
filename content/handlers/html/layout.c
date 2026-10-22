/*
 * Copyright 2005 Richard Wilson <info@tinct.net>
 * Copyright 2006 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2008 Michael Drake <tlsa@netsurf-browser.org>
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
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
 * HTML layout implementation.
 *
 * Layout is carried out in two stages:
 *
 * 1. + calculation of minimum / maximum box widths, and
 *    + determination of whether block level boxes will have >zero height
 *
 * 2. + layout (position and dimensions)
 *
 * In most cases the functions for the two stages are a corresponding pair
 * layout_minmax_X() and layout_X().
 */

#समावेश <assert.h>
#समावेश <limits.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <math.h>
#समावेश <dom/dom.h>

#समावेश "utils/log.h"
#समावेश "utils/talloc.h"
#समावेश "utils/utils.h"
#समावेश "utils/nsoption.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/nsurl.h"
#समावेश "netsurf/inttypes.h"
#समावेश "netsurf/content.h"
#समावेश "netsurf/browser_window.h"
#समावेश "netsurf/layout.h"
#समावेश "content/content.h"
#समावेश "content/content_protected.h"
#समावेश "css/utils.h"
#समावेश "desktop/scrollbar.h"
#समावेश "desktop/textarea.h"

#समावेश "html/html.h"
#समावेश "html/html_save.h"
#समावेश "html/private.h"
#समावेश "html/box.h"
#समावेश "html/box_inspect.h"
#समावेश "html/font.h"
#समावेश "html/form_internal.h"
#समावेश "html/layout.h"
#समावेश "html/layout_internal.h"
#समावेश "html/table.h"

/** Array of per-side access functions for computed style margins. */
स्थिर css_len_func margin_funcs[4] = {
	[शीर्ष_2]    = css_computed_margin_शीर्ष,
	[दायाँ_2]  = css_computed_margin_दायाँ,
	[BOTTOM] = css_computed_margin_bottom,
	[बायाँ_2]   = css_computed_margin_बायाँ,
};

/** Array of per-side access functions for computed style paddings. */
स्थिर css_len_func padding_funcs[4] = {
	[शीर्ष_2]    = css_computed_padding_शीर्ष,
	[दायाँ_2]  = css_computed_padding_दायाँ,
	[BOTTOM] = css_computed_padding_bottom,
	[बायाँ_2]   = css_computed_padding_बायाँ,
};

/** Array of per-side access functions for computed style border_widths. */
स्थिर css_len_func border_width_funcs[4] = {
	[शीर्ष_2]    = css_computed_border_शीर्ष_width,
	[दायाँ_2]  = css_computed_border_दायाँ_width,
	[BOTTOM] = css_computed_border_bottom_width,
	[बायाँ_2]   = css_computed_border_बायाँ_width,
};

/** Array of per-side access functions for computed style border styles. */
स्थिर css_border_style_func border_style_funcs[4] = {
	[शीर्ष_2]    = css_computed_border_शीर्ष_style,
	[दायाँ_2]  = css_computed_border_दायाँ_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[बायाँ_2]   = css_computed_border_बायाँ_style,
};

/** Array of per-side access functions for computed style border colors. */
स्थिर css_border_color_func border_color_funcs[4] = {
	[शीर्ष_2]    = css_computed_border_शीर्ष_color,
	[दायाँ_2]  = css_computed_border_दायाँ_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[बायाँ_2]   = css_computed_border_बायाँ_color,
};

/* forward declaration to break cycles */
अटल व्योम layout_minmax_block(
		काष्ठा box *block,
		स्थिर काष्ठा gui_layout_table *font_func,
		स्थिर html_content *content);

/**
 * Compute the size of replaced boxes with auto dimensions, according to
 * content.
 *
 * \param  box     Box with object
 * \param  width   Width value in px or AUTO.  If AUTO, updated to value in px.
 * \param  height  Height value in px or AUTO. If AUTO, updated to value in px.
 * \param  min_width  Box's min width, as given by layout_find_dimensions.
 * \param  max_width  Box's max width, as given by layout_find_dimensions.
 * \param  min_height  Box's min height, as given by layout_find_dimensions.
 * \param  max_height  Box's max height, as given by layout_find_dimensions.
 *
 * See CSS 2.1 sections 10.3 and 10.6.
 */
अटल व्योम
layout_get_object_dimensions(काष्ठा box *box,
			     पूर्णांक *width, पूर्णांक *height,
			     पूर्णांक min_width, पूर्णांक max_width,
			     पूर्णांक min_height, पूर्णांक max_height)
{
	निश्चित(box->object != शून्य);
	निश्चित(width != शून्य && height != शून्य);

	अगर (*width == AUTO && *height == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		पूर्णांक intrinsic_width = content_get_width(box->object);
		पूर्णांक intrinsic_height = content_get_height(box->object);

		/* use intrinsic dimensions */
		*width = intrinsic_width;
		*height = intrinsic_height;

		/* Deal with min/max-width first */
		अगर (min_width >  0 && min_width > *width) {
			*width = min_width;
			scaled = true;
		}
		अगर (max_width >= 0 && max_width < *width) {
			*width = max_width;
			scaled = true;
		}

		अगर (scaled && (intrinsic_width != 0)) {
			/* Update height */
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		}

		scaled = false;
		/* Deal with min/max-height */
		अगर (min_height >  0 && min_height > *height) {
			*height = min_height;
			scaled = true;
		}
		अगर (max_height >= 0 && max_height < *height) {
			*height = max_height;
			scaled = true;
		}

		अगर (scaled && (intrinsic_height != 0)) {
			/* Update width */
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		}

	} अन्यथा अगर (*width == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		पूर्णांक intrinsic_width = content_get_width(box->object);
		पूर्णांक intrinsic_height = content_get_height(box->object);

		अगर (intrinsic_height != 0)
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		अन्यथा
			*width = intrinsic_width;

		अगर (min_width >  0 && min_width > *width)
			*width = min_width;
		अगर (max_width >= 0 && max_width < *width)
			*width = max_width;

	} अन्यथा अगर (*height == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		पूर्णांक intrinsic_width = content_get_width(box->object);
		पूर्णांक intrinsic_height = content_get_height(box->object);

		अगर (min_width >  0 && min_width > *width)
			*width = min_width;
		अगर (max_width >= 0 && max_width < *width)
			*width = max_width;

		अगर (intrinsic_width != 0)
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		अन्यथा
			*height = intrinsic_height;
	}
}


/**
 * Calculate the text-indent length.
 *
 * \param  style  style of block
 * \param  width  width of containing block
 * \return  length of indent
 */
अटल पूर्णांक layout_text_indent(
		स्थिर css_unit_ctx *unit_len_ctx,
		स्थिर css_computed_style *style, पूर्णांक width)
{
	css_fixed मूल्य = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_text_indent(style, &मूल्य, &unit);

	अगर (unit == CSS_UNIT_PCT) {
		वापस FPCT_OF_INT_TOINT(मूल्य, width);
	} अन्यथा {
		वापस FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				मूल्य, unit));
	}
}


/**
 * Calculate minimum and maximum width of a table.
 *
 * \param table box of type TABLE
 * \param font_func Font functions
 * \param content  The HTML content we are laying out.
 * \post  table->min_width and table->max_width filled in,
 *        0 <= table->min_width <= table->max_width
 */
अटल व्योम layout_minmax_table(काष्ठा box *table,
		स्थिर काष्ठा gui_layout_table *font_func,
		स्थिर html_content *content)
{
	अचिन्हित पूर्णांक i, j;
	पूर्णांक width;
	पूर्णांक border_spacing_h = 0;
	पूर्णांक table_min = 0, table_max = 0;
	पूर्णांक extra_fixed = 0;
	भग्न extra_frac = 0;
	काष्ठा column *col;
	काष्ठा box *row_group, *row, *cell;

	/* check if the widths have already been calculated */
	अगर (table->max_width != UNKNOWN_MAX_WIDTH)
		वापस;

	अगर (table_calculate_column_types(&content->unit_len_ctx, table) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		वापस;
	}
	col = table->col;

	/* start with 0 except for fixed-width columns */
	क्रम (i = 0; i != table->columns; i++) {
		अगर (col[i].type == COLUMN_WIDTH_FIXED)
			col[i].min = col[i].max = col[i].width;
		अन्यथा
			col[i].min = col[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	अगर (css_computed_border_collapse(table->style) ==
			CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		css_computed_border_spacing(table->style, &h, &hu, &v, &vu);

		border_spacing_h = FIXTOINT(css_unit_len2device_px(
				table->style,
				&content->unit_len_ctx,
				h, hu));
	}

	/* 1st pass: consider cells with colspan 1 only */
	क्रम (row_group = table->children; row_group; row_group =row_group->अग्र)
	क्रम (row = row_group->children; row; row = row->अग्र)
	क्रम (cell = row->children; cell; cell = cell->अग्र) {
		निश्चित(cell->type == BOX_TABLE_CELL);
		निश्चित(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		निश्चित(cell->columns != 0);

		अगर (cell->columns != 1)
			जारी;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		अगर (col[i].positioned)
			जारी;

		/* update column min, max widths using cell widths */
		अगर (col[i].min < cell->min_width)
			col[i].min = cell->min_width;
		अगर (col[i].max < cell->max_width)
			col[i].max = cell->max_width;
	}

	/* 2nd pass: cells which span multiple columns */
	क्रम (row_group = table->children; row_group; row_group =row_group->अग्र)
	क्रम (row = row_group->children; row; row = row->अग्र)
	क्रम (cell = row->children; cell; cell = cell->अग्र) {
		अचिन्हित पूर्णांक flexible_columns = 0;
		पूर्णांक min = 0, max = 0, fixed_width = 0, extra;

		अगर (cell->columns == 1)
			जारी;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		क्रम (j = 0; j != cell->columns; j++) {
			min += col[i + j].min;
			अगर (col[i + j].type == COLUMN_WIDTH_FIXED)
				fixed_width += col[i + j].width;
			अन्यथा
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		अगर (min < cell->min_width) {
			अगर (flexible_columns == 0) {
				extra = 1 + (cell->min_width - min) /
						cell->columns;
				क्रम (j = 0; j != cell->columns; j++) {
					col[i + j].min += extra;
					अगर (col[i + j].max < col[i + j].min)
						col[i + j].max = col[i + j].min;
				}
			} अन्यथा {
				extra = 1 + (cell->min_width - min) /
						flexible_columns;
				क्रम (j = 0; j != cell->columns; j++) {
					अगर (col[i + j].type !=
							COLUMN_WIDTH_FIXED) {
						col[i + j].min += extra;
						अगर (col[i + j].max <
								col[i + j].min)
							col[i + j].max =
								col[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		क्रम (j = 0; j != cell->columns; j++)
			max += col[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		अगर (max < cell->max_width && flexible_columns) {
			extra = 1 + (cell->max_width - max) / flexible_columns;
			क्रम (j = 0; j != cell->columns; j++)
				अगर (col[i + j].type != COLUMN_WIDTH_FIXED)
					col[i + j].max += extra;
		}
	}

	क्रम (i = 0; i != table->columns; i++) {
		अगर (col[i].max < col[i].min) {
			box_dump(मानक_त्रुटि, table, 0, true);
			निश्चित(0);
		}
		table_min += col[i].min;
		table_max += col[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	अगर (css_computed_width_px(table->style, &content->unit_len_ctx,
			-1, &width) == CSS_WIDTH_SET) {
		अगर (table_min < width)
			table_min = width;
		अगर (table_max < width)
			table_max = width;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, बायाँ_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, दायाँ_2, true, true, true,
			&extra_fixed, &extra_frac);
	अगर (extra_fixed < 0)
		extra_fixed = 0;
	अगर (extra_frac < 0)
		extra_frac = 0;
	अगर (1.0 <= extra_frac)
		extra_frac = 0.9;
	table->min_width = (table_min + extra_fixed) / (1.0 - extra_frac);
	table->max_width = (table_max + extra_fixed) / (1.0 - extra_frac);
	table->min_width += (table->columns + 1) * border_spacing_h;
	table->max_width += (table->columns + 1) * border_spacing_h;

	निश्चित(0 <= table->min_width && table->min_width <= table->max_width);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
अटल अंतरभूत bool box_has_percentage_max_width(काष्ठा box *b)
{
	css_unit unit = CSS_UNIT_PX;
	क्रमागत css_max_width_e type;
	css_fixed मूल्य = 0;

	निश्चित(b != शून्य);

	type = css_computed_max_width(b->style, &मूल्य, &unit);
	वापस ((type == CSS_MAX_WIDTH_SET) && (unit == CSS_UNIT_PCT));
}

/**
 * Calculate minimum and maximum width of a line.
 *
 * \param first       a box in an inline container
 * \param line_min    updated to minimum width of line starting at first
 * \param line_max    updated to maximum width of line starting at first
 * \param first_line  true iff this is the first line in the inline container
 * \param line_has_height  updated to true or false, depending on line
 * \param font_func Font functions.
 * \return  first box in next line, or 0 if no more lines
 * \post  0 <= *line_min <= *line_max
 */
अटल काष्ठा box *
layout_minmax_पंक्ति(काष्ठा box *first,
		   पूर्णांक *पंक्ति_min,
		   पूर्णांक *पंक्ति_max,
		   bool first_पंक्ति,
		   bool *पंक्ति_has_height,
		   स्थिर काष्ठा gui_layout_table *font_func,
		   स्थिर html_content *content)
{
	पूर्णांक min = 0, max = 0, width, height, fixed;
	भग्न frac;
	माप_प्रकार i, j;
	काष्ठा box *b;
	काष्ठा box *block;
	plot_font_style_t fstyle;
	bool no_wrap;

	निश्चित(first->parent);
	निश्चित(first->parent->parent);
	निश्चित(first->parent->parent->style);

	block = first->parent->parent;
	no_wrap = (css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_PRE);

	*पंक्ति_has_height = false;

	/* corresponds to the pass 1 loop in layout_line() */
	क्रम (b = first; b; b = b->अग्र) {
		क्रमागत css_width_e wtype;
		क्रमागत css_height_e htype;
		क्रमागत css_box_sizing_e bs;
		css_fixed मूल्य = 0;
		css_unit unit = CSS_UNIT_PX;

		निश्चित(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		अगर (b->type == BOX_BR) {
			b = b->अग्र;
			अवरोध;
		}

		अगर (lh__box_is_float_box(b)) {
			निश्चित(b->children);
			अगर (b->children->type == BOX_TABLE)
				layout_minmax_table(b->children, font_func,
						content);
			अन्यथा
				layout_minmax_block(b->children, font_func,
						content);
			b->min_width = b->children->min_width;
			b->max_width = b->children->max_width;
			अगर (min < b->min_width)
				min = b->min_width;
			max += b->max_width;
			जारी;
		}

		अगर (b->type == BOX_INLINE_BLOCK || b->type == BOX_INLINE_FLEX) {
			layout_minmax_block(b, font_func, content);
			अगर (min < b->min_width)
				min = b->min_width;
			max += b->max_width;

			अगर (b->flags & HAS_HEIGHT)
				*पंक्ति_has_height = true;
			जारी;
		}

		निश्चित(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		अगर (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->style, बायाँ_2, true, true, true,
					&fixed, &frac);
			अगर (!b->inline_end)
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, दायाँ_2,
						true, true, true,
						&fixed, &frac);
			अगर (0 < fixed)
				max += fixed;
			*पंक्ति_has_height = true;
			/* \todo  update min width, consider fractional extra */
		} अन्यथा अगर (b->type == BOX_INLINE_END) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->inline_end->style, दायाँ_2,
					true, true, true,
					&fixed, &frac);
			अगर (0 < fixed)
				max += fixed;

			अगर (b->अग्र) {
				अगर (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*पंक्ति_has_height = true;
			जारी;
		}

		अगर (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			अगर (!b->text)
				जारी;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			अगर (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				अगर (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					पूर्णांक opt_maxwidth = 0;
					काष्ठा form_option *o;

					क्रम (o = b->parent->parent->gadget->
							तथ्य.select.वस्तुएँ; o;
							o = o->अग्र) {
						पूर्णांक opt_width;
						font_func->width(&fstyle,
								o->text,
								म_माप(o->text),
								&opt_width);

						अगर (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}

					b->width = opt_maxwidth;
					अगर (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;

				} अन्यथा {
					font_func->width(&fstyle, b->text,
						b->लंबाई, &b->width);
					b->flags |= MEASURED;
				}
			}
			max += b->width;
			अगर (b->अग्र) {
				अगर (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			अगर (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} अन्यथा अगर (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				अगर (min < b->width)
					min = b->width;

			} अन्यथा अगर (b->parent->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				करो {
					क्रम (j = i; j != b->लंबाई &&
							b->text[j] != ' '; j++)
						;
					font_func->width(&fstyle, b->text + i,
							 j - i, &width);
					अगर (min < width)
						min = width;
					i = j + 1;
				} जबतक (j != b->लंबाई);
			}

			*पंक्ति_has_height = true;

			जारी;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		निश्चित(b->style);

		bs = css_computed_box_sizing(block->style);

		/* calculate box width */
		wtype = css_computed_width_px(b->style,
				&content->unit_len_ctx, -1, &width);
		अगर (wtype == CSS_WIDTH_SET) {
			अगर (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, बायाँ_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, दायाँ_2,
						false, true, true,
						&fixed, &frac);
				अगर (width < fixed) {
					width = fixed;
				}
			}
			अगर (width < 0)
				width = 0;
		} अन्यथा {
			width = AUTO;
		}

		/* height */
		htype = css_computed_height(b->style, &मूल्य, &unit);
		अगर (htype == CSS_HEIGHT_SET) {
			height = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					मूल्य, unit));
		} अन्यथा {
			height = AUTO;
		}

		अगर (b->object || (b->flags & REPLACE_DIM)) {
			अगर (b->object) {
				पूर्णांक अस्थायी_height = height;
				layout_get_object_dimensions(b,
						&width, &अस्थायी_height,
						पूर्णांक_न्यून, पूर्णांक_उच्च,
						पूर्णांक_न्यून, पूर्णांक_उच्च);
			}

			fixed = frac = 0;
			अगर (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, बायाँ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, दायाँ_2,
						true, false, false,
						&fixed, &frac);
			} अन्यथा {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, बायाँ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, दायाँ_2,
						true, true, true,
						&fixed, &frac);
			}
			अगर (0 < width + fixed)
				width += fixed;
		} अन्यथा अगर (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			अगर (width == AUTO)
				width = 400;

			fixed = frac = 0;
			अगर (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, बायाँ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, दायाँ_2,
						true, false, false,
						&fixed, &frac);
			} अन्यथा {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, बायाँ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, दायाँ_2,
						true, true, true,
						&fixed, &frac);
			}

			अगर (0 < width + fixed)
				width += fixed;

		} अन्यथा {
			/* form control with no object */
			अगर (width == AUTO)
				width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		अगर (min < width && !box_has_percentage_max_width(b))
			min = width;
		अगर (width > 0)
			max += width;

		*पंक्ति_has_height = true;
	}

	अगर (first_पंक्ति) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		पूर्णांक text_indent = layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, 100);
		min = (min + text_indent < 0) ? 0 : min + text_indent;
		max = (max + text_indent < 0) ? 0 : max + text_indent;
	}

	*पंक्ति_min = min;
	*पंक्ति_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	निश्चित(b != first);
	निश्चित(0 <= *पंक्ति_min);
	निश्चित(*पंक्ति_min <= *पंक्ति_max);
	वापस b;
}


/**
 * Calculate minimum and maximum width of an inline container.
 *
 * \param inline_container  box of type INLINE_CONTAINER
 * \param[out] has_height set to true if container has height
 * \param font_func Font functions.
 * \post  inline_container->min_width and inline_container->max_width filled in,
 *        0 <= inline_container->min_width <= inline_container->max_width
 */
अटल व्योम
layout_minmax_inline_container(काष्ठा box *inline_container,
			       bool *has_height,
			       स्थिर काष्ठा gui_layout_table *font_func,
			       स्थिर html_content *content)
{
	काष्ठा box *child;
	पूर्णांक पंक्ति_min = 0, पंक्ति_max = 0;
	पूर्णांक min = 0, max = 0;
	bool first_पंक्ति = true;
	bool पंक्ति_has_height;

	निश्चित(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	अगर (inline_container->max_width != UNKNOWN_MAX_WIDTH)
		वापस;

	*has_height = false;

	क्रम (child = inline_container->children; child; ) {
		child = layout_minmax_पंक्ति(child, &पंक्ति_min, &पंक्ति_max,
				first_पंक्ति, &पंक्ति_has_height, font_func,
				content);
		अगर (min < पंक्ति_min)
			min = पंक्ति_min;
		अगर (max < पंक्ति_max)
			max = पंक्ति_max;
		first_पंक्ति = false;
		*has_height |= पंक्ति_has_height;
	}

	inline_container->min_width = min;
	inline_container->max_width = max;

	निश्चित(0 <= inline_container->min_width &&
			inline_container->min_width <=
			inline_container->max_width);
}

/**
 * Calculate minimum and maximum width of a block.
 *
 * \param block  box of type BLOCK, INLINE_BLOCK, or TABLE_CELL
 * \param font_func font functions
 * \param content The HTML content being layed out.
 * \post  block->min_width and block->max_width filled in,
 *        0 <= block->min_width <= block->max_width
 */
अटल व्योम layout_minmax_block(
		काष्ठा box *block,
		स्थिर काष्ठा gui_layout_table *font_func,
		स्थिर html_content *content)
{
	काष्ठा box *child;
	पूर्णांक min = 0, max = 0;
	पूर्णांक extra_fixed = 0;
	भग्न extra_frac = 0;
	क्रमागत css_width_e wtype = CSS_WIDTH_AUTO;
	css_fixed width = 0;
	css_unit wunit = CSS_UNIT_PX;
	क्रमागत css_height_e htype = CSS_HEIGHT_AUTO;
	css_fixed height = 0;
	css_unit hunit = CSS_UNIT_PX;
	क्रमागत css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool child_has_height = false;

	निश्चित(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	/* check if the widths have already been calculated */
	अगर (block->max_width != UNKNOWN_MAX_WIDTH)
		वापस;

	अगर (block->style != शून्य) {
		wtype = css_computed_width(block->style, &width, &wunit);
		htype = css_computed_height(block->style, &height, &hunit);
		bs = css_computed_box_sizing(block->style);
	}

	/* set whether the minimum width is of any interest for this box */
	अगर (((block->parent && lh__box_is_float_box(block->parent)) ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX) &&
			wtype != CSS_WIDTH_SET) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} अन्यथा अगर (block->type == BOX_TABLE_CELL) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} अन्यथा अगर (block->parent && (block->parent->flags & NEED_MIN) &&
			wtype != CSS_WIDTH_SET) {
		/* box inside shrink-to-fit context; need minimum width */
		block->flags |= NEED_MIN;
	} अन्यथा अगर (block->parent && (block->parent->type == BOX_FLEX)) {
		/* box is flex item */
		block->flags |= NEED_MIN;
	}

	अगर (block->gadget && (block->gadget->type == GADGET_TEXTBOX ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_FILE ||
			block->gadget->type == GADGET_TEXTAREA) &&
			block->style && wtype == CSS_WIDTH_AUTO) {
		css_fixed आकार = INTTOFIX(10);
		css_unit unit = CSS_UNIT_EM;

		min = max = FIXTOINT(css_unit_len2device_px(block->style,
				&content->unit_len_ctx, आकार, unit));

		block->flags |= HAS_HEIGHT;
	}

	अगर (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX) &&
			block->style && wtype == CSS_WIDTH_AUTO) {
		css_fixed आकार = INTTOFIX(1);
		css_unit unit = CSS_UNIT_EM;

		/* form checkbox or radio button
		 * if width is AUTO, set it to 1em */
		min = max = FIXTOINT(css_unit_len2device_px(block->style,
				&content->unit_len_ctx, आकार, unit));

		block->flags |= HAS_HEIGHT;
	}

	अगर (block->object) {
		अगर (content_get_type(block->object) == CONTENT_HTML) {
			layout_minmax_block(html_get_box_वृक्ष(block->object),
					font_func, content);
			min = html_get_box_वृक्ष(block->object)->min_width;
			max = html_get_box_वृक्ष(block->object)->max_width;
		} अन्यथा {
			min = max = content_get_width(block->object);
		}

		block->flags |= HAS_HEIGHT;
	} अन्यथा अगर (block->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		block->flags |= HAS_HEIGHT;
	} अन्यथा {
		/* recurse through children */
		क्रम (child = block->children; child; child = child->अग्र) {
			चयन (child->type) {
			हाल BOX_FLEX:
			हाल BOX_BLOCK:
				layout_minmax_block(child, font_func,
						content);
				अगर (child->flags & HAS_HEIGHT)
					child_has_height = true;
				अवरोध;
			हाल BOX_INLINE_CONTAINER:
				अगर (block->flags & NEED_MIN)
					child->flags |= NEED_MIN;

				layout_minmax_inline_container(child,
						&child_has_height, font_func,
						content);
				अगर (child_has_height &&
						child ==
						child->parent->children) {
					block->flags |= MAKE_HEIGHT;
				}
				अवरोध;
			हाल BOX_TABLE:
				layout_minmax_table(child, font_func,
						content);
				/* todo: fix for zero height tables */
				child_has_height = true;
				child->flags |= MAKE_HEIGHT;
				अवरोध;
			शेष:
				निश्चित(0);
			}
			निश्चित(child->max_width != UNKNOWN_MAX_WIDTH);

			अगर (child->style &&
					(css_computed_position(child->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(child->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				जारी;
			}

			अगर (lh__box_is_flex_container(block) &&
			    lh__flex_main_is_horizontal(block)) {
				अगर (block->style != शून्य &&
				    css_computed_flex_wrap(block->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += child->min_width;
				} अन्यथा {
					अगर (min < child->min_width)
						min = child->min_width;
				}
				max += child->max_width;

			} अन्यथा {
				अगर (min < child->min_width)
					min = child->min_width;
				अगर (max < child->max_width)
					max = child->max_width;
			}

			अगर (child_has_height)
				block->flags |= HAS_HEIGHT;
		}
	}

	अगर (max < min) {
		box_dump(मानक_त्रुटि, block, 0, true);
		निश्चित(0);
	}

	/* fixed width takes priority */
	अगर (block->type != BOX_TABLE_CELL && !lh__box_is_flex_item(block)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		क्रमागत css_max_width_e max_type;
		क्रमागत css_min_width_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed मूल्य = 0;
		पूर्णांक width;

		अगर (css_computed_width_px(block->style, &content->unit_len_ctx,
				-1, &width) == CSS_WIDTH_SET) {
			min = max = width;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_width(block->style, &मूल्य, &unit);
		अगर (min_type == CSS_MIN_WIDTH_SET && unit != CSS_UNIT_PCT) {
			पूर्णांक val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, मूल्य, unit));

			अगर (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_width(block->style, &मूल्य, &unit);
		अगर (max_type == CSS_MAX_WIDTH_SET && unit != CSS_UNIT_PCT) {
			पूर्णांक val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, मूल्य, unit));

			अगर (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	अगर (htype == CSS_HEIGHT_SET && hunit != CSS_UNIT_PCT &&
			height > INTTOFIX(0)) {
		block->flags |= MAKE_HEIGHT;
		block->flags |= HAS_HEIGHT;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_width(&content->unit_len_ctx, block->style, बायाँ_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, दायाँ_2,
			false, true, true, &extra_fixed, &extra_frac);

	अगर (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	अगर (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	अगर (max < min) {
		min = max;
	}

	calculate_mbp_width(&content->unit_len_ctx, block->style, बायाँ_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, दायाँ_2,
			true, false, false, &extra_fixed, &extra_frac);

	अगर (extra_fixed < 0)
		extra_fixed = 0;
	अगर (extra_frac < 0)
		extra_frac = 0;
	अगर (1.0 <= extra_frac)
		extra_frac = 0.9;
	अगर (block->style != शून्य &&
			(css_computed_float(block->style) == CSS_FLOAT_बायाँ ||
			css_computed_float(block->style) == CSS_FLOAT_दायाँ)) {
		/* floated boxs */
		block->min_width = min + extra_fixed;
		block->max_width = max + extra_fixed;
	} अन्यथा {
		/* not floated */
		block->min_width = (min + extra_fixed) / (1.0 - extra_frac);
		block->max_width = (max + extra_fixed) / (1.0 - extra_frac);
	}

	निश्चित(0 <= block->min_width);
	निश्चित(block->min_width <= block->max_width);
}


/**
 * Find next block that current margin collapses to.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box    box to start tree-order search from (top margin is included)
 * \param  block  box responsible for current block fromatting context
 * \param  viewport_height  height of viewport in px
 * \param  max_pos_margin  updated to to maximum positive margin encountered
 * \param  max_neg_margin  updated to to maximum negative margin encountered
 * \return  next box that current margin collapses to, or NULL if none.
 */
अटल काष्ठा box*
layout_अग्र_margin_block(स्थिर css_unit_ctx *unit_len_ctx,
			 काष्ठा box *box,
			 काष्ठा box *block,
			 पूर्णांक viewport_height,
			 पूर्णांक *max_pos_margin,
			 पूर्णांक *max_neg_margin)
{
	निश्चित(block != शून्य);

	जबतक (box != शून्य) {

		अगर (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			अगर (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						शून्य, शून्य, शून्य, शून्य,
						शून्य, शून्य, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				अगर (*max_pos_margin < box->margin[शीर्ष_2])
					*max_pos_margin = box->margin[शीर्ष_2];
				अन्यथा अगर (*max_neg_margin < -box->margin[शीर्ष_2])
					*max_neg_margin = -box->margin[शीर्ष_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			अगर (box->flags & MAKE_HEIGHT ||
					box->border[शीर्ष_2].width ||
					box->padding[शीर्ष_2] ||
					(box->style &&
					css_computed_overflow_y(box->style) !=
					CSS_OVERFLOW_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_first_child(box))) {
				/* Collapse to this box; return it */
				वापस box;
			}
		}


		/* Find next box */
		अगर (box->type == BOX_BLOCK && !box->object && box->children &&
				box->style &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} अन्यथा {
			अगर (!box->अग्र) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				करो {
					/* Apply bottom margin */
					अगर (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					अन्यथा अगर (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->parent;
				} जबतक (box != block && !box->अग्र);

				अगर (box == block) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					वापस block;
				}
			}

			/* Apply bottom margin */
			अगर (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			अन्यथा अगर (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->अग्र;

			/* Get margins */
			अगर (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						शून्य, शून्य, शून्य, शून्य,
						शून्य, शून्य, box->margin,
						box->padding, box->border);
			}
		}
	}

	वापस शून्य;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
अटल पूर्णांक layout_clear(काष्ठा box *fl, क्रमागत css_clear_e clear)
{
	पूर्णांक y = 0;
	क्रम (; fl; fl = fl->अग्र_float) {
		अगर ((clear == CSS_CLEAR_बायाँ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_बायाँ)
			अगर (y < fl->y + fl->height)
				y = fl->y + fl->height;
		अगर ((clear == CSS_CLEAR_दायाँ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_दायाँ)
			अगर (y < fl->y + fl->height)
				y = fl->y + fl->height;
	}
	वापस y;
}


/**
 * Find left and right edges in a vertical range.
 *
 * \param  fl	  first float in float list
 * \param  y0	  start of y range to search
 * \param  y1	  end of y range to search
 * \param  x0	  start left edge, updated to available left edge
 * \param  x1	  start right edge, updated to available right edge
 * \param  left	  returns float on left if present
 * \param  right  returns float on right if present
 */
अटल व्योम
find_sides(काष्ठा box *fl,
	   पूर्णांक y0, पूर्णांक y1,
	   पूर्णांक *x0, पूर्णांक *x1,
	   काष्ठा box **बायाँ,
	   काष्ठा box **दायाँ)
{
	पूर्णांक fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*बायाँ = *दायाँ = 0;
	क्रम (; fl; fl = fl->अग्र_float) {
		fy1 = fl->y + fl->height;
		अगर (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			वापस;
		}
		fy0 = fl->y;
		अगर (y0 < fy1 && fy0 <= y1) {
			अगर (fl->type == BOX_FLOAT_बायाँ) {
				fx1 = fl->x + fl->width;
				अगर (*x0 < fx1) {
					*x0 = fx1;
					*बायाँ = fl;
				}
			} अन्यथा {
				fx0 = fl->x;
				अगर (fx0 < *x1) {
					*x1 = fx0;
					*दायाँ = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *बायाँ, *दायाँ);
}




/**
 * Solve the width constraint as given in CSS 2.1 section 10.3.3.
 *
 * \param  box              Box to solve constraint for
 * \param  available_width  Max width available in pixels
 * \param  width	    Current box width
 * \param  lm		    Min left margin required to avoid floats in px.
 *				zero if not applicable
 * \param  rm		    Min right margin required to avoid floats in px.
 *				zero if not applicable
 * \param  max_width	    Box max-width ( -ve means no max-width to apply)
 * \param  min_width	    Box min-width ( <=0 means no min-width to apply)
 * \return		    New box width
 *
 * \post \a box's left/right margins will be updated.
 */
अटल पूर्णांक
layout_solve_width(काष्ठा box *box,
		   पूर्णांक available_width,
		   पूर्णांक width,
		   पूर्णांक lm,
		   पूर्णांक rm,
		   पूर्णांक max_width,
		   पूर्णांक min_width)
{
	bool auto_width = false;

	/* Increase specified left/right margins */
	अगर (box->margin[बायाँ_2] != AUTO && box->margin[बायाँ_2] < lm &&
			box->margin[बायाँ_2] >= 0)
		box->margin[बायाँ_2] = lm;
	अगर (box->margin[दायाँ_2] != AUTO && box->margin[दायाँ_2] < rm &&
			box->margin[दायाँ_2] >= 0)
		box->margin[दायाँ_2] = rm;

	/* Find width */
	अगर (width == AUTO) {
		पूर्णांक margin_बायाँ = box->margin[बायाँ_2];
		पूर्णांक margin_दायाँ = box->margin[दायाँ_2];

		अगर (margin_बायाँ == AUTO) {
			margin_बायाँ = lm;
		}
		अगर (margin_दायाँ == AUTO) {
			margin_दायाँ = rm;
		}

		width = available_width -
				(margin_बायाँ + box->border[बायाँ_2].width +
				box->padding[बायाँ_2] + box->padding[दायाँ_2] +
				box->border[दायाँ_2].width + margin_दायाँ);
		width = width < 0 ? 0 : width;
		auto_width = true;
	}

	अगर (max_width >= 0 && width > max_width) {
		/* max-width is admissable and width exceeds max-width */
		width = max_width;
		auto_width = false;
	}

	अगर (min_width > 0 && width < min_width) {
		/* min-width is admissable and width is less than max-width */
		width = min_width;
		auto_width = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	अगर (auto_width) {
		/* any other 'auto' become 0 or the minimum required values */
		अगर (box->margin[बायाँ_2] == AUTO) {
			box->margin[बायाँ_2] = lm;
		}
		अगर (box->margin[दायाँ_2] == AUTO) {
			box->margin[दायाँ_2] = rm;
		}
		वापस width;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	अगर (box->margin[बायाँ_2] != AUTO && box->margin[दायाँ_2] != AUTO &&
			box->parent != शून्य && box->parent->style != शून्य) {
		चयन (css_computed_text_align(box->parent->style)) {
		हाल CSS_TEXT_ALIGN_LIBCSS_दायाँ:
			box->margin[बायाँ_2] = AUTO;
			box->margin[दायाँ_2] = 0;
			अवरोध;
		हाल CSS_TEXT_ALIGN_LIBCSS_CENTER:
			box->margin[बायाँ_2] = box->margin[दायाँ_2] = AUTO;
			अवरोध;
		हाल CSS_TEXT_ALIGN_LIBCSS_बायाँ:
			box->margin[बायाँ_2] = 0;
			box->margin[दायाँ_2] = AUTO;
			अवरोध;
		शेष:
			/* Leave it alone; no HTML alignment */
			अवरोध;
		}
	}

	अगर (box->margin[बायाँ_2] == AUTO && box->margin[दायाँ_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[बायाँ_2] = box->margin[दायाँ_2] =
				(available_width - lm - rm -
				(box->border[बायाँ_2].width + box->padding[बायाँ_2] +
				width + box->padding[दायाँ_2] +
				box->border[दायाँ_2].width)) / 2;

		अगर (box->margin[बायाँ_2] < 0) {
			box->margin[दायाँ_2] += box->margin[बायाँ_2];
			box->margin[बायाँ_2] = 0;
		}

		box->margin[बायाँ_2] += lm;

	} अन्यथा अगर (box->margin[बायाँ_2] == AUTO) {
		box->margin[बायाँ_2] = available_width - lm -
				(box->border[बायाँ_2].width + box->padding[बायाँ_2] +
				width + box->padding[दायाँ_2] +
				box->border[दायाँ_2].width + box->margin[दायाँ_2]);
		box->margin[बायाँ_2] = box->margin[बायाँ_2] < lm
				? lm : box->margin[बायाँ_2];
	} अन्यथा {
		/* margin-right auto or "over-constrained" */
		box->margin[दायाँ_2] = available_width - rm -
				(box->margin[बायाँ_2] + box->border[बायाँ_2].width +
				 box->padding[बायाँ_2] + width +
				 box->padding[दायाँ_2] +
				 box->border[दायाँ_2].width);
	}

	वापस width;
}


/**
 * Compute dimensions of box, margins, paddings, and borders for a block-level
 * element.
 *
 * \param  unit_len_ctx          Length conversion context
 * \param  available_width  Max width available in pixels
 * \param  viewport_height  Height of viewport in pixels or -ve if unknown
 * \param  lm		    min left margin required to avoid floats in px.
 *				zero if not applicable
 * \param  rm		    min right margin required to avoid floats in px.
 *				zero if not applicable
 * \param  box		    box to find dimensions of. updated with new width,
 *			    height, margins, borders and paddings
 *
 * See CSS 2.1 10.3.3, 10.3.4, 10.6.2, and 10.6.3.
 */
अटल व्योम
layout_block_find_dimensions(स्थिर css_unit_ctx *unit_len_ctx,
			     पूर्णांक available_width,
			     पूर्णांक viewport_height,
			     पूर्णांक lm,
			     पूर्णांक rm,
			     काष्ठा box *box)
{
	पूर्णांक width, max_width, min_width;
	पूर्णांक height, max_height, min_height;
	पूर्णांक *margin = box->margin;
	पूर्णांक *padding = box->padding;
	काष्ठा box_border *border = box->border;
	स्थिर css_computed_style *style = box->style;

	layout_find_dimensions(unit_len_ctx, available_width, viewport_height, box,
			style, &width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	अगर (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	}

	box->width = layout_solve_width(box, available_width, width, lm, rm,
			max_width, min_width);
	box->height = height;

	अगर (margin[शीर्ष_2] == AUTO)
		margin[शीर्ष_2] = 0;
	अगर (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
अटल व्योम layout_block_add_scrollbar(काष्ठा box *box, पूर्णांक which)
{
	क्रमागत css_overflow_e overflow_x, overflow_y;

	निश्चित(box->type == BOX_BLOCK && (which == दायाँ_2 || which == BOTTOM));

	अगर (box->style == शून्य)
		वापस;

	overflow_x = css_computed_overflow_x(box->style);
	overflow_y = css_computed_overflow_y(box->style);

	अगर (which == BOTTOM &&
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		अगर (box->height != AUTO &&
				(overflow_x == CSS_OVERFLOW_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_WIDTH;
		}

	} अन्यथा अगर (which == दायाँ_2 &&
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		क्रमागत css_height_e htype;
		css_fixed height = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_height(box->style, &height, &hunit);

		अगर (which == दायाँ_2 && box->width != AUTO &&
				htype == CSS_HEIGHT_SET &&
				(overflow_y == CSS_OVERFLOW_SCROLL ||
				box_vscrollbar_present(box))) {
			box->width -= SCROLLBAR_WIDTH;
			box->padding[दायाँ_2] += SCROLLBAR_WIDTH;
		}
	}
}


/**
 * Moves the children of a box by a specified amount
 *
 * \param  box  top of tree of boxes
 * \param  x	the amount to move children by horizontally
 * \param  y	the amount to move children by vertically
 */
अटल व्योम layout_move_children(काष्ठा box *box, पूर्णांक x, पूर्णांक y)
{
	निश्चित(box);

	क्रम (box = box->children; box; box = box->अग्र) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_table(
		काष्ठा box *table,
		पूर्णांक available_width,
		html_content *content)
{
	अचिन्हित पूर्णांक columns = table->columns;  /* total columns */
	अचिन्हित पूर्णांक i;
	अचिन्हित पूर्णांक *row_span;
	पूर्णांक *excess_y;
	पूर्णांक table_width, min_width = 0, max_width = 0;
	पूर्णांक required_width = 0;
	पूर्णांक x, remainder = 0, गिनती = 0;
	पूर्णांक table_height = 0;
	पूर्णांक min_height = 0;
	पूर्णांक *xs;  /* array of column x positions */
	पूर्णांक auto_width;
	पूर्णांक spare_width;
	पूर्णांक relative_योग = 0;
	पूर्णांक border_spacing_h = 0, border_spacing_v = 0;
	पूर्णांक spare_height;
	पूर्णांक positioned_columns = 0;
	काष्ठा box *containing_block = शून्य;
	काष्ठा box *c;
	काष्ठा box *row;
	काष्ठा box *row_group;
	काष्ठा box **row_span_cell;
	काष्ठा column *col;
	स्थिर css_computed_style *style = table->style;
	क्रमागत css_height_e htype;
	css_fixed मूल्य = 0;
	css_unit unit = CSS_UNIT_PX;

	निश्चित(table->type == BOX_TABLE);
	निश्चित(style);
	निश्चित(table->children && table->children->children);
	निश्चित(columns);

	/* allocate working buffers */
	col = दो_स्मृति(columns * माप col[0]);
	excess_y = दो_स्मृति(columns * माप excess_y[0]);
	row_span = दो_स्मृति(columns * माप row_span[0]);
	row_span_cell = दो_स्मृति(columns * माप row_span_cell[0]);
	xs = दो_स्मृति((columns + 1) * माप xs[0]);
	अगर (!col || !xs || !row_span || !excess_y || !row_span_cell) {
		मुक्त(col);
		मुक्त(excess_y);
		मुक्त(row_span);
		मुक्त(row_span_cell);
		मुक्त(xs);
		वापस false;
	}

	स_नकल(col, table->col, माप(col[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1, table,
			style, 0, 0, 0, 0, 0, 0, table->margin, table->padding,
			table->border);
	क्रम (row_group = table->children; row_group;
			row_group = row_group->अग्र) {
		क्रम (row = row_group->children; row; row = row->अग्र) {
			क्रम (c = row->children; c; c = c->अग्र) {
				क्रमागत css_overflow_e overflow_x;
				क्रमागत css_overflow_e overflow_y;

				निश्चित(c->style);
				table_used_border_for_cell(
						&content->unit_len_ctx, c);
				layout_find_dimensions(&content->unit_len_ctx,
						available_width, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				overflow_x = css_computed_overflow_x(c->style);
				overflow_y = css_computed_overflow_y(c->style);

				अगर (overflow_x == CSS_OVERFLOW_SCROLL ||
						overflow_x ==
						CSS_OVERFLOW_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_WIDTH;
				}
				अगर (overflow_y == CSS_OVERFLOW_SCROLL ||
						overflow_y ==
						CSS_OVERFLOW_AUTO) {
					c->padding[दायाँ_2] += SCROLLBAR_WIDTH;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	अगर (css_computed_border_collapse(style) ==
			CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		css_computed_border_spacing(style, &h, &hu, &v, &vu);

		border_spacing_h = FIXTOINT(css_unit_len2device_px(
				style, &content->unit_len_ctx, h, hu));
		border_spacing_v = FIXTOINT(css_unit_len2device_px(
				style, &content->unit_len_ctx, v, vu));
	}

	/* find specified table width, or available width if auto-width */
	अगर (css_computed_width_px(style, &content->unit_len_ctx,
			available_width, &table_width) == CSS_WIDTH_SET) {
		/* specified width includes border */
		table_width -= table->border[बायाँ_2].width +
				table->border[दायाँ_2].width;
		table_width = table_width < 0 ? 0 : table_width;

		auto_width = table_width;
	} अन्यथा {
		table_width = AUTO;
		auto_width = available_width -
				((table->margin[बायाँ_2] == AUTO ? 0 :
						table->margin[बायाँ_2]) +
				 table->border[बायाँ_2].width +
				 table->padding[बायाँ_2] +
				 table->padding[दायाँ_2] +
				 table->border[दायाँ_2].width +
				 (table->margin[दायाँ_2] == AUTO ? 0 :
						table->margin[दायाँ_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_height(style, &मूल्य, &unit);
	अगर (htype == CSS_HEIGHT_SET) {
		अगर (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			अगर (css_computed_position(table->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				निश्चित(table->float_container);
				containing_block = table->float_container;
			} अन्यथा अगर (table->float_container &&
					css_computed_position(table->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(table->style) ==
						CSS_FLOAT_बायाँ ||
					css_computed_float(table->style) ==
						CSS_FLOAT_दायाँ)) {
				/* Table is a float */
				निश्चित(table->parent && table->parent->parent &&
						table->parent->parent->parent);
				containing_block =
						table->parent->parent->parent;
			} अन्यथा अगर (table->parent && table->parent->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_block = table->parent;
			} अन्यथा अगर (table->parent && table->parent->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				निश्चित(table->parent->parent);
				containing_block = table->parent->parent;
			}

			अगर (containing_block) {
				css_fixed ignored = 0;

				htype = css_computed_height(
						containing_block->style,
						&ignored, &unit);
			}

			अगर (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(table->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_HEIGHT_SET)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_height = FPCT_OF_INT_TOINT(मूल्य,
						containing_block->height);
			}
		} अन्यथा {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_height = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					मूल्य, unit));
		}
	}

	/* calculate width required by cells */
	क्रम (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      table,
		      i,
		      ((स्थिर अक्षर *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[col[i].type],
		      col[i].width,
		      col[i].min,
		      col[i].max);


		अगर (col[i].positioned) {
			positioned_columns++;
			जारी;
		} अन्यथा अगर (col[i].type == COLUMN_WIDTH_FIXED) {
			अगर (col[i].width < col[i].min)
				col[i].width = col[i].max = col[i].min;
			अन्यथा
				col[i].min = col[i].max = col[i].width;
			required_width += col[i].width;
		} अन्यथा अगर (col[i].type == COLUMN_WIDTH_PERCENT) {
			पूर्णांक width = col[i].width * auto_width / 100;
			required_width += col[i].min < width ? width :
					col[i].min;
		} अन्यथा
			required_width += col[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_width);
	}
	required_width += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", table_width,
	      table->min_width, table->max_width, auto_width, required_width);

	अगर (auto_width < required_width) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		क्रम (i = 0; i != columns; i++) {
			अगर (col[i].type == COLUMN_WIDTH_RELATIVE)
				जारी;
			अगर (col[i].type == COLUMN_WIDTH_PERCENT) {
				col[i].max = auto_width * col[i].width / 100;
				अगर (col[i].max < col[i].min)
					col[i].max = col[i].min;
			}
			min_width += col[i].min;
			max_width += col[i].max;
		}
	} अन्यथा {
		/* take percentages exactly */
		क्रम (i = 0; i != columns; i++) {
			अगर (col[i].type == COLUMN_WIDTH_RELATIVE)
				जारी;
			अगर (col[i].type == COLUMN_WIDTH_PERCENT) {
				पूर्णांक width = auto_width * col[i].width / 100;
				अगर (width < col[i].min)
					width = col[i].min;
				col[i].min = col[i].width = col[i].max = width;
				col[i].type = COLUMN_WIDTH_FIXED;
			}
			min_width += col[i].min;
			max_width += col[i].max;
		}
	}

	/* allocate relative widths */
	spare_width = auto_width;
	क्रम (i = 0; i != columns; i++) {
		अगर (col[i].type == COLUMN_WIDTH_RELATIVE)
			relative_योग += col[i].width;
		अन्यथा अगर (col[i].type == COLUMN_WIDTH_FIXED)
			spare_width -= col[i].width;
		अन्यथा
			spare_width -= col[i].min;
	}
	spare_width -= (columns + 1) * border_spacing_h;
	अगर (relative_योग != 0) {
		अगर (spare_width < 0)
			spare_width = 0;
		क्रम (i = 0; i != columns; i++) {
			अगर (col[i].type == COLUMN_WIDTH_RELATIVE) {
				col[i].min = उच्चमान(col[i].max =
						(भग्न) spare_width
						* (भग्न) col[i].width
						/ relative_योग);
				min_width += col[i].min;
				max_width += col[i].max;
			}
		}
	}
	min_width += (columns + 1) * border_spacing_h;
	max_width += (columns + 1) * border_spacing_h;

	अगर (auto_width <= min_width) {
		/* not enough space: minimise column widths */
		क्रम (i = 0; i < columns; i++) {
			col[i].width = col[i].min;
		}
		table_width = min_width;
	} अन्यथा अगर (max_width <= auto_width) {
		/* more space than maximum width */
		अगर (table_width == AUTO) {
			/* for auto-width tables, make columns max width */
			क्रम (i = 0; i < columns; i++) {
				col[i].width = col[i].max;
			}
			table_width = max_width;
		} अन्यथा {
			/* for fixed-width tables, distribute the extra space
			 * too */
			अचिन्हित पूर्णांक flexible_columns = 0;
			क्रम (i = 0; i != columns; i++)
				अगर (col[i].type != COLUMN_WIDTH_FIXED)
					flexible_columns++;
			अगर (flexible_columns == 0) {
				पूर्णांक extra = (table_width - max_width) / columns;
				remainder = (table_width - max_width) -
						(extra * columns);
				क्रम (i = 0; i != columns; i++) {
					col[i].width = col[i].max + extra;
					गिनती -= remainder;
					अगर (गिनती < 0) {
						col[i].width++;
						गिनती += columns;
					}
				}

			} अन्यथा {
				पूर्णांक extra = (table_width - max_width) /
						flexible_columns;
				remainder = (table_width - max_width) -
						(extra * flexible_columns);
				क्रम (i = 0; i != columns; i++)
					अगर (col[i].type != COLUMN_WIDTH_FIXED) {
						col[i].width = col[i].max +
								extra;
						गिनती -= remainder;
						अगर (गिनती < 0) {
							col[i].width++;
							गिनती += flexible_columns;
						}
					}
			}
		}
	} अन्यथा {
		/* space between min and max: fill it exactly */
		भग्न scale = (भग्न) (auto_width - min_width) /
				(भग्न) (max_width - min_width);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		क्रम (i = 0; i < columns; i++) {
			col[i].width = col[i].min + (पूर्णांक) (0.5 +
					(col[i].max - col[i].min) * scale);
		}
		table_width = auto_width;
	}

	xs[0] = x = border_spacing_h;
	क्रम (i = 0; i != columns; i++) {
		अगर (!col[i].positioned)
			x += col[i].width + border_spacing_h;
		xs[i + 1] = x;
		row_span[i] = 0;
		excess_y[i] = 0;
		row_span_cell[i] = 0;
	}

	/* position cells */
	table_height = border_spacing_v;
	क्रम (row_group = table->children; row_group;
			row_group = row_group->अग्र) {
		पूर्णांक row_group_height = 0;
		क्रम (row = row_group->children; row; row = row->अग्र) {
			पूर्णांक row_height = 0;

			htype = css_computed_height(row->style, &मूल्य, &unit);
			अगर (htype == CSS_HEIGHT_SET && unit != CSS_UNIT_PCT) {
				row_height = FIXTOINT(css_unit_len2device_px(
						row->style,
						&content->unit_len_ctx,
						मूल्य, unit));
			}
			क्रम (c = row->children; c; c = c->अग्र) {
				निश्चित(c->style);
				c->width = xs[c->start_column + c->columns] -
						xs[c->start_column] -
						border_spacing_h -
						c->border[बायाँ_2].width -
						c->padding[बायाँ_2] -
						c->padding[दायाँ_2] -
						c->border[दायाँ_2].width;
				c->float_children = 0;
				c->cached_place_below_level = 0;

				c->height = AUTO;
				अगर (!layout_block_context(c, -1, content)) {
					मुक्त(col);
					मुक्त(excess_y);
					मुक्त(row_span);
					मुक्त(row_span_cell);
					मुक्त(xs);
					वापस false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->height;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_height(c->style,
						&मूल्य, &unit);

				अगर (htype == CSS_HEIGHT_SET &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					पूर्णांक h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							मूल्य, unit));
					अगर (c->height < h)
						c->height = h;
				}
				/* specified row height is treated as a minimum
				 */
				अगर (c->height < row_height)
					c->height = row_height;
				c->x = xs[c->start_column] +
						c->border[बायाँ_2].width;
				c->y = c->border[शीर्ष_2].width;
				क्रम (i = 0; i != c->columns; i++) {
					row_span[c->start_column + i] = c->rows;
					excess_y[c->start_column + i] =
							c->border[शीर्ष_2].width +
							c->padding[शीर्ष_2] +
							c->height +
							c->padding[BOTTOM] +
							c->border[BOTTOM].width;
					row_span_cell[c->start_column + i] = 0;
				}
				row_span_cell[c->start_column] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[शीर्ष_2].width -
						c->padding[शीर्ष_2] -
						c->height -
						c->border[BOTTOM].width;
			}
			क्रम (i = 0; i != columns; i++)
				अगर (row_span[i] != 0)
					row_span[i]--;
				अन्यथा
					row_span_cell[i] = 0;
			अगर (row->अग्र || row_group->अग्र) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				क्रम (i = 0; i != columns; i++)
					अगर (row_span[i] == 0 && row_height <
							excess_y[i])
						row_height = excess_y[i];
			} अन्यथा {
				/* except in the last row */
				क्रम (i = 0; i != columns; i++)
					अगर (row_height < excess_y[i])
						row_height = excess_y[i];
			}
			क्रम (i = 0; i != columns; i++) {
				अगर (row_height < excess_y[i])
					excess_y[i] -= row_height;
				अन्यथा
					excess_y[i] = 0;
				अगर (row_span_cell[i] != 0)
					row_span_cell[i]->padding[BOTTOM] +=
							row_height +
							border_spacing_v;
			}

			row->x = 0;
			row->y = row_group_height;
			row->width = table_width;
			row->height = row_height;
			row_group_height += row_height + border_spacing_v;
		}
		row_group->x = 0;
		row_group->y = table_height;
		row_group->width = table_width;
		row_group->height = row_group_height;
		table_height += row_group_height;
	}
	/* Table height is either the height of the contents, or specified
	 * height if greater */
	table_height = max(table_height, min_height);
	/** \todo distribute spare height over the row groups / rows / cells */

	/* perform vertical alignment */
	क्रम (row_group = table->children; row_group;
			row_group = row_group->अग्र) {
		क्रम (row = row_group->children; row; row = row->अग्र) {
			क्रम (c = row->children; c; c = c->अग्र) {
				क्रमागत css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				spare_height = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->height - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &मूल्य, &unit);

				चयन (vertical_align) {
				हाल CSS_VERTICAL_ALIGN_SUB:
				हाल CSS_VERTICAL_ALIGN_SUPER:
				हाल CSS_VERTICAL_ALIGN_TEXT_शीर्ष:
				हाल CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				हाल CSS_VERTICAL_ALIGN_SET:
				हाल CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				हाल CSS_VERTICAL_ALIGN_शीर्ष:
					अवरोध;
				हाल CSS_VERTICAL_ALIGN_मध्य:
					c->padding[शीर्ष_2] += spare_height / 2;
					c->padding[BOTTOM] -= spare_height / 2;
					layout_move_children(c, 0,
							spare_height / 2);
					अवरोध;
				हाल CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[शीर्ष_2] += spare_height;
					c->padding[BOTTOM] -= spare_height;
					layout_move_children(c, 0,
							spare_height);
					अवरोध;
				हाल CSS_VERTICAL_ALIGN_INHERIT:
					निश्चित(0);
					अवरोध;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	अगर (table->margin[शीर्ष_2] == AUTO)
		table->margin[शीर्ष_2] = 0;
	अगर (table->margin[BOTTOM] == AUTO)
		table->margin[BOTTOM] = 0;

	मुक्त(col);
	मुक्त(excess_y);
	मुक्त(row_span);
	मुक्त(row_span_cell);
	मुक्त(xs);

	table->width = table_width;
	table->height = table_height;

	वापस true;
}


/**
 * Manimpulate box height according to CSS min-height and max-height properties
 *
 * \param  unit_len_ctx      CSS length conversion context for document.
 * \param  box		block to modify with any min-height or max-height
 * \param  container	containing block for absolutely positioned elements, or
 *			NULL for non absolutely positioned elements.
 * \return		whether the height has been changed
 */
अटल bool layout_apply_minmax_height(
		स्थिर css_unit_ctx *unit_len_ctx,
		काष्ठा box *box,
		काष्ठा box *container)
{
	पूर्णांक h;
	काष्ठा box *containing_block = शून्य;
	bool updated = false;

	/* Find containing block for percentage heights */
	अगर (box->style != शून्य && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		निश्चित(container);
		containing_block = container;
	} अन्यथा अगर (box->float_container && box->style != शून्य &&
			(css_computed_float(box->style) == CSS_FLOAT_बायाँ ||
			 css_computed_float(box->style) == CSS_FLOAT_दायाँ)) {
		/* Box is a float */
		निश्चित(box->parent && box->parent->parent &&
				box->parent->parent->parent);
		containing_block = box->parent->parent->parent;
	} अन्यथा अगर (box->parent && box->parent->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_block = box->parent;
	} अन्यथा अगर (box->parent && box->parent->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		निश्चित(box->parent->parent);
		containing_block = box->parent->parent;
	}

	अगर (box->style) {
		क्रमागत css_height_e htype = CSS_HEIGHT_AUTO;
		css_fixed मूल्य = 0;
		css_unit unit = CSS_UNIT_PX;

		अगर (containing_block) {
			htype = css_computed_height(containing_block->style,
					&मूल्य, &unit);
		}

		/* max-height */
		अगर (css_computed_max_height(box->style, &मूल्य, &unit) ==
				CSS_MAX_HEIGHT_SET) {
			अगर (unit == CSS_UNIT_PCT) {
				अगर (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(मूल्य,
						containing_block->height);
					अगर (h < box->height) {
						box->height = h;
						updated = true;
					}
				}
			} अन्यथा {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						मूल्य, unit));
				अगर (h < box->height) {
					box->height = h;
					updated = true;
				}
			}
		}

		/* min-height */
		अगर (ns_computed_min_height(box->style, &मूल्य, &unit) ==
				CSS_MIN_HEIGHT_SET) {
			अगर (unit == CSS_UNIT_PCT) {
				अगर (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(मूल्य,
						containing_block->height);
					अगर (h > box->height) {
						box->height = h;
						updated = true;
					}
				}
			} अन्यथा {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						मूल्य, unit));
				अगर (h > box->height) {
					box->height = h;
					updated = true;
				}
			}
		}
	}
	वापस updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
अटल bool layout_block_object(काष्ठा box *block)
{
	निश्चित(block);
	निश्चित(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_TABLE ||
			block->type == BOX_TABLE_CELL);
	निश्चित(block->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", block,
	      hlcache_handle_get_url(block->object), block->width);

	अगर (content_can_reformat(block->object)) {
		content_reformat(block->object, false, block->width, 1);
	} अन्यथा {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	वापस true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
अटल व्योम add_float_to_container(काष्ठा box *cont, काष्ठा box *b)
{
	काष्ठा box *box = cont->float_children;
	पूर्णांक b_bottom = b->y + b->height;

	निश्चित(b->type == BOX_FLOAT_बायाँ || b->type == BOX_FLOAT_दायाँ);

	अगर (box == शून्य) {
		/* No other float children */
		b->अग्र_float = शून्य;
		cont->float_children = b;
		वापस;
	} अन्यथा अगर (b_bottom >= box->y + box->height) {
		/* Goes at start of list */
		b->अग्र_float = cont->float_children;
		cont->float_children = b;
	} अन्यथा {
		काष्ठा box *पूर्व = शून्य;
		जबतक (box != शून्य && b_bottom < box->y + box->height) {
			पूर्व = box;
			box = box->अग्र_float;
		}
		अगर (पूर्व != शून्य) {
			b->अग्र_float = पूर्व->अग्र_float;
			पूर्व->अग्र_float = b;
		}
	}
}


/**
 * Split a text box.
 *
 * \param  content     memory pool for any new boxes
 * \param  fstyle      style for text in text box
 * \param  split_box   box with text to split
 * \param  new_length  new length for text in split_box, after splitting
 * \param  new_width   new width for text in split_box, after splitting
 * \return  true on success, false on memory exhaustion
 *
 * A new box is created and inserted into the box tree after split_box,
 * containing the text after new_length excluding the initial space character.
 */
अटल bool
layout_text_box_split(html_content *content,
		      plot_font_style_t *fstyle,
		      काष्ठा box *split_box,
		      माप_प्रकार new_लंबाई,
		      पूर्णांक new_width)
{
	पूर्णांक space_width = split_box->space;
	काष्ठा box *c2;
	स्थिर काष्ठा gui_layout_table *font_func = content->font_func;
	bool space = (split_box->text[new_लंबाई] == ' ');
	पूर्णांक used_लंबाई = new_लंबाई + (space ? 1 : 0);

	अगर ((space && space_width == 0) || space_width == UNKNOWN_WIDTH) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->width(fstyle, " ", 1, &space_width);
	}

	अगर (split_box->space == UNKNOWN_WIDTH)
		split_box->space = space_width;
	अगर (!space)
		space_width = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, split_box, माप *c2);
	अगर (!c2)
		वापस false;
	c2->flags |= CLONE;

	/* Set remaining text in c2 */
	c2->text += used_लंबाई;

	/* Set c2 according to the remaining text */
	c2->width -= new_width + space_width;
	c2->flags &= ~MEASURED; /* width has been estimated */
	c2->लंबाई = split_box->लंबाई - used_लंबाई;

	/* Update split_box for its reduced text */
	split_box->width = new_width;
	split_box->flags |= MEASURED;
	split_box->लंबाई = new_लंबाई;
	split_box->space = space_width;

	/* Insert c2 into box list */
	c2->अग्र = split_box->अग्र;
	split_box->अग्र = c2;
	c2->पूर्व = split_box;
	अगर (c2->अग्र)
		c2->अग्र->पूर्व = c2;
	अन्यथा
		c2->parent->last = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      split_box,
	      split_box->लंबाई,
	      (पूर्णांक)split_box->लंबाई,
	      split_box->text);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->लंबाई,
	      (पूर्णांक)c2->लंबाई,
	      c2->text);

	वापस true;
}


/**
 * Compute dimensions of box, margins, paddings, and borders for a floating
 * element using shrink-to-fit. Also used for inline-blocks.
 *
 * \param  unit_len_ctx          CSS length conversion context for document.
 * \param  available_width  Max width available in pixels
 * \param  style	    Box's style
 * \param  box		    Box for which to find dimensions
 *				Box margins, borders, paddings, width and
 *				height are updated.
 */
अटल व्योम
layout_float_find_dimensions(
		स्थिर css_unit_ctx *unit_len_ctx,
		पूर्णांक available_width,
		स्थिर css_computed_style *style,
		काष्ठा box *box)
{
	पूर्णांक width, height, max_width, min_width, max_height, min_height;
	पूर्णांक *margin = box->margin;
	पूर्णांक *padding = box->padding;
	काष्ठा box_border *border = box->border;
	क्रमागत css_overflow_e overflow_x = css_computed_overflow_x(style);
	क्रमागत css_overflow_e overflow_y = css_computed_overflow_y(style);
	पूर्णांक scrollbar_width_x =
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;
	पूर्णांक scrollbar_width_y =
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;

	layout_find_dimensions(unit_len_ctx, available_width, -1, box, style,
			&width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	अगर (margin[बायाँ_2] == AUTO)
		margin[बायाँ_2] = 0;
	अगर (margin[दायाँ_2] == AUTO)
		margin[दायाँ_2] = 0;

	अगर (box->gadget == शून्य) {
		padding[दायाँ_2] += scrollbar_width_y;
		padding[BOTTOM] += scrollbar_width_x;
	}

	अगर (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	} अन्यथा अगर (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed आकार = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		निश्चित(box->style);

		अगर (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			अगर (width == AUTO) {
				आकार = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						आकार, unit));
			}
			अगर (box->gadget->type == GADGET_FILE &&
					height == AUTO) {
				आकार = FLTTOFIX(1.5);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						आकार, unit));
			}
		}
		अगर (box->gadget->type == GADGET_TEXTAREA) {
			अगर (width == AUTO) {
				आकार = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						आकार, unit));
			}
			अगर (height == AUTO) {
				आकार = INTTOFIX(4);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						आकार, unit));
			}
		}
	} अन्यथा अगर (width == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		width = min(max(box->min_width, available_width),
				box->max_width);

		/* width includes margin, borders and padding */
		अगर (width == available_width) {
			width -= box->margin[बायाँ_2] + box->border[बायाँ_2].width +
					box->padding[बायाँ_2] +
					box->padding[दायाँ_2] +
					box->border[दायाँ_2].width +
					box->margin[दायाँ_2];
		} अन्यथा {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			पूर्णांक fixed = 0;
			भग्न frac = 0;
			calculate_mbp_width(unit_len_ctx, box->style, बायाँ_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_width(unit_len_ctx, box->style, दायाँ_2,
					true, true, true, &fixed, &frac);
			अगर (fixed < 0)
				fixed = 0;

			width -= fixed;
		}

		अगर (max_width >= 0 && width > max_width) width = max_width;
		अगर (min_width >  0 && width < min_width) width = min_width;

	} अन्यथा {
		अगर (max_width >= 0 && width > max_width) width = max_width;
		अगर (min_width >  0 && width < min_width) width = min_width;
		width -= scrollbar_width_y;
	}

	box->width = width;
	box->height = height;

	अगर (margin[शीर्ष_2] == AUTO)
		margin[शीर्ष_2] = 0;
	अगर (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Layout the contents of a float or inline block.
 *
 * \param  b	  float or inline block box
 * \param  width  available width
 * \param  content  memory pool for any new boxes
 * \return  true on success, false on memory exhaustion
 */
अटल bool layout_float(काष्ठा box *b, पूर्णांक width, html_content *content)
{
	निश्चित(b->type == BOX_TABLE ||
	       b->type == BOX_BLOCK ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_find_dimensions(&content->unit_len_ctx, width, b->style, b);
	अगर (b->type == BOX_TABLE || b->type == BOX_INLINE_FLEX) {
		अगर (b->type == BOX_TABLE) {
			अगर (!layout_table(b, width, content))
				वापस false;
		} अन्यथा {
			अगर (!layout_flex(b, width, content))
				वापस false;
		}
		अगर (b->margin[बायाँ_2] == AUTO)
			b->margin[बायाँ_2] = 0;
		अगर (b->margin[दायाँ_2] == AUTO)
			b->margin[दायाँ_2] = 0;
		अगर (b->margin[शीर्ष_2] == AUTO)
			b->margin[शीर्ष_2] = 0;
		अगर (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} अन्यथा {
		वापस layout_block_context(b, -1, content);
	}
	वापस true;
}


/**
 * Position a float in the first available space.
 *
 * \param  c	  float box to position
 * \param  width  available width
 * \param  cx	  x coordinate relative to cont to place float right of
 * \param  y	  y coordinate relative to cont to place float below
 * \param  cont	  ancestor box which defines horizontal space, for floats
 */
अटल व्योम
place_float_below(काष्ठा box *c, पूर्णांक width, पूर्णांक cx, पूर्णांक y, काष्ठा box *cont)
{
	पूर्णांक x0, x1, yy;
	काष्ठा box *बायाँ;
	काष्ठा box *दायाँ;

	yy = y > cont->cached_place_below_level ?
			y : cont->cached_place_below_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      width, cx, y, cont);

	करो {
		y = yy;
		x0 = cx;
		x1 = cx + width;
		find_sides(cont->float_children, y, y + c->height, &x0, &x1,
				&बायाँ, &दायाँ);
		अगर (बायाँ != 0 && दायाँ != 0) {
			yy = (बायाँ->y + बायाँ->height <
					दायाँ->y + दायाँ->height ?
					बायाँ->y + बायाँ->height :
					दायाँ->y + दायाँ->height);
		} अन्यथा अगर (बायाँ == 0 && दायाँ != 0) {
			yy = दायाँ->y + दायाँ->height;
		} अन्यथा अगर (बायाँ != 0 && दायाँ == 0) {
			yy = बायाँ->y + बायाँ->height;
		}
	} जबतक ((बायाँ != 0 || दायाँ != 0) && (c->width > x1 - x0));

	अगर (c->type == BOX_FLOAT_बायाँ) {
		c->x = x0;
	} अन्यथा {
		c->x = x1 - c->width;
	}
	c->y = y;
	cont->cached_place_below_level = y;
}


/**
 * Calculate line height from a style.
 */
अटल पूर्णांक पंक्ति_height(
		स्थिर css_unit_ctx *unit_len_ctx,
		स्थिर css_computed_style *style)
{
	क्रमागत css_पंक्ति_height_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed पंक्ति_height;

	निश्चित(style);

	lhtype = css_computed_पंक्ति_height(style, &lhvalue, &lhunit);
	अगर (lhtype == CSS_पंक्ति_HEIGHT_NORMAL) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_पंक्ति_HEIGHT_संख्या;
	}

	अगर (lhtype == CSS_पंक्ति_HEIGHT_संख्या ||
			lhunit == CSS_UNIT_PCT) {
		पंक्ति_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		अगर (lhtype != CSS_पंक्ति_HEIGHT_संख्या)
			पंक्ति_height = FDIV(पंक्ति_height, F_100);
	} अन्यथा {
		निश्चित(lhunit != CSS_UNIT_PCT);

		पंक्ति_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	वापस FIXTOINT(पंक्ति_height);
}


/**
 * Position a line of boxes in inline formatting context.
 *
 * \param  first   box at start of line
 * \param  width   available width on input, updated with actual width on output
 *                 (may be incorrect if the line gets split?)
 * \param  y	   coordinate of top of line, updated on exit to bottom
 * \param  cx	   coordinate of left of line relative to cont
 * \param  cy	   coordinate of top of line relative to cont
 * \param  cont	   ancestor box which defines horizontal space, for floats
 * \param  indent  apply any first-line indent
 * \param  has_text_children  at least one TEXT in the inline_container
 * \param  next_box  updated to first box for next line, or 0 at end
 * \param  content  memory pool for any new boxes
 * \return  true on success, false on memory exhaustion
 */
अटल bool
layout_पंक्ति(काष्ठा box *first,
	    पूर्णांक *width,
	    पूर्णांक *y,
	    पूर्णांक cx,
	    पूर्णांक cy,
	    काष्ठा box *cont,
	    bool indent,
	    bool has_text_children,
	    html_content *content,
	    काष्ठा box **अग्र_box)
{
	पूर्णांक height, used_height;
	पूर्णांक x0 = 0;
	पूर्णांक x1 = *width;
	पूर्णांक x, h, x_previous;
	पूर्णांक fy = cy;
	काष्ठा box *बायाँ;
	काष्ठा box *दायाँ;
	काष्ठा box *b;
	काष्ठा box *split_box = 0;
	काष्ठा box *d;
	काष्ठा box *br_box = 0;
	bool move_y = false;
	bool place_below = false;
	पूर्णांक space_before = 0, space_after = 0;
	अचिन्हित पूर्णांक inline_गिनती = 0;
	अचिन्हित पूर्णांक i;
	स्थिर काष्ठा gui_layout_table *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      first,
	      (पूर्णांक)first->लंबाई,
	      first->text,
	      *width,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	find_sides(cont->float_children, cy, cy, &x0, &x1, &बायाँ, &दायाँ);
	x0 -= cx;
	x1 -= cx;

	अगर (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	अगर (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	अगर (has_text_children || first->parent->parent->gadget)
		used_height = height = पंक्ति_height(&content->unit_len_ctx,
				first->parent->parent->style);
	अन्यथा
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		used_height = height = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	क्रम (x = 0, b = first; x <= x1 - x0 && b != 0; b = b->अग्र) {
		पूर्णांक min_width, max_width, min_height, max_height;

		निश्चित(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		अगर (b->type == BOX_BR)
			अवरोध;

		अगर (lh__box_is_float_box(b))
			जारी;
		अगर (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			जारी;

		निश्चित(b->style != शून्य);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		अगर (b->type == BOX_INLINE_BLOCK ||
		    b->type == BOX_INLINE_FLEX) {
			अगर (b->max_width != UNKNOWN_WIDTH)
				अगर (!layout_float(b, *width, content))
					वापस false;
			h = b->border[शीर्ष_2].width + b->padding[शीर्ष_2] + b->height +
					b->padding[BOTTOM] +
					b->border[BOTTOM].width;
			अगर (height < h)
				height = h;
			x += b->margin[बायाँ_2] + b->border[बायाँ_2].width +
					b->padding[बायाँ_2] + b->width +
					b->padding[दायाँ_2] +
					b->border[दायाँ_2].width +
					b->margin[दायाँ_2];
			space_after = 0;
			जारी;
		}

		अगर (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_find_dimensions(&content->unit_len_ctx,
					*width, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			क्रम (i = 0; i != 4; i++)
				अगर (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[बायाँ_2] + b->border[बायाँ_2].width +
					b->padding[बायाँ_2];
			अगर (b->inline_end) {
				b->inline_end->margin[दायाँ_2] = b->margin[दायाँ_2];
				b->inline_end->padding[दायाँ_2] =
						b->padding[दायाँ_2];
				b->inline_end->border[दायाँ_2] =
						b->border[दायाँ_2];
			} अन्यथा {
				x += b->padding[दायाँ_2] +
						b->border[दायाँ_2].width +
						b->margin[दायाँ_2];
			}
		} अन्यथा अगर (b->type == BOX_INLINE_END) {
			b->width = 0;
			अगर (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[दायाँ_2] + b->border[दायाँ_2].width +
					b->margin[दायाँ_2];
			जारी;
		}

		अगर (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->height = पंक्ति_height(&content->unit_len_ctx,
					b->style ? b->style :
					b->parent->parent->style);
			अगर (height < b->height)
				height = b->height;

			अगर (!b->text) {
				b->width = 0;
				space_after = 0;
				जारी;
			}

			अगर (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				अगर (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					पूर्णांक opt_maxwidth = 0;
					काष्ठा form_option *o;

					क्रम (o = b->parent->parent->gadget->
							तथ्य.select.वस्तुएँ; o;
							o = o->अग्र) {
						पूर्णांक opt_width;
						font_func->width(&fstyle,
								o->text,
								म_माप(o->text),
								&opt_width);

						अगर (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}
					b->width = opt_maxwidth;
					अगर (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;
				} अन्यथा {
					font_func->width(&fstyle, b->text,
							b->लंबाई, &b->width);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			अगर (b->text && (x + b->width < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->अग्र) {
				font_func->width(&fstyle, b->text,
						 b->लंबाई, &b->width);
				b->flags |= MEASURED;
			}

			x += b->width;
			अगर (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			जारी;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		निश्चित(b->style);

		layout_find_dimensions(&content->unit_len_ctx,
				*width, -1, b, b->style,
				&b->width, &b->height,
				&max_width, &min_width,
				&max_height, &min_height,
				शून्य, शून्य, शून्य);

		अगर (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->width, &b->height,
					min_width, max_width,
					min_height, max_height);
		} अन्यथा अगर (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			अगर (b->width == AUTO)
				b->width = 400;
			अगर (b->height == AUTO)
				b->height = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} अन्यथा {
			/* form control with no object */
			अगर (b->width == AUTO)
				b->width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			अगर (b->height == AUTO)
				b->height = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		अगर (b->object && content_can_reformat(b->object) &&
				b->width !=
				content_get_available_width(b->object)) {
			css_fixed मूल्य = 0;
			css_unit unit = CSS_UNIT_PX;
			क्रमागत css_height_e htype = css_computed_height(b->style,
					&मूल्य, &unit);

			content_reformat(b->object, false, b->width, b->height);

			अगर (htype == CSS_HEIGHT_AUTO)
				b->height = content_get_height(b->object);
		}

		अगर (height < b->height)
			height = b->height;

		x += b->width;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *width;
	find_sides(cont->float_children, cy, cy + height, &x0, &x1,
			&बायाँ, &दायाँ);
	x0 -= cx;
	x1 -= cx;

	अगर (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	अगर (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	क्रम (x = x_previous = 0, b = first; x <= x1 - x0 && b; b = b->अग्र) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		अगर (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} अन्यथा अगर (lh__box_is_inline_flow(b)) {
			निश्चित(b->width != UNKNOWN_WIDTH);

			x_previous = x;
			x += space_after;
			b->x = x;

			अगर ((b->type == BOX_INLINE && !b->inline_end) ||
					b->type == BOX_INLINE_BLOCK ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[बायाँ_2] + b->border[बायाँ_2].width;
				x = b->x + b->padding[बायाँ_2] + b->width +
						b->padding[दायाँ_2] +
						b->border[दायाँ_2].width +
						b->margin[दायाँ_2];
			} अन्यथा अगर (b->type == BOX_INLINE) {
				b->x += b->margin[बायाँ_2] + b->border[बायाँ_2].width;
				x = b->x + b->padding[बायाँ_2] + b->width;
			} अन्यथा अगर (b->type == BOX_INLINE_END) {
				b->height = b->inline_end->height;
				x += b->padding[दायाँ_2] +
						b->border[दायाँ_2].width +
						b->margin[दायाँ_2];
			} अन्यथा {
				x += b->width;
			}

			space_before = space_after;
			अगर (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			अन्यथा अगर (b->text || b->type == BOX_INLINE_END) {
				अगर (b->space == UNKNOWN_WIDTH) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} अन्यथा {
				space_after = 0;
			}
			split_box = b;
			move_y = true;
			inline_गिनती++;
		} अन्यथा अगर (b->type == BOX_BR) {
			b->x = x;
			b->width = 0;
			br_box = b;
			b = b->अग्र;
			split_box = 0;
			move_y = true;
			अवरोध;

		} अन्यथा {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_place_below_level = 0;
			b->float_container = d->float_container = cont;

			अगर (!layout_float(d, *width, content))
				वापस false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[शीर्ष_2],
			      d->border[शीर्ष_2].width);

			d->x = d->margin[बायाँ_2] + d->border[बायाँ_2].width;
			d->y = d->margin[शीर्ष_2] + d->border[शीर्ष_2].width;
			b->width = d->margin[बायाँ_2] + d->border[बायाँ_2].width +
					d->padding[बायाँ_2] + d->width +
					d->padding[दायाँ_2] +
					d->border[दायाँ_2].width +
					d->margin[दायाँ_2];
			b->height = d->margin[शीर्ष_2] + d->border[शीर्ष_2].width +
					d->padding[शीर्ष_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];

			अगर (b->width > (x1 - x0) - x)
				place_below = true;
			अगर (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_बायाँ && बायाँ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_दायाँ &&
						दायाँ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						बायाँ == 0 && दायाँ == 0)) &&
					(!place_below ||
					(बायाँ == 0 && दायाँ == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_place_below_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				अगर (b->type == BOX_FLOAT_बायाँ) {
					b->x = cx + x0;
					अगर (b->width > 0)
						x0 += b->width;
					बायाँ = b;
				} अन्यथा {
					b->x = cx + x1 - b->width;
					अगर (b->width > 0)
						x1 -= b->width;
					दायाँ = b;
				}
				b->y = cy;
			} अन्यथा {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				पूर्णांक fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_place_below_level) ?
						fcy :
						cont->cached_place_below_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + height : fy;

				place_float_below(b, *width, cx, fy, cont);
				fy = b->y;
				अगर (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_बायाँ && बायाँ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_दायाँ &&
						दायाँ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(बायाँ != 0 || दायाँ != 0)))) {
					/* to be cleared below existing
					 * floats */
					अगर (b->type == BOX_FLOAT_बायाँ)
						b->x = cx;
					अन्यथा
						b->x = cx + *width - b->width;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					अगर (fcy > cont->clear_level)
						cont->clear_level = fcy;
					अगर (b->y < fcy)
						b->y = fcy;
				}
				अगर (b->type == BOX_FLOAT_बायाँ)
					बायाँ = b;
				अन्यथा
					दायाँ = b;
			}
			add_float_to_container(cont, b);

			split_box = 0;
		}
	}

	अगर (x1 - x0 < x && split_box) {
		/* the last box went over the end */
		माप_प्रकार split = 0;
		पूर्णांक w;
		bool no_wrap = css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_previous;

		अगर (!no_wrap &&
		    (split_box->type == BOX_INLINE ||
		     split_box->type == BOX_TEXT) &&
		    !split_box->object &&
		    !(split_box->flags & REPLACE_DIM) &&
		    !(split_box->flags & IFRAME) &&
		    !split_box->gadget && split_box->text) {

			font_plot_style_from_css(&content->unit_len_ctx,
					split_box->style, &fstyle);
			/** \todo handle errors */
			font_func->split(&fstyle,
					 split_box->text,
					 split_box->लंबाई,
					 x1 - x0 - x - space_before,
					 &split,
					 &w);
		}

		/* split == 0 implies that text can't be split */

		अगर (split == 0)
			w = split_box->width;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      split_box,
		      (पूर्णांक)split_box->लंबाई,
		      split_box->text,
		      split,
		      w,
		      बायाँ,
		      दायाँ,
		      inline_गिनती);

		अगर ((split == 0 || x1 - x0 <= x + space_before + w) &&
				!बायाँ && !दायाँ && inline_गिनती == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			अगर (split == 0 || split == split_box->लंबाई) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = split_box->अग्र;
			} अन्यथा {
				/* cut off first word for this line */
				अगर (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					वापस false;
				b = split_box->अग्र;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} अन्यथा अगर ((split == 0 || x1 - x0 <= x + space_before + w) &&
				inline_गिनती == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			निश्चित(बायाँ || दायाँ);
			used_height = 0;
			अगर (बायाँ) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      बायाँ->y,
				      बायाँ->height);

				used_height = बायाँ->y + बायाँ->height - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      used_height);

			}
			अगर (दायाँ && used_height <
					दायाँ->y + दायाँ->height - cy + 1)
				used_height = दायाँ->y + दायाँ->height - cy + 1;

			अगर (used_height < 0)
				used_height = 0;

			b = split_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} अन्यथा अगर (split == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = split_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} अन्यथा {
			/* fit as many words as possible */
			निश्चित(split != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (पूर्णांक)split_box->लंबाई, split_box->text,
			      x1 - x0, split, w);

			अगर (split != split_box->लंबाई) {
				अगर (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					वापस false;
				b = split_box->अग्र;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	चयन (css_computed_text_align(first->parent->parent->style)) {
	हाल CSS_TEXT_ALIGN_दायाँ:
	हाल CSS_TEXT_ALIGN_LIBCSS_दायाँ:
		x0 = x1 - x;
		अवरोध;
	हाल CSS_TEXT_ALIGN_CENTER:
	हाल CSS_TEXT_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		अवरोध;
	हाल CSS_TEXT_ALIGN_बायाँ:
	हाल CSS_TEXT_ALIGN_LIBCSS_बायाँ:
	हाल CSS_TEXT_ALIGN_JUSTIFY:
		/* leave on left */
		अवरोध;
	हाल CSS_TEXT_ALIGN_DEFAULT:
		/* None; consider text direction */
		चयन (css_computed_direction(first->parent->parent->style)) {
		हाल CSS_DIRECTION_LTR:
			/* leave on left */
			अवरोध;
		हाल CSS_DIRECTION_RTL:
			x0 = x1 - x;
			अवरोध;
		}
		अवरोध;
	}

	क्रम (d = first; d != b; d = d->अग्र) {
		d->flags &= ~NEW_पंक्ति;

		अगर (d->type == BOX_INLINE_BLOCK &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			जारी;
		} अन्यथा अगर ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[शीर्ष_2];

			अगर (d->type == BOX_TEXT && d->height > used_height) {
				/* text */
				used_height = d->height;
			}
		} अन्यथा अगर ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_BLOCK) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[शीर्ष_2].width + d->margin[शीर्ष_2];
			h = d->margin[शीर्ष_2] + d->border[शीर्ष_2].width +
					d->padding[शीर्ष_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];
			अगर (used_height < h)
				used_height = h;
		}
	}

	first->flags |= NEW_पंक्ति;

	निश्चित(b != first || (move_y && 0 < used_height && (बायाँ || दायाँ)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	क्रम (d = first; d != b; d = d->अग्र) {
		अगर ((d->type == BOX_INLINE && d->inline_end) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			css_fixed मूल्य = 0;
			css_unit unit = CSS_UNIT_PX;
			चयन (css_computed_vertical_align(d->style, &मूल्य,
					&unit)) {
			हाल CSS_VERTICAL_ALIGN_SUPER:
			हाल CSS_VERTICAL_ALIGN_शीर्ष:
			हाल CSS_VERTICAL_ALIGN_TEXT_शीर्ष:
				/* already at top */
				अवरोध;
			हाल CSS_VERTICAL_ALIGN_SUB:
			हाल CSS_VERTICAL_ALIGN_BOTTOM:
			हाल CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				d->y += used_height - d->height;
				अवरोध;
			शेष:
			हाल CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (used_height - d->height);
				अवरोध;
			}
		}
	}

	/* handle clearance for br */
	अगर (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		पूर्णांक clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		अगर (used_height < clear_y - cy)
			used_height = clear_y - cy;
	}

	अगर (move_y)
		*y += used_height;
	*अग्र_box = b;
	*width = x; /* return actual width */
	वापस true;
}


/**
 * Layout lines of text or inline boxes with floats.
 *
 * \param box inline container box
 * \param width horizontal space available
 * \param cont ancestor box which defines horizontal space, for floats
 * \param cx box position relative to cont
 * \param cy box position relative to cont
 * \param content  memory pool for any new boxes
 * \return true on success, false on memory exhaustion
 */
अटल bool layout_inline_container(काष्ठा box *inline_container, पूर्णांक width,
		काष्ठा box *cont, पूर्णांक cx, पूर्णांक cy, html_content *content)
{
	bool first_पंक्ति = true;
	bool has_text_children;
	काष्ठा box *c, *अग्र;
	पूर्णांक y = 0;
	पूर्णांक curwidth,maxwidth = width;

	निश्चित(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      width,
	      cont,
	      cx,
	      cy);


	has_text_children = false;
	क्रम (c = inline_container->children; c; c = c->अग्र) {
		bool is_pre = false;

		अगर (c->style) {
			क्रमागत css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_पंक्ति ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		अगर ((lh__box_is_object(c) == false &&
				c->text && (c->लंबाई || is_pre)) ||
				c->type == BOX_BR)
			has_text_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	क्रम (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->width;
		अगर (!layout_पंक्ति(c, &curwidth, &y, cx, cy + y, cont, first_पंक्ति,
				has_text_children, content, &अग्र))
			वापस false;
		maxwidth = max(maxwidth,curwidth);
		c = अग्र;
		first_पंक्ति = false;
	}

	inline_container->width = maxwidth;
	inline_container->height = y;

	वापस true;
}


/* Documented in layout_intertnal.h */
bool layout_block_context(
		काष्ठा box *block,
		पूर्णांक viewport_height,
		html_content *content)
{
	काष्ठा box *box;
	पूर्णांक cx, cy;  /**< current coordinates */
	पूर्णांक max_pos_margin = 0;
	पूर्णांक max_neg_margin = 0;
	पूर्णांक y = 0;
	पूर्णांक lm, rm;
	काष्ठा box *margin_collapse = शून्य;
	bool in_margin = false;
	css_fixed gadget_आकार;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	निश्चित(block->type == BOX_BLOCK ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX);
	निश्चित(block->width != UNKNOWN_WIDTH);
	निश्चित(block->width != AUTO);

	block->float_children = शून्य;
	block->cached_place_below_level = 0;
	block->clear_level = 0;

	/* special case if the block contains an object */
	अगर (block->object) {
		पूर्णांक अस्थायी_width = block->width;
		अगर (!layout_block_object(block))
			वापस false;
		layout_get_object_dimensions(block, &अस्थायी_width,
				&block->height, पूर्णांक_न्यून, पूर्णांक_उच्च,
				पूर्णांक_न्यून, पूर्णांक_उच्च);
		वापस true;
	} अन्यथा अगर (block->flags & REPLACE_DIM) {
		वापस true;
	}

	/* special case if the block contains an radio button or checkbox */
	अगर (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_आकार = INTTOFIX(1);
		अगर (block->height == AUTO)
			block->height = FIXTOINT(css_unit_len2device_px(
					block->style,
					&content->unit_len_ctx,
					gadget_आकार, gadget_unit));
	}

	box = block->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = block->padding[शीर्ष_2];
	अगर (box)
		box->y = block->padding[शीर्ष_2];

	/* Step through the descendants of the block in depth-first order, but
	 * not into the children of boxes which aren't blocks. For example, if
	 * the tree passed to this function looks like this (box->type shown):
	 *
	 *  block -> BOX_BLOCK
	 *             BOX_BLOCK * (1)
	 *               BOX_INLINE_CONTAINER * (2)
	 *                 BOX_INLINE
	 *                 BOX_TEXT
	 *                 ...
	 *             BOX_BLOCK * (3)
	 *               BOX_TABLE * (4)
	 *                 BOX_TABLE_ROW
	 *                   BOX_TABLE_CELL
	 *                     ...
	 *                   BOX_TABLE_CELL
	 *                     ...
	 *               BOX_BLOCK * (5)
	 *                 BOX_INLINE_CONTAINER * (6)
	 *                   BOX_TEXT
	 *                   ...
	 * then the while loop will visit each box marked with *, setting box
	 * to each in the order shown. */
	जबतक (box) {
		क्रमागत css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
		क्रमागत css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

		निश्चित(box->type == BOX_BLOCK ||
				box->type == BOX_FLEX ||
				box->type == BOX_TABLE ||
				box->type == BOX_INLINE_CONTAINER);

		/* Tables are laid out before being positioned, because the
		 * position depends on the width which is calculated in
		 * table layout. Blocks and inline containers are positioned
		 * before being laid out, because width is not dependent on
		 * content, and the position is required during layout for
		 * correct handling of floats.
		 */

		अगर (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->parent->padding[बायाँ_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			जाओ advance_to_अग्र_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		अगर (margin_collapse == शून्य) {
			margin_collapse = layout_अग्र_margin_block(
					&content->unit_len_ctx, box, block,
					viewport_height,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			in_margin = true;
		}

		/* Clearance. */
		y = 0;
		अगर (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(block->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		अगर (box->style) {
			overflow_x = css_computed_overflow_x(box->style);
			overflow_y = css_computed_overflow_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		अगर (box->type == BOX_FLEX ||
		    box->type == BOX_BLOCK ||
		    box->flags & IFRAME) {
			अगर (lh__box_is_object(box) == false &&
					box->style &&
					(overflow_x != CSS_OVERFLOW_VISIBLE ||
					 overflow_y != CSS_OVERFLOW_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				पूर्णांक x0, x1, शीर्ष;
				काष्ठा box *बायाँ, *दायाँ;
				शीर्ष = cy + max_pos_margin - max_neg_margin;
				शीर्ष = (शीर्ष > y) ? शीर्ष : y;
				x0 = cx;
				x1 = cx + box->parent->width -
						box->parent->padding[बायाँ_2] -
						box->parent->padding[दायाँ_2];
				find_sides(block->float_children, शीर्ष, शीर्ष,
						&x0, &x1, &बायाँ, &दायाँ);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->parent->width -
						box->parent->padding[बायाँ_2] -
						box->parent->padding[दायाँ_2] -
						x1;
			}
			layout_block_find_dimensions(&content->unit_len_ctx,
					box->parent->width,
					viewport_height, lm, rm, box);
			अगर (box->type == BOX_BLOCK && !(box->flags & IFRAME)) {
				layout_block_add_scrollbar(box, दायाँ_2);
				layout_block_add_scrollbar(box, BOTTOM);
			}
		} अन्यथा अगर (box->type == BOX_TABLE) {
			अगर (box->style != शून्य) {
				क्रमागत css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style, &width,
						&unit);

				अगर (wtype == CSS_WIDTH_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					पूर्णांक x0, x1, शीर्ष;
					काष्ठा box *बायाँ, *दायाँ;
					शीर्ष = cy + max_pos_margin -
							max_neg_margin;
					शीर्ष = (शीर्ष > y) ? शीर्ष : y;
					x0 = cx;
					x1 = cx + box->parent->width -
						box->parent->padding[बायाँ_2] -
						box->parent->padding[दायाँ_2];
					find_sides(block->float_children,
						शीर्ष, शीर्ष, &x0, &x1,
						&बायाँ, &दायाँ);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->parent->width -
						box->parent->padding[बायाँ_2] -
						box->parent->padding[दायाँ_2] -
						x1;
				}
			}
			अगर (!layout_table(box, box->parent->width - lm - rm,
					content))
				वापस false;
			layout_solve_width(box, box->parent->width, box->width,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->parent->padding[बायाँ_2] + box->margin[बायाँ_2] +
				box->border[बायाँ_2].width;
		cx += box->x;

		/* Position box: vertical. */
		अगर (box->border[शीर्ष_2].width) {
			box->y += box->border[शीर्ष_2].width;
			cy += box->border[शीर्ष_2].width;
		}

		/* Vertical margin */
		अगर (((box->type == BOX_BLOCK && (box->flags & HAS_HEIGHT)) ||
		     box->type == BOX_FLEX ||
		     box->type == BOX_TABLE ||
		     (box->type == BOX_INLINE_CONTAINER &&
		      !box_is_first_child(box)) ||
		     margin_collapse == box) &&
		    in_margin == true) {
			/* Margin goes above this box. */
			cy += max_pos_margin - max_neg_margin;
			box->y += max_pos_margin - max_neg_margin;

			/* Current margin has been applied. */
			in_margin = false;
			max_pos_margin = max_neg_margin = 0;
		}

		/* Handle clearance */
		अगर (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		अगर (box->type == BOX_FLEX ||
				(box->type == BOX_BLOCK && box->style &&
				 (overflow_x != CSS_OVERFLOW_VISIBLE ||
				  overflow_y != CSS_OVERFLOW_VISIBLE))) {

			अगर (box->type == BOX_FLEX) {
				अगर (!layout_flex(box, box->width, content)) {
					वापस false;
				}
			} अन्यथा {
				layout_block_context(box,
						viewport_height, content);
			}

			cy += box->padding[शीर्ष_2];

			अगर (box->height == AUTO) {
				box->height = 0;
				layout_block_add_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->height + box->padding[BOTTOM] +
					box->border[BOTTOM].width;
			y = box->y + box->padding[शीर्ष_2] + box->height +
					box->padding[BOTTOM] +
					box->border[BOTTOM].width;

			/* Skip children, because they are done in the new
			 * block context */
			जाओ advance_to_अग्र_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->width);

		/* Layout (except tables). */
		अगर (box->object) {
			अगर (!layout_block_object(box))
				वापस false;

		} अन्यथा अगर (box->type == BOX_INLINE_CONTAINER) {
			box->width = box->parent->width;
			अगर (!layout_inline_container(box, box->width, block,
					cx, cy, content))
				वापस false;

		} अन्यथा अगर (box->type == BOX_TABLE) {
			/* Move down to avoid floats if necessary. */
			पूर्णांक x0, x1;
			काष्ठा box *बायाँ, *दायाँ;
			y = cy;
			जबतक (1) {
				क्रमागत css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style,
						&width, &unit);

				x0 = cx;
				x1 = cx + box->parent->width;
				find_sides(block->float_children, y,
						y + box->height,
						&x0, &x1, &बायाँ, &दायाँ);
				अगर (wtype == CSS_WIDTH_AUTO)
					अवरोध;
				अगर (box->width <= x1 - x0)
					अवरोध;
				अगर (!बायाँ && !दायाँ)
					अवरोध;
				अन्यथा अगर (!बायाँ)
					y = दायाँ->y + दायाँ->height + 1;
				अन्यथा अगर (!दायाँ)
					y = बायाँ->y + बायाँ->height + 1;
				अन्यथा अगर (बायाँ->y + बायाँ->height <
						दायाँ->y + दायाँ->height)
					y = बायाँ->y + बायाँ->height + 1;
				अन्यथा
					y = दायाँ->y + दायाँ->height + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		अगर (box->type == BOX_BLOCK && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			अगर (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = शून्य;
			}

			y = box->padding[शीर्ष_2];
			box = box->children;
			box->y = y;
			cy += y;
			जारी;
		} अन्यथा अगर (box->type == BOX_BLOCK || box->object ||
				box->flags & IFRAME)
			cy += box->padding[शीर्ष_2];

		अगर (box->type == BOX_BLOCK && box->height == AUTO) {
			box->height = 0;
			layout_block_add_scrollbar(box, BOTTOM);
		}

		cy += box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width;
		cx -= box->x;
		y = box->y + box->padding[शीर्ष_2] + box->height +
				box->padding[BOTTOM] +
				box->border[BOTTOM].width;

	advance_to_अग्र_box:
		अगर (!box->अग्र) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			करो {
				अगर (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = शून्य;
				}

				/* Apply bottom margin */
				अगर (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				अन्यथा अगर (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->parent;
				अगर (box == block)
					अवरोध;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				अगर (box->type == BOX_BLOCK &&
						box->flags & MAKE_HEIGHT) {
					margin_collapse = शून्य;
					in_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				अगर (box->height == AUTO) {
					box->height = y - box->padding[शीर्ष_2];

					अगर (box->type == BOX_BLOCK)
						layout_block_add_scrollbar(box,
								BOTTOM);
				} अन्यथा
					cy += box->height -
							(y - box->padding[शीर्ष_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				अगर (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_height(
							&content->unit_len_ctx,
							box, शून्य)) {
					/* Height altered */
					/* Set current cy */
					cy += box->height -
							(y - box->padding[शीर्ष_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].width;
				cx -= box->x;
				y = box->y + box->padding[शीर्ष_2] + box->height +
						box->padding[BOTTOM] +
						box->border[BOTTOM].width;

			} जबतक (box->अग्र == शून्य);
			अगर (box == block)
				अवरोध;
		}

		/* To next sibling. */

		अगर (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = शून्य;
		}

		अगर (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		अन्यथा अगर (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->अग्र;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	क्रम (box = block->float_children; box; box = box->अग्र_float) {
		y = box->y + box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width + box->margin[BOTTOM];
		अगर (cy < y)
			cy = y;
	}

	अगर (block->height == AUTO) {
		block->height = cy - block->padding[शीर्ष_2];
		अगर (block->type == BOX_BLOCK)
			layout_block_add_scrollbar(block, BOTTOM);
	}

	अगर (block->style && css_computed_position(block->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_height(&content->unit_len_ctx, block, शून्य);
	}

	अगर (block->gadget &&
			(block->gadget->type == GADGET_TEXTAREA ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		पूर्णांक ta_width = block->padding[बायाँ_2] + block->width +
				block->padding[दायाँ_2];
		पूर्णांक ta_height = block->padding[शीर्ष_2] + block->height +
				block->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				block->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_set_layout(block->gadget->तथ्य.text.ta,
				&fstyle, ta_width, ta_height,
				block->padding[शीर्ष_2], block->padding[दायाँ_2],
				block->padding[BOTTOM], block->padding[बायाँ_2]);
	}

	वापस true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
अटल bool
layout__get_element_tag(
		स्थिर dom_गाँठ *गाँठ,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_गाँठ_type गाँठ_type;
	dom_exception exc;

	exc = dom_गाँठ_get_गाँठ_type(गाँठ, &गाँठ_type);
	अगर (exc != DOM_NO_ERR ||
	    गाँठ_type != DOM_ELEMENT_गाँठ) {
		वापस false;
	}

	exc = dom_html_element_get_tag_type(गाँठ, &element_type);
	अगर (exc != DOM_NO_ERR) {
		वापस false;
	}

	*type = element_type;
	वापस true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
अटल अंतरभूत bool
layout__check_element_type(
		स्थिर dom_गाँठ *गाँठ,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	अगर (!layout__get_element_tag(गाँठ, &element_type)) {
		वापस false;
	}

	वापस element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
अटल bool
layout_get_li_मूल्य(dom_गाँठ *li_गाँठ, dom_long *मूल्य_out)
{
	dom_exception exc;
	dom_long मूल्य;
	bool has_मूल्य;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_गाँठ,
			corestring_dom_मूल्य,
			&has_मूल्य);
	अगर (exc != DOM_NO_ERR || has_मूल्य == false) {
		वापस false;
	}

	exc = dom_html_li_element_get_मूल्य(
			(dom_html_li_element *)li_गाँठ,
			&मूल्य);
	अगर (exc != DOM_NO_ERR) {
		वापस false;
	}

	*मूल्य_out = मूल्य;
	वापस true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
अटल bool
layout__get_ol_start(dom_गाँठ *ol_गाँठ, dom_long *start_out)
{
	dom_exception exc;
	dom_long start;
	bool has_start;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_गाँठ,
			corestring_dom_start,
			&has_start);
	अगर (exc != DOM_NO_ERR || has_start == false) {
		वापस false;
	}

	exc = dom_html_olist_element_get_start(
			(dom_html_olist_element *)ol_गाँठ,
			&start);
	अगर (exc != DOM_NO_ERR) {
		वापस false;
	}

	*start_out = start;
	वापस true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
अटल bool
layout__get_ol_reversed(dom_गाँठ *ol_गाँठ)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_गाँठ,
			corestring_dom_reversed,
			&has_reversed);
	अगर (exc != DOM_NO_ERR) {
		वापस false;
	}

	वापस has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
अटल bool
layout_get_list_item_गिनती(
		dom_गाँठ *list_owner, dom_long *गिनती_out)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_गाँठ *child;
	पूर्णांक गिनती;

	अगर (list_owner == शून्य) {
		वापस false;
	}

	अगर (!layout__get_element_tag(list_owner, &tag_type)) {
		वापस false;
	}

	अगर (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		वापस false;
	}

	exc = dom_गाँठ_get_first_child(list_owner, &child);
	अगर (exc != DOM_NO_ERR) {
		वापस false;
	}

	गिनती = 0;
	जबतक (child != शून्य) {
		dom_गाँठ *अस्थायी_गाँठ;

		अगर (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			काष्ठा box *child_box;
			अगर (dom_गाँठ_get_user_तथ्य(child,
					corestring_dom_ns_कुंजी_box_गाँठ_तथ्य,
					&child_box) != DOM_NO_ERR) {
				dom_गाँठ_unref(child);
				वापस false;
			}

			अगर (child_box != शून्य &&
			    child_box->list_marker != शून्य) {
				गिनती++;
			}
		}

		exc = dom_गाँठ_get_अग्र_sibling(child, &अस्थायी_गाँठ);
		dom_गाँठ_unref(child);
		अगर (exc != DOM_NO_ERR) {
			वापस false;
		}

		child = अस्थायी_गाँठ;
	}

	*गिनती_out = गिनती;
	वापस true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
अटल व्योम
layout_ordered_list_गिनती(
		काष्ठा box *box)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_गाँठ *child;
	पूर्णांक step = 1;
	dom_long अग्र;

	अगर (box->गाँठ == शून्य) {
		वापस;
	}

	अगर (!layout__get_element_tag(box->गाँठ, &tag_type)) {
		वापस;
	}

	अगर (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		वापस;
	}

	अग्र = 1;
	अगर (tag_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_start = layout__get_ol_start(box->गाँठ, &अग्र);
		bool have_reversed = layout__get_ol_reversed(box->गाँठ);

		अगर (have_reversed) {
			step = -1;
		}

		अगर (!have_start && have_reversed) {
			layout_get_list_item_गिनती(box->गाँठ, &अग्र);
		}
	}

	exc = dom_गाँठ_get_first_child(box->गाँठ, &child);
	अगर (exc != DOM_NO_ERR) {
		वापस;
	}

	जबतक (child != शून्य) {
		dom_गाँठ *अस्थायी_गाँठ;

		अगर (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			काष्ठा box *child_box;

			अगर (dom_गाँठ_get_user_तथ्य(child,
					corestring_dom_ns_कुंजी_box_गाँठ_तथ्य,
					&child_box) != DOM_NO_ERR) {
				dom_गाँठ_unref(child);
				वापस;
			}

			अगर (child_box != शून्य &&
			    child_box->list_marker != शून्य) {
				dom_long मूल्य;
				काष्ठा box *marker = child_box->list_marker;
				अगर (layout_get_li_मूल्य(child, &मूल्य)) {
					marker->list_मूल्य = मूल्य;
					अग्र = marker->list_मूल्य;
				} अन्यथा {
					marker->list_मूल्य = अग्र;
				}
				अग्र += step;
			}
		}

		exc = dom_गाँठ_get_अग्र_sibling(child, &अस्थायी_गाँठ);
		dom_गाँठ_unref(child);
		अगर (exc != DOM_NO_ERR) {
			वापस;
		}

		child = अस्थायी_गाँठ;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
अटल व्योम
layout__set_numerical_marker_text(
		स्थिर html_content *content,
		काष्ठा box *box)
{
	काष्ठा box *marker = box->list_marker;
	माप_प्रकार गणक_len;
	css_error css_res;
	क्रमागत {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_आकार = 20,
	};

	marker->text = talloc_array(content->bctx, अक्षर, LIST_MARKER_आकार);
	अगर (marker->text == शून्य) {
		वापस;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_मूल्य,
			marker->text, LIST_MARKER_आकार, &गणक_len);
	अगर (css_res == CSS_OK) {
		अगर (गणक_len > LIST_MARKER_आकार) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->text = talloc_realloc(content->bctx,
					marker->text,
					अक्षर,
					गणक_len);
			अगर (marker->text == शून्य) {
				वापस;
			}
			css_computed_format_list_style(box->style,
					marker->list_मूल्य, marker->text,
					गणक_len, &गणक_len);
		}
		marker->लंबाई = गणक_len;
	}
}

/**
 * Find out if box's style represents a numerical list style type.
 *
 * \param[in]  b  Box with style to test.
 * \return true if box has numerical list style type, false otherwise.
 */
अटल bool
layout__list_item_is_numerical(
		स्थिर काष्ठा box *b)
{
	क्रमागत css_list_style_type_e t = css_computed_list_style_type(b->style);

	चयन (t) {
	हाल CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	हाल CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	हाल CSS_LIST_STYLE_TYPE_SQUARE: /* Fall through. */
	हाल CSS_LIST_STYLE_TYPE_NONE:
		वापस false;

	शेष:
		वापस true;
	}
}

/**
 * Layout list markers.
 */
अटल व्योम
layout_lists(स्थिर html_content *content, काष्ठा box *box)
{
	काष्ठा box *child;

	layout_ordered_list_गिनती(box);

	क्रम (child = box->children; child; child = child->अग्र) {
		अगर (child->list_marker) {
			काष्ठा box *marker = child->list_marker;

			अगर (layout__list_item_is_numerical(child)) {
				अगर (marker->text == शून्य) {
					layout__set_numerical_marker_text(
							content, child);
				}
			}
			अगर (marker->object) {
				marker->width =
					content_get_width(marker->object);
				marker->x = -marker->width;
				marker->height =
					content_get_height(marker->object);
				marker->y = (पंक्ति_height(
						&content->unit_len_ctx,
						marker->style) -
						marker->height) / 2;
			} अन्यथा अगर (marker->text) {
				अगर (marker->width == UNKNOWN_WIDTH) {
					plot_font_style_t fstyle;
					font_plot_style_from_css(
							&content->unit_len_ctx,
							marker->style,
							&fstyle);
					content->font_func->width(&fstyle,
							marker->text,
							marker->लंबाई,
							&marker->width);
					marker->flags |= MEASURED;
				}
				marker->x = -marker->width;
				marker->y = 0;
				marker->height = पंक्ति_height(
						&content->unit_len_ctx,
						marker->style);
			} अन्यथा {
				marker->x = 0;
				marker->y = 0;
				marker->width = 0;
				marker->height = 0;
			}
			/* Gap between marker and content */
			marker->x -= 4;
		}
		layout_lists(content, child);
	}
}


/**
 * Compute box offsets for a relatively or absolutely positioned box with
 * respect to a box.
 *
 * \param  unit_len_ctx           Length conversion context
 * \param  box               box to compute offsets for
 * \param  containing_block  box to compute percentages with respect to
 * \param  top               updated to top offset, or AUTO
 * \param  right             updated to right offset, or AUTO
 * \param  bottom            updated to bottom offset, or AUTO
 * \param  left              updated to left offset, or AUTO
 *
 * See CSS 2.1 9.3.2. containing_block must have width and height.
 */
अटल व्योम
layout_compute_offsets(स्थिर css_unit_ctx *unit_len_ctx,
		       काष्ठा box *box,
		       काष्ठा box *containing_block,
		       पूर्णांक *शीर्ष,
		       पूर्णांक *दायाँ,
		       पूर्णांक *bottom,
		       पूर्णांक *बायाँ)
{
	uint32_t type;
	css_fixed मूल्य = 0;
	css_unit unit = CSS_UNIT_PX;

	निश्चित(containing_block->width != UNKNOWN_WIDTH);
	निश्चित(containing_block->width != AUTO);
	निश्चित(containing_block->height != AUTO);

	/*बायाँ*/
	type = css_computed_बायाँ(box->style, &मूल्य, &unit);
	अगर (type == CSS_बायाँ_SET) {
		अगर (unit == CSS_UNIT_PCT) {
			*बायाँ = FPCT_OF_INT_TOINT(मूल्य,
					containing_block->width);
		} अन्यथा {
			*बायाँ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					मूल्य, unit));
		}
	} अन्यथा {
		*बायाँ = AUTO;
	}

	/*दायाँ*/
	type = css_computed_दायाँ(box->style, &मूल्य, &unit);
	अगर (type == CSS_दायाँ_SET) {
		अगर (unit == CSS_UNIT_PCT) {
			*दायाँ = FPCT_OF_INT_TOINT(मूल्य,
					containing_block->width);
		} अन्यथा {
			*दायाँ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					मूल्य, unit));
		}
	} अन्यथा {
		*दायाँ = AUTO;
	}

	/*शीर्ष*/
	type = css_computed_शीर्ष(box->style, &मूल्य, &unit);
	अगर (type == CSS_शीर्ष_SET) {
		अगर (unit == CSS_UNIT_PCT) {
			*शीर्ष = FPCT_OF_INT_TOINT(मूल्य,
					containing_block->height);
		} अन्यथा {
			*शीर्ष = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					मूल्य, unit));
		}
	} अन्यथा {
		*शीर्ष = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &मूल्य, &unit);
	अगर (type == CSS_BOTTOM_SET) {
		अगर (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_OF_INT_TOINT(मूल्य,
					containing_block->height);
		} अन्यथा {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					मूल्य, unit));
		}
	} अन्यथा {
		*bottom = AUTO;
	}
}


/**
 * Layout and position an absolutely positioned box.
 *
 * \param  box               absolute box to layout and position
 * \param  containing_block  containing block
 * \param  cx                position of box relative to containing_block
 * \param  cy                position of box relative to containing_block
 * \param  content           memory pool for any new boxes
 * \return  true on success, false on memory exhaustion
 */
अटल bool
layout_absolute(काष्ठा box *box,
		काष्ठा box *containing_block,
		पूर्णांक cx, पूर्णांक cy,
		html_content *content)
{
	पूर्णांक static_बायाँ, static_शीर्ष;  /* static position */
	पूर्णांक शीर्ष, दायाँ, bottom, बायाँ;
	पूर्णांक width, height, max_width, min_width;
	पूर्णांक *margin = box->margin;
	पूर्णांक *padding = box->padding;
	काष्ठा box_border *border = box->border;
	पूर्णांक available_width = containing_block->width;
	पूर्णांक space;

	निश्चित(box->type == BOX_BLOCK || box->type == BOX_TABLE ||
			box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_बायाँ = cx + box->x;
	static_शीर्ष = cy + box->y;

	अगर (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_block->width += containing_block->padding[बायाँ_2] +
				containing_block->padding[दायाँ_2];
		containing_block->height += containing_block->padding[शीर्ष_2] +
				containing_block->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_block,
			&शीर्ष, &दायाँ, &bottom, &बायाँ);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_block;
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1,
			box, box->style, &width, &height,
			&max_width, &min_width, 0, 0,
			margin, padding, border);
	box->float_container = शून्य;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      बायाँ, margin[बायाँ_2], border[बायाँ_2].width, padding[बायाँ_2], width,
	      padding[दायाँ_2], border[दायाँ_2].width, margin[दायाँ_2], दायाँ,
	      containing_block->width);


	अगर (बायाँ == AUTO && width == AUTO && दायाँ == AUTO) {
		अगर (margin[बायाँ_2] == AUTO)
			margin[बायाँ_2] = 0;
		अगर (margin[दायाँ_2] == AUTO)
			margin[दायाँ_2] = 0;
		बायाँ = static_बायाँ;

		width = min(max(box->min_width, available_width),
			box->max_width);
		width -= box->margin[बायाँ_2] + box->border[बायाँ_2].width +
			box->padding[बायाँ_2] + box->padding[दायाँ_2] +
			box->border[दायाँ_2].width + box->margin[दायाँ_2];

		/* Adjust for {min|max}-width */
		अगर (max_width >= 0 && width > max_width) width = max_width;
		अगर (width < min_width) width = min_width;

		दायाँ = containing_block->width -
			बायाँ -
			margin[बायाँ_2] - border[बायाँ_2].width - padding[बायाँ_2] -
			width -
			padding[दायाँ_2] - border[दायाँ_2].width - margin[दायाँ_2];
	} अन्यथा अगर (बायाँ != AUTO && width != AUTO && दायाँ != AUTO) {

		/* Adjust for {min|max}-width */
		अगर (max_width >= 0 && width > max_width) width = max_width;
		अगर (min_width >  0 && width < min_width) width = min_width;

		अगर (margin[बायाँ_2] == AUTO && margin[दायाँ_2] == AUTO) {
			space = containing_block->width -
					बायाँ - border[बायाँ_2].width -
					padding[बायाँ_2] - width - padding[दायाँ_2] -
					border[दायाँ_2].width - दायाँ;
			अगर (space < 0) {
				margin[बायाँ_2] = 0;
				margin[दायाँ_2] = space;
			} अन्यथा {
				margin[बायाँ_2] = margin[दायाँ_2] = space / 2;
			}
		} अन्यथा अगर (margin[बायाँ_2] == AUTO) {
			margin[बायाँ_2] = containing_block->width -
					बायाँ - border[बायाँ_2].width -
					padding[बायाँ_2] - width - padding[दायाँ_2] -
					border[दायाँ_2].width - margin[दायाँ_2] -
					दायाँ;
		} अन्यथा अगर (margin[दायाँ_2] == AUTO) {
			margin[दायाँ_2] = containing_block->width -
					बायाँ - margin[बायाँ_2] -
					border[बायाँ_2].width -
					padding[बायाँ_2] - width - padding[दायाँ_2] -
					border[दायाँ_2].width - दायाँ;
		} अन्यथा {
			दायाँ = containing_block->width -
					बायाँ - margin[बायाँ_2] -
					border[बायाँ_2].width -
					padding[बायाँ_2] - width - padding[दायाँ_2] -
					border[दायाँ_2].width - margin[दायाँ_2];
		}
	} अन्यथा {
		अगर (margin[बायाँ_2] == AUTO)
			margin[बायाँ_2] = 0;
		अगर (margin[दायाँ_2] == AUTO)
			margin[दायाँ_2] = 0;

		अगर (बायाँ == AUTO && width == AUTO && दायाँ != AUTO) {
			available_width -= दायाँ;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[बायाँ_2] + box->border[बायाँ_2].width +
				box->padding[बायाँ_2] + box->padding[दायाँ_2] +
				box->border[दायाँ_2].width + box->margin[दायाँ_2];

			/* Adjust for {min|max}-width */
			अगर (max_width >= 0 && width > max_width)
				width = max_width;
			अगर (width < min_width)
				width = min_width;

			बायाँ = containing_block->width -
					margin[बायाँ_2] - border[बायाँ_2].width -
					padding[बायाँ_2] - width - padding[दायाँ_2] -
					border[दायाँ_2].width - margin[दायाँ_2] -
					दायाँ;
		} अन्यथा अगर (बायाँ == AUTO && width != AUTO && दायाँ == AUTO) {

			/* Adjust for {min|max}-width */
			अगर (max_width >= 0 && width > max_width)
				width = max_width;
			अगर (min_width >  0 && width < min_width)
				width = min_width;

			बायाँ = static_बायाँ;
			दायाँ = containing_block->width -
					बायाँ - margin[बायाँ_2] -
					border[बायाँ_2].width -
					padding[बायाँ_2] - width - padding[दायाँ_2] -
					border[दायाँ_2].width - margin[दायाँ_2];
		} अन्यथा अगर (बायाँ != AUTO && width == AUTO && दायाँ == AUTO) {
			available_width -= बायाँ;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[बायाँ_2] + box->border[बायाँ_2].width +
				box->padding[बायाँ_2] + box->padding[दायाँ_2] +
				box->border[दायाँ_2].width + box->margin[दायाँ_2];

			/* Adjust for {min|max}-width */
			अगर (max_width >= 0 && width > max_width)
				width = max_width;
			अगर (width < min_width)
				width = min_width;

			दायाँ = containing_block->width -
					बायाँ - margin[बायाँ_2] -
					border[बायाँ_2].width -
					padding[बायाँ_2] - width - padding[दायाँ_2] -
					border[दायाँ_2].width - margin[दायाँ_2];
		} अन्यथा अगर (बायाँ == AUTO && width != AUTO && दायाँ != AUTO) {

			/* Adjust for {min|max}-width */
			अगर (max_width >= 0 && width > max_width)
				width = max_width;
			अगर (width < min_width)
				width = min_width;

			बायाँ = containing_block->width -
					margin[बायाँ_2] - border[बायाँ_2].width -
					padding[बायाँ_2] - width - padding[दायाँ_2] -
					border[दायाँ_2].width - margin[दायाँ_2] -
					दायाँ;
		} अन्यथा अगर (बायाँ != AUTO && width == AUTO && दायाँ != AUTO) {
			width = containing_block->width -
					बायाँ - margin[बायाँ_2] -
					border[बायाँ_2].width -
					padding[बायाँ_2] - padding[दायाँ_2] -
					border[दायाँ_2].width - margin[दायाँ_2] -
					दायाँ;

			/* Adjust for {min|max}-width */
			अगर (max_width >= 0 && width > max_width)
				width = max_width;
			अगर (width < min_width)
				width = min_width;

		} अन्यथा अगर (बायाँ != AUTO && width != AUTO && दायाँ == AUTO) {

			/* Adjust for {min|max}-width */
			अगर (max_width >= 0 && width > max_width)
				width = max_width;
			अगर (width < min_width)
				width = min_width;

			दायाँ = containing_block->width -
					बायाँ - margin[बायाँ_2] -
					border[बायाँ_2].width -
					padding[बायाँ_2] - width - padding[दायाँ_2] -
					border[दायाँ_2].width - margin[दायाँ_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      बायाँ, margin[बायाँ_2], border[बायाँ_2].width, padding[बायाँ_2], width,
	      padding[दायाँ_2], border[दायाँ_2].width, margin[दायाँ_2], दायाँ,
	      containing_block->width);

	box->x = बायाँ + margin[बायाँ_2] + border[बायाँ_2].width - cx;
	अगर (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_block->width -= containing_block->padding[बायाँ_2] +
				containing_block->padding[दायाँ_2];
	} अन्यथा {
		/** \todo inline ancestors */
	}
	box->width = width;
	box->height = height;

	अगर (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->object || box->flags & IFRAME) {
		अगर (!layout_block_context(box, -1, content))
			वापस false;
	} अन्यथा अगर (box->type == BOX_TABLE) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		/* \todo  layout_table considers margins etc. again */
		अगर (!layout_table(box, width, content))
			वापस false;
		box->float_container = शून्य;
		layout_solve_width(box, box->parent->width, box->width, 0, 0,
				-1, -1);
	} अन्यथा अगर (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		अगर (!layout_flex(box, width, content))
			वापस false;
		box->float_container = शून्य;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      शीर्ष, margin[शीर्ष_2], border[शीर्ष_2].width, padding[शीर्ष_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	अगर (शीर्ष == AUTO && height == AUTO && bottom == AUTO) {
		शीर्ष = static_शीर्ष;
		height = box->height;
		अगर (margin[शीर्ष_2] == AUTO)
			margin[शीर्ष_2] = 0;
		अगर (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_block->height -
				शीर्ष - margin[शीर्ष_2] - border[शीर्ष_2].width -
				padding[शीर्ष_2] - height - padding[BOTTOM] -
				border[BOTTOM].width - margin[BOTTOM];
	} अन्यथा अगर (शीर्ष != AUTO && height != AUTO && bottom != AUTO) {
		अगर (margin[शीर्ष_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_block->height -
					शीर्ष - border[शीर्ष_2].width - padding[शीर्ष_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - bottom;
			margin[शीर्ष_2] = margin[BOTTOM] = space / 2;
		} अन्यथा अगर (margin[शीर्ष_2] == AUTO) {
			margin[शीर्ष_2] = containing_block->height -
					शीर्ष - border[शीर्ष_2].width - padding[शीर्ष_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} अन्यथा अगर (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_block->height -
					शीर्ष - margin[शीर्ष_2] - border[शीर्ष_2].width -
					padding[शीर्ष_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					bottom;
		} अन्यथा {
			bottom = containing_block->height -
					शीर्ष - margin[शीर्ष_2] - border[शीर्ष_2].width -
					padding[शीर्ष_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	} अन्यथा {
		अगर (margin[शीर्ष_2] == AUTO)
			margin[शीर्ष_2] = 0;
		अगर (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		अगर (शीर्ष == AUTO && height == AUTO && bottom != AUTO) {
			height = box->height;
			शीर्ष = containing_block->height -
					margin[शीर्ष_2] - border[शीर्ष_2].width -
					padding[शीर्ष_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} अन्यथा अगर (शीर्ष == AUTO && height != AUTO && bottom == AUTO) {
			शीर्ष = static_शीर्ष;
			bottom = containing_block->height -
					शीर्ष - margin[शीर्ष_2] - border[शीर्ष_2].width -
					padding[शीर्ष_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} अन्यथा अगर (शीर्ष != AUTO && height == AUTO && bottom == AUTO) {
			height = box->height;
			bottom = containing_block->height -
					शीर्ष - margin[शीर्ष_2] - border[शीर्ष_2].width -
					padding[शीर्ष_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} अन्यथा अगर (शीर्ष == AUTO && height != AUTO && bottom != AUTO) {
			शीर्ष = containing_block->height -
					margin[शीर्ष_2] - border[शीर्ष_2].width -
					padding[शीर्ष_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} अन्यथा अगर (शीर्ष != AUTO && height == AUTO && bottom != AUTO) {
			height = containing_block->height -
					शीर्ष - margin[शीर्ष_2] - border[शीर्ष_2].width -
					padding[शीर्ष_2] - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} अन्यथा अगर (शीर्ष != AUTO && height != AUTO && bottom == AUTO) {
			bottom = containing_block->height -
					शीर्ष - margin[शीर्ष_2] - border[शीर्ष_2].width -
					padding[शीर्ष_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      शीर्ष, margin[शीर्ष_2], border[शीर्ष_2].width, padding[शीर्ष_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	box->y = शीर्ष + margin[शीर्ष_2] + border[शीर्ष_2].width - cy;
	अगर (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_block->height -= containing_block->padding[शीर्ष_2] +
				containing_block->padding[BOTTOM];
	} अन्यथा {
		/** \todo Inline ancestors */
	}
	box->height = height;
	layout_apply_minmax_height(&content->unit_len_ctx, box, containing_block);

	वापस true;
}


/**
 * Recursively layout and position absolutely positioned boxes.
 *
 * \param  box               tree of boxes to layout
 * \param  containing_block  current containing block
 * \param  cx                position of box relative to containing_block
 * \param  cy                position of box relative to containing_block
 * \param  content           memory pool for any new boxes
 * \return  true on success, false on memory exhaustion
 */
अटल bool
layout_position_absolute(काष्ठा box *box,
			 काष्ठा box *containing_block,
			 पूर्णांक cx, पूर्णांक cy,
			 html_content *content)
{
	काष्ठा box *c;

	क्रम (c = box->children; c; c = c->अग्र) {
		अगर ((c->type == BOX_BLOCK || c->type == BOX_TABLE ||
				c->type == BOX_INLINE_BLOCK ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			अगर (!layout_absolute(c, containing_block,
					cx, cy, content))
				वापस false;
			अगर (!layout_position_absolute(c, c, 0, 0, content))
				वापस false;
		} अन्यथा अगर (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			अगर (!layout_position_absolute(c, c, 0, 0, content))
				वापस false;
		} अन्यथा {
			पूर्णांक px, py;
			अगर (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_बायाँ ||
					css_computed_float(c->style) ==
						CSS_FLOAT_दायाँ)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				काष्ठा box *p;
				px = c->x;
				py = c->y;
				क्रम (p = box->parent; p && !p->float_children;
						p = p->parent) {
					px -= p->x;
					py -= p->y;
				}
			} अन्यथा {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			अगर (!layout_position_absolute(c, containing_block,
					cx + px, cy + py, content))
				वापस false;
		}
	}

	वापस true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
अटल व्योम layout_compute_relative_offset(
		स्थिर css_unit_ctx *unit_len_ctx,
		काष्ठा box *box,
		पूर्णांक *x,
		पूर्णांक *y)
{
	पूर्णांक बायाँ, दायाँ, शीर्ष, bottom;
	काष्ठा box *containing_block;

	निश्चित(box && box->parent && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	अगर (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_बायाँ ||
			css_computed_float(box->style) == CSS_FLOAT_दायाँ)) {
		containing_block = box->float_container;
	} अन्यथा {
		containing_block = box->parent;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_block,
			&शीर्ष, &दायाँ, &bottom, &बायाँ);

	अगर (बायाँ == AUTO && दायाँ == AUTO)
		बायाँ = दायाँ = 0;
	अन्यथा अगर (बायाँ == AUTO)
		/* left is auto => computed = -right */
		बायाँ = -दायाँ;
	अन्यथा अगर (दायाँ == AUTO)
		/* right is auto => computed = -left */
		दायाँ = -बायाँ;
	अन्यथा {
		/* over constrained => examine direction property
		 * of containing block */
		अगर (containing_block->style &&
				css_computed_direction(
				containing_block->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			बायाँ = -दायाँ;
		} अन्यथा {
			/* assume LTR in all other cases */
			दायाँ = -बायाँ;
		}
	}

	निश्चित(बायाँ == -दायाँ);

	अगर (शीर्ष == AUTO && bottom == AUTO) {
		शीर्ष = bottom = 0;
	} अन्यथा अगर (शीर्ष == AUTO) {
		शीर्ष = -bottom;
	} अन्यथा {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -शीर्ष;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", बायाँ,
	      दायाँ, शीर्ष, bottom);

	*x = बायाँ;
	*y = शीर्ष;
}


/**
 * Adjust positions of relatively positioned boxes.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  root  box to adjust the position of
 * \param  fp    box which forms the block formatting context for children of
 *		 "root" which are floats
 * \param  fx    x offset due to intervening relatively positioned boxes
 *               between current box, "root", and the block formatting context
 *               box, "fp", for float children of "root"
 * \param  fy    y offset due to intervening relatively positioned boxes
 *               between current box, "root", and the block formatting context
 *               box, "fp", for float children of "root"
 */
अटल व्योम
layout_position_relative(
		स्थिर css_unit_ctx *unit_len_ctx,
		काष्ठा box *मूल,
		काष्ठा box *fp,
		पूर्णांक fx,
		पूर्णांक fy)
{
	काष्ठा box *box; /* for children of "root" */
	काष्ठा box *fn;  /* for block formatting context box for children of
			  * "box" */
	काष्ठा box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	पूर्णांक x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	पूर्णांक fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	निश्चित(मूल);

	/* Normal children */
	क्रम (box = मूल->children; box; box = box->अग्र) {

		अगर (box->type == BOX_TEXT)
			जारी;

		/* If relatively positioned, get offsets */
		अगर (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_offset(
					unit_len_ctx, box, &x, &y);
		अन्यथा
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		अगर (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_बायाँ ||
				css_computed_float(box->style) ==
					CSS_FLOAT_दायाँ) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			क्रम (fc = fp->float_children; fc; fc = fc->अग्र_float) {
				अगर (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		अगर (box->float_children) {
			fn = box;
			fnx = fny = 0;
		} अन्यथा {
			fn = fp;
			fnx = fx + x;
			fny = fy + y;
		}

		/* recurse first */
		layout_position_relative(unit_len_ctx, box, fn, fnx, fny);

		/* Ignore things we're not interested in. */
		अगर (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			जारी;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		अगर (box->type == BOX_INLINE && box->inline_end) {
			काष्ठा box *b;
			क्रम (b = box->अग्र; b && b != box->inline_end;
					b = b->अग्र) {
				b->x += x;
				b->y += y;
			}
		}
	}
}


/**
 * Find a box's bounding box relative to itself, i.e. the box's border edge box
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      box find bounding box of
 * \param  desc_x0  updated to left of box's bbox
 * \param  desc_y0  updated to top of box's bbox
 * \param  desc_x1  updated to right of box's bbox
 * \param  desc_y1  updated to bottom of box's bbox
 */
अटल व्योम
layout_get_box_bbox(
		स्थिर css_unit_ctx *unit_len_ctx,
		काष्ठा box *box,
		पूर्णांक *desc_x0, पूर्णांक *desc_y0,
		पूर्णांक *desc_x1, पूर्णांक *desc_y1)
{
	*desc_x0 = -box->border[बायाँ_2].width;
	*desc_y0 = -box->border[शीर्ष_2].width;
	*desc_x1 = box->padding[बायाँ_2] + box->width + box->padding[दायाँ_2] +
			box->border[दायाँ_2].width;
	*desc_y1 = box->padding[शीर्ष_2] + box->height + box->padding[BOTTOM] +
			box->border[BOTTOM].width;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	अगर (box->type == BOX_BLOCK && box->style != शून्य &&
			css_computed_overflow_y(box->style) ==
					CSS_OVERFLOW_VISIBLE &&
			box->object == शून्य) {
		css_fixed font_आकार = 0;
		css_unit font_unit = CSS_UNIT_PT;
		पूर्णांक text_height;

		css_computed_font_आकार(box->style, &font_आकार, &font_unit);
		text_height = css_unit_len2device_px(box->style, unit_len_ctx,
				font_आकार, font_unit);
		text_height = FIXTOINT(text_height * 3 / 4);
		*desc_y0 = (*desc_y0 < -text_height) ? *desc_y0 : -text_height;
	}
}


/**
 * Apply changes to box descendant_[xy][01] values due to given child.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      box to update
 * \param  child    a box, which may affect box's descendant bbox
 * \param  off_x    offset to apply to child->x coord to treat as child of box
 * \param  off_y    offset to apply to child->y coord to treat as child of box
 */
अटल व्योम
layout_update_descendant_bbox(
		स्थिर css_unit_ctx *unit_len_ctx,
		काष्ठा box *box,
		काष्ठा box *child,
		पूर्णांक off_x,
		पूर्णांक off_y)
{
	पूर्णांक child_desc_x0, child_desc_y0, child_desc_x1, child_desc_y1;

	/* get coordinates of child relative to box */
	पूर्णांक child_x = child->x - off_x;
	पूर्णांक child_y = child->y - off_y;

	bool html_object = (child->object &&
			content_get_type(child->object) == CONTENT_HTML);

	क्रमागत css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	क्रमागत css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

	अगर (child->style != शून्य) {
		overflow_x = css_computed_overflow_x(child->style);
		overflow_y = css_computed_overflow_y(child->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, child,
			&child_desc_x0, &child_desc_y0,
			&child_desc_x1, &child_desc_y1);

	अगर (overflow_x == CSS_OVERFLOW_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		child_desc_x0 = child->descendant_x0;
		child_desc_x1 = child->descendant_x1;
	}
	अगर (overflow_y == CSS_OVERFLOW_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		child_desc_y0 = child->descendant_y0;
		child_desc_y1 = child->descendant_y1;
	}

	child_desc_x0 += child_x;
	child_desc_y0 += child_y;
	child_desc_x1 += child_x;
	child_desc_y1 += child_y;

	/* increase box's descendant bbox to contain descendants */
	अगर (child_desc_x0 < box->descendant_x0)
		box->descendant_x0 = child_desc_x0;
	अगर (child_desc_y0 < box->descendant_y0)
		box->descendant_y0 = child_desc_y0;
	अगर (box->descendant_x1 < child_desc_x1)
		box->descendant_x1 = child_desc_x1;
	अगर (box->descendant_y1 < child_desc_y1)
		box->descendant_y1 = child_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
अटल व्योम layout_calculate_descendant_bboxes(
		स्थिर css_unit_ctx *unit_len_ctx,
		काष्ठा box *box)
{
	काष्ठा box *child;

	निश्चित(box->width != UNKNOWN_WIDTH);
	निश्चित(box->height != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	अगर (box->object && content_get_type(box->object) == CONTENT_HTML) {
		अगर (box->descendant_x1 < content_get_width(box->object))
			box->descendant_x1 = content_get_width(box->object);
		अगर (box->descendant_y1 < content_get_height(box->object))
			box->descendant_y1 = content_get_height(box->object);
	}

	अगर (box->iframe != शून्य) {
		पूर्णांक x, y;
		box_coords(box, &x, &y);

		browser_window_set_position(box->iframe, x, y);
		browser_window_set_dimensions(box->iframe,
				box->width, box->height);
		browser_window_reformat(box->iframe, true,
				box->width, box->height);
	}

	अगर (box->type == BOX_INLINE || box->type == BOX_TEXT)
		वापस;

	अगर (box->type == BOX_INLINE_END) {
		box = box->inline_end;
		क्रम (child = box->अग्र; child;
				child = child->अग्र) {
			अगर (child->type == BOX_FLOAT_बायाँ ||
					child->type == BOX_FLOAT_दायाँ)
				जारी;

			layout_update_descendant_bbox(unit_len_ctx, box, child,
					box->x, box->y);

			अगर (child == box->inline_end)
				अवरोध;
		}
		वापस;
	}

	अगर (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		वापस;

	क्रम (child = box->children; child; child = child->अग्र) {
		अगर (child->type == BOX_FLOAT_बायाँ ||
				child->type == BOX_FLOAT_दायाँ)
			जारी;

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		अगर (box->style && css_computed_overflow_x(box->style) ==
				CSS_OVERFLOW_HIDDEN &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_HIDDEN)
			जारी;

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	क्रम (child = box->float_children; child; child = child->अग्र_float) {
		निश्चित(child->type == BOX_FLOAT_बायाँ ||
				child->type == BOX_FLOAT_दायाँ);

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	अगर (box->list_marker) {
		child = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, पूर्णांक width, पूर्णांक height)
{
	bool ret;
	काष्ठा box *doc = content->layout;
	स्थिर काष्ठा gui_layout_table *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			width, height, nsurl_access(content_get_url(
					&content->base)));

	layout_minmax_block(doc, font_func, content);

	layout_block_find_dimensions(&content->unit_len_ctx,
			width, height, 0, 0, doc);
	doc->x = doc->margin[बायाँ_2] + doc->border[बायाँ_2].width;
	doc->y = doc->margin[शीर्ष_2] + doc->border[शीर्ष_2].width;
	width -= doc->margin[बायाँ_2] + doc->border[बायाँ_2].width +
			doc->padding[बायाँ_2] + doc->padding[दायाँ_2] +
			doc->border[दायाँ_2].width + doc->margin[दायाँ_2];
	अगर (width < 0) {
		width = 0;
	}
	doc->width = width;

	ret = layout_block_context(doc, height, content);

	/* make <html> and <body> fill available height */
	अगर (doc->y + doc->padding[शीर्ष_2] + doc->height + doc->padding[BOTTOM] +
			doc->border[BOTTOM].width + doc->margin[BOTTOM] <
			height) {
		doc->height = height - (doc->y + doc->padding[शीर्ष_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].width +
				doc->margin[BOTTOM]);
		अगर (doc->children)
			doc->children->height = doc->height -
					(doc->children->margin[शीर्ष_2] +
					 doc->children->border[शीर्ष_2].width +
					 doc->children->padding[शीर्ष_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].width +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	वापस ret;
}
