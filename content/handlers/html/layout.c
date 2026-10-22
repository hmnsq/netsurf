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
	[سر_2]    = css_computed_margin_سر,
	[ښی_2]  = css_computed_margin_ښی,
	[BOTTOM] = css_computed_margin_bottom,
	[کیڼ_2]   = css_computed_margin_کیڼ,
};

/** Array of per-side access functions for computed style paddings. */
ثابت css_len_func padding_funcs[4] = {
	[سر_2]    = css_computed_padding_سر,
	[ښی_2]  = css_computed_padding_ښی,
	[BOTTOM] = css_computed_padding_bottom,
	[کیڼ_2]   = css_computed_padding_کیڼ,
};

/** Array of per-side access functions for computed style border_widths. */
ثابت css_len_func border_پلنوالی_funcs[4] = {
	[سر_2]    = css_computed_border_سر_پلنوالی,
	[ښی_2]  = css_computed_border_ښی_پلنوالی,
	[BOTTOM] = css_computed_border_bottom_پلنوالی,
	[کیڼ_2]   = css_computed_border_کیڼ_پلنوالی,
};

/** Array of per-side access functions for computed style border styles. */
ثابت css_border_style_func border_style_funcs[4] = {
	[سر_2]    = css_computed_border_سر_style,
	[ښی_2]  = css_computed_border_ښی_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[کیڼ_2]   = css_computed_border_کیڼ_style,
};

