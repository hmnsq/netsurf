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
ساكن فراغ layout_minmax_بلاک(
		بنية box *بلاک,
		ثابت بنية gui_layout_جدول *font_func,
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
			     صحيح *چوڑائی, صحيح *اچائی,
			     صحيح min_چوڑائی, صحيح max_چوڑائی,
			     صحيح min_اچائی, صحيح max_اچائی)
{
	أكد(box->object != عدم);
	أكد(چوڑائی != عدم && اچائی != عدم);

	إذا (*چوڑائی == AUTO && *اچائی == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		صحيح intrinsic_چوڑائی = content_get_چوڑائی(box->object);
		صحيح intrinsic_اچائی = content_get_اچائی(box->object);

		/* use intrinsic dimensions */
		*چوڑائی = intrinsic_چوڑائی;
		*اچائی = intrinsic_اچائی;

		/* Deal with min/max-width first */
		إذا (min_چوڑائی >  0 && min_چوڑائی > *چوڑائی) {
			*چوڑائی = min_چوڑائی;
			scaled = true;
		}
		إذا (max_چوڑائی >= 0 && max_چوڑائی < *چوڑائی) {
			*چوڑائی = max_چوڑائی;
			scaled = true;
		}

		إذا (scaled && (intrinsic_چوڑائی != 0)) {
			/* Update height */
			*اچائی = (*چوڑائی * intrinsic_اچائی) /
					intrinsic_چوڑائی;
		}

		scaled = false;
		/* Deal with min/max-height */
		إذا (min_اچائی >  0 && min_اچائی > *اچائی) {
			*اچائی = min_اچائی;
			scaled = true;
		}
		إذا (max_اچائی >= 0 && max_اچائی < *اچائی) {
			*اچائی = max_اچائی;
			scaled = true;
		}

		إذا (scaled && (intrinsic_اچائی != 0)) {
			/* Update width */
			*چوڑائی = (*اچائی * intrinsic_چوڑائی) /
					intrinsic_اچائی;
		}

	} وإلا إذا (*چوڑائی == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		صحيح intrinsic_چوڑائی = content_get_چوڑائی(box->object);
		صحيح intrinsic_اچائی = content_get_اچائی(box->object);

		إذا (intrinsic_اچائی != 0)
			*چوڑائی = (*اچائی * intrinsic_چوڑائی) /
					intrinsic_اچائی;
		وإلا
			*چوڑائی = intrinsic_چوڑائی;

		إذا (min_چوڑائی >  0 && min_چوڑائی > *چوڑائی)
			*چوڑائی = min_چوڑائی;
		إذا (max_چوڑائی >= 0 && max_چوڑائی < *چوڑائی)
			*چوڑائی = max_چوڑائی;

	} وإلا إذا (*اچائی == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		صحيح intrinsic_چوڑائی = content_get_چوڑائی(box->object);
		صحيح intrinsic_اچائی = content_get_اچائی(box->object);

		إذا (min_چوڑائی >  0 && min_چوڑائی > *چوڑائی)
			*چوڑائی = min_چوڑائی;
		إذا (max_چوڑائی >= 0 && max_چوڑائی < *چوڑائی)
			*چوڑائی = max_چوڑائی;

		إذا (intrinsic_چوڑائی != 0)
			*اچائی = (*چوڑائی * intrinsic_اچائی) /
					intrinsic_چوڑائی;
		وإلا
			*اچائی = intrinsic_اچائی;
	}
}


/**
 * Calculate the text-indent length.
 *
 * \param  style  style of block
 * \param  width  width of containing block
 * \return  length of indent
 */
