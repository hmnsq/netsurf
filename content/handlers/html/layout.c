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
	[چوٹی_2]    = css_computed_margin_چوٹی,
	[دایاں_2]  = css_computed_margin_دایاں,
	[BOTTOM] = css_computed_margin_bottom,
	[بایاں_2]   = css_computed_margin_بایاں,
};

/** Array of per-side access functions for computed style paddings. */
ثابت css_len_func padding_funcs[4] = {
	[چوٹی_2]    = css_computed_padding_چوٹی,
	[دایاں_2]  = css_computed_padding_دایاں,
	[BOTTOM] = css_computed_padding_bottom,
	[بایاں_2]   = css_computed_padding_بایاں,
};

/** Array of per-side access functions for computed style border_widths. */
ثابت css_len_func border_چوڑائی_funcs[4] = {
	[چوٹی_2]    = css_computed_border_چوٹی_چوڑائی,
	[دایاں_2]  = css_computed_border_دایاں_چوڑائی,
	[BOTTOM] = css_computed_border_bottom_چوڑائی,
	[بایاں_2]   = css_computed_border_بایاں_چوڑائی,
};

/** Array of per-side access functions for computed style border styles. */
ثابت css_border_style_func border_style_funcs[4] = {
	[چوٹی_2]    = css_computed_border_چوٹی_style,
	[دایاں_2]  = css_computed_border_دایاں_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[بایاں_2]   = css_computed_border_بایاں_style,
};

/** Array of per-side access functions for computed style border colors. */
ثابت css_border_color_func border_color_funcs[4] = {
	[چوٹی_2]    = css_computed_border_چوٹی_color,
	[دایاں_2]  = css_computed_border_دایاں_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[بایاں_2]   = css_computed_border_بایاں_color,
};

