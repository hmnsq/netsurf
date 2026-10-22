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
	إذا (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		أرجع false;

	إذا (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		أرجع true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	أكد(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	أكد(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(عدم, unit_len_ctx, a->width, a->unit);
	bwidth = css_unit_len2device_px(عدم, unit_len_ctx, b->width, b->unit);

	إذا (awidth < bwidth)
		أرجع true;
	وإلا إذا (bwidth < awidth)
		أرجع false;

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

	إذا (impact < 0)
		أرجع true;
	وإلا إذا (impact > 0)
		أرجع false;

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

	إذا (impact < 0)
		أرجع true;
	وإلا إذا (impact > 0)
		أرجع false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	أرجع true;
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
table_cell_اتس_process_table(ثابت css_unit_ctx *unit_len_ctx,
			     بنية box *table,
			     بنية border *a,
			     box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_اتس_style(table->style);
	b.color = css_computed_border_اتس_color(table->style, &b.c);
	css_computed_border_اتس_width(table->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(table->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE;

	إذا (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
table_cell_اتس_process_row(ثابت css_unit_ctx *unit_len_ctx,
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

	إذا (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	إذا (row->children == عدم) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_اتس_style(row->style);
		b.color = css_computed_border_اتس_color(row->style, &b.c);
		css_computed_border_اتس_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		إذا (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		أرجع false;
	} وإلا {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		بنية box *c;
		bool processed = false;

		طالما (processed == false) {
			لكل (c = row->children; c != عدم; c = c->next) {
				/* Ignore cells to the left */
				إذا (c->start_column + c->columns - 1 <
				    cell->start_column)
					استمر;
				/* Ignore cells to the right */
				إذا (c->start_column > cell->start_column +
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

				إذا (table_border_is_more_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			إذا (processed == false) {
				/* There must be a preceding row */
				أكد(row->prev != عدم);

				row = row->prev;
			}
		}
	}

	أرجع true;
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
table_cell_اتس_process_group(ثابت css_unit_ctx *unit_len_ctx,
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

	إذا (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	إذا (group->last != عدم) {
		/* Process rows in group, starting with last */
		بنية box *row = group->last;

		طالما (table_cell_اتس_process_row(unit_len_ctx, cell, row,
						  a, a_src) == false) {
			إذا (row->prev == عدم) {
				أرجع false;
			} وإلا {
				row = row->prev;
			}
		}
	} وإلا {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_اتس_style(group->style);
		b.color = css_computed_border_اتس_color(group->style, &b.c);
		css_computed_border_اتس_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		إذا (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		أرجع false;
	}

	أرجع true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
table_used_کيري_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_کيري_style(cell->style);
	a.color = css_computed_border_کيري_color(cell->style, &a.c);
	css_computed_border_کيري_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	إذا (cell->prev != عدم || cell->start_column != 0) {
		/* Cell to the left -- consider its right border */
		بنية box *prev = عدم;

		إذا (cell->prev == عدم) {
			بنية box *row;

			/* Spanned from a previous row in current row group */
			لكل (row = cell->parent; row != عدم; row = row->prev) {
				لكل (prev = row->children; prev != عدم;
				     prev = prev->next) {
					إذا (prev->start_column +
					    prev->columns ==
					    cell->start_column)
						اكسر;
				}

				إذا (prev != عدم)
					اكسر;
			}

			أكد(prev != عدم);
		} وإلا {
			prev = cell->prev;
		}

		b.style = css_computed_border_کانن_style(prev->style);
		b.color = css_computed_border_کانن_color(prev->style, &b.c);
		css_computed_border_کانن_width(prev->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_CELL;

		إذا (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} وإلا {
		/* First cell in row, so consider rows and row group */
		بنية box *row = cell->parent;
		بنية box *group = row->parent;
		بنية box *table = group->parent;
		unsigned صحيح rows = cell->rows;

		طالما (rows-- > 0 && row != عدم) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_کيري_style(row->style);
			b.color = css_computed_border_کيري_color(
								 row->style, &b.c);
			css_computed_border_کيري_width(
						       row->style, &b.width, &b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			إذا (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_کيري_style(group->style);
		b.color = css_computed_border_کيري_color(group->style, &b.c);
		css_computed_border_کيري_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		إذا (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_کيري_style(table->style);
		b.color = css_computed_border_کيري_color(table->style, &b.c);
		css_computed_border_کيري_width(table->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		إذا (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[کيري_2].style = a.style;
	cell->border[کيري_2].c = a.c;
	cell->border[کيري_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
table_used_اتس_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;
	بنية box *row = cell->parent;
	bool process_group = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_اتس_style(cell->style);
	css_computed_border_اتس_color(cell->style, &a.c);
	css_computed_border_اتس_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	/* Top border of row */
	b.style = css_computed_border_اتس_style(row->style);
	css_computed_border_اتس_color(row->style, &b.c);
	css_computed_border_اتس_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	إذا (table_border_is_more_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	إذا (row->prev != عدم) {
		/* Consider row(s) above */
		طالما (table_cell_اتس_process_row(unit_len_ctx, cell, row->prev,
						  &a, &a_src) == false) {
			إذا (row->prev->prev == عدم) {
				/* Consider row group */
				process_group = true;
				اكسر;
			} وإلا {
				row = row->prev;
			}
		}
	} وإلا {
		process_group = true;
	}

	إذا (process_group) {
		بنية box *group = row->parent;

		/* Top border of row group */
		b.style = css_computed_border_اتس_style(group->style);
		b.color = css_computed_border_اتس_color(group->style, &b.c);
		css_computed_border_اتس_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		إذا (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		إذا (group->prev == عدم) {
			/* Top border of table */
			table_cell_اتس_process_table(unit_len_ctx,
						     group->parent, &a, &a_src);
		} وإلا {
			/* Process previous group(s) */
			طالما (table_cell_اتس_process_group(unit_len_ctx,
							    cell, group->prev,
							    &a, &a_src) == false) {
				إذا (group->prev->prev == عدم) {
					/* Top border of table */
					table_cell_اتس_process_table(unit_len_ctx,
								     group->parent,
								     &a, &a_src);
					اكسر;
				} وإلا {
					group = group->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[اتس_2].style = a.style;
	cell->border[اتس_2].c = a.c;
	cell->border[اتس_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
table_used_کانن_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_کانن_style(cell->style);
	css_computed_border_کانن_color(cell->style, &a.c);
	css_computed_border_کانن_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	إذا (cell->next != عدم || cell->start_column + cell->columns !=
	    cell->parent->parent->parent->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} وإلا {
		/* Last cell in row, so consider rows and row group */
		بنية box *row = cell->parent;
		بنية box *group = row->parent;
		بنية box *table = group->parent;
		unsigned صحيح rows = cell->rows;

		طالما (rows-- > 0 && row != عدم) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_کانن_style(row->style);
			b.color = css_computed_border_کانن_color(row->style,
								  &b.c);
			css_computed_border_کانن_width(row->style,
							&b.width,
							&b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			إذا (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_کانن_style(group->style);
		b.color = css_computed_border_کانن_color(group->style, &b.c);
		css_computed_border_کانن_width(group->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		إذا (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_کانن_style(table->style);
		b.color = css_computed_border_کانن_color(table->style, &b.c);
		css_computed_border_کانن_width(table->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		إذا (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[کانن_2].style = a.style;
	cell->border[کانن_2].c = a.c;
	cell->border[کانن_2].width = FIXTOINT(css_unit_len2device_px(
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

	طالما (rows-- > 0 && row != عدم)
		row = row->next;

	/** \todo Can cells span row groups? */

	إذا (row != عدم) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} وإلا {
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

		إذا (table_border_is_more_eyecatching(unit_len_ctx,
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

		إذا (table_border_is_more_eyecatching(unit_len_ctx,
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

		إذا (table_border_is_more_eyecatching(unit_len_ctx,
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

	إذا (table->col)
		/* table->col already constructed, for example frameset table */
		أرجع true;

	table->col = col = talloc_array(table, بنية column, table->columns);
	إذا (!col)
		أرجع false;

	لكل (i = 0; i != table->columns; i++) {
		col[i].type = COLUMN_WIDTH_UNKNOWN;
		col[i].width = 0;
		col[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	لكل (row_group = table->children; row_group; row_group =row_group->next)
		لكل (row = row_group->children; row; row = row->next)
			لكل (cell = row->children; cell; cell = cell->next) {
				تعداد css_width_e type;
				css_fixed نيلاي = 0;
				css_unit unit = CSS_UNIT_PX;

				أكد(cell->type == BOX_TABLE_CELL);
				أكد(cell->style);

				إذا (cell->columns != 1)
					استمر;
				i = cell->start_column;

				إذا (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					col[i].positioned = false;
				}

				type = css_computed_width(cell->style, &نيلاي, &unit);

				/* fixed width takes priority over any other width type */
				إذا (col[i].type != COLUMN_WIDTH_FIXED &&
				    type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_FIXED;
					col[i].width = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							نيلاي, unit));
					إذا (col[i].width < 0)
						col[i].width = 0;
					استمر;
				}

				إذا (col[i].type != COLUMN_WIDTH_UNKNOWN)
					استمر;

				إذا (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_PERCENT;
					col[i].width = FIXTOINT(نيلاي);
					إذا (col[i].width < 0)
						col[i].width = 0;
				} وإلا إذا (type == CSS_WIDTH_AUTO) {
					col[i].type = COLUMN_WIDTH_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	لكل (row_group = table->children; row_group; row_group =row_group->next)
		لكل (row = row_group->children; row; row = row->next)
			لكل (cell = row->children; cell; cell = cell->next) {
				unsigned صحيح fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				صحيح fixed_width = 0, percent_width = 0;
				تعداد css_width_e type;
				css_fixed نيلاي = 0;
				css_unit unit = CSS_UNIT_PX;

				إذا (cell->columns == 1)
					استمر;
				i = cell->start_column;

				لكل (j = i; j < i + cell->columns; j++) {
					col[j].positioned = false;
				}

				/* count column types in spanned cells */
				لكل (j = 0; j != cell->columns; j++) {
					إذا (col[i + j].type == COLUMN_WIDTH_FIXED) {
						fixed_width += col[i + j].width;
						fixed_columns++;
					} وإلا إذا (col[i + j].type == COLUMN_WIDTH_PERCENT) {
						percent_width += col[i + j].width;
						percent_columns++;
					} وإلا إذا (col[i + j].type == COLUMN_WIDTH_AUTO) {
						auto_columns++;
					} وإلا {
						unknown_columns++;
					}
				}

				إذا (!unknown_columns)
					استمر;

				type = css_computed_width(cell->style, &نيلاي, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				إذا (type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					صحيح width = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							نيلاي, unit)) -
						fixed_width) / unknown_columns;
					إذا (width < 0)
						width = 0;
					لكل (j = 0; j != cell->columns; j++) {
						إذا (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_FIXED;
							col[i + j].width = width;
						}
					}
				}

				/* as above for percentage width */
				إذا (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					صحيح width = (FIXTOFLT(نيلاي) -
						     percent_width) / unknown_columns;
					إذا (width < 0)
						width = 0;
					لكل (j = 0; j != cell->columns; j++) {
						إذا (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_PERCENT;
							col[i + j].width = width;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	لكل (i = 0; i != table->columns; i++) {
		إذا (col[i].type == COLUMN_WIDTH_UNKNOWN)
			col[i].type = COLUMN_WIDTH_AUTO;
	}

#ifdef TABLE_DEBUG
	لكل (i = 0; i != table->columns; i++)
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

	أرجع true;
}


/* exported interface documented in html/table.h */
فراغ table_used_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	صحيح side;

	أكد(cell->type == BOX_TABLE_CELL);

	إذا (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed width = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[کيري_2].style =
			css_computed_border_کيري_style(cell->style);
		css_computed_border_کيري_color(cell->style,
					       &cell->border[کيري_2].c);
		css_computed_border_کيري_width(cell->style, &width, &unit);
		cell->border[کيري_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Top border */
		cell->border[اتس_2].style =
			css_computed_border_اتس_style(cell->style);
		css_computed_border_اتس_color(cell->style,
					      &cell->border[اتس_2].c);
		css_computed_border_اتس_width(cell->style, &width, &unit);
		cell->border[اتس_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Right border */
		cell->border[کانن_2].style =
			css_computed_border_کانن_style(cell->style);
		css_computed_border_کانن_color(cell->style,
						&cell->border[کانن_2].c);
		css_computed_border_کانن_width(cell->style, &width, &unit);
		cell->border[کانن_2].width =
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
	} وإلا {
		/* Left border */
		table_used_کيري_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		table_used_اتس_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		table_used_کانن_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		table_used_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	لكل (side = 0; side != 4; side++) {
		إذا (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].width = 0;
	}
}
