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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <math.h>
#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/talloc.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "netsurf/browser_window.h"
#ಅನ್ತರ್ಭಾವ "netsurf/layout.h"
#ಅನ್ತರ್ಭಾವ "content/content.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "css/utils.h"
#ಅನ್ತರ್ಭಾವ "desktop/scrollbar.h"
#ಅನ್ತರ್ಭಾವ "desktop/textarea.h"

#ಅನ್ತರ್ಭಾವ "html/html.h"
#ಅನ್ತರ್ಭಾವ "html/html_save.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_inspect.h"
#ಅನ್ತರ್ಭಾವ "html/font.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"
#ಅನ್ತರ್ಭಾವ "html/layout.h"
#ಅನ್ತರ್ಭಾವ "html/layout_internal.h"
#ಅನ್ತರ್ಭಾವ "html/table.h"

/** Array of per-side access functions for computed style margins. */
ಧ್ರುವ css_len_func margin_funcs[4] = {
	[ತುದಿ_2]    = css_computed_margin_ತುದಿ,
	[ಬಲ_2]  = css_computed_margin_ಬಲ,
	[BOTTOM] = css_computed_margin_bottom,
	[ಎಡ_2]   = css_computed_margin_ಎಡ,
};

/** Array of per-side access functions for computed style paddings. */
ಧ್ರುವ css_len_func padding_funcs[4] = {
	[ತುದಿ_2]    = css_computed_padding_ತುದಿ,
	[ಬಲ_2]  = css_computed_padding_ಬಲ,
	[BOTTOM] = css_computed_padding_bottom,
	[ಎಡ_2]   = css_computed_padding_ಎಡ,
};

/** Array of per-side access functions for computed style border_widths. */
ಧ್ರುವ css_len_func border_width_funcs[4] = {
	[ತುದಿ_2]    = css_computed_border_ತುದಿ_width,
	[ಬಲ_2]  = css_computed_border_ಬಲ_width,
	[BOTTOM] = css_computed_border_bottom_width,
	[ಎಡ_2]   = css_computed_border_ಎಡ_width,
};

/** Array of per-side access functions for computed style border styles. */
ಧ್ರುವ css_border_style_func border_style_funcs[4] = {
	[ತುದಿ_2]    = css_computed_border_ತುದಿ_style,
	[ಬಲ_2]  = css_computed_border_ಬಲ_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[ಎಡ_2]   = css_computed_border_ಎಡ_style,
};

/** Array of per-side access functions for computed style border colors. */
ಧ್ರುವ css_border_color_func border_color_funcs[4] = {
	[ತುದಿ_2]    = css_computed_border_ತುದಿ_color,
	[ಬಲ_2]  = css_computed_border_ಬಲ_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[ಎಡ_2]   = css_computed_border_ಎಡ_color,
};

