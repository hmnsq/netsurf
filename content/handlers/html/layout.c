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
	[शिखरम्_2]    = css_computed_margin_शिखरम्,
	[दक्षिणः_2]  = css_computed_margin_दक्षिणः,
	[BOTTOM] = css_computed_margin_bottom,
	[वामः_2]   = css_computed_margin_वामः,
};

/** Array of per-side access functions for computed style paddings. */
ध्रुव css_len_func padding_funcs[4] = {
	[शिखरम्_2]    = css_computed_padding_शिखरम्,
	[दक्षिणः_2]  = css_computed_padding_दक्षिणः,
	[BOTTOM] = css_computed_padding_bottom,
	[वामः_2]   = css_computed_padding_वामः,
};

/** Array of per-side access functions for computed style border_widths. */
ध्रुव css_len_func border_विस्तारः_funcs[4] = {
	[शिखरम्_2]    = css_computed_border_शिखरम्_विस्तारः,
	[दक्षिणः_2]  = css_computed_border_दक्षिणः_विस्तारः,
	[BOTTOM] = css_computed_border_bottom_विस्तारः,
	[वामः_2]   = css_computed_border_वामः_विस्तारः,
};

/** Array of per-side access functions for computed style border styles. */
ध्रुव css_border_style_func border_style_funcs[4] = {
	[शिखरम्_2]    = css_computed_border_शिखरम्_style,
	[दक्षिणः_2]  = css_computed_border_दक्षिणः_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[वामः_2]   = css_computed_border_वामः_style,
};

/** Array of per-side access functions for computed style border colors. */
ध्रुव css_border_color_func border_color_funcs[4] = {
	[शिखरम्_2]    = css_computed_border_शिखरम्_color,
	[दक्षिणः_2]  = css_computed_border_दक्षिणः_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[वामः_2]   = css_computed_border_वामः_color,
};

