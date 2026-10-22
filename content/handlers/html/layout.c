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
	[چوٽي_2]    = css_computed_margin_چوٽي,
	[ساڄو_2]  = css_computed_margin_ساڄو,
	[BOTTOM] = css_computed_margin_bottom,
	[کاٻو_2]   = css_computed_margin_کاٻو,
};

/** Array of per-side access functions for computed style paddings. */
ثابت css_len_func padding_funcs[4] = {
	[چوٽي_2]    = css_computed_padding_چوٽي,
	[ساڄو_2]  = css_computed_padding_ساڄو,
	[BOTTOM] = css_computed_padding_bottom,
	[کاٻو_2]   = css_computed_padding_کاٻو,
};

/** Array of per-side access functions for computed style border_widths. */
ثابت css_len_func border_ويڪر_funcs[4] = {
	[چوٽي_2]    = css_computed_border_چوٽي_ويڪر,
	[ساڄو_2]  = css_computed_border_ساڄو_ويڪر,
	[BOTTOM] = css_computed_border_bottom_ويڪر,
	[کاٻو_2]   = css_computed_border_کاٻو_ويڪر,
};

/** Array of per-side access functions for computed style border styles. */
ثابت css_border_style_func border_style_funcs[4] = {
	[چوٽي_2]    = css_computed_border_چوٽي_style,
	[ساڄو_2]  = css_computed_border_ساڄو_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[کاٻو_2]   = css_computed_border_کاٻو_style,
};

/** Array of per-side access functions for computed style border colors. */
ثابت css_border_color_func border_color_funcs[4] = {
	[چوٽي_2]    = css_computed_border_چوٽي_color,
	[ساڄو_2]  = css_computed_border_ساڄو_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[کاٻو_2]   = css_computed_border_کاٻو_color,
};

