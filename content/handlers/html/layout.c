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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <math.h>
#అన్తర్భావ <dom/dom.h>

#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/talloc.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "netsurf/browser_window.h"
#అన్తర్భావ "netsurf/layout.h"
#అన్తర్భావ "content/content.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "css/utils.h"
#అన్తర్భావ "desktop/scrollbar.h"
#అన్తర్భావ "desktop/textarea.h"

#అన్తర్భావ "html/html.h"
#అన్తర్భావ "html/html_save.h"
#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_inspect.h"
#అన్తర్భావ "html/font.h"
#అన్తర్భావ "html/form_internal.h"
#అన్తర్భావ "html/layout.h"
#అన్తర్భావ "html/layout_internal.h"
#అన్తర్భావ "html/table.h"

/** Array of per-side access functions for computed style margins. */
స్థిరరాశి css_len_func margin_funcs[4] = {
	[శిఖరం_2]    = css_computed_margin_శిఖరం,
	[కుడి_2]  = css_computed_margin_కుడి,
	[BOTTOM] = css_computed_margin_bottom,
	[ఎడమ_2]   = css_computed_margin_ఎడమ,
};

/** Array of per-side access functions for computed style paddings. */
స్థిరరాశి css_len_func padding_funcs[4] = {
	[శిఖరం_2]    = css_computed_padding_శిఖరం,
	[కుడి_2]  = css_computed_padding_కుడి,
	[BOTTOM] = css_computed_padding_bottom,
	[ఎడమ_2]   = css_computed_padding_ఎడమ,
};

/** Array of per-side access functions for computed style border_widths. */
స్థిరరాశి css_len_func border_వెడల్పు_funcs[4] = {
	[శిఖరం_2]    = css_computed_border_శిఖరం_వెడల్పు,
	[కుడి_2]  = css_computed_border_కుడి_వెడల్పు,
	[BOTTOM] = css_computed_border_bottom_వెడల్పు,
	[ఎడమ_2]   = css_computed_border_ఎడమ_వెడల్పు,
};

/** Array of per-side access functions for computed style border styles. */
స్థిరరాశి css_border_style_func border_style_funcs[4] = {
	[శిఖరం_2]    = css_computed_border_శిఖరం_style,
	[కుడి_2]  = css_computed_border_కుడి_style,
	[BOTTOM] = css_computed_border_bottom_style,
	[ఎడమ_2]   = css_computed_border_ఎడమ_style,
};

/** Array of per-side access functions for computed style border colors. */
స్థిరరాశి css_border_color_func border_color_funcs[4] = {
	[శిఖరం_2]    = css_computed_border_శిఖరం_color,
	[కుడి_2]  = css_computed_border_కుడి_color,
	[BOTTOM] = css_computed_border_bottom_color,
	[ఎడమ_2]   = css_computed_border_ఎడమ_color,
};

/* forward declaration to break cycles */
స్థిర శూన్య layout_minmax_ఖండం(
		నిర్మాణం box *ఖండం,
		స్థిరరాశి నిర్మాణం gui_layout_పట్టిక *font_func,
		స్థిరరాశి html_content *content);

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
స్థిర శూన్య
layout_get_object_dimensions(నిర్మాణం box *box,
			     పూర్ణాంకం *వెడల్పు, పూర్ణాంకం *ఎత్తు,
			     పూర్ణాంకం min_వెడల్పు, పూర్ణాంకం max_వెడల్పు,
			     పూర్ణాంకం min_ఎత్తు, పూర్ణాంకం max_ఎత్తు)
{
	ప్రతిజ్ఞా(box->object != ఖాళీ);
	ప్రతిజ్ఞా(వెడల్పు != ఖాళీ && ఎత్తు != ఖాళీ);

	ఒకవేళ (*వెడల్పు == AUTO && *ఎత్తు == AUTO) {
		/* No given dimensions */

		bool scaled = false;
		పూర్ణాంకం intrinsic_వెడల్పు = content_get_వెడల్పు(box->object);
		పూర్ణాంకం intrinsic_ఎత్తు = content_get_ఎత్తు(box->object);

		/* use intrinsic dimensions */
		*వెడల్పు = intrinsic_వెడల్పు;
		*ఎత్తు = intrinsic_ఎత్తు;

		/* Deal with min/max-width first */
		ఒకవేళ (min_వెడల్పు >  0 && min_వెడల్పు > *వెడల్పు) {
			*వెడల్పు = min_వెడల్పు;
			scaled = true;
		}
		ఒకవేళ (max_వెడల్పు >= 0 && max_వెడల్పు < *వెడల్పు) {
			*వెడల్పు = max_వెడల్పు;
			scaled = true;
		}

		ఒకవేళ (scaled && (intrinsic_వెడల్పు != 0)) {
			/* Update height */
			*ఎత్తు = (*వెడల్పు * intrinsic_ఎత్తు) /
					intrinsic_వెడల్పు;
		}

		scaled = false;
		/* Deal with min/max-height */
		ఒకవేళ (min_ఎత్తు >  0 && min_ఎత్తు > *ఎత్తు) {
			*ఎత్తు = min_ఎత్తు;
			scaled = true;
		}
		ఒకవేళ (max_ఎత్తు >= 0 && max_ఎత్తు < *ఎత్తు) {
			*ఎత్తు = max_ఎత్తు;
			scaled = true;
		}

		ఒకవేళ (scaled && (intrinsic_ఎత్తు != 0)) {
			/* Update width */
			*వెడల్పు = (*ఎత్తు * intrinsic_వెడల్పు) /
					intrinsic_ఎత్తు;
		}

	} లేకపోతే ఒకవేళ (*వెడల్పు == AUTO) {
		/* Have given height; width is calculated from the given height
		 * and ratio of intrinsic dimensions */
		పూర్ణాంకం intrinsic_వెడల్పు = content_get_వెడల్పు(box->object);
		పూర్ణాంకం intrinsic_ఎత్తు = content_get_ఎత్తు(box->object);

		ఒకవేళ (intrinsic_ఎత్తు != 0)
			*వెడల్పు = (*ఎత్తు * intrinsic_వెడల్పు) /
					intrinsic_ఎత్తు;
		లేకపోతే
			*వెడల్పు = intrinsic_వెడల్పు;

		ఒకవేళ (min_వెడల్పు >  0 && min_వెడల్పు > *వెడల్పు)
			*వెడల్పు = min_వెడల్పు;
		ఒకవేళ (max_వెడల్పు >= 0 && max_వెడల్పు < *వెడల్పు)
			*వెడల్పు = max_వెడల్పు;

	} లేకపోతే ఒకవేళ (*ఎత్తు == AUTO) {
		/* Have given width; height is calculated from the given width
		 * and ratio of intrinsic dimensions */
		పూర్ణాంకం intrinsic_వెడల్పు = content_get_వెడల్పు(box->object);
		పూర్ణాంకం intrinsic_ఎత్తు = content_get_ఎత్తు(box->object);

		ఒకవేళ (min_వెడల్పు >  0 && min_వెడల్పు > *వెడల్పు)
			*వెడల్పు = min_వెడల్పు;
		ఒకవేళ (max_వెడల్పు >= 0 && max_వెడల్పు < *వెడల్పు)
			*వెడల్పు = max_వెడల్పు;

		ఒకవేళ (intrinsic_వెడల్పు != 0)
			*ఎత్తు = (*వెడల్పు * intrinsic_ఎత్తు) /
					intrinsic_వెడల్పు;
		లేకపోతే
			*ఎత్తు = intrinsic_ఎత్తు;
	}
}


/**
 * Calculate the text-indent length.
 *
 * \param  style  style of block
 * \param  width  width of containing block
 * \return  length of indent
 */