/* forward declaration to break cycles */
स्थिर शून्य layout_minmax_खण्डम्(
		संरचना box *खण्डम्,
		ध्रुव संरचना gui_layout_सारणी *font_func,
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
			     पूर्णाङ्क *विस्तारः, पूर्णाङ्क *height,
			     पूर्णाङ्क min_विस्तारः, पूर्णाङ्क max_विस्तारः,
			     पूर्णाङ्क min_height, पूर्णाङ्क max_height)
{
	प्रतिज्ञा(box->object != रिक्त);
	प्रतिज्ञा(विस्तारः != रिक्त && height != रिक्त);

	यदि (*विस्तारः == AUTO && *height == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		पूर्णाङ्क intrinsic_विस्तारः = content_get_विस्तारः(box->object);
		पूर्णाङ्क intrinsic_height = content_get_height(box->object);

		/* use intrinsic dimensions */
		*विस्तारः = intrinsic_विस्तारः;
		*height = intrinsic_height;

		/* Deal with min/max-width first */
		यदि (min_विस्तारः >  0 && min_विस्तारः > *विस्तारः) {
			*विस्तारः = min_विस्तारः;
			scaled = true;
		}
		यदि (max_विस्तारः >= 0 && max_विस्तारः < *विस्तारः) {
			*विस्तारः = max_विस्तारः;
			scaled = true;
		}

		यदि (scaled && (intrinsic_विस्तारः != 0)) {
			/* Update height */
			*height = (*विस्तारः * intrinsic_height) /
					intrinsic_विस्तारः;
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
			*विस्तारः = (*height * intrinsic_विस्तारः) /
					intrinsic_height;
		}

	} अन्यथा यदि (*विस्तारः == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		पूर्णाङ्क intrinsic_विस्तारः = content_get_विस्तारः(box->object);
		पूर्णाङ्क intrinsic_height = content_get_height(box->object);

		यदि (intrinsic_height != 0)
			*विस्तारः = (*height * intrinsic_विस्तारः) /
					intrinsic_height;
		अन्यथा
			*विस्तारः = intrinsic_विस्तारः;

		यदि (min_विस्तारः >  0 && min_विस्तारः > *विस्तारः)
			*विस्तारः = min_विस्तारः;
		यदि (max_विस्तारः >= 0 && max_विस्तारः < *विस्तारः)
			*विस्तारः = max_विस्तारः;

	} अन्यथा यदि (*height == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		पूर्णाङ्क intrinsic_विस्तारः = content_get_विस्तारः(box->object);
		पूर्णाङ्क intrinsic_height = content_get_height(box->object);

		यदि (min_विस्तारः >  0 && min_विस्तारः > *विस्तारः)
			*विस्तारः = min_विस्तारः;
		यदि (max_विस्तारः >= 0 && max_विस्तारः < *विस्तारः)
			*विस्तारः = max_विस्तारः;

		यदि (intrinsic_विस्तारः != 0)
			*height = (*विस्तारः * intrinsic_height) /
					intrinsic_विस्तारः;
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
		ध्रुव css_computed_style *style, पूर्णाङ्क विस्तारः)
{
	css_fixed मानम् = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_text_indent(style, &मानम्, &unit);

	यदि (unit == CSS_UNIT_PCT) {
		प्रत्यागम FPCT_सम्बन्धि_INT_TOINT(मानम्, विस्तारः);
	} अन्यथा {
		प्रत्यागम FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				मानम्, unit));
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
स्थिर शून्य layout_minmax_सारणी(संरचना box *सारणी,
		ध्रुव संरचना gui_layout_सारणी *font_func,
		ध्रुव html_content *content)
{
	अचिह्नित पूर्णाङ्क i, j;
	पूर्णाङ्क विस्तारः;
	पूर्णाङ्क border_spacing_h = 0;
	पूर्णाङ्क सारणी_min = 0, सारणी_max = 0;
	पूर्णाङ्क extra_fixed = 0;
	भिन्न extra_frac = 0;
	संरचना column *col;
	संरचना box *row_गणः, *row, *cell;

	/* check if the widths have already been calculated */
	यदि (सारणी->max_विस्तारः != UNKNOWN_MAX_विस्तारः)
		प्रत्यागम;

	यदि (सारणी_calculate_column_types(&content->unit_len_ctx, सारणी) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		प्रत्यागम;
	}
	col = सारणी->col;

	/* start with 0 except for fixed-width columns */
	प्रति (i = 0; i != सारणी->columns; i++) {
		यदि (col[i].type == COLUMN_विस्तारः_FIXED)
			col[i].min = col[i].max = col[i].विस्तारः;
		अन्यथा
			col[i].min = col[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	यदि (css_computed_border_collapse(सारणी->style) ==
			CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		css_computed_border_spacing(सारणी->style, &h, &hu, &v, &vu);

		border_spacing_h = FIXTOINT(css_unit_len2device_px(
				सारणी->style,
				&content->unit_len_ctx,
				h, hu));
	}

	/* 1st pass: consider cells with colspan 1 only */
	प्रति (row_गणः = सारणी->children; row_गणः; row_गणः =row_गणः->अग्र)
	प्रति (row = row_गणः->children; row; row = row->अग्र)
	प्रति (cell = row->children; cell; cell = cell->अग्र) {
		प्रतिज्ञा(cell->type == BOX_सारणी_CELL);
		प्रतिज्ञा(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		प्रतिज्ञा(cell->columns != 0);

		यदि (cell->columns != 1)
			अनुवर्त;

		layout_minmax_खण्डम्(cell, font_func, content);
		i = cell->start_column;

		यदि (col[i].positioned)
			अनुवर्त;

		/* update column min, max widths using cell widths */
		यदि (col[i].min < cell->min_विस्तारः)
			col[i].min = cell->min_विस्तारः;
		यदि (col[i].max < cell->max_विस्तारः)
			col[i].max = cell->max_विस्तारः;
	}

	/* 2nd pass: cells which span multiple columns */
	प्रति (row_गणः = सारणी->children; row_गणः; row_गणः =row_गणः->अग्र)
	प्रति (row = row_गणः->children; row; row = row->अग्र)
	प्रति (cell = row->children; cell; cell = cell->अग्र) {
		अचिह्नित पूर्णाङ्क flexible_columns = 0;
		पूर्णाङ्क min = 0, max = 0, fixed_विस्तारः = 0, extra;

		यदि (cell->columns == 1)
			अनुवर्त;

		layout_minmax_खण्डम्(cell, font_func, content);
		i = cell->start_column;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		प्रति (j = 0; j != cell->columns; j++) {
			min += col[i + j].min;
			यदि (col[i + j].type == COLUMN_विस्तारः_FIXED)
				fixed_विस्तारः += col[i + j].विस्तारः;
			अन्यथा
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		यदि (min < cell->min_विस्तारः) {
			यदि (flexible_columns == 0) {
				extra = 1 + (cell->min_विस्तारः - min) /
						cell->columns;
				प्रति (j = 0; j != cell->columns; j++) {
					col[i + j].min += extra;
					यदि (col[i + j].max < col[i + j].min)
						col[i + j].max = col[i + j].min;
				}
			} अन्यथा {
				extra = 1 + (cell->min_विस्तारः - min) /
						flexible_columns;
				प्रति (j = 0; j != cell->columns; j++) {
					यदि (col[i + j].type !=
							COLUMN_विस्तारः_FIXED) {
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
		यदि (max < cell->max_विस्तारः && flexible_columns) {
			extra = 1 + (cell->max_विस्तारः - max) / flexible_columns;
			प्रति (j = 0; j != cell->columns; j++)
				यदि (col[i + j].type != COLUMN_विस्तारः_FIXED)
					col[i + j].max += extra;
		}
	}

	प्रति (i = 0; i != सारणी->columns; i++) {
		यदि (col[i].max < col[i].min) {
			box_dump(stderr, सारणी, 0, true);
			प्रतिज्ञा(0);
		}
		सारणी_min += col[i].min;
		सारणी_max += col[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	यदि (css_computed_विस्तारः_px(सारणी->style, &content->unit_len_ctx,
			-1, &विस्तारः) == CSS_विस्तारः_समुच्चयः) {
		यदि (सारणी_min < विस्तारः)
			सारणी_min = विस्तारः;
		यदि (सारणी_max < विस्तारः)
			सारणी_max = विस्तारः;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_विस्तारः(&content->unit_len_ctx,
			सारणी->style, वामः_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_विस्तारः(&content->unit_len_ctx,
			सारणी->style, दक्षिणः_2, true, true, true,
			&extra_fixed, &extra_frac);
	यदि (extra_fixed < 0)
		extra_fixed = 0;
	यदि (extra_frac < 0)
		extra_frac = 0;
	यदि (1.0 <= extra_frac)
		extra_frac = 0.9;
	सारणी->min_विस्तारः = (सारणी_min + extra_fixed) / (1.0 - extra_frac);
	सारणी->max_विस्तारः = (सारणी_max + extra_fixed) / (1.0 - extra_frac);
	सारणी->min_विस्तारः += (सारणी->columns + 1) * border_spacing_h;
	सारणी->max_विस्तारः += (सारणी->columns + 1) * border_spacing_h;

	प्रतिज्ञा(0 <= सारणी->min_विस्तारः && सारणी->min_विस्तारः <= सारणी->max_विस्तारः);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
स्थिर अन्तःस्थ bool box_has_percentage_max_विस्तारः(संरचना box *b)
{
	css_unit unit = CSS_UNIT_PX;
	गणना css_max_विस्तारः_e type;
	css_fixed मानम् = 0;

	प्रतिज्ञा(b != रिक्त);

	type = css_computed_max_विस्तारः(b->style, &मानम्, &unit);
	प्रत्यागम ((type == CSS_MAX_विस्तारः_समुच्चयः) && (unit == CSS_UNIT_PCT));
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
layout_minmax_पङ्क्तिः(संरचना box *first,
		   पूर्णाङ्क *पङ्क्तिः_min,
		   पूर्णाङ्क *पङ्क्तिः_max,
		   bool first_पङ्क्तिः,
		   bool *पङ्क्तिः_has_height,
		   ध्रुव संरचना gui_layout_सारणी *font_func,
		   ध्रुव html_content *content)
{
	पूर्णाङ्क min = 0, max = 0, विस्तारः, height, fixed;
	भिन्न frac;
	size_t i, j;
	संरचना box *b;
	संरचना box *खण्डम्;
	plot_font_style_t fstyle;
	bool no_wrap;

	प्रतिज्ञा(first->parent);
	प्रतिज्ञा(first->parent->parent);
	प्रतिज्ञा(first->parent->parent->style);

	खण्डम् = first->parent->parent;
	no_wrap = (css_computed_white_space(खण्डम्->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(खण्डम्->style) ==
			CSS_WHITE_SPACE_PRE);

	*पङ्क्तिः_has_height = false;

	/* corresponds to the pass 1 loop in layout_line() */
	प्रति (b = first; b; b = b->अग्र) {
		गणना css_विस्तारः_e wtype;
		गणना css_height_e htype;
		गणना css_box_sizing_e bs;
		css_fixed मानम् = 0;
		css_unit unit = CSS_UNIT_PX;

		प्रतिज्ञा(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		यदि (b->type == BOX_BR) {
			b = b->अग्र;
			भङ्ग;
		}

		यदि (lh__box_is_float_box(b)) {
			प्रतिज्ञा(b->children);
			यदि (b->children->type == BOX_सारणी)
				layout_minmax_सारणी(b->children, font_func,
						content);
			अन्यथा
				layout_minmax_खण्डम्(b->children, font_func,
						content);
			b->min_विस्तारः = b->children->min_विस्तारः;
			b->max_विस्तारः = b->children->max_विस्तारः;
			यदि (min < b->min_विस्तारः)
				min = b->min_विस्तारः;
			max += b->max_विस्तारः;
			अनुवर्त;
		}

		यदि (b->type == BOX_INLINE_खण्डम् || b->type == BOX_INLINE_FLEX) {
			layout_minmax_खण्डम्(b, font_func, content);
			यदि (min < b->min_विस्तारः)
				min = b->min_विस्तारः;
			max += b->max_विस्तारः;

			यदि (b->flags & HAS_HEIGHT)
				*पङ्क्तिः_has_height = true;
			अनुवर्त;
		}

		प्रतिज्ञा(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		यदि (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_विस्तारः(&content->unit_len_ctx,
					b->style, वामः_2, true, true, true,
					&fixed, &frac);
			यदि (!b->inline_अन्तः)
				calculate_mbp_विस्तारः(&content->unit_len_ctx,
						b->style, दक्षिणः_2,
						true, true, true,
						&fixed, &frac);
			यदि (0 < fixed)
				max += fixed;
			*पङ्क्तिः_has_height = true;
			/* \todo  update min width, consider fractional extra */
		} अन्यथा यदि (b->type == BOX_INLINE_अन्तः) {
			fixed = frac = 0;
			calculate_mbp_विस्तारः(&content->unit_len_ctx,
					b->inline_अन्तः->style, दक्षिणः_2,
					true, true, true,
					&fixed, &frac);
			यदि (0 < fixed)
				max += fixed;

			यदि (b->अग्र) {
				यदि (b->space == UNKNOWN_विस्तारः) {
					font_func->विस्तारः(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*पङ्क्तिः_has_height = true;
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

			यदि (b->विस्तारः == UNKNOWN_विस्तारः) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				यदि (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					पूर्णाङ्क opt_maxwidth = 0;
					संरचना form_option *o;

					प्रति (o = b->parent->parent->gadget->
							दत्त.select.वस्तूनि; o;
							o = o->अग्र) {
						पूर्णाङ्क opt_विस्तारः;
						font_func->विस्तारः(&fstyle,
								o->text,
								वाक्यदैर्घ्यम्(o->text),
								&opt_विस्तारः);

						यदि (opt_maxwidth < opt_विस्तारः)
							opt_maxwidth =opt_विस्तारः;
					}

					b->विस्तारः = opt_maxwidth;
					यदि (nsoption_bool(core_select_menu))
						b->विस्तारः += SCROLLBAR_विस्तारः;

				} अन्यथा {
					font_func->विस्तारः(&fstyle, b->text,
						b->दीर्घता, &b->विस्तारः);
					b->flags |= MEASURED;
				}
			}
			max += b->विस्तारः;
			यदि (b->अग्र) {
				यदि (b->space == UNKNOWN_विस्तारः) {
					font_func->विस्तारः(&fstyle, " ", 1,
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
				यदि (min < b->विस्तारः)
					min = b->विस्तारः;

			} अन्यथा यदि (b->parent->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				कुरु {
					प्रति (j = i; j != b->दीर्घता &&
							b->text[j] != ' '; j++)
						;
					font_func->विस्तारः(&fstyle, b->text + i,
							 j - i, &विस्तारः);
					यदि (min < विस्तारः)
						min = विस्तारः;
					i = j + 1;
				} यावत् (j != b->दीर्घता);
			}

			*पङ्क्तिः_has_height = true;

			अनुवर्त;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		प्रतिज्ञा(b->style);

		bs = css_computed_box_sizing(खण्डम्->style);

		/* calculate box width */
		wtype = css_computed_विस्तारः_px(b->style,
				&content->unit_len_ctx, -1, &विस्तारः);
		यदि (wtype == CSS_विस्तारः_समुच्चयः) {
			यदि (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_विस्तारः(&content->unit_len_ctx,
						खण्डम्->style, वामः_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_विस्तारः(&content->unit_len_ctx,
						खण्डम्->style, दक्षिणः_2,
						false, true, true,
						&fixed, &frac);
				यदि (विस्तारः < fixed) {
					विस्तारः = fixed;
				}
			}
			यदि (विस्तारः < 0)
				विस्तारः = 0;
		} अन्यथा {
			विस्तारः = AUTO;
		}

		/* height */
		htype = css_computed_height(b->style, &मानम्, &unit);
		यदि (htype == CSS_HEIGHT_समुच्चयः) {
			height = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					मानम्, unit));
		} अन्यथा {
			height = AUTO;
		}

		यदि (b->object || (b->flags & REPLACE_DIM)) {
			यदि (b->object) {
				पूर्णाङ्क अस्थायी_height = height;
				layout_get_object_dimensions(b,
						&विस्तारः, &अस्थायी_height,
						INT_MIN, अधिकतम_पूर्णाङ्कः,
						INT_MIN, अधिकतम_पूर्णाङ्कः);
			}

			fixed = frac = 0;
			यदि (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_विस्तारः(&content->unit_len_ctx,
						b->style, वामः_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_विस्तारः(&content->unit_len_ctx,
						b->style, दक्षिणः_2,
						true, false, false,
						&fixed, &frac);
			} अन्यथा {
				calculate_mbp_विस्तारः(&content->unit_len_ctx,
						b->style, वामः_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_विस्तारः(&content->unit_len_ctx,
						b->style, दक्षिणः_2,
						true, true, true,
						&fixed, &frac);
			}
			यदि (0 < विस्तारः + fixed)
				विस्तारः += fixed;
		} अन्यथा यदि (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			यदि (विस्तारः == AUTO)
				विस्तारः = 400;

			fixed = frac = 0;
			यदि (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_विस्तारः(&content->unit_len_ctx,
						b->style, वामः_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_विस्तारः(&content->unit_len_ctx,
						b->style, दक्षिणः_2,
						true, false, false,
						&fixed, &frac);
			} अन्यथा {
				calculate_mbp_विस्तारः(&content->unit_len_ctx,
						b->style, वामः_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_विस्तारः(&content->unit_len_ctx,
						b->style, दक्षिणः_2,
						true, true, true,
						&fixed, &frac);
			}

			यदि (0 < विस्तारः + fixed)
				विस्तारः += fixed;

		} अन्यथा {
			/* form control with no object */
			यदि (विस्तारः == AUTO)
				विस्तारः = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		यदि (min < विस्तारः && !box_has_percentage_max_विस्तारः(b))
			min = विस्तारः;
		यदि (विस्तारः > 0)
			max += विस्तारः;

		*पङ्क्तिः_has_height = true;
	}

	यदि (first_पङ्क्तिः) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		पूर्णाङ्क text_indent = layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, 100);
		min = (min + text_indent < 0) ? 0 : min + text_indent;
		max = (max + text_indent < 0) ? 0 : max + text_indent;
	}

	*पङ्क्तिः_min = min;
	*पङ्क्तिः_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	प्रतिज्ञा(b != first);
	प्रतिज्ञा(0 <= *पङ्क्तिः_min);
	प्रतिज्ञा(*पङ्क्तिः_min <= *पङ्क्तिः_max);
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
			       ध्रुव संरचना gui_layout_सारणी *font_func,
			       ध्रुव html_content *content)
{
	संरचना box *child;
	पूर्णाङ्क पङ्क्तिः_min = 0, पङ्क्तिः_max = 0;
	पूर्णाङ्क min = 0, max = 0;
	bool first_पङ्क्तिः = true;
	bool पङ्क्तिः_has_height;

	प्रतिज्ञा(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	यदि (inline_container->max_विस्तारः != UNKNOWN_MAX_विस्तारः)
		प्रत्यागम;

	*has_height = false;

	प्रति (child = inline_container->children; child; ) {
		child = layout_minmax_पङ्क्तिः(child, &पङ्क्तिः_min, &पङ्क्तिः_max,
				first_पङ्क्तिः, &पङ्क्तिः_has_height, font_func,
				content);
		यदि (min < पङ्क्तिः_min)
			min = पङ्क्तिः_min;
		यदि (max < पङ्क्तिः_max)
			max = पङ्क्तिः_max;
		first_पङ्क्तिः = false;
		*has_height |= पङ्क्तिः_has_height;
	}

	inline_container->min_विस्तारः = min;
	inline_container->max_विस्तारः = max;

	प्रतिज्ञा(0 <= inline_container->min_विस्तारः &&
			inline_container->min_विस्तारः <=
			inline_container->max_विस्तारः);
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
स्थिर शून्य layout_minmax_खण्डम्(
		संरचना box *खण्डम्,
		ध्रुव संरचना gui_layout_सारणी *font_func,
		ध्रुव html_content *content)
{
	संरचना box *child;
	पूर्णाङ्क min = 0, max = 0;
	पूर्णाङ्क extra_fixed = 0;
	भिन्न extra_frac = 0;
	गणना css_विस्तारः_e wtype = CSS_विस्तारः_AUTO;
	css_fixed विस्तारः = 0;
	css_unit wunit = CSS_UNIT_PX;
	गणना css_height_e htype = CSS_HEIGHT_AUTO;
	css_fixed height = 0;
	css_unit hunit = CSS_UNIT_PX;
	गणना css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool child_has_height = false;

	प्रतिज्ञा(खण्डम्->type == BOX_खण्डम् ||
			खण्डम्->type == BOX_FLEX ||
			खण्डम्->type == BOX_INLINE_FLEX ||
			खण्डम्->type == BOX_INLINE_खण्डम् ||
			खण्डम्->type == BOX_सारणी_CELL);

	/* check if the widths have already been calculated */
	यदि (खण्डम्->max_विस्तारः != UNKNOWN_MAX_विस्तारः)
		प्रत्यागम;

	यदि (खण्डम्->style != रिक्त) {
		wtype = css_computed_विस्तारः(खण्डम्->style, &विस्तारः, &wunit);
		htype = css_computed_height(खण्डम्->style, &height, &hunit);
		bs = css_computed_box_sizing(खण्डम्->style);
	}

	/* set whether the minimum width is of any interest for this box */
	यदि (((खण्डम्->parent && lh__box_is_float_box(खण्डम्->parent)) ||
			खण्डम्->type == BOX_INLINE_खण्डम् ||
			खण्डम्->type == BOX_INLINE_FLEX) &&
			wtype != CSS_विस्तारः_समुच्चयः) {
		/* box shrinks to fit; need minimum width */
		खण्डम्->flags |= NEED_MIN;
	} अन्यथा यदि (खण्डम्->type == BOX_सारणी_CELL) {
		/* box shrinks to fit; need minimum width */
		खण्डम्->flags |= NEED_MIN;
	} अन्यथा यदि (खण्डम्->parent && (खण्डम्->parent->flags & NEED_MIN) &&
			wtype != CSS_विस्तारः_समुच्चयः) {
		/* box inside shrink-to-fit context; need minimum width */
		खण्डम्->flags |= NEED_MIN;
	} अन्यथा यदि (खण्डम्->parent && (खण्डम्->parent->type == BOX_FLEX)) {
		/* box is flex item */
		खण्डम्->flags |= NEED_MIN;
	}

	यदि (खण्डम्->gadget && (खण्डम्->gadget->type == GADGET_TEXTBOX ||
			खण्डम्->gadget->type == GADGET_PASSWORD ||
			खण्डम्->gadget->type == GADGET_FILE ||
			खण्डम्->gadget->type == GADGET_TEXTAREA) &&
			खण्डम्->style && wtype == CSS_विस्तारः_AUTO) {
		css_fixed आकार = INTTOFIX(10);
		css_unit unit = CSS_UNIT_EM;

		min = max = FIXTOINT(css_unit_len2device_px(खण्डम्->style,
				&content->unit_len_ctx, आकार, unit));

		खण्डम्->flags |= HAS_HEIGHT;
	}

	यदि (खण्डम्->gadget && (खण्डम्->gadget->type == GADGET_RADIO ||
			खण्डम्->gadget->type == GADGET_CHECKBOX) &&
			खण्डम्->style && wtype == CSS_विस्तारः_AUTO) {
		css_fixed आकार = INTTOFIX(1);
		css_unit unit = CSS_UNIT_EM;

		/* form checkbox or radio button
		 * if width is AUTO, set it to 1em */
		min = max = FIXTOINT(css_unit_len2device_px(खण्डम्->style,
				&content->unit_len_ctx, आकार, unit));

		खण्डम्->flags |= HAS_HEIGHT;
	}

	यदि (खण्डम्->object) {
		यदि (content_get_type(खण्डम्->object) == CONTENT_HTML) {
			layout_minmax_खण्डम्(html_get_box_वृक्षः(खण्डम्->object),
					font_func, content);
			min = html_get_box_वृक्षः(खण्डम्->object)->min_विस्तारः;
			max = html_get_box_वृक्षः(खण्डम्->object)->max_विस्तारः;
		} अन्यथा {
			min = max = content_get_विस्तारः(खण्डम्->object);
		}

		खण्डम्->flags |= HAS_HEIGHT;
	} अन्यथा यदि (खण्डम्->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		खण्डम्->flags |= HAS_HEIGHT;
	} अन्यथा {
		/* recurse through children */
		प्रति (child = खण्डम्->children; child; child = child->अग्र) {
			विकल्प (child->type) {
			पक्ष BOX_FLEX:
			पक्ष BOX_खण्डम्:
				layout_minmax_खण्डम्(child, font_func,
						content);
				यदि (child->flags & HAS_HEIGHT)
					child_has_height = true;
				भङ्ग;
			पक्ष BOX_INLINE_CONTAINER:
				यदि (खण्डम्->flags & NEED_MIN)
					child->flags |= NEED_MIN;

				layout_minmax_inline_container(child,
						&child_has_height, font_func,
						content);
				यदि (child_has_height &&
						child ==
						child->parent->children) {
					खण्डम्->flags |= MAKE_HEIGHT;
				}
				भङ्ग;
			पक्ष BOX_सारणी:
				layout_minmax_सारणी(child, font_func,
						content);
				/* todo: fix for zero height tables */
				child_has_height = true;
				child->flags |= MAKE_HEIGHT;
				भङ्ग;
			स्वभाव:
				प्रतिज्ञा(0);
			}
			प्रतिज्ञा(child->max_विस्तारः != UNKNOWN_MAX_विस्तारः);

			यदि (child->style &&
					(css_computed_position(child->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(child->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				अनुवर्त;
			}

			यदि (lh__box_is_flex_container(खण्डम्) &&
			    lh__flex_main_is_horizontal(खण्डम्)) {
				यदि (खण्डम्->style != रिक्त &&
				    css_computed_flex_wrap(खण्डम्->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += child->min_विस्तारः;
				} अन्यथा {
					यदि (min < child->min_विस्तारः)
						min = child->min_विस्तारः;
				}
				max += child->max_विस्तारः;

			} अन्यथा {
				यदि (min < child->min_विस्तारः)
					min = child->min_विस्तारः;
				यदि (max < child->max_विस्तारः)
					max = child->max_विस्तारः;
			}

			यदि (child_has_height)
				खण्डम्->flags |= HAS_HEIGHT;
		}
	}

	यदि (max < min) {
		box_dump(stderr, खण्डम्, 0, true);
		प्रतिज्ञा(0);
	}

	/* fixed width takes priority */
	यदि (खण्डम्->type != BOX_सारणी_CELL && !lh__box_is_flex_item(खण्डम्)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		गणना css_max_विस्तारः_e max_type;
		गणना css_min_विस्तारः_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed मानम् = 0;
		पूर्णाङ्क विस्तारः;

		यदि (css_computed_विस्तारः_px(खण्डम्->style, &content->unit_len_ctx,
				-1, &विस्तारः) == CSS_विस्तारः_समुच्चयः) {
			min = max = विस्तारः;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_विस्तारः(खण्डम्->style, &मानम्, &unit);
		यदि (min_type == CSS_MIN_विस्तारः_समुच्चयः && unit != CSS_UNIT_PCT) {
			पूर्णाङ्क val = FIXTOINT(css_unit_len2device_px(खण्डम्->style,
					&content->unit_len_ctx, मानम्, unit));

			यदि (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_विस्तारः(खण्डम्->style, &मानम्, &unit);
		यदि (max_type == CSS_MAX_विस्तारः_समुच्चयः && unit != CSS_UNIT_PCT) {
			पूर्णाङ्क val = FIXTOINT(css_unit_len2device_px(खण्डम्->style,
					&content->unit_len_ctx, मानम्, unit));

			यदि (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	यदि (htype == CSS_HEIGHT_समुच्चयः && hunit != CSS_UNIT_PCT &&
			height > INTTOFIX(0)) {
		खण्डम्->flags |= MAKE_HEIGHT;
		खण्डम्->flags |= HAS_HEIGHT;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_विस्तारः(&content->unit_len_ctx, खण्डम्->style, वामः_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_विस्तारः(&content->unit_len_ctx, खण्डम्->style, दक्षिणः_2,
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

	calculate_mbp_विस्तारः(&content->unit_len_ctx, खण्डम्->style, वामः_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_विस्तारः(&content->unit_len_ctx, खण्डम्->style, दक्षिणः_2,
			true, false, false, &extra_fixed, &extra_frac);

	यदि (extra_fixed < 0)
		extra_fixed = 0;
	यदि (extra_frac < 0)
		extra_frac = 0;
	यदि (1.0 <= extra_frac)
		extra_frac = 0.9;
	यदि (खण्डम्->style != रिक्त &&
			(css_computed_float(खण्डम्->style) == CSS_FLOAT_वामः ||
			css_computed_float(खण्डम्->style) == CSS_FLOAT_दक्षिणः)) {
		/* floated boxs */
		खण्डम्->min_विस्तारः = min + extra_fixed;
		खण्डम्->max_विस्तारः = max + extra_fixed;
	} अन्यथा {
		/* not floated */
		खण्डम्->min_विस्तारः = (min + extra_fixed) / (1.0 - extra_frac);
		खण्डम्->max_विस्तारः = (max + extra_fixed) / (1.0 - extra_frac);
	}

	प्रतिज्ञा(0 <= खण्डम्->min_विस्तारः);
	प्रतिज्ञा(खण्डम्->min_विस्तारः <= खण्डम्->max_विस्तारः);
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
layout_अग्र_margin_खण्डम्(ध्रुव css_unit_ctx *unit_len_ctx,
			 संरचना box *box,
			 संरचना box *खण्डम्,
			 पूर्णाङ्क viewport_height,
			 पूर्णाङ्क *max_pos_margin,
			 पूर्णाङ्क *max_neg_margin)
{
	प्रतिज्ञा(खण्डम् != रिक्त);

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
						box->parent->विस्तारः,
						viewport_height, box,
						box->style,
						रिक्त, रिक्त, रिक्त, रिक्त,
						रिक्त, रिक्त, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				यदि (*max_pos_margin < box->margin[शिखरम्_2])
					*max_pos_margin = box->margin[शिखरम्_2];
				अन्यथा यदि (*max_neg_margin < -box->margin[शिखरम्_2])
					*max_neg_margin = -box->margin[शिखरम्_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			यदि (box->flags & MAKE_HEIGHT ||
					box->border[शिखरम्_2].विस्तारः ||
					box->padding[शिखरम्_2] ||
					(box->style &&
					css_computed_अतिप्रवाहः_y(box->style) !=
					CSS_अतिप्रवाहः_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_first_child(box))) {
				/* Collapse to this box; return it */
				प्रत्यागम box;
			}
		}


		/* Find next box */
		यदि (box->type == BOX_खण्डम् && !box->object && box->children &&
				box->style &&
				css_computed_अतिप्रवाहः_y(box->style) ==
				CSS_अतिप्रवाहः_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} अन्यथा {
			यदि (!box->अग्र) {
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
				} यावत् (box != खण्डम् && !box->अग्र);

				यदि (box == खण्डम्) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					प्रत्यागम खण्डम्;
				}
			}

			/* Apply bottom margin */
			यदि (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			अन्यथा यदि (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->अग्र;

			/* Get margins */
			यदि (box->style) {
				layout_find_dimensions(unit_len_ctx,
						box->parent->विस्तारः,
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
	प्रति (; fl; fl = fl->अग्र_float) {
		यदि ((clear == CSS_CLEAR_वामः || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_वामः)
			यदि (y < fl->y + fl->height)
				y = fl->y + fl->height;
		यदि ((clear == CSS_CLEAR_दक्षिणः || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_दक्षिणः)
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
	   संरचना box **वामः,
	   संरचना box **दक्षिणः)
{
	पूर्णाङ्क fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*वामः = *दक्षिणः = 0;
	प्रति (; fl; fl = fl->अग्र_float) {
		fy1 = fl->y + fl->height;
		यदि (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			प्रत्यागम;
		}
		fy0 = fl->y;
		यदि (y0 < fy1 && fy0 <= y1) {
			यदि (fl->type == BOX_FLOAT_वामः) {
				fx1 = fl->x + fl->विस्तारः;
				यदि (*x0 < fx1) {
					*x0 = fx1;
					*वामः = fl;
				}
			} अन्यथा {
				fx0 = fl->x;
				यदि (fx0 < *x1) {
					*x1 = fx0;
					*दक्षिणः = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *वामः, *दक्षिणः);
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
layout_solve_विस्तारः(संरचना box *box,
		   पूर्णाङ्क उपलब्धम्_विस्तारः,
		   पूर्णाङ्क विस्तारः,
		   पूर्णाङ्क lm,
		   पूर्णाङ्क rm,
		   पूर्णाङ्क max_विस्तारः,
		   पूर्णाङ्क min_विस्तारः)
{
	bool auto_विस्तारः = false;

	/* Increase specified left/right margins */
	यदि (box->margin[वामः_2] != AUTO && box->margin[वामः_2] < lm &&
			box->margin[वामः_2] >= 0)
		box->margin[वामः_2] = lm;
	यदि (box->margin[दक्षिणः_2] != AUTO && box->margin[दक्षिणः_2] < rm &&
			box->margin[दक्षिणः_2] >= 0)
		box->margin[दक्षिणः_2] = rm;

	/* Find width */
	यदि (विस्तारः == AUTO) {
		पूर्णाङ्क margin_वामः = box->margin[वामः_2];
		पूर्णाङ्क margin_दक्षिणः = box->margin[दक्षिणः_2];

		यदि (margin_वामः == AUTO) {
			margin_वामः = lm;
		}
		यदि (margin_दक्षिणः == AUTO) {
			margin_दक्षिणः = rm;
		}

		विस्तारः = उपलब्धम्_विस्तारः -
				(margin_वामः + box->border[वामः_2].विस्तारः +
				box->padding[वामः_2] + box->padding[दक्षिणः_2] +
				box->border[दक्षिणः_2].विस्तारः + margin_दक्षिणः);
		विस्तारः = विस्तारः < 0 ? 0 : विस्तारः;
		auto_विस्तारः = true;
	}

	यदि (max_विस्तारः >= 0 && विस्तारः > max_विस्तारः) {
		/* max-width is admissable and width exceeds max-width */
		विस्तारः = max_विस्तारः;
		auto_विस्तारः = false;
	}

	यदि (min_विस्तारः > 0 && विस्तारः < min_विस्तारः) {
		/* min-width is admissable and width is less than max-width */
		विस्तारः = min_विस्तारः;
		auto_विस्तारः = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	यदि (auto_विस्तारः) {
		/* any other 'auto' become 0 or the minimum required values */
		यदि (box->margin[वामः_2] == AUTO) {
			box->margin[वामः_2] = lm;
		}
		यदि (box->margin[दक्षिणः_2] == AUTO) {
			box->margin[दक्षिणः_2] = rm;
		}
		प्रत्यागम विस्तारः;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	यदि (box->margin[वामः_2] != AUTO && box->margin[दक्षिणः_2] != AUTO &&
			box->parent != रिक्त && box->parent->style != रिक्त) {
		विकल्प (css_computed_text_align(box->parent->style)) {
		पक्ष CSS_TEXT_ALIGN_LIBCSS_दक्षिणः:
			box->margin[वामः_2] = AUTO;
			box->margin[दक्षिणः_2] = 0;
			भङ्ग;
		पक्ष CSS_TEXT_ALIGN_LIBCSS_CENTER:
			box->margin[वामः_2] = box->margin[दक्षिणः_2] = AUTO;
			भङ्ग;
		पक्ष CSS_TEXT_ALIGN_LIBCSS_वामः:
			box->margin[वामः_2] = 0;
			box->margin[दक्षिणः_2] = AUTO;
			भङ्ग;
		स्वभाव:
			/* Leave it alone; no HTML alignment */
			भङ्ग;
		}
	}

	यदि (box->margin[वामः_2] == AUTO && box->margin[दक्षिणः_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[वामः_2] = box->margin[दक्षिणः_2] =
				(उपलब्धम्_विस्तारः - lm - rm -
				(box->border[वामः_2].विस्तारः + box->padding[वामः_2] +
				विस्तारः + box->padding[दक्षिणः_2] +
				box->border[दक्षिणः_2].विस्तारः)) / 2;

		यदि (box->margin[वामः_2] < 0) {
			box->margin[दक्षिणः_2] += box->margin[वामः_2];
			box->margin[वामः_2] = 0;
		}

		box->margin[वामः_2] += lm;

	} अन्यथा यदि (box->margin[वामः_2] == AUTO) {
		box->margin[वामः_2] = उपलब्धम्_विस्तारः - lm -
				(box->border[वामः_2].विस्तारः + box->padding[वामः_2] +
				विस्तारः + box->padding[दक्षिणः_2] +
				box->border[दक्षिणः_2].विस्तारः + box->margin[दक्षिणः_2]);
		box->margin[वामः_2] = box->margin[वामः_2] < lm
				? lm : box->margin[वामः_2];
	} अन्यथा {
		/* margin-right auto or "over-constrained" */
		box->margin[दक्षिणः_2] = उपलब्धम्_विस्तारः - rm -
				(box->margin[वामः_2] + box->border[वामः_2].विस्तारः +
				 box->padding[वामः_2] + विस्तारः +
				 box->padding[दक्षिणः_2] +
				 box->border[दक्षिणः_2].विस्तारः);
	}

	प्रत्यागम विस्तारः;
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
layout_खण्डम्_find_dimensions(ध्रुव css_unit_ctx *unit_len_ctx,
			     पूर्णाङ्क उपलब्धम्_विस्तारः,
			     पूर्णाङ्क viewport_height,
			     पूर्णाङ्क lm,
			     पूर्णाङ्क rm,
			     संरचना box *box)
{
	पूर्णाङ्क विस्तारः, max_विस्तारः, min_विस्तारः;
	पूर्णाङ्क height, max_height, min_height;
	पूर्णाङ्क *margin = box->margin;
	पूर्णाङ्क *padding = box->padding;
	संरचना box_border *border = box->border;
	ध्रुव css_computed_style *style = box->style;

	layout_find_dimensions(unit_len_ctx, उपलब्धम्_विस्तारः, viewport_height, box,
			style, &विस्तारः, &height, &max_विस्तारः, &min_विस्तारः,
			&max_height, &min_height, margin, padding, border);

	यदि (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &विस्तारः, &height,
				min_विस्तारः, max_विस्तारः, min_height, max_height);
	}

	box->विस्तारः = layout_solve_विस्तारः(box, उपलब्धम्_विस्तारः, विस्तारः, lm, rm,
			max_विस्तारः, min_विस्तारः);
	box->height = height;

	यदि (margin[शिखरम्_2] == AUTO)
		margin[शिखरम्_2] = 0;
	यदि (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
स्थिर शून्य layout_खण्डम्_सङ्कलय_scrollbar(संरचना box *box, पूर्णाङ्क which)
{
	गणना css_अतिप्रवाहः_e अतिप्रवाहः_x, अतिप्रवाहः_y;

	प्रतिज्ञा(box->type == BOX_खण्डम् && (which == दक्षिणः_2 || which == BOTTOM));

	यदि (box->style == रिक्त)
		प्रत्यागम;

	अतिप्रवाहः_x = css_computed_अतिप्रवाहः_x(box->style);
	अतिप्रवाहः_y = css_computed_अतिप्रवाहः_y(box->style);

	यदि (which == BOTTOM &&
			(अतिप्रवाहः_x == CSS_अतिप्रवाहः_SCROLL ||
			 अतिप्रवाहः_x == CSS_अतिप्रवाहः_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		यदि (box->height != AUTO &&
				(अतिप्रवाहः_x == CSS_अतिप्रवाहः_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_विस्तारः;
		}

	} अन्यथा यदि (which == दक्षिणः_2 &&
			(अतिप्रवाहः_y == CSS_अतिप्रवाहः_SCROLL ||
			 अतिप्रवाहः_y == CSS_अतिप्रवाहः_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		गणना css_height_e htype;
		css_fixed height = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_height(box->style, &height, &hunit);

		यदि (which == दक्षिणः_2 && box->विस्तारः != AUTO &&
				htype == CSS_HEIGHT_समुच्चयः &&
				(अतिप्रवाहः_y == CSS_अतिप्रवाहः_SCROLL ||
				box_vscrollbar_present(box))) {
			box->विस्तारः -= SCROLLBAR_विस्तारः;
			box->padding[दक्षिणः_2] += SCROLLBAR_विस्तारः;
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

	प्रति (box = box->children; box; box = box->अग्र) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_सारणी(
		संरचना box *सारणी,
		पूर्णाङ्क उपलब्धम्_विस्तारः,
		html_content *content)
{
	अचिह्नित पूर्णाङ्क columns = सारणी->columns;  /* total columns */
	अचिह्नित पूर्णाङ्क i;
	अचिह्नित पूर्णाङ्क *row_विस्तृतिः;
	पूर्णाङ्क *excess_y;
	पूर्णाङ्क सारणी_विस्तारः, min_विस्तारः = 0, max_विस्तारः = 0;
	पूर्णाङ्क required_विस्तारः = 0;
	पूर्णाङ्क x, remainder = 0, गणनम् = 0;
	पूर्णाङ्क सारणी_height = 0;
	पूर्णाङ्क min_height = 0;
	पूर्णाङ्क *xs;  /* array of column x positions */
	पूर्णाङ्क auto_विस्तारः;
	पूर्णाङ्क अतिरिक्तः_विस्तारः;
	पूर्णाङ्क relative_योग = 0;
	पूर्णाङ्क border_spacing_h = 0, border_spacing_v = 0;
	पूर्णाङ्क अतिरिक्तः_height;
	पूर्णाङ्क positioned_columns = 0;
	संरचना box *containing_खण्डम् = रिक्त;
	संरचना box *c;
	संरचना box *row;
	संरचना box *row_गणः;
	संरचना box **row_विस्तृतिः_cell;
	संरचना column *col;
	ध्रुव css_computed_style *style = सारणी->style;
	गणना css_height_e htype;
	css_fixed मानम् = 0;
	css_unit unit = CSS_UNIT_PX;

	प्रतिज्ञा(सारणी->type == BOX_सारणी);
	प्रतिज्ञा(style);
	प्रतिज्ञा(सारणी->children && सारणी->children->children);
	प्रतिज्ञा(columns);

	/* allocate working buffers */
	col = स्मृतिं_देहि(columns * परिमाण col[0]);
	excess_y = स्मृतिं_देहि(columns * परिमाण excess_y[0]);
	row_विस्तृतिः = स्मृतिं_देहि(columns * परिमाण row_विस्तृतिः[0]);
	row_विस्तृतिः_cell = स्मृतिं_देहि(columns * परिमाण row_विस्तृतिः_cell[0]);
	xs = स्मृतिं_देहि((columns + 1) * परिमाण xs[0]);
	यदि (!col || !xs || !row_विस्तृतिः || !excess_y || !row_विस्तृतिः_cell) {
		मुक्त(col);
		मुक्त(excess_y);
		मुक्त(row_विस्तृतिः);
		मुक्त(row_विस्तृतिः_cell);
		मुक्त(xs);
		प्रत्यागम false;
	}

	स्मृतिं_प्रतिलिख(col, सारणी->col, परिमाण(col[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_find_dimensions(&content->unit_len_ctx, उपलब्धम्_विस्तारः, -1, सारणी,
			style, 0, 0, 0, 0, 0, 0, सारणी->margin, सारणी->padding,
			सारणी->border);
	प्रति (row_गणः = सारणी->children; row_गणः;
			row_गणः = row_गणः->अग्र) {
		प्रति (row = row_गणः->children; row; row = row->अग्र) {
			प्रति (c = row->children; c; c = c->अग्र) {
				गणना css_अतिप्रवाहः_e अतिप्रवाहः_x;
				गणना css_अतिप्रवाहः_e अतिप्रवाहः_y;

				प्रतिज्ञा(c->style);
				सारणी_प्रयुक्तम्_border_for_cell(
						&content->unit_len_ctx, c);
				layout_find_dimensions(&content->unit_len_ctx,
						उपलब्धम्_विस्तारः, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				अतिप्रवाहः_x = css_computed_अतिप्रवाहः_x(c->style);
				अतिप्रवाहः_y = css_computed_अतिप्रवाहः_y(c->style);

				यदि (अतिप्रवाहः_x == CSS_अतिप्रवाहः_SCROLL ||
						अतिप्रवाहः_x ==
						CSS_अतिप्रवाहः_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_विस्तारः;
				}
				यदि (अतिप्रवाहः_y == CSS_अतिप्रवाहः_SCROLL ||
						अतिप्रवाहः_y ==
						CSS_अतिप्रवाहः_AUTO) {
					c->padding[दक्षिणः_2] += SCROLLBAR_विस्तारः;
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
	यदि (css_computed_विस्तारः_px(style, &content->unit_len_ctx,
			उपलब्धम्_विस्तारः, &सारणी_विस्तारः) == CSS_विस्तारः_समुच्चयः) {
		/* specified width includes border */
		सारणी_विस्तारः -= सारणी->border[वामः_2].विस्तारः +
				सारणी->border[दक्षिणः_2].विस्तारः;
		सारणी_विस्तारः = सारणी_विस्तारः < 0 ? 0 : सारणी_विस्तारः;

		auto_विस्तारः = सारणी_विस्तारः;
	} अन्यथा {
		सारणी_विस्तारः = AUTO;
		auto_विस्तारः = उपलब्धम्_विस्तारः -
				((सारणी->margin[वामः_2] == AUTO ? 0 :
						सारणी->margin[वामः_2]) +
				 सारणी->border[वामः_2].विस्तारः +
				 सारणी->padding[वामः_2] +
				 सारणी->padding[दक्षिणः_2] +
				 सारणी->border[दक्षिणः_2].विस्तारः +
				 (सारणी->margin[दक्षिणः_2] == AUTO ? 0 :
						सारणी->margin[दक्षिणः_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_height(style, &मानम्, &unit);
	यदि (htype == CSS_HEIGHT_समुच्चयः) {
		यदि (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			यदि (css_computed_position(सारणी->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				प्रतिज्ञा(सारणी->float_container);
				containing_खण्डम् = सारणी->float_container;
			} अन्यथा यदि (सारणी->float_container &&
					css_computed_position(सारणी->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(सारणी->style) ==
						CSS_FLOAT_वामः ||
					css_computed_float(सारणी->style) ==
						CSS_FLOAT_दक्षिणः)) {
				/* Table is a float */
				प्रतिज्ञा(सारणी->parent && सारणी->parent->parent &&
						सारणी->parent->parent->parent);
				containing_खण्डम् =
						सारणी->parent->parent->parent;
			} अन्यथा यदि (सारणी->parent && सारणी->parent->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_खण्डम् = सारणी->parent;
			} अन्यथा यदि (सारणी->parent && सारणी->parent->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				प्रतिज्ञा(सारणी->parent->parent);
				containing_खण्डम् = सारणी->parent->parent;
			}

			यदि (containing_खण्डम्) {
				css_fixed ignored = 0;

				htype = css_computed_height(
						containing_खण्डम्->style,
						&ignored, &unit);
			}

			यदि (containing_खण्डम् &&
					containing_खण्डम्->height != AUTO &&
					(css_computed_position(सारणी->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_HEIGHT_समुच्चयः)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_height = FPCT_सम्बन्धि_INT_TOINT(मानम्,
						containing_खण्डम्->height);
			}
		} अन्यथा {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_height = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					मानम्, unit));
		}
	}

	/* calculate width required by cells */
	प्रति (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      सारणी,
		      i,
		      ((ध्रुव वर्ण *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[col[i].type],
		      col[i].विस्तारः,
		      col[i].min,
		      col[i].max);


		यदि (col[i].positioned) {
			positioned_columns++;
			अनुवर्त;
		} अन्यथा यदि (col[i].type == COLUMN_विस्तारः_FIXED) {
			यदि (col[i].विस्तारः < col[i].min)
				col[i].विस्तारः = col[i].max = col[i].min;
			अन्यथा
				col[i].min = col[i].max = col[i].विस्तारः;
			required_विस्तारः += col[i].विस्तारः;
		} अन्यथा यदि (col[i].type == COLUMN_विस्तारः_PERCENT) {
			पूर्णाङ्क विस्तारः = col[i].विस्तारः * auto_विस्तारः / 100;
			required_विस्तारः += col[i].min < विस्तारः ? विस्तारः :
					col[i].min;
		} अन्यथा
			required_विस्तारः += col[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_विस्तारः);
	}
	required_विस्तारः += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", सारणी_विस्तारः,
	      सारणी->min_विस्तारः, सारणी->max_विस्तारः, auto_विस्तारः, required_विस्तारः);

	यदि (auto_विस्तारः < required_विस्तारः) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		प्रति (i = 0; i != columns; i++) {
			यदि (col[i].type == COLUMN_विस्तारः_RELATIVE)
				अनुवर्त;
			यदि (col[i].type == COLUMN_विस्तारः_PERCENT) {
				col[i].max = auto_विस्तारः * col[i].विस्तारः / 100;
				यदि (col[i].max < col[i].min)
					col[i].max = col[i].min;
			}
			min_विस्तारः += col[i].min;
			max_विस्तारः += col[i].max;
		}
	} अन्यथा {
		/* take percentages exactly */
		प्रति (i = 0; i != columns; i++) {
			यदि (col[i].type == COLUMN_विस्तारः_RELATIVE)
				अनुवर्त;
			यदि (col[i].type == COLUMN_विस्तारः_PERCENT) {
				पूर्णाङ्क विस्तारः = auto_विस्तारः * col[i].विस्तारः / 100;
				यदि (विस्तारः < col[i].min)
					विस्तारः = col[i].min;
				col[i].min = col[i].विस्तारः = col[i].max = विस्तारः;
				col[i].type = COLUMN_विस्तारः_FIXED;
			}
			min_विस्तारः += col[i].min;
			max_विस्तारः += col[i].max;
		}
	}

	/* allocate relative widths */
	अतिरिक्तः_विस्तारः = auto_विस्तारः;
	प्रति (i = 0; i != columns; i++) {
		यदि (col[i].type == COLUMN_विस्तारः_RELATIVE)
			relative_योग += col[i].विस्तारः;
		अन्यथा यदि (col[i].type == COLUMN_विस्तारः_FIXED)
			अतिरिक्तः_विस्तारः -= col[i].विस्तारः;
		अन्यथा
			अतिरिक्तः_विस्तारः -= col[i].min;
	}
	अतिरिक्तः_विस्तारः -= (columns + 1) * border_spacing_h;
	यदि (relative_योग != 0) {
		यदि (अतिरिक्तः_विस्तारः < 0)
			अतिरिक्तः_विस्तारः = 0;
		प्रति (i = 0; i != columns; i++) {
			यदि (col[i].type == COLUMN_विस्तारः_RELATIVE) {
				col[i].min = ceil(col[i].max =
						(भिन्न) अतिरिक्तः_विस्तारः
						* (भिन्न) col[i].विस्तारः
						/ relative_योग);
				min_विस्तारः += col[i].min;
				max_विस्तारः += col[i].max;
			}
		}
	}
	min_विस्तारः += (columns + 1) * border_spacing_h;
	max_विस्तारः += (columns + 1) * border_spacing_h;

	यदि (auto_विस्तारः <= min_विस्तारः) {
		/* not enough space: minimise column widths */
		प्रति (i = 0; i < columns; i++) {
			col[i].विस्तारः = col[i].min;
		}
		सारणी_विस्तारः = min_विस्तारः;
	} अन्यथा यदि (max_विस्तारः <= auto_विस्तारः) {
		/* more space than maximum width */
		यदि (सारणी_विस्तारः == AUTO) {
			/* for auto-width tables, make columns max width */
			प्रति (i = 0; i < columns; i++) {
				col[i].विस्तारः = col[i].max;
			}
			सारणी_विस्तारः = max_विस्तारः;
		} अन्यथा {
			/* for fixed-width tables, distribute the extra space
			 * too */
			अचिह्नित पूर्णाङ्क flexible_columns = 0;
			प्रति (i = 0; i != columns; i++)
				यदि (col[i].type != COLUMN_विस्तारः_FIXED)
					flexible_columns++;
			यदि (flexible_columns == 0) {
				पूर्णाङ्क extra = (सारणी_विस्तारः - max_विस्तारः) / columns;
				remainder = (सारणी_विस्तारः - max_विस्तारः) -
						(extra * columns);
				प्रति (i = 0; i != columns; i++) {
					col[i].विस्तारः = col[i].max + extra;
					गणनम् -= remainder;
					यदि (गणनम् < 0) {
						col[i].विस्तारः++;
						गणनम् += columns;
					}
				}

			} अन्यथा {
				पूर्णाङ्क extra = (सारणी_विस्तारः - max_विस्तारः) /
						flexible_columns;
				remainder = (सारणी_विस्तारः - max_विस्तारः) -
						(extra * flexible_columns);
				प्रति (i = 0; i != columns; i++)
					यदि (col[i].type != COLUMN_विस्तारः_FIXED) {
						col[i].विस्तारः = col[i].max +
								extra;
						गणनम् -= remainder;
						यदि (गणनम् < 0) {
							col[i].विस्तारः++;
							गणनम् += flexible_columns;
						}
					}
			}
		}
	} अन्यथा {
		/* space between min and max: fill it exactly */
		भिन्न scale = (भिन्न) (auto_विस्तारः - min_विस्तारः) /
				(भिन्न) (max_विस्तारः - min_विस्तारः);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		प्रति (i = 0; i < columns; i++) {
			col[i].विस्तारः = col[i].min + (पूर्णाङ्क) (0.5 +
					(col[i].max - col[i].min) * scale);
		}
		सारणी_विस्तारः = auto_विस्तारः;
	}

	xs[0] = x = border_spacing_h;
	प्रति (i = 0; i != columns; i++) {
		यदि (!col[i].positioned)
			x += col[i].विस्तारः + border_spacing_h;
		xs[i + 1] = x;
		row_विस्तृतिः[i] = 0;
		excess_y[i] = 0;
		row_विस्तृतिः_cell[i] = 0;
	}

	/* position cells */
	सारणी_height = border_spacing_v;
	प्रति (row_गणः = सारणी->children; row_गणः;
			row_गणः = row_गणः->अग्र) {
		पूर्णाङ्क row_गणः_height = 0;
		प्रति (row = row_गणः->children; row; row = row->अग्र) {
			पूर्णाङ्क row_height = 0;

			htype = css_computed_height(row->style, &मानम्, &unit);
			यदि (htype == CSS_HEIGHT_समुच्चयः && unit != CSS_UNIT_PCT) {
				row_height = FIXTOINT(css_unit_len2device_px(
						row->style,
						&content->unit_len_ctx,
						मानम्, unit));
			}
			प्रति (c = row->children; c; c = c->अग्र) {
				प्रतिज्ञा(c->style);
				c->विस्तारः = xs[c->start_column + c->columns] -
						xs[c->start_column] -
						border_spacing_h -
						c->border[वामः_2].विस्तारः -
						c->padding[वामः_2] -
						c->padding[दक्षिणः_2] -
						c->border[दक्षिणः_2].विस्तारः;
				c->float_children = 0;
				c->cached_place_अधः_level = 0;

				c->height = AUTO;
				यदि (!layout_खण्डम्_context(c, -1, content)) {
					मुक्त(col);
					मुक्त(excess_y);
					मुक्त(row_विस्तृतिः);
					मुक्त(row_विस्तृतिः_cell);
					मुक्त(xs);
					प्रत्यागम false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->height;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_height(c->style,
						&मानम्, &unit);

				यदि (htype == CSS_HEIGHT_समुच्चयः &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					पूर्णाङ्क h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							मानम्, unit));
					यदि (c->height < h)
						c->height = h;
				}
				/* specified row height is treated as a minimum
				 */
				यदि (c->height < row_height)
					c->height = row_height;
				c->x = xs[c->start_column] +
						c->border[वामः_2].विस्तारः;
				c->y = c->border[शिखरम्_2].विस्तारः;
				प्रति (i = 0; i != c->columns; i++) {
					row_विस्तृतिः[c->start_column + i] = c->पङ्क्तिकाः;
					excess_y[c->start_column + i] =
							c->border[शिखरम्_2].विस्तारः +
							c->padding[शिखरम्_2] +
							c->height +
							c->padding[BOTTOM] +
							c->border[BOTTOM].विस्तारः;
					row_विस्तृतिः_cell[c->start_column + i] = 0;
				}
				row_विस्तृतिः_cell[c->start_column] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[शिखरम्_2].विस्तारः -
						c->padding[शिखरम्_2] -
						c->height -
						c->border[BOTTOM].विस्तारः;
			}
			प्रति (i = 0; i != columns; i++)
				यदि (row_विस्तृतिः[i] != 0)
					row_विस्तृतिः[i]--;
				अन्यथा
					row_विस्तृतिः_cell[i] = 0;
			यदि (row->अग्र || row_गणः->अग्र) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				प्रति (i = 0; i != columns; i++)
					यदि (row_विस्तृतिः[i] == 0 && row_height <
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
				यदि (row_विस्तृतिः_cell[i] != 0)
					row_विस्तृतिः_cell[i]->padding[BOTTOM] +=
							row_height +
							border_spacing_v;
			}

			row->x = 0;
			row->y = row_गणः_height;
			row->विस्तारः = सारणी_विस्तारः;
			row->height = row_height;
			row_गणः_height += row_height + border_spacing_v;
		}
		row_गणः->x = 0;
		row_गणः->y = सारणी_height;
		row_गणः->विस्तारः = सारणी_विस्तारः;
		row_गणः->height = row_गणः_height;
		सारणी_height += row_गणः_height;
	}
	/* Table height is either the height of the contents, or specified
	 * height if greater */
	सारणी_height = max(सारणी_height, min_height);
	/** \todo distribute spare height over the row groups / rows / cells */

	/* perform vertical alignment */
	प्रति (row_गणः = सारणी->children; row_गणः;
			row_गणः = row_गणः->अग्र) {
		प्रति (row = row_गणः->children; row; row = row->अग्र) {
			प्रति (c = row->children; c; c = c->अग्र) {
				गणना css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				अतिरिक्तः_height = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->height - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &मानम्, &unit);

				विकल्प (vertical_align) {
				पक्ष CSS_VERTICAL_ALIGN_SUB:
				पक्ष CSS_VERTICAL_ALIGN_SUPER:
				पक्ष CSS_VERTICAL_ALIGN_TEXT_शिखरम्:
				पक्ष CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				पक्ष CSS_VERTICAL_ALIGN_समुच्चयः:
				पक्ष CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				पक्ष CSS_VERTICAL_ALIGN_शिखरम्:
					भङ्ग;
				पक्ष CSS_VERTICAL_ALIGN_मध्यः:
					c->padding[शिखरम्_2] += अतिरिक्तः_height / 2;
					c->padding[BOTTOM] -= अतिरिक्तः_height / 2;
					layout_move_children(c, 0,
							अतिरिक्तः_height / 2);
					भङ्ग;
				पक्ष CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[शिखरम्_2] += अतिरिक्तः_height;
					c->padding[BOTTOM] -= अतिरिक्तः_height;
					layout_move_children(c, 0,
							अतिरिक्तः_height);
					भङ्ग;
				पक्ष CSS_VERTICAL_ALIGN_INHERIT:
					प्रतिज्ञा(0);
					भङ्ग;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	यदि (सारणी->margin[शिखरम्_2] == AUTO)
		सारणी->margin[शिखरम्_2] = 0;
	यदि (सारणी->margin[BOTTOM] == AUTO)
		सारणी->margin[BOTTOM] = 0;

	मुक्त(col);
	मुक्त(excess_y);
	मुक्त(row_विस्तृतिः);
	मुक्त(row_विस्तृतिः_cell);
	मुक्त(xs);

	सारणी->विस्तारः = सारणी_विस्तारः;
	सारणी->height = सारणी_height;

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
	संरचना box *containing_खण्डम् = रिक्त;
	bool updated = false;

	/* Find containing block for percentage heights */
	यदि (box->style != रिक्त && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		प्रतिज्ञा(container);
		containing_खण्डम् = container;
	} अन्यथा यदि (box->float_container && box->style != रिक्त &&
			(css_computed_float(box->style) == CSS_FLOAT_वामः ||
			 css_computed_float(box->style) == CSS_FLOAT_दक्षिणः)) {
		/* Box is a float */
		प्रतिज्ञा(box->parent && box->parent->parent &&
				box->parent->parent->parent);
		containing_खण्डम् = box->parent->parent->parent;
	} अन्यथा यदि (box->parent && box->parent->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_खण्डम् = box->parent;
	} अन्यथा यदि (box->parent && box->parent->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		प्रतिज्ञा(box->parent->parent);
		containing_खण्डम् = box->parent->parent;
	}

	यदि (box->style) {
		गणना css_height_e htype = CSS_HEIGHT_AUTO;
		css_fixed मानम् = 0;
		css_unit unit = CSS_UNIT_PX;

		यदि (containing_खण्डम्) {
			htype = css_computed_height(containing_खण्डम्->style,
					&मानम्, &unit);
		}

		/* max-height */
		यदि (css_computed_max_height(box->style, &मानम्, &unit) ==
				CSS_MAX_HEIGHT_समुच्चयः) {
			यदि (unit == CSS_UNIT_PCT) {
				यदि (containing_खण्डम् &&
					containing_खण्डम्->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_समुच्चयः)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_सम्बन्धि_INT_TOINT(मानम्,
						containing_खण्डम्->height);
					यदि (h < box->height) {
						box->height = h;
						updated = true;
					}
				}
			} अन्यथा {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						मानम्, unit));
				यदि (h < box->height) {
					box->height = h;
					updated = true;
				}
			}
		}

		/* min-height */
		यदि (ns_computed_min_height(box->style, &मानम्, &unit) ==
				CSS_MIN_HEIGHT_समुच्चयः) {
			यदि (unit == CSS_UNIT_PCT) {
				यदि (containing_खण्डम् &&
					containing_खण्डम्->height != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_HEIGHT_समुच्चयः)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_सम्बन्धि_INT_TOINT(मानम्,
						containing_खण्डम्->height);
					यदि (h > box->height) {
						box->height = h;
						updated = true;
					}
				}
			} अन्यथा {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						मानम्, unit));
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
स्थिर bool layout_खण्डम्_object(संरचना box *खण्डम्)
{
	प्रतिज्ञा(खण्डम्);
	प्रतिज्ञा(खण्डम्->type == BOX_खण्डम् ||
			खण्डम्->type == BOX_FLEX ||
			खण्डम्->type == BOX_INLINE_खण्डम् ||
			खण्डम्->type == BOX_INLINE_FLEX ||
			खण्डम्->type == BOX_सारणी ||
			खण्डम्->type == BOX_सारणी_CELL);
	प्रतिज्ञा(खण्डम्->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", खण्डम्,
	      hlcache_handle_get_url(खण्डम्->object), खण्डम्->विस्तारः);

	यदि (content_can_reformat(खण्डम्->object)) {
		content_reformat(खण्डम्->object, false, खण्डम्->विस्तारः, 1);
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
स्थिर शून्य सङ्कलय_float_गन्तव्यम्_container(संरचना box *cont, संरचना box *b)
{
	संरचना box *box = cont->float_children;
	पूर्णाङ्क b_bottom = b->y + b->height;

	प्रतिज्ञा(b->type == BOX_FLOAT_वामः || b->type == BOX_FLOAT_दक्षिणः);

	यदि (box == रिक्त) {
		/* No other float children */
		b->अग्र_float = रिक्त;
		cont->float_children = b;
		प्रत्यागम;
	} अन्यथा यदि (b_bottom >= box->y + box->height) {
		/* Goes at start of list */
		b->अग्र_float = cont->float_children;
		cont->float_children = b;
	} अन्यथा {
		संरचना box *पूर्व = रिक्त;
		यावत् (box != रिक्त && b_bottom < box->y + box->height) {
			पूर्व = box;
			box = box->अग्र_float;
		}
		यदि (पूर्व != रिक्त) {
			b->अग्र_float = पूर्व->अग्र_float;
			पूर्व->अग्र_float = b;
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
layout_text_box_विभज(html_content *content,
		      plot_font_style_t *fstyle,
		      संरचना box *विभज_box,
		      size_t new_दीर्घता,
		      पूर्णाङ्क new_विस्तारः)
{
	पूर्णाङ्क space_विस्तारः = विभज_box->space;
	संरचना box *c2;
	ध्रुव संरचना gui_layout_सारणी *font_func = content->font_func;
	bool space = (विभज_box->text[new_दीर्घता] == ' ');
	पूर्णाङ्क प्रयुक्तम्_दीर्घता = new_दीर्घता + (space ? 1 : 0);

	यदि ((space && space_विस्तारः == 0) || space_विस्तारः == UNKNOWN_विस्तारः) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->विस्तारः(fstyle, " ", 1, &space_विस्तारः);
	}

	यदि (विभज_box->space == UNKNOWN_विस्तारः)
		विभज_box->space = space_विस्तारः;
	यदि (!space)
		space_विस्तारः = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, विभज_box, परिमाण *c2);
	यदि (!c2)
		प्रत्यागम false;
	c2->flags |= CLONE;

	/* Set remaining text in c2 */
	c2->text += प्रयुक्तम्_दीर्घता;

	/* Set c2 according to the remaining text */
	c2->विस्तारः -= new_विस्तारः + space_विस्तारः;
	c2->flags &= ~MEASURED; /* width has been estimated */
	c2->दीर्घता = विभज_box->दीर्घता - प्रयुक्तम्_दीर्घता;

	/* Update split_box for its reduced text */
	विभज_box->विस्तारः = new_विस्तारः;
	विभज_box->flags |= MEASURED;
	विभज_box->दीर्घता = new_दीर्घता;
	विभज_box->space = space_विस्तारः;

	/* Insert c2 into box list */
	c2->अग्र = विभज_box->अग्र;
	विभज_box->अग्र = c2;
	c2->पूर्व = विभज_box;
	यदि (c2->अग्र)
		c2->अग्र->पूर्व = c2;
	अन्यथा
		c2->parent->चरमः = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      विभज_box,
	      विभज_box->दीर्घता,
	      (पूर्णाङ्क)विभज_box->दीर्घता,
	      विभज_box->text);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->दीर्घता,
	      (पूर्णाङ्क)c2->दीर्घता,
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
		पूर्णाङ्क उपलब्धम्_विस्तारः,
		ध्रुव css_computed_style *style,
		संरचना box *box)
{
	पूर्णाङ्क विस्तारः, height, max_विस्तारः, min_विस्तारः, max_height, min_height;
	पूर्णाङ्क *margin = box->margin;
	पूर्णाङ्क *padding = box->padding;
	संरचना box_border *border = box->border;
	गणना css_अतिप्रवाहः_e अतिप्रवाहः_x = css_computed_अतिप्रवाहः_x(style);
	गणना css_अतिप्रवाहः_e अतिप्रवाहः_y = css_computed_अतिप्रवाहः_y(style);
	पूर्णाङ्क scrollbar_विस्तारः_x =
			(अतिप्रवाहः_x == CSS_अतिप्रवाहः_SCROLL ||
			 अतिप्रवाहः_x == CSS_अतिप्रवाहः_AUTO) ?
			SCROLLBAR_विस्तारः : 0;
	पूर्णाङ्क scrollbar_विस्तारः_y =
			(अतिप्रवाहः_y == CSS_अतिप्रवाहः_SCROLL ||
			 अतिप्रवाहः_y == CSS_अतिप्रवाहः_AUTO) ?
			SCROLLBAR_विस्तारः : 0;

	layout_find_dimensions(unit_len_ctx, उपलब्धम्_विस्तारः, -1, box, style,
			&विस्तारः, &height, &max_विस्तारः, &min_विस्तारः,
			&max_height, &min_height, margin, padding, border);

	यदि (margin[वामः_2] == AUTO)
		margin[वामः_2] = 0;
	यदि (margin[दक्षिणः_2] == AUTO)
		margin[दक्षिणः_2] = 0;

	यदि (box->gadget == रिक्त) {
		padding[दक्षिणः_2] += scrollbar_विस्तारः_y;
		padding[BOTTOM] += scrollbar_विस्तारः_x;
	}

	यदि (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &विस्तारः, &height,
				min_विस्तारः, max_विस्तारः, min_height, max_height);
	} अन्यथा यदि (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed आकार = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		प्रतिज्ञा(box->style);

		यदि (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			यदि (विस्तारः == AUTO) {
				आकार = INTTOFIX(10);
				विस्तारः = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						आकार, unit));
			}
			यदि (box->gadget->type == GADGET_FILE &&
					height == AUTO) {
				आकार = FLTTOFIX(1.5);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						आकार, unit));
			}
		}
		यदि (box->gadget->type == GADGET_TEXTAREA) {
			यदि (विस्तारः == AUTO) {
				आकार = INTTOFIX(10);
				विस्तारः = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						आकार, unit));
			}
			यदि (height == AUTO) {
				आकार = INTTOFIX(4);
				height = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						आकार, unit));
			}
		}
	} अन्यथा यदि (विस्तारः == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		विस्तारः = min(max(box->min_विस्तारः, उपलब्धम्_विस्तारः),
				box->max_विस्तारः);

		/* width includes margin, borders and padding */
		यदि (विस्तारः == उपलब्धम्_विस्तारः) {
			विस्तारः -= box->margin[वामः_2] + box->border[वामः_2].विस्तारः +
					box->padding[वामः_2] +
					box->padding[दक्षिणः_2] +
					box->border[दक्षिणः_2].विस्तारः +
					box->margin[दक्षिणः_2];
		} अन्यथा {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			पूर्णाङ्क fixed = 0;
			भिन्न frac = 0;
			calculate_mbp_विस्तारः(unit_len_ctx, box->style, वामः_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_विस्तारः(unit_len_ctx, box->style, दक्षिणः_2,
					true, true, true, &fixed, &frac);
			यदि (fixed < 0)
				fixed = 0;

			विस्तारः -= fixed;
		}

		यदि (max_विस्तारः >= 0 && विस्तारः > max_विस्तारः) विस्तारः = max_विस्तारः;
		यदि (min_विस्तारः >  0 && विस्तारः < min_विस्तारः) विस्तारः = min_विस्तारः;

	} अन्यथा {
		यदि (max_विस्तारः >= 0 && विस्तारः > max_विस्तारः) विस्तारः = max_विस्तारः;
		यदि (min_विस्तारः >  0 && विस्तारः < min_विस्तारः) विस्तारः = min_विस्तारः;
		विस्तारः -= scrollbar_विस्तारः_y;
	}

	box->विस्तारः = विस्तारः;
	box->height = height;

	यदि (margin[शिखरम्_2] == AUTO)
		margin[शिखरम्_2] = 0;
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
स्थिर bool layout_float(संरचना box *b, पूर्णाङ्क विस्तारः, html_content *content)
{
	प्रतिज्ञा(b->type == BOX_सारणी ||
	       b->type == BOX_खण्डम् ||
	       b->type == BOX_INLINE_खण्डम् ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_find_dimensions(&content->unit_len_ctx, विस्तारः, b->style, b);
	यदि (b->type == BOX_सारणी || b->type == BOX_INLINE_FLEX) {
		यदि (b->type == BOX_सारणी) {
			यदि (!layout_सारणी(b, विस्तारः, content))
				प्रत्यागम false;
		} अन्यथा {
			यदि (!layout_flex(b, विस्तारः, content))
				प्रत्यागम false;
		}
		यदि (b->margin[वामः_2] == AUTO)
			b->margin[वामः_2] = 0;
		यदि (b->margin[दक्षिणः_2] == AUTO)
			b->margin[दक्षिणः_2] = 0;
		यदि (b->margin[शिखरम्_2] == AUTO)
			b->margin[शिखरम्_2] = 0;
		यदि (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} अन्यथा {
		प्रत्यागम layout_खण्डम्_context(b, -1, content);
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
place_float_अधः(संरचना box *c, पूर्णाङ्क विस्तारः, पूर्णाङ्क cx, पूर्णाङ्क y, संरचना box *cont)
{
	पूर्णाङ्क x0, x1, yy;
	संरचना box *वामः;
	संरचना box *दक्षिणः;

	yy = y > cont->cached_place_अधः_level ?
			y : cont->cached_place_अधः_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      विस्तारः, cx, y, cont);

	कुरु {
		y = yy;
		x0 = cx;
		x1 = cx + विस्तारः;
		find_sides(cont->float_children, y, y + c->height, &x0, &x1,
				&वामः, &दक्षिणः);
		यदि (वामः != 0 && दक्षिणः != 0) {
			yy = (वामः->y + वामः->height <
					दक्षिणः->y + दक्षिणः->height ?
					वामः->y + वामः->height :
					दक्षिणः->y + दक्षिणः->height);
		} अन्यथा यदि (वामः == 0 && दक्षिणः != 0) {
			yy = दक्षिणः->y + दक्षिणः->height;
		} अन्यथा यदि (वामः != 0 && दक्षिणः == 0) {
			yy = वामः->y + वामः->height;
		}
	} यावत् ((वामः != 0 || दक्षिणः != 0) && (c->विस्तारः > x1 - x0));

	यदि (c->type == BOX_FLOAT_वामः) {
		c->x = x0;
	} अन्यथा {
		c->x = x1 - c->विस्तारः;
	}
	c->y = y;
	cont->cached_place_अधः_level = y;
}


/**
 * Calculate line height from a style.
 */
स्थिर पूर्णाङ्क पङ्क्तिः_height(
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव css_computed_style *style)
{
	गणना css_पङ्क्तिः_height_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed पङ्क्तिः_height;

	प्रतिज्ञा(style);

	lhtype = css_computed_पङ्क्तिः_height(style, &lhvalue, &lhunit);
	यदि (lhtype == CSS_पङ्क्तिः_HEIGHT_प्रसामान्यम्) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_पङ्क्तिः_HEIGHT_सङ्ख्या;
	}

	यदि (lhtype == CSS_पङ्क्तिः_HEIGHT_सङ्ख्या ||
			lhunit == CSS_UNIT_PCT) {
		पङ्क्तिः_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		यदि (lhtype != CSS_पङ्क्तिः_HEIGHT_सङ्ख्या)
			पङ्क्तिः_height = FDIV(पङ्क्तिः_height, F_100);
	} अन्यथा {
		प्रतिज्ञा(lhunit != CSS_UNIT_PCT);

		पङ्क्तिः_height = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	प्रत्यागम FIXTOINT(पङ्क्तिः_height);
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
layout_पङ्क्तिः(संरचना box *first,
	    पूर्णाङ्क *विस्तारः,
	    पूर्णाङ्क *y,
	    पूर्णाङ्क cx,
	    पूर्णाङ्क cy,
	    संरचना box *cont,
	    bool indent,
	    bool has_text_children,
	    html_content *content,
	    संरचना box **अग्र_box)
{
	पूर्णाङ्क height, प्रयुक्तम्_height;
	पूर्णाङ्क x0 = 0;
	पूर्णाङ्क x1 = *विस्तारः;
	पूर्णाङ्क x, h, x_previous;
	पूर्णाङ्क fy = cy;
	संरचना box *वामः;
	संरचना box *दक्षिणः;
	संरचना box *b;
	संरचना box *विभज_box = 0;
	संरचना box *d;
	संरचना box *br_box = 0;
	bool move_y = false;
	bool place_अधः = false;
	पूर्णाङ्क space_before = 0, space_after = 0;
	अचिह्नित पूर्णाङ्क inline_गणनम् = 0;
	अचिह्नित पूर्णाङ्क i;
	ध्रुव संरचना gui_layout_सारणी *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      first,
	      (पूर्णाङ्क)first->दीर्घता,
	      first->text,
	      *विस्तारः,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	find_sides(cont->float_children, cy, cy, &x0, &x1, &वामः, &दक्षिणः);
	x0 -= cx;
	x1 -= cx;

	यदि (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *विस्तारः);

	यदि (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	यदि (has_text_children || first->parent->parent->gadget)
		प्रयुक्तम्_height = height = पङ्क्तिः_height(&content->unit_len_ctx,
				first->parent->parent->style);
	अन्यथा
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		प्रयुक्तम्_height = height = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	प्रति (x = 0, b = first; x <= x1 - x0 && b != 0; b = b->अग्र) {
		पूर्णाङ्क min_विस्तारः, max_विस्तारः, min_height, max_height;

		प्रतिज्ञा(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		यदि (b->type == BOX_BR)
			भङ्ग;

		यदि (lh__box_is_float_box(b))
			अनुवर्त;
		यदि (b->type == BOX_INLINE_खण्डम् &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			अनुवर्त;

		प्रतिज्ञा(b->style != रिक्त);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		यदि (b->type == BOX_INLINE_खण्डम् ||
		    b->type == BOX_INLINE_FLEX) {
			यदि (b->max_विस्तारः != UNKNOWN_विस्तारः)
				यदि (!layout_float(b, *विस्तारः, content))
					प्रत्यागम false;
			h = b->border[शिखरम्_2].विस्तारः + b->padding[शिखरम्_2] + b->height +
					b->padding[BOTTOM] +
					b->border[BOTTOM].विस्तारः;
			यदि (height < h)
				height = h;
			x += b->margin[वामः_2] + b->border[वामः_2].विस्तारः +
					b->padding[वामः_2] + b->विस्तारः +
					b->padding[दक्षिणः_2] +
					b->border[दक्षिणः_2].विस्तारः +
					b->margin[दक्षिणः_2];
			space_after = 0;
			अनुवर्त;
		}

		यदि (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_find_dimensions(&content->unit_len_ctx,
					*विस्तारः, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			प्रति (i = 0; i != 4; i++)
				यदि (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[वामः_2] + b->border[वामः_2].विस्तारः +
					b->padding[वामः_2];
			यदि (b->inline_अन्तः) {
				b->inline_अन्तः->margin[दक्षिणः_2] = b->margin[दक्षिणः_2];
				b->inline_अन्तः->padding[दक्षिणः_2] =
						b->padding[दक्षिणः_2];
				b->inline_अन्तः->border[दक्षिणः_2] =
						b->border[दक्षिणः_2];
			} अन्यथा {
				x += b->padding[दक्षिणः_2] +
						b->border[दक्षिणः_2].विस्तारः +
						b->margin[दक्षिणः_2];
			}
		} अन्यथा यदि (b->type == BOX_INLINE_अन्तः) {
			b->विस्तारः = 0;
			यदि (b->space == UNKNOWN_विस्तारः) {
				font_func->विस्तारः(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[दक्षिणः_2] + b->border[दक्षिणः_2].विस्तारः +
					b->margin[दक्षिणः_2];
			अनुवर्त;
		}

		यदि (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->height = पङ्क्तिः_height(&content->unit_len_ctx,
					b->style ? b->style :
					b->parent->parent->style);
			यदि (height < b->height)
				height = b->height;

			यदि (!b->text) {
				b->विस्तारः = 0;
				space_after = 0;
				अनुवर्त;
			}

			यदि (b->विस्तारः == UNKNOWN_विस्तारः) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				यदि (b->parent->parent->gadget &&
						b->parent->parent->gadget->type
						== GADGET_SELECT) {
					पूर्णाङ्क opt_maxwidth = 0;
					संरचना form_option *o;

					प्रति (o = b->parent->parent->gadget->
							दत्त.select.वस्तूनि; o;
							o = o->अग्र) {
						पूर्णाङ्क opt_विस्तारः;
						font_func->विस्तारः(&fstyle,
								o->text,
								वाक्यदैर्घ्यम्(o->text),
								&opt_विस्तारः);

						यदि (opt_maxwidth < opt_विस्तारः)
							opt_maxwidth =opt_विस्तारः;
					}
					b->विस्तारः = opt_maxwidth;
					यदि (nsoption_bool(core_select_menu))
						b->विस्तारः += SCROLLBAR_विस्तारः;
				} अन्यथा {
					font_func->विस्तारः(&fstyle, b->text,
							b->दीर्घता, &b->विस्तारः);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			यदि (b->text && (x + b->विस्तारः < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->अग्र) {
				font_func->विस्तारः(&fstyle, b->text,
						 b->दीर्घता, &b->विस्तारः);
				b->flags |= MEASURED;
			}

			x += b->विस्तारः;
			यदि (b->space == UNKNOWN_विस्तारः) {
				font_func->विस्तारः(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			अनुवर्त;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		प्रतिज्ञा(b->style);

		layout_find_dimensions(&content->unit_len_ctx,
				*विस्तारः, -1, b, b->style,
				&b->विस्तारः, &b->height,
				&max_विस्तारः, &min_विस्तारः,
				&max_height, &min_height,
				रिक्त, रिक्त, रिक्त);

		यदि (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->विस्तारः, &b->height,
					min_विस्तारः, max_विस्तारः,
					min_height, max_height);
		} अन्यथा यदि (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			यदि (b->विस्तारः == AUTO)
				b->विस्तारः = 400;
			यदि (b->height == AUTO)
				b->height = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} अन्यथा {
			/* form control with no object */
			यदि (b->विस्तारः == AUTO)
				b->विस्तारः = FIXTOINT(css_unit_len2device_px(
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
				b->विस्तारः !=
				content_get_उपलब्धम्_विस्तारः(b->object)) {
			css_fixed मानम् = 0;
			css_unit unit = CSS_UNIT_PX;
			गणना css_height_e htype = css_computed_height(b->style,
					&मानम्, &unit);

			content_reformat(b->object, false, b->विस्तारः, b->height);

			यदि (htype == CSS_HEIGHT_AUTO)
				b->height = content_get_height(b->object);
		}

		यदि (height < b->height)
			height = b->height;

		x += b->विस्तारः;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *विस्तारः;
	find_sides(cont->float_children, cy, cy + height, &x0, &x1,
			&वामः, &दक्षिणः);
	x0 -= cx;
	x1 -= cx;

	यदि (indent)
		x0 += layout_text_indent(&content->unit_len_ctx,
				first->parent->parent->style, *विस्तारः);

	यदि (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	प्रति (x = x_previous = 0, b = first; x <= x1 - x0 && b; b = b->अग्र) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		यदि (b->type == BOX_INLINE_खण्डम् &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} अन्यथा यदि (lh__box_is_inline_flow(b)) {
			प्रतिज्ञा(b->विस्तारः != UNKNOWN_विस्तारः);

			x_previous = x;
			x += space_after;
			b->x = x;

			यदि ((b->type == BOX_INLINE && !b->inline_अन्तः) ||
					b->type == BOX_INLINE_खण्डम् ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[वामः_2] + b->border[वामः_2].विस्तारः;
				x = b->x + b->padding[वामः_2] + b->विस्तारः +
						b->padding[दक्षिणः_2] +
						b->border[दक्षिणः_2].विस्तारः +
						b->margin[दक्षिणः_2];
			} अन्यथा यदि (b->type == BOX_INLINE) {
				b->x += b->margin[वामः_2] + b->border[वामः_2].विस्तारः;
				x = b->x + b->padding[वामः_2] + b->विस्तारः;
			} अन्यथा यदि (b->type == BOX_INLINE_अन्तः) {
				b->height = b->inline_अन्तः->height;
				x += b->padding[दक्षिणः_2] +
						b->border[दक्षिणः_2].विस्तारः +
						b->margin[दक्षिणः_2];
			} अन्यथा {
				x += b->विस्तारः;
			}

			space_before = space_after;
			यदि (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			अन्यथा यदि (b->text || b->type == BOX_INLINE_अन्तः) {
				यदि (b->space == UNKNOWN_विस्तारः) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->विस्तारः(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} अन्यथा {
				space_after = 0;
			}
			विभज_box = b;
			move_y = true;
			inline_गणनम्++;
		} अन्यथा यदि (b->type == BOX_BR) {
			b->x = x;
			b->विस्तारः = 0;
			br_box = b;
			b = b->अग्र;
			विभज_box = 0;
			move_y = true;
			भङ्ग;

		} अन्यथा {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_place_अधः_level = 0;
			b->float_container = d->float_container = cont;

			यदि (!layout_float(d, *विस्तारः, content))
				प्रत्यागम false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[शिखरम्_2],
			      d->border[शिखरम्_2].विस्तारः);

			d->x = d->margin[वामः_2] + d->border[वामः_2].विस्तारः;
			d->y = d->margin[शिखरम्_2] + d->border[शिखरम्_2].विस्तारः;
			b->विस्तारः = d->margin[वामः_2] + d->border[वामः_2].विस्तारः +
					d->padding[वामः_2] + d->विस्तारः +
					d->padding[दक्षिणः_2] +
					d->border[दक्षिणः_2].विस्तारः +
					d->margin[दक्षिणः_2];
			b->height = d->margin[शिखरम्_2] + d->border[शिखरम्_2].विस्तारः +
					d->padding[शिखरम्_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].विस्तारः +
					d->margin[BOTTOM];

			यदि (b->विस्तारः > (x1 - x0) - x)
				place_अधः = true;
			यदि (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_वामः && वामः == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_दक्षिणः &&
						दक्षिणः == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						वामः == 0 && दक्षिणः == 0)) &&
					(!place_अधः ||
					(वामः == 0 && दक्षिणः == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_place_अधः_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				यदि (b->type == BOX_FLOAT_वामः) {
					b->x = cx + x0;
					यदि (b->विस्तारः > 0)
						x0 += b->विस्तारः;
					वामः = b;
				} अन्यथा {
					b->x = cx + x1 - b->विस्तारः;
					यदि (b->विस्तारः > 0)
						x1 -= b->विस्तारः;
					दक्षिणः = b;
				}
				b->y = cy;
			} अन्यथा {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				पूर्णाङ्क fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_place_अधः_level) ?
						fcy :
						cont->cached_place_अधः_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + height : fy;

				place_float_अधः(b, *विस्तारः, cx, fy, cont);
				fy = b->y;
				यदि (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_वामः && वामः != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_दक्षिणः &&
						दक्षिणः != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(वामः != 0 || दक्षिणः != 0)))) {
					/* to be cleared below existing
					 * floats */
					यदि (b->type == BOX_FLOAT_वामः)
						b->x = cx;
					अन्यथा
						b->x = cx + *विस्तारः - b->विस्तारः;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					यदि (fcy > cont->clear_level)
						cont->clear_level = fcy;
					यदि (b->y < fcy)
						b->y = fcy;
				}
				यदि (b->type == BOX_FLOAT_वामः)
					वामः = b;
				अन्यथा
					दक्षिणः = b;
			}
			सङ्कलय_float_गन्तव्यम्_container(cont, b);

			विभज_box = 0;
		}
	}

	यदि (x1 - x0 < x && विभज_box) {
		/* the last box went over the end */
		size_t विभज = 0;
		पूर्णाङ्क w;
		bool no_wrap = css_computed_white_space(
				विभज_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				विभज_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_previous;

		यदि (!no_wrap &&
		    (विभज_box->type == BOX_INLINE ||
		     विभज_box->type == BOX_TEXT) &&
		    !विभज_box->object &&
		    !(विभज_box->flags & REPLACE_DIM) &&
		    !(विभज_box->flags & IFRAME) &&
		    !विभज_box->gadget && विभज_box->text) {

			font_plot_style_from_css(&content->unit_len_ctx,
					विभज_box->style, &fstyle);
			/** \todo handle errors */
			font_func->विभज(&fstyle,
					 विभज_box->text,
					 विभज_box->दीर्घता,
					 x1 - x0 - x - space_before,
					 &विभज,
					 &w);
		}

		/* split == 0 implies that text can't be split */

		यदि (विभज == 0)
			w = विभज_box->विस्तारः;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      विभज_box,
		      (पूर्णाङ्क)विभज_box->दीर्घता,
		      विभज_box->text,
		      विभज,
		      w,
		      वामः,
		      दक्षिणः,
		      inline_गणनम्);

		यदि ((विभज == 0 || x1 - x0 <= x + space_before + w) &&
				!वामः && !दक्षिणः && inline_गणनम् == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			यदि (विभज == 0 || विभज == विभज_box->दीर्घता) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = विभज_box->अग्र;
			} अन्यथा {
				/* cut off first word for this line */
				यदि (!layout_text_box_विभज(content, &fstyle,
						विभज_box, विभज, w))
					प्रत्यागम false;
				b = विभज_box->अग्र;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} अन्यथा यदि ((विभज == 0 || x1 - x0 <= x + space_before + w) &&
				inline_गणनम् == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			प्रतिज्ञा(वामः || दक्षिणः);
			प्रयुक्तम्_height = 0;
			यदि (वामः) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      वामः->y,
				      वामः->height);

				प्रयुक्तम्_height = वामः->y + वामः->height - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      प्रयुक्तम्_height);

			}
			यदि (दक्षिणः && प्रयुक्तम्_height <
					दक्षिणः->y + दक्षिणः->height - cy + 1)
				प्रयुक्तम्_height = दक्षिणः->y + दक्षिणः->height - cy + 1;

			यदि (प्रयुक्तम्_height < 0)
				प्रयुक्तम्_height = 0;

			b = विभज_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} अन्यथा यदि (विभज == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = विभज_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} अन्यथा {
			/* fit as many words as possible */
			प्रतिज्ञा(विभज != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (पूर्णाङ्क)विभज_box->दीर्घता, विभज_box->text,
			      x1 - x0, विभज, w);

			यदि (विभज != विभज_box->दीर्घता) {
				यदि (!layout_text_box_विभज(content, &fstyle,
						विभज_box, विभज, w))
					प्रत्यागम false;
				b = विभज_box->अग्र;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	विकल्प (css_computed_text_align(first->parent->parent->style)) {
	पक्ष CSS_TEXT_ALIGN_दक्षिणः:
	पक्ष CSS_TEXT_ALIGN_LIBCSS_दक्षिणः:
		x0 = x1 - x;
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_CENTER:
	पक्ष CSS_TEXT_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_वामः:
	पक्ष CSS_TEXT_ALIGN_LIBCSS_वामः:
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

	प्रति (d = first; d != b; d = d->अग्र) {
		d->flags &= ~NEW_पङ्क्तिः;

		यदि (d->type == BOX_INLINE_खण्डम् &&
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
				d->type == BOX_INLINE_अन्तः) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[शिखरम्_2];

			यदि (d->type == BOX_TEXT && d->height > प्रयुक्तम्_height) {
				/* text */
				प्रयुक्तम्_height = d->height;
			}
		} अन्यथा यदि ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_खण्डम्) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[शिखरम्_2].विस्तारः + d->margin[शिखरम्_2];
			h = d->margin[शिखरम्_2] + d->border[शिखरम्_2].विस्तारः +
					d->padding[शिखरम्_2] + d->height +
					d->padding[BOTTOM] +
					d->border[BOTTOM].विस्तारः +
					d->margin[BOTTOM];
			यदि (प्रयुक्तम्_height < h)
				प्रयुक्तम्_height = h;
		}
	}

	first->flags |= NEW_पङ्क्तिः;

	प्रतिज्ञा(b != first || (move_y && 0 < प्रयुक्तम्_height && (वामः || दक्षिणः)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	प्रति (d = first; d != b; d = d->अग्र) {
		यदि ((d->type == BOX_INLINE && d->inline_अन्तः) ||
				d->type == BOX_BR ||
				d->type == BOX_TEXT ||
				d->type == BOX_INLINE_अन्तः) {
			css_fixed मानम् = 0;
			css_unit unit = CSS_UNIT_PX;
			विकल्प (css_computed_vertical_align(d->style, &मानम्,
					&unit)) {
			पक्ष CSS_VERTICAL_ALIGN_SUPER:
			पक्ष CSS_VERTICAL_ALIGN_शिखरम्:
			पक्ष CSS_VERTICAL_ALIGN_TEXT_शिखरम्:
				/* already at top */
				भङ्ग;
			पक्ष CSS_VERTICAL_ALIGN_SUB:
			पक्ष CSS_VERTICAL_ALIGN_BOTTOM:
			पक्ष CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
				d->y += प्रयुक्तम्_height - d->height;
				भङ्ग;
			स्वभाव:
			पक्ष CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (प्रयुक्तम्_height - d->height);
				भङ्ग;
			}
		}
	}

	/* handle clearance for br */
	यदि (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		पूर्णाङ्क clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		यदि (प्रयुक्तम्_height < clear_y - cy)
			प्रयुक्तम्_height = clear_y - cy;
	}

	यदि (move_y)
		*y += प्रयुक्तम्_height;
	*अग्र_box = b;
	*विस्तारः = x; /* return actual width */
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
स्थिर bool layout_inline_container(संरचना box *inline_container, पूर्णाङ्क विस्तारः,
		संरचना box *cont, पूर्णाङ्क cx, पूर्णाङ्क cy, html_content *content)
{
	bool first_पङ्क्तिः = true;
	bool has_text_children;
	संरचना box *c, *अग्र;
	पूर्णाङ्क y = 0;
	पूर्णाङ्क curwidth,maxwidth = विस्तारः;

	प्रतिज्ञा(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      विस्तारः,
	      cont,
	      cx,
	      cy);


	has_text_children = false;
	प्रति (c = inline_container->children; c; c = c->अग्र) {
		bool is_pre = false;

		यदि (c->style) {
			गणना css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_पङ्क्तिः ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		यदि ((lh__box_is_object(c) == false &&
				c->text && (c->दीर्घता || is_pre)) ||
				c->type == BOX_BR)
			has_text_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	प्रति (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->विस्तारः;
		यदि (!layout_पङ्क्तिः(c, &curwidth, &y, cx, cy + y, cont, first_पङ्क्तिः,
				has_text_children, content, &अग्र))
			प्रत्यागम false;
		maxwidth = max(maxwidth,curwidth);
		c = अग्र;
		first_पङ्क्तिः = false;
	}

	inline_container->विस्तारः = maxwidth;
	inline_container->height = y;

	प्रत्यागम true;
}


/* Documented in layout_intertnal.h */
bool layout_खण्डम्_context(
		संरचना box *खण्डम्,
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
	bool अन्तर्_margin = false;
	css_fixed gadget_आकार;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	प्रतिज्ञा(खण्डम्->type == BOX_खण्डम् ||
			खण्डम्->type == BOX_INLINE_खण्डम् ||
			खण्डम्->type == BOX_सारणी_CELL ||
			खण्डम्->type == BOX_FLEX ||
			खण्डम्->type == BOX_INLINE_FLEX);
	प्रतिज्ञा(खण्डम्->विस्तारः != UNKNOWN_विस्तारः);
	प्रतिज्ञा(खण्डम्->विस्तारः != AUTO);

	खण्डम्->float_children = रिक्त;
	खण्डम्->cached_place_अधः_level = 0;
	खण्डम्->clear_level = 0;

	/* special case if the block contains an object */
	यदि (खण्डम्->object) {
		पूर्णाङ्क अस्थायी_विस्तारः = खण्डम्->विस्तारः;
		यदि (!layout_खण्डम्_object(खण्डम्))
			प्रत्यागम false;
		layout_get_object_dimensions(खण्डम्, &अस्थायी_विस्तारः,
				&खण्डम्->height, INT_MIN, अधिकतम_पूर्णाङ्कः,
				INT_MIN, अधिकतम_पूर्णाङ्कः);
		प्रत्यागम true;
	} अन्यथा यदि (खण्डम्->flags & REPLACE_DIM) {
		प्रत्यागम true;
	}

	/* special case if the block contains an radio button or checkbox */
	यदि (खण्डम्->gadget && (खण्डम्->gadget->type == GADGET_RADIO ||
			खण्डम्->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_आकार = INTTOFIX(1);
		यदि (खण्डम्->height == AUTO)
			खण्डम्->height = FIXTOINT(css_unit_len2device_px(
					खण्डम्->style,
					&content->unit_len_ctx,
					gadget_आकार, gadget_unit));
	}

	box = खण्डम्->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = खण्डम्->padding[शिखरम्_2];
	यदि (box)
		box->y = खण्डम्->padding[शिखरम्_2];

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
		गणना css_अतिप्रवाहः_e अतिप्रवाहः_x = CSS_अतिप्रवाहः_VISIBLE;
		गणना css_अतिप्रवाहः_e अतिप्रवाहः_y = CSS_अतिप्रवाहः_VISIBLE;

		प्रतिज्ञा(box->type == BOX_खण्डम् ||
				box->type == BOX_FLEX ||
				box->type == BOX_सारणी ||
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
			box->x = box->parent->padding[वामः_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			गच्छ advance_गन्तव्यम्_अग्र_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		यदि (margin_collapse == रिक्त) {
			margin_collapse = layout_अग्र_margin_खण्डम्(
					&content->unit_len_ctx, box, खण्डम्,
					viewport_height,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			अन्तर्_margin = true;
		}

		/* Clearance. */
		y = 0;
		यदि (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(खण्डम्->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		यदि (box->style) {
			अतिप्रवाहः_x = css_computed_अतिप्रवाहः_x(box->style);
			अतिप्रवाहः_y = css_computed_अतिप्रवाहः_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		यदि (box->type == BOX_FLEX ||
		    box->type == BOX_खण्डम् ||
		    box->flags & IFRAME) {
			यदि (lh__box_is_object(box) == false &&
					box->style &&
					(अतिप्रवाहः_x != CSS_अतिप्रवाहः_VISIBLE ||
					 अतिप्रवाहः_y != CSS_अतिप्रवाहः_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				पूर्णाङ्क x0, x1, शिखरम्;
				संरचना box *वामः, *दक्षिणः;
				शिखरम् = cy + max_pos_margin - max_neg_margin;
				शिखरम् = (शिखरम् > y) ? शिखरम् : y;
				x0 = cx;
				x1 = cx + box->parent->विस्तारः -
						box->parent->padding[वामः_2] -
						box->parent->padding[दक्षिणः_2];
				find_sides(खण्डम्->float_children, शिखरम्, शिखरम्,
						&x0, &x1, &वामः, &दक्षिणः);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->parent->विस्तारः -
						box->parent->padding[वामः_2] -
						box->parent->padding[दक्षिणः_2] -
						x1;
			}
			layout_खण्डम्_find_dimensions(&content->unit_len_ctx,
					box->parent->विस्तारः,
					viewport_height, lm, rm, box);
			यदि (box->type == BOX_खण्डम् && !(box->flags & IFRAME)) {
				layout_खण्डम्_सङ्कलय_scrollbar(box, दक्षिणः_2);
				layout_खण्डम्_सङ्कलय_scrollbar(box, BOTTOM);
			}
		} अन्यथा यदि (box->type == BOX_सारणी) {
			यदि (box->style != रिक्त) {
				गणना css_विस्तारः_e wtype;
				css_fixed विस्तारः = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_विस्तारः(box->style, &विस्तारः,
						&unit);

				यदि (wtype == CSS_विस्तारः_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					पूर्णाङ्क x0, x1, शिखरम्;
					संरचना box *वामः, *दक्षिणः;
					शिखरम् = cy + max_pos_margin -
							max_neg_margin;
					शिखरम् = (शिखरम् > y) ? शिखरम् : y;
					x0 = cx;
					x1 = cx + box->parent->विस्तारः -
						box->parent->padding[वामः_2] -
						box->parent->padding[दक्षिणः_2];
					find_sides(खण्डम्->float_children,
						शिखरम्, शिखरम्, &x0, &x1,
						&वामः, &दक्षिणः);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->parent->विस्तारः -
						box->parent->padding[वामः_2] -
						box->parent->padding[दक्षिणः_2] -
						x1;
				}
			}
			यदि (!layout_सारणी(box, box->parent->विस्तारः - lm - rm,
					content))
				प्रत्यागम false;
			layout_solve_विस्तारः(box, box->parent->विस्तारः, box->विस्तारः,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->parent->padding[वामः_2] + box->margin[वामः_2] +
				box->border[वामः_2].विस्तारः;
		cx += box->x;

		/* Position box: vertical. */
		यदि (box->border[शिखरम्_2].विस्तारः) {
			box->y += box->border[शिखरम्_2].विस्तारः;
			cy += box->border[शिखरम्_2].विस्तारः;
		}

		/* Vertical margin */
		यदि (((box->type == BOX_खण्डम् && (box->flags & HAS_HEIGHT)) ||
		     box->type == BOX_FLEX ||
		     box->type == BOX_सारणी ||
		     (box->type == BOX_INLINE_CONTAINER &&
		      !box_is_first_child(box)) ||
		     margin_collapse == box) &&
		    अन्तर्_margin == true) {
			/* Margin goes above this box. */
			cy += max_pos_margin - max_neg_margin;
			box->y += max_pos_margin - max_neg_margin;

			/* Current margin has been applied. */
			अन्तर्_margin = false;
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
				(box->type == BOX_खण्डम् && box->style &&
				 (अतिप्रवाहः_x != CSS_अतिप्रवाहः_VISIBLE ||
				  अतिप्रवाहः_y != CSS_अतिप्रवाहः_VISIBLE))) {

			यदि (box->type == BOX_FLEX) {
				यदि (!layout_flex(box, box->विस्तारः, content)) {
					प्रत्यागम false;
				}
			} अन्यथा {
				layout_खण्डम्_context(box,
						viewport_height, content);
			}

			cy += box->padding[शिखरम्_2];

			यदि (box->height == AUTO) {
				box->height = 0;
				layout_खण्डम्_सङ्कलय_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->height + box->padding[BOTTOM] +
					box->border[BOTTOM].विस्तारः;
			y = box->y + box->padding[शिखरम्_2] + box->height +
					box->padding[BOTTOM] +
					box->border[BOTTOM].विस्तारः;

			/* Skip children, because they are done in the new
			 * block context */
			गच्छ advance_गन्तव्यम्_अग्र_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->विस्तारः);

		/* Layout (except tables). */
		यदि (box->object) {
			यदि (!layout_खण्डम्_object(box))
				प्रत्यागम false;

		} अन्यथा यदि (box->type == BOX_INLINE_CONTAINER) {
			box->विस्तारः = box->parent->विस्तारः;
			यदि (!layout_inline_container(box, box->विस्तारः, खण्डम्,
					cx, cy, content))
				प्रत्यागम false;

		} अन्यथा यदि (box->type == BOX_सारणी) {
			/* Move down to avoid floats if necessary. */
			पूर्णाङ्क x0, x1;
			संरचना box *वामः, *दक्षिणः;
			y = cy;
			यावत् (1) {
				गणना css_विस्तारः_e wtype;
				css_fixed विस्तारः = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_विस्तारः(box->style,
						&विस्तारः, &unit);

				x0 = cx;
				x1 = cx + box->parent->विस्तारः;
				find_sides(खण्डम्->float_children, y,
						y + box->height,
						&x0, &x1, &वामः, &दक्षिणः);
				यदि (wtype == CSS_विस्तारः_AUTO)
					भङ्ग;
				यदि (box->विस्तारः <= x1 - x0)
					भङ्ग;
				यदि (!वामः && !दक्षिणः)
					भङ्ग;
				अन्यथा यदि (!वामः)
					y = दक्षिणः->y + दक्षिणः->height + 1;
				अन्यथा यदि (!दक्षिणः)
					y = वामः->y + वामः->height + 1;
				अन्यथा यदि (वामः->y + वामः->height <
						दक्षिणः->y + दक्षिणः->height)
					y = वामः->y + वामः->height + 1;
				अन्यथा
					y = दक्षिणः->y + दक्षिणः->height + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		यदि (box->type == BOX_खण्डम् && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			यदि (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = रिक्त;
			}

			y = box->padding[शिखरम्_2];
			box = box->children;
			box->y = y;
			cy += y;
			अनुवर्त;
		} अन्यथा यदि (box->type == BOX_खण्डम् || box->object ||
				box->flags & IFRAME)
			cy += box->padding[शिखरम्_2];

		यदि (box->type == BOX_खण्डम् && box->height == AUTO) {
			box->height = 0;
			layout_खण्डम्_सङ्कलय_scrollbar(box, BOTTOM);
		}

		cy += box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].विस्तारः;
		cx -= box->x;
		y = box->y + box->padding[शिखरम्_2] + box->height +
				box->padding[BOTTOM] +
				box->border[BOTTOM].विस्तारः;

	advance_गन्तव्यम्_अग्र_box:
		यदि (!box->अग्र) {
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
				यदि (box == खण्डम्)
					भङ्ग;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				यदि (box->type == BOX_खण्डम् &&
						box->flags & MAKE_HEIGHT) {
					margin_collapse = रिक्त;
					अन्तर्_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				यदि (box->height == AUTO) {
					box->height = y - box->padding[शिखरम्_2];

					यदि (box->type == BOX_खण्डम्)
						layout_खण्डम्_सङ्कलय_scrollbar(box,
								BOTTOM);
				} अन्यथा
					cy += box->height -
							(y - box->padding[शिखरम्_2]);

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
							(y - box->padding[शिखरम्_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].विस्तारः;
				cx -= box->x;
				y = box->y + box->padding[शिखरम्_2] + box->height +
						box->padding[BOTTOM] +
						box->border[BOTTOM].विस्तारः;

			} यावत् (box->अग्र == रिक्त);
			यदि (box == खण्डम्)
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

		box = box->अग्र;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	प्रति (box = खण्डम्->float_children; box; box = box->अग्र_float) {
		y = box->y + box->height + box->padding[BOTTOM] +
				box->border[BOTTOM].विस्तारः + box->margin[BOTTOM];
		यदि (cy < y)
			cy = y;
	}

	यदि (खण्डम्->height == AUTO) {
		खण्डम्->height = cy - खण्डम्->padding[शिखरम्_2];
		यदि (खण्डम्->type == BOX_खण्डम्)
			layout_खण्डम्_सङ्कलय_scrollbar(खण्डम्, BOTTOM);
	}

	यदि (खण्डम्->style && css_computed_position(खण्डम्->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_height(&content->unit_len_ctx, खण्डम्, रिक्त);
	}

	यदि (खण्डम्->gadget &&
			(खण्डम्->gadget->type == GADGET_TEXTAREA ||
			खण्डम्->gadget->type == GADGET_PASSWORD ||
			खण्डम्->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		पूर्णाङ्क ta_विस्तारः = खण्डम्->padding[वामः_2] + खण्डम्->विस्तारः +
				खण्डम्->padding[दक्षिणः_2];
		पूर्णाङ्क ta_height = खण्डम्->padding[शिखरम्_2] + खण्डम्->height +
				खण्डम्->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				खण्डम्->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_समुच्चयः_layout(खण्डम्->gadget->दत्त.text.ta,
				&fstyle, ta_विस्तारः, ta_height,
				खण्डम्->padding[शिखरम्_2], खण्डम्->padding[दक्षिणः_2],
				खण्डम्->padding[BOTTOM], खण्डम्->padding[वामः_2]);
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
layout_get_element_चिह्नकम्(
		ध्रुव dom_ग्रन्थिः *ग्रन्थिः,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_ग्रन्थिः_type ग्रन्थिः_type;
	dom_exception exc;

	exc = dom_ग्रन्थिः_get_ग्रन्थिः_type(ग्रन्थिः, &ग्रन्थिः_type);
	यदि (exc != DOM_NO_ERR ||
	    ग्रन्थिः_type != DOM_ELEMENT_ग्रन्थिः) {
		प्रत्यागम false;
	}

	exc = dom_html_element_get_चिह्नकम्_type(ग्रन्थिः, &element_type);
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
		ध्रुव dom_ग्रन्थिः *ग्रन्थिः,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	यदि (!layout_get_element_चिह्नकम्(ग्रन्थिः, &element_type)) {
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
layout_get_li_मानम्(dom_ग्रन्थिः *li_ग्रन्थिः, dom_long *मानम्_बहिर्गमनम्)
{
	dom_exception exc;
	dom_long मानम्;
	bool has_मानम्;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_ग्रन्थिः,
			corestring_dom_मानम्,
			&has_मानम्);
	यदि (exc != DOM_NO_ERR || has_मानम् == false) {
		प्रत्यागम false;
	}

	exc = dom_html_li_element_get_मानम्(
			(dom_html_li_element *)li_ग्रन्थिः,
			&मानम्);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम false;
	}

	*मानम्_बहिर्गमनम् = मानम्;
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
layout__get_ol_start(dom_ग्रन्थिः *ol_ग्रन्थिः, dom_long *start_बहिर्गमनम्)
{
	dom_exception exc;
	dom_long start;
	bool has_start;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_ग्रन्थिः,
			corestring_dom_start,
			&has_start);
	यदि (exc != DOM_NO_ERR || has_start == false) {
		प्रत्यागम false;
	}

	exc = dom_html_olist_element_get_start(
			(dom_html_olist_element *)ol_ग्रन्थिः,
			&start);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम false;
	}

	*start_बहिर्गमनम् = start;
	प्रत्यागम true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
स्थिर bool
layout__get_ol_reversed(dom_ग्रन्थिः *ol_ग्रन्थिः)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_ग्रन्थिः,
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
layout_get_list_item_गणनम्(
		dom_ग्रन्थिः *list_owner, dom_long *गणनम्_बहिर्गमनम्)
{
	dom_html_element_type चिह्नकम्_type;
	dom_exception exc;
	dom_ग्रन्थिः *child;
	पूर्णाङ्क गणनम्;

	यदि (list_owner == रिक्त) {
		प्रत्यागम false;
	}

	यदि (!layout_get_element_चिह्नकम्(list_owner, &चिह्नकम्_type)) {
		प्रत्यागम false;
	}

	यदि (चिह्नकम्_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    चिह्नकम्_type != DOM_HTML_ELEMENT_TYPE_UL) {
		प्रत्यागम false;
	}

	exc = dom_ग्रन्थिः_get_first_child(list_owner, &child);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम false;
	}

	गणनम् = 0;
	यावत् (child != रिक्त) {
		dom_ग्रन्थिः *अस्थायी_ग्रन्थिः;

		यदि (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			संरचना box *child_box;
			यदि (dom_ग्रन्थिः_get_user_दत्त(child,
					corestring_dom_ns_कुञ्चिका_box_ग्रन्थिः_दत्त,
					&child_box) != DOM_NO_ERR) {
				dom_ग्रन्थिः_unref(child);
				प्रत्यागम false;
			}

			यदि (child_box != रिक्त &&
			    child_box->list_marker != रिक्त) {
				गणनम्++;
			}
		}

		exc = dom_ग्रन्थिः_get_अग्र_sibling(child, &अस्थायी_ग्रन्थिः);
		dom_ग्रन्थिः_unref(child);
		यदि (exc != DOM_NO_ERR) {
			प्रत्यागम false;
		}

		child = अस्थायी_ग्रन्थिः;
	}

	*गणनम्_बहिर्गमनम् = गणनम्;
	प्रत्यागम true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
स्थिर शून्य
layout_ordered_list_गणनम्(
		संरचना box *box)
{
	dom_html_element_type चिह्नकम्_type;
	dom_exception exc;
	dom_ग्रन्थिः *child;
	पूर्णाङ्क चरणः = 1;
	dom_long अग्र;

	यदि (box->ग्रन्थिः == रिक्त) {
		प्रत्यागम;
	}

	यदि (!layout_get_element_चिह्नकम्(box->ग्रन्थिः, &चिह्नकम्_type)) {
		प्रत्यागम;
	}

	यदि (चिह्नकम्_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    चिह्नकम्_type != DOM_HTML_ELEMENT_TYPE_UL) {
		प्रत्यागम;
	}

	अग्र = 1;
	यदि (चिह्नकम्_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_start = layout__get_ol_start(box->ग्रन्थिः, &अग्र);
		bool have_reversed = layout__get_ol_reversed(box->ग्रन्थिः);

		यदि (have_reversed) {
			चरणः = -1;
		}

		यदि (!have_start && have_reversed) {
			layout_get_list_item_गणनम्(box->ग्रन्थिः, &अग्र);
		}
	}

	exc = dom_ग्रन्थिः_get_first_child(box->ग्रन्थिः, &child);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम;
	}

	यावत् (child != रिक्त) {
		dom_ग्रन्थिः *अस्थायी_ग्रन्थिः;

		यदि (layout__check_element_type(child,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			संरचना box *child_box;

			यदि (dom_ग्रन्थिः_get_user_दत्त(child,
					corestring_dom_ns_कुञ्चिका_box_ग्रन्थिः_दत्त,
					&child_box) != DOM_NO_ERR) {
				dom_ग्रन्थिः_unref(child);
				प्रत्यागम;
			}

			यदि (child_box != रिक्त &&
			    child_box->list_marker != रिक्त) {
				dom_long मानम्;
				संरचना box *marker = child_box->list_marker;
				यदि (layout_get_li_मानम्(child, &मानम्)) {
					marker->list_मानम् = मानम्;
					अग्र = marker->list_मानम्;
				} अन्यथा {
					marker->list_मानम् = अग्र;
				}
				अग्र += चरणः;
			}
		}

		exc = dom_ग्रन्थिः_get_अग्र_sibling(child, &अस्थायी_ग्रन्थिः);
		dom_ग्रन्थिः_unref(child);
		यदि (exc != DOM_NO_ERR) {
			प्रत्यागम;
		}

		child = अस्थायी_ग्रन्थिः;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
स्थिर शून्य
layout_समुच्चयः_numerical_marker_text(
		ध्रुव html_content *content,
		संरचना box *box)
{
	संरचना box *marker = box->list_marker;
	size_t गणक_len;
	css_error css_res;
	गणना {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_आकार = 20,
	};

	marker->text = talloc_array(content->bctx, वर्ण, LIST_MARKER_आकार);
	यदि (marker->text == रिक्त) {
		प्रत्यागम;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_मानम्,
			marker->text, LIST_MARKER_आकार, &गणक_len);
	यदि (css_res == CSS_OK) {
		यदि (गणक_len > LIST_MARKER_आकार) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->text = talloc_realloc(content->bctx,
					marker->text,
					वर्ण,
					गणक_len);
			यदि (marker->text == रिक्त) {
				प्रत्यागम;
			}
			css_computed_format_list_style(box->style,
					marker->list_मानम्, marker->text,
					गणक_len, &गणक_len);
		}
		marker->दीर्घता = गणक_len;
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
	पक्ष CSS_LIST_STYLE_TYPE_वर्गफलम्: /* Fall through. */
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

	layout_ordered_list_गणनम्(box);

	प्रति (child = box->children; child; child = child->अग्र) {
		यदि (child->list_marker) {
			संरचना box *marker = child->list_marker;

			यदि (layout__list_item_is_numerical(child)) {
				यदि (marker->text == रिक्त) {
					layout_समुच्चयः_numerical_marker_text(
							content, child);
				}
			}
			यदि (marker->object) {
				marker->विस्तारः =
					content_get_विस्तारः(marker->object);
				marker->x = -marker->विस्तारः;
				marker->height =
					content_get_height(marker->object);
				marker->y = (पङ्क्तिः_height(
						&content->unit_len_ctx,
						marker->style) -
						marker->height) / 2;
			} अन्यथा यदि (marker->text) {
				यदि (marker->विस्तारः == UNKNOWN_विस्तारः) {
					plot_font_style_t fstyle;
					font_plot_style_from_css(
							&content->unit_len_ctx,
							marker->style,
							&fstyle);
					content->font_func->विस्तारः(&fstyle,
							marker->text,
							marker->दीर्घता,
							&marker->विस्तारः);
					marker->flags |= MEASURED;
				}
				marker->x = -marker->विस्तारः;
				marker->y = 0;
				marker->height = पङ्क्तिः_height(
						&content->unit_len_ctx,
						marker->style);
			} अन्यथा {
				marker->x = 0;
				marker->y = 0;
				marker->विस्तारः = 0;
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
		       संरचना box *containing_खण्डम्,
		       पूर्णाङ्क *शिखरम्,
		       पूर्णाङ्क *दक्षिणः,
		       पूर्णाङ्क *bottom,
		       पूर्णाङ्क *वामः)
{
	uint32_t type;
	css_fixed मानम् = 0;
	css_unit unit = CSS_UNIT_PX;

	प्रतिज्ञा(containing_खण्डम्->विस्तारः != UNKNOWN_विस्तारः);
	प्रतिज्ञा(containing_खण्डम्->विस्तारः != AUTO);
	प्रतिज्ञा(containing_खण्डम्->height != AUTO);

	/*वामः*/
	type = css_computed_वामः(box->style, &मानम्, &unit);
	यदि (type == CSS_वामः_समुच्चयः) {
		यदि (unit == CSS_UNIT_PCT) {
			*वामः = FPCT_सम्बन्धि_INT_TOINT(मानम्,
					containing_खण्डम्->विस्तारः);
		} अन्यथा {
			*वामः = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					मानम्, unit));
		}
	} अन्यथा {
		*वामः = AUTO;
	}

	/*दक्षिणः*/
	type = css_computed_दक्षिणः(box->style, &मानम्, &unit);
	यदि (type == CSS_दक्षिणः_समुच्चयः) {
		यदि (unit == CSS_UNIT_PCT) {
			*दक्षिणः = FPCT_सम्बन्धि_INT_TOINT(मानम्,
					containing_खण्डम्->विस्तारः);
		} अन्यथा {
			*दक्षिणः = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					मानम्, unit));
		}
	} अन्यथा {
		*दक्षिणः = AUTO;
	}

	/*शिखरम्*/
	type = css_computed_शिखरम्(box->style, &मानम्, &unit);
	यदि (type == CSS_शिखरम्_समुच्चयः) {
		यदि (unit == CSS_UNIT_PCT) {
			*शिखरम् = FPCT_सम्बन्धि_INT_TOINT(मानम्,
					containing_खण्डम्->height);
		} अन्यथा {
			*शिखरम् = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					मानम्, unit));
		}
	} अन्यथा {
		*शिखरम् = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &मानम्, &unit);
	यदि (type == CSS_BOTTOM_समुच्चयः) {
		यदि (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_सम्बन्धि_INT_TOINT(मानम्,
					containing_खण्डम्->height);
		} अन्यथा {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					मानम्, unit));
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
		संरचना box *containing_खण्डम्,
		पूर्णाङ्क cx, पूर्णाङ्क cy,
		html_content *content)
{
	पूर्णाङ्क static_वामः, static_शिखरम्;  /* static position */
	पूर्णाङ्क शिखरम्, दक्षिणः, bottom, वामः;
	पूर्णाङ्क विस्तारः, height, max_विस्तारः, min_विस्तारः;
	पूर्णाङ्क *margin = box->margin;
	पूर्णाङ्क *padding = box->padding;
	संरचना box_border *border = box->border;
	पूर्णाङ्क उपलब्धम्_विस्तारः = containing_खण्डम्->विस्तारः;
	पूर्णाङ्क space;

	प्रतिज्ञा(box->type == BOX_खण्डम् || box->type == BOX_सारणी ||
			box->type == BOX_INLINE_खण्डम् ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_वामः = cx + box->x;
	static_शिखरम् = cy + box->y;

	यदि (containing_खण्डम्->type == BOX_खण्डम् ||
			containing_खण्डम्->type == BOX_INLINE_खण्डम् ||
			containing_खण्डम्->type == BOX_सारणी_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_खण्डम्->विस्तारः += containing_खण्डम्->padding[वामः_2] +
				containing_खण्डम्->padding[दक्षिणः_2];
		containing_खण्डम्->height += containing_खण्डम्->padding[शिखरम्_2] +
				containing_खण्डम्->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_खण्डम्,
			&शिखरम्, &दक्षिणः, &bottom, &वामः);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_खण्डम्;
	layout_find_dimensions(&content->unit_len_ctx, उपलब्धम्_विस्तारः, -1,
			box, box->style, &विस्तारः, &height,
			&max_विस्तारः, &min_विस्तारः, 0, 0,
			margin, padding, border);
	box->float_container = रिक्त;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      वामः, margin[वामः_2], border[वामः_2].विस्तारः, padding[वामः_2], विस्तारः,
	      padding[दक्षिणः_2], border[दक्षिणः_2].विस्तारः, margin[दक्षिणः_2], दक्षिणः,
	      containing_खण्डम्->विस्तारः);


	यदि (वामः == AUTO && विस्तारः == AUTO && दक्षिणः == AUTO) {
		यदि (margin[वामः_2] == AUTO)
			margin[वामः_2] = 0;
		यदि (margin[दक्षिणः_2] == AUTO)
			margin[दक्षिणः_2] = 0;
		वामः = static_वामः;

		विस्तारः = min(max(box->min_विस्तारः, उपलब्धम्_विस्तारः),
			box->max_विस्तारः);
		विस्तारः -= box->margin[वामः_2] + box->border[वामः_2].विस्तारः +
			box->padding[वामः_2] + box->padding[दक्षिणः_2] +
			box->border[दक्षिणः_2].विस्तारः + box->margin[दक्षिणः_2];

		/* Adjust for {min|max}-width */
		यदि (max_विस्तारः >= 0 && विस्तारः > max_विस्तारः) विस्तारः = max_विस्तारः;
		यदि (विस्तारः < min_विस्तारः) विस्तारः = min_विस्तारः;

		दक्षिणः = containing_खण्डम्->विस्तारः -
			वामः -
			margin[वामः_2] - border[वामः_2].विस्तारः - padding[वामः_2] -
			विस्तारः -
			padding[दक्षिणः_2] - border[दक्षिणः_2].विस्तारः - margin[दक्षिणः_2];
	} अन्यथा यदि (वामः != AUTO && विस्तारः != AUTO && दक्षिणः != AUTO) {

		/* Adjust for {min|max}-width */
		यदि (max_विस्तारः >= 0 && विस्तारः > max_विस्तारः) विस्तारः = max_विस्तारः;
		यदि (min_विस्तारः >  0 && विस्तारः < min_विस्तारः) विस्तारः = min_विस्तारः;

		यदि (margin[वामः_2] == AUTO && margin[दक्षिणः_2] == AUTO) {
			space = containing_खण्डम्->विस्तारः -
					वामः - border[वामः_2].विस्तारः -
					padding[वामः_2] - विस्तारः - padding[दक्षिणः_2] -
					border[दक्षिणः_2].विस्तारः - दक्षिणः;
			यदि (space < 0) {
				margin[वामः_2] = 0;
				margin[दक्षिणः_2] = space;
			} अन्यथा {
				margin[वामः_2] = margin[दक्षिणः_2] = space / 2;
			}
		} अन्यथा यदि (margin[वामः_2] == AUTO) {
			margin[वामः_2] = containing_खण्डम्->विस्तारः -
					वामः - border[वामः_2].विस्तारः -
					padding[वामः_2] - विस्तारः - padding[दक्षिणः_2] -
					border[दक्षिणः_2].विस्तारः - margin[दक्षिणः_2] -
					दक्षिणः;
		} अन्यथा यदि (margin[दक्षिणः_2] == AUTO) {
			margin[दक्षिणः_2] = containing_खण्डम्->विस्तारः -
					वामः - margin[वामः_2] -
					border[वामः_2].विस्तारः -
					padding[वामः_2] - विस्तारः - padding[दक्षिणः_2] -
					border[दक्षिणः_2].विस्तारः - दक्षिणः;
		} अन्यथा {
			दक्षिणः = containing_खण्डम्->विस्तारः -
					वामः - margin[वामः_2] -
					border[वामः_2].विस्तारः -
					padding[वामः_2] - विस्तारः - padding[दक्षिणः_2] -
					border[दक्षिणः_2].विस्तारः - margin[दक्षिणः_2];
		}
	} अन्यथा {
		यदि (margin[वामः_2] == AUTO)
			margin[वामः_2] = 0;
		यदि (margin[दक्षिणः_2] == AUTO)
			margin[दक्षिणः_2] = 0;

		यदि (वामः == AUTO && विस्तारः == AUTO && दक्षिणः != AUTO) {
			उपलब्धम्_विस्तारः -= दक्षिणः;

			विस्तारः = min(max(box->min_विस्तारः, उपलब्धम्_विस्तारः),
				box->max_विस्तारः);
			विस्तारः -= box->margin[वामः_2] + box->border[वामः_2].विस्तारः +
				box->padding[वामः_2] + box->padding[दक्षिणः_2] +
				box->border[दक्षिणः_2].विस्तारः + box->margin[दक्षिणः_2];

			/* Adjust for {min|max}-width */
			यदि (max_विस्तारः >= 0 && विस्तारः > max_विस्तारः)
				विस्तारः = max_विस्तारः;
			यदि (विस्तारः < min_विस्तारः)
				विस्तारः = min_विस्तारः;

			वामः = containing_खण्डम्->विस्तारः -
					margin[वामः_2] - border[वामः_2].विस्तारः -
					padding[वामः_2] - विस्तारः - padding[दक्षिणः_2] -
					border[दक्षिणः_2].विस्तारः - margin[दक्षिणः_2] -
					दक्षिणः;
		} अन्यथा यदि (वामः == AUTO && विस्तारः != AUTO && दक्षिणः == AUTO) {

			/* Adjust for {min|max}-width */
			यदि (max_विस्तारः >= 0 && विस्तारः > max_विस्तारः)
				विस्तारः = max_विस्तारः;
			यदि (min_विस्तारः >  0 && विस्तारः < min_विस्तारः)
				विस्तारः = min_विस्तारः;

			वामः = static_वामः;
			दक्षिणः = containing_खण्डम्->विस्तारः -
					वामः - margin[वामः_2] -
					border[वामः_2].विस्तारः -
					padding[वामः_2] - विस्तारः - padding[दक्षिणः_2] -
					border[दक्षिणः_2].विस्तारः - margin[दक्षिणः_2];
		} अन्यथा यदि (वामः != AUTO && विस्तारः == AUTO && दक्षिणः == AUTO) {
			उपलब्धम्_विस्तारः -= वामः;

			विस्तारः = min(max(box->min_विस्तारः, उपलब्धम्_विस्तारः),
				box->max_विस्तारः);
			विस्तारः -= box->margin[वामः_2] + box->border[वामः_2].विस्तारः +
				box->padding[वामः_2] + box->padding[दक्षिणः_2] +
				box->border[दक्षिणः_2].विस्तारः + box->margin[दक्षिणः_2];

			/* Adjust for {min|max}-width */
			यदि (max_विस्तारः >= 0 && विस्तारः > max_विस्तारः)
				विस्तारः = max_विस्तारः;
			यदि (विस्तारः < min_विस्तारः)
				विस्तारः = min_विस्तारः;

			दक्षिणः = containing_खण्डम्->विस्तारः -
					वामः - margin[वामः_2] -
					border[वामः_2].विस्तारः -
					padding[वामः_2] - विस्तारः - padding[दक्षिणः_2] -
					border[दक्षिणः_2].विस्तारः - margin[दक्षिणः_2];
		} अन्यथा यदि (वामः == AUTO && विस्तारः != AUTO && दक्षिणः != AUTO) {

			/* Adjust for {min|max}-width */
			यदि (max_विस्तारः >= 0 && विस्तारः > max_विस्तारः)
				विस्तारः = max_विस्तारः;
			यदि (विस्तारः < min_विस्तारः)
				विस्तारः = min_विस्तारः;

			वामः = containing_खण्डम्->विस्तारः -
					margin[वामः_2] - border[वामः_2].विस्तारः -
					padding[वामः_2] - विस्तारः - padding[दक्षिणः_2] -
					border[दक्षिणः_2].विस्तारः - margin[दक्षिणः_2] -
					दक्षिणः;
		} अन्यथा यदि (वामः != AUTO && विस्तारः == AUTO && दक्षिणः != AUTO) {
			विस्तारः = containing_खण्डम्->विस्तारः -
					वामः - margin[वामः_2] -
					border[वामः_2].विस्तारः -
					padding[वामः_2] - padding[दक्षिणः_2] -
					border[दक्षिणः_2].विस्तारः - margin[दक्षिणः_2] -
					दक्षिणः;

			/* Adjust for {min|max}-width */
			यदि (max_विस्तारः >= 0 && विस्तारः > max_विस्तारः)
				विस्तारः = max_विस्तारः;
			यदि (विस्तारः < min_विस्तारः)
				विस्तारः = min_विस्तारः;

		} अन्यथा यदि (वामः != AUTO && विस्तारः != AUTO && दक्षिणः == AUTO) {

			/* Adjust for {min|max}-width */
			यदि (max_विस्तारः >= 0 && विस्तारः > max_विस्तारः)
				विस्तारः = max_विस्तारः;
			यदि (विस्तारः < min_विस्तारः)
				विस्तारः = min_विस्तारः;

			दक्षिणः = containing_खण्डम्->विस्तारः -
					वामः - margin[वामः_2] -
					border[वामः_2].विस्तारः -
					padding[वामः_2] - विस्तारः - padding[दक्षिणः_2] -
					border[दक्षिणः_2].विस्तारः - margin[दक्षिणः_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      वामः, margin[वामः_2], border[वामः_2].विस्तारः, padding[वामः_2], विस्तारः,
	      padding[दक्षिणः_2], border[दक्षिणः_2].विस्तारः, margin[दक्षिणः_2], दक्षिणः,
	      containing_खण्डम्->विस्तारः);

	box->x = वामः + margin[वामः_2] + border[वामः_2].विस्तारः - cx;
	यदि (containing_खण्डम्->type == BOX_खण्डम् ||
			containing_खण्डम्->type == BOX_INLINE_खण्डम् ||
			containing_खण्डम्->type == BOX_सारणी_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_खण्डम्->विस्तारः -= containing_खण्डम्->padding[वामः_2] +
				containing_खण्डम्->padding[दक्षिणः_2];
	} अन्यथा {
		/** \todo inline ancestors */
	}
	box->विस्तारः = विस्तारः;
	box->height = height;

	यदि (box->type == BOX_खण्डम् || box->type == BOX_INLINE_खण्डम् ||
			box->object || box->flags & IFRAME) {
		यदि (!layout_खण्डम्_context(box, -1, content))
			प्रत्यागम false;
	} अन्यथा यदि (box->type == BOX_सारणी) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_खण्डम्;
		/* \todo  layout_table considers margins etc. again */
		यदि (!layout_सारणी(box, विस्तारः, content))
			प्रत्यागम false;
		box->float_container = रिक्त;
		layout_solve_विस्तारः(box, box->parent->विस्तारः, box->विस्तारः, 0, 0,
				-1, -1);
	} अन्यथा यदि (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_खण्डम्;
		यदि (!layout_flex(box, विस्तारः, content))
			प्रत्यागम false;
		box->float_container = रिक्त;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      शिखरम्, margin[शिखरम्_2], border[शिखरम्_2].विस्तारः, padding[शिखरम्_2], height,
	      padding[BOTTOM], border[BOTTOM].विस्तारः, margin[BOTTOM], bottom,
	      containing_खण्डम्->height);

	यदि (शिखरम् == AUTO && height == AUTO && bottom == AUTO) {
		शिखरम् = static_शिखरम्;
		height = box->height;
		यदि (margin[शिखरम्_2] == AUTO)
			margin[शिखरम्_2] = 0;
		यदि (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_खण्डम्->height -
				शिखरम् - margin[शिखरम्_2] - border[शिखरम्_2].विस्तारः -
				padding[शिखरम्_2] - height - padding[BOTTOM] -
				border[BOTTOM].विस्तारः - margin[BOTTOM];
	} अन्यथा यदि (शिखरम् != AUTO && height != AUTO && bottom != AUTO) {
		यदि (margin[शिखरम्_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_खण्डम्->height -
					शिखरम् - border[शिखरम्_2].विस्तारः - padding[शिखरम्_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].विस्तारः - bottom;
			margin[शिखरम्_2] = margin[BOTTOM] = space / 2;
		} अन्यथा यदि (margin[शिखरम्_2] == AUTO) {
			margin[शिखरम्_2] = containing_खण्डम्->height -
					शिखरम् - border[शिखरम्_2].विस्तारः - padding[शिखरम्_2] -
					height - padding[BOTTOM] -
					border[BOTTOM].विस्तारः - margin[BOTTOM] -
					bottom;
		} अन्यथा यदि (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_खण्डम्->height -
					शिखरम् - margin[शिखरम्_2] - border[शिखरम्_2].विस्तारः -
					padding[शिखरम्_2] - height -
					padding[BOTTOM] - border[BOTTOM].विस्तारः -
					bottom;
		} अन्यथा {
			bottom = containing_खण्डम्->height -
					शिखरम् - margin[शिखरम्_2] - border[शिखरम्_2].विस्तारः -
					padding[शिखरम्_2] - height -
					padding[BOTTOM] - border[BOTTOM].विस्तारः -
					margin[BOTTOM];
		}
	} अन्यथा {
		यदि (margin[शिखरम्_2] == AUTO)
			margin[शिखरम्_2] = 0;
		यदि (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		यदि (शिखरम् == AUTO && height == AUTO && bottom != AUTO) {
			height = box->height;
			शिखरम् = containing_खण्डम्->height -
					margin[शिखरम्_2] - border[शिखरम्_2].विस्तारः -
					padding[शिखरम्_2] - height -
					padding[BOTTOM] - border[BOTTOM].विस्तारः -
					margin[BOTTOM] - bottom;
		} अन्यथा यदि (शिखरम् == AUTO && height != AUTO && bottom == AUTO) {
			शिखरम् = static_शिखरम्;
			bottom = containing_खण्डम्->height -
					शिखरम् - margin[शिखरम्_2] - border[शिखरम्_2].विस्तारः -
					padding[शिखरम्_2] - height -
					padding[BOTTOM] - border[BOTTOM].विस्तारः -
					margin[BOTTOM];
		} अन्यथा यदि (शिखरम् != AUTO && height == AUTO && bottom == AUTO) {
			height = box->height;
			bottom = containing_खण्डम्->height -
					शिखरम् - margin[शिखरम्_2] - border[शिखरम्_2].विस्तारः -
					padding[शिखरम्_2] - height -
					padding[BOTTOM] - border[BOTTOM].विस्तारः -
					margin[BOTTOM];
		} अन्यथा यदि (शिखरम् == AUTO && height != AUTO && bottom != AUTO) {
			शिखरम् = containing_खण्डम्->height -
					margin[शिखरम्_2] - border[शिखरम्_2].विस्तारः -
					padding[शिखरम्_2] - height -
					padding[BOTTOM] - border[BOTTOM].विस्तारः -
					margin[BOTTOM] - bottom;
		} अन्यथा यदि (शिखरम् != AUTO && height == AUTO && bottom != AUTO) {
			height = containing_खण्डम्->height -
					शिखरम् - margin[शिखरम्_2] - border[शिखरम्_2].विस्तारः -
					padding[शिखरम्_2] - padding[BOTTOM] -
					border[BOTTOM].विस्तारः - margin[BOTTOM] -
					bottom;
		} अन्यथा यदि (शिखरम् != AUTO && height != AUTO && bottom == AUTO) {
			bottom = containing_खण्डम्->height -
					शिखरम् - margin[शिखरम्_2] - border[शिखरम्_2].विस्तारः -
					padding[शिखरम्_2] - height -
					padding[BOTTOM] - border[BOTTOM].विस्तारः -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      शिखरम्, margin[शिखरम्_2], border[शिखरम्_2].विस्तारः, padding[शिखरम्_2], height,
	      padding[BOTTOM], border[BOTTOM].विस्तारः, margin[BOTTOM], bottom,
	      containing_खण्डम्->height);

	box->y = शिखरम् + margin[शिखरम्_2] + border[शिखरम्_2].विस्तारः - cy;
	यदि (containing_खण्डम्->type == BOX_खण्डम् ||
			containing_खण्डम्->type == BOX_INLINE_खण्डम् ||
			containing_खण्डम्->type == BOX_सारणी_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_खण्डम्->height -= containing_खण्डम्->padding[शिखरम्_2] +
				containing_खण्डम्->padding[BOTTOM];
	} अन्यथा {
		/** \todo Inline ancestors */
	}
	box->height = height;
	layout_apply_minmax_height(&content->unit_len_ctx, box, containing_खण्डम्);

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
			 संरचना box *containing_खण्डम्,
			 पूर्णाङ्क cx, पूर्णाङ्क cy,
			 html_content *content)
{
	संरचना box *c;

	प्रति (c = box->children; c; c = c->अग्र) {
		यदि ((c->type == BOX_खण्डम् || c->type == BOX_सारणी ||
				c->type == BOX_INLINE_खण्डम् ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			यदि (!layout_absolute(c, containing_खण्डम्,
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
						CSS_FLOAT_वामः ||
					css_computed_float(c->style) ==
						CSS_FLOAT_दक्षिणः)) {
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
			यदि (!layout_position_absolute(c, containing_खण्डम्,
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
स्थिर शून्य layout_compute_relative_विस्थापनम्(
		ध्रुव css_unit_ctx *unit_len_ctx,
		संरचना box *box,
		पूर्णाङ्क *x,
		पूर्णाङ्क *y)
{
	पूर्णाङ्क वामः, दक्षिणः, शिखरम्, bottom;
	संरचना box *containing_खण्डम्;

	प्रतिज्ञा(box && box->parent && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	यदि (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_वामः ||
			css_computed_float(box->style) == CSS_FLOAT_दक्षिणः)) {
		containing_खण्डम् = box->float_container;
	} अन्यथा {
		containing_खण्डम् = box->parent;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_खण्डम्,
			&शिखरम्, &दक्षिणः, &bottom, &वामः);

	यदि (वामः == AUTO && दक्षिणः == AUTO)
		वामः = दक्षिणः = 0;
	अन्यथा यदि (वामः == AUTO)
		/* left is auto => computed = -right */
		वामः = -दक्षिणः;
	अन्यथा यदि (दक्षिणः == AUTO)
		/* right is auto => computed = -left */
		दक्षिणः = -वामः;
	अन्यथा {
		/* over constrained => examine direction property
		 * of containing block */
		यदि (containing_खण्डम्->style &&
				css_computed_direction(
				containing_खण्डम्->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			वामः = -दक्षिणः;
		} अन्यथा {
			/* assume LTR in all other cases */
			दक्षिणः = -वामः;
		}
	}

	प्रतिज्ञा(वामः == -दक्षिणः);

	यदि (शिखरम् == AUTO && bottom == AUTO) {
		शिखरम् = bottom = 0;
	} अन्यथा यदि (शिखरम् == AUTO) {
		शिखरम् = -bottom;
	} अन्यथा {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -शिखरम्;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", वामः,
	      दक्षिणः, शिखरम्, bottom);

	*x = वामः;
	*y = शिखरम्;
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
		संरचना box *मूलम्,
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

	प्रतिज्ञा(मूलम्);

	/* Normal children */
	प्रति (box = मूलम्->children; box; box = box->अग्र) {

		यदि (box->type == BOX_TEXT)
			अनुवर्त;

		/* If relatively positioned, get offsets */
		यदि (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_विस्थापनम्(
					unit_len_ctx, box, &x, &y);
		अन्यथा
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		यदि (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_वामः ||
				css_computed_float(box->style) ==
					CSS_FLOAT_दक्षिणः) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			प्रति (fc = fp->float_children; fc; fc = fc->अग्र_float) {
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
		यदि (box->type == BOX_INLINE && box->inline_अन्तः) {
			संरचना box *b;
			प्रति (b = box->अग्र; b && b != box->inline_अन्तः;
					b = b->अग्र) {
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
	*desc_x0 = -box->border[वामः_2].विस्तारः;
	*desc_y0 = -box->border[शिखरम्_2].विस्तारः;
	*desc_x1 = box->padding[वामः_2] + box->विस्तारः + box->padding[दक्षिणः_2] +
			box->border[दक्षिणः_2].विस्तारः;
	*desc_y1 = box->padding[शिखरम्_2] + box->height + box->padding[BOTTOM] +
			box->border[BOTTOM].विस्तारः;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	यदि (box->type == BOX_खण्डम् && box->style != रिक्त &&
			css_computed_अतिप्रवाहः_y(box->style) ==
					CSS_अतिप्रवाहः_VISIBLE &&
			box->object == रिक्त) {
		css_fixed font_आकार = 0;
		css_unit font_unit = CSS_UNIT_PT;
		पूर्णाङ्क text_height;

		css_computed_font_आकार(box->style, &font_आकार, &font_unit);
		text_height = css_unit_len2device_px(box->style, unit_len_ctx,
				font_आकार, font_unit);
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

	गणना css_अतिप्रवाहः_e अतिप्रवाहः_x = CSS_अतिप्रवाहः_VISIBLE;
	गणना css_अतिप्रवाहः_e अतिप्रवाहः_y = CSS_अतिप्रवाहः_VISIBLE;

	यदि (child->style != रिक्त) {
		अतिप्रवाहः_x = css_computed_अतिप्रवाहः_x(child->style);
		अतिप्रवाहः_y = css_computed_अतिप्रवाहः_y(child->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, child,
			&child_desc_x0, &child_desc_y0,
			&child_desc_x1, &child_desc_y1);

	यदि (अतिप्रवाहः_x == CSS_अतिप्रवाहः_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		child_desc_x0 = child->descendant_x0;
		child_desc_x1 = child->descendant_x1;
	}
	यदि (अतिप्रवाहः_y == CSS_अतिप्रवाहः_VISIBLE &&
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

	प्रतिज्ञा(box->विस्तारः != UNKNOWN_विस्तारः);
	प्रतिज्ञा(box->height != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	यदि (box->object && content_get_type(box->object) == CONTENT_HTML) {
		यदि (box->descendant_x1 < content_get_विस्तारः(box->object))
			box->descendant_x1 = content_get_विस्तारः(box->object);
		यदि (box->descendant_y1 < content_get_height(box->object))
			box->descendant_y1 = content_get_height(box->object);
	}

	यदि (box->iframe != रिक्त) {
		पूर्णाङ्क x, y;
		box_coords(box, &x, &y);

		browser_गवाक्षः_समुच्चयः_position(box->iframe, x, y);
		browser_गवाक्षः_समुच्चयः_dimensions(box->iframe,
				box->विस्तारः, box->height);
		browser_गवाक्षः_reformat(box->iframe, true,
				box->विस्तारः, box->height);
	}

	यदि (box->type == BOX_INLINE || box->type == BOX_TEXT)
		प्रत्यागम;

	यदि (box->type == BOX_INLINE_अन्तः) {
		box = box->inline_अन्तः;
		प्रति (child = box->अग्र; child;
				child = child->अग्र) {
			यदि (child->type == BOX_FLOAT_वामः ||
					child->type == BOX_FLOAT_दक्षिणः)
				अनुवर्त;

			layout_update_descendant_bbox(unit_len_ctx, box, child,
					box->x, box->y);

			यदि (child == box->inline_अन्तः)
				भङ्ग;
		}
		प्रत्यागम;
	}

	यदि (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		प्रत्यागम;

	प्रति (child = box->children; child; child = child->अग्र) {
		यदि (child->type == BOX_FLOAT_वामः ||
				child->type == BOX_FLOAT_दक्षिणः)
			अनुवर्त;

		layout_calculate_descendant_bboxes(unit_len_ctx, child);

		यदि (box->style && css_computed_अतिप्रवाहः_x(box->style) ==
				CSS_अतिप्रवाहः_HIDDEN &&
				css_computed_अतिप्रवाहः_y(box->style) ==
				CSS_अतिप्रवाहः_HIDDEN)
			अनुवर्त;

		layout_update_descendant_bbox(unit_len_ctx, box, child, 0, 0);
	}

	प्रति (child = box->float_children; child; child = child->अग्र_float) {
		प्रतिज्ञा(child->type == BOX_FLOAT_वामः ||
				child->type == BOX_FLOAT_दक्षिणः);

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
bool layout_document(html_content *content, पूर्णाङ्क विस्तारः, पूर्णाङ्क height)
{
	bool ret;
	संरचना box *doc = content->layout;
	ध्रुव संरचना gui_layout_सारणी *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			विस्तारः, height, nsurl_access(content_get_url(
					&content->आधारः)));

	layout_minmax_खण्डम्(doc, font_func, content);

	layout_खण्डम्_find_dimensions(&content->unit_len_ctx,
			विस्तारः, height, 0, 0, doc);
	doc->x = doc->margin[वामः_2] + doc->border[वामः_2].विस्तारः;
	doc->y = doc->margin[शिखरम्_2] + doc->border[शिखरम्_2].विस्तारः;
	विस्तारः -= doc->margin[वामः_2] + doc->border[वामः_2].विस्तारः +
			doc->padding[वामः_2] + doc->padding[दक्षिणः_2] +
			doc->border[दक्षिणः_2].विस्तारः + doc->margin[दक्षिणः_2];
	यदि (विस्तारः < 0) {
		विस्तारः = 0;
	}
	doc->विस्तारः = विस्तारः;

	ret = layout_खण्डम्_context(doc, height, content);

	/* make <html> and <body> fill available height */
	यदि (doc->y + doc->padding[शिखरम्_2] + doc->height + doc->padding[BOTTOM] +
			doc->border[BOTTOM].विस्तारः + doc->margin[BOTTOM] <
			height) {
		doc->height = height - (doc->y + doc->padding[शिखरम्_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].विस्तारः +
				doc->margin[BOTTOM]);
		यदि (doc->children)
			doc->children->height = doc->height -
					(doc->children->margin[शिखरम्_2] +
					 doc->children->border[शिखरम्_2].विस्तारः +
					 doc->children->padding[शिखरम्_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].विस्तारः +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	प्रत्यागम ret;
}