/* forward declaration to break cycles */
ساكن فراغ layout_minmax_بلاڪ(
		بنية box *بلاڪ,
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
			     صحيح *ويڪر, صحيح *اوچائي,
			     صحيح min_ويڪر, صحيح max_ويڪر,
			     صحيح min_اوچائي, صحيح max_اوچائي)
{
	أكد(box->object != عدم);
	أكد(ويڪر != عدم && اوچائي != عدم);

	إذا (*ويڪر == AUTO && *اوچائي == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		صحيح intrinsic_ويڪر = content_get_ويڪر(box->object);
		صحيح intrinsic_اوچائي = content_get_اوچائي(box->object);

		/* use intrinsic dimensions */
		*ويڪر = intrinsic_ويڪر;
		*اوچائي = intrinsic_اوچائي;

		/* Deal with min/max-width first */
		إذا (min_ويڪر >  0 && min_ويڪر > *ويڪر) {
			*ويڪر = min_ويڪر;
			scaled = true;
		}
		إذا (max_ويڪر >= 0 && max_ويڪر < *ويڪر) {
			*ويڪر = max_ويڪر;
			scaled = true;
		}

		إذا (scaled && (intrinsic_ويڪر != 0)) {
			/* Update height */
			*اوچائي = (*ويڪر * intrinsic_اوچائي) /
					intrinsic_ويڪر;
		}

		scaled = false;
		/* Deal with min/max-height */
		إذا (min_اوچائي >  0 && min_اوچائي > *اوچائي) {
			*اوچائي = min_اوچائي;
			scaled = true;
		}
		إذا (max_اوچائي >= 0 && max_اوچائي < *اوچائي) {
			*اوچائي = max_اوچائي;
			scaled = true;
		}

		إذا (scaled && (intrinsic_اوچائي != 0)) {
			/* Update width */
			*ويڪر = (*اوچائي * intrinsic_ويڪر) /
					intrinsic_اوچائي;
		}

	} وإلا إذا (*ويڪر == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		صحيح intrinsic_ويڪر = content_get_ويڪر(box->object);
		صحيح intrinsic_اوچائي = content_get_اوچائي(box->object);

		إذا (intrinsic_اوچائي != 0)
			*ويڪر = (*اوچائي * intrinsic_ويڪر) /
					intrinsic_اوچائي;
		وإلا
			*ويڪر = intrinsic_ويڪر;

		إذا (min_ويڪر >  0 && min_ويڪر > *ويڪر)
			*ويڪر = min_ويڪر;
		إذا (max_ويڪر >= 0 && max_ويڪر < *ويڪر)
			*ويڪر = max_ويڪر;

	} وإلا إذا (*اوچائي == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		صحيح intrinsic_ويڪر = content_get_ويڪر(box->object);
		صحيح intrinsic_اوچائي = content_get_اوچائي(box->object);

		إذا (min_ويڪر >  0 && min_ويڪر > *ويڪر)
			*ويڪر = min_ويڪر;
		إذا (max_ويڪر >= 0 && max_ويڪر < *ويڪر)
			*ويڪر = max_ويڪر;

		إذا (intrinsic_ويڪر != 0)
			*اوچائي = (*ويڪر * intrinsic_اوچائي) /
					intrinsic_ويڪر;
		وإلا
			*اوچائي = intrinsic_اوچائي;
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
		ثابت css_computed_style *style, صحيح ويڪر)
{
	css_fixed قدر = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_متن_indent(style, &قدر, &unit);

	إذا (unit == CSS_UNIT_PCT) {
		أرجع FPCT_جو_INT_TOINT(قدر, ويڪر);
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
	بنانشان صحيح i, j;
	صحيح ويڪر;
	صحيح border_spacing_h = 0;
	صحيح جدول_min = 0, جدول_max = 0;
	صحيح extra_fixed = 0;
	عائم extra_frac = 0;
	بنية ڪالم *کانو;
	بنية box *صف_گروهه, *صف, *cell;

	/* check if the widths have already been calculated */
	إذا (جدول->max_ويڪر != UNKNOWN_MAX_ويڪر)
		أرجع;

	إذا (جدول_calculate_ڪالم_types(&content->unit_len_ctx, جدول) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		أرجع;
	}
	کانو = جدول->کانو;

	/* start with 0 except for fixed-width columns */
	لكل (i = 0; i != جدول->columns; i++) {
		إذا (کانو[i].type == ڪالم_ويڪر_FIXED)
			کانو[i].min = کانو[i].max = کانو[i].ويڪر;
		وإلا
			کانو[i].min = کانو[i].max = 0;
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
	لكل (صف_گروهه = جدول->children; صف_گروهه; صف_گروهه =صف_گروهه->ايندڙ)
	لكل (صف = صف_گروهه->children; صف; صف = صف->ايندڙ)
	لكل (cell = صف->children; cell; cell = cell->ايندڙ) {
		أكد(cell->type == BOX_جدول_CELL);
		أكد(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		أكد(cell->columns != 0);

		إذا (cell->columns != 1)
			استمر;

		layout_minmax_بلاڪ(cell, font_func, content);
		i = cell->شروعات_ڪالم;

		إذا (کانو[i].positioned)
			استمر;

		/* update column min, max widths using cell widths */
		إذا (کانو[i].min < cell->min_ويڪر)
			کانو[i].min = cell->min_ويڪر;
		إذا (کانو[i].max < cell->max_ويڪر)
			کانو[i].max = cell->max_ويڪر;
	}

	/* 2nd pass: cells which span multiple columns */
	لكل (صف_گروهه = جدول->children; صف_گروهه; صف_گروهه =صف_گروهه->ايندڙ)
	لكل (صف = صف_گروهه->children; صف; صف = صف->ايندڙ)
	لكل (cell = صف->children; cell; cell = cell->ايندڙ) {
		بنانشان صحيح flexible_columns = 0;
		صحيح min = 0, max = 0, fixed_ويڪر = 0, extra;

		إذا (cell->columns == 1)
			استمر;

		layout_minmax_بلاڪ(cell, font_func, content);
		i = cell->شروعات_ڪالم;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		لكل (j = 0; j != cell->columns; j++) {
			min += کانو[i + j].min;
			إذا (کانو[i + j].type == ڪالم_ويڪر_FIXED)
				fixed_ويڪر += کانو[i + j].ويڪر;
			وإلا
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		إذا (min < cell->min_ويڪر) {
			إذا (flexible_columns == 0) {
				extra = 1 + (cell->min_ويڪر - min) /
						cell->columns;
				لكل (j = 0; j != cell->columns; j++) {
					کانو[i + j].min += extra;
					إذا (کانو[i + j].max < کانو[i + j].min)
						کانو[i + j].max = کانو[i + j].min;
				}
			} وإلا {
				extra = 1 + (cell->min_ويڪر - min) /
						flexible_columns;
				لكل (j = 0; j != cell->columns; j++) {
					إذا (کانو[i + j].type !=
							ڪالم_ويڪر_FIXED) {
						کانو[i + j].min += extra;
						إذا (کانو[i + j].max <
								کانو[i + j].min)
							کانو[i + j].max =
								کانو[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		لكل (j = 0; j != cell->columns; j++)
			max += کانو[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		إذا (max < cell->max_ويڪر && flexible_columns) {
			extra = 1 + (cell->max_ويڪر - max) / flexible_columns;
			لكل (j = 0; j != cell->columns; j++)
				إذا (کانو[i + j].type != ڪالم_ويڪر_FIXED)
					کانو[i + j].max += extra;
		}
	}

	لكل (i = 0; i != جدول->columns; i++) {
		إذا (کانو[i].max < کانو[i].min) {
			box_dump(stderr, جدول, 0, true);
			أكد(0);
		}
		جدول_min += کانو[i].min;
		جدول_max += کانو[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	إذا (css_computed_ويڪر_px(جدول->style, &content->unit_len_ctx,
			-1, &ويڪر) == CSS_ويڪر_سيٽ) {
		إذا (جدول_min < ويڪر)
			جدول_min = ويڪر;
		إذا (جدول_max < ويڪر)
			جدول_max = ويڪر;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_ويڪر(&content->unit_len_ctx,
			جدول->style, کاٻو_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_ويڪر(&content->unit_len_ctx,
			جدول->style, ساڄو_2, true, true, true,
			&extra_fixed, &extra_frac);
	إذا (extra_fixed < 0)
		extra_fixed = 0;
	إذا (extra_frac < 0)
		extra_frac = 0;
	إذا (1.0 <= extra_frac)
		extra_frac = 0.9;
	جدول->min_ويڪر = (جدول_min + extra_fixed) / (1.0 - extra_frac);
	جدول->max_ويڪر = (جدول_max + extra_fixed) / (1.0 - extra_frac);
	جدول->min_ويڪر += (جدول->columns + 1) * border_spacing_h;
	جدول->max_ويڪر += (جدول->columns + 1) * border_spacing_h;

	أكد(0 <= جدول->min_ويڪر && جدول->min_ويڪر <= جدول->max_ويڪر);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
ساكن مضمن bool box_has_percentage_max_ويڪر(بنية box *b)
{
	css_unit unit = CSS_UNIT_PX;
	تعداد css_max_ويڪر_e type;
	css_fixed قدر = 0;

	أكد(b != عدم);

	type = css_computed_max_ويڪر(b->style, &قدر, &unit);
	أرجع ((type == CSS_MAX_ويڪر_سيٽ) && (unit == CSS_UNIT_PCT));
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
layout_minmax_سٽ(بنية box *پهريون,
		   صحيح *سٽ_min,
		   صحيح *سٽ_max,
		   bool پهريون_سٽ,
		   bool *سٽ_has_اوچائي,
		   ثابت بنية gui_layout_جدول *font_func,
		   ثابت html_content *content)
{
	صحيح min = 0, max = 0, ويڪر, اوچائي, fixed;
	عائم frac;
	size_t i, j;
	بنية box *b;
	بنية box *بلاڪ;
	plot_font_style_t fstyle;
	bool no_wrap;

	أكد(پهريون->والد);
	أكد(پهريون->والد->والد);
	أكد(پهريون->والد->والد->style);

	بلاڪ = پهريون->والد->والد;
	no_wrap = (css_computed_white_space(بلاڪ->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(بلاڪ->style) ==
			CSS_WHITE_SPACE_PRE);

	*سٽ_has_اوچائي = false;

	/* corresponds to the pass 1 loop in layout_line() */
	لكل (b = پهريون; b; b = b->ايندڙ) {
		تعداد css_ويڪر_e wtype;
		تعداد css_اوچائي_e htype;
		تعداد css_box_sizing_e bs;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		أكد(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		إذا (b->type == BOX_BR) {
			b = b->ايندڙ;
			اكسر;
		}

		إذا (lh__box_is_float_box(b)) {
			أكد(b->children);
			إذا (b->children->type == BOX_جدول)
				layout_minmax_جدول(b->children, font_func,
						content);
			وإلا
				layout_minmax_بلاڪ(b->children, font_func,
						content);
			b->min_ويڪر = b->children->min_ويڪر;
			b->max_ويڪر = b->children->max_ويڪر;
			إذا (min < b->min_ويڪر)
				min = b->min_ويڪر;
			max += b->max_ويڪر;
			استمر;
		}

		إذا (b->type == BOX_INLINE_بلاڪ || b->type == BOX_INLINE_FLEX) {
			layout_minmax_بلاڪ(b, font_func, content);
			إذا (min < b->min_ويڪر)
				min = b->min_ويڪر;
			max += b->max_ويڪر;

			إذا (b->flags & HAS_اوچائي)
				*سٽ_has_اوچائي = true;
			استمر;
		}

		أكد(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		إذا (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_ويڪر(&content->unit_len_ctx,
					b->style, کاٻو_2, true, true, true,
					&fixed, &frac);
			إذا (!b->inline_پڇاڙي)
				calculate_mbp_ويڪر(&content->unit_len_ctx,
						b->style, ساڄو_2,
						true, true, true,
						&fixed, &frac);
			إذا (0 < fixed)
				max += fixed;
			*سٽ_has_اوچائي = true;
			/* \todo  update min width, consider fractional extra */
		} وإلا إذا (b->type == BOX_INLINE_پڇاڙي) {
			fixed = frac = 0;
			calculate_mbp_ويڪر(&content->unit_len_ctx,
					b->inline_پڇاڙي->style, ساڄو_2,
					true, true, true,
					&fixed, &frac);
			إذا (0 < fixed)
				max += fixed;

			إذا (b->ايندڙ) {
				إذا (b->space == UNKNOWN_ويڪر) {
					font_func->ويڪر(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*سٽ_has_اوچائي = true;
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

			إذا (b->ويڪر == UNKNOWN_ويڪر) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				إذا (b->والد->والد->gadget &&
						b->والد->والد->gadget->type
						== GADGET_SELECT) {
					صحيح opt_maxwidth = 0;
					بنية form_option *o;

					لكل (o = b->والد->والد->gadget->
							معلومات.select.شيون; o;
							o = o->ايندڙ) {
						صحيح opt_ويڪر;
						font_func->ويڪر(&fstyle,
								o->متن,
								لڙي_ڊيگهه(o->متن),
								&opt_ويڪر);

						إذا (opt_maxwidth < opt_ويڪر)
							opt_maxwidth =opt_ويڪر;
					}

					b->ويڪر = opt_maxwidth;
					إذا (nsoption_bool(core_select_menu))
						b->ويڪر += SCROLLBAR_ويڪر;

				} وإلا {
					font_func->ويڪر(&fstyle, b->متن,
						b->ڊيگهائي, &b->ويڪر);
					b->flags |= MEASURED;
				}
			}
			max += b->ويڪر;
			إذا (b->ايندڙ) {
				إذا (b->space == UNKNOWN_ويڪر) {
					font_func->ويڪر(&fstyle, " ", 1,
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
				إذا (min < b->ويڪر)
					min = b->ويڪر;

			} وإلا إذا (b->والد->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				افعل {
					لكل (j = i; j != b->ڊيگهائي &&
							b->متن[j] != ' '; j++)
						;
					font_func->ويڪر(&fstyle, b->متن + i,
							 j - i, &ويڪر);
					إذا (min < ويڪر)
						min = ويڪر;
					i = j + 1;
				} طالما (j != b->ڊيگهائي);
			}

			*سٽ_has_اوچائي = true;

			استمر;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		bs = css_computed_box_sizing(بلاڪ->style);

		/* calculate box width */
		wtype = css_computed_ويڪر_px(b->style,
				&content->unit_len_ctx, -1, &ويڪر);
		إذا (wtype == CSS_ويڪر_سيٽ) {
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_ويڪر(&content->unit_len_ctx,
						بلاڪ->style, کاٻو_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_ويڪر(&content->unit_len_ctx,
						بلاڪ->style, ساڄو_2,
						false, true, true,
						&fixed, &frac);
				إذا (ويڪر < fixed) {
					ويڪر = fixed;
				}
			}
			إذا (ويڪر < 0)
				ويڪر = 0;
		} وإلا {
			ويڪر = AUTO;
		}

		/*اوچائي*/
		htype = css_computed_اوچائي(b->style, &قدر, &unit);
		إذا (htype == CSS_اوچائي_سيٽ) {
			اوچائي = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					قدر, unit));
		} وإلا {
			اوچائي = AUTO;
		}

		إذا (b->object || (b->flags & REPLACE_DIM)) {
			إذا (b->object) {
				صحيح عارضي_اوچائي = اوچائي;
				layout_get_object_dimensions(b,
						&ويڪر, &عارضي_اوچائي,
						INT_MIN, وڌ_کان_وڌ_عدد,
						INT_MIN, وڌ_کان_وڌ_عدد);
			}

			fixed = frac = 0;
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_ويڪر(&content->unit_len_ctx,
						b->style, کاٻو_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_ويڪر(&content->unit_len_ctx,
						b->style, ساڄو_2,
						true, false, false,
						&fixed, &frac);
			} وإلا {
				calculate_mbp_ويڪر(&content->unit_len_ctx,
						b->style, کاٻو_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_ويڪر(&content->unit_len_ctx,
						b->style, ساڄو_2,
						true, true, true,
						&fixed, &frac);
			}
			إذا (0 < ويڪر + fixed)
				ويڪر += fixed;
		} وإلا إذا (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			إذا (ويڪر == AUTO)
				ويڪر = 400;

			fixed = frac = 0;
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_ويڪر(&content->unit_len_ctx,
						b->style, کاٻو_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_ويڪر(&content->unit_len_ctx,
						b->style, ساڄو_2,
						true, false, false,
						&fixed, &frac);
			} وإلا {
				calculate_mbp_ويڪر(&content->unit_len_ctx,
						b->style, کاٻو_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_ويڪر(&content->unit_len_ctx,
						b->style, ساڄو_2,
						true, true, true,
						&fixed, &frac);
			}

			إذا (0 < ويڪر + fixed)
				ويڪر += fixed;

		} وإلا {
			/* form control with no object */
			إذا (ويڪر == AUTO)
				ويڪر = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		إذا (min < ويڪر && !box_has_percentage_max_ويڪر(b))
			min = ويڪر;
		إذا (ويڪر > 0)
			max += ويڪر;

		*سٽ_has_اوچائي = true;
	}

	إذا (پهريون_سٽ) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		صحيح متن_indent = layout_متن_indent(&content->unit_len_ctx,
				پهريون->والد->والد->style, 100);
		min = (min + متن_indent < 0) ? 0 : min + متن_indent;
		max = (max + متن_indent < 0) ? 0 : max + متن_indent;
	}

	*سٽ_min = min;
	*سٽ_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	أكد(b != پهريون);
	أكد(0 <= *سٽ_min);
	أكد(*سٽ_min <= *سٽ_max);
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
			       bool *has_اوچائي,
			       ثابت بنية gui_layout_جدول *font_func,
			       ثابت html_content *content)
{
	بنية box *ٻار;
	صحيح سٽ_min = 0, سٽ_max = 0;
	صحيح min = 0, max = 0;
	bool پهريون_سٽ = true;
	bool سٽ_has_اوچائي;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	إذا (inline_container->max_ويڪر != UNKNOWN_MAX_ويڪر)
		أرجع;

	*has_اوچائي = false;

	لكل (ٻار = inline_container->children; ٻار; ) {
		ٻار = layout_minmax_سٽ(ٻار, &سٽ_min, &سٽ_max,
				پهريون_سٽ, &سٽ_has_اوچائي, font_func,
				content);
		إذا (min < سٽ_min)
			min = سٽ_min;
		إذا (max < سٽ_max)
			max = سٽ_max;
		پهريون_سٽ = false;
		*has_اوچائي |= سٽ_has_اوچائي;
	}

	inline_container->min_ويڪر = min;
	inline_container->max_ويڪر = max;

	أكد(0 <= inline_container->min_ويڪر &&
			inline_container->min_ويڪر <=
			inline_container->max_ويڪر);
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
ساكن فراغ layout_minmax_بلاڪ(
		بنية box *بلاڪ,
		ثابت بنية gui_layout_جدول *font_func,
		ثابت html_content *content)
{
	بنية box *ٻار;
	صحيح min = 0, max = 0;
	صحيح extra_fixed = 0;
	عائم extra_frac = 0;
	تعداد css_ويڪر_e wtype = CSS_ويڪر_AUTO;
	css_fixed ويڪر = 0;
	css_unit wunit = CSS_UNIT_PX;
	تعداد css_اوچائي_e htype = CSS_اوچائي_AUTO;
	css_fixed اوچائي = 0;
	css_unit hunit = CSS_UNIT_PX;
	تعداد css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool ٻار_has_اوچائي = false;

	أكد(بلاڪ->type == BOX_بلاڪ ||
			بلاڪ->type == BOX_FLEX ||
			بلاڪ->type == BOX_INLINE_FLEX ||
			بلاڪ->type == BOX_INLINE_بلاڪ ||
			بلاڪ->type == BOX_جدول_CELL);

	/* check if the widths have already been calculated */
	إذا (بلاڪ->max_ويڪر != UNKNOWN_MAX_ويڪر)
		أرجع;

	إذا (بلاڪ->style != عدم) {
		wtype = css_computed_ويڪر(بلاڪ->style, &ويڪر, &wunit);
		htype = css_computed_اوچائي(بلاڪ->style, &اوچائي, &hunit);
		bs = css_computed_box_sizing(بلاڪ->style);
	}

	/* set whether the minimum width is of any interest for this box */
	إذا (((بلاڪ->والد && lh__box_is_float_box(بلاڪ->والد)) ||
			بلاڪ->type == BOX_INLINE_بلاڪ ||
			بلاڪ->type == BOX_INLINE_FLEX) &&
			wtype != CSS_ويڪر_سيٽ) {
		/* box shrinks to fit; need minimum width */
		بلاڪ->flags |= NEED_MIN;
	} وإلا إذا (بلاڪ->type == BOX_جدول_CELL) {
		/* box shrinks to fit; need minimum width */
		بلاڪ->flags |= NEED_MIN;
	} وإلا إذا (بلاڪ->والد && (بلاڪ->والد->flags & NEED_MIN) &&
			wtype != CSS_ويڪر_سيٽ) {
		/* box inside shrink-to-fit context; need minimum width */
		بلاڪ->flags |= NEED_MIN;
	} وإلا إذا (بلاڪ->والد && (بلاڪ->والد->type == BOX_FLEX)) {
		/* box is flex item */
		بلاڪ->flags |= NEED_MIN;
	}

	إذا (بلاڪ->gadget && (بلاڪ->gadget->type == GADGET_TEXTBOX ||
			بلاڪ->gadget->type == GADGET_PASSWORD ||
			بلاڪ->gadget->type == GADGET_FILE ||
			بلاڪ->gadget->type == GADGET_TEXTAREA) &&
			بلاڪ->style && wtype == CSS_ويڪر_AUTO) {
		css_fixed سائيز = INTTOFIX(10);
		css_unit unit = CSS_UNIT_EM;

		min = max = FIXTOINT(css_unit_len2device_px(بلاڪ->style,
				&content->unit_len_ctx, سائيز, unit));

		بلاڪ->flags |= HAS_اوچائي;
	}

	إذا (بلاڪ->gadget && (بلاڪ->gadget->type == GADGET_RADIO ||
			بلاڪ->gadget->type == GADGET_CHECKBOX) &&
			بلاڪ->style && wtype == CSS_ويڪر_AUTO) {
		css_fixed سائيز = INTTOFIX(1);
		css_unit unit = CSS_UNIT_EM;

		/* form checkbox or radio button
		 * if width is AUTO, set it to 1em */
		min = max = FIXTOINT(css_unit_len2device_px(بلاڪ->style,
				&content->unit_len_ctx, سائيز, unit));

		بلاڪ->flags |= HAS_اوچائي;
	}

	إذا (بلاڪ->object) {
		إذا (content_get_type(بلاڪ->object) == CONTENT_HTML) {
			layout_minmax_بلاڪ(html_get_box_وڻ(بلاڪ->object),
					font_func, content);
			min = html_get_box_وڻ(بلاڪ->object)->min_ويڪر;
			max = html_get_box_وڻ(بلاڪ->object)->max_ويڪر;
		} وإلا {
			min = max = content_get_ويڪر(بلاڪ->object);
		}

		بلاڪ->flags |= HAS_اوچائي;
	} وإلا إذا (بلاڪ->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		بلاڪ->flags |= HAS_اوچائي;
	} وإلا {
		/* recurse through children */
		لكل (ٻار = بلاڪ->children; ٻار; ٻار = ٻار->ايندڙ) {
			بدل (ٻار->type) {
			حالة BOX_FLEX:
			حالة BOX_بلاڪ:
				layout_minmax_بلاڪ(ٻار, font_func,
						content);
				إذا (ٻار->flags & HAS_اوچائي)
					ٻار_has_اوچائي = true;
				اكسر;
			حالة BOX_INLINE_CONTAINER:
				إذا (بلاڪ->flags & NEED_MIN)
					ٻار->flags |= NEED_MIN;

				layout_minmax_inline_container(ٻار,
						&ٻار_has_اوچائي, font_func,
						content);
				إذا (ٻار_has_اوچائي &&
						ٻار ==
						ٻار->والد->children) {
					بلاڪ->flags |= MAKE_اوچائي;
				}
				اكسر;
			حالة BOX_جدول:
				layout_minmax_جدول(ٻار, font_func,
						content);
				/* todo: fix for zero height tables */
				ٻار_has_اوچائي = true;
				ٻار->flags |= MAKE_اوچائي;
				اكسر;
			افتراضي:
				أكد(0);
			}
			أكد(ٻار->max_ويڪر != UNKNOWN_MAX_ويڪر);

			إذا (ٻار->style &&
					(css_computed_position(ٻار->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(ٻار->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				استمر;
			}

			إذا (lh__box_is_flex_container(بلاڪ) &&
			    lh__flex_main_is_horizontal(بلاڪ)) {
				إذا (بلاڪ->style != عدم &&
				    css_computed_flex_wrap(بلاڪ->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += ٻار->min_ويڪر;
				} وإلا {
					إذا (min < ٻار->min_ويڪر)
						min = ٻار->min_ويڪر;
				}
				max += ٻار->max_ويڪر;

			} وإلا {
				إذا (min < ٻار->min_ويڪر)
					min = ٻار->min_ويڪر;
				إذا (max < ٻار->max_ويڪر)
					max = ٻار->max_ويڪر;
			}

			إذا (ٻار_has_اوچائي)
				بلاڪ->flags |= HAS_اوچائي;
		}
	}

	إذا (max < min) {
		box_dump(stderr, بلاڪ, 0, true);
		أكد(0);
	}

	/* fixed width takes priority */
	إذا (بلاڪ->type != BOX_جدول_CELL && !lh_box_is_flex_شي(بلاڪ)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		تعداد css_max_ويڪر_e max_type;
		تعداد css_min_ويڪر_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed قدر = 0;
		صحيح ويڪر;

		إذا (css_computed_ويڪر_px(بلاڪ->style, &content->unit_len_ctx,
				-1, &ويڪر) == CSS_ويڪر_سيٽ) {
			min = max = ويڪر;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_ويڪر(بلاڪ->style, &قدر, &unit);
		إذا (min_type == CSS_MIN_ويڪر_سيٽ && unit != CSS_UNIT_PCT) {
			صحيح val = FIXTOINT(css_unit_len2device_px(بلاڪ->style,
					&content->unit_len_ctx, قدر, unit));

			إذا (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_ويڪر(بلاڪ->style, &قدر, &unit);
		إذا (max_type == CSS_MAX_ويڪر_سيٽ && unit != CSS_UNIT_PCT) {
			صحيح val = FIXTOINT(css_unit_len2device_px(بلاڪ->style,
					&content->unit_len_ctx, قدر, unit));

			إذا (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	إذا (htype == CSS_اوچائي_سيٽ && hunit != CSS_UNIT_PCT &&
			اوچائي > INTTOFIX(0)) {
		بلاڪ->flags |= MAKE_اوچائي;
		بلاڪ->flags |= HAS_اوچائي;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_ويڪر(&content->unit_len_ctx, بلاڪ->style, کاٻو_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_ويڪر(&content->unit_len_ctx, بلاڪ->style, ساڄو_2,
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

	calculate_mbp_ويڪر(&content->unit_len_ctx, بلاڪ->style, کاٻو_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_ويڪر(&content->unit_len_ctx, بلاڪ->style, ساڄو_2,
			true, false, false, &extra_fixed, &extra_frac);

	إذا (extra_fixed < 0)
		extra_fixed = 0;
	إذا (extra_frac < 0)
		extra_frac = 0;
	إذا (1.0 <= extra_frac)
		extra_frac = 0.9;
	إذا (بلاڪ->style != عدم &&
			(css_computed_float(بلاڪ->style) == CSS_FLOAT_کاٻو ||
			css_computed_float(بلاڪ->style) == CSS_FLOAT_ساڄو)) {
		/* floated boxs */
		بلاڪ->min_ويڪر = min + extra_fixed;
		بلاڪ->max_ويڪر = max + extra_fixed;
	} وإلا {
		/* not floated */
		بلاڪ->min_ويڪر = (min + extra_fixed) / (1.0 - extra_frac);
		بلاڪ->max_ويڪر = (max + extra_fixed) / (1.0 - extra_frac);
	}

	أكد(0 <= بلاڪ->min_ويڪر);
	أكد(بلاڪ->min_ويڪر <= بلاڪ->max_ويڪر);
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
layout_ايندڙ_margin_بلاڪ(ثابت css_unit_ctx *unit_len_ctx,
			 بنية box *box,
			 بنية box *بلاڪ,
			 صحيح viewport_اوچائي,
			 صحيح *max_pos_margin,
			 صحيح *max_neg_margin)
{
	أكد(بلاڪ != عدم);

	طالما (box != عدم) {

		إذا (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			إذا (box->style) {
				layout_ڳوليو_dimensions(unit_len_ctx,
						box->والد->ويڪر,
						viewport_اوچائي, box,
						box->style,
						عدم, عدم, عدم, عدم,
						عدم, عدم, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				إذا (*max_pos_margin < box->margin[چوٽي_2])
					*max_pos_margin = box->margin[چوٽي_2];
				وإلا إذا (*max_neg_margin < -box->margin[چوٽي_2])
					*max_neg_margin = -box->margin[چوٽي_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			إذا (box->flags & MAKE_اوچائي ||
					box->border[چوٽي_2].ويڪر ||
					box->padding[چوٽي_2] ||
					(box->style &&
					css_computed_حد_کان_مٿي_y(box->style) !=
					CSS_حد_کان_مٿي_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_پهريون_ٻار(box))) {
				/* Collapse to this box; return it */
				أرجع box;
			}
		}


		/* Find next box */
		إذا (box->type == BOX_بلاڪ && !box->object && box->children &&
				box->style &&
				css_computed_حد_کان_مٿي_y(box->style) ==
				CSS_حد_کان_مٿي_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} وإلا {
			إذا (!box->ايندڙ) {
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

					box = box->والد;
				} طالما (box != بلاڪ && !box->ايندڙ);

				إذا (box == بلاڪ) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					أرجع بلاڪ;
				}
			}

			/* Apply bottom margin */
			إذا (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			وإلا إذا (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->ايندڙ;

			/* Get margins */
			إذا (box->style) {
				layout_ڳوليو_dimensions(unit_len_ctx,
						box->والد->ويڪر,
						viewport_اوچائي, box,
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
	لكل (; fl; fl = fl->ايندڙ_float) {
		إذا ((clear == CSS_CLEAR_کاٻو || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_کاٻو)
			إذا (y < fl->y + fl->اوچائي)
				y = fl->y + fl->اوچائي;
		إذا ((clear == CSS_CLEAR_ساڄو || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ساڄو)
			إذا (y < fl->y + fl->اوچائي)
				y = fl->y + fl->اوچائي;
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
ڳوليو_sides(بنية box *fl,
	   صحيح y0, صحيح y1,
	   صحيح *x0, صحيح *x1,
	   بنية box **کاٻو,
	   بنية box **ساڄو)
{
	صحيح fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*کاٻو = *ساڄو = 0;
	لكل (; fl; fl = fl->ايندڙ_float) {
		fy1 = fl->y + fl->اوچائي;
		إذا (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			أرجع;
		}
		fy0 = fl->y;
		إذا (y0 < fy1 && fy0 <= y1) {
			إذا (fl->type == BOX_FLOAT_کاٻو) {
				fx1 = fl->x + fl->ويڪر;
				إذا (*x0 < fx1) {
					*x0 = fx1;
					*کاٻو = fl;
				}
			} وإلا {
				fx0 = fl->x;
				إذا (fx0 < *x1) {
					*x1 = fx0;
					*ساڄو = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *کاٻو, *ساڄو);
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
layout_solve_ويڪر(بنية box *box,
		   صحيح موجود_ويڪر,
		   صحيح ويڪر,
		   صحيح lm,
		   صحيح rm,
		   صحيح max_ويڪر,
		   صحيح min_ويڪر)
{
	bool auto_ويڪر = false;

	/* Increase specified left/right margins */
	إذا (box->margin[کاٻو_2] != AUTO && box->margin[کاٻو_2] < lm &&
			box->margin[کاٻو_2] >= 0)
		box->margin[کاٻو_2] = lm;
	إذا (box->margin[ساڄو_2] != AUTO && box->margin[ساڄو_2] < rm &&
			box->margin[ساڄو_2] >= 0)
		box->margin[ساڄو_2] = rm;

	/* Find width */
	إذا (ويڪر == AUTO) {
		صحيح margin_کاٻو = box->margin[کاٻو_2];
		صحيح margin_ساڄو = box->margin[ساڄو_2];

		إذا (margin_کاٻو == AUTO) {
			margin_کاٻو = lm;
		}
		إذا (margin_ساڄو == AUTO) {
			margin_ساڄو = rm;
		}

		ويڪر = موجود_ويڪر -
				(margin_کاٻو + box->border[کاٻو_2].ويڪر +
				box->padding[کاٻو_2] + box->padding[ساڄو_2] +
				box->border[ساڄو_2].ويڪر + margin_ساڄو);
		ويڪر = ويڪر < 0 ? 0 : ويڪر;
		auto_ويڪر = true;
	}

	إذا (max_ويڪر >= 0 && ويڪر > max_ويڪر) {
		/* max-width is admissable and width exceeds max-width */
		ويڪر = max_ويڪر;
		auto_ويڪر = false;
	}

	إذا (min_ويڪر > 0 && ويڪر < min_ويڪر) {
		/* min-width is admissable and width is less than max-width */
		ويڪر = min_ويڪر;
		auto_ويڪر = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	إذا (auto_ويڪر) {
		/* any other 'auto' become 0 or the minimum required values */
		إذا (box->margin[کاٻو_2] == AUTO) {
			box->margin[کاٻو_2] = lm;
		}
		إذا (box->margin[ساڄو_2] == AUTO) {
			box->margin[ساڄو_2] = rm;
		}
		أرجع ويڪر;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	إذا (box->margin[کاٻو_2] != AUTO && box->margin[ساڄو_2] != AUTO &&
			box->والد != عدم && box->والد->style != عدم) {
		بدل (css_computed_متن_align(box->والد->style)) {
		حالة CSS_متن_ALIGN_LIBCSS_ساڄو:
			box->margin[کاٻو_2] = AUTO;
			box->margin[ساڄو_2] = 0;
			اكسر;
		حالة CSS_متن_ALIGN_LIBCSS_CENTER:
			box->margin[کاٻو_2] = box->margin[ساڄو_2] = AUTO;
			اكسر;
		حالة CSS_متن_ALIGN_LIBCSS_کاٻو:
			box->margin[کاٻو_2] = 0;
			box->margin[ساڄو_2] = AUTO;
			اكسر;
		افتراضي:
			/* Leave it alone; no HTML alignment */
			اكسر;
		}
	}

	إذا (box->margin[کاٻو_2] == AUTO && box->margin[ساڄو_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[کاٻو_2] = box->margin[ساڄو_2] =
				(موجود_ويڪر - lm - rm -
				(box->border[کاٻو_2].ويڪر + box->padding[کاٻو_2] +
				ويڪر + box->padding[ساڄو_2] +
				box->border[ساڄو_2].ويڪر)) / 2;

		إذا (box->margin[کاٻو_2] < 0) {
			box->margin[ساڄو_2] += box->margin[کاٻو_2];
			box->margin[کاٻو_2] = 0;
		}

		box->margin[کاٻو_2] += lm;

	} وإلا إذا (box->margin[کاٻو_2] == AUTO) {
		box->margin[کاٻو_2] = موجود_ويڪر - lm -
				(box->border[کاٻو_2].ويڪر + box->padding[کاٻو_2] +
				ويڪر + box->padding[ساڄو_2] +
				box->border[ساڄو_2].ويڪر + box->margin[ساڄو_2]);
		box->margin[کاٻو_2] = box->margin[کاٻو_2] < lm
				? lm : box->margin[کاٻو_2];
	} وإلا {
		/* margin-right auto or "over-constrained" */
		box->margin[ساڄو_2] = موجود_ويڪر - rm -
				(box->margin[کاٻو_2] + box->border[کاٻو_2].ويڪر +
				 box->padding[کاٻو_2] + ويڪر +
				 box->padding[ساڄو_2] +
				 box->border[ساڄو_2].ويڪر);
	}

	أرجع ويڪر;
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
layout_بلاڪ_ڳوليو_dimensions(ثابت css_unit_ctx *unit_len_ctx,
			     صحيح موجود_ويڪر,
			     صحيح viewport_اوچائي,
			     صحيح lm,
			     صحيح rm,
			     بنية box *box)
{
	صحيح ويڪر, max_ويڪر, min_ويڪر;
	صحيح اوچائي, max_اوچائي, min_اوچائي;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	ثابت css_computed_style *style = box->style;

	layout_ڳوليو_dimensions(unit_len_ctx, موجود_ويڪر, viewport_اوچائي, box,
			style, &ويڪر, &اوچائي, &max_ويڪر, &min_ويڪر,
			&max_اوچائي, &min_اوچائي, margin, padding, border);

	إذا (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &ويڪر, &اوچائي,
				min_ويڪر, max_ويڪر, min_اوچائي, max_اوچائي);
	}

	box->ويڪر = layout_solve_ويڪر(box, موجود_ويڪر, ويڪر, lm, rm,
			max_ويڪر, min_ويڪر);
	box->اوچائي = اوچائي;

	إذا (margin[چوٽي_2] == AUTO)
		margin[چوٽي_2] = 0;
	إذا (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
ساكن فراغ layout_بلاڪ_جوڙيو_scrollbar(بنية box *box, صحيح which)
{
	تعداد css_حد_کان_مٿي_e حد_کان_مٿي_x, حد_کان_مٿي_y;

	أكد(box->type == BOX_بلاڪ && (which == ساڄو_2 || which == BOTTOM));

	إذا (box->style == عدم)
		أرجع;

	حد_کان_مٿي_x = css_computed_حد_کان_مٿي_x(box->style);
	حد_کان_مٿي_y = css_computed_حد_کان_مٿي_y(box->style);

	إذا (which == BOTTOM &&
			(حد_کان_مٿي_x == CSS_حد_کان_مٿي_SCROLL ||
			 حد_کان_مٿي_x == CSS_حد_کان_مٿي_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		إذا (box->اوچائي != AUTO &&
				(حد_کان_مٿي_x == CSS_حد_کان_مٿي_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_ويڪر;
		}

	} وإلا إذا (which == ساڄو_2 &&
			(حد_کان_مٿي_y == CSS_حد_کان_مٿي_SCROLL ||
			 حد_کان_مٿي_y == CSS_حد_کان_مٿي_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		تعداد css_اوچائي_e htype;
		css_fixed اوچائي = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_اوچائي(box->style, &اوچائي, &hunit);

		إذا (which == ساڄو_2 && box->ويڪر != AUTO &&
				htype == CSS_اوچائي_سيٽ &&
				(حد_کان_مٿي_y == CSS_حد_کان_مٿي_SCROLL ||
				box_vscrollbar_present(box))) {
			box->ويڪر -= SCROLLBAR_ويڪر;
			box->padding[ساڄو_2] += SCROLLBAR_ويڪر;
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

	لكل (box = box->children; box; box = box->ايندڙ) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_جدول(
		بنية box *جدول,
		صحيح موجود_ويڪر,
		html_content *content)
{
	بنانشان صحيح columns = جدول->columns;  /* total columns */
	بنانشان صحيح i;
	بنانشان صحيح *صف_وسعت;
	صحيح *excess_y;
	صحيح جدول_ويڪر, min_ويڪر = 0, max_ويڪر = 0;
	صحيح required_ويڪر = 0;
	صحيح x, remainder = 0, ڳڻپ = 0;
	صحيح جدول_اوچائي = 0;
	صحيح min_اوچائي = 0;
	صحيح *xs;  /* array of column x positions */
	صحيح auto_ويڪر;
	صحيح اضافي_ويڪر;
	صحيح relative_جوڙ = 0;
	صحيح border_spacing_h = 0, border_spacing_v = 0;
	صحيح اضافي_اوچائي;
	صحيح positioned_columns = 0;
	بنية box *containing_بلاڪ = عدم;
	بنية box *c;
	بنية box *صف;
	بنية box *صف_گروهه;
	بنية box **صف_وسعت_cell;
	بنية ڪالم *کانو;
	ثابت css_computed_style *style = جدول->style;
	تعداد css_اوچائي_e htype;
	css_fixed قدر = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(جدول->type == BOX_جدول);
	أكد(style);
	أكد(جدول->children && جدول->children->children);
	أكد(columns);

	/* allocate working buffers */
	کانو = يادگيري_ڏيو(columns * حجم کانو[0]);
	excess_y = يادگيري_ڏيو(columns * حجم excess_y[0]);
	صف_وسعت = يادگيري_ڏيو(columns * حجم صف_وسعت[0]);
	صف_وسعت_cell = يادگيري_ڏيو(columns * حجم صف_وسعت_cell[0]);
	xs = يادگيري_ڏيو((columns + 1) * حجم xs[0]);
	إذا (!کانو || !xs || !صف_وسعت || !excess_y || !صف_وسعت_cell) {
		حرر(کانو);
		حرر(excess_y);
		حرر(صف_وسعت);
		حرر(صف_وسعت_cell);
		حرر(xs);
		أرجع false;
	}

	يادگيري_نقل(کانو, جدول->کانو, حجم(کانو[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_ڳوليو_dimensions(&content->unit_len_ctx, موجود_ويڪر, -1, جدول,
			style, 0, 0, 0, 0, 0, 0, جدول->margin, جدول->padding,
			جدول->border);
	لكل (صف_گروهه = جدول->children; صف_گروهه;
			صف_گروهه = صف_گروهه->ايندڙ) {
		لكل (صف = صف_گروهه->children; صف; صف = صف->ايندڙ) {
			لكل (c = صف->children; c; c = c->ايندڙ) {
				تعداد css_حد_کان_مٿي_e حد_کان_مٿي_x;
				تعداد css_حد_کان_مٿي_e حد_کان_مٿي_y;

				أكد(c->style);
				جدول_استعمال_ٿيل_border_for_cell(
						&content->unit_len_ctx, c);
				layout_ڳوليو_dimensions(&content->unit_len_ctx,
						موجود_ويڪر, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				حد_کان_مٿي_x = css_computed_حد_کان_مٿي_x(c->style);
				حد_کان_مٿي_y = css_computed_حد_کان_مٿي_y(c->style);

				إذا (حد_کان_مٿي_x == CSS_حد_کان_مٿي_SCROLL ||
						حد_کان_مٿي_x ==
						CSS_حد_کان_مٿي_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_ويڪر;
				}
				إذا (حد_کان_مٿي_y == CSS_حد_کان_مٿي_SCROLL ||
						حد_کان_مٿي_y ==
						CSS_حد_کان_مٿي_AUTO) {
					c->padding[ساڄو_2] += SCROLLBAR_ويڪر;
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
	إذا (css_computed_ويڪر_px(style, &content->unit_len_ctx,
			موجود_ويڪر, &جدول_ويڪر) == CSS_ويڪر_سيٽ) {
		/* specified width includes border */
		جدول_ويڪر -= جدول->border[کاٻو_2].ويڪر +
				جدول->border[ساڄو_2].ويڪر;
		جدول_ويڪر = جدول_ويڪر < 0 ? 0 : جدول_ويڪر;

		auto_ويڪر = جدول_ويڪر;
	} وإلا {
		جدول_ويڪر = AUTO;
		auto_ويڪر = موجود_ويڪر -
				((جدول->margin[کاٻو_2] == AUTO ? 0 :
						جدول->margin[کاٻو_2]) +
				 جدول->border[کاٻو_2].ويڪر +
				 جدول->padding[کاٻو_2] +
				 جدول->padding[ساڄو_2] +
				 جدول->border[ساڄو_2].ويڪر +
				 (جدول->margin[ساڄو_2] == AUTO ? 0 :
						جدول->margin[ساڄو_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_اوچائي(style, &قدر, &unit);
	إذا (htype == CSS_اوچائي_سيٽ) {
		إذا (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			إذا (css_computed_position(جدول->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				أكد(جدول->float_container);
				containing_بلاڪ = جدول->float_container;
			} وإلا إذا (جدول->float_container &&
					css_computed_position(جدول->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(جدول->style) ==
						CSS_FLOAT_کاٻو ||
					css_computed_float(جدول->style) ==
						CSS_FLOAT_ساڄو)) {
				/* Table is a float */
				أكد(جدول->والد && جدول->والد->والد &&
						جدول->والد->والد->والد);
				containing_بلاڪ =
						جدول->والد->والد->والد;
			} وإلا إذا (جدول->والد && جدول->والد->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_بلاڪ = جدول->والد;
			} وإلا إذا (جدول->والد && جدول->والد->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				أكد(جدول->والد->والد);
				containing_بلاڪ = جدول->والد->والد;
			}

			إذا (containing_بلاڪ) {
				css_fixed ignored = 0;

				htype = css_computed_اوچائي(
						containing_بلاڪ->style,
						&ignored, &unit);
			}

			إذا (containing_بلاڪ &&
					containing_بلاڪ->اوچائي != AUTO &&
					(css_computed_position(جدول->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_اوچائي_سيٽ)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_اوچائي = FPCT_جو_INT_TOINT(قدر,
						containing_بلاڪ->اوچائي);
			}
		} وإلا {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_اوچائي = FIXTOINT(css_unit_len2device_px(
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
				      })[کانو[i].type],
		      کانو[i].ويڪر,
		      کانو[i].min,
		      کانو[i].max);


		إذا (کانو[i].positioned) {
			positioned_columns++;
			استمر;
		} وإلا إذا (کانو[i].type == ڪالم_ويڪر_FIXED) {
			إذا (کانو[i].ويڪر < کانو[i].min)
				کانو[i].ويڪر = کانو[i].max = کانو[i].min;
			وإلا
				کانو[i].min = کانو[i].max = کانو[i].ويڪر;
			required_ويڪر += کانو[i].ويڪر;
		} وإلا إذا (کانو[i].type == ڪالم_ويڪر_PERCENT) {
			صحيح ويڪر = کانو[i].ويڪر * auto_ويڪر / 100;
			required_ويڪر += کانو[i].min < ويڪر ? ويڪر :
					کانو[i].min;
		} وإلا
			required_ويڪر += کانو[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_ويڪر);
	}
	required_ويڪر += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", جدول_ويڪر,
	      جدول->min_ويڪر, جدول->max_ويڪر, auto_ويڪر, required_ويڪر);

	إذا (auto_ويڪر < required_ويڪر) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		لكل (i = 0; i != columns; i++) {
			إذا (کانو[i].type == ڪالم_ويڪر_RELATIVE)
				استمر;
			إذا (کانو[i].type == ڪالم_ويڪر_PERCENT) {
				کانو[i].max = auto_ويڪر * کانو[i].ويڪر / 100;
				إذا (کانو[i].max < کانو[i].min)
					کانو[i].max = کانو[i].min;
			}
			min_ويڪر += کانو[i].min;
			max_ويڪر += کانو[i].max;
		}
	} وإلا {
		/* take percentages exactly */
		لكل (i = 0; i != columns; i++) {
			إذا (کانو[i].type == ڪالم_ويڪر_RELATIVE)
				استمر;
			إذا (کانو[i].type == ڪالم_ويڪر_PERCENT) {
				صحيح ويڪر = auto_ويڪر * کانو[i].ويڪر / 100;
				إذا (ويڪر < کانو[i].min)
					ويڪر = کانو[i].min;
				کانو[i].min = کانو[i].ويڪر = کانو[i].max = ويڪر;
				کانو[i].type = ڪالم_ويڪر_FIXED;
			}
			min_ويڪر += کانو[i].min;
			max_ويڪر += کانو[i].max;
		}
	}

	/* allocate relative widths */
	اضافي_ويڪر = auto_ويڪر;
	لكل (i = 0; i != columns; i++) {
		إذا (کانو[i].type == ڪالم_ويڪر_RELATIVE)
			relative_جوڙ += کانو[i].ويڪر;
		وإلا إذا (کانو[i].type == ڪالم_ويڪر_FIXED)
			اضافي_ويڪر -= کانو[i].ويڪر;
		وإلا
			اضافي_ويڪر -= کانو[i].min;
	}
	اضافي_ويڪر -= (columns + 1) * border_spacing_h;
	إذا (relative_جوڙ != 0) {
		إذا (اضافي_ويڪر < 0)
			اضافي_ويڪر = 0;
		لكل (i = 0; i != columns; i++) {
			إذا (کانو[i].type == ڪالم_ويڪر_RELATIVE) {
				کانو[i].min = ceil(کانو[i].max =
						(عائم) اضافي_ويڪر
						* (عائم) کانو[i].ويڪر
						/ relative_جوڙ);
				min_ويڪر += کانو[i].min;
				max_ويڪر += کانو[i].max;
			}
		}
	}
	min_ويڪر += (columns + 1) * border_spacing_h;
	max_ويڪر += (columns + 1) * border_spacing_h;

	إذا (auto_ويڪر <= min_ويڪر) {
		/* not enough space: minimise column widths */
		لكل (i = 0; i < columns; i++) {
			کانو[i].ويڪر = کانو[i].min;
		}
		جدول_ويڪر = min_ويڪر;
	} وإلا إذا (max_ويڪر <= auto_ويڪر) {
		/* more space than maximum width */
		إذا (جدول_ويڪر == AUTO) {
			/* for auto-width tables, make columns max width */
			لكل (i = 0; i < columns; i++) {
				کانو[i].ويڪر = کانو[i].max;
			}
			جدول_ويڪر = max_ويڪر;
		} وإلا {
			/* for fixed-width tables, distribute the extra space
			 * too */
			بنانشان صحيح flexible_columns = 0;
			لكل (i = 0; i != columns; i++)
				إذا (کانو[i].type != ڪالم_ويڪر_FIXED)
					flexible_columns++;
			إذا (flexible_columns == 0) {
				صحيح extra = (جدول_ويڪر - max_ويڪر) / columns;
				remainder = (جدول_ويڪر - max_ويڪر) -
						(extra * columns);
				لكل (i = 0; i != columns; i++) {
					کانو[i].ويڪر = کانو[i].max + extra;
					ڳڻپ -= remainder;
					إذا (ڳڻپ < 0) {
						کانو[i].ويڪر++;
						ڳڻپ += columns;
					}
				}

			} وإلا {
				صحيح extra = (جدول_ويڪر - max_ويڪر) /
						flexible_columns;
				remainder = (جدول_ويڪر - max_ويڪر) -
						(extra * flexible_columns);
				لكل (i = 0; i != columns; i++)
					إذا (کانو[i].type != ڪالم_ويڪر_FIXED) {
						کانو[i].ويڪر = کانو[i].max +
								extra;
						ڳڻپ -= remainder;
						إذا (ڳڻپ < 0) {
							کانو[i].ويڪر++;
							ڳڻپ += flexible_columns;
						}
					}
			}
		}
	} وإلا {
		/* space between min and max: fill it exactly */
		عائم scale = (عائم) (auto_ويڪر - min_ويڪر) /
				(عائم) (max_ويڪر - min_ويڪر);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		لكل (i = 0; i < columns; i++) {
			کانو[i].ويڪر = کانو[i].min + (صحيح) (0.5 +
					(کانو[i].max - کانو[i].min) * scale);
		}
		جدول_ويڪر = auto_ويڪر;
	}

	xs[0] = x = border_spacing_h;
	لكل (i = 0; i != columns; i++) {
		إذا (!کانو[i].positioned)
			x += کانو[i].ويڪر + border_spacing_h;
		xs[i + 1] = x;
		صف_وسعت[i] = 0;
		excess_y[i] = 0;
		صف_وسعت_cell[i] = 0;
	}

	/* position cells */
	جدول_اوچائي = border_spacing_v;
	لكل (صف_گروهه = جدول->children; صف_گروهه;
			صف_گروهه = صف_گروهه->ايندڙ) {
		صحيح صف_گروهه_اوچائي = 0;
		لكل (صف = صف_گروهه->children; صف; صف = صف->ايندڙ) {
			صحيح صف_اوچائي = 0;

			htype = css_computed_اوچائي(صف->style, &قدر, &unit);
			إذا (htype == CSS_اوچائي_سيٽ && unit != CSS_UNIT_PCT) {
				صف_اوچائي = FIXTOINT(css_unit_len2device_px(
						صف->style,
						&content->unit_len_ctx,
						قدر, unit));
			}
			لكل (c = صف->children; c; c = c->ايندڙ) {
				أكد(c->style);
				c->ويڪر = xs[c->شروعات_ڪالم + c->columns] -
						xs[c->شروعات_ڪالم] -
						border_spacing_h -
						c->border[کاٻو_2].ويڪر -
						c->padding[کاٻو_2] -
						c->padding[ساڄو_2] -
						c->border[ساڄو_2].ويڪر;
				c->float_children = 0;
				c->cached_رکو_هيٺ_level = 0;

				c->اوچائي = AUTO;
				إذا (!layout_بلاڪ_context(c, -1, content)) {
					حرر(کانو);
					حرر(excess_y);
					حرر(صف_وسعت);
					حرر(صف_وسعت_cell);
					حرر(xs);
					أرجع false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->اوچائي;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_اوچائي(c->style,
						&قدر, &unit);

				إذا (htype == CSS_اوچائي_سيٽ &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					صحيح h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							قدر, unit));
					إذا (c->اوچائي < h)
						c->اوچائي = h;
				}
				/* specified row height is treated as a minimum
				 */
				إذا (c->اوچائي < صف_اوچائي)
					c->اوچائي = صف_اوچائي;
				c->x = xs[c->شروعات_ڪالم] +
						c->border[کاٻو_2].ويڪر;
				c->y = c->border[چوٽي_2].ويڪر;
				لكل (i = 0; i != c->columns; i++) {
					صف_وسعت[c->شروعات_ڪالم + i] = c->صفون;
					excess_y[c->شروعات_ڪالم + i] =
							c->border[چوٽي_2].ويڪر +
							c->padding[چوٽي_2] +
							c->اوچائي +
							c->padding[BOTTOM] +
							c->border[BOTTOM].ويڪر;
					صف_وسعت_cell[c->شروعات_ڪالم + i] = 0;
				}
				صف_وسعت_cell[c->شروعات_ڪالم] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[چوٽي_2].ويڪر -
						c->padding[چوٽي_2] -
						c->اوچائي -
						c->border[BOTTOM].ويڪر;
			}
			لكل (i = 0; i != columns; i++)
				إذا (صف_وسعت[i] != 0)
					صف_وسعت[i]--;
				وإلا
					صف_وسعت_cell[i] = 0;
			إذا (صف->ايندڙ || صف_گروهه->ايندڙ) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				لكل (i = 0; i != columns; i++)
					إذا (صف_وسعت[i] == 0 && صف_اوچائي <
							excess_y[i])
						صف_اوچائي = excess_y[i];
			} وإلا {
				/* except in the last row */
				لكل (i = 0; i != columns; i++)
					إذا (صف_اوچائي < excess_y[i])
						صف_اوچائي = excess_y[i];
			}
			لكل (i = 0; i != columns; i++) {
				إذا (صف_اوچائي < excess_y[i])
					excess_y[i] -= صف_اوچائي;
				وإلا
					excess_y[i] = 0;
				إذا (صف_وسعت_cell[i] != 0)
					صف_وسعت_cell[i]->padding[BOTTOM] +=
							صف_اوچائي +
							border_spacing_v;
			}

			صف->x = 0;
			صف->y = صف_گروهه_اوچائي;
			صف->ويڪر = جدول_ويڪر;
			صف->اوچائي = صف_اوچائي;
			صف_گروهه_اوچائي += صف_اوچائي + border_spacing_v;
		}
		صف_گروهه->x = 0;
		صف_گروهه->y = جدول_اوچائي;
		صف_گروهه->ويڪر = جدول_ويڪر;
		صف_گروهه->اوچائي = صف_گروهه_اوچائي;
		جدول_اوچائي += صف_گروهه_اوچائي;
	}
	/* Table height is either the height of the contents, or specified
	 * height if greater */
	جدول_اوچائي = max(جدول_اوچائي, min_اوچائي);
	/** \todo distribute spare height over the row groups / rows / cells */

	/* perform vertical alignment */
	لكل (صف_گروهه = جدول->children; صف_گروهه;
			صف_گروهه = صف_گروهه->ايندڙ) {
		لكل (صف = صف_گروهه->children; صف; صف = صف->ايندڙ) {
			لكل (c = صف->children; c; c = c->ايندڙ) {
				تعداد css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				اضافي_اوچائي = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->اوچائي - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &قدر, &unit);

				بدل (vertical_align) {
				حالة CSS_VERTICAL_ALIGN_SUB:
				حالة CSS_VERTICAL_ALIGN_SUPER:
				حالة CSS_VERTICAL_ALIGN_متن_چوٽي:
				حالة CSS_VERTICAL_ALIGN_متن_BOTTOM:
				حالة CSS_VERTICAL_ALIGN_سيٽ:
				حالة CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				حالة CSS_VERTICAL_ALIGN_چوٽي:
					اكسر;
				حالة CSS_VERTICAL_ALIGN_وچ:
					c->padding[چوٽي_2] += اضافي_اوچائي / 2;
					c->padding[BOTTOM] -= اضافي_اوچائي / 2;
					layout_move_children(c, 0,
							اضافي_اوچائي / 2);
					اكسر;
				حالة CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[چوٽي_2] += اضافي_اوچائي;
					c->padding[BOTTOM] -= اضافي_اوچائي;
					layout_move_children(c, 0,
							اضافي_اوچائي);
					اكسر;
				حالة CSS_VERTICAL_ALIGN_INHERIT:
					أكد(0);
					اكسر;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	إذا (جدول->margin[چوٽي_2] == AUTO)
		جدول->margin[چوٽي_2] = 0;
	إذا (جدول->margin[BOTTOM] == AUTO)
		جدول->margin[BOTTOM] = 0;

	حرر(کانو);
	حرر(excess_y);
	حرر(صف_وسعت);
	حرر(صف_وسعت_cell);
	حرر(xs);

	جدول->ويڪر = جدول_ويڪر;
	جدول->اوچائي = جدول_اوچائي;

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
ساكن bool layout_apply_minmax_اوچائي(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *box,
		بنية box *container)
{
	صحيح h;
	بنية box *containing_بلاڪ = عدم;
	bool updated = false;

	/* Find containing block for percentage heights */
	إذا (box->style != عدم && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		أكد(container);
		containing_بلاڪ = container;
	} وإلا إذا (box->float_container && box->style != عدم &&
			(css_computed_float(box->style) == CSS_FLOAT_کاٻو ||
			 css_computed_float(box->style) == CSS_FLOAT_ساڄو)) {
		/* Box is a float */
		أكد(box->والد && box->والد->والد &&
				box->والد->والد->والد);
		containing_بلاڪ = box->والد->والد->والد;
	} وإلا إذا (box->والد && box->والد->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_بلاڪ = box->والد;
	} وإلا إذا (box->والد && box->والد->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		أكد(box->والد->والد);
		containing_بلاڪ = box->والد->والد;
	}

	إذا (box->style) {
		تعداد css_اوچائي_e htype = CSS_اوچائي_AUTO;
		css_fixed قدر = 0;
		css_unit unit = CSS_UNIT_PX;

		إذا (containing_بلاڪ) {
			htype = css_computed_اوچائي(containing_بلاڪ->style,
					&قدر, &unit);
		}

		/* max-height */
		إذا (css_computed_max_اوچائي(box->style, &قدر, &unit) ==
				CSS_MAX_اوچائي_سيٽ) {
			إذا (unit == CSS_UNIT_PCT) {
				إذا (containing_بلاڪ &&
					containing_بلاڪ->اوچائي != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_اوچائي_سيٽ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_جو_INT_TOINT(قدر,
						containing_بلاڪ->اوچائي);
					إذا (h < box->اوچائي) {
						box->اوچائي = h;
						updated = true;
					}
				}
			} وإلا {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						قدر, unit));
				إذا (h < box->اوچائي) {
					box->اوچائي = h;
					updated = true;
				}
			}
		}

		/* min-height */
		إذا (ns_computed_min_اوچائي(box->style, &قدر, &unit) ==
				CSS_MIN_اوچائي_سيٽ) {
			إذا (unit == CSS_UNIT_PCT) {
				إذا (containing_بلاڪ &&
					containing_بلاڪ->اوچائي != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_اوچائي_سيٽ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_جو_INT_TOINT(قدر,
						containing_بلاڪ->اوچائي);
					إذا (h > box->اوچائي) {
						box->اوچائي = h;
						updated = true;
					}
				}
			} وإلا {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						قدر, unit));
				إذا (h > box->اوچائي) {
					box->اوچائي = h;
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
ساكن bool layout_بلاڪ_object(بنية box *بلاڪ)
{
	أكد(بلاڪ);
	أكد(بلاڪ->type == BOX_بلاڪ ||
			بلاڪ->type == BOX_FLEX ||
			بلاڪ->type == BOX_INLINE_بلاڪ ||
			بلاڪ->type == BOX_INLINE_FLEX ||
			بلاڪ->type == BOX_جدول ||
			بلاڪ->type == BOX_جدول_CELL);
	أكد(بلاڪ->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", بلاڪ,
	      hlcache_handle_get_url(بلاڪ->object), بلاڪ->ويڪر);

	إذا (content_can_reformat(بلاڪ->object)) {
		content_reformat(بلاڪ->object, false, بلاڪ->ويڪر, 1);
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
ساكن فراغ جوڙيو_float_منزل_container(بنية box *cont, بنية box *b)
{
	بنية box *box = cont->float_children;
	صحيح b_bottom = b->y + b->اوچائي;

	أكد(b->type == BOX_FLOAT_کاٻو || b->type == BOX_FLOAT_ساڄو);

	إذا (box == عدم) {
		/* No other float children */
		b->ايندڙ_float = عدم;
		cont->float_children = b;
		أرجع;
	} وإلا إذا (b_bottom >= box->y + box->اوچائي) {
		/* Goes at start of list */
		b->ايندڙ_float = cont->float_children;
		cont->float_children = b;
	} وإلا {
		بنية box *prev = عدم;
		طالما (box != عدم && b_bottom < box->y + box->اوچائي) {
			prev = box;
			box = box->ايندڙ_float;
		}
		إذا (prev != عدم) {
			b->ايندڙ_float = prev->ايندڙ_float;
			prev->ايندڙ_float = b;
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
layout_متن_box_ورهايو(html_content *content,
		      plot_font_style_t *fstyle,
		      بنية box *ورهايو_box,
		      size_t new_ڊيگهائي,
		      صحيح new_ويڪر)
{
	صحيح space_ويڪر = ورهايو_box->space;
	بنية box *c2;
	ثابت بنية gui_layout_جدول *font_func = content->font_func;
	bool space = (ورهايو_box->متن[new_ڊيگهائي] == ' ');
	صحيح استعمال_ٿيل_ڊيگهائي = new_ڊيگهائي + (space ? 1 : 0);

	إذا ((space && space_ويڪر == 0) || space_ويڪر == UNKNOWN_ويڪر) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->ويڪر(fstyle, " ", 1, &space_ويڪر);
	}

	إذا (ورهايو_box->space == UNKNOWN_ويڪر)
		ورهايو_box->space = space_ويڪر;
	إذا (!space)
		space_ويڪر = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, ورهايو_box, حجم *c2);
	إذا (!c2)
		أرجع false;
	c2->flags |= CLONE;

	/* Set remaining text in c2 */
	c2->متن += استعمال_ٿيل_ڊيگهائي;

	/* Set c2 according to the remaining text */
	c2->ويڪر -= new_ويڪر + space_ويڪر;
	c2->flags &= ~MEASURED; /* width has been estimated */
	c2->ڊيگهائي = ورهايو_box->ڊيگهائي - استعمال_ٿيل_ڊيگهائي;

	/* Update split_box for its reduced text */
	ورهايو_box->ويڪر = new_ويڪر;
	ورهايو_box->flags |= MEASURED;
	ورهايو_box->ڊيگهائي = new_ڊيگهائي;
	ورهايو_box->space = space_ويڪر;

	/* Insert c2 into box list */
	c2->ايندڙ = ورهايو_box->ايندڙ;
	ورهايو_box->ايندڙ = c2;
	c2->prev = ورهايو_box;
	إذا (c2->ايندڙ)
		c2->ايندڙ->prev = c2;
	وإلا
		c2->والد->آخري = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      ورهايو_box,
	      ورهايو_box->ڊيگهائي,
	      (صحيح)ورهايو_box->ڊيگهائي,
	      ورهايو_box->متن);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->ڊيگهائي,
	      (صحيح)c2->ڊيگهائي,
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
layout_float_ڳوليو_dimensions(
		ثابت css_unit_ctx *unit_len_ctx,
		صحيح موجود_ويڪر,
		ثابت css_computed_style *style,
		بنية box *box)
{
	صحيح ويڪر, اوچائي, max_ويڪر, min_ويڪر, max_اوچائي, min_اوچائي;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	تعداد css_حد_کان_مٿي_e حد_کان_مٿي_x = css_computed_حد_کان_مٿي_x(style);
	تعداد css_حد_کان_مٿي_e حد_کان_مٿي_y = css_computed_حد_کان_مٿي_y(style);
	صحيح scrollbar_ويڪر_x =
			(حد_کان_مٿي_x == CSS_حد_کان_مٿي_SCROLL ||
			 حد_کان_مٿي_x == CSS_حد_کان_مٿي_AUTO) ?
			SCROLLBAR_ويڪر : 0;
	صحيح scrollbar_ويڪر_y =
			(حد_کان_مٿي_y == CSS_حد_کان_مٿي_SCROLL ||
			 حد_کان_مٿي_y == CSS_حد_کان_مٿي_AUTO) ?
			SCROLLBAR_ويڪر : 0;

	layout_ڳوليو_dimensions(unit_len_ctx, موجود_ويڪر, -1, box, style,
			&ويڪر, &اوچائي, &max_ويڪر, &min_ويڪر,
			&max_اوچائي, &min_اوچائي, margin, padding, border);

	إذا (margin[کاٻو_2] == AUTO)
		margin[کاٻو_2] = 0;
	إذا (margin[ساڄو_2] == AUTO)
		margin[ساڄو_2] = 0;

	إذا (box->gadget == عدم) {
		padding[ساڄو_2] += scrollbar_ويڪر_y;
		padding[BOTTOM] += scrollbar_ويڪر_x;
	}

	إذا (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &ويڪر, &اوچائي,
				min_ويڪر, max_ويڪر, min_اوچائي, max_اوچائي);
	} وإلا إذا (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed سائيز = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		أكد(box->style);

		إذا (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			إذا (ويڪر == AUTO) {
				سائيز = INTTOFIX(10);
				ويڪر = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						سائيز, unit));
			}
			إذا (box->gadget->type == GADGET_FILE &&
					اوچائي == AUTO) {
				سائيز = FLTTOFIX(1.5);
				اوچائي = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						سائيز, unit));
			}
		}
		إذا (box->gadget->type == GADGET_TEXTAREA) {
			إذا (ويڪر == AUTO) {
				سائيز = INTTOFIX(10);
				ويڪر = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						سائيز, unit));
			}
			إذا (اوچائي == AUTO) {
				سائيز = INTTOFIX(4);
				اوچائي = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						سائيز, unit));
			}
		}
	} وإلا إذا (ويڪر == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		ويڪر = min(max(box->min_ويڪر, موجود_ويڪر),
				box->max_ويڪر);

		/* width includes margin, borders and padding */
		إذا (ويڪر == موجود_ويڪر) {
			ويڪر -= box->margin[کاٻو_2] + box->border[کاٻو_2].ويڪر +
					box->padding[کاٻو_2] +
					box->padding[ساڄو_2] +
					box->border[ساڄو_2].ويڪر +
					box->margin[ساڄو_2];
		} وإلا {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			صحيح fixed = 0;
			عائم frac = 0;
			calculate_mbp_ويڪر(unit_len_ctx, box->style, کاٻو_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_ويڪر(unit_len_ctx, box->style, ساڄو_2,
					true, true, true, &fixed, &frac);
			إذا (fixed < 0)
				fixed = 0;

			ويڪر -= fixed;
		}

		إذا (max_ويڪر >= 0 && ويڪر > max_ويڪر) ويڪر = max_ويڪر;
		إذا (min_ويڪر >  0 && ويڪر < min_ويڪر) ويڪر = min_ويڪر;

	} وإلا {
		إذا (max_ويڪر >= 0 && ويڪر > max_ويڪر) ويڪر = max_ويڪر;
		إذا (min_ويڪر >  0 && ويڪر < min_ويڪر) ويڪر = min_ويڪر;
		ويڪر -= scrollbar_ويڪر_y;
	}

	box->ويڪر = ويڪر;
	box->اوچائي = اوچائي;

	إذا (margin[چوٽي_2] == AUTO)
		margin[چوٽي_2] = 0;
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
ساكن bool layout_float(بنية box *b, صحيح ويڪر, html_content *content)
{
	أكد(b->type == BOX_جدول ||
	       b->type == BOX_بلاڪ ||
	       b->type == BOX_INLINE_بلاڪ ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_ڳوليو_dimensions(&content->unit_len_ctx, ويڪر, b->style, b);
	إذا (b->type == BOX_جدول || b->type == BOX_INLINE_FLEX) {
		إذا (b->type == BOX_جدول) {
			إذا (!layout_جدول(b, ويڪر, content))
				أرجع false;
		} وإلا {
			إذا (!layout_flex(b, ويڪر, content))
				أرجع false;
		}
		إذا (b->margin[کاٻو_2] == AUTO)
			b->margin[کاٻو_2] = 0;
		إذا (b->margin[ساڄو_2] == AUTO)
			b->margin[ساڄو_2] = 0;
		إذا (b->margin[چوٽي_2] == AUTO)
			b->margin[چوٽي_2] = 0;
		إذا (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} وإلا {
		أرجع layout_بلاڪ_context(b, -1, content);
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
رکو_float_هيٺ(بنية box *c, صحيح ويڪر, صحيح cx, صحيح y, بنية box *cont)
{
	صحيح x0, x1, yy;
	بنية box *کاٻو;
	بنية box *ساڄو;

	yy = y > cont->cached_رکو_هيٺ_level ?
			y : cont->cached_رکو_هيٺ_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      ويڪر, cx, y, cont);

	افعل {
		y = yy;
		x0 = cx;
		x1 = cx + ويڪر;
		ڳوليو_sides(cont->float_children, y, y + c->اوچائي, &x0, &x1,
				&کاٻو, &ساڄو);
		إذا (کاٻو != 0 && ساڄو != 0) {
			yy = (کاٻو->y + کاٻو->اوچائي <
					ساڄو->y + ساڄو->اوچائي ?
					کاٻو->y + کاٻو->اوچائي :
					ساڄو->y + ساڄو->اوچائي);
		} وإلا إذا (کاٻو == 0 && ساڄو != 0) {
			yy = ساڄو->y + ساڄو->اوچائي;
		} وإلا إذا (کاٻو != 0 && ساڄو == 0) {
			yy = کاٻو->y + کاٻو->اوچائي;
		}
	} طالما ((کاٻو != 0 || ساڄو != 0) && (c->ويڪر > x1 - x0));

	إذا (c->type == BOX_FLOAT_کاٻو) {
		c->x = x0;
	} وإلا {
		c->x = x1 - c->ويڪر;
	}
	c->y = y;
	cont->cached_رکو_هيٺ_level = y;
}


/**
 * Calculate line height from a style.
 */
ساكن صحيح سٽ_اوچائي(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style)
{
	تعداد css_سٽ_اوچائي_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed سٽ_اوچائي;

	أكد(style);

	lhtype = css_computed_سٽ_اوچائي(style, &lhvalue, &lhunit);
	إذا (lhtype == CSS_سٽ_اوچائي_عام) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_سٽ_اوچائي_عدد;
	}

	إذا (lhtype == CSS_سٽ_اوچائي_عدد ||
			lhunit == CSS_UNIT_PCT) {
		سٽ_اوچائي = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		إذا (lhtype != CSS_سٽ_اوچائي_عدد)
			سٽ_اوچائي = FDIV(سٽ_اوچائي, F_100);
	} وإلا {
		أكد(lhunit != CSS_UNIT_PCT);

		سٽ_اوچائي = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	أرجع FIXTOINT(سٽ_اوچائي);
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
layout_سٽ(بنية box *پهريون,
	    صحيح *ويڪر,
	    صحيح *y,
	    صحيح cx,
	    صحيح cy,
	    بنية box *cont,
	    bool indent,
	    bool has_متن_children,
	    html_content *content,
	    بنية box **ايندڙ_box)
{
	صحيح اوچائي, استعمال_ٿيل_اوچائي;
	صحيح x0 = 0;
	صحيح x1 = *ويڪر;
	صحيح x, h, x_اڳيون;
	صحيح fy = cy;
	بنية box *کاٻو;
	بنية box *ساڄو;
	بنية box *b;
	بنية box *ورهايو_box = 0;
	بنية box *d;
	بنية box *br_box = 0;
	bool move_y = false;
	bool رکو_هيٺ = false;
	صحيح space_before = 0, space_after = 0;
	بنانشان صحيح inline_ڳڻپ = 0;
	بنانشان صحيح i;
	ثابت بنية gui_layout_جدول *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      پهريون,
	      (صحيح)پهريون->ڊيگهائي,
	      پهريون->متن,
	      *ويڪر,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	ڳوليو_sides(cont->float_children, cy, cy, &x0, &x1, &کاٻو, &ساڄو);
	x0 -= cx;
	x1 -= cx;

	إذا (indent)
		x0 += layout_متن_indent(&content->unit_len_ctx,
				پهريون->والد->والد->style, *ويڪر);

	إذا (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	إذا (has_متن_children || پهريون->والد->والد->gadget)
		استعمال_ٿيل_اوچائي = اوچائي = سٽ_اوچائي(&content->unit_len_ctx,
				پهريون->والد->والد->style);
	وإلا
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		استعمال_ٿيل_اوچائي = اوچائي = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	لكل (x = 0, b = پهريون; x <= x1 - x0 && b != 0; b = b->ايندڙ) {
		صحيح min_ويڪر, max_ويڪر, min_اوچائي, max_اوچائي;

		أكد(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		إذا (b->type == BOX_BR)
			اكسر;

		إذا (lh__box_is_float_box(b))
			استمر;
		إذا (b->type == BOX_INLINE_بلاڪ &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			استمر;

		أكد(b->style != عدم);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		إذا (b->type == BOX_INLINE_بلاڪ ||
		    b->type == BOX_INLINE_FLEX) {
			إذا (b->max_ويڪر != UNKNOWN_ويڪر)
				إذا (!layout_float(b, *ويڪر, content))
					أرجع false;
			h = b->border[چوٽي_2].ويڪر + b->padding[چوٽي_2] + b->اوچائي +
					b->padding[BOTTOM] +
					b->border[BOTTOM].ويڪر;
			إذا (اوچائي < h)
				اوچائي = h;
			x += b->margin[کاٻو_2] + b->border[کاٻو_2].ويڪر +
					b->padding[کاٻو_2] + b->ويڪر +
					b->padding[ساڄو_2] +
					b->border[ساڄو_2].ويڪر +
					b->margin[ساڄو_2];
			space_after = 0;
			استمر;
		}

		إذا (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_ڳوليو_dimensions(&content->unit_len_ctx,
					*ويڪر, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			لكل (i = 0; i != 4; i++)
				إذا (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[کاٻو_2] + b->border[کاٻو_2].ويڪر +
					b->padding[کاٻو_2];
			إذا (b->inline_پڇاڙي) {
				b->inline_پڇاڙي->margin[ساڄو_2] = b->margin[ساڄو_2];
				b->inline_پڇاڙي->padding[ساڄو_2] =
						b->padding[ساڄو_2];
				b->inline_پڇاڙي->border[ساڄو_2] =
						b->border[ساڄو_2];
			} وإلا {
				x += b->padding[ساڄو_2] +
						b->border[ساڄو_2].ويڪر +
						b->margin[ساڄو_2];
			}
		} وإلا إذا (b->type == BOX_INLINE_پڇاڙي) {
			b->ويڪر = 0;
			إذا (b->space == UNKNOWN_ويڪر) {
				font_func->ويڪر(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[ساڄو_2] + b->border[ساڄو_2].ويڪر +
					b->margin[ساڄو_2];
			استمر;
		}

		إذا (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->اوچائي = سٽ_اوچائي(&content->unit_len_ctx,
					b->style ? b->style :
					b->والد->والد->style);
			إذا (اوچائي < b->اوچائي)
				اوچائي = b->اوچائي;

			إذا (!b->متن) {
				b->ويڪر = 0;
				space_after = 0;
				استمر;
			}

			إذا (b->ويڪر == UNKNOWN_ويڪر) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				إذا (b->والد->والد->gadget &&
						b->والد->والد->gadget->type
						== GADGET_SELECT) {
					صحيح opt_maxwidth = 0;
					بنية form_option *o;

					لكل (o = b->والد->والد->gadget->
							معلومات.select.شيون; o;
							o = o->ايندڙ) {
						صحيح opt_ويڪر;
						font_func->ويڪر(&fstyle,
								o->متن,
								لڙي_ڊيگهه(o->متن),
								&opt_ويڪر);

						إذا (opt_maxwidth < opt_ويڪر)
							opt_maxwidth =opt_ويڪر;
					}
					b->ويڪر = opt_maxwidth;
					إذا (nsoption_bool(core_select_menu))
						b->ويڪر += SCROLLBAR_ويڪر;
				} وإلا {
					font_func->ويڪر(&fstyle, b->متن,
							b->ڊيگهائي, &b->ويڪر);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			إذا (b->متن && (x + b->ويڪر < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->ايندڙ) {
				font_func->ويڪر(&fstyle, b->متن,
						 b->ڊيگهائي, &b->ويڪر);
				b->flags |= MEASURED;
			}

			x += b->ويڪر;
			إذا (b->space == UNKNOWN_ويڪر) {
				font_func->ويڪر(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			استمر;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		layout_ڳوليو_dimensions(&content->unit_len_ctx,
				*ويڪر, -1, b, b->style,
				&b->ويڪر, &b->اوچائي,
				&max_ويڪر, &min_ويڪر,
				&max_اوچائي, &min_اوچائي,
				عدم, عدم, عدم);

		إذا (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->ويڪر, &b->اوچائي,
					min_ويڪر, max_ويڪر,
					min_اوچائي, max_اوچائي);
		} وإلا إذا (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			إذا (b->ويڪر == AUTO)
				b->ويڪر = 400;
			إذا (b->اوچائي == AUTO)
				b->اوچائي = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} وإلا {
			/* form control with no object */
			إذا (b->ويڪر == AUTO)
				b->ويڪر = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			إذا (b->اوچائي == AUTO)
				b->اوچائي = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		إذا (b->object && content_can_reformat(b->object) &&
				b->ويڪر !=
				content_get_موجود_ويڪر(b->object)) {
			css_fixed قدر = 0;
			css_unit unit = CSS_UNIT_PX;
			تعداد css_اوچائي_e htype = css_computed_اوچائي(b->style,
					&قدر, &unit);

			content_reformat(b->object, false, b->ويڪر, b->اوچائي);

			إذا (htype == CSS_اوچائي_AUTO)
				b->اوچائي = content_get_اوچائي(b->object);
		}

		إذا (اوچائي < b->اوچائي)
			اوچائي = b->اوچائي;

		x += b->ويڪر;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *ويڪر;
	ڳوليو_sides(cont->float_children, cy, cy + اوچائي, &x0, &x1,
			&کاٻو, &ساڄو);
	x0 -= cx;
	x1 -= cx;

	إذا (indent)
		x0 += layout_متن_indent(&content->unit_len_ctx,
				پهريون->والد->والد->style, *ويڪر);

	إذا (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	لكل (x = x_اڳيون = 0, b = پهريون; x <= x1 - x0 && b; b = b->ايندڙ) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		إذا (b->type == BOX_INLINE_بلاڪ &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} وإلا إذا (lh__box_is_inline_flow(b)) {
			أكد(b->ويڪر != UNKNOWN_ويڪر);

			x_اڳيون = x;
			x += space_after;
			b->x = x;

			إذا ((b->type == BOX_INLINE && !b->inline_پڇاڙي) ||
					b->type == BOX_INLINE_بلاڪ ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[کاٻو_2] + b->border[کاٻو_2].ويڪر;
				x = b->x + b->padding[کاٻو_2] + b->ويڪر +
						b->padding[ساڄو_2] +
						b->border[ساڄو_2].ويڪر +
						b->margin[ساڄو_2];
			} وإلا إذا (b->type == BOX_INLINE) {
				b->x += b->margin[کاٻو_2] + b->border[کاٻو_2].ويڪر;
				x = b->x + b->padding[کاٻو_2] + b->ويڪر;
			} وإلا إذا (b->type == BOX_INLINE_پڇاڙي) {
				b->اوچائي = b->inline_پڇاڙي->اوچائي;
				x += b->padding[ساڄو_2] +
						b->border[ساڄو_2].ويڪر +
						b->margin[ساڄو_2];
			} وإلا {
				x += b->ويڪر;
			}

			space_before = space_after;
			إذا (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			وإلا إذا (b->متن || b->type == BOX_INLINE_پڇاڙي) {
				إذا (b->space == UNKNOWN_ويڪر) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->ويڪر(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} وإلا {
				space_after = 0;
			}
			ورهايو_box = b;
			move_y = true;
			inline_ڳڻپ++;
		} وإلا إذا (b->type == BOX_BR) {
			b->x = x;
			b->ويڪر = 0;
			br_box = b;
			b = b->ايندڙ;
			ورهايو_box = 0;
			move_y = true;
			اكسر;

		} وإلا {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_رکو_هيٺ_level = 0;
			b->float_container = d->float_container = cont;

			إذا (!layout_float(d, *ويڪر, content))
				أرجع false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[چوٽي_2],
			      d->border[چوٽي_2].ويڪر);

			d->x = d->margin[کاٻو_2] + d->border[کاٻو_2].ويڪر;
			d->y = d->margin[چوٽي_2] + d->border[چوٽي_2].ويڪر;
			b->ويڪر = d->margin[کاٻو_2] + d->border[کاٻو_2].ويڪر +
					d->padding[کاٻو_2] + d->ويڪر +
					d->padding[ساڄو_2] +
					d->border[ساڄو_2].ويڪر +
					d->margin[ساڄو_2];
			b->اوچائي = d->margin[چوٽي_2] + d->border[چوٽي_2].ويڪر +
					d->padding[چوٽي_2] + d->اوچائي +
					d->padding[BOTTOM] +
					d->border[BOTTOM].ويڪر +
					d->margin[BOTTOM];

			إذا (b->ويڪر > (x1 - x0) - x)
				رکو_هيٺ = true;
			إذا (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_کاٻو && کاٻو == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ساڄو &&
						ساڄو == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						کاٻو == 0 && ساڄو == 0)) &&
					(!رکو_هيٺ ||
					(کاٻو == 0 && ساڄو == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_رکو_هيٺ_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				إذا (b->type == BOX_FLOAT_کاٻو) {
					b->x = cx + x0;
					إذا (b->ويڪر > 0)
						x0 += b->ويڪر;
					کاٻو = b;
				} وإلا {
					b->x = cx + x1 - b->ويڪر;
					إذا (b->ويڪر > 0)
						x1 -= b->ويڪر;
					ساڄو = b;
				}
				b->y = cy;
			} وإلا {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				صحيح fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_رکو_هيٺ_level) ?
						fcy :
						cont->cached_رکو_هيٺ_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + اوچائي : fy;

				رکو_float_هيٺ(b, *ويڪر, cx, fy, cont);
				fy = b->y;
				إذا (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_کاٻو && کاٻو != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ساڄو &&
						ساڄو != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(کاٻو != 0 || ساڄو != 0)))) {
					/* to be cleared below existing
					 * floats */
					إذا (b->type == BOX_FLOAT_کاٻو)
						b->x = cx;
					وإلا
						b->x = cx + *ويڪر - b->ويڪر;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					إذا (fcy > cont->clear_level)
						cont->clear_level = fcy;
					إذا (b->y < fcy)
						b->y = fcy;
				}
				إذا (b->type == BOX_FLOAT_کاٻو)
					کاٻو = b;
				وإلا
					ساڄو = b;
			}
			جوڙيو_float_منزل_container(cont, b);

			ورهايو_box = 0;
		}
	}

	إذا (x1 - x0 < x && ورهايو_box) {
		/* the last box went over the end */
		size_t ورهايو = 0;
		صحيح w;
		bool no_wrap = css_computed_white_space(
				ورهايو_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				ورهايو_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_اڳيون;

		إذا (!no_wrap &&
		    (ورهايو_box->type == BOX_INLINE ||
		     ورهايو_box->type == BOX_متن) &&
		    !ورهايو_box->object &&
		    !(ورهايو_box->flags & REPLACE_DIM) &&
		    !(ورهايو_box->flags & IFRAME) &&
		    !ورهايو_box->gadget && ورهايو_box->متن) {

			font_plot_style_from_css(&content->unit_len_ctx,
					ورهايو_box->style, &fstyle);
			/** \todo handle errors */
			font_func->ورهايو(&fstyle,
					 ورهايو_box->متن,
					 ورهايو_box->ڊيگهائي,
					 x1 - x0 - x - space_before,
					 &ورهايو,
					 &w);
		}

		/* split == 0 implies that text can't be split */

		إذا (ورهايو == 0)
			w = ورهايو_box->ويڪر;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      ورهايو_box,
		      (صحيح)ورهايو_box->ڊيگهائي,
		      ورهايو_box->متن,
		      ورهايو,
		      w,
		      کاٻو,
		      ساڄو,
		      inline_ڳڻپ);

		إذا ((ورهايو == 0 || x1 - x0 <= x + space_before + w) &&
				!کاٻو && !ساڄو && inline_ڳڻپ == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			إذا (ورهايو == 0 || ورهايو == ورهايو_box->ڊيگهائي) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = ورهايو_box->ايندڙ;
			} وإلا {
				/* cut off first word for this line */
				إذا (!layout_متن_box_ورهايو(content, &fstyle,
						ورهايو_box, ورهايو, w))
					أرجع false;
				b = ورهايو_box->ايندڙ;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} وإلا إذا ((ورهايو == 0 || x1 - x0 <= x + space_before + w) &&
				inline_ڳڻپ == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			أكد(کاٻو || ساڄو);
			استعمال_ٿيل_اوچائي = 0;
			إذا (کاٻو) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      کاٻو->y,
				      کاٻو->اوچائي);

				استعمال_ٿيل_اوچائي = کاٻو->y + کاٻو->اوچائي - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      استعمال_ٿيل_اوچائي);

			}
			إذا (ساڄو && استعمال_ٿيل_اوچائي <
					ساڄو->y + ساڄو->اوچائي - cy + 1)
				استعمال_ٿيل_اوچائي = ساڄو->y + ساڄو->اوچائي - cy + 1;

			إذا (استعمال_ٿيل_اوچائي < 0)
				استعمال_ٿيل_اوچائي = 0;

			b = ورهايو_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} وإلا إذا (ورهايو == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = ورهايو_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} وإلا {
			/* fit as many words as possible */
			أكد(ورهايو != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (صحيح)ورهايو_box->ڊيگهائي, ورهايو_box->متن,
			      x1 - x0, ورهايو, w);

			إذا (ورهايو != ورهايو_box->ڊيگهائي) {
				إذا (!layout_متن_box_ورهايو(content, &fstyle,
						ورهايو_box, ورهايو, w))
					أرجع false;
				b = ورهايو_box->ايندڙ;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	بدل (css_computed_متن_align(پهريون->والد->والد->style)) {
	حالة CSS_متن_ALIGN_ساڄو:
	حالة CSS_متن_ALIGN_LIBCSS_ساڄو:
		x0 = x1 - x;
		اكسر;
	حالة CSS_متن_ALIGN_CENTER:
	حالة CSS_متن_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		اكسر;
	حالة CSS_متن_ALIGN_کاٻو:
	حالة CSS_متن_ALIGN_LIBCSS_کاٻو:
	حالة CSS_متن_ALIGN_JUSTIFY:
		/* leave on left */
		اكسر;
	حالة CSS_متن_ALIGN_DEFAULT:
		/* None; consider text direction */
		بدل (css_computed_direction(پهريون->والد->والد->style)) {
		حالة CSS_DIRECTION_LTR:
			/* leave on left */
			اكسر;
		حالة CSS_DIRECTION_RTL:
			x0 = x1 - x;
			اكسر;
		}
		اكسر;
	}

	لكل (d = پهريون; d != b; d = d->ايندڙ) {
		d->flags &= ~NEW_سٽ;

		إذا (d->type == BOX_INLINE_بلاڪ &&
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
				d->type == BOX_INLINE_پڇاڙي) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[چوٽي_2];

			إذا (d->type == BOX_متن && d->اوچائي > استعمال_ٿيل_اوچائي) {
				/*متن*/
				استعمال_ٿيل_اوچائي = d->اوچائي;
			}
		} وإلا إذا ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_بلاڪ) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[چوٽي_2].ويڪر + d->margin[چوٽي_2];
			h = d->margin[چوٽي_2] + d->border[چوٽي_2].ويڪر +
					d->padding[چوٽي_2] + d->اوچائي +
					d->padding[BOTTOM] +
					d->border[BOTTOM].ويڪر +
					d->margin[BOTTOM];
			إذا (استعمال_ٿيل_اوچائي < h)
				استعمال_ٿيل_اوچائي = h;
		}
	}

	پهريون->flags |= NEW_سٽ;

	أكد(b != پهريون || (move_y && 0 < استعمال_ٿيل_اوچائي && (کاٻو || ساڄو)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	لكل (d = پهريون; d != b; d = d->ايندڙ) {
		إذا ((d->type == BOX_INLINE && d->inline_پڇاڙي) ||
				d->type == BOX_BR ||
				d->type == BOX_متن ||
				d->type == BOX_INLINE_پڇاڙي) {
			css_fixed قدر = 0;
			css_unit unit = CSS_UNIT_PX;
			بدل (css_computed_vertical_align(d->style, &قدر,
					&unit)) {
			حالة CSS_VERTICAL_ALIGN_SUPER:
			حالة CSS_VERTICAL_ALIGN_چوٽي:
			حالة CSS_VERTICAL_ALIGN_متن_چوٽي:
				/* already at top */
				اكسر;
			حالة CSS_VERTICAL_ALIGN_SUB:
			حالة CSS_VERTICAL_ALIGN_BOTTOM:
			حالة CSS_VERTICAL_ALIGN_متن_BOTTOM:
				d->y += استعمال_ٿيل_اوچائي - d->اوچائي;
				اكسر;
			افتراضي:
			حالة CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (استعمال_ٿيل_اوچائي - d->اوچائي);
				اكسر;
			}
		}
	}

	/* handle clearance for br */
	إذا (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		صحيح clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		إذا (استعمال_ٿيل_اوچائي < clear_y - cy)
			استعمال_ٿيل_اوچائي = clear_y - cy;
	}

	إذا (move_y)
		*y += استعمال_ٿيل_اوچائي;
	*ايندڙ_box = b;
	*ويڪر = x; /* return actual width */
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
ساكن bool layout_inline_container(بنية box *inline_container, صحيح ويڪر,
		بنية box *cont, صحيح cx, صحيح cy, html_content *content)
{
	bool پهريون_سٽ = true;
	bool has_متن_children;
	بنية box *c, *ايندڙ;
	صحيح y = 0;
	صحيح curwidth,maxwidth = ويڪر;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      ويڪر,
	      cont,
	      cx,
	      cy);


	has_متن_children = false;
	لكل (c = inline_container->children; c; c = c->ايندڙ) {
		bool is_pre = false;

		إذا (c->style) {
			تعداد css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_سٽ ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		إذا ((lh__box_is_object(c) == false &&
				c->متن && (c->ڊيگهائي || is_pre)) ||
				c->type == BOX_BR)
			has_متن_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	لكل (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->ويڪر;
		إذا (!layout_سٽ(c, &curwidth, &y, cx, cy + y, cont, پهريون_سٽ,
				has_متن_children, content, &ايندڙ))
			أرجع false;
		maxwidth = max(maxwidth,curwidth);
		c = ايندڙ;
		پهريون_سٽ = false;
	}

	inline_container->ويڪر = maxwidth;
	inline_container->اوچائي = y;

	أرجع true;
}


/* Documented in layout_intertnal.h */
bool layout_بلاڪ_context(
		بنية box *بلاڪ,
		صحيح viewport_اوچائي,
		html_content *content)
{
	بنية box *box;
	صحيح cx, cy;  /**< current coordinates */
	صحيح max_pos_margin = 0;
	صحيح max_neg_margin = 0;
	صحيح y = 0;
	صحيح lm, rm;
	بنية box *margin_collapse = عدم;
	bool اندريون_margin = false;
	css_fixed gadget_سائيز;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	أكد(بلاڪ->type == BOX_بلاڪ ||
			بلاڪ->type == BOX_INLINE_بلاڪ ||
			بلاڪ->type == BOX_جدول_CELL ||
			بلاڪ->type == BOX_FLEX ||
			بلاڪ->type == BOX_INLINE_FLEX);
	أكد(بلاڪ->ويڪر != UNKNOWN_ويڪر);
	أكد(بلاڪ->ويڪر != AUTO);

	بلاڪ->float_children = عدم;
	بلاڪ->cached_رکو_هيٺ_level = 0;
	بلاڪ->clear_level = 0;

	/* special case if the block contains an object */
	إذا (بلاڪ->object) {
		صحيح عارضي_ويڪر = بلاڪ->ويڪر;
		إذا (!layout_بلاڪ_object(بلاڪ))
			أرجع false;
		layout_get_object_dimensions(بلاڪ, &عارضي_ويڪر,
				&بلاڪ->اوچائي, INT_MIN, وڌ_کان_وڌ_عدد,
				INT_MIN, وڌ_کان_وڌ_عدد);
		أرجع true;
	} وإلا إذا (بلاڪ->flags & REPLACE_DIM) {
		أرجع true;
	}

	/* special case if the block contains an radio button or checkbox */
	إذا (بلاڪ->gadget && (بلاڪ->gadget->type == GADGET_RADIO ||
			بلاڪ->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_سائيز = INTTOFIX(1);
		إذا (بلاڪ->اوچائي == AUTO)
			بلاڪ->اوچائي = FIXTOINT(css_unit_len2device_px(
					بلاڪ->style,
					&content->unit_len_ctx,
					gadget_سائيز, gadget_unit));
	}

	box = بلاڪ->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = بلاڪ->padding[چوٽي_2];
	إذا (box)
		box->y = بلاڪ->padding[چوٽي_2];

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
		تعداد css_حد_کان_مٿي_e حد_کان_مٿي_x = CSS_حد_کان_مٿي_VISIBLE;
		تعداد css_حد_کان_مٿي_e حد_کان_مٿي_y = CSS_حد_کان_مٿي_VISIBLE;

		أكد(box->type == BOX_بلاڪ ||
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
			box->x = box->والد->padding[کاٻو_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			اذهب advance_منزل_ايندڙ_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		إذا (margin_collapse == عدم) {
			margin_collapse = layout_ايندڙ_margin_بلاڪ(
					&content->unit_len_ctx, box, بلاڪ,
					viewport_اوچائي,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			اندريون_margin = true;
		}

		/* Clearance. */
		y = 0;
		إذا (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(بلاڪ->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		إذا (box->style) {
			حد_کان_مٿي_x = css_computed_حد_کان_مٿي_x(box->style);
			حد_کان_مٿي_y = css_computed_حد_کان_مٿي_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		إذا (box->type == BOX_FLEX ||
		    box->type == BOX_بلاڪ ||
		    box->flags & IFRAME) {
			إذا (lh__box_is_object(box) == false &&
					box->style &&
					(حد_کان_مٿي_x != CSS_حد_کان_مٿي_VISIBLE ||
					 حد_کان_مٿي_y != CSS_حد_کان_مٿي_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				صحيح x0, x1, چوٽي;
				بنية box *کاٻو, *ساڄو;
				چوٽي = cy + max_pos_margin - max_neg_margin;
				چوٽي = (چوٽي > y) ? چوٽي : y;
				x0 = cx;
				x1 = cx + box->والد->ويڪر -
						box->والد->padding[کاٻو_2] -
						box->والد->padding[ساڄو_2];
				ڳوليو_sides(بلاڪ->float_children, چوٽي, چوٽي,
						&x0, &x1, &کاٻو, &ساڄو);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->والد->ويڪر -
						box->والد->padding[کاٻو_2] -
						box->والد->padding[ساڄو_2] -
						x1;
			}
			layout_بلاڪ_ڳوليو_dimensions(&content->unit_len_ctx,
					box->والد->ويڪر,
					viewport_اوچائي, lm, rm, box);
			إذا (box->type == BOX_بلاڪ && !(box->flags & IFRAME)) {
				layout_بلاڪ_جوڙيو_scrollbar(box, ساڄو_2);
				layout_بلاڪ_جوڙيو_scrollbar(box, BOTTOM);
			}
		} وإلا إذا (box->type == BOX_جدول) {
			إذا (box->style != عدم) {
				تعداد css_ويڪر_e wtype;
				css_fixed ويڪر = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_ويڪر(box->style, &ويڪر,
						&unit);

				إذا (wtype == CSS_ويڪر_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					صحيح x0, x1, چوٽي;
					بنية box *کاٻو, *ساڄو;
					چوٽي = cy + max_pos_margin -
							max_neg_margin;
					چوٽي = (چوٽي > y) ? چوٽي : y;
					x0 = cx;
					x1 = cx + box->والد->ويڪر -
						box->والد->padding[کاٻو_2] -
						box->والد->padding[ساڄو_2];
					ڳوليو_sides(بلاڪ->float_children,
						چوٽي, چوٽي, &x0, &x1,
						&کاٻو, &ساڄو);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->والد->ويڪر -
						box->والد->padding[کاٻو_2] -
						box->والد->padding[ساڄو_2] -
						x1;
				}
			}
			إذا (!layout_جدول(box, box->والد->ويڪر - lm - rm,
					content))
				أرجع false;
			layout_solve_ويڪر(box, box->والد->ويڪر, box->ويڪر,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->والد->padding[کاٻو_2] + box->margin[کاٻو_2] +
				box->border[کاٻو_2].ويڪر;
		cx += box->x;

		/* Position box: vertical. */
		إذا (box->border[چوٽي_2].ويڪر) {
			box->y += box->border[چوٽي_2].ويڪر;
			cy += box->border[چوٽي_2].ويڪر;
		}

		/* Vertical margin */
		إذا (((box->type == BOX_بلاڪ && (box->flags & HAS_اوچائي)) ||
		     box->type == BOX_FLEX ||
		     box->type == BOX_جدول ||
		     (box->type == BOX_INLINE_CONTAINER &&
		      !box_is_پهريون_ٻار(box)) ||
		     margin_collapse == box) &&
		    اندريون_margin == true) {
			/* Margin goes above this box. */
			cy += max_pos_margin - max_neg_margin;
			box->y += max_pos_margin - max_neg_margin;

			/* Current margin has been applied. */
			اندريون_margin = false;
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
				(box->type == BOX_بلاڪ && box->style &&
				 (حد_کان_مٿي_x != CSS_حد_کان_مٿي_VISIBLE ||
				  حد_کان_مٿي_y != CSS_حد_کان_مٿي_VISIBLE))) {

			إذا (box->type == BOX_FLEX) {
				إذا (!layout_flex(box, box->ويڪر, content)) {
					أرجع false;
				}
			} وإلا {
				layout_بلاڪ_context(box,
						viewport_اوچائي, content);
			}

			cy += box->padding[چوٽي_2];

			إذا (box->اوچائي == AUTO) {
				box->اوچائي = 0;
				layout_بلاڪ_جوڙيو_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->اوچائي + box->padding[BOTTOM] +
					box->border[BOTTOM].ويڪر;
			y = box->y + box->padding[چوٽي_2] + box->اوچائي +
					box->padding[BOTTOM] +
					box->border[BOTTOM].ويڪر;

			/* Skip children, because they are done in the new
			 * block context */
			اذهب advance_منزل_ايندڙ_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->ويڪر);

		/* Layout (except tables). */
		إذا (box->object) {
			إذا (!layout_بلاڪ_object(box))
				أرجع false;

		} وإلا إذا (box->type == BOX_INLINE_CONTAINER) {
			box->ويڪر = box->والد->ويڪر;
			إذا (!layout_inline_container(box, box->ويڪر, بلاڪ,
					cx, cy, content))
				أرجع false;

		} وإلا إذا (box->type == BOX_جدول) {
			/* Move down to avoid floats if necessary. */
			صحيح x0, x1;
			بنية box *کاٻو, *ساڄو;
			y = cy;
			طالما (1) {
				تعداد css_ويڪر_e wtype;
				css_fixed ويڪر = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_ويڪر(box->style,
						&ويڪر, &unit);

				x0 = cx;
				x1 = cx + box->والد->ويڪر;
				ڳوليو_sides(بلاڪ->float_children, y,
						y + box->اوچائي,
						&x0, &x1, &کاٻو, &ساڄو);
				إذا (wtype == CSS_ويڪر_AUTO)
					اكسر;
				إذا (box->ويڪر <= x1 - x0)
					اكسر;
				إذا (!کاٻو && !ساڄو)
					اكسر;
				وإلا إذا (!کاٻو)
					y = ساڄو->y + ساڄو->اوچائي + 1;
				وإلا إذا (!ساڄو)
					y = کاٻو->y + کاٻو->اوچائي + 1;
				وإلا إذا (کاٻو->y + کاٻو->اوچائي <
						ساڄو->y + ساڄو->اوچائي)
					y = کاٻو->y + کاٻو->اوچائي + 1;
				وإلا
					y = ساڄو->y + ساڄو->اوچائي + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		إذا (box->type == BOX_بلاڪ && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			إذا (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = عدم;
			}

			y = box->padding[چوٽي_2];
			box = box->children;
			box->y = y;
			cy += y;
			استمر;
		} وإلا إذا (box->type == BOX_بلاڪ || box->object ||
				box->flags & IFRAME)
			cy += box->padding[چوٽي_2];

		إذا (box->type == BOX_بلاڪ && box->اوچائي == AUTO) {
			box->اوچائي = 0;
			layout_بلاڪ_جوڙيو_scrollbar(box, BOTTOM);
		}

		cy += box->اوچائي + box->padding[BOTTOM] +
				box->border[BOTTOM].ويڪر;
		cx -= box->x;
		y = box->y + box->padding[چوٽي_2] + box->اوچائي +
				box->padding[BOTTOM] +
				box->border[BOTTOM].ويڪر;

	advance_منزل_ايندڙ_box:
		إذا (!box->ايندڙ) {
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

				box = box->والد;
				إذا (box == بلاڪ)
					اكسر;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				إذا (box->type == BOX_بلاڪ &&
						box->flags & MAKE_اوچائي) {
					margin_collapse = عدم;
					اندريون_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				إذا (box->اوچائي == AUTO) {
					box->اوچائي = y - box->padding[چوٽي_2];

					إذا (box->type == BOX_بلاڪ)
						layout_بلاڪ_جوڙيو_scrollbar(box,
								BOTTOM);
				} وإلا
					cy += box->اوچائي -
							(y - box->padding[چوٽي_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				إذا (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_اوچائي(
							&content->unit_len_ctx,
							box, عدم)) {
					/* Height altered */
					/* Set current cy */
					cy += box->اوچائي -
							(y - box->padding[چوٽي_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].ويڪر;
				cx -= box->x;
				y = box->y + box->padding[چوٽي_2] + box->اوچائي +
						box->padding[BOTTOM] +
						box->border[BOTTOM].ويڪر;

			} طالما (box->ايندڙ == عدم);
			إذا (box == بلاڪ)
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

		box = box->ايندڙ;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	لكل (box = بلاڪ->float_children; box; box = box->ايندڙ_float) {
		y = box->y + box->اوچائي + box->padding[BOTTOM] +
				box->border[BOTTOM].ويڪر + box->margin[BOTTOM];
		إذا (cy < y)
			cy = y;
	}

	إذا (بلاڪ->اوچائي == AUTO) {
		بلاڪ->اوچائي = cy - بلاڪ->padding[چوٽي_2];
		إذا (بلاڪ->type == BOX_بلاڪ)
			layout_بلاڪ_جوڙيو_scrollbar(بلاڪ, BOTTOM);
	}

	إذا (بلاڪ->style && css_computed_position(بلاڪ->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_اوچائي(&content->unit_len_ctx, بلاڪ, عدم);
	}

	إذا (بلاڪ->gadget &&
			(بلاڪ->gadget->type == GADGET_TEXTAREA ||
			بلاڪ->gadget->type == GADGET_PASSWORD ||
			بلاڪ->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		صحيح ta_ويڪر = بلاڪ->padding[کاٻو_2] + بلاڪ->ويڪر +
				بلاڪ->padding[ساڄو_2];
		صحيح ta_اوچائي = بلاڪ->padding[چوٽي_2] + بلاڪ->اوچائي +
				بلاڪ->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				بلاڪ->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_سيٽ_layout(بلاڪ->gadget->معلومات.متن.ta,
				&fstyle, ta_ويڪر, ta_اوچائي,
				بلاڪ->padding[چوٽي_2], بلاڪ->padding[ساڄو_2],
				بلاڪ->padding[BOTTOM], بلاڪ->padding[کاٻو_2]);
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
layout_get_element_ٽيگ(
		ثابت dom_ڳنڍ *ڳنڍ,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_ڳنڍ_type ڳنڍ_type;
	dom_exception exc;

	exc = dom_ڳنڍ_get_ڳنڍ_type(ڳنڍ, &ڳنڍ_type);
	إذا (exc != DOM_NO_ERR ||
	    ڳنڍ_type != DOM_ELEMENT_ڳنڍ) {
		أرجع false;
	}

	exc = dom_html_element_get_ٽيگ_type(ڳنڍ, &element_type);
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
		ثابت dom_ڳنڍ *ڳنڍ,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	إذا (!layout_get_element_ٽيگ(ڳنڍ, &element_type)) {
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
layout_get_li_قدر(dom_ڳنڍ *li_ڳنڍ, dom_long *قدر_ٻاهريون)
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
	exc = dom_element_has_attribute(li_ڳنڍ,
			corestring_dom_قدر,
			&has_قدر);
	إذا (exc != DOM_NO_ERR || has_قدر == false) {
		أرجع false;
	}

	exc = dom_html_li_element_get_قدر(
			(dom_html_li_element *)li_ڳنڍ,
			&قدر);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	*قدر_ٻاهريون = قدر;
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
layout_get_ol_شروعات(dom_ڳنڍ *ol_ڳنڍ, dom_long *شروعات_ٻاهريون)
{
	dom_exception exc;
	dom_long شروعات;
	bool has_شروعات;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_ڳنڍ,
			corestring_dom_شروعات,
			&has_شروعات);
	إذا (exc != DOM_NO_ERR || has_شروعات == false) {
		أرجع false;
	}

	exc = dom_html_olist_element_get_شروعات(
			(dom_html_olist_element *)ol_ڳنڍ,
			&شروعات);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	*شروعات_ٻاهريون = شروعات;
	أرجع true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
ساكن bool
layout__get_ol_reversed(dom_ڳنڍ *ol_ڳنڍ)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_ڳنڍ,
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
layout_get_list_شي_ڳڻپ(
		dom_ڳنڍ *list_owner, dom_long *ڳڻپ_ٻاهريون)
{
	dom_html_element_type ٽيگ_type;
	dom_exception exc;
	dom_ڳنڍ *ٻار;
	صحيح ڳڻپ;

	إذا (list_owner == عدم) {
		أرجع false;
	}

	إذا (!layout_get_element_ٽيگ(list_owner, &ٽيگ_type)) {
		أرجع false;
	}

	إذا (ٽيگ_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    ٽيگ_type != DOM_HTML_ELEMENT_TYPE_UL) {
		أرجع false;
	}

	exc = dom_ڳنڍ_get_پهريون_ٻار(list_owner, &ٻار);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	ڳڻپ = 0;
	طالما (ٻار != عدم) {
		dom_ڳنڍ *عارضي_ڳنڍ;

		إذا (layout__check_element_type(ٻار,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			بنية box *ٻار_box;
			إذا (dom_ڳنڍ_get_user_معلومات(ٻار,
					corestring_dom_ns_ڪنجي_box_ڳنڍ_معلومات,
					&ٻار_box) != DOM_NO_ERR) {
				dom_ڳنڍ_unref(ٻار);
				أرجع false;
			}

			إذا (ٻار_box != عدم &&
			    ٻار_box->list_marker != عدم) {
				ڳڻپ++;
			}
		}

		exc = dom_ڳنڍ_get_ايندڙ_sibling(ٻار, &عارضي_ڳنڍ);
		dom_ڳنڍ_unref(ٻار);
		إذا (exc != DOM_NO_ERR) {
			أرجع false;
		}

		ٻار = عارضي_ڳنڍ;
	}

	*ڳڻپ_ٻاهريون = ڳڻپ;
	أرجع true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
ساكن فراغ
layout_ordered_list_ڳڻپ(
		بنية box *box)
{
	dom_html_element_type ٽيگ_type;
	dom_exception exc;
	dom_ڳنڍ *ٻار;
	صحيح قدم = 1;
	dom_long ايندڙ;

	إذا (box->ڳنڍ == عدم) {
		أرجع;
	}

	إذا (!layout_get_element_ٽيگ(box->ڳنڍ, &ٽيگ_type)) {
		أرجع;
	}

	إذا (ٽيگ_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    ٽيگ_type != DOM_HTML_ELEMENT_TYPE_UL) {
		أرجع;
	}

	ايندڙ = 1;
	إذا (ٽيگ_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_شروعات = layout_get_ol_شروعات(box->ڳنڍ, &ايندڙ);
		bool have_reversed = layout__get_ol_reversed(box->ڳنڍ);

		إذا (have_reversed) {
			قدم = -1;
		}

		إذا (!have_شروعات && have_reversed) {
			layout_get_list_شي_ڳڻپ(box->ڳنڍ, &ايندڙ);
		}
	}

	exc = dom_ڳنڍ_get_پهريون_ٻار(box->ڳنڍ, &ٻار);
	إذا (exc != DOM_NO_ERR) {
		أرجع;
	}

	طالما (ٻار != عدم) {
		dom_ڳنڍ *عارضي_ڳنڍ;

		إذا (layout__check_element_type(ٻار,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			بنية box *ٻار_box;

			إذا (dom_ڳنڍ_get_user_معلومات(ٻار,
					corestring_dom_ns_ڪنجي_box_ڳنڍ_معلومات,
					&ٻار_box) != DOM_NO_ERR) {
				dom_ڳنڍ_unref(ٻار);
				أرجع;
			}

			إذا (ٻار_box != عدم &&
			    ٻار_box->list_marker != عدم) {
				dom_long قدر;
				بنية box *marker = ٻار_box->list_marker;
				إذا (layout_get_li_قدر(ٻار, &قدر)) {
					marker->list_قدر = قدر;
					ايندڙ = marker->list_قدر;
				} وإلا {
					marker->list_قدر = ايندڙ;
				}
				ايندڙ += قدم;
			}
		}

		exc = dom_ڳنڍ_get_ايندڙ_sibling(ٻار, &عارضي_ڳنڍ);
		dom_ڳنڍ_unref(ٻار);
		إذا (exc != DOM_NO_ERR) {
			أرجع;
		}

		ٻار = عارضي_ڳنڍ;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
ساكن فراغ
layout_سيٽ_numerical_marker_متن(
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
		LIST_MARKER_سائيز = 20,
	};

	marker->متن = talloc_array(content->bctx, حرف, LIST_MARKER_سائيز);
	إذا (marker->متن == عدم) {
		أرجع;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_قدر,
			marker->متن, LIST_MARKER_سائيز, &counter_len);
	إذا (css_res == CSS_OK) {
		إذا (counter_len > LIST_MARKER_سائيز) {
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
		marker->ڊيگهائي = counter_len;
	}
}

/**
 * Find out if box's style represents a numerical list style type.
 *
 * \param[in]  b  Box with style to test.
 * \return true if box has numerical list style type, false otherwise.
 */
ساكن bool
layout_list_شي_is_numerical(
		ثابت بنية box *b)
{
	تعداد css_list_style_type_e t = css_computed_list_style_type(b->style);

	بدل (t) {
	حالة CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	حالة CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	حالة CSS_LIST_STYLE_TYPE_چورس: /* Fall through. */
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
	بنية box *ٻار;

	layout_ordered_list_ڳڻپ(box);

	لكل (ٻار = box->children; ٻار; ٻار = ٻار->ايندڙ) {
		إذا (ٻار->list_marker) {
			بنية box *marker = ٻار->list_marker;

			إذا (layout_list_شي_is_numerical(ٻار)) {
				إذا (marker->متن == عدم) {
					layout_سيٽ_numerical_marker_متن(
							content, ٻار);
				}
			}
			إذا (marker->object) {
				marker->ويڪر =
					content_get_ويڪر(marker->object);
				marker->x = -marker->ويڪر;
				marker->اوچائي =
					content_get_اوچائي(marker->object);
				marker->y = (سٽ_اوچائي(
						&content->unit_len_ctx,
						marker->style) -
						marker->اوچائي) / 2;
			} وإلا إذا (marker->متن) {
				إذا (marker->ويڪر == UNKNOWN_ويڪر) {
					plot_font_style_t fstyle;
					font_plot_style_from_css(
							&content->unit_len_ctx,
							marker->style,
							&fstyle);
					content->font_func->ويڪر(&fstyle,
							marker->متن,
							marker->ڊيگهائي,
							&marker->ويڪر);
					marker->flags |= MEASURED;
				}
				marker->x = -marker->ويڪر;
				marker->y = 0;
				marker->اوچائي = سٽ_اوچائي(
						&content->unit_len_ctx,
						marker->style);
			} وإلا {
				marker->x = 0;
				marker->y = 0;
				marker->ويڪر = 0;
				marker->اوچائي = 0;
			}
			/* Gap between marker and content */
			marker->x -= 4;
		}
		layout_lists(content, ٻار);
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
		       بنية box *containing_بلاڪ,
		       صحيح *چوٽي,
		       صحيح *ساڄو,
		       صحيح *bottom,
		       صحيح *کاٻو)
{
	uint32_t type;
	css_fixed قدر = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(containing_بلاڪ->ويڪر != UNKNOWN_ويڪر);
	أكد(containing_بلاڪ->ويڪر != AUTO);
	أكد(containing_بلاڪ->اوچائي != AUTO);

	/*کاٻو*/
	type = css_computed_کاٻو(box->style, &قدر, &unit);
	إذا (type == CSS_کاٻو_سيٽ) {
		إذا (unit == CSS_UNIT_PCT) {
			*کاٻو = FPCT_جو_INT_TOINT(قدر,
					containing_بلاڪ->ويڪر);
		} وإلا {
			*کاٻو = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					قدر, unit));
		}
	} وإلا {
		*کاٻو = AUTO;
	}

	/*ساڄو*/
	type = css_computed_ساڄو(box->style, &قدر, &unit);
	إذا (type == CSS_ساڄو_سيٽ) {
		إذا (unit == CSS_UNIT_PCT) {
			*ساڄو = FPCT_جو_INT_TOINT(قدر,
					containing_بلاڪ->ويڪر);
		} وإلا {
			*ساڄو = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					قدر, unit));
		}
	} وإلا {
		*ساڄو = AUTO;
	}

	/*چوٽي*/
	type = css_computed_چوٽي(box->style, &قدر, &unit);
	إذا (type == CSS_چوٽي_سيٽ) {
		إذا (unit == CSS_UNIT_PCT) {
			*چوٽي = FPCT_جو_INT_TOINT(قدر,
					containing_بلاڪ->اوچائي);
		} وإلا {
			*چوٽي = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					قدر, unit));
		}
	} وإلا {
		*چوٽي = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &قدر, &unit);
	إذا (type == CSS_BOTTOM_سيٽ) {
		إذا (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_جو_INT_TOINT(قدر,
					containing_بلاڪ->اوچائي);
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
		بنية box *containing_بلاڪ,
		صحيح cx, صحيح cy,
		html_content *content)
{
	صحيح static_کاٻو, static_چوٽي;  /* static position */
	صحيح چوٽي, ساڄو, bottom, کاٻو;
	صحيح ويڪر, اوچائي, max_ويڪر, min_ويڪر;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	صحيح موجود_ويڪر = containing_بلاڪ->ويڪر;
	صحيح space;

	أكد(box->type == BOX_بلاڪ || box->type == BOX_جدول ||
			box->type == BOX_INLINE_بلاڪ ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_کاٻو = cx + box->x;
	static_چوٽي = cy + box->y;

	إذا (containing_بلاڪ->type == BOX_بلاڪ ||
			containing_بلاڪ->type == BOX_INLINE_بلاڪ ||
			containing_بلاڪ->type == BOX_جدول_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_بلاڪ->ويڪر += containing_بلاڪ->padding[کاٻو_2] +
				containing_بلاڪ->padding[ساڄو_2];
		containing_بلاڪ->اوچائي += containing_بلاڪ->padding[چوٽي_2] +
				containing_بلاڪ->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_بلاڪ,
			&چوٽي, &ساڄو, &bottom, &کاٻو);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_بلاڪ;
	layout_ڳوليو_dimensions(&content->unit_len_ctx, موجود_ويڪر, -1,
			box, box->style, &ويڪر, &اوچائي,
			&max_ويڪر, &min_ويڪر, 0, 0,
			margin, padding, border);
	box->float_container = عدم;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      کاٻو, margin[کاٻو_2], border[کاٻو_2].ويڪر, padding[کاٻو_2], ويڪر,
	      padding[ساڄو_2], border[ساڄو_2].ويڪر, margin[ساڄو_2], ساڄو,
	      containing_بلاڪ->ويڪر);


	إذا (کاٻو == AUTO && ويڪر == AUTO && ساڄو == AUTO) {
		إذا (margin[کاٻو_2] == AUTO)
			margin[کاٻو_2] = 0;
		إذا (margin[ساڄو_2] == AUTO)
			margin[ساڄو_2] = 0;
		کاٻو = static_کاٻو;

		ويڪر = min(max(box->min_ويڪر, موجود_ويڪر),
			box->max_ويڪر);
		ويڪر -= box->margin[کاٻو_2] + box->border[کاٻو_2].ويڪر +
			box->padding[کاٻو_2] + box->padding[ساڄو_2] +
			box->border[ساڄو_2].ويڪر + box->margin[ساڄو_2];

		/* Adjust for {min|max}-width */
		إذا (max_ويڪر >= 0 && ويڪر > max_ويڪر) ويڪر = max_ويڪر;
		إذا (ويڪر < min_ويڪر) ويڪر = min_ويڪر;

		ساڄو = containing_بلاڪ->ويڪر -
			کاٻو -
			margin[کاٻو_2] - border[کاٻو_2].ويڪر - padding[کاٻو_2] -
			ويڪر -
			padding[ساڄو_2] - border[ساڄو_2].ويڪر - margin[ساڄو_2];
	} وإلا إذا (کاٻو != AUTO && ويڪر != AUTO && ساڄو != AUTO) {

		/* Adjust for {min|max}-width */
		إذا (max_ويڪر >= 0 && ويڪر > max_ويڪر) ويڪر = max_ويڪر;
		إذا (min_ويڪر >  0 && ويڪر < min_ويڪر) ويڪر = min_ويڪر;

		إذا (margin[کاٻو_2] == AUTO && margin[ساڄو_2] == AUTO) {
			space = containing_بلاڪ->ويڪر -
					کاٻو - border[کاٻو_2].ويڪر -
					padding[کاٻو_2] - ويڪر - padding[ساڄو_2] -
					border[ساڄو_2].ويڪر - ساڄو;
			إذا (space < 0) {
				margin[کاٻو_2] = 0;
				margin[ساڄو_2] = space;
			} وإلا {
				margin[کاٻو_2] = margin[ساڄو_2] = space / 2;
			}
		} وإلا إذا (margin[کاٻو_2] == AUTO) {
			margin[کاٻو_2] = containing_بلاڪ->ويڪر -
					کاٻو - border[کاٻو_2].ويڪر -
					padding[کاٻو_2] - ويڪر - padding[ساڄو_2] -
					border[ساڄو_2].ويڪر - margin[ساڄو_2] -
					ساڄو;
		} وإلا إذا (margin[ساڄو_2] == AUTO) {
			margin[ساڄو_2] = containing_بلاڪ->ويڪر -
					کاٻو - margin[کاٻو_2] -
					border[کاٻو_2].ويڪر -
					padding[کاٻو_2] - ويڪر - padding[ساڄو_2] -
					border[ساڄو_2].ويڪر - ساڄو;
		} وإلا {
			ساڄو = containing_بلاڪ->ويڪر -
					کاٻو - margin[کاٻو_2] -
					border[کاٻو_2].ويڪر -
					padding[کاٻو_2] - ويڪر - padding[ساڄو_2] -
					border[ساڄو_2].ويڪر - margin[ساڄو_2];
		}
	} وإلا {
		إذا (margin[کاٻو_2] == AUTO)
			margin[کاٻو_2] = 0;
		إذا (margin[ساڄو_2] == AUTO)
			margin[ساڄو_2] = 0;

		إذا (کاٻو == AUTO && ويڪر == AUTO && ساڄو != AUTO) {
			موجود_ويڪر -= ساڄو;

			ويڪر = min(max(box->min_ويڪر, موجود_ويڪر),
				box->max_ويڪر);
			ويڪر -= box->margin[کاٻو_2] + box->border[کاٻو_2].ويڪر +
				box->padding[کاٻو_2] + box->padding[ساڄو_2] +
				box->border[ساڄو_2].ويڪر + box->margin[ساڄو_2];

			/* Adjust for {min|max}-width */
			إذا (max_ويڪر >= 0 && ويڪر > max_ويڪر)
				ويڪر = max_ويڪر;
			إذا (ويڪر < min_ويڪر)
				ويڪر = min_ويڪر;

			کاٻو = containing_بلاڪ->ويڪر -
					margin[کاٻو_2] - border[کاٻو_2].ويڪر -
					padding[کاٻو_2] - ويڪر - padding[ساڄو_2] -
					border[ساڄو_2].ويڪر - margin[ساڄو_2] -
					ساڄو;
		} وإلا إذا (کاٻو == AUTO && ويڪر != AUTO && ساڄو == AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_ويڪر >= 0 && ويڪر > max_ويڪر)
				ويڪر = max_ويڪر;
			إذا (min_ويڪر >  0 && ويڪر < min_ويڪر)
				ويڪر = min_ويڪر;

			کاٻو = static_کاٻو;
			ساڄو = containing_بلاڪ->ويڪر -
					کاٻو - margin[کاٻو_2] -
					border[کاٻو_2].ويڪر -
					padding[کاٻو_2] - ويڪر - padding[ساڄو_2] -
					border[ساڄو_2].ويڪر - margin[ساڄو_2];
		} وإلا إذا (کاٻو != AUTO && ويڪر == AUTO && ساڄو == AUTO) {
			موجود_ويڪر -= کاٻو;

			ويڪر = min(max(box->min_ويڪر, موجود_ويڪر),
				box->max_ويڪر);
			ويڪر -= box->margin[کاٻو_2] + box->border[کاٻو_2].ويڪر +
				box->padding[کاٻو_2] + box->padding[ساڄو_2] +
				box->border[ساڄو_2].ويڪر + box->margin[ساڄو_2];

			/* Adjust for {min|max}-width */
			إذا (max_ويڪر >= 0 && ويڪر > max_ويڪر)
				ويڪر = max_ويڪر;
			إذا (ويڪر < min_ويڪر)
				ويڪر = min_ويڪر;

			ساڄو = containing_بلاڪ->ويڪر -
					کاٻو - margin[کاٻو_2] -
					border[کاٻو_2].ويڪر -
					padding[کاٻو_2] - ويڪر - padding[ساڄو_2] -
					border[ساڄو_2].ويڪر - margin[ساڄو_2];
		} وإلا إذا (کاٻو == AUTO && ويڪر != AUTO && ساڄو != AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_ويڪر >= 0 && ويڪر > max_ويڪر)
				ويڪر = max_ويڪر;
			إذا (ويڪر < min_ويڪر)
				ويڪر = min_ويڪر;

			کاٻو = containing_بلاڪ->ويڪر -
					margin[کاٻو_2] - border[کاٻو_2].ويڪر -
					padding[کاٻو_2] - ويڪر - padding[ساڄو_2] -
					border[ساڄو_2].ويڪر - margin[ساڄو_2] -
					ساڄو;
		} وإلا إذا (کاٻو != AUTO && ويڪر == AUTO && ساڄو != AUTO) {
			ويڪر = containing_بلاڪ->ويڪر -
					کاٻو - margin[کاٻو_2] -
					border[کاٻو_2].ويڪر -
					padding[کاٻو_2] - padding[ساڄو_2] -
					border[ساڄو_2].ويڪر - margin[ساڄو_2] -
					ساڄو;

			/* Adjust for {min|max}-width */
			إذا (max_ويڪر >= 0 && ويڪر > max_ويڪر)
				ويڪر = max_ويڪر;
			إذا (ويڪر < min_ويڪر)
				ويڪر = min_ويڪر;

		} وإلا إذا (کاٻو != AUTO && ويڪر != AUTO && ساڄو == AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_ويڪر >= 0 && ويڪر > max_ويڪر)
				ويڪر = max_ويڪر;
			إذا (ويڪر < min_ويڪر)
				ويڪر = min_ويڪر;

			ساڄو = containing_بلاڪ->ويڪر -
					کاٻو - margin[کاٻو_2] -
					border[کاٻو_2].ويڪر -
					padding[کاٻو_2] - ويڪر - padding[ساڄو_2] -
					border[ساڄو_2].ويڪر - margin[ساڄو_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      کاٻو, margin[کاٻو_2], border[کاٻو_2].ويڪر, padding[کاٻو_2], ويڪر,
	      padding[ساڄو_2], border[ساڄو_2].ويڪر, margin[ساڄو_2], ساڄو,
	      containing_بلاڪ->ويڪر);

	box->x = کاٻو + margin[کاٻو_2] + border[کاٻو_2].ويڪر - cx;
	إذا (containing_بلاڪ->type == BOX_بلاڪ ||
			containing_بلاڪ->type == BOX_INLINE_بلاڪ ||
			containing_بلاڪ->type == BOX_جدول_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_بلاڪ->ويڪر -= containing_بلاڪ->padding[کاٻو_2] +
				containing_بلاڪ->padding[ساڄو_2];
	} وإلا {
		/** \todo inline ancestors */
	}
	box->ويڪر = ويڪر;
	box->اوچائي = اوچائي;

	إذا (box->type == BOX_بلاڪ || box->type == BOX_INLINE_بلاڪ ||
			box->object || box->flags & IFRAME) {
		إذا (!layout_بلاڪ_context(box, -1, content))
			أرجع false;
	} وإلا إذا (box->type == BOX_جدول) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_بلاڪ;
		/* \todo  layout_table considers margins etc. again */
		إذا (!layout_جدول(box, ويڪر, content))
			أرجع false;
		box->float_container = عدم;
		layout_solve_ويڪر(box, box->والد->ويڪر, box->ويڪر, 0, 0,
				-1, -1);
	} وإلا إذا (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_بلاڪ;
		إذا (!layout_flex(box, ويڪر, content))
			أرجع false;
		box->float_container = عدم;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      چوٽي, margin[چوٽي_2], border[چوٽي_2].ويڪر, padding[چوٽي_2], اوچائي,
	      padding[BOTTOM], border[BOTTOM].ويڪر, margin[BOTTOM], bottom,
	      containing_بلاڪ->اوچائي);

	إذا (چوٽي == AUTO && اوچائي == AUTO && bottom == AUTO) {
		چوٽي = static_چوٽي;
		اوچائي = box->اوچائي;
		إذا (margin[چوٽي_2] == AUTO)
			margin[چوٽي_2] = 0;
		إذا (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_بلاڪ->اوچائي -
				چوٽي - margin[چوٽي_2] - border[چوٽي_2].ويڪر -
				padding[چوٽي_2] - اوچائي - padding[BOTTOM] -
				border[BOTTOM].ويڪر - margin[BOTTOM];
	} وإلا إذا (چوٽي != AUTO && اوچائي != AUTO && bottom != AUTO) {
		إذا (margin[چوٽي_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_بلاڪ->اوچائي -
					چوٽي - border[چوٽي_2].ويڪر - padding[چوٽي_2] -
					اوچائي - padding[BOTTOM] -
					border[BOTTOM].ويڪر - bottom;
			margin[چوٽي_2] = margin[BOTTOM] = space / 2;
		} وإلا إذا (margin[چوٽي_2] == AUTO) {
			margin[چوٽي_2] = containing_بلاڪ->اوچائي -
					چوٽي - border[چوٽي_2].ويڪر - padding[چوٽي_2] -
					اوچائي - padding[BOTTOM] -
					border[BOTTOM].ويڪر - margin[BOTTOM] -
					bottom;
		} وإلا إذا (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_بلاڪ->اوچائي -
					چوٽي - margin[چوٽي_2] - border[چوٽي_2].ويڪر -
					padding[چوٽي_2] - اوچائي -
					padding[BOTTOM] - border[BOTTOM].ويڪر -
					bottom;
		} وإلا {
			bottom = containing_بلاڪ->اوچائي -
					چوٽي - margin[چوٽي_2] - border[چوٽي_2].ويڪر -
					padding[چوٽي_2] - اوچائي -
					padding[BOTTOM] - border[BOTTOM].ويڪر -
					margin[BOTTOM];
		}
	} وإلا {
		إذا (margin[چوٽي_2] == AUTO)
			margin[چوٽي_2] = 0;
		إذا (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		إذا (چوٽي == AUTO && اوچائي == AUTO && bottom != AUTO) {
			اوچائي = box->اوچائي;
			چوٽي = containing_بلاڪ->اوچائي -
					margin[چوٽي_2] - border[چوٽي_2].ويڪر -
					padding[چوٽي_2] - اوچائي -
					padding[BOTTOM] - border[BOTTOM].ويڪر -
					margin[BOTTOM] - bottom;
		} وإلا إذا (چوٽي == AUTO && اوچائي != AUTO && bottom == AUTO) {
			چوٽي = static_چوٽي;
			bottom = containing_بلاڪ->اوچائي -
					چوٽي - margin[چوٽي_2] - border[چوٽي_2].ويڪر -
					padding[چوٽي_2] - اوچائي -
					padding[BOTTOM] - border[BOTTOM].ويڪر -
					margin[BOTTOM];
		} وإلا إذا (چوٽي != AUTO && اوچائي == AUTO && bottom == AUTO) {
			اوچائي = box->اوچائي;
			bottom = containing_بلاڪ->اوچائي -
					چوٽي - margin[چوٽي_2] - border[چوٽي_2].ويڪر -
					padding[چوٽي_2] - اوچائي -
					padding[BOTTOM] - border[BOTTOM].ويڪر -
					margin[BOTTOM];
		} وإلا إذا (چوٽي == AUTO && اوچائي != AUTO && bottom != AUTO) {
			چوٽي = containing_بلاڪ->اوچائي -
					margin[چوٽي_2] - border[چوٽي_2].ويڪر -
					padding[چوٽي_2] - اوچائي -
					padding[BOTTOM] - border[BOTTOM].ويڪر -
					margin[BOTTOM] - bottom;
		} وإلا إذا (چوٽي != AUTO && اوچائي == AUTO && bottom != AUTO) {
			اوچائي = containing_بلاڪ->اوچائي -
					چوٽي - margin[چوٽي_2] - border[چوٽي_2].ويڪر -
					padding[چوٽي_2] - padding[BOTTOM] -
					border[BOTTOM].ويڪر - margin[BOTTOM] -
					bottom;
		} وإلا إذا (چوٽي != AUTO && اوچائي != AUTO && bottom == AUTO) {
			bottom = containing_بلاڪ->اوچائي -
					چوٽي - margin[چوٽي_2] - border[چوٽي_2].ويڪر -
					padding[چوٽي_2] - اوچائي -
					padding[BOTTOM] - border[BOTTOM].ويڪر -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      چوٽي, margin[چوٽي_2], border[چوٽي_2].ويڪر, padding[چوٽي_2], اوچائي,
	      padding[BOTTOM], border[BOTTOM].ويڪر, margin[BOTTOM], bottom,
	      containing_بلاڪ->اوچائي);

	box->y = چوٽي + margin[چوٽي_2] + border[چوٽي_2].ويڪر - cy;
	إذا (containing_بلاڪ->type == BOX_بلاڪ ||
			containing_بلاڪ->type == BOX_INLINE_بلاڪ ||
			containing_بلاڪ->type == BOX_جدول_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_بلاڪ->اوچائي -= containing_بلاڪ->padding[چوٽي_2] +
				containing_بلاڪ->padding[BOTTOM];
	} وإلا {
		/** \todo Inline ancestors */
	}
	box->اوچائي = اوچائي;
	layout_apply_minmax_اوچائي(&content->unit_len_ctx, box, containing_بلاڪ);

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
			 بنية box *containing_بلاڪ,
			 صحيح cx, صحيح cy,
			 html_content *content)
{
	بنية box *c;

	لكل (c = box->children; c; c = c->ايندڙ) {
		إذا ((c->type == BOX_بلاڪ || c->type == BOX_جدول ||
				c->type == BOX_INLINE_بلاڪ ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			إذا (!layout_absolute(c, containing_بلاڪ,
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
						CSS_FLOAT_کاٻو ||
					css_computed_float(c->style) ==
						CSS_FLOAT_ساڄو)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				بنية box *p;
				px = c->x;
				py = c->y;
				لكل (p = box->والد; p && !p->float_children;
						p = p->والد) {
					px -= p->x;
					py -= p->y;
				}
			} وإلا {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			إذا (!layout_position_absolute(c, containing_بلاڪ,
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
ساكن فراغ layout_compute_relative_وٿي(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *box,
		صحيح *x,
		صحيح *y)
{
	صحيح کاٻو, ساڄو, چوٽي, bottom;
	بنية box *containing_بلاڪ;

	أكد(box && box->والد && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	إذا (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_کاٻو ||
			css_computed_float(box->style) == CSS_FLOAT_ساڄو)) {
		containing_بلاڪ = box->float_container;
	} وإلا {
		containing_بلاڪ = box->والد;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_بلاڪ,
			&چوٽي, &ساڄو, &bottom, &کاٻو);

	إذا (کاٻو == AUTO && ساڄو == AUTO)
		کاٻو = ساڄو = 0;
	وإلا إذا (کاٻو == AUTO)
		/* left is auto => computed = -right */
		کاٻو = -ساڄو;
	وإلا إذا (ساڄو == AUTO)
		/* right is auto => computed = -left */
		ساڄو = -کاٻو;
	وإلا {
		/* over constrained => examine direction property
		 * of containing block */
		إذا (containing_بلاڪ->style &&
				css_computed_direction(
				containing_بلاڪ->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			کاٻو = -ساڄو;
		} وإلا {
			/* assume LTR in all other cases */
			ساڄو = -کاٻو;
		}
	}

	أكد(کاٻو == -ساڄو);

	إذا (چوٽي == AUTO && bottom == AUTO) {
		چوٽي = bottom = 0;
	} وإلا إذا (چوٽي == AUTO) {
		چوٽي = -bottom;
	} وإلا {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -چوٽي;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", کاٻو,
	      ساڄو, چوٽي, bottom);

	*x = کاٻو;
	*y = چوٽي;
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
		بنية box *پاڙ,
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

	أكد(پاڙ);

	/* Normal children */
	لكل (box = پاڙ->children; box; box = box->ايندڙ) {

		إذا (box->type == BOX_متن)
			استمر;

		/* If relatively positioned, get offsets */
		إذا (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_وٿي(
					unit_len_ctx, box, &x, &y);
		وإلا
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		إذا (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_کاٻو ||
				css_computed_float(box->style) ==
					CSS_FLOAT_ساڄو) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			لكل (fc = fp->float_children; fc; fc = fc->ايندڙ_float) {
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
		إذا (box->type == BOX_INLINE && box->inline_پڇاڙي) {
			بنية box *b;
			لكل (b = box->ايندڙ; b && b != box->inline_پڇاڙي;
					b = b->ايندڙ) {
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
	*desc_x0 = -box->border[کاٻو_2].ويڪر;
	*desc_y0 = -box->border[چوٽي_2].ويڪر;
	*desc_x1 = box->padding[کاٻو_2] + box->ويڪر + box->padding[ساڄو_2] +
			box->border[ساڄو_2].ويڪر;
	*desc_y1 = box->padding[چوٽي_2] + box->اوچائي + box->padding[BOTTOM] +
			box->border[BOTTOM].ويڪر;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	إذا (box->type == BOX_بلاڪ && box->style != عدم &&
			css_computed_حد_کان_مٿي_y(box->style) ==
					CSS_حد_کان_مٿي_VISIBLE &&
			box->object == عدم) {
		css_fixed font_سائيز = 0;
		css_unit font_unit = CSS_UNIT_PT;
		صحيح متن_اوچائي;

		css_computed_font_سائيز(box->style, &font_سائيز, &font_unit);
		متن_اوچائي = css_unit_len2device_px(box->style, unit_len_ctx,
				font_سائيز, font_unit);
		متن_اوچائي = FIXTOINT(متن_اوچائي * 3 / 4);
		*desc_y0 = (*desc_y0 < -متن_اوچائي) ? *desc_y0 : -متن_اوچائي;
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
		بنية box *ٻار,
		صحيح off_x,
		صحيح off_y)
{
	صحيح ٻار_desc_x0, ٻار_desc_y0, ٻار_desc_x1, ٻار_desc_y1;

	/* get coordinates of child relative to box */
	صحيح ٻار_x = ٻار->x - off_x;
	صحيح ٻار_y = ٻار->y - off_y;

	bool html_object = (ٻار->object &&
			content_get_type(ٻار->object) == CONTENT_HTML);

	تعداد css_حد_کان_مٿي_e حد_کان_مٿي_x = CSS_حد_کان_مٿي_VISIBLE;
	تعداد css_حد_کان_مٿي_e حد_کان_مٿي_y = CSS_حد_کان_مٿي_VISIBLE;

	إذا (ٻار->style != عدم) {
		حد_کان_مٿي_x = css_computed_حد_کان_مٿي_x(ٻار->style);
		حد_کان_مٿي_y = css_computed_حد_کان_مٿي_y(ٻار->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, ٻار,
			&ٻار_desc_x0, &ٻار_desc_y0,
			&ٻار_desc_x1, &ٻار_desc_y1);

	إذا (حد_کان_مٿي_x == CSS_حد_کان_مٿي_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		ٻار_desc_x0 = ٻار->descendant_x0;
		ٻار_desc_x1 = ٻار->descendant_x1;
	}
	إذا (حد_کان_مٿي_y == CSS_حد_کان_مٿي_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		ٻار_desc_y0 = ٻار->descendant_y0;
		ٻار_desc_y1 = ٻار->descendant_y1;
	}

	ٻار_desc_x0 += ٻار_x;
	ٻار_desc_y0 += ٻار_y;
	ٻار_desc_x1 += ٻار_x;
	ٻار_desc_y1 += ٻار_y;

	/* increase box's descendant bbox to contain descendants */
	إذا (ٻار_desc_x0 < box->descendant_x0)
		box->descendant_x0 = ٻار_desc_x0;
	إذا (ٻار_desc_y0 < box->descendant_y0)
		box->descendant_y0 = ٻار_desc_y0;
	إذا (box->descendant_x1 < ٻار_desc_x1)
		box->descendant_x1 = ٻار_desc_x1;
	إذا (box->descendant_y1 < ٻار_desc_y1)
		box->descendant_y1 = ٻار_desc_y1;
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
	بنية box *ٻار;

	أكد(box->ويڪر != UNKNOWN_ويڪر);
	أكد(box->اوچائي != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	إذا (box->object && content_get_type(box->object) == CONTENT_HTML) {
		إذا (box->descendant_x1 < content_get_ويڪر(box->object))
			box->descendant_x1 = content_get_ويڪر(box->object);
		إذا (box->descendant_y1 < content_get_اوچائي(box->object))
			box->descendant_y1 = content_get_اوچائي(box->object);
	}

	إذا (box->iframe != عدم) {
		صحيح x, y;
		box_coords(box, &x, &y);

		browser_دري_سيٽ_position(box->iframe, x, y);
		browser_دري_سيٽ_dimensions(box->iframe,
				box->ويڪر, box->اوچائي);
		browser_دري_reformat(box->iframe, true,
				box->ويڪر, box->اوچائي);
	}

	إذا (box->type == BOX_INLINE || box->type == BOX_متن)
		أرجع;

	إذا (box->type == BOX_INLINE_پڇاڙي) {
		box = box->inline_پڇاڙي;
		لكل (ٻار = box->ايندڙ; ٻار;
				ٻار = ٻار->ايندڙ) {
			إذا (ٻار->type == BOX_FLOAT_کاٻو ||
					ٻار->type == BOX_FLOAT_ساڄو)
				استمر;

			layout_update_descendant_bbox(unit_len_ctx, box, ٻار,
					box->x, box->y);

			إذا (ٻار == box->inline_پڇاڙي)
				اكسر;
		}
		أرجع;
	}

	إذا (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		أرجع;

	لكل (ٻار = box->children; ٻار; ٻار = ٻار->ايندڙ) {
		إذا (ٻار->type == BOX_FLOAT_کاٻو ||
				ٻار->type == BOX_FLOAT_ساڄو)
			استمر;

		layout_calculate_descendant_bboxes(unit_len_ctx, ٻار);

		إذا (box->style && css_computed_حد_کان_مٿي_x(box->style) ==
				CSS_حد_کان_مٿي_HIDDEN &&
				css_computed_حد_کان_مٿي_y(box->style) ==
				CSS_حد_کان_مٿي_HIDDEN)
			استمر;

		layout_update_descendant_bbox(unit_len_ctx, box, ٻار, 0, 0);
	}

	لكل (ٻار = box->float_children; ٻار; ٻار = ٻار->ايندڙ_float) {
		أكد(ٻار->type == BOX_FLOAT_کاٻو ||
				ٻار->type == BOX_FLOAT_ساڄو);

		layout_calculate_descendant_bboxes(unit_len_ctx, ٻار);

		layout_update_descendant_bbox(unit_len_ctx, box, ٻار, 0, 0);
	}

	إذا (box->list_marker) {
		ٻار = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, ٻار);

		layout_update_descendant_bbox(unit_len_ctx, box, ٻار, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, صحيح ويڪر, صحيح اوچائي)
{
	bool ret;
	بنية box *doc = content->layout;
	ثابت بنية gui_layout_جدول *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			ويڪر, اوچائي, nsurl_access(content_get_url(
					&content->بنياد)));

	layout_minmax_بلاڪ(doc, font_func, content);

	layout_بلاڪ_ڳوليو_dimensions(&content->unit_len_ctx,
			ويڪر, اوچائي, 0, 0, doc);
	doc->x = doc->margin[کاٻو_2] + doc->border[کاٻو_2].ويڪر;
	doc->y = doc->margin[چوٽي_2] + doc->border[چوٽي_2].ويڪر;
	ويڪر -= doc->margin[کاٻو_2] + doc->border[کاٻو_2].ويڪر +
			doc->padding[کاٻو_2] + doc->padding[ساڄو_2] +
			doc->border[ساڄو_2].ويڪر + doc->margin[ساڄو_2];
	إذا (ويڪر < 0) {
		ويڪر = 0;
	}
	doc->ويڪر = ويڪر;

	ret = layout_بلاڪ_context(doc, اوچائي, content);

	/* make <html> and <body> fill available height */
	إذا (doc->y + doc->padding[چوٽي_2] + doc->اوچائي + doc->padding[BOTTOM] +
			doc->border[BOTTOM].ويڪر + doc->margin[BOTTOM] <
			اوچائي) {
		doc->اوچائي = اوچائي - (doc->y + doc->padding[چوٽي_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].ويڪر +
				doc->margin[BOTTOM]);
		إذا (doc->children)
			doc->children->اوچائي = doc->اوچائي -
					(doc->children->margin[چوٽي_2] +
					 doc->children->border[چوٽي_2].ويڪر +
					 doc->children->padding[چوٽي_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].ويڪر +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	أرجع ret;
}
