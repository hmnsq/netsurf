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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <dom/dom.h>

#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/talloc.h"
#અન્તર્ભાવ "css/utils.h"

#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/table.h"

/* Define to enable verbose table debug */
#undef TABLE_DEBUG

/**
 * Container for border values during table border calculations
 */
સંરચના border {
	ગણના css_border_style_e style;	/**< border-style */
	ગણના css_border_color_e color;	/**< border-color type */
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
સ્થિર bool
table_border_is_more_eyecatching(અચલ css_unit_ctx *unit_len_ctx,
				 અચલ સંરચના border *a,
				 box_type a_src,
				 અચલ સંરચના border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	પૂર્ણાંક impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	જો (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		પાછા false;

	જો (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		પાછા true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	પ્રતિજ્ઞા(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	પ્રતિજ્ઞા(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(ખાલી, unit_len_ctx, a->width, a->unit);
	bwidth = css_unit_len2device_px(ખાલી, unit_len_ctx, b->width, b->unit);

	જો (awidth < bwidth)
		પાછા true;
	નહીંતર જો (bwidth < awidth)
		પાછા false;

	/* 3b -- sort by style */
	પસંદગી (a->style) {
	સ્થિતિ CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	મૂળભૂત:
		તોડો;
	}

	પસંદગી (b->style) {
	સ્થિતિ CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	મૂળભૂત:
		તોડો;
	}

	જો (impact < 0)
		પાછા true;
	નહીંતર જો (impact > 0)
		પાછા false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	પસંદગી (a_src) {
	સ્થિતિ BOX_TABLE_CELL:       impact++; fallthrough;
	સ્થિતિ BOX_TABLE_ROW:        impact++; fallthrough;
	સ્થિતિ BOX_TABLE_ROW_GROUP:  impact++; fallthrough;
	સ્થિતિ BOX_TABLE:            impact++; fallthrough;
	મૂળભૂત:
		તોડો;
	}

	/** \todo COL/COL_GROUP */
	પસંદગી (b_src) {
	સ્થિતિ BOX_TABLE_CELL:       impact--; fallthrough;
	સ્થિતિ BOX_TABLE_ROW:        impact--; fallthrough;
	સ્થિતિ BOX_TABLE_ROW_GROUP:  impact--; fallthrough;
	સ્થિતિ BOX_TABLE:            impact--; fallthrough;
	મૂળભૂત:
		તોડો;
	}

	જો (impact < 0)
		પાછા true;
	નહીંતર જો (impact > 0)
		પાછા false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	પાછા true;
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
સ્થિર શૂન્ય
table_cell_ટોચ_process_table(અચલ css_unit_ctx *unit_len_ctx,
			     સંરચના box *table,
			     સંરચના border *a,
			     box_type *a_src)
{
	સંરચના border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_ટોચ_style(table->style);
	b.color = css_computed_border_ટોચ_color(table->style, &b.c);
	css_computed_border_ટોચ_width(table->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(table->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE;

	જો (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
સ્થિર bool
table_cell_ટોચ_process_row(અચલ css_unit_ctx *unit_len_ctx,
			   સંરચના box *cell,
			   સંરચના box *row,
			   સંરચના border *a,
			   box_type *a_src)
{
	સંરચના border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(row->style);
	b.color = css_computed_border_bottom_color(row->style, &b.c);
	css_computed_border_bottom_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	જો (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	જો (row->children == ખાલી) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_ટોચ_style(row->style);
		b.color = css_computed_border_ટોચ_color(row->style, &b.c);
		css_computed_border_ટોચ_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		જો (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		પાછા false;
	} નહીંતર {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		સંરચના box *c;
		bool processed = false;

		જ્યારે (processed == false) {
			માટે (c = row->children; c != ખાલી; c = c->next) {
				/* Ignore cells to the left */
				જો (c->start_column + c->columns - 1 <
				    cell->start_column)
					ચાલુ;
				/* Ignore cells to the right */
				જો (c->start_column > cell->start_column +
				    cell->columns - 1)
					ચાલુ;

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

				જો (table_border_is_more_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			જો (processed == false) {
				/* There must be a preceding row */
				પ્રતિજ્ઞા(row->prev != ખાલી);

				row = row->prev;
			}
		}
	}

	પાછા true;
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
સ્થિર bool
table_cell_ટોચ_process_group(અચલ css_unit_ctx *unit_len_ctx,
			     સંરચના box *cell,
			     સંરચના box *group,
			     સંરચના border *a,
			     box_type *a_src)
{
	સંરચના border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(group->style);
	b.color = css_computed_border_bottom_color(group->style, &b.c);
	css_computed_border_bottom_width(group->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(group->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW_GROUP;

	જો (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	જો (group->last != ખાલી) {
		/* Process rows in group, starting with last */
		સંરચના box *row = group->last;

		જ્યારે (table_cell_ટોચ_process_row(unit_len_ctx, cell, row,
						  a, a_src) == false) {
			જો (row->prev == ખાલી) {
				પાછા false;
			} નહીંતર {
				row = row->prev;
			}
		}
	} નહીંતર {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_ટોચ_style(group->style);
		b.color = css_computed_border_ટોચ_color(group->style, &b.c);
		css_computed_border_ટોચ_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		જો (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		પાછા false;
	}

	પાછા true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
સ્થિર શૂન્ય
table_used_ડાબું_border_for_cell(અચલ css_unit_ctx *unit_len_ctx, સંરચના box *cell)
{
	સંરચના border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_ડાબું_style(cell->style);
	a.color = css_computed_border_ડાબું_color(cell->style, &a.c);
	css_computed_border_ડાબું_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	જો (cell->prev != ખાલી || cell->start_column != 0) {
		/* Cell to the left -- consider its right border */
		સંરચના box *prev = ખાલી;

		જો (cell->prev == ખાલી) {
			સંરચના box *row;

			/* Spanned from a previous row in current row group */
			માટે (row = cell->parent; row != ખાલી; row = row->prev) {
				માટે (prev = row->children; prev != ખાલી;
				     prev = prev->next) {
					જો (prev->start_column +
					    prev->columns ==
					    cell->start_column)
						તોડો;
				}

				જો (prev != ખાલી)
					તોડો;
			}

			પ્રતિજ્ઞા(prev != ખાલી);
		} નહીંતર {
			prev = cell->prev;
		}

		b.style = css_computed_border_જમણું_style(prev->style);
		b.color = css_computed_border_જમણું_color(prev->style, &b.c);
		css_computed_border_જમણું_width(prev->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_CELL;

		જો (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} નહીંતર {
		/* First cell in row, so consider rows and row group */
		સંરચના box *row = cell->parent;
		સંરચના box *group = row->parent;
		સંરચના box *table = group->parent;
		unsigned પૂર્ણાંક rows = cell->rows;

		જ્યારે (rows-- > 0 && row != ખાલી) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_ડાબું_style(row->style);
			b.color = css_computed_border_ડાબું_color(
								 row->style, &b.c);
			css_computed_border_ડાબું_width(
						       row->style, &b.width, &b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			જો (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_ડાબું_style(group->style);
		b.color = css_computed_border_ડાબું_color(group->style, &b.c);
		css_computed_border_ડાબું_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		જો (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_ડાબું_style(table->style);
		b.color = css_computed_border_ડાબું_color(table->style, &b.c);
		css_computed_border_ડાબું_width(table->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		જો (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[ડાબું_2].style = a.style;
	cell->border[ડાબું_2].c = a.c;
	cell->border[ડાબું_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
સ્થિર શૂન્ય
table_used_ટોચ_border_for_cell(અચલ css_unit_ctx *unit_len_ctx, સંરચના box *cell)
{
	સંરચના border a, b;
	box_type a_src, b_src;
	સંરચના box *row = cell->parent;
	bool process_group = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_ટોચ_style(cell->style);
	css_computed_border_ટોચ_color(cell->style, &a.c);
	css_computed_border_ટોચ_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	/* Top border of row */
	b.style = css_computed_border_ટોચ_style(row->style);
	css_computed_border_ટોચ_color(row->style, &b.c);
	css_computed_border_ટોચ_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	જો (table_border_is_more_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	જો (row->prev != ખાલી) {
		/* Consider row(s) above */
		જ્યારે (table_cell_ટોચ_process_row(unit_len_ctx, cell, row->prev,
						  &a, &a_src) == false) {
			જો (row->prev->prev == ખાલી) {
				/* Consider row group */
				process_group = true;
				તોડો;
			} નહીંતર {
				row = row->prev;
			}
		}
	} નહીંતર {
		process_group = true;
	}

	જો (process_group) {
		સંરચના box *group = row->parent;

		/* Top border of row group */
		b.style = css_computed_border_ટોચ_style(group->style);
		b.color = css_computed_border_ટોચ_color(group->style, &b.c);
		css_computed_border_ટોચ_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		જો (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		જો (group->prev == ખાલી) {
			/* Top border of table */
			table_cell_ટોચ_process_table(unit_len_ctx,
						     group->parent, &a, &a_src);
		} નહીંતર {
			/* Process previous group(s) */
			જ્યારે (table_cell_ટોચ_process_group(unit_len_ctx,
							    cell, group->prev,
							    &a, &a_src) == false) {
				જો (group->prev->prev == ખાલી) {
					/* Top border of table */
					table_cell_ટોચ_process_table(unit_len_ctx,
								     group->parent,
								     &a, &a_src);
					તોડો;
				} નહીંતર {
					group = group->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[ટોચ_2].style = a.style;
	cell->border[ટોચ_2].c = a.c;
	cell->border[ટોચ_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
સ્થિર શૂન્ય
table_used_જમણું_border_for_cell(અચલ css_unit_ctx *unit_len_ctx, સંરચના box *cell)
{
	સંરચના border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_જમણું_style(cell->style);
	css_computed_border_જમણું_color(cell->style, &a.c);
	css_computed_border_જમણું_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	જો (cell->next != ખાલી || cell->start_column + cell->columns !=
	    cell->parent->parent->parent->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} નહીંતર {
		/* Last cell in row, so consider rows and row group */
		સંરચના box *row = cell->parent;
		સંરચના box *group = row->parent;
		સંરચના box *table = group->parent;
		unsigned પૂર્ણાંક rows = cell->rows;

		જ્યારે (rows-- > 0 && row != ખાલી) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_જમણું_style(row->style);
			b.color = css_computed_border_જમણું_color(row->style,
								  &b.c);
			css_computed_border_જમણું_width(row->style,
							&b.width,
							&b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			જો (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_જમણું_style(group->style);
		b.color = css_computed_border_જમણું_color(group->style, &b.c);
		css_computed_border_જમણું_width(group->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		જો (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_જમણું_style(table->style);
		b.color = css_computed_border_જમણું_color(table->style, &b.c);
		css_computed_border_જમણું_width(table->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		જો (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[જમણું_2].style = a.style;
	cell->border[જમણું_2].c = a.c;
	cell->border[જમણું_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
સ્થિર શૂન્ય
table_used_bottom_border_for_cell(અચલ css_unit_ctx *unit_len_ctx,
				  સંરચના box *cell)
{
	સંરચના border a, b;
	box_type a_src, b_src;
	સંરચના box *row = cell->parent;
	unsigned પૂર્ણાંક rows = cell->rows;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	જ્યારે (rows-- > 0 && row != ખાલી)
		row = row->next;

	/** \todo Can cells span row groups? */

	જો (row != ખાલી) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} નહીંતર {
		/* Cell at bottom of table, so consider row and row group */
		સંરચના box *row = cell->parent;
		સંરચના box *group = row->parent;
		સંરચના box *table = group->parent;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(row->style);
		b.color = css_computed_border_bottom_color(row->style, &b.c);
		css_computed_border_bottom_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		જો (table_border_is_more_eyecatching(unit_len_ctx,
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

		જો (table_border_is_more_eyecatching(unit_len_ctx,
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

		જો (table_border_is_more_eyecatching(unit_len_ctx,
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
table_calculate_column_types(અચલ css_unit_ctx *unit_len_ctx, સંરચના box *table)
{
	unsigned પૂર્ણાંક i, j;
	સંરચના column *col;
	સંરચના box *row_group, *row, *cell;

	જો (table->col)
		/* table->col already constructed, for example frameset table */
		પાછા true;

	table->col = col = talloc_array(table, સંરચના column, table->columns);
	જો (!col)
		પાછા false;

	માટે (i = 0; i != table->columns; i++) {
		col[i].type = COLUMN_WIDTH_UNKNOWN;
		col[i].width = 0;
		col[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	માટે (row_group = table->children; row_group; row_group =row_group->next)
		માટે (row = row_group->children; row; row = row->next)
			માટે (cell = row->children; cell; cell = cell->next) {
				ગણના css_width_e type;
				css_fixed કિંમત = 0;
				css_unit unit = CSS_UNIT_PX;

				પ્રતિજ્ઞા(cell->type == BOX_TABLE_CELL);
				પ્રતિજ્ઞા(cell->style);

				જો (cell->columns != 1)
					ચાલુ;
				i = cell->start_column;

				જો (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					col[i].positioned = false;
				}

				type = css_computed_width(cell->style, &કિંમત, &unit);

				/* fixed width takes priority over any other width type */
				જો (col[i].type != COLUMN_WIDTH_FIXED &&
				    type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_FIXED;
					col[i].width = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							કિંમત, unit));
					જો (col[i].width < 0)
						col[i].width = 0;
					ચાલુ;
				}

				જો (col[i].type != COLUMN_WIDTH_UNKNOWN)
					ચાલુ;

				જો (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_PERCENT;
					col[i].width = FIXTOINT(કિંમત);
					જો (col[i].width < 0)
						col[i].width = 0;
				} નહીંતર જો (type == CSS_WIDTH_AUTO) {
					col[i].type = COLUMN_WIDTH_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	માટે (row_group = table->children; row_group; row_group =row_group->next)
		માટે (row = row_group->children; row; row = row->next)
			માટે (cell = row->children; cell; cell = cell->next) {
				unsigned પૂર્ણાંક fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				પૂર્ણાંક fixed_width = 0, percent_width = 0;
				ગણના css_width_e type;
				css_fixed કિંમત = 0;
				css_unit unit = CSS_UNIT_PX;

				જો (cell->columns == 1)
					ચાલુ;
				i = cell->start_column;

				માટે (j = i; j < i + cell->columns; j++) {
					col[j].positioned = false;
				}

				/* count column types in spanned cells */
				માટે (j = 0; j != cell->columns; j++) {
					જો (col[i + j].type == COLUMN_WIDTH_FIXED) {
						fixed_width += col[i + j].width;
						fixed_columns++;
					} નહીંતર જો (col[i + j].type == COLUMN_WIDTH_PERCENT) {
						percent_width += col[i + j].width;
						percent_columns++;
					} નહીંતર જો (col[i + j].type == COLUMN_WIDTH_AUTO) {
						auto_columns++;
					} નહીંતર {
						unknown_columns++;
					}
				}

				જો (!unknown_columns)
					ચાલુ;

				type = css_computed_width(cell->style, &કિંમત, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				જો (type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					પૂર્ણાંક width = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							કિંમત, unit)) -
						fixed_width) / unknown_columns;
					જો (width < 0)
						width = 0;
					માટે (j = 0; j != cell->columns; j++) {
						જો (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_FIXED;
							col[i + j].width = width;
						}
					}
				}

				/* as above for percentage width */
				જો (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					પૂર્ણાંક width = (FIXTOFLT(કિંમત) -
						     percent_width) / unknown_columns;
					જો (width < 0)
						width = 0;
					માટે (j = 0; j != cell->columns; j++) {
						જો (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_PERCENT;
							col[i + j].width = width;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	માટે (i = 0; i != table->columns; i++) {
		જો (col[i].type == COLUMN_WIDTH_UNKNOWN)
			col[i].type = COLUMN_WIDTH_AUTO;
	}

#ifdef TABLE_DEBUG
	માટે (i = 0; i != table->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      table,
		      i,
		      ((અચલ અક્ષર *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[col[i].type],
		      col[i].width);
#endif

	પાછા true;
}


/* exported interface documented in html/table.h */
શૂન્ય table_used_border_for_cell(અચલ css_unit_ctx *unit_len_ctx, સંરચના box *cell)
{
	પૂર્ણાંક side;

	પ્રતિજ્ઞા(cell->type == BOX_TABLE_CELL);

	જો (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed width = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[ડાબું_2].style =
			css_computed_border_ડાબું_style(cell->style);
		css_computed_border_ડાબું_color(cell->style,
					       &cell->border[ડાબું_2].c);
		css_computed_border_ડાબું_width(cell->style, &width, &unit);
		cell->border[ડાબું_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Top border */
		cell->border[ટોચ_2].style =
			css_computed_border_ટોચ_style(cell->style);
		css_computed_border_ટોચ_color(cell->style,
					      &cell->border[ટોચ_2].c);
		css_computed_border_ટોચ_width(cell->style, &width, &unit);
		cell->border[ટોચ_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Right border */
		cell->border[જમણું_2].style =
			css_computed_border_જમણું_style(cell->style);
		css_computed_border_જમણું_color(cell->style,
						&cell->border[જમણું_2].c);
		css_computed_border_જમણું_width(cell->style, &width, &unit);
		cell->border[જમણું_2].width =
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
	} નહીંતર {
		/* Left border */
		table_used_ડાબું_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		table_used_ટોચ_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		table_used_જમણું_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		table_used_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	માટે (side = 0; side != 4; side++) {
		જો (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].width = 0;
	}
}
