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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <math.h>
#અન્તર્ભાવ <dom/dom.h>

#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/talloc.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "netsurf/inttypes.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "netsurf/browser_window.h"
#અન્તર્ભાવ "netsurf/layout.h"
#અન્તર્ભાવ "content/content.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "css/utils.h"
#અન્તર્ભાવ "desktop/scrollbar.h"
#અન્તર્ભાવ "desktop/textarea.h"

#અન્તર્ભાવ "html/html.h"
#અન્તર્ભાવ "html/html_save.h"
#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_inspect.h"
#અન્તર્ભાવ "html/font.h"
#અન્તર્ભાવ "html/form_internal.h"
#અન્તર્ભાવ "html/layout.h"
#અન્તર્ભાવ "html/layout_internal.h"
#અન્તર્ભાવ "html/table.h"

/** Array of per-side access functions for computed style margins. */
અચલ css_len_func margin_funcs[4] = {
	[ટોચ_2]    = css_computed_margin_ટોચ,
	[જમણું_2]  = css_computed_margin_જમણું,
	[BOTTOM] = css_computed_margin_bottom,
	[ડાબું_2]   = css_computed_margin_ડાબું,
};

/** Array of per-side access functions for computed style paddings. */
અચલ css_len_func padding_funcs[4] = {
	[ટોચ_2]    = css_computed_padding_ટોચ,
	[જમણું_2]  = css_computed_padding_જમણું,
	[BOTTOM] = css_computed_padding_bottom,
	[ડાબું_2]   = css_computed_padding_ડાબું,
};

/** Array of per-side access functions for computed style border_widths. */
અચલ css_len_func border_width_funcs[4] = {
	[ટોચ_2]    = css_computed_border_ટોચ_width,
	[જમણું_2]  = css_computed_border_જમણું_width,
	[BOTTOM] = css_computed_border_bottom_width,
	[ડાબું_2]   = css_computed_border_ડાબું_width,
};

/** Array of per-side access functions for computed style border styles. */
અચલ css_border_style_func border_style_funcs[4] = {
	[ટોચ_2]    = css_computed_border_ટોચ_style,
	[જમણું_2]  = css_computed_border_જમણું_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[ડાબું_2]   = css_computed_border_ડાબું_style,
};

/** Array of per-side access functions for computed style border colors. */
અચલ css_border_color_func border_color_funcs[4] = {
	[ટોચ_2]    = css_computed_border_ટોચ_color,
	[જમણું_2]  = css_computed_border_જમણું_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[ડાબું_2]   = css_computed_border_ડાબું_color,
};

