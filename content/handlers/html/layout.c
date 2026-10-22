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
	[بالاترین_2]    = css_computed_margin_بالاترین,
	[راست_2]  = css_computed_margin_راست,
	[BOTTOM] = css_computed_margin_bottom,
	[چپ_2]   = css_computed_margin_چپ,
};

/** Array of per-side access functions for computed style paddings. */
ثابت css_len_func padding_funcs[4] = {
	[بالاترین_2]    = css_computed_padding_بالاترین,
	[راست_2]  = css_computed_padding_راست,
	[BOTTOM] = css_computed_padding_bottom,
	[چپ_2]   = css_computed_padding_چپ,
};

/** Array of per-side access functions for computed style border_widths. */
ثابت css_len_func border_عرض_funcs[4] = {
	[بالاترین_2]    = css_computed_border_بالاترین_عرض,
	[راست_2]  = css_computed_border_راست_عرض,
	[BOTTOM] = css_computed_border_bottom_عرض,
	[چپ_2]   = css_computed_border_چپ_عرض,
};

/** Array of per-side access functions for computed style border styles. */
ثابت css_border_style_func border_style_funcs[4] = {
	[بالاترین_2]    = css_computed_border_بالاترین_style,
	[راست_2]  = css_computed_border_راست_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[چپ_2]   = css_computed_border_چپ_style,
};

/** Array of per-side access functions for computed style border colors. */
ثابت css_border_color_func border_color_funcs[4] = {
	[بالاترین_2]    = css_computed_border_بالاترین_color,
	[راست_2]  = css_computed_border_راست_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[چپ_2]   = css_computed_border_چپ_color,
};

