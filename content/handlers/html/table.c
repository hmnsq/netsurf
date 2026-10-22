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

#ضمن <assert.h>
#ضمن <dom/dom.h>

#ضمن "utils/log.h"
#ضمن "utils/utils.h"
#ضمن "utils/talloc.h"
#ضمن "css/utils.h"

#ضمن "html/box.h"
#ضمن "html/table.h"

/* Define to enable verbose table debug */
#undef TABLE_DEBUG

/**
 * Container for border values during table border calculations
 */
بنية border {
	تعداد css_border_style_e style;	/**< border-style */
	تعداد css_border_color_e color;	/**< border-color type */
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
ساكن bool
table_border_is_more_eyecatching(ثابت css_unit_ctx *unit_len_ctx,
				 ثابت بنية border *a,
				 box_type a_src,
				 ثابت بنية border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	صحيح impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	אם (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		החזר false;

	אם (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		החזר true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	أكد(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	أكد(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(عدم, unit_len_ctx, a->width, a->unit);
	bwidth = css_unit_len2device_px(عدم, unit_len_ctx, b->width, b->unit);

	אם (awidth < bwidth)
		החזר true;
	אחרת אם (bwidth < awidth)
		החזר false;

	/* 3b -- sort by style */
	بدل (a->style) {
	حالة CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	حالة CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	حالة CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	حالة CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	حالة CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	حالة CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	حالة CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	حالة CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	افتراضي:
		اكسر;
	}

	بدل (b->style) {
	حالة CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	حالة CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	حالة CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	حالة CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	حالة CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	حالة CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	حالة CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	حالة CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	افتراضي:
		اكسر;
	}

	אם (impact < 0)
		החזר true;
	אחרת אם (impact > 0)
		החזר false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	بدل (a_src) {
	حالة BOX_TABLE_CELL:       impact++; fallthrough;
	حالة BOX_TABLE_ROW:        impact++; fallthrough;
	حالة BOX_TABLE_ROW_GROUP:  impact++; fallthrough;
	حالة BOX_TABLE:            impact++; fallthrough;
	افتراضي:
		اكسر;
	}

	/** \todo COL/COL_GROUP */
	بدل (b_src) {
	حالة BOX_TABLE_CELL:       impact--; fallthrough;
	حالة BOX_TABLE_ROW:        impact--; fallthrough;
	حالة BOX_TABLE_ROW_GROUP:  impact--; fallthrough;
	حالة BOX_TABLE:            impact--; fallthrough;
	افتراضي:
		اكسر;
	}

	אם (impact < 0)
		החזר true;
	אחרת אם (impact > 0)
		החזר false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	החזר true;
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
ساكن فراغ
table_cell_top_process_table(ثابت css_unit_ctx *unit_len_ctx,
			     بنية box *table,
			     بنية border *a,
			     box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_top_style(table->style);
	b.color = css_computed_border_top_color(table->style, &b.c);
	css_computed_border_top_width(table->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(table->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE;

	אם (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
ساكن bool
table_cell_top_process_row(ثابت css_unit_ctx *unit_len_ctx,
			   بنية box *cell,
			   بنية box *row,
			   بنية border *a,
			   box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(row->style);
	b.color = css_computed_border_bottom_color(row->style, &b.c);
	css_computed_border_bottom_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	אם (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	אם (row->children == عدم) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_top_style(row->style);
		b.color = css_computed_border_top_color(row->style, &b.c);
		css_computed_border_top_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		אם (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		החזר false;
	} אחרת {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		بنية box *c;
		bool processed = false;

		כלעוד (processed == false) {
			עבור (c = row->children; c != عدم; c = c->next) {
				/* Ignore cells to the left */
				אם (c->start_column + c->columns - 1 <
				    cell->start_column)
					استمر;
				/* Ignore cells to the right */
				אם (c->start_column > cell->start_column +
				    cell->columns - 1)
					استمر;

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

				אם (table_border_is_more_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			אם (processed == false) {
				/* There must be a preceding row */
				أكد(row->prev != عدم);

				row = row->prev;
			}
		}
	}

	החזר true;
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
ساكن bool
table_cell_top_process_group(ثابت css_unit_ctx *unit_len_ctx,
			     بنية box *cell,
			     بنية box *group,
			     بنية border *a,
			     box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(group->style);
	b.color = css_computed_border_bottom_color(group->style, &b.c);
	css_computed_border_bottom_width(group->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(group->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW_GROUP;

	אם (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	אם (group->last != عدم) {
		/* Process rows in group, starting with last */
		بنية box *row = group->last;

		כלעוד (table_cell_top_process_row(unit_len_ctx, cell, row,
						  a, a_src) == false) {
			אם (row->prev == عدم) {
				החזר false;
			} אחרת {
				row = row->prev;
			}
		}
	} אחרת {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_top_style(group->style);
		b.color = css_computed_border_top_color(group->style, &b.c);
		css_computed_border_top_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		אם (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		החזר false;
	}

	החזר true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
table_used_left_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_left_style(cell->style);
	a.color = css_computed_border_left_color(cell->style, &a.c);
	css_computed_border_left_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	אם (cell->prev != عدم || cell->start_column != 0) {
		/* Cell to the left -- consider its right border */
		بنية box *prev = عدم;

		אם (cell->prev == عدم) {
			بنية box *row;

			/* Spanned from a previous row in current row group */
			עבור (row = cell->parent; row != عدم; row = row->prev) {
				עבור (prev = row->children; prev != عدم;
				     prev = prev->next) {
					אם (prev->start_column +
					    prev->columns ==
					    cell->start_column)
						اكسر;
				}

				אם (prev != عدم)
					اكسر;
			}

			أكد(prev != عدم);
		} אחרת {
			prev = cell->prev;
		}

		b.style = css_computed_border_right_style(prev->style);
		b.color = css_computed_border_right_color(prev->style, &b.c);
		css_computed_border_right_width(prev->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_CELL;

		אם (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} אחרת {
		/* First cell in row, so consider rows and row group */
		بنية box *row = cell->parent;
		بنية box *group = row->parent;
		بنية box *table = group->parent;
		unsigned صحيح rows = cell->rows;

		כלעוד (rows-- > 0 && row != عدم) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_left_style(row->style);
			b.color = css_computed_border_left_color(
								 row->style, &b.c);
			css_computed_border_left_width(
						       row->style, &b.width, &b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			אם (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_left_style(group->style);
		b.color = css_computed_border_left_color(group->style, &b.c);
		css_computed_border_left_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		אם (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_left_style(table->style);
		b.color = css_computed_border_left_color(table->style, &b.c);
		css_computed_border_left_width(table->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		אם (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[LEFT].style = a.style;
	cell->border[LEFT].c = a.c;
	cell->border[LEFT].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
table_used_top_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;
	بنية box *row = cell->parent;
	bool process_group = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_top_style(cell->style);
	css_computed_border_top_color(cell->style, &a.c);
	css_computed_border_top_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	/* Top border of row */
	b.style = css_computed_border_top_style(row->style);
	css_computed_border_top_color(row->style, &b.c);
	css_computed_border_top_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	אם (table_border_is_more_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	אם (row->prev != عدم) {
		/* Consider row(s) above */
		כלעוד (table_cell_top_process_row(unit_len_ctx, cell, row->prev,
						  &a, &a_src) == false) {
			אם (row->prev->prev == عدم) {
				/* Consider row group */
				process_group = true;
				اكسر;
			} אחרת {
				row = row->prev;
			}
		}
	} אחרת {
		process_group = true;
	}

	אם (process_group) {
		بنية box *group = row->parent;

		/* Top border of row group */
		b.style = css_computed_border_top_style(group->style);
		b.color = css_computed_border_top_color(group->style, &b.c);
		css_computed_border_top_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		אם (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		אם (group->prev == عدم) {
			/* Top border of table */
			table_cell_top_process_table(unit_len_ctx,
						     group->parent, &a, &a_src);
		} אחרת {
			/* Process previous group(s) */
			כלעוד (table_cell_top_process_group(unit_len_ctx,
							    cell, group->prev,
							    &a, &a_src) == false) {
				אם (group->prev->prev == عدم) {
					/* Top border of table */
					table_cell_top_process_table(unit_len_ctx,
								     group->parent,
								     &a, &a_src);
					اكسر;
				} אחרת {
					group = group->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[TOP].style = a.style;
	cell->border[TOP].c = a.c;
	cell->border[TOP].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
table_used_right_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_right_style(cell->style);
	css_computed_border_right_color(cell->style, &a.c);
	css_computed_border_right_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	אם (cell->next != عدم || cell->start_column + cell->columns !=
	    cell->parent->parent->parent->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} אחרת {
		/* Last cell in row, so consider rows and row group */
		بنية box *row = cell->parent;
		بنية box *group = row->parent;
		بنية box *table = group->parent;
		unsigned صحيح rows = cell->rows;

		כלעוד (rows-- > 0 && row != عدم) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_right_style(row->style);
			b.color = css_computed_border_right_color(row->style,
								  &b.c);
			css_computed_border_right_width(row->style,
							&b.width,
							&b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			אם (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_right_style(group->style);
		b.color = css_computed_border_right_color(group->style, &b.c);
		css_computed_border_right_width(group->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		אם (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_right_style(table->style);
		b.color = css_computed_border_right_color(table->style, &b.c);
		css_computed_border_right_width(table->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		אם (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[RIGHT].style = a.style;
	cell->border[RIGHT].c = a.c;
	cell->border[RIGHT].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
table_used_bottom_border_for_cell(ثابت css_unit_ctx *unit_len_ctx,
				  بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;
	بنية box *row = cell->parent;
	unsigned صحيح rows = cell->rows;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	כלעוד (rows-- > 0 && row != عدم)
		row = row->next;

	/** \todo Can cells span row groups? */

	אם (row != عدم) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} אחרת {
		/* Cell at bottom of table, so consider row and row group */
		بنية box *row = cell->parent;
		بنية box *group = row->parent;
		بنية box *table = group->parent;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(row->style);
		b.color = css_computed_border_bottom_color(row->style, &b.c);
		css_computed_border_bottom_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		אם (table_border_is_more_eyecatching(unit_len_ctx,
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

		אם (table_border_is_more_eyecatching(unit_len_ctx,
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

		אם (table_border_is_more_eyecatching(unit_len_ctx,
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
table_calculate_column_types(ثابت css_unit_ctx *unit_len_ctx, بنية box *table)
{
	unsigned صحيح i, j;
	بنية column *col;
	بنية box *row_group, *row, *cell;

	אם (table->col)
		/* table->col already constructed, for example frameset table */
		החזר true;

	table->col = col = talloc_array(table, بنية column, table->columns);
	אם (!col)
		החזר false;

	עבור (i = 0; i != table->columns; i++) {
		col[i].type = COLUMN_WIDTH_UNKNOWN;
		col[i].width = 0;
		col[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	עבור (row_group = table->children; row_group; row_group =row_group->next)
		עבור (row = row_group->children; row; row = row->next)
			עבור (cell = row->children; cell; cell = cell->next) {
				تعداد css_width_e type;
				css_fixed value = 0;
				css_unit unit = CSS_UNIT_PX;

				أكد(cell->type == BOX_TABLE_CELL);
				أكد(cell->style);

				אם (cell->columns != 1)
					استمر;
				i = cell->start_column;

				אם (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					col[i].positioned = false;
				}

				type = css_computed_width(cell->style, &value, &unit);

				/* fixed width takes priority over any other width type */
				אם (col[i].type != COLUMN_WIDTH_FIXED &&
				    type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_FIXED;
					col[i].width = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							value, unit));
					אם (col[i].width < 0)
						col[i].width = 0;
					استمر;
				}

				אם (col[i].type != COLUMN_WIDTH_UNKNOWN)
					استمر;

				אם (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_PERCENT;
					col[i].width = FIXTOINT(value);
					אם (col[i].width < 0)
						col[i].width = 0;
				} אחרת אם (type == CSS_WIDTH_AUTO) {
					col[i].type = COLUMN_WIDTH_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	עבור (row_group = table->children; row_group; row_group =row_group->next)
		עבור (row = row_group->children; row; row = row->next)
			עבור (cell = row->children; cell; cell = cell->next) {
				unsigned صحيح fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				صحيح fixed_width = 0, percent_width = 0;
				تعداد css_width_e type;
				css_fixed value = 0;
				css_unit unit = CSS_UNIT_PX;

				אם (cell->columns == 1)
					استمر;
				i = cell->start_column;

				עבור (j = i; j < i + cell->columns; j++) {
					col[j].positioned = false;
				}

				/* count column types in spanned cells */
				עבור (j = 0; j != cell->columns; j++) {
					אם (col[i + j].type == COLUMN_WIDTH_FIXED) {
						fixed_width += col[i + j].width;
						fixed_columns++;
					} אחרת אם (col[i + j].type == COLUMN_WIDTH_PERCENT) {
						percent_width += col[i + j].width;
						percent_columns++;
					} אחרת אם (col[i + j].type == COLUMN_WIDTH_AUTO) {
						auto_columns++;
					} אחרת {
						unknown_columns++;
					}
				}

				אם (!unknown_columns)
					استمر;

				type = css_computed_width(cell->style, &value, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				אם (type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					صحيح width = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							value, unit)) -
						fixed_width) / unknown_columns;
					אם (width < 0)
						width = 0;
					עבור (j = 0; j != cell->columns; j++) {
						אם (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_FIXED;
							col[i + j].width = width;
						}
					}
				}

				/* as above for percentage width */
				אם (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					صحيح width = (FIXTOFLT(value) -
						     percent_width) / unknown_columns;
					אם (width < 0)
						width = 0;
					עבור (j = 0; j != cell->columns; j++) {
						אם (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_PERCENT;
							col[i + j].width = width;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	עבור (i = 0; i != table->columns; i++) {
		אם (col[i].type == COLUMN_WIDTH_UNKNOWN)
			col[i].type = COLUMN_WIDTH_AUTO;
	}

#ifdef TABLE_DEBUG
	עבור (i = 0; i != table->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      table,
		      i,
		      ((ثابت حرف *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[col[i].type],
		      col[i].width);
#endif

	החזר true;
}


/* exported interface documented in html/table.h */
فراغ table_used_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	صحيح side;

	أكد(cell->type == BOX_TABLE_CELL);

	אם (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed width = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[LEFT].style =
			css_computed_border_left_style(cell->style);
		css_computed_border_left_color(cell->style,
					       &cell->border[LEFT].c);
		css_computed_border_left_width(cell->style, &width, &unit);
		cell->border[LEFT].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Top border */
		cell->border[TOP].style =
			css_computed_border_top_style(cell->style);
		css_computed_border_top_color(cell->style,
					      &cell->border[TOP].c);
		css_computed_border_top_width(cell->style, &width, &unit);
		cell->border[TOP].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Right border */
		cell->border[RIGHT].style =
			css_computed_border_right_style(cell->style);
		css_computed_border_right_color(cell->style,
						&cell->border[RIGHT].c);
		css_computed_border_right_width(cell->style, &width, &unit);
		cell->border[RIGHT].width =
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
	} אחרת {
		/* Left border */
		table_used_left_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		table_used_top_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		table_used_right_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		table_used_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	עבור (side = 0; side != 4; side++) {
		אם (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].width = 0;
	}
}
