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

#شامل <assert.h>
#شامل <limits.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <math.h>
#شامل <dom/dom.h>

#شامل "utils/log.h"
#شامل "utils/talloc.h"
#شامل "utils/utils.h"
#شامل "utils/nsoption.h"
#شامل "utils/corestrings.h"
#شامل "utils/nsurl.h"
#شامل "netsurf/inttypes.h"
#شامل "netsurf/content.h"
#شامل "netsurf/browser_window.h"
#شامل "netsurf/layout.h"
#شامل "content/content.h"
#شامل "content/content_protected.h"
#شامل "css/utils.h"
#شامل "desktop/scrollbar.h"
#شامل "desktop/textarea.h"

#شامل "html/html.h"
#شامل "html/html_save.h"
#شامل "html/private.h"
#شامل "html/box.h"
#شامل "html/box_inspect.h"
#شامل "html/font.h"
#شامل "html/form_internal.h"
#شامل "html/layout.h"
#شامل "html/layout_internal.h"
#شامل "html/table.h"

/** Array of per-side access functions for computed style margins. */
ثابت css_len_func margin_funcs[4] = {
	[بالاترین_2]    = css_computed_margin_بالاترین,
	[راست_2]  = css_computed_margin_راست,
	[BOTTOM] = css_computed_margin_bottom,
	[چپ_2]   = css_computed_margin_چپ,
};

/** Array of per-side access functions for computed style paddings. */
ثابت css_len_func padding_funcs[4] = {
	[بالاترین_2]    = css_computed_padding_بالاترین,
	[راست_2]  = css_computed_padding_راست,
	[BOTTOM] = css_computed_padding_bottom,
	[چپ_2]   = css_computed_padding_چپ,
};

/** Array of per-side access functions for computed style border_widths. */
ثابت css_len_func border_width_funcs[4] = {
	[بالاترین_2]    = css_computed_border_بالاترین_width,
	[راست_2]  = css_computed_border_راست_width,
	[BOTTOM] = css_computed_border_bottom_width,
	[چپ_2]   = css_computed_border_چپ_width,
};

/** Array of per-side access functions for computed style border styles. */
ثابت css_border_style_func border_style_funcs[4] = {
	[بالاترین_2]    = css_computed_border_بالاترین_style,
	[راست_2]  = css_computed_border_راست_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[چپ_2]   = css_computed_border_چپ_style,
};

/** Array of per-side access functions for computed style border colors. */
ثابت css_border_color_func border_color_funcs[4] = {
	[بالاترین_2]    = css_computed_border_بالاترین_color,
	[راست_2]  = css_computed_border_راست_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[چپ_2]   = css_computed_border_چپ_color,
};

