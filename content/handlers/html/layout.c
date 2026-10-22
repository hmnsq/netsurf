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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <math.h>
#अन्तर्भाव <dom/dom.h>

#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/talloc.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "netsurf/browser_window.h"
#अन्तर्भाव "netsurf/layout.h"
#अन्तर्भाव "content/content.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "css/utils.h"
#अन्तर्भाव "desktop/scrollbar.h"
#अन्तर्भाव "desktop/textarea.h"

#अन्तर्भाव "html/html.h"
#अन्तर्भाव "html/html_save.h"
#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_inspect.h"
#अन्तर्भाव "html/font.h"
#अन्तर्भाव "html/form_internal.h"
#अन्तर्भाव "html/layout.h"
#अन्तर्भाव "html/layout_internal.h"
#अन्तर्भाव "html/table.h"

/** Array of per-side access functions for computed style margins. */
ध्रुव css_len_func margin_funcs[4] = {
	[शीर्ष_2]    = css_computed_margin_शीर्ष,
	[उजवा_2]  = css_computed_margin_उजवा,
	[BOTTOM] = css_computed_margin_bottom,
	[डावा_2]   = css_computed_margin_डावा,
};

/** Array of per-side access functions for computed style paddings. */
ध्रुव css_len_func padding_funcs[4] = {
	[शीर्ष_2]    = css_computed_padding_शीर्ष,
	[उजवा_2]  = css_computed_padding_उजवा,
	[BOTTOM] = css_computed_padding_bottom,
	[डावा_2]   = css_computed_padding_डावा,
};

/** Array of per-side access functions for computed style border_widths. */
ध्रुव css_len_func border_width_funcs[4] = {
	[शीर्ष_2]    = css_computed_border_शीर्ष_width,
	[उजवा_2]  = css_computed_border_उजवा_width,
	[BOTTOM] = css_computed_border_bottom_width,
	[डावा_2]   = css_computed_border_डावा_width,
};

/** Array of per-side access functions for computed style border styles. */
ध्रुव css_border_style_func border_style_funcs[4] = {
	[शीर्ष_2]    = css_computed_border_शीर्ष_style,
	[उजवा_2]  = css_computed_border_उजवा_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[डावा_2]   = css_computed_border_डावा_style,
};

/** Array of per-side access functions for computed style border colors. */
ध्रुव css_border_color_func border_color_funcs[4] = {
	[शीर्ष_2]    = css_computed_border_शीर्ष_color,
	[उजवा_2]  = css_computed_border_उजवा_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[डावा_2]   = css_computed_border_डावा_color,
};

