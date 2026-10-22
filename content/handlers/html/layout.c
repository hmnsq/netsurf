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

#ضمن <assert.h>
#ضمن <limits.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <math.h>
#ضمن <dom/dom.h>

#ضمن "utils/log.h"
#ضمن "utils/talloc.h"
#ضمن "utils/utils.h"
#ضمن "utils/nsoption.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/nsurl.h"
#ضمن "netsurf/inttypes.h"
#ضمن "netsurf/content.h"
#ضمن "netsurf/browser_window.h"
#ضمن "netsurf/layout.h"
#ضمن "content/content.h"
#ضمن "content/content_protected.h"
#ضمن "css/utils.h"
#ضمن "desktop/scrollbar.h"
#ضمن "desktop/textarea.h"

#ضمن "html/html.h"
#ضمن "html/html_save.h"
#ضمن "html/private.h"
#ضمن "html/box.h"
#ضمن "html/box_inspect.h"
#ضمن "html/font.h"
#ضمن "html/form_internal.h"
#ضمن "html/layout.h"
#ضمن "html/layout_internal.h"
#ضمن "html/table.h"

/** Array of per-side access functions for computed style margins. */
ثابت css_len_func margin_funcs[4] = {
	[لووتکە_2]    = css_computed_margin_لووتکە,
	[ڕاست_2]  = css_computed_margin_ڕاست,
	[BOTTOM] = css_computed_margin_bottom,
	[چەپ_2]   = css_computed_margin_چەپ,
};

/** Array of per-side access functions for computed style paddings. */
ثابت css_len_func padding_funcs[4] = {
	[لووتکە_2]    = css_computed_padding_لووتکە,
	[ڕاست_2]  = css_computed_padding_ڕاست,
	[BOTTOM] = css_computed_padding_bottom,
	[چەپ_2]   = css_computed_padding_چەپ,
};

/** Array of per-side access functions for computed style border_widths. */
ثابت css_len_func border_width_funcs[4] = {
	[لووتکە_2]    = css_computed_border_لووتکە_width,
	[ڕاست_2]  = css_computed_border_ڕاست_width,
	[BOTTOM] = css_computed_border_bottom_width,
	[چەپ_2]   = css_computed_border_چەپ_width,
};

/** Array of per-side access functions for computed style border styles. */
ثابت css_border_style_func border_style_funcs[4] = {
	[لووتکە_2]    = css_computed_border_لووتکە_style,
	[ڕاست_2]  = css_computed_border_ڕاست_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[چەپ_2]   = css_computed_border_چەپ_style,
};

/** Array of per-side access functions for computed style border colors. */
ثابت css_border_color_func border_color_funcs[4] = {
	[لووتکە_2]    = css_computed_border_لووتکە_color,
	[ڕاست_2]  = css_computed_border_ڕاست_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[چەپ_2]   = css_computed_border_چەپ_color,
};

