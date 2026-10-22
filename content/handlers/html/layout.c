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

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dom/dom.h>

#include "utils/log.h"
#include "utils/talloc.h"
#include "utils/utils.h"
#include "utils/nsoption.h"
#include "utils/corestrings.h"
#include "utils/nsurl.h"
#include "netsurf/inttypes.h"
#include "netsurf/content.h"
#include "netsurf/browser_window.h"
#include "netsurf/layout.h"
#include "content/content.h"
#include "content/content_protected.h"
#include "css/utils.h"
#include "desktop/scrollbar.h"
#include "desktop/textarea.h"

#include "html/html.h"
#include "html/html_save.h"
#include "html/private.h"
#include "html/box.h"
#include "html/box_inspect.h"
#include "html/font.h"
#include "html/form_internal.h"
#include "html/layout.h"
#include "html/layout_internal.h"
#include "html/table.h"

/** Array of per-side access functions for computed style margins. */
const css_len_func margin_funcs[4] = {
	[TOP]    = css_computed_margin_top,
	[RIGHT]  = css_computed_margin_right,
	[BOTTOM] = css_computed_margin_bottom,
	[LEFT]   = css_computed_margin_left,
};

/** Array of per-side access functions for computed style paddings. */
const css_len_func padding_funcs[4] = {
	[TOP]    = css_computed_padding_top,
	[RIGHT]  = css_computed_padding_right,
	[BOTTOM] = css_computed_padding_bottom,
	[LEFT]   = css_computed_padding_left,
};

/** Array of per-side access functions for computed style border_widths. */
const css_len_func border_width_funcs[4] = {
	[TOP]    = css_computed_border_top_width,
	[RIGHT]  = css_computed_border_right_width,
	[BOTTOM] = css_computed_border_bottom_width,
	[LEFT]   = css_computed_border_left_width,
};

/** Array of per-side access functions for computed style border styles. */
const css_border_style_func border_style_funcs[4] = {
	[TOP]    = css_computed_border_top_style,
	[RIGHT]  = css_computed_border_right_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[LEFT]   = css_computed_border_left_style,
};

/** Array of per-side access functions for computed style border colors. */
const css_border_color_func border_color_funcs[4] = {
	[TOP]    = css_computed_border_top_color,
	[RIGHT]  = css_computed_border_right_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[LEFT]   = css_computed_border_left_color,
};

