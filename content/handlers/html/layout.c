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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <math.h>
#அந்தர்பாவ <dom/dom.h>

#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/talloc.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "netsurf/inttypes.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "netsurf/browser_window.h"
#அந்தர்பாவ "netsurf/layout.h"
#அந்தர்பாவ "content/content.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "css/utils.h"
#அந்தர்பாவ "desktop/scrollbar.h"
#அந்தர்பாவ "desktop/textarea.h"

#அந்தர்பாவ "html/html.h"
#அந்தர்பாவ "html/html_save.h"
#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_inspect.h"
#அந்தர்பாவ "html/font.h"
#அந்தர்பாவ "html/form_internal.h"
#அந்தர்பாவ "html/layout.h"
#அந்தர்பாவ "html/layout_internal.h"
#அந்தர்பாவ "html/table.h"

/** Array of per-side access functions for computed style margins. */
மாறிலி css_len_func margin_funcs[4] = {
	[உச்சி_2]    = css_computed_margin_உச்சி,
	[வலது_2]  = css_computed_margin_வலது,
	[BOTTOM] = css_computed_margin_bottom,
	[இடது_2]   = css_computed_margin_இடது,
};

/** Array of per-side access functions for computed style paddings. */
மாறிலி css_len_func padding_funcs[4] = {
	[உச்சி_2]    = css_computed_padding_உச்சி,
	[வலது_2]  = css_computed_padding_வலது,
	[BOTTOM] = css_computed_padding_bottom,
	[இடது_2]   = css_computed_padding_இடது,
};

/** Array of per-side access functions for computed style border_widths. */
மாறிலி css_len_func border_width_funcs[4] = {
	[உச்சி_2]    = css_computed_border_உச்சி_width,
	[வலது_2]  = css_computed_border_வலது_width,
	[BOTTOM] = css_computed_border_bottom_width,
	[இடது_2]   = css_computed_border_இடது_width,
};

/** Array of per-side access functions for computed style border styles. */
மாறிலி css_border_style_func border_style_funcs[4] = {
	[உச்சி_2]    = css_computed_border_உச்சி_style,
	[வலது_2]  = css_computed_border_வலது_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[இடது_2]   = css_computed_border_இடது_style,
};

/** Array of per-side access functions for computed style border colors. */
மாறிலி css_border_color_func border_color_funcs[4] = {
	[உச்சி_2]    = css_computed_border_உச்சி_color,
	[வலது_2]  = css_computed_border_வலது_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[இடது_2]   = css_computed_border_இடது_color,
};