/* forward declaration to break cycles */
ಸ್ಥಿರ ಶೂನ್ಯ layout_minmax_block(
		ಸಂರಚನಾ box *block,
		ಧ್ರುವ ಸಂರಚನಾ gui_layout_table *font_func,
		ಧ್ರುವ html_content *content);

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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_get_object_dimensions(ಸಂರಚನಾ box *box,
			     ಪೂರ್ಣಾಙ್ಕ *width, ಪೂರ್ಣಾಙ್ಕ *height,
			     ಪೂರ್ಣಾಙ್ಕ min_width, ಪೂರ್ಣಾಙ್ಕ max_width,
			     ಪೂರ್ಣಾಙ್ಕ min_height, ಪೂರ್ಣಾಙ್ಕ max_height)
{
	ಪ್ರತಿಜ್ಞಾ(box->object != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(width != ರಿಕ್ತ && height != ರಿಕ್ತ);

	ಯದಿ (*width == AUTO && *height == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		ಪೂರ್ಣಾಙ್ಕ intrinsic_width = content_get_width(box->object);
		ಪೂರ್ಣಾಙ್ಕ intrinsic_height = content_get_height(box->object);

		/* use intrinsic dimensions */
		*width = intrinsic_width;
		*height = intrinsic_height;

		/* Deal with min/max-width first */
		ಯದಿ (min_width >  0 && min_width > *width) {
			*width = min_width;
			scaled = true;
		}
		ಯದಿ (max_width >= 0 && max_width < *width) {
			*width = max_width;
			scaled = true;
		}

		ಯದಿ (scaled && (intrinsic_width != 0)) {
			/* Update height */
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		}

		scaled = false;
		/* Deal with min/max-height */
		ಯದಿ (min_height >  0 && min_height > *height) {
			*height = min_height;
			scaled = true;
		}
		ಯದಿ (max_height >= 0 && max_height < *height) {
			*height = max_height;
			scaled = true;
		}

		ಯದಿ (scaled && (intrinsic_height != 0)) {
			/* Update width */
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		}

	} ಅನ್ಯಥಾ ಯದಿ (*width == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		ಪೂರ್ಣಾಙ್ಕ intrinsic_width = content_get_width(box->object);
		ಪೂರ್ಣಾಙ್ಕ intrinsic_height = content_get_height(box->object);

		ಯದಿ (intrinsic_height != 0)
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		ಅನ್ಯಥಾ
			*width = intrinsic_width;

		ಯದಿ (min_width >  0 && min_width > *width)
			*width = min_width;
		ಯದಿ (max_width >= 0 && max_width < *width)
			*width = max_width;

	} ಅನ್ಯಥಾ ಯದಿ (*height == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		ಪೂರ್ಣಾಙ್ಕ intrinsic_width = content_get_width(box->object);
		ಪೂರ್ಣಾಙ್ಕ intrinsic_height = content_get_height(box->object);

		ಯದಿ (min_width >  0 && min_width > *width)
			*width = min_width;
		ಯದಿ (max_width >= 0 && max_width < *width)
			*width = max_width;

		ಯದಿ (intrinsic_width != 0)
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		ಅನ್ಯಥಾ
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ layout_text_indent(
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಧ್ರುವ css_computed_style *style, ಪೂರ್ಣಾಙ್ಕ width)
{
	css_fixed ಮೌಲ್ಯ = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_text_indent(style, &ಮೌಲ್ಯ, &unit);

	ಯದಿ (unit == CSS_UNIT_PCT) {
		ಪ್ರತ್ಯಾಗಮ FPCT_OF_INT_TOINT(ಮೌಲ್ಯ, width);
	} ಅನ್ಯಥಾ {
		ಪ್ರತ್ಯಾಗಮ FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				ಮೌಲ್ಯ, unit));
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
ಸ್ಥಿರ ಶೂನ್ಯ layout_minmax_table(ಸಂರಚನಾ box *table,
		ಧ್ರುವ ಸಂರಚನಾ gui_layout_table *font_func,
		ಧ್ರುವ html_content *content)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i, j;
	ಪೂರ್ಣಾಙ್ಕ width;
	ಪೂರ್ಣಾಙ್ಕ border_spacing_h = 0;
	ಪೂರ್ಣಾಙ್ಕ table_min = 0, table_max = 0;
	ಪೂರ್ಣಾಙ್ಕ extra_fixed = 0;
	ಭಿನ್ನ extra_frac = 0;
	ಸಂರಚನಾ column *col;
	ಸಂರಚನಾ box *row_group, *row, *cell;

	/* check if the widths have already been calculated */
	ಯದಿ (table->max_width != UNKNOWN_MAX_WIDTH)
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (table_calculate_column_types(&content->unit_len_ctx, table) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		ಪ್ರತ್ಯಾಗಮ;
	}
	col = table->col;

	/* start with 0 except for fixed-width columns */
	ಪ್ರತಿ (i = 0; i != table->columns; i++) {
		ಯದಿ (col[i].type == COLUMN_WIDTH_FIXED)
			col[i].min = col[i].max = col[i].width;
		ಅನ್ಯಥಾ
			col[i].min = col[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	ಯದಿ (css_computed_border_collapse(table->style) ==
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
	ಪ್ರತಿ (row_group = table->children; row_group; row_group =row_group->next)
	ಪ್ರತಿ (row = row_group->children; row; row = row->next)
	ಪ್ರತಿ (cell = row->children; cell; cell = cell->next) {
		ಪ್ರತಿಜ್ಞಾ(cell->type == BOX_TABLE_CELL);
		ಪ್ರತಿಜ್ಞಾ(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		ಪ್ರತಿಜ್ಞಾ(cell->columns != 0);

		ಯದಿ (cell->columns != 1)
			ಅನುವರ್ತ;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		ಯದಿ (col[i].positioned)
			ಅನುವರ್ತ;

		/* update column min, max widths using cell widths */
		ಯದಿ (col[i].min < cell->min_width)
			col[i].min = cell->min_width;
		ಯದಿ (col[i].max < cell->max_width)
			col[i].max = cell->max_width;
	}

	/* 2nd pass: cells which span multiple columns */
	ಪ್ರತಿ (row_group = table->children; row_group; row_group =row_group->next)
	ಪ್ರತಿ (row = row_group->children; row; row = row->next)
	ಪ್ರತಿ (cell = row->children; cell; cell = cell->next) {
		unsigned ಪೂರ್ಣಾಙ್ಕ flexible_columns = 0;
		ಪೂರ್ಣಾಙ್ಕ min = 0, max = 0, fixed_width = 0, extra;

		ಯದಿ (cell->columns == 1)
			ಅನುವರ್ತ;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		ಪ್ರತಿ (j = 0; j != cell->columns; j++) {
			min += col[i + j].min;
			ಯದಿ (col[i + j].type == COLUMN_WIDTH_FIXED)
				fixed_width += col[i + j].width;
			ಅನ್ಯಥಾ
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		ಯದಿ (min < cell->min_width) {
			ಯದಿ (flexible_columns == 0) {
				extra = 1 + (cell->min_width - min) /
						cell->columns;
				ಪ್ರತಿ (j = 0; j != cell->columns; j++) {
					col[i + j].min += extra;
					ಯದಿ (col[i + j].max < col[i + j].min)
						col[i + j].max = col[i + j].min;
				}
			} ಅನ್ಯಥಾ {
				extra = 1 + (cell->min_width - min) /
						flexible_columns;
				ಪ್ರತಿ (j = 0; j != cell->columns; j++) {
					ಯದಿ (col[i + j].type !=
							COLUMN_WIDTH_FIXED) {
						col[i + j].min += extra;
						ಯದಿ (col[i + j].max <
								col[i + j].min)
							col[i + j].max =
								col[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		ಪ್ರತಿ (j = 0; j != cell->columns; j++)
			max += col[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		ಯದಿ (max < cell->max_width && flexible_columns) {
			extra = 1 + (cell->max_width - max) / flexible_columns;
			ಪ್ರತಿ (j = 0; j != cell->columns; j++)
				ಯದಿ (col[i + j].type != COLUMN_WIDTH_FIXED)
					col[i + j].max += extra;
		}
	}

	ಪ್ರತಿ (i = 0; i != table->columns; i++) {
		ಯದಿ (col[i].max < col[i].min) {
			box_dump(stderr, table, 0, true);
			ಪ್ರತಿಜ್ಞಾ(0);
		}
		table_min += col[i].min;
		table_max += col[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	ಯದಿ (css_computed_width_px(table->style, &content->unit_len_ctx,
			-1, &width) == CSS_WIDTH_SET) {
		ಯದಿ (table_min < width)
			table_min = width;
		ಯದಿ (table_max < width)
			table_max = width;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, ಎಡ_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, ಬಲ_2, true, true, true,
			&extra_fixed, &extra_frac);
	ಯದಿ (extra_fixed < 0)
		extra_fixed = 0;
	ಯದಿ (extra_frac < 0)
		extra_frac = 0;
	ಯದಿ (1.0 <= extra_frac)
		extra_frac = 0.9;
	table->min_width = (table_min + extra_fixed) / (1.0 - extra_frac);
	table->max_width = (table_max + extra_fixed) / (1.0 - extra_frac);
	table->min_width += (table->columns + 1) * border_spacing_h;
	table->max_width += (table->columns + 1) * border_spacing_h;

	ಪ್ರತಿಜ್ಞಾ(0 <= table->min_width && table->min_width <= table->max_width);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool box_has_percentage_max_width(ಸಂರಚನಾ box *b)
{
	css_unit unit = CSS_UNIT_PX;
	ಗಣನಾ css_max_width_e type;
	css_fixed ಮೌಲ್ಯ = 0;

	ಪ್ರತಿಜ್ಞಾ(b != ರಿಕ್ತ);

	type = css_computed_max_width(b->style, &ಮೌಲ್ಯ, &unit);
	ಪ್ರತ್ಯಾಗಮ ((type == CSS_MAX_WIDTH_SET) && (unit == CSS_UNIT_PCT));
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
ಸ್ಥಿರ ಸಂರಚನಾ box *
layout_minmax_ಸಾಲು(ಸಂರಚನಾ box *first,
		   ಪೂರ್ಣಾಙ್ಕ *ಸಾಲು_min,
		   ಪೂರ್ಣಾಙ್ಕ *ಸಾಲು_max,
		   bool first_ಸಾಲು,
		   bool *ಸಾಲು_has_height,
		   ಧ್ರುವ ಸಂರಚನಾ gui_layout_table *font_func,
		   ಧ್ರುವ html_content *content)
{
	ಪೂರ್ಣಾಙ್ಕ min = 0, max = 0, width, height, fixed;
	ಭಿನ್ನ frac;
	size_t i, j;
	ಸಂರಚನಾ box *b;
	ಸಂರಚನಾ box *block;
	plot_font_style_t fstyle;
	bool no_wrap;

	ಪ್ರತಿಜ್ಞಾ(first->parent);
	ಪ್ರತಿಜ್ಞಾ(first->parent->parent);
	ಪ್ರತಿಜ್ಞಾ(first->parent->parent->style);

	block = first->parent->parent;
	no_wrap = (css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_PRE);

	*ಸಾಲು_has_height = false;

	/* corresponds to the pass 1 loop in layout_line() */
	ಪ್ರತಿ (b = first; b; b = b->next) {
		ಗಣನಾ css_width_e wtype;
		ಗಣನಾ css_height_e htype;
		ಗಣನಾ css_box_sizing_e bs;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		ಪ್ರತಿಜ್ಞಾ(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		ಯದಿ (b->type == BOX_BR) {
			b = b->next;
			ಭಙ್ಗ;
		}

		ಯದಿ (lh__box_is_float_box(b)) {
			ಪ್ರತಿಜ್ಞಾ(b->children);
			ಯದಿ (b->children->type == BOX_TABLE)
				layout_minmax_table(b->children, font_func,
						content);
			ಅನ್ಯಥಾ
				layout_minmax_block(b->children, font_func,
						content);
			b->min_width = b->children->min_width;
			b->max_width = b->children->max_width;
			ಯದಿ (min < b->min_width)
				min = b->min_width;
			max += b->max_width;
			ಅನುವರ್ತ;
		}

		ಯದಿ (b->type == BOX_INLINE_BLOCK || b->type == BOX_INLINE_FLEX) {
			layout_minmax_block(b, font_func, content);
			ಯದಿ (min < b->min_width)
				min = b->min_width;
			max += b->max_width;

			ಯದಿ (b->flags & HAS_HEIGHT)
				*ಸಾಲು_has_height = true;
			ಅನುವರ್ತ;
		}

		ಪ್ರತಿಜ್ಞಾ(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		ಯದಿ (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->style, ಎಡ_2, true, true, true,
					&fixed, &frac);
			ಯದಿ (!b->inline_end)
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ಬಲ_2,
						true, true, true,
						&fixed, &frac);
			ಯದಿ (0 < fixed)
				max += fixed;
			*ಸಾಲು_has_height = true;
			/* \todo  update min width, consider fractional extra */
		} ಅನ್ಯಥಾ ಯದಿ (b->type == BOX_INLINE_END) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->inline_end->style, ಬಲ_2,
					true, true, true,
					&fixed, &frac);
			ಯದಿ (0 < fixed)
				max += fixed;

			ಯದಿ (b->next) {
				ಯದಿ (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*ಸಾಲು_has_height = true;
			ಅನುವರ್ತ;
		}

		ಯದಿ (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			ಯದಿ (!b->text)
				ಅನುವರ್ತ;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			ಯದಿ (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				ಯದಿ (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					ಪೂರ್ಣಾಙ್ಕ opt_maxwidth = 0;
					ಸಂರಚನಾ form_option *o;

					ಪ್ರತಿ (o = b->parent->parent->gadget->
							data.select.ವಸ್ತುಗಳು; o;
							o = o->next) {
						ಪೂರ್ಣಾಙ್ಕ opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						ಯದಿ (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}

					b->width = opt_maxwidth;
					ಯದಿ (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;

				} ಅನ್ಯಥಾ {
					font_func->width(&fstyle, b->text,
						b->length, &b->width);
					b->flags |= MEASURED;
				}
			}
			max += b->width;
			ಯದಿ (b->next) {
				ಯದಿ (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			ಯದಿ (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} ಅನ್ಯಥಾ ಯದಿ (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				ಯದಿ (min < b->width)
					min = b->width;

			} ಅನ್ಯಥಾ ಯದಿ (b->parent->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				ಕುರು {
					ಪ್ರತಿ (j = i; j != b->length &&
							b->text[j] != ' '; j++)
						;
					font_func->width(&fstyle, b->text + i,
							 j - i, &width);
					ಯದಿ (min < width)
						min = width;
					i = j + 1;
				} ಯಾವತ್ (j != b->length);
			}

			*ಸಾಲು_has_height = true;

			ಅನುವರ್ತ;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		ಪ್ರತಿಜ್ಞಾ(b->style);

		bs = css_computed_box_sizing(block->style);

		/* calculate box width */
		wtype = css_computed_width_px(b->style,
				&content->unit_len_ctx, -1, &width);
		ಯದಿ (wtype == CSS_WIDTH_SET) {
			ಯದಿ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, ಎಡ_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, ಬಲ_2,
						false, true, true,
						&fixed, &frac);
				ಯದಿ (width < fixed) {
					width = fixed;
				}
			}
			ಯದಿ (width < 0)
				width = 0;
		} ಅನ್ಯಥಾ {
			width = AUTO;
		}

		/* height */
		htype = css_computed_height(b->style, &ಮೌಲ್ಯ, &unit);
		ಯದಿ (htype == CSS_HEIGHT_SET) {
			height = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		} ಅನ್ಯಥಾ {
			height = AUTO;
		}

		ಯದಿ (b->object || (b->flags & REPLACE_DIM)) {
			ಯದಿ (b->object) {
				ಪೂರ್ಣಾಙ್ಕ temp_height = height;
				layout_get_object_dimensions(b,
						&width, &temp_height,
						INT_MIN, INT_MAX,
						INT_MIN, INT_MAX);
			}

			fixed = frac = 0;
			ಯದಿ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ಎಡ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ಬಲ_2,
						true, false, false,
						&fixed, &frac);
			} ಅನ್ಯಥಾ {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ಎಡ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ಬಲ_2,
						true, true, true,
						&fixed, &frac);
			}
			ಯದಿ (0 < width + fixed)
				width += fixed;
		} ಅನ್ಯಥಾ ಯದಿ (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			ಯದಿ (width == AUTO)
				width = 400;

			fixed = frac = 0;
			ಯದಿ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ಎಡ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ಬಲ_2,
						true, false, false,
						&fixed, &frac);
			} ಅನ್ಯಥಾ {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ಎಡ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ಬಲ_2,
						true, true, true,
						&fixed, &frac);
			}

			ಯದಿ (0 < width + fixed)
				width += fixed;

		} ಅನ್ಯಥಾ {
			/* form control with no object */
			ಯದಿ (width == AUTO)
				width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		ಯದಿ (min < width && !box_has_percentage_max_width(b))
			min = width;
		ಯದಿ (width > 0)
			max += width;

		*ಸಾಲು_has_height = true;
	}

	ಯದಿ (first_ಸಾಲು) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		ಪೂರ್ಣಾಙ್ಕ text_indent = layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, 100);
		min = (min + text_indent < 0) ? 0 : min + text_indent;
		max = (max + text_indent < 0) ? 0 : max + text_indent;
	}

	*ಸಾಲು_min = min;
	*ಸಾಲು_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	ಪ್ರತಿಜ್ಞಾ(b != first);
	ಪ್ರತಿಜ್ಞಾ(0 <= *ಸಾಲು_min);
	ಪ್ರತಿಜ್ಞಾ(*ಸಾಲು_min <= *ಸಾಲು_max);
	ಪ್ರತ್ಯಾಗಮ b;
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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_minmax_inline_container(ಸಂರಚನಾ box *inline_container,
			       bool *has_height,
			       ಧ್ರುವ ಸಂರಚನಾ gui_layout_table *font_func,
			       ಧ್ರುವ html_content *content)
{
	ಸಂರಚನಾ box *child;
	ಪೂರ್ಣಾಙ್ಕ ಸಾಲು_min = 0, ಸಾಲು_max = 0;
	ಪೂರ್ಣಾಙ್ಕ min = 0, max = 0;
	bool first_ಸಾಲು = true;
	bool ಸಾಲು_has_height;

	ಪ್ರತಿಜ್ಞಾ(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	ಯದಿ (inline_container->max_width != UNKNOWN_MAX_WIDTH)
		ಪ್ರತ್ಯಾಗಮ;

	*has_height = false;

	ಪ್ರತಿ (child = inline_container->children; child; ) {
		child = layout_minmax_ಸಾಲು(child, &ಸಾಲು_min, &ಸಾಲು_max,
				first_ಸಾಲು, &ಸಾಲು_has_height, font_func,
				content);
		ಯದಿ (min < ಸಾಲು_min)
			min = ಸಾಲು_min;
		ಯದಿ (max < ಸಾಲು_max)
			max = ಸಾಲು_max;
		first_ಸಾಲು = false;
		*has_height |= ಸಾಲು_has_height;
	}

	inline_container->min_width = min;
	inline_container->max_width = max;

	ಪ್ರತಿಜ್ಞಾ(0 <= inline_container->min_width &&
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
ಸ್ಥಿರ ಶೂನ್ಯ layout_minmax_block(
		ಸಂರಚನಾ box *block,
		ಧ್ರುವ ಸಂರಚನಾ gui_layout_table *font_func,
		ಧ್ರುವ html_content *content)
{
	ಸಂರಚನಾ box *child;
	ಪೂರ್ಣಾಙ್ಕ min = 0, max = 0;
	ಪೂರ್ಣಾಙ್ಕ extra_fixed = 0;
	ಭಿನ್ನ extra_frac = 0;
	ಗಣನಾ css_width_e wtype = CSS_WIDTH_AUTO;
	css_fixed width = 0;
	css_unit wunit = CSS_UNIT_PX;
	ಗಣನಾ css_height_e htype = CSS_HEIGHT_AUTO;
	css_fixed height = 0;
	css_unit hunit = CSS_UNIT_PX;
	ಗಣನಾ css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool child_has_height = false;

	ಪ್ರತಿಜ್ಞಾ(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	/* check if the widths have already been calculated */
	ಯದಿ (block->max_width != UNKNOWN_MAX_WIDTH)
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (block->style != ರಿಕ್ತ) {
		wtype = css_computed_width(block->style, &width, &wunit);
		htype = css_computed_height(block->style, &height, &hunit);
		bs = css_computed_box_sizing(block->style);
	}

	/* set whether the minimum width is of any interest for this box */
	ಯದಿ (((block->parent && lh__box_is_float_box(block->parent)) ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX) &&
			wtype != CSS_WIDTH_SET) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} ಅನ್ಯಥಾ ಯದಿ (block->type == BOX_TABLE_CELL) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} ಅನ್ಯಥಾ ಯದಿ (block->parent && (block->parent->flags & NEED_MIN) &&
			wtype != CSS_WIDTH_SET) {
		/* box inside shrink-to-fit context; need minimum width */
		block->flags |= NEED_MIN;
	} ಅನ್ಯಥಾ ಯದಿ (block->parent && (block->parent->type == BOX_FLEX)) {
		/* box is flex item */
		block->flags |= NEED_MIN;
	}

	ಯದಿ (block->gadget && (block->gadget->type == GADGET_TEXTBOX ||
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

	ಯದಿ (block->gadget && (block->gadget->type == GADGET_RADIO ||
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

	ಯದಿ (block->object) {
		ಯದಿ (content_get_type(block->object) == CONTENT_HTML) {
			layout_minmax_block(html_get_box_ವೃಕ್ಷ(block->object),
					font_func, content);
			min = html_get_box_ವೃಕ್ಷ(block->object)->min_width;
			max = html_get_box_ವೃಕ್ಷ(block->object)->max_width;
		} ಅನ್ಯಥಾ {
			min = max = content_get_width(block->object);
		}

		block->flags |= HAS_HEIGHT;
	} ಅನ್ಯಥಾ ಯದಿ (block->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		block->flags |= HAS_HEIGHT;
	} ಅನ್ಯಥಾ {
		/* recurse through children */
		ಪ್ರತಿ (child = block->children; child; child = child->next) {
			ವಿಕಲ್ಪ (child->type) {
			ಪಕ್ಷ BOX_FLEX:
			ಪಕ್ಷ BOX_BLOCK:
				layout_minmax_block(child, font_func,
						content);
				ಯದಿ (child->flags & HAS_HEIGHT)
					child_has_height = true;
				ಭಙ್ಗ;
			ಪಕ್ಷ BOX_INLINE_CONTAINER:
				ಯದಿ (block->flags & NEED_MIN)
					child->flags |= NEED_MIN;

				layout_minmax_inline_container(child,
						&child_has_height, font_func,
						content);
				ಯದಿ (child_has_height &&
						child ==
						child->parent->children) {
					block->flags |= MAKE_HEIGHT;
				}
				ಭಙ್ಗ;
			ಪಕ್ಷ BOX_TABLE:
				layout_minmax_table(child, font_func,
						content);
				/* todo: fix for zero height tables */
				child_has_height = true;
				child->flags |= MAKE_HEIGHT;
				ಭಙ್ಗ;
			ಸ್ವಭಾವ:
				ಪ್ರತಿಜ್ಞಾ(0);
			}
			ಪ್ರತಿಜ್ಞಾ(child->max_width != UNKNOWN_MAX_WIDTH);

			ಯದಿ (child->style &&
					(css_computed_position(child->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(child->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				ಅನುವರ್ತ;
			}

			ಯದಿ (lh__box_is_flex_container(block) &&
			    lh__flex_main_is_horizontal(block)) {
				ಯದಿ (block->style != ರಿಕ್ತ &&
				    css_computed_flex_wrap(block->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += child->min_width;
				} ಅನ್ಯಥಾ {
					ಯದಿ (min < child->min_width)
						min = child->min_width;
				}
				max += child->max_width;

			} ಅನ್ಯಥಾ {
				ಯದಿ (min < child->min_width)
					min = child->min_width;
				ಯದಿ (max < child->max_width)
					max = child->max_width;
			}

			ಯದಿ (child_has_height)
				block->flags |= HAS_HEIGHT;
		}
	}

	ಯದಿ (max < min) {
		box_dump(stderr, block, 0, true);
		ಪ್ರತಿಜ್ಞಾ(0);
	}

	/* fixed width takes priority */
	ಯದಿ (block->type != BOX_TABLE_CELL && !lh__box_is_flex_item(block)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		ಗಣನಾ css_max_width_e max_type;
		ಗಣನಾ css_min_width_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed ಮೌಲ್ಯ = 0;
		ಪೂರ್ಣಾಙ್ಕ width;

		ಯದಿ (css_computed_width_px(block->style, &content->unit_len_ctx,
				-1, &width) == CSS_WIDTH_SET) {
			min = max = width;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_width(block->style, &ಮೌಲ್ಯ, &unit);
		ಯದಿ (min_type == CSS_MIN_WIDTH_SET && unit != CSS_UNIT_PCT) {
			ಪೂರ್ಣಾಙ್ಕ val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, ಮೌಲ್ಯ, unit));

			ಯದಿ (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_width(block->style, &ಮೌಲ್ಯ, &unit);
		ಯದಿ (max_type == CSS_MAX_WIDTH_SET && unit != CSS_UNIT_PCT) {
			ಪೂರ್ಣಾಙ್ಕ val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, ಮೌಲ್ಯ, unit));

			ಯದಿ (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	ಯದಿ (htype == CSS_HEIGHT_SET && hunit != CSS_UNIT_PCT &&
			height > INTTOFIX(0)) {
		block->flags |= MAKE_HEIGHT;
		block->flags |= HAS_HEIGHT;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_width(&content->unit_len_ctx, block->style, ಎಡ_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, ಬಲ_2,
			false, true, true, &extra_fixed, &extra_frac);

	ಯದಿ (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	ಯದಿ (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	ಯದಿ (max < min) {
		min = max;
	}

	calculate_mbp_width(&content->unit_len_ctx, block->style, ಎಡ_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, ಬಲ_2,
			true, false, false, &extra_fixed, &extra_frac);

	ಯದಿ (extra_fixed < 0)
		extra_fixed = 0;
	ಯದಿ (extra_frac < 0)
		extra_frac = 0;
	ಯದಿ (1.0 <= extra_frac)
		extra_frac = 0.9;
	ಯದಿ (block->style != ರಿಕ್ತ &&
			(css_computed_float(block->style) == CSS_FLOAT_ಎಡ ||
			css_computed_float(block->style) == CSS_FLOAT_ಬಲ)) {
		/* floated boxs */
		block->min_width = min + extra_fixed;
		block->max_width = max + extra_fixed;
	} ಅನ್ಯಥಾ {
		/* not floated */
		block->min_width = (min + extra_fixed) / (1.0 - extra_frac);
		block->max_width = (max + extra_fixed) / (1.0 - extra_frac);
	}

	ಪ್ರತಿಜ್ಞಾ(0 <= block->min_width);
	ಪ್ರತಿಜ್ಞಾ(block->min_width <= block->max_width);
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
ಸ್ಥಿರ ಸಂರಚನಾ box*
layout_next_margin_block(ಧ್ರುವ css_unit_ctx *unit_len_ctx,
			 ಸಂರಚನಾ box *box,
			 ಸಂರಚನಾ box *block,
			 ಪೂರ್ಣಾಙ್ಕ viewport_height,
			 ಪೂರ್ಣಾಙ್ಕ *max_pos_margin,
			 ಪೂರ್ಣಾಙ್ಕ *max_neg_margin)
{
	ಪ್ರತಿಜ್ಞಾ(block != ರಿಕ್ತ);

	ಯಾವತ್ (box != ರಿಕ್ತ) {

		ಯದಿ (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			ಯದಿ (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ,
						ರಿಕ್ತ, ರಿಕ್ತ, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				ಯದಿ (*max_pos_margin < box->margin[ತುದಿ_2])
					*max_pos_margin = box->margin[ತುದಿ_2];
				ಅನ್ಯಥಾ ಯದಿ (*max_neg_margin < -box->margin[ತುದಿ_2])
					*max_neg_margin = -box->margin[ತುದಿ_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			ಯದಿ (box->flags & MAKE_HEIGHT ||
					box->border[ತುದಿ_2].width ||
					box->padding[ತುದಿ_2] ||
					(box->style &&
					css_computed_overflow_y(box->style) !=
					CSS_OVERFLOW_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_first_child(box))) {
				/* Collapse to this box; return it */
				ಪ್ರತ್ಯಾಗಮ box;
			}
		}


		/* Find next box */
		ಯದಿ (box->type == BOX_BLOCK && !box->object && box->children &&
				box->style &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} ಅನ್ಯಥಾ {
			ಯದಿ (!box->next) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				ಕುರು {
					/* Apply bottom margin */
					ಯದಿ (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					ಅನ್ಯಥಾ ಯದಿ (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->parent;
				} ಯಾವತ್ (box != block && !box->next);

				ಯದಿ (box == block) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					ಪ್ರತ್ಯಾಗಮ block;
				}
			}

			/* Apply bottom margin */
			ಯದಿ (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			ಅನ್ಯಥಾ ಯದಿ (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->next;

			/* Get margins */
			ಯದಿ (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ,
						ರಿಕ್ತ, ರಿಕ್ತ, box->margin,
						box->padding, box->border);
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ layout_clear(ಸಂರಚನಾ box *fl, ಗಣನಾ css_clear_e clear)
{
	ಪೂರ್ಣಾಙ್ಕ y = 0;
	ಪ್ರತಿ (; fl; fl = fl->next_float) {
		ಯದಿ ((clear == CSS_CLEAR_ಎಡ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ಎಡ)
			ಯದಿ (y < fl->y + fl->height)
				y = fl->y + fl->height;
		ಯದಿ ((clear == CSS_CLEAR_ಬಲ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ಬಲ)
			ಯದಿ (y < fl->y + fl->height)
				y = fl->y + fl->height;
	}
	ಪ್ರತ್ಯಾಗಮ y;
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
ಸ್ಥಿರ ಶೂನ್ಯ
find_sides(ಸಂರಚನಾ box *fl,
	   ಪೂರ್ಣಾಙ್ಕ y0, ಪೂರ್ಣಾಙ್ಕ y1,
	   ಪೂರ್ಣಾಙ್ಕ *x0, ಪೂರ್ಣಾಙ್ಕ *x1,
	   ಸಂರಚನಾ box **ಎಡ,
	   ಸಂರಚನಾ box **ಬಲ)
{
	ಪೂರ್ಣಾಙ್ಕ fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*ಎಡ = *ಬಲ = 0;
	ಪ್ರತಿ (; fl; fl = fl->next_float) {
		fy1 = fl->y + fl->height;
		ಯದಿ (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			ಪ್ರತ್ಯಾಗಮ;
		}
		fy0 = fl->y;
		ಯದಿ (y0 < fy1 && fy0 <= y1) {
			ಯದಿ (fl->type == BOX_FLOAT_ಎಡ) {
				fx1 = fl->x + fl->width;
				ಯದಿ (*x0 < fx1) {
					*x0 = fx1;
					*ಎಡ = fl;
				}
			} ಅನ್ಯಥಾ {
				fx0 = fl->x;
				ಯದಿ (fx0 < *x1) {
					*x1 = fx0;
					*ಬಲ = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *ಎಡ, *ಬಲ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
layout_solve_width(ಸಂರಚನಾ box *box,
		   ಪೂರ್ಣಾಙ್ಕ available_width,
		   ಪೂರ್ಣಾಙ್ಕ width,
		   ಪೂರ್ಣಾಙ್ಕ lm,
		   ಪೂರ್ಣಾಙ್ಕ rm,
		   ಪೂರ್ಣಾಙ್ಕ max_width,
		   ಪೂರ್ಣಾಙ್ಕ min_width)
{
	bool auto_width = false;

	/* Increase specified left/right margins */
	ಯದಿ (box->margin[ಎಡ_2] != AUTO && box->margin[ಎಡ_2] < lm &&
			box->margin[ಎಡ_2] >= 0)
		box->margin[ಎಡ_2] = lm;
	ಯದಿ (box->margin[ಬಲ_2] != AUTO && box->margin[ಬಲ_2] < rm &&
			box->margin[ಬಲ_2] >= 0)
		box->margin[ಬಲ_2] = rm;

	/* Find width */
	ಯದಿ (width == AUTO) {
		ಪೂರ್ಣಾಙ್ಕ margin_ಎಡ = box->margin[ಎಡ_2];
		ಪೂರ್ಣಾಙ್ಕ margin_ಬಲ = box->margin[ಬಲ_2];

		ಯದಿ (margin_ಎಡ == AUTO) {
			margin_ಎಡ = lm;
		}
		ಯದಿ (margin_ಬಲ == AUTO) {
			margin_ಬಲ = rm;
		}

		width = available_width -
				(margin_ಎಡ + box->border[ಎಡ_2].width +
				box->padding[ಎಡ_2] + box->padding[ಬಲ_2] +
				box->border[ಬಲ_2].width + margin_ಬಲ);
		width = width < 0 ? 0 : width;
		auto_width = true;
	}

	ಯದಿ (max_width >= 0 && width > max_width) {
		/* max-width is admissable and width exceeds max-width */
		width = max_width;
		auto_width = false;
	}

	ಯದಿ (min_width > 0 && width < min_width) {
		/* min-width is admissable and width is less than max-width */
		width = min_width;
		auto_width = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	ಯದಿ (auto_width) {
		/* any other 'auto' become 0 or the minimum required values */
		ಯದಿ (box->margin[ಎಡ_2] == AUTO) {
			box->margin[ಎಡ_2] = lm;
		}
		ಯದಿ (box->margin[ಬಲ_2] == AUTO) {
			box->margin[ಬಲ_2] = rm;
		}
		ಪ್ರತ್ಯಾಗಮ width;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	ಯದಿ (box->margin[ಎಡ_2] != AUTO && box->margin[ಬಲ_2] != AUTO &&
			box->parent != ರಿಕ್ತ && box->parent->style != ರಿಕ್ತ) {
		ವಿಕಲ್ಪ (css_computed_text_align(box->parent->style)) {
		ಪಕ್ಷ CSS_TEXT_ALIGN_LIBCSS_ಬಲ:
			box->margin[ಎಡ_2] = AUTO;
			box->margin[ಬಲ_2] = 0;
			ಭಙ್ಗ;
		ಪಕ್ಷ CSS_TEXT_ALIGN_LIBCSS_CENTER:
			box->margin[ಎಡ_2] = box->margin[ಬಲ_2] = AUTO;
			ಭಙ್ಗ;
		ಪಕ್ಷ CSS_TEXT_ALIGN_LIBCSS_ಎಡ:
			box->margin[ಎಡ_2] = 0;
			box->margin[ಬಲ_2] = AUTO;
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			/* Leave it alone; no HTML alignment */
			ಭಙ್ಗ;
		}
	}

	ಯದಿ (box->margin[ಎಡ_2] == AUTO && box->margin[ಬಲ_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[ಎಡ_2] = box->margin[ಬಲ_2] =
				(available_width - lm - rm -
				(box->border[ಎಡ_2].width + box->padding[ಎಡ_2] +
				width + box->padding[ಬಲ_2] +
				box->border[ಬಲ_2].width)) / 2;

		ಯದಿ (box->margin[ಎಡ_2] < 0) {
			box->margin[ಬಲ_2] += box->margin[ಎಡ_2];
			box->margin[ಎಡ_2] = 0;
		}

		box->margin[ಎಡ_2] += lm;

	} ಅನ್ಯಥಾ ಯದಿ (box->margin[ಎಡ_2] == AUTO) {
		box->margin[ಎಡ_2] = available_width - lm -
				(box->border[ಎಡ_2].width + box->padding[ಎಡ_2] +
				width + box->padding[ಬಲ_2] +
				box->border[ಬಲ_2].width + box->margin[ಬಲ_2]);
		box->margin[ಎಡ_2] = box->margin[ಎಡ_2] < lm
				? lm : box->margin[ಎಡ_2];
	} ಅನ್ಯಥಾ {
		/* margin-right auto or "over-constrained" */
		box->margin[ಬಲ_2] = available_width - rm -
				(box->margin[ಎಡ_2] + box->border[ಎಡ_2].width +
				 box->padding[ಎಡ_2] + width +
				 box->padding[ಬಲ_2] +
				 box->border[ಬಲ_2].width);
	}

	ಪ್ರತ್ಯಾಗಮ width;
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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_block_find_dimensions(ಧ್ರುವ css_unit_ctx *unit_len_ctx,
			     ಪೂರ್ಣಾಙ್ಕ available_width,
			     ಪೂರ್ಣಾಙ್ಕ viewport_height,
			     ಪೂರ್ಣಾಙ್ಕ lm,
			     ಪೂರ್ಣಾಙ್ಕ rm,
			     ಸಂರಚನಾ box *box)
{
	ಪೂರ್ಣಾಙ್ಕ width, max_width, min_width;
	ಪೂರ್ಣಾಙ್ಕ height, max_height, min_height;
	ಪೂರ್ಣಾಙ್ಕ *margin = box->margin;
	ಪೂರ್ಣಾಙ್ಕ *padding = box->padding;
	ಸಂರಚನಾ box_border *border = box->border;
	ಧ್ರುವ css_computed_style *style = box->style;

	layout_find_dimensions(unit_len_ctx, available_width, viewport_height, box,
			style, &width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	ಯದಿ (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	}

	box->width = layout_solve_width(box, available_width, width, lm, rm,
			max_width, min_width);
	box->height = height;

	ಯದಿ (margin[ತುದಿ_2] == AUTO)
		margin[ತುದಿ_2] = 0;
	ಯದಿ (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_block_add_scrollbar(ಸಂರಚನಾ box *box, ಪೂರ್ಣಾಙ್ಕ which)
{
	ಗಣನಾ css_overflow_e overflow_x, overflow_y;

	ಪ್ರತಿಜ್ಞಾ(box->type == BOX_BLOCK && (which == ಬಲ_2 || which == BOTTOM));

	ಯದಿ (box->style == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ;

	overflow_x = css_computed_overflow_x(box->style);
	overflow_y = css_computed_overflow_y(box->style);

	ಯದಿ (which == BOTTOM &&
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		ಯದಿ (box->height != AUTO &&
				(overflow_x == CSS_OVERFLOW_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_WIDTH;
		}

	} ಅನ್ಯಥಾ ಯದಿ (which == ಬಲ_2 &&
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		ಗಣನಾ css_height_e htype;
		css_fixed height = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_height(box->style, &height, &hunit);

		ಯದಿ (which == ಬಲ_2 && box->width != AUTO &&
				htype == CSS_HEIGHT_SET &&
				(overflow_y == CSS_OVERFLOW_SCROLL ||
				box_vscrollbar_present(box))) {
			box->width -= SCROLLBAR_WIDTH;
			box->padding[ಬಲ_2] += SCROLLBAR_WIDTH;
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
ಸ್ಥಿರ ಶೂನ್ಯ layout_move_children(ಸಂರಚನಾ box *box, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಪ್ರತಿಜ್ಞಾ(box);

	ಪ್ರತಿ (box = box->children; box; box = box->next) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_table(
		ಸಂರಚನಾ box *table,
		ಪೂರ್ಣಾಙ್ಕ available_width,
		html_content *content)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ columns = table->columns;  /* total columns */
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	unsigned ಪೂರ್ಣಾಙ್ಕ *row_span;
	ಪೂರ್ಣಾಙ್ಕ *excess_y;
	ಪೂರ್ಣಾಙ್ಕ table_width, min_width = 0, max_width = 0;
	ಪೂರ್ಣಾಙ್ಕ required_width = 0;
	ಪೂರ್ಣಾಙ್ಕ x, remainder = 0, ಎಣಿಕೆ = 0;
	ಪೂರ್ಣಾಙ್ಕ table_height = 0;
	ಪೂರ್ಣಾಙ್ಕ min_height = 0;
	ಪೂರ್ಣಾಙ್ಕ *xs;  /* array of column x positions */
	ಪೂರ್ಣಾಙ್ಕ auto_width;
	ಪೂರ್ಣಾಙ್ಕ spare_width;
	ಪೂರ್ಣಾಙ್ಕ relative_sum = 0;
	ಪೂರ್ಣಾಙ್ಕ border_spacing_h = 0, border_spacing_v = 0;
	ಪೂರ್ಣಾಙ್ಕ spare_height;
	ಪೂರ್ಣಾಙ್ಕ positioned_columns = 0;
	ಸಂರಚನಾ box *containing_block = ರಿಕ್ತ;
	ಸಂರಚನಾ box *c;
	ಸಂರಚನಾ box *row;
	ಸಂರಚನಾ box *row_group;
	ಸಂರಚನಾ box **row_span_cell;
	ಸಂರಚನಾ column *col;
	ಧ್ರುವ css_computed_style *style = table->style;
	ಗಣನಾ css_height_e htype;
	css_fixed ಮೌಲ್ಯ = 0;
	css_unit unit = CSS_UNIT_PX;

	ಪ್ರತಿಜ್ಞಾ(table->type == BOX_TABLE);
	ಪ್ರತಿಜ್ಞಾ(style);
	ಪ್ರತಿಜ್ಞಾ(table->children && table->children->children);
	ಪ್ರತಿಜ್ಞಾ(columns);

	/* allocate working buffers */
	col = malloc(columns * ಪರಿಮಾಣ col[0]);
	excess_y = malloc(columns * ಪರಿಮಾಣ excess_y[0]);
	row_span = malloc(columns * ಪರಿಮಾಣ row_span[0]);
	row_span_cell = malloc(columns * ಪರಿಮಾಣ row_span_cell[0]);
	xs = malloc((columns + 1) * ಪರಿಮಾಣ xs[0]);
	ಯದಿ (!col || !xs || !row_span || !excess_y || !row_span_cell) {
		ಮುಕ್ತ(col);
		ಮುಕ್ತ(excess_y);
		ಮುಕ್ತ(row_span);
		ಮುಕ್ತ(row_span_cell);
		ಮುಕ್ತ(xs);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	memcpy(col, table->col, ಪರಿಮಾಣ(col[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1, table,
			style, 0, 0, 0, 0, 0, 0, table->margin, table->padding,
			table->border);
	ಪ್ರತಿ (row_group = table->children; row_group;
			row_group = row_group->next) {
		ಪ್ರತಿ (row = row_group->children; row; row = row->next) {
			ಪ್ರತಿ (c = row->children; c; c = c->next) {
				ಗಣನಾ css_overflow_e overflow_x;
				ಗಣನಾ css_overflow_e overflow_y;

				ಪ್ರತಿಜ್ಞಾ(c->style);
				table_used_border_for_cell(
						&content->unit_len_ctx, c);
				layout_find_dimensions(&content->unit_len_ctx,
						available_width, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				overflow_x = css_computed_overflow_x(c->style);
				overflow_y = css_computed_overflow_y(c->style);

				ಯದಿ (overflow_x == CSS_OVERFLOW_SCROLL ||
						overflow_x ==
						CSS_OVERFLOW_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_WIDTH;
				}
				ಯದಿ (overflow_y == CSS_OVERFLOW_SCROLL ||
						overflow_y ==
						CSS_OVERFLOW_AUTO) {
					c->padding[ಬಲ_2] += SCROLLBAR_WIDTH;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	ಯದಿ (css_computed_border_collapse(style) ==
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
	ಯದಿ (css_computed_width_px(style, &content->unit_len_ctx,
			available_width, &table_width) == CSS_WIDTH_SET) {
		/* specified width includes border */
		table_width -= table->border[ಎಡ_2].width +
				table->border[ಬಲ_2].width;
		table_width = table_width < 0 ? 0 : table_width;

		auto_width = table_width;
	} ಅನ್ಯಥಾ {
		table_width = AUTO;
		auto_width = available_width -
				((table->margin[ಎಡ_2] == AUTO ? 0 :
						table->margin[ಎಡ_2]) +
				 table->border[ಎಡ_2].width +
				 table->padding[ಎಡ_2] +
				 table->padding[ಬಲ_2] +
				 table->border[ಬಲ_2].width +
				 (table->margin[ಬಲ_2] == AUTO ? 0 :
						table->margin[ಬಲ_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_height(style, &ಮೌಲ್ಯ, &unit);
	ಯದಿ (htype == CSS_HEIGHT_SET) {
		ಯದಿ (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			ಯದಿ (css_computed_position(table->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				ಪ್ರತಿಜ್ಞಾ(table->float_container);
				containing_block = table->float_container;
			} ಅನ್ಯಥಾ ಯದಿ (table->float_container &&
					css_computed_position(table->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(table->style) ==
						CSS_FLOAT_ಎಡ ||
					css_computed_float(table->style) ==
						CSS_FLOAT_ಬಲ)) {
				/* Table is a float */
				ಪ್ರತಿಜ್ಞಾ(table->parent && table->parent->parent &&
						table->parent->parent->parent);
				containing_block =
						table->parent->parent->parent;
			} ಅನ್ಯಥಾ ಯದಿ (table->parent && table->parent->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_block = table->parent;
			} ಅನ್ಯಥಾ ಯದಿ (table->parent && table->parent->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				ಪ್ರತಿಜ್ಞಾ(table->parent->parent);
				containing_block = table->parent->parent;
			}

			ಯದಿ (containing_block) {
				css_fixed ignored = 0;

				htype = css_computed_height(
						containing_block->style,
						&ignored, &unit);
			}

			ಯದಿ (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(table->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_HEIGHT_SET)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_height = FPCT_OF_INT_TOINT(ಮೌಲ್ಯ,
						containing_block->height);
			}
		} ಅನ್ಯಥಾ {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_height = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		}
	}

	/* calculate width required by cells */
	ಪ್ರತಿ (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      table,
		      i,
		      ((ಧ್ರುವ ವರ್ಣ *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[col[i].type],
		      col[i].width,
		      col[i].min,
		      col[i].max);


		ಯದಿ (col[i].positioned) {
			positioned_columns++;
			ಅನುವರ್ತ;
		} ಅನ್ಯಥಾ ಯದಿ (col[i].type == COLUMN_WIDTH_FIXED) {
			ಯದಿ (col[i].width < col[i].min)
				col[i].width = col[i].max = col[i].min;
			ಅನ್ಯಥಾ
				col[i].min = col[i].max = col[i].width;
			required_width += col[i].width;
		} ಅನ್ಯಥಾ ಯದಿ (col[i].type == COLUMN_WIDTH_PERCENT) {
			ಪೂರ್ಣಾಙ್ಕ width = col[i].width * auto_width / 100;
			required_width += col[i].min < width ? width :
					col[i].min;
		} ಅನ್ಯಥಾ
			required_width += col[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_width);
	}
	required_width += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", table_width,
	      table->min_width, table->max_width, auto_width, required_width);

	ಯದಿ (auto_width < required_width) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		ಪ್ರತಿ (i = 0; i != columns; i++) {
			ಯದಿ (col[i].type == COLUMN_WIDTH_RELATIVE)
				ಅನುವರ್ತ;
			ಯದಿ (col[i].type == COLUMN_WIDTH_PERCENT) {
				col[i].max = auto_width * col[i].width / 100;
				ಯದಿ (col[i].max < col[i].min)
					col[i].max = col[i].min;
			}
			min_width += col[i].min;
			max_width += col[i].max;
		}
	} ಅನ್ಯಥಾ {
		/* take percentages exactly */
		ಪ್ರತಿ (i = 0; i != columns; i++) {
			ಯದಿ (col[i].type == COLUMN_WIDTH_RELATIVE)
				ಅನುವರ್ತ;
			ಯದಿ (col[i].type == COLUMN_WIDTH_PERCENT) {
				ಪೂರ್ಣಾಙ್ಕ width = auto_width * col[i].width / 100;
				ಯದಿ (width < col[i].min)
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
	ಪ್ರತಿ (i = 0; i != columns; i++) {
		ಯದಿ (col[i].type == COLUMN_WIDTH_RELATIVE)
			relative_sum += col[i].width;
		ಅನ್ಯಥಾ ಯದಿ (col[i].type == COLUMN_WIDTH_FIXED)
			spare_width -= col[i].width;
		ಅನ್ಯಥಾ
			spare_width -= col[i].min;
	}
	spare_width -= (columns + 1) * border_spacing_h;
	ಯದಿ (relative_sum != 0) {
		ಯದಿ (spare_width < 0)
			spare_width = 0;
		ಪ್ರತಿ (i = 0; i != columns; i++) {
			ಯದಿ (col[i].type == COLUMN_WIDTH_RELATIVE) {
				col[i].min = ceil(col[i].max =
						(ಭಿನ್ನ) spare_width
						* (ಭಿನ್ನ) col[i].width
						/ relative_sum);
				min_width += col[i].min;
				max_width += col[i].max;
			}
		}
	}
	min_width += (columns + 1) * border_spacing_h;
	max_width += (columns + 1) * border_spacing_h;

	ಯದಿ (auto_width <= min_width) {
		/* not enough space: minimise column widths */
		ಪ್ರತಿ (i = 0; i < columns; i++) {
			col[i].width = col[i].min;
		}
		table_width = min_width;
	} ಅನ್ಯಥಾ ಯದಿ (max_width <= auto_width) {
		/* more space than maximum width */
		ಯದಿ (table_width == AUTO) {
			/* for auto-width tables, make columns max width */
			ಪ್ರತಿ (i = 0; i < columns; i++) {
				col[i].width = col[i].max;
			}
			table_width = max_width;
		} ಅನ್ಯಥಾ {
			/* for fixed-width tables, distribute the extra space
			 * too */
			unsigned ಪೂರ್ಣಾಙ್ಕ flexible_columns = 0;
			ಪ್ರತಿ (i = 0; i != columns; i++)
				ಯದಿ (col[i].type != COLUMN_WIDTH_FIXED)
					flexible_columns++;
			ಯದಿ (flexible_columns == 0) {
				ಪೂರ್ಣಾಙ್ಕ extra = (table_width - max_width) / columns;
				remainder = (table_width - max_width) -
						(extra * columns);
				ಪ್ರತಿ (i = 0; i != columns; i++) {
					col[i].width = col[i].max + extra;
					ಎಣಿಕೆ -= remainder;
					ಯದಿ (ಎಣಿಕೆ < 0) {
						col[i].width++;
						ಎಣಿಕೆ += columns;
					}
				}

			} ಅನ್ಯಥಾ {
				ಪೂರ್ಣಾಙ್ಕ extra = (table_width - max_width) /
						flexible_columns;
				remainder = (table_width - max_width) -
						(extra * flexible_columns);
				ಪ್ರತಿ (i = 0; i != columns; i++)
					ಯದಿ (col[i].type != COLUMN_WIDTH_FIXED) {
						col[i].width = col[i].max +
								extra;
						ಎಣಿಕೆ -= remainder;
						ಯದಿ (ಎಣಿಕೆ < 0) {
							col[i].width++;
							ಎಣಿಕೆ += flexible_columns;
						}
					}
			}
		}
	} ಅನ್ಯಥಾ {
		/* space between min and max: fill it exactly */
		ಭಿನ್ನ scale = (ಭಿನ್ನ) (auto_width - min_width) /
				(ಭಿನ್ನ) (max_width - min_width);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		ಪ್ರತಿ (i = 0; i < columns; i++) {
			col[i].width = col[i].min + (ಪೂರ್ಣಾಙ್ಕ) (0.5 +
					(col[i].max - col[i].min) * scale);
		}
		table_width = auto_width;
	}

	xs[0] = x = border_spacing_h;
	ಪ್ರತಿ (i = 0; i != columns; i++) {
		ಯದಿ (!col[i].positioned)
			x += col[i].width + border_spacing_h;
		xs[i + 1] = x;
		row_span[i] = 0;
		excess_y[i] = 0;
		row_span_cell[i] = 0;
	}

	/* position cells */
	table_height = border_spacing_v;
	ಪ್ರತಿ (row_group = table->children; row_group;
			row_group = row_group->next) {
		ಪೂರ್ಣಾಙ್ಕ row_group_height = 0;
		ಪ್ರತಿ (row = row_group->children; row; row = row->next) {
			ಪೂರ್ಣಾಙ್ಕ row_height = 0;

			htype = css_computed_height(row->style, &ಮೌಲ್ಯ, &unit);
			ಯದಿ (htype == CSS_HEIGHT_SET && unit != CSS_UNIT_PCT) {
				row_height = FIXTOINT(css_unit_len2device_px(
						row->style,
						&content->unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
			ಪ್ರತಿ (c = row->children; c; c = c->next) {
				ಪ್ರತಿಜ್ಞಾ(c->style);
				c->width = xs[c->start_column + c->columns] -
						xs[c->start_column] -
						border_spacing_h -
						c->border[ಎಡ_2].width -
						c->padding[ಎಡ_2] -
						c->padding[ಬಲ_2] -
						c->border[ಬಲ_2].width;
				c->float_children = 0;
				c->cached_place_below_level = 0;

				c->height = AUTO;
				ಯದಿ (!layout_block_context(c, -1, content)) {
					ಮುಕ್ತ(col);
					ಮುಕ್ತ(excess_y);
					ಮುಕ್ತ(row_span);
					ಮುಕ್ತ(row_span_cell);
					ಮುಕ್ತ(xs);
					ಪ್ರತ್ಯಾಗಮ false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->height;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_height(c->style,
						&ಮೌಲ್ಯ, &unit);

				ಯದಿ (htype == CSS_HEIGHT_SET &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					ಪೂರ್ಣಾಙ್ಕ h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							ಮೌಲ್ಯ, unit));
					ಯದಿ (c->height < h)
						c->height = h;
				}
				/* specified row height is treated as a minimum
				 */
				ಯದಿ (c->height < row_height)
					c->height = row_height;
				c->x = xs[c->start_column] +
						c->border[ಎಡ_2].width;
				c->y = c->border[ತುದಿ_2].width;
				ಪ್ರತಿ (i = 0; i != c->columns; i++) {
					row_span[c->start_column + i] = c->rows;
					excess_y[c->start_column + i] =
							c->border[ತುದಿ_2].width +
							c->padding[ತುದಿ_2] +
							c->height +
							c->padding[BOTTOM] +
							c->border[BOTTOM].width;
					row_span_cell[c->start_column + i] = 0;
				}
				row_span_cell[c->start_column] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[ತುದಿ_2].width -
						c->padding[ತುದಿ_2] -
						c->height -
						c->border[BOTTOM].width;
			}
			ಪ್ರತಿ (i = 0; i != columns; i++)
				ಯದಿ (row_span[i] != 0)
					row_span[i]--;
				ಅನ್ಯಥಾ
					row_span_cell[i] = 0;
			ಯದಿ (row->next || row_group->next) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				ಪ್ರತಿ (i = 0; i != columns; i++)
					ಯದಿ (row_span[i] == 0 && row_height <
							excess_y[i])
						row_height = excess_y[i];
			} ಅನ್ಯಥಾ {
				/* except in the last row */
				ಪ್ರತಿ (i = 0; i != columns; i++)
					ಯದಿ (row_height < excess_y[i])
						row_height = excess_y[i];
			}
			ಪ್ರತಿ (i = 0; i != columns; i++) {
				ಯದಿ (row_height < excess_y[i])
					excess_y[i] -= row_height;
				ಅನ್ಯಥಾ
					excess_y[i] = 0;
				ಯದಿ (row_span_cell[i] != 0)
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
	ಪ್ರತಿ (row_group = table->children; row_group;
			row_group = row_group->next) {
		ಪ್ರತಿ (row = row_group->children; row; row = row->next) {
			ಪ್ರತಿ (c = row->children; c; c = c->next) {
				ಗಣನಾ css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				spare_height = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->height - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &ಮೌಲ್ಯ, &unit);

				ವಿಕಲ್ಪ (vertical_align) {
				ಪಕ್ಷ CSS_VERTICAL_ALIGN_SUB:
				ಪಕ್ಷ CSS_VERTICAL_ALIGN_SUPER:
				ಪಕ್ಷ CSS_VERTICAL_ALIGN_TEXT_ತುದಿ:
				ಪಕ್ಷ CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				ಪಕ್ಷ CSS_VERTICAL_ALIGN_SET:
				ಪಕ್ಷ CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				ಪಕ್ಷ CSS_VERTICAL_ALIGN_ತುದಿ:
					ಭಙ್ಗ;
				ಪಕ್ಷ CSS_VERTICAL_ALIGN_ಮಧ್ಯ:
					c->padding[ತುದಿ_2] += spare_height / 2;
					c->padding[BOTTOM] -= spare_height / 2;
					layout_move_children(c, 0,
							spare_height / 2);
					ಭಙ್ಗ;
				ಪಕ್ಷ CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[ತುದಿ_2] += spare_height;
					c->padding[BOTTOM] -= spare_height;
					layout_move_children(c, 0,
							spare_height);
					ಭಙ್ಗ;
				ಪಕ್ಷ CSS_VERTICAL_ALIGN_INHERIT:
					ಪ್ರತಿಜ್ಞಾ(0);
					ಭಙ್ಗ;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	ಯದಿ (table->margin[ತುದಿ_2] == AUTO)
		table->margin[ತುದಿ_2] = 0;
	ಯದಿ (table->margin[BOTTOM] == AUTO)
		table->margin[BOTTOM] = 0;

	ಮುಕ್ತ(col);
	ಮುಕ್ತ(excess_y);
	ಮುಕ್ತ(row_span);
	ಮುಕ್ತ(row_span_cell);
	ಮುಕ್ತ(xs);

	table->width = table_width;
	table->height = table_height;

	ಪ್ರತ್ಯಾಗಮ true;
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
ಸ್ಥಿರ bool layout_apply_minmax_height(
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಸಂರಚನಾ box *box,
		ಸಂರಚನಾ box *container)
{
	ಪೂರ್ಣಾಙ್ಕ h;
	ಸಂರಚನಾ box *containing_block = ರಿಕ್ತ;
	bool updated = false;

	/* Find containing block for percentage heights */
	ಯದಿ (box->style != ರಿಕ್ತ && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		ಪ್ರತಿಜ್ಞಾ(container);
		containing_block = container;
	} ಅನ್ಯಥಾ ಯದಿ (box->float_container && box->style != ರಿಕ್ತ &&
			(css_computed_float(box->style) == CSS_FLOAT_ಎಡ ||
			 css_computed_float(box->style) == CSS_FLOAT_ಬಲ)) {
		/* Box is a float */
		ಪ್ರತಿಜ್ಞಾ(box->parent && box->parent->parent &&
				box->parent->parent->parent);
		containing_block = box->parent->parent->parent;
	} ಅನ್ಯಥಾ ಯದಿ (box->parent && box->parent->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_block = box->parent;
	} ಅನ್ಯಥಾ ಯದಿ (box->parent && box->parent->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		ಪ್ರತಿಜ್ಞಾ(box->parent->parent);
		containing_block = box->parent->parent;
	}

	ಯದಿ (box->style) {
		ಗಣನಾ css_height_e htype = CSS_HEIGHT_AUTO;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		ಯದಿ (containing_block) {
			htype = css_computed_height(containing_block->style,
					&ಮೌಲ್ಯ, &unit);
		}

		/* max-height */
		ಯದಿ (css_computed_max_height(box->style, &ಮೌಲ್ಯ, &unit) ==
				CSS_MAX_HEIGHT_SET) {
			ಯದಿ (unit == CSS_UNIT_PCT) {
				ಯದಿ (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(ಮೌಲ್ಯ,
						containing_block->height);
					ಯದಿ (h < box->height) {
						box->height = h;
						updated = true;
					}
				}
			} ಅನ್ಯಥಾ {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
				ಯದಿ (h < box->height) {
					box->height = h;
					updated = true;
				}
			}
		}

		/* min-height */
		ಯದಿ (ns_computed_min_height(box->style, &ಮೌಲ್ಯ, &unit) ==
				CSS_MIN_HEIGHT_SET) {
			ಯದಿ (unit == CSS_UNIT_PCT) {
				ಯದಿ (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(ಮೌಲ್ಯ,
						containing_block->height);
					ಯದಿ (h > box->height) {
						box->height = h;
						updated = true;
					}
				}
			} ಅನ್ಯಥಾ {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
				ಯದಿ (h > box->height) {
					box->height = h;
					updated = true;
				}
			}
		}
	}
	ಪ್ರತ್ಯಾಗಮ updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
ಸ್ಥಿರ bool layout_block_object(ಸಂರಚನಾ box *block)
{
	ಪ್ರತಿಜ್ಞಾ(block);
	ಪ್ರತಿಜ್ಞಾ(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_TABLE ||
			block->type == BOX_TABLE_CELL);
	ಪ್ರತಿಜ್ಞಾ(block->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", block,
	      hlcache_handle_get_url(block->object), block->width);

	ಯದಿ (content_can_reformat(block->object)) {
		content_reformat(block->object, false, block->width, 1);
	} ಅನ್ಯಥಾ {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
ಸ್ಥಿರ ಶೂನ್ಯ add_float_to_container(ಸಂರಚನಾ box *cont, ಸಂರಚನಾ box *b)
{
	ಸಂರಚನಾ box *box = cont->float_children;
	ಪೂರ್ಣಾಙ್ಕ b_bottom = b->y + b->height;

	ಪ್ರತಿಜ್ಞಾ(b->type == BOX_FLOAT_ಎಡ || b->type == BOX_FLOAT_ಬಲ);

	ಯದಿ (box == ರಿಕ್ತ) {
		/* No other float children */
		b->next_float = ರಿಕ್ತ;
		cont->float_children = b;
		ಪ್ರತ್ಯಾಗಮ;
	} ಅನ್ಯಥಾ ಯದಿ (b_bottom >= box->y + box->height) {
		/* Goes at start of list */
		b->next_float = cont->float_children;
		cont->float_children = b;
	} ಅನ್ಯಥಾ {
		ಸಂರಚನಾ box *prev = ರಿಕ್ತ;
		ಯಾವತ್ (box != ರಿಕ್ತ && b_bottom < box->y + box->height) {
			prev = box;
			box = box->next_float;
		}
		ಯದಿ (prev != ರಿಕ್ತ) {
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
ಸ್ಥಿರ bool
layout_text_box_split(html_content *content,
		      plot_font_style_t *fstyle,
		      ಸಂರಚನಾ box *split_box,
		      size_t new_length,
		      ಪೂರ್ಣಾಙ್ಕ new_width)
{
	ಪೂರ್ಣಾಙ್ಕ space_width = split_box->space;
	ಸಂರಚನಾ box *c2;
	ಧ್ರುವ ಸಂರಚನಾ gui_layout_table *font_func = content->font_func;
	bool space = (split_box->text[new_length] == ' ');
	ಪೂರ್ಣಾಙ್ಕ used_length = new_length + (space ? 1 : 0);

	ಯದಿ ((space && space_width == 0) || space_width == UNKNOWN_WIDTH) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->width(fstyle, " ", 1, &space_width);
	}

	ಯದಿ (split_box->space == UNKNOWN_WIDTH)
		split_box->space = space_width;
	ಯದಿ (!space)
		space_width = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, split_box, ಪರಿಮಾಣ *c2);
	ಯದಿ (!c2)
		ಪ್ರತ್ಯಾಗಮ false;
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
	ಯದಿ (c2->next)
		c2->next->prev = c2;
	ಅನ್ಯಥಾ
		c2->parent->last = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      split_box,
	      split_box->length,
	      (ಪೂರ್ಣಾಙ್ಕ)split_box->length,
	      split_box->text);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->length,
	      (ಪೂರ್ಣಾಙ್ಕ)c2->length,
	      c2->text);

	ಪ್ರತ್ಯಾಗಮ true;
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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_float_find_dimensions(
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಪೂರ್ಣಾಙ್ಕ available_width,
		ಧ್ರುವ css_computed_style *style,
		ಸಂರಚನಾ box *box)
{
	ಪೂರ್ಣಾಙ್ಕ width, height, max_width, min_width, max_height, min_height;
	ಪೂರ್ಣಾಙ್ಕ *margin = box->margin;
	ಪೂರ್ಣಾಙ್ಕ *padding = box->padding;
	ಸಂರಚನಾ box_border *border = box->border;
	ಗಣನಾ css_overflow_e overflow_x = css_computed_overflow_x(style);
	ಗಣನಾ css_overflow_e overflow_y = css_computed_overflow_y(style);
	ಪೂರ್ಣಾಙ್ಕ scrollbar_width_x =
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;
	ಪೂರ್ಣಾಙ್ಕ scrollbar_width_y =
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;

	layout_find_dimensions(unit_len_ctx, available_width, -1, box, style,
			&width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	ಯದಿ (margin[ಎಡ_2] == AUTO)
		margin[ಎಡ_2] = 0;
	ಯದಿ (margin[ಬಲ_2] == AUTO)
		margin[ಬಲ_2] = 0;

	ಯದಿ (box->gadget == ರಿಕ್ತ) {
		padding[ಬಲ_2] += scrollbar_width_y;
		padding[BOTTOM] += scrollbar_width_x;
	}

	ಯದಿ (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	} ಅನ್ಯಥಾ ಯದಿ (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed size = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		ಪ್ರತಿಜ್ಞಾ(box->style);

		ಯದಿ (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			ಯದಿ (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			ಯದಿ (box->gadget->type == GADGET_FILE &&
					height == AUTO) {
				size = FLTTOFIX(1.5);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
		ಯದಿ (box->gadget->type == GADGET_TEXTAREA) {
			ಯದಿ (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			ಯದಿ (height == AUTO) {
				size = INTTOFIX(4);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
	} ಅನ್ಯಥಾ ಯದಿ (width == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		width = min(max(box->min_width, available_width),
				box->max_width);

		/* width includes margin, borders and padding */
		ಯದಿ (width == available_width) {
			width -= box->margin[ಎಡ_2] + box->border[ಎಡ_2].width +
					box->padding[ಎಡ_2] +
					box->padding[ಬಲ_2] +
					box->border[ಬಲ_2].width +
					box->margin[ಬಲ_2];
		} ಅನ್ಯಥಾ {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			ಪೂರ್ಣಾಙ್ಕ fixed = 0;
			ಭಿನ್ನ frac = 0;
			calculate_mbp_width(unit_len_ctx, box->style, ಎಡ_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_width(unit_len_ctx, box->style, ಬಲ_2,
					true, true, true, &fixed, &frac);
			ಯದಿ (fixed < 0)
				fixed = 0;

			width -= fixed;
		}

		ಯದಿ (max_width >= 0 && width > max_width) width = max_width;
		ಯದಿ (min_width >  0 && width < min_width) width = min_width;

	} ಅನ್ಯಥಾ {
		ಯದಿ (max_width >= 0 && width > max_width) width = max_width;
		ಯದಿ (min_width >  0 && width < min_width) width = min_width;
		width -= scrollbar_width_y;
	}

	box->width = width;
	box->height = height;

	ಯದಿ (margin[ತುದಿ_2] == AUTO)
		margin[ತುದಿ_2] = 0;
	ಯದಿ (margin[BOTTOM] == AUTO)
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
ಸ್ಥಿರ bool layout_float(ಸಂರಚನಾ box *b, ಪೂರ್ಣಾಙ್ಕ width, html_content *content)
{
	ಪ್ರತಿಜ್ಞಾ(b->type == BOX_TABLE ||
	       b->type == BOX_BLOCK ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_find_dimensions(&content->unit_len_ctx, width, b->style, b);
	ಯದಿ (b->type == BOX_TABLE || b->type == BOX_INLINE_FLEX) {
		ಯದಿ (b->type == BOX_TABLE) {
			ಯದಿ (!layout_table(b, width, content))
				ಪ್ರತ್ಯಾಗಮ false;
		} ಅನ್ಯಥಾ {
			ಯದಿ (!layout_flex(b, width, content))
				ಪ್ರತ್ಯಾಗಮ false;
		}
		ಯದಿ (b->margin[ಎಡ_2] == AUTO)
			b->margin[ಎಡ_2] = 0;
		ಯದಿ (b->margin[ಬಲ_2] == AUTO)
			b->margin[ಬಲ_2] = 0;
		ಯದಿ (b->margin[ತುದಿ_2] == AUTO)
			b->margin[ತುದಿ_2] = 0;
		ಯದಿ (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} ಅನ್ಯಥಾ {
		ಪ್ರತ್ಯಾಗಮ layout_block_context(b, -1, content);
	}
	ಪ್ರತ್ಯಾಗಮ true;
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
ಸ್ಥಿರ ಶೂನ್ಯ
place_float_below(ಸಂರಚನಾ box *c, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ cx, ಪೂರ್ಣಾಙ್ಕ y, ಸಂರಚನಾ box *cont)
{
	ಪೂರ್ಣಾಙ್ಕ x0, x1, yy;
	ಸಂರಚನಾ box *ಎಡ;
	ಸಂರಚನಾ box *ಬಲ;

	yy = y > cont->cached_place_below_level ?
			y : cont->cached_place_below_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      width, cx, y, cont);

	ಕುರು {
		y = yy;
		x0 = cx;
		x1 = cx + width;
		find_sides(cont->float_children, y, y + c->height, &x0, &x1,
				&ಎಡ, &ಬಲ);
		ಯದಿ (ಎಡ != 0 && ಬಲ != 0) {
			yy = (ಎಡ->y + ಎಡ->height <
					ಬಲ->y + ಬಲ->height ?
					ಎಡ->y + ಎಡ->height :
					ಬಲ->y + ಬಲ->height);
		} ಅನ್ಯಥಾ ಯದಿ (ಎಡ == 0 && ಬಲ != 0) {
			yy = ಬಲ->y + ಬಲ->height;
		} ಅನ್ಯಥಾ ಯದಿ (ಎಡ != 0 && ಬಲ == 0) {
			yy = ಎಡ->y + ಎಡ->height;
		}
	} ಯಾವತ್ ((ಎಡ != 0 || ಬಲ != 0) && (c->width > x1 - x0));

	ಯದಿ (c->type == BOX_FLOAT_ಎಡ) {
		c->x = x0;
	} ಅನ್ಯಥಾ {
		c->x = x1 - c->width;
	}
	c->y = y;
	cont->cached_place_below_level = y;
}


/**
 * Calculate line height from a style.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಸಾಲು_height(
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಧ್ರುವ css_computed_style *style)
{
	ಗಣನಾ css_ಸಾಲು_height_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed ಸಾಲು_height;

	ಪ್ರತಿಜ್ಞಾ(style);

	lhtype = css_computed_ಸಾಲು_height(style, &lhvalue, &lhunit);
	ಯದಿ (lhtype == CSS_ಸಾಲು_HEIGHT_NORMAL) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_ಸಾಲು_HEIGHT_ಸಂಖ್ಯೆ;
	}

	ಯದಿ (lhtype == CSS_ಸಾಲು_HEIGHT_ಸಂಖ್ಯೆ ||
			lhunit == CSS_UNIT_PCT) {
		ಸಾಲು_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		ಯದಿ (lhtype != CSS_ಸಾಲು_HEIGHT_ಸಂಖ್ಯೆ)
			ಸಾಲು_height = FDIV(ಸಾಲು_height, F_100);
	} ಅನ್ಯಥಾ {
		ಪ್ರತಿಜ್ಞಾ(lhunit != CSS_UNIT_PCT);

		ಸಾಲು_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	ಪ್ರತ್ಯಾಗಮ FIXTOINT(ಸಾಲು_height);
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
ಸ್ಥಿರ bool
layout_ಸಾಲು(ಸಂರಚನಾ box *first,
	    ಪೂರ್ಣಾಙ್ಕ *width,
	    ಪೂರ್ಣಾಙ್ಕ *y,
	    ಪೂರ್ಣಾಙ್ಕ cx,
	    ಪೂರ್ಣಾಙ್ಕ cy,
	    ಸಂರಚನಾ box *cont,
	    bool indent,
	    bool has_text_children,
	    html_content *content,
	    ಸಂರಚನಾ box **next_box)
{
	ಪೂರ್ಣಾಙ್ಕ height, used_height;
	ಪೂರ್ಣಾಙ್ಕ x0 = 0;
	ಪೂರ್ಣಾಙ್ಕ x1 = *width;
	ಪೂರ್ಣಾಙ್ಕ x, h, x_previous;
	ಪೂರ್ಣಾಙ್ಕ fy = cy;
	ಸಂರಚನಾ box *ಎಡ;
	ಸಂರಚನಾ box *ಬಲ;
	ಸಂರಚನಾ box *b;
	ಸಂರಚನಾ box *split_box = 0;
	ಸಂರಚನಾ box *d;
	ಸಂರಚನಾ box *br_box = 0;
	bool move_y = false;
	bool place_below = false;
	ಪೂರ್ಣಾಙ್ಕ space_before = 0, space_after = 0;
	unsigned ಪೂರ್ಣಾಙ್ಕ inline_ಎಣಿಕೆ = 0;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಧ್ರುವ ಸಂರಚನಾ gui_layout_table *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      first,
	      (ಪೂರ್ಣಾಙ್ಕ)first->length,
	      first->text,
	      *width,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	find_sides(cont->float_children, cy, cy, &x0, &x1, &ಎಡ, &ಬಲ);
	x0 -= cx;
	x1 -= cx;

	ಯದಿ (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	ಯದಿ (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	ಯದಿ (has_text_children || first->parent->parent->gadget)
		used_height = height = ಸಾಲು_height(&content->unit_len_ctx,
				first->parent->parent->style);
	ಅನ್ಯಥಾ
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		used_height = height = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	ಪ್ರತಿ (x = 0, b = first; x <= x1 - x0 && b != 0; b = b->next) {
		ಪೂರ್ಣಾಙ್ಕ min_width, max_width, min_height, max_height;

		ಪ್ರತಿಜ್ಞಾ(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		ಯದಿ (b->type == BOX_BR)
			ಭಙ್ಗ;

		ಯದಿ (lh__box_is_float_box(b))
			ಅನುವರ್ತ;
		ಯದಿ (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			ಅನುವರ್ತ;

		ಪ್ರತಿಜ್ಞಾ(b->style != ರಿಕ್ತ);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		ಯದಿ (b->type == BOX_INLINE_BLOCK ||
		    b->type == BOX_INLINE_FLEX) {
			ಯದಿ (b->max_width != UNKNOWN_WIDTH)
				ಯದಿ (!layout_float(b, *width, content))
					ಪ್ರತ್ಯಾಗಮ false;
			h = b->border[ತುದಿ_2].width + b->padding[ತುದಿ_2] + b->height +
					b->padding[BOTTOM] +
					b->border[BOTTOM].width;
			ಯದಿ (height < h)
				height = h;
			x += b->margin[ಎಡ_2] + b->border[ಎಡ_2].width +
					b->padding[ಎಡ_2] + b->width +
					b->padding[ಬಲ_2] +
					b->border[ಬಲ_2].width +
					b->margin[ಬಲ_2];
			space_after = 0;
			ಅನುವರ್ತ;
		}

		ಯದಿ (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_find_dimensions(&content->unit_len_ctx,
					*width, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			ಪ್ರತಿ (i = 0; i != 4; i++)
				ಯದಿ (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[ಎಡ_2] + b->border[ಎಡ_2].width +
					b->padding[ಎಡ_2];
			ಯದಿ (b->inline_end) {
				b->inline_end->margin[ಬಲ_2] = b->margin[ಬಲ_2];
				b->inline_end->padding[ಬಲ_2] =
						b->padding[ಬಲ_2];
				b->inline_end->border[ಬಲ_2] =
						b->border[ಬಲ_2];
			} ಅನ್ಯಥಾ {
				x += b->padding[ಬಲ_2] +
						b->border[ಬಲ_2].width +
						b->margin[ಬಲ_2];
			}
		} ಅನ್ಯಥಾ ಯದಿ (b->type == BOX_INLINE_END) {
			b->width = 0;
			ಯದಿ (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[ಬಲ_2] + b->border[ಬಲ_2].width +
					b->margin[ಬಲ_2];
			ಅನುವರ್ತ;
		}

		ಯದಿ (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->height = ಸಾಲು_height(&content->unit_len_ctx,
					b->style ? b->style :
					b->parent->parent->style);
			ಯದಿ (height < b->height)
				height = b->height;

			ಯದಿ (!b->text) {
				b->width = 0;
				space_after = 0;
				ಅನುವರ್ತ;
			}

			ಯದಿ (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				ಯದಿ (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					ಪೂರ್ಣಾಙ್ಕ opt_maxwidth = 0;
					ಸಂರಚನಾ form_option *o;

					ಪ್ರತಿ (o = b->parent->parent->gadget->
							data.select.ವಸ್ತುಗಳು; o;
							o = o->next) {
						ಪೂರ್ಣಾಙ್ಕ opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						ಯದಿ (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}
					b->width = opt_maxwidth;
					ಯದಿ (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;
				} ಅನ್ಯಥಾ {
					font_func->width(&fstyle, b->text,
							b->length, &b->width);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			ಯದಿ (b->text && (x + b->width < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->next) {
				font_func->width(&fstyle, b->text,
						 b->length, &b->width);
				b->flags |= MEASURED;
			}

			x += b->width;
			ಯದಿ (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			ಅನುವರ್ತ;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		ಪ್ರತಿಜ್ಞಾ(b->style);

		layout_find_dimensions(&content->unit_len_ctx,
				*width, -1, b, b->style,
				&b->width, &b->height,
				&max_width, &min_width,
				&max_height, &min_height,
				ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ);

		ಯದಿ (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->width, &b->height,
					min_width, max_width,
					min_height, max_height);
		} ಅನ್ಯಥಾ ಯದಿ (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			ಯದಿ (b->width == AUTO)
				b->width = 400;
			ಯದಿ (b->height == AUTO)
				b->height = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} ಅನ್ಯಥಾ {
			/* form control with no object */
			ಯದಿ (b->width == AUTO)
				b->width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			ಯದಿ (b->height == AUTO)
				b->height = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		ಯದಿ (b->object && content_can_reformat(b->object) &&
				b->width !=
				content_get_available_width(b->object)) {
			css_fixed ಮೌಲ್ಯ = 0;
			css_unit unit = CSS_UNIT_PX;
			ಗಣನಾ css_height_e htype = css_computed_height(b->style,
					&ಮೌಲ್ಯ, &unit);

			content_reformat(b->object, false, b->width, b->height);

			ಯದಿ (htype == CSS_HEIGHT_AUTO)
				b->height = content_get_height(b->object);
		}

		ಯದಿ (height < b->height)
			height = b->height;

		x += b->width;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *width;
	find_sides(cont->float_children, cy, cy + height, &x0, &x1,
			&ಎಡ, &ಬಲ);
	x0 -= cx;
	x1 -= cx;

	ಯದಿ (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	ಯದಿ (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	ಪ್ರತಿ (x = x_previous = 0, b = first; x <= x1 - x0 && b; b = b->next) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		ಯದಿ (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} ಅನ್ಯಥಾ ಯದಿ (lh__box_is_inline_flow(b)) {
			ಪ್ರತಿಜ್ಞಾ(b->width != UNKNOWN_WIDTH);

			x_previous = x;
			x += space_after;
			b->x = x;

			ಯದಿ ((b->type == BOX_INLINE && !b->inline_end) ||
					b->type == BOX_INLINE_BLOCK ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[ಎಡ_2] + b->border[ಎಡ_2].width;
				x = b->x + b->padding[ಎಡ_2] + b->width +
						b->padding[ಬಲ_2] +
						b->border[ಬಲ_2].width +
						b->margin[ಬಲ_2];
			} ಅನ್ಯಥಾ ಯದಿ (b->type == BOX_INLINE) {
				b->x += b->margin[ಎಡ_2] + b->border[ಎಡ_2].width;
				x = b->x + b->padding[ಎಡ_2] + b->width;
			} ಅನ್ಯಥಾ ಯದಿ (b->type == BOX_INLINE_END) {
				b->height = b->inline_end->height;
				x += b->padding[ಬಲ_2] +
						b->border[ಬಲ_2].width +
						b->margin[ಬಲ_2];
			} ಅನ್ಯಥಾ {
				x += b->width;
			}

			space_before = space_after;
			ಯದಿ (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			ಅನ್ಯಥಾ ಯದಿ (b->text || b->type == BOX_INLINE_END) {
				ಯದಿ (b->space == UNKNOWN_WIDTH) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} ಅನ್ಯಥಾ {
				space_after = 0;
			}
			split_box = b;
			move_y = true;
			inline_ಎಣಿಕೆ++;
		} ಅನ್ಯಥಾ ಯದಿ (b->type == BOX_BR) {
			b->x = x;
			b->width = 0;
			br_box = b;
			b = b->next;
			split_box = 0;
			move_y = true;
			ಭಙ್ಗ;

		} ಅನ್ಯಥಾ {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_place_below_level = 0;
			b->float_container = d->float_container = cont;

			ಯದಿ (!layout_float(d, *width, content))
				ಪ್ರತ್ಯಾಗಮ false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[ತುದಿ_2],
			      d->border[ತುದಿ_2].width);

			d->x = d->margin[ಎಡ_2] + d->border[ಎಡ_2].width;
			d->y = d->margin[ತುದಿ_2] + d->border[ತುದಿ_2].width;
			b->width = d->margin[ಎಡ_2] + d->border[ಎಡ_2].width +
					d->padding[ಎಡ_2] + d->width +
					d->padding[ಬಲ_2] +
					d->border[ಬಲ_2].width +
					d->margin[ಬಲ_2];
			b->height = d->margin[ತುದಿ_2] + d->border[ತುದಿ_2].width +
					d->padding[ತುದಿ_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];

			ಯದಿ (b->width > (x1 - x0) - x)
				place_below = true;
			ಯದಿ (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ಎಡ && ಎಡ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ಬಲ &&
						ಬಲ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						ಎಡ == 0 && ಬಲ == 0)) &&
					(!place_below ||
					(ಎಡ == 0 && ಬಲ == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_place_below_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				ಯದಿ (b->type == BOX_FLOAT_ಎಡ) {
					b->x = cx + x0;
					ಯದಿ (b->width > 0)
						x0 += b->width;
					ಎಡ = b;
				} ಅನ್ಯಥಾ {
					b->x = cx + x1 - b->width;
					ಯದಿ (b->width > 0)
						x1 -= b->width;
					ಬಲ = b;
				}
				b->y = cy;
			} ಅನ್ಯಥಾ {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				ಪೂರ್ಣಾಙ್ಕ fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_place_below_level) ?
						fcy :
						cont->cached_place_below_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + height : fy;

				place_float_below(b, *width, cx, fy, cont);
				fy = b->y;
				ಯದಿ (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ಎಡ && ಎಡ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ಬಲ &&
						ಬಲ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(ಎಡ != 0 || ಬಲ != 0)))) {
					/* to be cleared below existing
					 * floats */
					ಯದಿ (b->type == BOX_FLOAT_ಎಡ)
						b->x = cx;
					ಅನ್ಯಥಾ
						b->x = cx + *width - b->width;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					ಯದಿ (fcy > cont->clear_level)
						cont->clear_level = fcy;
					ಯದಿ (b->y < fcy)
						b->y = fcy;
				}
				ಯದಿ (b->type == BOX_FLOAT_ಎಡ)
					ಎಡ = b;
				ಅನ್ಯಥಾ
					ಬಲ = b;
			}
			add_float_to_container(cont, b);

			split_box = 0;
		}
	}

	ಯದಿ (x1 - x0 < x && split_box) {
		/* the last box went over the end */
		size_t split = 0;
		ಪೂರ್ಣಾಙ್ಕ w;
		bool no_wrap = css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_previous;

		ಯದಿ (!no_wrap &&
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

		ಯದಿ (split == 0)
			w = split_box->width;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      split_box,
		      (ಪೂರ್ಣಾಙ್ಕ)split_box->length,
		      split_box->text,
		      split,
		      w,
		      ಎಡ,
		      ಬಲ,
		      inline_ಎಣಿಕೆ);

		ಯದಿ ((split == 0 || x1 - x0 <= x + space_before + w) &&
				!ಎಡ && !ಬಲ && inline_ಎಣಿಕೆ == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			ಯದಿ (split == 0 || split == split_box->length) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = split_box->next;
			} ಅನ್ಯಥಾ {
				/* cut off first word for this line */
				ಯದಿ (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					ಪ್ರತ್ಯಾಗಮ false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} ಅನ್ಯಥಾ ಯದಿ ((split == 0 || x1 - x0 <= x + space_before + w) &&
				inline_ಎಣಿಕೆ == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			ಪ್ರತಿಜ್ಞಾ(ಎಡ || ಬಲ);
			used_height = 0;
			ಯದಿ (ಎಡ) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      ಎಡ->y,
				      ಎಡ->height);

				used_height = ಎಡ->y + ಎಡ->height - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      used_height);

			}
			ಯದಿ (ಬಲ && used_height <
					ಬಲ->y + ಬಲ->height - cy + 1)
				used_height = ಬಲ->y + ಬಲ->height - cy + 1;

			ಯದಿ (used_height < 0)
				used_height = 0;

			b = split_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} ಅನ್ಯಥಾ ಯದಿ (split == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = split_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} ಅನ್ಯಥಾ {
			/* fit as many words as possible */
			ಪ್ರತಿಜ್ಞಾ(split != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (ಪೂರ್ಣಾಙ್ಕ)split_box->length, split_box->text,
			      x1 - x0, split, w);

			ಯದಿ (split != split_box->length) {
				ಯದಿ (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					ಪ್ರತ್ಯಾಗಮ false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	ವಿಕಲ್ಪ (css_computed_text_align(first->parent->parent->style)) {
	ಪಕ್ಷ CSS_TEXT_ALIGN_ಬಲ:
	ಪಕ್ಷ CSS_TEXT_ALIGN_LIBCSS_ಬಲ:
		x0 = x1 - x;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TEXT_ALIGN_CENTER:
	ಪಕ್ಷ CSS_TEXT_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TEXT_ALIGN_ಎಡ:
	ಪಕ್ಷ CSS_TEXT_ALIGN_LIBCSS_ಎಡ:
	ಪಕ್ಷ CSS_TEXT_ALIGN_JUSTIFY:
		/* leave on left */
		ಭಙ್ಗ;
	ಪಕ್ಷ CSS_TEXT_ALIGN_DEFAULT:
		/* None; consider text direction */
		ವಿಕಲ್ಪ (css_computed_direction(first->parent->parent->style)) {
		ಪಕ್ಷ CSS_DIRECTION_LTR:
			/* leave on left */
			ಭಙ್ಗ;
		ಪಕ್ಷ CSS_DIRECTION_RTL:
			x0 = x1 - x;
			ಭಙ್ಗ;
		}
		ಭಙ್ಗ;
	}

	ಪ್ರತಿ (d = first; d != b; d = d->next) {
		d->flags &= ~NEW_ಸಾಲು;

		ಯದಿ (d->type == BOX_INLINE_BLOCK &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			ಅನುವರ್ತ;
		} ಅನ್ಯಥಾ ಯದಿ ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[ತುದಿ_2];

			ಯದಿ (d->type == BOX_TEXT && d->height > used_height) {
				/* text */
				used_height = d->height;
			}
		} ಅನ್ಯಥಾ ಯದಿ ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_BLOCK) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[ತುದಿ_2].width + d->margin[ತುದಿ_2];
			h = d->margin[ತುದಿ_2] + d->border[ತುದಿ_2].width +
					d->padding[ತುದಿ_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];
			ಯದಿ (used_height < h)
				used_height = h;
		}
	}

	first->flags |= NEW_ಸಾಲು;

	ಪ್ರತಿಜ್ಞಾ(b != first || (move_y && 0 < used_height && (ಎಡ || ಬಲ)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	ಪ್ರತಿ (d = first; d != b; d = d->next) {
		ಯದಿ ((d->type == BOX_INLINE && d->inline_end) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			css_fixed ಮೌಲ್ಯ = 0;
			css_unit unit = CSS_UNIT_PX;
			ವಿಕಲ್ಪ (css_computed_vertical_align(d->style, &ಮೌಲ್ಯ,
					&unit)) {
			ಪಕ್ಷ CSS_VERTICAL_ALIGN_SUPER:
			ಪಕ್ಷ CSS_VERTICAL_ALIGN_ತುದಿ:
			ಪಕ್ಷ CSS_VERTICAL_ALIGN_TEXT_ತುದಿ:
				/* already at top */
				ಭಙ್ಗ;
			ಪಕ್ಷ CSS_VERTICAL_ALIGN_SUB:
			ಪಕ್ಷ CSS_VERTICAL_ALIGN_BOTTOM:
			ಪಕ್ಷ CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				d->y += used_height - d->height;
				ಭಙ್ಗ;
			ಸ್ವಭಾವ:
			ಪಕ್ಷ CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (used_height - d->height);
				ಭಙ್ಗ;
			}
		}
	}

	/* handle clearance for br */
	ಯದಿ (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		ಪೂರ್ಣಾಙ್ಕ clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		ಯದಿ (used_height < clear_y - cy)
			used_height = clear_y - cy;
	}

	ಯದಿ (move_y)
		*y += used_height;
	*next_box = b;
	*width = x; /* return actual width */
	ಪ್ರತ್ಯಾಗಮ true;
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
ಸ್ಥಿರ bool layout_inline_container(ಸಂರಚನಾ box *inline_container, ಪೂರ್ಣಾಙ್ಕ width,
		ಸಂರಚನಾ box *cont, ಪೂರ್ಣಾಙ್ಕ cx, ಪೂರ್ಣಾಙ್ಕ cy, html_content *content)
{
	bool first_ಸಾಲು = true;
	bool has_text_children;
	ಸಂರಚನಾ box *c, *next;
	ಪೂರ್ಣಾಙ್ಕ y = 0;
	ಪೂರ್ಣಾಙ್ಕ curwidth,maxwidth = width;

	ಪ್ರತಿಜ್ಞಾ(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      width,
	      cont,
	      cx,
	      cy);


	has_text_children = false;
	ಪ್ರತಿ (c = inline_container->children; c; c = c->next) {
		bool is_pre = false;

		ಯದಿ (c->style) {
			ಗಣನಾ css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_ಸಾಲು ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		ಯದಿ ((lh__box_is_object(c) == false &&
				c->text && (c->length || is_pre)) ||
				c->type == BOX_BR)
			has_text_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	ಪ್ರತಿ (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->width;
		ಯದಿ (!layout_ಸಾಲು(c, &curwidth, &y, cx, cy + y, cont, first_ಸಾಲು,
				has_text_children, content, &next))
			ಪ್ರತ್ಯಾಗಮ false;
		maxwidth = max(maxwidth,curwidth);
		c = next;
		first_ಸಾಲು = false;
	}

	inline_container->width = maxwidth;
	inline_container->height = y;

	ಪ್ರತ್ಯಾಗಮ true;
}


/* Documented in layout_intertnal.h */
bool layout_block_context(
		ಸಂರಚನಾ box *block,
		ಪೂರ್ಣಾಙ್ಕ viewport_height,
		html_content *content)
{
	ಸಂರಚನಾ box *box;
	ಪೂರ್ಣಾಙ್ಕ cx, cy;  /**< current coordinates */
	ಪೂರ್ಣಾಙ್ಕ max_pos_margin = 0;
	ಪೂರ್ಣಾಙ್ಕ max_neg_margin = 0;
	ಪೂರ್ಣಾಙ್ಕ y = 0;
	ಪೂರ್ಣಾಙ್ಕ lm, rm;
	ಸಂರಚನಾ box *margin_collapse = ರಿಕ್ತ;
	bool in_margin = false;
	css_fixed gadget_size;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	ಪ್ರತಿಜ್ಞಾ(block->type == BOX_BLOCK ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX);
	ಪ್ರತಿಜ್ಞಾ(block->width != UNKNOWN_WIDTH);
	ಪ್ರತಿಜ್ಞಾ(block->width != AUTO);

	block->float_children = ರಿಕ್ತ;
	block->cached_place_below_level = 0;
	block->clear_level = 0;

	/* special case if the block contains an object */
	ಯದಿ (block->object) {
		ಪೂರ್ಣಾಙ್ಕ temp_width = block->width;
		ಯದಿ (!layout_block_object(block))
			ಪ್ರತ್ಯಾಗಮ false;
		layout_get_object_dimensions(block, &temp_width,
				&block->height, INT_MIN, INT_MAX,
				INT_MIN, INT_MAX);
		ಪ್ರತ್ಯಾಗಮ true;
	} ಅನ್ಯಥಾ ಯದಿ (block->flags & REPLACE_DIM) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* special case if the block contains an radio button or checkbox */
	ಯದಿ (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_size = INTTOFIX(1);
		ಯದಿ (block->height == AUTO)
			block->height = FIXTOINT(css_unit_len2device_px(
					block->style,
					&content->unit_len_ctx,
					gadget_size, gadget_unit));
	}

	box = block->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = block->padding[ತುದಿ_2];
	ಯದಿ (box)
		box->y = block->padding[ತುದಿ_2];

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
	ಯಾವತ್ (box) {
		ಗಣನಾ css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
		ಗಣನಾ css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

		ಪ್ರತಿಜ್ಞಾ(box->type == BOX_BLOCK ||
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

		ಯದಿ (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->parent->padding[ಎಡ_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			ಗಚ್ಛ advance_to_next_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		ಯದಿ (margin_collapse == ರಿಕ್ತ) {
			margin_collapse = layout_next_margin_block(
					&content->unit_len_ctx, box, block,
					viewport_height,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			in_margin = true;
		}

		/* Clearance. */
		y = 0;
		ಯದಿ (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(block->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		ಯದಿ (box->style) {
			overflow_x = css_computed_overflow_x(box->style);
			overflow_y = css_computed_overflow_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		ಯದಿ (box->type == BOX_FLEX ||
		    box->type == BOX_BLOCK ||
		    box->flags & IFRAME) {
			ಯದಿ (lh__box_is_object(box) == false &&
					box->style &&
					(overflow_x != CSS_OVERFLOW_VISIBLE ||
					 overflow_y != CSS_OVERFLOW_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				ಪೂರ್ಣಾಙ್ಕ x0, x1, ತುದಿ;
				ಸಂರಚನಾ box *ಎಡ, *ಬಲ;
				ತುದಿ = cy + max_pos_margin - max_neg_margin;
				ತುದಿ = (ತುದಿ > y) ? ತುದಿ : y;
				x0 = cx;
				x1 = cx + box->parent->width -
						box->parent->padding[ಎಡ_2] -
						box->parent->padding[ಬಲ_2];
				find_sides(block->float_children, ತುದಿ, ತುದಿ,
						&x0, &x1, &ಎಡ, &ಬಲ);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->parent->width -
						box->parent->padding[ಎಡ_2] -
						box->parent->padding[ಬಲ_2] -
						x1;
			}
			layout_block_find_dimensions(&content->unit_len_ctx,
					box->parent->width,
					viewport_height, lm, rm, box);
			ಯದಿ (box->type == BOX_BLOCK && !(box->flags & IFRAME)) {
				layout_block_add_scrollbar(box, ಬಲ_2);
				layout_block_add_scrollbar(box, BOTTOM);
			}
		} ಅನ್ಯಥಾ ಯದಿ (box->type == BOX_TABLE) {
			ಯದಿ (box->style != ರಿಕ್ತ) {
				ಗಣನಾ css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style, &width,
						&unit);

				ಯದಿ (wtype == CSS_WIDTH_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					ಪೂರ್ಣಾಙ್ಕ x0, x1, ತುದಿ;
					ಸಂರಚನಾ box *ಎಡ, *ಬಲ;
					ತುದಿ = cy + max_pos_margin -
							max_neg_margin;
					ತುದಿ = (ತುದಿ > y) ? ತುದಿ : y;
					x0 = cx;
					x1 = cx + box->parent->width -
						box->parent->padding[ಎಡ_2] -
						box->parent->padding[ಬಲ_2];
					find_sides(block->float_children,
						ತುದಿ, ತುದಿ, &x0, &x1,
						&ಎಡ, &ಬಲ);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->parent->width -
						box->parent->padding[ಎಡ_2] -
						box->parent->padding[ಬಲ_2] -
						x1;
				}
			}
			ಯದಿ (!layout_table(box, box->parent->width - lm - rm,
					content))
				ಪ್ರತ್ಯಾಗಮ false;
			layout_solve_width(box, box->parent->width, box->width,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->parent->padding[ಎಡ_2] + box->margin[ಎಡ_2] +
				box->border[ಎಡ_2].width;
		cx += box->x;

		/* Position box: vertical. */
		ಯದಿ (box->border[ತುದಿ_2].width) {
			box->y += box->border[ತುದಿ_2].width;
			cy += box->border[ತುದಿ_2].width;
		}

		/* Vertical margin */
		ಯದಿ (((box->type == BOX_BLOCK && (box->flags & HAS_HEIGHT)) ||
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
		ಯದಿ (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		ಯದಿ (box->type == BOX_FLEX ||
				(box->type == BOX_BLOCK && box->style &&
				 (overflow_x != CSS_OVERFLOW_VISIBLE ||
				  overflow_y != CSS_OVERFLOW_VISIBLE))) {

			ಯದಿ (box->type == BOX_FLEX) {
				ಯದಿ (!layout_flex(box, box->width, content)) {
					ಪ್ರತ್ಯಾಗಮ false;
				}
			} ಅನ್ಯಥಾ {
				layout_block_context(box,
						viewport_height, content);
			}

			cy += box->padding[ತುದಿ_2];

			ಯದಿ (box->height == AUTO) {
				box->height = 0;
				layout_block_add_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->height + box->padding[BOTTOM] +
					box->border[BOTTOM].width;
			y = box->y + box->padding[ತುದಿ_2] + box->height +
					box->padding[BOTTOM] +
					box->border[BOTTOM].width;

			/* Skip children, because they are done in the new
			 * block context */
			ಗಚ್ಛ advance_to_next_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->width);

		/* Layout (except tables). */
		ಯದಿ (box->object) {
			ಯದಿ (!layout_block_object(box))
				ಪ್ರತ್ಯಾಗಮ false;

		} ಅನ್ಯಥಾ ಯದಿ (box->type == BOX_INLINE_CONTAINER) {
			box->width = box->parent->width;
			ಯದಿ (!layout_inline_container(box, box->width, block,
					cx, cy, content))
				ಪ್ರತ್ಯಾಗಮ false;

		} ಅನ್ಯಥಾ ಯದಿ (box->type == BOX_TABLE) {
			/* Move down to avoid floats if necessary. */
			ಪೂರ್ಣಾಙ್ಕ x0, x1;
			ಸಂರಚನಾ box *ಎಡ, *ಬಲ;
			y = cy;
			ಯಾವತ್ (1) {
				ಗಣನಾ css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style,
						&width, &unit);

				x0 = cx;
				x1 = cx + box->parent->width;
				find_sides(block->float_children, y,
						y + box->height,
						&x0, &x1, &ಎಡ, &ಬಲ);
				ಯದಿ (wtype == CSS_WIDTH_AUTO)
					ಭಙ್ಗ;
				ಯದಿ (box->width <= x1 - x0)
					ಭಙ್ಗ;
				ಯದಿ (!ಎಡ && !ಬಲ)
					ಭಙ್ಗ;
				ಅನ್ಯಥಾ ಯದಿ (!ಎಡ)
					y = ಬಲ->y + ಬಲ->height + 1;
				ಅನ್ಯಥಾ ಯದಿ (!ಬಲ)
					y = ಎಡ->y + ಎಡ->height + 1;
				ಅನ್ಯಥಾ ಯದಿ (ಎಡ->y + ಎಡ->height <
						ಬಲ->y + ಬಲ->height)
					y = ಎಡ->y + ಎಡ->height + 1;
				ಅನ್ಯಥಾ
					y = ಬಲ->y + ಬಲ->height + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		ಯದಿ (box->type == BOX_BLOCK && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			ಯದಿ (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = ರಿಕ್ತ;
			}

			y = box->padding[ತುದಿ_2];
			box = box->children;
			box->y = y;
			cy += y;
			ಅನುವರ್ತ;
		} ಅನ್ಯಥಾ ಯದಿ (box->type == BOX_BLOCK || box->object ||
				box->flags & IFRAME)
			cy += box->padding[ತುದಿ_2];

		ಯದಿ (box->type == BOX_BLOCK && box->height == AUTO) {
			box->height = 0;
			layout_block_add_scrollbar(box, BOTTOM);
		}

		cy += box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width;
		cx -= box->x;
		y = box->y + box->padding[ತುದಿ_2] + box->height +
				box->padding[BOTTOM] +
				box->border[BOTTOM].width;

	advance_to_next_box:
		ಯದಿ (!box->next) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			ಕುರು {
				ಯದಿ (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = ರಿಕ್ತ;
				}

				/* Apply bottom margin */
				ಯದಿ (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				ಅನ್ಯಥಾ ಯದಿ (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->parent;
				ಯದಿ (box == block)
					ಭಙ್ಗ;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				ಯದಿ (box->type == BOX_BLOCK &&
						box->flags & MAKE_HEIGHT) {
					margin_collapse = ರಿಕ್ತ;
					in_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				ಯದಿ (box->height == AUTO) {
					box->height = y - box->padding[ತುದಿ_2];

					ಯದಿ (box->type == BOX_BLOCK)
						layout_block_add_scrollbar(box,
								BOTTOM);
				} ಅನ್ಯಥಾ
					cy += box->height -
							(y - box->padding[ತುದಿ_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				ಯದಿ (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_height(
							&content->unit_len_ctx,
							box, ರಿಕ್ತ)) {
					/* Height altered */
					/* Set current cy */
					cy += box->height -
							(y - box->padding[ತುದಿ_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].width;
				cx -= box->x;
				y = box->y + box->padding[ತುದಿ_2] + box->height +
						box->padding[BOTTOM] +
						box->border[BOTTOM].width;

			} ಯಾವತ್ (box->next == ರಿಕ್ತ);
			ಯದಿ (box == block)
				ಭಙ್ಗ;
		}

		/* To next sibling. */

		ಯದಿ (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = ರಿಕ್ತ;
		}

		ಯದಿ (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		ಅನ್ಯಥಾ ಯದಿ (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->next;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	ಪ್ರತಿ (box = block->float_children; box; box = box->next_float) {
		y = box->y + box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width + box->margin[BOTTOM];
		ಯದಿ (cy < y)
			cy = y;
	}

	ಯದಿ (block->height == AUTO) {
		block->height = cy - block->padding[ತುದಿ_2];
		ಯದಿ (block->type == BOX_BLOCK)
			layout_block_add_scrollbar(block, BOTTOM);
	}

	ಯದಿ (block->style && css_computed_position(block->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_height(&content->unit_len_ctx, block, ರಿಕ್ತ);
	}

	ಯದಿ (block->gadget &&
			(block->gadget->type == GADGET_TEXTAREA ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		ಪೂರ್ಣಾಙ್ಕ ta_width = block->padding[ಎಡ_2] + block->width +
				block->padding[ಬಲ_2];
		ಪೂರ್ಣಾಙ್ಕ ta_height = block->padding[ತುದಿ_2] + block->height +
				block->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				block->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_set_layout(block->gadget->data.text.ta,
				&fstyle, ta_width, ta_height,
				block->padding[ತುದಿ_2], block->padding[ಬಲ_2],
				block->padding[BOTTOM], block->padding[ಎಡ_2]);
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
ಸ್ಥಿರ bool
layout__get_element_tag(
		ಧ್ರುವ dom_ಗಂಟು *ಗಂಟು,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_ಗಂಟು_type ಗಂಟು_type;
	dom_exception exc;

	exc = dom_ಗಂಟು_get_ಗಂಟು_type(ಗಂಟು, &ಗಂಟು_type);
	ಯದಿ (exc != DOM_NO_ERR ||
	    ಗಂಟು_type != DOM_ELEMENT_ಗಂಟು) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	exc = dom_html_element_get_tag_type(ಗಂಟು, &element_type);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	*type = element_type;
	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool
layout__check_element_type(
		ಧ್ರುವ dom_ಗಂಟು *ಗಂಟು,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	ಯದಿ (!layout__get_element_tag(ಗಂಟು, &element_type)) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ಸ್ಥಿರ bool
layout_get_li_ಮೌಲ್ಯ(dom_ಗಂಟು *li_ಗಂಟು, dom_long *ಮೌಲ್ಯ_out)
{
	dom_exception exc;
	dom_long ಮೌಲ್ಯ;
	bool has_ಮೌಲ್ಯ;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_ಗಂಟು,
			corestring_dom_ಮೌಲ್ಯ,
			&has_ಮೌಲ್ಯ);
	ಯದಿ (exc != DOM_NO_ERR || has_ಮೌಲ್ಯ == false) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	exc = dom_html_li_element_get_ಮೌಲ್ಯ(
			(dom_html_li_element *)li_ಗಂಟು,
			&ಮೌಲ್ಯ);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	*ಮೌಲ್ಯ_out = ಮೌಲ್ಯ;
	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ಸ್ಥಿರ bool
layout__get_ol_start(dom_ಗಂಟು *ol_ಗಂಟು, dom_long *start_out)
{
	dom_exception exc;
	dom_long start;
	bool has_start;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_ಗಂಟು,
			corestring_dom_start,
			&has_start);
	ಯದಿ (exc != DOM_NO_ERR || has_start == false) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	exc = dom_html_olist_element_get_start(
			(dom_html_olist_element *)ol_ಗಂಟು,
			&start);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	*start_out = start;
	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
ಸ್ಥಿರ bool
layout__get_ol_reversed(dom_ಗಂಟು *ol_ಗಂಟು)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_ಗಂಟು,
			corestring_dom_reversed,
			&has_reversed);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
ಸ್ಥಿರ bool
layout_get_list_item_ಎಣಿಕೆ(
		dom_ಗಂಟು *list_owner, dom_long *ಎಣಿಕೆ_out)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_ಗಂಟು *child;
	ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ;

	ಯದಿ (list_owner == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (!layout__get_element_tag(list_owner, &tag_type)) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	exc = dom_ಗಂಟು_get_first_child(list_owner, &child);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಎಣಿಕೆ = 0;
	ಯಾವತ್ (child != ರಿಕ್ತ) {
		dom_ಗಂಟು *temp_ಗಂಟು;

		ಯದಿ (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ಸಂರಚನಾ box *child_box;
			ಯದಿ (dom_ಗಂಟು_get_user_data(child,
					corestring_dom_ns_ಕೀಲಿ_box_ಗಂಟು_data,
					&child_box) != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(child);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			ಯದಿ (child_box != ರಿಕ್ತ &&
			    child_box->list_marker != ರಿಕ್ತ) {
				ಎಣಿಕೆ++;
			}
		}

		exc = dom_ಗಂಟು_get_next_sibling(child, &temp_ಗಂಟು);
		dom_ಗಂಟು_unref(child);
		ಯದಿ (exc != DOM_NO_ERR) {
			ಪ್ರತ್ಯಾಗಮ false;
		}

		child = temp_ಗಂಟು;
	}

	*ಎಣಿಕೆ_out = ಎಣಿಕೆ;
	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
layout_ordered_list_ಎಣಿಕೆ(
		ಸಂರಚನಾ box *box)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_ಗಂಟು *child;
	ಪೂರ್ಣಾಙ್ಕ step = 1;
	dom_long next;

	ಯದಿ (box->ಗಂಟು == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (!layout__get_element_tag(box->ಗಂಟು, &tag_type)) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	next = 1;
	ಯದಿ (tag_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_start = layout__get_ol_start(box->ಗಂಟು, &next);
		bool have_reversed = layout__get_ol_reversed(box->ಗಂಟು);

		ಯದಿ (have_reversed) {
			step = -1;
		}

		ಯದಿ (!have_start && have_reversed) {
			layout_get_list_item_ಎಣಿಕೆ(box->ಗಂಟು, &next);
		}
	}

	exc = dom_ಗಂಟು_get_first_child(box->ಗಂಟು, &child);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯಾವತ್ (child != ರಿಕ್ತ) {
		dom_ಗಂಟು *temp_ಗಂಟು;

		ಯದಿ (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ಸಂರಚನಾ box *child_box;

			ಯದಿ (dom_ಗಂಟು_get_user_data(child,
					corestring_dom_ns_ಕೀಲಿ_box_ಗಂಟು_data,
					&child_box) != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(child);
				ಪ್ರತ್ಯಾಗಮ;
			}

			ಯದಿ (child_box != ರಿಕ್ತ &&
			    child_box->list_marker != ರಿಕ್ತ) {
				dom_long ಮೌಲ್ಯ;
				ಸಂರಚನಾ box *marker = child_box->list_marker;
				ಯದಿ (layout_get_li_ಮೌಲ್ಯ(child, &ಮೌಲ್ಯ)) {
					marker->list_ಮೌಲ್ಯ = ಮೌಲ್ಯ;
					next = marker->list_ಮೌಲ್ಯ;
				} ಅನ್ಯಥಾ {
					marker->list_ಮೌಲ್ಯ = next;
				}
				next += step;
			}
		}

		exc = dom_ಗಂಟು_get_next_sibling(child, &temp_ಗಂಟು);
		dom_ಗಂಟು_unref(child);
		ಯದಿ (exc != DOM_NO_ERR) {
			ಪ್ರತ್ಯಾಗಮ;
		}

		child = temp_ಗಂಟು;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
layout__set_numerical_marker_text(
		ಧ್ರುವ html_content *content,
		ಸಂರಚನಾ box *box)
{
	ಸಂರಚನಾ box *marker = box->list_marker;
	size_t counter_len;
	css_error css_res;
	ಗಣನಾ {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_SIZE = 20,
	};

	marker->text = talloc_array(content->bctx, ವರ್ಣ, LIST_MARKER_SIZE);
	ಯದಿ (marker->text == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_ಮೌಲ್ಯ,
			marker->text, LIST_MARKER_SIZE, &counter_len);
	ಯದಿ (css_res == CSS_OK) {
		ಯದಿ (counter_len > LIST_MARKER_SIZE) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->text = talloc_realloc(content->bctx,
					marker->text,
					ವರ್ಣ,
					counter_len);
			ಯದಿ (marker->text == ರಿಕ್ತ) {
				ಪ್ರತ್ಯಾಗಮ;
			}
			css_computed_format_list_style(box->style,
					marker->list_ಮೌಲ್ಯ, marker->text,
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
ಸ್ಥಿರ bool
layout__list_item_is_numerical(
		ಧ್ರುವ ಸಂರಚನಾ box *b)
{
	ಗಣನಾ css_list_style_type_e t = css_computed_list_style_type(b->style);

	ವಿಕಲ್ಪ (t) {
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_SQUARE: /* Fall through. */
	ಪಕ್ಷ CSS_LIST_STYLE_TYPE_NONE:
		ಪ್ರತ್ಯಾಗಮ false;

	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ true;
	}
}

/**
 * Layout list markers.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
layout_lists(ಧ್ರುವ html_content *content, ಸಂರಚನಾ box *box)
{
	ಸಂರಚನಾ box *child;

	layout_ordered_list_ಎಣಿಕೆ(box);

	ಪ್ರತಿ (child = box->children; child; child = child->next) {
		ಯದಿ (child->list_marker) {
			ಸಂರಚನಾ box *marker = child->list_marker;

			ಯದಿ (layout__list_item_is_numerical(child)) {
				ಯದಿ (marker->text == ರಿಕ್ತ) {
					layout__set_numerical_marker_text(
							content, child);
				}
			}
			ಯದಿ (marker->object) {
				marker->width =
					content_get_width(marker->object);
				marker->x = -marker->width;
				marker->height =
					content_get_height(marker->object);
				marker->y = (ಸಾಲು_height(
						&content->unit_len_ctx,
						marker->style) -
						marker->height) / 2;
			} ಅನ್ಯಥಾ ಯದಿ (marker->text) {
				ಯದಿ (marker->width == UNKNOWN_WIDTH) {
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
				marker->height = ಸಾಲು_height(
						&content->unit_len_ctx,
						marker->style);
			} ಅನ್ಯಥಾ {
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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_compute_offsets(ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		       ಸಂರಚನಾ box *box,
		       ಸಂರಚನಾ box *containing_block,
		       ಪೂರ್ಣಾಙ್ಕ *ತುದಿ,
		       ಪೂರ್ಣಾಙ್ಕ *ಬಲ,
		       ಪೂರ್ಣಾಙ್ಕ *bottom,
		       ಪೂರ್ಣಾಙ್ಕ *ಎಡ)
{
	uint32_t type;
	css_fixed ಮೌಲ್ಯ = 0;
	css_unit unit = CSS_UNIT_PX;

	ಪ್ರತಿಜ್ಞಾ(containing_block->width != UNKNOWN_WIDTH);
	ಪ್ರತಿಜ್ಞಾ(containing_block->width != AUTO);
	ಪ್ರತಿಜ್ಞಾ(containing_block->height != AUTO);

	/*ಎಡ*/
	type = css_computed_ಎಡ(box->style, &ಮೌಲ್ಯ, &unit);
	ಯದಿ (type == CSS_ಎಡ_SET) {
		ಯದಿ (unit == CSS_UNIT_PCT) {
			*ಎಡ = FPCT_OF_INT_TOINT(ಮೌಲ್ಯ,
					containing_block->width);
		} ಅನ್ಯಥಾ {
			*ಎಡ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		}
	} ಅನ್ಯಥಾ {
		*ಎಡ = AUTO;
	}

	/*ಬಲ*/
	type = css_computed_ಬಲ(box->style, &ಮೌಲ್ಯ, &unit);
	ಯದಿ (type == CSS_ಬಲ_SET) {
		ಯದಿ (unit == CSS_UNIT_PCT) {
			*ಬಲ = FPCT_OF_INT_TOINT(ಮೌಲ್ಯ,
					containing_block->width);
		} ಅನ್ಯಥಾ {
			*ಬಲ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		}
	} ಅನ್ಯಥಾ {
		*ಬಲ = AUTO;
	}

	/*ತುದಿ*/
	type = css_computed_ತುದಿ(box->style, &ಮೌಲ್ಯ, &unit);
	ಯದಿ (type == CSS_ತುದಿ_SET) {
		ಯದಿ (unit == CSS_UNIT_PCT) {
			*ತುದಿ = FPCT_OF_INT_TOINT(ಮೌಲ್ಯ,
					containing_block->height);
		} ಅನ್ಯಥಾ {
			*ತುದಿ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		}
	} ಅನ್ಯಥಾ {
		*ತುದಿ = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &ಮೌಲ್ಯ, &unit);
	ಯದಿ (type == CSS_BOTTOM_SET) {
		ಯದಿ (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_OF_INT_TOINT(ಮೌಲ್ಯ,
					containing_block->height);
		} ಅನ್ಯಥಾ {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		}
	} ಅನ್ಯಥಾ {
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
ಸ್ಥಿರ bool
layout_absolute(ಸಂರಚನಾ box *box,
		ಸಂರಚನಾ box *containing_block,
		ಪೂರ್ಣಾಙ್ಕ cx, ಪೂರ್ಣಾಙ್ಕ cy,
		html_content *content)
{
	ಪೂರ್ಣಾಙ್ಕ static_ಎಡ, static_ತುದಿ;  /* static position */
	ಪೂರ್ಣಾಙ್ಕ ತುದಿ, ಬಲ, bottom, ಎಡ;
	ಪೂರ್ಣಾಙ್ಕ width, height, max_width, min_width;
	ಪೂರ್ಣಾಙ್ಕ *margin = box->margin;
	ಪೂರ್ಣಾಙ್ಕ *padding = box->padding;
	ಸಂರಚನಾ box_border *border = box->border;
	ಪೂರ್ಣಾಙ್ಕ available_width = containing_block->width;
	ಪೂರ್ಣಾಙ್ಕ space;

	ಪ್ರತಿಜ್ಞಾ(box->type == BOX_BLOCK || box->type == BOX_TABLE ||
			box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_ಎಡ = cx + box->x;
	static_ತುದಿ = cy + box->y;

	ಯದಿ (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_block->width += containing_block->padding[ಎಡ_2] +
				containing_block->padding[ಬಲ_2];
		containing_block->height += containing_block->padding[ತುದಿ_2] +
				containing_block->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_block,
			&ತುದಿ, &ಬಲ, &bottom, &ಎಡ);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_block;
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1,
			box, box->style, &width, &height,
			&max_width, &min_width, 0, 0,
			margin, padding, border);
	box->float_container = ರಿಕ್ತ;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ಎಡ, margin[ಎಡ_2], border[ಎಡ_2].width, padding[ಎಡ_2], width,
	      padding[ಬಲ_2], border[ಬಲ_2].width, margin[ಬಲ_2], ಬಲ,
	      containing_block->width);


	ಯದಿ (ಎಡ == AUTO && width == AUTO && ಬಲ == AUTO) {
		ಯದಿ (margin[ಎಡ_2] == AUTO)
			margin[ಎಡ_2] = 0;
		ಯದಿ (margin[ಬಲ_2] == AUTO)
			margin[ಬಲ_2] = 0;
		ಎಡ = static_ಎಡ;

		width = min(max(box->min_width, available_width),
			box->max_width);
		width -= box->margin[ಎಡ_2] + box->border[ಎಡ_2].width +
			box->padding[ಎಡ_2] + box->padding[ಬಲ_2] +
			box->border[ಬಲ_2].width + box->margin[ಬಲ_2];

		/* Adjust for {min|max}-width */
		ಯದಿ (max_width >= 0 && width > max_width) width = max_width;
		ಯದಿ (width < min_width) width = min_width;

		ಬಲ = containing_block->width -
			ಎಡ -
			margin[ಎಡ_2] - border[ಎಡ_2].width - padding[ಎಡ_2] -
			width -
			padding[ಬಲ_2] - border[ಬಲ_2].width - margin[ಬಲ_2];
	} ಅನ್ಯಥಾ ಯದಿ (ಎಡ != AUTO && width != AUTO && ಬಲ != AUTO) {

		/* Adjust for {min|max}-width */
		ಯದಿ (max_width >= 0 && width > max_width) width = max_width;
		ಯದಿ (min_width >  0 && width < min_width) width = min_width;

		ಯದಿ (margin[ಎಡ_2] == AUTO && margin[ಬಲ_2] == AUTO) {
			space = containing_block->width -
					ಎಡ - border[ಎಡ_2].width -
					padding[ಎಡ_2] - width - padding[ಬಲ_2] -
					border[ಬಲ_2].width - ಬಲ;
			ಯದಿ (space < 0) {
				margin[ಎಡ_2] = 0;
				margin[ಬಲ_2] = space;
			} ಅನ್ಯಥಾ {
				margin[ಎಡ_2] = margin[ಬಲ_2] = space / 2;
			}
		} ಅನ್ಯಥಾ ಯದಿ (margin[ಎಡ_2] == AUTO) {
			margin[ಎಡ_2] = containing_block->width -
					ಎಡ - border[ಎಡ_2].width -
					padding[ಎಡ_2] - width - padding[ಬಲ_2] -
					border[ಬಲ_2].width - margin[ಬಲ_2] -
					ಬಲ;
		} ಅನ್ಯಥಾ ಯದಿ (margin[ಬಲ_2] == AUTO) {
			margin[ಬಲ_2] = containing_block->width -
					ಎಡ - margin[ಎಡ_2] -
					border[ಎಡ_2].width -
					padding[ಎಡ_2] - width - padding[ಬಲ_2] -
					border[ಬಲ_2].width - ಬಲ;
		} ಅನ್ಯಥಾ {
			ಬಲ = containing_block->width -
					ಎಡ - margin[ಎಡ_2] -
					border[ಎಡ_2].width -
					padding[ಎಡ_2] - width - padding[ಬಲ_2] -
					border[ಬಲ_2].width - margin[ಬಲ_2];
		}
	} ಅನ್ಯಥಾ {
		ಯದಿ (margin[ಎಡ_2] == AUTO)
			margin[ಎಡ_2] = 0;
		ಯದಿ (margin[ಬಲ_2] == AUTO)
			margin[ಬಲ_2] = 0;

		ಯದಿ (ಎಡ == AUTO && width == AUTO && ಬಲ != AUTO) {
			available_width -= ಬಲ;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[ಎಡ_2] + box->border[ಎಡ_2].width +
				box->padding[ಎಡ_2] + box->padding[ಬಲ_2] +
				box->border[ಬಲ_2].width + box->margin[ಬಲ_2];

			/* Adjust for {min|max}-width */
			ಯದಿ (max_width >= 0 && width > max_width)
				width = max_width;
			ಯದಿ (width < min_width)
				width = min_width;

			ಎಡ = containing_block->width -
					margin[ಎಡ_2] - border[ಎಡ_2].width -
					padding[ಎಡ_2] - width - padding[ಬಲ_2] -
					border[ಬಲ_2].width - margin[ಬಲ_2] -
					ಬಲ;
		} ಅನ್ಯಥಾ ಯದಿ (ಎಡ == AUTO && width != AUTO && ಬಲ == AUTO) {

			/* Adjust for {min|max}-width */
			ಯದಿ (max_width >= 0 && width > max_width)
				width = max_width;
			ಯದಿ (min_width >  0 && width < min_width)
				width = min_width;

			ಎಡ = static_ಎಡ;
			ಬಲ = containing_block->width -
					ಎಡ - margin[ಎಡ_2] -
					border[ಎಡ_2].width -
					padding[ಎಡ_2] - width - padding[ಬಲ_2] -
					border[ಬಲ_2].width - margin[ಬಲ_2];
		} ಅನ್ಯಥಾ ಯದಿ (ಎಡ != AUTO && width == AUTO && ಬಲ == AUTO) {
			available_width -= ಎಡ;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[ಎಡ_2] + box->border[ಎಡ_2].width +
				box->padding[ಎಡ_2] + box->padding[ಬಲ_2] +
				box->border[ಬಲ_2].width + box->margin[ಬಲ_2];

			/* Adjust for {min|max}-width */
			ಯದಿ (max_width >= 0 && width > max_width)
				width = max_width;
			ಯದಿ (width < min_width)
				width = min_width;

			ಬಲ = containing_block->width -
					ಎಡ - margin[ಎಡ_2] -
					border[ಎಡ_2].width -
					padding[ಎಡ_2] - width - padding[ಬಲ_2] -
					border[ಬಲ_2].width - margin[ಬಲ_2];
		} ಅನ್ಯಥಾ ಯದಿ (ಎಡ == AUTO && width != AUTO && ಬಲ != AUTO) {

			/* Adjust for {min|max}-width */
			ಯದಿ (max_width >= 0 && width > max_width)
				width = max_width;
			ಯದಿ (width < min_width)
				width = min_width;

			ಎಡ = containing_block->width -
					margin[ಎಡ_2] - border[ಎಡ_2].width -
					padding[ಎಡ_2] - width - padding[ಬಲ_2] -
					border[ಬಲ_2].width - margin[ಬಲ_2] -
					ಬಲ;
		} ಅನ್ಯಥಾ ಯದಿ (ಎಡ != AUTO && width == AUTO && ಬಲ != AUTO) {
			width = containing_block->width -
					ಎಡ - margin[ಎಡ_2] -
					border[ಎಡ_2].width -
					padding[ಎಡ_2] - padding[ಬಲ_2] -
					border[ಬಲ_2].width - margin[ಬಲ_2] -
					ಬಲ;

			/* Adjust for {min|max}-width */
			ಯದಿ (max_width >= 0 && width > max_width)
				width = max_width;
			ಯದಿ (width < min_width)
				width = min_width;

		} ಅನ್ಯಥಾ ಯದಿ (ಎಡ != AUTO && width != AUTO && ಬಲ == AUTO) {

			/* Adjust for {min|max}-width */
			ಯದಿ (max_width >= 0 && width > max_width)
				width = max_width;
			ಯದಿ (width < min_width)
				width = min_width;

			ಬಲ = containing_block->width -
					ಎಡ - margin[ಎಡ_2] -
					border[ಎಡ_2].width -
					padding[ಎಡ_2] - width - padding[ಬಲ_2] -
					border[ಬಲ_2].width - margin[ಬಲ_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ಎಡ, margin[ಎಡ_2], border[ಎಡ_2].width, padding[ಎಡ_2], width,
	      padding[ಬಲ_2], border[ಬಲ_2].width, margin[ಬಲ_2], ಬಲ,
	      containing_block->width);

	box->x = ಎಡ + margin[ಎಡ_2] + border[ಎಡ_2].width - cx;
	ಯದಿ (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_block->width -= containing_block->padding[ಎಡ_2] +
				containing_block->padding[ಬಲ_2];
	} ಅನ್ಯಥಾ {
		/** \todo inline ancestors */
	}
	box->width = width;
	box->height = height;

	ಯದಿ (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->object || box->flags & IFRAME) {
		ಯದಿ (!layout_block_context(box, -1, content))
			ಪ್ರತ್ಯಾಗಮ false;
	} ಅನ್ಯಥಾ ಯದಿ (box->type == BOX_TABLE) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		/* \todo  layout_table considers margins etc. again */
		ಯದಿ (!layout_table(box, width, content))
			ಪ್ರತ್ಯಾಗಮ false;
		box->float_container = ರಿಕ್ತ;
		layout_solve_width(box, box->parent->width, box->width, 0, 0,
				-1, -1);
	} ಅನ್ಯಥಾ ಯದಿ (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		ಯದಿ (!layout_flex(box, width, content))
			ಪ್ರತ್ಯಾಗಮ false;
		box->float_container = ರಿಕ್ತ;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ತುದಿ, margin[ತುದಿ_2], border[ತುದಿ_2].width, padding[ತುದಿ_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	ಯದಿ (ತುದಿ == AUTO && height == AUTO && bottom == AUTO) {
		ತುದಿ = static_ತುದಿ;
		height = box->height;
		ಯದಿ (margin[ತುದಿ_2] == AUTO)
			margin[ತುದಿ_2] = 0;
		ಯದಿ (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_block->height -
				ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].width -
				padding[ತುದಿ_2] - height - padding[BOTTOM] -
				border[BOTTOM].width - margin[BOTTOM];
	} ಅನ್ಯಥಾ ಯದಿ (ತುದಿ != AUTO && height != AUTO && bottom != AUTO) {
		ಯದಿ (margin[ತುದಿ_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_block->height -
					ತುದಿ - border[ತುದಿ_2].width - padding[ತುದಿ_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - bottom;
			margin[ತುದಿ_2] = margin[BOTTOM] = space / 2;
		} ಅನ್ಯಥಾ ಯದಿ (margin[ತುದಿ_2] == AUTO) {
			margin[ತುದಿ_2] = containing_block->height -
					ತುದಿ - border[ತುದಿ_2].width - padding[ತುದಿ_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} ಅನ್ಯಥಾ ಯದಿ (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_block->height -
					ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].width -
					padding[ತುದಿ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					bottom;
		} ಅನ್ಯಥಾ {
			bottom = containing_block->height -
					ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].width -
					padding[ತುದಿ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	} ಅನ್ಯಥಾ {
		ಯದಿ (margin[ತುದಿ_2] == AUTO)
			margin[ತುದಿ_2] = 0;
		ಯದಿ (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		ಯದಿ (ತುದಿ == AUTO && height == AUTO && bottom != AUTO) {
			height = box->height;
			ತುದಿ = containing_block->height -
					margin[ತುದಿ_2] - border[ತುದಿ_2].width -
					padding[ತುದಿ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} ಅನ್ಯಥಾ ಯದಿ (ತುದಿ == AUTO && height != AUTO && bottom == AUTO) {
			ತುದಿ = static_ತುದಿ;
			bottom = containing_block->height -
					ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].width -
					padding[ತುದಿ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} ಅನ್ಯಥಾ ಯದಿ (ತುದಿ != AUTO && height == AUTO && bottom == AUTO) {
			height = box->height;
			bottom = containing_block->height -
					ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].width -
					padding[ತುದಿ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} ಅನ್ಯಥಾ ಯದಿ (ತುದಿ == AUTO && height != AUTO && bottom != AUTO) {
			ತುದಿ = containing_block->height -
					margin[ತುದಿ_2] - border[ತುದಿ_2].width -
					padding[ತುದಿ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} ಅನ್ಯಥಾ ಯದಿ (ತುದಿ != AUTO && height == AUTO && bottom != AUTO) {
			height = containing_block->height -
					ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].width -
					padding[ತುದಿ_2] - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} ಅನ್ಯಥಾ ಯದಿ (ತುದಿ != AUTO && height != AUTO && bottom == AUTO) {
			bottom = containing_block->height -
					ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].width -
					padding[ತುದಿ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ತುದಿ, margin[ತುದಿ_2], border[ತುದಿ_2].width, padding[ತುದಿ_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	box->y = ತುದಿ + margin[ತುದಿ_2] + border[ತುದಿ_2].width - cy;
	ಯದಿ (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_block->height -= containing_block->padding[ತುದಿ_2] +
				containing_block->padding[BOTTOM];
	} ಅನ್ಯಥಾ {
		/** \todo Inline ancestors */
	}
	box->height = height;
	layout_apply_minmax_height(&content->unit_len_ctx, box, containing_block);

	ಪ್ರತ್ಯಾಗಮ true;
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
ಸ್ಥಿರ bool
layout_position_absolute(ಸಂರಚನಾ box *box,
			 ಸಂರಚನಾ box *containing_block,
			 ಪೂರ್ಣಾಙ್ಕ cx, ಪೂರ್ಣಾಙ್ಕ cy,
			 html_content *content)
{
	ಸಂರಚನಾ box *c;

	ಪ್ರತಿ (c = box->children; c; c = c->next) {
		ಯದಿ ((c->type == BOX_BLOCK || c->type == BOX_TABLE ||
				c->type == BOX_INLINE_BLOCK ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			ಯದಿ (!layout_absolute(c, containing_block,
					cx, cy, content))
				ಪ್ರತ್ಯಾಗಮ false;
			ಯದಿ (!layout_position_absolute(c, c, 0, 0, content))
				ಪ್ರತ್ಯಾಗಮ false;
		} ಅನ್ಯಥಾ ಯದಿ (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			ಯದಿ (!layout_position_absolute(c, c, 0, 0, content))
				ಪ್ರತ್ಯಾಗಮ false;
		} ಅನ್ಯಥಾ {
			ಪೂರ್ಣಾಙ್ಕ px, py;
			ಯದಿ (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_ಎಡ ||
					css_computed_float(c->style) ==
						CSS_FLOAT_ಬಲ)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				ಸಂರಚನಾ box *p;
				px = c->x;
				py = c->y;
				ಪ್ರತಿ (p = box->parent; p && !p->float_children;
						p = p->parent) {
					px -= p->x;
					py -= p->y;
				}
			} ಅನ್ಯಥಾ {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			ಯದಿ (!layout_position_absolute(c, containing_block,
					cx + px, cy + py, content))
				ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_compute_relative_offset(
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಸಂರಚನಾ box *box,
		ಪೂರ್ಣಾಙ್ಕ *x,
		ಪೂರ್ಣಾಙ್ಕ *y)
{
	ಪೂರ್ಣಾಙ್ಕ ಎಡ, ಬಲ, ತುದಿ, bottom;
	ಸಂರಚನಾ box *containing_block;

	ಪ್ರತಿಜ್ಞಾ(box && box->parent && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	ಯದಿ (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_ಎಡ ||
			css_computed_float(box->style) == CSS_FLOAT_ಬಲ)) {
		containing_block = box->float_container;
	} ಅನ್ಯಥಾ {
		containing_block = box->parent;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_block,
			&ತುದಿ, &ಬಲ, &bottom, &ಎಡ);

	ಯದಿ (ಎಡ == AUTO && ಬಲ == AUTO)
		ಎಡ = ಬಲ = 0;
	ಅನ್ಯಥಾ ಯದಿ (ಎಡ == AUTO)
		/* left is auto => computed = -right */
		ಎಡ = -ಬಲ;
	ಅನ್ಯಥಾ ಯದಿ (ಬಲ == AUTO)
		/* right is auto => computed = -left */
		ಬಲ = -ಎಡ;
	ಅನ್ಯಥಾ {
		/* over constrained => examine direction property
		 * of containing block */
		ಯದಿ (containing_block->style &&
				css_computed_direction(
				containing_block->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			ಎಡ = -ಬಲ;
		} ಅನ್ಯಥಾ {
			/* assume LTR in all other cases */
			ಬಲ = -ಎಡ;
		}
	}

	ಪ್ರತಿಜ್ಞಾ(ಎಡ == -ಬಲ);

	ಯದಿ (ತುದಿ == AUTO && bottom == AUTO) {
		ತುದಿ = bottom = 0;
	} ಅನ್ಯಥಾ ಯದಿ (ತುದಿ == AUTO) {
		ತುದಿ = -bottom;
	} ಅನ್ಯಥಾ {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -ತುದಿ;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", ಎಡ,
	      ಬಲ, ತುದಿ, bottom);

	*x = ಎಡ;
	*y = ತುದಿ;
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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_position_relative(
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಸಂರಚನಾ box *ಬೇರು,
		ಸಂರಚನಾ box *fp,
		ಪೂರ್ಣಾಙ್ಕ fx,
		ಪೂರ್ಣಾಙ್ಕ fy)
{
	ಸಂರಚನಾ box *box; /* for children of "root" */
	ಸಂರಚನಾ box *fn;  /* for block formatting context box for children of
			  * "box" */
	ಸಂರಚನಾ box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	ಪೂರ್ಣಾಙ್ಕ x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	ಪೂರ್ಣಾಙ್ಕ fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	ಪ್ರತಿಜ್ಞಾ(ಬೇರು);

	/* Normal children */
	ಪ್ರತಿ (box = ಬೇರು->children; box; box = box->next) {

		ಯದಿ (box->type == BOX_TEXT)
			ಅನುವರ್ತ;

		/* If relatively positioned, get offsets */
		ಯದಿ (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_offset(
					unit_len_ctx, box, &x, &y);
		ಅನ್ಯಥಾ
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		ಯದಿ (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_ಎಡ ||
				css_computed_float(box->style) ==
					CSS_FLOAT_ಬಲ) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			ಪ್ರತಿ (fc = fp->float_children; fc; fc = fc->next_float) {
				ಯದಿ (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		ಯದಿ (box->float_children) {
			fn = box;
			fnx = fny = 0;
		} ಅನ್ಯಥಾ {
			fn = fp;
			fnx = fx + x;
			fny = fy + y;
		}

		/* recurse first */
		layout_position_relative(unit_len_ctx, box, fn, fnx, fny);

		/* Ignore things we're not interested in. */
		ಯದಿ (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			ಅನುವರ್ತ;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		ಯದಿ (box->type == BOX_INLINE && box->inline_end) {
			ಸಂರಚನಾ box *b;
			ಪ್ರತಿ (b = box->next; b && b != box->inline_end;
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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_get_box_bbox(
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಸಂರಚನಾ box *box,
		ಪೂರ್ಣಾಙ್ಕ *desc_x0, ಪೂರ್ಣಾಙ್ಕ *desc_y0,
		ಪೂರ್ಣಾಙ್ಕ *desc_x1, ಪೂರ್ಣಾಙ್ಕ *desc_y1)
{
	*desc_x0 = -box->border[ಎಡ_2].width;
	*desc_y0 = -box->border[ತುದಿ_2].width;
	*desc_x1 = box->padding[ಎಡ_2] + box->width + box->padding[ಬಲ_2] +
			box->border[ಬಲ_2].width;
	*desc_y1 = box->padding[ತುದಿ_2] + box->height + box->padding[BOTTOM] +
			box->border[BOTTOM].width;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	ಯದಿ (box->type == BOX_BLOCK && box->style != ರಿಕ್ತ &&
			css_computed_overflow_y(box->style) ==
					CSS_OVERFLOW_VISIBLE &&
			box->object == ರಿಕ್ತ) {
		css_fixed font_size = 0;
		css_unit font_unit = CSS_UNIT_PT;
		ಪೂರ್ಣಾಙ್ಕ text_height;

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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_update_descendant_bbox(
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಸಂರಚನಾ box *box,
		ಸಂರಚನಾ box *child,
		ಪೂರ್ಣಾಙ್ಕ off_x,
		ಪೂರ್ಣಾಙ್ಕ off_y)
{
	ಪೂರ್ಣಾಙ್ಕ child_desc_x0, child_desc_y0, child_desc_x1, child_desc_y1;

	/* get coordinates of child relative to box */
	ಪೂರ್ಣಾಙ್ಕ child_x = child->x - off_x;
	ಪೂರ್ಣಾಙ್ಕ child_y = child->y - off_y;

	bool html_object = (child->object &&
			content_get_type(child->object) == CONTENT_HTML);

	ಗಣನಾ css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	ಗಣನಾ css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

	ಯದಿ (child->style != ರಿಕ್ತ) {
		overflow_x = css_computed_overflow_x(child->style);
		overflow_y = css_computed_overflow_y(child->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, child,
			&child_desc_x0, &child_desc_y0,
			&child_desc_x1, &child_desc_y1);

	ಯದಿ (overflow_x == CSS_OVERFLOW_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		child_desc_x0 = child->descendant_x0;
		child_desc_x1 = child->descendant_x1;
	}
	ಯದಿ (overflow_y == CSS_OVERFLOW_VISIBLE &&
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
	ಯದಿ (child_desc_x0 < box->descendant_x0)
		box->descendant_x0 = child_desc_x0;
	ಯದಿ (child_desc_y0 < box->descendant_y0)
		box->descendant_y0 = child_desc_y0;
	ಯದಿ (box->descendant_x1 < child_desc_x1)
		box->descendant_x1 = child_desc_x1;
	ಯದಿ (box->descendant_y1 < child_desc_y1)
		box->descendant_y1 = child_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_calculate_descendant_bboxes(
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಸಂರಚನಾ box *box)
{
	ಸಂರಚನಾ box *child;

	ಪ್ರತಿಜ್ಞಾ(box->width != UNKNOWN_WIDTH);
	ಪ್ರತಿಜ್ಞಾ(box->height != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	ಯದಿ (box->object && content_get_type(box->object) == CONTENT_HTML) {
		ಯದಿ (box->descendant_x1 < content_get_width(box->object))
			box->descendant_x1 = content_get_width(box->object);
		ಯದಿ (box->descendant_y1 < content_get_height(box->object))
			box->descendant_y1 = content_get_height(box->object);
	}

	ಯದಿ (box->iframe != ರಿಕ್ತ) {
		ಪೂರ್ಣಾಙ್ಕ x, y;
		box_coords(box, &x, &y);

		browser_window_set_position(box->iframe, x, y);
		browser_window_set_dimensions(box->iframe,
				box->width, box->height);
		browser_window_reformat(box->iframe, true,
				box->width, box->height);
	}

	ಯದಿ (box->type == BOX_INLINE || box->type == BOX_TEXT)
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (box->type == BOX_INLINE_END) {
		box = box->inline_end;
		ಪ್ರತಿ (child = box->next; child;
				child = child->next) {
			ಯದಿ (child->type == BOX_FLOAT_ಎಡ ||
					child->type == BOX_FLOAT_ಬಲ)
				ಅನುವರ್ತ;

			layout_update_descendant_bbox(unit_len_ctx, box, child,
					box->x, box->y);

			ಯದಿ (child == box->inline_end)
				ಭಙ್ಗ;
		}
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		ಪ್ರತ್ಯಾಗಮ;

	ಪ್ರತಿ (child = box->children; child; child = child->next) {
		ಯದಿ (child->type == BOX_FLOAT_ಎಡ ||
				child->type == BOX_FLOAT_ಬಲ)
			ಅನುವರ್ತ;

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		ಯದಿ (box->style && css_computed_overflow_x(box->style) ==
				CSS_OVERFLOW_HIDDEN &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_HIDDEN)
			ಅನುವರ್ತ;

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	ಪ್ರತಿ (child = box->float_children; child; child = child->next_float) {
		ಪ್ರತಿಜ್ಞಾ(child->type == BOX_FLOAT_ಎಡ ||
				child->type == BOX_FLOAT_ಬಲ);

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	ಯದಿ (box->list_marker) {
		child = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height)
{
	bool ret;
	ಸಂರಚನಾ box *doc = content->layout;
	ಧ್ರುವ ಸಂರಚನಾ gui_layout_table *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			width, height, nsurl_access(content_get_url(
					&content->base)));

	layout_minmax_block(doc, font_func, content);

	layout_block_find_dimensions(&content->unit_len_ctx,
			width, height, 0, 0, doc);
	doc->x = doc->margin[ಎಡ_2] + doc->border[ಎಡ_2].width;
	doc->y = doc->margin[ತುದಿ_2] + doc->border[ತುದಿ_2].width;
	width -= doc->margin[ಎಡ_2] + doc->border[ಎಡ_2].width +
			doc->padding[ಎಡ_2] + doc->padding[ಬಲ_2] +
			doc->border[ಬಲ_2].width + doc->margin[ಬಲ_2];
	ಯದಿ (width < 0) {
		width = 0;
	}
	doc->width = width;

	ret = layout_block_context(doc, height, content);

	/* make <html> and <body> fill available height */
	ಯದಿ (doc->y + doc->padding[ತುದಿ_2] + doc->height + doc->padding[BOTTOM] +
			doc->border[BOTTOM].width + doc->margin[BOTTOM] <
			height) {
		doc->height = height - (doc->y + doc->padding[ತುದಿ_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].width +
				doc->margin[BOTTOM]);
		ಯದಿ (doc->children)
			doc->children->height = doc->height -
					(doc->children->margin[ತುದಿ_2] +
					 doc->children->border[ತುದಿ_2].width +
					 doc->children->padding[ತುದಿ_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].width +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	ಪ್ರತ್ಯಾಗಮ ret;
}