/* forward declaration to break cycles */
સ્થિર શૂન્ય layout_minmax_block(
		સંરચના box *block,
		અચલ સંરચના gui_layout_table *font_func,
		અચલ html_content *content);

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
સ્થિર શૂન્ય
layout_get_object_dimensions(સંરચના box *box,
			     પૂર્ણાંક *width, પૂર્ણાંક *height,
			     પૂર્ણાંક min_width, પૂર્ણાંક max_width,
			     પૂર્ણાંક min_height, પૂર્ણાંક max_height)
{
	પ્રતિજ્ઞા(box->object != ખાલી);
	પ્રતિજ્ઞા(width != ખાલી && height != ખાલી);

	જો (*width == AUTO && *height == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		પૂર્ણાંક intrinsic_width = content_get_width(box->object);
		પૂર્ણાંક intrinsic_height = content_get_height(box->object);

		/* use intrinsic dimensions */
		*width = intrinsic_width;
		*height = intrinsic_height;

		/* Deal with min/max-width first */
		જો (min_width >  0 && min_width > *width) {
			*width = min_width;
			scaled = true;
		}
		જો (max_width >= 0 && max_width < *width) {
			*width = max_width;
			scaled = true;
		}

		જો (scaled && (intrinsic_width != 0)) {
			/* Update height */
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		}

		scaled = false;
		/* Deal with min/max-height */
		જો (min_height >  0 && min_height > *height) {
			*height = min_height;
			scaled = true;
		}
		જો (max_height >= 0 && max_height < *height) {
			*height = max_height;
			scaled = true;
		}

		જો (scaled && (intrinsic_height != 0)) {
			/* Update width */
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		}

	} નહીંતર જો (*width == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		પૂર્ણાંક intrinsic_width = content_get_width(box->object);
		પૂર્ણાંક intrinsic_height = content_get_height(box->object);

		જો (intrinsic_height != 0)
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		નહીંતર
			*width = intrinsic_width;

		જો (min_width >  0 && min_width > *width)
			*width = min_width;
		જો (max_width >= 0 && max_width < *width)
			*width = max_width;

	} નહીંતર જો (*height == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		પૂર્ણાંક intrinsic_width = content_get_width(box->object);
		પૂર્ણાંક intrinsic_height = content_get_height(box->object);

		જો (min_width >  0 && min_width > *width)
			*width = min_width;
		જો (max_width >= 0 && max_width < *width)
			*width = max_width;

		જો (intrinsic_width != 0)
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		નહીંતર
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
સ્થિર પૂર્ણાંક layout_text_indent(
		અચલ css_unit_ctx *unit_len_ctx,
		અચલ css_computed_style *style, પૂર્ણાંક width)
{
	css_fixed કિંમત = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_text_indent(style, &કિંમત, &unit);

	જો (unit == CSS_UNIT_PCT) {
		પાછા FPCT_OF_INT_TOINT(કિંમત, width);
	} નહીંતર {
		પાછા FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				કિંમત, unit));
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
સ્થિર શૂન્ય layout_minmax_table(સંરચના box *table,
		અચલ સંરચના gui_layout_table *font_func,
		અચલ html_content *content)
{
	unsigned પૂર્ણાંક i, j;
	પૂર્ણાંક width;
	પૂર્ણાંક border_spacing_h = 0;
	પૂર્ણાંક table_min = 0, table_max = 0;
	પૂર્ણાંક extra_fixed = 0;
	દશાંશ extra_frac = 0;
	સંરચના column *col;
	સંરચના box *row_group, *row, *cell;

	/* check if the widths have already been calculated */
	જો (table->max_width != UNKNOWN_MAX_WIDTH)
		પાછા;

	જો (table_calculate_column_types(&content->unit_len_ctx, table) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		પાછા;
	}
	col = table->col;

	/* start with 0 except for fixed-width columns */
	માટે (i = 0; i != table->columns; i++) {
		જો (col[i].type == COLUMN_WIDTH_FIXED)
			col[i].min = col[i].max = col[i].width;
		નહીંતર
			col[i].min = col[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	જો (css_computed_border_collapse(table->style) ==
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
	માટે (row_group = table->children; row_group; row_group =row_group->next)
	માટે (row = row_group->children; row; row = row->next)
	માટે (cell = row->children; cell; cell = cell->next) {
		પ્રતિજ્ઞા(cell->type == BOX_TABLE_CELL);
		પ્રતિજ્ઞા(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		પ્રતિજ્ઞા(cell->columns != 0);

		જો (cell->columns != 1)
			ચાલુ;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		જો (col[i].positioned)
			ચાલુ;

		/* update column min, max widths using cell widths */
		જો (col[i].min < cell->min_width)
			col[i].min = cell->min_width;
		જો (col[i].max < cell->max_width)
			col[i].max = cell->max_width;
	}

	/* 2nd pass: cells which span multiple columns */
	માટે (row_group = table->children; row_group; row_group =row_group->next)
	માટે (row = row_group->children; row; row = row->next)
	માટે (cell = row->children; cell; cell = cell->next) {
		unsigned પૂર્ણાંક flexible_columns = 0;
		પૂર્ણાંક min = 0, max = 0, fixed_width = 0, extra;

		જો (cell->columns == 1)
			ચાલુ;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		માટે (j = 0; j != cell->columns; j++) {
			min += col[i + j].min;
			જો (col[i + j].type == COLUMN_WIDTH_FIXED)
				fixed_width += col[i + j].width;
			નહીંતર
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		જો (min < cell->min_width) {
			જો (flexible_columns == 0) {
				extra = 1 + (cell->min_width - min) /
						cell->columns;
				માટે (j = 0; j != cell->columns; j++) {
					col[i + j].min += extra;
					જો (col[i + j].max < col[i + j].min)
						col[i + j].max = col[i + j].min;
				}
			} નહીંતર {
				extra = 1 + (cell->min_width - min) /
						flexible_columns;
				માટે (j = 0; j != cell->columns; j++) {
					જો (col[i + j].type !=
							COLUMN_WIDTH_FIXED) {
						col[i + j].min += extra;
						જો (col[i + j].max <
								col[i + j].min)
							col[i + j].max =
								col[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		માટે (j = 0; j != cell->columns; j++)
			max += col[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		જો (max < cell->max_width && flexible_columns) {
			extra = 1 + (cell->max_width - max) / flexible_columns;
			માટે (j = 0; j != cell->columns; j++)
				જો (col[i + j].type != COLUMN_WIDTH_FIXED)
					col[i + j].max += extra;
		}
	}

	માટે (i = 0; i != table->columns; i++) {
		જો (col[i].max < col[i].min) {
			box_dump(stderr, table, 0, true);
			પ્રતિજ્ઞા(0);
		}
		table_min += col[i].min;
		table_max += col[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	જો (css_computed_width_px(table->style, &content->unit_len_ctx,
			-1, &width) == CSS_WIDTH_SET) {
		જો (table_min < width)
			table_min = width;
		જો (table_max < width)
			table_max = width;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, ડાબું_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, જમણું_2, true, true, true,
			&extra_fixed, &extra_frac);
	જો (extra_fixed < 0)
		extra_fixed = 0;
	જો (extra_frac < 0)
		extra_frac = 0;
	જો (1.0 <= extra_frac)
		extra_frac = 0.9;
	table->min_width = (table_min + extra_fixed) / (1.0 - extra_frac);
	table->max_width = (table_max + extra_fixed) / (1.0 - extra_frac);
	table->min_width += (table->columns + 1) * border_spacing_h;
	table->max_width += (table->columns + 1) * border_spacing_h;

	પ્રતિજ્ઞા(0 <= table->min_width && table->min_width <= table->max_width);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
સ્થિર અંતઃરેખ bool box_has_percentage_max_width(સંરચના box *b)
{
	css_unit unit = CSS_UNIT_PX;
	ગણના css_max_width_e type;
	css_fixed કિંમત = 0;

	પ્રતિજ્ઞા(b != ખાલી);

	type = css_computed_max_width(b->style, &કિંમત, &unit);
	પાછા ((type == CSS_MAX_WIDTH_SET) && (unit == CSS_UNIT_PCT));
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
સ્થિર સંરચના box *
layout_minmax_લીટી(સંરચના box *first,
		   પૂર્ણાંક *લીટી_min,
		   પૂર્ણાંક *લીટી_max,
		   bool first_લીટી,
		   bool *લીટી_has_height,
		   અચલ સંરચના gui_layout_table *font_func,
		   અચલ html_content *content)
{
	પૂર્ણાંક min = 0, max = 0, width, height, fixed;
	દશાંશ frac;
	size_t i, j;
	સંરચના box *b;
	સંરચના box *block;
	plot_font_style_t fstyle;
	bool no_wrap;

	પ્રતિજ્ઞા(first->parent);
	પ્રતિજ્ઞા(first->parent->parent);
	પ્રતિજ્ઞા(first->parent->parent->style);

	block = first->parent->parent;
	no_wrap = (css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_PRE);

	*લીટી_has_height = false;

	/* corresponds to the pass 1 loop in layout_line() */
	માટે (b = first; b; b = b->next) {
		ગણના css_width_e wtype;
		ગણના css_height_e htype;
		ગણના css_box_sizing_e bs;
		css_fixed કિંમત = 0;
		css_unit unit = CSS_UNIT_PX;

		પ્રતિજ્ઞા(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		જો (b->type == BOX_BR) {
			b = b->next;
			તોડો;
		}

		જો (lh__box_is_float_box(b)) {
			પ્રતિજ્ઞા(b->children);
			જો (b->children->type == BOX_TABLE)
				layout_minmax_table(b->children, font_func,
						content);
			નહીંતર
				layout_minmax_block(b->children, font_func,
						content);
			b->min_width = b->children->min_width;
			b->max_width = b->children->max_width;
			જો (min < b->min_width)
				min = b->min_width;
			max += b->max_width;
			ચાલુ;
		}

		જો (b->type == BOX_INLINE_BLOCK || b->type == BOX_INLINE_FLEX) {
			layout_minmax_block(b, font_func, content);
			જો (min < b->min_width)
				min = b->min_width;
			max += b->max_width;

			જો (b->flags & HAS_HEIGHT)
				*લીટી_has_height = true;
			ચાલુ;
		}

		પ્રતિજ્ઞા(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		જો (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->style, ડાબું_2, true, true, true,
					&fixed, &frac);
			જો (!b->inline_end)
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, જમણું_2,
						true, true, true,
						&fixed, &frac);
			જો (0 < fixed)
				max += fixed;
			*લીટી_has_height = true;
			/* \todo  update min width, consider fractional extra */
		} નહીંતર જો (b->type == BOX_INLINE_END) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->inline_end->style, જમણું_2,
					true, true, true,
					&fixed, &frac);
			જો (0 < fixed)
				max += fixed;

			જો (b->next) {
				જો (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*લીટી_has_height = true;
			ચાલુ;
		}

		જો (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			જો (!b->text)
				ચાલુ;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			જો (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				જો (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					પૂર્ણાંક opt_maxwidth = 0;
					સંરચના form_option *o;

					માટે (o = b->parent->parent->gadget->
							data.select.વસ્તુઓ; o;
							o = o->next) {
						પૂર્ણાંક opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						જો (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}

					b->width = opt_maxwidth;
					જો (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;

				} નહીંતર {
					font_func->width(&fstyle, b->text,
						b->length, &b->width);
					b->flags |= MEASURED;
				}
			}
			max += b->width;
			જો (b->next) {
				જો (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			જો (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} નહીંતર જો (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				જો (min < b->width)
					min = b->width;

			} નહીંતર જો (b->parent->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				કરો {
					માટે (j = i; j != b->length &&
							b->text[j] != ' '; j++)
						;
					font_func->width(&fstyle, b->text + i,
							 j - i, &width);
					જો (min < width)
						min = width;
					i = j + 1;
				} જ્યારે (j != b->length);
			}

			*લીટી_has_height = true;

			ચાલુ;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		પ્રતિજ્ઞા(b->style);

		bs = css_computed_box_sizing(block->style);

		/* calculate box width */
		wtype = css_computed_width_px(b->style,
				&content->unit_len_ctx, -1, &width);
		જો (wtype == CSS_WIDTH_SET) {
			જો (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, ડાબું_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, જમણું_2,
						false, true, true,
						&fixed, &frac);
				જો (width < fixed) {
					width = fixed;
				}
			}
			જો (width < 0)
				width = 0;
		} નહીંતર {
			width = AUTO;
		}

		/* height */
		htype = css_computed_height(b->style, &કિંમત, &unit);
		જો (htype == CSS_HEIGHT_SET) {
			height = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					કિંમત, unit));
		} નહીંતર {
			height = AUTO;
		}

		જો (b->object || (b->flags & REPLACE_DIM)) {
			જો (b->object) {
				પૂર્ણાંક temp_height = height;
				layout_get_object_dimensions(b,
						&width, &temp_height,
						INT_MIN, INT_MAX,
						INT_MIN, INT_MAX);
			}

			fixed = frac = 0;
			જો (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ડાબું_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, જમણું_2,
						true, false, false,
						&fixed, &frac);
			} નહીંતર {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ડાબું_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, જમણું_2,
						true, true, true,
						&fixed, &frac);
			}
			જો (0 < width + fixed)
				width += fixed;
		} નહીંતર જો (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			જો (width == AUTO)
				width = 400;

			fixed = frac = 0;
			જો (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ડાબું_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, જમણું_2,
						true, false, false,
						&fixed, &frac);
			} નહીંતર {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, ડાબું_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, જમણું_2,
						true, true, true,
						&fixed, &frac);
			}

			જો (0 < width + fixed)
				width += fixed;

		} નહીંતર {
			/* form control with no object */
			જો (width == AUTO)
				width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		જો (min < width && !box_has_percentage_max_width(b))
			min = width;
		જો (width > 0)
			max += width;

		*લીટી_has_height = true;
	}

	જો (first_લીટી) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		પૂર્ણાંક text_indent = layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, 100);
		min = (min + text_indent < 0) ? 0 : min + text_indent;
		max = (max + text_indent < 0) ? 0 : max + text_indent;
	}

	*લીટી_min = min;
	*લીટી_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	પ્રતિજ્ઞા(b != first);
	પ્રતિજ્ઞા(0 <= *લીટી_min);
	પ્રતિજ્ઞા(*લીટી_min <= *લીટી_max);
	પાછા b;
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
સ્થિર શૂન્ય
layout_minmax_inline_container(સંરચના box *inline_container,
			       bool *has_height,
			       અચલ સંરચના gui_layout_table *font_func,
			       અચલ html_content *content)
{
	સંરચના box *child;
	પૂર્ણાંક લીટી_min = 0, લીટી_max = 0;
	પૂર્ણાંક min = 0, max = 0;
	bool first_લીટી = true;
	bool લીટી_has_height;

	પ્રતિજ્ઞા(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	જો (inline_container->max_width != UNKNOWN_MAX_WIDTH)
		પાછા;

	*has_height = false;

	માટે (child = inline_container->children; child; ) {
		child = layout_minmax_લીટી(child, &લીટી_min, &લીટી_max,
				first_લીટી, &લીટી_has_height, font_func,
				content);
		જો (min < લીટી_min)
			min = લીટી_min;
		જો (max < લીટી_max)
			max = લીટી_max;
		first_લીટી = false;
		*has_height |= લીટી_has_height;
	}

	inline_container->min_width = min;
	inline_container->max_width = max;

	પ્રતિજ્ઞા(0 <= inline_container->min_width &&
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
સ્થિર શૂન્ય layout_minmax_block(
		સંરચના box *block,
		અચલ સંરચના gui_layout_table *font_func,
		અચલ html_content *content)
{
	સંરચના box *child;
	પૂર્ણાંક min = 0, max = 0;
	પૂર્ણાંક extra_fixed = 0;
	દશાંશ extra_frac = 0;
	ગણના css_width_e wtype = CSS_WIDTH_AUTO;
	css_fixed width = 0;
	css_unit wunit = CSS_UNIT_PX;
	ગણના css_height_e htype = CSS_HEIGHT_AUTO;
	css_fixed height = 0;
	css_unit hunit = CSS_UNIT_PX;
	ગણના css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool child_has_height = false;

	પ્રતિજ્ઞા(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	/* check if the widths have already been calculated */
	જો (block->max_width != UNKNOWN_MAX_WIDTH)
		પાછા;

	જો (block->style != ખાલી) {
		wtype = css_computed_width(block->style, &width, &wunit);
		htype = css_computed_height(block->style, &height, &hunit);
		bs = css_computed_box_sizing(block->style);
	}

	/* set whether the minimum width is of any interest for this box */
	જો (((block->parent && lh__box_is_float_box(block->parent)) ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX) &&
			wtype != CSS_WIDTH_SET) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} નહીંતર જો (block->type == BOX_TABLE_CELL) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} નહીંતર જો (block->parent && (block->parent->flags & NEED_MIN) &&
			wtype != CSS_WIDTH_SET) {
		/* box inside shrink-to-fit context; need minimum width */
		block->flags |= NEED_MIN;
	} નહીંતર જો (block->parent && (block->parent->type == BOX_FLEX)) {
		/* box is flex item */
		block->flags |= NEED_MIN;
	}

	જો (block->gadget && (block->gadget->type == GADGET_TEXTBOX ||
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

	જો (block->gadget && (block->gadget->type == GADGET_RADIO ||
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

	જો (block->object) {
		જો (content_get_type(block->object) == CONTENT_HTML) {
			layout_minmax_block(html_get_box_વૃક્ષ(block->object),
					font_func, content);
			min = html_get_box_વૃક્ષ(block->object)->min_width;
			max = html_get_box_વૃક્ષ(block->object)->max_width;
		} નહીંતર {
			min = max = content_get_width(block->object);
		}

		block->flags |= HAS_HEIGHT;
	} નહીંતર જો (block->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		block->flags |= HAS_HEIGHT;
	} નહીંતર {
		/* recurse through children */
		માટે (child = block->children; child; child = child->next) {
			પસંદગી (child->type) {
			સ્થિતિ BOX_FLEX:
			સ્થિતિ BOX_BLOCK:
				layout_minmax_block(child, font_func,
						content);
				જો (child->flags & HAS_HEIGHT)
					child_has_height = true;
				તોડો;
			સ્થિતિ BOX_INLINE_CONTAINER:
				જો (block->flags & NEED_MIN)
					child->flags |= NEED_MIN;

				layout_minmax_inline_container(child,
						&child_has_height, font_func,
						content);
				જો (child_has_height &&
						child ==
						child->parent->children) {
					block->flags |= MAKE_HEIGHT;
				}
				તોડો;
			સ્થિતિ BOX_TABLE:
				layout_minmax_table(child, font_func,
						content);
				/* todo: fix for zero height tables */
				child_has_height = true;
				child->flags |= MAKE_HEIGHT;
				તોડો;
			મૂળભૂત:
				પ્રતિજ્ઞા(0);
			}
			પ્રતિજ્ઞા(child->max_width != UNKNOWN_MAX_WIDTH);

			જો (child->style &&
					(css_computed_position(child->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(child->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				ચાલુ;
			}

			જો (lh__box_is_flex_container(block) &&
			    lh__flex_main_is_horizontal(block)) {
				જો (block->style != ખાલી &&
				    css_computed_flex_wrap(block->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += child->min_width;
				} નહીંતર {
					જો (min < child->min_width)
						min = child->min_width;
				}
				max += child->max_width;

			} નહીંતર {
				જો (min < child->min_width)
					min = child->min_width;
				જો (max < child->max_width)
					max = child->max_width;
			}

			જો (child_has_height)
				block->flags |= HAS_HEIGHT;
		}
	}

	જો (max < min) {
		box_dump(stderr, block, 0, true);
		પ્રતિજ્ઞા(0);
	}

	/* fixed width takes priority */
	જો (block->type != BOX_TABLE_CELL && !lh__box_is_flex_item(block)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		ગણના css_max_width_e max_type;
		ગણના css_min_width_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed કિંમત = 0;
		પૂર્ણાંક width;

		જો (css_computed_width_px(block->style, &content->unit_len_ctx,
				-1, &width) == CSS_WIDTH_SET) {
			min = max = width;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_width(block->style, &કિંમત, &unit);
		જો (min_type == CSS_MIN_WIDTH_SET && unit != CSS_UNIT_PCT) {
			પૂર્ણાંક val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, કિંમત, unit));

			જો (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_width(block->style, &કિંમત, &unit);
		જો (max_type == CSS_MAX_WIDTH_SET && unit != CSS_UNIT_PCT) {
			પૂર્ણાંક val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, કિંમત, unit));

			જો (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	જો (htype == CSS_HEIGHT_SET && hunit != CSS_UNIT_PCT &&
			height > INTTOFIX(0)) {
		block->flags |= MAKE_HEIGHT;
		block->flags |= HAS_HEIGHT;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_width(&content->unit_len_ctx, block->style, ડાબું_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, જમણું_2,
			false, true, true, &extra_fixed, &extra_frac);

	જો (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	જો (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	જો (max < min) {
		min = max;
	}

	calculate_mbp_width(&content->unit_len_ctx, block->style, ડાબું_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, જમણું_2,
			true, false, false, &extra_fixed, &extra_frac);

	જો (extra_fixed < 0)
		extra_fixed = 0;
	જો (extra_frac < 0)
		extra_frac = 0;
	જો (1.0 <= extra_frac)
		extra_frac = 0.9;
	જો (block->style != ખાલી &&
			(css_computed_float(block->style) == CSS_FLOAT_ડાબું ||
			css_computed_float(block->style) == CSS_FLOAT_જમણું)) {
		/* floated boxs */
		block->min_width = min + extra_fixed;
		block->max_width = max + extra_fixed;
	} નહીંતર {
		/* not floated */
		block->min_width = (min + extra_fixed) / (1.0 - extra_frac);
		block->max_width = (max + extra_fixed) / (1.0 - extra_frac);
	}

	પ્રતિજ્ઞા(0 <= block->min_width);
	પ્રતિજ્ઞા(block->min_width <= block->max_width);
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
સ્થિર સંરચના box*
layout_next_margin_block(અચલ css_unit_ctx *unit_len_ctx,
			 સંરચના box *box,
			 સંરચના box *block,
			 પૂર્ણાંક viewport_height,
			 પૂર્ણાંક *max_pos_margin,
			 પૂર્ણાંક *max_neg_margin)
{
	પ્રતિજ્ઞા(block != ખાલી);

	જ્યારે (box != ખાલી) {

		જો (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			જો (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						ખાલી, ખાલી, ખાલી, ખાલી,
						ખાલી, ખાલી, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				જો (*max_pos_margin < box->margin[ટોચ_2])
					*max_pos_margin = box->margin[ટોચ_2];
				નહીંતર જો (*max_neg_margin < -box->margin[ટોચ_2])
					*max_neg_margin = -box->margin[ટોચ_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			જો (box->flags & MAKE_HEIGHT ||
					box->border[ટોચ_2].width ||
					box->padding[ટોચ_2] ||
					(box->style &&
					css_computed_overflow_y(box->style) !=
					CSS_OVERFLOW_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_first_child(box))) {
				/* Collapse to this box; return it */
				પાછા box;
			}
		}


		/* Find next box */
		જો (box->type == BOX_BLOCK && !box->object && box->children &&
				box->style &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} નહીંતર {
			જો (!box->next) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				કરો {
					/* Apply bottom margin */
					જો (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					નહીંતર જો (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->parent;
				} જ્યારે (box != block && !box->next);

				જો (box == block) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					પાછા block;
				}
			}

			/* Apply bottom margin */
			જો (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			નહીંતર જો (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->next;

			/* Get margins */
			જો (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						ખાલી, ખાલી, ખાલી, ખાલી,
						ખાલી, ખાલી, box->margin,
						box->padding, box->border);
			}
		}
	}

	પાછા ખાલી;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
સ્થિર પૂર્ણાંક layout_clear(સંરચના box *fl, ગણના css_clear_e clear)
{
	પૂર્ણાંક y = 0;
	માટે (; fl; fl = fl->next_float) {
		જો ((clear == CSS_CLEAR_ડાબું || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ડાબું)
			જો (y < fl->y + fl->height)
				y = fl->y + fl->height;
		જો ((clear == CSS_CLEAR_જમણું || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_જમણું)
			જો (y < fl->y + fl->height)
				y = fl->y + fl->height;
	}
	પાછા y;
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
સ્થિર શૂન્ય
find_sides(સંરચના box *fl,
	   પૂર્ણાંક y0, પૂર્ણાંક y1,
	   પૂર્ણાંક *x0, પૂર્ણાંક *x1,
	   સંરચના box **ડાબું,
	   સંરચના box **જમણું)
{
	પૂર્ણાંક fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*ડાબું = *જમણું = 0;
	માટે (; fl; fl = fl->next_float) {
		fy1 = fl->y + fl->height;
		જો (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			પાછા;
		}
		fy0 = fl->y;
		જો (y0 < fy1 && fy0 <= y1) {
			જો (fl->type == BOX_FLOAT_ડાબું) {
				fx1 = fl->x + fl->width;
				જો (*x0 < fx1) {
					*x0 = fx1;
					*ડાબું = fl;
				}
			} નહીંતર {
				fx0 = fl->x;
				જો (fx0 < *x1) {
					*x1 = fx0;
					*જમણું = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *ડાબું, *જમણું);
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
સ્થિર પૂર્ણાંક
layout_solve_width(સંરચના box *box,
		   પૂર્ણાંક available_width,
		   પૂર્ણાંક width,
		   પૂર્ણાંક lm,
		   પૂર્ણાંક rm,
		   પૂર્ણાંક max_width,
		   પૂર્ણાંક min_width)
{
	bool auto_width = false;

	/* Increase specified left/right margins */
	જો (box->margin[ડાબું_2] != AUTO && box->margin[ડાબું_2] < lm &&
			box->margin[ડાબું_2] >= 0)
		box->margin[ડાબું_2] = lm;
	જો (box->margin[જમણું_2] != AUTO && box->margin[જમણું_2] < rm &&
			box->margin[જમણું_2] >= 0)
		box->margin[જમણું_2] = rm;

	/* Find width */
	જો (width == AUTO) {
		પૂર્ણાંક margin_ડાબું = box->margin[ડાબું_2];
		પૂર્ણાંક margin_જમણું = box->margin[જમણું_2];

		જો (margin_ડાબું == AUTO) {
			margin_ડાબું = lm;
		}
		જો (margin_જમણું == AUTO) {
			margin_જમણું = rm;
		}

		width = available_width -
				(margin_ડાબું + box->border[ડાબું_2].width +
				box->padding[ડાબું_2] + box->padding[જમણું_2] +
				box->border[જમણું_2].width + margin_જમણું);
		width = width < 0 ? 0 : width;
		auto_width = true;
	}

	જો (max_width >= 0 && width > max_width) {
		/* max-width is admissable and width exceeds max-width */
		width = max_width;
		auto_width = false;
	}

	જો (min_width > 0 && width < min_width) {
		/* min-width is admissable and width is less than max-width */
		width = min_width;
		auto_width = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	જો (auto_width) {
		/* any other 'auto' become 0 or the minimum required values */
		જો (box->margin[ડાબું_2] == AUTO) {
			box->margin[ડાબું_2] = lm;
		}
		જો (box->margin[જમણું_2] == AUTO) {
			box->margin[જમણું_2] = rm;
		}
		પાછા width;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	જો (box->margin[ડાબું_2] != AUTO && box->margin[જમણું_2] != AUTO &&
			box->parent != ખાલી && box->parent->style != ખાલી) {
		પસંદગી (css_computed_text_align(box->parent->style)) {
		સ્થિતિ CSS_TEXT_ALIGN_LIBCSS_જમણું:
			box->margin[ડાબું_2] = AUTO;
			box->margin[જમણું_2] = 0;
			તોડો;
		સ્થિતિ CSS_TEXT_ALIGN_LIBCSS_CENTER:
			box->margin[ડાબું_2] = box->margin[જમણું_2] = AUTO;
			તોડો;
		સ્થિતિ CSS_TEXT_ALIGN_LIBCSS_ડાબું:
			box->margin[ડાબું_2] = 0;
			box->margin[જમણું_2] = AUTO;
			તોડો;
		મૂળભૂત:
			/* Leave it alone; no HTML alignment */
			તોડો;
		}
	}

	જો (box->margin[ડાબું_2] == AUTO && box->margin[જમણું_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[ડાબું_2] = box->margin[જમણું_2] =
				(available_width - lm - rm -
				(box->border[ડાબું_2].width + box->padding[ડાબું_2] +
				width + box->padding[જમણું_2] +
				box->border[જમણું_2].width)) / 2;

		જો (box->margin[ડાબું_2] < 0) {
			box->margin[જમણું_2] += box->margin[ડાબું_2];
			box->margin[ડાબું_2] = 0;
		}

		box->margin[ડાબું_2] += lm;

	} નહીંતર જો (box->margin[ડાબું_2] == AUTO) {
		box->margin[ડાબું_2] = available_width - lm -
				(box->border[ડાબું_2].width + box->padding[ડાબું_2] +
				width + box->padding[જમણું_2] +
				box->border[જમણું_2].width + box->margin[જમણું_2]);
		box->margin[ડાબું_2] = box->margin[ડાબું_2] < lm
				? lm : box->margin[ડાબું_2];
	} નહીંતર {
		/* margin-right auto or "over-constrained" */
		box->margin[જમણું_2] = available_width - rm -
				(box->margin[ડાબું_2] + box->border[ડાબું_2].width +
				 box->padding[ડાબું_2] + width +
				 box->padding[જમણું_2] +
				 box->border[જમણું_2].width);
	}

	પાછા width;
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
સ્થિર શૂન્ય
layout_block_find_dimensions(અચલ css_unit_ctx *unit_len_ctx,
			     પૂર્ણાંક available_width,
			     પૂર્ણાંક viewport_height,
			     પૂર્ણાંક lm,
			     પૂર્ણાંક rm,
			     સંરચના box *box)
{
	પૂર્ણાંક width, max_width, min_width;
	પૂર્ણાંક height, max_height, min_height;
	પૂર્ણાંક *margin = box->margin;
	પૂર્ણાંક *padding = box->padding;
	સંરચના box_border *border = box->border;
	અચલ css_computed_style *style = box->style;

	layout_find_dimensions(unit_len_ctx, available_width, viewport_height, box,
			style, &width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	જો (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	}

	box->width = layout_solve_width(box, available_width, width, lm, rm,
			max_width, min_width);
	box->height = height;

	જો (margin[ટોચ_2] == AUTO)
		margin[ટોચ_2] = 0;
	જો (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
સ્થિર શૂન્ય layout_block_add_scrollbar(સંરચના box *box, પૂર્ણાંક which)
{
	ગણના css_overflow_e overflow_x, overflow_y;

	પ્રતિજ્ઞા(box->type == BOX_BLOCK && (which == જમણું_2 || which == BOTTOM));

	જો (box->style == ખાલી)
		પાછા;

	overflow_x = css_computed_overflow_x(box->style);
	overflow_y = css_computed_overflow_y(box->style);

	જો (which == BOTTOM &&
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		જો (box->height != AUTO &&
				(overflow_x == CSS_OVERFLOW_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_WIDTH;
		}

	} નહીંતર જો (which == જમણું_2 &&
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		ગણના css_height_e htype;
		css_fixed height = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_height(box->style, &height, &hunit);

		જો (which == જમણું_2 && box->width != AUTO &&
				htype == CSS_HEIGHT_SET &&
				(overflow_y == CSS_OVERFLOW_SCROLL ||
				box_vscrollbar_present(box))) {
			box->width -= SCROLLBAR_WIDTH;
			box->padding[જમણું_2] += SCROLLBAR_WIDTH;
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
સ્થિર શૂન્ય layout_move_children(સંરચના box *box, પૂર્ણાંક x, પૂર્ણાંક y)
{
	પ્રતિજ્ઞા(box);

	માટે (box = box->children; box; box = box->next) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_table(
		સંરચના box *table,
		પૂર્ણાંક available_width,
		html_content *content)
{
	unsigned પૂર્ણાંક columns = table->columns;  /* total columns */
	unsigned પૂર્ણાંક i;
	unsigned પૂર્ણાંક *row_span;
	પૂર્ણાંક *excess_y;
	પૂર્ણાંક table_width, min_width = 0, max_width = 0;
	પૂર્ણાંક required_width = 0;
	પૂર્ણાંક x, remainder = 0, ગણતરી = 0;
	પૂર્ણાંક table_height = 0;
	પૂર્ણાંક min_height = 0;
	પૂર્ણાંક *xs;  /* array of column x positions */
	પૂર્ણાંક auto_width;
	પૂર્ણાંક spare_width;
	પૂર્ણાંક relative_sum = 0;
	પૂર્ણાંક border_spacing_h = 0, border_spacing_v = 0;
	પૂર્ણાંક spare_height;
	પૂર્ણાંક positioned_columns = 0;
	સંરચના box *containing_block = ખાલી;
	સંરચના box *c;
	સંરચના box *row;
	સંરચના box *row_group;
	સંરચના box **row_span_cell;
	સંરચના column *col;
	અચલ css_computed_style *style = table->style;
	ગણના css_height_e htype;
	css_fixed કિંમત = 0;
	css_unit unit = CSS_UNIT_PX;

	પ્રતિજ્ઞા(table->type == BOX_TABLE);
	પ્રતિજ્ઞા(style);
	પ્રતિજ્ઞા(table->children && table->children->children);
	પ્રતિજ્ઞા(columns);

	/* allocate working buffers */
	col = malloc(columns * કદ col[0]);
	excess_y = malloc(columns * કદ excess_y[0]);
	row_span = malloc(columns * કદ row_span[0]);
	row_span_cell = malloc(columns * કદ row_span_cell[0]);
	xs = malloc((columns + 1) * કદ xs[0]);
	જો (!col || !xs || !row_span || !excess_y || !row_span_cell) {
		મુક્ત(col);
		મુક્ત(excess_y);
		મુક્ત(row_span);
		મુક્ત(row_span_cell);
		મુક્ત(xs);
		પાછા false;
	}

	memcpy(col, table->col, કદ(col[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1, table,
			style, 0, 0, 0, 0, 0, 0, table->margin, table->padding,
			table->border);
	માટે (row_group = table->children; row_group;
			row_group = row_group->next) {
		માટે (row = row_group->children; row; row = row->next) {
			માટે (c = row->children; c; c = c->next) {
				ગણના css_overflow_e overflow_x;
				ગણના css_overflow_e overflow_y;

				પ્રતિજ્ઞા(c->style);
				table_used_border_for_cell(
						&content->unit_len_ctx, c);
				layout_find_dimensions(&content->unit_len_ctx,
						available_width, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				overflow_x = css_computed_overflow_x(c->style);
				overflow_y = css_computed_overflow_y(c->style);

				જો (overflow_x == CSS_OVERFLOW_SCROLL ||
						overflow_x ==
						CSS_OVERFLOW_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_WIDTH;
				}
				જો (overflow_y == CSS_OVERFLOW_SCROLL ||
						overflow_y ==
						CSS_OVERFLOW_AUTO) {
					c->padding[જમણું_2] += SCROLLBAR_WIDTH;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	જો (css_computed_border_collapse(style) ==
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
	જો (css_computed_width_px(style, &content->unit_len_ctx,
			available_width, &table_width) == CSS_WIDTH_SET) {
		/* specified width includes border */
		table_width -= table->border[ડાબું_2].width +
				table->border[જમણું_2].width;
		table_width = table_width < 0 ? 0 : table_width;

		auto_width = table_width;
	} નહીંતર {
		table_width = AUTO;
		auto_width = available_width -
				((table->margin[ડાબું_2] == AUTO ? 0 :
						table->margin[ડાબું_2]) +
				 table->border[ડાબું_2].width +
				 table->padding[ડાબું_2] +
				 table->padding[જમણું_2] +
				 table->border[જમણું_2].width +
				 (table->margin[જમણું_2] == AUTO ? 0 :
						table->margin[જમણું_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_height(style, &કિંમત, &unit);
	જો (htype == CSS_HEIGHT_SET) {
		જો (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			જો (css_computed_position(table->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				પ્રતિજ્ઞા(table->float_container);
				containing_block = table->float_container;
			} નહીંતર જો (table->float_container &&
					css_computed_position(table->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(table->style) ==
						CSS_FLOAT_ડાબું ||
					css_computed_float(table->style) ==
						CSS_FLOAT_જમણું)) {
				/* Table is a float */
				પ્રતિજ્ઞા(table->parent && table->parent->parent &&
						table->parent->parent->parent);
				containing_block =
						table->parent->parent->parent;
			} નહીંતર જો (table->parent && table->parent->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_block = table->parent;
			} નહીંતર જો (table->parent && table->parent->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				પ્રતિજ્ઞા(table->parent->parent);
				containing_block = table->parent->parent;
			}

			જો (containing_block) {
				css_fixed ignored = 0;

				htype = css_computed_height(
						containing_block->style,
						&ignored, &unit);
			}

			જો (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(table->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_HEIGHT_SET)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_height = FPCT_OF_INT_TOINT(કિંમત,
						containing_block->height);
			}
		} નહીંતર {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_height = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					કિંમત, unit));
		}
	}

	/* calculate width required by cells */
	માટે (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      table,
		      i,
		      ((અચલ અક્ષર *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[col[i].type],
		      col[i].width,
		      col[i].min,
		      col[i].max);


		જો (col[i].positioned) {
			positioned_columns++;
			ચાલુ;
		} નહીંતર જો (col[i].type == COLUMN_WIDTH_FIXED) {
			જો (col[i].width < col[i].min)
				col[i].width = col[i].max = col[i].min;
			નહીંતર
				col[i].min = col[i].max = col[i].width;
			required_width += col[i].width;
		} નહીંતર જો (col[i].type == COLUMN_WIDTH_PERCENT) {
			પૂર્ણાંક width = col[i].width * auto_width / 100;
			required_width += col[i].min < width ? width :
					col[i].min;
		} નહીંતર
			required_width += col[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_width);
	}
	required_width += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", table_width,
	      table->min_width, table->max_width, auto_width, required_width);

	જો (auto_width < required_width) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		માટે (i = 0; i != columns; i++) {
			જો (col[i].type == COLUMN_WIDTH_RELATIVE)
				ચાલુ;
			જો (col[i].type == COLUMN_WIDTH_PERCENT) {
				col[i].max = auto_width * col[i].width / 100;
				જો (col[i].max < col[i].min)
					col[i].max = col[i].min;
			}
			min_width += col[i].min;
			max_width += col[i].max;
		}
	} નહીંતર {
		/* take percentages exactly */
		માટે (i = 0; i != columns; i++) {
			જો (col[i].type == COLUMN_WIDTH_RELATIVE)
				ચાલુ;
			જો (col[i].type == COLUMN_WIDTH_PERCENT) {
				પૂર્ણાંક width = auto_width * col[i].width / 100;
				જો (width < col[i].min)
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
	માટે (i = 0; i != columns; i++) {
		જો (col[i].type == COLUMN_WIDTH_RELATIVE)
			relative_sum += col[i].width;
		નહીંતર જો (col[i].type == COLUMN_WIDTH_FIXED)
			spare_width -= col[i].width;
		નહીંતર
			spare_width -= col[i].min;
	}
	spare_width -= (columns + 1) * border_spacing_h;
	જો (relative_sum != 0) {
		જો (spare_width < 0)
			spare_width = 0;
		માટે (i = 0; i != columns; i++) {
			જો (col[i].type == COLUMN_WIDTH_RELATIVE) {
				col[i].min = ceil(col[i].max =
						(દશાંશ) spare_width
						* (દશાંશ) col[i].width
						/ relative_sum);
				min_width += col[i].min;
				max_width += col[i].max;
			}
		}
	}
	min_width += (columns + 1) * border_spacing_h;
	max_width += (columns + 1) * border_spacing_h;

	જો (auto_width <= min_width) {
		/* not enough space: minimise column widths */
		માટે (i = 0; i < columns; i++) {
			col[i].width = col[i].min;
		}
		table_width = min_width;
	} નહીંતર જો (max_width <= auto_width) {
		/* more space than maximum width */
		જો (table_width == AUTO) {
			/* for auto-width tables, make columns max width */
			માટે (i = 0; i < columns; i++) {
				col[i].width = col[i].max;
			}
			table_width = max_width;
		} નહીંતર {
			/* for fixed-width tables, distribute the extra space
			 * too */
			unsigned પૂર્ણાંક flexible_columns = 0;
			માટે (i = 0; i != columns; i++)
				જો (col[i].type != COLUMN_WIDTH_FIXED)
					flexible_columns++;
			જો (flexible_columns == 0) {
				પૂર્ણાંક extra = (table_width - max_width) / columns;
				remainder = (table_width - max_width) -
						(extra * columns);
				માટે (i = 0; i != columns; i++) {
					col[i].width = col[i].max + extra;
					ગણતરી -= remainder;
					જો (ગણતરી < 0) {
						col[i].width++;
						ગણતરી += columns;
					}
				}

			} નહીંતર {
				પૂર્ણાંક extra = (table_width - max_width) /
						flexible_columns;
				remainder = (table_width - max_width) -
						(extra * flexible_columns);
				માટે (i = 0; i != columns; i++)
					જો (col[i].type != COLUMN_WIDTH_FIXED) {
						col[i].width = col[i].max +
								extra;
						ગણતરી -= remainder;
						જો (ગણતરી < 0) {
							col[i].width++;
							ગણતરી += flexible_columns;
						}
					}
			}
		}
	} નહીંતર {
		/* space between min and max: fill it exactly */
		દશાંશ scale = (દશાંશ) (auto_width - min_width) /
				(દશાંશ) (max_width - min_width);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		માટે (i = 0; i < columns; i++) {
			col[i].width = col[i].min + (પૂર્ણાંક) (0.5 +
					(col[i].max - col[i].min) * scale);
		}
		table_width = auto_width;
	}

	xs[0] = x = border_spacing_h;
	માટે (i = 0; i != columns; i++) {
		જો (!col[i].positioned)
			x += col[i].width + border_spacing_h;
		xs[i + 1] = x;
		row_span[i] = 0;
		excess_y[i] = 0;
		row_span_cell[i] = 0;
	}

	/* position cells */
	table_height = border_spacing_v;
	માટે (row_group = table->children; row_group;
			row_group = row_group->next) {
		પૂર્ણાંક row_group_height = 0;
		માટે (row = row_group->children; row; row = row->next) {
			પૂર્ણાંક row_height = 0;

			htype = css_computed_height(row->style, &કિંમત, &unit);
			જો (htype == CSS_HEIGHT_SET && unit != CSS_UNIT_PCT) {
				row_height = FIXTOINT(css_unit_len2device_px(
						row->style,
						&content->unit_len_ctx,
						કિંમત, unit));
			}
			માટે (c = row->children; c; c = c->next) {
				પ્રતિજ્ઞા(c->style);
				c->width = xs[c->start_column + c->columns] -
						xs[c->start_column] -
						border_spacing_h -
						c->border[ડાબું_2].width -
						c->padding[ડાબું_2] -
						c->padding[જમણું_2] -
						c->border[જમણું_2].width;
				c->float_children = 0;
				c->cached_place_below_level = 0;

				c->height = AUTO;
				જો (!layout_block_context(c, -1, content)) {
					મુક્ત(col);
					મુક્ત(excess_y);
					મુક્ત(row_span);
					મુક્ત(row_span_cell);
					મુક્ત(xs);
					પાછા false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->height;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_height(c->style,
						&કિંમત, &unit);

				જો (htype == CSS_HEIGHT_SET &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					પૂર્ણાંક h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							કિંમત, unit));
					જો (c->height < h)
						c->height = h;
				}
				/* specified row height is treated as a minimum
				 */
				જો (c->height < row_height)
					c->height = row_height;
				c->x = xs[c->start_column] +
						c->border[ડાબું_2].width;
				c->y = c->border[ટોચ_2].width;
				માટે (i = 0; i != c->columns; i++) {
					row_span[c->start_column + i] = c->rows;
					excess_y[c->start_column + i] =
							c->border[ટોચ_2].width +
							c->padding[ટોચ_2] +
							c->height +
							c->padding[BOTTOM] +
							c->border[BOTTOM].width;
					row_span_cell[c->start_column + i] = 0;
				}
				row_span_cell[c->start_column] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[ટોચ_2].width -
						c->padding[ટોચ_2] -
						c->height -
						c->border[BOTTOM].width;
			}
			માટે (i = 0; i != columns; i++)
				જો (row_span[i] != 0)
					row_span[i]--;
				નહીંતર
					row_span_cell[i] = 0;
			જો (row->next || row_group->next) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				માટે (i = 0; i != columns; i++)
					જો (row_span[i] == 0 && row_height <
							excess_y[i])
						row_height = excess_y[i];
			} નહીંતર {
				/* except in the last row */
				માટે (i = 0; i != columns; i++)
					જો (row_height < excess_y[i])
						row_height = excess_y[i];
			}
			માટે (i = 0; i != columns; i++) {
				જો (row_height < excess_y[i])
					excess_y[i] -= row_height;
				નહીંતર
					excess_y[i] = 0;
				જો (row_span_cell[i] != 0)
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
	માટે (row_group = table->children; row_group;
			row_group = row_group->next) {
		માટે (row = row_group->children; row; row = row->next) {
			માટે (c = row->children; c; c = c->next) {
				ગણના css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				spare_height = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->height - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &કિંમત, &unit);

				પસંદગી (vertical_align) {
				સ્થિતિ CSS_VERTICAL_ALIGN_SUB:
				સ્થિતિ CSS_VERTICAL_ALIGN_SUPER:
				સ્થિતિ CSS_VERTICAL_ALIGN_TEXT_ટોચ:
				સ્થિતિ CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				સ્થિતિ CSS_VERTICAL_ALIGN_SET:
				સ્થિતિ CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				સ્થિતિ CSS_VERTICAL_ALIGN_ટોચ:
					તોડો;
				સ્થિતિ CSS_VERTICAL_ALIGN_મધ્ય:
					c->padding[ટોચ_2] += spare_height / 2;
					c->padding[BOTTOM] -= spare_height / 2;
					layout_move_children(c, 0,
							spare_height / 2);
					તોડો;
				સ્થિતિ CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[ટોચ_2] += spare_height;
					c->padding[BOTTOM] -= spare_height;
					layout_move_children(c, 0,
							spare_height);
					તોડો;
				સ્થિતિ CSS_VERTICAL_ALIGN_INHERIT:
					પ્રતિજ્ઞા(0);
					તોડો;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	જો (table->margin[ટોચ_2] == AUTO)
		table->margin[ટોચ_2] = 0;
	જો (table->margin[BOTTOM] == AUTO)
		table->margin[BOTTOM] = 0;

	મુક્ત(col);
	મુક્ત(excess_y);
	મુક્ત(row_span);
	મુક્ત(row_span_cell);
	મુક્ત(xs);

	table->width = table_width;
	table->height = table_height;

	પાછા true;
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
સ્થિર bool layout_apply_minmax_height(
		અચલ css_unit_ctx *unit_len_ctx,
		સંરચના box *box,
		સંરચના box *container)
{
	પૂર્ણાંક h;
	સંરચના box *containing_block = ખાલી;
	bool updated = false;

	/* Find containing block for percentage heights */
	જો (box->style != ખાલી && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		પ્રતિજ્ઞા(container);
		containing_block = container;
	} નહીંતર જો (box->float_container && box->style != ખાલી &&
			(css_computed_float(box->style) == CSS_FLOAT_ડાબું ||
			 css_computed_float(box->style) == CSS_FLOAT_જમણું)) {
		/* Box is a float */
		પ્રતિજ્ઞા(box->parent && box->parent->parent &&
				box->parent->parent->parent);
		containing_block = box->parent->parent->parent;
	} નહીંતર જો (box->parent && box->parent->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_block = box->parent;
	} નહીંતર જો (box->parent && box->parent->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		પ્રતિજ્ઞા(box->parent->parent);
		containing_block = box->parent->parent;
	}

	જો (box->style) {
		ગણના css_height_e htype = CSS_HEIGHT_AUTO;
		css_fixed કિંમત = 0;
		css_unit unit = CSS_UNIT_PX;

		જો (containing_block) {
			htype = css_computed_height(containing_block->style,
					&કિંમત, &unit);
		}

		/* max-height */
		જો (css_computed_max_height(box->style, &કિંમત, &unit) ==
				CSS_MAX_HEIGHT_SET) {
			જો (unit == CSS_UNIT_PCT) {
				જો (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(કિંમત,
						containing_block->height);
					જો (h < box->height) {
						box->height = h;
						updated = true;
					}
				}
			} નહીંતર {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						કિંમત, unit));
				જો (h < box->height) {
					box->height = h;
					updated = true;
				}
			}
		}

		/* min-height */
		જો (ns_computed_min_height(box->style, &કિંમત, &unit) ==
				CSS_MIN_HEIGHT_SET) {
			જો (unit == CSS_UNIT_PCT) {
				જો (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(કિંમત,
						containing_block->height);
					જો (h > box->height) {
						box->height = h;
						updated = true;
					}
				}
			} નહીંતર {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						કિંમત, unit));
				જો (h > box->height) {
					box->height = h;
					updated = true;
				}
			}
		}
	}
	પાછા updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
સ્થિર bool layout_block_object(સંરચના box *block)
{
	પ્રતિજ્ઞા(block);
	પ્રતિજ્ઞા(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_TABLE ||
			block->type == BOX_TABLE_CELL);
	પ્રતિજ્ઞા(block->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", block,
	      hlcache_handle_get_url(block->object), block->width);

	જો (content_can_reformat(block->object)) {
		content_reformat(block->object, false, block->width, 1);
	} નહીંતર {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	પાછા true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
સ્થિર શૂન્ય add_float_to_container(સંરચના box *cont, સંરચના box *b)
{
	સંરચના box *box = cont->float_children;
	પૂર્ણાંક b_bottom = b->y + b->height;

	પ્રતિજ્ઞા(b->type == BOX_FLOAT_ડાબું || b->type == BOX_FLOAT_જમણું);

	જો (box == ખાલી) {
		/* No other float children */
		b->next_float = ખાલી;
		cont->float_children = b;
		પાછા;
	} નહીંતર જો (b_bottom >= box->y + box->height) {
		/* Goes at start of list */
		b->next_float = cont->float_children;
		cont->float_children = b;
	} નહીંતર {
		સંરચના box *prev = ખાલી;
		જ્યારે (box != ખાલી && b_bottom < box->y + box->height) {
			prev = box;
			box = box->next_float;
		}
		જો (prev != ખાલી) {
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
સ્થિર bool
layout_text_box_split(html_content *content,
		      plot_font_style_t *fstyle,
		      સંરચના box *split_box,
		      size_t new_length,
		      પૂર્ણાંક new_width)
{
	પૂર્ણાંક space_width = split_box->space;
	સંરચના box *c2;
	અચલ સંરચના gui_layout_table *font_func = content->font_func;
	bool space = (split_box->text[new_length] == ' ');
	પૂર્ણાંક used_length = new_length + (space ? 1 : 0);

	જો ((space && space_width == 0) || space_width == UNKNOWN_WIDTH) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->width(fstyle, " ", 1, &space_width);
	}

	જો (split_box->space == UNKNOWN_WIDTH)
		split_box->space = space_width;
	જો (!space)
		space_width = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, split_box, કદ *c2);
	જો (!c2)
		પાછા false;
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
	જો (c2->next)
		c2->next->prev = c2;
	નહીંતર
		c2->parent->last = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      split_box,
	      split_box->length,
	      (પૂર્ણાંક)split_box->length,
	      split_box->text);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->length,
	      (પૂર્ણાંક)c2->length,
	      c2->text);

	પાછા true;
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
સ્થિર શૂન્ય
layout_float_find_dimensions(
		અચલ css_unit_ctx *unit_len_ctx,
		પૂર્ણાંક available_width,
		અચલ css_computed_style *style,
		સંરચના box *box)
{
	પૂર્ણાંક width, height, max_width, min_width, max_height, min_height;
	પૂર્ણાંક *margin = box->margin;
	પૂર્ણાંક *padding = box->padding;
	સંરચના box_border *border = box->border;
	ગણના css_overflow_e overflow_x = css_computed_overflow_x(style);
	ગણના css_overflow_e overflow_y = css_computed_overflow_y(style);
	પૂર્ણાંક scrollbar_width_x =
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;
	પૂર્ણાંક scrollbar_width_y =
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;

	layout_find_dimensions(unit_len_ctx, available_width, -1, box, style,
			&width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	જો (margin[ડાબું_2] == AUTO)
		margin[ડાબું_2] = 0;
	જો (margin[જમણું_2] == AUTO)
		margin[જમણું_2] = 0;

	જો (box->gadget == ખાલી) {
		padding[જમણું_2] += scrollbar_width_y;
		padding[BOTTOM] += scrollbar_width_x;
	}

	જો (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	} નહીંતર જો (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed size = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		પ્રતિજ્ઞા(box->style);

		જો (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			જો (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			જો (box->gadget->type == GADGET_FILE &&
					height == AUTO) {
				size = FLTTOFIX(1.5);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
		જો (box->gadget->type == GADGET_TEXTAREA) {
			જો (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			જો (height == AUTO) {
				size = INTTOFIX(4);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
	} નહીંતર જો (width == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		width = min(max(box->min_width, available_width),
				box->max_width);

		/* width includes margin, borders and padding */
		જો (width == available_width) {
			width -= box->margin[ડાબું_2] + box->border[ડાબું_2].width +
					box->padding[ડાબું_2] +
					box->padding[જમણું_2] +
					box->border[જમણું_2].width +
					box->margin[જમણું_2];
		} નહીંતર {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			પૂર્ણાંક fixed = 0;
			દશાંશ frac = 0;
			calculate_mbp_width(unit_len_ctx, box->style, ડાબું_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_width(unit_len_ctx, box->style, જમણું_2,
					true, true, true, &fixed, &frac);
			જો (fixed < 0)
				fixed = 0;

			width -= fixed;
		}

		જો (max_width >= 0 && width > max_width) width = max_width;
		જો (min_width >  0 && width < min_width) width = min_width;

	} નહીંતર {
		જો (max_width >= 0 && width > max_width) width = max_width;
		જો (min_width >  0 && width < min_width) width = min_width;
		width -= scrollbar_width_y;
	}

	box->width = width;
	box->height = height;

	જો (margin[ટોચ_2] == AUTO)
		margin[ટોચ_2] = 0;
	જો (margin[BOTTOM] == AUTO)
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
સ્થિર bool layout_float(સંરચના box *b, પૂર્ણાંક width, html_content *content)
{
	પ્રતિજ્ઞા(b->type == BOX_TABLE ||
	       b->type == BOX_BLOCK ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_find_dimensions(&content->unit_len_ctx, width, b->style, b);
	જો (b->type == BOX_TABLE || b->type == BOX_INLINE_FLEX) {
		જો (b->type == BOX_TABLE) {
			જો (!layout_table(b, width, content))
				પાછા false;
		} નહીંતર {
			જો (!layout_flex(b, width, content))
				પાછા false;
		}
		જો (b->margin[ડાબું_2] == AUTO)
			b->margin[ડાબું_2] = 0;
		જો (b->margin[જમણું_2] == AUTO)
			b->margin[જમણું_2] = 0;
		જો (b->margin[ટોચ_2] == AUTO)
			b->margin[ટોચ_2] = 0;
		જો (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} નહીંતર {
		પાછા layout_block_context(b, -1, content);
	}
	પાછા true;
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
સ્થિર શૂન્ય
place_float_below(સંરચના box *c, પૂર્ણાંક width, પૂર્ણાંક cx, પૂર્ણાંક y, સંરચના box *cont)
{
	પૂર્ણાંક x0, x1, yy;
	સંરચના box *ડાબું;
	સંરચના box *જમણું;

	yy = y > cont->cached_place_below_level ?
			y : cont->cached_place_below_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      width, cx, y, cont);

	કરો {
		y = yy;
		x0 = cx;
		x1 = cx + width;
		find_sides(cont->float_children, y, y + c->height, &x0, &x1,
				&ડાબું, &જમણું);
		જો (ડાબું != 0 && જમણું != 0) {
			yy = (ડાબું->y + ડાબું->height <
					જમણું->y + જમણું->height ?
					ડાબું->y + ડાબું->height :
					જમણું->y + જમણું->height);
		} નહીંતર જો (ડાબું == 0 && જમણું != 0) {
			yy = જમણું->y + જમણું->height;
		} નહીંતર જો (ડાબું != 0 && જમણું == 0) {
			yy = ડાબું->y + ડાબું->height;
		}
	} જ્યારે ((ડાબું != 0 || જમણું != 0) && (c->width > x1 - x0));

	જો (c->type == BOX_FLOAT_ડાબું) {
		c->x = x0;
	} નહીંતર {
		c->x = x1 - c->width;
	}
	c->y = y;
	cont->cached_place_below_level = y;
}


/**
 * Calculate line height from a style.
 */
સ્થિર પૂર્ણાંક લીટી_height(
		અચલ css_unit_ctx *unit_len_ctx,
		અચલ css_computed_style *style)
{
	ગણના css_લીટી_height_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed લીટી_height;

	પ્રતિજ્ઞા(style);

	lhtype = css_computed_લીટી_height(style, &lhvalue, &lhunit);
	જો (lhtype == CSS_લીટી_HEIGHT_NORMAL) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_લીટી_HEIGHT_સંખ્યા;
	}

	જો (lhtype == CSS_લીટી_HEIGHT_સંખ્યા ||
			lhunit == CSS_UNIT_PCT) {
		લીટી_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		જો (lhtype != CSS_લીટી_HEIGHT_સંખ્યા)
			લીટી_height = FDIV(લીટી_height, F_100);
	} નહીંતર {
		પ્રતિજ્ઞા(lhunit != CSS_UNIT_PCT);

		લીટી_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	પાછા FIXTOINT(લીટી_height);
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
સ્થિર bool
layout_લીટી(સંરચના box *first,
	    પૂર્ણાંક *width,
	    પૂર્ણાંક *y,
	    પૂર્ણાંક cx,
	    પૂર્ણાંક cy,
	    સંરચના box *cont,
	    bool indent,
	    bool has_text_children,
	    html_content *content,
	    સંરચના box **next_box)
{
	પૂર્ણાંક height, used_height;
	પૂર્ણાંક x0 = 0;
	પૂર્ણાંક x1 = *width;
	પૂર્ણાંક x, h, x_previous;
	પૂર્ણાંક fy = cy;
	સંરચના box *ડાબું;
	સંરચના box *જમણું;
	સંરચના box *b;
	સંરચના box *split_box = 0;
	સંરચના box *d;
	સંરચના box *br_box = 0;
	bool move_y = false;
	bool place_below = false;
	પૂર્ણાંક space_before = 0, space_after = 0;
	unsigned પૂર્ણાંક inline_ગણતરી = 0;
	unsigned પૂર્ણાંક i;
	અચલ સંરચના gui_layout_table *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      first,
	      (પૂર્ણાંક)first->length,
	      first->text,
	      *width,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	find_sides(cont->float_children, cy, cy, &x0, &x1, &ડાબું, &જમણું);
	x0 -= cx;
	x1 -= cx;

	જો (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	જો (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	જો (has_text_children || first->parent->parent->gadget)
		used_height = height = લીટી_height(&content->unit_len_ctx,
				first->parent->parent->style);
	નહીંતર
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		used_height = height = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	માટે (x = 0, b = first; x <= x1 - x0 && b != 0; b = b->next) {
		પૂર્ણાંક min_width, max_width, min_height, max_height;

		પ્રતિજ્ઞા(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		જો (b->type == BOX_BR)
			તોડો;

		જો (lh__box_is_float_box(b))
			ચાલુ;
		જો (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			ચાલુ;

		પ્રતિજ્ઞા(b->style != ખાલી);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		જો (b->type == BOX_INLINE_BLOCK ||
		    b->type == BOX_INLINE_FLEX) {
			જો (b->max_width != UNKNOWN_WIDTH)
				જો (!layout_float(b, *width, content))
					પાછા false;
			h = b->border[ટોચ_2].width + b->padding[ટોચ_2] + b->height +
					b->padding[BOTTOM] +
					b->border[BOTTOM].width;
			જો (height < h)
				height = h;
			x += b->margin[ડાબું_2] + b->border[ડાબું_2].width +
					b->padding[ડાબું_2] + b->width +
					b->padding[જમણું_2] +
					b->border[જમણું_2].width +
					b->margin[જમણું_2];
			space_after = 0;
			ચાલુ;
		}

		જો (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_find_dimensions(&content->unit_len_ctx,
					*width, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			માટે (i = 0; i != 4; i++)
				જો (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[ડાબું_2] + b->border[ડાબું_2].width +
					b->padding[ડાબું_2];
			જો (b->inline_end) {
				b->inline_end->margin[જમણું_2] = b->margin[જમણું_2];
				b->inline_end->padding[જમણું_2] =
						b->padding[જમણું_2];
				b->inline_end->border[જમણું_2] =
						b->border[જમણું_2];
			} નહીંતર {
				x += b->padding[જમણું_2] +
						b->border[જમણું_2].width +
						b->margin[જમણું_2];
			}
		} નહીંતર જો (b->type == BOX_INLINE_END) {
			b->width = 0;
			જો (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[જમણું_2] + b->border[જમણું_2].width +
					b->margin[જમણું_2];
			ચાલુ;
		}

		જો (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->height = લીટી_height(&content->unit_len_ctx,
					b->style ? b->style :
					b->parent->parent->style);
			જો (height < b->height)
				height = b->height;

			જો (!b->text) {
				b->width = 0;
				space_after = 0;
				ચાલુ;
			}

			જો (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				જો (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					પૂર્ણાંક opt_maxwidth = 0;
					સંરચના form_option *o;

					માટે (o = b->parent->parent->gadget->
							data.select.વસ્તુઓ; o;
							o = o->next) {
						પૂર્ણાંક opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						જો (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}
					b->width = opt_maxwidth;
					જો (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;
				} નહીંતર {
					font_func->width(&fstyle, b->text,
							b->length, &b->width);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			જો (b->text && (x + b->width < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->next) {
				font_func->width(&fstyle, b->text,
						 b->length, &b->width);
				b->flags |= MEASURED;
			}

			x += b->width;
			જો (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			ચાલુ;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		પ્રતિજ્ઞા(b->style);

		layout_find_dimensions(&content->unit_len_ctx,
				*width, -1, b, b->style,
				&b->width, &b->height,
				&max_width, &min_width,
				&max_height, &min_height,
				ખાલી, ખાલી, ખાલી);

		જો (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->width, &b->height,
					min_width, max_width,
					min_height, max_height);
		} નહીંતર જો (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			જો (b->width == AUTO)
				b->width = 400;
			જો (b->height == AUTO)
				b->height = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} નહીંતર {
			/* form control with no object */
			જો (b->width == AUTO)
				b->width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			જો (b->height == AUTO)
				b->height = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		જો (b->object && content_can_reformat(b->object) &&
				b->width !=
				content_get_available_width(b->object)) {
			css_fixed કિંમત = 0;
			css_unit unit = CSS_UNIT_PX;
			ગણના css_height_e htype = css_computed_height(b->style,
					&કિંમત, &unit);

			content_reformat(b->object, false, b->width, b->height);

			જો (htype == CSS_HEIGHT_AUTO)
				b->height = content_get_height(b->object);
		}

		જો (height < b->height)
			height = b->height;

		x += b->width;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *width;
	find_sides(cont->float_children, cy, cy + height, &x0, &x1,
			&ડાબું, &જમણું);
	x0 -= cx;
	x1 -= cx;

	જો (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	જો (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	માટે (x = x_previous = 0, b = first; x <= x1 - x0 && b; b = b->next) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		જો (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} નહીંતર જો (lh__box_is_inline_flow(b)) {
			પ્રતિજ્ઞા(b->width != UNKNOWN_WIDTH);

			x_previous = x;
			x += space_after;
			b->x = x;

			જો ((b->type == BOX_INLINE && !b->inline_end) ||
					b->type == BOX_INLINE_BLOCK ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[ડાબું_2] + b->border[ડાબું_2].width;
				x = b->x + b->padding[ડાબું_2] + b->width +
						b->padding[જમણું_2] +
						b->border[જમણું_2].width +
						b->margin[જમણું_2];
			} નહીંતર જો (b->type == BOX_INLINE) {
				b->x += b->margin[ડાબું_2] + b->border[ડાબું_2].width;
				x = b->x + b->padding[ડાબું_2] + b->width;
			} નહીંતર જો (b->type == BOX_INLINE_END) {
				b->height = b->inline_end->height;
				x += b->padding[જમણું_2] +
						b->border[જમણું_2].width +
						b->margin[જમણું_2];
			} નહીંતર {
				x += b->width;
			}

			space_before = space_after;
			જો (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			નહીંતર જો (b->text || b->type == BOX_INLINE_END) {
				જો (b->space == UNKNOWN_WIDTH) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} નહીંતર {
				space_after = 0;
			}
			split_box = b;
			move_y = true;
			inline_ગણતરી++;
		} નહીંતર જો (b->type == BOX_BR) {
			b->x = x;
			b->width = 0;
			br_box = b;
			b = b->next;
			split_box = 0;
			move_y = true;
			તોડો;

		} નહીંતર {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_place_below_level = 0;
			b->float_container = d->float_container = cont;

			જો (!layout_float(d, *width, content))
				પાછા false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[ટોચ_2],
			      d->border[ટોચ_2].width);

			d->x = d->margin[ડાબું_2] + d->border[ડાબું_2].width;
			d->y = d->margin[ટોચ_2] + d->border[ટોચ_2].width;
			b->width = d->margin[ડાબું_2] + d->border[ડાબું_2].width +
					d->padding[ડાબું_2] + d->width +
					d->padding[જમણું_2] +
					d->border[જમણું_2].width +
					d->margin[જમણું_2];
			b->height = d->margin[ટોચ_2] + d->border[ટોચ_2].width +
					d->padding[ટોચ_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];

			જો (b->width > (x1 - x0) - x)
				place_below = true;
			જો (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ડાબું && ડાબું == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_જમણું &&
						જમણું == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						ડાબું == 0 && જમણું == 0)) &&
					(!place_below ||
					(ડાબું == 0 && જમણું == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_place_below_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				જો (b->type == BOX_FLOAT_ડાબું) {
					b->x = cx + x0;
					જો (b->width > 0)
						x0 += b->width;
					ડાબું = b;
				} નહીંતર {
					b->x = cx + x1 - b->width;
					જો (b->width > 0)
						x1 -= b->width;
					જમણું = b;
				}
				b->y = cy;
			} નહીંતર {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				પૂર્ણાંક fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_place_below_level) ?
						fcy :
						cont->cached_place_below_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + height : fy;

				place_float_below(b, *width, cx, fy, cont);
				fy = b->y;
				જો (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ડાબું && ડાબું != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_જમણું &&
						જમણું != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(ડાબું != 0 || જમણું != 0)))) {
					/* to be cleared below existing
					 * floats */
					જો (b->type == BOX_FLOAT_ડાબું)
						b->x = cx;
					નહીંતર
						b->x = cx + *width - b->width;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					જો (fcy > cont->clear_level)
						cont->clear_level = fcy;
					જો (b->y < fcy)
						b->y = fcy;
				}
				જો (b->type == BOX_FLOAT_ડાબું)
					ડાબું = b;
				નહીંતર
					જમણું = b;
			}
			add_float_to_container(cont, b);

			split_box = 0;
		}
	}

	જો (x1 - x0 < x && split_box) {
		/* the last box went over the end */
		size_t split = 0;
		પૂર્ણાંક w;
		bool no_wrap = css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_previous;

		જો (!no_wrap &&
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

		જો (split == 0)
			w = split_box->width;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      split_box,
		      (પૂર્ણાંક)split_box->length,
		      split_box->text,
		      split,
		      w,
		      ડાબું,
		      જમણું,
		      inline_ગણતરી);

		જો ((split == 0 || x1 - x0 <= x + space_before + w) &&
				!ડાબું && !જમણું && inline_ગણતરી == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			જો (split == 0 || split == split_box->length) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = split_box->next;
			} નહીંતર {
				/* cut off first word for this line */
				જો (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					પાછા false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} નહીંતર જો ((split == 0 || x1 - x0 <= x + space_before + w) &&
				inline_ગણતરી == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			પ્રતિજ્ઞા(ડાબું || જમણું);
			used_height = 0;
			જો (ડાબું) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      ડાબું->y,
				      ડાબું->height);

				used_height = ડાબું->y + ડાબું->height - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      used_height);

			}
			જો (જમણું && used_height <
					જમણું->y + જમણું->height - cy + 1)
				used_height = જમણું->y + જમણું->height - cy + 1;

			જો (used_height < 0)
				used_height = 0;

			b = split_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} નહીંતર જો (split == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = split_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} નહીંતર {
			/* fit as many words as possible */
			પ્રતિજ્ઞા(split != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (પૂર્ણાંક)split_box->length, split_box->text,
			      x1 - x0, split, w);

			જો (split != split_box->length) {
				જો (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					પાછા false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	પસંદગી (css_computed_text_align(first->parent->parent->style)) {
	સ્થિતિ CSS_TEXT_ALIGN_જમણું:
	સ્થિતિ CSS_TEXT_ALIGN_LIBCSS_જમણું:
		x0 = x1 - x;
		તોડો;
	સ્થિતિ CSS_TEXT_ALIGN_CENTER:
	સ્થિતિ CSS_TEXT_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		તોડો;
	સ્થિતિ CSS_TEXT_ALIGN_ડાબું:
	સ્થિતિ CSS_TEXT_ALIGN_LIBCSS_ડાબું:
	સ્થિતિ CSS_TEXT_ALIGN_JUSTIFY:
		/* leave on left */
		તોડો;
	સ્થિતિ CSS_TEXT_ALIGN_DEFAULT:
		/* None; consider text direction */
		પસંદગી (css_computed_direction(first->parent->parent->style)) {
		સ્થિતિ CSS_DIRECTION_LTR:
			/* leave on left */
			તોડો;
		સ્થિતિ CSS_DIRECTION_RTL:
			x0 = x1 - x;
			તોડો;
		}
		તોડો;
	}

	માટે (d = first; d != b; d = d->next) {
		d->flags &= ~NEW_લીટી;

		જો (d->type == BOX_INLINE_BLOCK &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			ચાલુ;
		} નહીંતર જો ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[ટોચ_2];

			જો (d->type == BOX_TEXT && d->height > used_height) {
				/* text */
				used_height = d->height;
			}
		} નહીંતર જો ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_BLOCK) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[ટોચ_2].width + d->margin[ટોચ_2];
			h = d->margin[ટોચ_2] + d->border[ટોચ_2].width +
					d->padding[ટોચ_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];
			જો (used_height < h)
				used_height = h;
		}
	}

	first->flags |= NEW_લીટી;

	પ્રતિજ્ઞા(b != first || (move_y && 0 < used_height && (ડાબું || જમણું)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	માટે (d = first; d != b; d = d->next) {
		જો ((d->type == BOX_INLINE && d->inline_end) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			css_fixed કિંમત = 0;
			css_unit unit = CSS_UNIT_PX;
			પસંદગી (css_computed_vertical_align(d->style, &કિંમત,
					&unit)) {
			સ્થિતિ CSS_VERTICAL_ALIGN_SUPER:
			સ્થિતિ CSS_VERTICAL_ALIGN_ટોચ:
			સ્થિતિ CSS_VERTICAL_ALIGN_TEXT_ટોચ:
				/* already at top */
				તોડો;
			સ્થિતિ CSS_VERTICAL_ALIGN_SUB:
			સ્થિતિ CSS_VERTICAL_ALIGN_BOTTOM:
			સ્થિતિ CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				d->y += used_height - d->height;
				તોડો;
			મૂળભૂત:
			સ્થિતિ CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (used_height - d->height);
				તોડો;
			}
		}
	}

	/* handle clearance for br */
	જો (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		પૂર્ણાંક clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		જો (used_height < clear_y - cy)
			used_height = clear_y - cy;
	}

	જો (move_y)
		*y += used_height;
	*next_box = b;
	*width = x; /* return actual width */
	પાછા true;
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
સ્થિર bool layout_inline_container(સંરચના box *inline_container, પૂર્ણાંક width,
		સંરચના box *cont, પૂર્ણાંક cx, પૂર્ણાંક cy, html_content *content)
{
	bool first_લીટી = true;
	bool has_text_children;
	સંરચના box *c, *next;
	પૂર્ણાંક y = 0;
	પૂર્ણાંક curwidth,maxwidth = width;

	પ્રતિજ્ઞા(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      width,
	      cont,
	      cx,
	      cy);


	has_text_children = false;
	માટે (c = inline_container->children; c; c = c->next) {
		bool is_pre = false;

		જો (c->style) {
			ગણના css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_લીટી ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		જો ((lh__box_is_object(c) == false &&
				c->text && (c->length || is_pre)) ||
				c->type == BOX_BR)
			has_text_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	માટે (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->width;
		જો (!layout_લીટી(c, &curwidth, &y, cx, cy + y, cont, first_લીટી,
				has_text_children, content, &next))
			પાછા false;
		maxwidth = max(maxwidth,curwidth);
		c = next;
		first_લીટી = false;
	}

	inline_container->width = maxwidth;
	inline_container->height = y;

	પાછા true;
}


/* Documented in layout_intertnal.h */
bool layout_block_context(
		સંરચના box *block,
		પૂર્ણાંક viewport_height,
		html_content *content)
{
	સંરચના box *box;
	પૂર્ણાંક cx, cy;  /**< current coordinates */
	પૂર્ણાંક max_pos_margin = 0;
	પૂર્ણાંક max_neg_margin = 0;
	પૂર્ણાંક y = 0;
	પૂર્ણાંક lm, rm;
	સંરચના box *margin_collapse = ખાલી;
	bool in_margin = false;
	css_fixed gadget_size;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	પ્રતિજ્ઞા(block->type == BOX_BLOCK ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX);
	પ્રતિજ્ઞા(block->width != UNKNOWN_WIDTH);
	પ્રતિજ્ઞા(block->width != AUTO);

	block->float_children = ખાલી;
	block->cached_place_below_level = 0;
	block->clear_level = 0;

	/* special case if the block contains an object */
	જો (block->object) {
		પૂર્ણાંક temp_width = block->width;
		જો (!layout_block_object(block))
			પાછા false;
		layout_get_object_dimensions(block, &temp_width,
				&block->height, INT_MIN, INT_MAX,
				INT_MIN, INT_MAX);
		પાછા true;
	} નહીંતર જો (block->flags & REPLACE_DIM) {
		પાછા true;
	}

	/* special case if the block contains an radio button or checkbox */
	જો (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_size = INTTOFIX(1);
		જો (block->height == AUTO)
			block->height = FIXTOINT(css_unit_len2device_px(
					block->style,
					&content->unit_len_ctx,
					gadget_size, gadget_unit));
	}

	box = block->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = block->padding[ટોચ_2];
	જો (box)
		box->y = block->padding[ટોચ_2];

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
	જ્યારે (box) {
		ગણના css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
		ગણના css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

		પ્રતિજ્ઞા(box->type == BOX_BLOCK ||
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

		જો (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->parent->padding[ડાબું_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			જાઓ advance_to_next_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		જો (margin_collapse == ખાલી) {
			margin_collapse = layout_next_margin_block(
					&content->unit_len_ctx, box, block,
					viewport_height,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			in_margin = true;
		}

		/* Clearance. */
		y = 0;
		જો (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(block->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		જો (box->style) {
			overflow_x = css_computed_overflow_x(box->style);
			overflow_y = css_computed_overflow_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		જો (box->type == BOX_FLEX ||
		    box->type == BOX_BLOCK ||
		    box->flags & IFRAME) {
			જો (lh__box_is_object(box) == false &&
					box->style &&
					(overflow_x != CSS_OVERFLOW_VISIBLE ||
					 overflow_y != CSS_OVERFLOW_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				પૂર્ણાંક x0, x1, ટોચ;
				સંરચના box *ડાબું, *જમણું;
				ટોચ = cy + max_pos_margin - max_neg_margin;
				ટોચ = (ટોચ > y) ? ટોચ : y;
				x0 = cx;
				x1 = cx + box->parent->width -
						box->parent->padding[ડાબું_2] -
						box->parent->padding[જમણું_2];
				find_sides(block->float_children, ટોચ, ટોચ,
						&x0, &x1, &ડાબું, &જમણું);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->parent->width -
						box->parent->padding[ડાબું_2] -
						box->parent->padding[જમણું_2] -
						x1;
			}
			layout_block_find_dimensions(&content->unit_len_ctx,
					box->parent->width,
					viewport_height, lm, rm, box);
			જો (box->type == BOX_BLOCK && !(box->flags & IFRAME)) {
				layout_block_add_scrollbar(box, જમણું_2);
				layout_block_add_scrollbar(box, BOTTOM);
			}
		} નહીંતર જો (box->type == BOX_TABLE) {
			જો (box->style != ખાલી) {
				ગણના css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style, &width,
						&unit);

				જો (wtype == CSS_WIDTH_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					પૂર્ણાંક x0, x1, ટોચ;
					સંરચના box *ડાબું, *જમણું;
					ટોચ = cy + max_pos_margin -
							max_neg_margin;
					ટોચ = (ટોચ > y) ? ટોચ : y;
					x0 = cx;
					x1 = cx + box->parent->width -
						box->parent->padding[ડાબું_2] -
						box->parent->padding[જમણું_2];
					find_sides(block->float_children,
						ટોચ, ટોચ, &x0, &x1,
						&ડાબું, &જમણું);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->parent->width -
						box->parent->padding[ડાબું_2] -
						box->parent->padding[જમણું_2] -
						x1;
				}
			}
			જો (!layout_table(box, box->parent->width - lm - rm,
					content))
				પાછા false;
			layout_solve_width(box, box->parent->width, box->width,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->parent->padding[ડાબું_2] + box->margin[ડાબું_2] +
				box->border[ડાબું_2].width;
		cx += box->x;

		/* Position box: vertical. */
		જો (box->border[ટોચ_2].width) {
			box->y += box->border[ટોચ_2].width;
			cy += box->border[ટોચ_2].width;
		}

		/* Vertical margin */
		જો (((box->type == BOX_BLOCK && (box->flags & HAS_HEIGHT)) ||
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
		જો (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		જો (box->type == BOX_FLEX ||
				(box->type == BOX_BLOCK && box->style &&
				 (overflow_x != CSS_OVERFLOW_VISIBLE ||
				  overflow_y != CSS_OVERFLOW_VISIBLE))) {

			જો (box->type == BOX_FLEX) {
				જો (!layout_flex(box, box->width, content)) {
					પાછા false;
				}
			} નહીંતર {
				layout_block_context(box,
						viewport_height, content);
			}

			cy += box->padding[ટોચ_2];

			જો (box->height == AUTO) {
				box->height = 0;
				layout_block_add_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->height + box->padding[BOTTOM] +
					box->border[BOTTOM].width;
			y = box->y + box->padding[ટોચ_2] + box->height +
					box->padding[BOTTOM] +
					box->border[BOTTOM].width;

			/* Skip children, because they are done in the new
			 * block context */
			જાઓ advance_to_next_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->width);

		/* Layout (except tables). */
		જો (box->object) {
			જો (!layout_block_object(box))
				પાછા false;

		} નહીંતર જો (box->type == BOX_INLINE_CONTAINER) {
			box->width = box->parent->width;
			જો (!layout_inline_container(box, box->width, block,
					cx, cy, content))
				પાછા false;

		} નહીંતર જો (box->type == BOX_TABLE) {
			/* Move down to avoid floats if necessary. */
			પૂર્ણાંક x0, x1;
			સંરચના box *ડાબું, *જમણું;
			y = cy;
			જ્યારે (1) {
				ગણના css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style,
						&width, &unit);

				x0 = cx;
				x1 = cx + box->parent->width;
				find_sides(block->float_children, y,
						y + box->height,
						&x0, &x1, &ડાબું, &જમણું);
				જો (wtype == CSS_WIDTH_AUTO)
					તોડો;
				જો (box->width <= x1 - x0)
					તોડો;
				જો (!ડાબું && !જમણું)
					તોડો;
				નહીંતર જો (!ડાબું)
					y = જમણું->y + જમણું->height + 1;
				નહીંતર જો (!જમણું)
					y = ડાબું->y + ડાબું->height + 1;
				નહીંતર જો (ડાબું->y + ડાબું->height <
						જમણું->y + જમણું->height)
					y = ડાબું->y + ડાબું->height + 1;
				નહીંતર
					y = જમણું->y + જમણું->height + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		જો (box->type == BOX_BLOCK && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			જો (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = ખાલી;
			}

			y = box->padding[ટોચ_2];
			box = box->children;
			box->y = y;
			cy += y;
			ચાલુ;
		} નહીંતર જો (box->type == BOX_BLOCK || box->object ||
				box->flags & IFRAME)
			cy += box->padding[ટોચ_2];

		જો (box->type == BOX_BLOCK && box->height == AUTO) {
			box->height = 0;
			layout_block_add_scrollbar(box, BOTTOM);
		}

		cy += box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width;
		cx -= box->x;
		y = box->y + box->padding[ટોચ_2] + box->height +
				box->padding[BOTTOM] +
				box->border[BOTTOM].width;

	advance_to_next_box:
		જો (!box->next) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			કરો {
				જો (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = ખાલી;
				}

				/* Apply bottom margin */
				જો (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				નહીંતર જો (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->parent;
				જો (box == block)
					તોડો;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				જો (box->type == BOX_BLOCK &&
						box->flags & MAKE_HEIGHT) {
					margin_collapse = ખાલી;
					in_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				જો (box->height == AUTO) {
					box->height = y - box->padding[ટોચ_2];

					જો (box->type == BOX_BLOCK)
						layout_block_add_scrollbar(box,
								BOTTOM);
				} નહીંતર
					cy += box->height -
							(y - box->padding[ટોચ_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				જો (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_height(
							&content->unit_len_ctx,
							box, ખાલી)) {
					/* Height altered */
					/* Set current cy */
					cy += box->height -
							(y - box->padding[ટોચ_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].width;
				cx -= box->x;
				y = box->y + box->padding[ટોચ_2] + box->height +
						box->padding[BOTTOM] +
						box->border[BOTTOM].width;

			} જ્યારે (box->next == ખાલી);
			જો (box == block)
				તોડો;
		}

		/* To next sibling. */

		જો (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = ખાલી;
		}

		જો (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		નહીંતર જો (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->next;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	માટે (box = block->float_children; box; box = box->next_float) {
		y = box->y + box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width + box->margin[BOTTOM];
		જો (cy < y)
			cy = y;
	}

	જો (block->height == AUTO) {
		block->height = cy - block->padding[ટોચ_2];
		જો (block->type == BOX_BLOCK)
			layout_block_add_scrollbar(block, BOTTOM);
	}

	જો (block->style && css_computed_position(block->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_height(&content->unit_len_ctx, block, ખાલી);
	}

	જો (block->gadget &&
			(block->gadget->type == GADGET_TEXTAREA ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		પૂર્ણાંક ta_width = block->padding[ડાબું_2] + block->width +
				block->padding[જમણું_2];
		પૂર્ણાંક ta_height = block->padding[ટોચ_2] + block->height +
				block->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				block->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_set_layout(block->gadget->data.text.ta,
				&fstyle, ta_width, ta_height,
				block->padding[ટોચ_2], block->padding[જમણું_2],
				block->padding[BOTTOM], block->padding[ડાબું_2]);
	}

	પાછા true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
સ્થિર bool
layout__get_element_tag(
		અચલ dom_ગાંઠ *ગાંઠ,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_ગાંઠ_type ગાંઠ_type;
	dom_exception exc;

	exc = dom_ગાંઠ_get_ગાંઠ_type(ગાંઠ, &ગાંઠ_type);
	જો (exc != DOM_NO_ERR ||
	    ગાંઠ_type != DOM_ELEMENT_ગાંઠ) {
		પાછા false;
	}

	exc = dom_html_element_get_tag_type(ગાંઠ, &element_type);
	જો (exc != DOM_NO_ERR) {
		પાછા false;
	}

	*type = element_type;
	પાછા true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
સ્થિર અંતઃરેખ bool
layout__check_element_type(
		અચલ dom_ગાંઠ *ગાંઠ,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	જો (!layout__get_element_tag(ગાંઠ, &element_type)) {
		પાછા false;
	}

	પાછા element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
સ્થિર bool
layout_get_li_કિંમત(dom_ગાંઠ *li_ગાંઠ, dom_long *કિંમત_out)
{
	dom_exception exc;
	dom_long કિંમત;
	bool has_કિંમત;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_ગાંઠ,
			corestring_dom_કિંમત,
			&has_કિંમત);
	જો (exc != DOM_NO_ERR || has_કિંમત == false) {
		પાછા false;
	}

	exc = dom_html_li_element_get_કિંમત(
			(dom_html_li_element *)li_ગાંઠ,
			&કિંમત);
	જો (exc != DOM_NO_ERR) {
		પાછા false;
	}

	*કિંમત_out = કિંમત;
	પાછા true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
સ્થિર bool
layout__get_ol_start(dom_ગાંઠ *ol_ગાંઠ, dom_long *start_out)
{
	dom_exception exc;
	dom_long start;
	bool has_start;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_ગાંઠ,
			corestring_dom_start,
			&has_start);
	જો (exc != DOM_NO_ERR || has_start == false) {
		પાછા false;
	}

	exc = dom_html_olist_element_get_start(
			(dom_html_olist_element *)ol_ગાંઠ,
			&start);
	જો (exc != DOM_NO_ERR) {
		પાછા false;
	}

	*start_out = start;
	પાછા true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
સ્થિર bool
layout__get_ol_reversed(dom_ગાંઠ *ol_ગાંઠ)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_ગાંઠ,
			corestring_dom_reversed,
			&has_reversed);
	જો (exc != DOM_NO_ERR) {
		પાછા false;
	}

	પાછા has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
સ્થિર bool
layout_get_list_item_ગણતરી(
		dom_ગાંઠ *list_owner, dom_long *ગણતરી_out)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_ગાંઠ *child;
	પૂર્ણાંક ગણતરી;

	જો (list_owner == ખાલી) {
		પાછા false;
	}

	જો (!layout__get_element_tag(list_owner, &tag_type)) {
		પાછા false;
	}

	જો (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		પાછા false;
	}

	exc = dom_ગાંઠ_get_first_child(list_owner, &child);
	જો (exc != DOM_NO_ERR) {
		પાછા false;
	}

	ગણતરી = 0;
	જ્યારે (child != ખાલી) {
		dom_ગાંઠ *temp_ગાંઠ;

		જો (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			સંરચના box *child_box;
			જો (dom_ગાંઠ_get_user_data(child,
					corestring_dom_ns_ચાવી_box_ગાંઠ_data,
					&child_box) != DOM_NO_ERR) {
				dom_ગાંઠ_unref(child);
				પાછા false;
			}

			જો (child_box != ખાલી &&
			    child_box->list_marker != ખાલી) {
				ગણતરી++;
			}
		}

		exc = dom_ગાંઠ_get_next_sibling(child, &temp_ગાંઠ);
		dom_ગાંઠ_unref(child);
		જો (exc != DOM_NO_ERR) {
			પાછા false;
		}

		child = temp_ગાંઠ;
	}

	*ગણતરી_out = ગણતરી;
	પાછા true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
સ્થિર શૂન્ય
layout_ordered_list_ગણતરી(
		સંરચના box *box)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_ગાંઠ *child;
	પૂર્ણાંક step = 1;
	dom_long next;

	જો (box->ગાંઠ == ખાલી) {
		પાછા;
	}

	જો (!layout__get_element_tag(box->ગાંઠ, &tag_type)) {
		પાછા;
	}

	જો (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		પાછા;
	}

	next = 1;
	જો (tag_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_start = layout__get_ol_start(box->ગાંઠ, &next);
		bool have_reversed = layout__get_ol_reversed(box->ગાંઠ);

		જો (have_reversed) {
			step = -1;
		}

		જો (!have_start && have_reversed) {
			layout_get_list_item_ગણતરી(box->ગાંઠ, &next);
		}
	}

	exc = dom_ગાંઠ_get_first_child(box->ગાંઠ, &child);
	જો (exc != DOM_NO_ERR) {
		પાછા;
	}

	જ્યારે (child != ખાલી) {
		dom_ગાંઠ *temp_ગાંઠ;

		જો (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			સંરચના box *child_box;

			જો (dom_ગાંઠ_get_user_data(child,
					corestring_dom_ns_ચાવી_box_ગાંઠ_data,
					&child_box) != DOM_NO_ERR) {
				dom_ગાંઠ_unref(child);
				પાછા;
			}

			જો (child_box != ખાલી &&
			    child_box->list_marker != ખાલી) {
				dom_long કિંમત;
				સંરચના box *marker = child_box->list_marker;
				જો (layout_get_li_કિંમત(child, &કિંમત)) {
					marker->list_કિંમત = કિંમત;
					next = marker->list_કિંમત;
				} નહીંતર {
					marker->list_કિંમત = next;
				}
				next += step;
			}
		}

		exc = dom_ગાંઠ_get_next_sibling(child, &temp_ગાંઠ);
		dom_ગાંઠ_unref(child);
		જો (exc != DOM_NO_ERR) {
			પાછા;
		}

		child = temp_ગાંઠ;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
સ્થિર શૂન્ય
layout__set_numerical_marker_text(
		અચલ html_content *content,
		સંરચના box *box)
{
	સંરચના box *marker = box->list_marker;
	size_t counter_len;
	css_error css_res;
	ગણના {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_SIZE = 20,
	};

	marker->text = talloc_array(content->bctx, અક્ષર, LIST_MARKER_SIZE);
	જો (marker->text == ખાલી) {
		પાછા;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_કિંમત,
			marker->text, LIST_MARKER_SIZE, &counter_len);
	જો (css_res == CSS_OK) {
		જો (counter_len > LIST_MARKER_SIZE) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->text = talloc_realloc(content->bctx,
					marker->text,
					અક્ષર,
					counter_len);
			જો (marker->text == ખાલી) {
				પાછા;
			}
			css_computed_format_list_style(box->style,
					marker->list_કિંમત, marker->text,
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
સ્થિર bool
layout__list_item_is_numerical(
		અચલ સંરચના box *b)
{
	ગણના css_list_style_type_e t = css_computed_list_style_type(b->style);

	પસંદગી (t) {
	સ્થિતિ CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	સ્થિતિ CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	સ્થિતિ CSS_LIST_STYLE_TYPE_SQUARE: /* Fall through. */
	સ્થિતિ CSS_LIST_STYLE_TYPE_NONE:
		પાછા false;

	મૂળભૂત:
		પાછા true;
	}
}

/**
 * Layout list markers.
 */
સ્થિર શૂન્ય
layout_lists(અચલ html_content *content, સંરચના box *box)
{
	સંરચના box *child;

	layout_ordered_list_ગણતરી(box);

	માટે (child = box->children; child; child = child->next) {
		જો (child->list_marker) {
			સંરચના box *marker = child->list_marker;

			જો (layout__list_item_is_numerical(child)) {
				જો (marker->text == ખાલી) {
					layout__set_numerical_marker_text(
							content, child);
				}
			}
			જો (marker->object) {
				marker->width =
					content_get_width(marker->object);
				marker->x = -marker->width;
				marker->height =
					content_get_height(marker->object);
				marker->y = (લીટી_height(
						&content->unit_len_ctx,
						marker->style) -
						marker->height) / 2;
			} નહીંતર જો (marker->text) {
				જો (marker->width == UNKNOWN_WIDTH) {
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
				marker->height = લીટી_height(
						&content->unit_len_ctx,
						marker->style);
			} નહીંતર {
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
સ્થિર શૂન્ય
layout_compute_offsets(અચલ css_unit_ctx *unit_len_ctx,
		       સંરચના box *box,
		       સંરચના box *containing_block,
		       પૂર્ણાંક *ટોચ,
		       પૂર્ણાંક *જમણું,
		       પૂર્ણાંક *bottom,
		       પૂર્ણાંક *ડાબું)
{
	uint32_t type;
	css_fixed કિંમત = 0;
	css_unit unit = CSS_UNIT_PX;

	પ્રતિજ્ઞા(containing_block->width != UNKNOWN_WIDTH);
	પ્રતિજ્ઞા(containing_block->width != AUTO);
	પ્રતિજ્ઞા(containing_block->height != AUTO);

	/*ડાબું*/
	type = css_computed_ડાબું(box->style, &કિંમત, &unit);
	જો (type == CSS_ડાબું_SET) {
		જો (unit == CSS_UNIT_PCT) {
			*ડાબું = FPCT_OF_INT_TOINT(કિંમત,
					containing_block->width);
		} નહીંતર {
			*ડાબું = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					કિંમત, unit));
		}
	} નહીંતર {
		*ડાબું = AUTO;
	}

	/*જમણું*/
	type = css_computed_જમણું(box->style, &કિંમત, &unit);
	જો (type == CSS_જમણું_SET) {
		જો (unit == CSS_UNIT_PCT) {
			*જમણું = FPCT_OF_INT_TOINT(કિંમત,
					containing_block->width);
		} નહીંતર {
			*જમણું = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					કિંમત, unit));
		}
	} નહીંતર {
		*જમણું = AUTO;
	}

	/*ટોચ*/
	type = css_computed_ટોચ(box->style, &કિંમત, &unit);
	જો (type == CSS_ટોચ_SET) {
		જો (unit == CSS_UNIT_PCT) {
			*ટોચ = FPCT_OF_INT_TOINT(કિંમત,
					containing_block->height);
		} નહીંતર {
			*ટોચ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					કિંમત, unit));
		}
	} નહીંતર {
		*ટોચ = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &કિંમત, &unit);
	જો (type == CSS_BOTTOM_SET) {
		જો (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_OF_INT_TOINT(કિંમત,
					containing_block->height);
		} નહીંતર {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					કિંમત, unit));
		}
	} નહીંતર {
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
સ્થિર bool
layout_absolute(સંરચના box *box,
		સંરચના box *containing_block,
		પૂર્ણાંક cx, પૂર્ણાંક cy,
		html_content *content)
{
	પૂર્ણાંક static_ડાબું, static_ટોચ;  /* static position */
	પૂર્ણાંક ટોચ, જમણું, bottom, ડાબું;
	પૂર્ણાંક width, height, max_width, min_width;
	પૂર્ણાંક *margin = box->margin;
	પૂર્ણાંક *padding = box->padding;
	સંરચના box_border *border = box->border;
	પૂર્ણાંક available_width = containing_block->width;
	પૂર્ણાંક space;

	પ્રતિજ્ઞા(box->type == BOX_BLOCK || box->type == BOX_TABLE ||
			box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_ડાબું = cx + box->x;
	static_ટોચ = cy + box->y;

	જો (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_block->width += containing_block->padding[ડાબું_2] +
				containing_block->padding[જમણું_2];
		containing_block->height += containing_block->padding[ટોચ_2] +
				containing_block->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_block,
			&ટોચ, &જમણું, &bottom, &ડાબું);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_block;
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1,
			box, box->style, &width, &height,
			&max_width, &min_width, 0, 0,
			margin, padding, border);
	box->float_container = ખાલી;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ડાબું, margin[ડાબું_2], border[ડાબું_2].width, padding[ડાબું_2], width,
	      padding[જમણું_2], border[જમણું_2].width, margin[જમણું_2], જમણું,
	      containing_block->width);


	જો (ડાબું == AUTO && width == AUTO && જમણું == AUTO) {
		જો (margin[ડાબું_2] == AUTO)
			margin[ડાબું_2] = 0;
		જો (margin[જમણું_2] == AUTO)
			margin[જમણું_2] = 0;
		ડાબું = static_ડાબું;

		width = min(max(box->min_width, available_width),
			box->max_width);
		width -= box->margin[ડાબું_2] + box->border[ડાબું_2].width +
			box->padding[ડાબું_2] + box->padding[જમણું_2] +
			box->border[જમણું_2].width + box->margin[જમણું_2];

		/* Adjust for {min|max}-width */
		જો (max_width >= 0 && width > max_width) width = max_width;
		જો (width < min_width) width = min_width;

		જમણું = containing_block->width -
			ડાબું -
			margin[ડાબું_2] - border[ડાબું_2].width - padding[ડાબું_2] -
			width -
			padding[જમણું_2] - border[જમણું_2].width - margin[જમણું_2];
	} નહીંતર જો (ડાબું != AUTO && width != AUTO && જમણું != AUTO) {

		/* Adjust for {min|max}-width */
		જો (max_width >= 0 && width > max_width) width = max_width;
		જો (min_width >  0 && width < min_width) width = min_width;

		જો (margin[ડાબું_2] == AUTO && margin[જમણું_2] == AUTO) {
			space = containing_block->width -
					ડાબું - border[ડાબું_2].width -
					padding[ડાબું_2] - width - padding[જમણું_2] -
					border[જમણું_2].width - જમણું;
			જો (space < 0) {
				margin[ડાબું_2] = 0;
				margin[જમણું_2] = space;
			} નહીંતર {
				margin[ડાબું_2] = margin[જમણું_2] = space / 2;
			}
		} નહીંતર જો (margin[ડાબું_2] == AUTO) {
			margin[ડાબું_2] = containing_block->width -
					ડાબું - border[ડાબું_2].width -
					padding[ડાબું_2] - width - padding[જમણું_2] -
					border[જમણું_2].width - margin[જમણું_2] -
					જમણું;
		} નહીંતર જો (margin[જમણું_2] == AUTO) {
			margin[જમણું_2] = containing_block->width -
					ડાબું - margin[ડાબું_2] -
					border[ડાબું_2].width -
					padding[ડાબું_2] - width - padding[જમણું_2] -
					border[જમણું_2].width - જમણું;
		} નહીંતર {
			જમણું = containing_block->width -
					ડાબું - margin[ડાબું_2] -
					border[ડાબું_2].width -
					padding[ડાબું_2] - width - padding[જમણું_2] -
					border[જમણું_2].width - margin[જમણું_2];
		}
	} નહીંતર {
		જો (margin[ડાબું_2] == AUTO)
			margin[ડાબું_2] = 0;
		જો (margin[જમણું_2] == AUTO)
			margin[જમણું_2] = 0;

		જો (ડાબું == AUTO && width == AUTO && જમણું != AUTO) {
			available_width -= જમણું;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[ડાબું_2] + box->border[ડાબું_2].width +
				box->padding[ડાબું_2] + box->padding[જમણું_2] +
				box->border[જમણું_2].width + box->margin[જમણું_2];

			/* Adjust for {min|max}-width */
			જો (max_width >= 0 && width > max_width)
				width = max_width;
			જો (width < min_width)
				width = min_width;

			ડાબું = containing_block->width -
					margin[ડાબું_2] - border[ડાબું_2].width -
					padding[ડાબું_2] - width - padding[જમણું_2] -
					border[જમણું_2].width - margin[જમણું_2] -
					જમણું;
		} નહીંતર જો (ડાબું == AUTO && width != AUTO && જમણું == AUTO) {

			/* Adjust for {min|max}-width */
			જો (max_width >= 0 && width > max_width)
				width = max_width;
			જો (min_width >  0 && width < min_width)
				width = min_width;

			ડાબું = static_ડાબું;
			જમણું = containing_block->width -
					ડાબું - margin[ડાબું_2] -
					border[ડાબું_2].width -
					padding[ડાબું_2] - width - padding[જમણું_2] -
					border[જમણું_2].width - margin[જમણું_2];
		} નહીંતર જો (ડાબું != AUTO && width == AUTO && જમણું == AUTO) {
			available_width -= ડાબું;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[ડાબું_2] + box->border[ડાબું_2].width +
				box->padding[ડાબું_2] + box->padding[જમણું_2] +
				box->border[જમણું_2].width + box->margin[જમણું_2];

			/* Adjust for {min|max}-width */
			જો (max_width >= 0 && width > max_width)
				width = max_width;
			જો (width < min_width)
				width = min_width;

			જમણું = containing_block->width -
					ડાબું - margin[ડાબું_2] -
					border[ડાબું_2].width -
					padding[ડાબું_2] - width - padding[જમણું_2] -
					border[જમણું_2].width - margin[જમણું_2];
		} નહીંતર જો (ડાબું == AUTO && width != AUTO && જમણું != AUTO) {

			/* Adjust for {min|max}-width */
			જો (max_width >= 0 && width > max_width)
				width = max_width;
			જો (width < min_width)
				width = min_width;

			ડાબું = containing_block->width -
					margin[ડાબું_2] - border[ડાબું_2].width -
					padding[ડાબું_2] - width - padding[જમણું_2] -
					border[જમણું_2].width - margin[જમણું_2] -
					જમણું;
		} નહીંતર જો (ડાબું != AUTO && width == AUTO && જમણું != AUTO) {
			width = containing_block->width -
					ડાબું - margin[ડાબું_2] -
					border[ડાબું_2].width -
					padding[ડાબું_2] - padding[જમણું_2] -
					border[જમણું_2].width - margin[જમણું_2] -
					જમણું;

			/* Adjust for {min|max}-width */
			જો (max_width >= 0 && width > max_width)
				width = max_width;
			જો (width < min_width)
				width = min_width;

		} નહીંતર જો (ડાબું != AUTO && width != AUTO && જમણું == AUTO) {

			/* Adjust for {min|max}-width */
			જો (max_width >= 0 && width > max_width)
				width = max_width;
			જો (width < min_width)
				width = min_width;

			જમણું = containing_block->width -
					ડાબું - margin[ડાબું_2] -
					border[ડાબું_2].width -
					padding[ડાબું_2] - width - padding[જમણું_2] -
					border[જમણું_2].width - margin[જમણું_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ડાબું, margin[ડાબું_2], border[ડાબું_2].width, padding[ડાબું_2], width,
	      padding[જમણું_2], border[જમણું_2].width, margin[જમણું_2], જમણું,
	      containing_block->width);

	box->x = ડાબું + margin[ડાબું_2] + border[ડાબું_2].width - cx;
	જો (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_block->width -= containing_block->padding[ડાબું_2] +
				containing_block->padding[જમણું_2];
	} નહીંતર {
		/** \todo inline ancestors */
	}
	box->width = width;
	box->height = height;

	જો (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->object || box->flags & IFRAME) {
		જો (!layout_block_context(box, -1, content))
			પાછા false;
	} નહીંતર જો (box->type == BOX_TABLE) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		/* \todo  layout_table considers margins etc. again */
		જો (!layout_table(box, width, content))
			પાછા false;
		box->float_container = ખાલી;
		layout_solve_width(box, box->parent->width, box->width, 0, 0,
				-1, -1);
	} નહીંતર જો (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		જો (!layout_flex(box, width, content))
			પાછા false;
		box->float_container = ખાલી;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ટોચ, margin[ટોચ_2], border[ટોચ_2].width, padding[ટોચ_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	જો (ટોચ == AUTO && height == AUTO && bottom == AUTO) {
		ટોચ = static_ટોચ;
		height = box->height;
		જો (margin[ટોચ_2] == AUTO)
			margin[ટોચ_2] = 0;
		જો (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_block->height -
				ટોચ - margin[ટોચ_2] - border[ટોચ_2].width -
				padding[ટોચ_2] - height - padding[BOTTOM] -
				border[BOTTOM].width - margin[BOTTOM];
	} નહીંતર જો (ટોચ != AUTO && height != AUTO && bottom != AUTO) {
		જો (margin[ટોચ_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_block->height -
					ટોચ - border[ટોચ_2].width - padding[ટોચ_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - bottom;
			margin[ટોચ_2] = margin[BOTTOM] = space / 2;
		} નહીંતર જો (margin[ટોચ_2] == AUTO) {
			margin[ટોચ_2] = containing_block->height -
					ટોચ - border[ટોચ_2].width - padding[ટોચ_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} નહીંતર જો (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_block->height -
					ટોચ - margin[ટોચ_2] - border[ટોચ_2].width -
					padding[ટોચ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					bottom;
		} નહીંતર {
			bottom = containing_block->height -
					ટોચ - margin[ટોચ_2] - border[ટોચ_2].width -
					padding[ટોચ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	} નહીંતર {
		જો (margin[ટોચ_2] == AUTO)
			margin[ટોચ_2] = 0;
		જો (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		જો (ટોચ == AUTO && height == AUTO && bottom != AUTO) {
			height = box->height;
			ટોચ = containing_block->height -
					margin[ટોચ_2] - border[ટોચ_2].width -
					padding[ટોચ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} નહીંતર જો (ટોચ == AUTO && height != AUTO && bottom == AUTO) {
			ટોચ = static_ટોચ;
			bottom = containing_block->height -
					ટોચ - margin[ટોચ_2] - border[ટોચ_2].width -
					padding[ટોચ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} નહીંતર જો (ટોચ != AUTO && height == AUTO && bottom == AUTO) {
			height = box->height;
			bottom = containing_block->height -
					ટોચ - margin[ટોચ_2] - border[ટોચ_2].width -
					padding[ટોચ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} નહીંતર જો (ટોચ == AUTO && height != AUTO && bottom != AUTO) {
			ટોચ = containing_block->height -
					margin[ટોચ_2] - border[ટોચ_2].width -
					padding[ટોચ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} નહીંતર જો (ટોચ != AUTO && height == AUTO && bottom != AUTO) {
			height = containing_block->height -
					ટોચ - margin[ટોચ_2] - border[ટોચ_2].width -
					padding[ટોચ_2] - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} નહીંતર જો (ટોચ != AUTO && height != AUTO && bottom == AUTO) {
			bottom = containing_block->height -
					ટોચ - margin[ટોચ_2] - border[ટોચ_2].width -
					padding[ટોચ_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ટોચ, margin[ટોચ_2], border[ટોચ_2].width, padding[ટોચ_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	box->y = ટોચ + margin[ટોચ_2] + border[ટોચ_2].width - cy;
	જો (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_block->height -= containing_block->padding[ટોચ_2] +
				containing_block->padding[BOTTOM];
	} નહીંતર {
		/** \todo Inline ancestors */
	}
	box->height = height;
	layout_apply_minmax_height(&content->unit_len_ctx, box, containing_block);

	પાછા true;
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
સ્થિર bool
layout_position_absolute(સંરચના box *box,
			 સંરચના box *containing_block,
			 પૂર્ણાંક cx, પૂર્ણાંક cy,
			 html_content *content)
{
	સંરચના box *c;

	માટે (c = box->children; c; c = c->next) {
		જો ((c->type == BOX_BLOCK || c->type == BOX_TABLE ||
				c->type == BOX_INLINE_BLOCK ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			જો (!layout_absolute(c, containing_block,
					cx, cy, content))
				પાછા false;
			જો (!layout_position_absolute(c, c, 0, 0, content))
				પાછા false;
		} નહીંતર જો (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			જો (!layout_position_absolute(c, c, 0, 0, content))
				પાછા false;
		} નહીંતર {
			પૂર્ણાંક px, py;
			જો (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_ડાબું ||
					css_computed_float(c->style) ==
						CSS_FLOAT_જમણું)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				સંરચના box *p;
				px = c->x;
				py = c->y;
				માટે (p = box->parent; p && !p->float_children;
						p = p->parent) {
					px -= p->x;
					py -= p->y;
				}
			} નહીંતર {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			જો (!layout_position_absolute(c, containing_block,
					cx + px, cy + py, content))
				પાછા false;
		}
	}

	પાછા true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
સ્થિર શૂન્ય layout_compute_relative_offset(
		અચલ css_unit_ctx *unit_len_ctx,
		સંરચના box *box,
		પૂર્ણાંક *x,
		પૂર્ણાંક *y)
{
	પૂર્ણાંક ડાબું, જમણું, ટોચ, bottom;
	સંરચના box *containing_block;

	પ્રતિજ્ઞા(box && box->parent && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	જો (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_ડાબું ||
			css_computed_float(box->style) == CSS_FLOAT_જમણું)) {
		containing_block = box->float_container;
	} નહીંતર {
		containing_block = box->parent;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_block,
			&ટોચ, &જમણું, &bottom, &ડાબું);

	જો (ડાબું == AUTO && જમણું == AUTO)
		ડાબું = જમણું = 0;
	નહીંતર જો (ડાબું == AUTO)
		/* left is auto => computed = -right */
		ડાબું = -જમણું;
	નહીંતર જો (જમણું == AUTO)
		/* right is auto => computed = -left */
		જમણું = -ડાબું;
	નહીંતર {
		/* over constrained => examine direction property
		 * of containing block */
		જો (containing_block->style &&
				css_computed_direction(
				containing_block->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			ડાબું = -જમણું;
		} નહીંતર {
			/* assume LTR in all other cases */
			જમણું = -ડાબું;
		}
	}

	પ્રતિજ્ઞા(ડાબું == -જમણું);

	જો (ટોચ == AUTO && bottom == AUTO) {
		ટોચ = bottom = 0;
	} નહીંતર જો (ટોચ == AUTO) {
		ટોચ = -bottom;
	} નહીંતર {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -ટોચ;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", ડાબું,
	      જમણું, ટોચ, bottom);

	*x = ડાબું;
	*y = ટોચ;
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
સ્થિર શૂન્ય
layout_position_relative(
		અચલ css_unit_ctx *unit_len_ctx,
		સંરચના box *મૂળ,
		સંરચના box *fp,
		પૂર્ણાંક fx,
		પૂર્ણાંક fy)
{
	સંરચના box *box; /* for children of "root" */
	સંરચના box *fn;  /* for block formatting context box for children of
			  * "box" */
	સંરચના box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	પૂર્ણાંક x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	પૂર્ણાંક fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	પ્રતિજ્ઞા(મૂળ);

	/* Normal children */
	માટે (box = મૂળ->children; box; box = box->next) {

		જો (box->type == BOX_TEXT)
			ચાલુ;

		/* If relatively positioned, get offsets */
		જો (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_offset(
					unit_len_ctx, box, &x, &y);
		નહીંતર
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		જો (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_ડાબું ||
				css_computed_float(box->style) ==
					CSS_FLOAT_જમણું) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			માટે (fc = fp->float_children; fc; fc = fc->next_float) {
				જો (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		જો (box->float_children) {
			fn = box;
			fnx = fny = 0;
		} નહીંતર {
			fn = fp;
			fnx = fx + x;
			fny = fy + y;
		}

		/* recurse first */
		layout_position_relative(unit_len_ctx, box, fn, fnx, fny);

		/* Ignore things we're not interested in. */
		જો (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			ચાલુ;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		જો (box->type == BOX_INLINE && box->inline_end) {
			સંરચના box *b;
			માટે (b = box->next; b && b != box->inline_end;
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
સ્થિર શૂન્ય
layout_get_box_bbox(
		અચલ css_unit_ctx *unit_len_ctx,
		સંરચના box *box,
		પૂર્ણાંક *desc_x0, પૂર્ણાંક *desc_y0,
		પૂર્ણાંક *desc_x1, પૂર્ણાંક *desc_y1)
{
	*desc_x0 = -box->border[ડાબું_2].width;
	*desc_y0 = -box->border[ટોચ_2].width;
	*desc_x1 = box->padding[ડાબું_2] + box->width + box->padding[જમણું_2] +
			box->border[જમણું_2].width;
	*desc_y1 = box->padding[ટોચ_2] + box->height + box->padding[BOTTOM] +
			box->border[BOTTOM].width;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	જો (box->type == BOX_BLOCK && box->style != ખાલી &&
			css_computed_overflow_y(box->style) ==
					CSS_OVERFLOW_VISIBLE &&
			box->object == ખાલી) {
		css_fixed font_size = 0;
		css_unit font_unit = CSS_UNIT_PT;
		પૂર્ણાંક text_height;

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
સ્થિર શૂન્ય
layout_update_descendant_bbox(
		અચલ css_unit_ctx *unit_len_ctx,
		સંરચના box *box,
		સંરચના box *child,
		પૂર્ણાંક off_x,
		પૂર્ણાંક off_y)
{
	પૂર્ણાંક child_desc_x0, child_desc_y0, child_desc_x1, child_desc_y1;

	/* get coordinates of child relative to box */
	પૂર્ણાંક child_x = child->x - off_x;
	પૂર્ણાંક child_y = child->y - off_y;

	bool html_object = (child->object &&
			content_get_type(child->object) == CONTENT_HTML);

	ગણના css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	ગણના css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

	જો (child->style != ખાલી) {
		overflow_x = css_computed_overflow_x(child->style);
		overflow_y = css_computed_overflow_y(child->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, child,
			&child_desc_x0, &child_desc_y0,
			&child_desc_x1, &child_desc_y1);

	જો (overflow_x == CSS_OVERFLOW_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		child_desc_x0 = child->descendant_x0;
		child_desc_x1 = child->descendant_x1;
	}
	જો (overflow_y == CSS_OVERFLOW_VISIBLE &&
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
	જો (child_desc_x0 < box->descendant_x0)
		box->descendant_x0 = child_desc_x0;
	જો (child_desc_y0 < box->descendant_y0)
		box->descendant_y0 = child_desc_y0;
	જો (box->descendant_x1 < child_desc_x1)
		box->descendant_x1 = child_desc_x1;
	જો (box->descendant_y1 < child_desc_y1)
		box->descendant_y1 = child_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
સ્થિર શૂન્ય layout_calculate_descendant_bboxes(
		અચલ css_unit_ctx *unit_len_ctx,
		સંરચના box *box)
{
	સંરચના box *child;

	પ્રતિજ્ઞા(box->width != UNKNOWN_WIDTH);
	પ્રતિજ્ઞા(box->height != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	જો (box->object && content_get_type(box->object) == CONTENT_HTML) {
		જો (box->descendant_x1 < content_get_width(box->object))
			box->descendant_x1 = content_get_width(box->object);
		જો (box->descendant_y1 < content_get_height(box->object))
			box->descendant_y1 = content_get_height(box->object);
	}

	જો (box->iframe != ખાલી) {
		પૂર્ણાંક x, y;
		box_coords(box, &x, &y);

		browser_window_set_position(box->iframe, x, y);
		browser_window_set_dimensions(box->iframe,
				box->width, box->height);
		browser_window_reformat(box->iframe, true,
				box->width, box->height);
	}

	જો (box->type == BOX_INLINE || box->type == BOX_TEXT)
		પાછા;

	જો (box->type == BOX_INLINE_END) {
		box = box->inline_end;
		માટે (child = box->next; child;
				child = child->next) {
			જો (child->type == BOX_FLOAT_ડાબું ||
					child->type == BOX_FLOAT_જમણું)
				ચાલુ;

			layout_update_descendant_bbox(unit_len_ctx, box, child,
					box->x, box->y);

			જો (child == box->inline_end)
				તોડો;
		}
		પાછા;
	}

	જો (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		પાછા;

	માટે (child = box->children; child; child = child->next) {
		જો (child->type == BOX_FLOAT_ડાબું ||
				child->type == BOX_FLOAT_જમણું)
			ચાલુ;

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		જો (box->style && css_computed_overflow_x(box->style) ==
				CSS_OVERFLOW_HIDDEN &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_HIDDEN)
			ચાલુ;

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	માટે (child = box->float_children; child; child = child->next_float) {
		પ્રતિજ્ઞા(child->type == BOX_FLOAT_ડાબું ||
				child->type == BOX_FLOAT_જમણું);

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	જો (box->list_marker) {
		child = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, પૂર્ણાંક width, પૂર્ણાંક height)
{
	bool ret;
	સંરચના box *doc = content->layout;
	અચલ સંરચના gui_layout_table *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			width, height, nsurl_access(content_get_url(
					&content->base)));

	layout_minmax_block(doc, font_func, content);

	layout_block_find_dimensions(&content->unit_len_ctx,
			width, height, 0, 0, doc);
	doc->x = doc->margin[ડાબું_2] + doc->border[ડાબું_2].width;
	doc->y = doc->margin[ટોચ_2] + doc->border[ટોચ_2].width;
	width -= doc->margin[ડાબું_2] + doc->border[ડાબું_2].width +
			doc->padding[ડાબું_2] + doc->padding[જમણું_2] +
			doc->border[જમણું_2].width + doc->margin[જમણું_2];
	જો (width < 0) {
		width = 0;
	}
	doc->width = width;

	ret = layout_block_context(doc, height, content);

	/* make <html> and <body> fill available height */
	જો (doc->y + doc->padding[ટોચ_2] + doc->height + doc->padding[BOTTOM] +
			doc->border[BOTTOM].width + doc->margin[BOTTOM] <
			height) {
		doc->height = height - (doc->y + doc->padding[ટોચ_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].width +
				doc->margin[BOTTOM]);
		જો (doc->children)
			doc->children->height = doc->height -
					(doc->children->margin[ટોચ_2] +
					 doc->children->border[ટોચ_2].width +
					 doc->children->padding[ટોચ_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].width +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	પાછા ret;
}
