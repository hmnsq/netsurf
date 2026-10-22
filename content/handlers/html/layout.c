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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <math.h>
#ଅନ୍ତର୍ଭାଵ <dom/dom.h>

#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/talloc.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/browser_window.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/layout.h"
#ଅନ୍ତର୍ଭାଵ "content/content.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "css/utils.h"
#ଅନ୍ତର୍ଭାଵ "desktop/scrollbar.h"
#ଅନ୍ତର୍ଭାଵ "desktop/textarea.h"

#ଅନ୍ତର୍ଭାଵ "html/html.h"
#ଅନ୍ତର୍ଭାଵ "html/html_save.h"
#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_inspect.h"
#ଅନ୍ତର୍ଭାଵ "html/font.h"
#ଅନ୍ତର୍ଭାଵ "html/form_internal.h"
#ଅନ୍ତର୍ଭାଵ "html/layout.h"
#ଅନ୍ତର୍ଭାଵ "html/layout_internal.h"
#ଅନ୍ତର୍ଭାଵ "html/table.h"

/** Array of per-side access functions for computed style margins. */
ଅଚଳ css_len_func margin_funcs[4] = {
	[ଶୀର୍ଷ_2]    = css_computed_margin_ଶୀର୍ଷ,
	[ଡାହାଣ_2]  = css_computed_margin_ଡାହାଣ,
	[BOTTOM] = css_computed_margin_bottom,
	[ବାମ_2]   = css_computed_margin_ବାମ,
};

/** Array of per-side access functions for computed style paddings. */
ଅଚଳ css_len_func padding_funcs[4] = {
	[ଶୀର୍ଷ_2]    = css_computed_padding_ଶୀର୍ଷ,
	[ଡାହାଣ_2]  = css_computed_padding_ଡାହାଣ,
	[BOTTOM] = css_computed_padding_bottom,
	[ବାମ_2]   = css_computed_padding_ବାମ,
};

/** Array of per-side access functions for computed style border_widths. */
ଅଚଳ css_len_func border_width_funcs[4] = {
	[ଶୀର୍ଷ_2]    = css_computed_border_ଶୀର୍ଷ_width,
	[ଡାହାଣ_2]  = css_computed_border_ଡାହାଣ_width,
	[BOTTOM] = css_computed_border_bottom_width,
	[ବାମ_2]   = css_computed_border_ବାମ_width,
};

/** Array of per-side access functions for computed style border styles. */
ଅଚଳ css_border_style_func border_style_funcs[4] = {
	[ଶୀର୍ଷ_2]    = css_computed_border_ଶୀର୍ଷ_style,
	[ଡାହାଣ_2]  = css_computed_border_ଡାହାଣ_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[ବାମ_2]   = css_computed_border_ବାମ_style,
};

/** Array of per-side access functions for computed style border colors. */
ଅଚଳ css_border_color_func border_color_funcs[4] = {
	[ଶୀର୍ଷ_2]    = css_computed_border_ଶୀର୍ଷ_color,
	[ଡାହାଣ_2]  = css_computed_border_ଡାହାଣ_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[ବାମ_2]   = css_computed_border_ବାମ_color,
};