/* forward declaration to break cycles */
static void layout_minmax_block(
		struct box *block,
		const struct gui_layout_table *font_func,
		const html_content *content);

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
static void
layout_get_object_dimensions(struct box *box,
			     целое *width, целое *height,
			     целое min_width, целое max_width,
			     целое min_height, целое max_height)
{
	assert(box->object != NULL);
	assert(width != NULL && height != NULL);

	если (*width == AUTO && *height == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		целое intrinsic_width = content_get_width(box->object);
		целое intrinsic_height = content_get_height(box->object);

		/* use intrinsic dimensions */
		*width = intrinsic_width;
		*height = intrinsic_height;

		/* Deal with min/max-width first */
		если (min_width >  0 && min_width > *width) {
			*width = min_width;
			scaled = true;
		}
		если (max_width >= 0 && max_width < *width) {
			*width = max_width;
			scaled = true;
		}

		если (scaled && (intrinsic_width != 0)) {
			/* Update height */
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		}

		scaled = false;
		/* Deal with min/max-height */
		если (min_height >  0 && min_height > *height) {
			*height = min_height;
			scaled = true;
		}
		если (max_height >= 0 && max_height < *height) {
			*height = max_height;
			scaled = true;
		}

		если (scaled && (intrinsic_height != 0)) {
			/* Update width */
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		}

	} иначе если (*width == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		целое intrinsic_width = content_get_width(box->object);
		целое intrinsic_height = content_get_height(box->object);

		если (intrinsic_height != 0)
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		иначе
			*width = intrinsic_width;

		если (min_width >  0 && min_width > *width)
			*width = min_width;
		если (max_width >= 0 && max_width < *width)
			*width = max_width;

	} иначе если (*height == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		целое intrinsic_width = content_get_width(box->object);
		целое intrinsic_height = content_get_height(box->object);

		если (min_width >  0 && min_width > *width)
			*width = min_width;
		если (max_width >= 0 && max_width < *width)
			*width = max_width;

		если (intrinsic_width != 0)
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		иначе
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
static целое layout_text_indent(
		const css_unit_ctx *unit_len_ctx,
		const css_computed_style *style, целое width)
{
	css_fixed value = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_text_indent(style, &value, &unit);

	если (unit == CSS_UNIT_PCT) {
		вернуть FPCT_OF_INT_TOINT(value, width);
	} иначе {
		вернуть FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				value, unit));
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
static void layout_minmax_table(struct box *table,
		const struct gui_layout_table *font_func,
		const html_content *content)
{
	unsigned целое i, j;
	целое width;
	целое border_spacing_h = 0;
	целое table_min = 0, table_max = 0;
	целое extra_fixed = 0;
	float extra_frac = 0;
	struct column *col;
	struct box *row_group, *row, *cell;

	/* check if the widths have already been calculated */
	если (table->max_width != UNKNOWN_MAX_WIDTH)
		вернуть;

	если (table_calculate_column_types(&content->unit_len_ctx, table) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		вернуть;
	}
	col = table->col;

	/* start with 0 except for fixed-width columns */
	для (i = 0; i != table->columns; i++) {
		если (col[i].type == COLUMN_WIDTH_FIXED)
			col[i].min = col[i].max = col[i].width;
		иначе
			col[i].min = col[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	если (css_computed_border_collapse(table->style) ==
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
	для (row_group = table->children; row_group; row_group =row_group->next)
	для (row = row_group->children; row; row = row->next)
	для (cell = row->children; cell; cell = cell->next) {
		assert(cell->type == BOX_TABLE_CELL);
		assert(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		assert(cell->columns != 0);

		если (cell->columns != 1)
			continue;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		если (col[i].positioned)
			continue;

		/* update column min, max widths using cell widths */
		если (col[i].min < cell->min_width)
			col[i].min = cell->min_width;
		если (col[i].max < cell->max_width)
			col[i].max = cell->max_width;
	}

	/* 2nd pass: cells which span multiple columns */
	для (row_group = table->children; row_group; row_group =row_group->next)
	для (row = row_group->children; row; row = row->next)
	для (cell = row->children; cell; cell = cell->next) {
		unsigned целое flexible_columns = 0;
		целое min = 0, max = 0, fixed_width = 0, extra;

		если (cell->columns == 1)
			continue;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		для (j = 0; j != cell->columns; j++) {
			min += col[i + j].min;
			если (col[i + j].type == COLUMN_WIDTH_FIXED)
				fixed_width += col[i + j].width;
			иначе
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		если (min < cell->min_width) {
			если (flexible_columns == 0) {
				extra = 1 + (cell->min_width - min) /
						cell->columns;
				для (j = 0; j != cell->columns; j++) {
					col[i + j].min += extra;
					если (col[i + j].max < col[i + j].min)
						col[i + j].max = col[i + j].min;
				}
			} иначе {
				extra = 1 + (cell->min_width - min) /
						flexible_columns;
				для (j = 0; j != cell->columns; j++) {
					если (col[i + j].type !=
							COLUMN_WIDTH_FIXED) {
						col[i + j].min += extra;
						если (col[i + j].max <
								col[i + j].min)
							col[i + j].max =
								col[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		для (j = 0; j != cell->columns; j++)
			max += col[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		если (max < cell->max_width && flexible_columns) {
			extra = 1 + (cell->max_width - max) / flexible_columns;
			для (j = 0; j != cell->columns; j++)
				если (col[i + j].type != COLUMN_WIDTH_FIXED)
					col[i + j].max += extra;
		}
	}

	для (i = 0; i != table->columns; i++) {
		если (col[i].max < col[i].min) {
			box_dump(stderr, table, 0, true);
			assert(0);
		}
		table_min += col[i].min;
		table_max += col[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	если (css_computed_width_px(table->style, &content->unit_len_ctx,
			-1, &width) == CSS_WIDTH_SET) {
		если (table_min < width)
			table_min = width;
		если (table_max < width)
			table_max = width;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, LEFT, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, RIGHT, true, true, true,
			&extra_fixed, &extra_frac);
	если (extra_fixed < 0)
		extra_fixed = 0;
	если (extra_frac < 0)
		extra_frac = 0;
	если (1.0 <= extra_frac)
		extra_frac = 0.9;
	table->min_width = (table_min + extra_fixed) / (1.0 - extra_frac);
	table->max_width = (table_max + extra_fixed) / (1.0 - extra_frac);
	table->min_width += (table->columns + 1) * border_spacing_h;
	table->max_width += (table->columns + 1) * border_spacing_h;

	assert(0 <= table->min_width && table->min_width <= table->max_width);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
static inline bool box_has_percentage_max_width(struct box *b)
{
	css_unit unit = CSS_UNIT_PX;
	enum css_max_width_e type;
	css_fixed value = 0;

	assert(b != NULL);

	type = css_computed_max_width(b->style, &value, &unit);
	вернуть ((type == CSS_MAX_WIDTH_SET) && (unit == CSS_UNIT_PCT));
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
static struct box *
layout_minmax_line(struct box *first,
		   целое *line_min,
		   целое *line_max,
		   bool first_line,
		   bool *line_has_height,
		   const struct gui_layout_table *font_func,
		   const html_content *content)
{
	целое min = 0, max = 0, width, height, fixed;
	float frac;
	size_t i, j;
	struct box *b;
	struct box *block;
	plot_font_style_t fstyle;
	bool no_wrap;

	assert(first->parent);
	assert(first->parent->parent);
	assert(first->parent->parent->style);

	block = first->parent->parent;
	no_wrap = (css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_PRE);

	*line_has_height = false;

	/* corresponds to the pass 1 loop in layout_line() */
	для (b = first; b; b = b->next) {
		enum css_width_e wtype;
		enum css_height_e htype;
		enum css_box_sizing_e bs;
		css_fixed value = 0;
		css_unit unit = CSS_UNIT_PX;

		assert(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		если (b->type == BOX_BR) {
			b = b->next;
			break;
		}

		если (lh__box_is_float_box(b)) {
			assert(b->children);
			если (b->children->type == BOX_TABLE)
				layout_minmax_table(b->children, font_func,
						content);
			иначе
				layout_minmax_block(b->children, font_func,
						content);
			b->min_width = b->children->min_width;
			b->max_width = b->children->max_width;
			если (min < b->min_width)
				min = b->min_width;
			max += b->max_width;
			continue;
		}

		если (b->type == BOX_INLINE_BLOCK || b->type == BOX_INLINE_FLEX) {
			layout_minmax_block(b, font_func, content);
			если (min < b->min_width)
				min = b->min_width;
			max += b->max_width;

			если (b->flags & HAS_HEIGHT)
				*line_has_height = true;
			continue;
		}

		assert(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		если (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->style, LEFT, true, true, true,
					&fixed, &frac);
			если (!b->inline_end)
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, RIGHT,
						true, true, true,
						&fixed, &frac);
			если (0 < fixed)
				max += fixed;
			*line_has_height = true;
			/* \todo  update min width, consider fractional extra */
		} иначе если (b->type == BOX_INLINE_END) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->inline_end->style, RIGHT,
					true, true, true,
					&fixed, &frac);
			если (0 < fixed)
				max += fixed;

			если (b->next) {
				если (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*line_has_height = true;
			continue;
		}

		если (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			если (!b->text)
				continue;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			если (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				если (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					целое opt_maxwidth = 0;
					struct form_option *o;

					для (o = b->parent->parent->gadget->
							data.select.items; o;
							o = o->next) {
						целое opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						если (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}

					b->width = opt_maxwidth;
					если (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;

				} иначе {
					font_func->width(&fstyle, b->text,
						b->length, &b->width);
					b->flags |= MEASURED;
				}
			}
			max += b->width;
			если (b->next) {
				если (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			если (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} иначе если (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				если (min < b->width)
					min = b->width;

			} иначе если (b->parent->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				do {
					для (j = i; j != b->length &&
							b->text[j] != ' '; j++)
						;
					font_func->width(&fstyle, b->text + i,
							 j - i, &width);
					если (min < width)
						min = width;
					i = j + 1;
				} пока (j != b->length);
			}

			*line_has_height = true;

			continue;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		assert(b->style);

		bs = css_computed_box_sizing(block->style);

		/* calculate box width */
		wtype = css_computed_width_px(b->style,
				&content->unit_len_ctx, -1, &width);
		если (wtype == CSS_WIDTH_SET) {
			если (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, LEFT,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, RIGHT,
						false, true, true,
						&fixed, &frac);
				если (width < fixed) {
					width = fixed;
				}
			}
			если (width < 0)
				width = 0;
		} иначе {
			width = AUTO;
		}

		/* height */
		htype = css_computed_height(b->style, &value, &unit);
		если (htype == CSS_HEIGHT_SET) {
			height = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					value, unit));
		} иначе {
			height = AUTO;
		}

		если (b->object || (b->flags & REPLACE_DIM)) {
			если (b->object) {
				целое temp_height = height;
				layout_get_object_dimensions(b,
						&width, &temp_height,
						INT_MIN, INT_MAX,
						INT_MIN, INT_MAX);
			}

			fixed = frac = 0;
			если (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, LEFT,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, RIGHT,
						true, false, false,
						&fixed, &frac);
			} иначе {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, LEFT,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, RIGHT,
						true, true, true,
						&fixed, &frac);
			}
			если (0 < width + fixed)
				width += fixed;
		} иначе если (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			если (width == AUTO)
				width = 400;

			fixed = frac = 0;
			если (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, LEFT,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, RIGHT,
						true, false, false,
						&fixed, &frac);
			} иначе {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, LEFT,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, RIGHT,
						true, true, true,
						&fixed, &frac);
			}

			если (0 < width + fixed)
				width += fixed;

		} иначе {
			/* form control with no object */
			если (width == AUTO)
				width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		если (min < width && !box_has_percentage_max_width(b))
			min = width;
		если (width > 0)
			max += width;

		*line_has_height = true;
	}

	если (first_line) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		целое text_indent = layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, 100);
		min = (min + text_indent < 0) ? 0 : min + text_indent;
		max = (max + text_indent < 0) ? 0 : max + text_indent;
	}

	*line_min = min;
	*line_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	assert(b != first);
	assert(0 <= *line_min);
	assert(*line_min <= *line_max);
	вернуть b;
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
static void
layout_minmax_inline_container(struct box *inline_container,
			       bool *has_height,
			       const struct gui_layout_table *font_func,
			       const html_content *content)
{
	struct box *child;
	целое line_min = 0, line_max = 0;
	целое min = 0, max = 0;
	bool first_line = true;
	bool line_has_height;

	assert(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	если (inline_container->max_width != UNKNOWN_MAX_WIDTH)
		вернуть;

	*has_height = false;

	для (child = inline_container->children; child; ) {
		child = layout_minmax_line(child, &line_min, &line_max,
				first_line, &line_has_height, font_func,
				content);
		если (min < line_min)
			min = line_min;
		если (max < line_max)
			max = line_max;
		first_line = false;
		*has_height |= line_has_height;
	}

	inline_container->min_width = min;
	inline_container->max_width = max;

	assert(0 <= inline_container->min_width &&
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
static void layout_minmax_block(
		struct box *block,
		const struct gui_layout_table *font_func,
		const html_content *content)
{
	struct box *child;
	целое min = 0, max = 0;
	целое extra_fixed = 0;
	float extra_frac = 0;
	enum css_width_e wtype = CSS_WIDTH_AUTO;
	css_fixed width = 0;
	css_unit wunit = CSS_UNIT_PX;
	enum css_height_e htype = CSS_HEIGHT_AUTO;
	css_fixed height = 0;
	css_unit hunit = CSS_UNIT_PX;
	enum css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool child_has_height = false;

	assert(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	/* check if the widths have already been calculated */
	если (block->max_width != UNKNOWN_MAX_WIDTH)
		вернуть;

	если (block->style != NULL) {
		wtype = css_computed_width(block->style, &width, &wunit);
		htype = css_computed_height(block->style, &height, &hunit);
		bs = css_computed_box_sizing(block->style);
	}

	/* set whether the minimum width is of any interest for this box */
	если (((block->parent && lh__box_is_float_box(block->parent)) ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX) &&
			wtype != CSS_WIDTH_SET) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} иначе если (block->type == BOX_TABLE_CELL) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} иначе если (block->parent && (block->parent->flags & NEED_MIN) &&
			wtype != CSS_WIDTH_SET) {
		/* box inside shrink-to-fit context; need minimum width */
		block->flags |= NEED_MIN;
	} иначе если (block->parent && (block->parent->type == BOX_FLEX)) {
		/* box is flex item */
		block->flags |= NEED_MIN;
	}

	если (block->gadget && (block->gadget->type == GADGET_TEXTBOX ||
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

	если (block->gadget && (block->gadget->type == GADGET_RADIO ||
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

	если (block->object) {
		если (content_get_type(block->object) == CONTENT_HTML) {
			layout_minmax_block(html_get_box_tree(block->object),
					font_func, content);
			min = html_get_box_tree(block->object)->min_width;
			max = html_get_box_tree(block->object)->max_width;
		} иначе {
			min = max = content_get_width(block->object);
		}

		block->flags |= HAS_HEIGHT;
	} иначе если (block->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		block->flags |= HAS_HEIGHT;
	} иначе {
		/* recurse through children */
		для (child = block->children; child; child = child->next) {
			switch (child->type) {
			case BOX_FLEX:
			case BOX_BLOCK:
				layout_minmax_block(child, font_func,
						content);
				если (child->flags & HAS_HEIGHT)
					child_has_height = true;
				break;
			case BOX_INLINE_CONTAINER:
				если (block->flags & NEED_MIN)
					child->flags |= NEED_MIN;

				layout_minmax_inline_container(child,
						&child_has_height, font_func,
						content);
				если (child_has_height &&
						child ==
						child->parent->children) {
					block->flags |= MAKE_HEIGHT;
				}
				break;
			case BOX_TABLE:
				layout_minmax_table(child, font_func,
						content);
				/* todo: fix for zero height tables */
				child_has_height = true;
				child->flags |= MAKE_HEIGHT;
				break;
			default:
				assert(0);
			}
			assert(child->max_width != UNKNOWN_MAX_WIDTH);

			если (child->style &&
					(css_computed_position(child->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(child->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				continue;
			}

			если (lh__box_is_flex_container(block) &&
			    lh__flex_main_is_horizontal(block)) {
				если (block->style != NULL &&
				    css_computed_flex_wrap(block->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += child->min_width;
				} иначе {
					если (min < child->min_width)
						min = child->min_width;
				}
				max += child->max_width;

			} иначе {
				если (min < child->min_width)
					min = child->min_width;
				если (max < child->max_width)
					max = child->max_width;
			}

			если (child_has_height)
				block->flags |= HAS_HEIGHT;
		}
	}

	если (max < min) {
		box_dump(stderr, block, 0, true);
		assert(0);
	}

	/* fixed width takes priority */
	если (block->type != BOX_TABLE_CELL && !lh__box_is_flex_item(block)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		enum css_max_width_e max_type;
		enum css_min_width_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed value = 0;
		целое width;

		если (css_computed_width_px(block->style, &content->unit_len_ctx,
				-1, &width) == CSS_WIDTH_SET) {
			min = max = width;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_width(block->style, &value, &unit);
		если (min_type == CSS_MIN_WIDTH_SET && unit != CSS_UNIT_PCT) {
			целое val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, value, unit));

			если (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_width(block->style, &value, &unit);
		если (max_type == CSS_MAX_WIDTH_SET && unit != CSS_UNIT_PCT) {
			целое val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, value, unit));

			если (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	если (htype == CSS_HEIGHT_SET && hunit != CSS_UNIT_PCT &&
			height > INTTOFIX(0)) {
		block->flags |= MAKE_HEIGHT;
		block->flags |= HAS_HEIGHT;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_width(&content->unit_len_ctx, block->style, LEFT,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, RIGHT,
			false, true, true, &extra_fixed, &extra_frac);

	если (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	если (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	если (max < min) {
		min = max;
	}

	calculate_mbp_width(&content->unit_len_ctx, block->style, LEFT,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, RIGHT,
			true, false, false, &extra_fixed, &extra_frac);

	если (extra_fixed < 0)
		extra_fixed = 0;
	если (extra_frac < 0)
		extra_frac = 0;
	если (1.0 <= extra_frac)
		extra_frac = 0.9;
	если (block->style != NULL &&
			(css_computed_float(block->style) == CSS_FLOAT_LEFT ||
			css_computed_float(block->style) == CSS_FLOAT_RIGHT)) {
		/* floated boxs */
		block->min_width = min + extra_fixed;
		block->max_width = max + extra_fixed;
	} иначе {
		/* not floated */
		block->min_width = (min + extra_fixed) / (1.0 - extra_frac);
		block->max_width = (max + extra_fixed) / (1.0 - extra_frac);
	}

	assert(0 <= block->min_width);
	assert(block->min_width <= block->max_width);
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
static struct box*
layout_next_margin_block(const css_unit_ctx *unit_len_ctx,
			 struct box *box,
			 struct box *block,
			 целое viewport_height,
			 целое *max_pos_margin,
			 целое *max_neg_margin)
{
	assert(block != NULL);

	пока (box != NULL) {

		если (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			если (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						NULL, NULL, NULL, NULL,
						NULL, NULL, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				если (*max_pos_margin < box->margin[TOP])
					*max_pos_margin = box->margin[TOP];
				иначе если (*max_neg_margin < -box->margin[TOP])
					*max_neg_margin = -box->margin[TOP];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			если (box->flags & MAKE_HEIGHT ||
					box->border[TOP].width ||
					box->padding[TOP] ||
					(box->style &&
					css_computed_overflow_y(box->style) !=
					CSS_OVERFLOW_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_first_child(box))) {
				/* Collapse to this box; return it */
				вернуть box;
			}
		}


		/* Find next box */
		если (box->type == BOX_BLOCK && !box->object && box->children &&
				box->style &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} иначе {
			если (!box->next) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				do {
					/* Apply bottom margin */
					если (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					иначе если (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->parent;
				} пока (box != block && !box->next);

				если (box == block) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					вернуть block;
				}
			}

			/* Apply bottom margin */
			если (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			иначе если (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->next;

			/* Get margins */
			если (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						NULL, NULL, NULL, NULL,
						NULL, NULL, box->margin,
						box->padding, box->border);
			}
		}
	}

	вернуть NULL;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
static целое layout_clear(struct box *fl, enum css_clear_e clear)
{
	целое y = 0;
	для (; fl; fl = fl->next_float) {
		если ((clear == CSS_CLEAR_LEFT || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_LEFT)
			если (y < fl->y + fl->height)
				y = fl->y + fl->height;
		если ((clear == CSS_CLEAR_RIGHT || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_RIGHT)
			если (y < fl->y + fl->height)
				y = fl->y + fl->height;
	}
	вернуть y;
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
static void
find_sides(struct box *fl,
	   целое y0, целое y1,
	   целое *x0, целое *x1,
	   struct box **left,
	   struct box **right)
{
	целое fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*left = *right = 0;
	для (; fl; fl = fl->next_float) {
		fy1 = fl->y + fl->height;
		если (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			вернуть;
		}
		fy0 = fl->y;
		если (y0 < fy1 && fy0 <= y1) {
			если (fl->type == BOX_FLOAT_LEFT) {
				fx1 = fl->x + fl->width;
				если (*x0 < fx1) {
					*x0 = fx1;
					*left = fl;
				}
			} иначе {
				fx0 = fl->x;
				если (fx0 < *x1) {
					*x1 = fx0;
					*right = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *left, *right);
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
static целое
layout_solve_width(struct box *box,
		   целое available_width,
		   целое width,
		   целое lm,
		   целое rm,
		   целое max_width,
		   целое min_width)
{
	bool auto_width = false;

	/* Increase specified left/right margins */
	если (box->margin[LEFT] != AUTO && box->margin[LEFT] < lm &&
			box->margin[LEFT] >= 0)
		box->margin[LEFT] = lm;
	если (box->margin[RIGHT] != AUTO && box->margin[RIGHT] < rm &&
			box->margin[RIGHT] >= 0)
		box->margin[RIGHT] = rm;

	/* Find width */
	если (width == AUTO) {
		целое margin_left = box->margin[LEFT];
		целое margin_right = box->margin[RIGHT];

		если (margin_left == AUTO) {
			margin_left = lm;
		}
		если (margin_right == AUTO) {
			margin_right = rm;
		}

		width = available_width -
				(margin_left + box->border[LEFT].width +
				box->padding[LEFT] + box->padding[RIGHT] +
				box->border[RIGHT].width + margin_right);
		width = width < 0 ? 0 : width;
		auto_width = true;
	}

	если (max_width >= 0 && width > max_width) {
		/* max-width is admissable and width exceeds max-width */
		width = max_width;
		auto_width = false;
	}

	если (min_width > 0 && width < min_width) {
		/* min-width is admissable and width is less than max-width */
		width = min_width;
		auto_width = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	если (auto_width) {
		/* any other 'auto' become 0 or the minimum required values */
		если (box->margin[LEFT] == AUTO) {
			box->margin[LEFT] = lm;
		}
		если (box->margin[RIGHT] == AUTO) {
			box->margin[RIGHT] = rm;
		}
		вернуть width;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	если (box->margin[LEFT] != AUTO && box->margin[RIGHT] != AUTO &&
			box->parent != NULL && box->parent->style != NULL) {
		switch (css_computed_text_align(box->parent->style)) {
		case CSS_TEXT_ALIGN_LIBCSS_RIGHT:
			box->margin[LEFT] = AUTO;
			box->margin[RIGHT] = 0;
			break;
		case CSS_TEXT_ALIGN_LIBCSS_CENTER:
			box->margin[LEFT] = box->margin[RIGHT] = AUTO;
			break;
		case CSS_TEXT_ALIGN_LIBCSS_LEFT:
			box->margin[LEFT] = 0;
			box->margin[RIGHT] = AUTO;
			break;
		default:
			/* Leave it alone; no HTML alignment */
			break;
		}
	}

	если (box->margin[LEFT] == AUTO && box->margin[RIGHT] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[LEFT] = box->margin[RIGHT] =
				(available_width - lm - rm -
				(box->border[LEFT].width + box->padding[LEFT] +
				width + box->padding[RIGHT] +
				box->border[RIGHT].width)) / 2;

		если (box->margin[LEFT] < 0) {
			box->margin[RIGHT] += box->margin[LEFT];
			box->margin[LEFT] = 0;
		}

		box->margin[LEFT] += lm;

	} иначе если (box->margin[LEFT] == AUTO) {
		box->margin[LEFT] = available_width - lm -
				(box->border[LEFT].width + box->padding[LEFT] +
				width + box->padding[RIGHT] +
				box->border[RIGHT].width + box->margin[RIGHT]);
		box->margin[LEFT] = box->margin[LEFT] < lm
				? lm : box->margin[LEFT];
	} иначе {
		/* margin-right auto or "over-constrained" */
		box->margin[RIGHT] = available_width - rm -
				(box->margin[LEFT] + box->border[LEFT].width +
				 box->padding[LEFT] + width +
				 box->padding[RIGHT] +
				 box->border[RIGHT].width);
	}

	вернуть width;
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
static void
layout_block_find_dimensions(const css_unit_ctx *unit_len_ctx,
			     целое available_width,
			     целое viewport_height,
			     целое lm,
			     целое rm,
			     struct box *box)
{
	целое width, max_width, min_width;
	целое height, max_height, min_height;
	целое *margin = box->margin;
	целое *padding = box->padding;
	struct box_border *border = box->border;
	const css_computed_style *style = box->style;

	layout_find_dimensions(unit_len_ctx, available_width, viewport_height, box,
			style, &width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	если (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	}

	box->width = layout_solve_width(box, available_width, width, lm, rm,
			max_width, min_width);
	box->height = height;

	если (margin[TOP] == AUTO)
		margin[TOP] = 0;
	если (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
static void layout_block_add_scrollbar(struct box *box, целое which)
{
	enum css_overflow_e overflow_x, overflow_y;

	assert(box->type == BOX_BLOCK && (which == RIGHT || which == BOTTOM));

	если (box->style == NULL)
		вернуть;

	overflow_x = css_computed_overflow_x(box->style);
	overflow_y = css_computed_overflow_y(box->style);

	если (which == BOTTOM &&
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		если (box->height != AUTO &&
				(overflow_x == CSS_OVERFLOW_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_WIDTH;
		}

	} иначе если (which == RIGHT &&
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		enum css_height_e htype;
		css_fixed height = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_height(box->style, &height, &hunit);

		если (which == RIGHT && box->width != AUTO &&
				htype == CSS_HEIGHT_SET &&
				(overflow_y == CSS_OVERFLOW_SCROLL ||
				box_vscrollbar_present(box))) {
			box->width -= SCROLLBAR_WIDTH;
			box->padding[RIGHT] += SCROLLBAR_WIDTH;
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
static void layout_move_children(struct box *box, целое x, целое y)
{
	assert(box);

	для (box = box->children; box; box = box->next) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_table(
		struct box *table,
		целое available_width,
		html_content *content)
{
	unsigned целое columns = table->columns;  /* total columns */
	unsigned целое i;
	unsigned целое *row_span;
	целое *excess_y;
	целое table_width, min_width = 0, max_width = 0;
	целое required_width = 0;
	целое x, remainder = 0, count = 0;
	целое table_height = 0;
	целое min_height = 0;
	целое *xs;  /* array of column x positions */
	целое auto_width;
	целое spare_width;
	целое relative_sum = 0;
	целое border_spacing_h = 0, border_spacing_v = 0;
	целое spare_height;
	целое positioned_columns = 0;
	struct box *containing_block = NULL;
	struct box *c;
	struct box *row;
	struct box *row_group;
	struct box **row_span_cell;
	struct column *col;
	const css_computed_style *style = table->style;
	enum css_height_e htype;
	css_fixed value = 0;
	css_unit unit = CSS_UNIT_PX;

	assert(table->type == BOX_TABLE);
	assert(style);
	assert(table->children && table->children->children);
	assert(columns);

	/* allocate working buffers */
	col = malloc(columns * sizeof col[0]);
	excess_y = malloc(columns * sizeof excess_y[0]);
	row_span = malloc(columns * sizeof row_span[0]);
	row_span_cell = malloc(columns * sizeof row_span_cell[0]);
	xs = malloc((columns + 1) * sizeof xs[0]);
	если (!col || !xs || !row_span || !excess_y || !row_span_cell) {
		free(col);
		free(excess_y);
		free(row_span);
		free(row_span_cell);
		free(xs);
		вернуть false;
	}

	memcpy(col, table->col, sizeof(col[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1, table,
			style, 0, 0, 0, 0, 0, 0, table->margin, table->padding,
			table->border);
	для (row_group = table->children; row_group;
			row_group = row_group->next) {
		для (row = row_group->children; row; row = row->next) {
			для (c = row->children; c; c = c->next) {
				enum css_overflow_e overflow_x;
				enum css_overflow_e overflow_y;

				assert(c->style);
				table_used_border_for_cell(
						&content->unit_len_ctx, c);
				layout_find_dimensions(&content->unit_len_ctx,
						available_width, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				overflow_x = css_computed_overflow_x(c->style);
				overflow_y = css_computed_overflow_y(c->style);

				если (overflow_x == CSS_OVERFLOW_SCROLL ||
						overflow_x ==
						CSS_OVERFLOW_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_WIDTH;
				}
				если (overflow_y == CSS_OVERFLOW_SCROLL ||
						overflow_y ==
						CSS_OVERFLOW_AUTO) {
					c->padding[RIGHT] += SCROLLBAR_WIDTH;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	если (css_computed_border_collapse(style) ==
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
	если (css_computed_width_px(style, &content->unit_len_ctx,
			available_width, &table_width) == CSS_WIDTH_SET) {
		/* specified width includes border */
		table_width -= table->border[LEFT].width +
				table->border[RIGHT].width;
		table_width = table_width < 0 ? 0 : table_width;

		auto_width = table_width;
	} иначе {
		table_width = AUTO;
		auto_width = available_width -
				((table->margin[LEFT] == AUTO ? 0 :
						table->margin[LEFT]) +
				 table->border[LEFT].width +
				 table->padding[LEFT] +
				 table->padding[RIGHT] +
				 table->border[RIGHT].width +
				 (table->margin[RIGHT] == AUTO ? 0 :
						table->margin[RIGHT]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_height(style, &value, &unit);
	если (htype == CSS_HEIGHT_SET) {
		если (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			если (css_computed_position(table->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				assert(table->float_container);
				containing_block = table->float_container;
			} иначе если (table->float_container &&
					css_computed_position(table->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(table->style) ==
						CSS_FLOAT_LEFT ||
					css_computed_float(table->style) ==
						CSS_FLOAT_RIGHT)) {
				/* Table is a float */
				assert(table->parent && table->parent->parent &&
						table->parent->parent->parent);
				containing_block =
						table->parent->parent->parent;
			} иначе если (table->parent && table->parent->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_block = table->parent;
			} иначе если (table->parent && table->parent->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				assert(table->parent->parent);
				containing_block = table->parent->parent;
			}

			если (containing_block) {
				css_fixed ignored = 0;

				htype = css_computed_height(
						containing_block->style,
						&ignored, &unit);
			}

			если (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(table->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_HEIGHT_SET)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_height = FPCT_OF_INT_TOINT(value,
						containing_block->height);
			}
		} иначе {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_height = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					value, unit));
		}
	}

	/* calculate width required by cells */
	для (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      table,
		      i,
		      ((const char *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[col[i].type],
		      col[i].width,
		      col[i].min,
		      col[i].max);


		если (col[i].positioned) {
			positioned_columns++;
			continue;
		} иначе если (col[i].type == COLUMN_WIDTH_FIXED) {
			если (col[i].width < col[i].min)
				col[i].width = col[i].max = col[i].min;
			иначе
				col[i].min = col[i].max = col[i].width;
			required_width += col[i].width;
		} иначе если (col[i].type == COLUMN_WIDTH_PERCENT) {
			целое width = col[i].width * auto_width / 100;
			required_width += col[i].min < width ? width :
					col[i].min;
		} иначе
			required_width += col[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_width);
	}
	required_width += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", table_width,
	      table->min_width, table->max_width, auto_width, required_width);

	если (auto_width < required_width) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		для (i = 0; i != columns; i++) {
			если (col[i].type == COLUMN_WIDTH_RELATIVE)
				continue;
			если (col[i].type == COLUMN_WIDTH_PERCENT) {
				col[i].max = auto_width * col[i].width / 100;
				если (col[i].max < col[i].min)
					col[i].max = col[i].min;
			}
			min_width += col[i].min;
			max_width += col[i].max;
		}
	} иначе {
		/* take percentages exactly */
		для (i = 0; i != columns; i++) {
			если (col[i].type == COLUMN_WIDTH_RELATIVE)
				continue;
			если (col[i].type == COLUMN_WIDTH_PERCENT) {
				целое width = auto_width * col[i].width / 100;
				если (width < col[i].min)
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
	для (i = 0; i != columns; i++) {
		если (col[i].type == COLUMN_WIDTH_RELATIVE)
			relative_sum += col[i].width;
		иначе если (col[i].type == COLUMN_WIDTH_FIXED)
			spare_width -= col[i].width;
		иначе
			spare_width -= col[i].min;
	}
	spare_width -= (columns + 1) * border_spacing_h;
	если (relative_sum != 0) {
		если (spare_width < 0)
			spare_width = 0;
		для (i = 0; i != columns; i++) {
			если (col[i].type == COLUMN_WIDTH_RELATIVE) {
				col[i].min = ceil(col[i].max =
						(float) spare_width
						* (float) col[i].width
						/ relative_sum);
				min_width += col[i].min;
				max_width += col[i].max;
			}
		}
	}
	min_width += (columns + 1) * border_spacing_h;
	max_width += (columns + 1) * border_spacing_h;

	если (auto_width <= min_width) {
		/* not enough space: minimise column widths */
		для (i = 0; i < columns; i++) {
			col[i].width = col[i].min;
		}
		table_width = min_width;
	} иначе если (max_width <= auto_width) {
		/* more space than maximum width */
		если (table_width == AUTO) {
			/* for auto-width tables, make columns max width */
			для (i = 0; i < columns; i++) {
				col[i].width = col[i].max;
			}
			table_width = max_width;
		} иначе {
			/* for fixed-width tables, distribute the extra space
			 * too */
			unsigned целое flexible_columns = 0;
			для (i = 0; i != columns; i++)
				если (col[i].type != COLUMN_WIDTH_FIXED)
					flexible_columns++;
			если (flexible_columns == 0) {
				целое extra = (table_width - max_width) / columns;
				remainder = (table_width - max_width) -
						(extra * columns);
				для (i = 0; i != columns; i++) {
					col[i].width = col[i].max + extra;
					count -= remainder;
					если (count < 0) {
						col[i].width++;
						count += columns;
					}
				}

			} иначе {
				целое extra = (table_width - max_width) /
						flexible_columns;
				remainder = (table_width - max_width) -
						(extra * flexible_columns);
				для (i = 0; i != columns; i++)
					если (col[i].type != COLUMN_WIDTH_FIXED) {
						col[i].width = col[i].max +
								extra;
						count -= remainder;
						если (count < 0) {
							col[i].width++;
							count += flexible_columns;
						}
					}
			}
		}
	} иначе {
		/* space between min and max: fill it exactly */
		float scale = (float) (auto_width - min_width) /
				(float) (max_width - min_width);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		для (i = 0; i < columns; i++) {
			col[i].width = col[i].min + (целое) (0.5 +
					(col[i].max - col[i].min) * scale);
		}
		table_width = auto_width;
	}

	xs[0] = x = border_spacing_h;
	для (i = 0; i != columns; i++) {
		если (!col[i].positioned)
			x += col[i].width + border_spacing_h;
		xs[i + 1] = x;
		row_span[i] = 0;
		excess_y[i] = 0;
		row_span_cell[i] = 0;
	}

	/* position cells */
	table_height = border_spacing_v;
	для (row_group = table->children; row_group;
			row_group = row_group->next) {
		целое row_group_height = 0;
		для (row = row_group->children; row; row = row->next) {
			целое row_height = 0;

			htype = css_computed_height(row->style, &value, &unit);
			если (htype == CSS_HEIGHT_SET && unit != CSS_UNIT_PCT) {
				row_height = FIXTOINT(css_unit_len2device_px(
						row->style,
						&content->unit_len_ctx,
						value, unit));
			}
			для (c = row->children; c; c = c->next) {
				assert(c->style);
				c->width = xs[c->start_column + c->columns] -
						xs[c->start_column] -
						border_spacing_h -
						c->border[LEFT].width -
						c->padding[LEFT] -
						c->padding[RIGHT] -
						c->border[RIGHT].width;
				c->float_children = 0;
				c->cached_place_below_level = 0;

				c->height = AUTO;
				если (!layout_block_context(c, -1, content)) {
					free(col);
					free(excess_y);
					free(row_span);
					free(row_span_cell);
					free(xs);
					вернуть false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->height;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_height(c->style,
						&value, &unit);

				если (htype == CSS_HEIGHT_SET &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					целое h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							value, unit));
					если (c->height < h)
						c->height = h;
				}
				/* specified row height is treated as a minimum
				 */
				если (c->height < row_height)
					c->height = row_height;
				c->x = xs[c->start_column] +
						c->border[LEFT].width;
				c->y = c->border[TOP].width;
				для (i = 0; i != c->columns; i++) {
					row_span[c->start_column + i] = c->rows;
					excess_y[c->start_column + i] =
							c->border[TOP].width +
							c->padding[TOP] +
							c->height +
							c->padding[BOTTOM] +
							c->border[BOTTOM].width;
					row_span_cell[c->start_column + i] = 0;
				}
				row_span_cell[c->start_column] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[TOP].width -
						c->padding[TOP] -
						c->height -
						c->border[BOTTOM].width;
			}
			для (i = 0; i != columns; i++)
				если (row_span[i] != 0)
					row_span[i]--;
				иначе
					row_span_cell[i] = 0;
			если (row->next || row_group->next) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				для (i = 0; i != columns; i++)
					если (row_span[i] == 0 && row_height <
							excess_y[i])
						row_height = excess_y[i];
			} иначе {
				/* except in the last row */
				для (i = 0; i != columns; i++)
					если (row_height < excess_y[i])
						row_height = excess_y[i];
			}
			для (i = 0; i != columns; i++) {
				если (row_height < excess_y[i])
					excess_y[i] -= row_height;
				иначе
					excess_y[i] = 0;
				если (row_span_cell[i] != 0)
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
	для (row_group = table->children; row_group;
			row_group = row_group->next) {
		для (row = row_group->children; row; row = row->next) {
			для (c = row->children; c; c = c->next) {
				enum css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				spare_height = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->height - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &value, &unit);

				switch (vertical_align) {
				case CSS_VERTICAL_ALIGN_SUB:
				case CSS_VERTICAL_ALIGN_SUPER:
				case CSS_VERTICAL_ALIGN_TEXT_TOP:
				case CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				case CSS_VERTICAL_ALIGN_SET:
				case CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				case CSS_VERTICAL_ALIGN_TOP:
					break;
				case CSS_VERTICAL_ALIGN_MIDDLE:
					c->padding[TOP] += spare_height / 2;
					c->padding[BOTTOM] -= spare_height / 2;
					layout_move_children(c, 0,
							spare_height / 2);
					break;
				case CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[TOP] += spare_height;
					c->padding[BOTTOM] -= spare_height;
					layout_move_children(c, 0,
							spare_height);
					break;
				case CSS_VERTICAL_ALIGN_INHERIT:
					assert(0);
					break;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	если (table->margin[TOP] == AUTO)
		table->margin[TOP] = 0;
	если (table->margin[BOTTOM] == AUTO)
		table->margin[BOTTOM] = 0;

	free(col);
	free(excess_y);
	free(row_span);
	free(row_span_cell);
	free(xs);

	table->width = table_width;
	table->height = table_height;

	вернуть true;
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
static bool layout_apply_minmax_height(
		const css_unit_ctx *unit_len_ctx,
		struct box *box,
		struct box *container)
{
	целое h;
	struct box *containing_block = NULL;
	bool updated = false;

	/* Find containing block for percentage heights */
	если (box->style != NULL && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		assert(container);
		containing_block = container;
	} иначе если (box->float_container && box->style != NULL &&
			(css_computed_float(box->style) == CSS_FLOAT_LEFT ||
			 css_computed_float(box->style) == CSS_FLOAT_RIGHT)) {
		/* Box is a float */
		assert(box->parent && box->parent->parent &&
				box->parent->parent->parent);
		containing_block = box->parent->parent->parent;
	} иначе если (box->parent && box->parent->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_block = box->parent;
	} иначе если (box->parent && box->parent->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		assert(box->parent->parent);
		containing_block = box->parent->parent;
	}

	если (box->style) {
		enum css_height_e htype = CSS_HEIGHT_AUTO;
		css_fixed value = 0;
		css_unit unit = CSS_UNIT_PX;

		если (containing_block) {
			htype = css_computed_height(containing_block->style,
					&value, &unit);
		}

		/* max-height */
		если (css_computed_max_height(box->style, &value, &unit) ==
				CSS_MAX_HEIGHT_SET) {
			если (unit == CSS_UNIT_PCT) {
				если (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(value,
						containing_block->height);
					если (h < box->height) {
						box->height = h;
						updated = true;
					}
				}
			} иначе {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						value, unit));
				если (h < box->height) {
					box->height = h;
					updated = true;
				}
			}
		}

		/* min-height */
		если (ns_computed_min_height(box->style, &value, &unit) ==
				CSS_MIN_HEIGHT_SET) {
			если (unit == CSS_UNIT_PCT) {
				если (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(value,
						containing_block->height);
					если (h > box->height) {
						box->height = h;
						updated = true;
					}
				}
			} иначе {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						value, unit));
				если (h > box->height) {
					box->height = h;
					updated = true;
				}
			}
		}
	}
	вернуть updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
static bool layout_block_object(struct box *block)
{
	assert(block);
	assert(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_TABLE ||
			block->type == BOX_TABLE_CELL);
	assert(block->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", block,
	      hlcache_handle_get_url(block->object), block->width);

	если (content_can_reformat(block->object)) {
		content_reformat(block->object, false, block->width, 1);
	} иначе {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	вернуть true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
static void add_float_to_container(struct box *cont, struct box *b)
{
	struct box *box = cont->float_children;
	целое b_bottom = b->y + b->height;

	assert(b->type == BOX_FLOAT_LEFT || b->type == BOX_FLOAT_RIGHT);

	если (box == NULL) {
		/* No other float children */
		b->next_float = NULL;
		cont->float_children = b;
		вернуть;
	} иначе если (b_bottom >= box->y + box->height) {
		/* Goes at start of list */
		b->next_float = cont->float_children;
		cont->float_children = b;
	} иначе {
		struct box *prev = NULL;
		пока (box != NULL && b_bottom < box->y + box->height) {
			prev = box;
			box = box->next_float;
		}
		если (prev != NULL) {
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
static bool
layout_text_box_split(html_content *content,
		      plot_font_style_t *fstyle,
		      struct box *split_box,
		      size_t new_length,
		      целое new_width)
{
	целое space_width = split_box->space;
	struct box *c2;
	const struct gui_layout_table *font_func = content->font_func;
	bool space = (split_box->text[new_length] == ' ');
	целое used_length = new_length + (space ? 1 : 0);

	если ((space && space_width == 0) || space_width == UNKNOWN_WIDTH) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->width(fstyle, " ", 1, &space_width);
	}

	если (split_box->space == UNKNOWN_WIDTH)
		split_box->space = space_width;
	если (!space)
		space_width = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, split_box, sizeof *c2);
	если (!c2)
		вернуть false;
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
	если (c2->next)
		c2->next->prev = c2;
	иначе
		c2->parent->last = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      split_box,
	      split_box->length,
	      (целое)split_box->length,
	      split_box->text);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->length,
	      (целое)c2->length,
	      c2->text);

	вернуть true;
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
static void
layout_float_find_dimensions(
		const css_unit_ctx *unit_len_ctx,
		целое available_width,
		const css_computed_style *style,
		struct box *box)
{
	целое width, height, max_width, min_width, max_height, min_height;
	целое *margin = box->margin;
	целое *padding = box->padding;
	struct box_border *border = box->border;
	enum css_overflow_e overflow_x = css_computed_overflow_x(style);
	enum css_overflow_e overflow_y = css_computed_overflow_y(style);
	целое scrollbar_width_x =
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;
	целое scrollbar_width_y =
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;

	layout_find_dimensions(unit_len_ctx, available_width, -1, box, style,
			&width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	если (margin[LEFT] == AUTO)
		margin[LEFT] = 0;
	если (margin[RIGHT] == AUTO)
		margin[RIGHT] = 0;

	если (box->gadget == NULL) {
		padding[RIGHT] += scrollbar_width_y;
		padding[BOTTOM] += scrollbar_width_x;
	}

	если (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	} иначе если (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed size = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		assert(box->style);

		если (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			если (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			если (box->gadget->type == GADGET_FILE &&
					height == AUTO) {
				size = FLTTOFIX(1.5);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
		если (box->gadget->type == GADGET_TEXTAREA) {
			если (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			если (height == AUTO) {
				size = INTTOFIX(4);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
	} иначе если (width == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		width = min(max(box->min_width, available_width),
				box->max_width);

		/* width includes margin, borders and padding */
		если (width == available_width) {
			width -= box->margin[LEFT] + box->border[LEFT].width +
					box->padding[LEFT] +
					box->padding[RIGHT] +
					box->border[RIGHT].width +
					box->margin[RIGHT];
		} иначе {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			целое fixed = 0;
			float frac = 0;
			calculate_mbp_width(unit_len_ctx, box->style, LEFT,
					true, true, true, &fixed, &frac);
			calculate_mbp_width(unit_len_ctx, box->style, RIGHT,
					true, true, true, &fixed, &frac);
			если (fixed < 0)
				fixed = 0;

			width -= fixed;
		}

		если (max_width >= 0 && width > max_width) width = max_width;
		если (min_width >  0 && width < min_width) width = min_width;

	} иначе {
		если (max_width >= 0 && width > max_width) width = max_width;
		если (min_width >  0 && width < min_width) width = min_width;
		width -= scrollbar_width_y;
	}

	box->width = width;
	box->height = height;

	если (margin[TOP] == AUTO)
		margin[TOP] = 0;
	если (margin[BOTTOM] == AUTO)
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
static bool layout_float(struct box *b, целое width, html_content *content)
{
	assert(b->type == BOX_TABLE ||
	       b->type == BOX_BLOCK ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_find_dimensions(&content->unit_len_ctx, width, b->style, b);
	если (b->type == BOX_TABLE || b->type == BOX_INLINE_FLEX) {
		если (b->type == BOX_TABLE) {
			если (!layout_table(b, width, content))
				вернуть false;
		} иначе {
			если (!layout_flex(b, width, content))
				вернуть false;
		}
		если (b->margin[LEFT] == AUTO)
			b->margin[LEFT] = 0;
		если (b->margin[RIGHT] == AUTO)
			b->margin[RIGHT] = 0;
		если (b->margin[TOP] == AUTO)
			b->margin[TOP] = 0;
		если (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} иначе {
		вернуть layout_block_context(b, -1, content);
	}
	вернуть true;
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
static void
place_float_below(struct box *c, целое width, целое cx, целое y, struct box *cont)
{
	целое x0, x1, yy;
	struct box *left;
	struct box *right;

	yy = y > cont->cached_place_below_level ?
			y : cont->cached_place_below_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      width, cx, y, cont);

	do {
		y = yy;
		x0 = cx;
		x1 = cx + width;
		find_sides(cont->float_children, y, y + c->height, &x0, &x1,
				&left, &right);
		если (left != 0 && right != 0) {
			yy = (left->y + left->height <
					right->y + right->height ?
					left->y + left->height :
					right->y + right->height);
		} иначе если (left == 0 && right != 0) {
			yy = right->y + right->height;
		} иначе если (left != 0 && right == 0) {
			yy = left->y + left->height;
		}
	} пока ((left != 0 || right != 0) && (c->width > x1 - x0));

	если (c->type == BOX_FLOAT_LEFT) {
		c->x = x0;
	} иначе {
		c->x = x1 - c->width;
	}
	c->y = y;
	cont->cached_place_below_level = y;
}


/**
 * Calculate line height from a style.
 */
static целое line_height(
		const css_unit_ctx *unit_len_ctx,
		const css_computed_style *style)
{
	enum css_line_height_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed line_height;

	assert(style);

	lhtype = css_computed_line_height(style, &lhvalue, &lhunit);
	если (lhtype == CSS_LINE_HEIGHT_NORMAL) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_LINE_HEIGHT_NUMBER;
	}

	если (lhtype == CSS_LINE_HEIGHT_NUMBER ||
			lhunit == CSS_UNIT_PCT) {
		line_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		если (lhtype != CSS_LINE_HEIGHT_NUMBER)
			line_height = FDIV(line_height, F_100);
	} иначе {
		assert(lhunit != CSS_UNIT_PCT);

		line_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	вернуть FIXTOINT(line_height);
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
static bool
layout_line(struct box *first,
	    целое *width,
	    целое *y,
	    целое cx,
	    целое cy,
	    struct box *cont,
	    bool indent,
	    bool has_text_children,
	    html_content *content,
	    struct box **next_box)
{
	целое height, used_height;
	целое x0 = 0;
	целое x1 = *width;
	целое x, h, x_previous;
	целое fy = cy;
	struct box *left;
	struct box *right;
	struct box *b;
	struct box *split_box = 0;
	struct box *d;
	struct box *br_box = 0;
	bool move_y = false;
	bool place_below = false;
	целое space_before = 0, space_after = 0;
	unsigned целое inline_count = 0;
	unsigned целое i;
	const struct gui_layout_table *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      first,
	      (целое)first->length,
	      first->text,
	      *width,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	find_sides(cont->float_children, cy, cy, &x0, &x1, &left, &right);
	x0 -= cx;
	x1 -= cx;

	если (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	если (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	если (has_text_children || first->parent->parent->gadget)
		used_height = height = line_height(&content->unit_len_ctx,
				first->parent->parent->style);
	иначе
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		used_height = height = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	для (x = 0, b = first; x <= x1 - x0 && b != 0; b = b->next) {
		целое min_width, max_width, min_height, max_height;

		assert(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		если (b->type == BOX_BR)
			break;

		если (lh__box_is_float_box(b))
			continue;
		если (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			continue;

		assert(b->style != NULL);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		если (b->type == BOX_INLINE_BLOCK ||
		    b->type == BOX_INLINE_FLEX) {
			если (b->max_width != UNKNOWN_WIDTH)
				если (!layout_float(b, *width, content))
					вернуть false;
			h = b->border[TOP].width + b->padding[TOP] + b->height +
					b->padding[BOTTOM] +
					b->border[BOTTOM].width;
			если (height < h)
				height = h;
			x += b->margin[LEFT] + b->border[LEFT].width +
					b->padding[LEFT] + b->width +
					b->padding[RIGHT] +
					b->border[RIGHT].width +
					b->margin[RIGHT];
			space_after = 0;
			continue;
		}

		если (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_find_dimensions(&content->unit_len_ctx,
					*width, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			для (i = 0; i != 4; i++)
				если (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[LEFT] + b->border[LEFT].width +
					b->padding[LEFT];
			если (b->inline_end) {
				b->inline_end->margin[RIGHT] = b->margin[RIGHT];
				b->inline_end->padding[RIGHT] =
						b->padding[RIGHT];
				b->inline_end->border[RIGHT] =
						b->border[RIGHT];
			} иначе {
				x += b->padding[RIGHT] +
						b->border[RIGHT].width +
						b->margin[RIGHT];
			}
		} иначе если (b->type == BOX_INLINE_END) {
			b->width = 0;
			если (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[RIGHT] + b->border[RIGHT].width +
					b->margin[RIGHT];
			continue;
		}

		если (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->height = line_height(&content->unit_len_ctx,
					b->style ? b->style :
					b->parent->parent->style);
			если (height < b->height)
				height = b->height;

			если (!b->text) {
				b->width = 0;
				space_after = 0;
				continue;
			}

			если (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				если (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					целое opt_maxwidth = 0;
					struct form_option *o;

					для (o = b->parent->parent->gadget->
							data.select.items; o;
							o = o->next) {
						целое opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						если (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}
					b->width = opt_maxwidth;
					если (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;
				} иначе {
					font_func->width(&fstyle, b->text,
							b->length, &b->width);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			если (b->text && (x + b->width < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->next) {
				font_func->width(&fstyle, b->text,
						 b->length, &b->width);
				b->flags |= MEASURED;
			}

			x += b->width;
			если (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			continue;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		assert(b->style);

		layout_find_dimensions(&content->unit_len_ctx,
				*width, -1, b, b->style,
				&b->width, &b->height,
				&max_width, &min_width,
				&max_height, &min_height,
				NULL, NULL, NULL);

		если (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->width, &b->height,
					min_width, max_width,
					min_height, max_height);
		} иначе если (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			если (b->width == AUTO)
				b->width = 400;
			если (b->height == AUTO)
				b->height = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} иначе {
			/* form control with no object */
			если (b->width == AUTO)
				b->width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			если (b->height == AUTO)
				b->height = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		если (b->object && content_can_reformat(b->object) &&
				b->width !=
				content_get_available_width(b->object)) {
			css_fixed value = 0;
			css_unit unit = CSS_UNIT_PX;
			enum css_height_e htype = css_computed_height(b->style,
					&value, &unit);

			content_reformat(b->object, false, b->width, b->height);

			если (htype == CSS_HEIGHT_AUTO)
				b->height = content_get_height(b->object);
		}

		если (height < b->height)
			height = b->height;

		x += b->width;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *width;
	find_sides(cont->float_children, cy, cy + height, &x0, &x1,
			&left, &right);
	x0 -= cx;
	x1 -= cx;

	если (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	если (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	для (x = x_previous = 0, b = first; x <= x1 - x0 && b; b = b->next) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		если (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} иначе если (lh__box_is_inline_flow(b)) {
			assert(b->width != UNKNOWN_WIDTH);

			x_previous = x;
			x += space_after;
			b->x = x;

			если ((b->type == BOX_INLINE && !b->inline_end) ||
					b->type == BOX_INLINE_BLOCK ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[LEFT] + b->border[LEFT].width;
				x = b->x + b->padding[LEFT] + b->width +
						b->padding[RIGHT] +
						b->border[RIGHT].width +
						b->margin[RIGHT];
			} иначе если (b->type == BOX_INLINE) {
				b->x += b->margin[LEFT] + b->border[LEFT].width;
				x = b->x + b->padding[LEFT] + b->width;
			} иначе если (b->type == BOX_INLINE_END) {
				b->height = b->inline_end->height;
				x += b->padding[RIGHT] +
						b->border[RIGHT].width +
						b->margin[RIGHT];
			} иначе {
				x += b->width;
			}

			space_before = space_after;
			если (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			иначе если (b->text || b->type == BOX_INLINE_END) {
				если (b->space == UNKNOWN_WIDTH) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} иначе {
				space_after = 0;
			}
			split_box = b;
			move_y = true;
			inline_count++;
		} иначе если (b->type == BOX_BR) {
			b->x = x;
			b->width = 0;
			br_box = b;
			b = b->next;
			split_box = 0;
			move_y = true;
			break;

		} иначе {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_place_below_level = 0;
			b->float_container = d->float_container = cont;

			если (!layout_float(d, *width, content))
				вернуть false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[TOP],
			      d->border[TOP].width);

			d->x = d->margin[LEFT] + d->border[LEFT].width;
			d->y = d->margin[TOP] + d->border[TOP].width;
			b->width = d->margin[LEFT] + d->border[LEFT].width +
					d->padding[LEFT] + d->width +
					d->padding[RIGHT] +
					d->border[RIGHT].width +
					d->margin[RIGHT];
			b->height = d->margin[TOP] + d->border[TOP].width +
					d->padding[TOP] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];

			если (b->width > (x1 - x0) - x)
				place_below = true;
			если (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_LEFT && left == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_RIGHT &&
						right == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						left == 0 && right == 0)) &&
					(!place_below ||
					(left == 0 && right == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_place_below_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				если (b->type == BOX_FLOAT_LEFT) {
					b->x = cx + x0;
					если (b->width > 0)
						x0 += b->width;
					left = b;
				} иначе {
					b->x = cx + x1 - b->width;
					если (b->width > 0)
						x1 -= b->width;
					right = b;
				}
				b->y = cy;
			} иначе {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				целое fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_place_below_level) ?
						fcy :
						cont->cached_place_below_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + height : fy;

				place_float_below(b, *width, cx, fy, cont);
				fy = b->y;
				если (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_LEFT && left != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_RIGHT &&
						right != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(left != 0 || right != 0)))) {
					/* to be cleared below existing
					 * floats */
					если (b->type == BOX_FLOAT_LEFT)
						b->x = cx;
					иначе
						b->x = cx + *width - b->width;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					если (fcy > cont->clear_level)
						cont->clear_level = fcy;
					если (b->y < fcy)
						b->y = fcy;
				}
				если (b->type == BOX_FLOAT_LEFT)
					left = b;
				иначе
					right = b;
			}
			add_float_to_container(cont, b);

			split_box = 0;
		}
	}

	если (x1 - x0 < x && split_box) {
		/* the last box went over the end */
		size_t split = 0;
		целое w;
		bool no_wrap = css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_previous;

		если (!no_wrap &&
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

		если (split == 0)
			w = split_box->width;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      split_box,
		      (целое)split_box->length,
		      split_box->text,
		      split,
		      w,
		      left,
		      right,
		      inline_count);

		если ((split == 0 || x1 - x0 <= x + space_before + w) &&
				!left && !right && inline_count == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			если (split == 0 || split == split_box->length) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = split_box->next;
			} иначе {
				/* cut off first word for this line */
				если (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					вернуть false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} иначе если ((split == 0 || x1 - x0 <= x + space_before + w) &&
				inline_count == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			assert(left || right);
			used_height = 0;
			если (left) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      left->y,
				      left->height);

				used_height = left->y + left->height - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      used_height);

			}
			если (right && used_height <
					right->y + right->height - cy + 1)
				used_height = right->y + right->height - cy + 1;

			если (used_height < 0)
				used_height = 0;

			b = split_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} иначе если (split == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = split_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} иначе {
			/* fit as many words as possible */
			assert(split != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (целое)split_box->length, split_box->text,
			      x1 - x0, split, w);

			если (split != split_box->length) {
				если (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					вернуть false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	switch (css_computed_text_align(first->parent->parent->style)) {
	case CSS_TEXT_ALIGN_RIGHT:
	case CSS_TEXT_ALIGN_LIBCSS_RIGHT:
		x0 = x1 - x;
		break;
	case CSS_TEXT_ALIGN_CENTER:
	case CSS_TEXT_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		break;
	case CSS_TEXT_ALIGN_LEFT:
	case CSS_TEXT_ALIGN_LIBCSS_LEFT:
	case CSS_TEXT_ALIGN_JUSTIFY:
		/* leave on left */
		break;
	case CSS_TEXT_ALIGN_DEFAULT:
		/* None; consider text direction */
		switch (css_computed_direction(first->parent->parent->style)) {
		case CSS_DIRECTION_LTR:
			/* leave on left */
			break;
		case CSS_DIRECTION_RTL:
			x0 = x1 - x;
			break;
		}
		break;
	}

	для (d = first; d != b; d = d->next) {
		d->flags &= ~NEW_LINE;

		если (d->type == BOX_INLINE_BLOCK &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			continue;
		} иначе если ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[TOP];

			если (d->type == BOX_TEXT && d->height > used_height) {
				/* text */
				used_height = d->height;
			}
		} иначе если ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_BLOCK) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[TOP].width + d->margin[TOP];
			h = d->margin[TOP] + d->border[TOP].width +
					d->padding[TOP] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];
			если (used_height < h)
				used_height = h;
		}
	}

	first->flags |= NEW_LINE;

	assert(b != first || (move_y && 0 < used_height && (left || right)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	для (d = first; d != b; d = d->next) {
		если ((d->type == BOX_INLINE && d->inline_end) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			css_fixed value = 0;
			css_unit unit = CSS_UNIT_PX;
			switch (css_computed_vertical_align(d->style, &value,
					&unit)) {
			case CSS_VERTICAL_ALIGN_SUPER:
			case CSS_VERTICAL_ALIGN_TOP:
			case CSS_VERTICAL_ALIGN_TEXT_TOP:
				/* already at top */
				break;
			case CSS_VERTICAL_ALIGN_SUB:
			case CSS_VERTICAL_ALIGN_BOTTOM:
			case CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				d->y += used_height - d->height;
				break;
			default:
			case CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (used_height - d->height);
				break;
			}
		}
	}

	/* handle clearance for br */
	если (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		целое clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		если (used_height < clear_y - cy)
			used_height = clear_y - cy;
	}

	если (move_y)
		*y += used_height;
	*next_box = b;
	*width = x; /* return actual width */
	вернуть true;
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
static bool layout_inline_container(struct box *inline_container, целое width,
		struct box *cont, целое cx, целое cy, html_content *content)
{
	bool first_line = true;
	bool has_text_children;
	struct box *c, *next;
	целое y = 0;
	целое curwidth,maxwidth = width;

	assert(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      width,
	      cont,
	      cx,
	      cy);


	has_text_children = false;
	для (c = inline_container->children; c; c = c->next) {
		bool is_pre = false;

		если (c->style) {
			enum css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_LINE ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		если ((lh__box_is_object(c) == false &&
				c->text && (c->length || is_pre)) ||
				c->type == BOX_BR)
			has_text_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	для (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->width;
		если (!layout_line(c, &curwidth, &y, cx, cy + y, cont, first_line,
				has_text_children, content, &next))
			вернуть false;
		maxwidth = max(maxwidth,curwidth);
		c = next;
		first_line = false;
	}

	inline_container->width = maxwidth;
	inline_container->height = y;

	вернуть true;
}


/* Documented in layout_intertnal.h */
bool layout_block_context(
		struct box *block,
		целое viewport_height,
		html_content *content)
{
	struct box *box;
	целое cx, cy;  /**< current coordinates */
	целое max_pos_margin = 0;
	целое max_neg_margin = 0;
	целое y = 0;
	целое lm, rm;
	struct box *margin_collapse = NULL;
	bool in_margin = false;
	css_fixed gadget_size;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	assert(block->type == BOX_BLOCK ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX);
	assert(block->width != UNKNOWN_WIDTH);
	assert(block->width != AUTO);

	block->float_children = NULL;
	block->cached_place_below_level = 0;
	block->clear_level = 0;

	/* special case if the block contains an object */
	если (block->object) {
		целое temp_width = block->width;
		если (!layout_block_object(block))
			вернуть false;
		layout_get_object_dimensions(block, &temp_width,
				&block->height, INT_MIN, INT_MAX,
				INT_MIN, INT_MAX);
		вернуть true;
	} иначе если (block->flags & REPLACE_DIM) {
		вернуть true;
	}

	/* special case if the block contains an radio button or checkbox */
	если (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_size = INTTOFIX(1);
		если (block->height == AUTO)
			block->height = FIXTOINT(css_unit_len2device_px(
					block->style,
					&content->unit_len_ctx,
					gadget_size, gadget_unit));
	}

	box = block->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = block->padding[TOP];
	если (box)
		box->y = block->padding[TOP];

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
	пока (box) {
		enum css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
		enum css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

		assert(box->type == BOX_BLOCK ||
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

		если (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->parent->padding[LEFT];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			goto advance_to_next_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		если (margin_collapse == NULL) {
			margin_collapse = layout_next_margin_block(
					&content->unit_len_ctx, box, block,
					viewport_height,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			in_margin = true;
		}

		/* Clearance. */
		y = 0;
		если (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(block->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		если (box->style) {
			overflow_x = css_computed_overflow_x(box->style);
			overflow_y = css_computed_overflow_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		если (box->type == BOX_FLEX ||
		    box->type == BOX_BLOCK ||
		    box->flags & IFRAME) {
			если (lh__box_is_object(box) == false &&
					box->style &&
					(overflow_x != CSS_OVERFLOW_VISIBLE ||
					 overflow_y != CSS_OVERFLOW_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				целое x0, x1, top;
				struct box *left, *right;
				top = cy + max_pos_margin - max_neg_margin;
				top = (top > y) ? top : y;
				x0 = cx;
				x1 = cx + box->parent->width -
						box->parent->padding[LEFT] -
						box->parent->padding[RIGHT];
				find_sides(block->float_children, top, top,
						&x0, &x1, &left, &right);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->parent->width -
						box->parent->padding[LEFT] -
						box->parent->padding[RIGHT] -
						x1;
			}
			layout_block_find_dimensions(&content->unit_len_ctx,
					box->parent->width,
					viewport_height, lm, rm, box);
			если (box->type == BOX_BLOCK && !(box->flags & IFRAME)) {
				layout_block_add_scrollbar(box, RIGHT);
				layout_block_add_scrollbar(box, BOTTOM);
			}
		} иначе если (box->type == BOX_TABLE) {
			если (box->style != NULL) {
				enum css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style, &width,
						&unit);

				если (wtype == CSS_WIDTH_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					целое x0, x1, top;
					struct box *left, *right;
					top = cy + max_pos_margin -
							max_neg_margin;
					top = (top > y) ? top : y;
					x0 = cx;
					x1 = cx + box->parent->width -
						box->parent->padding[LEFT] -
						box->parent->padding[RIGHT];
					find_sides(block->float_children,
						top, top, &x0, &x1,
						&left, &right);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->parent->width -
						box->parent->padding[LEFT] -
						box->parent->padding[RIGHT] -
						x1;
				}
			}
			если (!layout_table(box, box->parent->width - lm - rm,
					content))
				вернуть false;
			layout_solve_width(box, box->parent->width, box->width,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->parent->padding[LEFT] + box->margin[LEFT] +
				box->border[LEFT].width;
		cx += box->x;

		/* Position box: vertical. */
		если (box->border[TOP].width) {
			box->y += box->border[TOP].width;
			cy += box->border[TOP].width;
		}

		/* Vertical margin */
		если (((box->type == BOX_BLOCK && (box->flags & HAS_HEIGHT)) ||
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
		если (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		если (box->type == BOX_FLEX ||
				(box->type == BOX_BLOCK && box->style &&
				 (overflow_x != CSS_OVERFLOW_VISIBLE ||
				  overflow_y != CSS_OVERFLOW_VISIBLE))) {

			если (box->type == BOX_FLEX) {
				если (!layout_flex(box, box->width, content)) {
					вернуть false;
				}
			} иначе {
				layout_block_context(box,
						viewport_height, content);
			}

			cy += box->padding[TOP];

			если (box->height == AUTO) {
				box->height = 0;
				layout_block_add_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->height + box->padding[BOTTOM] +
					box->border[BOTTOM].width;
			y = box->y + box->padding[TOP] + box->height +
					box->padding[BOTTOM] +
					box->border[BOTTOM].width;

			/* Skip children, because they are done in the new
			 * block context */
			goto advance_to_next_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->width);

		/* Layout (except tables). */
		если (box->object) {
			если (!layout_block_object(box))
				вернуть false;

		} иначе если (box->type == BOX_INLINE_CONTAINER) {
			box->width = box->parent->width;
			если (!layout_inline_container(box, box->width, block,
					cx, cy, content))
				вернуть false;

		} иначе если (box->type == BOX_TABLE) {
			/* Move down to avoid floats if necessary. */
			целое x0, x1;
			struct box *left, *right;
			y = cy;
			пока (1) {
				enum css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style,
						&width, &unit);

				x0 = cx;
				x1 = cx + box->parent->width;
				find_sides(block->float_children, y,
						y + box->height,
						&x0, &x1, &left, &right);
				если (wtype == CSS_WIDTH_AUTO)
					break;
				если (box->width <= x1 - x0)
					break;
				если (!left && !right)
					break;
				иначе если (!left)
					y = right->y + right->height + 1;
				иначе если (!right)
					y = left->y + left->height + 1;
				иначе если (left->y + left->height <
						right->y + right->height)
					y = left->y + left->height + 1;
				иначе
					y = right->y + right->height + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		если (box->type == BOX_BLOCK && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			если (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = NULL;
			}

			y = box->padding[TOP];
			box = box->children;
			box->y = y;
			cy += y;
			continue;
		} иначе если (box->type == BOX_BLOCK || box->object ||
				box->flags & IFRAME)
			cy += box->padding[TOP];

		если (box->type == BOX_BLOCK && box->height == AUTO) {
			box->height = 0;
			layout_block_add_scrollbar(box, BOTTOM);
		}

		cy += box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width;
		cx -= box->x;
		y = box->y + box->padding[TOP] + box->height +
				box->padding[BOTTOM] +
				box->border[BOTTOM].width;

	advance_to_next_box:
		если (!box->next) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			do {
				если (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = NULL;
				}

				/* Apply bottom margin */
				если (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				иначе если (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->parent;
				если (box == block)
					break;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				если (box->type == BOX_BLOCK &&
						box->flags & MAKE_HEIGHT) {
					margin_collapse = NULL;
					in_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				если (box->height == AUTO) {
					box->height = y - box->padding[TOP];

					если (box->type == BOX_BLOCK)
						layout_block_add_scrollbar(box,
								BOTTOM);
				} иначе
					cy += box->height -
							(y - box->padding[TOP]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				если (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_height(
							&content->unit_len_ctx,
							box, NULL)) {
					/* Height altered */
					/* Set current cy */
					cy += box->height -
							(y - box->padding[TOP]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].width;
				cx -= box->x;
				y = box->y + box->padding[TOP] + box->height +
						box->padding[BOTTOM] +
						box->border[BOTTOM].width;

			} пока (box->next == NULL);
			если (box == block)
				break;
		}

		/* To next sibling. */

		если (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = NULL;
		}

		если (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		иначе если (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->next;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	для (box = block->float_children; box; box = box->next_float) {
		y = box->y + box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width + box->margin[BOTTOM];
		если (cy < y)
			cy = y;
	}

	если (block->height == AUTO) {
		block->height = cy - block->padding[TOP];
		если (block->type == BOX_BLOCK)
			layout_block_add_scrollbar(block, BOTTOM);
	}

	если (block->style && css_computed_position(block->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_height(&content->unit_len_ctx, block, NULL);
	}

	если (block->gadget &&
			(block->gadget->type == GADGET_TEXTAREA ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		целое ta_width = block->padding[LEFT] + block->width +
				block->padding[RIGHT];
		целое ta_height = block->padding[TOP] + block->height +
				block->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				block->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_set_layout(block->gadget->data.text.ta,
				&fstyle, ta_width, ta_height,
				block->padding[TOP], block->padding[RIGHT],
				block->padding[BOTTOM], block->padding[LEFT]);
	}

	вернуть true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
static bool
layout__get_element_tag(
		const dom_node *node,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_node_type node_type;
	dom_exception exc;

	exc = dom_node_get_node_type(node, &node_type);
	если (exc != DOM_NO_ERR ||
	    node_type != DOM_ELEMENT_NODE) {
		вернуть false;
	}

	exc = dom_html_element_get_tag_type(node, &element_type);
	если (exc != DOM_NO_ERR) {
		вернуть false;
	}

	*type = element_type;
	вернуть true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
static inline bool
layout__check_element_type(
		const dom_node *node,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	если (!layout__get_element_tag(node, &element_type)) {
		вернуть false;
	}

	вернуть element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
static bool
layout__get_li_value(dom_node *li_node, dom_long *value_out)
{
	dom_exception exc;
	dom_long value;
	bool has_value;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_node,
			corestring_dom_value,
			&has_value);
	если (exc != DOM_NO_ERR || has_value == false) {
		вернуть false;
	}

	exc = dom_html_li_element_get_value(
			(dom_html_li_element *)li_node,
			&value);
	если (exc != DOM_NO_ERR) {
		вернуть false;
	}

	*value_out = value;
	вернуть true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
static bool
layout__get_ol_start(dom_node *ol_node, dom_long *start_out)
{
	dom_exception exc;
	dom_long start;
	bool has_start;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_node,
			corestring_dom_start,
			&has_start);
	если (exc != DOM_NO_ERR || has_start == false) {
		вернуть false;
	}

	exc = dom_html_olist_element_get_start(
			(dom_html_olist_element *)ol_node,
			&start);
	если (exc != DOM_NO_ERR) {
		вернуть false;
	}

	*start_out = start;
	вернуть true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
static bool
layout__get_ol_reversed(dom_node *ol_node)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_node,
			corestring_dom_reversed,
			&has_reversed);
	если (exc != DOM_NO_ERR) {
		вернуть false;
	}

	вернуть has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
static bool
layout__get_list_item_count(
		dom_node *list_owner, dom_long *count_out)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_node *child;
	целое count;

	если (list_owner == NULL) {
		вернуть false;
	}

	если (!layout__get_element_tag(list_owner, &tag_type)) {
		вернуть false;
	}

	если (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		вернуть false;
	}

	exc = dom_node_get_first_child(list_owner, &child);
	если (exc != DOM_NO_ERR) {
		вернуть false;
	}

	count = 0;
	пока (child != NULL) {
		dom_node *temp_node;

		если (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			struct box *child_box;
			если (dom_node_get_user_data(child,
					corestring_dom___ns_key_box_node_data,
					&child_box) != DOM_NO_ERR) {
				dom_node_unref(child);
				вернуть false;
			}

			если (child_box != NULL &&
			    child_box->list_marker != NULL) {
				count++;
			}
		}

		exc = dom_node_get_next_sibling(child, &temp_node);
		dom_node_unref(child);
		если (exc != DOM_NO_ERR) {
			вернуть false;
		}

		child = temp_node;
	}

	*count_out = count;
	вернуть true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
static void
layout__ordered_list_count(
		struct box *box)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_node *child;
	целое step = 1;
	dom_long next;

	если (box->node == NULL) {
		вернуть;
	}

	если (!layout__get_element_tag(box->node, &tag_type)) {
		вернуть;
	}

	если (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		вернуть;
	}

	next = 1;
	если (tag_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_start = layout__get_ol_start(box->node, &next);
		bool have_reversed = layout__get_ol_reversed(box->node);

		если (have_reversed) {
			step = -1;
		}

		если (!have_start && have_reversed) {
			layout__get_list_item_count(box->node, &next);
		}
	}

	exc = dom_node_get_first_child(box->node, &child);
	если (exc != DOM_NO_ERR) {
		вернуть;
	}

	пока (child != NULL) {
		dom_node *temp_node;

		если (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			struct box *child_box;

			если (dom_node_get_user_data(child,
					corestring_dom___ns_key_box_node_data,
					&child_box) != DOM_NO_ERR) {
				dom_node_unref(child);
				вернуть;
			}

			если (child_box != NULL &&
			    child_box->list_marker != NULL) {
				dom_long value;
				struct box *marker = child_box->list_marker;
				если (layout__get_li_value(child, &value)) {
					marker->list_value = value;
					next = marker->list_value;
				} иначе {
					marker->list_value = next;
				}
				next += step;
			}
		}

		exc = dom_node_get_next_sibling(child, &temp_node);
		dom_node_unref(child);
		если (exc != DOM_NO_ERR) {
			вернуть;
		}

		child = temp_node;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
static void
layout__set_numerical_marker_text(
		const html_content *content,
		struct box *box)
{
	struct box *marker = box->list_marker;
	size_t counter_len;
	css_error css_res;
	enum {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_SIZE = 20,
	};

	marker->text = talloc_array(content->bctx, char, LIST_MARKER_SIZE);
	если (marker->text == NULL) {
		вернуть;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_value,
			marker->text, LIST_MARKER_SIZE, &counter_len);
	если (css_res == CSS_OK) {
		если (counter_len > LIST_MARKER_SIZE) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->text = talloc_realloc(content->bctx,
					marker->text,
					char,
					counter_len);
			если (marker->text == NULL) {
				вернуть;
			}
			css_computed_format_list_style(box->style,
					marker->list_value, marker->text,
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
static bool
layout__list_item_is_numerical(
		const struct box *b)
{
	enum css_list_style_type_e t = css_computed_list_style_type(b->style);

	switch (t) {
	case CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	case CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	case CSS_LIST_STYLE_TYPE_SQUARE: /* Fall through. */
	case CSS_LIST_STYLE_TYPE_NONE:
		вернуть false;

	default:
		вернуть true;
	}
}

/**
 * Layout list markers.
 */
static void
layout_lists(const html_content *content, struct box *box)
{
	struct box *child;

	layout__ordered_list_count(box);

	для (child = box->children; child; child = child->next) {
		если (child->list_marker) {
			struct box *marker = child->list_marker;

			если (layout__list_item_is_numerical(child)) {
				если (marker->text == NULL) {
					layout__set_numerical_marker_text(
							content, child);
				}
			}
			если (marker->object) {
				marker->width =
					content_get_width(marker->object);
				marker->x = -marker->width;
				marker->height =
					content_get_height(marker->object);
				marker->y = (line_height(
						&content->unit_len_ctx,
						marker->style) -
						marker->height) / 2;
			} иначе если (marker->text) {
				если (marker->width == UNKNOWN_WIDTH) {
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
				marker->height = line_height(
						&content->unit_len_ctx,
						marker->style);
			} иначе {
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
static void
layout_compute_offsets(const css_unit_ctx *unit_len_ctx,
		       struct box *box,
		       struct box *containing_block,
		       целое *top,
		       целое *right,
		       целое *bottom,
		       целое *left)
{
	uint32_t type;
	css_fixed value = 0;
	css_unit unit = CSS_UNIT_PX;

	assert(containing_block->width != UNKNOWN_WIDTH);
	assert(containing_block->width != AUTO);
	assert(containing_block->height != AUTO);

	/* left */
	type = css_computed_left(box->style, &value, &unit);
	если (type == CSS_LEFT_SET) {
		если (unit == CSS_UNIT_PCT) {
			*left = FPCT_OF_INT_TOINT(value,
					containing_block->width);
		} иначе {
			*left = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					value, unit));
		}
	} иначе {
		*left = AUTO;
	}

	/* right */
	type = css_computed_right(box->style, &value, &unit);
	если (type == CSS_RIGHT_SET) {
		если (unit == CSS_UNIT_PCT) {
			*right = FPCT_OF_INT_TOINT(value,
					containing_block->width);
		} иначе {
			*right = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					value, unit));
		}
	} иначе {
		*right = AUTO;
	}

	/* top */
	type = css_computed_top(box->style, &value, &unit);
	если (type == CSS_TOP_SET) {
		если (unit == CSS_UNIT_PCT) {
			*top = FPCT_OF_INT_TOINT(value,
					containing_block->height);
		} иначе {
			*top = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					value, unit));
		}
	} иначе {
		*top = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &value, &unit);
	если (type == CSS_BOTTOM_SET) {
		если (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_OF_INT_TOINT(value,
					containing_block->height);
		} иначе {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					value, unit));
		}
	} иначе {
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
static bool
layout_absolute(struct box *box,
		struct box *containing_block,
		целое cx, целое cy,
		html_content *content)
{
	целое static_left, static_top;  /* static position */
	целое top, right, bottom, left;
	целое width, height, max_width, min_width;
	целое *margin = box->margin;
	целое *padding = box->padding;
	struct box_border *border = box->border;
	целое available_width = containing_block->width;
	целое space;

	assert(box->type == BOX_BLOCK || box->type == BOX_TABLE ||
			box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_left = cx + box->x;
	static_top = cy + box->y;

	если (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_block->width += containing_block->padding[LEFT] +
				containing_block->padding[RIGHT];
		containing_block->height += containing_block->padding[TOP] +
				containing_block->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_block,
			&top, &right, &bottom, &left);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_block;
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1,
			box, box->style, &width, &height,
			&max_width, &min_width, 0, 0,
			margin, padding, border);
	box->float_container = NULL;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      left, margin[LEFT], border[LEFT].width, padding[LEFT], width,
	      padding[RIGHT], border[RIGHT].width, margin[RIGHT], right,
	      containing_block->width);


	если (left == AUTO && width == AUTO && right == AUTO) {
		если (margin[LEFT] == AUTO)
			margin[LEFT] = 0;
		если (margin[RIGHT] == AUTO)
			margin[RIGHT] = 0;
		left = static_left;

		width = min(max(box->min_width, available_width),
			box->max_width);
		width -= box->margin[LEFT] + box->border[LEFT].width +
			box->padding[LEFT] + box->padding[RIGHT] +
			box->border[RIGHT].width + box->margin[RIGHT];

		/* Adjust for {min|max}-width */
		если (max_width >= 0 && width > max_width) width = max_width;
		если (width < min_width) width = min_width;

		right = containing_block->width -
			left -
			margin[LEFT] - border[LEFT].width - padding[LEFT] -
			width -
			padding[RIGHT] - border[RIGHT].width - margin[RIGHT];
	} иначе если (left != AUTO && width != AUTO && right != AUTO) {

		/* Adjust for {min|max}-width */
		если (max_width >= 0 && width > max_width) width = max_width;
		если (min_width >  0 && width < min_width) width = min_width;

		если (margin[LEFT] == AUTO && margin[RIGHT] == AUTO) {
			space = containing_block->width -
					left - border[LEFT].width -
					padding[LEFT] - width - padding[RIGHT] -
					border[RIGHT].width - right;
			если (space < 0) {
				margin[LEFT] = 0;
				margin[RIGHT] = space;
			} иначе {
				margin[LEFT] = margin[RIGHT] = space / 2;
			}
		} иначе если (margin[LEFT] == AUTO) {
			margin[LEFT] = containing_block->width -
					left - border[LEFT].width -
					padding[LEFT] - width - padding[RIGHT] -
					border[RIGHT].width - margin[RIGHT] -
					right;
		} иначе если (margin[RIGHT] == AUTO) {
			margin[RIGHT] = containing_block->width -
					left - margin[LEFT] -
					border[LEFT].width -
					padding[LEFT] - width - padding[RIGHT] -
					border[RIGHT].width - right;
		} иначе {
			right = containing_block->width -
					left - margin[LEFT] -
					border[LEFT].width -
					padding[LEFT] - width - padding[RIGHT] -
					border[RIGHT].width - margin[RIGHT];
		}
	} иначе {
		если (margin[LEFT] == AUTO)
			margin[LEFT] = 0;
		если (margin[RIGHT] == AUTO)
			margin[RIGHT] = 0;

		если (left == AUTO && width == AUTO && right != AUTO) {
			available_width -= right;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[LEFT] + box->border[LEFT].width +
				box->padding[LEFT] + box->padding[RIGHT] +
				box->border[RIGHT].width + box->margin[RIGHT];

			/* Adjust for {min|max}-width */
			если (max_width >= 0 && width > max_width)
				width = max_width;
			если (width < min_width)
				width = min_width;

			left = containing_block->width -
					margin[LEFT] - border[LEFT].width -
					padding[LEFT] - width - padding[RIGHT] -
					border[RIGHT].width - margin[RIGHT] -
					right;
		} иначе если (left == AUTO && width != AUTO && right == AUTO) {

			/* Adjust for {min|max}-width */
			если (max_width >= 0 && width > max_width)
				width = max_width;
			если (min_width >  0 && width < min_width)
				width = min_width;

			left = static_left;
			right = containing_block->width -
					left - margin[LEFT] -
					border[LEFT].width -
					padding[LEFT] - width - padding[RIGHT] -
					border[RIGHT].width - margin[RIGHT];
		} иначе если (left != AUTO && width == AUTO && right == AUTO) {
			available_width -= left;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[LEFT] + box->border[LEFT].width +
				box->padding[LEFT] + box->padding[RIGHT] +
				box->border[RIGHT].width + box->margin[RIGHT];

			/* Adjust for {min|max}-width */
			если (max_width >= 0 && width > max_width)
				width = max_width;
			если (width < min_width)
				width = min_width;

			right = containing_block->width -
					left - margin[LEFT] -
					border[LEFT].width -
					padding[LEFT] - width - padding[RIGHT] -
					border[RIGHT].width - margin[RIGHT];
		} иначе если (left == AUTO && width != AUTO && right != AUTO) {

			/* Adjust for {min|max}-width */
			если (max_width >= 0 && width > max_width)
				width = max_width;
			если (width < min_width)
				width = min_width;

			left = containing_block->width -
					margin[LEFT] - border[LEFT].width -
					padding[LEFT] - width - padding[RIGHT] -
					border[RIGHT].width - margin[RIGHT] -
					right;
		} иначе если (left != AUTO && width == AUTO && right != AUTO) {
			width = containing_block->width -
					left - margin[LEFT] -
					border[LEFT].width -
					padding[LEFT] - padding[RIGHT] -
					border[RIGHT].width - margin[RIGHT] -
					right;

			/* Adjust for {min|max}-width */
			если (max_width >= 0 && width > max_width)
				width = max_width;
			если (width < min_width)
				width = min_width;

		} иначе если (left != AUTO && width != AUTO && right == AUTO) {

			/* Adjust for {min|max}-width */
			если (max_width >= 0 && width > max_width)
				width = max_width;
			если (width < min_width)
				width = min_width;

			right = containing_block->width -
					left - margin[LEFT] -
					border[LEFT].width -
					padding[LEFT] - width - padding[RIGHT] -
					border[RIGHT].width - margin[RIGHT];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      left, margin[LEFT], border[LEFT].width, padding[LEFT], width,
	      padding[RIGHT], border[RIGHT].width, margin[RIGHT], right,
	      containing_block->width);

	box->x = left + margin[LEFT] + border[LEFT].width - cx;
	если (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_block->width -= containing_block->padding[LEFT] +
				containing_block->padding[RIGHT];
	} иначе {
		/** \todo inline ancestors */
	}
	box->width = width;
	box->height = height;

	если (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->object || box->flags & IFRAME) {
		если (!layout_block_context(box, -1, content))
			вернуть false;
	} иначе если (box->type == BOX_TABLE) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		/* \todo  layout_table considers margins etc. again */
		если (!layout_table(box, width, content))
			вернуть false;
		box->float_container = NULL;
		layout_solve_width(box, box->parent->width, box->width, 0, 0,
				-1, -1);
	} иначе если (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		если (!layout_flex(box, width, content))
			вернуть false;
		box->float_container = NULL;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      top, margin[TOP], border[TOP].width, padding[TOP], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	если (top == AUTO && height == AUTO && bottom == AUTO) {
		top = static_top;
		height = box->height;
		если (margin[TOP] == AUTO)
			margin[TOP] = 0;
		если (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_block->height -
				top - margin[TOP] - border[TOP].width -
				padding[TOP] - height - padding[BOTTOM] -
				border[BOTTOM].width - margin[BOTTOM];
	} иначе если (top != AUTO && height != AUTO && bottom != AUTO) {
		если (margin[TOP] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_block->height -
					top - border[TOP].width - padding[TOP] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - bottom;
			margin[TOP] = margin[BOTTOM] = space / 2;
		} иначе если (margin[TOP] == AUTO) {
			margin[TOP] = containing_block->height -
					top - border[TOP].width - padding[TOP] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} иначе если (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_block->height -
					top - margin[TOP] - border[TOP].width -
					padding[TOP] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					bottom;
		} иначе {
			bottom = containing_block->height -
					top - margin[TOP] - border[TOP].width -
					padding[TOP] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	} иначе {
		если (margin[TOP] == AUTO)
			margin[TOP] = 0;
		если (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		если (top == AUTO && height == AUTO && bottom != AUTO) {
			height = box->height;
			top = containing_block->height -
					margin[TOP] - border[TOP].width -
					padding[TOP] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} иначе если (top == AUTO && height != AUTO && bottom == AUTO) {
			top = static_top;
			bottom = containing_block->height -
					top - margin[TOP] - border[TOP].width -
					padding[TOP] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} иначе если (top != AUTO && height == AUTO && bottom == AUTO) {
			height = box->height;
			bottom = containing_block->height -
					top - margin[TOP] - border[TOP].width -
					padding[TOP] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} иначе если (top == AUTO && height != AUTO && bottom != AUTO) {
			top = containing_block->height -
					margin[TOP] - border[TOP].width -
					padding[TOP] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} иначе если (top != AUTO && height == AUTO && bottom != AUTO) {
			height = containing_block->height -
					top - margin[TOP] - border[TOP].width -
					padding[TOP] - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} иначе если (top != AUTO && height != AUTO && bottom == AUTO) {
			bottom = containing_block->height -
					top - margin[TOP] - border[TOP].width -
					padding[TOP] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      top, margin[TOP], border[TOP].width, padding[TOP], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	box->y = top + margin[TOP] + border[TOP].width - cy;
	если (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_block->height -= containing_block->padding[TOP] +
				containing_block->padding[BOTTOM];
	} иначе {
		/** \todo Inline ancestors */
	}
	box->height = height;
	layout_apply_minmax_height(&content->unit_len_ctx, box, containing_block);

	вернуть true;
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
static bool
layout_position_absolute(struct box *box,
			 struct box *containing_block,
			 целое cx, целое cy,
			 html_content *content)
{
	struct box *c;

	для (c = box->children; c; c = c->next) {
		если ((c->type == BOX_BLOCK || c->type == BOX_TABLE ||
				c->type == BOX_INLINE_BLOCK ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			если (!layout_absolute(c, containing_block,
					cx, cy, content))
				вернуть false;
			если (!layout_position_absolute(c, c, 0, 0, content))
				вернуть false;
		} иначе если (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			если (!layout_position_absolute(c, c, 0, 0, content))
				вернуть false;
		} иначе {
			целое px, py;
			если (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_LEFT ||
					css_computed_float(c->style) ==
						CSS_FLOAT_RIGHT)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				struct box *p;
				px = c->x;
				py = c->y;
				для (p = box->parent; p && !p->float_children;
						p = p->parent) {
					px -= p->x;
					py -= p->y;
				}
			} иначе {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			если (!layout_position_absolute(c, containing_block,
					cx + px, cy + py, content))
				вернуть false;
		}
	}

	вернуть true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
static void layout_compute_relative_offset(
		const css_unit_ctx *unit_len_ctx,
		struct box *box,
		целое *x,
		целое *y)
{
	целое left, right, top, bottom;
	struct box *containing_block;

	assert(box && box->parent && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	если (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_LEFT ||
			css_computed_float(box->style) == CSS_FLOAT_RIGHT)) {
		containing_block = box->float_container;
	} иначе {
		containing_block = box->parent;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_block,
			&top, &right, &bottom, &left);

	если (left == AUTO && right == AUTO)
		left = right = 0;
	иначе если (left == AUTO)
		/* left is auto => computed = -right */
		left = -right;
	иначе если (right == AUTO)
		/* right is auto => computed = -left */
		right = -left;
	иначе {
		/* over constrained => examine direction property
		 * of containing block */
		если (containing_block->style &&
				css_computed_direction(
				containing_block->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			left = -right;
		} иначе {
			/* assume LTR in all other cases */
			right = -left;
		}
	}

	assert(left == -right);

	если (top == AUTO && bottom == AUTO) {
		top = bottom = 0;
	} иначе если (top == AUTO) {
		top = -bottom;
	} иначе {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -top;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", left,
	      right, top, bottom);

	*x = left;
	*y = top;
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
static void
layout_position_relative(
		const css_unit_ctx *unit_len_ctx,
		struct box *root,
		struct box *fp,
		целое fx,
		целое fy)
{
	struct box *box; /* for children of "root" */
	struct box *fn;  /* for block formatting context box for children of
			  * "box" */
	struct box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	целое x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	целое fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	assert(root);

	/* Normal children */
	для (box = root->children; box; box = box->next) {

		если (box->type == BOX_TEXT)
			continue;

		/* If relatively positioned, get offsets */
		если (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_offset(
					unit_len_ctx, box, &x, &y);
		иначе
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		если (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_LEFT ||
				css_computed_float(box->style) ==
					CSS_FLOAT_RIGHT) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			для (fc = fp->float_children; fc; fc = fc->next_float) {
				если (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		если (box->float_children) {
			fn = box;
			fnx = fny = 0;
		} иначе {
			fn = fp;
			fnx = fx + x;
			fny = fy + y;
		}

		/* recurse first */
		layout_position_relative(unit_len_ctx, box, fn, fnx, fny);

		/* Ignore things we're not interested in. */
		если (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			continue;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		если (box->type == BOX_INLINE && box->inline_end) {
			struct box *b;
			для (b = box->next; b && b != box->inline_end;
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
static void
layout_get_box_bbox(
		const css_unit_ctx *unit_len_ctx,
		struct box *box,
		целое *desc_x0, целое *desc_y0,
		целое *desc_x1, целое *desc_y1)
{
	*desc_x0 = -box->border[LEFT].width;
	*desc_y0 = -box->border[TOP].width;
	*desc_x1 = box->padding[LEFT] + box->width + box->padding[RIGHT] +
			box->border[RIGHT].width;
	*desc_y1 = box->padding[TOP] + box->height + box->padding[BOTTOM] +
			box->border[BOTTOM].width;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	если (box->type == BOX_BLOCK && box->style != NULL &&
			css_computed_overflow_y(box->style) ==
					CSS_OVERFLOW_VISIBLE &&
			box->object == NULL) {
		css_fixed font_size = 0;
		css_unit font_unit = CSS_UNIT_PT;
		целое text_height;

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
static void
layout_update_descendant_bbox(
		const css_unit_ctx *unit_len_ctx,
		struct box *box,
		struct box *child,
		целое off_x,
		целое off_y)
{
	целое child_desc_x0, child_desc_y0, child_desc_x1, child_desc_y1;

	/* get coordinates of child relative to box */
	целое child_x = child->x - off_x;
	целое child_y = child->y - off_y;

	bool html_object = (child->object &&
			content_get_type(child->object) == CONTENT_HTML);

	enum css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	enum css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

	если (child->style != NULL) {
		overflow_x = css_computed_overflow_x(child->style);
		overflow_y = css_computed_overflow_y(child->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, child,
			&child_desc_x0, &child_desc_y0,
			&child_desc_x1, &child_desc_y1);

	если (overflow_x == CSS_OVERFLOW_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		child_desc_x0 = child->descendant_x0;
		child_desc_x1 = child->descendant_x1;
	}
	если (overflow_y == CSS_OVERFLOW_VISIBLE &&
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
	если (child_desc_x0 < box->descendant_x0)
		box->descendant_x0 = child_desc_x0;
	если (child_desc_y0 < box->descendant_y0)
		box->descendant_y0 = child_desc_y0;
	если (box->descendant_x1 < child_desc_x1)
		box->descendant_x1 = child_desc_x1;
	если (box->descendant_y1 < child_desc_y1)
		box->descendant_y1 = child_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
static void layout_calculate_descendant_bboxes(
		const css_unit_ctx *unit_len_ctx,
		struct box *box)
{
	struct box *child;

	assert(box->width != UNKNOWN_WIDTH);
	assert(box->height != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	если (box->object && content_get_type(box->object) == CONTENT_HTML) {
		если (box->descendant_x1 < content_get_width(box->object))
			box->descendant_x1 = content_get_width(box->object);
		если (box->descendant_y1 < content_get_height(box->object))
			box->descendant_y1 = content_get_height(box->object);
	}

	если (box->iframe != NULL) {
		целое x, y;
		box_coords(box, &x, &y);

		browser_window_set_position(box->iframe, x, y);
		browser_window_set_dimensions(box->iframe,
				box->width, box->height);
		browser_window_reformat(box->iframe, true,
				box->width, box->height);
	}

	если (box->type == BOX_INLINE || box->type == BOX_TEXT)
		вернуть;

	если (box->type == BOX_INLINE_END) {
		box = box->inline_end;
		для (child = box->next; child;
				child = child->next) {
			если (child->type == BOX_FLOAT_LEFT ||
					child->type == BOX_FLOAT_RIGHT)
				continue;

			layout_update_descendant_bbox(unit_len_ctx, box, child,
					box->x, box->y);

			если (child == box->inline_end)
				break;
		}
		вернуть;
	}

	если (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		вернуть;

	для (child = box->children; child; child = child->next) {
		если (child->type == BOX_FLOAT_LEFT ||
				child->type == BOX_FLOAT_RIGHT)
			continue;

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		если (box->style && css_computed_overflow_x(box->style) ==
				CSS_OVERFLOW_HIDDEN &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_HIDDEN)
			continue;

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	для (child = box->float_children; child; child = child->next_float) {
		assert(child->type == BOX_FLOAT_LEFT ||
				child->type == BOX_FLOAT_RIGHT);

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	если (box->list_marker) {
		child = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, целое width, целое height)
{
	bool ret;
	struct box *doc = content->layout;
	const struct gui_layout_table *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			width, height, nsurl_access(content_get_url(
					&content->base)));

	layout_minmax_block(doc, font_func, content);

	layout_block_find_dimensions(&content->unit_len_ctx,
			width, height, 0, 0, doc);
	doc->x = doc->margin[LEFT] + doc->border[LEFT].width;
	doc->y = doc->margin[TOP] + doc->border[TOP].width;
	width -= doc->margin[LEFT] + doc->border[LEFT].width +
			doc->padding[LEFT] + doc->padding[RIGHT] +
			doc->border[RIGHT].width + doc->margin[RIGHT];
	если (width < 0) {
		width = 0;
	}
	doc->width = width;

	ret = layout_block_context(doc, height, content);

	/* make <html> and <body> fill available height */
	если (doc->y + doc->padding[TOP] + doc->height + doc->padding[BOTTOM] +
			doc->border[BOTTOM].width + doc->margin[BOTTOM] <
			height) {
		doc->height = height - (doc->y + doc->padding[TOP] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].width +
				doc->margin[BOTTOM]);
		если (doc->children)
			doc->children->height = doc->height -
					(doc->children->margin[TOP] +
					 doc->children->border[TOP].width +
					 doc->children->padding[TOP] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].width +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	вернуть ret;
}
