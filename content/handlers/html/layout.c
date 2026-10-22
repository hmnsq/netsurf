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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <math.h>
#অন্তর্ভাব <dom/dom.h>

#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/talloc.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "netsurf/browser_window.h"
#অন্তর্ভাব "netsurf/layout.h"
#অন্তর্ভাব "content/content.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "css/utils.h"
#অন্তর্ভাব "desktop/scrollbar.h"
#অন্তর্ভাব "desktop/textarea.h"

#অন্তর্ভাব "html/html.h"
#অন্তর্ভাব "html/html_save.h"
#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/box_inspect.h"
#অন্তর্ভাব "html/font.h"
#অন্তর্ভাব "html/form_internal.h"
#অন্তর্ভাব "html/layout.h"
#অন্তর্ভাব "html/layout_internal.h"
#অন্তর্ভাব "html/table.h"

/** Array of per-side access functions for computed style margins. */
ধ্রুবক css_len_func margin_funcs[4] = {
	[শীর্ষ_2]    = css_computed_margin_শীর্ষ,
	[ডান_2]  = css_computed_margin_ডান,
	[BOTTOM] = css_computed_margin_bottom,
	[বাম_2]   = css_computed_margin_বাম,
};

/** Array of per-side access functions for computed style paddings. */
ধ্রুবক css_len_func padding_funcs[4] = {
	[শীর্ষ_2]    = css_computed_padding_শীর্ষ,
	[ডান_2]  = css_computed_padding_ডান,
	[BOTTOM] = css_computed_padding_bottom,
	[বাম_2]   = css_computed_padding_বাম,
};

/** Array of per-side access functions for computed style border_widths. */
ধ্রুবক css_len_func border_width_funcs[4] = {
	[শীর্ষ_2]    = css_computed_border_শীর্ষ_width,
	[ডান_2]  = css_computed_border_ডান_width,
	[BOTTOM] = css_computed_border_bottom_width,
	[বাম_2]   = css_computed_border_বাম_width,
};

/** Array of per-side access functions for computed style border styles. */
ধ্রুবক css_border_style_func border_style_funcs[4] = {
	[শীর্ষ_2]    = css_computed_border_শীর্ষ_style,
	[ডান_2]  = css_computed_border_ডান_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[বাম_2]   = css_computed_border_বাম_style,
};

/** Array of per-side access functions for computed style border colors. */
ধ্রুবক css_border_color_func border_color_funcs[4] = {
	[শীর্ষ_2]    = css_computed_border_শীর্ষ_color,
	[ডান_2]  = css_computed_border_ডান_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[বাম_2]   = css_computed_border_বাম_color,
};