/* forward declaration to break cycles */
ସ୍ଥିର ଶୂନ୍ୟ layout_minmax_block(
		ସଂରଚନା box *block,
		ଅଚଳ ସଂରଚନା gui_layout_table *font_func,
		ଅଚଳ html_content *content);

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
ସ୍ଥିର ଶୂନ୍ୟ
layout_get_object_dimensions(ସଂରଚନା box *box,
			     ପୂର୍ଣ୍ଣସଂଖ୍ୟା *width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *height,
			     ପୂର୍ଣ୍ଣସଂଖ୍ୟା min_width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_width,
			     ପୂର୍ଣ୍ଣସଂଖ୍ୟା min_height, ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_height)
{
	ପ୍ରତିଜ୍ଞା(box->object != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(width != ଖାଲି && height != ଖାଲି);

	ଯଦି (*width == AUTO && *height == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା intrinsic_width = content_get_width(box->object);
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା intrinsic_height = content_get_height(box->object);

		/* use intrinsic dimensions */
		*width = intrinsic_width;
		*height = intrinsic_height;

		/* Deal with min/max-width first */
		ଯଦି (min_width >  0 && min_width > *width) {
			*width = min_width;
			scaled = true;
		}
		ଯଦି (max_width >= 0 && max_width < *width) {
			*width = max_width;
			scaled = true;
		}

		ଯଦି (scaled && (intrinsic_width != 0)) {
			/* Update height */
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		}

		scaled = false;
		/* Deal with min/max-height */
		ଯଦି (min_height >  0 && min_height > *height) {
			*height = min_height;
			scaled = true;
		}
		ଯଦି (max_height >= 0 && max_height < *height) {
			*height = max_height;
			scaled = true;
		}

		ଯଦି (scaled && (intrinsic_height != 0)) {
			/* Update width */
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		}

	} ଅନ୍ୟଥା ଯଦି (*width == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା intrinsic_width = content_get_width(box->object);
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା intrinsic_height = content_get_height(box->object);

		ଯଦି (intrinsic_height != 0)
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		ଅନ୍ୟଥା
			*width = intrinsic_width;

		ଯଦି (min_width >  0 && min_width > *width)
			*width = min_width;
		ଯଦି (max_width >= 0 && max_width < *width)
			*width = max_width;

	} ଅନ୍ୟଥା ଯଦି (*height == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା intrinsic_width = content_get_width(box->object);
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା intrinsic_height = content_get_height(box->object);

		ଯଦି (min_width >  0 && min_width > *width)
			*width = min_width;
		ଯଦି (max_width >= 0 && max_width < *width)
			*width = max_width;

		ଯଦି (intrinsic_width != 0)
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		ଅନ୍ୟଥା
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା layout_text_indent(
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ଅଚଳ css_computed_style *style, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width)
{
	css_fixed ମୂଲ୍ୟ = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_text_indent(style, &ମୂଲ୍ୟ, &unit);

	ଯଦି (unit == CSS_UNIT_PCT) {
		ଫେରନ୍ତୁ FPCT_OF_INT_TOINT(ମୂଲ୍ୟ, width);
	} ଅନ୍ୟଥା {
		ଫେରନ୍ତୁ FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				ମୂଲ୍ୟ, unit));
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
ସ୍ଥିର ଶୂନ୍ୟ layout_minmax_table(ସଂରଚନା box *table,
		ଅଚଳ ସଂରଚନା gui_layout_table *font_func,
		ଅଚଳ html_content *content)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା border_spacing_h = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା table_min = 0, table_max = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra_fixed = 0;
	ଦଶମିକ extra_frac = 0;
	ସଂରଚନା column *col;
	ସଂରଚନା box *row_group, *row, *cell;

	/* check if the widths have already been calculated */
	ଯଦି (table->max_width != UNKNOWN_MAX_WIDTH)
		ଫେରନ୍ତୁ;

	ଯଦି (table_calculate_column_types(&content->unit_len_ctx, table) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		ଫେରନ୍ତୁ;
	}
	col = table->col;

	/* start with 0 except for fixed-width columns */
	ପାଇଁ (i = 0; i != table->columns; i++) {
		ଯଦି (col[i].type == COLUMN_WIDTH_FIXED)
			col[i].min = col[i].max = col[i].width;
		ଅନ୍ୟଥା
			col[i].min = col[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	ଯଦି (css_computed_border_collapse(table->style) ==
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
	ପାଇଁ (row_group = table->children; row_group; row_group =row_group->next)
	ପାଇଁ (row = row_group->children; row; row = row->next)
	ପାଇଁ (cell = row->children; cell; cell = cell->next) {
		ପ୍ରତିଜ୍ଞା(cell->type == BOX_TABLE_CELL);
		ପ୍ରତିଜ୍ଞା(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		ପ୍ରତିଜ୍ଞା(cell->columns != 0);

		ଯଦି (cell->columns != 1)
			ଜାରି;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		ଯଦି (col[i].positioned)
			ଜାରି;

		/* update column min, max widths using cell widths */
		ଯଦି (col[i].min < cell->min_width)
			col[i].min = cell->min_width;
		ଯଦି (col[i].max < cell->max_width)
			col[i].max = cell->max_width;
	}

	/* 2nd pass: cells which span multiple columns */
	ପାଇଁ (row_group = table->children; row_group; row_group =row_group->next)
	ପାଇଁ (row = row_group->children; row; row = row->next)
	ପାଇଁ (cell = row->children; cell; cell = cell->next) {
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା flexible_columns = 0;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା min = 0, max = 0, fixed_width = 0, extra;

		ଯଦି (cell->columns == 1)
			ଜାରି;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		ପାଇଁ (j = 0; j != cell->columns; j++) {
			min += col[i + j].min;
			ଯଦି (col[i + j].type == COLUMN_WIDTH_FIXED)
				fixed_width += col[i + j].width;
			ଅନ୍ୟଥା
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		ଯଦି (min < cell->min_width) {
			ଯଦି (flexible_columns == 0) {
				extra = 1 + (cell->min_width - min) /
						cell->columns;
				ପାଇଁ (j = 0; j != cell->columns; j++) {
					col[i + j].min += extra;
					ଯଦି (col[i + j].max < col[i + j].min)
						col[i + j].max = col[i + j].min;
				}
			} ଅନ୍ୟଥା {
				extra = 1 + (cell->min_width - min) /
						flexible_columns;
				ପାଇଁ (j = 0; j != cell->columns; j++) {
					ଯଦି (col[i + j].type !=
							COLUMN_WIDTH_FIXED) {
						col[i + j].min += extra;
						ଯଦି (col[i + j].max <
								col[i + j].min)
							col[i + j].max =
								col[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		ପାଇଁ (j = 0; j != cell->columns; j++)
			max += col[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		ଯଦି (max < cell->max_width && flexible_columns) {
			extra = 1 + (cell->max_width - max) / flexible_columns;
			ପାଇଁ (j = 0; j != cell->columns; j++)
				ଯଦି (col[i + j].type != COLUMN_WIDTH_FIXED)
					col[i + j].max += extra;
		}
	}

	ପାଇଁ (i = 0; i != table->columns; i++) {
		ଯଦି (col[i].max < col[i].min) {
			box_dump(stderr, table, 0, true);
			ପ୍ରତିଜ୍ଞା(0);
		}
		table_min += col[i].min;
		table_max += col[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	ଯଦି (css_computed_width_px(table->style, &content->unit_len_ctx,
			-1, &width) == CSS_WIDTH_SET) {
		ଯଦି (table_min < width)
			table_min = width;
		ଯଦି (table_max < width)
			table_max = width;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, ବାମ_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, ଡାହାଣ_2, true, true, true,
			&extra_fixed, &extra_frac);
	ଯଦି (extra_fixed < 0)
		extra_fixed = 0;
	ଯଦି (extra_frac < 0)
		extra_frac = 0;
	ଯଦି (1.0 <= extra_frac)
		extra_frac = 0.9;
	table->min_width = (table_min + extra_fixed) / (1.0 - extra_frac);
	table->max_width = (table_max + extra_fixed) / (1.0 - extra_frac);
	table->min_width += (table->columns + 1) * border_spacing_h;
	table->max_width += (table->columns + 1) * border_spacing_h;

	ପ୍ରତିଜ୍ଞା(0 <= table->min_width && table->min_width <= table->max_width);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool box_has_percentage_max_width(ସଂରଚନା box *b)
{
	css_unit unit = CSS_UNIT_PX;
	ଗଣନା css_max_width_e type;
	css_fixed ମୂଲ୍ୟ = 0;

	ପ୍ରତିଜ୍ଞା(b != ଖାଲି);

	type = css_computed_max_width(b->style, &ମୂଲ୍ୟ, &unit);
	ଫେରନ୍ତୁ ((type == CSS_MAX_WIDTH_SET) && (unit == CSS_UNIT_PCT));
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
ସ୍ଥିର ସଂରଚନା box *
layout_minmax_ଧାଡ଼ି(ସଂରଚନା box *first,
		   ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ଧାଡ଼ି_min,
		   ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ଧାଡ଼ି_max,
		   bool first_ଧାଡ଼ି,
		   bool *ଧାଡ଼ି_has_height,
		   ଅଚଳ ସଂରଚନା gui_layout_table *font_func,
		   ଅଚଳ html_content *content)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା min = 0, max = 0, width, height, fixed;
	ଦଶମିକ frac;
	size_t i, j;
	ସଂରଚନା box *b;
	ସଂରଚନା box *block;
	plot_font_style_t fstyle;
	bool no_wrap;

	ପ୍ରତିଜ୍ଞା(first->parent);
	ପ୍ରତିଜ୍ଞା(first->parent->parent);
	ପ୍ରତିଜ୍ଞା(first->parent->parent->style);

	block = first->parent->parent;
	no_wrap = (css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_PRE);

	*ଧାଡ଼ି_has_height = false;

	/* corresponds to the pass 1 loop in layout_line() */
	ପାଇଁ (b = first; b; b = b->next) {
		ଗଣନା css_width_e wtype;
		ଗଣନା css_height_e htype;
		ଗଣନା css_box_sizing_e bs;
		css_fixed ମୂଲ୍ୟ = 0;
		css_unit unit = CSS_UNIT_PX;

		ପ୍ରତିଜ୍ଞା(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		ଯଦି (b->type == BOX_BR) {
			b = b->next;
			ଭାଙ୍ଗ;
		}

		ଯଦି (lh__box_is_float_box(b)) {
			ପ୍ରତିଜ୍ଞା(b->children);
			ଯଦି (b->children->type == BOX_TABLE)
				layout_minmax_table(b->children, font_func,
						content);
			ଅନ୍ୟଥା
				layout_minmax_block(b->children, font_func,
						content);
			b->min_width = b->children->min_width;
			b->max_width = b->children->max_width;
			ଯଦି (min < b->min_width)
				min = b->min_width;
			max += b->max_width;
			ଜାରି;
		}

		ଯଦି (b->type == BOX_INLINE_BLOCK || b->type == BOX_INLINE_FLEX) {
			layout_minmax_block(b, font_func, content);
			ଯଦି (min < b->min_width)
				min = b->min_width;
			max += b->max_width;

			ଯଦି (b->flags & HAS_HEIGHT)
				*ଧାଡ଼ି_has_height = true;
			ଜାରି;
		}

		ପ୍ରତିଜ୍ଞା(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		ଯଦି (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->style, ବାମ_2, true, true, true,
					&fixed, &frac);
			ଯଦି (!b->inline_end)
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ଡାହାଣ_2,
						true, true, true,
						&fixed, &frac);
			ଯଦି (0 < fixed)
				max += fixed;
			*ଧାଡ଼ି_has_height = true;
			/* \todo  update min width, consider fractional extra */
		} ଅନ୍ୟଥା ଯଦି (b->type == BOX_INLINE_END) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->inline_end->style, ଡାହାଣ_2,
					true, true, true,
					&fixed, &frac);
			ଯଦି (0 < fixed)
				max += fixed;

			ଯଦି (b->next) {
				ଯଦି (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*ଧାଡ଼ି_has_height = true;
			ଜାରି;
		}

		ଯଦି (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			ଯଦି (!b->text)
				ଜାରି;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			ଯଦି (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				ଯଦି (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_maxwidth = 0;
					ସଂରଚନା form_option *o;

					ପାଇଁ (o = b->parent->parent->gadget->
							data.select.ବସ୍ତୁଗୁଡ଼ିକ; o;
							o = o->next) {
						ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						ଯଦି (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}

					b->width = opt_maxwidth;
					ଯଦି (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;

				} ଅନ୍ୟଥା {
					font_func->width(&fstyle, b->text,
						b->length, &b->width);
					b->flags |= MEASURED;
				}
			}
			max += b->width;
			ଯଦି (b->next) {
				ଯଦି (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			ଯଦି (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} ଅନ୍ୟଥା ଯଦି (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				ଯଦି (min < b->width)
					min = b->width;

			} ଅନ୍ୟଥା ଯଦି (b->parent->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				କର {
					ପାଇଁ (j = i; j != b->length &&
							b->text[j] != ' '; j++)
						;
					font_func->width(&fstyle, b->text + i,
							 j - i, &width);
					ଯଦି (min < width)
						min = width;
					i = j + 1;
				} ଯେତେବେଳେ (j != b->length);
			}

			*ଧାଡ଼ି_has_height = true;

			ଜାରି;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		ପ୍ରତିଜ୍ଞା(b->style);

		bs = css_computed_box_sizing(block->style);

		/* calculate box width */
		wtype = css_computed_width_px(b->style,
				&content->unit_len_ctx, -1, &width);
		ଯଦି (wtype == CSS_WIDTH_SET) {
			ଯଦି (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, ବାମ_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, ଡାହାଣ_2,
						false, true, true,
						&fixed, &frac);
				ଯଦି (width < fixed) {
					width = fixed;
				}
			}
			ଯଦି (width < 0)
				width = 0;
		} ଅନ୍ୟଥା {
			width = AUTO;
		}

		/* height */
		htype = css_computed_height(b->style, &ମୂଲ୍ୟ, &unit);
		ଯଦି (htype == CSS_HEIGHT_SET) {
			height = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					ମୂଲ୍ୟ, unit));
		} ଅନ୍ୟଥା {
			height = AUTO;
		}

		ଯଦି (b->object || (b->flags & REPLACE_DIM)) {
			ଯଦି (b->object) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା temp_height = height;
				layout_get_object_dimensions(b,
						&width, &temp_height,
						INT_MIN, INT_MAX,
						INT_MIN, INT_MAX);
			}

			fixed = frac = 0;
			ଯଦି (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ବାମ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ଡାହାଣ_2,
						true, false, false,
						&fixed, &frac);
			} ଅନ୍ୟଥା {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ବାମ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ଡାହାଣ_2,
						true, true, true,
						&fixed, &frac);
			}
			ଯଦି (0 < width + fixed)
				width += fixed;
		} ଅନ୍ୟଥା ଯଦି (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			ଯଦି (width == AUTO)
				width = 400;

			fixed = frac = 0;
			ଯଦି (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ବାମ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ଡାହାଣ_2,
						true, false, false,
						&fixed, &frac);
			} ଅନ୍ୟଥା {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ବାମ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ଡାହାଣ_2,
						true, true, true,
						&fixed, &frac);
			}

			ଯଦି (0 < width + fixed)
				width += fixed;

		} ଅନ୍ୟଥା {
			/* form control with no object */
			ଯଦି (width == AUTO)
				width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		ଯଦି (min < width && !box_has_percentage_max_width(b))
			min = width;
		ଯଦି (width > 0)
			max += width;

		*ଧାଡ଼ି_has_height = true;
	}

	ଯଦି (first_ଧାଡ଼ି) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା text_indent = layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, 100);
		min = (min + text_indent < 0) ? 0 : min + text_indent;
		max = (max + text_indent < 0) ? 0 : max + text_indent;
	}

	*ଧାଡ଼ି_min = min;
	*ଧାଡ଼ି_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	ପ୍ରତିଜ୍ଞା(b != first);
	ପ୍ରତିଜ୍ଞା(0 <= *ଧାଡ଼ି_min);
	ପ୍ରତିଜ୍ଞା(*ଧାଡ଼ି_min <= *ଧାଡ଼ି_max);
	ଫେରନ୍ତୁ b;
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
ସ୍ଥିର ଶୂନ୍ୟ
layout_minmax_inline_container(ସଂରଚନା box *inline_container,
			       bool *has_height,
			       ଅଚଳ ସଂରଚନା gui_layout_table *font_func,
			       ଅଚଳ html_content *content)
{
	ସଂରଚନା box *child;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି_min = 0, ଧାଡ଼ି_max = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା min = 0, max = 0;
	bool first_ଧାଡ଼ି = true;
	bool ଧାଡ଼ି_has_height;

	ପ୍ରତିଜ୍ଞା(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	ଯଦି (inline_container->max_width != UNKNOWN_MAX_WIDTH)
		ଫେରନ୍ତୁ;

	*has_height = false;

	ପାଇଁ (child = inline_container->children; child; ) {
		child = layout_minmax_ଧାଡ଼ି(child, &ଧାଡ଼ି_min, &ଧାଡ଼ି_max,
				first_ଧାଡ଼ି, &ଧାଡ଼ି_has_height, font_func,
				content);
		ଯଦି (min < ଧାଡ଼ି_min)
			min = ଧାଡ଼ି_min;
		ଯଦି (max < ଧାଡ଼ି_max)
			max = ଧାଡ଼ି_max;
		first_ଧାଡ଼ି = false;
		*has_height |= ଧାଡ଼ି_has_height;
	}

	inline_container->min_width = min;
	inline_container->max_width = max;

	ପ୍ରତିଜ୍ଞା(0 <= inline_container->min_width &&
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
ସ୍ଥିର ଶୂନ୍ୟ layout_minmax_block(
		ସଂରଚନା box *block,
		ଅଚଳ ସଂରଚନା gui_layout_table *font_func,
		ଅଚଳ html_content *content)
{
	ସଂରଚନା box *child;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା min = 0, max = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra_fixed = 0;
	ଦଶମିକ extra_frac = 0;
	ଗଣନା css_width_e wtype = CSS_WIDTH_AUTO;
	css_fixed width = 0;
	css_unit wunit = CSS_UNIT_PX;
	ଗଣନା css_height_e htype = CSS_HEIGHT_AUTO;
	css_fixed height = 0;
	css_unit hunit = CSS_UNIT_PX;
	ଗଣନା css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool child_has_height = false;

	ପ୍ରତିଜ୍ଞା(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	/* check if the widths have already been calculated */
	ଯଦି (block->max_width != UNKNOWN_MAX_WIDTH)
		ଫେରନ୍ତୁ;

	ଯଦି (block->style != ଖାଲି) {
		wtype = css_computed_width(block->style, &width, &wunit);
		htype = css_computed_height(block->style, &height, &hunit);
		bs = css_computed_box_sizing(block->style);
	}

	/* set whether the minimum width is of any interest for this box */
	ଯଦି (((block->parent && lh__box_is_float_box(block->parent)) ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX) &&
			wtype != CSS_WIDTH_SET) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} ଅନ୍ୟଥା ଯଦି (block->type == BOX_TABLE_CELL) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} ଅନ୍ୟଥା ଯଦି (block->parent && (block->parent->flags & NEED_MIN) &&
			wtype != CSS_WIDTH_SET) {
		/* box inside shrink-to-fit context; need minimum width */
		block->flags |= NEED_MIN;
	} ଅନ୍ୟଥା ଯଦି (block->parent && (block->parent->type == BOX_FLEX)) {
		/* box is flex item */
		block->flags |= NEED_MIN;
	}

	ଯଦି (block->gadget && (block->gadget->type == GADGET_TEXTBOX ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_FILE ||
			block->gadget->type == GADGET_TEXTAREA) &&
			block->style && wtype == CSS_WIDTH_AUTO) {
		css_fixed size = INTTOFIX(10);
		css_unit unit = CSS_UNIT_EM;

		min = max = FIXTOINT(css_unit_len2device_px(block->style,
				&content->unit_len_ctx, size, unit));

		block->flags |= HAS_HEIGHT;
	}

	ଯଦି (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX) &&
			block->style && wtype == CSS_WIDTH_AUTO) {
		css_fixed size = INTTOFIX(1);
		css_unit unit = CSS_UNIT_EM;

		/* form checkbox or radio button
		 * if width is AUTO, set it to 1em */
		min = max = FIXTOINT(css_unit_len2device_px(block->style,
				&content->unit_len_ctx, size, unit));

		block->flags |= HAS_HEIGHT;
	}

	ଯଦି (block->object) {
		ଯଦି (content_get_type(block->object) == CONTENT_HTML) {
			layout_minmax_block(html_get_box_ବୃକ୍ଷ(block->object),
					font_func, content);
			min = html_get_box_ବୃକ୍ଷ(block->object)->min_width;
			max = html_get_box_ବୃକ୍ଷ(block->object)->max_width;
		} ଅନ୍ୟଥା {
			min = max = content_get_width(block->object);
		}

		block->flags |= HAS_HEIGHT;
	} ଅନ୍ୟଥା ଯଦି (block->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		block->flags |= HAS_HEIGHT;
	} ଅନ୍ୟଥା {
		/* recurse through children */
		ପାଇଁ (child = block->children; child; child = child->next) {
			ବାଛ (child->type) {
			ସ୍ଥିତି BOX_FLEX:
			ସ୍ଥିତି BOX_BLOCK:
				layout_minmax_block(child, font_func,
						content);
				ଯଦି (child->flags & HAS_HEIGHT)
					child_has_height = true;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି BOX_INLINE_CONTAINER:
				ଯଦି (block->flags & NEED_MIN)
					child->flags |= NEED_MIN;

				layout_minmax_inline_container(child,
						&child_has_height, font_func,
						content);
				ଯଦି (child_has_height &&
						child ==
						child->parent->children) {
					block->flags |= MAKE_HEIGHT;
				}
				ଭାଙ୍ଗ;
			ସ୍ଥିତି BOX_TABLE:
				layout_minmax_table(child, font_func,
						content);
				/* todo: fix for zero height tables */
				child_has_height = true;
				child->flags |= MAKE_HEIGHT;
				ଭାଙ୍ଗ;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				ପ୍ରତିଜ୍ଞା(0);
			}
			ପ୍ରତିଜ୍ଞା(child->max_width != UNKNOWN_MAX_WIDTH);

			ଯଦି (child->style &&
					(css_computed_position(child->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(child->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				ଜାରି;
			}

			ଯଦି (lh__box_is_flex_container(block) &&
			    lh__flex_main_is_horizontal(block)) {
				ଯଦି (block->style != ଖାଲି &&
				    css_computed_flex_wrap(block->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += child->min_width;
				} ଅନ୍ୟଥା {
					ଯଦି (min < child->min_width)
						min = child->min_width;
				}
				max += child->max_width;

			} ଅନ୍ୟଥା {
				ଯଦି (min < child->min_width)
					min = child->min_width;
				ଯଦି (max < child->max_width)
					max = child->max_width;
			}

			ଯଦି (child_has_height)
				block->flags |= HAS_HEIGHT;
		}
	}

	ଯଦି (max < min) {
		box_dump(stderr, block, 0, true);
		ପ୍ରତିଜ୍ଞା(0);
	}

	/* fixed width takes priority */
	ଯଦି (block->type != BOX_TABLE_CELL && !lh__box_is_flex_item(block)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		ଗଣନା css_max_width_e max_type;
		ଗଣନା css_min_width_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed ମୂଲ୍ୟ = 0;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା width;

		ଯଦି (css_computed_width_px(block->style, &content->unit_len_ctx,
				-1, &width) == CSS_WIDTH_SET) {
			min = max = width;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_width(block->style, &ମୂଲ୍ୟ, &unit);
		ଯଦି (min_type == CSS_MIN_WIDTH_SET && unit != CSS_UNIT_PCT) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, ମୂଲ୍ୟ, unit));

			ଯଦି (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_width(block->style, &ମୂଲ୍ୟ, &unit);
		ଯଦି (max_type == CSS_MAX_WIDTH_SET && unit != CSS_UNIT_PCT) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, ମୂଲ୍ୟ, unit));

			ଯଦି (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	ଯଦି (htype == CSS_HEIGHT_SET && hunit != CSS_UNIT_PCT &&
			height > INTTOFIX(0)) {
		block->flags |= MAKE_HEIGHT;
		block->flags |= HAS_HEIGHT;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_width(&content->unit_len_ctx, block->style, ବାମ_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, ଡାହାଣ_2,
			false, true, true, &extra_fixed, &extra_frac);

	ଯଦି (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	ଯଦି (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	ଯଦି (max < min) {
		min = max;
	}

	calculate_mbp_width(&content->unit_len_ctx, block->style, ବାମ_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, ଡାହାଣ_2,
			true, false, false, &extra_fixed, &extra_frac);

	ଯଦି (extra_fixed < 0)
		extra_fixed = 0;
	ଯଦି (extra_frac < 0)
		extra_frac = 0;
	ଯଦି (1.0 <= extra_frac)
		extra_frac = 0.9;
	ଯଦି (block->style != ଖାଲି &&
			(css_computed_float(block->style) == CSS_FLOAT_ବାମ ||
			css_computed_float(block->style) == CSS_FLOAT_ଡାହାଣ)) {
		/* floated boxs */
		block->min_width = min + extra_fixed;
		block->max_width = max + extra_fixed;
	} ଅନ୍ୟଥା {
		/* not floated */
		block->min_width = (min + extra_fixed) / (1.0 - extra_frac);
		block->max_width = (max + extra_fixed) / (1.0 - extra_frac);
	}

	ପ୍ରତିଜ୍ଞା(0 <= block->min_width);
	ପ୍ରତିଜ୍ଞା(block->min_width <= block->max_width);
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
ସ୍ଥିର ସଂରଚନା box*
layout_next_margin_block(ଅଚଳ css_unit_ctx *unit_len_ctx,
			 ସଂରଚନା box *box,
			 ସଂରଚନା box *block,
			 ପୂର୍ଣ୍ଣସଂଖ୍ୟା viewport_height,
			 ପୂର୍ଣ୍ଣସଂଖ୍ୟା *max_pos_margin,
			 ପୂର୍ଣ୍ଣସଂଖ୍ୟା *max_neg_margin)
{
	ପ୍ରତିଜ୍ଞା(block != ଖାଲି);

	ଯେତେବେଳେ (box != ଖାଲି) {

		ଯଦି (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			ଯଦି (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						ଖାଲି, ଖାଲି, ଖାଲି, ଖାଲି,
						ଖାଲି, ଖାଲି, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				ଯଦି (*max_pos_margin < box->margin[ଶୀର୍ଷ_2])
					*max_pos_margin = box->margin[ଶୀର୍ଷ_2];
				ଅନ୍ୟଥା ଯଦି (*max_neg_margin < -box->margin[ଶୀର୍ଷ_2])
					*max_neg_margin = -box->margin[ଶୀର୍ଷ_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			ଯଦି (box->flags & MAKE_HEIGHT ||
					box->border[ଶୀର୍ଷ_2].width ||
					box->padding[ଶୀର୍ଷ_2] ||
					(box->style &&
					css_computed_overflow_y(box->style) !=
					CSS_OVERFLOW_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_first_child(box))) {
				/* Collapse to this box; return it */
				ଫେରନ୍ତୁ box;
			}
		}


		/* Find next box */
		ଯଦି (box->type == BOX_BLOCK && !box->object && box->children &&
				box->style &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} ଅନ୍ୟଥା {
			ଯଦି (!box->next) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				କର {
					/* Apply bottom margin */
					ଯଦି (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					ଅନ୍ୟଥା ଯଦି (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->parent;
				} ଯେତେବେଳେ (box != block && !box->next);

				ଯଦି (box == block) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					ଫେରନ୍ତୁ block;
				}
			}

			/* Apply bottom margin */
			ଯଦି (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			ଅନ୍ୟଥା ଯଦି (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->next;

			/* Get margins */
			ଯଦି (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						ଖାଲି, ଖାଲି, ଖାଲି, ଖାଲି,
						ଖାଲି, ଖାଲି, box->margin,
						box->padding, box->border);
			}
		}
	}

	ଫେରନ୍ତୁ ଖାଲି;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା layout_clear(ସଂରଚନା box *fl, ଗଣନା css_clear_e clear)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା y = 0;
	ପାଇଁ (; fl; fl = fl->next_float) {
		ଯଦି ((clear == CSS_CLEAR_ବାମ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ବାମ)
			ଯଦି (y < fl->y + fl->height)
				y = fl->y + fl->height;
		ଯଦି ((clear == CSS_CLEAR_ଡାହାଣ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ଡାହାଣ)
			ଯଦି (y < fl->y + fl->height)
				y = fl->y + fl->height;
	}
	ଫେରନ୍ତୁ y;
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
ସ୍ଥିର ଶୂନ୍ୟ
find_sides(ସଂରଚନା box *fl,
	   ପୂର୍ଣ୍ଣସଂଖ୍ୟା y0, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y1,
	   ପୂର୍ଣ୍ଣସଂଖ୍ୟା *x0, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *x1,
	   ସଂରଚନା box **ବାମ,
	   ସଂରଚନା box **ଡାହାଣ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*ବାମ = *ଡାହାଣ = 0;
	ପାଇଁ (; fl; fl = fl->next_float) {
		fy1 = fl->y + fl->height;
		ଯଦି (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			ଫେରନ୍ତୁ;
		}
		fy0 = fl->y;
		ଯଦି (y0 < fy1 && fy0 <= y1) {
			ଯଦି (fl->type == BOX_FLOAT_ବାମ) {
				fx1 = fl->x + fl->width;
				ଯଦି (*x0 < fx1) {
					*x0 = fx1;
					*ବାମ = fl;
				}
			} ଅନ୍ୟଥା {
				fx0 = fl->x;
				ଯଦି (fx0 < *x1) {
					*x1 = fx0;
					*ଡାହାଣ = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *ବାମ, *ଡାହାଣ);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
layout_solve_width(ସଂରଚନା box *box,
		   ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width,
		   ପୂର୍ଣ୍ଣସଂଖ୍ୟା width,
		   ପୂର୍ଣ୍ଣସଂଖ୍ୟା lm,
		   ପୂର୍ଣ୍ଣସଂଖ୍ୟା rm,
		   ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_width,
		   ପୂର୍ଣ୍ଣସଂଖ୍ୟା min_width)
{
	bool auto_width = false;

	/* Increase specified left/right margins */
	ଯଦି (box->margin[ବାମ_2] != AUTO && box->margin[ବାମ_2] < lm &&
			box->margin[ବାମ_2] >= 0)
		box->margin[ବାମ_2] = lm;
	ଯଦି (box->margin[ଡାହାଣ_2] != AUTO && box->margin[ଡାହାଣ_2] < rm &&
			box->margin[ଡାହାଣ_2] >= 0)
		box->margin[ଡାହାଣ_2] = rm;

	/* Find width */
	ଯଦି (width == AUTO) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା margin_ବାମ = box->margin[ବାମ_2];
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା margin_ଡାହାଣ = box->margin[ଡାହାଣ_2];

		ଯଦି (margin_ବାମ == AUTO) {
			margin_ବାମ = lm;
		}
		ଯଦି (margin_ଡାହାଣ == AUTO) {
			margin_ଡାହାଣ = rm;
		}

		width = available_width -
				(margin_ବାମ + box->border[ବାମ_2].width +
				box->padding[ବାମ_2] + box->padding[ଡାହାଣ_2] +
				box->border[ଡାହାଣ_2].width + margin_ଡାହାଣ);
		width = width < 0 ? 0 : width;
		auto_width = true;
	}

	ଯଦି (max_width >= 0 && width > max_width) {
		/* max-width is admissable and width exceeds max-width */
		width = max_width;
		auto_width = false;
	}

	ଯଦି (min_width > 0 && width < min_width) {
		/* min-width is admissable and width is less than max-width */
		width = min_width;
		auto_width = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	ଯଦି (auto_width) {
		/* any other 'auto' become 0 or the minimum required values */
		ଯଦି (box->margin[ବାମ_2] == AUTO) {
			box->margin[ବାମ_2] = lm;
		}
		ଯଦି (box->margin[ଡାହାଣ_2] == AUTO) {
			box->margin[ଡାହାଣ_2] = rm;
		}
		ଫେରନ୍ତୁ width;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	ଯଦି (box->margin[ବାମ_2] != AUTO && box->margin[ଡାହାଣ_2] != AUTO &&
			box->parent != ଖାଲି && box->parent->style != ଖାଲି) {
		ବାଛ (css_computed_text_align(box->parent->style)) {
		ସ୍ଥିତି CSS_TEXT_ALIGN_LIBCSS_ଡାହାଣ:
			box->margin[ବାମ_2] = AUTO;
			box->margin[ଡାହାଣ_2] = 0;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି CSS_TEXT_ALIGN_LIBCSS_CENTER:
			box->margin[ବାମ_2] = box->margin[ଡାହାଣ_2] = AUTO;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି CSS_TEXT_ALIGN_LIBCSS_ବାମ:
			box->margin[ବାମ_2] = 0;
			box->margin[ଡାହାଣ_2] = AUTO;
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			/* Leave it alone; no HTML alignment */
			ଭାଙ୍ଗ;
		}
	}

	ଯଦି (box->margin[ବାମ_2] == AUTO && box->margin[ଡାହାଣ_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[ବାମ_2] = box->margin[ଡାହାଣ_2] =
				(available_width - lm - rm -
				(box->border[ବାମ_2].width + box->padding[ବାମ_2] +
				width + box->padding[ଡାହାଣ_2] +
				box->border[ଡାହାଣ_2].width)) / 2;

		ଯଦି (box->margin[ବାମ_2] < 0) {
			box->margin[ଡାହାଣ_2] += box->margin[ବାମ_2];
			box->margin[ବାମ_2] = 0;
		}

		box->margin[ବାମ_2] += lm;

	} ଅନ୍ୟଥା ଯଦି (box->margin[ବାମ_2] == AUTO) {
		box->margin[ବାମ_2] = available_width - lm -
				(box->border[ବାମ_2].width + box->padding[ବାମ_2] +
				width + box->padding[ଡାହାଣ_2] +
				box->border[ଡାହାଣ_2].width + box->margin[ଡାହାଣ_2]);
		box->margin[ବାମ_2] = box->margin[ବାମ_2] < lm
				? lm : box->margin[ବାମ_2];
	} ଅନ୍ୟଥା {
		/* margin-right auto or "over-constrained" */
		box->margin[ଡାହାଣ_2] = available_width - rm -
				(box->margin[ବାମ_2] + box->border[ବାମ_2].width +
				 box->padding[ବାମ_2] + width +
				 box->padding[ଡାହାଣ_2] +
				 box->border[ଡାହାଣ_2].width);
	}

	ଫେରନ୍ତୁ width;
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
ସ୍ଥିର ଶୂନ୍ୟ
layout_block_find_dimensions(ଅଚଳ css_unit_ctx *unit_len_ctx,
			     ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width,
			     ପୂର୍ଣ୍ଣସଂଖ୍ୟା viewport_height,
			     ପୂର୍ଣ୍ଣସଂଖ୍ୟା lm,
			     ପୂର୍ଣ୍ଣସଂଖ୍ୟା rm,
			     ସଂରଚନା box *box)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, max_width, min_width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା height, max_height, min_height;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା *margin = box->margin;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା *padding = box->padding;
	ସଂରଚନା box_border *border = box->border;
	ଅଚଳ css_computed_style *style = box->style;

	layout_find_dimensions(unit_len_ctx, available_width, viewport_height, box,
			style, &width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	ଯଦି (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	}

	box->width = layout_solve_width(box, available_width, width, lm, rm,
			max_width, min_width);
	box->height = height;

	ଯଦି (margin[ଶୀର୍ଷ_2] == AUTO)
		margin[ଶୀର୍ଷ_2] = 0;
	ଯଦି (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
ସ୍ଥିର ଶୂନ୍ୟ layout_block_add_scrollbar(ସଂରଚନା box *box, ପୂର୍ଣ୍ଣସଂଖ୍ୟା which)
{
	ଗଣନା css_overflow_e overflow_x, overflow_y;

	ପ୍ରତିଜ୍ଞା(box->type == BOX_BLOCK && (which == ଡାହାଣ_2 || which == BOTTOM));

	ଯଦି (box->style == ଖାଲି)
		ଫେରନ୍ତୁ;

	overflow_x = css_computed_overflow_x(box->style);
	overflow_y = css_computed_overflow_y(box->style);

	ଯଦି (which == BOTTOM &&
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		ଯଦି (box->height != AUTO &&
				(overflow_x == CSS_OVERFLOW_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_WIDTH;
		}

	} ଅନ୍ୟଥା ଯଦି (which == ଡାହାଣ_2 &&
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		ଗଣନା css_height_e htype;
		css_fixed height = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_height(box->style, &height, &hunit);

		ଯଦି (which == ଡାହାଣ_2 && box->width != AUTO &&
				htype == CSS_HEIGHT_SET &&
				(overflow_y == CSS_OVERFLOW_SCROLL ||
				box_vscrollbar_present(box))) {
			box->width -= SCROLLBAR_WIDTH;
			box->padding[ଡାହାଣ_2] += SCROLLBAR_WIDTH;
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
ସ୍ଥିର ଶୂନ୍ୟ layout_move_children(ସଂରଚନା box *box, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ପ୍ରତିଜ୍ଞା(box);

	ପାଇଁ (box = box->children; box; box = box->next) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_table(
		ସଂରଚନା box *table,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width,
		html_content *content)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା columns = table->columns;  /* total columns */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *row_span;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା *excess_y;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା table_width, min_width = 0, max_width = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା required_width = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, remainder = 0, ଗଣନା_2 = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା table_height = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା min_height = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା *xs;  /* array of column x positions */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା auto_width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା spare_width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା relative_sum = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା border_spacing_h = 0, border_spacing_v = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା spare_height;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା positioned_columns = 0;
	ସଂରଚନା box *containing_block = ଖାଲି;
	ସଂରଚନା box *c;
	ସଂରଚନା box *row;
	ସଂରଚନା box *row_group;
	ସଂରଚନା box **row_span_cell;
	ସଂରଚନା column *col;
	ଅଚଳ css_computed_style *style = table->style;
	ଗଣନା css_height_e htype;
	css_fixed ମୂଲ୍ୟ = 0;
	css_unit unit = CSS_UNIT_PX;

	ପ୍ରତିଜ୍ଞା(table->type == BOX_TABLE);
	ପ୍ରତିଜ୍ଞା(style);
	ପ୍ରତିଜ୍ଞା(table->children && table->children->children);
	ପ୍ରତିଜ୍ଞା(columns);

	/* allocate working buffers */
	col = malloc(columns * ଆକାର col[0]);
	excess_y = malloc(columns * ଆକାର excess_y[0]);
	row_span = malloc(columns * ଆକାର row_span[0]);
	row_span_cell = malloc(columns * ଆକାର row_span_cell[0]);
	xs = malloc((columns + 1) * ଆକାର xs[0]);
	ଯଦି (!col || !xs || !row_span || !excess_y || !row_span_cell) {
		ମୁକ୍ତ(col);
		ମୁକ୍ତ(excess_y);
		ମୁକ୍ତ(row_span);
		ମୁକ୍ତ(row_span_cell);
		ମୁକ୍ତ(xs);
		ଫେରନ୍ତୁ false;
	}

	memcpy(col, table->col, ଆକାର(col[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1, table,
			style, 0, 0, 0, 0, 0, 0, table->margin, table->padding,
			table->border);
	ପାଇଁ (row_group = table->children; row_group;
			row_group = row_group->next) {
		ପାଇଁ (row = row_group->children; row; row = row->next) {
			ପାଇଁ (c = row->children; c; c = c->next) {
				ଗଣନା css_overflow_e overflow_x;
				ଗଣନା css_overflow_e overflow_y;

				ପ୍ରତିଜ୍ଞା(c->style);
				table_used_border_for_cell(
						&content->unit_len_ctx, c);
				layout_find_dimensions(&content->unit_len_ctx,
						available_width, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				overflow_x = css_computed_overflow_x(c->style);
				overflow_y = css_computed_overflow_y(c->style);

				ଯଦି (overflow_x == CSS_OVERFLOW_SCROLL ||
						overflow_x ==
						CSS_OVERFLOW_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_WIDTH;
				}
				ଯଦି (overflow_y == CSS_OVERFLOW_SCROLL ||
						overflow_y ==
						CSS_OVERFLOW_AUTO) {
					c->padding[ଡାହାଣ_2] += SCROLLBAR_WIDTH;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	ଯଦି (css_computed_border_collapse(style) ==
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
	ଯଦି (css_computed_width_px(style, &content->unit_len_ctx,
			available_width, &table_width) == CSS_WIDTH_SET) {
		/* specified width includes border */
		table_width -= table->border[ବାମ_2].width +
				table->border[ଡାହାଣ_2].width;
		table_width = table_width < 0 ? 0 : table_width;

		auto_width = table_width;
	} ଅନ୍ୟଥା {
		table_width = AUTO;
		auto_width = available_width -
				((table->margin[ବାମ_2] == AUTO ? 0 :
						table->margin[ବାମ_2]) +
				 table->border[ବାମ_2].width +
				 table->padding[ବାମ_2] +
				 table->padding[ଡାହାଣ_2] +
				 table->border[ଡାହାଣ_2].width +
				 (table->margin[ଡାହାଣ_2] == AUTO ? 0 :
						table->margin[ଡାହାଣ_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_height(style, &ମୂଲ୍ୟ, &unit);
	ଯଦି (htype == CSS_HEIGHT_SET) {
		ଯଦି (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			ଯଦି (css_computed_position(table->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				ପ୍ରତିଜ୍ଞା(table->float_container);
				containing_block = table->float_container;
			} ଅନ୍ୟଥା ଯଦି (table->float_container &&
					css_computed_position(table->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(table->style) ==
						CSS_FLOAT_ବାମ ||
					css_computed_float(table->style) ==
						CSS_FLOAT_ଡାହାଣ)) {
				/* Table is a float */
				ପ୍ରତିଜ୍ଞା(table->parent && table->parent->parent &&
						table->parent->parent->parent);
				containing_block =
						table->parent->parent->parent;
			} ଅନ୍ୟଥା ଯଦି (table->parent && table->parent->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_block = table->parent;
			} ଅନ୍ୟଥା ଯଦି (table->parent && table->parent->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				ପ୍ରତିଜ୍ଞା(table->parent->parent);
				containing_block = table->parent->parent;
			}

			ଯଦି (containing_block) {
				css_fixed ignored = 0;

				htype = css_computed_height(
						containing_block->style,
						&ignored, &unit);
			}

			ଯଦି (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(table->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_HEIGHT_SET)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_height = FPCT_OF_INT_TOINT(ମୂଲ୍ୟ,
						containing_block->height);
			}
		} ଅନ୍ୟଥା {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_height = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					ମୂଲ୍ୟ, unit));
		}
	}

	/* calculate width required by cells */
	ପାଇଁ (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      table,
		      i,
		      ((ଅଚଳ ଅକ୍ଷର *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[col[i].type],
		      col[i].width,
		      col[i].min,
		      col[i].max);


		ଯଦି (col[i].positioned) {
			positioned_columns++;
			ଜାରି;
		} ଅନ୍ୟଥା ଯଦି (col[i].type == COLUMN_WIDTH_FIXED) {
			ଯଦି (col[i].width < col[i].min)
				col[i].width = col[i].max = col[i].min;
			ଅନ୍ୟଥା
				col[i].min = col[i].max = col[i].width;
			required_width += col[i].width;
		} ଅନ୍ୟଥା ଯଦି (col[i].type == COLUMN_WIDTH_PERCENT) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା width = col[i].width * auto_width / 100;
			required_width += col[i].min < width ? width :
					col[i].min;
		} ଅନ୍ୟଥା
			required_width += col[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_width);
	}
	required_width += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", table_width,
	      table->min_width, table->max_width, auto_width, required_width);

	ଯଦି (auto_width < required_width) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		ପାଇଁ (i = 0; i != columns; i++) {
			ଯଦି (col[i].type == COLUMN_WIDTH_RELATIVE)
				ଜାରି;
			ଯଦି (col[i].type == COLUMN_WIDTH_PERCENT) {
				col[i].max = auto_width * col[i].width / 100;
				ଯଦି (col[i].max < col[i].min)
					col[i].max = col[i].min;
			}
			min_width += col[i].min;
			max_width += col[i].max;
		}
	} ଅନ୍ୟଥା {
		/* take percentages exactly */
		ପାଇଁ (i = 0; i != columns; i++) {
			ଯଦି (col[i].type == COLUMN_WIDTH_RELATIVE)
				ଜାରି;
			ଯଦି (col[i].type == COLUMN_WIDTH_PERCENT) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା width = auto_width * col[i].width / 100;
				ଯଦି (width < col[i].min)
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
	ପାଇଁ (i = 0; i != columns; i++) {
		ଯଦି (col[i].type == COLUMN_WIDTH_RELATIVE)
			relative_sum += col[i].width;
		ଅନ୍ୟଥା ଯଦି (col[i].type == COLUMN_WIDTH_FIXED)
			spare_width -= col[i].width;
		ଅନ୍ୟଥା
			spare_width -= col[i].min;
	}
	spare_width -= (columns + 1) * border_spacing_h;
	ଯଦି (relative_sum != 0) {
		ଯଦି (spare_width < 0)
			spare_width = 0;
		ପାଇଁ (i = 0; i != columns; i++) {
			ଯଦି (col[i].type == COLUMN_WIDTH_RELATIVE) {
				col[i].min = ceil(col[i].max =
						(ଦଶମିକ) spare_width
						* (ଦଶମିକ) col[i].width
						/ relative_sum);
				min_width += col[i].min;
				max_width += col[i].max;
			}
		}
	}
	min_width += (columns + 1) * border_spacing_h;
	max_width += (columns + 1) * border_spacing_h;

	ଯଦି (auto_width <= min_width) {
		/* not enough space: minimise column widths */
		ପାଇଁ (i = 0; i < columns; i++) {
			col[i].width = col[i].min;
		}
		table_width = min_width;
	} ଅନ୍ୟଥା ଯଦି (max_width <= auto_width) {
		/* more space than maximum width */
		ଯଦି (table_width == AUTO) {
			/* for auto-width tables, make columns max width */
			ପାଇଁ (i = 0; i < columns; i++) {
				col[i].width = col[i].max;
			}
			table_width = max_width;
		} ଅନ୍ୟଥା {
			/* for fixed-width tables, distribute the extra space
			 * too */
			unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା flexible_columns = 0;
			ପାଇଁ (i = 0; i != columns; i++)
				ଯଦି (col[i].type != COLUMN_WIDTH_FIXED)
					flexible_columns++;
			ଯଦି (flexible_columns == 0) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra = (table_width - max_width) / columns;
				remainder = (table_width - max_width) -
						(extra * columns);
				ପାଇଁ (i = 0; i != columns; i++) {
					col[i].width = col[i].max + extra;
					ଗଣନା_2 -= remainder;
					ଯଦି (ଗଣନା_2 < 0) {
						col[i].width++;
						ଗଣନା_2 += columns;
					}
				}

			} ଅନ୍ୟଥା {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra = (table_width - max_width) /
						flexible_columns;
				remainder = (table_width - max_width) -
						(extra * flexible_columns);
				ପାଇଁ (i = 0; i != columns; i++)
					ଯଦି (col[i].type != COLUMN_WIDTH_FIXED) {
						col[i].width = col[i].max +
								extra;
						ଗଣନା_2 -= remainder;
						ଯଦି (ଗଣନା_2 < 0) {
							col[i].width++;
							ଗଣନା_2 += flexible_columns;
						}
					}
			}
		}
	} ଅନ୍ୟଥା {
		/* space between min and max: fill it exactly */
		ଦଶମିକ scale = (ଦଶମିକ) (auto_width - min_width) /
				(ଦଶମିକ) (max_width - min_width);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		ପାଇଁ (i = 0; i < columns; i++) {
			col[i].width = col[i].min + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (0.5 +
					(col[i].max - col[i].min) * scale);
		}
		table_width = auto_width;
	}

	xs[0] = x = border_spacing_h;
	ପାଇଁ (i = 0; i != columns; i++) {
		ଯଦି (!col[i].positioned)
			x += col[i].width + border_spacing_h;
		xs[i + 1] = x;
		row_span[i] = 0;
		excess_y[i] = 0;
		row_span_cell[i] = 0;
	}

	/* position cells */
	table_height = border_spacing_v;
	ପାଇଁ (row_group = table->children; row_group;
			row_group = row_group->next) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା row_group_height = 0;
		ପାଇଁ (row = row_group->children; row; row = row->next) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା row_height = 0;

			htype = css_computed_height(row->style, &ମୂଲ୍ୟ, &unit);
			ଯଦି (htype == CSS_HEIGHT_SET && unit != CSS_UNIT_PCT) {
				row_height = FIXTOINT(css_unit_len2device_px(
						row->style,
						&content->unit_len_ctx,
						ମୂଲ୍ୟ, unit));
			}
			ପାଇଁ (c = row->children; c; c = c->next) {
				ପ୍ରତିଜ୍ଞା(c->style);
				c->width = xs[c->start_column + c->columns] -
						xs[c->start_column] -
						border_spacing_h -
						c->border[ବାମ_2].width -
						c->padding[ବାମ_2] -
						c->padding[ଡାହାଣ_2] -
						c->border[ଡାହାଣ_2].width;
				c->float_children = 0;
				c->cached_place_below_level = 0;

				c->height = AUTO;
				ଯଦି (!layout_block_context(c, -1, content)) {
					ମୁକ୍ତ(col);
					ମୁକ୍ତ(excess_y);
					ମୁକ୍ତ(row_span);
					ମୁକ୍ତ(row_span_cell);
					ମୁକ୍ତ(xs);
					ଫେରନ୍ତୁ false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->height;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_height(c->style,
						&ମୂଲ୍ୟ, &unit);

				ଯଦି (htype == CSS_HEIGHT_SET &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					ପୂର୍ଣ୍ଣସଂଖ୍ୟା h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							ମୂଲ୍ୟ, unit));
					ଯଦି (c->height < h)
						c->height = h;
				}
				/* specified row height is treated as a minimum
				 */
				ଯଦି (c->height < row_height)
					c->height = row_height;
				c->x = xs[c->start_column] +
						c->border[ବାମ_2].width;
				c->y = c->border[ଶୀର୍ଷ_2].width;
				ପାଇଁ (i = 0; i != c->columns; i++) {
					row_span[c->start_column + i] = c->rows;
					excess_y[c->start_column + i] =
							c->border[ଶୀର୍ଷ_2].width +
							c->padding[ଶୀର୍ଷ_2] +
							c->height +
							c->padding[BOTTOM] +
							c->border[BOTTOM].width;
					row_span_cell[c->start_column + i] = 0;
				}
				row_span_cell[c->start_column] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[ଶୀର୍ଷ_2].width -
						c->padding[ଶୀର୍ଷ_2] -
						c->height -
						c->border[BOTTOM].width;
			}
			ପାଇଁ (i = 0; i != columns; i++)
				ଯଦି (row_span[i] != 0)
					row_span[i]--;
				ଅନ୍ୟଥା
					row_span_cell[i] = 0;
			ଯଦି (row->next || row_group->next) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				ପାଇଁ (i = 0; i != columns; i++)
					ଯଦି (row_span[i] == 0 && row_height <
							excess_y[i])
						row_height = excess_y[i];
			} ଅନ୍ୟଥା {
				/* except in the last row */
				ପାଇଁ (i = 0; i != columns; i++)
					ଯଦି (row_height < excess_y[i])
						row_height = excess_y[i];
			}
			ପାଇଁ (i = 0; i != columns; i++) {
				ଯଦି (row_height < excess_y[i])
					excess_y[i] -= row_height;
				ଅନ୍ୟଥା
					excess_y[i] = 0;
				ଯଦି (row_span_cell[i] != 0)
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
	ପାଇଁ (row_group = table->children; row_group;
			row_group = row_group->next) {
		ପାଇଁ (row = row_group->children; row; row = row->next) {
			ପାଇଁ (c = row->children; c; c = c->next) {
				ଗଣନା css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				spare_height = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->height - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &ମୂଲ୍ୟ, &unit);

				ବାଛ (vertical_align) {
				ସ୍ଥିତି CSS_VERTICAL_ALIGN_SUB:
				ସ୍ଥିତି CSS_VERTICAL_ALIGN_SUPER:
				ସ୍ଥିତି CSS_VERTICAL_ALIGN_TEXT_ଶୀର୍ଷ:
				ସ୍ଥିତି CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				ସ୍ଥିତି CSS_VERTICAL_ALIGN_SET:
				ସ୍ଥିତି CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				ସ୍ଥିତି CSS_VERTICAL_ALIGN_ଶୀର୍ଷ:
					ଭାଙ୍ଗ;
				ସ୍ଥିତି CSS_VERTICAL_ALIGN_ମଧ୍ୟ:
					c->padding[ଶୀର୍ଷ_2] += spare_height / 2;
					c->padding[BOTTOM] -= spare_height / 2;
					layout_move_children(c, 0,
							spare_height / 2);
					ଭାଙ୍ଗ;
				ସ୍ଥିତି CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[ଶୀର୍ଷ_2] += spare_height;
					c->padding[BOTTOM] -= spare_height;
					layout_move_children(c, 0,
							spare_height);
					ଭାଙ୍ଗ;
				ସ୍ଥିତି CSS_VERTICAL_ALIGN_INHERIT:
					ପ୍ରତିଜ୍ଞା(0);
					ଭାଙ୍ଗ;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	ଯଦି (table->margin[ଶୀର୍ଷ_2] == AUTO)
		table->margin[ଶୀର୍ଷ_2] = 0;
	ଯଦି (table->margin[BOTTOM] == AUTO)
		table->margin[BOTTOM] = 0;

	ମୁକ୍ତ(col);
	ମୁକ୍ତ(excess_y);
	ମୁକ୍ତ(row_span);
	ମୁକ୍ତ(row_span_cell);
	ମୁକ୍ତ(xs);

	table->width = table_width;
	table->height = table_height;

	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର bool layout_apply_minmax_height(
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ସଂରଚନା box *box,
		ସଂରଚନା box *container)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା h;
	ସଂରଚନା box *containing_block = ଖାଲି;
	bool updated = false;

	/* Find containing block for percentage heights */
	ଯଦି (box->style != ଖାଲି && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		ପ୍ରତିଜ୍ଞା(container);
		containing_block = container;
	} ଅନ୍ୟଥା ଯଦି (box->float_container && box->style != ଖାଲି &&
			(css_computed_float(box->style) == CSS_FLOAT_ବାମ ||
			 css_computed_float(box->style) == CSS_FLOAT_ଡାହାଣ)) {
		/* Box is a float */
		ପ୍ରତିଜ୍ଞା(box->parent && box->parent->parent &&
				box->parent->parent->parent);
		containing_block = box->parent->parent->parent;
	} ଅନ୍ୟଥା ଯଦି (box->parent && box->parent->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_block = box->parent;
	} ଅନ୍ୟଥା ଯଦି (box->parent && box->parent->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		ପ୍ରତିଜ୍ଞା(box->parent->parent);
		containing_block = box->parent->parent;
	}

	ଯଦି (box->style) {
		ଗଣନା css_height_e htype = CSS_HEIGHT_AUTO;
		css_fixed ମୂଲ୍ୟ = 0;
		css_unit unit = CSS_UNIT_PX;

		ଯଦି (containing_block) {
			htype = css_computed_height(containing_block->style,
					&ମୂଲ୍ୟ, &unit);
		}

		/* max-height */
		ଯଦି (css_computed_max_height(box->style, &ମୂଲ୍ୟ, &unit) ==
				CSS_MAX_HEIGHT_SET) {
			ଯଦି (unit == CSS_UNIT_PCT) {
				ଯଦି (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(ମୂଲ୍ୟ,
						containing_block->height);
					ଯଦି (h < box->height) {
						box->height = h;
						updated = true;
					}
				}
			} ଅନ୍ୟଥା {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ମୂଲ୍ୟ, unit));
				ଯଦି (h < box->height) {
					box->height = h;
					updated = true;
				}
			}
		}

		/* min-height */
		ଯଦି (ns_computed_min_height(box->style, &ମୂଲ୍ୟ, &unit) ==
				CSS_MIN_HEIGHT_SET) {
			ଯଦି (unit == CSS_UNIT_PCT) {
				ଯଦି (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(ମୂଲ୍ୟ,
						containing_block->height);
					ଯଦି (h > box->height) {
						box->height = h;
						updated = true;
					}
				}
			} ଅନ୍ୟଥା {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ମୂଲ୍ୟ, unit));
				ଯଦି (h > box->height) {
					box->height = h;
					updated = true;
				}
			}
		}
	}
	ଫେରନ୍ତୁ updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
ସ୍ଥିର bool layout_block_object(ସଂରଚନା box *block)
{
	ପ୍ରତିଜ୍ଞା(block);
	ପ୍ରତିଜ୍ଞା(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_TABLE ||
			block->type == BOX_TABLE_CELL);
	ପ୍ରତିଜ୍ଞା(block->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", block,
	      hlcache_handle_get_url(block->object), block->width);

	ଯଦି (content_can_reformat(block->object)) {
		content_reformat(block->object, false, block->width, 1);
	} ଅନ୍ୟଥା {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
ସ୍ଥିର ଶୂନ୍ୟ add_float_to_container(ସଂରଚନା box *cont, ସଂରଚନା box *b)
{
	ସଂରଚନା box *box = cont->float_children;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା b_bottom = b->y + b->height;

	ପ୍ରତିଜ୍ଞା(b->type == BOX_FLOAT_ବାମ || b->type == BOX_FLOAT_ଡାହାଣ);

	ଯଦି (box == ଖାଲି) {
		/* No other float children */
		b->next_float = ଖାଲି;
		cont->float_children = b;
		ଫେରନ୍ତୁ;
	} ଅନ୍ୟଥା ଯଦି (b_bottom >= box->y + box->height) {
		/* Goes at start of list */
		b->next_float = cont->float_children;
		cont->float_children = b;
	} ଅନ୍ୟଥା {
		ସଂରଚନା box *prev = ଖାଲି;
		ଯେତେବେଳେ (box != ଖାଲି && b_bottom < box->y + box->height) {
			prev = box;
			box = box->next_float;
		}
		ଯଦି (prev != ଖାଲି) {
			b->next_float = prev->next_float;
			prev->next_float = b;
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
ସ୍ଥିର bool
layout_text_box_split(html_content *content,
		      plot_font_style_t *fstyle,
		      ସଂରଚନା box *split_box,
		      size_t new_length,
		      ପୂର୍ଣ୍ଣସଂଖ୍ୟା new_width)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା space_width = split_box->space;
	ସଂରଚନା box *c2;
	ଅଚଳ ସଂରଚନା gui_layout_table *font_func = content->font_func;
	bool space = (split_box->text[new_length] == ' ');
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା used_length = new_length + (space ? 1 : 0);

	ଯଦି ((space && space_width == 0) || space_width == UNKNOWN_WIDTH) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->width(fstyle, " ", 1, &space_width);
	}

	ଯଦି (split_box->space == UNKNOWN_WIDTH)
		split_box->space = space_width;
	ଯଦି (!space)
		space_width = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, split_box, ଆକାର *c2);
	ଯଦି (!c2)
		ଫେରନ୍ତୁ false;
	c2->flags |= CLONE;

	/* Set remaining text in c2 */
	c2->text += used_length;

	/* Set c2 according to the remaining text */
	c2->width -= new_width + space_width;
	c2->flags &= ~MEASURED; /* width has been estimated */
	c2->length = split_box->length - used_length;

	/* Update split_box for its reduced text */
	split_box->width = new_width;
	split_box->flags |= MEASURED;
	split_box->length = new_length;
	split_box->space = space_width;

	/* Insert c2 into box list */
	c2->next = split_box->next;
	split_box->next = c2;
	c2->prev = split_box;
	ଯଦି (c2->next)
		c2->next->prev = c2;
	ଅନ୍ୟଥା
		c2->parent->last = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      split_box,
	      split_box->length,
	      (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)split_box->length,
	      split_box->text);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->length,
	      (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)c2->length,
	      c2->text);

	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର ଶୂନ୍ୟ
layout_float_find_dimensions(
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width,
		ଅଚଳ css_computed_style *style,
		ସଂରଚନା box *box)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, height, max_width, min_width, max_height, min_height;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା *margin = box->margin;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା *padding = box->padding;
	ସଂରଚନା box_border *border = box->border;
	ଗଣନା css_overflow_e overflow_x = css_computed_overflow_x(style);
	ଗଣନା css_overflow_e overflow_y = css_computed_overflow_y(style);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା scrollbar_width_x =
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା scrollbar_width_y =
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;

	layout_find_dimensions(unit_len_ctx, available_width, -1, box, style,
			&width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	ଯଦି (margin[ବାମ_2] == AUTO)
		margin[ବାମ_2] = 0;
	ଯଦି (margin[ଡାହାଣ_2] == AUTO)
		margin[ଡାହାଣ_2] = 0;

	ଯଦି (box->gadget == ଖାଲି) {
		padding[ଡାହାଣ_2] += scrollbar_width_y;
		padding[BOTTOM] += scrollbar_width_x;
	}

	ଯଦି (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	} ଅନ୍ୟଥା ଯଦି (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed size = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		ପ୍ରତିଜ୍ଞା(box->style);

		ଯଦି (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			ଯଦି (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			ଯଦି (box->gadget->type == GADGET_FILE &&
					height == AUTO) {
				size = FLTTOFIX(1.5);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
		ଯଦି (box->gadget->type == GADGET_TEXTAREA) {
			ଯଦି (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			ଯଦି (height == AUTO) {
				size = INTTOFIX(4);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
	} ଅନ୍ୟଥା ଯଦି (width == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		width = min(max(box->min_width, available_width),
				box->max_width);

		/* width includes margin, borders and padding */
		ଯଦି (width == available_width) {
			width -= box->margin[ବାମ_2] + box->border[ବାମ_2].width +
					box->padding[ବାମ_2] +
					box->padding[ଡାହାଣ_2] +
					box->border[ଡାହାଣ_2].width +
					box->margin[ଡାହାଣ_2];
		} ଅନ୍ୟଥା {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା fixed = 0;
			ଦଶମିକ frac = 0;
			calculate_mbp_width(unit_len_ctx, box->style, ବାମ_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_width(unit_len_ctx, box->style, ଡାହାଣ_2,
					true, true, true, &fixed, &frac);
			ଯଦି (fixed < 0)
				fixed = 0;

			width -= fixed;
		}

		ଯଦି (max_width >= 0 && width > max_width) width = max_width;
		ଯଦି (min_width >  0 && width < min_width) width = min_width;

	} ଅନ୍ୟଥା {
		ଯଦି (max_width >= 0 && width > max_width) width = max_width;
		ଯଦି (min_width >  0 && width < min_width) width = min_width;
		width -= scrollbar_width_y;
	}

	box->width = width;
	box->height = height;

	ଯଦି (margin[ଶୀର୍ଷ_2] == AUTO)
		margin[ଶୀର୍ଷ_2] = 0;
	ଯଦି (margin[BOTTOM] == AUTO)
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
ସ୍ଥିର bool layout_float(ସଂରଚନା box *b, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, html_content *content)
{
	ପ୍ରତିଜ୍ଞା(b->type == BOX_TABLE ||
	       b->type == BOX_BLOCK ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_find_dimensions(&content->unit_len_ctx, width, b->style, b);
	ଯଦି (b->type == BOX_TABLE || b->type == BOX_INLINE_FLEX) {
		ଯଦି (b->type == BOX_TABLE) {
			ଯଦି (!layout_table(b, width, content))
				ଫେରନ୍ତୁ false;
		} ଅନ୍ୟଥା {
			ଯଦି (!layout_flex(b, width, content))
				ଫେରନ୍ତୁ false;
		}
		ଯଦି (b->margin[ବାମ_2] == AUTO)
			b->margin[ବାମ_2] = 0;
		ଯଦି (b->margin[ଡାହାଣ_2] == AUTO)
			b->margin[ଡାହାଣ_2] = 0;
		ଯଦି (b->margin[ଶୀର୍ଷ_2] == AUTO)
			b->margin[ଶୀର୍ଷ_2] = 0;
		ଯଦି (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} ଅନ୍ୟଥା {
		ଫେରନ୍ତୁ layout_block_context(b, -1, content);
	}
	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର ଶୂନ୍ୟ
place_float_below(ସଂରଚନା box *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ସଂରଚନା box *cont)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x0, x1, yy;
	ସଂରଚନା box *ବାମ;
	ସଂରଚନା box *ଡାହାଣ;

	yy = y > cont->cached_place_below_level ?
			y : cont->cached_place_below_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      width, cx, y, cont);

	କର {
		y = yy;
		x0 = cx;
		x1 = cx + width;
		find_sides(cont->float_children, y, y + c->height, &x0, &x1,
				&ବାମ, &ଡାହାଣ);
		ଯଦି (ବାମ != 0 && ଡାହାଣ != 0) {
			yy = (ବାମ->y + ବାମ->height <
					ଡାହାଣ->y + ଡାହାଣ->height ?
					ବାମ->y + ବାମ->height :
					ଡାହାଣ->y + ଡାହାଣ->height);
		} ଅନ୍ୟଥା ଯଦି (ବାମ == 0 && ଡାହାଣ != 0) {
			yy = ଡାହାଣ->y + ଡାହାଣ->height;
		} ଅନ୍ୟଥା ଯଦି (ବାମ != 0 && ଡାହାଣ == 0) {
			yy = ବାମ->y + ବାମ->height;
		}
	} ଯେତେବେଳେ ((ବାମ != 0 || ଡାହାଣ != 0) && (c->width > x1 - x0));

	ଯଦି (c->type == BOX_FLOAT_ବାମ) {
		c->x = x0;
	} ଅନ୍ୟଥା {
		c->x = x1 - c->width;
	}
	c->y = y;
	cont->cached_place_below_level = y;
}


/**
 * Calculate line height from a style.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି_height(
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ଅଚଳ css_computed_style *style)
{
	ଗଣନା css_ଧାଡ଼ି_height_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed ଧାଡ଼ି_height;

	ପ୍ରତିଜ୍ଞା(style);

	lhtype = css_computed_ଧାଡ଼ି_height(style, &lhvalue, &lhunit);
	ଯଦି (lhtype == CSS_ଧାଡ଼ି_HEIGHT_NORMAL) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_ଧାଡ଼ି_HEIGHT_ସଂଖ୍ୟା;
	}

	ଯଦି (lhtype == CSS_ଧାଡ଼ି_HEIGHT_ସଂଖ୍ୟା ||
			lhunit == CSS_UNIT_PCT) {
		ଧାଡ଼ି_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		ଯଦି (lhtype != CSS_ଧାଡ଼ି_HEIGHT_ସଂଖ୍ୟା)
			ଧାଡ଼ି_height = FDIV(ଧାଡ଼ି_height, F_100);
	} ଅନ୍ୟଥା {
		ପ୍ରତିଜ୍ଞା(lhunit != CSS_UNIT_PCT);

		ଧାଡ଼ି_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	ଫେରନ୍ତୁ FIXTOINT(ଧାଡ଼ି_height);
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
ସ୍ଥିର bool
layout_ଧାଡ଼ି(ସଂରଚନା box *first,
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା *width,
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା *y,
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cx,
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cy,
	    ସଂରଚନା box *cont,
	    bool indent,
	    bool has_text_children,
	    html_content *content,
	    ସଂରଚନା box **next_box)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା height, used_height;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x0 = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x1 = *width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, h, x_previous;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fy = cy;
	ସଂରଚନା box *ବାମ;
	ସଂରଚନା box *ଡାହାଣ;
	ସଂରଚନା box *b;
	ସଂରଚନା box *split_box = 0;
	ସଂରଚନା box *d;
	ସଂରଚନା box *br_box = 0;
	bool move_y = false;
	bool place_below = false;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା space_before = 0, space_after = 0;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା inline_ଗଣନା = 0;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ଅଚଳ ସଂରଚନା gui_layout_table *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      first,
	      (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)first->length,
	      first->text,
	      *width,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	find_sides(cont->float_children, cy, cy, &x0, &x1, &ବାମ, &ଡାହାଣ);
	x0 -= cx;
	x1 -= cx;

	ଯଦି (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	ଯଦି (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	ଯଦି (has_text_children || first->parent->parent->gadget)
		used_height = height = ଧାଡ଼ି_height(&content->unit_len_ctx,
				first->parent->parent->style);
	ଅନ୍ୟଥା
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		used_height = height = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	ପାଇଁ (x = 0, b = first; x <= x1 - x0 && b != 0; b = b->next) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା min_width, max_width, min_height, max_height;

		ପ୍ରତିଜ୍ଞା(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		ଯଦି (b->type == BOX_BR)
			ଭାଙ୍ଗ;

		ଯଦି (lh__box_is_float_box(b))
			ଜାରି;
		ଯଦି (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			ଜାରି;

		ପ୍ରତିଜ୍ଞା(b->style != ଖାଲି);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		ଯଦି (b->type == BOX_INLINE_BLOCK ||
		    b->type == BOX_INLINE_FLEX) {
			ଯଦି (b->max_width != UNKNOWN_WIDTH)
				ଯଦି (!layout_float(b, *width, content))
					ଫେରନ୍ତୁ false;
			h = b->border[ଶୀର୍ଷ_2].width + b->padding[ଶୀର୍ଷ_2] + b->height +
					b->padding[BOTTOM] +
					b->border[BOTTOM].width;
			ଯଦି (height < h)
				height = h;
			x += b->margin[ବାମ_2] + b->border[ବାମ_2].width +
					b->padding[ବାମ_2] + b->width +
					b->padding[ଡାହାଣ_2] +
					b->border[ଡାହାଣ_2].width +
					b->margin[ଡାହାଣ_2];
			space_after = 0;
			ଜାରି;
		}

		ଯଦି (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_find_dimensions(&content->unit_len_ctx,
					*width, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			ପାଇଁ (i = 0; i != 4; i++)
				ଯଦି (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[ବାମ_2] + b->border[ବାମ_2].width +
					b->padding[ବାମ_2];
			ଯଦି (b->inline_end) {
				b->inline_end->margin[ଡାହାଣ_2] = b->margin[ଡାହାଣ_2];
				b->inline_end->padding[ଡାହାଣ_2] =
						b->padding[ଡାହାଣ_2];
				b->inline_end->border[ଡାହାଣ_2] =
						b->border[ଡାହାଣ_2];
			} ଅନ୍ୟଥା {
				x += b->padding[ଡାହାଣ_2] +
						b->border[ଡାହାଣ_2].width +
						b->margin[ଡାହାଣ_2];
			}
		} ଅନ୍ୟଥା ଯଦି (b->type == BOX_INLINE_END) {
			b->width = 0;
			ଯଦି (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[ଡାହାଣ_2] + b->border[ଡାହାଣ_2].width +
					b->margin[ଡାହାଣ_2];
			ଜାରି;
		}

		ଯଦି (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->height = ଧାଡ଼ି_height(&content->unit_len_ctx,
					b->style ? b->style :
					b->parent->parent->style);
			ଯଦି (height < b->height)
				height = b->height;

			ଯଦି (!b->text) {
				b->width = 0;
				space_after = 0;
				ଜାରି;
			}

			ଯଦି (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				ଯଦି (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_maxwidth = 0;
					ସଂରଚନା form_option *o;

					ପାଇଁ (o = b->parent->parent->gadget->
							data.select.ବସ୍ତୁଗୁଡ଼ିକ; o;
							o = o->next) {
						ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						ଯଦି (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}
					b->width = opt_maxwidth;
					ଯଦି (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;
				} ଅନ୍ୟଥା {
					font_func->width(&fstyle, b->text,
							b->length, &b->width);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			ଯଦି (b->text && (x + b->width < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->next) {
				font_func->width(&fstyle, b->text,
						 b->length, &b->width);
				b->flags |= MEASURED;
			}

			x += b->width;
			ଯଦି (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			ଜାରି;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		ପ୍ରତିଜ୍ଞା(b->style);

		layout_find_dimensions(&content->unit_len_ctx,
				*width, -1, b, b->style,
				&b->width, &b->height,
				&max_width, &min_width,
				&max_height, &min_height,
				ଖାଲି, ଖାଲି, ଖାଲି);

		ଯଦି (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->width, &b->height,
					min_width, max_width,
					min_height, max_height);
		} ଅନ୍ୟଥା ଯଦି (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			ଯଦି (b->width == AUTO)
				b->width = 400;
			ଯଦି (b->height == AUTO)
				b->height = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} ଅନ୍ୟଥା {
			/* form control with no object */
			ଯଦି (b->width == AUTO)
				b->width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			ଯଦି (b->height == AUTO)
				b->height = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		ଯଦି (b->object && content_can_reformat(b->object) &&
				b->width !=
				content_get_available_width(b->object)) {
			css_fixed ମୂଲ୍ୟ = 0;
			css_unit unit = CSS_UNIT_PX;
			ଗଣନା css_height_e htype = css_computed_height(b->style,
					&ମୂଲ୍ୟ, &unit);

			content_reformat(b->object, false, b->width, b->height);

			ଯଦି (htype == CSS_HEIGHT_AUTO)
				b->height = content_get_height(b->object);
		}

		ଯଦି (height < b->height)
			height = b->height;

		x += b->width;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *width;
	find_sides(cont->float_children, cy, cy + height, &x0, &x1,
			&ବାମ, &ଡାହାଣ);
	x0 -= cx;
	x1 -= cx;

	ଯଦି (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	ଯଦି (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	ପାଇଁ (x = x_previous = 0, b = first; x <= x1 - x0 && b; b = b->next) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		ଯଦି (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} ଅନ୍ୟଥା ଯଦି (lh__box_is_inline_flow(b)) {
			ପ୍ରତିଜ୍ଞା(b->width != UNKNOWN_WIDTH);

			x_previous = x;
			x += space_after;
			b->x = x;

			ଯଦି ((b->type == BOX_INLINE && !b->inline_end) ||
					b->type == BOX_INLINE_BLOCK ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[ବାମ_2] + b->border[ବାମ_2].width;
				x = b->x + b->padding[ବାମ_2] + b->width +
						b->padding[ଡାହାଣ_2] +
						b->border[ଡାହାଣ_2].width +
						b->margin[ଡାହାଣ_2];
			} ଅନ୍ୟଥା ଯଦି (b->type == BOX_INLINE) {
				b->x += b->margin[ବାମ_2] + b->border[ବାମ_2].width;
				x = b->x + b->padding[ବାମ_2] + b->width;
			} ଅନ୍ୟଥା ଯଦି (b->type == BOX_INLINE_END) {
				b->height = b->inline_end->height;
				x += b->padding[ଡାହାଣ_2] +
						b->border[ଡାହାଣ_2].width +
						b->margin[ଡାହାଣ_2];
			} ଅନ୍ୟଥା {
				x += b->width;
			}

			space_before = space_after;
			ଯଦି (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			ଅନ୍ୟଥା ଯଦି (b->text || b->type == BOX_INLINE_END) {
				ଯଦି (b->space == UNKNOWN_WIDTH) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} ଅନ୍ୟଥା {
				space_after = 0;
			}
			split_box = b;
			move_y = true;
			inline_ଗଣନା++;
		} ଅନ୍ୟଥା ଯଦି (b->type == BOX_BR) {
			b->x = x;
			b->width = 0;
			br_box = b;
			b = b->next;
			split_box = 0;
			move_y = true;
			ଭାଙ୍ଗ;

		} ଅନ୍ୟଥା {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_place_below_level = 0;
			b->float_container = d->float_container = cont;

			ଯଦି (!layout_float(d, *width, content))
				ଫେରନ୍ତୁ false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[ଶୀର୍ଷ_2],
			      d->border[ଶୀର୍ଷ_2].width);

			d->x = d->margin[ବାମ_2] + d->border[ବାମ_2].width;
			d->y = d->margin[ଶୀର୍ଷ_2] + d->border[ଶୀର୍ଷ_2].width;
			b->width = d->margin[ବାମ_2] + d->border[ବାମ_2].width +
					d->padding[ବାମ_2] + d->width +
					d->padding[ଡାହାଣ_2] +
					d->border[ଡାହାଣ_2].width +
					d->margin[ଡାହାଣ_2];
			b->height = d->margin[ଶୀର୍ଷ_2] + d->border[ଶୀର୍ଷ_2].width +
					d->padding[ଶୀର୍ଷ_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];

			ଯଦି (b->width > (x1 - x0) - x)
				place_below = true;
			ଯଦି (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ବାମ && ବାମ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ଡାହାଣ &&
						ଡାହାଣ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						ବାମ == 0 && ଡାହାଣ == 0)) &&
					(!place_below ||
					(ବାମ == 0 && ଡାହାଣ == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_place_below_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				ଯଦି (b->type == BOX_FLOAT_ବାମ) {
					b->x = cx + x0;
					ଯଦି (b->width > 0)
						x0 += b->width;
					ବାମ = b;
				} ଅନ୍ୟଥା {
					b->x = cx + x1 - b->width;
					ଯଦି (b->width > 0)
						x1 -= b->width;
					ଡାହାଣ = b;
				}
				b->y = cy;
			} ଅନ୍ୟଥା {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_place_below_level) ?
						fcy :
						cont->cached_place_below_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + height : fy;

				place_float_below(b, *width, cx, fy, cont);
				fy = b->y;
				ଯଦି (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ବାମ && ବାମ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ଡାହାଣ &&
						ଡାହାଣ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(ବାମ != 0 || ଡାହାଣ != 0)))) {
					/* to be cleared below existing
					 * floats */
					ଯଦି (b->type == BOX_FLOAT_ବାମ)
						b->x = cx;
					ଅନ୍ୟଥା
						b->x = cx + *width - b->width;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					ଯଦି (fcy > cont->clear_level)
						cont->clear_level = fcy;
					ଯଦି (b->y < fcy)
						b->y = fcy;
				}
				ଯଦି (b->type == BOX_FLOAT_ବାମ)
					ବାମ = b;
				ଅନ୍ୟଥା
					ଡାହାଣ = b;
			}
			add_float_to_container(cont, b);

			split_box = 0;
		}
	}

	ଯଦି (x1 - x0 < x && split_box) {
		/* the last box went over the end */
		size_t split = 0;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା w;
		bool no_wrap = css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_previous;

		ଯଦି (!no_wrap &&
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
					 split_box->length,
					 x1 - x0 - x - space_before,
					 &split,
					 &w);
		}

		/* split == 0 implies that text can't be split */

		ଯଦି (split == 0)
			w = split_box->width;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      split_box,
		      (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)split_box->length,
		      split_box->text,
		      split,
		      w,
		      ବାମ,
		      ଡାହାଣ,
		      inline_ଗଣନା);

		ଯଦି ((split == 0 || x1 - x0 <= x + space_before + w) &&
				!ବାମ && !ଡାହାଣ && inline_ଗଣନା == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			ଯଦି (split == 0 || split == split_box->length) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = split_box->next;
			} ଅନ୍ୟଥା {
				/* cut off first word for this line */
				ଯଦି (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					ଫେରନ୍ତୁ false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} ଅନ୍ୟଥା ଯଦି ((split == 0 || x1 - x0 <= x + space_before + w) &&
				inline_ଗଣନା == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			ପ୍ରତିଜ୍ଞା(ବାମ || ଡାହାଣ);
			used_height = 0;
			ଯଦି (ବାମ) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      ବାମ->y,
				      ବାମ->height);

				used_height = ବାମ->y + ବାମ->height - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      used_height);

			}
			ଯଦି (ଡାହାଣ && used_height <
					ଡାହାଣ->y + ଡାହାଣ->height - cy + 1)
				used_height = ଡାହାଣ->y + ଡାହାଣ->height - cy + 1;

			ଯଦି (used_height < 0)
				used_height = 0;

			b = split_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} ଅନ୍ୟଥା ଯଦି (split == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = split_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} ଅନ୍ୟଥା {
			/* fit as many words as possible */
			ପ୍ରତିଜ୍ଞା(split != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)split_box->length, split_box->text,
			      x1 - x0, split, w);

			ଯଦି (split != split_box->length) {
				ଯଦି (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					ଫେରନ୍ତୁ false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	ବାଛ (css_computed_text_align(first->parent->parent->style)) {
	ସ୍ଥିତି CSS_TEXT_ALIGN_ଡାହାଣ:
	ସ୍ଥିତି CSS_TEXT_ALIGN_LIBCSS_ଡାହାଣ:
		x0 = x1 - x;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TEXT_ALIGN_CENTER:
	ସ୍ଥିତି CSS_TEXT_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TEXT_ALIGN_ବାମ:
	ସ୍ଥିତି CSS_TEXT_ALIGN_LIBCSS_ବାମ:
	ସ୍ଥିତି CSS_TEXT_ALIGN_JUSTIFY:
		/* leave on left */
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CSS_TEXT_ALIGN_DEFAULT:
		/* None; consider text direction */
		ବାଛ (css_computed_direction(first->parent->parent->style)) {
		ସ୍ଥିତି CSS_DIRECTION_LTR:
			/* leave on left */
			ଭାଙ୍ଗ;
		ସ୍ଥିତି CSS_DIRECTION_RTL:
			x0 = x1 - x;
			ଭାଙ୍ଗ;
		}
		ଭାଙ୍ଗ;
	}

	ପାଇଁ (d = first; d != b; d = d->next) {
		d->flags &= ~NEW_ଧାଡ଼ି;

		ଯଦି (d->type == BOX_INLINE_BLOCK &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			ଜାରି;
		} ଅନ୍ୟଥା ଯଦି ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[ଶୀର୍ଷ_2];

			ଯଦି (d->type == BOX_TEXT && d->height > used_height) {
				/* text */
				used_height = d->height;
			}
		} ଅନ୍ୟଥା ଯଦି ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_BLOCK) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[ଶୀର୍ଷ_2].width + d->margin[ଶୀର୍ଷ_2];
			h = d->margin[ଶୀର୍ଷ_2] + d->border[ଶୀର୍ଷ_2].width +
					d->padding[ଶୀର୍ଷ_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];
			ଯଦି (used_height < h)
				used_height = h;
		}
	}

	first->flags |= NEW_ଧାଡ଼ି;

	ପ୍ରତିଜ୍ଞା(b != first || (move_y && 0 < used_height && (ବାମ || ଡାହାଣ)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	ପାଇଁ (d = first; d != b; d = d->next) {
		ଯଦି ((d->type == BOX_INLINE && d->inline_end) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			css_fixed ମୂଲ୍ୟ = 0;
			css_unit unit = CSS_UNIT_PX;
			ବାଛ (css_computed_vertical_align(d->style, &ମୂଲ୍ୟ,
					&unit)) {
			ସ୍ଥିତି CSS_VERTICAL_ALIGN_SUPER:
			ସ୍ଥିତି CSS_VERTICAL_ALIGN_ଶୀର୍ଷ:
			ସ୍ଥିତି CSS_VERTICAL_ALIGN_TEXT_ଶୀର୍ଷ:
				/* already at top */
				ଭାଙ୍ଗ;
			ସ୍ଥିତି CSS_VERTICAL_ALIGN_SUB:
			ସ୍ଥିତି CSS_VERTICAL_ALIGN_BOTTOM:
			ସ୍ଥିତି CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				d->y += used_height - d->height;
				ଭାଙ୍ଗ;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ସ୍ଥିତି CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (used_height - d->height);
				ଭାଙ୍ଗ;
			}
		}
	}

	/* handle clearance for br */
	ଯଦି (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		ଯଦି (used_height < clear_y - cy)
			used_height = clear_y - cy;
	}

	ଯଦି (move_y)
		*y += used_height;
	*next_box = b;
	*width = x; /* return actual width */
	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର bool layout_inline_container(ସଂରଚନା box *inline_container, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width,
		ସଂରଚନା box *cont, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cy, html_content *content)
{
	bool first_ଧାଡ଼ି = true;
	bool has_text_children;
	ସଂରଚନା box *c, *next;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା y = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା curwidth,maxwidth = width;

	ପ୍ରତିଜ୍ଞା(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      width,
	      cont,
	      cx,
	      cy);


	has_text_children = false;
	ପାଇଁ (c = inline_container->children; c; c = c->next) {
		bool is_pre = false;

		ଯଦି (c->style) {
			ଗଣନା css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_ଧାଡ଼ି ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		ଯଦି ((lh__box_is_object(c) == false &&
				c->text && (c->length || is_pre)) ||
				c->type == BOX_BR)
			has_text_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	ପାଇଁ (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->width;
		ଯଦି (!layout_ଧାଡ଼ି(c, &curwidth, &y, cx, cy + y, cont, first_ଧାଡ଼ି,
				has_text_children, content, &next))
			ଫେରନ୍ତୁ false;
		maxwidth = max(maxwidth,curwidth);
		c = next;
		first_ଧାଡ଼ି = false;
	}

	inline_container->width = maxwidth;
	inline_container->height = y;

	ଫେରନ୍ତୁ true;
}


/* Documented in layout_intertnal.h */
bool layout_block_context(
		ସଂରଚନା box *block,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା viewport_height,
		html_content *content)
{
	ସଂରଚନା box *box;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cx, cy;  /**< current coordinates */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_pos_margin = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_neg_margin = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା y = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା lm, rm;
	ସଂରଚନା box *margin_collapse = ଖାଲି;
	bool in_margin = false;
	css_fixed gadget_size;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	ପ୍ରତିଜ୍ଞା(block->type == BOX_BLOCK ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX);
	ପ୍ରତିଜ୍ଞା(block->width != UNKNOWN_WIDTH);
	ପ୍ରତିଜ୍ଞା(block->width != AUTO);

	block->float_children = ଖାଲି;
	block->cached_place_below_level = 0;
	block->clear_level = 0;

	/* special case if the block contains an object */
	ଯଦି (block->object) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା temp_width = block->width;
		ଯଦି (!layout_block_object(block))
			ଫେରନ୍ତୁ false;
		layout_get_object_dimensions(block, &temp_width,
				&block->height, INT_MIN, INT_MAX,
				INT_MIN, INT_MAX);
		ଫେରନ୍ତୁ true;
	} ଅନ୍ୟଥା ଯଦି (block->flags & REPLACE_DIM) {
		ଫେରନ୍ତୁ true;
	}

	/* special case if the block contains an radio button or checkbox */
	ଯଦି (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_size = INTTOFIX(1);
		ଯଦି (block->height == AUTO)
			block->height = FIXTOINT(css_unit_len2device_px(
					block->style,
					&content->unit_len_ctx,
					gadget_size, gadget_unit));
	}

	box = block->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = block->padding[ଶୀର୍ଷ_2];
	ଯଦି (box)
		box->y = block->padding[ଶୀର୍ଷ_2];

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
	ଯେତେବେଳେ (box) {
		ଗଣନା css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
		ଗଣନା css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

		ପ୍ରତିଜ୍ଞା(box->type == BOX_BLOCK ||
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

		ଯଦି (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->parent->padding[ବାମ_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			ଯାଅ advance_to_next_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		ଯଦି (margin_collapse == ଖାଲି) {
			margin_collapse = layout_next_margin_block(
					&content->unit_len_ctx, box, block,
					viewport_height,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			in_margin = true;
		}

		/* Clearance. */
		y = 0;
		ଯଦି (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(block->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		ଯଦି (box->style) {
			overflow_x = css_computed_overflow_x(box->style);
			overflow_y = css_computed_overflow_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		ଯଦି (box->type == BOX_FLEX ||
		    box->type == BOX_BLOCK ||
		    box->flags & IFRAME) {
			ଯଦି (lh__box_is_object(box) == false &&
					box->style &&
					(overflow_x != CSS_OVERFLOW_VISIBLE ||
					 overflow_y != CSS_OVERFLOW_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା x0, x1, ଶୀର୍ଷ;
				ସଂରଚନା box *ବାମ, *ଡାହାଣ;
				ଶୀର୍ଷ = cy + max_pos_margin - max_neg_margin;
				ଶୀର୍ଷ = (ଶୀର୍ଷ > y) ? ଶୀର୍ଷ : y;
				x0 = cx;
				x1 = cx + box->parent->width -
						box->parent->padding[ବାମ_2] -
						box->parent->padding[ଡାହାଣ_2];
				find_sides(block->float_children, ଶୀର୍ଷ, ଶୀର୍ଷ,
						&x0, &x1, &ବାମ, &ଡାହାଣ);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->parent->width -
						box->parent->padding[ବାମ_2] -
						box->parent->padding[ଡାହାଣ_2] -
						x1;
			}
			layout_block_find_dimensions(&content->unit_len_ctx,
					box->parent->width,
					viewport_height, lm, rm, box);
			ଯଦି (box->type == BOX_BLOCK && !(box->flags & IFRAME)) {
				layout_block_add_scrollbar(box, ଡାହାଣ_2);
				layout_block_add_scrollbar(box, BOTTOM);
			}
		} ଅନ୍ୟଥା ଯଦି (box->type == BOX_TABLE) {
			ଯଦି (box->style != ଖାଲି) {
				ଗଣନା css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style, &width,
						&unit);

				ଯଦି (wtype == CSS_WIDTH_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					ପୂର୍ଣ୍ଣସଂଖ୍ୟା x0, x1, ଶୀର୍ଷ;
					ସଂରଚନା box *ବାମ, *ଡାହାଣ;
					ଶୀର୍ଷ = cy + max_pos_margin -
							max_neg_margin;
					ଶୀର୍ଷ = (ଶୀର୍ଷ > y) ? ଶୀର୍ଷ : y;
					x0 = cx;
					x1 = cx + box->parent->width -
						box->parent->padding[ବାମ_2] -
						box->parent->padding[ଡାହାଣ_2];
					find_sides(block->float_children,
						ଶୀର୍ଷ, ଶୀର୍ଷ, &x0, &x1,
						&ବାମ, &ଡାହାଣ);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->parent->width -
						box->parent->padding[ବାମ_2] -
						box->parent->padding[ଡାହାଣ_2] -
						x1;
				}
			}
			ଯଦି (!layout_table(box, box->parent->width - lm - rm,
					content))
				ଫେରନ୍ତୁ false;
			layout_solve_width(box, box->parent->width, box->width,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->parent->padding[ବାମ_2] + box->margin[ବାମ_2] +
				box->border[ବାମ_2].width;
		cx += box->x;

		/* Position box: vertical. */
		ଯଦି (box->border[ଶୀର୍ଷ_2].width) {
			box->y += box->border[ଶୀର୍ଷ_2].width;
			cy += box->border[ଶୀର୍ଷ_2].width;
		}

		/* Vertical margin */
		ଯଦି (((box->type == BOX_BLOCK && (box->flags & HAS_HEIGHT)) ||
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
		ଯଦି (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		ଯଦି (box->type == BOX_FLEX ||
				(box->type == BOX_BLOCK && box->style &&
				 (overflow_x != CSS_OVERFLOW_VISIBLE ||
				  overflow_y != CSS_OVERFLOW_VISIBLE))) {

			ଯଦି (box->type == BOX_FLEX) {
				ଯଦି (!layout_flex(box, box->width, content)) {
					ଫେରନ୍ତୁ false;
				}
			} ଅନ୍ୟଥା {
				layout_block_context(box,
						viewport_height, content);
			}

			cy += box->padding[ଶୀର୍ଷ_2];

			ଯଦି (box->height == AUTO) {
				box->height = 0;
				layout_block_add_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->height + box->padding[BOTTOM] +
					box->border[BOTTOM].width;
			y = box->y + box->padding[ଶୀର୍ଷ_2] + box->height +
					box->padding[BOTTOM] +
					box->border[BOTTOM].width;

			/* Skip children, because they are done in the new
			 * block context */
			ଯାଅ advance_to_next_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->width);

		/* Layout (except tables). */
		ଯଦି (box->object) {
			ଯଦି (!layout_block_object(box))
				ଫେରନ୍ତୁ false;

		} ଅନ୍ୟଥା ଯଦି (box->type == BOX_INLINE_CONTAINER) {
			box->width = box->parent->width;
			ଯଦି (!layout_inline_container(box, box->width, block,
					cx, cy, content))
				ଫେରନ୍ତୁ false;

		} ଅନ୍ୟଥା ଯଦି (box->type == BOX_TABLE) {
			/* Move down to avoid floats if necessary. */
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା x0, x1;
			ସଂରଚନା box *ବାମ, *ଡାହାଣ;
			y = cy;
			ଯେତେବେଳେ (1) {
				ଗଣନା css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style,
						&width, &unit);

				x0 = cx;
				x1 = cx + box->parent->width;
				find_sides(block->float_children, y,
						y + box->height,
						&x0, &x1, &ବାମ, &ଡାହାଣ);
				ଯଦି (wtype == CSS_WIDTH_AUTO)
					ଭାଙ୍ଗ;
				ଯଦି (box->width <= x1 - x0)
					ଭାଙ୍ଗ;
				ଯଦି (!ବାମ && !ଡାହାଣ)
					ଭାଙ୍ଗ;
				ଅନ୍ୟଥା ଯଦି (!ବାମ)
					y = ଡାହାଣ->y + ଡାହାଣ->height + 1;
				ଅନ୍ୟଥା ଯଦି (!ଡାହାଣ)
					y = ବାମ->y + ବାମ->height + 1;
				ଅନ୍ୟଥା ଯଦି (ବାମ->y + ବାମ->height <
						ଡାହାଣ->y + ଡାହାଣ->height)
					y = ବାମ->y + ବାମ->height + 1;
				ଅନ୍ୟଥା
					y = ଡାହାଣ->y + ଡାହାଣ->height + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		ଯଦି (box->type == BOX_BLOCK && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			ଯଦି (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = ଖାଲି;
			}

			y = box->padding[ଶୀର୍ଷ_2];
			box = box->children;
			box->y = y;
			cy += y;
			ଜାରି;
		} ଅନ୍ୟଥା ଯଦି (box->type == BOX_BLOCK || box->object ||
				box->flags & IFRAME)
			cy += box->padding[ଶୀର୍ଷ_2];

		ଯଦି (box->type == BOX_BLOCK && box->height == AUTO) {
			box->height = 0;
			layout_block_add_scrollbar(box, BOTTOM);
		}

		cy += box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width;
		cx -= box->x;
		y = box->y + box->padding[ଶୀର୍ଷ_2] + box->height +
				box->padding[BOTTOM] +
				box->border[BOTTOM].width;

	advance_to_next_box:
		ଯଦି (!box->next) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			କର {
				ଯଦି (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = ଖାଲି;
				}

				/* Apply bottom margin */
				ଯଦି (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				ଅନ୍ୟଥା ଯଦି (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->parent;
				ଯଦି (box == block)
					ଭାଙ୍ଗ;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				ଯଦି (box->type == BOX_BLOCK &&
						box->flags & MAKE_HEIGHT) {
					margin_collapse = ଖାଲି;
					in_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				ଯଦି (box->height == AUTO) {
					box->height = y - box->padding[ଶୀର୍ଷ_2];

					ଯଦି (box->type == BOX_BLOCK)
						layout_block_add_scrollbar(box,
								BOTTOM);
				} ଅନ୍ୟଥା
					cy += box->height -
							(y - box->padding[ଶୀର୍ଷ_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				ଯଦି (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_height(
							&content->unit_len_ctx,
							box, ଖାଲି)) {
					/* Height altered */
					/* Set current cy */
					cy += box->height -
							(y - box->padding[ଶୀର୍ଷ_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].width;
				cx -= box->x;
				y = box->y + box->padding[ଶୀର୍ଷ_2] + box->height +
						box->padding[BOTTOM] +
						box->border[BOTTOM].width;

			} ଯେତେବେଳେ (box->next == ଖାଲି);
			ଯଦି (box == block)
				ଭାଙ୍ଗ;
		}

		/* To next sibling. */

		ଯଦି (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = ଖାଲି;
		}

		ଯଦି (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		ଅନ୍ୟଥା ଯଦି (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->next;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	ପାଇଁ (box = block->float_children; box; box = box->next_float) {
		y = box->y + box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width + box->margin[BOTTOM];
		ଯଦି (cy < y)
			cy = y;
	}

	ଯଦି (block->height == AUTO) {
		block->height = cy - block->padding[ଶୀର୍ଷ_2];
		ଯଦି (block->type == BOX_BLOCK)
			layout_block_add_scrollbar(block, BOTTOM);
	}

	ଯଦି (block->style && css_computed_position(block->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_height(&content->unit_len_ctx, block, ଖାଲି);
	}

	ଯଦି (block->gadget &&
			(block->gadget->type == GADGET_TEXTAREA ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ta_width = block->padding[ବାମ_2] + block->width +
				block->padding[ଡାହାଣ_2];
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ta_height = block->padding[ଶୀର୍ଷ_2] + block->height +
				block->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				block->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_set_layout(block->gadget->data.text.ta,
				&fstyle, ta_width, ta_height,
				block->padding[ଶୀର୍ଷ_2], block->padding[ଡାହାଣ_2],
				block->padding[BOTTOM], block->padding[ବାମ_2]);
	}

	ଫେରନ୍ତୁ true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
ସ୍ଥିର bool
layout__get_element_tag(
		ଅଚଳ dom_ଗଣ୍ଠି *ଗଣ୍ଠି,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_ଗଣ୍ଠି_type ଗଣ୍ଠି_type;
	dom_exception exc;

	exc = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(ଗଣ୍ଠି, &ଗଣ୍ଠି_type);
	ଯଦି (exc != DOM_NO_ERR ||
	    ଗଣ୍ଠି_type != DOM_ELEMENT_ଗଣ୍ଠି) {
		ଫେରନ୍ତୁ false;
	}

	exc = dom_html_element_get_tag_type(ଗଣ୍ଠି, &element_type);
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ false;
	}

	*type = element_type;
	ଫେରନ୍ତୁ true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool
layout__check_element_type(
		ଅଚଳ dom_ଗଣ୍ଠି *ଗଣ୍ଠି,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	ଯଦି (!layout__get_element_tag(ଗଣ୍ଠି, &element_type)) {
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ସ୍ଥିର bool
layout_get_li_ମୂଲ୍ୟ(dom_ଗଣ୍ଠି *li_ଗଣ୍ଠି, dom_long *ମୂଲ୍ୟ_out)
{
	dom_exception exc;
	dom_long ମୂଲ୍ୟ;
	bool has_ମୂଲ୍ୟ;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_ଗଣ୍ଠି,
			corestring_dom_ମୂଲ୍ୟ,
			&has_ମୂଲ୍ୟ);
	ଯଦି (exc != DOM_NO_ERR || has_ମୂଲ୍ୟ == false) {
		ଫେରନ୍ତୁ false;
	}

	exc = dom_html_li_element_get_ମୂଲ୍ୟ(
			(dom_html_li_element *)li_ଗଣ୍ଠି,
			&ମୂଲ୍ୟ);
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ false;
	}

	*ମୂଲ୍ୟ_out = ମୂଲ୍ୟ;
	ଫେରନ୍ତୁ true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ସ୍ଥିର bool
layout__get_ol_start(dom_ଗଣ୍ଠି *ol_ଗଣ୍ଠି, dom_long *start_out)
{
	dom_exception exc;
	dom_long start;
	bool has_start;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_ଗଣ୍ଠି,
			corestring_dom_start,
			&has_start);
	ଯଦି (exc != DOM_NO_ERR || has_start == false) {
		ଫେରନ୍ତୁ false;
	}

	exc = dom_html_olist_element_get_start(
			(dom_html_olist_element *)ol_ଗଣ୍ଠି,
			&start);
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ false;
	}

	*start_out = start;
	ଫେରନ୍ତୁ true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
ସ୍ଥିର bool
layout__get_ol_reversed(dom_ଗଣ୍ଠି *ol_ଗଣ୍ଠି)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_ଗଣ୍ଠି,
			corestring_dom_reversed,
			&has_reversed);
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
ସ୍ଥିର bool
layout_get_list_item_ଗଣନା(
		dom_ଗଣ୍ଠି *list_owner, dom_long *ଗଣନା_out)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_ଗଣ୍ଠି *child;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2;

	ଯଦି (list_owner == ଖାଲି) {
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (!layout__get_element_tag(list_owner, &tag_type)) {
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		ଫେରନ୍ତୁ false;
	}

	exc = dom_ଗଣ୍ଠି_get_first_child(list_owner, &child);
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ false;
	}

	ଗଣନା_2 = 0;
	ଯେତେବେଳେ (child != ଖାଲି) {
		dom_ଗଣ୍ଠି *temp_ଗଣ୍ଠି;

		ଯଦି (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ସଂରଚନା box *child_box;
			ଯଦି (dom_ଗଣ୍ଠି_get_user_data(child,
					corestring_dom_ns_ଚାବି_box_ଗଣ୍ଠି_data,
					&child_box) != DOM_NO_ERR) {
				dom_ଗଣ୍ଠି_unref(child);
				ଫେରନ୍ତୁ false;
			}

			ଯଦି (child_box != ଖାଲି &&
			    child_box->list_marker != ଖାଲି) {
				ଗଣନା_2++;
			}
		}

		exc = dom_ଗଣ୍ଠି_get_next_sibling(child, &temp_ଗଣ୍ଠି);
		dom_ଗଣ୍ଠି_unref(child);
		ଯଦି (exc != DOM_NO_ERR) {
			ଫେରନ୍ତୁ false;
		}

		child = temp_ଗଣ୍ଠି;
	}

	*ଗଣନା_out = ଗଣନା_2;
	ଫେରନ୍ତୁ true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
ସ୍ଥିର ଶୂନ୍ୟ
layout_ordered_list_ଗଣନା(
		ସଂରଚନା box *box)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_ଗଣ୍ଠି *child;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା step = 1;
	dom_long next;

	ଯଦି (box->ଗଣ୍ଠି == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}

	ଯଦି (!layout__get_element_tag(box->ଗଣ୍ଠି, &tag_type)) {
		ଫେରନ୍ତୁ;
	}

	ଯଦି (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		ଫେରନ୍ତୁ;
	}

	next = 1;
	ଯଦି (tag_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_start = layout__get_ol_start(box->ଗଣ୍ଠି, &next);
		bool have_reversed = layout__get_ol_reversed(box->ଗଣ୍ଠି);

		ଯଦି (have_reversed) {
			step = -1;
		}

		ଯଦି (!have_start && have_reversed) {
			layout_get_list_item_ଗଣନା(box->ଗଣ୍ଠି, &next);
		}
	}

	exc = dom_ଗଣ୍ଠି_get_first_child(box->ଗଣ୍ଠି, &child);
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ;
	}

	ଯେତେବେଳେ (child != ଖାଲି) {
		dom_ଗଣ୍ଠି *temp_ଗଣ୍ଠି;

		ଯଦି (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ସଂରଚନା box *child_box;

			ଯଦି (dom_ଗଣ୍ଠି_get_user_data(child,
					corestring_dom_ns_ଚାବି_box_ଗଣ୍ଠି_data,
					&child_box) != DOM_NO_ERR) {
				dom_ଗଣ୍ଠି_unref(child);
				ଫେରନ୍ତୁ;
			}

			ଯଦି (child_box != ଖାଲି &&
			    child_box->list_marker != ଖାଲି) {
				dom_long ମୂଲ୍ୟ;
				ସଂରଚନା box *marker = child_box->list_marker;
				ଯଦି (layout_get_li_ମୂଲ୍ୟ(child, &ମୂଲ୍ୟ)) {
					marker->list_ମୂଲ୍ୟ = ମୂଲ୍ୟ;
					next = marker->list_ମୂଲ୍ୟ;
				} ଅନ୍ୟଥା {
					marker->list_ମୂଲ୍ୟ = next;
				}
				next += step;
			}
		}

		exc = dom_ଗଣ୍ଠି_get_next_sibling(child, &temp_ଗଣ୍ଠି);
		dom_ଗଣ୍ଠି_unref(child);
		ଯଦି (exc != DOM_NO_ERR) {
			ଫେରନ୍ତୁ;
		}

		child = temp_ଗଣ୍ଠି;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
ସ୍ଥିର ଶୂନ୍ୟ
layout__set_numerical_marker_text(
		ଅଚଳ html_content *content,
		ସଂରଚନା box *box)
{
	ସଂରଚନା box *marker = box->list_marker;
	size_t counter_len;
	css_error css_res;
	ଗଣନା {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_SIZE = 20,
	};

	marker->text = talloc_array(content->bctx, ଅକ୍ଷର, LIST_MARKER_SIZE);
	ଯଦି (marker->text == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_ମୂଲ୍ୟ,
			marker->text, LIST_MARKER_SIZE, &counter_len);
	ଯଦି (css_res == CSS_OK) {
		ଯଦି (counter_len > LIST_MARKER_SIZE) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->text = talloc_realloc(content->bctx,
					marker->text,
					ଅକ୍ଷର,
					counter_len);
			ଯଦି (marker->text == ଖାଲି) {
				ଫେରନ୍ତୁ;
			}
			css_computed_format_list_style(box->style,
					marker->list_ମୂଲ୍ୟ, marker->text,
					counter_len, &counter_len);
		}
		marker->length = counter_len;
	}
}

/**
 * Find out if box's style represents a numerical list style type.
 *
 * \param[in]  b  Box with style to test.
 * \return true if box has numerical list style type, false otherwise.
 */
ସ୍ଥିର bool
layout__list_item_is_numerical(
		ଅଚଳ ସଂରଚନା box *b)
{
	ଗଣନା css_list_style_type_e t = css_computed_list_style_type(b->style);

	ବାଛ (t) {
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_SQUARE: /* Fall through. */
	ସ୍ଥିତି CSS_LIST_STYLE_TYPE_NONE:
		ଫେରନ୍ତୁ false;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ true;
	}
}

/**
 * Layout list markers.
 */
ସ୍ଥିର ଶୂନ୍ୟ
layout_lists(ଅଚଳ html_content *content, ସଂରଚନା box *box)
{
	ସଂରଚନା box *child;

	layout_ordered_list_ଗଣନା(box);

	ପାଇଁ (child = box->children; child; child = child->next) {
		ଯଦି (child->list_marker) {
			ସଂରଚନା box *marker = child->list_marker;

			ଯଦି (layout__list_item_is_numerical(child)) {
				ଯଦି (marker->text == ଖାଲି) {
					layout__set_numerical_marker_text(
							content, child);
				}
			}
			ଯଦି (marker->object) {
				marker->width =
					content_get_width(marker->object);
				marker->x = -marker->width;
				marker->height =
					content_get_height(marker->object);
				marker->y = (ଧାଡ଼ି_height(
						&content->unit_len_ctx,
						marker->style) -
						marker->height) / 2;
			} ଅନ୍ୟଥା ଯଦି (marker->text) {
				ଯଦି (marker->width == UNKNOWN_WIDTH) {
					plot_font_style_t fstyle;
					font_plot_style_from_css(
							&content->unit_len_ctx,
							marker->style,
							&fstyle);
					content->font_func->width(&fstyle,
							marker->text,
							marker->length,
							&marker->width);
					marker->flags |= MEASURED;
				}
				marker->x = -marker->width;
				marker->y = 0;
				marker->height = ଧାଡ଼ି_height(
						&content->unit_len_ctx,
						marker->style);
			} ଅନ୍ୟଥା {
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
ସ୍ଥିର ଶୂନ୍ୟ
layout_compute_offsets(ଅଚଳ css_unit_ctx *unit_len_ctx,
		       ସଂରଚନା box *box,
		       ସଂରଚନା box *containing_block,
		       ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ଶୀର୍ଷ,
		       ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ଡାହାଣ,
		       ପୂର୍ଣ୍ଣସଂଖ୍ୟା *bottom,
		       ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ବାମ)
{
	uint32_t type;
	css_fixed ମୂଲ୍ୟ = 0;
	css_unit unit = CSS_UNIT_PX;

	ପ୍ରତିଜ୍ଞା(containing_block->width != UNKNOWN_WIDTH);
	ପ୍ରତିଜ୍ଞା(containing_block->width != AUTO);
	ପ୍ରତିଜ୍ଞା(containing_block->height != AUTO);

	/*ବାମ*/
	type = css_computed_ବାମ(box->style, &ମୂଲ୍ୟ, &unit);
	ଯଦି (type == CSS_ବାମ_SET) {
		ଯଦି (unit == CSS_UNIT_PCT) {
			*ବାମ = FPCT_OF_INT_TOINT(ମୂଲ୍ୟ,
					containing_block->width);
		} ଅନ୍ୟଥା {
			*ବାମ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ମୂଲ୍ୟ, unit));
		}
	} ଅନ୍ୟଥା {
		*ବାମ = AUTO;
	}

	/*ଡାହାଣ*/
	type = css_computed_ଡାହାଣ(box->style, &ମୂଲ୍ୟ, &unit);
	ଯଦି (type == CSS_ଡାହାଣ_SET) {
		ଯଦି (unit == CSS_UNIT_PCT) {
			*ଡାହାଣ = FPCT_OF_INT_TOINT(ମୂଲ୍ୟ,
					containing_block->width);
		} ଅନ୍ୟଥା {
			*ଡାହାଣ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ମୂଲ୍ୟ, unit));
		}
	} ଅନ୍ୟଥା {
		*ଡାହାଣ = AUTO;
	}

	/*ଶୀର୍ଷ*/
	type = css_computed_ଶୀର୍ଷ(box->style, &ମୂଲ୍ୟ, &unit);
	ଯଦି (type == CSS_ଶୀର୍ଷ_SET) {
		ଯଦି (unit == CSS_UNIT_PCT) {
			*ଶୀର୍ଷ = FPCT_OF_INT_TOINT(ମୂଲ୍ୟ,
					containing_block->height);
		} ଅନ୍ୟଥା {
			*ଶୀର୍ଷ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ମୂଲ୍ୟ, unit));
		}
	} ଅନ୍ୟଥା {
		*ଶୀର୍ଷ = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &ମୂଲ୍ୟ, &unit);
	ଯଦି (type == CSS_BOTTOM_SET) {
		ଯଦି (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_OF_INT_TOINT(ମୂଲ୍ୟ,
					containing_block->height);
		} ଅନ୍ୟଥା {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ମୂଲ୍ୟ, unit));
		}
	} ଅନ୍ୟଥା {
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
ସ୍ଥିର bool
layout_absolute(ସଂରଚନା box *box,
		ସଂରଚନା box *containing_block,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା cx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cy,
		html_content *content)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା static_ବାମ, static_ଶୀର୍ଷ;  /* static position */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଶୀର୍ଷ, ଡାହାଣ, bottom, ବାମ;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, height, max_width, min_width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା *margin = box->margin;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା *padding = box->padding;
	ସଂରଚନା box_border *border = box->border;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width = containing_block->width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା space;

	ପ୍ରତିଜ୍ଞା(box->type == BOX_BLOCK || box->type == BOX_TABLE ||
			box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_ବାମ = cx + box->x;
	static_ଶୀର୍ଷ = cy + box->y;

	ଯଦି (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_block->width += containing_block->padding[ବାମ_2] +
				containing_block->padding[ଡାହାଣ_2];
		containing_block->height += containing_block->padding[ଶୀର୍ଷ_2] +
				containing_block->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_block,
			&ଶୀର୍ଷ, &ଡାହାଣ, &bottom, &ବାମ);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_block;
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1,
			box, box->style, &width, &height,
			&max_width, &min_width, 0, 0,
			margin, padding, border);
	box->float_container = ଖାଲି;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ବାମ, margin[ବାମ_2], border[ବାମ_2].width, padding[ବାମ_2], width,
	      padding[ଡାହାଣ_2], border[ଡାହାଣ_2].width, margin[ଡାହାଣ_2], ଡାହାଣ,
	      containing_block->width);


	ଯଦି (ବାମ == AUTO && width == AUTO && ଡାହାଣ == AUTO) {
		ଯଦି (margin[ବାମ_2] == AUTO)
			margin[ବାମ_2] = 0;
		ଯଦି (margin[ଡାହାଣ_2] == AUTO)
			margin[ଡାହାଣ_2] = 0;
		ବାମ = static_ବାମ;

		width = min(max(box->min_width, available_width),
			box->max_width);
		width -= box->margin[ବାମ_2] + box->border[ବାମ_2].width +
			box->padding[ବାମ_2] + box->padding[ଡାହାଣ_2] +
			box->border[ଡାହାଣ_2].width + box->margin[ଡାହାଣ_2];

		/* Adjust for {min|max}-width */
		ଯଦି (max_width >= 0 && width > max_width) width = max_width;
		ଯଦି (width < min_width) width = min_width;

		ଡାହାଣ = containing_block->width -
			ବାମ -
			margin[ବାମ_2] - border[ବାମ_2].width - padding[ବାମ_2] -
			width -
			padding[ଡାହାଣ_2] - border[ଡାହାଣ_2].width - margin[ଡାହାଣ_2];
	} ଅନ୍ୟଥା ଯଦି (ବାମ != AUTO && width != AUTO && ଡାହାଣ != AUTO) {

		/* Adjust for {min|max}-width */
		ଯଦି (max_width >= 0 && width > max_width) width = max_width;
		ଯଦି (min_width >  0 && width < min_width) width = min_width;

		ଯଦି (margin[ବାମ_2] == AUTO && margin[ଡାହାଣ_2] == AUTO) {
			space = containing_block->width -
					ବାମ - border[ବାମ_2].width -
					padding[ବାମ_2] - width - padding[ଡାହାଣ_2] -
					border[ଡାହାଣ_2].width - ଡାହାଣ;
			ଯଦି (space < 0) {
				margin[ବାମ_2] = 0;
				margin[ଡାହାଣ_2] = space;
			} ଅନ୍ୟଥା {
				margin[ବାମ_2] = margin[ଡାହାଣ_2] = space / 2;
			}
		} ଅନ୍ୟଥା ଯଦି (margin[ବାମ_2] == AUTO) {
			margin[ବାମ_2] = containing_block->width -
					ବାମ - border[ବାମ_2].width -
					padding[ବାମ_2] - width - padding[ଡାହାଣ_2] -
					border[ଡାହାଣ_2].width - margin[ଡାହାଣ_2] -
					ଡାହାଣ;
		} ଅନ୍ୟଥା ଯଦି (margin[ଡାହାଣ_2] == AUTO) {
			margin[ଡାହାଣ_2] = containing_block->width -
					ବାମ - margin[ବାମ_2] -
					border[ବାମ_2].width -
					padding[ବାମ_2] - width - padding[ଡାହାଣ_2] -
					border[ଡାହାଣ_2].width - ଡାହାଣ;
		} ଅନ୍ୟଥା {
			ଡାହାଣ = containing_block->width -
					ବାମ - margin[ବାମ_2] -
					border[ବାମ_2].width -
					padding[ବାମ_2] - width - padding[ଡାହାଣ_2] -
					border[ଡାହାଣ_2].width - margin[ଡାହାଣ_2];
		}
	} ଅନ୍ୟଥା {
		ଯଦି (margin[ବାମ_2] == AUTO)
			margin[ବାମ_2] = 0;
		ଯଦି (margin[ଡାହାଣ_2] == AUTO)
			margin[ଡାହାଣ_2] = 0;

		ଯଦି (ବାମ == AUTO && width == AUTO && ଡାହାଣ != AUTO) {
			available_width -= ଡାହାଣ;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[ବାମ_2] + box->border[ବାମ_2].width +
				box->padding[ବାମ_2] + box->padding[ଡାହାଣ_2] +
				box->border[ଡାହାଣ_2].width + box->margin[ଡାହାଣ_2];

			/* Adjust for {min|max}-width */
			ଯଦି (max_width >= 0 && width > max_width)
				width = max_width;
			ଯଦି (width < min_width)
				width = min_width;

			ବାମ = containing_block->width -
					margin[ବାମ_2] - border[ବାମ_2].width -
					padding[ବାମ_2] - width - padding[ଡାହାଣ_2] -
					border[ଡାହାଣ_2].width - margin[ଡାହାଣ_2] -
					ଡାହାଣ;
		} ଅନ୍ୟଥା ଯଦି (ବାମ == AUTO && width != AUTO && ଡାହାଣ == AUTO) {

			/* Adjust for {min|max}-width */
			ଯଦି (max_width >= 0 && width > max_width)
				width = max_width;
			ଯଦି (min_width >  0 && width < min_width)
				width = min_width;

			ବାମ = static_ବାମ;
			ଡାହାଣ = containing_block->width -
					ବାମ - margin[ବାମ_2] -
					border[ବାମ_2].width -
					padding[ବାମ_2] - width - padding[ଡାହାଣ_2] -
					border[ଡାହାଣ_2].width - margin[ଡାହାଣ_2];
		} ଅନ୍ୟଥା ଯଦି (ବାମ != AUTO && width == AUTO && ଡାହାଣ == AUTO) {
			available_width -= ବାମ;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[ବାମ_2] + box->border[ବାମ_2].width +
				box->padding[ବାମ_2] + box->padding[ଡାହାଣ_2] +
				box->border[ଡାହାଣ_2].width + box->margin[ଡାହାଣ_2];

			/* Adjust for {min|max}-width */
			ଯଦି (max_width >= 0 && width > max_width)
				width = max_width;
			ଯଦି (width < min_width)
				width = min_width;

			ଡାହାଣ = containing_block->width -
					ବାମ - margin[ବାମ_2] -
					border[ବାମ_2].width -
					padding[ବାମ_2] - width - padding[ଡାହାଣ_2] -
					border[ଡାହାଣ_2].width - margin[ଡାହାଣ_2];
		} ଅନ୍ୟଥା ଯଦି (ବାମ == AUTO && width != AUTO && ଡାହାଣ != AUTO) {

			/* Adjust for {min|max}-width */
			ଯଦି (max_width >= 0 && width > max_width)
				width = max_width;
			ଯଦି (width < min_width)
				width = min_width;

			ବାମ = containing_block->width -
					margin[ବାମ_2] - border[ବାମ_2].width -
					padding[ବାମ_2] - width - padding[ଡାହାଣ_2] -
					border[ଡାହାଣ_2].width - margin[ଡାହାଣ_2] -
					ଡାହାଣ;
		} ଅନ୍ୟଥା ଯଦି (ବାମ != AUTO && width == AUTO && ଡାହାଣ != AUTO) {
			width = containing_block->width -
					ବାମ - margin[ବାମ_2] -
					border[ବାମ_2].width -
					padding[ବାମ_2] - padding[ଡାହାଣ_2] -
					border[ଡାହାଣ_2].width - margin[ଡାହାଣ_2] -
					ଡାହାଣ;

			/* Adjust for {min|max}-width */
			ଯଦି (max_width >= 0 && width > max_width)
				width = max_width;
			ଯଦି (width < min_width)
				width = min_width;

		} ଅନ୍ୟଥା ଯଦି (ବାମ != AUTO && width != AUTO && ଡାହାଣ == AUTO) {

			/* Adjust for {min|max}-width */
			ଯଦି (max_width >= 0 && width > max_width)
				width = max_width;
			ଯଦି (width < min_width)
				width = min_width;

			ଡାହାଣ = containing_block->width -
					ବାମ - margin[ବାମ_2] -
					border[ବାମ_2].width -
					padding[ବାମ_2] - width - padding[ଡାହାଣ_2] -
					border[ଡାହାଣ_2].width - margin[ଡାହାଣ_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ବାମ, margin[ବାମ_2], border[ବାମ_2].width, padding[ବାମ_2], width,
	      padding[ଡାହାଣ_2], border[ଡାହାଣ_2].width, margin[ଡାହାଣ_2], ଡାହାଣ,
	      containing_block->width);

	box->x = ବାମ + margin[ବାମ_2] + border[ବାମ_2].width - cx;
	ଯଦି (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_block->width -= containing_block->padding[ବାମ_2] +
				containing_block->padding[ଡାହାଣ_2];
	} ଅନ୍ୟଥା {
		/** \todo inline ancestors */
	}
	box->width = width;
	box->height = height;

	ଯଦି (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->object || box->flags & IFRAME) {
		ଯଦି (!layout_block_context(box, -1, content))
			ଫେରନ୍ତୁ false;
	} ଅନ୍ୟଥା ଯଦି (box->type == BOX_TABLE) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		/* \todo  layout_table considers margins etc. again */
		ଯଦି (!layout_table(box, width, content))
			ଫେରନ୍ତୁ false;
		box->float_container = ଖାଲି;
		layout_solve_width(box, box->parent->width, box->width, 0, 0,
				-1, -1);
	} ଅନ୍ୟଥା ଯଦି (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		ଯଦି (!layout_flex(box, width, content))
			ଫେରନ୍ତୁ false;
		box->float_container = ଖାଲି;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ଶୀର୍ଷ, margin[ଶୀର୍ଷ_2], border[ଶୀର୍ଷ_2].width, padding[ଶୀର୍ଷ_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	ଯଦି (ଶୀର୍ଷ == AUTO && height == AUTO && bottom == AUTO) {
		ଶୀର୍ଷ = static_ଶୀର୍ଷ;
		height = box->height;
		ଯଦି (margin[ଶୀର୍ଷ_2] == AUTO)
			margin[ଶୀର୍ଷ_2] = 0;
		ଯଦି (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_block->height -
				ଶୀର୍ଷ - margin[ଶୀର୍ଷ_2] - border[ଶୀର୍ଷ_2].width -
				padding[ଶୀର୍ଷ_2] - height - padding[BOTTOM] -
				border[BOTTOM].width - margin[BOTTOM];
	} ଅନ୍ୟଥା ଯଦି (ଶୀର୍ଷ != AUTO && height != AUTO && bottom != AUTO) {
		ଯଦି (margin[ଶୀର୍ଷ_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_block->height -
					ଶୀର୍ଷ - border[ଶୀର୍ଷ_2].width - padding[ଶୀର୍ଷ_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - bottom;
			margin[ଶୀର୍ଷ_2] = margin[BOTTOM] = space / 2;
		} ଅନ୍ୟଥା ଯଦି (margin[ଶୀର୍ଷ_2] == AUTO) {
			margin[ଶୀର୍ଷ_2] = containing_block->height -
					ଶୀର୍ଷ - border[ଶୀର୍ଷ_2].width - padding[ଶୀର୍ଷ_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} ଅନ୍ୟଥା ଯଦି (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_block->height -
					ଶୀର୍ଷ - margin[ଶୀର୍ଷ_2] - border[ଶୀର୍ଷ_2].width -
					padding[ଶୀର୍ଷ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					bottom;
		} ଅନ୍ୟଥା {
			bottom = containing_block->height -
					ଶୀର୍ଷ - margin[ଶୀର୍ଷ_2] - border[ଶୀର୍ଷ_2].width -
					padding[ଶୀର୍ଷ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	} ଅନ୍ୟଥା {
		ଯଦି (margin[ଶୀର୍ଷ_2] == AUTO)
			margin[ଶୀର୍ଷ_2] = 0;
		ଯଦି (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		ଯଦି (ଶୀର୍ଷ == AUTO && height == AUTO && bottom != AUTO) {
			height = box->height;
			ଶୀର୍ଷ = containing_block->height -
					margin[ଶୀର୍ଷ_2] - border[ଶୀର୍ଷ_2].width -
					padding[ଶୀର୍ଷ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} ଅନ୍ୟଥା ଯଦି (ଶୀର୍ଷ == AUTO && height != AUTO && bottom == AUTO) {
			ଶୀର୍ଷ = static_ଶୀର୍ଷ;
			bottom = containing_block->height -
					ଶୀର୍ଷ - margin[ଶୀର୍ଷ_2] - border[ଶୀର୍ଷ_2].width -
					padding[ଶୀର୍ଷ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} ଅନ୍ୟଥା ଯଦି (ଶୀର୍ଷ != AUTO && height == AUTO && bottom == AUTO) {
			height = box->height;
			bottom = containing_block->height -
					ଶୀର୍ଷ - margin[ଶୀର୍ଷ_2] - border[ଶୀର୍ଷ_2].width -
					padding[ଶୀର୍ଷ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} ଅନ୍ୟଥା ଯଦି (ଶୀର୍ଷ == AUTO && height != AUTO && bottom != AUTO) {
			ଶୀର୍ଷ = containing_block->height -
					margin[ଶୀର୍ଷ_2] - border[ଶୀର୍ଷ_2].width -
					padding[ଶୀର୍ଷ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} ଅନ୍ୟଥା ଯଦି (ଶୀର୍ଷ != AUTO && height == AUTO && bottom != AUTO) {
			height = containing_block->height -
					ଶୀର୍ଷ - margin[ଶୀର୍ଷ_2] - border[ଶୀର୍ଷ_2].width -
					padding[ଶୀର୍ଷ_2] - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} ଅନ୍ୟଥା ଯଦି (ଶୀର୍ଷ != AUTO && height != AUTO && bottom == AUTO) {
			bottom = containing_block->height -
					ଶୀର୍ଷ - margin[ଶୀର୍ଷ_2] - border[ଶୀର୍ଷ_2].width -
					padding[ଶୀର୍ଷ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ଶୀର୍ଷ, margin[ଶୀର୍ଷ_2], border[ଶୀର୍ଷ_2].width, padding[ଶୀର୍ଷ_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	box->y = ଶୀର୍ଷ + margin[ଶୀର୍ଷ_2] + border[ଶୀର୍ଷ_2].width - cy;
	ଯଦି (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_block->height -= containing_block->padding[ଶୀର୍ଷ_2] +
				containing_block->padding[BOTTOM];
	} ଅନ୍ୟଥା {
		/** \todo Inline ancestors */
	}
	box->height = height;
	layout_apply_minmax_height(&content->unit_len_ctx, box, containing_block);

	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର bool
layout_position_absolute(ସଂରଚନା box *box,
			 ସଂରଚନା box *containing_block,
			 ପୂର୍ଣ୍ଣସଂଖ୍ୟା cx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cy,
			 html_content *content)
{
	ସଂରଚନା box *c;

	ପାଇଁ (c = box->children; c; c = c->next) {
		ଯଦି ((c->type == BOX_BLOCK || c->type == BOX_TABLE ||
				c->type == BOX_INLINE_BLOCK ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			ଯଦି (!layout_absolute(c, containing_block,
					cx, cy, content))
				ଫେରନ୍ତୁ false;
			ଯଦି (!layout_position_absolute(c, c, 0, 0, content))
				ଫେରନ୍ତୁ false;
		} ଅନ୍ୟଥା ଯଦି (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			ଯଦି (!layout_position_absolute(c, c, 0, 0, content))
				ଫେରନ୍ତୁ false;
		} ଅନ୍ୟଥା {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା px, py;
			ଯଦି (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_ବାମ ||
					css_computed_float(c->style) ==
						CSS_FLOAT_ଡାହାଣ)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				ସଂରଚନା box *p;
				px = c->x;
				py = c->y;
				ପାଇଁ (p = box->parent; p && !p->float_children;
						p = p->parent) {
					px -= p->x;
					py -= p->y;
				}
			} ଅନ୍ୟଥା {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			ଯଦି (!layout_position_absolute(c, containing_block,
					cx + px, cy + py, content))
				ଫେରନ୍ତୁ false;
		}
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
ସ୍ଥିର ଶୂନ୍ୟ layout_compute_relative_offset(
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ସଂରଚନା box *box,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *x,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *y)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ବାମ, ଡାହାଣ, ଶୀର୍ଷ, bottom;
	ସଂରଚନା box *containing_block;

	ପ୍ରତିଜ୍ଞା(box && box->parent && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	ଯଦି (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_ବାମ ||
			css_computed_float(box->style) == CSS_FLOAT_ଡାହାଣ)) {
		containing_block = box->float_container;
	} ଅନ୍ୟଥା {
		containing_block = box->parent;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_block,
			&ଶୀର୍ଷ, &ଡାହାଣ, &bottom, &ବାମ);

	ଯଦି (ବାମ == AUTO && ଡାହାଣ == AUTO)
		ବାମ = ଡାହାଣ = 0;
	ଅନ୍ୟଥା ଯଦି (ବାମ == AUTO)
		/* left is auto => computed = -right */
		ବାମ = -ଡାହାଣ;
	ଅନ୍ୟଥା ଯଦି (ଡାହାଣ == AUTO)
		/* right is auto => computed = -left */
		ଡାହାଣ = -ବାମ;
	ଅନ୍ୟଥା {
		/* over constrained => examine direction property
		 * of containing block */
		ଯଦି (containing_block->style &&
				css_computed_direction(
				containing_block->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			ବାମ = -ଡାହାଣ;
		} ଅନ୍ୟଥା {
			/* assume LTR in all other cases */
			ଡାହାଣ = -ବାମ;
		}
	}

	ପ୍ରତିଜ୍ଞା(ବାମ == -ଡାହାଣ);

	ଯଦି (ଶୀର୍ଷ == AUTO && bottom == AUTO) {
		ଶୀର୍ଷ = bottom = 0;
	} ଅନ୍ୟଥା ଯଦି (ଶୀର୍ଷ == AUTO) {
		ଶୀର୍ଷ = -bottom;
	} ଅନ୍ୟଥା {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -ଶୀର୍ଷ;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", ବାମ,
	      ଡାହାଣ, ଶୀର୍ଷ, bottom);

	*x = ବାମ;
	*y = ଶୀର୍ଷ;
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
ସ୍ଥିର ଶୂନ୍ୟ
layout_position_relative(
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ସଂରଚନା box *ମୂଳ,
		ସଂରଚନା box *fp,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fx,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fy)
{
	ସଂରଚନା box *box; /* for children of "root" */
	ସଂରଚନା box *fn;  /* for block formatting context box for children of
			  * "box" */
	ସଂରଚନା box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	ପ୍ରତିଜ୍ଞା(ମୂଳ);

	/* Normal children */
	ପାଇଁ (box = ମୂଳ->children; box; box = box->next) {

		ଯଦି (box->type == BOX_TEXT)
			ଜାରି;

		/* If relatively positioned, get offsets */
		ଯଦି (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_offset(
					unit_len_ctx, box, &x, &y);
		ଅନ୍ୟଥା
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		ଯଦି (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_ବାମ ||
				css_computed_float(box->style) ==
					CSS_FLOAT_ଡାହାଣ) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			ପାଇଁ (fc = fp->float_children; fc; fc = fc->next_float) {
				ଯଦି (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		ଯଦି (box->float_children) {
			fn = box;
			fnx = fny = 0;
		} ଅନ୍ୟଥା {
			fn = fp;
			fnx = fx + x;
			fny = fy + y;
		}

		/* recurse first */
		layout_position_relative(unit_len_ctx, box, fn, fnx, fny);

		/* Ignore things we're not interested in. */
		ଯଦି (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			ଜାରି;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		ଯଦି (box->type == BOX_INLINE && box->inline_end) {
			ସଂରଚନା box *b;
			ପାଇଁ (b = box->next; b && b != box->inline_end;
					b = b->next) {
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
ସ୍ଥିର ଶୂନ୍ୟ
layout_get_box_bbox(
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ସଂରଚନା box *box,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *desc_x0, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *desc_y0,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *desc_x1, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *desc_y1)
{
	*desc_x0 = -box->border[ବାମ_2].width;
	*desc_y0 = -box->border[ଶୀର୍ଷ_2].width;
	*desc_x1 = box->padding[ବାମ_2] + box->width + box->padding[ଡାହାଣ_2] +
			box->border[ଡାହାଣ_2].width;
	*desc_y1 = box->padding[ଶୀର୍ଷ_2] + box->height + box->padding[BOTTOM] +
			box->border[BOTTOM].width;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	ଯଦି (box->type == BOX_BLOCK && box->style != ଖାଲି &&
			css_computed_overflow_y(box->style) ==
					CSS_OVERFLOW_VISIBLE &&
			box->object == ଖାଲି) {
		css_fixed font_size = 0;
		css_unit font_unit = CSS_UNIT_PT;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା text_height;

		css_computed_font_size(box->style, &font_size, &font_unit);
		text_height = css_unit_len2device_px(box->style, unit_len_ctx,
				font_size, font_unit);
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
ସ୍ଥିର ଶୂନ୍ୟ
layout_update_descendant_bbox(
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ସଂରଚନା box *box,
		ସଂରଚନା box *child,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା off_x,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା off_y)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା child_desc_x0, child_desc_y0, child_desc_x1, child_desc_y1;

	/* get coordinates of child relative to box */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା child_x = child->x - off_x;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା child_y = child->y - off_y;

	bool html_object = (child->object &&
			content_get_type(child->object) == CONTENT_HTML);

	ଗଣନା css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	ଗଣନା css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

	ଯଦି (child->style != ଖାଲି) {
		overflow_x = css_computed_overflow_x(child->style);
		overflow_y = css_computed_overflow_y(child->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, child,
			&child_desc_x0, &child_desc_y0,
			&child_desc_x1, &child_desc_y1);

	ଯଦି (overflow_x == CSS_OVERFLOW_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		child_desc_x0 = child->descendant_x0;
		child_desc_x1 = child->descendant_x1;
	}
	ଯଦି (overflow_y == CSS_OVERFLOW_VISIBLE &&
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
	ଯଦି (child_desc_x0 < box->descendant_x0)
		box->descendant_x0 = child_desc_x0;
	ଯଦି (child_desc_y0 < box->descendant_y0)
		box->descendant_y0 = child_desc_y0;
	ଯଦି (box->descendant_x1 < child_desc_x1)
		box->descendant_x1 = child_desc_x1;
	ଯଦି (box->descendant_y1 < child_desc_y1)
		box->descendant_y1 = child_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
ସ୍ଥିର ଶୂନ୍ୟ layout_calculate_descendant_bboxes(
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ସଂରଚନା box *box)
{
	ସଂରଚନା box *child;

	ପ୍ରତିଜ୍ଞା(box->width != UNKNOWN_WIDTH);
	ପ୍ରତିଜ୍ଞା(box->height != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	ଯଦି (box->object && content_get_type(box->object) == CONTENT_HTML) {
		ଯଦି (box->descendant_x1 < content_get_width(box->object))
			box->descendant_x1 = content_get_width(box->object);
		ଯଦି (box->descendant_y1 < content_get_height(box->object))
			box->descendant_y1 = content_get_height(box->object);
	}

	ଯଦି (box->iframe != ଖାଲି) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, y;
		box_coords(box, &x, &y);

		browser_window_set_position(box->iframe, x, y);
		browser_window_set_dimensions(box->iframe,
				box->width, box->height);
		browser_window_reformat(box->iframe, true,
				box->width, box->height);
	}

	ଯଦି (box->type == BOX_INLINE || box->type == BOX_TEXT)
		ଫେରନ୍ତୁ;

	ଯଦି (box->type == BOX_INLINE_END) {
		box = box->inline_end;
		ପାଇଁ (child = box->next; child;
				child = child->next) {
			ଯଦି (child->type == BOX_FLOAT_ବାମ ||
					child->type == BOX_FLOAT_ଡାହାଣ)
				ଜାରି;

			layout_update_descendant_bbox(unit_len_ctx, box, child,
					box->x, box->y);

			ଯଦି (child == box->inline_end)
				ଭାଙ୍ଗ;
		}
		ଫେରନ୍ତୁ;
	}

	ଯଦି (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		ଫେରନ୍ତୁ;

	ପାଇଁ (child = box->children; child; child = child->next) {
		ଯଦି (child->type == BOX_FLOAT_ବାମ ||
				child->type == BOX_FLOAT_ଡାହାଣ)
			ଜାରି;

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		ଯଦି (box->style && css_computed_overflow_x(box->style) ==
				CSS_OVERFLOW_HIDDEN &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_HIDDEN)
			ଜାରି;

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	ପାଇଁ (child = box->float_children; child; child = child->next_float) {
		ପ୍ରତିଜ୍ଞା(child->type == BOX_FLOAT_ବାମ ||
				child->type == BOX_FLOAT_ଡାହାଣ);

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	ଯଦି (box->list_marker) {
		child = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height)
{
	bool ret;
	ସଂରଚନା box *doc = content->layout;
	ଅଚଳ ସଂରଚନା gui_layout_table *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			width, height, nsurl_access(content_get_url(
					&content->base)));

	layout_minmax_block(doc, font_func, content);

	layout_block_find_dimensions(&content->unit_len_ctx,
			width, height, 0, 0, doc);
	doc->x = doc->margin[ବାମ_2] + doc->border[ବାମ_2].width;
	doc->y = doc->margin[ଶୀର୍ଷ_2] + doc->border[ଶୀର୍ଷ_2].width;
	width -= doc->margin[ବାମ_2] + doc->border[ବାମ_2].width +
			doc->padding[ବାମ_2] + doc->padding[ଡାହାଣ_2] +
			doc->border[ଡାହାଣ_2].width + doc->margin[ଡାହାଣ_2];
	ଯଦି (width < 0) {
		width = 0;
	}
	doc->width = width;

	ret = layout_block_context(doc, height, content);

	/* make <html> and <body> fill available height */
	ଯଦି (doc->y + doc->padding[ଶୀର୍ଷ_2] + doc->height + doc->padding[BOTTOM] +
			doc->border[BOTTOM].width + doc->margin[BOTTOM] <
			height) {
		doc->height = height - (doc->y + doc->padding[ଶୀର୍ଷ_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].width +
				doc->margin[BOTTOM]);
		ଯଦି (doc->children)
			doc->children->height = doc->height -
					(doc->children->margin[ଶୀର୍ଷ_2] +
					 doc->children->border[ଶୀର୍ଷ_2].width +
					 doc->children->padding[ଶୀର୍ଷ_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].width +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	ଫେରନ୍ତୁ ret;
}