/* forward declaration to break cycles */
ساکن خالی layout_minmax_بلاک(
		ساخت box *بلاک,
		ثابت ساخت gui_layout_جدول *font_func,
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
ساکن خالی
layout_get_object_dimensions(ساخت box *box,
			     صحیح *چوڑائی, صحیح *اونچائی,
			     صحیح min_چوڑائی, صحیح max_چوڑائی,
			     صحیح min_اونچائی, صحیح max_اونچائی)
{
	أكد(box->object != تہی);
	أكد(چوڑائی != تہی && اونچائی != تہی);

	اگر (*چوڑائی == AUTO && *اونچائی == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		صحیح intrinsic_چوڑائی = content_get_چوڑائی(box->object);
		صحیح intrinsic_اونچائی = content_get_اونچائی(box->object);

		/* use intrinsic dimensions */
		*چوڑائی = intrinsic_چوڑائی;
		*اونچائی = intrinsic_اونچائی;

		/* Deal with min/max-width first */
		اگر (min_چوڑائی >  0 && min_چوڑائی > *چوڑائی) {
			*چوڑائی = min_چوڑائی;
			scaled = true;
		}
		اگر (max_چوڑائی >= 0 && max_چوڑائی < *چوڑائی) {
			*چوڑائی = max_چوڑائی;
			scaled = true;
		}

		اگر (scaled && (intrinsic_چوڑائی != 0)) {
			/* Update height */
			*اونچائی = (*چوڑائی * intrinsic_اونچائی) /
					intrinsic_چوڑائی;
		}

		scaled = false;
		/* Deal with min/max-height */
		اگر (min_اونچائی >  0 && min_اونچائی > *اونچائی) {
			*اونچائی = min_اونچائی;
			scaled = true;
		}
		اگر (max_اونچائی >= 0 && max_اونچائی < *اونچائی) {
			*اونچائی = max_اونچائی;
			scaled = true;
		}

		اگر (scaled && (intrinsic_اونچائی != 0)) {
			/* Update width */
			*چوڑائی = (*اونچائی * intrinsic_چوڑائی) /
					intrinsic_اونچائی;
		}

	} ورنہ اگر (*چوڑائی == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		صحیح intrinsic_چوڑائی = content_get_چوڑائی(box->object);
		صحیح intrinsic_اونچائی = content_get_اونچائی(box->object);

		اگر (intrinsic_اونچائی != 0)
			*چوڑائی = (*اونچائی * intrinsic_چوڑائی) /
					intrinsic_اونچائی;
		ورنہ
			*چوڑائی = intrinsic_چوڑائی;

		اگر (min_چوڑائی >  0 && min_چوڑائی > *چوڑائی)
			*چوڑائی = min_چوڑائی;
		اگر (max_چوڑائی >= 0 && max_چوڑائی < *چوڑائی)
			*چوڑائی = max_چوڑائی;

	} ورنہ اگر (*اونچائی == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		صحیح intrinsic_چوڑائی = content_get_چوڑائی(box->object);
		صحیح intrinsic_اونچائی = content_get_اونچائی(box->object);

		اگر (min_چوڑائی >  0 && min_چوڑائی > *چوڑائی)
			*چوڑائی = min_چوڑائی;
		اگر (max_چوڑائی >= 0 && max_چوڑائی < *چوڑائی)
			*چوڑائی = max_چوڑائی;

		اگر (intrinsic_چوڑائی != 0)
			*اونچائی = (*چوڑائی * intrinsic_اونچائی) /
					intrinsic_چوڑائی;
		ورنہ
			*اونچائی = intrinsic_اونچائی;
	}
}


/**
 * Calculate the text-indent length.
 *
 * \param  style  style of block
 * \param  width  width of containing block
 * \return  length of indent
 */
ساکن صحیح layout_متن_indent(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style, صحیح چوڑائی)
{
	css_fixed قدر = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_متن_indent(style, &قدر, &unit);

	اگر (unit == CSS_UNIT_PCT) {
		واپس FPCT_کا_INT_TOINT(قدر, چوڑائی);
	} ورنہ {
		واپس FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				قدر, unit));
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
ساکن خالی layout_minmax_جدول(ساخت box *جدول,
		ثابت ساخت gui_layout_جدول *font_func,
		ثابت html_content *content)
{
	غیرعلامتی صحیح i, j;
	صحیح چوڑائی;
	صحیح border_spacing_h = 0;
	صحیح جدول_min = 0, جدول_max = 0;
	صحیح extra_fixed = 0;
	اعشاری extra_frac = 0;
	ساخت عمود *ستون;
	ساخت box *صف_گروہ, *صف, *cell;

	/* check if the widths have already been calculated */
	اگر (جدول->max_چوڑائی != UNKNOWN_MAX_چوڑائی)
		واپس;

	اگر (جدول_calculate_عمود_types(&content->unit_len_ctx, جدول) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		واپس;
	}
	ستون = جدول->ستون;

	/* start with 0 except for fixed-width columns */
	برائے (i = 0; i != جدول->columns; i++) {
		اگر (ستون[i].type == عمود_چوڑائی_FIXED)
			ستون[i].min = ستون[i].max = ستون[i].چوڑائی;
		ورنہ
			ستون[i].min = ستون[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	اگر (css_computed_border_collapse(جدول->style) ==
			CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		css_computed_border_spacing(جدول->style, &h, &hu, &v, &vu);

		border_spacing_h = FIXTOINT(css_unit_len2device_px(
				جدول->style,
				&content->unit_len_ctx,
				h, hu));
	}

	/* 1st pass: consider cells with colspan 1 only */
	برائے (صف_گروہ = جدول->children; صف_گروہ; صف_گروہ =صف_گروہ->اگلا)
	برائے (صف = صف_گروہ->children; صف; صف = صف->اگلا)
	برائے (cell = صف->children; cell; cell = cell->اگلا) {
		أكد(cell->type == BOX_جدول_CELL);
		أكد(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		أكد(cell->columns != 0);

		اگر (cell->columns != 1)
			جاری;

		layout_minmax_بلاک(cell, font_func, content);
		i = cell->آغاز_عمود;

		اگر (ستون[i].positioned)
			جاری;

		/* update column min, max widths using cell widths */
		اگر (ستون[i].min < cell->min_چوڑائی)
			ستون[i].min = cell->min_چوڑائی;
		اگر (ستون[i].max < cell->max_چوڑائی)
			ستون[i].max = cell->max_چوڑائی;
	}

	/* 2nd pass: cells which span multiple columns */
	برائے (صف_گروہ = جدول->children; صف_گروہ; صف_گروہ =صف_گروہ->اگلا)
	برائے (صف = صف_گروہ->children; صف; صف = صف->اگلا)
	برائے (cell = صف->children; cell; cell = cell->اگلا) {
		غیرعلامتی صحیح flexible_columns = 0;
		صحیح min = 0, max = 0, fixed_چوڑائی = 0, extra;

		اگر (cell->columns == 1)
			جاری;

		layout_minmax_بلاک(cell, font_func, content);
		i = cell->آغاز_عمود;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		برائے (j = 0; j != cell->columns; j++) {
			min += ستون[i + j].min;
			اگر (ستون[i + j].type == عمود_چوڑائی_FIXED)
				fixed_چوڑائی += ستون[i + j].چوڑائی;
			ورنہ
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		اگر (min < cell->min_چوڑائی) {
			اگر (flexible_columns == 0) {
				extra = 1 + (cell->min_چوڑائی - min) /
						cell->columns;
				برائے (j = 0; j != cell->columns; j++) {
					ستون[i + j].min += extra;
					اگر (ستون[i + j].max < ستون[i + j].min)
						ستون[i + j].max = ستون[i + j].min;
				}
			} ورنہ {
				extra = 1 + (cell->min_چوڑائی - min) /
						flexible_columns;
				برائے (j = 0; j != cell->columns; j++) {
					اگر (ستون[i + j].type !=
							عمود_چوڑائی_FIXED) {
						ستون[i + j].min += extra;
						اگر (ستون[i + j].max <
								ستون[i + j].min)
							ستون[i + j].max =
								ستون[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		برائے (j = 0; j != cell->columns; j++)
			max += ستون[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		اگر (max < cell->max_چوڑائی && flexible_columns) {
			extra = 1 + (cell->max_چوڑائی - max) / flexible_columns;
			برائے (j = 0; j != cell->columns; j++)
				اگر (ستون[i + j].type != عمود_چوڑائی_FIXED)
					ستون[i + j].max += extra;
		}
	}

	برائے (i = 0; i != جدول->columns; i++) {
		اگر (ستون[i].max < ستون[i].min) {
			box_dump(stderr, جدول, 0, true);
			أكد(0);
		}
		جدول_min += ستون[i].min;
		جدول_max += ستون[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	اگر (css_computed_چوڑائی_px(جدول->style, &content->unit_len_ctx,
			-1, &چوڑائی) == CSS_چوڑائی_سیٹ) {
		اگر (جدول_min < چوڑائی)
			جدول_min = چوڑائی;
		اگر (جدول_max < چوڑائی)
			جدول_max = چوڑائی;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_چوڑائی(&content->unit_len_ctx,
			جدول->style, بایاں_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_چوڑائی(&content->unit_len_ctx,
			جدول->style, دایاں_2, true, true, true,
			&extra_fixed, &extra_frac);
	اگر (extra_fixed < 0)
		extra_fixed = 0;
	اگر (extra_frac < 0)
		extra_frac = 0;
	اگر (1.0 <= extra_frac)
		extra_frac = 0.9;
	جدول->min_چوڑائی = (جدول_min + extra_fixed) / (1.0 - extra_frac);
	جدول->max_چوڑائی = (جدول_max + extra_fixed) / (1.0 - extra_frac);
	جدول->min_چوڑائی += (جدول->columns + 1) * border_spacing_h;
	جدول->max_چوڑائی += (جدول->columns + 1) * border_spacing_h;

	أكد(0 <= جدول->min_چوڑائی && جدول->min_چوڑائی <= جدول->max_چوڑائی);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
ساکن اندرونی bool box_has_percentage_max_چوڑائی(ساخت box *b)
{
	css_unit unit = CSS_UNIT_PX;
	شمار css_max_چوڑائی_e type;
	css_fixed قدر = 0;

	أكد(b != تہی);

	type = css_computed_max_چوڑائی(b->style, &قدر, &unit);
	واپس ((type == CSS_MAX_چوڑائی_سیٹ) && (unit == CSS_UNIT_PCT));
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
ساکن ساخت box *
layout_minmax_سطر(ساخت box *پہلا,
		   صحیح *سطر_min,
		   صحیح *سطر_max,
		   bool پہلا_سطر,
		   bool *سطر_has_اونچائی,
		   ثابت ساخت gui_layout_جدول *font_func,
		   ثابت html_content *content)
{
	صحیح min = 0, max = 0, چوڑائی, اونچائی, fixed;
	اعشاری frac;
	size_t i, j;
	ساخت box *b;
	ساخت box *بلاک;
	plot_font_style_t fstyle;
	bool no_wrap;

	أكد(پہلا->والد);
	أكد(پہلا->والد->والد);
	أكد(پہلا->والد->والد->style);

	بلاک = پہلا->والد->والد;
	no_wrap = (css_computed_white_space(بلاک->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(بلاک->style) ==
			CSS_WHITE_SPACE_PRE);

	*سطر_has_اونچائی = false;

	/* corresponds to the pass 1 loop in layout_line() */
	برائے (b = پہلا; b; b = b->اگلا) {
		شمار css_چوڑائی_e wtype;
		شمار css_اونچائی_e htype;
		شمار css_box_sizing_e bs;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		أكد(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		اگر (b->type == BOX_BR) {
			b = b->اگلا;
			توڑو;
		}

		اگر (lh__box_is_float_box(b)) {
			أكد(b->children);
			اگر (b->children->type == BOX_جدول)
				layout_minmax_جدول(b->children, font_func,
						content);
			ورنہ
				layout_minmax_بلاک(b->children, font_func,
						content);
			b->min_چوڑائی = b->children->min_چوڑائی;
			b->max_چوڑائی = b->children->max_چوڑائی;
			اگر (min < b->min_چوڑائی)
				min = b->min_چوڑائی;
			max += b->max_چوڑائی;
			جاری;
		}

		اگر (b->type == BOX_INLINE_بلاک || b->type == BOX_INLINE_FLEX) {
			layout_minmax_بلاک(b, font_func, content);
			اگر (min < b->min_چوڑائی)
				min = b->min_چوڑائی;
			max += b->max_چوڑائی;

			اگر (b->flags & HAS_اونچائی)
				*سطر_has_اونچائی = true;
			جاری;
		}

		أكد(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		اگر (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_چوڑائی(&content->unit_len_ctx,
					b->style, بایاں_2, true, true, true,
					&fixed, &frac);
			اگر (!b->inline_انتہا)
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, دایاں_2,
						true, true, true,
						&fixed, &frac);
			اگر (0 < fixed)
				max += fixed;
			*سطر_has_اونچائی = true;
			/* \todo  update min width, consider fractional extra */
		} ورنہ اگر (b->type == BOX_INLINE_انتہا) {
			fixed = frac = 0;
			calculate_mbp_چوڑائی(&content->unit_len_ctx,
					b->inline_انتہا->style, دایاں_2,
					true, true, true,
					&fixed, &frac);
			اگر (0 < fixed)
				max += fixed;

			اگر (b->اگلا) {
				اگر (b->space == UNKNOWN_چوڑائی) {
					font_func->چوڑائی(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*سطر_has_اونچائی = true;
			جاری;
		}

		اگر (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			اگر (!b->متن)
				جاری;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			اگر (b->چوڑائی == UNKNOWN_چوڑائی) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				اگر (b->والد->والد->gadget &&
						b->والد->والد->gadget->type
						== GADGET_SELECT) {
					صحیح opt_maxwidth = 0;
					ساخت form_option *o;

					برائے (o = b->والد->والد->gadget->
							ڈیٹا.select.اشیاء; o;
							o = o->اگلا) {
						صحیح opt_چوڑائی;
						font_func->چوڑائی(&fstyle,
								o->متن,
								لڑی_لمبائی(o->متن),
								&opt_چوڑائی);

						اگر (opt_maxwidth < opt_چوڑائی)
							opt_maxwidth =opt_چوڑائی;
					}

					b->چوڑائی = opt_maxwidth;
					اگر (nsoption_bool(core_select_menu))
						b->چوڑائی += SCROLLBAR_چوڑائی;

				} ورنہ {
					font_func->چوڑائی(&fstyle, b->متن,
						b->لمبائی, &b->چوڑائی);
					b->flags |= MEASURED;
				}
			}
			max += b->چوڑائی;
			اگر (b->اگلا) {
				اگر (b->space == UNKNOWN_چوڑائی) {
					font_func->چوڑائی(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			اگر (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} ورنہ اگر (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				اگر (min < b->چوڑائی)
					min = b->چوڑائی;

			} ورنہ اگر (b->والد->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				کرو {
					برائے (j = i; j != b->لمبائی &&
							b->متن[j] != ' '; j++)
						;
					font_func->چوڑائی(&fstyle, b->متن + i,
							 j - i, &چوڑائی);
					اگر (min < چوڑائی)
						min = چوڑائی;
					i = j + 1;
				} جب (j != b->لمبائی);
			}

			*سطر_has_اونچائی = true;

			جاری;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		bs = css_computed_box_sizing(بلاک->style);

		/* calculate box width */
		wtype = css_computed_چوڑائی_px(b->style,
				&content->unit_len_ctx, -1, &چوڑائی);
		اگر (wtype == CSS_چوڑائی_سیٹ) {
			اگر (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						بلاک->style, بایاں_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						بلاک->style, دایاں_2,
						false, true, true,
						&fixed, &frac);
				اگر (چوڑائی < fixed) {
					چوڑائی = fixed;
				}
			}
			اگر (چوڑائی < 0)
				چوڑائی = 0;
		} ورنہ {
			چوڑائی = AUTO;
		}

		/*اونچائی*/
		htype = css_computed_اونچائی(b->style, &قدر, &unit);
		اگر (htype == CSS_اونچائی_سیٹ) {
			اونچائی = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					قدر, unit));
		} ورنہ {
			اونچائی = AUTO;
		}

		اگر (b->object || (b->flags & REPLACE_DIM)) {
			اگر (b->object) {
				صحیح عارضی_اونچائی = اونچائی;
				layout_get_object_dimensions(b,
						&چوڑائی, &عارضی_اونچائی,
						INT_MIN, زیادہ_سے_زیادہ_عدد,
						INT_MIN, زیادہ_سے_زیادہ_عدد);
			}

			fixed = frac = 0;
			اگر (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, بایاں_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, دایاں_2,
						true, false, false,
						&fixed, &frac);
			} ورنہ {
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, بایاں_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, دایاں_2,
						true, true, true,
						&fixed, &frac);
			}
			اگر (0 < چوڑائی + fixed)
				چوڑائی += fixed;
		} ورنہ اگر (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			اگر (چوڑائی == AUTO)
				چوڑائی = 400;

			fixed = frac = 0;
			اگر (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, بایاں_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, دایاں_2,
						true, false, false,
						&fixed, &frac);
			} ورنہ {
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, بایاں_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, دایاں_2,
						true, true, true,
						&fixed, &frac);
			}

			اگر (0 < چوڑائی + fixed)
				چوڑائی += fixed;

		} ورنہ {
			/* form control with no object */
			اگر (چوڑائی == AUTO)
				چوڑائی = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		اگر (min < چوڑائی && !box_has_percentage_max_چوڑائی(b))
			min = چوڑائی;
		اگر (چوڑائی > 0)
			max += چوڑائی;

		*سطر_has_اونچائی = true;
	}

	اگر (پہلا_سطر) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		صحیح متن_indent = layout_متن_indent(&content->unit_len_ctx,
				پہلا->والد->والد->style, 100);
		min = (min + متن_indent < 0) ? 0 : min + متن_indent;
		max = (max + متن_indent < 0) ? 0 : max + متن_indent;
	}

	*سطر_min = min;
	*سطر_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	أكد(b != پہلا);
	أكد(0 <= *سطر_min);
	أكد(*سطر_min <= *سطر_max);
	واپس b;
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
ساکن خالی
layout_minmax_inline_container(ساخت box *inline_container,
			       bool *has_اونچائی,
			       ثابت ساخت gui_layout_جدول *font_func,
			       ثابت html_content *content)
{
	ساخت box *بچہ;
	صحیح سطر_min = 0, سطر_max = 0;
	صحیح min = 0, max = 0;
	bool پہلا_سطر = true;
	bool سطر_has_اونچائی;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	اگر (inline_container->max_چوڑائی != UNKNOWN_MAX_چوڑائی)
		واپس;

	*has_اونچائی = false;

	برائے (بچہ = inline_container->children; بچہ; ) {
		بچہ = layout_minmax_سطر(بچہ, &سطر_min, &سطر_max,
				پہلا_سطر, &سطر_has_اونچائی, font_func,
				content);
		اگر (min < سطر_min)
			min = سطر_min;
		اگر (max < سطر_max)
			max = سطر_max;
		پہلا_سطر = false;
		*has_اونچائی |= سطر_has_اونچائی;
	}

	inline_container->min_چوڑائی = min;
	inline_container->max_چوڑائی = max;

	أكد(0 <= inline_container->min_چوڑائی &&
			inline_container->min_چوڑائی <=
			inline_container->max_چوڑائی);
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
ساکن خالی layout_minmax_بلاک(
		ساخت box *بلاک,
		ثابت ساخت gui_layout_جدول *font_func,
		ثابت html_content *content)
{
	ساخت box *بچہ;
	صحیح min = 0, max = 0;
	صحیح extra_fixed = 0;
	اعشاری extra_frac = 0;
	شمار css_چوڑائی_e wtype = CSS_چوڑائی_AUTO;
	css_fixed چوڑائی = 0;
	css_unit wunit = CSS_UNIT_PX;
	شمار css_اونچائی_e htype = CSS_اونچائی_AUTO;
	css_fixed اونچائی = 0;
	css_unit hunit = CSS_UNIT_PX;
	شمار css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool بچہ_has_اونچائی = false;

	أكد(بلاک->type == BOX_بلاک ||
			بلاک->type == BOX_FLEX ||
			بلاک->type == BOX_INLINE_FLEX ||
			بلاک->type == BOX_INLINE_بلاک ||
			بلاک->type == BOX_جدول_CELL);

	/* check if the widths have already been calculated */
	اگر (بلاک->max_چوڑائی != UNKNOWN_MAX_چوڑائی)
		واپس;

	اگر (بلاک->style != تہی) {
		wtype = css_computed_چوڑائی(بلاک->style, &چوڑائی, &wunit);
		htype = css_computed_اونچائی(بلاک->style, &اونچائی, &hunit);
		bs = css_computed_box_sizing(بلاک->style);
	}

	/* set whether the minimum width is of any interest for this box */
	اگر (((بلاک->والد && lh__box_is_float_box(بلاک->والد)) ||
			بلاک->type == BOX_INLINE_بلاک ||
			بلاک->type == BOX_INLINE_FLEX) &&
			wtype != CSS_چوڑائی_سیٹ) {
		/* box shrinks to fit; need minimum width */
		بلاک->flags |= NEED_MIN;
	} ورنہ اگر (بلاک->type == BOX_جدول_CELL) {
		/* box shrinks to fit; need minimum width */
		بلاک->flags |= NEED_MIN;
	} ورنہ اگر (بلاک->والد && (بلاک->والد->flags & NEED_MIN) &&
			wtype != CSS_چوڑائی_سیٹ) {
		/* box inside shrink-to-fit context; need minimum width */
		بلاک->flags |= NEED_MIN;
	} ورنہ اگر (بلاک->والد && (بلاک->والد->type == BOX_FLEX)) {
		/* box is flex item */
		بلاک->flags |= NEED_MIN;
	}

	اگر (بلاک->gadget && (بلاک->gadget->type == GADGET_TEXTBOX ||
			بلاک->gadget->type == GADGET_PASSWORD ||
			بلاک->gadget->type == GADGET_FILE ||
			بلاک->gadget->type == GADGET_TEXTAREA) &&
			بلاک->style && wtype == CSS_چوڑائی_AUTO) {
		css_fixed سائز = INTTOFIX(10);
		css_unit unit = CSS_UNIT_EM;

		min = max = FIXTOINT(css_unit_len2device_px(بلاک->style,
				&content->unit_len_ctx, سائز, unit));

		بلاک->flags |= HAS_اونچائی;
	}

	اگر (بلاک->gadget && (بلاک->gadget->type == GADGET_RADIO ||
			بلاک->gadget->type == GADGET_CHECKBOX) &&
			بلاک->style && wtype == CSS_چوڑائی_AUTO) {
		css_fixed سائز = INTTOFIX(1);
		css_unit unit = CSS_UNIT_EM;

		/* form checkbox or radio button
		 * if width is AUTO, set it to 1em */
		min = max = FIXTOINT(css_unit_len2device_px(بلاک->style,
				&content->unit_len_ctx, سائز, unit));

		بلاک->flags |= HAS_اونچائی;
	}

	اگر (بلاک->object) {
		اگر (content_get_type(بلاک->object) == CONTENT_HTML) {
			layout_minmax_بلاک(html_get_box_درخت(بلاک->object),
					font_func, content);
			min = html_get_box_درخت(بلاک->object)->min_چوڑائی;
			max = html_get_box_درخت(بلاک->object)->max_چوڑائی;
		} ورنہ {
			min = max = content_get_چوڑائی(بلاک->object);
		}

		بلاک->flags |= HAS_اونچائی;
	} ورنہ اگر (بلاک->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		بلاک->flags |= HAS_اونچائی;
	} ورنہ {
		/* recurse through children */
		برائے (بچہ = بلاک->children; بچہ; بچہ = بچہ->اگلا) {
			انتخاب (بچہ->type) {
			حالت BOX_FLEX:
			حالت BOX_بلاک:
				layout_minmax_بلاک(بچہ, font_func,
						content);
				اگر (بچہ->flags & HAS_اونچائی)
					بچہ_has_اونچائی = true;
				توڑو;
			حالت BOX_INLINE_CONTAINER:
				اگر (بلاک->flags & NEED_MIN)
					بچہ->flags |= NEED_MIN;

				layout_minmax_inline_container(بچہ,
						&بچہ_has_اونچائی, font_func,
						content);
				اگر (بچہ_has_اونچائی &&
						بچہ ==
						بچہ->والد->children) {
					بلاک->flags |= MAKE_اونچائی;
				}
				توڑو;
			حالت BOX_جدول:
				layout_minmax_جدول(بچہ, font_func,
						content);
				/* todo: fix for zero height tables */
				بچہ_has_اونچائی = true;
				بچہ->flags |= MAKE_اونچائی;
				توڑو;
			طےشدہ:
				أكد(0);
			}
			أكد(بچہ->max_چوڑائی != UNKNOWN_MAX_چوڑائی);

			اگر (بچہ->style &&
					(css_computed_position(بچہ->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(بچہ->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				جاری;
			}

			اگر (lh__box_is_flex_container(بلاک) &&
			    lh__flex_main_is_horizontal(بلاک)) {
				اگر (بلاک->style != تہی &&
				    css_computed_flex_wrap(بلاک->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += بچہ->min_چوڑائی;
				} ورنہ {
					اگر (min < بچہ->min_چوڑائی)
						min = بچہ->min_چوڑائی;
				}
				max += بچہ->max_چوڑائی;

			} ورنہ {
				اگر (min < بچہ->min_چوڑائی)
					min = بچہ->min_چوڑائی;
				اگر (max < بچہ->max_چوڑائی)
					max = بچہ->max_چوڑائی;
			}

			اگر (بچہ_has_اونچائی)
				بلاک->flags |= HAS_اونچائی;
		}
	}

	اگر (max < min) {
		box_dump(stderr, بلاک, 0, true);
		أكد(0);
	}

	/* fixed width takes priority */
	اگر (بلاک->type != BOX_جدول_CELL && !lh_box_is_flex_شے(بلاک)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		شمار css_max_چوڑائی_e max_type;
		شمار css_min_چوڑائی_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed قدر = 0;
		صحیح چوڑائی;

		اگر (css_computed_چوڑائی_px(بلاک->style, &content->unit_len_ctx,
				-1, &چوڑائی) == CSS_چوڑائی_سیٹ) {
			min = max = چوڑائی;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_چوڑائی(بلاک->style, &قدر, &unit);
		اگر (min_type == CSS_MIN_چوڑائی_سیٹ && unit != CSS_UNIT_PCT) {
			صحیح val = FIXTOINT(css_unit_len2device_px(بلاک->style,
					&content->unit_len_ctx, قدر, unit));

			اگر (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_چوڑائی(بلاک->style, &قدر, &unit);
		اگر (max_type == CSS_MAX_چوڑائی_سیٹ && unit != CSS_UNIT_PCT) {
			صحیح val = FIXTOINT(css_unit_len2device_px(بلاک->style,
					&content->unit_len_ctx, قدر, unit));

			اگر (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	اگر (htype == CSS_اونچائی_سیٹ && hunit != CSS_UNIT_PCT &&
			اونچائی > INTTOFIX(0)) {
		بلاک->flags |= MAKE_اونچائی;
		بلاک->flags |= HAS_اونچائی;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_چوڑائی(&content->unit_len_ctx, بلاک->style, بایاں_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_چوڑائی(&content->unit_len_ctx, بلاک->style, دایاں_2,
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

	calculate_mbp_چوڑائی(&content->unit_len_ctx, بلاک->style, بایاں_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_چوڑائی(&content->unit_len_ctx, بلاک->style, دایاں_2,
			true, false, false, &extra_fixed, &extra_frac);

	اگر (extra_fixed < 0)
		extra_fixed = 0;
	اگر (extra_frac < 0)
		extra_frac = 0;
	اگر (1.0 <= extra_frac)
		extra_frac = 0.9;
	اگر (بلاک->style != تہی &&
			(css_computed_float(بلاک->style) == CSS_FLOAT_بایاں ||
			css_computed_float(بلاک->style) == CSS_FLOAT_دایاں)) {
		/* floated boxs */
		بلاک->min_چوڑائی = min + extra_fixed;
		بلاک->max_چوڑائی = max + extra_fixed;
	} ورنہ {
		/* not floated */
		بلاک->min_چوڑائی = (min + extra_fixed) / (1.0 - extra_frac);
		بلاک->max_چوڑائی = (max + extra_fixed) / (1.0 - extra_frac);
	}

	أكد(0 <= بلاک->min_چوڑائی);
	أكد(بلاک->min_چوڑائی <= بلاک->max_چوڑائی);
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
ساکن ساخت box*
layout_اگلا_margin_بلاک(ثابت css_unit_ctx *unit_len_ctx,
			 ساخت box *box,
			 ساخت box *بلاک,
			 صحیح viewport_اونچائی,
			 صحیح *max_pos_margin,
			 صحیح *max_neg_margin)
{
	أكد(بلاک != تہی);

	جب (box != تہی) {

		اگر (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			اگر (box->style) {
				layout_ڈھونڈو_dimensions(unit_len_ctx,
						box->والد->چوڑائی,
						viewport_اونچائی, box,
						box->style,
						تہی, تہی, تہی, تہی,
						تہی, تہی, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				اگر (*max_pos_margin < box->margin[چوٹی_2])
					*max_pos_margin = box->margin[چوٹی_2];
				ورنہ اگر (*max_neg_margin < -box->margin[چوٹی_2])
					*max_neg_margin = -box->margin[چوٹی_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			اگر (box->flags & MAKE_اونچائی ||
					box->border[چوٹی_2].چوڑائی ||
					box->padding[چوٹی_2] ||
					(box->style &&
					css_computed_حد_سے_تجاوز_y(box->style) !=
					CSS_حد_سے_تجاوز_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_پہلا_بچہ(box))) {
				/* Collapse to this box; return it */
				واپس box;
			}
		}


		/* Find next box */
		اگر (box->type == BOX_بلاک && !box->object && box->children &&
				box->style &&
				css_computed_حد_سے_تجاوز_y(box->style) ==
				CSS_حد_سے_تجاوز_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} ورنہ {
			اگر (!box->اگلا) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				کرو {
					/* Apply bottom margin */
					اگر (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					ورنہ اگر (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->والد;
				} جب (box != بلاک && !box->اگلا);

				اگر (box == بلاک) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					واپس بلاک;
				}
			}

			/* Apply bottom margin */
			اگر (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			ورنہ اگر (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->اگلا;

			/* Get margins */
			اگر (box->style) {
				layout_ڈھونڈو_dimensions(unit_len_ctx,
						box->والد->چوڑائی,
						viewport_اونچائی, box,
						box->style,
						تہی, تہی, تہی, تہی,
						تہی, تہی, box->margin,
						box->padding, box->border);
			}
		}
	}

	واپس تہی;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
ساکن صحیح layout_clear(ساخت box *fl, شمار css_clear_e clear)
{
	صحیح y = 0;
	برائے (; fl; fl = fl->اگلا_float) {
		اگر ((clear == CSS_CLEAR_بایاں || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_بایاں)
			اگر (y < fl->y + fl->اونچائی)
				y = fl->y + fl->اونچائی;
		اگر ((clear == CSS_CLEAR_دایاں || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_دایاں)
			اگر (y < fl->y + fl->اونچائی)
				y = fl->y + fl->اونچائی;
	}
	واپس y;
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
ساکن خالی
ڈھونڈو_sides(ساخت box *fl,
	   صحیح y0, صحیح y1,
	   صحیح *x0, صحیح *x1,
	   ساخت box **بایاں,
	   ساخت box **دایاں)
{
	صحیح fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*بایاں = *دایاں = 0;
	برائے (; fl; fl = fl->اگلا_float) {
		fy1 = fl->y + fl->اونچائی;
		اگر (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			واپس;
		}
		fy0 = fl->y;
		اگر (y0 < fy1 && fy0 <= y1) {
			اگر (fl->type == BOX_FLOAT_بایاں) {
				fx1 = fl->x + fl->چوڑائی;
				اگر (*x0 < fx1) {
					*x0 = fx1;
					*بایاں = fl;
				}
			} ورنہ {
				fx0 = fl->x;
				اگر (fx0 < *x1) {
					*x1 = fx0;
					*دایاں = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *بایاں, *دایاں);
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
ساکن صحیح
layout_solve_چوڑائی(ساخت box *box,
		   صحیح دستیاب_چوڑائی,
		   صحیح چوڑائی,
		   صحیح lm,
		   صحیح rm,
		   صحیح max_چوڑائی,
		   صحیح min_چوڑائی)
{
	bool auto_چوڑائی = false;

	/* Increase specified left/right margins */
	اگر (box->margin[بایاں_2] != AUTO && box->margin[بایاں_2] < lm &&
			box->margin[بایاں_2] >= 0)
		box->margin[بایاں_2] = lm;
	اگر (box->margin[دایاں_2] != AUTO && box->margin[دایاں_2] < rm &&
			box->margin[دایاں_2] >= 0)
		box->margin[دایاں_2] = rm;

	/* Find width */
	اگر (چوڑائی == AUTO) {
		صحیح margin_بایاں = box->margin[بایاں_2];
		صحیح margin_دایاں = box->margin[دایاں_2];

		اگر (margin_بایاں == AUTO) {
			margin_بایاں = lm;
		}
		اگر (margin_دایاں == AUTO) {
			margin_دایاں = rm;
		}

		چوڑائی = دستیاب_چوڑائی -
				(margin_بایاں + box->border[بایاں_2].چوڑائی +
				box->padding[بایاں_2] + box->padding[دایاں_2] +
				box->border[دایاں_2].چوڑائی + margin_دایاں);
		چوڑائی = چوڑائی < 0 ? 0 : چوڑائی;
		auto_چوڑائی = true;
	}

	اگر (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی) {
		/* max-width is admissable and width exceeds max-width */
		چوڑائی = max_چوڑائی;
		auto_چوڑائی = false;
	}

	اگر (min_چوڑائی > 0 && چوڑائی < min_چوڑائی) {
		/* min-width is admissable and width is less than max-width */
		چوڑائی = min_چوڑائی;
		auto_چوڑائی = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	اگر (auto_چوڑائی) {
		/* any other 'auto' become 0 or the minimum required values */
		اگر (box->margin[بایاں_2] == AUTO) {
			box->margin[بایاں_2] = lm;
		}
		اگر (box->margin[دایاں_2] == AUTO) {
			box->margin[دایاں_2] = rm;
		}
		واپس چوڑائی;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	اگر (box->margin[بایاں_2] != AUTO && box->margin[دایاں_2] != AUTO &&
			box->والد != تہی && box->والد->style != تہی) {
		انتخاب (css_computed_متن_align(box->والد->style)) {
		حالت CSS_متن_ALIGN_LIBCSS_دایاں:
			box->margin[بایاں_2] = AUTO;
			box->margin[دایاں_2] = 0;
			توڑو;
		حالت CSS_متن_ALIGN_LIBCSS_CENTER:
			box->margin[بایاں_2] = box->margin[دایاں_2] = AUTO;
			توڑو;
		حالت CSS_متن_ALIGN_LIBCSS_بایاں:
			box->margin[بایاں_2] = 0;
			box->margin[دایاں_2] = AUTO;
			توڑو;
		طےشدہ:
			/* Leave it alone; no HTML alignment */
			توڑو;
		}
	}

	اگر (box->margin[بایاں_2] == AUTO && box->margin[دایاں_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[بایاں_2] = box->margin[دایاں_2] =
				(دستیاب_چوڑائی - lm - rm -
				(box->border[بایاں_2].چوڑائی + box->padding[بایاں_2] +
				چوڑائی + box->padding[دایاں_2] +
				box->border[دایاں_2].چوڑائی)) / 2;

		اگر (box->margin[بایاں_2] < 0) {
			box->margin[دایاں_2] += box->margin[بایاں_2];
			box->margin[بایاں_2] = 0;
		}

		box->margin[بایاں_2] += lm;

	} ورنہ اگر (box->margin[بایاں_2] == AUTO) {
		box->margin[بایاں_2] = دستیاب_چوڑائی - lm -
				(box->border[بایاں_2].چوڑائی + box->padding[بایاں_2] +
				چوڑائی + box->padding[دایاں_2] +
				box->border[دایاں_2].چوڑائی + box->margin[دایاں_2]);
		box->margin[بایاں_2] = box->margin[بایاں_2] < lm
				? lm : box->margin[بایاں_2];
	} ورنہ {
		/* margin-right auto or "over-constrained" */
		box->margin[دایاں_2] = دستیاب_چوڑائی - rm -
				(box->margin[بایاں_2] + box->border[بایاں_2].چوڑائی +
				 box->padding[بایاں_2] + چوڑائی +
				 box->padding[دایاں_2] +
				 box->border[دایاں_2].چوڑائی);
	}

	واپس چوڑائی;
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
ساکن خالی
layout_بلاک_ڈھونڈو_dimensions(ثابت css_unit_ctx *unit_len_ctx,
			     صحیح دستیاب_چوڑائی,
			     صحیح viewport_اونچائی,
			     صحیح lm,
			     صحیح rm,
			     ساخت box *box)
{
	صحیح چوڑائی, max_چوڑائی, min_چوڑائی;
	صحیح اونچائی, max_اونچائی, min_اونچائی;
	صحیح *margin = box->margin;
	صحیح *padding = box->padding;
	ساخت box_border *border = box->border;
	ثابت css_computed_style *style = box->style;

	layout_ڈھونڈو_dimensions(unit_len_ctx, دستیاب_چوڑائی, viewport_اونچائی, box,
			style, &چوڑائی, &اونچائی, &max_چوڑائی, &min_چوڑائی,
			&max_اونچائی, &min_اونچائی, margin, padding, border);

	اگر (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &چوڑائی, &اونچائی,
				min_چوڑائی, max_چوڑائی, min_اونچائی, max_اونچائی);
	}

	box->چوڑائی = layout_solve_چوڑائی(box, دستیاب_چوڑائی, چوڑائی, lm, rm,
			max_چوڑائی, min_چوڑائی);
	box->اونچائی = اونچائی;

	اگر (margin[چوٹی_2] == AUTO)
		margin[چوٹی_2] = 0;
	اگر (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
ساکن خالی layout_بلاک_جوڑو_scrollbar(ساخت box *box, صحیح which)
{
	شمار css_حد_سے_تجاوز_e حد_سے_تجاوز_x, حد_سے_تجاوز_y;

	أكد(box->type == BOX_بلاک && (which == دایاں_2 || which == BOTTOM));

	اگر (box->style == تہی)
		واپس;

	حد_سے_تجاوز_x = css_computed_حد_سے_تجاوز_x(box->style);
	حد_سے_تجاوز_y = css_computed_حد_سے_تجاوز_y(box->style);

	اگر (which == BOTTOM &&
			(حد_سے_تجاوز_x == CSS_حد_سے_تجاوز_SCROLL ||
			 حد_سے_تجاوز_x == CSS_حد_سے_تجاوز_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		اگر (box->اونچائی != AUTO &&
				(حد_سے_تجاوز_x == CSS_حد_سے_تجاوز_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_چوڑائی;
		}

	} ورنہ اگر (which == دایاں_2 &&
			(حد_سے_تجاوز_y == CSS_حد_سے_تجاوز_SCROLL ||
			 حد_سے_تجاوز_y == CSS_حد_سے_تجاوز_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		شمار css_اونچائی_e htype;
		css_fixed اونچائی = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_اونچائی(box->style, &اونچائی, &hunit);

		اگر (which == دایاں_2 && box->چوڑائی != AUTO &&
				htype == CSS_اونچائی_سیٹ &&
				(حد_سے_تجاوز_y == CSS_حد_سے_تجاوز_SCROLL ||
				box_vscrollbar_present(box))) {
			box->چوڑائی -= SCROLLBAR_چوڑائی;
			box->padding[دایاں_2] += SCROLLBAR_چوڑائی;
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
ساکن خالی layout_move_children(ساخت box *box, صحیح x, صحیح y)
{
	أكد(box);

	برائے (box = box->children; box; box = box->اگلا) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_جدول(
		ساخت box *جدول,
		صحیح دستیاب_چوڑائی,
		html_content *content)
{
	غیرعلامتی صحیح columns = جدول->columns;  /* total columns */
	غیرعلامتی صحیح i;
	غیرعلامتی صحیح *صف_وسعت;
	صحیح *excess_y;
	صحیح جدول_چوڑائی, min_چوڑائی = 0, max_چوڑائی = 0;
	صحیح required_چوڑائی = 0;
	صحیح x, remainder = 0, گنتی = 0;
	صحیح جدول_اونچائی = 0;
	صحیح min_اونچائی = 0;
	صحیح *xs;  /* array of column x positions */
	صحیح auto_چوڑائی;
	صحیح فالتو_چوڑائی;
	صحیح relative_مجموع = 0;
	صحیح border_spacing_h = 0, border_spacing_v = 0;
	صحیح فالتو_اونچائی;
	صحیح positioned_columns = 0;
	ساخت box *containing_بلاک = تہی;
	ساخت box *c;
	ساخت box *صف;
	ساخت box *صف_گروہ;
	ساخت box **صف_وسعت_cell;
	ساخت عمود *ستون;
	ثابت css_computed_style *style = جدول->style;
	شمار css_اونچائی_e htype;
	css_fixed قدر = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(جدول->type == BOX_جدول);
	أكد(style);
	أكد(جدول->children && جدول->children->children);
	أكد(columns);

	/* allocate working buffers */
	ستون = حافظہ_دو(columns * حجم ستون[0]);
	excess_y = حافظہ_دو(columns * حجم excess_y[0]);
	صف_وسعت = حافظہ_دو(columns * حجم صف_وسعت[0]);
	صف_وسعت_cell = حافظہ_دو(columns * حجم صف_وسعت_cell[0]);
	xs = حافظہ_دو((columns + 1) * حجم xs[0]);
	اگر (!ستون || !xs || !صف_وسعت || !excess_y || !صف_وسعت_cell) {
		آزاد(ستون);
		آزاد(excess_y);
		آزاد(صف_وسعت);
		آزاد(صف_وسعت_cell);
		آزاد(xs);
		واپس false;
	}

	حافظہ_نقل(ستون, جدول->ستون, حجم(ستون[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_ڈھونڈو_dimensions(&content->unit_len_ctx, دستیاب_چوڑائی, -1, جدول,
			style, 0, 0, 0, 0, 0, 0, جدول->margin, جدول->padding,
			جدول->border);
	برائے (صف_گروہ = جدول->children; صف_گروہ;
			صف_گروہ = صف_گروہ->اگلا) {
		برائے (صف = صف_گروہ->children; صف; صف = صف->اگلا) {
			برائے (c = صف->children; c; c = c->اگلا) {
				شمار css_حد_سے_تجاوز_e حد_سے_تجاوز_x;
				شمار css_حد_سے_تجاوز_e حد_سے_تجاوز_y;

				أكد(c->style);
				جدول_مستعمل_border_for_cell(
						&content->unit_len_ctx, c);
				layout_ڈھونڈو_dimensions(&content->unit_len_ctx,
						دستیاب_چوڑائی, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				حد_سے_تجاوز_x = css_computed_حد_سے_تجاوز_x(c->style);
				حد_سے_تجاوز_y = css_computed_حد_سے_تجاوز_y(c->style);

				اگر (حد_سے_تجاوز_x == CSS_حد_سے_تجاوز_SCROLL ||
						حد_سے_تجاوز_x ==
						CSS_حد_سے_تجاوز_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_چوڑائی;
				}
				اگر (حد_سے_تجاوز_y == CSS_حد_سے_تجاوز_SCROLL ||
						حد_سے_تجاوز_y ==
						CSS_حد_سے_تجاوز_AUTO) {
					c->padding[دایاں_2] += SCROLLBAR_چوڑائی;
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
	اگر (css_computed_چوڑائی_px(style, &content->unit_len_ctx,
			دستیاب_چوڑائی, &جدول_چوڑائی) == CSS_چوڑائی_سیٹ) {
		/* specified width includes border */
		جدول_چوڑائی -= جدول->border[بایاں_2].چوڑائی +
				جدول->border[دایاں_2].چوڑائی;
		جدول_چوڑائی = جدول_چوڑائی < 0 ? 0 : جدول_چوڑائی;

		auto_چوڑائی = جدول_چوڑائی;
	} ورنہ {
		جدول_چوڑائی = AUTO;
		auto_چوڑائی = دستیاب_چوڑائی -
				((جدول->margin[بایاں_2] == AUTO ? 0 :
						جدول->margin[بایاں_2]) +
				 جدول->border[بایاں_2].چوڑائی +
				 جدول->padding[بایاں_2] +
				 جدول->padding[دایاں_2] +
				 جدول->border[دایاں_2].چوڑائی +
				 (جدول->margin[دایاں_2] == AUTO ? 0 :
						جدول->margin[دایاں_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_اونچائی(style, &قدر, &unit);
	اگر (htype == CSS_اونچائی_سیٹ) {
		اگر (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			اگر (css_computed_position(جدول->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				أكد(جدول->float_container);
				containing_بلاک = جدول->float_container;
			} ورنہ اگر (جدول->float_container &&
					css_computed_position(جدول->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(جدول->style) ==
						CSS_FLOAT_بایاں ||
					css_computed_float(جدول->style) ==
						CSS_FLOAT_دایاں)) {
				/* Table is a float */
				أكد(جدول->والد && جدول->والد->والد &&
						جدول->والد->والد->والد);
				containing_بلاک =
						جدول->والد->والد->والد;
			} ورنہ اگر (جدول->والد && جدول->والد->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_بلاک = جدول->والد;
			} ورنہ اگر (جدول->والد && جدول->والد->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				أكد(جدول->والد->والد);
				containing_بلاک = جدول->والد->والد;
			}

			اگر (containing_بلاک) {
				css_fixed ignored = 0;

				htype = css_computed_اونچائی(
						containing_بلاک->style,
						&ignored, &unit);
			}

			اگر (containing_بلاک &&
					containing_بلاک->اونچائی != AUTO &&
					(css_computed_position(جدول->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_اونچائی_سیٹ)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_اونچائی = FPCT_کا_INT_TOINT(قدر,
						containing_بلاک->اونچائی);
			}
		} ورنہ {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_اونچائی = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					قدر, unit));
		}
	}

	/* calculate width required by cells */
	برائے (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      جدول,
		      i,
		      ((ثابت حرف *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[ستون[i].type],
		      ستون[i].چوڑائی,
		      ستون[i].min,
		      ستون[i].max);


		اگر (ستون[i].positioned) {
			positioned_columns++;
			جاری;
		} ورنہ اگر (ستون[i].type == عمود_چوڑائی_FIXED) {
			اگر (ستون[i].چوڑائی < ستون[i].min)
				ستون[i].چوڑائی = ستون[i].max = ستون[i].min;
			ورنہ
				ستون[i].min = ستون[i].max = ستون[i].چوڑائی;
			required_چوڑائی += ستون[i].چوڑائی;
		} ورنہ اگر (ستون[i].type == عمود_چوڑائی_PERCENT) {
			صحیح چوڑائی = ستون[i].چوڑائی * auto_چوڑائی / 100;
			required_چوڑائی += ستون[i].min < چوڑائی ? چوڑائی :
					ستون[i].min;
		} ورنہ
			required_چوڑائی += ستون[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_چوڑائی);
	}
	required_چوڑائی += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", جدول_چوڑائی,
	      جدول->min_چوڑائی, جدول->max_چوڑائی, auto_چوڑائی, required_چوڑائی);

	اگر (auto_چوڑائی < required_چوڑائی) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		برائے (i = 0; i != columns; i++) {
			اگر (ستون[i].type == عمود_چوڑائی_RELATIVE)
				جاری;
			اگر (ستون[i].type == عمود_چوڑائی_PERCENT) {
				ستون[i].max = auto_چوڑائی * ستون[i].چوڑائی / 100;
				اگر (ستون[i].max < ستون[i].min)
					ستون[i].max = ستون[i].min;
			}
			min_چوڑائی += ستون[i].min;
			max_چوڑائی += ستون[i].max;
		}
	} ورنہ {
		/* take percentages exactly */
		برائے (i = 0; i != columns; i++) {
			اگر (ستون[i].type == عمود_چوڑائی_RELATIVE)
				جاری;
			اگر (ستون[i].type == عمود_چوڑائی_PERCENT) {
				صحیح چوڑائی = auto_چوڑائی * ستون[i].چوڑائی / 100;
				اگر (چوڑائی < ستون[i].min)
					چوڑائی = ستون[i].min;
				ستون[i].min = ستون[i].چوڑائی = ستون[i].max = چوڑائی;
				ستون[i].type = عمود_چوڑائی_FIXED;
			}
			min_چوڑائی += ستون[i].min;
			max_چوڑائی += ستون[i].max;
		}
	}

	/* allocate relative widths */
	فالتو_چوڑائی = auto_چوڑائی;
	برائے (i = 0; i != columns; i++) {
		اگر (ستون[i].type == عمود_چوڑائی_RELATIVE)
			relative_مجموع += ستون[i].چوڑائی;
		ورنہ اگر (ستون[i].type == عمود_چوڑائی_FIXED)
			فالتو_چوڑائی -= ستون[i].چوڑائی;
		ورنہ
			فالتو_چوڑائی -= ستون[i].min;
	}
	فالتو_چوڑائی -= (columns + 1) * border_spacing_h;
	اگر (relative_مجموع != 0) {
		اگر (فالتو_چوڑائی < 0)
			فالتو_چوڑائی = 0;
		برائے (i = 0; i != columns; i++) {
			اگر (ستون[i].type == عمود_چوڑائی_RELATIVE) {
				ستون[i].min = ceil(ستون[i].max =
						(اعشاری) فالتو_چوڑائی
						* (اعشاری) ستون[i].چوڑائی
						/ relative_مجموع);
				min_چوڑائی += ستون[i].min;
				max_چوڑائی += ستون[i].max;
			}
		}
	}
	min_چوڑائی += (columns + 1) * border_spacing_h;
	max_چوڑائی += (columns + 1) * border_spacing_h;

	اگر (auto_چوڑائی <= min_چوڑائی) {
		/* not enough space: minimise column widths */
		برائے (i = 0; i < columns; i++) {
			ستون[i].چوڑائی = ستون[i].min;
		}
		جدول_چوڑائی = min_چوڑائی;
	} ورنہ اگر (max_چوڑائی <= auto_چوڑائی) {
		/* more space than maximum width */
		اگر (جدول_چوڑائی == AUTO) {
			/* for auto-width tables, make columns max width */
			برائے (i = 0; i < columns; i++) {
				ستون[i].چوڑائی = ستون[i].max;
			}
			جدول_چوڑائی = max_چوڑائی;
		} ورنہ {
			/* for fixed-width tables, distribute the extra space
			 * too */
			غیرعلامتی صحیح flexible_columns = 0;
			برائے (i = 0; i != columns; i++)
				اگر (ستون[i].type != عمود_چوڑائی_FIXED)
					flexible_columns++;
			اگر (flexible_columns == 0) {
				صحیح extra = (جدول_چوڑائی - max_چوڑائی) / columns;
				remainder = (جدول_چوڑائی - max_چوڑائی) -
						(extra * columns);
				برائے (i = 0; i != columns; i++) {
					ستون[i].چوڑائی = ستون[i].max + extra;
					گنتی -= remainder;
					اگر (گنتی < 0) {
						ستون[i].چوڑائی++;
						گنتی += columns;
					}
				}

			} ورنہ {
				صحیح extra = (جدول_چوڑائی - max_چوڑائی) /
						flexible_columns;
				remainder = (جدول_چوڑائی - max_چوڑائی) -
						(extra * flexible_columns);
				برائے (i = 0; i != columns; i++)
					اگر (ستون[i].type != عمود_چوڑائی_FIXED) {
						ستون[i].چوڑائی = ستون[i].max +
								extra;
						گنتی -= remainder;
						اگر (گنتی < 0) {
							ستون[i].چوڑائی++;
							گنتی += flexible_columns;
						}
					}
			}
		}
	} ورنہ {
		/* space between min and max: fill it exactly */
		اعشاری scale = (اعشاری) (auto_چوڑائی - min_چوڑائی) /
				(اعشاری) (max_چوڑائی - min_چوڑائی);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		برائے (i = 0; i < columns; i++) {
			ستون[i].چوڑائی = ستون[i].min + (صحیح) (0.5 +
					(ستون[i].max - ستون[i].min) * scale);
		}
		جدول_چوڑائی = auto_چوڑائی;
	}

	xs[0] = x = border_spacing_h;
	برائے (i = 0; i != columns; i++) {
		اگر (!ستون[i].positioned)
			x += ستون[i].چوڑائی + border_spacing_h;
		xs[i + 1] = x;
		صف_وسعت[i] = 0;
		excess_y[i] = 0;
		صف_وسعت_cell[i] = 0;
	}

	/* position cells */
	جدول_اونچائی = border_spacing_v;
	برائے (صف_گروہ = جدول->children; صف_گروہ;
			صف_گروہ = صف_گروہ->اگلا) {
		صحیح صف_گروہ_اونچائی = 0;
		برائے (صف = صف_گروہ->children; صف; صف = صف->اگلا) {
			صحیح صف_اونچائی = 0;

			htype = css_computed_اونچائی(صف->style, &قدر, &unit);
			اگر (htype == CSS_اونچائی_سیٹ && unit != CSS_UNIT_PCT) {
				صف_اونچائی = FIXTOINT(css_unit_len2device_px(
						صف->style,
						&content->unit_len_ctx,
						قدر, unit));
			}
			برائے (c = صف->children; c; c = c->اگلا) {
				أكد(c->style);
				c->چوڑائی = xs[c->آغاز_عمود + c->columns] -
						xs[c->آغاز_عمود] -
						border_spacing_h -
						c->border[بایاں_2].چوڑائی -
						c->padding[بایاں_2] -
						c->padding[دایاں_2] -
						c->border[دایاں_2].چوڑائی;
				c->float_children = 0;
				c->cached_رکھو_نیچے_level = 0;

				c->اونچائی = AUTO;
				اگر (!layout_بلاک_context(c, -1, content)) {
					آزاد(ستون);
					آزاد(excess_y);
					آزاد(صف_وسعت);
					آزاد(صف_وسعت_cell);
					آزاد(xs);
					واپس false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->اونچائی;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_اونچائی(c->style,
						&قدر, &unit);

				اگر (htype == CSS_اونچائی_سیٹ &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					صحیح h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							قدر, unit));
					اگر (c->اونچائی < h)
						c->اونچائی = h;
				}
				/* specified row height is treated as a minimum
				 */
				اگر (c->اونچائی < صف_اونچائی)
					c->اونچائی = صف_اونچائی;
				c->x = xs[c->آغاز_عمود] +
						c->border[بایاں_2].چوڑائی;
				c->y = c->border[چوٹی_2].چوڑائی;
				برائے (i = 0; i != c->columns; i++) {
					صف_وسعت[c->آغاز_عمود + i] = c->صفیں;
					excess_y[c->آغاز_عمود + i] =
							c->border[چوٹی_2].چوڑائی +
							c->padding[چوٹی_2] +
							c->اونچائی +
							c->padding[BOTTOM] +
							c->border[BOTTOM].چوڑائی;
					صف_وسعت_cell[c->آغاز_عمود + i] = 0;
				}
				صف_وسعت_cell[c->آغاز_عمود] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[چوٹی_2].چوڑائی -
						c->padding[چوٹی_2] -
						c->اونچائی -
						c->border[BOTTOM].چوڑائی;
			}
			برائے (i = 0; i != columns; i++)
				اگر (صف_وسعت[i] != 0)
					صف_وسعت[i]--;
				ورنہ
					صف_وسعت_cell[i] = 0;
			اگر (صف->اگلا || صف_گروہ->اگلا) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				برائے (i = 0; i != columns; i++)
					اگر (صف_وسعت[i] == 0 && صف_اونچائی <
							excess_y[i])
						صف_اونچائی = excess_y[i];
			} ورنہ {
				/* except in the last row */
				برائے (i = 0; i != columns; i++)
					اگر (صف_اونچائی < excess_y[i])
						صف_اونچائی = excess_y[i];
			}
			برائے (i = 0; i != columns; i++) {
				اگر (صف_اونچائی < excess_y[i])
					excess_y[i] -= صف_اونچائی;
				ورنہ
					excess_y[i] = 0;
				اگر (صف_وسعت_cell[i] != 0)
					صف_وسعت_cell[i]->padding[BOTTOM] +=
							صف_اونچائی +
							border_spacing_v;
			}

			صف->x = 0;
			صف->y = صف_گروہ_اونچائی;
			صف->چوڑائی = جدول_چوڑائی;
			صف->اونچائی = صف_اونچائی;
			صف_گروہ_اونچائی += صف_اونچائی + border_spacing_v;
		}
		صف_گروہ->x = 0;
		صف_گروہ->y = جدول_اونچائی;
		صف_گروہ->چوڑائی = جدول_چوڑائی;
		صف_گروہ->اونچائی = صف_گروہ_اونچائی;
		جدول_اونچائی += صف_گروہ_اونچائی;
	}
	/* Table height is either the height of the contents, or specified
	 * height if greater */
	جدول_اونچائی = max(جدول_اونچائی, min_اونچائی);
	/** \todo distribute spare height over the row groups / rows / cells */

	/* perform vertical alignment */
	برائے (صف_گروہ = جدول->children; صف_گروہ;
			صف_گروہ = صف_گروہ->اگلا) {
		برائے (صف = صف_گروہ->children; صف; صف = صف->اگلا) {
			برائے (c = صف->children; c; c = c->اگلا) {
				شمار css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				فالتو_اونچائی = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->اونچائی - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &قدر, &unit);

				انتخاب (vertical_align) {
				حالت CSS_VERTICAL_ALIGN_SUB:
				حالت CSS_VERTICAL_ALIGN_SUPER:
				حالت CSS_VERTICAL_ALIGN_متن_چوٹی:
				حالت CSS_VERTICAL_ALIGN_متن_BOTTOM:
				حالت CSS_VERTICAL_ALIGN_سیٹ:
				حالت CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				حالت CSS_VERTICAL_ALIGN_چوٹی:
					توڑو;
				حالت CSS_VERTICAL_ALIGN_درمیان:
					c->padding[چوٹی_2] += فالتو_اونچائی / 2;
					c->padding[BOTTOM] -= فالتو_اونچائی / 2;
					layout_move_children(c, 0,
							فالتو_اونچائی / 2);
					توڑو;
				حالت CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[چوٹی_2] += فالتو_اونچائی;
					c->padding[BOTTOM] -= فالتو_اونچائی;
					layout_move_children(c, 0,
							فالتو_اونچائی);
					توڑو;
				حالت CSS_VERTICAL_ALIGN_INHERIT:
					أكد(0);
					توڑو;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	اگر (جدول->margin[چوٹی_2] == AUTO)
		جدول->margin[چوٹی_2] = 0;
	اگر (جدول->margin[BOTTOM] == AUTO)
		جدول->margin[BOTTOM] = 0;

	آزاد(ستون);
	آزاد(excess_y);
	آزاد(صف_وسعت);
	آزاد(صف_وسعت_cell);
	آزاد(xs);

	جدول->چوڑائی = جدول_چوڑائی;
	جدول->اونچائی = جدول_اونچائی;

	واپس true;
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
ساکن bool layout_apply_minmax_اونچائی(
		ثابت css_unit_ctx *unit_len_ctx,
		ساخت box *box,
		ساخت box *container)
{
	صحیح h;
	ساخت box *containing_بلاک = تہی;
	bool updated = false;

	/* Find containing block for percentage heights */
	اگر (box->style != تہی && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		أكد(container);
		containing_بلاک = container;
	} ورنہ اگر (box->float_container && box->style != تہی &&
			(css_computed_float(box->style) == CSS_FLOAT_بایاں ||
			 css_computed_float(box->style) == CSS_FLOAT_دایاں)) {
		/* Box is a float */
		أكد(box->والد && box->والد->والد &&
				box->والد->والد->والد);
		containing_بلاک = box->والد->والد->والد;
	} ورنہ اگر (box->والد && box->والد->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_بلاک = box->والد;
	} ورنہ اگر (box->والد && box->والد->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		أكد(box->والد->والد);
		containing_بلاک = box->والد->والد;
	}

	اگر (box->style) {
		شمار css_اونچائی_e htype = CSS_اونچائی_AUTO;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		اگر (containing_بلاک) {
			htype = css_computed_اونچائی(containing_بلاک->style,
					&قدر, &unit);
		}

		/* max-height */
		اگر (css_computed_max_اونچائی(box->style, &قدر, &unit) ==
				CSS_MAX_اونچائی_سیٹ) {
			اگر (unit == CSS_UNIT_PCT) {
				اگر (containing_بلاک &&
					containing_بلاک->اونچائی != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_اونچائی_سیٹ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_کا_INT_TOINT(قدر,
						containing_بلاک->اونچائی);
					اگر (h < box->اونچائی) {
						box->اونچائی = h;
						updated = true;
					}
				}
			} ورنہ {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						قدر, unit));
				اگر (h < box->اونچائی) {
					box->اونچائی = h;
					updated = true;
				}
			}
		}

		/* min-height */
		اگر (ns_computed_min_اونچائی(box->style, &قدر, &unit) ==
				CSS_MIN_اونچائی_سیٹ) {
			اگر (unit == CSS_UNIT_PCT) {
				اگر (containing_بلاک &&
					containing_بلاک->اونچائی != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_اونچائی_سیٹ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_کا_INT_TOINT(قدر,
						containing_بلاک->اونچائی);
					اگر (h > box->اونچائی) {
						box->اونچائی = h;
						updated = true;
					}
				}
			} ورنہ {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						قدر, unit));
				اگر (h > box->اونچائی) {
					box->اونچائی = h;
					updated = true;
				}
			}
		}
	}
	واپس updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
ساکن bool layout_بلاک_object(ساخت box *بلاک)
{
	أكد(بلاک);
	أكد(بلاک->type == BOX_بلاک ||
			بلاک->type == BOX_FLEX ||
			بلاک->type == BOX_INLINE_بلاک ||
			بلاک->type == BOX_INLINE_FLEX ||
			بلاک->type == BOX_جدول ||
			بلاک->type == BOX_جدول_CELL);
	أكد(بلاک->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", بلاک,
	      hlcache_handle_get_url(بلاک->object), بلاک->چوڑائی);

	اگر (content_can_reformat(بلاک->object)) {
		content_reformat(بلاک->object, false, بلاک->چوڑائی, 1);
	} ورنہ {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	واپس true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
ساکن خالی جوڑو_float_مقصد_container(ساخت box *cont, ساخت box *b)
{
	ساخت box *box = cont->float_children;
	صحیح b_bottom = b->y + b->اونچائی;

	أكد(b->type == BOX_FLOAT_بایاں || b->type == BOX_FLOAT_دایاں);

	اگر (box == تہی) {
		/* No other float children */
		b->اگلا_float = تہی;
		cont->float_children = b;
		واپس;
	} ورنہ اگر (b_bottom >= box->y + box->اونچائی) {
		/* Goes at start of list */
		b->اگلا_float = cont->float_children;
		cont->float_children = b;
	} ورنہ {
		ساخت box *پچھلا = تہی;
		جب (box != تہی && b_bottom < box->y + box->اونچائی) {
			پچھلا = box;
			box = box->اگلا_float;
		}
		اگر (پچھلا != تہی) {
			b->اگلا_float = پچھلا->اگلا_float;
			پچھلا->اگلا_float = b;
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
ساکن bool
layout_متن_box_بانٹو(html_content *content,
		      plot_font_style_t *fstyle,
		      ساخت box *بانٹو_box,
		      size_t new_لمبائی,
		      صحیح new_چوڑائی)
{
	صحیح space_چوڑائی = بانٹو_box->space;
	ساخت box *c2;
	ثابت ساخت gui_layout_جدول *font_func = content->font_func;
	bool space = (بانٹو_box->متن[new_لمبائی] == ' ');
	صحیح مستعمل_لمبائی = new_لمبائی + (space ? 1 : 0);

	اگر ((space && space_چوڑائی == 0) || space_چوڑائی == UNKNOWN_چوڑائی) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->چوڑائی(fstyle, " ", 1, &space_چوڑائی);
	}

	اگر (بانٹو_box->space == UNKNOWN_چوڑائی)
		بانٹو_box->space = space_چوڑائی;
	اگر (!space)
		space_چوڑائی = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, بانٹو_box, حجم *c2);
	اگر (!c2)
		واپس false;
	c2->flags |= CLONE;

	/* Set remaining text in c2 */
	c2->متن += مستعمل_لمبائی;

	/* Set c2 according to the remaining text */
	c2->چوڑائی -= new_چوڑائی + space_چوڑائی;
	c2->flags &= ~MEASURED; /* width has been estimated */
	c2->لمبائی = بانٹو_box->لمبائی - مستعمل_لمبائی;

	/* Update split_box for its reduced text */
	بانٹو_box->چوڑائی = new_چوڑائی;
	بانٹو_box->flags |= MEASURED;
	بانٹو_box->لمبائی = new_لمبائی;
	بانٹو_box->space = space_چوڑائی;

	/* Insert c2 into box list */
	c2->اگلا = بانٹو_box->اگلا;
	بانٹو_box->اگلا = c2;
	c2->پچھلا = بانٹو_box;
	اگر (c2->اگلا)
		c2->اگلا->پچھلا = c2;
	ورنہ
		c2->والد->آخری = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      بانٹو_box,
	      بانٹو_box->لمبائی,
	      (صحیح)بانٹو_box->لمبائی,
	      بانٹو_box->متن);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->لمبائی,
	      (صحیح)c2->لمبائی,
	      c2->متن);

	واپس true;
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
ساکن خالی
layout_float_ڈھونڈو_dimensions(
		ثابت css_unit_ctx *unit_len_ctx,
		صحیح دستیاب_چوڑائی,
		ثابت css_computed_style *style,
		ساخت box *box)
{
	صحیح چوڑائی, اونچائی, max_چوڑائی, min_چوڑائی, max_اونچائی, min_اونچائی;
	صحیح *margin = box->margin;
	صحیح *padding = box->padding;
	ساخت box_border *border = box->border;
	شمار css_حد_سے_تجاوز_e حد_سے_تجاوز_x = css_computed_حد_سے_تجاوز_x(style);
	شمار css_حد_سے_تجاوز_e حد_سے_تجاوز_y = css_computed_حد_سے_تجاوز_y(style);
	صحیح scrollbar_چوڑائی_x =
			(حد_سے_تجاوز_x == CSS_حد_سے_تجاوز_SCROLL ||
			 حد_سے_تجاوز_x == CSS_حد_سے_تجاوز_AUTO) ?
			SCROLLBAR_چوڑائی : 0;
	صحیح scrollbar_چوڑائی_y =
			(حد_سے_تجاوز_y == CSS_حد_سے_تجاوز_SCROLL ||
			 حد_سے_تجاوز_y == CSS_حد_سے_تجاوز_AUTO) ?
			SCROLLBAR_چوڑائی : 0;

	layout_ڈھونڈو_dimensions(unit_len_ctx, دستیاب_چوڑائی, -1, box, style,
			&چوڑائی, &اونچائی, &max_چوڑائی, &min_چوڑائی,
			&max_اونچائی, &min_اونچائی, margin, padding, border);

	اگر (margin[بایاں_2] == AUTO)
		margin[بایاں_2] = 0;
	اگر (margin[دایاں_2] == AUTO)
		margin[دایاں_2] = 0;

	اگر (box->gadget == تہی) {
		padding[دایاں_2] += scrollbar_چوڑائی_y;
		padding[BOTTOM] += scrollbar_چوڑائی_x;
	}

	اگر (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &چوڑائی, &اونچائی,
				min_چوڑائی, max_چوڑائی, min_اونچائی, max_اونچائی);
	} ورنہ اگر (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed سائز = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		أكد(box->style);

		اگر (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			اگر (چوڑائی == AUTO) {
				سائز = INTTOFIX(10);
				چوڑائی = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						سائز, unit));
			}
			اگر (box->gadget->type == GADGET_FILE &&
					اونچائی == AUTO) {
				سائز = FLTTOFIX(1.5);
				اونچائی = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						سائز, unit));
			}
		}
		اگر (box->gadget->type == GADGET_TEXTAREA) {
			اگر (چوڑائی == AUTO) {
				سائز = INTTOFIX(10);
				چوڑائی = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						سائز, unit));
			}
			اگر (اونچائی == AUTO) {
				سائز = INTTOFIX(4);
				اونچائی = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						سائز, unit));
			}
		}
	} ورنہ اگر (چوڑائی == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		چوڑائی = min(max(box->min_چوڑائی, دستیاب_چوڑائی),
				box->max_چوڑائی);

		/* width includes margin, borders and padding */
		اگر (چوڑائی == دستیاب_چوڑائی) {
			چوڑائی -= box->margin[بایاں_2] + box->border[بایاں_2].چوڑائی +
					box->padding[بایاں_2] +
					box->padding[دایاں_2] +
					box->border[دایاں_2].چوڑائی +
					box->margin[دایاں_2];
		} ورنہ {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			صحیح fixed = 0;
			اعشاری frac = 0;
			calculate_mbp_چوڑائی(unit_len_ctx, box->style, بایاں_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_چوڑائی(unit_len_ctx, box->style, دایاں_2,
					true, true, true, &fixed, &frac);
			اگر (fixed < 0)
				fixed = 0;

			چوڑائی -= fixed;
		}

		اگر (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی) چوڑائی = max_چوڑائی;
		اگر (min_چوڑائی >  0 && چوڑائی < min_چوڑائی) چوڑائی = min_چوڑائی;

	} ورنہ {
		اگر (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی) چوڑائی = max_چوڑائی;
		اگر (min_چوڑائی >  0 && چوڑائی < min_چوڑائی) چوڑائی = min_چوڑائی;
		چوڑائی -= scrollbar_چوڑائی_y;
	}

	box->چوڑائی = چوڑائی;
	box->اونچائی = اونچائی;

	اگر (margin[چوٹی_2] == AUTO)
		margin[چوٹی_2] = 0;
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
ساکن bool layout_float(ساخت box *b, صحیح چوڑائی, html_content *content)
{
	أكد(b->type == BOX_جدول ||
	       b->type == BOX_بلاک ||
	       b->type == BOX_INLINE_بلاک ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_ڈھونڈو_dimensions(&content->unit_len_ctx, چوڑائی, b->style, b);
	اگر (b->type == BOX_جدول || b->type == BOX_INLINE_FLEX) {
		اگر (b->type == BOX_جدول) {
			اگر (!layout_جدول(b, چوڑائی, content))
				واپس false;
		} ورنہ {
			اگر (!layout_flex(b, چوڑائی, content))
				واپس false;
		}
		اگر (b->margin[بایاں_2] == AUTO)
			b->margin[بایاں_2] = 0;
		اگر (b->margin[دایاں_2] == AUTO)
			b->margin[دایاں_2] = 0;
		اگر (b->margin[چوٹی_2] == AUTO)
			b->margin[چوٹی_2] = 0;
		اگر (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} ورنہ {
		واپس layout_بلاک_context(b, -1, content);
	}
	واپس true;
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
ساکن خالی
رکھو_float_نیچے(ساخت box *c, صحیح چوڑائی, صحیح cx, صحیح y, ساخت box *cont)
{
	صحیح x0, x1, yy;
	ساخت box *بایاں;
	ساخت box *دایاں;

	yy = y > cont->cached_رکھو_نیچے_level ?
			y : cont->cached_رکھو_نیچے_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      چوڑائی, cx, y, cont);

	کرو {
		y = yy;
		x0 = cx;
		x1 = cx + چوڑائی;
		ڈھونڈو_sides(cont->float_children, y, y + c->اونچائی, &x0, &x1,
				&بایاں, &دایاں);
		اگر (بایاں != 0 && دایاں != 0) {
			yy = (بایاں->y + بایاں->اونچائی <
					دایاں->y + دایاں->اونچائی ?
					بایاں->y + بایاں->اونچائی :
					دایاں->y + دایاں->اونچائی);
		} ورنہ اگر (بایاں == 0 && دایاں != 0) {
			yy = دایاں->y + دایاں->اونچائی;
		} ورنہ اگر (بایاں != 0 && دایاں == 0) {
			yy = بایاں->y + بایاں->اونچائی;
		}
	} جب ((بایاں != 0 || دایاں != 0) && (c->چوڑائی > x1 - x0));

	اگر (c->type == BOX_FLOAT_بایاں) {
		c->x = x0;
	} ورنہ {
		c->x = x1 - c->چوڑائی;
	}
	c->y = y;
	cont->cached_رکھو_نیچے_level = y;
}


/**
 * Calculate line height from a style.
 */
ساکن صحیح سطر_اونچائی(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style)
{
	شمار css_سطر_اونچائی_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed سطر_اونچائی;

	أكد(style);

	lhtype = css_computed_سطر_اونچائی(style, &lhvalue, &lhunit);
	اگر (lhtype == CSS_سطر_اونچائی_معمول) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_سطر_اونچائی_نمبر;
	}

	اگر (lhtype == CSS_سطر_اونچائی_نمبر ||
			lhunit == CSS_UNIT_PCT) {
		سطر_اونچائی = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		اگر (lhtype != CSS_سطر_اونچائی_نمبر)
			سطر_اونچائی = FDIV(سطر_اونچائی, F_100);
	} ورنہ {
		أكد(lhunit != CSS_UNIT_PCT);

		سطر_اونچائی = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	واپس FIXTOINT(سطر_اونچائی);
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
ساکن bool
layout_سطر(ساخت box *پہلا,
	    صحیح *چوڑائی,
	    صحیح *y,
	    صحیح cx,
	    صحیح cy,
	    ساخت box *cont,
	    bool indent,
	    bool has_متن_children,
	    html_content *content,
	    ساخت box **اگلا_box)
{
	صحیح اونچائی, مستعمل_اونچائی;
	صحیح x0 = 0;
	صحیح x1 = *چوڑائی;
	صحیح x, h, x_پچھلا;
	صحیح fy = cy;
	ساخت box *بایاں;
	ساخت box *دایاں;
	ساخت box *b;
	ساخت box *بانٹو_box = 0;
	ساخت box *d;
	ساخت box *br_box = 0;
	bool move_y = false;
	bool رکھو_نیچے = false;
	صحیح space_before = 0, space_after = 0;
	غیرعلامتی صحیح inline_گنتی = 0;
	غیرعلامتی صحیح i;
	ثابت ساخت gui_layout_جدول *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      پہلا,
	      (صحیح)پہلا->لمبائی,
	      پہلا->متن,
	      *چوڑائی,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	ڈھونڈو_sides(cont->float_children, cy, cy, &x0, &x1, &بایاں, &دایاں);
	x0 -= cx;
	x1 -= cx;

	اگر (indent)
		x0 += layout_متن_indent(&content->unit_len_ctx,
				پہلا->والد->والد->style, *چوڑائی);

	اگر (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	اگر (has_متن_children || پہلا->والد->والد->gadget)
		مستعمل_اونچائی = اونچائی = سطر_اونچائی(&content->unit_len_ctx,
				پہلا->والد->والد->style);
	ورنہ
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		مستعمل_اونچائی = اونچائی = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	برائے (x = 0, b = پہلا; x <= x1 - x0 && b != 0; b = b->اگلا) {
		صحیح min_چوڑائی, max_چوڑائی, min_اونچائی, max_اونچائی;

		أكد(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		اگر (b->type == BOX_BR)
			توڑو;

		اگر (lh__box_is_float_box(b))
			جاری;
		اگر (b->type == BOX_INLINE_بلاک &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			جاری;

		أكد(b->style != تہی);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		اگر (b->type == BOX_INLINE_بلاک ||
		    b->type == BOX_INLINE_FLEX) {
			اگر (b->max_چوڑائی != UNKNOWN_چوڑائی)
				اگر (!layout_float(b, *چوڑائی, content))
					واپس false;
			h = b->border[چوٹی_2].چوڑائی + b->padding[چوٹی_2] + b->اونچائی +
					b->padding[BOTTOM] +
					b->border[BOTTOM].چوڑائی;
			اگر (اونچائی < h)
				اونچائی = h;
			x += b->margin[بایاں_2] + b->border[بایاں_2].چوڑائی +
					b->padding[بایاں_2] + b->چوڑائی +
					b->padding[دایاں_2] +
					b->border[دایاں_2].چوڑائی +
					b->margin[دایاں_2];
			space_after = 0;
			جاری;
		}

		اگر (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_ڈھونڈو_dimensions(&content->unit_len_ctx,
					*چوڑائی, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			برائے (i = 0; i != 4; i++)
				اگر (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[بایاں_2] + b->border[بایاں_2].چوڑائی +
					b->padding[بایاں_2];
			اگر (b->inline_انتہا) {
				b->inline_انتہا->margin[دایاں_2] = b->margin[دایاں_2];
				b->inline_انتہا->padding[دایاں_2] =
						b->padding[دایاں_2];
				b->inline_انتہا->border[دایاں_2] =
						b->border[دایاں_2];
			} ورنہ {
				x += b->padding[دایاں_2] +
						b->border[دایاں_2].چوڑائی +
						b->margin[دایاں_2];
			}
		} ورنہ اگر (b->type == BOX_INLINE_انتہا) {
			b->چوڑائی = 0;
			اگر (b->space == UNKNOWN_چوڑائی) {
				font_func->چوڑائی(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[دایاں_2] + b->border[دایاں_2].چوڑائی +
					b->margin[دایاں_2];
			جاری;
		}

		اگر (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->اونچائی = سطر_اونچائی(&content->unit_len_ctx,
					b->style ? b->style :
					b->والد->والد->style);
			اگر (اونچائی < b->اونچائی)
				اونچائی = b->اونچائی;

			اگر (!b->متن) {
				b->چوڑائی = 0;
				space_after = 0;
				جاری;
			}

			اگر (b->چوڑائی == UNKNOWN_چوڑائی) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				اگر (b->والد->والد->gadget &&
						b->والد->والد->gadget->type
						== GADGET_SELECT) {
					صحیح opt_maxwidth = 0;
					ساخت form_option *o;

					برائے (o = b->والد->والد->gadget->
							ڈیٹا.select.اشیاء; o;
							o = o->اگلا) {
						صحیح opt_چوڑائی;
						font_func->چوڑائی(&fstyle,
								o->متن,
								لڑی_لمبائی(o->متن),
								&opt_چوڑائی);

						اگر (opt_maxwidth < opt_چوڑائی)
							opt_maxwidth =opt_چوڑائی;
					}
					b->چوڑائی = opt_maxwidth;
					اگر (nsoption_bool(core_select_menu))
						b->چوڑائی += SCROLLBAR_چوڑائی;
				} ورنہ {
					font_func->چوڑائی(&fstyle, b->متن,
							b->لمبائی, &b->چوڑائی);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			اگر (b->متن && (x + b->چوڑائی < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->اگلا) {
				font_func->چوڑائی(&fstyle, b->متن,
						 b->لمبائی, &b->چوڑائی);
				b->flags |= MEASURED;
			}

			x += b->چوڑائی;
			اگر (b->space == UNKNOWN_چوڑائی) {
				font_func->چوڑائی(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			جاری;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		layout_ڈھونڈو_dimensions(&content->unit_len_ctx,
				*چوڑائی, -1, b, b->style,
				&b->چوڑائی, &b->اونچائی,
				&max_چوڑائی, &min_چوڑائی,
				&max_اونچائی, &min_اونچائی,
				تہی, تہی, تہی);

		اگر (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->چوڑائی, &b->اونچائی,
					min_چوڑائی, max_چوڑائی,
					min_اونچائی, max_اونچائی);
		} ورنہ اگر (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			اگر (b->چوڑائی == AUTO)
				b->چوڑائی = 400;
			اگر (b->اونچائی == AUTO)
				b->اونچائی = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} ورنہ {
			/* form control with no object */
			اگر (b->چوڑائی == AUTO)
				b->چوڑائی = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			اگر (b->اونچائی == AUTO)
				b->اونچائی = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		اگر (b->object && content_can_reformat(b->object) &&
				b->چوڑائی !=
				content_get_دستیاب_چوڑائی(b->object)) {
			css_fixed قدر = 0;
			css_unit unit = CSS_UNIT_PX;
			شمار css_اونچائی_e htype = css_computed_اونچائی(b->style,
					&قدر, &unit);

			content_reformat(b->object, false, b->چوڑائی, b->اونچائی);

			اگر (htype == CSS_اونچائی_AUTO)
				b->اونچائی = content_get_اونچائی(b->object);
		}

		اگر (اونچائی < b->اونچائی)
			اونچائی = b->اونچائی;

		x += b->چوڑائی;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *چوڑائی;
	ڈھونڈو_sides(cont->float_children, cy, cy + اونچائی, &x0, &x1,
			&بایاں, &دایاں);
	x0 -= cx;
	x1 -= cx;

	اگر (indent)
		x0 += layout_متن_indent(&content->unit_len_ctx,
				پہلا->والد->والد->style, *چوڑائی);

	اگر (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	برائے (x = x_پچھلا = 0, b = پہلا; x <= x1 - x0 && b; b = b->اگلا) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		اگر (b->type == BOX_INLINE_بلاک &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} ورنہ اگر (lh__box_is_inline_flow(b)) {
			أكد(b->چوڑائی != UNKNOWN_چوڑائی);

			x_پچھلا = x;
			x += space_after;
			b->x = x;

			اگر ((b->type == BOX_INLINE && !b->inline_انتہا) ||
					b->type == BOX_INLINE_بلاک ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[بایاں_2] + b->border[بایاں_2].چوڑائی;
				x = b->x + b->padding[بایاں_2] + b->چوڑائی +
						b->padding[دایاں_2] +
						b->border[دایاں_2].چوڑائی +
						b->margin[دایاں_2];
			} ورنہ اگر (b->type == BOX_INLINE) {
				b->x += b->margin[بایاں_2] + b->border[بایاں_2].چوڑائی;
				x = b->x + b->padding[بایاں_2] + b->چوڑائی;
			} ورنہ اگر (b->type == BOX_INLINE_انتہا) {
				b->اونچائی = b->inline_انتہا->اونچائی;
				x += b->padding[دایاں_2] +
						b->border[دایاں_2].چوڑائی +
						b->margin[دایاں_2];
			} ورنہ {
				x += b->چوڑائی;
			}

			space_before = space_after;
			اگر (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			ورنہ اگر (b->متن || b->type == BOX_INLINE_انتہا) {
				اگر (b->space == UNKNOWN_چوڑائی) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->چوڑائی(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} ورنہ {
				space_after = 0;
			}
			بانٹو_box = b;
			move_y = true;
			inline_گنتی++;
		} ورنہ اگر (b->type == BOX_BR) {
			b->x = x;
			b->چوڑائی = 0;
			br_box = b;
			b = b->اگلا;
			بانٹو_box = 0;
			move_y = true;
			توڑو;

		} ورنہ {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_رکھو_نیچے_level = 0;
			b->float_container = d->float_container = cont;

			اگر (!layout_float(d, *چوڑائی, content))
				واپس false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[چوٹی_2],
			      d->border[چوٹی_2].چوڑائی);

			d->x = d->margin[بایاں_2] + d->border[بایاں_2].چوڑائی;
			d->y = d->margin[چوٹی_2] + d->border[چوٹی_2].چوڑائی;
			b->چوڑائی = d->margin[بایاں_2] + d->border[بایاں_2].چوڑائی +
					d->padding[بایاں_2] + d->چوڑائی +
					d->padding[دایاں_2] +
					d->border[دایاں_2].چوڑائی +
					d->margin[دایاں_2];
			b->اونچائی = d->margin[چوٹی_2] + d->border[چوٹی_2].چوڑائی +
					d->padding[چوٹی_2] + d->اونچائی +
					d->padding[BOTTOM] +
					d->border[BOTTOM].چوڑائی +
					d->margin[BOTTOM];

			اگر (b->چوڑائی > (x1 - x0) - x)
				رکھو_نیچے = true;
			اگر (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_بایاں && بایاں == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_دایاں &&
						دایاں == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						بایاں == 0 && دایاں == 0)) &&
					(!رکھو_نیچے ||
					(بایاں == 0 && دایاں == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_رکھو_نیچے_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				اگر (b->type == BOX_FLOAT_بایاں) {
					b->x = cx + x0;
					اگر (b->چوڑائی > 0)
						x0 += b->چوڑائی;
					بایاں = b;
				} ورنہ {
					b->x = cx + x1 - b->چوڑائی;
					اگر (b->چوڑائی > 0)
						x1 -= b->چوڑائی;
					دایاں = b;
				}
				b->y = cy;
			} ورنہ {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				صحیح fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_رکھو_نیچے_level) ?
						fcy :
						cont->cached_رکھو_نیچے_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + اونچائی : fy;

				رکھو_float_نیچے(b, *چوڑائی, cx, fy, cont);
				fy = b->y;
				اگر (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_بایاں && بایاں != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_دایاں &&
						دایاں != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(بایاں != 0 || دایاں != 0)))) {
					/* to be cleared below existing
					 * floats */
					اگر (b->type == BOX_FLOAT_بایاں)
						b->x = cx;
					ورنہ
						b->x = cx + *چوڑائی - b->چوڑائی;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					اگر (fcy > cont->clear_level)
						cont->clear_level = fcy;
					اگر (b->y < fcy)
						b->y = fcy;
				}
				اگر (b->type == BOX_FLOAT_بایاں)
					بایاں = b;
				ورنہ
					دایاں = b;
			}
			جوڑو_float_مقصد_container(cont, b);

			بانٹو_box = 0;
		}
	}

	اگر (x1 - x0 < x && بانٹو_box) {
		/* the last box went over the end */
		size_t بانٹو = 0;
		صحیح w;
		bool no_wrap = css_computed_white_space(
				بانٹو_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				بانٹو_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_پچھلا;

		اگر (!no_wrap &&
		    (بانٹو_box->type == BOX_INLINE ||
		     بانٹو_box->type == BOX_متن) &&
		    !بانٹو_box->object &&
		    !(بانٹو_box->flags & REPLACE_DIM) &&
		    !(بانٹو_box->flags & IFRAME) &&
		    !بانٹو_box->gadget && بانٹو_box->متن) {

			font_plot_style_from_css(&content->unit_len_ctx,
					بانٹو_box->style, &fstyle);
			/** \todo handle errors */
			font_func->بانٹو(&fstyle,
					 بانٹو_box->متن,
					 بانٹو_box->لمبائی,
					 x1 - x0 - x - space_before,
					 &بانٹو,
					 &w);
		}

		/* split == 0 implies that text can't be split */

		اگر (بانٹو == 0)
			w = بانٹو_box->چوڑائی;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      بانٹو_box,
		      (صحیح)بانٹو_box->لمبائی,
		      بانٹو_box->متن,
		      بانٹو,
		      w,
		      بایاں,
		      دایاں,
		      inline_گنتی);

		اگر ((بانٹو == 0 || x1 - x0 <= x + space_before + w) &&
				!بایاں && !دایاں && inline_گنتی == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			اگر (بانٹو == 0 || بانٹو == بانٹو_box->لمبائی) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = بانٹو_box->اگلا;
			} ورنہ {
				/* cut off first word for this line */
				اگر (!layout_متن_box_بانٹو(content, &fstyle,
						بانٹو_box, بانٹو, w))
					واپس false;
				b = بانٹو_box->اگلا;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} ورنہ اگر ((بانٹو == 0 || x1 - x0 <= x + space_before + w) &&
				inline_گنتی == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			أكد(بایاں || دایاں);
			مستعمل_اونچائی = 0;
			اگر (بایاں) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      بایاں->y,
				      بایاں->اونچائی);

				مستعمل_اونچائی = بایاں->y + بایاں->اونچائی - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      مستعمل_اونچائی);

			}
			اگر (دایاں && مستعمل_اونچائی <
					دایاں->y + دایاں->اونچائی - cy + 1)
				مستعمل_اونچائی = دایاں->y + دایاں->اونچائی - cy + 1;

			اگر (مستعمل_اونچائی < 0)
				مستعمل_اونچائی = 0;

			b = بانٹو_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} ورنہ اگر (بانٹو == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = بانٹو_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} ورنہ {
			/* fit as many words as possible */
			أكد(بانٹو != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (صحیح)بانٹو_box->لمبائی, بانٹو_box->متن,
			      x1 - x0, بانٹو, w);

			اگر (بانٹو != بانٹو_box->لمبائی) {
				اگر (!layout_متن_box_بانٹو(content, &fstyle,
						بانٹو_box, بانٹو, w))
					واپس false;
				b = بانٹو_box->اگلا;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	انتخاب (css_computed_متن_align(پہلا->والد->والد->style)) {
	حالت CSS_متن_ALIGN_دایاں:
	حالت CSS_متن_ALIGN_LIBCSS_دایاں:
		x0 = x1 - x;
		توڑو;
	حالت CSS_متن_ALIGN_CENTER:
	حالت CSS_متن_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		توڑو;
	حالت CSS_متن_ALIGN_بایاں:
	حالت CSS_متن_ALIGN_LIBCSS_بایاں:
	حالت CSS_متن_ALIGN_JUSTIFY:
		/* leave on left */
		توڑو;
	حالت CSS_متن_ALIGN_DEFAULT:
		/* None; consider text direction */
		انتخاب (css_computed_direction(پہلا->والد->والد->style)) {
		حالت CSS_DIRECTION_LTR:
			/* leave on left */
			توڑو;
		حالت CSS_DIRECTION_RTL:
			x0 = x1 - x;
			توڑو;
		}
		توڑو;
	}

	برائے (d = پہلا; d != b; d = d->اگلا) {
		d->flags &= ~NEW_سطر;

		اگر (d->type == BOX_INLINE_بلاک &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			جاری;
		} ورنہ اگر ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_متن ||
				d->type == BOX_INLINE_انتہا) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[چوٹی_2];

			اگر (d->type == BOX_متن && d->اونچائی > مستعمل_اونچائی) {
				/*متن*/
				مستعمل_اونچائی = d->اونچائی;
			}
		} ورنہ اگر ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_بلاک) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[چوٹی_2].چوڑائی + d->margin[چوٹی_2];
			h = d->margin[چوٹی_2] + d->border[چوٹی_2].چوڑائی +
					d->padding[چوٹی_2] + d->اونچائی +
					d->padding[BOTTOM] +
					d->border[BOTTOM].چوڑائی +
					d->margin[BOTTOM];
			اگر (مستعمل_اونچائی < h)
				مستعمل_اونچائی = h;
		}
	}

	پہلا->flags |= NEW_سطر;

	أكد(b != پہلا || (move_y && 0 < مستعمل_اونچائی && (بایاں || دایاں)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	برائے (d = پہلا; d != b; d = d->اگلا) {
		اگر ((d->type == BOX_INLINE && d->inline_انتہا) ||
				d->type == BOX_BR ||
				d->type == BOX_متن ||
				d->type == BOX_INLINE_انتہا) {
			css_fixed قدر = 0;
			css_unit unit = CSS_UNIT_PX;
			انتخاب (css_computed_vertical_align(d->style, &قدر,
					&unit)) {
			حالت CSS_VERTICAL_ALIGN_SUPER:
			حالت CSS_VERTICAL_ALIGN_چوٹی:
			حالت CSS_VERTICAL_ALIGN_متن_چوٹی:
				/* already at top */
				توڑو;
			حالت CSS_VERTICAL_ALIGN_SUB:
			حالت CSS_VERTICAL_ALIGN_BOTTOM:
			حالت CSS_VERTICAL_ALIGN_متن_BOTTOM:
				d->y += مستعمل_اونچائی - d->اونچائی;
				توڑو;
			طےشدہ:
			حالت CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (مستعمل_اونچائی - d->اونچائی);
				توڑو;
			}
		}
	}

	/* handle clearance for br */
	اگر (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		صحیح clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		اگر (مستعمل_اونچائی < clear_y - cy)
			مستعمل_اونچائی = clear_y - cy;
	}

	اگر (move_y)
		*y += مستعمل_اونچائی;
	*اگلا_box = b;
	*چوڑائی = x; /* return actual width */
	واپس true;
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
ساکن bool layout_inline_container(ساخت box *inline_container, صحیح چوڑائی,
		ساخت box *cont, صحیح cx, صحیح cy, html_content *content)
{
	bool پہلا_سطر = true;
	bool has_متن_children;
	ساخت box *c, *اگلا;
	صحیح y = 0;
	صحیح curwidth,maxwidth = چوڑائی;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      چوڑائی,
	      cont,
	      cx,
	      cy);


	has_متن_children = false;
	برائے (c = inline_container->children; c; c = c->اگلا) {
		bool is_pre = false;

		اگر (c->style) {
			شمار css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_سطر ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		اگر ((lh__box_is_object(c) == false &&
				c->متن && (c->لمبائی || is_pre)) ||
				c->type == BOX_BR)
			has_متن_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	برائے (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->چوڑائی;
		اگر (!layout_سطر(c, &curwidth, &y, cx, cy + y, cont, پہلا_سطر,
				has_متن_children, content, &اگلا))
			واپس false;
		maxwidth = max(maxwidth,curwidth);
		c = اگلا;
		پہلا_سطر = false;
	}

	inline_container->چوڑائی = maxwidth;
	inline_container->اونچائی = y;

	واپس true;
}


/* Documented in layout_intertnal.h */
bool layout_بلاک_context(
		ساخت box *بلاک,
		صحیح viewport_اونچائی,
		html_content *content)
{
	ساخت box *box;
	صحیح cx, cy;  /**< current coordinates */
	صحیح max_pos_margin = 0;
	صحیح max_neg_margin = 0;
	صحیح y = 0;
	صحیح lm, rm;
	ساخت box *margin_collapse = تہی;
	bool اندر_margin = false;
	css_fixed gadget_سائز;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	أكد(بلاک->type == BOX_بلاک ||
			بلاک->type == BOX_INLINE_بلاک ||
			بلاک->type == BOX_جدول_CELL ||
			بلاک->type == BOX_FLEX ||
			بلاک->type == BOX_INLINE_FLEX);
	أكد(بلاک->چوڑائی != UNKNOWN_چوڑائی);
	أكد(بلاک->چوڑائی != AUTO);

	بلاک->float_children = تہی;
	بلاک->cached_رکھو_نیچے_level = 0;
	بلاک->clear_level = 0;

	/* special case if the block contains an object */
	اگر (بلاک->object) {
		صحیح عارضی_چوڑائی = بلاک->چوڑائی;
		اگر (!layout_بلاک_object(بلاک))
			واپس false;
		layout_get_object_dimensions(بلاک, &عارضی_چوڑائی,
				&بلاک->اونچائی, INT_MIN, زیادہ_سے_زیادہ_عدد,
				INT_MIN, زیادہ_سے_زیادہ_عدد);
		واپس true;
	} ورنہ اگر (بلاک->flags & REPLACE_DIM) {
		واپس true;
	}

	/* special case if the block contains an radio button or checkbox */
	اگر (بلاک->gadget && (بلاک->gadget->type == GADGET_RADIO ||
			بلاک->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_سائز = INTTOFIX(1);
		اگر (بلاک->اونچائی == AUTO)
			بلاک->اونچائی = FIXTOINT(css_unit_len2device_px(
					بلاک->style,
					&content->unit_len_ctx,
					gadget_سائز, gadget_unit));
	}

	box = بلاک->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = بلاک->padding[چوٹی_2];
	اگر (box)
		box->y = بلاک->padding[چوٹی_2];

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
	جب (box) {
		شمار css_حد_سے_تجاوز_e حد_سے_تجاوز_x = CSS_حد_سے_تجاوز_VISIBLE;
		شمار css_حد_سے_تجاوز_e حد_سے_تجاوز_y = CSS_حد_سے_تجاوز_VISIBLE;

		أكد(box->type == BOX_بلاک ||
				box->type == BOX_FLEX ||
				box->type == BOX_جدول ||
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
			box->x = box->والد->padding[بایاں_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			جاؤ advance_مقصد_اگلا_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		اگر (margin_collapse == تہی) {
			margin_collapse = layout_اگلا_margin_بلاک(
					&content->unit_len_ctx, box, بلاک,
					viewport_اونچائی,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			اندر_margin = true;
		}

		/* Clearance. */
		y = 0;
		اگر (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(بلاک->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		اگر (box->style) {
			حد_سے_تجاوز_x = css_computed_حد_سے_تجاوز_x(box->style);
			حد_سے_تجاوز_y = css_computed_حد_سے_تجاوز_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		اگر (box->type == BOX_FLEX ||
		    box->type == BOX_بلاک ||
		    box->flags & IFRAME) {
			اگر (lh__box_is_object(box) == false &&
					box->style &&
					(حد_سے_تجاوز_x != CSS_حد_سے_تجاوز_VISIBLE ||
					 حد_سے_تجاوز_y != CSS_حد_سے_تجاوز_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				صحیح x0, x1, چوٹی;
				ساخت box *بایاں, *دایاں;
				چوٹی = cy + max_pos_margin - max_neg_margin;
				چوٹی = (چوٹی > y) ? چوٹی : y;
				x0 = cx;
				x1 = cx + box->والد->چوڑائی -
						box->والد->padding[بایاں_2] -
						box->والد->padding[دایاں_2];
				ڈھونڈو_sides(بلاک->float_children, چوٹی, چوٹی,
						&x0, &x1, &بایاں, &دایاں);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->والد->چوڑائی -
						box->والد->padding[بایاں_2] -
						box->والد->padding[دایاں_2] -
						x1;
			}
			layout_بلاک_ڈھونڈو_dimensions(&content->unit_len_ctx,
					box->والد->چوڑائی,
					viewport_اونچائی, lm, rm, box);
			اگر (box->type == BOX_بلاک && !(box->flags & IFRAME)) {
				layout_بلاک_جوڑو_scrollbar(box, دایاں_2);
				layout_بلاک_جوڑو_scrollbar(box, BOTTOM);
			}
		} ورنہ اگر (box->type == BOX_جدول) {
			اگر (box->style != تہی) {
				شمار css_چوڑائی_e wtype;
				css_fixed چوڑائی = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_چوڑائی(box->style, &چوڑائی,
						&unit);

				اگر (wtype == CSS_چوڑائی_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					صحیح x0, x1, چوٹی;
					ساخت box *بایاں, *دایاں;
					چوٹی = cy + max_pos_margin -
							max_neg_margin;
					چوٹی = (چوٹی > y) ? چوٹی : y;
					x0 = cx;
					x1 = cx + box->والد->چوڑائی -
						box->والد->padding[بایاں_2] -
						box->والد->padding[دایاں_2];
					ڈھونڈو_sides(بلاک->float_children,
						چوٹی, چوٹی, &x0, &x1,
						&بایاں, &دایاں);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->والد->چوڑائی -
						box->والد->padding[بایاں_2] -
						box->والد->padding[دایاں_2] -
						x1;
				}
			}
			اگر (!layout_جدول(box, box->والد->چوڑائی - lm - rm,
					content))
				واپس false;
			layout_solve_چوڑائی(box, box->والد->چوڑائی, box->چوڑائی,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->والد->padding[بایاں_2] + box->margin[بایاں_2] +
				box->border[بایاں_2].چوڑائی;
		cx += box->x;

		/* Position box: vertical. */
		اگر (box->border[چوٹی_2].چوڑائی) {
			box->y += box->border[چوٹی_2].چوڑائی;
			cy += box->border[چوٹی_2].چوڑائی;
		}

		/* Vertical margin */
		اگر (((box->type == BOX_بلاک && (box->flags & HAS_اونچائی)) ||
		     box->type == BOX_FLEX ||
		     box->type == BOX_جدول ||
		     (box->type == BOX_INLINE_CONTAINER &&
		      !box_is_پہلا_بچہ(box)) ||
		     margin_collapse == box) &&
		    اندر_margin == true) {
			/* Margin goes above this box. */
			cy += max_pos_margin - max_neg_margin;
			box->y += max_pos_margin - max_neg_margin;

			/* Current margin has been applied. */
			اندر_margin = false;
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
				(box->type == BOX_بلاک && box->style &&
				 (حد_سے_تجاوز_x != CSS_حد_سے_تجاوز_VISIBLE ||
				  حد_سے_تجاوز_y != CSS_حد_سے_تجاوز_VISIBLE))) {

			اگر (box->type == BOX_FLEX) {
				اگر (!layout_flex(box, box->چوڑائی, content)) {
					واپس false;
				}
			} ورنہ {
				layout_بلاک_context(box,
						viewport_اونچائی, content);
			}

			cy += box->padding[چوٹی_2];

			اگر (box->اونچائی == AUTO) {
				box->اونچائی = 0;
				layout_بلاک_جوڑو_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->اونچائی + box->padding[BOTTOM] +
					box->border[BOTTOM].چوڑائی;
			y = box->y + box->padding[چوٹی_2] + box->اونچائی +
					box->padding[BOTTOM] +
					box->border[BOTTOM].چوڑائی;

			/* Skip children, because they are done in the new
			 * block context */
			جاؤ advance_مقصد_اگلا_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->چوڑائی);

		/* Layout (except tables). */
		اگر (box->object) {
			اگر (!layout_بلاک_object(box))
				واپس false;

		} ورنہ اگر (box->type == BOX_INLINE_CONTAINER) {
			box->چوڑائی = box->والد->چوڑائی;
			اگر (!layout_inline_container(box, box->چوڑائی, بلاک,
					cx, cy, content))
				واپس false;

		} ورنہ اگر (box->type == BOX_جدول) {
			/* Move down to avoid floats if necessary. */
			صحیح x0, x1;
			ساخت box *بایاں, *دایاں;
			y = cy;
			جب (1) {
				شمار css_چوڑائی_e wtype;
				css_fixed چوڑائی = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_چوڑائی(box->style,
						&چوڑائی, &unit);

				x0 = cx;
				x1 = cx + box->والد->چوڑائی;
				ڈھونڈو_sides(بلاک->float_children, y,
						y + box->اونچائی,
						&x0, &x1, &بایاں, &دایاں);
				اگر (wtype == CSS_چوڑائی_AUTO)
					توڑو;
				اگر (box->چوڑائی <= x1 - x0)
					توڑو;
				اگر (!بایاں && !دایاں)
					توڑو;
				ورنہ اگر (!بایاں)
					y = دایاں->y + دایاں->اونچائی + 1;
				ورنہ اگر (!دایاں)
					y = بایاں->y + بایاں->اونچائی + 1;
				ورنہ اگر (بایاں->y + بایاں->اونچائی <
						دایاں->y + دایاں->اونچائی)
					y = بایاں->y + بایاں->اونچائی + 1;
				ورنہ
					y = دایاں->y + دایاں->اونچائی + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		اگر (box->type == BOX_بلاک && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			اگر (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = تہی;
			}

			y = box->padding[چوٹی_2];
			box = box->children;
			box->y = y;
			cy += y;
			جاری;
		} ورنہ اگر (box->type == BOX_بلاک || box->object ||
				box->flags & IFRAME)
			cy += box->padding[چوٹی_2];

		اگر (box->type == BOX_بلاک && box->اونچائی == AUTO) {
			box->اونچائی = 0;
			layout_بلاک_جوڑو_scrollbar(box, BOTTOM);
		}

		cy += box->اونچائی + box->padding[BOTTOM] +
				box->border[BOTTOM].چوڑائی;
		cx -= box->x;
		y = box->y + box->padding[چوٹی_2] + box->اونچائی +
				box->padding[BOTTOM] +
				box->border[BOTTOM].چوڑائی;

	advance_مقصد_اگلا_box:
		اگر (!box->اگلا) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			کرو {
				اگر (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = تہی;
				}

				/* Apply bottom margin */
				اگر (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				ورنہ اگر (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->والد;
				اگر (box == بلاک)
					توڑو;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				اگر (box->type == BOX_بلاک &&
						box->flags & MAKE_اونچائی) {
					margin_collapse = تہی;
					اندر_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				اگر (box->اونچائی == AUTO) {
					box->اونچائی = y - box->padding[چوٹی_2];

					اگر (box->type == BOX_بلاک)
						layout_بلاک_جوڑو_scrollbar(box,
								BOTTOM);
				} ورنہ
					cy += box->اونچائی -
							(y - box->padding[چوٹی_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				اگر (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_اونچائی(
							&content->unit_len_ctx,
							box, تہی)) {
					/* Height altered */
					/* Set current cy */
					cy += box->اونچائی -
							(y - box->padding[چوٹی_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].چوڑائی;
				cx -= box->x;
				y = box->y + box->padding[چوٹی_2] + box->اونچائی +
						box->padding[BOTTOM] +
						box->border[BOTTOM].چوڑائی;

			} جب (box->اگلا == تہی);
			اگر (box == بلاک)
				توڑو;
		}

		/* To next sibling. */

		اگر (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = تہی;
		}

		اگر (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		ورنہ اگر (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->اگلا;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	برائے (box = بلاک->float_children; box; box = box->اگلا_float) {
		y = box->y + box->اونچائی + box->padding[BOTTOM] +
				box->border[BOTTOM].چوڑائی + box->margin[BOTTOM];
		اگر (cy < y)
			cy = y;
	}

	اگر (بلاک->اونچائی == AUTO) {
		بلاک->اونچائی = cy - بلاک->padding[چوٹی_2];
		اگر (بلاک->type == BOX_بلاک)
			layout_بلاک_جوڑو_scrollbar(بلاک, BOTTOM);
	}

	اگر (بلاک->style && css_computed_position(بلاک->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_اونچائی(&content->unit_len_ctx, بلاک, تہی);
	}

	اگر (بلاک->gadget &&
			(بلاک->gadget->type == GADGET_TEXTAREA ||
			بلاک->gadget->type == GADGET_PASSWORD ||
			بلاک->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		صحیح ta_چوڑائی = بلاک->padding[بایاں_2] + بلاک->چوڑائی +
				بلاک->padding[دایاں_2];
		صحیح ta_اونچائی = بلاک->padding[چوٹی_2] + بلاک->اونچائی +
				بلاک->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				بلاک->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_سیٹ_layout(بلاک->gadget->ڈیٹا.متن.ta,
				&fstyle, ta_چوڑائی, ta_اونچائی,
				بلاک->padding[چوٹی_2], بلاک->padding[دایاں_2],
				بلاک->padding[BOTTOM], بلاک->padding[بایاں_2]);
	}

	واپس true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
ساکن bool
layout_get_element_لیبل(
		ثابت dom_گرہ *گرہ,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_گرہ_type گرہ_type;
	dom_exception exc;

	exc = dom_گرہ_get_گرہ_type(گرہ, &گرہ_type);
	اگر (exc != DOM_NO_ERR ||
	    گرہ_type != DOM_ELEMENT_گرہ) {
		واپس false;
	}

	exc = dom_html_element_get_لیبل_type(گرہ, &element_type);
	اگر (exc != DOM_NO_ERR) {
		واپس false;
	}

	*type = element_type;
	واپس true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
ساکن اندرونی bool
layout__check_element_type(
		ثابت dom_گرہ *گرہ,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	اگر (!layout_get_element_لیبل(گرہ, &element_type)) {
		واپس false;
	}

	واپس element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ساکن bool
layout_get_li_قدر(dom_گرہ *li_گرہ, dom_long *قدر_برآمد)
{
	dom_exception exc;
	dom_long قدر;
	bool has_قدر;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_گرہ,
			corestring_dom_قدر,
			&has_قدر);
	اگر (exc != DOM_NO_ERR || has_قدر == false) {
		واپس false;
	}

	exc = dom_html_li_element_get_قدر(
			(dom_html_li_element *)li_گرہ,
			&قدر);
	اگر (exc != DOM_NO_ERR) {
		واپس false;
	}

	*قدر_برآمد = قدر;
	واپس true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ساکن bool
layout_get_ol_آغاز(dom_گرہ *ol_گرہ, dom_long *آغاز_برآمد)
{
	dom_exception exc;
	dom_long آغاز;
	bool has_آغاز;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_گرہ,
			corestring_dom_آغاز,
			&has_آغاز);
	اگر (exc != DOM_NO_ERR || has_آغاز == false) {
		واپس false;
	}

	exc = dom_html_olist_element_get_آغاز(
			(dom_html_olist_element *)ol_گرہ,
			&آغاز);
	اگر (exc != DOM_NO_ERR) {
		واپس false;
	}

	*آغاز_برآمد = آغاز;
	واپس true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
ساکن bool
layout__get_ol_reversed(dom_گرہ *ol_گرہ)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_گرہ,
			corestring_dom_reversed,
			&has_reversed);
	اگر (exc != DOM_NO_ERR) {
		واپس false;
	}

	واپس has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
ساکن bool
layout_get_list_شے_گنتی(
		dom_گرہ *list_owner, dom_long *گنتی_برآمد)
{
	dom_html_element_type لیبل_type;
	dom_exception exc;
	dom_گرہ *بچہ;
	صحیح گنتی;

	اگر (list_owner == تہی) {
		واپس false;
	}

	اگر (!layout_get_element_لیبل(list_owner, &لیبل_type)) {
		واپس false;
	}

	اگر (لیبل_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    لیبل_type != DOM_HTML_ELEMENT_TYPE_UL) {
		واپس false;
	}

	exc = dom_گرہ_get_پہلا_بچہ(list_owner, &بچہ);
	اگر (exc != DOM_NO_ERR) {
		واپس false;
	}

	گنتی = 0;
	جب (بچہ != تہی) {
		dom_گرہ *عارضی_گرہ;

		اگر (layout__check_element_type(بچہ,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ساخت box *بچہ_box;
			اگر (dom_گرہ_get_user_ڈیٹا(بچہ,
					corestring_dom_ns_کلید_box_گرہ_ڈیٹا,
					&بچہ_box) != DOM_NO_ERR) {
				dom_گرہ_unref(بچہ);
				واپس false;
			}

			اگر (بچہ_box != تہی &&
			    بچہ_box->list_marker != تہی) {
				گنتی++;
			}
		}

		exc = dom_گرہ_get_اگلا_sibling(بچہ, &عارضی_گرہ);
		dom_گرہ_unref(بچہ);
		اگر (exc != DOM_NO_ERR) {
			واپس false;
		}

		بچہ = عارضی_گرہ;
	}

	*گنتی_برآمد = گنتی;
	واپس true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
ساکن خالی
layout_ordered_list_گنتی(
		ساخت box *box)
{
	dom_html_element_type لیبل_type;
	dom_exception exc;
	dom_گرہ *بچہ;
	صحیح قدم = 1;
	dom_long اگلا;

	اگر (box->گرہ == تہی) {
		واپس;
	}

	اگر (!layout_get_element_لیبل(box->گرہ, &لیبل_type)) {
		واپس;
	}

	اگر (لیبل_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    لیبل_type != DOM_HTML_ELEMENT_TYPE_UL) {
		واپس;
	}

	اگلا = 1;
	اگر (لیبل_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_آغاز = layout_get_ol_آغاز(box->گرہ, &اگلا);
		bool have_reversed = layout__get_ol_reversed(box->گرہ);

		اگر (have_reversed) {
			قدم = -1;
		}

		اگر (!have_آغاز && have_reversed) {
			layout_get_list_شے_گنتی(box->گرہ, &اگلا);
		}
	}

	exc = dom_گرہ_get_پہلا_بچہ(box->گرہ, &بچہ);
	اگر (exc != DOM_NO_ERR) {
		واپس;
	}

	جب (بچہ != تہی) {
		dom_گرہ *عارضی_گرہ;

		اگر (layout__check_element_type(بچہ,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ساخت box *بچہ_box;

			اگر (dom_گرہ_get_user_ڈیٹا(بچہ,
					corestring_dom_ns_کلید_box_گرہ_ڈیٹا,
					&بچہ_box) != DOM_NO_ERR) {
				dom_گرہ_unref(بچہ);
				واپس;
			}

			اگر (بچہ_box != تہی &&
			    بچہ_box->list_marker != تہی) {
				dom_long قدر;
				ساخت box *marker = بچہ_box->list_marker;
				اگر (layout_get_li_قدر(بچہ, &قدر)) {
					marker->list_قدر = قدر;
					اگلا = marker->list_قدر;
				} ورنہ {
					marker->list_قدر = اگلا;
				}
				اگلا += قدم;
			}
		}

		exc = dom_گرہ_get_اگلا_sibling(بچہ, &عارضی_گرہ);
		dom_گرہ_unref(بچہ);
		اگر (exc != DOM_NO_ERR) {
			واپس;
		}

		بچہ = عارضی_گرہ;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
ساکن خالی
layout_سیٹ_numerical_marker_متن(
		ثابت html_content *content,
		ساخت box *box)
{
	ساخت box *marker = box->list_marker;
	size_t شمار_len;
	css_error css_res;
	شمار {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_سائز = 20,
	};

	marker->متن = talloc_array(content->bctx, حرف, LIST_MARKER_سائز);
	اگر (marker->متن == تہی) {
		واپس;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_قدر,
			marker->متن, LIST_MARKER_سائز, &شمار_len);
	اگر (css_res == CSS_OK) {
		اگر (شمار_len > LIST_MARKER_سائز) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->متن = talloc_realloc(content->bctx,
					marker->متن,
					حرف,
					شمار_len);
			اگر (marker->متن == تہی) {
				واپس;
			}
			css_computed_format_list_style(box->style,
					marker->list_قدر, marker->متن,
					شمار_len, &شمار_len);
		}
		marker->لمبائی = شمار_len;
	}
}

/**
 * Find out if box's style represents a numerical list style type.
 *
 * \param[in]  b  Box with style to test.
 * \return true if box has numerical list style type, false otherwise.
 */
ساکن bool
layout_list_شے_is_numerical(
		ثابت ساخت box *b)
{
	شمار css_list_style_type_e t = css_computed_list_style_type(b->style);

	انتخاب (t) {
	حالت CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	حالت CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	حالت CSS_LIST_STYLE_TYPE_مربع: /* Fall through. */
	حالت CSS_LIST_STYLE_TYPE_NONE:
		واپس false;

	طےشدہ:
		واپس true;
	}
}

/**
 * Layout list markers.
 */
ساکن خالی
layout_lists(ثابت html_content *content, ساخت box *box)
{
	ساخت box *بچہ;

	layout_ordered_list_گنتی(box);

	برائے (بچہ = box->children; بچہ; بچہ = بچہ->اگلا) {
		اگر (بچہ->list_marker) {
			ساخت box *marker = بچہ->list_marker;

			اگر (layout_list_شے_is_numerical(بچہ)) {
				اگر (marker->متن == تہی) {
					layout_سیٹ_numerical_marker_متن(
							content, بچہ);
				}
			}
			اگر (marker->object) {
				marker->چوڑائی =
					content_get_چوڑائی(marker->object);
				marker->x = -marker->چوڑائی;
				marker->اونچائی =
					content_get_اونچائی(marker->object);
				marker->y = (سطر_اونچائی(
						&content->unit_len_ctx,
						marker->style) -
						marker->اونچائی) / 2;
			} ورنہ اگر (marker->متن) {
				اگر (marker->چوڑائی == UNKNOWN_چوڑائی) {
					plot_font_style_t fstyle;
					font_plot_style_from_css(
							&content->unit_len_ctx,
							marker->style,
							&fstyle);
					content->font_func->چوڑائی(&fstyle,
							marker->متن,
							marker->لمبائی,
							&marker->چوڑائی);
					marker->flags |= MEASURED;
				}
				marker->x = -marker->چوڑائی;
				marker->y = 0;
				marker->اونچائی = سطر_اونچائی(
						&content->unit_len_ctx,
						marker->style);
			} ورنہ {
				marker->x = 0;
				marker->y = 0;
				marker->چوڑائی = 0;
				marker->اونچائی = 0;
			}
			/* Gap between marker and content */
			marker->x -= 4;
		}
		layout_lists(content, بچہ);
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
ساکن خالی
layout_compute_offsets(ثابت css_unit_ctx *unit_len_ctx,
		       ساخت box *box,
		       ساخت box *containing_بلاک,
		       صحیح *چوٹی,
		       صحیح *دایاں,
		       صحیح *bottom,
		       صحیح *بایاں)
{
	uint32_t type;
	css_fixed قدر = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(containing_بلاک->چوڑائی != UNKNOWN_چوڑائی);
	أكد(containing_بلاک->چوڑائی != AUTO);
	أكد(containing_بلاک->اونچائی != AUTO);

	/*بایاں*/
	type = css_computed_بایاں(box->style, &قدر, &unit);
	اگر (type == CSS_بایاں_سیٹ) {
		اگر (unit == CSS_UNIT_PCT) {
			*بایاں = FPCT_کا_INT_TOINT(قدر,
					containing_بلاک->چوڑائی);
		} ورنہ {
			*بایاں = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					قدر, unit));
		}
	} ورنہ {
		*بایاں = AUTO;
	}

	/*دایاں*/
	type = css_computed_دایاں(box->style, &قدر, &unit);
	اگر (type == CSS_دایاں_سیٹ) {
		اگر (unit == CSS_UNIT_PCT) {
			*دایاں = FPCT_کا_INT_TOINT(قدر,
					containing_بلاک->چوڑائی);
		} ورنہ {
			*دایاں = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					قدر, unit));
		}
	} ورنہ {
		*دایاں = AUTO;
	}

	/*چوٹی*/
	type = css_computed_چوٹی(box->style, &قدر, &unit);
	اگر (type == CSS_چوٹی_سیٹ) {
		اگر (unit == CSS_UNIT_PCT) {
			*چوٹی = FPCT_کا_INT_TOINT(قدر,
					containing_بلاک->اونچائی);
		} ورنہ {
			*چوٹی = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					قدر, unit));
		}
	} ورنہ {
		*چوٹی = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &قدر, &unit);
	اگر (type == CSS_BOTTOM_سیٹ) {
		اگر (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_کا_INT_TOINT(قدر,
					containing_بلاک->اونچائی);
		} ورنہ {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					قدر, unit));
		}
	} ورنہ {
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
ساکن bool
layout_absolute(ساخت box *box,
		ساخت box *containing_بلاک,
		صحیح cx, صحیح cy,
		html_content *content)
{
	صحیح static_بایاں, static_چوٹی;  /* static position */
	صحیح چوٹی, دایاں, bottom, بایاں;
	صحیح چوڑائی, اونچائی, max_چوڑائی, min_چوڑائی;
	صحیح *margin = box->margin;
	صحیح *padding = box->padding;
	ساخت box_border *border = box->border;
	صحیح دستیاب_چوڑائی = containing_بلاک->چوڑائی;
	صحیح space;

	أكد(box->type == BOX_بلاک || box->type == BOX_جدول ||
			box->type == BOX_INLINE_بلاک ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_بایاں = cx + box->x;
	static_چوٹی = cy + box->y;

	اگر (containing_بلاک->type == BOX_بلاک ||
			containing_بلاک->type == BOX_INLINE_بلاک ||
			containing_بلاک->type == BOX_جدول_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_بلاک->چوڑائی += containing_بلاک->padding[بایاں_2] +
				containing_بلاک->padding[دایاں_2];
		containing_بلاک->اونچائی += containing_بلاک->padding[چوٹی_2] +
				containing_بلاک->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_بلاک,
			&چوٹی, &دایاں, &bottom, &بایاں);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_بلاک;
	layout_ڈھونڈو_dimensions(&content->unit_len_ctx, دستیاب_چوڑائی, -1,
			box, box->style, &چوڑائی, &اونچائی,
			&max_چوڑائی, &min_چوڑائی, 0, 0,
			margin, padding, border);
	box->float_container = تہی;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      بایاں, margin[بایاں_2], border[بایاں_2].چوڑائی, padding[بایاں_2], چوڑائی,
	      padding[دایاں_2], border[دایاں_2].چوڑائی, margin[دایاں_2], دایاں,
	      containing_بلاک->چوڑائی);


	اگر (بایاں == AUTO && چوڑائی == AUTO && دایاں == AUTO) {
		اگر (margin[بایاں_2] == AUTO)
			margin[بایاں_2] = 0;
		اگر (margin[دایاں_2] == AUTO)
			margin[دایاں_2] = 0;
		بایاں = static_بایاں;

		چوڑائی = min(max(box->min_چوڑائی, دستیاب_چوڑائی),
			box->max_چوڑائی);
		چوڑائی -= box->margin[بایاں_2] + box->border[بایاں_2].چوڑائی +
			box->padding[بایاں_2] + box->padding[دایاں_2] +
			box->border[دایاں_2].چوڑائی + box->margin[دایاں_2];

		/* Adjust for {min|max}-width */
		اگر (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی) چوڑائی = max_چوڑائی;
		اگر (چوڑائی < min_چوڑائی) چوڑائی = min_چوڑائی;

		دایاں = containing_بلاک->چوڑائی -
			بایاں -
			margin[بایاں_2] - border[بایاں_2].چوڑائی - padding[بایاں_2] -
			چوڑائی -
			padding[دایاں_2] - border[دایاں_2].چوڑائی - margin[دایاں_2];
	} ورنہ اگر (بایاں != AUTO && چوڑائی != AUTO && دایاں != AUTO) {

		/* Adjust for {min|max}-width */
		اگر (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی) چوڑائی = max_چوڑائی;
		اگر (min_چوڑائی >  0 && چوڑائی < min_چوڑائی) چوڑائی = min_چوڑائی;

		اگر (margin[بایاں_2] == AUTO && margin[دایاں_2] == AUTO) {
			space = containing_بلاک->چوڑائی -
					بایاں - border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - دایاں;
			اگر (space < 0) {
				margin[بایاں_2] = 0;
				margin[دایاں_2] = space;
			} ورنہ {
				margin[بایاں_2] = margin[دایاں_2] = space / 2;
			}
		} ورنہ اگر (margin[بایاں_2] == AUTO) {
			margin[بایاں_2] = containing_بلاک->چوڑائی -
					بایاں - border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2] -
					دایاں;
		} ورنہ اگر (margin[دایاں_2] == AUTO) {
			margin[دایاں_2] = containing_بلاک->چوڑائی -
					بایاں - margin[بایاں_2] -
					border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - دایاں;
		} ورنہ {
			دایاں = containing_بلاک->چوڑائی -
					بایاں - margin[بایاں_2] -
					border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2];
		}
	} ورنہ {
		اگر (margin[بایاں_2] == AUTO)
			margin[بایاں_2] = 0;
		اگر (margin[دایاں_2] == AUTO)
			margin[دایاں_2] = 0;

		اگر (بایاں == AUTO && چوڑائی == AUTO && دایاں != AUTO) {
			دستیاب_چوڑائی -= دایاں;

			چوڑائی = min(max(box->min_چوڑائی, دستیاب_چوڑائی),
				box->max_چوڑائی);
			چوڑائی -= box->margin[بایاں_2] + box->border[بایاں_2].چوڑائی +
				box->padding[بایاں_2] + box->padding[دایاں_2] +
				box->border[دایاں_2].چوڑائی + box->margin[دایاں_2];

			/* Adjust for {min|max}-width */
			اگر (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی)
				چوڑائی = max_چوڑائی;
			اگر (چوڑائی < min_چوڑائی)
				چوڑائی = min_چوڑائی;

			بایاں = containing_بلاک->چوڑائی -
					margin[بایاں_2] - border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2] -
					دایاں;
		} ورنہ اگر (بایاں == AUTO && چوڑائی != AUTO && دایاں == AUTO) {

			/* Adjust for {min|max}-width */
			اگر (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی)
				چوڑائی = max_چوڑائی;
			اگر (min_چوڑائی >  0 && چوڑائی < min_چوڑائی)
				چوڑائی = min_چوڑائی;

			بایاں = static_بایاں;
			دایاں = containing_بلاک->چوڑائی -
					بایاں - margin[بایاں_2] -
					border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2];
		} ورنہ اگر (بایاں != AUTO && چوڑائی == AUTO && دایاں == AUTO) {
			دستیاب_چوڑائی -= بایاں;

			چوڑائی = min(max(box->min_چوڑائی, دستیاب_چوڑائی),
				box->max_چوڑائی);
			چوڑائی -= box->margin[بایاں_2] + box->border[بایاں_2].چوڑائی +
				box->padding[بایاں_2] + box->padding[دایاں_2] +
				box->border[دایاں_2].چوڑائی + box->margin[دایاں_2];

			/* Adjust for {min|max}-width */
			اگر (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی)
				چوڑائی = max_چوڑائی;
			اگر (چوڑائی < min_چوڑائی)
				چوڑائی = min_چوڑائی;

			دایاں = containing_بلاک->چوڑائی -
					بایاں - margin[بایاں_2] -
					border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2];
		} ورنہ اگر (بایاں == AUTO && چوڑائی != AUTO && دایاں != AUTO) {

			/* Adjust for {min|max}-width */
			اگر (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی)
				چوڑائی = max_چوڑائی;
			اگر (چوڑائی < min_چوڑائی)
				چوڑائی = min_چوڑائی;

			بایاں = containing_بلاک->چوڑائی -
					margin[بایاں_2] - border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2] -
					دایاں;
		} ورنہ اگر (بایاں != AUTO && چوڑائی == AUTO && دایاں != AUTO) {
			چوڑائی = containing_بلاک->چوڑائی -
					بایاں - margin[بایاں_2] -
					border[بایاں_2].چوڑائی -
					padding[بایاں_2] - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2] -
					دایاں;

			/* Adjust for {min|max}-width */
			اگر (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی)
				چوڑائی = max_چوڑائی;
			اگر (چوڑائی < min_چوڑائی)
				چوڑائی = min_چوڑائی;

		} ورنہ اگر (بایاں != AUTO && چوڑائی != AUTO && دایاں == AUTO) {

			/* Adjust for {min|max}-width */
			اگر (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی)
				چوڑائی = max_چوڑائی;
			اگر (چوڑائی < min_چوڑائی)
				چوڑائی = min_چوڑائی;

			دایاں = containing_بلاک->چوڑائی -
					بایاں - margin[بایاں_2] -
					border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      بایاں, margin[بایاں_2], border[بایاں_2].چوڑائی, padding[بایاں_2], چوڑائی,
	      padding[دایاں_2], border[دایاں_2].چوڑائی, margin[دایاں_2], دایاں,
	      containing_بلاک->چوڑائی);

	box->x = بایاں + margin[بایاں_2] + border[بایاں_2].چوڑائی - cx;
	اگر (containing_بلاک->type == BOX_بلاک ||
			containing_بلاک->type == BOX_INLINE_بلاک ||
			containing_بلاک->type == BOX_جدول_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_بلاک->چوڑائی -= containing_بلاک->padding[بایاں_2] +
				containing_بلاک->padding[دایاں_2];
	} ورنہ {
		/** \todo inline ancestors */
	}
	box->چوڑائی = چوڑائی;
	box->اونچائی = اونچائی;

	اگر (box->type == BOX_بلاک || box->type == BOX_INLINE_بلاک ||
			box->object || box->flags & IFRAME) {
		اگر (!layout_بلاک_context(box, -1, content))
			واپس false;
	} ورنہ اگر (box->type == BOX_جدول) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_بلاک;
		/* \todo  layout_table considers margins etc. again */
		اگر (!layout_جدول(box, چوڑائی, content))
			واپس false;
		box->float_container = تہی;
		layout_solve_چوڑائی(box, box->والد->چوڑائی, box->چوڑائی, 0, 0,
				-1, -1);
	} ورنہ اگر (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_بلاک;
		اگر (!layout_flex(box, چوڑائی, content))
			واپس false;
		box->float_container = تہی;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      چوٹی, margin[چوٹی_2], border[چوٹی_2].چوڑائی, padding[چوٹی_2], اونچائی,
	      padding[BOTTOM], border[BOTTOM].چوڑائی, margin[BOTTOM], bottom,
	      containing_بلاک->اونچائی);

	اگر (چوٹی == AUTO && اونچائی == AUTO && bottom == AUTO) {
		چوٹی = static_چوٹی;
		اونچائی = box->اونچائی;
		اگر (margin[چوٹی_2] == AUTO)
			margin[چوٹی_2] = 0;
		اگر (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_بلاک->اونچائی -
				چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
				padding[چوٹی_2] - اونچائی - padding[BOTTOM] -
				border[BOTTOM].چوڑائی - margin[BOTTOM];
	} ورنہ اگر (چوٹی != AUTO && اونچائی != AUTO && bottom != AUTO) {
		اگر (margin[چوٹی_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_بلاک->اونچائی -
					چوٹی - border[چوٹی_2].چوڑائی - padding[چوٹی_2] -
					اونچائی - padding[BOTTOM] -
					border[BOTTOM].چوڑائی - bottom;
			margin[چوٹی_2] = margin[BOTTOM] = space / 2;
		} ورنہ اگر (margin[چوٹی_2] == AUTO) {
			margin[چوٹی_2] = containing_بلاک->اونچائی -
					چوٹی - border[چوٹی_2].چوڑائی - padding[چوٹی_2] -
					اونچائی - padding[BOTTOM] -
					border[BOTTOM].چوڑائی - margin[BOTTOM] -
					bottom;
		} ورنہ اگر (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_بلاک->اونچائی -
					چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اونچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					bottom;
		} ورنہ {
			bottom = containing_بلاک->اونچائی -
					چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اونچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					margin[BOTTOM];
		}
	} ورنہ {
		اگر (margin[چوٹی_2] == AUTO)
			margin[چوٹی_2] = 0;
		اگر (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		اگر (چوٹی == AUTO && اونچائی == AUTO && bottom != AUTO) {
			اونچائی = box->اونچائی;
			چوٹی = containing_بلاک->اونچائی -
					margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اونچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					margin[BOTTOM] - bottom;
		} ورنہ اگر (چوٹی == AUTO && اونچائی != AUTO && bottom == AUTO) {
			چوٹی = static_چوٹی;
			bottom = containing_بلاک->اونچائی -
					چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اونچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					margin[BOTTOM];
		} ورنہ اگر (چوٹی != AUTO && اونچائی == AUTO && bottom == AUTO) {
			اونچائی = box->اونچائی;
			bottom = containing_بلاک->اونچائی -
					چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اونچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					margin[BOTTOM];
		} ورنہ اگر (چوٹی == AUTO && اونچائی != AUTO && bottom != AUTO) {
			چوٹی = containing_بلاک->اونچائی -
					margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اونچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					margin[BOTTOM] - bottom;
		} ورنہ اگر (چوٹی != AUTO && اونچائی == AUTO && bottom != AUTO) {
			اونچائی = containing_بلاک->اونچائی -
					چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - padding[BOTTOM] -
					border[BOTTOM].چوڑائی - margin[BOTTOM] -
					bottom;
		} ورنہ اگر (چوٹی != AUTO && اونچائی != AUTO && bottom == AUTO) {
			bottom = containing_بلاک->اونچائی -
					چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اونچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      چوٹی, margin[چوٹی_2], border[چوٹی_2].چوڑائی, padding[چوٹی_2], اونچائی,
	      padding[BOTTOM], border[BOTTOM].چوڑائی, margin[BOTTOM], bottom,
	      containing_بلاک->اونچائی);

	box->y = چوٹی + margin[چوٹی_2] + border[چوٹی_2].چوڑائی - cy;
	اگر (containing_بلاک->type == BOX_بلاک ||
			containing_بلاک->type == BOX_INLINE_بلاک ||
			containing_بلاک->type == BOX_جدول_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_بلاک->اونچائی -= containing_بلاک->padding[چوٹی_2] +
				containing_بلاک->padding[BOTTOM];
	} ورنہ {
		/** \todo Inline ancestors */
	}
	box->اونچائی = اونچائی;
	layout_apply_minmax_اونچائی(&content->unit_len_ctx, box, containing_بلاک);

	واپس true;
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
ساکن bool
layout_position_absolute(ساخت box *box,
			 ساخت box *containing_بلاک,
			 صحیح cx, صحیح cy,
			 html_content *content)
{
	ساخت box *c;

	برائے (c = box->children; c; c = c->اگلا) {
		اگر ((c->type == BOX_بلاک || c->type == BOX_جدول ||
				c->type == BOX_INLINE_بلاک ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			اگر (!layout_absolute(c, containing_بلاک,
					cx, cy, content))
				واپس false;
			اگر (!layout_position_absolute(c, c, 0, 0, content))
				واپس false;
		} ورنہ اگر (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			اگر (!layout_position_absolute(c, c, 0, 0, content))
				واپس false;
		} ورنہ {
			صحیح px, py;
			اگر (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_بایاں ||
					css_computed_float(c->style) ==
						CSS_FLOAT_دایاں)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				ساخت box *p;
				px = c->x;
				py = c->y;
				برائے (p = box->والد; p && !p->float_children;
						p = p->والد) {
					px -= p->x;
					py -= p->y;
				}
			} ورنہ {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			اگر (!layout_position_absolute(c, containing_بلاک,
					cx + px, cy + py, content))
				واپس false;
		}
	}

	واپس true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
ساکن خالی layout_compute_relative_انحراف(
		ثابت css_unit_ctx *unit_len_ctx,
		ساخت box *box,
		صحیح *x,
		صحیح *y)
{
	صحیح بایاں, دایاں, چوٹی, bottom;
	ساخت box *containing_بلاک;

	أكد(box && box->والد && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	اگر (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_بایاں ||
			css_computed_float(box->style) == CSS_FLOAT_دایاں)) {
		containing_بلاک = box->float_container;
	} ورنہ {
		containing_بلاک = box->والد;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_بلاک,
			&چوٹی, &دایاں, &bottom, &بایاں);

	اگر (بایاں == AUTO && دایاں == AUTO)
		بایاں = دایاں = 0;
	ورنہ اگر (بایاں == AUTO)
		/* left is auto => computed = -right */
		بایاں = -دایاں;
	ورنہ اگر (دایاں == AUTO)
		/* right is auto => computed = -left */
		دایاں = -بایاں;
	ورنہ {
		/* over constrained => examine direction property
		 * of containing block */
		اگر (containing_بلاک->style &&
				css_computed_direction(
				containing_بلاک->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			بایاں = -دایاں;
		} ورنہ {
			/* assume LTR in all other cases */
			دایاں = -بایاں;
		}
	}

	أكد(بایاں == -دایاں);

	اگر (چوٹی == AUTO && bottom == AUTO) {
		چوٹی = bottom = 0;
	} ورنہ اگر (چوٹی == AUTO) {
		چوٹی = -bottom;
	} ورنہ {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -چوٹی;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", بایاں,
	      دایاں, چوٹی, bottom);

	*x = بایاں;
	*y = چوٹی;
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
ساکن خالی
layout_position_relative(
		ثابت css_unit_ctx *unit_len_ctx,
		ساخت box *جڑ,
		ساخت box *fp,
		صحیح fx,
		صحیح fy)
{
	ساخت box *box; /* for children of "root" */
	ساخت box *fn;  /* for block formatting context box for children of
			  * "box" */
	ساخت box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	صحیح x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	صحیح fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	أكد(جڑ);

	/* Normal children */
	برائے (box = جڑ->children; box; box = box->اگلا) {

		اگر (box->type == BOX_متن)
			جاری;

		/* If relatively positioned, get offsets */
		اگر (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_انحراف(
					unit_len_ctx, box, &x, &y);
		ورنہ
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		اگر (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_بایاں ||
				css_computed_float(box->style) ==
					CSS_FLOAT_دایاں) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			برائے (fc = fp->float_children; fc; fc = fc->اگلا_float) {
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
		} ورنہ {
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
			جاری;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		اگر (box->type == BOX_INLINE && box->inline_انتہا) {
			ساخت box *b;
			برائے (b = box->اگلا; b && b != box->inline_انتہا;
					b = b->اگلا) {
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
ساکن خالی
layout_get_box_bbox(
		ثابت css_unit_ctx *unit_len_ctx,
		ساخت box *box,
		صحیح *desc_x0, صحیح *desc_y0,
		صحیح *desc_x1, صحیح *desc_y1)
{
	*desc_x0 = -box->border[بایاں_2].چوڑائی;
	*desc_y0 = -box->border[چوٹی_2].چوڑائی;
	*desc_x1 = box->padding[بایاں_2] + box->چوڑائی + box->padding[دایاں_2] +
			box->border[دایاں_2].چوڑائی;
	*desc_y1 = box->padding[چوٹی_2] + box->اونچائی + box->padding[BOTTOM] +
			box->border[BOTTOM].چوڑائی;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	اگر (box->type == BOX_بلاک && box->style != تہی &&
			css_computed_حد_سے_تجاوز_y(box->style) ==
					CSS_حد_سے_تجاوز_VISIBLE &&
			box->object == تہی) {
		css_fixed font_سائز = 0;
		css_unit font_unit = CSS_UNIT_PT;
		صحیح متن_اونچائی;

		css_computed_font_سائز(box->style, &font_سائز, &font_unit);
		متن_اونچائی = css_unit_len2device_px(box->style, unit_len_ctx,
				font_سائز, font_unit);
		متن_اونچائی = FIXTOINT(متن_اونچائی * 3 / 4);
		*desc_y0 = (*desc_y0 < -متن_اونچائی) ? *desc_y0 : -متن_اونچائی;
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
ساکن خالی
layout_update_descendant_bbox(
		ثابت css_unit_ctx *unit_len_ctx,
		ساخت box *box,
		ساخت box *بچہ,
		صحیح off_x,
		صحیح off_y)
{
	صحیح بچہ_desc_x0, بچہ_desc_y0, بچہ_desc_x1, بچہ_desc_y1;

	/* get coordinates of child relative to box */
	صحیح بچہ_x = بچہ->x - off_x;
	صحیح بچہ_y = بچہ->y - off_y;

	bool html_object = (بچہ->object &&
			content_get_type(بچہ->object) == CONTENT_HTML);

	شمار css_حد_سے_تجاوز_e حد_سے_تجاوز_x = CSS_حد_سے_تجاوز_VISIBLE;
	شمار css_حد_سے_تجاوز_e حد_سے_تجاوز_y = CSS_حد_سے_تجاوز_VISIBLE;

	اگر (بچہ->style != تہی) {
		حد_سے_تجاوز_x = css_computed_حد_سے_تجاوز_x(بچہ->style);
		حد_سے_تجاوز_y = css_computed_حد_سے_تجاوز_y(بچہ->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, بچہ,
			&بچہ_desc_x0, &بچہ_desc_y0,
			&بچہ_desc_x1, &بچہ_desc_y1);

	اگر (حد_سے_تجاوز_x == CSS_حد_سے_تجاوز_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		بچہ_desc_x0 = بچہ->descendant_x0;
		بچہ_desc_x1 = بچہ->descendant_x1;
	}
	اگر (حد_سے_تجاوز_y == CSS_حد_سے_تجاوز_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		بچہ_desc_y0 = بچہ->descendant_y0;
		بچہ_desc_y1 = بچہ->descendant_y1;
	}

	بچہ_desc_x0 += بچہ_x;
	بچہ_desc_y0 += بچہ_y;
	بچہ_desc_x1 += بچہ_x;
	بچہ_desc_y1 += بچہ_y;

	/* increase box's descendant bbox to contain descendants */
	اگر (بچہ_desc_x0 < box->descendant_x0)
		box->descendant_x0 = بچہ_desc_x0;
	اگر (بچہ_desc_y0 < box->descendant_y0)
		box->descendant_y0 = بچہ_desc_y0;
	اگر (box->descendant_x1 < بچہ_desc_x1)
		box->descendant_x1 = بچہ_desc_x1;
	اگر (box->descendant_y1 < بچہ_desc_y1)
		box->descendant_y1 = بچہ_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
ساکن خالی layout_calculate_descendant_bboxes(
		ثابت css_unit_ctx *unit_len_ctx,
		ساخت box *box)
{
	ساخت box *بچہ;

	أكد(box->چوڑائی != UNKNOWN_چوڑائی);
	أكد(box->اونچائی != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	اگر (box->object && content_get_type(box->object) == CONTENT_HTML) {
		اگر (box->descendant_x1 < content_get_چوڑائی(box->object))
			box->descendant_x1 = content_get_چوڑائی(box->object);
		اگر (box->descendant_y1 < content_get_اونچائی(box->object))
			box->descendant_y1 = content_get_اونچائی(box->object);
	}

	اگر (box->iframe != تہی) {
		صحیح x, y;
		box_coords(box, &x, &y);

		browser_کھڑکی_سیٹ_position(box->iframe, x, y);
		browser_کھڑکی_سیٹ_dimensions(box->iframe,
				box->چوڑائی, box->اونچائی);
		browser_کھڑکی_reformat(box->iframe, true,
				box->چوڑائی, box->اونچائی);
	}

	اگر (box->type == BOX_INLINE || box->type == BOX_متن)
		واپس;

	اگر (box->type == BOX_INLINE_انتہا) {
		box = box->inline_انتہا;
		برائے (بچہ = box->اگلا; بچہ;
				بچہ = بچہ->اگلا) {
			اگر (بچہ->type == BOX_FLOAT_بایاں ||
					بچہ->type == BOX_FLOAT_دایاں)
				جاری;

			layout_update_descendant_bbox(unit_len_ctx, box, بچہ,
					box->x, box->y);

			اگر (بچہ == box->inline_انتہا)
				توڑو;
		}
		واپس;
	}

	اگر (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		واپس;

	برائے (بچہ = box->children; بچہ; بچہ = بچہ->اگلا) {
		اگر (بچہ->type == BOX_FLOAT_بایاں ||
				بچہ->type == BOX_FLOAT_دایاں)
			جاری;

		layout_calculate_descendant_bboxes(unit_len_ctx, بچہ);

		اگر (box->style && css_computed_حد_سے_تجاوز_x(box->style) ==
				CSS_حد_سے_تجاوز_HIDDEN &&
				css_computed_حد_سے_تجاوز_y(box->style) ==
				CSS_حد_سے_تجاوز_HIDDEN)
			جاری;

		layout_update_descendant_bbox(unit_len_ctx, box, بچہ, 0, 0);
	}

	برائے (بچہ = box->float_children; بچہ; بچہ = بچہ->اگلا_float) {
		أكد(بچہ->type == BOX_FLOAT_بایاں ||
				بچہ->type == BOX_FLOAT_دایاں);

		layout_calculate_descendant_bboxes(unit_len_ctx, بچہ);

		layout_update_descendant_bbox(unit_len_ctx, box, بچہ, 0, 0);
	}

	اگر (box->list_marker) {
		بچہ = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, بچہ);

		layout_update_descendant_bbox(unit_len_ctx, box, بچہ, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, صحیح چوڑائی, صحیح اونچائی)
{
	bool ret;
	ساخت box *doc = content->layout;
	ثابت ساخت gui_layout_جدول *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			چوڑائی, اونچائی, nsurl_access(content_get_url(
					&content->بنیاد)));

	layout_minmax_بلاک(doc, font_func, content);

	layout_بلاک_ڈھونڈو_dimensions(&content->unit_len_ctx,
			چوڑائی, اونچائی, 0, 0, doc);
	doc->x = doc->margin[بایاں_2] + doc->border[بایاں_2].چوڑائی;
	doc->y = doc->margin[چوٹی_2] + doc->border[چوٹی_2].چوڑائی;
	چوڑائی -= doc->margin[بایاں_2] + doc->border[بایاں_2].چوڑائی +
			doc->padding[بایاں_2] + doc->padding[دایاں_2] +
			doc->border[دایاں_2].چوڑائی + doc->margin[دایاں_2];
	اگر (چوڑائی < 0) {
		چوڑائی = 0;
	}
	doc->چوڑائی = چوڑائی;

	ret = layout_بلاک_context(doc, اونچائی, content);

	/* make <html> and <body> fill available height */
	اگر (doc->y + doc->padding[چوٹی_2] + doc->اونچائی + doc->padding[BOTTOM] +
			doc->border[BOTTOM].چوڑائی + doc->margin[BOTTOM] <
			اونچائی) {
		doc->اونچائی = اونچائی - (doc->y + doc->padding[چوٹی_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].چوڑائی +
				doc->margin[BOTTOM]);
		اگر (doc->children)
			doc->children->اونچائی = doc->اونچائی -
					(doc->children->margin[چوٹی_2] +
					 doc->children->border[چوٹی_2].چوڑائی +
					 doc->children->padding[چوٹی_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].چوڑائی +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	واپس ret;
}