/* forward declaration to break cycles */
ایستا تهی layout_minmax_block(
		ساختار box *block,
		ثابت ساختار gui_layout_table *font_func,
		ثابت html_content *content);

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
ایستا تهی
layout_get_object_dimensions(ساختار box *box,
			     صحیح *width, صحیح *height,
			     صحیح min_width, صحیح max_width,
			     صحیح min_height, صحیح max_height)
{
	أكد(box->object != پوچ);
	أكد(width != پوچ && height != پوچ);

	اگر (*width == AUTO && *height == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		صحیح intrinsic_width = content_get_width(box->object);
		صحیح intrinsic_height = content_get_height(box->object);

		/* use intrinsic dimensions */
		*width = intrinsic_width;
		*height = intrinsic_height;

		/* Deal with min/max-width first */
		اگر (min_width >  0 && min_width > *width) {
			*width = min_width;
			scaled = true;
		}
		اگر (max_width >= 0 && max_width < *width) {
			*width = max_width;
			scaled = true;
		}

		اگر (scaled && (intrinsic_width != 0)) {
			/* Update height */
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		}

		scaled = false;
		/* Deal with min/max-height */
		اگر (min_height >  0 && min_height > *height) {
			*height = min_height;
			scaled = true;
		}
		اگر (max_height >= 0 && max_height < *height) {
			*height = max_height;
			scaled = true;
		}

		اگر (scaled && (intrinsic_height != 0)) {
			/* Update width */
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		}

	} وگرنه اگر (*width == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		صحیح intrinsic_width = content_get_width(box->object);
		صحیح intrinsic_height = content_get_height(box->object);

		اگر (intrinsic_height != 0)
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		وگرنه
			*width = intrinsic_width;

		اگر (min_width >  0 && min_width > *width)
			*width = min_width;
		اگر (max_width >= 0 && max_width < *width)
			*width = max_width;

	} وگرنه اگر (*height == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		صحیح intrinsic_width = content_get_width(box->object);
		صحیح intrinsic_height = content_get_height(box->object);

		اگر (min_width >  0 && min_width > *width)
			*width = min_width;
		اگر (max_width >= 0 && max_width < *width)
			*width = max_width;

		اگر (intrinsic_width != 0)
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		وگرنه
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
ایستا صحیح layout_text_indent(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style, صحیح width)
{
	css_fixed مقدار = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_text_indent(style, &مقدار, &unit);

	اگر (unit == CSS_UNIT_PCT) {
		بازگشت FPCT_OF_INT_TOINT(مقدار, width);
	} وگرنه {
		بازگشت FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				مقدار, unit));
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
ایستا تهی layout_minmax_table(ساختار box *table,
		ثابت ساختار gui_layout_table *font_func,
		ثابت html_content *content)
{
	unsigned صحیح i, j;
	صحیح width;
	صحیح border_spacing_h = 0;
	صحیح table_min = 0, table_max = 0;
	صحیح extra_fixed = 0;
	اعشاری extra_frac = 0;
	ساختار column *col;
	ساختار box *row_group, *row, *cell;

	/* check if the widths have already been calculated */
	اگر (table->max_width != UNKNOWN_MAX_WIDTH)
		بازگشت;

	اگر (table_calculate_column_types(&content->unit_len_ctx, table) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		بازگشت;
	}
	col = table->col;

	/* start with 0 except for fixed-width columns */
	برای (i = 0; i != table->columns; i++) {
		اگر (col[i].type == COLUMN_WIDTH_FIXED)
			col[i].min = col[i].max = col[i].width;
		وگرنه
			col[i].min = col[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	اگر (css_computed_border_collapse(table->style) ==
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
	برای (row_group = table->children; row_group; row_group =row_group->next)
	برای (row = row_group->children; row; row = row->next)
	برای (cell = row->children; cell; cell = cell->next) {
		أكد(cell->type == BOX_TABLE_CELL);
		أكد(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		أكد(cell->columns != 0);

		اگر (cell->columns != 1)
			ادامه;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		اگر (col[i].positioned)
			ادامه;

		/* update column min, max widths using cell widths */
		اگر (col[i].min < cell->min_width)
			col[i].min = cell->min_width;
		اگر (col[i].max < cell->max_width)
			col[i].max = cell->max_width;
	}

	/* 2nd pass: cells which span multiple columns */
	برای (row_group = table->children; row_group; row_group =row_group->next)
	برای (row = row_group->children; row; row = row->next)
	برای (cell = row->children; cell; cell = cell->next) {
		unsigned صحیح flexible_columns = 0;
		صحیح min = 0, max = 0, fixed_width = 0, extra;

		اگر (cell->columns == 1)
			ادامه;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		برای (j = 0; j != cell->columns; j++) {
			min += col[i + j].min;
			اگر (col[i + j].type == COLUMN_WIDTH_FIXED)
				fixed_width += col[i + j].width;
			وگرنه
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		اگر (min < cell->min_width) {
			اگر (flexible_columns == 0) {
				extra = 1 + (cell->min_width - min) /
						cell->columns;
				برای (j = 0; j != cell->columns; j++) {
					col[i + j].min += extra;
					اگر (col[i + j].max < col[i + j].min)
						col[i + j].max = col[i + j].min;
				}
			} وگرنه {
				extra = 1 + (cell->min_width - min) /
						flexible_columns;
				برای (j = 0; j != cell->columns; j++) {
					اگر (col[i + j].type !=
							COLUMN_WIDTH_FIXED) {
						col[i + j].min += extra;
						اگر (col[i + j].max <
								col[i + j].min)
							col[i + j].max =
								col[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		برای (j = 0; j != cell->columns; j++)
			max += col[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		اگر (max < cell->max_width && flexible_columns) {
			extra = 1 + (cell->max_width - max) / flexible_columns;
			برای (j = 0; j != cell->columns; j++)
				اگر (col[i + j].type != COLUMN_WIDTH_FIXED)
					col[i + j].max += extra;
		}
	}

	برای (i = 0; i != table->columns; i++) {
		اگر (col[i].max < col[i].min) {
			box_dump(stderr, table, 0, true);
			أكد(0);
		}
		table_min += col[i].min;
		table_max += col[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	اگر (css_computed_width_px(table->style, &content->unit_len_ctx,
			-1, &width) == CSS_WIDTH_SET) {
		اگر (table_min < width)
			table_min = width;
		اگر (table_max < width)
			table_max = width;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, چپ_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, راست_2, true, true, true,
			&extra_fixed, &extra_frac);
	اگر (extra_fixed < 0)
		extra_fixed = 0;
	اگر (extra_frac < 0)
		extra_frac = 0;
	اگر (1.0 <= extra_frac)
		extra_frac = 0.9;
	table->min_width = (table_min + extra_fixed) / (1.0 - extra_frac);
	table->max_width = (table_max + extra_fixed) / (1.0 - extra_frac);
	table->min_width += (table->columns + 1) * border_spacing_h;
	table->max_width += (table->columns + 1) * border_spacing_h;

	أكد(0 <= table->min_width && table->min_width <= table->max_width);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
ایستا درونخطی bool box_has_percentage_max_width(ساختار box *b)
{
	css_unit unit = CSS_UNIT_PX;
	شمارش css_max_width_e type;
	css_fixed مقدار = 0;

	أكد(b != پوچ);

	type = css_computed_max_width(b->style, &مقدار, &unit);
	بازگشت ((type == CSS_MAX_WIDTH_SET) && (unit == CSS_UNIT_PCT));
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
ایستا ساختار box *
layout_minmax_سطر(ساختار box *first,
		   صحیح *سطر_min,
		   صحیح *سطر_max,
		   bool first_سطر,
		   bool *سطر_has_height,
		   ثابت ساختار gui_layout_table *font_func,
		   ثابت html_content *content)
{
	صحیح min = 0, max = 0, width, height, fixed;
	اعشاری frac;
	size_t i, j;
	ساختار box *b;
	ساختار box *block;
	plot_font_style_t fstyle;
	bool no_wrap;

	أكد(first->parent);
	أكد(first->parent->parent);
	أكد(first->parent->parent->style);

	block = first->parent->parent;
	no_wrap = (css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_PRE);

	*سطر_has_height = false;

	/* corresponds to the pass 1 loop in layout_line() */
	برای (b = first; b; b = b->next) {
		شمارش css_width_e wtype;
		شمارش css_height_e htype;
		شمارش css_box_sizing_e bs;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		أكد(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		اگر (b->type == BOX_BR) {
			b = b->next;
			بشکن;
		}

		اگر (lh__box_is_float_box(b)) {
			أكد(b->children);
			اگر (b->children->type == BOX_TABLE)
				layout_minmax_table(b->children, font_func,
						content);
			وگرنه
				layout_minmax_block(b->children, font_func,
						content);
			b->min_width = b->children->min_width;
			b->max_width = b->children->max_width;
			اگر (min < b->min_width)
				min = b->min_width;
			max += b->max_width;
			ادامه;
		}

		اگر (b->type == BOX_INLINE_BLOCK || b->type == BOX_INLINE_FLEX) {
			layout_minmax_block(b, font_func, content);
			اگر (min < b->min_width)
				min = b->min_width;
			max += b->max_width;

			اگر (b->flags & HAS_HEIGHT)
				*سطر_has_height = true;
			ادامه;
		}

		أكد(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		اگر (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->style, چپ_2, true, true, true,
					&fixed, &frac);
			اگر (!b->inline_end)
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, راست_2,
						true, true, true,
						&fixed, &frac);
			اگر (0 < fixed)
				max += fixed;
			*سطر_has_height = true;
			/* \todo  update min width, consider fractional extra */
		} وگرنه اگر (b->type == BOX_INLINE_END) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->inline_end->style, راست_2,
					true, true, true,
					&fixed, &frac);
			اگر (0 < fixed)
				max += fixed;

			اگر (b->next) {
				اگر (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*سطر_has_height = true;
			ادامه;
		}

		اگر (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			اگر (!b->text)
				ادامه;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			اگر (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				اگر (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					صحیح opt_maxwidth = 0;
					ساختار form_option *o;

					برای (o = b->parent->parent->gadget->
							data.select.اقلام; o;
							o = o->next) {
						صحیح opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						اگر (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}

					b->width = opt_maxwidth;
					اگر (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;

				} وگرنه {
					font_func->width(&fstyle, b->text,
						b->length, &b->width);
					b->flags |= MEASURED;
				}
			}
			max += b->width;
			اگر (b->next) {
				اگر (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			اگر (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} وگرنه اگر (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				اگر (min < b->width)
					min = b->width;

			} وگرنه اگر (b->parent->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				انجامبده {
					برای (j = i; j != b->length &&
							b->text[j] != ' '; j++)
						;
					font_func->width(&fstyle, b->text + i,
							 j - i, &width);
					اگر (min < width)
						min = width;
					i = j + 1;
				} وقتی (j != b->length);
			}

			*سطر_has_height = true;

			ادامه;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		bs = css_computed_box_sizing(block->style);

		/* calculate box width */
		wtype = css_computed_width_px(b->style,
				&content->unit_len_ctx, -1, &width);
		اگر (wtype == CSS_WIDTH_SET) {
			اگر (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, چپ_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, راست_2,
						false, true, true,
						&fixed, &frac);
				اگر (width < fixed) {
					width = fixed;
				}
			}
			اگر (width < 0)
				width = 0;
		} وگرنه {
			width = AUTO;
		}

		/* height */
		htype = css_computed_height(b->style, &مقدار, &unit);
		اگر (htype == CSS_HEIGHT_SET) {
			height = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					مقدار, unit));
		} وگرنه {
			height = AUTO;
		}

		اگر (b->object || (b->flags & REPLACE_DIM)) {
			اگر (b->object) {
				صحیح temp_height = height;
				layout_get_object_dimensions(b,
						&width, &temp_height,
						INT_MIN, INT_MAX,
						INT_MIN, INT_MAX);
			}

			fixed = frac = 0;
			اگر (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, چپ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, راست_2,
						true, false, false,
						&fixed, &frac);
			} وگرنه {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, چپ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, راست_2,
						true, true, true,
						&fixed, &frac);
			}
			اگر (0 < width + fixed)
				width += fixed;
		} وگرنه اگر (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			اگر (width == AUTO)
				width = 400;

			fixed = frac = 0;
			اگر (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, چپ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, راست_2,
						true, false, false,
						&fixed, &frac);
			} وگرنه {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, چپ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, راست_2,
						true, true, true,
						&fixed, &frac);
			}

			اگر (0 < width + fixed)
				width += fixed;

		} وگرنه {
			/* form control with no object */
			اگر (width == AUTO)
				width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		اگر (min < width && !box_has_percentage_max_width(b))
			min = width;
		اگر (width > 0)
			max += width;

		*سطر_has_height = true;
	}

	اگر (first_سطر) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		صحیح text_indent = layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, 100);
		min = (min + text_indent < 0) ? 0 : min + text_indent;
		max = (max + text_indent < 0) ? 0 : max + text_indent;
	}

	*سطر_min = min;
	*سطر_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	أكد(b != first);
	أكد(0 <= *سطر_min);
	أكد(*سطر_min <= *سطر_max);
	بازگشت b;
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
ایستا تهی
layout_minmax_inline_container(ساختار box *inline_container,
			       bool *has_height,
			       ثابت ساختار gui_layout_table *font_func,
			       ثابت html_content *content)
{
	ساختار box *child;
	صحیح سطر_min = 0, سطر_max = 0;
	صحیح min = 0, max = 0;
	bool first_سطر = true;
	bool سطر_has_height;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	اگر (inline_container->max_width != UNKNOWN_MAX_WIDTH)
		بازگشت;

	*has_height = false;

	برای (child = inline_container->children; child; ) {
		child = layout_minmax_سطر(child, &سطر_min, &سطر_max,
				first_سطر, &سطر_has_height, font_func,
				content);
		اگر (min < سطر_min)
			min = سطر_min;
		اگر (max < سطر_max)
			max = سطر_max;
		first_سطر = false;
		*has_height |= سطر_has_height;
	}

	inline_container->min_width = min;
	inline_container->max_width = max;

	أكد(0 <= inline_container->min_width &&
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
ایستا تهی layout_minmax_block(
		ساختار box *block,
		ثابت ساختار gui_layout_table *font_func,
		ثابت html_content *content)
{
	ساختار box *child;
	صحیح min = 0, max = 0;
	صحیح extra_fixed = 0;
	اعشاری extra_frac = 0;
	شمارش css_width_e wtype = CSS_WIDTH_AUTO;
	css_fixed width = 0;
	css_unit wunit = CSS_UNIT_PX;
	شمارش css_height_e htype = CSS_HEIGHT_AUTO;
	css_fixed height = 0;
	css_unit hunit = CSS_UNIT_PX;
	شمارش css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool child_has_height = false;

	أكد(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	/* check if the widths have already been calculated */
	اگر (block->max_width != UNKNOWN_MAX_WIDTH)
		بازگشت;

	اگر (block->style != پوچ) {
		wtype = css_computed_width(block->style, &width, &wunit);
		htype = css_computed_height(block->style, &height, &hunit);
		bs = css_computed_box_sizing(block->style);
	}

	/* set whether the minimum width is of any interest for this box */
	اگر (((block->parent && lh__box_is_float_box(block->parent)) ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX) &&
			wtype != CSS_WIDTH_SET) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} وگرنه اگر (block->type == BOX_TABLE_CELL) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} وگرنه اگر (block->parent && (block->parent->flags & NEED_MIN) &&
			wtype != CSS_WIDTH_SET) {
		/* box inside shrink-to-fit context; need minimum width */
		block->flags |= NEED_MIN;
	} وگرنه اگر (block->parent && (block->parent->type == BOX_FLEX)) {
		/* box is flex item */
		block->flags |= NEED_MIN;
	}

	اگر (block->gadget && (block->gadget->type == GADGET_TEXTBOX ||
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

	اگر (block->gadget && (block->gadget->type == GADGET_RADIO ||
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

	اگر (block->object) {
		اگر (content_get_type(block->object) == CONTENT_HTML) {
			layout_minmax_block(html_get_box_درخت(block->object),
					font_func, content);
			min = html_get_box_درخت(block->object)->min_width;
			max = html_get_box_درخت(block->object)->max_width;
		} وگرنه {
			min = max = content_get_width(block->object);
		}

		block->flags |= HAS_HEIGHT;
	} وگرنه اگر (block->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		block->flags |= HAS_HEIGHT;
	} وگرنه {
		/* recurse through children */
		برای (child = block->children; child; child = child->next) {
			انتخاب (child->type) {
			حالت BOX_FLEX:
			حالت BOX_BLOCK:
				layout_minmax_block(child, font_func,
						content);
				اگر (child->flags & HAS_HEIGHT)
					child_has_height = true;
				بشکن;
			حالت BOX_INLINE_CONTAINER:
				اگر (block->flags & NEED_MIN)
					child->flags |= NEED_MIN;

				layout_minmax_inline_container(child,
						&child_has_height, font_func,
						content);
				اگر (child_has_height &&
						child ==
						child->parent->children) {
					block->flags |= MAKE_HEIGHT;
				}
				بشکن;
			حالت BOX_TABLE:
				layout_minmax_table(child, font_func,
						content);
				/* todo: fix for zero height tables */
				child_has_height = true;
				child->flags |= MAKE_HEIGHT;
				بشکن;
			پیشفرض:
				أكد(0);
			}
			أكد(child->max_width != UNKNOWN_MAX_WIDTH);

			اگر (child->style &&
					(css_computed_position(child->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(child->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				ادامه;
			}

			اگر (lh__box_is_flex_container(block) &&
			    lh__flex_main_is_horizontal(block)) {
				اگر (block->style != پوچ &&
				    css_computed_flex_wrap(block->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += child->min_width;
				} وگرنه {
					اگر (min < child->min_width)
						min = child->min_width;
				}
				max += child->max_width;

			} وگرنه {
				اگر (min < child->min_width)
					min = child->min_width;
				اگر (max < child->max_width)
					max = child->max_width;
			}

			اگر (child_has_height)
				block->flags |= HAS_HEIGHT;
		}
	}

	اگر (max < min) {
		box_dump(stderr, block, 0, true);
		أكد(0);
	}

	/* fixed width takes priority */
	اگر (block->type != BOX_TABLE_CELL && !lh__box_is_flex_item(block)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		شمارش css_max_width_e max_type;
		شمارش css_min_width_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed مقدار = 0;
		صحیح width;

		اگر (css_computed_width_px(block->style, &content->unit_len_ctx,
				-1, &width) == CSS_WIDTH_SET) {
			min = max = width;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_width(block->style, &مقدار, &unit);
		اگر (min_type == CSS_MIN_WIDTH_SET && unit != CSS_UNIT_PCT) {
			صحیح val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, مقدار, unit));

			اگر (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_width(block->style, &مقدار, &unit);
		اگر (max_type == CSS_MAX_WIDTH_SET && unit != CSS_UNIT_PCT) {
			صحیح val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, مقدار, unit));

			اگر (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	اگر (htype == CSS_HEIGHT_SET && hunit != CSS_UNIT_PCT &&
			height > INTTOFIX(0)) {
		block->flags |= MAKE_HEIGHT;
		block->flags |= HAS_HEIGHT;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_width(&content->unit_len_ctx, block->style, چپ_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, راست_2,
			false, true, true, &extra_fixed, &extra_frac);

	اگر (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	اگر (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	اگر (max < min) {
		min = max;
	}

	calculate_mbp_width(&content->unit_len_ctx, block->style, چپ_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, راست_2,
			true, false, false, &extra_fixed, &extra_frac);

	اگر (extra_fixed < 0)
		extra_fixed = 0;
	اگر (extra_frac < 0)
		extra_frac = 0;
	اگر (1.0 <= extra_frac)
		extra_frac = 0.9;
	اگر (block->style != پوچ &&
			(css_computed_float(block->style) == CSS_FLOAT_چپ ||
			css_computed_float(block->style) == CSS_FLOAT_راست)) {
		/* floated boxs */
		block->min_width = min + extra_fixed;
		block->max_width = max + extra_fixed;
	} وگرنه {
		/* not floated */
		block->min_width = (min + extra_fixed) / (1.0 - extra_frac);
		block->max_width = (max + extra_fixed) / (1.0 - extra_frac);
	}

	أكد(0 <= block->min_width);
	أكد(block->min_width <= block->max_width);
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
ایستا ساختار box*
layout_next_margin_block(ثابت css_unit_ctx *unit_len_ctx,
			 ساختار box *box,
			 ساختار box *block,
			 صحیح viewport_height,
			 صحیح *max_pos_margin,
			 صحیح *max_neg_margin)
{
	أكد(block != پوچ);

	وقتی (box != پوچ) {

		اگر (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			اگر (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						پوچ, پوچ, پوچ, پوچ,
						پوچ, پوچ, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				اگر (*max_pos_margin < box->margin[بالاترین_2])
					*max_pos_margin = box->margin[بالاترین_2];
				وگرنه اگر (*max_neg_margin < -box->margin[بالاترین_2])
					*max_neg_margin = -box->margin[بالاترین_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			اگر (box->flags & MAKE_HEIGHT ||
					box->border[بالاترین_2].width ||
					box->padding[بالاترین_2] ||
					(box->style &&
					css_computed_overflow_y(box->style) !=
					CSS_OVERFLOW_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_first_child(box))) {
				/* Collapse to this box; return it */
				بازگشت box;
			}
		}


		/* Find next box */
		اگر (box->type == BOX_BLOCK && !box->object && box->children &&
				box->style &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} وگرنه {
			اگر (!box->next) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				انجامبده {
					/* Apply bottom margin */
					اگر (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					وگرنه اگر (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->parent;
				} وقتی (box != block && !box->next);

				اگر (box == block) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					بازگشت block;
				}
			}

			/* Apply bottom margin */
			اگر (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			وگرنه اگر (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->next;

			/* Get margins */
			اگر (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						پوچ, پوچ, پوچ, پوچ,
						پوچ, پوچ, box->margin,
						box->padding, box->border);
			}
		}
	}

	بازگشت پوچ;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
ایستا صحیح layout_clear(ساختار box *fl, شمارش css_clear_e clear)
{
	صحیح y = 0;
	برای (; fl; fl = fl->next_float) {
		اگر ((clear == CSS_CLEAR_چپ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_چپ)
			اگر (y < fl->y + fl->height)
				y = fl->y + fl->height;
		اگر ((clear == CSS_CLEAR_راست || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_راست)
			اگر (y < fl->y + fl->height)
				y = fl->y + fl->height;
	}
	بازگشت y;
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
ایستا تهی
find_sides(ساختار box *fl,
	   صحیح y0, صحیح y1,
	   صحیح *x0, صحیح *x1,
	   ساختار box **چپ,
	   ساختار box **راست)
{
	صحیح fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*چپ = *راست = 0;
	برای (; fl; fl = fl->next_float) {
		fy1 = fl->y + fl->height;
		اگر (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			بازگشت;
		}
		fy0 = fl->y;
		اگر (y0 < fy1 && fy0 <= y1) {
			اگر (fl->type == BOX_FLOAT_چپ) {
				fx1 = fl->x + fl->width;
				اگر (*x0 < fx1) {
					*x0 = fx1;
					*چپ = fl;
				}
			} وگرنه {
				fx0 = fl->x;
				اگر (fx0 < *x1) {
					*x1 = fx0;
					*راست = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *چپ, *راست);
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
ایستا صحیح
layout_solve_width(ساختار box *box,
		   صحیح available_width,
		   صحیح width,
		   صحیح lm,
		   صحیح rm,
		   صحیح max_width,
		   صحیح min_width)
{
	bool auto_width = false;

	/* Increase specified left/right margins */
	اگر (box->margin[چپ_2] != AUTO && box->margin[چپ_2] < lm &&
			box->margin[چپ_2] >= 0)
		box->margin[چپ_2] = lm;
	اگر (box->margin[راست_2] != AUTO && box->margin[راست_2] < rm &&
			box->margin[راست_2] >= 0)
		box->margin[راست_2] = rm;

	/* Find width */
	اگر (width == AUTO) {
		صحیح margin_چپ = box->margin[چپ_2];
		صحیح margin_راست = box->margin[راست_2];

		اگر (margin_چپ == AUTO) {
			margin_چپ = lm;
		}
		اگر (margin_راست == AUTO) {
			margin_راست = rm;
		}

		width = available_width -
				(margin_چپ + box->border[چپ_2].width +
				box->padding[چپ_2] + box->padding[راست_2] +
				box->border[راست_2].width + margin_راست);
		width = width < 0 ? 0 : width;
		auto_width = true;
	}

	اگر (max_width >= 0 && width > max_width) {
		/* max-width is admissable and width exceeds max-width */
		width = max_width;
		auto_width = false;
	}

	اگر (min_width > 0 && width < min_width) {
		/* min-width is admissable and width is less than max-width */
		width = min_width;
		auto_width = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	اگر (auto_width) {
		/* any other 'auto' become 0 or the minimum required values */
		اگر (box->margin[چپ_2] == AUTO) {
			box->margin[چپ_2] = lm;
		}
		اگر (box->margin[راست_2] == AUTO) {
			box->margin[راست_2] = rm;
		}
		بازگشت width;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	اگر (box->margin[چپ_2] != AUTO && box->margin[راست_2] != AUTO &&
			box->parent != پوچ && box->parent->style != پوچ) {
		انتخاب (css_computed_text_align(box->parent->style)) {
		حالت CSS_TEXT_ALIGN_LIBCSS_راست:
			box->margin[چپ_2] = AUTO;
			box->margin[راست_2] = 0;
			بشکن;
		حالت CSS_TEXT_ALIGN_LIBCSS_CENTER:
			box->margin[چپ_2] = box->margin[راست_2] = AUTO;
			بشکن;
		حالت CSS_TEXT_ALIGN_LIBCSS_چپ:
			box->margin[چپ_2] = 0;
			box->margin[راست_2] = AUTO;
			بشکن;
		پیشفرض:
			/* Leave it alone; no HTML alignment */
			بشکن;
		}
	}

	اگر (box->margin[چپ_2] == AUTO && box->margin[راست_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[چپ_2] = box->margin[راست_2] =
				(available_width - lm - rm -
				(box->border[چپ_2].width + box->padding[چپ_2] +
				width + box->padding[راست_2] +
				box->border[راست_2].width)) / 2;

		اگر (box->margin[چپ_2] < 0) {
			box->margin[راست_2] += box->margin[چپ_2];
			box->margin[چپ_2] = 0;
		}

		box->margin[چپ_2] += lm;

	} وگرنه اگر (box->margin[چپ_2] == AUTO) {
		box->margin[چپ_2] = available_width - lm -
				(box->border[چپ_2].width + box->padding[چپ_2] +
				width + box->padding[راست_2] +
				box->border[راست_2].width + box->margin[راست_2]);
		box->margin[چپ_2] = box->margin[چپ_2] < lm
				? lm : box->margin[چپ_2];
	} وگرنه {
		/* margin-right auto or "over-constrained" */
		box->margin[راست_2] = available_width - rm -
				(box->margin[چپ_2] + box->border[چپ_2].width +
				 box->padding[چپ_2] + width +
				 box->padding[راست_2] +
				 box->border[راست_2].width);
	}

	بازگشت width;
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
ایستا تهی
layout_block_find_dimensions(ثابت css_unit_ctx *unit_len_ctx,
			     صحیح available_width,
			     صحیح viewport_height,
			     صحیح lm,
			     صحیح rm,
			     ساختار box *box)
{
	صحیح width, max_width, min_width;
	صحیح height, max_height, min_height;
	صحیح *margin = box->margin;
	صحیح *padding = box->padding;
	ساختار box_border *border = box->border;
	ثابت css_computed_style *style = box->style;

	layout_find_dimensions(unit_len_ctx, available_width, viewport_height, box,
			style, &width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	اگر (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	}

	box->width = layout_solve_width(box, available_width, width, lm, rm,
			max_width, min_width);
	box->height = height;

	اگر (margin[بالاترین_2] == AUTO)
		margin[بالاترین_2] = 0;
	اگر (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
ایستا تهی layout_block_add_scrollbar(ساختار box *box, صحیح which)
{
	شمارش css_overflow_e overflow_x, overflow_y;

	أكد(box->type == BOX_BLOCK && (which == راست_2 || which == BOTTOM));

	اگر (box->style == پوچ)
		بازگشت;

	overflow_x = css_computed_overflow_x(box->style);
	overflow_y = css_computed_overflow_y(box->style);

	اگر (which == BOTTOM &&
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		اگر (box->height != AUTO &&
				(overflow_x == CSS_OVERFLOW_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_WIDTH;
		}

	} وگرنه اگر (which == راست_2 &&
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		شمارش css_height_e htype;
		css_fixed height = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_height(box->style, &height, &hunit);

		اگر (which == راست_2 && box->width != AUTO &&
				htype == CSS_HEIGHT_SET &&
				(overflow_y == CSS_OVERFLOW_SCROLL ||
				box_vscrollbar_present(box))) {
			box->width -= SCROLLBAR_WIDTH;
			box->padding[راست_2] += SCROLLBAR_WIDTH;
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
ایستا تهی layout_move_children(ساختار box *box, صحیح x, صحیح y)
{
	أكد(box);

	برای (box = box->children; box; box = box->next) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_table(
		ساختار box *table,
		صحیح available_width,
		html_content *content)
{
	unsigned صحیح columns = table->columns;  /* total columns */
	unsigned صحیح i;
	unsigned صحیح *row_span;
	صحیح *excess_y;
	صحیح table_width, min_width = 0, max_width = 0;
	صحیح required_width = 0;
	صحیح x, remainder = 0, شمار = 0;
	صحیح table_height = 0;
	صحیح min_height = 0;
	صحیح *xs;  /* array of column x positions */
	صحیح auto_width;
	صحیح spare_width;
	صحیح relative_sum = 0;
	صحیح border_spacing_h = 0, border_spacing_v = 0;
	صحیح spare_height;
	صحیح positioned_columns = 0;
	ساختار box *containing_block = پوچ;
	ساختار box *c;
	ساختار box *row;
	ساختار box *row_group;
	ساختار box **row_span_cell;
	ساختار column *col;
	ثابت css_computed_style *style = table->style;
	شمارش css_height_e htype;
	css_fixed مقدار = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(table->type == BOX_TABLE);
	أكد(style);
	أكد(table->children && table->children->children);
	أكد(columns);

	/* allocate working buffers */
	col = malloc(columns * اندازه col[0]);
	excess_y = malloc(columns * اندازه excess_y[0]);
	row_span = malloc(columns * اندازه row_span[0]);
	row_span_cell = malloc(columns * اندازه row_span_cell[0]);
	xs = malloc((columns + 1) * اندازه xs[0]);
	اگر (!col || !xs || !row_span || !excess_y || !row_span_cell) {
		آزاد(col);
		آزاد(excess_y);
		آزاد(row_span);
		آزاد(row_span_cell);
		آزاد(xs);
		بازگشت false;
	}

	memcpy(col, table->col, اندازه(col[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1, table,
			style, 0, 0, 0, 0, 0, 0, table->margin, table->padding,
			table->border);
	برای (row_group = table->children; row_group;
			row_group = row_group->next) {
		برای (row = row_group->children; row; row = row->next) {
			برای (c = row->children; c; c = c->next) {
				شمارش css_overflow_e overflow_x;
				شمارش css_overflow_e overflow_y;

				أكد(c->style);
				table_used_border_for_cell(
						&content->unit_len_ctx, c);
				layout_find_dimensions(&content->unit_len_ctx,
						available_width, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				overflow_x = css_computed_overflow_x(c->style);
				overflow_y = css_computed_overflow_y(c->style);

				اگر (overflow_x == CSS_OVERFLOW_SCROLL ||
						overflow_x ==
						CSS_OVERFLOW_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_WIDTH;
				}
				اگر (overflow_y == CSS_OVERFLOW_SCROLL ||
						overflow_y ==
						CSS_OVERFLOW_AUTO) {
					c->padding[راست_2] += SCROLLBAR_WIDTH;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	اگر (css_computed_border_collapse(style) ==
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
	اگر (css_computed_width_px(style, &content->unit_len_ctx,
			available_width, &table_width) == CSS_WIDTH_SET) {
		/* specified width includes border */
		table_width -= table->border[چپ_2].width +
				table->border[راست_2].width;
		table_width = table_width < 0 ? 0 : table_width;

		auto_width = table_width;
	} وگرنه {
		table_width = AUTO;
		auto_width = available_width -
				((table->margin[چپ_2] == AUTO ? 0 :
						table->margin[چپ_2]) +
				 table->border[چپ_2].width +
				 table->padding[چپ_2] +
				 table->padding[راست_2] +
				 table->border[راست_2].width +
				 (table->margin[راست_2] == AUTO ? 0 :
						table->margin[راست_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_height(style, &مقدار, &unit);
	اگر (htype == CSS_HEIGHT_SET) {
		اگر (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			اگر (css_computed_position(table->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				أكد(table->float_container);
				containing_block = table->float_container;
			} وگرنه اگر (table->float_container &&
					css_computed_position(table->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(table->style) ==
						CSS_FLOAT_چپ ||
					css_computed_float(table->style) ==
						CSS_FLOAT_راست)) {
				/* Table is a float */
				أكد(table->parent && table->parent->parent &&
						table->parent->parent->parent);
				containing_block =
						table->parent->parent->parent;
			} وگرنه اگر (table->parent && table->parent->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_block = table->parent;
			} وگرنه اگر (table->parent && table->parent->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				أكد(table->parent->parent);
				containing_block = table->parent->parent;
			}

			اگر (containing_block) {
				css_fixed ignored = 0;

				htype = css_computed_height(
						containing_block->style,
						&ignored, &unit);
			}

			اگر (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(table->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_HEIGHT_SET)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_height = FPCT_OF_INT_TOINT(مقدار,
						containing_block->height);
			}
		} وگرنه {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_height = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					مقدار, unit));
		}
	}

	/* calculate width required by cells */
	برای (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      table,
		      i,
		      ((ثابت نویسه *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[col[i].type],
		      col[i].width,
		      col[i].min,
		      col[i].max);


		اگر (col[i].positioned) {
			positioned_columns++;
			ادامه;
		} وگرنه اگر (col[i].type == COLUMN_WIDTH_FIXED) {
			اگر (col[i].width < col[i].min)
				col[i].width = col[i].max = col[i].min;
			وگرنه
				col[i].min = col[i].max = col[i].width;
			required_width += col[i].width;
		} وگرنه اگر (col[i].type == COLUMN_WIDTH_PERCENT) {
			صحیح width = col[i].width * auto_width / 100;
			required_width += col[i].min < width ? width :
					col[i].min;
		} وگرنه
			required_width += col[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_width);
	}
	required_width += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", table_width,
	      table->min_width, table->max_width, auto_width, required_width);

	اگر (auto_width < required_width) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		برای (i = 0; i != columns; i++) {
			اگر (col[i].type == COLUMN_WIDTH_RELATIVE)
				ادامه;
			اگر (col[i].type == COLUMN_WIDTH_PERCENT) {
				col[i].max = auto_width * col[i].width / 100;
				اگر (col[i].max < col[i].min)
					col[i].max = col[i].min;
			}
			min_width += col[i].min;
			max_width += col[i].max;
		}
	} وگرنه {
		/* take percentages exactly */
		برای (i = 0; i != columns; i++) {
			اگر (col[i].type == COLUMN_WIDTH_RELATIVE)
				ادامه;
			اگر (col[i].type == COLUMN_WIDTH_PERCENT) {
				صحیح width = auto_width * col[i].width / 100;
				اگر (width < col[i].min)
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
	برای (i = 0; i != columns; i++) {
		اگر (col[i].type == COLUMN_WIDTH_RELATIVE)
			relative_sum += col[i].width;
		وگرنه اگر (col[i].type == COLUMN_WIDTH_FIXED)
			spare_width -= col[i].width;
		وگرنه
			spare_width -= col[i].min;
	}
	spare_width -= (columns + 1) * border_spacing_h;
	اگر (relative_sum != 0) {
		اگر (spare_width < 0)
			spare_width = 0;
		برای (i = 0; i != columns; i++) {
			اگر (col[i].type == COLUMN_WIDTH_RELATIVE) {
				col[i].min = ceil(col[i].max =
						(اعشاری) spare_width
						* (اعشاری) col[i].width
						/ relative_sum);
				min_width += col[i].min;
				max_width += col[i].max;
			}
		}
	}
	min_width += (columns + 1) * border_spacing_h;
	max_width += (columns + 1) * border_spacing_h;

	اگر (auto_width <= min_width) {
		/* not enough space: minimise column widths */
		برای (i = 0; i < columns; i++) {
			col[i].width = col[i].min;
		}
		table_width = min_width;
	} وگرنه اگر (max_width <= auto_width) {
		/* more space than maximum width */
		اگر (table_width == AUTO) {
			/* for auto-width tables, make columns max width */
			برای (i = 0; i < columns; i++) {
				col[i].width = col[i].max;
			}
			table_width = max_width;
		} وگرنه {
			/* for fixed-width tables, distribute the extra space
			 * too */
			unsigned صحیح flexible_columns = 0;
			برای (i = 0; i != columns; i++)
				اگر (col[i].type != COLUMN_WIDTH_FIXED)
					flexible_columns++;
			اگر (flexible_columns == 0) {
				صحیح extra = (table_width - max_width) / columns;
				remainder = (table_width - max_width) -
						(extra * columns);
				برای (i = 0; i != columns; i++) {
					col[i].width = col[i].max + extra;
					شمار -= remainder;
					اگر (شمار < 0) {
						col[i].width++;
						شمار += columns;
					}
				}

			} وگرنه {
				صحیح extra = (table_width - max_width) /
						flexible_columns;
				remainder = (table_width - max_width) -
						(extra * flexible_columns);
				برای (i = 0; i != columns; i++)
					اگر (col[i].type != COLUMN_WIDTH_FIXED) {
						col[i].width = col[i].max +
								extra;
						شمار -= remainder;
						اگر (شمار < 0) {
							col[i].width++;
							شمار += flexible_columns;
						}
					}
			}
		}
	} وگرنه {
		/* space between min and max: fill it exactly */
		اعشاری scale = (اعشاری) (auto_width - min_width) /
				(اعشاری) (max_width - min_width);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		برای (i = 0; i < columns; i++) {
			col[i].width = col[i].min + (صحیح) (0.5 +
					(col[i].max - col[i].min) * scale);
		}
		table_width = auto_width;
	}

	xs[0] = x = border_spacing_h;
	برای (i = 0; i != columns; i++) {
		اگر (!col[i].positioned)
			x += col[i].width + border_spacing_h;
		xs[i + 1] = x;
		row_span[i] = 0;
		excess_y[i] = 0;
		row_span_cell[i] = 0;
	}

	/* position cells */
	table_height = border_spacing_v;
	برای (row_group = table->children; row_group;
			row_group = row_group->next) {
		صحیح row_group_height = 0;
		برای (row = row_group->children; row; row = row->next) {
			صحیح row_height = 0;

			htype = css_computed_height(row->style, &مقدار, &unit);
			اگر (htype == CSS_HEIGHT_SET && unit != CSS_UNIT_PCT) {
				row_height = FIXTOINT(css_unit_len2device_px(
						row->style,
						&content->unit_len_ctx,
						مقدار, unit));
			}
			برای (c = row->children; c; c = c->next) {
				أكد(c->style);
				c->width = xs[c->start_column + c->columns] -
						xs[c->start_column] -
						border_spacing_h -
						c->border[چپ_2].width -
						c->padding[چپ_2] -
						c->padding[راست_2] -
						c->border[راست_2].width;
				c->float_children = 0;
				c->cached_place_below_level = 0;

				c->height = AUTO;
				اگر (!layout_block_context(c, -1, content)) {
					آزاد(col);
					آزاد(excess_y);
					آزاد(row_span);
					آزاد(row_span_cell);
					آزاد(xs);
					بازگشت false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->height;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_height(c->style,
						&مقدار, &unit);

				اگر (htype == CSS_HEIGHT_SET &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					صحیح h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							مقدار, unit));
					اگر (c->height < h)
						c->height = h;
				}
				/* specified row height is treated as a minimum
				 */
				اگر (c->height < row_height)
					c->height = row_height;
				c->x = xs[c->start_column] +
						c->border[چپ_2].width;
				c->y = c->border[بالاترین_2].width;
				برای (i = 0; i != c->columns; i++) {
					row_span[c->start_column + i] = c->rows;
					excess_y[c->start_column + i] =
							c->border[بالاترین_2].width +
							c->padding[بالاترین_2] +
							c->height +
							c->padding[BOTTOM] +
							c->border[BOTTOM].width;
					row_span_cell[c->start_column + i] = 0;
				}
				row_span_cell[c->start_column] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[بالاترین_2].width -
						c->padding[بالاترین_2] -
						c->height -
						c->border[BOTTOM].width;
			}
			برای (i = 0; i != columns; i++)
				اگر (row_span[i] != 0)
					row_span[i]--;
				وگرنه
					row_span_cell[i] = 0;
			اگر (row->next || row_group->next) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				برای (i = 0; i != columns; i++)
					اگر (row_span[i] == 0 && row_height <
							excess_y[i])
						row_height = excess_y[i];
			} وگرنه {
				/* except in the last row */
				برای (i = 0; i != columns; i++)
					اگر (row_height < excess_y[i])
						row_height = excess_y[i];
			}
			برای (i = 0; i != columns; i++) {
				اگر (row_height < excess_y[i])
					excess_y[i] -= row_height;
				وگرنه
					excess_y[i] = 0;
				اگر (row_span_cell[i] != 0)
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
	برای (row_group = table->children; row_group;
			row_group = row_group->next) {
		برای (row = row_group->children; row; row = row->next) {
			برای (c = row->children; c; c = c->next) {
				شمارش css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				spare_height = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->height - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &مقدار, &unit);

				انتخاب (vertical_align) {
				حالت CSS_VERTICAL_ALIGN_SUB:
				حالت CSS_VERTICAL_ALIGN_SUPER:
				حالت CSS_VERTICAL_ALIGN_TEXT_بالاترین:
				حالت CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				حالت CSS_VERTICAL_ALIGN_SET:
				حالت CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				حالت CSS_VERTICAL_ALIGN_بالاترین:
					بشکن;
				حالت CSS_VERTICAL_ALIGN_میانه:
					c->padding[بالاترین_2] += spare_height / 2;
					c->padding[BOTTOM] -= spare_height / 2;
					layout_move_children(c, 0,
							spare_height / 2);
					بشکن;
				حالت CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[بالاترین_2] += spare_height;
					c->padding[BOTTOM] -= spare_height;
					layout_move_children(c, 0,
							spare_height);
					بشکن;
				حالت CSS_VERTICAL_ALIGN_INHERIT:
					أكد(0);
					بشکن;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	اگر (table->margin[بالاترین_2] == AUTO)
		table->margin[بالاترین_2] = 0;
	اگر (table->margin[BOTTOM] == AUTO)
		table->margin[BOTTOM] = 0;

	آزاد(col);
	آزاد(excess_y);
	آزاد(row_span);
	آزاد(row_span_cell);
	آزاد(xs);

	table->width = table_width;
	table->height = table_height;

	بازگشت true;
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
ایستا bool layout_apply_minmax_height(
		ثابت css_unit_ctx *unit_len_ctx,
		ساختار box *box,
		ساختار box *container)
{
	صحیح h;
	ساختار box *containing_block = پوچ;
	bool updated = false;

	/* Find containing block for percentage heights */
	اگر (box->style != پوچ && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		أكد(container);
		containing_block = container;
	} وگرنه اگر (box->float_container && box->style != پوچ &&
			(css_computed_float(box->style) == CSS_FLOAT_چپ ||
			 css_computed_float(box->style) == CSS_FLOAT_راست)) {
		/* Box is a float */
		أكد(box->parent && box->parent->parent &&
				box->parent->parent->parent);
		containing_block = box->parent->parent->parent;
	} وگرنه اگر (box->parent && box->parent->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_block = box->parent;
	} وگرنه اگر (box->parent && box->parent->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		أكد(box->parent->parent);
		containing_block = box->parent->parent;
	}

	اگر (box->style) {
		شمارش css_height_e htype = CSS_HEIGHT_AUTO;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		اگر (containing_block) {
			htype = css_computed_height(containing_block->style,
					&مقدار, &unit);
		}

		/* max-height */
		اگر (css_computed_max_height(box->style, &مقدار, &unit) ==
				CSS_MAX_HEIGHT_SET) {
			اگر (unit == CSS_UNIT_PCT) {
				اگر (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(مقدار,
						containing_block->height);
					اگر (h < box->height) {
						box->height = h;
						updated = true;
					}
				}
			} وگرنه {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						مقدار, unit));
				اگر (h < box->height) {
					box->height = h;
					updated = true;
				}
			}
		}

		/* min-height */
		اگر (ns_computed_min_height(box->style, &مقدار, &unit) ==
				CSS_MIN_HEIGHT_SET) {
			اگر (unit == CSS_UNIT_PCT) {
				اگر (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(مقدار,
						containing_block->height);
					اگر (h > box->height) {
						box->height = h;
						updated = true;
					}
				}
			} وگرنه {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						مقدار, unit));
				اگر (h > box->height) {
					box->height = h;
					updated = true;
				}
			}
		}
	}
	بازگشت updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
ایستا bool layout_block_object(ساختار box *block)
{
	أكد(block);
	أكد(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_TABLE ||
			block->type == BOX_TABLE_CELL);
	أكد(block->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", block,
	      hlcache_handle_get_url(block->object), block->width);

	اگر (content_can_reformat(block->object)) {
		content_reformat(block->object, false, block->width, 1);
	} وگرنه {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	بازگشت true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
ایستا تهی add_float_to_container(ساختار box *cont, ساختار box *b)
{
	ساختار box *box = cont->float_children;
	صحیح b_bottom = b->y + b->height;

	أكد(b->type == BOX_FLOAT_چپ || b->type == BOX_FLOAT_راست);

	اگر (box == پوچ) {
		/* No other float children */
		b->next_float = پوچ;
		cont->float_children = b;
		بازگشت;
	} وگرنه اگر (b_bottom >= box->y + box->height) {
		/* Goes at start of list */
		b->next_float = cont->float_children;
		cont->float_children = b;
	} وگرنه {
		ساختار box *prev = پوچ;
		وقتی (box != پوچ && b_bottom < box->y + box->height) {
			prev = box;
			box = box->next_float;
		}
		اگر (prev != پوچ) {
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
ایستا bool
layout_text_box_split(html_content *content,
		      plot_font_style_t *fstyle,
		      ساختار box *split_box,
		      size_t new_length,
		      صحیح new_width)
{
	صحیح space_width = split_box->space;
	ساختار box *c2;
	ثابت ساختار gui_layout_table *font_func = content->font_func;
	bool space = (split_box->text[new_length] == ' ');
	صحیح used_length = new_length + (space ? 1 : 0);

	اگر ((space && space_width == 0) || space_width == UNKNOWN_WIDTH) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->width(fstyle, " ", 1, &space_width);
	}

	اگر (split_box->space == UNKNOWN_WIDTH)
		split_box->space = space_width;
	اگر (!space)
		space_width = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, split_box, اندازه *c2);
	اگر (!c2)
		بازگشت false;
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
	اگر (c2->next)
		c2->next->prev = c2;
	وگرنه
		c2->parent->last = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      split_box,
	      split_box->length,
	      (صحیح)split_box->length,
	      split_box->text);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->length,
	      (صحیح)c2->length,
	      c2->text);

	بازگشت true;
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
ایستا تهی
layout_float_find_dimensions(
		ثابت css_unit_ctx *unit_len_ctx,
		صحیح available_width,
		ثابت css_computed_style *style,
		ساختار box *box)
{
	صحیح width, height, max_width, min_width, max_height, min_height;
	صحیح *margin = box->margin;
	صحیح *padding = box->padding;
	ساختار box_border *border = box->border;
	شمارش css_overflow_e overflow_x = css_computed_overflow_x(style);
	شمارش css_overflow_e overflow_y = css_computed_overflow_y(style);
	صحیح scrollbar_width_x =
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;
	صحیح scrollbar_width_y =
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;

	layout_find_dimensions(unit_len_ctx, available_width, -1, box, style,
			&width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	اگر (margin[چپ_2] == AUTO)
		margin[چپ_2] = 0;
	اگر (margin[راست_2] == AUTO)
		margin[راست_2] = 0;

	اگر (box->gadget == پوچ) {
		padding[راست_2] += scrollbar_width_y;
		padding[BOTTOM] += scrollbar_width_x;
	}

	اگر (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	} وگرنه اگر (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed size = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		أكد(box->style);

		اگر (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			اگر (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			اگر (box->gadget->type == GADGET_FILE &&
					height == AUTO) {
				size = FLTTOFIX(1.5);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
		اگر (box->gadget->type == GADGET_TEXTAREA) {
			اگر (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			اگر (height == AUTO) {
				size = INTTOFIX(4);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
	} وگرنه اگر (width == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		width = min(max(box->min_width, available_width),
				box->max_width);

		/* width includes margin, borders and padding */
		اگر (width == available_width) {
			width -= box->margin[چپ_2] + box->border[چپ_2].width +
					box->padding[چپ_2] +
					box->padding[راست_2] +
					box->border[راست_2].width +
					box->margin[راست_2];
		} وگرنه {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			صحیح fixed = 0;
			اعشاری frac = 0;
			calculate_mbp_width(unit_len_ctx, box->style, چپ_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_width(unit_len_ctx, box->style, راست_2,
					true, true, true, &fixed, &frac);
			اگر (fixed < 0)
				fixed = 0;

			width -= fixed;
		}

		اگر (max_width >= 0 && width > max_width) width = max_width;
		اگر (min_width >  0 && width < min_width) width = min_width;

	} وگرنه {
		اگر (max_width >= 0 && width > max_width) width = max_width;
		اگر (min_width >  0 && width < min_width) width = min_width;
		width -= scrollbar_width_y;
	}

	box->width = width;
	box->height = height;

	اگر (margin[بالاترین_2] == AUTO)
		margin[بالاترین_2] = 0;
	اگر (margin[BOTTOM] == AUTO)
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
ایستا bool layout_float(ساختار box *b, صحیح width, html_content *content)
{
	أكد(b->type == BOX_TABLE ||
	       b->type == BOX_BLOCK ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_find_dimensions(&content->unit_len_ctx, width, b->style, b);
	اگر (b->type == BOX_TABLE || b->type == BOX_INLINE_FLEX) {
		اگر (b->type == BOX_TABLE) {
			اگر (!layout_table(b, width, content))
				بازگشت false;
		} وگرنه {
			اگر (!layout_flex(b, width, content))
				بازگشت false;
		}
		اگر (b->margin[چپ_2] == AUTO)
			b->margin[چپ_2] = 0;
		اگر (b->margin[راست_2] == AUTO)
			b->margin[راست_2] = 0;
		اگر (b->margin[بالاترین_2] == AUTO)
			b->margin[بالاترین_2] = 0;
		اگر (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} وگرنه {
		بازگشت layout_block_context(b, -1, content);
	}
	بازگشت true;
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
ایستا تهی
place_float_below(ساختار box *c, صحیح width, صحیح cx, صحیح y, ساختار box *cont)
{
	صحیح x0, x1, yy;
	ساختار box *چپ;
	ساختار box *راست;

	yy = y > cont->cached_place_below_level ?
			y : cont->cached_place_below_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      width, cx, y, cont);

	انجامبده {
		y = yy;
		x0 = cx;
		x1 = cx + width;
		find_sides(cont->float_children, y, y + c->height, &x0, &x1,
				&چپ, &راست);
		اگر (چپ != 0 && راست != 0) {
			yy = (چپ->y + چپ->height <
					راست->y + راست->height ?
					چپ->y + چپ->height :
					راست->y + راست->height);
		} وگرنه اگر (چپ == 0 && راست != 0) {
			yy = راست->y + راست->height;
		} وگرنه اگر (چپ != 0 && راست == 0) {
			yy = چپ->y + چپ->height;
		}
	} وقتی ((چپ != 0 || راست != 0) && (c->width > x1 - x0));

	اگر (c->type == BOX_FLOAT_چپ) {
		c->x = x0;
	} وگرنه {
		c->x = x1 - c->width;
	}
	c->y = y;
	cont->cached_place_below_level = y;
}


/**
 * Calculate line height from a style.
 */
ایستا صحیح سطر_height(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style)
{
	شمارش css_سطر_height_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed سطر_height;

	أكد(style);

	lhtype = css_computed_سطر_height(style, &lhvalue, &lhunit);
	اگر (lhtype == CSS_سطر_HEIGHT_NORMAL) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_سطر_HEIGHT_عدد;
	}

	اگر (lhtype == CSS_سطر_HEIGHT_عدد ||
			lhunit == CSS_UNIT_PCT) {
		سطر_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		اگر (lhtype != CSS_سطر_HEIGHT_عدد)
			سطر_height = FDIV(سطر_height, F_100);
	} وگرنه {
		أكد(lhunit != CSS_UNIT_PCT);

		سطر_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	بازگشت FIXTOINT(سطر_height);
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
ایستا bool
layout_سطر(ساختار box *first,
	    صحیح *width,
	    صحیح *y,
	    صحیح cx,
	    صحیح cy,
	    ساختار box *cont,
	    bool indent,
	    bool has_text_children,
	    html_content *content,
	    ساختار box **next_box)
{
	صحیح height, used_height;
	صحیح x0 = 0;
	صحیح x1 = *width;
	صحیح x, h, x_previous;
	صحیح fy = cy;
	ساختار box *چپ;
	ساختار box *راست;
	ساختار box *b;
	ساختار box *split_box = 0;
	ساختار box *d;
	ساختار box *br_box = 0;
	bool move_y = false;
	bool place_below = false;
	صحیح space_before = 0, space_after = 0;
	unsigned صحیح inline_شمار = 0;
	unsigned صحیح i;
	ثابت ساختار gui_layout_table *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      first,
	      (صحیح)first->length,
	      first->text,
	      *width,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	find_sides(cont->float_children, cy, cy, &x0, &x1, &چپ, &راست);
	x0 -= cx;
	x1 -= cx;

	اگر (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	اگر (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	اگر (has_text_children || first->parent->parent->gadget)
		used_height = height = سطر_height(&content->unit_len_ctx,
				first->parent->parent->style);
	وگرنه
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		used_height = height = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	برای (x = 0, b = first; x <= x1 - x0 && b != 0; b = b->next) {
		صحیح min_width, max_width, min_height, max_height;

		أكد(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		اگر (b->type == BOX_BR)
			بشکن;

		اگر (lh__box_is_float_box(b))
			ادامه;
		اگر (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			ادامه;

		أكد(b->style != پوچ);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		اگر (b->type == BOX_INLINE_BLOCK ||
		    b->type == BOX_INLINE_FLEX) {
			اگر (b->max_width != UNKNOWN_WIDTH)
				اگر (!layout_float(b, *width, content))
					بازگشت false;
			h = b->border[بالاترین_2].width + b->padding[بالاترین_2] + b->height +
					b->padding[BOTTOM] +
					b->border[BOTTOM].width;
			اگر (height < h)
				height = h;
			x += b->margin[چپ_2] + b->border[چپ_2].width +
					b->padding[چپ_2] + b->width +
					b->padding[راست_2] +
					b->border[راست_2].width +
					b->margin[راست_2];
			space_after = 0;
			ادامه;
		}

		اگر (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_find_dimensions(&content->unit_len_ctx,
					*width, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			برای (i = 0; i != 4; i++)
				اگر (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[چپ_2] + b->border[چپ_2].width +
					b->padding[چپ_2];
			اگر (b->inline_end) {
				b->inline_end->margin[راست_2] = b->margin[راست_2];
				b->inline_end->padding[راست_2] =
						b->padding[راست_2];
				b->inline_end->border[راست_2] =
						b->border[راست_2];
			} وگرنه {
				x += b->padding[راست_2] +
						b->border[راست_2].width +
						b->margin[راست_2];
			}
		} وگرنه اگر (b->type == BOX_INLINE_END) {
			b->width = 0;
			اگر (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[راست_2] + b->border[راست_2].width +
					b->margin[راست_2];
			ادامه;
		}

		اگر (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->height = سطر_height(&content->unit_len_ctx,
					b->style ? b->style :
					b->parent->parent->style);
			اگر (height < b->height)
				height = b->height;

			اگر (!b->text) {
				b->width = 0;
				space_after = 0;
				ادامه;
			}

			اگر (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				اگر (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					صحیح opt_maxwidth = 0;
					ساختار form_option *o;

					برای (o = b->parent->parent->gadget->
							data.select.اقلام; o;
							o = o->next) {
						صحیح opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						اگر (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}
					b->width = opt_maxwidth;
					اگر (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;
				} وگرنه {
					font_func->width(&fstyle, b->text,
							b->length, &b->width);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			اگر (b->text && (x + b->width < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->next) {
				font_func->width(&fstyle, b->text,
						 b->length, &b->width);
				b->flags |= MEASURED;
			}

			x += b->width;
			اگر (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			ادامه;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		layout_find_dimensions(&content->unit_len_ctx,
				*width, -1, b, b->style,
				&b->width, &b->height,
				&max_width, &min_width,
				&max_height, &min_height,
				پوچ, پوچ, پوچ);

		اگر (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->width, &b->height,
					min_width, max_width,
					min_height, max_height);
		} وگرنه اگر (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			اگر (b->width == AUTO)
				b->width = 400;
			اگر (b->height == AUTO)
				b->height = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} وگرنه {
			/* form control with no object */
			اگر (b->width == AUTO)
				b->width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			اگر (b->height == AUTO)
				b->height = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		اگر (b->object && content_can_reformat(b->object) &&
				b->width !=
				content_get_available_width(b->object)) {
			css_fixed مقدار = 0;
			css_unit unit = CSS_UNIT_PX;
			شمارش css_height_e htype = css_computed_height(b->style,
					&مقدار, &unit);

			content_reformat(b->object, false, b->width, b->height);

			اگر (htype == CSS_HEIGHT_AUTO)
				b->height = content_get_height(b->object);
		}

		اگر (height < b->height)
			height = b->height;

		x += b->width;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *width;
	find_sides(cont->float_children, cy, cy + height, &x0, &x1,
			&چپ, &راست);
	x0 -= cx;
	x1 -= cx;

	اگر (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	اگر (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	برای (x = x_previous = 0, b = first; x <= x1 - x0 && b; b = b->next) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		اگر (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} وگرنه اگر (lh__box_is_inline_flow(b)) {
			أكد(b->width != UNKNOWN_WIDTH);

			x_previous = x;
			x += space_after;
			b->x = x;

			اگر ((b->type == BOX_INLINE && !b->inline_end) ||
					b->type == BOX_INLINE_BLOCK ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[چپ_2] + b->border[چپ_2].width;
				x = b->x + b->padding[چپ_2] + b->width +
						b->padding[راست_2] +
						b->border[راست_2].width +
						b->margin[راست_2];
			} وگرنه اگر (b->type == BOX_INLINE) {
				b->x += b->margin[چپ_2] + b->border[چپ_2].width;
				x = b->x + b->padding[چپ_2] + b->width;
			} وگرنه اگر (b->type == BOX_INLINE_END) {
				b->height = b->inline_end->height;
				x += b->padding[راست_2] +
						b->border[راست_2].width +
						b->margin[راست_2];
			} وگرنه {
				x += b->width;
			}

			space_before = space_after;
			اگر (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			وگرنه اگر (b->text || b->type == BOX_INLINE_END) {
				اگر (b->space == UNKNOWN_WIDTH) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} وگرنه {
				space_after = 0;
			}
			split_box = b;
			move_y = true;
			inline_شمار++;
		} وگرنه اگر (b->type == BOX_BR) {
			b->x = x;
			b->width = 0;
			br_box = b;
			b = b->next;
			split_box = 0;
			move_y = true;
			بشکن;

		} وگرنه {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_place_below_level = 0;
			b->float_container = d->float_container = cont;

			اگر (!layout_float(d, *width, content))
				بازگشت false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[بالاترین_2],
			      d->border[بالاترین_2].width);

			d->x = d->margin[چپ_2] + d->border[چپ_2].width;
			d->y = d->margin[بالاترین_2] + d->border[بالاترین_2].width;
			b->width = d->margin[چپ_2] + d->border[چپ_2].width +
					d->padding[چپ_2] + d->width +
					d->padding[راست_2] +
					d->border[راست_2].width +
					d->margin[راست_2];
			b->height = d->margin[بالاترین_2] + d->border[بالاترین_2].width +
					d->padding[بالاترین_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];

			اگر (b->width > (x1 - x0) - x)
				place_below = true;
			اگر (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_چپ && چپ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_راست &&
						راست == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						چپ == 0 && راست == 0)) &&
					(!place_below ||
					(چپ == 0 && راست == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_place_below_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				اگر (b->type == BOX_FLOAT_چپ) {
					b->x = cx + x0;
					اگر (b->width > 0)
						x0 += b->width;
					چپ = b;
				} وگرنه {
					b->x = cx + x1 - b->width;
					اگر (b->width > 0)
						x1 -= b->width;
					راست = b;
				}
				b->y = cy;
			} وگرنه {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				صحیح fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_place_below_level) ?
						fcy :
						cont->cached_place_below_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + height : fy;

				place_float_below(b, *width, cx, fy, cont);
				fy = b->y;
				اگر (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_چپ && چپ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_راست &&
						راست != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(چپ != 0 || راست != 0)))) {
					/* to be cleared below existing
					 * floats */
					اگر (b->type == BOX_FLOAT_چپ)
						b->x = cx;
					وگرنه
						b->x = cx + *width - b->width;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					اگر (fcy > cont->clear_level)
						cont->clear_level = fcy;
					اگر (b->y < fcy)
						b->y = fcy;
				}
				اگر (b->type == BOX_FLOAT_چپ)
					چپ = b;
				وگرنه
					راست = b;
			}
			add_float_to_container(cont, b);

			split_box = 0;
		}
	}

	اگر (x1 - x0 < x && split_box) {
		/* the last box went over the end */
		size_t split = 0;
		صحیح w;
		bool no_wrap = css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_previous;

		اگر (!no_wrap &&
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

		اگر (split == 0)
			w = split_box->width;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      split_box,
		      (صحیح)split_box->length,
		      split_box->text,
		      split,
		      w,
		      چپ,
		      راست,
		      inline_شمار);

		اگر ((split == 0 || x1 - x0 <= x + space_before + w) &&
				!چپ && !راست && inline_شمار == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			اگر (split == 0 || split == split_box->length) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = split_box->next;
			} وگرنه {
				/* cut off first word for this line */
				اگر (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					بازگشت false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} وگرنه اگر ((split == 0 || x1 - x0 <= x + space_before + w) &&
				inline_شمار == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			أكد(چپ || راست);
			used_height = 0;
			اگر (چپ) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      چپ->y,
				      چپ->height);

				used_height = چپ->y + چپ->height - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      used_height);

			}
			اگر (راست && used_height <
					راست->y + راست->height - cy + 1)
				used_height = راست->y + راست->height - cy + 1;

			اگر (used_height < 0)
				used_height = 0;

			b = split_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} وگرنه اگر (split == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = split_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} وگرنه {
			/* fit as many words as possible */
			أكد(split != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (صحیح)split_box->length, split_box->text,
			      x1 - x0, split, w);

			اگر (split != split_box->length) {
				اگر (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					بازگشت false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	انتخاب (css_computed_text_align(first->parent->parent->style)) {
	حالت CSS_TEXT_ALIGN_راست:
	حالت CSS_TEXT_ALIGN_LIBCSS_راست:
		x0 = x1 - x;
		بشکن;
	حالت CSS_TEXT_ALIGN_CENTER:
	حالت CSS_TEXT_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		بشکن;
	حالت CSS_TEXT_ALIGN_چپ:
	حالت CSS_TEXT_ALIGN_LIBCSS_چپ:
	حالت CSS_TEXT_ALIGN_JUSTIFY:
		/* leave on left */
		بشکن;
	حالت CSS_TEXT_ALIGN_DEFAULT:
		/* None; consider text direction */
		انتخاب (css_computed_direction(first->parent->parent->style)) {
		حالت CSS_DIRECTION_LTR:
			/* leave on left */
			بشکن;
		حالت CSS_DIRECTION_RTL:
			x0 = x1 - x;
			بشکن;
		}
		بشکن;
	}

	برای (d = first; d != b; d = d->next) {
		d->flags &= ~NEW_سطر;

		اگر (d->type == BOX_INLINE_BLOCK &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			ادامه;
		} وگرنه اگر ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[بالاترین_2];

			اگر (d->type == BOX_TEXT && d->height > used_height) {
				/* text */
				used_height = d->height;
			}
		} وگرنه اگر ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_BLOCK) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[بالاترین_2].width + d->margin[بالاترین_2];
			h = d->margin[بالاترین_2] + d->border[بالاترین_2].width +
					d->padding[بالاترین_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];
			اگر (used_height < h)
				used_height = h;
		}
	}

	first->flags |= NEW_سطر;

	أكد(b != first || (move_y && 0 < used_height && (چپ || راست)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	برای (d = first; d != b; d = d->next) {
		اگر ((d->type == BOX_INLINE && d->inline_end) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			css_fixed مقدار = 0;
			css_unit unit = CSS_UNIT_PX;
			انتخاب (css_computed_vertical_align(d->style, &مقدار,
					&unit)) {
			حالت CSS_VERTICAL_ALIGN_SUPER:
			حالت CSS_VERTICAL_ALIGN_بالاترین:
			حالت CSS_VERTICAL_ALIGN_TEXT_بالاترین:
				/* already at top */
				بشکن;
			حالت CSS_VERTICAL_ALIGN_SUB:
			حالت CSS_VERTICAL_ALIGN_BOTTOM:
			حالت CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				d->y += used_height - d->height;
				بشکن;
			پیشفرض:
			حالت CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (used_height - d->height);
				بشکن;
			}
		}
	}

	/* handle clearance for br */
	اگر (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		صحیح clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		اگر (used_height < clear_y - cy)
			used_height = clear_y - cy;
	}

	اگر (move_y)
		*y += used_height;
	*next_box = b;
	*width = x; /* return actual width */
	بازگشت true;
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
ایستا bool layout_inline_container(ساختار box *inline_container, صحیح width,
		ساختار box *cont, صحیح cx, صحیح cy, html_content *content)
{
	bool first_سطر = true;
	bool has_text_children;
	ساختار box *c, *next;
	صحیح y = 0;
	صحیح curwidth,maxwidth = width;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      width,
	      cont,
	      cx,
	      cy);


	has_text_children = false;
	برای (c = inline_container->children; c; c = c->next) {
		bool is_pre = false;

		اگر (c->style) {
			شمارش css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_سطر ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		اگر ((lh__box_is_object(c) == false &&
				c->text && (c->length || is_pre)) ||
				c->type == BOX_BR)
			has_text_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	برای (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->width;
		اگر (!layout_سطر(c, &curwidth, &y, cx, cy + y, cont, first_سطر,
				has_text_children, content, &next))
			بازگشت false;
		maxwidth = max(maxwidth,curwidth);
		c = next;
		first_سطر = false;
	}

	inline_container->width = maxwidth;
	inline_container->height = y;

	بازگشت true;
}


/* Documented in layout_intertnal.h */
bool layout_block_context(
		ساختار box *block,
		صحیح viewport_height,
		html_content *content)
{
	ساختار box *box;
	صحیح cx, cy;  /**< current coordinates */
	صحیح max_pos_margin = 0;
	صحیح max_neg_margin = 0;
	صحیح y = 0;
	صحیح lm, rm;
	ساختار box *margin_collapse = پوچ;
	bool in_margin = false;
	css_fixed gadget_size;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	أكد(block->type == BOX_BLOCK ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX);
	أكد(block->width != UNKNOWN_WIDTH);
	أكد(block->width != AUTO);

	block->float_children = پوچ;
	block->cached_place_below_level = 0;
	block->clear_level = 0;

	/* special case if the block contains an object */
	اگر (block->object) {
		صحیح temp_width = block->width;
		اگر (!layout_block_object(block))
			بازگشت false;
		layout_get_object_dimensions(block, &temp_width,
				&block->height, INT_MIN, INT_MAX,
				INT_MIN, INT_MAX);
		بازگشت true;
	} وگرنه اگر (block->flags & REPLACE_DIM) {
		بازگشت true;
	}

	/* special case if the block contains an radio button or checkbox */
	اگر (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_size = INTTOFIX(1);
		اگر (block->height == AUTO)
			block->height = FIXTOINT(css_unit_len2device_px(
					block->style,
					&content->unit_len_ctx,
					gadget_size, gadget_unit));
	}

	box = block->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = block->padding[بالاترین_2];
	اگر (box)
		box->y = block->padding[بالاترین_2];

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
	وقتی (box) {
		شمارش css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
		شمارش css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

		أكد(box->type == BOX_BLOCK ||
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

		اگر (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->parent->padding[چپ_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			برو advance_to_next_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		اگر (margin_collapse == پوچ) {
			margin_collapse = layout_next_margin_block(
					&content->unit_len_ctx, box, block,
					viewport_height,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			in_margin = true;
		}

		/* Clearance. */
		y = 0;
		اگر (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(block->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		اگر (box->style) {
			overflow_x = css_computed_overflow_x(box->style);
			overflow_y = css_computed_overflow_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		اگر (box->type == BOX_FLEX ||
		    box->type == BOX_BLOCK ||
		    box->flags & IFRAME) {
			اگر (lh__box_is_object(box) == false &&
					box->style &&
					(overflow_x != CSS_OVERFLOW_VISIBLE ||
					 overflow_y != CSS_OVERFLOW_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				صحیح x0, x1, بالاترین;
				ساختار box *چپ, *راست;
				بالاترین = cy + max_pos_margin - max_neg_margin;
				بالاترین = (بالاترین > y) ? بالاترین : y;
				x0 = cx;
				x1 = cx + box->parent->width -
						box->parent->padding[چپ_2] -
						box->parent->padding[راست_2];
				find_sides(block->float_children, بالاترین, بالاترین,
						&x0, &x1, &چپ, &راست);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->parent->width -
						box->parent->padding[چپ_2] -
						box->parent->padding[راست_2] -
						x1;
			}
			layout_block_find_dimensions(&content->unit_len_ctx,
					box->parent->width,
					viewport_height, lm, rm, box);
			اگر (box->type == BOX_BLOCK && !(box->flags & IFRAME)) {
				layout_block_add_scrollbar(box, راست_2);
				layout_block_add_scrollbar(box, BOTTOM);
			}
		} وگرنه اگر (box->type == BOX_TABLE) {
			اگر (box->style != پوچ) {
				شمارش css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style, &width,
						&unit);

				اگر (wtype == CSS_WIDTH_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					صحیح x0, x1, بالاترین;
					ساختار box *چپ, *راست;
					بالاترین = cy + max_pos_margin -
							max_neg_margin;
					بالاترین = (بالاترین > y) ? بالاترین : y;
					x0 = cx;
					x1 = cx + box->parent->width -
						box->parent->padding[چپ_2] -
						box->parent->padding[راست_2];
					find_sides(block->float_children,
						بالاترین, بالاترین, &x0, &x1,
						&چپ, &راست);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->parent->width -
						box->parent->padding[چپ_2] -
						box->parent->padding[راست_2] -
						x1;
				}
			}
			اگر (!layout_table(box, box->parent->width - lm - rm,
					content))
				بازگشت false;
			layout_solve_width(box, box->parent->width, box->width,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->parent->padding[چپ_2] + box->margin[چپ_2] +
				box->border[چپ_2].width;
		cx += box->x;

		/* Position box: vertical. */
		اگر (box->border[بالاترین_2].width) {
			box->y += box->border[بالاترین_2].width;
			cy += box->border[بالاترین_2].width;
		}

		/* Vertical margin */
		اگر (((box->type == BOX_BLOCK && (box->flags & HAS_HEIGHT)) ||
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
		اگر (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		اگر (box->type == BOX_FLEX ||
				(box->type == BOX_BLOCK && box->style &&
				 (overflow_x != CSS_OVERFLOW_VISIBLE ||
				  overflow_y != CSS_OVERFLOW_VISIBLE))) {

			اگر (box->type == BOX_FLEX) {
				اگر (!layout_flex(box, box->width, content)) {
					بازگشت false;
				}
			} وگرنه {
				layout_block_context(box,
						viewport_height, content);
			}

			cy += box->padding[بالاترین_2];

			اگر (box->height == AUTO) {
				box->height = 0;
				layout_block_add_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->height + box->padding[BOTTOM] +
					box->border[BOTTOM].width;
			y = box->y + box->padding[بالاترین_2] + box->height +
					box->padding[BOTTOM] +
					box->border[BOTTOM].width;

			/* Skip children, because they are done in the new
			 * block context */
			برو advance_to_next_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->width);

		/* Layout (except tables). */
		اگر (box->object) {
			اگر (!layout_block_object(box))
				بازگشت false;

		} وگرنه اگر (box->type == BOX_INLINE_CONTAINER) {
			box->width = box->parent->width;
			اگر (!layout_inline_container(box, box->width, block,
					cx, cy, content))
				بازگشت false;

		} وگرنه اگر (box->type == BOX_TABLE) {
			/* Move down to avoid floats if necessary. */
			صحیح x0, x1;
			ساختار box *چپ, *راست;
			y = cy;
			وقتی (1) {
				شمارش css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style,
						&width, &unit);

				x0 = cx;
				x1 = cx + box->parent->width;
				find_sides(block->float_children, y,
						y + box->height,
						&x0, &x1, &چپ, &راست);
				اگر (wtype == CSS_WIDTH_AUTO)
					بشکن;
				اگر (box->width <= x1 - x0)
					بشکن;
				اگر (!چپ && !راست)
					بشکن;
				وگرنه اگر (!چپ)
					y = راست->y + راست->height + 1;
				وگرنه اگر (!راست)
					y = چپ->y + چپ->height + 1;
				وگرنه اگر (چپ->y + چپ->height <
						راست->y + راست->height)
					y = چپ->y + چپ->height + 1;
				وگرنه
					y = راست->y + راست->height + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		اگر (box->type == BOX_BLOCK && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			اگر (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = پوچ;
			}

			y = box->padding[بالاترین_2];
			box = box->children;
			box->y = y;
			cy += y;
			ادامه;
		} وگرنه اگر (box->type == BOX_BLOCK || box->object ||
				box->flags & IFRAME)
			cy += box->padding[بالاترین_2];

		اگر (box->type == BOX_BLOCK && box->height == AUTO) {
			box->height = 0;
			layout_block_add_scrollbar(box, BOTTOM);
		}

		cy += box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width;
		cx -= box->x;
		y = box->y + box->padding[بالاترین_2] + box->height +
				box->padding[BOTTOM] +
				box->border[BOTTOM].width;

	advance_to_next_box:
		اگر (!box->next) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			انجامبده {
				اگر (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = پوچ;
				}

				/* Apply bottom margin */
				اگر (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				وگرنه اگر (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->parent;
				اگر (box == block)
					بشکن;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				اگر (box->type == BOX_BLOCK &&
						box->flags & MAKE_HEIGHT) {
					margin_collapse = پوچ;
					in_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				اگر (box->height == AUTO) {
					box->height = y - box->padding[بالاترین_2];

					اگر (box->type == BOX_BLOCK)
						layout_block_add_scrollbar(box,
								BOTTOM);
				} وگرنه
					cy += box->height -
							(y - box->padding[بالاترین_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				اگر (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_height(
							&content->unit_len_ctx,
							box, پوچ)) {
					/* Height altered */
					/* Set current cy */
					cy += box->height -
							(y - box->padding[بالاترین_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].width;
				cx -= box->x;
				y = box->y + box->padding[بالاترین_2] + box->height +
						box->padding[BOTTOM] +
						box->border[BOTTOM].width;

			} وقتی (box->next == پوچ);
			اگر (box == block)
				بشکن;
		}

		/* To next sibling. */

		اگر (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = پوچ;
		}

		اگر (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		وگرنه اگر (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->next;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	برای (box = block->float_children; box; box = box->next_float) {
		y = box->y + box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width + box->margin[BOTTOM];
		اگر (cy < y)
			cy = y;
	}

	اگر (block->height == AUTO) {
		block->height = cy - block->padding[بالاترین_2];
		اگر (block->type == BOX_BLOCK)
			layout_block_add_scrollbar(block, BOTTOM);
	}

	اگر (block->style && css_computed_position(block->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_height(&content->unit_len_ctx, block, پوچ);
	}

	اگر (block->gadget &&
			(block->gadget->type == GADGET_TEXTAREA ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		صحیح ta_width = block->padding[چپ_2] + block->width +
				block->padding[راست_2];
		صحیح ta_height = block->padding[بالاترین_2] + block->height +
				block->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				block->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_set_layout(block->gadget->data.text.ta,
				&fstyle, ta_width, ta_height,
				block->padding[بالاترین_2], block->padding[راست_2],
				block->padding[BOTTOM], block->padding[چپ_2]);
	}

	بازگشت true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
ایستا bool
layout__get_element_tag(
		ثابت dom_گره *گره,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_گره_type گره_type;
	dom_exception exc;

	exc = dom_گره_get_گره_type(گره, &گره_type);
	اگر (exc != DOM_NO_ERR ||
	    گره_type != DOM_ELEMENT_گره) {
		بازگشت false;
	}

	exc = dom_html_element_get_tag_type(گره, &element_type);
	اگر (exc != DOM_NO_ERR) {
		بازگشت false;
	}

	*type = element_type;
	بازگشت true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
ایستا درونخطی bool
layout__check_element_type(
		ثابت dom_گره *گره,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	اگر (!layout__get_element_tag(گره, &element_type)) {
		بازگشت false;
	}

	بازگشت element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ایستا bool
layout_get_li_مقدار(dom_گره *li_گره, dom_long *مقدار_out)
{
	dom_exception exc;
	dom_long مقدار;
	bool has_مقدار;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_گره,
			corestring_dom_مقدار,
			&has_مقدار);
	اگر (exc != DOM_NO_ERR || has_مقدار == false) {
		بازگشت false;
	}

	exc = dom_html_li_element_get_مقدار(
			(dom_html_li_element *)li_گره,
			&مقدار);
	اگر (exc != DOM_NO_ERR) {
		بازگشت false;
	}

	*مقدار_out = مقدار;
	بازگشت true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ایستا bool
layout__get_ol_start(dom_گره *ol_گره, dom_long *start_out)
{
	dom_exception exc;
	dom_long start;
	bool has_start;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_گره,
			corestring_dom_start,
			&has_start);
	اگر (exc != DOM_NO_ERR || has_start == false) {
		بازگشت false;
	}

	exc = dom_html_olist_element_get_start(
			(dom_html_olist_element *)ol_گره,
			&start);
	اگر (exc != DOM_NO_ERR) {
		بازگشت false;
	}

	*start_out = start;
	بازگشت true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
ایستا bool
layout__get_ol_reversed(dom_گره *ol_گره)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_گره,
			corestring_dom_reversed,
			&has_reversed);
	اگر (exc != DOM_NO_ERR) {
		بازگشت false;
	}

	بازگشت has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
ایستا bool
layout_get_list_item_شمار(
		dom_گره *list_owner, dom_long *شمار_out)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_گره *child;
	صحیح شمار;

	اگر (list_owner == پوچ) {
		بازگشت false;
	}

	اگر (!layout__get_element_tag(list_owner, &tag_type)) {
		بازگشت false;
	}

	اگر (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		بازگشت false;
	}

	exc = dom_گره_get_first_child(list_owner, &child);
	اگر (exc != DOM_NO_ERR) {
		بازگشت false;
	}

	شمار = 0;
	وقتی (child != پوچ) {
		dom_گره *temp_گره;

		اگر (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ساختار box *child_box;
			اگر (dom_گره_get_user_data(child,
					corestring_dom_ns_کلید_box_گره_data,
					&child_box) != DOM_NO_ERR) {
				dom_گره_unref(child);
				بازگشت false;
			}

			اگر (child_box != پوچ &&
			    child_box->list_marker != پوچ) {
				شمار++;
			}
		}

		exc = dom_گره_get_next_sibling(child, &temp_گره);
		dom_گره_unref(child);
		اگر (exc != DOM_NO_ERR) {
			بازگشت false;
		}

		child = temp_گره;
	}

	*شمار_out = شمار;
	بازگشت true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
ایستا تهی
layout_ordered_list_شمار(
		ساختار box *box)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_گره *child;
	صحیح step = 1;
	dom_long next;

	اگر (box->گره == پوچ) {
		بازگشت;
	}

	اگر (!layout__get_element_tag(box->گره, &tag_type)) {
		بازگشت;
	}

	اگر (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		بازگشت;
	}

	next = 1;
	اگر (tag_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_start = layout__get_ol_start(box->گره, &next);
		bool have_reversed = layout__get_ol_reversed(box->گره);

		اگر (have_reversed) {
			step = -1;
		}

		اگر (!have_start && have_reversed) {
			layout_get_list_item_شمار(box->گره, &next);
		}
	}

	exc = dom_گره_get_first_child(box->گره, &child);
	اگر (exc != DOM_NO_ERR) {
		بازگشت;
	}

	وقتی (child != پوچ) {
		dom_گره *temp_گره;

		اگر (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ساختار box *child_box;

			اگر (dom_گره_get_user_data(child,
					corestring_dom_ns_کلید_box_گره_data,
					&child_box) != DOM_NO_ERR) {
				dom_گره_unref(child);
				بازگشت;
			}

			اگر (child_box != پوچ &&
			    child_box->list_marker != پوچ) {
				dom_long مقدار;
				ساختار box *marker = child_box->list_marker;
				اگر (layout_get_li_مقدار(child, &مقدار)) {
					marker->list_مقدار = مقدار;
					next = marker->list_مقدار;
				} وگرنه {
					marker->list_مقدار = next;
				}
				next += step;
			}
		}

		exc = dom_گره_get_next_sibling(child, &temp_گره);
		dom_گره_unref(child);
		اگر (exc != DOM_NO_ERR) {
			بازگشت;
		}

		child = temp_گره;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
ایستا تهی
layout__set_numerical_marker_text(
		ثابت html_content *content,
		ساختار box *box)
{
	ساختار box *marker = box->list_marker;
	size_t counter_len;
	css_error css_res;
	شمارش {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_SIZE = 20,
	};

	marker->text = talloc_array(content->bctx, نویسه, LIST_MARKER_SIZE);
	اگر (marker->text == پوچ) {
		بازگشت;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_مقدار,
			marker->text, LIST_MARKER_SIZE, &counter_len);
	اگر (css_res == CSS_OK) {
		اگر (counter_len > LIST_MARKER_SIZE) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->text = talloc_realloc(content->bctx,
					marker->text,
					نویسه,
					counter_len);
			اگر (marker->text == پوچ) {
				بازگشت;
			}
			css_computed_format_list_style(box->style,
					marker->list_مقدار, marker->text,
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
ایستا bool
layout__list_item_is_numerical(
		ثابت ساختار box *b)
{
	شمارش css_list_style_type_e t = css_computed_list_style_type(b->style);

	انتخاب (t) {
	حالت CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	حالت CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	حالت CSS_LIST_STYLE_TYPE_SQUARE: /* Fall through. */
	حالت CSS_LIST_STYLE_TYPE_NONE:
		بازگشت false;

	پیشفرض:
		بازگشت true;
	}
}

/**
 * Layout list markers.
 */
ایستا تهی
layout_lists(ثابت html_content *content, ساختار box *box)
{
	ساختار box *child;

	layout_ordered_list_شمار(box);

	برای (child = box->children; child; child = child->next) {
		اگر (child->list_marker) {
			ساختار box *marker = child->list_marker;

			اگر (layout__list_item_is_numerical(child)) {
				اگر (marker->text == پوچ) {
					layout__set_numerical_marker_text(
							content, child);
				}
			}
			اگر (marker->object) {
				marker->width =
					content_get_width(marker->object);
				marker->x = -marker->width;
				marker->height =
					content_get_height(marker->object);
				marker->y = (سطر_height(
						&content->unit_len_ctx,
						marker->style) -
						marker->height) / 2;
			} وگرنه اگر (marker->text) {
				اگر (marker->width == UNKNOWN_WIDTH) {
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
				marker->height = سطر_height(
						&content->unit_len_ctx,
						marker->style);
			} وگرنه {
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
ایستا تهی
layout_compute_offsets(ثابت css_unit_ctx *unit_len_ctx,
		       ساختار box *box,
		       ساختار box *containing_block,
		       صحیح *بالاترین,
		       صحیح *راست,
		       صحیح *bottom,
		       صحیح *چپ)
{
	uint32_t type;
	css_fixed مقدار = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(containing_block->width != UNKNOWN_WIDTH);
	أكد(containing_block->width != AUTO);
	أكد(containing_block->height != AUTO);

	/*چپ*/
	type = css_computed_چپ(box->style, &مقدار, &unit);
	اگر (type == CSS_چپ_SET) {
		اگر (unit == CSS_UNIT_PCT) {
			*چپ = FPCT_OF_INT_TOINT(مقدار,
					containing_block->width);
		} وگرنه {
			*چپ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					مقدار, unit));
		}
	} وگرنه {
		*چپ = AUTO;
	}

	/*راست*/
	type = css_computed_راست(box->style, &مقدار, &unit);
	اگر (type == CSS_راست_SET) {
		اگر (unit == CSS_UNIT_PCT) {
			*راست = FPCT_OF_INT_TOINT(مقدار,
					containing_block->width);
		} وگرنه {
			*راست = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					مقدار, unit));
		}
	} وگرنه {
		*راست = AUTO;
	}

	/*بالاترین*/
	type = css_computed_بالاترین(box->style, &مقدار, &unit);
	اگر (type == CSS_بالاترین_SET) {
		اگر (unit == CSS_UNIT_PCT) {
			*بالاترین = FPCT_OF_INT_TOINT(مقدار,
					containing_block->height);
		} وگرنه {
			*بالاترین = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					مقدار, unit));
		}
	} وگرنه {
		*بالاترین = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &مقدار, &unit);
	اگر (type == CSS_BOTTOM_SET) {
		اگر (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_OF_INT_TOINT(مقدار,
					containing_block->height);
		} وگرنه {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					مقدار, unit));
		}
	} وگرنه {
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
ایستا bool
layout_absolute(ساختار box *box,
		ساختار box *containing_block,
		صحیح cx, صحیح cy,
		html_content *content)
{
	صحیح static_چپ, static_بالاترین;  /* static position */
	صحیح بالاترین, راست, bottom, چپ;
	صحیح width, height, max_width, min_width;
	صحیح *margin = box->margin;
	صحیح *padding = box->padding;
	ساختار box_border *border = box->border;
	صحیح available_width = containing_block->width;
	صحیح space;

	أكد(box->type == BOX_BLOCK || box->type == BOX_TABLE ||
			box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_چپ = cx + box->x;
	static_بالاترین = cy + box->y;

	اگر (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_block->width += containing_block->padding[چپ_2] +
				containing_block->padding[راست_2];
		containing_block->height += containing_block->padding[بالاترین_2] +
				containing_block->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_block,
			&بالاترین, &راست, &bottom, &چپ);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_block;
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1,
			box, box->style, &width, &height,
			&max_width, &min_width, 0, 0,
			margin, padding, border);
	box->float_container = پوچ;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      چپ, margin[چپ_2], border[چپ_2].width, padding[چپ_2], width,
	      padding[راست_2], border[راست_2].width, margin[راست_2], راست,
	      containing_block->width);


	اگر (چپ == AUTO && width == AUTO && راست == AUTO) {
		اگر (margin[چپ_2] == AUTO)
			margin[چپ_2] = 0;
		اگر (margin[راست_2] == AUTO)
			margin[راست_2] = 0;
		چپ = static_چپ;

		width = min(max(box->min_width, available_width),
			box->max_width);
		width -= box->margin[چپ_2] + box->border[چپ_2].width +
			box->padding[چپ_2] + box->padding[راست_2] +
			box->border[راست_2].width + box->margin[راست_2];

		/* Adjust for {min|max}-width */
		اگر (max_width >= 0 && width > max_width) width = max_width;
		اگر (width < min_width) width = min_width;

		راست = containing_block->width -
			چپ -
			margin[چپ_2] - border[چپ_2].width - padding[چپ_2] -
			width -
			padding[راست_2] - border[راست_2].width - margin[راست_2];
	} وگرنه اگر (چپ != AUTO && width != AUTO && راست != AUTO) {

		/* Adjust for {min|max}-width */
		اگر (max_width >= 0 && width > max_width) width = max_width;
		اگر (min_width >  0 && width < min_width) width = min_width;

		اگر (margin[چپ_2] == AUTO && margin[راست_2] == AUTO) {
			space = containing_block->width -
					چپ - border[چپ_2].width -
					padding[چپ_2] - width - padding[راست_2] -
					border[راست_2].width - راست;
			اگر (space < 0) {
				margin[چپ_2] = 0;
				margin[راست_2] = space;
			} وگرنه {
				margin[چپ_2] = margin[راست_2] = space / 2;
			}
		} وگرنه اگر (margin[چپ_2] == AUTO) {
			margin[چپ_2] = containing_block->width -
					چپ - border[چپ_2].width -
					padding[چپ_2] - width - padding[راست_2] -
					border[راست_2].width - margin[راست_2] -
					راست;
		} وگرنه اگر (margin[راست_2] == AUTO) {
			margin[راست_2] = containing_block->width -
					چپ - margin[چپ_2] -
					border[چپ_2].width -
					padding[چپ_2] - width - padding[راست_2] -
					border[راست_2].width - راست;
		} وگرنه {
			راست = containing_block->width -
					چپ - margin[چپ_2] -
					border[چپ_2].width -
					padding[چپ_2] - width - padding[راست_2] -
					border[راست_2].width - margin[راست_2];
		}
	} وگرنه {
		اگر (margin[چپ_2] == AUTO)
			margin[چپ_2] = 0;
		اگر (margin[راست_2] == AUTO)
			margin[راست_2] = 0;

		اگر (چپ == AUTO && width == AUTO && راست != AUTO) {
			available_width -= راست;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[چپ_2] + box->border[چپ_2].width +
				box->padding[چپ_2] + box->padding[راست_2] +
				box->border[راست_2].width + box->margin[راست_2];

			/* Adjust for {min|max}-width */
			اگر (max_width >= 0 && width > max_width)
				width = max_width;
			اگر (width < min_width)
				width = min_width;

			چپ = containing_block->width -
					margin[چپ_2] - border[چپ_2].width -
					padding[چپ_2] - width - padding[راست_2] -
					border[راست_2].width - margin[راست_2] -
					راست;
		} وگرنه اگر (چپ == AUTO && width != AUTO && راست == AUTO) {

			/* Adjust for {min|max}-width */
			اگر (max_width >= 0 && width > max_width)
				width = max_width;
			اگر (min_width >  0 && width < min_width)
				width = min_width;

			چپ = static_چپ;
			راست = containing_block->width -
					چپ - margin[چپ_2] -
					border[چپ_2].width -
					padding[چپ_2] - width - padding[راست_2] -
					border[راست_2].width - margin[راست_2];
		} وگرنه اگر (چپ != AUTO && width == AUTO && راست == AUTO) {
			available_width -= چپ;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[چپ_2] + box->border[چپ_2].width +
				box->padding[چپ_2] + box->padding[راست_2] +
				box->border[راست_2].width + box->margin[راست_2];

			/* Adjust for {min|max}-width */
			اگر (max_width >= 0 && width > max_width)
				width = max_width;
			اگر (width < min_width)
				width = min_width;

			راست = containing_block->width -
					چپ - margin[چپ_2] -
					border[چپ_2].width -
					padding[چپ_2] - width - padding[راست_2] -
					border[راست_2].width - margin[راست_2];
		} وگرنه اگر (چپ == AUTO && width != AUTO && راست != AUTO) {

			/* Adjust for {min|max}-width */
			اگر (max_width >= 0 && width > max_width)
				width = max_width;
			اگر (width < min_width)
				width = min_width;

			چپ = containing_block->width -
					margin[چپ_2] - border[چپ_2].width -
					padding[چپ_2] - width - padding[راست_2] -
					border[راست_2].width - margin[راست_2] -
					راست;
		} وگرنه اگر (چپ != AUTO && width == AUTO && راست != AUTO) {
			width = containing_block->width -
					چپ - margin[چپ_2] -
					border[چپ_2].width -
					padding[چپ_2] - padding[راست_2] -
					border[راست_2].width - margin[راست_2] -
					راست;

			/* Adjust for {min|max}-width */
			اگر (max_width >= 0 && width > max_width)
				width = max_width;
			اگر (width < min_width)
				width = min_width;

		} وگرنه اگر (چپ != AUTO && width != AUTO && راست == AUTO) {

			/* Adjust for {min|max}-width */
			اگر (max_width >= 0 && width > max_width)
				width = max_width;
			اگر (width < min_width)
				width = min_width;

			راست = containing_block->width -
					چپ - margin[چپ_2] -
					border[چپ_2].width -
					padding[چپ_2] - width - padding[راست_2] -
					border[راست_2].width - margin[راست_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      چپ, margin[چپ_2], border[چپ_2].width, padding[چپ_2], width,
	      padding[راست_2], border[راست_2].width, margin[راست_2], راست,
	      containing_block->width);

	box->x = چپ + margin[چپ_2] + border[چپ_2].width - cx;
	اگر (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_block->width -= containing_block->padding[چپ_2] +
				containing_block->padding[راست_2];
	} وگرنه {
		/** \todo inline ancestors */
	}
	box->width = width;
	box->height = height;

	اگر (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->object || box->flags & IFRAME) {
		اگر (!layout_block_context(box, -1, content))
			بازگشت false;
	} وگرنه اگر (box->type == BOX_TABLE) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		/* \todo  layout_table considers margins etc. again */
		اگر (!layout_table(box, width, content))
			بازگشت false;
		box->float_container = پوچ;
		layout_solve_width(box, box->parent->width, box->width, 0, 0,
				-1, -1);
	} وگرنه اگر (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		اگر (!layout_flex(box, width, content))
			بازگشت false;
		box->float_container = پوچ;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      بالاترین, margin[بالاترین_2], border[بالاترین_2].width, padding[بالاترین_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	اگر (بالاترین == AUTO && height == AUTO && bottom == AUTO) {
		بالاترین = static_بالاترین;
		height = box->height;
		اگر (margin[بالاترین_2] == AUTO)
			margin[بالاترین_2] = 0;
		اگر (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_block->height -
				بالاترین - margin[بالاترین_2] - border[بالاترین_2].width -
				padding[بالاترین_2] - height - padding[BOTTOM] -
				border[BOTTOM].width - margin[BOTTOM];
	} وگرنه اگر (بالاترین != AUTO && height != AUTO && bottom != AUTO) {
		اگر (margin[بالاترین_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_block->height -
					بالاترین - border[بالاترین_2].width - padding[بالاترین_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - bottom;
			margin[بالاترین_2] = margin[BOTTOM] = space / 2;
		} وگرنه اگر (margin[بالاترین_2] == AUTO) {
			margin[بالاترین_2] = containing_block->height -
					بالاترین - border[بالاترین_2].width - padding[بالاترین_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} وگرنه اگر (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_block->height -
					بالاترین - margin[بالاترین_2] - border[بالاترین_2].width -
					padding[بالاترین_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					bottom;
		} وگرنه {
			bottom = containing_block->height -
					بالاترین - margin[بالاترین_2] - border[بالاترین_2].width -
					padding[بالاترین_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	} وگرنه {
		اگر (margin[بالاترین_2] == AUTO)
			margin[بالاترین_2] = 0;
		اگر (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		اگر (بالاترین == AUTO && height == AUTO && bottom != AUTO) {
			height = box->height;
			بالاترین = containing_block->height -
					margin[بالاترین_2] - border[بالاترین_2].width -
					padding[بالاترین_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} وگرنه اگر (بالاترین == AUTO && height != AUTO && bottom == AUTO) {
			بالاترین = static_بالاترین;
			bottom = containing_block->height -
					بالاترین - margin[بالاترین_2] - border[بالاترین_2].width -
					padding[بالاترین_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} وگرنه اگر (بالاترین != AUTO && height == AUTO && bottom == AUTO) {
			height = box->height;
			bottom = containing_block->height -
					بالاترین - margin[بالاترین_2] - border[بالاترین_2].width -
					padding[بالاترین_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} وگرنه اگر (بالاترین == AUTO && height != AUTO && bottom != AUTO) {
			بالاترین = containing_block->height -
					margin[بالاترین_2] - border[بالاترین_2].width -
					padding[بالاترین_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} وگرنه اگر (بالاترین != AUTO && height == AUTO && bottom != AUTO) {
			height = containing_block->height -
					بالاترین - margin[بالاترین_2] - border[بالاترین_2].width -
					padding[بالاترین_2] - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} وگرنه اگر (بالاترین != AUTO && height != AUTO && bottom == AUTO) {
			bottom = containing_block->height -
					بالاترین - margin[بالاترین_2] - border[بالاترین_2].width -
					padding[بالاترین_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      بالاترین, margin[بالاترین_2], border[بالاترین_2].width, padding[بالاترین_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	box->y = بالاترین + margin[بالاترین_2] + border[بالاترین_2].width - cy;
	اگر (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_block->height -= containing_block->padding[بالاترین_2] +
				containing_block->padding[BOTTOM];
	} وگرنه {
		/** \todo Inline ancestors */
	}
	box->height = height;
	layout_apply_minmax_height(&content->unit_len_ctx, box, containing_block);

	بازگشت true;
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
ایستا bool
layout_position_absolute(ساختار box *box,
			 ساختار box *containing_block,
			 صحیح cx, صحیح cy,
			 html_content *content)
{
	ساختار box *c;

	برای (c = box->children; c; c = c->next) {
		اگر ((c->type == BOX_BLOCK || c->type == BOX_TABLE ||
				c->type == BOX_INLINE_BLOCK ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			اگر (!layout_absolute(c, containing_block,
					cx, cy, content))
				بازگشت false;
			اگر (!layout_position_absolute(c, c, 0, 0, content))
				بازگشت false;
		} وگرنه اگر (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			اگر (!layout_position_absolute(c, c, 0, 0, content))
				بازگشت false;
		} وگرنه {
			صحیح px, py;
			اگر (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_چپ ||
					css_computed_float(c->style) ==
						CSS_FLOAT_راست)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				ساختار box *p;
				px = c->x;
				py = c->y;
				برای (p = box->parent; p && !p->float_children;
						p = p->parent) {
					px -= p->x;
					py -= p->y;
				}
			} وگرنه {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			اگر (!layout_position_absolute(c, containing_block,
					cx + px, cy + py, content))
				بازگشت false;
		}
	}

	بازگشت true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
ایستا تهی layout_compute_relative_offset(
		ثابت css_unit_ctx *unit_len_ctx,
		ساختار box *box,
		صحیح *x,
		صحیح *y)
{
	صحیح چپ, راست, بالاترین, bottom;
	ساختار box *containing_block;

	أكد(box && box->parent && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	اگر (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_چپ ||
			css_computed_float(box->style) == CSS_FLOAT_راست)) {
		containing_block = box->float_container;
	} وگرنه {
		containing_block = box->parent;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_block,
			&بالاترین, &راست, &bottom, &چپ);

	اگر (چپ == AUTO && راست == AUTO)
		چپ = راست = 0;
	وگرنه اگر (چپ == AUTO)
		/* left is auto => computed = -right */
		چپ = -راست;
	وگرنه اگر (راست == AUTO)
		/* right is auto => computed = -left */
		راست = -چپ;
	وگرنه {
		/* over constrained => examine direction property
		 * of containing block */
		اگر (containing_block->style &&
				css_computed_direction(
				containing_block->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			چپ = -راست;
		} وگرنه {
			/* assume LTR in all other cases */
			راست = -چپ;
		}
	}

	أكد(چپ == -راست);

	اگر (بالاترین == AUTO && bottom == AUTO) {
		بالاترین = bottom = 0;
	} وگرنه اگر (بالاترین == AUTO) {
		بالاترین = -bottom;
	} وگرنه {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -بالاترین;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", چپ,
	      راست, بالاترین, bottom);

	*x = چپ;
	*y = بالاترین;
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
ایستا تهی
layout_position_relative(
		ثابت css_unit_ctx *unit_len_ctx,
		ساختار box *ریشه,
		ساختار box *fp,
		صحیح fx,
		صحیح fy)
{
	ساختار box *box; /* for children of "root" */
	ساختار box *fn;  /* for block formatting context box for children of
			  * "box" */
	ساختار box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	صحیح x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	صحیح fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	أكد(ریشه);

	/* Normal children */
	برای (box = ریشه->children; box; box = box->next) {

		اگر (box->type == BOX_TEXT)
			ادامه;

		/* If relatively positioned, get offsets */
		اگر (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_offset(
					unit_len_ctx, box, &x, &y);
		وگرنه
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		اگر (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_چپ ||
				css_computed_float(box->style) ==
					CSS_FLOAT_راست) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			برای (fc = fp->float_children; fc; fc = fc->next_float) {
				اگر (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		اگر (box->float_children) {
			fn = box;
			fnx = fny = 0;
		} وگرنه {
			fn = fp;
			fnx = fx + x;
			fny = fy + y;
		}

		/* recurse first */
		layout_position_relative(unit_len_ctx, box, fn, fnx, fny);

		/* Ignore things we're not interested in. */
		اگر (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			ادامه;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		اگر (box->type == BOX_INLINE && box->inline_end) {
			ساختار box *b;
			برای (b = box->next; b && b != box->inline_end;
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
ایستا تهی
layout_get_box_bbox(
		ثابت css_unit_ctx *unit_len_ctx,
		ساختار box *box,
		صحیح *desc_x0, صحیح *desc_y0,
		صحیح *desc_x1, صحیح *desc_y1)
{
	*desc_x0 = -box->border[چپ_2].width;
	*desc_y0 = -box->border[بالاترین_2].width;
	*desc_x1 = box->padding[چپ_2] + box->width + box->padding[راست_2] +
			box->border[راست_2].width;
	*desc_y1 = box->padding[بالاترین_2] + box->height + box->padding[BOTTOM] +
			box->border[BOTTOM].width;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	اگر (box->type == BOX_BLOCK && box->style != پوچ &&
			css_computed_overflow_y(box->style) ==
					CSS_OVERFLOW_VISIBLE &&
			box->object == پوچ) {
		css_fixed font_size = 0;
		css_unit font_unit = CSS_UNIT_PT;
		صحیح text_height;

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
ایستا تهی
layout_update_descendant_bbox(
		ثابت css_unit_ctx *unit_len_ctx,
		ساختار box *box,
		ساختار box *child,
		صحیح off_x,
		صحیح off_y)
{
	صحیح child_desc_x0, child_desc_y0, child_desc_x1, child_desc_y1;

	/* get coordinates of child relative to box */
	صحیح child_x = child->x - off_x;
	صحیح child_y = child->y - off_y;

	bool html_object = (child->object &&
			content_get_type(child->object) == CONTENT_HTML);

	شمارش css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	شمارش css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

	اگر (child->style != پوچ) {
		overflow_x = css_computed_overflow_x(child->style);
		overflow_y = css_computed_overflow_y(child->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, child,
			&child_desc_x0, &child_desc_y0,
			&child_desc_x1, &child_desc_y1);

	اگر (overflow_x == CSS_OVERFLOW_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		child_desc_x0 = child->descendant_x0;
		child_desc_x1 = child->descendant_x1;
	}
	اگر (overflow_y == CSS_OVERFLOW_VISIBLE &&
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
	اگر (child_desc_x0 < box->descendant_x0)
		box->descendant_x0 = child_desc_x0;
	اگر (child_desc_y0 < box->descendant_y0)
		box->descendant_y0 = child_desc_y0;
	اگر (box->descendant_x1 < child_desc_x1)
		box->descendant_x1 = child_desc_x1;
	اگر (box->descendant_y1 < child_desc_y1)
		box->descendant_y1 = child_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
ایستا تهی layout_calculate_descendant_bboxes(
		ثابت css_unit_ctx *unit_len_ctx,
		ساختار box *box)
{
	ساختار box *child;

	أكد(box->width != UNKNOWN_WIDTH);
	أكد(box->height != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	اگر (box->object && content_get_type(box->object) == CONTENT_HTML) {
		اگر (box->descendant_x1 < content_get_width(box->object))
			box->descendant_x1 = content_get_width(box->object);
		اگر (box->descendant_y1 < content_get_height(box->object))
			box->descendant_y1 = content_get_height(box->object);
	}

	اگر (box->iframe != پوچ) {
		صحیح x, y;
		box_coords(box, &x, &y);

		browser_window_set_position(box->iframe, x, y);
		browser_window_set_dimensions(box->iframe,
				box->width, box->height);
		browser_window_reformat(box->iframe, true,
				box->width, box->height);
	}

	اگر (box->type == BOX_INLINE || box->type == BOX_TEXT)
		بازگشت;

	اگر (box->type == BOX_INLINE_END) {
		box = box->inline_end;
		برای (child = box->next; child;
				child = child->next) {
			اگر (child->type == BOX_FLOAT_چپ ||
					child->type == BOX_FLOAT_راست)
				ادامه;

			layout_update_descendant_bbox(unit_len_ctx, box, child,
					box->x, box->y);

			اگر (child == box->inline_end)
				بشکن;
		}
		بازگشت;
	}

	اگر (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		بازگشت;

	برای (child = box->children; child; child = child->next) {
		اگر (child->type == BOX_FLOAT_چپ ||
				child->type == BOX_FLOAT_راست)
			ادامه;

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		اگر (box->style && css_computed_overflow_x(box->style) ==
				CSS_OVERFLOW_HIDDEN &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_HIDDEN)
			ادامه;

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	برای (child = box->float_children; child; child = child->next_float) {
		أكد(child->type == BOX_FLOAT_چپ ||
				child->type == BOX_FLOAT_راست);

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	اگر (box->list_marker) {
		child = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, صحیح width, صحیح height)
{
	bool ret;
	ساختار box *doc = content->layout;
	ثابت ساختار gui_layout_table *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			width, height, nsurl_access(content_get_url(
					&content->base)));

	layout_minmax_block(doc, font_func, content);

	layout_block_find_dimensions(&content->unit_len_ctx,
			width, height, 0, 0, doc);
	doc->x = doc->margin[چپ_2] + doc->border[چپ_2].width;
	doc->y = doc->margin[بالاترین_2] + doc->border[بالاترین_2].width;
	width -= doc->margin[چپ_2] + doc->border[چپ_2].width +
			doc->padding[چپ_2] + doc->padding[راست_2] +
			doc->border[راست_2].width + doc->margin[راست_2];
	اگر (width < 0) {
		width = 0;
	}
	doc->width = width;

	ret = layout_block_context(doc, height, content);

	/* make <html> and <body> fill available height */
	اگر (doc->y + doc->padding[بالاترین_2] + doc->height + doc->padding[BOTTOM] +
			doc->border[BOTTOM].width + doc->margin[BOTTOM] <
			height) {
		doc->height = height - (doc->y + doc->padding[بالاترین_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].width +
				doc->margin[BOTTOM]);
		اگر (doc->children)
			doc->children->height = doc->height -
					(doc->children->margin[بالاترین_2] +
					 doc->children->border[بالاترین_2].width +
					 doc->children->padding[بالاترین_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].width +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	بازگشت ret;
}