స్థిర పూర్ణాంకం layout_పాఠ్యం_indent(
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		స్థిరరాశి css_computed_style *style, పూర్ణాంకం వెడల్పు)
{
	css_fixed విలువ = 0;
	css_unit unit = CSS_UNIT_PX;

	css_computed_పాఠ్యం_indent(style, &విలువ, &unit);

	ఒకవేళ (unit == CSS_UNIT_PCT) {
		తిరిగి FPCT_యొక్క_INT_TOINT(విలువ, వెడల్పు);
	} లేకపోతే {
		తిరిగి FIXTOINT(css_unit_len2device_px(style, unit_len_ctx,
				విలువ, unit));
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
స్థిర శూన్య layout_minmax_పట్టిక(నిర్మాణం box *పట్టిక,
		స్థిరరాశి నిర్మాణం gui_layout_పట్టిక *font_func,
		స్థిరరాశి html_content *content)
{
	గుర్తులేని పూర్ణాంకం i, j;
	పూర్ణాంకం వెడల్పు;
	పూర్ణాంకం border_spacing_h = 0;
	పూర్ణాంకం పట్టిక_min = 0, పట్టిక_max = 0;
	పూర్ణాంకం extra_fixed = 0;
	దశాంశం extra_frac = 0;
	నిర్మాణం స్తంభం *నిలువు;
	నిర్మాణం box *అడ్డువరుస_సమూహం, *అడ్డువరుస, *cell;

	/* check if the widths have already been calculated */
	ఒకవేళ (పట్టిక->max_వెడల్పు != UNKNOWN_MAX_వెడల్పు)
		తిరిగి;

	ఒకవేళ (పట్టిక_calculate_స్తంభం_types(&content->unit_len_ctx, పట్టిక) == false) {
		NSLOG(netsurf, ERROR,
				"Could not establish table column types.");
		తిరిగి;
	}
	నిలువు = పట్టిక->నిలువు;

	/* start with 0 except for fixed-width columns */
	కొరకు (i = 0; i != పట్టిక->columns; i++) {
		ఒకవేళ (నిలువు[i].type == స్తంభం_వెడల్పు_FIXED)
			నిలువు[i].min = నిలువు[i].max = నిలువు[i].వెడల్పు;
		లేకపోతే
			నిలువు[i].min = నిలువు[i].max = 0;
	}

	/* border-spacing is used in the separated borders model */
	ఒకవేళ (css_computed_border_collapse(పట్టిక->style) ==
			CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		css_computed_border_spacing(పట్టిక->style, &h, &hu, &v, &vu);

		border_spacing_h = FIXTOINT(css_unit_len2device_px(
				పట్టిక->style,
				&content->unit_len_ctx,
				h, hu));
	}

	/* 1st pass: consider cells with colspan 1 only */
	కొరకు (అడ్డువరుస_సమూహం = పట్టిక->children; అడ్డువరుస_సమూహం; అడ్డువరుస_సమూహం =అడ్డువరుస_సమూహం->తదుపరి)
	కొరకు (అడ్డువరుస = అడ్డువరుస_సమూహం->children; అడ్డువరుస; అడ్డువరుస = అడ్డువరుస->తదుపరి)
	కొరకు (cell = అడ్డువరుస->children; cell; cell = cell->తదుపరి) {
		ప్రతిజ్ఞా(cell->type == BOX_పట్టిక_CELL);
		ప్రతిజ్ఞా(cell->style);
		/** TODO: Handle colspan="0" correctly.
		 *        It's currently converted to 1 in box normaisation */
		ప్రతిజ్ఞా(cell->columns != 0);

		ఒకవేళ (cell->columns != 1)
			కొనసాగు;

		layout_minmax_ఖండం(cell, font_func, content);
		i = cell->ప్రారంభం_స్తంభం;

		ఒకవేళ (నిలువు[i].positioned)
			కొనసాగు;

		/* update column min, max widths using cell widths */
		ఒకవేళ (నిలువు[i].min < cell->min_వెడల్పు)
			నిలువు[i].min = cell->min_వెడల్పు;
		ఒకవేళ (నిలువు[i].max < cell->max_వెడల్పు)
			నిలువు[i].max = cell->max_వెడల్పు;
	}

	/* 2nd pass: cells which span multiple columns */
	కొరకు (అడ్డువరుస_సమూహం = పట్టిక->children; అడ్డువరుస_సమూహం; అడ్డువరుస_సమూహం =అడ్డువరుస_సమూహం->తదుపరి)
	కొరకు (అడ్డువరుస = అడ్డువరుస_సమూహం->children; అడ్డువరుస; అడ్డువరుస = అడ్డువరుస->తదుపరి)
	కొరకు (cell = అడ్డువరుస->children; cell; cell = cell->తదుపరి) {
		గుర్తులేని పూర్ణాంకం flexible_columns = 0;
		పూర్ణాంకం min = 0, max = 0, fixed_వెడల్పు = 0, extra;

		ఒకవేళ (cell->columns == 1)
			కొనసాగు;

		layout_minmax_ఖండం(cell, font_func, content);
		i = cell->ప్రారంభం_స్తంభం;

		/* find min width so far of spanned columns, and count
		 * number of non-fixed spanned columns and total fixed width */
		కొరకు (j = 0; j != cell->columns; j++) {
			min += నిలువు[i + j].min;
			ఒకవేళ (నిలువు[i + j].type == స్తంభం_వెడల్పు_FIXED)
				fixed_వెడల్పు += నిలువు[i + j].వెడల్పు;
			లేకపోతే
				flexible_columns++;
		}
		min += (cell->columns - 1) * border_spacing_h;

		/* distribute extra min to spanned columns */
		ఒకవేళ (min < cell->min_వెడల్పు) {
			ఒకవేళ (flexible_columns == 0) {
				extra = 1 + (cell->min_వెడల్పు - min) /
						cell->columns;
				కొరకు (j = 0; j != cell->columns; j++) {
					నిలువు[i + j].min += extra;
					ఒకవేళ (నిలువు[i + j].max < నిలువు[i + j].min)
						నిలువు[i + j].max = నిలువు[i + j].min;
				}
			} లేకపోతే {
				extra = 1 + (cell->min_వెడల్పు - min) /
						flexible_columns;
				కొరకు (j = 0; j != cell->columns; j++) {
					ఒకవేళ (నిలువు[i + j].type !=
							స్తంభం_వెడల్పు_FIXED) {
						నిలువు[i + j].min += extra;
						ఒకవేళ (నిలువు[i + j].max <
								నిలువు[i + j].min)
							నిలువు[i + j].max =
								నిలువు[i + j].min;
					}
				}
			}
		}

		/* find max width so far of spanned columns */
		కొరకు (j = 0; j != cell->columns; j++)
			max += నిలువు[i + j].max;
		max += (cell->columns - 1) * border_spacing_h;

		/* distribute extra max to spanned columns */
		ఒకవేళ (max < cell->max_వెడల్పు && flexible_columns) {
			extra = 1 + (cell->max_వెడల్పు - max) / flexible_columns;
			కొరకు (j = 0; j != cell->columns; j++)
				ఒకవేళ (నిలువు[i + j].type != స్తంభం_వెడల్పు_FIXED)
					నిలువు[i + j].max += extra;
		}
	}

	కొరకు (i = 0; i != పట్టిక->columns; i++) {
		ఒకవేళ (నిలువు[i].max < నిలువు[i].min) {
			box_dump(stderr, పట్టిక, 0, true);
			ప్రతిజ్ఞా(0);
		}
		పట్టిక_min += నిలువు[i].min;
		పట్టిక_max += నిలువు[i].max;
	}

	/* fixed width takes priority, unless it is too narrow */
	ఒకవేళ (css_computed_వెడల్పు_px(పట్టిక->style, &content->unit_len_ctx,
			-1, &వెడల్పు) == CSS_వెడల్పు_సమితి) {
		ఒకవేళ (పట్టిక_min < వెడల్పు)
			పట్టిక_min = వెడల్పు;
		ఒకవేళ (పట్టిక_max < వెడల్పు)
			పట్టిక_max = వెడల్పు;
	}

	/* add margins, border, padding to min, max widths */
	calculate_mbp_వెడల్పు(&content->unit_len_ctx,
			పట్టిక->style, ఎడమ_2, true, true, true,
			&extra_fixed, &extra_frac);
	calculate_mbp_వెడల్పు(&content->unit_len_ctx,
			పట్టిక->style, కుడి_2, true, true, true,
			&extra_fixed, &extra_frac);
	ఒకవేళ (extra_fixed < 0)
		extra_fixed = 0;
	ఒకవేళ (extra_frac < 0)
		extra_frac = 0;
	ఒకవేళ (1.0 <= extra_frac)
		extra_frac = 0.9;
	పట్టిక->min_వెడల్పు = (పట్టిక_min + extra_fixed) / (1.0 - extra_frac);
	పట్టిక->max_వెడల్పు = (పట్టిక_max + extra_fixed) / (1.0 - extra_frac);
	పట్టిక->min_వెడల్పు += (పట్టిక->columns + 1) * border_spacing_h;
	పట్టిక->max_వెడల్పు += (పట్టిక->columns + 1) * border_spacing_h;

	ప్రతిజ్ఞా(0 <= పట్టిక->min_వెడల్పు && పట్టిక->min_వెడల్పు <= పట్టిక->max_వెడల్పు);
}

/**
 * Helper to check if a box has percentage max width.
 *
 * \param[in]  b  Box to check.
 * \return true iff box has percnetage max width.
 */
స్థిర అంతఃవరుస bool box_has_percentage_max_వెడల్పు(నిర్మాణం box *b)
{
	css_unit unit = CSS_UNIT_PX;
	గణన css_max_వెడల్పు_e type;
	css_fixed విలువ = 0;

	ప్రతిజ్ఞా(b != ఖాళీ);

	type = css_computed_max_వెడల్పు(b->style, &విలువ, &unit);
	తిరిగి ((type == CSS_MAX_వెడల్పు_సమితి) && (unit == CSS_UNIT_PCT));
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
స్థిర నిర్మాణం box *
layout_minmax_పంక్తి(నిర్మాణం box *మొదటి,
		   పూర్ణాంకం *పంక్తి_min,
		   పూర్ణాంకం *పంక్తి_max,
		   bool మొదటి_పంక్తి,
		   bool *పంక్తి_has_ఎత్తు,
		   స్థిరరాశి నిర్మాణం gui_layout_పట్టిక *font_func,
		   స్థిరరాశి html_content *content)
{
	పూర్ణాంకం min = 0, max = 0, వెడల్పు, ఎత్తు, fixed;
	దశాంశం frac;
	size_t i, j;
	నిర్మాణం box *b;
	నిర్మాణం box *ఖండం;
	plot_font_style_t fstyle;
	bool no_wrap;

	ప్రతిజ్ఞా(మొదటి->మాతృ);
	ప్రతిజ్ఞా(మొదటి->మాతృ->మాతృ);
	ప్రతిజ్ఞా(మొదటి->మాతృ->మాతృ->style);

	ఖండం = మొదటి->మాతృ->మాతృ;
	no_wrap = (css_computed_white_space(ఖండం->style) ==
			CSS_WHITE_SPACE_NOWRAP ||
			css_computed_white_space(ఖండం->style) ==
			CSS_WHITE_SPACE_PRE);

	*పంక్తి_has_ఎత్తు = false;

	/* corresponds to the pass 1 loop in layout_line() */
	కొరకు (b = మొదటి; b; b = b->తదుపరి) {
		గణన css_వెడల్పు_e wtype;
		గణన css_ఎత్తు_e htype;
		గణన css_box_sizing_e bs;
		css_fixed విలువ = 0;
		css_unit unit = CSS_UNIT_PX;

		ప్రతిజ్ఞా(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG, "%p: min %i, max %i", b, min, max);

		ఒకవేళ (b->type == BOX_BR) {
			b = b->తదుపరి;
			ఆపు;
		}

		ఒకవేళ (lh__box_is_float_box(b)) {
			ప్రతిజ్ఞా(b->children);
			ఒకవేళ (b->children->type == BOX_పట్టిక)
				layout_minmax_పట్టిక(b->children, font_func,
						content);
			లేకపోతే
				layout_minmax_ఖండం(b->children, font_func,
						content);
			b->min_వెడల్పు = b->children->min_వెడల్పు;
			b->max_వెడల్పు = b->children->max_వెడల్పు;
			ఒకవేళ (min < b->min_వెడల్పు)
				min = b->min_వెడల్పు;
			max += b->max_వెడల్పు;
			కొనసాగు;
		}

		ఒకవేళ (b->type == BOX_INLINE_ఖండం || b->type == BOX_INLINE_FLEX) {
			layout_minmax_ఖండం(b, font_func, content);
			ఒకవేళ (min < b->min_వెడల్పు)
				min = b->min_వెడల్పు;
			max += b->max_వెడల్పు;

			ఒకవేళ (b->flags & HAS_ఎత్తు)
				*పంక్తి_has_ఎత్తు = true;
			కొనసాగు;
		}

		ప్రతిజ్ఞా(b->style);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		ఒకవేళ (b->type == BOX_INLINE && !b->object &&
				!(b->flags & REPLACE_DIM) &&
				!(b->flags & IFRAME)) {
			fixed = frac = 0;
			calculate_mbp_వెడల్పు(&content->unit_len_ctx,
					b->style, ఎడమ_2, true, true, true,
					&fixed, &frac);
			ఒకవేళ (!b->inline_చివర)
				calculate_mbp_వెడల్పు(&content->unit_len_ctx,
						b->style, కుడి_2,
						true, true, true,
						&fixed, &frac);
			ఒకవేళ (0 < fixed)
				max += fixed;
			*పంక్తి_has_ఎత్తు = true;
			/* \todo  update min width, consider fractional extra */
		} లేకపోతే ఒకవేళ (b->type == BOX_INLINE_చివర) {
			fixed = frac = 0;
			calculate_mbp_వెడల్పు(&content->unit_len_ctx,
					b->inline_చివర->style, కుడి_2,
					true, true, true,
					&fixed, &frac);
			ఒకవేళ (0 < fixed)
				max += fixed;

			ఒకవేళ (b->తదుపరి) {
				ఒకవేళ (b->space == UNKNOWN_వెడల్పు) {
					font_func->వెడల్పు(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			*పంక్తి_has_ఎత్తు = true;
			కొనసాగు;
		}

		ఒకవేళ (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			bool no_wrap_box;
			ఒకవేళ (!b->పాఠ్యం)
				కొనసాగు;

			no_wrap_box = (css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_NOWRAP ||
					css_computed_white_space(b->style) ==
					CSS_WHITE_SPACE_PRE);

			ఒకవేళ (b->వెడల్పు == UNKNOWN_వెడల్పు) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				ఒకవేళ (b->మాతృ->మాతృ->gadget &&
						b->మాతృ->మాతృ->gadget->type
						== GADGET_SELECT) {
					పూర్ణాంకం opt_maxwidth = 0;
					నిర్మాణం form_option *o;

					కొరకు (o = b->మాతృ->మాతృ->gadget->
							దత్తాంశం.select.వస్తువులు; o;
							o = o->తదుపరి) {
						పూర్ణాంకం opt_వెడల్పు;
						font_func->వెడల్పు(&fstyle,
								o->పాఠ్యం,
								అక్షరశ్రేణి_పొడవు(o->పాఠ్యం),
								&opt_వెడల్పు);

						ఒకవేళ (opt_maxwidth < opt_వెడల్పు)
							opt_maxwidth =opt_వెడల్పు;
					}

					b->వెడల్పు = opt_maxwidth;
					ఒకవేళ (nsoption_bool(core_select_menu))
						b->వెడల్పు += SCROLLBAR_వెడల్పు;

				} లేకపోతే {
					font_func->వెడల్పు(&fstyle, b->పాఠ్యం,
						b->నిడివి, &b->వెడల్పు);
					b->flags |= MEASURED;
				}
			}
			max += b->వెడల్పు;
			ఒకవేళ (b->తదుపరి) {
				ఒకవేళ (b->space == UNKNOWN_వెడల్పు) {
					font_func->వెడల్పు(&fstyle, " ", 1,
							 &b->space);
				}
				max += b->space;
			}

			ఒకవేళ (no_wrap) {
				/* Don't wrap due to block style,
				 * so min is the same as max */
				min = max;

			} లేకపోతే ఒకవేళ (no_wrap_box) {
				/* This inline box can't be wrapped,
				 * for min, consider box's width */
				ఒకవేళ (min < b->వెడల్పు)
					min = b->వెడల్పు;

			} లేకపోతే ఒకవేళ (b->మాతృ->flags & NEED_MIN) {
				/* If we care what the minimum width is,
				 * calculate it.  (It's only needed if we're
				 * shrinking-to-fit.) */
				/* min = widest single word */
				i = 0;
				చేయి {
					కొరకు (j = i; j != b->నిడివి &&
							b->పాఠ్యం[j] != ' '; j++)
						;
					font_func->వెడల్పు(&fstyle, b->పాఠ్యం + i,
							 j - i, &వెడల్పు);
					ఒకవేళ (min < వెడల్పు)
						min = వెడల్పు;
					i = j + 1;
				} అయితే (j != b->నిడివి);
			}

			*పంక్తి_has_ఎత్తు = true;

			కొనసాగు;
		}

		/* inline replaced, 10.3.2 and 10.6.2 */
		ప్రతిజ్ఞా(b->style);

		bs = css_computed_box_sizing(ఖండం->style);

		/* calculate box width */
		wtype = css_computed_వెడల్పు_px(b->style,
				&content->unit_len_ctx, -1, &వెడల్పు);
		ఒకవేళ (wtype == CSS_వెడల్పు_సమితి) {
			ఒకవేళ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				fixed = frac = 0;
				calculate_mbp_వెడల్పు(&content->unit_len_ctx,
						ఖండం->style, ఎడమ_2,
						false, true, true,
						&fixed, &frac);
				calculate_mbp_వెడల్పు(&content->unit_len_ctx,
						ఖండం->style, కుడి_2,
						false, true, true,
						&fixed, &frac);
				ఒకవేళ (వెడల్పు < fixed) {
					వెడల్పు = fixed;
				}
			}
			ఒకవేళ (వెడల్పు < 0)
				వెడల్పు = 0;
		} లేకపోతే {
			వెడల్పు = AUTO;
		}

		/*ఎత్తు*/
		htype = css_computed_ఎత్తు(b->style, &విలువ, &unit);
		ఒకవేళ (htype == CSS_ఎత్తు_సమితి) {
			ఎత్తు = FIXTOINT(css_unit_len2device_px(
					b->style,
					&content->unit_len_ctx,
					విలువ, unit));
		} లేకపోతే {
			ఎత్తు = AUTO;
		}

		ఒకవేళ (b->object || (b->flags & REPLACE_DIM)) {
			ఒకవేళ (b->object) {
				పూర్ణాంకం తాత్కాలిక_ఎత్తు = ఎత్తు;
				layout_get_object_dimensions(b,
						&వెడల్పు, &తాత్కాలిక_ఎత్తు,
						INT_MIN, గరిష్ఠ_పూర్ణాంకం,
						INT_MIN, గరిష్ఠ_పూర్ణాంకం);
			}

			fixed = frac = 0;
			ఒకవేళ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_వెడల్పు(&content->unit_len_ctx,
						b->style, ఎడమ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_వెడల్పు(&content->unit_len_ctx,
						b->style, కుడి_2,
						true, false, false,
						&fixed, &frac);
			} లేకపోతే {
				calculate_mbp_వెడల్పు(&content->unit_len_ctx,
						b->style, ఎడమ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_వెడల్పు(&content->unit_len_ctx,
						b->style, కుడి_2,
						true, true, true,
						&fixed, &frac);
			}
			ఒకవేళ (0 < వెడల్పు + fixed)
				వెడల్పు += fixed;
		} లేకపోతే ఒకవేళ (b->flags & IFRAME) {
			/* TODO: handle percentage widths properly */
			ఒకవేళ (వెడల్పు == AUTO)
				వెడల్పు = 400;

			fixed = frac = 0;
			ఒకవేళ (bs == CSS_BOX_SIZING_BORDER_BOX) {
				calculate_mbp_వెడల్పు(&content->unit_len_ctx,
						b->style, ఎడమ_2,
						true, false, false,
						&fixed, &frac);
				calculate_mbp_వెడల్పు(&content->unit_len_ctx,
						b->style, కుడి_2,
						true, false, false,
						&fixed, &frac);
			} లేకపోతే {
				calculate_mbp_వెడల్పు(&content->unit_len_ctx,
						b->style, ఎడమ_2,
						true, true, true,
						&fixed, &frac);
				calculate_mbp_వెడల్పు(&content->unit_len_ctx,
						b->style, కుడి_2,
						true, true, true,
						&fixed, &frac);
			}

			ఒకవేళ (0 < వెడల్పు + fixed)
				వెడల్పు += fixed;

		} లేకపోతే {
			/* form control with no object */
			ఒకవేళ (వెడల్పు == AUTO)
				వెడల్పు = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx,
						INTTOFIX(1), CSS_UNIT_EM));
		}

		ఒకవేళ (min < వెడల్పు && !box_has_percentage_max_వెడల్పు(b))
			min = వెడల్పు;
		ఒకవేళ (వెడల్పు > 0)
			max += వెడల్పు;

		*పంక్తి_has_ఎత్తు = true;
	}

	ఒకవేళ (మొదటి_పంక్తి) {
		/* todo: handle percentage values properly */
		/* todo: handle text-indent interaction with floats */
		పూర్ణాంకం పాఠ్యం_indent = layout_పాఠ్యం_indent(&content->unit_len_ctx,
				మొదటి->మాతృ->మాతృ->style, 100);
		min = (min + పాఠ్యం_indent < 0) ? 0 : min + పాఠ్యం_indent;
		max = (max + పాఠ్యం_indent < 0) ? 0 : max + పాఠ్యం_indent;
	}

	*పంక్తి_min = min;
	*పంక్తి_max = max;

	NSLOG(layout, DEBUG,  "line_min %i, line_max %i", min, max);

	ప్రతిజ్ఞా(b != మొదటి);
	ప్రతిజ్ఞా(0 <= *పంక్తి_min);
	ప్రతిజ్ఞా(*పంక్తి_min <= *పంక్తి_max);
	తిరిగి b;
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
స్థిర శూన్య
layout_minmax_inline_container(నిర్మాణం box *inline_container,
			       bool *has_ఎత్తు,
			       స్థిరరాశి నిర్మాణం gui_layout_పట్టిక *font_func,
			       స్థిరరాశి html_content *content)
{
	నిర్మాణం box *బిడ్డ;
	పూర్ణాంకం పంక్తి_min = 0, పంక్తి_max = 0;
	పూర్ణాంకం min = 0, max = 0;
	bool మొదటి_పంక్తి = true;
	bool పంక్తి_has_ఎత్తు;

	ప్రతిజ్ఞా(inline_container->type == BOX_INLINE_CONTAINER);

	/* check if the widths have already been calculated */
	ఒకవేళ (inline_container->max_వెడల్పు != UNKNOWN_MAX_వెడల్పు)
		తిరిగి;

	*has_ఎత్తు = false;

	కొరకు (బిడ్డ = inline_container->children; బిడ్డ; ) {
		బిడ్డ = layout_minmax_పంక్తి(బిడ్డ, &పంక్తి_min, &పంక్తి_max,
				మొదటి_పంక్తి, &పంక్తి_has_ఎత్తు, font_func,
				content);
		ఒకవేళ (min < పంక్తి_min)
			min = పంక్తి_min;
		ఒకవేళ (max < పంక్తి_max)
			max = పంక్తి_max;
		మొదటి_పంక్తి = false;
		*has_ఎత్తు |= పంక్తి_has_ఎత్తు;
	}

	inline_container->min_వెడల్పు = min;
	inline_container->max_వెడల్పు = max;

	ప్రతిజ్ఞా(0 <= inline_container->min_వెడల్పు &&
			inline_container->min_వెడల్పు <=
			inline_container->max_వెడల్పు);
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
స్థిర శూన్య layout_minmax_ఖండం(
		నిర్మాణం box *ఖండం,
		స్థిరరాశి నిర్మాణం gui_layout_పట్టిక *font_func,
		స్థిరరాశి html_content *content)
{
	నిర్మాణం box *బిడ్డ;
	పూర్ణాంకం min = 0, max = 0;
	పూర్ణాంకం extra_fixed = 0;
	దశాంశం extra_frac = 0;
	గణన css_వెడల్పు_e wtype = CSS_వెడల్పు_AUTO;
	css_fixed వెడల్పు = 0;
	css_unit wunit = CSS_UNIT_PX;
	గణన css_ఎత్తు_e htype = CSS_ఎత్తు_AUTO;
	css_fixed ఎత్తు = 0;
	css_unit hunit = CSS_UNIT_PX;
	గణన css_box_sizing_e bs = CSS_BOX_SIZING_CONTENT_BOX;
	bool using_min_border_box = false;
	bool using_max_border_box = false;
	bool బిడ్డ_has_ఎత్తు = false;

	ప్రతిజ్ఞా(ఖండం->type == BOX_ఖండం ||
			ఖండం->type == BOX_FLEX ||
			ఖండం->type == BOX_INLINE_FLEX ||
			ఖండం->type == BOX_INLINE_ఖండం ||
			ఖండం->type == BOX_పట్టిక_CELL);

	/* check if the widths have already been calculated */
	ఒకవేళ (ఖండం->max_వెడల్పు != UNKNOWN_MAX_వెడల్పు)
		తిరిగి;

	ఒకవేళ (ఖండం->style != ఖాళీ) {
		wtype = css_computed_వెడల్పు(ఖండం->style, &వెడల్పు, &wunit);
		htype = css_computed_ఎత్తు(ఖండం->style, &ఎత్తు, &hunit);
		bs = css_computed_box_sizing(ఖండం->style);
	}

	/* set whether the minimum width is of any interest for this box */
	ఒకవేళ (((ఖండం->మాతృ && lh__box_is_float_box(ఖండం->మాతృ)) ||
			ఖండం->type == BOX_INLINE_ఖండం ||
			ఖండం->type == BOX_INLINE_FLEX) &&
			wtype != CSS_వెడల్పు_సమితి) {
		/* box shrinks to fit; need minimum width */
		ఖండం->flags |= NEED_MIN;
	} లేకపోతే ఒకవేళ (ఖండం->type == BOX_పట్టిక_CELL) {
		/* box shrinks to fit; need minimum width */
		ఖండం->flags |= NEED_MIN;
	} లేకపోతే ఒకవేళ (ఖండం->మాతృ && (ఖండం->మాతృ->flags & NEED_MIN) &&
			wtype != CSS_వెడల్పు_సమితి) {
		/* box inside shrink-to-fit context; need minimum width */
		ఖండం->flags |= NEED_MIN;
	} లేకపోతే ఒకవేళ (ఖండం->మాతృ && (ఖండం->మాతృ->type == BOX_FLEX)) {
		/* box is flex item */
		ఖండం->flags |= NEED_MIN;
	}

	ఒకవేళ (ఖండం->gadget && (ఖండం->gadget->type == GADGET_TEXTBOX ||
			ఖండం->gadget->type == GADGET_PASSWORD ||
			ఖండం->gadget->type == GADGET_FILE ||
			ఖండం->gadget->type == GADGET_TEXTAREA) &&
			ఖండం->style && wtype == CSS_వెడల్పు_AUTO) {
		css_fixed కొలత = INTTOFIX(10);
		css_unit unit = CSS_UNIT_EM;

		min = max = FIXTOINT(css_unit_len2device_px(ఖండం->style,
				&content->unit_len_ctx, కొలత, unit));

		ఖండం->flags |= HAS_ఎత్తు;
	}

	ఒకవేళ (ఖండం->gadget && (ఖండం->gadget->type == GADGET_RADIO ||
			ఖండం->gadget->type == GADGET_CHECKBOX) &&
			ఖండం->style && wtype == CSS_వెడల్పు_AUTO) {
		css_fixed కొలత = INTTOFIX(1);
		css_unit unit = CSS_UNIT_EM;

		/* form checkbox or radio button
		 * if width is AUTO, set it to 1em */
		min = max = FIXTOINT(css_unit_len2device_px(ఖండం->style,
				&content->unit_len_ctx, కొలత, unit));

		ఖండం->flags |= HAS_ఎత్తు;
	}

	ఒకవేళ (ఖండం->object) {
		ఒకవేళ (content_get_type(ఖండం->object) == CONTENT_HTML) {
			layout_minmax_ఖండం(html_get_box_వృక్షం(ఖండం->object),
					font_func, content);
			min = html_get_box_వృక్షం(ఖండం->object)->min_వెడల్పు;
			max = html_get_box_వృక్షం(ఖండం->object)->max_వెడల్పు;
		} లేకపోతే {
			min = max = content_get_వెడల్పు(ఖండం->object);
		}

		ఖండం->flags |= HAS_ఎత్తు;
	} లేకపోతే ఒకవేళ (ఖండం->flags & IFRAME) {
		/** \todo do we need to know the min/max width of the iframe's
		 * content? */
		ఖండం->flags |= HAS_ఎత్తు;
	} లేకపోతే {
		/* recurse through children */
		కొరకు (బిడ్డ = ఖండం->children; బిడ్డ; బిడ్డ = బిడ్డ->తదుపరి) {
			ఎంపిక (బిడ్డ->type) {
			స్థితి BOX_FLEX:
			స్థితి BOX_ఖండం:
				layout_minmax_ఖండం(బిడ్డ, font_func,
						content);
				ఒకవేళ (బిడ్డ->flags & HAS_ఎత్తు)
					బిడ్డ_has_ఎత్తు = true;
				ఆపు;
			స్థితి BOX_INLINE_CONTAINER:
				ఒకవేళ (ఖండం->flags & NEED_MIN)
					బిడ్డ->flags |= NEED_MIN;

				layout_minmax_inline_container(బిడ్డ,
						&బిడ్డ_has_ఎత్తు, font_func,
						content);
				ఒకవేళ (బిడ్డ_has_ఎత్తు &&
						బిడ్డ ==
						బిడ్డ->మాతృ->children) {
					ఖండం->flags |= MAKE_ఎత్తు;
				}
				ఆపు;
			స్థితి BOX_పట్టిక:
				layout_minmax_పట్టిక(బిడ్డ, font_func,
						content);
				/* todo: fix for zero height tables */
				బిడ్డ_has_ఎత్తు = true;
				బిడ్డ->flags |= MAKE_ఎత్తు;
				ఆపు;
			అప్రమేయ:
				ప్రతిజ్ఞా(0);
			}
			ప్రతిజ్ఞా(బిడ్డ->max_వెడల్పు != UNKNOWN_MAX_వెడల్పు);

			ఒకవేళ (బిడ్డ->style &&
					(css_computed_position(బిడ్డ->style) ==
							CSS_POSITION_ABSOLUTE ||
					css_computed_position(బిడ్డ->style) ==
							CSS_POSITION_FIXED)) {
				/* This child is positioned out of normal flow,
				 * so it will have no affect on width */
				కొనసాగు;
			}

			ఒకవేళ (lh__box_is_flex_container(ఖండం) &&
			    lh__flex_main_is_horizontal(ఖండం)) {
				ఒకవేళ (ఖండం->style != ఖాళీ &&
				    css_computed_flex_wrap(ఖండం->style) ==
						CSS_FLEX_WRAP_NOWRAP) {
					min += బిడ్డ->min_వెడల్పు;
				} లేకపోతే {
					ఒకవేళ (min < బిడ్డ->min_వెడల్పు)
						min = బిడ్డ->min_వెడల్పు;
				}
				max += బిడ్డ->max_వెడల్పు;

			} లేకపోతే {
				ఒకవేళ (min < బిడ్డ->min_వెడల్పు)
					min = బిడ్డ->min_వెడల్పు;
				ఒకవేళ (max < బిడ్డ->max_వెడల్పు)
					max = బిడ్డ->max_వెడల్పు;
			}

			ఒకవేళ (బిడ్డ_has_ఎత్తు)
				ఖండం->flags |= HAS_ఎత్తు;
		}
	}

	ఒకవేళ (max < min) {
		box_dump(stderr, ఖండం, 0, true);
		ప్రతిజ్ఞా(0);
	}

	/* fixed width takes priority */
	ఒకవేళ (ఖండం->type != BOX_పట్టిక_CELL && !lh_box_is_flex_అంశం(ఖండం)) {
		bool border_box = bs == CSS_BOX_SIZING_BORDER_BOX;
		గణన css_max_వెడల్పు_e max_type;
		గణన css_min_వెడల్పు_e min_type;
		css_unit unit = CSS_UNIT_PX;
		css_fixed విలువ = 0;
		పూర్ణాంకం వెడల్పు;

		ఒకవేళ (css_computed_వెడల్పు_px(ఖండం->style, &content->unit_len_ctx,
				-1, &వెడల్పు) == CSS_వెడల్పు_సమితి) {
			min = max = వెడల్పు;
			using_max_border_box = border_box;
			using_min_border_box = border_box;
		}

		min_type = css_computed_min_వెడల్పు(ఖండం->style, &విలువ, &unit);
		ఒకవేళ (min_type == CSS_MIN_వెడల్పు_సమితి && unit != CSS_UNIT_PCT) {
			పూర్ణాంకం val = FIXTOINT(css_unit_len2device_px(ఖండం->style,
					&content->unit_len_ctx, విలువ, unit));

			ఒకవేళ (min < val) {
				min = val;
				using_min_border_box = border_box;
			}
		}

		max_type = css_computed_max_వెడల్పు(ఖండం->style, &విలువ, &unit);
		ఒకవేళ (max_type == CSS_MAX_వెడల్పు_సమితి && unit != CSS_UNIT_PCT) {
			పూర్ణాంకం val = FIXTOINT(css_unit_len2device_px(ఖండం->style,
					&content->unit_len_ctx, విలువ, unit));

			ఒకవేళ (val >= 0 && max > val) {
				max = val;
				using_max_border_box = border_box;
			}
		}
	}

	ఒకవేళ (htype == CSS_ఎత్తు_సమితి && hunit != CSS_UNIT_PCT &&
			ఎత్తు > INTTOFIX(0)) {
		ఖండం->flags |= MAKE_ఎత్తు;
		ఖండం->flags |= HAS_ఎత్తు;
	}

	/* add margins, border, padding to min, max widths */
	/* Note: we don't know available width here so percentage margin
	 * and paddings are wrong. */
	calculate_mbp_వెడల్పు(&content->unit_len_ctx, ఖండం->style, ఎడమ_2,
			false, true, true, &extra_fixed, &extra_frac);
	calculate_mbp_వెడల్పు(&content->unit_len_ctx, ఖండం->style, కుడి_2,
			false, true, true, &extra_fixed, &extra_frac);

	ఒకవేళ (using_max_border_box) {
		max -= extra_fixed;
		max = max(max, 0);
	}

	ఒకవేళ (using_min_border_box) {
		min -= extra_fixed;
		min = max(min, 0);
	}

	ఒకవేళ (max < min) {
		min = max;
	}

	calculate_mbp_వెడల్పు(&content->unit_len_ctx, ఖండం->style, ఎడమ_2,
			true, false, false, &extra_fixed, &extra_frac);
	calculate_mbp_వెడల్పు(&content->unit_len_ctx, ఖండం->style, కుడి_2,
			true, false, false, &extra_fixed, &extra_frac);

	ఒకవేళ (extra_fixed < 0)
		extra_fixed = 0;
	ఒకవేళ (extra_frac < 0)
		extra_frac = 0;
	ఒకవేళ (1.0 <= extra_frac)
		extra_frac = 0.9;
	ఒకవేళ (ఖండం->style != ఖాళీ &&
			(css_computed_float(ఖండం->style) == CSS_FLOAT_ఎడమ ||
			css_computed_float(ఖండం->style) == CSS_FLOAT_కుడి)) {
		/* floated boxs */
		ఖండం->min_వెడల్పు = min + extra_fixed;
		ఖండం->max_వెడల్పు = max + extra_fixed;
	} లేకపోతే {
		/* not floated */
		ఖండం->min_వెడల్పు = (min + extra_fixed) / (1.0 - extra_frac);
		ఖండం->max_వెడల్పు = (max + extra_fixed) / (1.0 - extra_frac);
	}

	ప్రతిజ్ఞా(0 <= ఖండం->min_వెడల్పు);
	ప్రతిజ్ఞా(ఖండం->min_వెడల్పు <= ఖండం->max_వెడల్పు);
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
స్థిర నిర్మాణం box*
layout_తదుపరి_margin_ఖండం(స్థిరరాశి css_unit_ctx *unit_len_ctx,
			 నిర్మాణం box *box,
			 నిర్మాణం box *ఖండం,
			 పూర్ణాంకం viewport_ఎత్తు,
			 పూర్ణాంకం *max_pos_margin,
			 పూర్ణాంకం *max_neg_margin)
{
	ప్రతిజ్ఞా(ఖండం != ఖాళీ);

	అయితే (box != ఖాళీ) {

		ఒకవేళ (box->type == BOX_INLINE_CONTAINER || (box->style &&
				(css_computed_position(box->style) !=
					CSS_POSITION_ABSOLUTE &&
				 css_computed_position(box->style) !=
					CSS_POSITION_FIXED))) {
			/* Not positioned */

			/* Get margins */
			ఒకవేళ (box->style) {
				layout_కనుగొను_dimensions(unit_len_ctx,
						box->మాతృ->వెడల్పు,
						viewport_ఎత్తు, box,
						box->style,
						ఖాళీ, ఖాళీ, ఖాళీ, ఖాళీ,
						ఖాళీ, ఖాళీ, box->margin,
						box->padding, box->border);

				/* Apply top margin */
				ఒకవేళ (*max_pos_margin < box->margin[శిఖరం_2])
					*max_pos_margin = box->margin[శిఖరం_2];
				లేకపోతే ఒకవేళ (*max_neg_margin < -box->margin[శిఖరం_2])
					*max_neg_margin = -box->margin[శిఖరం_2];
			}

			/* Check whether box is the box current margin collapses
			 * to */
			ఒకవేళ (box->flags & MAKE_ఎత్తు ||
					box->border[శిఖరం_2].వెడల్పు ||
					box->padding[శిఖరం_2] ||
					(box->style &&
					css_computed_అధికప్రవాహం_y(box->style) !=
					CSS_అధికప్రవాహం_VISIBLE) ||
					(box->type == BOX_INLINE_CONTAINER &&
					!box_is_మొదటి_బిడ్డ(box))) {
				/* Collapse to this box; return it */
				తిరిగి box;
			}
		}


		/* Find next box */
		ఒకవేళ (box->type == BOX_ఖండం && !box->object && box->children &&
				box->style &&
				css_computed_అధికప్రవాహం_y(box->style) ==
				CSS_అధికప్రవాహం_VISIBLE) {
			/* Down into children. */
			box = box->children;
		} లేకపోతే {
			ఒకవేళ (!box->తదుపరి) {
				/* No more siblings:
				 * Go up to first ancestor with a sibling. */
				చేయి {
					/* Apply bottom margin */
					ఒకవేళ (*max_pos_margin <
							box->margin[BOTTOM])
						*max_pos_margin =
							box->margin[BOTTOM];
					లేకపోతే ఒకవేళ (*max_neg_margin <
							-box->margin[BOTTOM])
						*max_neg_margin =
							-box->margin[BOTTOM];

					box = box->మాతృ;
				} అయితే (box != ఖండం && !box->తదుపరి);

				ఒకవేళ (box == ఖండం) {
					/* Margins don't collapse with stuff
					 * outside the block formatting context
					 */
					తిరిగి ఖండం;
				}
			}

			/* Apply bottom margin */
			ఒకవేళ (*max_pos_margin < box->margin[BOTTOM])
				*max_pos_margin = box->margin[BOTTOM];
			లేకపోతే ఒకవేళ (*max_neg_margin < -box->margin[BOTTOM])
				*max_neg_margin = -box->margin[BOTTOM];

			/* To next sibling. */
			box = box->తదుపరి;

			/* Get margins */
			ఒకవేళ (box->style) {
				layout_కనుగొను_dimensions(unit_len_ctx,
						box->మాతృ->వెడల్పు,
						viewport_ఎత్తు, box,
						box->style,
						ఖాళీ, ఖాళీ, ఖాళీ, ఖాళీ,
						ఖాళీ, ఖాళీ, box->margin,
						box->padding, box->border);
			}
		}
	}

	తిరిగి ఖాళీ;
}


/**
 * Find y coordinate which clears all floats on left and/or right.
 *
 * \param  fl	  first float in float list
 * \param  clear  type of clear
 * \return  y coordinate relative to ancestor box for floats
 */
స్థిర పూర్ణాంకం layout_clear(నిర్మాణం box *fl, గణన css_clear_e clear)
{
	పూర్ణాంకం y = 0;
	కొరకు (; fl; fl = fl->తదుపరి_float) {
		ఒకవేళ ((clear == CSS_CLEAR_ఎడమ || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_ఎడమ)
			ఒకవేళ (y < fl->y + fl->ఎత్తు)
				y = fl->y + fl->ఎత్తు;
		ఒకవేళ ((clear == CSS_CLEAR_కుడి || clear == CSS_CLEAR_BOTH) &&
				fl->type == BOX_FLOAT_కుడి)
			ఒకవేళ (y < fl->y + fl->ఎత్తు)
				y = fl->y + fl->ఎత్తు;
	}
	తిరిగి y;
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
స్థిర శూన్య
కనుగొను_sides(నిర్మాణం box *fl,
	   పూర్ణాంకం y0, పూర్ణాంకం y1,
	   పూర్ణాంకం *x0, పూర్ణాంకం *x1,
	   నిర్మాణం box **ఎడమ,
	   నిర్మాణం box **కుడి)
{
	పూర్ణాంకం fy0, fy1, fx0, fx1;

	NSLOG(layout, DEBUG, "y0 %i, y1 %i, x0 %i, x1 %i", y0, y1, *x0, *x1);

	*ఎడమ = *కుడి = 0;
	కొరకు (; fl; fl = fl->తదుపరి_float) {
		fy1 = fl->y + fl->ఎత్తు;
		ఒకవేళ (fy1 < y0) {
			/* Floats are sorted in order of decreasing bottom pos.
			 * Past here, all floats will be too high to concern us.
			 */
			తిరిగి;
		}
		fy0 = fl->y;
		ఒకవేళ (y0 < fy1 && fy0 <= y1) {
			ఒకవేళ (fl->type == BOX_FLOAT_ఎడమ) {
				fx1 = fl->x + fl->వెడల్పు;
				ఒకవేళ (*x0 < fx1) {
					*x0 = fx1;
					*ఎడమ = fl;
				}
			} లేకపోతే {
				fx0 = fl->x;
				ఒకవేళ (fx0 < *x1) {
					*x1 = fx0;
					*కుడి = fl;
				}
			}
		}
	}

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, left %p, right %p", *x0, *x1,
	      *ఎడమ, *కుడి);
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
స్థిర పూర్ణాంకం
layout_solve_వెడల్పు(నిర్మాణం box *box,
		   పూర్ణాంకం అందుబాటు_వెడల్పు,
		   పూర్ణాంకం వెడల్పు,
		   పూర్ణాంకం lm,
		   పూర్ణాంకం rm,
		   పూర్ణాంకం max_వెడల్పు,
		   పూర్ణాంకం min_వెడల్పు)
{
	bool auto_వెడల్పు = false;

	/* Increase specified left/right margins */
	ఒకవేళ (box->margin[ఎడమ_2] != AUTO && box->margin[ఎడమ_2] < lm &&
			box->margin[ఎడమ_2] >= 0)
		box->margin[ఎడమ_2] = lm;
	ఒకవేళ (box->margin[కుడి_2] != AUTO && box->margin[కుడి_2] < rm &&
			box->margin[కుడి_2] >= 0)
		box->margin[కుడి_2] = rm;

	/* Find width */
	ఒకవేళ (వెడల్పు == AUTO) {
		పూర్ణాంకం margin_ఎడమ = box->margin[ఎడమ_2];
		పూర్ణాంకం margin_కుడి = box->margin[కుడి_2];

		ఒకవేళ (margin_ఎడమ == AUTO) {
			margin_ఎడమ = lm;
		}
		ఒకవేళ (margin_కుడి == AUTO) {
			margin_కుడి = rm;
		}

		వెడల్పు = అందుబాటు_వెడల్పు -
				(margin_ఎడమ + box->border[ఎడమ_2].వెడల్పు +
				box->padding[ఎడమ_2] + box->padding[కుడి_2] +
				box->border[కుడి_2].వెడల్పు + margin_కుడి);
		వెడల్పు = వెడల్పు < 0 ? 0 : వెడల్పు;
		auto_వెడల్పు = true;
	}

	ఒకవేళ (max_వెడల్పు >= 0 && వెడల్పు > max_వెడల్పు) {
		/* max-width is admissable and width exceeds max-width */
		వెడల్పు = max_వెడల్పు;
		auto_వెడల్పు = false;
	}

	ఒకవేళ (min_వెడల్పు > 0 && వెడల్పు < min_వెడల్పు) {
		/* min-width is admissable and width is less than max-width */
		వెడల్పు = min_వెడల్పు;
		auto_వెడల్పు = false;
	}

	/* Width was auto, and unconstrained by min/max width, so we're done */
	ఒకవేళ (auto_వెడల్పు) {
		/* any other 'auto' become 0 or the minimum required values */
		ఒకవేళ (box->margin[ఎడమ_2] == AUTO) {
			box->margin[ఎడమ_2] = lm;
		}
		ఒకవేళ (box->margin[కుడి_2] == AUTO) {
			box->margin[కుడి_2] = rm;
		}
		తిరిగి వెడల్పు;
	}

	/* Width was not auto, or was constrained by min/max width
	 * Need to compute left/right margins */

	/* HTML alignment (only applies to over-constrained boxes) */
	ఒకవేళ (box->margin[ఎడమ_2] != AUTO && box->margin[కుడి_2] != AUTO &&
			box->మాతృ != ఖాళీ && box->మాతృ->style != ఖాళీ) {
		ఎంపిక (css_computed_పాఠ్యం_align(box->మాతృ->style)) {
		స్థితి CSS_పాఠ్యం_ALIGN_LIBCSS_కుడి:
			box->margin[ఎడమ_2] = AUTO;
			box->margin[కుడి_2] = 0;
			ఆపు;
		స్థితి CSS_పాఠ్యం_ALIGN_LIBCSS_CENTER:
			box->margin[ఎడమ_2] = box->margin[కుడి_2] = AUTO;
			ఆపు;
		స్థితి CSS_పాఠ్యం_ALIGN_LIBCSS_ఎడమ:
			box->margin[ఎడమ_2] = 0;
			box->margin[కుడి_2] = AUTO;
			ఆపు;
		అప్రమేయ:
			/* Leave it alone; no HTML alignment */
			ఆపు;
		}
	}

	ఒకవేళ (box->margin[ఎడమ_2] == AUTO && box->margin[కుడి_2] == AUTO) {
		/* make the margins equal, centering the element */
		box->margin[ఎడమ_2] = box->margin[కుడి_2] =
				(అందుబాటు_వెడల్పు - lm - rm -
				(box->border[ఎడమ_2].వెడల్పు + box->padding[ఎడమ_2] +
				వెడల్పు + box->padding[కుడి_2] +
				box->border[కుడి_2].వెడల్పు)) / 2;

		ఒకవేళ (box->margin[ఎడమ_2] < 0) {
			box->margin[కుడి_2] += box->margin[ఎడమ_2];
			box->margin[ఎడమ_2] = 0;
		}

		box->margin[ఎడమ_2] += lm;

	} లేకపోతే ఒకవేళ (box->margin[ఎడమ_2] == AUTO) {
		box->margin[ఎడమ_2] = అందుబాటు_వెడల్పు - lm -
				(box->border[ఎడమ_2].వెడల్పు + box->padding[ఎడమ_2] +
				వెడల్పు + box->padding[కుడి_2] +
				box->border[కుడి_2].వెడల్పు + box->margin[కుడి_2]);
		box->margin[ఎడమ_2] = box->margin[ఎడమ_2] < lm
				? lm : box->margin[ఎడమ_2];
	} లేకపోతే {
		/* margin-right auto or "over-constrained" */
		box->margin[కుడి_2] = అందుబాటు_వెడల్పు - rm -
				(box->margin[ఎడమ_2] + box->border[ఎడమ_2].వెడల్పు +
				 box->padding[ఎడమ_2] + వెడల్పు +
				 box->padding[కుడి_2] +
				 box->border[కుడి_2].వెడల్పు);
	}

	తిరిగి వెడల్పు;
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
స్థిర శూన్య
layout_ఖండం_కనుగొను_dimensions(స్థిరరాశి css_unit_ctx *unit_len_ctx,
			     పూర్ణాంకం అందుబాటు_వెడల్పు,
			     పూర్ణాంకం viewport_ఎత్తు,
			     పూర్ణాంకం lm,
			     పూర్ణాంకం rm,
			     నిర్మాణం box *box)
{
	పూర్ణాంకం వెడల్పు, max_వెడల్పు, min_వెడల్పు;
	పూర్ణాంకం ఎత్తు, max_ఎత్తు, min_ఎత్తు;
	పూర్ణాంకం *margin = box->margin;
	పూర్ణాంకం *padding = box->padding;
	నిర్మాణం box_border *border = box->border;
	స్థిరరాశి css_computed_style *style = box->style;

	layout_కనుగొను_dimensions(unit_len_ctx, అందుబాటు_వెడల్పు, viewport_ఎత్తు, box,
			style, &వెడల్పు, &ఎత్తు, &max_వెడల్పు, &min_వెడల్పు,
			&max_ఎత్తు, &min_ఎత్తు, margin, padding, border);

	ఒకవేళ (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* block-level replaced element, see 10.3.4 and 10.6.2 */
		layout_get_object_dimensions(box, &వెడల్పు, &ఎత్తు,
				min_వెడల్పు, max_వెడల్పు, min_ఎత్తు, max_ఎత్తు);
	}

	box->వెడల్పు = layout_solve_వెడల్పు(box, అందుబాటు_వెడల్పు, వెడల్పు, lm, rm,
			max_వెడల్పు, min_వెడల్పు);
	box->ఎత్తు = ఎత్తు;

	ఒకవేళ (margin[శిఖరం_2] == AUTO)
		margin[శిఖరం_2] = 0;
	ఒకవేళ (margin[BOTTOM] == AUTO)
		margin[BOTTOM] = 0;
}


/**
 * Manipulate a block's [RB]padding/height/width to accommodate scrollbars
 *
 * \param  box	  Box to apply scrollbar space too. Must be BOX_BLOCK.
 * \param  which  Which scrollbar to make space for. Must be RIGHT or BOTTOM.
 */
స్థిర శూన్య layout_ఖండం_కూడు_scrollbar(నిర్మాణం box *box, పూర్ణాంకం which)
{
	గణన css_అధికప్రవాహం_e అధికప్రవాహం_x, అధికప్రవాహం_y;

	ప్రతిజ్ఞా(box->type == BOX_ఖండం && (which == కుడి_2 || which == BOTTOM));

	ఒకవేళ (box->style == ఖాళీ)
		తిరిగి;

	అధికప్రవాహం_x = css_computed_అధికప్రవాహం_x(box->style);
	అధికప్రవాహం_y = css_computed_అధికప్రవాహం_y(box->style);

	ఒకవేళ (which == BOTTOM &&
			(అధికప్రవాహం_x == CSS_అధికప్రవాహం_SCROLL ||
			 అధికప్రవాహం_x == CSS_అధికప్రవాహం_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbar, unless height is AUTO */
		ఒకవేళ (box->ఎత్తు != AUTO &&
				(అధికప్రవాహం_x == CSS_అధికప్రవాహం_SCROLL ||
				box_hscrollbar_present(box))) {
			box->padding[BOTTOM] += SCROLLBAR_వెడల్పు;
		}

	} లేకపోతే ఒకవేళ (which == కుడి_2 &&
			(అధికప్రవాహం_y == CSS_అధికప్రవాహం_SCROLL ||
			 అధికప్రవాహం_y == CSS_అధికప్రవాహం_AUTO ||
			(box->object &&
			 content_get_type(box->object) == CONTENT_HTML))) {
		/* make space for scrollbars, unless width is AUTO */
		గణన css_ఎత్తు_e htype;
		css_fixed ఎత్తు = 0;
		css_unit hunit = CSS_UNIT_PX;
		htype = css_computed_ఎత్తు(box->style, &ఎత్తు, &hunit);

		ఒకవేళ (which == కుడి_2 && box->వెడల్పు != AUTO &&
				htype == CSS_ఎత్తు_సమితి &&
				(అధికప్రవాహం_y == CSS_అధికప్రవాహం_SCROLL ||
				box_vscrollbar_present(box))) {
			box->వెడల్పు -= SCROLLBAR_వెడల్పు;
			box->padding[కుడి_2] += SCROLLBAR_వెడల్పు;
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
స్థిర శూన్య layout_move_children(నిర్మాణం box *box, పూర్ణాంకం x, పూర్ణాంకం y)
{
	ప్రతిజ్ఞా(box);

	కొరకు (box = box->children; box; box = box->తదుపరి) {
		box->x += x;
		box->y += y;
	}
}


/* Documented in layout_internal.h */
bool layout_పట్టిక(
		నిర్మాణం box *పట్టిక,
		పూర్ణాంకం అందుబాటు_వెడల్పు,
		html_content *content)
{
	గుర్తులేని పూర్ణాంకం columns = పట్టిక->columns;  /* total columns */
	గుర్తులేని పూర్ణాంకం i;
	గుర్తులేని పూర్ణాంకం *అడ్డువరుస_పరిధి;
	పూర్ణాంకం *excess_y;
	పూర్ణాంకం పట్టిక_వెడల్పు, min_వెడల్పు = 0, max_వెడల్పు = 0;
	పూర్ణాంకం required_వెడల్పు = 0;
	పూర్ణాంకం x, remainder = 0, లెక్క = 0;
	పూర్ణాంకం పట్టిక_ఎత్తు = 0;
	పూర్ణాంకం min_ఎత్తు = 0;
	పూర్ణాంకం *xs;  /* array of column x positions */
	పూర్ణాంకం auto_వెడల్పు;
	పూర్ణాంకం అదనపు_వెడల్పు;
	పూర్ణాంకం relative_కూడిక = 0;
	పూర్ణాంకం border_spacing_h = 0, border_spacing_v = 0;
	పూర్ణాంకం అదనపు_ఎత్తు;
	పూర్ణాంకం positioned_columns = 0;
	నిర్మాణం box *containing_ఖండం = ఖాళీ;
	నిర్మాణం box *c;
	నిర్మాణం box *అడ్డువరుస;
	నిర్మాణం box *అడ్డువరుస_సమూహం;
	నిర్మాణం box **అడ్డువరుస_పరిధి_cell;
	నిర్మాణం స్తంభం *నిలువు;
	స్థిరరాశి css_computed_style *style = పట్టిక->style;
	గణన css_ఎత్తు_e htype;
	css_fixed విలువ = 0;
	css_unit unit = CSS_UNIT_PX;

	ప్రతిజ్ఞా(పట్టిక->type == BOX_పట్టిక);
	ప్రతిజ్ఞా(style);
	ప్రతిజ్ఞా(పట్టిక->children && పట్టిక->children->children);
	ప్రతిజ్ఞా(columns);

	/* allocate working buffers */
	నిలువు = జ్ఞాపకం_ఇవ్వు(columns * పరిమాణం నిలువు[0]);
	excess_y = జ్ఞాపకం_ఇవ్వు(columns * పరిమాణం excess_y[0]);
	అడ్డువరుస_పరిధి = జ్ఞాపకం_ఇవ్వు(columns * పరిమాణం అడ్డువరుస_పరిధి[0]);
	అడ్డువరుస_పరిధి_cell = జ్ఞాపకం_ఇవ్వు(columns * పరిమాణం అడ్డువరుస_పరిధి_cell[0]);
	xs = జ్ఞాపకం_ఇవ్వు((columns + 1) * పరిమాణం xs[0]);
	ఒకవేళ (!నిలువు || !xs || !అడ్డువరుస_పరిధి || !excess_y || !అడ్డువరుస_పరిధి_cell) {
		విడుదల(నిలువు);
		విడుదల(excess_y);
		విడుదల(అడ్డువరుస_పరిధి);
		విడుదల(అడ్డువరుస_పరిధి_cell);
		విడుదల(xs);
		తిరిగి false;
	}

	జ్ఞాపకం_నకలు(నిలువు, పట్టిక->నిలువు, పరిమాణం(నిలువు[0]) * columns);

	/* find margins, paddings, and borders for table and cells */
	layout_కనుగొను_dimensions(&content->unit_len_ctx, అందుబాటు_వెడల్పు, -1, పట్టిక,
			style, 0, 0, 0, 0, 0, 0, పట్టిక->margin, పట్టిక->padding,
			పట్టిక->border);
	కొరకు (అడ్డువరుస_సమూహం = పట్టిక->children; అడ్డువరుస_సమూహం;
			అడ్డువరుస_సమూహం = అడ్డువరుస_సమూహం->తదుపరి) {
		కొరకు (అడ్డువరుస = అడ్డువరుస_సమూహం->children; అడ్డువరుస; అడ్డువరుస = అడ్డువరుస->తదుపరి) {
			కొరకు (c = అడ్డువరుస->children; c; c = c->తదుపరి) {
				గణన css_అధికప్రవాహం_e అధికప్రవాహం_x;
				గణన css_అధికప్రవాహం_e అధికప్రవాహం_y;

				ప్రతిజ్ఞా(c->style);
				పట్టిక_వాడిన_border_for_cell(
						&content->unit_len_ctx, c);
				layout_కనుగొను_dimensions(&content->unit_len_ctx,
						అందుబాటు_వెడల్పు, -1, c,
						c->style, 0, 0, 0, 0, 0, 0,
						0, c->padding, c->border);

				అధికప్రవాహం_x = css_computed_అధికప్రవాహం_x(c->style);
				అధికప్రవాహం_y = css_computed_అధికప్రవాహం_y(c->style);

				ఒకవేళ (అధికప్రవాహం_x == CSS_అధికప్రవాహం_SCROLL ||
						అధికప్రవాహం_x ==
						CSS_అధికప్రవాహం_AUTO) {
					c->padding[BOTTOM] += SCROLLBAR_వెడల్పు;
				}
				ఒకవేళ (అధికప్రవాహం_y == CSS_అధికప్రవాహం_SCROLL ||
						అధికప్రవాహం_y ==
						CSS_అధికప్రవాహం_AUTO) {
					c->padding[కుడి_2] += SCROLLBAR_వెడల్పు;
				}
			}
		}
	}

	/* border-spacing is used in the separated borders model */
	ఒకవేళ (css_computed_border_collapse(style) ==
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
	ఒకవేళ (css_computed_వెడల్పు_px(style, &content->unit_len_ctx,
			అందుబాటు_వెడల్పు, &పట్టిక_వెడల్పు) == CSS_వెడల్పు_సమితి) {
		/* specified width includes border */
		పట్టిక_వెడల్పు -= పట్టిక->border[ఎడమ_2].వెడల్పు +
				పట్టిక->border[కుడి_2].వెడల్పు;
		పట్టిక_వెడల్పు = పట్టిక_వెడల్పు < 0 ? 0 : పట్టిక_వెడల్పు;

		auto_వెడల్పు = పట్టిక_వెడల్పు;
	} లేకపోతే {
		పట్టిక_వెడల్పు = AUTO;
		auto_వెడల్పు = అందుబాటు_వెడల్పు -
				((పట్టిక->margin[ఎడమ_2] == AUTO ? 0 :
						పట్టిక->margin[ఎడమ_2]) +
				 పట్టిక->border[ఎడమ_2].వెడల్పు +
				 పట్టిక->padding[ఎడమ_2] +
				 పట్టిక->padding[కుడి_2] +
				 పట్టిక->border[కుడి_2].వెడల్పు +
				 (పట్టిక->margin[కుడి_2] == AUTO ? 0 :
						పట్టిక->margin[కుడి_2]));
	}

	/* Find any table height specified within CSS/HTML */
	htype = css_computed_ఎత్తు(style, &విలువ, &unit);
	ఒకవేళ (htype == CSS_ఎత్తు_సమితి) {
		ఒకవేళ (unit == CSS_UNIT_PCT) {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			ఒకవేళ (css_computed_position(పట్టిక->style) ==
					CSS_POSITION_ABSOLUTE) {
				/* Table is absolutely positioned */
				ప్రతిజ్ఞా(పట్టిక->float_container);
				containing_ఖండం = పట్టిక->float_container;
			} లేకపోతే ఒకవేళ (పట్టిక->float_container &&
					css_computed_position(పట్టిక->style) !=
						CSS_POSITION_ABSOLUTE &&
					(css_computed_float(పట్టిక->style) ==
						CSS_FLOAT_ఎడమ ||
					css_computed_float(పట్టిక->style) ==
						CSS_FLOAT_కుడి)) {
				/* Table is a float */
				ప్రతిజ్ఞా(పట్టిక->మాతృ && పట్టిక->మాతృ->మాతృ &&
						పట్టిక->మాతృ->మాతృ->మాతృ);
				containing_ఖండం =
						పట్టిక->మాతృ->మాతృ->మాతృ;
			} లేకపోతే ఒకవేళ (పట్టిక->మాతృ && పట్టిక->మాతృ->type !=
					BOX_INLINE_CONTAINER) {
				/* Table is a block level element */
				containing_ఖండం = పట్టిక->మాతృ;
			} లేకపోతే ఒకవేళ (పట్టిక->మాతృ && పట్టిక->మాతృ->type ==
					BOX_INLINE_CONTAINER) {
				/* Table is an inline block */
				ప్రతిజ్ఞా(పట్టిక->మాతృ->మాతృ);
				containing_ఖండం = పట్టిక->మాతృ->మాతృ;
			}

			ఒకవేళ (containing_ఖండం) {
				css_fixed ignored = 0;

				htype = css_computed_ఎత్తు(
						containing_ఖండం->style,
						&ignored, &unit);
			}

			ఒకవేళ (containing_ఖండం &&
					containing_ఖండం->ఎత్తు != AUTO &&
					(css_computed_position(పట్టిక->style) ==
							CSS_POSITION_ABSOLUTE ||
					htype == CSS_ఎత్తు_సమితి)) {
				/* Table is absolutely positioned or its
				 * containing block has a valid specified
				 * height. (CSS 2.1 Section 10.5) */
				min_ఎత్తు = FPCT_యొక్క_INT_TOINT(విలువ,
						containing_ఖండం->ఎత్తు);
			}
		} లేకపోతే {
			/* This is the minimum height for the table
			 * (see 17.5.3) */
			min_ఎత్తు = FIXTOINT(css_unit_len2device_px(
					style, &content->unit_len_ctx,
					విలువ, unit));
		}
	}

	/* calculate width required by cells */
	కొరకు (i = 0; i != columns; i++) {

		NSLOG(layout, DEBUG,
		      "table %p, column %u: type %s, width %i, min %i, max %i",
		      పట్టిక,
		      i,
		      ((స్థిరరాశి అక్షరం *[]){
			      "UNKNOWN",
				      "FIXED",
				      "AUTO",
				      "PERCENT",
				      "RELATIVE",
				      })[నిలువు[i].type],
		      నిలువు[i].వెడల్పు,
		      నిలువు[i].min,
		      నిలువు[i].max);


		ఒకవేళ (నిలువు[i].positioned) {
			positioned_columns++;
			కొనసాగు;
		} లేకపోతే ఒకవేళ (నిలువు[i].type == స్తంభం_వెడల్పు_FIXED) {
			ఒకవేళ (నిలువు[i].వెడల్పు < నిలువు[i].min)
				నిలువు[i].వెడల్పు = నిలువు[i].max = నిలువు[i].min;
			లేకపోతే
				నిలువు[i].min = నిలువు[i].max = నిలువు[i].వెడల్పు;
			required_వెడల్పు += నిలువు[i].వెడల్పు;
		} లేకపోతే ఒకవేళ (నిలువు[i].type == స్తంభం_వెడల్పు_PERCENT) {
			పూర్ణాంకం వెడల్పు = నిలువు[i].వెడల్పు * auto_వెడల్పు / 100;
			required_వెడల్పు += నిలువు[i].min < వెడల్పు ? వెడల్పు :
					నిలువు[i].min;
		} లేకపోతే
			required_వెడల్పు += నిలువు[i].min;

		NSLOG(layout, DEBUG,  "required_width %i", required_వెడల్పు);
	}
	required_వెడల్పు += (columns + 1 - positioned_columns) *
			border_spacing_h;

	NSLOG(layout, DEBUG,
	      "width %i, min %i, max %i, auto %i, required %i", పట్టిక_వెడల్పు,
	      పట్టిక->min_వెడల్పు, పట్టిక->max_వెడల్పు, auto_వెడల్పు, required_వెడల్పు);

	ఒకవేళ (auto_వెడల్పు < required_వెడల్పు) {
		/* table narrower than required width for columns:
		 * treat percentage widths as maximums */
		కొరకు (i = 0; i != columns; i++) {
			ఒకవేళ (నిలువు[i].type == స్తంభం_వెడల్పు_RELATIVE)
				కొనసాగు;
			ఒకవేళ (నిలువు[i].type == స్తంభం_వెడల్పు_PERCENT) {
				నిలువు[i].max = auto_వెడల్పు * నిలువు[i].వెడల్పు / 100;
				ఒకవేళ (నిలువు[i].max < నిలువు[i].min)
					నిలువు[i].max = నిలువు[i].min;
			}
			min_వెడల్పు += నిలువు[i].min;
			max_వెడల్పు += నిలువు[i].max;
		}
	} లేకపోతే {
		/* take percentages exactly */
		కొరకు (i = 0; i != columns; i++) {
			ఒకవేళ (నిలువు[i].type == స్తంభం_వెడల్పు_RELATIVE)
				కొనసాగు;
			ఒకవేళ (నిలువు[i].type == స్తంభం_వెడల్పు_PERCENT) {
				పూర్ణాంకం వెడల్పు = auto_వెడల్పు * నిలువు[i].వెడల్పు / 100;
				ఒకవేళ (వెడల్పు < నిలువు[i].min)
					వెడల్పు = నిలువు[i].min;
				నిలువు[i].min = నిలువు[i].వెడల్పు = నిలువు[i].max = వెడల్పు;
				నిలువు[i].type = స్తంభం_వెడల్పు_FIXED;
			}
			min_వెడల్పు += నిలువు[i].min;
			max_వెడల్పు += నిలువు[i].max;
		}
	}

	/* allocate relative widths */
	అదనపు_వెడల్పు = auto_వెడల్పు;
	కొరకు (i = 0; i != columns; i++) {
		ఒకవేళ (నిలువు[i].type == స్తంభం_వెడల్పు_RELATIVE)
			relative_కూడిక += నిలువు[i].వెడల్పు;
		లేకపోతే ఒకవేళ (నిలువు[i].type == స్తంభం_వెడల్పు_FIXED)
			అదనపు_వెడల్పు -= నిలువు[i].వెడల్పు;
		లేకపోతే
			అదనపు_వెడల్పు -= నిలువు[i].min;
	}
	అదనపు_వెడల్పు -= (columns + 1) * border_spacing_h;
	ఒకవేళ (relative_కూడిక != 0) {
		ఒకవేళ (అదనపు_వెడల్పు < 0)
			అదనపు_వెడల్పు = 0;
		కొరకు (i = 0; i != columns; i++) {
			ఒకవేళ (నిలువు[i].type == స్తంభం_వెడల్పు_RELATIVE) {
				నిలువు[i].min = ceil(నిలువు[i].max =
						(దశాంశం) అదనపు_వెడల్పు
						* (దశాంశం) నిలువు[i].వెడల్పు
						/ relative_కూడిక);
				min_వెడల్పు += నిలువు[i].min;
				max_వెడల్పు += నిలువు[i].max;
			}
		}
	}
	min_వెడల్పు += (columns + 1) * border_spacing_h;
	max_వెడల్పు += (columns + 1) * border_spacing_h;

	ఒకవేళ (auto_వెడల్పు <= min_వెడల్పు) {
		/* not enough space: minimise column widths */
		కొరకు (i = 0; i < columns; i++) {
			నిలువు[i].వెడల్పు = నిలువు[i].min;
		}
		పట్టిక_వెడల్పు = min_వెడల్పు;
	} లేకపోతే ఒకవేళ (max_వెడల్పు <= auto_వెడల్పు) {
		/* more space than maximum width */
		ఒకవేళ (పట్టిక_వెడల్పు == AUTO) {
			/* for auto-width tables, make columns max width */
			కొరకు (i = 0; i < columns; i++) {
				నిలువు[i].వెడల్పు = నిలువు[i].max;
			}
			పట్టిక_వెడల్పు = max_వెడల్పు;
		} లేకపోతే {
			/* for fixed-width tables, distribute the extra space
			 * too */
			గుర్తులేని పూర్ణాంకం flexible_columns = 0;
			కొరకు (i = 0; i != columns; i++)
				ఒకవేళ (నిలువు[i].type != స్తంభం_వెడల్పు_FIXED)
					flexible_columns++;
			ఒకవేళ (flexible_columns == 0) {
				పూర్ణాంకం extra = (పట్టిక_వెడల్పు - max_వెడల్పు) / columns;
				remainder = (పట్టిక_వెడల్పు - max_వెడల్పు) -
						(extra * columns);
				కొరకు (i = 0; i != columns; i++) {
					నిలువు[i].వెడల్పు = నిలువు[i].max + extra;
					లెక్క -= remainder;
					ఒకవేళ (లెక్క < 0) {
						నిలువు[i].వెడల్పు++;
						లెక్క += columns;
					}
				}

			} లేకపోతే {
				పూర్ణాంకం extra = (పట్టిక_వెడల్పు - max_వెడల్పు) /
						flexible_columns;
				remainder = (పట్టిక_వెడల్పు - max_వెడల్పు) -
						(extra * flexible_columns);
				కొరకు (i = 0; i != columns; i++)
					ఒకవేళ (నిలువు[i].type != స్తంభం_వెడల్పు_FIXED) {
						నిలువు[i].వెడల్పు = నిలువు[i].max +
								extra;
						లెక్క -= remainder;
						ఒకవేళ (లెక్క < 0) {
							నిలువు[i].వెడల్పు++;
							లెక్క += flexible_columns;
						}
					}
			}
		}
	} లేకపోతే {
		/* space between min and max: fill it exactly */
		దశాంశం scale = (దశాంశం) (auto_వెడల్పు - min_వెడల్పు) /
				(దశాంశం) (max_వెడల్పు - min_వెడల్పు);
		/* fprintf(stderr, "filling, scale %f\n", scale); */
		కొరకు (i = 0; i < columns; i++) {
			నిలువు[i].వెడల్పు = నిలువు[i].min + (పూర్ణాంకం) (0.5 +
					(నిలువు[i].max - నిలువు[i].min) * scale);
		}
		పట్టిక_వెడల్పు = auto_వెడల్పు;
	}

	xs[0] = x = border_spacing_h;
	కొరకు (i = 0; i != columns; i++) {
		ఒకవేళ (!నిలువు[i].positioned)
			x += నిలువు[i].వెడల్పు + border_spacing_h;
		xs[i + 1] = x;
		అడ్డువరుస_పరిధి[i] = 0;
		excess_y[i] = 0;
		అడ్డువరుస_పరిధి_cell[i] = 0;
	}

	/* position cells */
	పట్టిక_ఎత్తు = border_spacing_v;
	కొరకు (అడ్డువరుస_సమూహం = పట్టిక->children; అడ్డువరుస_సమూహం;
			అడ్డువరుస_సమూహం = అడ్డువరుస_సమూహం->తదుపరి) {
		పూర్ణాంకం అడ్డువరుస_సమూహం_ఎత్తు = 0;
		కొరకు (అడ్డువరుస = అడ్డువరుస_సమూహం->children; అడ్డువరుస; అడ్డువరుస = అడ్డువరుస->తదుపరి) {
			పూర్ణాంకం అడ్డువరుస_ఎత్తు = 0;

			htype = css_computed_ఎత్తు(అడ్డువరుస->style, &విలువ, &unit);
			ఒకవేళ (htype == CSS_ఎత్తు_సమితి && unit != CSS_UNIT_PCT) {
				అడ్డువరుస_ఎత్తు = FIXTOINT(css_unit_len2device_px(
						అడ్డువరుస->style,
						&content->unit_len_ctx,
						విలువ, unit));
			}
			కొరకు (c = అడ్డువరుస->children; c; c = c->తదుపరి) {
				ప్రతిజ్ఞా(c->style);
				c->వెడల్పు = xs[c->ప్రారంభం_స్తంభం + c->columns] -
						xs[c->ప్రారంభం_స్తంభం] -
						border_spacing_h -
						c->border[ఎడమ_2].వెడల్పు -
						c->padding[ఎడమ_2] -
						c->padding[కుడి_2] -
						c->border[కుడి_2].వెడల్పు;
				c->float_children = 0;
				c->cached_ఉంచు_క్రింద_level = 0;

				c->ఎత్తు = AUTO;
				ఒకవేళ (!layout_ఖండం_context(c, -1, content)) {
					విడుదల(నిలువు);
					విడుదల(excess_y);
					విడుదల(అడ్డువరుస_పరిధి);
					విడుదల(అడ్డువరుస_పరిధి_cell);
					విడుదల(xs);
					తిరిగి false;
				}
				/* warning: c->descendant_y0 and
				 * c->descendant_y1 used as temporary storage
				 * until after vertical alignment is complete */
				c->descendant_y0 = c->ఎత్తు;
				c->descendant_y1 = c->padding[BOTTOM];

				htype = css_computed_ఎత్తు(c->style,
						&విలువ, &unit);

				ఒకవేళ (htype == CSS_ఎత్తు_సమితి &&
						unit != CSS_UNIT_PCT) {
					/* some sites use height="1" or similar
					 * to attempt to make cells as small as
					 * possible, so treat it as a minimum */
					పూర్ణాంకం h = FIXTOINT(css_unit_len2device_px(
							c->style,
							&content->unit_len_ctx,
							విలువ, unit));
					ఒకవేళ (c->ఎత్తు < h)
						c->ఎత్తు = h;
				}
				/* specified row height is treated as a minimum
				 */
				ఒకవేళ (c->ఎత్తు < అడ్డువరుస_ఎత్తు)
					c->ఎత్తు = అడ్డువరుస_ఎత్తు;
				c->x = xs[c->ప్రారంభం_స్తంభం] +
						c->border[ఎడమ_2].వెడల్పు;
				c->y = c->border[శిఖరం_2].వెడల్పు;
				కొరకు (i = 0; i != c->columns; i++) {
					అడ్డువరుస_పరిధి[c->ప్రారంభం_స్తంభం + i] = c->అడ్డువరుసలు;
					excess_y[c->ప్రారంభం_స్తంభం + i] =
							c->border[శిఖరం_2].వెడల్పు +
							c->padding[శిఖరం_2] +
							c->ఎత్తు +
							c->padding[BOTTOM] +
							c->border[BOTTOM].వెడల్పు;
					అడ్డువరుస_పరిధి_cell[c->ప్రారంభం_స్తంభం + i] = 0;
				}
				అడ్డువరుస_పరిధి_cell[c->ప్రారంభం_స్తంభం] = c;
				c->padding[BOTTOM] = -border_spacing_v -
						c->border[శిఖరం_2].వెడల్పు -
						c->padding[శిఖరం_2] -
						c->ఎత్తు -
						c->border[BOTTOM].వెడల్పు;
			}
			కొరకు (i = 0; i != columns; i++)
				ఒకవేళ (అడ్డువరుస_పరిధి[i] != 0)
					అడ్డువరుస_పరిధి[i]--;
				లేకపోతే
					అడ్డువరుస_పరిధి_cell[i] = 0;
			ఒకవేళ (అడ్డువరుస->తదుపరి || అడ్డువరుస_సమూహం->తదుపరి) {
				/* row height is greatest excess of a cell
				 * which ends in this row */
				కొరకు (i = 0; i != columns; i++)
					ఒకవేళ (అడ్డువరుస_పరిధి[i] == 0 && అడ్డువరుస_ఎత్తు <
							excess_y[i])
						అడ్డువరుస_ఎత్తు = excess_y[i];
			} లేకపోతే {
				/* except in the last row */
				కొరకు (i = 0; i != columns; i++)
					ఒకవేళ (అడ్డువరుస_ఎత్తు < excess_y[i])
						అడ్డువరుస_ఎత్తు = excess_y[i];
			}
			కొరకు (i = 0; i != columns; i++) {
				ఒకవేళ (అడ్డువరుస_ఎత్తు < excess_y[i])
					excess_y[i] -= అడ్డువరుస_ఎత్తు;
				లేకపోతే
					excess_y[i] = 0;
				ఒకవేళ (అడ్డువరుస_పరిధి_cell[i] != 0)
					అడ్డువరుస_పరిధి_cell[i]->padding[BOTTOM] +=
							అడ్డువరుస_ఎత్తు +
							border_spacing_v;
			}

			అడ్డువరుస->x = 0;
			అడ్డువరుస->y = అడ్డువరుస_సమూహం_ఎత్తు;
			అడ్డువరుస->వెడల్పు = పట్టిక_వెడల్పు;
			అడ్డువరుస->ఎత్తు = అడ్డువరుస_ఎత్తు;
			అడ్డువరుస_సమూహం_ఎత్తు += అడ్డువరుస_ఎత్తు + border_spacing_v;
		}
		అడ్డువరుస_సమూహం->x = 0;
		అడ్డువరుస_సమూహం->y = పట్టిక_ఎత్తు;
		అడ్డువరుస_సమూహం->వెడల్పు = పట్టిక_వెడల్పు;
		అడ్డువరుస_సమూహం->ఎత్తు = అడ్డువరుస_సమూహం_ఎత్తు;
		పట్టిక_ఎత్తు += అడ్డువరుస_సమూహం_ఎత్తు;
	}
	/* Table height is either the height of the contents, or specified
	 * height if greater */
	పట్టిక_ఎత్తు = max(పట్టిక_ఎత్తు, min_ఎత్తు);
	/** \todo distribute spare height over the row groups / rows / cells */

	/* perform vertical alignment */
	కొరకు (అడ్డువరుస_సమూహం = పట్టిక->children; అడ్డువరుస_సమూహం;
			అడ్డువరుస_సమూహం = అడ్డువరుస_సమూహం->తదుపరి) {
		కొరకు (అడ్డువరుస = అడ్డువరుస_సమూహం->children; అడ్డువరుస; అడ్డువరుస = అడ్డువరుస->తదుపరి) {
			కొరకు (c = అడ్డువరుస->children; c; c = c->తదుపరి) {
				గణన css_vertical_align_e vertical_align;

				/* unextended bottom padding is in
				 * c->descendant_y1, and unextended
				 * cell height is in c->descendant_y0 */
				అదనపు_ఎత్తు = (c->padding[BOTTOM] -
						c->descendant_y1) +
						(c->ఎత్తు - c->descendant_y0);

				vertical_align = css_computed_vertical_align(
						c->style, &విలువ, &unit);

				ఎంపిక (vertical_align) {
				స్థితి CSS_VERTICAL_ALIGN_SUB:
				స్థితి CSS_VERTICAL_ALIGN_SUPER:
				స్థితి CSS_VERTICAL_ALIGN_పాఠ్యం_శిఖరం:
				స్థితి CSS_VERTICAL_ALIGN_పాఠ్యం_BOTTOM:
				స్థితి CSS_VERTICAL_ALIGN_సమితి:
				స్థితి CSS_VERTICAL_ALIGN_BASELINE:
					/* todo: baseline alignment, for now
					 * just use ALIGN_TOP */
				స్థితి CSS_VERTICAL_ALIGN_శిఖరం:
					ఆపు;
				స్థితి CSS_VERTICAL_ALIGN_మధ్య:
					c->padding[శిఖరం_2] += అదనపు_ఎత్తు / 2;
					c->padding[BOTTOM] -= అదనపు_ఎత్తు / 2;
					layout_move_children(c, 0,
							అదనపు_ఎత్తు / 2);
					ఆపు;
				స్థితి CSS_VERTICAL_ALIGN_BOTTOM:
					c->padding[శిఖరం_2] += అదనపు_ఎత్తు;
					c->padding[BOTTOM] -= అదనపు_ఎత్తు;
					layout_move_children(c, 0,
							అదనపు_ఎత్తు);
					ఆపు;
				స్థితి CSS_VERTICAL_ALIGN_INHERIT:
					ప్రతిజ్ఞా(0);
					ఆపు;
				}
			}
		}
	}

	/* Top and bottom margins of 'auto' are set to 0.  CSS2.1 10.6.3 */
	ఒకవేళ (పట్టిక->margin[శిఖరం_2] == AUTO)
		పట్టిక->margin[శిఖరం_2] = 0;
	ఒకవేళ (పట్టిక->margin[BOTTOM] == AUTO)
		పట్టిక->margin[BOTTOM] = 0;

	విడుదల(నిలువు);
	విడుదల(excess_y);
	విడుదల(అడ్డువరుస_పరిధి);
	విడుదల(అడ్డువరుస_పరిధి_cell);
	విడుదల(xs);

	పట్టిక->వెడల్పు = పట్టిక_వెడల్పు;
	పట్టిక->ఎత్తు = పట్టిక_ఎత్తు;

	తిరిగి true;
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
స్థిర bool layout_apply_minmax_ఎత్తు(
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		నిర్మాణం box *box,
		నిర్మాణం box *container)
{
	పూర్ణాంకం h;
	నిర్మాణం box *containing_ఖండం = ఖాళీ;
	bool updated = false;

	/* Find containing block for percentage heights */
	ఒకవేళ (box->style != ఖాళీ && css_computed_position(box->style) ==
			CSS_POSITION_ABSOLUTE) {
		/* Box is absolutely positioned */
		ప్రతిజ్ఞా(container);
		containing_ఖండం = container;
	} లేకపోతే ఒకవేళ (box->float_container && box->style != ఖాళీ &&
			(css_computed_float(box->style) == CSS_FLOAT_ఎడమ ||
			 css_computed_float(box->style) == CSS_FLOAT_కుడి)) {
		/* Box is a float */
		ప్రతిజ్ఞా(box->మాతృ && box->మాతృ->మాతృ &&
				box->మాతృ->మాతృ->మాతృ);
		containing_ఖండం = box->మాతృ->మాతృ->మాతృ;
	} లేకపోతే ఒకవేళ (box->మాతృ && box->మాతృ->type != BOX_INLINE_CONTAINER) {
		/* Box is a block level element */
		containing_ఖండం = box->మాతృ;
	} లేకపోతే ఒకవేళ (box->మాతృ && box->మాతృ->type == BOX_INLINE_CONTAINER) {
		/* Box is an inline block */
		ప్రతిజ్ఞా(box->మాతృ->మాతృ);
		containing_ఖండం = box->మాతృ->మాతృ;
	}

	ఒకవేళ (box->style) {
		గణన css_ఎత్తు_e htype = CSS_ఎత్తు_AUTO;
		css_fixed విలువ = 0;
		css_unit unit = CSS_UNIT_PX;

		ఒకవేళ (containing_ఖండం) {
			htype = css_computed_ఎత్తు(containing_ఖండం->style,
					&విలువ, &unit);
		}

		/* max-height */
		ఒకవేళ (css_computed_max_ఎత్తు(box->style, &విలువ, &unit) ==
				CSS_MAX_ఎత్తు_సమితి) {
			ఒకవేళ (unit == CSS_UNIT_PCT) {
				ఒకవేళ (containing_ఖండం &&
					containing_ఖండం->ఎత్తు != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_ఎత్తు_సమితి)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_యొక్క_INT_TOINT(విలువ,
						containing_ఖండం->ఎత్తు);
					ఒకవేళ (h < box->ఎత్తు) {
						box->ఎత్తు = h;
						updated = true;
					}
				}
			} లేకపోతే {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						విలువ, unit));
				ఒకవేళ (h < box->ఎత్తు) {
					box->ఎత్తు = h;
					updated = true;
				}
			}
		}

		/* min-height */
		ఒకవేళ (ns_computed_min_ఎత్తు(box->style, &విలువ, &unit) ==
				CSS_MIN_ఎత్తు_సమితి) {
			ఒకవేళ (unit == CSS_UNIT_PCT) {
				ఒకవేళ (containing_ఖండం &&
					containing_ఖండం->ఎత్తు != AUTO &&
					(css_computed_position(box->style) ==
							CSS_POSITION_ABSOLUTE ||
						htype == CSS_ఎత్తు_సమితి)) {
					/* Box is absolutely positioned or its
					 * containing block has a valid
					 * specified height. (CSS 2.1
					 * Section 10.5) */
					h = FPCT_యొక్క_INT_TOINT(విలువ,
						containing_ఖండం->ఎత్తు);
					ఒకవేళ (h > box->ఎత్తు) {
						box->ఎత్తు = h;
						updated = true;
					}
				}
			} లేకపోతే {
				h = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						విలువ, unit));
				ఒకవేళ (h > box->ఎత్తు) {
					box->ఎత్తు = h;
					updated = true;
				}
			}
		}
	}
	తిరిగి updated;
}


/**
 * Layout a block which contains an object.
 *
 * \param  block  box of type BLOCK, INLINE_BLOCK, TABLE, or TABLE_CELL
 * \return  true on success, false on memory exhaustion
 */
స్థిర bool layout_ఖండం_object(నిర్మాణం box *ఖండం)
{
	ప్రతిజ్ఞా(ఖండం);
	ప్రతిజ్ఞా(ఖండం->type == BOX_ఖండం ||
			ఖండం->type == BOX_FLEX ||
			ఖండం->type == BOX_INLINE_ఖండం ||
			ఖండం->type == BOX_INLINE_FLEX ||
			ఖండం->type == BOX_పట్టిక ||
			ఖండం->type == BOX_పట్టిక_CELL);
	ప్రతిజ్ఞా(ఖండం->object);

	NSLOG(layout, DEBUG,  "block %p, object %p, width %i", ఖండం,
	      hlcache_handle_get_url(ఖండం->object), ఖండం->వెడల్పు);

	ఒకవేళ (content_can_reformat(ఖండం->object)) {
		content_reformat(ఖండం->object, false, ఖండం->వెడల్పు, 1);
	} లేకపోతే {
		/* Non-HTML objects */
		/* this case handled already in
		 * layout_block_find_dimensions() */
	}

	తిరిగి true;
}


/**
 * Insert a float into a container.
 *
 * \param  cont	  block formatting context block, used to contain float
 * \param  b      box to add to float
 *
 * This sorts floats in order of descending bottom edges.
 */
స్థిర శూన్య కూడు_float_గమ్యం_container(నిర్మాణం box *cont, నిర్మాణం box *b)
{
	నిర్మాణం box *box = cont->float_children;
	పూర్ణాంకం b_bottom = b->y + b->ఎత్తు;

	ప్రతిజ్ఞా(b->type == BOX_FLOAT_ఎడమ || b->type == BOX_FLOAT_కుడి);

	ఒకవేళ (box == ఖాళీ) {
		/* No other float children */
		b->తదుపరి_float = ఖాళీ;
		cont->float_children = b;
		తిరిగి;
	} లేకపోతే ఒకవేళ (b_bottom >= box->y + box->ఎత్తు) {
		/* Goes at start of list */
		b->తదుపరి_float = cont->float_children;
		cont->float_children = b;
	} లేకపోతే {
		నిర్మాణం box *prev = ఖాళీ;
		అయితే (box != ఖాళీ && b_bottom < box->y + box->ఎత్తు) {
			prev = box;
			box = box->తదుపరి_float;
		}
		ఒకవేళ (prev != ఖాళీ) {
			b->తదుపరి_float = prev->తదుపరి_float;
			prev->తదుపరి_float = b;
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
స్థిర bool
layout_పాఠ్యం_box_విభజించు(html_content *content,
		      plot_font_style_t *fstyle,
		      నిర్మాణం box *విభజించు_box,
		      size_t new_నిడివి,
		      పూర్ణాంకం new_వెడల్పు)
{
	పూర్ణాంకం space_వెడల్పు = విభజించు_box->space;
	నిర్మాణం box *c2;
	స్థిరరాశి నిర్మాణం gui_layout_పట్టిక *font_func = content->font_func;
	bool space = (విభజించు_box->పాఠ్యం[new_నిడివి] == ' ');
	పూర్ణాంకం వాడిన_నిడివి = new_నిడివి + (space ? 1 : 0);

	ఒకవేళ ((space && space_వెడల్పు == 0) || space_వెడల్పు == UNKNOWN_వెడల్పు) {
		/* We're need to add a space, and we don't know how big
		 * it's to be, OR we have a space of unknown width anyway;
		 * Calculate space width */
		font_func->వెడల్పు(fstyle, " ", 1, &space_వెడల్పు);
	}

	ఒకవేళ (విభజించు_box->space == UNKNOWN_వెడల్పు)
		విభజించు_box->space = space_వెడల్పు;
	ఒకవేళ (!space)
		space_వెడల్పు = 0;

	/* Create clone of split_box, c2 */
	c2 = talloc_memdup(content->bctx, విభజించు_box, పరిమాణం *c2);
	ఒకవేళ (!c2)
		తిరిగి false;
	c2->flags |= CLONE;

	/* Set remaining text in c2 */
	c2->పాఠ్యం += వాడిన_నిడివి;

	/* Set c2 according to the remaining text */
	c2->వెడల్పు -= new_వెడల్పు + space_వెడల్పు;
	c2->flags &= ~MEASURED; /* width has been estimated */
	c2->నిడివి = విభజించు_box->నిడివి - వాడిన_నిడివి;

	/* Update split_box for its reduced text */
	విభజించు_box->వెడల్పు = new_వెడల్పు;
	విభజించు_box->flags |= MEASURED;
	విభజించు_box->నిడివి = new_నిడివి;
	విభజించు_box->space = space_వెడల్పు;

	/* Insert c2 into box list */
	c2->తదుపరి = విభజించు_box->తదుపరి;
	విభజించు_box->తదుపరి = c2;
	c2->prev = విభజించు_box;
	ఒకవేళ (c2->తదుపరి)
		c2->తదుపరి->prev = c2;
	లేకపోతే
		c2->మాతృ->చివరి = c2;

	NSLOG(layout, DEBUG,
	      "split_box %p len: %" PRIsizet " \"%.*s\"",
	      విభజించు_box,
	      విభజించు_box->నిడివి,
	      (పూర్ణాంకం)విభజించు_box->నిడివి,
	      విభజించు_box->పాఠ్యం);
	NSLOG(layout, DEBUG,
	      "  new_box %p len: %" PRIsizet " \"%.*s\"",
	      c2,
	      c2->నిడివి,
	      (పూర్ణాంకం)c2->నిడివి,
	      c2->పాఠ్యం);

	తిరిగి true;
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
స్థిర శూన్య
layout_float_కనుగొను_dimensions(
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		పూర్ణాంకం అందుబాటు_వెడల్పు,
		స్థిరరాశి css_computed_style *style,
		నిర్మాణం box *box)
{
	పూర్ణాంకం వెడల్పు, ఎత్తు, max_వెడల్పు, min_వెడల్పు, max_ఎత్తు, min_ఎత్తు;
	పూర్ణాంకం *margin = box->margin;
	పూర్ణాంకం *padding = box->padding;
	నిర్మాణం box_border *border = box->border;
	గణన css_అధికప్రవాహం_e అధికప్రవాహం_x = css_computed_అధికప్రవాహం_x(style);
	గణన css_అధికప్రవాహం_e అధికప్రవాహం_y = css_computed_అధికప్రవాహం_y(style);
	పూర్ణాంకం scrollbar_వెడల్పు_x =
			(అధికప్రవాహం_x == CSS_అధికప్రవాహం_SCROLL ||
			 అధికప్రవాహం_x == CSS_అధికప్రవాహం_AUTO) ?
			SCROLLBAR_వెడల్పు : 0;
	పూర్ణాంకం scrollbar_వెడల్పు_y =
			(అధికప్రవాహం_y == CSS_అధికప్రవాహం_SCROLL ||
			 అధికప్రవాహం_y == CSS_అధికప్రవాహం_AUTO) ?
			SCROLLBAR_వెడల్పు : 0;

	layout_కనుగొను_dimensions(unit_len_ctx, అందుబాటు_వెడల్పు, -1, box, style,
			&వెడల్పు, &ఎత్తు, &max_వెడల్పు, &min_వెడల్పు,
			&max_ఎత్తు, &min_ఎత్తు, margin, padding, border);

	ఒకవేళ (margin[ఎడమ_2] == AUTO)
		margin[ఎడమ_2] = 0;
	ఒకవేళ (margin[కుడి_2] == AUTO)
		margin[కుడి_2] = 0;

	ఒకవేళ (box->gadget == ఖాళీ) {
		padding[కుడి_2] += scrollbar_వెడల్పు_y;
		padding[BOTTOM] += scrollbar_వెడల్పు_x;
	}

	ఒకవేళ (box->object && !(box->flags & REPLACE_DIM) &&
			content_get_type(box->object) != CONTENT_HTML) {
		/* Floating replaced element, with intrinsic width or height.
		 * See 10.3.6 and 10.6.2 */
		layout_get_object_dimensions(box, &వెడల్పు, &ఎత్తు,
				min_వెడల్పు, max_వెడల్పు, min_ఎత్తు, max_ఎత్తు);
	} లేకపోతే ఒకవేళ (box->gadget && (box->gadget->type == GADGET_TEXTBOX ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_FILE ||
			box->gadget->type == GADGET_TEXTAREA)) {
		css_fixed కొలత = 0;
		css_unit unit = CSS_UNIT_EM;

		/* Give sensible dimensions to gadgets, with auto width/height,
		 * that don't shrink to fit contained text. */
		ప్రతిజ్ఞా(box->style);

		ఒకవేళ (box->gadget->type == GADGET_TEXTBOX ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_FILE) {
			ఒకవేళ (వెడల్పు == AUTO) {
				కొలత = INTTOFIX(10);
				వెడల్పు = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						కొలత, unit));
			}
			ఒకవేళ (box->gadget->type == GADGET_FILE &&
					ఎత్తు == AUTO) {
				కొలత = FLTTOFIX(1.5);
				ఎత్తు = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						కొలత, unit));
			}
		}
		ఒకవేళ (box->gadget->type == GADGET_TEXTAREA) {
			ఒకవేళ (వెడల్పు == AUTO) {
				కొలత = INTTOFIX(10);
				వెడల్పు = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						కొలత, unit));
			}
			ఒకవేళ (ఎత్తు == AUTO) {
				కొలత = INTTOFIX(4);
				ఎత్తు = FIXTOINT(css_unit_len2device_px(
						box->style, unit_len_ctx,
						కొలత, unit));
			}
		}
	} లేకపోతే ఒకవేళ (వెడల్పు == AUTO) {
		/* CSS 2.1 section 10.3.5 */
		వెడల్పు = min(max(box->min_వెడల్పు, అందుబాటు_వెడల్పు),
				box->max_వెడల్పు);

		/* width includes margin, borders and padding */
		ఒకవేళ (వెడల్పు == అందుబాటు_వెడల్పు) {
			వెడల్పు -= box->margin[ఎడమ_2] + box->border[ఎడమ_2].వెడల్పు +
					box->padding[ఎడమ_2] +
					box->padding[కుడి_2] +
					box->border[కుడి_2].వెడల్పు +
					box->margin[కుడి_2];
		} లేకపోతే {
			/* width was obtained from a min_width or max_width
			 * value, so need to use the same method for calculating
			 * mbp as was used in layout_minmax_block() */
			పూర్ణాంకం fixed = 0;
			దశాంశం frac = 0;
			calculate_mbp_వెడల్పు(unit_len_ctx, box->style, ఎడమ_2,
					true, true, true, &fixed, &frac);
			calculate_mbp_వెడల్పు(unit_len_ctx, box->style, కుడి_2,
					true, true, true, &fixed, &frac);
			ఒకవేళ (fixed < 0)
				fixed = 0;

			వెడల్పు -= fixed;
		}

		ఒకవేళ (max_వెడల్పు >= 0 && వెడల్పు > max_వెడల్పు) వెడల్పు = max_వెడల్పు;
		ఒకవేళ (min_వెడల్పు >  0 && వెడల్పు < min_వెడల్పు) వెడల్పు = min_వెడల్పు;

	} లేకపోతే {
		ఒకవేళ (max_వెడల్పు >= 0 && వెడల్పు > max_వెడల్పు) వెడల్పు = max_వెడల్పు;
		ఒకవేళ (min_వెడల్పు >  0 && వెడల్పు < min_వెడల్పు) వెడల్పు = min_వెడల్పు;
		వెడల్పు -= scrollbar_వెడల్పు_y;
	}

	box->వెడల్పు = వెడల్పు;
	box->ఎత్తు = ఎత్తు;

	ఒకవేళ (margin[శిఖరం_2] == AUTO)
		margin[శిఖరం_2] = 0;
	ఒకవేళ (margin[BOTTOM] == AUTO)
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
స్థిర bool layout_float(నిర్మాణం box *b, పూర్ణాంకం వెడల్పు, html_content *content)
{
	ప్రతిజ్ఞా(b->type == BOX_పట్టిక ||
	       b->type == BOX_ఖండం ||
	       b->type == BOX_INLINE_ఖండం ||
	       b->type == BOX_FLEX ||
	       b->type == BOX_INLINE_FLEX);
	layout_float_కనుగొను_dimensions(&content->unit_len_ctx, వెడల్పు, b->style, b);
	ఒకవేళ (b->type == BOX_పట్టిక || b->type == BOX_INLINE_FLEX) {
		ఒకవేళ (b->type == BOX_పట్టిక) {
			ఒకవేళ (!layout_పట్టిక(b, వెడల్పు, content))
				తిరిగి false;
		} లేకపోతే {
			ఒకవేళ (!layout_flex(b, వెడల్పు, content))
				తిరిగి false;
		}
		ఒకవేళ (b->margin[ఎడమ_2] == AUTO)
			b->margin[ఎడమ_2] = 0;
		ఒకవేళ (b->margin[కుడి_2] == AUTO)
			b->margin[కుడి_2] = 0;
		ఒకవేళ (b->margin[శిఖరం_2] == AUTO)
			b->margin[శిఖరం_2] = 0;
		ఒకవేళ (b->margin[BOTTOM] == AUTO)
			b->margin[BOTTOM] = 0;
	} లేకపోతే {
		తిరిగి layout_ఖండం_context(b, -1, content);
	}
	తిరిగి true;
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
స్థిర శూన్య
ఉంచు_float_క్రింద(నిర్మాణం box *c, పూర్ణాంకం వెడల్పు, పూర్ణాంకం cx, పూర్ణాంకం y, నిర్మాణం box *cont)
{
	పూర్ణాంకం x0, x1, yy;
	నిర్మాణం box *ఎడమ;
	నిర్మాణం box *కుడి;

	yy = y > cont->cached_ఉంచు_క్రింద_level ?
			y : cont->cached_ఉంచు_క్రింద_level;

	NSLOG(layout, DEBUG,
	      "c %p, width %i, cx %i, y %i, cont %p", c,
	      వెడల్పు, cx, y, cont);

	చేయి {
		y = yy;
		x0 = cx;
		x1 = cx + వెడల్పు;
		కనుగొను_sides(cont->float_children, y, y + c->ఎత్తు, &x0, &x1,
				&ఎడమ, &కుడి);
		ఒకవేళ (ఎడమ != 0 && కుడి != 0) {
			yy = (ఎడమ->y + ఎడమ->ఎత్తు <
					కుడి->y + కుడి->ఎత్తు ?
					ఎడమ->y + ఎడమ->ఎత్తు :
					కుడి->y + కుడి->ఎత్తు);
		} లేకపోతే ఒకవేళ (ఎడమ == 0 && కుడి != 0) {
			yy = కుడి->y + కుడి->ఎత్తు;
		} లేకపోతే ఒకవేళ (ఎడమ != 0 && కుడి == 0) {
			yy = ఎడమ->y + ఎడమ->ఎత్తు;
		}
	} అయితే ((ఎడమ != 0 || కుడి != 0) && (c->వెడల్పు > x1 - x0));

	ఒకవేళ (c->type == BOX_FLOAT_ఎడమ) {
		c->x = x0;
	} లేకపోతే {
		c->x = x1 - c->వెడల్పు;
	}
	c->y = y;
	cont->cached_ఉంచు_క్రింద_level = y;
}


/**
 * Calculate line height from a style.
 */
స్థిర పూర్ణాంకం పంక్తి_ఎత్తు(
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		స్థిరరాశి css_computed_style *style)
{
	గణన css_పంక్తి_ఎత్తు_e lhtype;
	css_fixed lhvalue = 0;
	css_unit lhunit = CSS_UNIT_PX;
	css_fixed పంక్తి_ఎత్తు;

	ప్రతిజ్ఞా(style);

	lhtype = css_computed_పంక్తి_ఎత్తు(style, &lhvalue, &lhunit);
	ఒకవేళ (lhtype == CSS_పంక్తి_ఎత్తు_సాధారణ) {
		/* Normal => use a constant of 1.3 * font-size */
		lhvalue = FLTTOFIX(1.3);
		lhtype = CSS_పంక్తి_ఎత్తు_సంఖ్య;
	}

	ఒకవేళ (lhtype == CSS_పంక్తి_ఎత్తు_సంఖ్య ||
			lhunit == CSS_UNIT_PCT) {
		పంక్తి_ఎత్తు = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, CSS_UNIT_EM);

		ఒకవేళ (lhtype != CSS_పంక్తి_ఎత్తు_సంఖ్య)
			పంక్తి_ఎత్తు = FDIV(పంక్తి_ఎత్తు, F_100);
	} లేకపోతే {
		ప్రతిజ్ఞా(lhunit != CSS_UNIT_PCT);

		పంక్తి_ఎత్తు = css_unit_len2device_px(style, unit_len_ctx,
				lhvalue, lhunit);
	}

	తిరిగి FIXTOINT(పంక్తి_ఎత్తు);
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
స్థిర bool
layout_పంక్తి(నిర్మాణం box *మొదటి,
	    పూర్ణాంకం *వెడల్పు,
	    పూర్ణాంకం *y,
	    పూర్ణాంకం cx,
	    పూర్ణాంకం cy,
	    నిర్మాణం box *cont,
	    bool indent,
	    bool has_పాఠ్యం_children,
	    html_content *content,
	    నిర్మాణం box **తదుపరి_box)
{
	పూర్ణాంకం ఎత్తు, వాడిన_ఎత్తు;
	పూర్ణాంకం x0 = 0;
	పూర్ణాంకం x1 = *వెడల్పు;
	పూర్ణాంకం x, h, x_మునుపటి;
	పూర్ణాంకం fy = cy;
	నిర్మాణం box *ఎడమ;
	నిర్మాణం box *కుడి;
	నిర్మాణం box *b;
	నిర్మాణం box *విభజించు_box = 0;
	నిర్మాణం box *d;
	నిర్మాణం box *br_box = 0;
	bool move_y = false;
	bool ఉంచు_క్రింద = false;
	పూర్ణాంకం space_before = 0, space_after = 0;
	గుర్తులేని పూర్ణాంకం inline_లెక్క = 0;
	గుర్తులేని పూర్ణాంకం i;
	స్థిరరాశి నిర్మాణం gui_layout_పట్టిక *font_func = content->font_func;
	plot_font_style_t fstyle;

	NSLOG(layout, DEBUG,
	      "first %p, first->text '%.*s', width %i, y %i, cx %i, cy %i",
	      మొదటి,
	      (పూర్ణాంకం)మొదటి->నిడివి,
	      మొదటి->పాఠ్యం,
	      *వెడల్పు,
	      *y,
	      cx,
	      cy);

	/* find sides at top of line */
	x0 += cx;
	x1 += cx;
	కనుగొను_sides(cont->float_children, cy, cy, &x0, &x1, &ఎడమ, &కుడి);
	x0 -= cx;
	x1 -= cx;

	ఒకవేళ (indent)
		x0 += layout_పాఠ్యం_indent(&content->unit_len_ctx,
				మొదటి->మాతృ->మాతృ->style, *వెడల్పు);

	ఒకవేళ (x1 < x0)
		x1 = x0;

	/* get minimum line height from containing block.
	 * this is the line-height if there are text children and also in the
	 * case of an initially empty text input */
	ఒకవేళ (has_పాఠ్యం_children || మొదటి->మాతృ->మాతృ->gadget)
		వాడిన_ఎత్తు = ఎత్తు = పంక్తి_ఎత్తు(&content->unit_len_ctx,
				మొదటి->మాతృ->మాతృ->style);
	లేకపోతే
		/* inline containers with no text are usually for layout and
		 * look better with no minimum line-height */
		వాడిన_ఎత్తు = ఎత్తు = 0;

	/* pass 1: find height of line assuming sides at top of line: loop
	 * body executed at least once
	 * keep in sync with the loop in layout_minmax_line() */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);


	కొరకు (x = 0, b = మొదటి; x <= x1 - x0 && b != 0; b = b->తదుపరి) {
		పూర్ణాంకం min_వెడల్పు, max_వెడల్పు, min_ఎత్తు, max_ఎత్తు;

		ప్రతిజ్ఞా(lh__box_is_inline_content(b));

		NSLOG(layout, DEBUG,  "pass 1: b %p, x %i", b, x);

		ఒకవేళ (b->type == BOX_BR)
			ఆపు;

		ఒకవేళ (lh__box_is_float_box(b))
			కొనసాగు;
		ఒకవేళ (b->type == BOX_INLINE_ఖండం &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED))
			కొనసాగు;

		ప్రతిజ్ఞా(b->style != ఖాళీ);
		font_plot_style_from_css(&content->unit_len_ctx, b->style, &fstyle);

		x += space_after;

		ఒకవేళ (b->type == BOX_INLINE_ఖండం ||
		    b->type == BOX_INLINE_FLEX) {
			ఒకవేళ (b->max_వెడల్పు != UNKNOWN_వెడల్పు)
				ఒకవేళ (!layout_float(b, *వెడల్పు, content))
					తిరిగి false;
			h = b->border[శిఖరం_2].వెడల్పు + b->padding[శిఖరం_2] + b->ఎత్తు +
					b->padding[BOTTOM] +
					b->border[BOTTOM].వెడల్పు;
			ఒకవేళ (ఎత్తు < h)
				ఎత్తు = h;
			x += b->margin[ఎడమ_2] + b->border[ఎడమ_2].వెడల్పు +
					b->padding[ఎడమ_2] + b->వెడల్పు +
					b->padding[కుడి_2] +
					b->border[కుడి_2].వెడల్పు +
					b->margin[కుడి_2];
			space_after = 0;
			కొనసాగు;
		}

		ఒకవేళ (b->type == BOX_INLINE) {
			/* calculate borders, margins, and padding */
			layout_కనుగొను_dimensions(&content->unit_len_ctx,
					*వెడల్పు, -1, b, b->style, 0, 0, 0, 0,
					0, 0, b->margin, b->padding, b->border);
			కొరకు (i = 0; i != 4; i++)
				ఒకవేళ (b->margin[i] == AUTO)
					b->margin[i] = 0;
			x += b->margin[ఎడమ_2] + b->border[ఎడమ_2].వెడల్పు +
					b->padding[ఎడమ_2];
			ఒకవేళ (b->inline_చివర) {
				b->inline_చివర->margin[కుడి_2] = b->margin[కుడి_2];
				b->inline_చివర->padding[కుడి_2] =
						b->padding[కుడి_2];
				b->inline_చివర->border[కుడి_2] =
						b->border[కుడి_2];
			} లేకపోతే {
				x += b->padding[కుడి_2] +
						b->border[కుడి_2].వెడల్పు +
						b->margin[కుడి_2];
			}
		} లేకపోతే ఒకవేళ (b->type == BOX_INLINE_చివర) {
			b->వెడల్పు = 0;
			ఒకవేళ (b->space == UNKNOWN_వెడల్పు) {
				font_func->వెడల్పు(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;

			x += b->padding[కుడి_2] + b->border[కుడి_2].వెడల్పు +
					b->margin[కుడి_2];
			కొనసాగు;
		}

		ఒకవేళ (lh__box_is_replace(b) == false) {
			/* inline non-replaced, 10.3.1 and 10.6.1 */
			b->ఎత్తు = పంక్తి_ఎత్తు(&content->unit_len_ctx,
					b->style ? b->style :
					b->మాతృ->మాతృ->style);
			ఒకవేళ (ఎత్తు < b->ఎత్తు)
				ఎత్తు = b->ఎత్తు;

			ఒకవేళ (!b->పాఠ్యం) {
				b->వెడల్పు = 0;
				space_after = 0;
				కొనసాగు;
			}

			ఒకవేళ (b->వెడల్పు == UNKNOWN_వెడల్పు) {
				/** \todo handle errors */

				/* If it's a select element, we must use the
				 * width of the widest option text */
				ఒకవేళ (b->మాతృ->మాతృ->gadget &&
						b->మాతృ->మాతృ->gadget->type
						== GADGET_SELECT) {
					పూర్ణాంకం opt_maxwidth = 0;
					నిర్మాణం form_option *o;

					కొరకు (o = b->మాతృ->మాతృ->gadget->
							దత్తాంశం.select.వస్తువులు; o;
							o = o->తదుపరి) {
						పూర్ణాంకం opt_వెడల్పు;
						font_func->వెడల్పు(&fstyle,
								o->పాఠ్యం,
								అక్షరశ్రేణి_పొడవు(o->పాఠ్యం),
								&opt_వెడల్పు);

						ఒకవేళ (opt_maxwidth < opt_వెడల్పు)
							opt_maxwidth =opt_వెడల్పు;
					}
					b->వెడల్పు = opt_maxwidth;
					ఒకవేళ (nsoption_bool(core_select_menu))
						b->వెడల్పు += SCROLLBAR_వెడల్పు;
				} లేకపోతే {
					font_func->వెడల్పు(&fstyle, b->పాఠ్యం,
							b->నిడివి, &b->వెడల్పు);
					b->flags |= MEASURED;
				}
			}

			/* If the current text has not been measured (i.e. its
			 * width was estimated after splitting), and it fits on
			 * the line, measure it properly, so next box is placed
			 * correctly. */
			ఒకవేళ (b->పాఠ్యం && (x + b->వెడల్పు < x1 - x0) &&
					!(b->flags & MEASURED) &&
					b->తదుపరి) {
				font_func->వెడల్పు(&fstyle, b->పాఠ్యం,
						 b->నిడివి, &b->వెడల్పు);
				b->flags |= MEASURED;
			}

			x += b->వెడల్పు;
			ఒకవేళ (b->space == UNKNOWN_వెడల్పు) {
				font_func->వెడల్పు(&fstyle, " ", 1, &b->space);
				/** \todo handle errors */
			}
			space_after = b->space;
			కొనసాగు;
		}

		space_after = 0;

		/* inline replaced, 10.3.2 and 10.6.2 */
		ప్రతిజ్ఞా(b->style);

		layout_కనుగొను_dimensions(&content->unit_len_ctx,
				*వెడల్పు, -1, b, b->style,
				&b->వెడల్పు, &b->ఎత్తు,
				&max_వెడల్పు, &min_వెడల్పు,
				&max_ఎత్తు, &min_ఎత్తు,
				ఖాళీ, ఖాళీ, ఖాళీ);

		ఒకవేళ (b->object && !(b->flags & REPLACE_DIM)) {
			layout_get_object_dimensions(b, &b->వెడల్పు, &b->ఎత్తు,
					min_వెడల్పు, max_వెడల్పు,
					min_ఎత్తు, max_ఎత్తు);
		} లేకపోతే ఒకవేళ (b->flags & IFRAME) {
			/* TODO: should we look at the content dimensions? */
			ఒకవేళ (b->వెడల్పు == AUTO)
				b->వెడల్పు = 400;
			ఒకవేళ (b->ఎత్తు == AUTO)
				b->ఎత్తు = 300;

			/* We reformat the iframe browser window to new
			 * dimensions in pass 2 */
		} లేకపోతే {
			/* form control with no object */
			ఒకవేళ (b->వెడల్పు == AUTO)
				b->వెడల్పు = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
			ఒకవేళ (b->ఎత్తు == AUTO)
				b->ఎత్తు = FIXTOINT(css_unit_len2device_px(
						b->style,
						&content->unit_len_ctx, INTTOFIX(1),
						CSS_UNIT_EM));
		}

		/* Reformat object to new box size */
		ఒకవేళ (b->object && content_can_reformat(b->object) &&
				b->వెడల్పు !=
				content_get_అందుబాటు_వెడల్పు(b->object)) {
			css_fixed విలువ = 0;
			css_unit unit = CSS_UNIT_PX;
			గణన css_ఎత్తు_e htype = css_computed_ఎత్తు(b->style,
					&విలువ, &unit);

			content_reformat(b->object, false, b->వెడల్పు, b->ఎత్తు);

			ఒకవేళ (htype == CSS_ఎత్తు_AUTO)
				b->ఎత్తు = content_get_ఎత్తు(b->object);
		}

		ఒకవేళ (ఎత్తు < b->ఎత్తు)
			ఎత్తు = b->ఎత్తు;

		x += b->వెడల్పు;
	}

	/* find new sides using this height */
	x0 = cx;
	x1 = cx + *వెడల్పు;
	కనుగొను_sides(cont->float_children, cy, cy + ఎత్తు, &x0, &x1,
			&ఎడమ, &కుడి);
	x0 -= cx;
	x1 -= cx;

	ఒకవేళ (indent)
		x0 += layout_పాఠ్యం_indent(&content->unit_len_ctx,
				మొదటి->మాతృ->మాతృ->style, *వెడల్పు);

	ఒకవేళ (x1 < x0)
		x1 = x0;

	space_after = space_before = 0;

	/* pass 2: place boxes in line: loop body executed at least once */

	NSLOG(layout, DEBUG,  "x0 %i, x1 %i, x1 - x0 %i", x0, x1, x1 - x0);

	కొరకు (x = x_మునుపటి = 0, b = మొదటి; x <= x1 - x0 && b; b = b->తదుపరి) {

		NSLOG(layout, DEBUG,  "pass 2: b %p, x %i", b, x);

		ఒకవేళ (b->type == BOX_INLINE_ఖండం &&
				(css_computed_position(b->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(b->style) ==
						CSS_POSITION_FIXED)) {
			b->x = x + space_after;

		} లేకపోతే ఒకవేళ (lh__box_is_inline_flow(b)) {
			ప్రతిజ్ఞా(b->వెడల్పు != UNKNOWN_వెడల్పు);

			x_మునుపటి = x;
			x += space_after;
			b->x = x;

			ఒకవేళ ((b->type == BOX_INLINE && !b->inline_చివర) ||
					b->type == BOX_INLINE_ఖండం ||
					b->type == BOX_INLINE_FLEX) {
				b->x += b->margin[ఎడమ_2] + b->border[ఎడమ_2].వెడల్పు;
				x = b->x + b->padding[ఎడమ_2] + b->వెడల్పు +
						b->padding[కుడి_2] +
						b->border[కుడి_2].వెడల్పు +
						b->margin[కుడి_2];
			} లేకపోతే ఒకవేళ (b->type == BOX_INLINE) {
				b->x += b->margin[ఎడమ_2] + b->border[ఎడమ_2].వెడల్పు;
				x = b->x + b->padding[ఎడమ_2] + b->వెడల్పు;
			} లేకపోతే ఒకవేళ (b->type == BOX_INLINE_చివర) {
				b->ఎత్తు = b->inline_చివర->ఎత్తు;
				x += b->padding[కుడి_2] +
						b->border[కుడి_2].వెడల్పు +
						b->margin[కుడి_2];
			} లేకపోతే {
				x += b->వెడల్పు;
			}

			space_before = space_after;
			ఒకవేళ (b->object || b->flags & REPLACE_DIM ||
					b->flags & IFRAME)
				space_after = 0;
			లేకపోతే ఒకవేళ (b->పాఠ్యం || b->type == BOX_INLINE_చివర) {
				ఒకవేళ (b->space == UNKNOWN_వెడల్పు) {
					font_plot_style_from_css(
							&content->unit_len_ctx,
							b->style, &fstyle);
					/** \todo handle errors */
					font_func->వెడల్పు(&fstyle, " ", 1,
							 &b->space);
				}
				space_after = b->space;
			} లేకపోతే {
				space_after = 0;
			}
			విభజించు_box = b;
			move_y = true;
			inline_లెక్క++;
		} లేకపోతే ఒకవేళ (b->type == BOX_BR) {
			b->x = x;
			b->వెడల్పు = 0;
			br_box = b;
			b = b->తదుపరి;
			విభజించు_box = 0;
			move_y = true;
			ఆపు;

		} లేకపోతే {
			/* float */
			NSLOG(layout, DEBUG,  "float %p", b);

			d = b->children;
			d->float_children = 0;
			d->cached_ఉంచు_క్రింద_level = 0;
			b->float_container = d->float_container = cont;

			ఒకవేళ (!layout_float(d, *వెడల్పు, content))
				తిరిగి false;

			NSLOG(layout, DEBUG,
			      "%p : %d %d",
			      d,
			      d->margin[శిఖరం_2],
			      d->border[శిఖరం_2].వెడల్పు);

			d->x = d->margin[ఎడమ_2] + d->border[ఎడమ_2].వెడల్పు;
			d->y = d->margin[శిఖరం_2] + d->border[శిఖరం_2].వెడల్పు;
			b->వెడల్పు = d->margin[ఎడమ_2] + d->border[ఎడమ_2].వెడల్పు +
					d->padding[ఎడమ_2] + d->వెడల్పు +
					d->padding[కుడి_2] +
					d->border[కుడి_2].వెడల్పు +
					d->margin[కుడి_2];
			b->ఎత్తు = d->margin[శిఖరం_2] + d->border[శిఖరం_2].వెడల్పు +
					d->padding[శిఖరం_2] + d->ఎత్తు +
					d->padding[BOTTOM] +
					d->border[BOTTOM].వెడల్పు +
					d->margin[BOTTOM];

			ఒకవేళ (b->వెడల్పు > (x1 - x0) - x)
				ఉంచు_క్రింద = true;
			ఒకవేళ (d->style && (css_computed_clear(d->style) ==
						CSS_CLEAR_NONE ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_ఎడమ && ఎడమ == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_కుడి &&
						కుడి == 0) ||
					(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						ఎడమ == 0 && కుడి == 0)) &&
					(!ఉంచు_క్రింద ||
					(ఎడమ == 0 && కుడి == 0 && x == 0)) &&
					cy >= cont->clear_level &&
					cy >= cont->cached_ఉంచు_క్రింద_level) {
				/* + not cleared or,
				 *   cleared and there are no floats to clear
				 * + fits without needing to be placed below or,
				 *   this line is empty with no floats
				 * + current y, cy, is below the clear level
				 *
				 * Float affects current line */
				ఒకవేళ (b->type == BOX_FLOAT_ఎడమ) {
					b->x = cx + x0;
					ఒకవేళ (b->వెడల్పు > 0)
						x0 += b->వెడల్పు;
					ఎడమ = b;
				} లేకపోతే {
					b->x = cx + x1 - b->వెడల్పు;
					ఒకవేళ (b->వెడల్పు > 0)
						x1 -= b->వెడల్పు;
					కుడి = b;
				}
				b->y = cy;
			} లేకపోతే {
				/* cleared or doesn't fit on line */
				/* place below into next available space */
				పూర్ణాంకం fcy = (cy > cont->clear_level) ? cy :
						cont->clear_level;
				fcy = (fcy > cont->cached_ఉంచు_క్రింద_level) ?
						fcy :
						cont->cached_ఉంచు_క్రింద_level;
				fy = (fy > fcy) ? fy : fcy;
				fy = (fy == cy) ? fy + ఎత్తు : fy;

				ఉంచు_float_క్రింద(b, *వెడల్పు, cx, fy, cont);
				fy = b->y;
				ఒకవేళ (d->style && (
						(css_computed_clear(d->style) ==
						CSS_CLEAR_ఎడమ && ఎడమ != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_కుడి &&
						కుడి != 0) ||
						(css_computed_clear(d->style) ==
						CSS_CLEAR_BOTH &&
						(ఎడమ != 0 || కుడి != 0)))) {
					/* to be cleared below existing
					 * floats */
					ఒకవేళ (b->type == BOX_FLOAT_ఎడమ)
						b->x = cx;
					లేకపోతే
						b->x = cx + *వెడల్పు - b->వెడల్పు;

					fcy = layout_clear(cont->float_children,
						css_computed_clear(d->style));
					ఒకవేళ (fcy > cont->clear_level)
						cont->clear_level = fcy;
					ఒకవేళ (b->y < fcy)
						b->y = fcy;
				}
				ఒకవేళ (b->type == BOX_FLOAT_ఎడమ)
					ఎడమ = b;
				లేకపోతే
					కుడి = b;
			}
			కూడు_float_గమ్యం_container(cont, b);

			విభజించు_box = 0;
		}
	}

	ఒకవేళ (x1 - x0 < x && విభజించు_box) {
		/* the last box went over the end */
		size_t విభజించు = 0;
		పూర్ణాంకం w;
		bool no_wrap = css_computed_white_space(
				విభజించు_box->style) == CSS_WHITE_SPACE_NOWRAP ||
				css_computed_white_space(
				విభజించు_box->style) == CSS_WHITE_SPACE_PRE;

		x = x_మునుపటి;

		ఒకవేళ (!no_wrap &&
		    (విభజించు_box->type == BOX_INLINE ||
		     విభజించు_box->type == BOX_పాఠ్యం) &&
		    !విభజించు_box->object &&
		    !(విభజించు_box->flags & REPLACE_DIM) &&
		    !(విభజించు_box->flags & IFRAME) &&
		    !విభజించు_box->gadget && విభజించు_box->పాఠ్యం) {

			font_plot_style_from_css(&content->unit_len_ctx,
					విభజించు_box->style, &fstyle);
			/** \todo handle errors */
			font_func->విభజించు(&fstyle,
					 విభజించు_box->పాఠ్యం,
					 విభజించు_box->నిడివి,
					 x1 - x0 - x - space_before,
					 &విభజించు,
					 &w);
		}

		/* split == 0 implies that text can't be split */

		ఒకవేళ (విభజించు == 0)
			w = విభజించు_box->వెడల్పు;


		NSLOG(layout, DEBUG,
		      "splitting: split_box %p \"%.*s\", spilt %"PRIsizet
		      ", w %i, left %p, right %p, inline_count %u",
		      విభజించు_box,
		      (పూర్ణాంకం)విభజించు_box->నిడివి,
		      విభజించు_box->పాఠ్యం,
		      విభజించు,
		      w,
		      ఎడమ,
		      కుడి,
		      inline_లెక్క);

		ఒకవేళ ((విభజించు == 0 || x1 - x0 <= x + space_before + w) &&
				!ఎడమ && !కుడి && inline_లెక్క == 1) {
			/* first word of box doesn't fit, but no floats and
			 * first box on line so force in */
			ఒకవేళ (విభజించు == 0 || విభజించు == విభజించు_box->నిడివి) {
				/* only one word in this box, or not text
				 * or white-space:nowrap */
				b = విభజించు_box->తదుపరి;
			} లేకపోతే {
				/* cut off first word for this line */
				ఒకవేళ (!layout_పాఠ్యం_box_విభజించు(content, &fstyle,
						విభజించు_box, విభజించు, w))
					తిరిగి false;
				b = విభజించు_box->తదుపరి;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "forcing");

		} లేకపోతే ఒకవేళ ((విభజించు == 0 || x1 - x0 <= x + space_before + w) &&
				inline_లెక్క == 1) {
			/* first word of first box doesn't fit, but a float is
			 * taking some of the width so move below it */
			ప్రతిజ్ఞా(ఎడమ || కుడి);
			వాడిన_ఎత్తు = 0;
			ఒకవేళ (ఎడమ) {

				NSLOG(layout, DEBUG,
				      "cy %i, left->y %i, left->height %i",
				      cy,
				      ఎడమ->y,
				      ఎడమ->ఎత్తు);

				వాడిన_ఎత్తు = ఎడమ->y + ఎడమ->ఎత్తు - cy + 1;

				NSLOG(layout, DEBUG,  "used_height %i",
				      వాడిన_ఎత్తు);

			}
			ఒకవేళ (కుడి && వాడిన_ఎత్తు <
					కుడి->y + కుడి->ఎత్తు - cy + 1)
				వాడిన_ఎత్తు = కుడి->y + కుడి->ఎత్తు - cy + 1;

			ఒకవేళ (వాడిన_ఎత్తు < 0)
				వాడిన_ఎత్తు = 0;

			b = విభజించు_box;

			NSLOG(layout, DEBUG,  "moving below float");

		} లేకపోతే ఒకవేళ (విభజించు == 0 || x1 - x0 <= x + space_before + w) {
			/* first word of box doesn't fit so leave box for next
			 * line */
			b = విభజించు_box;

			NSLOG(layout, DEBUG,  "leaving for next line");

		} లేకపోతే {
			/* fit as many words as possible */
			ప్రతిజ్ఞా(విభజించు != 0);

			NSLOG(layout, DEBUG,
			      "'%.*s' %i %"PRIsizet" %i",
			      (పూర్ణాంకం)విభజించు_box->నిడివి, విభజించు_box->పాఠ్యం,
			      x1 - x0, విభజించు, w);

			ఒకవేళ (విభజించు != విభజించు_box->నిడివి) {
				ఒకవేళ (!layout_పాఠ్యం_box_విభజించు(content, &fstyle,
						విభజించు_box, విభజించు, w))
					తిరిగి false;
				b = విభజించు_box->తదుపరి;
			}
			x += space_before + w;

			NSLOG(layout, DEBUG,  "fitting words");

		}
		move_y = true;
	}

	/* set positions */
	ఎంపిక (css_computed_పాఠ్యం_align(మొదటి->మాతృ->మాతృ->style)) {
	స్థితి CSS_పాఠ్యం_ALIGN_కుడి:
	స్థితి CSS_పాఠ్యం_ALIGN_LIBCSS_కుడి:
		x0 = x1 - x;
		ఆపు;
	స్థితి CSS_పాఠ్యం_ALIGN_CENTER:
	స్థితి CSS_పాఠ్యం_ALIGN_LIBCSS_CENTER:
		x0 = (x0 + (x1 - x)) / 2;
		ఆపు;
	స్థితి CSS_పాఠ్యం_ALIGN_ఎడమ:
	స్థితి CSS_పాఠ్యం_ALIGN_LIBCSS_ఎడమ:
	స్థితి CSS_పాఠ్యం_ALIGN_JUSTIFY:
		/* leave on left */
		ఆపు;
	స్థితి CSS_పాఠ్యం_ALIGN_DEFAULT:
		/* None; consider text direction */
		ఎంపిక (css_computed_direction(మొదటి->మాతృ->మాతృ->style)) {
		స్థితి CSS_DIRECTION_LTR:
			/* leave on left */
			ఆపు;
		స్థితి CSS_DIRECTION_RTL:
			x0 = x1 - x;
			ఆపు;
		}
		ఆపు;
	}

	కొరకు (d = మొదటి; d != b; d = d->తదుపరి) {
		d->flags &= ~NEW_పంక్తి;

		ఒకవేళ (d->type == BOX_INLINE_ఖండం &&
				(css_computed_position(d->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(d->style) ==
						CSS_POSITION_FIXED)) {
			/* positioned inline-blocks:
			 * set static position (x,y) only, rest of positioning
			 * is handled later */
			d->x += x0;
			d->y = *y;
			కొనసాగు;
		} లేకపోతే ఒకవేళ ((d->type == BOX_INLINE &&
				lh__box_is_replace(d) == false) ||
				d->type == BOX_BR ||
				d->type == BOX_పాఠ్యం ||
				d->type == BOX_INLINE_చివర) {
			/* regular (non-replaced) inlines */
			d->x += x0;
			d->y = *y - d->padding[శిఖరం_2];

			ఒకవేళ (d->type == BOX_పాఠ్యం && d->ఎత్తు > వాడిన_ఎత్తు) {
				/*పాఠ్యం*/
				వాడిన_ఎత్తు = d->ఎత్తు;
			}
		} లేకపోతే ఒకవేళ ((d->type == BOX_INLINE) ||
				d->type == BOX_INLINE_ఖండం) {
			/* replaced inlines and inline-blocks */
			d->x += x0;
			d->y = *y + d->border[శిఖరం_2].వెడల్పు + d->margin[శిఖరం_2];
			h = d->margin[శిఖరం_2] + d->border[శిఖరం_2].వెడల్పు +
					d->padding[శిఖరం_2] + d->ఎత్తు +
					d->padding[BOTTOM] +
					d->border[BOTTOM].వెడల్పు +
					d->margin[BOTTOM];
			ఒకవేళ (వాడిన_ఎత్తు < h)
				వాడిన_ఎత్తు = h;
		}
	}

	మొదటి->flags |= NEW_పంక్తి;

	ప్రతిజ్ఞా(b != మొదటి || (move_y && 0 < వాడిన_ఎత్తు && (ఎడమ || కుడి)));

	/* handle vertical-align by adjusting box y values */
	/** \todo  proper vertical alignment handling */
	కొరకు (d = మొదటి; d != b; d = d->తదుపరి) {
		ఒకవేళ ((d->type == BOX_INLINE && d->inline_చివర) ||
				d->type == BOX_BR ||
				d->type == BOX_పాఠ్యం ||
				d->type == BOX_INLINE_చివర) {
			css_fixed విలువ = 0;
			css_unit unit = CSS_UNIT_PX;
			ఎంపిక (css_computed_vertical_align(d->style, &విలువ,
					&unit)) {
			స్థితి CSS_VERTICAL_ALIGN_SUPER:
			స్థితి CSS_VERTICAL_ALIGN_శిఖరం:
			స్థితి CSS_VERTICAL_ALIGN_పాఠ్యం_శిఖరం:
				/* already at top */
				ఆపు;
			స్థితి CSS_VERTICAL_ALIGN_SUB:
			స్థితి CSS_VERTICAL_ALIGN_BOTTOM:
			స్థితి CSS_VERTICAL_ALIGN_పాఠ్యం_BOTTOM:
				d->y += వాడిన_ఎత్తు - d->ఎత్తు;
				ఆపు;
			అప్రమేయ:
			స్థితి CSS_VERTICAL_ALIGN_BASELINE:
				d->y += 0.75 * (వాడిన_ఎత్తు - d->ఎత్తు);
				ఆపు;
			}
		}
	}

	/* handle clearance for br */
	ఒకవేళ (br_box && css_computed_clear(br_box->style) != CSS_CLEAR_NONE) {
		పూర్ణాంకం clear_y = layout_clear(cont->float_children,
				css_computed_clear(br_box->style));
		ఒకవేళ (వాడిన_ఎత్తు < clear_y - cy)
			వాడిన_ఎత్తు = clear_y - cy;
	}

	ఒకవేళ (move_y)
		*y += వాడిన_ఎత్తు;
	*తదుపరి_box = b;
	*వెడల్పు = x; /* return actual width */
	తిరిగి true;
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
స్థిర bool layout_inline_container(నిర్మాణం box *inline_container, పూర్ణాంకం వెడల్పు,
		నిర్మాణం box *cont, పూర్ణాంకం cx, పూర్ణాంకం cy, html_content *content)
{
	bool మొదటి_పంక్తి = true;
	bool has_పాఠ్యం_children;
	నిర్మాణం box *c, *తదుపరి;
	పూర్ణాంకం y = 0;
	పూర్ణాంకం curwidth,maxwidth = వెడల్పు;

	ప్రతిజ్ఞా(inline_container->type == BOX_INLINE_CONTAINER);

	NSLOG(layout, DEBUG,
	      "inline_container %p, width %i, cont %p, cx %i, cy %i",
	      inline_container,
	      వెడల్పు,
	      cont,
	      cx,
	      cy);


	has_పాఠ్యం_children = false;
	కొరకు (c = inline_container->children; c; c = c->తదుపరి) {
		bool is_pre = false;

		ఒకవేళ (c->style) {
			గణన css_white_space_e whitespace;

			whitespace = css_computed_white_space(c->style);

			is_pre = (whitespace == CSS_WHITE_SPACE_PRE ||
				whitespace == CSS_WHITE_SPACE_PRE_పంక్తి ||
				whitespace == CSS_WHITE_SPACE_PRE_WRAP);
		}

		ఒకవేళ ((lh__box_is_object(c) == false &&
				c->పాఠ్యం && (c->నిడివి || is_pre)) ||
				c->type == BOX_BR)
			has_పాఠ్యం_children = true;
	}

	/** \todo fix wrapping so that a box with horizontal scrollbar will
	 * shrink back to 'width' if no word is wider than 'width' (Or just set
	 * curwidth = width and have the multiword lines wrap to the min width)
	 */
	కొరకు (c = inline_container->children; c; ) {

		NSLOG(layout, DEBUG, "c %p", c);

		curwidth = inline_container->వెడల్పు;
		ఒకవేళ (!layout_పంక్తి(c, &curwidth, &y, cx, cy + y, cont, మొదటి_పంక్తి,
				has_పాఠ్యం_children, content, &తదుపరి))
			తిరిగి false;
		maxwidth = max(maxwidth,curwidth);
		c = తదుపరి;
		మొదటి_పంక్తి = false;
	}

	inline_container->వెడల్పు = maxwidth;
	inline_container->ఎత్తు = y;

	తిరిగి true;
}


/* Documented in layout_intertnal.h */
bool layout_ఖండం_context(
		నిర్మాణం box *ఖండం,
		పూర్ణాంకం viewport_ఎత్తు,
		html_content *content)
{
	నిర్మాణం box *box;
	పూర్ణాంకం cx, cy;  /**< current coordinates */
	పూర్ణాంకం max_pos_margin = 0;
	పూర్ణాంకం max_neg_margin = 0;
	పూర్ణాంకం y = 0;
	పూర్ణాంకం lm, rm;
	నిర్మాణం box *margin_collapse = ఖాళీ;
	bool లోపల_margin = false;
	css_fixed gadget_కొలత;
	css_unit gadget_unit; /* Checkbox / radio buttons */

	ప్రతిజ్ఞా(ఖండం->type == BOX_ఖండం ||
			ఖండం->type == BOX_INLINE_ఖండం ||
			ఖండం->type == BOX_పట్టిక_CELL ||
			ఖండం->type == BOX_FLEX ||
			ఖండం->type == BOX_INLINE_FLEX);
	ప్రతిజ్ఞా(ఖండం->వెడల్పు != UNKNOWN_వెడల్పు);
	ప్రతిజ్ఞా(ఖండం->వెడల్పు != AUTO);

	ఖండం->float_children = ఖాళీ;
	ఖండం->cached_ఉంచు_క్రింద_level = 0;
	ఖండం->clear_level = 0;

	/* special case if the block contains an object */
	ఒకవేళ (ఖండం->object) {
		పూర్ణాంకం తాత్కాలిక_వెడల్పు = ఖండం->వెడల్పు;
		ఒకవేళ (!layout_ఖండం_object(ఖండం))
			తిరిగి false;
		layout_get_object_dimensions(ఖండం, &తాత్కాలిక_వెడల్పు,
				&ఖండం->ఎత్తు, INT_MIN, గరిష్ఠ_పూర్ణాంకం,
				INT_MIN, గరిష్ఠ_పూర్ణాంకం);
		తిరిగి true;
	} లేకపోతే ఒకవేళ (ఖండం->flags & REPLACE_DIM) {
		తిరిగి true;
	}

	/* special case if the block contains an radio button or checkbox */
	ఒకవేళ (ఖండం->gadget && (ఖండం->gadget->type == GADGET_RADIO ||
			ఖండం->gadget->type == GADGET_CHECKBOX)) {
		/* form checkbox or radio button
		 * if width or height is AUTO, set it to 1em */
		gadget_unit = CSS_UNIT_EM;
		gadget_కొలత = INTTOFIX(1);
		ఒకవేళ (ఖండం->ఎత్తు == AUTO)
			ఖండం->ఎత్తు = FIXTOINT(css_unit_len2device_px(
					ఖండం->style,
					&content->unit_len_ctx,
					gadget_కొలత, gadget_unit));
	}

	box = ఖండం->children;
	/* set current coordinates to top-left of the block */
	cx = 0;
	y = cy = ఖండం->padding[శిఖరం_2];
	ఒకవేళ (box)
		box->y = ఖండం->padding[శిఖరం_2];

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
	అయితే (box) {
		గణన css_అధికప్రవాహం_e అధికప్రవాహం_x = CSS_అధికప్రవాహం_VISIBLE;
		గణన css_అధికప్రవాహం_e అధికప్రవాహం_y = CSS_అధికప్రవాహం_VISIBLE;

		ప్రతిజ్ఞా(box->type == BOX_ఖండం ||
				box->type == BOX_FLEX ||
				box->type == BOX_పట్టిక ||
				box->type == BOX_INLINE_CONTAINER);

		/* Tables are laid out before being positioned, because the
		 * position depends on the width which is calculated in
		 * table layout. Blocks and inline containers are positioned
		 * before being laid out, because width is not dependent on
		 * content, and the position is required during layout for
		 * correct handling of floats.
		 */

		ఒకవేళ (box->style &&
				(css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE ||
				 css_computed_position(box->style) ==
					CSS_POSITION_FIXED)) {
			box->x = box->మాతృ->padding[ఎడమ_2];
			/* absolute positioned; this element will establish
			 * its own block context when it gets laid out later,
			 * so no need to look at its children now. */
			వెళ్ళు advance_గమ్యం_తదుపరి_box;
		}

		/* If we don't know which box the current margin collapses
		 * through to, find out.  Update the pos/neg margin values. */
		ఒకవేళ (margin_collapse == ఖాళీ) {
			margin_collapse = layout_తదుపరి_margin_ఖండం(
					&content->unit_len_ctx, box, ఖండం,
					viewport_ఎత్తు,
					&max_pos_margin, &max_neg_margin);
			/* We have a margin that has not yet been applied. */
			లోపల_margin = true;
		}

		/* Clearance. */
		y = 0;
		ఒకవేళ (box->style && css_computed_clear(box->style) !=
				CSS_CLEAR_NONE)
			y = layout_clear(ఖండం->float_children,
					css_computed_clear(box->style));

		/* Find box's overflow properties */
		ఒకవేళ (box->style) {
			అధికప్రవాహం_x = css_computed_అధికప్రవాహం_x(box->style);
			అధికప్రవాహం_y = css_computed_అధికప్రవాహం_y(box->style);
		}

		/* Blocks establishing a block formatting context get minimum
		 * left and right margins to avoid any floats. */
		lm = rm = 0;

		ఒకవేళ (box->type == BOX_FLEX ||
		    box->type == BOX_ఖండం ||
		    box->flags & IFRAME) {
			ఒకవేళ (lh__box_is_object(box) == false &&
					box->style &&
					(అధికప్రవాహం_x != CSS_అధికప్రవాహం_VISIBLE ||
					 అధికప్రవాహం_y != CSS_అధికప్రవాహం_VISIBLE)) {
				/* box establishes new block formatting context
				 * so available width may be diminished due to
				 * floats. */
				పూర్ణాంకం x0, x1, శిఖరం;
				నిర్మాణం box *ఎడమ, *కుడి;
				శిఖరం = cy + max_pos_margin - max_neg_margin;
				శిఖరం = (శిఖరం > y) ? శిఖరం : y;
				x0 = cx;
				x1 = cx + box->మాతృ->వెడల్పు -
						box->మాతృ->padding[ఎడమ_2] -
						box->మాతృ->padding[కుడి_2];
				కనుగొను_sides(ఖండం->float_children, శిఖరం, శిఖరం,
						&x0, &x1, &ఎడమ, &కుడి);
				/* calculate min required left & right margins
				 * needed to avoid floats */
				lm = x0 - cx;
				rm = cx + box->మాతృ->వెడల్పు -
						box->మాతృ->padding[ఎడమ_2] -
						box->మాతృ->padding[కుడి_2] -
						x1;
			}
			layout_ఖండం_కనుగొను_dimensions(&content->unit_len_ctx,
					box->మాతృ->వెడల్పు,
					viewport_ఎత్తు, lm, rm, box);
			ఒకవేళ (box->type == BOX_ఖండం && !(box->flags & IFRAME)) {
				layout_ఖండం_కూడు_scrollbar(box, కుడి_2);
				layout_ఖండం_కూడు_scrollbar(box, BOTTOM);
			}
		} లేకపోతే ఒకవేళ (box->type == BOX_పట్టిక) {
			ఒకవేళ (box->style != ఖాళీ) {
				గణన css_వెడల్పు_e wtype;
				css_fixed వెడల్పు = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_వెడల్పు(box->style, &వెడల్పు,
						&unit);

				ఒకవేళ (wtype == CSS_వెడల్పు_AUTO) {
					/* max available width may be
					 * diminished due to floats. */
					పూర్ణాంకం x0, x1, శిఖరం;
					నిర్మాణం box *ఎడమ, *కుడి;
					శిఖరం = cy + max_pos_margin -
							max_neg_margin;
					శిఖరం = (శిఖరం > y) ? శిఖరం : y;
					x0 = cx;
					x1 = cx + box->మాతృ->వెడల్పు -
						box->మాతృ->padding[ఎడమ_2] -
						box->మాతృ->padding[కుడి_2];
					కనుగొను_sides(ఖండం->float_children,
						శిఖరం, శిఖరం, &x0, &x1,
						&ఎడమ, &కుడి);
					/* calculate min required left & right
					 * margins needed to avoid floats */
					lm = x0 - cx;
					rm = cx + box->మాతృ->వెడల్పు -
						box->మాతృ->padding[ఎడమ_2] -
						box->మాతృ->padding[కుడి_2] -
						x1;
				}
			}
			ఒకవేళ (!layout_పట్టిక(box, box->మాతృ->వెడల్పు - lm - rm,
					content))
				తిరిగి false;
			layout_solve_వెడల్పు(box, box->మాతృ->వెడల్పు, box->వెడల్పు,
					lm, rm, -1, -1);
		}

		/* Position box: horizontal. */
		box->x = box->మాతృ->padding[ఎడమ_2] + box->margin[ఎడమ_2] +
				box->border[ఎడమ_2].వెడల్పు;
		cx += box->x;

		/* Position box: vertical. */
		ఒకవేళ (box->border[శిఖరం_2].వెడల్పు) {
			box->y += box->border[శిఖరం_2].వెడల్పు;
			cy += box->border[శిఖరం_2].వెడల్పు;
		}

		/* Vertical margin */
		ఒకవేళ (((box->type == BOX_ఖండం && (box->flags & HAS_ఎత్తు)) ||
		     box->type == BOX_FLEX ||
		     box->type == BOX_పట్టిక ||
		     (box->type == BOX_INLINE_CONTAINER &&
		      !box_is_మొదటి_బిడ్డ(box)) ||
		     margin_collapse == box) &&
		    లోపల_margin == true) {
			/* Margin goes above this box. */
			cy += max_pos_margin - max_neg_margin;
			box->y += max_pos_margin - max_neg_margin;

			/* Current margin has been applied. */
			లోపల_margin = false;
			max_pos_margin = max_neg_margin = 0;
		}

		/* Handle clearance */
		ఒకవేళ (box->type != BOX_INLINE_CONTAINER &&
				(y > 0) && (cy < y)) {
			/* box clears something*/
			box->y += y - cy;
			cy = y;
		}

		/* Unless the box has an overflow style of visible, the box
		 * establishes a new block context. */
		ఒకవేళ (box->type == BOX_FLEX ||
				(box->type == BOX_ఖండం && box->style &&
				 (అధికప్రవాహం_x != CSS_అధికప్రవాహం_VISIBLE ||
				  అధికప్రవాహం_y != CSS_అధికప్రవాహం_VISIBLE))) {

			ఒకవేళ (box->type == BOX_FLEX) {
				ఒకవేళ (!layout_flex(box, box->వెడల్పు, content)) {
					తిరిగి false;
				}
			} లేకపోతే {
				layout_ఖండం_context(box,
						viewport_ఎత్తు, content);
			}

			cy += box->padding[శిఖరం_2];

			ఒకవేళ (box->ఎత్తు == AUTO) {
				box->ఎత్తు = 0;
				layout_ఖండం_కూడు_scrollbar(box, BOTTOM);
			}

			cx -= box->x;
			cy += box->ఎత్తు + box->padding[BOTTOM] +
					box->border[BOTTOM].వెడల్పు;
			y = box->y + box->padding[శిఖరం_2] + box->ఎత్తు +
					box->padding[BOTTOM] +
					box->border[BOTTOM].వెడల్పు;

			/* Skip children, because they are done in the new
			 * block context */
			వెళ్ళు advance_గమ్యం_తదుపరి_box;
		}

		NSLOG(layout, DEBUG,  "box %p, cx %i, cy %i, width %i",
				box, cx, cy, box->వెడల్పు);

		/* Layout (except tables). */
		ఒకవేళ (box->object) {
			ఒకవేళ (!layout_ఖండం_object(box))
				తిరిగి false;

		} లేకపోతే ఒకవేళ (box->type == BOX_INLINE_CONTAINER) {
			box->వెడల్పు = box->మాతృ->వెడల్పు;
			ఒకవేళ (!layout_inline_container(box, box->వెడల్పు, ఖండం,
					cx, cy, content))
				తిరిగి false;

		} లేకపోతే ఒకవేళ (box->type == BOX_పట్టిక) {
			/* Move down to avoid floats if necessary. */
			పూర్ణాంకం x0, x1;
			నిర్మాణం box *ఎడమ, *కుడి;
			y = cy;
			అయితే (1) {
				గణన css_వెడల్పు_e wtype;
				css_fixed వెడల్పు = 0;
				css_unit unit = CSS_UNIT_PX;

				wtype = css_computed_వెడల్పు(box->style,
						&వెడల్పు, &unit);

				x0 = cx;
				x1 = cx + box->మాతృ->వెడల్పు;
				కనుగొను_sides(ఖండం->float_children, y,
						y + box->ఎత్తు,
						&x0, &x1, &ఎడమ, &కుడి);
				ఒకవేళ (wtype == CSS_వెడల్పు_AUTO)
					ఆపు;
				ఒకవేళ (box->వెడల్పు <= x1 - x0)
					ఆపు;
				ఒకవేళ (!ఎడమ && !కుడి)
					ఆపు;
				లేకపోతే ఒకవేళ (!ఎడమ)
					y = కుడి->y + కుడి->ఎత్తు + 1;
				లేకపోతే ఒకవేళ (!కుడి)
					y = ఎడమ->y + ఎడమ->ఎత్తు + 1;
				లేకపోతే ఒకవేళ (ఎడమ->y + ఎడమ->ఎత్తు <
						కుడి->y + కుడి->ఎత్తు)
					y = ఎడమ->y + ఎడమ->ఎత్తు + 1;
				లేకపోతే
					y = కుడి->y + కుడి->ఎత్తు + 1;
			}
			box->x += x0 - cx;
			cx = x0;
			box->y += y - cy;
			cy = y;
		}

		/* Advance to next box. */
		ఒకవేళ (box->type == BOX_ఖండం && !box->object && !(box->iframe) &&
				box->children) {
			/* Down into children. */

			ఒకవేళ (box == margin_collapse) {
				/* Current margin collapsed though to this box.
				 * Unset margin_collapse. */
				margin_collapse = ఖాళీ;
			}

			y = box->padding[శిఖరం_2];
			box = box->children;
			box->y = y;
			cy += y;
			కొనసాగు;
		} లేకపోతే ఒకవేళ (box->type == BOX_ఖండం || box->object ||
				box->flags & IFRAME)
			cy += box->padding[శిఖరం_2];

		ఒకవేళ (box->type == BOX_ఖండం && box->ఎత్తు == AUTO) {
			box->ఎత్తు = 0;
			layout_ఖండం_కూడు_scrollbar(box, BOTTOM);
		}

		cy += box->ఎత్తు + box->padding[BOTTOM] +
				box->border[BOTTOM].వెడల్పు;
		cx -= box->x;
		y = box->y + box->padding[శిఖరం_2] + box->ఎత్తు +
				box->padding[BOTTOM] +
				box->border[BOTTOM].వెడల్పు;

	advance_గమ్యం_తదుపరి_box:
		ఒకవేళ (!box->తదుపరి) {
			/* No more siblings:
			 * up to first ancestor with a sibling. */

			చేయి {
				ఒకవేళ (box == margin_collapse) {
					/* Current margin collapsed though to
					 * this box.  Unset margin_collapse. */
					margin_collapse = ఖాళీ;
				}

				/* Apply bottom margin */
				ఒకవేళ (max_pos_margin < box->margin[BOTTOM])
					max_pos_margin = box->margin[BOTTOM];
				లేకపోతే ఒకవేళ (max_neg_margin < -box->margin[BOTTOM])
					max_neg_margin = -box->margin[BOTTOM];

				box = box->మాతృ;
				ఒకవేళ (box == ఖండం)
					ఆపు;

				/* Margin is invalidated if this is a box
				 * margins can't collapse through. */
				ఒకవేళ (box->type == BOX_ఖండం &&
						box->flags & MAKE_ఎత్తు) {
					margin_collapse = ఖాళీ;
					లోపల_margin = false;
					max_pos_margin = max_neg_margin = 0;
				}

				ఒకవేళ (box->ఎత్తు == AUTO) {
					box->ఎత్తు = y - box->padding[శిఖరం_2];

					ఒకవేళ (box->type == BOX_ఖండం)
						layout_ఖండం_కూడు_scrollbar(box,
								BOTTOM);
				} లేకపోతే
					cy += box->ఎత్తు -
							(y - box->padding[శిఖరం_2]);

				/* Apply any min-height and max-height to
				 * boxes in normal flow */
				ఒకవేళ (box->style &&
					css_computed_position(box->style) !=
						CSS_POSITION_ABSOLUTE &&
						layout_apply_minmax_ఎత్తు(
							&content->unit_len_ctx,
							box, ఖాళీ)) {
					/* Height altered */
					/* Set current cy */
					cy += box->ఎత్తు -
							(y - box->padding[శిఖరం_2]);
				}

				cy += box->padding[BOTTOM] +
						box->border[BOTTOM].వెడల్పు;
				cx -= box->x;
				y = box->y + box->padding[శిఖరం_2] + box->ఎత్తు +
						box->padding[BOTTOM] +
						box->border[BOTTOM].వెడల్పు;

			} అయితే (box->తదుపరి == ఖాళీ);
			ఒకవేళ (box == ఖండం)
				ఆపు;
		}

		/* To next sibling. */

		ఒకవేళ (box == margin_collapse) {
			/* Current margin collapsed though to this box.
			 * Unset margin_collapse. */
			margin_collapse = ఖాళీ;
		}

		ఒకవేళ (max_pos_margin < box->margin[BOTTOM])
			max_pos_margin = box->margin[BOTTOM];
		లేకపోతే ఒకవేళ (max_neg_margin < -box->margin[BOTTOM])
			max_neg_margin = -box->margin[BOTTOM];

		box = box->తదుపరి;
		box->y = y;
	}

	/* Account for bottom margin of last contained block */
	cy += max_pos_margin - max_neg_margin;

	/* Increase height to contain any floats inside (CSS 2.1 10.6.7). */
	కొరకు (box = ఖండం->float_children; box; box = box->తదుపరి_float) {
		y = box->y + box->ఎత్తు + box->padding[BOTTOM] +
				box->border[BOTTOM].వెడల్పు + box->margin[BOTTOM];
		ఒకవేళ (cy < y)
			cy = y;
	}

	ఒకవేళ (ఖండం->ఎత్తు == AUTO) {
		ఖండం->ఎత్తు = cy - ఖండం->padding[శిఖరం_2];
		ఒకవేళ (ఖండం->type == BOX_ఖండం)
			layout_ఖండం_కూడు_scrollbar(ఖండం, BOTTOM);
	}

	ఒకవేళ (ఖండం->style && css_computed_position(ఖండం->style) !=
			CSS_POSITION_ABSOLUTE) {
		/* Block is in normal flow */
		layout_apply_minmax_ఎత్తు(&content->unit_len_ctx, ఖండం, ఖాళీ);
	}

	ఒకవేళ (ఖండం->gadget &&
			(ఖండం->gadget->type == GADGET_TEXTAREA ||
			ఖండం->gadget->type == GADGET_PASSWORD ||
			ఖండం->gadget->type == GADGET_TEXTBOX)) {
		plot_font_style_t fstyle;
		పూర్ణాంకం ta_వెడల్పు = ఖండం->padding[ఎడమ_2] + ఖండం->వెడల్పు +
				ఖండం->padding[కుడి_2];
		పూర్ణాంకం ta_ఎత్తు = ఖండం->padding[శిఖరం_2] + ఖండం->ఎత్తు +
				ఖండం->padding[BOTTOM];
		font_plot_style_from_css(&content->unit_len_ctx,
				ఖండం->style, &fstyle);
		fstyle.background = NS_TRANSPARENT;
		textarea_సమితి_layout(ఖండం->gadget->దత్తాంశం.పాఠ్యం.ta,
				&fstyle, ta_వెడల్పు, ta_ఎత్తు,
				ఖండం->padding[శిఖరం_2], ఖండం->padding[కుడి_2],
				ఖండం->padding[BOTTOM], ఖండం->padding[ఎడమ_2]);
	}

	తిరిగి true;
}

/**
 * Get a dom node's element tag type.
 *
 * \param[in]  node  Node to get tag type of.
 * \param[in]  type  Returns element tag type on success.
 * \return true if on success, false otherwise.
 */
స్థిర bool
layout_get_element_ట్యాగ్(
		స్థిరరాశి dom_కణుపు *కణుపు,
		dom_html_element_type *type)
{
	dom_html_element_type element_type;
	dom_కణుపు_type కణుపు_type;
	dom_exception exc;

	exc = dom_కణుపు_get_కణుపు_type(కణుపు, &కణుపు_type);
	ఒకవేళ (exc != DOM_NO_ERR ||
	    కణుపు_type != DOM_ELEMENT_కణుపు) {
		తిరిగి false;
	}

	exc = dom_html_element_get_ట్యాగ్_type(కణుపు, &element_type);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి false;
	}

	*type = element_type;
	తిరిగి true;
}


/**
 * Check a node's tag type.
 *
 * \param[in]  node  Node to check tag type of.
 * \param[in]  type  Tag type to test for.
 * \return true if if node has given type, false otherwise.
 */
స్థిర అంతఃవరుస bool
layout__check_element_type(
		స్థిరరాశి dom_కణుపు *కణుపు,
		dom_html_element_type type)
{
	dom_html_element_type element_type;

	ఒకవేళ (!layout_get_element_ట్యాగ్(కణుపు, &element_type)) {
		తిరిగి false;
	}

	తిరిగి element_type == type;
}


/**
 * Helper to get attribute value from a LI node.
 *
 * \param[in]  li_node    DOM node for the LI element;
 * \param[out] value_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
స్థిర bool
layout_get_li_విలువ(dom_కణుపు *li_కణుపు, dom_long *విలువ_బహిర్గతం)
{
	dom_exception exc;
	dom_long విలువ;
	bool has_విలువ;

	/** \todo
	 * dom_html_li_element_get_value() is rubbish and we can't tell
	 * a lack of value attribute or invalid value from a perfectly
	 * valid '-1'.
	 *
	 * This helps for the common case of no value.  However we should
	 * fix libdom to have some kind of sane interface to get numerical
	 * attributes.
	 */
	exc = dom_element_has_attribute(li_కణుపు,
			corestring_dom_విలువ,
			&has_విలువ);
	ఒకవేళ (exc != DOM_NO_ERR || has_విలువ == false) {
		తిరిగి false;
	}

	exc = dom_html_li_element_get_విలువ(
			(dom_html_li_element *)li_కణుపు,
			&విలువ);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి false;
	}

	*విలువ_బహిర్గతం = విలువ;
	తిరిగి true;
}


/**
 * Helper to get start attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \param[out] start_out  Returns the value on success.
 * \return true if node has value, otherwise false.
 */
స్థిర bool
layout_get_ol_ప్రారంభం(dom_కణుపు *ol_కణుపు, dom_long *ప్రారంభం_బహిర్గతం)
{
	dom_exception exc;
	dom_long ప్రారంభం;
	bool has_ప్రారంభం;

	/** \todo
	 * see layout__get_li_value().
	 */
	exc = dom_element_has_attribute(ol_కణుపు,
			corestring_dom_ప్రారంభం,
			&has_ప్రారంభం);
	ఒకవేళ (exc != DOM_NO_ERR || has_ప్రారంభం == false) {
		తిరిగి false;
	}

	exc = dom_html_olist_element_get_ప్రారంభం(
			(dom_html_olist_element *)ol_కణుపు,
			&ప్రారంభం);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి false;
	}

	*ప్రారంభం_బహిర్గతం = ప్రారంభం;
	తిరిగి true;
}


/**
 * Helper to get reversed attribute value from a OL node.
 *
 * \param[in]  ol_node    DOM node for the OL element;
 * \return true if node has reversed, otherwise false.
 */
స్థిర bool
layout__get_ol_reversed(dom_కణుపు *ol_కణుపు)
{
	dom_exception exc;
	bool has_reversed;

	exc = dom_element_has_attribute(ol_కణుపు,
			corestring_dom_reversed,
			&has_reversed);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి false;
	}

	తిరిగి has_reversed;
}


/**
 * Get the number of list items for a list owner.
 *
 * \param[in]  list_owner  DOM node to count list items for.
 * \param[in]  count_out   Returns list item count on success.
 * \return true on success, otherwise false.
 */
స్థిర bool
layout_get_list_అంశం_లెక్క(
		dom_కణుపు *list_owner, dom_long *లెక్క_బహిర్గతం)
{
	dom_html_element_type ట్యాగ్_type;
	dom_exception exc;
	dom_కణుపు *బిడ్డ;
	పూర్ణాంకం లెక్క;

	ఒకవేళ (list_owner == ఖాళీ) {
		తిరిగి false;
	}

	ఒకవేళ (!layout_get_element_ట్యాగ్(list_owner, &ట్యాగ్_type)) {
		తిరిగి false;
	}

	ఒకవేళ (ట్యాగ్_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    ట్యాగ్_type != DOM_HTML_ELEMENT_TYPE_UL) {
		తిరిగి false;
	}

	exc = dom_కణుపు_get_మొదటి_బిడ్డ(list_owner, &బిడ్డ);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి false;
	}

	లెక్క = 0;
	అయితే (బిడ్డ != ఖాళీ) {
		dom_కణుపు *తాత్కాలిక_కణుపు;

		ఒకవేళ (layout__check_element_type(బిడ్డ,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			నిర్మాణం box *బిడ్డ_box;
			ఒకవేళ (dom_కణుపు_get_user_దత్తాంశం(బిడ్డ,
					corestring_dom_ns_కీలకం_box_కణుపు_దత్తాంశం,
					&బిడ్డ_box) != DOM_NO_ERR) {
				dom_కణుపు_unref(బిడ్డ);
				తిరిగి false;
			}

			ఒకవేళ (బిడ్డ_box != ఖాళీ &&
			    బిడ్డ_box->list_marker != ఖాళీ) {
				లెక్క++;
			}
		}

		exc = dom_కణుపు_get_తదుపరి_sibling(బిడ్డ, &తాత్కాలిక_కణుపు);
		dom_కణుపు_unref(బిడ్డ);
		ఒకవేళ (exc != DOM_NO_ERR) {
			తిరిగి false;
		}

		బిడ్డ = తాత్కాలిక_కణుపు;
	}

	*లెక్క_బహిర్గతం = లెక్క;
	తిరిగి true;
}


/**
 * Handle list item counting, if this is a list owner box.
 *
 * \param[in]  box  Box to do list item counting for.
 */
స్థిర శూన్య
layout_ordered_list_లెక్క(
		నిర్మాణం box *box)
{
	dom_html_element_type ట్యాగ్_type;
	dom_exception exc;
	dom_కణుపు *బిడ్డ;
	పూర్ణాంకం అడుగు = 1;
	dom_long తదుపరి;

	ఒకవేళ (box->కణుపు == ఖాళీ) {
		తిరిగి;
	}

	ఒకవేళ (!layout_get_element_ట్యాగ్(box->కణుపు, &ట్యాగ్_type)) {
		తిరిగి;
	}

	ఒకవేళ (ట్యాగ్_type != DOM_HTML_ELEMENT_TYPE_OL &&
	    ట్యాగ్_type != DOM_HTML_ELEMENT_TYPE_UL) {
		తిరిగి;
	}

	తదుపరి = 1;
	ఒకవేళ (ట్యాగ్_type == DOM_HTML_ELEMENT_TYPE_OL) {
		bool have_ప్రారంభం = layout_get_ol_ప్రారంభం(box->కణుపు, &తదుపరి);
		bool have_reversed = layout__get_ol_reversed(box->కణుపు);

		ఒకవేళ (have_reversed) {
			అడుగు = -1;
		}

		ఒకవేళ (!have_ప్రారంభం && have_reversed) {
			layout_get_list_అంశం_లెక్క(box->కణుపు, &తదుపరి);
		}
	}

	exc = dom_కణుపు_get_మొదటి_బిడ్డ(box->కణుపు, &బిడ్డ);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి;
	}

	అయితే (బిడ్డ != ఖాళీ) {
		dom_కణుపు *తాత్కాలిక_కణుపు;

		ఒకవేళ (layout__check_element_type(బిడ్డ,
				DOM_HTML_ELEMENT_TYPE_LI)) {
			నిర్మాణం box *బిడ్డ_box;

			ఒకవేళ (dom_కణుపు_get_user_దత్తాంశం(బిడ్డ,
					corestring_dom_ns_కీలకం_box_కణుపు_దత్తాంశం,
					&బిడ్డ_box) != DOM_NO_ERR) {
				dom_కణుపు_unref(బిడ్డ);
				తిరిగి;
			}

			ఒకవేళ (బిడ్డ_box != ఖాళీ &&
			    బిడ్డ_box->list_marker != ఖాళీ) {
				dom_long విలువ;
				నిర్మాణం box *marker = బిడ్డ_box->list_marker;
				ఒకవేళ (layout_get_li_విలువ(బిడ్డ, &విలువ)) {
					marker->list_విలువ = విలువ;
					తదుపరి = marker->list_విలువ;
				} లేకపోతే {
					marker->list_విలువ = తదుపరి;
				}
				తదుపరి += అడుగు;
			}
		}

		exc = dom_కణుపు_get_తదుపరి_sibling(బిడ్డ, &తాత్కాలిక_కణుపు);
		dom_కణుపు_unref(బిడ్డ);
		ఒకవేళ (exc != DOM_NO_ERR) {
			తిరిగి;
		}

		బిడ్డ = తాత్కాలిక_కణుపు;
	}
}

/**
 * Set up the marker text for a numerical list item.
 *
 * \param[in]  content  The HTML content.
 * \param[in]  box      The list item's main box.
 */
స్థిర శూన్య
layout_సమితి_numerical_marker_పాఠ్యం(
		స్థిరరాశి html_content *content,
		నిర్మాణం box *box)
{
	నిర్మాణం box *marker = box->list_marker;
	size_t counter_len;
	css_error css_res;
	గణన {
		/**
		 * initial length of a list marker buffer
		 *
		 * enough for 9,999,999,999,999,999,999 in decimal
		 * or five characters for 4-byte UTF-8.
		 */
		LIST_MARKER_కొలత = 20,
	};

	marker->పాఠ్యం = talloc_array(content->bctx, అక్షరం, LIST_MARKER_కొలత);
	ఒకవేళ (marker->పాఠ్యం == ఖాళీ) {
		తిరిగి;
	}

	css_res = css_computed_format_list_style(box->style, marker->list_విలువ,
			marker->పాఠ్యం, LIST_MARKER_కొలత, &counter_len);
	ఒకవేళ (css_res == CSS_OK) {
		ఒకవేళ (counter_len > LIST_MARKER_కొలత) {
			/* Use computed size as marker did not fit in
			 * default allocation. */
			marker->పాఠ్యం = talloc_realloc(content->bctx,
					marker->పాఠ్యం,
					అక్షరం,
					counter_len);
			ఒకవేళ (marker->పాఠ్యం == ఖాళీ) {
				తిరిగి;
			}
			css_computed_format_list_style(box->style,
					marker->list_విలువ, marker->పాఠ్యం,
					counter_len, &counter_len);
		}
		marker->నిడివి = counter_len;
	}
}

/**
 * Find out if box's style represents a numerical list style type.
 *
 * \param[in]  b  Box with style to test.
 * \return true if box has numerical list style type, false otherwise.
 */
స్థిర bool
layout_list_అంశం_is_numerical(
		స్థిరరాశి నిర్మాణం box *b)
{
	గణన css_list_style_type_e t = css_computed_list_style_type(b->style);

	ఎంపిక (t) {
	స్థితి CSS_LIST_STYLE_TYPE_DISC:   /* Fall through. */
	స్థితి CSS_LIST_STYLE_TYPE_CIRCLE: /* Fall through. */
	స్థితి CSS_LIST_STYLE_TYPE_వర్గం: /* Fall through. */
	స్థితి CSS_LIST_STYLE_TYPE_NONE:
		తిరిగి false;

	అప్రమేయ:
		తిరిగి true;
	}
}

/**
 * Layout list markers.
 */
స్థిర శూన్య
layout_lists(స్థిరరాశి html_content *content, నిర్మాణం box *box)
{
	నిర్మాణం box *బిడ్డ;

	layout_ordered_list_లెక్క(box);

	కొరకు (బిడ్డ = box->children; బిడ్డ; బిడ్డ = బిడ్డ->తదుపరి) {
		ఒకవేళ (బిడ్డ->list_marker) {
			నిర్మాణం box *marker = బిడ్డ->list_marker;

			ఒకవేళ (layout_list_అంశం_is_numerical(బిడ్డ)) {
				ఒకవేళ (marker->పాఠ్యం == ఖాళీ) {
					layout_సమితి_numerical_marker_పాఠ్యం(
							content, బిడ్డ);
				}
			}
			ఒకవేళ (marker->object) {
				marker->వెడల్పు =
					content_get_వెడల్పు(marker->object);
				marker->x = -marker->వెడల్పు;
				marker->ఎత్తు =
					content_get_ఎత్తు(marker->object);
				marker->y = (పంక్తి_ఎత్తు(
						&content->unit_len_ctx,
						marker->style) -
						marker->ఎత్తు) / 2;
			} లేకపోతే ఒకవేళ (marker->పాఠ్యం) {
				ఒకవేళ (marker->వెడల్పు == UNKNOWN_వెడల్పు) {
					plot_font_style_t fstyle;
					font_plot_style_from_css(
							&content->unit_len_ctx,
							marker->style,
							&fstyle);
					content->font_func->వెడల్పు(&fstyle,
							marker->పాఠ్యం,
							marker->నిడివి,
							&marker->వెడల్పు);
					marker->flags |= MEASURED;
				}
				marker->x = -marker->వెడల్పు;
				marker->y = 0;
				marker->ఎత్తు = పంక్తి_ఎత్తు(
						&content->unit_len_ctx,
						marker->style);
			} లేకపోతే {
				marker->x = 0;
				marker->y = 0;
				marker->వెడల్పు = 0;
				marker->ఎత్తు = 0;
			}
			/* Gap between marker and content */
			marker->x -= 4;
		}
		layout_lists(content, బిడ్డ);
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
స్థిర శూన్య
layout_compute_offsets(స్థిరరాశి css_unit_ctx *unit_len_ctx,
		       నిర్మాణం box *box,
		       నిర్మాణం box *containing_ఖండం,
		       పూర్ణాంకం *శిఖరం,
		       పూర్ణాంకం *కుడి,
		       పూర్ణాంకం *bottom,
		       పూర్ణాంకం *ఎడమ)
{
	uint32_t type;
	css_fixed విలువ = 0;
	css_unit unit = CSS_UNIT_PX;

	ప్రతిజ్ఞా(containing_ఖండం->వెడల్పు != UNKNOWN_వెడల్పు);
	ప్రతిజ్ఞా(containing_ఖండం->వెడల్పు != AUTO);
	ప్రతిజ్ఞా(containing_ఖండం->ఎత్తు != AUTO);

	/*ఎడమ*/
	type = css_computed_ఎడమ(box->style, &విలువ, &unit);
	ఒకవేళ (type == CSS_ఎడమ_సమితి) {
		ఒకవేళ (unit == CSS_UNIT_PCT) {
			*ఎడమ = FPCT_యొక్క_INT_TOINT(విలువ,
					containing_ఖండం->వెడల్పు);
		} లేకపోతే {
			*ఎడమ = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					విలువ, unit));
		}
	} లేకపోతే {
		*ఎడమ = AUTO;
	}

	/*కుడి*/
	type = css_computed_కుడి(box->style, &విలువ, &unit);
	ఒకవేళ (type == CSS_కుడి_సమితి) {
		ఒకవేళ (unit == CSS_UNIT_PCT) {
			*కుడి = FPCT_యొక్క_INT_TOINT(విలువ,
					containing_ఖండం->వెడల్పు);
		} లేకపోతే {
			*కుడి = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					విలువ, unit));
		}
	} లేకపోతే {
		*కుడి = AUTO;
	}

	/*శిఖరం*/
	type = css_computed_శిఖరం(box->style, &విలువ, &unit);
	ఒకవేళ (type == CSS_శిఖరం_సమితి) {
		ఒకవేళ (unit == CSS_UNIT_PCT) {
			*శిఖరం = FPCT_యొక్క_INT_TOINT(విలువ,
					containing_ఖండం->ఎత్తు);
		} లేకపోతే {
			*శిఖరం = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					విలువ, unit));
		}
	} లేకపోతే {
		*శిఖరం = AUTO;
	}

	/* bottom */
	type = css_computed_bottom(box->style, &విలువ, &unit);
	ఒకవేళ (type == CSS_BOTTOM_సమితి) {
		ఒకవేళ (unit == CSS_UNIT_PCT) {
			*bottom = FPCT_యొక్క_INT_TOINT(విలువ,
					containing_ఖండం->ఎత్తు);
		} లేకపోతే {
			*bottom = FIXTOINT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					విలువ, unit));
		}
	} లేకపోతే {
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
స్థిర bool
layout_absolute(నిర్మాణం box *box,
		నిర్మాణం box *containing_ఖండం,
		పూర్ణాంకం cx, పూర్ణాంకం cy,
		html_content *content)
{
	పూర్ణాంకం static_ఎడమ, static_శిఖరం;  /* static position */
	పూర్ణాంకం శిఖరం, కుడి, bottom, ఎడమ;
	పూర్ణాంకం వెడల్పు, ఎత్తు, max_వెడల్పు, min_వెడల్పు;
	పూర్ణాంకం *margin = box->margin;
	పూర్ణాంకం *padding = box->padding;
	నిర్మాణం box_border *border = box->border;
	పూర్ణాంకం అందుబాటు_వెడల్పు = containing_ఖండం->వెడల్పు;
	పూర్ణాంకం space;

	ప్రతిజ్ఞా(box->type == BOX_ఖండం || box->type == BOX_పట్టిక ||
			box->type == BOX_INLINE_ఖండం ||
			box->type == BOX_FLEX ||
			box->type == BOX_INLINE_FLEX);

	/* The static position is where the box would be if it was not
	 * absolutely positioned. The x and y are filled in by
	 * layout_block_context(). */
	static_ఎడమ = cx + box->x;
	static_శిఖరం = cy + box->y;

	ఒకవేళ (containing_ఖండం->type == BOX_ఖండం ||
			containing_ఖండం->type == BOX_INLINE_ఖండం ||
			containing_ఖండం->type == BOX_పట్టిక_CELL) {
		/* Block level container => temporarily increase containing
		 * block dimensions to include padding (we restore this
		 * again at the end) */
		containing_ఖండం->వెడల్పు += containing_ఖండం->padding[ఎడమ_2] +
				containing_ఖండం->padding[కుడి_2];
		containing_ఖండం->ఎత్తు += containing_ఖండం->padding[శిఖరం_2] +
				containing_ఖండం->padding[BOTTOM];
	}

	layout_compute_offsets(&content->unit_len_ctx, box, containing_ఖండం,
			&శిఖరం, &కుడి, &bottom, &ఎడమ);

	/* Pass containing block into layout_find_dimensions via the float
	 * containing block box member. This is unused for absolutely positioned
	 * boxes because a box can't be floated and absolutely positioned. */
	box->float_container = containing_ఖండం;
	layout_కనుగొను_dimensions(&content->unit_len_ctx, అందుబాటు_వెడల్పు, -1,
			box, box->style, &వెడల్పు, &ఎత్తు,
			&max_వెడల్పు, &min_వెడల్పు, 0, 0,
			margin, padding, border);
	box->float_container = ఖాళీ;

	/* 10.3.7 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ఎడమ, margin[ఎడమ_2], border[ఎడమ_2].వెడల్పు, padding[ఎడమ_2], వెడల్పు,
	      padding[కుడి_2], border[కుడి_2].వెడల్పు, margin[కుడి_2], కుడి,
	      containing_ఖండం->వెడల్పు);


	ఒకవేళ (ఎడమ == AUTO && వెడల్పు == AUTO && కుడి == AUTO) {
		ఒకవేళ (margin[ఎడమ_2] == AUTO)
			margin[ఎడమ_2] = 0;
		ఒకవేళ (margin[కుడి_2] == AUTO)
			margin[కుడి_2] = 0;
		ఎడమ = static_ఎడమ;

		వెడల్పు = min(max(box->min_వెడల్పు, అందుబాటు_వెడల్పు),
			box->max_వెడల్పు);
		వెడల్పు -= box->margin[ఎడమ_2] + box->border[ఎడమ_2].వెడల్పు +
			box->padding[ఎడమ_2] + box->padding[కుడి_2] +
			box->border[కుడి_2].వెడల్పు + box->margin[కుడి_2];

		/* Adjust for {min|max}-width */
		ఒకవేళ (max_వెడల్పు >= 0 && వెడల్పు > max_వెడల్పు) వెడల్పు = max_వెడల్పు;
		ఒకవేళ (వెడల్పు < min_వెడల్పు) వెడల్పు = min_వెడల్పు;

		కుడి = containing_ఖండం->వెడల్పు -
			ఎడమ -
			margin[ఎడమ_2] - border[ఎడమ_2].వెడల్పు - padding[ఎడమ_2] -
			వెడల్పు -
			padding[కుడి_2] - border[కుడి_2].వెడల్పు - margin[కుడి_2];
	} లేకపోతే ఒకవేళ (ఎడమ != AUTO && వెడల్పు != AUTO && కుడి != AUTO) {

		/* Adjust for {min|max}-width */
		ఒకవేళ (max_వెడల్పు >= 0 && వెడల్పు > max_వెడల్పు) వెడల్పు = max_వెడల్పు;
		ఒకవేళ (min_వెడల్పు >  0 && వెడల్పు < min_వెడల్పు) వెడల్పు = min_వెడల్పు;

		ఒకవేళ (margin[ఎడమ_2] == AUTO && margin[కుడి_2] == AUTO) {
			space = containing_ఖండం->వెడల్పు -
					ఎడమ - border[ఎడమ_2].వెడల్పు -
					padding[ఎడమ_2] - వెడల్పు - padding[కుడి_2] -
					border[కుడి_2].వెడల్పు - కుడి;
			ఒకవేళ (space < 0) {
				margin[ఎడమ_2] = 0;
				margin[కుడి_2] = space;
			} లేకపోతే {
				margin[ఎడమ_2] = margin[కుడి_2] = space / 2;
			}
		} లేకపోతే ఒకవేళ (margin[ఎడమ_2] == AUTO) {
			margin[ఎడమ_2] = containing_ఖండం->వెడల్పు -
					ఎడమ - border[ఎడమ_2].వెడల్పు -
					padding[ఎడమ_2] - వెడల్పు - padding[కుడి_2] -
					border[కుడి_2].వెడల్పు - margin[కుడి_2] -
					కుడి;
		} లేకపోతే ఒకవేళ (margin[కుడి_2] == AUTO) {
			margin[కుడి_2] = containing_ఖండం->వెడల్పు -
					ఎడమ - margin[ఎడమ_2] -
					border[ఎడమ_2].వెడల్పు -
					padding[ఎడమ_2] - వెడల్పు - padding[కుడి_2] -
					border[కుడి_2].వెడల్పు - కుడి;
		} లేకపోతే {
			కుడి = containing_ఖండం->వెడల్పు -
					ఎడమ - margin[ఎడమ_2] -
					border[ఎడమ_2].వెడల్పు -
					padding[ఎడమ_2] - వెడల్పు - padding[కుడి_2] -
					border[కుడి_2].వెడల్పు - margin[కుడి_2];
		}
	} లేకపోతే {
		ఒకవేళ (margin[ఎడమ_2] == AUTO)
			margin[ఎడమ_2] = 0;
		ఒకవేళ (margin[కుడి_2] == AUTO)
			margin[కుడి_2] = 0;

		ఒకవేళ (ఎడమ == AUTO && వెడల్పు == AUTO && కుడి != AUTO) {
			అందుబాటు_వెడల్పు -= కుడి;

			వెడల్పు = min(max(box->min_వెడల్పు, అందుబాటు_వెడల్పు),
				box->max_వెడల్పు);
			వెడల్పు -= box->margin[ఎడమ_2] + box->border[ఎడమ_2].వెడల్పు +
				box->padding[ఎడమ_2] + box->padding[కుడి_2] +
				box->border[కుడి_2].వెడల్పు + box->margin[కుడి_2];

			/* Adjust for {min|max}-width */
			ఒకవేళ (max_వెడల్పు >= 0 && వెడల్పు > max_వెడల్పు)
				వెడల్పు = max_వెడల్పు;
			ఒకవేళ (వెడల్పు < min_వెడల్పు)
				వెడల్పు = min_వెడల్పు;

			ఎడమ = containing_ఖండం->వెడల్పు -
					margin[ఎడమ_2] - border[ఎడమ_2].వెడల్పు -
					padding[ఎడమ_2] - వెడల్పు - padding[కుడి_2] -
					border[కుడి_2].వెడల్పు - margin[కుడి_2] -
					కుడి;
		} లేకపోతే ఒకవేళ (ఎడమ == AUTO && వెడల్పు != AUTO && కుడి == AUTO) {

			/* Adjust for {min|max}-width */
			ఒకవేళ (max_వెడల్పు >= 0 && వెడల్పు > max_వెడల్పు)
				వెడల్పు = max_వెడల్పు;
			ఒకవేళ (min_వెడల్పు >  0 && వెడల్పు < min_వెడల్పు)
				వెడల్పు = min_వెడల్పు;

			ఎడమ = static_ఎడమ;
			కుడి = containing_ఖండం->వెడల్పు -
					ఎడమ - margin[ఎడమ_2] -
					border[ఎడమ_2].వెడల్పు -
					padding[ఎడమ_2] - వెడల్పు - padding[కుడి_2] -
					border[కుడి_2].వెడల్పు - margin[కుడి_2];
		} లేకపోతే ఒకవేళ (ఎడమ != AUTO && వెడల్పు == AUTO && కుడి == AUTO) {
			అందుబాటు_వెడల్పు -= ఎడమ;

			వెడల్పు = min(max(box->min_వెడల్పు, అందుబాటు_వెడల్పు),
				box->max_వెడల్పు);
			వెడల్పు -= box->margin[ఎడమ_2] + box->border[ఎడమ_2].వెడల్పు +
				box->padding[ఎడమ_2] + box->padding[కుడి_2] +
				box->border[కుడి_2].వెడల్పు + box->margin[కుడి_2];

			/* Adjust for {min|max}-width */
			ఒకవేళ (max_వెడల్పు >= 0 && వెడల్పు > max_వెడల్పు)
				వెడల్పు = max_వెడల్పు;
			ఒకవేళ (వెడల్పు < min_వెడల్పు)
				వెడల్పు = min_వెడల్పు;

			కుడి = containing_ఖండం->వెడల్పు -
					ఎడమ - margin[ఎడమ_2] -
					border[ఎడమ_2].వెడల్పు -
					padding[ఎడమ_2] - వెడల్పు - padding[కుడి_2] -
					border[కుడి_2].వెడల్పు - margin[కుడి_2];
		} లేకపోతే ఒకవేళ (ఎడమ == AUTO && వెడల్పు != AUTO && కుడి != AUTO) {

			/* Adjust for {min|max}-width */
			ఒకవేళ (max_వెడల్పు >= 0 && వెడల్పు > max_వెడల్పు)
				వెడల్పు = max_వెడల్పు;
			ఒకవేళ (వెడల్పు < min_వెడల్పు)
				వెడల్పు = min_వెడల్పు;

			ఎడమ = containing_ఖండం->వెడల్పు -
					margin[ఎడమ_2] - border[ఎడమ_2].వెడల్పు -
					padding[ఎడమ_2] - వెడల్పు - padding[కుడి_2] -
					border[కుడి_2].వెడల్పు - margin[కుడి_2] -
					కుడి;
		} లేకపోతే ఒకవేళ (ఎడమ != AUTO && వెడల్పు == AUTO && కుడి != AUTO) {
			వెడల్పు = containing_ఖండం->వెడల్పు -
					ఎడమ - margin[ఎడమ_2] -
					border[ఎడమ_2].వెడల్పు -
					padding[ఎడమ_2] - padding[కుడి_2] -
					border[కుడి_2].వెడల్పు - margin[కుడి_2] -
					కుడి;

			/* Adjust for {min|max}-width */
			ఒకవేళ (max_వెడల్పు >= 0 && వెడల్పు > max_వెడల్పు)
				వెడల్పు = max_వెడల్పు;
			ఒకవేళ (వెడల్పు < min_వెడల్పు)
				వెడల్పు = min_వెడల్పు;

		} లేకపోతే ఒకవేళ (ఎడమ != AUTO && వెడల్పు != AUTO && కుడి == AUTO) {

			/* Adjust for {min|max}-width */
			ఒకవేళ (max_వెడల్పు >= 0 && వెడల్పు > max_వెడల్పు)
				వెడల్పు = max_వెడల్పు;
			ఒకవేళ (వెడల్పు < min_వెడల్పు)
				వెడల్పు = min_వెడల్పు;

			కుడి = containing_ఖండం->వెడల్పు -
					ఎడమ - margin[ఎడమ_2] -
					border[ఎడమ_2].వెడల్పు -
					padding[ఎడమ_2] - వెడల్పు - padding[కుడి_2] -
					border[కుడి_2].వెడల్పు - margin[కుడి_2];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      ఎడమ, margin[ఎడమ_2], border[ఎడమ_2].వెడల్పు, padding[ఎడమ_2], వెడల్పు,
	      padding[కుడి_2], border[కుడి_2].వెడల్పు, margin[కుడి_2], కుడి,
	      containing_ఖండం->వెడల్పు);

	box->x = ఎడమ + margin[ఎడమ_2] + border[ఎడమ_2].వెడల్పు - cx;
	ఒకవేళ (containing_ఖండం->type == BOX_ఖండం ||
			containing_ఖండం->type == BOX_INLINE_ఖండం ||
			containing_ఖండం->type == BOX_పట్టిక_CELL) {
		/* Block-level ancestor => reset container's width */
		containing_ఖండం->వెడల్పు -= containing_ఖండం->padding[ఎడమ_2] +
				containing_ఖండం->padding[కుడి_2];
	} లేకపోతే {
		/** \todo inline ancestors */
	}
	box->వెడల్పు = వెడల్పు;
	box->ఎత్తు = ఎత్తు;

	ఒకవేళ (box->type == BOX_ఖండం || box->type == BOX_INLINE_ఖండం ||
			box->object || box->flags & IFRAME) {
		ఒకవేళ (!layout_ఖండం_context(box, -1, content))
			తిరిగి false;
	} లేకపోతే ఒకవేళ (box->type == BOX_పట్టిక) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_ఖండం;
		/* \todo  layout_table considers margins etc. again */
		ఒకవేళ (!layout_పట్టిక(box, వెడల్పు, content))
			తిరిగి false;
		box->float_container = ఖాళీ;
		layout_solve_వెడల్పు(box, box->మాతృ->వెడల్పు, box->వెడల్పు, 0, 0,
				-1, -1);
	} లేకపోతే ఒకవేళ (box->type == BOX_FLEX || box->type == BOX_INLINE_FLEX) {
		/* layout_table also expects the containing block to be
		 * stored in the float_container field */
		box->float_container = containing_ఖండం;
		ఒకవేళ (!layout_flex(box, వెడల్పు, content))
			తిరిగి false;
		box->float_container = ఖాళీ;
	}

	/* 10.6.4 */
	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      శిఖరం, margin[శిఖరం_2], border[శిఖరం_2].వెడల్పు, padding[శిఖరం_2], ఎత్తు,
	      padding[BOTTOM], border[BOTTOM].వెడల్పు, margin[BOTTOM], bottom,
	      containing_ఖండం->ఎత్తు);

	ఒకవేళ (శిఖరం == AUTO && ఎత్తు == AUTO && bottom == AUTO) {
		శిఖరం = static_శిఖరం;
		ఎత్తు = box->ఎత్తు;
		ఒకవేళ (margin[శిఖరం_2] == AUTO)
			margin[శిఖరం_2] = 0;
		ఒకవేళ (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		bottom = containing_ఖండం->ఎత్తు -
				శిఖరం - margin[శిఖరం_2] - border[శిఖరం_2].వెడల్పు -
				padding[శిఖరం_2] - ఎత్తు - padding[BOTTOM] -
				border[BOTTOM].వెడల్పు - margin[BOTTOM];
	} లేకపోతే ఒకవేళ (శిఖరం != AUTO && ఎత్తు != AUTO && bottom != AUTO) {
		ఒకవేళ (margin[శిఖరం_2] == AUTO && margin[BOTTOM] == AUTO) {
			space = containing_ఖండం->ఎత్తు -
					శిఖరం - border[శిఖరం_2].వెడల్పు - padding[శిఖరం_2] -
					ఎత్తు - padding[BOTTOM] -
					border[BOTTOM].వెడల్పు - bottom;
			margin[శిఖరం_2] = margin[BOTTOM] = space / 2;
		} లేకపోతే ఒకవేళ (margin[శిఖరం_2] == AUTO) {
			margin[శిఖరం_2] = containing_ఖండం->ఎత్తు -
					శిఖరం - border[శిఖరం_2].వెడల్పు - padding[శిఖరం_2] -
					ఎత్తు - padding[BOTTOM] -
					border[BOTTOM].వెడల్పు - margin[BOTTOM] -
					bottom;
		} లేకపోతే ఒకవేళ (margin[BOTTOM] == AUTO) {
			margin[BOTTOM] = containing_ఖండం->ఎత్తు -
					శిఖరం - margin[శిఖరం_2] - border[శిఖరం_2].వెడల్పు -
					padding[శిఖరం_2] - ఎత్తు -
					padding[BOTTOM] - border[BOTTOM].వెడల్పు -
					bottom;
		} లేకపోతే {
			bottom = containing_ఖండం->ఎత్తు -
					శిఖరం - margin[శిఖరం_2] - border[శిఖరం_2].వెడల్పు -
					padding[శిఖరం_2] - ఎత్తు -
					padding[BOTTOM] - border[BOTTOM].వెడల్పు -
					margin[BOTTOM];
		}
	} లేకపోతే {
		ఒకవేళ (margin[శిఖరం_2] == AUTO)
			margin[శిఖరం_2] = 0;
		ఒకవేళ (margin[BOTTOM] == AUTO)
			margin[BOTTOM] = 0;
		ఒకవేళ (శిఖరం == AUTO && ఎత్తు == AUTO && bottom != AUTO) {
			ఎత్తు = box->ఎత్తు;
			శిఖరం = containing_ఖండం->ఎత్తు -
					margin[శిఖరం_2] - border[శిఖరం_2].వెడల్పు -
					padding[శిఖరం_2] - ఎత్తు -
					padding[BOTTOM] - border[BOTTOM].వెడల్పు -
					margin[BOTTOM] - bottom;
		} లేకపోతే ఒకవేళ (శిఖరం == AUTO && ఎత్తు != AUTO && bottom == AUTO) {
			శిఖరం = static_శిఖరం;
			bottom = containing_ఖండం->ఎత్తు -
					శిఖరం - margin[శిఖరం_2] - border[శిఖరం_2].వెడల్పు -
					padding[శిఖరం_2] - ఎత్తు -
					padding[BOTTOM] - border[BOTTOM].వెడల్పు -
					margin[BOTTOM];
		} లేకపోతే ఒకవేళ (శిఖరం != AUTO && ఎత్తు == AUTO && bottom == AUTO) {
			ఎత్తు = box->ఎత్తు;
			bottom = containing_ఖండం->ఎత్తు -
					శిఖరం - margin[శిఖరం_2] - border[శిఖరం_2].వెడల్పు -
					padding[శిఖరం_2] - ఎత్తు -
					padding[BOTTOM] - border[BOTTOM].వెడల్పు -
					margin[BOTTOM];
		} లేకపోతే ఒకవేళ (శిఖరం == AUTO && ఎత్తు != AUTO && bottom != AUTO) {
			శిఖరం = containing_ఖండం->ఎత్తు -
					margin[శిఖరం_2] - border[శిఖరం_2].వెడల్పు -
					padding[శిఖరం_2] - ఎత్తు -
					padding[BOTTOM] - border[BOTTOM].వెడల్పు -
					margin[BOTTOM] - bottom;
		} లేకపోతే ఒకవేళ (శిఖరం != AUTO && ఎత్తు == AUTO && bottom != AUTO) {
			ఎత్తు = containing_ఖండం->ఎత్తు -
					శిఖరం - margin[శిఖరం_2] - border[శిఖరం_2].వెడల్పు -
					padding[శిఖరం_2] - padding[BOTTOM] -
					border[BOTTOM].వెడల్పు - margin[BOTTOM] -
					bottom;
		} లేకపోతే ఒకవేళ (శిఖరం != AUTO && ఎత్తు != AUTO && bottom == AUTO) {
			bottom = containing_ఖండం->ఎత్తు -
					శిఖరం - margin[శిఖరం_2] - border[శిఖరం_2].వెడల్పు -
					padding[శిఖరం_2] - ఎత్తు -
					padding[BOTTOM] - border[BOTTOM].వెడల్పు -
					margin[BOTTOM];
		}
	}

	NSLOG(layout, DEBUG,
	      "%i + %i + %i + %i + %i + %i + %i + %i + %i = %i",
	      శిఖరం, margin[శిఖరం_2], border[శిఖరం_2].వెడల్పు, padding[శిఖరం_2], ఎత్తు,
	      padding[BOTTOM], border[BOTTOM].వెడల్పు, margin[BOTTOM], bottom,
	      containing_ఖండం->ఎత్తు);

	box->y = శిఖరం + margin[శిఖరం_2] + border[శిఖరం_2].వెడల్పు - cy;
	ఒకవేళ (containing_ఖండం->type == BOX_ఖండం ||
			containing_ఖండం->type == BOX_INLINE_ఖండం ||
			containing_ఖండం->type == BOX_పట్టిక_CELL) {
		/* Block-level ancestor => reset container's height */
		containing_ఖండం->ఎత్తు -= containing_ఖండం->padding[శిఖరం_2] +
				containing_ఖండం->padding[BOTTOM];
	} లేకపోతే {
		/** \todo Inline ancestors */
	}
	box->ఎత్తు = ఎత్తు;
	layout_apply_minmax_ఎత్తు(&content->unit_len_ctx, box, containing_ఖండం);

	తిరిగి true;
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
స్థిర bool
layout_position_absolute(నిర్మాణం box *box,
			 నిర్మాణం box *containing_ఖండం,
			 పూర్ణాంకం cx, పూర్ణాంకం cy,
			 html_content *content)
{
	నిర్మాణం box *c;

	కొరకు (c = box->children; c; c = c->తదుపరి) {
		ఒకవేళ ((c->type == BOX_ఖండం || c->type == BOX_పట్టిక ||
				c->type == BOX_INLINE_ఖండం ||
				c->type == BOX_FLEX ||
				c->type == BOX_INLINE_FLEX) &&
				(css_computed_position(c->style) ==
						CSS_POSITION_ABSOLUTE ||
				 css_computed_position(c->style) ==
						CSS_POSITION_FIXED)) {
			ఒకవేళ (!layout_absolute(c, containing_ఖండం,
					cx, cy, content))
				తిరిగి false;
			ఒకవేళ (!layout_position_absolute(c, c, 0, 0, content))
				తిరిగి false;
		} లేకపోతే ఒకవేళ (c->style && css_computed_position(c->style) ==
				CSS_POSITION_RELATIVE) {
			ఒకవేళ (!layout_position_absolute(c, c, 0, 0, content))
				తిరిగి false;
		} లేకపోతే {
			పూర్ణాంకం px, py;
			ఒకవేళ (c->style && (css_computed_float(c->style) ==
						CSS_FLOAT_ఎడమ ||
					css_computed_float(c->style) ==
						CSS_FLOAT_కుడి)) {
				/* Float x/y coords are relative to nearest
				 * ansestor with float_children, rather than
				 * relative to parent. Need to get x/y relative
				 * to parent */
				నిర్మాణం box *p;
				px = c->x;
				py = c->y;
				కొరకు (p = box->మాతృ; p && !p->float_children;
						p = p->మాతృ) {
					px -= p->x;
					py -= p->y;
				}
			} లేకపోతే {
				/* Not a float, so box x/y coords are relative
				 * to parent */
				px = c->x;
				py = c->y;
			}
			ఒకవేళ (!layout_position_absolute(c, containing_ఖండం,
					cx + px, cy + py, content))
				తిరిగి false;
		}
	}

	తిరిగి true;
}


/**
 * Compute a box's relative offset as per CSS 2.1 9.4.3
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box	Box to compute relative offsets for.
 * \param  x	Receives relative offset in x.
 * \param  y	Receives relative offset in y.
 */
స్థిర శూన్య layout_compute_relative_స్థానభ్రంశం(
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		నిర్మాణం box *box,
		పూర్ణాంకం *x,
		పూర్ణాంకం *y)
{
	పూర్ణాంకం ఎడమ, కుడి, శిఖరం, bottom;
	నిర్మాణం box *containing_ఖండం;

	ప్రతిజ్ఞా(box && box->మాతృ && box->style &&
			css_computed_position(box->style) ==
			CSS_POSITION_RELATIVE);

	ఒకవేళ (box->float_container &&
			(css_computed_float(box->style) == CSS_FLOAT_ఎడమ ||
			css_computed_float(box->style) == CSS_FLOAT_కుడి)) {
		containing_ఖండం = box->float_container;
	} లేకపోతే {
		containing_ఖండం = box->మాతృ;
	}

	layout_compute_offsets(unit_len_ctx, box, containing_ఖండం,
			&శిఖరం, &కుడి, &bottom, &ఎడమ);

	ఒకవేళ (ఎడమ == AUTO && కుడి == AUTO)
		ఎడమ = కుడి = 0;
	లేకపోతే ఒకవేళ (ఎడమ == AUTO)
		/* left is auto => computed = -right */
		ఎడమ = -కుడి;
	లేకపోతే ఒకవేళ (కుడి == AUTO)
		/* right is auto => computed = -left */
		కుడి = -ఎడమ;
	లేకపోతే {
		/* over constrained => examine direction property
		 * of containing block */
		ఒకవేళ (containing_ఖండం->style &&
				css_computed_direction(
				containing_ఖండం->style) ==
				CSS_DIRECTION_RTL) {
			/* right wins */
			ఎడమ = -కుడి;
		} లేకపోతే {
			/* assume LTR in all other cases */
			కుడి = -ఎడమ;
		}
	}

	ప్రతిజ్ఞా(ఎడమ == -కుడి);

	ఒకవేళ (శిఖరం == AUTO && bottom == AUTO) {
		శిఖరం = bottom = 0;
	} లేకపోతే ఒకవేళ (శిఖరం == AUTO) {
		శిఖరం = -bottom;
	} లేకపోతే {
		/* bottom is AUTO, or neither are AUTO */
		bottom = -శిఖరం;
	}

	NSLOG(layout, DEBUG,  "left %i, right %i, top %i, bottom %i", ఎడమ,
	      కుడి, శిఖరం, bottom);

	*x = ఎడమ;
	*y = శిఖరం;
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
స్థిర శూన్య
layout_position_relative(
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		నిర్మాణం box *మూలం,
		నిర్మాణం box *fp,
		పూర్ణాంకం fx,
		పూర్ణాంకం fy)
{
	నిర్మాణం box *box; /* for children of "root" */
	నిర్మాణం box *fn;  /* for block formatting context box for children of
			  * "box" */
	నిర్మాణం box *fc;  /* for float children of the block formatting context,
			  * "fp" */
	పూర్ణాంకం x, y;	 /* for the offsets resulting from any relative
			  * positioning on the current block */
	పూర్ణాంకం fnx, fny;    /* for affsets which apply to flat children of "box" */

	/**\todo ensure containing box is large enough after moving boxes */

	ప్రతిజ్ఞా(మూలం);

	/* Normal children */
	కొరకు (box = మూలం->children; box; box = box->తదుపరి) {

		ఒకవేళ (box->type == BOX_పాఠ్యం)
			కొనసాగు;

		/* If relatively positioned, get offsets */
		ఒకవేళ (box->style && css_computed_position(box->style) ==
				CSS_POSITION_RELATIVE)
			layout_compute_relative_స్థానభ్రంశం(
					unit_len_ctx, box, &x, &y);
		లేకపోతే
			x = y = 0;

		/* Adjust float coordinates.
		 * (note float x and y are relative to their block formatting
		 * context box and not their parent) */
		ఒకవేళ (box->style && (css_computed_float(box->style) ==
					CSS_FLOAT_ఎడమ ||
				css_computed_float(box->style) ==
					CSS_FLOAT_కుడి) &&
				(fx != 0 || fy != 0)) {
			/* box is a float and there is a float offset to
			 * apply */
			కొరకు (fc = fp->float_children; fc; fc = fc->తదుపరి_float) {
				ఒకవేళ (box == fc->children) {
					/* Box is floated in the block
					 * formatting context block, fp.
					 * Apply float offsets. */
					box->x += fx;
					box->y += fy;
					fx = fy = 0;
				}
			}
		}

		ఒకవేళ (box->float_children) {
			fn = box;
			fnx = fny = 0;
		} లేకపోతే {
			fn = fp;
			fnx = fx + x;
			fny = fy + y;
		}

		/* recurse first */
		layout_position_relative(unit_len_ctx, box, fn, fnx, fny);

		/* Ignore things we're not interested in. */
		ఒకవేళ (!box->style || (box->style &&
				css_computed_position(box->style) !=
				CSS_POSITION_RELATIVE))
			కొనసాగు;

		box->x += x;
		box->y += y;

		/* Handle INLINEs - their "children" are in fact
		 * the sibling boxes between the INLINE and
		 * INLINE_END boxes */
		ఒకవేళ (box->type == BOX_INLINE && box->inline_చివర) {
			నిర్మాణం box *b;
			కొరకు (b = box->తదుపరి; b && b != box->inline_చివర;
					b = b->తదుపరి) {
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
స్థిర శూన్య
layout_get_box_bbox(
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		నిర్మాణం box *box,
		పూర్ణాంకం *desc_x0, పూర్ణాంకం *desc_y0,
		పూర్ణాంకం *desc_x1, పూర్ణాంకం *desc_y1)
{
	*desc_x0 = -box->border[ఎడమ_2].వెడల్పు;
	*desc_y0 = -box->border[శిఖరం_2].వెడల్పు;
	*desc_x1 = box->padding[ఎడమ_2] + box->వెడల్పు + box->padding[కుడి_2] +
			box->border[కుడి_2].వెడల్పు;
	*desc_y1 = box->padding[శిఖరం_2] + box->ఎత్తు + box->padding[BOTTOM] +
			box->border[BOTTOM].వెడల్పు;

	/* To stop the top of text getting clipped when css line-height is
	 * reduced, we increase the top of the descendant bbox. */
	ఒకవేళ (box->type == BOX_ఖండం && box->style != ఖాళీ &&
			css_computed_అధికప్రవాహం_y(box->style) ==
					CSS_అధికప్రవాహం_VISIBLE &&
			box->object == ఖాళీ) {
		css_fixed font_కొలత = 0;
		css_unit font_unit = CSS_UNIT_PT;
		పూర్ణాంకం పాఠ్యం_ఎత్తు;

		css_computed_font_కొలత(box->style, &font_కొలత, &font_unit);
		పాఠ్యం_ఎత్తు = css_unit_len2device_px(box->style, unit_len_ctx,
				font_కొలత, font_unit);
		పాఠ్యం_ఎత్తు = FIXTOINT(పాఠ్యం_ఎత్తు * 3 / 4);
		*desc_y0 = (*desc_y0 < -పాఠ్యం_ఎత్తు) ? *desc_y0 : -పాఠ్యం_ఎత్తు;
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
స్థిర శూన్య
layout_update_descendant_bbox(
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		నిర్మాణం box *box,
		నిర్మాణం box *బిడ్డ,
		పూర్ణాంకం off_x,
		పూర్ణాంకం off_y)
{
	పూర్ణాంకం బిడ్డ_desc_x0, బిడ్డ_desc_y0, బిడ్డ_desc_x1, బిడ్డ_desc_y1;

	/* get coordinates of child relative to box */
	పూర్ణాంకం బిడ్డ_x = బిడ్డ->x - off_x;
	పూర్ణాంకం బిడ్డ_y = బిడ్డ->y - off_y;

	bool html_object = (బిడ్డ->object &&
			content_get_type(బిడ్డ->object) == CONTENT_HTML);

	గణన css_అధికప్రవాహం_e అధికప్రవాహం_x = CSS_అధికప్రవాహం_VISIBLE;
	గణన css_అధికప్రవాహం_e అధికప్రవాహం_y = CSS_అధికప్రవాహం_VISIBLE;

	ఒకవేళ (బిడ్డ->style != ఖాళీ) {
		అధికప్రవాహం_x = css_computed_అధికప్రవాహం_x(బిడ్డ->style);
		అధికప్రవాహం_y = css_computed_అధికప్రవాహం_y(బిడ్డ->style);
	}

	/* Get child's border edge */
	layout_get_box_bbox(unit_len_ctx, బిడ్డ,
			&బిడ్డ_desc_x0, &బిడ్డ_desc_y0,
			&బిడ్డ_desc_x1, &బిడ్డ_desc_y1);

	ఒకవేళ (అధికప్రవాహం_x == CSS_అధికప్రవాహం_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		బిడ్డ_desc_x0 = బిడ్డ->descendant_x0;
		బిడ్డ_desc_x1 = బిడ్డ->descendant_x1;
	}
	ఒకవేళ (అధికప్రవాహం_y == CSS_అధికప్రవాహం_VISIBLE &&
			html_object == false) {
		/* get child's descendant bbox relative to box */
		బిడ్డ_desc_y0 = బిడ్డ->descendant_y0;
		బిడ్డ_desc_y1 = బిడ్డ->descendant_y1;
	}

	బిడ్డ_desc_x0 += బిడ్డ_x;
	బిడ్డ_desc_y0 += బిడ్డ_y;
	బిడ్డ_desc_x1 += బిడ్డ_x;
	బిడ్డ_desc_y1 += బిడ్డ_y;

	/* increase box's descendant bbox to contain descendants */
	ఒకవేళ (బిడ్డ_desc_x0 < box->descendant_x0)
		box->descendant_x0 = బిడ్డ_desc_x0;
	ఒకవేళ (బిడ్డ_desc_y0 < box->descendant_y0)
		box->descendant_y0 = బిడ్డ_desc_y0;
	ఒకవేళ (box->descendant_x1 < బిడ్డ_desc_x1)
		box->descendant_x1 = బిడ్డ_desc_x1;
	ఒకవేళ (box->descendant_y1 < బిడ్డ_desc_y1)
		box->descendant_y1 = బిడ్డ_desc_y1;
}


/**
 * Recursively calculate the descendant_[xy][01] values for a laid-out box tree
 * and inform iframe browser windows of their size and position.
 *
 * \param  unit_len_ctx  Length conversion context
 * \param  box      tree of boxes to update
 */
స్థిర శూన్య layout_calculate_descendant_bboxes(
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		నిర్మాణం box *box)
{
	నిర్మాణం box *బిడ్డ;

	ప్రతిజ్ఞా(box->వెడల్పు != UNKNOWN_వెడల్పు);
	ప్రతిజ్ఞా(box->ఎత్తు != AUTO);
	/* assert((box->width >= 0) && (box->height >= 0)); */

	/* Initialise box's descendant box to border edge box */
	layout_get_box_bbox(unit_len_ctx, box,
			&box->descendant_x0, &box->descendant_y0,
			&box->descendant_x1, &box->descendant_y1);

	/* Extend it to contain HTML contents if box is replaced */
	ఒకవేళ (box->object && content_get_type(box->object) == CONTENT_HTML) {
		ఒకవేళ (box->descendant_x1 < content_get_వెడల్పు(box->object))
			box->descendant_x1 = content_get_వెడల్పు(box->object);
		ఒకవేళ (box->descendant_y1 < content_get_ఎత్తు(box->object))
			box->descendant_y1 = content_get_ఎత్తు(box->object);
	}

	ఒకవేళ (box->iframe != ఖాళీ) {
		పూర్ణాంకం x, y;
		box_coords(box, &x, &y);

		browser_కిటికీ_సమితి_position(box->iframe, x, y);
		browser_కిటికీ_సమితి_dimensions(box->iframe,
				box->వెడల్పు, box->ఎత్తు);
		browser_కిటికీ_reformat(box->iframe, true,
				box->వెడల్పు, box->ఎత్తు);
	}

	ఒకవేళ (box->type == BOX_INLINE || box->type == BOX_పాఠ్యం)
		తిరిగి;

	ఒకవేళ (box->type == BOX_INLINE_చివర) {
		box = box->inline_చివర;
		కొరకు (బిడ్డ = box->తదుపరి; బిడ్డ;
				బిడ్డ = బిడ్డ->తదుపరి) {
			ఒకవేళ (బిడ్డ->type == BOX_FLOAT_ఎడమ ||
					బిడ్డ->type == BOX_FLOAT_కుడి)
				కొనసాగు;

			layout_update_descendant_bbox(unit_len_ctx, box, బిడ్డ,
					box->x, box->y);

			ఒకవేళ (బిడ్డ == box->inline_చివర)
				ఆపు;
		}
		తిరిగి;
	}

	ఒకవేళ (box->flags & REPLACE_DIM)
		/* Box's children aren't displayed if the box is replaced */
		తిరిగి;

	కొరకు (బిడ్డ = box->children; బిడ్డ; బిడ్డ = బిడ్డ->తదుపరి) {
		ఒకవేళ (బిడ్డ->type == BOX_FLOAT_ఎడమ ||
				బిడ్డ->type == BOX_FLOAT_కుడి)
			కొనసాగు;

		layout_calculate_descendant_bboxes(unit_len_ctx, బిడ్డ);

		ఒకవేళ (box->style && css_computed_అధికప్రవాహం_x(box->style) ==
				CSS_అధికప్రవాహం_HIDDEN &&
				css_computed_అధికప్రవాహం_y(box->style) ==
				CSS_అధికప్రవాహం_HIDDEN)
			కొనసాగు;

		layout_update_descendant_bbox(unit_len_ctx, box, బిడ్డ, 0, 0);
	}

	కొరకు (బిడ్డ = box->float_children; బిడ్డ; బిడ్డ = బిడ్డ->తదుపరి_float) {
		ప్రతిజ్ఞా(బిడ్డ->type == BOX_FLOAT_ఎడమ ||
				బిడ్డ->type == BOX_FLOAT_కుడి);

		layout_calculate_descendant_bboxes(unit_len_ctx, బిడ్డ);

		layout_update_descendant_bbox(unit_len_ctx, box, బిడ్డ, 0, 0);
	}

	ఒకవేళ (box->list_marker) {
		బిడ్డ = box->list_marker;
		layout_calculate_descendant_bboxes(unit_len_ctx, బిడ్డ);

		layout_update_descendant_bbox(unit_len_ctx, box, బిడ్డ, 0, 0);
	}
}


/* exported function documented in html/layout.h */
bool layout_document(html_content *content, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు)
{
	bool ret;
	నిర్మాణం box *doc = content->layout;
	స్థిరరాశి నిర్మాణం gui_layout_పట్టిక *font_func = content->font_func;

	NSLOG(layout, DEBUG, "Doing layout to %ix%i of %s",
			వెడల్పు, ఎత్తు, nsurl_access(content_get_url(
					&content->పునాది)));

	layout_minmax_ఖండం(doc, font_func, content);

	layout_ఖండం_కనుగొను_dimensions(&content->unit_len_ctx,
			వెడల్పు, ఎత్తు, 0, 0, doc);
	doc->x = doc->margin[ఎడమ_2] + doc->border[ఎడమ_2].వెడల్పు;
	doc->y = doc->margin[శిఖరం_2] + doc->border[శిఖరం_2].వెడల్పు;
	వెడల్పు -= doc->margin[ఎడమ_2] + doc->border[ఎడమ_2].వెడల్పు +
			doc->padding[ఎడమ_2] + doc->padding[కుడి_2] +
			doc->border[కుడి_2].వెడల్పు + doc->margin[కుడి_2];
	ఒకవేళ (వెడల్పు < 0) {
		వెడల్పు = 0;
	}
	doc->వెడల్పు = వెడల్పు;

	ret = layout_ఖండం_context(doc, ఎత్తు, content);

	/* make <html> and <body> fill available height */
	ఒకవేళ (doc->y + doc->padding[శిఖరం_2] + doc->ఎత్తు + doc->padding[BOTTOM] +
			doc->border[BOTTOM].వెడల్పు + doc->margin[BOTTOM] <
			ఎత్తు) {
		doc->ఎత్తు = ఎత్తు - (doc->y + doc->padding[శిఖరం_2] +
				doc->padding[BOTTOM] +
				doc->border[BOTTOM].వెడల్పు +
				doc->margin[BOTTOM]);
		ఒకవేళ (doc->children)
			doc->children->ఎత్తు = doc->ఎత్తు -
					(doc->children->margin[శిఖరం_2] +
					 doc->children->border[శిఖరం_2].వెడల్పు +
					 doc->children->padding[శిఖరం_2] +
					 doc->children->padding[BOTTOM] +
					 doc->children->border[BOTTOM].వెడల్పు +
					 doc->children->margin[BOTTOM]);
	}

	layout_lists(content, doc);
	layout_position_absolute(doc, doc, 0, 0, content);
	layout_position_relative(&content->unit_len_ctx, doc, doc, 0, 0);

	layout_calculate_descendant_bboxes(&content->unit_len_ctx, doc);

	తిరిగి ret;
}
