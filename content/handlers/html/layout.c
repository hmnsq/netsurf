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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <math.h>
#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/talloc.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "netsurf/browser_window.h"
#ಅನ್ತರ್ಭಾವ "netsurf/layout.h"
#ಅನ್ತರ್ಭಾವ "content/content.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "css/utils.h"
#ಅನ್ತರ್ಭಾವ "desktop/scrollbar.h"
#ಅನ್ತರ್ಭಾವ "desktop/textarea.h"

#ಅನ್ತರ್ಭಾವ "html/html.h"
#ಅನ್ತರ್ಭಾವ "html/html_save.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_inspect.h"
#ಅನ್ತರ್ಭಾವ "html/font.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"
#ಅನ್ತರ್ಭಾವ "html/layout.h"
#ಅನ್ತರ್ಭಾವ "html/layout_internal.h"
#ಅನ್ತರ್ಭಾವ "html/table.h"

/** Array of per-side access functions for computed style margins. */
ಸ್ಥಿರಾಂಕ css_len_func margin_funcs[4] = {
	[ತುದಿ_2]    = css_computed_margin_ತುದಿ,
	[ಬಲ_2]  = css_computed_margin_ಬಲ,
	[BOTTOM] = css_computed_margin_bottom,
	[ಎಡ_2]   = css_computed_margin_ಎಡ,
};

/** Array of per-side access functions for computed style paddings. */
ಸ್ಥಿರಾಂಕ css_len_func padding_funcs[4] = {
	[ತುದಿ_2]    = css_computed_padding_ತುದಿ,
	[ಬಲ_2]  = css_computed_padding_ಬಲ,
	[BOTTOM] = css_computed_padding_bottom,
	[ಎಡ_2]   = css_computed_padding_ಎಡ,
};

/** Array of per-side access functions for computed style border_widths. */
ಸ್ಥಿರಾಂಕ css_len_func border_ಅಗಲ_funcs[4] = {
	[ತುದಿ_2]    = css_computed_border_ತುದಿ_ಅಗಲ,
	[ಬಲ_2]  = css_computed_border_ಬಲ_ಅಗಲ,
	[BOTTOM] = css_computed_border_bottom_ಅಗಲ,
	[ಎಡ_2]   = css_computed_border_ಎಡ_ಅಗಲ,
};

/** Array of per-side access functions for computed style border styles. */
ಸ್ಥಿರಾಂಕ css_border_style_func border_style_funcs[4] = {
	[ತುದಿ_2]    = css_computed_border_ತುದಿ_style,
	[ಬಲ_2]  = css_computed_border_ಬಲ_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[ಎಡ_2]   = css_computed_border_ಎಡ_style,
};

/** Array of per-side access functions for computed style border colors. */
ಸ್ಥಿರಾಂಕ css_border_color_func border_color_funcs[4] = {
	[ತುದಿ_2]    = css_computed_border_ತುದಿ_color,
	[ಬಲ_2]  = css_computed_border_ಬಲ_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[ಎಡ_2]   = css_computed_border_ಎಡ_color,
};