/* forward declaration to break cycles */
ساكن فراغ layout_minmax_بلوک(
		بنية box *بلوک,
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
			     صحيح *عرض, صحيح *ارتفاع,
			     صحيح min_عرض, صحيح max_عرض,
			     صحيح min_ارتفاع, صحيح max_ارتفاع)
{
	أكد(box->object != عدم);
	أكد(عرض != عدم && ارتفاع != عدم);

	إذا (*عرض == AUTO && *ارتفاع == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		صحيح intrinsic_عرض = content_get_عرض(box->object);
		صحيح intrinsic_ارتفاع = content_get_ارتفاع(box->object);

		/* use intrinsic dimensions */
		*عرض = intrinsic_عرض;
		*ارتفاع = intrinsic_ارتفاع;

		/* Deal with min/max-width first */
		إذا (min_عرض >  0 && min_عرض > *عرض) {
			*عرض = min_عرض;
			scaled = true;
		}
		إذا (max_عرض >= 0 && max_عرض < *عرض) {
			*عرض = max_عرض;
			scaled = true;
		}

		إذا (scaled && (intrinsic_عرض != 0)) {
			/* Update height */
			*ارتفاع = (*عرض * intrinsic_ارتفاع) /
					intrinsic_عرض;
		}

		scaled = false;
		/* Deal with min/max-height */
		إذا (min_ارتفاع >  0 && min_ارتفاع > *ارتفاع) {
			*ارتفاع = min_ارتفاع;
			scaled = true;
		}
		إذا (max_ارتفاع >= 0 && max_ارتفاع < *ارتفاع) {
			*ارتفاع = max_ارتفاع;
			scaled = true;
		}

		إذا (scaled && (intrinsic_ارتفاع != 0)) {
			/* Update width */
			*عرض = (*ارتفاع * intrinsic_عرض) /
					intrinsic_ارتفاع;
		}

	} وإلا إذا (*عرض == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		صحيح intrinsic_عرض = content_get_عرض(box->object);
		صحيح intrinsic_ارتفاع = content_get_ارتفاع(box->object);

		إذا (intrinsic_ارتفاع != 0)
			*عرض = (*ارتفاع * intrinsic_عرض) /
					intrinsic_ارتفاع;
		وإلا
			*عرض = intrinsic_عرض;

		إذا (min_عرض >  0 && min_عرض > *عرض)
			*عرض = min_عرض;
		إذا (max_عرض >= 0 && max_عرض < *عرض)
			*عرض = max_عرض;

	} وإلا إذا (*ارتفاع == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		صحيح intrinsic_عرض = content_get_عرض(box->object);
		صحيح intrinsic_ارتفاع = content_get_ارتفاع(box->object);

		إذا (min_عرض >  0 && min_عرض > *عرض)
			*عرض = min_عرض;
		إذا (max_عرض >= 0 && max_عرض < *عرض)
			*عرض = max_عرض;

		إذا (intrinsic_عرض != 0)
			*ارتفاع = (*عرض * intrinsic_ارتفاع) /
					intrinsic_عرض;
		وإلا
			*ارتفاع = intrinsic_ارتفاع;
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
		ثابت css_computed_style *style, صحيح عرض)
{
	css_fixed مقدار = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_متن_indent(style, &مقدار, &unit);

	إذا (unit == CSS_UNIT_PCT) {
		أرجع FPCT_از_INT_TOINT(مقدار, عرض);
	} وإلا {
		أرجع FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				مقدار, unit));
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
	بیعلامت صحيح i, j;
	صحيح عرض;
	صحيح border_spacing_h = 0;
	صحيح جدول_min = 0, جدول_max = 0;
	صحيح extra_fixed = 0;
	عائم extra_frac = 0;
	بنية ستونی *ستون;
	بنية box *ردیف_گروه, *ردیف, *cell;

	/* check if the widths have already been calculated */
	إذا (جدول->max_عرض != UNKNOWN_MAX_عرض)
		أرجع;

	إذا (جدول_calculate_ستونی_types(&content->unit_len_ctx, جدول) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		أرجع;
	}
	ستون = جدول->ستون;

	/* start with 0 except for fixed-width columns */
	لكل (i = 0; i != جدول->columns; i++) {
		إذا (ستون[i].type == ستونی_عرض_FIXED)
			ستون[i].min = ستون[i].max = ستون[i].عرض;
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
	لكل (ردیف_گروه = جدول->children; ردیف_گروه; ردیف_گروه =ردیف_گروه->بعدی)
	لكل (ردیف = ردیف_گروه->children; ردیف; ردیف = ردیف->بعدی)
	لكل (cell = ردیف->children; cell; cell = cell->بعدی) {
		أكد(cell->type == BOX_جدول_CELL);
		أكد(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		أكد(cell->columns != 0);

		إذا (cell->columns != 1)
			استمر;

		layout_minmax_بلوک(cell, font_func, content);
		i = cell->شروع_ستونی;

		إذا (ستون[i].positioned)
			استمر;

		/* update column min, max widths using cell widths */
		إذا (ستون[i].min < cell->min_عرض)
			ستون[i].min = cell->min_عرض;
		إذا (ستون[i].max < cell->max_عرض)
			ستون[i].max = cell->max_عرض;
	}

	/* 2nd pass: cells which span multiple columns */
	لكل (ردیف_گروه = جدول->children; ردیف_گروه; ردیف_گروه =ردیف_گروه->بعدی)
	لكل (ردیف = ردیف_گروه->children; ردیف; ردیف = ردیف->بعدی)
	لكل (cell = ردیف->children; cell; cell = cell->بعدی) {
		بیعلامت صحيح flexible_columns = 0;
		صحيح min = 0, max = 0, fixed_عرض = 0, extra;

		إذا (cell->columns == 1)
			استمر;

		layout_minmax_بلوک(cell, font_func, content);
		i = cell->شروع_ستونی;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		لكل (j = 0; j != cell->columns; j++) {
			min += ستون[i + j].min;
			إذا (ستون[i + j].type == ستونی_عرض_FIXED)
				fixed_عرض += ستون[i + j].عرض;
			وإلا
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		إذا (min < cell->min_عرض) {
			إذا (flexible_columns == 0) {
				extra = 1 + (cell->min_عرض - min) /
						cell->columns;
				لكل (j = 0; j != cell->columns; j++) {
					ستون[i + j].min += extra;
					إذا (ستون[i + j].max < ستون[i + j].min)
						ستون[i + j].max = ستون[i + j].min;
				}
			} وإلا {
				extra = 1 + (cell->min_عرض - min) /
						flexible_columns;
				لكل (j = 0; j != cell->columns; j++) {
					إذا (ستون[i + j].type !=
							ستونی_عرض_FIXED) {
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
		إذا (max < cell->max_عرض && flexible_columns) {
			extra = 1 + (cell->max_عرض - max) / flexible_columns;
			لكل (j = 0; j != cell->columns; j++)
				إذا (ستون[i + j].type != ستونی_عرض_FIXED)
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
	إذا (css_computed_عرض_px(جدول->style, &content->unit_len_ctx,
			-1, &عرض) == CSS_عرض_مجموعه) {
		إذا (جدول_min < عرض)
			جدول_min = عرض;
		إذا (جدول_max < عرض)
			جدول_max = عرض;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_عرض(&content->unit_len_ctx,
			جدول->style, چپ_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_عرض(&content->unit_len_ctx,
			جدول->style, راست_2, true, true, true,
			&extra_fixed, &extra_frac);
	إذا (extra_fixed < 0)
		extra_fixed = 0;
	إذا (extra_frac < 0)
		extra_frac = 0;
	إذا (1.0 <= extra_frac)
		extra_frac = 0.9;
	جدول->min_عرض = (جدول_min + extra_fixed) / (1.0 - extra_frac);
	جدول->max_عرض = (جدول_max + extra_fixed) / (1.0 - extra_frac);
	جدول->min_عرض += (جدول->columns + 1) * border_spacing_h;
	جدول->max_عرض += (جدول->columns + 1) * border_spacing_h;

	أكد(0 <= جدول->min_عرض && جدول->min_عرض <= جدول->max_عرض);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
ساكن مضمن bool box_has_percentage_max_عرض(بنية box *b)
{
	css_unit unit = CSS_UNIT_PX;
	تعداد css_max_عرض_e type;
	css_fixed مقدار = 0;

	أكد(b != عدم);

	type = css_computed_max_عرض(b->style, &مقدار, &unit);
	أرجع ((type == CSS_MAX_عرض_مجموعه) && (unit == CSS_UNIT_PCT));
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
layout_minmax_سطر(بنية box *اولی,
		   صحيح *سطر_min,
		   صحيح *سطر_max,
		   bool اولی_سطر,
		   bool *سطر_has_ارتفاع,
		   ثابت بنية gui_layout_جدول *font_func,
		   ثابت html_content *content)
{
	صحيح min = 0, max = 0, عرض, ارتفاع, fixed;
	عائم frac;
	size_t i, j;
	بنية box *b;
	بنية box *بلوک;
	plot_font_style_t fstyle;
	bool no_wrap;

	أكد(اولی->والد);
	أكد(اولی->والد->والد);
	أكد(اولی->والد->والد->style);

	بلوک = اولی->والد->والد;
	no_wrap = (css_computed_white_space(بلوک->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(بلوک->style) ==
			CSS_WHITE_SPACE_PRE);

	*سطر_has_ارتفاع = false;

	/* corresponds to the pass 1 loop in layout_line() */
	لكل (b = اولی; b; b = b->بعدی) {
		تعداد css_عرض_e wtype;
		تعداد css_ارتفاع_e htype;
		تعداد css_box_sizing_e bs;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		أكد(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		إذا (b->type == BOX_BR) {
			b = b->بعدی;
			اكسر;
		}

		إذا (lh__box_is_float_box(b)) {
			أكد(b->children);
			إذا (b->children->type == BOX_جدول)
				layout_minmax_جدول(b->children, font_func,
						content);
			وإلا
				layout_minmax_بلوک(b->children, font_func,
						content);
			b->min_عرض = b->children->min_عرض;
			b->max_عرض = b->children->max_عرض;
			إذا (min < b->min_عرض)
				min = b->min_عرض;
			max += b->max_عرض;
			استمر;
		}

		إذا (b->type == BOX_INLINE_بلوک || b->type == BOX_INLINE_FLEX) {
			layout_minmax_بلوک(b, font_func, content);
			إذا (min < b->min_عرض)
				min = b->min_عرض;
			max += b->max_عرض;

			إذا (b->flags & HAS_ارتفاع)
				*سطر_has_ارتفاع = true;
			استمر;
		}

		أكد(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		إذا (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_عرض(&content->unit_len_ctx,
					b->style, چپ_2, true, true, true,
					&fixed, &frac);
			إذا (!b->inline_انتها)
				calculate_mbp_عرض(&content->unit_len_ctx,
						b->style, راست_2,
						true, true, true,
						&fixed, &frac);
			إذا (0 < fixed)
				max += fixed;
			*سطر_has_ارتفاع = true;
			/* \todo  update min width, consider fractional extra */
		} وإلا إذا (b->type == BOX_INLINE_انتها) {
			fixed = frac = 0;
			calculate_mbp_عرض(&content->unit_len_ctx,
					b->inline_انتها->style, راست_2,
					true, true, true,
					&fixed, &frac);
			إذا (0 < fixed)
				max += fixed;

			إذا (b->بعدی) {
				إذا (b->space == UNKNOWN_عرض) {
					font_func->عرض(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*سطر_has_ارتفاع = true;
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

			إذا (b->عرض == UNKNOWN_عرض) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				إذا (b->والد->والد->gadget &&
						b->والد->والد->gadget->type
						== GADGET_SELECT) {
					صحيح opt_maxwidth = 0;
					بنية form_option *o;

					لكل (o = b->والد->والد->gadget->
							داده.select.اقلام; o;
							o = o->بعدی) {
						صحيح opt_عرض;
						font_func->عرض(&fstyle,
								o->متن,
								طول_رشته(o->متن),
								&opt_عرض);

						إذا (opt_maxwidth < opt_عرض)
							opt_maxwidth =opt_عرض;
					}

					b->عرض = opt_maxwidth;
					إذا (nsoption_bool(core_select_menu))
						b->عرض += SCROLLBAR_عرض;

				} وإلا {
					font_func->عرض(&fstyle, b->متن,
						b->درازی, &b->عرض);
					b->flags |= MEASURED;
				}
			}
			max += b->عرض;
			إذا (b->بعدی) {
				إذا (b->space == UNKNOWN_عرض) {
					font_func->عرض(&fstyle, " ", 1,
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
				إذا (min < b->عرض)
					min = b->عرض;

			} وإلا إذا (b->والد->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				افعل {
					لكل (j = i; j != b->درازی &&
							b->متن[j] != ' '; j++)
						;
					font_func->عرض(&fstyle, b->متن + i,
							 j - i, &عرض);
					إذا (min < عرض)
						min = عرض;
					i = j + 1;
				} طالما (j != b->درازی);
			}

			*سطر_has_ارتفاع = true;

			استمر;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		bs = css_computed_box_sizing(بلوک->style);

		/* calculate box width */
		wtype = css_computed_عرض_px(b->style,
				&content->unit_len_ctx, -1, &عرض);
		إذا (wtype == CSS_عرض_مجموعه) {
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_عرض(&content->unit_len_ctx,
						بلوک->style, چپ_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_عرض(&content->unit_len_ctx,
						بلوک->style, راست_2,
						false, true, true,
						&fixed, &frac);
				إذا (عرض < fixed) {
					عرض = fixed;
				}
			}
			إذا (عرض < 0)
				عرض = 0;
		} وإلا {
			عرض = AUTO;
		}

		/*ارتفاع*/
		htype = css_computed_ارتفاع(b->style, &مقدار, &unit);
		إذا (htype == CSS_ارتفاع_مجموعه) {
			ارتفاع = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					مقدار, unit));
		} وإلا {
			ارتفاع = AUTO;
		}

		إذا (b->object || (b->flags & REPLACE_DIM)) {
			إذا (b->object) {
				صحيح موقت_ارتفاع = ارتفاع;
				layout_get_object_dimensions(b,
						&عرض, &موقت_ارتفاع,
						INT_MIN, بیشترین_صحیح,
						INT_MIN, بیشترین_صحیح);
			}

			fixed = frac = 0;
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_عرض(&content->unit_len_ctx,
						b->style, چپ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_عرض(&content->unit_len_ctx,
						b->style, راست_2,
						true, false, false,
						&fixed, &frac);
			} وإلا {
				calculate_mbp_عرض(&content->unit_len_ctx,
						b->style, چپ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_عرض(&content->unit_len_ctx,
						b->style, راست_2,
						true, true, true,
						&fixed, &frac);
			}
			إذا (0 < عرض + fixed)
				عرض += fixed;
		} وإلا إذا (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			إذا (عرض == AUTO)
				عرض = 400;

			fixed = frac = 0;
			إذا (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_عرض(&content->unit_len_ctx,
						b->style, چپ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_عرض(&content->unit_len_ctx,
						b->style, راست_2,
						true, false, false,
						&fixed, &frac);
			} وإلا {
				calculate_mbp_عرض(&content->unit_len_ctx,
						b->style, چپ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_عرض(&content->unit_len_ctx,
						b->style, راست_2,
						true, true, true,
						&fixed, &frac);
			}

			إذا (0 < عرض + fixed)
				عرض += fixed;

		} وإلا {
			/* form control with no object */
			إذا (عرض == AUTO)
				عرض = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		إذا (min < عرض && !box_has_percentage_max_عرض(b))
			min = عرض;
		إذا (عرض > 0)
			max += عرض;

		*سطر_has_ارتفاع = true;
	}

	إذا (اولی_سطر) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		صحيح متن_indent = layout_متن_indent(&content->unit_len_ctx,
				اولی->والد->والد->style, 100);
		min = (min + متن_indent < 0) ? 0 : min + متن_indent;
		max = (max + متن_indent < 0) ? 0 : max + متن_indent;
	}

	*سطر_min = min;
	*سطر_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	أكد(b != اولی);
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
			       bool *has_ارتفاع,
			       ثابت بنية gui_layout_جدول *font_func,
			       ثابت html_content *content)
{
	بنية box *فرزند;
	صحيح سطر_min = 0, سطر_max = 0;
	صحيح min = 0, max = 0;
	bool اولی_سطر = true;
	bool سطر_has_ارتفاع;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	إذا (inline_container->max_عرض != UNKNOWN_MAX_عرض)
		أرجع;

	*has_ارتفاع = false;

	لكل (فرزند = inline_container->children; فرزند; ) {
		فرزند = layout_minmax_سطر(فرزند, &سطر_min, &سطر_max,
				اولی_سطر, &سطر_has_ارتفاع, font_func,
				content);
		إذا (min < سطر_min)
			min = سطر_min;
		إذا (max < سطر_max)
			max = سطر_max;
		اولی_سطر = false;
		*has_ارتفاع |= سطر_has_ارتفاع;
	}

	inline_container->min_عرض = min;
	inline_container->max_عرض = max;

	أكد(0 <= inline_container->min_عرض &&
			inline_container->min_عرض <=
			inline_container->max_عرض);
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
ساكن فراغ layout_minmax_بلوک(
		بنية box *بلوک,
		ثابت بنية gui_layout_جدول *font_func,
		ثابت html_content *content)
{
	بنية box *فرزند;
	صحيح min = 0, max = 0;
	صحيح extra_fixed = 0;
	عائم extra_frac = 0;
	تعداد css_عرض_e wtype = CSS_عرض_AUTO;
	css_fixed عرض = 0;
	css_unit wunit = CSS_UNIT_PX;
	تعداد css_ارتفاع_e htype = CSS_ارتفاع_AUTO;
	css_fixed ارتفاع = 0;
	css_unit hunit = CSS_UNIT_PX;
	تعداد css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool فرزند_has_ارتفاع = false;

	أكد(بلوک->type == BOX_بلوک ||
			بلوک->type == BOX_FLEX ||
			بلوک->type == BOX_INLINE_FLEX ||
			بلوک->type == BOX_INLINE_بلوک ||
			بلوک->type == BOX_جدول_CELL);

	/* check if the widths have already been calculated */
	إذا (بلوک->max_عرض != UNKNOWN_MAX_عرض)
		أرجع;

	إذا (بلوک->style != عدم) {
		wtype = css_computed_عرض(بلوک->style, &عرض, &wunit);
		htype = css_computed_ارتفاع(بلوک->style, &ارتفاع, &hunit);
		bs = css_computed_box_sizing(بلوک->style);
	}

	/* set whether the minimum width is of any interest for this box */
	إذا (((بلوک->والد && lh__box_is_float_box(بلوک->والد)) ||
			بلوک->type == BOX_INLINE_بلوک ||
			بلوک->type == BOX_INLINE_FLEX) &&
			wtype != CSS_عرض_مجموعه) {
		/* box shrinks to fit; need minimum width */
		بلوک->flags |= NEED_MIN;
	} وإلا إذا (بلوک->type == BOX_جدول_CELL) {
		/* box shrinks to fit; need minimum width */
		بلوک->flags |= NEED_MIN;
	} وإلا إذا (بلوک->والد && (بلوک->والد->flags & NEED_MIN) &&
			wtype != CSS_عرض_مجموعه) {
		/* box inside shrink-to-fit context; need minimum width */
		بلوک->flags |= NEED_MIN;
	} وإلا إذا (بلوک->والد && (بلوک->والد->type == BOX_FLEX)) {
		/* box is flex item */
		بلوک->flags |= NEED_MIN;
	}

	إذا (بلوک->gadget && (بلوک->gadget->type == GADGET_TEXTBOX ||
			بلوک->gadget->type == GADGET_PASSWORD ||
			بلوک->gadget->type == GADGET_FILE ||
			بلوک->gadget->type == GADGET_TEXTAREA) &&
			بلوک->style && wtype == CSS_عرض_AUTO) {
		css_fixed اندازه = INTTOFIX(10);
		css_unit unit = CSS_UNIT_EM;

		min = max = FIXTOINT(css_unit_len2device_px(بلوک->style,
				&content->unit_len_ctx, اندازه, unit));

		بلوک->flags |= HAS_ارتفاع;
	}

	إذا (بلوک->gadget && (بلوک->gadget->type == GADGET_RADIO ||
			بلوک->gadget->type == GADGET_CHECKBOX) &&
			بلوک->style && wtype == CSS_عرض_AUTO) {
		css_fixed اندازه = INTTOFIX(1);
		css_unit unit = CSS_UNIT_EM;

		/* form checkbox or radio button
		 * if width is AUTO, set it to 1em */
		min = max = FIXTOINT(css_unit_len2device_px(بلوک->style,
				&content->unit_len_ctx, اندازه, unit));

		بلوک->flags |= HAS_ارتفاع;
	}

	إذا (بلوک->object) {
		إذا (content_get_type(بلوک->object) == CONTENT_HTML) {
			layout_minmax_بلوک(html_get_box_درخت(بلوک->object),
					font_func, content);
			min = html_get_box_درخت(بلوک->object)->min_عرض;
			max = html_get_box_درخت(بلوک->object)->max_عرض;
		} وإلا {
			min = max = content_get_عرض(بلوک->object);
		}

		بلوک->flags |= HAS_ارتفاع;
	} وإلا إذا (بلوک->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		بلوک->flags |= HAS_ارتفاع;
	} وإلا {
		/* recurse through children */
		لكل (فرزند = بلوک->children; فرزند; فرزند = فرزند->بعدی) {
			بدل (فرزند->type) {
			حالة BOX_FLEX:
			حالة BOX_بلوک:
				layout_minmax_بلوک(فرزند, font_func,
						content);
				إذا (فرزند->flags & HAS_ارتفاع)
					فرزند_has_ارتفاع = true;
				اكسر;
			حالة BOX_INLINE_CONTAINER:
				إذا (بلوک->flags & NEED_MIN)
					فرزند->flags |= NEED_MIN;

				layout_minmax_inline_container(فرزند,
						&فرزند_has_ارتفاع, font_func,
						content);
				إذا (فرزند_has_ارتفاع &&
						فرزند ==
						فرزند->والد->children) {
					بلوک->flags |= MAKE_ارتفاع;
				}
				اكسر;
			حالة BOX_جدول:
				layout_minmax_جدول(فرزند, font_func,
						content);
				/* todo: fix for zero height tables */
				فرزند_has_ارتفاع = true;
				فرزند->flags |= MAKE_ارتفاع;
				اكسر;
			افتراضي:
				أكد(0);
			}
			أكد(فرزند->max_عرض != UNKNOWN_MAX_عرض);

			إذا (فرزند->style &&
					(css_computed_position(فرزند->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(فرزند->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				استمر;
			}

			إذا (lh__box_is_flex_container(بلوک) &&
			    lh__flex_main_is_horizontal(بلوک)) {
				إذا (بلوک->style != عدم &&
				    css_computed_flex_wrap(بلوک->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += فرزند->min_عرض;
				} وإلا {
					إذا (min < فرزند->min_عرض)
						min = فرزند->min_عرض;
				}
				max += فرزند->max_عرض;

			} وإلا {
				إذا (min < فرزند->min_عرض)
					min = فرزند->min_عرض;
				إذا (max < فرزند->max_عرض)
					max = فرزند->max_عرض;
			}

			إذا (فرزند_has_ارتفاع)
				بلوک->flags |= HAS_ارتفاع;
		}
	}

	إذا (max < min) {
		box_dump(stderr, بلوک, 0, true);
		أكد(0);
	}

	/* fixed width takes priority */
	إذا (بلوک->type != BOX_جدول_CELL && !lh_box_is_flex_قلم(بلوک)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		تعداد css_max_عرض_e max_type;
		تعداد css_min_عرض_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed مقدار = 0;
		صحيح عرض;

		إذا (css_computed_عرض_px(بلوک->style, &content->unit_len_ctx,
				-1, &عرض) == CSS_عرض_مجموعه) {
			min = max = عرض;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_عرض(بلوک->style, &مقدار, &unit);
		إذا (min_type == CSS_MIN_عرض_مجموعه && unit != CSS_UNIT_PCT) {
			صحيح val = FIXTOINT(css_unit_len2device_px(بلوک->style,
					&content->unit_len_ctx, مقدار, unit));

			إذا (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_عرض(بلوک->style, &مقدار, &unit);
		إذا (max_type == CSS_MAX_عرض_مجموعه && unit != CSS_UNIT_PCT) {
			صحيح val = FIXTOINT(css_unit_len2device_px(بلوک->style,
					&content->unit_len_ctx, مقدار, unit));

			إذا (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	إذا (htype == CSS_ارتفاع_مجموعه && hunit != CSS_UNIT_PCT &&
			ارتفاع > INTTOFIX(0)) {
		بلوک->flags |= MAKE_ارتفاع;
		بلوک->flags |= HAS_ارتفاع;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_عرض(&content->unit_len_ctx, بلوک->style, چپ_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_عرض(&content->unit_len_ctx, بلوک->style, راست_2,
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

	calculate_mbp_عرض(&content->unit_len_ctx, بلوک->style, چپ_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_عرض(&content->unit_len_ctx, بلوک->style, راست_2,
			true, false, false, &extra_fixed, &extra_frac);

	إذا (extra_fixed < 0)
		extra_fixed = 0;
	إذا (extra_frac < 0)
		extra_frac = 0;
	إذا (1.0 <= extra_frac)
		extra_frac = 0.9;
	إذا (بلوک->style != عدم &&
			(css_computed_float(بلوک->style) == CSS_FLOAT_چپ ||
			css_computed_float(بلوک->style) == CSS_FLOAT_راست)) {
		/* floated boxs */
		بلوک->min_عرض = min + extra_fixed;
		بلوک->max_عرض = max + extra_fixed;
	} وإلا {
		/* not floated */
		بلوک->min_عرض = (min + extra_fixed) / (1.0 - extra_frac);
		بلوک->max_عرض = (max + extra_fixed) / (1.0 - extra_frac);
	}

	أكد(0 <= بلوک->min_عرض);
	أكد(بلوک->min_عرض <= بلوک->max_عرض);
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
layout_بعدی_margin_بلوک(ثابت css_unit_ctx *unit_len_ctx,
			 بنية box *box,
			 بنية box *بلوک,
			 صحيح viewport_ارتفاع,
			 صحيح *max_pos_margin,
			 صحيح *max_neg_margin)
{
	أكد(بلوک != عدم);

	طالما (box != عدم) {

		إذا (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			إذا (box->style) {
				layout_پیدا_کن_dimensions(unit_len_ctx,
						box->والد->عرض,
						viewport_ارتفاع, box,
						box->style,
						عدم, عدم, عدم, عدم,
						عدم, عدم, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				إذا (*max_pos_margin < box->margin[بالاترین_2])
					*max_pos_margin = box->margin[بالاترین_2];
				وإلا إذا (*max_neg_margin < -box->margin[بالاترین_2])
					*max_neg_margin = -box->margin[بالاترین_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			إذا (box->flags & MAKE_ارتفاع ||
					box->border[بالاترین_2].عرض ||
					box->padding[بالاترین_2] ||
					(box->style &&
					css_computed_سرریز_y(box->style) !=
					CSS_سرریز_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_اولی_فرزند(box))) {
				/* Collapse to this box; return it */
				أرجع box;
			}
		}


		/* Find next box */
		إذا (box->type == BOX_بلوک && !box->object && box->children &&
				box->style &&
				css_computed_سرریز_y(box->style) ==
				CSS_سرریز_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} وإلا {
			إذا (!box->بعدی) {
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
				} طالما (box != بلوک && !box->بعدی);

				إذا (box == بلوک) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					أرجع بلوک;
				}
			}

			/* Apply bottom margin */
			إذا (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			وإلا إذا (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->بعدی;

			/* Get margins */
			إذا (box->style) {
				layout_پیدا_کن_dimensions(unit_len_ctx,
						box->والد->عرض,
						viewport_ارتفاع, box,
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
	لكل (; fl; fl = fl->بعدی_float) {
		إذا ((clear == CSS_CLEAR_چپ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_چپ)
			إذا (y < fl->y + fl->ارتفاع)
				y = fl->y + fl->ارتفاع;
		إذا ((clear == CSS_CLEAR_راست || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_راست)
			إذا (y < fl->y + fl->ارتفاع)
				y = fl->y + fl->ارتفاع;
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
پیدا_کن_sides(بنية box *fl,
	   صحيح y0, صحيح y1,
	   صحيح *x0, صحيح *x1,
	   بنية box **چپ,
	   بنية box **راست)
{
	صحيح fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*چپ = *راست = 0;
	لكل (; fl; fl = fl->بعدی_float) {
		fy1 = fl->y + fl->ارتفاع;
		إذا (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			أرجع;
		}
		fy0 = fl->y;
		إذا (y0 < fy1 && fy0 <= y1) {
			إذا (fl->type == BOX_FLOAT_چپ) {
				fx1 = fl->x + fl->عرض;
				إذا (*x0 < fx1) {
					*x0 = fx1;
					*چپ = fl;
				}
			} وإلا {
				fx0 = fl->x;
				إذا (fx0 < *x1) {
					*x1 = fx0;
					*راست = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *چپ, *راست);
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
layout_solve_عرض(بنية box *box,
		   صحيح موجود_عرض,
		   صحيح عرض,
		   صحيح lm,
		   صحيح rm,
		   صحيح max_عرض,
		   صحيح min_عرض)
{
	bool auto_عرض = false;

	/* Increase specified left/right margins */
	إذا (box->margin[چپ_2] != AUTO && box->margin[چپ_2] < lm &&
			box->margin[چپ_2] >= 0)
		box->margin[چپ_2] = lm;
	إذا (box->margin[راست_2] != AUTO && box->margin[راست_2] < rm &&
			box->margin[راست_2] >= 0)
		box->margin[راست_2] = rm;

	/* Find width */
	إذا (عرض == AUTO) {
		صحيح margin_چپ = box->margin[چپ_2];
		صحيح margin_راست = box->margin[راست_2];

		إذا (margin_چپ == AUTO) {
			margin_چپ = lm;
		}
		إذا (margin_راست == AUTO) {
			margin_راست = rm;
		}

		عرض = موجود_عرض -
				(margin_چپ + box->border[چپ_2].عرض +
				box->padding[چپ_2] + box->padding[راست_2] +
				box->border[راست_2].عرض + margin_راست);
		عرض = عرض < 0 ? 0 : عرض;
		auto_عرض = true;
	}

	إذا (max_عرض >= 0 && عرض > max_عرض) {
		/* max-width is admissable and width exceeds max-width */
		عرض = max_عرض;
		auto_عرض = false;
	}

	إذا (min_عرض > 0 && عرض < min_عرض) {
		/* min-width is admissable and width is less than max-width */
		عرض = min_عرض;
		auto_عرض = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	إذا (auto_عرض) {
		/* any other 'auto' become 0 or the minimum required values */
		إذا (box->margin[چپ_2] == AUTO) {
			box->margin[چپ_2] = lm;
		}
		إذا (box->margin[راست_2] == AUTO) {
			box->margin[راست_2] = rm;
		}
		أرجع عرض;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	إذا (box->margin[چپ_2] != AUTO && box->margin[راست_2] != AUTO &&
			box->والد != عدم && box->والد->style != عدم) {
		بدل (css_computed_متن_align(box->والد->style)) {
		حالة CSS_متن_ALIGN_LIBCSS_راست:
			box->margin[چپ_2] = AUTO;
			box->margin[راست_2] = 0;
			اكسر;
		حالة CSS_متن_ALIGN_LIBCSS_CENTER:
			box->margin[چپ_2] = box->margin[راست_2] = AUTO;
			اكسر;
		حالة CSS_متن_ALIGN_LIBCSS_چپ:
			box->margin[چپ_2] = 0;
			box->margin[راست_2] = AUTO;
			اكسر;
		افتراضي:
			/* Leave it alone; no HTML alignment */
			اكسر;
		}
	}

	إذا (box->margin[چپ_2] == AUTO && box->margin[راست_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[چپ_2] = box->margin[راست_2] =
				(موجود_عرض - lm - rm -
				(box->border[چپ_2].عرض + box->padding[چپ_2] +
				عرض + box->padding[راست_2] +
				box->border[راست_2].عرض)) / 2;

		إذا (box->margin[چپ_2] < 0) {
			box->margin[راست_2] += box->margin[چپ_2];
			box->margin[چپ_2] = 0;
		}

		box->margin[چپ_2] += lm;

	} وإلا إذا (box->margin[چپ_2] == AUTO) {
		box->margin[چپ_2] = موجود_عرض - lm -
				(box->border[چپ_2].عرض + box->padding[چپ_2] +
				عرض + box->padding[راست_2] +
				box->border[راست_2].عرض + box->margin[راست_2]);
		box->margin[چپ_2] = box->margin[چپ_2] < lm
				? lm : box->margin[چپ_2];
	} وإلا {
		/* margin-right auto or "over-constrained" */
		box->margin[راست_2] = موجود_عرض - rm -
				(box->margin[چپ_2] + box->border[چپ_2].عرض +
				 box->padding[چپ_2] + عرض +
				 box->padding[راست_2] +
				 box->border[راست_2].عرض);
	}

	أرجع عرض;
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
layout_بلوک_پیدا_کن_dimensions(ثابت css_unit_ctx *unit_len_ctx,
			     صحيح موجود_عرض,
			     صحيح viewport_ارتفاع,
			     صحيح lm,
			     صحيح rm,
			     بنية box *box)
{
	صحيح عرض, max_عرض, min_عرض;
	صحيح ارتفاع, max_ارتفاع, min_ارتفاع;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	ثابت css_computed_style *style = box->style;

	layout_پیدا_کن_dimensions(unit_len_ctx, موجود_عرض, viewport_ارتفاع, box,
			style, &عرض, &ارتفاع, &max_عرض, &min_عرض,
			&max_ارتفاع, &min_ارتفاع, margin, padding, border);

	إذا (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &عرض, &ارتفاع,
				min_عرض, max_عرض, min_ارتفاع, max_ارتفاع);
	}

	box->عرض = layout_solve_عرض(box, موجود_عرض, عرض, lm, rm,
			max_عرض, min_عرض);
	box->ارتفاع = ارتفاع;

	إذا (margin[بالاترین_2] == AUTO)
		margin[بالاترین_2] = 0;
	إذا (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
ساكن فراغ layout_بلوک_جمع_کن_scrollbar(بنية box *box, صحيح which)
{
	تعداد css_سرریز_e سرریز_x, سرریز_y;

	أكد(box->type == BOX_بلوک && (which == راست_2 || which == BOTTOM));

	إذا (box->style == عدم)
		أرجع;

	سرریز_x = css_computed_سرریز_x(box->style);
	سرریز_y = css_computed_سرریز_y(box->style);

	إذا (which == BOTTOM &&
			(سرریز_x == CSS_سرریز_SCROLL ||
			 سرریز_x == CSS_سرریز_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		إذا (box->ارتفاع != AUTO &&
				(سرریز_x == CSS_سرریز_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_عرض;
		}

	} وإلا إذا (which == راست_2 &&
			(سرریز_y == CSS_سرریز_SCROLL ||
			 سرریز_y == CSS_سرریز_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		تعداد css_ارتفاع_e htype;
		css_fixed ارتفاع = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_ارتفاع(box->style, &ارتفاع, &hunit);

		إذا (which == راست_2 && box->عرض != AUTO &&
				htype == CSS_ارتفاع_مجموعه &&
				(سرریز_y == CSS_سرریز_SCROLL ||
				box_vscrollbar_present(box))) {
			box->عرض -= SCROLLBAR_عرض;
			box->padding[راست_2] += SCROLLBAR_عرض;
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

	لكل (box = box->children; box; box = box->بعدی) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_جدول(
		بنية box *جدول,
		صحيح موجود_عرض,
		html_content *content)
{
	بیعلامت صحيح columns = جدول->columns;  /* total columns */
	بیعلامت صحيح i;
	بیعلامت صحيح *ردیف_بازه;
	صحيح *excess_y;
	صحيح جدول_عرض, min_عرض = 0, max_عرض = 0;
	صحيح required_عرض = 0;
	صحيح x, remainder = 0, شمار = 0;
	صحيح جدول_ارتفاع = 0;
	صحيح min_ارتفاع = 0;
	صحيح *xs;  /* array of column x positions */
	صحيح auto_عرض;
	صحيح یدکی_عرض;
	صحيح relative_جمع = 0;
	صحيح border_spacing_h = 0, border_spacing_v = 0;
	صحيح یدکی_ارتفاع;
	صحيح positioned_columns = 0;
	بنية box *containing_بلوک = عدم;
	بنية box *c;
	بنية box *ردیف;
	بنية box *ردیف_گروه;
	بنية box **ردیف_بازه_cell;
	بنية ستونی *ستون;
	ثابت css_computed_style *style = جدول->style;
	تعداد css_ارتفاع_e htype;
	css_fixed مقدار = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(جدول->type == BOX_جدول);
	أكد(style);
	أكد(جدول->children && جدول->children->children);
	أكد(columns);

	/* allocate working buffers */
	ستون = حافظه_بده(columns * حجم ستون[0]);
	excess_y = حافظه_بده(columns * حجم excess_y[0]);
	ردیف_بازه = حافظه_بده(columns * حجم ردیف_بازه[0]);
	ردیف_بازه_cell = حافظه_بده(columns * حجم ردیف_بازه_cell[0]);
	xs = حافظه_بده((columns + 1) * حجم xs[0]);
	إذا (!ستون || !xs || !ردیف_بازه || !excess_y || !ردیف_بازه_cell) {
		حرر(ستون);
		حرر(excess_y);
		حرر(ردیف_بازه);
		حرر(ردیف_بازه_cell);
		حرر(xs);
		أرجع false;
	}

	کپی_حافظه(ستون, جدول->ستون, حجم(ستون[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_پیدا_کن_dimensions(&content->unit_len_ctx, موجود_عرض, -1, جدول,
			style, 0, 0, 0, 0, 0, 0, جدول->margin, جدول->padding,
			جدول->border);
	لكل (ردیف_گروه = جدول->children; ردیف_گروه;
			ردیف_گروه = ردیف_گروه->بعدی) {
		لكل (ردیف = ردیف_گروه->children; ردیف; ردیف = ردیف->بعدی) {
			لكل (c = ردیف->children; c; c = c->بعدی) {
				تعداد css_سرریز_e سرریز_x;
				تعداد css_سرریز_e سرریز_y;

				أكد(c->style);
				جدول_استفاده_شده_border_for_cell(
						&content->unit_len_ctx, c);
				layout_پیدا_کن_dimensions(&content->unit_len_ctx,
						موجود_عرض, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				سرریز_x = css_computed_سرریز_x(c->style);
				سرریز_y = css_computed_سرریز_y(c->style);

				إذا (سرریز_x == CSS_سرریز_SCROLL ||
						سرریز_x ==
						CSS_سرریز_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_عرض;
				}
				إذا (سرریز_y == CSS_سرریز_SCROLL ||
						سرریز_y ==
						CSS_سرریز_AUTO) {
					c->padding[راست_2] += SCROLLBAR_عرض;
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
	إذا (css_computed_عرض_px(style, &content->unit_len_ctx,
			موجود_عرض, &جدول_عرض) == CSS_عرض_مجموعه) {
		/* specified width includes border */
		جدول_عرض -= جدول->border[چپ_2].عرض +
				جدول->border[راست_2].عرض;
		جدول_عرض = جدول_عرض < 0 ? 0 : جدول_عرض;

		auto_عرض = جدول_عرض;
	} وإلا {
		جدول_عرض = AUTO;
		auto_عرض = موجود_عرض -
				((جدول->margin[چپ_2] == AUTO ? 0 :
						جدول->margin[چپ_2]) +
				 جدول->border[چپ_2].عرض +
				 جدول->padding[چپ_2] +
				 جدول->padding[راست_2] +
				 جدول->border[راست_2].عرض +
				 (جدول->margin[راست_2] == AUTO ? 0 :
						جدول->margin[راست_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_ارتفاع(style, &مقدار, &unit);
	إذا (htype == CSS_ارتفاع_مجموعه) {
		إذا (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			إذا (css_computed_position(جدول->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				أكد(جدول->float_container);
				containing_بلوک = جدول->float_container;
			} وإلا إذا (جدول->float_container &&
					css_computed_position(جدول->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(جدول->style) ==
						CSS_FLOAT_چپ ||
					css_computed_float(جدول->style) ==
						CSS_FLOAT_راست)) {
				/* Table is a float */
				أكد(جدول->والد && جدول->والد->والد &&
						جدول->والد->والد->والد);
				containing_بلوک =
						جدول->والد->والد->والد;
			} وإلا إذا (جدول->والد && جدول->والد->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_بلوک = جدول->والد;
			} وإلا إذا (جدول->والد && جدول->والد->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				أكد(جدول->والد->والد);
				containing_بلوک = جدول->والد->والد;
			}

			إذا (containing_بلوک) {
				css_fixed ignored = 0;

				htype = css_computed_ارتفاع(
						containing_بلوک->style,
						&ignored, &unit);
			}

			إذا (containing_بلوک &&
					containing_بلوک->ارتفاع != AUTO &&
					(css_computed_position(جدول->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_ارتفاع_مجموعه)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_ارتفاع = FPCT_از_INT_TOINT(مقدار,
						containing_بلوک->ارتفاع);
			}
		} وإلا {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_ارتفاع = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					مقدار, unit));
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
		      ستون[i].عرض,
		      ستون[i].min,
		      ستون[i].max);


		إذا (ستون[i].positioned) {
			positioned_columns++;
			استمر;
		} وإلا إذا (ستون[i].type == ستونی_عرض_FIXED) {
			إذا (ستون[i].عرض < ستون[i].min)
				ستون[i].عرض = ستون[i].max = ستون[i].min;
			وإلا
				ستون[i].min = ستون[i].max = ستون[i].عرض;
			required_عرض += ستون[i].عرض;
		} وإلا إذا (ستون[i].type == ستونی_عرض_PERCENT) {
			صحيح عرض = ستون[i].عرض * auto_عرض / 100;
			required_عرض += ستون[i].min < عرض ? عرض :
					ستون[i].min;
		} وإلا
			required_عرض += ستون[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_عرض);
	}
	required_عرض += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", جدول_عرض,
	      جدول->min_عرض, جدول->max_عرض, auto_عرض, required_عرض);

	إذا (auto_عرض < required_عرض) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		لكل (i = 0; i != columns; i++) {
			إذا (ستون[i].type == ستونی_عرض_RELATIVE)
				استمر;
			إذا (ستون[i].type == ستونی_عرض_PERCENT) {
				ستون[i].max = auto_عرض * ستون[i].عرض / 100;
				إذا (ستون[i].max < ستون[i].min)
					ستون[i].max = ستون[i].min;
			}
			min_عرض += ستون[i].min;
			max_عرض += ستون[i].max;
		}
	} وإلا {
		/* take percentages exactly */
		لكل (i = 0; i != columns; i++) {
			إذا (ستون[i].type == ستونی_عرض_RELATIVE)
				استمر;
			إذا (ستون[i].type == ستونی_عرض_PERCENT) {
				صحيح عرض = auto_عرض * ستون[i].عرض / 100;
				إذا (عرض < ستون[i].min)
					عرض = ستون[i].min;
				ستون[i].min = ستون[i].عرض = ستون[i].max = عرض;
				ستون[i].type = ستونی_عرض_FIXED;
			}
			min_عرض += ستون[i].min;
			max_عرض += ستون[i].max;
		}
	}

	/* allocate relative widths */
	یدکی_عرض = auto_عرض;
	لكل (i = 0; i != columns; i++) {
		إذا (ستون[i].type == ستونی_عرض_RELATIVE)
			relative_جمع += ستون[i].عرض;
		وإلا إذا (ستون[i].type == ستونی_عرض_FIXED)
			یدکی_عرض -= ستون[i].عرض;
		وإلا
			یدکی_عرض -= ستون[i].min;
	}
	یدکی_عرض -= (columns + 1) * border_spacing_h;
	إذا (relative_جمع != 0) {
		إذا (یدکی_عرض < 0)
			یدکی_عرض = 0;
		لكل (i = 0; i != columns; i++) {
			إذا (ستون[i].type == ستونی_عرض_RELATIVE) {
				ستون[i].min = ceil(ستون[i].max =
						(عائم) یدکی_عرض
						* (عائم) ستون[i].عرض
						/ relative_جمع);
				min_عرض += ستون[i].min;
				max_عرض += ستون[i].max;
			}
		}
	}
	min_عرض += (columns + 1) * border_spacing_h;
	max_عرض += (columns + 1) * border_spacing_h;

	إذا (auto_عرض <= min_عرض) {
		/* not enough space: minimise column widths */
		لكل (i = 0; i < columns; i++) {
			ستون[i].عرض = ستون[i].min;
		}
		جدول_عرض = min_عرض;
	} وإلا إذا (max_عرض <= auto_عرض) {
		/* more space than maximum width */
		إذا (جدول_عرض == AUTO) {
			/* for auto-width tables, make columns max width */
			لكل (i = 0; i < columns; i++) {
				ستون[i].عرض = ستون[i].max;
			}
			جدول_عرض = max_عرض;
		} وإلا {
			/* for fixed-width tables, distribute the extra space
			 * too */
			بیعلامت صحيح flexible_columns = 0;
			لكل (i = 0; i != columns; i++)
				إذا (ستون[i].type != ستونی_عرض_FIXED)
					flexible_columns++;
			إذا (flexible_columns == 0) {
				صحيح extra = (جدول_عرض - max_عرض) / columns;
				remainder = (جدول_عرض - max_عرض) -
						(extra * columns);
				لكل (i = 0; i != columns; i++) {
					ستون[i].عرض = ستون[i].max + extra;
					شمار -= remainder;
					إذا (شمار < 0) {
						ستون[i].عرض++;
						شمار += columns;
					}
				}

			} وإلا {
				صحيح extra = (جدول_عرض - max_عرض) /
						flexible_columns;
				remainder = (جدول_عرض - max_عرض) -
						(extra * flexible_columns);
				لكل (i = 0; i != columns; i++)
					إذا (ستون[i].type != ستونی_عرض_FIXED) {
						ستون[i].عرض = ستون[i].max +
								extra;
						شمار -= remainder;
						إذا (شمار < 0) {
							ستون[i].عرض++;
							شمار += flexible_columns;
						}
					}
			}
		}
	} وإلا {
		/* space between min and max: fill it exactly */
		عائم scale = (عائم) (auto_عرض - min_عرض) /
				(عائم) (max_عرض - min_عرض);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		لكل (i = 0; i < columns; i++) {
			ستون[i].عرض = ستون[i].min + (صحيح) (0.5 +
					(ستون[i].max - ستون[i].min) * scale);
		}
		جدول_عرض = auto_عرض;
	}

	xs[0] = x = border_spacing_h;
	لكل (i = 0; i != columns; i++) {
		إذا (!ستون[i].positioned)
			x += ستون[i].عرض + border_spacing_h;
		xs[i + 1] = x;
		ردیف_بازه[i] = 0;
		excess_y[i] = 0;
		ردیف_بازه_cell[i] = 0;
	}

	/* position cells */
	جدول_ارتفاع = border_spacing_v;
	لكل (ردیف_گروه = جدول->children; ردیف_گروه;
			ردیف_گروه = ردیف_گروه->بعدی) {
		صحيح ردیف_گروه_ارتفاع = 0;
		لكل (ردیف = ردیف_گروه->children; ردیف; ردیف = ردیف->بعدی) {
			صحيح ردیف_ارتفاع = 0;

			htype = css_computed_ارتفاع(ردیف->style, &مقدار, &unit);
			إذا (htype == CSS_ارتفاع_مجموعه && unit != CSS_UNIT_PCT) {
				ردیف_ارتفاع = FIXTOINT(css_unit_len2device_px(
						ردیف->style,
						&content->unit_len_ctx,
						مقدار, unit));
			}
			لكل (c = ردیف->children; c; c = c->بعدی) {
				أكد(c->style);
				c->عرض = xs[c->شروع_ستونی + c->columns] -
						xs[c->شروع_ستونی] -
						border_spacing_h -
						c->border[چپ_2].عرض -
						c->padding[چپ_2] -
						c->padding[راست_2] -
						c->border[راست_2].عرض;
				c->float_children = 0;
				c->cached_قراربده_زیر_level = 0;

				c->ارتفاع = AUTO;
				إذا (!layout_بلوک_context(c, -1, content)) {
					حرر(ستون);
					حرر(excess_y);
					حرر(ردیف_بازه);
					حرر(ردیف_بازه_cell);
					حرر(xs);
					أرجع false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->ارتفاع;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_ارتفاع(c->style,
						&مقدار, &unit);

				إذا (htype == CSS_ارتفاع_مجموعه &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					صحيح h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							مقدار, unit));
					إذا (c->ارتفاع < h)
						c->ارتفاع = h;
				}
				/* specified row height is treated as a minimum
				 */
				إذا (c->ارتفاع < ردیف_ارتفاع)
					c->ارتفاع = ردیف_ارتفاع;
				c->x = xs[c->شروع_ستونی] +
						c->border[چپ_2].عرض;
				c->y = c->border[بالاترین_2].عرض;
				لكل (i = 0; i != c->columns; i++) {
					ردیف_بازه[c->شروع_ستونی + i] = c->ردیفها;
					excess_y[c->شروع_ستونی + i] =
							c->border[بالاترین_2].عرض +
							c->padding[بالاترین_2] +
							c->ارتفاع +
							c->padding[BOTTOM] +
							c->border[BOTTOM].عرض;
					ردیف_بازه_cell[c->شروع_ستونی + i] = 0;
				}
				ردیف_بازه_cell[c->شروع_ستونی] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[بالاترین_2].عرض -
						c->padding[بالاترین_2] -
						c->ارتفاع -
						c->border[BOTTOM].عرض;
			}
			لكل (i = 0; i != columns; i++)
				إذا (ردیف_بازه[i] != 0)
					ردیف_بازه[i]--;
				وإلا
					ردیف_بازه_cell[i] = 0;
			إذا (ردیف->بعدی || ردیف_گروه->بعدی) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				لكل (i = 0; i != columns; i++)
					إذا (ردیف_بازه[i] == 0 && ردیف_ارتفاع <
							excess_y[i])
						ردیف_ارتفاع = excess_y[i];
			} وإلا {
				/* except in the last row */
				لكل (i = 0; i != columns; i++)
					إذا (ردیف_ارتفاع < excess_y[i])
						ردیف_ارتفاع = excess_y[i];
			}
			لكل (i = 0; i != columns; i++) {
				إذا (ردیف_ارتفاع < excess_y[i])
					excess_y[i] -= ردیف_ارتفاع;
				وإلا
					excess_y[i] = 0;
				إذا (ردیف_بازه_cell[i] != 0)
					ردیف_بازه_cell[i]->padding[BOTTOM] +=
							ردیف_ارتفاع +
							border_spacing_v;
			}

			ردیف->x = 0;
			ردیف->y = ردیف_گروه_ارتفاع;
			ردیف->عرض = جدول_عرض;
			ردیف->ارتفاع = ردیف_ارتفاع;
			ردیف_گروه_ارتفاع += ردیف_ارتفاع + border_spacing_v;
		}
		ردیف_گروه->x = 0;
		ردیف_گروه->y = جدول_ارتفاع;
		ردیف_گروه->عرض = جدول_عرض;
		ردیف_گروه->ارتفاع = ردیف_گروه_ارتفاع;
		جدول_ارتفاع += ردیف_گروه_ارتفاع;
	}
	/* Table height is either the height of the contents, or specified
	 * height if greater */
	جدول_ارتفاع = max(جدول_ارتفاع, min_ارتفاع);
	/** \todo distribute spare height over the row groups / rows / cells */

	/* perform vertical alignment */
	لكل (ردیف_گروه = جدول->children; ردیف_گروه;
			ردیف_گروه = ردیف_گروه->بعدی) {
		لكل (ردیف = ردیف_گروه->children; ردیف; ردیف = ردیف->بعدی) {
			لكل (c = ردیف->children; c; c = c->بعدی) {
				تعداد css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				یدکی_ارتفاع = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->ارتفاع - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &مقدار, &unit);

				بدل (vertical_align) {
				حالة CSS_VERTICAL_ALIGN_SUB:
				حالة CSS_VERTICAL_ALIGN_SUPER:
				حالة CSS_VERTICAL_ALIGN_متن_بالاترین:
				حالة CSS_VERTICAL_ALIGN_متن_BOTTOM:
				حالة CSS_VERTICAL_ALIGN_مجموعه:
				حالة CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				حالة CSS_VERTICAL_ALIGN_بالاترین:
					اكسر;
				حالة CSS_VERTICAL_ALIGN_میانه:
					c->padding[بالاترین_2] += یدکی_ارتفاع / 2;
					c->padding[BOTTOM] -= یدکی_ارتفاع / 2;
					layout_move_children(c, 0,
							یدکی_ارتفاع / 2);
					اكسر;
				حالة CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[بالاترین_2] += یدکی_ارتفاع;
					c->padding[BOTTOM] -= یدکی_ارتفاع;
					layout_move_children(c, 0,
							یدکی_ارتفاع);
					اكسر;
				حالة CSS_VERTICAL_ALIGN_INHERIT:
					أكد(0);
					اكسر;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	إذا (جدول->margin[بالاترین_2] == AUTO)
		جدول->margin[بالاترین_2] = 0;
	إذا (جدول->margin[BOTTOM] == AUTO)
		جدول->margin[BOTTOM] = 0;

	حرر(ستون);
	حرر(excess_y);
	حرر(ردیف_بازه);
	حرر(ردیف_بازه_cell);
	حرر(xs);

	جدول->عرض = جدول_عرض;
	جدول->ارتفاع = جدول_ارتفاع;

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
ساكن bool layout_apply_minmax_ارتفاع(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *box,
		بنية box *container)
{
	صحيح h;
	بنية box *containing_بلوک = عدم;
	bool updated = false;

	/* Find containing block for percentage heights */
	إذا (box->style != عدم && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		أكد(container);
		containing_بلوک = container;
	} وإلا إذا (box->float_container && box->style != عدم &&
			(css_computed_float(box->style) == CSS_FLOAT_چپ ||
			 css_computed_float(box->style) == CSS_FLOAT_راست)) {
		/* Box is a float */
		أكد(box->والد && box->والد->والد &&
				box->والد->والد->والد);
		containing_بلوک = box->والد->والد->والد;
	} وإلا إذا (box->والد && box->والد->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_بلوک = box->والد;
	} وإلا إذا (box->والد && box->والد->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		أكد(box->والد->والد);
		containing_بلوک = box->والد->والد;
	}

	إذا (box->style) {
		تعداد css_ارتفاع_e htype = CSS_ارتفاع_AUTO;
		css_fixed مقدار = 0;
		css_unit unit = CSS_UNIT_PX;

		إذا (containing_بلوک) {
			htype = css_computed_ارتفاع(containing_بلوک->style,
					&مقدار, &unit);
		}

		/* max-height */
		إذا (css_computed_max_ارتفاع(box->style, &مقدار, &unit) ==
				CSS_MAX_ارتفاع_مجموعه) {
			إذا (unit == CSS_UNIT_PCT) {
				إذا (containing_بلوک &&
					containing_بلوک->ارتفاع != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_ارتفاع_مجموعه)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_از_INT_TOINT(مقدار,
						containing_بلوک->ارتفاع);
					إذا (h < box->ارتفاع) {
						box->ارتفاع = h;
						updated = true;
					}
				}
			} وإلا {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						مقدار, unit));
				إذا (h < box->ارتفاع) {
					box->ارتفاع = h;
					updated = true;
				}
			}
		}

		/* min-height */
		إذا (ns_computed_min_ارتفاع(box->style, &مقدار, &unit) ==
				CSS_MIN_ارتفاع_مجموعه) {
			إذا (unit == CSS_UNIT_PCT) {
				إذا (containing_بلوک &&
					containing_بلوک->ارتفاع != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_ارتفاع_مجموعه)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_از_INT_TOINT(مقدار,
						containing_بلوک->ارتفاع);
					إذا (h > box->ارتفاع) {
						box->ارتفاع = h;
						updated = true;
					}
				}
			} وإلا {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						مقدار, unit));
				إذا (h > box->ارتفاع) {
					box->ارتفاع = h;
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
ساكن bool layout_بلوک_object(بنية box *بلوک)
{
	أكد(بلوک);
	أكد(بلوک->type == BOX_بلوک ||
			بلوک->type == BOX_FLEX ||
			بلوک->type == BOX_INLINE_بلوک ||
			بلوک->type == BOX_INLINE_FLEX ||
			بلوک->type == BOX_جدول ||
			بلوک->type == BOX_جدول_CELL);
	أكد(بلوک->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", بلوک,
	      hlcache_handle_get_url(بلوک->object), بلوک->عرض);

	إذا (content_can_reformat(بلوک->object)) {
		content_reformat(بلوک->object, false, بلوک->عرض, 1);
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
ساكن فراغ جمع_کن_float_مقصد_container(بنية box *cont, بنية box *b)
{
	بنية box *box = cont->float_children;
	صحيح b_bottom = b->y + b->ارتفاع;

	أكد(b->type == BOX_FLOAT_چپ || b->type == BOX_FLOAT_راست);

	إذا (box == عدم) {
		/* No other float children */
		b->بعدی_float = عدم;
		cont->float_children = b;
		أرجع;
	} وإلا إذا (b_bottom >= box->y + box->ارتفاع) {
		/* Goes at start of list */
		b->بعدی_float = cont->float_children;
		cont->float_children = b;
	} وإلا {
		بنية box *prev = عدم;
		طالما (box != عدم && b_bottom < box->y + box->ارتفاع) {
			prev = box;
			box = box->بعدی_float;
		}
		إذا (prev != عدم) {
			b->بعدی_float = prev->بعدی_float;
			prev->بعدی_float = b;
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
layout_متن_box_تقسیم(html_content *content,
		      plot_font_style_t *fstyle,
		      بنية box *تقسیم_box,
		      size_t new_درازی,
		      صحيح new_عرض)
{
	صحيح space_عرض = تقسیم_box->space;
	بنية box *c2;
	ثابت بنية gui_layout_جدول *font_func = content->font_func;
	bool space = (تقسیم_box->متن[new_درازی] == ' ');
	صحيح استفاده_شده_درازی = new_درازی + (space ? 1 : 0);

	إذا ((space && space_عرض == 0) || space_عرض == UNKNOWN_عرض) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->عرض(fstyle, " ", 1, &space_عرض);
	}

	إذا (تقسیم_box->space == UNKNOWN_عرض)
		تقسیم_box->space = space_عرض;
	إذا (!space)
		space_عرض = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, تقسیم_box, حجم *c2);
	إذا (!c2)
		أرجع false;
	c2->flags |= CLONE;

	/* Set remaining text in c2 */
	c2->متن += استفاده_شده_درازی;

	/* Set c2 according to the remaining text */
	c2->عرض -= new_عرض + space_عرض;
	c2->flags &= ~MEASURED; /* width has been estimated */
	c2->درازی = تقسیم_box->درازی - استفاده_شده_درازی;

	/* Update split_box for its reduced text */
	تقسیم_box->عرض = new_عرض;
	تقسیم_box->flags |= MEASURED;
	تقسیم_box->درازی = new_درازی;
	تقسیم_box->space = space_عرض;

	/* Insert c2 into box list */
	c2->بعدی = تقسیم_box->بعدی;
	تقسیم_box->بعدی = c2;
	c2->prev = تقسیم_box;
	إذا (c2->بعدی)
		c2->بعدی->prev = c2;
	وإلا
		c2->والد->آخرین = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      تقسیم_box,
	      تقسیم_box->درازی,
	      (صحيح)تقسیم_box->درازی,
	      تقسیم_box->متن);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->درازی,
	      (صحيح)c2->درازی,
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
layout_float_پیدا_کن_dimensions(
		ثابت css_unit_ctx *unit_len_ctx,
		صحيح موجود_عرض,
		ثابت css_computed_style *style,
		بنية box *box)
{
	صحيح عرض, ارتفاع, max_عرض, min_عرض, max_ارتفاع, min_ارتفاع;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	تعداد css_سرریز_e سرریز_x = css_computed_سرریز_x(style);
	تعداد css_سرریز_e سرریز_y = css_computed_سرریز_y(style);
	صحيح scrollbar_عرض_x =
			(سرریز_x == CSS_سرریز_SCROLL ||
			 سرریز_x == CSS_سرریز_AUTO) ?
			SCROLLBAR_عرض : 0;
	صحيح scrollbar_عرض_y =
			(سرریز_y == CSS_سرریز_SCROLL ||
			 سرریز_y == CSS_سرریز_AUTO) ?
			SCROLLBAR_عرض : 0;

	layout_پیدا_کن_dimensions(unit_len_ctx, موجود_عرض, -1, box, style,
			&عرض, &ارتفاع, &max_عرض, &min_عرض,
			&max_ارتفاع, &min_ارتفاع, margin, padding, border);

	إذا (margin[چپ_2] == AUTO)
		margin[چپ_2] = 0;
	إذا (margin[راست_2] == AUTO)
		margin[راست_2] = 0;

	إذا (box->gadget == عدم) {
		padding[راست_2] += scrollbar_عرض_y;
		padding[BOTTOM] += scrollbar_عرض_x;
	}

	إذا (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &عرض, &ارتفاع,
				min_عرض, max_عرض, min_ارتفاع, max_ارتفاع);
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
			إذا (عرض == AUTO) {
				اندازه = INTTOFIX(10);
				عرض = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						اندازه, unit));
			}
			إذا (box->gadget->type == GADGET_FILE &&
					ارتفاع == AUTO) {
				اندازه = FLTTOFIX(1.5);
				ارتفاع = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						اندازه, unit));
			}
		}
		إذا (box->gadget->type == GADGET_TEXTAREA) {
			إذا (عرض == AUTO) {
				اندازه = INTTOFIX(10);
				عرض = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						اندازه, unit));
			}
			إذا (ارتفاع == AUTO) {
				اندازه = INTTOFIX(4);
				ارتفاع = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						اندازه, unit));
			}
		}
	} وإلا إذا (عرض == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		عرض = min(max(box->min_عرض, موجود_عرض),
				box->max_عرض);

		/* width includes margin, borders and padding */
		إذا (عرض == موجود_عرض) {
			عرض -= box->margin[چپ_2] + box->border[چپ_2].عرض +
					box->padding[چپ_2] +
					box->padding[راست_2] +
					box->border[راست_2].عرض +
					box->margin[راست_2];
		} وإلا {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			صحيح fixed = 0;
			عائم frac = 0;
			calculate_mbp_عرض(unit_len_ctx, box->style, چپ_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_عرض(unit_len_ctx, box->style, راست_2,
					true, true, true, &fixed, &frac);
			إذا (fixed < 0)
				fixed = 0;

			عرض -= fixed;
		}

		إذا (max_عرض >= 0 && عرض > max_عرض) عرض = max_عرض;
		إذا (min_عرض >  0 && عرض < min_عرض) عرض = min_عرض;

	} وإلا {
		إذا (max_عرض >= 0 && عرض > max_عرض) عرض = max_عرض;
		إذا (min_عرض >  0 && عرض < min_عرض) عرض = min_عرض;
		عرض -= scrollbar_عرض_y;
	}

	box->عرض = عرض;
	box->ارتفاع = ارتفاع;

	إذا (margin[بالاترین_2] == AUTO)
		margin[بالاترین_2] = 0;
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
ساكن bool layout_float(بنية box *b, صحيح عرض, html_content *content)
{
	أكد(b->type == BOX_جدول ||
	       b->type == BOX_بلوک ||
	       b->type == BOX_INLINE_بلوک ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_پیدا_کن_dimensions(&content->unit_len_ctx, عرض, b->style, b);
	إذا (b->type == BOX_جدول || b->type == BOX_INLINE_FLEX) {
		إذا (b->type == BOX_جدول) {
			إذا (!layout_جدول(b, عرض, content))
				أرجع false;
		} وإلا {
			إذا (!layout_flex(b, عرض, content))
				أرجع false;
		}
		إذا (b->margin[چپ_2] == AUTO)
			b->margin[چپ_2] = 0;
		إذا (b->margin[راست_2] == AUTO)
			b->margin[راست_2] = 0;
		إذا (b->margin[بالاترین_2] == AUTO)
			b->margin[بالاترین_2] = 0;
		إذا (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} وإلا {
		أرجع layout_بلوک_context(b, -1, content);
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
قراربده_float_زیر(بنية box *c, صحيح عرض, صحيح cx, صحيح y, بنية box *cont)
{
	صحيح x0, x1, yy;
	بنية box *چپ;
	بنية box *راست;

	yy = y > cont->cached_قراربده_زیر_level ?
			y : cont->cached_قراربده_زیر_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      عرض, cx, y, cont);

	افعل {
		y = yy;
		x0 = cx;
		x1 = cx + عرض;
		پیدا_کن_sides(cont->float_children, y, y + c->ارتفاع, &x0, &x1,
				&چپ, &راست);
		إذا (چپ != 0 && راست != 0) {
			yy = (چپ->y + چپ->ارتفاع <
					راست->y + راست->ارتفاع ?
					چپ->y + چپ->ارتفاع :
					راست->y + راست->ارتفاع);
		} وإلا إذا (چپ == 0 && راست != 0) {
			yy = راست->y + راست->ارتفاع;
		} وإلا إذا (چپ != 0 && راست == 0) {
			yy = چپ->y + چپ->ارتفاع;
		}
	} طالما ((چپ != 0 || راست != 0) && (c->عرض > x1 - x0));

	إذا (c->type == BOX_FLOAT_چپ) {
		c->x = x0;
	} وإلا {
		c->x = x1 - c->عرض;
	}
	c->y = y;
	cont->cached_قراربده_زیر_level = y;
}


/**
 * Calculate line height from a style.
 */
ساكن صحيح سطر_ارتفاع(
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *style)
{
	تعداد css_سطر_ارتفاع_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed سطر_ارتفاع;

	أكد(style);

	lhtype = css_computed_سطر_ارتفاع(style, &lhvalue, &lhunit);
	إذا (lhtype == CSS_سطر_ارتفاع_نرمال) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_سطر_ارتفاع_عدد;
	}

	إذا (lhtype == CSS_سطر_ارتفاع_عدد ||
			lhunit == CSS_UNIT_PCT) {
		سطر_ارتفاع = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		إذا (lhtype != CSS_سطر_ارتفاع_عدد)
			سطر_ارتفاع = FDIV(سطر_ارتفاع, F_100);
	} وإلا {
		أكد(lhunit != CSS_UNIT_PCT);

		سطر_ارتفاع = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	أرجع FIXTOINT(سطر_ارتفاع);
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
layout_سطر(بنية box *اولی,
	    صحيح *عرض,
	    صحيح *y,
	    صحيح cx,
	    صحيح cy,
	    بنية box *cont,
	    bool indent,
	    bool has_متن_children,
	    html_content *content,
	    بنية box **بعدی_box)
{
	صحيح ارتفاع, استفاده_شده_ارتفاع;
	صحيح x0 = 0;
	صحيح x1 = *عرض;
	صحيح x, h, x_قبلی;
	صحيح fy = cy;
	بنية box *چپ;
	بنية box *راست;
	بنية box *b;
	بنية box *تقسیم_box = 0;
	بنية box *d;
	بنية box *br_box = 0;
	bool move_y = false;
	bool قراربده_زیر = false;
	صحيح space_before = 0, space_after = 0;
	بیعلامت صحيح inline_شمار = 0;
	بیعلامت صحيح i;
	ثابت بنية gui_layout_جدول *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      اولی,
	      (صحيح)اولی->درازی,
	      اولی->متن,
	      *عرض,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	پیدا_کن_sides(cont->float_children, cy, cy, &x0, &x1, &چپ, &راست);
	x0 -= cx;
	x1 -= cx;

	إذا (indent)
		x0 += layout_متن_indent(&content->unit_len_ctx,
				اولی->والد->والد->style, *عرض);

	إذا (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	إذا (has_متن_children || اولی->والد->والد->gadget)
		استفاده_شده_ارتفاع = ارتفاع = سطر_ارتفاع(&content->unit_len_ctx,
				اولی->والد->والد->style);
	وإلا
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		استفاده_شده_ارتفاع = ارتفاع = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	لكل (x = 0, b = اولی; x <= x1 - x0 && b != 0; b = b->بعدی) {
		صحيح min_عرض, max_عرض, min_ارتفاع, max_ارتفاع;

		أكد(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		إذا (b->type == BOX_BR)
			اكسر;

		إذا (lh__box_is_float_box(b))
			استمر;
		إذا (b->type == BOX_INLINE_بلوک &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			استمر;

		أكد(b->style != عدم);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		إذا (b->type == BOX_INLINE_بلوک ||
		    b->type == BOX_INLINE_FLEX) {
			إذا (b->max_عرض != UNKNOWN_عرض)
				إذا (!layout_float(b, *عرض, content))
					أرجع false;
			h = b->border[بالاترین_2].عرض + b->padding[بالاترین_2] + b->ارتفاع +
					b->padding[BOTTOM] +
					b->border[BOTTOM].عرض;
			إذا (ارتفاع < h)
				ارتفاع = h;
			x += b->margin[چپ_2] + b->border[چپ_2].عرض +
					b->padding[چپ_2] + b->عرض +
					b->padding[راست_2] +
					b->border[راست_2].عرض +
					b->margin[راست_2];
			space_after = 0;
			استمر;
		}

		إذا (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_پیدا_کن_dimensions(&content->unit_len_ctx,
					*عرض, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			لكل (i = 0; i != 4; i++)
				إذا (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[چپ_2] + b->border[چپ_2].عرض +
					b->padding[چپ_2];
			إذا (b->inline_انتها) {
				b->inline_انتها->margin[راست_2] = b->margin[راست_2];
				b->inline_انتها->padding[راست_2] =
						b->padding[راست_2];
				b->inline_انتها->border[راست_2] =
						b->border[راست_2];
			} وإلا {
				x += b->padding[راست_2] +
						b->border[راست_2].عرض +
						b->margin[راست_2];
			}
		} وإلا إذا (b->type == BOX_INLINE_انتها) {
			b->عرض = 0;
			إذا (b->space == UNKNOWN_عرض) {
				font_func->عرض(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[راست_2] + b->border[راست_2].عرض +
					b->margin[راست_2];
			استمر;
		}

		إذا (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->ارتفاع = سطر_ارتفاع(&content->unit_len_ctx,
					b->style ? b->style :
					b->والد->والد->style);
			إذا (ارتفاع < b->ارتفاع)
				ارتفاع = b->ارتفاع;

			إذا (!b->متن) {
				b->عرض = 0;
				space_after = 0;
				استمر;
			}

			إذا (b->عرض == UNKNOWN_عرض) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				إذا (b->والد->والد->gadget &&
						b->والد->والد->gadget->type
						== GADGET_SELECT) {
					صحيح opt_maxwidth = 0;
					بنية form_option *o;

					لكل (o = b->والد->والد->gadget->
							داده.select.اقلام; o;
							o = o->بعدی) {
						صحيح opt_عرض;
						font_func->عرض(&fstyle,
								o->متن,
								طول_رشته(o->متن),
								&opt_عرض);

						إذا (opt_maxwidth < opt_عرض)
							opt_maxwidth =opt_عرض;
					}
					b->عرض = opt_maxwidth;
					إذا (nsoption_bool(core_select_menu))
						b->عرض += SCROLLBAR_عرض;
				} وإلا {
					font_func->عرض(&fstyle, b->متن,
							b->درازی, &b->عرض);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			إذا (b->متن && (x + b->عرض < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->بعدی) {
				font_func->عرض(&fstyle, b->متن,
						 b->درازی, &b->عرض);
				b->flags |= MEASURED;
			}

			x += b->عرض;
			إذا (b->space == UNKNOWN_عرض) {
				font_func->عرض(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			استمر;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		أكد(b->style);

		layout_پیدا_کن_dimensions(&content->unit_len_ctx,
				*عرض, -1, b, b->style,
				&b->عرض, &b->ارتفاع,
				&max_عرض, &min_عرض,
				&max_ارتفاع, &min_ارتفاع,
				عدم, عدم, عدم);

		إذا (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->عرض, &b->ارتفاع,
					min_عرض, max_عرض,
					min_ارتفاع, max_ارتفاع);
		} وإلا إذا (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			إذا (b->عرض == AUTO)
				b->عرض = 400;
			إذا (b->ارتفاع == AUTO)
				b->ارتفاع = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} وإلا {
			/* form control with no object */
			إذا (b->عرض == AUTO)
				b->عرض = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			إذا (b->ارتفاع == AUTO)
				b->ارتفاع = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		إذا (b->object && content_can_reformat(b->object) &&
				b->عرض !=
				content_get_موجود_عرض(b->object)) {
			css_fixed مقدار = 0;
			css_unit unit = CSS_UNIT_PX;
			تعداد css_ارتفاع_e htype = css_computed_ارتفاع(b->style,
					&مقدار, &unit);

			content_reformat(b->object, false, b->عرض, b->ارتفاع);

			إذا (htype == CSS_ارتفاع_AUTO)
				b->ارتفاع = content_get_ارتفاع(b->object);
		}

		إذا (ارتفاع < b->ارتفاع)
			ارتفاع = b->ارتفاع;

		x += b->عرض;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *عرض;
	پیدا_کن_sides(cont->float_children, cy, cy + ارتفاع, &x0, &x1,
			&چپ, &راست);
	x0 -= cx;
	x1 -= cx;

	إذا (indent)
		x0 += layout_متن_indent(&content->unit_len_ctx,
				اولی->والد->والد->style, *عرض);

	إذا (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	لكل (x = x_قبلی = 0, b = اولی; x <= x1 - x0 && b; b = b->بعدی) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		إذا (b->type == BOX_INLINE_بلوک &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} وإلا إذا (lh__box_is_inline_flow(b)) {
			أكد(b->عرض != UNKNOWN_عرض);

			x_قبلی = x;
			x += space_after;
			b->x = x;

			إذا ((b->type == BOX_INLINE && !b->inline_انتها) ||
					b->type == BOX_INLINE_بلوک ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[چپ_2] + b->border[چپ_2].عرض;
				x = b->x + b->padding[چپ_2] + b->عرض +
						b->padding[راست_2] +
						b->border[راست_2].عرض +
						b->margin[راست_2];
			} وإلا إذا (b->type == BOX_INLINE) {
				b->x += b->margin[چپ_2] + b->border[چپ_2].عرض;
				x = b->x + b->padding[چپ_2] + b->عرض;
			} وإلا إذا (b->type == BOX_INLINE_انتها) {
				b->ارتفاع = b->inline_انتها->ارتفاع;
				x += b->padding[راست_2] +
						b->border[راست_2].عرض +
						b->margin[راست_2];
			} وإلا {
				x += b->عرض;
			}

			space_before = space_after;
			إذا (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			وإلا إذا (b->متن || b->type == BOX_INLINE_انتها) {
				إذا (b->space == UNKNOWN_عرض) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->عرض(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} وإلا {
				space_after = 0;
			}
			تقسیم_box = b;
			move_y = true;
			inline_شمار++;
		} وإلا إذا (b->type == BOX_BR) {
			b->x = x;
			b->عرض = 0;
			br_box = b;
			b = b->بعدی;
			تقسیم_box = 0;
			move_y = true;
			اكسر;

		} وإلا {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_قراربده_زیر_level = 0;
			b->float_container = d->float_container = cont;

			إذا (!layout_float(d, *عرض, content))
				أرجع false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[بالاترین_2],
			      d->border[بالاترین_2].عرض);

			d->x = d->margin[چپ_2] + d->border[چپ_2].عرض;
			d->y = d->margin[بالاترین_2] + d->border[بالاترین_2].عرض;
			b->عرض = d->margin[چپ_2] + d->border[چپ_2].عرض +
					d->padding[چپ_2] + d->عرض +
					d->padding[راست_2] +
					d->border[راست_2].عرض +
					d->margin[راست_2];
			b->ارتفاع = d->margin[بالاترین_2] + d->border[بالاترین_2].عرض +
					d->padding[بالاترین_2] + d->ارتفاع +
					d->padding[BOTTOM] +
					d->border[BOTTOM].عرض +
					d->margin[BOTTOM];

			إذا (b->عرض > (x1 - x0) - x)
				قراربده_زیر = true;
			إذا (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_چپ && چپ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_راست &&
						راست == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						چپ == 0 && راست == 0)) &&
					(!قراربده_زیر ||
					(چپ == 0 && راست == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_قراربده_زیر_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				إذا (b->type == BOX_FLOAT_چپ) {
					b->x = cx + x0;
					إذا (b->عرض > 0)
						x0 += b->عرض;
					چپ = b;
				} وإلا {
					b->x = cx + x1 - b->عرض;
					إذا (b->عرض > 0)
						x1 -= b->عرض;
					راست = b;
				}
				b->y = cy;
			} وإلا {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				صحيح fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_قراربده_زیر_level) ?
						fcy :
						cont->cached_قراربده_زیر_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + ارتفاع : fy;

				قراربده_float_زیر(b, *عرض, cx, fy, cont);
				fy = b->y;
				إذا (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_چپ && چپ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_راست &&
						راست != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(چپ != 0 || راست != 0)))) {
					/* to be cleared below existing
					 * floats */
					إذا (b->type == BOX_FLOAT_چپ)
						b->x = cx;
					وإلا
						b->x = cx + *عرض - b->عرض;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					إذا (fcy > cont->clear_level)
						cont->clear_level = fcy;
					إذا (b->y < fcy)
						b->y = fcy;
				}
				إذا (b->type == BOX_FLOAT_چپ)
					چپ = b;
				وإلا
					راست = b;
			}
			جمع_کن_float_مقصد_container(cont, b);

			تقسیم_box = 0;
		}
	}

	إذا (x1 - x0 < x && تقسیم_box) {
		/* the last box went over the end */
		size_t تقسیم = 0;
		صحيح w;
		bool no_wrap = css_computed_white_space(
				تقسیم_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				تقسیم_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_قبلی;

		إذا (!no_wrap &&
		    (تقسیم_box->type == BOX_INLINE ||
		     تقسیم_box->type == BOX_متن) &&
		    !تقسیم_box->object &&
		    !(تقسیم_box->flags & REPLACE_DIM) &&
		    !(تقسیم_box->flags & IFRAME) &&
		    !تقسیم_box->gadget && تقسیم_box->متن) {

			font_plot_style_from_css(&content->unit_len_ctx,
					تقسیم_box->style, &fstyle);
			/** \todo handle errors */
			font_func->تقسیم(&fstyle,
					 تقسیم_box->متن,
					 تقسیم_box->درازی,
					 x1 - x0 - x - space_before,
					 &تقسیم,
					 &w);
		}

		/* split == 0 implies that text can't be split */

		إذا (تقسیم == 0)
			w = تقسیم_box->عرض;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      تقسیم_box,
		      (صحيح)تقسیم_box->درازی,
		      تقسیم_box->متن,
		      تقسیم,
		      w,
		      چپ,
		      راست,
		      inline_شمار);

		إذا ((تقسیم == 0 || x1 - x0 <= x + space_before + w) &&
				!چپ && !راست && inline_شمار == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			إذا (تقسیم == 0 || تقسیم == تقسیم_box->درازی) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = تقسیم_box->بعدی;
			} وإلا {
				/* cut off first word for this line */
				إذا (!layout_متن_box_تقسیم(content, &fstyle,
						تقسیم_box, تقسیم, w))
					أرجع false;
				b = تقسیم_box->بعدی;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} وإلا إذا ((تقسیم == 0 || x1 - x0 <= x + space_before + w) &&
				inline_شمار == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			أكد(چپ || راست);
			استفاده_شده_ارتفاع = 0;
			إذا (چپ) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      چپ->y,
				      چپ->ارتفاع);

				استفاده_شده_ارتفاع = چپ->y + چپ->ارتفاع - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      استفاده_شده_ارتفاع);

			}
			إذا (راست && استفاده_شده_ارتفاع <
					راست->y + راست->ارتفاع - cy + 1)
				استفاده_شده_ارتفاع = راست->y + راست->ارتفاع - cy + 1;

			إذا (استفاده_شده_ارتفاع < 0)
				استفاده_شده_ارتفاع = 0;

			b = تقسیم_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} وإلا إذا (تقسیم == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = تقسیم_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} وإلا {
			/* fit as many words as possible */
			أكد(تقسیم != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (صحيح)تقسیم_box->درازی, تقسیم_box->متن,
			      x1 - x0, تقسیم, w);

			إذا (تقسیم != تقسیم_box->درازی) {
				إذا (!layout_متن_box_تقسیم(content, &fstyle,
						تقسیم_box, تقسیم, w))
					أرجع false;
				b = تقسیم_box->بعدی;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	بدل (css_computed_متن_align(اولی->والد->والد->style)) {
	حالة CSS_متن_ALIGN_راست:
	حالة CSS_متن_ALIGN_LIBCSS_راست:
		x0 = x1 - x;
		اكسر;
	حالة CSS_متن_ALIGN_CENTER:
	حالة CSS_متن_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		اكسر;
	حالة CSS_متن_ALIGN_چپ:
	حالة CSS_متن_ALIGN_LIBCSS_چپ:
	حالة CSS_متن_ALIGN_JUSTIFY:
		/* leave on left */
		اكسر;
	حالة CSS_متن_ALIGN_DEFAULT:
		/* None; consider text direction */
		بدل (css_computed_direction(اولی->والد->والد->style)) {
		حالة CSS_DIRECTION_LTR:
			/* leave on left */
			اكسر;
		حالة CSS_DIRECTION_RTL:
			x0 = x1 - x;
			اكسر;
		}
		اكسر;
	}

	لكل (d = اولی; d != b; d = d->بعدی) {
		d->flags &= ~NEW_سطر;

		إذا (d->type == BOX_INLINE_بلوک &&
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
				d->type == BOX_INLINE_انتها) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[بالاترین_2];

			إذا (d->type == BOX_متن && d->ارتفاع > استفاده_شده_ارتفاع) {
				/*متن*/
				استفاده_شده_ارتفاع = d->ارتفاع;
			}
		} وإلا إذا ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_بلوک) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[بالاترین_2].عرض + d->margin[بالاترین_2];
			h = d->margin[بالاترین_2] + d->border[بالاترین_2].عرض +
					d->padding[بالاترین_2] + d->ارتفاع +
					d->padding[BOTTOM] +
					d->border[BOTTOM].عرض +
					d->margin[BOTTOM];
			إذا (استفاده_شده_ارتفاع < h)
				استفاده_شده_ارتفاع = h;
		}
	}

	اولی->flags |= NEW_سطر;

	أكد(b != اولی || (move_y && 0 < استفاده_شده_ارتفاع && (چپ || راست)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	لكل (d = اولی; d != b; d = d->بعدی) {
		إذا ((d->type == BOX_INLINE && d->inline_انتها) ||
				d->type == BOX_BR ||
				d->type == BOX_متن ||
				d->type == BOX_INLINE_انتها) {
			css_fixed مقدار = 0;
			css_unit unit = CSS_UNIT_PX;
			بدل (css_computed_vertical_align(d->style, &مقدار,
					&unit)) {
			حالة CSS_VERTICAL_ALIGN_SUPER:
			حالة CSS_VERTICAL_ALIGN_بالاترین:
			حالة CSS_VERTICAL_ALIGN_متن_بالاترین:
				/* already at top */
				اكسر;
			حالة CSS_VERTICAL_ALIGN_SUB:
			حالة CSS_VERTICAL_ALIGN_BOTTOM:
			حالة CSS_VERTICAL_ALIGN_متن_BOTTOM:
				d->y += استفاده_شده_ارتفاع - d->ارتفاع;
				اكسر;
			افتراضي:
			حالة CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (استفاده_شده_ارتفاع - d->ارتفاع);
				اكسر;
			}
		}
	}

	/* handle clearance for br */
	إذا (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		صحيح clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		إذا (استفاده_شده_ارتفاع < clear_y - cy)
			استفاده_شده_ارتفاع = clear_y - cy;
	}

	إذا (move_y)
		*y += استفاده_شده_ارتفاع;
	*بعدی_box = b;
	*عرض = x; /* return actual width */
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
ساكن bool layout_inline_container(بنية box *inline_container, صحيح عرض,
		بنية box *cont, صحيح cx, صحيح cy, html_content *content)
{
	bool اولی_سطر = true;
	bool has_متن_children;
	بنية box *c, *بعدی;
	صحيح y = 0;
	صحيح curwidth,maxwidth = عرض;

	أكد(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      عرض,
	      cont,
	      cx,
	      cy);


	has_متن_children = false;
	لكل (c = inline_container->children; c; c = c->بعدی) {
		bool is_pre = false;

		إذا (c->style) {
			تعداد css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_سطر ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		إذا ((lh__box_is_object(c) == false &&
				c->متن && (c->درازی || is_pre)) ||
				c->type == BOX_BR)
			has_متن_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	لكل (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->عرض;
		إذا (!layout_سطر(c, &curwidth, &y, cx, cy + y, cont, اولی_سطر,
				has_متن_children, content, &بعدی))
			أرجع false;
		maxwidth = max(maxwidth,curwidth);
		c = بعدی;
		اولی_سطر = false;
	}

	inline_container->عرض = maxwidth;
	inline_container->ارتفاع = y;

	أرجع true;
}


/* Documented in layout_intertnal.h */
bool layout_بلوک_context(
		بنية box *بلوک,
		صحيح viewport_ارتفاع,
		html_content *content)
{
	بنية box *box;
	صحيح cx, cy;  /**< current coordinates */
	صحيح max_pos_margin = 0;
	صحيح max_neg_margin = 0;
	صحيح y = 0;
	صحيح lm, rm;
	بنية box *margin_collapse = عدم;
	bool درون_margin = false;
	css_fixed gadget_اندازه;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	أكد(بلوک->type == BOX_بلوک ||
			بلوک->type == BOX_INLINE_بلوک ||
			بلوک->type == BOX_جدول_CELL ||
			بلوک->type == BOX_FLEX ||
			بلوک->type == BOX_INLINE_FLEX);
	أكد(بلوک->عرض != UNKNOWN_عرض);
	أكد(بلوک->عرض != AUTO);

	بلوک->float_children = عدم;
	بلوک->cached_قراربده_زیر_level = 0;
	بلوک->clear_level = 0;

	/* special case if the block contains an object */
	إذا (بلوک->object) {
		صحيح موقت_عرض = بلوک->عرض;
		إذا (!layout_بلوک_object(بلوک))
			أرجع false;
		layout_get_object_dimensions(بلوک, &موقت_عرض,
				&بلوک->ارتفاع, INT_MIN, بیشترین_صحیح,
				INT_MIN, بیشترین_صحیح);
		أرجع true;
	} وإلا إذا (بلوک->flags & REPLACE_DIM) {
		أرجع true;
	}

	/* special case if the block contains an radio button or checkbox */
	إذا (بلوک->gadget && (بلوک->gadget->type == GADGET_RADIO ||
			بلوک->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_اندازه = INTTOFIX(1);
		إذا (بلوک->ارتفاع == AUTO)
			بلوک->ارتفاع = FIXTOINT(css_unit_len2device_px(
					بلوک->style,
					&content->unit_len_ctx,
					gadget_اندازه, gadget_unit));
	}

	box = بلوک->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = بلوک->padding[بالاترین_2];
	إذا (box)
		box->y = بلوک->padding[بالاترین_2];

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
		تعداد css_سرریز_e سرریز_x = CSS_سرریز_VISIBLE;
		تعداد css_سرریز_e سرریز_y = CSS_سرریز_VISIBLE;

		أكد(box->type == BOX_بلوک ||
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
			box->x = box->والد->padding[چپ_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			اذهب advance_مقصد_بعدی_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		إذا (margin_collapse == عدم) {
			margin_collapse = layout_بعدی_margin_بلوک(
					&content->unit_len_ctx, box, بلوک,
					viewport_ارتفاع,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			درون_margin = true;
		}

		/* Clearance. */
		y = 0;
		إذا (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(بلوک->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		إذا (box->style) {
			سرریز_x = css_computed_سرریز_x(box->style);
			سرریز_y = css_computed_سرریز_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		إذا (box->type == BOX_FLEX ||
		    box->type == BOX_بلوک ||
		    box->flags & IFRAME) {
			إذا (lh__box_is_object(box) == false &&
					box->style &&
					(سرریز_x != CSS_سرریز_VISIBLE ||
					 سرریز_y != CSS_سرریز_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				صحيح x0, x1, بالاترین;
				بنية box *چپ, *راست;
				بالاترین = cy + max_pos_margin - max_neg_margin;
				بالاترین = (بالاترین > y) ? بالاترین : y;
				x0 = cx;
				x1 = cx + box->والد->عرض -
						box->والد->padding[چپ_2] -
						box->والد->padding[راست_2];
				پیدا_کن_sides(بلوک->float_children, بالاترین, بالاترین,
						&x0, &x1, &چپ, &راست);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->والد->عرض -
						box->والد->padding[چپ_2] -
						box->والد->padding[راست_2] -
						x1;
			}
			layout_بلوک_پیدا_کن_dimensions(&content->unit_len_ctx,
					box->والد->عرض,
					viewport_ارتفاع, lm, rm, box);
			إذا (box->type == BOX_بلوک && !(box->flags & IFRAME)) {
				layout_بلوک_جمع_کن_scrollbar(box, راست_2);
				layout_بلوک_جمع_کن_scrollbar(box, BOTTOM);
			}
		} وإلا إذا (box->type == BOX_جدول) {
			إذا (box->style != عدم) {
				تعداد css_عرض_e wtype;
				css_fixed عرض = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_عرض(box->style, &عرض,
						&unit);

				إذا (wtype == CSS_عرض_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					صحيح x0, x1, بالاترین;
					بنية box *چپ, *راست;
					بالاترین = cy + max_pos_margin -
							max_neg_margin;
					بالاترین = (بالاترین > y) ? بالاترین : y;
					x0 = cx;
					x1 = cx + box->والد->عرض -
						box->والد->padding[چپ_2] -
						box->والد->padding[راست_2];
					پیدا_کن_sides(بلوک->float_children,
						بالاترین, بالاترین, &x0, &x1,
						&چپ, &راست);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->والد->عرض -
						box->والد->padding[چپ_2] -
						box->والد->padding[راست_2] -
						x1;
				}
			}
			إذا (!layout_جدول(box, box->والد->عرض - lm - rm,
					content))
				أرجع false;
			layout_solve_عرض(box, box->والد->عرض, box->عرض,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->والد->padding[چپ_2] + box->margin[چپ_2] +
				box->border[چپ_2].عرض;
		cx += box->x;

		/* Position box: vertical. */
		إذا (box->border[بالاترین_2].عرض) {
			box->y += box->border[بالاترین_2].عرض;
			cy += box->border[بالاترین_2].عرض;
		}

		/* Vertical margin */
		إذا (((box->type == BOX_بلوک && (box->flags & HAS_ارتفاع)) ||
		     box->type == BOX_FLEX ||
		     box->type == BOX_جدول ||
		     (box->type == BOX_INLINE_CONTAINER &&
		      !box_is_اولی_فرزند(box)) ||
		     margin_collapse == box) &&
		    درون_margin == true) {
			/* Margin goes above this box. */
			cy += max_pos_margin - max_neg_margin;
			box->y += max_pos_margin - max_neg_margin;

			/* Current margin has been applied. */
			درون_margin = false;
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
				(box->type == BOX_بلوک && box->style &&
				 (سرریز_x != CSS_سرریز_VISIBLE ||
				  سرریز_y != CSS_سرریز_VISIBLE))) {

			إذا (box->type == BOX_FLEX) {
				إذا (!layout_flex(box, box->عرض, content)) {
					أرجع false;
				}
			} وإلا {
				layout_بلوک_context(box,
						viewport_ارتفاع, content);
			}

			cy += box->padding[بالاترین_2];

			إذا (box->ارتفاع == AUTO) {
				box->ارتفاع = 0;
				layout_بلوک_جمع_کن_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->ارتفاع + box->padding[BOTTOM] +
					box->border[BOTTOM].عرض;
			y = box->y + box->padding[بالاترین_2] + box->ارتفاع +
					box->padding[BOTTOM] +
					box->border[BOTTOM].عرض;

			/* Skip children, because they are done in the new
			 * block context */
			اذهب advance_مقصد_بعدی_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->عرض);

		/* Layout (except tables). */
		إذا (box->object) {
			إذا (!layout_بلوک_object(box))
				أرجع false;

		} وإلا إذا (box->type == BOX_INLINE_CONTAINER) {
			box->عرض = box->والد->عرض;
			إذا (!layout_inline_container(box, box->عرض, بلوک,
					cx, cy, content))
				أرجع false;

		} وإلا إذا (box->type == BOX_جدول) {
			/* Move down to avoid floats if necessary. */
			صحيح x0, x1;
			بنية box *چپ, *راست;
			y = cy;
			طالما (1) {
				تعداد css_عرض_e wtype;
				css_fixed عرض = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_عرض(box->style,
						&عرض, &unit);

				x0 = cx;
				x1 = cx + box->والد->عرض;
				پیدا_کن_sides(بلوک->float_children, y,
						y + box->ارتفاع,
						&x0, &x1, &چپ, &راست);
				إذا (wtype == CSS_عرض_AUTO)
					اكسر;
				إذا (box->عرض <= x1 - x0)
					اكسر;
				إذا (!چپ && !راست)
					اكسر;
				وإلا إذا (!چپ)
					y = راست->y + راست->ارتفاع + 1;
				وإلا إذا (!راست)
					y = چپ->y + چپ->ارتفاع + 1;
				وإلا إذا (چپ->y + چپ->ارتفاع <
						راست->y + راست->ارتفاع)
					y = چپ->y + چپ->ارتفاع + 1;
				وإلا
					y = راست->y + راست->ارتفاع + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		إذا (box->type == BOX_بلوک && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			إذا (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = عدم;
			}

			y = box->padding[بالاترین_2];
			box = box->children;
			box->y = y;
			cy += y;
			استمر;
		} وإلا إذا (box->type == BOX_بلوک || box->object ||
				box->flags & IFRAME)
			cy += box->padding[بالاترین_2];

		إذا (box->type == BOX_بلوک && box->ارتفاع == AUTO) {
			box->ارتفاع = 0;
			layout_بلوک_جمع_کن_scrollbar(box, BOTTOM);
		}

		cy += box->ارتفاع + box->padding[BOTTOM] +
				box->border[BOTTOM].عرض;
		cx -= box->x;
		y = box->y + box->padding[بالاترین_2] + box->ارتفاع +
				box->padding[BOTTOM] +
				box->border[BOTTOM].عرض;

	advance_مقصد_بعدی_box:
		إذا (!box->بعدی) {
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
				إذا (box == بلوک)
					اكسر;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				إذا (box->type == BOX_بلوک &&
						box->flags & MAKE_ارتفاع) {
					margin_collapse = عدم;
					درون_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				إذا (box->ارتفاع == AUTO) {
					box->ارتفاع = y - box->padding[بالاترین_2];

					إذا (box->type == BOX_بلوک)
						layout_بلوک_جمع_کن_scrollbar(box,
								BOTTOM);
				} وإلا
					cy += box->ارتفاع -
							(y - box->padding[بالاترین_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				إذا (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_ارتفاع(
							&content->unit_len_ctx,
							box, عدم)) {
					/* Height altered */
					/* Set current cy */
					cy += box->ارتفاع -
							(y - box->padding[بالاترین_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].عرض;
				cx -= box->x;
				y = box->y + box->padding[بالاترین_2] + box->ارتفاع +
						box->padding[BOTTOM] +
						box->border[BOTTOM].عرض;

			} طالما (box->بعدی == عدم);
			إذا (box == بلوک)
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

		box = box->بعدی;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	لكل (box = بلوک->float_children; box; box = box->بعدی_float) {
		y = box->y + box->ارتفاع + box->padding[BOTTOM] +
				box->border[BOTTOM].عرض + box->margin[BOTTOM];
		إذا (cy < y)
			cy = y;
	}

	إذا (بلوک->ارتفاع == AUTO) {
		بلوک->ارتفاع = cy - بلوک->padding[بالاترین_2];
		إذا (بلوک->type == BOX_بلوک)
			layout_بلوک_جمع_کن_scrollbar(بلوک, BOTTOM);
	}

	إذا (بلوک->style && css_computed_position(بلوک->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_ارتفاع(&content->unit_len_ctx, بلوک, عدم);
	}

	إذا (بلوک->gadget &&
			(بلوک->gadget->type == GADGET_TEXTAREA ||
			بلوک->gadget->type == GADGET_PASSWORD ||
			بلوک->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		صحيح ta_عرض = بلوک->padding[چپ_2] + بلوک->عرض +
				بلوک->padding[راست_2];
		صحيح ta_ارتفاع = بلوک->padding[بالاترین_2] + بلوک->ارتفاع +
				بلوک->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				بلوک->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_مجموعه_layout(بلوک->gadget->داده.متن.ta,
				&fstyle, ta_عرض, ta_ارتفاع,
				بلوک->padding[بالاترین_2], بلوک->padding[راست_2],
				بلوک->padding[BOTTOM], بلوک->padding[چپ_2]);
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
layout_get_element_برچسب(
		ثابت dom_گره *گره,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_گره_type گره_type;
	dom_exception exc;

	exc = dom_گره_get_گره_type(گره, &گره_type);
	إذا (exc != DOM_NO_ERR ||
	    گره_type != DOM_ELEMENT_گره) {
		أرجع false;
	}

	exc = dom_html_element_get_برچسب_type(گره, &element_type);
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
		ثابت dom_گره *گره,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	إذا (!layout_get_element_برچسب(گره, &element_type)) {
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
layout_get_li_مقدار(dom_گره *li_گره, dom_long *مقدار_بیرون)
{
	dom_exception exc;
	dom_long مقدار;
	bool has_مقدار;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_گره,
			corestring_dom_مقدار,
			&has_مقدار);
	إذا (exc != DOM_NO_ERR || has_مقدار == false) {
		أرجع false;
	}

	exc = dom_html_li_element_get_مقدار(
			(dom_html_li_element *)li_گره,
			&مقدار);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	*مقدار_بیرون = مقدار;
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
layout_get_ol_شروع(dom_گره *ol_گره, dom_long *شروع_بیرون)
{
	dom_exception exc;
	dom_long شروع;
	bool has_شروع;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_گره,
			corestring_dom_شروع,
			&has_شروع);
	إذا (exc != DOM_NO_ERR || has_شروع == false) {
		أرجع false;
	}

	exc = dom_html_olist_element_get_شروع(
			(dom_html_olist_element *)ol_گره,
			&شروع);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	*شروع_بیرون = شروع;
	أرجع true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
ساكن bool
layout__get_ol_reversed(dom_گره *ol_گره)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_گره,
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
layout_get_list_قلم_شمار(
		dom_گره *list_owner, dom_long *شمار_بیرون)
{
	dom_html_element_type برچسب_type;
	dom_exception exc;
	dom_گره *فرزند;
	صحيح شمار;

	إذا (list_owner == عدم) {
		أرجع false;
	}

	إذا (!layout_get_element_برچسب(list_owner, &برچسب_type)) {
		أرجع false;
	}

	إذا (برچسب_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    برچسب_type != DOM_HTML_ELEMENT_TYPE_UL) {
		أرجع false;
	}

	exc = dom_گره_get_اولی_فرزند(list_owner, &فرزند);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	شمار = 0;
	طالما (فرزند != عدم) {
		dom_گره *موقت_گره;

		إذا (layout__check_element_type(فرزند,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			بنية box *فرزند_box;
			إذا (dom_گره_get_user_داده(فرزند,
					corestring_dom_ns_کلید_box_گره_داده,
					&فرزند_box) != DOM_NO_ERR) {
				dom_گره_unref(فرزند);
				أرجع false;
			}

			إذا (فرزند_box != عدم &&
			    فرزند_box->list_marker != عدم) {
				شمار++;
			}
		}

		exc = dom_گره_get_بعدی_sibling(فرزند, &موقت_گره);
		dom_گره_unref(فرزند);
		إذا (exc != DOM_NO_ERR) {
			أرجع false;
		}

		فرزند = موقت_گره;
	}

	*شمار_بیرون = شمار;
	أرجع true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
ساكن فراغ
layout_ordered_list_شمار(
		بنية box *box)
{
	dom_html_element_type برچسب_type;
	dom_exception exc;
	dom_گره *فرزند;
	صحيح گام = 1;
	dom_long بعدی;

	إذا (box->گره == عدم) {
		أرجع;
	}

	إذا (!layout_get_element_برچسب(box->گره, &برچسب_type)) {
		أرجع;
	}

	إذا (برچسب_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    برچسب_type != DOM_HTML_ELEMENT_TYPE_UL) {
		أرجع;
	}

	بعدی = 1;
	إذا (برچسب_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_شروع = layout_get_ol_شروع(box->گره, &بعدی);
		bool have_reversed = layout__get_ol_reversed(box->گره);

		إذا (have_reversed) {
			گام = -1;
		}

		إذا (!have_شروع && have_reversed) {
			layout_get_list_قلم_شمار(box->گره, &بعدی);
		}
	}

	exc = dom_گره_get_اولی_فرزند(box->گره, &فرزند);
	إذا (exc != DOM_NO_ERR) {
		أرجع;
	}

	طالما (فرزند != عدم) {
		dom_گره *موقت_گره;

		إذا (layout__check_element_type(فرزند,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			بنية box *فرزند_box;

			إذا (dom_گره_get_user_داده(فرزند,
					corestring_dom_ns_کلید_box_گره_داده,
					&فرزند_box) != DOM_NO_ERR) {
				dom_گره_unref(فرزند);
				أرجع;
			}

			إذا (فرزند_box != عدم &&
			    فرزند_box->list_marker != عدم) {
				dom_long مقدار;
				بنية box *marker = فرزند_box->list_marker;
				إذا (layout_get_li_مقدار(فرزند, &مقدار)) {
					marker->list_مقدار = مقدار;
					بعدی = marker->list_مقدار;
				} وإلا {
					marker->list_مقدار = بعدی;
				}
				بعدی += گام;
			}
		}

		exc = dom_گره_get_بعدی_sibling(فرزند, &موقت_گره);
		dom_گره_unref(فرزند);
		إذا (exc != DOM_NO_ERR) {
			أرجع;
		}

		فرزند = موقت_گره;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
ساكن فراغ
layout_مجموعه_numerical_marker_متن(
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

	css_res = css_computed_format_list_style(box->style, marker->list_مقدار,
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
					marker->list_مقدار, marker->متن,
					counter_len, &counter_len);
		}
		marker->درازی = counter_len;
	}
}

/**
 * Find out if box's style represents a numerical list style type.
 *
 * \param[in]  b  Box with style to test.
 * \return true if box has numerical list style type, false otherwise.
 */
ساكن bool
layout_list_قلم_is_numerical(
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
	بنية box *فرزند;

	layout_ordered_list_شمار(box);

	لكل (فرزند = box->children; فرزند; فرزند = فرزند->بعدی) {
		إذا (فرزند->list_marker) {
			بنية box *marker = فرزند->list_marker;

			إذا (layout_list_قلم_is_numerical(فرزند)) {
				إذا (marker->متن == عدم) {
					layout_مجموعه_numerical_marker_متن(
							content, فرزند);
				}
			}
			إذا (marker->object) {
				marker->عرض =
					content_get_عرض(marker->object);
				marker->x = -marker->عرض;
				marker->ارتفاع =
					content_get_ارتفاع(marker->object);
				marker->y = (سطر_ارتفاع(
						&content->unit_len_ctx,
						marker->style) -
						marker->ارتفاع) / 2;
			} وإلا إذا (marker->متن) {
				إذا (marker->عرض == UNKNOWN_عرض) {
					plot_font_style_t fstyle;
					font_plot_style_from_css(
							&content->unit_len_ctx,
							marker->style,
							&fstyle);
					content->font_func->عرض(&fstyle,
							marker->متن,
							marker->درازی,
							&marker->عرض);
					marker->flags |= MEASURED;
				}
				marker->x = -marker->عرض;
				marker->y = 0;
				marker->ارتفاع = سطر_ارتفاع(
						&content->unit_len_ctx,
						marker->style);
			} وإلا {
				marker->x = 0;
				marker->y = 0;
				marker->عرض = 0;
				marker->ارتفاع = 0;
			}
			/* Gap between marker and content */
			marker->x -= 4;
		}
		layout_lists(content, فرزند);
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
		       بنية box *containing_بلوک,
		       صحيح *بالاترین,
		       صحيح *راست,
		       صحيح *bottom,
		       صحيح *چپ)
{
	uint32_t type;
	css_fixed مقدار = 0;
	css_unit unit = CSS_UNIT_PX;

	أكد(containing_بلوک->عرض != UNKNOWN_عرض);
	أكد(containing_بلوک->عرض != AUTO);
	أكد(containing_بلوک->ارتفاع != AUTO);

	/*چپ*/
	type = css_computed_چپ(box->style, &مقدار, &unit);
	إذا (type == CSS_چپ_مجموعه) {
		إذا (unit == CSS_UNIT_PCT) {
			*چپ = FPCT_از_INT_TOINT(مقدار,
					containing_بلوک->عرض);
		} وإلا {
			*چپ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					مقدار, unit));
		}
	} وإلا {
		*چپ = AUTO;
	}

	/*راست*/
	type = css_computed_راست(box->style, &مقدار, &unit);
	إذا (type == CSS_راست_مجموعه) {
		إذا (unit == CSS_UNIT_PCT) {
			*راست = FPCT_از_INT_TOINT(مقدار,
					containing_بلوک->عرض);
		} وإلا {
			*راست = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					مقدار, unit));
		}
	} وإلا {
		*راست = AUTO;
	}

	/*بالاترین*/
	type = css_computed_بالاترین(box->style, &مقدار, &unit);
	إذا (type == CSS_بالاترین_مجموعه) {
		إذا (unit == CSS_UNIT_PCT) {
			*بالاترین = FPCT_از_INT_TOINT(مقدار,
					containing_بلوک->ارتفاع);
		} وإلا {
			*بالاترین = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					مقدار, unit));
		}
	} وإلا {
		*بالاترین = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &مقدار, &unit);
	إذا (type == CSS_BOTTOM_مجموعه) {
		إذا (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_از_INT_TOINT(مقدار,
					containing_بلوک->ارتفاع);
		} وإلا {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					مقدار, unit));
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
		بنية box *containing_بلوک,
		صحيح cx, صحيح cy,
		html_content *content)
{
	صحيح static_چپ, static_بالاترین;  /* static position */
	صحيح بالاترین, راست, bottom, چپ;
	صحيح عرض, ارتفاع, max_عرض, min_عرض;
	صحيح *margin = box->margin;
	صحيح *padding = box->padding;
	بنية box_border *border = box->border;
	صحيح موجود_عرض = containing_بلوک->عرض;
	صحيح space;

	أكد(box->type == BOX_بلوک || box->type == BOX_جدول ||
			box->type == BOX_INLINE_بلوک ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_چپ = cx + box->x;
	static_بالاترین = cy + box->y;

	إذا (containing_بلوک->type == BOX_بلوک ||
			containing_بلوک->type == BOX_INLINE_بلوک ||
			containing_بلوک->type == BOX_جدول_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_بلوک->عرض += containing_بلوک->padding[چپ_2] +
				containing_بلوک->padding[راست_2];
		containing_بلوک->ارتفاع += containing_بلوک->padding[بالاترین_2] +
				containing_بلوک->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_بلوک,
			&بالاترین, &راست, &bottom, &چپ);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_بلوک;
	layout_پیدا_کن_dimensions(&content->unit_len_ctx, موجود_عرض, -1,
			box, box->style, &عرض, &ارتفاع,
			&max_عرض, &min_عرض, 0, 0,
			margin, padding, border);
	box->float_container = عدم;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      چپ, margin[چپ_2], border[چپ_2].عرض, padding[چپ_2], عرض,
	      padding[راست_2], border[راست_2].عرض, margin[راست_2], راست,
	      containing_بلوک->عرض);


	إذا (چپ == AUTO && عرض == AUTO && راست == AUTO) {
		إذا (margin[چپ_2] == AUTO)
			margin[چپ_2] = 0;
		إذا (margin[راست_2] == AUTO)
			margin[راست_2] = 0;
		چپ = static_چپ;

		عرض = min(max(box->min_عرض, موجود_عرض),
			box->max_عرض);
		عرض -= box->margin[چپ_2] + box->border[چپ_2].عرض +
			box->padding[چپ_2] + box->padding[راست_2] +
			box->border[راست_2].عرض + box->margin[راست_2];

		/* Adjust for {min|max}-width */
		إذا (max_عرض >= 0 && عرض > max_عرض) عرض = max_عرض;
		إذا (عرض < min_عرض) عرض = min_عرض;

		راست = containing_بلوک->عرض -
			چپ -
			margin[چپ_2] - border[چپ_2].عرض - padding[چپ_2] -
			عرض -
			padding[راست_2] - border[راست_2].عرض - margin[راست_2];
	} وإلا إذا (چپ != AUTO && عرض != AUTO && راست != AUTO) {

		/* Adjust for {min|max}-width */
		إذا (max_عرض >= 0 && عرض > max_عرض) عرض = max_عرض;
		إذا (min_عرض >  0 && عرض < min_عرض) عرض = min_عرض;

		إذا (margin[چپ_2] == AUTO && margin[راست_2] == AUTO) {
			space = containing_بلوک->عرض -
					چپ - border[چپ_2].عرض -
					padding[چپ_2] - عرض - padding[راست_2] -
					border[راست_2].عرض - راست;
			إذا (space < 0) {
				margin[چپ_2] = 0;
				margin[راست_2] = space;
			} وإلا {
				margin[چپ_2] = margin[راست_2] = space / 2;
			}
		} وإلا إذا (margin[چپ_2] == AUTO) {
			margin[چپ_2] = containing_بلوک->عرض -
					چپ - border[چپ_2].عرض -
					padding[چپ_2] - عرض - padding[راست_2] -
					border[راست_2].عرض - margin[راست_2] -
					راست;
		} وإلا إذا (margin[راست_2] == AUTO) {
			margin[راست_2] = containing_بلوک->عرض -
					چپ - margin[چپ_2] -
					border[چپ_2].عرض -
					padding[چپ_2] - عرض - padding[راست_2] -
					border[راست_2].عرض - راست;
		} وإلا {
			راست = containing_بلوک->عرض -
					چپ - margin[چپ_2] -
					border[چپ_2].عرض -
					padding[چپ_2] - عرض - padding[راست_2] -
					border[راست_2].عرض - margin[راست_2];
		}
	} وإلا {
		إذا (margin[چپ_2] == AUTO)
			margin[چپ_2] = 0;
		إذا (margin[راست_2] == AUTO)
			margin[راست_2] = 0;

		إذا (چپ == AUTO && عرض == AUTO && راست != AUTO) {
			موجود_عرض -= راست;

			عرض = min(max(box->min_عرض, موجود_عرض),
				box->max_عرض);
			عرض -= box->margin[چپ_2] + box->border[چپ_2].عرض +
				box->padding[چپ_2] + box->padding[راست_2] +
				box->border[راست_2].عرض + box->margin[راست_2];

			/* Adjust for {min|max}-width */
			إذا (max_عرض >= 0 && عرض > max_عرض)
				عرض = max_عرض;
			إذا (عرض < min_عرض)
				عرض = min_عرض;

			چپ = containing_بلوک->عرض -
					margin[چپ_2] - border[چپ_2].عرض -
					padding[چپ_2] - عرض - padding[راست_2] -
					border[راست_2].عرض - margin[راست_2] -
					راست;
		} وإلا إذا (چپ == AUTO && عرض != AUTO && راست == AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_عرض >= 0 && عرض > max_عرض)
				عرض = max_عرض;
			إذا (min_عرض >  0 && عرض < min_عرض)
				عرض = min_عرض;

			چپ = static_چپ;
			راست = containing_بلوک->عرض -
					چپ - margin[چپ_2] -
					border[چپ_2].عرض -
					padding[چپ_2] - عرض - padding[راست_2] -
					border[راست_2].عرض - margin[راست_2];
		} وإلا إذا (چپ != AUTO && عرض == AUTO && راست == AUTO) {
			موجود_عرض -= چپ;

			عرض = min(max(box->min_عرض, موجود_عرض),
				box->max_عرض);
			عرض -= box->margin[چپ_2] + box->border[چپ_2].عرض +
				box->padding[چپ_2] + box->padding[راست_2] +
				box->border[راست_2].عرض + box->margin[راست_2];

			/* Adjust for {min|max}-width */
			إذا (max_عرض >= 0 && عرض > max_عرض)
				عرض = max_عرض;
			إذا (عرض < min_عرض)
				عرض = min_عرض;

			راست = containing_بلوک->عرض -
					چپ - margin[چپ_2] -
					border[چپ_2].عرض -
					padding[چپ_2] - عرض - padding[راست_2] -
					border[راست_2].عرض - margin[راست_2];
		} وإلا إذا (چپ == AUTO && عرض != AUTO && راست != AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_عرض >= 0 && عرض > max_عرض)
				عرض = max_عرض;
			إذا (عرض < min_عرض)
				عرض = min_عرض;

			چپ = containing_بلوک->عرض -
					margin[چپ_2] - border[چپ_2].عرض -
					padding[چپ_2] - عرض - padding[راست_2] -
					border[راست_2].عرض - margin[راست_2] -
					راست;
		} وإلا إذا (چپ != AUTO && عرض == AUTO && راست != AUTO) {
			عرض = containing_بلوک->عرض -
					چپ - margin[چپ_2] -
					border[چپ_2].عرض -
					padding[چپ_2] - padding[راست_2] -
					border[راست_2].عرض - margin[راست_2] -
					راست;

			/* Adjust for {min|max}-width */
			إذا (max_عرض >= 0 && عرض > max_عرض)
				عرض = max_عرض;
			إذا (عرض < min_عرض)
				عرض = min_عرض;

		} وإلا إذا (چپ != AUTO && عرض != AUTO && راست == AUTO) {

			/* Adjust for {min|max}-width */
			إذا (max_عرض >= 0 && عرض > max_عرض)
				عرض = max_عرض;
			إذا (عرض < min_عرض)
				عرض = min_عرض;

			راست = containing_بلوک->عرض -
					چپ - margin[چپ_2] -
					border[چپ_2].عرض -
					padding[چپ_2] - عرض - padding[راست_2] -
					border[راست_2].عرض - margin[راست_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      چپ, margin[چپ_2], border[چپ_2].عرض, padding[چپ_2], عرض,
	      padding[راست_2], border[راست_2].عرض, margin[راست_2], راست,
	      containing_بلوک->عرض);

	box->x = چپ + margin[چپ_2] + border[چپ_2].عرض - cx;
	إذا (containing_بلوک->type == BOX_بلوک ||
			containing_بلوک->type == BOX_INLINE_بلوک ||
			containing_بلوک->type == BOX_جدول_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_بلوک->عرض -= containing_بلوک->padding[چپ_2] +
				containing_بلوک->padding[راست_2];
	} وإلا {
		/** \todo inline ancestors */
	}
	box->عرض = عرض;
	box->ارتفاع = ارتفاع;

	إذا (box->type == BOX_بلوک || box->type == BOX_INLINE_بلوک ||
			box->object || box->flags & IFRAME) {
		إذا (!layout_بلوک_context(box, -1, content))
			أرجع false;
	} وإلا إذا (box->type == BOX_جدول) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_بلوک;
		/* \todo  layout_table considers margins etc. again */
		إذا (!layout_جدول(box, عرض, content))
			أرجع false;
		box->float_container = عدم;
		layout_solve_عرض(box, box->والد->عرض, box->عرض, 0, 0,
				-1, -1);
	} وإلا إذا (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_بلوک;
		إذا (!layout_flex(box, عرض, content))
			أرجع false;
		box->float_container = عدم;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      بالاترین, margin[بالاترین_2], border[بالاترین_2].عرض, padding[بالاترین_2], ارتفاع,
	      padding[BOTTOM], border[BOTTOM].عرض, margin[BOTTOM], bottom,
	      containing_بلوک->ارتفاع);

	إذا (بالاترین == AUTO && ارتفاع == AUTO && bottom == AUTO) {
		بالاترین = static_بالاترین;
		ارتفاع = box->ارتفاع;
		إذا (margin[بالاترین_2] == AUTO)
			margin[بالاترین_2] = 0;
		إذا (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_بلوک->ارتفاع -
				بالاترین - margin[بالاترین_2] - border[بالاترین_2].عرض -
				padding[بالاترین_2] - ارتفاع - padding[BOTTOM] -
				border[BOTTOM].عرض - margin[BOTTOM];
	} وإلا إذا (بالاترین != AUTO && ارتفاع != AUTO && bottom != AUTO) {
		إذا (margin[بالاترین_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_بلوک->ارتفاع -
					بالاترین - border[بالاترین_2].عرض - padding[بالاترین_2] -
					ارتفاع - padding[BOTTOM] -
					border[BOTTOM].عرض - bottom;
			margin[بالاترین_2] = margin[BOTTOM] = space / 2;
		} وإلا إذا (margin[بالاترین_2] == AUTO) {
			margin[بالاترین_2] = containing_بلوک->ارتفاع -
					بالاترین - border[بالاترین_2].عرض - padding[بالاترین_2] -
					ارتفاع - padding[BOTTOM] -
					border[BOTTOM].عرض - margin[BOTTOM] -
					bottom;
		} وإلا إذا (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_بلوک->ارتفاع -
					بالاترین - margin[بالاترین_2] - border[بالاترین_2].عرض -
					padding[بالاترین_2] - ارتفاع -
					padding[BOTTOM] - border[BOTTOM].عرض -
					bottom;
		} وإلا {
			bottom = containing_بلوک->ارتفاع -
					بالاترین - margin[بالاترین_2] - border[بالاترین_2].عرض -
					padding[بالاترین_2] - ارتفاع -
					padding[BOTTOM] - border[BOTTOM].عرض -
					margin[BOTTOM];
		}
	} وإلا {
		إذا (margin[بالاترین_2] == AUTO)
			margin[بالاترین_2] = 0;
		إذا (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		إذا (بالاترین == AUTO && ارتفاع == AUTO && bottom != AUTO) {
			ارتفاع = box->ارتفاع;
			بالاترین = containing_بلوک->ارتفاع -
					margin[بالاترین_2] - border[بالاترین_2].عرض -
					padding[بالاترین_2] - ارتفاع -
					padding[BOTTOM] - border[BOTTOM].عرض -
					margin[BOTTOM] - bottom;
		} وإلا إذا (بالاترین == AUTO && ارتفاع != AUTO && bottom == AUTO) {
			بالاترین = static_بالاترین;
			bottom = containing_بلوک->ارتفاع -
					بالاترین - margin[بالاترین_2] - border[بالاترین_2].عرض -
					padding[بالاترین_2] - ارتفاع -
					padding[BOTTOM] - border[BOTTOM].عرض -
					margin[BOTTOM];
		} وإلا إذا (بالاترین != AUTO && ارتفاع == AUTO && bottom == AUTO) {
			ارتفاع = box->ارتفاع;
			bottom = containing_بلوک->ارتفاع -
					بالاترین - margin[بالاترین_2] - border[بالاترین_2].عرض -
					padding[بالاترین_2] - ارتفاع -
					padding[BOTTOM] - border[BOTTOM].عرض -
					margin[BOTTOM];
		} وإلا إذا (بالاترین == AUTO && ارتفاع != AUTO && bottom != AUTO) {
			بالاترین = containing_بلوک->ارتفاع -
					margin[بالاترین_2] - border[بالاترین_2].عرض -
					padding[بالاترین_2] - ارتفاع -
					padding[BOTTOM] - border[BOTTOM].عرض -
					margin[BOTTOM] - bottom;
		} وإلا إذا (بالاترین != AUTO && ارتفاع == AUTO && bottom != AUTO) {
			ارتفاع = containing_بلوک->ارتفاع -
					بالاترین - margin[بالاترین_2] - border[بالاترین_2].عرض -
					padding[بالاترین_2] - padding[BOTTOM] -
					border[BOTTOM].عرض - margin[BOTTOM] -
					bottom;
		} وإلا إذا (بالاترین != AUTO && ارتفاع != AUTO && bottom == AUTO) {
			bottom = containing_بلوک->ارتفاع -
					بالاترین - margin[بالاترین_2] - border[بالاترین_2].عرض -
					padding[بالاترین_2] - ارتفاع -
					padding[BOTTOM] - border[BOTTOM].عرض -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      بالاترین, margin[بالاترین_2], border[بالاترین_2].عرض, padding[بالاترین_2], ارتفاع,
	      padding[BOTTOM], border[BOTTOM].عرض, margin[BOTTOM], bottom,
	      containing_بلوک->ارتفاع);

	box->y = بالاترین + margin[بالاترین_2] + border[بالاترین_2].عرض - cy;
	إذا (containing_بلوک->type == BOX_بلوک ||
			containing_بلوک->type == BOX_INLINE_بلوک ||
			containing_بلوک->type == BOX_جدول_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_بلوک->ارتفاع -= containing_بلوک->padding[بالاترین_2] +
				containing_بلوک->padding[BOTTOM];
	} وإلا {
		/** \todo Inline ancestors */
	}
	box->ارتفاع = ارتفاع;
	layout_apply_minmax_ارتفاع(&content->unit_len_ctx, box, containing_بلوک);

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
			 بنية box *containing_بلوک,
			 صحيح cx, صحيح cy,
			 html_content *content)
{
	بنية box *c;

	لكل (c = box->children; c; c = c->بعدی) {
		إذا ((c->type == BOX_بلوک || c->type == BOX_جدول ||
				c->type == BOX_INLINE_بلوک ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			إذا (!layout_absolute(c, containing_بلوک,
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
						CSS_FLOAT_چپ ||
					css_computed_float(c->style) ==
						CSS_FLOAT_راست)) {
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
			إذا (!layout_position_absolute(c, containing_بلوک,
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
ساكن فراغ layout_compute_relative_جابه_جایی(
		ثابت css_unit_ctx *unit_len_ctx,
		بنية box *box,
		صحيح *x,
		صحيح *y)
{
	صحيح چپ, راست, بالاترین, bottom;
	بنية box *containing_بلوک;

	أكد(box && box->والد && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	إذا (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_چپ ||
			css_computed_float(box->style) == CSS_FLOAT_راست)) {
		containing_بلوک = box->float_container;
	} وإلا {
		containing_بلوک = box->والد;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_بلوک,
			&بالاترین, &راست, &bottom, &چپ);

	إذا (چپ == AUTO && راست == AUTO)
		چپ = راست = 0;
	وإلا إذا (چپ == AUTO)
		/* left is auto => computed = -right */
		چپ = -راست;
	وإلا إذا (راست == AUTO)
		/* right is auto => computed = -left */
		راست = -چپ;
	وإلا {
		/* over constrained => examine direction property
		 * of containing block */
		إذا (containing_بلوک->style &&
				css_computed_direction(
				containing_بلوک->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			چپ = -راست;
		} وإلا {
			/* assume LTR in all other cases */
			راست = -چپ;
		}
	}

	أكد(چپ == -راست);

	إذا (بالاترین == AUTO && bottom == AUTO) {
		بالاترین = bottom = 0;
	} وإلا إذا (بالاترین == AUTO) {
		بالاترین = -bottom;
	} وإلا {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -بالاترین;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", چپ,
	      راست, بالاترین, bottom);

	*x = چپ;
	*y = بالاترین;
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
		بنية box *ریشه,
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

	أكد(ریشه);

	/* Normal children */
	لكل (box = ریشه->children; box; box = box->بعدی) {

		إذا (box->type == BOX_متن)
			استمر;

		/* If relatively positioned, get offsets */
		إذا (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_جابه_جایی(
					unit_len_ctx, box, &x, &y);
		وإلا
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		إذا (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_چپ ||
				css_computed_float(box->style) ==
					CSS_FLOAT_راست) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			لكل (fc = fp->float_children; fc; fc = fc->بعدی_float) {
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
		إذا (box->type == BOX_INLINE && box->inline_انتها) {
			بنية box *b;
			لكل (b = box->بعدی; b && b != box->inline_انتها;
					b = b->بعدی) {
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
	*desc_x0 = -box->border[چپ_2].عرض;
	*desc_y0 = -box->border[بالاترین_2].عرض;
	*desc_x1 = box->padding[چپ_2] + box->عرض + box->padding[راست_2] +
			box->border[راست_2].عرض;
	*desc_y1 = box->padding[بالاترین_2] + box->ارتفاع + box->padding[BOTTOM] +
			box->border[BOTTOM].عرض;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	إذا (box->type == BOX_بلوک && box->style != عدم &&
			css_computed_سرریز_y(box->style) ==
					CSS_سرریز_VISIBLE &&
			box->object == عدم) {
		css_fixed font_اندازه = 0;
		css_unit font_unit = CSS_UNIT_PT;
		صحيح متن_ارتفاع;

		css_computed_font_اندازه(box->style, &font_اندازه, &font_unit);
		متن_ارتفاع = css_unit_len2device_px(box->style, unit_len_ctx,
				font_اندازه, font_unit);
		متن_ارتفاع = FIXTOINT(متن_ارتفاع * 3 / 4);
		*desc_y0 = (*desc_y0 < -متن_ارتفاع) ? *desc_y0 : -متن_ارتفاع;
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
		بنية box *فرزند,
		صحيح off_x,
		صحيح off_y)
{
	صحيح فرزند_desc_x0, فرزند_desc_y0, فرزند_desc_x1, فرزند_desc_y1;

	/* get coordinates of child relative to box */
	صحيح فرزند_x = فرزند->x - off_x;
	صحيح فرزند_y = فرزند->y - off_y;

	bool html_object = (فرزند->object &&
			content_get_type(فرزند->object) == CONTENT_HTML);

	تعداد css_سرریز_e سرریز_x = CSS_سرریز_VISIBLE;
	تعداد css_سرریز_e سرریز_y = CSS_سرریز_VISIBLE;

	إذا (فرزند->style != عدم) {
		سرریز_x = css_computed_سرریز_x(فرزند->style);
		سرریز_y = css_computed_سرریز_y(فرزند->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, فرزند,
			&فرزند_desc_x0, &فرزند_desc_y0,
			&فرزند_desc_x1, &فرزند_desc_y1);

	إذا (سرریز_x == CSS_سرریز_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		فرزند_desc_x0 = فرزند->descendant_x0;
		فرزند_desc_x1 = فرزند->descendant_x1;
	}
	إذا (سرریز_y == CSS_سرریز_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		فرزند_desc_y0 = فرزند->descendant_y0;
		فرزند_desc_y1 = فرزند->descendant_y1;
	}

	فرزند_desc_x0 += فرزند_x;
	فرزند_desc_y0 += فرزند_y;
	فرزند_desc_x1 += فرزند_x;
	فرزند_desc_y1 += فرزند_y;

	/* increase box's descendant bbox to contain descendants */
	إذا (فرزند_desc_x0 < box->descendant_x0)
		box->descendant_x0 = فرزند_desc_x0;
	إذا (فرزند_desc_y0 < box->descendant_y0)
		box->descendant_y0 = فرزند_desc_y0;
	إذا (box->descendant_x1 < فرزند_desc_x1)
		box->descendant_x1 = فرزند_desc_x1;
	إذا (box->descendant_y1 < فرزند_desc_y1)
		box->descendant_y1 = فرزند_desc_y1;
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
	بنية box *فرزند;

	أكد(box->عرض != UNKNOWN_عرض);
	أكد(box->ارتفاع != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	إذا (box->object && content_get_type(box->object) == CONTENT_HTML) {
		إذا (box->descendant_x1 < content_get_عرض(box->object))
			box->descendant_x1 = content_get_عرض(box->object);
		إذا (box->descendant_y1 < content_get_ارتفاع(box->object))
			box->descendant_y1 = content_get_ارتفاع(box->object);
	}

	إذا (box->iframe != عدم) {
		صحيح x, y;
		box_coords(box, &x, &y);

		browser_پنجره_مجموعه_position(box->iframe, x, y);
		browser_پنجره_مجموعه_dimensions(box->iframe,
				box->عرض, box->ارتفاع);
		browser_پنجره_reformat(box->iframe, true,
				box->عرض, box->ارتفاع);
	}

	إذا (box->type == BOX_INLINE || box->type == BOX_متن)
		أرجع;

	إذا (box->type == BOX_INLINE_انتها) {
		box = box->inline_انتها;
		لكل (فرزند = box->بعدی; فرزند;
				فرزند = فرزند->بعدی) {
			إذا (فرزند->type == BOX_FLOAT_چپ ||
					فرزند->type == BOX_FLOAT_راست)
				استمر;

			layout_update_descendant_bbox(unit_len_ctx, box, فرزند,
					box->x, box->y);

			إذا (فرزند == box->inline_انتها)
				اكسر;
		}
		أرجع;
	}

	إذا (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		أرجع;

	لكل (فرزند = box->children; فرزند; فرزند = فرزند->بعدی) {
		إذا (فرزند->type == BOX_FLOAT_چپ ||
				فرزند->type == BOX_FLOAT_راست)
			استمر;

		layout_calculate_descendant_bboxes(unit_len_ctx, فرزند);

		إذا (box->style && css_computed_سرریز_x(box->style) ==
				CSS_سرریز_HIDDEN &&
				css_computed_سرریز_y(box->style) ==
				CSS_سرریز_HIDDEN)
			استمر;

		layout_update_descendant_bbox(unit_len_ctx, box, فرزند, 0, 0);
	}

	لكل (فرزند = box->float_children; فرزند; فرزند = فرزند->بعدی_float) {
		أكد(فرزند->type == BOX_FLOAT_چپ ||
				فرزند->type == BOX_FLOAT_راست);

		layout_calculate_descendant_bboxes(unit_len_ctx, فرزند);

		layout_update_descendant_bbox(unit_len_ctx, box, فرزند, 0, 0);
	}

	إذا (box->list_marker) {
		فرزند = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, فرزند);

		layout_update_descendant_bbox(unit_len_ctx, box, فرزند, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, صحيح عرض, صحيح ارتفاع)
{
	bool ret;
	بنية box *doc = content->layout;
	ثابت بنية gui_layout_جدول *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			عرض, ارتفاع, nsurl_access(content_get_url(
					&content->پایه)));

	layout_minmax_بلوک(doc, font_func, content);

	layout_بلوک_پیدا_کن_dimensions(&content->unit_len_ctx,
			عرض, ارتفاع, 0, 0, doc);
	doc->x = doc->margin[چپ_2] + doc->border[چپ_2].عرض;
	doc->y = doc->margin[بالاترین_2] + doc->border[بالاترین_2].عرض;
	عرض -= doc->margin[چپ_2] + doc->border[چپ_2].عرض +
			doc->padding[چپ_2] + doc->padding[راست_2] +
			doc->border[راست_2].عرض + doc->margin[راست_2];
	إذا (عرض < 0) {
		عرض = 0;
	}
	doc->عرض = عرض;

	ret = layout_بلوک_context(doc, ارتفاع, content);

	/* make <html> and <body> fill available height */
	إذا (doc->y + doc->padding[بالاترین_2] + doc->ارتفاع + doc->padding[BOTTOM] +
			doc->border[BOTTOM].عرض + doc->margin[BOTTOM] <
			ارتفاع) {
		doc->ارتفاع = ارتفاع - (doc->y + doc->padding[بالاترین_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].عرض +
				doc->margin[BOTTOM]);
		إذا (doc->children)
			doc->children->ارتفاع = doc->ارتفاع -
					(doc->children->margin[بالاترین_2] +
					 doc->children->border[بالاترین_2].عرض +
					 doc->children->padding[بالاترین_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].عرض +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	أرجع ret;
}
