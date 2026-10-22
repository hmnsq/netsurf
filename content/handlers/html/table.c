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

#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <dom/dom.h>

#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/talloc.h"
#অন্তর্ভাৱ "css/utils.h"

#অন্তর্ভাৱ "html/box.h"
#অন্তর্ভাৱ "html/table.h"

/* Define to enable verbose table debug */
#undef TABLE_DEBUG

/**
 * Container for border values during table border calculations
 */
সংরচনা border {
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
table_border_is_more_eyecatching(ধ্রুৱ css_unit_ctx *unit_len_ctx,
				 ধ্রুৱ সংরচনা border *a,
				 box_type a_src,
				 ধ্রুৱ সংরচনা border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	পূর্ণাঙ্ক impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	যদি (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		ঘূৰাই false;

	যদি (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		ঘূৰাই true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	প্রতিজ্ঞা(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	প্রতিজ্ঞা(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(রিক্ত, unit_len_ctx, a->width, a->unit);
	bwidth = css_unit_len2device_px(রিক্ত, unit_len_ctx, b->width, b->unit);

	যদি (awidth < bwidth)
		ঘূৰাই true;
	অন্যথা যদি (bwidth < awidth)
		ঘূৰাই false;

	/* 3b -- sort by style */
	ৱিকল্প (a->style) {
	পক্ষ CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	পক্ষ CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	পক্ষ CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	পক্ষ CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	পক্ষ CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	পক্ষ CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	পক্ষ CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	পক্ষ CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	স্ৱভাৱ:
		ভঙ্গ;
	}

	ৱিকল্প (b->style) {
	পক্ষ CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	পক্ষ CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	পক্ষ CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	পক্ষ CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	পক্ষ CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	পক্ষ CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	পক্ষ CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	পক্ষ CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	স্ৱভাৱ:
		ভঙ্গ;
	}

	যদি (impact < 0)
		ঘূৰাই true;
	অন্যথা যদি (impact > 0)
		ঘূৰাই false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	ৱিকল্প (a_src) {
	পক্ষ BOX_TABLE_CELL:       impact++; fallthrough;
	পক্ষ BOX_TABLE_ROW:        impact++; fallthrough;
	পক্ষ BOX_TABLE_ROW_GROUP:  impact++; fallthrough;
	পক্ষ BOX_TABLE:            impact++; fallthrough;
	স্ৱভাৱ:
		ভঙ্গ;
	}

	/** \todo COL/COL_GROUP */
	ৱিকল্প (b_src) {
	পক্ষ BOX_TABLE_CELL:       impact--; fallthrough;
	পক্ষ BOX_TABLE_ROW:        impact--; fallthrough;
	পক্ষ BOX_TABLE_ROW_GROUP:  impact--; fallthrough;
	পক্ষ BOX_TABLE:            impact--; fallthrough;
	স্ৱভাৱ:
		ভঙ্গ;
	}

	যদি (impact < 0)
		ঘূৰাই true;
	অন্যথা যদি (impact > 0)
		ঘূৰাই false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	ঘূৰাই true;
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
table_cell_শীৰ্ষ_process_table(ধ্রুৱ css_unit_ctx *unit_len_ctx,
			     সংরচনা box *table,
			     সংরচনা border *a,
			     box_type *a_src)
{
	সংরচনা border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_শীৰ্ষ_style(table->style);
	b.color = css_computed_border_শীৰ্ষ_color(table->style, &b.c);
	css_computed_border_শীৰ্ষ_width(table->style, &b.width, &b.unit);
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
table_cell_শীৰ্ষ_process_row(ধ্রুৱ css_unit_ctx *unit_len_ctx,
			   সংরচনা box *cell,
			   সংরচনা box *row,
			   সংরচনা border *a,
			   box_type *a_src)
{
	সংরচনা border b;
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

	যদি (row->children == রিক্ত) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_শীৰ্ষ_style(row->style);
		b.color = css_computed_border_শীৰ্ষ_color(row->style, &b.c);
		css_computed_border_শীৰ্ষ_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		ঘূৰাই false;
	} অন্যথা {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		সংরচনা box *c;
		bool processed = false;

		যেতিয়ালৈ (processed == false) {
			বাবে (c = row->children; c != রিক্ত; c = c->next) {
				/* Ignore cells to the left */
				যদি (c->start_column + c->columns - 1 <
				    cell->start_column)
					অনুৱর্ত;
				/* Ignore cells to the right */
				যদি (c->start_column > cell->start_column +
				    cell->columns - 1)
					অনুৱর্ত;

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
				প্রতিজ্ঞা(row->prev != রিক্ত);

				row = row->prev;
			}
		}
	}

	ঘূৰাই true;
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
table_cell_শীৰ্ষ_process_group(ধ্রুৱ css_unit_ctx *unit_len_ctx,
			     সংরচনা box *cell,
			     সংরচনা box *group,
			     সংরচনা border *a,
			     box_type *a_src)
{
	সংরচনা border b;
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

	যদি (group->last != রিক্ত) {
		/* Process rows in group, starting with last */
		সংরচনা box *row = group->last;

		যেতিয়ালৈ (table_cell_শীৰ্ষ_process_row(unit_len_ctx, cell, row,
						  a, a_src) == false) {
			যদি (row->prev == রিক্ত) {
				ঘূৰাই false;
			} অন্যথা {
				row = row->prev;
			}
		}
	} অন্যথা {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_শীৰ্ষ_style(group->style);
		b.color = css_computed_border_শীৰ্ষ_color(group->style, &b.c);
		css_computed_border_শীৰ্ষ_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		ঘূৰাই false;
	}

	ঘূৰাই true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
স্থির শূন্য
table_used_বাম_border_for_cell(ধ্রুৱ css_unit_ctx *unit_len_ctx, সংরচনা box *cell)
{
	সংরচনা border a, b;
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

	যদি (cell->prev != রিক্ত || cell->start_column != 0) {
		/* Cell to the left -- consider its right border */
		সংরচনা box *prev = রিক্ত;

		যদি (cell->prev == রিক্ত) {
			সংরচনা box *row;

			/* Spanned from a previous row in current row group */
			বাবে (row = cell->parent; row != রিক্ত; row = row->prev) {
				বাবে (prev = row->children; prev != রিক্ত;
				     prev = prev->next) {
					যদি (prev->start_column +
					    prev->columns ==
					    cell->start_column)
						ভঙ্গ;
				}

				যদি (prev != রিক্ত)
					ভঙ্গ;
			}

			প্রতিজ্ঞা(prev != রিক্ত);
		} অন্যথা {
			prev = cell->prev;
		}

		b.style = css_computed_border_ডান_style(prev->style);
		b.color = css_computed_border_ডান_color(prev->style, &b.c);
		css_computed_border_ডান_width(prev->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(prev->style, unit_len_ctx,
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
		সংরচনা box *row = cell->parent;
		সংরচনা box *group = row->parent;
		সংরচনা box *table = group->parent;
		unsigned পূর্ণাঙ্ক rows = cell->rows;

		যেতিয়ালৈ (rows-- > 0 && row != রিক্ত) {
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

			row = row->next;
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
table_used_শীৰ্ষ_border_for_cell(ধ্রুৱ css_unit_ctx *unit_len_ctx, সংরচনা box *cell)
{
	সংরচনা border a, b;
	box_type a_src, b_src;
	সংরচনা box *row = cell->parent;
	bool process_group = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_শীৰ্ষ_style(cell->style);
	css_computed_border_শীৰ্ষ_color(cell->style, &a.c);
	css_computed_border_শীৰ্ষ_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	/* Top border of row */
	b.style = css_computed_border_শীৰ্ষ_style(row->style);
	css_computed_border_শীৰ্ষ_color(row->style, &b.c);
	css_computed_border_শীৰ্ষ_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	যদি (table_border_is_more_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	যদি (row->prev != রিক্ত) {
		/* Consider row(s) above */
		যেতিয়ালৈ (table_cell_শীৰ্ষ_process_row(unit_len_ctx, cell, row->prev,
						  &a, &a_src) == false) {
			যদি (row->prev->prev == রিক্ত) {
				/* Consider row group */
				process_group = true;
				ভঙ্গ;
			} অন্যথা {
				row = row->prev;
			}
		}
	} অন্যথা {
		process_group = true;
	}

	যদি (process_group) {
		সংরচনা box *group = row->parent;

		/* Top border of row group */
		b.style = css_computed_border_শীৰ্ষ_style(group->style);
		b.color = css_computed_border_শীৰ্ষ_color(group->style, &b.c);
		css_computed_border_শীৰ্ষ_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		যদি (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		যদি (group->prev == রিক্ত) {
			/* Top border of table */
			table_cell_শীৰ্ষ_process_table(unit_len_ctx,
						     group->parent, &a, &a_src);
		} অন্যথা {
			/* Process previous group(s) */
			যেতিয়ালৈ (table_cell_শীৰ্ষ_process_group(unit_len_ctx,
							    cell, group->prev,
							    &a, &a_src) == false) {
				যদি (group->prev->prev == রিক্ত) {
					/* Top border of table */
					table_cell_শীৰ্ষ_process_table(unit_len_ctx,
								     group->parent,
								     &a, &a_src);
					ভঙ্গ;
				} অন্যথা {
					group = group->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[শীৰ্ষ_2].style = a.style;
	cell->border[শীৰ্ষ_2].c = a.c;
	cell->border[শীৰ্ষ_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
স্থির শূন্য
table_used_ডান_border_for_cell(ধ্রুৱ css_unit_ctx *unit_len_ctx, সংরচনা box *cell)
{
	সংরচনা border a, b;
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

	যদি (cell->next != রিক্ত || cell->start_column + cell->columns !=
	    cell->parent->parent->parent->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} অন্যথা {
		/* Last cell in row, so consider rows and row group */
		সংরচনা box *row = cell->parent;
		সংরচনা box *group = row->parent;
		সংরচনা box *table = group->parent;
		unsigned পূর্ণাঙ্ক rows = cell->rows;

		যেতিয়ালৈ (rows-- > 0 && row != রিক্ত) {
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

			row = row->next;
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
table_used_bottom_border_for_cell(ধ্রুৱ css_unit_ctx *unit_len_ctx,
				  সংরচনা box *cell)
{
	সংরচনা border a, b;
	box_type a_src, b_src;
	সংরচনা box *row = cell->parent;
	unsigned পূর্ণাঙ্ক rows = cell->rows;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	যেতিয়ালৈ (rows-- > 0 && row != রিক্ত)
		row = row->next;

	/** \todo Can cells span row groups? */

	যদি (row != রিক্ত) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} অন্যথা {
		/* Cell at bottom of table, so consider row and row group */
		সংরচনা box *row = cell->parent;
		সংরচনা box *group = row->parent;
		সংরচনা box *table = group->parent;

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
table_calculate_column_types(ধ্রুৱ css_unit_ctx *unit_len_ctx, সংরচনা box *table)
{
	unsigned পূর্ণাঙ্ক i, j;
	সংরচনা column *col;
	সংরচনা box *row_group, *row, *cell;

	যদি (table->col)
		/* table->col already constructed, for example frameset table */
		ঘূৰাই true;

	table->col = col = talloc_array(table, সংরচনা column, table->columns);
	যদি (!col)
		ঘূৰাই false;

	বাবে (i = 0; i != table->columns; i++) {
		col[i].type = COLUMN_WIDTH_UNKNOWN;
		col[i].width = 0;
		col[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	বাবে (row_group = table->children; row_group; row_group =row_group->next)
		বাবে (row = row_group->children; row; row = row->next)
			বাবে (cell = row->children; cell; cell = cell->next) {
				গণনা css_width_e type;
				css_fixed মান = 0;
				css_unit unit = CSS_UNIT_PX;

				প্রতিজ্ঞা(cell->type == BOX_TABLE_CELL);
				প্রতিজ্ঞা(cell->style);

				যদি (cell->columns != 1)
					অনুৱর্ত;
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
					অনুৱর্ত;
				}

				যদি (col[i].type != COLUMN_WIDTH_UNKNOWN)
					অনুৱর্ত;

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
	বাবে (row_group = table->children; row_group; row_group =row_group->next)
		বাবে (row = row_group->children; row; row = row->next)
			বাবে (cell = row->children; cell; cell = cell->next) {
				unsigned পূর্ণাঙ্ক fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				পূর্ণাঙ্ক fixed_width = 0, percent_width = 0;
				গণনা css_width_e type;
				css_fixed মান = 0;
				css_unit unit = CSS_UNIT_PX;

				যদি (cell->columns == 1)
					অনুৱর্ত;
				i = cell->start_column;

				বাবে (j = i; j < i + cell->columns; j++) {
					col[j].positioned = false;
				}

				/* count column types in spanned cells */
				বাবে (j = 0; j != cell->columns; j++) {
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
					অনুৱর্ত;

				type = css_computed_width(cell->style, &মান, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				যদি (type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					পূর্ণাঙ্ক width = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							মান, unit)) -
						fixed_width) / unknown_columns;
					যদি (width < 0)
						width = 0;
					বাবে (j = 0; j != cell->columns; j++) {
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
					পূর্ণাঙ্ক width = (FIXTOFLT(মান) -
						     percent_width) / unknown_columns;
					যদি (width < 0)
						width = 0;
					বাবে (j = 0; j != cell->columns; j++) {
						যদি (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_PERCENT;
							col[i + j].width = width;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	বাবে (i = 0; i != table->columns; i++) {
		যদি (col[i].type == COLUMN_WIDTH_UNKNOWN)
			col[i].type = COLUMN_WIDTH_AUTO;
	}

#ifdef TABLE_DEBUG
	বাবে (i = 0; i != table->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      table,
		      i,
		      ((ধ্রুৱ ৱর্ণ *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[col[i].type],
		      col[i].width);
#endif

	ঘূৰাই true;
}


/* exported interface documented in html/table.h */
শূন্য table_used_border_for_cell(ধ্রুৱ css_unit_ctx *unit_len_ctx, সংরচনা box *cell)
{
	পূর্ণাঙ্ক side;

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
		cell->border[শীৰ্ষ_2].style =
			css_computed_border_শীৰ্ষ_style(cell->style);
		css_computed_border_শীৰ্ষ_color(cell->style,
					      &cell->border[শীৰ্ষ_2].c);
		css_computed_border_শীৰ্ষ_width(cell->style, &width, &unit);
		cell->border[শীৰ্ষ_2].width =
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
		table_used_শীৰ্ষ_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		table_used_ডান_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		table_used_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	বাবে (side = 0; side != 4; side++) {
		যদি (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].width = 0;
	}
}