/* forward declaration to break cycles */
நிலையான வெறுமை layout_minmax_block(
		கட்டமைப்பு box *block,
		மாறிலி கட்டமைப்பு gui_layout_table *font_func,
		மாறிலி html_content *content);

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
நிலையான வெறுமை
layout_get_object_dimensions(கட்டமைப்பு box *box,
			     முழு *width, முழு *height,
			     முழு min_width, முழு max_width,
			     முழு min_height, முழு max_height)
{
	ப்ரதிஜ்ஞா(box->object != வெற்று);
	ப்ரதிஜ்ஞா(width != வெற்று && height != வெற்று);

	எனில் (*width == AUTO && *height == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		முழு intrinsic_width = content_get_width(box->object);
		முழு intrinsic_height = content_get_height(box->object);

		/* use intrinsic dimensions */
		*width = intrinsic_width;
		*height = intrinsic_height;

		/* Deal with min/max-width first */
		எனில் (min_width >  0 && min_width > *width) {
			*width = min_width;
			scaled = true;
		}
		எனில் (max_width >= 0 && max_width < *width) {
			*width = max_width;
			scaled = true;
		}

		எனில் (scaled && (intrinsic_width != 0)) {
			/* Update height */
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		}

		scaled = false;
		/* Deal with min/max-height */
		எனில் (min_height >  0 && min_height > *height) {
			*height = min_height;
			scaled = true;
		}
		எனில் (max_height >= 0 && max_height < *height) {
			*height = max_height;
			scaled = true;
		}

		எனில் (scaled && (intrinsic_height != 0)) {
			/* Update width */
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		}

	} இல்லையெனில் எனில் (*width == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		முழு intrinsic_width = content_get_width(box->object);
		முழு intrinsic_height = content_get_height(box->object);

		எனில் (intrinsic_height != 0)
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		இல்லையெனில்
			*width = intrinsic_width;

		எனில் (min_width >  0 && min_width > *width)
			*width = min_width;
		எனில் (max_width >= 0 && max_width < *width)
			*width = max_width;

	} இல்லையெனில் எனில் (*height == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		முழு intrinsic_width = content_get_width(box->object);
		முழு intrinsic_height = content_get_height(box->object);

		எனில் (min_width >  0 && min_width > *width)
			*width = min_width;
		எனில் (max_width >= 0 && max_width < *width)
			*width = max_width;

		எனில் (intrinsic_width != 0)
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		இல்லையெனில்
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
நிலையான முழு layout_text_indent(
		மாறிலி css_unit_ctx *unit_len_ctx,
		மாறிலி css_computed_style *style, முழு width)
{
	css_fixed மதிப்பு = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_text_indent(style, &மதிப்பு, &unit);

	எனில் (unit == CSS_UNIT_PCT) {
		திரும்பு FPCT_OF_INT_TOINT(மதிப்பு, width);
	} இல்லையெனில் {
		திரும்பு FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				மதிப்பு, unit));
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
நிலையான வெறுமை layout_minmax_table(கட்டமைப்பு box *table,
		மாறிலி கட்டமைப்பு gui_layout_table *font_func,
		மாறிலி html_content *content)
{
	unsigned முழு i, j;
	முழு width;
	முழு border_spacing_h = 0;
	முழு table_min = 0, table_max = 0;
	முழு extra_fixed = 0;
	தசம extra_frac = 0;
	கட்டமைப்பு column *col;
	கட்டமைப்பு box *row_group, *row, *cell;

	/* check if the widths have already been calculated */
	எனில் (table->max_width != UNKNOWN_MAX_WIDTH)
		திரும்பு;

	எனில் (table_calculate_column_types(&content->unit_len_ctx, table) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		திரும்பு;
	}
	col = table->col;

	/* start with 0 except for fixed-width columns */
	க்கு (i = 0; i != table->columns; i++) {
		எனில் (col[i].type == COLUMN_WIDTH_FIXED)
			col[i].min = col[i].max = col[i].width;
		இல்லையெனில்
			col[i].min = col[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	எனில் (css_computed_border_collapse(table->style) ==
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
	க்கு (row_group = table->children; row_group; row_group =row_group->next)
	க்கு (row = row_group->children; row; row = row->next)
	க்கு (cell = row->children; cell; cell = cell->next) {
		ப்ரதிஜ்ஞா(cell->type == BOX_TABLE_CELL);
		ப்ரதிஜ்ஞா(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		ப்ரதிஜ்ஞா(cell->columns != 0);

		எனில் (cell->columns != 1)
			தொடர்;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		எனில் (col[i].positioned)
			தொடர்;

		/* update column min, max widths using cell widths */
		எனில் (col[i].min < cell->min_width)
			col[i].min = cell->min_width;
		எனில் (col[i].max < cell->max_width)
			col[i].max = cell->max_width;
	}

	/* 2nd pass: cells which span multiple columns */
	க்கு (row_group = table->children; row_group; row_group =row_group->next)
	க்கு (row = row_group->children; row; row = row->next)
	க்கு (cell = row->children; cell; cell = cell->next) {
		unsigned முழு flexible_columns = 0;
		முழு min = 0, max = 0, fixed_width = 0, extra;

		எனில் (cell->columns == 1)
			தொடர்;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		க்கு (j = 0; j != cell->columns; j++) {
			min += col[i + j].min;
			எனில் (col[i + j].type == COLUMN_WIDTH_FIXED)
				fixed_width += col[i + j].width;
			இல்லையெனில்
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		எனில் (min < cell->min_width) {
			எனில் (flexible_columns == 0) {
				extra = 1 + (cell->min_width - min) /
						cell->columns;
				க்கு (j = 0; j != cell->columns; j++) {
					col[i + j].min += extra;
					எனில் (col[i + j].max < col[i + j].min)
						col[i + j].max = col[i + j].min;
				}
			} இல்லையெனில் {
				extra = 1 + (cell->min_width - min) /
						flexible_columns;
				க்கு (j = 0; j != cell->columns; j++) {
					எனில் (col[i + j].type !=
							COLUMN_WIDTH_FIXED) {
						col[i + j].min += extra;
						எனில் (col[i + j].max <
								col[i + j].min)
							col[i + j].max =
								col[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		க்கு (j = 0; j != cell->columns; j++)
			max += col[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		எனில் (max < cell->max_width && flexible_columns) {
			extra = 1 + (cell->max_width - max) / flexible_columns;
			க்கு (j = 0; j != cell->columns; j++)
				எனில் (col[i + j].type != COLUMN_WIDTH_FIXED)
					col[i + j].max += extra;
		}
	}

	க்கு (i = 0; i != table->columns; i++) {
		எனில் (col[i].max < col[i].min) {
			box_dump(stderr, table, 0, true);
			ப்ரதிஜ்ஞா(0);
		}
		table_min += col[i].min;
		table_max += col[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	எனில் (css_computed_width_px(table->style, &content->unit_len_ctx,
			-1, &width) == CSS_WIDTH_SET) {
		எனில் (table_min < width)
			table_min = width;
		எனில் (table_max < width)
			table_max = width;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, இடது_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, வலது_2, true, true, true,
			&extra_fixed, &extra_frac);
	எனில் (extra_fixed < 0)
		extra_fixed = 0;
	எனில் (extra_frac < 0)
		extra_frac = 0;
	எனில் (1.0 <= extra_frac)
		extra_frac = 0.9;
	table->min_width = (table_min + extra_fixed) / (1.0 - extra_frac);
	table->max_width = (table_max + extra_fixed) / (1.0 - extra_frac);
	table->min_width += (table->columns + 1) * border_spacing_h;
	table->max_width += (table->columns + 1) * border_spacing_h;

	ப்ரதிஜ்ஞா(0 <= table->min_width && table->min_width <= table->max_width);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
நிலையான உள்வரி bool box_has_percentage_max_width(கட்டமைப்பு box *b)
{
	css_unit unit = CSS_UNIT_PX;
	எண்ணிக்கை css_max_width_e type;
	css_fixed மதிப்பு = 0;

	ப்ரதிஜ்ஞா(b != வெற்று);

	type = css_computed_max_width(b->style, &மதிப்பு, &unit);
	திரும்பு ((type == CSS_MAX_WIDTH_SET) && (unit == CSS_UNIT_PCT));
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
நிலையான கட்டமைப்பு box *
layout_minmax_வரி(கட்டமைப்பு box *first,
		   முழு *வரி_min,
		   முழு *வரி_max,
		   bool first_வரி,
		   bool *வரி_has_height,
		   மாறிலி கட்டமைப்பு gui_layout_table *font_func,
		   மாறிலி html_content *content)
{
	முழு min = 0, max = 0, width, height, fixed;
	தசம frac;
	size_t i, j;
	கட்டமைப்பு box *b;
	கட்டமைப்பு box *block;
	plot_font_style_t fstyle;
	bool no_wrap;

	ப்ரதிஜ்ஞா(first->parent);
	ப்ரதிஜ்ஞா(first->parent->parent);
	ப்ரதிஜ்ஞா(first->parent->parent->style);

	block = first->parent->parent;
	no_wrap = (css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_PRE);

	*வரி_has_height = false;

	/* corresponds to the pass 1 loop in layout_line() */
	க்கு (b = first; b; b = b->next) {
		எண்ணிக்கை css_width_e wtype;
		எண்ணிக்கை css_height_e htype;
		எண்ணிக்கை css_box_sizing_e bs;
		css_fixed மதிப்பு = 0;
		css_unit unit = CSS_UNIT_PX;

		ப்ரதிஜ்ஞா(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		எனில் (b->type == BOX_BR) {
			b = b->next;
			நிறுத்து;
		}

		எனில் (lh__box_is_float_box(b)) {
			ப்ரதிஜ்ஞா(b->children);
			எனில் (b->children->type == BOX_TABLE)
				layout_minmax_table(b->children, font_func,
						content);
			இல்லையெனில்
				layout_minmax_block(b->children, font_func,
						content);
			b->min_width = b->children->min_width;
			b->max_width = b->children->max_width;
			எனில் (min < b->min_width)
				min = b->min_width;
			max += b->max_width;
			தொடர்;
		}

		எனில் (b->type == BOX_INLINE_BLOCK || b->type == BOX_INLINE_FLEX) {
			layout_minmax_block(b, font_func, content);
			எனில் (min < b->min_width)
				min = b->min_width;
			max += b->max_width;

			எனில் (b->flags & HAS_HEIGHT)
				*வரி_has_height = true;
			தொடர்;
		}

		ப்ரதிஜ்ஞா(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		எனில் (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->style, இடது_2, true, true, true,
					&fixed, &frac);
			எனில் (!b->inline_end)
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, வலது_2,
						true, true, true,
						&fixed, &frac);
			எனில் (0 < fixed)
				max += fixed;
			*வரி_has_height = true;
			/* \todo  update min width, consider fractional extra */
		} இல்லையெனில் எனில் (b->type == BOX_INLINE_END) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->inline_end->style, வலது_2,
					true, true, true,
					&fixed, &frac);
			எனில் (0 < fixed)
				max += fixed;

			எனில் (b->next) {
				எனில் (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*வரி_has_height = true;
			தொடர்;
		}

		எனில் (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			எனில் (!b->text)
				தொடர்;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			எனில் (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				எனில் (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					முழு opt_maxwidth = 0;
					கட்டமைப்பு form_option *o;

					க்கு (o = b->parent->parent->gadget->
							data.select.பொருட்கள்; o;
							o = o->next) {
						முழு opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						எனில் (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}

					b->width = opt_maxwidth;
					எனில் (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;

				} இல்லையெனில் {
					font_func->width(&fstyle, b->text,
						b->length, &b->width);
					b->flags |= MEASURED;
				}
			}
			max += b->width;
			எனில் (b->next) {
				எனில் (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			எனில் (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} இல்லையெனில் எனில் (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				எனில் (min < b->width)
					min = b->width;

			} இல்லையெனில் எனில் (b->parent->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				செய் {
					க்கு (j = i; j != b->length &&
							b->text[j] != ' '; j++)
						;
					font_func->width(&fstyle, b->text + i,
							 j - i, &width);
					எனில் (min < width)
						min = width;
					i = j + 1;
				} போது (j != b->length);
			}

			*வரி_has_height = true;

			தொடர்;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		ப்ரதிஜ்ஞா(b->style);

		bs = css_computed_box_sizing(block->style);

		/* calculate box width */
		wtype = css_computed_width_px(b->style,
				&content->unit_len_ctx, -1, &width);
		எனில் (wtype == CSS_WIDTH_SET) {
			எனில் (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, இடது_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, வலது_2,
						false, true, true,
						&fixed, &frac);
				எனில் (width < fixed) {
					width = fixed;
				}
			}
			எனில் (width < 0)
				width = 0;
		} இல்லையெனில் {
			width = AUTO;
		}

		/* height */
		htype = css_computed_height(b->style, &மதிப்பு, &unit);
		எனில் (htype == CSS_HEIGHT_SET) {
			height = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					மதிப்பு, unit));
		} இல்லையெனில் {
			height = AUTO;
		}

		எனில் (b->object || (b->flags & REPLACE_DIM)) {
			எனில் (b->object) {
				முழு temp_height = height;
				layout_get_object_dimensions(b,
						&width, &temp_height,
						INT_MIN, INT_MAX,
						INT_MIN, INT_MAX);
			}

			fixed = frac = 0;
			எனில் (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, இடது_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, வலது_2,
						true, false, false,
						&fixed, &frac);
			} இல்லையெனில் {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, இடது_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, வலது_2,
						true, true, true,
						&fixed, &frac);
			}
			எனில் (0 < width + fixed)
				width += fixed;
		} இல்லையெனில் எனில் (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			எனில் (width == AUTO)
				width = 400;

			fixed = frac = 0;
			எனில் (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, இடது_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, வலது_2,
						true, false, false,
						&fixed, &frac);
			} இல்லையெனில் {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, இடது_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, வலது_2,
						true, true, true,
						&fixed, &frac);
			}

			எனில் (0 < width + fixed)
				width += fixed;

		} இல்லையெனில் {
			/* form control with no object */
			எனில் (width == AUTO)
				width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		எனில் (min < width && !box_has_percentage_max_width(b))
			min = width;
		எனில் (width > 0)
			max += width;

		*வரி_has_height = true;
	}

	எனில் (first_வரி) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		முழு text_indent = layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, 100);
		min = (min + text_indent < 0) ? 0 : min + text_indent;
		max = (max + text_indent < 0) ? 0 : max + text_indent;
	}

	*வரி_min = min;
	*வரி_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	ப்ரதிஜ்ஞா(b != first);
	ப்ரதிஜ்ஞா(0 <= *வரி_min);
	ப்ரதிஜ்ஞா(*வரி_min <= *வரி_max);
	திரும்பு b;
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
நிலையான வெறுமை
layout_minmax_inline_container(கட்டமைப்பு box *inline_container,
			       bool *has_height,
			       மாறிலி கட்டமைப்பு gui_layout_table *font_func,
			       மாறிலி html_content *content)
{
	கட்டமைப்பு box *child;
	முழு வரி_min = 0, வரி_max = 0;
	முழு min = 0, max = 0;
	bool first_வரி = true;
	bool வரி_has_height;

	ப்ரதிஜ்ஞா(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	எனில் (inline_container->max_width != UNKNOWN_MAX_WIDTH)
		திரும்பு;

	*has_height = false;

	க்கு (child = inline_container->children; child; ) {
		child = layout_minmax_வரி(child, &வரி_min, &வரி_max,
				first_வரி, &வரி_has_height, font_func,
				content);
		எனில் (min < வரி_min)
			min = வரி_min;
		எனில் (max < வரி_max)
			max = வரி_max;
		first_வரி = false;
		*has_height |= வரி_has_height;
	}

	inline_container->min_width = min;
	inline_container->max_width = max;

	ப்ரதிஜ்ஞா(0 <= inline_container->min_width &&
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
நிலையான வெறுமை layout_minmax_block(
		கட்டமைப்பு box *block,
		மாறிலி கட்டமைப்பு gui_layout_table *font_func,
		மாறிலி html_content *content)
{
	கட்டமைப்பு box *child;
	முழு min = 0, max = 0;
	முழு extra_fixed = 0;
	தசம extra_frac = 0;
	எண்ணிக்கை css_width_e wtype = CSS_WIDTH_AUTO;
	css_fixed width = 0;
	css_unit wunit = CSS_UNIT_PX;
	எண்ணிக்கை css_height_e htype = CSS_HEIGHT_AUTO;
	css_fixed height = 0;
	css_unit hunit = CSS_UNIT_PX;
	எண்ணிக்கை css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool child_has_height = false;

	ப்ரதிஜ்ஞா(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	/* check if the widths have already been calculated */
	எனில் (block->max_width != UNKNOWN_MAX_WIDTH)
		திரும்பு;

	எனில் (block->style != வெற்று) {
		wtype = css_computed_width(block->style, &width, &wunit);
		htype = css_computed_height(block->style, &height, &hunit);
		bs = css_computed_box_sizing(block->style);
	}

	/* set whether the minimum width is of any interest for this box */
	எனில் (((block->parent && lh__box_is_float_box(block->parent)) ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX) &&
			wtype != CSS_WIDTH_SET) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} இல்லையெனில் எனில் (block->type == BOX_TABLE_CELL) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} இல்லையெனில் எனில் (block->parent && (block->parent->flags & NEED_MIN) &&
			wtype != CSS_WIDTH_SET) {
		/* box inside shrink-to-fit context; need minimum width */
		block->flags |= NEED_MIN;
	} இல்லையெனில் எனில் (block->parent && (block->parent->type == BOX_FLEX)) {
		/* box is flex item */
		block->flags |= NEED_MIN;
	}

	எனில் (block->gadget && (block->gadget->type == GADGET_TEXTBOX ||
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

	எனில் (block->gadget && (block->gadget->type == GADGET_RADIO ||
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

	எனில் (block->object) {
		எனில் (content_get_type(block->object) == CONTENT_HTML) {
			layout_minmax_block(html_get_box_மரம்(block->object),
					font_func, content);
			min = html_get_box_மரம்(block->object)->min_width;
			max = html_get_box_மரம்(block->object)->max_width;
		} இல்லையெனில் {
			min = max = content_get_width(block->object);
		}

		block->flags |= HAS_HEIGHT;
	} இல்லையெனில் எனில் (block->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		block->flags |= HAS_HEIGHT;
	} இல்லையெனில் {
		/* recurse through children */
		க்கு (child = block->children; child; child = child->next) {
			தேர்வு (child->type) {
			நிலை BOX_FLEX:
			நிலை BOX_BLOCK:
				layout_minmax_block(child, font_func,
						content);
				எனில் (child->flags & HAS_HEIGHT)
					child_has_height = true;
				நிறுத்து;
			நிலை BOX_INLINE_CONTAINER:
				எனில் (block->flags & NEED_MIN)
					child->flags |= NEED_MIN;

				layout_minmax_inline_container(child,
						&child_has_height, font_func,
						content);
				எனில் (child_has_height &&
						child ==
						child->parent->children) {
					block->flags |= MAKE_HEIGHT;
				}
				நிறுத்து;
			நிலை BOX_TABLE:
				layout_minmax_table(child, font_func,
						content);
				/* todo: fix for zero height tables */
				child_has_height = true;
				child->flags |= MAKE_HEIGHT;
				நிறுத்து;
			இயல்பு:
				ப்ரதிஜ்ஞா(0);
			}
			ப்ரதிஜ்ஞா(child->max_width != UNKNOWN_MAX_WIDTH);

			எனில் (child->style &&
					(css_computed_position(child->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(child->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				தொடர்;
			}

			எனில் (lh__box_is_flex_container(block) &&
			    lh__flex_main_is_horizontal(block)) {
				எனில் (block->style != வெற்று &&
				    css_computed_flex_wrap(block->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += child->min_width;
				} இல்லையெனில் {
					எனில் (min < child->min_width)
						min = child->min_width;
				}
				max += child->max_width;

			} இல்லையெனில் {
				எனில் (min < child->min_width)
					min = child->min_width;
				எனில் (max < child->max_width)
					max = child->max_width;
			}

			எனில் (child_has_height)
				block->flags |= HAS_HEIGHT;
		}
	}

	எனில் (max < min) {
		box_dump(stderr, block, 0, true);
		ப்ரதிஜ்ஞா(0);
	}

	/* fixed width takes priority */
	எனில் (block->type != BOX_TABLE_CELL && !lh__box_is_flex_item(block)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		எண்ணிக்கை css_max_width_e max_type;
		எண்ணிக்கை css_min_width_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed மதிப்பு = 0;
		முழு width;

		எனில் (css_computed_width_px(block->style, &content->unit_len_ctx,
				-1, &width) == CSS_WIDTH_SET) {
			min = max = width;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_width(block->style, &மதிப்பு, &unit);
		எனில் (min_type == CSS_MIN_WIDTH_SET && unit != CSS_UNIT_PCT) {
			முழு val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, மதிப்பு, unit));

			எனில் (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_width(block->style, &மதிப்பு, &unit);
		எனில் (max_type == CSS_MAX_WIDTH_SET && unit != CSS_UNIT_PCT) {
			முழு val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, மதிப்பு, unit));

			எனில் (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	எனில் (htype == CSS_HEIGHT_SET && hunit != CSS_UNIT_PCT &&
			height > INTTOFIX(0)) {
		block->flags |= MAKE_HEIGHT;
		block->flags |= HAS_HEIGHT;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_width(&content->unit_len_ctx, block->style, இடது_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, வலது_2,
			false, true, true, &extra_fixed, &extra_frac);

	எனில் (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	எனில் (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	எனில் (max < min) {
		min = max;
	}

	calculate_mbp_width(&content->unit_len_ctx, block->style, இடது_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, வலது_2,
			true, false, false, &extra_fixed, &extra_frac);

	எனில் (extra_fixed < 0)
		extra_fixed = 0;
	எனில் (extra_frac < 0)
		extra_frac = 0;
	எனில் (1.0 <= extra_frac)
		extra_frac = 0.9;
	எனில் (block->style != வெற்று &&
			(css_computed_float(block->style) == CSS_FLOAT_இடது ||
			css_computed_float(block->style) == CSS_FLOAT_வலது)) {
		/* floated boxs */
		block->min_width = min + extra_fixed;
		block->max_width = max + extra_fixed;
	} இல்லையெனில் {
		/* not floated */
		block->min_width = (min + extra_fixed) / (1.0 - extra_frac);
		block->max_width = (max + extra_fixed) / (1.0 - extra_frac);
	}

	ப்ரதிஜ்ஞா(0 <= block->min_width);
	ப்ரதிஜ்ஞா(block->min_width <= block->max_width);
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
நிலையான கட்டமைப்பு box*
layout_next_margin_block(மாறிலி css_unit_ctx *unit_len_ctx,
			 கட்டமைப்பு box *box,
			 கட்டமைப்பு box *block,
			 முழு viewport_height,
			 முழு *max_pos_margin,
			 முழு *max_neg_margin)
{
	ப்ரதிஜ்ஞா(block != வெற்று);

	போது (box != வெற்று) {

		எனில் (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			எனில் (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						வெற்று, வெற்று, வெற்று, வெற்று,
						வெற்று, வெற்று, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				எனில் (*max_pos_margin < box->margin[உச்சி_2])
					*max_pos_margin = box->margin[உச்சி_2];
				இல்லையெனில் எனில் (*max_neg_margin < -box->margin[உச்சி_2])
					*max_neg_margin = -box->margin[உச்சி_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			எனில் (box->flags & MAKE_HEIGHT ||
					box->border[உச்சி_2].width ||
					box->padding[உச்சி_2] ||
					(box->style &&
					css_computed_overflow_y(box->style) !=
					CSS_OVERFLOW_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_first_child(box))) {
				/* Collapse to this box; return it */
				திரும்பு box;
			}
		}


		/* Find next box */
		எனில் (box->type == BOX_BLOCK && !box->object && box->children &&
				box->style &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} இல்லையெனில் {
			எனில் (!box->next) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				செய் {
					/* Apply bottom margin */
					எனில் (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					இல்லையெனில் எனில் (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->parent;
				} போது (box != block && !box->next);

				எனில் (box == block) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					திரும்பு block;
				}
			}

			/* Apply bottom margin */
			எனில் (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			இல்லையெனில் எனில் (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->next;

			/* Get margins */
			எனில் (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						வெற்று, வெற்று, வெற்று, வெற்று,
						வெற்று, வெற்று, box->margin,
						box->padding, box->border);
			}
		}
	}

	திரும்பு வெற்று;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
நிலையான முழு layout_clear(கட்டமைப்பு box *fl, எண்ணிக்கை css_clear_e clear)
{
	முழு y = 0;
	க்கு (; fl; fl = fl->next_float) {
		எனில் ((clear == CSS_CLEAR_இடது || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_இடது)
			எனில் (y < fl->y + fl->height)
				y = fl->y + fl->height;
		எனில் ((clear == CSS_CLEAR_வலது || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_வலது)
			எனில் (y < fl->y + fl->height)
				y = fl->y + fl->height;
	}
	திரும்பு y;
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
நிலையான வெறுமை
find_sides(கட்டமைப்பு box *fl,
	   முழு y0, முழு y1,
	   முழு *x0, முழு *x1,
	   கட்டமைப்பு box **இடது,
	   கட்டமைப்பு box **வலது)
{
	முழு fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*இடது = *வலது = 0;
	க்கு (; fl; fl = fl->next_float) {
		fy1 = fl->y + fl->height;
		எனில் (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			திரும்பு;
		}
		fy0 = fl->y;
		எனில் (y0 < fy1 && fy0 <= y1) {
			எனில் (fl->type == BOX_FLOAT_இடது) {
				fx1 = fl->x + fl->width;
				எனில் (*x0 < fx1) {
					*x0 = fx1;
					*இடது = fl;
				}
			} இல்லையெனில் {
				fx0 = fl->x;
				எனில் (fx0 < *x1) {
					*x1 = fx0;
					*வலது = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *இடது, *வலது);
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
நிலையான முழு
layout_solve_width(கட்டமைப்பு box *box,
		   முழு available_width,
		   முழு width,
		   முழு lm,
		   முழு rm,
		   முழு max_width,
		   முழு min_width)
{
	bool auto_width = false;

	/* Increase specified left/right margins */
	எனில் (box->margin[இடது_2] != AUTO && box->margin[இடது_2] < lm &&
			box->margin[இடது_2] >= 0)
		box->margin[இடது_2] = lm;
	எனில் (box->margin[வலது_2] != AUTO && box->margin[வலது_2] < rm &&
			box->margin[வலது_2] >= 0)
		box->margin[வலது_2] = rm;

	/* Find width */
	எனில் (width == AUTO) {
		முழு margin_இடது = box->margin[இடது_2];
		முழு margin_வலது = box->margin[வலது_2];

		எனில் (margin_இடது == AUTO) {
			margin_இடது = lm;
		}
		எனில் (margin_வலது == AUTO) {
			margin_வலது = rm;
		}

		width = available_width -
				(margin_இடது + box->border[இடது_2].width +
				box->padding[இடது_2] + box->padding[வலது_2] +
				box->border[வலது_2].width + margin_வலது);
		width = width < 0 ? 0 : width;
		auto_width = true;
	}

	எனில் (max_width >= 0 && width > max_width) {
		/* max-width is admissable and width exceeds max-width */
		width = max_width;
		auto_width = false;
	}

	எனில் (min_width > 0 && width < min_width) {
		/* min-width is admissable and width is less than max-width */
		width = min_width;
		auto_width = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	எனில் (auto_width) {
		/* any other 'auto' become 0 or the minimum required values */
		எனில் (box->margin[இடது_2] == AUTO) {
			box->margin[இடது_2] = lm;
		}
		எனில் (box->margin[வலது_2] == AUTO) {
			box->margin[வலது_2] = rm;
		}
		திரும்பு width;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	எனில் (box->margin[இடது_2] != AUTO && box->margin[வலது_2] != AUTO &&
			box->parent != வெற்று && box->parent->style != வெற்று) {
		தேர்வு (css_computed_text_align(box->parent->style)) {
		நிலை CSS_TEXT_ALIGN_LIBCSS_வலது:
			box->margin[இடது_2] = AUTO;
			box->margin[வலது_2] = 0;
			நிறுத்து;
		நிலை CSS_TEXT_ALIGN_LIBCSS_CENTER:
			box->margin[இடது_2] = box->margin[வலது_2] = AUTO;
			நிறுத்து;
		நிலை CSS_TEXT_ALIGN_LIBCSS_இடது:
			box->margin[இடது_2] = 0;
			box->margin[வலது_2] = AUTO;
			நிறுத்து;
		இயல்பு:
			/* Leave it alone; no HTML alignment */
			நிறுத்து;
		}
	}

	எனில் (box->margin[இடது_2] == AUTO && box->margin[வலது_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[இடது_2] = box->margin[வலது_2] =
				(available_width - lm - rm -
				(box->border[இடது_2].width + box->padding[இடது_2] +
				width + box->padding[வலது_2] +
				box->border[வலது_2].width)) / 2;

		எனில் (box->margin[இடது_2] < 0) {
			box->margin[வலது_2] += box->margin[இடது_2];
			box->margin[இடது_2] = 0;
		}

		box->margin[இடது_2] += lm;

	} இல்லையெனில் எனில் (box->margin[இடது_2] == AUTO) {
		box->margin[இடது_2] = available_width - lm -
				(box->border[இடது_2].width + box->padding[இடது_2] +
				width + box->padding[வலது_2] +
				box->border[வலது_2].width + box->margin[வலது_2]);
		box->margin[இடது_2] = box->margin[இடது_2] < lm
				? lm : box->margin[இடது_2];
	} இல்லையெனில் {
		/* margin-right auto or "over-constrained" */
		box->margin[வலது_2] = available_width - rm -
				(box->margin[இடது_2] + box->border[இடது_2].width +
				 box->padding[இடது_2] + width +
				 box->padding[வலது_2] +
				 box->border[வலது_2].width);
	}

	திரும்பு width;
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
நிலையான வெறுமை
layout_block_find_dimensions(மாறிலி css_unit_ctx *unit_len_ctx,
			     முழு available_width,
			     முழு viewport_height,
			     முழு lm,
			     முழு rm,
			     கட்டமைப்பு box *box)
{
	முழு width, max_width, min_width;
	முழு height, max_height, min_height;
	முழு *margin = box->margin;
	முழு *padding = box->padding;
	கட்டமைப்பு box_border *border = box->border;
	மாறிலி css_computed_style *style = box->style;

	layout_find_dimensions(unit_len_ctx, available_width, viewport_height, box,
			style, &width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	எனில் (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	}

	box->width = layout_solve_width(box, available_width, width, lm, rm,
			max_width, min_width);
	box->height = height;

	எனில் (margin[உச்சி_2] == AUTO)
		margin[உச்சி_2] = 0;
	எனில் (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
நிலையான வெறுமை layout_block_add_scrollbar(கட்டமைப்பு box *box, முழு which)
{
	எண்ணிக்கை css_overflow_e overflow_x, overflow_y;

	ப்ரதிஜ்ஞா(box->type == BOX_BLOCK && (which == வலது_2 || which == BOTTOM));

	எனில் (box->style == வெற்று)
		திரும்பு;

	overflow_x = css_computed_overflow_x(box->style);
	overflow_y = css_computed_overflow_y(box->style);

	எனில் (which == BOTTOM &&
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		எனில் (box->height != AUTO &&
				(overflow_x == CSS_OVERFLOW_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_WIDTH;
		}

	} இல்லையெனில் எனில் (which == வலது_2 &&
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		எண்ணிக்கை css_height_e htype;
		css_fixed height = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_height(box->style, &height, &hunit);

		எனில் (which == வலது_2 && box->width != AUTO &&
				htype == CSS_HEIGHT_SET &&
				(overflow_y == CSS_OVERFLOW_SCROLL ||
				box_vscrollbar_present(box))) {
			box->width -= SCROLLBAR_WIDTH;
			box->padding[வலது_2] += SCROLLBAR_WIDTH;
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
நிலையான வெறுமை layout_move_children(கட்டமைப்பு box *box, முழு x, முழு y)
{
	ப்ரதிஜ்ஞா(box);

	க்கு (box = box->children; box; box = box->next) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_table(
		கட்டமைப்பு box *table,
		முழு available_width,
		html_content *content)
{
	unsigned முழு columns = table->columns;  /* total columns */
	unsigned முழு i;
	unsigned முழு *row_span;
	முழு *excess_y;
	முழு table_width, min_width = 0, max_width = 0;
	முழு required_width = 0;
	முழு x, remainder = 0, மொத்தம் = 0;
	முழு table_height = 0;
	முழு min_height = 0;
	முழு *xs;  /* array of column x positions */
	முழு auto_width;
	முழு spare_width;
	முழு relative_sum = 0;
	முழு border_spacing_h = 0, border_spacing_v = 0;
	முழு spare_height;
	முழு positioned_columns = 0;
	கட்டமைப்பு box *containing_block = வெற்று;
	கட்டமைப்பு box *c;
	கட்டமைப்பு box *row;
	கட்டமைப்பு box *row_group;
	கட்டமைப்பு box **row_span_cell;
	கட்டமைப்பு column *col;
	மாறிலி css_computed_style *style = table->style;
	எண்ணிக்கை css_height_e htype;
	css_fixed மதிப்பு = 0;
	css_unit unit = CSS_UNIT_PX;

	ப்ரதிஜ்ஞா(table->type == BOX_TABLE);
	ப்ரதிஜ்ஞா(style);
	ப்ரதிஜ்ஞா(table->children && table->children->children);
	ப்ரதிஜ்ஞா(columns);

	/* allocate working buffers */
	col = malloc(columns * அளவு col[0]);
	excess_y = malloc(columns * அளவு excess_y[0]);
	row_span = malloc(columns * அளவு row_span[0]);
	row_span_cell = malloc(columns * அளவு row_span_cell[0]);
	xs = malloc((columns + 1) * அளவு xs[0]);
	எனில் (!col || !xs || !row_span || !excess_y || !row_span_cell) {
		விடுவி(col);
		விடுவி(excess_y);
		விடுவி(row_span);
		விடுவி(row_span_cell);
		விடுவி(xs);
		திரும்பு false;
	}

	memcpy(col, table->col, அளவு(col[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1, table,
			style, 0, 0, 0, 0, 0, 0, table->margin, table->padding,
			table->border);
	க்கு (row_group = table->children; row_group;
			row_group = row_group->next) {
		க்கு (row = row_group->children; row; row = row->next) {
			க்கு (c = row->children; c; c = c->next) {
				எண்ணிக்கை css_overflow_e overflow_x;
				எண்ணிக்கை css_overflow_e overflow_y;

				ப்ரதிஜ்ஞா(c->style);
				table_used_border_for_cell(
						&content->unit_len_ctx, c);
				layout_find_dimensions(&content->unit_len_ctx,
						available_width, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				overflow_x = css_computed_overflow_x(c->style);
				overflow_y = css_computed_overflow_y(c->style);

				எனில் (overflow_x == CSS_OVERFLOW_SCROLL ||
						overflow_x ==
						CSS_OVERFLOW_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_WIDTH;
				}
				எனில் (overflow_y == CSS_OVERFLOW_SCROLL ||
						overflow_y ==
						CSS_OVERFLOW_AUTO) {
					c->padding[வலது_2] += SCROLLBAR_WIDTH;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	எனில் (css_computed_border_collapse(style) ==
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
	எனில் (css_computed_width_px(style, &content->unit_len_ctx,
			available_width, &table_width) == CSS_WIDTH_SET) {
		/* specified width includes border */
		table_width -= table->border[இடது_2].width +
				table->border[வலது_2].width;
		table_width = table_width < 0 ? 0 : table_width;

		auto_width = table_width;
	} இல்லையெனில் {
		table_width = AUTO;
		auto_width = available_width -
				((table->margin[இடது_2] == AUTO ? 0 :
						table->margin[இடது_2]) +
				 table->border[இடது_2].width +
				 table->padding[இடது_2] +
				 table->padding[வலது_2] +
				 table->border[வலது_2].width +
				 (table->margin[வலது_2] == AUTO ? 0 :
						table->margin[வலது_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_height(style, &மதிப்பு, &unit);
	எனில் (htype == CSS_HEIGHT_SET) {
		எனில் (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			எனில் (css_computed_position(table->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				ப்ரதிஜ்ஞா(table->float_container);
				containing_block = table->float_container;
			} இல்லையெனில் எனில் (table->float_container &&
					css_computed_position(table->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(table->style) ==
						CSS_FLOAT_இடது ||
					css_computed_float(table->style) ==
						CSS_FLOAT_வலது)) {
				/* Table is a float */
				ப்ரதிஜ்ஞா(table->parent && table->parent->parent &&
						table->parent->parent->parent);
				containing_block =
						table->parent->parent->parent;
			} இல்லையெனில் எனில் (table->parent && table->parent->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_block = table->parent;
			} இல்லையெனில் எனில் (table->parent && table->parent->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				ப்ரதிஜ்ஞா(table->parent->parent);
				containing_block = table->parent->parent;
			}

			எனில் (containing_block) {
				css_fixed ignored = 0;

				htype = css_computed_height(
						containing_block->style,
						&ignored, &unit);
			}

			எனில் (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(table->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_HEIGHT_SET)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_height = FPCT_OF_INT_TOINT(மதிப்பு,
						containing_block->height);
			}
		} இல்லையெனில் {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_height = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					மதிப்பு, unit));
		}
	}

	/* calculate width required by cells */
	க்கு (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      table,
		      i,
		      ((மாறிலி எழுத்து *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[col[i].type],
		      col[i].width,
		      col[i].min,
		      col[i].max);


		எனில் (col[i].positioned) {
			positioned_columns++;
			தொடர்;
		} இல்லையெனில் எனில் (col[i].type == COLUMN_WIDTH_FIXED) {
			எனில் (col[i].width < col[i].min)
				col[i].width = col[i].max = col[i].min;
			இல்லையெனில்
				col[i].min = col[i].max = col[i].width;
			required_width += col[i].width;
		} இல்லையெனில் எனில் (col[i].type == COLUMN_WIDTH_PERCENT) {
			முழு width = col[i].width * auto_width / 100;
			required_width += col[i].min < width ? width :
					col[i].min;
		} இல்லையெனில்
			required_width += col[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_width);
	}
	required_width += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", table_width,
	      table->min_width, table->max_width, auto_width, required_width);

	எனில் (auto_width < required_width) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		க்கு (i = 0; i != columns; i++) {
			எனில் (col[i].type == COLUMN_WIDTH_RELATIVE)
				தொடர்;
			எனில் (col[i].type == COLUMN_WIDTH_PERCENT) {
				col[i].max = auto_width * col[i].width / 100;
				எனில் (col[i].max < col[i].min)
					col[i].max = col[i].min;
			}
			min_width += col[i].min;
			max_width += col[i].max;
		}
	} இல்லையெனில் {
		/* take percentages exactly */
		க்கு (i = 0; i != columns; i++) {
			எனில் (col[i].type == COLUMN_WIDTH_RELATIVE)
				தொடர்;
			எனில் (col[i].type == COLUMN_WIDTH_PERCENT) {
				முழு width = auto_width * col[i].width / 100;
				எனில் (width < col[i].min)
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
	க்கு (i = 0; i != columns; i++) {
		எனில் (col[i].type == COLUMN_WIDTH_RELATIVE)
			relative_sum += col[i].width;
		இல்லையெனில் எனில் (col[i].type == COLUMN_WIDTH_FIXED)
			spare_width -= col[i].width;
		இல்லையெனில்
			spare_width -= col[i].min;
	}
	spare_width -= (columns + 1) * border_spacing_h;
	எனில் (relative_sum != 0) {
		எனில் (spare_width < 0)
			spare_width = 0;
		க்கு (i = 0; i != columns; i++) {
			எனில் (col[i].type == COLUMN_WIDTH_RELATIVE) {
				col[i].min = ceil(col[i].max =
						(தசம) spare_width
						* (தசம) col[i].width
						/ relative_sum);
				min_width += col[i].min;
				max_width += col[i].max;
			}
		}
	}
	min_width += (columns + 1) * border_spacing_h;
	max_width += (columns + 1) * border_spacing_h;

	எனில் (auto_width <= min_width) {
		/* not enough space: minimise column widths */
		க்கு (i = 0; i < columns; i++) {
			col[i].width = col[i].min;
		}
		table_width = min_width;
	} இல்லையெனில் எனில் (max_width <= auto_width) {
		/* more space than maximum width */
		எனில் (table_width == AUTO) {
			/* for auto-width tables, make columns max width */
			க்கு (i = 0; i < columns; i++) {
				col[i].width = col[i].max;
			}
			table_width = max_width;
		} இல்லையெனில் {
			/* for fixed-width tables, distribute the extra space
			 * too */
			unsigned முழு flexible_columns = 0;
			க்கு (i = 0; i != columns; i++)
				எனில் (col[i].type != COLUMN_WIDTH_FIXED)
					flexible_columns++;
			எனில் (flexible_columns == 0) {
				முழு extra = (table_width - max_width) / columns;
				remainder = (table_width - max_width) -
						(extra * columns);
				க்கு (i = 0; i != columns; i++) {
					col[i].width = col[i].max + extra;
					மொத்தம் -= remainder;
					எனில் (மொத்தம் < 0) {
						col[i].width++;
						மொத்தம் += columns;
					}
				}

			} இல்லையெனில் {
				முழு extra = (table_width - max_width) /
						flexible_columns;
				remainder = (table_width - max_width) -
						(extra * flexible_columns);
				க்கு (i = 0; i != columns; i++)
					எனில் (col[i].type != COLUMN_WIDTH_FIXED) {
						col[i].width = col[i].max +
								extra;
						மொத்தம் -= remainder;
						எனில் (மொத்தம் < 0) {
							col[i].width++;
							மொத்தம் += flexible_columns;
						}
					}
			}
		}
	} இல்லையெனில் {
		/* space between min and max: fill it exactly */
		தசம scale = (தசம) (auto_width - min_width) /
				(தசம) (max_width - min_width);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		க்கு (i = 0; i < columns; i++) {
			col[i].width = col[i].min + (முழு) (0.5 +
					(col[i].max - col[i].min) * scale);
		}
		table_width = auto_width;
	}

	xs[0] = x = border_spacing_h;
	க்கு (i = 0; i != columns; i++) {
		எனில் (!col[i].positioned)
			x += col[i].width + border_spacing_h;
		xs[i + 1] = x;
		row_span[i] = 0;
		excess_y[i] = 0;
		row_span_cell[i] = 0;
	}

	/* position cells */
	table_height = border_spacing_v;
	க்கு (row_group = table->children; row_group;
			row_group = row_group->next) {
		முழு row_group_height = 0;
		க்கு (row = row_group->children; row; row = row->next) {
			முழு row_height = 0;

			htype = css_computed_height(row->style, &மதிப்பு, &unit);
			எனில் (htype == CSS_HEIGHT_SET && unit != CSS_UNIT_PCT) {
				row_height = FIXTOINT(css_unit_len2device_px(
						row->style,
						&content->unit_len_ctx,
						மதிப்பு, unit));
			}
			க்கு (c = row->children; c; c = c->next) {
				ப்ரதிஜ்ஞா(c->style);
				c->width = xs[c->start_column + c->columns] -
						xs[c->start_column] -
						border_spacing_h -
						c->border[இடது_2].width -
						c->padding[இடது_2] -
						c->padding[வலது_2] -
						c->border[வலது_2].width;
				c->float_children = 0;
				c->cached_place_below_level = 0;

				c->height = AUTO;
				எனில் (!layout_block_context(c, -1, content)) {
					விடுவி(col);
					விடுவி(excess_y);
					விடுவி(row_span);
					விடுவி(row_span_cell);
					விடுவி(xs);
					திரும்பு false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->height;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_height(c->style,
						&மதிப்பு, &unit);

				எனில் (htype == CSS_HEIGHT_SET &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					முழு h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							மதிப்பு, unit));
					எனில் (c->height < h)
						c->height = h;
				}
				/* specified row height is treated as a minimum
				 */
				எனில் (c->height < row_height)
					c->height = row_height;
				c->x = xs[c->start_column] +
						c->border[இடது_2].width;
				c->y = c->border[உச்சி_2].width;
				க்கு (i = 0; i != c->columns; i++) {
					row_span[c->start_column + i] = c->rows;
					excess_y[c->start_column + i] =
							c->border[உச்சி_2].width +
							c->padding[உச்சி_2] +
							c->height +
							c->padding[BOTTOM] +
							c->border[BOTTOM].width;
					row_span_cell[c->start_column + i] = 0;
				}
				row_span_cell[c->start_column] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[உச்சி_2].width -
						c->padding[உச்சி_2] -
						c->height -
						c->border[BOTTOM].width;
			}
			க்கு (i = 0; i != columns; i++)
				எனில் (row_span[i] != 0)
					row_span[i]--;
				இல்லையெனில்
					row_span_cell[i] = 0;
			எனில் (row->next || row_group->next) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				க்கு (i = 0; i != columns; i++)
					எனில் (row_span[i] == 0 && row_height <
							excess_y[i])
						row_height = excess_y[i];
			} இல்லையெனில் {
				/* except in the last row */
				க்கு (i = 0; i != columns; i++)
					எனில் (row_height < excess_y[i])
						row_height = excess_y[i];
			}
			க்கு (i = 0; i != columns; i++) {
				எனில் (row_height < excess_y[i])
					excess_y[i] -= row_height;
				இல்லையெனில்
					excess_y[i] = 0;
				எனில் (row_span_cell[i] != 0)
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
	க்கு (row_group = table->children; row_group;
			row_group = row_group->next) {
		க்கு (row = row_group->children; row; row = row->next) {
			க்கு (c = row->children; c; c = c->next) {
				எண்ணிக்கை css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				spare_height = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->height - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &மதிப்பு, &unit);

				தேர்வு (vertical_align) {
				நிலை CSS_VERTICAL_ALIGN_SUB:
				நிலை CSS_VERTICAL_ALIGN_SUPER:
				நிலை CSS_VERTICAL_ALIGN_TEXT_உச்சி:
				நிலை CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				நிலை CSS_VERTICAL_ALIGN_SET:
				நிலை CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				நிலை CSS_VERTICAL_ALIGN_உச்சி:
					நிறுத்து;
				நிலை CSS_VERTICAL_ALIGN_நடு:
					c->padding[உச்சி_2] += spare_height / 2;
					c->padding[BOTTOM] -= spare_height / 2;
					layout_move_children(c, 0,
							spare_height / 2);
					நிறுத்து;
				நிலை CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[உச்சி_2] += spare_height;
					c->padding[BOTTOM] -= spare_height;
					layout_move_children(c, 0,
							spare_height);
					நிறுத்து;
				நிலை CSS_VERTICAL_ALIGN_INHERIT:
					ப்ரதிஜ்ஞா(0);
					நிறுத்து;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	எனில் (table->margin[உச்சி_2] == AUTO)
		table->margin[உச்சி_2] = 0;
	எனில் (table->margin[BOTTOM] == AUTO)
		table->margin[BOTTOM] = 0;

	விடுவி(col);
	விடுவி(excess_y);
	விடுவி(row_span);
	விடுவி(row_span_cell);
	விடுவி(xs);

	table->width = table_width;
	table->height = table_height;

	திரும்பு true;
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
நிலையான bool layout_apply_minmax_height(
		மாறிலி css_unit_ctx *unit_len_ctx,
		கட்டமைப்பு box *box,
		கட்டமைப்பு box *container)
{
	முழு h;
	கட்டமைப்பு box *containing_block = வெற்று;
	bool updated = false;

	/* Find containing block for percentage heights */
	எனில் (box->style != வெற்று && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		ப்ரதிஜ்ஞா(container);
		containing_block = container;
	} இல்லையெனில் எனில் (box->float_container && box->style != வெற்று &&
			(css_computed_float(box->style) == CSS_FLOAT_இடது ||
			 css_computed_float(box->style) == CSS_FLOAT_வலது)) {
		/* Box is a float */
		ப்ரதிஜ்ஞா(box->parent && box->parent->parent &&
				box->parent->parent->parent);
		containing_block = box->parent->parent->parent;
	} இல்லையெனில் எனில் (box->parent && box->parent->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_block = box->parent;
	} இல்லையெனில் எனில் (box->parent && box->parent->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		ப்ரதிஜ்ஞா(box->parent->parent);
		containing_block = box->parent->parent;
	}

	எனில் (box->style) {
		எண்ணிக்கை css_height_e htype = CSS_HEIGHT_AUTO;
		css_fixed மதிப்பு = 0;
		css_unit unit = CSS_UNIT_PX;

		எனில் (containing_block) {
			htype = css_computed_height(containing_block->style,
					&மதிப்பு, &unit);
		}

		/* max-height */
		எனில் (css_computed_max_height(box->style, &மதிப்பு, &unit) ==
				CSS_MAX_HEIGHT_SET) {
			எனில் (unit == CSS_UNIT_PCT) {
				எனில் (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(மதிப்பு,
						containing_block->height);
					எனில் (h < box->height) {
						box->height = h;
						updated = true;
					}
				}
			} இல்லையெனில் {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						மதிப்பு, unit));
				எனில் (h < box->height) {
					box->height = h;
					updated = true;
				}
			}
		}

		/* min-height */
		எனில் (ns_computed_min_height(box->style, &மதிப்பு, &unit) ==
				CSS_MIN_HEIGHT_SET) {
			எனில் (unit == CSS_UNIT_PCT) {
				எனில் (containing_block &&
					containing_block->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_SET)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_OF_INT_TOINT(மதிப்பு,
						containing_block->height);
					எனில் (h > box->height) {
						box->height = h;
						updated = true;
					}
				}
			} இல்லையெனில் {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						மதிப்பு, unit));
				எனில் (h > box->height) {
					box->height = h;
					updated = true;
				}
			}
		}
	}
	திரும்பு updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
நிலையான bool layout_block_object(கட்டமைப்பு box *block)
{
	ப்ரதிஜ்ஞா(block);
	ப்ரதிஜ்ஞா(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_TABLE ||
			block->type == BOX_TABLE_CELL);
	ப்ரதிஜ்ஞா(block->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", block,
	      hlcache_handle_get_url(block->object), block->width);

	எனில் (content_can_reformat(block->object)) {
		content_reformat(block->object, false, block->width, 1);
	} இல்லையெனில் {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	திரும்பு true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
நிலையான வெறுமை add_float_to_container(கட்டமைப்பு box *cont, கட்டமைப்பு box *b)
{
	கட்டமைப்பு box *box = cont->float_children;
	முழு b_bottom = b->y + b->height;

	ப்ரதிஜ்ஞா(b->type == BOX_FLOAT_இடது || b->type == BOX_FLOAT_வலது);

	எனில் (box == வெற்று) {
		/* No other float children */
		b->next_float = வெற்று;
		cont->float_children = b;
		திரும்பு;
	} இல்லையெனில் எனில் (b_bottom >= box->y + box->height) {
		/* Goes at start of list */
		b->next_float = cont->float_children;
		cont->float_children = b;
	} இல்லையெனில் {
		கட்டமைப்பு box *prev = வெற்று;
		போது (box != வெற்று && b_bottom < box->y + box->height) {
			prev = box;
			box = box->next_float;
		}
		எனில் (prev != வெற்று) {
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
நிலையான bool
layout_text_box_split(html_content *content,
		      plot_font_style_t *fstyle,
		      கட்டமைப்பு box *split_box,
		      size_t new_length,
		      முழு new_width)
{
	முழு space_width = split_box->space;
	கட்டமைப்பு box *c2;
	மாறிலி கட்டமைப்பு gui_layout_table *font_func = content->font_func;
	bool space = (split_box->text[new_length] == ' ');
	முழு used_length = new_length + (space ? 1 : 0);

	எனில் ((space && space_width == 0) || space_width == UNKNOWN_WIDTH) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->width(fstyle, " ", 1, &space_width);
	}

	எனில் (split_box->space == UNKNOWN_WIDTH)
		split_box->space = space_width;
	எனில் (!space)
		space_width = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, split_box, அளவு *c2);
	எனில் (!c2)
		திரும்பு false;
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
	எனில் (c2->next)
		c2->next->prev = c2;
	இல்லையெனில்
		c2->parent->last = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      split_box,
	      split_box->length,
	      (முழு)split_box->length,
	      split_box->text);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->length,
	      (முழு)c2->length,
	      c2->text);

	திரும்பு true;
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
நிலையான வெறுமை
layout_float_find_dimensions(
		மாறிலி css_unit_ctx *unit_len_ctx,
		முழு available_width,
		மாறிலி css_computed_style *style,
		கட்டமைப்பு box *box)
{
	முழு width, height, max_width, min_width, max_height, min_height;
	முழு *margin = box->margin;
	முழு *padding = box->padding;
	கட்டமைப்பு box_border *border = box->border;
	எண்ணிக்கை css_overflow_e overflow_x = css_computed_overflow_x(style);
	எண்ணிக்கை css_overflow_e overflow_y = css_computed_overflow_y(style);
	முழு scrollbar_width_x =
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;
	முழு scrollbar_width_y =
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;

	layout_find_dimensions(unit_len_ctx, available_width, -1, box, style,
			&width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	எனில் (margin[இடது_2] == AUTO)
		margin[இடது_2] = 0;
	எனில் (margin[வலது_2] == AUTO)
		margin[வலது_2] = 0;

	எனில் (box->gadget == வெற்று) {
		padding[வலது_2] += scrollbar_width_y;
		padding[BOTTOM] += scrollbar_width_x;
	}

	எனில் (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	} இல்லையெனில் எனில் (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed size = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		ப்ரதிஜ்ஞா(box->style);

		எனில் (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			எனில் (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			எனில் (box->gadget->type == GADGET_FILE &&
					height == AUTO) {
				size = FLTTOFIX(1.5);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
		எனில் (box->gadget->type == GADGET_TEXTAREA) {
			எனில் (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			எனில் (height == AUTO) {
				size = INTTOFIX(4);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
	} இல்லையெனில் எனில் (width == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		width = min(max(box->min_width, available_width),
				box->max_width);

		/* width includes margin, borders and padding */
		எனில் (width == available_width) {
			width -= box->margin[இடது_2] + box->border[இடது_2].width +
					box->padding[இடது_2] +
					box->padding[வலது_2] +
					box->border[வலது_2].width +
					box->margin[வலது_2];
		} இல்லையெனில் {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			முழு fixed = 0;
			தசம frac = 0;
			calculate_mbp_width(unit_len_ctx, box->style, இடது_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_width(unit_len_ctx, box->style, வலது_2,
					true, true, true, &fixed, &frac);
			எனில் (fixed < 0)
				fixed = 0;

			width -= fixed;
		}

		எனில் (max_width >= 0 && width > max_width) width = max_width;
		எனில் (min_width >  0 && width < min_width) width = min_width;

	} இல்லையெனில் {
		எனில் (max_width >= 0 && width > max_width) width = max_width;
		எனில் (min_width >  0 && width < min_width) width = min_width;
		width -= scrollbar_width_y;
	}

	box->width = width;
	box->height = height;

	எனில் (margin[உச்சி_2] == AUTO)
		margin[உச்சி_2] = 0;
	எனில் (margin[BOTTOM] == AUTO)
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
நிலையான bool layout_float(கட்டமைப்பு box *b, முழு width, html_content *content)
{
	ப்ரதிஜ்ஞா(b->type == BOX_TABLE ||
	       b->type == BOX_BLOCK ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_find_dimensions(&content->unit_len_ctx, width, b->style, b);
	எனில் (b->type == BOX_TABLE || b->type == BOX_INLINE_FLEX) {
		எனில் (b->type == BOX_TABLE) {
			எனில் (!layout_table(b, width, content))
				திரும்பு false;
		} இல்லையெனில் {
			எனில் (!layout_flex(b, width, content))
				திரும்பு false;
		}
		எனில் (b->margin[இடது_2] == AUTO)
			b->margin[இடது_2] = 0;
		எனில் (b->margin[வலது_2] == AUTO)
			b->margin[வலது_2] = 0;
		எனில் (b->margin[உச்சி_2] == AUTO)
			b->margin[உச்சி_2] = 0;
		எனில் (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} இல்லையெனில் {
		திரும்பு layout_block_context(b, -1, content);
	}
	திரும்பு true;
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
நிலையான வெறுமை
place_float_below(கட்டமைப்பு box *c, முழு width, முழு cx, முழு y, கட்டமைப்பு box *cont)
{
	முழு x0, x1, yy;
	கட்டமைப்பு box *இடது;
	கட்டமைப்பு box *வலது;

	yy = y > cont->cached_place_below_level ?
			y : cont->cached_place_below_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      width, cx, y, cont);

	செய் {
		y = yy;
		x0 = cx;
		x1 = cx + width;
		find_sides(cont->float_children, y, y + c->height, &x0, &x1,
				&இடது, &வலது);
		எனில் (இடது != 0 && வலது != 0) {
			yy = (இடது->y + இடது->height <
					வலது->y + வலது->height ?
					இடது->y + இடது->height :
					வலது->y + வலது->height);
		} இல்லையெனில் எனில் (இடது == 0 && வலது != 0) {
			yy = வலது->y + வலது->height;
		} இல்லையெனில் எனில் (இடது != 0 && வலது == 0) {
			yy = இடது->y + இடது->height;
		}
	} போது ((இடது != 0 || வலது != 0) && (c->width > x1 - x0));

	எனில் (c->type == BOX_FLOAT_இடது) {
		c->x = x0;
	} இல்லையெனில் {
		c->x = x1 - c->width;
	}
	c->y = y;
	cont->cached_place_below_level = y;
}


/**
 * Calculate line height from a style.
 */
நிலையான முழு வரி_height(
		மாறிலி css_unit_ctx *unit_len_ctx,
		மாறிலி css_computed_style *style)
{
	எண்ணிக்கை css_வரி_height_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed வரி_height;

	ப்ரதிஜ்ஞா(style);

	lhtype = css_computed_வரி_height(style, &lhvalue, &lhunit);
	எனில் (lhtype == CSS_வரி_HEIGHT_NORMAL) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_வரி_HEIGHT_எண்;
	}

	எனில் (lhtype == CSS_வரி_HEIGHT_எண் ||
			lhunit == CSS_UNIT_PCT) {
		வரி_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		எனில் (lhtype != CSS_வரி_HEIGHT_எண்)
			வரி_height = FDIV(வரி_height, F_100);
	} இல்லையெனில் {
		ப்ரதிஜ்ஞா(lhunit != CSS_UNIT_PCT);

		வரி_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	திரும்பு FIXTOINT(வரி_height);
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
நிலையான bool
layout_வரி(கட்டமைப்பு box *first,
	    முழு *width,
	    முழு *y,
	    முழு cx,
	    முழு cy,
	    கட்டமைப்பு box *cont,
	    bool indent,
	    bool has_text_children,
	    html_content *content,
	    கட்டமைப்பு box **next_box)
{
	முழு height, used_height;
	முழு x0 = 0;
	முழு x1 = *width;
	முழு x, h, x_previous;
	முழு fy = cy;
	கட்டமைப்பு box *இடது;
	கட்டமைப்பு box *வலது;
	கட்டமைப்பு box *b;
	கட்டமைப்பு box *split_box = 0;
	கட்டமைப்பு box *d;
	கட்டமைப்பு box *br_box = 0;
	bool move_y = false;
	bool place_below = false;
	முழு space_before = 0, space_after = 0;
	unsigned முழு inline_மொத்தம் = 0;
	unsigned முழு i;
	மாறிலி கட்டமைப்பு gui_layout_table *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      first,
	      (முழு)first->length,
	      first->text,
	      *width,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	find_sides(cont->float_children, cy, cy, &x0, &x1, &இடது, &வலது);
	x0 -= cx;
	x1 -= cx;

	எனில் (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	எனில் (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	எனில் (has_text_children || first->parent->parent->gadget)
		used_height = height = வரி_height(&content->unit_len_ctx,
				first->parent->parent->style);
	இல்லையெனில்
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		used_height = height = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	க்கு (x = 0, b = first; x <= x1 - x0 && b != 0; b = b->next) {
		முழு min_width, max_width, min_height, max_height;

		ப்ரதிஜ்ஞா(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		எனில் (b->type == BOX_BR)
			நிறுத்து;

		எனில் (lh__box_is_float_box(b))
			தொடர்;
		எனில் (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			தொடர்;

		ப்ரதிஜ்ஞா(b->style != வெற்று);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		எனில் (b->type == BOX_INLINE_BLOCK ||
		    b->type == BOX_INLINE_FLEX) {
			எனில் (b->max_width != UNKNOWN_WIDTH)
				எனில் (!layout_float(b, *width, content))
					திரும்பு false;
			h = b->border[உச்சி_2].width + b->padding[உச்சி_2] + b->height +
					b->padding[BOTTOM] +
					b->border[BOTTOM].width;
			எனில் (height < h)
				height = h;
			x += b->margin[இடது_2] + b->border[இடது_2].width +
					b->padding[இடது_2] + b->width +
					b->padding[வலது_2] +
					b->border[வலது_2].width +
					b->margin[வலது_2];
			space_after = 0;
			தொடர்;
		}

		எனில் (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_find_dimensions(&content->unit_len_ctx,
					*width, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			க்கு (i = 0; i != 4; i++)
				எனில் (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[இடது_2] + b->border[இடது_2].width +
					b->padding[இடது_2];
			எனில் (b->inline_end) {
				b->inline_end->margin[வலது_2] = b->margin[வலது_2];
				b->inline_end->padding[வலது_2] =
						b->padding[வலது_2];
				b->inline_end->border[வலது_2] =
						b->border[வலது_2];
			} இல்லையெனில் {
				x += b->padding[வலது_2] +
						b->border[வலது_2].width +
						b->margin[வலது_2];
			}
		} இல்லையெனில் எனில் (b->type == BOX_INLINE_END) {
			b->width = 0;
			எனில் (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[வலது_2] + b->border[வலது_2].width +
					b->margin[வலது_2];
			தொடர்;
		}

		எனில் (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->height = வரி_height(&content->unit_len_ctx,
					b->style ? b->style :
					b->parent->parent->style);
			எனில் (height < b->height)
				height = b->height;

			எனில் (!b->text) {
				b->width = 0;
				space_after = 0;
				தொடர்;
			}

			எனில் (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				எனில் (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					முழு opt_maxwidth = 0;
					கட்டமைப்பு form_option *o;

					க்கு (o = b->parent->parent->gadget->
							data.select.பொருட்கள்; o;
							o = o->next) {
						முழு opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						எனில் (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}
					b->width = opt_maxwidth;
					எனில் (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;
				} இல்லையெனில் {
					font_func->width(&fstyle, b->text,
							b->length, &b->width);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			எனில் (b->text && (x + b->width < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->next) {
				font_func->width(&fstyle, b->text,
						 b->length, &b->width);
				b->flags |= MEASURED;
			}

			x += b->width;
			எனில் (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			தொடர்;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		ப்ரதிஜ்ஞா(b->style);

		layout_find_dimensions(&content->unit_len_ctx,
				*width, -1, b, b->style,
				&b->width, &b->height,
				&max_width, &min_width,
				&max_height, &min_height,
				வெற்று, வெற்று, வெற்று);

		எனில் (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->width, &b->height,
					min_width, max_width,
					min_height, max_height);
		} இல்லையெனில் எனில் (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			எனில் (b->width == AUTO)
				b->width = 400;
			எனில் (b->height == AUTO)
				b->height = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} இல்லையெனில் {
			/* form control with no object */
			எனில் (b->width == AUTO)
				b->width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			எனில் (b->height == AUTO)
				b->height = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		எனில் (b->object && content_can_reformat(b->object) &&
				b->width !=
				content_get_available_width(b->object)) {
			css_fixed மதிப்பு = 0;
			css_unit unit = CSS_UNIT_PX;
			எண்ணிக்கை css_height_e htype = css_computed_height(b->style,
					&மதிப்பு, &unit);

			content_reformat(b->object, false, b->width, b->height);

			எனில் (htype == CSS_HEIGHT_AUTO)
				b->height = content_get_height(b->object);
		}

		எனில் (height < b->height)
			height = b->height;

		x += b->width;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *width;
	find_sides(cont->float_children, cy, cy + height, &x0, &x1,
			&இடது, &வலது);
	x0 -= cx;
	x1 -= cx;

	எனில் (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	எனில் (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	க்கு (x = x_previous = 0, b = first; x <= x1 - x0 && b; b = b->next) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		எனில் (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} இல்லையெனில் எனில் (lh__box_is_inline_flow(b)) {
			ப்ரதிஜ்ஞா(b->width != UNKNOWN_WIDTH);

			x_previous = x;
			x += space_after;
			b->x = x;

			எனில் ((b->type == BOX_INLINE && !b->inline_end) ||
					b->type == BOX_INLINE_BLOCK ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[இடது_2] + b->border[இடது_2].width;
				x = b->x + b->padding[இடது_2] + b->width +
						b->padding[வலது_2] +
						b->border[வலது_2].width +
						b->margin[வலது_2];
			} இல்லையெனில் எனில் (b->type == BOX_INLINE) {
				b->x += b->margin[இடது_2] + b->border[இடது_2].width;
				x = b->x + b->padding[இடது_2] + b->width;
			} இல்லையெனில் எனில் (b->type == BOX_INLINE_END) {
				b->height = b->inline_end->height;
				x += b->padding[வலது_2] +
						b->border[வலது_2].width +
						b->margin[வலது_2];
			} இல்லையெனில் {
				x += b->width;
			}

			space_before = space_after;
			எனில் (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			இல்லையெனில் எனில் (b->text || b->type == BOX_INLINE_END) {
				எனில் (b->space == UNKNOWN_WIDTH) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} இல்லையெனில் {
				space_after = 0;
			}
			split_box = b;
			move_y = true;
			inline_மொத்தம்++;
		} இல்லையெனில் எனில் (b->type == BOX_BR) {
			b->x = x;
			b->width = 0;
			br_box = b;
			b = b->next;
			split_box = 0;
			move_y = true;
			நிறுத்து;

		} இல்லையெனில் {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_place_below_level = 0;
			b->float_container = d->float_container = cont;

			எனில் (!layout_float(d, *width, content))
				திரும்பு false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[உச்சி_2],
			      d->border[உச்சி_2].width);

			d->x = d->margin[இடது_2] + d->border[இடது_2].width;
			d->y = d->margin[உச்சி_2] + d->border[உச்சி_2].width;
			b->width = d->margin[இடது_2] + d->border[இடது_2].width +
					d->padding[இடது_2] + d->width +
					d->padding[வலது_2] +
					d->border[வலது_2].width +
					d->margin[வலது_2];
			b->height = d->margin[உச்சி_2] + d->border[உச்சி_2].width +
					d->padding[உச்சி_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];

			எனில் (b->width > (x1 - x0) - x)
				place_below = true;
			எனில் (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_இடது && இடது == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_வலது &&
						வலது == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						இடது == 0 && வலது == 0)) &&
					(!place_below ||
					(இடது == 0 && வலது == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_place_below_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				எனில் (b->type == BOX_FLOAT_இடது) {
					b->x = cx + x0;
					எனில் (b->width > 0)
						x0 += b->width;
					இடது = b;
				} இல்லையெனில் {
					b->x = cx + x1 - b->width;
					எனில் (b->width > 0)
						x1 -= b->width;
					வலது = b;
				}
				b->y = cy;
			} இல்லையெனில் {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				முழு fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_place_below_level) ?
						fcy :
						cont->cached_place_below_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + height : fy;

				place_float_below(b, *width, cx, fy, cont);
				fy = b->y;
				எனில் (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_இடது && இடது != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_வலது &&
						வலது != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(இடது != 0 || வலது != 0)))) {
					/* to be cleared below existing
					 * floats */
					எனில் (b->type == BOX_FLOAT_இடது)
						b->x = cx;
					இல்லையெனில்
						b->x = cx + *width - b->width;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					எனில் (fcy > cont->clear_level)
						cont->clear_level = fcy;
					எனில் (b->y < fcy)
						b->y = fcy;
				}
				எனில் (b->type == BOX_FLOAT_இடது)
					இடது = b;
				இல்லையெனில்
					வலது = b;
			}
			add_float_to_container(cont, b);

			split_box = 0;
		}
	}

	எனில் (x1 - x0 < x && split_box) {
		/* the last box went over the end */
		size_t split = 0;
		முழு w;
		bool no_wrap = css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_previous;

		எனில் (!no_wrap &&
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

		எனில் (split == 0)
			w = split_box->width;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      split_box,
		      (முழு)split_box->length,
		      split_box->text,
		      split,
		      w,
		      இடது,
		      வலது,
		      inline_மொத்தம்);

		எனில் ((split == 0 || x1 - x0 <= x + space_before + w) &&
				!இடது && !வலது && inline_மொத்தம் == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			எனில் (split == 0 || split == split_box->length) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = split_box->next;
			} இல்லையெனில் {
				/* cut off first word for this line */
				எனில் (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					திரும்பு false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} இல்லையெனில் எனில் ((split == 0 || x1 - x0 <= x + space_before + w) &&
				inline_மொத்தம் == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			ப்ரதிஜ்ஞா(இடது || வலது);
			used_height = 0;
			எனில் (இடது) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      இடது->y,
				      இடது->height);

				used_height = இடது->y + இடது->height - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      used_height);

			}
			எனில் (வலது && used_height <
					வலது->y + வலது->height - cy + 1)
				used_height = வலது->y + வலது->height - cy + 1;

			எனில் (used_height < 0)
				used_height = 0;

			b = split_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} இல்லையெனில் எனில் (split == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = split_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} இல்லையெனில் {
			/* fit as many words as possible */
			ப்ரதிஜ்ஞா(split != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (முழு)split_box->length, split_box->text,
			      x1 - x0, split, w);

			எனில் (split != split_box->length) {
				எனில் (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					திரும்பு false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	தேர்வு (css_computed_text_align(first->parent->parent->style)) {
	நிலை CSS_TEXT_ALIGN_வலது:
	நிலை CSS_TEXT_ALIGN_LIBCSS_வலது:
		x0 = x1 - x;
		நிறுத்து;
	நிலை CSS_TEXT_ALIGN_CENTER:
	நிலை CSS_TEXT_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		நிறுத்து;
	நிலை CSS_TEXT_ALIGN_இடது:
	நிலை CSS_TEXT_ALIGN_LIBCSS_இடது:
	நிலை CSS_TEXT_ALIGN_JUSTIFY:
		/* leave on left */
		நிறுத்து;
	நிலை CSS_TEXT_ALIGN_DEFAULT:
		/* None; consider text direction */
		தேர்வு (css_computed_direction(first->parent->parent->style)) {
		நிலை CSS_DIRECTION_LTR:
			/* leave on left */
			நிறுத்து;
		நிலை CSS_DIRECTION_RTL:
			x0 = x1 - x;
			நிறுத்து;
		}
		நிறுத்து;
	}

	க்கு (d = first; d != b; d = d->next) {
		d->flags &= ~NEW_வரி;

		எனில் (d->type == BOX_INLINE_BLOCK &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			தொடர்;
		} இல்லையெனில் எனில் ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[உச்சி_2];

			எனில் (d->type == BOX_TEXT && d->height > used_height) {
				/* text */
				used_height = d->height;
			}
		} இல்லையெனில் எனில் ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_BLOCK) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[உச்சி_2].width + d->margin[உச்சி_2];
			h = d->margin[உச்சி_2] + d->border[உச்சி_2].width +
					d->padding[உச்சி_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];
			எனில் (used_height < h)
				used_height = h;
		}
	}

	first->flags |= NEW_வரி;

	ப்ரதிஜ்ஞா(b != first || (move_y && 0 < used_height && (இடது || வலது)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	க்கு (d = first; d != b; d = d->next) {
		எனில் ((d->type == BOX_INLINE && d->inline_end) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			css_fixed மதிப்பு = 0;
			css_unit unit = CSS_UNIT_PX;
			தேர்வு (css_computed_vertical_align(d->style, &மதிப்பு,
					&unit)) {
			நிலை CSS_VERTICAL_ALIGN_SUPER:
			நிலை CSS_VERTICAL_ALIGN_உச்சி:
			நிலை CSS_VERTICAL_ALIGN_TEXT_உச்சி:
				/* already at top */
				நிறுத்து;
			நிலை CSS_VERTICAL_ALIGN_SUB:
			நிலை CSS_VERTICAL_ALIGN_BOTTOM:
			நிலை CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				d->y += used_height - d->height;
				நிறுத்து;
			இயல்பு:
			நிலை CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (used_height - d->height);
				நிறுத்து;
			}
		}
	}

	/* handle clearance for br */
	எனில் (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		முழு clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		எனில் (used_height < clear_y - cy)
			used_height = clear_y - cy;
	}

	எனில் (move_y)
		*y += used_height;
	*next_box = b;
	*width = x; /* return actual width */
	திரும்பு true;
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
நிலையான bool layout_inline_container(கட்டமைப்பு box *inline_container, முழு width,
		கட்டமைப்பு box *cont, முழு cx, முழு cy, html_content *content)
{
	bool first_வரி = true;
	bool has_text_children;
	கட்டமைப்பு box *c, *next;
	முழு y = 0;
	முழு curwidth,maxwidth = width;

	ப்ரதிஜ்ஞா(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      width,
	      cont,
	      cx,
	      cy);


	has_text_children = false;
	க்கு (c = inline_container->children; c; c = c->next) {
		bool is_pre = false;

		எனில் (c->style) {
			எண்ணிக்கை css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_வரி ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		எனில் ((lh__box_is_object(c) == false &&
				c->text && (c->length || is_pre)) ||
				c->type == BOX_BR)
			has_text_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	க்கு (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->width;
		எனில் (!layout_வரி(c, &curwidth, &y, cx, cy + y, cont, first_வரி,
				has_text_children, content, &next))
			திரும்பு false;
		maxwidth = max(maxwidth,curwidth);
		c = next;
		first_வரி = false;
	}

	inline_container->width = maxwidth;
	inline_container->height = y;

	திரும்பு true;
}


/* Documented in layout_intertnal.h */
bool layout_block_context(
		கட்டமைப்பு box *block,
		முழு viewport_height,
		html_content *content)
{
	கட்டமைப்பு box *box;
	முழு cx, cy;  /**< current coordinates */
	முழு max_pos_margin = 0;
	முழு max_neg_margin = 0;
	முழு y = 0;
	முழு lm, rm;
	கட்டமைப்பு box *margin_collapse = வெற்று;
	bool in_margin = false;
	css_fixed gadget_size;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	ப்ரதிஜ்ஞா(block->type == BOX_BLOCK ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX);
	ப்ரதிஜ்ஞா(block->width != UNKNOWN_WIDTH);
	ப்ரதிஜ்ஞா(block->width != AUTO);

	block->float_children = வெற்று;
	block->cached_place_below_level = 0;
	block->clear_level = 0;

	/* special case if the block contains an object */
	எனில் (block->object) {
		முழு temp_width = block->width;
		எனில் (!layout_block_object(block))
			திரும்பு false;
		layout_get_object_dimensions(block, &temp_width,
				&block->height, INT_MIN, INT_MAX,
				INT_MIN, INT_MAX);
		திரும்பு true;
	} இல்லையெனில் எனில் (block->flags & REPLACE_DIM) {
		திரும்பு true;
	}

	/* special case if the block contains an radio button or checkbox */
	எனில் (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_size = INTTOFIX(1);
		எனில் (block->height == AUTO)
			block->height = FIXTOINT(css_unit_len2device_px(
					block->style,
					&content->unit_len_ctx,
					gadget_size, gadget_unit));
	}

	box = block->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = block->padding[உச்சி_2];
	எனில் (box)
		box->y = block->padding[உச்சி_2];

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
	போது (box) {
		எண்ணிக்கை css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
		எண்ணிக்கை css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

		ப்ரதிஜ்ஞா(box->type == BOX_BLOCK ||
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

		எனில் (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->parent->padding[இடது_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			செல் advance_to_next_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		எனில் (margin_collapse == வெற்று) {
			margin_collapse = layout_next_margin_block(
					&content->unit_len_ctx, box, block,
					viewport_height,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			in_margin = true;
		}

		/* Clearance. */
		y = 0;
		எனில் (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(block->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		எனில் (box->style) {
			overflow_x = css_computed_overflow_x(box->style);
			overflow_y = css_computed_overflow_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		எனில் (box->type == BOX_FLEX ||
		    box->type == BOX_BLOCK ||
		    box->flags & IFRAME) {
			எனில் (lh__box_is_object(box) == false &&
					box->style &&
					(overflow_x != CSS_OVERFLOW_VISIBLE ||
					 overflow_y != CSS_OVERFLOW_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				முழு x0, x1, உச்சி;
				கட்டமைப்பு box *இடது, *வலது;
				உச்சி = cy + max_pos_margin - max_neg_margin;
				உச்சி = (உச்சி > y) ? உச்சி : y;
				x0 = cx;
				x1 = cx + box->parent->width -
						box->parent->padding[இடது_2] -
						box->parent->padding[வலது_2];
				find_sides(block->float_children, உச்சி, உச்சி,
						&x0, &x1, &இடது, &வலது);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->parent->width -
						box->parent->padding[இடது_2] -
						box->parent->padding[வலது_2] -
						x1;
			}
			layout_block_find_dimensions(&content->unit_len_ctx,
					box->parent->width,
					viewport_height, lm, rm, box);
			எனில் (box->type == BOX_BLOCK && !(box->flags & IFRAME)) {
				layout_block_add_scrollbar(box, வலது_2);
				layout_block_add_scrollbar(box, BOTTOM);
			}
		} இல்லையெனில் எனில் (box->type == BOX_TABLE) {
			எனில் (box->style != வெற்று) {
				எண்ணிக்கை css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style, &width,
						&unit);

				எனில் (wtype == CSS_WIDTH_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					முழு x0, x1, உச்சி;
					கட்டமைப்பு box *இடது, *வலது;
					உச்சி = cy + max_pos_margin -
							max_neg_margin;
					உச்சி = (உச்சி > y) ? உச்சி : y;
					x0 = cx;
					x1 = cx + box->parent->width -
						box->parent->padding[இடது_2] -
						box->parent->padding[வலது_2];
					find_sides(block->float_children,
						உச்சி, உச்சி, &x0, &x1,
						&இடது, &வலது);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->parent->width -
						box->parent->padding[இடது_2] -
						box->parent->padding[வலது_2] -
						x1;
				}
			}
			எனில் (!layout_table(box, box->parent->width - lm - rm,
					content))
				திரும்பு false;
			layout_solve_width(box, box->parent->width, box->width,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->parent->padding[இடது_2] + box->margin[இடது_2] +
				box->border[இடது_2].width;
		cx += box->x;

		/* Position box: vertical. */
		எனில் (box->border[உச்சி_2].width) {
			box->y += box->border[உச்சி_2].width;
			cy += box->border[உச்சி_2].width;
		}

		/* Vertical margin */
		எனில் (((box->type == BOX_BLOCK && (box->flags & HAS_HEIGHT)) ||
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
		எனில் (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		எனில் (box->type == BOX_FLEX ||
				(box->type == BOX_BLOCK && box->style &&
				 (overflow_x != CSS_OVERFLOW_VISIBLE ||
				  overflow_y != CSS_OVERFLOW_VISIBLE))) {

			எனில் (box->type == BOX_FLEX) {
				எனில் (!layout_flex(box, box->width, content)) {
					திரும்பு false;
				}
			} இல்லையெனில் {
				layout_block_context(box,
						viewport_height, content);
			}

			cy += box->padding[உச்சி_2];

			எனில் (box->height == AUTO) {
				box->height = 0;
				layout_block_add_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->height + box->padding[BOTTOM] +
					box->border[BOTTOM].width;
			y = box->y + box->padding[உச்சி_2] + box->height +
					box->padding[BOTTOM] +
					box->border[BOTTOM].width;

			/* Skip children, because they are done in the new
			 * block context */
			செல் advance_to_next_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->width);

		/* Layout (except tables). */
		எனில் (box->object) {
			எனில் (!layout_block_object(box))
				திரும்பு false;

		} இல்லையெனில் எனில் (box->type == BOX_INLINE_CONTAINER) {
			box->width = box->parent->width;
			எனில் (!layout_inline_container(box, box->width, block,
					cx, cy, content))
				திரும்பு false;

		} இல்லையெனில் எனில் (box->type == BOX_TABLE) {
			/* Move down to avoid floats if necessary. */
			முழு x0, x1;
			கட்டமைப்பு box *இடது, *வலது;
			y = cy;
			போது (1) {
				எண்ணிக்கை css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style,
						&width, &unit);

				x0 = cx;
				x1 = cx + box->parent->width;
				find_sides(block->float_children, y,
						y + box->height,
						&x0, &x1, &இடது, &வலது);
				எனில் (wtype == CSS_WIDTH_AUTO)
					நிறுத்து;
				எனில் (box->width <= x1 - x0)
					நிறுத்து;
				எனில் (!இடது && !வலது)
					நிறுத்து;
				இல்லையெனில் எனில் (!இடது)
					y = வலது->y + வலது->height + 1;
				இல்லையெனில் எனில் (!வலது)
					y = இடது->y + இடது->height + 1;
				இல்லையெனில் எனில் (இடது->y + இடது->height <
						வலது->y + வலது->height)
					y = இடது->y + இடது->height + 1;
				இல்லையெனில்
					y = வலது->y + வலது->height + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		எனில் (box->type == BOX_BLOCK && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			எனில் (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = வெற்று;
			}

			y = box->padding[உச்சி_2];
			box = box->children;
			box->y = y;
			cy += y;
			தொடர்;
		} இல்லையெனில் எனில் (box->type == BOX_BLOCK || box->object ||
				box->flags & IFRAME)
			cy += box->padding[உச்சி_2];

		எனில் (box->type == BOX_BLOCK && box->height == AUTO) {
			box->height = 0;
			layout_block_add_scrollbar(box, BOTTOM);
		}

		cy += box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width;
		cx -= box->x;
		y = box->y + box->padding[உச்சி_2] + box->height +
				box->padding[BOTTOM] +
				box->border[BOTTOM].width;

	advance_to_next_box:
		எனில் (!box->next) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			செய் {
				எனில் (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = வெற்று;
				}

				/* Apply bottom margin */
				எனில் (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				இல்லையெனில் எனில் (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->parent;
				எனில் (box == block)
					நிறுத்து;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				எனில் (box->type == BOX_BLOCK &&
						box->flags & MAKE_HEIGHT) {
					margin_collapse = வெற்று;
					in_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				எனில் (box->height == AUTO) {
					box->height = y - box->padding[உச்சி_2];

					எனில் (box->type == BOX_BLOCK)
						layout_block_add_scrollbar(box,
								BOTTOM);
				} இல்லையெனில்
					cy += box->height -
							(y - box->padding[உச்சி_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				எனில் (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_height(
							&content->unit_len_ctx,
							box, வெற்று)) {
					/* Height altered */
					/* Set current cy */
					cy += box->height -
							(y - box->padding[உச்சி_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].width;
				cx -= box->x;
				y = box->y + box->padding[உச்சி_2] + box->height +
						box->padding[BOTTOM] +
						box->border[BOTTOM].width;

			} போது (box->next == வெற்று);
			எனில் (box == block)
				நிறுத்து;
		}

		/* To next sibling. */

		எனில் (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = வெற்று;
		}

		எனில் (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		இல்லையெனில் எனில் (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->next;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	க்கு (box = block->float_children; box; box = box->next_float) {
		y = box->y + box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width + box->margin[BOTTOM];
		எனில் (cy < y)
			cy = y;
	}

	எனில் (block->height == AUTO) {
		block->height = cy - block->padding[உச்சி_2];
		எனில் (block->type == BOX_BLOCK)
			layout_block_add_scrollbar(block, BOTTOM);
	}

	எனில் (block->style && css_computed_position(block->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_height(&content->unit_len_ctx, block, வெற்று);
	}

	எனில் (block->gadget &&
			(block->gadget->type == GADGET_TEXTAREA ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		முழு ta_width = block->padding[இடது_2] + block->width +
				block->padding[வலது_2];
		முழு ta_height = block->padding[உச்சி_2] + block->height +
				block->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				block->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_set_layout(block->gadget->data.text.ta,
				&fstyle, ta_width, ta_height,
				block->padding[உச்சி_2], block->padding[வலது_2],
				block->padding[BOTTOM], block->padding[இடது_2]);
	}

	திரும்பு true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
நிலையான bool
layout__get_element_tag(
		மாறிலி dom_முனை *முனை,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_முனை_type முனை_type;
	dom_exception exc;

	exc = dom_முனை_get_முனை_type(முனை, &முனை_type);
	எனில் (exc != DOM_NO_ERR ||
	    முனை_type != DOM_ELEMENT_முனை) {
		திரும்பு false;
	}

	exc = dom_html_element_get_tag_type(முனை, &element_type);
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு false;
	}

	*type = element_type;
	திரும்பு true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
நிலையான உள்வரி bool
layout__check_element_type(
		மாறிலி dom_முனை *முனை,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	எனில் (!layout__get_element_tag(முனை, &element_type)) {
		திரும்பு false;
	}

	திரும்பு element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
நிலையான bool
layout_get_li_மதிப்பு(dom_முனை *li_முனை, dom_long *மதிப்பு_out)
{
	dom_exception exc;
	dom_long மதிப்பு;
	bool has_மதிப்பு;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_முனை,
			corestring_dom_மதிப்பு,
			&has_மதிப்பு);
	எனில் (exc != DOM_NO_ERR || has_மதிப்பு == false) {
		திரும்பு false;
	}

	exc = dom_html_li_element_get_மதிப்பு(
			(dom_html_li_element *)li_முனை,
			&மதிப்பு);
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு false;
	}

	*மதிப்பு_out = மதிப்பு;
	திரும்பு true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
நிலையான bool
layout__get_ol_start(dom_முனை *ol_முனை, dom_long *start_out)
{
	dom_exception exc;
	dom_long start;
	bool has_start;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_முனை,
			corestring_dom_start,
			&has_start);
	எனில் (exc != DOM_NO_ERR || has_start == false) {
		திரும்பு false;
	}

	exc = dom_html_olist_element_get_start(
			(dom_html_olist_element *)ol_முனை,
			&start);
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு false;
	}

	*start_out = start;
	திரும்பு true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
நிலையான bool
layout__get_ol_reversed(dom_முனை *ol_முனை)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_முனை,
			corestring_dom_reversed,
			&has_reversed);
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு false;
	}

	திரும்பு has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
நிலையான bool
layout_get_list_item_மொத்தம்(
		dom_முனை *list_owner, dom_long *மொத்தம்_out)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_முனை *child;
	முழு மொத்தம்;

	எனில் (list_owner == வெற்று) {
		திரும்பு false;
	}

	எனில் (!layout__get_element_tag(list_owner, &tag_type)) {
		திரும்பு false;
	}

	எனில் (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		திரும்பு false;
	}

	exc = dom_முனை_get_first_child(list_owner, &child);
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு false;
	}

	மொத்தம் = 0;
	போது (child != வெற்று) {
		dom_முனை *temp_முனை;

		எனில் (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			கட்டமைப்பு box *child_box;
			எனில் (dom_முனை_get_user_data(child,
					corestring_dom_ns_சாவி_box_முனை_data,
					&child_box) != DOM_NO_ERR) {
				dom_முனை_unref(child);
				திரும்பு false;
			}

			எனில் (child_box != வெற்று &&
			    child_box->list_marker != வெற்று) {
				மொத்தம்++;
			}
		}

		exc = dom_முனை_get_next_sibling(child, &temp_முனை);
		dom_முனை_unref(child);
		எனில் (exc != DOM_NO_ERR) {
			திரும்பு false;
		}

		child = temp_முனை;
	}

	*மொத்தம்_out = மொத்தம்;
	திரும்பு true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
நிலையான வெறுமை
layout_ordered_list_மொத்தம்(
		கட்டமைப்பு box *box)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_முனை *child;
	முழு step = 1;
	dom_long next;

	எனில் (box->முனை == வெற்று) {
		திரும்பு;
	}

	எனில் (!layout__get_element_tag(box->முனை, &tag_type)) {
		திரும்பு;
	}

	எனில் (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		திரும்பு;
	}

	next = 1;
	எனில் (tag_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_start = layout__get_ol_start(box->முனை, &next);
		bool have_reversed = layout__get_ol_reversed(box->முனை);

		எனில் (have_reversed) {
			step = -1;
		}

		எனில் (!have_start && have_reversed) {
			layout_get_list_item_மொத்தம்(box->முனை, &next);
		}
	}

	exc = dom_முனை_get_first_child(box->முனை, &child);
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு;
	}

	போது (child != வெற்று) {
		dom_முனை *temp_முனை;

		எனில் (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			கட்டமைப்பு box *child_box;

			எனில் (dom_முனை_get_user_data(child,
					corestring_dom_ns_சாவி_box_முனை_data,
					&child_box) != DOM_NO_ERR) {
				dom_முனை_unref(child);
				திரும்பு;
			}

			எனில் (child_box != வெற்று &&
			    child_box->list_marker != வெற்று) {
				dom_long மதிப்பு;
				கட்டமைப்பு box *marker = child_box->list_marker;
				எனில் (layout_get_li_மதிப்பு(child, &மதிப்பு)) {
					marker->list_மதிப்பு = மதிப்பு;
					next = marker->list_மதிப்பு;
				} இல்லையெனில் {
					marker->list_மதிப்பு = next;
				}
				next += step;
			}
		}

		exc = dom_முனை_get_next_sibling(child, &temp_முனை);
		dom_முனை_unref(child);
		எனில் (exc != DOM_NO_ERR) {
			திரும்பு;
		}

		child = temp_முனை;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
நிலையான வெறுமை
layout__set_numerical_marker_text(
		மாறிலி html_content *content,
		கட்டமைப்பு box *box)
{
	கட்டமைப்பு box *marker = box->list_marker;
	size_t counter_len;
	css_error css_res;
	எண்ணிக்கை {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_SIZE = 20,
	};

	marker->text = talloc_array(content->bctx, எழுத்து, LIST_MARKER_SIZE);
	எனில் (marker->text == வெற்று) {
		திரும்பு;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_மதிப்பு,
			marker->text, LIST_MARKER_SIZE, &counter_len);
	எனில் (css_res == CSS_OK) {
		எனில் (counter_len > LIST_MARKER_SIZE) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->text = talloc_realloc(content->bctx,
					marker->text,
					எழுத்து,
					counter_len);
			எனில் (marker->text == வெற்று) {
				திரும்பு;
			}
			css_computed_format_list_style(box->style,
					marker->list_மதிப்பு, marker->text,
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
நிலையான bool
layout__list_item_is_numerical(
		மாறிலி கட்டமைப்பு box *b)
{
	எண்ணிக்கை css_list_style_type_e t = css_computed_list_style_type(b->style);

	தேர்வு (t) {
	நிலை CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	நிலை CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	நிலை CSS_LIST_STYLE_TYPE_SQUARE: /* Fall through. */
	நிலை CSS_LIST_STYLE_TYPE_NONE:
		திரும்பு false;

	இயல்பு:
		திரும்பு true;
	}
}

/**
 * Layout list markers.
 */
நிலையான வெறுமை
layout_lists(மாறிலி html_content *content, கட்டமைப்பு box *box)
{
	கட்டமைப்பு box *child;

	layout_ordered_list_மொத்தம்(box);

	க்கு (child = box->children; child; child = child->next) {
		எனில் (child->list_marker) {
			கட்டமைப்பு box *marker = child->list_marker;

			எனில் (layout__list_item_is_numerical(child)) {
				எனில் (marker->text == வெற்று) {
					layout__set_numerical_marker_text(
							content, child);
				}
			}
			எனில் (marker->object) {
				marker->width =
					content_get_width(marker->object);
				marker->x = -marker->width;
				marker->height =
					content_get_height(marker->object);
				marker->y = (வரி_height(
						&content->unit_len_ctx,
						marker->style) -
						marker->height) / 2;
			} இல்லையெனில் எனில் (marker->text) {
				எனில் (marker->width == UNKNOWN_WIDTH) {
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
				marker->height = வரி_height(
						&content->unit_len_ctx,
						marker->style);
			} இல்லையெனில் {
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
நிலையான வெறுமை
layout_compute_offsets(மாறிலி css_unit_ctx *unit_len_ctx,
		       கட்டமைப்பு box *box,
		       கட்டமைப்பு box *containing_block,
		       முழு *உச்சி,
		       முழு *வலது,
		       முழு *bottom,
		       முழு *இடது)
{
	uint32_t type;
	css_fixed மதிப்பு = 0;
	css_unit unit = CSS_UNIT_PX;

	ப்ரதிஜ்ஞா(containing_block->width != UNKNOWN_WIDTH);
	ப்ரதிஜ்ஞா(containing_block->width != AUTO);
	ப்ரதிஜ்ஞா(containing_block->height != AUTO);

	/*இடது*/
	type = css_computed_இடது(box->style, &மதிப்பு, &unit);
	எனில் (type == CSS_இடது_SET) {
		எனில் (unit == CSS_UNIT_PCT) {
			*இடது = FPCT_OF_INT_TOINT(மதிப்பு,
					containing_block->width);
		} இல்லையெனில் {
			*இடது = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					மதிப்பு, unit));
		}
	} இல்லையெனில் {
		*இடது = AUTO;
	}

	/*வலது*/
	type = css_computed_வலது(box->style, &மதிப்பு, &unit);
	எனில் (type == CSS_வலது_SET) {
		எனில் (unit == CSS_UNIT_PCT) {
			*வலது = FPCT_OF_INT_TOINT(மதிப்பு,
					containing_block->width);
		} இல்லையெனில் {
			*வலது = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					மதிப்பு, unit));
		}
	} இல்லையெனில் {
		*வலது = AUTO;
	}

	/*உச்சி*/
	type = css_computed_உச்சி(box->style, &மதிப்பு, &unit);
	எனில் (type == CSS_உச்சி_SET) {
		எனில் (unit == CSS_UNIT_PCT) {
			*உச்சி = FPCT_OF_INT_TOINT(மதிப்பு,
					containing_block->height);
		} இல்லையெனில் {
			*உச்சி = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					மதிப்பு, unit));
		}
	} இல்லையெனில் {
		*உச்சி = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &மதிப்பு, &unit);
	எனில் (type == CSS_BOTTOM_SET) {
		எனில் (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_OF_INT_TOINT(மதிப்பு,
					containing_block->height);
		} இல்லையெனில் {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					மதிப்பு, unit));
		}
	} இல்லையெனில் {
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
நிலையான bool
layout_absolute(கட்டமைப்பு box *box,
		கட்டமைப்பு box *containing_block,
		முழு cx, முழு cy,
		html_content *content)
{
	முழு static_இடது, static_உச்சி;  /* static position */
	முழு உச்சி, வலது, bottom, இடது;
	முழு width, height, max_width, min_width;
	முழு *margin = box->margin;
	முழு *padding = box->padding;
	கட்டமைப்பு box_border *border = box->border;
	முழு available_width = containing_block->width;
	முழு space;

	ப்ரதிஜ்ஞா(box->type == BOX_BLOCK || box->type == BOX_TABLE ||
			box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_இடது = cx + box->x;
	static_உச்சி = cy + box->y;

	எனில் (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_block->width += containing_block->padding[இடது_2] +
				containing_block->padding[வலது_2];
		containing_block->height += containing_block->padding[உச்சி_2] +
				containing_block->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_block,
			&உச்சி, &வலது, &bottom, &இடது);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_block;
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1,
			box, box->style, &width, &height,
			&max_width, &min_width, 0, 0,
			margin, padding, border);
	box->float_container = வெற்று;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      இடது, margin[இடது_2], border[இடது_2].width, padding[இடது_2], width,
	      padding[வலது_2], border[வலது_2].width, margin[வலது_2], வலது,
	      containing_block->width);


	எனில் (இடது == AUTO && width == AUTO && வலது == AUTO) {
		எனில் (margin[இடது_2] == AUTO)
			margin[இடது_2] = 0;
		எனில் (margin[வலது_2] == AUTO)
			margin[வலது_2] = 0;
		இடது = static_இடது;

		width = min(max(box->min_width, available_width),
			box->max_width);
		width -= box->margin[இடது_2] + box->border[இடது_2].width +
			box->padding[இடது_2] + box->padding[வலது_2] +
			box->border[வலது_2].width + box->margin[வலது_2];

		/* Adjust for {min|max}-width */
		எனில் (max_width >= 0 && width > max_width) width = max_width;
		எனில் (width < min_width) width = min_width;

		வலது = containing_block->width -
			இடது -
			margin[இடது_2] - border[இடது_2].width - padding[இடது_2] -
			width -
			padding[வலது_2] - border[வலது_2].width - margin[வலது_2];
	} இல்லையெனில் எனில் (இடது != AUTO && width != AUTO && வலது != AUTO) {

		/* Adjust for {min|max}-width */
		எனில் (max_width >= 0 && width > max_width) width = max_width;
		எனில் (min_width >  0 && width < min_width) width = min_width;

		எனில் (margin[இடது_2] == AUTO && margin[வலது_2] == AUTO) {
			space = containing_block->width -
					இடது - border[இடது_2].width -
					padding[இடது_2] - width - padding[வலது_2] -
					border[வலது_2].width - வலது;
			எனில் (space < 0) {
				margin[இடது_2] = 0;
				margin[வலது_2] = space;
			} இல்லையெனில் {
				margin[இடது_2] = margin[வலது_2] = space / 2;
			}
		} இல்லையெனில் எனில் (margin[இடது_2] == AUTO) {
			margin[இடது_2] = containing_block->width -
					இடது - border[இடது_2].width -
					padding[இடது_2] - width - padding[வலது_2] -
					border[வலது_2].width - margin[வலது_2] -
					வலது;
		} இல்லையெனில் எனில் (margin[வலது_2] == AUTO) {
			margin[வலது_2] = containing_block->width -
					இடது - margin[இடது_2] -
					border[இடது_2].width -
					padding[இடது_2] - width - padding[வலது_2] -
					border[வலது_2].width - வலது;
		} இல்லையெனில் {
			வலது = containing_block->width -
					இடது - margin[இடது_2] -
					border[இடது_2].width -
					padding[இடது_2] - width - padding[வலது_2] -
					border[வலது_2].width - margin[வலது_2];
		}
	} இல்லையெனில் {
		எனில் (margin[இடது_2] == AUTO)
			margin[இடது_2] = 0;
		எனில் (margin[வலது_2] == AUTO)
			margin[வலது_2] = 0;

		எனில் (இடது == AUTO && width == AUTO && வலது != AUTO) {
			available_width -= வலது;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[இடது_2] + box->border[இடது_2].width +
				box->padding[இடது_2] + box->padding[வலது_2] +
				box->border[வலது_2].width + box->margin[வலது_2];

			/* Adjust for {min|max}-width */
			எனில் (max_width >= 0 && width > max_width)
				width = max_width;
			எனில் (width < min_width)
				width = min_width;

			இடது = containing_block->width -
					margin[இடது_2] - border[இடது_2].width -
					padding[இடது_2] - width - padding[வலது_2] -
					border[வலது_2].width - margin[வலது_2] -
					வலது;
		} இல்லையெனில் எனில் (இடது == AUTO && width != AUTO && வலது == AUTO) {

			/* Adjust for {min|max}-width */
			எனில் (max_width >= 0 && width > max_width)
				width = max_width;
			எனில் (min_width >  0 && width < min_width)
				width = min_width;

			இடது = static_இடது;
			வலது = containing_block->width -
					இடது - margin[இடது_2] -
					border[இடது_2].width -
					padding[இடது_2] - width - padding[வலது_2] -
					border[வலது_2].width - margin[வலது_2];
		} இல்லையெனில் எனில் (இடது != AUTO && width == AUTO && வலது == AUTO) {
			available_width -= இடது;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[இடது_2] + box->border[இடது_2].width +
				box->padding[இடது_2] + box->padding[வலது_2] +
				box->border[வலது_2].width + box->margin[வலது_2];

			/* Adjust for {min|max}-width */
			எனில் (max_width >= 0 && width > max_width)
				width = max_width;
			எனில் (width < min_width)
				width = min_width;

			வலது = containing_block->width -
					இடது - margin[இடது_2] -
					border[இடது_2].width -
					padding[இடது_2] - width - padding[வலது_2] -
					border[வலது_2].width - margin[வலது_2];
		} இல்லையெனில் எனில் (இடது == AUTO && width != AUTO && வலது != AUTO) {

			/* Adjust for {min|max}-width */
			எனில் (max_width >= 0 && width > max_width)
				width = max_width;
			எனில் (width < min_width)
				width = min_width;

			இடது = containing_block->width -
					margin[இடது_2] - border[இடது_2].width -
					padding[இடது_2] - width - padding[வலது_2] -
					border[வலது_2].width - margin[வலது_2] -
					வலது;
		} இல்லையெனில் எனில் (இடது != AUTO && width == AUTO && வலது != AUTO) {
			width = containing_block->width -
					இடது - margin[இடது_2] -
					border[இடது_2].width -
					padding[இடது_2] - padding[வலது_2] -
					border[வலது_2].width - margin[வலது_2] -
					வலது;

			/* Adjust for {min|max}-width */
			எனில் (max_width >= 0 && width > max_width)
				width = max_width;
			எனில் (width < min_width)
				width = min_width;

		} இல்லையெனில் எனில் (இடது != AUTO && width != AUTO && வலது == AUTO) {

			/* Adjust for {min|max}-width */
			எனில் (max_width >= 0 && width > max_width)
				width = max_width;
			எனில் (width < min_width)
				width = min_width;

			வலது = containing_block->width -
					இடது - margin[இடது_2] -
					border[இடது_2].width -
					padding[இடது_2] - width - padding[வலது_2] -
					border[வலது_2].width - margin[வலது_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      இடது, margin[இடது_2], border[இடது_2].width, padding[இடது_2], width,
	      padding[வலது_2], border[வலது_2].width, margin[வலது_2], வலது,
	      containing_block->width);

	box->x = இடது + margin[இடது_2] + border[இடது_2].width - cx;
	எனில் (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_block->width -= containing_block->padding[இடது_2] +
				containing_block->padding[வலது_2];
	} இல்லையெனில் {
		/** \todo inline ancestors */
	}
	box->width = width;
	box->height = height;

	எனில் (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->object || box->flags & IFRAME) {
		எனில் (!layout_block_context(box, -1, content))
			திரும்பு false;
	} இல்லையெனில் எனில் (box->type == BOX_TABLE) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		/* \todo  layout_table considers margins etc. again */
		எனில் (!layout_table(box, width, content))
			திரும்பு false;
		box->float_container = வெற்று;
		layout_solve_width(box, box->parent->width, box->width, 0, 0,
				-1, -1);
	} இல்லையெனில் எனில் (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		எனில் (!layout_flex(box, width, content))
			திரும்பு false;
		box->float_container = வெற்று;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      உச்சி, margin[உச்சி_2], border[உச்சி_2].width, padding[உச்சி_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	எனில் (உச்சி == AUTO && height == AUTO && bottom == AUTO) {
		உச்சி = static_உச்சி;
		height = box->height;
		எனில் (margin[உச்சி_2] == AUTO)
			margin[உச்சி_2] = 0;
		எனில் (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_block->height -
				உச்சி - margin[உச்சி_2] - border[உச்சி_2].width -
				padding[உச்சி_2] - height - padding[BOTTOM] -
				border[BOTTOM].width - margin[BOTTOM];
	} இல்லையெனில் எனில் (உச்சி != AUTO && height != AUTO && bottom != AUTO) {
		எனில் (margin[உச்சி_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_block->height -
					உச்சி - border[உச்சி_2].width - padding[உச்சி_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - bottom;
			margin[உச்சி_2] = margin[BOTTOM] = space / 2;
		} இல்லையெனில் எனில் (margin[உச்சி_2] == AUTO) {
			margin[உச்சி_2] = containing_block->height -
					உச்சி - border[உச்சி_2].width - padding[உச்சி_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} இல்லையெனில் எனில் (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_block->height -
					உச்சி - margin[உச்சி_2] - border[உச்சி_2].width -
					padding[உச்சி_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					bottom;
		} இல்லையெனில் {
			bottom = containing_block->height -
					உச்சி - margin[உச்சி_2] - border[உச்சி_2].width -
					padding[உச்சி_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	} இல்லையெனில் {
		எனில் (margin[உச்சி_2] == AUTO)
			margin[உச்சி_2] = 0;
		எனில் (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		எனில் (உச்சி == AUTO && height == AUTO && bottom != AUTO) {
			height = box->height;
			உச்சி = containing_block->height -
					margin[உச்சி_2] - border[உச்சி_2].width -
					padding[உச்சி_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} இல்லையெனில் எனில் (உச்சி == AUTO && height != AUTO && bottom == AUTO) {
			உச்சி = static_உச்சி;
			bottom = containing_block->height -
					உச்சி - margin[உச்சி_2] - border[உச்சி_2].width -
					padding[உச்சி_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} இல்லையெனில் எனில் (உச்சி != AUTO && height == AUTO && bottom == AUTO) {
			height = box->height;
			bottom = containing_block->height -
					உச்சி - margin[உச்சி_2] - border[உச்சி_2].width -
					padding[உச்சி_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} இல்லையெனில் எனில் (உச்சி == AUTO && height != AUTO && bottom != AUTO) {
			உச்சி = containing_block->height -
					margin[உச்சி_2] - border[உச்சி_2].width -
					padding[உச்சி_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} இல்லையெனில் எனில் (உச்சி != AUTO && height == AUTO && bottom != AUTO) {
			height = containing_block->height -
					உச்சி - margin[உச்சி_2] - border[உச்சி_2].width -
					padding[உச்சி_2] - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} இல்லையெனில் எனில் (உச்சி != AUTO && height != AUTO && bottom == AUTO) {
			bottom = containing_block->height -
					உச்சி - margin[உச்சி_2] - border[உச்சி_2].width -
					padding[உச்சி_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      உச்சி, margin[உச்சி_2], border[உச்சி_2].width, padding[உச்சி_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	box->y = உச்சி + margin[உச்சி_2] + border[உச்சி_2].width - cy;
	எனில் (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_block->height -= containing_block->padding[உச்சி_2] +
				containing_block->padding[BOTTOM];
	} இல்லையெனில் {
		/** \todo Inline ancestors */
	}
	box->height = height;
	layout_apply_minmax_height(&content->unit_len_ctx, box, containing_block);

	திரும்பு true;
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
நிலையான bool
layout_position_absolute(கட்டமைப்பு box *box,
			 கட்டமைப்பு box *containing_block,
			 முழு cx, முழு cy,
			 html_content *content)
{
	கட்டமைப்பு box *c;

	க்கு (c = box->children; c; c = c->next) {
		எனில் ((c->type == BOX_BLOCK || c->type == BOX_TABLE ||
				c->type == BOX_INLINE_BLOCK ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			எனில் (!layout_absolute(c, containing_block,
					cx, cy, content))
				திரும்பு false;
			எனில் (!layout_position_absolute(c, c, 0, 0, content))
				திரும்பு false;
		} இல்லையெனில் எனில் (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			எனில் (!layout_position_absolute(c, c, 0, 0, content))
				திரும்பு false;
		} இல்லையெனில் {
			முழு px, py;
			எனில் (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_இடது ||
					css_computed_float(c->style) ==
						CSS_FLOAT_வலது)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				கட்டமைப்பு box *p;
				px = c->x;
				py = c->y;
				க்கு (p = box->parent; p && !p->float_children;
						p = p->parent) {
					px -= p->x;
					py -= p->y;
				}
			} இல்லையெனில் {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			எனில் (!layout_position_absolute(c, containing_block,
					cx + px, cy + py, content))
				திரும்பு false;
		}
	}

	திரும்பு true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
நிலையான வெறுமை layout_compute_relative_offset(
		மாறிலி css_unit_ctx *unit_len_ctx,
		கட்டமைப்பு box *box,
		முழு *x,
		முழு *y)
{
	முழு இடது, வலது, உச்சி, bottom;
	கட்டமைப்பு box *containing_block;

	ப்ரதிஜ்ஞா(box && box->parent && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	எனில் (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_இடது ||
			css_computed_float(box->style) == CSS_FLOAT_வலது)) {
		containing_block = box->float_container;
	} இல்லையெனில் {
		containing_block = box->parent;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_block,
			&உச்சி, &வலது, &bottom, &இடது);

	எனில் (இடது == AUTO && வலது == AUTO)
		இடது = வலது = 0;
	இல்லையெனில் எனில் (இடது == AUTO)
		/* left is auto => computed = -right */
		இடது = -வலது;
	இல்லையெனில் எனில் (வலது == AUTO)
		/* right is auto => computed = -left */
		வலது = -இடது;
	இல்லையெனில் {
		/* over constrained => examine direction property
		 * of containing block */
		எனில் (containing_block->style &&
				css_computed_direction(
				containing_block->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			இடது = -வலது;
		} இல்லையெனில் {
			/* assume LTR in all other cases */
			வலது = -இடது;
		}
	}

	ப்ரதிஜ்ஞா(இடது == -வலது);

	எனில் (உச்சி == AUTO && bottom == AUTO) {
		உச்சி = bottom = 0;
	} இல்லையெனில் எனில் (உச்சி == AUTO) {
		உச்சி = -bottom;
	} இல்லையெனில் {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -உச்சி;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", இடது,
	      வலது, உச்சி, bottom);

	*x = இடது;
	*y = உச்சி;
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
நிலையான வெறுமை
layout_position_relative(
		மாறிலி css_unit_ctx *unit_len_ctx,
		கட்டமைப்பு box *வேர்,
		கட்டமைப்பு box *fp,
		முழு fx,
		முழு fy)
{
	கட்டமைப்பு box *box; /* for children of "root" */
	கட்டமைப்பு box *fn;  /* for block formatting context box for children of
			  * "box" */
	கட்டமைப்பு box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	முழு x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	முழு fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	ப்ரதிஜ்ஞா(வேர்);

	/* Normal children */
	க்கு (box = வேர்->children; box; box = box->next) {

		எனில் (box->type == BOX_TEXT)
			தொடர்;

		/* If relatively positioned, get offsets */
		எனில் (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_offset(
					unit_len_ctx, box, &x, &y);
		இல்லையெனில்
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		எனில் (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_இடது ||
				css_computed_float(box->style) ==
					CSS_FLOAT_வலது) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			க்கு (fc = fp->float_children; fc; fc = fc->next_float) {
				எனில் (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		எனில் (box->float_children) {
			fn = box;
			fnx = fny = 0;
		} இல்லையெனில் {
			fn = fp;
			fnx = fx + x;
			fny = fy + y;
		}

		/* recurse first */
		layout_position_relative(unit_len_ctx, box, fn, fnx, fny);

		/* Ignore things we're not interested in. */
		எனில் (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			தொடர்;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		எனில் (box->type == BOX_INLINE && box->inline_end) {
			கட்டமைப்பு box *b;
			க்கு (b = box->next; b && b != box->inline_end;
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
நிலையான வெறுமை
layout_get_box_bbox(
		மாறிலி css_unit_ctx *unit_len_ctx,
		கட்டமைப்பு box *box,
		முழு *desc_x0, முழு *desc_y0,
		முழு *desc_x1, முழு *desc_y1)
{
	*desc_x0 = -box->border[இடது_2].width;
	*desc_y0 = -box->border[உச்சி_2].width;
	*desc_x1 = box->padding[இடது_2] + box->width + box->padding[வலது_2] +
			box->border[வலது_2].width;
	*desc_y1 = box->padding[உச்சி_2] + box->height + box->padding[BOTTOM] +
			box->border[BOTTOM].width;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	எனில் (box->type == BOX_BLOCK && box->style != வெற்று &&
			css_computed_overflow_y(box->style) ==
					CSS_OVERFLOW_VISIBLE &&
			box->object == வெற்று) {
		css_fixed font_size = 0;
		css_unit font_unit = CSS_UNIT_PT;
		முழு text_height;

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
நிலையான வெறுமை
layout_update_descendant_bbox(
		மாறிலி css_unit_ctx *unit_len_ctx,
		கட்டமைப்பு box *box,
		கட்டமைப்பு box *child,
		முழு off_x,
		முழு off_y)
{
	முழு child_desc_x0, child_desc_y0, child_desc_x1, child_desc_y1;

	/* get coordinates of child relative to box */
	முழு child_x = child->x - off_x;
	முழு child_y = child->y - off_y;

	bool html_object = (child->object &&
			content_get_type(child->object) == CONTENT_HTML);

	எண்ணிக்கை css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	எண்ணிக்கை css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

	எனில் (child->style != வெற்று) {
		overflow_x = css_computed_overflow_x(child->style);
		overflow_y = css_computed_overflow_y(child->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, child,
			&child_desc_x0, &child_desc_y0,
			&child_desc_x1, &child_desc_y1);

	எனில் (overflow_x == CSS_OVERFLOW_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		child_desc_x0 = child->descendant_x0;
		child_desc_x1 = child->descendant_x1;
	}
	எனில் (overflow_y == CSS_OVERFLOW_VISIBLE &&
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
	எனில் (child_desc_x0 < box->descendant_x0)
		box->descendant_x0 = child_desc_x0;
	எனில் (child_desc_y0 < box->descendant_y0)
		box->descendant_y0 = child_desc_y0;
	எனில் (box->descendant_x1 < child_desc_x1)
		box->descendant_x1 = child_desc_x1;
	எனில் (box->descendant_y1 < child_desc_y1)
		box->descendant_y1 = child_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
நிலையான வெறுமை layout_calculate_descendant_bboxes(
		மாறிலி css_unit_ctx *unit_len_ctx,
		கட்டமைப்பு box *box)
{
	கட்டமைப்பு box *child;

	ப்ரதிஜ்ஞா(box->width != UNKNOWN_WIDTH);
	ப்ரதிஜ்ஞா(box->height != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	எனில் (box->object && content_get_type(box->object) == CONTENT_HTML) {
		எனில் (box->descendant_x1 < content_get_width(box->object))
			box->descendant_x1 = content_get_width(box->object);
		எனில் (box->descendant_y1 < content_get_height(box->object))
			box->descendant_y1 = content_get_height(box->object);
	}

	எனில் (box->iframe != வெற்று) {
		முழு x, y;
		box_coords(box, &x, &y);

		browser_window_set_position(box->iframe, x, y);
		browser_window_set_dimensions(box->iframe,
				box->width, box->height);
		browser_window_reformat(box->iframe, true,
				box->width, box->height);
	}

	எனில் (box->type == BOX_INLINE || box->type == BOX_TEXT)
		திரும்பு;

	எனில் (box->type == BOX_INLINE_END) {
		box = box->inline_end;
		க்கு (child = box->next; child;
				child = child->next) {
			எனில் (child->type == BOX_FLOAT_இடது ||
					child->type == BOX_FLOAT_வலது)
				தொடர்;

			layout_update_descendant_bbox(unit_len_ctx, box, child,
					box->x, box->y);

			எனில் (child == box->inline_end)
				நிறுத்து;
		}
		திரும்பு;
	}

	எனில் (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		திரும்பு;

	க்கு (child = box->children; child; child = child->next) {
		எனில் (child->type == BOX_FLOAT_இடது ||
				child->type == BOX_FLOAT_வலது)
			தொடர்;

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		எனில் (box->style && css_computed_overflow_x(box->style) ==
				CSS_OVERFLOW_HIDDEN &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_HIDDEN)
			தொடர்;

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	க்கு (child = box->float_children; child; child = child->next_float) {
		ப்ரதிஜ்ஞா(child->type == BOX_FLOAT_இடது ||
				child->type == BOX_FLOAT_வலது);

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	எனில் (box->list_marker) {
		child = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, முழு width, முழு height)
{
	bool ret;
	கட்டமைப்பு box *doc = content->layout;
	மாறிலி கட்டமைப்பு gui_layout_table *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			width, height, nsurl_access(content_get_url(
					&content->base)));

	layout_minmax_block(doc, font_func, content);

	layout_block_find_dimensions(&content->unit_len_ctx,
			width, height, 0, 0, doc);
	doc->x = doc->margin[இடது_2] + doc->border[இடது_2].width;
	doc->y = doc->margin[உச்சி_2] + doc->border[உச்சி_2].width;
	width -= doc->margin[இடது_2] + doc->border[இடது_2].width +
			doc->padding[இடது_2] + doc->padding[வலது_2] +
			doc->border[வலது_2].width + doc->margin[வலது_2];
	எனில் (width < 0) {
		width = 0;
	}
	doc->width = width;

	ret = layout_block_context(doc, height, content);

	/* make <html> and <body> fill available height */
	எனில் (doc->y + doc->padding[உச்சி_2] + doc->height + doc->padding[BOTTOM] +
			doc->border[BOTTOM].width + doc->margin[BOTTOM] <
			height) {
		doc->height = height - (doc->y + doc->padding[உச்சி_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].width +
				doc->margin[BOTTOM]);
		எனில் (doc->children)
			doc->children->height = doc->height -
					(doc->children->margin[உச்சி_2] +
					 doc->children->border[உச்சி_2].width +
					 doc->children->padding[உச்சி_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].width +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	திரும்பு ret;
}