/* forward declaration to break cycles */
স্থির শূন্য layout_minmax_block(
		কাঠামো box *block,
		ধ্রুবক কাঠামো gui_layout_table *font_func,
		ধ্রুবক html_content *content);

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
স্থির শূন্য
layout_get_object_dimensions(কাঠামো box *box,
			     পূর্ণসংখ্যা *width, পূর্ণসংখ্যা *height,
			     পূর্ণসংখ্যা min_width, পূর্ণসংখ্যা max_width,
			     পূর্ণসংখ্যা min_height, পূর্ণসংখ্যা max_height)
{
	প্রতিজ্ঞা(box->object != ফাঁকা);
	প্রতিজ্ঞা(width != ফাঁকা && height != ফাঁকা);

	যদি (*width == AUTO && *height == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		পূর্ণসংখ্যা intrinsic_width = content_get_width(box->object);
		পূর্ণসংখ্যা intrinsic_height = content_get_height(box->object);

		/* use intrinsic dimensions */
		*width = intrinsic_width;
		*height = intrinsic_height;

		/* Deal with min/max-width first */
		যদি (min_width >  0 && min_width > *width) {
			*width = min_width;
			scaled = true;
		}
		যদি (max_width >= 0 && max_width < *width) {
			*width = max_width;
			scaled = true;
		}

		যদি (scaled && (intrinsic_width != 0)) {
			/* Update height */
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		}

		scaled = false;
		/* Deal with min/max-height */
		যদি (min_height >  0 && min_height > *height) {
			*height = min_height;
			scaled = true;
		}
		যদি (max_height >= 0 && max_height < *height) {
			*height = max_height;
			scaled = true;
		}

		যদি (scaled && (intrinsic_height != 0)) {
			/* Update width */
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		}

	} অন্যথা যদি (*width == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		পূর্ণসংখ্যা intrinsic_width = content_get_width(box->object);
		পূর্ণসংখ্যা intrinsic_height = content_get_height(box->object);

		যদি (intrinsic_height != 0)
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		অন্যথা
			*width = intrinsic_width;

		যদি (min_width >  0 && min_width > *width)
			*width = min_width;
		যদি (max_width >= 0 && max_width < *width)
			*width = max_width;

	} অন্যথা যদি (*height == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		পূর্ণসংখ্যা intrinsic_width = content_get_width(box->object);
		পূর্ণসংখ্যা intrinsic_height = content_get_height(box->object);

		যদি (min_width >  0 && min_width > *width)
			*width = min_width;
		যদি (max_width >= 0 && max_width < *width)
			*width = max_width;

		যদি (intrinsic_width != 0)
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		অন্যথা
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
স্থির পূর্ণসংখ্যা layout_text_indent(
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		ধ্রুবক css_computed_style *style, পূর্ণসংখ্যা width)
{
	css_fixed মান = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_text_indent(style, &মান, &unit);

	যদি (unit == CSS_UNIT_PCT) {
		ফেরত FPCT_OF_INT_TOINT(মান, width);
	} অন্যথা {
		ফেরত FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				মান, unit));
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
স্থির শূন্য layout_minmax_table(কাঠামো box *table,
		ধ্রুবক কাঠামো gui_layout_table *font_func,
		ধ্রুবক html_content *content)
{
	unsigned পূর্ণসংখ্যা i, j;
	পূর্ণসংখ্যা width;
	পূর্ণসংখ্যা border_spacing_h = 0;
	পূর্ণসংখ্যা table_min = 0, table_max = 0;
	পূর্ণসংখ্যা extra_fixed = 0;
	দশমিক extra_frac = 0;
	কাঠামো column *col;
	কাঠামো box *row_group, *row, *cell;

	/* check if the widths have already been calculated */
	যদি (table->max_width != UNKNOWN_MAX_WIDTH)
		ফেরত;

	যদি (table_calculate_column_types(&content->unit_len_ctx, table) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		ফেরত;
	}
	col = table->col;

	/* start with 0 except for fixed-width columns */
	জন্য (i = 0; i != table->columns; i++) {
		যদি (col[i].type == COLUMN_WIDTH_FIXED)
			col[i].min = col[i].max = col[i].width;
		অন্যথা
			col[i].min = col[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	যদি (css_computed_border_collapse(table->style) ==
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
	জন্য (row_group = table->children; row_group; row_group =row_group->পরবর্তী)
	জন্য (row = row_group->children; row; row = row->পরবর্তী)
	জন্য (cell = row->children; cell; cell = cell->পরবর্তী) {
		প্রতিজ্ঞা(cell->type == BOX_TABLE_CELL);
		প্রতিজ্ঞা(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		প্রতিজ্ঞা(cell->columns != 0);

		যদি (cell->columns != 1)
			চালু;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		যদি (col[i].positioned)
			চালু;

		/* update column min, max widths using cell widths */
		যদি (col[i].min < cell->min_width)
			col[i].min = cell->min_width;
		যদি (col[i].max < cell->max_width)
			col[i].max = cell->max_width;
	}

	/* 2nd pass: cells which span multiple columns */
	জন্য (row_group = table->children; row_group; row_group =row_group->পরবর্তী)
	জন্য (row = row_group->children; row; row = row->পরবর্তী)
	জন্য (cell = row->children; cell; cell = cell->পরবর্তী) {
		unsigned পূর্ণসংখ্যা flexible_columns = 0;
		পূর্ণসংখ্যা min = 0, max = 0, fixed_width = 0, extra;

		যদি (cell->columns == 1)
			চালু;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		জন্য (j = 0; j != cell->columns; j++) {
			min += col[i + j].min;
			যদি (col[i + j].type == COLUMN_WIDTH_FIXED)
				fixed_width += col[i + j].width;
			অন্যথা
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		যদি (min < cell->min_width) {
			যদি (flexible_columns == 0) {
				extra = 1 + (cell->min_width - min) /
						cell->columns;
				জন্য (j = 0; j != cell->columns; j++) {
					col[i + j].min += extra;
					যদি (col[i + j].max < col[i + j].min)
						col[i + j].max = col[i + j].min;
				}
			} অন্যথা {
				extra = 1 + (cell->min_width - min) /
						flexible_columns;
				জন্য (j = 0; j != cell->columns; j++) {
					যদি (col[i + j].type !=
							COLUMN_WIDTH_FIXED) {
						col[i + j].min += extra;
						যদি (col[i + j].max <
								col[i + j].min)
							col[i + j].max =
								col[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		জন্য (j = 0; j != cell->columns; j++)
			max += col[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		যদি (max < cell->max_width && flexible_columns) {
			extra = 1 + (cell->max_width - max) / flexible_columns;
			জন্য (j = 0; j != cell->columns; j++)
				যদি (col[i + j].type != COLUMN_WIDTH_FIXED)
					col[i + j].max += extra;
		}
	}

	জন্য (i = 0; i != table->columns; i++) {
		যদি (col[i].max < col[i].min) {
			box_dump(stderr, table, 0, true);
			প্রতিজ্ঞা(0);
		}
		table_min += col[i].min;
		table_max += col[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	যদি (css_computed_width_px(table->style, &content->unit_len_ctx,
			-1, &width) == CSS_WIDTH_SET) {
		যদি (table_min < width)
			table_min = width;
		যদি (table_max < width)
			table_max = width;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, বাম_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, ডান_2, true, true, true,
			&extra_fixed, &extra_frac);
	যদি (extra_fixed < 0)
		extra_fixed = 0;
	যদি (extra_frac < 0)
		extra_frac = 0;
	যদি (1.0 <= extra_frac)
		extra_frac = 0.9;
	table->min_width = (table_min + extra_fixed) / (1.0 - extra_frac);
	table->max_width = (table_max + extra_fixed) / (1.0 - extra_frac);
	table->min_width += (table->columns + 1) * border_spacing_h;
	table->max_width += (table->columns + 1) * border_spacing_h;

	প্রতিজ্ঞা(0 <= table->min_width && table->min_width <= table->max_width);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
স্থির অন্তঃরেখ bool box_has_percentage_max_width(কাঠামো box *b)
{
	css_unit unit = CSS_UNIT_PX;
	গণনা css_max_width_e type;
	css_fixed মান = 0;

	প্রতিজ্ঞা(b != ফাঁকা);

	type = css_computed_max_width(b->style, &মান, &unit);
	ফেরত ((type == CSS_MAX_WIDTH_SET) && (unit == CSS_UNIT_PCT));
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
স্থির কাঠামো box *
layout_minmax_সারি(কাঠামো box *first,
		   পূর্ণসংখ্যা *সারি_min,
		   পূর্ণসংখ্যা *সারি_max,
		   bool first_সারি,
		   bool *সারি_has_height,
		   ধ্রুবক কাঠামো gui_layout_table *font_func,
		   ধ্রুবক html_content *content)
{
	পূর্ণসংখ্যা min = 0, max = 0, width, height, fixed;
	দশমিক frac;
	size_t i, j;
	কাঠামো box *b;
	কাঠামো box *block;
	plot_font_style_t fstyle;
	bool no_wrap;

	প্রতিজ্ঞা(first->parent);
	প্রতিজ্ঞা(first->parent->parent);
	প্রতিজ্ঞা(first->parent->parent->style);

	block = first->parent->parent;
	no_wrap = (css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_PRE);

	*সারি_has_height = false;

	/* corresponds to the pass 1 loop in layout_line() */
	জন্য (b = first; b; b = b->পরবর্তী) {
		গণনা css_width_e wtype;
		গণনা css_height_e htype;
		গণনা css_box_sizing_e bs;
		css_fixed মান = 0;
		css_unit unit = CSS_UNIT_PX;

		প্রতিজ্ঞা(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		যদি (b->type == BOX_BR) {
			b = b->পরবর্তী;
			বিরতি;
		}

		যদি (lh__box_is_float_box(b)) {
			প্রতিজ্ঞা(b->children);
			যদি (b->children->type == BOX_TABLE)
				layout_minmax_table(b->children, font_func,
						content);
			অন্যথা
				layout_minmax_block(b->children, font_func,
						content);
			b->min_width = b->children->min_width;
			b->max_width = b->children->max_width;
			যদি (min < b->min_width)
				min = b->min_width;
			max += b->max_width;
			চালু;
		}

		যদি (b->type == BOX_INLINE_BLOCK || b->type == BOX_INLINE_FLEX) {
			layout_minmax_block(b, font_func, content);
			যদি (min < b->min_width)
				min = b->min_width;
			max += b->max_width;

			যদি (b->flags & HAS_HEIGHT)
				*সারি_has_height = true;
			চালু;
		}

		প্রতিজ্ঞা(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		যদি (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->style, বাম_2, true, true, true,
					&fixed, &frac);
			যদি (!b->inline_end)
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ডান_2,
						true, true, true,
						&fixed, &frac);
			যদি (0 < fixed)
				max += fixed;
			*সারি_has_height = true;
			/* \todo  update min width, consider fractional extra */
		} অন্যথা যদি (b->type == BOX_INLINE_END) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->inline_end->style, ডান_2,
					true, true, true,
					&fixed, &frac);
			যদি (0 < fixed)
				max += fixed;

			যদি (b->পরবর্তী) {
				যদি (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*সারি_has_height = true;
			চালু;
		}

		যদি (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			যদি (!b->text)
				চালু;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			যদি (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				যদি (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					পূর্ণসংখ্যা opt_maxwidth = 0;
					কাঠামো form_option *o;

					জন্য (o = b->parent->parent->gadget->
							তথ্য.select.বস্তুসমূহ; o;
							o = o->পরবর্তী) {
						পূর্ণসংখ্যা opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						যদি (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}

					b->width = opt_maxwidth;
					যদি (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;

				} অন্যথা {
					font_func->width(&fstyle, b->text,
						b->দৈর্ঘ্য, &b->width);
					b->flags |= MEASURED;
				}
			}
			max += b->width;
			যদি (b->পরবর্তী) {
				যদি (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			যদি (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} অন্যথা যদি (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				যদি (min < b->width)
					min = b->width;

			} অন্যথা যদি (b->parent->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				করো {
					জন্য (j = i; j != b->দৈর্ঘ্য &&
							b->text[j] != ' '; j++)
						;
					font_func->width(&fstyle, b->text + i,
							 j - i, &width);
					যদি (min < width)
						min = width;
					i = j + 1;
				} যখন (j != b->দৈর্ঘ্য);
			}

			*সারি_has_height = true;

			চালু;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		প্রতিজ্ঞা(b->style);

		bs = css_computed_box_sizing(block->style);

		/* calculate box width */
		wtype = css_computed_width_px(b->style,
				&content->unit_len_ctx, -1, &width);
		যদি (wtype == CSS_WIDTH_SET) {
			যদি (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, বাম_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, ডান_2,
						false, true, true,
						&fixed, &frac);
				যদি (width < fixed) {
					width = fixed;
				}
			}
			যদি (width < 0)
				width = 0;
		} অন্যথা {
			width = AUTO;
		}

		/* height */
		htype = css_computed_height(b->style, &মান, &unit);
		যদি (htype == CSS_HEIGHT_SET) {
			height = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					মান, unit));
		} অন্যথা {
			height = AUTO;
		}

		যদি (b->object || (b->flags & REPLACE_DIM)) {
			যদি (b->object) {
				পূর্ণসংখ্যা অস্থায়ী_height = height;
				layout_get_object_dimensions(b,
						&width, &অস্থায়ী_height,
						INT_MIN, INT_MAX,
						INT_MIN, INT_MAX);
			}

			fixed = frac = 0;
			যদি (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, বাম_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ডান_2,
						true, false, false,
						&fixed, &frac);
			} অন্যথা {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, বাম_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ডান_2,
						true, true, true,
						&fixed, &frac);
			}
			যদি (0 < width + fixed)
				width += fixed;
		} অন্যথা যদি (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			যদি (width == AUTO)
				width = 400;

			fixed = frac = 0;
			যদি (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, বাম_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ডান_2,
						true, false, false,
						&fixed, &frac);
			} অন্যথা {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, বাম_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ডান_2,
						true, true, true,
						&fixed, &frac);
			}

			যদি (0 < width + fixed)
				width += fixed;

		} অন্যথা {
			/* form control with no object */
			যদি (width == AUTO)
				width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		যদি (min < width && !box_has_percentage_max_width(b))
			min = width;
		যদি (width > 0)
			max += width;

		*সারি_has_height = true;
	}

	যদি (first_সারি) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		পূর্ণসংখ্যা text_indent = layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, 100);
		min = (min + text_indent < 0) ? 0 : min + text_indent;
		max = (max + text_indent < 0) ? 0 : max + text_indent;
	}

	*সারি_min = min;
	*সারি_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	প্রতিজ্ঞা(b != first);
	প্রতিজ্ঞা(0 <= *সারি_min);
	প্রতিজ্ঞা(*সারি_min <= *সারি_max);
	ফেরত b;
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
স্থির শূন্য
layout_minmax_inline_container(কাঠামো box *inline_container,
			       bool *has_height,
			       ধ্রুবক কাঠামো gui_layout_table *font_func,
			       ধ্রুবক html_content *content)
{
	কাঠামো box *child;
	পূর্ণসংখ্যা সারি_min = 0, সারি_max = 0;
	পূর্ণসংখ্যা min = 0, max = 0;
	bool first_সারি = true;
	bool সারি_has_height;

	প্রতিজ্ঞা(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	যদি (inline_container->max_width != UNKNOWN_MAX_WIDTH)
		ফেরত;

	*has_height = false;

	জন্য (child = inline_container->children; child; ) {
		child = layout_minmax_সারি(child, &সারি_min, &সারি_max,
				first_সারি, &সারি_has_height, font_func,
				content);
		যদি (min < সারি_min)
			min = সারি_min;
		যদি (max < সারি_max)
			max = সারি_max;
		first_সারি = false;
		*has_height |= সারি_has_height;
	}

	inline_container->min_width = min;
	inline_container->max_width = max;

	প্রতিজ্ঞা(0 <= inline_container->min_width &&
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
স্থির শূন্য layout_minmax_block(
		কাঠামো box *block,
		ধ্রুবক কাঠামো gui_layout_table *font_func,
		ধ্রুবক html_content *content)
{
	কাঠামো box *child;
	পূর্ণসংখ্যা min = 0, max = 0;
	পূর্ণসংখ্যা extra_fixed = 0;
	দশমিক extra_frac = 0;
	গণনা css_width_e wtype = CSS_WIDTH_AUTO;
	css_fixed width = 0;
	css_unit wunit = CSS_UNIT_PX;
	গণনা css_height_e htype = CSS_HEIGHT_AUTO;
	css_fixed height = 0;
	css_unit hunit = CSS_UNIT_PX;
	গণনা css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool child_has_height = false;

	প্রতিজ্ঞা(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	/* check if the widths have already been calculated */
	যদি (block->max_width != UNKNOWN_MAX_WIDTH)
		ফেরত;

	যদি (block->style != ফাঁকা) {
		wtype = css_computed_width(block->style, &width, &wunit);
		htype = css_computed_height(block->style, &height, &hunit);
		bs = css_computed_box_sizing(block->style);
	}

	/* set whether the minimum width is of any interest for this box */
	যদি (((block->parent && lh__box_is_float_box(block->parent)) ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX) &&
			wtype != CSS_WIDTH_SET) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} অন্যথা যদি (block->type == BOX_TABLE_CELL) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} অন্যথা যদি (block->parent && (block->parent->flags & NEED_MIN) &&
			wtype != CSS_WIDTH_SET) {
		/* box inside shrink-to-fit context; need minimum width */
		block->flags |= NEED_MIN;
	} অন্যথা যদি (block->parent && (block->parent->type == BOX_FLEX)) {
		/* box is flex item */
		block->flags |= NEED_MIN;
	}

	যদি (block->gadget && (block->gadget->type == GADGET_TEXTBOX ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_FILE ||
			block->gadget->type == GADGET_TEXTAREA) &&
			block->style && wtype == CSS_WIDTH_AUTO) {
		css_fixed আকার_2 = INTTOFIX(10);
		css_unit unit = CSS_UNIT_EM;

		min = max = FIXTOINT(css_unit_len2device_px(block->style,
				&content->unit_len_ctx, আকার_2, unit));

		block->flags |= HAS_HEIGHT;
	}

	যদি (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX) &&
			block->style && wtype == CSS_WIDTH_AUTO) {
		css_fixed আকার_2 = INTTOFIX(1);
		css_unit unit = CSS_UNIT_EM;

		/* form checkbox or radio button
		 * if width is AUTO, set it to 1em */
		min = max = FIXTOINT(css_unit_len2device_px(block->style,
				&content->unit_len_ctx, আকার_2, unit));

		block->flags |= HAS_HEIGHT;
	}

	যদি (block->object) {
		যদি (content_get_type(block->object) == CONTENT_HTML) {
			layout_minmax_block(html_get_box_বৃক্ষ(block->object),
					font_func, content);
			min = html_get_box_বৃক্ষ(block->object)->min_width;
			max = html_get_box_বৃক্ষ(block->object)->max_width;
		} অন্যথা {
			min = max = content_get_width(block->object);
		}

		block->flags |= HAS_HEIGHT;
	} অন্যথা যদি (block->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		block->flags |= HAS_HEIGHT;
	} অন্যথা {
		/* recurse through children */
		জন্য (child = block->children; child; child = child->পরবর্তী) {
			নির্বাচন (child->type) {
			ক্ষেত্র BOX_FLEX:
			ক্ষেত্র BOX_BLOCK:
				layout_minmax_block(child, font_func,
						content);
				যদি (child->flags & HAS_HEIGHT)
					child_has_height = true;
				বিরতি;
			ক্ষেত্র BOX_INLINE_CONTAINER:
				যদি (block->flags & NEED_MIN)
					child->flags |= NEED_MIN;

				layout_minmax_inline_container(child,
						&child_has_height, font_func,
						content);
				যদি (child_has_height &&
						child ==
						child->parent->children) {
					block->flags |= MAKE_HEIGHT;
				}
				বিরতি;
			ক্ষেত্র BOX_TABLE:
				layout_minmax_table(child, font_func,
						content);
				/* todo: fix for zero height tables */
				child_has_height = true;
				child->flags |= MAKE_HEIGHT;
				বিরতি;
			পূর্বনির্ধারিত:
				প্রতিজ্ঞা(0);
			}
			প্রতিজ্ঞা(child->max_width != UNKNOWN_MAX_WIDTH);

			যদি (child->style &&
					(css_computed_position(child->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(child->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				চালু;
			}

			যদি (lh__box_is_flex_container(block) &&
			    lh__flex_main_is_horizontal(block)) {
				যদি (block->style != ফাঁকা &&
				    css_computed_flex_wrap(block->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += child->min_width;
				} অন্যথা {
					যদি (min < child->min_width)
						min = child->min_width;
				}
				max += child->max_width;

			} অন্যথা {
				যদি (min < child->min_width)
					min = child->min_width;
				যদি (max < child->max_width)
					max = child->max_width;
			}

			যদি (child_has_height)
				block->flags |= HAS_HEIGHT;
		}
	}

	যদি (max < min) {
		box_dump(stderr, block, 0, true);
		প্রতিজ্ঞা(0);
	}

	/* fixed width takes priority */
	যদি (block->type != BOX_TABLE_CELL && !lh__box_is_flex_item(block)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		গণনা css_max_width_e max_type;
		গণনা css_min_width_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed মান = 0;
		পূর্ণসংখ্যা width;

		যদি (css_computed_width_px(block->style, &content->unit_len_ctx,
				-1, &width) == CSS_WIDTH_SET) {
			min = max = width;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_width(block->style, &মান, &unit);
		যদি (min_type == CSS_MIN_WIDTH_SET && unit != CSS_UNIT_PCT) {
			পূর্ণসংখ্যা val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, মান, unit));

			যদি (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_width(block->style, &মান, &unit);
		যদি (max_type == CSS_MAX_WIDTH_SET && unit != CSS_UNIT_PCT) {
			পূর্ণসংখ্যা val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, মান, unit));

			যদি (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	যদি (htype == CSS_HEIGHT_SET && hunit != CSS_UNIT_PCT &&
			height > INTTOFIX(0)) {
		block->flags |= MAKE_HEIGHT;
		block->flags |= HAS_HEIGHT;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_width(&content->unit_len_ctx, block->style, বাম_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, ডান_2,
			false, true, true, &extra_fixed, &extra_frac);

	যদি (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	যদি (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	যদি (max < min) {
		min = max;
	}

	calculate_mbp_width(&content->unit_len_ctx, block->style, বাম_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, ডান_2,
			true, false, false, &extra_fixed, &extra_frac);

	যদি (extra_fixed < 0)
		extra_fixed = 0;
	যদি (extra_frac < 0)
		extra_frac = 0;
	যদি (1.0 <= extra_frac)
		extra_frac = 0.9;
	যদি (block->style != ফাঁকা &&
			(css_computed_float(block->style) == CSS_FLOAT_বাম ||
			css_computed_float(block->style) == CSS_FLOAT_ডান)) {
		/* floated boxs */
		block->min_width = min + extra_fixed;
		block->max_width = max + extra_fixed;
	} অন্যথা {
		/* not floated */
		block->min_width = (min + extra_fixed) / (1.0 - extra_frac);
		block->max_width = (max + extra_fixed) / (1.0 - extra_frac);
	}

	প্রতিজ্ঞা(0 <= block->min_width);
	প্রতিজ্ঞা(block->min_width <= block->max_width);
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
স্থির কাঠামো box*
layout_পরবর্তী_margin_block(ধ্রুবক css_unit_ctx *unit_len_ctx,
			 কাঠামো box *box,
			 কাঠামো box *block,
			 পূর্ণসংখ্যা viewport_height,
			 পূর্ণসংখ্যা *max_pos_margin,
			 পূর্ণসংখ্যা *max_neg_margin)
{
	প্রতিজ্ঞা(block != ফাঁকা);

	যখন (box != ফাঁকা) {

		যদি (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			যদি (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						ফাঁকা, ফাঁকা, ফাঁকা, ফাঁকা,
						ফাঁকা, ফাঁকা, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				যদি (*max_pos_margin < box->margin[শীর্ষ_2])
					*max_pos_margin = box->margin[শীর্ষ_2];
				অন্যথা যদি (*max_neg_margin < -box->margin[শীর্ষ_2])
					*max_neg_margin = -box->margin[শীর্ষ_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			যদি (box->flags & MAKE_HEIGHT ||
					box->border[শীর্ষ_2].width ||
					box->padding[শীর্ষ_2] ||
					(box->style &&
					css_computed_overflow_y(box->style) !=
					CSS_OVERFLOW_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_first_child(box))) {
				/* Collapse to this box; return it */
				ফেরত box;
			}
		}


		/* Find next box */
		যদি (box->type == BOX_BLOCK && !box->object && box->children &&
				box->style &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} অন্যথা {
			যদি (!box->পরবর্তী) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				করো {
					/* Apply bottom margin */
					যদি (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					অন্যথা যদি (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->parent;
				} যখন (box != block && !box->পরবর্তী);

				যদি (box == block) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					ফেরত block;
				}
			}

			/* Apply bottom margin */
			যদি (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			অন্যথা যদি (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->পরবর্তী;

			/* Get margins */
			যদি (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						ফাঁকা, ফাঁকা, ফাঁকা, ফাঁকা,
						ফাঁকা, ফাঁকা, box->margin,
						box->padding, box->border);
			}
		}
	}

	ফেরত ফাঁকা;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
স্থির পূর্ণসংখ্যা layout_clear(কাঠামো box *fl, গণনা css_clear_e clear)
{
	পূর্ণসংখ্যা y = 0;
	জন্য (; fl; fl = fl->পরবর্তী_float) {
		যদি ((clear == CSS_CLEAR_বাম || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_বাম)
			যদি (y < fl->y + fl->height)
				y = fl->y + fl->height;
		যদি ((clear == CSS_CLEAR_ডান || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ডান)
			যদি (y < fl->y + fl->height)
				y = fl->y + fl->height;
	}
	ফেরত y;
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
স্থির শূন্য
find_sides(কাঠামো box *fl,
	   পূর্ণসংখ্যা y0, পূর্ণসংখ্যা y1,
	   পূর্ণসংখ্যা *x0, পূর্ণসংখ্যা *x1,
	   কাঠামো box **বাম,
	   কাঠামো box **ডান)
{
	পূর্ণসংখ্যা fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*বাম = *ডান = 0;
	জন্য (; fl; fl = fl->পরবর্তী_float) {
		fy1 = fl->y + fl->height;
		যদি (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			ফেরত;
		}
		fy0 = fl->y;
		যদি (y0 < fy1 && fy0 <= y1) {
			যদি (fl->type == BOX_FLOAT_বাম) {
				fx1 = fl->x + fl->width;
				যদি (*x0 < fx1) {
					*x0 = fx1;
					*বাম = fl;
				}
			} অন্যথা {
				fx0 = fl->x;
				যদি (fx0 < *x1) {
					*x1 = fx0;
					*ডান = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *বাম, *ডান);
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
স্থির পূর্ণসংখ্যা
layout_solve_width(কাঠামো box *box,
		   পূর্ণসংখ্যা available_width,
		   পূর্ণসংখ্যা width,
		   পূর্ণসংখ্যা lm,
		   পূর্ণসংখ্যা rm,
		   পূর্ণসংখ্যা max_width,
		   পূর্ণসংখ্যা min_width)
{
	bool auto_width = false;

	/* Increase specified left/right margins */
	যদি (box->margin[বাম_2] != AUTO && box->margin[বাম_2] < lm &&
			box->margin[বাম_2] >= 0)
		box->margin[বাম_2] = lm;
	যদি (box->margin[ডান_2] != AUTO && box->margin[ডান_2] < rm &&
			box->margin[ডান_2] >= 0)
		box->margin[ডান_2] = rm;

	/* Find width */
	যদি (width == AUTO) {
		পূর্ণসংখ্যা margin_বাম = box->margin[বাম_2];
		পূর্ণসংখ্যা margin_ডান = box->margin[ডান_2];

		যদি (margin_বাম == AUTO) {
			margin_বাম = lm;
		}
		যদি (margin_ডান == AUTO) {
			margin_ডান = rm;
		}

		width = available_width -
				(margin_বাম + box->border[বাম_2].width +
				box->padding[বাম_2] + box->padding[ডান_2] +
				box->border[ডান_2].width + margin_ডান);
		width = width < 0 ? 0 : width;
		auto_width = true;
	}

	যদি (max_width >= 0 && width > max_width) {
		/* max-width is admissable and width exceeds max-width */
		width = max_width;
		auto_width = false;
	}

	যদি (min_width > 0 && width < min_width) {
		/* min-width is admissable and width is less than max-width */
		width = min_width;
		auto_width = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	যদি (auto_width) {
		/* any other 'auto' become 0 or the minimum required values */
		যদি (box->margin[বাম_2] == AUTO) {
			box->margin[বাম_2] = lm;
		}
		যদি (box->margin[ডান_2] == AUTO) {
			box->margin[ডান_2] = rm;
		}
		ফেরত width;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	যদি (box->margin[বাম_2] != AUTO && box->margin[ডান_2] != AUTO &&
			box->parent != ফাঁকা && box->parent->style != ফাঁকা) {
		নির্বাচন (css_computed_text_align(box->parent->style)) {
		ক্ষেত্র CSS_TEXT_ALIGN_LIBCSS_ডান:
			box->margin[বাম_2] = AUTO;
			box->margin[ডান_2] = 0;
			বিরতি;
		ক্ষেত্র CSS_TEXT_ALIGN_LIBCSS_CENTER:
			box->margin[বাম_2] = box->margin[ডান_2] = AUTO;
			বিরতি;
		ক্ষেত্র CSS_TEXT_ALIGN_LIBCSS_বাম:
			box->margin[বাম_2] = 0;
			box->margin[ডান_2] = AUTO;
			বিরতি;
		পূর্বনির্ধারিত:
			/* Leave it alone; no HTML alignment */
			বিরতি;
		}
	}

	যদি (box->margin[বাম_2] == AUTO && box->margin[ডান_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[বাম_2] = box->margin[ডান_2] =
				(available_width - lm - rm -
				(box->border[বাম_2].width + box->padding[বাম_2] +
				width + box->padding[ডান_2] +
				box->border[ডান_2].width)) / 2;

		যদি (box->margin[বাম_2] < 0) {
			box->margin[ডান_2] += box->margin[বাম_2];
			box->margin[বাম_2] = 0;
		}

		box->margin[বাম_2] += lm;

	} অন্যথা যদি (box->margin[বাম_2] == AUTO) {
		box->margin[বাম_2] = available_width - lm -
				(box->border[বাম_2].width + box->padding[বাম_2] +
				width + box->padding[ডান_2] +
				box->border[ডান_2].width + box->margin[ডান_2]);
		box->margin[বাম_2] = box->margin[বাম_2] < lm
				? lm : box->margin[বাম_2];
	} অন্যথা {
		/* margin-right auto or "over-constrained" */
		box->margin[ডান_2] = available_width - rm -
				(box->margin[বাম_2] + box->border[বাম_2].width +
				 box->padding[বাম_2] + width +
				 box->padding[ডান_2] +
				 box->border[ডান_2].width);
	}

	ফেরত width;
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
স্থির শূন্য
layout_block_find_dimensions(ধ্রুবক css_unit_ctx *unit_len_ctx,
			     পূর্ণসংখ্যা available_width,
			     পূর্ণসংখ্যা viewport_height,
			     পূর্ণসংখ্যা lm,
			     পূর্ণসংখ্যা rm,
			     কাঠামো box *box)
{
	পূর্ণসংখ্যা width, max_width, min_width;
	পূর্ণসংখ্যা height, max_height, min_height;
	পূর্ণসংখ্যা *margin = box->margin;
	পূর্ণসংখ্যা *padding = box->padding;
	কাঠামো box_border *border = box->border;
	ধ্রুবক css_computed_style *style = box->style;

	layout_find_dimensions(unit_len_ctx, available_width, viewport_height, box,
			style, &width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	যদি (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	}

	box->width = layout_solve_width(box, available_width, width, lm, rm,
			max_width, min_width);
	box->height = height;

	যদি (margin[শীর্ষ_2] == AUTO)
		margin[শীর্ষ_2] = 0;
	যদি (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
স্থির শূন্য layout_block_add_scrollbar(কাঠামো box *box, পূর্ণসংখ্যা which)
{
	গণনা css_overflow_e overflow_x, overflow_y;

	প্রতিজ্ঞা(box->type == BOX_BLOCK && (which == ডান_2 || which == BOTTOM));

	যদি (box->style == ফাঁকা)
		ফেরত;

	overflow_x = css_computed_overflow_x(box->style);
	overflow_y = css_computed_overflow_y(box->style);

	যদি (which == BOTTOM &&
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		যদি (box->height != AUTO &&
				(overflow_x == CSS_OVERFLOW_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_WIDTH;
		}

	} অন্যথা যদি (which == ডান_2 &&
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		গণনা css_height_e htype;
		css_fixed height = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_height(box->style, &height, &hunit);

		যদি (which == ডান_2 && box->width != AUTO &&
				htype == CSS_HEIGHT_SET &&
				(overflow_y == CSS_OVERFLOW_SCROLL ||
				box_vscrollbar_present(box))) {
			box->width -= SCROLLBAR_WIDTH;
			box->padding[ডান_2] += SCROLLBAR_WIDTH;
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
স্থির শূন্য layout_move_children(কাঠামো box *box, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	প্রতিজ্ঞা(box);

	জন্য (box = box->children; box; box = box->পরবর্তী) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_table(
		কাঠামো box *table,
		পূর্ণসংখ্যা available_width,
		html_content *content)
{
	unsigned পূর্ণসংখ্যা columns = table->columns;  /* total columns */
	unsigned পূর্ণসংখ্যা i;
	unsigned পূর্ণসংখ্যা *row_span;
	পূর্ণসংখ্যা *excess_y;
	পূর্ণসংখ্যা table_width, min_width = 0, max_width = 0;
	পূর্ণসংখ্যা required_width = 0;
	পূর্ণসংখ্যা x, remainder = 0, গুনতি = 0;
	পূর্ণসংখ্যা table_height = 0;
	পূর্ণসংখ্যা min_height = 0;
	পূর্ণসংখ্যা *xs;  /* array of column x positions */
	পূর্ণসংখ্যা auto_width;
	পূর্ণসংখ্যা spare_width;
	পূর্ণসংখ্যা relative_যোগ = 0;
	পূর্ণসংখ্যা border_spacing_h = 0, border_spacing_v = 0;
	পূর্ণসংখ্যা spare_height;
	পূর্ণসংখ্যা positioned_columns = 0;
	কাঠামো box *containing_block = ফাঁকা;
	কাঠামো box *c;
	কাঠামো box *row;
	কাঠামো box *row_group;
	কাঠামো box **row_span_cell;
	কাঠামো column *col;
	ধ্রুবক css_computed_style *style = table->style;
	গণনা css_height_e htype;
	css_fixed মান = 0;
	css_unit unit = CSS_UNIT_PX;

	প্রতিজ্ঞা(table->type == BOX_TABLE);
	প্রতিজ্ঞা(style);
	প্রতিজ্ঞা(table->children && table->children->children);
	প্রতিজ্ঞা(columns);

	/* allocate working buffers */
	col = malloc(columns * আকার col[0]);
	excess_y = malloc(columns * আকার excess_y[0]);
	row_span = malloc(columns * আকার row_span[0]);
	row_span_cell = malloc(columns * আকার row_span_cell[0]);
	xs = malloc((columns + 1) * আকার xs[0]);
	যদি (!col || !xs || !row_span || !excess_y || !row_span_cell) {
		মুক্ত(col);
		মুক্ত(excess_y);
		মুক্ত(row_span);
		মুক্ত(row_span_cell);
		মুক্ত(xs);
		ফেরত false;
	}

	memcpy(col, table->col, আকার(col[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1, table,
			style, 0, 0, 0, 0, 0, 0, table->margin, table->padding,
			table->border);
	জন্য (row_group = table->children; row_group;
			row_group = row_group->পরবর্তী) {
		জন্য (row = row_group->children; row; row = row->পরবর্তী) {
			জন্য (c = row->children; c; c = c->পরবর্তী) {
				গণনা css_overflow_e overflow_x;
				গণনা css_overflow_e overflow_y;

				প্রতিজ্ঞা(c->style);
				table_used_border_for_cell(
						&content->unit_len_ctx, c);
				layout_find_dimensions(&content->unit_len_ctx,
						available_width, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				overflow_x = css_computed_overflow_x(c->style);
				overflow_y = css_computed_overflow_y(c->style);

				যদি (overflow_x == CSS_OVERFLOW_SCROLL ||
						overflow_x ==
						CSS_OVERFLOW_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_WIDTH;
				}
				যদি (overflow_y == CSS_OVERFLOW_SCROLL ||
						overflow_y ==
						CSS_OVERFLOW_AUTO) {
					c->padding[ডান_2] += SCROLLBAR_WIDTH;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	যদি (css_computed_border_collapse(style) ==
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
	যদি (css_computed_width_px(style, &content->unit_len_ctx,
			available_width, &table_width) == CSS_WIDTH_SET) {
		/* specified width includes border */
		table_width -= table->border[বাম_2].width +
				table->border[ডান_2].width;
		table_width = table_width < 0 ? 0 : table_width;

		auto_width = table_width;
	} অন্যথা {
		table_width = AUTO;
		auto_width = available_width -
				((table->margin[বাম_2] == AUTO ? 0 :
						table->margin[বাম_2]) +
				 table->border[বাম_2].width +
				 table->padding[বাম_2] +
				 table->padding[ডান_2] +
				 table->border[ডান_2].width +
				 (table->margin[ডান_2] == AUTO ? 0 :
						table->margin[ডান_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_height(style, &মান, &unit);
	যদি (htype == CSS_HEIGHT_SET) {
		যদি (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			যদি (css_computed_position(table->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				প্রতিজ্ঞা(table->float_container);
				containing_block = table->float_container;
			} অন্যথা যদি (table->float_container &&
					css_computed_position(table->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(table->style) ==
						CSS_FLOAT_বাম ||
					css_computed_float(table->style) ==
						CSS_FLOAT_ডান)) {
				/* Table is a float */
				প্রতিজ্ঞা(table->parent && table->parent->parent &&
						table->parent->parent->parent);
				containing_block =
						table->parent->parent->parent;
			} অন্যথা যদি (table->parent && table->parent->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_block = table->parent;
			} অন্যথা যদি (table->parent && table->parent->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				প্রতিজ্ঞা(table->parent->parent);
				containing_block = table->parent->parent;
			}

			যদি (containing_block) {
				css_fixed ignored = 0;

				htype = css_computed_height(
						containing_block->style,
						&ignored, &unit);
			}

			যদি (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(table->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_HEIGHT_SET)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_height = FPCT_OF_INT_TOINT(মান,
						containing_block->height);
			}
		} অন্যথা {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_height = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					মান, unit));
		}
	}

	/* calculate width required by cells */
	জন্য (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      table,
		      i,
		      ((ধ্রুবক অক্ষর *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[col[i].type],
		      col[i].width,
		      col[i].min,
		      col[i].max);


		যদি (col[i].positioned) {
			positioned_columns++;
			চালু;
		} অন্যথা যদি (col[i].type == COLUMN_WIDTH_FIXED) {
			যদি (col[i].width < col[i].min)
				col[i].width = col[i].max = col[i].min;
			অন্যথা
				col[i].min = col[i].max = col[i].width;
			required_width += col[i].width;
		} অন্যথা যদি (col[i].type == COLUMN_WIDTH_PERCENT) {
			পূর্ণসংখ্যা width = col[i].width * auto_width / 100;
			required_width += col[i].min < width ? width :
					col[i].min;
		} অন্যথা
			required_width += col[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_width);
	}
	required_width += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", table_width,
	      table->min_width, table->max_width, auto_width, required_width);

	যদি (auto_width < required_width) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		জন্য (i = 0; i != columns; i++) {
			যদি (col[i].type == COLUMN_WIDTH_RELATIVE)
				চালু;
			যদি (col[i].type == COLUMN_WIDTH_PERCENT) {
				col[i].max = auto_width * col[i].width / 100;
				যদি (col[i].max < col[i].min)
					col[i].max = col[i].min;
			}
			min_width += col[i].min;
			max_width += col[i].max;
		}
	} অন্যথা {
		/* take percentages exactly */
		জন্য (i = 0; i != columns; i++) {
			যদি (col[i].type == COLUMN_WIDTH_RELATIVE)
				চালু;
			যদি (col[i].type == COLUMN_WIDTH_PERCENT) {
				পূর্ণসংখ্যা width = auto_width * col[i].width / 100;
				যদি (width < col[i].min)
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
	জন্য (i = 0; i != columns; i++) {
		যদি (col[i].type == COLUMN_WIDTH_RELATIVE)
			relative_যোগ += col[i].width;
		অন্যথা যদি (col[i].type == COLUMN_WIDTH_FIXED)
			spare_width -= col[i].width;
		অন্যথা
			spare_width -= col[i].min;
	}
	spare_width -= (columns + 1) * border_spacing_h;
	যদি (relative_যোগ != 0) {
		যদি (spare_width < 0)
			spare_width = 0;
		জন্য (i = 0; i != columns; i++) {
			যদি (col[i].type == COLUMN_WIDTH_RELATIVE) {
				col[i].min = ceil(col[i].max =
						(দশমিক) spare_width
						* (দশমিক) col[i].width
						/ relative_যোগ);
				min_width += col[i].min;
				max_width += col[i].max;
			}
		}
	}
	min_width += (columns + 1) * border_spacing_h;
	max_width += (columns + 1) * border_spacing_h;

	যদি (auto_width <= min_width) {
		/* not enough space: minimise column widths */
		জন্য (i = 0; i < columns; i++) {
			col[i].width = col[i].min;
		}
		table_width = min_width;
	} অন্যথা যদি (max_width <= auto_width) {
		/* more space than maximum width */
		যদি (table_width == AUTO) {
			/* for auto-width tables, make columns max width */
			জন্য (i = 0; i < columns; i++) {
				col[i].width = col[i].max;
			}
			table_width = max_width;
		} অন্যথা {
			/* for fixed-width tables, distribute the extra space
			 * too */
			unsigned পূর্ণসংখ্যা flexible_columns = 0;
			জন্য (i = 0; i != columns; i++)
				যদি (col[i].type != COLUMN_WIDTH_FIXED)
					flexible_columns++;
			যদি (flexible_columns == 0) {
				পূর্ণসংখ্যা extra = (table_width - max_width) / columns;
				remainder = (table_width - max_width) -
						(extra * columns);
				জন্য (i = 0; i != columns; i++) {
					col[i].width = col[i].max + extra;
					গুনতি -= remainder;
					যদি (গুনতি < 0) {
						col[i].width++;
						গুনতি += columns;
					}
				}

			} অন্যথা {
				পূর্ণসংখ্যা extra = (table_width - max_width) /
						flexible_columns;
				remainder = (table_width - max_width) -
						(extra * flexible_columns);
				জন্য (i = 0; i != columns; i++)
					যদি (col[i].type != COLUMN_WIDTH_FIXED) {
						col[i].width = col[i].max +
								extra;
						গুনতি -= remainder;
						যদি (গুনতি < 0) {
							col[i].width++;
							গুনতি += flexible_columns;
						}
					}
			}
		}
	} অন্যথা {
		/* space between min and max: fill it exactly */
		দশমিক scale = (দশমিক) (auto_width - min_width) /
				(দশমিক) (max_width - min_width);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		জন্য (i = 0; i < columns; i++) {
			col[i].width = col[i].min + (পূর্ণসংখ্যা) (0.5 +
					(col[i].max - col[i].min) * scale);
		}
		table_width = auto_width;
	}

	xs[0] = x = border_spacing_h;
	জন্য (i = 0; i != columns; i++) {
		যদি (!col[i].positioned)
			x += col[i].width + border_spacing_h;
		xs[i + 1] = x;
		row_span[i] = 0;
		excess_y[i] = 0;
		row_span_cell[i] = 0;
	}

	/* position cells */
	table_height = border_spacing_v;
	জন্য (row_group = table->children; row_group;
			row_group = row_group->পরবর্তী) {
		পূর্ণসংখ্যা row_group_height = 0;
		জন্য (row = row_group->children; row; row = row->পরবর্তী) {
			পূর্ণসংখ্যা row_height = 0;

			htype = css_computed_height(row->style, &মান, &unit);
			যদি (htype == CSS_HEIGHT_SET && unit != CSS_UNIT_PCT) {
				row_height = FIXTOINT(css_unit_len2device_px(
						row->style,
						&content->unit_len_ctx,
						মান, unit));
			}
			জন্য (c = row->children; c; c = c->পরবর্তী) {
				প্রতিজ্ঞা(c->style);
				c->width = xs[c->start_column + c->columns] -
						xs[c->start_column] -
						border_spacing_h -
						c->border[বাম_2].width -
						c->padding[বাম_2] -
						c->padding[ডান_2] -
						c->border[ডান_2].width;
				c->float_children = 0;
				c->cached_place_below_level = 0;

				c->height = AUTO;
				যদি (!layout_block_context(c, -1, content)) {
					মুক্ত(col);
					মুক্ত(excess_y);
					মুক্ত(row_span);
					মুক্ত(row_span_cell);
					মুক্ত(xs);
					ফেরত false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->height;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_height(c->style,
						&মান, &unit);

				যদি (htype == CSS_HEIGHT_SET &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					পূর্ণসংখ্যা h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							মান, unit));
					যদি (c->height < h)
						c->height = h;
				}
				/* specified row height is treated as a minimum
				 */
				যদি (c->height < row_height)
					c->height = row_height;
				c->x = xs[c->start_column] +
						c->border[বাম_2].width;
				c->y = c->border[শীর্ষ_2].width;
				জন্য (i = 0; i != c->columns; i++) {
					row_span[c->start_column + i] = c->rows;
					excess_y[c->start_column + i] =
							c->border[শীর্ষ_2].width +
							c->padding[শীর্ষ_2] +
							c->height +
							c->padding[BOTTOM] +
							c->border[BOTTOM].width;
					row_span_cell[c->start_column + i] = 0;
				}
				row_span_cell[c->start_column] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[শীর্ষ_2].width -
						c->padding[শীর্ষ_2] -
						c->height -
						c->border[BOTTOM].width;
			}
			জন্য (i = 0; i != columns; i++)
				যদি (row_span[i] != 0)
					row_span[i]--;
				অন্যথা
					row_span_cell[i] = 0;
			যদি (row->পরবর্তী || row_group->পরবর্তী) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				জন্য (i = 0; i != columns; i++)
					যদি (row_span[i] == 0 && row_height <
							excess_y[i])
						row_height = excess_y[i];
			} অন্যথা {
				/* except in the last row */
				জন্য (i = 0; i != columns; i++)
					যদি (row_height < excess_y[i])
						row_height = excess_y[i];
			}
			জন্য (i = 0; i != columns; i++) {
				যদি (row_height < excess_y[i])
					excess_y[i] -= row_height;
				অন্যথা
					excess_y[i] = 0;
				যদি (row_span_cell[i] != 0)
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
	জন্য (row_group = table->children; row_group;
			row_group = row_group->পরবর্তী) {
		জন্য (row = row_group->children; row; row = row->পরবর্তী) {
			জন্য (c = row->children; c; c = c->পরবর্তী) {
				গণনা css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				spare_height = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->height - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &মান, &unit);

				নির্বাচন (vertical_align) {
				ক্ষেত্র CSS_VERTICAL_ALIGN_SUB:
				ক্ষেত্র CSS_VERTICAL_ALIGN_SUPER:
				ক্ষেত্র CSS_VERTICAL_ALIGN_TEXT_শীর্ষ:
				ক্ষেত্র CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				ক্ষেত্র CSS_VERTICAL_ALIGN_SET:
				ক্ষেত্র CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				ক্ষেত্র CSS_VERTICAL_ALIGN_শীর্ষ:
					বিরতি;
				ক্ষেত্র CSS_VERTICAL_ALIGN_মধ্য:
					c->padding[শীর্ষ_2] += spare_height / 2;
					c->padding[BOTTOM] -= spare_height / 2;
					layout_move_children(c, 0,
							spare_height / 2);
					বিরতি;
				ক্ষেত্র CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[শীর্ষ_2] += spare_height;
					c->padding[BOTTOM] -= spare_height;
					layout_move_children(c, 0,
							spare_height);
					বিরতি;
				ক্ষেত্র CSS_VERTICAL_ALIGN_INHERIT:
					প্রতিজ্ঞা(0);
					বিরতি;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	যদি (table->margin[শীর্ষ_2] == AUTO)
		table->margin[শীর্ষ_2] = 0;
	যদি (table->margin[BOTTOM] == AUTO)
		table->margin[BOTTOM] = 0;

	মুক্ত(col);
	মুক্ত(excess_y);
	মুক্ত(row_span);
	মুক্ত(row_span_cell);
	মুক্ত(xs);

	table->width = table_width;
	table->height = table_height;

	ফেরত true;
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
স্থির bool layout_apply_minmax_height(
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		কাঠামো box *box,
		কাঠামো box *container)
{
	পূর্ণসংখ্যা h;
	কাঠামো box *containing_block = ফাঁকা;
	bool updated = false;

	/* Find containing block for percentage heights */
	যদি (box->style != ফাঁকা && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		প্রতিজ্ঞা(container);
		containing_block = container;
	} অন্যথা যদি (box->float_container && box->style != ফাঁকা &&
			(css_computed_float(box->style) == CSS_FLOAT_বাম ||
			 css_computed_float(box->style) == CSS_FLOAT_ডান)) {
		/* Box is a float */
		প্রতিজ্ঞা(box->parent && box->parent->parent &&
				box->parent->parent->parent);
		containing_block = box->parent->parent->parent;
	} অন্যথা যদি (box->parent && box->parent->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_block = box->parent;
	} অন্যথা যদি (box->parent && box->parent->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		প্রতিজ্ঞা(box->parent->parent);
		containing_block = box->parent->parent;
	}

	যদি (box->style) {
		গণনা css_height_e htype = CSS_HEIGHT_AUTO;
		css_fixed মান = 0;
		css_unit unit = CSS_UNIT_PX;

		যদি (containing_block) {
			htype = css_computed_height(containing_block->style,
					&মান, &unit);
		}

		/* max-height */
		যদি (css_computed_max_height(box->style, &মান, &unit) ==
				CSS_MAX_HEIGHT_SET) {
			যদি (unit == CSS_UNIT_PCT) {
				যদি (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(মান,
						containing_block->height);
					যদি (h < box->height) {
						box->height = h;
						updated = true;
					}
				}
			} অন্যথা {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						মান, unit));
				যদি (h < box->height) {
					box->height = h;
					updated = true;
				}
			}
		}

		/* min-height */
		যদি (ns_computed_min_height(box->style, &মান, &unit) ==
				CSS_MIN_HEIGHT_SET) {
			যদি (unit == CSS_UNIT_PCT) {
				যদি (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(মান,
						containing_block->height);
					যদি (h > box->height) {
						box->height = h;
						updated = true;
					}
				}
			} অন্যথা {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						মান, unit));
				যদি (h > box->height) {
					box->height = h;
					updated = true;
				}
			}
		}
	}
	ফেরত updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
স্থির bool layout_block_object(কাঠামো box *block)
{
	প্রতিজ্ঞা(block);
	প্রতিজ্ঞা(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_TABLE ||
			block->type == BOX_TABLE_CELL);
	প্রতিজ্ঞা(block->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", block,
	      hlcache_handle_get_url(block->object), block->width);

	যদি (content_can_reformat(block->object)) {
		content_reformat(block->object, false, block->width, 1);
	} অন্যথা {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	ফেরত true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
স্থির শূন্য add_float_to_container(কাঠামো box *cont, কাঠামো box *b)
{
	কাঠামো box *box = cont->float_children;
	পূর্ণসংখ্যা b_bottom = b->y + b->height;

	প্রতিজ্ঞা(b->type == BOX_FLOAT_বাম || b->type == BOX_FLOAT_ডান);

	যদি (box == ফাঁকা) {
		/* No other float children */
		b->পরবর্তী_float = ফাঁকা;
		cont->float_children = b;
		ফেরত;
	} অন্যথা যদি (b_bottom >= box->y + box->height) {
		/* Goes at start of list */
		b->পরবর্তী_float = cont->float_children;
		cont->float_children = b;
	} অন্যথা {
		কাঠামো box *পূর্ব = ফাঁকা;
		যখন (box != ফাঁকা && b_bottom < box->y + box->height) {
			পূর্ব = box;
			box = box->পরবর্তী_float;
		}
		যদি (পূর্ব != ফাঁকা) {
			b->পরবর্তী_float = পূর্ব->পরবর্তী_float;
			পূর্ব->পরবর্তী_float = b;
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
স্থির bool
layout_text_box_split(html_content *content,
		      plot_font_style_t *fstyle,
		      কাঠামো box *split_box,
		      size_t new_দৈর্ঘ্য,
		      পূর্ণসংখ্যা new_width)
{
	পূর্ণসংখ্যা space_width = split_box->space;
	কাঠামো box *c2;
	ধ্রুবক কাঠামো gui_layout_table *font_func = content->font_func;
	bool space = (split_box->text[new_দৈর্ঘ্য] == ' ');
	পূর্ণসংখ্যা used_দৈর্ঘ্য = new_দৈর্ঘ্য + (space ? 1 : 0);

	যদি ((space && space_width == 0) || space_width == UNKNOWN_WIDTH) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->width(fstyle, " ", 1, &space_width);
	}

	যদি (split_box->space == UNKNOWN_WIDTH)
		split_box->space = space_width;
	যদি (!space)
		space_width = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, split_box, আকার *c2);
	যদি (!c2)
		ফেরত false;
	c2->flags |= CLONE;

	/* Set remaining text in c2 */
	c2->text += used_দৈর্ঘ্য;

	/* Set c2 according to the remaining text */
	c2->width -= new_width + space_width;
	c2->flags &= ~MEASURED; /* width has been estimated */
	c2->দৈর্ঘ্য = split_box->দৈর্ঘ্য - used_দৈর্ঘ্য;

	/* Update split_box for its reduced text */
	split_box->width = new_width;
	split_box->flags |= MEASURED;
	split_box->দৈর্ঘ্য = new_দৈর্ঘ্য;
	split_box->space = space_width;

	/* Insert c2 into box list */
	c2->পরবর্তী = split_box->পরবর্তী;
	split_box->পরবর্তী = c2;
	c2->পূর্ব = split_box;
	যদি (c2->পরবর্তী)
		c2->পরবর্তী->পূর্ব = c2;
	অন্যথা
		c2->parent->last = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      split_box,
	      split_box->দৈর্ঘ্য,
	      (পূর্ণসংখ্যা)split_box->দৈর্ঘ্য,
	      split_box->text);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->দৈর্ঘ্য,
	      (পূর্ণসংখ্যা)c2->দৈর্ঘ্য,
	      c2->text);

	ফেরত true;
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
স্থির শূন্য
layout_float_find_dimensions(
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		পূর্ণসংখ্যা available_width,
		ধ্রুবক css_computed_style *style,
		কাঠামো box *box)
{
	পূর্ণসংখ্যা width, height, max_width, min_width, max_height, min_height;
	পূর্ণসংখ্যা *margin = box->margin;
	পূর্ণসংখ্যা *padding = box->padding;
	কাঠামো box_border *border = box->border;
	গণনা css_overflow_e overflow_x = css_computed_overflow_x(style);
	গণনা css_overflow_e overflow_y = css_computed_overflow_y(style);
	পূর্ণসংখ্যা scrollbar_width_x =
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;
	পূর্ণসংখ্যা scrollbar_width_y =
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;

	layout_find_dimensions(unit_len_ctx, available_width, -1, box, style,
			&width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	যদি (margin[বাম_2] == AUTO)
		margin[বাম_2] = 0;
	যদি (margin[ডান_2] == AUTO)
		margin[ডান_2] = 0;

	যদি (box->gadget == ফাঁকা) {
		padding[ডান_2] += scrollbar_width_y;
		padding[BOTTOM] += scrollbar_width_x;
	}

	যদি (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	} অন্যথা যদি (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed আকার_2 = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		প্রতিজ্ঞা(box->style);

		যদি (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			যদি (width == AUTO) {
				আকার_2 = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						আকার_2, unit));
			}
			যদি (box->gadget->type == GADGET_FILE &&
					height == AUTO) {
				আকার_2 = FLTTOFIX(1.5);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						আকার_2, unit));
			}
		}
		যদি (box->gadget->type == GADGET_TEXTAREA) {
			যদি (width == AUTO) {
				আকার_2 = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						আকার_2, unit));
			}
			যদি (height == AUTO) {
				আকার_2 = INTTOFIX(4);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						আকার_2, unit));
			}
		}
	} অন্যথা যদি (width == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		width = min(max(box->min_width, available_width),
				box->max_width);

		/* width includes margin, borders and padding */
		যদি (width == available_width) {
			width -= box->margin[বাম_2] + box->border[বাম_2].width +
					box->padding[বাম_2] +
					box->padding[ডান_2] +
					box->border[ডান_2].width +
					box->margin[ডান_2];
		} অন্যথা {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			পূর্ণসংখ্যা fixed = 0;
			দশমিক frac = 0;
			calculate_mbp_width(unit_len_ctx, box->style, বাম_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_width(unit_len_ctx, box->style, ডান_2,
					true, true, true, &fixed, &frac);
			যদি (fixed < 0)
				fixed = 0;

			width -= fixed;
		}

		যদি (max_width >= 0 && width > max_width) width = max_width;
		যদি (min_width >  0 && width < min_width) width = min_width;

	} অন্যথা {
		যদি (max_width >= 0 && width > max_width) width = max_width;
		যদি (min_width >  0 && width < min_width) width = min_width;
		width -= scrollbar_width_y;
	}

	box->width = width;
	box->height = height;

	যদি (margin[শীর্ষ_2] == AUTO)
		margin[শীর্ষ_2] = 0;
	যদি (margin[BOTTOM] == AUTO)
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
স্থির bool layout_float(কাঠামো box *b, পূর্ণসংখ্যা width, html_content *content)
{
	প্রতিজ্ঞা(b->type == BOX_TABLE ||
	       b->type == BOX_BLOCK ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_find_dimensions(&content->unit_len_ctx, width, b->style, b);
	যদি (b->type == BOX_TABLE || b->type == BOX_INLINE_FLEX) {
		যদি (b->type == BOX_TABLE) {
			যদি (!layout_table(b, width, content))
				ফেরত false;
		} অন্যথা {
			যদি (!layout_flex(b, width, content))
				ফেরত false;
		}
		যদি (b->margin[বাম_2] == AUTO)
			b->margin[বাম_2] = 0;
		যদি (b->margin[ডান_2] == AUTO)
			b->margin[ডান_2] = 0;
		যদি (b->margin[শীর্ষ_2] == AUTO)
			b->margin[শীর্ষ_2] = 0;
		যদি (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} অন্যথা {
		ফেরত layout_block_context(b, -1, content);
	}
	ফেরত true;
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
স্থির শূন্য
place_float_below(কাঠামো box *c, পূর্ণসংখ্যা width, পূর্ণসংখ্যা cx, পূর্ণসংখ্যা y, কাঠামো box *cont)
{
	পূর্ণসংখ্যা x0, x1, yy;
	কাঠামো box *বাম;
	কাঠামো box *ডান;

	yy = y > cont->cached_place_below_level ?
			y : cont->cached_place_below_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      width, cx, y, cont);

	করো {
		y = yy;
		x0 = cx;
		x1 = cx + width;
		find_sides(cont->float_children, y, y + c->height, &x0, &x1,
				&বাম, &ডান);
		যদি (বাম != 0 && ডান != 0) {
			yy = (বাম->y + বাম->height <
					ডান->y + ডান->height ?
					বাম->y + বাম->height :
					ডান->y + ডান->height);
		} অন্যথা যদি (বাম == 0 && ডান != 0) {
			yy = ডান->y + ডান->height;
		} অন্যথা যদি (বাম != 0 && ডান == 0) {
			yy = বাম->y + বাম->height;
		}
	} যখন ((বাম != 0 || ডান != 0) && (c->width > x1 - x0));

	যদি (c->type == BOX_FLOAT_বাম) {
		c->x = x0;
	} অন্যথা {
		c->x = x1 - c->width;
	}
	c->y = y;
	cont->cached_place_below_level = y;
}


/**
 * Calculate line height from a style.
 */
স্থির পূর্ণসংখ্যা সারি_height(
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		ধ্রুবক css_computed_style *style)
{
	গণনা css_সারি_height_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed সারি_height;

	প্রতিজ্ঞা(style);

	lhtype = css_computed_সারি_height(style, &lhvalue, &lhunit);
	যদি (lhtype == CSS_সারি_HEIGHT_NORMAL) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_সারি_HEIGHT_সংখ্যা;
	}

	যদি (lhtype == CSS_সারি_HEIGHT_সংখ্যা ||
			lhunit == CSS_UNIT_PCT) {
		সারি_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		যদি (lhtype != CSS_সারি_HEIGHT_সংখ্যা)
			সারি_height = FDIV(সারি_height, F_100);
	} অন্যথা {
		প্রতিজ্ঞা(lhunit != CSS_UNIT_PCT);

		সারি_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	ফেরত FIXTOINT(সারি_height);
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
স্থির bool
layout_সারি(কাঠামো box *first,
	    পূর্ণসংখ্যা *width,
	    পূর্ণসংখ্যা *y,
	    পূর্ণসংখ্যা cx,
	    পূর্ণসংখ্যা cy,
	    কাঠামো box *cont,
	    bool indent,
	    bool has_text_children,
	    html_content *content,
	    কাঠামো box **পরবর্তী_box)
{
	পূর্ণসংখ্যা height, used_height;
	পূর্ণসংখ্যা x0 = 0;
	পূর্ণসংখ্যা x1 = *width;
	পূর্ণসংখ্যা x, h, x_previous;
	পূর্ণসংখ্যা fy = cy;
	কাঠামো box *বাম;
	কাঠামো box *ডান;
	কাঠামো box *b;
	কাঠামো box *split_box = 0;
	কাঠামো box *d;
	কাঠামো box *br_box = 0;
	bool move_y = false;
	bool place_below = false;
	পূর্ণসংখ্যা space_before = 0, space_after = 0;
	unsigned পূর্ণসংখ্যা inline_গুনতি = 0;
	unsigned পূর্ণসংখ্যা i;
	ধ্রুবক কাঠামো gui_layout_table *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      first,
	      (পূর্ণসংখ্যা)first->দৈর্ঘ্য,
	      first->text,
	      *width,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	find_sides(cont->float_children, cy, cy, &x0, &x1, &বাম, &ডান);
	x0 -= cx;
	x1 -= cx;

	যদি (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	যদি (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	যদি (has_text_children || first->parent->parent->gadget)
		used_height = height = সারি_height(&content->unit_len_ctx,
				first->parent->parent->style);
	অন্যথা
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		used_height = height = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	জন্য (x = 0, b = first; x <= x1 - x0 && b != 0; b = b->পরবর্তী) {
		পূর্ণসংখ্যা min_width, max_width, min_height, max_height;

		প্রতিজ্ঞা(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		যদি (b->type == BOX_BR)
			বিরতি;

		যদি (lh__box_is_float_box(b))
			চালু;
		যদি (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			চালু;

		প্রতিজ্ঞা(b->style != ফাঁকা);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		যদি (b->type == BOX_INLINE_BLOCK ||
		    b->type == BOX_INLINE_FLEX) {
			যদি (b->max_width != UNKNOWN_WIDTH)
				যদি (!layout_float(b, *width, content))
					ফেরত false;
			h = b->border[শীর্ষ_2].width + b->padding[শীর্ষ_2] + b->height +
					b->padding[BOTTOM] +
					b->border[BOTTOM].width;
			যদি (height < h)
				height = h;
			x += b->margin[বাম_2] + b->border[বাম_2].width +
					b->padding[বাম_2] + b->width +
					b->padding[ডান_2] +
					b->border[ডান_2].width +
					b->margin[ডান_2];
			space_after = 0;
			চালু;
		}

		যদি (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_find_dimensions(&content->unit_len_ctx,
					*width, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			জন্য (i = 0; i != 4; i++)
				যদি (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[বাম_2] + b->border[বাম_2].width +
					b->padding[বাম_2];
			যদি (b->inline_end) {
				b->inline_end->margin[ডান_2] = b->margin[ডান_2];
				b->inline_end->padding[ডান_2] =
						b->padding[ডান_2];
				b->inline_end->border[ডান_2] =
						b->border[ডান_2];
			} অন্যথা {
				x += b->padding[ডান_2] +
						b->border[ডান_2].width +
						b->margin[ডান_2];
			}
		} অন্যথা যদি (b->type == BOX_INLINE_END) {
			b->width = 0;
			যদি (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[ডান_2] + b->border[ডান_2].width +
					b->margin[ডান_2];
			চালু;
		}

		যদি (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->height = সারি_height(&content->unit_len_ctx,
					b->style ? b->style :
					b->parent->parent->style);
			যদি (height < b->height)
				height = b->height;

			যদি (!b->text) {
				b->width = 0;
				space_after = 0;
				চালু;
			}

			যদি (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				যদি (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					পূর্ণসংখ্যা opt_maxwidth = 0;
					কাঠামো form_option *o;

					জন্য (o = b->parent->parent->gadget->
							তথ্য.select.বস্তুসমূহ; o;
							o = o->পরবর্তী) {
						পূর্ণসংখ্যা opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						যদি (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}
					b->width = opt_maxwidth;
					যদি (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;
				} অন্যথা {
					font_func->width(&fstyle, b->text,
							b->দৈর্ঘ্য, &b->width);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			যদি (b->text && (x + b->width < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->পরবর্তী) {
				font_func->width(&fstyle, b->text,
						 b->দৈর্ঘ্য, &b->width);
				b->flags |= MEASURED;
			}

			x += b->width;
			যদি (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			চালু;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		প্রতিজ্ঞা(b->style);

		layout_find_dimensions(&content->unit_len_ctx,
				*width, -1, b, b->style,
				&b->width, &b->height,
				&max_width, &min_width,
				&max_height, &min_height,
				ফাঁকা, ফাঁকা, ফাঁকা);

		যদি (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->width, &b->height,
					min_width, max_width,
					min_height, max_height);
		} অন্যথা যদি (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			যদি (b->width == AUTO)
				b->width = 400;
			যদি (b->height == AUTO)
				b->height = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} অন্যথা {
			/* form control with no object */
			যদি (b->width == AUTO)
				b->width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			যদি (b->height == AUTO)
				b->height = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		যদি (b->object && content_can_reformat(b->object) &&
				b->width !=
				content_get_available_width(b->object)) {
			css_fixed মান = 0;
			css_unit unit = CSS_UNIT_PX;
			গণনা css_height_e htype = css_computed_height(b->style,
					&মান, &unit);

			content_reformat(b->object, false, b->width, b->height);

			যদি (htype == CSS_HEIGHT_AUTO)
				b->height = content_get_height(b->object);
		}

		যদি (height < b->height)
			height = b->height;

		x += b->width;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *width;
	find_sides(cont->float_children, cy, cy + height, &x0, &x1,
			&বাম, &ডান);
	x0 -= cx;
	x1 -= cx;

	যদি (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	যদি (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	জন্য (x = x_previous = 0, b = first; x <= x1 - x0 && b; b = b->পরবর্তী) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		যদি (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} অন্যথা যদি (lh__box_is_inline_flow(b)) {
			প্রতিজ্ঞা(b->width != UNKNOWN_WIDTH);

			x_previous = x;
			x += space_after;
			b->x = x;

			যদি ((b->type == BOX_INLINE && !b->inline_end) ||
					b->type == BOX_INLINE_BLOCK ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[বাম_2] + b->border[বাম_2].width;
				x = b->x + b->padding[বাম_2] + b->width +
						b->padding[ডান_2] +
						b->border[ডান_2].width +
						b->margin[ডান_2];
			} অন্যথা যদি (b->type == BOX_INLINE) {
				b->x += b->margin[বাম_2] + b->border[বাম_2].width;
				x = b->x + b->padding[বাম_2] + b->width;
			} অন্যথা যদি (b->type == BOX_INLINE_END) {
				b->height = b->inline_end->height;
				x += b->padding[ডান_2] +
						b->border[ডান_2].width +
						b->margin[ডান_2];
			} অন্যথা {
				x += b->width;
			}

			space_before = space_after;
			যদি (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			অন্যথা যদি (b->text || b->type == BOX_INLINE_END) {
				যদি (b->space == UNKNOWN_WIDTH) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} অন্যথা {
				space_after = 0;
			}
			split_box = b;
			move_y = true;
			inline_গুনতি++;
		} অন্যথা যদি (b->type == BOX_BR) {
			b->x = x;
			b->width = 0;
			br_box = b;
			b = b->পরবর্তী;
			split_box = 0;
			move_y = true;
			বিরতি;

		} অন্যথা {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_place_below_level = 0;
			b->float_container = d->float_container = cont;

			যদি (!layout_float(d, *width, content))
				ফেরত false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[শীর্ষ_2],
			      d->border[শীর্ষ_2].width);

			d->x = d->margin[বাম_2] + d->border[বাম_2].width;
			d->y = d->margin[শীর্ষ_2] + d->border[শীর্ষ_2].width;
			b->width = d->margin[বাম_2] + d->border[বাম_2].width +
					d->padding[বাম_2] + d->width +
					d->padding[ডান_2] +
					d->border[ডান_2].width +
					d->margin[ডান_2];
			b->height = d->margin[শীর্ষ_2] + d->border[শীর্ষ_2].width +
					d->padding[শীর্ষ_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];

			যদি (b->width > (x1 - x0) - x)
				place_below = true;
			যদি (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_বাম && বাম == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ডান &&
						ডান == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						বাম == 0 && ডান == 0)) &&
					(!place_below ||
					(বাম == 0 && ডান == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_place_below_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				যদি (b->type == BOX_FLOAT_বাম) {
					b->x = cx + x0;
					যদি (b->width > 0)
						x0 += b->width;
					বাম = b;
				} অন্যথা {
					b->x = cx + x1 - b->width;
					যদি (b->width > 0)
						x1 -= b->width;
					ডান = b;
				}
				b->y = cy;
			} অন্যথা {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				পূর্ণসংখ্যা fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_place_below_level) ?
						fcy :
						cont->cached_place_below_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + height : fy;

				place_float_below(b, *width, cx, fy, cont);
				fy = b->y;
				যদি (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_বাম && বাম != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ডান &&
						ডান != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(বাম != 0 || ডান != 0)))) {
					/* to be cleared below existing
					 * floats */
					যদি (b->type == BOX_FLOAT_বাম)
						b->x = cx;
					অন্যথা
						b->x = cx + *width - b->width;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					যদি (fcy > cont->clear_level)
						cont->clear_level = fcy;
					যদি (b->y < fcy)
						b->y = fcy;
				}
				যদি (b->type == BOX_FLOAT_বাম)
					বাম = b;
				অন্যথা
					ডান = b;
			}
			add_float_to_container(cont, b);

			split_box = 0;
		}
	}

	যদি (x1 - x0 < x && split_box) {
		/* the last box went over the end */
		size_t split = 0;
		পূর্ণসংখ্যা w;
		bool no_wrap = css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_previous;

		যদি (!no_wrap &&
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
					 split_box->দৈর্ঘ্য,
					 x1 - x0 - x - space_before,
					 &split,
					 &w);
		}

		/* split == 0 implies that text can't be split */

		যদি (split == 0)
			w = split_box->width;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      split_box,
		      (পূর্ণসংখ্যা)split_box->দৈর্ঘ্য,
		      split_box->text,
		      split,
		      w,
		      বাম,
		      ডান,
		      inline_গুনতি);

		যদি ((split == 0 || x1 - x0 <= x + space_before + w) &&
				!বাম && !ডান && inline_গুনতি == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			যদি (split == 0 || split == split_box->দৈর্ঘ্য) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = split_box->পরবর্তী;
			} অন্যথা {
				/* cut off first word for this line */
				যদি (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					ফেরত false;
				b = split_box->পরবর্তী;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} অন্যথা যদি ((split == 0 || x1 - x0 <= x + space_before + w) &&
				inline_গুনতি == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			প্রতিজ্ঞা(বাম || ডান);
			used_height = 0;
			যদি (বাম) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      বাম->y,
				      বাম->height);

				used_height = বাম->y + বাম->height - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      used_height);

			}
			যদি (ডান && used_height <
					ডান->y + ডান->height - cy + 1)
				used_height = ডান->y + ডান->height - cy + 1;

			যদি (used_height < 0)
				used_height = 0;

			b = split_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} অন্যথা যদি (split == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = split_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} অন্যথা {
			/* fit as many words as possible */
			প্রতিজ্ঞা(split != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (পূর্ণসংখ্যা)split_box->দৈর্ঘ্য, split_box->text,
			      x1 - x0, split, w);

			যদি (split != split_box->দৈর্ঘ্য) {
				যদি (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					ফেরত false;
				b = split_box->পরবর্তী;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	নির্বাচন (css_computed_text_align(first->parent->parent->style)) {
	ক্ষেত্র CSS_TEXT_ALIGN_ডান:
	ক্ষেত্র CSS_TEXT_ALIGN_LIBCSS_ডান:
		x0 = x1 - x;
		বিরতি;
	ক্ষেত্র CSS_TEXT_ALIGN_CENTER:
	ক্ষেত্র CSS_TEXT_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		বিরতি;
	ক্ষেত্র CSS_TEXT_ALIGN_বাম:
	ক্ষেত্র CSS_TEXT_ALIGN_LIBCSS_বাম:
	ক্ষেত্র CSS_TEXT_ALIGN_JUSTIFY:
		/* leave on left */
		বিরতি;
	ক্ষেত্র CSS_TEXT_ALIGN_DEFAULT:
		/* None; consider text direction */
		নির্বাচন (css_computed_direction(first->parent->parent->style)) {
		ক্ষেত্র CSS_DIRECTION_LTR:
			/* leave on left */
			বিরতি;
		ক্ষেত্র CSS_DIRECTION_RTL:
			x0 = x1 - x;
			বিরতি;
		}
		বিরতি;
	}

	জন্য (d = first; d != b; d = d->পরবর্তী) {
		d->flags &= ~NEW_সারি;

		যদি (d->type == BOX_INLINE_BLOCK &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			চালু;
		} অন্যথা যদি ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[শীর্ষ_2];

			যদি (d->type == BOX_TEXT && d->height > used_height) {
				/* text */
				used_height = d->height;
			}
		} অন্যথা যদি ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_BLOCK) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[শীর্ষ_2].width + d->margin[শীর্ষ_2];
			h = d->margin[শীর্ষ_2] + d->border[শীর্ষ_2].width +
					d->padding[শীর্ষ_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];
			যদি (used_height < h)
				used_height = h;
		}
	}

	first->flags |= NEW_সারি;

	প্রতিজ্ঞা(b != first || (move_y && 0 < used_height && (বাম || ডান)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	জন্য (d = first; d != b; d = d->পরবর্তী) {
		যদি ((d->type == BOX_INLINE && d->inline_end) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			css_fixed মান = 0;
			css_unit unit = CSS_UNIT_PX;
			নির্বাচন (css_computed_vertical_align(d->style, &মান,
					&unit)) {
			ক্ষেত্র CSS_VERTICAL_ALIGN_SUPER:
			ক্ষেত্র CSS_VERTICAL_ALIGN_শীর্ষ:
			ক্ষেত্র CSS_VERTICAL_ALIGN_TEXT_শীর্ষ:
				/* already at top */
				বিরতি;
			ক্ষেত্র CSS_VERTICAL_ALIGN_SUB:
			ক্ষেত্র CSS_VERTICAL_ALIGN_BOTTOM:
			ক্ষেত্র CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				d->y += used_height - d->height;
				বিরতি;
			পূর্বনির্ধারিত:
			ক্ষেত্র CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (used_height - d->height);
				বিরতি;
			}
		}
	}

	/* handle clearance for br */
	যদি (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		পূর্ণসংখ্যা clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		যদি (used_height < clear_y - cy)
			used_height = clear_y - cy;
	}

	যদি (move_y)
		*y += used_height;
	*পরবর্তী_box = b;
	*width = x; /* return actual width */
	ফেরত true;
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
স্থির bool layout_inline_container(কাঠামো box *inline_container, পূর্ণসংখ্যা width,
		কাঠামো box *cont, পূর্ণসংখ্যা cx, পূর্ণসংখ্যা cy, html_content *content)
{
	bool first_সারি = true;
	bool has_text_children;
	কাঠামো box *c, *পরবর্তী;
	পূর্ণসংখ্যা y = 0;
	পূর্ণসংখ্যা curwidth,maxwidth = width;

	প্রতিজ্ঞা(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      width,
	      cont,
	      cx,
	      cy);


	has_text_children = false;
	জন্য (c = inline_container->children; c; c = c->পরবর্তী) {
		bool is_pre = false;

		যদি (c->style) {
			গণনা css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_সারি ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		যদি ((lh__box_is_object(c) == false &&
				c->text && (c->দৈর্ঘ্য || is_pre)) ||
				c->type == BOX_BR)
			has_text_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	জন্য (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->width;
		যদি (!layout_সারি(c, &curwidth, &y, cx, cy + y, cont, first_সারি,
				has_text_children, content, &পরবর্তী))
			ফেরত false;
		maxwidth = max(maxwidth,curwidth);
		c = পরবর্তী;
		first_সারি = false;
	}

	inline_container->width = maxwidth;
	inline_container->height = y;

	ফেরত true;
}


/* Documented in layout_intertnal.h */
bool layout_block_context(
		কাঠামো box *block,
		পূর্ণসংখ্যা viewport_height,
		html_content *content)
{
	কাঠামো box *box;
	পূর্ণসংখ্যা cx, cy;  /**< current coordinates */
	পূর্ণসংখ্যা max_pos_margin = 0;
	পূর্ণসংখ্যা max_neg_margin = 0;
	পূর্ণসংখ্যা y = 0;
	পূর্ণসংখ্যা lm, rm;
	কাঠামো box *margin_collapse = ফাঁকা;
	bool in_margin = false;
	css_fixed gadget_আকার;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	প্রতিজ্ঞা(block->type == BOX_BLOCK ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX);
	প্রতিজ্ঞা(block->width != UNKNOWN_WIDTH);
	প্রতিজ্ঞা(block->width != AUTO);

	block->float_children = ফাঁকা;
	block->cached_place_below_level = 0;
	block->clear_level = 0;

	/* special case if the block contains an object */
	যদি (block->object) {
		পূর্ণসংখ্যা অস্থায়ী_width = block->width;
		যদি (!layout_block_object(block))
			ফেরত false;
		layout_get_object_dimensions(block, &অস্থায়ী_width,
				&block->height, INT_MIN, INT_MAX,
				INT_MIN, INT_MAX);
		ফেরত true;
	} অন্যথা যদি (block->flags & REPLACE_DIM) {
		ফেরত true;
	}

	/* special case if the block contains an radio button or checkbox */
	যদি (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_আকার = INTTOFIX(1);
		যদি (block->height == AUTO)
			block->height = FIXTOINT(css_unit_len2device_px(
					block->style,
					&content->unit_len_ctx,
					gadget_আকার, gadget_unit));
	}

	box = block->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = block->padding[শীর্ষ_2];
	যদি (box)
		box->y = block->padding[শীর্ষ_2];

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
	যখন (box) {
		গণনা css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
		গণনা css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

		প্রতিজ্ঞা(box->type == BOX_BLOCK ||
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

		যদি (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->parent->padding[বাম_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			যাও advance_to_পরবর্তী_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		যদি (margin_collapse == ফাঁকা) {
			margin_collapse = layout_পরবর্তী_margin_block(
					&content->unit_len_ctx, box, block,
					viewport_height,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			in_margin = true;
		}

		/* Clearance. */
		y = 0;
		যদি (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(block->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		যদি (box->style) {
			overflow_x = css_computed_overflow_x(box->style);
			overflow_y = css_computed_overflow_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		যদি (box->type == BOX_FLEX ||
		    box->type == BOX_BLOCK ||
		    box->flags & IFRAME) {
			যদি (lh__box_is_object(box) == false &&
					box->style &&
					(overflow_x != CSS_OVERFLOW_VISIBLE ||
					 overflow_y != CSS_OVERFLOW_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				পূর্ণসংখ্যা x0, x1, শীর্ষ;
				কাঠামো box *বাম, *ডান;
				শীর্ষ = cy + max_pos_margin - max_neg_margin;
				শীর্ষ = (শীর্ষ > y) ? শীর্ষ : y;
				x0 = cx;
				x1 = cx + box->parent->width -
						box->parent->padding[বাম_2] -
						box->parent->padding[ডান_2];
				find_sides(block->float_children, শীর্ষ, শীর্ষ,
						&x0, &x1, &বাম, &ডান);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->parent->width -
						box->parent->padding[বাম_2] -
						box->parent->padding[ডান_2] -
						x1;
			}
			layout_block_find_dimensions(&content->unit_len_ctx,
					box->parent->width,
					viewport_height, lm, rm, box);
			যদি (box->type == BOX_BLOCK && !(box->flags & IFRAME)) {
				layout_block_add_scrollbar(box, ডান_2);
				layout_block_add_scrollbar(box, BOTTOM);
			}
		} অন্যথা যদি (box->type == BOX_TABLE) {
			যদি (box->style != ফাঁকা) {
				গণনা css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style, &width,
						&unit);

				যদি (wtype == CSS_WIDTH_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					পূর্ণসংখ্যা x0, x1, শীর্ষ;
					কাঠামো box *বাম, *ডান;
					শীর্ষ = cy + max_pos_margin -
							max_neg_margin;
					শীর্ষ = (শীর্ষ > y) ? শীর্ষ : y;
					x0 = cx;
					x1 = cx + box->parent->width -
						box->parent->padding[বাম_2] -
						box->parent->padding[ডান_2];
					find_sides(block->float_children,
						শীর্ষ, শীর্ষ, &x0, &x1,
						&বাম, &ডান);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->parent->width -
						box->parent->padding[বাম_2] -
						box->parent->padding[ডান_2] -
						x1;
				}
			}
			যদি (!layout_table(box, box->parent->width - lm - rm,
					content))
				ফেরত false;
			layout_solve_width(box, box->parent->width, box->width,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->parent->padding[বাম_2] + box->margin[বাম_2] +
				box->border[বাম_2].width;
		cx += box->x;

		/* Position box: vertical. */
		যদি (box->border[শীর্ষ_2].width) {
			box->y += box->border[শীর্ষ_2].width;
			cy += box->border[শীর্ষ_2].width;
		}

		/* Vertical margin */
		যদি (((box->type == BOX_BLOCK && (box->flags & HAS_HEIGHT)) ||
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
		যদি (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		যদি (box->type == BOX_FLEX ||
				(box->type == BOX_BLOCK && box->style &&
				 (overflow_x != CSS_OVERFLOW_VISIBLE ||
				  overflow_y != CSS_OVERFLOW_VISIBLE))) {

			যদি (box->type == BOX_FLEX) {
				যদি (!layout_flex(box, box->width, content)) {
					ফেরত false;
				}
			} অন্যথা {
				layout_block_context(box,
						viewport_height, content);
			}

			cy += box->padding[শীর্ষ_2];

			যদি (box->height == AUTO) {
				box->height = 0;
				layout_block_add_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->height + box->padding[BOTTOM] +
					box->border[BOTTOM].width;
			y = box->y + box->padding[শীর্ষ_2] + box->height +
					box->padding[BOTTOM] +
					box->border[BOTTOM].width;

			/* Skip children, because they are done in the new
			 * block context */
			যাও advance_to_পরবর্তী_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->width);

		/* Layout (except tables). */
		যদি (box->object) {
			যদি (!layout_block_object(box))
				ফেরত false;

		} অন্যথা যদি (box->type == BOX_INLINE_CONTAINER) {
			box->width = box->parent->width;
			যদি (!layout_inline_container(box, box->width, block,
					cx, cy, content))
				ফেরত false;

		} অন্যথা যদি (box->type == BOX_TABLE) {
			/* Move down to avoid floats if necessary. */
			পূর্ণসংখ্যা x0, x1;
			কাঠামো box *বাম, *ডান;
			y = cy;
			যখন (1) {
				গণনা css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style,
						&width, &unit);

				x0 = cx;
				x1 = cx + box->parent->width;
				find_sides(block->float_children, y,
						y + box->height,
						&x0, &x1, &বাম, &ডান);
				যদি (wtype == CSS_WIDTH_AUTO)
					বিরতি;
				যদি (box->width <= x1 - x0)
					বিরতি;
				যদি (!বাম && !ডান)
					বিরতি;
				অন্যথা যদি (!বাম)
					y = ডান->y + ডান->height + 1;
				অন্যথা যদি (!ডান)
					y = বাম->y + বাম->height + 1;
				অন্যথা যদি (বাম->y + বাম->height <
						ডান->y + ডান->height)
					y = বাম->y + বাম->height + 1;
				অন্যথা
					y = ডান->y + ডান->height + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		যদি (box->type == BOX_BLOCK && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			যদি (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = ফাঁকা;
			}

			y = box->padding[শীর্ষ_2];
			box = box->children;
			box->y = y;
			cy += y;
			চালু;
		} অন্যথা যদি (box->type == BOX_BLOCK || box->object ||
				box->flags & IFRAME)
			cy += box->padding[শীর্ষ_2];

		যদি (box->type == BOX_BLOCK && box->height == AUTO) {
			box->height = 0;
			layout_block_add_scrollbar(box, BOTTOM);
		}

		cy += box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width;
		cx -= box->x;
		y = box->y + box->padding[শীর্ষ_2] + box->height +
				box->padding[BOTTOM] +
				box->border[BOTTOM].width;

	advance_to_পরবর্তী_box:
		যদি (!box->পরবর্তী) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			করো {
				যদি (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = ফাঁকা;
				}

				/* Apply bottom margin */
				যদি (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				অন্যথা যদি (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->parent;
				যদি (box == block)
					বিরতি;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				যদি (box->type == BOX_BLOCK &&
						box->flags & MAKE_HEIGHT) {
					margin_collapse = ফাঁকা;
					in_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				যদি (box->height == AUTO) {
					box->height = y - box->padding[শীর্ষ_2];

					যদি (box->type == BOX_BLOCK)
						layout_block_add_scrollbar(box,
								BOTTOM);
				} অন্যথা
					cy += box->height -
							(y - box->padding[শীর্ষ_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				যদি (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_height(
							&content->unit_len_ctx,
							box, ফাঁকা)) {
					/* Height altered */
					/* Set current cy */
					cy += box->height -
							(y - box->padding[শীর্ষ_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].width;
				cx -= box->x;
				y = box->y + box->padding[শীর্ষ_2] + box->height +
						box->padding[BOTTOM] +
						box->border[BOTTOM].width;

			} যখন (box->পরবর্তী == ফাঁকা);
			যদি (box == block)
				বিরতি;
		}

		/* To next sibling. */

		যদি (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = ফাঁকা;
		}

		যদি (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		অন্যথা যদি (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->পরবর্তী;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	জন্য (box = block->float_children; box; box = box->পরবর্তী_float) {
		y = box->y + box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width + box->margin[BOTTOM];
		যদি (cy < y)
			cy = y;
	}

	যদি (block->height == AUTO) {
		block->height = cy - block->padding[শীর্ষ_2];
		যদি (block->type == BOX_BLOCK)
			layout_block_add_scrollbar(block, BOTTOM);
	}

	যদি (block->style && css_computed_position(block->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_height(&content->unit_len_ctx, block, ফাঁকা);
	}

	যদি (block->gadget &&
			(block->gadget->type == GADGET_TEXTAREA ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		পূর্ণসংখ্যা ta_width = block->padding[বাম_2] + block->width +
				block->padding[ডান_2];
		পূর্ণসংখ্যা ta_height = block->padding[শীর্ষ_2] + block->height +
				block->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				block->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_set_layout(block->gadget->তথ্য.text.ta,
				&fstyle, ta_width, ta_height,
				block->padding[শীর্ষ_2], block->padding[ডান_2],
				block->padding[BOTTOM], block->padding[বাম_2]);
	}

	ফেরত true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
স্থির bool
layout__get_element_tag(
		ধ্রুবক dom_গ্রন্থি *গ্রন্থি,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_গ্রন্থি_type গ্রন্থি_type;
	dom_exception exc;

	exc = dom_গ্রন্থি_get_গ্রন্থি_type(গ্রন্থি, &গ্রন্থি_type);
	যদি (exc != DOM_NO_ERR ||
	    গ্রন্থি_type != DOM_ELEMENT_গ্রন্থি) {
		ফেরত false;
	}

	exc = dom_html_element_get_tag_type(গ্রন্থি, &element_type);
	যদি (exc != DOM_NO_ERR) {
		ফেরত false;
	}

	*type = element_type;
	ফেরত true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
স্থির অন্তঃরেখ bool
layout__check_element_type(
		ধ্রুবক dom_গ্রন্থি *গ্রন্থি,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	যদি (!layout__get_element_tag(গ্রন্থি, &element_type)) {
		ফেরত false;
	}

	ফেরত element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
স্থির bool
layout_get_li_মান(dom_গ্রন্থি *li_গ্রন্থি, dom_long *মান_out)
{
	dom_exception exc;
	dom_long মান;
	bool has_মান;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_গ্রন্থি,
			corestring_dom_মান,
			&has_মান);
	যদি (exc != DOM_NO_ERR || has_মান == false) {
		ফেরত false;
	}

	exc = dom_html_li_element_get_মান(
			(dom_html_li_element *)li_গ্রন্থি,
			&মান);
	যদি (exc != DOM_NO_ERR) {
		ফেরত false;
	}

	*মান_out = মান;
	ফেরত true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
স্থির bool
layout__get_ol_start(dom_গ্রন্থি *ol_গ্রন্থি, dom_long *start_out)
{
	dom_exception exc;
	dom_long start;
	bool has_start;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_গ্রন্থি,
			corestring_dom_start,
			&has_start);
	যদি (exc != DOM_NO_ERR || has_start == false) {
		ফেরত false;
	}

	exc = dom_html_olist_element_get_start(
			(dom_html_olist_element *)ol_গ্রন্থি,
			&start);
	যদি (exc != DOM_NO_ERR) {
		ফেরত false;
	}

	*start_out = start;
	ফেরত true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
স্থির bool
layout__get_ol_reversed(dom_গ্রন্থি *ol_গ্রন্থি)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_গ্রন্থি,
			corestring_dom_reversed,
			&has_reversed);
	যদি (exc != DOM_NO_ERR) {
		ফেরত false;
	}

	ফেরত has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
স্থির bool
layout_get_list_item_গুনতি(
		dom_গ্রন্থি *list_owner, dom_long *গুনতি_out)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_গ্রন্থি *child;
	পূর্ণসংখ্যা গুনতি;

	যদি (list_owner == ফাঁকা) {
		ফেরত false;
	}

	যদি (!layout__get_element_tag(list_owner, &tag_type)) {
		ফেরত false;
	}

	যদি (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		ফেরত false;
	}

	exc = dom_গ্রন্থি_get_first_child(list_owner, &child);
	যদি (exc != DOM_NO_ERR) {
		ফেরত false;
	}

	গুনতি = 0;
	যখন (child != ফাঁকা) {
		dom_গ্রন্থি *অস্থায়ী_গ্রন্থি;

		যদি (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			কাঠামো box *child_box;
			যদি (dom_গ্রন্থি_get_user_তথ্য(child,
					corestring_dom_ns_চাবি_box_গ্রন্থি_তথ্য,
					&child_box) != DOM_NO_ERR) {
				dom_গ্রন্থি_unref(child);
				ফেরত false;
			}

			যদি (child_box != ফাঁকা &&
			    child_box->list_marker != ফাঁকা) {
				গুনতি++;
			}
		}

		exc = dom_গ্রন্থি_get_পরবর্তী_sibling(child, &অস্থায়ী_গ্রন্থি);
		dom_গ্রন্থি_unref(child);
		যদি (exc != DOM_NO_ERR) {
			ফেরত false;
		}

		child = অস্থায়ী_গ্রন্থি;
	}

	*গুনতি_out = গুনতি;
	ফেরত true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
স্থির শূন্য
layout_ordered_list_গুনতি(
		কাঠামো box *box)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_গ্রন্থি *child;
	পূর্ণসংখ্যা step = 1;
	dom_long পরবর্তী;

	যদি (box->গ্রন্থি == ফাঁকা) {
		ফেরত;
	}

	যদি (!layout__get_element_tag(box->গ্রন্থি, &tag_type)) {
		ফেরত;
	}

	যদি (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		ফেরত;
	}

	পরবর্তী = 1;
	যদি (tag_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_start = layout__get_ol_start(box->গ্রন্থি, &পরবর্তী);
		bool have_reversed = layout__get_ol_reversed(box->গ্রন্থি);

		যদি (have_reversed) {
			step = -1;
		}

		যদি (!have_start && have_reversed) {
			layout_get_list_item_গুনতি(box->গ্রন্থি, &পরবর্তী);
		}
	}

	exc = dom_গ্রন্থি_get_first_child(box->গ্রন্থি, &child);
	যদি (exc != DOM_NO_ERR) {
		ফেরত;
	}

	যখন (child != ফাঁকা) {
		dom_গ্রন্থি *অস্থায়ী_গ্রন্থি;

		যদি (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			কাঠামো box *child_box;

			যদি (dom_গ্রন্থি_get_user_তথ্য(child,
					corestring_dom_ns_চাবি_box_গ্রন্থি_তথ্য,
					&child_box) != DOM_NO_ERR) {
				dom_গ্রন্থি_unref(child);
				ফেরত;
			}

			যদি (child_box != ফাঁকা &&
			    child_box->list_marker != ফাঁকা) {
				dom_long মান;
				কাঠামো box *marker = child_box->list_marker;
				যদি (layout_get_li_মান(child, &মান)) {
					marker->list_মান = মান;
					পরবর্তী = marker->list_মান;
				} অন্যথা {
					marker->list_মান = পরবর্তী;
				}
				পরবর্তী += step;
			}
		}

		exc = dom_গ্রন্থি_get_পরবর্তী_sibling(child, &অস্থায়ী_গ্রন্থি);
		dom_গ্রন্থি_unref(child);
		যদি (exc != DOM_NO_ERR) {
			ফেরত;
		}

		child = অস্থায়ী_গ্রন্থি;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
স্থির শূন্য
layout__set_numerical_marker_text(
		ধ্রুবক html_content *content,
		কাঠামো box *box)
{
	কাঠামো box *marker = box->list_marker;
	size_t গণক_len;
	css_error css_res;
	গণনা {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_আকার = 20,
	};

	marker->text = talloc_array(content->bctx, অক্ষর, LIST_MARKER_আকার);
	যদি (marker->text == ফাঁকা) {
		ফেরত;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_মান,
			marker->text, LIST_MARKER_আকার, &গণক_len);
	যদি (css_res == CSS_OK) {
		যদি (গণক_len > LIST_MARKER_আকার) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->text = talloc_realloc(content->bctx,
					marker->text,
					অক্ষর,
					গণক_len);
			যদি (marker->text == ফাঁকা) {
				ফেরত;
			}
			css_computed_format_list_style(box->style,
					marker->list_মান, marker->text,
					গণক_len, &গণক_len);
		}
		marker->দৈর্ঘ্য = গণক_len;
	}
}

/**
 * Find out if box's style represents a numerical list style type.
 *
 * \param[in]  b  Box with style to test.
 * \return true if box has numerical list style type, false otherwise.
 */
স্থির bool
layout__list_item_is_numerical(
		ধ্রুবক কাঠামো box *b)
{
	গণনা css_list_style_type_e t = css_computed_list_style_type(b->style);

	নির্বাচন (t) {
	ক্ষেত্র CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	ক্ষেত্র CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	ক্ষেত্র CSS_LIST_STYLE_TYPE_SQUARE: /* Fall through. */
	ক্ষেত্র CSS_LIST_STYLE_TYPE_NONE:
		ফেরত false;

	পূর্বনির্ধারিত:
		ফেরত true;
	}
}

/**
 * Layout list markers.
 */
স্থির শূন্য
layout_lists(ধ্রুবক html_content *content, কাঠামো box *box)
{
	কাঠামো box *child;

	layout_ordered_list_গুনতি(box);

	জন্য (child = box->children; child; child = child->পরবর্তী) {
		যদি (child->list_marker) {
			কাঠামো box *marker = child->list_marker;

			যদি (layout__list_item_is_numerical(child)) {
				যদি (marker->text == ফাঁকা) {
					layout__set_numerical_marker_text(
							content, child);
				}
			}
			যদি (marker->object) {
				marker->width =
					content_get_width(marker->object);
				marker->x = -marker->width;
				marker->height =
					content_get_height(marker->object);
				marker->y = (সারি_height(
						&content->unit_len_ctx,
						marker->style) -
						marker->height) / 2;
			} অন্যথা যদি (marker->text) {
				যদি (marker->width == UNKNOWN_WIDTH) {
					plot_font_style_t fstyle;
					font_plot_style_from_css(
							&content->unit_len_ctx,
							marker->style,
							&fstyle);
					content->font_func->width(&fstyle,
							marker->text,
							marker->দৈর্ঘ্য,
							&marker->width);
					marker->flags |= MEASURED;
				}
				marker->x = -marker->width;
				marker->y = 0;
				marker->height = সারি_height(
						&content->unit_len_ctx,
						marker->style);
			} অন্যথা {
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
স্থির শূন্য
layout_compute_offsets(ধ্রুবক css_unit_ctx *unit_len_ctx,
		       কাঠামো box *box,
		       কাঠামো box *containing_block,
		       পূর্ণসংখ্যা *শীর্ষ,
		       পূর্ণসংখ্যা *ডান,
		       পূর্ণসংখ্যা *bottom,
		       পূর্ণসংখ্যা *বাম)
{
	uint32_t type;
	css_fixed মান = 0;
	css_unit unit = CSS_UNIT_PX;

	প্রতিজ্ঞা(containing_block->width != UNKNOWN_WIDTH);
	প্রতিজ্ঞা(containing_block->width != AUTO);
	প্রতিজ্ঞা(containing_block->height != AUTO);

	/*বাম*/
	type = css_computed_বাম(box->style, &মান, &unit);
	যদি (type == CSS_বাম_SET) {
		যদি (unit == CSS_UNIT_PCT) {
			*বাম = FPCT_OF_INT_TOINT(মান,
					containing_block->width);
		} অন্যথা {
			*বাম = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					মান, unit));
		}
	} অন্যথা {
		*বাম = AUTO;
	}

	/*ডান*/
	type = css_computed_ডান(box->style, &মান, &unit);
	যদি (type == CSS_ডান_SET) {
		যদি (unit == CSS_UNIT_PCT) {
			*ডান = FPCT_OF_INT_TOINT(মান,
					containing_block->width);
		} অন্যথা {
			*ডান = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					মান, unit));
		}
	} অন্যথা {
		*ডান = AUTO;
	}

	/*শীর্ষ*/
	type = css_computed_শীর্ষ(box->style, &মান, &unit);
	যদি (type == CSS_শীর্ষ_SET) {
		যদি (unit == CSS_UNIT_PCT) {
			*শীর্ষ = FPCT_OF_INT_TOINT(মান,
					containing_block->height);
		} অন্যথা {
			*শীর্ষ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					মান, unit));
		}
	} অন্যথা {
		*শীর্ষ = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &মান, &unit);
	যদি (type == CSS_BOTTOM_SET) {
		যদি (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_OF_INT_TOINT(মান,
					containing_block->height);
		} অন্যথা {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					মান, unit));
		}
	} অন্যথা {
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
স্থির bool
layout_absolute(কাঠামো box *box,
		কাঠামো box *containing_block,
		পূর্ণসংখ্যা cx, পূর্ণসংখ্যা cy,
		html_content *content)
{
	পূর্ণসংখ্যা static_বাম, static_শীর্ষ;  /* static position */
	পূর্ণসংখ্যা শীর্ষ, ডান, bottom, বাম;
	পূর্ণসংখ্যা width, height, max_width, min_width;
	পূর্ণসংখ্যা *margin = box->margin;
	পূর্ণসংখ্যা *padding = box->padding;
	কাঠামো box_border *border = box->border;
	পূর্ণসংখ্যা available_width = containing_block->width;
	পূর্ণসংখ্যা space;

	প্রতিজ্ঞা(box->type == BOX_BLOCK || box->type == BOX_TABLE ||
			box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_বাম = cx + box->x;
	static_শীর্ষ = cy + box->y;

	যদি (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_block->width += containing_block->padding[বাম_2] +
				containing_block->padding[ডান_2];
		containing_block->height += containing_block->padding[শীর্ষ_2] +
				containing_block->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_block,
			&শীর্ষ, &ডান, &bottom, &বাম);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_block;
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1,
			box, box->style, &width, &height,
			&max_width, &min_width, 0, 0,
			margin, padding, border);
	box->float_container = ফাঁকা;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      বাম, margin[বাম_2], border[বাম_2].width, padding[বাম_2], width,
	      padding[ডান_2], border[ডান_2].width, margin[ডান_2], ডান,
	      containing_block->width);


	যদি (বাম == AUTO && width == AUTO && ডান == AUTO) {
		যদি (margin[বাম_2] == AUTO)
			margin[বাম_2] = 0;
		যদি (margin[ডান_2] == AUTO)
			margin[ডান_2] = 0;
		বাম = static_বাম;

		width = min(max(box->min_width, available_width),
			box->max_width);
		width -= box->margin[বাম_2] + box->border[বাম_2].width +
			box->padding[বাম_2] + box->padding[ডান_2] +
			box->border[ডান_2].width + box->margin[ডান_2];

		/* Adjust for {min|max}-width */
		যদি (max_width >= 0 && width > max_width) width = max_width;
		যদি (width < min_width) width = min_width;

		ডান = containing_block->width -
			বাম -
			margin[বাম_2] - border[বাম_2].width - padding[বাম_2] -
			width -
			padding[ডান_2] - border[ডান_2].width - margin[ডান_2];
	} অন্যথা যদি (বাম != AUTO && width != AUTO && ডান != AUTO) {

		/* Adjust for {min|max}-width */
		যদি (max_width >= 0 && width > max_width) width = max_width;
		যদি (min_width >  0 && width < min_width) width = min_width;

		যদি (margin[বাম_2] == AUTO && margin[ডান_2] == AUTO) {
			space = containing_block->width -
					বাম - border[বাম_2].width -
					padding[বাম_2] - width - padding[ডান_2] -
					border[ডান_2].width - ডান;
			যদি (space < 0) {
				margin[বাম_2] = 0;
				margin[ডান_2] = space;
			} অন্যথা {
				margin[বাম_2] = margin[ডান_2] = space / 2;
			}
		} অন্যথা যদি (margin[বাম_2] == AUTO) {
			margin[বাম_2] = containing_block->width -
					বাম - border[বাম_2].width -
					padding[বাম_2] - width - padding[ডান_2] -
					border[ডান_2].width - margin[ডান_2] -
					ডান;
		} অন্যথা যদি (margin[ডান_2] == AUTO) {
			margin[ডান_2] = containing_block->width -
					বাম - margin[বাম_2] -
					border[বাম_2].width -
					padding[বাম_2] - width - padding[ডান_2] -
					border[ডান_2].width - ডান;
		} অন্যথা {
			ডান = containing_block->width -
					বাম - margin[বাম_2] -
					border[বাম_2].width -
					padding[বাম_2] - width - padding[ডান_2] -
					border[ডান_2].width - margin[ডান_2];
		}
	} অন্যথা {
		যদি (margin[বাম_2] == AUTO)
			margin[বাম_2] = 0;
		যদি (margin[ডান_2] == AUTO)
			margin[ডান_2] = 0;

		যদি (বাম == AUTO && width == AUTO && ডান != AUTO) {
			available_width -= ডান;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[বাম_2] + box->border[বাম_2].width +
				box->padding[বাম_2] + box->padding[ডান_2] +
				box->border[ডান_2].width + box->margin[ডান_2];

			/* Adjust for {min|max}-width */
			যদি (max_width >= 0 && width > max_width)
				width = max_width;
			যদি (width < min_width)
				width = min_width;

			বাম = containing_block->width -
					margin[বাম_2] - border[বাম_2].width -
					padding[বাম_2] - width - padding[ডান_2] -
					border[ডান_2].width - margin[ডান_2] -
					ডান;
		} অন্যথা যদি (বাম == AUTO && width != AUTO && ডান == AUTO) {

			/* Adjust for {min|max}-width */
			যদি (max_width >= 0 && width > max_width)
				width = max_width;
			যদি (min_width >  0 && width < min_width)
				width = min_width;

			বাম = static_বাম;
			ডান = containing_block->width -
					বাম - margin[বাম_2] -
					border[বাম_2].width -
					padding[বাম_2] - width - padding[ডান_2] -
					border[ডান_2].width - margin[ডান_2];
		} অন্যথা যদি (বাম != AUTO && width == AUTO && ডান == AUTO) {
			available_width -= বাম;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[বাম_2] + box->border[বাম_2].width +
				box->padding[বাম_2] + box->padding[ডান_2] +
				box->border[ডান_2].width + box->margin[ডান_2];

			/* Adjust for {min|max}-width */
			যদি (max_width >= 0 && width > max_width)
				width = max_width;
			যদি (width < min_width)
				width = min_width;

			ডান = containing_block->width -
					বাম - margin[বাম_2] -
					border[বাম_2].width -
					padding[বাম_2] - width - padding[ডান_2] -
					border[ডান_2].width - margin[ডান_2];
		} অন্যথা যদি (বাম == AUTO && width != AUTO && ডান != AUTO) {

			/* Adjust for {min|max}-width */
			যদি (max_width >= 0 && width > max_width)
				width = max_width;
			যদি (width < min_width)
				width = min_width;

			বাম = containing_block->width -
					margin[বাম_2] - border[বাম_2].width -
					padding[বাম_2] - width - padding[ডান_2] -
					border[ডান_2].width - margin[ডান_2] -
					ডান;
		} অন্যথা যদি (বাম != AUTO && width == AUTO && ডান != AUTO) {
			width = containing_block->width -
					বাম - margin[বাম_2] -
					border[বাম_2].width -
					padding[বাম_2] - padding[ডান_2] -
					border[ডান_2].width - margin[ডান_2] -
					ডান;

			/* Adjust for {min|max}-width */
			যদি (max_width >= 0 && width > max_width)
				width = max_width;
			যদি (width < min_width)
				width = min_width;

		} অন্যথা যদি (বাম != AUTO && width != AUTO && ডান == AUTO) {

			/* Adjust for {min|max}-width */
			যদি (max_width >= 0 && width > max_width)
				width = max_width;
			যদি (width < min_width)
				width = min_width;

			ডান = containing_block->width -
					বাম - margin[বাম_2] -
					border[বাম_2].width -
					padding[বাম_2] - width - padding[ডান_2] -
					border[ডান_2].width - margin[ডান_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      বাম, margin[বাম_2], border[বাম_2].width, padding[বাম_2], width,
	      padding[ডান_2], border[ডান_2].width, margin[ডান_2], ডান,
	      containing_block->width);

	box->x = বাম + margin[বাম_2] + border[বাম_2].width - cx;
	যদি (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_block->width -= containing_block->padding[বাম_2] +
				containing_block->padding[ডান_2];
	} অন্যথা {
		/** \todo inline ancestors */
	}
	box->width = width;
	box->height = height;

	যদি (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->object || box->flags & IFRAME) {
		যদি (!layout_block_context(box, -1, content))
			ফেরত false;
	} অন্যথা যদি (box->type == BOX_TABLE) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		/* \todo  layout_table considers margins etc. again */
		যদি (!layout_table(box, width, content))
			ফেরত false;
		box->float_container = ফাঁকা;
		layout_solve_width(box, box->parent->width, box->width, 0, 0,
				-1, -1);
	} অন্যথা যদি (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		যদি (!layout_flex(box, width, content))
			ফেরত false;
		box->float_container = ফাঁকা;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      শীর্ষ, margin[শীর্ষ_2], border[শীর্ষ_2].width, padding[শীর্ষ_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	যদি (শীর্ষ == AUTO && height == AUTO && bottom == AUTO) {
		শীর্ষ = static_শীর্ষ;
		height = box->height;
		যদি (margin[শীর্ষ_2] == AUTO)
			margin[শীর্ষ_2] = 0;
		যদি (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_block->height -
				শীর্ষ - margin[শীর্ষ_2] - border[শীর্ষ_2].width -
				padding[শীর্ষ_2] - height - padding[BOTTOM] -
				border[BOTTOM].width - margin[BOTTOM];
	} অন্যথা যদি (শীর্ষ != AUTO && height != AUTO && bottom != AUTO) {
		যদি (margin[শীর্ষ_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_block->height -
					শীর্ষ - border[শীর্ষ_2].width - padding[শীর্ষ_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - bottom;
			margin[শীর্ষ_2] = margin[BOTTOM] = space / 2;
		} অন্যথা যদি (margin[শীর্ষ_2] == AUTO) {
			margin[শীর্ষ_2] = containing_block->height -
					শীর্ষ - border[শীর্ষ_2].width - padding[শীর্ষ_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} অন্যথা যদি (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_block->height -
					শীর্ষ - margin[শীর্ষ_2] - border[শীর্ষ_2].width -
					padding[শীর্ষ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					bottom;
		} অন্যথা {
			bottom = containing_block->height -
					শীর্ষ - margin[শীর্ষ_2] - border[শীর্ষ_2].width -
					padding[শীর্ষ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	} অন্যথা {
		যদি (margin[শীর্ষ_2] == AUTO)
			margin[শীর্ষ_2] = 0;
		যদি (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		যদি (শীর্ষ == AUTO && height == AUTO && bottom != AUTO) {
			height = box->height;
			শীর্ষ = containing_block->height -
					margin[শীর্ষ_2] - border[শীর্ষ_2].width -
					padding[শীর্ষ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} অন্যথা যদি (শীর্ষ == AUTO && height != AUTO && bottom == AUTO) {
			শীর্ষ = static_শীর্ষ;
			bottom = containing_block->height -
					শীর্ষ - margin[শীর্ষ_2] - border[শীর্ষ_2].width -
					padding[শীর্ষ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} অন্যথা যদি (শীর্ষ != AUTO && height == AUTO && bottom == AUTO) {
			height = box->height;
			bottom = containing_block->height -
					শীর্ষ - margin[শীর্ষ_2] - border[শীর্ষ_2].width -
					padding[শীর্ষ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} অন্যথা যদি (শীর্ষ == AUTO && height != AUTO && bottom != AUTO) {
			শীর্ষ = containing_block->height -
					margin[শীর্ষ_2] - border[শীর্ষ_2].width -
					padding[শীর্ষ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} অন্যথা যদি (শীর্ষ != AUTO && height == AUTO && bottom != AUTO) {
			height = containing_block->height -
					শীর্ষ - margin[শীর্ষ_2] - border[শীর্ষ_2].width -
					padding[শীর্ষ_2] - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} অন্যথা যদি (শীর্ষ != AUTO && height != AUTO && bottom == AUTO) {
			bottom = containing_block->height -
					শীর্ষ - margin[শীর্ষ_2] - border[শীর্ষ_2].width -
					padding[শীর্ষ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      শীর্ষ, margin[শীর্ষ_2], border[শীর্ষ_2].width, padding[শীর্ষ_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	box->y = শীর্ষ + margin[শীর্ষ_2] + border[শীর্ষ_2].width - cy;
	যদি (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_block->height -= containing_block->padding[শীর্ষ_2] +
				containing_block->padding[BOTTOM];
	} অন্যথা {
		/** \todo Inline ancestors */
	}
	box->height = height;
	layout_apply_minmax_height(&content->unit_len_ctx, box, containing_block);

	ফেরত true;
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
স্থির bool
layout_position_absolute(কাঠামো box *box,
			 কাঠামো box *containing_block,
			 পূর্ণসংখ্যা cx, পূর্ণসংখ্যা cy,
			 html_content *content)
{
	কাঠামো box *c;

	জন্য (c = box->children; c; c = c->পরবর্তী) {
		যদি ((c->type == BOX_BLOCK || c->type == BOX_TABLE ||
				c->type == BOX_INLINE_BLOCK ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			যদি (!layout_absolute(c, containing_block,
					cx, cy, content))
				ফেরত false;
			যদি (!layout_position_absolute(c, c, 0, 0, content))
				ফেরত false;
		} অন্যথা যদি (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			যদি (!layout_position_absolute(c, c, 0, 0, content))
				ফেরত false;
		} অন্যথা {
			পূর্ণসংখ্যা px, py;
			যদি (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_বাম ||
					css_computed_float(c->style) ==
						CSS_FLOAT_ডান)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				কাঠামো box *p;
				px = c->x;
				py = c->y;
				জন্য (p = box->parent; p && !p->float_children;
						p = p->parent) {
					px -= p->x;
					py -= p->y;
				}
			} অন্যথা {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			যদি (!layout_position_absolute(c, containing_block,
					cx + px, cy + py, content))
				ফেরত false;
		}
	}

	ফেরত true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
স্থির শূন্য layout_compute_relative_offset(
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		কাঠামো box *box,
		পূর্ণসংখ্যা *x,
		পূর্ণসংখ্যা *y)
{
	পূর্ণসংখ্যা বাম, ডান, শীর্ষ, bottom;
	কাঠামো box *containing_block;

	প্রতিজ্ঞা(box && box->parent && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	যদি (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_বাম ||
			css_computed_float(box->style) == CSS_FLOAT_ডান)) {
		containing_block = box->float_container;
	} অন্যথা {
		containing_block = box->parent;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_block,
			&শীর্ষ, &ডান, &bottom, &বাম);

	যদি (বাম == AUTO && ডান == AUTO)
		বাম = ডান = 0;
	অন্যথা যদি (বাম == AUTO)
		/* left is auto => computed = -right */
		বাম = -ডান;
	অন্যথা যদি (ডান == AUTO)
		/* right is auto => computed = -left */
		ডান = -বাম;
	অন্যথা {
		/* over constrained => examine direction property
		 * of containing block */
		যদি (containing_block->style &&
				css_computed_direction(
				containing_block->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			বাম = -ডান;
		} অন্যথা {
			/* assume LTR in all other cases */
			ডান = -বাম;
		}
	}

	প্রতিজ্ঞা(বাম == -ডান);

	যদি (শীর্ষ == AUTO && bottom == AUTO) {
		শীর্ষ = bottom = 0;
	} অন্যথা যদি (শীর্ষ == AUTO) {
		শীর্ষ = -bottom;
	} অন্যথা {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -শীর্ষ;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", বাম,
	      ডান, শীর্ষ, bottom);

	*x = বাম;
	*y = শীর্ষ;
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
স্থির শূন্য
layout_position_relative(
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		কাঠামো box *মূল,
		কাঠামো box *fp,
		পূর্ণসংখ্যা fx,
		পূর্ণসংখ্যা fy)
{
	কাঠামো box *box; /* for children of "root" */
	কাঠামো box *fn;  /* for block formatting context box for children of
			  * "box" */
	কাঠামো box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	পূর্ণসংখ্যা x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	পূর্ণসংখ্যা fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	প্রতিজ্ঞা(মূল);

	/* Normal children */
	জন্য (box = মূল->children; box; box = box->পরবর্তী) {

		যদি (box->type == BOX_TEXT)
			চালু;

		/* If relatively positioned, get offsets */
		যদি (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_offset(
					unit_len_ctx, box, &x, &y);
		অন্যথা
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		যদি (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_বাম ||
				css_computed_float(box->style) ==
					CSS_FLOAT_ডান) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			জন্য (fc = fp->float_children; fc; fc = fc->পরবর্তী_float) {
				যদি (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		যদি (box->float_children) {
			fn = box;
			fnx = fny = 0;
		} অন্যথা {
			fn = fp;
			fnx = fx + x;
			fny = fy + y;
		}

		/* recurse first */
		layout_position_relative(unit_len_ctx, box, fn, fnx, fny);

		/* Ignore things we're not interested in. */
		যদি (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			চালু;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		যদি (box->type == BOX_INLINE && box->inline_end) {
			কাঠামো box *b;
			জন্য (b = box->পরবর্তী; b && b != box->inline_end;
					b = b->পরবর্তী) {
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
স্থির শূন্য
layout_get_box_bbox(
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		কাঠামো box *box,
		পূর্ণসংখ্যা *desc_x0, পূর্ণসংখ্যা *desc_y0,
		পূর্ণসংখ্যা *desc_x1, পূর্ণসংখ্যা *desc_y1)
{
	*desc_x0 = -box->border[বাম_2].width;
	*desc_y0 = -box->border[শীর্ষ_2].width;
	*desc_x1 = box->padding[বাম_2] + box->width + box->padding[ডান_2] +
			box->border[ডান_2].width;
	*desc_y1 = box->padding[শীর্ষ_2] + box->height + box->padding[BOTTOM] +
			box->border[BOTTOM].width;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	যদি (box->type == BOX_BLOCK && box->style != ফাঁকা &&
			css_computed_overflow_y(box->style) ==
					CSS_OVERFLOW_VISIBLE &&
			box->object == ফাঁকা) {
		css_fixed font_আকার = 0;
		css_unit font_unit = CSS_UNIT_PT;
		পূর্ণসংখ্যা text_height;

		css_computed_font_আকার(box->style, &font_আকার, &font_unit);
		text_height = css_unit_len2device_px(box->style, unit_len_ctx,
				font_আকার, font_unit);
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
স্থির শূন্য
layout_update_descendant_bbox(
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		কাঠামো box *box,
		কাঠামো box *child,
		পূর্ণসংখ্যা off_x,
		পূর্ণসংখ্যা off_y)
{
	পূর্ণসংখ্যা child_desc_x0, child_desc_y0, child_desc_x1, child_desc_y1;

	/* get coordinates of child relative to box */
	পূর্ণসংখ্যা child_x = child->x - off_x;
	পূর্ণসংখ্যা child_y = child->y - off_y;

	bool html_object = (child->object &&
			content_get_type(child->object) == CONTENT_HTML);

	গণনা css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	গণনা css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

	যদি (child->style != ফাঁকা) {
		overflow_x = css_computed_overflow_x(child->style);
		overflow_y = css_computed_overflow_y(child->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, child,
			&child_desc_x0, &child_desc_y0,
			&child_desc_x1, &child_desc_y1);

	যদি (overflow_x == CSS_OVERFLOW_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		child_desc_x0 = child->descendant_x0;
		child_desc_x1 = child->descendant_x1;
	}
	যদি (overflow_y == CSS_OVERFLOW_VISIBLE &&
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
	যদি (child_desc_x0 < box->descendant_x0)
		box->descendant_x0 = child_desc_x0;
	যদি (child_desc_y0 < box->descendant_y0)
		box->descendant_y0 = child_desc_y0;
	যদি (box->descendant_x1 < child_desc_x1)
		box->descendant_x1 = child_desc_x1;
	যদি (box->descendant_y1 < child_desc_y1)
		box->descendant_y1 = child_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
স্থির শূন্য layout_calculate_descendant_bboxes(
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		কাঠামো box *box)
{
	কাঠামো box *child;

	প্রতিজ্ঞা(box->width != UNKNOWN_WIDTH);
	প্রতিজ্ঞা(box->height != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	যদি (box->object && content_get_type(box->object) == CONTENT_HTML) {
		যদি (box->descendant_x1 < content_get_width(box->object))
			box->descendant_x1 = content_get_width(box->object);
		যদি (box->descendant_y1 < content_get_height(box->object))
			box->descendant_y1 = content_get_height(box->object);
	}

	যদি (box->iframe != ফাঁকা) {
		পূর্ণসংখ্যা x, y;
		box_coords(box, &x, &y);

		browser_window_set_position(box->iframe, x, y);
		browser_window_set_dimensions(box->iframe,
				box->width, box->height);
		browser_window_reformat(box->iframe, true,
				box->width, box->height);
	}

	যদি (box->type == BOX_INLINE || box->type == BOX_TEXT)
		ফেরত;

	যদি (box->type == BOX_INLINE_END) {
		box = box->inline_end;
		জন্য (child = box->পরবর্তী; child;
				child = child->পরবর্তী) {
			যদি (child->type == BOX_FLOAT_বাম ||
					child->type == BOX_FLOAT_ডান)
				চালু;

			layout_update_descendant_bbox(unit_len_ctx, box, child,
					box->x, box->y);

			যদি (child == box->inline_end)
				বিরতি;
		}
		ফেরত;
	}

	যদি (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		ফেরত;

	জন্য (child = box->children; child; child = child->পরবর্তী) {
		যদি (child->type == BOX_FLOAT_বাম ||
				child->type == BOX_FLOAT_ডান)
			চালু;

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		যদি (box->style && css_computed_overflow_x(box->style) ==
				CSS_OVERFLOW_HIDDEN &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_HIDDEN)
			চালু;

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	জন্য (child = box->float_children; child; child = child->পরবর্তী_float) {
		প্রতিজ্ঞা(child->type == BOX_FLOAT_বাম ||
				child->type == BOX_FLOAT_ডান);

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	যদি (box->list_marker) {
		child = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height)
{
	bool ret;
	কাঠামো box *doc = content->layout;
	ধ্রুবক কাঠামো gui_layout_table *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			width, height, nsurl_access(content_get_url(
					&content->base)));

	layout_minmax_block(doc, font_func, content);

	layout_block_find_dimensions(&content->unit_len_ctx,
			width, height, 0, 0, doc);
	doc->x = doc->margin[বাম_2] + doc->border[বাম_2].width;
	doc->y = doc->margin[শীর্ষ_2] + doc->border[শীর্ষ_2].width;
	width -= doc->margin[বাম_2] + doc->border[বাম_2].width +
			doc->padding[বাম_2] + doc->padding[ডান_2] +
			doc->border[ডান_2].width + doc->margin[ডান_2];
	যদি (width < 0) {
		width = 0;
	}
	doc->width = width;

	ret = layout_block_context(doc, height, content);

	/* make <html> and <body> fill available height */
	যদি (doc->y + doc->padding[শীর্ষ_2] + doc->height + doc->padding[BOTTOM] +
			doc->border[BOTTOM].width + doc->margin[BOTTOM] <
			height) {
		doc->height = height - (doc->y + doc->padding[শীর্ষ_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].width +
				doc->margin[BOTTOM]);
		যদি (doc->children)
			doc->children->height = doc->height -
					(doc->children->margin[শীর্ষ_2] +
					 doc->children->border[শীর্ষ_2].width +
					 doc->children->padding[শীর্ষ_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].width +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	ফেরত ret;
}
