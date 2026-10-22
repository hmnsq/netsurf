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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <dom/dom.h>

#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/talloc.h"
#അന്തര്ഭാവ "css/utils.h"

#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/table.h"

/* Define to enable verbose table debug */
#undef TABLE_DEBUG

/**
 * Container for border values during table border calculations
 */
ഘടന border {
	എണ്ണൽ css_border_style_e style;	/**< border-style */
	എണ്ണൽ css_border_color_e color;	/**< border-color type */
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
സ്ഥിതം bool
table_border_is_more_eyecatching(സ്ഥിരം css_unit_ctx *unit_len_ctx,
				 സ്ഥിരം ഘടന border *a,
				 box_type a_src,
				 സ്ഥിരം ഘടന border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	പൂർണ്ണസംഖ്യ impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	എങ്കിൽ (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		തിരികെ false;

	എങ്കിൽ (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		തിരികെ true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	പ്രതിജ്ഞാ(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	പ്രതിജ്ഞാ(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(ശൂന്യ, unit_len_ctx, a->width, a->unit);
	bwidth = css_unit_len2device_px(ശൂന്യ, unit_len_ctx, b->width, b->unit);

	എങ്കിൽ (awidth < bwidth)
		തിരികെ true;
	അല്ലെങ്കിൽ എങ്കിൽ (bwidth < awidth)
		തിരികെ false;

	/* 3b -- sort by style */
	തിരഞ്ഞെടുപ്പ് (a->style) {
	അവസ്ഥ CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	തിരഞ്ഞെടുപ്പ് (b->style) {
	അവസ്ഥ CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	എങ്കിൽ (impact < 0)
		തിരികെ true;
	അല്ലെങ്കിൽ എങ്കിൽ (impact > 0)
		തിരികെ false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	തിരഞ്ഞെടുപ്പ് (a_src) {
	അവസ്ഥ BOX_TABLE_CELL:       impact++; fallthrough;
	അവസ്ഥ BOX_TABLE_ROW:        impact++; fallthrough;
	അവസ്ഥ BOX_TABLE_ROW_GROUP:  impact++; fallthrough;
	അവസ്ഥ BOX_TABLE:            impact++; fallthrough;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/** \todo COL/COL_GROUP */
	തിരഞ്ഞെടുപ്പ് (b_src) {
	അവസ്ഥ BOX_TABLE_CELL:       impact--; fallthrough;
	അവസ്ഥ BOX_TABLE_ROW:        impact--; fallthrough;
	അവസ്ഥ BOX_TABLE_ROW_GROUP:  impact--; fallthrough;
	അവസ്ഥ BOX_TABLE:            impact--; fallthrough;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	എങ്കിൽ (impact < 0)
		തിരികെ true;
	അല്ലെങ്കിൽ എങ്കിൽ (impact > 0)
		തിരികെ false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	തിരികെ true;
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
സ്ഥിതം ശൂന്യം
table_cell_മുകൾ_process_table(സ്ഥിരം css_unit_ctx *unit_len_ctx,
			     ഘടന box *table,
			     ഘടന border *a,
			     box_type *a_src)
{
	ഘടന border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_മുകൾ_style(table->style);
	b.color = css_computed_border_മുകൾ_color(table->style, &b.c);
	css_computed_border_മുകൾ_width(table->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(table->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE;

	എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
സ്ഥിതം bool
table_cell_മുകൾ_process_row(സ്ഥിരം css_unit_ctx *unit_len_ctx,
			   ഘടന box *cell,
			   ഘടന box *row,
			   ഘടന border *a,
			   box_type *a_src)
{
	ഘടന border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(row->style);
	b.color = css_computed_border_bottom_color(row->style, &b.c);
	css_computed_border_bottom_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	എങ്കിൽ (row->children == ശൂന്യ) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_മുകൾ_style(row->style);
		b.color = css_computed_border_മുകൾ_color(row->style, &b.c);
		css_computed_border_മുകൾ_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		തിരികെ false;
	} അല്ലെങ്കിൽ {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		ഘടന box *c;
		bool processed = false;

		ആകുമ്പോൾ (processed == false) {
			വേണ്ടി (c = row->children; c != ശൂന്യ; c = c->next) {
				/* Ignore cells to the left */
				എങ്കിൽ (c->start_column + c->columns - 1 <
				    cell->start_column)
					തുടരുക;
				/* Ignore cells to the right */
				എങ്കിൽ (c->start_column > cell->start_column +
				    cell->columns - 1)
					തുടരുക;

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

				എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			എങ്കിൽ (processed == false) {
				/* There must be a preceding row */
				പ്രതിജ്ഞാ(row->prev != ശൂന്യ);

				row = row->prev;
			}
		}
	}

	തിരികെ true;
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
സ്ഥിതം bool
table_cell_മുകൾ_process_group(സ്ഥിരം css_unit_ctx *unit_len_ctx,
			     ഘടന box *cell,
			     ഘടന box *group,
			     ഘടന border *a,
			     box_type *a_src)
{
	ഘടന border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(group->style);
	b.color = css_computed_border_bottom_color(group->style, &b.c);
	css_computed_border_bottom_width(group->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(group->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW_GROUP;

	എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	എങ്കിൽ (group->last != ശൂന്യ) {
		/* Process rows in group, starting with last */
		ഘടന box *row = group->last;

		ആകുമ്പോൾ (table_cell_മുകൾ_process_row(unit_len_ctx, cell, row,
						  a, a_src) == false) {
			എങ്കിൽ (row->prev == ശൂന്യ) {
				തിരികെ false;
			} അല്ലെങ്കിൽ {
				row = row->prev;
			}
		}
	} അല്ലെങ്കിൽ {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_മുകൾ_style(group->style);
		b.color = css_computed_border_മുകൾ_color(group->style, &b.c);
		css_computed_border_മുകൾ_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		തിരികെ false;
	}

	തിരികെ true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
സ്ഥിതം ശൂന്യം
table_used_ഇടത്_border_for_cell(സ്ഥിരം css_unit_ctx *unit_len_ctx, ഘടന box *cell)
{
	ഘടന border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_ഇടത്_style(cell->style);
	a.color = css_computed_border_ഇടത്_color(cell->style, &a.c);
	css_computed_border_ഇടത്_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	എങ്കിൽ (cell->prev != ശൂന്യ || cell->start_column != 0) {
		/* Cell to the left -- consider its right border */
		ഘടന box *prev = ശൂന്യ;

		എങ്കിൽ (cell->prev == ശൂന്യ) {
			ഘടന box *row;

			/* Spanned from a previous row in current row group */
			വേണ്ടി (row = cell->parent; row != ശൂന്യ; row = row->prev) {
				വേണ്ടി (prev = row->children; prev != ശൂന്യ;
				     prev = prev->next) {
					എങ്കിൽ (prev->start_column +
					    prev->columns ==
					    cell->start_column)
						നിർത്തുക;
				}

				എങ്കിൽ (prev != ശൂന്യ)
					നിർത്തുക;
			}

			പ്രതിജ്ഞാ(prev != ശൂന്യ);
		} അല്ലെങ്കിൽ {
			prev = cell->prev;
		}

		b.style = css_computed_border_വലത്_style(prev->style);
		b.color = css_computed_border_വലത്_color(prev->style, &b.c);
		css_computed_border_വലത്_width(prev->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_CELL;

		എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} അല്ലെങ്കിൽ {
		/* First cell in row, so consider rows and row group */
		ഘടന box *row = cell->parent;
		ഘടന box *group = row->parent;
		ഘടന box *table = group->parent;
		unsigned പൂർണ്ണസംഖ്യ rows = cell->rows;

		ആകുമ്പോൾ (rows-- > 0 && row != ശൂന്യ) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_ഇടത്_style(row->style);
			b.color = css_computed_border_ഇടത്_color(
								 row->style, &b.c);
			css_computed_border_ഇടത്_width(
						       row->style, &b.width, &b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_ഇടത്_style(group->style);
		b.color = css_computed_border_ഇടത്_color(group->style, &b.c);
		css_computed_border_ഇടത്_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_ഇടത്_style(table->style);
		b.color = css_computed_border_ഇടത്_color(table->style, &b.c);
		css_computed_border_ഇടത്_width(table->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[ഇടത്_2].style = a.style;
	cell->border[ഇടത്_2].c = a.c;
	cell->border[ഇടത്_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
സ്ഥിതം ശൂന്യം
table_used_മുകൾ_border_for_cell(സ്ഥിരം css_unit_ctx *unit_len_ctx, ഘടന box *cell)
{
	ഘടന border a, b;
	box_type a_src, b_src;
	ഘടന box *row = cell->parent;
	bool process_group = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_മുകൾ_style(cell->style);
	css_computed_border_മുകൾ_color(cell->style, &a.c);
	css_computed_border_മുകൾ_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	/* Top border of row */
	b.style = css_computed_border_മുകൾ_style(row->style);
	css_computed_border_മുകൾ_color(row->style, &b.c);
	css_computed_border_മുകൾ_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	എങ്കിൽ (row->prev != ശൂന്യ) {
		/* Consider row(s) above */
		ആകുമ്പോൾ (table_cell_മുകൾ_process_row(unit_len_ctx, cell, row->prev,
						  &a, &a_src) == false) {
			എങ്കിൽ (row->prev->prev == ശൂന്യ) {
				/* Consider row group */
				process_group = true;
				നിർത്തുക;
			} അല്ലെങ്കിൽ {
				row = row->prev;
			}
		}
	} അല്ലെങ്കിൽ {
		process_group = true;
	}

	എങ്കിൽ (process_group) {
		ഘടന box *group = row->parent;

		/* Top border of row group */
		b.style = css_computed_border_മുകൾ_style(group->style);
		b.color = css_computed_border_മുകൾ_color(group->style, &b.c);
		css_computed_border_മുകൾ_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		എങ്കിൽ (group->prev == ശൂന്യ) {
			/* Top border of table */
			table_cell_മുകൾ_process_table(unit_len_ctx,
						     group->parent, &a, &a_src);
		} അല്ലെങ്കിൽ {
			/* Process previous group(s) */
			ആകുമ്പോൾ (table_cell_മുകൾ_process_group(unit_len_ctx,
							    cell, group->prev,
							    &a, &a_src) == false) {
				എങ്കിൽ (group->prev->prev == ശൂന്യ) {
					/* Top border of table */
					table_cell_മുകൾ_process_table(unit_len_ctx,
								     group->parent,
								     &a, &a_src);
					നിർത്തുക;
				} അല്ലെങ്കിൽ {
					group = group->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[മുകൾ_2].style = a.style;
	cell->border[മുകൾ_2].c = a.c;
	cell->border[മുകൾ_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
സ്ഥിതം ശൂന്യം
table_used_വലത്_border_for_cell(സ്ഥിരം css_unit_ctx *unit_len_ctx, ഘടന box *cell)
{
	ഘടന border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_വലത്_style(cell->style);
	css_computed_border_വലത്_color(cell->style, &a.c);
	css_computed_border_വലത്_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	എങ്കിൽ (cell->next != ശൂന്യ || cell->start_column + cell->columns !=
	    cell->parent->parent->parent->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} അല്ലെങ്കിൽ {
		/* Last cell in row, so consider rows and row group */
		ഘടന box *row = cell->parent;
		ഘടന box *group = row->parent;
		ഘടന box *table = group->parent;
		unsigned പൂർണ്ണസംഖ്യ rows = cell->rows;

		ആകുമ്പോൾ (rows-- > 0 && row != ശൂന്യ) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_വലത്_style(row->style);
			b.color = css_computed_border_വലത്_color(row->style,
								  &b.c);
			css_computed_border_വലത്_width(row->style,
							&b.width,
							&b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_വലത്_style(group->style);
		b.color = css_computed_border_വലത്_color(group->style, &b.c);
		css_computed_border_വലത്_width(group->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_വലത്_style(table->style);
		b.color = css_computed_border_വലത്_color(table->style, &b.c);
		css_computed_border_വലത്_width(table->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[വലത്_2].style = a.style;
	cell->border[വലത്_2].c = a.c;
	cell->border[വലത്_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
സ്ഥിതം ശൂന്യം
table_used_bottom_border_for_cell(സ്ഥിരം css_unit_ctx *unit_len_ctx,
				  ഘടന box *cell)
{
	ഘടന border a, b;
	box_type a_src, b_src;
	ഘടന box *row = cell->parent;
	unsigned പൂർണ്ണസംഖ്യ rows = cell->rows;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	ആകുമ്പോൾ (rows-- > 0 && row != ശൂന്യ)
		row = row->next;

	/** \todo Can cells span row groups? */

	എങ്കിൽ (row != ശൂന്യ) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} അല്ലെങ്കിൽ {
		/* Cell at bottom of table, so consider row and row group */
		ഘടന box *row = cell->parent;
		ഘടന box *group = row->parent;
		ഘടന box *table = group->parent;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(row->style);
		b.color = css_computed_border_bottom_color(row->style, &b.c);
		css_computed_border_bottom_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
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

		എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
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

		എങ്കിൽ (table_border_is_more_eyecatching(unit_len_ctx,
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
table_calculate_column_types(സ്ഥിരം css_unit_ctx *unit_len_ctx, ഘടന box *table)
{
	unsigned പൂർണ്ണസംഖ്യ i, j;
	ഘടന column *col;
	ഘടന box *row_group, *row, *cell;

	എങ്കിൽ (table->col)
		/* table->col already constructed, for example frameset table */
		തിരികെ true;

	table->col = col = talloc_array(table, ഘടന column, table->columns);
	എങ്കിൽ (!col)
		തിരികെ false;

	വേണ്ടി (i = 0; i != table->columns; i++) {
		col[i].type = COLUMN_WIDTH_UNKNOWN;
		col[i].width = 0;
		col[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	വേണ്ടി (row_group = table->children; row_group; row_group =row_group->next)
		വേണ്ടി (row = row_group->children; row; row = row->next)
			വേണ്ടി (cell = row->children; cell; cell = cell->next) {
				എണ്ണൽ css_width_e type;
				css_fixed മൂല്യം = 0;
				css_unit unit = CSS_UNIT_PX;

				പ്രതിജ്ഞാ(cell->type == BOX_TABLE_CELL);
				പ്രതിജ്ഞാ(cell->style);

				എങ്കിൽ (cell->columns != 1)
					തുടരുക;
				i = cell->start_column;

				എങ്കിൽ (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					col[i].positioned = false;
				}

				type = css_computed_width(cell->style, &മൂല്യം, &unit);

				/* fixed width takes priority over any other width type */
				എങ്കിൽ (col[i].type != COLUMN_WIDTH_FIXED &&
				    type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_FIXED;
					col[i].width = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							മൂല്യം, unit));
					എങ്കിൽ (col[i].width < 0)
						col[i].width = 0;
					തുടരുക;
				}

				എങ്കിൽ (col[i].type != COLUMN_WIDTH_UNKNOWN)
					തുടരുക;

				എങ്കിൽ (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_PERCENT;
					col[i].width = FIXTOINT(മൂല്യം);
					എങ്കിൽ (col[i].width < 0)
						col[i].width = 0;
				} അല്ലെങ്കിൽ എങ്കിൽ (type == CSS_WIDTH_AUTO) {
					col[i].type = COLUMN_WIDTH_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	വേണ്ടി (row_group = table->children; row_group; row_group =row_group->next)
		വേണ്ടി (row = row_group->children; row; row = row->next)
			വേണ്ടി (cell = row->children; cell; cell = cell->next) {
				unsigned പൂർണ്ണസംഖ്യ fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				പൂർണ്ണസംഖ്യ fixed_width = 0, percent_width = 0;
				എണ്ണൽ css_width_e type;
				css_fixed മൂല്യം = 0;
				css_unit unit = CSS_UNIT_PX;

				എങ്കിൽ (cell->columns == 1)
					തുടരുക;
				i = cell->start_column;

				വേണ്ടി (j = i; j < i + cell->columns; j++) {
					col[j].positioned = false;
				}

				/* count column types in spanned cells */
				വേണ്ടി (j = 0; j != cell->columns; j++) {
					എങ്കിൽ (col[i + j].type == COLUMN_WIDTH_FIXED) {
						fixed_width += col[i + j].width;
						fixed_columns++;
					} അല്ലെങ്കിൽ എങ്കിൽ (col[i + j].type == COLUMN_WIDTH_PERCENT) {
						percent_width += col[i + j].width;
						percent_columns++;
					} അല്ലെങ്കിൽ എങ്കിൽ (col[i + j].type == COLUMN_WIDTH_AUTO) {
						auto_columns++;
					} അല്ലെങ്കിൽ {
						unknown_columns++;
					}
				}

				എങ്കിൽ (!unknown_columns)
					തുടരുക;

				type = css_computed_width(cell->style, &മൂല്യം, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				എങ്കിൽ (type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					പൂർണ്ണസംഖ്യ width = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							മൂല്യം, unit)) -
						fixed_width) / unknown_columns;
					എങ്കിൽ (width < 0)
						width = 0;
					വേണ്ടി (j = 0; j != cell->columns; j++) {
						എങ്കിൽ (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_FIXED;
							col[i + j].width = width;
						}
					}
				}

				/* as above for percentage width */
				എങ്കിൽ (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					പൂർണ്ണസംഖ്യ width = (FIXTOFLT(മൂല്യം) -
						     percent_width) / unknown_columns;
					എങ്കിൽ (width < 0)
						width = 0;
					വേണ്ടി (j = 0; j != cell->columns; j++) {
						എങ്കിൽ (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_PERCENT;
							col[i + j].width = width;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	വേണ്ടി (i = 0; i != table->columns; i++) {
		എങ്കിൽ (col[i].type == COLUMN_WIDTH_UNKNOWN)
			col[i].type = COLUMN_WIDTH_AUTO;
	}

#ifdef TABLE_DEBUG
	വേണ്ടി (i = 0; i != table->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      table,
		      i,
		      ((സ്ഥിരം അക്ഷരം *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[col[i].type],
		      col[i].width);
#endif

	തിരികെ true;
}


/* exported interface documented in html/table.h */
ശൂന്യം table_used_border_for_cell(സ്ഥിരം css_unit_ctx *unit_len_ctx, ഘടന box *cell)
{
	പൂർണ്ണസംഖ്യ side;

	പ്രതിജ്ഞാ(cell->type == BOX_TABLE_CELL);

	എങ്കിൽ (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed width = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[ഇടത്_2].style =
			css_computed_border_ഇടത്_style(cell->style);
		css_computed_border_ഇടത്_color(cell->style,
					       &cell->border[ഇടത്_2].c);
		css_computed_border_ഇടത്_width(cell->style, &width, &unit);
		cell->border[ഇടത്_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Top border */
		cell->border[മുകൾ_2].style =
			css_computed_border_മുകൾ_style(cell->style);
		css_computed_border_മുകൾ_color(cell->style,
					      &cell->border[മുകൾ_2].c);
		css_computed_border_മുകൾ_width(cell->style, &width, &unit);
		cell->border[മുകൾ_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Right border */
		cell->border[വലത്_2].style =
			css_computed_border_വലത്_style(cell->style);
		css_computed_border_വലത്_color(cell->style,
						&cell->border[വലത്_2].c);
		css_computed_border_വലത്_width(cell->style, &width, &unit);
		cell->border[വലത്_2].width =
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
	} അല്ലെങ്കിൽ {
		/* Left border */
		table_used_ഇടത്_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		table_used_മുകൾ_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		table_used_വലത്_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		table_used_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	വേണ്ടി (side = 0; side != 4; side++) {
		എങ്കിൽ (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].width = 0;
	}
}