ساكن صحيح layout_متن_indent(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style, صحيح چوڑائی)
{
	css_fixed قدر = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_متن_indent(style, &قدر, &unit);

	إذا (unit == CSS_UNIT_PCT) {
		أرجع FPCT_دا_INT_TOINT(قدر, چوڑائی);
	} وإلا {
		أرجع FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
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
ساكن فراغ layout_minmax_جدول(بنية box *جدول,
		ثابت بنية gui_layout_جدول *font_func,
		ثابت html_content *content)
{
	بنانشانی صحيح i, j;
	صحيح چوڑائی;
	صحيح border_spacing_h = 0;
	صحيح جدول_min = 0, جدول_max = 0;
	صحيح extra_fixed = 0;
	عائم extra_frac = 0;
	بنية کالم *خانہ;
	بنية box *صف_گروہ, *صف, *cell;

	/* check if the widths have already been calculated */
	إذا (جدول->max_چوڑائی != UNKNOWN_MAX_چوڑائی)
		أرجع;

	إذا (جدول_calculate_کالم_types(&content->unit_len_ctx, جدول) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		أرجع;
	}
	خانہ = جدول->خانہ;

	/* start with 0 except for fixed-width columns */
	لكل (i = 0; i != جدول->columns; i++) {
		إذا (خانہ[i].type == کالم_چوڑائی_FIXED)
			خانہ[i].min = خانہ[i].max = خانہ[i].چوڑائی;
		وإلا
			خانہ[i].min = خانہ[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	إذا (css_computed_border_collapse(جدول->style) ==
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
	لكل (صف_گروہ = جدول->children; صف_گروہ; صف_گروہ =صف_گروہ->اگلا)
	لكل (صف = صف_گروہ->children; صف; صف = صف->اگلا)
	لكل (cell = صف->children; cell; cell = cell->اگلا) {
		أكد(cell->type == BOX_جدول_CELL);
		أكد(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		أكد(cell->columns != 0);

		إذا (cell->columns != 1)
			استمر;

		layout_minmax_بلاک(cell, font_func, content);
		i = cell->شروع_کالم;

		إذا (خانہ[i].positioned)
			استمر;

		/* update column min, max widths using cell widths */
		إذا (خانہ[i].min < cell->min_چوڑائی)
			خانہ[i].min = cell->min_چوڑائی;
		إذا (خانہ[i].max < cell->max_چوڑائی)
			خانہ[i].max = cell->max_چوڑائی;
	}

	/* 2nd pass: cells which span multiple columns */
	لكل (صف_گروہ = جدول->children; صف_گروہ; صف_گروہ =صف_گروہ->اگلا)
	لكل (صف = صف_گروہ->children; صف; صف = صف->اگلا)
	لكل (cell = صف->children; cell; cell = cell->اگلا) {
		بنانشانی صحيح flexible_columns = 0;
		صحيح min = 0, max = 0, fixed_چوڑائی = 0, extra;

		إذا (cell->columns == 1)
			استمر;

		layout_minmax_بلاک(cell, font_func, content);
		i = cell->شروع_کالم;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		لكل (j = 0; j != cell->columns; j++) {
			min += خانہ[i + j].min;
			إذا (خانہ[i + j].type == کالم_چوڑائی_FIXED)
				fixed_چوڑائی += خانہ[i + j].چوڑائی;
			وإلا
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		إذا (min < cell->min_چوڑائی) {
			إذا (flexible_columns == 0) {
				extra = 1 + (cell->min_چوڑائی - min) /
						cell->columns;
				لكل (j = 0; j != cell->columns; j++) {
					خانہ[i + j].min += extra;
					إذا (خانہ[i + j].max < خانہ[i + j].min)
						خانہ[i + j].max = خانہ[i + j].min;
				}
			} وإلا {
				extra = 1 + (cell->min_چوڑائی - min) /
						flexible_columns;
				لكل (j = 0; j != cell->columns; j++) {
					إذا (خانہ[i + j].type !=
							کالم_چوڑائی_FIXED) {
						خانہ[i + j].min += extra;
						إذا (خانہ[i + j].max <
								خانہ[i + j].min)
							خانہ[i + j].max =
								خانہ[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		لكل (j = 0; j != cell->columns; j++)
			max += خانہ[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		إذا (max < cell->max_چوڑائی && flexible_columns) {
			extra = 1 + (cell->max_چوڑائی - max) / flexible_columns;
			لكل (j = 0; j != cell->columns; j++)
				إذا (خانہ[i + j].type != کالم_چوڑائی_FIXED)
					خانہ[i + j].max += extra;
		}
	}

	لكل (i = 0; i != جدول->columns; i++) {
		إذا (خانہ[i].max < خانہ[i].min) {
			box_dump(stderr, جدول, 0, true);
			أكد(0);
		}
		جدول_min += خانہ[i].min;
		جدول_max += خانہ[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	إذا (css_computed_چوڑائی_px(جدول->style, &content->unit_len_ctx,
			-1, &چوڑائی) == CSS_چوڑائی_سیٹ) {
		إذا (جدول_min < چوڑائی)
			جدول_min = چوڑائی;
		إذا (جدول_max < چوڑائی)
			جدول_max = چوڑائی;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_چوڑائی(&content->unit_len_ctx,
			جدول->style, بایاں_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_چوڑائی(&content->unit_len_ctx,
			جدول->style, دایاں_2, true, true, true,
			&extra_fixed, &extra_frac);
	إذا (extra_fixed < 0)
		extra_fixed = 0;
	إذا (extra_frac < 0)
		extra_frac = 0;
	إذا (1.0 <= extra_frac)
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
ساكن مضمن bool box_has_percentage_max_چوڑائی(بنية box *b)
{
	css_unit unit = CSS_UNIT_PX;
	تعداد css_max_چوڑائی_e type;
	css_fixed قدر = 0;

	أكد(b != عدم);

	type = css_computed_max_چوڑائی(b->style, &قدر, &unit);
	أرجع ((type == CSS_MAX_چوڑائی_سیٹ) && (unit == CSS_UNIT_PCT));
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
layout_minmax_سطر(بنية box *پہلا,
		   صحيح *سطر_min,
		   صحيح *سطر_max,
		   bool پہلا_سطر,
		   bool *سطر_has_اچائی,
		   ثابت بنية gui_layout_جدول *font_func,
		   ثابت html_content *content)
{
	صحيح min = 0, max = 0, چوڑائی, اچائی, fixed;
	عائم frac;
	size_t i, j;
	بنية box *b;
	بنية box *بلاک;
	plot_font_style_t fstyle;
	bool no_wrap;

	أكد(پہلا->ماپا);
	أكد(پہلا->ماپا->ماپا);
	أكد(پہلا->ماپا->ماپا->style);

	بلاک = پہلا->ماپا->ماپا;
	no_wrap = (css_computed_white_space(بلاک->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(بلاک->style) ==
			CSS_WHITE_SPACE_PRE);

	*سطر_has_اچائی = false;

	/* corresponds to the pass 1 loop in layout_line() */
	لكل (b = پہلا; b; b = b->اگلا) {
		تعداد css_چوڑائی_e wtype;
		تعداد css_اچائی_e htype;
		تعداد css_box_sizing_e bs;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		أكد(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		إذا (b->type == BOX_BR) {
			b = b->اگلا;
			اكسر;
		}

		إذا (lh__box_is_float_box(b)) {
			أكد(b->children);
			إذا (b->children->type == BOX_جدول)
				layout_minmax_جدول(b->children, font_func,
						content);
			وإلا
				layout_minmax_بلاک(b->children, font_func,
						content);
			b->min_چوڑائی = b->children->min_چوڑائی;
			b->max_چوڑائی = b->children->max_چوڑائی;
			إذا (min < b->min_چوڑائی)
				min = b->min_چوڑائی;
			max += b->max_چوڑائی;
			استمر;
		}

		إذا (b->type == BOX_INLINE_بلاک || b->type == BOX_INLINE_FLEX) {
			layout_minmax_بلاک(b, font_func, content);
			إذا (min < b->min_چوڑائی)
				min = b->min_چوڑائی;
			max += b->max_چوڑائی;

			إذا (b->flags & HAS_اچائی)
				*سطر_has_اچائی = true;
			استمر;
		}

		أكد(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		إذا (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_چوڑائی(&content->unit_len_ctx,
					b->style, بایاں_2, true, true, true,
					&fixed, &frac);
			إذا (!b->inline_سرا)
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, دایاں_2,
						true, true, true,
						&fixed, &frac);
			إذا (0 < fixed)
				max += fixed;
			*سطر_has_اچائی = true;
			/* \todo  update min width, consider fractional extra */
		} وإلا إذا (b->type == BOX_INLINE_سرا) {
			fixed = frac = 0;
			calculate_mbp_چوڑائی(&content->unit_len_ctx,
					b->inline_سرا->style, دایاں_2,
					true, true, true,
					&fixed, &frac);
			إذا (0 < fixed)
				max += fixed;

			إذا (b->اگلا) {
				إذا (b->space == UNKNOWN_چوڑائی) {
					font_func->چوڑائی(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*سطر_has_اچائی = true;
			استمر;
		}

		إذا (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			إذا (!b->متن)
				استمر;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			إذا (b->چوڑائی == UNKNOWN_چوڑائی) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				إذا (b->ماپا->ماپا->gadget &&
						b->ماپا->ماپا->gadget->type
						== GADGET_SELECT) {
					صحيح opt_maxwidth = 0;
					بنية form_option *o;

					لكل (o = b->ماپا->ماپا->gadget->
							انکڑے.select.اشیاء; o;
							o = o->اگلا) {
						صحيح opt_چوڑائی;
						font_func->چوڑائی(&fstyle,
								o->متن,
								لڑی_لمبائی(o->متن),
								&opt_چوڑائی);

						إذا (opt_maxwidth < opt_چوڑائی)
							opt_maxwidth =opt_چوڑائی;
					}

					b->چوڑائی = opt_maxwidth;
					إذا (nsoption_bool(core_select_menu))
						b->چوڑائی += SCROLLBAR_چوڑائی;

				} وإلا {
					font_func->چوڑائی(&fstyle, b->متن,
						b->لمائی, &b->چوڑائی);
					b->flags |= MEASURED;
				}
			}
			max += b->چوڑائی;
			إذا (b->اگلا) {
				إذا (b->space == UNKNOWN_چوڑائی) {
					font_func->چوڑائی(&fstyle, " ", 1,
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
				إذا (min < b->چوڑائی)
					min = b->چوڑائی;

			} وإلا إذا (b->ماپا->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				افعل {
					لكل (j = i; j != b->لمائی &&
							b->متن[j] != ' '; j++)
						;
					font_func->چوڑائی(&fstyle, b->متن + i,
							 j - i, &چوڑائی);
					إذا (min < چوڑائی)
						min = چوڑائی;
					i = j + 1;
				} طالما (j != b->لمائی);
			}

			*سطر_has_اچائی = true;

			استمر;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		bs = css_computed_box_sizing(بلاک->style);

		/* calculate box width */
		wtype = css_computed_چوڑائی_px(b->style,
				&content->unit_len_ctx, -1, &چوڑائی);
		إذا (wtype == CSS_چوڑائی_سیٹ) {
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						بلاک->style, بایاں_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						بلاک->style, دایاں_2,
						false, true, true,
						&fixed, &frac);
				إذا (چوڑائی < fixed) {
					چوڑائی = fixed;
				}
			}
			إذا (چوڑائی < 0)
				چوڑائی = 0;
		} وإلا {
			چوڑائی = AUTO;
		}

		/*اچائی*/
		htype = css_computed_اچائی(b->style, &قدر, &unit);
		إذا (htype == CSS_اچائی_سیٹ) {
			اچائی = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					قدر, unit));
		} وإلا {
			اچائی = AUTO;
		}

		إذا (b->object || (b->flags & REPLACE_DIM)) {
			إذا (b->object) {
				صحيح عارضی_اچائی = اچائی;
				layout_get_object_dimensions(b,
						&چوڑائی, &عارضی_اچائی,
						INT_MIN, ودھ_توں_ودھ_عدد,
						INT_MIN, ودھ_توں_ودھ_عدد);
			}

			fixed = frac = 0;
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, بایاں_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, دایاں_2,
						true, false, false,
						&fixed, &frac);
			} وإلا {
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, بایاں_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, دایاں_2,
						true, true, true,
						&fixed, &frac);
			}
			إذا (0 < چوڑائی + fixed)
				چوڑائی += fixed;
		} وإلا إذا (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			إذا (چوڑائی == AUTO)
				چوڑائی = 400;

			fixed = frac = 0;
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, بایاں_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, دایاں_2,
						true, false, false,
						&fixed, &frac);
			} وإلا {
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, بایاں_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_چوڑائی(&content->unit_len_ctx,
						b->style, دایاں_2,
						true, true, true,
						&fixed, &frac);
			}

			إذا (0 < چوڑائی + fixed)
				چوڑائی += fixed;

		} وإلا {
			/* form control with no object */
			إذا (چوڑائی == AUTO)
				چوڑائی = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		إذا (min < چوڑائی && !box_has_percentage_max_چوڑائی(b))
			min = چوڑائی;
		إذا (چوڑائی > 0)
			max += چوڑائی;

		*سطر_has_اچائی = true;
	}

	إذا (پہلا_سطر) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		صحيح متن_indent = layout_متن_indent(&content->unit_len_ctx,
				پہلا->ماپا->ماپا->style, 100);
		min = (min + متن_indent < 0) ? 0 : min + متن_indent;
		max = (max + متن_indent < 0) ? 0 : max + متن_indent;
	}

	*سطر_min = min;
	*سطر_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	أكد(b != پہلا);
	أكد(0 <= *سطر_min);
	أكد(*سطر_min <= *سطر_max);
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
			       bool *has_اچائی,
			       ثابت بنية gui_layout_جدول *font_func,
			       ثابت html_content *content)
{
	بنية box *بچہ;
	صحيح سطر_min = 0, سطر_max = 0;
	صحيح min = 0, max = 0;
	bool پہلا_سطر = true;
	bool سطر_has_اچائی;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	إذا (inline_container->max_چوڑائی != UNKNOWN_MAX_چوڑائی)
		أرجع;

	*has_اچائی = false;

	لكل (بچہ = inline_container->children; بچہ; ) {
		بچہ = layout_minmax_سطر(بچہ, &سطر_min, &سطر_max,
				پہلا_سطر, &سطر_has_اچائی, font_func,
				content);
		إذا (min < سطر_min)
			min = سطر_min;
		إذا (max < سطر_max)
			max = سطر_max;
		پہلا_سطر = false;
		*has_اچائی |= سطر_has_اچائی;
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
ساكن فراغ layout_minmax_بلاک(
		بنية box *بلاک,
		ثابت بنية gui_layout_جدول *font_func,
		ثابت html_content *content)
{
	بنية box *بچہ;
	صحيح min = 0, max = 0;
	صحيح extra_fixed = 0;
	عائم extra_frac = 0;
	تعداد css_چوڑائی_e wtype = CSS_چوڑائی_AUTO;
	css_fixed چوڑائی = 0;
	css_unit wunit = CSS_UNIT_PX;
	تعداد css_اچائی_e htype = CSS_اچائی_AUTO;
	css_fixed اچائی = 0;
	css_unit hunit = CSS_UNIT_PX;
	تعداد css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool بچہ_has_اچائی = false;

	أكد(بلاک->type == BOX_بلاک ||
			بلاک->type == BOX_FLEX ||
			بلاک->type == BOX_INLINE_FLEX ||
			بلاک->type == BOX_INLINE_بلاک ||
			بلاک->type == BOX_جدول_CELL);

	/* check if the widths have already been calculated */
	إذا (بلاک->max_چوڑائی != UNKNOWN_MAX_چوڑائی)
		أرجع;

	إذا (بلاک->style != عدم) {
		wtype = css_computed_چوڑائی(بلاک->style, &چوڑائی, &wunit);
		htype = css_computed_اچائی(بلاک->style, &اچائی, &hunit);
		bs = css_computed_box_sizing(بلاک->style);
	}

	/* set whether the minimum width is of any interest for this box */
	إذا (((بلاک->ماپا && lh__box_is_float_box(بلاک->ماپا)) ||
			بلاک->type == BOX_INLINE_بلاک ||
			بلاک->type == BOX_INLINE_FLEX) &&
			wtype != CSS_چوڑائی_سیٹ) {
		/* box shrinks to fit; need minimum width */
		بلاک->flags |= NEED_MIN;
	} وإلا إذا (بلاک->type == BOX_جدول_CELL) {
		/* box shrinks to fit; need minimum width */
		بلاک->flags |= NEED_MIN;
	} وإلا إذا (بلاک->ماپا && (بلاک->ماپا->flags & NEED_MIN) &&
			wtype != CSS_چوڑائی_سیٹ) {
		/* box inside shrink-to-fit context; need minimum width */
		بلاک->flags |= NEED_MIN;
	} وإلا إذا (بلاک->ماپا && (بلاک->ماپا->type == BOX_FLEX)) {
		/* box is flex item */
		بلاک->flags |= NEED_MIN;
	}

	إذا (بلاک->gadget && (بلاک->gadget->type == GADGET_TEXTBOX ||
			بلاک->gadget->type == GADGET_PASSWORD ||
			بلاک->gadget->type == GADGET_FILE ||
			بلاک->gadget->type == GADGET_TEXTAREA) &&
			بلاک->style && wtype == CSS_چوڑائی_AUTO) {
		css_fixed ناپ = INTTOFIX(10);
		css_unit unit = CSS_UNIT_EM;

		min = max = FIXTOINT(css_unit_len2device_px(بلاک->style,
				&content->unit_len_ctx, ناپ, unit));

		بلاک->flags |= HAS_اچائی;
	}

	إذا (بلاک->gadget && (بلاک->gadget->type == GADGET_RADIO ||
			بلاک->gadget->type == GADGET_CHECKBOX) &&
			بلاک->style && wtype == CSS_چوڑائی_AUTO) {
		css_fixed ناپ = INTTOFIX(1);
		css_unit unit = CSS_UNIT_EM;

		/* form checkbox or radio button
		 * if width is AUTO, set it to 1em */
		min = max = FIXTOINT(css_unit_len2device_px(بلاک->style,
				&content->unit_len_ctx, ناپ, unit));

		بلاک->flags |= HAS_اچائی;
	}

	إذا (بلاک->object) {
		إذا (content_get_type(بلاک->object) == CONTENT_HTML) {
			layout_minmax_بلاک(html_get_box_درخت(بلاک->object),
					font_func, content);
			min = html_get_box_درخت(بلاک->object)->min_چوڑائی;
			max = html_get_box_درخت(بلاک->object)->max_چوڑائی;
		} وإلا {
			min = max = content_get_چوڑائی(بلاک->object);
		}

		بلاک->flags |= HAS_اچائی;
	} وإلا إذا (بلاک->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		بلاک->flags |= HAS_اچائی;
	} وإلا {
		/* recurse through children */
		لكل (بچہ = بلاک->children; بچہ; بچہ = بچہ->اگلا) {
			بدل (بچہ->type) {
			حالة BOX_FLEX:
			حالة BOX_بلاک:
				layout_minmax_بلاک(بچہ, font_func,
						content);
				إذا (بچہ->flags & HAS_اچائی)
					بچہ_has_اچائی = true;
				اكسر;
			حالة BOX_INLINE_CONTAINER:
				إذا (بلاک->flags & NEED_MIN)
					بچہ->flags |= NEED_MIN;

				layout_minmax_inline_container(بچہ,
						&بچہ_has_اچائی, font_func,
						content);
				إذا (بچہ_has_اچائی &&
						بچہ ==
						بچہ->ماپا->children) {
					بلاک->flags |= MAKE_اچائی;
				}
				اكسر;
			حالة BOX_جدول:
				layout_minmax_جدول(بچہ, font_func,
						content);
				/* todo: fix for zero height tables */
				بچہ_has_اچائی = true;
				بچہ->flags |= MAKE_اچائی;
				اكسر;
			افتراضي:
				أكد(0);
			}
			أكد(بچہ->max_چوڑائی != UNKNOWN_MAX_چوڑائی);

			إذا (بچہ->style &&
					(css_computed_position(بچہ->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(بچہ->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				استمر;
			}

			إذا (lh__box_is_flex_container(بلاک) &&
			    lh__flex_main_is_horizontal(بلاک)) {
				إذا (بلاک->style != عدم &&
				    css_computed_flex_wrap(بلاک->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += بچہ->min_چوڑائی;
				} وإلا {
					إذا (min < بچہ->min_چوڑائی)
						min = بچہ->min_چوڑائی;
				}
				max += بچہ->max_چوڑائی;

			} وإلا {
				إذا (min < بچہ->min_چوڑائی)
					min = بچہ->min_چوڑائی;
				إذا (max < بچہ->max_چوڑائی)
					max = بچہ->max_چوڑائی;
			}

			إذا (بچہ_has_اچائی)
				بلاک->flags |= HAS_اچائی;
		}
	}

	إذا (max < min) {
		box_dump(stderr, بلاک, 0, true);
		أكد(0);
	}

	/* fixed width takes priority */
	إذا (بلاک->type != BOX_جدول_CELL && !lh_box_is_flex_شے(بلاک)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		تعداد css_max_چوڑائی_e max_type;
		تعداد css_min_چوڑائی_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed قدر = 0;
		صحيح چوڑائی;

		إذا (css_computed_چوڑائی_px(بلاک->style, &content->unit_len_ctx,
				-1, &چوڑائی) == CSS_چوڑائی_سیٹ) {
			min = max = چوڑائی;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_چوڑائی(بلاک->style, &قدر, &unit);
		إذا (min_type == CSS_MIN_چوڑائی_سیٹ && unit != CSS_UNIT_PCT) {
			صحيح val = FIXTOINT(css_unit_len2device_px(بلاک->style,
					&content->unit_len_ctx, قدر, unit));

			إذا (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_چوڑائی(بلاک->style, &قدر, &unit);
		إذا (max_type == CSS_MAX_چوڑائی_سیٹ && unit != CSS_UNIT_PCT) {
			صحيح val = FIXTOINT(css_unit_len2device_px(بلاک->style,
					&content->unit_len_ctx, قدر, unit));

			إذا (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	إذا (htype == CSS_اچائی_سیٹ && hunit != CSS_UNIT_PCT &&
			اچائی > INTTOFIX(0)) {
		بلاک->flags |= MAKE_اچائی;
		بلاک->flags |= HAS_اچائی;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_چوڑائی(&content->unit_len_ctx, بلاک->style, بایاں_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_چوڑائی(&content->unit_len_ctx, بلاک->style, دایاں_2,
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

	calculate_mbp_چوڑائی(&content->unit_len_ctx, بلاک->style, بایاں_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_چوڑائی(&content->unit_len_ctx, بلاک->style, دایاں_2,
			true, false, false, &extra_fixed, &extra_frac);

	إذا (extra_fixed < 0)
		extra_fixed = 0;
	إذا (extra_frac < 0)
		extra_frac = 0;
	إذا (1.0 <= extra_frac)
		extra_frac = 0.9;
	إذا (بلاک->style != عدم &&
			(css_computed_float(بلاک->style) == CSS_FLOAT_بایاں ||
			css_computed_float(بلاک->style) == CSS_FLOAT_دایاں)) {
		/* floated boxs */
		بلاک->min_چوڑائی = min + extra_fixed;
		بلاک->max_چوڑائی = max + extra_fixed;
	} وإلا {
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
ساكن بنية box*
layout_اگلا_margin_بلاک(ثابت css_unit_ctx *unit_len_ctx,
			 بنية box *box,
			 بنية box *بلاک,
			 صحيح viewport_اچائی,
			 صحيح *max_pos_margin,
			 صحيح *max_neg_margin)
{
	أكد(بلاک != عدم);

	طالما (box != عدم) {

		إذا (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			إذا (box->style) {
				layout_لبھو_dimensions(unit_len_ctx,
						box->ماپا->چوڑائی,
						viewport_اچائی, box,
						box->style,
						عدم, عدم, عدم, عدم,
						عدم, عدم, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				إذا (*max_pos_margin < box->margin[چوٹی_2])
					*max_pos_margin = box->margin[چوٹی_2];
				وإلا إذا (*max_neg_margin < -box->margin[چوٹی_2])
					*max_neg_margin = -box->margin[چوٹی_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			إذا (box->flags & MAKE_اچائی ||
					box->border[چوٹی_2].چوڑائی ||
					box->padding[چوٹی_2] ||
					(box->style &&
					css_computed_حد_توں_ودھ_y(box->style) !=
					CSS_حد_توں_ودھ_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_پہلا_بچہ(box))) {
				/* Collapse to this box; return it */
				أرجع box;
			}
		}


		/* Find next box */
		إذا (box->type == BOX_بلاک && !box->object && box->children &&
				box->style &&
				css_computed_حد_توں_ودھ_y(box->style) ==
				CSS_حد_توں_ودھ_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} وإلا {
			إذا (!box->اگلا) {
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

					box = box->ماپا;
				} طالما (box != بلاک && !box->اگلا);

				إذا (box == بلاک) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					أرجع بلاک;
				}
			}

			/* Apply bottom margin */
			إذا (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			وإلا إذا (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->اگلا;

			/* Get margins */
			إذا (box->style) {
				layout_لبھو_dimensions(unit_len_ctx,
						box->ماپا->چوڑائی,
						viewport_اچائی, box,
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
	لكل (; fl; fl = fl->اگلا_float) {
		إذا ((clear == CSS_CLEAR_بایاں || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_بایاں)
			إذا (y < fl->y + fl->اچائی)
				y = fl->y + fl->اچائی;
		إذا ((clear == CSS_CLEAR_دایاں || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_دایاں)
			إذا (y < fl->y + fl->اچائی)
				y = fl->y + fl->اچائی;
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
لبھو_sides(بنية box *fl,
	   صحيح y0, صحيح y1,
	   صحيح *x0, صحيح *x1,
	   بنية box **بایاں,
	   بنية box **دایاں)
{
	صحيح fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*بایاں = *دایاں = 0;
	لكل (; fl; fl = fl->اگلا_float) {
		fy1 = fl->y + fl->اچائی;
		إذا (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			أرجع;
		}
		fy0 = fl->y;
		إذا (y0 < fy1 && fy0 <= y1) {
			إذا (fl->type == BOX_FLOAT_بایاں) {
				fx1 = fl->x + fl->چوڑائی;
				إذا (*x0 < fx1) {
					*x0 = fx1;
					*بایاں = fl;
				}
			} وإلا {
				fx0 = fl->x;
				إذا (fx0 < *x1) {
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
ساكن صحيح
layout_solve_چوڑائی(بنية box *box,
		   صحيح دستیاب_چوڑائی,
		   صحيح چوڑائی,
		   صحيح lm,
		   صحيح rm,
		   صحيح max_چوڑائی,
		   صحيح min_چوڑائی)
{
	bool auto_چوڑائی = false;

	/* Increase specified left/right margins */
	إذا (box->margin[بایاں_2] != AUTO && box->margin[بایاں_2] < lm &&
			box->margin[بایاں_2] >= 0)
		box->margin[بایاں_2] = lm;
	إذا (box->margin[دایاں_2] != AUTO && box->margin[دایاں_2] < rm &&
			box->margin[دایاں_2] >= 0)
		box->margin[دایاں_2] = rm;

	/* Find width */
	إذا (چوڑائی == AUTO) {
		صحيح margin_بایاں = box->margin[بایاں_2];
		صحيح margin_دایاں = box->margin[دایاں_2];

		إذا (margin_بایاں == AUTO) {
			margin_بایاں = lm;
		}
		إذا (margin_دایاں == AUTO) {
			margin_دایاں = rm;
		}

		چوڑائی = دستیاب_چوڑائی -
				(margin_بایاں + box->border[بایاں_2].چوڑائی +
				box->padding[بایاں_2] + box->padding[دایاں_2] +
				box->border[دایاں_2].چوڑائی + margin_دایاں);
		چوڑائی = چوڑائی < 0 ? 0 : چوڑائی;
		auto_چوڑائی = true;
	}

	إذا (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی) {
		/* max-width is admissable and width exceeds max-width */
		چوڑائی = max_چوڑائی;
		auto_چوڑائی = false;
	}

	إذا (min_چوڑائی > 0 && چوڑائی < min_چوڑائی) {
		/* min-width is admissable and width is less than max-width */
		چوڑائی = min_چوڑائی;
		auto_چوڑائی = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	إذا (auto_چوڑائی) {
		/* any other 'auto' become 0 or the minimum required values */
		إذا (box->margin[بایاں_2] == AUTO) {
			box->margin[بایاں_2] = lm;
		}
		إذا (box->margin[دایاں_2] == AUTO) {
			box->margin[دایاں_2] = rm;
		}
		أرجع چوڑائی;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	إذا (box->margin[بایاں_2] != AUTO && box->margin[دایاں_2] != AUTO &&
			box->ماپا != عدم && box->ماپا->style != عدم) {
		بدل (css_computed_متن_align(box->ماپا->style)) {
		حالة CSS_متن_ALIGN_LIBCSS_دایاں:
			box->margin[بایاں_2] = AUTO;
			box->margin[دایاں_2] = 0;
			اكسر;
		حالة CSS_متن_ALIGN_LIBCSS_CENTER:
			box->margin[بایاں_2] = box->margin[دایاں_2] = AUTO;
			اكسر;
		حالة CSS_متن_ALIGN_LIBCSS_بایاں:
			box->margin[بایاں_2] = 0;
			box->margin[دایاں_2] = AUTO;
			اكسر;
		افتراضي:
			/* Leave it alone; no HTML alignment */
			اكسر;
		}
	}

	إذا (box->margin[بایاں_2] == AUTO && box->margin[دایاں_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[بایاں_2] = box->margin[دایاں_2] =
				(دستیاب_چوڑائی - lm - rm -
				(box->border[بایاں_2].چوڑائی + box->padding[بایاں_2] +
				چوڑائی + box->padding[دایاں_2] +
				box->border[دایاں_2].چوڑائی)) / 2;

		إذا (box->margin[بایاں_2] < 0) {
			box->margin[دایاں_2] += box->margin[بایاں_2];
			box->margin[بایاں_2] = 0;
		}

		box->margin[بایاں_2] += lm;

	} وإلا إذا (box->margin[بایاں_2] == AUTO) {
		box->margin[بایاں_2] = دستیاب_چوڑائی - lm -
				(box->border[بایاں_2].چوڑائی + box->padding[بایاں_2] +
				چوڑائی + box->padding[دایاں_2] +
				box->border[دایاں_2].چوڑائی + box->margin[دایاں_2]);
		box->margin[بایاں_2] = box->margin[بایاں_2] < lm
				? lm : box->margin[بایاں_2];
	} وإلا {
		/* margin-right auto or "over-constrained" */
		box->margin[دایاں_2] = دستیاب_چوڑائی - rm -
				(box->margin[بایاں_2] + box->border[بایاں_2].چوڑائی +
				 box->padding[بایاں_2] + چوڑائی +
				 box->padding[دایاں_2] +
				 box->border[دایاں_2].چوڑائی);
	}

	أرجع چوڑائی;
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
layout_بلاک_لبھو_dimensions(ثابت css_unit_ctx *unit_len_ctx,
			     صحيح دستیاب_چوڑائی,
			     صحيح viewport_اچائی,
			     صحيح lm,
			     صحيح rm,
			     بنية box *box)
{
	صحيح چوڑائی, max_چوڑائی, min_چوڑائی;
	صحيح اچائی, max_اچائی, min_اچائی;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	ثابت css_computed_style *style = box->style;

	layout_لبھو_dimensions(unit_len_ctx, دستیاب_چوڑائی, viewport_اچائی, box,
			style, &چوڑائی, &اچائی, &max_چوڑائی, &min_چوڑائی,
			&max_اچائی, &min_اچائی, margin, padding, border);

	إذا (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &چوڑائی, &اچائی,
				min_چوڑائی, max_چوڑائی, min_اچائی, max_اچائی);
	}

	box->چوڑائی = layout_solve_چوڑائی(box, دستیاب_چوڑائی, چوڑائی, lm, rm,
			max_چوڑائی, min_چوڑائی);
	box->اچائی = اچائی;

	إذا (margin[چوٹی_2] == AUTO)
		margin[چوٹی_2] = 0;
	إذا (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
ساكن فراغ layout_بلاک_جوڑو_scrollbar(بنية box *box, صحيح which)
{
	تعداد css_حد_توں_ودھ_e حد_توں_ودھ_x, حد_توں_ودھ_y;

	أكد(box->type == BOX_بلاک && (which == دایاں_2 || which == BOTTOM));

	إذا (box->style == عدم)
		أرجع;

	حد_توں_ودھ_x = css_computed_حد_توں_ودھ_x(box->style);
	حد_توں_ودھ_y = css_computed_حد_توں_ودھ_y(box->style);

	إذا (which == BOTTOM &&
			(حد_توں_ودھ_x == CSS_حد_توں_ودھ_SCROLL ||
			 حد_توں_ودھ_x == CSS_حد_توں_ودھ_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		إذا (box->اچائی != AUTO &&
				(حد_توں_ودھ_x == CSS_حد_توں_ودھ_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_چوڑائی;
		}

	} وإلا إذا (which == دایاں_2 &&
			(حد_توں_ودھ_y == CSS_حد_توں_ودھ_SCROLL ||
			 حد_توں_ودھ_y == CSS_حد_توں_ودھ_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		تعداد css_اچائی_e htype;
		css_fixed اچائی = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_اچائی(box->style, &اچائی, &hunit);

		إذا (which == دایاں_2 && box->چوڑائی != AUTO &&
				htype == CSS_اچائی_سیٹ &&
				(حد_توں_ودھ_y == CSS_حد_توں_ودھ_SCROLL ||
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
ساكن فراغ layout_move_children(بنية box *box, صحيح x, صحيح y)
{
	أكد(box);

	لكل (box = box->children; box; box = box->اگلا) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_جدول(
		بنية box *جدول,
		صحيح دستیاب_چوڑائی,
		html_content *content)
{
	بنانشانی صحيح columns = جدول->columns;  /* total columns */
	بنانشانی صحيح i;
	بنانشانی صحيح *صف_وسعت;
	صحيح *excess_y;
	صحيح جدول_چوڑائی, min_چوڑائی = 0, max_چوڑائی = 0;
	صحيح required_چوڑائی = 0;
	صحيح x, remainder = 0, گنتی = 0;
	صحيح جدول_اچائی = 0;
	صحيح min_اچائی = 0;
	صحيح *xs;  /* array of column x positions */
	صحيح auto_چوڑائی;
	صحيح وادھو_چوڑائی;
	صحيح relative_جوڑ = 0;
	صحيح border_spacing_h = 0, border_spacing_v = 0;
	صحيح وادھو_اچائی;
	صحيح positioned_columns = 0;
	بنية box *containing_بلاک = عدم;
	بنية box *c;
	بنية box *صف;
	بنية box *صف_گروہ;
	بنية box **صف_وسعت_cell;
	بنية کالم *خانہ;
	ثابت css_computed_style *style = جدول->style;
	تعداد css_اچائی_e htype;
	css_fixed قدر = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(جدول->type == BOX_جدول);
	أكد(style);
	أكد(جدول->children && جدول->children->children);
	أكد(columns);

	/* allocate working buffers */
	خانہ = حافظہ_دیو(columns * حجم خانہ[0]);
	excess_y = حافظہ_دیو(columns * حجم excess_y[0]);
	صف_وسعت = حافظہ_دیو(columns * حجم صف_وسعت[0]);
	صف_وسعت_cell = حافظہ_دیو(columns * حجم صف_وسعت_cell[0]);
	xs = حافظہ_دیو((columns + 1) * حجم xs[0]);
	إذا (!خانہ || !xs || !صف_وسعت || !excess_y || !صف_وسعت_cell) {
		حرر(خانہ);
		حرر(excess_y);
		حرر(صف_وسعت);
		حرر(صف_وسعت_cell);
		حرر(xs);
		أرجع false;
	}

	حافظہ_نقل(خانہ, جدول->خانہ, حجم(خانہ[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_لبھو_dimensions(&content->unit_len_ctx, دستیاب_چوڑائی, -1, جدول,
			style, 0, 0, 0, 0, 0, 0, جدول->margin, جدول->padding,
			جدول->border);
	لكل (صف_گروہ = جدول->children; صف_گروہ;
			صف_گروہ = صف_گروہ->اگلا) {
		لكل (صف = صف_گروہ->children; صف; صف = صف->اگلا) {
			لكل (c = صف->children; c; c = c->اگلا) {
				تعداد css_حد_توں_ودھ_e حد_توں_ودھ_x;
				تعداد css_حد_توں_ودھ_e حد_توں_ودھ_y;

				أكد(c->style);
				جدول_ورتیا_border_for_cell(
						&content->unit_len_ctx, c);
				layout_لبھو_dimensions(&content->unit_len_ctx,
						دستیاب_چوڑائی, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				حد_توں_ودھ_x = css_computed_حد_توں_ودھ_x(c->style);
				حد_توں_ودھ_y = css_computed_حد_توں_ودھ_y(c->style);

				إذا (حد_توں_ودھ_x == CSS_حد_توں_ودھ_SCROLL ||
						حد_توں_ودھ_x ==
						CSS_حد_توں_ودھ_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_چوڑائی;
				}
				إذا (حد_توں_ودھ_y == CSS_حد_توں_ودھ_SCROLL ||
						حد_توں_ودھ_y ==
						CSS_حد_توں_ودھ_AUTO) {
					c->padding[دایاں_2] += SCROLLBAR_چوڑائی;
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
	إذا (css_computed_چوڑائی_px(style, &content->unit_len_ctx,
			دستیاب_چوڑائی, &جدول_چوڑائی) == CSS_چوڑائی_سیٹ) {
		/* specified width includes border */
		جدول_چوڑائی -= جدول->border[بایاں_2].چوڑائی +
				جدول->border[دایاں_2].چوڑائی;
		جدول_چوڑائی = جدول_چوڑائی < 0 ? 0 : جدول_چوڑائی;

		auto_چوڑائی = جدول_چوڑائی;
	} وإلا {
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
	htype = css_computed_اچائی(style, &قدر, &unit);
	إذا (htype == CSS_اچائی_سیٹ) {
		إذا (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			إذا (css_computed_position(جدول->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				أكد(جدول->float_container);
				containing_بلاک = جدول->float_container;
			} وإلا إذا (جدول->float_container &&
					css_computed_position(جدول->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(جدول->style) ==
						CSS_FLOAT_بایاں ||
					css_computed_float(جدول->style) ==
						CSS_FLOAT_دایاں)) {
				/* Table is a float */
				أكد(جدول->ماپا && جدول->ماپا->ماپا &&
						جدول->ماپا->ماپا->ماپا);
				containing_بلاک =
						جدول->ماپا->ماپا->ماپا;
			} وإلا إذا (جدول->ماپا && جدول->ماپا->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_بلاک = جدول->ماپا;
			} وإلا إذا (جدول->ماپا && جدول->ماپا->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				أكد(جدول->ماپا->ماپا);
				containing_بلاک = جدول->ماپا->ماپا;
			}

			إذا (containing_بلاک) {
				css_fixed ignored = 0;

				htype = css_computed_اچائی(
						containing_بلاک->style,
						&ignored, &unit);
			}

			إذا (containing_بلاک &&
					containing_بلاک->اچائی != AUTO &&
					(css_computed_position(جدول->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_اچائی_سیٹ)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_اچائی = FPCT_دا_INT_TOINT(قدر,
						containing_بلاک->اچائی);
			}
		} وإلا {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_اچائی = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					قدر, unit));
		}
	}

	/* calculate width required by cells */
	لكل (i = 0; i != columns; i++) {

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
				      })[خانہ[i].type],
		      خانہ[i].چوڑائی,
		      خانہ[i].min,
		      خانہ[i].max);


		إذا (خانہ[i].positioned) {
			positioned_columns++;
			استمر;
		} وإلا إذا (خانہ[i].type == کالم_چوڑائی_FIXED) {
			إذا (خانہ[i].چوڑائی < خانہ[i].min)
				خانہ[i].چوڑائی = خانہ[i].max = خانہ[i].min;
			وإلا
				خانہ[i].min = خانہ[i].max = خانہ[i].چوڑائی;
			required_چوڑائی += خانہ[i].چوڑائی;
		} وإلا إذا (خانہ[i].type == کالم_چوڑائی_PERCENT) {
			صحيح چوڑائی = خانہ[i].چوڑائی * auto_چوڑائی / 100;
			required_چوڑائی += خانہ[i].min < چوڑائی ? چوڑائی :
					خانہ[i].min;
		} وإلا
			required_چوڑائی += خانہ[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_چوڑائی);
	}
	required_چوڑائی += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", جدول_چوڑائی,
	      جدول->min_چوڑائی, جدول->max_چوڑائی, auto_چوڑائی, required_چوڑائی);

	إذا (auto_چوڑائی < required_چوڑائی) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		لكل (i = 0; i != columns; i++) {
			إذا (خانہ[i].type == کالم_چوڑائی_RELATIVE)
				استمر;
			إذا (خانہ[i].type == کالم_چوڑائی_PERCENT) {
				خانہ[i].max = auto_چوڑائی * خانہ[i].چوڑائی / 100;
				إذا (خانہ[i].max < خانہ[i].min)
					خانہ[i].max = خانہ[i].min;
			}
			min_چوڑائی += خانہ[i].min;
			max_چوڑائی += خانہ[i].max;
		}
	} وإلا {
		/* take percentages exactly */
		لكل (i = 0; i != columns; i++) {
			إذا (خانہ[i].type == کالم_چوڑائی_RELATIVE)
				استمر;
			إذا (خانہ[i].type == کالم_چوڑائی_PERCENT) {
				صحيح چوڑائی = auto_چوڑائی * خانہ[i].چوڑائی / 100;
				إذا (چوڑائی < خانہ[i].min)
					چوڑائی = خانہ[i].min;
				خانہ[i].min = خانہ[i].چوڑائی = خانہ[i].max = چوڑائی;
				خانہ[i].type = کالم_چوڑائی_FIXED;
			}
			min_چوڑائی += خانہ[i].min;
			max_چوڑائی += خانہ[i].max;
		}
	}

	/* allocate relative widths */
	وادھو_چوڑائی = auto_چوڑائی;
	لكل (i = 0; i != columns; i++) {
		إذا (خانہ[i].type == کالم_چوڑائی_RELATIVE)
			relative_جوڑ += خانہ[i].چوڑائی;
		وإلا إذا (خانہ[i].type == کالم_چوڑائی_FIXED)
			وادھو_چوڑائی -= خانہ[i].چوڑائی;
		وإلا
			وادھو_چوڑائی -= خانہ[i].min;
	}
	وادھو_چوڑائی -= (columns + 1) * border_spacing_h;
	إذا (relative_جوڑ != 0) {
		إذا (وادھو_چوڑائی < 0)
			وادھو_چوڑائی = 0;
		لكل (i = 0; i != columns; i++) {
			إذا (خانہ[i].type == کالم_چوڑائی_RELATIVE) {
				خانہ[i].min = ceil(خانہ[i].max =
						(عائم) وادھو_چوڑائی
						* (عائم) خانہ[i].چوڑائی
						/ relative_جوڑ);
				min_چوڑائی += خانہ[i].min;
				max_چوڑائی += خانہ[i].max;
			}
		}
	}
	min_چوڑائی += (columns + 1) * border_spacing_h;
	max_چوڑائی += (columns + 1) * border_spacing_h;

	إذا (auto_چوڑائی <= min_چوڑائی) {
		/* not enough space: minimise column widths */
		لكل (i = 0; i < columns; i++) {
			خانہ[i].چوڑائی = خانہ[i].min;
		}
		جدول_چوڑائی = min_چوڑائی;
	} وإلا إذا (max_چوڑائی <= auto_چوڑائی) {
		/* more space than maximum width */
		إذا (جدول_چوڑائی == AUTO) {
			/* for auto-width tables, make columns max width */
			لكل (i = 0; i < columns; i++) {
				خانہ[i].چوڑائی = خانہ[i].max;
			}
			جدول_چوڑائی = max_چوڑائی;
		} وإلا {
			/* for fixed-width tables, distribute the extra space
			 * too */
			بنانشانی صحيح flexible_columns = 0;
			لكل (i = 0; i != columns; i++)
				إذا (خانہ[i].type != کالم_چوڑائی_FIXED)
					flexible_columns++;
			إذا (flexible_columns == 0) {
				صحيح extra = (جدول_چوڑائی - max_چوڑائی) / columns;
				remainder = (جدول_چوڑائی - max_چوڑائی) -
						(extra * columns);
				لكل (i = 0; i != columns; i++) {
					خانہ[i].چوڑائی = خانہ[i].max + extra;
					گنتی -= remainder;
					إذا (گنتی < 0) {
						خانہ[i].چوڑائی++;
						گنتی += columns;
					}
				}

			} وإلا {
				صحيح extra = (جدول_چوڑائی - max_چوڑائی) /
						flexible_columns;
				remainder = (جدول_چوڑائی - max_چوڑائی) -
						(extra * flexible_columns);
				لكل (i = 0; i != columns; i++)
					إذا (خانہ[i].type != کالم_چوڑائی_FIXED) {
						خانہ[i].چوڑائی = خانہ[i].max +
								extra;
						گنتی -= remainder;
						إذا (گنتی < 0) {
							خانہ[i].چوڑائی++;
							گنتی += flexible_columns;
						}
					}
			}
		}
	} وإلا {
		/* space between min and max: fill it exactly */
		عائم scale = (عائم) (auto_چوڑائی - min_چوڑائی) /
				(عائم) (max_چوڑائی - min_چوڑائی);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		لكل (i = 0; i < columns; i++) {
			خانہ[i].چوڑائی = خانہ[i].min + (صحيح) (0.5 +
					(خانہ[i].max - خانہ[i].min) * scale);
		}
		جدول_چوڑائی = auto_چوڑائی;
	}

	xs[0] = x = border_spacing_h;
	لكل (i = 0; i != columns; i++) {
		إذا (!خانہ[i].positioned)
			x += خانہ[i].چوڑائی + border_spacing_h;
		xs[i + 1] = x;
		صف_وسعت[i] = 0;
		excess_y[i] = 0;
		صف_وسعت_cell[i] = 0;
	}

	/* position cells */
	جدول_اچائی = border_spacing_v;
	لكل (صف_گروہ = جدول->children; صف_گروہ;
			صف_گروہ = صف_گروہ->اگلا) {
		صحيح صف_گروہ_اچائی = 0;
		لكل (صف = صف_گروہ->children; صف; صف = صف->اگلا) {
			صحيح صف_اچائی = 0;

			htype = css_computed_اچائی(صف->style, &قدر, &unit);
			إذا (htype == CSS_اچائی_سیٹ && unit != CSS_UNIT_PCT) {
				صف_اچائی = FIXTOINT(css_unit_len2device_px(
						صف->style,
						&content->unit_len_ctx,
						قدر, unit));
			}
			لكل (c = صف->children; c; c = c->اگلا) {
				أكد(c->style);
				c->چوڑائی = xs[c->شروع_کالم + c->columns] -
						xs[c->شروع_کالم] -
						border_spacing_h -
						c->border[بایاں_2].چوڑائی -
						c->padding[بایاں_2] -
						c->padding[دایاں_2] -
						c->border[دایاں_2].چوڑائی;
				c->float_children = 0;
				c->cached_رکھو_ہیٹھاں_level = 0;

				c->اچائی = AUTO;
				إذا (!layout_بلاک_context(c, -1, content)) {
					حرر(خانہ);
					حرر(excess_y);
					حرر(صف_وسعت);
					حرر(صف_وسعت_cell);
					حرر(xs);
					أرجع false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->اچائی;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_اچائی(c->style,
						&قدر, &unit);

				إذا (htype == CSS_اچائی_سیٹ &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					صحيح h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							قدر, unit));
					إذا (c->اچائی < h)
						c->اچائی = h;
				}
				/* specified row height is treated as a minimum
				 */
				إذا (c->اچائی < صف_اچائی)
					c->اچائی = صف_اچائی;
				c->x = xs[c->شروع_کالم] +
						c->border[بایاں_2].چوڑائی;
				c->y = c->border[چوٹی_2].چوڑائی;
				لكل (i = 0; i != c->columns; i++) {
					صف_وسعت[c->شروع_کالم + i] = c->صفاں;
					excess_y[c->شروع_کالم + i] =
							c->border[چوٹی_2].چوڑائی +
							c->padding[چوٹی_2] +
							c->اچائی +
							c->padding[BOTTOM] +
							c->border[BOTTOM].چوڑائی;
					صف_وسعت_cell[c->شروع_کالم + i] = 0;
				}
				صف_وسعت_cell[c->شروع_کالم] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[چوٹی_2].چوڑائی -
						c->padding[چوٹی_2] -
						c->اچائی -
						c->border[BOTTOM].چوڑائی;
			}
			لكل (i = 0; i != columns; i++)
				إذا (صف_وسعت[i] != 0)
					صف_وسعت[i]--;
				وإلا
					صف_وسعت_cell[i] = 0;
			إذا (صف->اگلا || صف_گروہ->اگلا) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				لكل (i = 0; i != columns; i++)
					إذا (صف_وسعت[i] == 0 && صف_اچائی <
							excess_y[i])
						صف_اچائی = excess_y[i];
			} وإلا {
				/* except in the last row */
				لكل (i = 0; i != columns; i++)
					إذا (صف_اچائی < excess_y[i])
						صف_اچائی = excess_y[i];
			}
			لكل (i = 0; i != columns; i++) {
				إذا (صف_اچائی < excess_y[i])
					excess_y[i] -= صف_اچائی;
				وإلا
					excess_y[i] = 0;
				إذا (صف_وسعت_cell[i] != 0)
					صف_وسعت_cell[i]->padding[BOTTOM] +=
							صف_اچائی +
							border_spacing_v;
			}

			صف->x = 0;
			صف->y = صف_گروہ_اچائی;
			صف->چوڑائی = جدول_چوڑائی;
			صف->اچائی = صف_اچائی;
			صف_گروہ_اچائی += صف_اچائی + border_spacing_v;
		}
		صف_گروہ->x = 0;
		صف_گروہ->y = جدول_اچائی;
		صف_گروہ->چوڑائی = جدول_چوڑائی;
		صف_گروہ->اچائی = صف_گروہ_اچائی;
		جدول_اچائی += صف_گروہ_اچائی;
	}
	/* Table height is either the height of the contents, or specified
	 * height if greater */
	جدول_اچائی = max(جدول_اچائی, min_اچائی);
	/** \todo distribute spare height over the row groups / rows / cells */

	/* perform vertical alignment */
	لكل (صف_گروہ = جدول->children; صف_گروہ;
			صف_گروہ = صف_گروہ->اگلا) {
		لكل (صف = صف_گروہ->children; صف; صف = صف->اگلا) {
			لكل (c = صف->children; c; c = c->اگلا) {
				تعداد css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				وادھو_اچائی = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->اچائی - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &قدر, &unit);

				بدل (vertical_align) {
				حالة CSS_VERTICAL_ALIGN_SUB:
				حالة CSS_VERTICAL_ALIGN_SUPER:
				حالة CSS_VERTICAL_ALIGN_متن_چوٹی:
				حالة CSS_VERTICAL_ALIGN_متن_BOTTOM:
				حالة CSS_VERTICAL_ALIGN_سیٹ:
				حالة CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				حالة CSS_VERTICAL_ALIGN_چوٹی:
					اكسر;
				حالة CSS_VERTICAL_ALIGN_درمیان:
					c->padding[چوٹی_2] += وادھو_اچائی / 2;
					c->padding[BOTTOM] -= وادھو_اچائی / 2;
					layout_move_children(c, 0,
							وادھو_اچائی / 2);
					اكسر;
				حالة CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[چوٹی_2] += وادھو_اچائی;
					c->padding[BOTTOM] -= وادھو_اچائی;
					layout_move_children(c, 0,
							وادھو_اچائی);
					اكسر;
				حالة CSS_VERTICAL_ALIGN_INHERIT:
					أكد(0);
					اكسر;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	إذا (جدول->margin[چوٹی_2] == AUTO)
		جدول->margin[چوٹی_2] = 0;
	إذا (جدول->margin[BOTTOM] == AUTO)
		جدول->margin[BOTTOM] = 0;

	حرر(خانہ);
	حرر(excess_y);
	حرر(صف_وسعت);
	حرر(صف_وسعت_cell);
	حرر(xs);

	جدول->چوڑائی = جدول_چوڑائی;
	جدول->اچائی = جدول_اچائی;

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
ساكن bool layout_apply_minmax_اچائی(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *box,
		بنية box *container)
{
	صحيح h;
	بنية box *containing_بلاک = عدم;
	bool updated = false;

	/* Find containing block for percentage heights */
	إذا (box->style != عدم && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		أكد(container);
		containing_بلاک = container;
	} وإلا إذا (box->float_container && box->style != عدم &&
			(css_computed_float(box->style) == CSS_FLOAT_بایاں ||
			 css_computed_float(box->style) == CSS_FLOAT_دایاں)) {
		/* Box is a float */
		أكد(box->ماپا && box->ماپا->ماپا &&
				box->ماپا->ماپا->ماپا);
		containing_بلاک = box->ماپا->ماپا->ماپا;
	} وإلا إذا (box->ماپا && box->ماپا->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_بلاک = box->ماپا;
	} وإلا إذا (box->ماپا && box->ماپا->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		أكد(box->ماپا->ماپا);
		containing_بلاک = box->ماپا->ماپا;
	}

	إذا (box->style) {
		تعداد css_اچائی_e htype = CSS_اچائی_AUTO;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		إذا (containing_بلاک) {
			htype = css_computed_اچائی(containing_بلاک->style,
					&قدر, &unit);
		}

		/* max-height */
		إذا (css_computed_max_اچائی(box->style, &قدر, &unit) ==
				CSS_MAX_اچائی_سیٹ) {
			إذا (unit == CSS_UNIT_PCT) {
				إذا (containing_بلاک &&
					containing_بلاک->اچائی != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_اچائی_سیٹ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_دا_INT_TOINT(قدر,
						containing_بلاک->اچائی);
					إذا (h < box->اچائی) {
						box->اچائی = h;
						updated = true;
					}
				}
			} وإلا {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						قدر, unit));
				إذا (h < box->اچائی) {
					box->اچائی = h;
					updated = true;
				}
			}
		}

		/* min-height */
		إذا (ns_computed_min_اچائی(box->style, &قدر, &unit) ==
				CSS_MIN_اچائی_سیٹ) {
			إذا (unit == CSS_UNIT_PCT) {
				إذا (containing_بلاک &&
					containing_بلاک->اچائی != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_اچائی_سیٹ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_دا_INT_TOINT(قدر,
						containing_بلاک->اچائی);
					إذا (h > box->اچائی) {
						box->اچائی = h;
						updated = true;
					}
				}
			} وإلا {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						قدر, unit));
				إذا (h > box->اچائی) {
					box->اچائی = h;
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
ساكن bool layout_بلاک_object(بنية box *بلاک)
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

	إذا (content_can_reformat(بلاک->object)) {
		content_reformat(بلاک->object, false, بلاک->چوڑائی, 1);
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
ساكن فراغ جوڑو_float_منزل_container(بنية box *cont, بنية box *b)
{
	بنية box *box = cont->float_children;
	صحيح b_bottom = b->y + b->اچائی;

	أكد(b->type == BOX_FLOAT_بایاں || b->type == BOX_FLOAT_دایاں);

	إذا (box == عدم) {
		/* No other float children */
		b->اگلا_float = عدم;
		cont->float_children = b;
		أرجع;
	} وإلا إذا (b_bottom >= box->y + box->اچائی) {
		/* Goes at start of list */
		b->اگلا_float = cont->float_children;
		cont->float_children = b;
	} وإلا {
		بنية box *prev = عدم;
		طالما (box != عدم && b_bottom < box->y + box->اچائی) {
			prev = box;
			box = box->اگلا_float;
		}
		إذا (prev != عدم) {
			b->اگلا_float = prev->اگلا_float;
			prev->اگلا_float = b;
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
layout_متن_box_ونڈو(html_content *content,
		      plot_font_style_t *fstyle,
		      بنية box *ونڈو_box,
		      size_t new_لمائی,
		      صحيح new_چوڑائی)
{
	صحيح space_چوڑائی = ونڈو_box->space;
	بنية box *c2;
	ثابت بنية gui_layout_جدول *font_func = content->font_func;
	bool space = (ونڈو_box->متن[new_لمائی] == ' ');
	صحيح ورتیا_لمائی = new_لمائی + (space ? 1 : 0);

	إذا ((space && space_چوڑائی == 0) || space_چوڑائی == UNKNOWN_چوڑائی) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->چوڑائی(fstyle, " ", 1, &space_چوڑائی);
	}

	إذا (ونڈو_box->space == UNKNOWN_چوڑائی)
		ونڈو_box->space = space_چوڑائی;
	إذا (!space)
		space_چوڑائی = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, ونڈو_box, حجم *c2);
	إذا (!c2)
		أرجع false;
	c2->flags |= CLONE;

	/* Set remaining text in c2 */
	c2->متن += ورتیا_لمائی;

	/* Set c2 according to the remaining text */
	c2->چوڑائی -= new_چوڑائی + space_چوڑائی;
	c2->flags &= ~MEASURED; /* width has been estimated */
	c2->لمائی = ونڈو_box->لمائی - ورتیا_لمائی;

	/* Update split_box for its reduced text */
	ونڈو_box->چوڑائی = new_چوڑائی;
	ونڈو_box->flags |= MEASURED;
	ونڈو_box->لمائی = new_لمائی;
	ونڈو_box->space = space_چوڑائی;

	/* Insert c2 into box list */
	c2->اگلا = ونڈو_box->اگلا;
	ونڈو_box->اگلا = c2;
	c2->prev = ونڈو_box;
	إذا (c2->اگلا)
		c2->اگلا->prev = c2;
	وإلا
		c2->ماپا->آخری = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      ونڈو_box,
	      ونڈو_box->لمائی,
	      (صحيح)ونڈو_box->لمائی,
	      ونڈو_box->متن);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->لمائی,
	      (صحيح)c2->لمائی,
	      c2->متن);

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
layout_float_لبھو_dimensions(
		ثابت css_unit_ctx *unit_len_ctx,
		صحيح دستیاب_چوڑائی,
		ثابت css_computed_style *style,
		بنية box *box)
{
	صحيح چوڑائی, اچائی, max_چوڑائی, min_چوڑائی, max_اچائی, min_اچائی;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	تعداد css_حد_توں_ودھ_e حد_توں_ودھ_x = css_computed_حد_توں_ودھ_x(style);
	تعداد css_حد_توں_ودھ_e حد_توں_ودھ_y = css_computed_حد_توں_ودھ_y(style);
	صحيح scrollbar_چوڑائی_x =
			(حد_توں_ودھ_x == CSS_حد_توں_ودھ_SCROLL ||
			 حد_توں_ودھ_x == CSS_حد_توں_ودھ_AUTO) ?
			SCROLLBAR_چوڑائی : 0;
	صحيح scrollbar_چوڑائی_y =
			(حد_توں_ودھ_y == CSS_حد_توں_ودھ_SCROLL ||
			 حد_توں_ودھ_y == CSS_حد_توں_ودھ_AUTO) ?
			SCROLLBAR_چوڑائی : 0;

	layout_لبھو_dimensions(unit_len_ctx, دستیاب_چوڑائی, -1, box, style,
			&چوڑائی, &اچائی, &max_چوڑائی, &min_چوڑائی,
			&max_اچائی, &min_اچائی, margin, padding, border);

	إذا (margin[بایاں_2] == AUTO)
		margin[بایاں_2] = 0;
	إذا (margin[دایاں_2] == AUTO)
		margin[دایاں_2] = 0;

	إذا (box->gadget == عدم) {
		padding[دایاں_2] += scrollbar_چوڑائی_y;
		padding[BOTTOM] += scrollbar_چوڑائی_x;
	}

	إذا (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &چوڑائی, &اچائی,
				min_چوڑائی, max_چوڑائی, min_اچائی, max_اچائی);
	} وإلا إذا (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed ناپ = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		أكد(box->style);

		إذا (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			إذا (چوڑائی == AUTO) {
				ناپ = INTTOFIX(10);
				چوڑائی = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ناپ, unit));
			}
			إذا (box->gadget->type == GADGET_FILE &&
					اچائی == AUTO) {
				ناپ = FLTTOFIX(1.5);
				اچائی = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ناپ, unit));
			}
		}
		إذا (box->gadget->type == GADGET_TEXTAREA) {
			إذا (چوڑائی == AUTO) {
				ناپ = INTTOFIX(10);
				چوڑائی = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ناپ, unit));
			}
			إذا (اچائی == AUTO) {
				ناپ = INTTOFIX(4);
				اچائی = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ناپ, unit));
			}
		}
	} وإلا إذا (چوڑائی == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		چوڑائی = min(max(box->min_چوڑائی, دستیاب_چوڑائی),
				box->max_چوڑائی);

		/* width includes margin, borders and padding */
		إذا (چوڑائی == دستیاب_چوڑائی) {
			چوڑائی -= box->margin[بایاں_2] + box->border[بایاں_2].چوڑائی +
					box->padding[بایاں_2] +
					box->padding[دایاں_2] +
					box->border[دایاں_2].چوڑائی +
					box->margin[دایاں_2];
		} وإلا {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			صحيح fixed = 0;
			عائم frac = 0;
			calculate_mbp_چوڑائی(unit_len_ctx, box->style, بایاں_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_چوڑائی(unit_len_ctx, box->style, دایاں_2,
					true, true, true, &fixed, &frac);
			إذا (fixed < 0)
				fixed = 0;

			چوڑائی -= fixed;
		}

		إذا (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی) چوڑائی = max_چوڑائی;
		إذا (min_چوڑائی >  0 && چوڑائی < min_چوڑائی) چوڑائی = min_چوڑائی;

	} وإلا {
		إذا (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی) چوڑائی = max_چوڑائی;
		إذا (min_چوڑائی >  0 && چوڑائی < min_چوڑائی) چوڑائی = min_چوڑائی;
		چوڑائی -= scrollbar_چوڑائی_y;
	}

	box->چوڑائی = چوڑائی;
	box->اچائی = اچائی;

	إذا (margin[چوٹی_2] == AUTO)
		margin[چوٹی_2] = 0;
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
ساكن bool layout_float(بنية box *b, صحيح چوڑائی, html_content *content)
{
	أكد(b->type == BOX_جدول ||
	       b->type == BOX_بلاک ||
	       b->type == BOX_INLINE_بلاک ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_لبھو_dimensions(&content->unit_len_ctx, چوڑائی, b->style, b);
	إذا (b->type == BOX_جدول || b->type == BOX_INLINE_FLEX) {
		إذا (b->type == BOX_جدول) {
			إذا (!layout_جدول(b, چوڑائی, content))
				أرجع false;
		} وإلا {
			إذا (!layout_flex(b, چوڑائی, content))
				أرجع false;
		}
		إذا (b->margin[بایاں_2] == AUTO)
			b->margin[بایاں_2] = 0;
		إذا (b->margin[دایاں_2] == AUTO)
			b->margin[دایاں_2] = 0;
		إذا (b->margin[چوٹی_2] == AUTO)
			b->margin[چوٹی_2] = 0;
		إذا (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} وإلا {
		أرجع layout_بلاک_context(b, -1, content);
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
رکھو_float_ہیٹھاں(بنية box *c, صحيح چوڑائی, صحيح cx, صحيح y, بنية box *cont)
{
	صحيح x0, x1, yy;
	بنية box *بایاں;
	بنية box *دایاں;

	yy = y > cont->cached_رکھو_ہیٹھاں_level ?
			y : cont->cached_رکھو_ہیٹھاں_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      چوڑائی, cx, y, cont);

	افعل {
		y = yy;
		x0 = cx;
		x1 = cx + چوڑائی;
		لبھو_sides(cont->float_children, y, y + c->اچائی, &x0, &x1,
				&بایاں, &دایاں);
		إذا (بایاں != 0 && دایاں != 0) {
			yy = (بایاں->y + بایاں->اچائی <
					دایاں->y + دایاں->اچائی ?
					بایاں->y + بایاں->اچائی :
					دایاں->y + دایاں->اچائی);
		} وإلا إذا (بایاں == 0 && دایاں != 0) {
			yy = دایاں->y + دایاں->اچائی;
		} وإلا إذا (بایاں != 0 && دایاں == 0) {
			yy = بایاں->y + بایاں->اچائی;
		}
	} طالما ((بایاں != 0 || دایاں != 0) && (c->چوڑائی > x1 - x0));

	إذا (c->type == BOX_FLOAT_بایاں) {
		c->x = x0;
	} وإلا {
		c->x = x1 - c->چوڑائی;
	}
	c->y = y;
	cont->cached_رکھو_ہیٹھاں_level = y;
}


/**
 * Calculate line height from a style.
 */
ساكن صحيح سطر_اچائی(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style)
{
	تعداد css_سطر_اچائی_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed سطر_اچائی;

	أكد(style);

	lhtype = css_computed_سطر_اچائی(style, &lhvalue, &lhunit);
	إذا (lhtype == CSS_سطر_اچائی_عام) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_سطر_اچائی_نمبر;
	}

	إذا (lhtype == CSS_سطر_اچائی_نمبر ||
			lhunit == CSS_UNIT_PCT) {
		سطر_اچائی = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		إذا (lhtype != CSS_سطر_اچائی_نمبر)
			سطر_اچائی = FDIV(سطر_اچائی, F_100);
	} وإلا {
		أكد(lhunit != CSS_UNIT_PCT);

		سطر_اچائی = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	أرجع FIXTOINT(سطر_اچائی);
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
layout_سطر(بنية box *پہلا,
	    صحيح *چوڑائی,
	    صحيح *y,
	    صحيح cx,
	    صحيح cy,
	    بنية box *cont,
	    bool indent,
	    bool has_متن_children,
	    html_content *content,
	    بنية box **اگلا_box)
{
	صحيح اچائی, ورتیا_اچائی;
	صحيح x0 = 0;
	صحيح x1 = *چوڑائی;
	صحيح x, h, x_پچھلا;
	صحيح fy = cy;
	بنية box *بایاں;
	بنية box *دایاں;
	بنية box *b;
	بنية box *ونڈو_box = 0;
	بنية box *d;
	بنية box *br_box = 0;
	bool move_y = false;
	bool رکھو_ہیٹھاں = false;
	صحيح space_before = 0, space_after = 0;
	بنانشانی صحيح inline_گنتی = 0;
	بنانشانی صحيح i;
	ثابت بنية gui_layout_جدول *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      پہلا,
	      (صحيح)پہلا->لمائی,
	      پہلا->متن,
	      *چوڑائی,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	لبھو_sides(cont->float_children, cy, cy, &x0, &x1, &بایاں, &دایاں);
	x0 -= cx;
	x1 -= cx;

	إذا (indent)
		x0 += layout_متن_indent(&content->unit_len_ctx,
				پہلا->ماپا->ماپا->style, *چوڑائی);

	إذا (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	إذا (has_متن_children || پہلا->ماپا->ماپا->gadget)
		ورتیا_اچائی = اچائی = سطر_اچائی(&content->unit_len_ctx,
				پہلا->ماپا->ماپا->style);
	وإلا
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		ورتیا_اچائی = اچائی = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	لكل (x = 0, b = پہلا; x <= x1 - x0 && b != 0; b = b->اگلا) {
		صحيح min_چوڑائی, max_چوڑائی, min_اچائی, max_اچائی;

		أكد(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		إذا (b->type == BOX_BR)
			اكسر;

		إذا (lh__box_is_float_box(b))
			استمر;
		إذا (b->type == BOX_INLINE_بلاک &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			استمر;

		أكد(b->style != عدم);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		إذا (b->type == BOX_INLINE_بلاک ||
		    b->type == BOX_INLINE_FLEX) {
			إذا (b->max_چوڑائی != UNKNOWN_چوڑائی)
				إذا (!layout_float(b, *چوڑائی, content))
					أرجع false;
			h = b->border[چوٹی_2].چوڑائی + b->padding[چوٹی_2] + b->اچائی +
					b->padding[BOTTOM] +
					b->border[BOTTOM].چوڑائی;
			إذا (اچائی < h)
				اچائی = h;
			x += b->margin[بایاں_2] + b->border[بایاں_2].چوڑائی +
					b->padding[بایاں_2] + b->چوڑائی +
					b->padding[دایاں_2] +
					b->border[دایاں_2].چوڑائی +
					b->margin[دایاں_2];
			space_after = 0;
			استمر;
		}

		إذا (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_لبھو_dimensions(&content->unit_len_ctx,
					*چوڑائی, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			لكل (i = 0; i != 4; i++)
				إذا (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[بایاں_2] + b->border[بایاں_2].چوڑائی +
					b->padding[بایاں_2];
			إذا (b->inline_سرا) {
				b->inline_سرا->margin[دایاں_2] = b->margin[دایاں_2];
				b->inline_سرا->padding[دایاں_2] =
						b->padding[دایاں_2];
				b->inline_سرا->border[دایاں_2] =
						b->border[دایاں_2];
			} وإلا {
				x += b->padding[دایاں_2] +
						b->border[دایاں_2].چوڑائی +
						b->margin[دایاں_2];
			}
		} وإلا إذا (b->type == BOX_INLINE_سرا) {
			b->چوڑائی = 0;
			إذا (b->space == UNKNOWN_چوڑائی) {
				font_func->چوڑائی(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[دایاں_2] + b->border[دایاں_2].چوڑائی +
					b->margin[دایاں_2];
			استمر;
		}

		إذا (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->اچائی = سطر_اچائی(&content->unit_len_ctx,
					b->style ? b->style :
					b->ماپا->ماپا->style);
			إذا (اچائی < b->اچائی)
				اچائی = b->اچائی;

			إذا (!b->متن) {
				b->چوڑائی = 0;
				space_after = 0;
				استمر;
			}

			إذا (b->چوڑائی == UNKNOWN_چوڑائی) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				إذا (b->ماپا->ماپا->gadget &&
						b->ماپا->ماپا->gadget->type
						== GADGET_SELECT) {
					صحيح opt_maxwidth = 0;
					بنية form_option *o;

					لكل (o = b->ماپا->ماپا->gadget->
							انکڑے.select.اشیاء; o;
							o = o->اگلا) {
						صحيح opt_چوڑائی;
						font_func->چوڑائی(&fstyle,
								o->متن,
								لڑی_لمبائی(o->متن),
								&opt_چوڑائی);

						إذا (opt_maxwidth < opt_چوڑائی)
							opt_maxwidth =opt_چوڑائی;
					}
					b->چوڑائی = opt_maxwidth;
					إذا (nsoption_bool(core_select_menu))
						b->چوڑائی += SCROLLBAR_چوڑائی;
				} وإلا {
					font_func->چوڑائی(&fstyle, b->متن,
							b->لمائی, &b->چوڑائی);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			إذا (b->متن && (x + b->چوڑائی < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->اگلا) {
				font_func->چوڑائی(&fstyle, b->متن,
						 b->لمائی, &b->چوڑائی);
				b->flags |= MEASURED;
			}

			x += b->چوڑائی;
			إذا (b->space == UNKNOWN_چوڑائی) {
				font_func->چوڑائی(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			استمر;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		layout_لبھو_dimensions(&content->unit_len_ctx,
				*چوڑائی, -1, b, b->style,
				&b->چوڑائی, &b->اچائی,
				&max_چوڑائی, &min_چوڑائی,
				&max_اچائی, &min_اچائی,
				عدم, عدم, عدم);

		إذا (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->چوڑائی, &b->اچائی,
					min_چوڑائی, max_چوڑائی,
					min_اچائی, max_اچائی);
		} وإلا إذا (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			إذا (b->چوڑائی == AUTO)
				b->چوڑائی = 400;
			إذا (b->اچائی == AUTO)
				b->اچائی = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} وإلا {
			/* form control with no object */
			إذا (b->چوڑائی == AUTO)
				b->چوڑائی = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			إذا (b->اچائی == AUTO)
				b->اچائی = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		إذا (b->object && content_can_reformat(b->object) &&
				b->چوڑائی !=
				content_get_دستیاب_چوڑائی(b->object)) {
			css_fixed قدر = 0;
			css_unit unit = CSS_UNIT_PX;
			تعداد css_اچائی_e htype = css_computed_اچائی(b->style,
					&قدر, &unit);

			content_reformat(b->object, false, b->چوڑائی, b->اچائی);

			إذا (htype == CSS_اچائی_AUTO)
				b->اچائی = content_get_اچائی(b->object);
		}

		إذا (اچائی < b->اچائی)
			اچائی = b->اچائی;

		x += b->چوڑائی;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *چوڑائی;
	لبھو_sides(cont->float_children, cy, cy + اچائی, &x0, &x1,
			&بایاں, &دایاں);
	x0 -= cx;
	x1 -= cx;

	إذا (indent)
		x0 += layout_متن_indent(&content->unit_len_ctx,
				پہلا->ماپا->ماپا->style, *چوڑائی);

	إذا (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	لكل (x = x_پچھلا = 0, b = پہلا; x <= x1 - x0 && b; b = b->اگلا) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		إذا (b->type == BOX_INLINE_بلاک &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} وإلا إذا (lh__box_is_inline_flow(b)) {
			أكد(b->چوڑائی != UNKNOWN_چوڑائی);

			x_پچھلا = x;
			x += space_after;
			b->x = x;

			إذا ((b->type == BOX_INLINE && !b->inline_سرا) ||
					b->type == BOX_INLINE_بلاک ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[بایاں_2] + b->border[بایاں_2].چوڑائی;
				x = b->x + b->padding[بایاں_2] + b->چوڑائی +
						b->padding[دایاں_2] +
						b->border[دایاں_2].چوڑائی +
						b->margin[دایاں_2];
			} وإلا إذا (b->type == BOX_INLINE) {
				b->x += b->margin[بایاں_2] + b->border[بایاں_2].چوڑائی;
				x = b->x + b->padding[بایاں_2] + b->چوڑائی;
			} وإلا إذا (b->type == BOX_INLINE_سرا) {
				b->اچائی = b->inline_سرا->اچائی;
				x += b->padding[دایاں_2] +
						b->border[دایاں_2].چوڑائی +
						b->margin[دایاں_2];
			} وإلا {
				x += b->چوڑائی;
			}

			space_before = space_after;
			إذا (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			وإلا إذا (b->متن || b->type == BOX_INLINE_سرا) {
				إذا (b->space == UNKNOWN_چوڑائی) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->چوڑائی(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} وإلا {
				space_after = 0;
			}
			ونڈو_box = b;
			move_y = true;
			inline_گنتی++;
		} وإلا إذا (b->type == BOX_BR) {
			b->x = x;
			b->چوڑائی = 0;
			br_box = b;
			b = b->اگلا;
			ونڈو_box = 0;
			move_y = true;
			اكسر;

		} وإلا {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_رکھو_ہیٹھاں_level = 0;
			b->float_container = d->float_container = cont;

			إذا (!layout_float(d, *چوڑائی, content))
				أرجع false;

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
			b->اچائی = d->margin[چوٹی_2] + d->border[چوٹی_2].چوڑائی +
					d->padding[چوٹی_2] + d->اچائی +
					d->padding[BOTTOM] +
					d->border[BOTTOM].چوڑائی +
					d->margin[BOTTOM];

			إذا (b->چوڑائی > (x1 - x0) - x)
				رکھو_ہیٹھاں = true;
			إذا (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_بایاں && بایاں == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_دایاں &&
						دایاں == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						بایاں == 0 && دایاں == 0)) &&
					(!رکھو_ہیٹھاں ||
					(بایاں == 0 && دایاں == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_رکھو_ہیٹھاں_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				إذا (b->type == BOX_FLOAT_بایاں) {
					b->x = cx + x0;
					إذا (b->چوڑائی > 0)
						x0 += b->چوڑائی;
					بایاں = b;
				} وإلا {
					b->x = cx + x1 - b->چوڑائی;
					إذا (b->چوڑائی > 0)
						x1 -= b->چوڑائی;
					دایاں = b;
				}
				b->y = cy;
			} وإلا {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				صحيح fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_رکھو_ہیٹھاں_level) ?
						fcy :
						cont->cached_رکھو_ہیٹھاں_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + اچائی : fy;

				رکھو_float_ہیٹھاں(b, *چوڑائی, cx, fy, cont);
				fy = b->y;
				إذا (d->style && (
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
					إذا (b->type == BOX_FLOAT_بایاں)
						b->x = cx;
					وإلا
						b->x = cx + *چوڑائی - b->چوڑائی;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					إذا (fcy > cont->clear_level)
						cont->clear_level = fcy;
					إذا (b->y < fcy)
						b->y = fcy;
				}
				إذا (b->type == BOX_FLOAT_بایاں)
					بایاں = b;
				وإلا
					دایاں = b;
			}
			جوڑو_float_منزل_container(cont, b);

			ونڈو_box = 0;
		}
	}

	إذا (x1 - x0 < x && ونڈو_box) {
		/* the last box went over the end */
		size_t ونڈو = 0;
		صحيح w;
		bool no_wrap = css_computed_white_space(
				ونڈو_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				ونڈو_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_پچھلا;

		إذا (!no_wrap &&
		    (ونڈو_box->type == BOX_INLINE ||
		     ونڈو_box->type == BOX_متن) &&
		    !ونڈو_box->object &&
		    !(ونڈو_box->flags & REPLACE_DIM) &&
		    !(ونڈو_box->flags & IFRAME) &&
		    !ونڈو_box->gadget && ونڈو_box->متن) {

			font_plot_style_from_css(&content->unit_len_ctx,
					ونڈو_box->style, &fstyle);
			/** \todo handle errors */
			font_func->ونڈو(&fstyle,
					 ونڈو_box->متن,
					 ونڈو_box->لمائی,
					 x1 - x0 - x - space_before,
					 &ونڈو,
					 &w);
		}

		/* split == 0 implies that text can't be split */

		إذا (ونڈو == 0)
			w = ونڈو_box->چوڑائی;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      ونڈو_box,
		      (صحيح)ونڈو_box->لمائی,
		      ونڈو_box->متن,
		      ونڈو,
		      w,
		      بایاں,
		      دایاں,
		      inline_گنتی);

		إذا ((ونڈو == 0 || x1 - x0 <= x + space_before + w) &&
				!بایاں && !دایاں && inline_گنتی == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			إذا (ونڈو == 0 || ونڈو == ونڈو_box->لمائی) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = ونڈو_box->اگلا;
			} وإلا {
				/* cut off first word for this line */
				إذا (!layout_متن_box_ونڈو(content, &fstyle,
						ونڈو_box, ونڈو, w))
					أرجع false;
				b = ونڈو_box->اگلا;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} وإلا إذا ((ونڈو == 0 || x1 - x0 <= x + space_before + w) &&
				inline_گنتی == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			أكد(بایاں || دایاں);
			ورتیا_اچائی = 0;
			إذا (بایاں) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      بایاں->y,
				      بایاں->اچائی);

				ورتیا_اچائی = بایاں->y + بایاں->اچائی - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      ورتیا_اچائی);

			}
			إذا (دایاں && ورتیا_اچائی <
					دایاں->y + دایاں->اچائی - cy + 1)
				ورتیا_اچائی = دایاں->y + دایاں->اچائی - cy + 1;

			إذا (ورتیا_اچائی < 0)
				ورتیا_اچائی = 0;

			b = ونڈو_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} وإلا إذا (ونڈو == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = ونڈو_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} وإلا {
			/* fit as many words as possible */
			أكد(ونڈو != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (صحيح)ونڈو_box->لمائی, ونڈو_box->متن,
			      x1 - x0, ونڈو, w);

			إذا (ونڈو != ونڈو_box->لمائی) {
				إذا (!layout_متن_box_ونڈو(content, &fstyle,
						ونڈو_box, ونڈو, w))
					أرجع false;
				b = ونڈو_box->اگلا;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	بدل (css_computed_متن_align(پہلا->ماپا->ماپا->style)) {
	حالة CSS_متن_ALIGN_دایاں:
	حالة CSS_متن_ALIGN_LIBCSS_دایاں:
		x0 = x1 - x;
		اكسر;
	حالة CSS_متن_ALIGN_CENTER:
	حالة CSS_متن_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		اكسر;
	حالة CSS_متن_ALIGN_بایاں:
	حالة CSS_متن_ALIGN_LIBCSS_بایاں:
	حالة CSS_متن_ALIGN_JUSTIFY:
		/* leave on left */
		اكسر;
	حالة CSS_متن_ALIGN_DEFAULT:
		/* None; consider text direction */
		بدل (css_computed_direction(پہلا->ماپا->ماپا->style)) {
		حالة CSS_DIRECTION_LTR:
			/* leave on left */
			اكسر;
		حالة CSS_DIRECTION_RTL:
			x0 = x1 - x;
			اكسر;
		}
		اكسر;
	}

	لكل (d = پہلا; d != b; d = d->اگلا) {
		d->flags &= ~NEW_سطر;

		إذا (d->type == BOX_INLINE_بلاک &&
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
				d->type == BOX_متن ||
				d->type == BOX_INLINE_سرا) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[چوٹی_2];

			إذا (d->type == BOX_متن && d->اچائی > ورتیا_اچائی) {
				/*متن*/
				ورتیا_اچائی = d->اچائی;
			}
		} وإلا إذا ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_بلاک) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[چوٹی_2].چوڑائی + d->margin[چوٹی_2];
			h = d->margin[چوٹی_2] + d->border[چوٹی_2].چوڑائی +
					d->padding[چوٹی_2] + d->اچائی +
					d->padding[BOTTOM] +
					d->border[BOTTOM].چوڑائی +
					d->margin[BOTTOM];
			إذا (ورتیا_اچائی < h)
				ورتیا_اچائی = h;
		}
	}

	پہلا->flags |= NEW_سطر;

	أكد(b != پہلا || (move_y && 0 < ورتیا_اچائی && (بایاں || دایاں)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	لكل (d = پہلا; d != b; d = d->اگلا) {
		إذا ((d->type == BOX_INLINE && d->inline_سرا) ||
				d->type == BOX_BR ||
				d->type == BOX_متن ||
				d->type == BOX_INLINE_سرا) {
			css_fixed قدر = 0;
			css_unit unit = CSS_UNIT_PX;
			بدل (css_computed_vertical_align(d->style, &قدر,
					&unit)) {
			حالة CSS_VERTICAL_ALIGN_SUPER:
			حالة CSS_VERTICAL_ALIGN_چوٹی:
			حالة CSS_VERTICAL_ALIGN_متن_چوٹی:
				/* already at top */
				اكسر;
			حالة CSS_VERTICAL_ALIGN_SUB:
			حالة CSS_VERTICAL_ALIGN_BOTTOM:
			حالة CSS_VERTICAL_ALIGN_متن_BOTTOM:
				d->y += ورتیا_اچائی - d->اچائی;
				اكسر;
			افتراضي:
			حالة CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (ورتیا_اچائی - d->اچائی);
				اكسر;
			}
		}
	}

	/* handle clearance for br */
	إذا (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		صحيح clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		إذا (ورتیا_اچائی < clear_y - cy)
			ورتیا_اچائی = clear_y - cy;
	}

	إذا (move_y)
		*y += ورتیا_اچائی;
	*اگلا_box = b;
	*چوڑائی = x; /* return actual width */
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
ساكن bool layout_inline_container(بنية box *inline_container, صحيح چوڑائی,
		بنية box *cont, صحيح cx, صحيح cy, html_content *content)
{
	bool پہلا_سطر = true;
	bool has_متن_children;
	بنية box *c, *اگلا;
	صحيح y = 0;
	صحيح curwidth,maxwidth = چوڑائی;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      چوڑائی,
	      cont,
	      cx,
	      cy);


	has_متن_children = false;
	لكل (c = inline_container->children; c; c = c->اگلا) {
		bool is_pre = false;

		إذا (c->style) {
			تعداد css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_سطر ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		إذا ((lh__box_is_object(c) == false &&
				c->متن && (c->لمائی || is_pre)) ||
				c->type == BOX_BR)
			has_متن_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	لكل (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->چوڑائی;
		إذا (!layout_سطر(c, &curwidth, &y, cx, cy + y, cont, پہلا_سطر,
				has_متن_children, content, &اگلا))
			أرجع false;
		maxwidth = max(maxwidth,curwidth);
		c = اگلا;
		پہلا_سطر = false;
	}

	inline_container->چوڑائی = maxwidth;
	inline_container->اچائی = y;

	أرجع true;
}


/* Documented in layout_intertnal.h */
bool layout_بلاک_context(
		بنية box *بلاک,
		صحيح viewport_اچائی,
		html_content *content)
{
	بنية box *box;
	صحيح cx, cy;  /**< current coordinates */
	صحيح max_pos_margin = 0;
	صحيح max_neg_margin = 0;
	صحيح y = 0;
	صحيح lm, rm;
	بنية box *margin_collapse = عدم;
	bool وچ_margin = false;
	css_fixed gadget_ناپ;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	أكد(بلاک->type == BOX_بلاک ||
			بلاک->type == BOX_INLINE_بلاک ||
			بلاک->type == BOX_جدول_CELL ||
			بلاک->type == BOX_FLEX ||
			بلاک->type == BOX_INLINE_FLEX);
	أكد(بلاک->چوڑائی != UNKNOWN_چوڑائی);
	أكد(بلاک->چوڑائی != AUTO);

	بلاک->float_children = عدم;
	بلاک->cached_رکھو_ہیٹھاں_level = 0;
	بلاک->clear_level = 0;

	/* special case if the block contains an object */
	إذا (بلاک->object) {
		صحيح عارضی_چوڑائی = بلاک->چوڑائی;
		إذا (!layout_بلاک_object(بلاک))
			أرجع false;
		layout_get_object_dimensions(بلاک, &عارضی_چوڑائی,
				&بلاک->اچائی, INT_MIN, ودھ_توں_ودھ_عدد,
				INT_MIN, ودھ_توں_ودھ_عدد);
		أرجع true;
	} وإلا إذا (بلاک->flags & REPLACE_DIM) {
		أرجع true;
	}

	/* special case if the block contains an radio button or checkbox */
	إذا (بلاک->gadget && (بلاک->gadget->type == GADGET_RADIO ||
			بلاک->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_ناپ = INTTOFIX(1);
		إذا (بلاک->اچائی == AUTO)
			بلاک->اچائی = FIXTOINT(css_unit_len2device_px(
					بلاک->style,
					&content->unit_len_ctx,
					gadget_ناپ, gadget_unit));
	}

	box = بلاک->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = بلاک->padding[چوٹی_2];
	إذا (box)
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
	طالما (box) {
		تعداد css_حد_توں_ودھ_e حد_توں_ودھ_x = CSS_حد_توں_ودھ_VISIBLE;
		تعداد css_حد_توں_ودھ_e حد_توں_ودھ_y = CSS_حد_توں_ودھ_VISIBLE;

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

		إذا (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->ماپا->padding[بایاں_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			اذهب advance_منزل_اگلا_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		إذا (margin_collapse == عدم) {
			margin_collapse = layout_اگلا_margin_بلاک(
					&content->unit_len_ctx, box, بلاک,
					viewport_اچائی,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			وچ_margin = true;
		}

		/* Clearance. */
		y = 0;
		إذا (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(بلاک->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		إذا (box->style) {
			حد_توں_ودھ_x = css_computed_حد_توں_ودھ_x(box->style);
			حد_توں_ودھ_y = css_computed_حد_توں_ودھ_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		إذا (box->type == BOX_FLEX ||
		    box->type == BOX_بلاک ||
		    box->flags & IFRAME) {
			إذا (lh__box_is_object(box) == false &&
					box->style &&
					(حد_توں_ودھ_x != CSS_حد_توں_ودھ_VISIBLE ||
					 حد_توں_ودھ_y != CSS_حد_توں_ودھ_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				صحيح x0, x1, چوٹی;
				بنية box *بایاں, *دایاں;
				چوٹی = cy + max_pos_margin - max_neg_margin;
				چوٹی = (چوٹی > y) ? چوٹی : y;
				x0 = cx;
				x1 = cx + box->ماپا->چوڑائی -
						box->ماپا->padding[بایاں_2] -
						box->ماپا->padding[دایاں_2];
				لبھو_sides(بلاک->float_children, چوٹی, چوٹی,
						&x0, &x1, &بایاں, &دایاں);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->ماپا->چوڑائی -
						box->ماپا->padding[بایاں_2] -
						box->ماپا->padding[دایاں_2] -
						x1;
			}
			layout_بلاک_لبھو_dimensions(&content->unit_len_ctx,
					box->ماپا->چوڑائی,
					viewport_اچائی, lm, rm, box);
			إذا (box->type == BOX_بلاک && !(box->flags & IFRAME)) {
				layout_بلاک_جوڑو_scrollbar(box, دایاں_2);
				layout_بلاک_جوڑو_scrollbar(box, BOTTOM);
			}
		} وإلا إذا (box->type == BOX_جدول) {
			إذا (box->style != عدم) {
				تعداد css_چوڑائی_e wtype;
				css_fixed چوڑائی = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_چوڑائی(box->style, &چوڑائی,
						&unit);

				إذا (wtype == CSS_چوڑائی_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					صحيح x0, x1, چوٹی;
					بنية box *بایاں, *دایاں;
					چوٹی = cy + max_pos_margin -
							max_neg_margin;
					چوٹی = (چوٹی > y) ? چوٹی : y;
					x0 = cx;
					x1 = cx + box->ماپا->چوڑائی -
						box->ماپا->padding[بایاں_2] -
						box->ماپا->padding[دایاں_2];
					لبھو_sides(بلاک->float_children,
						چوٹی, چوٹی, &x0, &x1,
						&بایاں, &دایاں);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->ماپا->چوڑائی -
						box->ماپا->padding[بایاں_2] -
						box->ماپا->padding[دایاں_2] -
						x1;
				}
			}
			إذا (!layout_جدول(box, box->ماپا->چوڑائی - lm - rm,
					content))
				أرجع false;
			layout_solve_چوڑائی(box, box->ماپا->چوڑائی, box->چوڑائی,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->ماپا->padding[بایاں_2] + box->margin[بایاں_2] +
				box->border[بایاں_2].چوڑائی;
		cx += box->x;

		/* Position box: vertical. */
		إذا (box->border[چوٹی_2].چوڑائی) {
			box->y += box->border[چوٹی_2].چوڑائی;
			cy += box->border[چوٹی_2].چوڑائی;
		}

		/* Vertical margin */
		إذا (((box->type == BOX_بلاک && (box->flags & HAS_اچائی)) ||
		     box->type == BOX_FLEX ||
		     box->type == BOX_جدول ||
		     (box->type == BOX_INLINE_CONTAINER &&
		      !box_is_پہلا_بچہ(box)) ||
		     margin_collapse == box) &&
		    وچ_margin == true) {
			/* Margin goes above this box. */
			cy += max_pos_margin - max_neg_margin;
			box->y += max_pos_margin - max_neg_margin;

			/* Current margin has been applied. */
			وچ_margin = false;
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
				(box->type == BOX_بلاک && box->style &&
				 (حد_توں_ودھ_x != CSS_حد_توں_ودھ_VISIBLE ||
				  حد_توں_ودھ_y != CSS_حد_توں_ودھ_VISIBLE))) {

			إذا (box->type == BOX_FLEX) {
				إذا (!layout_flex(box, box->چوڑائی, content)) {
					أرجع false;
				}
			} وإلا {
				layout_بلاک_context(box,
						viewport_اچائی, content);
			}

			cy += box->padding[چوٹی_2];

			إذا (box->اچائی == AUTO) {
				box->اچائی = 0;
				layout_بلاک_جوڑو_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->اچائی + box->padding[BOTTOM] +
					box->border[BOTTOM].چوڑائی;
			y = box->y + box->padding[چوٹی_2] + box->اچائی +
					box->padding[BOTTOM] +
					box->border[BOTTOM].چوڑائی;

			/* Skip children, because they are done in the new
			 * block context */
			اذهب advance_منزل_اگلا_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->چوڑائی);

		/* Layout (except tables). */
		إذا (box->object) {
			إذا (!layout_بلاک_object(box))
				أرجع false;

		} وإلا إذا (box->type == BOX_INLINE_CONTAINER) {
			box->چوڑائی = box->ماپا->چوڑائی;
			إذا (!layout_inline_container(box, box->چوڑائی, بلاک,
					cx, cy, content))
				أرجع false;

		} وإلا إذا (box->type == BOX_جدول) {
			/* Move down to avoid floats if necessary. */
			صحيح x0, x1;
			بنية box *بایاں, *دایاں;
			y = cy;
			طالما (1) {
				تعداد css_چوڑائی_e wtype;
				css_fixed چوڑائی = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_چوڑائی(box->style,
						&چوڑائی, &unit);

				x0 = cx;
				x1 = cx + box->ماپا->چوڑائی;
				لبھو_sides(بلاک->float_children, y,
						y + box->اچائی,
						&x0, &x1, &بایاں, &دایاں);
				إذا (wtype == CSS_چوڑائی_AUTO)
					اكسر;
				إذا (box->چوڑائی <= x1 - x0)
					اكسر;
				إذا (!بایاں && !دایاں)
					اكسر;
				وإلا إذا (!بایاں)
					y = دایاں->y + دایاں->اچائی + 1;
				وإلا إذا (!دایاں)
					y = بایاں->y + بایاں->اچائی + 1;
				وإلا إذا (بایاں->y + بایاں->اچائی <
						دایاں->y + دایاں->اچائی)
					y = بایاں->y + بایاں->اچائی + 1;
				وإلا
					y = دایاں->y + دایاں->اچائی + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		إذا (box->type == BOX_بلاک && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			إذا (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = عدم;
			}

			y = box->padding[چوٹی_2];
			box = box->children;
			box->y = y;
			cy += y;
			استمر;
		} وإلا إذا (box->type == BOX_بلاک || box->object ||
				box->flags & IFRAME)
			cy += box->padding[چوٹی_2];

		إذا (box->type == BOX_بلاک && box->اچائی == AUTO) {
			box->اچائی = 0;
			layout_بلاک_جوڑو_scrollbar(box, BOTTOM);
		}

		cy += box->اچائی + box->padding[BOTTOM] +
				box->border[BOTTOM].چوڑائی;
		cx -= box->x;
		y = box->y + box->padding[چوٹی_2] + box->اچائی +
				box->padding[BOTTOM] +
				box->border[BOTTOM].چوڑائی;

	advance_منزل_اگلا_box:
		إذا (!box->اگلا) {
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

				box = box->ماپا;
				إذا (box == بلاک)
					اكسر;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				إذا (box->type == BOX_بلاک &&
						box->flags & MAKE_اچائی) {
					margin_collapse = عدم;
					وچ_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				إذا (box->اچائی == AUTO) {
					box->اچائی = y - box->padding[چوٹی_2];

					إذا (box->type == BOX_بلاک)
						layout_بلاک_جوڑو_scrollbar(box,
								BOTTOM);
				} وإلا
					cy += box->اچائی -
							(y - box->padding[چوٹی_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				إذا (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_اچائی(
							&content->unit_len_ctx,
							box, عدم)) {
					/* Height altered */
					/* Set current cy */
					cy += box->اچائی -
							(y - box->padding[چوٹی_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].چوڑائی;
				cx -= box->x;
				y = box->y + box->padding[چوٹی_2] + box->اچائی +
						box->padding[BOTTOM] +
						box->border[BOTTOM].چوڑائی;

			} طالما (box->اگلا == عدم);
			إذا (box == بلاک)
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

		box = box->اگلا;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	لكل (box = بلاک->float_children; box; box = box->اگلا_float) {
		y = box->y + box->اچائی + box->padding[BOTTOM] +
				box->border[BOTTOM].چوڑائی + box->margin[BOTTOM];
		إذا (cy < y)
			cy = y;
	}

	إذا (بلاک->اچائی == AUTO) {
		بلاک->اچائی = cy - بلاک->padding[چوٹی_2];
		إذا (بلاک->type == BOX_بلاک)
			layout_بلاک_جوڑو_scrollbar(بلاک, BOTTOM);
	}

	إذا (بلاک->style && css_computed_position(بلاک->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_اچائی(&content->unit_len_ctx, بلاک, عدم);
	}

	إذا (بلاک->gadget &&
			(بلاک->gadget->type == GADGET_TEXTAREA ||
			بلاک->gadget->type == GADGET_PASSWORD ||
			بلاک->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		صحيح ta_چوڑائی = بلاک->padding[بایاں_2] + بلاک->چوڑائی +
				بلاک->padding[دایاں_2];
		صحيح ta_اچائی = بلاک->padding[چوٹی_2] + بلاک->اچائی +
				بلاک->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				بلاک->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_سیٹ_layout(بلاک->gadget->انکڑے.متن.ta,
				&fstyle, ta_چوڑائی, ta_اچائی,
				بلاک->padding[چوٹی_2], بلاک->padding[دایاں_2],
				بلاک->padding[BOTTOM], بلاک->padding[بایاں_2]);
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
layout_get_element_ٹیگ(
		ثابت dom_گرہ *گرہ,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_گرہ_type گرہ_type;
	dom_exception exc;

	exc = dom_گرہ_get_گرہ_type(گرہ, &گرہ_type);
	إذا (exc != DOM_NO_ERR ||
	    گرہ_type != DOM_ELEMENT_گرہ) {
		أرجع false;
	}

	exc = dom_html_element_get_ٹیگ_type(گرہ, &element_type);
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
		ثابت dom_گرہ *گرہ,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	إذا (!layout_get_element_ٹیگ(گرہ, &element_type)) {
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
layout_get_li_قدر(dom_گرہ *li_گرہ, dom_long *قدر_باہرلا)
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
	إذا (exc != DOM_NO_ERR || has_قدر == false) {
		أرجع false;
	}

	exc = dom_html_li_element_get_قدر(
			(dom_html_li_element *)li_گرہ,
			&قدر);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	*قدر_باہرلا = قدر;
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
layout_get_ol_شروع(dom_گرہ *ol_گرہ, dom_long *شروع_باہرلا)
{
	dom_exception exc;
	dom_long شروع;
	bool has_شروع;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_گرہ,
			corestring_dom_شروع,
			&has_شروع);
	إذا (exc != DOM_NO_ERR || has_شروع == false) {
		أرجع false;
	}

	exc = dom_html_olist_element_get_شروع(
			(dom_html_olist_element *)ol_گرہ,
			&شروع);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	*شروع_باہرلا = شروع;
	أرجع true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
ساكن bool
layout__get_ol_reversed(dom_گرہ *ol_گرہ)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_گرہ,
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
layout_get_list_شے_گنتی(
		dom_گرہ *list_owner, dom_long *گنتی_باہرلا)
{
	dom_html_element_type ٹیگ_type;
	dom_exception exc;
	dom_گرہ *بچہ;
	صحيح گنتی;

	إذا (list_owner == عدم) {
		أرجع false;
	}

	إذا (!layout_get_element_ٹیگ(list_owner, &ٹیگ_type)) {
		أرجع false;
	}

	إذا (ٹیگ_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    ٹیگ_type != DOM_HTML_ELEMENT_TYPE_UL) {
		أرجع false;
	}

	exc = dom_گرہ_get_پہلا_بچہ(list_owner, &بچہ);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	گنتی = 0;
	طالما (بچہ != عدم) {
		dom_گرہ *عارضی_گرہ;

		إذا (layout__check_element_type(بچہ,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			بنية box *بچہ_box;
			إذا (dom_گرہ_get_user_انکڑے(بچہ,
					corestring_dom_ns_کلید_box_گرہ_انکڑے,
					&بچہ_box) != DOM_NO_ERR) {
				dom_گرہ_unref(بچہ);
				أرجع false;
			}

			إذا (بچہ_box != عدم &&
			    بچہ_box->list_marker != عدم) {
				گنتی++;
			}
		}

		exc = dom_گرہ_get_اگلا_sibling(بچہ, &عارضی_گرہ);
		dom_گرہ_unref(بچہ);
		إذا (exc != DOM_NO_ERR) {
			أرجع false;
		}

		بچہ = عارضی_گرہ;
	}

	*گنتی_باہرلا = گنتی;
	أرجع true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
ساكن فراغ
layout_ordered_list_گنتی(
		بنية box *box)
{
	dom_html_element_type ٹیگ_type;
	dom_exception exc;
	dom_گرہ *بچہ;
	صحيح قدم = 1;
	dom_long اگلا;

	إذا (box->گرہ == عدم) {
		أرجع;
	}

	إذا (!layout_get_element_ٹیگ(box->گرہ, &ٹیگ_type)) {
		أرجع;
	}

	إذا (ٹیگ_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    ٹیگ_type != DOM_HTML_ELEMENT_TYPE_UL) {
		أرجع;
	}

	اگلا = 1;
	إذا (ٹیگ_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_شروع = layout_get_ol_شروع(box->گرہ, &اگلا);
		bool have_reversed = layout__get_ol_reversed(box->گرہ);

		إذا (have_reversed) {
			قدم = -1;
		}

		إذا (!have_شروع && have_reversed) {
			layout_get_list_شے_گنتی(box->گرہ, &اگلا);
		}
	}

	exc = dom_گرہ_get_پہلا_بچہ(box->گرہ, &بچہ);
	إذا (exc != DOM_NO_ERR) {
		أرجع;
	}

	طالما (بچہ != عدم) {
		dom_گرہ *عارضی_گرہ;

		إذا (layout__check_element_type(بچہ,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			بنية box *بچہ_box;

			إذا (dom_گرہ_get_user_انکڑے(بچہ,
					corestring_dom_ns_کلید_box_گرہ_انکڑے,
					&بچہ_box) != DOM_NO_ERR) {
				dom_گرہ_unref(بچہ);
				أرجع;
			}

			إذا (بچہ_box != عدم &&
			    بچہ_box->list_marker != عدم) {
				dom_long قدر;
				بنية box *marker = بچہ_box->list_marker;
				إذا (layout_get_li_قدر(بچہ, &قدر)) {
					marker->list_قدر = قدر;
					اگلا = marker->list_قدر;
				} وإلا {
					marker->list_قدر = اگلا;
				}
				اگلا += قدم;
			}
		}

		exc = dom_گرہ_get_اگلا_sibling(بچہ, &عارضی_گرہ);
		dom_گرہ_unref(بچہ);
		إذا (exc != DOM_NO_ERR) {
			أرجع;
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
ساكن فراغ
layout_سیٹ_numerical_marker_متن(
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
		LIST_MARKER_ناپ = 20,
	};

	marker->متن = talloc_array(content->bctx, حرف, LIST_MARKER_ناپ);
	إذا (marker->متن == عدم) {
		أرجع;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_قدر,
			marker->متن, LIST_MARKER_ناپ, &counter_len);
	إذا (css_res == CSS_OK) {
		إذا (counter_len > LIST_MARKER_ناپ) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->متن = talloc_realloc(content->bctx,
					marker->متن,
					حرف,
					counter_len);
			إذا (marker->متن == عدم) {
				أرجع;
			}
			css_computed_format_list_style(box->style,
					marker->list_قدر, marker->متن,
					counter_len, &counter_len);
		}
		marker->لمائی = counter_len;
	}
}

/**
 * Find out if box's style represents a numerical list style type.
 *
 * \param[in]  b  Box with style to test.
 * \return true if box has numerical list style type, false otherwise.
 */
ساكن bool
layout_list_شے_is_numerical(
		ثابت بنية box *b)
{
	تعداد css_list_style_type_e t = css_computed_list_style_type(b->style);

	بدل (t) {
	حالة CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	حالة CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	حالة CSS_LIST_STYLE_TYPE_مربع: /* Fall through. */
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
	بنية box *بچہ;

	layout_ordered_list_گنتی(box);

	لكل (بچہ = box->children; بچہ; بچہ = بچہ->اگلا) {
		إذا (بچہ->list_marker) {
			بنية box *marker = بچہ->list_marker;

			إذا (layout_list_شے_is_numerical(بچہ)) {
				إذا (marker->متن == عدم) {
					layout_سیٹ_numerical_marker_متن(
							content, بچہ);
				}
			}
			إذا (marker->object) {
				marker->چوڑائی =
					content_get_چوڑائی(marker->object);
				marker->x = -marker->چوڑائی;
				marker->اچائی =
					content_get_اچائی(marker->object);
				marker->y = (سطر_اچائی(
						&content->unit_len_ctx,
						marker->style) -
						marker->اچائی) / 2;
			} وإلا إذا (marker->متن) {
				إذا (marker->چوڑائی == UNKNOWN_چوڑائی) {
					plot_font_style_t fstyle;
					font_plot_style_from_css(
							&content->unit_len_ctx,
							marker->style,
							&fstyle);
					content->font_func->چوڑائی(&fstyle,
							marker->متن,
							marker->لمائی,
							&marker->چوڑائی);
					marker->flags |= MEASURED;
				}
				marker->x = -marker->چوڑائی;
				marker->y = 0;
				marker->اچائی = سطر_اچائی(
						&content->unit_len_ctx,
						marker->style);
			} وإلا {
				marker->x = 0;
				marker->y = 0;
				marker->چوڑائی = 0;
				marker->اچائی = 0;
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
ساكن فراغ
layout_compute_offsets(ثابت css_unit_ctx *unit_len_ctx,
		       بنية box *box,
		       بنية box *containing_بلاک,
		       صحيح *چوٹی,
		       صحيح *دایاں,
		       صحيح *bottom,
		       صحيح *بایاں)
{
	uint32_t type;
	css_fixed قدر = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(containing_بلاک->چوڑائی != UNKNOWN_چوڑائی);
	أكد(containing_بلاک->چوڑائی != AUTO);
	أكد(containing_بلاک->اچائی != AUTO);

	/*بایاں*/
	type = css_computed_بایاں(box->style, &قدر, &unit);
	إذا (type == CSS_بایاں_سیٹ) {
		إذا (unit == CSS_UNIT_PCT) {
			*بایاں = FPCT_دا_INT_TOINT(قدر,
					containing_بلاک->چوڑائی);
		} وإلا {
			*بایاں = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					قدر, unit));
		}
	} وإلا {
		*بایاں = AUTO;
	}

	/*دایاں*/
	type = css_computed_دایاں(box->style, &قدر, &unit);
	إذا (type == CSS_دایاں_سیٹ) {
		إذا (unit == CSS_UNIT_PCT) {
			*دایاں = FPCT_دا_INT_TOINT(قدر,
					containing_بلاک->چوڑائی);
		} وإلا {
			*دایاں = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					قدر, unit));
		}
	} وإلا {
		*دایاں = AUTO;
	}

	/*چوٹی*/
	type = css_computed_چوٹی(box->style, &قدر, &unit);
	إذا (type == CSS_چوٹی_سیٹ) {
		إذا (unit == CSS_UNIT_PCT) {
			*چوٹی = FPCT_دا_INT_TOINT(قدر,
					containing_بلاک->اچائی);
		} وإلا {
			*چوٹی = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					قدر, unit));
		}
	} وإلا {
		*چوٹی = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &قدر, &unit);
	إذا (type == CSS_BOTTOM_سیٹ) {
		إذا (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_دا_INT_TOINT(قدر,
					containing_بلاک->اچائی);
		} وإلا {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					قدر, unit));
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
		بنية box *containing_بلاک,
		صحيح cx, صحيح cy,
		html_content *content)
{
	صحيح static_بایاں, static_چوٹی;  /* static position */
	صحيح چوٹی, دایاں, bottom, بایاں;
	صحيح چوڑائی, اچائی, max_چوڑائی, min_چوڑائی;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	صحيح دستیاب_چوڑائی = containing_بلاک->چوڑائی;
	صحيح space;

	أكد(box->type == BOX_بلاک || box->type == BOX_جدول ||
			box->type == BOX_INLINE_بلاک ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_بایاں = cx + box->x;
	static_چوٹی = cy + box->y;

	إذا (containing_بلاک->type == BOX_بلاک ||
			containing_بلاک->type == BOX_INLINE_بلاک ||
			containing_بلاک->type == BOX_جدول_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_بلاک->چوڑائی += containing_بلاک->padding[بایاں_2] +
				containing_بلاک->padding[دایاں_2];
		containing_بلاک->اچائی += containing_بلاک->padding[چوٹی_2] +
				containing_بلاک->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_بلاک,
			&چوٹی, &دایاں, &bottom, &بایاں);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_بلاک;
	layout_لبھو_dimensions(&content->unit_len_ctx, دستیاب_چوڑائی, -1,
			box, box->style, &چوڑائی, &اچائی,
			&max_چوڑائی, &min_چوڑائی, 0, 0,
			margin, padding, border);
	box->float_container = عدم;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      بایاں, margin[بایاں_2], border[بایاں_2].چوڑائی, padding[بایاں_2], چوڑائی,
	      padding[دایاں_2], border[دایاں_2].چوڑائی, margin[دایاں_2], دایاں,
	      containing_بلاک->چوڑائی);


	إذا (بایاں == AUTO && چوڑائی == AUTO && دایاں == AUTO) {
		إذا (margin[بایاں_2] == AUTO)
			margin[بایاں_2] = 0;
		إذا (margin[دایاں_2] == AUTO)
			margin[دایاں_2] = 0;
		بایاں = static_بایاں;

		چوڑائی = min(max(box->min_چوڑائی, دستیاب_چوڑائی),
			box->max_چوڑائی);
		چوڑائی -= box->margin[بایاں_2] + box->border[بایاں_2].چوڑائی +
			box->padding[بایاں_2] + box->padding[دایاں_2] +
			box->border[دایاں_2].چوڑائی + box->margin[دایاں_2];

		/* Adjust for {min|max}-width */
		إذا (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی) چوڑائی = max_چوڑائی;
		إذا (چوڑائی < min_چوڑائی) چوڑائی = min_چوڑائی;

		دایاں = containing_بلاک->چوڑائی -
			بایاں -
			margin[بایاں_2] - border[بایاں_2].چوڑائی - padding[بایاں_2] -
			چوڑائی -
			padding[دایاں_2] - border[دایاں_2].چوڑائی - margin[دایاں_2];
	} وإلا إذا (بایاں != AUTO && چوڑائی != AUTO && دایاں != AUTO) {

		/* Adjust for {min|max}-width */
		إذا (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی) چوڑائی = max_چوڑائی;
		إذا (min_چوڑائی >  0 && چوڑائی < min_چوڑائی) چوڑائی = min_چوڑائی;

		إذا (margin[بایاں_2] == AUTO && margin[دایاں_2] == AUTO) {
			space = containing_بلاک->چوڑائی -
					بایاں - border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - دایاں;
			إذا (space < 0) {
				margin[بایاں_2] = 0;
				margin[دایاں_2] = space;
			} وإلا {
				margin[بایاں_2] = margin[دایاں_2] = space / 2;
			}
		} وإلا إذا (margin[بایاں_2] == AUTO) {
			margin[بایاں_2] = containing_بلاک->چوڑائی -
					بایاں - border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2] -
					دایاں;
		} وإلا إذا (margin[دایاں_2] == AUTO) {
			margin[دایاں_2] = containing_بلاک->چوڑائی -
					بایاں - margin[بایاں_2] -
					border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - دایاں;
		} وإلا {
			دایاں = containing_بلاک->چوڑائی -
					بایاں - margin[بایاں_2] -
					border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2];
		}
	} وإلا {
		إذا (margin[بایاں_2] == AUTO)
			margin[بایاں_2] = 0;
		إذا (margin[دایاں_2] == AUTO)
			margin[دایاں_2] = 0;

		إذا (بایاں == AUTO && چوڑائی == AUTO && دایاں != AUTO) {
			دستیاب_چوڑائی -= دایاں;

			چوڑائی = min(max(box->min_چوڑائی, دستیاب_چوڑائی),
				box->max_چوڑائی);
			چوڑائی -= box->margin[بایاں_2] + box->border[بایاں_2].چوڑائی +
				box->padding[بایاں_2] + box->padding[دایاں_2] +
				box->border[دایاں_2].چوڑائی + box->margin[دایاں_2];

			/* Adjust for {min|max}-width */
			إذا (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی)
				چوڑائی = max_چوڑائی;
			إذا (چوڑائی < min_چوڑائی)
				چوڑائی = min_چوڑائی;

			بایاں = containing_بلاک->چوڑائی -
					margin[بایاں_2] - border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2] -
					دایاں;
		} وإلا إذا (بایاں == AUTO && چوڑائی != AUTO && دایاں == AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی)
				چوڑائی = max_چوڑائی;
			إذا (min_چوڑائی >  0 && چوڑائی < min_چوڑائی)
				چوڑائی = min_چوڑائی;

			بایاں = static_بایاں;
			دایاں = containing_بلاک->چوڑائی -
					بایاں - margin[بایاں_2] -
					border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2];
		} وإلا إذا (بایاں != AUTO && چوڑائی == AUTO && دایاں == AUTO) {
			دستیاب_چوڑائی -= بایاں;

			چوڑائی = min(max(box->min_چوڑائی, دستیاب_چوڑائی),
				box->max_چوڑائی);
			چوڑائی -= box->margin[بایاں_2] + box->border[بایاں_2].چوڑائی +
				box->padding[بایاں_2] + box->padding[دایاں_2] +
				box->border[دایاں_2].چوڑائی + box->margin[دایاں_2];

			/* Adjust for {min|max}-width */
			إذا (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی)
				چوڑائی = max_چوڑائی;
			إذا (چوڑائی < min_چوڑائی)
				چوڑائی = min_چوڑائی;

			دایاں = containing_بلاک->چوڑائی -
					بایاں - margin[بایاں_2] -
					border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2];
		} وإلا إذا (بایاں == AUTO && چوڑائی != AUTO && دایاں != AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی)
				چوڑائی = max_چوڑائی;
			إذا (چوڑائی < min_چوڑائی)
				چوڑائی = min_چوڑائی;

			بایاں = containing_بلاک->چوڑائی -
					margin[بایاں_2] - border[بایاں_2].چوڑائی -
					padding[بایاں_2] - چوڑائی - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2] -
					دایاں;
		} وإلا إذا (بایاں != AUTO && چوڑائی == AUTO && دایاں != AUTO) {
			چوڑائی = containing_بلاک->چوڑائی -
					بایاں - margin[بایاں_2] -
					border[بایاں_2].چوڑائی -
					padding[بایاں_2] - padding[دایاں_2] -
					border[دایاں_2].چوڑائی - margin[دایاں_2] -
					دایاں;

			/* Adjust for {min|max}-width */
			إذا (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی)
				چوڑائی = max_چوڑائی;
			إذا (چوڑائی < min_چوڑائی)
				چوڑائی = min_چوڑائی;

		} وإلا إذا (بایاں != AUTO && چوڑائی != AUTO && دایاں == AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_چوڑائی >= 0 && چوڑائی > max_چوڑائی)
				چوڑائی = max_چوڑائی;
			إذا (چوڑائی < min_چوڑائی)
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
	إذا (containing_بلاک->type == BOX_بلاک ||
			containing_بلاک->type == BOX_INLINE_بلاک ||
			containing_بلاک->type == BOX_جدول_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_بلاک->چوڑائی -= containing_بلاک->padding[بایاں_2] +
				containing_بلاک->padding[دایاں_2];
	} وإلا {
		/** \todo inline ancestors */
	}
	box->چوڑائی = چوڑائی;
	box->اچائی = اچائی;

	إذا (box->type == BOX_بلاک || box->type == BOX_INLINE_بلاک ||
			box->object || box->flags & IFRAME) {
		إذا (!layout_بلاک_context(box, -1, content))
			أرجع false;
	} وإلا إذا (box->type == BOX_جدول) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_بلاک;
		/* \todo  layout_table considers margins etc. again */
		إذا (!layout_جدول(box, چوڑائی, content))
			أرجع false;
		box->float_container = عدم;
		layout_solve_چوڑائی(box, box->ماپا->چوڑائی, box->چوڑائی, 0, 0,
				-1, -1);
	} وإلا إذا (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_بلاک;
		إذا (!layout_flex(box, چوڑائی, content))
			أرجع false;
		box->float_container = عدم;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      چوٹی, margin[چوٹی_2], border[چوٹی_2].چوڑائی, padding[چوٹی_2], اچائی,
	      padding[BOTTOM], border[BOTTOM].چوڑائی, margin[BOTTOM], bottom,
	      containing_بلاک->اچائی);

	إذا (چوٹی == AUTO && اچائی == AUTO && bottom == AUTO) {
		چوٹی = static_چوٹی;
		اچائی = box->اچائی;
		إذا (margin[چوٹی_2] == AUTO)
			margin[چوٹی_2] = 0;
		إذا (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_بلاک->اچائی -
				چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
				padding[چوٹی_2] - اچائی - padding[BOTTOM] -
				border[BOTTOM].چوڑائی - margin[BOTTOM];
	} وإلا إذا (چوٹی != AUTO && اچائی != AUTO && bottom != AUTO) {
		إذا (margin[چوٹی_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_بلاک->اچائی -
					چوٹی - border[چوٹی_2].چوڑائی - padding[چوٹی_2] -
					اچائی - padding[BOTTOM] -
					border[BOTTOM].چوڑائی - bottom;
			margin[چوٹی_2] = margin[BOTTOM] = space / 2;
		} وإلا إذا (margin[چوٹی_2] == AUTO) {
			margin[چوٹی_2] = containing_بلاک->اچائی -
					چوٹی - border[چوٹی_2].چوڑائی - padding[چوٹی_2] -
					اچائی - padding[BOTTOM] -
					border[BOTTOM].چوڑائی - margin[BOTTOM] -
					bottom;
		} وإلا إذا (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_بلاک->اچائی -
					چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					bottom;
		} وإلا {
			bottom = containing_بلاک->اچائی -
					چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					margin[BOTTOM];
		}
	} وإلا {
		إذا (margin[چوٹی_2] == AUTO)
			margin[چوٹی_2] = 0;
		إذا (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		إذا (چوٹی == AUTO && اچائی == AUTO && bottom != AUTO) {
			اچائی = box->اچائی;
			چوٹی = containing_بلاک->اچائی -
					margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					margin[BOTTOM] - bottom;
		} وإلا إذا (چوٹی == AUTO && اچائی != AUTO && bottom == AUTO) {
			چوٹی = static_چوٹی;
			bottom = containing_بلاک->اچائی -
					چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					margin[BOTTOM];
		} وإلا إذا (چوٹی != AUTO && اچائی == AUTO && bottom == AUTO) {
			اچائی = box->اچائی;
			bottom = containing_بلاک->اچائی -
					چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					margin[BOTTOM];
		} وإلا إذا (چوٹی == AUTO && اچائی != AUTO && bottom != AUTO) {
			چوٹی = containing_بلاک->اچائی -
					margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					margin[BOTTOM] - bottom;
		} وإلا إذا (چوٹی != AUTO && اچائی == AUTO && bottom != AUTO) {
			اچائی = containing_بلاک->اچائی -
					چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - padding[BOTTOM] -
					border[BOTTOM].چوڑائی - margin[BOTTOM] -
					bottom;
		} وإلا إذا (چوٹی != AUTO && اچائی != AUTO && bottom == AUTO) {
			bottom = containing_بلاک->اچائی -
					چوٹی - margin[چوٹی_2] - border[چوٹی_2].چوڑائی -
					padding[چوٹی_2] - اچائی -
					padding[BOTTOM] - border[BOTTOM].چوڑائی -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      چوٹی, margin[چوٹی_2], border[چوٹی_2].چوڑائی, padding[چوٹی_2], اچائی,
	      padding[BOTTOM], border[BOTTOM].چوڑائی, margin[BOTTOM], bottom,
	      containing_بلاک->اچائی);

	box->y = چوٹی + margin[چوٹی_2] + border[چوٹی_2].چوڑائی - cy;
	إذا (containing_بلاک->type == BOX_بلاک ||
			containing_بلاک->type == BOX_INLINE_بلاک ||
			containing_بلاک->type == BOX_جدول_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_بلاک->اچائی -= containing_بلاک->padding[چوٹی_2] +
				containing_بلاک->padding[BOTTOM];
	} وإلا {
		/** \todo Inline ancestors */
	}
	box->اچائی = اچائی;
	layout_apply_minmax_اچائی(&content->unit_len_ctx, box, containing_بلاک);

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
			 بنية box *containing_بلاک,
			 صحيح cx, صحيح cy,
			 html_content *content)
{
	بنية box *c;

	لكل (c = box->children; c; c = c->اگلا) {
		إذا ((c->type == BOX_بلاک || c->type == BOX_جدول ||
				c->type == BOX_INLINE_بلاک ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			إذا (!layout_absolute(c, containing_بلاک,
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
						CSS_FLOAT_بایاں ||
					css_computed_float(c->style) ==
						CSS_FLOAT_دایاں)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				بنية box *p;
				px = c->x;
				py = c->y;
				لكل (p = box->ماپا; p && !p->float_children;
						p = p->ماپا) {
					px -= p->x;
					py -= p->y;
				}
			} وإلا {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			إذا (!layout_position_absolute(c, containing_بلاک,
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
ساكن فراغ layout_compute_relative_وتھ(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *box,
		صحيح *x,
		صحيح *y)
{
	صحيح بایاں, دایاں, چوٹی, bottom;
	بنية box *containing_بلاک;

	أكد(box && box->ماپا && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	إذا (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_بایاں ||
			css_computed_float(box->style) == CSS_FLOAT_دایاں)) {
		containing_بلاک = box->float_container;
	} وإلا {
		containing_بلاک = box->ماپا;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_بلاک,
			&چوٹی, &دایاں, &bottom, &بایاں);

	إذا (بایاں == AUTO && دایاں == AUTO)
		بایاں = دایاں = 0;
	وإلا إذا (بایاں == AUTO)
		/* left is auto => computed = -right */
		بایاں = -دایاں;
	وإلا إذا (دایاں == AUTO)
		/* right is auto => computed = -left */
		دایاں = -بایاں;
	وإلا {
		/* over constrained => examine direction property
		 * of containing block */
		إذا (containing_بلاک->style &&
				css_computed_direction(
				containing_بلاک->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			بایاں = -دایاں;
		} وإلا {
			/* assume LTR in all other cases */
			دایاں = -بایاں;
		}
	}

	أكد(بایاں == -دایاں);

	إذا (چوٹی == AUTO && bottom == AUTO) {
		چوٹی = bottom = 0;
	} وإلا إذا (چوٹی == AUTO) {
		چوٹی = -bottom;
	} وإلا {
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
ساكن فراغ
layout_position_relative(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *جڑ,
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

	أكد(جڑ);

	/* Normal children */
	لكل (box = جڑ->children; box; box = box->اگلا) {

		إذا (box->type == BOX_متن)
			استمر;

		/* If relatively positioned, get offsets */
		إذا (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_وتھ(
					unit_len_ctx, box, &x, &y);
		وإلا
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		إذا (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_بایاں ||
				css_computed_float(box->style) ==
					CSS_FLOAT_دایاں) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			لكل (fc = fp->float_children; fc; fc = fc->اگلا_float) {
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
		إذا (box->type == BOX_INLINE && box->inline_سرا) {
			بنية box *b;
			لكل (b = box->اگلا; b && b != box->inline_سرا;
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
ساكن فراغ
layout_get_box_bbox(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *box,
		صحيح *desc_x0, صحيح *desc_y0,
		صحيح *desc_x1, صحيح *desc_y1)
{
	*desc_x0 = -box->border[بایاں_2].چوڑائی;
	*desc_y0 = -box->border[چوٹی_2].چوڑائی;
	*desc_x1 = box->padding[بایاں_2] + box->چوڑائی + box->padding[دایاں_2] +
			box->border[دایاں_2].چوڑائی;
	*desc_y1 = box->padding[چوٹی_2] + box->اچائی + box->padding[BOTTOM] +
			box->border[BOTTOM].چوڑائی;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	إذا (box->type == BOX_بلاک && box->style != عدم &&
			css_computed_حد_توں_ودھ_y(box->style) ==
					CSS_حد_توں_ودھ_VISIBLE &&
			box->object == عدم) {
		css_fixed font_ناپ = 0;
		css_unit font_unit = CSS_UNIT_PT;
		صحيح متن_اچائی;

		css_computed_font_ناپ(box->style, &font_ناپ, &font_unit);
		متن_اچائی = css_unit_len2device_px(box->style, unit_len_ctx,
				font_ناپ, font_unit);
		متن_اچائی = FIXTOINT(متن_اچائی * 3 / 4);
		*desc_y0 = (*desc_y0 < -متن_اچائی) ? *desc_y0 : -متن_اچائی;
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
		بنية box *بچہ,
		صحيح off_x,
		صحيح off_y)
{
	صحيح بچہ_desc_x0, بچہ_desc_y0, بچہ_desc_x1, بچہ_desc_y1;

	/* get coordinates of child relative to box */
	صحيح بچہ_x = بچہ->x - off_x;
	صحيح بچہ_y = بچہ->y - off_y;

	bool html_object = (بچہ->object &&
			content_get_type(بچہ->object) == CONTENT_HTML);

	تعداد css_حد_توں_ودھ_e حد_توں_ودھ_x = CSS_حد_توں_ودھ_VISIBLE;
	تعداد css_حد_توں_ودھ_e حد_توں_ودھ_y = CSS_حد_توں_ودھ_VISIBLE;

	إذا (بچہ->style != عدم) {
		حد_توں_ودھ_x = css_computed_حد_توں_ودھ_x(بچہ->style);
		حد_توں_ودھ_y = css_computed_حد_توں_ودھ_y(بچہ->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, بچہ,
			&بچہ_desc_x0, &بچہ_desc_y0,
			&بچہ_desc_x1, &بچہ_desc_y1);

	إذا (حد_توں_ودھ_x == CSS_حد_توں_ودھ_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		بچہ_desc_x0 = بچہ->descendant_x0;
		بچہ_desc_x1 = بچہ->descendant_x1;
	}
	إذا (حد_توں_ودھ_y == CSS_حد_توں_ودھ_VISIBLE &&
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
	إذا (بچہ_desc_x0 < box->descendant_x0)
		box->descendant_x0 = بچہ_desc_x0;
	إذا (بچہ_desc_y0 < box->descendant_y0)
		box->descendant_y0 = بچہ_desc_y0;
	إذا (box->descendant_x1 < بچہ_desc_x1)
		box->descendant_x1 = بچہ_desc_x1;
	إذا (box->descendant_y1 < بچہ_desc_y1)
		box->descendant_y1 = بچہ_desc_y1;
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
	بنية box *بچہ;

	أكد(box->چوڑائی != UNKNOWN_چوڑائی);
	أكد(box->اچائی != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	إذا (box->object && content_get_type(box->object) == CONTENT_HTML) {
		إذا (box->descendant_x1 < content_get_چوڑائی(box->object))
			box->descendant_x1 = content_get_چوڑائی(box->object);
		إذا (box->descendant_y1 < content_get_اچائی(box->object))
			box->descendant_y1 = content_get_اچائی(box->object);
	}

	إذا (box->iframe != عدم) {
		صحيح x, y;
		box_coords(box, &x, &y);

		browser_کھڑکی_سیٹ_position(box->iframe, x, y);
		browser_کھڑکی_سیٹ_dimensions(box->iframe,
				box->چوڑائی, box->اچائی);
		browser_کھڑکی_reformat(box->iframe, true,
				box->چوڑائی, box->اچائی);
	}

	إذا (box->type == BOX_INLINE || box->type == BOX_متن)
		أرجع;

	إذا (box->type == BOX_INLINE_سرا) {
		box = box->inline_سرا;
		لكل (بچہ = box->اگلا; بچہ;
				بچہ = بچہ->اگلا) {
			إذا (بچہ->type == BOX_FLOAT_بایاں ||
					بچہ->type == BOX_FLOAT_دایاں)
				استمر;

			layout_update_descendant_bbox(unit_len_ctx, box, بچہ,
					box->x, box->y);

			إذا (بچہ == box->inline_سرا)
				اكسر;
		}
		أرجع;
	}

	إذا (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		أرجع;

	لكل (بچہ = box->children; بچہ; بچہ = بچہ->اگلا) {
		إذا (بچہ->type == BOX_FLOAT_بایاں ||
				بچہ->type == BOX_FLOAT_دایاں)
			استمر;

		layout_calculate_descendant_bboxes(unit_len_ctx, بچہ);

		إذا (box->style && css_computed_حد_توں_ودھ_x(box->style) ==
				CSS_حد_توں_ودھ_HIDDEN &&
				css_computed_حد_توں_ودھ_y(box->style) ==
				CSS_حد_توں_ودھ_HIDDEN)
			استمر;

		layout_update_descendant_bbox(unit_len_ctx, box, بچہ, 0, 0);
	}

	لكل (بچہ = box->float_children; بچہ; بچہ = بچہ->اگلا_float) {
		أكد(بچہ->type == BOX_FLOAT_بایاں ||
				بچہ->type == BOX_FLOAT_دایاں);

		layout_calculate_descendant_bboxes(unit_len_ctx, بچہ);

		layout_update_descendant_bbox(unit_len_ctx, box, بچہ, 0, 0);
	}

	إذا (box->list_marker) {
		بچہ = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, بچہ);

		layout_update_descendant_bbox(unit_len_ctx, box, بچہ, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, صحيح چوڑائی, صحيح اچائی)
{
	bool ret;
	بنية box *doc = content->layout;
	ثابت بنية gui_layout_جدول *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			چوڑائی, اچائی, nsurl_access(content_get_url(
					&content->ادھار)));

	layout_minmax_بلاک(doc, font_func, content);

	layout_بلاک_لبھو_dimensions(&content->unit_len_ctx,
			چوڑائی, اچائی, 0, 0, doc);
	doc->x = doc->margin[بایاں_2] + doc->border[بایاں_2].چوڑائی;
	doc->y = doc->margin[چوٹی_2] + doc->border[چوٹی_2].چوڑائی;
	چوڑائی -= doc->margin[بایاں_2] + doc->border[بایاں_2].چوڑائی +
			doc->padding[بایاں_2] + doc->padding[دایاں_2] +
			doc->border[دایاں_2].چوڑائی + doc->margin[دایاں_2];
	إذا (چوڑائی < 0) {
		چوڑائی = 0;
	}
	doc->چوڑائی = چوڑائی;

	ret = layout_بلاک_context(doc, اچائی, content);

	/* make <html> and <body> fill available height */
	إذا (doc->y + doc->padding[چوٹی_2] + doc->اچائی + doc->padding[BOTTOM] +
			doc->border[BOTTOM].چوڑائی + doc->margin[BOTTOM] <
			اچائی) {
		doc->اچائی = اچائی - (doc->y + doc->padding[چوٹی_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].چوڑائی +
				doc->margin[BOTTOM]);
		إذا (doc->children)
			doc->children->اچائی = doc->اچائی -
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

	أرجع ret;
}