/* forward declaration to break cycles */
ساكن فراغ layout_minmax_block(
		بنية box *block,
		ثابت بنية gui_layout_table *font_func,
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
ساكن فراغ
layout_get_object_dimensions(بنية box *box,
			     صحيح *width, صحيح *height,
			     صحيح min_width, صحيح max_width,
			     صحيح min_height, صحيح max_height)
{
	أكد(box->object != عدم);
	أكد(width != عدم && height != عدم);

	إذا (*width == AUTO && *height == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		صحيح intrinsic_width = content_get_width(box->object);
		صحيح intrinsic_height = content_get_height(box->object);

		/* use intrinsic dimensions */
		*width = intrinsic_width;
		*height = intrinsic_height;

		/* Deal with min/max-width first */
		إذا (min_width >  0 && min_width > *width) {
			*width = min_width;
			scaled = true;
		}
		إذا (max_width >= 0 && max_width < *width) {
			*width = max_width;
			scaled = true;
		}

		إذا (scaled && (intrinsic_width != 0)) {
			/* Update height */
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		}

		scaled = false;
		/* Deal with min/max-height */
		إذا (min_height >  0 && min_height > *height) {
			*height = min_height;
			scaled = true;
		}
		إذا (max_height >= 0 && max_height < *height) {
			*height = max_height;
			scaled = true;
		}

		إذا (scaled && (intrinsic_height != 0)) {
			/* Update width */
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		}

	} وإلا إذا (*width == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		صحيح intrinsic_width = content_get_width(box->object);
		صحيح intrinsic_height = content_get_height(box->object);

		إذا (intrinsic_height != 0)
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		وإلا
			*width = intrinsic_width;

		إذا (min_width >  0 && min_width > *width)
			*width = min_width;
		إذا (max_width >= 0 && max_width < *width)
			*width = max_width;

	} وإلا إذا (*height == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		صحيح intrinsic_width = content_get_width(box->object);
		صحيح intrinsic_height = content_get_height(box->object);

		إذا (min_width >  0 && min_width > *width)
			*width = min_width;
		إذا (max_width >= 0 && max_width < *width)
			*width = max_width;

		إذا (intrinsic_width != 0)
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		وإلا
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
ساكن صحيح layout_text_indent(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style, صحيح width)
{
	css_fixed بەها = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_text_indent(style, &بەها, &unit);

	إذا (unit == CSS_UNIT_PCT) {
		أرجع FPCT_OF_INT_TOINT(بەها, width);
	} وإلا {
		أرجع FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				بەها, unit));
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
ساكن فراغ layout_minmax_table(بنية box *table,
		ثابت بنية gui_layout_table *font_func,
		ثابت html_content *content)
{
	unsigned صحيح i, j;
	صحيح width;
	صحيح border_spacing_h = 0;
	صحيح table_min = 0, table_max = 0;
	صحيح extra_fixed = 0;
	عائم extra_frac = 0;
	بنية column *col;
	بنية box *row_group, *row, *cell;

	/* check if the widths have already been calculated */
	إذا (table->max_width != UNKNOWN_MAX_WIDTH)
		أرجع;

	إذا (table_calculate_column_types(&content->unit_len_ctx, table) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		أرجع;
	}
	col = table->col;

	/* start with 0 except for fixed-width columns */
	لكل (i = 0; i != table->columns; i++) {
		إذا (col[i].type == COLUMN_WIDTH_FIXED)
			col[i].min = col[i].max = col[i].width;
		وإلا
			col[i].min = col[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	إذا (css_computed_border_collapse(table->style) ==
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
	لكل (row_group = table->children; row_group; row_group =row_group->next)
	لكل (row = row_group->children; row; row = row->next)
	لكل (cell = row->children; cell; cell = cell->next) {
		أكد(cell->type == BOX_TABLE_CELL);
		أكد(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		أكد(cell->columns != 0);

		إذا (cell->columns != 1)
			استمر;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		إذا (col[i].positioned)
			استمر;

		/* update column min, max widths using cell widths */
		إذا (col[i].min < cell->min_width)
			col[i].min = cell->min_width;
		إذا (col[i].max < cell->max_width)
			col[i].max = cell->max_width;
	}

	/* 2nd pass: cells which span multiple columns */
	لكل (row_group = table->children; row_group; row_group =row_group->next)
	لكل (row = row_group->children; row; row = row->next)
	لكل (cell = row->children; cell; cell = cell->next) {
		unsigned صحيح flexible_columns = 0;
		صحيح min = 0, max = 0, fixed_width = 0, extra;

		إذا (cell->columns == 1)
			استمر;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		لكل (j = 0; j != cell->columns; j++) {
			min += col[i + j].min;
			إذا (col[i + j].type == COLUMN_WIDTH_FIXED)
				fixed_width += col[i + j].width;
			وإلا
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		إذا (min < cell->min_width) {
			إذا (flexible_columns == 0) {
				extra = 1 + (cell->min_width - min) /
						cell->columns;
				لكل (j = 0; j != cell->columns; j++) {
					col[i + j].min += extra;
					إذا (col[i + j].max < col[i + j].min)
						col[i + j].max = col[i + j].min;
				}
			} وإلا {
				extra = 1 + (cell->min_width - min) /
						flexible_columns;
				لكل (j = 0; j != cell->columns; j++) {
					إذا (col[i + j].type !=
							COLUMN_WIDTH_FIXED) {
						col[i + j].min += extra;
						إذا (col[i + j].max <
								col[i + j].min)
							col[i + j].max =
								col[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		لكل (j = 0; j != cell->columns; j++)
			max += col[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		إذا (max < cell->max_width && flexible_columns) {
			extra = 1 + (cell->max_width - max) / flexible_columns;
			لكل (j = 0; j != cell->columns; j++)
				إذا (col[i + j].type != COLUMN_WIDTH_FIXED)
					col[i + j].max += extra;
		}
	}

	لكل (i = 0; i != table->columns; i++) {
		إذا (col[i].max < col[i].min) {
			box_dump(stderr, table, 0, true);
			أكد(0);
		}
		table_min += col[i].min;
		table_max += col[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	إذا (css_computed_width_px(table->style, &content->unit_len_ctx,
			-1, &width) == CSS_WIDTH_SET) {
		إذا (table_min < width)
			table_min = width;
		إذا (table_max < width)
			table_max = width;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, چەپ_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, ڕاست_2, true, true, true,
			&extra_fixed, &extra_frac);
	إذا (extra_fixed < 0)
		extra_fixed = 0;
	إذا (extra_frac < 0)
		extra_frac = 0;
	إذا (1.0 <= extra_frac)
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
ساكن مضمن bool box_has_percentage_max_width(بنية box *b)
{
	css_unit unit = CSS_UNIT_PX;
	تعداد css_max_width_e type;
	css_fixed بەها = 0;

	أكد(b != عدم);

	type = css_computed_max_width(b->style, &بەها, &unit);
	أرجع ((type == CSS_MAX_WIDTH_SET) && (unit == CSS_UNIT_PCT));
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
ساكن بنية box *
layout_minmax_هێڵ(بنية box *first,
		   صحيح *هێڵ_min,
		   صحيح *هێڵ_max,
		   bool first_هێڵ,
		   bool *هێڵ_has_height,
		   ثابت بنية gui_layout_table *font_func,
		   ثابت html_content *content)
{
	صحيح min = 0, max = 0, width, height, fixed;
	عائم frac;
	size_t i, j;
	بنية box *b;
	بنية box *block;
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

	*هێڵ_has_height = false;

	/* corresponds to the pass 1 loop in layout_line() */
	لكل (b = first; b; b = b->next) {
		تعداد css_width_e wtype;
		تعداد css_height_e htype;
		تعداد css_box_sizing_e bs;
		css_fixed بەها = 0;
		css_unit unit = CSS_UNIT_PX;

		أكد(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		إذا (b->type == BOX_BR) {
			b = b->next;
			اكسر;
		}

		إذا (lh__box_is_float_box(b)) {
			أكد(b->children);
			إذا (b->children->type == BOX_TABLE)
				layout_minmax_table(b->children, font_func,
						content);
			وإلا
				layout_minmax_block(b->children, font_func,
						content);
			b->min_width = b->children->min_width;
			b->max_width = b->children->max_width;
			إذا (min < b->min_width)
				min = b->min_width;
			max += b->max_width;
			استمر;
		}

		إذا (b->type == BOX_INLINE_BLOCK || b->type == BOX_INLINE_FLEX) {
			layout_minmax_block(b, font_func, content);
			إذا (min < b->min_width)
				min = b->min_width;
			max += b->max_width;

			إذا (b->flags & HAS_HEIGHT)
				*هێڵ_has_height = true;
			استمر;
		}

		أكد(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		إذا (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->style, چەپ_2, true, true, true,
					&fixed, &frac);
			إذا (!b->inline_end)
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ڕاست_2,
						true, true, true,
						&fixed, &frac);
			إذا (0 < fixed)
				max += fixed;
			*هێڵ_has_height = true;
			/* \todo  update min width, consider fractional extra */
		} وإلا إذا (b->type == BOX_INLINE_END) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->inline_end->style, ڕاست_2,
					true, true, true,
					&fixed, &frac);
			إذا (0 < fixed)
				max += fixed;

			إذا (b->next) {
				إذا (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*هێڵ_has_height = true;
			استمر;
		}

		إذا (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			إذا (!b->text)
				استمر;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			إذا (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				إذا (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					صحيح opt_maxwidth = 0;
					بنية form_option *o;

					لكل (o = b->parent->parent->gadget->
							data.select.کاڵاکان; o;
							o = o->next) {
						صحيح opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						إذا (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}

					b->width = opt_maxwidth;
					إذا (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;

				} وإلا {
					font_func->width(&fstyle, b->text,
						b->length, &b->width);
					b->flags |= MEASURED;
				}
			}
			max += b->width;
			إذا (b->next) {
				إذا (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			إذا (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} وإلا إذا (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				إذا (min < b->width)
					min = b->width;

			} وإلا إذا (b->parent->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				افعل {
					لكل (j = i; j != b->length &&
							b->text[j] != ' '; j++)
						;
					font_func->width(&fstyle, b->text + i,
							 j - i, &width);
					إذا (min < width)
						min = width;
					i = j + 1;
				} طالما (j != b->length);
			}

			*هێڵ_has_height = true;

			استمر;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		bs = css_computed_box_sizing(block->style);

		/* calculate box width */
		wtype = css_computed_width_px(b->style,
				&content->unit_len_ctx, -1, &width);
		إذا (wtype == CSS_WIDTH_SET) {
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, چەپ_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, ڕاست_2,
						false, true, true,
						&fixed, &frac);
				إذا (width < fixed) {
					width = fixed;
				}
			}
			إذا (width < 0)
				width = 0;
		} وإلا {
			width = AUTO;
		}

		/* height */
		htype = css_computed_height(b->style, &بەها, &unit);
		إذا (htype == CSS_HEIGHT_SET) {
			height = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					بەها, unit));
		} وإلا {
			height = AUTO;
		}

		إذا (b->object || (b->flags & REPLACE_DIM)) {
			إذا (b->object) {
				صحيح temp_height = height;
				layout_get_object_dimensions(b,
						&width, &temp_height,
						INT_MIN, INT_MAX,
						INT_MIN, INT_MAX);
			}

			fixed = frac = 0;
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, چەپ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ڕاست_2,
						true, false, false,
						&fixed, &frac);
			} وإلا {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, چەپ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ڕاست_2,
						true, true, true,
						&fixed, &frac);
			}
			إذا (0 < width + fixed)
				width += fixed;
		} وإلا إذا (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			إذا (width == AUTO)
				width = 400;

			fixed = frac = 0;
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, چەپ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ڕاست_2,
						true, false, false,
						&fixed, &frac);
			} وإلا {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, چەپ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ڕاست_2,
						true, true, true,
						&fixed, &frac);
			}

			إذا (0 < width + fixed)
				width += fixed;

		} وإلا {
			/* form control with no object */
			إذا (width == AUTO)
				width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		إذا (min < width && !box_has_percentage_max_width(b))
			min = width;
		إذا (width > 0)
			max += width;

		*هێڵ_has_height = true;
	}

	إذا (first_هێڵ) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		صحيح text_indent = layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, 100);
		min = (min + text_indent < 0) ? 0 : min + text_indent;
		max = (max + text_indent < 0) ? 0 : max + text_indent;
	}

	*هێڵ_min = min;
	*هێڵ_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	أكد(b != first);
	أكد(0 <= *هێڵ_min);
	أكد(*هێڵ_min <= *هێڵ_max);
	أرجع b;
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
ساكن فراغ
layout_minmax_inline_container(بنية box *inline_container,
			       bool *has_height,
			       ثابت بنية gui_layout_table *font_func,
			       ثابت html_content *content)
{
	بنية box *child;
	صحيح هێڵ_min = 0, هێڵ_max = 0;
	صحيح min = 0, max = 0;
	bool first_هێڵ = true;
	bool هێڵ_has_height;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	إذا (inline_container->max_width != UNKNOWN_MAX_WIDTH)
		أرجع;

	*has_height = false;

	لكل (child = inline_container->children; child; ) {
		child = layout_minmax_هێڵ(child, &هێڵ_min, &هێڵ_max,
				first_هێڵ, &هێڵ_has_height, font_func,
				content);
		إذا (min < هێڵ_min)
			min = هێڵ_min;
		إذا (max < هێڵ_max)
			max = هێڵ_max;
		first_هێڵ = false;
		*has_height |= هێڵ_has_height;
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
ساكن فراغ layout_minmax_block(
		بنية box *block,
		ثابت بنية gui_layout_table *font_func,
		ثابت html_content *content)
{
	بنية box *child;
	صحيح min = 0, max = 0;
	صحيح extra_fixed = 0;
	عائم extra_frac = 0;
	تعداد css_width_e wtype = CSS_WIDTH_AUTO;
	css_fixed width = 0;
	css_unit wunit = CSS_UNIT_PX;
	تعداد css_height_e htype = CSS_HEIGHT_AUTO;
	css_fixed height = 0;
	css_unit hunit = CSS_UNIT_PX;
	تعداد css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool child_has_height = false;

	أكد(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	/* check if the widths have already been calculated */
	إذا (block->max_width != UNKNOWN_MAX_WIDTH)
		أرجع;

	إذا (block->style != عدم) {
		wtype = css_computed_width(block->style, &width, &wunit);
		htype = css_computed_height(block->style, &height, &hunit);
		bs = css_computed_box_sizing(block->style);
	}

	/* set whether the minimum width is of any interest for this box */
	إذا (((block->parent && lh__box_is_float_box(block->parent)) ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX) &&
			wtype != CSS_WIDTH_SET) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} وإلا إذا (block->type == BOX_TABLE_CELL) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} وإلا إذا (block->parent && (block->parent->flags & NEED_MIN) &&
			wtype != CSS_WIDTH_SET) {
		/* box inside shrink-to-fit context; need minimum width */
		block->flags |= NEED_MIN;
	} وإلا إذا (block->parent && (block->parent->type == BOX_FLEX)) {
		/* box is flex item */
		block->flags |= NEED_MIN;
	}

	إذا (block->gadget && (block->gadget->type == GADGET_TEXTBOX ||
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

	إذا (block->gadget && (block->gadget->type == GADGET_RADIO ||
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

	إذا (block->object) {
		إذا (content_get_type(block->object) == CONTENT_HTML) {
			layout_minmax_block(html_get_box_درەخت(block->object),
					font_func, content);
			min = html_get_box_درەخت(block->object)->min_width;
			max = html_get_box_درەخت(block->object)->max_width;
		} وإلا {
			min = max = content_get_width(block->object);
		}

		block->flags |= HAS_HEIGHT;
	} وإلا إذا (block->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		block->flags |= HAS_HEIGHT;
	} وإلا {
		/* recurse through children */
		لكل (child = block->children; child; child = child->next) {
			بدل (child->type) {
			حالة BOX_FLEX:
			حالة BOX_BLOCK:
				layout_minmax_block(child, font_func,
						content);
				إذا (child->flags & HAS_HEIGHT)
					child_has_height = true;
				اكسر;
			حالة BOX_INLINE_CONTAINER:
				إذا (block->flags & NEED_MIN)
					child->flags |= NEED_MIN;

				layout_minmax_inline_container(child,
						&child_has_height, font_func,
						content);
				إذا (child_has_height &&
						child ==
						child->parent->children) {
					block->flags |= MAKE_HEIGHT;
				}
				اكسر;
			حالة BOX_TABLE:
				layout_minmax_table(child, font_func,
						content);
				/* todo: fix for zero height tables */
				child_has_height = true;
				child->flags |= MAKE_HEIGHT;
				اكسر;
			افتراضي:
				أكد(0);
			}
			أكد(child->max_width != UNKNOWN_MAX_WIDTH);

			إذا (child->style &&
					(css_computed_position(child->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(child->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				استمر;
			}

			إذا (lh__box_is_flex_container(block) &&
			    lh__flex_main_is_horizontal(block)) {
				إذا (block->style != عدم &&
				    css_computed_flex_wrap(block->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += child->min_width;
				} وإلا {
					إذا (min < child->min_width)
						min = child->min_width;
				}
				max += child->max_width;

			} وإلا {
				إذا (min < child->min_width)
					min = child->min_width;
				إذا (max < child->max_width)
					max = child->max_width;
			}

			إذا (child_has_height)
				block->flags |= HAS_HEIGHT;
		}
	}

	إذا (max < min) {
		box_dump(stderr, block, 0, true);
		أكد(0);
	}

	/* fixed width takes priority */
	إذا (block->type != BOX_TABLE_CELL && !lh__box_is_flex_item(block)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		تعداد css_max_width_e max_type;
		تعداد css_min_width_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed بەها = 0;
		صحيح width;

		إذا (css_computed_width_px(block->style, &content->unit_len_ctx,
				-1, &width) == CSS_WIDTH_SET) {
			min = max = width;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_width(block->style, &بەها, &unit);
		إذا (min_type == CSS_MIN_WIDTH_SET && unit != CSS_UNIT_PCT) {
			صحيح val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, بەها, unit));

			إذا (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_width(block->style, &بەها, &unit);
		إذا (max_type == CSS_MAX_WIDTH_SET && unit != CSS_UNIT_PCT) {
			صحيح val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, بەها, unit));

			إذا (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	إذا (htype == CSS_HEIGHT_SET && hunit != CSS_UNIT_PCT &&
			height > INTTOFIX(0)) {
		block->flags |= MAKE_HEIGHT;
		block->flags |= HAS_HEIGHT;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_width(&content->unit_len_ctx, block->style, چەپ_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, ڕاست_2,
			false, true, true, &extra_fixed, &extra_frac);

	إذا (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	إذا (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	إذا (max < min) {
		min = max;
	}

	calculate_mbp_width(&content->unit_len_ctx, block->style, چەپ_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, ڕاست_2,
			true, false, false, &extra_fixed, &extra_frac);

	إذا (extra_fixed < 0)
		extra_fixed = 0;
	إذا (extra_frac < 0)
		extra_frac = 0;
	إذا (1.0 <= extra_frac)
		extra_frac = 0.9;
	إذا (block->style != عدم &&
			(css_computed_float(block->style) == CSS_FLOAT_چەپ ||
			css_computed_float(block->style) == CSS_FLOAT_ڕاست)) {
		/* floated boxs */
		block->min_width = min + extra_fixed;
		block->max_width = max + extra_fixed;
	} وإلا {
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
ساكن بنية box*
layout_next_margin_block(ثابت css_unit_ctx *unit_len_ctx,
			 بنية box *box,
			 بنية box *block,
			 صحيح viewport_height,
			 صحيح *max_pos_margin,
			 صحيح *max_neg_margin)
{
	أكد(block != عدم);

	طالما (box != عدم) {

		إذا (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			إذا (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						عدم, عدم, عدم, عدم,
						عدم, عدم, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				إذا (*max_pos_margin < box->margin[لووتکە_2])
					*max_pos_margin = box->margin[لووتکە_2];
				وإلا إذا (*max_neg_margin < -box->margin[لووتکە_2])
					*max_neg_margin = -box->margin[لووتکە_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			إذا (box->flags & MAKE_HEIGHT ||
					box->border[لووتکە_2].width ||
					box->padding[لووتکە_2] ||
					(box->style &&
					css_computed_overflow_y(box->style) !=
					CSS_OVERFLOW_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_first_child(box))) {
				/* Collapse to this box; return it */
				أرجع box;
			}
		}


		/* Find next box */
		إذا (box->type == BOX_BLOCK && !box->object && box->children &&
				box->style &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} وإلا {
			إذا (!box->next) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				افعل {
					/* Apply bottom margin */
					إذا (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					وإلا إذا (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->parent;
				} طالما (box != block && !box->next);

				إذا (box == block) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					أرجع block;
				}
			}

			/* Apply bottom margin */
			إذا (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			وإلا إذا (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->next;

			/* Get margins */
			إذا (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						عدم, عدم, عدم, عدم,
						عدم, عدم, box->margin,
						box->padding, box->border);
			}
		}
	}

	أرجع عدم;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
ساكن صحيح layout_clear(بنية box *fl, تعداد css_clear_e clear)
{
	صحيح y = 0;
	لكل (; fl; fl = fl->next_float) {
		إذا ((clear == CSS_CLEAR_چەپ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_چەپ)
			إذا (y < fl->y + fl->height)
				y = fl->y + fl->height;
		إذا ((clear == CSS_CLEAR_ڕاست || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ڕاست)
			إذا (y < fl->y + fl->height)
				y = fl->y + fl->height;
	}
	أرجع y;
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
ساكن فراغ
find_sides(بنية box *fl,
	   صحيح y0, صحيح y1,
	   صحيح *x0, صحيح *x1,
	   بنية box **چەپ,
	   بنية box **ڕاست)
{
	صحيح fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*چەپ = *ڕاست = 0;
	لكل (; fl; fl = fl->next_float) {
		fy1 = fl->y + fl->height;
		إذا (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			أرجع;
		}
		fy0 = fl->y;
		إذا (y0 < fy1 && fy0 <= y1) {
			إذا (fl->type == BOX_FLOAT_چەپ) {
				fx1 = fl->x + fl->width;
				إذا (*x0 < fx1) {
					*x0 = fx1;
					*چەپ = fl;
				}
			} وإلا {
				fx0 = fl->x;
				إذا (fx0 < *x1) {
					*x1 = fx0;
					*ڕاست = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *چەپ, *ڕاست);
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
ساكن صحيح
layout_solve_width(بنية box *box,
		   صحيح available_width,
		   صحيح width,
		   صحيح lm,
		   صحيح rm,
		   صحيح max_width,
		   صحيح min_width)
{
	bool auto_width = false;

	/* Increase specified left/right margins */
	إذا (box->margin[چەپ_2] != AUTO && box->margin[چەپ_2] < lm &&
			box->margin[چەپ_2] >= 0)
		box->margin[چەپ_2] = lm;
	إذا (box->margin[ڕاست_2] != AUTO && box->margin[ڕاست_2] < rm &&
			box->margin[ڕاست_2] >= 0)
		box->margin[ڕاست_2] = rm;

	/* Find width */
	إذا (width == AUTO) {
		صحيح margin_چەپ = box->margin[چەپ_2];
		صحيح margin_ڕاست = box->margin[ڕاست_2];

		إذا (margin_چەپ == AUTO) {
			margin_چەپ = lm;
		}
		إذا (margin_ڕاست == AUTO) {
			margin_ڕاست = rm;
		}

		width = available_width -
				(margin_چەپ + box->border[چەپ_2].width +
				box->padding[چەپ_2] + box->padding[ڕاست_2] +
				box->border[ڕاست_2].width + margin_ڕاست);
		width = width < 0 ? 0 : width;
		auto_width = true;
	}

	إذا (max_width >= 0 && width > max_width) {
		/* max-width is admissable and width exceeds max-width */
		width = max_width;
		auto_width = false;
	}

	إذا (min_width > 0 && width < min_width) {
		/* min-width is admissable and width is less than max-width */
		width = min_width;
		auto_width = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	إذا (auto_width) {
		/* any other 'auto' become 0 or the minimum required values */
		إذا (box->margin[چەپ_2] == AUTO) {
			box->margin[چەپ_2] = lm;
		}
		إذا (box->margin[ڕاست_2] == AUTO) {
			box->margin[ڕاست_2] = rm;
		}
		أرجع width;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	إذا (box->margin[چەپ_2] != AUTO && box->margin[ڕاست_2] != AUTO &&
			box->parent != عدم && box->parent->style != عدم) {
		بدل (css_computed_text_align(box->parent->style)) {
		حالة CSS_TEXT_ALIGN_LIBCSS_ڕاست:
			box->margin[چەپ_2] = AUTO;
			box->margin[ڕاست_2] = 0;
			اكسر;
		حالة CSS_TEXT_ALIGN_LIBCSS_CENTER:
			box->margin[چەپ_2] = box->margin[ڕاست_2] = AUTO;
			اكسر;
		حالة CSS_TEXT_ALIGN_LIBCSS_چەپ:
			box->margin[چەپ_2] = 0;
			box->margin[ڕاست_2] = AUTO;
			اكسر;
		افتراضي:
			/* Leave it alone; no HTML alignment */
			اكسر;
		}
	}

	إذا (box->margin[چەپ_2] == AUTO && box->margin[ڕاست_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[چەپ_2] = box->margin[ڕاست_2] =
				(available_width - lm - rm -
				(box->border[چەپ_2].width + box->padding[چەپ_2] +
				width + box->padding[ڕاست_2] +
				box->border[ڕاست_2].width)) / 2;

		إذا (box->margin[چەپ_2] < 0) {
			box->margin[ڕاست_2] += box->margin[چەپ_2];
			box->margin[چەپ_2] = 0;
		}

		box->margin[چەپ_2] += lm;

	} وإلا إذا (box->margin[چەپ_2] == AUTO) {
		box->margin[چەپ_2] = available_width - lm -
				(box->border[چەپ_2].width + box->padding[چەپ_2] +
				width + box->padding[ڕاست_2] +
				box->border[ڕاست_2].width + box->margin[ڕاست_2]);
		box->margin[چەپ_2] = box->margin[چەپ_2] < lm
				? lm : box->margin[چەپ_2];
	} وإلا {
		/* margin-right auto or "over-constrained" */
		box->margin[ڕاست_2] = available_width - rm -
				(box->margin[چەپ_2] + box->border[چەپ_2].width +
				 box->padding[چەپ_2] + width +
				 box->padding[ڕاست_2] +
				 box->border[ڕاست_2].width);
	}

	أرجع width;
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
ساكن فراغ
layout_block_find_dimensions(ثابت css_unit_ctx *unit_len_ctx,
			     صحيح available_width,
			     صحيح viewport_height,
			     صحيح lm,
			     صحيح rm,
			     بنية box *box)
{
	صحيح width, max_width, min_width;
	صحيح height, max_height, min_height;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	ثابت css_computed_style *style = box->style;

	layout_find_dimensions(unit_len_ctx, available_width, viewport_height, box,
			style, &width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	إذا (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	}

	box->width = layout_solve_width(box, available_width, width, lm, rm,
			max_width, min_width);
	box->height = height;

	إذا (margin[لووتکە_2] == AUTO)
		margin[لووتکە_2] = 0;
	إذا (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
ساكن فراغ layout_block_add_scrollbar(بنية box *box, صحيح which)
{
	تعداد css_overflow_e overflow_x, overflow_y;

	أكد(box->type == BOX_BLOCK && (which == ڕاست_2 || which == BOTTOM));

	إذا (box->style == عدم)
		أرجع;

	overflow_x = css_computed_overflow_x(box->style);
	overflow_y = css_computed_overflow_y(box->style);

	إذا (which == BOTTOM &&
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		إذا (box->height != AUTO &&
				(overflow_x == CSS_OVERFLOW_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_WIDTH;
		}

	} وإلا إذا (which == ڕاست_2 &&
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		تعداد css_height_e htype;
		css_fixed height = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_height(box->style, &height, &hunit);

		إذا (which == ڕاست_2 && box->width != AUTO &&
				htype == CSS_HEIGHT_SET &&
				(overflow_y == CSS_OVERFLOW_SCROLL ||
				box_vscrollbar_present(box))) {
			box->width -= SCROLLBAR_WIDTH;
			box->padding[ڕاست_2] += SCROLLBAR_WIDTH;
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
ساكن فراغ layout_move_children(بنية box *box, صحيح x, صحيح y)
{
	أكد(box);

	لكل (box = box->children; box; box = box->next) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_table(
		بنية box *table,
		صحيح available_width,
		html_content *content)
{
	unsigned صحيح columns = table->columns;  /* total columns */
	unsigned صحيح i;
	unsigned صحيح *row_span;
	صحيح *excess_y;
	صحيح table_width, min_width = 0, max_width = 0;
	صحيح required_width = 0;
	صحيح x, remainder = 0, ژماردن = 0;
	صحيح table_height = 0;
	صحيح min_height = 0;
	صحيح *xs;  /* array of column x positions */
	صحيح auto_width;
	صحيح spare_width;
	صحيح relative_sum = 0;
	صحيح border_spacing_h = 0, border_spacing_v = 0;
	صحيح spare_height;
	صحيح positioned_columns = 0;
	بنية box *containing_block = عدم;
	بنية box *c;
	بنية box *row;
	بنية box *row_group;
	بنية box **row_span_cell;
	بنية column *col;
	ثابت css_computed_style *style = table->style;
	تعداد css_height_e htype;
	css_fixed بەها = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(table->type == BOX_TABLE);
	أكد(style);
	أكد(table->children && table->children->children);
	أكد(columns);

	/* allocate working buffers */
	col = malloc(columns * حجم col[0]);
	excess_y = malloc(columns * حجم excess_y[0]);
	row_span = malloc(columns * حجم row_span[0]);
	row_span_cell = malloc(columns * حجم row_span_cell[0]);
	xs = malloc((columns + 1) * حجم xs[0]);
	إذا (!col || !xs || !row_span || !excess_y || !row_span_cell) {
		حرر(col);
		حرر(excess_y);
		حرر(row_span);
		حرر(row_span_cell);
		حرر(xs);
		أرجع false;
	}

	memcpy(col, table->col, حجم(col[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1, table,
			style, 0, 0, 0, 0, 0, 0, table->margin, table->padding,
			table->border);
	لكل (row_group = table->children; row_group;
			row_group = row_group->next) {
		لكل (row = row_group->children; row; row = row->next) {
			لكل (c = row->children; c; c = c->next) {
				تعداد css_overflow_e overflow_x;
				تعداد css_overflow_e overflow_y;

				أكد(c->style);
				table_used_border_for_cell(
						&content->unit_len_ctx, c);
				layout_find_dimensions(&content->unit_len_ctx,
						available_width, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				overflow_x = css_computed_overflow_x(c->style);
				overflow_y = css_computed_overflow_y(c->style);

				إذا (overflow_x == CSS_OVERFLOW_SCROLL ||
						overflow_x ==
						CSS_OVERFLOW_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_WIDTH;
				}
				إذا (overflow_y == CSS_OVERFLOW_SCROLL ||
						overflow_y ==
						CSS_OVERFLOW_AUTO) {
					c->padding[ڕاست_2] += SCROLLBAR_WIDTH;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	إذا (css_computed_border_collapse(style) ==
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
	إذا (css_computed_width_px(style, &content->unit_len_ctx,
			available_width, &table_width) == CSS_WIDTH_SET) {
		/* specified width includes border */
		table_width -= table->border[چەپ_2].width +
				table->border[ڕاست_2].width;
		table_width = table_width < 0 ? 0 : table_width;

		auto_width = table_width;
	} وإلا {
		table_width = AUTO;
		auto_width = available_width -
				((table->margin[چەپ_2] == AUTO ? 0 :
						table->margin[چەپ_2]) +
				 table->border[چەپ_2].width +
				 table->padding[چەپ_2] +
				 table->padding[ڕاست_2] +
				 table->border[ڕاست_2].width +
				 (table->margin[ڕاست_2] == AUTO ? 0 :
						table->margin[ڕاست_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_height(style, &بەها, &unit);
	إذا (htype == CSS_HEIGHT_SET) {
		إذا (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			إذا (css_computed_position(table->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				أكد(table->float_container);
				containing_block = table->float_container;
			} وإلا إذا (table->float_container &&
					css_computed_position(table->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(table->style) ==
						CSS_FLOAT_چەپ ||
					css_computed_float(table->style) ==
						CSS_FLOAT_ڕاست)) {
				/* Table is a float */
				أكد(table->parent && table->parent->parent &&
						table->parent->parent->parent);
				containing_block =
						table->parent->parent->parent;
			} وإلا إذا (table->parent && table->parent->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_block = table->parent;
			} وإلا إذا (table->parent && table->parent->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				أكد(table->parent->parent);
				containing_block = table->parent->parent;
			}

			إذا (containing_block) {
				css_fixed ignored = 0;

				htype = css_computed_height(
						containing_block->style,
						&ignored, &unit);
			}

			إذا (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(table->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_HEIGHT_SET)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_height = FPCT_OF_INT_TOINT(بەها,
						containing_block->height);
			}
		} وإلا {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_height = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					بەها, unit));
		}
	}

	/* calculate width required by cells */
	لكل (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      table,
		      i,
		      ((ثابت حرف *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[col[i].type],
		      col[i].width,
		      col[i].min,
		      col[i].max);


		إذا (col[i].positioned) {
			positioned_columns++;
			استمر;
		} وإلا إذا (col[i].type == COLUMN_WIDTH_FIXED) {
			إذا (col[i].width < col[i].min)
				col[i].width = col[i].max = col[i].min;
			وإلا
				col[i].min = col[i].max = col[i].width;
			required_width += col[i].width;
		} وإلا إذا (col[i].type == COLUMN_WIDTH_PERCENT) {
			صحيح width = col[i].width * auto_width / 100;
			required_width += col[i].min < width ? width :
					col[i].min;
		} وإلا
			required_width += col[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_width);
	}
	required_width += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", table_width,
	      table->min_width, table->max_width, auto_width, required_width);

	إذا (auto_width < required_width) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		لكل (i = 0; i != columns; i++) {
			إذا (col[i].type == COLUMN_WIDTH_RELATIVE)
				استمر;
			إذا (col[i].type == COLUMN_WIDTH_PERCENT) {
				col[i].max = auto_width * col[i].width / 100;
				إذا (col[i].max < col[i].min)
					col[i].max = col[i].min;
			}
			min_width += col[i].min;
			max_width += col[i].max;
		}
	} وإلا {
		/* take percentages exactly */
		لكل (i = 0; i != columns; i++) {
			إذا (col[i].type == COLUMN_WIDTH_RELATIVE)
				استمر;
			إذا (col[i].type == COLUMN_WIDTH_PERCENT) {
				صحيح width = auto_width * col[i].width / 100;
				إذا (width < col[i].min)
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
	لكل (i = 0; i != columns; i++) {
		إذا (col[i].type == COLUMN_WIDTH_RELATIVE)
			relative_sum += col[i].width;
		وإلا إذا (col[i].type == COLUMN_WIDTH_FIXED)
			spare_width -= col[i].width;
		وإلا
			spare_width -= col[i].min;
	}
	spare_width -= (columns + 1) * border_spacing_h;
	إذا (relative_sum != 0) {
		إذا (spare_width < 0)
			spare_width = 0;
		لكل (i = 0; i != columns; i++) {
			إذا (col[i].type == COLUMN_WIDTH_RELATIVE) {
				col[i].min = ceil(col[i].max =
						(عائم) spare_width
						* (عائم) col[i].width
						/ relative_sum);
				min_width += col[i].min;
				max_width += col[i].max;
			}
		}
	}
	min_width += (columns + 1) * border_spacing_h;
	max_width += (columns + 1) * border_spacing_h;

	إذا (auto_width <= min_width) {
		/* not enough space: minimise column widths */
		لكل (i = 0; i < columns; i++) {
			col[i].width = col[i].min;
		}
		table_width = min_width;
	} وإلا إذا (max_width <= auto_width) {
		/* more space than maximum width */
		إذا (table_width == AUTO) {
			/* for auto-width tables, make columns max width */
			لكل (i = 0; i < columns; i++) {
				col[i].width = col[i].max;
			}
			table_width = max_width;
		} وإلا {
			/* for fixed-width tables, distribute the extra space
			 * too */
			unsigned صحيح flexible_columns = 0;
			لكل (i = 0; i != columns; i++)
				إذا (col[i].type != COLUMN_WIDTH_FIXED)
					flexible_columns++;
			إذا (flexible_columns == 0) {
				صحيح extra = (table_width - max_width) / columns;
				remainder = (table_width - max_width) -
						(extra * columns);
				لكل (i = 0; i != columns; i++) {
					col[i].width = col[i].max + extra;
					ژماردن -= remainder;
					إذا (ژماردن < 0) {
						col[i].width++;
						ژماردن += columns;
					}
				}

			} وإلا {
				صحيح extra = (table_width - max_width) /
						flexible_columns;
				remainder = (table_width - max_width) -
						(extra * flexible_columns);
				لكل (i = 0; i != columns; i++)
					إذا (col[i].type != COLUMN_WIDTH_FIXED) {
						col[i].width = col[i].max +
								extra;
						ژماردن -= remainder;
						إذا (ژماردن < 0) {
							col[i].width++;
							ژماردن += flexible_columns;
						}
					}
			}
		}
	} وإلا {
		/* space between min and max: fill it exactly */
		عائم scale = (عائم) (auto_width - min_width) /
				(عائم) (max_width - min_width);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		لكل (i = 0; i < columns; i++) {
			col[i].width = col[i].min + (صحيح) (0.5 +
					(col[i].max - col[i].min) * scale);
		}
		table_width = auto_width;
	}

	xs[0] = x = border_spacing_h;
	لكل (i = 0; i != columns; i++) {
		إذا (!col[i].positioned)
			x += col[i].width + border_spacing_h;
		xs[i + 1] = x;
		row_span[i] = 0;
		excess_y[i] = 0;
		row_span_cell[i] = 0;
	}

	/* position cells */
	table_height = border_spacing_v;
	لكل (row_group = table->children; row_group;
			row_group = row_group->next) {
		صحيح row_group_height = 0;
		لكل (row = row_group->children; row; row = row->next) {
			صحيح row_height = 0;

			htype = css_computed_height(row->style, &بەها, &unit);
			إذا (htype == CSS_HEIGHT_SET && unit != CSS_UNIT_PCT) {
				row_height = FIXTOINT(css_unit_len2device_px(
						row->style,
						&content->unit_len_ctx,
						بەها, unit));
			}
			لكل (c = row->children; c; c = c->next) {
				أكد(c->style);
				c->width = xs[c->start_column + c->columns] -
						xs[c->start_column] -
						border_spacing_h -
						c->border[چەپ_2].width -
						c->padding[چەپ_2] -
						c->padding[ڕاست_2] -
						c->border[ڕاست_2].width;
				c->float_children = 0;
				c->cached_place_below_level = 0;

				c->height = AUTO;
				إذا (!layout_block_context(c, -1, content)) {
					حرر(col);
					حرر(excess_y);
					حرر(row_span);
					حرر(row_span_cell);
					حرر(xs);
					أرجع false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->height;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_height(c->style,
						&بەها, &unit);

				إذا (htype == CSS_HEIGHT_SET &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					صحيح h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							بەها, unit));
					إذا (c->height < h)
						c->height = h;
				}
				/* specified row height is treated as a minimum
				 */
				إذا (c->height < row_height)
					c->height = row_height;
				c->x = xs[c->start_column] +
						c->border[چەپ_2].width;
				c->y = c->border[لووتکە_2].width;
				لكل (i = 0; i != c->columns; i++) {
					row_span[c->start_column + i] = c->rows;
					excess_y[c->start_column + i] =
							c->border[لووتکە_2].width +
							c->padding[لووتکە_2] +
							c->height +
							c->padding[BOTTOM] +
							c->border[BOTTOM].width;
					row_span_cell[c->start_column + i] = 0;
				}
				row_span_cell[c->start_column] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[لووتکە_2].width -
						c->padding[لووتکە_2] -
						c->height -
						c->border[BOTTOM].width;
			}
			لكل (i = 0; i != columns; i++)
				إذا (row_span[i] != 0)
					row_span[i]--;
				وإلا
					row_span_cell[i] = 0;
			إذا (row->next || row_group->next) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				لكل (i = 0; i != columns; i++)
					إذا (row_span[i] == 0 && row_height <
							excess_y[i])
						row_height = excess_y[i];
			} وإلا {
				/* except in the last row */
				لكل (i = 0; i != columns; i++)
					إذا (row_height < excess_y[i])
						row_height = excess_y[i];
			}
			لكل (i = 0; i != columns; i++) {
				إذا (row_height < excess_y[i])
					excess_y[i] -= row_height;
				وإلا
					excess_y[i] = 0;
				إذا (row_span_cell[i] != 0)
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
	لكل (row_group = table->children; row_group;
			row_group = row_group->next) {
		لكل (row = row_group->children; row; row = row->next) {
			لكل (c = row->children; c; c = c->next) {
				تعداد css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				spare_height = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->height - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &بەها, &unit);

				بدل (vertical_align) {
				حالة CSS_VERTICAL_ALIGN_SUB:
				حالة CSS_VERTICAL_ALIGN_SUPER:
				حالة CSS_VERTICAL_ALIGN_TEXT_لووتکە:
				حالة CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				حالة CSS_VERTICAL_ALIGN_SET:
				حالة CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				حالة CSS_VERTICAL_ALIGN_لووتکە:
					اكسر;
				حالة CSS_VERTICAL_ALIGN_ناوەڕاست:
					c->padding[لووتکە_2] += spare_height / 2;
					c->padding[BOTTOM] -= spare_height / 2;
					layout_move_children(c, 0,
							spare_height / 2);
					اكسر;
				حالة CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[لووتکە_2] += spare_height;
					c->padding[BOTTOM] -= spare_height;
					layout_move_children(c, 0,
							spare_height);
					اكسر;
				حالة CSS_VERTICAL_ALIGN_INHERIT:
					أكد(0);
					اكسر;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	إذا (table->margin[لووتکە_2] == AUTO)
		table->margin[لووتکە_2] = 0;
	إذا (table->margin[BOTTOM] == AUTO)
		table->margin[BOTTOM] = 0;

	حرر(col);
	حرر(excess_y);
	حرر(row_span);
	حرر(row_span_cell);
	حرر(xs);

	table->width = table_width;
	table->height = table_height;

	أرجع true;
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
ساكن bool layout_apply_minmax_height(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *box,
		بنية box *container)
{
	صحيح h;
	بنية box *containing_block = عدم;
	bool updated = false;

	/* Find containing block for percentage heights */
	إذا (box->style != عدم && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		أكد(container);
		containing_block = container;
	} وإلا إذا (box->float_container && box->style != عدم &&
			(css_computed_float(box->style) == CSS_FLOAT_چەپ ||
			 css_computed_float(box->style) == CSS_FLOAT_ڕاست)) {
		/* Box is a float */
		أكد(box->parent && box->parent->parent &&
				box->parent->parent->parent);
		containing_block = box->parent->parent->parent;
	} وإلا إذا (box->parent && box->parent->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_block = box->parent;
	} وإلا إذا (box->parent && box->parent->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		أكد(box->parent->parent);
		containing_block = box->parent->parent;
	}

	إذا (box->style) {
		تعداد css_height_e htype = CSS_HEIGHT_AUTO;
		css_fixed بەها = 0;
		css_unit unit = CSS_UNIT_PX;

		إذا (containing_block) {
			htype = css_computed_height(containing_block->style,
					&بەها, &unit);
		}

		/* max-height */
		إذا (css_computed_max_height(box->style, &بەها, &unit) ==
				CSS_MAX_HEIGHT_SET) {
			إذا (unit == CSS_UNIT_PCT) {
				إذا (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(بەها,
						containing_block->height);
					إذا (h < box->height) {
						box->height = h;
						updated = true;
					}
				}
			} وإلا {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						بەها, unit));
				إذا (h < box->height) {
					box->height = h;
					updated = true;
				}
			}
		}

		/* min-height */
		إذا (ns_computed_min_height(box->style, &بەها, &unit) ==
				CSS_MIN_HEIGHT_SET) {
			إذا (unit == CSS_UNIT_PCT) {
				إذا (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(بەها,
						containing_block->height);
					إذا (h > box->height) {
						box->height = h;
						updated = true;
					}
				}
			} وإلا {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						بەها, unit));
				إذا (h > box->height) {
					box->height = h;
					updated = true;
				}
			}
		}
	}
	أرجع updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
ساكن bool layout_block_object(بنية box *block)
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

	إذا (content_can_reformat(block->object)) {
		content_reformat(block->object, false, block->width, 1);
	} وإلا {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	أرجع true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
ساكن فراغ add_float_to_container(بنية box *cont, بنية box *b)
{
	بنية box *box = cont->float_children;
	صحيح b_bottom = b->y + b->height;

	أكد(b->type == BOX_FLOAT_چەپ || b->type == BOX_FLOAT_ڕاست);

	إذا (box == عدم) {
		/* No other float children */
		b->next_float = عدم;
		cont->float_children = b;
		أرجع;
	} وإلا إذا (b_bottom >= box->y + box->height) {
		/* Goes at start of list */
		b->next_float = cont->float_children;
		cont->float_children = b;
	} وإلا {
		بنية box *prev = عدم;
		طالما (box != عدم && b_bottom < box->y + box->height) {
			prev = box;
			box = box->next_float;
		}
		إذا (prev != عدم) {
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
ساكن bool
layout_text_box_split(html_content *content,
		      plot_font_style_t *fstyle,
		      بنية box *split_box,
		      size_t new_length,
		      صحيح new_width)
{
	صحيح space_width = split_box->space;
	بنية box *c2;
	ثابت بنية gui_layout_table *font_func = content->font_func;
	bool space = (split_box->text[new_length] == ' ');
	صحيح used_length = new_length + (space ? 1 : 0);

	إذا ((space && space_width == 0) || space_width == UNKNOWN_WIDTH) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->width(fstyle, " ", 1, &space_width);
	}

	إذا (split_box->space == UNKNOWN_WIDTH)
		split_box->space = space_width;
	إذا (!space)
		space_width = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, split_box, حجم *c2);
	إذا (!c2)
		أرجع false;
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
	إذا (c2->next)
		c2->next->prev = c2;
	وإلا
		c2->parent->last = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      split_box,
	      split_box->length,
	      (صحيح)split_box->length,
	      split_box->text);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->length,
	      (صحيح)c2->length,
	      c2->text);

	أرجع true;
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
ساكن فراغ
layout_float_find_dimensions(
		ثابت css_unit_ctx *unit_len_ctx,
		صحيح available_width,
		ثابت css_computed_style *style,
		بنية box *box)
{
	صحيح width, height, max_width, min_width, max_height, min_height;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	تعداد css_overflow_e overflow_x = css_computed_overflow_x(style);
	تعداد css_overflow_e overflow_y = css_computed_overflow_y(style);
	صحيح scrollbar_width_x =
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;
	صحيح scrollbar_width_y =
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;

	layout_find_dimensions(unit_len_ctx, available_width, -1, box, style,
			&width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	إذا (margin[چەپ_2] == AUTO)
		margin[چەپ_2] = 0;
	إذا (margin[ڕاست_2] == AUTO)
		margin[ڕاست_2] = 0;

	إذا (box->gadget == عدم) {
		padding[ڕاست_2] += scrollbar_width_y;
		padding[BOTTOM] += scrollbar_width_x;
	}

	إذا (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	} وإلا إذا (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed size = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		أكد(box->style);

		إذا (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			إذا (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			إذا (box->gadget->type == GADGET_FILE &&
					height == AUTO) {
				size = FLTTOFIX(1.5);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
		إذا (box->gadget->type == GADGET_TEXTAREA) {
			إذا (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			إذا (height == AUTO) {
				size = INTTOFIX(4);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
	} وإلا إذا (width == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		width = min(max(box->min_width, available_width),
				box->max_width);

		/* width includes margin, borders and padding */
		إذا (width == available_width) {
			width -= box->margin[چەپ_2] + box->border[چەپ_2].width +
					box->padding[چەپ_2] +
					box->padding[ڕاست_2] +
					box->border[ڕاست_2].width +
					box->margin[ڕاست_2];
		} وإلا {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			صحيح fixed = 0;
			عائم frac = 0;
			calculate_mbp_width(unit_len_ctx, box->style, چەپ_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_width(unit_len_ctx, box->style, ڕاست_2,
					true, true, true, &fixed, &frac);
			إذا (fixed < 0)
				fixed = 0;

			width -= fixed;
		}

		إذا (max_width >= 0 && width > max_width) width = max_width;
		إذا (min_width >  0 && width < min_width) width = min_width;

	} وإلا {
		إذا (max_width >= 0 && width > max_width) width = max_width;
		إذا (min_width >  0 && width < min_width) width = min_width;
		width -= scrollbar_width_y;
	}

	box->width = width;
	box->height = height;

	إذا (margin[لووتکە_2] == AUTO)
		margin[لووتکە_2] = 0;
	إذا (margin[BOTTOM] == AUTO)
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
ساكن bool layout_float(بنية box *b, صحيح width, html_content *content)
{
	أكد(b->type == BOX_TABLE ||
	       b->type == BOX_BLOCK ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_find_dimensions(&content->unit_len_ctx, width, b->style, b);
	إذا (b->type == BOX_TABLE || b->type == BOX_INLINE_FLEX) {
		إذا (b->type == BOX_TABLE) {
			إذا (!layout_table(b, width, content))
				أرجع false;
		} وإلا {
			إذا (!layout_flex(b, width, content))
				أرجع false;
		}
		إذا (b->margin[چەپ_2] == AUTO)
			b->margin[چەپ_2] = 0;
		إذا (b->margin[ڕاست_2] == AUTO)
			b->margin[ڕاست_2] = 0;
		إذا (b->margin[لووتکە_2] == AUTO)
			b->margin[لووتکە_2] = 0;
		إذا (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} وإلا {
		أرجع layout_block_context(b, -1, content);
	}
	أرجع true;
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
ساكن فراغ
place_float_below(بنية box *c, صحيح width, صحيح cx, صحيح y, بنية box *cont)
{
	صحيح x0, x1, yy;
	بنية box *چەپ;
	بنية box *ڕاست;

	yy = y > cont->cached_place_below_level ?
			y : cont->cached_place_below_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      width, cx, y, cont);

	افعل {
		y = yy;
		x0 = cx;
		x1 = cx + width;
		find_sides(cont->float_children, y, y + c->height, &x0, &x1,
				&چەپ, &ڕاست);
		إذا (چەپ != 0 && ڕاست != 0) {
			yy = (چەپ->y + چەپ->height <
					ڕاست->y + ڕاست->height ?
					چەپ->y + چەپ->height :
					ڕاست->y + ڕاست->height);
		} وإلا إذا (چەپ == 0 && ڕاست != 0) {
			yy = ڕاست->y + ڕاست->height;
		} وإلا إذا (چەپ != 0 && ڕاست == 0) {
			yy = چەپ->y + چەپ->height;
		}
	} طالما ((چەپ != 0 || ڕاست != 0) && (c->width > x1 - x0));

	إذا (c->type == BOX_FLOAT_چەپ) {
		c->x = x0;
	} وإلا {
		c->x = x1 - c->width;
	}
	c->y = y;
	cont->cached_place_below_level = y;
}


/**
 * Calculate line height from a style.
 */
ساكن صحيح هێڵ_height(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style)
{
	تعداد css_هێڵ_height_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed هێڵ_height;

	أكد(style);

	lhtype = css_computed_هێڵ_height(style, &lhvalue, &lhunit);
	إذا (lhtype == CSS_هێڵ_HEIGHT_NORMAL) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_هێڵ_HEIGHT_ژمار;
	}

	إذا (lhtype == CSS_هێڵ_HEIGHT_ژمار ||
			lhunit == CSS_UNIT_PCT) {
		هێڵ_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		إذا (lhtype != CSS_هێڵ_HEIGHT_ژمار)
			هێڵ_height = FDIV(هێڵ_height, F_100);
	} وإلا {
		أكد(lhunit != CSS_UNIT_PCT);

		هێڵ_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	أرجع FIXTOINT(هێڵ_height);
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
ساكن bool
layout_هێڵ(بنية box *first,
	    صحيح *width,
	    صحيح *y,
	    صحيح cx,
	    صحيح cy,
	    بنية box *cont,
	    bool indent,
	    bool has_text_children,
	    html_content *content,
	    بنية box **next_box)
{
	صحيح height, used_height;
	صحيح x0 = 0;
	صحيح x1 = *width;
	صحيح x, h, x_previous;
	صحيح fy = cy;
	بنية box *چەپ;
	بنية box *ڕاست;
	بنية box *b;
	بنية box *split_box = 0;
	بنية box *d;
	بنية box *br_box = 0;
	bool move_y = false;
	bool place_below = false;
	صحيح space_before = 0, space_after = 0;
	unsigned صحيح inline_ژماردن = 0;
	unsigned صحيح i;
	ثابت بنية gui_layout_table *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      first,
	      (صحيح)first->length,
	      first->text,
	      *width,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	find_sides(cont->float_children, cy, cy, &x0, &x1, &چەپ, &ڕاست);
	x0 -= cx;
	x1 -= cx;

	إذا (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	إذا (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	إذا (has_text_children || first->parent->parent->gadget)
		used_height = height = هێڵ_height(&content->unit_len_ctx,
				first->parent->parent->style);
	وإلا
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		used_height = height = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	لكل (x = 0, b = first; x <= x1 - x0 && b != 0; b = b->next) {
		صحيح min_width, max_width, min_height, max_height;

		أكد(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		إذا (b->type == BOX_BR)
			اكسر;

		إذا (lh__box_is_float_box(b))
			استمر;
		إذا (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			استمر;

		أكد(b->style != عدم);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		إذا (b->type == BOX_INLINE_BLOCK ||
		    b->type == BOX_INLINE_FLEX) {
			إذا (b->max_width != UNKNOWN_WIDTH)
				إذا (!layout_float(b, *width, content))
					أرجع false;
			h = b->border[لووتکە_2].width + b->padding[لووتکە_2] + b->height +
					b->padding[BOTTOM] +
					b->border[BOTTOM].width;
			إذا (height < h)
				height = h;
			x += b->margin[چەپ_2] + b->border[چەپ_2].width +
					b->padding[چەپ_2] + b->width +
					b->padding[ڕاست_2] +
					b->border[ڕاست_2].width +
					b->margin[ڕاست_2];
			space_after = 0;
			استمر;
		}

		إذا (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_find_dimensions(&content->unit_len_ctx,
					*width, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			لكل (i = 0; i != 4; i++)
				إذا (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[چەپ_2] + b->border[چەپ_2].width +
					b->padding[چەپ_2];
			إذا (b->inline_end) {
				b->inline_end->margin[ڕاست_2] = b->margin[ڕاست_2];
				b->inline_end->padding[ڕاست_2] =
						b->padding[ڕاست_2];
				b->inline_end->border[ڕاست_2] =
						b->border[ڕاست_2];
			} وإلا {
				x += b->padding[ڕاست_2] +
						b->border[ڕاست_2].width +
						b->margin[ڕاست_2];
			}
		} وإلا إذا (b->type == BOX_INLINE_END) {
			b->width = 0;
			إذا (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[ڕاست_2] + b->border[ڕاست_2].width +
					b->margin[ڕاست_2];
			استمر;
		}

		إذا (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->height = هێڵ_height(&content->unit_len_ctx,
					b->style ? b->style :
					b->parent->parent->style);
			إذا (height < b->height)
				height = b->height;

			إذا (!b->text) {
				b->width = 0;
				space_after = 0;
				استمر;
			}

			إذا (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				إذا (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					صحيح opt_maxwidth = 0;
					بنية form_option *o;

					لكل (o = b->parent->parent->gadget->
							data.select.کاڵاکان; o;
							o = o->next) {
						صحيح opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						إذا (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}
					b->width = opt_maxwidth;
					إذا (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;
				} وإلا {
					font_func->width(&fstyle, b->text,
							b->length, &b->width);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			إذا (b->text && (x + b->width < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->next) {
				font_func->width(&fstyle, b->text,
						 b->length, &b->width);
				b->flags |= MEASURED;
			}

			x += b->width;
			إذا (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			استمر;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		layout_find_dimensions(&content->unit_len_ctx,
				*width, -1, b, b->style,
				&b->width, &b->height,
				&max_width, &min_width,
				&max_height, &min_height,
				عدم, عدم, عدم);

		إذا (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->width, &b->height,
					min_width, max_width,
					min_height, max_height);
		} وإلا إذا (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			إذا (b->width == AUTO)
				b->width = 400;
			إذا (b->height == AUTO)
				b->height = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} وإلا {
			/* form control with no object */
			إذا (b->width == AUTO)
				b->width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			إذا (b->height == AUTO)
				b->height = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		إذا (b->object && content_can_reformat(b->object) &&
				b->width !=
				content_get_available_width(b->object)) {
			css_fixed بەها = 0;
			css_unit unit = CSS_UNIT_PX;
			تعداد css_height_e htype = css_computed_height(b->style,
					&بەها, &unit);

			content_reformat(b->object, false, b->width, b->height);

			إذا (htype == CSS_HEIGHT_AUTO)
				b->height = content_get_height(b->object);
		}

		إذا (height < b->height)
			height = b->height;

		x += b->width;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *width;
	find_sides(cont->float_children, cy, cy + height, &x0, &x1,
			&چەپ, &ڕاست);
	x0 -= cx;
	x1 -= cx;

	إذا (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	إذا (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	لكل (x = x_previous = 0, b = first; x <= x1 - x0 && b; b = b->next) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		إذا (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} وإلا إذا (lh__box_is_inline_flow(b)) {
			أكد(b->width != UNKNOWN_WIDTH);

			x_previous = x;
			x += space_after;
			b->x = x;

			إذا ((b->type == BOX_INLINE && !b->inline_end) ||
					b->type == BOX_INLINE_BLOCK ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[چەپ_2] + b->border[چەپ_2].width;
				x = b->x + b->padding[چەپ_2] + b->width +
						b->padding[ڕاست_2] +
						b->border[ڕاست_2].width +
						b->margin[ڕاست_2];
			} وإلا إذا (b->type == BOX_INLINE) {
				b->x += b->margin[چەپ_2] + b->border[چەپ_2].width;
				x = b->x + b->padding[چەپ_2] + b->width;
			} وإلا إذا (b->type == BOX_INLINE_END) {
				b->height = b->inline_end->height;
				x += b->padding[ڕاست_2] +
						b->border[ڕاست_2].width +
						b->margin[ڕاست_2];
			} وإلا {
				x += b->width;
			}

			space_before = space_after;
			إذا (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			وإلا إذا (b->text || b->type == BOX_INLINE_END) {
				إذا (b->space == UNKNOWN_WIDTH) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} وإلا {
				space_after = 0;
			}
			split_box = b;
			move_y = true;
			inline_ژماردن++;
		} وإلا إذا (b->type == BOX_BR) {
			b->x = x;
			b->width = 0;
			br_box = b;
			b = b->next;
			split_box = 0;
			move_y = true;
			اكسر;

		} وإلا {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_place_below_level = 0;
			b->float_container = d->float_container = cont;

			إذا (!layout_float(d, *width, content))
				أرجع false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[لووتکە_2],
			      d->border[لووتکە_2].width);

			d->x = d->margin[چەپ_2] + d->border[چەپ_2].width;
			d->y = d->margin[لووتکە_2] + d->border[لووتکە_2].width;
			b->width = d->margin[چەپ_2] + d->border[چەپ_2].width +
					d->padding[چەپ_2] + d->width +
					d->padding[ڕاست_2] +
					d->border[ڕاست_2].width +
					d->margin[ڕاست_2];
			b->height = d->margin[لووتکە_2] + d->border[لووتکە_2].width +
					d->padding[لووتکە_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];

			إذا (b->width > (x1 - x0) - x)
				place_below = true;
			إذا (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_چەپ && چەپ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ڕاست &&
						ڕاست == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						چەپ == 0 && ڕاست == 0)) &&
					(!place_below ||
					(چەپ == 0 && ڕاست == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_place_below_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				إذا (b->type == BOX_FLOAT_چەپ) {
					b->x = cx + x0;
					إذا (b->width > 0)
						x0 += b->width;
					چەپ = b;
				} وإلا {
					b->x = cx + x1 - b->width;
					إذا (b->width > 0)
						x1 -= b->width;
					ڕاست = b;
				}
				b->y = cy;
			} وإلا {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				صحيح fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_place_below_level) ?
						fcy :
						cont->cached_place_below_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + height : fy;

				place_float_below(b, *width, cx, fy, cont);
				fy = b->y;
				إذا (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_چەپ && چەپ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ڕاست &&
						ڕاست != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(چەپ != 0 || ڕاست != 0)))) {
					/* to be cleared below existing
					 * floats */
					إذا (b->type == BOX_FLOAT_چەپ)
						b->x = cx;
					وإلا
						b->x = cx + *width - b->width;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					إذا (fcy > cont->clear_level)
						cont->clear_level = fcy;
					إذا (b->y < fcy)
						b->y = fcy;
				}
				إذا (b->type == BOX_FLOAT_چەپ)
					چەپ = b;
				وإلا
					ڕاست = b;
			}
			add_float_to_container(cont, b);

			split_box = 0;
		}
	}

	إذا (x1 - x0 < x && split_box) {
		/* the last box went over the end */
		size_t split = 0;
		صحيح w;
		bool no_wrap = css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_previous;

		إذا (!no_wrap &&
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

		إذا (split == 0)
			w = split_box->width;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      split_box,
		      (صحيح)split_box->length,
		      split_box->text,
		      split,
		      w,
		      چەپ,
		      ڕاست,
		      inline_ژماردن);

		إذا ((split == 0 || x1 - x0 <= x + space_before + w) &&
				!چەپ && !ڕاست && inline_ژماردن == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			إذا (split == 0 || split == split_box->length) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = split_box->next;
			} وإلا {
				/* cut off first word for this line */
				إذا (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					أرجع false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} وإلا إذا ((split == 0 || x1 - x0 <= x + space_before + w) &&
				inline_ژماردن == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			أكد(چەپ || ڕاست);
			used_height = 0;
			إذا (چەپ) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      چەپ->y,
				      چەپ->height);

				used_height = چەپ->y + چەپ->height - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      used_height);

			}
			إذا (ڕاست && used_height <
					ڕاست->y + ڕاست->height - cy + 1)
				used_height = ڕاست->y + ڕاست->height - cy + 1;

			إذا (used_height < 0)
				used_height = 0;

			b = split_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} وإلا إذا (split == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = split_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} وإلا {
			/* fit as many words as possible */
			أكد(split != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (صحيح)split_box->length, split_box->text,
			      x1 - x0, split, w);

			إذا (split != split_box->length) {
				إذا (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					أرجع false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	بدل (css_computed_text_align(first->parent->parent->style)) {
	حالة CSS_TEXT_ALIGN_ڕاست:
	حالة CSS_TEXT_ALIGN_LIBCSS_ڕاست:
		x0 = x1 - x;
		اكسر;
	حالة CSS_TEXT_ALIGN_CENTER:
	حالة CSS_TEXT_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		اكسر;
	حالة CSS_TEXT_ALIGN_چەپ:
	حالة CSS_TEXT_ALIGN_LIBCSS_چەپ:
	حالة CSS_TEXT_ALIGN_JUSTIFY:
		/* leave on left */
		اكسر;
	حالة CSS_TEXT_ALIGN_DEFAULT:
		/* None; consider text direction */
		بدل (css_computed_direction(first->parent->parent->style)) {
		حالة CSS_DIRECTION_LTR:
			/* leave on left */
			اكسر;
		حالة CSS_DIRECTION_RTL:
			x0 = x1 - x;
			اكسر;
		}
		اكسر;
	}

	لكل (d = first; d != b; d = d->next) {
		d->flags &= ~NEW_هێڵ;

		إذا (d->type == BOX_INLINE_BLOCK &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			استمر;
		} وإلا إذا ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[لووتکە_2];

			إذا (d->type == BOX_TEXT && d->height > used_height) {
				/* text */
				used_height = d->height;
			}
		} وإلا إذا ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_BLOCK) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[لووتکە_2].width + d->margin[لووتکە_2];
			h = d->margin[لووتکە_2] + d->border[لووتکە_2].width +
					d->padding[لووتکە_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];
			إذا (used_height < h)
				used_height = h;
		}
	}

	first->flags |= NEW_هێڵ;

	أكد(b != first || (move_y && 0 < used_height && (چەپ || ڕاست)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	لكل (d = first; d != b; d = d->next) {
		إذا ((d->type == BOX_INLINE && d->inline_end) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			css_fixed بەها = 0;
			css_unit unit = CSS_UNIT_PX;
			بدل (css_computed_vertical_align(d->style, &بەها,
					&unit)) {
			حالة CSS_VERTICAL_ALIGN_SUPER:
			حالة CSS_VERTICAL_ALIGN_لووتکە:
			حالة CSS_VERTICAL_ALIGN_TEXT_لووتکە:
				/* already at top */
				اكسر;
			حالة CSS_VERTICAL_ALIGN_SUB:
			حالة CSS_VERTICAL_ALIGN_BOTTOM:
			حالة CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				d->y += used_height - d->height;
				اكسر;
			افتراضي:
			حالة CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (used_height - d->height);
				اكسر;
			}
		}
	}

	/* handle clearance for br */
	إذا (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		صحيح clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		إذا (used_height < clear_y - cy)
			used_height = clear_y - cy;
	}

	إذا (move_y)
		*y += used_height;
	*next_box = b;
	*width = x; /* return actual width */
	أرجع true;
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
ساكن bool layout_inline_container(بنية box *inline_container, صحيح width,
		بنية box *cont, صحيح cx, صحيح cy, html_content *content)
{
	bool first_هێڵ = true;
	bool has_text_children;
	بنية box *c, *next;
	صحيح y = 0;
	صحيح curwidth,maxwidth = width;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      width,
	      cont,
	      cx,
	      cy);


	has_text_children = false;
	لكل (c = inline_container->children; c; c = c->next) {
		bool is_pre = false;

		إذا (c->style) {
			تعداد css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_هێڵ ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		إذا ((lh__box_is_object(c) == false &&
				c->text && (c->length || is_pre)) ||
				c->type == BOX_BR)
			has_text_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	لكل (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->width;
		إذا (!layout_هێڵ(c, &curwidth, &y, cx, cy + y, cont, first_هێڵ,
				has_text_children, content, &next))
			أرجع false;
		maxwidth = max(maxwidth,curwidth);
		c = next;
		first_هێڵ = false;
	}

	inline_container->width = maxwidth;
	inline_container->height = y;

	أرجع true;
}


/* Documented in layout_intertnal.h */
bool layout_block_context(
		بنية box *block,
		صحيح viewport_height,
		html_content *content)
{
	بنية box *box;
	صحيح cx, cy;  /**< current coordinates */
	صحيح max_pos_margin = 0;
	صحيح max_neg_margin = 0;
	صحيح y = 0;
	صحيح lm, rm;
	بنية box *margin_collapse = عدم;
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

	block->float_children = عدم;
	block->cached_place_below_level = 0;
	block->clear_level = 0;

	/* special case if the block contains an object */
	إذا (block->object) {
		صحيح temp_width = block->width;
		إذا (!layout_block_object(block))
			أرجع false;
		layout_get_object_dimensions(block, &temp_width,
				&block->height, INT_MIN, INT_MAX,
				INT_MIN, INT_MAX);
		أرجع true;
	} وإلا إذا (block->flags & REPLACE_DIM) {
		أرجع true;
	}

	/* special case if the block contains an radio button or checkbox */
	إذا (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_size = INTTOFIX(1);
		إذا (block->height == AUTO)
			block->height = FIXTOINT(css_unit_len2device_px(
					block->style,
					&content->unit_len_ctx,
					gadget_size, gadget_unit));
	}

	box = block->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = block->padding[لووتکە_2];
	إذا (box)
		box->y = block->padding[لووتکە_2];

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
	طالما (box) {
		تعداد css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
		تعداد css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

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

		إذا (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->parent->padding[چەپ_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			اذهب advance_to_next_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		إذا (margin_collapse == عدم) {
			margin_collapse = layout_next_margin_block(
					&content->unit_len_ctx, box, block,
					viewport_height,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			in_margin = true;
		}

		/* Clearance. */
		y = 0;
		إذا (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(block->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		إذا (box->style) {
			overflow_x = css_computed_overflow_x(box->style);
			overflow_y = css_computed_overflow_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		إذا (box->type == BOX_FLEX ||
		    box->type == BOX_BLOCK ||
		    box->flags & IFRAME) {
			إذا (lh__box_is_object(box) == false &&
					box->style &&
					(overflow_x != CSS_OVERFLOW_VISIBLE ||
					 overflow_y != CSS_OVERFLOW_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				صحيح x0, x1, لووتکە;
				بنية box *چەپ, *ڕاست;
				لووتکە = cy + max_pos_margin - max_neg_margin;
				لووتکە = (لووتکە > y) ? لووتکە : y;
				x0 = cx;
				x1 = cx + box->parent->width -
						box->parent->padding[چەپ_2] -
						box->parent->padding[ڕاست_2];
				find_sides(block->float_children, لووتکە, لووتکە,
						&x0, &x1, &چەپ, &ڕاست);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->parent->width -
						box->parent->padding[چەپ_2] -
						box->parent->padding[ڕاست_2] -
						x1;
			}
			layout_block_find_dimensions(&content->unit_len_ctx,
					box->parent->width,
					viewport_height, lm, rm, box);
			إذا (box->type == BOX_BLOCK && !(box->flags & IFRAME)) {
				layout_block_add_scrollbar(box, ڕاست_2);
				layout_block_add_scrollbar(box, BOTTOM);
			}
		} وإلا إذا (box->type == BOX_TABLE) {
			إذا (box->style != عدم) {
				تعداد css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style, &width,
						&unit);

				إذا (wtype == CSS_WIDTH_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					صحيح x0, x1, لووتکە;
					بنية box *چەپ, *ڕاست;
					لووتکە = cy + max_pos_margin -
							max_neg_margin;
					لووتکە = (لووتکە > y) ? لووتکە : y;
					x0 = cx;
					x1 = cx + box->parent->width -
						box->parent->padding[چەپ_2] -
						box->parent->padding[ڕاست_2];
					find_sides(block->float_children,
						لووتکە, لووتکە, &x0, &x1,
						&چەپ, &ڕاست);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->parent->width -
						box->parent->padding[چەپ_2] -
						box->parent->padding[ڕاست_2] -
						x1;
				}
			}
			إذا (!layout_table(box, box->parent->width - lm - rm,
					content))
				أرجع false;
			layout_solve_width(box, box->parent->width, box->width,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->parent->padding[چەپ_2] + box->margin[چەپ_2] +
				box->border[چەپ_2].width;
		cx += box->x;

		/* Position box: vertical. */
		إذا (box->border[لووتکە_2].width) {
			box->y += box->border[لووتکە_2].width;
			cy += box->border[لووتکە_2].width;
		}

		/* Vertical margin */
		إذا (((box->type == BOX_BLOCK && (box->flags & HAS_HEIGHT)) ||
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
		إذا (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		إذا (box->type == BOX_FLEX ||
				(box->type == BOX_BLOCK && box->style &&
				 (overflow_x != CSS_OVERFLOW_VISIBLE ||
				  overflow_y != CSS_OVERFLOW_VISIBLE))) {

			إذا (box->type == BOX_FLEX) {
				إذا (!layout_flex(box, box->width, content)) {
					أرجع false;
				}
			} وإلا {
				layout_block_context(box,
						viewport_height, content);
			}

			cy += box->padding[لووتکە_2];

			إذا (box->height == AUTO) {
				box->height = 0;
				layout_block_add_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->height + box->padding[BOTTOM] +
					box->border[BOTTOM].width;
			y = box->y + box->padding[لووتکە_2] + box->height +
					box->padding[BOTTOM] +
					box->border[BOTTOM].width;

			/* Skip children, because they are done in the new
			 * block context */
			اذهب advance_to_next_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->width);

		/* Layout (except tables). */
		إذا (box->object) {
			إذا (!layout_block_object(box))
				أرجع false;

		} وإلا إذا (box->type == BOX_INLINE_CONTAINER) {
			box->width = box->parent->width;
			إذا (!layout_inline_container(box, box->width, block,
					cx, cy, content))
				أرجع false;

		} وإلا إذا (box->type == BOX_TABLE) {
			/* Move down to avoid floats if necessary. */
			صحيح x0, x1;
			بنية box *چەپ, *ڕاست;
			y = cy;
			طالما (1) {
				تعداد css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style,
						&width, &unit);

				x0 = cx;
				x1 = cx + box->parent->width;
				find_sides(block->float_children, y,
						y + box->height,
						&x0, &x1, &چەپ, &ڕاست);
				إذا (wtype == CSS_WIDTH_AUTO)
					اكسر;
				إذا (box->width <= x1 - x0)
					اكسر;
				إذا (!چەپ && !ڕاست)
					اكسر;
				وإلا إذا (!چەپ)
					y = ڕاست->y + ڕاست->height + 1;
				وإلا إذا (!ڕاست)
					y = چەپ->y + چەپ->height + 1;
				وإلا إذا (چەپ->y + چەپ->height <
						ڕاست->y + ڕاست->height)
					y = چەپ->y + چەپ->height + 1;
				وإلا
					y = ڕاست->y + ڕاست->height + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		إذا (box->type == BOX_BLOCK && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			إذا (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = عدم;
			}

			y = box->padding[لووتکە_2];
			box = box->children;
			box->y = y;
			cy += y;
			استمر;
		} وإلا إذا (box->type == BOX_BLOCK || box->object ||
				box->flags & IFRAME)
			cy += box->padding[لووتکە_2];

		إذا (box->type == BOX_BLOCK && box->height == AUTO) {
			box->height = 0;
			layout_block_add_scrollbar(box, BOTTOM);
		}

		cy += box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width;
		cx -= box->x;
		y = box->y + box->padding[لووتکە_2] + box->height +
				box->padding[BOTTOM] +
				box->border[BOTTOM].width;

	advance_to_next_box:
		إذا (!box->next) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			افعل {
				إذا (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = عدم;
				}

				/* Apply bottom margin */
				إذا (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				وإلا إذا (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->parent;
				إذا (box == block)
					اكسر;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				إذا (box->type == BOX_BLOCK &&
						box->flags & MAKE_HEIGHT) {
					margin_collapse = عدم;
					in_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				إذا (box->height == AUTO) {
					box->height = y - box->padding[لووتکە_2];

					إذا (box->type == BOX_BLOCK)
						layout_block_add_scrollbar(box,
								BOTTOM);
				} وإلا
					cy += box->height -
							(y - box->padding[لووتکە_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				إذا (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_height(
							&content->unit_len_ctx,
							box, عدم)) {
					/* Height altered */
					/* Set current cy */
					cy += box->height -
							(y - box->padding[لووتکە_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].width;
				cx -= box->x;
				y = box->y + box->padding[لووتکە_2] + box->height +
						box->padding[BOTTOM] +
						box->border[BOTTOM].width;

			} طالما (box->next == عدم);
			إذا (box == block)
				اكسر;
		}

		/* To next sibling. */

		إذا (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = عدم;
		}

		إذا (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		وإلا إذا (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->next;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	لكل (box = block->float_children; box; box = box->next_float) {
		y = box->y + box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width + box->margin[BOTTOM];
		إذا (cy < y)
			cy = y;
	}

	إذا (block->height == AUTO) {
		block->height = cy - block->padding[لووتکە_2];
		إذا (block->type == BOX_BLOCK)
			layout_block_add_scrollbar(block, BOTTOM);
	}

	إذا (block->style && css_computed_position(block->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_height(&content->unit_len_ctx, block, عدم);
	}

	إذا (block->gadget &&
			(block->gadget->type == GADGET_TEXTAREA ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		صحيح ta_width = block->padding[چەپ_2] + block->width +
				block->padding[ڕاست_2];
		صحيح ta_height = block->padding[لووتکە_2] + block->height +
				block->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				block->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_set_layout(block->gadget->data.text.ta,
				&fstyle, ta_width, ta_height,
				block->padding[لووتکە_2], block->padding[ڕاست_2],
				block->padding[BOTTOM], block->padding[چەپ_2]);
	}

	أرجع true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
ساكن bool
layout__get_element_tag(
		ثابت dom_گرێ *گرێ,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_گرێ_type گرێ_type;
	dom_exception exc;

	exc = dom_گرێ_get_گرێ_type(گرێ, &گرێ_type);
	إذا (exc != DOM_NO_ERR ||
	    گرێ_type != DOM_ELEMENT_گرێ) {
		أرجع false;
	}

	exc = dom_html_element_get_tag_type(گرێ, &element_type);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	*type = element_type;
	أرجع true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
ساكن مضمن bool
layout__check_element_type(
		ثابت dom_گرێ *گرێ,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	إذا (!layout__get_element_tag(گرێ, &element_type)) {
		أرجع false;
	}

	أرجع element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ساكن bool
layout_get_li_بەها(dom_گرێ *li_گرێ, dom_long *بەها_out)
{
	dom_exception exc;
	dom_long بەها;
	bool has_بەها;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_گرێ,
			corestring_dom_بەها,
			&has_بەها);
	إذا (exc != DOM_NO_ERR || has_بەها == false) {
		أرجع false;
	}

	exc = dom_html_li_element_get_بەها(
			(dom_html_li_element *)li_گرێ,
			&بەها);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	*بەها_out = بەها;
	أرجع true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ساكن bool
layout__get_ol_start(dom_گرێ *ol_گرێ, dom_long *start_out)
{
	dom_exception exc;
	dom_long start;
	bool has_start;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_گرێ,
			corestring_dom_start,
			&has_start);
	إذا (exc != DOM_NO_ERR || has_start == false) {
		أرجع false;
	}

	exc = dom_html_olist_element_get_start(
			(dom_html_olist_element *)ol_گرێ,
			&start);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	*start_out = start;
	أرجع true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
ساكن bool
layout__get_ol_reversed(dom_گرێ *ol_گرێ)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_گرێ,
			corestring_dom_reversed,
			&has_reversed);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	أرجع has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
ساكن bool
layout_get_list_item_ژماردن(
		dom_گرێ *list_owner, dom_long *ژماردن_out)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_گرێ *child;
	صحيح ژماردن;

	إذا (list_owner == عدم) {
		أرجع false;
	}

	إذا (!layout__get_element_tag(list_owner, &tag_type)) {
		أرجع false;
	}

	إذا (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		أرجع false;
	}

	exc = dom_گرێ_get_first_child(list_owner, &child);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	ژماردن = 0;
	طالما (child != عدم) {
		dom_گرێ *temp_گرێ;

		إذا (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			بنية box *child_box;
			إذا (dom_گرێ_get_user_data(child,
					corestring_dom_ns_کلیل_box_گرێ_data,
					&child_box) != DOM_NO_ERR) {
				dom_گرێ_unref(child);
				أرجع false;
			}

			إذا (child_box != عدم &&
			    child_box->list_marker != عدم) {
				ژماردن++;
			}
		}

		exc = dom_گرێ_get_next_sibling(child, &temp_گرێ);
		dom_گرێ_unref(child);
		إذا (exc != DOM_NO_ERR) {
			أرجع false;
		}

		child = temp_گرێ;
	}

	*ژماردن_out = ژماردن;
	أرجع true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
ساكن فراغ
layout_ordered_list_ژماردن(
		بنية box *box)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_گرێ *child;
	صحيح step = 1;
	dom_long next;

	إذا (box->گرێ == عدم) {
		أرجع;
	}

	إذا (!layout__get_element_tag(box->گرێ, &tag_type)) {
		أرجع;
	}

	إذا (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		أرجع;
	}

	next = 1;
	إذا (tag_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_start = layout__get_ol_start(box->گرێ, &next);
		bool have_reversed = layout__get_ol_reversed(box->گرێ);

		إذا (have_reversed) {
			step = -1;
		}

		إذا (!have_start && have_reversed) {
			layout_get_list_item_ژماردن(box->گرێ, &next);
		}
	}

	exc = dom_گرێ_get_first_child(box->گرێ, &child);
	إذا (exc != DOM_NO_ERR) {
		أرجع;
	}

	طالما (child != عدم) {
		dom_گرێ *temp_گرێ;

		إذا (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			بنية box *child_box;

			إذا (dom_گرێ_get_user_data(child,
					corestring_dom_ns_کلیل_box_گرێ_data,
					&child_box) != DOM_NO_ERR) {
				dom_گرێ_unref(child);
				أرجع;
			}

			إذا (child_box != عدم &&
			    child_box->list_marker != عدم) {
				dom_long بەها;
				بنية box *marker = child_box->list_marker;
				إذا (layout_get_li_بەها(child, &بەها)) {
					marker->list_بەها = بەها;
					next = marker->list_بەها;
				} وإلا {
					marker->list_بەها = next;
				}
				next += step;
			}
		}

		exc = dom_گرێ_get_next_sibling(child, &temp_گرێ);
		dom_گرێ_unref(child);
		إذا (exc != DOM_NO_ERR) {
			أرجع;
		}

		child = temp_گرێ;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
ساكن فراغ
layout__set_numerical_marker_text(
		ثابت html_content *content,
		بنية box *box)
{
	بنية box *marker = box->list_marker;
	size_t counter_len;
	css_error css_res;
	تعداد {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_SIZE = 20,
	};

	marker->text = talloc_array(content->bctx, حرف, LIST_MARKER_SIZE);
	إذا (marker->text == عدم) {
		أرجع;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_بەها,
			marker->text, LIST_MARKER_SIZE, &counter_len);
	إذا (css_res == CSS_OK) {
		إذا (counter_len > LIST_MARKER_SIZE) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->text = talloc_realloc(content->bctx,
					marker->text,
					حرف,
					counter_len);
			إذا (marker->text == عدم) {
				أرجع;
			}
			css_computed_format_list_style(box->style,
					marker->list_بەها, marker->text,
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
ساكن bool
layout__list_item_is_numerical(
		ثابت بنية box *b)
{
	تعداد css_list_style_type_e t = css_computed_list_style_type(b->style);

	بدل (t) {
	حالة CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	حالة CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	حالة CSS_LIST_STYLE_TYPE_SQUARE: /* Fall through. */
	حالة CSS_LIST_STYLE_TYPE_NONE:
		أرجع false;

	افتراضي:
		أرجع true;
	}
}

/**
 * Layout list markers.
 */
ساكن فراغ
layout_lists(ثابت html_content *content, بنية box *box)
{
	بنية box *child;

	layout_ordered_list_ژماردن(box);

	لكل (child = box->children; child; child = child->next) {
		إذا (child->list_marker) {
			بنية box *marker = child->list_marker;

			إذا (layout__list_item_is_numerical(child)) {
				إذا (marker->text == عدم) {
					layout__set_numerical_marker_text(
							content, child);
				}
			}
			إذا (marker->object) {
				marker->width =
					content_get_width(marker->object);
				marker->x = -marker->width;
				marker->height =
					content_get_height(marker->object);
				marker->y = (هێڵ_height(
						&content->unit_len_ctx,
						marker->style) -
						marker->height) / 2;
			} وإلا إذا (marker->text) {
				إذا (marker->width == UNKNOWN_WIDTH) {
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
				marker->height = هێڵ_height(
						&content->unit_len_ctx,
						marker->style);
			} وإلا {
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
ساكن فراغ
layout_compute_offsets(ثابت css_unit_ctx *unit_len_ctx,
		       بنية box *box,
		       بنية box *containing_block,
		       صحيح *لووتکە,
		       صحيح *ڕاست,
		       صحيح *bottom,
		       صحيح *چەپ)
{
	uint32_t type;
	css_fixed بەها = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(containing_block->width != UNKNOWN_WIDTH);
	أكد(containing_block->width != AUTO);
	أكد(containing_block->height != AUTO);

	/*چەپ*/
	type = css_computed_چەپ(box->style, &بەها, &unit);
	إذا (type == CSS_چەپ_SET) {
		إذا (unit == CSS_UNIT_PCT) {
			*چەپ = FPCT_OF_INT_TOINT(بەها,
					containing_block->width);
		} وإلا {
			*چەپ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					بەها, unit));
		}
	} وإلا {
		*چەپ = AUTO;
	}

	/*ڕاست*/
	type = css_computed_ڕاست(box->style, &بەها, &unit);
	إذا (type == CSS_ڕاست_SET) {
		إذا (unit == CSS_UNIT_PCT) {
			*ڕاست = FPCT_OF_INT_TOINT(بەها,
					containing_block->width);
		} وإلا {
			*ڕاست = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					بەها, unit));
		}
	} وإلا {
		*ڕاست = AUTO;
	}

	/*لووتکە*/
	type = css_computed_لووتکە(box->style, &بەها, &unit);
	إذا (type == CSS_لووتکە_SET) {
		إذا (unit == CSS_UNIT_PCT) {
			*لووتکە = FPCT_OF_INT_TOINT(بەها,
					containing_block->height);
		} وإلا {
			*لووتکە = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					بەها, unit));
		}
	} وإلا {
		*لووتکە = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &بەها, &unit);
	إذا (type == CSS_BOTTOM_SET) {
		إذا (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_OF_INT_TOINT(بەها,
					containing_block->height);
		} وإلا {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					بەها, unit));
		}
	} وإلا {
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
ساكن bool
layout_absolute(بنية box *box,
		بنية box *containing_block,
		صحيح cx, صحيح cy,
		html_content *content)
{
	صحيح static_چەپ, static_لووتکە;  /* static position */
	صحيح لووتکە, ڕاست, bottom, چەپ;
	صحيح width, height, max_width, min_width;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	صحيح available_width = containing_block->width;
	صحيح space;

	أكد(box->type == BOX_BLOCK || box->type == BOX_TABLE ||
			box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_چەپ = cx + box->x;
	static_لووتکە = cy + box->y;

	إذا (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_block->width += containing_block->padding[چەپ_2] +
				containing_block->padding[ڕاست_2];
		containing_block->height += containing_block->padding[لووتکە_2] +
				containing_block->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_block,
			&لووتکە, &ڕاست, &bottom, &چەپ);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_block;
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1,
			box, box->style, &width, &height,
			&max_width, &min_width, 0, 0,
			margin, padding, border);
	box->float_container = عدم;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      چەپ, margin[چەپ_2], border[چەپ_2].width, padding[چەپ_2], width,
	      padding[ڕاست_2], border[ڕاست_2].width, margin[ڕاست_2], ڕاست,
	      containing_block->width);


	إذا (چەپ == AUTO && width == AUTO && ڕاست == AUTO) {
		إذا (margin[چەپ_2] == AUTO)
			margin[چەپ_2] = 0;
		إذا (margin[ڕاست_2] == AUTO)
			margin[ڕاست_2] = 0;
		چەپ = static_چەپ;

		width = min(max(box->min_width, available_width),
			box->max_width);
		width -= box->margin[چەپ_2] + box->border[چەپ_2].width +
			box->padding[چەپ_2] + box->padding[ڕاست_2] +
			box->border[ڕاست_2].width + box->margin[ڕاست_2];

		/* Adjust for {min|max}-width */
		إذا (max_width >= 0 && width > max_width) width = max_width;
		إذا (width < min_width) width = min_width;

		ڕاست = containing_block->width -
			چەپ -
			margin[چەپ_2] - border[چەپ_2].width - padding[چەپ_2] -
			width -
			padding[ڕاست_2] - border[ڕاست_2].width - margin[ڕاست_2];
	} وإلا إذا (چەپ != AUTO && width != AUTO && ڕاست != AUTO) {

		/* Adjust for {min|max}-width */
		إذا (max_width >= 0 && width > max_width) width = max_width;
		إذا (min_width >  0 && width < min_width) width = min_width;

		إذا (margin[چەپ_2] == AUTO && margin[ڕاست_2] == AUTO) {
			space = containing_block->width -
					چەپ - border[چەپ_2].width -
					padding[چەپ_2] - width - padding[ڕاست_2] -
					border[ڕاست_2].width - ڕاست;
			إذا (space < 0) {
				margin[چەپ_2] = 0;
				margin[ڕاست_2] = space;
			} وإلا {
				margin[چەپ_2] = margin[ڕاست_2] = space / 2;
			}
		} وإلا إذا (margin[چەپ_2] == AUTO) {
			margin[چەپ_2] = containing_block->width -
					چەپ - border[چەپ_2].width -
					padding[چەپ_2] - width - padding[ڕاست_2] -
					border[ڕاست_2].width - margin[ڕاست_2] -
					ڕاست;
		} وإلا إذا (margin[ڕاست_2] == AUTO) {
			margin[ڕاست_2] = containing_block->width -
					چەپ - margin[چەپ_2] -
					border[چەپ_2].width -
					padding[چەپ_2] - width - padding[ڕاست_2] -
					border[ڕاست_2].width - ڕاست;
		} وإلا {
			ڕاست = containing_block->width -
					چەپ - margin[چەپ_2] -
					border[چەپ_2].width -
					padding[چەپ_2] - width - padding[ڕاست_2] -
					border[ڕاست_2].width - margin[ڕاست_2];
		}
	} وإلا {
		إذا (margin[چەپ_2] == AUTO)
			margin[چەپ_2] = 0;
		إذا (margin[ڕاست_2] == AUTO)
			margin[ڕاست_2] = 0;

		إذا (چەپ == AUTO && width == AUTO && ڕاست != AUTO) {
			available_width -= ڕاست;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[چەپ_2] + box->border[چەپ_2].width +
				box->padding[چەپ_2] + box->padding[ڕاست_2] +
				box->border[ڕاست_2].width + box->margin[ڕاست_2];

			/* Adjust for {min|max}-width */
			إذا (max_width >= 0 && width > max_width)
				width = max_width;
			إذا (width < min_width)
				width = min_width;

			چەپ = containing_block->width -
					margin[چەپ_2] - border[چەپ_2].width -
					padding[چەپ_2] - width - padding[ڕاست_2] -
					border[ڕاست_2].width - margin[ڕاست_2] -
					ڕاست;
		} وإلا إذا (چەپ == AUTO && width != AUTO && ڕاست == AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_width >= 0 && width > max_width)
				width = max_width;
			إذا (min_width >  0 && width < min_width)
				width = min_width;

			چەپ = static_چەپ;
			ڕاست = containing_block->width -
					چەپ - margin[چەپ_2] -
					border[چەپ_2].width -
					padding[چەپ_2] - width - padding[ڕاست_2] -
					border[ڕاست_2].width - margin[ڕاست_2];
		} وإلا إذا (چەپ != AUTO && width == AUTO && ڕاست == AUTO) {
			available_width -= چەپ;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[چەپ_2] + box->border[چەپ_2].width +
				box->padding[چەپ_2] + box->padding[ڕاست_2] +
				box->border[ڕاست_2].width + box->margin[ڕاست_2];

			/* Adjust for {min|max}-width */
			إذا (max_width >= 0 && width > max_width)
				width = max_width;
			إذا (width < min_width)
				width = min_width;

			ڕاست = containing_block->width -
					چەپ - margin[چەپ_2] -
					border[چەپ_2].width -
					padding[چەپ_2] - width - padding[ڕاست_2] -
					border[ڕاست_2].width - margin[ڕاست_2];
		} وإلا إذا (چەپ == AUTO && width != AUTO && ڕاست != AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_width >= 0 && width > max_width)
				width = max_width;
			إذا (width < min_width)
				width = min_width;

			چەپ = containing_block->width -
					margin[چەپ_2] - border[چەپ_2].width -
					padding[چەپ_2] - width - padding[ڕاست_2] -
					border[ڕاست_2].width - margin[ڕاست_2] -
					ڕاست;
		} وإلا إذا (چەپ != AUTO && width == AUTO && ڕاست != AUTO) {
			width = containing_block->width -
					چەپ - margin[چەپ_2] -
					border[چەپ_2].width -
					padding[چەپ_2] - padding[ڕاست_2] -
					border[ڕاست_2].width - margin[ڕاست_2] -
					ڕاست;

			/* Adjust for {min|max}-width */
			إذا (max_width >= 0 && width > max_width)
				width = max_width;
			إذا (width < min_width)
				width = min_width;

		} وإلا إذا (چەپ != AUTO && width != AUTO && ڕاست == AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_width >= 0 && width > max_width)
				width = max_width;
			إذا (width < min_width)
				width = min_width;

			ڕاست = containing_block->width -
					چەپ - margin[چەپ_2] -
					border[چەپ_2].width -
					padding[چەپ_2] - width - padding[ڕاست_2] -
					border[ڕاست_2].width - margin[ڕاست_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      چەپ, margin[چەپ_2], border[چەپ_2].width, padding[چەپ_2], width,
	      padding[ڕاست_2], border[ڕاست_2].width, margin[ڕاست_2], ڕاست,
	      containing_block->width);

	box->x = چەپ + margin[چەپ_2] + border[چەپ_2].width - cx;
	إذا (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_block->width -= containing_block->padding[چەپ_2] +
				containing_block->padding[ڕاست_2];
	} وإلا {
		/** \todo inline ancestors */
	}
	box->width = width;
	box->height = height;

	إذا (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->object || box->flags & IFRAME) {
		إذا (!layout_block_context(box, -1, content))
			أرجع false;
	} وإلا إذا (box->type == BOX_TABLE) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		/* \todo  layout_table considers margins etc. again */
		إذا (!layout_table(box, width, content))
			أرجع false;
		box->float_container = عدم;
		layout_solve_width(box, box->parent->width, box->width, 0, 0,
				-1, -1);
	} وإلا إذا (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		إذا (!layout_flex(box, width, content))
			أرجع false;
		box->float_container = عدم;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      لووتکە, margin[لووتکە_2], border[لووتکە_2].width, padding[لووتکە_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	إذا (لووتکە == AUTO && height == AUTO && bottom == AUTO) {
		لووتکە = static_لووتکە;
		height = box->height;
		إذا (margin[لووتکە_2] == AUTO)
			margin[لووتکە_2] = 0;
		إذا (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_block->height -
				لووتکە - margin[لووتکە_2] - border[لووتکە_2].width -
				padding[لووتکە_2] - height - padding[BOTTOM] -
				border[BOTTOM].width - margin[BOTTOM];
	} وإلا إذا (لووتکە != AUTO && height != AUTO && bottom != AUTO) {
		إذا (margin[لووتکە_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_block->height -
					لووتکە - border[لووتکە_2].width - padding[لووتکە_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - bottom;
			margin[لووتکە_2] = margin[BOTTOM] = space / 2;
		} وإلا إذا (margin[لووتکە_2] == AUTO) {
			margin[لووتکە_2] = containing_block->height -
					لووتکە - border[لووتکە_2].width - padding[لووتکە_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} وإلا إذا (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_block->height -
					لووتکە - margin[لووتکە_2] - border[لووتکە_2].width -
					padding[لووتکە_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					bottom;
		} وإلا {
			bottom = containing_block->height -
					لووتکە - margin[لووتکە_2] - border[لووتکە_2].width -
					padding[لووتکە_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	} وإلا {
		إذا (margin[لووتکە_2] == AUTO)
			margin[لووتکە_2] = 0;
		إذا (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		إذا (لووتکە == AUTO && height == AUTO && bottom != AUTO) {
			height = box->height;
			لووتکە = containing_block->height -
					margin[لووتکە_2] - border[لووتکە_2].width -
					padding[لووتکە_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} وإلا إذا (لووتکە == AUTO && height != AUTO && bottom == AUTO) {
			لووتکە = static_لووتکە;
			bottom = containing_block->height -
					لووتکە - margin[لووتکە_2] - border[لووتکە_2].width -
					padding[لووتکە_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} وإلا إذا (لووتکە != AUTO && height == AUTO && bottom == AUTO) {
			height = box->height;
			bottom = containing_block->height -
					لووتکە - margin[لووتکە_2] - border[لووتکە_2].width -
					padding[لووتکە_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} وإلا إذا (لووتکە == AUTO && height != AUTO && bottom != AUTO) {
			لووتکە = containing_block->height -
					margin[لووتکە_2] - border[لووتکە_2].width -
					padding[لووتکە_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} وإلا إذا (لووتکە != AUTO && height == AUTO && bottom != AUTO) {
			height = containing_block->height -
					لووتکە - margin[لووتکە_2] - border[لووتکە_2].width -
					padding[لووتکە_2] - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} وإلا إذا (لووتکە != AUTO && height != AUTO && bottom == AUTO) {
			bottom = containing_block->height -
					لووتکە - margin[لووتکە_2] - border[لووتکە_2].width -
					padding[لووتکە_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      لووتکە, margin[لووتکە_2], border[لووتکە_2].width, padding[لووتکە_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	box->y = لووتکە + margin[لووتکە_2] + border[لووتکە_2].width - cy;
	إذا (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_block->height -= containing_block->padding[لووتکە_2] +
				containing_block->padding[BOTTOM];
	} وإلا {
		/** \todo Inline ancestors */
	}
	box->height = height;
	layout_apply_minmax_height(&content->unit_len_ctx, box, containing_block);

	أرجع true;
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
ساكن bool
layout_position_absolute(بنية box *box,
			 بنية box *containing_block,
			 صحيح cx, صحيح cy,
			 html_content *content)
{
	بنية box *c;

	لكل (c = box->children; c; c = c->next) {
		إذا ((c->type == BOX_BLOCK || c->type == BOX_TABLE ||
				c->type == BOX_INLINE_BLOCK ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			إذا (!layout_absolute(c, containing_block,
					cx, cy, content))
				أرجع false;
			إذا (!layout_position_absolute(c, c, 0, 0, content))
				أرجع false;
		} وإلا إذا (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			إذا (!layout_position_absolute(c, c, 0, 0, content))
				أرجع false;
		} وإلا {
			صحيح px, py;
			إذا (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_چەپ ||
					css_computed_float(c->style) ==
						CSS_FLOAT_ڕاست)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				بنية box *p;
				px = c->x;
				py = c->y;
				لكل (p = box->parent; p && !p->float_children;
						p = p->parent) {
					px -= p->x;
					py -= p->y;
				}
			} وإلا {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			إذا (!layout_position_absolute(c, containing_block,
					cx + px, cy + py, content))
				أرجع false;
		}
	}

	أرجع true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
ساكن فراغ layout_compute_relative_offset(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *box,
		صحيح *x,
		صحيح *y)
{
	صحيح چەپ, ڕاست, لووتکە, bottom;
	بنية box *containing_block;

	أكد(box && box->parent && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	إذا (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_چەپ ||
			css_computed_float(box->style) == CSS_FLOAT_ڕاست)) {
		containing_block = box->float_container;
	} وإلا {
		containing_block = box->parent;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_block,
			&لووتکە, &ڕاست, &bottom, &چەپ);

	إذا (چەپ == AUTO && ڕاست == AUTO)
		چەپ = ڕاست = 0;
	وإلا إذا (چەپ == AUTO)
		/* left is auto => computed = -right */
		چەپ = -ڕاست;
	وإلا إذا (ڕاست == AUTO)
		/* right is auto => computed = -left */
		ڕاست = -چەپ;
	وإلا {
		/* over constrained => examine direction property
		 * of containing block */
		إذا (containing_block->style &&
				css_computed_direction(
				containing_block->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			چەپ = -ڕاست;
		} وإلا {
			/* assume LTR in all other cases */
			ڕاست = -چەپ;
		}
	}

	أكد(چەپ == -ڕاست);

	إذا (لووتکە == AUTO && bottom == AUTO) {
		لووتکە = bottom = 0;
	} وإلا إذا (لووتکە == AUTO) {
		لووتکە = -bottom;
	} وإلا {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -لووتکە;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", چەپ,
	      ڕاست, لووتکە, bottom);

	*x = چەپ;
	*y = لووتکە;
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
ساكن فراغ
layout_position_relative(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *ڕەگ,
		بنية box *fp,
		صحيح fx,
		صحيح fy)
{
	بنية box *box; /* for children of "root" */
	بنية box *fn;  /* for block formatting context box for children of
			  * "box" */
	بنية box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	صحيح x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	صحيح fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	أكد(ڕەگ);

	/* Normal children */
	لكل (box = ڕەگ->children; box; box = box->next) {

		إذا (box->type == BOX_TEXT)
			استمر;

		/* If relatively positioned, get offsets */
		إذا (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_offset(
					unit_len_ctx, box, &x, &y);
		وإلا
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		إذا (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_چەپ ||
				css_computed_float(box->style) ==
					CSS_FLOAT_ڕاست) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			لكل (fc = fp->float_children; fc; fc = fc->next_float) {
				إذا (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		إذا (box->float_children) {
			fn = box;
			fnx = fny = 0;
		} وإلا {
			fn = fp;
			fnx = fx + x;
			fny = fy + y;
		}

		/* recurse first */
		layout_position_relative(unit_len_ctx, box, fn, fnx, fny);

		/* Ignore things we're not interested in. */
		إذا (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			استمر;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		إذا (box->type == BOX_INLINE && box->inline_end) {
			بنية box *b;
			لكل (b = box->next; b && b != box->inline_end;
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
ساكن فراغ
layout_get_box_bbox(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *box,
		صحيح *desc_x0, صحيح *desc_y0,
		صحيح *desc_x1, صحيح *desc_y1)
{
	*desc_x0 = -box->border[چەپ_2].width;
	*desc_y0 = -box->border[لووتکە_2].width;
	*desc_x1 = box->padding[چەپ_2] + box->width + box->padding[ڕاست_2] +
			box->border[ڕاست_2].width;
	*desc_y1 = box->padding[لووتکە_2] + box->height + box->padding[BOTTOM] +
			box->border[BOTTOM].width;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	إذا (box->type == BOX_BLOCK && box->style != عدم &&
			css_computed_overflow_y(box->style) ==
					CSS_OVERFLOW_VISIBLE &&
			box->object == عدم) {
		css_fixed font_size = 0;
		css_unit font_unit = CSS_UNIT_PT;
		صحيح text_height;

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
ساكن فراغ
layout_update_descendant_bbox(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *box,
		بنية box *child,
		صحيح off_x,
		صحيح off_y)
{
	صحيح child_desc_x0, child_desc_y0, child_desc_x1, child_desc_y1;

	/* get coordinates of child relative to box */
	صحيح child_x = child->x - off_x;
	صحيح child_y = child->y - off_y;

	bool html_object = (child->object &&
			content_get_type(child->object) == CONTENT_HTML);

	تعداد css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	تعداد css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

	إذا (child->style != عدم) {
		overflow_x = css_computed_overflow_x(child->style);
		overflow_y = css_computed_overflow_y(child->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, child,
			&child_desc_x0, &child_desc_y0,
			&child_desc_x1, &child_desc_y1);

	إذا (overflow_x == CSS_OVERFLOW_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		child_desc_x0 = child->descendant_x0;
		child_desc_x1 = child->descendant_x1;
	}
	إذا (overflow_y == CSS_OVERFLOW_VISIBLE &&
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
	إذا (child_desc_x0 < box->descendant_x0)
		box->descendant_x0 = child_desc_x0;
	إذا (child_desc_y0 < box->descendant_y0)
		box->descendant_y0 = child_desc_y0;
	إذا (box->descendant_x1 < child_desc_x1)
		box->descendant_x1 = child_desc_x1;
	إذا (box->descendant_y1 < child_desc_y1)
		box->descendant_y1 = child_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
ساكن فراغ layout_calculate_descendant_bboxes(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *box)
{
	بنية box *child;

	أكد(box->width != UNKNOWN_WIDTH);
	أكد(box->height != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	إذا (box->object && content_get_type(box->object) == CONTENT_HTML) {
		إذا (box->descendant_x1 < content_get_width(box->object))
			box->descendant_x1 = content_get_width(box->object);
		إذا (box->descendant_y1 < content_get_height(box->object))
			box->descendant_y1 = content_get_height(box->object);
	}

	إذا (box->iframe != عدم) {
		صحيح x, y;
		box_coords(box, &x, &y);

		browser_window_set_position(box->iframe, x, y);
		browser_window_set_dimensions(box->iframe,
				box->width, box->height);
		browser_window_reformat(box->iframe, true,
				box->width, box->height);
	}

	إذا (box->type == BOX_INLINE || box->type == BOX_TEXT)
		أرجع;

	إذا (box->type == BOX_INLINE_END) {
		box = box->inline_end;
		لكل (child = box->next; child;
				child = child->next) {
			إذا (child->type == BOX_FLOAT_چەپ ||
					child->type == BOX_FLOAT_ڕاست)
				استمر;

			layout_update_descendant_bbox(unit_len_ctx, box, child,
					box->x, box->y);

			إذا (child == box->inline_end)
				اكسر;
		}
		أرجع;
	}

	إذا (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		أرجع;

	لكل (child = box->children; child; child = child->next) {
		إذا (child->type == BOX_FLOAT_چەپ ||
				child->type == BOX_FLOAT_ڕاست)
			استمر;

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		إذا (box->style && css_computed_overflow_x(box->style) ==
				CSS_OVERFLOW_HIDDEN &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_HIDDEN)
			استمر;

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	لكل (child = box->float_children; child; child = child->next_float) {
		أكد(child->type == BOX_FLOAT_چەپ ||
				child->type == BOX_FLOAT_ڕاست);

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	إذا (box->list_marker) {
		child = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, صحيح width, صحيح height)
{
	bool ret;
	بنية box *doc = content->layout;
	ثابت بنية gui_layout_table *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			width, height, nsurl_access(content_get_url(
					&content->base)));

	layout_minmax_block(doc, font_func, content);

	layout_block_find_dimensions(&content->unit_len_ctx,
			width, height, 0, 0, doc);
	doc->x = doc->margin[چەپ_2] + doc->border[چەپ_2].width;
	doc->y = doc->margin[لووتکە_2] + doc->border[لووتکە_2].width;
	width -= doc->margin[چەپ_2] + doc->border[چەپ_2].width +
			doc->padding[چەپ_2] + doc->padding[ڕاست_2] +
			doc->border[ڕاست_2].width + doc->margin[ڕاست_2];
	إذا (width < 0) {
		width = 0;
	}
	doc->width = width;

	ret = layout_block_context(doc, height, content);

	/* make <html> and <body> fill available height */
	إذا (doc->y + doc->padding[لووتکە_2] + doc->height + doc->padding[BOTTOM] +
			doc->border[BOTTOM].width + doc->margin[BOTTOM] <
			height) {
		doc->height = height - (doc->y + doc->padding[لووتکە_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].width +
				doc->margin[BOTTOM]);
		إذا (doc->children)
			doc->children->height = doc->height -
					(doc->children->margin[لووتکە_2] +
					 doc->children->border[لووتکە_2].width +
					 doc->children->padding[لووتکە_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].width +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	أرجع ret;
}
