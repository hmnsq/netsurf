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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <math.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/talloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/browser_window.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/layout.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/scrollbar.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/textarea.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/html.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/html_save.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_inspect.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/font.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/form_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/layout.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/layout_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/table.h"

/** Array of per-side access functions for computed style margins. */
ਅਚਲ css_len_func margin_funcs[4] = {
	[ਸਿਖਰ_2]    = css_computed_margin_ਸਿਖਰ,
	[ਸੱਜਾ_2]  = css_computed_margin_ਸੱਜਾ,
	[BOTTOM] = css_computed_margin_bottom,
	[ਖੱਬਾ_2]   = css_computed_margin_ਖੱਬਾ,
};

/** Array of per-side access functions for computed style paddings. */
ਅਚਲ css_len_func padding_funcs[4] = {
	[ਸਿਖਰ_2]    = css_computed_padding_ਸਿਖਰ,
	[ਸੱਜਾ_2]  = css_computed_padding_ਸੱਜਾ,
	[BOTTOM] = css_computed_padding_bottom,
	[ਖੱਬਾ_2]   = css_computed_padding_ਖੱਬਾ,
};

/** Array of per-side access functions for computed style border_widths. */
ਅਚਲ css_len_func border_ਚੌੜਾਈ_funcs[4] = {
	[ਸਿਖਰ_2]    = css_computed_border_ਸਿਖਰ_ਚੌੜਾਈ,
	[ਸੱਜਾ_2]  = css_computed_border_ਸੱਜਾ_ਚੌੜਾਈ,
	[BOTTOM] = css_computed_border_bottom_ਚੌੜਾਈ,
	[ਖੱਬਾ_2]   = css_computed_border_ਖੱਬਾ_ਚੌੜਾਈ,
};

/** Array of per-side access functions for computed style border styles. */
ਅਚਲ css_border_style_func border_style_funcs[4] = {
	[ਸਿਖਰ_2]    = css_computed_border_ਸਿਖਰ_style,
	[ਸੱਜਾ_2]  = css_computed_border_ਸੱਜਾ_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[ਖੱਬਾ_2]   = css_computed_border_ਖੱਬਾ_style,
};

/** Array of per-side access functions for computed style border colors. */
ਅਚਲ css_border_color_func border_color_funcs[4] = {
	[ਸਿਖਰ_2]    = css_computed_border_ਸਿਖਰ_color,
	[ਸੱਜਾ_2]  = css_computed_border_ਸੱਜਾ_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[ਖੱਬਾ_2]   = css_computed_border_ਖੱਬਾ_color,
};

/* forward declaration to break cycles */
ਸਥਿਰ ਖਾਲੀ layout_minmax_ਖੰਡ(
		ਬਣਤਰ box *ਖੰਡ,
		ਅਚਲ ਬਣਤਰ gui_layout_ਸਾਰਣੀ *font_func,
		ਅਚਲ html_content *content);

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
ਸਥਿਰ ਖਾਲੀ
layout_get_object_dimensions(ਬਣਤਰ box *box,
			     ਪੂਰਨ *ਚੌੜਾਈ, ਪੂਰਨ *ਉਚਾਈ,
			     ਪੂਰਨ min_ਚੌੜਾਈ, ਪੂਰਨ max_ਚੌੜਾਈ,
			     ਪੂਰਨ min_ਉਚਾਈ, ਪੂਰਨ max_ਉਚਾਈ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(box->object != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਚੌੜਾਈ != ਨਿਰਾ && ਉਚਾਈ != ਨਿਰਾ);

	ਜੇ (*ਚੌੜਾਈ == AUTO && *ਉਚਾਈ == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		ਪੂਰਨ intrinsic_ਚੌੜਾਈ = content_get_ਚੌੜਾਈ(box->object);
		ਪੂਰਨ intrinsic_ਉਚਾਈ = content_get_ਉਚਾਈ(box->object);

		/* use intrinsic dimensions */
		*ਚੌੜਾਈ = intrinsic_ਚੌੜਾਈ;
		*ਉਚਾਈ = intrinsic_ਉਚਾਈ;

		/* Deal with min/max-width first */
		ਜੇ (min_ਚੌੜਾਈ >  0 && min_ਚੌੜਾਈ > *ਚੌੜਾਈ) {
			*ਚੌੜਾਈ = min_ਚੌੜਾਈ;
			scaled = true;
		}
		ਜੇ (max_ਚੌੜਾਈ >= 0 && max_ਚੌੜਾਈ < *ਚੌੜਾਈ) {
			*ਚੌੜਾਈ = max_ਚੌੜਾਈ;
			scaled = true;
		}

		ਜੇ (scaled && (intrinsic_ਚੌੜਾਈ != 0)) {
			/* Update height */
			*ਉਚਾਈ = (*ਚੌੜਾਈ * intrinsic_ਉਚਾਈ) /
					intrinsic_ਚੌੜਾਈ;
		}

		scaled = false;
		/* Deal with min/max-height */
		ਜੇ (min_ਉਚਾਈ >  0 && min_ਉਚਾਈ > *ਉਚਾਈ) {
			*ਉਚਾਈ = min_ਉਚਾਈ;
			scaled = true;
		}
		ਜੇ (max_ਉਚਾਈ >= 0 && max_ਉਚਾਈ < *ਉਚਾਈ) {
			*ਉਚਾਈ = max_ਉਚਾਈ;
			scaled = true;
		}

		ਜੇ (scaled && (intrinsic_ਉਚਾਈ != 0)) {
			/* Update width */
			*ਚੌੜਾਈ = (*ਉਚਾਈ * intrinsic_ਚੌੜਾਈ) /
					intrinsic_ਉਚਾਈ;
		}

	} ਨਹੀਂਤਾਂ ਜੇ (*ਚੌੜਾਈ == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		ਪੂਰਨ intrinsic_ਚੌੜਾਈ = content_get_ਚੌੜਾਈ(box->object);
		ਪੂਰਨ intrinsic_ਉਚਾਈ = content_get_ਉਚਾਈ(box->object);

		ਜੇ (intrinsic_ਉਚਾਈ != 0)
			*ਚੌੜਾਈ = (*ਉਚਾਈ * intrinsic_ਚੌੜਾਈ) /
					intrinsic_ਉਚਾਈ;
		ਨਹੀਂਤਾਂ
			*ਚੌੜਾਈ = intrinsic_ਚੌੜਾਈ;

		ਜੇ (min_ਚੌੜਾਈ >  0 && min_ਚੌੜਾਈ > *ਚੌੜਾਈ)
			*ਚੌੜਾਈ = min_ਚੌੜਾਈ;
		ਜੇ (max_ਚੌੜਾਈ >= 0 && max_ਚੌੜਾਈ < *ਚੌੜਾਈ)
			*ਚੌੜਾਈ = max_ਚੌੜਾਈ;

	} ਨਹੀਂਤਾਂ ਜੇ (*ਉਚਾਈ == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		ਪੂਰਨ intrinsic_ਚੌੜਾਈ = content_get_ਚੌੜਾਈ(box->object);
		ਪੂਰਨ intrinsic_ਉਚਾਈ = content_get_ਉਚਾਈ(box->object);

		ਜੇ (min_ਚੌੜਾਈ >  0 && min_ਚੌੜਾਈ > *ਚੌੜਾਈ)
			*ਚੌੜਾਈ = min_ਚੌੜਾਈ;
		ਜੇ (max_ਚੌੜਾਈ >= 0 && max_ਚੌੜਾਈ < *ਚੌੜਾਈ)
			*ਚੌੜਾਈ = max_ਚੌੜਾਈ;

		ਜੇ (intrinsic_ਚੌੜਾਈ != 0)
			*ਉਚਾਈ = (*ਚੌੜਾਈ * intrinsic_ਉਚਾਈ) /
					intrinsic_ਚੌੜਾਈ;
		ਨਹੀਂਤਾਂ
			*ਉਚਾਈ = intrinsic_ਉਚਾਈ;
	}
}


/**
 * Calculate the text-indent length.
 *
 * \param  style  style of block
 * \param  width  width of containing block
 * \return  length of indent
 */
