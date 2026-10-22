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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <math.h>
#അന്തര്ഭാവ <dom/dom.h>

#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/talloc.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "netsurf/inttypes.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "netsurf/browser_window.h"
#അന്തര്ഭാവ "netsurf/layout.h"
#അന്തര്ഭാവ "content/content.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "css/utils.h"
#അന്തര്ഭാവ "desktop/scrollbar.h"
#അന്തര്ഭാവ "desktop/textarea.h"

#അന്തര്ഭാവ "html/html.h"
#അന്തര്ഭാവ "html/html_save.h"
#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_inspect.h"
#അന്തര്ഭാവ "html/font.h"
#അന്തര്ഭാവ "html/form_internal.h"
#അന്തര്ഭാവ "html/layout.h"
#അന്തര്ഭാവ "html/layout_internal.h"
#അന്തര്ഭാവ "html/table.h"

/** Array of per-side access functions for computed style margins. */
സ്ഥിരം css_len_func margin_funcs[4] = {
	[മുകൾ_2]    = css_computed_margin_മുകൾ,
	[വലത്_2]  = css_computed_margin_വലത്,
	[BOTTOM] = css_computed_margin_bottom,
	[ഇടത്_2]   = css_computed_margin_ഇടത്,
};

/** Array of per-side access functions for computed style paddings. */
സ്ഥിരം css_len_func padding_funcs[4] = {
	[മുകൾ_2]    = css_computed_padding_മുകൾ,
	[വലത്_2]  = css_computed_padding_വലത്,
	[BOTTOM] = css_computed_padding_bottom,
	[ഇടത്_2]   = css_computed_padding_ഇടത്,
};

/** Array of per-side access functions for computed style border_widths. */
സ്ഥിരം css_len_func border_width_funcs[4] = {
	[മുകൾ_2]    = css_computed_border_മുകൾ_width,
	[വലത്_2]  = css_computed_border_വലത്_width,
	[BOTTOM] = css_computed_border_bottom_width,
	[ഇടത്_2]   = css_computed_border_ഇടത്_width,
};

/** Array of per-side access functions for computed style border styles. */
സ്ഥിരം css_border_style_func border_style_funcs[4] = {
	[മുകൾ_2]    = css_computed_border_മുകൾ_style,
	[വലത്_2]  = css_computed_border_വലത്_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[ഇടത്_2]   = css_computed_border_ഇടത്_style,
};

/** Array of per-side access functions for computed style border colors. */
സ്ഥിരം css_border_color_func border_color_funcs[4] = {
	[മുകൾ_2]    = css_computed_border_മുകൾ_color,
	[വലത്_2]  = css_computed_border_വലത്_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[ഇടത്_2]   = css_computed_border_ഇടത്_color,
};

