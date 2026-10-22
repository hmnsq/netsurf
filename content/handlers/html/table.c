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

#شامل <assert.h>
#شامل <dom/dom.h>

#شامل "utils/log.h"
#شامل "utils/utils.h"
#شامل "utils/talloc.h"
#شامل "css/utils.h"

#شامل "html/box.h"
#شامل "html/table.h"

/* Define to enable verbose table debug */
#undef TABLE_DEBUG

/**
 * Container for border values during table border calculations
 */
ساختار border {
	شمارش css_border_style_e style;	/**< border-style */
	شمارش css_border_color_e color;	/**< border-color type */
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
ایستا bool
table_border_is_more_eyecatching(ثابت css_unit_ctx *unit_len_ctx,
				 ثابت ساختار border *a,
				 box_type a_src,
				 ثابت ساختار border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	صحیح impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	اگر (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		بازگشت false;

	اگر (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		بازگشت true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	أكد(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	أكد(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(پوچ, unit_len_ctx, a->width, a->unit);
	bwidth = css_unit_len2device_px(پوچ, unit_len_ctx, b->width, b->unit);

	اگر (awidth < bwidth)
		بازگشت true;
	وگرنه اگر (bwidth < awidth)
		بازگشت false;

	/* 3b -- sort by style */
	انتخاب (a->style) {
	حالت CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	حالت CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	حالت CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	حالت CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	حالت CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	حالت CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	حالت CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	حالت CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	پیشفرض:
		بشکن;
	}

	انتخاب (b->style) {
	حالت CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	حالت CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	حالت CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	حالت CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	حالت CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	حالت CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	حالت CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	حالت CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	پیشفرض:
		بشکن;
	}

	اگر (impact < 0)
		بازگشت true;
	وگرنه اگر (impact > 0)
		بازگشت false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	انتخاب (a_src) {
	حالت BOX_TABLE_CELL:       impact++; fallthrough;
	حالت BOX_TABLE_ROW:        impact++; fallthrough;
	حالت BOX_TABLE_ROW_GROUP:  impact++; fallthrough;
	حالت BOX_TABLE:            impact++; fallthrough;
	پیشفرض:
		بشکن;
	}

	/** \todo COL/COL_GROUP */
	انتخاب (b_src) {
	حالت BOX_TABLE_CELL:       impact--; fallthrough;
	حالت BOX_TABLE_ROW:        impact--; fallthrough;
	حالت BOX_TABLE_ROW_GROUP:  impact--; fallthrough;
	حالت BOX_TABLE:            impact--; fallthrough;
	پیشفرض:
		بشکن;
	}

	اگر (impact < 0)
		بازگشت true;
	وگرنه اگر (impact > 0)
		بازگشت false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	بازگشت true;
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
ایستا تهی
table_cell_بالاترین_process_table(ثابت css_unit_ctx *unit_len_ctx,
			     ساختار box *table,
			     ساختار border *a,
			     box_type *a_src)
{
	ساختار border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_بالاترین_style(table->style);
	b.color = css_computed_border_بالاترین_color(table->style, &b.c);
	css_computed_border_بالاترین_width(table->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(table->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE;

	اگر (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
ایستا bool
table_cell_بالاترین_process_row(ثابت css_unit_ctx *unit_len_ctx,
			   ساختار box *cell,
			   ساختار box *row,
			   ساختار border *a,
			   box_type *a_src)
{
	ساختار border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(row->style);
	b.color = css_computed_border_bottom_color(row->style, &b.c);
	css_computed_border_bottom_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	اگر (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	اگر (row->children == پوچ) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_بالاترین_style(row->style);
		b.color = css_computed_border_بالاترین_color(row->style, &b.c);
		css_computed_border_بالاترین_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		اگر (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		بازگشت false;
	} وگرنه {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		ساختار box *c;
		bool processed = false;

		وقتی (processed == false) {
			برای (c = row->children; c != پوچ; c = c->next) {
				/* Ignore cells to the left */
				اگر (c->start_column + c->columns - 1 <
				    cell->start_column)
					ادامه;
				/* Ignore cells to the right */
				اگر (c->start_column > cell->start_column +
				    cell->columns - 1)
					ادامه;

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

				اگر (table_border_is_more_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			اگر (processed == false) {
				/* There must be a preceding row */
				أكد(row->prev != پوچ);

				row = row->prev;
			}
		}
	}

	بازگشت true;
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
ایستا bool
table_cell_بالاترین_process_group(ثابت css_unit_ctx *unit_len_ctx,
			     ساختار box *cell,
			     ساختار box *group,
			     ساختار border *a,
			     box_type *a_src)
{
	ساختار border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(group->style);
	b.color = css_computed_border_bottom_color(group->style, &b.c);
	css_computed_border_bottom_width(group->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(group->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW_GROUP;

	اگر (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	اگر (group->last != پوچ) {
		/* Process rows in group, starting with last */
		ساختار box *row = group->last;

		وقتی (table_cell_بالاترین_process_row(unit_len_ctx, cell, row,
						  a, a_src) == false) {
			اگر (row->prev == پوچ) {
				بازگشت false;
			} وگرنه {
				row = row->prev;
			}
		}
	} وگرنه {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_بالاترین_style(group->style);
		b.color = css_computed_border_بالاترین_color(group->style, &b.c);
		css_computed_border_بالاترین_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		اگر (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		بازگشت false;
	}

	بازگشت true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ایستا تهی
table_used_چپ_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, ساختار box *cell)
{
	ساختار border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_چپ_style(cell->style);
	a.color = css_computed_border_چپ_color(cell->style, &a.c);
	css_computed_border_چپ_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	اگر (cell->prev != پوچ || cell->start_column != 0) {
		/* Cell to the left -- consider its right border */
		ساختار box *prev = پوچ;

		اگر (cell->prev == پوچ) {
			ساختار box *row;

			/* Spanned from a previous row in current row group */
			برای (row = cell->parent; row != پوچ; row = row->prev) {
				برای (prev = row->children; prev != پوچ;
				     prev = prev->next) {
					اگر (prev->start_column +
					    prev->columns ==
					    cell->start_column)
						بشکن;
				}

				اگر (prev != پوچ)
					بشکن;
			}

			أكد(prev != پوچ);
		} وگرنه {
			prev = cell->prev;
		}

		b.style = css_computed_border_راست_style(prev->style);
		b.color = css_computed_border_راست_color(prev->style, &b.c);
		css_computed_border_راست_width(prev->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_CELL;

		اگر (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} وگرنه {
		/* First cell in row, so consider rows and row group */
		ساختار box *row = cell->parent;
		ساختار box *group = row->parent;
		ساختار box *table = group->parent;
		unsigned صحیح rows = cell->rows;

		وقتی (rows-- > 0 && row != پوچ) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_چپ_style(row->style);
			b.color = css_computed_border_چپ_color(
								 row->style, &b.c);
			css_computed_border_چپ_width(
						       row->style, &b.width, &b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			اگر (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_چپ_style(group->style);
		b.color = css_computed_border_چپ_color(group->style, &b.c);
		css_computed_border_چپ_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		اگر (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_چپ_style(table->style);
		b.color = css_computed_border_چپ_color(table->style, &b.c);
		css_computed_border_چپ_width(table->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		اگر (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[چپ_2].style = a.style;
	cell->border[چپ_2].c = a.c;
	cell->border[چپ_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ایستا تهی
table_used_بالاترین_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, ساختار box *cell)
{
	ساختار border a, b;
	box_type a_src, b_src;
	ساختار box *row = cell->parent;
	bool process_group = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_بالاترین_style(cell->style);
	css_computed_border_بالاترین_color(cell->style, &a.c);
	css_computed_border_بالاترین_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	/* Top border of row */
	b.style = css_computed_border_بالاترین_style(row->style);
	css_computed_border_بالاترین_color(row->style, &b.c);
	css_computed_border_بالاترین_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	اگر (table_border_is_more_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	اگر (row->prev != پوچ) {
		/* Consider row(s) above */
		وقتی (table_cell_بالاترین_process_row(unit_len_ctx, cell, row->prev,
						  &a, &a_src) == false) {
			اگر (row->prev->prev == پوچ) {
				/* Consider row group */
				process_group = true;
				بشکن;
			} وگرنه {
				row = row->prev;
			}
		}
	} وگرنه {
		process_group = true;
	}

	اگر (process_group) {
		ساختار box *group = row->parent;

		/* Top border of row group */
		b.style = css_computed_border_بالاترین_style(group->style);
		b.color = css_computed_border_بالاترین_color(group->style, &b.c);
		css_computed_border_بالاترین_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		اگر (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		اگر (group->prev == پوچ) {
			/* Top border of table */
			table_cell_بالاترین_process_table(unit_len_ctx,
						     group->parent, &a, &a_src);
		} وگرنه {
			/* Process previous group(s) */
			وقتی (table_cell_بالاترین_process_group(unit_len_ctx,
							    cell, group->prev,
							    &a, &a_src) == false) {
				اگر (group->prev->prev == پوچ) {
					/* Top border of table */
					table_cell_بالاترین_process_table(unit_len_ctx,
								     group->parent,
								     &a, &a_src);
					بشکن;
				} وگرنه {
					group = group->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[بالاترین_2].style = a.style;
	cell->border[بالاترین_2].c = a.c;
	cell->border[بالاترین_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ایستا تهی
table_used_راست_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, ساختار box *cell)
{
	ساختار border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_راست_style(cell->style);
	css_computed_border_راست_color(cell->style, &a.c);
	css_computed_border_راست_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	اگر (cell->next != پوچ || cell->start_column + cell->columns !=
	    cell->parent->parent->parent->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} وگرنه {
		/* Last cell in row, so consider rows and row group */
		ساختار box *row = cell->parent;
		ساختار box *group = row->parent;
		ساختار box *table = group->parent;
		unsigned صحیح rows = cell->rows;

		وقتی (rows-- > 0 && row != پوچ) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_راست_style(row->style);
			b.color = css_computed_border_راست_color(row->style,
								  &b.c);
			css_computed_border_راست_width(row->style,
							&b.width,
							&b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			اگر (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_راست_style(group->style);
		b.color = css_computed_border_راست_color(group->style, &b.c);
		css_computed_border_راست_width(group->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		اگر (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_راست_style(table->style);
		b.color = css_computed_border_راست_color(table->style, &b.c);
		css_computed_border_راست_width(table->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		اگر (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[راست_2].style = a.style;
	cell->border[راست_2].c = a.c;
	cell->border[راست_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ایستا تهی
table_used_bottom_border_for_cell(ثابت css_unit_ctx *unit_len_ctx,
				  ساختار box *cell)
{
	ساختار border a, b;
	box_type a_src, b_src;
	ساختار box *row = cell->parent;
	unsigned صحیح rows = cell->rows;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	وقتی (rows-- > 0 && row != پوچ)
		row = row->next;

	/** \todo Can cells span row groups? */

	اگر (row != پوچ) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} وگرنه {
		/* Cell at bottom of table, so consider row and row group */
		ساختار box *row = cell->parent;
		ساختار box *group = row->parent;
		ساختار box *table = group->parent;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(row->style);
		b.color = css_computed_border_bottom_color(row->style, &b.c);
		css_computed_border_bottom_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		اگر (table_border_is_more_eyecatching(unit_len_ctx,
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

		اگر (table_border_is_more_eyecatching(unit_len_ctx,
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

		اگر (table_border_is_more_eyecatching(unit_len_ctx,
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
table_calculate_column_types(ثابت css_unit_ctx *unit_len_ctx, ساختار box *table)
{
	unsigned صحیح i, j;
	ساختار column *col;
	ساختار box *row_group, *row, *cell;

	اگر (table->col)
		/* table->col already constructed, for example frameset table */
		بازگشت true;

	table->col = col = talloc_array(table, ساختار column, table->columns);
	اگر (!col)
		بازگشت false;

	برای (i = 0; i != table->columns; i++) {
		col[i].type = COLUMN_WIDTH_UNKNOWN;
		col[i].width = 0;
		col[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	برای (row_group = table->children; row_group; row_group =row_group->next)
		برای (row = row_group->children; row; row = row->next)
			برای (cell = row->children; cell; cell = cell->next) {
				شمارش css_width_e type;
				css_fixed مقدار = 0;
				css_unit unit = CSS_UNIT_PX;

				أكد(cell->type == BOX_TABLE_CELL);
				أكد(cell->style);

				اگر (cell->columns != 1)
					ادامه;
				i = cell->start_column;

				اگر (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					col[i].positioned = false;
				}

				type = css_computed_width(cell->style, &مقدار, &unit);

				/* fixed width takes priority over any other width type */
				اگر (col[i].type != COLUMN_WIDTH_FIXED &&
				    type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_FIXED;
					col[i].width = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							مقدار, unit));
					اگر (col[i].width < 0)
						col[i].width = 0;
					ادامه;
				}

				اگر (col[i].type != COLUMN_WIDTH_UNKNOWN)
					ادامه;

				اگر (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_PERCENT;
					col[i].width = FIXTOINT(مقدار);
					اگر (col[i].width < 0)
						col[i].width = 0;
				} وگرنه اگر (type == CSS_WIDTH_AUTO) {
					col[i].type = COLUMN_WIDTH_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	برای (row_group = table->children; row_group; row_group =row_group->next)
		برای (row = row_group->children; row; row = row->next)
			برای (cell = row->children; cell; cell = cell->next) {
				unsigned صحیح fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				صحیح fixed_width = 0, percent_width = 0;
				شمارش css_width_e type;
				css_fixed مقدار = 0;
				css_unit unit = CSS_UNIT_PX;

				اگر (cell->columns == 1)
					ادامه;
				i = cell->start_column;

				برای (j = i; j < i + cell->columns; j++) {
					col[j].positioned = false;
				}

				/* count column types in spanned cells */
				برای (j = 0; j != cell->columns; j++) {
					اگر (col[i + j].type == COLUMN_WIDTH_FIXED) {
						fixed_width += col[i + j].width;
						fixed_columns++;
					} وگرنه اگر (col[i + j].type == COLUMN_WIDTH_PERCENT) {
						percent_width += col[i + j].width;
						percent_columns++;
					} وگرنه اگر (col[i + j].type == COLUMN_WIDTH_AUTO) {
						auto_columns++;
					} وگرنه {
						unknown_columns++;
					}
				}

				اگر (!unknown_columns)
					ادامه;

				type = css_computed_width(cell->style, &مقدار, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				اگر (type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					صحیح width = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							مقدار, unit)) -
						fixed_width) / unknown_columns;
					اگر (width < 0)
						width = 0;
					برای (j = 0; j != cell->columns; j++) {
						اگر (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_FIXED;
							col[i + j].width = width;
						}
					}
				}

				/* as above for percentage width */
				اگر (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					صحیح width = (FIXTOFLT(مقدار) -
						     percent_width) / unknown_columns;
					اگر (width < 0)
						width = 0;
					برای (j = 0; j != cell->columns; j++) {
						اگر (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_PERCENT;
							col[i + j].width = width;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	برای (i = 0; i != table->columns; i++) {
		اگر (col[i].type == COLUMN_WIDTH_UNKNOWN)
			col[i].type = COLUMN_WIDTH_AUTO;
	}

#ifdef TABLE_DEBUG
	برای (i = 0; i != table->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      table,
		      i,
		      ((ثابت نویسه *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[col[i].type],
		      col[i].width);
#endif

	بازگشت true;
}


/* exported interface documented in html/table.h */
تهی table_used_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, ساختار box *cell)
{
	صحیح side;

	أكد(cell->type == BOX_TABLE_CELL);

	اگر (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed width = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[چپ_2].style =
			css_computed_border_چپ_style(cell->style);
		css_computed_border_چپ_color(cell->style,
					       &cell->border[چپ_2].c);
		css_computed_border_چپ_width(cell->style, &width, &unit);
		cell->border[چپ_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Top border */
		cell->border[بالاترین_2].style =
			css_computed_border_بالاترین_style(cell->style);
		css_computed_border_بالاترین_color(cell->style,
					      &cell->border[بالاترین_2].c);
		css_computed_border_بالاترین_width(cell->style, &width, &unit);
		cell->border[بالاترین_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Right border */
		cell->border[راست_2].style =
			css_computed_border_راست_style(cell->style);
		css_computed_border_راست_color(cell->style,
						&cell->border[راست_2].c);
		css_computed_border_راست_width(cell->style, &width, &unit);
		cell->border[راست_2].width =
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
	} وگرنه {
		/* Left border */
		table_used_چپ_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		table_used_بالاترین_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		table_used_راست_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		table_used_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	برای (side = 0; side != 4; side++) {
		اگر (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].width = 0;
	}
}