/* forward declaration to break cycles */
स्थिर शून्य layout_minmax_block(
		संरचना box *block,
		ध्रुव संरचना gui_layout_table *font_func,
		ध्रुव html_content *content);

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
स्थिर शून्य
layout_get_object_dimensions(संरचना box *box,
			     पूर्णाङ्क *width, पूर्णाङ्क *height,
			     पूर्णाङ्क min_width, पूर्णाङ्क max_width,
			     पूर्णाङ्क min_height, पूर्णाङ्क max_height)
{
	प्रतिज्ञा(box->object != रिक्त);
	प्रतिज्ञा(width != रिक्त && height != रिक्त);

	यदि (*width == AUTO && *height == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		पूर्णाङ्क intrinsic_width = content_get_width(box->object);
		पूर्णाङ्क intrinsic_height = content_get_height(box->object);

		/* use intrinsic dimensions */
		*width = intrinsic_width;
		*height = intrinsic_height;

		/* Deal with min/max-width first */
		यदि (min_width >  0 && min_width > *width) {
			*width = min_width;
			scaled = true;
		}
		यदि (max_width >= 0 && max_width < *width) {
			*width = max_width;
			scaled = true;
		}

		यदि (scaled && (intrinsic_width != 0)) {
			/* Update height */
			*height = (*width * intrinsic_height) /
					intrinsic_width;
		}

		scaled = false;
		/* Deal with min/max-height */
		यदि (min_height >  0 && min_height > *height) {
			*height = min_height;
			scaled = true;
		}
		यदि (max_height >= 0 && max_height < *height) {
			*height = max_height;
			scaled = true;
		}

		यदि (scaled && (intrinsic_height != 0)) {
			/* Update width */
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		}

	} अन्यथा यदि (*width == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		पूर्णाङ्क intrinsic_width = content_get_width(box->object);
		पूर्णाङ्क intrinsic_height = content_get_height(box->object);

		यदि (intrinsic_height != 0)
			*width = (*height * intrinsic_width) /
					intrinsic_height;
		अन्यथा
			*width = intrinsic_width;

		यदि (min_width >  0 && min_width > *width)
			*width = min_width;
		यदि (max_width >= 0 && max_width < *width)
			*width = max_width;

	} अन्यथा यदि (*height == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		पूर्णाङ्क intrinsic_width = content_get_width(box->object);
		पूर्णाङ्क intrinsic_height = content_get_height(box->object);

		यदि (min_width >  0 && min_width > *width)
			*width = min_width;
		यदि (max_width >= 0 && max_width < *width)
			*width = max_width;

		यदि (intrinsic_width != 0)
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
स्थिर पूर्णाङ्क layout_text_indent(
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव css_computed_style *style, पूर्णाङ्क width)
{
	css_fixed मूल्य = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_text_indent(style, &मूल्य, &unit);

	यदि (unit == CSS_UNIT_PCT) {
		प्रत्यागम FPCT_OF_INT_TOINT(मूल्य, width);
	} अन्यथा {
		प्रत्यागम FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
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
स्थिर शून्य layout_minmax_table(संरचना box *table,
		ध्रुव संरचना gui_layout_table *font_func,
		ध्रुव html_content *content)
{
	unsigned पूर्णाङ्क i, j;
	पूर्णाङ्क width;
	पूर्णाङ्क border_spacing_h = 0;
	पूर्णाङ्क table_min = 0, table_max = 0;
	पूर्णाङ्क extra_fixed = 0;
	भिन्न extra_frac = 0;
	संरचना column *col;
	संरचना box *row_group, *row, *cell;

	/* check if the widths have already been calculated */
	यदि (table->max_width != UNKNOWN_MAX_WIDTH)
		प्रत्यागम;

	यदि (table_calculate_column_types(&content->unit_len_ctx, table) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		प्रत्यागम;
	}
	col = table->col;

	/* start with 0 except for fixed-width columns */
	प्रति (i = 0; i != table->columns; i++) {
		यदि (col[i].type == COLUMN_WIDTH_FIXED)
			col[i].min = col[i].max = col[i].width;
		अन्यथा
			col[i].min = col[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	यदि (css_computed_border_collapse(table->style) ==
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
	प्रति (row_group = table->children; row_group; row_group =row_group->next)
	प्रति (row = row_group->children; row; row = row->next)
	प्रति (cell = row->children; cell; cell = cell->next) {
		प्रतिज्ञा(cell->type == BOX_TABLE_CELL);
		प्रतिज्ञा(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		प्रतिज्ञा(cell->columns != 0);

		यदि (cell->columns != 1)
			अनुवर्त;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		यदि (col[i].positioned)
			अनुवर्त;

		/* update column min, max widths using cell widths */
		यदि (col[i].min < cell->min_width)
			col[i].min = cell->min_width;
		यदि (col[i].max < cell->max_width)
			col[i].max = cell->max_width;
	}

	/* 2nd pass: cells which span multiple columns */
	प्रति (row_group = table->children; row_group; row_group =row_group->next)
	प्रति (row = row_group->children; row; row = row->next)
	प्रति (cell = row->children; cell; cell = cell->next) {
		unsigned पूर्णाङ्क flexible_columns = 0;
		पूर्णाङ्क min = 0, max = 0, fixed_width = 0, extra;

		यदि (cell->columns == 1)
			अनुवर्त;

		layout_minmax_block(cell, font_func, content);
		i = cell->start_column;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		प्रति (j = 0; j != cell->columns; j++) {
			min += col[i + j].min;
			यदि (col[i + j].type == COLUMN_WIDTH_FIXED)
				fixed_width += col[i + j].width;
			अन्यथा
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		यदि (min < cell->min_width) {
			यदि (flexible_columns == 0) {
				extra = 1 + (cell->min_width - min) /
						cell->columns;
				प्रति (j = 0; j != cell->columns; j++) {
					col[i + j].min += extra;
					यदि (col[i + j].max < col[i + j].min)
						col[i + j].max = col[i + j].min;
				}
			} अन्यथा {
				extra = 1 + (cell->min_width - min) /
						flexible_columns;
				प्रति (j = 0; j != cell->columns; j++) {
					यदि (col[i + j].type !=
							COLUMN_WIDTH_FIXED) {
						col[i + j].min += extra;
						यदि (col[i + j].max <
								col[i + j].min)
							col[i + j].max =
								col[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		प्रति (j = 0; j != cell->columns; j++)
			max += col[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		यदि (max < cell->max_width && flexible_columns) {
			extra = 1 + (cell->max_width - max) / flexible_columns;
			प्रति (j = 0; j != cell->columns; j++)
				यदि (col[i + j].type != COLUMN_WIDTH_FIXED)
					col[i + j].max += extra;
		}
	}

	प्रति (i = 0; i != table->columns; i++) {
		यदि (col[i].max < col[i].min) {
			box_dump(stderr, table, 0, true);
			प्रतिज्ञा(0);
		}
		table_min += col[i].min;
		table_max += col[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	यदि (css_computed_width_px(table->style, &content->unit_len_ctx,
			-1, &width) == CSS_WIDTH_SET) {
		यदि (table_min < width)
			table_min = width;
		यदि (table_max < width)
			table_max = width;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, डावा_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx,
			table->style, उजवा_2, true, true, true,
			&extra_fixed, &extra_frac);
	यदि (extra_fixed < 0)
		extra_fixed = 0;
	यदि (extra_frac < 0)
		extra_frac = 0;
	यदि (1.0 <= extra_frac)
		extra_frac = 0.9;
	table->min_width = (table_min + extra_fixed) / (1.0 - extra_frac);
	table->max_width = (table_max + extra_fixed) / (1.0 - extra_frac);
	table->min_width += (table->columns + 1) * border_spacing_h;
	table->max_width += (table->columns + 1) * border_spacing_h;

	प्रतिज्ञा(0 <= table->min_width && table->min_width <= table->max_width);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
स्थिर अन्तःस्थ bool box_has_percentage_max_width(संरचना box *b)
{
	css_unit unit = CSS_UNIT_PX;
	गणना css_max_width_e type;
	css_fixed मूल्य = 0;

	प्रतिज्ञा(b != रिक्त);

	type = css_computed_max_width(b->style, &मूल्य, &unit);
	प्रत्यागम ((type == CSS_MAX_WIDTH_SET) && (unit == CSS_UNIT_PCT));
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
स्थिर संरचना box *
layout_minmax_ओळ(संरचना box *first,
		   पूर्णाङ्क *ओळ_min,
		   पूर्णाङ्क *ओळ_max,
		   bool first_ओळ,
		   bool *ओळ_has_height,
		   ध्रुव संरचना gui_layout_table *font_func,
		   ध्रुव html_content *content)
{
	पूर्णाङ्क min = 0, max = 0, width, height, fixed;
	भिन्न frac;
	size_t i, j;
	संरचना box *b;
	संरचना box *block;
	plot_font_style_t fstyle;
	bool no_wrap;

	प्रतिज्ञा(first->parent);
	प्रतिज्ञा(first->parent->parent);
	प्रतिज्ञा(first->parent->parent->style);

	block = first->parent->parent;
	no_wrap = (css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(block->style) ==
			CSS_WHITE_SPACE_PRE);

	*ओळ_has_height = false;

	/* corresponds to the pass 1 loop in layout_line() */
	प्रति (b = first; b; b = b->next) {
		गणना css_width_e wtype;
		गणना css_height_e htype;
		गणना css_box_sizing_e bs;
		css_fixed मूल्य = 0;
		css_unit unit = CSS_UNIT_PX;

		प्रतिज्ञा(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		यदि (b->type == BOX_BR) {
			b = b->next;
			भङ्ग;
		}

		यदि (lh__box_is_float_box(b)) {
			प्रतिज्ञा(b->children);
			यदि (b->children->type == BOX_TABLE)
				layout_minmax_table(b->children, font_func,
						content);
			अन्यथा
				layout_minmax_block(b->children, font_func,
						content);
			b->min_width = b->children->min_width;
			b->max_width = b->children->max_width;
			यदि (min < b->min_width)
				min = b->min_width;
			max += b->max_width;
			अनुवर्त;
		}

		यदि (b->type == BOX_INLINE_BLOCK || b->type == BOX_INLINE_FLEX) {
			layout_minmax_block(b, font_func, content);
			यदि (min < b->min_width)
				min = b->min_width;
			max += b->max_width;

			यदि (b->flags & HAS_HEIGHT)
				*ओळ_has_height = true;
			अनुवर्त;
		}

		प्रतिज्ञा(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		यदि (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->style, डावा_2, true, true, true,
					&fixed, &frac);
			यदि (!b->inline_end)
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, उजवा_2,
						true, true, true,
						&fixed, &frac);
			यदि (0 < fixed)
				max += fixed;
			*ओळ_has_height = true;
			/* \todo  update min width, consider fractional extra */
		} अन्यथा यदि (b->type == BOX_INLINE_END) {
			fixed = frac = 0;
			calculate_mbp_width(&content->unit_len_ctx,
					b->inline_end->style, उजवा_2,
					true, true, true,
					&fixed, &frac);
			यदि (0 < fixed)
				max += fixed;

			यदि (b->next) {
				यदि (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*ओळ_has_height = true;
			अनुवर्त;
		}

		यदि (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			यदि (!b->text)
				अनुवर्त;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			यदि (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				यदि (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					पूर्णाङ्क opt_maxwidth = 0;
					संरचना form_option *o;

					प्रति (o = b->parent->parent->gadget->
							data.select.वस्तुसंच; o;
							o = o->next) {
						पूर्णाङ्क opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						यदि (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}

					b->width = opt_maxwidth;
					यदि (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;

				} अन्यथा {
					font_func->width(&fstyle, b->text,
						b->length, &b->width);
					b->flags |= MEASURED;
				}
			}
			max += b->width;
			यदि (b->next) {
				यदि (b->space == UNKNOWN_WIDTH) {
					font_func->width(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			यदि (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} अन्यथा यदि (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				यदि (min < b->width)
					min = b->width;

			} अन्यथा यदि (b->parent->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				कुरु {
					प्रति (j = i; j != b->length &&
							b->text[j] != ' '; j++)
						;
					font_func->width(&fstyle, b->text + i,
							 j - i, &width);
					यदि (min < width)
						min = width;
					i = j + 1;
				} यावत् (j != b->length);
			}

			*ओळ_has_height = true;

			अनुवर्त;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		प्रतिज्ञा(b->style);

		bs = css_computed_box_sizing(block->style);

		/* calculate box width */
		wtype = css_computed_width_px(b->style,
				&content->unit_len_ctx, -1, &width);
		यदि (wtype == CSS_WIDTH_SET) {
			यदि (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, डावा_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						block->style, उजवा_2,
						false, true, true,
						&fixed, &frac);
				यदि (width < fixed) {
					width = fixed;
				}
			}
			यदि (width < 0)
				width = 0;
		} अन्यथा {
			width = AUTO;
		}

		/* height */
		htype = css_computed_height(b->style, &मूल्य, &unit);
		यदि (htype == CSS_HEIGHT_SET) {
			height = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					मूल्य, unit));
		} अन्यथा {
			height = AUTO;
		}

		यदि (b->object || (b->flags & REPLACE_DIM)) {
			यदि (b->object) {
				पूर्णाङ्क temp_height = height;
				layout_get_object_dimensions(b,
						&width, &temp_height,
						INT_MIN, INT_MAX,
						INT_MIN, INT_MAX);
			}

			fixed = frac = 0;
			यदि (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, डावा_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, उजवा_2,
						true, false, false,
						&fixed, &frac);
			} अन्यथा {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, डावा_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, उजवा_2,
						true, true, true,
						&fixed, &frac);
			}
			यदि (0 < width + fixed)
				width += fixed;
		} अन्यथा यदि (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			यदि (width == AUTO)
				width = 400;

			fixed = frac = 0;
			यदि (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, डावा_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, उजवा_2,
						true, false, false,
						&fixed, &frac);
			} अन्यथा {
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, डावा_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_width(&content->unit_len_ctx,
						b->style, उजवा_2,
						true, true, true,
						&fixed, &frac);
			}

			यदि (0 < width + fixed)
				width += fixed;

		} अन्यथा {
			/* form control with no object */
			यदि (width == AUTO)
				width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		यदि (min < width && !box_has_percentage_max_width(b))
			min = width;
		यदि (width > 0)
			max += width;

		*ओळ_has_height = true;
	}

	यदि (first_ओळ) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		पूर्णाङ्क text_indent = layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, 100);
		min = (min + text_indent < 0) ? 0 : min + text_indent;
		max = (max + text_indent < 0) ? 0 : max + text_indent;
	}

	*ओळ_min = min;
	*ओळ_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	प्रतिज्ञा(b != first);
	प्रतिज्ञा(0 <= *ओळ_min);
	प्रतिज्ञा(*ओळ_min <= *ओळ_max);
	प्रत्यागम b;
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
स्थिर शून्य
layout_minmax_inline_container(संरचना box *inline_container,
			       bool *has_height,
			       ध्रुव संरचना gui_layout_table *font_func,
			       ध्रुव html_content *content)
{
	संरचना box *child;
	पूर्णाङ्क ओळ_min = 0, ओळ_max = 0;
	पूर्णाङ्क min = 0, max = 0;
	bool first_ओळ = true;
	bool ओळ_has_height;

	प्रतिज्ञा(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	यदि (inline_container->max_width != UNKNOWN_MAX_WIDTH)
		प्रत्यागम;

	*has_height = false;

	प्रति (child = inline_container->children; child; ) {
		child = layout_minmax_ओळ(child, &ओळ_min, &ओळ_max,
				first_ओळ, &ओळ_has_height, font_func,
				content);
		यदि (min < ओळ_min)
			min = ओळ_min;
		यदि (max < ओळ_max)
			max = ओळ_max;
		first_ओळ = false;
		*has_height |= ओळ_has_height;
	}

	inline_container->min_width = min;
	inline_container->max_width = max;

	प्रतिज्ञा(0 <= inline_container->min_width &&
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
स्थिर शून्य layout_minmax_block(
		संरचना box *block,
		ध्रुव संरचना gui_layout_table *font_func,
		ध्रुव html_content *content)
{
	संरचना box *child;
	पूर्णाङ्क min = 0, max = 0;
	पूर्णाङ्क extra_fixed = 0;
	भिन्न extra_frac = 0;
	गणना css_width_e wtype = CSS_WIDTH_AUTO;
	css_fixed width = 0;
	css_unit wunit = CSS_UNIT_PX;
	गणना css_height_e htype = CSS_HEIGHT_AUTO;
	css_fixed height = 0;
	css_unit hunit = CSS_UNIT_PX;
	गणना css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool child_has_height = false;

	प्रतिज्ञा(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	/* check if the widths have already been calculated */
	यदि (block->max_width != UNKNOWN_MAX_WIDTH)
		प्रत्यागम;

	यदि (block->style != रिक्त) {
		wtype = css_computed_width(block->style, &width, &wunit);
		htype = css_computed_height(block->style, &height, &hunit);
		bs = css_computed_box_sizing(block->style);
	}

	/* set whether the minimum width is of any interest for this box */
	यदि (((block->parent && lh__box_is_float_box(block->parent)) ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX) &&
			wtype != CSS_WIDTH_SET) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} अन्यथा यदि (block->type == BOX_TABLE_CELL) {
		/* box shrinks to fit; need minimum width */
		block->flags |= NEED_MIN;
	} अन्यथा यदि (block->parent && (block->parent->flags & NEED_MIN) &&
			wtype != CSS_WIDTH_SET) {
		/* box inside shrink-to-fit context; need minimum width */
		block->flags |= NEED_MIN;
	} अन्यथा यदि (block->parent && (block->parent->type == BOX_FLEX)) {
		/* box is flex item */
		block->flags |= NEED_MIN;
	}

	यदि (block->gadget && (block->gadget->type == GADGET_TEXTBOX ||
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

	यदि (block->gadget && (block->gadget->type == GADGET_RADIO ||
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

	यदि (block->object) {
		यदि (content_get_type(block->object) == CONTENT_HTML) {
			layout_minmax_block(html_get_box_वृक्ष(block->object),
					font_func, content);
			min = html_get_box_वृक्ष(block->object)->min_width;
			max = html_get_box_वृक्ष(block->object)->max_width;
		} अन्यथा {
			min = max = content_get_width(block->object);
		}

		block->flags |= HAS_HEIGHT;
	} अन्यथा यदि (block->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		block->flags |= HAS_HEIGHT;
	} अन्यथा {
		/* recurse through children */
		प्रति (child = block->children; child; child = child->next) {
			विकल्प (child->type) {
			पक्ष BOX_FLEX:
			पक्ष BOX_BLOCK:
				layout_minmax_block(child, font_func,
						content);
				यदि (child->flags & HAS_HEIGHT)
					child_has_height = true;
				भङ्ग;
			पक्ष BOX_INLINE_CONTAINER:
				यदि (block->flags & NEED_MIN)
					child->flags |= NEED_MIN;

				layout_minmax_inline_container(child,
						&child_has_height, font_func,
						content);
				यदि (child_has_height &&
						child ==
						child->parent->children) {
					block->flags |= MAKE_HEIGHT;
				}
				भङ्ग;
			पक्ष BOX_TABLE:
				layout_minmax_table(child, font_func,
						content);
				/* todo: fix for zero height tables */
				child_has_height = true;
				child->flags |= MAKE_HEIGHT;
				भङ्ग;
			स्वभाव:
				प्रतिज्ञा(0);
			}
			प्रतिज्ञा(child->max_width != UNKNOWN_MAX_WIDTH);

			यदि (child->style &&
					(css_computed_position(child->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(child->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				अनुवर्त;
			}

			यदि (lh__box_is_flex_container(block) &&
			    lh__flex_main_is_horizontal(block)) {
				यदि (block->style != रिक्त &&
				    css_computed_flex_wrap(block->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += child->min_width;
				} अन्यथा {
					यदि (min < child->min_width)
						min = child->min_width;
				}
				max += child->max_width;

			} अन्यथा {
				यदि (min < child->min_width)
					min = child->min_width;
				यदि (max < child->max_width)
					max = child->max_width;
			}

			यदि (child_has_height)
				block->flags |= HAS_HEIGHT;
		}
	}

	यदि (max < min) {
		box_dump(stderr, block, 0, true);
		प्रतिज्ञा(0);
	}

	/* fixed width takes priority */
	यदि (block->type != BOX_TABLE_CELL && !lh__box_is_flex_item(block)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		गणना css_max_width_e max_type;
		गणना css_min_width_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed मूल्य = 0;
		पूर्णाङ्क width;

		यदि (css_computed_width_px(block->style, &content->unit_len_ctx,
				-1, &width) == CSS_WIDTH_SET) {
			min = max = width;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_width(block->style, &मूल्य, &unit);
		यदि (min_type == CSS_MIN_WIDTH_SET && unit != CSS_UNIT_PCT) {
			पूर्णाङ्क val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, मूल्य, unit));

			यदि (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_width(block->style, &मूल्य, &unit);
		यदि (max_type == CSS_MAX_WIDTH_SET && unit != CSS_UNIT_PCT) {
			पूर्णाङ्क val = FIXTOINT(css_unit_len2device_px(block->style,
					&content->unit_len_ctx, मूल्य, unit));

			यदि (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	यदि (htype == CSS_HEIGHT_SET && hunit != CSS_UNIT_PCT &&
			height > INTTOFIX(0)) {
		block->flags |= MAKE_HEIGHT;
		block->flags |= HAS_HEIGHT;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_width(&content->unit_len_ctx, block->style, डावा_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, उजवा_2,
			false, true, true, &extra_fixed, &extra_frac);

	यदि (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	यदि (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	यदि (max < min) {
		min = max;
	}

	calculate_mbp_width(&content->unit_len_ctx, block->style, डावा_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_width(&content->unit_len_ctx, block->style, उजवा_2,
			true, false, false, &extra_fixed, &extra_frac);

	यदि (extra_fixed < 0)
		extra_fixed = 0;
	यदि (extra_frac < 0)
		extra_frac = 0;
	यदि (1.0 <= extra_frac)
		extra_frac = 0.9;
	यदि (block->style != रिक्त &&
			(css_computed_float(block->style) == CSS_FLOAT_डावा ||
			css_computed_float(block->style) == CSS_FLOAT_उजवा)) {
		/* floated boxs */
		block->min_width = min + extra_fixed;
		block->max_width = max + extra_fixed;
	} अन्यथा {
		/* not floated */
		block->min_width = (min + extra_fixed) / (1.0 - extra_frac);
		block->max_width = (max + extra_fixed) / (1.0 - extra_frac);
	}

	प्रतिज्ञा(0 <= block->min_width);
	प्रतिज्ञा(block->min_width <= block->max_width);
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
स्थिर संरचना box*
layout_next_margin_block(ध्रुव css_unit_ctx *unit_len_ctx,
			 संरचना box *box,
			 संरचना box *block,
			 पूर्णाङ्क viewport_height,
			 पूर्णाङ्क *max_pos_margin,
			 पूर्णाङ्क *max_neg_margin)
{
	प्रतिज्ञा(block != रिक्त);

	यावत् (box != रिक्त) {

		यदि (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			यदि (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						रिक्त, रिक्त, रिक्त, रिक्त,
						रिक्त, रिक्त, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				यदि (*max_pos_margin < box->margin[शीर्ष_2])
					*max_pos_margin = box->margin[शीर्ष_2];
				अन्यथा यदि (*max_neg_margin < -box->margin[शीर्ष_2])
					*max_neg_margin = -box->margin[शीर्ष_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			यदि (box->flags & MAKE_HEIGHT ||
					box->border[शीर्ष_2].width ||
					box->padding[शीर्ष_2] ||
					(box->style &&
					css_computed_overflow_y(box->style) !=
					CSS_OVERFLOW_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_first_child(box))) {
				/* Collapse to this box; return it */
				प्रत्यागम box;
			}
		}


		/* Find next box */
		यदि (box->type == BOX_BLOCK && !box->object && box->children &&
				box->style &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} अन्यथा {
			यदि (!box->next) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				कुरु {
					/* Apply bottom margin */
					यदि (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					अन्यथा यदि (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->parent;
				} यावत् (box != block && !box->next);

				यदि (box == block) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					प्रत्यागम block;
				}
			}

			/* Apply bottom margin */
			यदि (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			अन्यथा यदि (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->next;

			/* Get margins */
			यदि (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->width,
						viewport_height, box,
						box->style,
						रिक्त, रिक्त, रिक्त, रिक्त,
						रिक्त, रिक्त, box->margin,
						box->padding, box->border);
			}
		}
	}

	प्रत्यागम रिक्त;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
स्थिर पूर्णाङ्क layout_clear(संरचना box *fl, गणना css_clear_e clear)
{
	पूर्णाङ्क y = 0;
	प्रति (; fl; fl = fl->next_float) {
		यदि ((clear == CSS_CLEAR_डावा || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_डावा)
			यदि (y < fl->y + fl->height)
				y = fl->y + fl->height;
		यदि ((clear == CSS_CLEAR_उजवा || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_उजवा)
			यदि (y < fl->y + fl->height)
				y = fl->y + fl->height;
	}
	प्रत्यागम y;
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
स्थिर शून्य
find_sides(संरचना box *fl,
	   पूर्णाङ्क y0, पूर्णाङ्क y1,
	   पूर्णाङ्क *x0, पूर्णाङ्क *x1,
	   संरचना box **डावा,
	   संरचना box **उजवा)
{
	पूर्णाङ्क fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*डावा = *उजवा = 0;
	प्रति (; fl; fl = fl->next_float) {
		fy1 = fl->y + fl->height;
		यदि (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			प्रत्यागम;
		}
		fy0 = fl->y;
		यदि (y0 < fy1 && fy0 <= y1) {
			यदि (fl->type == BOX_FLOAT_डावा) {
				fx1 = fl->x + fl->width;
				यदि (*x0 < fx1) {
					*x0 = fx1;
					*डावा = fl;
				}
			} अन्यथा {
				fx0 = fl->x;
				यदि (fx0 < *x1) {
					*x1 = fx0;
					*उजवा = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *डावा, *उजवा);
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
स्थिर पूर्णाङ्क
layout_solve_width(संरचना box *box,
		   पूर्णाङ्क available_width,
		   पूर्णाङ्क width,
		   पूर्णाङ्क lm,
		   पूर्णाङ्क rm,
		   पूर्णाङ्क max_width,
		   पूर्णाङ्क min_width)
{
	bool auto_width = false;

	/* Increase specified left/right margins */
	यदि (box->margin[डावा_2] != AUTO && box->margin[डावा_2] < lm &&
			box->margin[डावा_2] >= 0)
		box->margin[डावा_2] = lm;
	यदि (box->margin[उजवा_2] != AUTO && box->margin[उजवा_2] < rm &&
			box->margin[उजवा_2] >= 0)
		box->margin[उजवा_2] = rm;

	/* Find width */
	यदि (width == AUTO) {
		पूर्णाङ्क margin_डावा = box->margin[डावा_2];
		पूर्णाङ्क margin_उजवा = box->margin[उजवा_2];

		यदि (margin_डावा == AUTO) {
			margin_डावा = lm;
		}
		यदि (margin_उजवा == AUTO) {
			margin_उजवा = rm;
		}

		width = available_width -
				(margin_डावा + box->border[डावा_2].width +
				box->padding[डावा_2] + box->padding[उजवा_2] +
				box->border[उजवा_2].width + margin_उजवा);
		width = width < 0 ? 0 : width;
		auto_width = true;
	}

	यदि (max_width >= 0 && width > max_width) {
		/* max-width is admissable and width exceeds max-width */
		width = max_width;
		auto_width = false;
	}

	यदि (min_width > 0 && width < min_width) {
		/* min-width is admissable and width is less than max-width */
		width = min_width;
		auto_width = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	यदि (auto_width) {
		/* any other 'auto' become 0 or the minimum required values */
		यदि (box->margin[डावा_2] == AUTO) {
			box->margin[डावा_2] = lm;
		}
		यदि (box->margin[उजवा_2] == AUTO) {
			box->margin[उजवा_2] = rm;
		}
		प्रत्यागम width;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	यदि (box->margin[डावा_2] != AUTO && box->margin[उजवा_2] != AUTO &&
			box->parent != रिक्त && box->parent->style != रिक्त) {
		विकल्प (css_computed_text_align(box->parent->style)) {
		पक्ष CSS_TEXT_ALIGN_LIBCSS_उजवा:
			box->margin[डावा_2] = AUTO;
			box->margin[उजवा_2] = 0;
			भङ्ग;
		पक्ष CSS_TEXT_ALIGN_LIBCSS_CENTER:
			box->margin[डावा_2] = box->margin[उजवा_2] = AUTO;
			भङ्ग;
		पक्ष CSS_TEXT_ALIGN_LIBCSS_डावा:
			box->margin[डावा_2] = 0;
			box->margin[उजवा_2] = AUTO;
			भङ्ग;
		स्वभाव:
			/* Leave it alone; no HTML alignment */
			भङ्ग;
		}
	}

	यदि (box->margin[डावा_2] == AUTO && box->margin[उजवा_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[डावा_2] = box->margin[उजवा_2] =
				(available_width - lm - rm -
				(box->border[डावा_2].width + box->padding[डावा_2] +
				width + box->padding[उजवा_2] +
				box->border[उजवा_2].width)) / 2;

		यदि (box->margin[डावा_2] < 0) {
			box->margin[उजवा_2] += box->margin[डावा_2];
			box->margin[डावा_2] = 0;
		}

		box->margin[डावा_2] += lm;

	} अन्यथा यदि (box->margin[डावा_2] == AUTO) {
		box->margin[डावा_2] = available_width - lm -
				(box->border[डावा_2].width + box->padding[डावा_2] +
				width + box->padding[उजवा_2] +
				box->border[उजवा_2].width + box->margin[उजवा_2]);
		box->margin[डावा_2] = box->margin[डावा_2] < lm
				? lm : box->margin[डावा_2];
	} अन्यथा {
		/* margin-right auto or "over-constrained" */
		box->margin[उजवा_2] = available_width - rm -
				(box->margin[डावा_2] + box->border[डावा_2].width +
				 box->padding[डावा_2] + width +
				 box->padding[उजवा_2] +
				 box->border[उजवा_2].width);
	}

	प्रत्यागम width;
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
स्थिर शून्य
layout_block_find_dimensions(ध्रुव css_unit_ctx *unit_len_ctx,
			     पूर्णाङ्क available_width,
			     पूर्णाङ्क viewport_height,
			     पूर्णाङ्क lm,
			     पूर्णाङ्क rm,
			     संरचना box *box)
{
	पूर्णाङ्क width, max_width, min_width;
	पूर्णाङ्क height, max_height, min_height;
	पूर्णाङ्क *margin = box->margin;
	पूर्णाङ्क *padding = box->padding;
	संरचना box_border *border = box->border;
	ध्रुव css_computed_style *style = box->style;

	layout_find_dimensions(unit_len_ctx, available_width, viewport_height, box,
			style, &width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	यदि (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	}

	box->width = layout_solve_width(box, available_width, width, lm, rm,
			max_width, min_width);
	box->height = height;

	यदि (margin[शीर्ष_2] == AUTO)
		margin[शीर्ष_2] = 0;
	यदि (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
स्थिर शून्य layout_block_add_scrollbar(संरचना box *box, पूर्णाङ्क which)
{
	गणना css_overflow_e overflow_x, overflow_y;

	प्रतिज्ञा(box->type == BOX_BLOCK && (which == उजवा_2 || which == BOTTOM));

	यदि (box->style == रिक्त)
		प्रत्यागम;

	overflow_x = css_computed_overflow_x(box->style);
	overflow_y = css_computed_overflow_y(box->style);

	यदि (which == BOTTOM &&
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		यदि (box->height != AUTO &&
				(overflow_x == CSS_OVERFLOW_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_WIDTH;
		}

	} अन्यथा यदि (which == उजवा_2 &&
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		गणना css_height_e htype;
		css_fixed height = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_height(box->style, &height, &hunit);

		यदि (which == उजवा_2 && box->width != AUTO &&
				htype == CSS_HEIGHT_SET &&
				(overflow_y == CSS_OVERFLOW_SCROLL ||
				box_vscrollbar_present(box))) {
			box->width -= SCROLLBAR_WIDTH;
			box->padding[उजवा_2] += SCROLLBAR_WIDTH;
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
स्थिर शून्य layout_move_children(संरचना box *box, पूर्णाङ्क x, पूर्णाङ्क y)
{
	प्रतिज्ञा(box);

	प्रति (box = box->children; box; box = box->next) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_table(
		संरचना box *table,
		पूर्णाङ्क available_width,
		html_content *content)
{
	unsigned पूर्णाङ्क columns = table->columns;  /* total columns */
	unsigned पूर्णाङ्क i;
	unsigned पूर्णाङ्क *row_span;
	पूर्णाङ्क *excess_y;
	पूर्णाङ्क table_width, min_width = 0, max_width = 0;
	पूर्णाङ्क required_width = 0;
	पूर्णाङ्क x, remainder = 0, मोजणी = 0;
	पूर्णाङ्क table_height = 0;
	पूर्णाङ्क min_height = 0;
	पूर्णाङ्क *xs;  /* array of column x positions */
	पूर्णाङ्क auto_width;
	पूर्णाङ्क spare_width;
	पूर्णाङ्क relative_sum = 0;
	पूर्णाङ्क border_spacing_h = 0, border_spacing_v = 0;
	पूर्णाङ्क spare_height;
	पूर्णाङ्क positioned_columns = 0;
	संरचना box *containing_block = रिक्त;
	संरचना box *c;
	संरचना box *row;
	संरचना box *row_group;
	संरचना box **row_span_cell;
	संरचना column *col;
	ध्रुव css_computed_style *style = table->style;
	गणना css_height_e htype;
	css_fixed मूल्य = 0;
	css_unit unit = CSS_UNIT_PX;

	प्रतिज्ञा(table->type == BOX_TABLE);
	प्रतिज्ञा(style);
	प्रतिज्ञा(table->children && table->children->children);
	प्रतिज्ञा(columns);

	/* allocate working buffers */
	col = malloc(columns * परिमाण col[0]);
	excess_y = malloc(columns * परिमाण excess_y[0]);
	row_span = malloc(columns * परिमाण row_span[0]);
	row_span_cell = malloc(columns * परिमाण row_span_cell[0]);
	xs = malloc((columns + 1) * परिमाण xs[0]);
	यदि (!col || !xs || !row_span || !excess_y || !row_span_cell) {
		मुक्त(col);
		मुक्त(excess_y);
		मुक्त(row_span);
		मुक्त(row_span_cell);
		मुक्त(xs);
		प्रत्यागम false;
	}

	memcpy(col, table->col, परिमाण(col[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1, table,
			style, 0, 0, 0, 0, 0, 0, table->margin, table->padding,
			table->border);
	प्रति (row_group = table->children; row_group;
			row_group = row_group->next) {
		प्रति (row = row_group->children; row; row = row->next) {
			प्रति (c = row->children; c; c = c->next) {
				गणना css_overflow_e overflow_x;
				गणना css_overflow_e overflow_y;

				प्रतिज्ञा(c->style);
				table_used_border_for_cell(
						&content->unit_len_ctx, c);
				layout_find_dimensions(&content->unit_len_ctx,
						available_width, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				overflow_x = css_computed_overflow_x(c->style);
				overflow_y = css_computed_overflow_y(c->style);

				यदि (overflow_x == CSS_OVERFLOW_SCROLL ||
						overflow_x ==
						CSS_OVERFLOW_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_WIDTH;
				}
				यदि (overflow_y == CSS_OVERFLOW_SCROLL ||
						overflow_y ==
						CSS_OVERFLOW_AUTO) {
					c->padding[उजवा_2] += SCROLLBAR_WIDTH;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	यदि (css_computed_border_collapse(style) ==
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
	यदि (css_computed_width_px(style, &content->unit_len_ctx,
			available_width, &table_width) == CSS_WIDTH_SET) {
		/* specified width includes border */
		table_width -= table->border[डावा_2].width +
				table->border[उजवा_2].width;
		table_width = table_width < 0 ? 0 : table_width;

		auto_width = table_width;
	} अन्यथा {
		table_width = AUTO;
		auto_width = available_width -
				((table->margin[डावा_2] == AUTO ? 0 :
						table->margin[डावा_2]) +
				 table->border[डावा_2].width +
				 table->padding[डावा_2] +
				 table->padding[उजवा_2] +
				 table->border[उजवा_2].width +
				 (table->margin[उजवा_2] == AUTO ? 0 :
						table->margin[उजवा_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_height(style, &मूल्य, &unit);
	यदि (htype == CSS_HEIGHT_SET) {
		यदि (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			यदि (css_computed_position(table->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				प्रतिज्ञा(table->float_container);
				containing_block = table->float_container;
			} अन्यथा यदि (table->float_container &&
					css_computed_position(table->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(table->style) ==
						CSS_FLOAT_डावा ||
					css_computed_float(table->style) ==
						CSS_FLOAT_उजवा)) {
				/* Table is a float */
				प्रतिज्ञा(table->parent && table->parent->parent &&
						table->parent->parent->parent);
				containing_block =
						table->parent->parent->parent;
			} अन्यथा यदि (table->parent && table->parent->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_block = table->parent;
			} अन्यथा यदि (table->parent && table->parent->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				प्रतिज्ञा(table->parent->parent);
				containing_block = table->parent->parent;
			}

			यदि (containing_block) {
				css_fixed ignored = 0;

				htype = css_computed_height(
						containing_block->style,
						&ignored, &unit);
			}

			यदि (containing_block &&
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
	प्रति (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      table,
		      i,
		      ((ध्रुव वर्ण *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[col[i].type],
		      col[i].width,
		      col[i].min,
		      col[i].max);


		यदि (col[i].positioned) {
			positioned_columns++;
			अनुवर्त;
		} अन्यथा यदि (col[i].type == COLUMN_WIDTH_FIXED) {
			यदि (col[i].width < col[i].min)
				col[i].width = col[i].max = col[i].min;
			अन्यथा
				col[i].min = col[i].max = col[i].width;
			required_width += col[i].width;
		} अन्यथा यदि (col[i].type == COLUMN_WIDTH_PERCENT) {
			पूर्णाङ्क width = col[i].width * auto_width / 100;
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

	यदि (auto_width < required_width) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		प्रति (i = 0; i != columns; i++) {
			यदि (col[i].type == COLUMN_WIDTH_RELATIVE)
				अनुवर्त;
			यदि (col[i].type == COLUMN_WIDTH_PERCENT) {
				col[i].max = auto_width * col[i].width / 100;
				यदि (col[i].max < col[i].min)
					col[i].max = col[i].min;
			}
			min_width += col[i].min;
			max_width += col[i].max;
		}
	} अन्यथा {
		/* take percentages exactly */
		प्रति (i = 0; i != columns; i++) {
			यदि (col[i].type == COLUMN_WIDTH_RELATIVE)
				अनुवर्त;
			यदि (col[i].type == COLUMN_WIDTH_PERCENT) {
				पूर्णाङ्क width = auto_width * col[i].width / 100;
				यदि (width < col[i].min)
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
	प्रति (i = 0; i != columns; i++) {
		यदि (col[i].type == COLUMN_WIDTH_RELATIVE)
			relative_sum += col[i].width;
		अन्यथा यदि (col[i].type == COLUMN_WIDTH_FIXED)
			spare_width -= col[i].width;
		अन्यथा
			spare_width -= col[i].min;
	}
	spare_width -= (columns + 1) * border_spacing_h;
	यदि (relative_sum != 0) {
		यदि (spare_width < 0)
			spare_width = 0;
		प्रति (i = 0; i != columns; i++) {
			यदि (col[i].type == COLUMN_WIDTH_RELATIVE) {
				col[i].min = ceil(col[i].max =
						(भिन्न) spare_width
						* (भिन्न) col[i].width
						/ relative_sum);
				min_width += col[i].min;
				max_width += col[i].max;
			}
		}
	}
	min_width += (columns + 1) * border_spacing_h;
	max_width += (columns + 1) * border_spacing_h;

	यदि (auto_width <= min_width) {
		/* not enough space: minimise column widths */
		प्रति (i = 0; i < columns; i++) {
			col[i].width = col[i].min;
		}
		table_width = min_width;
	} अन्यथा यदि (max_width <= auto_width) {
		/* more space than maximum width */
		यदि (table_width == AUTO) {
			/* for auto-width tables, make columns max width */
			प्रति (i = 0; i < columns; i++) {
				col[i].width = col[i].max;
			}
			table_width = max_width;
		} अन्यथा {
			/* for fixed-width tables, distribute the extra space
			 * too */
			unsigned पूर्णाङ्क flexible_columns = 0;
			प्रति (i = 0; i != columns; i++)
				यदि (col[i].type != COLUMN_WIDTH_FIXED)
					flexible_columns++;
			यदि (flexible_columns == 0) {
				पूर्णाङ्क extra = (table_width - max_width) / columns;
				remainder = (table_width - max_width) -
						(extra * columns);
				प्रति (i = 0; i != columns; i++) {
					col[i].width = col[i].max + extra;
					मोजणी -= remainder;
					यदि (मोजणी < 0) {
						col[i].width++;
						मोजणी += columns;
					}
				}

			} अन्यथा {
				पूर्णाङ्क extra = (table_width - max_width) /
						flexible_columns;
				remainder = (table_width - max_width) -
						(extra * flexible_columns);
				प्रति (i = 0; i != columns; i++)
					यदि (col[i].type != COLUMN_WIDTH_FIXED) {
						col[i].width = col[i].max +
								extra;
						मोजणी -= remainder;
						यदि (मोजणी < 0) {
							col[i].width++;
							मोजणी += flexible_columns;
						}
					}
			}
		}
	} अन्यथा {
		/* space between min and max: fill it exactly */
		भिन्न scale = (भिन्न) (auto_width - min_width) /
				(भिन्न) (max_width - min_width);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		प्रति (i = 0; i < columns; i++) {
			col[i].width = col[i].min + (पूर्णाङ्क) (0.5 +
					(col[i].max - col[i].min) * scale);
		}
		table_width = auto_width;
	}

	xs[0] = x = border_spacing_h;
	प्रति (i = 0; i != columns; i++) {
		यदि (!col[i].positioned)
			x += col[i].width + border_spacing_h;
		xs[i + 1] = x;
		row_span[i] = 0;
		excess_y[i] = 0;
		row_span_cell[i] = 0;
	}

	/* position cells */
	table_height = border_spacing_v;
	प्रति (row_group = table->children; row_group;
			row_group = row_group->next) {
		पूर्णाङ्क row_group_height = 0;
		प्रति (row = row_group->children; row; row = row->next) {
			पूर्णाङ्क row_height = 0;

			htype = css_computed_height(row->style, &मूल्य, &unit);
			यदि (htype == CSS_HEIGHT_SET && unit != CSS_UNIT_PCT) {
				row_height = FIXTOINT(css_unit_len2device_px(
						row->style,
						&content->unit_len_ctx,
						मूल्य, unit));
			}
			प्रति (c = row->children; c; c = c->next) {
				प्रतिज्ञा(c->style);
				c->width = xs[c->start_column + c->columns] -
						xs[c->start_column] -
						border_spacing_h -
						c->border[डावा_2].width -
						c->padding[डावा_2] -
						c->padding[उजवा_2] -
						c->border[उजवा_2].width;
				c->float_children = 0;
				c->cached_place_below_level = 0;

				c->height = AUTO;
				यदि (!layout_block_context(c, -1, content)) {
					मुक्त(col);
					मुक्त(excess_y);
					मुक्त(row_span);
					मुक्त(row_span_cell);
					मुक्त(xs);
					प्रत्यागम false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->height;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_height(c->style,
						&मूल्य, &unit);

				यदि (htype == CSS_HEIGHT_SET &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					पूर्णाङ्क h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							मूल्य, unit));
					यदि (c->height < h)
						c->height = h;
				}
				/* specified row height is treated as a minimum
				 */
				यदि (c->height < row_height)
					c->height = row_height;
				c->x = xs[c->start_column] +
						c->border[डावा_2].width;
				c->y = c->border[शीर्ष_2].width;
				प्रति (i = 0; i != c->columns; i++) {
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
			प्रति (i = 0; i != columns; i++)
				यदि (row_span[i] != 0)
					row_span[i]--;
				अन्यथा
					row_span_cell[i] = 0;
			यदि (row->next || row_group->next) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				प्रति (i = 0; i != columns; i++)
					यदि (row_span[i] == 0 && row_height <
							excess_y[i])
						row_height = excess_y[i];
			} अन्यथा {
				/* except in the last row */
				प्रति (i = 0; i != columns; i++)
					यदि (row_height < excess_y[i])
						row_height = excess_y[i];
			}
			प्रति (i = 0; i != columns; i++) {
				यदि (row_height < excess_y[i])
					excess_y[i] -= row_height;
				अन्यथा
					excess_y[i] = 0;
				यदि (row_span_cell[i] != 0)
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
	प्रति (row_group = table->children; row_group;
			row_group = row_group->next) {
		प्रति (row = row_group->children; row; row = row->next) {
			प्रति (c = row->children; c; c = c->next) {
				गणना css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				spare_height = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->height - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &मूल्य, &unit);

				विकल्प (vertical_align) {
				पक्ष CSS_VERTICAL_ALIGN_SUB:
				पक्ष CSS_VERTICAL_ALIGN_SUPER:
				पक्ष CSS_VERTICAL_ALIGN_TEXT_शीर्ष:
				पक्ष CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				पक्ष CSS_VERTICAL_ALIGN_SET:
				पक्ष CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				पक्ष CSS_VERTICAL_ALIGN_शीर्ष:
					भङ्ग;
				पक्ष CSS_VERTICAL_ALIGN_मधला:
					c->padding[शीर्ष_2] += spare_height / 2;
					c->padding[BOTTOM] -= spare_height / 2;
					layout_move_children(c, 0,
							spare_height / 2);
					भङ्ग;
				पक्ष CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[शीर्ष_2] += spare_height;
					c->padding[BOTTOM] -= spare_height;
					layout_move_children(c, 0,
							spare_height);
					भङ्ग;
				पक्ष CSS_VERTICAL_ALIGN_INHERIT:
					प्रतिज्ञा(0);
					भङ्ग;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	यदि (table->margin[शीर्ष_2] == AUTO)
		table->margin[शीर्ष_2] = 0;
	यदि (table->margin[BOTTOM] == AUTO)
		table->margin[BOTTOM] = 0;

	मुक्त(col);
	मुक्त(excess_y);
	मुक्त(row_span);
	मुक्त(row_span_cell);
	मुक्त(xs);

	table->width = table_width;
	table->height = table_height;

	प्रत्यागम true;
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
स्थिर bool layout_apply_minmax_height(
		ध्रुव css_unit_ctx *unit_len_ctx,
		संरचना box *box,
		संरचना box *container)
{
	पूर्णाङ्क h;
	संरचना box *containing_block = रिक्त;
	bool updated = false;

	/* Find containing block for percentage heights */
	यदि (box->style != रिक्त && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		प्रतिज्ञा(container);
		containing_block = container;
	} अन्यथा यदि (box->float_container && box->style != रिक्त &&
			(css_computed_float(box->style) == CSS_FLOAT_डावा ||
			 css_computed_float(box->style) == CSS_FLOAT_उजवा)) {
		/* Box is a float */
		प्रतिज्ञा(box->parent && box->parent->parent &&
				box->parent->parent->parent);
		containing_block = box->parent->parent->parent;
	} अन्यथा यदि (box->parent && box->parent->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_block = box->parent;
	} अन्यथा यदि (box->parent && box->parent->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		प्रतिज्ञा(box->parent->parent);
		containing_block = box->parent->parent;
	}

	यदि (box->style) {
		गणना css_height_e htype = CSS_HEIGHT_AUTO;
		css_fixed मूल्य = 0;
		css_unit unit = CSS_UNIT_PX;

		यदि (containing_block) {
			htype = css_computed_height(containing_block->style,
					&मूल्य, &unit);
		}

		/* max-height */
		यदि (css_computed_max_height(box->style, &मूल्य, &unit) ==
				CSS_MAX_HEIGHT_SET) {
			यदि (unit == CSS_UNIT_PCT) {
				यदि (containing_block &&
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
					यदि (h < box->height) {
						box->height = h;
						updated = true;
					}
				}
			} अन्यथा {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						मूल्य, unit));
				यदि (h < box->height) {
					box->height = h;
					updated = true;
				}
			}
		}

		/* min-height */
		यदि (ns_computed_min_height(box->style, &मूल्य, &unit) ==
				CSS_MIN_HEIGHT_SET) {
			यदि (unit == CSS_UNIT_PCT) {
				यदि (containing_block &&
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
					यदि (h > box->height) {
						box->height = h;
						updated = true;
					}
				}
			} अन्यथा {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						मूल्य, unit));
				यदि (h > box->height) {
					box->height = h;
					updated = true;
				}
			}
		}
	}
	प्रत्यागम updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
स्थिर bool layout_block_object(संरचना box *block)
{
	प्रतिज्ञा(block);
	प्रतिज्ञा(block->type == BOX_BLOCK ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_INLINE_FLEX ||
			block->type == BOX_TABLE ||
			block->type == BOX_TABLE_CELL);
	प्रतिज्ञा(block->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", block,
	      hlcache_handle_get_url(block->object), block->width);

	यदि (content_can_reformat(block->object)) {
		content_reformat(block->object, false, block->width, 1);
	} अन्यथा {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	प्रत्यागम true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
स्थिर शून्य add_float_to_container(संरचना box *cont, संरचना box *b)
{
	संरचना box *box = cont->float_children;
	पूर्णाङ्क b_bottom = b->y + b->height;

	प्रतिज्ञा(b->type == BOX_FLOAT_डावा || b->type == BOX_FLOAT_उजवा);

	यदि (box == रिक्त) {
		/* No other float children */
		b->next_float = रिक्त;
		cont->float_children = b;
		प्रत्यागम;
	} अन्यथा यदि (b_bottom >= box->y + box->height) {
		/* Goes at start of list */
		b->next_float = cont->float_children;
		cont->float_children = b;
	} अन्यथा {
		संरचना box *prev = रिक्त;
		यावत् (box != रिक्त && b_bottom < box->y + box->height) {
			prev = box;
			box = box->next_float;
		}
		यदि (prev != रिक्त) {
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
स्थिर bool
layout_text_box_split(html_content *content,
		      plot_font_style_t *fstyle,
		      संरचना box *split_box,
		      size_t new_length,
		      पूर्णाङ्क new_width)
{
	पूर्णाङ्क space_width = split_box->space;
	संरचना box *c2;
	ध्रुव संरचना gui_layout_table *font_func = content->font_func;
	bool space = (split_box->text[new_length] == ' ');
	पूर्णाङ्क used_length = new_length + (space ? 1 : 0);

	यदि ((space && space_width == 0) || space_width == UNKNOWN_WIDTH) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->width(fstyle, " ", 1, &space_width);
	}

	यदि (split_box->space == UNKNOWN_WIDTH)
		split_box->space = space_width;
	यदि (!space)
		space_width = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, split_box, परिमाण *c2);
	यदि (!c2)
		प्रत्यागम false;
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
	यदि (c2->next)
		c2->next->prev = c2;
	अन्यथा
		c2->parent->last = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      split_box,
	      split_box->length,
	      (पूर्णाङ्क)split_box->length,
	      split_box->text);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->length,
	      (पूर्णाङ्क)c2->length,
	      c2->text);

	प्रत्यागम true;
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
स्थिर शून्य
layout_float_find_dimensions(
		ध्रुव css_unit_ctx *unit_len_ctx,
		पूर्णाङ्क available_width,
		ध्रुव css_computed_style *style,
		संरचना box *box)
{
	पूर्णाङ्क width, height, max_width, min_width, max_height, min_height;
	पूर्णाङ्क *margin = box->margin;
	पूर्णाङ्क *padding = box->padding;
	संरचना box_border *border = box->border;
	गणना css_overflow_e overflow_x = css_computed_overflow_x(style);
	गणना css_overflow_e overflow_y = css_computed_overflow_y(style);
	पूर्णाङ्क scrollbar_width_x =
			(overflow_x == CSS_OVERFLOW_SCROLL ||
			 overflow_x == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;
	पूर्णाङ्क scrollbar_width_y =
			(overflow_y == CSS_OVERFLOW_SCROLL ||
			 overflow_y == CSS_OVERFLOW_AUTO) ?
			SCROLLBAR_WIDTH : 0;

	layout_find_dimensions(unit_len_ctx, available_width, -1, box, style,
			&width, &height, &max_width, &min_width,
			&max_height, &min_height, margin, padding, border);

	यदि (margin[डावा_2] == AUTO)
		margin[डावा_2] = 0;
	यदि (margin[उजवा_2] == AUTO)
		margin[उजवा_2] = 0;

	यदि (box->gadget == रिक्त) {
		padding[उजवा_2] += scrollbar_width_y;
		padding[BOTTOM] += scrollbar_width_x;
	}

	यदि (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &width, &height,
				min_width, max_width, min_height, max_height);
	} अन्यथा यदि (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed size = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		प्रतिज्ञा(box->style);

		यदि (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			यदि (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			यदि (box->gadget->type == GADGET_FILE &&
					height == AUTO) {
				size = FLTTOFIX(1.5);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
		यदि (box->gadget->type == GADGET_TEXTAREA) {
			यदि (width == AUTO) {
				size = INTTOFIX(10);
				width = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
			यदि (height == AUTO) {
				size = INTTOFIX(4);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						size, unit));
			}
		}
	} अन्यथा यदि (width == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		width = min(max(box->min_width, available_width),
				box->max_width);

		/* width includes margin, borders and padding */
		यदि (width == available_width) {
			width -= box->margin[डावा_2] + box->border[डावा_2].width +
					box->padding[डावा_2] +
					box->padding[उजवा_2] +
					box->border[उजवा_2].width +
					box->margin[उजवा_2];
		} अन्यथा {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			पूर्णाङ्क fixed = 0;
			भिन्न frac = 0;
			calculate_mbp_width(unit_len_ctx, box->style, डावा_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_width(unit_len_ctx, box->style, उजवा_2,
					true, true, true, &fixed, &frac);
			यदि (fixed < 0)
				fixed = 0;

			width -= fixed;
		}

		यदि (max_width >= 0 && width > max_width) width = max_width;
		यदि (min_width >  0 && width < min_width) width = min_width;

	} अन्यथा {
		यदि (max_width >= 0 && width > max_width) width = max_width;
		यदि (min_width >  0 && width < min_width) width = min_width;
		width -= scrollbar_width_y;
	}

	box->width = width;
	box->height = height;

	यदि (margin[शीर्ष_2] == AUTO)
		margin[शीर्ष_2] = 0;
	यदि (margin[BOTTOM] == AUTO)
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
स्थिर bool layout_float(संरचना box *b, पूर्णाङ्क width, html_content *content)
{
	प्रतिज्ञा(b->type == BOX_TABLE ||
	       b->type == BOX_BLOCK ||
	       b->type == BOX_INLINE_BLOCK ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_find_dimensions(&content->unit_len_ctx, width, b->style, b);
	यदि (b->type == BOX_TABLE || b->type == BOX_INLINE_FLEX) {
		यदि (b->type == BOX_TABLE) {
			यदि (!layout_table(b, width, content))
				प्रत्यागम false;
		} अन्यथा {
			यदि (!layout_flex(b, width, content))
				प्रत्यागम false;
		}
		यदि (b->margin[डावा_2] == AUTO)
			b->margin[डावा_2] = 0;
		यदि (b->margin[उजवा_2] == AUTO)
			b->margin[उजवा_2] = 0;
		यदि (b->margin[शीर्ष_2] == AUTO)
			b->margin[शीर्ष_2] = 0;
		यदि (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} अन्यथा {
		प्रत्यागम layout_block_context(b, -1, content);
	}
	प्रत्यागम true;
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
स्थिर शून्य
place_float_below(संरचना box *c, पूर्णाङ्क width, पूर्णाङ्क cx, पूर्णाङ्क y, संरचना box *cont)
{
	पूर्णाङ्क x0, x1, yy;
	संरचना box *डावा;
	संरचना box *उजवा;

	yy = y > cont->cached_place_below_level ?
			y : cont->cached_place_below_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      width, cx, y, cont);

	कुरु {
		y = yy;
		x0 = cx;
		x1 = cx + width;
		find_sides(cont->float_children, y, y + c->height, &x0, &x1,
				&डावा, &उजवा);
		यदि (डावा != 0 && उजवा != 0) {
			yy = (डावा->y + डावा->height <
					उजवा->y + उजवा->height ?
					डावा->y + डावा->height :
					उजवा->y + उजवा->height);
		} अन्यथा यदि (डावा == 0 && उजवा != 0) {
			yy = उजवा->y + उजवा->height;
		} अन्यथा यदि (डावा != 0 && उजवा == 0) {
			yy = डावा->y + डावा->height;
		}
	} यावत् ((डावा != 0 || उजवा != 0) && (c->width > x1 - x0));

	यदि (c->type == BOX_FLOAT_डावा) {
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
स्थिर पूर्णाङ्क ओळ_height(
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव css_computed_style *style)
{
	गणना css_ओळ_height_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed ओळ_height;

	प्रतिज्ञा(style);

	lhtype = css_computed_ओळ_height(style, &lhvalue, &lhunit);
	यदि (lhtype == CSS_ओळ_HEIGHT_NORMAL) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_ओळ_HEIGHT_संख्या;
	}

	यदि (lhtype == CSS_ओळ_HEIGHT_संख्या ||
			lhunit == CSS_UNIT_PCT) {
		ओळ_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		यदि (lhtype != CSS_ओळ_HEIGHT_संख्या)
			ओळ_height = FDIV(ओळ_height, F_100);
	} अन्यथा {
		प्रतिज्ञा(lhunit != CSS_UNIT_PCT);

		ओळ_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	प्रत्यागम FIXTOINT(ओळ_height);
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
स्थिर bool
layout_ओळ(संरचना box *first,
	    पूर्णाङ्क *width,
	    पूर्णाङ्क *y,
	    पूर्णाङ्क cx,
	    पूर्णाङ्क cy,
	    संरचना box *cont,
	    bool indent,
	    bool has_text_children,
	    html_content *content,
	    संरचना box **next_box)
{
	पूर्णाङ्क height, used_height;
	पूर्णाङ्क x0 = 0;
	पूर्णाङ्क x1 = *width;
	पूर्णाङ्क x, h, x_previous;
	पूर्णाङ्क fy = cy;
	संरचना box *डावा;
	संरचना box *उजवा;
	संरचना box *b;
	संरचना box *split_box = 0;
	संरचना box *d;
	संरचना box *br_box = 0;
	bool move_y = false;
	bool place_below = false;
	पूर्णाङ्क space_before = 0, space_after = 0;
	unsigned पूर्णाङ्क inline_मोजणी = 0;
	unsigned पूर्णाङ्क i;
	ध्रुव संरचना gui_layout_table *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      first,
	      (पूर्णाङ्क)first->length,
	      first->text,
	      *width,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	find_sides(cont->float_children, cy, cy, &x0, &x1, &डावा, &उजवा);
	x0 -= cx;
	x1 -= cx;

	यदि (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	यदि (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	यदि (has_text_children || first->parent->parent->gadget)
		used_height = height = ओळ_height(&content->unit_len_ctx,
				first->parent->parent->style);
	अन्यथा
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		used_height = height = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	प्रति (x = 0, b = first; x <= x1 - x0 && b != 0; b = b->next) {
		पूर्णाङ्क min_width, max_width, min_height, max_height;

		प्रतिज्ञा(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		यदि (b->type == BOX_BR)
			भङ्ग;

		यदि (lh__box_is_float_box(b))
			अनुवर्त;
		यदि (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			अनुवर्त;

		प्रतिज्ञा(b->style != रिक्त);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		यदि (b->type == BOX_INLINE_BLOCK ||
		    b->type == BOX_INLINE_FLEX) {
			यदि (b->max_width != UNKNOWN_WIDTH)
				यदि (!layout_float(b, *width, content))
					प्रत्यागम false;
			h = b->border[शीर्ष_2].width + b->padding[शीर्ष_2] + b->height +
					b->padding[BOTTOM] +
					b->border[BOTTOM].width;
			यदि (height < h)
				height = h;
			x += b->margin[डावा_2] + b->border[डावा_2].width +
					b->padding[डावा_2] + b->width +
					b->padding[उजवा_2] +
					b->border[उजवा_2].width +
					b->margin[उजवा_2];
			space_after = 0;
			अनुवर्त;
		}

		यदि (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_find_dimensions(&content->unit_len_ctx,
					*width, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			प्रति (i = 0; i != 4; i++)
				यदि (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[डावा_2] + b->border[डावा_2].width +
					b->padding[डावा_2];
			यदि (b->inline_end) {
				b->inline_end->margin[उजवा_2] = b->margin[उजवा_2];
				b->inline_end->padding[उजवा_2] =
						b->padding[उजवा_2];
				b->inline_end->border[उजवा_2] =
						b->border[उजवा_2];
			} अन्यथा {
				x += b->padding[उजवा_2] +
						b->border[उजवा_2].width +
						b->margin[उजवा_2];
			}
		} अन्यथा यदि (b->type == BOX_INLINE_END) {
			b->width = 0;
			यदि (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[उजवा_2] + b->border[उजवा_2].width +
					b->margin[उजवा_2];
			अनुवर्त;
		}

		यदि (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->height = ओळ_height(&content->unit_len_ctx,
					b->style ? b->style :
					b->parent->parent->style);
			यदि (height < b->height)
				height = b->height;

			यदि (!b->text) {
				b->width = 0;
				space_after = 0;
				अनुवर्त;
			}

			यदि (b->width == UNKNOWN_WIDTH) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				यदि (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					पूर्णाङ्क opt_maxwidth = 0;
					संरचना form_option *o;

					प्रति (o = b->parent->parent->gadget->
							data.select.वस्तुसंच; o;
							o = o->next) {
						पूर्णाङ्क opt_width;
						font_func->width(&fstyle,
								o->text,
								strlen(o->text),
								&opt_width);

						यदि (opt_maxwidth < opt_width)
							opt_maxwidth =opt_width;
					}
					b->width = opt_maxwidth;
					यदि (nsoption_bool(core_select_menu))
						b->width += SCROLLBAR_WIDTH;
				} अन्यथा {
					font_func->width(&fstyle, b->text,
							b->length, &b->width);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			यदि (b->text && (x + b->width < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->next) {
				font_func->width(&fstyle, b->text,
						 b->length, &b->width);
				b->flags |= MEASURED;
			}

			x += b->width;
			यदि (b->space == UNKNOWN_WIDTH) {
				font_func->width(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			अनुवर्त;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		प्रतिज्ञा(b->style);

		layout_find_dimensions(&content->unit_len_ctx,
				*width, -1, b, b->style,
				&b->width, &b->height,
				&max_width, &min_width,
				&max_height, &min_height,
				रिक्त, रिक्त, रिक्त);

		यदि (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->width, &b->height,
					min_width, max_width,
					min_height, max_height);
		} अन्यथा यदि (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			यदि (b->width == AUTO)
				b->width = 400;
			यदि (b->height == AUTO)
				b->height = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} अन्यथा {
			/* form control with no object */
			यदि (b->width == AUTO)
				b->width = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			यदि (b->height == AUTO)
				b->height = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		यदि (b->object && content_can_reformat(b->object) &&
				b->width !=
				content_get_available_width(b->object)) {
			css_fixed मूल्य = 0;
			css_unit unit = CSS_UNIT_PX;
			गणना css_height_e htype = css_computed_height(b->style,
					&मूल्य, &unit);

			content_reformat(b->object, false, b->width, b->height);

			यदि (htype == CSS_HEIGHT_AUTO)
				b->height = content_get_height(b->object);
		}

		यदि (height < b->height)
			height = b->height;

		x += b->width;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *width;
	find_sides(cont->float_children, cy, cy + height, &x0, &x1,
			&डावा, &उजवा);
	x0 -= cx;
	x1 -= cx;

	यदि (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *width);

	यदि (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	प्रति (x = x_previous = 0, b = first; x <= x1 - x0 && b; b = b->next) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		यदि (b->type == BOX_INLINE_BLOCK &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} अन्यथा यदि (lh__box_is_inline_flow(b)) {
			प्रतिज्ञा(b->width != UNKNOWN_WIDTH);

			x_previous = x;
			x += space_after;
			b->x = x;

			यदि ((b->type == BOX_INLINE && !b->inline_end) ||
					b->type == BOX_INLINE_BLOCK ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[डावा_2] + b->border[डावा_2].width;
				x = b->x + b->padding[डावा_2] + b->width +
						b->padding[उजवा_2] +
						b->border[उजवा_2].width +
						b->margin[उजवा_2];
			} अन्यथा यदि (b->type == BOX_INLINE) {
				b->x += b->margin[डावा_2] + b->border[डावा_2].width;
				x = b->x + b->padding[डावा_2] + b->width;
			} अन्यथा यदि (b->type == BOX_INLINE_END) {
				b->height = b->inline_end->height;
				x += b->padding[उजवा_2] +
						b->border[उजवा_2].width +
						b->margin[उजवा_2];
			} अन्यथा {
				x += b->width;
			}

			space_before = space_after;
			यदि (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			अन्यथा यदि (b->text || b->type == BOX_INLINE_END) {
				यदि (b->space == UNKNOWN_WIDTH) {
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
			inline_मोजणी++;
		} अन्यथा यदि (b->type == BOX_BR) {
			b->x = x;
			b->width = 0;
			br_box = b;
			b = b->next;
			split_box = 0;
			move_y = true;
			भङ्ग;

		} अन्यथा {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_place_below_level = 0;
			b->float_container = d->float_container = cont;

			यदि (!layout_float(d, *width, content))
				प्रत्यागम false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[शीर्ष_2],
			      d->border[शीर्ष_2].width);

			d->x = d->margin[डावा_2] + d->border[डावा_2].width;
			d->y = d->margin[शीर्ष_2] + d->border[शीर्ष_2].width;
			b->width = d->margin[डावा_2] + d->border[डावा_2].width +
					d->padding[डावा_2] + d->width +
					d->padding[उजवा_2] +
					d->border[उजवा_2].width +
					d->margin[उजवा_2];
			b->height = d->margin[शीर्ष_2] + d->border[शीर्ष_2].width +
					d->padding[शीर्ष_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];

			यदि (b->width > (x1 - x0) - x)
				place_below = true;
			यदि (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_डावा && डावा == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_उजवा &&
						उजवा == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						डावा == 0 && उजवा == 0)) &&
					(!place_below ||
					(डावा == 0 && उजवा == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_place_below_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				यदि (b->type == BOX_FLOAT_डावा) {
					b->x = cx + x0;
					यदि (b->width > 0)
						x0 += b->width;
					डावा = b;
				} अन्यथा {
					b->x = cx + x1 - b->width;
					यदि (b->width > 0)
						x1 -= b->width;
					उजवा = b;
				}
				b->y = cy;
			} अन्यथा {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				पूर्णाङ्क fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_place_below_level) ?
						fcy :
						cont->cached_place_below_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + height : fy;

				place_float_below(b, *width, cx, fy, cont);
				fy = b->y;
				यदि (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_डावा && डावा != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_उजवा &&
						उजवा != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(डावा != 0 || उजवा != 0)))) {
					/* to be cleared below existing
					 * floats */
					यदि (b->type == BOX_FLOAT_डावा)
						b->x = cx;
					अन्यथा
						b->x = cx + *width - b->width;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					यदि (fcy > cont->clear_level)
						cont->clear_level = fcy;
					यदि (b->y < fcy)
						b->y = fcy;
				}
				यदि (b->type == BOX_FLOAT_डावा)
					डावा = b;
				अन्यथा
					उजवा = b;
			}
			add_float_to_container(cont, b);

			split_box = 0;
		}
	}

	यदि (x1 - x0 < x && split_box) {
		/* the last box went over the end */
		size_t split = 0;
		पूर्णाङ्क w;
		bool no_wrap = css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				split_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_previous;

		यदि (!no_wrap &&
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

		यदि (split == 0)
			w = split_box->width;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      split_box,
		      (पूर्णाङ्क)split_box->length,
		      split_box->text,
		      split,
		      w,
		      डावा,
		      उजवा,
		      inline_मोजणी);

		यदि ((split == 0 || x1 - x0 <= x + space_before + w) &&
				!डावा && !उजवा && inline_मोजणी == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			यदि (split == 0 || split == split_box->length) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = split_box->next;
			} अन्यथा {
				/* cut off first word for this line */
				यदि (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					प्रत्यागम false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} अन्यथा यदि ((split == 0 || x1 - x0 <= x + space_before + w) &&
				inline_मोजणी == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			प्रतिज्ञा(डावा || उजवा);
			used_height = 0;
			यदि (डावा) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      डावा->y,
				      डावा->height);

				used_height = डावा->y + डावा->height - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      used_height);

			}
			यदि (उजवा && used_height <
					उजवा->y + उजवा->height - cy + 1)
				used_height = उजवा->y + उजवा->height - cy + 1;

			यदि (used_height < 0)
				used_height = 0;

			b = split_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} अन्यथा यदि (split == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = split_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} अन्यथा {
			/* fit as many words as possible */
			प्रतिज्ञा(split != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (पूर्णाङ्क)split_box->length, split_box->text,
			      x1 - x0, split, w);

			यदि (split != split_box->length) {
				यदि (!layout_text_box_split(content, &fstyle,
						split_box, split, w))
					प्रत्यागम false;
				b = split_box->next;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	विकल्प (css_computed_text_align(first->parent->parent->style)) {
	पक्ष CSS_TEXT_ALIGN_उजवा:
	पक्ष CSS_TEXT_ALIGN_LIBCSS_उजवा:
		x0 = x1 - x;
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_CENTER:
	पक्ष CSS_TEXT_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_डावा:
	पक्ष CSS_TEXT_ALIGN_LIBCSS_डावा:
	पक्ष CSS_TEXT_ALIGN_JUSTIFY:
		/* leave on left */
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_DEFAULT:
		/* None; consider text direction */
		विकल्प (css_computed_direction(first->parent->parent->style)) {
		पक्ष CSS_DIRECTION_LTR:
			/* leave on left */
			भङ्ग;
		पक्ष CSS_DIRECTION_RTL:
			x0 = x1 - x;
			भङ्ग;
		}
		भङ्ग;
	}

	प्रति (d = first; d != b; d = d->next) {
		d->flags &= ~NEW_ओळ;

		यदि (d->type == BOX_INLINE_BLOCK &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			अनुवर्त;
		} अन्यथा यदि ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[शीर्ष_2];

			यदि (d->type == BOX_TEXT && d->height > used_height) {
				/* text */
				used_height = d->height;
			}
		} अन्यथा यदि ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_BLOCK) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[शीर्ष_2].width + d->margin[शीर्ष_2];
			h = d->margin[शीर्ष_2] + d->border[शीर्ष_2].width +
					d->padding[शीर्ष_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].width +
					d->margin[BOTTOM];
			यदि (used_height < h)
				used_height = h;
		}
	}

	first->flags |= NEW_ओळ;

	प्रतिज्ञा(b != first || (move_y && 0 < used_height && (डावा || उजवा)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	प्रति (d = first; d != b; d = d->next) {
		यदि ((d->type == BOX_INLINE && d->inline_end) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_END) {
			css_fixed मूल्य = 0;
			css_unit unit = CSS_UNIT_PX;
			विकल्प (css_computed_vertical_align(d->style, &मूल्य,
					&unit)) {
			पक्ष CSS_VERTICAL_ALIGN_SUPER:
			पक्ष CSS_VERTICAL_ALIGN_शीर्ष:
			पक्ष CSS_VERTICAL_ALIGN_TEXT_शीर्ष:
				/* already at top */
				भङ्ग;
			पक्ष CSS_VERTICAL_ALIGN_SUB:
			पक्ष CSS_VERTICAL_ALIGN_BOTTOM:
			पक्ष CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				d->y += used_height - d->height;
				भङ्ग;
			स्वभाव:
			पक्ष CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (used_height - d->height);
				भङ्ग;
			}
		}
	}

	/* handle clearance for br */
	यदि (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		पूर्णाङ्क clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		यदि (used_height < clear_y - cy)
			used_height = clear_y - cy;
	}

	यदि (move_y)
		*y += used_height;
	*next_box = b;
	*width = x; /* return actual width */
	प्रत्यागम true;
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
स्थिर bool layout_inline_container(संरचना box *inline_container, पूर्णाङ्क width,
		संरचना box *cont, पूर्णाङ्क cx, पूर्णाङ्क cy, html_content *content)
{
	bool first_ओळ = true;
	bool has_text_children;
	संरचना box *c, *next;
	पूर्णाङ्क y = 0;
	पूर्णाङ्क curwidth,maxwidth = width;

	प्रतिज्ञा(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      width,
	      cont,
	      cx,
	      cy);


	has_text_children = false;
	प्रति (c = inline_container->children; c; c = c->next) {
		bool is_pre = false;

		यदि (c->style) {
			गणना css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_ओळ ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		यदि ((lh__box_is_object(c) == false &&
				c->text && (c->length || is_pre)) ||
				c->type == BOX_BR)
			has_text_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	प्रति (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->width;
		यदि (!layout_ओळ(c, &curwidth, &y, cx, cy + y, cont, first_ओळ,
				has_text_children, content, &next))
			प्रत्यागम false;
		maxwidth = max(maxwidth,curwidth);
		c = next;
		first_ओळ = false;
	}

	inline_container->width = maxwidth;
	inline_container->height = y;

	प्रत्यागम true;
}


/* Documented in layout_intertnal.h */
bool layout_block_context(
		संरचना box *block,
		पूर्णाङ्क viewport_height,
		html_content *content)
{
	संरचना box *box;
	पूर्णाङ्क cx, cy;  /**< current coordinates */
	पूर्णाङ्क max_pos_margin = 0;
	पूर्णाङ्क max_neg_margin = 0;
	पूर्णाङ्क y = 0;
	पूर्णाङ्क lm, rm;
	संरचना box *margin_collapse = रिक्त;
	bool in_margin = false;
	css_fixed gadget_size;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	प्रतिज्ञा(block->type == BOX_BLOCK ||
			block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL ||
			block->type == BOX_FLEX ||
			block->type == BOX_INLINE_FLEX);
	प्रतिज्ञा(block->width != UNKNOWN_WIDTH);
	प्रतिज्ञा(block->width != AUTO);

	block->float_children = रिक्त;
	block->cached_place_below_level = 0;
	block->clear_level = 0;

	/* special case if the block contains an object */
	यदि (block->object) {
		पूर्णाङ्क temp_width = block->width;
		यदि (!layout_block_object(block))
			प्रत्यागम false;
		layout_get_object_dimensions(block, &temp_width,
				&block->height, INT_MIN, INT_MAX,
				INT_MIN, INT_MAX);
		प्रत्यागम true;
	} अन्यथा यदि (block->flags & REPLACE_DIM) {
		प्रत्यागम true;
	}

	/* special case if the block contains an radio button or checkbox */
	यदि (block->gadget && (block->gadget->type == GADGET_RADIO ||
			block->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_size = INTTOFIX(1);
		यदि (block->height == AUTO)
			block->height = FIXTOINT(css_unit_len2device_px(
					block->style,
					&content->unit_len_ctx,
					gadget_size, gadget_unit));
	}

	box = block->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = block->padding[शीर्ष_2];
	यदि (box)
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
	यावत् (box) {
		गणना css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
		गणना css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

		प्रतिज्ञा(box->type == BOX_BLOCK ||
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

		यदि (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->parent->padding[डावा_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			गच्छ advance_to_next_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		यदि (margin_collapse == रिक्त) {
			margin_collapse = layout_next_margin_block(
					&content->unit_len_ctx, box, block,
					viewport_height,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			in_margin = true;
		}

		/* Clearance. */
		y = 0;
		यदि (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(block->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		यदि (box->style) {
			overflow_x = css_computed_overflow_x(box->style);
			overflow_y = css_computed_overflow_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		यदि (box->type == BOX_FLEX ||
		    box->type == BOX_BLOCK ||
		    box->flags & IFRAME) {
			यदि (lh__box_is_object(box) == false &&
					box->style &&
					(overflow_x != CSS_OVERFLOW_VISIBLE ||
					 overflow_y != CSS_OVERFLOW_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				पूर्णाङ्क x0, x1, शीर्ष;
				संरचना box *डावा, *उजवा;
				शीर्ष = cy + max_pos_margin - max_neg_margin;
				शीर्ष = (शीर्ष > y) ? शीर्ष : y;
				x0 = cx;
				x1 = cx + box->parent->width -
						box->parent->padding[डावा_2] -
						box->parent->padding[उजवा_2];
				find_sides(block->float_children, शीर्ष, शीर्ष,
						&x0, &x1, &डावा, &उजवा);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->parent->width -
						box->parent->padding[डावा_2] -
						box->parent->padding[उजवा_2] -
						x1;
			}
			layout_block_find_dimensions(&content->unit_len_ctx,
					box->parent->width,
					viewport_height, lm, rm, box);
			यदि (box->type == BOX_BLOCK && !(box->flags & IFRAME)) {
				layout_block_add_scrollbar(box, उजवा_2);
				layout_block_add_scrollbar(box, BOTTOM);
			}
		} अन्यथा यदि (box->type == BOX_TABLE) {
			यदि (box->style != रिक्त) {
				गणना css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style, &width,
						&unit);

				यदि (wtype == CSS_WIDTH_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					पूर्णाङ्क x0, x1, शीर्ष;
					संरचना box *डावा, *उजवा;
					शीर्ष = cy + max_pos_margin -
							max_neg_margin;
					शीर्ष = (शीर्ष > y) ? शीर्ष : y;
					x0 = cx;
					x1 = cx + box->parent->width -
						box->parent->padding[डावा_2] -
						box->parent->padding[उजवा_2];
					find_sides(block->float_children,
						शीर्ष, शीर्ष, &x0, &x1,
						&डावा, &उजवा);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->parent->width -
						box->parent->padding[डावा_2] -
						box->parent->padding[उजवा_2] -
						x1;
				}
			}
			यदि (!layout_table(box, box->parent->width - lm - rm,
					content))
				प्रत्यागम false;
			layout_solve_width(box, box->parent->width, box->width,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->parent->padding[डावा_2] + box->margin[डावा_2] +
				box->border[डावा_2].width;
		cx += box->x;

		/* Position box: vertical. */
		यदि (box->border[शीर्ष_2].width) {
			box->y += box->border[शीर्ष_2].width;
			cy += box->border[शीर्ष_2].width;
		}

		/* Vertical margin */
		यदि (((box->type == BOX_BLOCK && (box->flags & HAS_HEIGHT)) ||
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
		यदि (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		यदि (box->type == BOX_FLEX ||
				(box->type == BOX_BLOCK && box->style &&
				 (overflow_x != CSS_OVERFLOW_VISIBLE ||
				  overflow_y != CSS_OVERFLOW_VISIBLE))) {

			यदि (box->type == BOX_FLEX) {
				यदि (!layout_flex(box, box->width, content)) {
					प्रत्यागम false;
				}
			} अन्यथा {
				layout_block_context(box,
						viewport_height, content);
			}

			cy += box->padding[शीर्ष_2];

			यदि (box->height == AUTO) {
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
			गच्छ advance_to_next_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->width);

		/* Layout (except tables). */
		यदि (box->object) {
			यदि (!layout_block_object(box))
				प्रत्यागम false;

		} अन्यथा यदि (box->type == BOX_INLINE_CONTAINER) {
			box->width = box->parent->width;
			यदि (!layout_inline_container(box, box->width, block,
					cx, cy, content))
				प्रत्यागम false;

		} अन्यथा यदि (box->type == BOX_TABLE) {
			/* Move down to avoid floats if necessary. */
			पूर्णाङ्क x0, x1;
			संरचना box *डावा, *उजवा;
			y = cy;
			यावत् (1) {
				गणना css_width_e wtype;
				css_fixed width = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_width(box->style,
						&width, &unit);

				x0 = cx;
				x1 = cx + box->parent->width;
				find_sides(block->float_children, y,
						y + box->height,
						&x0, &x1, &डावा, &उजवा);
				यदि (wtype == CSS_WIDTH_AUTO)
					भङ्ग;
				यदि (box->width <= x1 - x0)
					भङ्ग;
				यदि (!डावा && !उजवा)
					भङ्ग;
				अन्यथा यदि (!डावा)
					y = उजवा->y + उजवा->height + 1;
				अन्यथा यदि (!उजवा)
					y = डावा->y + डावा->height + 1;
				अन्यथा यदि (डावा->y + डावा->height <
						उजवा->y + उजवा->height)
					y = डावा->y + डावा->height + 1;
				अन्यथा
					y = उजवा->y + उजवा->height + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		यदि (box->type == BOX_BLOCK && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			यदि (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = रिक्त;
			}

			y = box->padding[शीर्ष_2];
			box = box->children;
			box->y = y;
			cy += y;
			अनुवर्त;
		} अन्यथा यदि (box->type == BOX_BLOCK || box->object ||
				box->flags & IFRAME)
			cy += box->padding[शीर्ष_2];

		यदि (box->type == BOX_BLOCK && box->height == AUTO) {
			box->height = 0;
			layout_block_add_scrollbar(box, BOTTOM);
		}

		cy += box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width;
		cx -= box->x;
		y = box->y + box->padding[शीर्ष_2] + box->height +
				box->padding[BOTTOM] +
				box->border[BOTTOM].width;

	advance_to_next_box:
		यदि (!box->next) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			कुरु {
				यदि (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = रिक्त;
				}

				/* Apply bottom margin */
				यदि (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				अन्यथा यदि (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->parent;
				यदि (box == block)
					भङ्ग;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				यदि (box->type == BOX_BLOCK &&
						box->flags & MAKE_HEIGHT) {
					margin_collapse = रिक्त;
					in_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				यदि (box->height == AUTO) {
					box->height = y - box->padding[शीर्ष_2];

					यदि (box->type == BOX_BLOCK)
						layout_block_add_scrollbar(box,
								BOTTOM);
				} अन्यथा
					cy += box->height -
							(y - box->padding[शीर्ष_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				यदि (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_height(
							&content->unit_len_ctx,
							box, रिक्त)) {
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

			} यावत् (box->next == रिक्त);
			यदि (box == block)
				भङ्ग;
		}

		/* To next sibling. */

		यदि (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = रिक्त;
		}

		यदि (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		अन्यथा यदि (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->next;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	प्रति (box = block->float_children; box; box = box->next_float) {
		y = box->y + box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].width + box->margin[BOTTOM];
		यदि (cy < y)
			cy = y;
	}

	यदि (block->height == AUTO) {
		block->height = cy - block->padding[शीर्ष_2];
		यदि (block->type == BOX_BLOCK)
			layout_block_add_scrollbar(block, BOTTOM);
	}

	यदि (block->style && css_computed_position(block->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_height(&content->unit_len_ctx, block, रिक्त);
	}

	यदि (block->gadget &&
			(block->gadget->type == GADGET_TEXTAREA ||
			block->gadget->type == GADGET_PASSWORD ||
			block->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		पूर्णाङ्क ta_width = block->padding[डावा_2] + block->width +
				block->padding[उजवा_2];
		पूर्णाङ्क ta_height = block->padding[शीर्ष_2] + block->height +
				block->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				block->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_set_layout(block->gadget->data.text.ta,
				&fstyle, ta_width, ta_height,
				block->padding[शीर्ष_2], block->padding[उजवा_2],
				block->padding[BOTTOM], block->padding[डावा_2]);
	}

	प्रत्यागम true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
स्थिर bool
layout__get_element_tag(
		ध्रुव dom_गाठ *गाठ,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_गाठ_type गाठ_type;
	dom_exception exc;

	exc = dom_गाठ_get_गाठ_type(गाठ, &गाठ_type);
	यदि (exc != DOM_NO_ERR ||
	    गाठ_type != DOM_ELEMENT_गाठ) {
		प्रत्यागम false;
	}

	exc = dom_html_element_get_tag_type(गाठ, &element_type);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम false;
	}

	*type = element_type;
	प्रत्यागम true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
स्थिर अन्तःस्थ bool
layout__check_element_type(
		ध्रुव dom_गाठ *गाठ,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	यदि (!layout__get_element_tag(गाठ, &element_type)) {
		प्रत्यागम false;
	}

	प्रत्यागम element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
स्थिर bool
layout_get_li_मूल्य(dom_गाठ *li_गाठ, dom_long *मूल्य_out)
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
	exc = dom_element_has_attribute(li_गाठ,
			corestring_dom_मूल्य,
			&has_मूल्य);
	यदि (exc != DOM_NO_ERR || has_मूल्य == false) {
		प्रत्यागम false;
	}

	exc = dom_html_li_element_get_मूल्य(
			(dom_html_li_element *)li_गाठ,
			&मूल्य);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम false;
	}

	*मूल्य_out = मूल्य;
	प्रत्यागम true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
स्थिर bool
layout__get_ol_start(dom_गाठ *ol_गाठ, dom_long *start_out)
{
	dom_exception exc;
	dom_long start;
	bool has_start;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_गाठ,
			corestring_dom_start,
			&has_start);
	यदि (exc != DOM_NO_ERR || has_start == false) {
		प्रत्यागम false;
	}

	exc = dom_html_olist_element_get_start(
			(dom_html_olist_element *)ol_गाठ,
			&start);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम false;
	}

	*start_out = start;
	प्रत्यागम true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
स्थिर bool
layout__get_ol_reversed(dom_गाठ *ol_गाठ)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_गाठ,
			corestring_dom_reversed,
			&has_reversed);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम false;
	}

	प्रत्यागम has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
स्थिर bool
layout_get_list_item_मोजणी(
		dom_गाठ *list_owner, dom_long *मोजणी_out)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_गाठ *child;
	पूर्णाङ्क मोजणी;

	यदि (list_owner == रिक्त) {
		प्रत्यागम false;
	}

	यदि (!layout__get_element_tag(list_owner, &tag_type)) {
		प्रत्यागम false;
	}

	यदि (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		प्रत्यागम false;
	}

	exc = dom_गाठ_get_first_child(list_owner, &child);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम false;
	}

	मोजणी = 0;
	यावत् (child != रिक्त) {
		dom_गाठ *temp_गाठ;

		यदि (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			संरचना box *child_box;
			यदि (dom_गाठ_get_user_data(child,
					corestring_dom_ns_किल्ली_box_गाठ_data,
					&child_box) != DOM_NO_ERR) {
				dom_गाठ_unref(child);
				प्रत्यागम false;
			}

			यदि (child_box != रिक्त &&
			    child_box->list_marker != रिक्त) {
				मोजणी++;
			}
		}

		exc = dom_गाठ_get_next_sibling(child, &temp_गाठ);
		dom_गाठ_unref(child);
		यदि (exc != DOM_NO_ERR) {
			प्रत्यागम false;
		}

		child = temp_गाठ;
	}

	*मोजणी_out = मोजणी;
	प्रत्यागम true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
स्थिर शून्य
layout_ordered_list_मोजणी(
		संरचना box *box)
{
	dom_html_element_type tag_type;
	dom_exception exc;
	dom_गाठ *child;
	पूर्णाङ्क step = 1;
	dom_long next;

	यदि (box->गाठ == रिक्त) {
		प्रत्यागम;
	}

	यदि (!layout__get_element_tag(box->गाठ, &tag_type)) {
		प्रत्यागम;
	}

	यदि (tag_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    tag_type != DOM_HTML_ELEMENT_TYPE_UL) {
		प्रत्यागम;
	}

	next = 1;
	यदि (tag_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_start = layout__get_ol_start(box->गाठ, &next);
		bool have_reversed = layout__get_ol_reversed(box->गाठ);

		यदि (have_reversed) {
			step = -1;
		}

		यदि (!have_start && have_reversed) {
			layout_get_list_item_मोजणी(box->गाठ, &next);
		}
	}

	exc = dom_गाठ_get_first_child(box->गाठ, &child);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम;
	}

	यावत् (child != रिक्त) {
		dom_गाठ *temp_गाठ;

		यदि (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			संरचना box *child_box;

			यदि (dom_गाठ_get_user_data(child,
					corestring_dom_ns_किल्ली_box_गाठ_data,
					&child_box) != DOM_NO_ERR) {
				dom_गाठ_unref(child);
				प्रत्यागम;
			}

			यदि (child_box != रिक्त &&
			    child_box->list_marker != रिक्त) {
				dom_long मूल्य;
				संरचना box *marker = child_box->list_marker;
				यदि (layout_get_li_मूल्य(child, &मूल्य)) {
					marker->list_मूल्य = मूल्य;
					next = marker->list_मूल्य;
				} अन्यथा {
					marker->list_मूल्य = next;
				}
				next += step;
			}
		}

		exc = dom_गाठ_get_next_sibling(child, &temp_गाठ);
		dom_गाठ_unref(child);
		यदि (exc != DOM_NO_ERR) {
			प्रत्यागम;
		}

		child = temp_गाठ;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
स्थिर शून्य
layout__set_numerical_marker_text(
		ध्रुव html_content *content,
		संरचना box *box)
{
	संरचना box *marker = box->list_marker;
	size_t counter_len;
	css_error css_res;
	गणना {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_SIZE = 20,
	};

	marker->text = talloc_array(content->bctx, वर्ण, LIST_MARKER_SIZE);
	यदि (marker->text == रिक्त) {
		प्रत्यागम;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_मूल्य,
			marker->text, LIST_MARKER_SIZE, &counter_len);
	यदि (css_res == CSS_OK) {
		यदि (counter_len > LIST_MARKER_SIZE) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->text = talloc_realloc(content->bctx,
					marker->text,
					वर्ण,
					counter_len);
			यदि (marker->text == रिक्त) {
				प्रत्यागम;
			}
			css_computed_format_list_style(box->style,
					marker->list_मूल्य, marker->text,
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
स्थिर bool
layout__list_item_is_numerical(
		ध्रुव संरचना box *b)
{
	गणना css_list_style_type_e t = css_computed_list_style_type(b->style);

	विकल्प (t) {
	पक्ष CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	पक्ष CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	पक्ष CSS_LIST_STYLE_TYPE_SQUARE: /* Fall through. */
	पक्ष CSS_LIST_STYLE_TYPE_NONE:
		प्रत्यागम false;

	स्वभाव:
		प्रत्यागम true;
	}
}

/**
 * Layout list markers.
 */
स्थिर शून्य
layout_lists(ध्रुव html_content *content, संरचना box *box)
{
	संरचना box *child;

	layout_ordered_list_मोजणी(box);

	प्रति (child = box->children; child; child = child->next) {
		यदि (child->list_marker) {
			संरचना box *marker = child->list_marker;

			यदि (layout__list_item_is_numerical(child)) {
				यदि (marker->text == रिक्त) {
					layout__set_numerical_marker_text(
							content, child);
				}
			}
			यदि (marker->object) {
				marker->width =
					content_get_width(marker->object);
				marker->x = -marker->width;
				marker->height =
					content_get_height(marker->object);
				marker->y = (ओळ_height(
						&content->unit_len_ctx,
						marker->style) -
						marker->height) / 2;
			} अन्यथा यदि (marker->text) {
				यदि (marker->width == UNKNOWN_WIDTH) {
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
				marker->height = ओळ_height(
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
स्थिर शून्य
layout_compute_offsets(ध्रुव css_unit_ctx *unit_len_ctx,
		       संरचना box *box,
		       संरचना box *containing_block,
		       पूर्णाङ्क *शीर्ष,
		       पूर्णाङ्क *उजवा,
		       पूर्णाङ्क *bottom,
		       पूर्णाङ्क *डावा)
{
	uint32_t type;
	css_fixed मूल्य = 0;
	css_unit unit = CSS_UNIT_PX;

	प्रतिज्ञा(containing_block->width != UNKNOWN_WIDTH);
	प्रतिज्ञा(containing_block->width != AUTO);
	प्रतिज्ञा(containing_block->height != AUTO);

	/*डावा*/
	type = css_computed_डावा(box->style, &मूल्य, &unit);
	यदि (type == CSS_डावा_SET) {
		यदि (unit == CSS_UNIT_PCT) {
			*डावा = FPCT_OF_INT_TOINT(मूल्य,
					containing_block->width);
		} अन्यथा {
			*डावा = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					मूल्य, unit));
		}
	} अन्यथा {
		*डावा = AUTO;
	}

	/*उजवा*/
	type = css_computed_उजवा(box->style, &मूल्य, &unit);
	यदि (type == CSS_उजवा_SET) {
		यदि (unit == CSS_UNIT_PCT) {
			*उजवा = FPCT_OF_INT_TOINT(मूल्य,
					containing_block->width);
		} अन्यथा {
			*उजवा = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					मूल्य, unit));
		}
	} अन्यथा {
		*उजवा = AUTO;
	}

	/*शीर्ष*/
	type = css_computed_शीर्ष(box->style, &मूल्य, &unit);
	यदि (type == CSS_शीर्ष_SET) {
		यदि (unit == CSS_UNIT_PCT) {
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
	यदि (type == CSS_BOTTOM_SET) {
		यदि (unit == CSS_UNIT_PCT) {
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
स्थिर bool
layout_absolute(संरचना box *box,
		संरचना box *containing_block,
		पूर्णाङ्क cx, पूर्णाङ्क cy,
		html_content *content)
{
	पूर्णाङ्क static_डावा, static_शीर्ष;  /* static position */
	पूर्णाङ्क शीर्ष, उजवा, bottom, डावा;
	पूर्णाङ्क width, height, max_width, min_width;
	पूर्णाङ्क *margin = box->margin;
	पूर्णाङ्क *padding = box->padding;
	संरचना box_border *border = box->border;
	पूर्णाङ्क available_width = containing_block->width;
	पूर्णाङ्क space;

	प्रतिज्ञा(box->type == BOX_BLOCK || box->type == BOX_TABLE ||
			box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_डावा = cx + box->x;
	static_शीर्ष = cy + box->y;

	यदि (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_block->width += containing_block->padding[डावा_2] +
				containing_block->padding[उजवा_2];
		containing_block->height += containing_block->padding[शीर्ष_2] +
				containing_block->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_block,
			&शीर्ष, &उजवा, &bottom, &डावा);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_block;
	layout_find_dimensions(&content->unit_len_ctx, available_width, -1,
			box, box->style, &width, &height,
			&max_width, &min_width, 0, 0,
			margin, padding, border);
	box->float_container = रिक्त;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      डावा, margin[डावा_2], border[डावा_2].width, padding[डावा_2], width,
	      padding[उजवा_2], border[उजवा_2].width, margin[उजवा_2], उजवा,
	      containing_block->width);


	यदि (डावा == AUTO && width == AUTO && उजवा == AUTO) {
		यदि (margin[डावा_2] == AUTO)
			margin[डावा_2] = 0;
		यदि (margin[उजवा_2] == AUTO)
			margin[उजवा_2] = 0;
		डावा = static_डावा;

		width = min(max(box->min_width, available_width),
			box->max_width);
		width -= box->margin[डावा_2] + box->border[डावा_2].width +
			box->padding[डावा_2] + box->padding[उजवा_2] +
			box->border[उजवा_2].width + box->margin[उजवा_2];

		/* Adjust for {min|max}-width */
		यदि (max_width >= 0 && width > max_width) width = max_width;
		यदि (width < min_width) width = min_width;

		उजवा = containing_block->width -
			डावा -
			margin[डावा_2] - border[डावा_2].width - padding[डावा_2] -
			width -
			padding[उजवा_2] - border[उजवा_2].width - margin[उजवा_2];
	} अन्यथा यदि (डावा != AUTO && width != AUTO && उजवा != AUTO) {

		/* Adjust for {min|max}-width */
		यदि (max_width >= 0 && width > max_width) width = max_width;
		यदि (min_width >  0 && width < min_width) width = min_width;

		यदि (margin[डावा_2] == AUTO && margin[उजवा_2] == AUTO) {
			space = containing_block->width -
					डावा - border[डावा_2].width -
					padding[डावा_2] - width - padding[उजवा_2] -
					border[उजवा_2].width - उजवा;
			यदि (space < 0) {
				margin[डावा_2] = 0;
				margin[उजवा_2] = space;
			} अन्यथा {
				margin[डावा_2] = margin[उजवा_2] = space / 2;
			}
		} अन्यथा यदि (margin[डावा_2] == AUTO) {
			margin[डावा_2] = containing_block->width -
					डावा - border[डावा_2].width -
					padding[डावा_2] - width - padding[उजवा_2] -
					border[उजवा_2].width - margin[उजवा_2] -
					उजवा;
		} अन्यथा यदि (margin[उजवा_2] == AUTO) {
			margin[उजवा_2] = containing_block->width -
					डावा - margin[डावा_2] -
					border[डावा_2].width -
					padding[डावा_2] - width - padding[उजवा_2] -
					border[उजवा_2].width - उजवा;
		} अन्यथा {
			उजवा = containing_block->width -
					डावा - margin[डावा_2] -
					border[डावा_2].width -
					padding[डावा_2] - width - padding[उजवा_2] -
					border[उजवा_2].width - margin[उजवा_2];
		}
	} अन्यथा {
		यदि (margin[डावा_2] == AUTO)
			margin[डावा_2] = 0;
		यदि (margin[उजवा_2] == AUTO)
			margin[उजवा_2] = 0;

		यदि (डावा == AUTO && width == AUTO && उजवा != AUTO) {
			available_width -= उजवा;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[डावा_2] + box->border[डावा_2].width +
				box->padding[डावा_2] + box->padding[उजवा_2] +
				box->border[उजवा_2].width + box->margin[उजवा_2];

			/* Adjust for {min|max}-width */
			यदि (max_width >= 0 && width > max_width)
				width = max_width;
			यदि (width < min_width)
				width = min_width;

			डावा = containing_block->width -
					margin[डावा_2] - border[डावा_2].width -
					padding[डावा_2] - width - padding[उजवा_2] -
					border[उजवा_2].width - margin[उजवा_2] -
					उजवा;
		} अन्यथा यदि (डावा == AUTO && width != AUTO && उजवा == AUTO) {

			/* Adjust for {min|max}-width */
			यदि (max_width >= 0 && width > max_width)
				width = max_width;
			यदि (min_width >  0 && width < min_width)
				width = min_width;

			डावा = static_डावा;
			उजवा = containing_block->width -
					डावा - margin[डावा_2] -
					border[डावा_2].width -
					padding[डावा_2] - width - padding[उजवा_2] -
					border[उजवा_2].width - margin[उजवा_2];
		} अन्यथा यदि (डावा != AUTO && width == AUTO && उजवा == AUTO) {
			available_width -= डावा;

			width = min(max(box->min_width, available_width),
				box->max_width);
			width -= box->margin[डावा_2] + box->border[डावा_2].width +
				box->padding[डावा_2] + box->padding[उजवा_2] +
				box->border[उजवा_2].width + box->margin[उजवा_2];

			/* Adjust for {min|max}-width */
			यदि (max_width >= 0 && width > max_width)
				width = max_width;
			यदि (width < min_width)
				width = min_width;

			उजवा = containing_block->width -
					डावा - margin[डावा_2] -
					border[डावा_2].width -
					padding[डावा_2] - width - padding[उजवा_2] -
					border[उजवा_2].width - margin[उजवा_2];
		} अन्यथा यदि (डावा == AUTO && width != AUTO && उजवा != AUTO) {

			/* Adjust for {min|max}-width */
			यदि (max_width >= 0 && width > max_width)
				width = max_width;
			यदि (width < min_width)
				width = min_width;

			डावा = containing_block->width -
					margin[डावा_2] - border[डावा_2].width -
					padding[डावा_2] - width - padding[उजवा_2] -
					border[उजवा_2].width - margin[उजवा_2] -
					उजवा;
		} अन्यथा यदि (डावा != AUTO && width == AUTO && उजवा != AUTO) {
			width = containing_block->width -
					डावा - margin[डावा_2] -
					border[डावा_2].width -
					padding[डावा_2] - padding[उजवा_2] -
					border[उजवा_2].width - margin[उजवा_2] -
					उजवा;

			/* Adjust for {min|max}-width */
			यदि (max_width >= 0 && width > max_width)
				width = max_width;
			यदि (width < min_width)
				width = min_width;

		} अन्यथा यदि (डावा != AUTO && width != AUTO && उजवा == AUTO) {

			/* Adjust for {min|max}-width */
			यदि (max_width >= 0 && width > max_width)
				width = max_width;
			यदि (width < min_width)
				width = min_width;

			उजवा = containing_block->width -
					डावा - margin[डावा_2] -
					border[डावा_2].width -
					padding[डावा_2] - width - padding[उजवा_2] -
					border[उजवा_2].width - margin[उजवा_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      डावा, margin[डावा_2], border[डावा_2].width, padding[डावा_2], width,
	      padding[उजवा_2], border[उजवा_2].width, margin[उजवा_2], उजवा,
	      containing_block->width);

	box->x = डावा + margin[डावा_2] + border[डावा_2].width - cx;
	यदि (containing_block->type == BOX_BLOCK ||
			containing_block->type == BOX_INLINE_BLOCK ||
			containing_block->type == BOX_TABLE_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_block->width -= containing_block->padding[डावा_2] +
				containing_block->padding[उजवा_2];
	} अन्यथा {
		/** \todo inline ancestors */
	}
	box->width = width;
	box->height = height;

	यदि (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->object || box->flags & IFRAME) {
		यदि (!layout_block_context(box, -1, content))
			प्रत्यागम false;
	} अन्यथा यदि (box->type == BOX_TABLE) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		/* \todo  layout_table considers margins etc. again */
		यदि (!layout_table(box, width, content))
			प्रत्यागम false;
		box->float_container = रिक्त;
		layout_solve_width(box, box->parent->width, box->width, 0, 0,
				-1, -1);
	} अन्यथा यदि (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_block;
		यदि (!layout_flex(box, width, content))
			प्रत्यागम false;
		box->float_container = रिक्त;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      शीर्ष, margin[शीर्ष_2], border[शीर्ष_2].width, padding[शीर्ष_2], height,
	      padding[BOTTOM], border[BOTTOM].width, margin[BOTTOM], bottom,
	      containing_block->height);

	यदि (शीर्ष == AUTO && height == AUTO && bottom == AUTO) {
		शीर्ष = static_शीर्ष;
		height = box->height;
		यदि (margin[शीर्ष_2] == AUTO)
			margin[शीर्ष_2] = 0;
		यदि (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_block->height -
				शीर्ष - margin[शीर्ष_2] - border[शीर्ष_2].width -
				padding[शीर्ष_2] - height - padding[BOTTOM] -
				border[BOTTOM].width - margin[BOTTOM];
	} अन्यथा यदि (शीर्ष != AUTO && height != AUTO && bottom != AUTO) {
		यदि (margin[शीर्ष_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_block->height -
					शीर्ष - border[शीर्ष_2].width - padding[शीर्ष_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - bottom;
			margin[शीर्ष_2] = margin[BOTTOM] = space / 2;
		} अन्यथा यदि (margin[शीर्ष_2] == AUTO) {
			margin[शीर्ष_2] = containing_block->height -
					शीर्ष - border[शीर्ष_2].width - padding[शीर्ष_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} अन्यथा यदि (margin[BOTTOM] == AUTO) {
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
		यदि (margin[शीर्ष_2] == AUTO)
			margin[शीर्ष_2] = 0;
		यदि (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		यदि (शीर्ष == AUTO && height == AUTO && bottom != AUTO) {
			height = box->height;
			शीर्ष = containing_block->height -
					margin[शीर्ष_2] - border[शीर्ष_2].width -
					padding[शीर्ष_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} अन्यथा यदि (शीर्ष == AUTO && height != AUTO && bottom == AUTO) {
			शीर्ष = static_शीर्ष;
			bottom = containing_block->height -
					शीर्ष - margin[शीर्ष_2] - border[शीर्ष_2].width -
					padding[शीर्ष_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} अन्यथा यदि (शीर्ष != AUTO && height == AUTO && bottom == AUTO) {
			height = box->height;
			bottom = containing_block->height -
					शीर्ष - margin[शीर्ष_2] - border[शीर्ष_2].width -
					padding[शीर्ष_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM];
		} अन्यथा यदि (शीर्ष == AUTO && height != AUTO && bottom != AUTO) {
			शीर्ष = containing_block->height -
					margin[शीर्ष_2] - border[शीर्ष_2].width -
					padding[शीर्ष_2] - height -
					padding[BOTTOM] - border[BOTTOM].width -
					margin[BOTTOM] - bottom;
		} अन्यथा यदि (शीर्ष != AUTO && height == AUTO && bottom != AUTO) {
			height = containing_block->height -
					शीर्ष - margin[शीर्ष_2] - border[शीर्ष_2].width -
					padding[शीर्ष_2] - padding[BOTTOM] -
					border[BOTTOM].width - margin[BOTTOM] -
					bottom;
		} अन्यथा यदि (शीर्ष != AUTO && height != AUTO && bottom == AUTO) {
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
	यदि (containing_block->type == BOX_BLOCK ||
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

	प्रत्यागम true;
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
स्थिर bool
layout_position_absolute(संरचना box *box,
			 संरचना box *containing_block,
			 पूर्णाङ्क cx, पूर्णाङ्क cy,
			 html_content *content)
{
	संरचना box *c;

	प्रति (c = box->children; c; c = c->next) {
		यदि ((c->type == BOX_BLOCK || c->type == BOX_TABLE ||
				c->type == BOX_INLINE_BLOCK ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			यदि (!layout_absolute(c, containing_block,
					cx, cy, content))
				प्रत्यागम false;
			यदि (!layout_position_absolute(c, c, 0, 0, content))
				प्रत्यागम false;
		} अन्यथा यदि (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			यदि (!layout_position_absolute(c, c, 0, 0, content))
				प्रत्यागम false;
		} अन्यथा {
			पूर्णाङ्क px, py;
			यदि (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_डावा ||
					css_computed_float(c->style) ==
						CSS_FLOAT_उजवा)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				संरचना box *p;
				px = c->x;
				py = c->y;
				प्रति (p = box->parent; p && !p->float_children;
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
			यदि (!layout_position_absolute(c, containing_block,
					cx + px, cy + py, content))
				प्रत्यागम false;
		}
	}

	प्रत्यागम true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
स्थिर शून्य layout_compute_relative_offset(
		ध्रुव css_unit_ctx *unit_len_ctx,
		संरचना box *box,
		पूर्णाङ्क *x,
		पूर्णाङ्क *y)
{
	पूर्णाङ्क डावा, उजवा, शीर्ष, bottom;
	संरचना box *containing_block;

	प्रतिज्ञा(box && box->parent && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	यदि (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_डावा ||
			css_computed_float(box->style) == CSS_FLOAT_उजवा)) {
		containing_block = box->float_container;
	} अन्यथा {
		containing_block = box->parent;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_block,
			&शीर्ष, &उजवा, &bottom, &डावा);

	यदि (डावा == AUTO && उजवा == AUTO)
		डावा = उजवा = 0;
	अन्यथा यदि (डावा == AUTO)
		/* left is auto => computed = -right */
		डावा = -उजवा;
	अन्यथा यदि (उजवा == AUTO)
		/* right is auto => computed = -left */
		उजवा = -डावा;
	अन्यथा {
		/* over constrained => examine direction property
		 * of containing block */
		यदि (containing_block->style &&
				css_computed_direction(
				containing_block->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			डावा = -उजवा;
		} अन्यथा {
			/* assume LTR in all other cases */
			उजवा = -डावा;
		}
	}

	प्रतिज्ञा(डावा == -उजवा);

	यदि (शीर्ष == AUTO && bottom == AUTO) {
		शीर्ष = bottom = 0;
	} अन्यथा यदि (शीर्ष == AUTO) {
		शीर्ष = -bottom;
	} अन्यथा {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -शीर्ष;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", डावा,
	      उजवा, शीर्ष, bottom);

	*x = डावा;
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
स्थिर शून्य
layout_position_relative(
		ध्रुव css_unit_ctx *unit_len_ctx,
		संरचना box *मूळ,
		संरचना box *fp,
		पूर्णाङ्क fx,
		पूर्णाङ्क fy)
{
	संरचना box *box; /* for children of "root" */
	संरचना box *fn;  /* for block formatting context box for children of
			  * "box" */
	संरचना box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	पूर्णाङ्क x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	पूर्णाङ्क fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	प्रतिज्ञा(मूळ);

	/* Normal children */
	प्रति (box = मूळ->children; box; box = box->next) {

		यदि (box->type == BOX_TEXT)
			अनुवर्त;

		/* If relatively positioned, get offsets */
		यदि (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_offset(
					unit_len_ctx, box, &x, &y);
		अन्यथा
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		यदि (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_डावा ||
				css_computed_float(box->style) ==
					CSS_FLOAT_उजवा) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			प्रति (fc = fp->float_children; fc; fc = fc->next_float) {
				यदि (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		यदि (box->float_children) {
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
		यदि (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			अनुवर्त;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		यदि (box->type == BOX_INLINE && box->inline_end) {
			संरचना box *b;
			प्रति (b = box->next; b && b != box->inline_end;
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
स्थिर शून्य
layout_get_box_bbox(
		ध्रुव css_unit_ctx *unit_len_ctx,
		संरचना box *box,
		पूर्णाङ्क *desc_x0, पूर्णाङ्क *desc_y0,
		पूर्णाङ्क *desc_x1, पूर्णाङ्क *desc_y1)
{
	*desc_x0 = -box->border[डावा_2].width;
	*desc_y0 = -box->border[शीर्ष_2].width;
	*desc_x1 = box->padding[डावा_2] + box->width + box->padding[उजवा_2] +
			box->border[उजवा_2].width;
	*desc_y1 = box->padding[शीर्ष_2] + box->height + box->padding[BOTTOM] +
			box->border[BOTTOM].width;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	यदि (box->type == BOX_BLOCK && box->style != रिक्त &&
			css_computed_overflow_y(box->style) ==
					CSS_OVERFLOW_VISIBLE &&
			box->object == रिक्त) {
		css_fixed font_size = 0;
		css_unit font_unit = CSS_UNIT_PT;
		पूर्णाङ्क text_height;

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
स्थिर शून्य
layout_update_descendant_bbox(
		ध्रुव css_unit_ctx *unit_len_ctx,
		संरचना box *box,
		संरचना box *child,
		पूर्णाङ्क off_x,
		पूर्णाङ्क off_y)
{
	पूर्णाङ्क child_desc_x0, child_desc_y0, child_desc_x1, child_desc_y1;

	/* get coordinates of child relative to box */
	पूर्णाङ्क child_x = child->x - off_x;
	पूर्णाङ्क child_y = child->y - off_y;

	bool html_object = (child->object &&
			content_get_type(child->object) == CONTENT_HTML);

	गणना css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	गणना css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;

	यदि (child->style != रिक्त) {
		overflow_x = css_computed_overflow_x(child->style);
		overflow_y = css_computed_overflow_y(child->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, child,
			&child_desc_x0, &child_desc_y0,
			&child_desc_x1, &child_desc_y1);

	यदि (overflow_x == CSS_OVERFLOW_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		child_desc_x0 = child->descendant_x0;
		child_desc_x1 = child->descendant_x1;
	}
	यदि (overflow_y == CSS_OVERFLOW_VISIBLE &&
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
	यदि (child_desc_x0 < box->descendant_x0)
		box->descendant_x0 = child_desc_x0;
	यदि (child_desc_y0 < box->descendant_y0)
		box->descendant_y0 = child_desc_y0;
	यदि (box->descendant_x1 < child_desc_x1)
		box->descendant_x1 = child_desc_x1;
	यदि (box->descendant_y1 < child_desc_y1)
		box->descendant_y1 = child_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
स्थिर शून्य layout_calculate_descendant_bboxes(
		ध्रुव css_unit_ctx *unit_len_ctx,
		संरचना box *box)
{
	संरचना box *child;

	प्रतिज्ञा(box->width != UNKNOWN_WIDTH);
	प्रतिज्ञा(box->height != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	यदि (box->object && content_get_type(box->object) == CONTENT_HTML) {
		यदि (box->descendant_x1 < content_get_width(box->object))
			box->descendant_x1 = content_get_width(box->object);
		यदि (box->descendant_y1 < content_get_height(box->object))
			box->descendant_y1 = content_get_height(box->object);
	}

	यदि (box->iframe != रिक्त) {
		पूर्णाङ्क x, y;
		box_coords(box, &x, &y);

		browser_window_set_position(box->iframe, x, y);
		browser_window_set_dimensions(box->iframe,
				box->width, box->height);
		browser_window_reformat(box->iframe, true,
				box->width, box->height);
	}

	यदि (box->type == BOX_INLINE || box->type == BOX_TEXT)
		प्रत्यागम;

	यदि (box->type == BOX_INLINE_END) {
		box = box->inline_end;
		प्रति (child = box->next; child;
				child = child->next) {
			यदि (child->type == BOX_FLOAT_डावा ||
					child->type == BOX_FLOAT_उजवा)
				अनुवर्त;

			layout_update_descendant_bbox(unit_len_ctx, box, child,
					box->x, box->y);

			यदि (child == box->inline_end)
				भङ्ग;
		}
		प्रत्यागम;
	}

	यदि (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		प्रत्यागम;

	प्रति (child = box->children; child; child = child->next) {
		यदि (child->type == BOX_FLOAT_डावा ||
				child->type == BOX_FLOAT_उजवा)
			अनुवर्त;

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		यदि (box->style && css_computed_overflow_x(box->style) ==
				CSS_OVERFLOW_HIDDEN &&
				css_computed_overflow_y(box->style) ==
				CSS_OVERFLOW_HIDDEN)
			अनुवर्त;

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	प्रति (child = box->float_children; child; child = child->next_float) {
		प्रतिज्ञा(child->type == BOX_FLOAT_डावा ||
				child->type == BOX_FLOAT_उजवा);

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	यदि (box->list_marker) {
		child = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, पूर्णाङ्क width, पूर्णाङ्क height)
{
	bool ret;
	संरचना box *doc = content->layout;
	ध्रुव संरचना gui_layout_table *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			width, height, nsurl_access(content_get_url(
					&content->base)));

	layout_minmax_block(doc, font_func, content);

	layout_block_find_dimensions(&content->unit_len_ctx,
			width, height, 0, 0, doc);
	doc->x = doc->margin[डावा_2] + doc->border[डावा_2].width;
	doc->y = doc->margin[शीर्ष_2] + doc->border[शीर्ष_2].width;
	width -= doc->margin[डावा_2] + doc->border[डावा_2].width +
			doc->padding[डावा_2] + doc->padding[उजवा_2] +
			doc->border[उजवा_2].width + doc->margin[उजवा_2];
	यदि (width < 0) {
		width = 0;
	}
	doc->width = width;

	ret = layout_block_context(doc, height, content);

	/* make <html> and <body> fill available height */
	यदि (doc->y + doc->padding[शीर्ष_2] + doc->height + doc->padding[BOTTOM] +
			doc->border[BOTTOM].width + doc->margin[BOTTOM] <
			height) {
		doc->height = height - (doc->y + doc->padding[शीर्ष_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].width +
				doc->margin[BOTTOM]);
		यदि (doc->children)
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

	प्रत्यागम ret;
}