/* forward declaration to break cycles */
ಸ್ಥಿರ ಶೂನ್ಯ layout_minmax_ಖಂಡ(
		ರಚನೆ box *ಖಂಡ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ gui_layout_ಕೋಷ್ಟಕ *font_func,
		ಸ್ಥಿರಾಂಕ html_content *content);

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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_get_object_dimensions(ರಚನೆ box *box,
			     ಪೂರ್ಣಾಂಕ *ಅಗಲ, ಪೂರ್ಣಾಂಕ *ಎತ್ತರ,
			     ಪೂರ್ಣಾಂಕ min_ಅಗಲ, ಪೂರ್ಣಾಂಕ max_ಅಗಲ,
			     ಪೂರ್ಣಾಂಕ min_ಎತ್ತರ, ಪೂರ್ಣಾಂಕ max_ಎತ್ತರ)
{
	ಪ್ರತಿಜ್ಞಾ(box->object != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(ಅಗಲ != ಖಾಲಿ && ಎತ್ತರ != ಖಾಲಿ);

	ಆದರೆ (*ಅಗಲ == AUTO && *ಎತ್ತರ == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		ಪೂರ್ಣಾಂಕ intrinsic_ಅಗಲ = content_get_ಅಗಲ(box->object);
		ಪೂರ್ಣಾಂಕ intrinsic_ಎತ್ತರ = content_get_ಎತ್ತರ(box->object);

		/* use intrinsic dimensions */
		*ಅಗಲ = intrinsic_ಅಗಲ;
		*ಎತ್ತರ = intrinsic_ಎತ್ತರ;

		/* Deal with min/max-width first */
		ಆದರೆ (min_ಅಗಲ >  0 && min_ಅಗಲ > *ಅಗಲ) {
			*ಅಗಲ = min_ಅಗಲ;
			scaled = true;
		}
		ಆದರೆ (max_ಅಗಲ >= 0 && max_ಅಗಲ < *ಅಗಲ) {
			*ಅಗಲ = max_ಅಗಲ;
			scaled = true;
		}

		ಆದರೆ (scaled && (intrinsic_ಅಗಲ != 0)) {
			/* Update height */
			*ಎತ್ತರ = (*ಅಗಲ * intrinsic_ಎತ್ತರ) /
					intrinsic_ಅಗಲ;
		}

		scaled = false;
		/* Deal with min/max-height */
		ಆದರೆ (min_ಎತ್ತರ >  0 && min_ಎತ್ತರ > *ಎತ್ತರ) {
			*ಎತ್ತರ = min_ಎತ್ತರ;
			scaled = true;
		}
		ಆದರೆ (max_ಎತ್ತರ >= 0 && max_ಎತ್ತರ < *ಎತ್ತರ) {
			*ಎತ್ತರ = max_ಎತ್ತರ;
			scaled = true;
		}

		ಆದರೆ (scaled && (intrinsic_ಎತ್ತರ != 0)) {
			/* Update width */
			*ಅಗಲ = (*ಎತ್ತರ * intrinsic_ಅಗಲ) /
					intrinsic_ಎತ್ತರ;
		}

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*ಅಗಲ == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		ಪೂರ್ಣಾಂಕ intrinsic_ಅಗಲ = content_get_ಅಗಲ(box->object);
		ಪೂರ್ಣಾಂಕ intrinsic_ಎತ್ತರ = content_get_ಎತ್ತರ(box->object);

		ಆದರೆ (intrinsic_ಎತ್ತರ != 0)
			*ಅಗಲ = (*ಎತ್ತರ * intrinsic_ಅಗಲ) /
					intrinsic_ಎತ್ತರ;
		ಇಲ್ಲದಿದ್ದರೆ
			*ಅಗಲ = intrinsic_ಅಗಲ;

		ಆದರೆ (min_ಅಗಲ >  0 && min_ಅಗಲ > *ಅಗಲ)
			*ಅಗಲ = min_ಅಗಲ;
		ಆದರೆ (max_ಅಗಲ >= 0 && max_ಅಗಲ < *ಅಗಲ)
			*ಅಗಲ = max_ಅಗಲ;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*ಎತ್ತರ == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		ಪೂರ್ಣಾಂಕ intrinsic_ಅಗಲ = content_get_ಅಗಲ(box->object);
		ಪೂರ್ಣಾಂಕ intrinsic_ಎತ್ತರ = content_get_ಎತ್ತರ(box->object);

		ಆದರೆ (min_ಅಗಲ >  0 && min_ಅಗಲ > *ಅಗಲ)
			*ಅಗಲ = min_ಅಗಲ;
		ಆದರೆ (max_ಅಗಲ >= 0 && max_ಅಗಲ < *ಅಗಲ)
			*ಅಗಲ = max_ಅಗಲ;

		ಆದರೆ (intrinsic_ಅಗಲ != 0)
			*ಎತ್ತರ = (*ಅಗಲ * intrinsic_ಎತ್ತರ) /
					intrinsic_ಅಗಲ;
		ಇಲ್ಲದಿದ್ದರೆ
			*ಎತ್ತರ = intrinsic_ಎತ್ತರ;
	}
}


/**
 * Calculate the text-indent length.
 *
 * \param  style  style of block
 * \param  width  width of containing block
 * \return  length of indent
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ layout_ಪಠ್ಯ_indent(
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಸ್ಥಿರಾಂಕ css_computed_style *style, ಪೂರ್ಣಾಂಕ ಅಗಲ)
{
	css_fixed ಮೌಲ್ಯ = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_ಪಠ್ಯ_indent(style, &ಮೌಲ್ಯ, &unit);

	ಆದರೆ (unit == CSS_UNIT_PCT) {
		ಹಿಂತಿರುಗಿ FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ, ಅಗಲ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಹಿಂತಿರುಗಿ FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				ಮೌಲ್ಯ, unit));
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
ಸ್ಥಿರ ಶೂನ್ಯ layout_minmax_ಕೋಷ್ಟಕ(ರಚನೆ box *ಕೋಷ್ಟಕ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ gui_layout_ಕೋಷ್ಟಕ *font_func,
		ಸ್ಥಿರಾಂಕ html_content *content)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i, j;
	ಪೂರ್ಣಾಂಕ ಅಗಲ;
	ಪೂರ್ಣಾಂಕ border_spacing_h = 0;
	ಪೂರ್ಣಾಂಕ ಕೋಷ್ಟಕ_min = 0, ಕೋಷ್ಟಕ_max = 0;
	ಪೂರ್ಣಾಂಕ extra_fixed = 0;
	ದಶಮಾಂಶ extra_frac = 0;
	ರಚನೆ ಸ್ತಂಭ *ಲಂಬಸಾಲು;
	ರಚನೆ box *ಅಡ್ಡಸಾಲು_ಗುಂಪು, *ಅಡ್ಡಸಾಲು, *cell;

	/* check if the widths have already been calculated */
	ಆದರೆ (ಕೋಷ್ಟಕ->max_ಅಗಲ != UNKNOWN_MAX_ಅಗಲ)
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (ಕೋಷ್ಟಕ_calculate_ಸ್ತಂಭ_types(&content->unit_len_ctx, ಕೋಷ್ಟಕ) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		ಹಿಂತಿರುಗಿ;
	}
	ಲಂಬಸಾಲು = ಕೋಷ್ಟಕ->ಲಂಬಸಾಲು;

	/* start with 0 except for fixed-width columns */
	ಗಾಗಿ (i = 0; i != ಕೋಷ್ಟಕ->columns; i++) {
		ಆದರೆ (ಲಂಬಸಾಲು[i].type == ಸ್ತಂಭ_ಅಗಲ_FIXED)
			ಲಂಬಸಾಲು[i].min = ಲಂಬಸಾಲು[i].max = ಲಂಬಸಾಲು[i].ಅಗಲ;
		ಇಲ್ಲದಿದ್ದರೆ
			ಲಂಬಸಾಲು[i].min = ಲಂಬಸಾಲು[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	ಆದರೆ (css_computed_border_collapse(ಕೋಷ್ಟಕ->style) ==
			CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		css_computed_border_spacing(ಕೋಷ್ಟಕ->style, &h, &hu, &v, &vu);

		border_spacing_h = FIXTOINT(css_unit_len2device_px(
				ಕೋಷ್ಟಕ->style,
				&content->unit_len_ctx,
				h, hu));
	}

	/* 1st pass: consider cells with colspan 1 only */
	ಗಾಗಿ (ಅಡ್ಡಸಾಲು_ಗುಂಪು = ಕೋಷ್ಟಕ->children; ಅಡ್ಡಸಾಲು_ಗುಂಪು; ಅಡ್ಡಸಾಲು_ಗುಂಪು =ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಮುಂದಿನ)
	ಗಾಗಿ (ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು_ಗುಂಪು->children; ಅಡ್ಡಸಾಲು; ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->ಮುಂದಿನ)
	ಗಾಗಿ (cell = ಅಡ್ಡಸಾಲು->children; cell; cell = cell->ಮುಂದಿನ) {
		ಪ್ರತಿಜ್ಞಾ(cell->type == BOX_ಕೋಷ್ಟಕ_CELL);
		ಪ್ರತಿಜ್ಞಾ(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		ಪ್ರತಿಜ್ಞಾ(cell->columns != 0);

		ಆದರೆ (cell->columns != 1)
			ಮುಂದುವರಿ;

		layout_minmax_ಖಂಡ(cell, font_func, content);
		i = cell->ಆರಂಭ_ಸ್ತಂಭ;

		ಆದರೆ (ಲಂಬಸಾಲು[i].positioned)
			ಮುಂದುವರಿ;

		/* update column min, max widths using cell widths */
		ಆದರೆ (ಲಂಬಸಾಲು[i].min < cell->min_ಅಗಲ)
			ಲಂಬಸಾಲು[i].min = cell->min_ಅಗಲ;
		ಆದರೆ (ಲಂಬಸಾಲು[i].max < cell->max_ಅಗಲ)
			ಲಂಬಸಾಲು[i].max = cell->max_ಅಗಲ;
	}

	/* 2nd pass: cells which span multiple columns */
	ಗಾಗಿ (ಅಡ್ಡಸಾಲು_ಗುಂಪು = ಕೋಷ್ಟಕ->children; ಅಡ್ಡಸಾಲು_ಗುಂಪು; ಅಡ್ಡಸಾಲು_ಗುಂಪು =ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಮುಂದಿನ)
	ಗಾಗಿ (ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು_ಗುಂಪು->children; ಅಡ್ಡಸಾಲು; ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->ಮುಂದಿನ)
	ಗಾಗಿ (cell = ಅಡ್ಡಸಾಲು->children; cell; cell = cell->ಮುಂದಿನ) {
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flexible_columns = 0;
		ಪೂರ್ಣಾಂಕ min = 0, max = 0, fixed_ಅಗಲ = 0, extra;

		ಆದರೆ (cell->columns == 1)
			ಮುಂದುವರಿ;

		layout_minmax_ಖಂಡ(cell, font_func, content);
		i = cell->ಆರಂಭ_ಸ್ತಂಭ;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		ಗಾಗಿ (j = 0; j != cell->columns; j++) {
			min += ಲಂಬಸಾಲು[i + j].min;
			ಆದರೆ (ಲಂಬಸಾಲು[i + j].type == ಸ್ತಂಭ_ಅಗಲ_FIXED)
				fixed_ಅಗಲ += ಲಂಬಸಾಲು[i + j].ಅಗಲ;
			ಇಲ್ಲದಿದ್ದರೆ
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		ಆದರೆ (min < cell->min_ಅಗಲ) {
			ಆದರೆ (flexible_columns == 0) {
				extra = 1 + (cell->min_ಅಗಲ - min) /
						cell->columns;
				ಗಾಗಿ (j = 0; j != cell->columns; j++) {
					ಲಂಬಸಾಲು[i + j].min += extra;
					ಆದರೆ (ಲಂಬಸಾಲು[i + j].max < ಲಂಬಸಾಲು[i + j].min)
						ಲಂಬಸಾಲು[i + j].max = ಲಂಬಸಾಲು[i + j].min;
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				extra = 1 + (cell->min_ಅಗಲ - min) /
						flexible_columns;
				ಗಾಗಿ (j = 0; j != cell->columns; j++) {
					ಆದರೆ (ಲಂಬಸಾಲು[i + j].type !=
							ಸ್ತಂಭ_ಅಗಲ_FIXED) {
						ಲಂಬಸಾಲು[i + j].min += extra;
						ಆದರೆ (ಲಂಬಸಾಲು[i + j].max <
								ಲಂಬಸಾಲು[i + j].min)
							ಲಂಬಸಾಲು[i + j].max =
								ಲಂಬಸಾಲು[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		ಗಾಗಿ (j = 0; j != cell->columns; j++)
			max += ಲಂಬಸಾಲು[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		ಆದರೆ (max < cell->max_ಅಗಲ && flexible_columns) {
			extra = 1 + (cell->max_ಅಗಲ - max) / flexible_columns;
			ಗಾಗಿ (j = 0; j != cell->columns; j++)
				ಆದರೆ (ಲಂಬಸಾಲು[i + j].type != ಸ್ತಂಭ_ಅಗಲ_FIXED)
					ಲಂಬಸಾಲು[i + j].max += extra;
		}
	}

	ಗಾಗಿ (i = 0; i != ಕೋಷ್ಟಕ->columns; i++) {
		ಆದರೆ (ಲಂಬಸಾಲು[i].max < ಲಂಬಸಾಲು[i].min) {
			box_dump(stderr, ಕೋಷ್ಟಕ, 0, true);
			ಪ್ರತಿಜ್ಞಾ(0);
		}
		ಕೋಷ್ಟಕ_min += ಲಂಬಸಾಲು[i].min;
		ಕೋಷ್ಟಕ_max += ಲಂಬಸಾಲು[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	ಆದರೆ (css_computed_ಅಗಲ_px(ಕೋಷ್ಟಕ->style, &content->unit_len_ctx,
			-1, &ಅಗಲ) == CSS_ಅಗಲ_ಗಣ) {
		ಆದರೆ (ಕೋಷ್ಟಕ_min < ಅಗಲ)
			ಕೋಷ್ಟಕ_min = ಅಗಲ;
		ಆದರೆ (ಕೋಷ್ಟಕ_max < ಅಗಲ)
			ಕೋಷ್ಟಕ_max = ಅಗಲ;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
			ಕೋಷ್ಟಕ->style, ಎಡ_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
			ಕೋಷ್ಟಕ->style, ಬಲ_2, true, true, true,
			&extra_fixed, &extra_frac);
	ಆದರೆ (extra_fixed < 0)
		extra_fixed = 0;
	ಆದರೆ (extra_frac < 0)
		extra_frac = 0;
	ಆದರೆ (1.0 <= extra_frac)
		extra_frac = 0.9;
	ಕೋಷ್ಟಕ->min_ಅಗಲ = (ಕೋಷ್ಟಕ_min + extra_fixed) / (1.0 - extra_frac);
	ಕೋಷ್ಟಕ->max_ಅಗಲ = (ಕೋಷ್ಟಕ_max + extra_fixed) / (1.0 - extra_frac);
	ಕೋಷ್ಟಕ->min_ಅಗಲ += (ಕೋಷ್ಟಕ->columns + 1) * border_spacing_h;
	ಕೋಷ್ಟಕ->max_ಅಗಲ += (ಕೋಷ್ಟಕ->columns + 1) * border_spacing_h;

	ಪ್ರತಿಜ್ಞಾ(0 <= ಕೋಷ್ಟಕ->min_ಅಗಲ && ಕೋಷ್ಟಕ->min_ಅಗಲ <= ಕೋಷ್ಟಕ->max_ಅಗಲ);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool box_has_percentage_max_ಅಗಲ(ರಚನೆ box *b)
{
	css_unit unit = CSS_UNIT_PX;
	ಎಣಿಕೆ css_max_ಅಗಲ_e type;
	css_fixed ಮೌಲ್ಯ = 0;

	ಪ್ರತಿಜ್ಞಾ(b != ಖಾಲಿ);

	type = css_computed_max_ಅಗಲ(b->style, &ಮೌಲ್ಯ, &unit);
	ಹಿಂತಿರುಗಿ ((type == CSS_MAX_ಅಗಲ_ಗಣ) && (unit == CSS_UNIT_PCT));
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
ಸ್ಥಿರ ರಚನೆ box *
layout_minmax_ಸಾಲು(ರಚನೆ box *ಮೊದಲ,
		   ಪೂರ್ಣಾಂಕ *ಸಾಲು_min,
		   ಪೂರ್ಣಾಂಕ *ಸಾಲು_max,
		   bool ಮೊದಲ_ಸಾಲು,
		   bool *ಸಾಲು_has_ಎತ್ತರ,
		   ಸ್ಥಿರಾಂಕ ರಚನೆ gui_layout_ಕೋಷ್ಟಕ *font_func,
		   ಸ್ಥಿರಾಂಕ html_content *content)
{
	ಪೂರ್ಣಾಂಕ min = 0, max = 0, ಅಗಲ, ಎತ್ತರ, fixed;
	ದಶಮಾಂಶ frac;
	size_t i, j;
	ರಚನೆ box *b;
	ರಚನೆ box *ಖಂಡ;
	plot_font_style_t fstyle;
	bool no_wrap;

	ಪ್ರತಿಜ್ಞಾ(ಮೊದಲ->ಪೋಷಕ);
	ಪ್ರತಿಜ್ಞಾ(ಮೊದಲ->ಪೋಷಕ->ಪೋಷಕ);
	ಪ್ರತಿಜ್ಞಾ(ಮೊದಲ->ಪೋಷಕ->ಪೋಷಕ->style);

	ಖಂಡ = ಮೊದಲ->ಪೋಷಕ->ಪೋಷಕ;
	no_wrap = (css_computed_white_space(ಖಂಡ->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(ಖಂಡ->style) ==
			CSS_WHITE_SPACE_PRE);

	*ಸಾಲು_has_ಎತ್ತರ = false;

	/* corresponds to the pass 1 loop in layout_line() */
	ಗಾಗಿ (b = ಮೊದಲ; b; b = b->ಮುಂದಿನ) {
		ಎಣಿಕೆ css_ಅಗಲ_e wtype;
		ಎಣಿಕೆ css_ಎತ್ತರ_e htype;
		ಎಣಿಕೆ css_box_sizing_e bs;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		ಪ್ರತಿಜ್ಞಾ(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		ಆದರೆ (b->type == BOX_BR) {
			b = b->ಮುಂದಿನ;
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (lh__box_is_float_box(b)) {
			ಪ್ರತಿಜ್ಞಾ(b->children);
			ಆದರೆ (b->children->type == BOX_ಕೋಷ್ಟಕ)
				layout_minmax_ಕೋಷ್ಟಕ(b->children, font_func,
						content);
			ಇಲ್ಲದಿದ್ದರೆ
				layout_minmax_ಖಂಡ(b->children, font_func,
						content);
			b->min_ಅಗಲ = b->children->min_ಅಗಲ;
			b->max_ಅಗಲ = b->children->max_ಅಗಲ;
			ಆದರೆ (min < b->min_ಅಗಲ)
				min = b->min_ಅಗಲ;
			max += b->max_ಅಗಲ;
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (b->type == BOX_INLINE_ಖಂಡ || b->type == BOX_INLINE_FLEX) {
			layout_minmax_ಖಂಡ(b, font_func, content);
			ಆದರೆ (min < b->min_ಅಗಲ)
				min = b->min_ಅಗಲ;
			max += b->max_ಅಗಲ;

			ಆದರೆ (b->flags & HAS_ಎತ್ತರ)
				*ಸಾಲು_has_ಎತ್ತರ = true;
			ಮುಂದುವರಿ;
		}

		ಪ್ರತಿಜ್ಞಾ(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		ಆದರೆ (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
					b->style, ಎಡ_2, true, true, true,
					&fixed, &frac);
			ಆದರೆ (!b->inline_ಕೊನೆ)
				calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
						b->style, ಬಲ_2,
						true, true, true,
						&fixed, &frac);
			ಆದರೆ (0 < fixed)
				max += fixed;
			*ಸಾಲು_has_ಎತ್ತರ = true;
			/* \todo  update min width, consider fractional extra */
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (b->type == BOX_INLINE_ಕೊನೆ) {
			fixed = frac = 0;
			calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
					b->inline_ಕೊನೆ->style, ಬಲ_2,
					true, true, true,
					&fixed, &frac);
			ಆದರೆ (0 < fixed)
				max += fixed;

			ಆದರೆ (b->ಮುಂದಿನ) {
				ಆದರೆ (b->space == UNKNOWN_ಅಗಲ) {
					font_func->ಅಗಲ(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*ಸಾಲು_has_ಎತ್ತರ = true;
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			ಆದರೆ (!b->ಪಠ್ಯ)
				ಮುಂದುವರಿ;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			ಆದರೆ (b->ಅಗಲ == UNKNOWN_ಅಗಲ) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				ಆದರೆ (b->ಪೋಷಕ->ಪೋಷಕ->gadget &&
						b->ಪೋಷಕ->ಪೋಷಕ->gadget->type
						== GADGET_SELECT) {
					ಪೂರ್ಣಾಂಕ opt_maxwidth = 0;
					ರಚನೆ form_option *o;

					ಗಾಗಿ (o = b->ಪೋಷಕ->ಪೋಷಕ->gadget->
							ದತ್ತಾಂಶ.select.ವಸ್ತುಗಳು; o;
							o = o->ಮುಂದಿನ) {
						ಪೂರ್ಣಾಂಕ opt_ಅಗಲ;
						font_func->ಅಗಲ(&fstyle,
								o->ಪಠ್ಯ,
								ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(o->ಪಠ್ಯ),
								&opt_ಅಗಲ);

						ಆದರೆ (opt_maxwidth < opt_ಅಗಲ)
							opt_maxwidth =opt_ಅಗಲ;
					}

					b->ಅಗಲ = opt_maxwidth;
					ಆದರೆ (nsoption_bool(core_select_menu))
						b->ಅಗಲ += SCROLLBAR_ಅಗಲ;

				} ಇಲ್ಲದಿದ್ದರೆ {
					font_func->ಅಗಲ(&fstyle, b->ಪಠ್ಯ,
						b->ಉದ್ದಳತೆ, &b->ಅಗಲ);
					b->flags |= MEASURED;
				}
			}
			max += b->ಅಗಲ;
			ಆದರೆ (b->ಮುಂದಿನ) {
				ಆದರೆ (b->space == UNKNOWN_ಅಗಲ) {
					font_func->ಅಗಲ(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			ಆದರೆ (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				ಆದರೆ (min < b->ಅಗಲ)
					min = b->ಅಗಲ;

			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (b->ಪೋಷಕ->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				ಮಾಡು {
					ಗಾಗಿ (j = i; j != b->ಉದ್ದಳತೆ &&
							b->ಪಠ್ಯ[j] != ' '; j++)
						;
					font_func->ಅಗಲ(&fstyle, b->ಪಠ್ಯ + i,
							 j - i, &ಅಗಲ);
					ಆದರೆ (min < ಅಗಲ)
						min = ಅಗಲ;
					i = j + 1;
				} ಆಗ (j != b->ಉದ್ದಳತೆ);
			}

			*ಸಾಲು_has_ಎತ್ತರ = true;

			ಮುಂದುವರಿ;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		ಪ್ರತಿಜ್ಞಾ(b->style);

		bs = css_computed_box_sizing(ಖಂಡ->style);

		/* calculate box width */
		wtype = css_computed_ಅಗಲ_px(b->style,
				&content->unit_len_ctx, -1, &ಅಗಲ);
		ಆದರೆ (wtype == CSS_ಅಗಲ_ಗಣ) {
			ಆದರೆ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
						ಖಂಡ->style, ಎಡ_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
						ಖಂಡ->style, ಬಲ_2,
						false, true, true,
						&fixed, &frac);
				ಆದರೆ (ಅಗಲ < fixed) {
					ಅಗಲ = fixed;
				}
			}
			ಆದರೆ (ಅಗಲ < 0)
				ಅಗಲ = 0;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಅಗಲ = AUTO;
		}

		/*ಎತ್ತರ*/
		htype = css_computed_ಎತ್ತರ(b->style, &ಮೌಲ್ಯ, &unit);
		ಆದರೆ (htype == CSS_ಎತ್ತರ_ಗಣ) {
			ಎತ್ತರ = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಎತ್ತರ = AUTO;
		}

		ಆದರೆ (b->object || (b->flags & REPLACE_DIM)) {
			ಆದರೆ (b->object) {
				ಪೂರ್ಣಾಂಕ ತಾತ್ಕಾಲಿಕ_ಎತ್ತರ = ಎತ್ತರ;
				layout_get_object_dimensions(b,
						&ಅಗಲ, &ತಾತ್ಕಾಲಿಕ_ಎತ್ತರ,
						INT_MIN, ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ,
						INT_MIN, ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ);
			}

			fixed = frac = 0;
			ಆದರೆ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
						b->style, ಎಡ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
						b->style, ಬಲ_2,
						true, false, false,
						&fixed, &frac);
			} ಇಲ್ಲದಿದ್ದರೆ {
				calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
						b->style, ಎಡ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
						b->style, ಬಲ_2,
						true, true, true,
						&fixed, &frac);
			}
			ಆದರೆ (0 < ಅಗಲ + fixed)
				ಅಗಲ += fixed;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			ಆದರೆ (ಅಗಲ == AUTO)
				ಅಗಲ = 400;

			fixed = frac = 0;
			ಆದರೆ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
						b->style, ಎಡ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
						b->style, ಬಲ_2,
						true, false, false,
						&fixed, &frac);
			} ಇಲ್ಲದಿದ್ದರೆ {
				calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
						b->style, ಎಡ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_ಅಗಲ(&content->unit_len_ctx,
						b->style, ಬಲ_2,
						true, true, true,
						&fixed, &frac);
			}

			ಆದರೆ (0 < ಅಗಲ + fixed)
				ಅಗಲ += fixed;

		} ಇಲ್ಲದಿದ್ದರೆ {
			/* form control with no object */
			ಆದರೆ (ಅಗಲ == AUTO)
				ಅಗಲ = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		ಆದರೆ (min < ಅಗಲ && !box_has_percentage_max_ಅಗಲ(b))
			min = ಅಗಲ;
		ಆದರೆ (ಅಗಲ > 0)
			max += ಅಗಲ;

		*ಸಾಲು_has_ಎತ್ತರ = true;
	}

	ಆದರೆ (ಮೊದಲ_ಸಾಲು) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		ಪೂರ್ಣಾಂಕ ಪಠ್ಯ_indent = layout_ಪಠ್ಯ_indent(&content->unit_len_ctx,
				ಮೊದಲ->ಪೋಷಕ->ಪೋಷಕ->style, 100);
		min = (min + ಪಠ್ಯ_indent < 0) ? 0 : min + ಪಠ್ಯ_indent;
		max = (max + ಪಠ್ಯ_indent < 0) ? 0 : max + ಪಠ್ಯ_indent;
	}

	*ಸಾಲು_min = min;
	*ಸಾಲು_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	ಪ್ರತಿಜ್ಞಾ(b != ಮೊದಲ);
	ಪ್ರತಿಜ್ಞಾ(0 <= *ಸಾಲು_min);
	ಪ್ರತಿಜ್ಞಾ(*ಸಾಲು_min <= *ಸಾಲು_max);
	ಹಿಂತಿರುಗಿ b;
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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_minmax_inline_container(ರಚನೆ box *inline_container,
			       bool *has_ಎತ್ತರ,
			       ಸ್ಥಿರಾಂಕ ರಚನೆ gui_layout_ಕೋಷ್ಟಕ *font_func,
			       ಸ್ಥಿರಾಂಕ html_content *content)
{
	ರಚನೆ box *ಮಗು;
	ಪೂರ್ಣಾಂಕ ಸಾಲು_min = 0, ಸಾಲು_max = 0;
	ಪೂರ್ಣಾಂಕ min = 0, max = 0;
	bool ಮೊದಲ_ಸಾಲು = true;
	bool ಸಾಲು_has_ಎತ್ತರ;

	ಪ್ರತಿಜ್ಞಾ(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	ಆದರೆ (inline_container->max_ಅಗಲ != UNKNOWN_MAX_ಅಗಲ)
		ಹಿಂತಿರುಗಿ;

	*has_ಎತ್ತರ = false;

	ಗಾಗಿ (ಮಗು = inline_container->children; ಮಗು; ) {
		ಮಗು = layout_minmax_ಸಾಲು(ಮಗು, &ಸಾಲು_min, &ಸಾಲು_max,
				ಮೊದಲ_ಸಾಲು, &ಸಾಲು_has_ಎತ್ತರ, font_func,
				content);
		ಆದರೆ (min < ಸಾಲು_min)
			min = ಸಾಲು_min;
		ಆದರೆ (max < ಸಾಲು_max)
			max = ಸಾಲು_max;
		ಮೊದಲ_ಸಾಲು = false;
		*has_ಎತ್ತರ |= ಸಾಲು_has_ಎತ್ತರ;
	}

	inline_container->min_ಅಗಲ = min;
	inline_container->max_ಅಗಲ = max;

	ಪ್ರತಿಜ್ಞಾ(0 <= inline_container->min_ಅಗಲ &&
			inline_container->min_ಅಗಲ <=
			inline_container->max_ಅಗಲ);
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
ಸ್ಥಿರ ಶೂನ್ಯ layout_minmax_ಖಂಡ(
		ರಚನೆ box *ಖಂಡ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ gui_layout_ಕೋಷ್ಟಕ *font_func,
		ಸ್ಥಿರಾಂಕ html_content *content)
{
	ರಚನೆ box *ಮಗು;
	ಪೂರ್ಣಾಂಕ min = 0, max = 0;
	ಪೂರ್ಣಾಂಕ extra_fixed = 0;
	ದಶಮಾಂಶ extra_frac = 0;
	ಎಣಿಕೆ css_ಅಗಲ_e wtype = CSS_ಅಗಲ_AUTO;
	css_fixed ಅಗಲ = 0;
	css_unit wunit = CSS_UNIT_PX;
	ಎಣಿಕೆ css_ಎತ್ತರ_e htype = CSS_ಎತ್ತರ_AUTO;
	css_fixed ಎತ್ತರ = 0;
	css_unit hunit = CSS_UNIT_PX;
	ಎಣಿಕೆ css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool ಮಗು_has_ಎತ್ತರ = false;

	ಪ್ರತಿಜ್ಞಾ(ಖಂಡ->type == BOX_ಖಂಡ ||
			ಖಂಡ->type == BOX_FLEX ||
			ಖಂಡ->type == BOX_INLINE_FLEX ||
			ಖಂಡ->type == BOX_INLINE_ಖಂಡ ||
			ಖಂಡ->type == BOX_ಕೋಷ್ಟಕ_CELL);

	/* check if the widths have already been calculated */
	ಆದರೆ (ಖಂಡ->max_ಅಗಲ != UNKNOWN_MAX_ಅಗಲ)
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (ಖಂಡ->style != ಖಾಲಿ) {
		wtype = css_computed_ಅಗಲ(ಖಂಡ->style, &ಅಗಲ, &wunit);
		htype = css_computed_ಎತ್ತರ(ಖಂಡ->style, &ಎತ್ತರ, &hunit);
		bs = css_computed_box_sizing(ಖಂಡ->style);
	}

	/* set whether the minimum width is of any interest for this box */
	ಆದರೆ (((ಖಂಡ->ಪೋಷಕ && lh__box_is_float_box(ಖಂಡ->ಪೋಷಕ)) ||
			ಖಂಡ->type == BOX_INLINE_ಖಂಡ ||
			ಖಂಡ->type == BOX_INLINE_FLEX) &&
			wtype != CSS_ಅಗಲ_ಗಣ) {
		/* box shrinks to fit; need minimum width */
		ಖಂಡ->flags |= NEED_MIN;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಖಂಡ->type == BOX_ಕೋಷ್ಟಕ_CELL) {
		/* box shrinks to fit; need minimum width */
		ಖಂಡ->flags |= NEED_MIN;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಖಂಡ->ಪೋಷಕ && (ಖಂಡ->ಪೋಷಕ->flags & NEED_MIN) &&
			wtype != CSS_ಅಗಲ_ಗಣ) {
		/* box inside shrink-to-fit context; need minimum width */
		ಖಂಡ->flags |= NEED_MIN;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಖಂಡ->ಪೋಷಕ && (ಖಂಡ->ಪೋಷಕ->type == BOX_FLEX)) {
		/* box is flex item */
		ಖಂಡ->flags |= NEED_MIN;
	}

	ಆದರೆ (ಖಂಡ->gadget && (ಖಂಡ->gadget->type == GADGET_TEXTBOX ||
			ಖಂಡ->gadget->type == GADGET_PASSWORD ||
			ಖಂಡ->gadget->type == GADGET_FILE ||
			ಖಂಡ->gadget->type == GADGET_TEXTAREA) &&
			ಖಂಡ->style && wtype == CSS_ಅಗಲ_AUTO) {
		css_fixed ಗಾತ್ರ_2 = INTTOFIX(10);
		css_unit unit = CSS_UNIT_EM;

		min = max = FIXTOINT(css_unit_len2device_px(ಖಂಡ->style,
				&content->unit_len_ctx, ಗಾತ್ರ_2, unit));

		ಖಂಡ->flags |= HAS_ಎತ್ತರ;
	}

	ಆದರೆ (ಖಂಡ->gadget && (ಖಂಡ->gadget->type == GADGET_RADIO ||
			ಖಂಡ->gadget->type == GADGET_CHECKBOX) &&
			ಖಂಡ->style && wtype == CSS_ಅಗಲ_AUTO) {
		css_fixed ಗಾತ್ರ_2 = INTTOFIX(1);
		css_unit unit = CSS_UNIT_EM;

		/* form checkbox or radio button
		 * if width is AUTO, set it to 1em */
		min = max = FIXTOINT(css_unit_len2device_px(ಖಂಡ->style,
				&content->unit_len_ctx, ಗಾತ್ರ_2, unit));

		ಖಂಡ->flags |= HAS_ಎತ್ತರ;
	}

	ಆದರೆ (ಖಂಡ->object) {
		ಆದರೆ (content_get_type(ಖಂಡ->object) == CONTENT_HTML) {
			layout_minmax_ಖಂಡ(html_get_box_ವೃಕ್ಷ(ಖಂಡ->object),
					font_func, content);
			min = html_get_box_ವೃಕ್ಷ(ಖಂಡ->object)->min_ಅಗಲ;
			max = html_get_box_ವೃಕ್ಷ(ಖಂಡ->object)->max_ಅಗಲ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			min = max = content_get_ಅಗಲ(ಖಂಡ->object);
		}

		ಖಂಡ->flags |= HAS_ಎತ್ತರ;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಖಂಡ->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		ಖಂಡ->flags |= HAS_ಎತ್ತರ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* recurse through children */
		ಗಾಗಿ (ಮಗು = ಖಂಡ->children; ಮಗು; ಮಗು = ಮಗು->ಮುಂದಿನ) {
			ಆಯ್ಕೆ (ಮಗು->type) {
			ಸ್ಥಿತಿ BOX_FLEX:
			ಸ್ಥಿತಿ BOX_ಖಂಡ:
				layout_minmax_ಖಂಡ(ಮಗು, font_func,
						content);
				ಆದರೆ (ಮಗು->flags & HAS_ಎತ್ತರ)
					ಮಗು_has_ಎತ್ತರ = true;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ BOX_INLINE_CONTAINER:
				ಆದರೆ (ಖಂಡ->flags & NEED_MIN)
					ಮಗು->flags |= NEED_MIN;

				layout_minmax_inline_container(ಮಗು,
						&ಮಗು_has_ಎತ್ತರ, font_func,
						content);
				ಆದರೆ (ಮಗು_has_ಎತ್ತರ &&
						ಮಗು ==
						ಮಗು->ಪೋಷಕ->children) {
					ಖಂಡ->flags |= MAKE_ಎತ್ತರ;
				}
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ:
				layout_minmax_ಕೋಷ್ಟಕ(ಮಗು, font_func,
						content);
				/* todo: fix for zero height tables */
				ಮಗು_has_ಎತ್ತರ = true;
				ಮಗು->flags |= MAKE_ಎತ್ತರ;
				ನಿಲ್ಲಿಸು;
			ಪೂರ್ವನಿಯೋಜಿತ:
				ಪ್ರತಿಜ್ಞಾ(0);
			}
			ಪ್ರತಿಜ್ಞಾ(ಮಗು->max_ಅಗಲ != UNKNOWN_MAX_ಅಗಲ);

			ಆದರೆ (ಮಗು->style &&
					(css_computed_position(ಮಗು->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(ಮಗು->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				ಮುಂದುವರಿ;
			}

			ಆದರೆ (lh__box_is_flex_container(ಖಂಡ) &&
			    lh__flex_main_is_horizontal(ಖಂಡ)) {
				ಆದರೆ (ಖಂಡ->style != ಖಾಲಿ &&
				    css_computed_flex_wrap(ಖಂಡ->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += ಮಗು->min_ಅಗಲ;
				} ಇಲ್ಲದಿದ್ದರೆ {
					ಆದರೆ (min < ಮಗು->min_ಅಗಲ)
						min = ಮಗು->min_ಅಗಲ;
				}
				max += ಮಗು->max_ಅಗಲ;

			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (min < ಮಗು->min_ಅಗಲ)
					min = ಮಗು->min_ಅಗಲ;
				ಆದರೆ (max < ಮಗು->max_ಅಗಲ)
					max = ಮಗು->max_ಅಗಲ;
			}

			ಆದರೆ (ಮಗು_has_ಎತ್ತರ)
				ಖಂಡ->flags |= HAS_ಎತ್ತರ;
		}
	}

	ಆದರೆ (max < min) {
		box_dump(stderr, ಖಂಡ, 0, true);
		ಪ್ರತಿಜ್ಞಾ(0);
	}

	/* fixed width takes priority */
	ಆದರೆ (ಖಂಡ->type != BOX_ಕೋಷ್ಟಕ_CELL && !lh_box_is_flex_ಅಂಶ(ಖಂಡ)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		ಎಣಿಕೆ css_max_ಅಗಲ_e max_type;
		ಎಣಿಕೆ css_min_ಅಗಲ_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed ಮೌಲ್ಯ = 0;
		ಪೂರ್ಣಾಂಕ ಅಗಲ;

		ಆದರೆ (css_computed_ಅಗಲ_px(ಖಂಡ->style, &content->unit_len_ctx,
				-1, &ಅಗಲ) == CSS_ಅಗಲ_ಗಣ) {
			min = max = ಅಗಲ;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_ಅಗಲ(ಖಂಡ->style, &ಮೌಲ್ಯ, &unit);
		ಆದರೆ (min_type == CSS_MIN_ಅಗಲ_ಗಣ && unit != CSS_UNIT_PCT) {
			ಪೂರ್ಣಾಂಕ val = FIXTOINT(css_unit_len2device_px(ಖಂಡ->style,
					&content->unit_len_ctx, ಮೌಲ್ಯ, unit));

			ಆದರೆ (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_ಅಗಲ(ಖಂಡ->style, &ಮೌಲ್ಯ, &unit);
		ಆದರೆ (max_type == CSS_MAX_ಅಗಲ_ಗಣ && unit != CSS_UNIT_PCT) {
			ಪೂರ್ಣಾಂಕ val = FIXTOINT(css_unit_len2device_px(ಖಂಡ->style,
					&content->unit_len_ctx, ಮೌಲ್ಯ, unit));

			ಆದರೆ (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	ಆದರೆ (htype == CSS_ಎತ್ತರ_ಗಣ && hunit != CSS_UNIT_PCT &&
			ಎತ್ತರ > INTTOFIX(0)) {
		ಖಂಡ->flags |= MAKE_ಎತ್ತರ;
		ಖಂಡ->flags |= HAS_ಎತ್ತರ;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_ಅಗಲ(&content->unit_len_ctx, ಖಂಡ->style, ಎಡ_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_ಅಗಲ(&content->unit_len_ctx, ಖಂಡ->style, ಬಲ_2,
			false, true, true, &extra_fixed, &extra_frac);

	ಆದರೆ (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	ಆದರೆ (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	ಆದರೆ (max < min) {
		min = max;
	}

	calculate_mbp_ಅಗಲ(&content->unit_len_ctx, ಖಂಡ->style, ಎಡ_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_ಅಗಲ(&content->unit_len_ctx, ಖಂಡ->style, ಬಲ_2,
			true, false, false, &extra_fixed, &extra_frac);

	ಆದರೆ (extra_fixed < 0)
		extra_fixed = 0;
	ಆದರೆ (extra_frac < 0)
		extra_frac = 0;
	ಆದರೆ (1.0 <= extra_frac)
		extra_frac = 0.9;
	ಆದರೆ (ಖಂಡ->style != ಖಾಲಿ &&
			(css_computed_float(ಖಂಡ->style) == CSS_FLOAT_ಎಡ ||
			css_computed_float(ಖಂಡ->style) == CSS_FLOAT_ಬಲ)) {
		/* floated boxs */
		ಖಂಡ->min_ಅಗಲ = min + extra_fixed;
		ಖಂಡ->max_ಅಗಲ = max + extra_fixed;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* not floated */
		ಖಂಡ->min_ಅಗಲ = (min + extra_fixed) / (1.0 - extra_frac);
		ಖಂಡ->max_ಅಗಲ = (max + extra_fixed) / (1.0 - extra_frac);
	}

	ಪ್ರತಿಜ್ಞಾ(0 <= ಖಂಡ->min_ಅಗಲ);
	ಪ್ರತಿಜ್ಞಾ(ಖಂಡ->min_ಅಗಲ <= ಖಂಡ->max_ಅಗಲ);
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
ಸ್ಥಿರ ರಚನೆ box*
layout_ಮುಂದಿನ_margin_ಖಂಡ(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
			 ರಚನೆ box *box,
			 ರಚನೆ box *ಖಂಡ,
			 ಪೂರ್ಣಾಂಕ viewport_ಎತ್ತರ,
			 ಪೂರ್ಣಾಂಕ *max_pos_margin,
			 ಪೂರ್ಣಾಂಕ *max_neg_margin)
{
	ಪ್ರತಿಜ್ಞಾ(ಖಂಡ != ಖಾಲಿ);

	ಆಗ (box != ಖಾಲಿ) {

		ಆದರೆ (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			ಆದರೆ (box->style) {
				layout_ಕಂಡುಹಿಡಿ_dimensions(unit_len_ctx,
						box->ಪೋಷಕ->ಅಗಲ,
						viewport_ಎತ್ತರ, box,
						box->style,
						ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ,
						ಖಾಲಿ, ಖಾಲಿ, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				ಆದರೆ (*max_pos_margin < box->margin[ತುದಿ_2])
					*max_pos_margin = box->margin[ತುದಿ_2];
				ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*max_neg_margin < -box->margin[ತುದಿ_2])
					*max_neg_margin = -box->margin[ತುದಿ_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			ಆದರೆ (box->flags & MAKE_ಎತ್ತರ ||
					box->border[ತುದಿ_2].ಅಗಲ ||
					box->padding[ತುದಿ_2] ||
					(box->style &&
					css_computed_ಅತಿಪ್ರವಾಹ_y(box->style) !=
					CSS_ಅತಿಪ್ರವಾಹ_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_ಮೊದಲ_ಮಗು(box))) {
				/* Collapse to this box; return it */
				ಹಿಂತಿರುಗಿ box;
			}
		}


		/* Find next box */
		ಆದರೆ (box->type == BOX_ಖಂಡ && !box->object && box->children &&
				box->style &&
				css_computed_ಅತಿಪ್ರವಾಹ_y(box->style) ==
				CSS_ಅತಿಪ್ರವಾಹ_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (!box->ಮುಂದಿನ) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				ಮಾಡು {
					/* Apply bottom margin */
					ಆದರೆ (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->ಪೋಷಕ;
				} ಆಗ (box != ಖಂಡ && !box->ಮುಂದಿನ);

				ಆದರೆ (box == ಖಂಡ) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					ಹಿಂತಿರುಗಿ ಖಂಡ;
				}
			}

			/* Apply bottom margin */
			ಆದರೆ (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->ಮುಂದಿನ;

			/* Get margins */
			ಆದರೆ (box->style) {
				layout_ಕಂಡುಹಿಡಿ_dimensions(unit_len_ctx,
						box->ಪೋಷಕ->ಅಗಲ,
						viewport_ಎತ್ತರ, box,
						box->style,
						ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ,
						ಖಾಲಿ, ಖಾಲಿ, box->margin,
						box->padding, box->border);
			}
		}
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ layout_clear(ರಚನೆ box *fl, ಎಣಿಕೆ css_clear_e clear)
{
	ಪೂರ್ಣಾಂಕ y = 0;
	ಗಾಗಿ (; fl; fl = fl->ಮುಂದಿನ_float) {
		ಆದರೆ ((clear == CSS_CLEAR_ಎಡ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ಎಡ)
			ಆದರೆ (y < fl->y + fl->ಎತ್ತರ)
				y = fl->y + fl->ಎತ್ತರ;
		ಆದರೆ ((clear == CSS_CLEAR_ಬಲ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ಬಲ)
			ಆದರೆ (y < fl->y + fl->ಎತ್ತರ)
				y = fl->y + fl->ಎತ್ತರ;
	}
	ಹಿಂತಿರುಗಿ y;
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
ಸ್ಥಿರ ಶೂನ್ಯ
ಕಂಡುಹಿಡಿ_sides(ರಚನೆ box *fl,
	   ಪೂರ್ಣಾಂಕ y0, ಪೂರ್ಣಾಂಕ y1,
	   ಪೂರ್ಣಾಂಕ *x0, ಪೂರ್ಣಾಂಕ *x1,
	   ರಚನೆ box **ಎಡ,
	   ರಚನೆ box **ಬಲ)
{
	ಪೂರ್ಣಾಂಕ fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*ಎಡ = *ಬಲ = 0;
	ಗಾಗಿ (; fl; fl = fl->ಮುಂದಿನ_float) {
		fy1 = fl->y + fl->ಎತ್ತರ;
		ಆದರೆ (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			ಹಿಂತಿರುಗಿ;
		}
		fy0 = fl->y;
		ಆದರೆ (y0 < fy1 && fy0 <= y1) {
			ಆದರೆ (fl->type == BOX_FLOAT_ಎಡ) {
				fx1 = fl->x + fl->ಅಗಲ;
				ಆದರೆ (*x0 < fx1) {
					*x0 = fx1;
					*ಎಡ = fl;
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				fx0 = fl->x;
				ಆದರೆ (fx0 < *x1) {
					*x1 = fx0;
					*ಬಲ = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *ಎಡ, *ಬಲ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
layout_solve_ಅಗಲ(ರಚನೆ box *box,
		   ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ,
		   ಪೂರ್ಣಾಂಕ ಅಗಲ,
		   ಪೂರ್ಣಾಂಕ lm,
		   ಪೂರ್ಣಾಂಕ rm,
		   ಪೂರ್ಣಾಂಕ max_ಅಗಲ,
		   ಪೂರ್ಣಾಂಕ min_ಅಗಲ)
{
	bool auto_ಅಗಲ = false;

	/* Increase specified left/right margins */
	ಆದರೆ (box->margin[ಎಡ_2] != AUTO && box->margin[ಎಡ_2] < lm &&
			box->margin[ಎಡ_2] >= 0)
		box->margin[ಎಡ_2] = lm;
	ಆದರೆ (box->margin[ಬಲ_2] != AUTO && box->margin[ಬಲ_2] < rm &&
			box->margin[ಬಲ_2] >= 0)
		box->margin[ಬಲ_2] = rm;

	/* Find width */
	ಆದರೆ (ಅಗಲ == AUTO) {
		ಪೂರ್ಣಾಂಕ margin_ಎಡ = box->margin[ಎಡ_2];
		ಪೂರ್ಣಾಂಕ margin_ಬಲ = box->margin[ಬಲ_2];

		ಆದರೆ (margin_ಎಡ == AUTO) {
			margin_ಎಡ = lm;
		}
		ಆದರೆ (margin_ಬಲ == AUTO) {
			margin_ಬಲ = rm;
		}

		ಅಗಲ = ಲಭ್ಯ_ಅಗಲ -
				(margin_ಎಡ + box->border[ಎಡ_2].ಅಗಲ +
				box->padding[ಎಡ_2] + box->padding[ಬಲ_2] +
				box->border[ಬಲ_2].ಅಗಲ + margin_ಬಲ);
		ಅಗಲ = ಅಗಲ < 0 ? 0 : ಅಗಲ;
		auto_ಅಗಲ = true;
	}

	ಆದರೆ (max_ಅಗಲ >= 0 && ಅಗಲ > max_ಅಗಲ) {
		/* max-width is admissable and width exceeds max-width */
		ಅಗಲ = max_ಅಗಲ;
		auto_ಅಗಲ = false;
	}

	ಆದರೆ (min_ಅಗಲ > 0 && ಅಗಲ < min_ಅಗಲ) {
		/* min-width is admissable and width is less than max-width */
		ಅಗಲ = min_ಅಗಲ;
		auto_ಅಗಲ = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	ಆದರೆ (auto_ಅಗಲ) {
		/* any other 'auto' become 0 or the minimum required values */
		ಆದರೆ (box->margin[ಎಡ_2] == AUTO) {
			box->margin[ಎಡ_2] = lm;
		}
		ಆದರೆ (box->margin[ಬಲ_2] == AUTO) {
			box->margin[ಬಲ_2] = rm;
		}
		ಹಿಂತಿರುಗಿ ಅಗಲ;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	ಆದರೆ (box->margin[ಎಡ_2] != AUTO && box->margin[ಬಲ_2] != AUTO &&
			box->ಪೋಷಕ != ಖಾಲಿ && box->ಪೋಷಕ->style != ಖಾಲಿ) {
		ಆಯ್ಕೆ (css_computed_ಪಠ್ಯ_align(box->ಪೋಷಕ->style)) {
		ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_LIBCSS_ಬಲ:
			box->margin[ಎಡ_2] = AUTO;
			box->margin[ಬಲ_2] = 0;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_LIBCSS_CENTER:
			box->margin[ಎಡ_2] = box->margin[ಬಲ_2] = AUTO;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_LIBCSS_ಎಡ:
			box->margin[ಎಡ_2] = 0;
			box->margin[ಬಲ_2] = AUTO;
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			/* Leave it alone; no HTML alignment */
			ನಿಲ್ಲಿಸು;
		}
	}

	ಆದರೆ (box->margin[ಎಡ_2] == AUTO && box->margin[ಬಲ_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[ಎಡ_2] = box->margin[ಬಲ_2] =
				(ಲಭ್ಯ_ಅಗಲ - lm - rm -
				(box->border[ಎಡ_2].ಅಗಲ + box->padding[ಎಡ_2] +
				ಅಗಲ + box->padding[ಬಲ_2] +
				box->border[ಬಲ_2].ಅಗಲ)) / 2;

		ಆದರೆ (box->margin[ಎಡ_2] < 0) {
			box->margin[ಬಲ_2] += box->margin[ಎಡ_2];
			box->margin[ಎಡ_2] = 0;
		}

		box->margin[ಎಡ_2] += lm;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->margin[ಎಡ_2] == AUTO) {
		box->margin[ಎಡ_2] = ಲಭ್ಯ_ಅಗಲ - lm -
				(box->border[ಎಡ_2].ಅಗಲ + box->padding[ಎಡ_2] +
				ಅಗಲ + box->padding[ಬಲ_2] +
				box->border[ಬಲ_2].ಅಗಲ + box->margin[ಬಲ_2]);
		box->margin[ಎಡ_2] = box->margin[ಎಡ_2] < lm
				? lm : box->margin[ಎಡ_2];
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* margin-right auto or "over-constrained" */
		box->margin[ಬಲ_2] = ಲಭ್ಯ_ಅಗಲ - rm -
				(box->margin[ಎಡ_2] + box->border[ಎಡ_2].ಅಗಲ +
				 box->padding[ಎಡ_2] + ಅಗಲ +
				 box->padding[ಬಲ_2] +
				 box->border[ಬಲ_2].ಅಗಲ);
	}

	ಹಿಂತಿರುಗಿ ಅಗಲ;
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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_ಖಂಡ_ಕಂಡುಹಿಡಿ_dimensions(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
			     ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ,
			     ಪೂರ್ಣಾಂಕ viewport_ಎತ್ತರ,
			     ಪೂರ್ಣಾಂಕ lm,
			     ಪೂರ್ಣಾಂಕ rm,
			     ರಚನೆ box *box)
{
	ಪೂರ್ಣಾಂಕ ಅಗಲ, max_ಅಗಲ, min_ಅಗಲ;
	ಪೂರ್ಣಾಂಕ ಎತ್ತರ, max_ಎತ್ತರ, min_ಎತ್ತರ;
	ಪೂರ್ಣಾಂಕ *margin = box->margin;
	ಪೂರ್ಣಾಂಕ *padding = box->padding;
	ರಚನೆ box_border *border = box->border;
	ಸ್ಥಿರಾಂಕ css_computed_style *style = box->style;

	layout_ಕಂಡುಹಿಡಿ_dimensions(unit_len_ctx, ಲಭ್ಯ_ಅಗಲ, viewport_ಎತ್ತರ, box,
			style, &ಅಗಲ, &ಎತ್ತರ, &max_ಅಗಲ, &min_ಅಗಲ,
			&max_ಎತ್ತರ, &min_ಎತ್ತರ, margin, padding, border);

	ಆದರೆ (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &ಅಗಲ, &ಎತ್ತರ,
				min_ಅಗಲ, max_ಅಗಲ, min_ಎತ್ತರ, max_ಎತ್ತರ);
	}

	box->ಅಗಲ = layout_solve_ಅಗಲ(box, ಲಭ್ಯ_ಅಗಲ, ಅಗಲ, lm, rm,
			max_ಅಗಲ, min_ಅಗಲ);
	box->ಎತ್ತರ = ಎತ್ತರ;

	ಆದರೆ (margin[ತುದಿ_2] == AUTO)
		margin[ತುದಿ_2] = 0;
	ಆದರೆ (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_ಖಂಡ_ಕೂಡಿಸು_scrollbar(ರಚನೆ box *box, ಪೂರ್ಣಾಂಕ which)
{
	ಎಣಿಕೆ css_ಅತಿಪ್ರವಾಹ_e ಅತಿಪ್ರವಾಹ_x, ಅತಿಪ್ರವಾಹ_y;

	ಪ್ರತಿಜ್ಞಾ(box->type == BOX_ಖಂಡ && (which == ಬಲ_2 || which == BOTTOM));

	ಆದರೆ (box->style == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ;

	ಅತಿಪ್ರವಾಹ_x = css_computed_ಅತಿಪ್ರವಾಹ_x(box->style);
	ಅತಿಪ್ರವಾಹ_y = css_computed_ಅತಿಪ್ರವಾಹ_y(box->style);

	ಆದರೆ (which == BOTTOM &&
			(ಅತಿಪ್ರವಾಹ_x == CSS_ಅತಿಪ್ರವಾಹ_SCROLL ||
			 ಅತಿಪ್ರವಾಹ_x == CSS_ಅತಿಪ್ರವಾಹ_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		ಆದರೆ (box->ಎತ್ತರ != AUTO &&
				(ಅತಿಪ್ರವಾಹ_x == CSS_ಅತಿಪ್ರವಾಹ_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_ಅಗಲ;
		}

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (which == ಬಲ_2 &&
			(ಅತಿಪ್ರವಾಹ_y == CSS_ಅತಿಪ್ರವಾಹ_SCROLL ||
			 ಅತಿಪ್ರವಾಹ_y == CSS_ಅತಿಪ್ರವಾಹ_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		ಎಣಿಕೆ css_ಎತ್ತರ_e htype;
		css_fixed ಎತ್ತರ = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_ಎತ್ತರ(box->style, &ಎತ್ತರ, &hunit);

		ಆದರೆ (which == ಬಲ_2 && box->ಅಗಲ != AUTO &&
				htype == CSS_ಎತ್ತರ_ಗಣ &&
				(ಅತಿಪ್ರವಾಹ_y == CSS_ಅತಿಪ್ರವಾಹ_SCROLL ||
				box_vscrollbar_present(box))) {
			box->ಅಗಲ -= SCROLLBAR_ಅಗಲ;
			box->padding[ಬಲ_2] += SCROLLBAR_ಅಗಲ;
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
ಸ್ಥಿರ ಶೂನ್ಯ layout_move_children(ರಚನೆ box *box, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ಪ್ರತಿಜ್ಞಾ(box);

	ಗಾಗಿ (box = box->children; box; box = box->ಮುಂದಿನ) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_ಕೋಷ್ಟಕ(
		ರಚನೆ box *ಕೋಷ್ಟಕ,
		ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ,
		html_content *content)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ columns = ಕೋಷ್ಟಕ->columns;  /* total columns */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *ಅಡ್ಡಸಾಲು_ಹರವು;
	ಪೂರ್ಣಾಂಕ *excess_y;
	ಪೂರ್ಣಾಂಕ ಕೋಷ್ಟಕ_ಅಗಲ, min_ಅಗಲ = 0, max_ಅಗಲ = 0;
	ಪೂರ್ಣಾಂಕ required_ಅಗಲ = 0;
	ಪೂರ್ಣಾಂಕ x, remainder = 0, ಎಣಿಕೆ_2 = 0;
	ಪೂರ್ಣಾಂಕ ಕೋಷ್ಟಕ_ಎತ್ತರ = 0;
	ಪೂರ್ಣಾಂಕ min_ಎತ್ತರ = 0;
	ಪೂರ್ಣಾಂಕ *xs;  /* array of column x positions */
	ಪೂರ್ಣಾಂಕ auto_ಅಗಲ;
	ಪೂರ್ಣಾಂಕ ಹೆಚ್ಚುವರಿ_ಅಗಲ;
	ಪೂರ್ಣಾಂಕ relative_ಮೊತ್ತ = 0;
	ಪೂರ್ಣಾಂಕ border_spacing_h = 0, border_spacing_v = 0;
	ಪೂರ್ಣಾಂಕ ಹೆಚ್ಚುವರಿ_ಎತ್ತರ;
	ಪೂರ್ಣಾಂಕ positioned_columns = 0;
	ರಚನೆ box *containing_ಖಂಡ = ಖಾಲಿ;
	ರಚನೆ box *c;
	ರಚನೆ box *ಅಡ್ಡಸಾಲು;
	ರಚನೆ box *ಅಡ್ಡಸಾಲು_ಗುಂಪು;
	ರಚನೆ box **ಅಡ್ಡಸಾಲು_ಹರವು_cell;
	ರಚನೆ ಸ್ತಂಭ *ಲಂಬಸಾಲು;
	ಸ್ಥಿರಾಂಕ css_computed_style *style = ಕೋಷ್ಟಕ->style;
	ಎಣಿಕೆ css_ಎತ್ತರ_e htype;
	css_fixed ಮೌಲ್ಯ = 0;
	css_unit unit = CSS_UNIT_PX;

	ಪ್ರತಿಜ್ಞಾ(ಕೋಷ್ಟಕ->type == BOX_ಕೋಷ್ಟಕ);
	ಪ್ರತಿಜ್ಞಾ(style);
	ಪ್ರತಿಜ್ಞಾ(ಕೋಷ್ಟಕ->children && ಕೋಷ್ಟಕ->children->children);
	ಪ್ರತಿಜ್ಞಾ(columns);

	/* allocate working buffers */
	ಲಂಬಸಾಲು = ಸ್ಮರಣೆ_ಕೊಡು(columns * ಗಾತ್ರ ಲಂಬಸಾಲು[0]);
	excess_y = ಸ್ಮರಣೆ_ಕೊಡು(columns * ಗಾತ್ರ excess_y[0]);
	ಅಡ್ಡಸಾಲು_ಹರವು = ಸ್ಮರಣೆ_ಕೊಡು(columns * ಗಾತ್ರ ಅಡ್ಡಸಾಲು_ಹರವು[0]);
	ಅಡ್ಡಸಾಲು_ಹರವು_cell = ಸ್ಮರಣೆ_ಕೊಡು(columns * ಗಾತ್ರ ಅಡ್ಡಸಾಲು_ಹರವು_cell[0]);
	xs = ಸ್ಮರಣೆ_ಕೊಡು((columns + 1) * ಗಾತ್ರ xs[0]);
	ಆದರೆ (!ಲಂಬಸಾಲು || !xs || !ಅಡ್ಡಸಾಲು_ಹರವು || !excess_y || !ಅಡ್ಡಸಾಲು_ಹರವು_cell) {
		ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು);
		ಬಿಡುಗಡೆ(excess_y);
		ಬಿಡುಗಡೆ(ಅಡ್ಡಸಾಲು_ಹರವು);
		ಬಿಡುಗಡೆ(ಅಡ್ಡಸಾಲು_ಹರವು_cell);
		ಬಿಡುಗಡೆ(xs);
		ಹಿಂತಿರುಗಿ false;
	}

	ಸ್ಮರಣೆ_ನಕಲು(ಲಂಬಸಾಲು, ಕೋಷ್ಟಕ->ಲಂಬಸಾಲು, ಗಾತ್ರ(ಲಂಬಸಾಲು[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_ಕಂಡುಹಿಡಿ_dimensions(&content->unit_len_ctx, ಲಭ್ಯ_ಅಗಲ, -1, ಕೋಷ್ಟಕ,
			style, 0, 0, 0, 0, 0, 0, ಕೋಷ್ಟಕ->margin, ಕೋಷ್ಟಕ->padding,
			ಕೋಷ್ಟಕ->border);
	ಗಾಗಿ (ಅಡ್ಡಸಾಲು_ಗುಂಪು = ಕೋಷ್ಟಕ->children; ಅಡ್ಡಸಾಲು_ಗುಂಪು;
			ಅಡ್ಡಸಾಲು_ಗುಂಪು = ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಮುಂದಿನ) {
		ಗಾಗಿ (ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು_ಗುಂಪು->children; ಅಡ್ಡಸಾಲು; ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->ಮುಂದಿನ) {
			ಗಾಗಿ (c = ಅಡ್ಡಸಾಲು->children; c; c = c->ಮುಂದಿನ) {
				ಎಣಿಕೆ css_ಅತಿಪ್ರವಾಹ_e ಅತಿಪ್ರವಾಹ_x;
				ಎಣಿಕೆ css_ಅತಿಪ್ರವಾಹ_e ಅತಿಪ್ರವಾಹ_y;

				ಪ್ರತಿಜ್ಞಾ(c->style);
				ಕೋಷ್ಟಕ_ಬಳಸಿದ_border_for_cell(
						&content->unit_len_ctx, c);
				layout_ಕಂಡುಹಿಡಿ_dimensions(&content->unit_len_ctx,
						ಲಭ್ಯ_ಅಗಲ, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				ಅತಿಪ್ರವಾಹ_x = css_computed_ಅತಿಪ್ರವಾಹ_x(c->style);
				ಅತಿಪ್ರವಾಹ_y = css_computed_ಅತಿಪ್ರವಾಹ_y(c->style);

				ಆದರೆ (ಅತಿಪ್ರವಾಹ_x == CSS_ಅತಿಪ್ರವಾಹ_SCROLL ||
						ಅತಿಪ್ರವಾಹ_x ==
						CSS_ಅತಿಪ್ರವಾಹ_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_ಅಗಲ;
				}
				ಆದರೆ (ಅತಿಪ್ರವಾಹ_y == CSS_ಅತಿಪ್ರವಾಹ_SCROLL ||
						ಅತಿಪ್ರವಾಹ_y ==
						CSS_ಅತಿಪ್ರವಾಹ_AUTO) {
					c->padding[ಬಲ_2] += SCROLLBAR_ಅಗಲ;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	ಆದರೆ (css_computed_border_collapse(style) ==
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
	ಆದರೆ (css_computed_ಅಗಲ_px(style, &content->unit_len_ctx,
			ಲಭ್ಯ_ಅಗಲ, &ಕೋಷ್ಟಕ_ಅಗಲ) == CSS_ಅಗಲ_ಗಣ) {
		/* specified width includes border */
		ಕೋಷ್ಟಕ_ಅಗಲ -= ಕೋಷ್ಟಕ->border[ಎಡ_2].ಅಗಲ +
				ಕೋಷ್ಟಕ->border[ಬಲ_2].ಅಗಲ;
		ಕೋಷ್ಟಕ_ಅಗಲ = ಕೋಷ್ಟಕ_ಅಗಲ < 0 ? 0 : ಕೋಷ್ಟಕ_ಅಗಲ;

		auto_ಅಗಲ = ಕೋಷ್ಟಕ_ಅಗಲ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಕೋಷ್ಟಕ_ಅಗಲ = AUTO;
		auto_ಅಗಲ = ಲಭ್ಯ_ಅಗಲ -
				((ಕೋಷ್ಟಕ->margin[ಎಡ_2] == AUTO ? 0 :
						ಕೋಷ್ಟಕ->margin[ಎಡ_2]) +
				 ಕೋಷ್ಟಕ->border[ಎಡ_2].ಅಗಲ +
				 ಕೋಷ್ಟಕ->padding[ಎಡ_2] +
				 ಕೋಷ್ಟಕ->padding[ಬಲ_2] +
				 ಕೋಷ್ಟಕ->border[ಬಲ_2].ಅಗಲ +
				 (ಕೋಷ್ಟಕ->margin[ಬಲ_2] == AUTO ? 0 :
						ಕೋಷ್ಟಕ->margin[ಬಲ_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_ಎತ್ತರ(style, &ಮೌಲ್ಯ, &unit);
	ಆದರೆ (htype == CSS_ಎತ್ತರ_ಗಣ) {
		ಆದರೆ (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			ಆದರೆ (css_computed_position(ಕೋಷ್ಟಕ->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				ಪ್ರತಿಜ್ಞಾ(ಕೋಷ್ಟಕ->float_container);
				containing_ಖಂಡ = ಕೋಷ್ಟಕ->float_container;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೋಷ್ಟಕ->float_container &&
					css_computed_position(ಕೋಷ್ಟಕ->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(ಕೋಷ್ಟಕ->style) ==
						CSS_FLOAT_ಎಡ ||
					css_computed_float(ಕೋಷ್ಟಕ->style) ==
						CSS_FLOAT_ಬಲ)) {
				/* Table is a float */
				ಪ್ರತಿಜ್ಞಾ(ಕೋಷ್ಟಕ->ಪೋಷಕ && ಕೋಷ್ಟಕ->ಪೋಷಕ->ಪೋಷಕ &&
						ಕೋಷ್ಟಕ->ಪೋಷಕ->ಪೋಷಕ->ಪೋಷಕ);
				containing_ಖಂಡ =
						ಕೋಷ್ಟಕ->ಪೋಷಕ->ಪೋಷಕ->ಪೋಷಕ;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೋಷ್ಟಕ->ಪೋಷಕ && ಕೋಷ್ಟಕ->ಪೋಷಕ->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_ಖಂಡ = ಕೋಷ್ಟಕ->ಪೋಷಕ;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೋಷ್ಟಕ->ಪೋಷಕ && ಕೋಷ್ಟಕ->ಪೋಷಕ->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				ಪ್ರತಿಜ್ಞಾ(ಕೋಷ್ಟಕ->ಪೋಷಕ->ಪೋಷಕ);
				containing_ಖಂಡ = ಕೋಷ್ಟಕ->ಪೋಷಕ->ಪೋಷಕ;
			}

			ಆದರೆ (containing_ಖಂಡ) {
				css_fixed ignored = 0;

				htype = css_computed_ಎತ್ತರ(
						containing_ಖಂಡ->style,
						&ignored, &unit);
			}

			ಆದರೆ (containing_ಖಂಡ &&
					containing_ಖಂಡ->ಎತ್ತರ != AUTO &&
					(css_computed_position(ಕೋಷ್ಟಕ->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_ಎತ್ತರ_ಗಣ)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_ಎತ್ತರ = FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ,
						containing_ಖಂಡ->ಎತ್ತರ);
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_ಎತ್ತರ = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		}
	}

	/* calculate width required by cells */
	ಗಾಗಿ (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      ಕೋಷ್ಟಕ,
		      i,
		      ((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[ಲಂಬಸಾಲು[i].type],
		      ಲಂಬಸಾಲು[i].ಅಗಲ,
		      ಲಂಬಸಾಲು[i].min,
		      ಲಂಬಸಾಲು[i].max);


		ಆದರೆ (ಲಂಬಸಾಲು[i].positioned) {
			positioned_columns++;
			ಮುಂದುವರಿ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಲಂಬಸಾಲು[i].type == ಸ್ತಂಭ_ಅಗಲ_FIXED) {
			ಆದರೆ (ಲಂಬಸಾಲು[i].ಅಗಲ < ಲಂಬಸಾಲು[i].min)
				ಲಂಬಸಾಲು[i].ಅಗಲ = ಲಂಬಸಾಲು[i].max = ಲಂಬಸಾಲು[i].min;
			ಇಲ್ಲದಿದ್ದರೆ
				ಲಂಬಸಾಲು[i].min = ಲಂಬಸಾಲು[i].max = ಲಂಬಸಾಲು[i].ಅಗಲ;
			required_ಅಗಲ += ಲಂಬಸಾಲು[i].ಅಗಲ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಲಂಬಸಾಲು[i].type == ಸ್ತಂಭ_ಅಗಲ_PERCENT) {
			ಪೂರ್ಣಾಂಕ ಅಗಲ = ಲಂಬಸಾಲು[i].ಅಗಲ * auto_ಅಗಲ / 100;
			required_ಅಗಲ += ಲಂಬಸಾಲು[i].min < ಅಗಲ ? ಅಗಲ :
					ಲಂಬಸಾಲು[i].min;
		} ಇಲ್ಲದಿದ್ದರೆ
			required_ಅಗಲ += ಲಂಬಸಾಲು[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_ಅಗಲ);
	}
	required_ಅಗಲ += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", ಕೋಷ್ಟಕ_ಅಗಲ,
	      ಕೋಷ್ಟಕ->min_ಅಗಲ, ಕೋಷ್ಟಕ->max_ಅಗಲ, auto_ಅಗಲ, required_ಅಗಲ);

	ಆದರೆ (auto_ಅಗಲ < required_ಅಗಲ) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		ಗಾಗಿ (i = 0; i != columns; i++) {
			ಆದರೆ (ಲಂಬಸಾಲು[i].type == ಸ್ತಂಭ_ಅಗಲ_RELATIVE)
				ಮುಂದುವರಿ;
			ಆದರೆ (ಲಂಬಸಾಲು[i].type == ಸ್ತಂಭ_ಅಗಲ_PERCENT) {
				ಲಂಬಸಾಲು[i].max = auto_ಅಗಲ * ಲಂಬಸಾಲು[i].ಅಗಲ / 100;
				ಆದರೆ (ಲಂಬಸಾಲು[i].max < ಲಂಬಸಾಲು[i].min)
					ಲಂಬಸಾಲು[i].max = ಲಂಬಸಾಲು[i].min;
			}
			min_ಅಗಲ += ಲಂಬಸಾಲು[i].min;
			max_ಅಗಲ += ಲಂಬಸಾಲು[i].max;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* take percentages exactly */
		ಗಾಗಿ (i = 0; i != columns; i++) {
			ಆದರೆ (ಲಂಬಸಾಲು[i].type == ಸ್ತಂಭ_ಅಗಲ_RELATIVE)
				ಮುಂದುವರಿ;
			ಆದರೆ (ಲಂಬಸಾಲು[i].type == ಸ್ತಂಭ_ಅಗಲ_PERCENT) {
				ಪೂರ್ಣಾಂಕ ಅಗಲ = auto_ಅಗಲ * ಲಂಬಸಾಲು[i].ಅಗಲ / 100;
				ಆದರೆ (ಅಗಲ < ಲಂಬಸಾಲು[i].min)
					ಅಗಲ = ಲಂಬಸಾಲು[i].min;
				ಲಂಬಸಾಲು[i].min = ಲಂಬಸಾಲು[i].ಅಗಲ = ಲಂಬಸಾಲು[i].max = ಅಗಲ;
				ಲಂಬಸಾಲು[i].type = ಸ್ತಂಭ_ಅಗಲ_FIXED;
			}
			min_ಅಗಲ += ಲಂಬಸಾಲು[i].min;
			max_ಅಗಲ += ಲಂಬಸಾಲು[i].max;
		}
	}

	/* allocate relative widths */
	ಹೆಚ್ಚುವರಿ_ಅಗಲ = auto_ಅಗಲ;
	ಗಾಗಿ (i = 0; i != columns; i++) {
		ಆದರೆ (ಲಂಬಸಾಲು[i].type == ಸ್ತಂಭ_ಅಗಲ_RELATIVE)
			relative_ಮೊತ್ತ += ಲಂಬಸಾಲು[i].ಅಗಲ;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಲಂಬಸಾಲು[i].type == ಸ್ತಂಭ_ಅಗಲ_FIXED)
			ಹೆಚ್ಚುವರಿ_ಅಗಲ -= ಲಂಬಸಾಲು[i].ಅಗಲ;
		ಇಲ್ಲದಿದ್ದರೆ
			ಹೆಚ್ಚುವರಿ_ಅಗಲ -= ಲಂಬಸಾಲು[i].min;
	}
	ಹೆಚ್ಚುವರಿ_ಅಗಲ -= (columns + 1) * border_spacing_h;
	ಆದರೆ (relative_ಮೊತ್ತ != 0) {
		ಆದರೆ (ಹೆಚ್ಚುವರಿ_ಅಗಲ < 0)
			ಹೆಚ್ಚುವರಿ_ಅಗಲ = 0;
		ಗಾಗಿ (i = 0; i != columns; i++) {
			ಆದರೆ (ಲಂಬಸಾಲು[i].type == ಸ್ತಂಭ_ಅಗಲ_RELATIVE) {
				ಲಂಬಸಾಲು[i].min = ceil(ಲಂಬಸಾಲು[i].max =
						(ದಶಮಾಂಶ) ಹೆಚ್ಚುವರಿ_ಅಗಲ
						* (ದಶಮಾಂಶ) ಲಂಬಸಾಲು[i].ಅಗಲ
						/ relative_ಮೊತ್ತ);
				min_ಅಗಲ += ಲಂಬಸಾಲು[i].min;
				max_ಅಗಲ += ಲಂಬಸಾಲು[i].max;
			}
		}
	}
	min_ಅಗಲ += (columns + 1) * border_spacing_h;
	max_ಅಗಲ += (columns + 1) * border_spacing_h;

	ಆದರೆ (auto_ಅಗಲ <= min_ಅಗಲ) {
		/* not enough space: minimise column widths */
		ಗಾಗಿ (i = 0; i < columns; i++) {
			ಲಂಬಸಾಲು[i].ಅಗಲ = ಲಂಬಸಾಲು[i].min;
		}
		ಕೋಷ್ಟಕ_ಅಗಲ = min_ಅಗಲ;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (max_ಅಗಲ <= auto_ಅಗಲ) {
		/* more space than maximum width */
		ಆದರೆ (ಕೋಷ್ಟಕ_ಅಗಲ == AUTO) {
			/* for auto-width tables, make columns max width */
			ಗಾಗಿ (i = 0; i < columns; i++) {
				ಲಂಬಸಾಲು[i].ಅಗಲ = ಲಂಬಸಾಲು[i].max;
			}
			ಕೋಷ್ಟಕ_ಅಗಲ = max_ಅಗಲ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* for fixed-width tables, distribute the extra space
			 * too */
			ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flexible_columns = 0;
			ಗಾಗಿ (i = 0; i != columns; i++)
				ಆದರೆ (ಲಂಬಸಾಲು[i].type != ಸ್ತಂಭ_ಅಗಲ_FIXED)
					flexible_columns++;
			ಆದರೆ (flexible_columns == 0) {
				ಪೂರ್ಣಾಂಕ extra = (ಕೋಷ್ಟಕ_ಅಗಲ - max_ಅಗಲ) / columns;
				remainder = (ಕೋಷ್ಟಕ_ಅಗಲ - max_ಅಗಲ) -
						(extra * columns);
				ಗಾಗಿ (i = 0; i != columns; i++) {
					ಲಂಬಸಾಲು[i].ಅಗಲ = ಲಂಬಸಾಲು[i].max + extra;
					ಎಣಿಕೆ_2 -= remainder;
					ಆದರೆ (ಎಣಿಕೆ_2 < 0) {
						ಲಂಬಸಾಲು[i].ಅಗಲ++;
						ಎಣಿಕೆ_2 += columns;
					}
				}

			} ಇಲ್ಲದಿದ್ದರೆ {
				ಪೂರ್ಣಾಂಕ extra = (ಕೋಷ್ಟಕ_ಅಗಲ - max_ಅಗಲ) /
						flexible_columns;
				remainder = (ಕೋಷ್ಟಕ_ಅಗಲ - max_ಅಗಲ) -
						(extra * flexible_columns);
				ಗಾಗಿ (i = 0; i != columns; i++)
					ಆದರೆ (ಲಂಬಸಾಲು[i].type != ಸ್ತಂಭ_ಅಗಲ_FIXED) {
						ಲಂಬಸಾಲು[i].ಅಗಲ = ಲಂಬಸಾಲು[i].max +
								extra;
						ಎಣಿಕೆ_2 -= remainder;
						ಆದರೆ (ಎಣಿಕೆ_2 < 0) {
							ಲಂಬಸಾಲು[i].ಅಗಲ++;
							ಎಣಿಕೆ_2 += flexible_columns;
						}
					}
			}
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* space between min and max: fill it exactly */
		ದಶಮಾಂಶ scale = (ದಶಮಾಂಶ) (auto_ಅಗಲ - min_ಅಗಲ) /
				(ದಶಮಾಂಶ) (max_ಅಗಲ - min_ಅಗಲ);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		ಗಾಗಿ (i = 0; i < columns; i++) {
			ಲಂಬಸಾಲು[i].ಅಗಲ = ಲಂಬಸಾಲು[i].min + (ಪೂರ್ಣಾಂಕ) (0.5 +
					(ಲಂಬಸಾಲು[i].max - ಲಂಬಸಾಲು[i].min) * scale);
		}
		ಕೋಷ್ಟಕ_ಅಗಲ = auto_ಅಗಲ;
	}

	xs[0] = x = border_spacing_h;
	ಗಾಗಿ (i = 0; i != columns; i++) {
		ಆದರೆ (!ಲಂಬಸಾಲು[i].positioned)
			x += ಲಂಬಸಾಲು[i].ಅಗಲ + border_spacing_h;
		xs[i + 1] = x;
		ಅಡ್ಡಸಾಲು_ಹರವು[i] = 0;
		excess_y[i] = 0;
		ಅಡ್ಡಸಾಲು_ಹರವು_cell[i] = 0;
	}

	/* position cells */
	ಕೋಷ್ಟಕ_ಎತ್ತರ = border_spacing_v;
	ಗಾಗಿ (ಅಡ್ಡಸಾಲು_ಗುಂಪು = ಕೋಷ್ಟಕ->children; ಅಡ್ಡಸಾಲು_ಗುಂಪು;
			ಅಡ್ಡಸಾಲು_ಗುಂಪು = ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಮುಂದಿನ) {
		ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲು_ಗುಂಪು_ಎತ್ತರ = 0;
		ಗಾಗಿ (ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು_ಗುಂಪು->children; ಅಡ್ಡಸಾಲು; ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->ಮುಂದಿನ) {
			ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲು_ಎತ್ತರ = 0;

			htype = css_computed_ಎತ್ತರ(ಅಡ್ಡಸಾಲು->style, &ಮೌಲ್ಯ, &unit);
			ಆದರೆ (htype == CSS_ಎತ್ತರ_ಗಣ && unit != CSS_UNIT_PCT) {
				ಅಡ್ಡಸಾಲು_ಎತ್ತರ = FIXTOINT(css_unit_len2device_px(
						ಅಡ್ಡಸಾಲು->style,
						&content->unit_len_ctx,
						ಮೌಲ್ಯ, unit));
			}
			ಗಾಗಿ (c = ಅಡ್ಡಸಾಲು->children; c; c = c->ಮುಂದಿನ) {
				ಪ್ರತಿಜ್ಞಾ(c->style);
				c->ಅಗಲ = xs[c->ಆರಂಭ_ಸ್ತಂಭ + c->columns] -
						xs[c->ಆರಂಭ_ಸ್ತಂಭ] -
						border_spacing_h -
						c->border[ಎಡ_2].ಅಗಲ -
						c->padding[ಎಡ_2] -
						c->padding[ಬಲ_2] -
						c->border[ಬಲ_2].ಅಗಲ;
				c->float_children = 0;
				c->cached_ಇರಿಸು_ಕೆಳಗೆ_level = 0;

				c->ಎತ್ತರ = AUTO;
				ಆದರೆ (!layout_ಖಂಡ_context(c, -1, content)) {
					ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು);
					ಬಿಡುಗಡೆ(excess_y);
					ಬಿಡುಗಡೆ(ಅಡ್ಡಸಾಲು_ಹರವು);
					ಬಿಡುಗಡೆ(ಅಡ್ಡಸಾಲು_ಹರವು_cell);
					ಬಿಡುಗಡೆ(xs);
					ಹಿಂತಿರುಗಿ false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->ಎತ್ತರ;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_ಎತ್ತರ(c->style,
						&ಮೌಲ್ಯ, &unit);

				ಆದರೆ (htype == CSS_ಎತ್ತರ_ಗಣ &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					ಪೂರ್ಣಾಂಕ h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							ಮೌಲ್ಯ, unit));
					ಆದರೆ (c->ಎತ್ತರ < h)
						c->ಎತ್ತರ = h;
				}
				/* specified row height is treated as a minimum
				 */
				ಆದರೆ (c->ಎತ್ತರ < ಅಡ್ಡಸಾಲು_ಎತ್ತರ)
					c->ಎತ್ತರ = ಅಡ್ಡಸಾಲು_ಎತ್ತರ;
				c->x = xs[c->ಆರಂಭ_ಸ್ತಂಭ] +
						c->border[ಎಡ_2].ಅಗಲ;
				c->y = c->border[ತುದಿ_2].ಅಗಲ;
				ಗಾಗಿ (i = 0; i != c->columns; i++) {
					ಅಡ್ಡಸಾಲು_ಹರವು[c->ಆರಂಭ_ಸ್ತಂಭ + i] = c->ಅಡ್ಡಸಾಲುಗಳು;
					excess_y[c->ಆರಂಭ_ಸ್ತಂಭ + i] =
							c->border[ತುದಿ_2].ಅಗಲ +
							c->padding[ತುದಿ_2] +
							c->ಎತ್ತರ +
							c->padding[BOTTOM] +
							c->border[BOTTOM].ಅಗಲ;
					ಅಡ್ಡಸಾಲು_ಹರವು_cell[c->ಆರಂಭ_ಸ್ತಂಭ + i] = 0;
				}
				ಅಡ್ಡಸಾಲು_ಹರವು_cell[c->ಆರಂಭ_ಸ್ತಂಭ] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[ತುದಿ_2].ಅಗಲ -
						c->padding[ತುದಿ_2] -
						c->ಎತ್ತರ -
						c->border[BOTTOM].ಅಗಲ;
			}
			ಗಾಗಿ (i = 0; i != columns; i++)
				ಆದರೆ (ಅಡ್ಡಸಾಲು_ಹರವು[i] != 0)
					ಅಡ್ಡಸಾಲು_ಹರವು[i]--;
				ಇಲ್ಲದಿದ್ದರೆ
					ಅಡ್ಡಸಾಲು_ಹರವು_cell[i] = 0;
			ಆದರೆ (ಅಡ್ಡಸಾಲು->ಮುಂದಿನ || ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಮುಂದಿನ) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				ಗಾಗಿ (i = 0; i != columns; i++)
					ಆದರೆ (ಅಡ್ಡಸಾಲು_ಹರವು[i] == 0 && ಅಡ್ಡಸಾಲು_ಎತ್ತರ <
							excess_y[i])
						ಅಡ್ಡಸಾಲು_ಎತ್ತರ = excess_y[i];
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* except in the last row */
				ಗಾಗಿ (i = 0; i != columns; i++)
					ಆದರೆ (ಅಡ್ಡಸಾಲು_ಎತ್ತರ < excess_y[i])
						ಅಡ್ಡಸಾಲು_ಎತ್ತರ = excess_y[i];
			}
			ಗಾಗಿ (i = 0; i != columns; i++) {
				ಆದರೆ (ಅಡ್ಡಸಾಲು_ಎತ್ತರ < excess_y[i])
					excess_y[i] -= ಅಡ್ಡಸಾಲು_ಎತ್ತರ;
				ಇಲ್ಲದಿದ್ದರೆ
					excess_y[i] = 0;
				ಆದರೆ (ಅಡ್ಡಸಾಲು_ಹರವು_cell[i] != 0)
					ಅಡ್ಡಸಾಲು_ಹರವು_cell[i]->padding[BOTTOM] +=
							ಅಡ್ಡಸಾಲು_ಎತ್ತರ +
							border_spacing_v;
			}

			ಅಡ್ಡಸಾಲು->x = 0;
			ಅಡ್ಡಸಾಲು->y = ಅಡ್ಡಸಾಲು_ಗುಂಪು_ಎತ್ತರ;
			ಅಡ್ಡಸಾಲು->ಅಗಲ = ಕೋಷ್ಟಕ_ಅಗಲ;
			ಅಡ್ಡಸಾಲು->ಎತ್ತರ = ಅಡ್ಡಸಾಲು_ಎತ್ತರ;
			ಅಡ್ಡಸಾಲು_ಗುಂಪು_ಎತ್ತರ += ಅಡ್ಡಸಾಲು_ಎತ್ತರ + border_spacing_v;
		}
		ಅಡ್ಡಸಾಲು_ಗುಂಪು->x = 0;
		ಅಡ್ಡಸಾಲು_ಗುಂಪು->y = ಕೋಷ್ಟಕ_ಎತ್ತರ;
		ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಅಗಲ = ಕೋಷ್ಟಕ_ಅಗಲ;
		ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಎತ್ತರ = ಅಡ್ಡಸಾಲು_ಗುಂಪು_ಎತ್ತರ;
		ಕೋಷ್ಟಕ_ಎತ್ತರ += ಅಡ್ಡಸಾಲು_ಗುಂಪು_ಎತ್ತರ;
	}
	/* Table height is either the height of the contents, or specified
	 * height if greater */
	ಕೋಷ್ಟಕ_ಎತ್ತರ = max(ಕೋಷ್ಟಕ_ಎತ್ತರ, min_ಎತ್ತರ);
	/** \todo distribute spare height over the row groups / rows / cells */

	/* perform vertical alignment */
	ಗಾಗಿ (ಅಡ್ಡಸಾಲು_ಗುಂಪು = ಕೋಷ್ಟಕ->children; ಅಡ್ಡಸಾಲು_ಗುಂಪು;
			ಅಡ್ಡಸಾಲು_ಗುಂಪು = ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಮುಂದಿನ) {
		ಗಾಗಿ (ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು_ಗುಂಪು->children; ಅಡ್ಡಸಾಲು; ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->ಮುಂದಿನ) {
			ಗಾಗಿ (c = ಅಡ್ಡಸಾಲು->children; c; c = c->ಮುಂದಿನ) {
				ಎಣಿಕೆ css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				ಹೆಚ್ಚುವರಿ_ಎತ್ತರ = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->ಎತ್ತರ - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &ಮೌಲ್ಯ, &unit);

				ಆಯ್ಕೆ (vertical_align) {
				ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_SUB:
				ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_SUPER:
				ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_ಪಠ್ಯ_ತುದಿ:
				ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_ಪಠ್ಯ_BOTTOM:
				ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_ಗಣ:
				ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_ತುದಿ:
					ನಿಲ್ಲಿಸು;
				ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_ಮಧ್ಯ:
					c->padding[ತುದಿ_2] += ಹೆಚ್ಚುವರಿ_ಎತ್ತರ / 2;
					c->padding[BOTTOM] -= ಹೆಚ್ಚುವರಿ_ಎತ್ತರ / 2;
					layout_move_children(c, 0,
							ಹೆಚ್ಚುವರಿ_ಎತ್ತರ / 2);
					ನಿಲ್ಲಿಸು;
				ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[ತುದಿ_2] += ಹೆಚ್ಚುವರಿ_ಎತ್ತರ;
					c->padding[BOTTOM] -= ಹೆಚ್ಚುವರಿ_ಎತ್ತರ;
					layout_move_children(c, 0,
							ಹೆಚ್ಚುವರಿ_ಎತ್ತರ);
					ನಿಲ್ಲಿಸು;
				ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_INHERIT:
					ಪ್ರತಿಜ್ಞಾ(0);
					ನಿಲ್ಲಿಸು;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	ಆದರೆ (ಕೋಷ್ಟಕ->margin[ತುದಿ_2] == AUTO)
		ಕೋಷ್ಟಕ->margin[ತುದಿ_2] = 0;
	ಆದರೆ (ಕೋಷ್ಟಕ->margin[BOTTOM] == AUTO)
		ಕೋಷ್ಟಕ->margin[BOTTOM] = 0;

	ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು);
	ಬಿಡುಗಡೆ(excess_y);
	ಬಿಡುಗಡೆ(ಅಡ್ಡಸಾಲು_ಹರವು);
	ಬಿಡುಗಡೆ(ಅಡ್ಡಸಾಲು_ಹರವು_cell);
	ಬಿಡುಗಡೆ(xs);

	ಕೋಷ್ಟಕ->ಅಗಲ = ಕೋಷ್ಟಕ_ಅಗಲ;
	ಕೋಷ್ಟಕ->ಎತ್ತರ = ಕೋಷ್ಟಕ_ಎತ್ತರ;

	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ bool layout_apply_minmax_ಎತ್ತರ(
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ರಚನೆ box *box,
		ರಚನೆ box *container)
{
	ಪೂರ್ಣಾಂಕ h;
	ರಚನೆ box *containing_ಖಂಡ = ಖಾಲಿ;
	bool updated = false;

	/* Find containing block for percentage heights */
	ಆದರೆ (box->style != ಖಾಲಿ && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		ಪ್ರತಿಜ್ಞಾ(container);
		containing_ಖಂಡ = container;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->float_container && box->style != ಖಾಲಿ &&
			(css_computed_float(box->style) == CSS_FLOAT_ಎಡ ||
			 css_computed_float(box->style) == CSS_FLOAT_ಬಲ)) {
		/* Box is a float */
		ಪ್ರತಿಜ್ಞಾ(box->ಪೋಷಕ && box->ಪೋಷಕ->ಪೋಷಕ &&
				box->ಪೋಷಕ->ಪೋಷಕ->ಪೋಷಕ);
		containing_ಖಂಡ = box->ಪೋಷಕ->ಪೋಷಕ->ಪೋಷಕ;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->ಪೋಷಕ && box->ಪೋಷಕ->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_ಖಂಡ = box->ಪೋಷಕ;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->ಪೋಷಕ && box->ಪೋಷಕ->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		ಪ್ರತಿಜ್ಞಾ(box->ಪೋಷಕ->ಪೋಷಕ);
		containing_ಖಂಡ = box->ಪೋಷಕ->ಪೋಷಕ;
	}

	ಆದರೆ (box->style) {
		ಎಣಿಕೆ css_ಎತ್ತರ_e htype = CSS_ಎತ್ತರ_AUTO;
		css_fixed ಮೌಲ್ಯ = 0;
		css_unit unit = CSS_UNIT_PX;

		ಆದರೆ (containing_ಖಂಡ) {
			htype = css_computed_ಎತ್ತರ(containing_ಖಂಡ->style,
					&ಮೌಲ್ಯ, &unit);
		}

		/* max-height */
		ಆದರೆ (css_computed_max_ಎತ್ತರ(box->style, &ಮೌಲ್ಯ, &unit) ==
				CSS_MAX_ಎತ್ತರ_ಗಣ) {
			ಆದರೆ (unit == CSS_UNIT_PCT) {
				ಆದರೆ (containing_ಖಂಡ &&
					containing_ಖಂಡ->ಎತ್ತರ != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_ಎತ್ತರ_ಗಣ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ,
						containing_ಖಂಡ->ಎತ್ತರ);
					ಆದರೆ (h < box->ಎತ್ತರ) {
						box->ಎತ್ತರ = h;
						updated = true;
					}
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
				ಆದರೆ (h < box->ಎತ್ತರ) {
					box->ಎತ್ತರ = h;
					updated = true;
				}
			}
		}

		/* min-height */
		ಆದರೆ (ns_computed_min_ಎತ್ತರ(box->style, &ಮೌಲ್ಯ, &unit) ==
				CSS_MIN_ಎತ್ತರ_ಗಣ) {
			ಆದರೆ (unit == CSS_UNIT_PCT) {
				ಆದರೆ (containing_ಖಂಡ &&
					containing_ಖಂಡ->ಎತ್ತರ != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_ಎತ್ತರ_ಗಣ)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ,
						containing_ಖಂಡ->ಎತ್ತರ);
					ಆದರೆ (h > box->ಎತ್ತರ) {
						box->ಎತ್ತರ = h;
						updated = true;
					}
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ಮೌಲ್ಯ, unit));
				ಆದರೆ (h > box->ಎತ್ತರ) {
					box->ಎತ್ತರ = h;
					updated = true;
				}
			}
		}
	}
	ಹಿಂತಿರುಗಿ updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
ಸ್ಥಿರ bool layout_ಖಂಡ_object(ರಚನೆ box *ಖಂಡ)
{
	ಪ್ರತಿಜ್ಞಾ(ಖಂಡ);
	ಪ್ರತಿಜ್ಞಾ(ಖಂಡ->type == BOX_ಖಂಡ ||
			ಖಂಡ->type == BOX_FLEX ||
			ಖಂಡ->type == BOX_INLINE_ಖಂಡ ||
			ಖಂಡ->type == BOX_INLINE_FLEX ||
			ಖಂಡ->type == BOX_ಕೋಷ್ಟಕ ||
			ಖಂಡ->type == BOX_ಕೋಷ್ಟಕ_CELL);
	ಪ್ರತಿಜ್ಞಾ(ಖಂಡ->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", ಖಂಡ,
	      hlcache_handle_get_url(ಖಂಡ->object), ಖಂಡ->ಅಗಲ);

	ಆದರೆ (content_can_reformat(ಖಂಡ->object)) {
		content_reformat(ಖಂಡ->object, false, ಖಂಡ->ಅಗಲ, 1);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
ಸ್ಥಿರ ಶೂನ್ಯ ಕೂಡಿಸು_float_ಗಮ್ಯ_container(ರಚನೆ box *cont, ರಚನೆ box *b)
{
	ರಚನೆ box *box = cont->float_children;
	ಪೂರ್ಣಾಂಕ b_bottom = b->y + b->ಎತ್ತರ;

	ಪ್ರತಿಜ್ಞಾ(b->type == BOX_FLOAT_ಎಡ || b->type == BOX_FLOAT_ಬಲ);

	ಆದರೆ (box == ಖಾಲಿ) {
		/* No other float children */
		b->ಮುಂದಿನ_float = ಖಾಲಿ;
		cont->float_children = b;
		ಹಿಂತಿರುಗಿ;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (b_bottom >= box->y + box->ಎತ್ತರ) {
		/* Goes at start of list */
		b->ಮುಂದಿನ_float = cont->float_children;
		cont->float_children = b;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ರಚನೆ box *prev = ಖಾಲಿ;
		ಆಗ (box != ಖಾಲಿ && b_bottom < box->y + box->ಎತ್ತರ) {
			prev = box;
			box = box->ಮುಂದಿನ_float;
		}
		ಆದರೆ (prev != ಖಾಲಿ) {
			b->ಮುಂದಿನ_float = prev->ಮುಂದಿನ_float;
			prev->ಮುಂದಿನ_float = b;
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
ಸ್ಥಿರ bool
layout_ಪಠ್ಯ_box_ವಿಭಜಿಸು(html_content *content,
		      plot_font_style_t *fstyle,
		      ರಚನೆ box *ವಿಭಜಿಸು_box,
		      size_t new_ಉದ್ದಳತೆ,
		      ಪೂರ್ಣಾಂಕ new_ಅಗಲ)
{
	ಪೂರ್ಣಾಂಕ space_ಅಗಲ = ವಿಭಜಿಸು_box->space;
	ರಚನೆ box *c2;
	ಸ್ಥಿರಾಂಕ ರಚನೆ gui_layout_ಕೋಷ್ಟಕ *font_func = content->font_func;
	bool space = (ವಿಭಜಿಸು_box->ಪಠ್ಯ[new_ಉದ್ದಳತೆ] == ' ');
	ಪೂರ್ಣಾಂಕ ಬಳಸಿದ_ಉದ್ದಳತೆ = new_ಉದ್ದಳತೆ + (space ? 1 : 0);

	ಆದರೆ ((space && space_ಅಗಲ == 0) || space_ಅಗಲ == UNKNOWN_ಅಗಲ) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->ಅಗಲ(fstyle, " ", 1, &space_ಅಗಲ);
	}

	ಆದರೆ (ವಿಭಜಿಸು_box->space == UNKNOWN_ಅಗಲ)
		ವಿಭಜಿಸು_box->space = space_ಅಗಲ;
	ಆದರೆ (!space)
		space_ಅಗಲ = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, ವಿಭಜಿಸು_box, ಗಾತ್ರ *c2);
	ಆದರೆ (!c2)
		ಹಿಂತಿರುಗಿ false;
	c2->flags |= CLONE;

	/* Set remaining text in c2 */
	c2->ಪಠ್ಯ += ಬಳಸಿದ_ಉದ್ದಳತೆ;

	/* Set c2 according to the remaining text */
	c2->ಅಗಲ -= new_ಅಗಲ + space_ಅಗಲ;
	c2->flags &= ~MEASURED; /* width has been estimated */
	c2->ಉದ್ದಳತೆ = ವಿಭಜಿಸು_box->ಉದ್ದಳತೆ - ಬಳಸಿದ_ಉದ್ದಳತೆ;

	/* Update split_box for its reduced text */
	ವಿಭಜಿಸು_box->ಅಗಲ = new_ಅಗಲ;
	ವಿಭಜಿಸು_box->flags |= MEASURED;
	ವಿಭಜಿಸು_box->ಉದ್ದಳತೆ = new_ಉದ್ದಳತೆ;
	ವಿಭಜಿಸು_box->space = space_ಅಗಲ;

	/* Insert c2 into box list */
	c2->ಮುಂದಿನ = ವಿಭಜಿಸು_box->ಮುಂದಿನ;
	ವಿಭಜಿಸು_box->ಮುಂದಿನ = c2;
	c2->prev = ವಿಭಜಿಸು_box;
	ಆದರೆ (c2->ಮುಂದಿನ)
		c2->ಮುಂದಿನ->prev = c2;
	ಇಲ್ಲದಿದ್ದರೆ
		c2->ಪೋಷಕ->ಕೊನೆಯ = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      ವಿಭಜಿಸು_box,
	      ವಿಭಜಿಸು_box->ಉದ್ದಳತೆ,
	      (ಪೂರ್ಣಾಂಕ)ವಿಭಜಿಸು_box->ಉದ್ದಳತೆ,
	      ವಿಭಜಿಸು_box->ಪಠ್ಯ);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->ಉದ್ದಳತೆ,
	      (ಪೂರ್ಣಾಂಕ)c2->ಉದ್ದಳತೆ,
	      c2->ಪಠ್ಯ);

	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_float_ಕಂಡುಹಿಡಿ_dimensions(
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ,
		ಸ್ಥಿರಾಂಕ css_computed_style *style,
		ರಚನೆ box *box)
{
	ಪೂರ್ಣಾಂಕ ಅಗಲ, ಎತ್ತರ, max_ಅಗಲ, min_ಅಗಲ, max_ಎತ್ತರ, min_ಎತ್ತರ;
	ಪೂರ್ಣಾಂಕ *margin = box->margin;
	ಪೂರ್ಣಾಂಕ *padding = box->padding;
	ರಚನೆ box_border *border = box->border;
	ಎಣಿಕೆ css_ಅತಿಪ್ರವಾಹ_e ಅತಿಪ್ರವಾಹ_x = css_computed_ಅತಿಪ್ರವಾಹ_x(style);
	ಎಣಿಕೆ css_ಅತಿಪ್ರವಾಹ_e ಅತಿಪ್ರವಾಹ_y = css_computed_ಅತಿಪ್ರವಾಹ_y(style);
	ಪೂರ್ಣಾಂಕ scrollbar_ಅಗಲ_x =
			(ಅತಿಪ್ರವಾಹ_x == CSS_ಅತಿಪ್ರವಾಹ_SCROLL ||
			 ಅತಿಪ್ರವಾಹ_x == CSS_ಅತಿಪ್ರವಾಹ_AUTO) ?
			SCROLLBAR_ಅಗಲ : 0;
	ಪೂರ್ಣಾಂಕ scrollbar_ಅಗಲ_y =
			(ಅತಿಪ್ರವಾಹ_y == CSS_ಅತಿಪ್ರವಾಹ_SCROLL ||
			 ಅತಿಪ್ರವಾಹ_y == CSS_ಅತಿಪ್ರವಾಹ_AUTO) ?
			SCROLLBAR_ಅಗಲ : 0;

	layout_ಕಂಡುಹಿಡಿ_dimensions(unit_len_ctx, ಲಭ್ಯ_ಅಗಲ, -1, box, style,
			&ಅಗಲ, &ಎತ್ತರ, &max_ಅಗಲ, &min_ಅಗಲ,
			&max_ಎತ್ತರ, &min_ಎತ್ತರ, margin, padding, border);

	ಆದರೆ (margin[ಎಡ_2] == AUTO)
		margin[ಎಡ_2] = 0;
	ಆದರೆ (margin[ಬಲ_2] == AUTO)
		margin[ಬಲ_2] = 0;

	ಆದರೆ (box->gadget == ಖಾಲಿ) {
		padding[ಬಲ_2] += scrollbar_ಅಗಲ_y;
		padding[BOTTOM] += scrollbar_ಅಗಲ_x;
	}

	ಆದರೆ (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &ಅಗಲ, &ಎತ್ತರ,
				min_ಅಗಲ, max_ಅಗಲ, min_ಎತ್ತರ, max_ಎತ್ತರ);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed ಗಾತ್ರ_2 = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		ಪ್ರತಿಜ್ಞಾ(box->style);

		ಆದರೆ (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			ಆದರೆ (ಅಗಲ == AUTO) {
				ಗಾತ್ರ_2 = INTTOFIX(10);
				ಅಗಲ = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ಗಾತ್ರ_2, unit));
			}
			ಆದರೆ (box->gadget->type == GADGET_FILE &&
					ಎತ್ತರ == AUTO) {
				ಗಾತ್ರ_2 = FLTTOFIX(1.5);
				ಎತ್ತರ = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ಗಾತ್ರ_2, unit));
			}
		}
		ಆದರೆ (box->gadget->type == GADGET_TEXTAREA) {
			ಆದರೆ (ಅಗಲ == AUTO) {
				ಗಾತ್ರ_2 = INTTOFIX(10);
				ಅಗಲ = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ಗಾತ್ರ_2, unit));
			}
			ಆದರೆ (ಎತ್ತರ == AUTO) {
				ಗಾತ್ರ_2 = INTTOFIX(4);
				ಎತ್ತರ = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						ಗಾತ್ರ_2, unit));
			}
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಗಲ == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		ಅಗಲ = min(max(box->min_ಅಗಲ, ಲಭ್ಯ_ಅಗಲ),
				box->max_ಅಗಲ);

		/* width includes margin, borders and padding */
		ಆದರೆ (ಅಗಲ == ಲಭ್ಯ_ಅಗಲ) {
			ಅಗಲ -= box->margin[ಎಡ_2] + box->border[ಎಡ_2].ಅಗಲ +
					box->padding[ಎಡ_2] +
					box->padding[ಬಲ_2] +
					box->border[ಬಲ_2].ಅಗಲ +
					box->margin[ಬಲ_2];
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			ಪೂರ್ಣಾಂಕ fixed = 0;
			ದಶಮಾಂಶ frac = 0;
			calculate_mbp_ಅಗಲ(unit_len_ctx, box->style, ಎಡ_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_ಅಗಲ(unit_len_ctx, box->style, ಬಲ_2,
					true, true, true, &fixed, &frac);
			ಆದರೆ (fixed < 0)
				fixed = 0;

			ಅಗಲ -= fixed;
		}

		ಆದರೆ (max_ಅಗಲ >= 0 && ಅಗಲ > max_ಅಗಲ) ಅಗಲ = max_ಅಗಲ;
		ಆದರೆ (min_ಅಗಲ >  0 && ಅಗಲ < min_ಅಗಲ) ಅಗಲ = min_ಅಗಲ;

	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (max_ಅಗಲ >= 0 && ಅಗಲ > max_ಅಗಲ) ಅಗಲ = max_ಅಗಲ;
		ಆದರೆ (min_ಅಗಲ >  0 && ಅಗಲ < min_ಅಗಲ) ಅಗಲ = min_ಅಗಲ;
		ಅಗಲ -= scrollbar_ಅಗಲ_y;
	}

	box->ಅಗಲ = ಅಗಲ;
	box->ಎತ್ತರ = ಎತ್ತರ;

	ಆದರೆ (margin[ತುದಿ_2] == AUTO)
		margin[ತುದಿ_2] = 0;
	ಆದರೆ (margin[BOTTOM] == AUTO)
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
ಸ್ಥಿರ bool layout_float(ರಚನೆ box *b, ಪೂರ್ಣಾಂಕ ಅಗಲ, html_content *content)
{
	ಪ್ರತಿಜ್ಞಾ(b->type == BOX_ಕೋಷ್ಟಕ ||
	       b->type == BOX_ಖಂಡ ||
	       b->type == BOX_INLINE_ಖಂಡ ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_ಕಂಡುಹಿಡಿ_dimensions(&content->unit_len_ctx, ಅಗಲ, b->style, b);
	ಆದರೆ (b->type == BOX_ಕೋಷ್ಟಕ || b->type == BOX_INLINE_FLEX) {
		ಆದರೆ (b->type == BOX_ಕೋಷ್ಟಕ) {
			ಆದರೆ (!layout_ಕೋಷ್ಟಕ(b, ಅಗಲ, content))
				ಹಿಂತಿರುಗಿ false;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (!layout_flex(b, ಅಗಲ, content))
				ಹಿಂತಿರುಗಿ false;
		}
		ಆದರೆ (b->margin[ಎಡ_2] == AUTO)
			b->margin[ಎಡ_2] = 0;
		ಆದರೆ (b->margin[ಬಲ_2] == AUTO)
			b->margin[ಬಲ_2] = 0;
		ಆದರೆ (b->margin[ತುದಿ_2] == AUTO)
			b->margin[ತುದಿ_2] = 0;
		ಆದರೆ (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಹಿಂತಿರುಗಿ layout_ಖಂಡ_context(b, -1, content);
	}
	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ ಶೂನ್ಯ
ಇರಿಸು_float_ಕೆಳಗೆ(ರಚನೆ box *c, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ cx, ಪೂರ್ಣಾಂಕ y, ರಚನೆ box *cont)
{
	ಪೂರ್ಣಾಂಕ x0, x1, yy;
	ರಚನೆ box *ಎಡ;
	ರಚನೆ box *ಬಲ;

	yy = y > cont->cached_ಇರಿಸು_ಕೆಳಗೆ_level ?
			y : cont->cached_ಇರಿಸು_ಕೆಳಗೆ_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      ಅಗಲ, cx, y, cont);

	ಮಾಡು {
		y = yy;
		x0 = cx;
		x1 = cx + ಅಗಲ;
		ಕಂಡುಹಿಡಿ_sides(cont->float_children, y, y + c->ಎತ್ತರ, &x0, &x1,
				&ಎಡ, &ಬಲ);
		ಆದರೆ (ಎಡ != 0 && ಬಲ != 0) {
			yy = (ಎಡ->y + ಎಡ->ಎತ್ತರ <
					ಬಲ->y + ಬಲ->ಎತ್ತರ ?
					ಎಡ->y + ಎಡ->ಎತ್ತರ :
					ಬಲ->y + ಬಲ->ಎತ್ತರ);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಡ == 0 && ಬಲ != 0) {
			yy = ಬಲ->y + ಬಲ->ಎತ್ತರ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಡ != 0 && ಬಲ == 0) {
			yy = ಎಡ->y + ಎಡ->ಎತ್ತರ;
		}
	} ಆಗ ((ಎಡ != 0 || ಬಲ != 0) && (c->ಅಗಲ > x1 - x0));

	ಆದರೆ (c->type == BOX_FLOAT_ಎಡ) {
		c->x = x0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		c->x = x1 - c->ಅಗಲ;
	}
	c->y = y;
	cont->cached_ಇರಿಸು_ಕೆಳಗೆ_level = y;
}


/**
 * Calculate line height from a style.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಾಲು_ಎತ್ತರ(
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಸ್ಥಿರಾಂಕ css_computed_style *style)
{
	ಎಣಿಕೆ css_ಸಾಲು_ಎತ್ತರ_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed ಸಾಲು_ಎತ್ತರ;

	ಪ್ರತಿಜ್ಞಾ(style);

	lhtype = css_computed_ಸಾಲು_ಎತ್ತರ(style, &lhvalue, &lhunit);
	ಆದರೆ (lhtype == CSS_ಸಾಲು_ಎತ್ತರ_ಸಾಮಾನ್ಯ) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_ಸಾಲು_ಎತ್ತರ_ಸಂಖ್ಯೆ;
	}

	ಆದರೆ (lhtype == CSS_ಸಾಲು_ಎತ್ತರ_ಸಂಖ್ಯೆ ||
			lhunit == CSS_UNIT_PCT) {
		ಸಾಲು_ಎತ್ತರ = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		ಆದರೆ (lhtype != CSS_ಸಾಲು_ಎತ್ತರ_ಸಂಖ್ಯೆ)
			ಸಾಲು_ಎತ್ತರ = FDIV(ಸಾಲು_ಎತ್ತರ, F_100);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಪ್ರತಿಜ್ಞಾ(lhunit != CSS_UNIT_PCT);

		ಸಾಲು_ಎತ್ತರ = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	ಹಿಂತಿರುಗಿ FIXTOINT(ಸಾಲು_ಎತ್ತರ);
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
ಸ್ಥಿರ bool
layout_ಸಾಲು(ರಚನೆ box *ಮೊದಲ,
	    ಪೂರ್ಣಾಂಕ *ಅಗಲ,
	    ಪೂರ್ಣಾಂಕ *y,
	    ಪೂರ್ಣಾಂಕ cx,
	    ಪೂರ್ಣಾಂಕ cy,
	    ರಚನೆ box *cont,
	    bool indent,
	    bool has_ಪಠ್ಯ_children,
	    html_content *content,
	    ರಚನೆ box **ಮುಂದಿನ_box)
{
	ಪೂರ್ಣಾಂಕ ಎತ್ತರ, ಬಳಸಿದ_ಎತ್ತರ;
	ಪೂರ್ಣಾಂಕ x0 = 0;
	ಪೂರ್ಣಾಂಕ x1 = *ಅಗಲ;
	ಪೂರ್ಣಾಂಕ x, h, x_ಹಿಂದಿನ;
	ಪೂರ್ಣಾಂಕ fy = cy;
	ರಚನೆ box *ಎಡ;
	ರಚನೆ box *ಬಲ;
	ರಚನೆ box *b;
	ರಚನೆ box *ವಿಭಜಿಸು_box = 0;
	ರಚನೆ box *d;
	ರಚನೆ box *br_box = 0;
	bool move_y = false;
	bool ಇರಿಸು_ಕೆಳಗೆ = false;
	ಪೂರ್ಣಾಂಕ space_before = 0, space_after = 0;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ inline_ಎಣಿಕೆ = 0;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	ಸ್ಥಿರಾಂಕ ರಚನೆ gui_layout_ಕೋಷ್ಟಕ *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      ಮೊದಲ,
	      (ಪೂರ್ಣಾಂಕ)ಮೊದಲ->ಉದ್ದಳತೆ,
	      ಮೊದಲ->ಪಠ್ಯ,
	      *ಅಗಲ,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	ಕಂಡುಹಿಡಿ_sides(cont->float_children, cy, cy, &x0, &x1, &ಎಡ, &ಬಲ);
	x0 -= cx;
	x1 -= cx;

	ಆದರೆ (indent)
		x0 += layout_ಪಠ್ಯ_indent(&content->unit_len_ctx,
				ಮೊದಲ->ಪೋಷಕ->ಪೋಷಕ->style, *ಅಗಲ);

	ಆದರೆ (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	ಆದರೆ (has_ಪಠ್ಯ_children || ಮೊದಲ->ಪೋಷಕ->ಪೋಷಕ->gadget)
		ಬಳಸಿದ_ಎತ್ತರ = ಎತ್ತರ = ಸಾಲು_ಎತ್ತರ(&content->unit_len_ctx,
				ಮೊದಲ->ಪೋಷಕ->ಪೋಷಕ->style);
	ಇಲ್ಲದಿದ್ದರೆ
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		ಬಳಸಿದ_ಎತ್ತರ = ಎತ್ತರ = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	ಗಾಗಿ (x = 0, b = ಮೊದಲ; x <= x1 - x0 && b != 0; b = b->ಮುಂದಿನ) {
		ಪೂರ್ಣಾಂಕ min_ಅಗಲ, max_ಅಗಲ, min_ಎತ್ತರ, max_ಎತ್ತರ;

		ಪ್ರತಿಜ್ಞಾ(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		ಆದರೆ (b->type == BOX_BR)
			ನಿಲ್ಲಿಸು;

		ಆದರೆ (lh__box_is_float_box(b))
			ಮುಂದುವರಿ;
		ಆದರೆ (b->type == BOX_INLINE_ಖಂಡ &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			ಮುಂದುವರಿ;

		ಪ್ರತಿಜ್ಞಾ(b->style != ಖಾಲಿ);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		ಆದರೆ (b->type == BOX_INLINE_ಖಂಡ ||
		    b->type == BOX_INLINE_FLEX) {
			ಆದರೆ (b->max_ಅಗಲ != UNKNOWN_ಅಗಲ)
				ಆದರೆ (!layout_float(b, *ಅಗಲ, content))
					ಹಿಂತಿರುಗಿ false;
			h = b->border[ತುದಿ_2].ಅಗಲ + b->padding[ತುದಿ_2] + b->ಎತ್ತರ +
					b->padding[BOTTOM] +
					b->border[BOTTOM].ಅಗಲ;
			ಆದರೆ (ಎತ್ತರ < h)
				ಎತ್ತರ = h;
			x += b->margin[ಎಡ_2] + b->border[ಎಡ_2].ಅಗಲ +
					b->padding[ಎಡ_2] + b->ಅಗಲ +
					b->padding[ಬಲ_2] +
					b->border[ಬಲ_2].ಅಗಲ +
					b->margin[ಬಲ_2];
			space_after = 0;
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_ಕಂಡುಹಿಡಿ_dimensions(&content->unit_len_ctx,
					*ಅಗಲ, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			ಗಾಗಿ (i = 0; i != 4; i++)
				ಆದರೆ (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[ಎಡ_2] + b->border[ಎಡ_2].ಅಗಲ +
					b->padding[ಎಡ_2];
			ಆದರೆ (b->inline_ಕೊನೆ) {
				b->inline_ಕೊನೆ->margin[ಬಲ_2] = b->margin[ಬಲ_2];
				b->inline_ಕೊನೆ->padding[ಬಲ_2] =
						b->padding[ಬಲ_2];
				b->inline_ಕೊನೆ->border[ಬಲ_2] =
						b->border[ಬಲ_2];
			} ಇಲ್ಲದಿದ್ದರೆ {
				x += b->padding[ಬಲ_2] +
						b->border[ಬಲ_2].ಅಗಲ +
						b->margin[ಬಲ_2];
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (b->type == BOX_INLINE_ಕೊನೆ) {
			b->ಅಗಲ = 0;
			ಆದರೆ (b->space == UNKNOWN_ಅಗಲ) {
				font_func->ಅಗಲ(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[ಬಲ_2] + b->border[ಬಲ_2].ಅಗಲ +
					b->margin[ಬಲ_2];
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->ಎತ್ತರ = ಸಾಲು_ಎತ್ತರ(&content->unit_len_ctx,
					b->style ? b->style :
					b->ಪೋಷಕ->ಪೋಷಕ->style);
			ಆದರೆ (ಎತ್ತರ < b->ಎತ್ತರ)
				ಎತ್ತರ = b->ಎತ್ತರ;

			ಆದರೆ (!b->ಪಠ್ಯ) {
				b->ಅಗಲ = 0;
				space_after = 0;
				ಮುಂದುವರಿ;
			}

			ಆದರೆ (b->ಅಗಲ == UNKNOWN_ಅಗಲ) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				ಆದರೆ (b->ಪೋಷಕ->ಪೋಷಕ->gadget &&
						b->ಪೋಷಕ->ಪೋಷಕ->gadget->type
						== GADGET_SELECT) {
					ಪೂರ್ಣಾಂಕ opt_maxwidth = 0;
					ರಚನೆ form_option *o;

					ಗಾಗಿ (o = b->ಪೋಷಕ->ಪೋಷಕ->gadget->
							ದತ್ತಾಂಶ.select.ವಸ್ತುಗಳು; o;
							o = o->ಮುಂದಿನ) {
						ಪೂರ್ಣಾಂಕ opt_ಅಗಲ;
						font_func->ಅಗಲ(&fstyle,
								o->ಪಠ್ಯ,
								ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(o->ಪಠ್ಯ),
								&opt_ಅಗಲ);

						ಆದರೆ (opt_maxwidth < opt_ಅಗಲ)
							opt_maxwidth =opt_ಅಗಲ;
					}
					b->ಅಗಲ = opt_maxwidth;
					ಆದರೆ (nsoption_bool(core_select_menu))
						b->ಅಗಲ += SCROLLBAR_ಅಗಲ;
				} ಇಲ್ಲದಿದ್ದರೆ {
					font_func->ಅಗಲ(&fstyle, b->ಪಠ್ಯ,
							b->ಉದ್ದಳತೆ, &b->ಅಗಲ);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			ಆದರೆ (b->ಪಠ್ಯ && (x + b->ಅಗಲ < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->ಮುಂದಿನ) {
				font_func->ಅಗಲ(&fstyle, b->ಪಠ್ಯ,
						 b->ಉದ್ದಳತೆ, &b->ಅಗಲ);
				b->flags |= MEASURED;
			}

			x += b->ಅಗಲ;
			ಆದರೆ (b->space == UNKNOWN_ಅಗಲ) {
				font_func->ಅಗಲ(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			ಮುಂದುವರಿ;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		ಪ್ರತಿಜ್ಞಾ(b->style);

		layout_ಕಂಡುಹಿಡಿ_dimensions(&content->unit_len_ctx,
				*ಅಗಲ, -1, b, b->style,
				&b->ಅಗಲ, &b->ಎತ್ತರ,
				&max_ಅಗಲ, &min_ಅಗಲ,
				&max_ಎತ್ತರ, &min_ಎತ್ತರ,
				ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ);

		ಆದರೆ (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->ಅಗಲ, &b->ಎತ್ತರ,
					min_ಅಗಲ, max_ಅಗಲ,
					min_ಎತ್ತರ, max_ಎತ್ತರ);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			ಆದರೆ (b->ಅಗಲ == AUTO)
				b->ಅಗಲ = 400;
			ಆದರೆ (b->ಎತ್ತರ == AUTO)
				b->ಎತ್ತರ = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* form control with no object */
			ಆದರೆ (b->ಅಗಲ == AUTO)
				b->ಅಗಲ = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			ಆದರೆ (b->ಎತ್ತರ == AUTO)
				b->ಎತ್ತರ = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		ಆದರೆ (b->object && content_can_reformat(b->object) &&
				b->ಅಗಲ !=
				content_get_ಲಭ್ಯ_ಅಗಲ(b->object)) {
			css_fixed ಮೌಲ್ಯ = 0;
			css_unit unit = CSS_UNIT_PX;
			ಎಣಿಕೆ css_ಎತ್ತರ_e htype = css_computed_ಎತ್ತರ(b->style,
					&ಮೌಲ್ಯ, &unit);

			content_reformat(b->object, false, b->ಅಗಲ, b->ಎತ್ತರ);

			ಆದರೆ (htype == CSS_ಎತ್ತರ_AUTO)
				b->ಎತ್ತರ = content_get_ಎತ್ತರ(b->object);
		}

		ಆದರೆ (ಎತ್ತರ < b->ಎತ್ತರ)
			ಎತ್ತರ = b->ಎತ್ತರ;

		x += b->ಅಗಲ;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *ಅಗಲ;
	ಕಂಡುಹಿಡಿ_sides(cont->float_children, cy, cy + ಎತ್ತರ, &x0, &x1,
			&ಎಡ, &ಬಲ);
	x0 -= cx;
	x1 -= cx;

	ಆದರೆ (indent)
		x0 += layout_ಪಠ್ಯ_indent(&content->unit_len_ctx,
				ಮೊದಲ->ಪೋಷಕ->ಪೋಷಕ->style, *ಅಗಲ);

	ಆದರೆ (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	ಗಾಗಿ (x = x_ಹಿಂದಿನ = 0, b = ಮೊದಲ; x <= x1 - x0 && b; b = b->ಮುಂದಿನ) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		ಆದರೆ (b->type == BOX_INLINE_ಖಂಡ &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lh__box_is_inline_flow(b)) {
			ಪ್ರತಿಜ್ಞಾ(b->ಅಗಲ != UNKNOWN_ಅಗಲ);

			x_ಹಿಂದಿನ = x;
			x += space_after;
			b->x = x;

			ಆದರೆ ((b->type == BOX_INLINE && !b->inline_ಕೊನೆ) ||
					b->type == BOX_INLINE_ಖಂಡ ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[ಎಡ_2] + b->border[ಎಡ_2].ಅಗಲ;
				x = b->x + b->padding[ಎಡ_2] + b->ಅಗಲ +
						b->padding[ಬಲ_2] +
						b->border[ಬಲ_2].ಅಗಲ +
						b->margin[ಬಲ_2];
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (b->type == BOX_INLINE) {
				b->x += b->margin[ಎಡ_2] + b->border[ಎಡ_2].ಅಗಲ;
				x = b->x + b->padding[ಎಡ_2] + b->ಅಗಲ;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (b->type == BOX_INLINE_ಕೊನೆ) {
				b->ಎತ್ತರ = b->inline_ಕೊನೆ->ಎತ್ತರ;
				x += b->padding[ಬಲ_2] +
						b->border[ಬಲ_2].ಅಗಲ +
						b->margin[ಬಲ_2];
			} ಇಲ್ಲದಿದ್ದರೆ {
				x += b->ಅಗಲ;
			}

			space_before = space_after;
			ಆದರೆ (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (b->ಪಠ್ಯ || b->type == BOX_INLINE_ಕೊನೆ) {
				ಆದರೆ (b->space == UNKNOWN_ಅಗಲ) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->ಅಗಲ(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} ಇಲ್ಲದಿದ್ದರೆ {
				space_after = 0;
			}
			ವಿಭಜಿಸು_box = b;
			move_y = true;
			inline_ಎಣಿಕೆ++;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (b->type == BOX_BR) {
			b->x = x;
			b->ಅಗಲ = 0;
			br_box = b;
			b = b->ಮುಂದಿನ;
			ವಿಭಜಿಸು_box = 0;
			move_y = true;
			ನಿಲ್ಲಿಸು;

		} ಇಲ್ಲದಿದ್ದರೆ {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_ಇರಿಸು_ಕೆಳಗೆ_level = 0;
			b->float_container = d->float_container = cont;

			ಆದರೆ (!layout_float(d, *ಅಗಲ, content))
				ಹಿಂತಿರುಗಿ false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[ತುದಿ_2],
			      d->border[ತುದಿ_2].ಅಗಲ);

			d->x = d->margin[ಎಡ_2] + d->border[ಎಡ_2].ಅಗಲ;
			d->y = d->margin[ತುದಿ_2] + d->border[ತುದಿ_2].ಅಗಲ;
			b->ಅಗಲ = d->margin[ಎಡ_2] + d->border[ಎಡ_2].ಅಗಲ +
					d->padding[ಎಡ_2] + d->ಅಗಲ +
					d->padding[ಬಲ_2] +
					d->border[ಬಲ_2].ಅಗಲ +
					d->margin[ಬಲ_2];
			b->ಎತ್ತರ = d->margin[ತುದಿ_2] + d->border[ತುದಿ_2].ಅಗಲ +
					d->padding[ತುದಿ_2] + d->ಎತ್ತರ +
					d->padding[BOTTOM] +
					d->border[BOTTOM].ಅಗಲ +
					d->margin[BOTTOM];

			ಆದರೆ (b->ಅಗಲ > (x1 - x0) - x)
				ಇರಿಸು_ಕೆಳಗೆ = true;
			ಆದರೆ (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ಎಡ && ಎಡ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ಬಲ &&
						ಬಲ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						ಎಡ == 0 && ಬಲ == 0)) &&
					(!ಇರಿಸು_ಕೆಳಗೆ ||
					(ಎಡ == 0 && ಬಲ == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_ಇರಿಸು_ಕೆಳಗೆ_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				ಆದರೆ (b->type == BOX_FLOAT_ಎಡ) {
					b->x = cx + x0;
					ಆದರೆ (b->ಅಗಲ > 0)
						x0 += b->ಅಗಲ;
					ಎಡ = b;
				} ಇಲ್ಲದಿದ್ದರೆ {
					b->x = cx + x1 - b->ಅಗಲ;
					ಆದರೆ (b->ಅಗಲ > 0)
						x1 -= b->ಅಗಲ;
					ಬಲ = b;
				}
				b->y = cy;
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				ಪೂರ್ಣಾಂಕ fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_ಇರಿಸು_ಕೆಳಗೆ_level) ?
						fcy :
						cont->cached_ಇರಿಸು_ಕೆಳಗೆ_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + ಎತ್ತರ : fy;

				ಇರಿಸು_float_ಕೆಳಗೆ(b, *ಅಗಲ, cx, fy, cont);
				fy = b->y;
				ಆದರೆ (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ಎಡ && ಎಡ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ಬಲ &&
						ಬಲ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(ಎಡ != 0 || ಬಲ != 0)))) {
					/* to be cleared below existing
					 * floats */
					ಆದರೆ (b->type == BOX_FLOAT_ಎಡ)
						b->x = cx;
					ಇಲ್ಲದಿದ್ದರೆ
						b->x = cx + *ಅಗಲ - b->ಅಗಲ;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					ಆದರೆ (fcy > cont->clear_level)
						cont->clear_level = fcy;
					ಆದರೆ (b->y < fcy)
						b->y = fcy;
				}
				ಆದರೆ (b->type == BOX_FLOAT_ಎಡ)
					ಎಡ = b;
				ಇಲ್ಲದಿದ್ದರೆ
					ಬಲ = b;
			}
			ಕೂಡಿಸು_float_ಗಮ್ಯ_container(cont, b);

			ವಿಭಜಿಸು_box = 0;
		}
	}

	ಆದರೆ (x1 - x0 < x && ವಿಭಜಿಸು_box) {
		/* the last box went over the end */
		size_t ವಿಭಜಿಸು = 0;
		ಪೂರ್ಣಾಂಕ w;
		bool no_wrap = css_computed_white_space(
				ವಿಭಜಿಸು_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				ವಿಭಜಿಸು_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_ಹಿಂದಿನ;

		ಆದರೆ (!no_wrap &&
		    (ವಿಭಜಿಸು_box->type == BOX_INLINE ||
		     ವಿಭಜಿಸು_box->type == BOX_ಪಠ್ಯ) &&
		    !ವಿಭಜಿಸು_box->object &&
		    !(ವಿಭಜಿಸು_box->flags & REPLACE_DIM) &&
		    !(ವಿಭಜಿಸು_box->flags & IFRAME) &&
		    !ವಿಭಜಿಸು_box->gadget && ವಿಭಜಿಸು_box->ಪಠ್ಯ) {

			font_plot_style_from_css(&content->unit_len_ctx,
					ವಿಭಜಿಸು_box->style, &fstyle);
			/** \todo handle errors */
			font_func->ವಿಭಜಿಸು(&fstyle,
					 ವಿಭಜಿಸು_box->ಪಠ್ಯ,
					 ವಿಭಜಿಸು_box->ಉದ್ದಳತೆ,
					 x1 - x0 - x - space_before,
					 &ವಿಭಜಿಸು,
					 &w);
		}

		/* split == 0 implies that text can't be split */

		ಆದರೆ (ವಿಭಜಿಸು == 0)
			w = ವಿಭಜಿಸು_box->ಅಗಲ;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      ವಿಭಜಿಸು_box,
		      (ಪೂರ್ಣಾಂಕ)ವಿಭಜಿಸು_box->ಉದ್ದಳತೆ,
		      ವಿಭಜಿಸು_box->ಪಠ್ಯ,
		      ವಿಭಜಿಸು,
		      w,
		      ಎಡ,
		      ಬಲ,
		      inline_ಎಣಿಕೆ);

		ಆದರೆ ((ವಿಭಜಿಸು == 0 || x1 - x0 <= x + space_before + w) &&
				!ಎಡ && !ಬಲ && inline_ಎಣಿಕೆ == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			ಆದರೆ (ವಿಭಜಿಸು == 0 || ವಿಭಜಿಸು == ವಿಭಜಿಸು_box->ಉದ್ದಳತೆ) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = ವಿಭಜಿಸು_box->ಮುಂದಿನ;
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* cut off first word for this line */
				ಆದರೆ (!layout_ಪಠ್ಯ_box_ವಿಭಜಿಸು(content, &fstyle,
						ವಿಭಜಿಸು_box, ವಿಭಜಿಸು, w))
					ಹಿಂತಿರುಗಿ false;
				b = ವಿಭಜಿಸು_box->ಮುಂದಿನ;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((ವಿಭಜಿಸು == 0 || x1 - x0 <= x + space_before + w) &&
				inline_ಎಣಿಕೆ == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			ಪ್ರತಿಜ್ಞಾ(ಎಡ || ಬಲ);
			ಬಳಸಿದ_ಎತ್ತರ = 0;
			ಆದರೆ (ಎಡ) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      ಎಡ->y,
				      ಎಡ->ಎತ್ತರ);

				ಬಳಸಿದ_ಎತ್ತರ = ಎಡ->y + ಎಡ->ಎತ್ತರ - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      ಬಳಸಿದ_ಎತ್ತರ);

			}
			ಆದರೆ (ಬಲ && ಬಳಸಿದ_ಎತ್ತರ <
					ಬಲ->y + ಬಲ->ಎತ್ತರ - cy + 1)
				ಬಳಸಿದ_ಎತ್ತರ = ಬಲ->y + ಬಲ->ಎತ್ತರ - cy + 1;

			ಆದರೆ (ಬಳಸಿದ_ಎತ್ತರ < 0)
				ಬಳಸಿದ_ಎತ್ತರ = 0;

			b = ವಿಭಜಿಸು_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ವಿಭಜಿಸು == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = ವಿಭಜಿಸು_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} ಇಲ್ಲದಿದ್ದರೆ {
			/* fit as many words as possible */
			ಪ್ರತಿಜ್ಞಾ(ವಿಭಜಿಸು != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (ಪೂರ್ಣಾಂಕ)ವಿಭಜಿಸು_box->ಉದ್ದಳತೆ, ವಿಭಜಿಸು_box->ಪಠ್ಯ,
			      x1 - x0, ವಿಭಜಿಸು, w);

			ಆದರೆ (ವಿಭಜಿಸು != ವಿಭಜಿಸು_box->ಉದ್ದಳತೆ) {
				ಆದರೆ (!layout_ಪಠ್ಯ_box_ವಿಭಜಿಸು(content, &fstyle,
						ವಿಭಜಿಸು_box, ವಿಭಜಿಸು, w))
					ಹಿಂತಿರುಗಿ false;
				b = ವಿಭಜಿಸು_box->ಮುಂದಿನ;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	ಆಯ್ಕೆ (css_computed_ಪಠ್ಯ_align(ಮೊದಲ->ಪೋಷಕ->ಪೋಷಕ->style)) {
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_ಬಲ:
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_LIBCSS_ಬಲ:
		x0 = x1 - x;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_CENTER:
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_ಎಡ:
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_LIBCSS_ಎಡ:
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_JUSTIFY:
		/* leave on left */
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CSS_ಪಠ್ಯ_ALIGN_DEFAULT:
		/* None; consider text direction */
		ಆಯ್ಕೆ (css_computed_direction(ಮೊದಲ->ಪೋಷಕ->ಪೋಷಕ->style)) {
		ಸ್ಥಿತಿ CSS_DIRECTION_LTR:
			/* leave on left */
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ CSS_DIRECTION_RTL:
			x0 = x1 - x;
			ನಿಲ್ಲಿಸು;
		}
		ನಿಲ್ಲಿಸು;
	}

	ಗಾಗಿ (d = ಮೊದಲ; d != b; d = d->ಮುಂದಿನ) {
		d->flags &= ~NEW_ಸಾಲು;

		ಆದರೆ (d->type == BOX_INLINE_ಖಂಡ &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			ಮುಂದುವರಿ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_ಪಠ್ಯ ||
				d->type == BOX_INLINE_ಕೊನೆ) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[ತುದಿ_2];

			ಆದರೆ (d->type == BOX_ಪಠ್ಯ && d->ಎತ್ತರ > ಬಳಸಿದ_ಎತ್ತರ) {
				/*ಪಠ್ಯ*/
				ಬಳಸಿದ_ಎತ್ತರ = d->ಎತ್ತರ;
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_ಖಂಡ) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[ತುದಿ_2].ಅಗಲ + d->margin[ತುದಿ_2];
			h = d->margin[ತುದಿ_2] + d->border[ತುದಿ_2].ಅಗಲ +
					d->padding[ತುದಿ_2] + d->ಎತ್ತರ +
					d->padding[BOTTOM] +
					d->border[BOTTOM].ಅಗಲ +
					d->margin[BOTTOM];
			ಆದರೆ (ಬಳಸಿದ_ಎತ್ತರ < h)
				ಬಳಸಿದ_ಎತ್ತರ = h;
		}
	}

	ಮೊದಲ->flags |= NEW_ಸಾಲು;

	ಪ್ರತಿಜ್ಞಾ(b != ಮೊದಲ || (move_y && 0 < ಬಳಸಿದ_ಎತ್ತರ && (ಎಡ || ಬಲ)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	ಗಾಗಿ (d = ಮೊದಲ; d != b; d = d->ಮುಂದಿನ) {
		ಆದರೆ ((d->type == BOX_INLINE && d->inline_ಕೊನೆ) ||
				d->type == BOX_BR ||
				d->type == BOX_ಪಠ್ಯ ||
				d->type == BOX_INLINE_ಕೊನೆ) {
			css_fixed ಮೌಲ್ಯ = 0;
			css_unit unit = CSS_UNIT_PX;
			ಆಯ್ಕೆ (css_computed_vertical_align(d->style, &ಮೌಲ್ಯ,
					&unit)) {
			ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_SUPER:
			ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_ತುದಿ:
			ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_ಪಠ್ಯ_ತುದಿ:
				/* already at top */
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_SUB:
			ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_BOTTOM:
			ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_ಪಠ್ಯ_BOTTOM:
				d->y += ಬಳಸಿದ_ಎತ್ತರ - d->ಎತ್ತರ;
				ನಿಲ್ಲಿಸು;
			ಪೂರ್ವನಿಯೋಜಿತ:
			ಸ್ಥಿತಿ CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (ಬಳಸಿದ_ಎತ್ತರ - d->ಎತ್ತರ);
				ನಿಲ್ಲಿಸು;
			}
		}
	}

	/* handle clearance for br */
	ಆದರೆ (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		ಪೂರ್ಣಾಂಕ clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		ಆದರೆ (ಬಳಸಿದ_ಎತ್ತರ < clear_y - cy)
			ಬಳಸಿದ_ಎತ್ತರ = clear_y - cy;
	}

	ಆದರೆ (move_y)
		*y += ಬಳಸಿದ_ಎತ್ತರ;
	*ಮುಂದಿನ_box = b;
	*ಅಗಲ = x; /* return actual width */
	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ bool layout_inline_container(ರಚನೆ box *inline_container, ಪೂರ್ಣಾಂಕ ಅಗಲ,
		ರಚನೆ box *cont, ಪೂರ್ಣಾಂಕ cx, ಪೂರ್ಣಾಂಕ cy, html_content *content)
{
	bool ಮೊದಲ_ಸಾಲು = true;
	bool has_ಪಠ್ಯ_children;
	ರಚನೆ box *c, *ಮುಂದಿನ;
	ಪೂರ್ಣಾಂಕ y = 0;
	ಪೂರ್ಣಾಂಕ curwidth,maxwidth = ಅಗಲ;

	ಪ್ರತಿಜ್ಞಾ(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      ಅಗಲ,
	      cont,
	      cx,
	      cy);


	has_ಪಠ್ಯ_children = false;
	ಗಾಗಿ (c = inline_container->children; c; c = c->ಮುಂದಿನ) {
		bool is_pre = false;

		ಆದರೆ (c->style) {
			ಎಣಿಕೆ css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_ಸಾಲು ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		ಆದರೆ ((lh__box_is_object(c) == false &&
				c->ಪಠ್ಯ && (c->ಉದ್ದಳತೆ || is_pre)) ||
				c->type == BOX_BR)
			has_ಪಠ್ಯ_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	ಗಾಗಿ (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->ಅಗಲ;
		ಆದರೆ (!layout_ಸಾಲು(c, &curwidth, &y, cx, cy + y, cont, ಮೊದಲ_ಸಾಲು,
				has_ಪಠ್ಯ_children, content, &ಮುಂದಿನ))
			ಹಿಂತಿರುಗಿ false;
		maxwidth = max(maxwidth,curwidth);
		c = ಮುಂದಿನ;
		ಮೊದಲ_ಸಾಲು = false;
	}

	inline_container->ಅಗಲ = maxwidth;
	inline_container->ಎತ್ತರ = y;

	ಹಿಂತಿರುಗಿ true;
}


/* Documented in layout_intertnal.h */
bool layout_ಖಂಡ_context(
		ರಚನೆ box *ಖಂಡ,
		ಪೂರ್ಣಾಂಕ viewport_ಎತ್ತರ,
		html_content *content)
{
	ರಚನೆ box *box;
	ಪೂರ್ಣಾಂಕ cx, cy;  /**< current coordinates */
	ಪೂರ್ಣಾಂಕ max_pos_margin = 0;
	ಪೂರ್ಣಾಂಕ max_neg_margin = 0;
	ಪೂರ್ಣಾಂಕ y = 0;
	ಪೂರ್ಣಾಂಕ lm, rm;
	ರಚನೆ box *margin_collapse = ಖಾಲಿ;
	bool ಒಳ_margin = false;
	css_fixed gadget_ಗಾತ್ರ;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	ಪ್ರತಿಜ್ಞಾ(ಖಂಡ->type == BOX_ಖಂಡ ||
			ಖಂಡ->type == BOX_INLINE_ಖಂಡ ||
			ಖಂಡ->type == BOX_ಕೋಷ್ಟಕ_CELL ||
			ಖಂಡ->type == BOX_FLEX ||
			ಖಂಡ->type == BOX_INLINE_FLEX);
	ಪ್ರತಿಜ್ಞಾ(ಖಂಡ->ಅಗಲ != UNKNOWN_ಅಗಲ);
	ಪ್ರತಿಜ್ಞಾ(ಖಂಡ->ಅಗಲ != AUTO);

	ಖಂಡ->float_children = ಖಾಲಿ;
	ಖಂಡ->cached_ಇರಿಸು_ಕೆಳಗೆ_level = 0;
	ಖಂಡ->clear_level = 0;

	/* special case if the block contains an object */
	ಆದರೆ (ಖಂಡ->object) {
		ಪೂರ್ಣಾಂಕ ತಾತ್ಕಾಲಿಕ_ಅಗಲ = ಖಂಡ->ಅಗಲ;
		ಆದರೆ (!layout_ಖಂಡ_object(ಖಂಡ))
			ಹಿಂತಿರುಗಿ false;
		layout_get_object_dimensions(ಖಂಡ, &ತಾತ್ಕಾಲಿಕ_ಅಗಲ,
				&ಖಂಡ->ಎತ್ತರ, INT_MIN, ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ,
				INT_MIN, ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ);
		ಹಿಂತಿರುಗಿ true;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಖಂಡ->flags & REPLACE_DIM) {
		ಹಿಂತಿರುಗಿ true;
	}

	/* special case if the block contains an radio button or checkbox */
	ಆದರೆ (ಖಂಡ->gadget && (ಖಂಡ->gadget->type == GADGET_RADIO ||
			ಖಂಡ->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_ಗಾತ್ರ = INTTOFIX(1);
		ಆದರೆ (ಖಂಡ->ಎತ್ತರ == AUTO)
			ಖಂಡ->ಎತ್ತರ = FIXTOINT(css_unit_len2device_px(
					ಖಂಡ->style,
					&content->unit_len_ctx,
					gadget_ಗಾತ್ರ, gadget_unit));
	}

	box = ಖಂಡ->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = ಖಂಡ->padding[ತುದಿ_2];
	ಆದರೆ (box)
		box->y = ಖಂಡ->padding[ತುದಿ_2];

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
	ಆಗ (box) {
		ಎಣಿಕೆ css_ಅತಿಪ್ರವಾಹ_e ಅತಿಪ್ರವಾಹ_x = CSS_ಅತಿಪ್ರವಾಹ_VISIBLE;
		ಎಣಿಕೆ css_ಅತಿಪ್ರವಾಹ_e ಅತಿಪ್ರವಾಹ_y = CSS_ಅತಿಪ್ರವಾಹ_VISIBLE;

		ಪ್ರತಿಜ್ಞಾ(box->type == BOX_ಖಂಡ ||
				box->type == BOX_FLEX ||
				box->type == BOX_ಕೋಷ್ಟಕ ||
				box->type == BOX_INLINE_CONTAINER);

		/* Tables are laid out before being positioned, because the
		 * position depends on the width which is calculated in
		 * table layout. Blocks and inline containers are positioned
		 * before being laid out, because width is not dependent on
		 * content, and the position is required during layout for
		 * correct handling of floats.
		 */

		ಆದರೆ (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->ಪೋಷಕ->padding[ಎಡ_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			ಹೋಗು advance_ಗಮ್ಯ_ಮುಂದಿನ_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		ಆದರೆ (margin_collapse == ಖಾಲಿ) {
			margin_collapse = layout_ಮುಂದಿನ_margin_ಖಂಡ(
					&content->unit_len_ctx, box, ಖಂಡ,
					viewport_ಎತ್ತರ,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			ಒಳ_margin = true;
		}

		/* Clearance. */
		y = 0;
		ಆದರೆ (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(ಖಂಡ->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		ಆದರೆ (box->style) {
			ಅತಿಪ್ರವಾಹ_x = css_computed_ಅತಿಪ್ರವಾಹ_x(box->style);
			ಅತಿಪ್ರವಾಹ_y = css_computed_ಅತಿಪ್ರವಾಹ_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		ಆದರೆ (box->type == BOX_FLEX ||
		    box->type == BOX_ಖಂಡ ||
		    box->flags & IFRAME) {
			ಆದರೆ (lh__box_is_object(box) == false &&
					box->style &&
					(ಅತಿಪ್ರವಾಹ_x != CSS_ಅತಿಪ್ರವಾಹ_VISIBLE ||
					 ಅತಿಪ್ರವಾಹ_y != CSS_ಅತಿಪ್ರವಾಹ_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				ಪೂರ್ಣಾಂಕ x0, x1, ತುದಿ;
				ರಚನೆ box *ಎಡ, *ಬಲ;
				ತುದಿ = cy + max_pos_margin - max_neg_margin;
				ತುದಿ = (ತುದಿ > y) ? ತುದಿ : y;
				x0 = cx;
				x1 = cx + box->ಪೋಷಕ->ಅಗಲ -
						box->ಪೋಷಕ->padding[ಎಡ_2] -
						box->ಪೋಷಕ->padding[ಬಲ_2];
				ಕಂಡುಹಿಡಿ_sides(ಖಂಡ->float_children, ತುದಿ, ತುದಿ,
						&x0, &x1, &ಎಡ, &ಬಲ);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->ಪೋಷಕ->ಅಗಲ -
						box->ಪೋಷಕ->padding[ಎಡ_2] -
						box->ಪೋಷಕ->padding[ಬಲ_2] -
						x1;
			}
			layout_ಖಂಡ_ಕಂಡುಹಿಡಿ_dimensions(&content->unit_len_ctx,
					box->ಪೋಷಕ->ಅಗಲ,
					viewport_ಎತ್ತರ, lm, rm, box);
			ಆದರೆ (box->type == BOX_ಖಂಡ && !(box->flags & IFRAME)) {
				layout_ಖಂಡ_ಕೂಡಿಸು_scrollbar(box, ಬಲ_2);
				layout_ಖಂಡ_ಕೂಡಿಸು_scrollbar(box, BOTTOM);
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->type == BOX_ಕೋಷ್ಟಕ) {
			ಆದರೆ (box->style != ಖಾಲಿ) {
				ಎಣಿಕೆ css_ಅಗಲ_e wtype;
				css_fixed ಅಗಲ = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_ಅಗಲ(box->style, &ಅಗಲ,
						&unit);

				ಆದರೆ (wtype == CSS_ಅಗಲ_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					ಪೂರ್ಣಾಂಕ x0, x1, ತುದಿ;
					ರಚನೆ box *ಎಡ, *ಬಲ;
					ತುದಿ = cy + max_pos_margin -
							max_neg_margin;
					ತುದಿ = (ತುದಿ > y) ? ತುದಿ : y;
					x0 = cx;
					x1 = cx + box->ಪೋಷಕ->ಅಗಲ -
						box->ಪೋಷಕ->padding[ಎಡ_2] -
						box->ಪೋಷಕ->padding[ಬಲ_2];
					ಕಂಡುಹಿಡಿ_sides(ಖಂಡ->float_children,
						ತುದಿ, ತುದಿ, &x0, &x1,
						&ಎಡ, &ಬಲ);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->ಪೋಷಕ->ಅಗಲ -
						box->ಪೋಷಕ->padding[ಎಡ_2] -
						box->ಪೋಷಕ->padding[ಬಲ_2] -
						x1;
				}
			}
			ಆದರೆ (!layout_ಕೋಷ್ಟಕ(box, box->ಪೋಷಕ->ಅಗಲ - lm - rm,
					content))
				ಹಿಂತಿರುಗಿ false;
			layout_solve_ಅಗಲ(box, box->ಪೋಷಕ->ಅಗಲ, box->ಅಗಲ,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->ಪೋಷಕ->padding[ಎಡ_2] + box->margin[ಎಡ_2] +
				box->border[ಎಡ_2].ಅಗಲ;
		cx += box->x;

		/* Position box: vertical. */
		ಆದರೆ (box->border[ತುದಿ_2].ಅಗಲ) {
			box->y += box->border[ತುದಿ_2].ಅಗಲ;
			cy += box->border[ತುದಿ_2].ಅಗಲ;
		}

		/* Vertical margin */
		ಆದರೆ (((box->type == BOX_ಖಂಡ && (box->flags & HAS_ಎತ್ತರ)) ||
		     box->type == BOX_FLEX ||
		     box->type == BOX_ಕೋಷ್ಟಕ ||
		     (box->type == BOX_INLINE_CONTAINER &&
		      !box_is_ಮೊದಲ_ಮಗು(box)) ||
		     margin_collapse == box) &&
		    ಒಳ_margin == true) {
			/* Margin goes above this box. */
			cy += max_pos_margin - max_neg_margin;
			box->y += max_pos_margin - max_neg_margin;

			/* Current margin has been applied. */
			ಒಳ_margin = false;
			max_pos_margin = max_neg_margin = 0;
		}

		/* Handle clearance */
		ಆದರೆ (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		ಆದರೆ (box->type == BOX_FLEX ||
				(box->type == BOX_ಖಂಡ && box->style &&
				 (ಅತಿಪ್ರವಾಹ_x != CSS_ಅತಿಪ್ರವಾಹ_VISIBLE ||
				  ಅತಿಪ್ರವಾಹ_y != CSS_ಅತಿಪ್ರವಾಹ_VISIBLE))) {

			ಆದರೆ (box->type == BOX_FLEX) {
				ಆದರೆ (!layout_flex(box, box->ಅಗಲ, content)) {
					ಹಿಂತಿರುಗಿ false;
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				layout_ಖಂಡ_context(box,
						viewport_ಎತ್ತರ, content);
			}

			cy += box->padding[ತುದಿ_2];

			ಆದರೆ (box->ಎತ್ತರ == AUTO) {
				box->ಎತ್ತರ = 0;
				layout_ಖಂಡ_ಕೂಡಿಸು_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->ಎತ್ತರ + box->padding[BOTTOM] +
					box->border[BOTTOM].ಅಗಲ;
			y = box->y + box->padding[ತುದಿ_2] + box->ಎತ್ತರ +
					box->padding[BOTTOM] +
					box->border[BOTTOM].ಅಗಲ;

			/* Skip children, because they are done in the new
			 * block context */
			ಹೋಗು advance_ಗಮ್ಯ_ಮುಂದಿನ_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->ಅಗಲ);

		/* Layout (except tables). */
		ಆದರೆ (box->object) {
			ಆದರೆ (!layout_ಖಂಡ_object(box))
				ಹಿಂತಿರುಗಿ false;

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->type == BOX_INLINE_CONTAINER) {
			box->ಅಗಲ = box->ಪೋಷಕ->ಅಗಲ;
			ಆದರೆ (!layout_inline_container(box, box->ಅಗಲ, ಖಂಡ,
					cx, cy, content))
				ಹಿಂತಿರುಗಿ false;

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->type == BOX_ಕೋಷ್ಟಕ) {
			/* Move down to avoid floats if necessary. */
			ಪೂರ್ಣಾಂಕ x0, x1;
			ರಚನೆ box *ಎಡ, *ಬಲ;
			y = cy;
			ಆಗ (1) {
				ಎಣಿಕೆ css_ಅಗಲ_e wtype;
				css_fixed ಅಗಲ = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_ಅಗಲ(box->style,
						&ಅಗಲ, &unit);

				x0 = cx;
				x1 = cx + box->ಪೋಷಕ->ಅಗಲ;
				ಕಂಡುಹಿಡಿ_sides(ಖಂಡ->float_children, y,
						y + box->ಎತ್ತರ,
						&x0, &x1, &ಎಡ, &ಬಲ);
				ಆದರೆ (wtype == CSS_ಅಗಲ_AUTO)
					ನಿಲ್ಲಿಸು;
				ಆದರೆ (box->ಅಗಲ <= x1 - x0)
					ನಿಲ್ಲಿಸು;
				ಆದರೆ (!ಎಡ && !ಬಲ)
					ನಿಲ್ಲಿಸು;
				ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ಎಡ)
					y = ಬಲ->y + ಬಲ->ಎತ್ತರ + 1;
				ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ಬಲ)
					y = ಎಡ->y + ಎಡ->ಎತ್ತರ + 1;
				ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಡ->y + ಎಡ->ಎತ್ತರ <
						ಬಲ->y + ಬಲ->ಎತ್ತರ)
					y = ಎಡ->y + ಎಡ->ಎತ್ತರ + 1;
				ಇಲ್ಲದಿದ್ದರೆ
					y = ಬಲ->y + ಬಲ->ಎತ್ತರ + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		ಆದರೆ (box->type == BOX_ಖಂಡ && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			ಆದರೆ (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = ಖಾಲಿ;
			}

			y = box->padding[ತುದಿ_2];
			box = box->children;
			box->y = y;
			cy += y;
			ಮುಂದುವರಿ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->type == BOX_ಖಂಡ || box->object ||
				box->flags & IFRAME)
			cy += box->padding[ತುದಿ_2];

		ಆದರೆ (box->type == BOX_ಖಂಡ && box->ಎತ್ತರ == AUTO) {
			box->ಎತ್ತರ = 0;
			layout_ಖಂಡ_ಕೂಡಿಸು_scrollbar(box, BOTTOM);
		}

		cy += box->ಎತ್ತರ + box->padding[BOTTOM] +
				box->border[BOTTOM].ಅಗಲ;
		cx -= box->x;
		y = box->y + box->padding[ತುದಿ_2] + box->ಎತ್ತರ +
				box->padding[BOTTOM] +
				box->border[BOTTOM].ಅಗಲ;

	advance_ಗಮ್ಯ_ಮುಂದಿನ_box:
		ಆದರೆ (!box->ಮುಂದಿನ) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			ಮಾಡು {
				ಆದರೆ (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = ಖಾಲಿ;
				}

				/* Apply bottom margin */
				ಆದರೆ (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->ಪೋಷಕ;
				ಆದರೆ (box == ಖಂಡ)
					ನಿಲ್ಲಿಸು;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				ಆದರೆ (box->type == BOX_ಖಂಡ &&
						box->flags & MAKE_ಎತ್ತರ) {
					margin_collapse = ಖಾಲಿ;
					ಒಳ_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				ಆದರೆ (box->ಎತ್ತರ == AUTO) {
					box->ಎತ್ತರ = y - box->padding[ತುದಿ_2];

					ಆದರೆ (box->type == BOX_ಖಂಡ)
						layout_ಖಂಡ_ಕೂಡಿಸು_scrollbar(box,
								BOTTOM);
				} ಇಲ್ಲದಿದ್ದರೆ
					cy += box->ಎತ್ತರ -
							(y - box->padding[ತುದಿ_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				ಆದರೆ (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_ಎತ್ತರ(
							&content->unit_len_ctx,
							box, ಖಾಲಿ)) {
					/* Height altered */
					/* Set current cy */
					cy += box->ಎತ್ತರ -
							(y - box->padding[ತುದಿ_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].ಅಗಲ;
				cx -= box->x;
				y = box->y + box->padding[ತುದಿ_2] + box->ಎತ್ತರ +
						box->padding[BOTTOM] +
						box->border[BOTTOM].ಅಗಲ;

			} ಆಗ (box->ಮುಂದಿನ == ಖಾಲಿ);
			ಆದರೆ (box == ಖಂಡ)
				ನಿಲ್ಲಿಸು;
		}

		/* To next sibling. */

		ಆದರೆ (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = ಖಾಲಿ;
		}

		ಆದರೆ (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->ಮುಂದಿನ;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	ಗಾಗಿ (box = ಖಂಡ->float_children; box; box = box->ಮುಂದಿನ_float) {
		y = box->y + box->ಎತ್ತರ + box->padding[BOTTOM] +
				box->border[BOTTOM].ಅಗಲ + box->margin[BOTTOM];
		ಆದರೆ (cy < y)
			cy = y;
	}

	ಆದರೆ (ಖಂಡ->ಎತ್ತರ == AUTO) {
		ಖಂಡ->ಎತ್ತರ = cy - ಖಂಡ->padding[ತುದಿ_2];
		ಆದರೆ (ಖಂಡ->type == BOX_ಖಂಡ)
			layout_ಖಂಡ_ಕೂಡಿಸು_scrollbar(ಖಂಡ, BOTTOM);
	}

	ಆದರೆ (ಖಂಡ->style && css_computed_position(ಖಂಡ->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_ಎತ್ತರ(&content->unit_len_ctx, ಖಂಡ, ಖಾಲಿ);
	}

	ಆದರೆ (ಖಂಡ->gadget &&
			(ಖಂಡ->gadget->type == GADGET_TEXTAREA ||
			ಖಂಡ->gadget->type == GADGET_PASSWORD ||
			ಖಂಡ->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		ಪೂರ್ಣಾಂಕ ta_ಅಗಲ = ಖಂಡ->padding[ಎಡ_2] + ಖಂಡ->ಅಗಲ +
				ಖಂಡ->padding[ಬಲ_2];
		ಪೂರ್ಣಾಂಕ ta_ಎತ್ತರ = ಖಂಡ->padding[ತುದಿ_2] + ಖಂಡ->ಎತ್ತರ +
				ಖಂಡ->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				ಖಂಡ->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_ಗಣ_layout(ಖಂಡ->gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta,
				&fstyle, ta_ಅಗಲ, ta_ಎತ್ತರ,
				ಖಂಡ->padding[ತುದಿ_2], ಖಂಡ->padding[ಬಲ_2],
				ಖಂಡ->padding[BOTTOM], ಖಂಡ->padding[ಎಡ_2]);
	}

	ಹಿಂತಿರುಗಿ true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
ಸ್ಥಿರ bool
layout_get_element_ಟ್ಯಾಗ್(
		ಸ್ಥಿರಾಂಕ dom_ಗಂಟು *ಗಂಟು,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_ಗಂಟು_type ಗಂಟು_type;
	dom_exception exc;

	exc = dom_ಗಂಟು_get_ಗಂಟು_type(ಗಂಟು, &ಗಂಟು_type);
	ಆದರೆ (exc != DOM_NO_ERR ||
	    ಗಂಟು_type != DOM_ELEMENT_ಗಂಟು) {
		ಹಿಂತಿರುಗಿ false;
	}

	exc = dom_html_element_get_ಟ್ಯಾಗ್_type(ಗಂಟು, &element_type);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ false;
	}

	*type = element_type;
	ಹಿಂತಿರುಗಿ true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool
layout__check_element_type(
		ಸ್ಥಿರಾಂಕ dom_ಗಂಟು *ಗಂಟು,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	ಆದರೆ (!layout_get_element_ಟ್ಯಾಗ್(ಗಂಟು, &element_type)) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ಸ್ಥಿರ bool
layout_get_li_ಮೌಲ್ಯ(dom_ಗಂಟು *li_ಗಂಟು, dom_long *ಮೌಲ್ಯ_ಹೊರಹೋಗು)
{
	dom_exception exc;
	dom_long ಮೌಲ್ಯ;
	bool has_ಮೌಲ್ಯ;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_ಗಂಟು,
			corestring_dom_ಮೌಲ್ಯ,
			&has_ಮೌಲ್ಯ);
	ಆದರೆ (exc != DOM_NO_ERR || has_ಮೌಲ್ಯ == false) {
		ಹಿಂತಿರುಗಿ false;
	}

	exc = dom_html_li_element_get_ಮೌಲ್ಯ(
			(dom_html_li_element *)li_ಗಂಟು,
			&ಮೌಲ್ಯ);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ false;
	}

	*ಮೌಲ್ಯ_ಹೊರಹೋಗು = ಮೌಲ್ಯ;
	ಹಿಂತಿರುಗಿ true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
ಸ್ಥಿರ bool
layout_get_ol_ಆರಂಭ(dom_ಗಂಟು *ol_ಗಂಟು, dom_long *ಆರಂಭ_ಹೊರಹೋಗು)
{
	dom_exception exc;
	dom_long ಆರಂಭ;
	bool has_ಆರಂಭ;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_ಗಂಟು,
			corestring_dom_ಆರಂಭ,
			&has_ಆರಂಭ);
	ಆದರೆ (exc != DOM_NO_ERR || has_ಆರಂಭ == false) {
		ಹಿಂತಿರುಗಿ false;
	}

	exc = dom_html_olist_element_get_ಆರಂಭ(
			(dom_html_olist_element *)ol_ಗಂಟು,
			&ಆರಂಭ);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ false;
	}

	*ಆರಂಭ_ಹೊರಹೋಗು = ಆರಂಭ;
	ಹಿಂತಿರುಗಿ true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
ಸ್ಥಿರ bool
layout__get_ol_reversed(dom_ಗಂಟು *ol_ಗಂಟು)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_ಗಂಟು,
			corestring_dom_reversed,
			&has_reversed);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
ಸ್ಥಿರ bool
layout_get_list_ಅಂಶ_ಎಣಿಕೆ(
		dom_ಗಂಟು *list_owner, dom_long *ಎಣಿಕೆ_ಹೊರಹೋಗು)
{
	dom_html_element_type ಟ್ಯಾಗ್_type;
	dom_exception exc;
	dom_ಗಂಟು *ಮಗು;
	ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2;

	ಆದರೆ (list_owner == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (!layout_get_element_ಟ್ಯಾಗ್(list_owner, &ಟ್ಯಾಗ್_type)) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (ಟ್ಯಾಗ್_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    ಟ್ಯಾಗ್_type != DOM_HTML_ELEMENT_TYPE_UL) {
		ಹಿಂತಿರುಗಿ false;
	}

	exc = dom_ಗಂಟು_get_ಮೊದಲ_ಮಗು(list_owner, &ಮಗು);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಎಣಿಕೆ_2 = 0;
	ಆಗ (ಮಗು != ಖಾಲಿ) {
		dom_ಗಂಟು *ತಾತ್ಕಾಲಿಕ_ಗಂಟು;

		ಆದರೆ (layout__check_element_type(ಮಗು,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ರಚನೆ box *ಮಗು_box;
			ಆದರೆ (dom_ಗಂಟು_get_user_ದತ್ತಾಂಶ(ಮಗು,
					corestring_dom_ns_ಕೀಲಿ_box_ಗಂಟು_ದತ್ತಾಂಶ,
					&ಮಗು_box) != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(ಮಗು);
				ಹಿಂತಿರುಗಿ false;
			}

			ಆದರೆ (ಮಗು_box != ಖಾಲಿ &&
			    ಮಗು_box->list_marker != ಖಾಲಿ) {
				ಎಣಿಕೆ_2++;
			}
		}

		exc = dom_ಗಂಟು_get_ಮುಂದಿನ_sibling(ಮಗು, &ತಾತ್ಕಾಲಿಕ_ಗಂಟು);
		dom_ಗಂಟು_unref(ಮಗು);
		ಆದರೆ (exc != DOM_NO_ERR) {
			ಹಿಂತಿರುಗಿ false;
		}

		ಮಗು = ತಾತ್ಕಾಲಿಕ_ಗಂಟು;
	}

	*ಎಣಿಕೆ_ಹೊರಹೋಗು = ಎಣಿಕೆ_2;
	ಹಿಂತಿರುಗಿ true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
layout_ordered_list_ಎಣಿಕೆ(
		ರಚನೆ box *box)
{
	dom_html_element_type ಟ್ಯಾಗ್_type;
	dom_exception exc;
	dom_ಗಂಟು *ಮಗು;
	ಪೂರ್ಣಾಂಕ ಹೆಜ್ಜೆ = 1;
	dom_long ಮುಂದಿನ;

	ಆದರೆ (box->ಗಂಟು == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (!layout_get_element_ಟ್ಯಾಗ್(box->ಗಂಟು, &ಟ್ಯಾಗ್_type)) {
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (ಟ್ಯಾಗ್_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    ಟ್ಯಾಗ್_type != DOM_HTML_ELEMENT_TYPE_UL) {
		ಹಿಂತಿರುಗಿ;
	}

	ಮುಂದಿನ = 1;
	ಆದರೆ (ಟ್ಯಾಗ್_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_ಆರಂಭ = layout_get_ol_ಆರಂಭ(box->ಗಂಟು, &ಮುಂದಿನ);
		bool have_reversed = layout__get_ol_reversed(box->ಗಂಟು);

		ಆದರೆ (have_reversed) {
			ಹೆಜ್ಜೆ = -1;
		}

		ಆದರೆ (!have_ಆರಂಭ && have_reversed) {
			layout_get_list_ಅಂಶ_ಎಣಿಕೆ(box->ಗಂಟು, &ಮುಂದಿನ);
		}
	}

	exc = dom_ಗಂಟು_get_ಮೊದಲ_ಮಗು(box->ಗಂಟು, &ಮಗು);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ;
	}

	ಆಗ (ಮಗು != ಖಾಲಿ) {
		dom_ಗಂಟು *ತಾತ್ಕಾಲಿಕ_ಗಂಟು;

		ಆದರೆ (layout__check_element_type(ಮಗು,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			ರಚನೆ box *ಮಗು_box;

			ಆದರೆ (dom_ಗಂಟು_get_user_ದತ್ತಾಂಶ(ಮಗು,
					corestring_dom_ns_ಕೀಲಿ_box_ಗಂಟು_ದತ್ತಾಂಶ,
					&ಮಗು_box) != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(ಮಗು);
				ಹಿಂತಿರುಗಿ;
			}

			ಆದರೆ (ಮಗು_box != ಖಾಲಿ &&
			    ಮಗು_box->list_marker != ಖಾಲಿ) {
				dom_long ಮೌಲ್ಯ;
				ರಚನೆ box *marker = ಮಗು_box->list_marker;
				ಆದರೆ (layout_get_li_ಮೌಲ್ಯ(ಮಗು, &ಮೌಲ್ಯ)) {
					marker->list_ಮೌಲ್ಯ = ಮೌಲ್ಯ;
					ಮುಂದಿನ = marker->list_ಮೌಲ್ಯ;
				} ಇಲ್ಲದಿದ್ದರೆ {
					marker->list_ಮೌಲ್ಯ = ಮುಂದಿನ;
				}
				ಮುಂದಿನ += ಹೆಜ್ಜೆ;
			}
		}

		exc = dom_ಗಂಟು_get_ಮುಂದಿನ_sibling(ಮಗು, &ತಾತ್ಕಾಲಿಕ_ಗಂಟು);
		dom_ಗಂಟು_unref(ಮಗು);
		ಆದರೆ (exc != DOM_NO_ERR) {
			ಹಿಂತಿರುಗಿ;
		}

		ಮಗು = ತಾತ್ಕಾಲಿಕ_ಗಂಟು;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
layout_ಗಣ_numerical_marker_ಪಠ್ಯ(
		ಸ್ಥಿರಾಂಕ html_content *content,
		ರಚನೆ box *box)
{
	ರಚನೆ box *marker = box->list_marker;
	size_t counter_len;
	css_error css_res;
	ಎಣಿಕೆ {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_ಗಾತ್ರ = 20,
	};

	marker->ಪಠ್ಯ = talloc_array(content->bctx, ಅಕ್ಷರ, LIST_MARKER_ಗಾತ್ರ);
	ಆದರೆ (marker->ಪಠ್ಯ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_ಮೌಲ್ಯ,
			marker->ಪಠ್ಯ, LIST_MARKER_ಗಾತ್ರ, &counter_len);
	ಆದರೆ (css_res == CSS_OK) {
		ಆದರೆ (counter_len > LIST_MARKER_ಗಾತ್ರ) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->ಪಠ್ಯ = talloc_realloc(content->bctx,
					marker->ಪಠ್ಯ,
					ಅಕ್ಷರ,
					counter_len);
			ಆದರೆ (marker->ಪಠ್ಯ == ಖಾಲಿ) {
				ಹಿಂತಿರುಗಿ;
			}
			css_computed_format_list_style(box->style,
					marker->list_ಮೌಲ್ಯ, marker->ಪಠ್ಯ,
					counter_len, &counter_len);
		}
		marker->ಉದ್ದಳತೆ = counter_len;
	}
}

/**
 * Find out if box's style represents a numerical list style type.
 *
 * \param[in]  b  Box with style to test.
 * \return true if box has numerical list style type, false otherwise.
 */
ಸ್ಥಿರ bool
layout_list_ಅಂಶ_is_numerical(
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *b)
{
	ಎಣಿಕೆ css_list_style_type_e t = css_computed_list_style_type(b->style);

	ಆಯ್ಕೆ (t) {
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_ವರ್ಗಫಲ: /* Fall through. */
	ಸ್ಥಿತಿ CSS_LIST_STYLE_TYPE_NONE:
		ಹಿಂತಿರುಗಿ false;

	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ true;
	}
}

/**
 * Layout list markers.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
layout_lists(ಸ್ಥಿರಾಂಕ html_content *content, ರಚನೆ box *box)
{
	ರಚನೆ box *ಮಗು;

	layout_ordered_list_ಎಣಿಕೆ(box);

	ಗಾಗಿ (ಮಗು = box->children; ಮಗು; ಮಗು = ಮಗು->ಮುಂದಿನ) {
		ಆದರೆ (ಮಗು->list_marker) {
			ರಚನೆ box *marker = ಮಗು->list_marker;

			ಆದರೆ (layout_list_ಅಂಶ_is_numerical(ಮಗು)) {
				ಆದರೆ (marker->ಪಠ್ಯ == ಖಾಲಿ) {
					layout_ಗಣ_numerical_marker_ಪಠ್ಯ(
							content, ಮಗು);
				}
			}
			ಆದರೆ (marker->object) {
				marker->ಅಗಲ =
					content_get_ಅಗಲ(marker->object);
				marker->x = -marker->ಅಗಲ;
				marker->ಎತ್ತರ =
					content_get_ಎತ್ತರ(marker->object);
				marker->y = (ಸಾಲು_ಎತ್ತರ(
						&content->unit_len_ctx,
						marker->style) -
						marker->ಎತ್ತರ) / 2;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (marker->ಪಠ್ಯ) {
				ಆದರೆ (marker->ಅಗಲ == UNKNOWN_ಅಗಲ) {
					plot_font_style_t fstyle;
					font_plot_style_from_css(
							&content->unit_len_ctx,
							marker->style,
							&fstyle);
					content->font_func->ಅಗಲ(&fstyle,
							marker->ಪಠ್ಯ,
							marker->ಉದ್ದಳತೆ,
							&marker->ಅಗಲ);
					marker->flags |= MEASURED;
				}
				marker->x = -marker->ಅಗಲ;
				marker->y = 0;
				marker->ಎತ್ತರ = ಸಾಲು_ಎತ್ತರ(
						&content->unit_len_ctx,
						marker->style);
			} ಇಲ್ಲದಿದ್ದರೆ {
				marker->x = 0;
				marker->y = 0;
				marker->ಅಗಲ = 0;
				marker->ಎತ್ತರ = 0;
			}
			/* Gap between marker and content */
			marker->x -= 4;
		}
		layout_lists(content, ಮಗು);
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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_compute_offsets(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		       ರಚನೆ box *box,
		       ರಚನೆ box *containing_ಖಂಡ,
		       ಪೂರ್ಣಾಂಕ *ತುದಿ,
		       ಪೂರ್ಣಾಂಕ *ಬಲ,
		       ಪೂರ್ಣಾಂಕ *bottom,
		       ಪೂರ್ಣಾಂಕ *ಎಡ)
{
	uint32_t type;
	css_fixed ಮೌಲ್ಯ = 0;
	css_unit unit = CSS_UNIT_PX;

	ಪ್ರತಿಜ್ಞಾ(containing_ಖಂಡ->ಅಗಲ != UNKNOWN_ಅಗಲ);
	ಪ್ರತಿಜ್ಞಾ(containing_ಖಂಡ->ಅಗಲ != AUTO);
	ಪ್ರತಿಜ್ಞಾ(containing_ಖಂಡ->ಎತ್ತರ != AUTO);

	/*ಎಡ*/
	type = css_computed_ಎಡ(box->style, &ಮೌಲ್ಯ, &unit);
	ಆದರೆ (type == CSS_ಎಡ_ಗಣ) {
		ಆದರೆ (unit == CSS_UNIT_PCT) {
			*ಎಡ = FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ,
					containing_ಖಂಡ->ಅಗಲ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			*ಎಡ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		*ಎಡ = AUTO;
	}

	/*ಬಲ*/
	type = css_computed_ಬಲ(box->style, &ಮೌಲ್ಯ, &unit);
	ಆದರೆ (type == CSS_ಬಲ_ಗಣ) {
		ಆದರೆ (unit == CSS_UNIT_PCT) {
			*ಬಲ = FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ,
					containing_ಖಂಡ->ಅಗಲ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			*ಬಲ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		*ಬಲ = AUTO;
	}

	/*ತುದಿ*/
	type = css_computed_ತುದಿ(box->style, &ಮೌಲ್ಯ, &unit);
	ಆದರೆ (type == CSS_ತುದಿ_ಗಣ) {
		ಆದರೆ (unit == CSS_UNIT_PCT) {
			*ತುದಿ = FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ,
					containing_ಖಂಡ->ಎತ್ತರ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			*ತುದಿ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		*ತುದಿ = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &ಮೌಲ್ಯ, &unit);
	ಆದರೆ (type == CSS_BOTTOM_ಗಣ) {
		ಆದರೆ (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_ದ_INT_TOINT(ಮೌಲ್ಯ,
					containing_ಖಂಡ->ಎತ್ತರ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					ಮೌಲ್ಯ, unit));
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
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
ಸ್ಥಿರ bool
layout_absolute(ರಚನೆ box *box,
		ರಚನೆ box *containing_ಖಂಡ,
		ಪೂರ್ಣಾಂಕ cx, ಪೂರ್ಣಾಂಕ cy,
		html_content *content)
{
	ಪೂರ್ಣಾಂಕ static_ಎಡ, static_ತುದಿ;  /* static position */
	ಪೂರ್ಣಾಂಕ ತುದಿ, ಬಲ, bottom, ಎಡ;
	ಪೂರ್ಣಾಂಕ ಅಗಲ, ಎತ್ತರ, max_ಅಗಲ, min_ಅಗಲ;
	ಪೂರ್ಣಾಂಕ *margin = box->margin;
	ಪೂರ್ಣಾಂಕ *padding = box->padding;
	ರಚನೆ box_border *border = box->border;
	ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ = containing_ಖಂಡ->ಅಗಲ;
	ಪೂರ್ಣಾಂಕ space;

	ಪ್ರತಿಜ್ಞಾ(box->type == BOX_ಖಂಡ || box->type == BOX_ಕೋಷ್ಟಕ ||
			box->type == BOX_INLINE_ಖಂಡ ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_ಎಡ = cx + box->x;
	static_ತುದಿ = cy + box->y;

	ಆದರೆ (containing_ಖಂಡ->type == BOX_ಖಂಡ ||
			containing_ಖಂಡ->type == BOX_INLINE_ಖಂಡ ||
			containing_ಖಂಡ->type == BOX_ಕೋಷ್ಟಕ_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_ಖಂಡ->ಅಗಲ += containing_ಖಂಡ->padding[ಎಡ_2] +
				containing_ಖಂಡ->padding[ಬಲ_2];
		containing_ಖಂಡ->ಎತ್ತರ += containing_ಖಂಡ->padding[ತುದಿ_2] +
				containing_ಖಂಡ->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_ಖಂಡ,
			&ತುದಿ, &ಬಲ, &bottom, &ಎಡ);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_ಖಂಡ;
	layout_ಕಂಡುಹಿಡಿ_dimensions(&content->unit_len_ctx, ಲಭ್ಯ_ಅಗಲ, -1,
			box, box->style, &ಅಗಲ, &ಎತ್ತರ,
			&max_ಅಗಲ, &min_ಅಗಲ, 0, 0,
			margin, padding, border);
	box->float_container = ಖಾಲಿ;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ಎಡ, margin[ಎಡ_2], border[ಎಡ_2].ಅಗಲ, padding[ಎಡ_2], ಅಗಲ,
	      padding[ಬಲ_2], border[ಬಲ_2].ಅಗಲ, margin[ಬಲ_2], ಬಲ,
	      containing_ಖಂಡ->ಅಗಲ);


	ಆದರೆ (ಎಡ == AUTO && ಅಗಲ == AUTO && ಬಲ == AUTO) {
		ಆದರೆ (margin[ಎಡ_2] == AUTO)
			margin[ಎಡ_2] = 0;
		ಆದರೆ (margin[ಬಲ_2] == AUTO)
			margin[ಬಲ_2] = 0;
		ಎಡ = static_ಎಡ;

		ಅಗಲ = min(max(box->min_ಅಗಲ, ಲಭ್ಯ_ಅಗಲ),
			box->max_ಅಗಲ);
		ಅಗಲ -= box->margin[ಎಡ_2] + box->border[ಎಡ_2].ಅಗಲ +
			box->padding[ಎಡ_2] + box->padding[ಬಲ_2] +
			box->border[ಬಲ_2].ಅಗಲ + box->margin[ಬಲ_2];

		/* Adjust for {min|max}-width */
		ಆದರೆ (max_ಅಗಲ >= 0 && ಅಗಲ > max_ಅಗಲ) ಅಗಲ = max_ಅಗಲ;
		ಆದರೆ (ಅಗಲ < min_ಅಗಲ) ಅಗಲ = min_ಅಗಲ;

		ಬಲ = containing_ಖಂಡ->ಅಗಲ -
			ಎಡ -
			margin[ಎಡ_2] - border[ಎಡ_2].ಅಗಲ - padding[ಎಡ_2] -
			ಅಗಲ -
			padding[ಬಲ_2] - border[ಬಲ_2].ಅಗಲ - margin[ಬಲ_2];
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಡ != AUTO && ಅಗಲ != AUTO && ಬಲ != AUTO) {

		/* Adjust for {min|max}-width */
		ಆದರೆ (max_ಅಗಲ >= 0 && ಅಗಲ > max_ಅಗಲ) ಅಗಲ = max_ಅಗಲ;
		ಆದರೆ (min_ಅಗಲ >  0 && ಅಗಲ < min_ಅಗಲ) ಅಗಲ = min_ಅಗಲ;

		ಆದರೆ (margin[ಎಡ_2] == AUTO && margin[ಬಲ_2] == AUTO) {
			space = containing_ಖಂಡ->ಅಗಲ -
					ಎಡ - border[ಎಡ_2].ಅಗಲ -
					padding[ಎಡ_2] - ಅಗಲ - padding[ಬಲ_2] -
					border[ಬಲ_2].ಅಗಲ - ಬಲ;
			ಆದರೆ (space < 0) {
				margin[ಎಡ_2] = 0;
				margin[ಬಲ_2] = space;
			} ಇಲ್ಲದಿದ್ದರೆ {
				margin[ಎಡ_2] = margin[ಬಲ_2] = space / 2;
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (margin[ಎಡ_2] == AUTO) {
			margin[ಎಡ_2] = containing_ಖಂಡ->ಅಗಲ -
					ಎಡ - border[ಎಡ_2].ಅಗಲ -
					padding[ಎಡ_2] - ಅಗಲ - padding[ಬಲ_2] -
					border[ಬಲ_2].ಅಗಲ - margin[ಬಲ_2] -
					ಬಲ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (margin[ಬಲ_2] == AUTO) {
			margin[ಬಲ_2] = containing_ಖಂಡ->ಅಗಲ -
					ಎಡ - margin[ಎಡ_2] -
					border[ಎಡ_2].ಅಗಲ -
					padding[ಎಡ_2] - ಅಗಲ - padding[ಬಲ_2] -
					border[ಬಲ_2].ಅಗಲ - ಬಲ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಬಲ = containing_ಖಂಡ->ಅಗಲ -
					ಎಡ - margin[ಎಡ_2] -
					border[ಎಡ_2].ಅಗಲ -
					padding[ಎಡ_2] - ಅಗಲ - padding[ಬಲ_2] -
					border[ಬಲ_2].ಅಗಲ - margin[ಬಲ_2];
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (margin[ಎಡ_2] == AUTO)
			margin[ಎಡ_2] = 0;
		ಆದರೆ (margin[ಬಲ_2] == AUTO)
			margin[ಬಲ_2] = 0;

		ಆದರೆ (ಎಡ == AUTO && ಅಗಲ == AUTO && ಬಲ != AUTO) {
			ಲಭ್ಯ_ಅಗಲ -= ಬಲ;

			ಅಗಲ = min(max(box->min_ಅಗಲ, ಲಭ್ಯ_ಅಗಲ),
				box->max_ಅಗಲ);
			ಅಗಲ -= box->margin[ಎಡ_2] + box->border[ಎಡ_2].ಅಗಲ +
				box->padding[ಎಡ_2] + box->padding[ಬಲ_2] +
				box->border[ಬಲ_2].ಅಗಲ + box->margin[ಬಲ_2];

			/* Adjust for {min|max}-width */
			ಆದರೆ (max_ಅಗಲ >= 0 && ಅಗಲ > max_ಅಗಲ)
				ಅಗಲ = max_ಅಗಲ;
			ಆದರೆ (ಅಗಲ < min_ಅಗಲ)
				ಅಗಲ = min_ಅಗಲ;

			ಎಡ = containing_ಖಂಡ->ಅಗಲ -
					margin[ಎಡ_2] - border[ಎಡ_2].ಅಗಲ -
					padding[ಎಡ_2] - ಅಗಲ - padding[ಬಲ_2] -
					border[ಬಲ_2].ಅಗಲ - margin[ಬಲ_2] -
					ಬಲ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಡ == AUTO && ಅಗಲ != AUTO && ಬಲ == AUTO) {

			/* Adjust for {min|max}-width */
			ಆದರೆ (max_ಅಗಲ >= 0 && ಅಗಲ > max_ಅಗಲ)
				ಅಗಲ = max_ಅಗಲ;
			ಆದರೆ (min_ಅಗಲ >  0 && ಅಗಲ < min_ಅಗಲ)
				ಅಗಲ = min_ಅಗಲ;

			ಎಡ = static_ಎಡ;
			ಬಲ = containing_ಖಂಡ->ಅಗಲ -
					ಎಡ - margin[ಎಡ_2] -
					border[ಎಡ_2].ಅಗಲ -
					padding[ಎಡ_2] - ಅಗಲ - padding[ಬಲ_2] -
					border[ಬಲ_2].ಅಗಲ - margin[ಬಲ_2];
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಡ != AUTO && ಅಗಲ == AUTO && ಬಲ == AUTO) {
			ಲಭ್ಯ_ಅಗಲ -= ಎಡ;

			ಅಗಲ = min(max(box->min_ಅಗಲ, ಲಭ್ಯ_ಅಗಲ),
				box->max_ಅಗಲ);
			ಅಗಲ -= box->margin[ಎಡ_2] + box->border[ಎಡ_2].ಅಗಲ +
				box->padding[ಎಡ_2] + box->padding[ಬಲ_2] +
				box->border[ಬಲ_2].ಅಗಲ + box->margin[ಬಲ_2];

			/* Adjust for {min|max}-width */
			ಆದರೆ (max_ಅಗಲ >= 0 && ಅಗಲ > max_ಅಗಲ)
				ಅಗಲ = max_ಅಗಲ;
			ಆದರೆ (ಅಗಲ < min_ಅಗಲ)
				ಅಗಲ = min_ಅಗಲ;

			ಬಲ = containing_ಖಂಡ->ಅಗಲ -
					ಎಡ - margin[ಎಡ_2] -
					border[ಎಡ_2].ಅಗಲ -
					padding[ಎಡ_2] - ಅಗಲ - padding[ಬಲ_2] -
					border[ಬಲ_2].ಅಗಲ - margin[ಬಲ_2];
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಡ == AUTO && ಅಗಲ != AUTO && ಬಲ != AUTO) {

			/* Adjust for {min|max}-width */
			ಆದರೆ (max_ಅಗಲ >= 0 && ಅಗಲ > max_ಅಗಲ)
				ಅಗಲ = max_ಅಗಲ;
			ಆದರೆ (ಅಗಲ < min_ಅಗಲ)
				ಅಗಲ = min_ಅಗಲ;

			ಎಡ = containing_ಖಂಡ->ಅಗಲ -
					margin[ಎಡ_2] - border[ಎಡ_2].ಅಗಲ -
					padding[ಎಡ_2] - ಅಗಲ - padding[ಬಲ_2] -
					border[ಬಲ_2].ಅಗಲ - margin[ಬಲ_2] -
					ಬಲ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಡ != AUTO && ಅಗಲ == AUTO && ಬಲ != AUTO) {
			ಅಗಲ = containing_ಖಂಡ->ಅಗಲ -
					ಎಡ - margin[ಎಡ_2] -
					border[ಎಡ_2].ಅಗಲ -
					padding[ಎಡ_2] - padding[ಬಲ_2] -
					border[ಬಲ_2].ಅಗಲ - margin[ಬಲ_2] -
					ಬಲ;

			/* Adjust for {min|max}-width */
			ಆದರೆ (max_ಅಗಲ >= 0 && ಅಗಲ > max_ಅಗಲ)
				ಅಗಲ = max_ಅಗಲ;
			ಆದರೆ (ಅಗಲ < min_ಅಗಲ)
				ಅಗಲ = min_ಅಗಲ;

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಡ != AUTO && ಅಗಲ != AUTO && ಬಲ == AUTO) {

			/* Adjust for {min|max}-width */
			ಆದರೆ (max_ಅಗಲ >= 0 && ಅಗಲ > max_ಅಗಲ)
				ಅಗಲ = max_ಅಗಲ;
			ಆದರೆ (ಅಗಲ < min_ಅಗಲ)
				ಅಗಲ = min_ಅಗಲ;

			ಬಲ = containing_ಖಂಡ->ಅಗಲ -
					ಎಡ - margin[ಎಡ_2] -
					border[ಎಡ_2].ಅಗಲ -
					padding[ಎಡ_2] - ಅಗಲ - padding[ಬಲ_2] -
					border[ಬಲ_2].ಅಗಲ - margin[ಬಲ_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ಎಡ, margin[ಎಡ_2], border[ಎಡ_2].ಅಗಲ, padding[ಎಡ_2], ಅಗಲ,
	      padding[ಬಲ_2], border[ಬಲ_2].ಅಗಲ, margin[ಬಲ_2], ಬಲ,
	      containing_ಖಂಡ->ಅಗಲ);

	box->x = ಎಡ + margin[ಎಡ_2] + border[ಎಡ_2].ಅಗಲ - cx;
	ಆದರೆ (containing_ಖಂಡ->type == BOX_ಖಂಡ ||
			containing_ಖಂಡ->type == BOX_INLINE_ಖಂಡ ||
			containing_ಖಂಡ->type == BOX_ಕೋಷ್ಟಕ_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_ಖಂಡ->ಅಗಲ -= containing_ಖಂಡ->padding[ಎಡ_2] +
				containing_ಖಂಡ->padding[ಬಲ_2];
	} ಇಲ್ಲದಿದ್ದರೆ {
		/** \todo inline ancestors */
	}
	box->ಅಗಲ = ಅಗಲ;
	box->ಎತ್ತರ = ಎತ್ತರ;

	ಆದರೆ (box->type == BOX_ಖಂಡ || box->type == BOX_INLINE_ಖಂಡ ||
			box->object || box->flags & IFRAME) {
		ಆದರೆ (!layout_ಖಂಡ_context(box, -1, content))
			ಹಿಂತಿರುಗಿ false;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->type == BOX_ಕೋಷ್ಟಕ) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_ಖಂಡ;
		/* \todo  layout_table considers margins etc. again */
		ಆದರೆ (!layout_ಕೋಷ್ಟಕ(box, ಅಗಲ, content))
			ಹಿಂತಿರುಗಿ false;
		box->float_container = ಖಾಲಿ;
		layout_solve_ಅಗಲ(box, box->ಪೋಷಕ->ಅಗಲ, box->ಅಗಲ, 0, 0,
				-1, -1);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_ಖಂಡ;
		ಆದರೆ (!layout_flex(box, ಅಗಲ, content))
			ಹಿಂತಿರುಗಿ false;
		box->float_container = ಖಾಲಿ;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ತುದಿ, margin[ತುದಿ_2], border[ತುದಿ_2].ಅಗಲ, padding[ತುದಿ_2], ಎತ್ತರ,
	      padding[BOTTOM], border[BOTTOM].ಅಗಲ, margin[BOTTOM], bottom,
	      containing_ಖಂಡ->ಎತ್ತರ);

	ಆದರೆ (ತುದಿ == AUTO && ಎತ್ತರ == AUTO && bottom == AUTO) {
		ತುದಿ = static_ತುದಿ;
		ಎತ್ತರ = box->ಎತ್ತರ;
		ಆದರೆ (margin[ತುದಿ_2] == AUTO)
			margin[ತುದಿ_2] = 0;
		ಆದರೆ (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_ಖಂಡ->ಎತ್ತರ -
				ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].ಅಗಲ -
				padding[ತುದಿ_2] - ಎತ್ತರ - padding[BOTTOM] -
				border[BOTTOM].ಅಗಲ - margin[BOTTOM];
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ತುದಿ != AUTO && ಎತ್ತರ != AUTO && bottom != AUTO) {
		ಆದರೆ (margin[ತುದಿ_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_ಖಂಡ->ಎತ್ತರ -
					ತುದಿ - border[ತುದಿ_2].ಅಗಲ - padding[ತುದಿ_2] -
					ಎತ್ತರ - padding[BOTTOM] -
					border[BOTTOM].ಅಗಲ - bottom;
			margin[ತುದಿ_2] = margin[BOTTOM] = space / 2;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (margin[ತುದಿ_2] == AUTO) {
			margin[ತುದಿ_2] = containing_ಖಂಡ->ಎತ್ತರ -
					ತುದಿ - border[ತುದಿ_2].ಅಗಲ - padding[ತುದಿ_2] -
					ಎತ್ತರ - padding[BOTTOM] -
					border[BOTTOM].ಅಗಲ - margin[BOTTOM] -
					bottom;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_ಖಂಡ->ಎತ್ತರ -
					ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].ಅಗಲ -
					padding[ತುದಿ_2] - ಎತ್ತರ -
					padding[BOTTOM] - border[BOTTOM].ಅಗಲ -
					bottom;
		} ಇಲ್ಲದಿದ್ದರೆ {
			bottom = containing_ಖಂಡ->ಎತ್ತರ -
					ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].ಅಗಲ -
					padding[ತುದಿ_2] - ಎತ್ತರ -
					padding[BOTTOM] - border[BOTTOM].ಅಗಲ -
					margin[BOTTOM];
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (margin[ತುದಿ_2] == AUTO)
			margin[ತುದಿ_2] = 0;
		ಆದರೆ (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		ಆದರೆ (ತುದಿ == AUTO && ಎತ್ತರ == AUTO && bottom != AUTO) {
			ಎತ್ತರ = box->ಎತ್ತರ;
			ತುದಿ = containing_ಖಂಡ->ಎತ್ತರ -
					margin[ತುದಿ_2] - border[ತುದಿ_2].ಅಗಲ -
					padding[ತುದಿ_2] - ಎತ್ತರ -
					padding[BOTTOM] - border[BOTTOM].ಅಗಲ -
					margin[BOTTOM] - bottom;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ತುದಿ == AUTO && ಎತ್ತರ != AUTO && bottom == AUTO) {
			ತುದಿ = static_ತುದಿ;
			bottom = containing_ಖಂಡ->ಎತ್ತರ -
					ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].ಅಗಲ -
					padding[ತುದಿ_2] - ಎತ್ತರ -
					padding[BOTTOM] - border[BOTTOM].ಅಗಲ -
					margin[BOTTOM];
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ತುದಿ != AUTO && ಎತ್ತರ == AUTO && bottom == AUTO) {
			ಎತ್ತರ = box->ಎತ್ತರ;
			bottom = containing_ಖಂಡ->ಎತ್ತರ -
					ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].ಅಗಲ -
					padding[ತುದಿ_2] - ಎತ್ತರ -
					padding[BOTTOM] - border[BOTTOM].ಅಗಲ -
					margin[BOTTOM];
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ತುದಿ == AUTO && ಎತ್ತರ != AUTO && bottom != AUTO) {
			ತುದಿ = containing_ಖಂಡ->ಎತ್ತರ -
					margin[ತುದಿ_2] - border[ತುದಿ_2].ಅಗಲ -
					padding[ತುದಿ_2] - ಎತ್ತರ -
					padding[BOTTOM] - border[BOTTOM].ಅಗಲ -
					margin[BOTTOM] - bottom;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ತುದಿ != AUTO && ಎತ್ತರ == AUTO && bottom != AUTO) {
			ಎತ್ತರ = containing_ಖಂಡ->ಎತ್ತರ -
					ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].ಅಗಲ -
					padding[ತುದಿ_2] - padding[BOTTOM] -
					border[BOTTOM].ಅಗಲ - margin[BOTTOM] -
					bottom;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ತುದಿ != AUTO && ಎತ್ತರ != AUTO && bottom == AUTO) {
			bottom = containing_ಖಂಡ->ಎತ್ತರ -
					ತುದಿ - margin[ತುದಿ_2] - border[ತುದಿ_2].ಅಗಲ -
					padding[ತುದಿ_2] - ಎತ್ತರ -
					padding[BOTTOM] - border[BOTTOM].ಅಗಲ -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ತುದಿ, margin[ತುದಿ_2], border[ತುದಿ_2].ಅಗಲ, padding[ತುದಿ_2], ಎತ್ತರ,
	      padding[BOTTOM], border[BOTTOM].ಅಗಲ, margin[BOTTOM], bottom,
	      containing_ಖಂಡ->ಎತ್ತರ);

	box->y = ತುದಿ + margin[ತುದಿ_2] + border[ತುದಿ_2].ಅಗಲ - cy;
	ಆದರೆ (containing_ಖಂಡ->type == BOX_ಖಂಡ ||
			containing_ಖಂಡ->type == BOX_INLINE_ಖಂಡ ||
			containing_ಖಂಡ->type == BOX_ಕೋಷ್ಟಕ_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_ಖಂಡ->ಎತ್ತರ -= containing_ಖಂಡ->padding[ತುದಿ_2] +
				containing_ಖಂಡ->padding[BOTTOM];
	} ಇಲ್ಲದಿದ್ದರೆ {
		/** \todo Inline ancestors */
	}
	box->ಎತ್ತರ = ಎತ್ತರ;
	layout_apply_minmax_ಎತ್ತರ(&content->unit_len_ctx, box, containing_ಖಂಡ);

	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ bool
layout_position_absolute(ರಚನೆ box *box,
			 ರಚನೆ box *containing_ಖಂಡ,
			 ಪೂರ್ಣಾಂಕ cx, ಪೂರ್ಣಾಂಕ cy,
			 html_content *content)
{
	ರಚನೆ box *c;

	ಗಾಗಿ (c = box->children; c; c = c->ಮುಂದಿನ) {
		ಆದರೆ ((c->type == BOX_ಖಂಡ || c->type == BOX_ಕೋಷ್ಟಕ ||
				c->type == BOX_INLINE_ಖಂಡ ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			ಆದರೆ (!layout_absolute(c, containing_ಖಂಡ,
					cx, cy, content))
				ಹಿಂತಿರುಗಿ false;
			ಆದರೆ (!layout_position_absolute(c, c, 0, 0, content))
				ಹಿಂತಿರುಗಿ false;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			ಆದರೆ (!layout_position_absolute(c, c, 0, 0, content))
				ಹಿಂತಿರುಗಿ false;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಪೂರ್ಣಾಂಕ px, py;
			ಆದರೆ (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_ಎಡ ||
					css_computed_float(c->style) ==
						CSS_FLOAT_ಬಲ)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				ರಚನೆ box *p;
				px = c->x;
				py = c->y;
				ಗಾಗಿ (p = box->ಪೋಷಕ; p && !p->float_children;
						p = p->ಪೋಷಕ) {
					px -= p->x;
					py -= p->y;
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			ಆದರೆ (!layout_position_absolute(c, containing_ಖಂಡ,
					cx + px, cy + py, content))
				ಹಿಂತಿರುಗಿ false;
		}
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_compute_relative_ಸ್ಥಾನಾಂತರ(
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ರಚನೆ box *box,
		ಪೂರ್ಣಾಂಕ *x,
		ಪೂರ್ಣಾಂಕ *y)
{
	ಪೂರ್ಣಾಂಕ ಎಡ, ಬಲ, ತುದಿ, bottom;
	ರಚನೆ box *containing_ಖಂಡ;

	ಪ್ರತಿಜ್ಞಾ(box && box->ಪೋಷಕ && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	ಆದರೆ (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_ಎಡ ||
			css_computed_float(box->style) == CSS_FLOAT_ಬಲ)) {
		containing_ಖಂಡ = box->float_container;
	} ಇಲ್ಲದಿದ್ದರೆ {
		containing_ಖಂಡ = box->ಪೋಷಕ;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_ಖಂಡ,
			&ತುದಿ, &ಬಲ, &bottom, &ಎಡ);

	ಆದರೆ (ಎಡ == AUTO && ಬಲ == AUTO)
		ಎಡ = ಬಲ = 0;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಡ == AUTO)
		/* left is auto => computed = -right */
		ಎಡ = -ಬಲ;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಬಲ == AUTO)
		/* right is auto => computed = -left */
		ಬಲ = -ಎಡ;
	ಇಲ್ಲದಿದ್ದರೆ {
		/* over constrained => examine direction property
		 * of containing block */
		ಆದರೆ (containing_ಖಂಡ->style &&
				css_computed_direction(
				containing_ಖಂಡ->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			ಎಡ = -ಬಲ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* assume LTR in all other cases */
			ಬಲ = -ಎಡ;
		}
	}

	ಪ್ರತಿಜ್ಞಾ(ಎಡ == -ಬಲ);

	ಆದರೆ (ತುದಿ == AUTO && bottom == AUTO) {
		ತುದಿ = bottom = 0;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ತುದಿ == AUTO) {
		ತುದಿ = -bottom;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -ತುದಿ;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", ಎಡ,
	      ಬಲ, ತುದಿ, bottom);

	*x = ಎಡ;
	*y = ತುದಿ;
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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_position_relative(
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ರಚನೆ box *ಬೇರು,
		ರಚನೆ box *fp,
		ಪೂರ್ಣಾಂಕ fx,
		ಪೂರ್ಣಾಂಕ fy)
{
	ರಚನೆ box *box; /* for children of "root" */
	ರಚನೆ box *fn;  /* for block formatting context box for children of
			  * "box" */
	ರಚನೆ box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	ಪೂರ್ಣಾಂಕ x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	ಪೂರ್ಣಾಂಕ fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	ಪ್ರತಿಜ್ಞಾ(ಬೇರು);

	/* Normal children */
	ಗಾಗಿ (box = ಬೇರು->children; box; box = box->ಮುಂದಿನ) {

		ಆದರೆ (box->type == BOX_ಪಠ್ಯ)
			ಮುಂದುವರಿ;

		/* If relatively positioned, get offsets */
		ಆದರೆ (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_ಸ್ಥಾನಾಂತರ(
					unit_len_ctx, box, &x, &y);
		ಇಲ್ಲದಿದ್ದರೆ
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		ಆದರೆ (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_ಎಡ ||
				css_computed_float(box->style) ==
					CSS_FLOAT_ಬಲ) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			ಗಾಗಿ (fc = fp->float_children; fc; fc = fc->ಮುಂದಿನ_float) {
				ಆದರೆ (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		ಆದರೆ (box->float_children) {
			fn = box;
			fnx = fny = 0;
		} ಇಲ್ಲದಿದ್ದರೆ {
			fn = fp;
			fnx = fx + x;
			fny = fy + y;
		}

		/* recurse first */
		layout_position_relative(unit_len_ctx, box, fn, fnx, fny);

		/* Ignore things we're not interested in. */
		ಆದರೆ (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			ಮುಂದುವರಿ;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		ಆದರೆ (box->type == BOX_INLINE && box->inline_ಕೊನೆ) {
			ರಚನೆ box *b;
			ಗಾಗಿ (b = box->ಮುಂದಿನ; b && b != box->inline_ಕೊನೆ;
					b = b->ಮುಂದಿನ) {
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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_get_box_bbox(
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ರಚನೆ box *box,
		ಪೂರ್ಣಾಂಕ *desc_x0, ಪೂರ್ಣಾಂಕ *desc_y0,
		ಪೂರ್ಣಾಂಕ *desc_x1, ಪೂರ್ಣಾಂಕ *desc_y1)
{
	*desc_x0 = -box->border[ಎಡ_2].ಅಗಲ;
	*desc_y0 = -box->border[ತುದಿ_2].ಅಗಲ;
	*desc_x1 = box->padding[ಎಡ_2] + box->ಅಗಲ + box->padding[ಬಲ_2] +
			box->border[ಬಲ_2].ಅಗಲ;
	*desc_y1 = box->padding[ತುದಿ_2] + box->ಎತ್ತರ + box->padding[BOTTOM] +
			box->border[BOTTOM].ಅಗಲ;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	ಆದರೆ (box->type == BOX_ಖಂಡ && box->style != ಖಾಲಿ &&
			css_computed_ಅತಿಪ್ರವಾಹ_y(box->style) ==
					CSS_ಅತಿಪ್ರವಾಹ_VISIBLE &&
			box->object == ಖಾಲಿ) {
		css_fixed font_ಗಾತ್ರ = 0;
		css_unit font_unit = CSS_UNIT_PT;
		ಪೂರ್ಣಾಂಕ ಪಠ್ಯ_ಎತ್ತರ;

		css_computed_font_ಗಾತ್ರ(box->style, &font_ಗಾತ್ರ, &font_unit);
		ಪಠ್ಯ_ಎತ್ತರ = css_unit_len2device_px(box->style, unit_len_ctx,
				font_ಗಾತ್ರ, font_unit);
		ಪಠ್ಯ_ಎತ್ತರ = FIXTOINT(ಪಠ್ಯ_ಎತ್ತರ * 3 / 4);
		*desc_y0 = (*desc_y0 < -ಪಠ್ಯ_ಎತ್ತರ) ? *desc_y0 : -ಪಠ್ಯ_ಎತ್ತರ;
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
ಸ್ಥಿರ ಶೂನ್ಯ
layout_update_descendant_bbox(
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ರಚನೆ box *box,
		ರಚನೆ box *ಮಗು,
		ಪೂರ್ಣಾಂಕ off_x,
		ಪೂರ್ಣಾಂಕ off_y)
{
	ಪೂರ್ಣಾಂಕ ಮಗು_desc_x0, ಮಗು_desc_y0, ಮಗು_desc_x1, ಮಗು_desc_y1;

	/* get coordinates of child relative to box */
	ಪೂರ್ಣಾಂಕ ಮಗು_x = ಮಗು->x - off_x;
	ಪೂರ್ಣಾಂಕ ಮಗು_y = ಮಗು->y - off_y;

	bool html_object = (ಮಗು->object &&
			content_get_type(ಮಗು->object) == CONTENT_HTML);

	ಎಣಿಕೆ css_ಅತಿಪ್ರವಾಹ_e ಅತಿಪ್ರವಾಹ_x = CSS_ಅತಿಪ್ರವಾಹ_VISIBLE;
	ಎಣಿಕೆ css_ಅತಿಪ್ರವಾಹ_e ಅತಿಪ್ರವಾಹ_y = CSS_ಅತಿಪ್ರವಾಹ_VISIBLE;

	ಆದರೆ (ಮಗು->style != ಖಾಲಿ) {
		ಅತಿಪ್ರವಾಹ_x = css_computed_ಅತಿಪ್ರವಾಹ_x(ಮಗು->style);
		ಅತಿಪ್ರವಾಹ_y = css_computed_ಅತಿಪ್ರವಾಹ_y(ಮಗು->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, ಮಗು,
			&ಮಗು_desc_x0, &ಮಗು_desc_y0,
			&ಮಗು_desc_x1, &ಮಗು_desc_y1);

	ಆದರೆ (ಅತಿಪ್ರವಾಹ_x == CSS_ಅತಿಪ್ರವಾಹ_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		ಮಗು_desc_x0 = ಮಗು->descendant_x0;
		ಮಗು_desc_x1 = ಮಗು->descendant_x1;
	}
	ಆದರೆ (ಅತಿಪ್ರವಾಹ_y == CSS_ಅತಿಪ್ರವಾಹ_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		ಮಗು_desc_y0 = ಮಗು->descendant_y0;
		ಮಗು_desc_y1 = ಮಗು->descendant_y1;
	}

	ಮಗು_desc_x0 += ಮಗು_x;
	ಮಗು_desc_y0 += ಮಗು_y;
	ಮಗು_desc_x1 += ಮಗು_x;
	ಮಗು_desc_y1 += ಮಗು_y;

	/* increase box's descendant bbox to contain descendants */
	ಆದರೆ (ಮಗು_desc_x0 < box->descendant_x0)
		box->descendant_x0 = ಮಗು_desc_x0;
	ಆದರೆ (ಮಗು_desc_y0 < box->descendant_y0)
		box->descendant_y0 = ಮಗು_desc_y0;
	ಆದರೆ (box->descendant_x1 < ಮಗು_desc_x1)
		box->descendant_x1 = ಮಗು_desc_x1;
	ಆದರೆ (box->descendant_y1 < ಮಗು_desc_y1)
		box->descendant_y1 = ಮಗು_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_calculate_descendant_bboxes(
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ರಚನೆ box *box)
{
	ರಚನೆ box *ಮಗು;

	ಪ್ರತಿಜ್ಞಾ(box->ಅಗಲ != UNKNOWN_ಅಗಲ);
	ಪ್ರತಿಜ್ಞಾ(box->ಎತ್ತರ != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	ಆದರೆ (box->object && content_get_type(box->object) == CONTENT_HTML) {
		ಆದರೆ (box->descendant_x1 < content_get_ಅಗಲ(box->object))
			box->descendant_x1 = content_get_ಅಗಲ(box->object);
		ಆದರೆ (box->descendant_y1 < content_get_ಎತ್ತರ(box->object))
			box->descendant_y1 = content_get_ಎತ್ತರ(box->object);
	}

	ಆದರೆ (box->iframe != ಖಾಲಿ) {
		ಪೂರ್ಣಾಂಕ x, y;
		box_coords(box, &x, &y);

		browser_ಕಿಟಕಿ_ಗಣ_position(box->iframe, x, y);
		browser_ಕಿಟಕಿ_ಗಣ_dimensions(box->iframe,
				box->ಅಗಲ, box->ಎತ್ತರ);
		browser_ಕಿಟಕಿ_reformat(box->iframe, true,
				box->ಅಗಲ, box->ಎತ್ತರ);
	}

	ಆದರೆ (box->type == BOX_INLINE || box->type == BOX_ಪಠ್ಯ)
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (box->type == BOX_INLINE_ಕೊನೆ) {
		box = box->inline_ಕೊನೆ;
		ಗಾಗಿ (ಮಗು = box->ಮುಂದಿನ; ಮಗು;
				ಮಗು = ಮಗು->ಮುಂದಿನ) {
			ಆದರೆ (ಮಗು->type == BOX_FLOAT_ಎಡ ||
					ಮಗು->type == BOX_FLOAT_ಬಲ)
				ಮುಂದುವರಿ;

			layout_update_descendant_bbox(unit_len_ctx, box, ಮಗು,
					box->x, box->y);

			ಆದರೆ (ಮಗು == box->inline_ಕೊನೆ)
				ನಿಲ್ಲಿಸು;
		}
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		ಹಿಂತಿರುಗಿ;

	ಗಾಗಿ (ಮಗು = box->children; ಮಗು; ಮಗು = ಮಗು->ಮುಂದಿನ) {
		ಆದರೆ (ಮಗು->type == BOX_FLOAT_ಎಡ ||
				ಮಗು->type == BOX_FLOAT_ಬಲ)
			ಮುಂದುವರಿ;

		layout_calculate_descendant_bboxes(unit_len_ctx, ಮಗು);

		ಆದರೆ (box->style && css_computed_ಅತಿಪ್ರವಾಹ_x(box->style) ==
				CSS_ಅತಿಪ್ರವಾಹ_HIDDEN &&
				css_computed_ಅತಿಪ್ರವಾಹ_y(box->style) ==
				CSS_ಅತಿಪ್ರವಾಹ_HIDDEN)
			ಮುಂದುವರಿ;

		layout_update_descendant_bbox(unit_len_ctx, box, ಮಗು, 0, 0);
	}

	ಗಾಗಿ (ಮಗು = box->float_children; ಮಗು; ಮಗು = ಮಗು->ಮುಂದಿನ_float) {
		ಪ್ರತಿಜ್ಞಾ(ಮಗು->type == BOX_FLOAT_ಎಡ ||
				ಮಗು->type == BOX_FLOAT_ಬಲ);

		layout_calculate_descendant_bboxes(unit_len_ctx, ಮಗು);

		layout_update_descendant_bbox(unit_len_ctx, box, ಮಗು, 0, 0);
	}

	ಆದರೆ (box->list_marker) {
		ಮಗು = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, ಮಗು);

		layout_update_descendant_bbox(unit_len_ctx, box, ಮಗು, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ)
{
	bool ret;
	ರಚನೆ box *doc = content->layout;
	ಸ್ಥಿರಾಂಕ ರಚನೆ gui_layout_ಕೋಷ್ಟಕ *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			ಅಗಲ, ಎತ್ತರ, nsurl_access(content_get_url(
					&content->ತಳ)));

	layout_minmax_ಖಂಡ(doc, font_func, content);

	layout_ಖಂಡ_ಕಂಡುಹಿಡಿ_dimensions(&content->unit_len_ctx,
			ಅಗಲ, ಎತ್ತರ, 0, 0, doc);
	doc->x = doc->margin[ಎಡ_2] + doc->border[ಎಡ_2].ಅಗಲ;
	doc->y = doc->margin[ತುದಿ_2] + doc->border[ತುದಿ_2].ಅಗಲ;
	ಅಗಲ -= doc->margin[ಎಡ_2] + doc->border[ಎಡ_2].ಅಗಲ +
			doc->padding[ಎಡ_2] + doc->padding[ಬಲ_2] +
			doc->border[ಬಲ_2].ಅಗಲ + doc->margin[ಬಲ_2];
	ಆದರೆ (ಅಗಲ < 0) {
		ಅಗಲ = 0;
	}
	doc->ಅಗಲ = ಅಗಲ;

	ret = layout_ಖಂಡ_context(doc, ಎತ್ತರ, content);

	/* make <html> and <body> fill available height */
	ಆದರೆ (doc->y + doc->padding[ತುದಿ_2] + doc->ಎತ್ತರ + doc->padding[BOTTOM] +
			doc->border[BOTTOM].ಅಗಲ + doc->margin[BOTTOM] <
			ಎತ್ತರ) {
		doc->ಎತ್ತರ = ಎತ್ತರ - (doc->y + doc->padding[ತುದಿ_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].ಅಗಲ +
				doc->margin[BOTTOM]);
		ಆದರೆ (doc->children)
			doc->children->ಎತ್ತರ = doc->ಎತ್ತರ -
					(doc->children->margin[ತುದಿ_2] +
					 doc->children->border[ತುದಿ_2].ಅಗಲ +
					 doc->children->padding[ತುದಿ_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].ಅಗಲ +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	ಹಿಂತಿರುಗಿ ret;
}
