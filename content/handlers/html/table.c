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

#include <assert.h>
#include <dom/dom.h>

#include "utils/log.h"
#include "utils/utils.h"
#include "utils/talloc.h"
#include "css/utils.h"

#include "html/box.h"
#include "html/table.h"

/* Define to enable verbose table debug */
#undef TABLE_DEBUG

/**
 * Container for border values during table border calculations
 */
sa_nracanaa border {
	ga_nanaa css_border_style_e style;	/**< border-style */
	ga_nanaa css_border_color_e color;	/**< border-color type */
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
static bool
table_border_is_more_eyecatching(const css_unit_ctx *unit_len_ctx,
				 const sa_nracanaa border *a,
				 box_type a_src,
				 const sa_nracanaa border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	ཧྲིལ impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	གལ་ཏེ (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		ཕྱིར false;

	གལ་ཏེ (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		ཕྱིར true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	assert(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	assert(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(NULL, unit_len_ctx, a->width, a->unit);
	bwidth = css_unit_len2device_px(NULL, unit_len_ctx, b->width, b->unit);

	གལ་ཏེ (awidth < bwidth)
		ཕྱིར true;
	དེ་མིན གལ་ཏེ (bwidth < awidth)
		ཕྱིར false;

	/* 3b -- sort by style */
	switch (a->style) {
	pak_sa CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	pak_sa CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	pak_sa CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	pak_sa CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	pak_sa CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	pak_sa CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	pak_sa CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	pak_sa CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	default:
		bha_nga;
	}

	switch (b->style) {
	pak_sa CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	pak_sa CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	pak_sa CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	pak_sa CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	pak_sa CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	pak_sa CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	pak_sa CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	pak_sa CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	default:
		bha_nga;
	}

	གལ་ཏེ (impact < 0)
		ཕྱིར true;
	དེ་མིན གལ་ཏེ (impact > 0)
		ཕྱིར false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	switch (a_src) {
	pak_sa BOX_TABLE_CELL:       impact++; fallthrough;
	pak_sa BOX_TABLE_ROW:        impact++; fallthrough;
	pak_sa BOX_TABLE_ROW_GROUP:  impact++; fallthrough;
	pak_sa BOX_TABLE:            impact++; fallthrough;
	default:
		bha_nga;
	}

	/** \todo COL/COL_GROUP */
	switch (b_src) {
	pak_sa BOX_TABLE_CELL:       impact--; fallthrough;
	pak_sa BOX_TABLE_ROW:        impact--; fallthrough;
	pak_sa BOX_TABLE_ROW_GROUP:  impact--; fallthrough;
	pak_sa BOX_TABLE:            impact--; fallthrough;
	default:
		bha_nga;
	}

	གལ་ཏེ (impact < 0)
		ཕྱིར true;
	དེ་མིན གལ་ཏེ (impact > 0)
		ཕྱིར false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	ཕྱིར true;
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
static void
table_cell_top_process_table(const css_unit_ctx *unit_len_ctx,
			     sa_nracanaa box *table,
			     sa_nracanaa border *a,
			     box_type *a_src)
{
	sa_nracanaa border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_top_style(table->style);
	b.color = css_computed_border_top_color(table->style, &b.c);
	css_computed_border_top_width(table->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(table->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE;

	གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
static bool
table_cell_top_process_row(const css_unit_ctx *unit_len_ctx,
			   sa_nracanaa box *cell,
			   sa_nracanaa box *row,
			   sa_nracanaa border *a,
			   box_type *a_src)
{
	sa_nracanaa border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(row->style);
	b.color = css_computed_border_bottom_color(row->style, &b.c);
	css_computed_border_bottom_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	གལ་ཏེ (row->children == NULL) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_top_style(row->style);
		b.color = css_computed_border_top_color(row->style, &b.c);
		css_computed_border_top_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		ཕྱིར false;
	} དེ་མིན {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		sa_nracanaa box *c;
		bool processed = false;

		སྐབས (processed == false) {
			ཆེད་དུ (c = row->children; c != NULL; c = c->next) {
				/* Ignore cells to the left */
				གལ་ཏེ (c->start_column + c->columns - 1 <
				    cell->start_column)
					continue;
				/* Ignore cells to the right */
				གལ་ཏེ (c->start_column > cell->start_column +
				    cell->columns - 1)
					continue;

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

				གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			གལ་ཏེ (processed == false) {
				/* There must be a preceding row */
				assert(row->prev != NULL);

				row = row->prev;
			}
		}
	}

	ཕྱིར true;
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
static bool
table_cell_top_process_group(const css_unit_ctx *unit_len_ctx,
			     sa_nracanaa box *cell,
			     sa_nracanaa box *group,
			     sa_nracanaa border *a,
			     box_type *a_src)
{
	sa_nracanaa border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(group->style);
	b.color = css_computed_border_bottom_color(group->style, &b.c);
	css_computed_border_bottom_width(group->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(group->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW_GROUP;

	གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	གལ་ཏེ (group->last != NULL) {
		/* Process rows in group, starting with last */
		sa_nracanaa box *row = group->last;

		སྐབས (table_cell_top_process_row(unit_len_ctx, cell, row,
						  a, a_src) == false) {
			གལ་ཏེ (row->prev == NULL) {
				ཕྱིར false;
			} དེ་མིན {
				row = row->prev;
			}
		}
	} དེ་མིན {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_top_style(group->style);
		b.color = css_computed_border_top_color(group->style, &b.c);
		css_computed_border_top_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		ཕྱིར false;
	}

	ཕྱིར true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
static void
table_used_left_border_for_cell(const css_unit_ctx *unit_len_ctx, sa_nracanaa box *cell)
{
	sa_nracanaa border a, b;
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

	གལ་ཏེ (cell->prev != NULL || cell->start_column != 0) {
		/* Cell to the left -- consider its right border */
		sa_nracanaa box *prev = NULL;

		གལ་ཏེ (cell->prev == NULL) {
			sa_nracanaa box *row;

			/* Spanned from a previous row in current row group */
			ཆེད་དུ (row = cell->parent; row != NULL; row = row->prev) {
				ཆེད་དུ (prev = row->children; prev != NULL;
				     prev = prev->next) {
					གལ་ཏེ (prev->start_column +
					    prev->columns ==
					    cell->start_column)
						bha_nga;
				}

				གལ་ཏེ (prev != NULL)
					bha_nga;
			}

			assert(prev != NULL);
		} དེ་མིན {
			prev = cell->prev;
		}

		b.style = css_computed_border_right_style(prev->style);
		b.color = css_computed_border_right_color(prev->style, &b.c);
		css_computed_border_right_width(prev->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_CELL;

		གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} དེ་མིན {
		/* First cell in row, so consider rows and row group */
		sa_nracanaa box *row = cell->parent;
		sa_nracanaa box *group = row->parent;
		sa_nracanaa box *table = group->parent;
		unsigned ཧྲིལ rows = cell->rows;

		སྐབས (rows-- > 0 && row != NULL) {
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

			གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
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

		གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
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

		གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
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
static void
table_used_top_border_for_cell(const css_unit_ctx *unit_len_ctx, sa_nracanaa box *cell)
{
	sa_nracanaa border a, b;
	box_type a_src, b_src;
	sa_nracanaa box *row = cell->parent;
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

	གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	གལ་ཏེ (row->prev != NULL) {
		/* Consider row(s) above */
		སྐབས (table_cell_top_process_row(unit_len_ctx, cell, row->prev,
						  &a, &a_src) == false) {
			གལ་ཏེ (row->prev->prev == NULL) {
				/* Consider row group */
				process_group = true;
				bha_nga;
			} དེ་མིན {
				row = row->prev;
			}
		}
	} དེ་མིན {
		process_group = true;
	}

	གལ་ཏེ (process_group) {
		sa_nracanaa box *group = row->parent;

		/* Top border of row group */
		b.style = css_computed_border_top_style(group->style);
		b.color = css_computed_border_top_color(group->style, &b.c);
		css_computed_border_top_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		གལ་ཏེ (group->prev == NULL) {
			/* Top border of table */
			table_cell_top_process_table(unit_len_ctx,
						     group->parent, &a, &a_src);
		} དེ་མིན {
			/* Process previous group(s) */
			སྐབས (table_cell_top_process_group(unit_len_ctx,
							    cell, group->prev,
							    &a, &a_src) == false) {
				གལ་ཏེ (group->prev->prev == NULL) {
					/* Top border of table */
					table_cell_top_process_table(unit_len_ctx,
								     group->parent,
								     &a, &a_src);
					bha_nga;
				} དེ་མིན {
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
static void
table_used_right_border_for_cell(const css_unit_ctx *unit_len_ctx, sa_nracanaa box *cell)
{
	sa_nracanaa border a, b;
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

	གལ་ཏེ (cell->next != NULL || cell->start_column + cell->columns !=
	    cell->parent->parent->parent->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} དེ་མིན {
		/* Last cell in row, so consider rows and row group */
		sa_nracanaa box *row = cell->parent;
		sa_nracanaa box *group = row->parent;
		sa_nracanaa box *table = group->parent;
		unsigned ཧྲིལ rows = cell->rows;

		སྐབས (rows-- > 0 && row != NULL) {
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

			གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
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

		གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
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

		གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
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
static void
table_used_bottom_border_for_cell(const css_unit_ctx *unit_len_ctx,
				  sa_nracanaa box *cell)
{
	sa_nracanaa border a, b;
	box_type a_src, b_src;
	sa_nracanaa box *row = cell->parent;
	unsigned ཧྲིལ rows = cell->rows;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	སྐབས (rows-- > 0 && row != NULL)
		row = row->next;

	/** \todo Can cells span row groups? */

	གལ་ཏེ (row != NULL) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} དེ་མིན {
		/* Cell at bottom of table, so consider row and row group */
		sa_nracanaa box *row = cell->parent;
		sa_nracanaa box *group = row->parent;
		sa_nracanaa box *table = group->parent;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(row->style);
		b.color = css_computed_border_bottom_color(row->style, &b.c);
		css_computed_border_bottom_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
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

		གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
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

		གལ་ཏེ (table_border_is_more_eyecatching(unit_len_ctx,
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
table_calculate_column_types(const css_unit_ctx *unit_len_ctx, sa_nracanaa box *table)
{
	unsigned ཧྲིལ i, j;
	sa_nracanaa column *col;
	sa_nracanaa box *row_group, *row, *cell;

	གལ་ཏེ (table->col)
		/* table->col already constructed, for example frameset table */
		ཕྱིར true;

	table->col = col = talloc_array(table, sa_nracanaa column, table->columns);
	གལ་ཏེ (!col)
		ཕྱིར false;

	ཆེད་དུ (i = 0; i != table->columns; i++) {
		col[i].type = COLUMN_WIDTH_UNKNOWN;
		col[i].width = 0;
		col[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	ཆེད་དུ (row_group = table->children; row_group; row_group =row_group->next)
		ཆེད་དུ (row = row_group->children; row; row = row->next)
			ཆེད་དུ (cell = row->children; cell; cell = cell->next) {
				ga_nanaa css_width_e type;
				css_fixed value = 0;
				css_unit unit = CSS_UNIT_PX;

				assert(cell->type == BOX_TABLE_CELL);
				assert(cell->style);

				གལ་ཏེ (cell->columns != 1)
					continue;
				i = cell->start_column;

				གལ་ཏེ (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					col[i].positioned = false;
				}

				type = css_computed_width(cell->style, &value, &unit);

				/* fixed width takes priority over any other width type */
				གལ་ཏེ (col[i].type != COLUMN_WIDTH_FIXED &&
				    type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_FIXED;
					col[i].width = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							value, unit));
					གལ་ཏེ (col[i].width < 0)
						col[i].width = 0;
					continue;
				}

				གལ་ཏེ (col[i].type != COLUMN_WIDTH_UNKNOWN)
					continue;

				གལ་ཏེ (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_PERCENT;
					col[i].width = FIXTOINT(value);
					གལ་ཏེ (col[i].width < 0)
						col[i].width = 0;
				} དེ་མིན གལ་ཏེ (type == CSS_WIDTH_AUTO) {
					col[i].type = COLUMN_WIDTH_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	ཆེད་དུ (row_group = table->children; row_group; row_group =row_group->next)
		ཆེད་དུ (row = row_group->children; row; row = row->next)
			ཆེད་དུ (cell = row->children; cell; cell = cell->next) {
				unsigned ཧྲིལ fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				ཧྲིལ fixed_width = 0, percent_width = 0;
				ga_nanaa css_width_e type;
				css_fixed value = 0;
				css_unit unit = CSS_UNIT_PX;

				གལ་ཏེ (cell->columns == 1)
					continue;
				i = cell->start_column;

				ཆེད་དུ (j = i; j < i + cell->columns; j++) {
					col[j].positioned = false;
				}

				/* count column types in spanned cells */
				ཆེད་དུ (j = 0; j != cell->columns; j++) {
					གལ་ཏེ (col[i + j].type == COLUMN_WIDTH_FIXED) {
						fixed_width += col[i + j].width;
						fixed_columns++;
					} དེ་མིན གལ་ཏེ (col[i + j].type == COLUMN_WIDTH_PERCENT) {
						percent_width += col[i + j].width;
						percent_columns++;
					} དེ་མིན གལ་ཏེ (col[i + j].type == COLUMN_WIDTH_AUTO) {
						auto_columns++;
					} དེ་མིན {
						unknown_columns++;
					}
				}

				གལ་ཏེ (!unknown_columns)
					continue;

				type = css_computed_width(cell->style, &value, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				གལ་ཏེ (type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					ཧྲིལ width = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							value, unit)) -
						fixed_width) / unknown_columns;
					གལ་ཏེ (width < 0)
						width = 0;
					ཆེད་དུ (j = 0; j != cell->columns; j++) {
						གལ་ཏེ (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_FIXED;
							col[i + j].width = width;
						}
					}
				}

				/* as above for percentage width */
				གལ་ཏེ (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					ཧྲིལ width = (FIXTOFLT(value) -
						     percent_width) / unknown_columns;
					གལ་ཏེ (width < 0)
						width = 0;
					ཆེད་དུ (j = 0; j != cell->columns; j++) {
						གལ་ཏེ (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_PERCENT;
							col[i + j].width = width;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	ཆེད་དུ (i = 0; i != table->columns; i++) {
		གལ་ཏེ (col[i].type == COLUMN_WIDTH_UNKNOWN)
			col[i].type = COLUMN_WIDTH_AUTO;
	}

#ifdef TABLE_DEBUG
	ཆེད་དུ (i = 0; i != table->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      table,
		      i,
		      ((const var_na *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[col[i].type],
		      col[i].width);
#endif

	ཕྱིར true;
}


/* exported interface documented in html/table.h */
void table_used_border_for_cell(const css_unit_ctx *unit_len_ctx, sa_nracanaa box *cell)
{
	ཧྲིལ side;

	assert(cell->type == BOX_TABLE_CELL);

	གལ་ཏེ (css_computed_border_collapse(cell->style) ==
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
	} དེ་མིན {
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
	ཆེད་དུ (side = 0; side != 4; side++) {
		གལ་ཏེ (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].width = 0;
	}
}
