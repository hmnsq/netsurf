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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <dom/dom.h>

#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/talloc.h"
#ଅନ୍ତର୍ଭାଵ "css/utils.h"

#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/table.h"

/* Define to enable verbose table debug */
#undef TABLE_DEBUG

/**
 * Container for border values during table border calculations
 */
ସଂରଚନା border {
	ଗଣନା css_border_style_e style;	/**< border-style */
	ଗଣନା css_border_color_e color;	/**< border-color type */
	css_color c;			/**< border-color value */
	css_fixed width;		/**< border-width length */
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
ସ୍ଥିର bool
table_border_is_more_eyecatching(ଅଚଳ css_unit_ctx *unit_len_ctx,
				 ଅଚଳ ସଂରଚନା border *a,
				 box_type a_src,
				 ଅଚଳ ସଂରଚନା border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	ଯଦି (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		ଫେରନ୍ତୁ false;

	ଯଦି (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		ଫେରନ୍ତୁ true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	ପ୍ରତିଜ୍ଞା(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	ପ୍ରତିଜ୍ଞା(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(ଖାଲି, unit_len_ctx, a->width, a->unit);
	bwidth = css_unit_len2device_px(ଖାଲି, unit_len_ctx, b->width, b->unit);

	ଯଦି (awidth < bwidth)
		ଫେରନ୍ତୁ true;
	ଅନ୍ୟଥା ଯଦି (bwidth < awidth)
		ଫେରନ୍ତୁ false;

	/* 3b -- sort by style */
	ବାଛ (a->style) {
	ସ୍ଥିତି CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	ବାଛ (b->style) {
	ସ୍ଥିତି CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	ଯଦି (impact < 0)
		ଫେରନ୍ତୁ true;
	ଅନ୍ୟଥା ଯଦି (impact > 0)
		ଫେରନ୍ତୁ false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	ବାଛ (a_src) {
	ସ୍ଥିତି BOX_TABLE_CELL:       impact++; fallthrough;
	ସ୍ଥିତି BOX_TABLE_ROW:        impact++; fallthrough;
	ସ୍ଥିତି BOX_TABLE_ROW_GROUP:  impact++; fallthrough;
	ସ୍ଥିତି BOX_TABLE:            impact++; fallthrough;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/** \todo COL/COL_GROUP */
	ବାଛ (b_src) {
	ସ୍ଥିତି BOX_TABLE_CELL:       impact--; fallthrough;
	ସ୍ଥିତି BOX_TABLE_ROW:        impact--; fallthrough;
	ସ୍ଥିତି BOX_TABLE_ROW_GROUP:  impact--; fallthrough;
	ସ୍ଥିତି BOX_TABLE:            impact--; fallthrough;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	ଯଦି (impact < 0)
		ଫେରନ୍ତୁ true;
	ଅନ୍ୟଥା ଯଦି (impact > 0)
		ଫେରନ୍ତୁ false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର ଶୂନ୍ୟ
table_cell_ଶୀର୍ଷ_process_table(ଅଚଳ css_unit_ctx *unit_len_ctx,
			     ସଂରଚନା box *table,
			     ସଂରଚନା border *a,
			     box_type *a_src)
{
	ସଂରଚନା border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_ଶୀର୍ଷ_style(table->style);
	b.color = css_computed_border_ଶୀର୍ଷ_color(table->style, &b.c);
	css_computed_border_ଶୀର୍ଷ_width(table->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(table->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE;

	ଯଦି (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
ସ୍ଥିର bool
table_cell_ଶୀର୍ଷ_process_row(ଅଚଳ css_unit_ctx *unit_len_ctx,
			   ସଂରଚନା box *cell,
			   ସଂରଚନା box *row,
			   ସଂରଚନା border *a,
			   box_type *a_src)
{
	ସଂରଚନା border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(row->style);
	b.color = css_computed_border_bottom_color(row->style, &b.c);
	css_computed_border_bottom_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	ଯଦି (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	ଯଦି (row->children == ଖାଲି) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_ଶୀର୍ଷ_style(row->style);
		b.color = css_computed_border_ଶୀର୍ଷ_color(row->style, &b.c);
		css_computed_border_ଶୀର୍ଷ_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		ଫେରନ୍ତୁ false;
	} ଅନ୍ୟଥା {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		ସଂରଚନା box *c;
		bool processed = false;

		ଯେତେବେଳେ (processed == false) {
			ପାଇଁ (c = row->children; c != ଖାଲି; c = c->next) {
				/* Ignore cells to the left */
				ଯଦି (c->start_column + c->columns - 1 <
				    cell->start_column)
					ଜାରି;
				/* Ignore cells to the right */
				ଯଦି (c->start_column > cell->start_column +
				    cell->columns - 1)
					ଜାରି;

				/* Flag that we've processed a cell */
				processed = true;

				/* Consider bottom border */
				b.style = css_computed_border_bottom_style(
							c->style);
				b.color = css_computed_border_bottom_color(
							c->style, &b.c);
				css_computed_border_bottom_width(c->style,
							&b.width, &b.unit);
				b.width = css_unit_len2device_px(
						c->style, unit_len_ctx,
						b.width, b.unit);
				b.unit = CSS_UNIT_PX;
				b_src = BOX_TABLE_CELL;

				ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			ଯଦି (processed == false) {
				/* There must be a preceding row */
				ପ୍ରତିଜ୍ଞା(row->prev != ଖାଲି);

				row = row->prev;
			}
		}
	}

	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର bool
table_cell_ଶୀର୍ଷ_process_group(ଅଚଳ css_unit_ctx *unit_len_ctx,
			     ସଂରଚନା box *cell,
			     ସଂରଚନା box *group,
			     ସଂରଚନା border *a,
			     box_type *a_src)
{
	ସଂରଚନା border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(group->style);
	b.color = css_computed_border_bottom_color(group->style, &b.c);
	css_computed_border_bottom_width(group->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(group->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW_GROUP;

	ଯଦି (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	ଯଦି (group->last != ଖାଲି) {
		/* Process rows in group, starting with last */
		ସଂରଚନା box *row = group->last;

		ଯେତେବେଳେ (table_cell_ଶୀର୍ଷ_process_row(unit_len_ctx, cell, row,
						  a, a_src) == false) {
			ଯଦି (row->prev == ଖାଲି) {
				ଫେରନ୍ତୁ false;
			} ଅନ୍ୟଥା {
				row = row->prev;
			}
		}
	} ଅନ୍ୟଥା {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_ଶୀର୍ଷ_style(group->style);
		b.color = css_computed_border_ଶୀର୍ଷ_color(group->style, &b.c);
		css_computed_border_ଶୀର୍ଷ_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ସ୍ଥିର ଶୂନ୍ୟ
table_used_ବାମ_border_for_cell(ଅଚଳ css_unit_ctx *unit_len_ctx, ସଂରଚନା box *cell)
{
	ସଂରଚନା border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_ବାମ_style(cell->style);
	a.color = css_computed_border_ବାମ_color(cell->style, &a.c);
	css_computed_border_ବାମ_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	ଯଦି (cell->prev != ଖାଲି || cell->start_column != 0) {
		/* Cell to the left -- consider its right border */
		ସଂରଚନା box *prev = ଖାଲି;

		ଯଦି (cell->prev == ଖାଲି) {
			ସଂରଚନା box *row;

			/* Spanned from a previous row in current row group */
			ପାଇଁ (row = cell->parent; row != ଖାଲି; row = row->prev) {
				ପାଇଁ (prev = row->children; prev != ଖାଲି;
				     prev = prev->next) {
					ଯଦି (prev->start_column +
					    prev->columns ==
					    cell->start_column)
						ଭାଙ୍ଗ;
				}

				ଯଦି (prev != ଖାଲି)
					ଭାଙ୍ଗ;
			}

			ପ୍ରତିଜ୍ଞା(prev != ଖାଲି);
		} ଅନ୍ୟଥା {
			prev = cell->prev;
		}

		b.style = css_computed_border_ଡାହାଣ_style(prev->style);
		b.color = css_computed_border_ଡାହାଣ_color(prev->style, &b.c);
		css_computed_border_ଡାହାଣ_width(prev->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_CELL;

		ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} ଅନ୍ୟଥା {
		/* First cell in row, so consider rows and row group */
		ସଂରଚନା box *row = cell->parent;
		ସଂରଚନା box *group = row->parent;
		ସଂରଚନା box *table = group->parent;
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା rows = cell->rows;

		ଯେତେବେଳେ (rows-- > 0 && row != ଖାଲି) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_ବାମ_style(row->style);
			b.color = css_computed_border_ବାମ_color(
								 row->style, &b.c);
			css_computed_border_ବାମ_width(
						       row->style, &b.width, &b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_ବାମ_style(group->style);
		b.color = css_computed_border_ବାମ_color(group->style, &b.c);
		css_computed_border_ବାମ_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_ବାମ_style(table->style);
		b.color = css_computed_border_ବାମ_color(table->style, &b.c);
		css_computed_border_ବାମ_width(table->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[ବାମ_2].style = a.style;
	cell->border[ବାମ_2].c = a.c;
	cell->border[ବାମ_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ସ୍ଥିର ଶୂନ୍ୟ
table_used_ଶୀର୍ଷ_border_for_cell(ଅଚଳ css_unit_ctx *unit_len_ctx, ସଂରଚନା box *cell)
{
	ସଂରଚନା border a, b;
	box_type a_src, b_src;
	ସଂରଚନା box *row = cell->parent;
	bool process_group = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_ଶୀର୍ଷ_style(cell->style);
	css_computed_border_ଶୀର୍ଷ_color(cell->style, &a.c);
	css_computed_border_ଶୀର୍ଷ_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	/* Top border of row */
	b.style = css_computed_border_ଶୀର୍ଷ_style(row->style);
	css_computed_border_ଶୀର୍ଷ_color(row->style, &b.c);
	css_computed_border_ଶୀର୍ଷ_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	ଯଦି (table_border_is_more_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	ଯଦି (row->prev != ଖାଲି) {
		/* Consider row(s) above */
		ଯେତେବେଳେ (table_cell_ଶୀର୍ଷ_process_row(unit_len_ctx, cell, row->prev,
						  &a, &a_src) == false) {
			ଯଦି (row->prev->prev == ଖାଲି) {
				/* Consider row group */
				process_group = true;
				ଭାଙ୍ଗ;
			} ଅନ୍ୟଥା {
				row = row->prev;
			}
		}
	} ଅନ୍ୟଥା {
		process_group = true;
	}

	ଯଦି (process_group) {
		ସଂରଚନା box *group = row->parent;

		/* Top border of row group */
		b.style = css_computed_border_ଶୀର୍ଷ_style(group->style);
		b.color = css_computed_border_ଶୀର୍ଷ_color(group->style, &b.c);
		css_computed_border_ଶୀର୍ଷ_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		ଯଦି (group->prev == ଖାଲି) {
			/* Top border of table */
			table_cell_ଶୀର୍ଷ_process_table(unit_len_ctx,
						     group->parent, &a, &a_src);
		} ଅନ୍ୟଥା {
			/* Process previous group(s) */
			ଯେତେବେଳେ (table_cell_ଶୀର୍ଷ_process_group(unit_len_ctx,
							    cell, group->prev,
							    &a, &a_src) == false) {
				ଯଦି (group->prev->prev == ଖାଲି) {
					/* Top border of table */
					table_cell_ଶୀର୍ଷ_process_table(unit_len_ctx,
								     group->parent,
								     &a, &a_src);
					ଭାଙ୍ଗ;
				} ଅନ୍ୟଥା {
					group = group->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[ଶୀର୍ଷ_2].style = a.style;
	cell->border[ଶୀର୍ଷ_2].c = a.c;
	cell->border[ଶୀର୍ଷ_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ସ୍ଥିର ଶୂନ୍ୟ
table_used_ଡାହାଣ_border_for_cell(ଅଚଳ css_unit_ctx *unit_len_ctx, ସଂରଚନା box *cell)
{
	ସଂରଚନା border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_ଡାହାଣ_style(cell->style);
	css_computed_border_ଡାହାଣ_color(cell->style, &a.c);
	css_computed_border_ଡାହାଣ_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	ଯଦି (cell->next != ଖାଲି || cell->start_column + cell->columns !=
	    cell->parent->parent->parent->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} ଅନ୍ୟଥା {
		/* Last cell in row, so consider rows and row group */
		ସଂରଚନା box *row = cell->parent;
		ସଂରଚନା box *group = row->parent;
		ସଂରଚନା box *table = group->parent;
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା rows = cell->rows;

		ଯେତେବେଳେ (rows-- > 0 && row != ଖାଲି) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_ଡାହାଣ_style(row->style);
			b.color = css_computed_border_ଡାହାଣ_color(row->style,
								  &b.c);
			css_computed_border_ଡାହାଣ_width(row->style,
							&b.width,
							&b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_ଡାହାଣ_style(group->style);
		b.color = css_computed_border_ଡାହାଣ_color(group->style, &b.c);
		css_computed_border_ଡାହାଣ_width(group->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_ଡାହାଣ_style(table->style);
		b.color = css_computed_border_ଡାହାଣ_color(table->style, &b.c);
		css_computed_border_ଡାହାଣ_width(table->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[ଡାହାଣ_2].style = a.style;
	cell->border[ଡାହାଣ_2].c = a.c;
	cell->border[ଡାହାଣ_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ସ୍ଥିର ଶୂନ୍ୟ
table_used_bottom_border_for_cell(ଅଚଳ css_unit_ctx *unit_len_ctx,
				  ସଂରଚନା box *cell)
{
	ସଂରଚନା border a, b;
	box_type a_src, b_src;
	ସଂରଚନା box *row = cell->parent;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା rows = cell->rows;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	ଯେତେବେଳେ (rows-- > 0 && row != ଖାଲି)
		row = row->next;

	/** \todo Can cells span row groups? */

	ଯଦି (row != ଖାଲି) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} ଅନ୍ୟଥା {
		/* Cell at bottom of table, so consider row and row group */
		ସଂରଚନା box *row = cell->parent;
		ସଂରଚନା box *group = row->parent;
		ସଂରଚନା box *table = group->parent;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(row->style);
		b.color = css_computed_border_bottom_color(row->style, &b.c);
		css_computed_border_bottom_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* Row group -- consider its bottom border */
		b.style = css_computed_border_bottom_style(group->style);
		b.color = css_computed_border_bottom_color(group->style, &b.c);
		css_computed_border_bottom_width(group->style,
						 &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its bottom border */
		b.style = css_computed_border_bottom_style(table->style);
		b.color = css_computed_border_bottom_color(table->style, &b.c);
		css_computed_border_bottom_width(table->style,
						 &b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		ଯଦି (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
		}
	}

	/* a now contains the used bottom border for the cell */
	cell->border[BOTTOM].style = a.style;
	cell->border[BOTTOM].c = a.c;
	cell->border[BOTTOM].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/* exported interface documented in html/table.h */
bool
table_calculate_column_types(ଅଚଳ css_unit_ctx *unit_len_ctx, ସଂରଚନା box *table)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
	ସଂରଚନା column *col;
	ସଂରଚନା box *row_group, *row, *cell;

	ଯଦି (table->col)
		/* table->col already constructed, for example frameset table */
		ଫେରନ୍ତୁ true;

	table->col = col = talloc_array(table, ସଂରଚନା column, table->columns);
	ଯଦି (!col)
		ଫେରନ୍ତୁ false;

	ପାଇଁ (i = 0; i != table->columns; i++) {
		col[i].type = COLUMN_WIDTH_UNKNOWN;
		col[i].width = 0;
		col[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	ପାଇଁ (row_group = table->children; row_group; row_group =row_group->next)
		ପାଇଁ (row = row_group->children; row; row = row->next)
			ପାଇଁ (cell = row->children; cell; cell = cell->next) {
				ଗଣନା css_width_e type;
				css_fixed ମୂଲ୍ୟ = 0;
				css_unit unit = CSS_UNIT_PX;

				ପ୍ରତିଜ୍ଞା(cell->type == BOX_TABLE_CELL);
				ପ୍ରତିଜ୍ଞା(cell->style);

				ଯଦି (cell->columns != 1)
					ଜାରି;
				i = cell->start_column;

				ଯଦି (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					col[i].positioned = false;
				}

				type = css_computed_width(cell->style, &ମୂଲ୍ୟ, &unit);

				/* fixed width takes priority over any other width type */
				ଯଦି (col[i].type != COLUMN_WIDTH_FIXED &&
				    type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_FIXED;
					col[i].width = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							ମୂଲ୍ୟ, unit));
					ଯଦି (col[i].width < 0)
						col[i].width = 0;
					ଜାରି;
				}

				ଯଦି (col[i].type != COLUMN_WIDTH_UNKNOWN)
					ଜାରି;

				ଯଦି (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_PERCENT;
					col[i].width = FIXTOINT(ମୂଲ୍ୟ);
					ଯଦି (col[i].width < 0)
						col[i].width = 0;
				} ଅନ୍ୟଥା ଯଦି (type == CSS_WIDTH_AUTO) {
					col[i].type = COLUMN_WIDTH_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	ପାଇଁ (row_group = table->children; row_group; row_group =row_group->next)
		ପାଇଁ (row = row_group->children; row; row = row->next)
			ପାଇଁ (cell = row->children; cell; cell = cell->next) {
				unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା fixed_width = 0, percent_width = 0;
				ଗଣନା css_width_e type;
				css_fixed ମୂଲ୍ୟ = 0;
				css_unit unit = CSS_UNIT_PX;

				ଯଦି (cell->columns == 1)
					ଜାରି;
				i = cell->start_column;

				ପାଇଁ (j = i; j < i + cell->columns; j++) {
					col[j].positioned = false;
				}

				/* count column types in spanned cells */
				ପାଇଁ (j = 0; j != cell->columns; j++) {
					ଯଦି (col[i + j].type == COLUMN_WIDTH_FIXED) {
						fixed_width += col[i + j].width;
						fixed_columns++;
					} ଅନ୍ୟଥା ଯଦି (col[i + j].type == COLUMN_WIDTH_PERCENT) {
						percent_width += col[i + j].width;
						percent_columns++;
					} ଅନ୍ୟଥା ଯଦି (col[i + j].type == COLUMN_WIDTH_AUTO) {
						auto_columns++;
					} ଅନ୍ୟଥା {
						unknown_columns++;
					}
				}

				ଯଦି (!unknown_columns)
					ଜାରି;

				type = css_computed_width(cell->style, &ମୂଲ୍ୟ, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				ଯଦି (type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					ପୂର୍ଣ୍ଣସଂଖ୍ୟା width = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							ମୂଲ୍ୟ, unit)) -
						fixed_width) / unknown_columns;
					ଯଦି (width < 0)
						width = 0;
					ପାଇଁ (j = 0; j != cell->columns; j++) {
						ଯଦି (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_FIXED;
							col[i + j].width = width;
						}
					}
				}

				/* as above for percentage width */
				ଯଦି (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					ପୂର୍ଣ୍ଣସଂଖ୍ୟା width = (FIXTOFLT(ମୂଲ୍ୟ) -
						     percent_width) / unknown_columns;
					ଯଦି (width < 0)
						width = 0;
					ପାଇଁ (j = 0; j != cell->columns; j++) {
						ଯଦି (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_PERCENT;
							col[i + j].width = width;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	ପାଇଁ (i = 0; i != table->columns; i++) {
		ଯଦି (col[i].type == COLUMN_WIDTH_UNKNOWN)
			col[i].type = COLUMN_WIDTH_AUTO;
	}

#ifdef TABLE_DEBUG
	ପାଇଁ (i = 0; i != table->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      table,
		      i,
		      ((ଅଚଳ ଅକ୍ଷର *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[col[i].type],
		      col[i].width);
#endif

	ଫେରନ୍ତୁ true;
}


/* exported interface documented in html/table.h */
ଶୂନ୍ୟ table_used_border_for_cell(ଅଚଳ css_unit_ctx *unit_len_ctx, ସଂରଚନା box *cell)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା side;

	ପ୍ରତିଜ୍ଞା(cell->type == BOX_TABLE_CELL);

	ଯଦି (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed width = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[ବାମ_2].style =
			css_computed_border_ବାମ_style(cell->style);
		css_computed_border_ବାମ_color(cell->style,
					       &cell->border[ବାମ_2].c);
		css_computed_border_ବାମ_width(cell->style, &width, &unit);
		cell->border[ବାମ_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Top border */
		cell->border[ଶୀର୍ଷ_2].style =
			css_computed_border_ଶୀର୍ଷ_style(cell->style);
		css_computed_border_ଶୀର୍ଷ_color(cell->style,
					      &cell->border[ଶୀର୍ଷ_2].c);
		css_computed_border_ଶୀର୍ଷ_width(cell->style, &width, &unit);
		cell->border[ଶୀର୍ଷ_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Right border */
		cell->border[ଡାହାଣ_2].style =
			css_computed_border_ଡାହାଣ_style(cell->style);
		css_computed_border_ଡାହାଣ_color(cell->style,
						&cell->border[ଡାହାଣ_2].c);
		css_computed_border_ଡାହାଣ_width(cell->style, &width, &unit);
		cell->border[ଡାହାଣ_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Bottom border */
		cell->border[BOTTOM].style =
			css_computed_border_bottom_style(cell->style);
		css_computed_border_bottom_color(cell->style,
						 &cell->border[BOTTOM].c);
		css_computed_border_bottom_width(cell->style, &width, &unit);
		cell->border[BOTTOM].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));
	} ଅନ୍ୟଥା {
		/* Left border */
		table_used_ବାମ_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		table_used_ଶୀର୍ଷ_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		table_used_ଡାହାଣ_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		table_used_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	ପାଇଁ (side = 0; side != 4; side++) {
		ଯଦି (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].width = 0;
	}
}
