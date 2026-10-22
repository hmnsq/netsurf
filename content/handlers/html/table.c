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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <dom/dom.h>

#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/talloc.h"
#অন্তর্ভাব "css/utils.h"

#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/table.h"

/* Define to enable verbose table debug */
#undef TABLE_DEBUG

/**
 * Container for border values during table border calculations
 */
কাঠামো border {
	গণনা css_border_style_e style;	/**< border-style */
	গণনা css_border_color_e color;	/**< border-color type */
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
স্থির bool
table_border_is_more_eyecatching(ধ্রুবক css_unit_ctx *unit_len_ctx,
				 ধ্রুবক কাঠামো border *a,
				 box_type a_src,
				 ধ্রুবক কাঠামো border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	পূর্ণসংখ্যা impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	যদি (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		ফেরত false;

	যদি (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		ফেরত true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	প্রতিজ্ঞা(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	প্রতিজ্ঞা(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(ফাঁকা, unit_len_ctx, a->width, a->unit);
	bwidth = css_unit_len2device_px(ফাঁকা, unit_len_ctx, b->width, b->unit);

	যদি (awidth < bwidth)
		ফেরত true;
	অন্যথা যদি (bwidth < awidth)
		ফেরত false;

	/* 3b -- sort by style */
	নির্বাচন (a->style) {
	ক্ষেত্র CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	নির্বাচন (b->style) {
	ক্ষেত্র CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	যদি (impact < 0)
		ফেরত true;
	অন্যথা যদি (impact > 0)
		ফেরত false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	নির্বাচন (a_src) {
	ক্ষেত্র BOX_TABLE_CELL:       impact++; fallthrough;
	ক্ষেত্র BOX_TABLE_ROW:        impact++; fallthrough;
	ক্ষেত্র BOX_TABLE_ROW_GROUP:  impact++; fallthrough;
	ক্ষেত্র BOX_TABLE:            impact++; fallthrough;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/** \todo COL/COL_GROUP */
	নির্বাচন (b_src) {
	ক্ষেত্র BOX_TABLE_CELL:       impact--; fallthrough;
	ক্ষেত্র BOX_TABLE_ROW:        impact--; fallthrough;
	ক্ষেত্র BOX_TABLE_ROW_GROUP:  impact--; fallthrough;
	ক্ষেত্র BOX_TABLE:            impact--; fallthrough;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	যদি (impact < 0)
		ফেরত true;
	অন্যথা যদি (impact > 0)
		ফেরত false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	ফেরত true;
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
স্থির শূন্য
table_cell_শীর্ষ_process_table(ধ্রুবক css_unit_ctx *unit_len_ctx,
			     কাঠামো box *table,
			     কাঠামো border *a,
			     box_type *a_src)
{
	কাঠামো border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_শীর্ষ_style(table->style);
	b.color = css_computed_border_শীর্ষ_color(table->style, &b.c);
	css_computed_border_শীর্ষ_width(table->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(table->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE;

	যদি (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
স্থির bool
table_cell_শীর্ষ_process_row(ধ্রুবক css_unit_ctx *unit_len_ctx,
			   কাঠামো box *cell,
			   কাঠামো box *row,
			   কাঠামো border *a,
			   box_type *a_src)
{
	কাঠামো border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(row->style);
	b.color = css_computed_border_bottom_color(row->style, &b.c);
	css_computed_border_bottom_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	যদি (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	যদি (row->children == ফাঁকা) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_শীর্ষ_style(row->style);
		b.color = css_computed_border_শীর্ষ_color(row->style, &b.c);
		css_computed_border_শীর্ষ_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		ফেরত false;
	} অন্যথা {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		কাঠামো box *c;
		bool processed = false;

		যখন (processed == false) {
			জন্য (c = row->children; c != ফাঁকা; c = c->পরবর্তী) {
				/* Ignore cells to the left */
				যদি (c->start_column + c->columns - 1 <
				    cell->start_column)
					চালু;
				/* Ignore cells to the right */
				যদি (c->start_column > cell->start_column +
				    cell->columns - 1)
					চালু;

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

				যদি (table_border_is_more_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			যদি (processed == false) {
				/* There must be a preceding row */
				প্রতিজ্ঞা(row->পূর্ব != ফাঁকা);

				row = row->পূর্ব;
			}
		}
	}

	ফেরত true;
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
স্থির bool
table_cell_শীর্ষ_process_group(ধ্রুবক css_unit_ctx *unit_len_ctx,
			     কাঠামো box *cell,
			     কাঠামো box *group,
			     কাঠামো border *a,
			     box_type *a_src)
{
	কাঠামো border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(group->style);
	b.color = css_computed_border_bottom_color(group->style, &b.c);
	css_computed_border_bottom_width(group->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(group->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW_GROUP;

	যদি (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	যদি (group->last != ফাঁকা) {
		/* Process rows in group, starting with last */
		কাঠামো box *row = group->last;

		যখন (table_cell_শীর্ষ_process_row(unit_len_ctx, cell, row,
						  a, a_src) == false) {
			যদি (row->পূর্ব == ফাঁকা) {
				ফেরত false;
			} অন্যথা {
				row = row->পূর্ব;
			}
		}
	} অন্যথা {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_শীর্ষ_style(group->style);
		b.color = css_computed_border_শীর্ষ_color(group->style, &b.c);
		css_computed_border_শীর্ষ_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		ফেরত false;
	}

	ফেরত true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
স্থির শূন্য
table_used_বাম_border_for_cell(ধ্রুবক css_unit_ctx *unit_len_ctx, কাঠামো box *cell)
{
	কাঠামো border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_বাম_style(cell->style);
	a.color = css_computed_border_বাম_color(cell->style, &a.c);
	css_computed_border_বাম_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	যদি (cell->পূর্ব != ফাঁকা || cell->start_column != 0) {
		/* Cell to the left -- consider its right border */
		কাঠামো box *পূর্ব = ফাঁকা;

		যদি (cell->পূর্ব == ফাঁকা) {
			কাঠামো box *row;

			/* Spanned from a previous row in current row group */
			জন্য (row = cell->parent; row != ফাঁকা; row = row->পূর্ব) {
				জন্য (পূর্ব = row->children; পূর্ব != ফাঁকা;
				     পূর্ব = পূর্ব->পরবর্তী) {
					যদি (পূর্ব->start_column +
					    পূর্ব->columns ==
					    cell->start_column)
						বিরতি;
				}

				যদি (পূর্ব != ফাঁকা)
					বিরতি;
			}

			প্রতিজ্ঞা(পূর্ব != ফাঁকা);
		} অন্যথা {
			পূর্ব = cell->পূর্ব;
		}

		b.style = css_computed_border_ডান_style(পূর্ব->style);
		b.color = css_computed_border_ডান_color(পূর্ব->style, &b.c);
		css_computed_border_ডান_width(পূর্ব->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(পূর্ব->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_CELL;

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} অন্যথা {
		/* First cell in row, so consider rows and row group */
		কাঠামো box *row = cell->parent;
		কাঠামো box *group = row->parent;
		কাঠামো box *table = group->parent;
		unsigned পূর্ণসংখ্যা rows = cell->rows;

		যখন (rows-- > 0 && row != ফাঁকা) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_বাম_style(row->style);
			b.color = css_computed_border_বাম_color(
								 row->style, &b.c);
			css_computed_border_বাম_width(
						       row->style, &b.width, &b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			যদি (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->পরবর্তী;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_বাম_style(group->style);
		b.color = css_computed_border_বাম_color(group->style, &b.c);
		css_computed_border_বাম_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_বাম_style(table->style);
		b.color = css_computed_border_বাম_color(table->style, &b.c);
		css_computed_border_বাম_width(table->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[বাম_2].style = a.style;
	cell->border[বাম_2].c = a.c;
	cell->border[বাম_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
স্থির শূন্য
table_used_শীর্ষ_border_for_cell(ধ্রুবক css_unit_ctx *unit_len_ctx, কাঠামো box *cell)
{
	কাঠামো border a, b;
	box_type a_src, b_src;
	কাঠামো box *row = cell->parent;
	bool process_group = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_শীর্ষ_style(cell->style);
	css_computed_border_শীর্ষ_color(cell->style, &a.c);
	css_computed_border_শীর্ষ_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	/* Top border of row */
	b.style = css_computed_border_শীর্ষ_style(row->style);
	css_computed_border_শীর্ষ_color(row->style, &b.c);
	css_computed_border_শীর্ষ_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	যদি (table_border_is_more_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	যদি (row->পূর্ব != ফাঁকা) {
		/* Consider row(s) above */
		যখন (table_cell_শীর্ষ_process_row(unit_len_ctx, cell, row->পূর্ব,
						  &a, &a_src) == false) {
			যদি (row->পূর্ব->পূর্ব == ফাঁকা) {
				/* Consider row group */
				process_group = true;
				বিরতি;
			} অন্যথা {
				row = row->পূর্ব;
			}
		}
	} অন্যথা {
		process_group = true;
	}

	যদি (process_group) {
		কাঠামো box *group = row->parent;

		/* Top border of row group */
		b.style = css_computed_border_শীর্ষ_style(group->style);
		b.color = css_computed_border_শীর্ষ_color(group->style, &b.c);
		css_computed_border_শীর্ষ_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		যদি (group->পূর্ব == ফাঁকা) {
			/* Top border of table */
			table_cell_শীর্ষ_process_table(unit_len_ctx,
						     group->parent, &a, &a_src);
		} অন্যথা {
			/* Process previous group(s) */
			যখন (table_cell_শীর্ষ_process_group(unit_len_ctx,
							    cell, group->পূর্ব,
							    &a, &a_src) == false) {
				যদি (group->পূর্ব->পূর্ব == ফাঁকা) {
					/* Top border of table */
					table_cell_শীর্ষ_process_table(unit_len_ctx,
								     group->parent,
								     &a, &a_src);
					বিরতি;
				} অন্যথা {
					group = group->পূর্ব;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[শীর্ষ_2].style = a.style;
	cell->border[শীর্ষ_2].c = a.c;
	cell->border[শীর্ষ_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
স্থির শূন্য
table_used_ডান_border_for_cell(ধ্রুবক css_unit_ctx *unit_len_ctx, কাঠামো box *cell)
{
	কাঠামো border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_ডান_style(cell->style);
	css_computed_border_ডান_color(cell->style, &a.c);
	css_computed_border_ডান_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	যদি (cell->পরবর্তী != ফাঁকা || cell->start_column + cell->columns !=
	    cell->parent->parent->parent->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} অন্যথা {
		/* Last cell in row, so consider rows and row group */
		কাঠামো box *row = cell->parent;
		কাঠামো box *group = row->parent;
		কাঠামো box *table = group->parent;
		unsigned পূর্ণসংখ্যা rows = cell->rows;

		যখন (rows-- > 0 && row != ফাঁকা) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_ডান_style(row->style);
			b.color = css_computed_border_ডান_color(row->style,
								  &b.c);
			css_computed_border_ডান_width(row->style,
							&b.width,
							&b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			যদি (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->পরবর্তী;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_ডান_style(group->style);
		b.color = css_computed_border_ডান_color(group->style, &b.c);
		css_computed_border_ডান_width(group->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_ডান_style(table->style);
		b.color = css_computed_border_ডান_color(table->style, &b.c);
		css_computed_border_ডান_width(table->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[ডান_2].style = a.style;
	cell->border[ডান_2].c = a.c;
	cell->border[ডান_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
স্থির শূন্য
table_used_bottom_border_for_cell(ধ্রুবক css_unit_ctx *unit_len_ctx,
				  কাঠামো box *cell)
{
	কাঠামো border a, b;
	box_type a_src, b_src;
	কাঠামো box *row = cell->parent;
	unsigned পূর্ণসংখ্যা rows = cell->rows;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	যখন (rows-- > 0 && row != ফাঁকা)
		row = row->পরবর্তী;

	/** \todo Can cells span row groups? */

	যদি (row != ফাঁকা) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} অন্যথা {
		/* Cell at bottom of table, so consider row and row group */
		কাঠামো box *row = cell->parent;
		কাঠামো box *group = row->parent;
		কাঠামো box *table = group->parent;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(row->style);
		b.color = css_computed_border_bottom_color(row->style, &b.c);
		css_computed_border_bottom_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
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

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
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

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
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
table_calculate_column_types(ধ্রুবক css_unit_ctx *unit_len_ctx, কাঠামো box *table)
{
	unsigned পূর্ণসংখ্যা i, j;
	কাঠামো column *col;
	কাঠামো box *row_group, *row, *cell;

	যদি (table->col)
		/* table->col already constructed, for example frameset table */
		ফেরত true;

	table->col = col = talloc_array(table, কাঠামো column, table->columns);
	যদি (!col)
		ফেরত false;

	জন্য (i = 0; i != table->columns; i++) {
		col[i].type = COLUMN_WIDTH_UNKNOWN;
		col[i].width = 0;
		col[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	জন্য (row_group = table->children; row_group; row_group =row_group->পরবর্তী)
		জন্য (row = row_group->children; row; row = row->পরবর্তী)
			জন্য (cell = row->children; cell; cell = cell->পরবর্তী) {
				গণনা css_width_e type;
				css_fixed মান = 0;
				css_unit unit = CSS_UNIT_PX;

				প্রতিজ্ঞা(cell->type == BOX_TABLE_CELL);
				প্রতিজ্ঞা(cell->style);

				যদি (cell->columns != 1)
					চালু;
				i = cell->start_column;

				যদি (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					col[i].positioned = false;
				}

				type = css_computed_width(cell->style, &মান, &unit);

				/* fixed width takes priority over any other width type */
				যদি (col[i].type != COLUMN_WIDTH_FIXED &&
				    type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_FIXED;
					col[i].width = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							মান, unit));
					যদি (col[i].width < 0)
						col[i].width = 0;
					চালু;
				}

				যদি (col[i].type != COLUMN_WIDTH_UNKNOWN)
					চালু;

				যদি (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_PERCENT;
					col[i].width = FIXTOINT(মান);
					যদি (col[i].width < 0)
						col[i].width = 0;
				} অন্যথা যদি (type == CSS_WIDTH_AUTO) {
					col[i].type = COLUMN_WIDTH_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	জন্য (row_group = table->children; row_group; row_group =row_group->পরবর্তী)
		জন্য (row = row_group->children; row; row = row->পরবর্তী)
			জন্য (cell = row->children; cell; cell = cell->পরবর্তী) {
				unsigned পূর্ণসংখ্যা fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				পূর্ণসংখ্যা fixed_width = 0, percent_width = 0;
				গণনা css_width_e type;
				css_fixed মান = 0;
				css_unit unit = CSS_UNIT_PX;

				যদি (cell->columns == 1)
					চালু;
				i = cell->start_column;

				জন্য (j = i; j < i + cell->columns; j++) {
					col[j].positioned = false;
				}

				/* count column types in spanned cells */
				জন্য (j = 0; j != cell->columns; j++) {
					যদি (col[i + j].type == COLUMN_WIDTH_FIXED) {
						fixed_width += col[i + j].width;
						fixed_columns++;
					} অন্যথা যদি (col[i + j].type == COLUMN_WIDTH_PERCENT) {
						percent_width += col[i + j].width;
						percent_columns++;
					} অন্যথা যদি (col[i + j].type == COLUMN_WIDTH_AUTO) {
						auto_columns++;
					} অন্যথা {
						unknown_columns++;
					}
				}

				যদি (!unknown_columns)
					চালু;

				type = css_computed_width(cell->style, &মান, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				যদি (type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					পূর্ণসংখ্যা width = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							মান, unit)) -
						fixed_width) / unknown_columns;
					যদি (width < 0)
						width = 0;
					জন্য (j = 0; j != cell->columns; j++) {
						যদি (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_FIXED;
							col[i + j].width = width;
						}
					}
				}

				/* as above for percentage width */
				যদি (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					পূর্ণসংখ্যা width = (FIXTOFLT(মান) -
						     percent_width) / unknown_columns;
					যদি (width < 0)
						width = 0;
					জন্য (j = 0; j != cell->columns; j++) {
						যদি (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_PERCENT;
							col[i + j].width = width;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	জন্য (i = 0; i != table->columns; i++) {
		যদি (col[i].type == COLUMN_WIDTH_UNKNOWN)
			col[i].type = COLUMN_WIDTH_AUTO;
	}

#ifdef TABLE_DEBUG
	জন্য (i = 0; i != table->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      table,
		      i,
		      ((ধ্রুবক অক্ষর *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[col[i].type],
		      col[i].width);
#endif

	ফেরত true;
}


/* exported interface documented in html/table.h */
শূন্য table_used_border_for_cell(ধ্রুবক css_unit_ctx *unit_len_ctx, কাঠামো box *cell)
{
	পূর্ণসংখ্যা side;

	প্রতিজ্ঞা(cell->type == BOX_TABLE_CELL);

	যদি (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed width = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[বাম_2].style =
			css_computed_border_বাম_style(cell->style);
		css_computed_border_বাম_color(cell->style,
					       &cell->border[বাম_2].c);
		css_computed_border_বাম_width(cell->style, &width, &unit);
		cell->border[বাম_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Top border */
		cell->border[শীর্ষ_2].style =
			css_computed_border_শীর্ষ_style(cell->style);
		css_computed_border_শীর্ষ_color(cell->style,
					      &cell->border[শীর্ষ_2].c);
		css_computed_border_শীর্ষ_width(cell->style, &width, &unit);
		cell->border[শীর্ষ_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Right border */
		cell->border[ডান_2].style =
			css_computed_border_ডান_style(cell->style);
		css_computed_border_ডান_color(cell->style,
						&cell->border[ডান_2].c);
		css_computed_border_ডান_width(cell->style, &width, &unit);
		cell->border[ডান_2].width =
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
	} অন্যথা {
		/* Left border */
		table_used_বাম_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		table_used_শীর্ষ_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		table_used_ডান_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		table_used_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	জন্য (side = 0; side != 4; side++) {
		যদি (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].width = 0;
	}
}