ਸਥਿਰ ਪੂਰਨ layout_ਪਾਠ_indent(
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਅਚਲ css_computed_style *style, ਪੂਰਨ ਚੌੜਾਈ)
{
	css_fixed ਮੁੱਲ = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_ਪਾਠ_indent(style, &ਮੁੱਲ, &unit);

	ਜੇ (unit == CSS_UNIT_PCT) {
		ਵਾਪਸ FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ, ਚੌੜਾਈ);
	} ਨਹੀਂਤਾਂ {
		ਵਾਪਸ FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				ਮੁੱਲ, unit));
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
ਸਥਿਰ ਖਾਲੀ layout_minmax_ਸਾਰਣੀ(ਬਣਤਰ box *ਸਾਰਣੀ,
		ਅਚਲ ਬਣਤਰ gui_layout_ਸਾਰਣੀ *font_func,
		ਅਚਲ html_content *content)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i, j;
	ਪੂਰਨ ਚੌੜਾਈ;
	ਪੂਰਨ border_spacing_h = 0;
	ਪੂਰਨ ਸਾਰਣੀ_min = 0, ਸਾਰਣੀ_max = 0;
	ਪੂਰਨ extra_fixed = 0;
	ਦਸ਼ਮਲਵ extra_frac = 0;
	ਬਣਤਰ ਕਾਲਮ *ਖਾਨਾ;
	ਬਣਤਰ box *ਪੰਕਤੀ_ਸਮੂਹ, *ਪੰਕਤੀ, *cell;

	/* check if the widths have already been calculated */
	ਜੇ (ਸਾਰਣੀ->max_ਚੌੜਾਈ != UNKNOWN_MAX_ਚੌੜਾਈ)
		ਵਾਪਸ;

	ਜੇ (ਸਾਰਣੀ_calculate_ਕਾਲਮ_types(&content->unit_len_ctx, ਸਾਰਣੀ) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		ਵਾਪਸ;
	}
	ਖਾਨਾ = ਸਾਰਣੀ->ਖਾਨਾ;

	/* start with 0 except for fixed-width columns */
	ਲਈ (i = 0; i != ਸਾਰਣੀ->columns; i++) {
		ਜੇ (ਖਾਨਾ[i].type == ਕਾਲਮ_ਚੌੜਾਈ_FIXED)
			ਖਾਨਾ[i].min = ਖਾਨਾ[i].max = ਖਾਨਾ[i].ਚੌੜਾਈ;
		ਨਹੀਂਤਾਂ
			ਖਾਨਾ[i].min = ਖਾਨਾ[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	ਜੇ (css_computed_border_collapse(ਸਾਰਣੀ->style) ==
			CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		css_computed_border_spacing(ਸਾਰਣੀ->style, &h, &hu, &v, &vu);

		border_spacing_h = FIXTOINT(css_unit_len2device_px(
				ਸਾਰਣੀ->style,
				&content->unit_len_ctx,
				h, hu));
	}

	/* 1st pass: consider cells with colspan 1 only */
	ਲਈ (ਪੰਕਤੀ_ਸਮੂਹ = ਸਾਰਣੀ->children; ਪੰਕਤੀ_ਸਮੂਹ; ਪੰਕਤੀ_ਸਮੂਹ =ਪੰਕਤੀ_ਸਮੂਹ->ਅਗਲਾ)
	ਲਈ (ਪੰਕਤੀ = ਪੰਕਤੀ_ਸਮੂਹ->children; ਪੰਕਤੀ; ਪੰਕਤੀ = ਪੰਕਤੀ->ਅਗਲਾ)
	ਲਈ (cell = ਪੰਕਤੀ->children; cell; cell = cell->ਅਗਲਾ) {
		ਪ੍ਰਤਿਜ੍ਞਾ(cell->type == BOX_ਸਾਰਣੀ_CELL);
		ਪ੍ਰਤਿਜ੍ਞਾ(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		ਪ੍ਰਤਿਜ੍ਞਾ(cell->columns != 0);

		ਜੇ (cell->columns != 1)
			ਜਾਰੀ;

		layout_minmax_ਖੰਡ(cell, font_func, content);
		i = cell->ਸ਼ੁਰੂ_ਕਾਲਮ;

		ਜੇ (ਖਾਨਾ[i].positioned)
			ਜਾਰੀ;

		/* update column min, max widths using cell widths */
		ਜੇ (ਖਾਨਾ[i].min < cell->min_ਚੌੜਾਈ)
			ਖਾਨਾ[i].min = cell->min_ਚੌੜਾਈ;
		ਜੇ (ਖਾਨਾ[i].max < cell->max_ਚੌੜਾਈ)
			ਖਾਨਾ[i].max = cell->max_ਚੌੜਾਈ;
	}

	/* 2nd pass: cells which span multiple columns */
	ਲਈ (ਪੰਕਤੀ_ਸਮੂਹ = ਸਾਰਣੀ->children; ਪੰਕਤੀ_ਸਮੂਹ; ਪੰਕਤੀ_ਸਮੂਹ =ਪੰਕਤੀ_ਸਮੂਹ->ਅਗਲਾ)
	ਲਈ (ਪੰਕਤੀ = ਪੰਕਤੀ_ਸਮੂਹ->children; ਪੰਕਤੀ; ਪੰਕਤੀ = ਪੰਕਤੀ->ਅਗਲਾ)
	ਲਈ (cell = ਪੰਕਤੀ->children; cell; cell = cell->ਅਗਲਾ) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flexible_columns = 0;
		ਪੂਰਨ min = 0, max = 0, fixed_ਚੌੜਾਈ = 0, extra;

		ਜੇ (cell->columns == 1)
			ਜਾਰੀ;

		layout_minmax_ਖੰਡ(cell, font_func, content);
		i = cell->ਸ਼ੁਰੂ_ਕਾਲਮ;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		ਲਈ (j = 0; j != cell->columns; j++) {
			min += ਖਾਨਾ[i + j].min;
			ਜੇ (ਖਾਨਾ[i + j].type == ਕਾਲਮ_ਚੌੜਾਈ_FIXED)
				fixed_ਚੌੜਾਈ += ਖਾਨਾ[i + j].ਚੌੜਾਈ;
			ਨਹੀਂਤਾਂ
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		ਜੇ (min < cell->min_ਚੌੜਾਈ) {
			ਜੇ (flexible_columns == 0) {
				extra = 1 + (cell->min_ਚੌੜਾਈ - min) /
						cell->columns;
				ਲਈ (j = 0; j != cell->columns; j++) {
					ਖਾਨਾ[i + j].min += extra;
					ਜੇ (ਖਾਨਾ[i + j].max < ਖਾਨਾ[i + j].min)
						ਖਾਨਾ[i + j].max = ਖਾਨਾ[i + j].min;
				}
			} ਨਹੀਂਤਾਂ {
				extra = 1 + (cell->min_ਚੌੜਾਈ - min) /
						flexible_columns;
				ਲਈ (j = 0; j != cell->columns; j++) {
					ਜੇ (ਖਾਨਾ[i + j].type !=
							ਕਾਲਮ_ਚੌੜਾਈ_FIXED) {
						ਖਾਨਾ[i + j].min += extra;
						ਜੇ (ਖਾਨਾ[i + j].max <
								ਖਾਨਾ[i + j].min)
							ਖਾਨਾ[i + j].max =
								ਖਾਨਾ[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		ਲਈ (j = 0; j != cell->columns; j++)
			max += ਖਾਨਾ[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		ਜੇ (max < cell->max_ਚੌੜਾਈ && flexible_columns) {
			extra = 1 + (cell->max_ਚੌੜਾਈ - max) / flexible_columns;
			ਲਈ (j = 0; j != cell->columns; j++)
				ਜੇ (ਖਾਨਾ[i + j].type != ਕਾਲਮ_ਚੌੜਾਈ_FIXED)
					ਖਾਨਾ[i + j].max += extra;
		}
	}

	ਲਈ (i = 0; i != ਸਾਰਣੀ->columns; i++) {
		ਜੇ (ਖਾਨਾ[i].max < ਖਾਨਾ[i].min) {
			box_dump(stderr, ਸਾਰਣੀ, 0, true);
			ਪ੍ਰਤਿਜ੍ਞਾ(0);
		}
		ਸਾਰਣੀ_min += ਖਾਨਾ[i].min;
		ਸਾਰਣੀ_max += ਖਾਨਾ[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	ਜੇ (css_computed_ਚੌੜਾਈ_px(ਸਾਰਣੀ->style, &content->unit_len_ctx,
			-1, &ਚੌੜਾਈ) == CSS_ਚੌੜਾਈ_ਸੈੱਟ) {
		ਜੇ (ਸਾਰਣੀ_min < ਚੌੜਾਈ)
			ਸਾਰਣੀ_min = ਚੌੜਾਈ;
		ਜੇ (ਸਾਰਣੀ_max < ਚੌੜਾਈ)
			ਸਾਰਣੀ_max = ਚੌੜਾਈ;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
			ਸਾਰਣੀ->style, ਖੱਬਾ_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
			ਸਾਰਣੀ->style, ਸੱਜਾ_2, true, true, true,
			&extra_fixed, &extra_frac);
	ਜੇ (extra_fixed < 0)
		extra_fixed = 0;
	ਜੇ (extra_frac < 0)
		extra_frac = 0;
	ਜੇ (1.0 <= extra_frac)
		extra_frac = 0.9;
	ਸਾਰਣੀ->min_ਚੌੜਾਈ = (ਸਾਰਣੀ_min + extra_fixed) / (1.0 - extra_frac);
	ਸਾਰਣੀ->max_ਚੌੜਾਈ = (ਸਾਰਣੀ_max + extra_fixed) / (1.0 - extra_frac);
	ਸਾਰਣੀ->min_ਚੌੜਾਈ += (ਸਾਰਣੀ->columns + 1) * border_spacing_h;
	ਸਾਰਣੀ->max_ਚੌੜਾਈ += (ਸਾਰਣੀ->columns + 1) * border_spacing_h;

	ਪ੍ਰਤਿਜ੍ਞਾ(0 <= ਸਾਰਣੀ->min_ਚੌੜਾਈ && ਸਾਰਣੀ->min_ਚੌੜਾਈ <= ਸਾਰਣੀ->max_ਚੌੜਾਈ);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool box_has_percentage_max_ਚੌੜਾਈ(ਬਣਤਰ box *b)
{
	css_unit unit = CSS_UNIT_PX;
	ਗਿਣਤੀ css_max_ਚੌੜਾਈ_e type;
	css_fixed ਮੁੱਲ = 0;

	ਪ੍ਰਤਿਜ੍ਞਾ(b != ਨਿਰਾ);

	type = css_computed_max_ਚੌੜਾਈ(b->style, &ਮੁੱਲ, &unit);
	ਵਾਪਸ ((type == CSS_MAX_ਚੌੜਾਈ_ਸੈੱਟ) && (unit == CSS_UNIT_PCT));
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
ਸਥਿਰ ਬਣਤਰ box *
layout_minmax_ਸਤਰ(ਬਣਤਰ box *ਪਹਿਲਾ,
		   ਪੂਰਨ *ਸਤਰ_min,
		   ਪੂਰਨ *ਸਤਰ_max,
		   bool ਪਹਿਲਾ_ਸਤਰ,
		   bool *ਸਤਰ_has_ਉਚਾਈ,
		   ਅਚਲ ਬਣਤਰ gui_layout_ਸਾਰਣੀ *font_func,
		   ਅਚਲ html_content *content)
{
	ਪੂਰਨ min = 0, max = 0, ਚੌੜਾਈ, ਉਚਾਈ, fixed;
	ਦਸ਼ਮਲਵ frac;
	size_t i, j;
	ਬਣਤਰ box *b;
	ਬਣਤਰ box *ਖੰਡ;
	plot_font_style_t fstyle;
	bool no_wrap;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਪਹਿਲਾ->ਮਾਪਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਪਹਿਲਾ->ਮਾਪਾ->ਮਾਪਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਪਹਿਲਾ->ਮਾਪਾ->ਮਾਪਾ->style);

	ਖੰਡ = ਪਹਿਲਾ->ਮਾਪਾ->ਮਾਪਾ;
	no_wrap = (css_computed_white_space(ਖੰਡ->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(ਖੰਡ->style) ==
			CSS_WHITE_SPACE_PRE);

	*ਸਤਰ_has_ਉਚਾਈ = false;

	/* corresponds to the pass 1 loop in layout_line() */
	ਲਈ (b = ਪਹਿਲਾ; b; b = b->ਅਗਲਾ) {
		ਗਿਣਤੀ css_ਚੌੜਾਈ_e wtype;
		ਗਿਣਤੀ css_ਉਚਾਈ_e htype;
		ਗਿਣਤੀ css_box_sizing_e bs;
		css_fixed ਮੁੱਲ = 0;
		css_unit unit = CSS_UNIT_PX;

		ਪ੍ਰਤਿਜ੍ਞਾ(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		ਜੇ (b->type == BOX_BR) {
			b = b->ਅਗਲਾ;
			ਰੋਕੋ;
		}

		ਜੇ (lh__box_is_float_box(b)) {
			ਪ੍ਰਤਿਜ੍ਞਾ(b->children);
			ਜੇ (b->children->type == BOX_ਸਾਰਣੀ)
				layout_minmax_ਸਾਰਣੀ(b->children, font_func,
						content);
			ਨਹੀਂਤਾਂ
				layout_minmax_ਖੰਡ(b->children, font_func,
						content);
			b->min_ਚੌੜਾਈ = b->children->min_ਚੌੜਾਈ;
			b->max_ਚੌੜਾਈ = b->children->max_ਚੌੜਾਈ;
			ਜੇ (min < b->min_ਚੌੜਾਈ)
				min = b->min_ਚੌੜਾਈ;
			max += b->max_ਚੌੜਾਈ;
			ਜਾਰੀ;
		}

		ਜੇ (b->type == BOX_INLINE_ਖੰਡ || b->type == BOX_INLINE_FLEX) {
			layout_minmax_ਖੰਡ(b, font_func, content);
			ਜੇ (min < b->min_ਚੌੜਾਈ)
				min = b->min_ਚੌੜਾਈ;
			max += b->max_ਚੌੜਾਈ;

			ਜੇ (b->flags & HAS_ਉਚਾਈ)
				*ਸਤਰ_has_ਉਚਾਈ = true;
			ਜਾਰੀ;
		}

		ਪ੍ਰਤਿਜ੍ਞਾ(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		ਜੇ (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
					b->style, ਖੱਬਾ_2, true, true, true,
					&fixed, &frac);
			ਜੇ (!b->inline_ਸਿਰਾ)
				calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
						b->style, ਸੱਜਾ_2,
						true, true, true,
						&fixed, &frac);
			ਜੇ (0 < fixed)
				max += fixed;
			*ਸਤਰ_has_ਉਚਾਈ = true;
			/* \todo  update min width, consider fractional extra */
		} ਨਹੀਂਤਾਂ ਜੇ (b->type == BOX_INLINE_ਸਿਰਾ) {
			fixed = frac = 0;
			calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
					b->inline_ਸਿਰਾ->style, ਸੱਜਾ_2,
					true, true, true,
					&fixed, &frac);
			ਜੇ (0 < fixed)
				max += fixed;

			ਜੇ (b->ਅਗਲਾ) {
				ਜੇ (b->space == UNKNOWN_ਚੌੜਾਈ) {
					font_func->ਚੌੜਾਈ(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*ਸਤਰ_has_ਉਚਾਈ = true;
			ਜਾਰੀ;
		}

		ਜੇ (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			ਜੇ (!b->ਪਾਠ)
				ਜਾਰੀ;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			ਜੇ (b->ਚੌੜਾਈ == UNKNOWN_ਚੌੜਾਈ) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				ਜੇ (b->ਮਾਪਾ->ਮਾਪਾ->gadget &&
						b->ਮਾਪਾ->ਮਾਪਾ->gadget->type
						== GADGET_SELECT) {
					ਪੂਰਨ opt_maxwidth = 0;
					ਬਣਤਰ form_option *o;

					ਲਈ (o = b->ਮਾਪਾ->ਮਾਪਾ->gadget->
							ਅੰਕੜੇ.select.ਚੀਜ਼ਾਂ; o;
							o = o->ਅਗਲਾ) {
						ਪੂਰਨ opt_ਚੌੜਾਈ;
						font_func->ਚੌੜਾਈ(&fstyle,
								o->ਪਾਠ,
								ਲੜੀ_ਲੰਬਾਈ(o->ਪਾਠ),
								&opt_ਚੌੜਾਈ);

						ਜੇ (opt_maxwidth < opt_ਚੌੜਾਈ)
							opt_maxwidth =opt_ਚੌੜਾਈ;
					}

					b->ਚੌੜਾਈ = opt_maxwidth;
					ਜੇ (nsoption_bool(core_select_menu))
						b->ਚੌੜਾਈ += SCROLLBAR_ਚੌੜਾਈ;

				} ਨਹੀਂਤਾਂ {
					font_func->ਚੌੜਾਈ(&fstyle, b->ਪਾਠ,
						b->ਲੰਮਾਈ, &b->ਚੌੜਾਈ);
					b->flags |= MEASURED;
				}
			}
			max += b->ਚੌੜਾਈ;
			ਜੇ (b->ਅਗਲਾ) {
				ਜੇ (b->space == UNKNOWN_ਚੌੜਾਈ) {
					font_func->ਚੌੜਾਈ(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			ਜੇ (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} ਨਹੀਂਤਾਂ ਜੇ (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				ਜੇ (min < b->ਚੌੜਾਈ)
					min = b->ਚੌੜਾਈ;

			} ਨਹੀਂਤਾਂ ਜੇ (b->ਮਾਪਾ->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				ਕਰੋ {
					ਲਈ (j = i; j != b->ਲੰਮਾਈ &&
							b->ਪਾਠ[j] != ' '; j++)
						;
					font_func->ਚੌੜਾਈ(&fstyle, b->ਪਾਠ + i,
							 j - i, &ਚੌੜਾਈ);
					ਜੇ (min < ਚੌੜਾਈ)
						min = ਚੌੜਾਈ;
					i = j + 1;
				} ਜਦੋਂ (j != b->ਲੰਮਾਈ);
			}

			*ਸਤਰ_has_ਉਚਾਈ = true;

			ਜਾਰੀ;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		ਪ੍ਰਤਿਜ੍ਞਾ(b->style);

		bs = css_computed_box_sizing(ਖੰਡ->style);

		/* calculate box width */
		wtype = css_computed_ਚੌੜਾਈ_px(b->style,
				&content->unit_len_ctx, -1, &ਚੌੜਾਈ);
		ਜੇ (wtype == CSS_ਚੌੜਾਈ_ਸੈੱਟ) {
			ਜੇ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
						ਖੰਡ->style, ਖੱਬਾ_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
						ਖੰਡ->style, ਸੱਜਾ_2,
						false, true, true,
						&fixed, &frac);
				ਜੇ (ਚੌੜਾਈ < fixed) {
					ਚੌੜਾਈ = fixed;
				}
			}
			ਜੇ (ਚੌੜਾਈ < 0)
				ਚੌੜਾਈ = 0;
		} ਨਹੀਂਤਾਂ {
			ਚੌੜਾਈ = AUTO;
		}

		/*ਉਚਾਈ*/
		htype = css_computed_ਉਚਾਈ(b->style, &ਮੁੱਲ, &unit);
		ਜੇ (htype == CSS_ਉਚਾਈ_ਸੈੱਟ) {
			ਉਚਾਈ = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					ਮੁੱਲ, unit));
		} ਨਹੀਂਤਾਂ {
			ਉਚਾਈ = AUTO;
		}

		ਜੇ (b->object || (b->flags & REPLACE_DIM)) {
			ਜੇ (b->object) {
				ਪੂਰਨ ਆਰਜ਼ੀ_ਉਚਾਈ = ਉਚਾਈ;
				layout_get_object_dimensions(b,
						&ਚੌੜਾਈ, &ਆਰਜ਼ੀ_ਉਚਾਈ,
						INT_MIN, ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ,
						INT_MIN, ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ);
			}

			fixed = frac = 0;
			ਜੇ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
						b->style, ਖੱਬਾ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
						b->style, ਸੱਜਾ_2,
						true, false, false,
						&fixed, &frac);
			} ਨਹੀਂਤਾਂ {
				calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
						b->style, ਖੱਬਾ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
						b->style, ਸੱਜਾ_2,
						true, true, true,
						&fixed, &frac);
			}
			ਜੇ (0 < ਚੌੜਾਈ + fixed)
				ਚੌੜਾਈ += fixed;
		} ਨਹੀਂਤਾਂ ਜੇ (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			ਜੇ (ਚੌੜਾਈ == AUTO)
				ਚੌੜਾਈ = 400;

			fixed = frac = 0;
			ਜੇ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
						b->style, ਖੱਬਾ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
						b->style, ਸੱਜਾ_2,
						true, false, false,
						&fixed, &frac);
			} ਨਹੀਂਤਾਂ {
				calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
						b->style, ਖੱਬਾ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx,
						b->style, ਸੱਜਾ_2,
						true, true, true,
						&fixed, &frac);
			}

			ਜੇ (0 < ਚੌੜਾਈ + fixed)
				ਚੌੜਾਈ += fixed;

		} ਨਹੀਂਤਾਂ {
			/* form control with no object */
			ਜੇ (ਚੌੜਾਈ == AUTO)
				ਚੌੜਾਈ = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		ਜੇ (min < ਚੌੜਾਈ && !box_has_percentage_max_ਚੌੜਾਈ(b))
			min = ਚੌੜਾਈ;
		ਜੇ (ਚੌੜਾਈ > 0)
			max += ਚੌੜਾਈ;

		*ਸਤਰ_has_ਉਚਾਈ = true;
	}

	ਜੇ (ਪਹਿਲਾ_ਸਤਰ) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		ਪੂਰਨ ਪਾਠ_indent = layout_ਪਾਠ_indent(&content->unit_len_ctx,
				ਪਹਿਲਾ->ਮਾਪਾ->ਮਾਪਾ->style, 100);
		min = (min + ਪਾਠ_indent < 0) ? 0 : min + ਪਾਠ_indent;
		max = (max + ਪਾਠ_indent < 0) ? 0 : max + ਪਾਠ_indent;
	}

	*ਸਤਰ_min = min;
	*ਸਤਰ_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	ਪ੍ਰਤਿਜ੍ਞਾ(b != ਪਹਿਲਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(0 <= *ਸਤਰ_min);
	ਪ੍ਰਤਿਜ੍ਞਾ(*ਸਤਰ_min <= *ਸਤਰ_max);
	ਵਾਪਸ b;
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
ਸਥਿਰ ਖਾਲੀ
layout_minmax_inline_container(ਬਣਤਰ box *inline_container,
			       bool *has_ਉਚਾਈ,
			       ਅਚਲ ਬਣਤਰ gui_layout_ਸਾਰਣੀ *font_func,
			       ਅਚਲ html_content *content)
{
	ਬਣਤਰ box *ਬੱਚਾ;
	ਪੂਰਨ ਸਤਰ_min = 0, ਸਤਰ_max = 0;
	ਪੂਰਨ min = 0, max = 0;
	bool ਪਹਿਲਾ_ਸਤਰ = true;
	bool ਸਤਰ_has_ਉਚਾਈ;

	ਪ੍ਰਤਿਜ੍ਞਾ(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	ਜੇ (inline_container->max_ਚੌੜਾਈ != UNKNOWN_MAX_ਚੌੜਾਈ)
		ਵਾਪਸ;

	*has_ਉਚਾਈ = false;

	ਲਈ (ਬੱਚਾ = inline_container->children; ਬੱਚਾ; ) {
		ਬੱਚਾ = layout_minmax_ਸਤਰ(ਬੱਚਾ, &ਸਤਰ_min, &ਸਤਰ_max,
				ਪਹਿਲਾ_ਸਤਰ, &ਸਤਰ_has_ਉਚਾਈ, font_func,
				content);
		ਜੇ (min < ਸਤਰ_min)
			min = ਸਤਰ_min;
		ਜੇ (max < ਸਤਰ_max)
			max = ਸਤਰ_max;
		ਪਹਿਲਾ_ਸਤਰ = false;
		*has_ਉਚਾਈ |= ਸਤਰ_has_ਉਚਾਈ;
	}

	inline_container->min_ਚੌੜਾਈ = min;
	inline_container->max_ਚੌੜਾਈ = max;

	ਪ੍ਰਤਿਜ੍ਞਾ(0 <= inline_container->min_ਚੌੜਾਈ &&
			inline_container->min_ਚੌੜਾਈ <=
			inline_container->max_ਚੌੜਾਈ);
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
ਸਥਿਰ ਖਾਲੀ layout_minmax_ਖੰਡ(
		ਬਣਤਰ box *ਖੰਡ,
		ਅਚਲ ਬਣਤਰ gui_layout_ਸਾਰਣੀ *font_func,
		ਅਚਲ html_content *content)
{
	ਬਣਤਰ box *ਬੱਚਾ;
	ਪੂਰਨ min = 0, max = 0;
	ਪੂਰਨ extra_fixed = 0;
	ਦਸ਼ਮਲਵ extra_frac = 0;
	ਗਿਣਤੀ css_ਚੌੜਾਈ_e wtype = CSS_ਚੌੜਾਈ_AUTO;
	css_fixed ਚੌੜਾਈ = 0;
	css_unit wunit = CSS_UNIT_PX;
	ਗਿਣਤੀ css_ਉਚਾਈ_e htype = CSS_ਉਚਾਈ_AUTO;
	css_fixed ਉਚਾਈ = 0;
	css_unit hunit = CSS_UNIT_PX;
	ਗਿਣਤੀ css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool ਬੱਚਾ_has_ਉਚਾਈ = false;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਖੰਡ->type == BOX_ਖੰਡ ||
			ਖੰਡ->type == BOX_FLEX ||
			ਖੰਡ->type == BOX_INLINE_FLEX ||
			ਖੰਡ->type == BOX_INLINE_ਖੰਡ ||
			ਖੰਡ->type == BOX_ਸਾਰਣੀ_CELL);

	/* check if the widths have already been calculated */
	ਜੇ (ਖੰਡ->max_ਚੌੜਾਈ != UNKNOWN_MAX_ਚੌੜਾਈ)
		ਵਾਪਸ;

	ਜੇ (ਖੰਡ->style != ਨਿਰਾ) {
		wtype = css_computed_ਚੌੜਾਈ(ਖੰਡ->style, &ਚੌੜਾਈ, &wunit);
		htype = css_computed_ਉਚਾਈ(ਖੰਡ->style, &ਉਚਾਈ, &hunit);
		bs = css_computed_box_sizing(ਖੰਡ->style);
	}

	/* set whether the minimum width is of any interest for this box */
	ਜੇ (((ਖੰਡ->ਮਾਪਾ && lh__box_is_float_box(ਖੰਡ->ਮਾਪਾ)) ||
			ਖੰਡ->type == BOX_INLINE_ਖੰਡ ||
			ਖੰਡ->type == BOX_INLINE_FLEX) &&
			wtype != CSS_ਚੌੜਾਈ_ਸੈੱਟ) {
		/* box shrinks to fit; need minimum width */
		ਖੰਡ->flags |= NEED_MIN;
	} ਨਹੀਂਤਾਂ ਜੇ (ਖੰਡ->type == BOX_ਸਾਰਣੀ_CELL) {
		/* box shrinks to fit; need minimum width */
		ਖੰਡ->flags |= NEED_MIN;
	} ਨਹੀਂਤਾਂ ਜੇ (ਖੰਡ->ਮਾਪਾ && (ਖੰਡ->ਮਾਪਾ->flags & NEED_MIN) &&
			wtype != CSS_ਚੌੜਾਈ_ਸੈੱਟ) {
		/* box inside shrink-to-fit context; need minimum width */
		ਖੰਡ->flags |= NEED_MIN;
	} ਨਹੀਂਤਾਂ ਜੇ (ਖੰਡ->ਮਾਪਾ && (ਖੰਡ->ਮਾਪਾ->type == BOX_FLEX)) {
		/* box is flex item */
		ਖੰਡ->flags |= NEED_MIN;
	}

	ਜੇ (ਖੰਡ->gadget && (ਖੰਡ->gadget->type == GADGET_TEXTBOX ||
			ਖੰਡ->gadget->type == GADGET_PASSWORD ||
			ਖੰਡ->gadget->type == GADGET_FILE ||
			ਖੰਡ->gadget->type == GADGET_TEXTAREA) &&
			ਖੰਡ->style && wtype == CSS_ਚੌੜਾਈ_AUTO) {
		css_fixed ਮਾਪ = INTTOFIX(10);
		css_unit unit = CSS_UNIT_EM;

		min = max = FIXTOINT(css_unit_len2device_px(ਖੰਡ->style,
				&content->unit_len_ctx, ਮਾਪ, unit));

		ਖੰਡ->flags |= HAS_ਉਚਾਈ;
	}

	ਜੇ (ਖੰਡ->gadget && (ਖੰਡ->gadget->type == GADGET_RADIO ||
			ਖੰਡ->gadget->type == GADGET_CHECKBOX) &&
			ਖੰਡ->style && wtype == CSS_ਚੌੜਾਈ_AUTO) {
		css_fixed ਮਾਪ = INTTOFIX(1);
		css_unit unit = CSS_UNIT_EM;

		/* form checkbox or radio button
		 * if width is AUTO, set it to 1em */
		min = max = FIXTOINT(css_unit_len2device_px(ਖੰਡ->style,
				&content->unit_len_ctx, ਮਾਪ, unit));

		ਖੰਡ->flags |= HAS_ਉਚਾਈ;
	}

	ਜੇ (ਖੰਡ->object) {
		ਜੇ (content_get_type(ਖੰਡ->object) == CONTENT_HTML) {
			layout_minmax_ਖੰਡ(html_get_box_ਰੁੱਖ(ਖੰਡ->object),
					font_func, content);
			min = html_get_box_ਰੁੱਖ(ਖੰਡ->object)->min_ਚੌੜਾਈ;
			max = html_get_box_ਰੁੱਖ(ਖੰਡ->object)->max_ਚੌੜਾਈ;
		} ਨਹੀਂਤਾਂ {
			min = max = content_get_ਚੌੜਾਈ(ਖੰਡ->object);
		}

		ਖੰਡ->flags |= HAS_ਉਚਾਈ;
	} ਨਹੀਂਤਾਂ ਜੇ (ਖੰਡ->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		ਖੰਡ->flags |= HAS_ਉਚਾਈ;
	} ਨਹੀਂਤਾਂ {
		/* recurse through children */
		ਲਈ (ਬੱਚਾ = ਖੰਡ->children; ਬੱਚਾ; ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ) {
			ਚੋਣ (ਬੱਚਾ->type) {
			ਹਾਲਤ BOX_FLEX:
			ਹਾਲਤ BOX_ਖੰਡ:
				layout_minmax_ਖੰਡ(ਬੱਚਾ, font_func,
						content);
				ਜੇ (ਬੱਚਾ->flags & HAS_ਉਚਾਈ)
					ਬੱਚਾ_has_ਉਚਾਈ = true;
				ਰੋਕੋ;
			ਹਾਲਤ BOX_INLINE_CONTAINER:
				ਜੇ (ਖੰਡ->flags & NEED_MIN)
					ਬੱਚਾ->flags |= NEED_MIN;

				layout_minmax_inline_container(ਬੱਚਾ,
						&ਬੱਚਾ_has_ਉਚਾਈ, font_func,
						content);
				ਜੇ (ਬੱਚਾ_has_ਉਚਾਈ &&
						ਬੱਚਾ ==
						ਬੱਚਾ->ਮਾਪਾ->children) {
					ਖੰਡ->flags |= MAKE_ਉਚਾਈ;
				}
				ਰੋਕੋ;
			ਹਾਲਤ BOX_ਸਾਰਣੀ:
				layout_minmax_ਸਾਰਣੀ(ਬੱਚਾ, font_func,
						content);
				/* todo: fix for zero height tables */
				ਬੱਚਾ_has_ਉਚਾਈ = true;
				ਬੱਚਾ->flags |= MAKE_ਉਚਾਈ;
				ਰੋਕੋ;
			ਮੂਲ:
				ਪ੍ਰਤਿਜ੍ਞਾ(0);
			}
			ਪ੍ਰਤਿਜ੍ਞਾ(ਬੱਚਾ->max_ਚੌੜਾਈ != UNKNOWN_MAX_ਚੌੜਾਈ);

			ਜੇ (ਬੱਚਾ->style &&
					(css_computed_position(ਬੱਚਾ->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(ਬੱਚਾ->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				ਜਾਰੀ;
			}

			ਜੇ (lh__box_is_flex_container(ਖੰਡ) &&
			    lh__flex_main_is_horizontal(ਖੰਡ)) {
				ਜੇ (ਖੰਡ->style != ਨਿਰਾ &&
				    css_computed_flex_wrap(ਖੰਡ->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += ਬੱਚਾ->min_ਚੌੜਾਈ;
				} ਨਹੀਂਤਾਂ {
					ਜੇ (min < ਬੱਚਾ->min_ਚੌੜਾਈ)
						min = ਬੱਚਾ->min_ਚੌੜਾਈ;
				}
				max += ਬੱਚਾ->max_ਚੌੜਾਈ;

			} ਨਹੀਂਤਾਂ {
				ਜੇ (min < ਬੱਚਾ->min_ਚੌੜਾਈ)
					min = ਬੱਚਾ->min_ਚੌੜਾਈ;
				ਜੇ (max < ਬੱਚਾ->max_ਚੌੜਾਈ)
					max = ਬੱਚਾ->max_ਚੌੜਾਈ;
			}

			ਜੇ (ਬੱਚਾ_has_ਉਚਾਈ)
				ਖੰਡ->flags |= HAS_ਉਚਾਈ;
		}
	}

	ਜੇ (max < min) {
		box_dump(stderr, ਖੰਡ, 0, true);
		ਪ੍ਰਤਿਜ੍ਞਾ(0);
	}

	/* fixed width takes priority */
	ਜੇ (ਖੰਡ->type != BOX_ਸਾਰਣੀ_CELL && !lh_box_is_flex_ਵਸਤੂ(ਖੰਡ)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		ਗਿਣਤੀ css_max_ਚੌੜਾਈ_e max_type;
		ਗਿਣਤੀ css_min_ਚੌੜਾਈ_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed ਮੁੱਲ = 0;
		ਪੂਰਨ ਚੌੜਾਈ;

		ਜੇ (css_computed_ਚੌੜਾਈ_px(ਖੰਡ->style, &content->unit_len_ctx,
				-1, &ਚੌੜਾਈ) == CSS_ਚੌੜਾਈ_ਸੈੱਟ) {
			min = max = ਚੌੜਾਈ;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_ਚੌੜਾਈ(ਖੰਡ->style, &ਮੁੱਲ, &unit);
		ਜੇ (min_type == CSS_MIN_ਚੌੜਾਈ_ਸੈੱਟ && unit != CSS_UNIT_PCT) {
			ਪੂਰਨ val = FIXTOINT(css_unit_len2device_px(ਖੰਡ->style,
					&content->unit_len_ctx, ਮੁੱਲ, unit));

			ਜੇ (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_ਚੌੜਾਈ(ਖੰਡ->style, &ਮੁੱਲ, &unit);
		ਜੇ (max_type == CSS_MAX_ਚੌੜਾਈ_ਸੈੱਟ && unit != CSS_UNIT_PCT) {
			ਪੂਰਨ val = FIXTOINT(css_unit_len2device_px(ਖੰਡ->style,
					&content->unit_len_ctx, ਮੁੱਲ, unit));

			ਜੇ (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	ਜੇ (htype == CSS_ਉਚਾਈ_ਸੈੱਟ && hunit != CSS_UNIT_PCT &&
			ਉਚਾਈ > INTTOFIX(0)) {
		ਖੰਡ->flags |= MAKE_ਉਚਾਈ;
		ਖੰਡ->flags |= HAS_ਉਚਾਈ;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx, ਖੰਡ->style, ਖੱਬਾ_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx, ਖੰਡ->style, ਸੱਜਾ_2,
			false, true, true, &extra_fixed, &extra_frac);

	ਜੇ (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	ਜੇ (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	ਜੇ (max < min) {
		min = max;
	}

	calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx, ਖੰਡ->style, ਖੱਬਾ_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_ਚੌੜਾਈ(&content->unit_len_ctx, ਖੰਡ->style, ਸੱਜਾ_2,
			true, false, false, &extra_fixed, &extra_frac);

	ਜੇ (extra_fixed < 0)
		extra_fixed = 0;
	ਜੇ (extra_frac < 0)
		extra_frac = 0;
	ਜੇ (1.0 <= extra_frac)
		extra_frac = 0.9;
	ਜੇ (ਖੰਡ->style != ਨਿਰਾ &&
			(css_computed_float(ਖੰਡ->style) == CSS_FLOAT_ਖੱਬਾ ||
			css_computed_float(ਖੰਡ->style) == CSS_FLOAT_ਸੱਜਾ)) {
		/* floated boxs */
		ਖੰਡ->min_ਚੌੜਾਈ = min + extra_fixed;
		ਖੰਡ->max_ਚੌੜਾਈ = max + extra_fixed;
	} ਨਹੀਂਤਾਂ {
		/* not floated */
		ਖੰਡ->min_ਚੌੜਾਈ = (min + extra_fixed) / (1.0 - extra_frac);
		ਖੰਡ->max_ਚੌੜਾਈ = (max + extra_fixed) / (1.0 - extra_frac);
	}

	ਪ੍ਰਤਿਜ੍ਞਾ(0 <= ਖੰਡ->min_ਚੌੜਾਈ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਖੰਡ->min_ਚੌੜਾਈ <= ਖੰਡ->max_ਚੌੜਾਈ);
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
ਸਥਿਰ ਬਣਤਰ box*
layout_ਅਗਲਾ_margin_ਖੰਡ(ਅਚਲ css_unit_ctx *unit_len_ctx,
			 ਬਣਤਰ box *box,
			 ਬਣਤਰ box *ਖੰਡ,
			 ਪੂਰਨ viewport_ਉਚਾਈ,
			 ਪੂਰਨ *max_pos_margin,
			 ਪੂਰਨ *max_neg_margin)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(ਖੰਡ != ਨਿਰਾ);

	ਜਦੋਂ (box != ਨਿਰਾ) {

		ਜੇ (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			ਜੇ (box->style) {
				layout_ਲੱਭੋ_dimensions(unit_len_ctx,
						box->ਮਾਪਾ->ਚੌੜਾਈ,
						viewport_ਉਚਾਈ, box,
						box->style,
						ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ,
						ਨਿਰਾ, ਨਿਰਾ, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				ਜੇ (*max_pos_margin < box->margin[ਸਿਖਰ_2])
					*max_pos_margin = box->margin[ਸਿਖਰ_2];
				ਨਹੀਂਤਾਂ ਜੇ (*max_neg_margin < -box->margin[ਸਿਖਰ_2])
					*max_neg_margin = -box->margin[ਸਿਖਰ_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			ਜੇ (box->flags & MAKE_ਉਚਾਈ ||
					box->border[ਸਿਖਰ_2].ਚੌੜਾਈ ||
					box->padding[ਸਿਖਰ_2] ||
					(box->style &&
					css_computed_ਉਛਾਲ_y(box->style) !=
					CSS_ਉਛਾਲ_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_ਪਹਿਲਾ_ਬੱਚਾ(box))) {
				/* Collapse to this box; return it */
				ਵਾਪਸ box;
			}
		}


		/* Find next box */
		ਜੇ (box->type == BOX_ਖੰਡ && !box->object && box->children &&
				box->style &&
				css_computed_ਉਛਾਲ_y(box->style) ==
				CSS_ਉਛਾਲ_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} ਨਹੀਂਤਾਂ {
			ਜੇ (!box->ਅਗਲਾ) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				ਕਰੋ {
					/* Apply bottom margin */
					ਜੇ (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					ਨਹੀਂਤਾਂ ਜੇ (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->ਮਾਪਾ;
				} ਜਦੋਂ (box != ਖੰਡ && !box->ਅਗਲਾ);

				ਜੇ (box == ਖੰਡ) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					ਵਾਪਸ ਖੰਡ;
				}
			}

			/* Apply bottom margin */
			ਜੇ (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			ਨਹੀਂਤਾਂ ਜੇ (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->ਅਗਲਾ;

			/* Get margins */
			ਜੇ (box->style) {
				layout_ਲੱਭੋ_dimensions(unit_len_ctx,
						box->ਮਾਪਾ->ਚੌੜਾਈ,
						viewport_ਉਚਾਈ, box,
						box->style,
						ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ,
						ਨਿਰਾ, ਨਿਰਾ, box->margin,
						box->padding, box->border);
			}
		}
	}

	ਵਾਪਸ ਨਿਰਾ;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
ਸਥਿਰ ਪੂਰਨ layout_clear(ਬਣਤਰ box *fl, ਗਿਣਤੀ css_clear_e clear)
{
	ਪੂਰਨ y = 0;
	ਲਈ (; fl; fl = fl->ਅਗਲਾ_float) {
		ਜੇ ((clear == CSS_CLEAR_ਖੱਬਾ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ਖੱਬਾ)
			ਜੇ (y < fl->y + fl->ਉਚਾਈ)
				y = fl->y + fl->ਉਚਾਈ;
		ਜੇ ((clear == CSS_CLEAR_ਸੱਜਾ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ਸੱਜਾ)
			ਜੇ (y < fl->y + fl->ਉਚਾਈ)
				y = fl->y + fl->ਉਚਾਈ;
	}
	ਵਾਪਸ y;
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
ਸਥਿਰ ਖਾਲੀ
ਲੱਭੋ_sides(ਬਣਤਰ box *fl,
	   ਪੂਰਨ y0, ਪੂਰਨ y1,
	   ਪੂਰਨ *x0, ਪੂਰਨ *x1,
	   ਬਣਤਰ box **ਖੱਬਾ,
	   ਬਣਤਰ box **ਸੱਜਾ)
{
	ਪੂਰਨ fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*ਖੱਬਾ = *ਸੱਜਾ = 0;
	ਲਈ (; fl; fl = fl->ਅਗਲਾ_float) {
		fy1 = fl->y + fl->ਉਚਾਈ;
		ਜੇ (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			ਵਾਪਸ;
		}
		fy0 = fl->y;
		ਜੇ (y0 < fy1 && fy0 <= y1) {
			ਜੇ (fl->type == BOX_FLOAT_ਖੱਬਾ) {
				fx1 = fl->x + fl->ਚੌੜਾਈ;
				ਜੇ (*x0 < fx1) {
					*x0 = fx1;
					*ਖੱਬਾ = fl;
				}
			} ਨਹੀਂਤਾਂ {
				fx0 = fl->x;
				ਜੇ (fx0 < *x1) {
					*x1 = fx0;
					*ਸੱਜਾ = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *ਖੱਬਾ, *ਸੱਜਾ);
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
ਸਥਿਰ ਪੂਰਨ
layout_solve_ਚੌੜਾਈ(ਬਣਤਰ box *box,
		   ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ,
		   ਪੂਰਨ ਚੌੜਾਈ,
		   ਪੂਰਨ lm,
		   ਪੂਰਨ rm,
		   ਪੂਰਨ max_ਚੌੜਾਈ,
		   ਪੂਰਨ min_ਚੌੜਾਈ)
{
	bool auto_ਚੌੜਾਈ = false;

	/* Increase specified left/right margins */
	ਜੇ (box->margin[ਖੱਬਾ_2] != AUTO && box->margin[ਖੱਬਾ_2] < lm &&
			box->margin[ਖੱਬਾ_2] >= 0)
		box->margin[ਖੱਬਾ_2] = lm;
	ਜੇ (box->margin[ਸੱਜਾ_2] != AUTO && box->margin[ਸੱਜਾ_2] < rm &&
			box->margin[ਸੱਜਾ_2] >= 0)
		box->margin[ਸੱਜਾ_2] = rm;

	/* Find width */
	ਜੇ (ਚੌੜਾਈ == AUTO) {
		ਪੂਰਨ margin_ਖੱਬਾ = box->margin[ਖੱਬਾ_2];
		ਪੂਰਨ margin_ਸੱਜਾ = box->margin[ਸੱਜਾ_2];

		ਜੇ (margin_ਖੱਬਾ == AUTO) {
			margin_ਖੱਬਾ = lm;
		}
		ਜੇ (margin_ਸੱਜਾ == AUTO) {
			margin_ਸੱਜਾ = rm;
		}

		ਚੌੜਾਈ = ਉਪਲਬਧ_ਚੌੜਾਈ -
				(margin_ਖੱਬਾ + box->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
				box->padding[ਖੱਬਾ_2] + box->padding[ਸੱਜਾ_2] +
				box->border[ਸੱਜਾ_2].ਚੌੜਾਈ + margin_ਸੱਜਾ);
		ਚੌੜਾਈ = ਚੌੜਾਈ < 0 ? 0 : ਚੌੜਾਈ;
		auto_ਚੌੜਾਈ = true;
	}

	ਜੇ (max_ਚੌੜਾਈ >= 0 && ਚੌੜਾਈ > max_ਚੌੜਾਈ) {
		/* max-width is admissable and width exceeds max-width */
		ਚੌੜਾਈ = max_ਚੌੜਾਈ;
		auto_ਚੌੜਾਈ = false;
	}

	ਜੇ (min_ਚੌੜਾਈ > 0 && ਚੌੜਾਈ < min_ਚੌੜਾਈ) {
		/* min-width is admissable and width is less than max-width */
		ਚੌੜਾਈ = min_ਚੌੜਾਈ;
		auto_ਚੌੜਾਈ = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	ਜੇ (auto_ਚੌੜਾਈ) {
		/* any other 'auto' become 0 or the minimum required values */
		ਜੇ (box->margin[ਖੱਬਾ_2] == AUTO) {
			box->margin[ਖੱਬਾ_2] = lm;
		}
		ਜੇ (box->margin[ਸੱਜਾ_2] == AUTO) {
			box->margin[ਸੱਜਾ_2] = rm;
		}
		ਵਾਪਸ ਚੌੜਾਈ;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	ਜੇ (box->margin[ਖੱਬਾ_2] != AUTO && box->margin[ਸੱਜਾ_2] != AUTO &&
			box->ਮਾਪਾ != ਨਿਰਾ && box->ਮਾਪਾ->style != ਨਿਰਾ) {
		ਚੋਣ (css_computed_ਪਾਠ_align(box->ਮਾਪਾ->style)) {
		ਹਾਲਤ CSS_ਪਾਠ_ALIGN_LIBCSS_ਸੱਜਾ:
			box->margin[ਖੱਬਾ_2] = AUTO;
			box->margin[ਸੱਜਾ_2] = 0;
			ਰੋਕੋ;
		ਹਾਲਤ CSS_ਪਾਠ_ALIGN_LIBCSS_CENTER:
			box->margin[ਖੱਬਾ_2] = box->margin[ਸੱਜਾ_2] = AUTO;
			ਰੋਕੋ;
		ਹਾਲਤ CSS_ਪਾਠ_ALIGN_LIBCSS_ਖੱਬਾ:
			box->margin[ਖੱਬਾ_2] = 0;
			box->margin[ਸੱਜਾ_2] = AUTO;
			ਰੋਕੋ;
		ਮੂਲ:
			/* Leave it alone; no HTML alignment */
			ਰੋਕੋ;
		}
	}

	ਜੇ (box->margin[ਖੱਬਾ_2] == AUTO && box->margin[ਸੱਜਾ_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[ਖੱਬਾ_2] = box->margin[ਸੱਜਾ_2] =
				(ਉਪਲਬਧ_ਚੌੜਾਈ - lm - rm -
				(box->border[ਖੱਬਾ_2].ਚੌੜਾਈ + box->padding[ਖੱਬਾ_2] +
				ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2] +
				box->border[ਸੱਜਾ_2].ਚੌੜਾਈ)) / 2;

		ਜੇ (box->margin[ਖੱਬਾ_2] < 0) {
			box->margin[ਸੱਜਾ_2] += box->margin[ਖੱਬਾ_2];
			box->margin[ਖੱਬਾ_2] = 0;
		}

		box->margin[ਖੱਬਾ_2] += lm;

	} ਨਹੀਂਤਾਂ ਜੇ (box->margin[ਖੱਬਾ_2] == AUTO) {
		box->margin[ਖੱਬਾ_2] = ਉਪਲਬਧ_ਚੌੜਾਈ - lm -
				(box->border[ਖੱਬਾ_2].ਚੌੜਾਈ + box->padding[ਖੱਬਾ_2] +
				ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2] +
				box->border[ਸੱਜਾ_2].ਚੌੜਾਈ + box->margin[ਸੱਜਾ_2]);
		box->margin[ਖੱਬਾ_2] = box->margin[ਖੱਬਾ_2] < lm
				? lm : box->margin[ਖੱਬਾ_2];
	} ਨਹੀਂਤਾਂ {
		/* margin-right auto or "over-constrained" */
		box->margin[ਸੱਜਾ_2] = ਉਪਲਬਧ_ਚੌੜਾਈ - rm -
				(box->margin[ਖੱਬਾ_2] + box->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
				 box->padding[ਖੱਬਾ_2] + ਚੌੜਾਈ +
				 box->padding[ਸੱਜਾ_2] +
				 box->border[ਸੱਜਾ_2].ਚੌੜਾਈ);
	}

	ਵਾਪਸ ਚੌੜਾਈ;
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
ਸਥਿਰ ਖਾਲੀ
layout_ਖੰਡ_ਲੱਭੋ_dimensions(ਅਚਲ css_unit_ctx *unit_len_ctx,
			     ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ,
			     ਪੂਰਨ viewport_ਉਚਾਈ,
			     ਪੂਰਨ lm,
			     ਪੂਰਨ rm,
			     ਬਣਤਰ box *box)
{
	ਪੂਰਨ ਚੌੜਾਈ, max_ਚੌੜਾਈ, min_ਚੌੜਾਈ;
	ਪੂਰਨ ਉਚਾਈ, max_ਉਚਾਈ, min_ਉਚਾਈ;
	ਪੂਰਨ *margin = box->margin;
	ਪੂਰਨ *padding = box->padding;
	ਬਣਤਰ box_border *border = box->border;
	ਅਚਲ css_computed_style *style = box->style;

	layout_ਲੱਭੋ_dimensions(unit_len_ctx, ਉਪਲਬਧ_ਚੌੜਾਈ, viewport_ਉਚਾਈ, box,
			style, &ਚੌੜਾਈ, &ਉਚਾਈ, &max_ਚੌੜਾਈ, &min_ਚੌੜਾਈ,
			&max_ਉਚਾਈ, &min_ਉਚਾਈ, margin, padding, border);

	ਜੇ (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &ਚੌੜਾਈ, &ਉਚਾਈ,
				min_ਚੌੜਾਈ, max_ਚੌੜਾਈ, min_ਉਚਾਈ, max_ਉਚਾਈ);
	}

	box->ਚੌੜਾਈ = layout_solve_ਚੌੜਾਈ(box, ਉਪਲਬਧ_ਚੌੜਾਈ, ਚੌੜਾਈ, lm, rm,
			max_ਚੌੜਾਈ, min_ਚੌੜਾਈ);
	box->ਉਚਾਈ = ਉਚਾਈ;

	ਜੇ (margin[ਸਿਖਰ_2] == AUTO)
		margin[ਸਿਖਰ_2] = 0;
	ਜੇ (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
ਸਥਿਰ ਖਾਲੀ layout_ਖੰਡ_ਜੋੜੋ_scrollbar(ਬਣਤਰ box *box, ਪੂਰਨ which)
{
	ਗਿਣਤੀ css_ਉਛਾਲ_e ਉਛਾਲ_x, ਉਛਾਲ_y;

	ਪ੍ਰਤਿਜ੍ਞਾ(box->type == BOX_ਖੰਡ && (which == ਸੱਜਾ_2 || which == BOTTOM));

	ਜੇ (box->style == ਨਿਰਾ)
		ਵਾਪਸ;

	ਉਛਾਲ_x = css_computed_ਉਛਾਲ_x(box->style);
	ਉਛਾਲ_y = css_computed_ਉਛਾਲ_y(box->style);

	ਜੇ (which == BOTTOM &&
			(ਉਛਾਲ_x == CSS_ਉਛਾਲ_SCROLL ||
			 ਉਛਾਲ_x == CSS_ਉਛਾਲ_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		ਜੇ (box->ਉਚਾਈ != AUTO &&
				(ਉਛਾਲ_x == CSS_ਉਛਾਲ_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_ਚੌੜਾਈ;
		}

	} ਨਹੀਂਤਾਂ ਜੇ (which == ਸੱਜਾ_2 &&
			(ਉਛਾਲ_y == CSS_ਉਛਾਲ_SCROLL ||
			 ਉਛਾਲ_y == CSS_ਉਛਾਲ_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		ਗਿਣਤੀ css_ਉਚਾਈ_e htype;
		css_fixed ਉਚਾਈ = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_ਉਚਾਈ(box->style, &ਉਚਾਈ, &hunit);

		ਜੇ (which == ਸੱਜਾ_2 && box->ਚੌੜਾਈ != AUTO &&
				htype == CSS_ਉਚਾਈ_ਸੈੱਟ &&
				(ਉਛਾਲ_y == CSS_ਉਛਾਲ_SCROLL ||
				box_vscrollbar_present(box))) {
			box->ਚੌੜਾਈ -= SCROLLBAR_ਚੌੜਾਈ;
			box->padding[ਸੱਜਾ_2] += SCROLLBAR_ਚੌੜਾਈ;
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
ਸਥਿਰ ਖਾਲੀ layout_move_children(ਬਣਤਰ box *box, ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(box);

	ਲਈ (box = box->children; box; box = box->ਅਗਲਾ) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_ਸਾਰਣੀ(
		ਬਣਤਰ box *ਸਾਰਣੀ,
		ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ,
		html_content *content)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ columns = ਸਾਰਣੀ->columns;  /* total columns */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *ਪੰਕਤੀ_ਵਿਸਤਾਰ;
	ਪੂਰਨ *excess_y;
	ਪੂਰਨ ਸਾਰਣੀ_ਚੌੜਾਈ, min_ਚੌੜਾਈ = 0, max_ਚੌੜਾਈ = 0;
	ਪੂਰਨ required_ਚੌੜਾਈ = 0;
	ਪੂਰਨ x, remainder = 0, ਗਿਣਤੀ_2 = 0;
	ਪੂਰਨ ਸਾਰਣੀ_ਉਚਾਈ = 0;
	ਪੂਰਨ min_ਉਚਾਈ = 0;
	ਪੂਰਨ *xs;  /* array of column x positions */
	ਪੂਰਨ auto_ਚੌੜਾਈ;
	ਪੂਰਨ ਵਾਧੂ_ਚੌੜਾਈ;
	ਪੂਰਨ relative_ਜੋੜ = 0;
	ਪੂਰਨ border_spacing_h = 0, border_spacing_v = 0;
	ਪੂਰਨ ਵਾਧੂ_ਉਚਾਈ;
	ਪੂਰਨ positioned_columns = 0;
	ਬਣਤਰ box *containing_ਖੰਡ = ਨਿਰਾ;
	ਬਣਤਰ box *c;
	ਬਣਤਰ box *ਪੰਕਤੀ;
	ਬਣਤਰ box *ਪੰਕਤੀ_ਸਮੂਹ;
	ਬਣਤਰ box **ਪੰਕਤੀ_ਵਿਸਤਾਰ_cell;
	ਬਣਤਰ ਕਾਲਮ *ਖਾਨਾ;
	ਅਚਲ css_computed_style *style = ਸਾਰਣੀ->style;
	ਗਿਣਤੀ css_ਉਚਾਈ_e htype;
	css_fixed ਮੁੱਲ = 0;
	css_unit unit = CSS_UNIT_PX;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਸਾਰਣੀ->type == BOX_ਸਾਰਣੀ);
	ਪ੍ਰਤਿਜ੍ਞਾ(style);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਸਾਰਣੀ->children && ਸਾਰਣੀ->children->children);
	ਪ੍ਰਤਿਜ੍ਞਾ(columns);

	/* allocate working buffers */
	ਖਾਨਾ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(columns * ਆਕਾਰ ਖਾਨਾ[0]);
	excess_y = ਸਮ੍ਰਿਤੀ_ਦਿਓ(columns * ਆਕਾਰ excess_y[0]);
	ਪੰਕਤੀ_ਵਿਸਤਾਰ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(columns * ਆਕਾਰ ਪੰਕਤੀ_ਵਿਸਤਾਰ[0]);
	ਪੰਕਤੀ_ਵਿਸਤਾਰ_cell = ਸਮ੍ਰਿਤੀ_ਦਿਓ(columns * ਆਕਾਰ ਪੰਕਤੀ_ਵਿਸਤਾਰ_cell[0]);
	xs = ਸਮ੍ਰਿਤੀ_ਦਿਓ((columns + 1) * ਆਕਾਰ xs[0]);
	ਜੇ (!ਖਾਨਾ || !xs || !ਪੰਕਤੀ_ਵਿਸਤਾਰ || !excess_y || !ਪੰਕਤੀ_ਵਿਸਤਾਰ_cell) {
		ਮੁਕਤ(ਖਾਨਾ);
		ਮੁਕਤ(excess_y);
		ਮੁਕਤ(ਪੰਕਤੀ_ਵਿਸਤਾਰ);
		ਮੁਕਤ(ਪੰਕਤੀ_ਵਿਸਤਾਰ_cell);
		ਮੁਕਤ(xs);
		ਵਾਪਸ false;
	}

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਖਾਨਾ, ਸਾਰਣੀ->ਖਾਨਾ, ਆਕਾਰ(ਖਾਨਾ[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_ਲੱਭੋ_dimensions(&content->unit_len_ctx, ਉਪਲਬਧ_ਚੌੜਾਈ, -1, ਸਾਰਣੀ,
			style, 0, 0, 0, 0, 0, 0, ਸਾਰਣੀ->margin, ਸਾਰਣੀ->padding,
			ਸਾਰਣੀ->border);
	ਲਈ (ਪੰਕਤੀ_ਸਮੂਹ = ਸਾਰਣੀ->children; ਪੰਕਤੀ_ਸਮੂਹ;
			ਪੰਕਤੀ_ਸਮੂਹ = ਪੰਕਤੀ_ਸਮੂਹ->ਅਗਲਾ) {
		ਲਈ (ਪੰਕਤੀ = ਪੰਕਤੀ_ਸਮੂਹ->children; ਪੰਕਤੀ; ਪੰਕਤੀ = ਪੰਕਤੀ->ਅਗਲਾ) {
			ਲਈ (c = ਪੰਕਤੀ->children; c; c = c->ਅਗਲਾ) {
				ਗਿਣਤੀ css_ਉਛਾਲ_e ਉਛਾਲ_x;
				ਗਿਣਤੀ css_ਉਛਾਲ_e ਉਛਾਲ_y;

				ਪ੍ਰਤਿਜ੍ਞਾ(c->style);
				ਸਾਰਣੀ_ਵਰਤਿਆ_border_for_cell(
						&content->unit_len_ctx, c);
				layout_ਲੱਭੋ_dimensions(&content->unit_len_ctx,
						ਉਪਲਬਧ_ਚੌੜਾਈ, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				ਉਛਾਲ_x = css_computed_ਉਛਾਲ_x(c->style);
				ਉਛਾਲ_y = css_computed_ਉਛਾਲ_y(c->style);

				ਜੇ (ਉਛਾਲ_x == CSS_ਉਛਾਲ_SCROLL ||
						ਉਛਾਲ_x ==
						CSS_ਉਛਾਲ_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_ਚੌੜਾਈ;
				}
				ਜੇ (ਉਛਾਲ_y == CSS_ਉਛਾਲ_SCROLL ||
						ਉਛਾਲ_y ==
						CSS_ਉਛਾਲ_AUTO) {
					c->padding[ਸੱਜਾ_2] += SCROLLBAR_ਚੌੜਾਈ;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	ਜੇ (css_computed_border_collapse(style) ==
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
	ਜੇ (css_computed_ਚੌੜਾਈ_px(style, &content->unit_len_ctx,
			ਉਪਲਬਧ_ਚੌੜਾਈ, &ਸਾਰਣੀ_ਚੌੜਾਈ) == CSS_ਚੌੜਾਈ_ਸੈੱਟ) {
		/* specified width includes border */
		ਸਾਰਣੀ_ਚੌੜਾਈ -= ਸਾਰਣੀ->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
				ਸਾਰਣੀ->border[ਸੱਜਾ_2].ਚੌੜਾਈ;
		ਸਾਰਣੀ_ਚੌੜਾਈ = ਸਾਰਣੀ_ਚੌੜਾਈ < 0 ? 0 : ਸਾਰਣੀ_ਚੌੜਾਈ;

		auto_ਚੌੜਾਈ = ਸਾਰਣੀ_ਚੌੜਾਈ;
	} ਨਹੀਂਤਾਂ {
		ਸਾਰਣੀ_ਚੌੜਾਈ = AUTO;
		auto_ਚੌੜਾਈ = ਉਪਲਬਧ_ਚੌੜਾਈ -
				((ਸਾਰਣੀ->margin[ਖੱਬਾ_2] == AUTO ? 0 :
						ਸਾਰਣੀ->margin[ਖੱਬਾ_2]) +
				 ਸਾਰਣੀ->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
				 ਸਾਰਣੀ->padding[ਖੱਬਾ_2] +
				 ਸਾਰਣੀ->padding[ਸੱਜਾ_2] +
				 ਸਾਰਣੀ->border[ਸੱਜਾ_2].ਚੌੜਾਈ +
				 (ਸਾਰਣੀ->margin[ਸੱਜਾ_2] == AUTO ? 0 :
						ਸਾਰਣੀ->margin[ਸੱਜਾ_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_ਉਚਾਈ(style, &ਮੁੱਲ, &unit);
	ਜੇ (htype == CSS_ਉਚਾਈ_ਸੈੱਟ) {
		ਜੇ (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			ਜੇ (css_computed_position(ਸਾਰਣੀ->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				ਪ੍ਰਤਿਜ੍ਞਾ(ਸਾਰਣੀ->float_container);
				containing_ਖੰਡ = ਸਾਰਣੀ->float_container;
			} ਨਹੀਂਤਾਂ ਜੇ (ਸਾਰਣੀ->float_container &&
					css_computed_position(ਸਾਰਣੀ->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(ਸਾਰਣੀ->style) ==
						CSS_FLOAT_ਖੱਬਾ ||
					css_computed_float(ਸਾਰਣੀ->style) ==
						CSS_FLOAT_ਸੱਜਾ)) {
				/* Table is a float */
				ਪ੍ਰਤਿਜ੍ਞਾ(ਸਾਰਣੀ->ਮਾਪਾ && ਸਾਰਣੀ->ਮਾਪਾ->ਮਾਪਾ &&
						ਸਾਰਣੀ->ਮਾਪਾ->ਮਾਪਾ->ਮਾਪਾ);
				containing_ਖੰਡ =
						ਸਾਰਣੀ->ਮਾਪਾ->ਮਾਪਾ->ਮਾਪਾ;
			} ਨਹੀਂਤਾਂ ਜੇ (ਸਾਰਣੀ->ਮਾਪਾ && ਸਾਰਣੀ->ਮਾਪਾ->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_ਖੰਡ = ਸਾਰਣੀ->ਮਾਪਾ;
			} ਨਹੀਂਤਾਂ ਜੇ (ਸਾਰਣੀ->ਮਾਪਾ && ਸਾਰਣੀ->ਮਾਪਾ->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				ਪ੍ਰਤਿਜ੍ਞਾ(ਸਾਰਣੀ->ਮਾਪਾ->ਮਾਪਾ);
				containing_ਖੰਡ = ਸਾਰਣੀ->ਮਾਪਾ->ਮਾਪਾ;
			}

			ਜੇ (containing_ਖੰਡ) {
				css_fixed ignored = 0;

				htype = css_computed_ਉਚਾਈ(
						containing_ਖੰਡ->style,
						&ignored, &unit);
			}

			ਜੇ (containing_ਖੰਡ &&
					containing_ਖੰਡ->ਉਚਾਈ != AUTO &&
					(css_computed_position(ਸਾਰਣੀ->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_ਉਚਾਈ_ਸੈੱਟ)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_ਉਚਾਈ = FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ,
						containing_ਖੰਡ->ਉਚਾਈ);
			}
		} ਨਹੀਂਤਾਂ {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_ਉਚਾਈ = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					ਮੁੱਲ, unit));
		}
	}

	/* calculate width required by cells */
	ਲਈ (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      ਸਾਰਣੀ,
		      i,
		      ((ਅਚਲ ਅੱਖਰ *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[ਖਾਨਾ[i].type],
		      ਖਾਨਾ[i].ਚੌੜਾਈ,
		      ਖਾਨਾ[i].min,
		      ਖਾਨਾ[i].max);


		ਜੇ (ਖਾਨਾ[i].positioned) {
			positioned_columns++;
			ਜਾਰੀ;
		} ਨਹੀਂਤਾਂ ਜੇ (ਖਾਨਾ[i].type == ਕਾਲਮ_ਚੌੜਾਈ_FIXED) {
			ਜੇ (ਖਾਨਾ[i].ਚੌੜਾਈ < ਖਾਨਾ[i].min)
				ਖਾਨਾ[i].ਚੌੜਾਈ = ਖਾਨਾ[i].max = ਖਾਨਾ[i].min;
			ਨਹੀਂਤਾਂ
				ਖਾਨਾ[i].min = ਖਾਨਾ[i].max = ਖਾਨਾ[i].ਚੌੜਾਈ;
			required_ਚੌੜਾਈ += ਖਾਨਾ[i].ਚੌੜਾਈ;
		} ਨਹੀਂਤਾਂ ਜੇ (ਖਾਨਾ[i].type == ਕਾਲਮ_ਚੌੜਾਈ_PERCENT) {
			ਪੂਰਨ ਚੌੜਾਈ = ਖਾਨਾ[i].ਚੌੜਾਈ * auto_ਚੌੜਾਈ / 100;
			required_ਚੌੜਾਈ += ਖਾਨਾ[i].min < ਚੌੜਾਈ ? ਚੌੜਾਈ :
					ਖਾਨਾ[i].min;
		} ਨਹੀਂਤਾਂ
			required_ਚੌੜਾਈ += ਖਾਨਾ[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_ਚੌੜਾਈ);
	}
	required_ਚੌੜਾਈ += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", ਸਾਰਣੀ_ਚੌੜਾਈ,
	      ਸਾਰਣੀ->min_ਚੌੜਾਈ, ਸਾਰਣੀ->max_ਚੌੜਾਈ, auto_ਚੌੜਾਈ, required_ਚੌੜਾਈ);

	ਜੇ (auto_ਚੌੜਾਈ < required_ਚੌੜਾਈ) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		ਲਈ (i = 0; i != columns; i++) {
			ਜੇ (ਖਾਨਾ[i].type == ਕਾਲਮ_ਚੌੜਾਈ_RELATIVE)
				ਜਾਰੀ;
			ਜੇ (ਖਾਨਾ[i].type == ਕਾਲਮ_ਚੌੜਾਈ_PERCENT) {
				ਖਾਨਾ[i].max = auto_ਚੌੜਾਈ * ਖਾਨਾ[i].ਚੌੜਾਈ / 100;
				ਜੇ (ਖਾਨਾ[i].max < ਖਾਨਾ[i].min)
					ਖਾਨਾ[i].max = ਖਾਨਾ[i].min;
			}
			min_ਚੌੜਾਈ += ਖਾਨਾ[i].min;
			max_ਚੌੜਾਈ += ਖਾਨਾ[i].max;
		}
	} ਨਹੀਂਤਾਂ {
		/* take percentages exactly */
		ਲਈ (i = 0; i != columns; i++) {
			ਜੇ (ਖਾਨਾ[i].type == ਕਾਲਮ_ਚੌੜਾਈ_RELATIVE)
				ਜਾਰੀ;
			ਜੇ (ਖਾਨਾ[i].type == ਕਾਲਮ_ਚੌੜਾਈ_PERCENT) {
				ਪੂਰਨ ਚੌੜਾਈ = auto_ਚੌੜਾਈ * ਖਾਨਾ[i].ਚੌੜਾਈ / 100;
				ਜੇ (ਚੌੜਾਈ < ਖਾਨਾ[i].min)
					ਚੌੜਾਈ = ਖਾਨਾ[i].min;
				ਖਾਨਾ[i].min = ਖਾਨਾ[i].ਚੌੜਾਈ = ਖਾਨਾ[i].max = ਚੌੜਾਈ;
				ਖਾਨਾ[i].type = ਕਾਲਮ_ਚੌੜਾਈ_FIXED;
			}
			min_ਚੌੜਾਈ += ਖਾਨਾ[i].min;
			max_ਚੌੜਾਈ += ਖਾਨਾ[i].max;
		}
	}

	/* allocate relative widths */
	ਵਾਧੂ_ਚੌੜਾਈ = auto_ਚੌੜਾਈ;
	ਲਈ (i = 0; i != columns; i++) {
		ਜੇ (ਖਾਨਾ[i].type == ਕਾਲਮ_ਚੌੜਾਈ_RELATIVE)
			relative_ਜੋੜ += ਖਾਨਾ[i].ਚੌੜਾਈ;
		ਨਹੀਂਤਾਂ ਜੇ (ਖਾਨਾ[i].type == ਕਾਲਮ_ਚੌੜਾਈ_FIXED)
			ਵਾਧੂ_ਚੌੜਾਈ -= ਖਾਨਾ[i].ਚੌੜਾਈ;
		ਨਹੀਂਤਾਂ
			ਵਾਧੂ_ਚੌੜਾਈ -= ਖਾਨਾ[i].min;
	}
	ਵਾਧੂ_ਚੌੜਾਈ -= (columns + 1) * border_spacing_h;
	ਜੇ (relative_ਜੋੜ != 0) {
		ਜੇ (ਵਾਧੂ_ਚੌੜਾਈ < 0)
			ਵਾਧੂ_ਚੌੜਾਈ = 0;
		ਲਈ (i = 0; i != columns; i++) {
			ਜੇ (ਖਾਨਾ[i].type == ਕਾਲਮ_ਚੌੜਾਈ_RELATIVE) {
				ਖਾਨਾ[i].min = ceil(ਖਾਨਾ[i].max =
						(ਦਸ਼ਮਲਵ) ਵਾਧੂ_ਚੌੜਾਈ
						* (ਦਸ਼ਮਲਵ) ਖਾਨਾ[i].ਚੌੜਾਈ
						/ relative_ਜੋੜ);
				min_ਚੌੜਾਈ += ਖਾਨਾ[i].min;
				max_ਚੌੜਾਈ += ਖਾਨਾ[i].max;
			}
		}
	}
	min_ਚੌੜਾਈ += (columns + 1) * border_spacing_h;
	max_ਚੌੜਾਈ += (columns + 1) * border_spacing_h;

	ਜੇ (auto_ਚੌੜਾਈ <= min_ਚੌੜਾਈ) {
		/* not enough space: minimise column widths */
		ਲਈ (i = 0; i < columns; i++) {
			ਖਾਨਾ[i].ਚੌੜਾਈ = ਖਾਨਾ[i].min;
		}
		ਸਾਰਣੀ_ਚੌੜਾਈ = min_ਚੌੜਾਈ;
	} ਨਹੀਂਤਾਂ ਜੇ (max_ਚੌੜਾਈ <= auto_ਚੌੜਾਈ) {
		/* more space than maximum width */
		ਜੇ (ਸਾਰਣੀ_ਚੌੜਾਈ == AUTO) {
			/* for auto-width tables, make columns max width */
			ਲਈ (i = 0; i < columns; i++) {
				ਖਾਨਾ[i].ਚੌੜਾਈ = ਖਾਨਾ[i].max;
			}
			ਸਾਰਣੀ_ਚੌੜਾਈ = max_ਚੌੜਾਈ;
		} ਨਹੀਂਤਾਂ {
			/* for fixed-width tables, distribute the extra space
			 * too */
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flexible_columns = 0;
			ਲਈ (i = 0; i != columns; i++)
				ਜੇ (ਖਾਨਾ[i].type != ਕਾਲਮ_ਚੌੜਾਈ_FIXED)
					flexible_columns++;
			ਜੇ (flexible_columns == 0) {
				ਪੂਰਨ extra = (ਸਾਰਣੀ_ਚੌੜਾਈ - max_ਚੌੜਾਈ) / columns;
				remainder = (ਸਾਰਣੀ_ਚੌੜਾਈ - max_ਚੌੜਾਈ) -
						(extra * columns);
				ਲਈ (i = 0; i != columns; i++) {
					ਖਾਨਾ[i].ਚੌੜਾਈ = ਖਾਨਾ[i].max + extra;
					ਗਿਣਤੀ_2 -= remainder;
					ਜੇ (ਗਿਣਤੀ_2 < 0) {
						ਖਾਨਾ[i].ਚੌੜਾਈ++;
						ਗਿਣਤੀ_2 += columns;
					}
				}

			} ਨਹੀਂਤਾਂ {
				ਪੂਰਨ extra = (ਸਾਰਣੀ_ਚੌੜਾਈ - max_ਚੌੜਾਈ) /
						flexible_columns;
				remainder = (ਸਾਰਣੀ_ਚੌੜਾਈ - max_ਚੌੜਾਈ) -
						(extra * flexible_columns);
				ਲਈ (i = 0; i != columns; i++)
					ਜੇ (ਖਾਨਾ[i].type != ਕਾਲਮ_ਚੌੜਾਈ_FIXED) {
						ਖਾਨਾ[i].ਚੌੜਾਈ = ਖਾਨਾ[i].max +
								extra;
						ਗਿਣਤੀ_2 -= remainder;
						ਜੇ (ਗਿਣਤੀ_2 < 0) {
							ਖਾਨਾ[i].ਚੌੜਾਈ++;
							ਗਿਣਤੀ_2 += flexible_columns;
						}
					}
			}
		}
	} ਨਹੀਂਤਾਂ {
		/* space between min and max: fill it exactly */
		ਦਸ਼ਮਲਵ scale = (ਦਸ਼ਮਲਵ) (auto_ਚੌੜਾਈ - min_ਚੌੜਾਈ) /
				(ਦਸ਼ਮਲਵ) (max_ਚੌੜਾਈ - min_ਚੌੜਾਈ);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		ਲਈ (i = 0; i < columns; i++) {
			ਖਾਨਾ[i].ਚੌੜਾਈ = ਖਾਨਾ[i].min + (ਪੂਰਨ) (0.5 +
					(ਖਾਨਾ[i].max - ਖਾਨਾ[i].min) * scale);
		}
		ਸਾਰਣੀ_ਚੌੜਾਈ = auto_ਚੌੜਾਈ;
	}

	xs[0] = x = border_spacing_h;
	ਲਈ (i = 0; i != columns; i++) {
		ਜੇ (!ਖਾਨਾ[i].positioned)
			x += ਖਾਨਾ[i].ਚੌੜਾਈ + border_spacing_h;
		xs[i + 1] = x;
		ਪੰਕਤੀ_ਵਿਸਤਾਰ[i] = 0;
		excess_y[i] = 0;
		ਪੰਕਤੀ_ਵਿਸਤਾਰ_cell[i] = 0;
	}

	/* position cells */
	ਸਾਰਣੀ_ਉਚਾਈ = border_spacing_v;
	ਲਈ (ਪੰਕਤੀ_ਸਮੂਹ = ਸਾਰਣੀ->children; ਪੰਕਤੀ_ਸਮੂਹ;
			ਪੰਕਤੀ_ਸਮੂਹ = ਪੰਕਤੀ_ਸਮੂਹ->ਅਗਲਾ) {
		ਪੂਰਨ ਪੰਕਤੀ_ਸਮੂਹ_ਉਚਾਈ = 0;
		ਲਈ (ਪੰਕਤੀ = ਪੰਕਤੀ_ਸਮੂਹ->children; ਪੰਕਤੀ; ਪੰਕਤੀ = ਪੰਕਤੀ->ਅਗਲਾ) {
			ਪੂਰਨ ਪੰਕਤੀ_ਉਚਾਈ = 0;

			htype = css_computed_ਉਚਾਈ(ਪੰਕਤੀ->style, &ਮੁੱਲ, &unit);
			ਜੇ (htype == CSS_ਉਚਾਈ_ਸੈੱਟ && unit != CSS_UNIT_PCT) {
				ਪੰਕਤੀ_ਉਚਾਈ = FIXTOINT(css_unit_len2device_px(
						ਪੰਕਤੀ->style,
						&content->unit_len_ctx,
						ਮੁੱਲ, unit));
			}
			ਲਈ (c = ਪੰਕਤੀ->children; c; c = c->ਅਗਲਾ) {
				ਪ੍ਰਤਿਜ੍ਞਾ(c->style);
				c->ਚੌੜਾਈ = xs[c->ਸ਼ੁਰੂ_ਕਾਲਮ + c->columns] -
						xs[c->ਸ਼ੁਰੂ_ਕਾਲਮ] -
						border_spacing_h -
						c->border[ਖੱਬਾ_2].ਚੌੜਾਈ -
						c->padding[ਖੱਬਾ_2] -
						c->padding[ਸੱਜਾ_2] -
						c->border[ਸੱਜਾ_2].ਚੌੜਾਈ;
				c->float_children = 0;
				c->cached_ਰੱਖੋ_ਹੇਠਾਂ_level = 0;

				c->ਉਚਾਈ = AUTO;
				ਜੇ (!layout_ਖੰਡ_context(c, -1, content)) {
					ਮੁਕਤ(ਖਾਨਾ);
					ਮੁਕਤ(excess_y);
					ਮੁਕਤ(ਪੰਕਤੀ_ਵਿਸਤਾਰ);
					ਮੁਕਤ(ਪੰਕਤੀ_ਵਿਸਤਾਰ_cell);
					ਮੁਕਤ(xs);
					ਵਾਪਸ false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->ਉਚਾਈ;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_ਉਚਾਈ(c->style,
						&ਮੁੱਲ, &unit);

				ਜੇ (htype == CSS_ਉਚਾਈ_ਸੈੱਟ &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					ਪੂਰਨ h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							ਮੁੱਲ, unit));
					ਜੇ (c->ਉਚਾਈ < h)
						c->ਉਚਾਈ = h;
				}
				/* specified row height is treated as a minimum
				 */
				ਜੇ (c->ਉਚਾਈ < ਪੰਕਤੀ_ਉਚਾਈ)
					c->ਉਚਾਈ = ਪੰਕਤੀ_ਉਚਾਈ;
				c->x = xs[c->ਸ਼ੁਰੂ_ਕਾਲਮ] +
						c->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
				c->y = c->border[ਸਿਖਰ_2].ਚੌੜਾਈ;
				ਲਈ (i = 0; i != c->columns; i++) {
					ਪੰਕਤੀ_ਵਿਸਤਾਰ[c->ਸ਼ੁਰੂ_ਕਾਲਮ + i] = c->ਪੰਕਤੀਆਂ;
					excess_y[c->ਸ਼ੁਰੂ_ਕਾਲਮ + i] =
							c->border[ਸਿਖਰ_2].ਚੌੜਾਈ +
							c->padding[ਸਿਖਰ_2] +
							c->ਉਚਾਈ +
							c->padding[BOTTOM] +
							c->border[BOTTOM].ਚੌੜਾਈ;
					ਪੰਕਤੀ_ਵਿਸਤਾਰ_cell[c->ਸ਼ੁਰੂ_ਕਾਲਮ + i] = 0;
				}
				ਪੰਕਤੀ_ਵਿਸਤਾਰ_cell[c->ਸ਼ੁਰੂ_ਕਾਲਮ] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[ਸਿਖਰ_2].ਚੌੜਾਈ -
						c->padding[ਸਿਖਰ_2] -
						c->ਉਚਾਈ -
						c->border[BOTTOM].ਚੌੜਾਈ;
			}
			ਲਈ (i = 0; i != columns; i++)
				ਜੇ (ਪੰਕਤੀ_ਵਿਸਤਾਰ[i] != 0)
					ਪੰਕਤੀ_ਵਿਸਤਾਰ[i]--;
				ਨਹੀਂਤਾਂ
					ਪੰਕਤੀ_ਵਿਸਤਾਰ_cell[i] = 0;
			ਜੇ (ਪੰਕਤੀ->ਅਗਲਾ || ਪੰਕਤੀ_ਸਮੂਹ->ਅਗਲਾ) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				ਲਈ (i = 0; i != columns; i++)
					ਜੇ (ਪੰਕਤੀ_ਵਿਸਤਾਰ[i] == 0 && ਪੰਕਤੀ_ਉਚਾਈ <
							excess_y[i])
						ਪੰਕਤੀ_ਉਚਾਈ = excess_y[i];
			} ਨਹੀਂਤਾਂ {
				/* except in the last row */
				ਲਈ (i = 0; i != columns; i++)
					ਜੇ (ਪੰਕਤੀ_ਉਚਾਈ < excess_y[i])
						ਪੰਕਤੀ_ਉਚਾਈ = excess_y[i];
			}
			ਲਈ (i = 0; i != columns; i++) {
				ਜੇ (ਪੰਕਤੀ_ਉਚਾਈ < excess_y[i])
					excess_y[i] -= ਪੰਕਤੀ_ਉਚਾਈ;
				ਨਹੀਂਤਾਂ
					excess_y[i] = 0;
				ਜੇ (ਪੰਕਤੀ_ਵਿਸਤਾਰ_cell[i] != 0)
					ਪੰਕਤੀ_ਵਿਸਤਾਰ_cell[i]->padding[BOTTOM] +=
							ਪੰਕਤੀ_ਉਚਾਈ +
							border_spacing_v;
			}

			ਪੰਕਤੀ->x = 0;
			ਪੰਕਤੀ->y = ਪੰਕਤੀ_ਸਮੂਹ_ਉਚਾਈ;
			ਪੰਕਤੀ->ਚੌੜਾਈ = ਸਾਰਣੀ_ਚੌੜਾਈ;
			ਪੰਕਤੀ->ਉਚਾਈ = ਪੰਕਤੀ_ਉਚਾਈ;
			ਪੰਕਤੀ_ਸਮੂਹ_ਉਚਾਈ += ਪੰਕਤੀ_ਉਚਾਈ + border_spacing_v;
		}
		ਪੰਕਤੀ_ਸਮੂਹ->x = 0;
		ਪੰਕਤੀ_ਸਮੂਹ->y = ਸਾਰਣੀ_ਉਚਾਈ;
		ਪੰਕਤੀ_ਸਮੂਹ->ਚੌੜਾਈ = ਸਾਰਣੀ_ਚੌੜਾਈ;
		ਪੰਕਤੀ_ਸਮੂਹ->ਉਚਾਈ = ਪੰਕਤੀ_ਸਮੂਹ_ਉਚਾਈ;
		ਸਾਰਣੀ_ਉਚਾਈ += ਪੰਕਤੀ_ਸਮੂਹ_ਉਚਾਈ;
	}
	/* Table height is either the height of the contents, or specified
	 * height if greater */
	ਸਾਰਣੀ_ਉਚਾਈ = max(ਸਾਰਣੀ_ਉਚਾਈ, min_ਉਚਾਈ);
	/** \todo distribute spare height over the row groups / rows / cells */

	/* perform vertical alignment */
	ਲਈ (ਪੰਕਤੀ_ਸਮੂਹ = ਸਾਰਣੀ->children; ਪੰਕਤੀ_ਸਮੂਹ;
			ਪੰਕਤੀ_ਸਮੂਹ = ਪੰਕਤੀ_ਸਮੂਹ->ਅਗਲਾ) {
		ਲਈ (ਪੰਕਤੀ = ਪੰਕਤੀ_ਸਮੂਹ->children; ਪੰਕਤੀ; ਪੰਕਤੀ = ਪੰਕਤੀ->ਅਗਲਾ) {
			ਲਈ (c = ਪੰਕਤੀ->children; c; c = c->ਅਗਲਾ) {
				ਗਿਣਤੀ css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				ਵਾਧੂ_ਉਚਾਈ = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->ਉਚਾਈ - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &ਮੁੱਲ, &unit);

				ਚੋਣ (vertical_align) {
				ਹਾਲਤ CSS_VERTICAL_ALIGN_SUB:
				ਹਾਲਤ CSS_VERTICAL_ALIGN_SUPER:
				ਹਾਲਤ CSS_VERTICAL_ALIGN_ਪਾਠ_ਸਿਖਰ:
				ਹਾਲਤ CSS_VERTICAL_ALIGN_ਪਾਠ_BOTTOM:
				ਹਾਲਤ CSS_VERTICAL_ALIGN_ਸੈੱਟ:
				ਹਾਲਤ CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				ਹਾਲਤ CSS_VERTICAL_ALIGN_ਸਿਖਰ:
					ਰੋਕੋ;
				ਹਾਲਤ CSS_VERTICAL_ALIGN_ਵਿਚਕਾਰ:
					c->padding[ਸਿਖਰ_2] += ਵਾਧੂ_ਉਚਾਈ / 2;
					c->padding[BOTTOM] -= ਵਾਧੂ_ਉਚਾਈ / 2;
					layout_move_children(c, 0,
							ਵਾਧੂ_ਉਚਾਈ / 2);
					ਰੋਕੋ;
				ਹਾਲਤ CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[ਸਿਖਰ_2] += ਵਾਧੂ_ਉਚਾਈ;
					c->padding[BOTTOM] -= ਵਾਧੂ_ਉਚਾਈ;
					layout_move_children(c, 0,
							ਵਾਧੂ_ਉਚਾਈ);
					ਰੋਕੋ;
				ਹਾਲਤ CSS_VERTICAL_ALIGN_INHERIT:
					ਪ੍ਰਤਿਜ੍ਞਾ(0);
					ਰੋਕੋ;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	ਜੇ (ਸਾਰਣੀ->margin[ਸਿਖਰ_2] == AUTO)
		ਸਾਰਣੀ->margin[ਸਿਖਰ_2] = 0;
	ਜੇ (ਸਾਰਣੀ->margin[BOTTOM] == AUTO)
		ਸਾਰਣੀ->margin[BOTTOM] = 0;

	ਮੁਕਤ(ਖਾਨਾ);
	ਮੁਕਤ(excess_y);
	ਮੁਕਤ(ਪੰਕਤੀ_ਵਿਸਤਾਰ);
	ਮੁਕਤ(ਪੰਕਤੀ_ਵਿਸਤਾਰ_cell);
	ਮੁਕਤ(xs);

	ਸਾਰਣੀ->ਚੌੜਾਈ = ਸਾਰਣੀ_ਚੌੜਾਈ;
	ਸਾਰਣੀ->ਉਚਾਈ = ਸਾਰਣੀ_ਉਚਾਈ;

	ਵਾਪਸ true;
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
ਸਥਿਰ bool layout_apply_minmax_ਉਚਾਈ(
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਬਣਤਰ box *box,
		ਬਣਤਰ box *container)
{
	ਪੂਰਨ h;
	ਬਣਤਰ box *containing_ਖੰਡ = ਨਿਰਾ;
	bool updated = false;

	/* Find containing block for percentage heights */
	ਜੇ (box->style != ਨਿਰਾ && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		ਪ੍ਰਤਿਜ੍ਞਾ(container);
		containing_ਖੰਡ = container;
	} ਨਹੀਂਤਾਂ ਜੇ (box->float_container && box->style != ਨਿਰਾ &&
			(css_computed_float(box->style) == CSS_FLOAT_ਖੱਬਾ ||
			 css_computed_float(box->style) == CSS_FLOAT_ਸੱਜਾ)) {
		/* Box is a float */
		ਪ੍ਰਤਿਜ੍ਞਾ(box->ਮਾਪਾ && box->ਮਾਪਾ->ਮਾਪਾ &&
				box->ਮਾਪਾ->ਮਾਪਾ->ਮਾਪਾ);
		containing_ਖੰਡ = box->ਮਾਪਾ->ਮਾਪਾ->ਮਾਪਾ;
	} ਨਹੀਂਤਾਂ ਜੇ (box->ਮਾਪਾ && box->ਮਾਪਾ->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_ਖੰਡ = box->ਮਾਪਾ;
	} ਨਹੀਂਤਾਂ ਜੇ (box->ਮਾਪਾ && box->ਮਾਪਾ->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		ਪ੍ਰਤਿਜ੍ਞਾ(box->ਮਾਪਾ->ਮਾਪਾ);
		containing_ਖੰਡ = box->ਮਾਪਾ->ਮਾਪਾ;
	}

	ਜੇ (box->style) {
		ਗਿਣਤੀ css_ਉਚਾਈ_e htype = CSS_ਉਚਾਈ_AUTO;
		css_fixed ਮੁੱਲ = 0;
		css_unit unit = CSS_UNIT_PX;

		ਜੇ (containing_ਖੰਡ) {
			htype = css_computed_ਉਚਾਈ(containing_ਖੰਡ->style,
					&ਮੁੱਲ, &unit);
		}

		/* max-height */
		ਜੇ (css_computed_max_ਉਚਾਈ(box->style, &ਮੁੱਲ, &unit) ==
				CSS_MAX_ਉਚਾਈ_ਸੈੱਟ) {
			ਜੇ (unit == CSS_UNIT_PCT) {
				ਜੇ (containing_ਖੰਡ &&
					containing_ਖੰਡ->ਉਚਾਈ != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_ਉਚਾਈ_ਸੈੱਟ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ,
						containing_ਖੰਡ->ਉਚਾਈ);
					ਜੇ (h < box->ਉਚਾਈ) {
						box->ਉਚਾਈ = h;
						updated = true;
					}
				}
			} ਨਹੀਂਤਾਂ {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ਮੁੱਲ, unit));
				ਜੇ (h < box->ਉਚਾਈ) {
					box->ਉਚਾਈ = h;
					updated = true;
				}
			}
		}

		/* min-height */
		ਜੇ (ns_computed_min_ਉਚਾਈ(box->style, &ਮੁੱਲ, &unit) ==
				CSS_MIN_ਉਚਾਈ_ਸੈੱਟ) {
			ਜੇ (unit == CSS_UNIT_PCT) {
				ਜੇ (containing_ਖੰਡ &&
					containing_ਖੰਡ->ਉਚਾਈ != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_ਉਚਾਈ_ਸੈੱਟ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ,
						containing_ਖੰਡ->ਉਚਾਈ);
					ਜੇ (h > box->ਉਚਾਈ) {
						box->ਉਚਾਈ = h;
						updated = true;
					}
				}
			} ਨਹੀਂਤਾਂ {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ਮੁੱਲ, unit));
				ਜੇ (h > box->ਉਚਾਈ) {
					box->ਉਚਾਈ = h;
					updated = true;
				}
			}
		}
	}
	ਵਾਪਸ updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
ਸਥਿਰ bool layout_ਖੰਡ_object(ਬਣਤਰ box *ਖੰਡ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(ਖੰਡ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਖੰਡ->type == BOX_ਖੰਡ ||
			ਖੰਡ->type == BOX_FLEX ||
			ਖੰਡ->type == BOX_INLINE_ਖੰਡ ||
			ਖੰਡ->type == BOX_INLINE_FLEX ||
			ਖੰਡ->type == BOX_ਸਾਰਣੀ ||
			ਖੰਡ->type == BOX_ਸਾਰਣੀ_CELL);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਖੰਡ->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", ਖੰਡ,
	      hlcache_handle_get_url(ਖੰਡ->object), ਖੰਡ->ਚੌੜਾਈ);

	ਜੇ (content_can_reformat(ਖੰਡ->object)) {
		content_reformat(ਖੰਡ->object, false, ਖੰਡ->ਚੌੜਾਈ, 1);
	} ਨਹੀਂਤਾਂ {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	ਵਾਪਸ true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_float_ਮੰਜ਼ਿਲ_container(ਬਣਤਰ box *cont, ਬਣਤਰ box *b)
{
	ਬਣਤਰ box *box = cont->float_children;
	ਪੂਰਨ b_bottom = b->y + b->ਉਚਾਈ;

	ਪ੍ਰਤਿਜ੍ਞਾ(b->type == BOX_FLOAT_ਖੱਬਾ || b->type == BOX_FLOAT_ਸੱਜਾ);

	ਜੇ (box == ਨਿਰਾ) {
		/* No other float children */
		b->ਅਗਲਾ_float = ਨਿਰਾ;
		cont->float_children = b;
		ਵਾਪਸ;
	} ਨਹੀਂਤਾਂ ਜੇ (b_bottom >= box->y + box->ਉਚਾਈ) {
		/* Goes at start of list */
		b->ਅਗਲਾ_float = cont->float_children;
		cont->float_children = b;
	} ਨਹੀਂਤਾਂ {
		ਬਣਤਰ box *prev = ਨਿਰਾ;
		ਜਦੋਂ (box != ਨਿਰਾ && b_bottom < box->y + box->ਉਚਾਈ) {
			prev = box;
			box = box->ਅਗਲਾ_float;
		}
		ਜੇ (prev != ਨਿਰਾ) {
			b->ਅਗਲਾ_float = prev->ਅਗਲਾ_float;
			prev->ਅਗਲਾ_float = b;
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
ਸਥਿਰ bool
layout_ਪਾਠ_box_ਵੰਡੋ(html_content *content,
		      plot_font_style_t *fstyle,
		      ਬਣਤਰ box *ਵੰਡੋ_box,
		      size_t new_ਲੰਮਾਈ,
		      ਪੂਰਨ new_ਚੌੜਾਈ)
{
	ਪੂਰਨ space_ਚੌੜਾਈ = ਵੰਡੋ_box->space;
	ਬਣਤਰ box *c2;
	ਅਚਲ ਬਣਤਰ gui_layout_ਸਾਰਣੀ *font_func = content->font_func;
	bool space = (ਵੰਡੋ_box->ਪਾਠ[new_ਲੰਮਾਈ] == ' ');
	ਪੂਰਨ ਵਰਤਿਆ_ਲੰਮਾਈ = new_ਲੰਮਾਈ + (space ? 1 : 0);

	ਜੇ ((space && space_ਚੌੜਾਈ == 0) || space_ਚੌੜਾਈ == UNKNOWN_ਚੌੜਾਈ) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->ਚੌੜਾਈ(fstyle, " ", 1, &space_ਚੌੜਾਈ);
	}

	ਜੇ (ਵੰਡੋ_box->space == UNKNOWN_ਚੌੜਾਈ)
		ਵੰਡੋ_box->space = space_ਚੌੜਾਈ;
	ਜੇ (!space)
		space_ਚੌੜਾਈ = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, ਵੰਡੋ_box, ਆਕਾਰ *c2);
	ਜੇ (!c2)
		ਵਾਪਸ false;
	c2->flags |= CLONE;

	/* Set remaining text in c2 */
	c2->ਪਾਠ += ਵਰਤਿਆ_ਲੰਮਾਈ;

	/* Set c2 according to the remaining text */
	c2->ਚੌੜਾਈ -= new_ਚੌੜਾਈ + space_ਚੌੜਾਈ;
	c2->flags &= ~MEASURED; /* width has been estimated */
	c2->ਲੰਮਾਈ = ਵੰਡੋ_box->ਲੰਮਾਈ - ਵਰਤਿਆ_ਲੰਮਾਈ;

	/* Update split_box for its reduced text */
	ਵੰਡੋ_box->ਚੌੜਾਈ = new_ਚੌੜਾਈ;
	ਵੰਡੋ_box->flags |= MEASURED;
	ਵੰਡੋ_box->ਲੰਮਾਈ = new_ਲੰਮਾਈ;
	ਵੰਡੋ_box->space = space_ਚੌੜਾਈ;

	/* Insert c2 into box list */
	c2->ਅਗਲਾ = ਵੰਡੋ_box->ਅਗਲਾ;
	ਵੰਡੋ_box->ਅਗਲਾ = c2;
	c2->prev = ਵੰਡੋ_box;
	ਜੇ (c2->ਅਗਲਾ)
		c2->ਅਗਲਾ->prev = c2;
	ਨਹੀਂਤਾਂ
		c2->ਮਾਪਾ->ਆਖਰੀ = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      ਵੰਡੋ_box,
	      ਵੰਡੋ_box->ਲੰਮਾਈ,
	      (ਪੂਰਨ)ਵੰਡੋ_box->ਲੰਮਾਈ,
	      ਵੰਡੋ_box->ਪਾਠ);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->ਲੰਮਾਈ,
	      (ਪੂਰਨ)c2->ਲੰਮਾਈ,
	      c2->ਪਾਠ);

	ਵਾਪਸ true;
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
ਸਥਿਰ ਖਾਲੀ
layout_float_ਲੱਭੋ_dimensions(
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ,
		ਅਚਲ css_computed_style *style,
		ਬਣਤਰ box *box)
{
	ਪੂਰਨ ਚੌੜਾਈ, ਉਚਾਈ, max_ਚੌੜਾਈ, min_ਚੌੜਾਈ, max_ਉਚਾਈ, min_ਉਚਾਈ;
	ਪੂਰਨ *margin = box->margin;
	ਪੂਰਨ *padding = box->padding;
	ਬਣਤਰ box_border *border = box->border;
	ਗਿਣਤੀ css_ਉਛਾਲ_e ਉਛਾਲ_x = css_computed_ਉਛਾਲ_x(style);
	ਗਿਣਤੀ css_ਉਛਾਲ_e ਉਛਾਲ_y = css_computed_ਉਛਾਲ_y(style);
	ਪੂਰਨ scrollbar_ਚੌੜਾਈ_x =
			(ਉਛਾਲ_x == CSS_ਉਛਾਲ_SCROLL ||
			 ਉਛਾਲ_x == CSS_ਉਛਾਲ_AUTO) ?
			SCROLLBAR_ਚੌੜਾਈ : 0;
	ਪੂਰਨ scrollbar_ਚੌੜਾਈ_y =
			(ਉਛਾਲ_y == CSS_ਉਛਾਲ_SCROLL ||
			 ਉਛਾਲ_y == CSS_ਉਛਾਲ_AUTO) ?
			SCROLLBAR_ਚੌੜਾਈ : 0;

	layout_ਲੱਭੋ_dimensions(unit_len_ctx, ਉਪਲਬਧ_ਚੌੜਾਈ, -1, box, style,
			&ਚੌੜਾਈ, &ਉਚਾਈ, &max_ਚੌੜਾਈ, &min_ਚੌੜਾਈ,
			&max_ਉਚਾਈ, &min_ਉਚਾਈ, margin, padding, border);

	ਜੇ (margin[ਖੱਬਾ_2] == AUTO)
		margin[ਖੱਬਾ_2] = 0;
	ਜੇ (margin[ਸੱਜਾ_2] == AUTO)
		margin[ਸੱਜਾ_2] = 0;

	ਜੇ (box->gadget == ਨਿਰਾ) {
		padding[ਸੱਜਾ_2] += scrollbar_ਚੌੜਾਈ_y;
		padding[BOTTOM] += scrollbar_ਚੌੜਾਈ_x;
	}

	ਜੇ (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &ਚੌੜਾਈ, &ਉਚਾਈ,
				min_ਚੌੜਾਈ, max_ਚੌੜਾਈ, min_ਉਚਾਈ, max_ਉਚਾਈ);
	} ਨਹੀਂਤਾਂ ਜੇ (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed ਮਾਪ = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		ਪ੍ਰਤਿਜ੍ਞਾ(box->style);

		ਜੇ (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			ਜੇ (ਚੌੜਾਈ == AUTO) {
				ਮਾਪ = INTTOFIX(10);
				ਚੌੜਾਈ = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ਮਾਪ, unit));
			}
			ਜੇ (box->gadget->type == GADGET_FILE &&
					ਉਚਾਈ == AUTO) {
				ਮਾਪ = FLTTOFIX(1.5);
				ਉਚਾਈ = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ਮਾਪ, unit));
			}
		}
		ਜੇ (box->gadget->type == GADGET_TEXTAREA) {
			ਜੇ (ਚੌੜਾਈ == AUTO) {
				ਮਾਪ = INTTOFIX(10);
				ਚੌੜਾਈ = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ਮਾਪ, unit));
			}
			ਜੇ (ਉਚਾਈ == AUTO) {
				ਮਾਪ = INTTOFIX(4);
				ਉਚਾਈ = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ਮਾਪ, unit));
			}
		}
	} ਨਹੀਂਤਾਂ ਜੇ (ਚੌੜਾਈ == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		ਚੌੜਾਈ = min(max(box->min_ਚੌੜਾਈ, ਉਪਲਬਧ_ਚੌੜਾਈ),
				box->max_ਚੌੜਾਈ);

		/* width includes margin, borders and padding */
		ਜੇ (ਚੌੜਾਈ == ਉਪਲਬਧ_ਚੌੜਾਈ) {
			ਚੌੜਾਈ -= box->margin[ਖੱਬਾ_2] + box->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
					box->padding[ਖੱਬਾ_2] +
					box->padding[ਸੱਜਾ_2] +
					box->border[ਸੱਜਾ_2].ਚੌੜਾਈ +
					box->margin[ਸੱਜਾ_2];
		} ਨਹੀਂਤਾਂ {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			ਪੂਰਨ fixed = 0;
			ਦਸ਼ਮਲਵ frac = 0;
			calculate_mbp_ਚੌੜਾਈ(unit_len_ctx, box->style, ਖੱਬਾ_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_ਚੌੜਾਈ(unit_len_ctx, box->style, ਸੱਜਾ_2,
					true, true, true, &fixed, &frac);
			ਜੇ (fixed < 0)
				fixed = 0;

			ਚੌੜਾਈ -= fixed;
		}

		ਜੇ (max_ਚੌੜਾਈ >= 0 && ਚੌੜਾਈ > max_ਚੌੜਾਈ) ਚੌੜਾਈ = max_ਚੌੜਾਈ;
		ਜੇ (min_ਚੌੜਾਈ >  0 && ਚੌੜਾਈ < min_ਚੌੜਾਈ) ਚੌੜਾਈ = min_ਚੌੜਾਈ;

	} ਨਹੀਂਤਾਂ {
		ਜੇ (max_ਚੌੜਾਈ >= 0 && ਚੌੜਾਈ > max_ਚੌੜਾਈ) ਚੌੜਾਈ = max_ਚੌੜਾਈ;
		ਜੇ (min_ਚੌੜਾਈ >  0 && ਚੌੜਾਈ < min_ਚੌੜਾਈ) ਚੌੜਾਈ = min_ਚੌੜਾਈ;
		ਚੌੜਾਈ -= scrollbar_ਚੌੜਾਈ_y;
	}

	box->ਚੌੜਾਈ = ਚੌੜਾਈ;
	box->ਉਚਾਈ = ਉਚਾਈ;

	ਜੇ (margin[ਸਿਖਰ_2] == AUTO)
		margin[ਸਿਖਰ_2] = 0;
	ਜੇ (margin[BOTTOM] == AUTO)
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
ਸਥਿਰ bool layout_float(ਬਣਤਰ box *b, ਪੂਰਨ ਚੌੜਾਈ, html_content *content)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(b->type == BOX_ਸਾਰਣੀ ||
	       b->type == BOX_ਖੰਡ ||
	       b->type == BOX_INLINE_ਖੰਡ ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_ਲੱਭੋ_dimensions(&content->unit_len_ctx, ਚੌੜਾਈ, b->style, b);
	ਜੇ (b->type == BOX_ਸਾਰਣੀ || b->type == BOX_INLINE_FLEX) {
		ਜੇ (b->type == BOX_ਸਾਰਣੀ) {
			ਜੇ (!layout_ਸਾਰਣੀ(b, ਚੌੜਾਈ, content))
				ਵਾਪਸ false;
		} ਨਹੀਂਤਾਂ {
			ਜੇ (!layout_flex(b, ਚੌੜਾਈ, content))
				ਵਾਪਸ false;
		}
		ਜੇ (b->margin[ਖੱਬਾ_2] == AUTO)
			b->margin[ਖੱਬਾ_2] = 0;
		ਜੇ (b->margin[ਸੱਜਾ_2] == AUTO)
			b->margin[ਸੱਜਾ_2] = 0;
		ਜੇ (b->margin[ਸਿਖਰ_2] == AUTO)
			b->margin[ਸਿਖਰ_2] = 0;
		ਜੇ (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} ਨਹੀਂਤਾਂ {
		ਵਾਪਸ layout_ਖੰਡ_context(b, -1, content);
	}
	ਵਾਪਸ true;
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
ਸਥਿਰ ਖਾਲੀ
ਰੱਖੋ_float_ਹੇਠਾਂ(ਬਣਤਰ box *c, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ cx, ਪੂਰਨ y, ਬਣਤਰ box *cont)
{
	ਪੂਰਨ x0, x1, yy;
	ਬਣਤਰ box *ਖੱਬਾ;
	ਬਣਤਰ box *ਸੱਜਾ;

	yy = y > cont->cached_ਰੱਖੋ_ਹੇਠਾਂ_level ?
			y : cont->cached_ਰੱਖੋ_ਹੇਠਾਂ_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      ਚੌੜਾਈ, cx, y, cont);

	ਕਰੋ {
		y = yy;
		x0 = cx;
		x1 = cx + ਚੌੜਾਈ;
		ਲੱਭੋ_sides(cont->float_children, y, y + c->ਉਚਾਈ, &x0, &x1,
				&ਖੱਬਾ, &ਸੱਜਾ);
		ਜੇ (ਖੱਬਾ != 0 && ਸੱਜਾ != 0) {
			yy = (ਖੱਬਾ->y + ਖੱਬਾ->ਉਚਾਈ <
					ਸੱਜਾ->y + ਸੱਜਾ->ਉਚਾਈ ?
					ਖੱਬਾ->y + ਖੱਬਾ->ਉਚਾਈ :
					ਸੱਜਾ->y + ਸੱਜਾ->ਉਚਾਈ);
		} ਨਹੀਂਤਾਂ ਜੇ (ਖੱਬਾ == 0 && ਸੱਜਾ != 0) {
			yy = ਸੱਜਾ->y + ਸੱਜਾ->ਉਚਾਈ;
		} ਨਹੀਂਤਾਂ ਜੇ (ਖੱਬਾ != 0 && ਸੱਜਾ == 0) {
			yy = ਖੱਬਾ->y + ਖੱਬਾ->ਉਚਾਈ;
		}
	} ਜਦੋਂ ((ਖੱਬਾ != 0 || ਸੱਜਾ != 0) && (c->ਚੌੜਾਈ > x1 - x0));

	ਜੇ (c->type == BOX_FLOAT_ਖੱਬਾ) {
		c->x = x0;
	} ਨਹੀਂਤਾਂ {
		c->x = x1 - c->ਚੌੜਾਈ;
	}
	c->y = y;
	cont->cached_ਰੱਖੋ_ਹੇਠਾਂ_level = y;
}


/**
 * Calculate line height from a style.
 */
ਸਥਿਰ ਪੂਰਨ ਸਤਰ_ਉਚਾਈ(
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਅਚਲ css_computed_style *style)
{
	ਗਿਣਤੀ css_ਸਤਰ_ਉਚਾਈ_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed ਸਤਰ_ਉਚਾਈ;

	ਪ੍ਰਤਿਜ੍ਞਾ(style);

	lhtype = css_computed_ਸਤਰ_ਉਚਾਈ(style, &lhvalue, &lhunit);
	ਜੇ (lhtype == CSS_ਸਤਰ_ਉਚਾਈ_ਸਾਧਾਰਨ) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_ਸਤਰ_ਉਚਾਈ_ਸੰਖਿਆ;
	}

	ਜੇ (lhtype == CSS_ਸਤਰ_ਉਚਾਈ_ਸੰਖਿਆ ||
			lhunit == CSS_UNIT_PCT) {
		ਸਤਰ_ਉਚਾਈ = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		ਜੇ (lhtype != CSS_ਸਤਰ_ਉਚਾਈ_ਸੰਖਿਆ)
			ਸਤਰ_ਉਚਾਈ = FDIV(ਸਤਰ_ਉਚਾਈ, F_100);
	} ਨਹੀਂਤਾਂ {
		ਪ੍ਰਤਿਜ੍ਞਾ(lhunit != CSS_UNIT_PCT);

		ਸਤਰ_ਉਚਾਈ = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	ਵਾਪਸ FIXTOINT(ਸਤਰ_ਉਚਾਈ);
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
ਸਥਿਰ bool
layout_ਸਤਰ(ਬਣਤਰ box *ਪਹਿਲਾ,
	    ਪੂਰਨ *ਚੌੜਾਈ,
	    ਪੂਰਨ *y,
	    ਪੂਰਨ cx,
	    ਪੂਰਨ cy,
	    ਬਣਤਰ box *cont,
	    bool indent,
	    bool has_ਪਾਠ_children,
	    html_content *content,
	    ਬਣਤਰ box **ਅਗਲਾ_box)
{
	ਪੂਰਨ ਉਚਾਈ, ਵਰਤਿਆ_ਉਚਾਈ;
	ਪੂਰਨ x0 = 0;
	ਪੂਰਨ x1 = *ਚੌੜਾਈ;
	ਪੂਰਨ x, h, x_ਪਿਛਲਾ;
	ਪੂਰਨ fy = cy;
	ਬਣਤਰ box *ਖੱਬਾ;
	ਬਣਤਰ box *ਸੱਜਾ;
	ਬਣਤਰ box *b;
	ਬਣਤਰ box *ਵੰਡੋ_box = 0;
	ਬਣਤਰ box *d;
	ਬਣਤਰ box *br_box = 0;
	bool move_y = false;
	bool ਰੱਖੋ_ਹੇਠਾਂ = false;
	ਪੂਰਨ space_before = 0, space_after = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ inline_ਗਿਣਤੀ = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਅਚਲ ਬਣਤਰ gui_layout_ਸਾਰਣੀ *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      ਪਹਿਲਾ,
	      (ਪੂਰਨ)ਪਹਿਲਾ->ਲੰਮਾਈ,
	      ਪਹਿਲਾ->ਪਾਠ,
	      *ਚੌੜਾਈ,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	ਲੱਭੋ_sides(cont->float_children, cy, cy, &x0, &x1, &ਖੱਬਾ, &ਸੱਜਾ);
	x0 -= cx;
	x1 -= cx;

	ਜੇ (indent)
		x0 += layout_ਪਾਠ_indent(&content->unit_len_ctx,
				ਪਹਿਲਾ->ਮਾਪਾ->ਮਾਪਾ->style, *ਚੌੜਾਈ);

	ਜੇ (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	ਜੇ (has_ਪਾਠ_children || ਪਹਿਲਾ->ਮਾਪਾ->ਮਾਪਾ->gadget)
		ਵਰਤਿਆ_ਉਚਾਈ = ਉਚਾਈ = ਸਤਰ_ਉਚਾਈ(&content->unit_len_ctx,
				ਪਹਿਲਾ->ਮਾਪਾ->ਮਾਪਾ->style);
	ਨਹੀਂਤਾਂ
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		ਵਰਤਿਆ_ਉਚਾਈ = ਉਚਾਈ = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	ਲਈ (x = 0, b = ਪਹਿਲਾ; x <= x1 - x0 && b != 0; b = b->ਅਗਲਾ) {
		ਪੂਰਨ min_ਚੌੜਾਈ, max_ਚੌੜਾਈ, min_ਉਚਾਈ, max_ਉਚਾਈ;

		ਪ੍ਰਤਿਜ੍ਞਾ(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		ਜੇ (b->type == BOX_BR)
			ਰੋਕੋ;

		ਜੇ (lh__box_is_float_box(b))
			ਜਾਰੀ;
		ਜੇ (b->type == BOX_INLINE_ਖੰਡ &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			ਜਾਰੀ;

		ਪ੍ਰਤਿਜ੍ਞਾ(b->style != ਨਿਰਾ);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		ਜੇ (b->type == BOX_INLINE_ਖੰਡ ||
		    b->type == BOX_INLINE_FLEX) {
			ਜੇ (b->max_ਚੌੜਾਈ != UNKNOWN_ਚੌੜਾਈ)
				ਜੇ (!layout_float(b, *ਚੌੜਾਈ, content))
					ਵਾਪਸ false;
			h = b->border[ਸਿਖਰ_2].ਚੌੜਾਈ + b->padding[ਸਿਖਰ_2] + b->ਉਚਾਈ +
					b->padding[BOTTOM] +
					b->border[BOTTOM].ਚੌੜਾਈ;
			ਜੇ (ਉਚਾਈ < h)
				ਉਚਾਈ = h;
			x += b->margin[ਖੱਬਾ_2] + b->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
					b->padding[ਖੱਬਾ_2] + b->ਚੌੜਾਈ +
					b->padding[ਸੱਜਾ_2] +
					b->border[ਸੱਜਾ_2].ਚੌੜਾਈ +
					b->margin[ਸੱਜਾ_2];
			space_after = 0;
			ਜਾਰੀ;
		}

		ਜੇ (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_ਲੱਭੋ_dimensions(&content->unit_len_ctx,
					*ਚੌੜਾਈ, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			ਲਈ (i = 0; i != 4; i++)
				ਜੇ (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[ਖੱਬਾ_2] + b->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
					b->padding[ਖੱਬਾ_2];
			ਜੇ (b->inline_ਸਿਰਾ) {
				b->inline_ਸਿਰਾ->margin[ਸੱਜਾ_2] = b->margin[ਸੱਜਾ_2];
				b->inline_ਸਿਰਾ->padding[ਸੱਜਾ_2] =
						b->padding[ਸੱਜਾ_2];
				b->inline_ਸਿਰਾ->border[ਸੱਜਾ_2] =
						b->border[ਸੱਜਾ_2];
			} ਨਹੀਂਤਾਂ {
				x += b->padding[ਸੱਜਾ_2] +
						b->border[ਸੱਜਾ_2].ਚੌੜਾਈ +
						b->margin[ਸੱਜਾ_2];
			}
		} ਨਹੀਂਤਾਂ ਜੇ (b->type == BOX_INLINE_ਸਿਰਾ) {
			b->ਚੌੜਾਈ = 0;
			ਜੇ (b->space == UNKNOWN_ਚੌੜਾਈ) {
				font_func->ਚੌੜਾਈ(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[ਸੱਜਾ_2] + b->border[ਸੱਜਾ_2].ਚੌੜਾਈ +
					b->margin[ਸੱਜਾ_2];
			ਜਾਰੀ;
		}

		ਜੇ (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->ਉਚਾਈ = ਸਤਰ_ਉਚਾਈ(&content->unit_len_ctx,
					b->style ? b->style :
					b->ਮਾਪਾ->ਮਾਪਾ->style);
			ਜੇ (ਉਚਾਈ < b->ਉਚਾਈ)
				ਉਚਾਈ = b->ਉਚਾਈ;

			ਜੇ (!b->ਪਾਠ) {
				b->ਚੌੜਾਈ = 0;
				space_after = 0;
				ਜਾਰੀ;
			}

			ਜੇ (b->ਚੌੜਾਈ == UNKNOWN_ਚੌੜਾਈ) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				ਜੇ (b->ਮਾਪਾ->ਮਾਪਾ->gadget &&
						b->ਮਾਪਾ->ਮਾਪਾ->gadget->type
						== GADGET_SELECT) {
					ਪੂਰਨ opt_maxwidth = 0;
					ਬਣਤਰ form_option *o;

					ਲਈ (o = b->ਮਾਪਾ->ਮਾਪਾ->gadget->
							ਅੰਕੜੇ.select.ਚੀਜ਼ਾਂ; o;
							o = o->ਅਗਲਾ) {
						ਪੂਰਨ opt_ਚੌੜਾਈ;
						font_func->ਚੌੜਾਈ(&fstyle,
								o->ਪਾਠ,
								ਲੜੀ_ਲੰਬਾਈ(o->ਪਾਠ),
								&opt_ਚੌੜਾਈ);

						ਜੇ (opt_maxwidth < opt_ਚੌੜਾਈ)
							opt_maxwidth =opt_ਚੌੜਾਈ;
					}
					b->ਚੌੜਾਈ = opt_maxwidth;
					ਜੇ (nsoption_bool(core_select_menu))
						b->ਚੌੜਾਈ += SCROLLBAR_ਚੌੜਾਈ;
				} ਨਹੀਂਤਾਂ {
					font_func->ਚੌੜਾਈ(&fstyle, b->ਪਾਠ,
							b->ਲੰਮਾਈ, &b->ਚੌੜਾਈ);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			ਜੇ (b->ਪਾਠ && (x + b->ਚੌੜਾਈ < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->ਅਗਲਾ) {
				font_func->ਚੌੜਾਈ(&fstyle, b->ਪਾਠ,
						 b->ਲੰਮਾਈ, &b->ਚੌੜਾਈ);
				b->flags |= MEASURED;
			}

			x += b->ਚੌੜਾਈ;
			ਜੇ (b->space == UNKNOWN_ਚੌੜਾਈ) {
				font_func->ਚੌੜਾਈ(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			ਜਾਰੀ;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		ਪ੍ਰਤਿਜ੍ਞਾ(b->style);

		layout_ਲੱਭੋ_dimensions(&content->unit_len_ctx,
				*ਚੌੜਾਈ, -1, b, b->style,
				&b->ਚੌੜਾਈ, &b->ਉਚਾਈ,
				&max_ਚੌੜਾਈ, &min_ਚੌੜਾਈ,
				&max_ਉਚਾਈ, &min_ਉਚਾਈ,
				ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ);

		ਜੇ (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->ਚੌੜਾਈ, &b->ਉਚਾਈ,
					min_ਚੌੜਾਈ, max_ਚੌੜਾਈ,
					min_ਉਚਾਈ, max_ਉਚਾਈ);
		} ਨਹੀਂਤਾਂ ਜੇ (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			ਜੇ (b->ਚੌੜਾਈ == AUTO)
				b->ਚੌੜਾਈ = 400;
			ਜੇ (b->ਉਚਾਈ == AUTO)
				b->ਉਚਾਈ = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} ਨਹੀਂਤਾਂ {
			/* form control with no object */
			ਜੇ (b->ਚੌੜਾਈ == AUTO)
				b->ਚੌੜਾਈ = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			ਜੇ (b->ਉਚਾਈ == AUTO)
				b->ਉਚਾਈ = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		ਜੇ (b->object && content_can_reformat(b->object) &&
				b->ਚੌੜਾਈ !=
				content_get_ਉਪਲਬਧ_ਚੌੜਾਈ(b->object)) {
			css_fixed ਮੁੱਲ = 0;
			css_unit unit = CSS_UNIT_PX;
			ਗਿਣਤੀ css_ਉਚਾਈ_e htype = css_computed_ਉਚਾਈ(b->style,
					&ਮੁੱਲ, &unit);

			content_reformat(b->object, false, b->ਚੌੜਾਈ, b->ਉਚਾਈ);

			ਜੇ (htype == CSS_ਉਚਾਈ_AUTO)
				b->ਉਚਾਈ = content_get_ਉਚਾਈ(b->object);
		}

		ਜੇ (ਉਚਾਈ < b->ਉਚਾਈ)
			ਉਚਾਈ = b->ਉਚਾਈ;

		x += b->ਚੌੜਾਈ;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *ਚੌੜਾਈ;
	ਲੱਭੋ_sides(cont->float_children, cy, cy + ਉਚਾਈ, &x0, &x1,
			&ਖੱਬਾ, &ਸੱਜਾ);
	x0 -= cx;
	x1 -= cx;

	ਜੇ (indent)
		x0 += layout_ਪਾਠ_indent(&content->unit_len_ctx,
				ਪਹਿਲਾ->ਮਾਪਾ->ਮਾਪਾ->style, *ਚੌੜਾਈ);

	ਜੇ (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	ਲਈ (x = x_ਪਿਛਲਾ = 0, b = ਪਹਿਲਾ; x <= x1 - x0 && b; b = b->ਅਗਲਾ) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		ਜੇ (b->type == BOX_INLINE_ਖੰਡ &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} ਨਹੀਂਤਾਂ ਜੇ (lh__box_is_inline_flow(b)) {
			ਪ੍ਰਤਿਜ੍ਞਾ(b->ਚੌੜਾਈ != UNKNOWN_ਚੌੜਾਈ);

			x_ਪਿਛਲਾ = x;
			x += space_after;
			b->x = x;

			ਜੇ ((b->type == BOX_INLINE && !b->inline_ਸਿਰਾ) ||
					b->type == BOX_INLINE_ਖੰਡ ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[ਖੱਬਾ_2] + b->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
				x = b->x + b->padding[ਖੱਬਾ_2] + b->ਚੌੜਾਈ +
						b->padding[ਸੱਜਾ_2] +
						b->border[ਸੱਜਾ_2].ਚੌੜਾਈ +
						b->margin[ਸੱਜਾ_2];
			} ਨਹੀਂਤਾਂ ਜੇ (b->type == BOX_INLINE) {
				b->x += b->margin[ਖੱਬਾ_2] + b->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
				x = b->x + b->padding[ਖੱਬਾ_2] + b->ਚੌੜਾਈ;
			} ਨਹੀਂਤਾਂ ਜੇ (b->type == BOX_INLINE_ਸਿਰਾ) {
				b->ਉਚਾਈ = b->inline_ਸਿਰਾ->ਉਚਾਈ;
				x += b->padding[ਸੱਜਾ_2] +
						b->border[ਸੱਜਾ_2].ਚੌੜਾਈ +
						b->margin[ਸੱਜਾ_2];
			} ਨਹੀਂਤਾਂ {
				x += b->ਚੌੜਾਈ;
			}

			space_before = space_after;
			ਜੇ (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			ਨਹੀਂਤਾਂ ਜੇ (b->ਪਾਠ || b->type == BOX_INLINE_ਸਿਰਾ) {
				ਜੇ (b->space == UNKNOWN_ਚੌੜਾਈ) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->ਚੌੜਾਈ(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} ਨਹੀਂਤਾਂ {
				space_after = 0;
			}
			ਵੰਡੋ_box = b;
			move_y = true;
			inline_ਗਿਣਤੀ++;
		} ਨਹੀਂਤਾਂ ਜੇ (b->type == BOX_BR) {
			b->x = x;
			b->ਚੌੜਾਈ = 0;
			br_box = b;
			b = b->ਅਗਲਾ;
			ਵੰਡੋ_box = 0;
			move_y = true;
			ਰੋਕੋ;

		} ਨਹੀਂਤਾਂ {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_ਰੱਖੋ_ਹੇਠਾਂ_level = 0;
			b->float_container = d->float_container = cont;

			ਜੇ (!layout_float(d, *ਚੌੜਾਈ, content))
				ਵਾਪਸ false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[ਸਿਖਰ_2],
			      d->border[ਸਿਖਰ_2].ਚੌੜਾਈ);

			d->x = d->margin[ਖੱਬਾ_2] + d->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
			d->y = d->margin[ਸਿਖਰ_2] + d->border[ਸਿਖਰ_2].ਚੌੜਾਈ;
			b->ਚੌੜਾਈ = d->margin[ਖੱਬਾ_2] + d->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
					d->padding[ਖੱਬਾ_2] + d->ਚੌੜਾਈ +
					d->padding[ਸੱਜਾ_2] +
					d->border[ਸੱਜਾ_2].ਚੌੜਾਈ +
					d->margin[ਸੱਜਾ_2];
			b->ਉਚਾਈ = d->margin[ਸਿਖਰ_2] + d->border[ਸਿਖਰ_2].ਚੌੜਾਈ +
					d->padding[ਸਿਖਰ_2] + d->ਉਚਾਈ +
					d->padding[BOTTOM] +
					d->border[BOTTOM].ਚੌੜਾਈ +
					d->margin[BOTTOM];

			ਜੇ (b->ਚੌੜਾਈ > (x1 - x0) - x)
				ਰੱਖੋ_ਹੇਠਾਂ = true;
			ਜੇ (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ਖੱਬਾ && ਖੱਬਾ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ਸੱਜਾ &&
						ਸੱਜਾ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						ਖੱਬਾ == 0 && ਸੱਜਾ == 0)) &&
					(!ਰੱਖੋ_ਹੇਠਾਂ ||
					(ਖੱਬਾ == 0 && ਸੱਜਾ == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_ਰੱਖੋ_ਹੇਠਾਂ_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				ਜੇ (b->type == BOX_FLOAT_ਖੱਬਾ) {
					b->x = cx + x0;
					ਜੇ (b->ਚੌੜਾਈ > 0)
						x0 += b->ਚੌੜਾਈ;
					ਖੱਬਾ = b;
				} ਨਹੀਂਤਾਂ {
					b->x = cx + x1 - b->ਚੌੜਾਈ;
					ਜੇ (b->ਚੌੜਾਈ > 0)
						x1 -= b->ਚੌੜਾਈ;
					ਸੱਜਾ = b;
				}
				b->y = cy;
			} ਨਹੀਂਤਾਂ {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				ਪੂਰਨ fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_ਰੱਖੋ_ਹੇਠਾਂ_level) ?
						fcy :
						cont->cached_ਰੱਖੋ_ਹੇਠਾਂ_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + ਉਚਾਈ : fy;

				ਰੱਖੋ_float_ਹੇਠਾਂ(b, *ਚੌੜਾਈ, cx, fy, cont);
				fy = b->y;
				ਜੇ (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ਖੱਬਾ && ਖੱਬਾ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ਸੱਜਾ &&
						ਸੱਜਾ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(ਖੱਬਾ != 0 || ਸੱਜਾ != 0)))) {
					/* to be cleared below existing
					 * floats */
					ਜੇ (b->type == BOX_FLOAT_ਖੱਬਾ)
						b->x = cx;
					ਨਹੀਂਤਾਂ
						b->x = cx + *ਚੌੜਾਈ - b->ਚੌੜਾਈ;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					ਜੇ (fcy > cont->clear_level)
						cont->clear_level = fcy;
					ਜੇ (b->y < fcy)
						b->y = fcy;
				}
				ਜੇ (b->type == BOX_FLOAT_ਖੱਬਾ)
					ਖੱਬਾ = b;
				ਨਹੀਂਤਾਂ
					ਸੱਜਾ = b;
			}
			ਜੋੜੋ_float_ਮੰਜ਼ਿਲ_container(cont, b);

			ਵੰਡੋ_box = 0;
		}
	}

	ਜੇ (x1 - x0 < x && ਵੰਡੋ_box) {
		/* the last box went over the end */
		size_t ਵੰਡੋ = 0;
		ਪੂਰਨ w;
		bool no_wrap = css_computed_white_space(
				ਵੰਡੋ_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				ਵੰਡੋ_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_ਪਿਛਲਾ;

		ਜੇ (!no_wrap &&
		    (ਵੰਡੋ_box->type == BOX_INLINE ||
		     ਵੰਡੋ_box->type == BOX_ਪਾਠ) &&
		    !ਵੰਡੋ_box->object &&
		    !(ਵੰਡੋ_box->flags & REPLACE_DIM) &&
		    !(ਵੰਡੋ_box->flags & IFRAME) &&
		    !ਵੰਡੋ_box->gadget && ਵੰਡੋ_box->ਪਾਠ) {

			font_plot_style_from_css(&content->unit_len_ctx,
					ਵੰਡੋ_box->style, &fstyle);
			/** \todo handle errors */
			font_func->ਵੰਡੋ(&fstyle,
					 ਵੰਡੋ_box->ਪਾਠ,
					 ਵੰਡੋ_box->ਲੰਮਾਈ,
					 x1 - x0 - x - space_before,
					 &ਵੰਡੋ,
					 &w);
		}

		/* split == 0 implies that text can't be split */

		ਜੇ (ਵੰਡੋ == 0)
			w = ਵੰਡੋ_box->ਚੌੜਾਈ;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      ਵੰਡੋ_box,
		      (ਪੂਰਨ)ਵੰਡੋ_box->ਲੰਮਾਈ,
		      ਵੰਡੋ_box->ਪਾਠ,
		      ਵੰਡੋ,
		      w,
		      ਖੱਬਾ,
		      ਸੱਜਾ,
		      inline_ਗਿਣਤੀ);

		ਜੇ ((ਵੰਡੋ == 0 || x1 - x0 <= x + space_before + w) &&
				!ਖੱਬਾ && !ਸੱਜਾ && inline_ਗਿਣਤੀ == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			ਜੇ (ਵੰਡੋ == 0 || ਵੰਡੋ == ਵੰਡੋ_box->ਲੰਮਾਈ) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = ਵੰਡੋ_box->ਅਗਲਾ;
			} ਨਹੀਂਤਾਂ {
				/* cut off first word for this line */
				ਜੇ (!layout_ਪਾਠ_box_ਵੰਡੋ(content, &fstyle,
						ਵੰਡੋ_box, ਵੰਡੋ, w))
					ਵਾਪਸ false;
				b = ਵੰਡੋ_box->ਅਗਲਾ;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} ਨਹੀਂਤਾਂ ਜੇ ((ਵੰਡੋ == 0 || x1 - x0 <= x + space_before + w) &&
				inline_ਗਿਣਤੀ == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			ਪ੍ਰਤਿਜ੍ਞਾ(ਖੱਬਾ || ਸੱਜਾ);
			ਵਰਤਿਆ_ਉਚਾਈ = 0;
			ਜੇ (ਖੱਬਾ) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      ਖੱਬਾ->y,
				      ਖੱਬਾ->ਉਚਾਈ);

				ਵਰਤਿਆ_ਉਚਾਈ = ਖੱਬਾ->y + ਖੱਬਾ->ਉਚਾਈ - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      ਵਰਤਿਆ_ਉਚਾਈ);

			}
			ਜੇ (ਸੱਜਾ && ਵਰਤਿਆ_ਉਚਾਈ <
					ਸੱਜਾ->y + ਸੱਜਾ->ਉਚਾਈ - cy + 1)
				ਵਰਤਿਆ_ਉਚਾਈ = ਸੱਜਾ->y + ਸੱਜਾ->ਉਚਾਈ - cy + 1;

			ਜੇ (ਵਰਤਿਆ_ਉਚਾਈ < 0)
				ਵਰਤਿਆ_ਉਚਾਈ = 0;

			b = ਵੰਡੋ_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} ਨਹੀਂਤਾਂ ਜੇ (ਵੰਡੋ == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = ਵੰਡੋ_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} ਨਹੀਂਤਾਂ {
			/* fit as many words as possible */
			ਪ੍ਰਤਿਜ੍ਞਾ(ਵੰਡੋ != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (ਪੂਰਨ)ਵੰਡੋ_box->ਲੰਮਾਈ, ਵੰਡੋ_box->ਪਾਠ,
			      x1 - x0, ਵੰਡੋ, w);

			ਜੇ (ਵੰਡੋ != ਵੰਡੋ_box->ਲੰਮਾਈ) {
				ਜੇ (!layout_ਪਾਠ_box_ਵੰਡੋ(content, &fstyle,
						ਵੰਡੋ_box, ਵੰਡੋ, w))
					ਵਾਪਸ false;
				b = ਵੰਡੋ_box->ਅਗਲਾ;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	ਚੋਣ (css_computed_ਪਾਠ_align(ਪਹਿਲਾ->ਮਾਪਾ->ਮਾਪਾ->style)) {
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_ਸੱਜਾ:
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_LIBCSS_ਸੱਜਾ:
		x0 = x1 - x;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_CENTER:
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_ਖੱਬਾ:
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_LIBCSS_ਖੱਬਾ:
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_JUSTIFY:
		/* leave on left */
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_DEFAULT:
		/* None; consider text direction */
		ਚੋਣ (css_computed_direction(ਪਹਿਲਾ->ਮਾਪਾ->ਮਾਪਾ->style)) {
		ਹਾਲਤ CSS_DIRECTION_LTR:
			/* leave on left */
			ਰੋਕੋ;
		ਹਾਲਤ CSS_DIRECTION_RTL:
			x0 = x1 - x;
			ਰੋਕੋ;
		}
		ਰੋਕੋ;
	}

	ਲਈ (d = ਪਹਿਲਾ; d != b; d = d->ਅਗਲਾ) {
		d->flags &= ~NEW_ਸਤਰ;

		ਜੇ (d->type == BOX_INLINE_ਖੰਡ &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			ਜਾਰੀ;
		} ਨਹੀਂਤਾਂ ਜੇ ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_ਪਾਠ ||
				d->type == BOX_INLINE_ਸਿਰਾ) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[ਸਿਖਰ_2];

			ਜੇ (d->type == BOX_ਪਾਠ && d->ਉਚਾਈ > ਵਰਤਿਆ_ਉਚਾਈ) {
				/*ਪਾਠ*/
				ਵਰਤਿਆ_ਉਚਾਈ = d->ਉਚਾਈ;
			}
		} ਨਹੀਂਤਾਂ ਜੇ ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_ਖੰਡ) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[ਸਿਖਰ_2].ਚੌੜਾਈ + d->margin[ਸਿਖਰ_2];
			h = d->margin[ਸਿਖਰ_2] + d->border[ਸਿਖਰ_2].ਚੌੜਾਈ +
					d->padding[ਸਿਖਰ_2] + d->ਉਚਾਈ +
					d->padding[BOTTOM] +
					d->border[BOTTOM].ਚੌੜਾਈ +
					d->margin[BOTTOM];
			ਜੇ (ਵਰਤਿਆ_ਉਚਾਈ < h)
				ਵਰਤਿਆ_ਉਚਾਈ = h;
		}
	}

	ਪਹਿਲਾ->flags |= NEW_ਸਤਰ;

	ਪ੍ਰਤਿਜ੍ਞਾ(b != ਪਹਿਲਾ || (move_y && 0 < ਵਰਤਿਆ_ਉਚਾਈ && (ਖੱਬਾ || ਸੱਜਾ)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	ਲਈ (d = ਪਹਿਲਾ; d != b; d = d->ਅਗਲਾ) {
		ਜੇ ((d->type == BOX_INLINE && d->inline_ਸਿਰਾ) ||
				d->type == BOX_BR ||
				d->type == BOX_ਪਾਠ ||
				d->type == BOX_INLINE_ਸਿਰਾ) {
			css_fixed ਮੁੱਲ = 0;
			css_unit unit = CSS_UNIT_PX;
			ਚੋਣ (css_computed_vertical_align(d->style, &ਮੁੱਲ,
					&unit)) {
			ਹਾਲਤ CSS_VERTICAL_ALIGN_SUPER:
			ਹਾਲਤ CSS_VERTICAL_ALIGN_ਸਿਖਰ:
			ਹਾਲਤ CSS_VERTICAL_ALIGN_ਪਾਠ_ਸਿਖਰ:
				/* already at top */
				ਰੋਕੋ;
			ਹਾਲਤ CSS_VERTICAL_ALIGN_SUB:
			ਹਾਲਤ CSS_VERTICAL_ALIGN_BOTTOM:
			ਹਾਲਤ CSS_VERTICAL_ALIGN_ਪਾਠ_BOTTOM:
				d->y += ਵਰਤਿਆ_ਉਚਾਈ - d->ਉਚਾਈ;
				ਰੋਕੋ;
			ਮੂਲ:
			ਹਾਲਤ CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (ਵਰਤਿਆ_ਉਚਾਈ - d->ਉਚਾਈ);
				ਰੋਕੋ;
			}
		}
	}

	/* handle clearance for br */
	ਜੇ (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		ਪੂਰਨ clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		ਜੇ (ਵਰਤਿਆ_ਉਚਾਈ < clear_y - cy)
			ਵਰਤਿਆ_ਉਚਾਈ = clear_y - cy;
	}

	ਜੇ (move_y)
		*y += ਵਰਤਿਆ_ਉਚਾਈ;
	*ਅਗਲਾ_box = b;
	*ਚੌੜਾਈ = x; /* return actual width */
	ਵਾਪਸ true;
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
ਸਥਿਰ bool layout_inline_container(ਬਣਤਰ box *inline_container, ਪੂਰਨ ਚੌੜਾਈ,
		ਬਣਤਰ box *cont, ਪੂਰਨ cx, ਪੂਰਨ cy, html_content *content)
{
	bool ਪਹਿਲਾ_ਸਤਰ = true;
	bool has_ਪਾਠ_children;
	ਬਣਤਰ box *c, *ਅਗਲਾ;
	ਪੂਰਨ y = 0;
	ਪੂਰਨ curwidth,maxwidth = ਚੌੜਾਈ;

	ਪ੍ਰਤਿਜ੍ਞਾ(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      ਚੌੜਾਈ,
	      cont,
	      cx,
	      cy);


	has_ਪਾਠ_children = false;
	ਲਈ (c = inline_container->children; c; c = c->ਅਗਲਾ) {
		bool is_pre = false;

		ਜੇ (c->style) {
			ਗਿਣਤੀ css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_ਸਤਰ ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		ਜੇ ((lh__box_is_object(c) == false &&
				c->ਪਾਠ && (c->ਲੰਮਾਈ || is_pre)) ||
				c->type == BOX_BR)
			has_ਪਾਠ_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	ਲਈ (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->ਚੌੜਾਈ;
		ਜੇ (!layout_ਸਤਰ(c, &curwidth, &y, cx, cy + y, cont, ਪਹਿਲਾ_ਸਤਰ,
				has_ਪਾਠ_children, content, &ਅਗਲਾ))
			ਵਾਪਸ false;
		maxwidth = max(maxwidth,curwidth);
		c = ਅਗਲਾ;
		ਪਹਿਲਾ_ਸਤਰ = false;
	}

	inline_container->ਚੌੜਾਈ = maxwidth;
	inline_container->ਉਚਾਈ = y;

	ਵਾਪਸ true;
}


/* Documented in layout_intertnal.h */
bool layout_ਖੰਡ_context(
		ਬਣਤਰ box *ਖੰਡ,
		ਪੂਰਨ viewport_ਉਚਾਈ,
		html_content *content)
{
	ਬਣਤਰ box *box;
	ਪੂਰਨ cx, cy;  /**< current coordinates */
	ਪੂਰਨ max_pos_margin = 0;
	ਪੂਰਨ max_neg_margin = 0;
	ਪੂਰਨ y = 0;
	ਪੂਰਨ lm, rm;
	ਬਣਤਰ box *margin_collapse = ਨਿਰਾ;
	bool ਵਿੱਚ_margin = false;
	css_fixed gadget_ਮਾਪ;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	ਪ੍ਰਤਿਜ੍ਞਾ(ਖੰਡ->type == BOX_ਖੰਡ ||
			ਖੰਡ->type == BOX_INLINE_ਖੰਡ ||
			ਖੰਡ->type == BOX_ਸਾਰਣੀ_CELL ||
			ਖੰਡ->type == BOX_FLEX ||
			ਖੰਡ->type == BOX_INLINE_FLEX);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਖੰਡ->ਚੌੜਾਈ != UNKNOWN_ਚੌੜਾਈ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਖੰਡ->ਚੌੜਾਈ != AUTO);

	ਖੰਡ->float_children = ਨਿਰਾ;
	ਖੰਡ->cached_ਰੱਖੋ_ਹੇਠਾਂ_level = 0;
	ਖੰਡ->clear_level = 0;

	/* special case if the block contains an object */
	ਜੇ (ਖੰਡ->object) {
		ਪੂਰਨ ਆਰਜ਼ੀ_ਚੌੜਾਈ = ਖੰਡ->ਚੌੜਾਈ;
		ਜੇ (!layout_ਖੰਡ_object(ਖੰਡ))
			ਵਾਪਸ false;
		layout_get_object_dimensions(ਖੰਡ, &ਆਰਜ਼ੀ_ਚੌੜਾਈ,
				&ਖੰਡ->ਉਚਾਈ, INT_MIN, ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ,
				INT_MIN, ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ);
		ਵਾਪਸ true;
	} ਨਹੀਂਤਾਂ ਜੇ (ਖੰਡ->flags & REPLACE_DIM) {
		ਵਾਪਸ true;
	}

	/* special case if the block contains an radio button or checkbox */
	ਜੇ (ਖੰਡ->gadget && (ਖੰਡ->gadget->type == GADGET_RADIO ||
			ਖੰਡ->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_ਮਾਪ = INTTOFIX(1);
		ਜੇ (ਖੰਡ->ਉਚਾਈ == AUTO)
			ਖੰਡ->ਉਚਾਈ = FIXTOINT(css_unit_len2device_px(
					ਖੰਡ->style,
					&content->unit_len_ctx,
					gadget_ਮਾਪ, gadget_unit));
	}

	box = ਖੰਡ->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = ਖੰਡ->padding[ਸਿਖਰ_2];
	ਜੇ (box)
		box->y = ਖੰਡ->padding[ਸਿਖਰ_2];

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
	ਜਦੋਂ (box) {
		ਗਿਣਤੀ css_ਉਛਾਲ_e ਉਛਾਲ_x = CSS_ਉਛਾਲ_VISIBLE;
		ਗਿਣਤੀ css_ਉਛਾਲ_e ਉਛਾਲ_y = CSS_ਉਛਾਲ_VISIBLE;

		ਪ੍ਰਤਿਜ੍ਞਾ(box->type == BOX_ਖੰਡ ||
				box->type == BOX_FLEX ||
				box->type == BOX_ਸਾਰਣੀ ||
				box->type == BOX_INLINE_CONTAINER);

		/* Tables are laid out before being positioned, because the
		 * position depends on the width which is calculated in
		 * table layout. Blocks and inline containers are positioned
		 * before being laid out, because width is not dependent on
		 * content, and the position is required during layout for
		 * correct handling of floats.
		 */

		ਜੇ (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->ਮਾਪਾ->padding[ਖੱਬਾ_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			ਜਾਓ advance_ਮੰਜ਼ਿਲ_ਅਗਲਾ_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		ਜੇ (margin_collapse == ਨਿਰਾ) {
			margin_collapse = layout_ਅਗਲਾ_margin_ਖੰਡ(
					&content->unit_len_ctx, box, ਖੰਡ,
					viewport_ਉਚਾਈ,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			ਵਿੱਚ_margin = true;
		}

		/* Clearance. */
		y = 0;
		ਜੇ (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(ਖੰਡ->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		ਜੇ (box->style) {
			ਉਛਾਲ_x = css_computed_ਉਛਾਲ_x(box->style);
			ਉਛਾਲ_y = css_computed_ਉਛਾਲ_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		ਜੇ (box->type == BOX_FLEX ||
		    box->type == BOX_ਖੰਡ ||
		    box->flags & IFRAME) {
			ਜੇ (lh__box_is_object(box) == false &&
					box->style &&
					(ਉਛਾਲ_x != CSS_ਉਛਾਲ_VISIBLE ||
					 ਉਛਾਲ_y != CSS_ਉਛਾਲ_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				ਪੂਰਨ x0, x1, ਸਿਖਰ;
				ਬਣਤਰ box *ਖੱਬਾ, *ਸੱਜਾ;
				ਸਿਖਰ = cy + max_pos_margin - max_neg_margin;
				ਸਿਖਰ = (ਸਿਖਰ > y) ? ਸਿਖਰ : y;
				x0 = cx;
				x1 = cx + box->ਮਾਪਾ->ਚੌੜਾਈ -
						box->ਮਾਪਾ->padding[ਖੱਬਾ_2] -
						box->ਮਾਪਾ->padding[ਸੱਜਾ_2];
				ਲੱਭੋ_sides(ਖੰਡ->float_children, ਸਿਖਰ, ਸਿਖਰ,
						&x0, &x1, &ਖੱਬਾ, &ਸੱਜਾ);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->ਮਾਪਾ->ਚੌੜਾਈ -
						box->ਮਾਪਾ->padding[ਖੱਬਾ_2] -
						box->ਮਾਪਾ->padding[ਸੱਜਾ_2] -
						x1;
			}
			layout_ਖੰਡ_ਲੱਭੋ_dimensions(&content->unit_len_ctx,
					box->ਮਾਪਾ->ਚੌੜਾਈ,
					viewport_ਉਚਾਈ, lm, rm, box);
			ਜੇ (box->type == BOX_ਖੰਡ && !(box->flags & IFRAME)) {
				layout_ਖੰਡ_ਜੋੜੋ_scrollbar(box, ਸੱਜਾ_2);
				layout_ਖੰਡ_ਜੋੜੋ_scrollbar(box, BOTTOM);
			}
		} ਨਹੀਂਤਾਂ ਜੇ (box->type == BOX_ਸਾਰਣੀ) {
			ਜੇ (box->style != ਨਿਰਾ) {
				ਗਿਣਤੀ css_ਚੌੜਾਈ_e wtype;
				css_fixed ਚੌੜਾਈ = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_ਚੌੜਾਈ(box->style, &ਚੌੜਾਈ,
						&unit);

				ਜੇ (wtype == CSS_ਚੌੜਾਈ_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					ਪੂਰਨ x0, x1, ਸਿਖਰ;
					ਬਣਤਰ box *ਖੱਬਾ, *ਸੱਜਾ;
					ਸਿਖਰ = cy + max_pos_margin -
							max_neg_margin;
					ਸਿਖਰ = (ਸਿਖਰ > y) ? ਸਿਖਰ : y;
					x0 = cx;
					x1 = cx + box->ਮਾਪਾ->ਚੌੜਾਈ -
						box->ਮਾਪਾ->padding[ਖੱਬਾ_2] -
						box->ਮਾਪਾ->padding[ਸੱਜਾ_2];
					ਲੱਭੋ_sides(ਖੰਡ->float_children,
						ਸਿਖਰ, ਸਿਖਰ, &x0, &x1,
						&ਖੱਬਾ, &ਸੱਜਾ);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->ਮਾਪਾ->ਚੌੜਾਈ -
						box->ਮਾਪਾ->padding[ਖੱਬਾ_2] -
						box->ਮਾਪਾ->padding[ਸੱਜਾ_2] -
						x1;
				}
			}
			ਜੇ (!layout_ਸਾਰਣੀ(box, box->ਮਾਪਾ->ਚੌੜਾਈ - lm - rm,
					content))
				ਵਾਪਸ false;
			layout_solve_ਚੌੜਾਈ(box, box->ਮਾਪਾ->ਚੌੜਾਈ, box->ਚੌੜਾਈ,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->ਮਾਪਾ->padding[ਖੱਬਾ_2] + box->margin[ਖੱਬਾ_2] +
				box->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
		cx += box->x;

		/* Position box: vertical. */
		ਜੇ (box->border[ਸਿਖਰ_2].ਚੌੜਾਈ) {
			box->y += box->border[ਸਿਖਰ_2].ਚੌੜਾਈ;
			cy += box->border[ਸਿਖਰ_2].ਚੌੜਾਈ;
		}

		/* Vertical margin */
		ਜੇ (((box->type == BOX_ਖੰਡ && (box->flags & HAS_ਉਚਾਈ)) ||
		     box->type == BOX_FLEX ||
		     box->type == BOX_ਸਾਰਣੀ ||
		     (box->type == BOX_INLINE_CONTAINER &&
		      !box_is_ਪਹਿਲਾ_ਬੱਚਾ(box)) ||
		     margin_collapse == box) &&
		    ਵਿੱਚ_margin == true) {
			/* Margin goes above this box. */
			cy += max_pos_margin - max_neg_margin;
			box->y += max_pos_margin - max_neg_margin;

			/* Current margin has been applied. */
			ਵਿੱਚ_margin = false;
			max_pos_margin = max_neg_margin = 0;
		}

		/* Handle clearance */
		ਜੇ (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		ਜੇ (box->type == BOX_FLEX ||
				(box->type == BOX_ਖੰਡ && box->style &&
				 (ਉਛਾਲ_x != CSS_ਉਛਾਲ_VISIBLE ||
				  ਉਛਾਲ_y != CSS_ਉਛਾਲ_VISIBLE))) {

			ਜੇ (box->type == BOX_FLEX) {
				ਜੇ (!layout_flex(box, box->ਚੌੜਾਈ, content)) {
					ਵਾਪਸ false;
				}
			} ਨਹੀਂਤਾਂ {
				layout_ਖੰਡ_context(box,
						viewport_ਉਚਾਈ, content);
			}

			cy += box->padding[ਸਿਖਰ_2];

			ਜੇ (box->ਉਚਾਈ == AUTO) {
				box->ਉਚਾਈ = 0;
				layout_ਖੰਡ_ਜੋੜੋ_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->ਉਚਾਈ + box->padding[BOTTOM] +
					box->border[BOTTOM].ਚੌੜਾਈ;
			y = box->y + box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ +
					box->padding[BOTTOM] +
					box->border[BOTTOM].ਚੌੜਾਈ;

			/* Skip children, because they are done in the new
			 * block context */
			ਜਾਓ advance_ਮੰਜ਼ਿਲ_ਅਗਲਾ_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->ਚੌੜਾਈ);

		/* Layout (except tables). */
		ਜੇ (box->object) {
			ਜੇ (!layout_ਖੰਡ_object(box))
				ਵਾਪਸ false;

		} ਨਹੀਂਤਾਂ ਜੇ (box->type == BOX_INLINE_CONTAINER) {
			box->ਚੌੜਾਈ = box->ਮਾਪਾ->ਚੌੜਾਈ;
			ਜੇ (!layout_inline_container(box, box->ਚੌੜਾਈ, ਖੰਡ,
					cx, cy, content))
				ਵਾਪਸ false;

		} ਨਹੀਂਤਾਂ ਜੇ (box->type == BOX_ਸਾਰਣੀ) {
			/* Move down to avoid floats if necessary. */
			ਪੂਰਨ x0, x1;
			ਬਣਤਰ box *ਖੱਬਾ, *ਸੱਜਾ;
			y = cy;
			ਜਦੋਂ (1) {
				ਗਿਣਤੀ css_ਚੌੜਾਈ_e wtype;
				css_fixed ਚੌੜਾਈ = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_ਚੌੜਾਈ(box->style,
						&ਚੌੜਾਈ, &unit);

				x0 = cx;
				x1 = cx + box->ਮਾਪਾ->ਚੌੜਾਈ;
				ਲੱਭੋ_sides(ਖੰਡ->float_children, y,
						y + box->ਉਚਾਈ,
						&x0, &x1, &ਖੱਬਾ, &ਸੱਜਾ);
				ਜੇ (wtype == CSS_ਚੌੜਾਈ_AUTO)
					ਰੋਕੋ;
				ਜੇ (box->ਚੌੜਾਈ <= x1 - x0)
					ਰੋਕੋ;
				ਜੇ (!ਖੱਬਾ && !ਸੱਜਾ)
					ਰੋਕੋ;
				ਨਹੀਂਤਾਂ ਜੇ (!ਖੱਬਾ)
					y = ਸੱਜਾ->y + ਸੱਜਾ->ਉਚਾਈ + 1;
				ਨਹੀਂਤਾਂ ਜੇ (!ਸੱਜਾ)
					y = ਖੱਬਾ->y + ਖੱਬਾ->ਉਚਾਈ + 1;
				ਨਹੀਂਤਾਂ ਜੇ (ਖੱਬਾ->y + ਖੱਬਾ->ਉਚਾਈ <
						ਸੱਜਾ->y + ਸੱਜਾ->ਉਚਾਈ)
					y = ਖੱਬਾ->y + ਖੱਬਾ->ਉਚਾਈ + 1;
				ਨਹੀਂਤਾਂ
					y = ਸੱਜਾ->y + ਸੱਜਾ->ਉਚਾਈ + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		ਜੇ (box->type == BOX_ਖੰਡ && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			ਜੇ (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = ਨਿਰਾ;
			}

			y = box->padding[ਸਿਖਰ_2];
			box = box->children;
			box->y = y;
			cy += y;
			ਜਾਰੀ;
		} ਨਹੀਂਤਾਂ ਜੇ (box->type == BOX_ਖੰਡ || box->object ||
				box->flags & IFRAME)
			cy += box->padding[ਸਿਖਰ_2];

		ਜੇ (box->type == BOX_ਖੰਡ && box->ਉਚਾਈ == AUTO) {
			box->ਉਚਾਈ = 0;
			layout_ਖੰਡ_ਜੋੜੋ_scrollbar(box, BOTTOM);
		}

		cy += box->ਉਚਾਈ + box->padding[BOTTOM] +
				box->border[BOTTOM].ਚੌੜਾਈ;
		cx -= box->x;
		y = box->y + box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ +
				box->padding[BOTTOM] +
				box->border[BOTTOM].ਚੌੜਾਈ;

	advance_ਮੰਜ਼ਿਲ_ਅਗਲਾ_box:
		ਜੇ (!box->ਅਗਲਾ) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			ਕਰੋ {
				ਜੇ (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = ਨਿਰਾ;
				}

				/* Apply bottom margin */
				ਜੇ (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				ਨਹੀਂਤਾਂ ਜੇ (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->ਮਾਪਾ;
				ਜੇ (box == ਖੰਡ)
					ਰੋਕੋ;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				ਜੇ (box->type == BOX_ਖੰਡ &&
						box->flags & MAKE_ਉਚਾਈ) {
					margin_collapse = ਨਿਰਾ;
					ਵਿੱਚ_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				ਜੇ (box->ਉਚਾਈ == AUTO) {
					box->ਉਚਾਈ = y - box->padding[ਸਿਖਰ_2];

					ਜੇ (box->type == BOX_ਖੰਡ)
						layout_ਖੰਡ_ਜੋੜੋ_scrollbar(box,
								BOTTOM);
				} ਨਹੀਂਤਾਂ
					cy += box->ਉਚਾਈ -
							(y - box->padding[ਸਿਖਰ_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				ਜੇ (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_ਉਚਾਈ(
							&content->unit_len_ctx,
							box, ਨਿਰਾ)) {
					/* Height altered */
					/* Set current cy */
					cy += box->ਉਚਾਈ -
							(y - box->padding[ਸਿਖਰ_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].ਚੌੜਾਈ;
				cx -= box->x;
				y = box->y + box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ +
						box->padding[BOTTOM] +
						box->border[BOTTOM].ਚੌੜਾਈ;

			} ਜਦੋਂ (box->ਅਗਲਾ == ਨਿਰਾ);
			ਜੇ (box == ਖੰਡ)
				ਰੋਕੋ;
		}

		/* To next sibling. */

		ਜੇ (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = ਨਿਰਾ;
		}

		ਜੇ (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		ਨਹੀਂਤਾਂ ਜੇ (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->ਅਗਲਾ;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	ਲਈ (box = ਖੰਡ->float_children; box; box = box->ਅਗਲਾ_float) {
		y = box->y + box->ਉਚਾਈ + box->padding[BOTTOM] +
				box->border[BOTTOM].ਚੌੜਾਈ + box->margin[BOTTOM];
		ਜੇ (cy < y)
			cy = y;
	}

	ਜੇ (ਖੰਡ->ਉਚਾਈ == AUTO) {
		ਖੰਡ->ਉਚਾਈ = cy - ਖੰਡ->padding[ਸਿਖਰ_2];
		ਜੇ (ਖੰਡ->type == BOX_ਖੰਡ)
			layout_ਖੰਡ_ਜੋੜੋ_scrollbar(ਖੰਡ, BOTTOM);
	}

	ਜੇ (ਖੰਡ->style && css_computed_position(ਖੰਡ->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_ਉਚਾਈ(&content->unit_len_ctx, ਖੰਡ, ਨਿਰਾ);
	}

	ਜੇ (ਖੰਡ->gadget &&
			(ਖੰਡ->gadget->type == GADGET_TEXTAREA ||
			ਖੰਡ->gadget->type == GADGET_PASSWORD ||
			ਖੰਡ->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		ਪੂਰਨ ta_ਚੌੜਾਈ = ਖੰਡ->padding[ਖੱਬਾ_2] + ਖੰਡ->ਚੌੜਾਈ +
				ਖੰਡ->padding[ਸੱਜਾ_2];
		ਪੂਰਨ ta_ਉਚਾਈ = ਖੰਡ->padding[ਸਿਖਰ_2] + ਖੰਡ->ਉਚਾਈ +
				ਖੰਡ->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				ਖੰਡ->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_ਸੈੱਟ_layout(ਖੰਡ->gadget->ਅੰਕੜੇ.ਪਾਠ.ta,
				&fstyle, ta_ਚੌੜਾਈ, ta_ਉਚਾਈ,
				ਖੰਡ->padding[ਸਿਖਰ_2], ਖੰਡ->padding[ਸੱਜਾ_2],
				ਖੰਡ->padding[BOTTOM], ਖੰਡ->padding[ਖੱਬਾ_2]);
	}

	ਵਾਪਸ true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
ਸਥਿਰ bool
layout_get_element_ਟੈਗ(
		ਅਚਲ dom_ਗੰਢ *ਗੰਢ,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_ਗੰਢ_type ਗੰਢ_type;
	dom_exception exc;

	exc = dom_ਗੰਢ_get_ਗੰਢ_type(ਗੰਢ, &ਗੰਢ_type);
	ਜੇ (exc != DOM_NO_ERR ||
	    ਗੰਢ_type != DOM_ELEMENT_ਗੰਢ) {
		ਵਾਪਸ false;
	}

	exc = dom_html_element_get_ਟੈਗ_type(ਗੰਢ, &element_type);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ false;
	}

	*type = element_type;
	ਵਾਪਸ true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool
layout__check_element_type(
		ਅਚਲ dom_ਗੰਢ *ਗੰਢ,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	ਜੇ (!layout_get_element_ਟੈਗ(ਗੰਢ, &element_type)) {
		ਵਾਪਸ false;
	}

	ਵਾਪਸ element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ਸਥਿਰ bool
layout_get_li_ਮੁੱਲ(dom_ਗੰਢ *li_ਗੰਢ, dom_long *ਮੁੱਲ_ਬਾਹਰਲਾ)
{
	dom_exception exc;
	dom_long ਮੁੱਲ;
	bool has_ਮੁੱਲ;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_ਗੰਢ,
			corestring_dom_ਮੁੱਲ,
			&has_ਮੁੱਲ);
	ਜੇ (exc != DOM_NO_ERR || has_ਮੁੱਲ == false) {
		ਵਾਪਸ false;
	}

	exc = dom_html_li_element_get_ਮੁੱਲ(
			(dom_html_li_element *)li_ਗੰਢ,
			&ਮੁੱਲ);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ false;
	}

	*ਮੁੱਲ_ਬਾਹਰਲਾ = ਮੁੱਲ;
	ਵਾਪਸ true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ਸਥਿਰ bool
layout_get_ol_ਸ਼ੁਰੂ(dom_ਗੰਢ *ol_ਗੰਢ, dom_long *ਸ਼ੁਰੂ_ਬਾਹਰਲਾ)
{
	dom_exception exc;
	dom_long ਸ਼ੁਰੂ;
	bool has_ਸ਼ੁਰੂ;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_ਗੰਢ,
			corestring_dom_ਸ਼ੁਰੂ,
			&has_ਸ਼ੁਰੂ);
	ਜੇ (exc != DOM_NO_ERR || has_ਸ਼ੁਰੂ == false) {
		ਵਾਪਸ false;
	}

	exc = dom_html_olist_element_get_ਸ਼ੁਰੂ(
			(dom_html_olist_element *)ol_ਗੰਢ,
			&ਸ਼ੁਰੂ);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ false;
	}

	*ਸ਼ੁਰੂ_ਬਾਹਰਲਾ = ਸ਼ੁਰੂ;
	ਵਾਪਸ true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
ਸਥਿਰ bool
layout__get_ol_reversed(dom_ਗੰਢ *ol_ਗੰਢ)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_ਗੰਢ,
			corestring_dom_reversed,
			&has_reversed);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ false;
	}

	ਵਾਪਸ has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
ਸਥਿਰ bool
layout_get_list_ਵਸਤੂ_ਗਿਣਤੀ(
		dom_ਗੰਢ *list_owner, dom_long *ਗਿਣਤੀ_ਬਾਹਰਲਾ)
{
	dom_html_element_type ਟੈਗ_type;
	dom_exception exc;
	dom_ਗੰਢ *ਬੱਚਾ;
	ਪੂਰਨ ਗਿਣਤੀ_2;

	ਜੇ (list_owner == ਨਿਰਾ) {
		ਵਾਪਸ false;
	}

	ਜੇ (!layout_get_element_ਟੈਗ(list_owner, &ਟੈਗ_type)) {
		ਵਾਪਸ false;
	}

	ਜੇ (ਟੈਗ_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    ਟੈਗ_type != DOM_HTML_ELEMENT_TYPE_UL) {
		ਵਾਪਸ false;
	}

	exc = dom_ਗੰਢ_get_ਪਹਿਲਾ_ਬੱਚਾ(list_owner, &ਬੱਚਾ);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ false;
	}

	ਗਿਣਤੀ_2 = 0;
	ਜਦੋਂ (ਬੱਚਾ != ਨਿਰਾ) {
		dom_ਗੰਢ *ਆਰਜ਼ੀ_ਗੰਢ;

		ਜੇ (layout__check_element_type(ਬੱਚਾ,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ਬਣਤਰ box *ਬੱਚਾ_box;
			ਜੇ (dom_ਗੰਢ_get_user_ਅੰਕੜੇ(ਬੱਚਾ,
					corestring_dom_ns_ਕੁੰਜੀ_box_ਗੰਢ_ਅੰਕੜੇ,
					&ਬੱਚਾ_box) != DOM_NO_ERR) {
				dom_ਗੰਢ_unref(ਬੱਚਾ);
				ਵਾਪਸ false;
			}

			ਜੇ (ਬੱਚਾ_box != ਨਿਰਾ &&
			    ਬੱਚਾ_box->list_marker != ਨਿਰਾ) {
				ਗਿਣਤੀ_2++;
			}
		}

		exc = dom_ਗੰਢ_get_ਅਗਲਾ_sibling(ਬੱਚਾ, &ਆਰਜ਼ੀ_ਗੰਢ);
		dom_ਗੰਢ_unref(ਬੱਚਾ);
		ਜੇ (exc != DOM_NO_ERR) {
			ਵਾਪਸ false;
		}

		ਬੱਚਾ = ਆਰਜ਼ੀ_ਗੰਢ;
	}

	*ਗਿਣਤੀ_ਬਾਹਰਲਾ = ਗਿਣਤੀ_2;
	ਵਾਪਸ true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
ਸਥਿਰ ਖਾਲੀ
layout_ordered_list_ਗਿਣਤੀ(
		ਬਣਤਰ box *box)
{
	dom_html_element_type ਟੈਗ_type;
	dom_exception exc;
	dom_ਗੰਢ *ਬੱਚਾ;
	ਪੂਰਨ ਕਦਮ = 1;
	dom_long ਅਗਲਾ;

	ਜੇ (box->ਗੰਢ == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	ਜੇ (!layout_get_element_ਟੈਗ(box->ਗੰਢ, &ਟੈਗ_type)) {
		ਵਾਪਸ;
	}

	ਜੇ (ਟੈਗ_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    ਟੈਗ_type != DOM_HTML_ELEMENT_TYPE_UL) {
		ਵਾਪਸ;
	}

	ਅਗਲਾ = 1;
	ਜੇ (ਟੈਗ_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_ਸ਼ੁਰੂ = layout_get_ol_ਸ਼ੁਰੂ(box->ਗੰਢ, &ਅਗਲਾ);
		bool have_reversed = layout__get_ol_reversed(box->ਗੰਢ);

		ਜੇ (have_reversed) {
			ਕਦਮ = -1;
		}

		ਜੇ (!have_ਸ਼ੁਰੂ && have_reversed) {
			layout_get_list_ਵਸਤੂ_ਗਿਣਤੀ(box->ਗੰਢ, &ਅਗਲਾ);
		}
	}

	exc = dom_ਗੰਢ_get_ਪਹਿਲਾ_ਬੱਚਾ(box->ਗੰਢ, &ਬੱਚਾ);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ;
	}

	ਜਦੋਂ (ਬੱਚਾ != ਨਿਰਾ) {
		dom_ਗੰਢ *ਆਰਜ਼ੀ_ਗੰਢ;

		ਜੇ (layout__check_element_type(ਬੱਚਾ,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ਬਣਤਰ box *ਬੱਚਾ_box;

			ਜੇ (dom_ਗੰਢ_get_user_ਅੰਕੜੇ(ਬੱਚਾ,
					corestring_dom_ns_ਕੁੰਜੀ_box_ਗੰਢ_ਅੰਕੜੇ,
					&ਬੱਚਾ_box) != DOM_NO_ERR) {
				dom_ਗੰਢ_unref(ਬੱਚਾ);
				ਵਾਪਸ;
			}

			ਜੇ (ਬੱਚਾ_box != ਨਿਰਾ &&
			    ਬੱਚਾ_box->list_marker != ਨਿਰਾ) {
				dom_long ਮੁੱਲ;
				ਬਣਤਰ box *marker = ਬੱਚਾ_box->list_marker;
				ਜੇ (layout_get_li_ਮੁੱਲ(ਬੱਚਾ, &ਮੁੱਲ)) {
					marker->list_ਮੁੱਲ = ਮੁੱਲ;
					ਅਗਲਾ = marker->list_ਮੁੱਲ;
				} ਨਹੀਂਤਾਂ {
					marker->list_ਮੁੱਲ = ਅਗਲਾ;
				}
				ਅਗਲਾ += ਕਦਮ;
			}
		}

		exc = dom_ਗੰਢ_get_ਅਗਲਾ_sibling(ਬੱਚਾ, &ਆਰਜ਼ੀ_ਗੰਢ);
		dom_ਗੰਢ_unref(ਬੱਚਾ);
		ਜੇ (exc != DOM_NO_ERR) {
			ਵਾਪਸ;
		}

		ਬੱਚਾ = ਆਰਜ਼ੀ_ਗੰਢ;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
ਸਥਿਰ ਖਾਲੀ
layout_ਸੈੱਟ_numerical_marker_ਪਾਠ(
		ਅਚਲ html_content *content,
		ਬਣਤਰ box *box)
{
	ਬਣਤਰ box *marker = box->list_marker;
	size_t counter_len;
	css_error css_res;
	ਗਿਣਤੀ {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_ਮਾਪ = 20,
	};

	marker->ਪਾਠ = talloc_array(content->bctx, ਅੱਖਰ, LIST_MARKER_ਮਾਪ);
	ਜੇ (marker->ਪਾਠ == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_ਮੁੱਲ,
			marker->ਪਾਠ, LIST_MARKER_ਮਾਪ, &counter_len);
	ਜੇ (css_res == CSS_OK) {
		ਜੇ (counter_len > LIST_MARKER_ਮਾਪ) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->ਪਾਠ = talloc_realloc(content->bctx,
					marker->ਪਾਠ,
					ਅੱਖਰ,
					counter_len);
			ਜੇ (marker->ਪਾਠ == ਨਿਰਾ) {
				ਵਾਪਸ;
			}
			css_computed_format_list_style(box->style,
					marker->list_ਮੁੱਲ, marker->ਪਾਠ,
					counter_len, &counter_len);
		}
		marker->ਲੰਮਾਈ = counter_len;
	}
}

/**
 * Find out if box's style represents a numerical list style type.
 *
 * \param[in]  b  Box with style to test.
 * \return true if box has numerical list style type, false otherwise.
 */
ਸਥਿਰ bool
layout_list_ਵਸਤੂ_is_numerical(
		ਅਚਲ ਬਣਤਰ box *b)
{
	ਗਿਣਤੀ css_list_style_type_e t = css_computed_list_style_type(b->style);

	ਚੋਣ (t) {
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_ਵਰਗਫਲ: /* Fall through. */
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_NONE:
		ਵਾਪਸ false;

	ਮੂਲ:
		ਵਾਪਸ true;
	}
}

/**
 * Layout list markers.
 */
ਸਥਿਰ ਖਾਲੀ
layout_lists(ਅਚਲ html_content *content, ਬਣਤਰ box *box)
{
	ਬਣਤਰ box *ਬੱਚਾ;

	layout_ordered_list_ਗਿਣਤੀ(box);

	ਲਈ (ਬੱਚਾ = box->children; ਬੱਚਾ; ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ) {
		ਜੇ (ਬੱਚਾ->list_marker) {
			ਬਣਤਰ box *marker = ਬੱਚਾ->list_marker;

			ਜੇ (layout_list_ਵਸਤੂ_is_numerical(ਬੱਚਾ)) {
				ਜੇ (marker->ਪਾਠ == ਨਿਰਾ) {
					layout_ਸੈੱਟ_numerical_marker_ਪਾਠ(
							content, ਬੱਚਾ);
				}
			}
			ਜੇ (marker->object) {
				marker->ਚੌੜਾਈ =
					content_get_ਚੌੜਾਈ(marker->object);
				marker->x = -marker->ਚੌੜਾਈ;
				marker->ਉਚਾਈ =
					content_get_ਉਚਾਈ(marker->object);
				marker->y = (ਸਤਰ_ਉਚਾਈ(
						&content->unit_len_ctx,
						marker->style) -
						marker->ਉਚਾਈ) / 2;
			} ਨਹੀਂਤਾਂ ਜੇ (marker->ਪਾਠ) {
				ਜੇ (marker->ਚੌੜਾਈ == UNKNOWN_ਚੌੜਾਈ) {
					plot_font_style_t fstyle;
					font_plot_style_from_css(
							&content->unit_len_ctx,
							marker->style,
							&fstyle);
					content->font_func->ਚੌੜਾਈ(&fstyle,
							marker->ਪਾਠ,
							marker->ਲੰਮਾਈ,
							&marker->ਚੌੜਾਈ);
					marker->flags |= MEASURED;
				}
				marker->x = -marker->ਚੌੜਾਈ;
				marker->y = 0;
				marker->ਉਚਾਈ = ਸਤਰ_ਉਚਾਈ(
						&content->unit_len_ctx,
						marker->style);
			} ਨਹੀਂਤਾਂ {
				marker->x = 0;
				marker->y = 0;
				marker->ਚੌੜਾਈ = 0;
				marker->ਉਚਾਈ = 0;
			}
			/* Gap between marker and content */
			marker->x -= 4;
		}
		layout_lists(content, ਬੱਚਾ);
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
ਸਥਿਰ ਖਾਲੀ
layout_compute_offsets(ਅਚਲ css_unit_ctx *unit_len_ctx,
		       ਬਣਤਰ box *box,
		       ਬਣਤਰ box *containing_ਖੰਡ,
		       ਪੂਰਨ *ਸਿਖਰ,
		       ਪੂਰਨ *ਸੱਜਾ,
		       ਪੂਰਨ *bottom,
		       ਪੂਰਨ *ਖੱਬਾ)
{
	uint32_t type;
	css_fixed ਮੁੱਲ = 0;
	css_unit unit = CSS_UNIT_PX;

	ਪ੍ਰਤਿਜ੍ਞਾ(containing_ਖੰਡ->ਚੌੜਾਈ != UNKNOWN_ਚੌੜਾਈ);
	ਪ੍ਰਤਿਜ੍ਞਾ(containing_ਖੰਡ->ਚੌੜਾਈ != AUTO);
	ਪ੍ਰਤਿਜ੍ਞਾ(containing_ਖੰਡ->ਉਚਾਈ != AUTO);

	/*ਖੱਬਾ*/
	type = css_computed_ਖੱਬਾ(box->style, &ਮੁੱਲ, &unit);
	ਜੇ (type == CSS_ਖੱਬਾ_ਸੈੱਟ) {
		ਜੇ (unit == CSS_UNIT_PCT) {
			*ਖੱਬਾ = FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ,
					containing_ਖੰਡ->ਚੌੜਾਈ);
		} ਨਹੀਂਤਾਂ {
			*ਖੱਬਾ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ਮੁੱਲ, unit));
		}
	} ਨਹੀਂਤਾਂ {
		*ਖੱਬਾ = AUTO;
	}

	/*ਸੱਜਾ*/
	type = css_computed_ਸੱਜਾ(box->style, &ਮੁੱਲ, &unit);
	ਜੇ (type == CSS_ਸੱਜਾ_ਸੈੱਟ) {
		ਜੇ (unit == CSS_UNIT_PCT) {
			*ਸੱਜਾ = FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ,
					containing_ਖੰਡ->ਚੌੜਾਈ);
		} ਨਹੀਂਤਾਂ {
			*ਸੱਜਾ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ਮੁੱਲ, unit));
		}
	} ਨਹੀਂਤਾਂ {
		*ਸੱਜਾ = AUTO;
	}

	/*ਸਿਖਰ*/
	type = css_computed_ਸਿਖਰ(box->style, &ਮੁੱਲ, &unit);
	ਜੇ (type == CSS_ਸਿਖਰ_ਸੈੱਟ) {
		ਜੇ (unit == CSS_UNIT_PCT) {
			*ਸਿਖਰ = FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ,
					containing_ਖੰਡ->ਉਚਾਈ);
		} ਨਹੀਂਤਾਂ {
			*ਸਿਖਰ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ਮੁੱਲ, unit));
		}
	} ਨਹੀਂਤਾਂ {
		*ਸਿਖਰ = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &ਮੁੱਲ, &unit);
	ਜੇ (type == CSS_BOTTOM_ਸੈੱਟ) {
		ਜੇ (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_ਦਾ_INT_TOINT(ਮੁੱਲ,
					containing_ਖੰਡ->ਉਚਾਈ);
		} ਨਹੀਂਤਾਂ {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ਮੁੱਲ, unit));
		}
	} ਨਹੀਂਤਾਂ {
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
ਸਥਿਰ bool
layout_absolute(ਬਣਤਰ box *box,
		ਬਣਤਰ box *containing_ਖੰਡ,
		ਪੂਰਨ cx, ਪੂਰਨ cy,
		html_content *content)
{
	ਪੂਰਨ static_ਖੱਬਾ, static_ਸਿਖਰ;  /* static position */
	ਪੂਰਨ ਸਿਖਰ, ਸੱਜਾ, bottom, ਖੱਬਾ;
	ਪੂਰਨ ਚੌੜਾਈ, ਉਚਾਈ, max_ਚੌੜਾਈ, min_ਚੌੜਾਈ;
	ਪੂਰਨ *margin = box->margin;
	ਪੂਰਨ *padding = box->padding;
	ਬਣਤਰ box_border *border = box->border;
	ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ = containing_ਖੰਡ->ਚੌੜਾਈ;
	ਪੂਰਨ space;

	ਪ੍ਰਤਿਜ੍ਞਾ(box->type == BOX_ਖੰਡ || box->type == BOX_ਸਾਰਣੀ ||
			box->type == BOX_INLINE_ਖੰਡ ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_ਖੱਬਾ = cx + box->x;
	static_ਸਿਖਰ = cy + box->y;

	ਜੇ (containing_ਖੰਡ->type == BOX_ਖੰਡ ||
			containing_ਖੰਡ->type == BOX_INLINE_ਖੰਡ ||
			containing_ਖੰਡ->type == BOX_ਸਾਰਣੀ_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_ਖੰਡ->ਚੌੜਾਈ += containing_ਖੰਡ->padding[ਖੱਬਾ_2] +
				containing_ਖੰਡ->padding[ਸੱਜਾ_2];
		containing_ਖੰਡ->ਉਚਾਈ += containing_ਖੰਡ->padding[ਸਿਖਰ_2] +
				containing_ਖੰਡ->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_ਖੰਡ,
			&ਸਿਖਰ, &ਸੱਜਾ, &bottom, &ਖੱਬਾ);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_ਖੰਡ;
	layout_ਲੱਭੋ_dimensions(&content->unit_len_ctx, ਉਪਲਬਧ_ਚੌੜਾਈ, -1,
			box, box->style, &ਚੌੜਾਈ, &ਉਚਾਈ,
			&max_ਚੌੜਾਈ, &min_ਚੌੜਾਈ, 0, 0,
			margin, padding, border);
	box->float_container = ਨਿਰਾ;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ਖੱਬਾ, margin[ਖੱਬਾ_2], border[ਖੱਬਾ_2].ਚੌੜਾਈ, padding[ਖੱਬਾ_2], ਚੌੜਾਈ,
	      padding[ਸੱਜਾ_2], border[ਸੱਜਾ_2].ਚੌੜਾਈ, margin[ਸੱਜਾ_2], ਸੱਜਾ,
	      containing_ਖੰਡ->ਚੌੜਾਈ);


	ਜੇ (ਖੱਬਾ == AUTO && ਚੌੜਾਈ == AUTO && ਸੱਜਾ == AUTO) {
		ਜੇ (margin[ਖੱਬਾ_2] == AUTO)
			margin[ਖੱਬਾ_2] = 0;
		ਜੇ (margin[ਸੱਜਾ_2] == AUTO)
			margin[ਸੱਜਾ_2] = 0;
		ਖੱਬਾ = static_ਖੱਬਾ;

		ਚੌੜਾਈ = min(max(box->min_ਚੌੜਾਈ, ਉਪਲਬਧ_ਚੌੜਾਈ),
			box->max_ਚੌੜਾਈ);
		ਚੌੜਾਈ -= box->margin[ਖੱਬਾ_2] + box->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
			box->padding[ਖੱਬਾ_2] + box->padding[ਸੱਜਾ_2] +
			box->border[ਸੱਜਾ_2].ਚੌੜਾਈ + box->margin[ਸੱਜਾ_2];

		/* Adjust for {min|max}-width */
		ਜੇ (max_ਚੌੜਾਈ >= 0 && ਚੌੜਾਈ > max_ਚੌੜਾਈ) ਚੌੜਾਈ = max_ਚੌੜਾਈ;
		ਜੇ (ਚੌੜਾਈ < min_ਚੌੜਾਈ) ਚੌੜਾਈ = min_ਚੌੜਾਈ;

		ਸੱਜਾ = containing_ਖੰਡ->ਚੌੜਾਈ -
			ਖੱਬਾ -
			margin[ਖੱਬਾ_2] - border[ਖੱਬਾ_2].ਚੌੜਾਈ - padding[ਖੱਬਾ_2] -
			ਚੌੜਾਈ -
			padding[ਸੱਜਾ_2] - border[ਸੱਜਾ_2].ਚੌੜਾਈ - margin[ਸੱਜਾ_2];
	} ਨਹੀਂਤਾਂ ਜੇ (ਖੱਬਾ != AUTO && ਚੌੜਾਈ != AUTO && ਸੱਜਾ != AUTO) {

		/* Adjust for {min|max}-width */
		ਜੇ (max_ਚੌੜਾਈ >= 0 && ਚੌੜਾਈ > max_ਚੌੜਾਈ) ਚੌੜਾਈ = max_ਚੌੜਾਈ;
		ਜੇ (min_ਚੌੜਾਈ >  0 && ਚੌੜਾਈ < min_ਚੌੜਾਈ) ਚੌੜਾਈ = min_ਚੌੜਾਈ;

		ਜੇ (margin[ਖੱਬਾ_2] == AUTO && margin[ਸੱਜਾ_2] == AUTO) {
			space = containing_ਖੰਡ->ਚੌੜਾਈ -
					ਖੱਬਾ - border[ਖੱਬਾ_2].ਚੌੜਾਈ -
					padding[ਖੱਬਾ_2] - ਚੌੜਾਈ - padding[ਸੱਜਾ_2] -
					border[ਸੱਜਾ_2].ਚੌੜਾਈ - ਸੱਜਾ;
			ਜੇ (space < 0) {
				margin[ਖੱਬਾ_2] = 0;
				margin[ਸੱਜਾ_2] = space;
			} ਨਹੀਂਤਾਂ {
				margin[ਖੱਬਾ_2] = margin[ਸੱਜਾ_2] = space / 2;
			}
		} ਨਹੀਂਤਾਂ ਜੇ (margin[ਖੱਬਾ_2] == AUTO) {
			margin[ਖੱਬਾ_2] = containing_ਖੰਡ->ਚੌੜਾਈ -
					ਖੱਬਾ - border[ਖੱਬਾ_2].ਚੌੜਾਈ -
					padding[ਖੱਬਾ_2] - ਚੌੜਾਈ - padding[ਸੱਜਾ_2] -
					border[ਸੱਜਾ_2].ਚੌੜਾਈ - margin[ਸੱਜਾ_2] -
					ਸੱਜਾ;
		} ਨਹੀਂਤਾਂ ਜੇ (margin[ਸੱਜਾ_2] == AUTO) {
			margin[ਸੱਜਾ_2] = containing_ਖੰਡ->ਚੌੜਾਈ -
					ਖੱਬਾ - margin[ਖੱਬਾ_2] -
					border[ਖੱਬਾ_2].ਚੌੜਾਈ -
					padding[ਖੱਬਾ_2] - ਚੌੜਾਈ - padding[ਸੱਜਾ_2] -
					border[ਸੱਜਾ_2].ਚੌੜਾਈ - ਸੱਜਾ;
		} ਨਹੀਂਤਾਂ {
			ਸੱਜਾ = containing_ਖੰਡ->ਚੌੜਾਈ -
					ਖੱਬਾ - margin[ਖੱਬਾ_2] -
					border[ਖੱਬਾ_2].ਚੌੜਾਈ -
					padding[ਖੱਬਾ_2] - ਚੌੜਾਈ - padding[ਸੱਜਾ_2] -
					border[ਸੱਜਾ_2].ਚੌੜਾਈ - margin[ਸੱਜਾ_2];
		}
	} ਨਹੀਂਤਾਂ {
		ਜੇ (margin[ਖੱਬਾ_2] == AUTO)
			margin[ਖੱਬਾ_2] = 0;
		ਜੇ (margin[ਸੱਜਾ_2] == AUTO)
			margin[ਸੱਜਾ_2] = 0;

		ਜੇ (ਖੱਬਾ == AUTO && ਚੌੜਾਈ == AUTO && ਸੱਜਾ != AUTO) {
			ਉਪਲਬਧ_ਚੌੜਾਈ -= ਸੱਜਾ;

			ਚੌੜਾਈ = min(max(box->min_ਚੌੜਾਈ, ਉਪਲਬਧ_ਚੌੜਾਈ),
				box->max_ਚੌੜਾਈ);
			ਚੌੜਾਈ -= box->margin[ਖੱਬਾ_2] + box->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
				box->padding[ਖੱਬਾ_2] + box->padding[ਸੱਜਾ_2] +
				box->border[ਸੱਜਾ_2].ਚੌੜਾਈ + box->margin[ਸੱਜਾ_2];

			/* Adjust for {min|max}-width */
			ਜੇ (max_ਚੌੜਾਈ >= 0 && ਚੌੜਾਈ > max_ਚੌੜਾਈ)
				ਚੌੜਾਈ = max_ਚੌੜਾਈ;
			ਜੇ (ਚੌੜਾਈ < min_ਚੌੜਾਈ)
				ਚੌੜਾਈ = min_ਚੌੜਾਈ;

			ਖੱਬਾ = containing_ਖੰਡ->ਚੌੜਾਈ -
					margin[ਖੱਬਾ_2] - border[ਖੱਬਾ_2].ਚੌੜਾਈ -
					padding[ਖੱਬਾ_2] - ਚੌੜਾਈ - padding[ਸੱਜਾ_2] -
					border[ਸੱਜਾ_2].ਚੌੜਾਈ - margin[ਸੱਜਾ_2] -
					ਸੱਜਾ;
		} ਨਹੀਂਤਾਂ ਜੇ (ਖੱਬਾ == AUTO && ਚੌੜਾਈ != AUTO && ਸੱਜਾ == AUTO) {

			/* Adjust for {min|max}-width */
			ਜੇ (max_ਚੌੜਾਈ >= 0 && ਚੌੜਾਈ > max_ਚੌੜਾਈ)
				ਚੌੜਾਈ = max_ਚੌੜਾਈ;
			ਜੇ (min_ਚੌੜਾਈ >  0 && ਚੌੜਾਈ < min_ਚੌੜਾਈ)
				ਚੌੜਾਈ = min_ਚੌੜਾਈ;

			ਖੱਬਾ = static_ਖੱਬਾ;
			ਸੱਜਾ = containing_ਖੰਡ->ਚੌੜਾਈ -
					ਖੱਬਾ - margin[ਖੱਬਾ_2] -
					border[ਖੱਬਾ_2].ਚੌੜਾਈ -
					padding[ਖੱਬਾ_2] - ਚੌੜਾਈ - padding[ਸੱਜਾ_2] -
					border[ਸੱਜਾ_2].ਚੌੜਾਈ - margin[ਸੱਜਾ_2];
		} ਨਹੀਂਤਾਂ ਜੇ (ਖੱਬਾ != AUTO && ਚੌੜਾਈ == AUTO && ਸੱਜਾ == AUTO) {
			ਉਪਲਬਧ_ਚੌੜਾਈ -= ਖੱਬਾ;

			ਚੌੜਾਈ = min(max(box->min_ਚੌੜਾਈ, ਉਪਲਬਧ_ਚੌੜਾਈ),
				box->max_ਚੌੜਾਈ);
			ਚੌੜਾਈ -= box->margin[ਖੱਬਾ_2] + box->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
				box->padding[ਖੱਬਾ_2] + box->padding[ਸੱਜਾ_2] +
				box->border[ਸੱਜਾ_2].ਚੌੜਾਈ + box->margin[ਸੱਜਾ_2];

			/* Adjust for {min|max}-width */
			ਜੇ (max_ਚੌੜਾਈ >= 0 && ਚੌੜਾਈ > max_ਚੌੜਾਈ)
				ਚੌੜਾਈ = max_ਚੌੜਾਈ;
			ਜੇ (ਚੌੜਾਈ < min_ਚੌੜਾਈ)
				ਚੌੜਾਈ = min_ਚੌੜਾਈ;

			ਸੱਜਾ = containing_ਖੰਡ->ਚੌੜਾਈ -
					ਖੱਬਾ - margin[ਖੱਬਾ_2] -
					border[ਖੱਬਾ_2].ਚੌੜਾਈ -
					padding[ਖੱਬਾ_2] - ਚੌੜਾਈ - padding[ਸੱਜਾ_2] -
					border[ਸੱਜਾ_2].ਚੌੜਾਈ - margin[ਸੱਜਾ_2];
		} ਨਹੀਂਤਾਂ ਜੇ (ਖੱਬਾ == AUTO && ਚੌੜਾਈ != AUTO && ਸੱਜਾ != AUTO) {

			/* Adjust for {min|max}-width */
			ਜੇ (max_ਚੌੜਾਈ >= 0 && ਚੌੜਾਈ > max_ਚੌੜਾਈ)
				ਚੌੜਾਈ = max_ਚੌੜਾਈ;
			ਜੇ (ਚੌੜਾਈ < min_ਚੌੜਾਈ)
				ਚੌੜਾਈ = min_ਚੌੜਾਈ;

			ਖੱਬਾ = containing_ਖੰਡ->ਚੌੜਾਈ -
					margin[ਖੱਬਾ_2] - border[ਖੱਬਾ_2].ਚੌੜਾਈ -
					padding[ਖੱਬਾ_2] - ਚੌੜਾਈ - padding[ਸੱਜਾ_2] -
					border[ਸੱਜਾ_2].ਚੌੜਾਈ - margin[ਸੱਜਾ_2] -
					ਸੱਜਾ;
		} ਨਹੀਂਤਾਂ ਜੇ (ਖੱਬਾ != AUTO && ਚੌੜਾਈ == AUTO && ਸੱਜਾ != AUTO) {
			ਚੌੜਾਈ = containing_ਖੰਡ->ਚੌੜਾਈ -
					ਖੱਬਾ - margin[ਖੱਬਾ_2] -
					border[ਖੱਬਾ_2].ਚੌੜਾਈ -
					padding[ਖੱਬਾ_2] - padding[ਸੱਜਾ_2] -
					border[ਸੱਜਾ_2].ਚੌੜਾਈ - margin[ਸੱਜਾ_2] -
					ਸੱਜਾ;

			/* Adjust for {min|max}-width */
			ਜੇ (max_ਚੌੜਾਈ >= 0 && ਚੌੜਾਈ > max_ਚੌੜਾਈ)
				ਚੌੜਾਈ = max_ਚੌੜਾਈ;
			ਜੇ (ਚੌੜਾਈ < min_ਚੌੜਾਈ)
				ਚੌੜਾਈ = min_ਚੌੜਾਈ;

		} ਨਹੀਂਤਾਂ ਜੇ (ਖੱਬਾ != AUTO && ਚੌੜਾਈ != AUTO && ਸੱਜਾ == AUTO) {

			/* Adjust for {min|max}-width */
			ਜੇ (max_ਚੌੜਾਈ >= 0 && ਚੌੜਾਈ > max_ਚੌੜਾਈ)
				ਚੌੜਾਈ = max_ਚੌੜਾਈ;
			ਜੇ (ਚੌੜਾਈ < min_ਚੌੜਾਈ)
				ਚੌੜਾਈ = min_ਚੌੜਾਈ;

			ਸੱਜਾ = containing_ਖੰਡ->ਚੌੜਾਈ -
					ਖੱਬਾ - margin[ਖੱਬਾ_2] -
					border[ਖੱਬਾ_2].ਚੌੜਾਈ -
					padding[ਖੱਬਾ_2] - ਚੌੜਾਈ - padding[ਸੱਜਾ_2] -
					border[ਸੱਜਾ_2].ਚੌੜਾਈ - margin[ਸੱਜਾ_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ਖੱਬਾ, margin[ਖੱਬਾ_2], border[ਖੱਬਾ_2].ਚੌੜਾਈ, padding[ਖੱਬਾ_2], ਚੌੜਾਈ,
	      padding[ਸੱਜਾ_2], border[ਸੱਜਾ_2].ਚੌੜਾਈ, margin[ਸੱਜਾ_2], ਸੱਜਾ,
	      containing_ਖੰਡ->ਚੌੜਾਈ);

	box->x = ਖੱਬਾ + margin[ਖੱਬਾ_2] + border[ਖੱਬਾ_2].ਚੌੜਾਈ - cx;
	ਜੇ (containing_ਖੰਡ->type == BOX_ਖੰਡ ||
			containing_ਖੰਡ->type == BOX_INLINE_ਖੰਡ ||
			containing_ਖੰਡ->type == BOX_ਸਾਰਣੀ_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_ਖੰਡ->ਚੌੜਾਈ -= containing_ਖੰਡ->padding[ਖੱਬਾ_2] +
				containing_ਖੰਡ->padding[ਸੱਜਾ_2];
	} ਨਹੀਂਤਾਂ {
		/** \todo inline ancestors */
	}
	box->ਚੌੜਾਈ = ਚੌੜਾਈ;
	box->ਉਚਾਈ = ਉਚਾਈ;

	ਜੇ (box->type == BOX_ਖੰਡ || box->type == BOX_INLINE_ਖੰਡ ||
			box->object || box->flags & IFRAME) {
		ਜੇ (!layout_ਖੰਡ_context(box, -1, content))
			ਵਾਪਸ false;
	} ਨਹੀਂਤਾਂ ਜੇ (box->type == BOX_ਸਾਰਣੀ) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_ਖੰਡ;
		/* \todo  layout_table considers margins etc. again */
		ਜੇ (!layout_ਸਾਰਣੀ(box, ਚੌੜਾਈ, content))
			ਵਾਪਸ false;
		box->float_container = ਨਿਰਾ;
		layout_solve_ਚੌੜਾਈ(box, box->ਮਾਪਾ->ਚੌੜਾਈ, box->ਚੌੜਾਈ, 0, 0,
				-1, -1);
	} ਨਹੀਂਤਾਂ ਜੇ (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_ਖੰਡ;
		ਜੇ (!layout_flex(box, ਚੌੜਾਈ, content))
			ਵਾਪਸ false;
		box->float_container = ਨਿਰਾ;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ਸਿਖਰ, margin[ਸਿਖਰ_2], border[ਸਿਖਰ_2].ਚੌੜਾਈ, padding[ਸਿਖਰ_2], ਉਚਾਈ,
	      padding[BOTTOM], border[BOTTOM].ਚੌੜਾਈ, margin[BOTTOM], bottom,
	      containing_ਖੰਡ->ਉਚਾਈ);

	ਜੇ (ਸਿਖਰ == AUTO && ਉਚਾਈ == AUTO && bottom == AUTO) {
		ਸਿਖਰ = static_ਸਿਖਰ;
		ਉਚਾਈ = box->ਉਚਾਈ;
		ਜੇ (margin[ਸਿਖਰ_2] == AUTO)
			margin[ਸਿਖਰ_2] = 0;
		ਜੇ (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_ਖੰਡ->ਉਚਾਈ -
				ਸਿਖਰ - margin[ਸਿਖਰ_2] - border[ਸਿਖਰ_2].ਚੌੜਾਈ -
				padding[ਸਿਖਰ_2] - ਉਚਾਈ - padding[BOTTOM] -
				border[BOTTOM].ਚੌੜਾਈ - margin[BOTTOM];
	} ਨਹੀਂਤਾਂ ਜੇ (ਸਿਖਰ != AUTO && ਉਚਾਈ != AUTO && bottom != AUTO) {
		ਜੇ (margin[ਸਿਖਰ_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_ਖੰਡ->ਉਚਾਈ -
					ਸਿਖਰ - border[ਸਿਖਰ_2].ਚੌੜਾਈ - padding[ਸਿਖਰ_2] -
					ਉਚਾਈ - padding[BOTTOM] -
					border[BOTTOM].ਚੌੜਾਈ - bottom;
			margin[ਸਿਖਰ_2] = margin[BOTTOM] = space / 2;
		} ਨਹੀਂਤਾਂ ਜੇ (margin[ਸਿਖਰ_2] == AUTO) {
			margin[ਸਿਖਰ_2] = containing_ਖੰਡ->ਉਚਾਈ -
					ਸਿਖਰ - border[ਸਿਖਰ_2].ਚੌੜਾਈ - padding[ਸਿਖਰ_2] -
					ਉਚਾਈ - padding[BOTTOM] -
					border[BOTTOM].ਚੌੜਾਈ - margin[BOTTOM] -
					bottom;
		} ਨਹੀਂਤਾਂ ਜੇ (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_ਖੰਡ->ਉਚਾਈ -
					ਸਿਖਰ - margin[ਸਿਖਰ_2] - border[ਸਿਖਰ_2].ਚੌੜਾਈ -
					padding[ਸਿਖਰ_2] - ਉਚਾਈ -
					padding[BOTTOM] - border[BOTTOM].ਚੌੜਾਈ -
					bottom;
		} ਨਹੀਂਤਾਂ {
			bottom = containing_ਖੰਡ->ਉਚਾਈ -
					ਸਿਖਰ - margin[ਸਿਖਰ_2] - border[ਸਿਖਰ_2].ਚੌੜਾਈ -
					padding[ਸਿਖਰ_2] - ਉਚਾਈ -
					padding[BOTTOM] - border[BOTTOM].ਚੌੜਾਈ -
					margin[BOTTOM];
		}
	} ਨਹੀਂਤਾਂ {
		ਜੇ (margin[ਸਿਖਰ_2] == AUTO)
			margin[ਸਿਖਰ_2] = 0;
		ਜੇ (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		ਜੇ (ਸਿਖਰ == AUTO && ਉਚਾਈ == AUTO && bottom != AUTO) {
			ਉਚਾਈ = box->ਉਚਾਈ;
			ਸਿਖਰ = containing_ਖੰਡ->ਉਚਾਈ -
					margin[ਸਿਖਰ_2] - border[ਸਿਖਰ_2].ਚੌੜਾਈ -
					padding[ਸਿਖਰ_2] - ਉਚਾਈ -
					padding[BOTTOM] - border[BOTTOM].ਚੌੜਾਈ -
					margin[BOTTOM] - bottom;
		} ਨਹੀਂਤਾਂ ਜੇ (ਸਿਖਰ == AUTO && ਉਚਾਈ != AUTO && bottom == AUTO) {
			ਸਿਖਰ = static_ਸਿਖਰ;
			bottom = containing_ਖੰਡ->ਉਚਾਈ -
					ਸਿਖਰ - margin[ਸਿਖਰ_2] - border[ਸਿਖਰ_2].ਚੌੜਾਈ -
					padding[ਸਿਖਰ_2] - ਉਚਾਈ -
					padding[BOTTOM] - border[BOTTOM].ਚੌੜਾਈ -
					margin[BOTTOM];
		} ਨਹੀਂਤਾਂ ਜੇ (ਸਿਖਰ != AUTO && ਉਚਾਈ == AUTO && bottom == AUTO) {
			ਉਚਾਈ = box->ਉਚਾਈ;
			bottom = containing_ਖੰਡ->ਉਚਾਈ -
					ਸਿਖਰ - margin[ਸਿਖਰ_2] - border[ਸਿਖਰ_2].ਚੌੜਾਈ -
					padding[ਸਿਖਰ_2] - ਉਚਾਈ -
					padding[BOTTOM] - border[BOTTOM].ਚੌੜਾਈ -
					margin[BOTTOM];
		} ਨਹੀਂਤਾਂ ਜੇ (ਸਿਖਰ == AUTO && ਉਚਾਈ != AUTO && bottom != AUTO) {
			ਸਿਖਰ = containing_ਖੰਡ->ਉਚਾਈ -
					margin[ਸਿਖਰ_2] - border[ਸਿਖਰ_2].ਚੌੜਾਈ -
					padding[ਸਿਖਰ_2] - ਉਚਾਈ -
					padding[BOTTOM] - border[BOTTOM].ਚੌੜਾਈ -
					margin[BOTTOM] - bottom;
		} ਨਹੀਂਤਾਂ ਜੇ (ਸਿਖਰ != AUTO && ਉਚਾਈ == AUTO && bottom != AUTO) {
			ਉਚਾਈ = containing_ਖੰਡ->ਉਚਾਈ -
					ਸਿਖਰ - margin[ਸਿਖਰ_2] - border[ਸਿਖਰ_2].ਚੌੜਾਈ -
					padding[ਸਿਖਰ_2] - padding[BOTTOM] -
					border[BOTTOM].ਚੌੜਾਈ - margin[BOTTOM] -
					bottom;
		} ਨਹੀਂਤਾਂ ਜੇ (ਸਿਖਰ != AUTO && ਉਚਾਈ != AUTO && bottom == AUTO) {
			bottom = containing_ਖੰਡ->ਉਚਾਈ -
					ਸਿਖਰ - margin[ਸਿਖਰ_2] - border[ਸਿਖਰ_2].ਚੌੜਾਈ -
					padding[ਸਿਖਰ_2] - ਉਚਾਈ -
					padding[BOTTOM] - border[BOTTOM].ਚੌੜਾਈ -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ਸਿਖਰ, margin[ਸਿਖਰ_2], border[ਸਿਖਰ_2].ਚੌੜਾਈ, padding[ਸਿਖਰ_2], ਉਚਾਈ,
	      padding[BOTTOM], border[BOTTOM].ਚੌੜਾਈ, margin[BOTTOM], bottom,
	      containing_ਖੰਡ->ਉਚਾਈ);

	box->y = ਸਿਖਰ + margin[ਸਿਖਰ_2] + border[ਸਿਖਰ_2].ਚੌੜਾਈ - cy;
	ਜੇ (containing_ਖੰਡ->type == BOX_ਖੰਡ ||
			containing_ਖੰਡ->type == BOX_INLINE_ਖੰਡ ||
			containing_ਖੰਡ->type == BOX_ਸਾਰਣੀ_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_ਖੰਡ->ਉਚਾਈ -= containing_ਖੰਡ->padding[ਸਿਖਰ_2] +
				containing_ਖੰਡ->padding[BOTTOM];
	} ਨਹੀਂਤਾਂ {
		/** \todo Inline ancestors */
	}
	box->ਉਚਾਈ = ਉਚਾਈ;
	layout_apply_minmax_ਉਚਾਈ(&content->unit_len_ctx, box, containing_ਖੰਡ);

	ਵਾਪਸ true;
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
ਸਥਿਰ bool
layout_position_absolute(ਬਣਤਰ box *box,
			 ਬਣਤਰ box *containing_ਖੰਡ,
			 ਪੂਰਨ cx, ਪੂਰਨ cy,
			 html_content *content)
{
	ਬਣਤਰ box *c;

	ਲਈ (c = box->children; c; c = c->ਅਗਲਾ) {
		ਜੇ ((c->type == BOX_ਖੰਡ || c->type == BOX_ਸਾਰਣੀ ||
				c->type == BOX_INLINE_ਖੰਡ ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			ਜੇ (!layout_absolute(c, containing_ਖੰਡ,
					cx, cy, content))
				ਵਾਪਸ false;
			ਜੇ (!layout_position_absolute(c, c, 0, 0, content))
				ਵਾਪਸ false;
		} ਨਹੀਂਤਾਂ ਜੇ (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			ਜੇ (!layout_position_absolute(c, c, 0, 0, content))
				ਵਾਪਸ false;
		} ਨਹੀਂਤਾਂ {
			ਪੂਰਨ px, py;
			ਜੇ (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_ਖੱਬਾ ||
					css_computed_float(c->style) ==
						CSS_FLOAT_ਸੱਜਾ)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				ਬਣਤਰ box *p;
				px = c->x;
				py = c->y;
				ਲਈ (p = box->ਮਾਪਾ; p && !p->float_children;
						p = p->ਮਾਪਾ) {
					px -= p->x;
					py -= p->y;
				}
			} ਨਹੀਂਤਾਂ {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			ਜੇ (!layout_position_absolute(c, containing_ਖੰਡ,
					cx + px, cy + py, content))
				ਵਾਪਸ false;
		}
	}

	ਵਾਪਸ true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
ਸਥਿਰ ਖਾਲੀ layout_compute_relative_ਵਿੱਥ(
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਬਣਤਰ box *box,
		ਪੂਰਨ *x,
		ਪੂਰਨ *y)
{
	ਪੂਰਨ ਖੱਬਾ, ਸੱਜਾ, ਸਿਖਰ, bottom;
	ਬਣਤਰ box *containing_ਖੰਡ;

	ਪ੍ਰਤਿਜ੍ਞਾ(box && box->ਮਾਪਾ && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	ਜੇ (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_ਖੱਬਾ ||
			css_computed_float(box->style) == CSS_FLOAT_ਸੱਜਾ)) {
		containing_ਖੰਡ = box->float_container;
	} ਨਹੀਂਤਾਂ {
		containing_ਖੰਡ = box->ਮਾਪਾ;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_ਖੰਡ,
			&ਸਿਖਰ, &ਸੱਜਾ, &bottom, &ਖੱਬਾ);

	ਜੇ (ਖੱਬਾ == AUTO && ਸੱਜਾ == AUTO)
		ਖੱਬਾ = ਸੱਜਾ = 0;
	ਨਹੀਂਤਾਂ ਜੇ (ਖੱਬਾ == AUTO)
		/* left is auto => computed = -right */
		ਖੱਬਾ = -ਸੱਜਾ;
	ਨਹੀਂਤਾਂ ਜੇ (ਸੱਜਾ == AUTO)
		/* right is auto => computed = -left */
		ਸੱਜਾ = -ਖੱਬਾ;
	ਨਹੀਂਤਾਂ {
		/* over constrained => examine direction property
		 * of containing block */
		ਜੇ (containing_ਖੰਡ->style &&
				css_computed_direction(
				containing_ਖੰਡ->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			ਖੱਬਾ = -ਸੱਜਾ;
		} ਨਹੀਂਤਾਂ {
			/* assume LTR in all other cases */
			ਸੱਜਾ = -ਖੱਬਾ;
		}
	}

	ਪ੍ਰਤਿਜ੍ਞਾ(ਖੱਬਾ == -ਸੱਜਾ);

	ਜੇ (ਸਿਖਰ == AUTO && bottom == AUTO) {
		ਸਿਖਰ = bottom = 0;
	} ਨਹੀਂਤਾਂ ਜੇ (ਸਿਖਰ == AUTO) {
		ਸਿਖਰ = -bottom;
	} ਨਹੀਂਤਾਂ {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -ਸਿਖਰ;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", ਖੱਬਾ,
	      ਸੱਜਾ, ਸਿਖਰ, bottom);

	*x = ਖੱਬਾ;
	*y = ਸਿਖਰ;
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
ਸਥਿਰ ਖਾਲੀ
layout_position_relative(
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਬਣਤਰ box *ਜੜ੍ਹ,
		ਬਣਤਰ box *fp,
		ਪੂਰਨ fx,
		ਪੂਰਨ fy)
{
	ਬਣਤਰ box *box; /* for children of "root" */
	ਬਣਤਰ box *fn;  /* for block formatting context box for children of
			  * "box" */
	ਬਣਤਰ box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	ਪੂਰਨ x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	ਪੂਰਨ fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	ਪ੍ਰਤਿਜ੍ਞਾ(ਜੜ੍ਹ);

	/* Normal children */
	ਲਈ (box = ਜੜ੍ਹ->children; box; box = box->ਅਗਲਾ) {

		ਜੇ (box->type == BOX_ਪਾਠ)
			ਜਾਰੀ;

		/* If relatively positioned, get offsets */
		ਜੇ (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_ਵਿੱਥ(
					unit_len_ctx, box, &x, &y);
		ਨਹੀਂਤਾਂ
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		ਜੇ (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_ਖੱਬਾ ||
				css_computed_float(box->style) ==
					CSS_FLOAT_ਸੱਜਾ) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			ਲਈ (fc = fp->float_children; fc; fc = fc->ਅਗਲਾ_float) {
				ਜੇ (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		ਜੇ (box->float_children) {
			fn = box;
			fnx = fny = 0;
		} ਨਹੀਂਤਾਂ {
			fn = fp;
			fnx = fx + x;
			fny = fy + y;
		}

		/* recurse first */
		layout_position_relative(unit_len_ctx, box, fn, fnx, fny);

		/* Ignore things we're not interested in. */
		ਜੇ (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			ਜਾਰੀ;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		ਜੇ (box->type == BOX_INLINE && box->inline_ਸਿਰਾ) {
			ਬਣਤਰ box *b;
			ਲਈ (b = box->ਅਗਲਾ; b && b != box->inline_ਸਿਰਾ;
					b = b->ਅਗਲਾ) {
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
ਸਥਿਰ ਖਾਲੀ
layout_get_box_bbox(
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਬਣਤਰ box *box,
		ਪੂਰਨ *desc_x0, ਪੂਰਨ *desc_y0,
		ਪੂਰਨ *desc_x1, ਪੂਰਨ *desc_y1)
{
	*desc_x0 = -box->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
	*desc_y0 = -box->border[ਸਿਖਰ_2].ਚੌੜਾਈ;
	*desc_x1 = box->padding[ਖੱਬਾ_2] + box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2] +
			box->border[ਸੱਜਾ_2].ਚੌੜਾਈ;
	*desc_y1 = box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ + box->padding[BOTTOM] +
			box->border[BOTTOM].ਚੌੜਾਈ;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	ਜੇ (box->type == BOX_ਖੰਡ && box->style != ਨਿਰਾ &&
			css_computed_ਉਛਾਲ_y(box->style) ==
					CSS_ਉਛਾਲ_VISIBLE &&
			box->object == ਨਿਰਾ) {
		css_fixed font_ਮਾਪ = 0;
		css_unit font_unit = CSS_UNIT_PT;
		ਪੂਰਨ ਪਾਠ_ਉਚਾਈ;

		css_computed_font_ਮਾਪ(box->style, &font_ਮਾਪ, &font_unit);
		ਪਾਠ_ਉਚਾਈ = css_unit_len2device_px(box->style, unit_len_ctx,
				font_ਮਾਪ, font_unit);
		ਪਾਠ_ਉਚਾਈ = FIXTOINT(ਪਾਠ_ਉਚਾਈ * 3 / 4);
		*desc_y0 = (*desc_y0 < -ਪਾਠ_ਉਚਾਈ) ? *desc_y0 : -ਪਾਠ_ਉਚਾਈ;
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
ਸਥਿਰ ਖਾਲੀ
layout_update_descendant_bbox(
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਬਣਤਰ box *box,
		ਬਣਤਰ box *ਬੱਚਾ,
		ਪੂਰਨ off_x,
		ਪੂਰਨ off_y)
{
	ਪੂਰਨ ਬੱਚਾ_desc_x0, ਬੱਚਾ_desc_y0, ਬੱਚਾ_desc_x1, ਬੱਚਾ_desc_y1;

	/* get coordinates of child relative to box */
	ਪੂਰਨ ਬੱਚਾ_x = ਬੱਚਾ->x - off_x;
	ਪੂਰਨ ਬੱਚਾ_y = ਬੱਚਾ->y - off_y;

	bool html_object = (ਬੱਚਾ->object &&
			content_get_type(ਬੱਚਾ->object) == CONTENT_HTML);

	ਗਿਣਤੀ css_ਉਛਾਲ_e ਉਛਾਲ_x = CSS_ਉਛਾਲ_VISIBLE;
	ਗਿਣਤੀ css_ਉਛਾਲ_e ਉਛਾਲ_y = CSS_ਉਛਾਲ_VISIBLE;

	ਜੇ (ਬੱਚਾ->style != ਨਿਰਾ) {
		ਉਛਾਲ_x = css_computed_ਉਛਾਲ_x(ਬੱਚਾ->style);
		ਉਛਾਲ_y = css_computed_ਉਛਾਲ_y(ਬੱਚਾ->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, ਬੱਚਾ,
			&ਬੱਚਾ_desc_x0, &ਬੱਚਾ_desc_y0,
			&ਬੱਚਾ_desc_x1, &ਬੱਚਾ_desc_y1);

	ਜੇ (ਉਛਾਲ_x == CSS_ਉਛਾਲ_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		ਬੱਚਾ_desc_x0 = ਬੱਚਾ->descendant_x0;
		ਬੱਚਾ_desc_x1 = ਬੱਚਾ->descendant_x1;
	}
	ਜੇ (ਉਛਾਲ_y == CSS_ਉਛਾਲ_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		ਬੱਚਾ_desc_y0 = ਬੱਚਾ->descendant_y0;
		ਬੱਚਾ_desc_y1 = ਬੱਚਾ->descendant_y1;
	}

	ਬੱਚਾ_desc_x0 += ਬੱਚਾ_x;
	ਬੱਚਾ_desc_y0 += ਬੱਚਾ_y;
	ਬੱਚਾ_desc_x1 += ਬੱਚਾ_x;
	ਬੱਚਾ_desc_y1 += ਬੱਚਾ_y;

	/* increase box's descendant bbox to contain descendants */
	ਜੇ (ਬੱਚਾ_desc_x0 < box->descendant_x0)
		box->descendant_x0 = ਬੱਚਾ_desc_x0;
	ਜੇ (ਬੱਚਾ_desc_y0 < box->descendant_y0)
		box->descendant_y0 = ਬੱਚਾ_desc_y0;
	ਜੇ (box->descendant_x1 < ਬੱਚਾ_desc_x1)
		box->descendant_x1 = ਬੱਚਾ_desc_x1;
	ਜੇ (box->descendant_y1 < ਬੱਚਾ_desc_y1)
		box->descendant_y1 = ਬੱਚਾ_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
ਸਥਿਰ ਖਾਲੀ layout_calculate_descendant_bboxes(
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਬਣਤਰ box *box)
{
	ਬਣਤਰ box *ਬੱਚਾ;

	ਪ੍ਰਤਿਜ੍ਞਾ(box->ਚੌੜਾਈ != UNKNOWN_ਚੌੜਾਈ);
	ਪ੍ਰਤਿਜ੍ਞਾ(box->ਉਚਾਈ != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	ਜੇ (box->object && content_get_type(box->object) == CONTENT_HTML) {
		ਜੇ (box->descendant_x1 < content_get_ਚੌੜਾਈ(box->object))
			box->descendant_x1 = content_get_ਚੌੜਾਈ(box->object);
		ਜੇ (box->descendant_y1 < content_get_ਉਚਾਈ(box->object))
			box->descendant_y1 = content_get_ਉਚਾਈ(box->object);
	}

	ਜੇ (box->iframe != ਨਿਰਾ) {
		ਪੂਰਨ x, y;
		box_coords(box, &x, &y);

		browser_ਖਿੜਕੀ_ਸੈੱਟ_position(box->iframe, x, y);
		browser_ਖਿੜਕੀ_ਸੈੱਟ_dimensions(box->iframe,
				box->ਚੌੜਾਈ, box->ਉਚਾਈ);
		browser_ਖਿੜਕੀ_reformat(box->iframe, true,
				box->ਚੌੜਾਈ, box->ਉਚਾਈ);
	}

	ਜੇ (box->type == BOX_INLINE || box->type == BOX_ਪਾਠ)
		ਵਾਪਸ;

	ਜੇ (box->type == BOX_INLINE_ਸਿਰਾ) {
		box = box->inline_ਸਿਰਾ;
		ਲਈ (ਬੱਚਾ = box->ਅਗਲਾ; ਬੱਚਾ;
				ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ) {
			ਜੇ (ਬੱਚਾ->type == BOX_FLOAT_ਖੱਬਾ ||
					ਬੱਚਾ->type == BOX_FLOAT_ਸੱਜਾ)
				ਜਾਰੀ;

			layout_update_descendant_bbox(unit_len_ctx, box, ਬੱਚਾ,
					box->x, box->y);

			ਜੇ (ਬੱਚਾ == box->inline_ਸਿਰਾ)
				ਰੋਕੋ;
		}
		ਵਾਪਸ;
	}

	ਜੇ (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		ਵਾਪਸ;

	ਲਈ (ਬੱਚਾ = box->children; ਬੱਚਾ; ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ) {
		ਜੇ (ਬੱਚਾ->type == BOX_FLOAT_ਖੱਬਾ ||
				ਬੱਚਾ->type == BOX_FLOAT_ਸੱਜਾ)
			ਜਾਰੀ;

		layout_calculate_descendant_bboxes(unit_len_ctx, ਬੱਚਾ);

		ਜੇ (box->style && css_computed_ਉਛਾਲ_x(box->style) ==
				CSS_ਉਛਾਲ_HIDDEN &&
				css_computed_ਉਛਾਲ_y(box->style) ==
				CSS_ਉਛਾਲ_HIDDEN)
			ਜਾਰੀ;

		layout_update_descendant_bbox(unit_len_ctx, box, ਬੱਚਾ, 0, 0);
	}

	ਲਈ (ਬੱਚਾ = box->float_children; ਬੱਚਾ; ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ_float) {
		ਪ੍ਰਤਿਜ੍ਞਾ(ਬੱਚਾ->type == BOX_FLOAT_ਖੱਬਾ ||
				ਬੱਚਾ->type == BOX_FLOAT_ਸੱਜਾ);

		layout_calculate_descendant_bboxes(unit_len_ctx, ਬੱਚਾ);

		layout_update_descendant_bbox(unit_len_ctx, box, ਬੱਚਾ, 0, 0);
	}

	ਜੇ (box->list_marker) {
		ਬੱਚਾ = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, ਬੱਚਾ);

		layout_update_descendant_bbox(unit_len_ctx, box, ਬੱਚਾ, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ)
{
	bool ret;
	ਬਣਤਰ box *doc = content->layout;
	ਅਚਲ ਬਣਤਰ gui_layout_ਸਾਰਣੀ *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			ਚੌੜਾਈ, ਉਚਾਈ, nsurl_access(content_get_url(
					&content->ਅਧਾਰ)));

	layout_minmax_ਖੰਡ(doc, font_func, content);

	layout_ਖੰਡ_ਲੱਭੋ_dimensions(&content->unit_len_ctx,
			ਚੌੜਾਈ, ਉਚਾਈ, 0, 0, doc);
	doc->x = doc->margin[ਖੱਬਾ_2] + doc->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
	doc->y = doc->margin[ਸਿਖਰ_2] + doc->border[ਸਿਖਰ_2].ਚੌੜਾਈ;
	ਚੌੜਾਈ -= doc->margin[ਖੱਬਾ_2] + doc->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
			doc->padding[ਖੱਬਾ_2] + doc->padding[ਸੱਜਾ_2] +
			doc->border[ਸੱਜਾ_2].ਚੌੜਾਈ + doc->margin[ਸੱਜਾ_2];
	ਜੇ (ਚੌੜਾਈ < 0) {
		ਚੌੜਾਈ = 0;
	}
	doc->ਚੌੜਾਈ = ਚੌੜਾਈ;

	ret = layout_ਖੰਡ_context(doc, ਉਚਾਈ, content);

	/* make <html> and <body> fill available height */
	ਜੇ (doc->y + doc->padding[ਸਿਖਰ_2] + doc->ਉਚਾਈ + doc->padding[BOTTOM] +
			doc->border[BOTTOM].ਚੌੜਾਈ + doc->margin[BOTTOM] <
			ਉਚਾਈ) {
		doc->ਉਚਾਈ = ਉਚਾਈ - (doc->y + doc->padding[ਸਿਖਰ_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].ਚੌੜਾਈ +
				doc->margin[BOTTOM]);
		ਜੇ (doc->children)
			doc->children->ਉਚਾਈ = doc->ਉਚਾਈ -
					(doc->children->margin[ਸਿਖਰ_2] +
					 doc->children->border[ਸਿਖਰ_2].ਚੌੜਾਈ +
					 doc->children->padding[ਸਿਖਰ_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].ਚੌੜਾਈ +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	ਵਾਪਸ ret;
}
