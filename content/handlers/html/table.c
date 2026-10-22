/*
 * Copyright 2005 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2005 Richard Wilson <info@tinct.net>
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
 * implementation of HTML table processing and layout.
 */

#అన్తర్భావ <assert.h>
#అన్తర్భావ <dom/dom.h>

#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/talloc.h"
#అన్తర్భావ "css/utils.h"

#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/table.h"

/* Define to enable verbose table debug */
#నిర్వచనం_రద్దు పట్టిక_DEBUG

/**
 * Container for border values during table border calculations
 */
నిర్మాణం border {
	గణన css_border_style_e style;	/**< border-style */
	గణన css_border_color_e color;	/**< border-color type */
	css_color c;			/**< border-color value */
	css_fixed వెడల్పు;		/**< border-width length */
	css_unit unit;			/**< border-width units */
};


/**
 * Determine if a border style is more eyecatching than another
 *
 * \param unit_len_ctx  Length conversion context
 * \param a        Reference border style
 * \param a_src    Source of \a a
 * \param b        Candidate border style
 * \param b_src    Source of \a b
 * \return True if \a b is more eyecatching than \a a
 */
స్థిర bool
పట్టిక_border_is_మరిన్ని_eyecatching(స్థిరరాశి css_unit_ctx *unit_len_ctx,
				 స్థిరరాశి నిర్మాణం border *a,
				 box_type a_src,
				 స్థిరరాశి నిర్మాణం border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	పూర్ణాంకం impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	ఒకవేళ (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		తిరిగి false;

	ఒకవేళ (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		తిరిగి true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	ప్రతిజ్ఞా(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	ప్రతిజ్ఞా(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(ఖాళీ, unit_len_ctx, a->వెడల్పు, a->unit);
	bwidth = css_unit_len2device_px(ఖాళీ, unit_len_ctx, b->వెడల్పు, b->unit);

	ఒకవేళ (awidth < bwidth)
		తిరిగి true;
	లేకపోతే ఒకవేళ (bwidth < awidth)
		తిరిగి false;

	/* 3b -- sort by style */
	ఎంపిక (a->style) {
	స్థితి CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	స్థితి CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	స్థితి CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	స్థితి CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	స్థితి CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	స్థితి CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	స్థితి CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	స్థితి CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	అప్రమేయ:
		ఆపు;
	}

	ఎంపిక (b->style) {
	స్థితి CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	స్థితి CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	స్థితి CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	స్థితి CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	స్థితి CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	స్థితి CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	స్థితి CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	స్థితి CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	అప్రమేయ:
		ఆపు;
	}

	ఒకవేళ (impact < 0)
		తిరిగి true;
	లేకపోతే ఒకవేళ (impact > 0)
		తిరిగి false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	ఎంపిక (a_src) {
	స్థితి BOX_పట్టిక_CELL:       impact++; fallthrough;
	స్థితి BOX_పట్టిక_అడ్డువరుస:        impact++; fallthrough;
	స్థితి BOX_పట్టిక_అడ్డువరుస_సమూహం:  impact++; fallthrough;
	స్థితి BOX_పట్టిక:            impact++; fallthrough;
	అప్రమేయ:
		ఆపు;
	}

	/** \todo COL/COL_GROUP */
	ఎంపిక (b_src) {
	స్థితి BOX_పట్టిక_CELL:       impact--; fallthrough;
	స్థితి BOX_పట్టిక_అడ్డువరుస:        impact--; fallthrough;
	స్థితి BOX_పట్టిక_అడ్డువరుస_సమూహం:  impact--; fallthrough;
	స్థితి BOX_పట్టిక:            impact--; fallthrough;
	అప్రమేయ:
		ఆపు;
	}

	ఒకవేళ (impact < 0)
		తిరిగి true;
	లేకపోతే ఒకవేళ (impact > 0)
		తిరిగి false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	తిరిగి true;
}


/**
 * Process a table
 *
 * \param unit_len_ctx  Length conversion context
 * \param table    Table to process
 * \param a        Current border style for cell
 * \param a_src    Source of \a a
 *
 * \post \a a will be updated with most eyecatching style
 * \post \a a_src will be updated also
 */
స్థిర శూన్య
పట్టిక_cell_శిఖరం_process_పట్టిక(స్థిరరాశి css_unit_ctx *unit_len_ctx,
			     నిర్మాణం box *పట్టిక,
			     నిర్మాణం border *a,
			     box_type *a_src)
{
	నిర్మాణం border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_శిఖరం_style(పట్టిక->style);
	b.color = css_computed_border_శిఖరం_color(పట్టిక->style, &b.c);
	css_computed_border_శిఖరం_వెడల్పు(పట్టిక->style, &b.వెడల్పు, &b.unit);
	b.వెడల్పు = css_unit_len2device_px(పట్టిక->style, unit_len_ctx,
			b.వెడల్పు, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_పట్టిక;

	ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}
}


/**
 * Process a row
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Cell being considered
 * \param row      Row to process
 * \param a        Current border style for cell
 * \param a_src    Source of \a a
 * \return true if row has cells, false otherwise
 *
 * \post \a a will be updated with most eyecatching style
 * \post \a a_src will be updated also
 */
స్థిర bool
పట్టిక_cell_శిఖరం_process_అడ్డువరుస(స్థిరరాశి css_unit_ctx *unit_len_ctx,
			   నిర్మాణం box *cell,
			   నిర్మాణం box *అడ్డువరుస,
			   నిర్మాణం border *a,
			   box_type *a_src)
{
	నిర్మాణం border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(అడ్డువరుస->style);
	b.color = css_computed_border_bottom_color(అడ్డువరుస->style, &b.c);
	css_computed_border_bottom_వెడల్పు(అడ్డువరుస->style, &b.వెడల్పు, &b.unit);
	b.వెడల్పు = css_unit_len2device_px(అడ్డువరుస->style, unit_len_ctx,
			b.వెడల్పు, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_పట్టిక_అడ్డువరుస;

	ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	ఒకవేళ (అడ్డువరుస->children == ఖాళీ) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_శిఖరం_style(అడ్డువరుస->style);
		b.color = css_computed_border_శిఖరం_color(అడ్డువరుస->style, &b.c);
		css_computed_border_శిఖరం_వెడల్పు(అడ్డువరుస->style, &b.వెడల్పు, &b.unit);
		b.వెడల్పు = css_unit_len2device_px(అడ్డువరుస->style, unit_len_ctx,
				b.వెడల్పు, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_పట్టిక_అడ్డువరుస;

		ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		తిరిగి false;
	} లేకపోతే {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		నిర్మాణం box *c;
		bool processed = false;

		అయితే (processed == false) {
			కొరకు (c = అడ్డువరుస->children; c != ఖాళీ; c = c->తదుపరి) {
				/* Ignore cells to the left */
				ఒకవేళ (c->ప్రారంభం_స్తంభం + c->columns - 1 <
				    cell->ప్రారంభం_స్తంభం)
					కొనసాగు;
				/* Ignore cells to the right */
				ఒకవేళ (c->ప్రారంభం_స్తంభం > cell->ప్రారంభం_స్తంభం +
				    cell->columns - 1)
					కొనసాగు;

				/* Flag that we've processed a cell */
				processed = true;

				/* Consider bottom border */
				b.style = css_computed_border_bottom_style(
							c->style);
				b.color = css_computed_border_bottom_color(
							c->style, &b.c);
				css_computed_border_bottom_వెడల్పు(c->style,
							&b.వెడల్పు, &b.unit);
				b.వెడల్పు = css_unit_len2device_px(
						c->style, unit_len_ctx,
						b.వెడల్పు, b.unit);
				b.unit = CSS_UNIT_PX;
				b_src = BOX_పట్టిక_CELL;

				ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			ఒకవేళ (processed == false) {
				/* There must be a preceding row */
				ప్రతిజ్ఞా(అడ్డువరుస->prev != ఖాళీ);

				అడ్డువరుస = అడ్డువరుస->prev;
			}
		}
	}

	తిరిగి true;
}


/**
 * Process a group
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Cell being considered
 * \param group    Group to process
 * \param a        Current border style for cell
 * \param a_src    Source of \a a
 * \return true if group has non-empty rows, false otherwise
 *
 * \post \a a will be updated with most eyecatching style
 * \post \a a_src will be updated also
 */
స్థిర bool
పట్టిక_cell_శిఖరం_process_సమూహం(స్థిరరాశి css_unit_ctx *unit_len_ctx,
			     నిర్మాణం box *cell,
			     నిర్మాణం box *సమూహం,
			     నిర్మాణం border *a,
			     box_type *a_src)
{
	నిర్మాణం border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(సమూహం->style);
	b.color = css_computed_border_bottom_color(సమూహం->style, &b.c);
	css_computed_border_bottom_వెడల్పు(సమూహం->style, &b.వెడల్పు, &b.unit);
	b.వెడల్పు = css_unit_len2device_px(సమూహం->style, unit_len_ctx,
			b.వెడల్పు, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_పట్టిక_అడ్డువరుస_సమూహం;

	ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	ఒకవేళ (సమూహం->చివరి != ఖాళీ) {
		/* Process rows in group, starting with last */
		నిర్మాణం box *అడ్డువరుస = సమూహం->చివరి;

		అయితే (పట్టిక_cell_శిఖరం_process_అడ్డువరుస(unit_len_ctx, cell, అడ్డువరుస,
						  a, a_src) == false) {
			ఒకవేళ (అడ్డువరుస->prev == ఖాళీ) {
				తిరిగి false;
			} లేకపోతే {
				అడ్డువరుస = అడ్డువరుస->prev;
			}
		}
	} లేకపోతే {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_శిఖరం_style(సమూహం->style);
		b.color = css_computed_border_శిఖరం_color(సమూహం->style, &b.c);
		css_computed_border_శిఖరం_వెడల్పు(సమూహం->style, &b.వెడల్పు, &b.unit);
		b.వెడల్పు = css_unit_len2device_px(సమూహం->style, unit_len_ctx,
				b.వెడల్పు, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_పట్టిక_అడ్డువరుస_సమూహం;

		ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		తిరిగి false;
	}

	తిరిగి true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
స్థిర శూన్య
పట్టిక_వాడిన_ఎడమ_border_for_cell(స్థిరరాశి css_unit_ctx *unit_len_ctx, నిర్మాణం box *cell)
{
	నిర్మాణం border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_ఎడమ_style(cell->style);
	a.color = css_computed_border_ఎడమ_color(cell->style, &a.c);
	css_computed_border_ఎడమ_వెడల్పు(cell->style, &a.వెడల్పు, &a.unit);
	a.వెడల్పు = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.వెడల్పు, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_పట్టిక_CELL;

	ఒకవేళ (cell->prev != ఖాళీ || cell->ప్రారంభం_స్తంభం != 0) {
		/* Cell to the left -- consider its right border */
		నిర్మాణం box *prev = ఖాళీ;

		ఒకవేళ (cell->prev == ఖాళీ) {
			నిర్మాణం box *అడ్డువరుస;

			/* Spanned from a previous row in current row group */
			కొరకు (అడ్డువరుస = cell->మాతృ; అడ్డువరుస != ఖాళీ; అడ్డువరుస = అడ్డువరుస->prev) {
				కొరకు (prev = అడ్డువరుస->children; prev != ఖాళీ;
				     prev = prev->తదుపరి) {
					ఒకవేళ (prev->ప్రారంభం_స్తంభం +
					    prev->columns ==
					    cell->ప్రారంభం_స్తంభం)
						ఆపు;
				}

				ఒకవేళ (prev != ఖాళీ)
					ఆపు;
			}

			ప్రతిజ్ఞా(prev != ఖాళీ);
		} లేకపోతే {
			prev = cell->prev;
		}

		b.style = css_computed_border_కుడి_style(prev->style);
		b.color = css_computed_border_కుడి_color(prev->style, &b.c);
		css_computed_border_కుడి_వెడల్పు(prev->style, &b.వెడల్పు, &b.unit);
		b.వెడల్పు = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.వెడల్పు, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_పట్టిక_CELL;

		ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} లేకపోతే {
		/* First cell in row, so consider rows and row group */
		నిర్మాణం box *అడ్డువరుస = cell->మాతృ;
		నిర్మాణం box *సమూహం = అడ్డువరుస->మాతృ;
		నిర్మాణం box *పట్టిక = సమూహం->మాతృ;
		గుర్తులేని పూర్ణాంకం అడ్డువరుసలు = cell->అడ్డువరుసలు;

		అయితే (అడ్డువరుసలు-- > 0 && అడ్డువరుస != ఖాళీ) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_ఎడమ_style(అడ్డువరుస->style);
			b.color = css_computed_border_ఎడమ_color(
								 అడ్డువరుస->style, &b.c);
			css_computed_border_ఎడమ_వెడల్పు(
						       అడ్డువరుస->style, &b.వెడల్పు, &b.unit);
			b.వెడల్పు = css_unit_len2device_px(
					అడ్డువరుస->style, unit_len_ctx,
					b.వెడల్పు, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_పట్టిక_అడ్డువరుస;

			ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			అడ్డువరుస = అడ్డువరుస->తదుపరి;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_ఎడమ_style(సమూహం->style);
		b.color = css_computed_border_ఎడమ_color(సమూహం->style, &b.c);
		css_computed_border_ఎడమ_వెడల్పు(సమూహం->style, &b.వెడల్పు, &b.unit);
		b.వెడల్పు = css_unit_len2device_px(సమూహం->style, unit_len_ctx,
				b.వెడల్పు, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_పట్టిక_అడ్డువరుస_సమూహం;

		ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_ఎడమ_style(పట్టిక->style);
		b.color = css_computed_border_ఎడమ_color(పట్టిక->style, &b.c);
		css_computed_border_ఎడమ_వెడల్పు(పట్టిక->style, &b.వెడల్పు, &b.unit);
		b.వెడల్పు = css_unit_len2device_px(పట్టిక->style, unit_len_ctx,
				b.వెడల్పు, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_పట్టిక;

		ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[ఎడమ_2].style = a.style;
	cell->border[ఎడమ_2].c = a.c;
	cell->border[ఎడమ_2].వెడల్పు = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.వెడల్పు, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
స్థిర శూన్య
పట్టిక_వాడిన_శిఖరం_border_for_cell(స్థిరరాశి css_unit_ctx *unit_len_ctx, నిర్మాణం box *cell)
{
	నిర్మాణం border a, b;
	box_type a_src, b_src;
	నిర్మాణం box *అడ్డువరుస = cell->మాతృ;
	bool process_సమూహం = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_శిఖరం_style(cell->style);
	css_computed_border_శిఖరం_color(cell->style, &a.c);
	css_computed_border_శిఖరం_వెడల్పు(cell->style, &a.వెడల్పు, &a.unit);
	a.వెడల్పు = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.వెడల్పు, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_పట్టిక_CELL;

	/* Top border of row */
	b.style = css_computed_border_శిఖరం_style(అడ్డువరుస->style);
	css_computed_border_శిఖరం_color(అడ్డువరుస->style, &b.c);
	css_computed_border_శిఖరం_వెడల్పు(అడ్డువరుస->style, &b.వెడల్పు, &b.unit);
	b.వెడల్పు = css_unit_len2device_px(అడ్డువరుస->style, unit_len_ctx,
			b.వెడల్పు, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_పట్టిక_అడ్డువరుస;

	ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	ఒకవేళ (అడ్డువరుస->prev != ఖాళీ) {
		/* Consider row(s) above */
		అయితే (పట్టిక_cell_శిఖరం_process_అడ్డువరుస(unit_len_ctx, cell, అడ్డువరుస->prev,
						  &a, &a_src) == false) {
			ఒకవేళ (అడ్డువరుస->prev->prev == ఖాళీ) {
				/* Consider row group */
				process_సమూహం = true;
				ఆపు;
			} లేకపోతే {
				అడ్డువరుస = అడ్డువరుస->prev;
			}
		}
	} లేకపోతే {
		process_సమూహం = true;
	}

	ఒకవేళ (process_సమూహం) {
		నిర్మాణం box *సమూహం = అడ్డువరుస->మాతృ;

		/* Top border of row group */
		b.style = css_computed_border_శిఖరం_style(సమూహం->style);
		b.color = css_computed_border_శిఖరం_color(సమూహం->style, &b.c);
		css_computed_border_శిఖరం_వెడల్పు(సమూహం->style, &b.వెడల్పు, &b.unit);
		b.వెడల్పు = css_unit_len2device_px(సమూహం->style, unit_len_ctx,
				b.వెడల్పు, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_పట్టిక_అడ్డువరుస_సమూహం;

		ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		ఒకవేళ (సమూహం->prev == ఖాళీ) {
			/* Top border of table */
			పట్టిక_cell_శిఖరం_process_పట్టిక(unit_len_ctx,
						     సమూహం->మాతృ, &a, &a_src);
		} లేకపోతే {
			/* Process previous group(s) */
			అయితే (పట్టిక_cell_శిఖరం_process_సమూహం(unit_len_ctx,
							    cell, సమూహం->prev,
							    &a, &a_src) == false) {
				ఒకవేళ (సమూహం->prev->prev == ఖాళీ) {
					/* Top border of table */
					పట్టిక_cell_శిఖరం_process_పట్టిక(unit_len_ctx,
								     సమూహం->మాతృ,
								     &a, &a_src);
					ఆపు;
				} లేకపోతే {
					సమూహం = సమూహం->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[శిఖరం_2].style = a.style;
	cell->border[శిఖరం_2].c = a.c;
	cell->border[శిఖరం_2].వెడల్పు = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.వెడల్పు, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
స్థిర శూన్య
పట్టిక_వాడిన_కుడి_border_for_cell(స్థిరరాశి css_unit_ctx *unit_len_ctx, నిర్మాణం box *cell)
{
	నిర్మాణం border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_కుడి_style(cell->style);
	css_computed_border_కుడి_color(cell->style, &a.c);
	css_computed_border_కుడి_వెడల్పు(cell->style, &a.వెడల్పు, &a.unit);
	a.వెడల్పు = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.వెడల్పు, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_పట్టిక_CELL;

	ఒకవేళ (cell->తదుపరి != ఖాళీ || cell->ప్రారంభం_స్తంభం + cell->columns !=
	    cell->మాతృ->మాతృ->మాతృ->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.వెడల్పు = 0;
		a.unit = CSS_UNIT_PX;
	} లేకపోతే {
		/* Last cell in row, so consider rows and row group */
		నిర్మాణం box *అడ్డువరుస = cell->మాతృ;
		నిర్మాణం box *సమూహం = అడ్డువరుస->మాతృ;
		నిర్మాణం box *పట్టిక = సమూహం->మాతృ;
		గుర్తులేని పూర్ణాంకం అడ్డువరుసలు = cell->అడ్డువరుసలు;

		అయితే (అడ్డువరుసలు-- > 0 && అడ్డువరుస != ఖాళీ) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_కుడి_style(అడ్డువరుస->style);
			b.color = css_computed_border_కుడి_color(అడ్డువరుస->style,
								  &b.c);
			css_computed_border_కుడి_వెడల్పు(అడ్డువరుస->style,
							&b.వెడల్పు,
							&b.unit);
			b.వెడల్పు = css_unit_len2device_px(
					అడ్డువరుస->style, unit_len_ctx,
					b.వెడల్పు, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_పట్టిక_అడ్డువరుస;

			ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			అడ్డువరుస = అడ్డువరుస->తదుపరి;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_కుడి_style(సమూహం->style);
		b.color = css_computed_border_కుడి_color(సమూహం->style, &b.c);
		css_computed_border_కుడి_వెడల్పు(సమూహం->style,
						&b.వెడల్పు, &b.unit);
		b.వెడల్పు = css_unit_len2device_px(సమూహం->style, unit_len_ctx,
				b.వెడల్పు, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_పట్టిక_అడ్డువరుస_సమూహం;

		ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_కుడి_style(పట్టిక->style);
		b.color = css_computed_border_కుడి_color(పట్టిక->style, &b.c);
		css_computed_border_కుడి_వెడల్పు(పట్టిక->style,
						&b.వెడల్పు, &b.unit);
		b.వెడల్పు = css_unit_len2device_px(పట్టిక->style, unit_len_ctx,
				b.వెడల్పు, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_పట్టిక;

		ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[కుడి_2].style = a.style;
	cell->border[కుడి_2].c = a.c;
	cell->border[కుడి_2].వెడల్పు = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.వెడల్పు, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
స్థిర శూన్య
పట్టిక_వాడిన_bottom_border_for_cell(స్థిరరాశి css_unit_ctx *unit_len_ctx,
				  నిర్మాణం box *cell)
{
	నిర్మాణం border a, b;
	box_type a_src, b_src;
	నిర్మాణం box *అడ్డువరుస = cell->మాతృ;
	గుర్తులేని పూర్ణాంకం అడ్డువరుసలు = cell->అడ్డువరుసలు;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_వెడల్పు(cell->style, &a.వెడల్పు, &a.unit);
	a.వెడల్పు = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.వెడల్పు, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_పట్టిక_CELL;

	అయితే (అడ్డువరుసలు-- > 0 && అడ్డువరుస != ఖాళీ)
		అడ్డువరుస = అడ్డువరుస->తదుపరి;

	/** \todo Can cells span row groups? */

	ఒకవేళ (అడ్డువరుస != ఖాళీ) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.వెడల్పు = 0;
		a.unit = CSS_UNIT_PX;
	} లేకపోతే {
		/* Cell at bottom of table, so consider row and row group */
		నిర్మాణం box *అడ్డువరుస = cell->మాతృ;
		నిర్మాణం box *సమూహం = అడ్డువరుస->మాతృ;
		నిర్మాణం box *పట్టిక = సమూహం->మాతృ;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(అడ్డువరుస->style);
		b.color = css_computed_border_bottom_color(అడ్డువరుస->style, &b.c);
		css_computed_border_bottom_వెడల్పు(అడ్డువరుస->style, &b.వెడల్పు, &b.unit);
		b.వెడల్పు = css_unit_len2device_px(అడ్డువరుస->style, unit_len_ctx,
				b.వెడల్పు, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_పట్టిక_అడ్డువరుస;

		ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* Row group -- consider its bottom border */
		b.style = css_computed_border_bottom_style(సమూహం->style);
		b.color = css_computed_border_bottom_color(సమూహం->style, &b.c);
		css_computed_border_bottom_వెడల్పు(సమూహం->style,
						 &b.వెడల్పు, &b.unit);
		b.వెడల్పు = css_unit_len2device_px(సమూహం->style, unit_len_ctx,
				b.వెడల్పు, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_పట్టిక_అడ్డువరుస_సమూహం;

		ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its bottom border */
		b.style = css_computed_border_bottom_style(పట్టిక->style);
		b.color = css_computed_border_bottom_color(పట్టిక->style, &b.c);
		css_computed_border_bottom_వెడల్పు(పట్టిక->style,
						 &b.వెడల్పు, &b.unit);
		b.వెడల్పు = css_unit_len2device_px(పట్టిక->style, unit_len_ctx,
				b.వెడల్పు, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_పట్టిక;

		ఒకవేళ (పట్టిక_border_is_మరిన్ని_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
		}
	}

	/* a now contains the used bottom border for the cell */
	cell->border[BOTTOM].style = a.style;
	cell->border[BOTTOM].c = a.c;
	cell->border[BOTTOM].వెడల్పు = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.వెడల్పు, a.unit));
}


/* exported interface documented in html/table.h */
bool
పట్టిక_calculate_స్తంభం_types(స్థిరరాశి css_unit_ctx *unit_len_ctx, నిర్మాణం box *పట్టిక)
{
	గుర్తులేని పూర్ణాంకం i, j;
	నిర్మాణం స్తంభం *నిలువు;
	నిర్మాణం box *అడ్డువరుస_సమూహం, *అడ్డువరుస, *cell;

	ఒకవేళ (పట్టిక->నిలువు)
		/* table->col already constructed, for example frameset table */
		తిరిగి true;

	పట్టిక->నిలువు = నిలువు = talloc_array(పట్టిక, నిర్మాణం స్తంభం, పట్టిక->columns);
	ఒకవేళ (!నిలువు)
		తిరిగి false;

	కొరకు (i = 0; i != పట్టిక->columns; i++) {
		నిలువు[i].type = స్తంభం_వెడల్పు_UNKNOWN;
		నిలువు[i].వెడల్పు = 0;
		నిలువు[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	కొరకు (అడ్డువరుస_సమూహం = పట్టిక->children; అడ్డువరుస_సమూహం; అడ్డువరుస_సమూహం =అడ్డువరుస_సమూహం->తదుపరి)
		కొరకు (అడ్డువరుస = అడ్డువరుస_సమూహం->children; అడ్డువరుస; అడ్డువరుస = అడ్డువరుస->తదుపరి)
			కొరకు (cell = అడ్డువరుస->children; cell; cell = cell->తదుపరి) {
				గణన css_వెడల్పు_e type;
				css_fixed విలువ = 0;
				css_unit unit = CSS_UNIT_PX;

				ప్రతిజ్ఞా(cell->type == BOX_పట్టిక_CELL);
				ప్రతిజ్ఞా(cell->style);

				ఒకవేళ (cell->columns != 1)
					కొనసాగు;
				i = cell->ప్రారంభం_స్తంభం;

				ఒకవేళ (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					నిలువు[i].positioned = false;
				}

				type = css_computed_వెడల్పు(cell->style, &విలువ, &unit);

				/* fixed width takes priority over any other width type */
				ఒకవేళ (నిలువు[i].type != స్తంభం_వెడల్పు_FIXED &&
				    type == CSS_వెడల్పు_సమితి && unit != CSS_UNIT_PCT) {
					నిలువు[i].type = స్తంభం_వెడల్పు_FIXED;
					నిలువు[i].వెడల్పు = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							విలువ, unit));
					ఒకవేళ (నిలువు[i].వెడల్పు < 0)
						నిలువు[i].వెడల్పు = 0;
					కొనసాగు;
				}

				ఒకవేళ (నిలువు[i].type != స్తంభం_వెడల్పు_UNKNOWN)
					కొనసాగు;

				ఒకవేళ (type == CSS_వెడల్పు_సమితి && unit == CSS_UNIT_PCT) {
					నిలువు[i].type = స్తంభం_వెడల్పు_PERCENT;
					నిలువు[i].వెడల్పు = FIXTOINT(విలువ);
					ఒకవేళ (నిలువు[i].వెడల్పు < 0)
						నిలువు[i].వెడల్పు = 0;
				} లేకపోతే ఒకవేళ (type == CSS_వెడల్పు_AUTO) {
					నిలువు[i].type = స్తంభం_వెడల్పు_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	కొరకు (అడ్డువరుస_సమూహం = పట్టిక->children; అడ్డువరుస_సమూహం; అడ్డువరుస_సమూహం =అడ్డువరుస_సమూహం->తదుపరి)
		కొరకు (అడ్డువరుస = అడ్డువరుస_సమూహం->children; అడ్డువరుస; అడ్డువరుస = అడ్డువరుస->తదుపరి)
			కొరకు (cell = అడ్డువరుస->children; cell; cell = cell->తదుపరి) {
				గుర్తులేని పూర్ణాంకం fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				పూర్ణాంకం fixed_వెడల్పు = 0, percent_వెడల్పు = 0;
				గణన css_వెడల్పు_e type;
				css_fixed విలువ = 0;
				css_unit unit = CSS_UNIT_PX;

				ఒకవేళ (cell->columns == 1)
					కొనసాగు;
				i = cell->ప్రారంభం_స్తంభం;

				కొరకు (j = i; j < i + cell->columns; j++) {
					నిలువు[j].positioned = false;
				}

				/* count column types in spanned cells */
				కొరకు (j = 0; j != cell->columns; j++) {
					ఒకవేళ (నిలువు[i + j].type == స్తంభం_వెడల్పు_FIXED) {
						fixed_వెడల్పు += నిలువు[i + j].వెడల్పు;
						fixed_columns++;
					} లేకపోతే ఒకవేళ (నిలువు[i + j].type == స్తంభం_వెడల్పు_PERCENT) {
						percent_వెడల్పు += నిలువు[i + j].వెడల్పు;
						percent_columns++;
					} లేకపోతే ఒకవేళ (నిలువు[i + j].type == స్తంభం_వెడల్పు_AUTO) {
						auto_columns++;
					} లేకపోతే {
						unknown_columns++;
					}
				}

				ఒకవేళ (!unknown_columns)
					కొనసాగు;

				type = css_computed_వెడల్పు(cell->style, &విలువ, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				ఒకవేళ (type == CSS_వెడల్పు_సమితి && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					పూర్ణాంకం వెడల్పు = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							విలువ, unit)) -
						fixed_వెడల్పు) / unknown_columns;
					ఒకవేళ (వెడల్పు < 0)
						వెడల్పు = 0;
					కొరకు (j = 0; j != cell->columns; j++) {
						ఒకవేళ (నిలువు[i + j].type == స్తంభం_వెడల్పు_UNKNOWN) {
							నిలువు[i + j].type = స్తంభం_వెడల్పు_FIXED;
							నిలువు[i + j].వెడల్పు = వెడల్పు;
						}
					}
				}

				/* as above for percentage width */
				ఒకవేళ (type == CSS_వెడల్పు_సమితి && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					పూర్ణాంకం వెడల్పు = (FIXTOFLT(విలువ) -
						     percent_వెడల్పు) / unknown_columns;
					ఒకవేళ (వెడల్పు < 0)
						వెడల్పు = 0;
					కొరకు (j = 0; j != cell->columns; j++) {
						ఒకవేళ (నిలువు[i + j].type == స్తంభం_వెడల్పు_UNKNOWN) {
							నిలువు[i + j].type = స్తంభం_వెడల్పు_PERCENT;
							నిలువు[i + j].వెడల్పు = వెడల్పు;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	కొరకు (i = 0; i != పట్టిక->columns; i++) {
		ఒకవేళ (నిలువు[i].type == స్తంభం_వెడల్పు_UNKNOWN)
			నిలువు[i].type = స్తంభం_వెడల్పు_AUTO;
	}

#నిర్వచితమైతే పట్టిక_DEBUG
	కొరకు (i = 0; i != పట్టిక->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      పట్టిక,
		      i,
		      ((స్థిరరాశి అక్షరం *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[నిలువు[i].type],
		      నిలువు[i].వెడల్పు);
#అయితే_ముగింపు

	తిరిగి true;
}


/* exported interface documented in html/table.h */
శూన్య పట్టిక_వాడిన_border_for_cell(స్థిరరాశి css_unit_ctx *unit_len_ctx, నిర్మాణం box *cell)
{
	పూర్ణాంకం side;

	ప్రతిజ్ఞా(cell->type == BOX_పట్టిక_CELL);

	ఒకవేళ (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed వెడల్పు = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[ఎడమ_2].style =
			css_computed_border_ఎడమ_style(cell->style);
		css_computed_border_ఎడమ_color(cell->style,
					       &cell->border[ఎడమ_2].c);
		css_computed_border_ఎడమ_వెడల్పు(cell->style, &వెడల్పు, &unit);
		cell->border[ఎడమ_2].వెడల్పు =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					వెడల్పు, unit));

		/* Top border */
		cell->border[శిఖరం_2].style =
			css_computed_border_శిఖరం_style(cell->style);
		css_computed_border_శిఖరం_color(cell->style,
					      &cell->border[శిఖరం_2].c);
		css_computed_border_శిఖరం_వెడల్పు(cell->style, &వెడల్పు, &unit);
		cell->border[శిఖరం_2].వెడల్పు =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					వెడల్పు, unit));

		/* Right border */
		cell->border[కుడి_2].style =
			css_computed_border_కుడి_style(cell->style);
		css_computed_border_కుడి_color(cell->style,
						&cell->border[కుడి_2].c);
		css_computed_border_కుడి_వెడల్పు(cell->style, &వెడల్పు, &unit);
		cell->border[కుడి_2].వెడల్పు =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					వెడల్పు, unit));

		/* Bottom border */
		cell->border[BOTTOM].style =
			css_computed_border_bottom_style(cell->style);
		css_computed_border_bottom_color(cell->style,
						 &cell->border[BOTTOM].c);
		css_computed_border_bottom_వెడల్పు(cell->style, &వెడల్పు, &unit);
		cell->border[BOTTOM].వెడల్పు =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					వెడల్పు, unit));
	} లేకపోతే {
		/* Left border */
		పట్టిక_వాడిన_ఎడమ_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		పట్టిక_వాడిన_శిఖరం_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		పట్టిక_వాడిన_కుడి_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		పట్టిక_వాడిన_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	కొరకు (side = 0; side != 4; side++) {
		ఒకవేళ (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].వెడల్పు = 0;
	}
}