/** Array of per-side access functions for computed style border colors. */
ثابت css_border_color_func border_color_funcs[4] = {
	[سر_2]    = css_computed_border_سر_color,
	[ښی_2]  = css_computed_border_ښی_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[کیڼ_2]   = css_computed_border_کیڼ_color,
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
			     صحيح *پلنوالی, صحيح *لوړوالی,
			     صحيح min_پلنوالی, صحيح max_پلنوالی,
			     صحيح min_لوړوالی, صحيح max_لوړوالی)
{
	أكد(box->object != عدم);
	أكد(پلنوالی != عدم && لوړوالی != عدم);

	إذا (*پلنوالی == AUTO && *لوړوالی == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		صحيح intrinsic_پلنوالی = content_get_پلنوالی(box->object);
		صحيح intrinsic_لوړوالی = content_get_لوړوالی(box->object);

		/* use intrinsic dimensions */
		*پلنوالی = intrinsic_پلنوالی;
		*لوړوالی = intrinsic_لوړوالی;

		/* Deal with min/max-width first */
		إذا (min_پلنوالی >  0 && min_پلنوالی > *پلنوالی) {
			*پلنوالی = min_پلنوالی;
			scaled = true;
		}
		إذا (max_پلنوالی >= 0 && max_پلنوالی < *پلنوالی) {
			*پلنوالی = max_پلنوالی;
			scaled = true;
		}

		إذا (scaled && (intrinsic_پلنوالی != 0)) {
			/* Update height */
			*لوړوالی = (*پلنوالی * intrinsic_لوړوالی) /
					intrinsic_پلنوالی;
		}

		scaled = false;
		/* Deal with min/max-height */
		إذا (min_لوړوالی >  0 && min_لوړوالی > *لوړوالی) {
			*لوړوالی = min_لوړوالی;
			scaled = true;
		}
		إذا (max_لوړوالی >= 0 && max_لوړوالی < *لوړوالی) {
			*لوړوالی = max_لوړوالی;
			scaled = true;
		}

		إذا (scaled && (intrinsic_لوړوالی != 0)) {
			/* Update width */
			*پلنوالی = (*لوړوالی * intrinsic_پلنوالی) /
					intrinsic_لوړوالی;
		}

	} وإلا إذا (*پلنوالی == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		صحيح intrinsic_پلنوالی = content_get_پلنوالی(box->object);
		صحيح intrinsic_لوړوالی = content_get_لوړوالی(box->object);

		إذا (intrinsic_لوړوالی != 0)
			*پلنوالی = (*لوړوالی * intrinsic_پلنوالی) /
					intrinsic_لوړوالی;
		وإلا
			*پلنوالی = intrinsic_پلنوالی;

		إذا (min_پلنوالی >  0 && min_پلنوالی > *پلنوالی)
			*پلنوالی = min_پلنوالی;
		إذا (max_پلنوالی >= 0 && max_پلنوالی < *پلنوالی)
			*پلنوالی = max_پلنوالی;

	} وإلا إذا (*لوړوالی == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		صحيح intrinsic_پلنوالی = content_get_پلنوالی(box->object);
		صحيح intrinsic_لوړوالی = content_get_لوړوالی(box->object);

		إذا (min_پلنوالی >  0 && min_پلنوالی > *پلنوالی)
			*پلنوالی = min_پلنوالی;
		إذا (max_پلنوالی >= 0 && max_پلنوالی < *پلنوالی)
			*پلنوالی = max_پلنوالی;

		إذا (intrinsic_پلنوالی != 0)
			*لوړوالی = (*پلنوالی * intrinsic_لوړوالی) /
					intrinsic_پلنوالی;
		وإلا
			*لوړوالی = intrinsic_لوړوالی;
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
		ثابت css_computed_style *style, صحيح پلنوالی)
{
	css_fixed ارزښت = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_متن_indent(style, &ارزښت, &unit);

	إذا (unit == CSS_UNIT_PCT) {
		أرجع FPCT_د_INT_TOINT(ارزښت, پلنوالی);
	} وإلا {
		أرجع FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				ارزښت, unit));
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
	بینښه صحيح i, j;
	صحيح پلنوالی;
	صحيح border_spacing_h = 0;
	صحيح جدول_min = 0, جدول_max = 0;
	صحيح extra_fixed = 0;
	عائم extra_frac = 0;
	بنية ستنه *ستون;
	بنية box *لیکه_ډله, *لیکه, *cell;

	/* check if the widths have already been calculated */
	إذا (جدول->max_پلنوالی != UNKNOWN_MAX_پلنوالی)
		أرجع;

	إذا (جدول_calculate_ستنه_types(&content->unit_len_ctx, جدول) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		أرجع;
	}
	ستون = جدول->ستون;

	/* start with 0 except for fixed-width columns */
	لكل (i = 0; i != جدول->columns; i++) {
		إذا (ستون[i].type == ستنه_پلنوالی_FIXED)
			ستون[i].min = ستون[i].max = ستون[i].پلنوالی;
		وإلا
			ستون[i].min = ستون[i].max = 0;
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
	لكل (لیکه_ډله = جدول->children; لیکه_ډله; لیکه_ډله =لیکه_ډله->بل)
	لكل (لیکه = لیکه_ډله->children; لیکه; لیکه = لیکه->بل)
	لكل (cell = لیکه->children; cell; cell = cell->بل) {
		أكد(cell->type == BOX_جدول_CELL);
		أكد(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		أكد(cell->columns != 0);

		إذا (cell->columns != 1)
			استمر;

		layout_minmax_بلاک(cell, font_func, content);
		i = cell->پیل_ستنه;

		إذا (ستون[i].positioned)
			استمر;

		/* update column min, max widths using cell widths */
		إذا (ستون[i].min < cell->min_پلنوالی)
			ستون[i].min = cell->min_پلنوالی;
		إذا (ستون[i].max < cell->max_پلنوالی)
			ستون[i].max = cell->max_پلنوالی;
	}

	/* 2nd pass: cells which span multiple columns */
	لكل (لیکه_ډله = جدول->children; لیکه_ډله; لیکه_ډله =لیکه_ډله->بل)
	لكل (لیکه = لیکه_ډله->children; لیکه; لیکه = لیکه->بل)
	لكل (cell = لیکه->children; cell; cell = cell->بل) {
		بینښه صحيح flexible_columns = 0;
		صحيح min = 0, max = 0, fixed_پلنوالی = 0, extra;

		إذا (cell->columns == 1)
			استمر;

		layout_minmax_بلاک(cell, font_func, content);
		i = cell->پیل_ستنه;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		لكل (j = 0; j != cell->columns; j++) {
			min += ستون[i + j].min;
			إذا (ستون[i + j].type == ستنه_پلنوالی_FIXED)
				fixed_پلنوالی += ستون[i + j].پلنوالی;
			وإلا
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		إذا (min < cell->min_پلنوالی) {
			إذا (flexible_columns == 0) {
				extra = 1 + (cell->min_پلنوالی - min) /
						cell->columns;
				لكل (j = 0; j != cell->columns; j++) {
					ستون[i + j].min += extra;
					إذا (ستون[i + j].max < ستون[i + j].min)
						ستون[i + j].max = ستون[i + j].min;
				}
			} وإلا {
				extra = 1 + (cell->min_پلنوالی - min) /
						flexible_columns;
				لكل (j = 0; j != cell->columns; j++) {
					إذا (ستون[i + j].type !=
							ستنه_پلنوالی_FIXED) {
						ستون[i + j].min += extra;
						إذا (ستون[i + j].max <
								ستون[i + j].min)
							ستون[i + j].max =
								ستون[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		لكل (j = 0; j != cell->columns; j++)
			max += ستون[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		إذا (max < cell->max_پلنوالی && flexible_columns) {
			extra = 1 + (cell->max_پلنوالی - max) / flexible_columns;
			لكل (j = 0; j != cell->columns; j++)
				إذا (ستون[i + j].type != ستنه_پلنوالی_FIXED)
					ستون[i + j].max += extra;
		}
	}

	لكل (i = 0; i != جدول->columns; i++) {
		إذا (ستون[i].max < ستون[i].min) {
			box_dump(stderr, جدول, 0, true);
			أكد(0);
		}
		جدول_min += ستون[i].min;
		جدول_max += ستون[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	إذا (css_computed_پلنوالی_px(جدول->style, &content->unit_len_ctx,
			-1, &پلنوالی) == CSS_پلنوالی_سېټ) {
		إذا (جدول_min < پلنوالی)
			جدول_min = پلنوالی;
		إذا (جدول_max < پلنوالی)
			جدول_max = پلنوالی;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_پلنوالی(&content->unit_len_ctx,
			جدول->style, کیڼ_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_پلنوالی(&content->unit_len_ctx,
			جدول->style, ښی_2, true, true, true,
			&extra_fixed, &extra_frac);
	إذا (extra_fixed < 0)
		extra_fixed = 0;
	إذا (extra_frac < 0)
		extra_frac = 0;
	إذا (1.0 <= extra_frac)
		extra_frac = 0.9;
	جدول->min_پلنوالی = (جدول_min + extra_fixed) / (1.0 - extra_frac);
	جدول->max_پلنوالی = (جدول_max + extra_fixed) / (1.0 - extra_frac);
	جدول->min_پلنوالی += (جدول->columns + 1) * border_spacing_h;
	جدول->max_پلنوالی += (جدول->columns + 1) * border_spacing_h;

	أكد(0 <= جدول->min_پلنوالی && جدول->min_پلنوالی <= جدول->max_پلنوالی);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
ساكن مضمن bool box_has_percentage_max_پلنوالی(بنية box *b)
{
	css_unit unit = CSS_UNIT_PX;
	تعداد css_max_پلنوالی_e type;
	css_fixed ارزښت = 0;

	أكد(b != عدم);

	type = css_computed_max_پلنوالی(b->style, &ارزښت, &unit);
	أرجع ((type == CSS_MAX_پلنوالی_سېټ) && (unit == CSS_UNIT_PCT));
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
layout_minmax_کرښه(بنية box *لومړی,
		   صحيح *کرښه_min,
		   صحيح *کرښه_max,
		   bool لومړی_کرښه,
		   bool *کرښه_has_لوړوالی,
		   ثابت بنية gui_layout_جدول *font_func,
		   ثابت html_content *content)
{
	صحيح min = 0, max = 0, پلنوالی, لوړوالی, fixed;
	عائم frac;
	size_t i, j;
	بنية box *b;
	بنية box *بلاک;
	plot_font_style_t fstyle;
	bool no_wrap;

	أكد(لومړی->پلار);
	أكد(لومړی->پلار->پلار);
	أكد(لومړی->پلار->پلار->style);

	بلاک = لومړی->پلار->پلار;
	no_wrap = (css_computed_white_space(بلاک->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(بلاک->style) ==
			CSS_WHITE_SPACE_PRE);

	*کرښه_has_لوړوالی = false;

	/* corresponds to the pass 1 loop in layout_line() */
	لكل (b = لومړی; b; b = b->بل) {
		تعداد css_پلنوالی_e wtype;
		تعداد css_لوړوالی_e htype;
		تعداد css_box_sizing_e bs;
		css_fixed ارزښت = 0;
		css_unit unit = CSS_UNIT_PX;

		أكد(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		إذا (b->type == BOX_BR) {
			b = b->بل;
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
			b->min_پلنوالی = b->children->min_پلنوالی;
			b->max_پلنوالی = b->children->max_پلنوالی;
			إذا (min < b->min_پلنوالی)
				min = b->min_پلنوالی;
			max += b->max_پلنوالی;
			استمر;
		}

		إذا (b->type == BOX_INLINE_بلاک || b->type == BOX_INLINE_FLEX) {
			layout_minmax_بلاک(b, font_func, content);
			إذا (min < b->min_پلنوالی)
				min = b->min_پلنوالی;
			max += b->max_پلنوالی;

			إذا (b->flags & HAS_لوړوالی)
				*کرښه_has_لوړوالی = true;
			استمر;
		}

		أكد(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		إذا (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_پلنوالی(&content->unit_len_ctx,
					b->style, کیڼ_2, true, true, true,
					&fixed, &frac);
			إذا (!b->inline_پای_ټکی)
				calculate_mbp_پلنوالی(&content->unit_len_ctx,
						b->style, ښی_2,
						true, true, true,
						&fixed, &frac);
			إذا (0 < fixed)
				max += fixed;
			*کرښه_has_لوړوالی = true;
			/* \todo  update min width, consider fractional extra */
		} وإلا إذا (b->type == BOX_INLINE_پای_ټکی) {
			fixed = frac = 0;
			calculate_mbp_پلنوالی(&content->unit_len_ctx,
					b->inline_پای_ټکی->style, ښی_2,
					true, true, true,
					&fixed, &frac);
			إذا (0 < fixed)
				max += fixed;

			إذا (b->بل) {
				إذا (b->space == UNKNOWN_پلنوالی) {
					font_func->پلنوالی(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*کرښه_has_لوړوالی = true;
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

			إذا (b->پلنوالی == UNKNOWN_پلنوالی) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				إذا (b->پلار->پلار->gadget &&
						b->پلار->پلار->gadget->type
						== GADGET_SELECT) {
					صحيح opt_maxwidth = 0;
					بنية form_option *o;

					لكل (o = b->پلار->پلار->gadget->
							معلومات.select.توکي; o;
							o = o->بل) {
						صحيح opt_پلنوالی;
						font_func->پلنوالی(&fstyle,
								o->متن,
								مزي_اوږدوالی(o->متن),
								&opt_پلنوالی);

						إذا (opt_maxwidth < opt_پلنوالی)
							opt_maxwidth =opt_پلنوالی;
					}

					b->پلنوالی = opt_maxwidth;
					إذا (nsoption_bool(core_select_menu))
						b->پلنوالی += SCROLLBAR_پلنوالی;

				} وإلا {
					font_func->پلنوالی(&fstyle, b->متن,
						b->اوږدتیا, &b->پلنوالی);
					b->flags |= MEASURED;
				}
			}
			max += b->پلنوالی;
			إذا (b->بل) {
				إذا (b->space == UNKNOWN_پلنوالی) {
					font_func->پلنوالی(&fstyle, " ", 1,
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
				إذا (min < b->پلنوالی)
					min = b->پلنوالی;

			} وإلا إذا (b->پلار->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				افعل {
					لكل (j = i; j != b->اوږدتیا &&
							b->متن[j] != ' '; j++)
						;
					font_func->پلنوالی(&fstyle, b->متن + i,
							 j - i, &پلنوالی);
					إذا (min < پلنوالی)
						min = پلنوالی;
					i = j + 1;
				} طالما (j != b->اوږدتیا);
			}

			*کرښه_has_لوړوالی = true;

			استمر;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		bs = css_computed_box_sizing(بلاک->style);

		/* calculate box width */
		wtype = css_computed_پلنوالی_px(b->style,
				&content->unit_len_ctx, -1, &پلنوالی);
		إذا (wtype == CSS_پلنوالی_سېټ) {
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_پلنوالی(&content->unit_len_ctx,
						بلاک->style, کیڼ_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_پلنوالی(&content->unit_len_ctx,
						بلاک->style, ښی_2,
						false, true, true,
						&fixed, &frac);
				إذا (پلنوالی < fixed) {
					پلنوالی = fixed;
				}
			}
			إذا (پلنوالی < 0)
				پلنوالی = 0;
		} وإلا {
			پلنوالی = AUTO;
		}

		/*لوړوالی*/
		htype = css_computed_لوړوالی(b->style, &ارزښت, &unit);
		إذا (htype == CSS_لوړوالی_سېټ) {
			لوړوالی = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					ارزښت, unit));
		} وإلا {
			لوړوالی = AUTO;
		}

		إذا (b->object || (b->flags & REPLACE_DIM)) {
			إذا (b->object) {
				صحيح لنډمهاله_لوړوالی = لوړوالی;
				layout_get_object_dimensions(b,
						&پلنوالی, &لنډمهاله_لوړوالی,
						INT_MIN, لوی_عدد,
						INT_MIN, لوی_عدد);
			}

			fixed = frac = 0;
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_پلنوالی(&content->unit_len_ctx,
						b->style, کیڼ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_پلنوالی(&content->unit_len_ctx,
						b->style, ښی_2,
						true, false, false,
						&fixed, &frac);
			} وإلا {
				calculate_mbp_پلنوالی(&content->unit_len_ctx,
						b->style, کیڼ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_پلنوالی(&content->unit_len_ctx,
						b->style, ښی_2,
						true, true, true,
						&fixed, &frac);
			}
			إذا (0 < پلنوالی + fixed)
				پلنوالی += fixed;
		} وإلا إذا (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			إذا (پلنوالی == AUTO)
				پلنوالی = 400;

			fixed = frac = 0;
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_پلنوالی(&content->unit_len_ctx,
						b->style, کیڼ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_پلنوالی(&content->unit_len_ctx,
						b->style, ښی_2,
						true, false, false,
						&fixed, &frac);
			} وإلا {
				calculate_mbp_پلنوالی(&content->unit_len_ctx,
						b->style, کیڼ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_پلنوالی(&content->unit_len_ctx,
						b->style, ښی_2,
						true, true, true,
						&fixed, &frac);
			}

			إذا (0 < پلنوالی + fixed)
				پلنوالی += fixed;

		} وإلا {
			/* form control with no object */
			إذا (پلنوالی == AUTO)
				پلنوالی = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		إذا (min < پلنوالی && !box_has_percentage_max_پلنوالی(b))
			min = پلنوالی;
		إذا (پلنوالی > 0)
			max += پلنوالی;

		*کرښه_has_لوړوالی = true;
	}

	إذا (لومړی_کرښه) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		صحيح متن_indent = layout_متن_indent(&content->unit_len_ctx,
				لومړی->پلار->پلار->style, 100);
		min = (min + متن_indent < 0) ? 0 : min + متن_indent;
		max = (max + متن_indent < 0) ? 0 : max + متن_indent;
	}

	*کرښه_min = min;
	*کرښه_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	أكد(b != لومړی);
	أكد(0 <= *کرښه_min);
	أكد(*کرښه_min <= *کرښه_max);
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
			       bool *has_لوړوالی,
			       ثابت بنية gui_layout_جدول *font_func,
			       ثابت html_content *content)
{
	بنية box *ماشوم;
	صحيح کرښه_min = 0, کرښه_max = 0;
	صحيح min = 0, max = 0;
	bool لومړی_کرښه = true;
	bool کرښه_has_لوړوالی;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	إذا (inline_container->max_پلنوالی != UNKNOWN_MAX_پلنوالی)
		أرجع;

	*has_لوړوالی = false;

	لكل (ماشوم = inline_container->children; ماشوم; ) {
		ماشوم = layout_minmax_کرښه(ماشوم, &کرښه_min, &کرښه_max,
				لومړی_کرښه, &کرښه_has_لوړوالی, font_func,
				content);
		إذا (min < کرښه_min)
			min = کرښه_min;
		إذا (max < کرښه_max)
			max = کرښه_max;
		لومړی_کرښه = false;
		*has_لوړوالی |= کرښه_has_لوړوالی;
	}

	inline_container->min_پلنوالی = min;
	inline_container->max_پلنوالی = max;

	أكد(0 <= inline_container->min_پلنوالی &&
			inline_container->min_پلنوالی <=
			inline_container->max_پلنوالی);
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
	بنية box *ماشوم;
	صحيح min = 0, max = 0;
	صحيح extra_fixed = 0;
	عائم extra_frac = 0;
	تعداد css_پلنوالی_e wtype = CSS_پلنوالی_AUTO;
	css_fixed پلنوالی = 0;
	css_unit wunit = CSS_UNIT_PX;
	تعداد css_لوړوالی_e htype = CSS_لوړوالی_AUTO;
	css_fixed لوړوالی = 0;
	css_unit hunit = CSS_UNIT_PX;
	تعداد css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool ماشوم_has_لوړوالی = false;

	أكد(بلاک->type == BOX_بلاک ||
			بلاک->type == BOX_FLEX ||
			بلاک->type == BOX_INLINE_FLEX ||
			بلاک->type == BOX_INLINE_بلاک ||
			بلاک->type == BOX_جدول_CELL);

	/* check if the widths have already been calculated */
	إذا (بلاک->max_پلنوالی != UNKNOWN_MAX_پلنوالی)
		أرجع;

	إذا (بلاک->style != عدم) {
		wtype = css_computed_پلنوالی(بلاک->style, &پلنوالی, &wunit);
		htype = css_computed_لوړوالی(بلاک->style, &لوړوالی, &hunit);
		bs = css_computed_box_sizing(بلاک->style);
	}

	/* set whether the minimum width is of any interest for this box */
	إذا (((بلاک->پلار && lh__box_is_float_box(بلاک->پلار)) ||
			بلاک->type == BOX_INLINE_بلاک ||
			بلاک->type == BOX_INLINE_FLEX) &&
			wtype != CSS_پلنوالی_سېټ) {
		/* box shrinks to fit; need minimum width */
		بلاک->flags |= NEED_MIN;
	} وإلا إذا (بلاک->type == BOX_جدول_CELL) {
		/* box shrinks to fit; need minimum width */
		بلاک->flags |= NEED_MIN;
	} وإلا إذا (بلاک->پلار && (بلاک->پلار->flags & NEED_MIN) &&
			wtype != CSS_پلنوالی_سېټ) {
		/* box inside shrink-to-fit context; need minimum width */
		بلاک->flags |= NEED_MIN;
	} وإلا إذا (بلاک->پلار && (بلاک->پلار->type == BOX_FLEX)) {
		/* box is flex item */
		بلاک->flags |= NEED_MIN;
	}

	إذا (بلاک->gadget && (بلاک->gadget->type == GADGET_TEXTBOX ||
			بلاک->gadget->type == GADGET_PASSWORD ||
			بلاک->gadget->type == GADGET_FILE ||
			بلاک->gadget->type == GADGET_TEXTAREA) &&
			بلاک->style && wtype == CSS_پلنوالی_AUTO) {
		css_fixed اندازه = INTTOFIX(10);
		css_unit unit = CSS_UNIT_EM;

		min = max = FIXTOINT(css_unit_len2device_px(بلاک->style,
				&content->unit_len_ctx, اندازه, unit));

		بلاک->flags |= HAS_لوړوالی;
	}

	إذا (بلاک->gadget && (بلاک->gadget->type == GADGET_RADIO ||
			بلاک->gadget->type == GADGET_CHECKBOX) &&
			بلاک->style && wtype == CSS_پلنوالی_AUTO) {
		css_fixed اندازه = INTTOFIX(1);
		css_unit unit = CSS_UNIT_EM;

		/* form checkbox or radio button
		 * if width is AUTO, set it to 1em */
		min = max = FIXTOINT(css_unit_len2device_px(بلاک->style,
				&content->unit_len_ctx, اندازه, unit));

		بلاک->flags |= HAS_لوړوالی;
	}

	إذا (بلاک->object) {
		إذا (content_get_type(بلاک->object) == CONTENT_HTML) {
			layout_minmax_بلاک(html_get_box_ونه(بلاک->object),
					font_func, content);
			min = html_get_box_ونه(بلاک->object)->min_پلنوالی;
			max = html_get_box_ونه(بلاک->object)->max_پلنوالی;
		} وإلا {
			min = max = content_get_پلنوالی(بلاک->object);
		}

		بلاک->flags |= HAS_لوړوالی;
	} وإلا إذا (بلاک->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		بلاک->flags |= HAS_لوړوالی;
	} وإلا {
		/* recurse through children */
		لكل (ماشوم = بلاک->children; ماشوم; ماشوم = ماشوم->بل) {
			بدل (ماشوم->type) {
			حالة BOX_FLEX:
			حالة BOX_بلاک:
				layout_minmax_بلاک(ماشوم, font_func,
						content);
				إذا (ماشوم->flags & HAS_لوړوالی)
					ماشوم_has_لوړوالی = true;
				اكسر;
			حالة BOX_INLINE_CONTAINER:
				إذا (بلاک->flags & NEED_MIN)
					ماشوم->flags |= NEED_MIN;

				layout_minmax_inline_container(ماشوم,
						&ماشوم_has_لوړوالی, font_func,
						content);
				إذا (ماشوم_has_لوړوالی &&
						ماشوم ==
						ماشوم->پلار->children) {
					بلاک->flags |= MAKE_لوړوالی;
				}
				اكسر;
			حالة BOX_جدول:
				layout_minmax_جدول(ماشوم, font_func,
						content);
				/* todo: fix for zero height tables */
				ماشوم_has_لوړوالی = true;
				ماشوم->flags |= MAKE_لوړوالی;
				اكسر;
			افتراضي:
				أكد(0);
			}
			أكد(ماشوم->max_پلنوالی != UNKNOWN_MAX_پلنوالی);

			إذا (ماشوم->style &&
					(css_computed_position(ماشوم->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(ماشوم->style) ==
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
					min += ماشوم->min_پلنوالی;
				} وإلا {
					إذا (min < ماشوم->min_پلنوالی)
						min = ماشوم->min_پلنوالی;
				}
				max += ماشوم->max_پلنوالی;

			} وإلا {
				إذا (min < ماشوم->min_پلنوالی)
					min = ماشوم->min_پلنوالی;
				إذا (max < ماشوم->max_پلنوالی)
					max = ماشوم->max_پلنوالی;
			}

			إذا (ماشوم_has_لوړوالی)
				بلاک->flags |= HAS_لوړوالی;
		}
	}

	إذا (max < min) {
		box_dump(stderr, بلاک, 0, true);
		أكد(0);
	}

	/* fixed width takes priority */
	إذا (بلاک->type != BOX_جدول_CELL && !lh_box_is_flex_توکی(بلاک)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		تعداد css_max_پلنوالی_e max_type;
		تعداد css_min_پلنوالی_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed ارزښت = 0;
		صحيح پلنوالی;

		إذا (css_computed_پلنوالی_px(بلاک->style, &content->unit_len_ctx,
				-1, &پلنوالی) == CSS_پلنوالی_سېټ) {
			min = max = پلنوالی;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_پلنوالی(بلاک->style, &ارزښت, &unit);
		إذا (min_type == CSS_MIN_پلنوالی_سېټ && unit != CSS_UNIT_PCT) {
			صحيح val = FIXTOINT(css_unit_len2device_px(بلاک->style,
					&content->unit_len_ctx, ارزښت, unit));

			إذا (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_پلنوالی(بلاک->style, &ارزښت, &unit);
		إذا (max_type == CSS_MAX_پلنوالی_سېټ && unit != CSS_UNIT_PCT) {
			صحيح val = FIXTOINT(css_unit_len2device_px(بلاک->style,
					&content->unit_len_ctx, ارزښت, unit));

			إذا (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	إذا (htype == CSS_لوړوالی_سېټ && hunit != CSS_UNIT_PCT &&
			لوړوالی > INTTOFIX(0)) {
		بلاک->flags |= MAKE_لوړوالی;
		بلاک->flags |= HAS_لوړوالی;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_پلنوالی(&content->unit_len_ctx, بلاک->style, کیڼ_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_پلنوالی(&content->unit_len_ctx, بلاک->style, ښی_2,
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

	calculate_mbp_پلنوالی(&content->unit_len_ctx, بلاک->style, کیڼ_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_پلنوالی(&content->unit_len_ctx, بلاک->style, ښی_2,
			true, false, false, &extra_fixed, &extra_frac);

	إذا (extra_fixed < 0)
		extra_fixed = 0;
	إذا (extra_frac < 0)
		extra_frac = 0;
	إذا (1.0 <= extra_frac)
		extra_frac = 0.9;
	إذا (بلاک->style != عدم &&
			(css_computed_float(بلاک->style) == CSS_FLOAT_کیڼ ||
			css_computed_float(بلاک->style) == CSS_FLOAT_ښی)) {
		/* floated boxs */
		بلاک->min_پلنوالی = min + extra_fixed;
		بلاک->max_پلنوالی = max + extra_fixed;
	} وإلا {
		/* not floated */
		بلاک->min_پلنوالی = (min + extra_fixed) / (1.0 - extra_frac);
		بلاک->max_پلنوالی = (max + extra_fixed) / (1.0 - extra_frac);
	}

	أكد(0 <= بلاک->min_پلنوالی);
	أكد(بلاک->min_پلنوالی <= بلاک->max_پلنوالی);
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
layout_بل_margin_بلاک(ثابت css_unit_ctx *unit_len_ctx,
			 بنية box *box,
			 بنية box *بلاک,
			 صحيح viewport_لوړوالی,
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
				layout_پیدا_کړه_dimensions(unit_len_ctx,
						box->پلار->پلنوالی,
						viewport_لوړوالی, box,
						box->style,
						عدم, عدم, عدم, عدم,
						عدم, عدم, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				إذا (*max_pos_margin < box->margin[سر_2])
					*max_pos_margin = box->margin[سر_2];
				وإلا إذا (*max_neg_margin < -box->margin[سر_2])
					*max_neg_margin = -box->margin[سر_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			إذا (box->flags & MAKE_لوړوالی ||
					box->border[سر_2].پلنوالی ||
					box->padding[سر_2] ||
					(box->style &&
					css_computed_ډکېدنه_y(box->style) !=
					CSS_ډکېدنه_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_لومړی_ماشوم(box))) {
				/* Collapse to this box; return it */
				أرجع box;
			}
		}


		/* Find next box */
		إذا (box->type == BOX_بلاک && !box->object && box->children &&
				box->style &&
				css_computed_ډکېدنه_y(box->style) ==
				CSS_ډکېدنه_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} وإلا {
			إذا (!box->بل) {
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

					box = box->پلار;
				} طالما (box != بلاک && !box->بل);

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
			box = box->بل;

			/* Get margins */
			إذا (box->style) {
				layout_پیدا_کړه_dimensions(unit_len_ctx,
						box->پلار->پلنوالی,
						viewport_لوړوالی, box,
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
	لكل (; fl; fl = fl->بل_float) {
		إذا ((clear == CSS_CLEAR_کیڼ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_کیڼ)
			إذا (y < fl->y + fl->لوړوالی)
				y = fl->y + fl->لوړوالی;
		إذا ((clear == CSS_CLEAR_ښی || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ښی)
			إذا (y < fl->y + fl->لوړوالی)
				y = fl->y + fl->لوړوالی;
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
پیدا_کړه_sides(بنية box *fl,
	   صحيح y0, صحيح y1,
	   صحيح *x0, صحيح *x1,
	   بنية box **کیڼ,
	   بنية box **ښی)
{
	صحيح fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*کیڼ = *ښی = 0;
	لكل (; fl; fl = fl->بل_float) {
		fy1 = fl->y + fl->لوړوالی;
		إذا (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			أرجع;
		}
		fy0 = fl->y;
		إذا (y0 < fy1 && fy0 <= y1) {
			إذا (fl->type == BOX_FLOAT_کیڼ) {
				fx1 = fl->x + fl->پلنوالی;
				إذا (*x0 < fx1) {
					*x0 = fx1;
					*کیڼ = fl;
				}
			} وإلا {
				fx0 = fl->x;
				إذا (fx0 < *x1) {
					*x1 = fx0;
					*ښی = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *کیڼ, *ښی);
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
layout_solve_پلنوالی(بنية box *box,
		   صحيح شته_پلنوالی,
		   صحيح پلنوالی,
		   صحيح lm,
		   صحيح rm,
		   صحيح max_پلنوالی,
		   صحيح min_پلنوالی)
{
	bool auto_پلنوالی = false;

	/* Increase specified left/right margins */
	إذا (box->margin[کیڼ_2] != AUTO && box->margin[کیڼ_2] < lm &&
			box->margin[کیڼ_2] >= 0)
		box->margin[کیڼ_2] = lm;
	إذا (box->margin[ښی_2] != AUTO && box->margin[ښی_2] < rm &&
			box->margin[ښی_2] >= 0)
		box->margin[ښی_2] = rm;

	/* Find width */
	إذا (پلنوالی == AUTO) {
		صحيح margin_کیڼ = box->margin[کیڼ_2];
		صحيح margin_ښی = box->margin[ښی_2];

		إذا (margin_کیڼ == AUTO) {
			margin_کیڼ = lm;
		}
		إذا (margin_ښی == AUTO) {
			margin_ښی = rm;
		}

		پلنوالی = شته_پلنوالی -
				(margin_کیڼ + box->border[کیڼ_2].پلنوالی +
				box->padding[کیڼ_2] + box->padding[ښی_2] +
				box->border[ښی_2].پلنوالی + margin_ښی);
		پلنوالی = پلنوالی < 0 ? 0 : پلنوالی;
		auto_پلنوالی = true;
	}

	إذا (max_پلنوالی >= 0 && پلنوالی > max_پلنوالی) {
		/* max-width is admissable and width exceeds max-width */
		پلنوالی = max_پلنوالی;
		auto_پلنوالی = false;
	}

	إذا (min_پلنوالی > 0 && پلنوالی < min_پلنوالی) {
		/* min-width is admissable and width is less than max-width */
		پلنوالی = min_پلنوالی;
		auto_پلنوالی = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	إذا (auto_پلنوالی) {
		/* any other 'auto' become 0 or the minimum required values */
		إذا (box->margin[کیڼ_2] == AUTO) {
			box->margin[کیڼ_2] = lm;
		}
		إذا (box->margin[ښی_2] == AUTO) {
			box->margin[ښی_2] = rm;
		}
		أرجع پلنوالی;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	إذا (box->margin[کیڼ_2] != AUTO && box->margin[ښی_2] != AUTO &&
			box->پلار != عدم && box->پلار->style != عدم) {
		بدل (css_computed_متن_align(box->پلار->style)) {
		حالة CSS_متن_ALIGN_LIBCSS_ښی:
			box->margin[کیڼ_2] = AUTO;
			box->margin[ښی_2] = 0;
			اكسر;
		حالة CSS_متن_ALIGN_LIBCSS_CENTER:
			box->margin[کیڼ_2] = box->margin[ښی_2] = AUTO;
			اكسر;
		حالة CSS_متن_ALIGN_LIBCSS_کیڼ:
			box->margin[کیڼ_2] = 0;
			box->margin[ښی_2] = AUTO;
			اكسر;
		افتراضي:
			/* Leave it alone; no HTML alignment */
			اكسر;
		}
	}

	إذا (box->margin[کیڼ_2] == AUTO && box->margin[ښی_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[کیڼ_2] = box->margin[ښی_2] =
				(شته_پلنوالی - lm - rm -
				(box->border[کیڼ_2].پلنوالی + box->padding[کیڼ_2] +
				پلنوالی + box->padding[ښی_2] +
				box->border[ښی_2].پلنوالی)) / 2;

		إذا (box->margin[کیڼ_2] < 0) {
			box->margin[ښی_2] += box->margin[کیڼ_2];
			box->margin[کیڼ_2] = 0;
		}

		box->margin[کیڼ_2] += lm;

	} وإلا إذا (box->margin[کیڼ_2] == AUTO) {
		box->margin[کیڼ_2] = شته_پلنوالی - lm -
				(box->border[کیڼ_2].پلنوالی + box->padding[کیڼ_2] +
				پلنوالی + box->padding[ښی_2] +
				box->border[ښی_2].پلنوالی + box->margin[ښی_2]);
		box->margin[کیڼ_2] = box->margin[کیڼ_2] < lm
				? lm : box->margin[کیڼ_2];
	} وإلا {
		/* margin-right auto or "over-constrained" */
		box->margin[ښی_2] = شته_پلنوالی - rm -
				(box->margin[کیڼ_2] + box->border[کیڼ_2].پلنوالی +
				 box->padding[کیڼ_2] + پلنوالی +
				 box->padding[ښی_2] +
				 box->border[ښی_2].پلنوالی);
	}

	أرجع پلنوالی;
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
layout_بلاک_پیدا_کړه_dimensions(ثابت css_unit_ctx *unit_len_ctx,
			     صحيح شته_پلنوالی,
			     صحيح viewport_لوړوالی,
			     صحيح lm,
			     صحيح rm,
			     بنية box *box)
{
	صحيح پلنوالی, max_پلنوالی, min_پلنوالی;
	صحيح لوړوالی, max_لوړوالی, min_لوړوالی;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	ثابت css_computed_style *style = box->style;

	layout_پیدا_کړه_dimensions(unit_len_ctx, شته_پلنوالی, viewport_لوړوالی, box,
			style, &پلنوالی, &لوړوالی, &max_پلنوالی, &min_پلنوالی,
			&max_لوړوالی, &min_لوړوالی, margin, padding, border);

	إذا (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &پلنوالی, &لوړوالی,
				min_پلنوالی, max_پلنوالی, min_لوړوالی, max_لوړوالی);
	}

	box->پلنوالی = layout_solve_پلنوالی(box, شته_پلنوالی, پلنوالی, lm, rm,
			max_پلنوالی, min_پلنوالی);
	box->لوړوالی = لوړوالی;

	إذا (margin[سر_2] == AUTO)
		margin[سر_2] = 0;
	إذا (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
ساكن فراغ layout_بلاک_جمع_کړه_scrollbar(بنية box *box, صحيح which)
{
	تعداد css_ډکېدنه_e ډکېدنه_x, ډکېدنه_y;

	أكد(box->type == BOX_بلاک && (which == ښی_2 || which == BOTTOM));

	إذا (box->style == عدم)
		أرجع;

	ډکېدنه_x = css_computed_ډکېدنه_x(box->style);
	ډکېدنه_y = css_computed_ډکېدنه_y(box->style);

	إذا (which == BOTTOM &&
			(ډکېدنه_x == CSS_ډکېدنه_SCROLL ||
			 ډکېدنه_x == CSS_ډکېدنه_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		إذا (box->لوړوالی != AUTO &&
				(ډکېدنه_x == CSS_ډکېدنه_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_پلنوالی;
		}

	} وإلا إذا (which == ښی_2 &&
			(ډکېدنه_y == CSS_ډکېدنه_SCROLL ||
			 ډکېدنه_y == CSS_ډکېدنه_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		تعداد css_لوړوالی_e htype;
		css_fixed لوړوالی = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_لوړوالی(box->style, &لوړوالی, &hunit);

		إذا (which == ښی_2 && box->پلنوالی != AUTO &&
				htype == CSS_لوړوالی_سېټ &&
				(ډکېدنه_y == CSS_ډکېدنه_SCROLL ||
				box_vscrollbar_present(box))) {
			box->پلنوالی -= SCROLLBAR_پلنوالی;
			box->padding[ښی_2] += SCROLLBAR_پلنوالی;
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

	لكل (box = box->children; box; box = box->بل) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_جدول(
		بنية box *جدول,
		صحيح شته_پلنوالی,
		html_content *content)
{
	بینښه صحيح columns = جدول->columns;  /* total columns */
	بینښه صحيح i;
	بینښه صحيح *لیکه_پراختیا;
	صحيح *excess_y;
	صحيح جدول_پلنوالی, min_پلنوالی = 0, max_پلنوالی = 0;
	صحيح required_پلنوالی = 0;
	صحيح x, remainder = 0, شمېر = 0;
	صحيح جدول_لوړوالی = 0;
	صحيح min_لوړوالی = 0;
	صحيح *xs;  /* array of column x positions */
	صحيح auto_پلنوالی;
	صحيح اضافي_پلنوالی;
	صحيح relative_مجموعه = 0;
	صحيح border_spacing_h = 0, border_spacing_v = 0;
	صحيح اضافي_لوړوالی;
	صحيح positioned_columns = 0;
	بنية box *containing_بلاک = عدم;
	بنية box *c;
	بنية box *لیکه;
	بنية box *لیکه_ډله;
	بنية box **لیکه_پراختیا_cell;
	بنية ستنه *ستون;
	ثابت css_computed_style *style = جدول->style;
	تعداد css_لوړوالی_e htype;
	css_fixed ارزښت = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(جدول->type == BOX_جدول);
	أكد(style);
	أكد(جدول->children && جدول->children->children);
	أكد(columns);

	/* allocate working buffers */
	ستون = حافظه_ورکړه(columns * حجم ستون[0]);
	excess_y = حافظه_ورکړه(columns * حجم excess_y[0]);
	لیکه_پراختیا = حافظه_ورکړه(columns * حجم لیکه_پراختیا[0]);
	لیکه_پراختیا_cell = حافظه_ورکړه(columns * حجم لیکه_پراختیا_cell[0]);
	xs = حافظه_ورکړه((columns + 1) * حجم xs[0]);
	إذا (!ستون || !xs || !لیکه_پراختیا || !excess_y || !لیکه_پراختیا_cell) {
		حرر(ستون);
		حرر(excess_y);
		حرر(لیکه_پراختیا);
		حرر(لیکه_پراختیا_cell);
		حرر(xs);
		أرجع false;
	}

	حافظه_نقل(ستون, جدول->ستون, حجم(ستون[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_پیدا_کړه_dimensions(&content->unit_len_ctx, شته_پلنوالی, -1, جدول,
			style, 0, 0, 0, 0, 0, 0, جدول->margin, جدول->padding,
			جدول->border);
	لكل (لیکه_ډله = جدول->children; لیکه_ډله;
			لیکه_ډله = لیکه_ډله->بل) {
		لكل (لیکه = لیکه_ډله->children; لیکه; لیکه = لیکه->بل) {
			لكل (c = لیکه->children; c; c = c->بل) {
				تعداد css_ډکېدنه_e ډکېدنه_x;
				تعداد css_ډکېدنه_e ډکېدنه_y;

				أكد(c->style);
				جدول_کارول_شوی_border_for_cell(
						&content->unit_len_ctx, c);
				layout_پیدا_کړه_dimensions(&content->unit_len_ctx,
						شته_پلنوالی, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				ډکېدنه_x = css_computed_ډکېدنه_x(c->style);
				ډکېدنه_y = css_computed_ډکېدنه_y(c->style);

				إذا (ډکېدنه_x == CSS_ډکېدنه_SCROLL ||
						ډکېدنه_x ==
						CSS_ډکېدنه_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_پلنوالی;
				}
				إذا (ډکېدنه_y == CSS_ډکېدنه_SCROLL ||
						ډکېدنه_y ==
						CSS_ډکېدنه_AUTO) {
					c->padding[ښی_2] += SCROLLBAR_پلنوالی;
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
	إذا (css_computed_پلنوالی_px(style, &content->unit_len_ctx,
			شته_پلنوالی, &جدول_پلنوالی) == CSS_پلنوالی_سېټ) {
		/* specified width includes border */
		جدول_پلنوالی -= جدول->border[کیڼ_2].پلنوالی +
				جدول->border[ښی_2].پلنوالی;
		جدول_پلنوالی = جدول_پلنوالی < 0 ? 0 : جدول_پلنوالی;

		auto_پلنوالی = جدول_پلنوالی;
	} وإلا {
		جدول_پلنوالی = AUTO;
		auto_پلنوالی = شته_پلنوالی -
				((جدول->margin[کیڼ_2] == AUTO ? 0 :
						جدول->margin[کیڼ_2]) +
				 جدول->border[کیڼ_2].پلنوالی +
				 جدول->padding[کیڼ_2] +
				 جدول->padding[ښی_2] +
				 جدول->border[ښی_2].پلنوالی +
				 (جدول->margin[ښی_2] == AUTO ? 0 :
						جدول->margin[ښی_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_لوړوالی(style, &ارزښت, &unit);
	إذا (htype == CSS_لوړوالی_سېټ) {
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
						CSS_FLOAT_کیڼ ||
					css_computed_float(جدول->style) ==
						CSS_FLOAT_ښی)) {
				/* Table is a float */
				أكد(جدول->پلار && جدول->پلار->پلار &&
						جدول->پلار->پلار->پلار);
				containing_بلاک =
						جدول->پلار->پلار->پلار;
			} وإلا إذا (جدول->پلار && جدول->پلار->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_بلاک = جدول->پلار;
			} وإلا إذا (جدول->پلار && جدول->پلار->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				أكد(جدول->پلار->پلار);
				containing_بلاک = جدول->پلار->پلار;
			}

			إذا (containing_بلاک) {
				css_fixed ignored = 0;

				htype = css_computed_لوړوالی(
						containing_بلاک->style,
						&ignored, &unit);
			}

			إذا (containing_بلاک &&
					containing_بلاک->لوړوالی != AUTO &&
					(css_computed_position(جدول->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_لوړوالی_سېټ)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_لوړوالی = FPCT_د_INT_TOINT(ارزښت,
						containing_بلاک->لوړوالی);
			}
		} وإلا {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_لوړوالی = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					ارزښت, unit));
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
				      })[ستون[i].type],
		      ستون[i].پلنوالی,
		      ستون[i].min,
		      ستون[i].max);


		إذا (ستون[i].positioned) {
			positioned_columns++;
			استمر;
		} وإلا إذا (ستون[i].type == ستنه_پلنوالی_FIXED) {
			إذا (ستون[i].پلنوالی < ستون[i].min)
				ستون[i].پلنوالی = ستون[i].max = ستون[i].min;
			وإلا
				ستون[i].min = ستون[i].max = ستون[i].پلنوالی;
			required_پلنوالی += ستون[i].پلنوالی;
		} وإلا إذا (ستون[i].type == ستنه_پلنوالی_PERCENT) {
			صحيح پلنوالی = ستون[i].پلنوالی * auto_پلنوالی / 100;
			required_پلنوالی += ستون[i].min < پلنوالی ? پلنوالی :
					ستون[i].min;
		} وإلا
			required_پلنوالی += ستون[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_پلنوالی);
	}
	required_پلنوالی += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", جدول_پلنوالی,
	      جدول->min_پلنوالی, جدول->max_پلنوالی, auto_پلنوالی, required_پلنوالی);

	إذا (auto_پلنوالی < required_پلنوالی) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		لكل (i = 0; i != columns; i++) {
			إذا (ستون[i].type == ستنه_پلنوالی_RELATIVE)
				استمر;
			إذا (ستون[i].type == ستنه_پلنوالی_PERCENT) {
				ستون[i].max = auto_پلنوالی * ستون[i].پلنوالی / 100;
				إذا (ستون[i].max < ستون[i].min)
					ستون[i].max = ستون[i].min;
			}
			min_پلنوالی += ستون[i].min;
			max_پلنوالی += ستون[i].max;
		}
	} وإلا {
		/* take percentages exactly */
		لكل (i = 0; i != columns; i++) {
			إذا (ستون[i].type == ستنه_پلنوالی_RELATIVE)
				استمر;
			إذا (ستون[i].type == ستنه_پلنوالی_PERCENT) {
				صحيح پلنوالی = auto_پلنوالی * ستون[i].پلنوالی / 100;
				إذا (پلنوالی < ستون[i].min)
					پلنوالی = ستون[i].min;
				ستون[i].min = ستون[i].پلنوالی = ستون[i].max = پلنوالی;
				ستون[i].type = ستنه_پلنوالی_FIXED;
			}
			min_پلنوالی += ستون[i].min;
			max_پلنوالی += ستون[i].max;
		}
	}

	/* allocate relative widths */
	اضافي_پلنوالی = auto_پلنوالی;
	لكل (i = 0; i != columns; i++) {
		إذا (ستون[i].type == ستنه_پلنوالی_RELATIVE)
			relative_مجموعه += ستون[i].پلنوالی;
		وإلا إذا (ستون[i].type == ستنه_پلنوالی_FIXED)
			اضافي_پلنوالی -= ستون[i].پلنوالی;
		وإلا
			اضافي_پلنوالی -= ستون[i].min;
	}
	اضافي_پلنوالی -= (columns + 1) * border_spacing_h;
	إذا (relative_مجموعه != 0) {
		إذا (اضافي_پلنوالی < 0)
			اضافي_پلنوالی = 0;
		لكل (i = 0; i != columns; i++) {
			إذا (ستون[i].type == ستنه_پلنوالی_RELATIVE) {
				ستون[i].min = ceil(ستون[i].max =
						(عائم) اضافي_پلنوالی
						* (عائم) ستون[i].پلنوالی
						/ relative_مجموعه);
				min_پلنوالی += ستون[i].min;
				max_پلنوالی += ستون[i].max;
			}
		}
	}
	min_پلنوالی += (columns + 1) * border_spacing_h;
	max_پلنوالی += (columns + 1) * border_spacing_h;

	إذا (auto_پلنوالی <= min_پلنوالی) {
		/* not enough space: minimise column widths */
		لكل (i = 0; i < columns; i++) {
			ستون[i].پلنوالی = ستون[i].min;
		}
		جدول_پلنوالی = min_پلنوالی;
	} وإلا إذا (max_پلنوالی <= auto_پلنوالی) {
		/* more space than maximum width */
		إذا (جدول_پلنوالی == AUTO) {
			/* for auto-width tables, make columns max width */
			لكل (i = 0; i < columns; i++) {
				ستون[i].پلنوالی = ستون[i].max;
			}
			جدول_پلنوالی = max_پلنوالی;
		} وإلا {
			/* for fixed-width tables, distribute the extra space
			 * too */
			بینښه صحيح flexible_columns = 0;
			لكل (i = 0; i != columns; i++)
				إذا (ستون[i].type != ستنه_پلنوالی_FIXED)
					flexible_columns++;
			إذا (flexible_columns == 0) {
				صحيح extra = (جدول_پلنوالی - max_پلنوالی) / columns;
				remainder = (جدول_پلنوالی - max_پلنوالی) -
						(extra * columns);
				لكل (i = 0; i != columns; i++) {
					ستون[i].پلنوالی = ستون[i].max + extra;
					شمېر -= remainder;
					إذا (شمېر < 0) {
						ستون[i].پلنوالی++;
						شمېر += columns;
					}
				}

			} وإلا {
				صحيح extra = (جدول_پلنوالی - max_پلنوالی) /
						flexible_columns;
				remainder = (جدول_پلنوالی - max_پلنوالی) -
						(extra * flexible_columns);
				لكل (i = 0; i != columns; i++)
					إذا (ستون[i].type != ستنه_پلنوالی_FIXED) {
						ستون[i].پلنوالی = ستون[i].max +
								extra;
						شمېر -= remainder;
						إذا (شمېر < 0) {
							ستون[i].پلنوالی++;
							شمېر += flexible_columns;
						}
					}
			}
		}
	} وإلا {
		/* space between min and max: fill it exactly */
		عائم scale = (عائم) (auto_پلنوالی - min_پلنوالی) /
				(عائم) (max_پلنوالی - min_پلنوالی);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		لكل (i = 0; i < columns; i++) {
			ستون[i].پلنوالی = ستون[i].min + (صحيح) (0.5 +
					(ستون[i].max - ستون[i].min) * scale);
		}
		جدول_پلنوالی = auto_پلنوالی;
	}

	xs[0] = x = border_spacing_h;
	لكل (i = 0; i != columns; i++) {
		إذا (!ستون[i].positioned)
			x += ستون[i].پلنوالی + border_spacing_h;
		xs[i + 1] = x;
		لیکه_پراختیا[i] = 0;
		excess_y[i] = 0;
		لیکه_پراختیا_cell[i] = 0;
	}

	/* position cells */
	جدول_لوړوالی = border_spacing_v;
	لكل (لیکه_ډله = جدول->children; لیکه_ډله;
			لیکه_ډله = لیکه_ډله->بل) {
		صحيح لیکه_ډله_لوړوالی = 0;
		لكل (لیکه = لیکه_ډله->children; لیکه; لیکه = لیکه->بل) {
			صحيح لیکه_لوړوالی = 0;

			htype = css_computed_لوړوالی(لیکه->style, &ارزښت, &unit);
			إذا (htype == CSS_لوړوالی_سېټ && unit != CSS_UNIT_PCT) {
				لیکه_لوړوالی = FIXTOINT(css_unit_len2device_px(
						لیکه->style,
						&content->unit_len_ctx,
						ارزښت, unit));
			}
			لكل (c = لیکه->children; c; c = c->بل) {
				أكد(c->style);
				c->پلنوالی = xs[c->پیل_ستنه + c->columns] -
						xs[c->پیل_ستنه] -
						border_spacing_h -
						c->border[کیڼ_2].پلنوالی -
						c->padding[کیڼ_2] -
						c->padding[ښی_2] -
						c->border[ښی_2].پلنوالی;
				c->float_children = 0;
				c->cached_ځای_پرځای_کړه_لاندې_level = 0;

				c->لوړوالی = AUTO;
				إذا (!layout_بلاک_context(c, -1, content)) {
					حرر(ستون);
					حرر(excess_y);
					حرر(لیکه_پراختیا);
					حرر(لیکه_پراختیا_cell);
					حرر(xs);
					أرجع false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->لوړوالی;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_لوړوالی(c->style,
						&ارزښت, &unit);

				إذا (htype == CSS_لوړوالی_سېټ &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					صحيح h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							ارزښت, unit));
					إذا (c->لوړوالی < h)
						c->لوړوالی = h;
				}
				/* specified row height is treated as a minimum
				 */
				إذا (c->لوړوالی < لیکه_لوړوالی)
					c->لوړوالی = لیکه_لوړوالی;
				c->x = xs[c->پیل_ستنه] +
						c->border[کیڼ_2].پلنوالی;
				c->y = c->border[سر_2].پلنوالی;
				لكل (i = 0; i != c->columns; i++) {
					لیکه_پراختیا[c->پیل_ستنه + i] = c->لیکې;
					excess_y[c->پیل_ستنه + i] =
							c->border[سر_2].پلنوالی +
							c->padding[سر_2] +
							c->لوړوالی +
							c->padding[BOTTOM] +
							c->border[BOTTOM].پلنوالی;
					لیکه_پراختیا_cell[c->پیل_ستنه + i] = 0;
				}
				لیکه_پراختیا_cell[c->پیل_ستنه] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[سر_2].پلنوالی -
						c->padding[سر_2] -
						c->لوړوالی -
						c->border[BOTTOM].پلنوالی;
			}
			لكل (i = 0; i != columns; i++)
				إذا (لیکه_پراختیا[i] != 0)
					لیکه_پراختیا[i]--;
				وإلا
					لیکه_پراختیا_cell[i] = 0;
			إذا (لیکه->بل || لیکه_ډله->بل) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				لكل (i = 0; i != columns; i++)
					إذا (لیکه_پراختیا[i] == 0 && لیکه_لوړوالی <
							excess_y[i])
						لیکه_لوړوالی = excess_y[i];
			} وإلا {
				/* except in the last row */
				لكل (i = 0; i != columns; i++)
					إذا (لیکه_لوړوالی < excess_y[i])
						لیکه_لوړوالی = excess_y[i];
			}
			لكل (i = 0; i != columns; i++) {
				إذا (لیکه_لوړوالی < excess_y[i])
					excess_y[i] -= لیکه_لوړوالی;
				وإلا
					excess_y[i] = 0;
				إذا (لیکه_پراختیا_cell[i] != 0)
					لیکه_پراختیا_cell[i]->padding[BOTTOM] +=
							لیکه_لوړوالی +
							border_spacing_v;
			}

			لیکه->x = 0;
			لیکه->y = لیکه_ډله_لوړوالی;
			لیکه->پلنوالی = جدول_پلنوالی;
			لیکه->لوړوالی = لیکه_لوړوالی;
			لیکه_ډله_لوړوالی += لیکه_لوړوالی + border_spacing_v;
		}
		لیکه_ډله->x = 0;
		لیکه_ډله->y = جدول_لوړوالی;
		لیکه_ډله->پلنوالی = جدول_پلنوالی;
		لیکه_ډله->لوړوالی = لیکه_ډله_لوړوالی;
		جدول_لوړوالی += لیکه_ډله_لوړوالی;
	}
	/* Table height is either the height of the contents, or specified
	 * height if greater */
	جدول_لوړوالی = max(جدول_لوړوالی, min_لوړوالی);
	/** \todo distribute spare height over the row groups / rows / cells */

	/* perform vertical alignment */
	لكل (لیکه_ډله = جدول->children; لیکه_ډله;
			لیکه_ډله = لیکه_ډله->بل) {
		لكل (لیکه = لیکه_ډله->children; لیکه; لیکه = لیکه->بل) {
			لكل (c = لیکه->children; c; c = c->بل) {
				تعداد css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				اضافي_لوړوالی = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->لوړوالی - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &ارزښت, &unit);

				بدل (vertical_align) {
				حالة CSS_VERTICAL_ALIGN_SUB:
				حالة CSS_VERTICAL_ALIGN_SUPER:
				حالة CSS_VERTICAL_ALIGN_متن_سر:
				حالة CSS_VERTICAL_ALIGN_متن_BOTTOM:
				حالة CSS_VERTICAL_ALIGN_سېټ:
				حالة CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				حالة CSS_VERTICAL_ALIGN_سر:
					اكسر;
				حالة CSS_VERTICAL_ALIGN_منځ:
					c->padding[سر_2] += اضافي_لوړوالی / 2;
					c->padding[BOTTOM] -= اضافي_لوړوالی / 2;
					layout_move_children(c, 0,
							اضافي_لوړوالی / 2);
					اكسر;
				حالة CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[سر_2] += اضافي_لوړوالی;
					c->padding[BOTTOM] -= اضافي_لوړوالی;
					layout_move_children(c, 0,
							اضافي_لوړوالی);
					اكسر;
				حالة CSS_VERTICAL_ALIGN_INHERIT:
					أكد(0);
					اكسر;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	إذا (جدول->margin[سر_2] == AUTO)
		جدول->margin[سر_2] = 0;
	إذا (جدول->margin[BOTTOM] == AUTO)
		جدول->margin[BOTTOM] = 0;

	حرر(ستون);
	حرر(excess_y);
	حرر(لیکه_پراختیا);
	حرر(لیکه_پراختیا_cell);
	حرر(xs);

	جدول->پلنوالی = جدول_پلنوالی;
	جدول->لوړوالی = جدول_لوړوالی;

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
ساكن bool layout_apply_minmax_لوړوالی(
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
			(css_computed_float(box->style) == CSS_FLOAT_کیڼ ||
			 css_computed_float(box->style) == CSS_FLOAT_ښی)) {
		/* Box is a float */
		أكد(box->پلار && box->پلار->پلار &&
				box->پلار->پلار->پلار);
		containing_بلاک = box->پلار->پلار->پلار;
	} وإلا إذا (box->پلار && box->پلار->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_بلاک = box->پلار;
	} وإلا إذا (box->پلار && box->پلار->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		أكد(box->پلار->پلار);
		containing_بلاک = box->پلار->پلار;
	}

	إذا (box->style) {
		تعداد css_لوړوالی_e htype = CSS_لوړوالی_AUTO;
		css_fixed ارزښت = 0;
		css_unit unit = CSS_UNIT_PX;

		إذا (containing_بلاک) {
			htype = css_computed_لوړوالی(containing_بلاک->style,
					&ارزښت, &unit);
		}

		/* max-height */
		إذا (css_computed_max_لوړوالی(box->style, &ارزښت, &unit) ==
				CSS_MAX_لوړوالی_سېټ) {
			إذا (unit == CSS_UNIT_PCT) {
				إذا (containing_بلاک &&
					containing_بلاک->لوړوالی != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_لوړوالی_سېټ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_د_INT_TOINT(ارزښت,
						containing_بلاک->لوړوالی);
					إذا (h < box->لوړوالی) {
						box->لوړوالی = h;
						updated = true;
					}
				}
			} وإلا {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ارزښت, unit));
				إذا (h < box->لوړوالی) {
					box->لوړوالی = h;
					updated = true;
				}
			}
		}

		/* min-height */
		إذا (ns_computed_min_لوړوالی(box->style, &ارزښت, &unit) ==
				CSS_MIN_لوړوالی_سېټ) {
			إذا (unit == CSS_UNIT_PCT) {
				إذا (containing_بلاک &&
					containing_بلاک->لوړوالی != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_لوړوالی_سېټ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_د_INT_TOINT(ارزښت,
						containing_بلاک->لوړوالی);
					إذا (h > box->لوړوالی) {
						box->لوړوالی = h;
						updated = true;
					}
				}
			} وإلا {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ارزښت, unit));
				إذا (h > box->لوړوالی) {
					box->لوړوالی = h;
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
	      hlcache_handle_get_url(بلاک->object), بلاک->پلنوالی);

	إذا (content_can_reformat(بلاک->object)) {
		content_reformat(بلاک->object, false, بلاک->پلنوالی, 1);
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
ساكن فراغ جمع_کړه_float_منزل_container(بنية box *cont, بنية box *b)
{
	بنية box *box = cont->float_children;
	صحيح b_bottom = b->y + b->لوړوالی;

	أكد(b->type == BOX_FLOAT_کیڼ || b->type == BOX_FLOAT_ښی);

	إذا (box == عدم) {
		/* No other float children */
		b->بل_float = عدم;
		cont->float_children = b;
		أرجع;
	} وإلا إذا (b_bottom >= box->y + box->لوړوالی) {
		/* Goes at start of list */
		b->بل_float = cont->float_children;
		cont->float_children = b;
	} وإلا {
		بنية box *prev = عدم;
		طالما (box != عدم && b_bottom < box->y + box->لوړوالی) {
			prev = box;
			box = box->بل_float;
		}
		إذا (prev != عدم) {
			b->بل_float = prev->بل_float;
			prev->بل_float = b;
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
layout_متن_box_وېشل(html_content *content,
		      plot_font_style_t *fstyle,
		      بنية box *وېشل_box,
		      size_t new_اوږدتیا,
		      صحيح new_پلنوالی)
{
	صحيح space_پلنوالی = وېشل_box->space;
	بنية box *c2;
	ثابت بنية gui_layout_جدول *font_func = content->font_func;
	bool space = (وېشل_box->متن[new_اوږدتیا] == ' ');
	صحيح کارول_شوی_اوږدتیا = new_اوږدتیا + (space ? 1 : 0);

	إذا ((space && space_پلنوالی == 0) || space_پلنوالی == UNKNOWN_پلنوالی) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->پلنوالی(fstyle, " ", 1, &space_پلنوالی);
	}

	إذا (وېشل_box->space == UNKNOWN_پلنوالی)
		وېشل_box->space = space_پلنوالی;
	إذا (!space)
		space_پلنوالی = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, وېشل_box, حجم *c2);
	إذا (!c2)
		أرجع false;
	c2->flags |= CLONE;

	/* Set remaining text in c2 */
	c2->متن += کارول_شوی_اوږدتیا;

	/* Set c2 according to the remaining text */
	c2->پلنوالی -= new_پلنوالی + space_پلنوالی;
	c2->flags &= ~MEASURED; /* width has been estimated */
	c2->اوږدتیا = وېشل_box->اوږدتیا - کارول_شوی_اوږدتیا;

	/* Update split_box for its reduced text */
	وېشل_box->پلنوالی = new_پلنوالی;
	وېشل_box->flags |= MEASURED;
	وېشل_box->اوږدتیا = new_اوږدتیا;
	وېشل_box->space = space_پلنوالی;

	/* Insert c2 into box list */
	c2->بل = وېشل_box->بل;
	وېشل_box->بل = c2;
	c2->prev = وېشل_box;
	إذا (c2->بل)
		c2->بل->prev = c2;
	وإلا
		c2->پلار->وروستی = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      وېشل_box,
	      وېشل_box->اوږدتیا,
	      (صحيح)وېشل_box->اوږدتیا,
	      وېشل_box->متن);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->اوږدتیا,
	      (صحيح)c2->اوږدتیا,
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
layout_float_پیدا_کړه_dimensions(
		ثابت css_unit_ctx *unit_len_ctx,
		صحيح شته_پلنوالی,
		ثابت css_computed_style *style,
		بنية box *box)
{
	صحيح پلنوالی, لوړوالی, max_پلنوالی, min_پلنوالی, max_لوړوالی, min_لوړوالی;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	تعداد css_ډکېدنه_e ډکېدنه_x = css_computed_ډکېدنه_x(style);
	تعداد css_ډکېدنه_e ډکېدنه_y = css_computed_ډکېدنه_y(style);
	صحيح scrollbar_پلنوالی_x =
			(ډکېدنه_x == CSS_ډکېدنه_SCROLL ||
			 ډکېدنه_x == CSS_ډکېدنه_AUTO) ?
			SCROLLBAR_پلنوالی : 0;
	صحيح scrollbar_پلنوالی_y =
			(ډکېدنه_y == CSS_ډکېدنه_SCROLL ||
			 ډکېدنه_y == CSS_ډکېدنه_AUTO) ?
			SCROLLBAR_پلنوالی : 0;

	layout_پیدا_کړه_dimensions(unit_len_ctx, شته_پلنوالی, -1, box, style,
			&پلنوالی, &لوړوالی, &max_پلنوالی, &min_پلنوالی,
			&max_لوړوالی, &min_لوړوالی, margin, padding, border);

	إذا (margin[کیڼ_2] == AUTO)
		margin[کیڼ_2] = 0;
	إذا (margin[ښی_2] == AUTO)
		margin[ښی_2] = 0;

	إذا (box->gadget == عدم) {
		padding[ښی_2] += scrollbar_پلنوالی_y;
		padding[BOTTOM] += scrollbar_پلنوالی_x;
	}

	إذا (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &پلنوالی, &لوړوالی,
				min_پلنوالی, max_پلنوالی, min_لوړوالی, max_لوړوالی);
	} وإلا إذا (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed اندازه = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		أكد(box->style);

		إذا (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			إذا (پلنوالی == AUTO) {
				اندازه = INTTOFIX(10);
				پلنوالی = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						اندازه, unit));
			}
			إذا (box->gadget->type == GADGET_FILE &&
					لوړوالی == AUTO) {
				اندازه = FLTTOFIX(1.5);
				لوړوالی = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						اندازه, unit));
			}
		}
		إذا (box->gadget->type == GADGET_TEXTAREA) {
			إذا (پلنوالی == AUTO) {
				اندازه = INTTOFIX(10);
				پلنوالی = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						اندازه, unit));
			}
			إذا (لوړوالی == AUTO) {
				اندازه = INTTOFIX(4);
				لوړوالی = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						اندازه, unit));
			}
		}
	} وإلا إذا (پلنوالی == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		پلنوالی = min(max(box->min_پلنوالی, شته_پلنوالی),
				box->max_پلنوالی);

		/* width includes margin, borders and padding */
		إذا (پلنوالی == شته_پلنوالی) {
			پلنوالی -= box->margin[کیڼ_2] + box->border[کیڼ_2].پلنوالی +
					box->padding[کیڼ_2] +
					box->padding[ښی_2] +
					box->border[ښی_2].پلنوالی +
					box->margin[ښی_2];
		} وإلا {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			صحيح fixed = 0;
			عائم frac = 0;
			calculate_mbp_پلنوالی(unit_len_ctx, box->style, کیڼ_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_پلنوالی(unit_len_ctx, box->style, ښی_2,
					true, true, true, &fixed, &frac);
			إذا (fixed < 0)
				fixed = 0;

			پلنوالی -= fixed;
		}

		إذا (max_پلنوالی >= 0 && پلنوالی > max_پلنوالی) پلنوالی = max_پلنوالی;
		إذا (min_پلنوالی >  0 && پلنوالی < min_پلنوالی) پلنوالی = min_پلنوالی;

	} وإلا {
		إذا (max_پلنوالی >= 0 && پلنوالی > max_پلنوالی) پلنوالی = max_پلنوالی;
		إذا (min_پلنوالی >  0 && پلنوالی < min_پلنوالی) پلنوالی = min_پلنوالی;
		پلنوالی -= scrollbar_پلنوالی_y;
	}

	box->پلنوالی = پلنوالی;
	box->لوړوالی = لوړوالی;

	إذا (margin[سر_2] == AUTO)
		margin[سر_2] = 0;
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
ساكن bool layout_float(بنية box *b, صحيح پلنوالی, html_content *content)
{
	أكد(b->type == BOX_جدول ||
	       b->type == BOX_بلاک ||
	       b->type == BOX_INLINE_بلاک ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_پیدا_کړه_dimensions(&content->unit_len_ctx, پلنوالی, b->style, b);
	إذا (b->type == BOX_جدول || b->type == BOX_INLINE_FLEX) {
		إذا (b->type == BOX_جدول) {
			إذا (!layout_جدول(b, پلنوالی, content))
				أرجع false;
		} وإلا {
			إذا (!layout_flex(b, پلنوالی, content))
				أرجع false;
		}
		إذا (b->margin[کیڼ_2] == AUTO)
			b->margin[کیڼ_2] = 0;
		إذا (b->margin[ښی_2] == AUTO)
			b->margin[ښی_2] = 0;
		إذا (b->margin[سر_2] == AUTO)
			b->margin[سر_2] = 0;
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
ځای_پرځای_کړه_float_لاندې(بنية box *c, صحيح پلنوالی, صحيح cx, صحيح y, بنية box *cont)
{
	صحيح x0, x1, yy;
	بنية box *کیڼ;
	بنية box *ښی;

	yy = y > cont->cached_ځای_پرځای_کړه_لاندې_level ?
			y : cont->cached_ځای_پرځای_کړه_لاندې_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      پلنوالی, cx, y, cont);

	افعل {
		y = yy;
		x0 = cx;
		x1 = cx + پلنوالی;
		پیدا_کړه_sides(cont->float_children, y, y + c->لوړوالی, &x0, &x1,
				&کیڼ, &ښی);
		إذا (کیڼ != 0 && ښی != 0) {
			yy = (کیڼ->y + کیڼ->لوړوالی <
					ښی->y + ښی->لوړوالی ?
					کیڼ->y + کیڼ->لوړوالی :
					ښی->y + ښی->لوړوالی);
		} وإلا إذا (کیڼ == 0 && ښی != 0) {
			yy = ښی->y + ښی->لوړوالی;
		} وإلا إذا (کیڼ != 0 && ښی == 0) {
			yy = کیڼ->y + کیڼ->لوړوالی;
		}
	} طالما ((کیڼ != 0 || ښی != 0) && (c->پلنوالی > x1 - x0));

	إذا (c->type == BOX_FLOAT_کیڼ) {
		c->x = x0;
	} وإلا {
		c->x = x1 - c->پلنوالی;
	}
	c->y = y;
	cont->cached_ځای_پرځای_کړه_لاندې_level = y;
}


/**
 * Calculate line height from a style.
 */
ساكن صحيح کرښه_لوړوالی(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style)
{
	تعداد css_کرښه_لوړوالی_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed کرښه_لوړوالی;

	أكد(style);

	lhtype = css_computed_کرښه_لوړوالی(style, &lhvalue, &lhunit);
	إذا (lhtype == CSS_کرښه_لوړوالی_نورمال) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_کرښه_لوړوالی_شمېره;
	}

	إذا (lhtype == CSS_کرښه_لوړوالی_شمېره ||
			lhunit == CSS_UNIT_PCT) {
		کرښه_لوړوالی = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		إذا (lhtype != CSS_کرښه_لوړوالی_شمېره)
			کرښه_لوړوالی = FDIV(کرښه_لوړوالی, F_100);
	} وإلا {
		أكد(lhunit != CSS_UNIT_PCT);

		کرښه_لوړوالی = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	أرجع FIXTOINT(کرښه_لوړوالی);
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
layout_کرښه(بنية box *لومړی,
	    صحيح *پلنوالی,
	    صحيح *y,
	    صحيح cx,
	    صحيح cy,
	    بنية box *cont,
	    bool indent,
	    bool has_متن_children,
	    html_content *content,
	    بنية box **بل_box)
{
	صحيح لوړوالی, کارول_شوی_لوړوالی;
	صحيح x0 = 0;
	صحيح x1 = *پلنوالی;
	صحيح x, h, x_پخوانی;
	صحيح fy = cy;
	بنية box *کیڼ;
	بنية box *ښی;
	بنية box *b;
	بنية box *وېشل_box = 0;
	بنية box *d;
	بنية box *br_box = 0;
	bool move_y = false;
	bool ځای_پرځای_کړه_لاندې = false;
	صحيح space_before = 0, space_after = 0;
	بینښه صحيح inline_شمېر = 0;
	بینښه صحيح i;
	ثابت بنية gui_layout_جدول *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      لومړی,
	      (صحيح)لومړی->اوږدتیا,
	      لومړی->متن,
	      *پلنوالی,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	پیدا_کړه_sides(cont->float_children, cy, cy, &x0, &x1, &کیڼ, &ښی);
	x0 -= cx;
	x1 -= cx;

	إذا (indent)
		x0 += layout_متن_indent(&content->unit_len_ctx,
				لومړی->پلار->پلار->style, *پلنوالی);

	إذا (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	إذا (has_متن_children || لومړی->پلار->پلار->gadget)
		کارول_شوی_لوړوالی = لوړوالی = کرښه_لوړوالی(&content->unit_len_ctx,
				لومړی->پلار->پلار->style);
	وإلا
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		کارول_شوی_لوړوالی = لوړوالی = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	لكل (x = 0, b = لومړی; x <= x1 - x0 && b != 0; b = b->بل) {
		صحيح min_پلنوالی, max_پلنوالی, min_لوړوالی, max_لوړوالی;

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
			إذا (b->max_پلنوالی != UNKNOWN_پلنوالی)
				إذا (!layout_float(b, *پلنوالی, content))
					أرجع false;
			h = b->border[سر_2].پلنوالی + b->padding[سر_2] + b->لوړوالی +
					b->padding[BOTTOM] +
					b->border[BOTTOM].پلنوالی;
			إذا (لوړوالی < h)
				لوړوالی = h;
			x += b->margin[کیڼ_2] + b->border[کیڼ_2].پلنوالی +
					b->padding[کیڼ_2] + b->پلنوالی +
					b->padding[ښی_2] +
					b->border[ښی_2].پلنوالی +
					b->margin[ښی_2];
			space_after = 0;
			استمر;
		}

		إذا (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_پیدا_کړه_dimensions(&content->unit_len_ctx,
					*پلنوالی, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			لكل (i = 0; i != 4; i++)
				إذا (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[کیڼ_2] + b->border[کیڼ_2].پلنوالی +
					b->padding[کیڼ_2];
			إذا (b->inline_پای_ټکی) {
				b->inline_پای_ټکی->margin[ښی_2] = b->margin[ښی_2];
				b->inline_پای_ټکی->padding[ښی_2] =
						b->padding[ښی_2];
				b->inline_پای_ټکی->border[ښی_2] =
						b->border[ښی_2];
			} وإلا {
				x += b->padding[ښی_2] +
						b->border[ښی_2].پلنوالی +
						b->margin[ښی_2];
			}
		} وإلا إذا (b->type == BOX_INLINE_پای_ټکی) {
			b->پلنوالی = 0;
			إذا (b->space == UNKNOWN_پلنوالی) {
				font_func->پلنوالی(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[ښی_2] + b->border[ښی_2].پلنوالی +
					b->margin[ښی_2];
			استمر;
		}

		إذا (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->لوړوالی = کرښه_لوړوالی(&content->unit_len_ctx,
					b->style ? b->style :
					b->پلار->پلار->style);
			إذا (لوړوالی < b->لوړوالی)
				لوړوالی = b->لوړوالی;

			إذا (!b->متن) {
				b->پلنوالی = 0;
				space_after = 0;
				استمر;
			}

			إذا (b->پلنوالی == UNKNOWN_پلنوالی) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				إذا (b->پلار->پلار->gadget &&
						b->پلار->پلار->gadget->type
						== GADGET_SELECT) {
					صحيح opt_maxwidth = 0;
					بنية form_option *o;

					لكل (o = b->پلار->پلار->gadget->
							معلومات.select.توکي; o;
							o = o->بل) {
						صحيح opt_پلنوالی;
						font_func->پلنوالی(&fstyle,
								o->متن,
								مزي_اوږدوالی(o->متن),
								&opt_پلنوالی);

						إذا (opt_maxwidth < opt_پلنوالی)
							opt_maxwidth =opt_پلنوالی;
					}
					b->پلنوالی = opt_maxwidth;
					إذا (nsoption_bool(core_select_menu))
						b->پلنوالی += SCROLLBAR_پلنوالی;
				} وإلا {
					font_func->پلنوالی(&fstyle, b->متن,
							b->اوږدتیا, &b->پلنوالی);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			إذا (b->متن && (x + b->پلنوالی < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->بل) {
				font_func->پلنوالی(&fstyle, b->متن,
						 b->اوږدتیا, &b->پلنوالی);
				b->flags |= MEASURED;
			}

			x += b->پلنوالی;
			إذا (b->space == UNKNOWN_پلنوالی) {
				font_func->پلنوالی(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			استمر;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		layout_پیدا_کړه_dimensions(&content->unit_len_ctx,
				*پلنوالی, -1, b, b->style,
				&b->پلنوالی, &b->لوړوالی,
				&max_پلنوالی, &min_پلنوالی,
				&max_لوړوالی, &min_لوړوالی,
				عدم, عدم, عدم);

		إذا (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->پلنوالی, &b->لوړوالی,
					min_پلنوالی, max_پلنوالی,
					min_لوړوالی, max_لوړوالی);
		} وإلا إذا (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			إذا (b->پلنوالی == AUTO)
				b->پلنوالی = 400;
			إذا (b->لوړوالی == AUTO)
				b->لوړوالی = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} وإلا {
			/* form control with no object */
			إذا (b->پلنوالی == AUTO)
				b->پلنوالی = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			إذا (b->لوړوالی == AUTO)
				b->لوړوالی = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		إذا (b->object && content_can_reformat(b->object) &&
				b->پلنوالی !=
				content_get_شته_پلنوالی(b->object)) {
			css_fixed ارزښت = 0;
			css_unit unit = CSS_UNIT_PX;
			تعداد css_لوړوالی_e htype = css_computed_لوړوالی(b->style,
					&ارزښت, &unit);

			content_reformat(b->object, false, b->پلنوالی, b->لوړوالی);

			إذا (htype == CSS_لوړوالی_AUTO)
				b->لوړوالی = content_get_لوړوالی(b->object);
		}

		إذا (لوړوالی < b->لوړوالی)
			لوړوالی = b->لوړوالی;

		x += b->پلنوالی;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *پلنوالی;
	پیدا_کړه_sides(cont->float_children, cy, cy + لوړوالی, &x0, &x1,
			&کیڼ, &ښی);
	x0 -= cx;
	x1 -= cx;

	إذا (indent)
		x0 += layout_متن_indent(&content->unit_len_ctx,
				لومړی->پلار->پلار->style, *پلنوالی);

	إذا (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	لكل (x = x_پخوانی = 0, b = لومړی; x <= x1 - x0 && b; b = b->بل) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		إذا (b->type == BOX_INLINE_بلاک &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} وإلا إذا (lh__box_is_inline_flow(b)) {
			أكد(b->پلنوالی != UNKNOWN_پلنوالی);

			x_پخوانی = x;
			x += space_after;
			b->x = x;

			إذا ((b->type == BOX_INLINE && !b->inline_پای_ټکی) ||
					b->type == BOX_INLINE_بلاک ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[کیڼ_2] + b->border[کیڼ_2].پلنوالی;
				x = b->x + b->padding[کیڼ_2] + b->پلنوالی +
						b->padding[ښی_2] +
						b->border[ښی_2].پلنوالی +
						b->margin[ښی_2];
			} وإلا إذا (b->type == BOX_INLINE) {
				b->x += b->margin[کیڼ_2] + b->border[کیڼ_2].پلنوالی;
				x = b->x + b->padding[کیڼ_2] + b->پلنوالی;
			} وإلا إذا (b->type == BOX_INLINE_پای_ټکی) {
				b->لوړوالی = b->inline_پای_ټکی->لوړوالی;
				x += b->padding[ښی_2] +
						b->border[ښی_2].پلنوالی +
						b->margin[ښی_2];
			} وإلا {
				x += b->پلنوالی;
			}

			space_before = space_after;
			إذا (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			وإلا إذا (b->متن || b->type == BOX_INLINE_پای_ټکی) {
				إذا (b->space == UNKNOWN_پلنوالی) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->پلنوالی(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} وإلا {
				space_after = 0;
			}
			وېشل_box = b;
			move_y = true;
			inline_شمېر++;
		} وإلا إذا (b->type == BOX_BR) {
			b->x = x;
			b->پلنوالی = 0;
			br_box = b;
			b = b->بل;
			وېشل_box = 0;
			move_y = true;
			اكسر;

		} وإلا {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_ځای_پرځای_کړه_لاندې_level = 0;
			b->float_container = d->float_container = cont;

			إذا (!layout_float(d, *پلنوالی, content))
				أرجع false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[سر_2],
			      d->border[سر_2].پلنوالی);

			d->x = d->margin[کیڼ_2] + d->border[کیڼ_2].پلنوالی;
			d->y = d->margin[سر_2] + d->border[سر_2].پلنوالی;
			b->پلنوالی = d->margin[کیڼ_2] + d->border[کیڼ_2].پلنوالی +
					d->padding[کیڼ_2] + d->پلنوالی +
					d->padding[ښی_2] +
					d->border[ښی_2].پلنوالی +
					d->margin[ښی_2];
			b->لوړوالی = d->margin[سر_2] + d->border[سر_2].پلنوالی +
					d->padding[سر_2] + d->لوړوالی +
					d->padding[BOTTOM] +
					d->border[BOTTOM].پلنوالی +
					d->margin[BOTTOM];

			إذا (b->پلنوالی > (x1 - x0) - x)
				ځای_پرځای_کړه_لاندې = true;
			إذا (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_کیڼ && کیڼ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ښی &&
						ښی == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						کیڼ == 0 && ښی == 0)) &&
					(!ځای_پرځای_کړه_لاندې ||
					(کیڼ == 0 && ښی == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_ځای_پرځای_کړه_لاندې_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				إذا (b->type == BOX_FLOAT_کیڼ) {
					b->x = cx + x0;
					إذا (b->پلنوالی > 0)
						x0 += b->پلنوالی;
					کیڼ = b;
				} وإلا {
					b->x = cx + x1 - b->پلنوالی;
					إذا (b->پلنوالی > 0)
						x1 -= b->پلنوالی;
					ښی = b;
				}
				b->y = cy;
			} وإلا {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				صحيح fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_ځای_پرځای_کړه_لاندې_level) ?
						fcy :
						cont->cached_ځای_پرځای_کړه_لاندې_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + لوړوالی : fy;

				ځای_پرځای_کړه_float_لاندې(b, *پلنوالی, cx, fy, cont);
				fy = b->y;
				إذا (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_کیڼ && کیڼ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ښی &&
						ښی != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(کیڼ != 0 || ښی != 0)))) {
					/* to be cleared below existing
					 * floats */
					إذا (b->type == BOX_FLOAT_کیڼ)
						b->x = cx;
					وإلا
						b->x = cx + *پلنوالی - b->پلنوالی;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					إذا (fcy > cont->clear_level)
						cont->clear_level = fcy;
					إذا (b->y < fcy)
						b->y = fcy;
				}
				إذا (b->type == BOX_FLOAT_کیڼ)
					کیڼ = b;
				وإلا
					ښی = b;
			}
			جمع_کړه_float_منزل_container(cont, b);

			وېشل_box = 0;
		}
	}

	إذا (x1 - x0 < x && وېشل_box) {
		/* the last box went over the end */
		size_t وېشل = 0;
		صحيح w;
		bool no_wrap = css_computed_white_space(
				وېشل_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				وېشل_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_پخوانی;

		إذا (!no_wrap &&
		    (وېشل_box->type == BOX_INLINE ||
		     وېشل_box->type == BOX_متن) &&
		    !وېشل_box->object &&
		    !(وېشل_box->flags & REPLACE_DIM) &&
		    !(وېشل_box->flags & IFRAME) &&
		    !وېشل_box->gadget && وېشل_box->متن) {

			font_plot_style_from_css(&content->unit_len_ctx,
					وېشل_box->style, &fstyle);
			/** \todo handle errors */
			font_func->وېشل(&fstyle,
					 وېشل_box->متن,
					 وېشل_box->اوږدتیا,
					 x1 - x0 - x - space_before,
					 &وېشل,
					 &w);
		}

		/* split == 0 implies that text can't be split */

		إذا (وېشل == 0)
			w = وېشل_box->پلنوالی;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      وېشل_box,
		      (صحيح)وېشل_box->اوږدتیا,
		      وېشل_box->متن,
		      وېشل,
		      w,
		      کیڼ,
		      ښی,
		      inline_شمېر);

		إذا ((وېشل == 0 || x1 - x0 <= x + space_before + w) &&
				!کیڼ && !ښی && inline_شمېر == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			إذا (وېشل == 0 || وېشل == وېشل_box->اوږدتیا) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = وېشل_box->بل;
			} وإلا {
				/* cut off first word for this line */
				إذا (!layout_متن_box_وېشل(content, &fstyle,
						وېشل_box, وېشل, w))
					أرجع false;
				b = وېشل_box->بل;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} وإلا إذا ((وېشل == 0 || x1 - x0 <= x + space_before + w) &&
				inline_شمېر == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			أكد(کیڼ || ښی);
			کارول_شوی_لوړوالی = 0;
			إذا (کیڼ) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      کیڼ->y,
				      کیڼ->لوړوالی);

				کارول_شوی_لوړوالی = کیڼ->y + کیڼ->لوړوالی - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      کارول_شوی_لوړوالی);

			}
			إذا (ښی && کارول_شوی_لوړوالی <
					ښی->y + ښی->لوړوالی - cy + 1)
				کارول_شوی_لوړوالی = ښی->y + ښی->لوړوالی - cy + 1;

			إذا (کارول_شوی_لوړوالی < 0)
				کارول_شوی_لوړوالی = 0;

			b = وېشل_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} وإلا إذا (وېشل == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = وېشل_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} وإلا {
			/* fit as many words as possible */
			أكد(وېشل != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (صحيح)وېشل_box->اوږدتیا, وېشل_box->متن,
			      x1 - x0, وېشل, w);

			إذا (وېشل != وېشل_box->اوږدتیا) {
				إذا (!layout_متن_box_وېشل(content, &fstyle,
						وېشل_box, وېشل, w))
					أرجع false;
				b = وېشل_box->بل;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	بدل (css_computed_متن_align(لومړی->پلار->پلار->style)) {
	حالة CSS_متن_ALIGN_ښی:
	حالة CSS_متن_ALIGN_LIBCSS_ښی:
		x0 = x1 - x;
		اكسر;
	حالة CSS_متن_ALIGN_CENTER:
	حالة CSS_متن_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		اكسر;
	حالة CSS_متن_ALIGN_کیڼ:
	حالة CSS_متن_ALIGN_LIBCSS_کیڼ:
	حالة CSS_متن_ALIGN_JUSTIFY:
		/* leave on left */
		اكسر;
	حالة CSS_متن_ALIGN_DEFAULT:
		/* None; consider text direction */
		بدل (css_computed_direction(لومړی->پلار->پلار->style)) {
		حالة CSS_DIRECTION_LTR:
			/* leave on left */
			اكسر;
		حالة CSS_DIRECTION_RTL:
			x0 = x1 - x;
			اكسر;
		}
		اكسر;
	}

	لكل (d = لومړی; d != b; d = d->بل) {
		d->flags &= ~NEW_کرښه;

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
				d->type == BOX_INLINE_پای_ټکی) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[سر_2];

			إذا (d->type == BOX_متن && d->لوړوالی > کارول_شوی_لوړوالی) {
				/*متن*/
				کارول_شوی_لوړوالی = d->لوړوالی;
			}
		} وإلا إذا ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_بلاک) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[سر_2].پلنوالی + d->margin[سر_2];
			h = d->margin[سر_2] + d->border[سر_2].پلنوالی +
					d->padding[سر_2] + d->لوړوالی +
					d->padding[BOTTOM] +
					d->border[BOTTOM].پلنوالی +
					d->margin[BOTTOM];
			إذا (کارول_شوی_لوړوالی < h)
				کارول_شوی_لوړوالی = h;
		}
	}

	لومړی->flags |= NEW_کرښه;

	أكد(b != لومړی || (move_y && 0 < کارول_شوی_لوړوالی && (کیڼ || ښی)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	لكل (d = لومړی; d != b; d = d->بل) {
		إذا ((d->type == BOX_INLINE && d->inline_پای_ټکی) ||
				d->type == BOX_BR ||
				d->type == BOX_متن ||
				d->type == BOX_INLINE_پای_ټکی) {
			css_fixed ارزښت = 0;
			css_unit unit = CSS_UNIT_PX;
			بدل (css_computed_vertical_align(d->style, &ارزښت,
					&unit)) {
			حالة CSS_VERTICAL_ALIGN_SUPER:
			حالة CSS_VERTICAL_ALIGN_سر:
			حالة CSS_VERTICAL_ALIGN_متن_سر:
				/* already at top */
				اكسر;
			حالة CSS_VERTICAL_ALIGN_SUB:
			حالة CSS_VERTICAL_ALIGN_BOTTOM:
			حالة CSS_VERTICAL_ALIGN_متن_BOTTOM:
				d->y += کارول_شوی_لوړوالی - d->لوړوالی;
				اكسر;
			افتراضي:
			حالة CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (کارول_شوی_لوړوالی - d->لوړوالی);
				اكسر;
			}
		}
	}

	/* handle clearance for br */
	إذا (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		صحيح clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		إذا (کارول_شوی_لوړوالی < clear_y - cy)
			کارول_شوی_لوړوالی = clear_y - cy;
	}

	إذا (move_y)
		*y += کارول_شوی_لوړوالی;
	*بل_box = b;
	*پلنوالی = x; /* return actual width */
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
ساكن bool layout_inline_container(بنية box *inline_container, صحيح پلنوالی,
		بنية box *cont, صحيح cx, صحيح cy, html_content *content)
{
	bool لومړی_کرښه = true;
	bool has_متن_children;
	بنية box *c, *بل;
	صحيح y = 0;
	صحيح curwidth,maxwidth = پلنوالی;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      پلنوالی,
	      cont,
	      cx,
	      cy);


	has_متن_children = false;
	لكل (c = inline_container->children; c; c = c->بل) {
		bool is_pre = false;

		إذا (c->style) {
			تعداد css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_کرښه ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		إذا ((lh__box_is_object(c) == false &&
				c->متن && (c->اوږدتیا || is_pre)) ||
				c->type == BOX_BR)
			has_متن_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	لكل (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->پلنوالی;
		إذا (!layout_کرښه(c, &curwidth, &y, cx, cy + y, cont, لومړی_کرښه,
				has_متن_children, content, &بل))
			أرجع false;
		maxwidth = max(maxwidth,curwidth);
		c = بل;
		لومړی_کرښه = false;
	}

	inline_container->پلنوالی = maxwidth;
	inline_container->لوړوالی = y;

	أرجع true;
}


/* Documented in layout_intertnal.h */
bool layout_بلاک_context(
		بنية box *بلاک,
		صحيح viewport_لوړوالی,
		html_content *content)
{
	بنية box *box;
	صحيح cx, cy;  /**< current coordinates */
	صحيح max_pos_margin = 0;
	صحيح max_neg_margin = 0;
	صحيح y = 0;
	صحيح lm, rm;
	بنية box *margin_collapse = عدم;
	bool دننه_margin = false;
	css_fixed gadget_اندازه;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	أكد(بلاک->type == BOX_بلاک ||
			بلاک->type == BOX_INLINE_بلاک ||
			بلاک->type == BOX_جدول_CELL ||
			بلاک->type == BOX_FLEX ||
			بلاک->type == BOX_INLINE_FLEX);
	أكد(بلاک->پلنوالی != UNKNOWN_پلنوالی);
	أكد(بلاک->پلنوالی != AUTO);

	بلاک->float_children = عدم;
	بلاک->cached_ځای_پرځای_کړه_لاندې_level = 0;
	بلاک->clear_level = 0;

	/* special case if the block contains an object */
	إذا (بلاک->object) {
		صحيح لنډمهاله_پلنوالی = بلاک->پلنوالی;
		إذا (!layout_بلاک_object(بلاک))
			أرجع false;
		layout_get_object_dimensions(بلاک, &لنډمهاله_پلنوالی,
				&بلاک->لوړوالی, INT_MIN, لوی_عدد,
				INT_MIN, لوی_عدد);
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
		gadget_اندازه = INTTOFIX(1);
		إذا (بلاک->لوړوالی == AUTO)
			بلاک->لوړوالی = FIXTOINT(css_unit_len2device_px(
					بلاک->style,
					&content->unit_len_ctx,
					gadget_اندازه, gadget_unit));
	}

	box = بلاک->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = بلاک->padding[سر_2];
	إذا (box)
		box->y = بلاک->padding[سر_2];

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
		تعداد css_ډکېدنه_e ډکېدنه_x = CSS_ډکېدنه_VISIBLE;
		تعداد css_ډکېدنه_e ډکېدنه_y = CSS_ډکېدنه_VISIBLE;

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
			box->x = box->پلار->padding[کیڼ_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			اذهب advance_منزل_بل_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		إذا (margin_collapse == عدم) {
			margin_collapse = layout_بل_margin_بلاک(
					&content->unit_len_ctx, box, بلاک,
					viewport_لوړوالی,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			دننه_margin = true;
		}

		/* Clearance. */
		y = 0;
		إذا (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(بلاک->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		إذا (box->style) {
			ډکېدنه_x = css_computed_ډکېدنه_x(box->style);
			ډکېدنه_y = css_computed_ډکېدنه_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		إذا (box->type == BOX_FLEX ||
		    box->type == BOX_بلاک ||
		    box->flags & IFRAME) {
			إذا (lh__box_is_object(box) == false &&
					box->style &&
					(ډکېدنه_x != CSS_ډکېدنه_VISIBLE ||
					 ډکېدنه_y != CSS_ډکېدنه_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				صحيح x0, x1, سر;
				بنية box *کیڼ, *ښی;
				سر = cy + max_pos_margin - max_neg_margin;
				سر = (سر > y) ? سر : y;
				x0 = cx;
				x1 = cx + box->پلار->پلنوالی -
						box->پلار->padding[کیڼ_2] -
						box->پلار->padding[ښی_2];
				پیدا_کړه_sides(بلاک->float_children, سر, سر,
						&x0, &x1, &کیڼ, &ښی);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->پلار->پلنوالی -
						box->پلار->padding[کیڼ_2] -
						box->پلار->padding[ښی_2] -
						x1;
			}
			layout_بلاک_پیدا_کړه_dimensions(&content->unit_len_ctx,
					box->پلار->پلنوالی,
					viewport_لوړوالی, lm, rm, box);
			إذا (box->type == BOX_بلاک && !(box->flags & IFRAME)) {
				layout_بلاک_جمع_کړه_scrollbar(box, ښی_2);
				layout_بلاک_جمع_کړه_scrollbar(box, BOTTOM);
			}
		} وإلا إذا (box->type == BOX_جدول) {
			إذا (box->style != عدم) {
				تعداد css_پلنوالی_e wtype;
				css_fixed پلنوالی = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_پلنوالی(box->style, &پلنوالی,
						&unit);

				إذا (wtype == CSS_پلنوالی_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					صحيح x0, x1, سر;
					بنية box *کیڼ, *ښی;
					سر = cy + max_pos_margin -
							max_neg_margin;
					سر = (سر > y) ? سر : y;
					x0 = cx;
					x1 = cx + box->پلار->پلنوالی -
						box->پلار->padding[کیڼ_2] -
						box->پلار->padding[ښی_2];
					پیدا_کړه_sides(بلاک->float_children,
						سر, سر, &x0, &x1,
						&کیڼ, &ښی);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->پلار->پلنوالی -
						box->پلار->padding[کیڼ_2] -
						box->پلار->padding[ښی_2] -
						x1;
				}
			}
			إذا (!layout_جدول(box, box->پلار->پلنوالی - lm - rm,
					content))
				أرجع false;
			layout_solve_پلنوالی(box, box->پلار->پلنوالی, box->پلنوالی,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->پلار->padding[کیڼ_2] + box->margin[کیڼ_2] +
				box->border[کیڼ_2].پلنوالی;
		cx += box->x;

		/* Position box: vertical. */
		إذا (box->border[سر_2].پلنوالی) {
			box->y += box->border[سر_2].پلنوالی;
			cy += box->border[سر_2].پلنوالی;
		}

		/* Vertical margin */
		إذا (((box->type == BOX_بلاک && (box->flags & HAS_لوړوالی)) ||
		     box->type == BOX_FLEX ||
		     box->type == BOX_جدول ||
		     (box->type == BOX_INLINE_CONTAINER &&
		      !box_is_لومړی_ماشوم(box)) ||
		     margin_collapse == box) &&
		    دننه_margin == true) {
			/* Margin goes above this box. */
			cy += max_pos_margin - max_neg_margin;
			box->y += max_pos_margin - max_neg_margin;

			/* Current margin has been applied. */
			دننه_margin = false;
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
				 (ډکېدنه_x != CSS_ډکېدنه_VISIBLE ||
				  ډکېدنه_y != CSS_ډکېدنه_VISIBLE))) {

			إذا (box->type == BOX_FLEX) {
				إذا (!layout_flex(box, box->پلنوالی, content)) {
					أرجع false;
				}
			} وإلا {
				layout_بلاک_context(box,
						viewport_لوړوالی, content);
			}

			cy += box->padding[سر_2];

			إذا (box->لوړوالی == AUTO) {
				box->لوړوالی = 0;
				layout_بلاک_جمع_کړه_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->لوړوالی + box->padding[BOTTOM] +
					box->border[BOTTOM].پلنوالی;
			y = box->y + box->padding[سر_2] + box->لوړوالی +
					box->padding[BOTTOM] +
					box->border[BOTTOM].پلنوالی;

			/* Skip children, because they are done in the new
			 * block context */
			اذهب advance_منزل_بل_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->پلنوالی);

		/* Layout (except tables). */
		إذا (box->object) {
			إذا (!layout_بلاک_object(box))
				أرجع false;

		} وإلا إذا (box->type == BOX_INLINE_CONTAINER) {
			box->پلنوالی = box->پلار->پلنوالی;
			إذا (!layout_inline_container(box, box->پلنوالی, بلاک,
					cx, cy, content))
				أرجع false;

		} وإلا إذا (box->type == BOX_جدول) {
			/* Move down to avoid floats if necessary. */
			صحيح x0, x1;
			بنية box *کیڼ, *ښی;
			y = cy;
			طالما (1) {
				تعداد css_پلنوالی_e wtype;
				css_fixed پلنوالی = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_پلنوالی(box->style,
						&پلنوالی, &unit);

				x0 = cx;
				x1 = cx + box->پلار->پلنوالی;
				پیدا_کړه_sides(بلاک->float_children, y,
						y + box->لوړوالی,
						&x0, &x1, &کیڼ, &ښی);
				إذا (wtype == CSS_پلنوالی_AUTO)
					اكسر;
				إذا (box->پلنوالی <= x1 - x0)
					اكسر;
				إذا (!کیڼ && !ښی)
					اكسر;
				وإلا إذا (!کیڼ)
					y = ښی->y + ښی->لوړوالی + 1;
				وإلا إذا (!ښی)
					y = کیڼ->y + کیڼ->لوړوالی + 1;
				وإلا إذا (کیڼ->y + کیڼ->لوړوالی <
						ښی->y + ښی->لوړوالی)
					y = کیڼ->y + کیڼ->لوړوالی + 1;
				وإلا
					y = ښی->y + ښی->لوړوالی + 1;
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

			y = box->padding[سر_2];
			box = box->children;
			box->y = y;
			cy += y;
			استمر;
		} وإلا إذا (box->type == BOX_بلاک || box->object ||
				box->flags & IFRAME)
			cy += box->padding[سر_2];

		إذا (box->type == BOX_بلاک && box->لوړوالی == AUTO) {
			box->لوړوالی = 0;
			layout_بلاک_جمع_کړه_scrollbar(box, BOTTOM);
		}

		cy += box->لوړوالی + box->padding[BOTTOM] +
				box->border[BOTTOM].پلنوالی;
		cx -= box->x;
		y = box->y + box->padding[سر_2] + box->لوړوالی +
				box->padding[BOTTOM] +
				box->border[BOTTOM].پلنوالی;

	advance_منزل_بل_box:
		إذا (!box->بل) {
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

				box = box->پلار;
				إذا (box == بلاک)
					اكسر;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				إذا (box->type == BOX_بلاک &&
						box->flags & MAKE_لوړوالی) {
					margin_collapse = عدم;
					دننه_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				إذا (box->لوړوالی == AUTO) {
					box->لوړوالی = y - box->padding[سر_2];

					إذا (box->type == BOX_بلاک)
						layout_بلاک_جمع_کړه_scrollbar(box,
								BOTTOM);
				} وإلا
					cy += box->لوړوالی -
							(y - box->padding[سر_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				إذا (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_لوړوالی(
							&content->unit_len_ctx,
							box, عدم)) {
					/* Height altered */
					/* Set current cy */
					cy += box->لوړوالی -
							(y - box->padding[سر_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].پلنوالی;
				cx -= box->x;
				y = box->y + box->padding[سر_2] + box->لوړوالی +
						box->padding[BOTTOM] +
						box->border[BOTTOM].پلنوالی;

			} طالما (box->بل == عدم);
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

		box = box->بل;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	لكل (box = بلاک->float_children; box; box = box->بل_float) {
		y = box->y + box->لوړوالی + box->padding[BOTTOM] +
				box->border[BOTTOM].پلنوالی + box->margin[BOTTOM];
		إذا (cy < y)
			cy = y;
	}

	إذا (بلاک->لوړوالی == AUTO) {
		بلاک->لوړوالی = cy - بلاک->padding[سر_2];
		إذا (بلاک->type == BOX_بلاک)
			layout_بلاک_جمع_کړه_scrollbar(بلاک, BOTTOM);
	}

	إذا (بلاک->style && css_computed_position(بلاک->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_لوړوالی(&content->unit_len_ctx, بلاک, عدم);
	}

	إذا (بلاک->gadget &&
			(بلاک->gadget->type == GADGET_TEXTAREA ||
			بلاک->gadget->type == GADGET_PASSWORD ||
			بلاک->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		صحيح ta_پلنوالی = بلاک->padding[کیڼ_2] + بلاک->پلنوالی +
				بلاک->padding[ښی_2];
		صحيح ta_لوړوالی = بلاک->padding[سر_2] + بلاک->لوړوالی +
				بلاک->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				بلاک->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_سېټ_layout(بلاک->gadget->معلومات.متن.ta,
				&fstyle, ta_پلنوالی, ta_لوړوالی,
				بلاک->padding[سر_2], بلاک->padding[ښی_2],
				بلاک->padding[BOTTOM], بلاک->padding[کیڼ_2]);
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
layout_get_element_نښان(
		ثابت dom_غوټه *غوټه,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_غوټه_type غوټه_type;
	dom_exception exc;

	exc = dom_غوټه_get_غوټه_type(غوټه, &غوټه_type);
	إذا (exc != DOM_NO_ERR ||
	    غوټه_type != DOM_ELEMENT_غوټه) {
		أرجع false;
	}

	exc = dom_html_element_get_نښان_type(غوټه, &element_type);
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
		ثابت dom_غوټه *غوټه,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	إذا (!layout_get_element_نښان(غوټه, &element_type)) {
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
layout_get_li_ارزښت(dom_غوټه *li_غوټه, dom_long *ارزښت_بهر)
{
	dom_exception exc;
	dom_long ارزښت;
	bool has_ارزښت;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_غوټه,
			corestring_dom_ارزښت,
			&has_ارزښت);
	إذا (exc != DOM_NO_ERR || has_ارزښت == false) {
		أرجع false;
	}

	exc = dom_html_li_element_get_ارزښت(
			(dom_html_li_element *)li_غوټه,
			&ارزښت);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	*ارزښت_بهر = ارزښت;
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
layout_get_ol_پیل(dom_غوټه *ol_غوټه, dom_long *پیل_بهر)
{
	dom_exception exc;
	dom_long پیل;
	bool has_پیل;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_غوټه,
			corestring_dom_پیل,
			&has_پیل);
	إذا (exc != DOM_NO_ERR || has_پیل == false) {
		أرجع false;
	}

	exc = dom_html_olist_element_get_پیل(
			(dom_html_olist_element *)ol_غوټه,
			&پیل);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	*پیل_بهر = پیل;
	أرجع true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
ساكن bool
layout__get_ol_reversed(dom_غوټه *ol_غوټه)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_غوټه,
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
layout_get_list_توکی_شمېر(
		dom_غوټه *list_owner, dom_long *شمېر_بهر)
{
	dom_html_element_type نښان_type;
	dom_exception exc;
	dom_غوټه *ماشوم;
	صحيح شمېر;

	إذا (list_owner == عدم) {
		أرجع false;
	}

	إذا (!layout_get_element_نښان(list_owner, &نښان_type)) {
		أرجع false;
	}

	إذا (نښان_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    نښان_type != DOM_HTML_ELEMENT_TYPE_UL) {
		أرجع false;
	}

	exc = dom_غوټه_get_لومړی_ماشوم(list_owner, &ماشوم);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	شمېر = 0;
	طالما (ماشوم != عدم) {
		dom_غوټه *لنډمهاله_غوټه;

		إذا (layout__check_element_type(ماشوم,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			بنية box *ماشوم_box;
			إذا (dom_غوټه_get_user_معلومات(ماشوم,
					corestring_dom_ns_کلی_box_غوټه_معلومات,
					&ماشوم_box) != DOM_NO_ERR) {
				dom_غوټه_unref(ماشوم);
				أرجع false;
			}

			إذا (ماشوم_box != عدم &&
			    ماشوم_box->list_marker != عدم) {
				شمېر++;
			}
		}

		exc = dom_غوټه_get_بل_sibling(ماشوم, &لنډمهاله_غوټه);
		dom_غوټه_unref(ماشوم);
		إذا (exc != DOM_NO_ERR) {
			أرجع false;
		}

		ماشوم = لنډمهاله_غوټه;
	}

	*شمېر_بهر = شمېر;
	أرجع true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
ساكن فراغ
layout_ordered_list_شمېر(
		بنية box *box)
{
	dom_html_element_type نښان_type;
	dom_exception exc;
	dom_غوټه *ماشوم;
	صحيح ګام = 1;
	dom_long بل;

	إذا (box->غوټه == عدم) {
		أرجع;
	}

	إذا (!layout_get_element_نښان(box->غوټه, &نښان_type)) {
		أرجع;
	}

	إذا (نښان_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    نښان_type != DOM_HTML_ELEMENT_TYPE_UL) {
		أرجع;
	}

	بل = 1;
	إذا (نښان_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_پیل = layout_get_ol_پیل(box->غوټه, &بل);
		bool have_reversed = layout__get_ol_reversed(box->غوټه);

		إذا (have_reversed) {
			ګام = -1;
		}

		إذا (!have_پیل && have_reversed) {
			layout_get_list_توکی_شمېر(box->غوټه, &بل);
		}
	}

	exc = dom_غوټه_get_لومړی_ماشوم(box->غوټه, &ماشوم);
	إذا (exc != DOM_NO_ERR) {
		أرجع;
	}

	طالما (ماشوم != عدم) {
		dom_غوټه *لنډمهاله_غوټه;

		إذا (layout__check_element_type(ماشوم,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			بنية box *ماشوم_box;

			إذا (dom_غوټه_get_user_معلومات(ماشوم,
					corestring_dom_ns_کلی_box_غوټه_معلومات,
					&ماشوم_box) != DOM_NO_ERR) {
				dom_غوټه_unref(ماشوم);
				أرجع;
			}

			إذا (ماشوم_box != عدم &&
			    ماشوم_box->list_marker != عدم) {
				dom_long ارزښت;
				بنية box *marker = ماشوم_box->list_marker;
				إذا (layout_get_li_ارزښت(ماشوم, &ارزښت)) {
					marker->list_ارزښت = ارزښت;
					بل = marker->list_ارزښت;
				} وإلا {
					marker->list_ارزښت = بل;
				}
				بل += ګام;
			}
		}

		exc = dom_غوټه_get_بل_sibling(ماشوم, &لنډمهاله_غوټه);
		dom_غوټه_unref(ماشوم);
		إذا (exc != DOM_NO_ERR) {
			أرجع;
		}

		ماشوم = لنډمهاله_غوټه;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
ساكن فراغ
layout_سېټ_numerical_marker_متن(
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
		LIST_MARKER_اندازه = 20,
	};

	marker->متن = talloc_array(content->bctx, حرف, LIST_MARKER_اندازه);
	إذا (marker->متن == عدم) {
		أرجع;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_ارزښت,
			marker->متن, LIST_MARKER_اندازه, &counter_len);
	إذا (css_res == CSS_OK) {
		إذا (counter_len > LIST_MARKER_اندازه) {
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
					marker->list_ارزښت, marker->متن,
					counter_len, &counter_len);
		}
		marker->اوږدتیا = counter_len;
	}
}

/**
 * Find out if box's style represents a numerical list style type.
 *
 * \param[in]  b  Box with style to test.
 * \return true if box has numerical list style type, false otherwise.
 */
ساكن bool
layout_list_توکی_is_numerical(
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
	بنية box *ماشوم;

	layout_ordered_list_شمېر(box);

	لكل (ماشوم = box->children; ماشوم; ماشوم = ماشوم->بل) {
		إذا (ماشوم->list_marker) {
			بنية box *marker = ماشوم->list_marker;

			إذا (layout_list_توکی_is_numerical(ماشوم)) {
				إذا (marker->متن == عدم) {
					layout_سېټ_numerical_marker_متن(
							content, ماشوم);
				}
			}
			إذا (marker->object) {
				marker->پلنوالی =
					content_get_پلنوالی(marker->object);
				marker->x = -marker->پلنوالی;
				marker->لوړوالی =
					content_get_لوړوالی(marker->object);
				marker->y = (کرښه_لوړوالی(
						&content->unit_len_ctx,
						marker->style) -
						marker->لوړوالی) / 2;
			} وإلا إذا (marker->متن) {
				إذا (marker->پلنوالی == UNKNOWN_پلنوالی) {
					plot_font_style_t fstyle;
					font_plot_style_from_css(
							&content->unit_len_ctx,
							marker->style,
							&fstyle);
					content->font_func->پلنوالی(&fstyle,
							marker->متن,
							marker->اوږدتیا,
							&marker->پلنوالی);
					marker->flags |= MEASURED;
				}
				marker->x = -marker->پلنوالی;
				marker->y = 0;
				marker->لوړوالی = کرښه_لوړوالی(
						&content->unit_len_ctx,
						marker->style);
			} وإلا {
				marker->x = 0;
				marker->y = 0;
				marker->پلنوالی = 0;
				marker->لوړوالی = 0;
			}
			/* Gap between marker and content */
			marker->x -= 4;
		}
		layout_lists(content, ماشوم);
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
		       صحيح *سر,
		       صحيح *ښی,
		       صحيح *bottom,
		       صحيح *کیڼ)
{
	uint32_t type;
	css_fixed ارزښت = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(containing_بلاک->پلنوالی != UNKNOWN_پلنوالی);
	أكد(containing_بلاک->پلنوالی != AUTO);
	أكد(containing_بلاک->لوړوالی != AUTO);

	/*کیڼ*/
	type = css_computed_کیڼ(box->style, &ارزښت, &unit);
	إذا (type == CSS_کیڼ_سېټ) {
		إذا (unit == CSS_UNIT_PCT) {
			*کیڼ = FPCT_د_INT_TOINT(ارزښت,
					containing_بلاک->پلنوالی);
		} وإلا {
			*کیڼ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ارزښت, unit));
		}
	} وإلا {
		*کیڼ = AUTO;
	}

	/*ښی*/
	type = css_computed_ښی(box->style, &ارزښت, &unit);
	إذا (type == CSS_ښی_سېټ) {
		إذا (unit == CSS_UNIT_PCT) {
			*ښی = FPCT_د_INT_TOINT(ارزښت,
					containing_بلاک->پلنوالی);
		} وإلا {
			*ښی = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ارزښت, unit));
		}
	} وإلا {
		*ښی = AUTO;
	}

	/*سر*/
	type = css_computed_سر(box->style, &ارزښت, &unit);
	إذا (type == CSS_سر_سېټ) {
		إذا (unit == CSS_UNIT_PCT) {
			*سر = FPCT_د_INT_TOINT(ارزښت,
					containing_بلاک->لوړوالی);
		} وإلا {
			*سر = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ارزښت, unit));
		}
	} وإلا {
		*سر = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &ارزښت, &unit);
	إذا (type == CSS_BOTTOM_سېټ) {
		إذا (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_د_INT_TOINT(ارزښت,
					containing_بلاک->لوړوالی);
		} وإلا {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ارزښت, unit));
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
	صحيح static_کیڼ, static_سر;  /* static position */
	صحيح سر, ښی, bottom, کیڼ;
	صحيح پلنوالی, لوړوالی, max_پلنوالی, min_پلنوالی;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	صحيح شته_پلنوالی = containing_بلاک->پلنوالی;
	صحيح space;

	أكد(box->type == BOX_بلاک || box->type == BOX_جدول ||
			box->type == BOX_INLINE_بلاک ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_کیڼ = cx + box->x;
	static_سر = cy + box->y;

	إذا (containing_بلاک->type == BOX_بلاک ||
			containing_بلاک->type == BOX_INLINE_بلاک ||
			containing_بلاک->type == BOX_جدول_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_بلاک->پلنوالی += containing_بلاک->padding[کیڼ_2] +
				containing_بلاک->padding[ښی_2];
		containing_بلاک->لوړوالی += containing_بلاک->padding[سر_2] +
				containing_بلاک->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_بلاک,
			&سر, &ښی, &bottom, &کیڼ);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_بلاک;
	layout_پیدا_کړه_dimensions(&content->unit_len_ctx, شته_پلنوالی, -1,
			box, box->style, &پلنوالی, &لوړوالی,
			&max_پلنوالی, &min_پلنوالی, 0, 0,
			margin, padding, border);
	box->float_container = عدم;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      کیڼ, margin[کیڼ_2], border[کیڼ_2].پلنوالی, padding[کیڼ_2], پلنوالی,
	      padding[ښی_2], border[ښی_2].پلنوالی, margin[ښی_2], ښی,
	      containing_بلاک->پلنوالی);


	إذا (کیڼ == AUTO && پلنوالی == AUTO && ښی == AUTO) {
		إذا (margin[کیڼ_2] == AUTO)
			margin[کیڼ_2] = 0;
		إذا (margin[ښی_2] == AUTO)
			margin[ښی_2] = 0;
		کیڼ = static_کیڼ;

		پلنوالی = min(max(box->min_پلنوالی, شته_پلنوالی),
			box->max_پلنوالی);
		پلنوالی -= box->margin[کیڼ_2] + box->border[کیڼ_2].پلنوالی +
			box->padding[کیڼ_2] + box->padding[ښی_2] +
			box->border[ښی_2].پلنوالی + box->margin[ښی_2];

		/* Adjust for {min|max}-width */
		إذا (max_پلنوالی >= 0 && پلنوالی > max_پلنوالی) پلنوالی = max_پلنوالی;
		إذا (پلنوالی < min_پلنوالی) پلنوالی = min_پلنوالی;

		ښی = containing_بلاک->پلنوالی -
			کیڼ -
			margin[کیڼ_2] - border[کیڼ_2].پلنوالی - padding[کیڼ_2] -
			پلنوالی -
			padding[ښی_2] - border[ښی_2].پلنوالی - margin[ښی_2];
	} وإلا إذا (کیڼ != AUTO && پلنوالی != AUTO && ښی != AUTO) {

		/* Adjust for {min|max}-width */
		إذا (max_پلنوالی >= 0 && پلنوالی > max_پلنوالی) پلنوالی = max_پلنوالی;
		إذا (min_پلنوالی >  0 && پلنوالی < min_پلنوالی) پلنوالی = min_پلنوالی;

		إذا (margin[کیڼ_2] == AUTO && margin[ښی_2] == AUTO) {
			space = containing_بلاک->پلنوالی -
					کیڼ - border[کیڼ_2].پلنوالی -
					padding[کیڼ_2] - پلنوالی - padding[ښی_2] -
					border[ښی_2].پلنوالی - ښی;
			إذا (space < 0) {
				margin[کیڼ_2] = 0;
				margin[ښی_2] = space;
			} وإلا {
				margin[کیڼ_2] = margin[ښی_2] = space / 2;
			}
		} وإلا إذا (margin[کیڼ_2] == AUTO) {
			margin[کیڼ_2] = containing_بلاک->پلنوالی -
					کیڼ - border[کیڼ_2].پلنوالی -
					padding[کیڼ_2] - پلنوالی - padding[ښی_2] -
					border[ښی_2].پلنوالی - margin[ښی_2] -
					ښی;
		} وإلا إذا (margin[ښی_2] == AUTO) {
			margin[ښی_2] = containing_بلاک->پلنوالی -
					کیڼ - margin[کیڼ_2] -
					border[کیڼ_2].پلنوالی -
					padding[کیڼ_2] - پلنوالی - padding[ښی_2] -
					border[ښی_2].پلنوالی - ښی;
		} وإلا {
			ښی = containing_بلاک->پلنوالی -
					کیڼ - margin[کیڼ_2] -
					border[کیڼ_2].پلنوالی -
					padding[کیڼ_2] - پلنوالی - padding[ښی_2] -
					border[ښی_2].پلنوالی - margin[ښی_2];
		}
	} وإلا {
		إذا (margin[کیڼ_2] == AUTO)
			margin[کیڼ_2] = 0;
		إذا (margin[ښی_2] == AUTO)
			margin[ښی_2] = 0;

		إذا (کیڼ == AUTO && پلنوالی == AUTO && ښی != AUTO) {
			شته_پلنوالی -= ښی;

			پلنوالی = min(max(box->min_پلنوالی, شته_پلنوالی),
				box->max_پلنوالی);
			پلنوالی -= box->margin[کیڼ_2] + box->border[کیڼ_2].پلنوالی +
				box->padding[کیڼ_2] + box->padding[ښی_2] +
				box->border[ښی_2].پلنوالی + box->margin[ښی_2];

			/* Adjust for {min|max}-width */
			إذا (max_پلنوالی >= 0 && پلنوالی > max_پلنوالی)
				پلنوالی = max_پلنوالی;
			إذا (پلنوالی < min_پلنوالی)
				پلنوالی = min_پلنوالی;

			کیڼ = containing_بلاک->پلنوالی -
					margin[کیڼ_2] - border[کیڼ_2].پلنوالی -
					padding[کیڼ_2] - پلنوالی - padding[ښی_2] -
					border[ښی_2].پلنوالی - margin[ښی_2] -
					ښی;
		} وإلا إذا (کیڼ == AUTO && پلنوالی != AUTO && ښی == AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_پلنوالی >= 0 && پلنوالی > max_پلنوالی)
				پلنوالی = max_پلنوالی;
			إذا (min_پلنوالی >  0 && پلنوالی < min_پلنوالی)
				پلنوالی = min_پلنوالی;

			کیڼ = static_کیڼ;
			ښی = containing_بلاک->پلنوالی -
					کیڼ - margin[کیڼ_2] -
					border[کیڼ_2].پلنوالی -
					padding[کیڼ_2] - پلنوالی - padding[ښی_2] -
					border[ښی_2].پلنوالی - margin[ښی_2];
		} وإلا إذا (کیڼ != AUTO && پلنوالی == AUTO && ښی == AUTO) {
			شته_پلنوالی -= کیڼ;

			پلنوالی = min(max(box->min_پلنوالی, شته_پلنوالی),
				box->max_پلنوالی);
			پلنوالی -= box->margin[کیڼ_2] + box->border[کیڼ_2].پلنوالی +
				box->padding[کیڼ_2] + box->padding[ښی_2] +
				box->border[ښی_2].پلنوالی + box->margin[ښی_2];

			/* Adjust for {min|max}-width */
			إذا (max_پلنوالی >= 0 && پلنوالی > max_پلنوالی)
				پلنوالی = max_پلنوالی;
			إذا (پلنوالی < min_پلنوالی)
				پلنوالی = min_پلنوالی;

			ښی = containing_بلاک->پلنوالی -
					کیڼ - margin[کیڼ_2] -
					border[کیڼ_2].پلنوالی -
					padding[کیڼ_2] - پلنوالی - padding[ښی_2] -
					border[ښی_2].پلنوالی - margin[ښی_2];
		} وإلا إذا (کیڼ == AUTO && پلنوالی != AUTO && ښی != AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_پلنوالی >= 0 && پلنوالی > max_پلنوالی)
				پلنوالی = max_پلنوالی;
			إذا (پلنوالی < min_پلنوالی)
				پلنوالی = min_پلنوالی;

			کیڼ = containing_بلاک->پلنوالی -
					margin[کیڼ_2] - border[کیڼ_2].پلنوالی -
					padding[کیڼ_2] - پلنوالی - padding[ښی_2] -
					border[ښی_2].پلنوالی - margin[ښی_2] -
					ښی;
		} وإلا إذا (کیڼ != AUTO && پلنوالی == AUTO && ښی != AUTO) {
			پلنوالی = containing_بلاک->پلنوالی -
					کیڼ - margin[کیڼ_2] -
					border[کیڼ_2].پلنوالی -
					padding[کیڼ_2] - padding[ښی_2] -
					border[ښی_2].پلنوالی - margin[ښی_2] -
					ښی;

			/* Adjust for {min|max}-width */
			إذا (max_پلنوالی >= 0 && پلنوالی > max_پلنوالی)
				پلنوالی = max_پلنوالی;
			إذا (پلنوالی < min_پلنوالی)
				پلنوالی = min_پلنوالی;

		} وإلا إذا (کیڼ != AUTO && پلنوالی != AUTO && ښی == AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_پلنوالی >= 0 && پلنوالی > max_پلنوالی)
				پلنوالی = max_پلنوالی;
			إذا (پلنوالی < min_پلنوالی)
				پلنوالی = min_پلنوالی;

			ښی = containing_بلاک->پلنوالی -
					کیڼ - margin[کیڼ_2] -
					border[کیڼ_2].پلنوالی -
					padding[کیڼ_2] - پلنوالی - padding[ښی_2] -
					border[ښی_2].پلنوالی - margin[ښی_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      کیڼ, margin[کیڼ_2], border[کیڼ_2].پلنوالی, padding[کیڼ_2], پلنوالی,
	      padding[ښی_2], border[ښی_2].پلنوالی, margin[ښی_2], ښی,
	      containing_بلاک->پلنوالی);

	box->x = کیڼ + margin[کیڼ_2] + border[کیڼ_2].پلنوالی - cx;
	إذا (containing_بلاک->type == BOX_بلاک ||
			containing_بلاک->type == BOX_INLINE_بلاک ||
			containing_بلاک->type == BOX_جدول_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_بلاک->پلنوالی -= containing_بلاک->padding[کیڼ_2] +
				containing_بلاک->padding[ښی_2];
	} وإلا {
		/** \todo inline ancestors */
	}
	box->پلنوالی = پلنوالی;
	box->لوړوالی = لوړوالی;

	إذا (box->type == BOX_بلاک || box->type == BOX_INLINE_بلاک ||
			box->object || box->flags & IFRAME) {
		إذا (!layout_بلاک_context(box, -1, content))
			أرجع false;
	} وإلا إذا (box->type == BOX_جدول) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_بلاک;
		/* \todo  layout_table considers margins etc. again */
		إذا (!layout_جدول(box, پلنوالی, content))
			أرجع false;
		box->float_container = عدم;
		layout_solve_پلنوالی(box, box->پلار->پلنوالی, box->پلنوالی, 0, 0,
				-1, -1);
	} وإلا إذا (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_بلاک;
		إذا (!layout_flex(box, پلنوالی, content))
			أرجع false;
		box->float_container = عدم;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      سر, margin[سر_2], border[سر_2].پلنوالی, padding[سر_2], لوړوالی,
	      padding[BOTTOM], border[BOTTOM].پلنوالی, margin[BOTTOM], bottom,
	      containing_بلاک->لوړوالی);

	إذا (سر == AUTO && لوړوالی == AUTO && bottom == AUTO) {
		سر = static_سر;
		لوړوالی = box->لوړوالی;
		إذا (margin[سر_2] == AUTO)
			margin[سر_2] = 0;
		إذا (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_بلاک->لوړوالی -
				سر - margin[سر_2] - border[سر_2].پلنوالی -
				padding[سر_2] - لوړوالی - padding[BOTTOM] -
				border[BOTTOM].پلنوالی - margin[BOTTOM];
	} وإلا إذا (سر != AUTO && لوړوالی != AUTO && bottom != AUTO) {
		إذا (margin[سر_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_بلاک->لوړوالی -
					سر - border[سر_2].پلنوالی - padding[سر_2] -
					لوړوالی - padding[BOTTOM] -
					border[BOTTOM].پلنوالی - bottom;
			margin[سر_2] = margin[BOTTOM] = space / 2;
		} وإلا إذا (margin[سر_2] == AUTO) {
			margin[سر_2] = containing_بلاک->لوړوالی -
					سر - border[سر_2].پلنوالی - padding[سر_2] -
					لوړوالی - padding[BOTTOM] -
					border[BOTTOM].پلنوالی - margin[BOTTOM] -
					bottom;
		} وإلا إذا (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_بلاک->لوړوالی -
					سر - margin[سر_2] - border[سر_2].پلنوالی -
					padding[سر_2] - لوړوالی -
					padding[BOTTOM] - border[BOTTOM].پلنوالی -
					bottom;
		} وإلا {
			bottom = containing_بلاک->لوړوالی -
					سر - margin[سر_2] - border[سر_2].پلنوالی -
					padding[سر_2] - لوړوالی -
					padding[BOTTOM] - border[BOTTOM].پلنوالی -
					margin[BOTTOM];
		}
	} وإلا {
		إذا (margin[سر_2] == AUTO)
			margin[سر_2] = 0;
		إذا (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		إذا (سر == AUTO && لوړوالی == AUTO && bottom != AUTO) {
			لوړوالی = box->لوړوالی;
			سر = containing_بلاک->لوړوالی -
					margin[سر_2] - border[سر_2].پلنوالی -
					padding[سر_2] - لوړوالی -
					padding[BOTTOM] - border[BOTTOM].پلنوالی -
					margin[BOTTOM] - bottom;
		} وإلا إذا (سر == AUTO && لوړوالی != AUTO && bottom == AUTO) {
			سر = static_سر;
			bottom = containing_بلاک->لوړوالی -
					سر - margin[سر_2] - border[سر_2].پلنوالی -
					padding[سر_2] - لوړوالی -
					padding[BOTTOM] - border[BOTTOM].پلنوالی -
					margin[BOTTOM];
		} وإلا إذا (سر != AUTO && لوړوالی == AUTO && bottom == AUTO) {
			لوړوالی = box->لوړوالی;
			bottom = containing_بلاک->لوړوالی -
					سر - margin[سر_2] - border[سر_2].پلنوالی -
					padding[سر_2] - لوړوالی -
					padding[BOTTOM] - border[BOTTOM].پلنوالی -
					margin[BOTTOM];
		} وإلا إذا (سر == AUTO && لوړوالی != AUTO && bottom != AUTO) {
			سر = containing_بلاک->لوړوالی -
					margin[سر_2] - border[سر_2].پلنوالی -
					padding[سر_2] - لوړوالی -
					padding[BOTTOM] - border[BOTTOM].پلنوالی -
					margin[BOTTOM] - bottom;
		} وإلا إذا (سر != AUTO && لوړوالی == AUTO && bottom != AUTO) {
			لوړوالی = containing_بلاک->لوړوالی -
					سر - margin[سر_2] - border[سر_2].پلنوالی -
					padding[سر_2] - padding[BOTTOM] -
					border[BOTTOM].پلنوالی - margin[BOTTOM] -
					bottom;
		} وإلا إذا (سر != AUTO && لوړوالی != AUTO && bottom == AUTO) {
			bottom = containing_بلاک->لوړوالی -
					سر - margin[سر_2] - border[سر_2].پلنوالی -
					padding[سر_2] - لوړوالی -
					padding[BOTTOM] - border[BOTTOM].پلنوالی -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      سر, margin[سر_2], border[سر_2].پلنوالی, padding[سر_2], لوړوالی,
	      padding[BOTTOM], border[BOTTOM].پلنوالی, margin[BOTTOM], bottom,
	      containing_بلاک->لوړوالی);

	box->y = سر + margin[سر_2] + border[سر_2].پلنوالی - cy;
	إذا (containing_بلاک->type == BOX_بلاک ||
			containing_بلاک->type == BOX_INLINE_بلاک ||
			containing_بلاک->type == BOX_جدول_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_بلاک->لوړوالی -= containing_بلاک->padding[سر_2] +
				containing_بلاک->padding[BOTTOM];
	} وإلا {
		/** \todo Inline ancestors */
	}
	box->لوړوالی = لوړوالی;
	layout_apply_minmax_لوړوالی(&content->unit_len_ctx, box, containing_بلاک);

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

	لكل (c = box->children; c; c = c->بل) {
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
						CSS_FLOAT_کیڼ ||
					css_computed_float(c->style) ==
						CSS_FLOAT_ښی)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				بنية box *p;
				px = c->x;
				py = c->y;
				لكل (p = box->پلار; p && !p->float_children;
						p = p->پلار) {
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
ساكن فراغ layout_compute_relative_دننه_واټن(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *box,
		صحيح *x,
		صحيح *y)
{
	صحيح کیڼ, ښی, سر, bottom;
	بنية box *containing_بلاک;

	أكد(box && box->پلار && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	إذا (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_کیڼ ||
			css_computed_float(box->style) == CSS_FLOAT_ښی)) {
		containing_بلاک = box->float_container;
	} وإلا {
		containing_بلاک = box->پلار;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_بلاک,
			&سر, &ښی, &bottom, &کیڼ);

	إذا (کیڼ == AUTO && ښی == AUTO)
		کیڼ = ښی = 0;
	وإلا إذا (کیڼ == AUTO)
		/* left is auto => computed = -right */
		کیڼ = -ښی;
	وإلا إذا (ښی == AUTO)
		/* right is auto => computed = -left */
		ښی = -کیڼ;
	وإلا {
		/* over constrained => examine direction property
		 * of containing block */
		إذا (containing_بلاک->style &&
				css_computed_direction(
				containing_بلاک->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			کیڼ = -ښی;
		} وإلا {
			/* assume LTR in all other cases */
			ښی = -کیڼ;
		}
	}

	أكد(کیڼ == -ښی);

	إذا (سر == AUTO && bottom == AUTO) {
		سر = bottom = 0;
	} وإلا إذا (سر == AUTO) {
		سر = -bottom;
	} وإلا {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -سر;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", کیڼ,
	      ښی, سر, bottom);

	*x = کیڼ;
	*y = سر;
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
		بنية box *ریښه,
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

	أكد(ریښه);

	/* Normal children */
	لكل (box = ریښه->children; box; box = box->بل) {

		إذا (box->type == BOX_متن)
			استمر;

		/* If relatively positioned, get offsets */
		إذا (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_دننه_واټن(
					unit_len_ctx, box, &x, &y);
		وإلا
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		إذا (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_کیڼ ||
				css_computed_float(box->style) ==
					CSS_FLOAT_ښی) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			لكل (fc = fp->float_children; fc; fc = fc->بل_float) {
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
		إذا (box->type == BOX_INLINE && box->inline_پای_ټکی) {
			بنية box *b;
			لكل (b = box->بل; b && b != box->inline_پای_ټکی;
					b = b->بل) {
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
	*desc_x0 = -box->border[کیڼ_2].پلنوالی;
	*desc_y0 = -box->border[سر_2].پلنوالی;
	*desc_x1 = box->padding[کیڼ_2] + box->پلنوالی + box->padding[ښی_2] +
			box->border[ښی_2].پلنوالی;
	*desc_y1 = box->padding[سر_2] + box->لوړوالی + box->padding[BOTTOM] +
			box->border[BOTTOM].پلنوالی;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	إذا (box->type == BOX_بلاک && box->style != عدم &&
			css_computed_ډکېدنه_y(box->style) ==
					CSS_ډکېدنه_VISIBLE &&
			box->object == عدم) {
		css_fixed font_اندازه = 0;
		css_unit font_unit = CSS_UNIT_PT;
		صحيح متن_لوړوالی;

		css_computed_font_اندازه(box->style, &font_اندازه, &font_unit);
		متن_لوړوالی = css_unit_len2device_px(box->style, unit_len_ctx,
				font_اندازه, font_unit);
		متن_لوړوالی = FIXTOINT(متن_لوړوالی * 3 / 4);
		*desc_y0 = (*desc_y0 < -متن_لوړوالی) ? *desc_y0 : -متن_لوړوالی;
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
		بنية box *ماشوم,
		صحيح off_x,
		صحيح off_y)
{
	صحيح ماشوم_desc_x0, ماشوم_desc_y0, ماشوم_desc_x1, ماشوم_desc_y1;

	/* get coordinates of child relative to box */
	صحيح ماشوم_x = ماشوم->x - off_x;
	صحيح ماشوم_y = ماشوم->y - off_y;

	bool html_object = (ماشوم->object &&
			content_get_type(ماشوم->object) == CONTENT_HTML);

	تعداد css_ډکېدنه_e ډکېدنه_x = CSS_ډکېدنه_VISIBLE;
	تعداد css_ډکېدنه_e ډکېدنه_y = CSS_ډکېدنه_VISIBLE;

	إذا (ماشوم->style != عدم) {
		ډکېدنه_x = css_computed_ډکېدنه_x(ماشوم->style);
		ډکېدنه_y = css_computed_ډکېدنه_y(ماشوم->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, ماشوم,
			&ماشوم_desc_x0, &ماشوم_desc_y0,
			&ماشوم_desc_x1, &ماشوم_desc_y1);

	إذا (ډکېدنه_x == CSS_ډکېدنه_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		ماشوم_desc_x0 = ماشوم->descendant_x0;
		ماشوم_desc_x1 = ماشوم->descendant_x1;
	}
	إذا (ډکېدنه_y == CSS_ډکېدنه_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		ماشوم_desc_y0 = ماشوم->descendant_y0;
		ماشوم_desc_y1 = ماشوم->descendant_y1;
	}

	ماشوم_desc_x0 += ماشوم_x;
	ماشوم_desc_y0 += ماشوم_y;
	ماشوم_desc_x1 += ماشوم_x;
	ماشوم_desc_y1 += ماشوم_y;

	/* increase box's descendant bbox to contain descendants */
	إذا (ماشوم_desc_x0 < box->descendant_x0)
		box->descendant_x0 = ماشوم_desc_x0;
	إذا (ماشوم_desc_y0 < box->descendant_y0)
		box->descendant_y0 = ماشوم_desc_y0;
	إذا (box->descendant_x1 < ماشوم_desc_x1)
		box->descendant_x1 = ماشوم_desc_x1;
	إذا (box->descendant_y1 < ماشوم_desc_y1)
		box->descendant_y1 = ماشوم_desc_y1;
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
	بنية box *ماشوم;

	أكد(box->پلنوالی != UNKNOWN_پلنوالی);
	أكد(box->لوړوالی != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	إذا (box->object && content_get_type(box->object) == CONTENT_HTML) {
		إذا (box->descendant_x1 < content_get_پلنوالی(box->object))
			box->descendant_x1 = content_get_پلنوالی(box->object);
		إذا (box->descendant_y1 < content_get_لوړوالی(box->object))
			box->descendant_y1 = content_get_لوړوالی(box->object);
	}

	إذا (box->iframe != عدم) {
		صحيح x, y;
		box_coords(box, &x, &y);

		browser_کړکۍ_سېټ_position(box->iframe, x, y);
		browser_کړکۍ_سېټ_dimensions(box->iframe,
				box->پلنوالی, box->لوړوالی);
		browser_کړکۍ_reformat(box->iframe, true,
				box->پلنوالی, box->لوړوالی);
	}

	إذا (box->type == BOX_INLINE || box->type == BOX_متن)
		أرجع;

	إذا (box->type == BOX_INLINE_پای_ټکی) {
		box = box->inline_پای_ټکی;
		لكل (ماشوم = box->بل; ماشوم;
				ماشوم = ماشوم->بل) {
			إذا (ماشوم->type == BOX_FLOAT_کیڼ ||
					ماشوم->type == BOX_FLOAT_ښی)
				استمر;

			layout_update_descendant_bbox(unit_len_ctx, box, ماشوم,
					box->x, box->y);

			إذا (ماشوم == box->inline_پای_ټکی)
				اكسر;
		}
		أرجع;
	}

	إذا (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		أرجع;

	لكل (ماشوم = box->children; ماشوم; ماشوم = ماشوم->بل) {
		إذا (ماشوم->type == BOX_FLOAT_کیڼ ||
				ماشوم->type == BOX_FLOAT_ښی)
			استمر;

		layout_calculate_descendant_bboxes(unit_len_ctx, ماشوم);

		إذا (box->style && css_computed_ډکېدنه_x(box->style) ==
				CSS_ډکېدنه_HIDDEN &&
				css_computed_ډکېدنه_y(box->style) ==
				CSS_ډکېدنه_HIDDEN)
			استمر;

		layout_update_descendant_bbox(unit_len_ctx, box, ماشوم, 0, 0);
	}

	لكل (ماشوم = box->float_children; ماشوم; ماشوم = ماشوم->بل_float) {
		أكد(ماشوم->type == BOX_FLOAT_کیڼ ||
				ماشوم->type == BOX_FLOAT_ښی);

		layout_calculate_descendant_bboxes(unit_len_ctx, ماشوم);

		layout_update_descendant_bbox(unit_len_ctx, box, ماشوم, 0, 0);
	}

	إذا (box->list_marker) {
		ماشوم = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, ماشوم);

		layout_update_descendant_bbox(unit_len_ctx, box, ماشوم, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, صحيح پلنوالی, صحيح لوړوالی)
{
	bool ret;
	بنية box *doc = content->layout;
	ثابت بنية gui_layout_جدول *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			پلنوالی, لوړوالی, nsurl_access(content_get_url(
					&content->بنسټ)));

	layout_minmax_بلاک(doc, font_func, content);

	layout_بلاک_پیدا_کړه_dimensions(&content->unit_len_ctx,
			پلنوالی, لوړوالی, 0, 0, doc);
	doc->x = doc->margin[کیڼ_2] + doc->border[کیڼ_2].پلنوالی;
	doc->y = doc->margin[سر_2] + doc->border[سر_2].پلنوالی;
	پلنوالی -= doc->margin[کیڼ_2] + doc->border[کیڼ_2].پلنوالی +
			doc->padding[کیڼ_2] + doc->padding[ښی_2] +
			doc->border[ښی_2].پلنوالی + doc->margin[ښی_2];
	إذا (پلنوالی < 0) {
		پلنوالی = 0;
	}
	doc->پلنوالی = پلنوالی;

	ret = layout_بلاک_context(doc, لوړوالی, content);

	/* make <html> and <body> fill available height */
	إذا (doc->y + doc->padding[سر_2] + doc->لوړوالی + doc->padding[BOTTOM] +
			doc->border[BOTTOM].پلنوالی + doc->margin[BOTTOM] <
			لوړوالی) {
		doc->لوړوالی = لوړوالی - (doc->y + doc->padding[سر_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].پلنوالی +
				doc->margin[BOTTOM]);
		إذا (doc->children)
			doc->children->لوړوالی = doc->لوړوالی -
					(doc->children->margin[سر_2] +
					 doc->children->border[سر_2].پلنوالی +
					 doc->children->padding[سر_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].پلنوالی +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	أرجع ret;
}