/* forward declaration to break cycles */
സ്ഥിതം ശൂന്യം layout_minmax_block(
		ഘടന box *block,
		സ്ഥിരം ഘടന gui_layout_table *font_func,
		സ്ഥിരം html_content *content);

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
സ്ഥിതം ശൂന്യം
layout_get_object_dimensions(ഘടന box *box,
			     പൂർണ്ണസംഖ്യ *width, പൂർണ്ണസംഖ്യ *height,
			     പൂർണ്ണസംഖ്യ min_width, പൂർണ്ണസംഖ്യ max_width,
			     പൂർണ്ണസംഖ്യ min_height, പൂർണ്ണസംഖ്യ max_height)
{
	പ്രതിജ്ഞാ(box->object != ശൂന്യ);
	പ്രതിജ്ഞാ(width != ശൂന്യ && height != ശൂന്യ);

	എങ്കിൽ (*width == AUTO && *height == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		പൂർണ്ണസംഖ്യ intrinsic_width = content_get_width(box->object);
		പൂർണ്ണസംഖ്യ intrinsic_height = content_get_height(box->object);

		/* use intrinsic dimensions */
		*width = intrinsic_width;
		*height = intrinsic_height;

		/* Deal with min/max-width first */
		എങ്കിൽ (min_width >  0 && min_width > *width) {
			*width = min_width;
			scaled = true;
		}
		എങ്കിൽ (max_width >= 0 && max_width < *width) {
			*width = max_width;
			scaled = true;
		}

		എങ്കിൽ (scaled && (intrinsic_width != 0)) {
			/* Update height */
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		}

		scaled = false;
		/* Deal with min/max-height */
		എങ്കിൽ (min_height >  0 && min_height > *height) {
			*height = min_height;
			scaled = true;
		}
		എങ്കിൽ (max_height >= 0 && max_height < *height) {
			*height = max_height;
			scaled = true;
		}

		എങ്കിൽ (scaled && (intrinsic_height != 0)) {
			/* Update width */
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		}

	} അല്ലെങ്കിൽ എങ്കിൽ (*width == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		പൂർണ്ണസംഖ്യ intrinsic_width = content_get_width(box->object);
		പൂർണ്ണസംഖ്യ intrinsic_height = content_get_height(box->object);

		എങ്കിൽ (intrinsic_height != 0)
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		അല്ലെങ്കിൽ
			*width = intrinsic_width;

		എങ്കിൽ (min_width >  0 && min_width > *width)
			*width = min_width;
		എങ്കിൽ (max_width >= 0 && max_width < *width)
			*width = max_width;

	} അല്ലെങ്കിൽ എങ്കിൽ (*height == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		പൂർണ്ണസംഖ്യ intrinsic_width = content_get_width(box->object);
		പൂർണ്ണസംഖ്യ intrinsic_height = content_get_height(box->object);

		എങ്കിൽ (min_width >  0 && min_width > *width)
			*width = min_width;
		എങ്കിൽ (max_width >= 0 && max_width < *width)
			*width = max_width;

		എങ്കിൽ (intrinsic_width != 0)
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		അല്ലെങ്കിൽ
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
സ്ഥിതം പൂർണ്ണസംഖ്യ layout_text_indent(
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		സ്ഥിരം css_computed_style *style, പൂർണ്ണസംഖ്യ width)
{
	css_fixed മൂല്യം = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_text_indent(style, &മൂല്യം, &unit);

	എങ്കിൽ (unit == CSS_UNIT_PCT) {
		തിരികെ FPCT_OF_INT_TOINT(മൂല്യം, width);
	} അല്ലെങ്കിൽ {
		തിരികെ FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				മൂല്യം, unit));
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
സ്ഥിതം ശൂന്യം layout_minmax_table(ഘടന box *table,
		സ്ഥിരം ഘടന gui_layout_table *font_func,
		സ്ഥിരം html_content *content)
{
	unsigned പൂർണ്ണസംഖ്യ i, j;
	പൂർണ്ണസംഖ്യ width;
	പൂർണ്ണസംഖ്യ border_spacing_h = 0;
	പൂർണ്ണസംഖ്യ table_min = 0, table_max = 0;
	പൂർണ്ണസംഖ്യ extra_fixed = 0;
	ദശാംശം extra_frac = 0;
	ഘടന column *col;
	ഘടന box *row_group, *row, *cell;

	/* check if the widths have already been calculated */
	എങ്കിൽ (table->max_width != UNKNOWN_MAX_WIDTH)
		തിരികെ;

	എങ്കിൽ (table_calculate_column_types(&content->unit_len_ctx, table) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		തിരികെ;
	}
	col = table->col;

	/* start with 0 except for fixed-width columns */
	വേണ്ടി (i = 0; i != table->columns; i++) {
		എങ്കിൽ (col[i].type == COLUMN_WIDTH_FIXED)
			col[i].min = col[i].max = col[i].width;
		അല്ലെങ്കിൽ
			col[i].min = col[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	എങ്കിൽ (css_computed_border_collapse(table->style) ==
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
	വേണ്ടി (row_group = table->children; row_group; row_group =row_group->next)
	വേണ്ടി (row = row_group->children; row; row = row->next)
	വേണ്ടി (cell = row->children; cell; cell = cell->next) {
		പ്രതിജ്ഞാ(cell->type == BOX_TABLE_CELL);
		പ്രതിജ്ഞാ(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		പ്രതിജ്ഞാ(cell->columns != 0);

		എങ്കിൽ (cell->columns != 1)
			തുടരുക;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		എങ്കിൽ (col[i].positioned)
			തുടരുക;

		/* update column min, max widths using cell widths */
		എങ്കിൽ (col[i].min < cell->min_width)
			col[i].min = cell->min_width;
		എങ്കിൽ (col[i].max < cell->max_width)
			col[i].max = cell->max_width;
	}

	/* 2nd pass: cells which span multiple columns */
	വേണ്ടി (row_group = table->children; row_group; row_group =row_group->next)
	വേണ്ടി (row = row_group->children; row; row = row->next)
	വേണ്ടി (cell = row->children; cell; cell = cell->next) {
		unsigned പൂർണ്ണസംഖ്യ flexible_columns = 0;
		പൂർണ്ണസംഖ്യ min = 0, max = 0, fixed_width = 0, extra;

		എങ്കിൽ (cell->columns == 1)
			തുടരുക;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		വേണ്ടി (j = 0; j != cell->columns; j++) {
			min += col[i + j].min;
			എങ്കിൽ (col[i + j].type == COLUMN_WIDTH_FIXED)
				fixed_width += col[i + j].width;
			അല്ലെങ്കിൽ
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		എങ്കിൽ (min < cell->min_width) {
			എങ്കിൽ (flexible_columns == 0) {
				extra = 1 + (cell->min_width - min) /
						cell->columns;
				വേണ്ടി (j = 0; j != cell->columns; j++) {
					col[i + j].min += extra;
					എങ്കിൽ (col[i + j].max < col[i + j].min)
						col[i + j].max = col[i + j].min;
				}
			} അല്ലെങ്കിൽ {
				extra = 1 + (cell->min_width - min) /
						flexible_columns;
				വേണ്ടി (j = 0; j != cell->columns; j++) {
					എങ്കിൽ (col[i + j].type !=
							COLUMN_WIDTH_FIXED) {
						col[i + j].min += extra;
						എങ്കിൽ (col[i + j].max <
								col[i + j].min)
							col[i + j].max =
								col[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		വേണ്ടി (j = 0; j != cell->columns; j++)
			max += col[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		എങ്കിൽ (max < cell->max_width && flexible_columns) {
			extra = 1 + (cell->max_width - max) / flexible_columns;
			വേണ്ടി (j = 0; j != cell->columns; j++)
				എങ്കിൽ (col[i + j].type != COLUMN_WIDTH_FIXED)
					col[i + j].max += extra;
		}
	}

	വേണ്ടി (i = 0; i != table->columns; i++) {
		എങ്കിൽ (col[i].max < col[i].min) {
			box_dump(stderr, table, 0, true);
			പ്രതിജ്ഞാ(0);
		}
		table_min += col[i].min;
		table_max += col[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	എങ്കിൽ (css_computed_width_px(table->style, &content->unit_len_ctx,
			-1, &width) == CSS_WIDTH_SET) {
		എങ്കിൽ (table_min < width)
			table_min = width;
		എങ്കിൽ (table_max < width)
			table_max = width;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, ഇടത്_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, വലത്_2, true, true, true,
			&extra_fixed, &extra_frac);
	എങ്കിൽ (extra_fixed < 0)
		extra_fixed = 0;
	എങ്കിൽ (extra_frac < 0)
		extra_frac = 0;
	എങ്കിൽ (1.0 <= extra_frac)
		extra_frac = 0.9;
	table->min_width = (table_min + extra_fixed) / (1.0 - extra_frac);
	table->max_width = (table_max + extra_fixed) / (1.0 - extra_frac);
	table->min_width += (table->columns + 1) * border_spacing_h;
	table->max_width += (table->columns + 1) * border_spacing_h;

	പ്രതിജ്ഞാ(0 <= table->min_width && table->min_width <= table->max_width);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
സ്ഥിതം വരിയിൽ bool box_has_percentage_max_width(ഘടന box *b)
{
	css_unit unit = CSS_UNIT_PX;
	എണ്ണൽ css_max_width_e type;
	css_fixed മൂല്യം = 0;

	പ്രതിജ്ഞാ(b != ശൂന്യ);

	type = css_computed_max_width(b->style, &മൂല്യം, &unit);
	തിരികെ ((type == CSS_MAX_WIDTH_SET) && (unit == CSS_UNIT_PCT));
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
സ്ഥിതം ഘടന box *
layout_minmax_വരി(ഘടന box *first,
		   പൂർണ്ണസംഖ്യ *വരി_min,
		   പൂർണ്ണസംഖ്യ *വരി_max,
		   bool first_വരി,
		   bool *വരി_has_height,
		   സ്ഥിരം ഘടന gui_layout_table *font_func,
		   സ്ഥിരം html_content *content)
{
	പൂർണ്ണസംഖ്യ min = 0, max = 0, width, height, fixed;
	ദശാംശം frac;
	size_t i, j;
	ഘടന box *b;
	ഘടന box *block;
	plot_font_style_t fstyle;
	bool no_wrap;

	പ്രതിജ്ഞാ(first->parent);
	പ്രതിജ്ഞാ(first->parent->parent);
	പ്രതിജ്ഞാ(first->parent->parent->style);

	block = first->parent->parent;
	no_wrap = (css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_PRE);

	*വരി_has_height = false;

	/* corresponds to the pass 1 loop in layout_line() */
	വേണ്ടി (b = first; b; b = b->next) {
		എണ്ണൽ css_width_e wtype;
		എണ്ണൽ css_height_e htype;
		എണ്ണൽ css_box_sizing_e bs;
		css_fixed മൂല്യം = 0;
		css_unit unit = CSS_UNIT_PX;

		പ്രതിജ്ഞാ(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		എങ്കിൽ (b->type == BOX_BR) {
			b = b->next;
			നിർത്തുക;
		}

		എങ്കിൽ (lh__box_is_float_box(b)) {
			പ്രതിജ്ഞാ(b->children);
			എങ്കിൽ (b->children->type == BOX_TABLE)
				layout_minmax_table(b->children, font_func,
						content);
			അല്ലെങ്കിൽ
				layout_minmax_block(b->children, font_func,
						content);
			b->min_width = b->children->min_width;
			b->max_width = b->children->max_width;
			എങ്കിൽ (min < b->min_width)
				min = b->min_width;
			max += b->max_width;
			തുടരുക;
		}

		എങ്കിൽ (b->type == BOX_INLINE_BLOCK || b->type == BOX_INLINE_FLEX) {
			layout_minmax_block(b, font_func, content);
			എങ്കിൽ (min < b->min_width)
				min = b->min_width;
			max += b->max_width;

			എങ്കിൽ (b->flags & HAS_HEIGHT)
				*വരി_has_height = true;
			തുടരുക;
		}

		പ്രതിജ്ഞാ(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		എങ്കിൽ (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->style, ഇടത്_2, true, true, true,
					&fixed, &frac);
			എങ്കിൽ (!b->inline_end)
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, വലത്_2,
						true, true, true,
						&fixed, &frac);
			എങ്കിൽ (0 < fixed)
				max += fixed;
			*വരി_has_height = true;
			/* \todo  update min width, consider fractional extra */
		} അല്ലെങ്കിൽ എങ്കിൽ (b->type == BOX_INLINE_END) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->inline_end->style, വലത്_2,
					true, true, true,
					&fixed, &frac);
			എങ്കിൽ (0 < fixed)
				max += fixed;

			എങ്കിൽ (b->next) {
				എങ്കിൽ (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*വരി_has_height = true;
			തുടരുക;
		}

		എങ്കിൽ (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			എങ്കിൽ (!b->text)
				തുടരുക;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			എങ്കിൽ (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				എങ്കിൽ (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					പൂർണ്ണസംഖ്യ opt_maxwidth = 0;
					ഘടന form_option *o;

					വേണ്ടി (o = b->parent->parent->gadget->
							data.select.ഇനങ്ങൾ; o;
							o = o->next) {
						പൂർണ്ണസംഖ്യ opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						എങ്കിൽ (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}

					b->width = opt_maxwidth;
					എങ്കിൽ (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;

				} അല്ലെങ്കിൽ {
					font_func->width(&fstyle, b->text,
						b->length, &b->width);
					b->flags |= MEASURED;
				}
			}
			max += b->width;
			എങ്കിൽ (b->next) {
				എങ്കിൽ (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			എങ്കിൽ (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} അല്ലെങ്കിൽ എങ്കിൽ (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				എങ്കിൽ (min < b->width)
					min = b->width;

			} അല്ലെങ്കിൽ എങ്കിൽ (b->parent->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				ചെയ്യുക {
					വേണ്ടി (j = i; j != b->length &&
							b->text[j] != ' '; j++)
						;
					font_func->width(&fstyle, b->text + i,
							 j - i, &width);
					എങ്കിൽ (min < width)
						min = width;
					i = j + 1;
				} ആകുമ്പോൾ (j != b->length);
			}

			*വരി_has_height = true;

			തുടരുക;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		പ്രതിജ്ഞാ(b->style);

		bs = css_computed_box_sizing(block->style);

		/* calculate box width */
		wtype = css_computed_width_px(b->style,
				&content->unit_len_ctx, -1, &width);
		എങ്കിൽ (wtype == CSS_WIDTH_SET) {
			എങ്കിൽ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, ഇടത്_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, വലത്_2,
						false, true, true,
						&fixed, &frac);
				എങ്കിൽ (width < fixed) {
					width = fixed;
				}
			}
			എങ്കിൽ (width < 0)
				width = 0;
		} അല്ലെങ്കിൽ {
			width = AUTO;
		}

		/* height */
		htype = css_computed_height(b->style, &മൂല്യം, &unit);
		എങ്കിൽ (htype == CSS_HEIGHT_SET) {
			height = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					മൂല്യം, unit));
		} അല്ലെങ്കിൽ {
			height = AUTO;
		}

		എങ്കിൽ (b->object || (b->flags & REPLACE_DIM)) {
			എങ്കിൽ (b->object) {
				പൂർണ്ണസംഖ്യ temp_height = height;
				layout_get_object_dimensions(b,
						&width, &temp_height,
						INT_MIN, INT_MAX,
						INT_MIN, INT_MAX);
			}

			fixed = frac = 0;
			എങ്കിൽ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ഇടത്_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, വലത്_2,
						true, false, false,
						&fixed, &frac);
			} അല്ലെങ്കിൽ {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ഇടത്_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, വലത്_2,
						true, true, true,
						&fixed, &frac);
			}
			എങ്കിൽ (0 < width + fixed)
				width += fixed;
		} അല്ലെങ്കിൽ എങ്കിൽ (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			എങ്കിൽ (width == AUTO)
				width = 400;

			fixed = frac = 0;
			എങ്കിൽ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ഇടത്_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, വലത്_2,
						true, false, false,
						&fixed, &frac);
			} അല്ലെങ്കിൽ {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ഇടത്_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, വലത്_2,
						true, true, true,
						&fixed, &frac);
			}

			എങ്കിൽ (0 < width + fixed)
				width += fixed;

		} അല്ലെങ്കിൽ {
			/* form control with no object */
			എങ്കിൽ (width == AUTO)
				width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		എങ്കിൽ (min < width && !box_has_percentage_max_width(b))
			min = width;
		എങ്കിൽ (width > 0)
			max += width;

		*വരി_has_height = true;
	}

	എങ്കിൽ (first_വരി) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		പൂർണ്ണസംഖ്യ text_indent = layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, 100);
		min = (min + text_indent < 0) ? 0 : min + text_indent;
		max = (max + text_indent < 0) ? 0 : max + text_indent;
	}

	*വരി_min = min;
	*വരി_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	പ്രതിജ്ഞാ(b != first);
	പ്രതിജ്ഞാ(0 <= *വരി_min);
	പ്രതിജ്ഞാ(*വരി_min <= *വരി_max);
	തിരികെ b;
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
സ്ഥിതം ശൂന്യം
layout_minmax_inline_container(ഘടന box *inline_container,
			       bool *has_height,
			       സ്ഥിരം ഘടന gui_layout_table *font_func,
			       സ്ഥിരം html_content *content)
{
	ഘടന box *child;
	പൂർണ്ണസംഖ്യ വരി_min = 0, വരി_max = 0;
	പൂർണ്ണസംഖ്യ min = 0, max = 0;
	bool first_വരി = true;
	bool വരി_has_height;

	പ്രതിജ്ഞാ(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	എങ്കിൽ (inline_container->max_width != UNKNOWN_MAX_WIDTH)
		തിരികെ;

	*has_height = false;

	വേണ്ടി (child = inline_container->children; child; ) {
		child = layout_minmax_വരി(child, &വരി_min, &വരി_max,
				first_വരി, &വരി_has_height, font_func,
				content);
		എങ്കിൽ (min < വരി_min)
			min = വരി_min;
		എങ്കിൽ (max < വരി_max)
			max = വരി_max;
		first_വരി = false;
		*has_height |= വരി_has_height;
	}

	inline_container->min_width = min;
	inline_container->max_width = max;

	പ്രതിജ്ഞാ(0 <= inline_container->min_width &&
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
സ്ഥിതം ശൂന്യം layout_minmax_block(
		ഘടന box *block,
		സ്ഥിരം ഘടന gui_layout_table *font_func,
		സ്ഥിരം html_content *content)
{
	ഘടന box *child;
	പൂർണ്ണസംഖ്യ min = 0, max = 0;
	പൂർണ്ണസംഖ്യ extra_fixed = 0;
	ദശാംശം extra_frac = 0;
	എണ്ണൽ css_width_e wtype = CSS_WIDTH_AUTO;
	css_fixed width = 0;
	css_unit wunit = CSS_UNIT_PX;
	എണ്ണൽ css_height_e htype = CSS_HEIGHT_AUTO;
	css_fixed height = 0;
	css_unit hunit = CSS_UNIT_PX;
	എണ്ണൽ css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool child_has_height = false;

	പ്രതിജ്ഞാ(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	/* check if the widths have already been calculated */
	എങ്കിൽ (block->max_width != UNKNOWN_MAX_WIDTH)
		തിരികെ;

	എങ്കിൽ (block->style != ശൂന്യ) {
		wtype = css_computed_width(block->style, &width, &wunit);
		htype = css_computed_height(block->style, &height, &hunit);
		bs = css_computed_box_sizing(block->style);
	}

	/* set whether the minimum width is of any interest for this box */
	എങ്കിൽ (((block->parent && lh__box_is_float_box(block->parent)) ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX) &&
			wtype != CSS_WIDTH_SET) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} അല്ലെങ്കിൽ എങ്കിൽ (block->type == BOX_TABLE_CELL) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} അല്ലെങ്കിൽ എങ്കിൽ (block->parent && (block->parent->flags & NEED_MIN) &&
			wtype != CSS_WIDTH_SET) {
		/* box inside shrink-to-fit context; need minimum width */
		block->flags |= NEED_MIN;
	} അല്ലെങ്കിൽ എങ്കിൽ (block->parent && (block->parent->type == BOX_FLEX)) {
		/* box is flex item */
		block->flags |= NEED_MIN;
	}

	എങ്കിൽ (block->gadget && (block->gadget->type == GADGET_TEXTBOX ||
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

	എങ്കിൽ (block->gadget && (block->gadget->type == GADGET_RADIO ||
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

	എങ്കിൽ (block->object) {
		എങ്കിൽ (content_get_type(block->object) == CONTENT_HTML) {
			layout_minmax_block(html_get_box_വൃക്ഷം(block->object),
					font_func, content);
			min = html_get_box_വൃക്ഷം(block->object)->min_width;
			max = html_get_box_വൃക്ഷം(block->object)->max_width;
		} അല്ലെങ്കിൽ {
			min = max = content_get_width(block->object);
		}

		block->flags |= HAS_HEIGHT;
	} അല്ലെങ്കിൽ എങ്കിൽ (block->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		block->flags |= HAS_HEIGHT;
	} അല്ലെങ്കിൽ {
		/* recurse through children */
		വേണ്ടി (child = block->children; child; child = child->next) {
			തിരഞ്ഞെടുപ്പ് (child->type) {
			അവസ്ഥ BOX_FLEX:
			അവസ്ഥ BOX_BLOCK:
				layout_minmax_block(child, font_func,
						content);
				എങ്കിൽ (child->flags & HAS_HEIGHT)
					child_has_height = true;
				നിർത്തുക;
			അവസ്ഥ BOX_INLINE_CONTAINER:
				എങ്കിൽ (block->flags & NEED_MIN)
					child->flags |= NEED_MIN;

				layout_minmax_inline_container(child,
						&child_has_height, font_func,
						content);
				എങ്കിൽ (child_has_height &&
						child ==
						child->parent->children) {
					block->flags |= MAKE_HEIGHT;
				}
				നിർത്തുക;
			അവസ്ഥ BOX_TABLE:
				layout_minmax_table(child, font_func,
						content);
				/* todo: fix for zero height tables */
				child_has_height = true;
				child->flags |= MAKE_HEIGHT;
				നിർത്തുക;
			സ്ഥിരസ്ഥിതി:
				പ്രതിജ്ഞാ(0);
			}
			പ്രതിജ്ഞാ(child->max_width != UNKNOWN_MAX_WIDTH);

			എങ്കിൽ (child->style &&
					(css_computed_position(child->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(child->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				തുടരുക;
			}

			എങ്കിൽ (lh__box_is_flex_container(block) &&
			    lh__flex_main_is_horizontal(block)) {
				എങ്കിൽ (block->style != ശൂന്യ &&
				    css_computed_flex_wrap(block->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += child->min_width;
				} അല്ലെങ്കിൽ {
					എങ്കിൽ (min < child->min_width)
						min = child->min_width;
				}
				max += child->max_width;

			} അല്ലെങ്കിൽ {
				എങ്കിൽ (min < child->min_width)
					min = child->min_width;
				എങ്കിൽ (max < child->max_width)
					max = child->max_width;
			}

			എങ്കിൽ (child_has_height)
				block->flags |= HAS_HEIGHT;
		}
	}

	എങ്കിൽ (max < min) {
		box_dump(stderr, block, 0, true);
		പ്രതിജ്ഞാ(0);
	}

	/* fixed width takes priority */
	എങ്കിൽ (block->type != BOX_TABLE_CELL && !lh__box_is_flex_item(block)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		എണ്ണൽ css_max_width_e max_type;
		എണ്ണൽ css_min_width_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed മൂല്യം = 0;
		പൂർണ്ണസംഖ്യ width;

		എങ്കിൽ (css_computed_width_px(block->style, &content->unit_len_ctx,
				-1, &width) == CSS_WIDTH_SET) {
			min = max = width;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_width(block->style, &മൂല്യം, &unit);
		എങ്കിൽ (min_type == CSS_MIN_WIDTH_SET && unit != CSS_UNIT_PCT) {
			പൂർണ്ണസംഖ്യ val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, മൂല്യം, unit));

			എങ്കിൽ (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_width(block->style, &മൂല്യം, &unit);
		എങ്കിൽ (max_type == CSS_MAX_WIDTH_SET && unit != CSS_UNIT_PCT) {
			പൂർണ്ണസംഖ്യ val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, മൂല്യം, unit));

			എങ്കിൽ (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	എങ്കിൽ (htype == CSS_HEIGHT_SET && hunit != CSS_UNIT_PCT &&
			height > INTTOFIX(0)) {
		block->flags |= MAKE_HEIGHT;
		block->flags |= HAS_HEIGHT;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_width(&content->unit_len_ctx, block->style, ഇടത്_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, വലത്_2,
			false, true, true, &extra_fixed, &extra_frac);

	എങ്കിൽ (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	എങ്കിൽ (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	എങ്കിൽ (max < min) {
		min = max;
	}

	calculate_mbp_width(&content->unit_len_ctx, block->style, ഇടത്_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, വലത്_2,
			true, false, false, &extra_fixed, &extra_frac);

	എങ്കിൽ (extra_fixed < 0)
		extra_fixed = 0;
	എങ്കിൽ (extra_frac < 0)
		extra_frac = 0;
	എങ്കിൽ (1.0 <= extra_frac)
		extra_frac = 0.9;
	എങ്കിൽ (block->style != ശൂന്യ &&
			(css_computed_float(block->style) == CSS_FLOAT_ഇടത് ||
			css_computed_float(block->style) == CSS_FLOAT_വലത്)) {
		/* floated boxs */
		block->min_width = min + extra_fixed;
		block->max_width = max + extra_fixed;
	} അല്ലെങ്കിൽ {
		/* not floated */
		block->min_width = (min + extra_fixed) / (1.0 - extra_frac);
		block->max_width = (max + extra_fixed) / (1.0 - extra_frac);
	}

	പ്രതിജ്ഞാ(0 <= block->min_width);
	പ്രതിജ്ഞാ(block->min_width <= block->max_width);
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
സ്ഥിതം ഘടന box*
layout_next_margin_block(സ്ഥിരം css_unit_ctx *unit_len_ctx,
			 ഘടന box *box,
			 ഘടന box *block,
			 പൂർണ്ണസംഖ്യ viewport_height,
			 പൂർണ്ണസംഖ്യ *max_pos_margin,
			 പൂർണ്ണസംഖ്യ *max_neg_margin)
{
	പ്രതിജ്ഞാ(block != ശൂന്യ);

	ആകുമ്പോൾ (box != ശൂന്യ) {

		എങ്കിൽ (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			എങ്കിൽ (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						ശൂന്യ, ശൂന്യ, ശൂന്യ, ശൂന്യ,
						ശൂന്യ, ശൂന്യ, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				എങ്കിൽ (*max_pos_margin < box->margin[മുകൾ_2])
					*max_pos_margin = box->margin[മുകൾ_2];
				അല്ലെങ്കിൽ എങ്കിൽ (*max_neg_margin < -box->margin[മുകൾ_2])
					*max_neg_margin = -box->margin[മുകൾ_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			എങ്കിൽ (box->flags & MAKE_HEIGHT ||
					box->border[മുകൾ_2].width ||
					box->padding[മുകൾ_2] ||
					(box->style &&
					css_computed_overflow_y(box->style) !=
					CSS_OVERFLOW_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_first_child(box))) {
				/* Collapse to this box; return it */
				തിരികെ box;
			}
		}


		/* Find next box */
		എങ്കിൽ (box->type == BOX_BLOCK && !box->object && box->children &&
				box->style &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (!box->next) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				ചെയ്യുക {
					/* Apply bottom margin */
					എങ്കിൽ (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					അല്ലെങ്കിൽ എങ്കിൽ (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->parent;
				} ആകുമ്പോൾ (box != block && !box->next);

				എങ്കിൽ (box == block) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					തിരികെ block;
				}
			}

			/* Apply bottom margin */
			എങ്കിൽ (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			അല്ലെങ്കിൽ എങ്കിൽ (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->next;

			/* Get margins */
			എങ്കിൽ (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						ശൂന്യ, ശൂന്യ, ശൂന്യ, ശൂന്യ,
						ശൂന്യ, ശൂന്യ, box->margin,
						box->padding, box->border);
			}
		}
	}

	തിരികെ ശൂന്യ;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ layout_clear(ഘടന box *fl, എണ്ണൽ css_clear_e clear)
{
	പൂർണ്ണസംഖ്യ y = 0;
	വേണ്ടി (; fl; fl = fl->next_float) {
		എങ്കിൽ ((clear == CSS_CLEAR_ഇടത് || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ഇടത്)
			എങ്കിൽ (y < fl->y + fl->height)
				y = fl->y + fl->height;
		എങ്കിൽ ((clear == CSS_CLEAR_വലത് || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_വലത്)
			എങ്കിൽ (y < fl->y + fl->height)
				y = fl->y + fl->height;
	}
	തിരികെ y;
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
സ്ഥിതം ശൂന്യം
find_sides(ഘടന box *fl,
	   പൂർണ്ണസംഖ്യ y0, പൂർണ്ണസംഖ്യ y1,
	   പൂർണ്ണസംഖ്യ *x0, പൂർണ്ണസംഖ്യ *x1,
	   ഘടന box **ഇടത്,
	   ഘടന box **വലത്)
{
	പൂർണ്ണസംഖ്യ fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*ഇടത് = *വലത് = 0;
	വേണ്ടി (; fl; fl = fl->next_float) {
		fy1 = fl->y + fl->height;
		എങ്കിൽ (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			തിരികെ;
		}
		fy0 = fl->y;
		എങ്കിൽ (y0 < fy1 && fy0 <= y1) {
			എങ്കിൽ (fl->type == BOX_FLOAT_ഇടത്) {
				fx1 = fl->x + fl->width;
				എങ്കിൽ (*x0 < fx1) {
					*x0 = fx1;
					*ഇടത് = fl;
				}
			} അല്ലെങ്കിൽ {
				fx0 = fl->x;
				എങ്കിൽ (fx0 < *x1) {
					*x1 = fx0;
					*വലത് = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *ഇടത്, *വലത്);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ
layout_solve_width(ഘടന box *box,
		   പൂർണ്ണസംഖ്യ available_width,
		   പൂർണ്ണസംഖ്യ width,
		   പൂർണ്ണസംഖ്യ lm,
		   പൂർണ്ണസംഖ്യ rm,
		   പൂർണ്ണസംഖ്യ max_width,
		   പൂർണ്ണസംഖ്യ min_width)
{
	bool auto_width = false;

	/* Increase specified left/right margins */
	എങ്കിൽ (box->margin[ഇടത്_2] != AUTO && box->margin[ഇടത്_2] < lm &&
			box->margin[ഇടത്_2] >= 0)
		box->margin[ഇടത്_2] = lm;
	എങ്കിൽ (box->margin[വലത്_2] != AUTO && box->margin[വലത്_2] < rm &&
			box->margin[വലത്_2] >= 0)
		box->margin[വലത്_2] = rm;

	/* Find width */
	എങ്കിൽ (width == AUTO) {
		പൂർണ്ണസംഖ്യ margin_ഇടത് = box->margin[ഇടത്_2];
		പൂർണ്ണസംഖ്യ margin_വലത് = box->margin[വലത്_2];

		എങ്കിൽ (margin_ഇടത് == AUTO) {
			margin_ഇടത് = lm;
		}
		എങ്കിൽ (margin_വലത് == AUTO) {
			margin_വലത് = rm;
		}

		width = available_width -
				(margin_ഇടത് + box->border[ഇടത്_2].width +
				box->padding[ഇടത്_2] + box->padding[വലത്_2] +
				box->border[വലത്_2].width + margin_വലത്);
		width = width < 0 ? 0 : width;
		auto_width = true;
	}

	എങ്കിൽ (max_width >= 0 && width > max_width) {
		/* max-width is admissable and width exceeds max-width */
		width = max_width;
		auto_width = false;
	}

	എങ്കിൽ (min_width > 0 && width < min_width) {
		/* min-width is admissable and width is less than max-width */
		width = min_width;
		auto_width = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	എങ്കിൽ (auto_width) {
		/* any other 'auto' become 0 or the minimum required values */
		എങ്കിൽ (box->margin[ഇടത്_2] == AUTO) {
			box->margin[ഇടത്_2] = lm;
		}
		എങ്കിൽ (box->margin[വലത്_2] == AUTO) {
			box->margin[വലത്_2] = rm;
		}
		തിരികെ width;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	എങ്കിൽ (box->margin[ഇടത്_2] != AUTO && box->margin[വലത്_2] != AUTO &&
			box->parent != ശൂന്യ && box->parent->style != ശൂന്യ) {
		തിരഞ്ഞെടുപ്പ് (css_computed_text_align(box->parent->style)) {
		അവസ്ഥ CSS_TEXT_ALIGN_LIBCSS_വലത്:
			box->margin[ഇടത്_2] = AUTO;
			box->margin[വലത്_2] = 0;
			നിർത്തുക;
		അവസ്ഥ CSS_TEXT_ALIGN_LIBCSS_CENTER:
			box->margin[ഇടത്_2] = box->margin[വലത്_2] = AUTO;
			നിർത്തുക;
		അവസ്ഥ CSS_TEXT_ALIGN_LIBCSS_ഇടത്:
			box->margin[ഇടത്_2] = 0;
			box->margin[വലത്_2] = AUTO;
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			/* Leave it alone; no HTML alignment */
			നിർത്തുക;
		}
	}

	എങ്കിൽ (box->margin[ഇടത്_2] == AUTO && box->margin[വലത്_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[ഇടത്_2] = box->margin[വലത്_2] =
				(available_width - lm - rm -
				(box->border[ഇടത്_2].width + box->padding[ഇടത്_2] +
				width + box->padding[വലത്_2] +
				box->border[വലത്_2].width)) / 2;

		എങ്കിൽ (box->margin[ഇടത്_2] < 0) {
			box->margin[വലത്_2] += box->margin[ഇടത്_2];
			box->margin[ഇടത്_2] = 0;
		}

		box->margin[ഇടത്_2] += lm;

	} അല്ലെങ്കിൽ എങ്കിൽ (box->margin[ഇടത്_2] == AUTO) {
		box->margin[ഇടത്_2] = available_width - lm -
				(box->border[ഇടത്_2].width + box->padding[ഇടത്_2] +
				width + box->padding[വലത്_2] +
				box->border[വലത്_2].width + box->margin[വലത്_2]);
		box->margin[ഇടത്_2] = box->margin[ഇടത്_2] < lm
				? lm : box->margin[ഇടത്_2];
	} അല്ലെങ്കിൽ {
		/* margin-right auto or "over-constrained" */
		box->margin[വലത്_2] = available_width - rm -
				(box->margin[ഇടത്_2] + box->border[ഇടത്_2].width +
				 box->padding[ഇടത്_2] + width +
				 box->padding[വലത്_2] +
				 box->border[വലത്_2].width);
	}

	തിരികെ width;
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
സ്ഥിതം ശൂന്യം
layout_block_find_dimensions(സ്ഥിരം css_unit_ctx *unit_len_ctx,
			     പൂർണ്ണസംഖ്യ available_width,
			     പൂർണ്ണസംഖ്യ viewport_height,
			     പൂർണ്ണസംഖ്യ lm,
			     പൂർണ്ണസംഖ്യ rm,
			     ഘടന box *box)
{
	പൂർണ്ണസംഖ്യ width, max_width, min_width;
	പൂർണ്ണസംഖ്യ height, max_height, min_height;
	പൂർണ്ണസംഖ്യ *margin = box->margin;
	പൂർണ്ണസംഖ്യ *padding = box->padding;
	ഘടന box_border *border = box->border;
	സ്ഥിരം css_computed_style *style = box->style;

	layout_find_dimensions(unit_len_ctx, available_width, viewport_height, box,
			style, &width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	എങ്കിൽ (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	}

	box->width = layout_solve_width(box, available_width, width, lm, rm,
			max_width, min_width);
	box->height = height;

	എങ്കിൽ (margin[മുകൾ_2] == AUTO)
		margin[മുകൾ_2] = 0;
	എങ്കിൽ (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
സ്ഥിതം ശൂന്യം layout_block_add_scrollbar(ഘടന box *box, പൂർണ്ണസംഖ്യ which)
{
	എണ്ണൽ css_overflow_e overflow_x, overflow_y;

	പ്രതിജ്ഞാ(box->type == BOX_BLOCK && (which == വലത്_2 || which == BOTTOM));

	എങ്കിൽ (box->style == ശൂന്യ)
		തിരികെ;

	overflow_x = css_computed_overflow_x(box->style);
	overflow_y = css_computed_overflow_y(box->style);

	എങ്കിൽ (which == BOTTOM &&
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		എങ്കിൽ (box->height != AUTO &&
				(overflow_x == CSS_OVERFLOW_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_WIDTH;
		}

	} അല്ലെങ്കിൽ എങ്കിൽ (which == വലത്_2 &&
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		എണ്ണൽ css_height_e htype;
		css_fixed height = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_height(box->style, &height, &hunit);

		എങ്കിൽ (which == വലത്_2 && box->width != AUTO &&
				htype == CSS_HEIGHT_SET &&
				(overflow_y == CSS_OVERFLOW_SCROLL ||
				box_vscrollbar_present(box))) {
			box->width -= SCROLLBAR_WIDTH;
			box->padding[വലത്_2] += SCROLLBAR_WIDTH;
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
സ്ഥിതം ശൂന്യം layout_move_children(ഘടന box *box, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	പ്രതിജ്ഞാ(box);

	വേണ്ടി (box = box->children; box; box = box->next) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_table(
		ഘടന box *table,
		പൂർണ്ണസംഖ്യ available_width,
		html_content *content)
{
	unsigned പൂർണ്ണസംഖ്യ columns = table->columns;  /* total columns */
	unsigned പൂർണ്ണസംഖ്യ i;
	unsigned പൂർണ്ണസംഖ്യ *row_span;
	പൂർണ്ണസംഖ്യ *excess_y;
	പൂർണ്ണസംഖ്യ table_width, min_width = 0, max_width = 0;
	പൂർണ്ണസംഖ്യ required_width = 0;
	പൂർണ്ണസംഖ്യ x, remainder = 0, എണ്ണം = 0;
	പൂർണ്ണസംഖ്യ table_height = 0;
	പൂർണ്ണസംഖ്യ min_height = 0;
	പൂർണ്ണസംഖ്യ *xs;  /* array of column x positions */
	പൂർണ്ണസംഖ്യ auto_width;
	പൂർണ്ണസംഖ്യ spare_width;
	പൂർണ്ണസംഖ്യ relative_sum = 0;
	പൂർണ്ണസംഖ്യ border_spacing_h = 0, border_spacing_v = 0;
	പൂർണ്ണസംഖ്യ spare_height;
	പൂർണ്ണസംഖ്യ positioned_columns = 0;
	ഘടന box *containing_block = ശൂന്യ;
	ഘടന box *c;
	ഘടന box *row;
	ഘടന box *row_group;
	ഘടന box **row_span_cell;
	ഘടന column *col;
	സ്ഥിരം css_computed_style *style = table->style;
	എണ്ണൽ css_height_e htype;
	css_fixed മൂല്യം = 0;
	css_unit unit = CSS_UNIT_PX;

	പ്രതിജ്ഞാ(table->type == BOX_TABLE);
	പ്രതിജ്ഞാ(style);
	പ്രതിജ്ഞാ(table->children && table->children->children);
	പ്രതിജ്ഞാ(columns);

	/* allocate working buffers */
	col = malloc(columns * വലുപ്പം col[0]);
	excess_y = malloc(columns * വലുപ്പം excess_y[0]);
	row_span = malloc(columns * വലുപ്പം row_span[0]);
	row_span_cell = malloc(columns * വലുപ്പം row_span_cell[0]);
	xs = malloc((columns + 1) * വലുപ്പം xs[0]);
	എങ്കിൽ (!col || !xs || !row_span || !excess_y || !row_span_cell) {
		സ്വതന്ത്രം(col);
		സ്വതന്ത്രം(excess_y);
		സ്വതന്ത്രം(row_span);
		സ്വതന്ത്രം(row_span_cell);
		സ്വതന്ത്രം(xs);
		തിരികെ false;
	}

	memcpy(col, table->col, വലുപ്പം(col[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1, table,
			style, 0, 0, 0, 0, 0, 0, table->margin, table->padding,
			table->border);
	വേണ്ടി (row_group = table->children; row_group;
			row_group = row_group->next) {
		വേണ്ടി (row = row_group->children; row; row = row->next) {
			വേണ്ടി (c = row->children; c; c = c->next) {
				എണ്ണൽ css_overflow_e overflow_x;
				എണ്ണൽ css_overflow_e overflow_y;

				പ്രതിജ്ഞാ(c->style);
				table_used_border_for_cell(
						&content->unit_len_ctx, c);
				layout_find_dimensions(&content->unit_len_ctx,
						available_width, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				overflow_x = css_computed_overflow_x(c->style);
				overflow_y = css_computed_overflow_y(c->style);

				എങ്കിൽ (overflow_x == CSS_OVERFLOW_SCROLL ||
						overflow_x ==
						CSS_OVERFLOW_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_WIDTH;
				}
				എങ്കിൽ (overflow_y == CSS_OVERFLOW_SCROLL ||
						overflow_y ==
						CSS_OVERFLOW_AUTO) {
					c->padding[വലത്_2] += SCROLLBAR_WIDTH;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	എങ്കിൽ (css_computed_border_collapse(style) ==
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
	എങ്കിൽ (css_computed_width_px(style, &content->unit_len_ctx,
			available_width, &table_width) == CSS_WIDTH_SET) {
		/* specified width includes border */
		table_width -= table->border[ഇടത്_2].width +
				table->border[വലത്_2].width;
		table_width = table_width < 0 ? 0 : table_width;

		auto_width = table_width;
	} അല്ലെങ്കിൽ {
		table_width = AUTO;
		auto_width = available_width -
				((table->margin[ഇടത്_2] == AUTO ? 0 :
						table->margin[ഇടത്_2]) +
				 table->border[ഇടത്_2].width +
				 table->padding[ഇടത്_2] +
				 table->padding[വലത്_2] +
				 table->border[വലത്_2].width +
				 (table->margin[വലത്_2] == AUTO ? 0 :
						table->margin[വലത്_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_height(style, &മൂല്യം, &unit);
	എങ്കിൽ (htype == CSS_HEIGHT_SET) {
		എങ്കിൽ (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			എങ്കിൽ (css_computed_position(table->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				പ്രതിജ്ഞാ(table->float_container);
				containing_block = table->float_container;
			} അല്ലെങ്കിൽ എങ്കിൽ (table->float_container &&
					css_computed_position(table->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(table->style) ==
						CSS_FLOAT_ഇടത് ||
					css_computed_float(table->style) ==
						CSS_FLOAT_വലത്)) {
				/* Table is a float */
				പ്രതിജ്ഞാ(table->parent && table->parent->parent &&
						table->parent->parent->parent);
				containing_block =
						table->parent->parent->parent;
			} അല്ലെങ്കിൽ എങ്കിൽ (table->parent && table->parent->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_block = table->parent;
			} അല്ലെങ്കിൽ എങ്കിൽ (table->parent && table->parent->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				പ്രതിജ്ഞാ(table->parent->parent);
				containing_block = table->parent->parent;
			}

			എങ്കിൽ (containing_block) {
				css_fixed ignored = 0;

				htype = css_computed_height(
						containing_block->style,
						&ignored, &unit);
			}

			എങ്കിൽ (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(table->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_HEIGHT_SET)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_height = FPCT_OF_INT_TOINT(മൂല്യം,
						containing_block->height);
			}
		} അല്ലെങ്കിൽ {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_height = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					മൂല്യം, unit));
		}
	}

	/* calculate width required by cells */
	വേണ്ടി (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      table,
		      i,
		      ((സ്ഥിരം അക്ഷരം *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[col[i].type],
		      col[i].width,
		      col[i].min,
		      col[i].max);


		എങ്കിൽ (col[i].positioned) {
			positioned_columns++;
			തുടരുക;
		} അല്ലെങ്കിൽ എങ്കിൽ (col[i].type == COLUMN_WIDTH_FIXED) {
			എങ്കിൽ (col[i].width < col[i].min)
				col[i].width = col[i].max = col[i].min;
			അല്ലെങ്കിൽ
				col[i].min = col[i].max = col[i].width;
			required_width += col[i].width;
		} അല്ലെങ്കിൽ എങ്കിൽ (col[i].type == COLUMN_WIDTH_PERCENT) {
			പൂർണ്ണസംഖ്യ width = col[i].width * auto_width / 100;
			required_width += col[i].min < width ? width :
					col[i].min;
		} അല്ലെങ്കിൽ
			required_width += col[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_width);
	}
	required_width += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", table_width,
	      table->min_width, table->max_width, auto_width, required_width);

	എങ്കിൽ (auto_width < required_width) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		വേണ്ടി (i = 0; i != columns; i++) {
			എങ്കിൽ (col[i].type == COLUMN_WIDTH_RELATIVE)
				തുടരുക;
			എങ്കിൽ (col[i].type == COLUMN_WIDTH_PERCENT) {
				col[i].max = auto_width * col[i].width / 100;
				എങ്കിൽ (col[i].max < col[i].min)
					col[i].max = col[i].min;
			}
			min_width += col[i].min;
			max_width += col[i].max;
		}
	} അല്ലെങ്കിൽ {
		/* take percentages exactly */
		വേണ്ടി (i = 0; i != columns; i++) {
			എങ്കിൽ (col[i].type == COLUMN_WIDTH_RELATIVE)
				തുടരുക;
			എങ്കിൽ (col[i].type == COLUMN_WIDTH_PERCENT) {
				പൂർണ്ണസംഖ്യ width = auto_width * col[i].width / 100;
				എങ്കിൽ (width < col[i].min)
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
	വേണ്ടി (i = 0; i != columns; i++) {
		എങ്കിൽ (col[i].type == COLUMN_WIDTH_RELATIVE)
			relative_sum += col[i].width;
		അല്ലെങ്കിൽ എങ്കിൽ (col[i].type == COLUMN_WIDTH_FIXED)
			spare_width -= col[i].width;
		അല്ലെങ്കിൽ
			spare_width -= col[i].min;
	}
	spare_width -= (columns + 1) * border_spacing_h;
	എങ്കിൽ (relative_sum != 0) {
		എങ്കിൽ (spare_width < 0)
			spare_width = 0;
		വേണ്ടി (i = 0; i != columns; i++) {
			എങ്കിൽ (col[i].type == COLUMN_WIDTH_RELATIVE) {
				col[i].min = ceil(col[i].max =
						(ദശാംശം) spare_width
						* (ദശാംശം) col[i].width
						/ relative_sum);
				min_width += col[i].min;
				max_width += col[i].max;
			}
		}
	}
	min_width += (columns + 1) * border_spacing_h;
	max_width += (columns + 1) * border_spacing_h;

	എങ്കിൽ (auto_width <= min_width) {
		/* not enough space: minimise column widths */
		വേണ്ടി (i = 0; i < columns; i++) {
			col[i].width = col[i].min;
		}
		table_width = min_width;
	} അല്ലെങ്കിൽ എങ്കിൽ (max_width <= auto_width) {
		/* more space than maximum width */
		എങ്കിൽ (table_width == AUTO) {
			/* for auto-width tables, make columns max width */
			വേണ്ടി (i = 0; i < columns; i++) {
				col[i].width = col[i].max;
			}
			table_width = max_width;
		} അല്ലെങ്കിൽ {
			/* for fixed-width tables, distribute the extra space
			 * too */
			unsigned പൂർണ്ണസംഖ്യ flexible_columns = 0;
			വേണ്ടി (i = 0; i != columns; i++)
				എങ്കിൽ (col[i].type != COLUMN_WIDTH_FIXED)
					flexible_columns++;
			എങ്കിൽ (flexible_columns == 0) {
				പൂർണ്ണസംഖ്യ extra = (table_width - max_width) / columns;
				remainder = (table_width - max_width) -
						(extra * columns);
				വേണ്ടി (i = 0; i != columns; i++) {
					col[i].width = col[i].max + extra;
					എണ്ണം -= remainder;
					എങ്കിൽ (എണ്ണം < 0) {
						col[i].width++;
						എണ്ണം += columns;
					}
				}

			} അല്ലെങ്കിൽ {
				പൂർണ്ണസംഖ്യ extra = (table_width - max_width) /
						flexible_columns;
				remainder = (table_width - max_width) -
						(extra * flexible_columns);
				വേണ്ടി (i = 0; i != columns; i++)
					എങ്കിൽ (col[i].type != COLUMN_WIDTH_FIXED) {
						col[i].width = col[i].max +
								extra;
						എണ്ണം -= remainder;
						എങ്കിൽ (എണ്ണം < 0) {
							col[i].width++;
							എണ്ണം += flexible_columns;
						}
					}
			}
		}
	} അല്ലെങ്കിൽ {
		/* space between min and max: fill it exactly */
		ദശാംശം scale = (ദശാംശം) (auto_width - min_width) /
				(ദശാംശം) (max_width - min_width);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		വേണ്ടി (i = 0; i < columns; i++) {
			col[i].width = col[i].min + (പൂർണ്ണസംഖ്യ) (0.5 +
					(col[i].max - col[i].min) * scale);
		}
		table_width = auto_width;
	}

	xs[0] = x = border_spacing_h;
	വേണ്ടി (i = 0; i != columns; i++) {
		എങ്കിൽ (!col[i].positioned)
			x += col[i].width + border_spacing_h;
		xs[i + 1] = x;
		row_span[i] = 0;
		excess_y[i] = 0;
		row_span_cell[i] = 0;
	}

	/* position cells */
	table_height = border_spacing_v;
	വേണ്ടി (row_group = table->children; row_group;
			row_group = row_group->next) {
		പൂർണ്ണസംഖ്യ row_group_height = 0;
		വേണ്ടി (row = row_group->children; row; row = row->next) {
			പൂർണ്ണസംഖ്യ row_height = 0;

			htype = css_computed_height(row->style, &മൂല്യം, &unit);
			എങ്കിൽ (htype == CSS_HEIGHT_SET && unit != CSS_UNIT_PCT) {
				row_height = FIXTOINT(css_unit_len2device_px(
						row->style,
						&content->unit_len_ctx,
						മൂല്യം, unit));
			}
			വേണ്ടി (c = row->children; c; c = c->next) {
				പ്രതിജ്ഞാ(c->style);
				c->width = xs[c->start_column + c->columns] -
						xs[c->start_column] -
						border_spacing_h -
						c->border[ഇടത്_2].width -
						c->padding[ഇടത്_2] -
						c->padding[വലത്_2] -
						c->border[വലത്_2].width;
				c->float_children = 0;
				c->cached_place_below_level = 0;

				c->height = AUTO;
				എങ്കിൽ (!layout_block_context(c, -1, content)) {
					സ്വതന്ത്രം(col);
					സ്വതന്ത്രം(excess_y);
					സ്വതന്ത്രം(row_span);
					സ്വതന്ത്രം(row_span_cell);
					സ്വതന്ത്രം(xs);
					തിരികെ false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->height;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_height(c->style,
						&മൂല്യം, &unit);

				എങ്കിൽ (htype == CSS_HEIGHT_SET &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					പൂർണ്ണസംഖ്യ h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							മൂല്യം, unit));
					എങ്കിൽ (c->height < h)
						c->height = h;
				}
				/* specified row height is treated as a minimum
				 */
				എങ്കിൽ (c->height < row_height)
					c->height = row_height;
				c->x = xs[c->start_column] +
						c->border[ഇടത്_2].width;
				c->y = c->border[മുകൾ_2].width;
				വേണ്ടി (i = 0; i != c->columns; i++) {
					row_span[c->start_column + i] = c->rows;
					excess_y[c->start_column + i] =
							c->border[മുകൾ_2].width +
							c->padding[മുകൾ_2] +
							c->height +
							c->padding[BOTTOM] +
							c->border[BOTTOM].width;
					row_span_cell[c->start_column + i] = 0;
				}
				row_span_cell[c->start_column] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[മുകൾ_2].width -
						c->padding[മുകൾ_2] -
						c->height -
						c->border[BOTTOM].width;
			}
			വേണ്ടി (i = 0; i != columns; i++)
				എങ്കിൽ (row_span[i] != 0)
					row_span[i]--;
				അല്ലെങ്കിൽ
					row_span_cell[i] = 0;
			എങ്കിൽ (row->next || row_group->next) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				വേണ്ടി (i = 0; i != columns; i++)
					എങ്കിൽ (row_span[i] == 0 && row_height <
							excess_y[i])
						row_height = excess_y[i];
			} അല്ലെങ്കിൽ {
				/* except in the last row */
				വേണ്ടി (i = 0; i != columns; i++)
					എങ്കിൽ (row_height < excess_y[i])
						row_height = excess_y[i];
			}
			വേണ്ടി (i = 0; i != columns; i++) {
				എങ്കിൽ (row_height < excess_y[i])
					excess_y[i] -= row_height;
				അല്ലെങ്കിൽ
					excess_y[i] = 0;
				എങ്കിൽ (row_span_cell[i] != 0)
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
	വേണ്ടി (row_group = table->children; row_group;
			row_group = row_group->next) {
		വേണ്ടി (row = row_group->children; row; row = row->next) {
			വേണ്ടി (c = row->children; c; c = c->next) {
				എണ്ണൽ css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				spare_height = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->height - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &മൂല്യം, &unit);

				തിരഞ്ഞെടുപ്പ് (vertical_align) {
				അവസ്ഥ CSS_VERTICAL_ALIGN_SUB:
				അവസ്ഥ CSS_VERTICAL_ALIGN_SUPER:
				അവസ്ഥ CSS_VERTICAL_ALIGN_TEXT_മുകൾ:
				അവസ്ഥ CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				അവസ്ഥ CSS_VERTICAL_ALIGN_SET:
				അവസ്ഥ CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				അവസ്ഥ CSS_VERTICAL_ALIGN_മുകൾ:
					നിർത്തുക;
				അവസ്ഥ CSS_VERTICAL_ALIGN_നടുവ്:
					c->padding[മുകൾ_2] += spare_height / 2;
					c->padding[BOTTOM] -= spare_height / 2;
					layout_move_children(c, 0,
							spare_height / 2);
					നിർത്തുക;
				അവസ്ഥ CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[മുകൾ_2] += spare_height;
					c->padding[BOTTOM] -= spare_height;
					layout_move_children(c, 0,
							spare_height);
					നിർത്തുക;
				അവസ്ഥ CSS_VERTICAL_ALIGN_INHERIT:
					പ്രതിജ്ഞാ(0);
					നിർത്തുക;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	എങ്കിൽ (table->margin[മുകൾ_2] == AUTO)
		table->margin[മുകൾ_2] = 0;
	എങ്കിൽ (table->margin[BOTTOM] == AUTO)
		table->margin[BOTTOM] = 0;

	സ്വതന്ത്രം(col);
	സ്വതന്ത്രം(excess_y);
	സ്വതന്ത്രം(row_span);
	സ്വതന്ത്രം(row_span_cell);
	സ്വതന്ത്രം(xs);

	table->width = table_width;
	table->height = table_height;

	തിരികെ true;
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
സ്ഥിതം bool layout_apply_minmax_height(
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		ഘടന box *box,
		ഘടന box *container)
{
	പൂർണ്ണസംഖ്യ h;
	ഘടന box *containing_block = ശൂന്യ;
	bool updated = false;

	/* Find containing block for percentage heights */
	എങ്കിൽ (box->style != ശൂന്യ && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		പ്രതിജ്ഞാ(container);
		containing_block = container;
	} അല്ലെങ്കിൽ എങ്കിൽ (box->float_container && box->style != ശൂന്യ &&
			(css_computed_float(box->style) == CSS_FLOAT_ഇടത് ||
			 css_computed_float(box->style) == CSS_FLOAT_വലത്)) {
		/* Box is a float */
		പ്രതിജ്ഞാ(box->parent && box->parent->parent &&
				box->parent->parent->parent);
		containing_block = box->parent->parent->parent;
	} അല്ലെങ്കിൽ എങ്കിൽ (box->parent && box->parent->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_block = box->parent;
	} അല്ലെങ്കിൽ എങ്കിൽ (box->parent && box->parent->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		പ്രതിജ്ഞാ(box->parent->parent);
		containing_block = box->parent->parent;
	}

	എങ്കിൽ (box->style) {
		എണ്ണൽ css_height_e htype = CSS_HEIGHT_AUTO;
		css_fixed മൂല്യം = 0;
		css_unit unit = CSS_UNIT_PX;

		എങ്കിൽ (containing_block) {
			htype = css_computed_height(containing_block->style,
					&മൂല്യം, &unit);
		}

		/* max-height */
		എങ്കിൽ (css_computed_max_height(box->style, &മൂല്യം, &unit) ==
				CSS_MAX_HEIGHT_SET) {
			എങ്കിൽ (unit == CSS_UNIT_PCT) {
				എങ്കിൽ (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(മൂല്യം,
						containing_block->height);
					എങ്കിൽ (h < box->height) {
						box->height = h;
						updated = true;
					}
				}
			} അല്ലെങ്കിൽ {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						മൂല്യം, unit));
				എങ്കിൽ (h < box->height) {
					box->height = h;
					updated = true;
				}
			}
		}

		/* min-height */
		എങ്കിൽ (ns_computed_min_height(box->style, &മൂല്യം, &unit) ==
				CSS_MIN_HEIGHT_SET) {
			എങ്കിൽ (unit == CSS_UNIT_PCT) {
				എങ്കിൽ (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(മൂല്യം,
						containing_block->height);
					എങ്കിൽ (h > box->height) {
						box->height = h;
						updated = true;
					}
				}
			} അല്ലെങ്കിൽ {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						മൂല്യം, unit));
				എങ്കിൽ (h > box->height) {
					box->height = h;
					updated = true;
				}
			}
		}
	}
	തിരികെ updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
സ്ഥിതം bool layout_block_object(ഘടന box *block)
{
	പ്രതിജ്ഞാ(block);
	പ്രതിജ്ഞാ(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_TABLE ||
			block->type == BOX_TABLE_CELL);
	പ്രതിജ്ഞാ(block->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", block,
	      hlcache_handle_get_url(block->object), block->width);

	എങ്കിൽ (content_can_reformat(block->object)) {
		content_reformat(block->object, false, block->width, 1);
	} അല്ലെങ്കിൽ {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	തിരികെ true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
സ്ഥിതം ശൂന്യം add_float_to_container(ഘടന box *cont, ഘടന box *b)
{
	ഘടന box *box = cont->float_children;
	പൂർണ്ണസംഖ്യ b_bottom = b->y + b->height;

	പ്രതിജ്ഞാ(b->type == BOX_FLOAT_ഇടത് || b->type == BOX_FLOAT_വലത്);

	എങ്കിൽ (box == ശൂന്യ) {
		/* No other float children */
		b->next_float = ശൂന്യ;
		cont->float_children = b;
		തിരികെ;
	} അല്ലെങ്കിൽ എങ്കിൽ (b_bottom >= box->y + box->height) {
		/* Goes at start of list */
		b->next_float = cont->float_children;
		cont->float_children = b;
	} അല്ലെങ്കിൽ {
		ഘടന box *prev = ശൂന്യ;
		ആകുമ്പോൾ (box != ശൂന്യ && b_bottom < box->y + box->height) {
			prev = box;
			box = box->next_float;
		}
		എങ്കിൽ (prev != ശൂന്യ) {
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
സ്ഥിതം bool
layout_text_box_split(html_content *content,
		      plot_font_style_t *fstyle,
		      ഘടന box *split_box,
		      size_t new_length,
		      പൂർണ്ണസംഖ്യ new_width)
{
	പൂർണ്ണസംഖ്യ space_width = split_box->space;
	ഘടന box *c2;
	സ്ഥിരം ഘടന gui_layout_table *font_func = content->font_func;
	bool space = (split_box->text[new_length] == ' ');
	പൂർണ്ണസംഖ്യ used_length = new_length + (space ? 1 : 0);

	എങ്കിൽ ((space && space_width == 0) || space_width == UNKNOWN_WIDTH) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->width(fstyle, " ", 1, &space_width);
	}

	എങ്കിൽ (split_box->space == UNKNOWN_WIDTH)
		split_box->space = space_width;
	എങ്കിൽ (!space)
		space_width = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, split_box, വലുപ്പം *c2);
	എങ്കിൽ (!c2)
		തിരികെ false;
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
	എങ്കിൽ (c2->next)
		c2->next->prev = c2;
	അല്ലെങ്കിൽ
		c2->parent->last = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      split_box,
	      split_box->length,
	      (പൂർണ്ണസംഖ്യ)split_box->length,
	      split_box->text);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->length,
	      (പൂർണ്ണസംഖ്യ)c2->length,
	      c2->text);

	തിരികെ true;
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
സ്ഥിതം ശൂന്യം
layout_float_find_dimensions(
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		പൂർണ്ണസംഖ്യ available_width,
		സ്ഥിരം css_computed_style *style,
		ഘടന box *box)
{
	പൂർണ്ണസംഖ്യ width, height, max_width, min_width, max_height, min_height;
	പൂർണ്ണസംഖ്യ *margin = box->margin;
	പൂർണ്ണസംഖ്യ *padding = box->padding;
	ഘടന box_border *border = box->border;
	എണ്ണൽ css_overflow_e overflow_x = css_computed_overflow_x(style);
	എണ്ണൽ css_overflow_e overflow_y = css_computed_overflow_y(style);
	പൂർണ്ണസംഖ്യ scrollbar_width_x =
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;
	പൂർണ്ണസംഖ്യ scrollbar_width_y =
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;

	layout_find_dimensions(unit_len_ctx, available_width, -1, box, style,
			&width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	എങ്കിൽ (margin[ഇടത്_2] == AUTO)
		margin[ഇടത്_2] = 0;
	എങ്കിൽ (margin[വലത്_2] == AUTO)
		margin[വലത്_2] = 0;

	എങ്കിൽ (box->gadget == ശൂന്യ) {
		padding[വലത്_2] += scrollbar_width_y;
		padding[BOTTOM] += scrollbar_width_x;
	}

	എങ്കിൽ (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	} അല്ലെങ്കിൽ എങ്കിൽ (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed size = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		പ്രതിജ്ഞാ(box->style);

		എങ്കിൽ (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			എങ്കിൽ (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			എങ്കിൽ (box->gadget->type == GADGET_FILE &&
					height == AUTO) {
				size = FLTTOFIX(1.5);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
		എങ്കിൽ (box->gadget->type == GADGET_TEXTAREA) {
			എങ്കിൽ (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			എങ്കിൽ (height == AUTO) {
				size = INTTOFIX(4);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (width == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		width = min(max(box->min_width, available_width),
				box->max_width);

		/* width includes margin, borders and padding */
		എങ്കിൽ (width == available_width) {
			width -= box->margin[ഇടത്_2] + box->border[ഇടത്_2].width +
					box->padding[ഇടത്_2] +
					box->padding[വലത്_2] +
					box->border[വലത്_2].width +
					box->margin[വലത്_2];
		} അല്ലെങ്കിൽ {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			പൂർണ്ണസംഖ്യ fixed = 0;
			ദശാംശം frac = 0;
			calculate_mbp_width(unit_len_ctx, box->style, ഇടത്_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_width(unit_len_ctx, box->style, വലത്_2,
					true, true, true, &fixed, &frac);
			എങ്കിൽ (fixed < 0)
				fixed = 0;

			width -= fixed;
		}

		എങ്കിൽ (max_width >= 0 && width > max_width) width = max_width;
		എങ്കിൽ (min_width >  0 && width < min_width) width = min_width;

	} അല്ലെങ്കിൽ {
		എങ്കിൽ (max_width >= 0 && width > max_width) width = max_width;
		എങ്കിൽ (min_width >  0 && width < min_width) width = min_width;
		width -= scrollbar_width_y;
	}

	box->width = width;
	box->height = height;

	എങ്കിൽ (margin[മുകൾ_2] == AUTO)
		margin[മുകൾ_2] = 0;
	എങ്കിൽ (margin[BOTTOM] == AUTO)
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
സ്ഥിതം bool layout_float(ഘടന box *b, പൂർണ്ണസംഖ്യ width, html_content *content)
{
	പ്രതിജ്ഞാ(b->type == BOX_TABLE ||
	       b->type == BOX_BLOCK ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_find_dimensions(&content->unit_len_ctx, width, b->style, b);
	എങ്കിൽ (b->type == BOX_TABLE || b->type == BOX_INLINE_FLEX) {
		എങ്കിൽ (b->type == BOX_TABLE) {
			എങ്കിൽ (!layout_table(b, width, content))
				തിരികെ false;
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (!layout_flex(b, width, content))
				തിരികെ false;
		}
		എങ്കിൽ (b->margin[ഇടത്_2] == AUTO)
			b->margin[ഇടത്_2] = 0;
		എങ്കിൽ (b->margin[വലത്_2] == AUTO)
			b->margin[വലത്_2] = 0;
		എങ്കിൽ (b->margin[മുകൾ_2] == AUTO)
			b->margin[മുകൾ_2] = 0;
		എങ്കിൽ (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} അല്ലെങ്കിൽ {
		തിരികെ layout_block_context(b, -1, content);
	}
	തിരികെ true;
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
സ്ഥിതം ശൂന്യം
place_float_below(ഘടന box *c, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ cx, പൂർണ്ണസംഖ്യ y, ഘടന box *cont)
{
	പൂർണ്ണസംഖ്യ x0, x1, yy;
	ഘടന box *ഇടത്;
	ഘടന box *വലത്;

	yy = y > cont->cached_place_below_level ?
			y : cont->cached_place_below_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      width, cx, y, cont);

	ചെയ്യുക {
		y = yy;
		x0 = cx;
		x1 = cx + width;
		find_sides(cont->float_children, y, y + c->height, &x0, &x1,
				&ഇടത്, &വലത്);
		എങ്കിൽ (ഇടത് != 0 && വലത് != 0) {
			yy = (ഇടത്->y + ഇടത്->height <
					വലത്->y + വലത്->height ?
					ഇടത്->y + ഇടത്->height :
					വലത്->y + വലത്->height);
		} അല്ലെങ്കിൽ എങ്കിൽ (ഇടത് == 0 && വലത് != 0) {
			yy = വലത്->y + വലത്->height;
		} അല്ലെങ്കിൽ എങ്കിൽ (ഇടത് != 0 && വലത് == 0) {
			yy = ഇടത്->y + ഇടത്->height;
		}
	} ആകുമ്പോൾ ((ഇടത് != 0 || വലത് != 0) && (c->width > x1 - x0));

	എങ്കിൽ (c->type == BOX_FLOAT_ഇടത്) {
		c->x = x0;
	} അല്ലെങ്കിൽ {
		c->x = x1 - c->width;
	}
	c->y = y;
	cont->cached_place_below_level = y;
}


/**
 * Calculate line height from a style.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ വരി_height(
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		സ്ഥിരം css_computed_style *style)
{
	എണ്ണൽ css_വരി_height_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed വരി_height;

	പ്രതിജ്ഞാ(style);

	lhtype = css_computed_വരി_height(style, &lhvalue, &lhunit);
	എങ്കിൽ (lhtype == CSS_വരി_HEIGHT_NORMAL) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_വരി_HEIGHT_സംഖ്യ;
	}

	എങ്കിൽ (lhtype == CSS_വരി_HEIGHT_സംഖ്യ ||
			lhunit == CSS_UNIT_PCT) {
		വരി_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		എങ്കിൽ (lhtype != CSS_വരി_HEIGHT_സംഖ്യ)
			വരി_height = FDIV(വരി_height, F_100);
	} അല്ലെങ്കിൽ {
		പ്രതിജ്ഞാ(lhunit != CSS_UNIT_PCT);

		വരി_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	തിരികെ FIXTOINT(വരി_height);
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
സ്ഥിതം bool
layout_വരി(ഘടന box *first,
	    പൂർണ്ണസംഖ്യ *width,
	    പൂർണ്ണസംഖ്യ *y,
	    പൂർണ്ണസംഖ്യ cx,
	    പൂർണ്ണസംഖ്യ cy,
	    ഘടന box *cont,
	    bool indent,
	    bool has_text_children,
	    html_content *content,
	    ഘടന box **next_box)
{
	പൂർണ്ണസംഖ്യ height, used_height;
	പൂർണ്ണസംഖ്യ x0 = 0;
	പൂർണ്ണസംഖ്യ x1 = *width;
	പൂർണ്ണസംഖ്യ x, h, x_previous;
	പൂർണ്ണസംഖ്യ fy = cy;
	ഘടന box *ഇടത്;
	ഘടന box *വലത്;
	ഘടന box *b;
	ഘടന box *split_box = 0;
	ഘടന box *d;
	ഘടന box *br_box = 0;
	bool move_y = false;
	bool place_below = false;
	പൂർണ്ണസംഖ്യ space_before = 0, space_after = 0;
	unsigned പൂർണ്ണസംഖ്യ inline_എണ്ണം = 0;
	unsigned പൂർണ്ണസംഖ്യ i;
	സ്ഥിരം ഘടന gui_layout_table *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      first,
	      (പൂർണ്ണസംഖ്യ)first->length,
	      first->text,
	      *width,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	find_sides(cont->float_children, cy, cy, &x0, &x1, &ഇടത്, &വലത്);
	x0 -= cx;
	x1 -= cx;

	എങ്കിൽ (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	എങ്കിൽ (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	എങ്കിൽ (has_text_children || first->parent->parent->gadget)
		used_height = height = വരി_height(&content->unit_len_ctx,
				first->parent->parent->style);
	അല്ലെങ്കിൽ
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		used_height = height = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	വേണ്ടി (x = 0, b = first; x <= x1 - x0 && b != 0; b = b->next) {
		പൂർണ്ണസംഖ്യ min_width, max_width, min_height, max_height;

		പ്രതിജ്ഞാ(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		എങ്കിൽ (b->type == BOX_BR)
			നിർത്തുക;

		എങ്കിൽ (lh__box_is_float_box(b))
			തുടരുക;
		എങ്കിൽ (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			തുടരുക;

		പ്രതിജ്ഞാ(b->style != ശൂന്യ);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		എങ്കിൽ (b->type == BOX_INLINE_BLOCK ||
		    b->type == BOX_INLINE_FLEX) {
			എങ്കിൽ (b->max_width != UNKNOWN_WIDTH)
				എങ്കിൽ (!layout_float(b, *width, content))
					തിരികെ false;
			h = b->border[മുകൾ_2].width + b->padding[മുകൾ_2] + b->height +
					b->padding[BOTTOM] +
					b->border[BOTTOM].width;
			എങ്കിൽ (height < h)
				height = h;
			x += b->margin[ഇടത്_2] + b->border[ഇടത്_2].width +
					b->padding[ഇടത്_2] + b->width +
					b->padding[വലത്_2] +
					b->border[വലത്_2].width +
					b->margin[വലത്_2];
			space_after = 0;
			തുടരുക;
		}

		എങ്കിൽ (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_find_dimensions(&content->unit_len_ctx,
					*width, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			വേണ്ടി (i = 0; i != 4; i++)
				എങ്കിൽ (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[ഇടത്_2] + b->border[ഇടത്_2].width +
					b->padding[ഇടത്_2];
			എങ്കിൽ (b->inline_end) {
				b->inline_end->margin[വലത്_2] = b->margin[വലത്_2];
				b->inline_end->padding[വലത്_2] =
						b->padding[വലത്_2];
				b->inline_end->border[വലത്_2] =
						b->border[വലത്_2];
			} അല്ലെങ്കിൽ {
				x += b->padding[വലത്_2] +
						b->border[വലത്_2].width +
						b->margin[വലത്_2];
			}
		} അല്ലെങ്കിൽ എങ്കിൽ (b->type == BOX_INLINE_END) {
			b->width = 0;
			എങ്കിൽ (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[വലത്_2] + b->border[വലത്_2].width +
					b->margin[വലത്_2];
			തുടരുക;
		}

		എങ്കിൽ (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->height = വരി_height(&content->unit_len_ctx,
					b->style ? b->style :
					b->parent->parent->style);
			എങ്കിൽ (height < b->height)
				height = b->height;

			എങ്കിൽ (!b->text) {
				b->width = 0;
				space_after = 0;
				തുടരുക;
			}

			എങ്കിൽ (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				എങ്കിൽ (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					പൂർണ്ണസംഖ്യ opt_maxwidth = 0;
					ഘടന form_option *o;

					വേണ്ടി (o = b->parent->parent->gadget->
							data.select.ഇനങ്ങൾ; o;
							o = o->next) {
						പൂർണ്ണസംഖ്യ opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						എങ്കിൽ (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}
					b->width = opt_maxwidth;
					എങ്കിൽ (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;
				} അല്ലെങ്കിൽ {
					font_func->width(&fstyle, b->text,
							b->length, &b->width);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			എങ്കിൽ (b->text && (x + b->width < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->next) {
				font_func->width(&fstyle, b->text,
						 b->length, &b->width);
				b->flags |= MEASURED;
			}

			x += b->width;
			എങ്കിൽ (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			തുടരുക;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		പ്രതിജ്ഞാ(b->style);

		layout_find_dimensions(&content->unit_len_ctx,
				*width, -1, b, b->style,
				&b->width, &b->height,
				&max_width, &min_width,
				&max_height, &min_height,
				ശൂന്യ, ശൂന്യ, ശൂന്യ);

		എങ്കിൽ (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->width, &b->height,
					min_width, max_width,
					min_height, max_height);
		} അല്ലെങ്കിൽ എങ്കിൽ (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			എങ്കിൽ (b->width == AUTO)
				b->width = 400;
			എങ്കിൽ (b->height == AUTO)
				b->height = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} അല്ലെങ്കിൽ {
			/* form control with no object */
			എങ്കിൽ (b->width == AUTO)
				b->width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			എങ്കിൽ (b->height == AUTO)
				b->height = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		എങ്കിൽ (b->object && content_can_reformat(b->object) &&
				b->width !=
				content_get_available_width(b->object)) {
			css_fixed മൂല്യം = 0;
			css_unit unit = CSS_UNIT_PX;
			എണ്ണൽ css_height_e htype = css_computed_height(b->style,
					&മൂല്യം, &unit);

			content_reformat(b->object, false, b->width, b->height);

			എങ്കിൽ (htype == CSS_HEIGHT_AUTO)
				b->height = content_get_height(b->object);
		}

		എങ്കിൽ (height < b->height)
			height = b->height;

		x += b->width;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *width;
	find_sides(cont->float_children, cy, cy + height, &x0, &x1,
			&ഇടത്, &വലത്);
	x0 -= cx;
	x1 -= cx;

	എങ്കിൽ (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	എങ്കിൽ (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	വേണ്ടി (x = x_previous = 0, b = first; x <= x1 - x0 && b; b = b->next) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		എങ്കിൽ (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} അല്ലെങ്കിൽ എങ്കിൽ (lh__box_is_inline_flow(b)) {
			പ്രതിജ്ഞാ(b->width != UNKNOWN_WIDTH);

			x_previous = x;
			x += space_after;
			b->x = x;

			എങ്കിൽ ((b->type == BOX_INLINE && !b->inline_end) ||
					b->type == BOX_INLINE_BLOCK ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[ഇടത്_2] + b->border[ഇടത്_2].width;
				x = b->x + b->padding[ഇടത്_2] + b->width +
						b->padding[വലത്_2] +
						b->border[വലത്_2].width +
						b->margin[വലത്_2];
			} അല്ലെങ്കിൽ എങ്കിൽ (b->type == BOX_INLINE) {
				b->x += b->margin[ഇടത്_2] + b->border[ഇടത്_2].width;
				x = b->x + b->padding[ഇടത്_2] + b->width;
			} അല്ലെങ്കിൽ എങ്കിൽ (b->type == BOX_INLINE_END) {
				b->height = b->inline_end->height;
				x += b->padding[വലത്_2] +
						b->border[വലത്_2].width +
						b->margin[വലത്_2];
			} അല്ലെങ്കിൽ {
				x += b->width;
			}

			space_before = space_after;
			എങ്കിൽ (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			അല്ലെങ്കിൽ എങ്കിൽ (b->text || b->type == BOX_INLINE_END) {
				എങ്കിൽ (b->space == UNKNOWN_WIDTH) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} അല്ലെങ്കിൽ {
				space_after = 0;
			}
			split_box = b;
			move_y = true;
			inline_എണ്ണം++;
		} അല്ലെങ്കിൽ എങ്കിൽ (b->type == BOX_BR) {
			b->x = x;
			b->width = 0;
			br_box = b;
			b = b->next;
			split_box = 0;
			move_y = true;
			നിർത്തുക;

		} അല്ലെങ്കിൽ {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_place_below_level = 0;
			b->float_container = d->float_container = cont;

			എങ്കിൽ (!layout_float(d, *width, content))
				തിരികെ false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[മുകൾ_2],
			      d->border[മുകൾ_2].width);

			d->x = d->margin[ഇടത്_2] + d->border[ഇടത്_2].width;
			d->y = d->margin[മുകൾ_2] + d->border[മുകൾ_2].width;
			b->width = d->margin[ഇടത്_2] + d->border[ഇടത്_2].width +
					d->padding[ഇടത്_2] + d->width +
					d->padding[വലത്_2] +
					d->border[വലത്_2].width +
					d->margin[വലത്_2];
			b->height = d->margin[മുകൾ_2] + d->border[മുകൾ_2].width +
					d->padding[മുകൾ_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];

			എങ്കിൽ (b->width > (x1 - x0) - x)
				place_below = true;
			എങ്കിൽ (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ഇടത് && ഇടത് == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_വലത് &&
						വലത് == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						ഇടത് == 0 && വലത് == 0)) &&
					(!place_below ||
					(ഇടത് == 0 && വലത് == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_place_below_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				എങ്കിൽ (b->type == BOX_FLOAT_ഇടത്) {
					b->x = cx + x0;
					എങ്കിൽ (b->width > 0)
						x0 += b->width;
					ഇടത് = b;
				} അല്ലെങ്കിൽ {
					b->x = cx + x1 - b->width;
					എങ്കിൽ (b->width > 0)
						x1 -= b->width;
					വലത് = b;
				}
				b->y = cy;
			} അല്ലെങ്കിൽ {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				പൂർണ്ണസംഖ്യ fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_place_below_level) ?
						fcy :
						cont->cached_place_below_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + height : fy;

				place_float_below(b, *width, cx, fy, cont);
				fy = b->y;
				എങ്കിൽ (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ഇടത് && ഇടത് != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_വലത് &&
						വലത് != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(ഇടത് != 0 || വലത് != 0)))) {
					/* to be cleared below existing
					 * floats */
					എങ്കിൽ (b->type == BOX_FLOAT_ഇടത്)
						b->x = cx;
					അല്ലെങ്കിൽ
						b->x = cx + *width - b->width;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					എങ്കിൽ (fcy > cont->clear_level)
						cont->clear_level = fcy;
					എങ്കിൽ (b->y < fcy)
						b->y = fcy;
				}
				എങ്കിൽ (b->type == BOX_FLOAT_ഇടത്)
					ഇടത് = b;
				അല്ലെങ്കിൽ
					വലത് = b;
			}
			add_float_to_container(cont, b);

			split_box = 0;
		}
	}

	എങ്കിൽ (x1 - x0 < x && split_box) {
		/* the last box went over the end */
		size_t split = 0;
		പൂർണ്ണസംഖ്യ w;
		bool no_wrap = css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_previous;

		എങ്കിൽ (!no_wrap &&
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

		എങ്കിൽ (split == 0)
			w = split_box->width;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      split_box,
		      (പൂർണ്ണസംഖ്യ)split_box->length,
		      split_box->text,
		      split,
		      w,
		      ഇടത്,
		      വലത്,
		      inline_എണ്ണം);

		എങ്കിൽ ((split == 0 || x1 - x0 <= x + space_before + w) &&
				!ഇടത് && !വലത് && inline_എണ്ണം == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			എങ്കിൽ (split == 0 || split == split_box->length) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = split_box->next;
			} അല്ലെങ്കിൽ {
				/* cut off first word for this line */
				എങ്കിൽ (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					തിരികെ false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} അല്ലെങ്കിൽ എങ്കിൽ ((split == 0 || x1 - x0 <= x + space_before + w) &&
				inline_എണ്ണം == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			പ്രതിജ്ഞാ(ഇടത് || വലത്);
			used_height = 0;
			എങ്കിൽ (ഇടത്) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      ഇടത്->y,
				      ഇടത്->height);

				used_height = ഇടത്->y + ഇടത്->height - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      used_height);

			}
			എങ്കിൽ (വലത് && used_height <
					വലത്->y + വലത്->height - cy + 1)
				used_height = വലത്->y + വലത്->height - cy + 1;

			എങ്കിൽ (used_height < 0)
				used_height = 0;

			b = split_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} അല്ലെങ്കിൽ എങ്കിൽ (split == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = split_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} അല്ലെങ്കിൽ {
			/* fit as many words as possible */
			പ്രതിജ്ഞാ(split != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (പൂർണ്ണസംഖ്യ)split_box->length, split_box->text,
			      x1 - x0, split, w);

			എങ്കിൽ (split != split_box->length) {
				എങ്കിൽ (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					തിരികെ false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	തിരഞ്ഞെടുപ്പ് (css_computed_text_align(first->parent->parent->style)) {
	അവസ്ഥ CSS_TEXT_ALIGN_വലത്:
	അവസ്ഥ CSS_TEXT_ALIGN_LIBCSS_വലത്:
		x0 = x1 - x;
		നിർത്തുക;
	അവസ്ഥ CSS_TEXT_ALIGN_CENTER:
	അവസ്ഥ CSS_TEXT_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		നിർത്തുക;
	അവസ്ഥ CSS_TEXT_ALIGN_ഇടത്:
	അവസ്ഥ CSS_TEXT_ALIGN_LIBCSS_ഇടത്:
	അവസ്ഥ CSS_TEXT_ALIGN_JUSTIFY:
		/* leave on left */
		നിർത്തുക;
	അവസ്ഥ CSS_TEXT_ALIGN_DEFAULT:
		/* None; consider text direction */
		തിരഞ്ഞെടുപ്പ് (css_computed_direction(first->parent->parent->style)) {
		അവസ്ഥ CSS_DIRECTION_LTR:
			/* leave on left */
			നിർത്തുക;
		അവസ്ഥ CSS_DIRECTION_RTL:
			x0 = x1 - x;
			നിർത്തുക;
		}
		നിർത്തുക;
	}

	വേണ്ടി (d = first; d != b; d = d->next) {
		d->flags &= ~NEW_വരി;

		എങ്കിൽ (d->type == BOX_INLINE_BLOCK &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			തുടരുക;
		} അല്ലെങ്കിൽ എങ്കിൽ ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[മുകൾ_2];

			എങ്കിൽ (d->type == BOX_TEXT && d->height > used_height) {
				/* text */
				used_height = d->height;
			}
		} അല്ലെങ്കിൽ എങ്കിൽ ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_BLOCK) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[മുകൾ_2].width + d->margin[മുകൾ_2];
			h = d->margin[മുകൾ_2] + d->border[മുകൾ_2].width +
					d->padding[മുകൾ_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];
			എങ്കിൽ (used_height < h)
				used_height = h;
		}
	}

	first->flags |= NEW_വരി;

	പ്രതിജ്ഞാ(b != first || (move_y && 0 < used_height && (ഇടത് || വലത്)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	വേണ്ടി (d = first; d != b; d = d->next) {
		എങ്കിൽ ((d->type == BOX_INLINE && d->inline_end) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			css_fixed മൂല്യം = 0;
			css_unit unit = CSS_UNIT_PX;
			തിരഞ്ഞെടുപ്പ് (css_computed_vertical_align(d->style, &മൂല്യം,
					&unit)) {
			അവസ്ഥ CSS_VERTICAL_ALIGN_SUPER:
			അവസ്ഥ CSS_VERTICAL_ALIGN_മുകൾ:
			അവസ്ഥ CSS_VERTICAL_ALIGN_TEXT_മുകൾ:
				/* already at top */
				നിർത്തുക;
			അവസ്ഥ CSS_VERTICAL_ALIGN_SUB:
			അവസ്ഥ CSS_VERTICAL_ALIGN_BOTTOM:
			അവസ്ഥ CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				d->y += used_height - d->height;
				നിർത്തുക;
			സ്ഥിരസ്ഥിതി:
			അവസ്ഥ CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (used_height - d->height);
				നിർത്തുക;
			}
		}
	}

	/* handle clearance for br */
	എങ്കിൽ (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		പൂർണ്ണസംഖ്യ clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		എങ്കിൽ (used_height < clear_y - cy)
			used_height = clear_y - cy;
	}

	എങ്കിൽ (move_y)
		*y += used_height;
	*next_box = b;
	*width = x; /* return actual width */
	തിരികെ true;
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
സ്ഥിതം bool layout_inline_container(ഘടന box *inline_container, പൂർണ്ണസംഖ്യ width,
		ഘടന box *cont, പൂർണ്ണസംഖ്യ cx, പൂർണ്ണസംഖ്യ cy, html_content *content)
{
	bool first_വരി = true;
	bool has_text_children;
	ഘടന box *c, *next;
	പൂർണ്ണസംഖ്യ y = 0;
	പൂർണ്ണസംഖ്യ curwidth,maxwidth = width;

	പ്രതിജ്ഞാ(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      width,
	      cont,
	      cx,
	      cy);


	has_text_children = false;
	വേണ്ടി (c = inline_container->children; c; c = c->next) {
		bool is_pre = false;

		എങ്കിൽ (c->style) {
			എണ്ണൽ css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_വരി ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		എങ്കിൽ ((lh__box_is_object(c) == false &&
				c->text && (c->length || is_pre)) ||
				c->type == BOX_BR)
			has_text_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	വേണ്ടി (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->width;
		എങ്കിൽ (!layout_വരി(c, &curwidth, &y, cx, cy + y, cont, first_വരി,
				has_text_children, content, &next))
			തിരികെ false;
		maxwidth = max(maxwidth,curwidth);
		c = next;
		first_വരി = false;
	}

	inline_container->width = maxwidth;
	inline_container->height = y;

	തിരികെ true;
}


/* Documented in layout_intertnal.h */
bool layout_block_context(
		ഘടന box *block,
		പൂർണ്ണസംഖ്യ viewport_height,
		html_content *content)
{
	ഘടന box *box;
	പൂർണ്ണസംഖ്യ cx, cy;  /**< current coordinates */
	പൂർണ്ണസംഖ്യ max_pos_margin = 0;
	പൂർണ്ണസംഖ്യ max_neg_margin = 0;
	പൂർണ്ണസംഖ്യ y = 0;
	പൂർണ്ണസംഖ്യ lm, rm;
	ഘടന box *margin_collapse = ശൂന്യ;
	bool in_margin = false;
	css_fixed gadget_size;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	പ്രതിജ്ഞാ(block->type == BOX_BLOCK ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX);
	പ്രതിജ്ഞാ(block->width != UNKNOWN_WIDTH);
	പ്രതിജ്ഞാ(block->width != AUTO);

	block->float_children = ശൂന്യ;
	block->cached_place_below_level = 0;
	block->clear_level = 0;

	/* special case if the block contains an object */
	എങ്കിൽ (block->object) {
		പൂർണ്ണസംഖ്യ temp_width = block->width;
		എങ്കിൽ (!layout_block_object(block))
			തിരികെ false;
		layout_get_object_dimensions(block, &temp_width,
				&block->height, INT_MIN, INT_MAX,
				INT_MIN, INT_MAX);
		തിരികെ true;
	} അല്ലെങ്കിൽ എങ്കിൽ (block->flags & REPLACE_DIM) {
		തിരികെ true;
	}

	/* special case if the block contains an radio button or checkbox */
	എങ്കിൽ (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_size = INTTOFIX(1);
		എങ്കിൽ (block->height == AUTO)
			block->height = FIXTOINT(css_unit_len2device_px(
					block->style,
					&content->unit_len_ctx,
					gadget_size, gadget_unit));
	}

	box = block->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = block->padding[മുകൾ_2];
	എങ്കിൽ (box)
		box->y = block->padding[മുകൾ_2];

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
	ആകുമ്പോൾ (box) {
		എണ്ണൽ css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
		എണ്ണൽ css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

		പ്രതിജ്ഞാ(box->type == BOX_BLOCK ||
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

		എങ്കിൽ (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->parent->padding[ഇടത്_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			പോകുക advance_to_next_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		എങ്കിൽ (margin_collapse == ശൂന്യ) {
			margin_collapse = layout_next_margin_block(
					&content->unit_len_ctx, box, block,
					viewport_height,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			in_margin = true;
		}

		/* Clearance. */
		y = 0;
		എങ്കിൽ (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(block->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		എങ്കിൽ (box->style) {
			overflow_x = css_computed_overflow_x(box->style);
			overflow_y = css_computed_overflow_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		എങ്കിൽ (box->type == BOX_FLEX ||
		    box->type == BOX_BLOCK ||
		    box->flags & IFRAME) {
			എങ്കിൽ (lh__box_is_object(box) == false &&
					box->style &&
					(overflow_x != CSS_OVERFLOW_VISIBLE ||
					 overflow_y != CSS_OVERFLOW_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				പൂർണ്ണസംഖ്യ x0, x1, മുകൾ;
				ഘടന box *ഇടത്, *വലത്;
				മുകൾ = cy + max_pos_margin - max_neg_margin;
				മുകൾ = (മുകൾ > y) ? മുകൾ : y;
				x0 = cx;
				x1 = cx + box->parent->width -
						box->parent->padding[ഇടത്_2] -
						box->parent->padding[വലത്_2];
				find_sides(block->float_children, മുകൾ, മുകൾ,
						&x0, &x1, &ഇടത്, &വലത്);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->parent->width -
						box->parent->padding[ഇടത്_2] -
						box->parent->padding[വലത്_2] -
						x1;
			}
			layout_block_find_dimensions(&content->unit_len_ctx,
					box->parent->width,
					viewport_height, lm, rm, box);
			എങ്കിൽ (box->type == BOX_BLOCK && !(box->flags & IFRAME)) {
				layout_block_add_scrollbar(box, വലത്_2);
				layout_block_add_scrollbar(box, BOTTOM);
			}
		} അല്ലെങ്കിൽ എങ്കിൽ (box->type == BOX_TABLE) {
			എങ്കിൽ (box->style != ശൂന്യ) {
				എണ്ണൽ css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style, &width,
						&unit);

				എങ്കിൽ (wtype == CSS_WIDTH_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					പൂർണ്ണസംഖ്യ x0, x1, മുകൾ;
					ഘടന box *ഇടത്, *വലത്;
					മുകൾ = cy + max_pos_margin -
							max_neg_margin;
					മുകൾ = (മുകൾ > y) ? മുകൾ : y;
					x0 = cx;
					x1 = cx + box->parent->width -
						box->parent->padding[ഇടത്_2] -
						box->parent->padding[വലത്_2];
					find_sides(block->float_children,
						മുകൾ, മുകൾ, &x0, &x1,
						&ഇടത്, &വലത്);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->parent->width -
						box->parent->padding[ഇടത്_2] -
						box->parent->padding[വലത്_2] -
						x1;
				}
			}
			എങ്കിൽ (!layout_table(box, box->parent->width - lm - rm,
					content))
				തിരികെ false;
			layout_solve_width(box, box->parent->width, box->width,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->parent->padding[ഇടത്_2] + box->margin[ഇടത്_2] +
				box->border[ഇടത്_2].width;
		cx += box->x;

		/* Position box: vertical. */
		എങ്കിൽ (box->border[മുകൾ_2].width) {
			box->y += box->border[മുകൾ_2].width;
			cy += box->border[മുകൾ_2].width;
		}

		/* Vertical margin */
		എങ്കിൽ (((box->type == BOX_BLOCK && (box->flags & HAS_HEIGHT)) ||
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
		എങ്കിൽ (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		എങ്കിൽ (box->type == BOX_FLEX ||
				(box->type == BOX_BLOCK && box->style &&
				 (overflow_x != CSS_OVERFLOW_VISIBLE ||
				  overflow_y != CSS_OVERFLOW_VISIBLE))) {

			എങ്കിൽ (box->type == BOX_FLEX) {
				എങ്കിൽ (!layout_flex(box, box->width, content)) {
					തിരികെ false;
				}
			} അല്ലെങ്കിൽ {
				layout_block_context(box,
						viewport_height, content);
			}

			cy += box->padding[മുകൾ_2];

			എങ്കിൽ (box->height == AUTO) {
				box->height = 0;
				layout_block_add_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->height + box->padding[BOTTOM] +
					box->border[BOTTOM].width;
			y = box->y + box->padding[മുകൾ_2] + box->height +
					box->padding[BOTTOM] +
					box->border[BOTTOM].width;

			/* Skip children, because they are done in the new
			 * block context */
			പോകുക advance_to_next_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->width);

		/* Layout (except tables). */
		എങ്കിൽ (box->object) {
			എങ്കിൽ (!layout_block_object(box))
				തിരികെ false;

		} അല്ലെങ്കിൽ എങ്കിൽ (box->type == BOX_INLINE_CONTAINER) {
			box->width = box->parent->width;
			എങ്കിൽ (!layout_inline_container(box, box->width, block,
					cx, cy, content))
				തിരികെ false;

		} അല്ലെങ്കിൽ എങ്കിൽ (box->type == BOX_TABLE) {
			/* Move down to avoid floats if necessary. */
			പൂർണ്ണസംഖ്യ x0, x1;
			ഘടന box *ഇടത്, *വലത്;
			y = cy;
			ആകുമ്പോൾ (1) {
				എണ്ണൽ css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style,
						&width, &unit);

				x0 = cx;
				x1 = cx + box->parent->width;
				find_sides(block->float_children, y,
						y + box->height,
						&x0, &x1, &ഇടത്, &വലത്);
				എങ്കിൽ (wtype == CSS_WIDTH_AUTO)
					നിർത്തുക;
				എങ്കിൽ (box->width <= x1 - x0)
					നിർത്തുക;
				എങ്കിൽ (!ഇടത് && !വലത്)
					നിർത്തുക;
				അല്ലെങ്കിൽ എങ്കിൽ (!ഇടത്)
					y = വലത്->y + വലത്->height + 1;
				അല്ലെങ്കിൽ എങ്കിൽ (!വലത്)
					y = ഇടത്->y + ഇടത്->height + 1;
				അല്ലെങ്കിൽ എങ്കിൽ (ഇടത്->y + ഇടത്->height <
						വലത്->y + വലത്->height)
					y = ഇടത്->y + ഇടത്->height + 1;
				അല്ലെങ്കിൽ
					y = വലത്->y + വലത്->height + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		എങ്കിൽ (box->type == BOX_BLOCK && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			എങ്കിൽ (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = ശൂന്യ;
			}

			y = box->padding[മുകൾ_2];
			box = box->children;
			box->y = y;
			cy += y;
			തുടരുക;
		} അല്ലെങ്കിൽ എങ്കിൽ (box->type == BOX_BLOCK || box->object ||
				box->flags & IFRAME)
			cy += box->padding[മുകൾ_2];

		എങ്കിൽ (box->type == BOX_BLOCK && box->height == AUTO) {
			box->height = 0;
			layout_block_add_scrollbar(box, BOTTOM);
		}

		cy += box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width;
		cx -= box->x;
		y = box->y + box->padding[മുകൾ_2] + box->height +
				box->padding[BOTTOM] +
				box->border[BOTTOM].width;

	advance_to_next_box:
		എങ്കിൽ (!box->next) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			ചെയ്യുക {
				എങ്കിൽ (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = ശൂന്യ;
				}

				/* Apply bottom margin */
				എങ്കിൽ (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				അല്ലെങ്കിൽ എങ്കിൽ (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->parent;
				എങ്കിൽ (box == block)
					നിർത്തുക;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				എങ്കിൽ (box->type == BOX_BLOCK &&
						box->flags & MAKE_HEIGHT) {
					margin_collapse = ശൂന്യ;
					in_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				എങ്കിൽ (box->height == AUTO) {
					box->height = y - box->padding[മുകൾ_2];

					എങ്കിൽ (box->type == BOX_BLOCK)
						layout_block_add_scrollbar(box,
								BOTTOM);
				} അല്ലെങ്കിൽ
					cy += box->height -
							(y - box->padding[മുകൾ_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				എങ്കിൽ (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_height(
							&content->unit_len_ctx,
							box, ശൂന്യ)) {
					/* Height altered */
					/* Set current cy */
					cy += box->height -
							(y - box->padding[മുകൾ_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].width;
				cx -= box->x;
				y = box->y + box->padding[മുകൾ_2] + box->height +
						box->padding[BOTTOM] +
						box->border[BOTTOM].width;

			} ആകുമ്പോൾ (box->next == ശൂന്യ);
			എങ്കിൽ (box == block)
				നിർത്തുക;
		}

		/* To next sibling. */

		എങ്കിൽ (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = ശൂന്യ;
		}

		എങ്കിൽ (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		അല്ലെങ്കിൽ എങ്കിൽ (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->next;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	വേണ്ടി (box = block->float_children; box; box = box->next_float) {
		y = box->y + box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width + box->margin[BOTTOM];
		എങ്കിൽ (cy < y)
			cy = y;
	}

	എങ്കിൽ (block->height == AUTO) {
		block->height = cy - block->padding[മുകൾ_2];
		എങ്കിൽ (block->type == BOX_BLOCK)
			layout_block_add_scrollbar(block, BOTTOM);
	}

	എങ്കിൽ (block->style && css_computed_position(block->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_height(&content->unit_len_ctx, block, ശൂന്യ);
	}

	എങ്കിൽ (block->gadget &&
			(block->gadget->type == GADGET_TEXTAREA ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		പൂർണ്ണസംഖ്യ ta_width = block->padding[ഇടത്_2] + block->width +
				block->padding[വലത്_2];
		പൂർണ്ണസംഖ്യ ta_height = block->padding[മുകൾ_2] + block->height +
				block->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				block->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_set_layout(block->gadget->data.text.ta,
				&fstyle, ta_width, ta_height,
				block->padding[മുകൾ_2], block->padding[വലത്_2],
				block->padding[BOTTOM], block->padding[ഇടത്_2]);
	}

	തിരികെ true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
സ്ഥിതം bool
layout__get_element_tag(
		സ്ഥിരം dom_കണ്ണി *കണ്ണി,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_കണ്ണി_type കണ്ണി_type;
	dom_exception exc;

	exc = dom_കണ്ണി_get_കണ്ണി_type(കണ്ണി, &കണ്ണി_type);
	എങ്കിൽ (exc != DOM_NO_ERR ||
	    കണ്ണി_type != DOM_ELEMENT_കണ്ണി) {
		തിരികെ false;
	}

	exc = dom_html_element_get_tag_type(കണ്ണി, &element_type);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ false;
	}

	*type = element_type;
	തിരികെ true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
സ്ഥിതം വരിയിൽ bool
layout__check_element_type(
		സ്ഥിരം dom_കണ്ണി *കണ്ണി,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	എങ്കിൽ (!layout__get_element_tag(കണ്ണി, &element_type)) {
		തിരികെ false;
	}

	തിരികെ element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
സ്ഥിതം bool
layout_get_li_മൂല്യം(dom_കണ്ണി *li_കണ്ണി, dom_long *മൂല്യം_out)
{
	dom_exception exc;
	dom_long മൂല്യം;
	bool has_മൂല്യം;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_കണ്ണി,
			corestring_dom_മൂല്യം,
			&has_മൂല്യം);
	എങ്കിൽ (exc != DOM_NO_ERR || has_മൂല്യം == false) {
		തിരികെ false;
	}

	exc = dom_html_li_element_get_മൂല്യം(
			(dom_html_li_element *)li_കണ്ണി,
			&മൂല്യം);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ false;
	}

	*മൂല്യം_out = മൂല്യം;
	തിരികെ true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
സ്ഥിതം bool
layout__get_ol_start(dom_കണ്ണി *ol_കണ്ണി, dom_long *start_out)
{
	dom_exception exc;
	dom_long start;
	bool has_start;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_കണ്ണി,
			corestring_dom_start,
			&has_start);
	എങ്കിൽ (exc != DOM_NO_ERR || has_start == false) {
		തിരികെ false;
	}

	exc = dom_html_olist_element_get_start(
			(dom_html_olist_element *)ol_കണ്ണി,
			&start);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ false;
	}

	*start_out = start;
	തിരികെ true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
സ്ഥിതം bool
layout__get_ol_reversed(dom_കണ്ണി *ol_കണ്ണി)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_കണ്ണി,
			corestring_dom_reversed,
			&has_reversed);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ false;
	}

	തിരികെ has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
സ്ഥിതം bool
layout_get_list_item_എണ്ണം(
		dom_കണ്ണി *list_owner, dom_long *എണ്ണം_out)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_കണ്ണി *child;
	പൂർണ്ണസംഖ്യ എണ്ണം;

	എങ്കിൽ (list_owner == ശൂന്യ) {
		തിരികെ false;
	}

	എങ്കിൽ (!layout__get_element_tag(list_owner, &tag_type)) {
		തിരികെ false;
	}

	എങ്കിൽ (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		തിരികെ false;
	}

	exc = dom_കണ്ണി_get_first_child(list_owner, &child);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ false;
	}

	എണ്ണം = 0;
	ആകുമ്പോൾ (child != ശൂന്യ) {
		dom_കണ്ണി *temp_കണ്ണി;

		എങ്കിൽ (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ഘടന box *child_box;
			എങ്കിൽ (dom_കണ്ണി_get_user_data(child,
					corestring_dom_ns_താക്കോൽ_box_കണ്ണി_data,
					&child_box) != DOM_NO_ERR) {
				dom_കണ്ണി_unref(child);
				തിരികെ false;
			}

			എങ്കിൽ (child_box != ശൂന്യ &&
			    child_box->list_marker != ശൂന്യ) {
				എണ്ണം++;
			}
		}

		exc = dom_കണ്ണി_get_next_sibling(child, &temp_കണ്ണി);
		dom_കണ്ണി_unref(child);
		എങ്കിൽ (exc != DOM_NO_ERR) {
			തിരികെ false;
		}

		child = temp_കണ്ണി;
	}

	*എണ്ണം_out = എണ്ണം;
	തിരികെ true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
സ്ഥിതം ശൂന്യം
layout_ordered_list_എണ്ണം(
		ഘടന box *box)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_കണ്ണി *child;
	പൂർണ്ണസംഖ്യ step = 1;
	dom_long next;

	എങ്കിൽ (box->കണ്ണി == ശൂന്യ) {
		തിരികെ;
	}

	എങ്കിൽ (!layout__get_element_tag(box->കണ്ണി, &tag_type)) {
		തിരികെ;
	}

	എങ്കിൽ (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		തിരികെ;
	}

	next = 1;
	എങ്കിൽ (tag_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_start = layout__get_ol_start(box->കണ്ണി, &next);
		bool have_reversed = layout__get_ol_reversed(box->കണ്ണി);

		എങ്കിൽ (have_reversed) {
			step = -1;
		}

		എങ്കിൽ (!have_start && have_reversed) {
			layout_get_list_item_എണ്ണം(box->കണ്ണി, &next);
		}
	}

	exc = dom_കണ്ണി_get_first_child(box->കണ്ണി, &child);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ;
	}

	ആകുമ്പോൾ (child != ശൂന്യ) {
		dom_കണ്ണി *temp_കണ്ണി;

		എങ്കിൽ (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ഘടന box *child_box;

			എങ്കിൽ (dom_കണ്ണി_get_user_data(child,
					corestring_dom_ns_താക്കോൽ_box_കണ്ണി_data,
					&child_box) != DOM_NO_ERR) {
				dom_കണ്ണി_unref(child);
				തിരികെ;
			}

			എങ്കിൽ (child_box != ശൂന്യ &&
			    child_box->list_marker != ശൂന്യ) {
				dom_long മൂല്യം;
				ഘടന box *marker = child_box->list_marker;
				എങ്കിൽ (layout_get_li_മൂല്യം(child, &മൂല്യം)) {
					marker->list_മൂല്യം = മൂല്യം;
					next = marker->list_മൂല്യം;
				} അല്ലെങ്കിൽ {
					marker->list_മൂല്യം = next;
				}
				next += step;
			}
		}

		exc = dom_കണ്ണി_get_next_sibling(child, &temp_കണ്ണി);
		dom_കണ്ണി_unref(child);
		എങ്കിൽ (exc != DOM_NO_ERR) {
			തിരികെ;
		}

		child = temp_കണ്ണി;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
സ്ഥിതം ശൂന്യം
layout__set_numerical_marker_text(
		സ്ഥിരം html_content *content,
		ഘടന box *box)
{
	ഘടന box *marker = box->list_marker;
	size_t counter_len;
	css_error css_res;
	എണ്ണൽ {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_SIZE = 20,
	};

	marker->text = talloc_array(content->bctx, അക്ഷരം, LIST_MARKER_SIZE);
	എങ്കിൽ (marker->text == ശൂന്യ) {
		തിരികെ;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_മൂല്യം,
			marker->text, LIST_MARKER_SIZE, &counter_len);
	എങ്കിൽ (css_res == CSS_OK) {
		എങ്കിൽ (counter_len > LIST_MARKER_SIZE) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->text = talloc_realloc(content->bctx,
					marker->text,
					അക്ഷരം,
					counter_len);
			എങ്കിൽ (marker->text == ശൂന്യ) {
				തിരികെ;
			}
			css_computed_format_list_style(box->style,
					marker->list_മൂല്യം, marker->text,
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
സ്ഥിതം bool
layout__list_item_is_numerical(
		സ്ഥിരം ഘടന box *b)
{
	എണ്ണൽ css_list_style_type_e t = css_computed_list_style_type(b->style);

	തിരഞ്ഞെടുപ്പ് (t) {
	അവസ്ഥ CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	അവസ്ഥ CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	അവസ്ഥ CSS_LIST_STYLE_TYPE_SQUARE: /* Fall through. */
	അവസ്ഥ CSS_LIST_STYLE_TYPE_NONE:
		തിരികെ false;

	സ്ഥിരസ്ഥിതി:
		തിരികെ true;
	}
}

/**
 * Layout list markers.
 */
സ്ഥിതം ശൂന്യം
layout_lists(സ്ഥിരം html_content *content, ഘടന box *box)
{
	ഘടന box *child;

	layout_ordered_list_എണ്ണം(box);

	വേണ്ടി (child = box->children; child; child = child->next) {
		എങ്കിൽ (child->list_marker) {
			ഘടന box *marker = child->list_marker;

			എങ്കിൽ (layout__list_item_is_numerical(child)) {
				എങ്കിൽ (marker->text == ശൂന്യ) {
					layout__set_numerical_marker_text(
							content, child);
				}
			}
			എങ്കിൽ (marker->object) {
				marker->width =
					content_get_width(marker->object);
				marker->x = -marker->width;
				marker->height =
					content_get_height(marker->object);
				marker->y = (വരി_height(
						&content->unit_len_ctx,
						marker->style) -
						marker->height) / 2;
			} അല്ലെങ്കിൽ എങ്കിൽ (marker->text) {
				എങ്കിൽ (marker->width == UNKNOWN_WIDTH) {
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
				marker->height = വരി_height(
						&content->unit_len_ctx,
						marker->style);
			} അല്ലെങ്കിൽ {
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
സ്ഥിതം ശൂന്യം
layout_compute_offsets(സ്ഥിരം css_unit_ctx *unit_len_ctx,
		       ഘടന box *box,
		       ഘടന box *containing_block,
		       പൂർണ്ണസംഖ്യ *മുകൾ,
		       പൂർണ്ണസംഖ്യ *വലത്,
		       പൂർണ്ണസംഖ്യ *bottom,
		       പൂർണ്ണസംഖ്യ *ഇടത്)
{
	uint32_t type;
	css_fixed മൂല്യം = 0;
	css_unit unit = CSS_UNIT_PX;

	പ്രതിജ്ഞാ(containing_block->width != UNKNOWN_WIDTH);
	പ്രതിജ്ഞാ(containing_block->width != AUTO);
	പ്രതിജ്ഞാ(containing_block->height != AUTO);

	/*ഇടത്*/
	type = css_computed_ഇടത്(box->style, &മൂല്യം, &unit);
	എങ്കിൽ (type == CSS_ഇടത്_SET) {
		എങ്കിൽ (unit == CSS_UNIT_PCT) {
			*ഇടത് = FPCT_OF_INT_TOINT(മൂല്യം,
					containing_block->width);
		} അല്ലെങ്കിൽ {
			*ഇടത് = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					മൂല്യം, unit));
		}
	} അല്ലെങ്കിൽ {
		*ഇടത് = AUTO;
	}

	/*വലത്*/
	type = css_computed_വലത്(box->style, &മൂല്യം, &unit);
	എങ്കിൽ (type == CSS_വലത്_SET) {
		എങ്കിൽ (unit == CSS_UNIT_PCT) {
			*വലത് = FPCT_OF_INT_TOINT(മൂല്യം,
					containing_block->width);
		} അല്ലെങ്കിൽ {
			*വലത് = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					മൂല്യം, unit));
		}
	} അല്ലെങ്കിൽ {
		*വലത് = AUTO;
	}

	/*മുകൾ*/
	type = css_computed_മുകൾ(box->style, &മൂല്യം, &unit);
	എങ്കിൽ (type == CSS_മുകൾ_SET) {
		എങ്കിൽ (unit == CSS_UNIT_PCT) {
			*മുകൾ = FPCT_OF_INT_TOINT(മൂല്യം,
					containing_block->height);
		} അല്ലെങ്കിൽ {
			*മുകൾ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					മൂല്യം, unit));
		}
	} അല്ലെങ്കിൽ {
		*മുകൾ = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &മൂല്യം, &unit);
	എങ്കിൽ (type == CSS_BOTTOM_SET) {
		എങ്കിൽ (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_OF_INT_TOINT(മൂല്യം,
					containing_block->height);
		} അല്ലെങ്കിൽ {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					മൂല്യം, unit));
		}
	} അല്ലെങ്കിൽ {
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
സ്ഥിതം bool
layout_absolute(ഘടന box *box,
		ഘടന box *containing_block,
		പൂർണ്ണസംഖ്യ cx, പൂർണ്ണസംഖ്യ cy,
		html_content *content)
{
	പൂർണ്ണസംഖ്യ static_ഇടത്, static_മുകൾ;  /* static position */
	പൂർണ്ണസംഖ്യ മുകൾ, വലത്, bottom, ഇടത്;
	പൂർണ്ണസംഖ്യ width, height, max_width, min_width;
	പൂർണ്ണസംഖ്യ *margin = box->margin;
	പൂർണ്ണസംഖ്യ *padding = box->padding;
	ഘടന box_border *border = box->border;
	പൂർണ്ണസംഖ്യ available_width = containing_block->width;
	പൂർണ്ണസംഖ്യ space;

	പ്രതിജ്ഞാ(box->type == BOX_BLOCK || box->type == BOX_TABLE ||
			box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_ഇടത് = cx + box->x;
	static_മുകൾ = cy + box->y;

	എങ്കിൽ (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_block->width += containing_block->padding[ഇടത്_2] +
				containing_block->padding[വലത്_2];
		containing_block->height += containing_block->padding[മുകൾ_2] +
				containing_block->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_block,
			&മുകൾ, &വലത്, &bottom, &ഇടത്);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_block;
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1,
			box, box->style, &width, &height,
			&max_width, &min_width, 0, 0,
			margin, padding, border);
	box->float_container = ശൂന്യ;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ഇടത്, margin[ഇടത്_2], border[ഇടത്_2].width, padding[ഇടത്_2], width,
	      padding[വലത്_2], border[വലത്_2].width, margin[വലത്_2], വലത്,
	      containing_block->width);


	എങ്കിൽ (ഇടത് == AUTO && width == AUTO && വലത് == AUTO) {
		എങ്കിൽ (margin[ഇടത്_2] == AUTO)
			margin[ഇടത്_2] = 0;
		എങ്കിൽ (margin[വലത്_2] == AUTO)
			margin[വലത്_2] = 0;
		ഇടത് = static_ഇടത്;

		width = min(max(box->min_width, available_width),
			box->max_width);
		width -= box->margin[ഇടത്_2] + box->border[ഇടത്_2].width +
			box->padding[ഇടത്_2] + box->padding[വലത്_2] +
			box->border[വലത്_2].width + box->margin[വലത്_2];

		/* Adjust for {min|max}-width */
		എങ്കിൽ (max_width >= 0 && width > max_width) width = max_width;
		എങ്കിൽ (width < min_width) width = min_width;

		വലത് = containing_block->width -
			ഇടത് -
			margin[ഇടത്_2] - border[ഇടത്_2].width - padding[ഇടത്_2] -
			width -
			padding[വലത്_2] - border[വലത്_2].width - margin[വലത്_2];
	} അല്ലെങ്കിൽ എങ്കിൽ (ഇടത് != AUTO && width != AUTO && വലത് != AUTO) {

		/* Adjust for {min|max}-width */
		എങ്കിൽ (max_width >= 0 && width > max_width) width = max_width;
		എങ്കിൽ (min_width >  0 && width < min_width) width = min_width;

		എങ്കിൽ (margin[ഇടത്_2] == AUTO && margin[വലത്_2] == AUTO) {
			space = containing_block->width -
					ഇടത് - border[ഇടത്_2].width -
					padding[ഇടത്_2] - width - padding[വലത്_2] -
					border[വലത്_2].width - വലത്;
			എങ്കിൽ (space < 0) {
				margin[ഇടത്_2] = 0;
				margin[വലത്_2] = space;
			} അല്ലെങ്കിൽ {
				margin[ഇടത്_2] = margin[വലത്_2] = space / 2;
			}
		} അല്ലെങ്കിൽ എങ്കിൽ (margin[ഇടത്_2] == AUTO) {
			margin[ഇടത്_2] = containing_block->width -
					ഇടത് - border[ഇടത്_2].width -
					padding[ഇടത്_2] - width - padding[വലത്_2] -
					border[വലത്_2].width - margin[വലത്_2] -
					വലത്;
		} അല്ലെങ്കിൽ എങ്കിൽ (margin[വലത്_2] == AUTO) {
			margin[വലത്_2] = containing_block->width -
					ഇടത് - margin[ഇടത്_2] -
					border[ഇടത്_2].width -
					padding[ഇടത്_2] - width - padding[വലത്_2] -
					border[വലത്_2].width - വലത്;
		} അല്ലെങ്കിൽ {
			വലത് = containing_block->width -
					ഇടത് - margin[ഇടത്_2] -
					border[ഇടത്_2].width -
					padding[ഇടത്_2] - width - padding[വലത്_2] -
					border[വലത്_2].width - margin[വലത്_2];
		}
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (margin[ഇടത്_2] == AUTO)
			margin[ഇടത്_2] = 0;
		എങ്കിൽ (margin[വലത്_2] == AUTO)
			margin[വലത്_2] = 0;

		എങ്കിൽ (ഇടത് == AUTO && width == AUTO && വലത് != AUTO) {
			available_width -= വലത്;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[ഇടത്_2] + box->border[ഇടത്_2].width +
				box->padding[ഇടത്_2] + box->padding[വലത്_2] +
				box->border[വലത്_2].width + box->margin[വലത്_2];

			/* Adjust for {min|max}-width */
			എങ്കിൽ (max_width >= 0 && width > max_width)
				width = max_width;
			എങ്കിൽ (width < min_width)
				width = min_width;

			ഇടത് = containing_block->width -
					margin[ഇടത്_2] - border[ഇടത്_2].width -
					padding[ഇടത്_2] - width - padding[വലത്_2] -
					border[വലത്_2].width - margin[വലത്_2] -
					വലത്;
		} അല്ലെങ്കിൽ എങ്കിൽ (ഇടത് == AUTO && width != AUTO && വലത് == AUTO) {

			/* Adjust for {min|max}-width */
			എങ്കിൽ (max_width >= 0 && width > max_width)
				width = max_width;
			എങ്കിൽ (min_width >  0 && width < min_width)
				width = min_width;

			ഇടത് = static_ഇടത്;
			വലത് = containing_block->width -
					ഇടത് - margin[ഇടത്_2] -
					border[ഇടത്_2].width -
					padding[ഇടത്_2] - width - padding[വലത്_2] -
					border[വലത്_2].width - margin[വലത്_2];
		} അല്ലെങ്കിൽ എങ്കിൽ (ഇടത് != AUTO && width == AUTO && വലത് == AUTO) {
			available_width -= ഇടത്;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[ഇടത്_2] + box->border[ഇടത്_2].width +
				box->padding[ഇടത്_2] + box->padding[വലത്_2] +
				box->border[വലത്_2].width + box->margin[വലത്_2];

			/* Adjust for {min|max}-width */
			എങ്കിൽ (max_width >= 0 && width > max_width)
				width = max_width;
			എങ്കിൽ (width < min_width)
				width = min_width;

			വലത് = containing_block->width -
					ഇടത് - margin[ഇടത്_2] -
					border[ഇടത്_2].width -
					padding[ഇടത്_2] - width - padding[വലത്_2] -
					border[വലത്_2].width - margin[വലത്_2];
		} അല്ലെങ്കിൽ എങ്കിൽ (ഇടത് == AUTO && width != AUTO && വലത് != AUTO) {

			/* Adjust for {min|max}-width */
			എങ്കിൽ (max_width >= 0 && width > max_width)
				width = max_width;
			എങ്കിൽ (width < min_width)
				width = min_width;

			ഇടത് = containing_block->width -
					margin[ഇടത്_2] - border[ഇടത്_2].width -
					padding[ഇടത്_2] - width - padding[വലത്_2] -
					border[വലത്_2].width - margin[വലത്_2] -
					വലത്;
		} അല്ലെങ്കിൽ എങ്കിൽ (ഇടത് != AUTO && width == AUTO && വലത് != AUTO) {
			width = containing_block->width -
					ഇടത് - margin[ഇടത്_2] -
					border[ഇടത്_2].width -
					padding[ഇടത്_2] - padding[വലത്_2] -
					border[വലത്_2].width - margin[വലത്_2] -
					വലത്;

			/* Adjust for {min|max}-width */
			എങ്കിൽ (max_width >= 0 && width > max_width)
				width = max_width;
			എങ്കിൽ (width < min_width)
				width = min_width;

		} അല്ലെങ്കിൽ എങ്കിൽ (ഇടത് != AUTO && width != AUTO && വലത് == AUTO) {

			/* Adjust for {min|max}-width */
			എങ്കിൽ (max_width >= 0 && width > max_width)
				width = max_width;
			എങ്കിൽ (width < min_width)
				width = min_width;

			വലത് = containing_block->width -
					ഇടത് - margin[ഇടത്_2] -
					border[ഇടത്_2].width -
					padding[ഇടത്_2] - width - padding[വലത്_2] -
					border[വലത്_2].width - margin[വലത്_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ഇടത്, margin[ഇടത്_2], border[ഇടത്_2].width, padding[ഇടത്_2], width,
	      padding[വലത്_2], border[വലത്_2].width, margin[വലത്_2], വലത്,
	      containing_block->width);

	box->x = ഇടത് + margin[ഇടത്_2] + border[ഇടത്_2].width - cx;
	എങ്കിൽ (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_block->width -= containing_block->padding[ഇടത്_2] +
				containing_block->padding[വലത്_2];
	} അല്ലെങ്കിൽ {
		/** \todo inline ancestors */
	}
	box->width = width;
	box->height = height;

	എങ്കിൽ (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->object || box->flags & IFRAME) {
		എങ്കിൽ (!layout_block_context(box, -1, content))
			തിരികെ false;
	} അല്ലെങ്കിൽ എങ്കിൽ (box->type == BOX_TABLE) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		/* \todo  layout_table considers margins etc. again */
		എങ്കിൽ (!layout_table(box, width, content))
			തിരികെ false;
		box->float_container = ശൂന്യ;
		layout_solve_width(box, box->parent->width, box->width, 0, 0,
				-1, -1);
	} അല്ലെങ്കിൽ എങ്കിൽ (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		എങ്കിൽ (!layout_flex(box, width, content))
			തിരികെ false;
		box->float_container = ശൂന്യ;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      മുകൾ, margin[മുകൾ_2], border[മുകൾ_2].width, padding[മുകൾ_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	എങ്കിൽ (മുകൾ == AUTO && height == AUTO && bottom == AUTO) {
		മുകൾ = static_മുകൾ;
		height = box->height;
		എങ്കിൽ (margin[മുകൾ_2] == AUTO)
			margin[മുകൾ_2] = 0;
		എങ്കിൽ (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_block->height -
				മുകൾ - margin[മുകൾ_2] - border[മുകൾ_2].width -
				padding[മുകൾ_2] - height - padding[BOTTOM] -
				border[BOTTOM].width - margin[BOTTOM];
	} അല്ലെങ്കിൽ എങ്കിൽ (മുകൾ != AUTO && height != AUTO && bottom != AUTO) {
		എങ്കിൽ (margin[മുകൾ_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_block->height -
					മുകൾ - border[മുകൾ_2].width - padding[മുകൾ_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - bottom;
			margin[മുകൾ_2] = margin[BOTTOM] = space / 2;
		} അല്ലെങ്കിൽ എങ്കിൽ (margin[മുകൾ_2] == AUTO) {
			margin[മുകൾ_2] = containing_block->height -
					മുകൾ - border[മുകൾ_2].width - padding[മുകൾ_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} അല്ലെങ്കിൽ എങ്കിൽ (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_block->height -
					മുകൾ - margin[മുകൾ_2] - border[മുകൾ_2].width -
					padding[മുകൾ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					bottom;
		} അല്ലെങ്കിൽ {
			bottom = containing_block->height -
					മുകൾ - margin[മുകൾ_2] - border[മുകൾ_2].width -
					padding[മുകൾ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (margin[മുകൾ_2] == AUTO)
			margin[മുകൾ_2] = 0;
		എങ്കിൽ (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		എങ്കിൽ (മുകൾ == AUTO && height == AUTO && bottom != AUTO) {
			height = box->height;
			മുകൾ = containing_block->height -
					margin[മുകൾ_2] - border[മുകൾ_2].width -
					padding[മുകൾ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} അല്ലെങ്കിൽ എങ്കിൽ (മുകൾ == AUTO && height != AUTO && bottom == AUTO) {
			മുകൾ = static_മുകൾ;
			bottom = containing_block->height -
					മുകൾ - margin[മുകൾ_2] - border[മുകൾ_2].width -
					padding[മുകൾ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} അല്ലെങ്കിൽ എങ്കിൽ (മുകൾ != AUTO && height == AUTO && bottom == AUTO) {
			height = box->height;
			bottom = containing_block->height -
					മുകൾ - margin[മുകൾ_2] - border[മുകൾ_2].width -
					padding[മുകൾ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} അല്ലെങ്കിൽ എങ്കിൽ (മുകൾ == AUTO && height != AUTO && bottom != AUTO) {
			മുകൾ = containing_block->height -
					margin[മുകൾ_2] - border[മുകൾ_2].width -
					padding[മുകൾ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} അല്ലെങ്കിൽ എങ്കിൽ (മുകൾ != AUTO && height == AUTO && bottom != AUTO) {
			height = containing_block->height -
					മുകൾ - margin[മുകൾ_2] - border[മുകൾ_2].width -
					padding[മുകൾ_2] - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} അല്ലെങ്കിൽ എങ്കിൽ (മുകൾ != AUTO && height != AUTO && bottom == AUTO) {
			bottom = containing_block->height -
					മുകൾ - margin[മുകൾ_2] - border[മുകൾ_2].width -
					padding[മുകൾ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      മുകൾ, margin[മുകൾ_2], border[മുകൾ_2].width, padding[മുകൾ_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	box->y = മുകൾ + margin[മുകൾ_2] + border[മുകൾ_2].width - cy;
	എങ്കിൽ (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_block->height -= containing_block->padding[മുകൾ_2] +
				containing_block->padding[BOTTOM];
	} അല്ലെങ്കിൽ {
		/** \todo Inline ancestors */
	}
	box->height = height;
	layout_apply_minmax_height(&content->unit_len_ctx, box, containing_block);

	തിരികെ true;
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
സ്ഥിതം bool
layout_position_absolute(ഘടന box *box,
			 ഘടന box *containing_block,
			 പൂർണ്ണസംഖ്യ cx, പൂർണ്ണസംഖ്യ cy,
			 html_content *content)
{
	ഘടന box *c;

	വേണ്ടി (c = box->children; c; c = c->next) {
		എങ്കിൽ ((c->type == BOX_BLOCK || c->type == BOX_TABLE ||
				c->type == BOX_INLINE_BLOCK ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			എങ്കിൽ (!layout_absolute(c, containing_block,
					cx, cy, content))
				തിരികെ false;
			എങ്കിൽ (!layout_position_absolute(c, c, 0, 0, content))
				തിരികെ false;
		} അല്ലെങ്കിൽ എങ്കിൽ (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			എങ്കിൽ (!layout_position_absolute(c, c, 0, 0, content))
				തിരികെ false;
		} അല്ലെങ്കിൽ {
			പൂർണ്ണസംഖ്യ px, py;
			എങ്കിൽ (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_ഇടത് ||
					css_computed_float(c->style) ==
						CSS_FLOAT_വലത്)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				ഘടന box *p;
				px = c->x;
				py = c->y;
				വേണ്ടി (p = box->parent; p && !p->float_children;
						p = p->parent) {
					px -= p->x;
					py -= p->y;
				}
			} അല്ലെങ്കിൽ {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			എങ്കിൽ (!layout_position_absolute(c, containing_block,
					cx + px, cy + py, content))
				തിരികെ false;
		}
	}

	തിരികെ true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
സ്ഥിതം ശൂന്യം layout_compute_relative_offset(
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		ഘടന box *box,
		പൂർണ്ണസംഖ്യ *x,
		പൂർണ്ണസംഖ്യ *y)
{
	പൂർണ്ണസംഖ്യ ഇടത്, വലത്, മുകൾ, bottom;
	ഘടന box *containing_block;

	പ്രതിജ്ഞാ(box && box->parent && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	എങ്കിൽ (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_ഇടത് ||
			css_computed_float(box->style) == CSS_FLOAT_വലത്)) {
		containing_block = box->float_container;
	} അല്ലെങ്കിൽ {
		containing_block = box->parent;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_block,
			&മുകൾ, &വലത്, &bottom, &ഇടത്);

	എങ്കിൽ (ഇടത് == AUTO && വലത് == AUTO)
		ഇടത് = വലത് = 0;
	അല്ലെങ്കിൽ എങ്കിൽ (ഇടത് == AUTO)
		/* left is auto => computed = -right */
		ഇടത് = -വലത്;
	അല്ലെങ്കിൽ എങ്കിൽ (വലത് == AUTO)
		/* right is auto => computed = -left */
		വലത് = -ഇടത്;
	അല്ലെങ്കിൽ {
		/* over constrained => examine direction property
		 * of containing block */
		എങ്കിൽ (containing_block->style &&
				css_computed_direction(
				containing_block->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			ഇടത് = -വലത്;
		} അല്ലെങ്കിൽ {
			/* assume LTR in all other cases */
			വലത് = -ഇടത്;
		}
	}

	പ്രതിജ്ഞാ(ഇടത് == -വലത്);

	എങ്കിൽ (മുകൾ == AUTO && bottom == AUTO) {
		മുകൾ = bottom = 0;
	} അല്ലെങ്കിൽ എങ്കിൽ (മുകൾ == AUTO) {
		മുകൾ = -bottom;
	} അല്ലെങ്കിൽ {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -മുകൾ;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", ഇടത്,
	      വലത്, മുകൾ, bottom);

	*x = ഇടത്;
	*y = മുകൾ;
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
സ്ഥിതം ശൂന്യം
layout_position_relative(
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		ഘടന box *വേര്,
		ഘടന box *fp,
		പൂർണ്ണസംഖ്യ fx,
		പൂർണ്ണസംഖ്യ fy)
{
	ഘടന box *box; /* for children of "root" */
	ഘടന box *fn;  /* for block formatting context box for children of
			  * "box" */
	ഘടന box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	പൂർണ്ണസംഖ്യ x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	പൂർണ്ണസംഖ്യ fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	പ്രതിജ്ഞാ(വേര്);

	/* Normal children */
	വേണ്ടി (box = വേര്->children; box; box = box->next) {

		എങ്കിൽ (box->type == BOX_TEXT)
			തുടരുക;

		/* If relatively positioned, get offsets */
		എങ്കിൽ (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_offset(
					unit_len_ctx, box, &x, &y);
		അല്ലെങ്കിൽ
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		എങ്കിൽ (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_ഇടത് ||
				css_computed_float(box->style) ==
					CSS_FLOAT_വലത്) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			വേണ്ടി (fc = fp->float_children; fc; fc = fc->next_float) {
				എങ്കിൽ (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		എങ്കിൽ (box->float_children) {
			fn = box;
			fnx = fny = 0;
		} അല്ലെങ്കിൽ {
			fn = fp;
			fnx = fx + x;
			fny = fy + y;
		}

		/* recurse first */
		layout_position_relative(unit_len_ctx, box, fn, fnx, fny);

		/* Ignore things we're not interested in. */
		എങ്കിൽ (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			തുടരുക;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		എങ്കിൽ (box->type == BOX_INLINE && box->inline_end) {
			ഘടന box *b;
			വേണ്ടി (b = box->next; b && b != box->inline_end;
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
സ്ഥിതം ശൂന്യം
layout_get_box_bbox(
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		ഘടന box *box,
		പൂർണ്ണസംഖ്യ *desc_x0, പൂർണ്ണസംഖ്യ *desc_y0,
		പൂർണ്ണസംഖ്യ *desc_x1, പൂർണ്ണസംഖ്യ *desc_y1)
{
	*desc_x0 = -box->border[ഇടത്_2].width;
	*desc_y0 = -box->border[മുകൾ_2].width;
	*desc_x1 = box->padding[ഇടത്_2] + box->width + box->padding[വലത്_2] +
			box->border[വലത്_2].width;
	*desc_y1 = box->padding[മുകൾ_2] + box->height + box->padding[BOTTOM] +
			box->border[BOTTOM].width;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	എങ്കിൽ (box->type == BOX_BLOCK && box->style != ശൂന്യ &&
			css_computed_overflow_y(box->style) ==
					CSS_OVERFLOW_VISIBLE &&
			box->object == ശൂന്യ) {
		css_fixed font_size = 0;
		css_unit font_unit = CSS_UNIT_PT;
		പൂർണ്ണസംഖ്യ text_height;

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
സ്ഥിതം ശൂന്യം
layout_update_descendant_bbox(
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		ഘടന box *box,
		ഘടന box *child,
		പൂർണ്ണസംഖ്യ off_x,
		പൂർണ്ണസംഖ്യ off_y)
{
	പൂർണ്ണസംഖ്യ child_desc_x0, child_desc_y0, child_desc_x1, child_desc_y1;

	/* get coordinates of child relative to box */
	പൂർണ്ണസംഖ്യ child_x = child->x - off_x;
	പൂർണ്ണസംഖ്യ child_y = child->y - off_y;

	bool html_object = (child->object &&
			content_get_type(child->object) == CONTENT_HTML);

	എണ്ണൽ css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	എണ്ണൽ css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

	എങ്കിൽ (child->style != ശൂന്യ) {
		overflow_x = css_computed_overflow_x(child->style);
		overflow_y = css_computed_overflow_y(child->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, child,
			&child_desc_x0, &child_desc_y0,
			&child_desc_x1, &child_desc_y1);

	എങ്കിൽ (overflow_x == CSS_OVERFLOW_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		child_desc_x0 = child->descendant_x0;
		child_desc_x1 = child->descendant_x1;
	}
	എങ്കിൽ (overflow_y == CSS_OVERFLOW_VISIBLE &&
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
	എങ്കിൽ (child_desc_x0 < box->descendant_x0)
		box->descendant_x0 = child_desc_x0;
	എങ്കിൽ (child_desc_y0 < box->descendant_y0)
		box->descendant_y0 = child_desc_y0;
	എങ്കിൽ (box->descendant_x1 < child_desc_x1)
		box->descendant_x1 = child_desc_x1;
	എങ്കിൽ (box->descendant_y1 < child_desc_y1)
		box->descendant_y1 = child_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
സ്ഥിതം ശൂന്യം layout_calculate_descendant_bboxes(
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		ഘടന box *box)
{
	ഘടന box *child;

	പ്രതിജ്ഞാ(box->width != UNKNOWN_WIDTH);
	പ്രതിജ്ഞാ(box->height != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	എങ്കിൽ (box->object && content_get_type(box->object) == CONTENT_HTML) {
		എങ്കിൽ (box->descendant_x1 < content_get_width(box->object))
			box->descendant_x1 = content_get_width(box->object);
		എങ്കിൽ (box->descendant_y1 < content_get_height(box->object))
			box->descendant_y1 = content_get_height(box->object);
	}

	എങ്കിൽ (box->iframe != ശൂന്യ) {
		പൂർണ്ണസംഖ്യ x, y;
		box_coords(box, &x, &y);

		browser_window_set_position(box->iframe, x, y);
		browser_window_set_dimensions(box->iframe,
				box->width, box->height);
		browser_window_reformat(box->iframe, true,
				box->width, box->height);
	}

	എങ്കിൽ (box->type == BOX_INLINE || box->type == BOX_TEXT)
		തിരികെ;

	എങ്കിൽ (box->type == BOX_INLINE_END) {
		box = box->inline_end;
		വേണ്ടി (child = box->next; child;
				child = child->next) {
			എങ്കിൽ (child->type == BOX_FLOAT_ഇടത് ||
					child->type == BOX_FLOAT_വലത്)
				തുടരുക;

			layout_update_descendant_bbox(unit_len_ctx, box, child,
					box->x, box->y);

			എങ്കിൽ (child == box->inline_end)
				നിർത്തുക;
		}
		തിരികെ;
	}

	എങ്കിൽ (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		തിരികെ;

	വേണ്ടി (child = box->children; child; child = child->next) {
		എങ്കിൽ (child->type == BOX_FLOAT_ഇടത് ||
				child->type == BOX_FLOAT_വലത്)
			തുടരുക;

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		എങ്കിൽ (box->style && css_computed_overflow_x(box->style) ==
				CSS_OVERFLOW_HIDDEN &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_HIDDEN)
			തുടരുക;

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	വേണ്ടി (child = box->float_children; child; child = child->next_float) {
		പ്രതിജ്ഞാ(child->type == BOX_FLOAT_ഇടത് ||
				child->type == BOX_FLOAT_വലത്);

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	എങ്കിൽ (box->list_marker) {
		child = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height)
{
	bool ret;
	ഘടന box *doc = content->layout;
	സ്ഥിരം ഘടന gui_layout_table *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			width, height, nsurl_access(content_get_url(
					&content->base)));

	layout_minmax_block(doc, font_func, content);

	layout_block_find_dimensions(&content->unit_len_ctx,
			width, height, 0, 0, doc);
	doc->x = doc->margin[ഇടത്_2] + doc->border[ഇടത്_2].width;
	doc->y = doc->margin[മുകൾ_2] + doc->border[മുകൾ_2].width;
	width -= doc->margin[ഇടത്_2] + doc->border[ഇടത്_2].width +
			doc->padding[ഇടത്_2] + doc->padding[വലത്_2] +
			doc->border[വലത്_2].width + doc->margin[വലത്_2];
	എങ്കിൽ (width < 0) {
		width = 0;
	}
	doc->width = width;

	ret = layout_block_context(doc, height, content);

	/* make <html> and <body> fill available height */
	എങ്കിൽ (doc->y + doc->padding[മുകൾ_2] + doc->height + doc->padding[BOTTOM] +
			doc->border[BOTTOM].width + doc->margin[BOTTOM] <
			height) {
		doc->height = height - (doc->y + doc->padding[മുകൾ_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].width +
				doc->margin[BOTTOM]);
		എങ്കിൽ (doc->children)
			doc->children->height = doc->height -
					(doc->children->margin[മുകൾ_2] +
					 doc->children->border[മുകൾ_2].width +
					 doc->children->padding[മുകൾ_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].width +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	തിരികെ ret;
}
