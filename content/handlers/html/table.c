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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <dom/dom.h>

#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/talloc.h"
#அந்தர்பாவ "css/utils.h"

#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/table.h"

/* Define to enable verbose table debug */
#undef TABLE_DEBUG

/**
 * Container for border values during table border calculations
 */
கட்டமைப்பு border {
	எண்ணிக்கை css_border_style_e style;	/**< border-style */
	எண்ணிக்கை css_border_color_e color;	/**< border-color type */
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
நிலையான bool
table_border_is_more_eyecatching(மாறிலி css_unit_ctx *unit_len_ctx,
				 மாறிலி கட்டமைப்பு border *a,
				 box_type a_src,
				 மாறிலி கட்டமைப்பு border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	முழு impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	எனில் (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		திரும்பு false;

	எனில் (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		திரும்பு true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	ப்ரதிஜ்ஞா(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	ப்ரதிஜ்ஞா(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(வெற்று, unit_len_ctx, a->width, a->unit);
	bwidth = css_unit_len2device_px(வெற்று, unit_len_ctx, b->width, b->unit);

	எனில் (awidth < bwidth)
		திரும்பு true;
	இல்லையெனில் எனில் (bwidth < awidth)
		திரும்பு false;

	/* 3b -- sort by style */
	தேர்வு (a->style) {
	நிலை CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	நிலை CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	நிலை CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	நிலை CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	நிலை CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	நிலை CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	நிலை CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	நிலை CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	இயல்பு:
		நிறுத்து;
	}

	தேர்வு (b->style) {
	நிலை CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	நிலை CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	நிலை CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	நிலை CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	நிலை CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	நிலை CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	நிலை CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	நிலை CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	இயல்பு:
		நிறுத்து;
	}

	எனில் (impact < 0)
		திரும்பு true;
	இல்லையெனில் எனில் (impact > 0)
		திரும்பு false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	தேர்வு (a_src) {
	நிலை BOX_TABLE_CELL:       impact++; fallthrough;
	நிலை BOX_TABLE_ROW:        impact++; fallthrough;
	நிலை BOX_TABLE_ROW_GROUP:  impact++; fallthrough;
	நிலை BOX_TABLE:            impact++; fallthrough;
	இயல்பு:
		நிறுத்து;
	}

	/** \todo COL/COL_GROUP */
	தேர்வு (b_src) {
	நிலை BOX_TABLE_CELL:       impact--; fallthrough;
	நிலை BOX_TABLE_ROW:        impact--; fallthrough;
	நிலை BOX_TABLE_ROW_GROUP:  impact--; fallthrough;
	நிலை BOX_TABLE:            impact--; fallthrough;
	இயல்பு:
		நிறுத்து;
	}

	எனில் (impact < 0)
		திரும்பு true;
	இல்லையெனில் எனில் (impact > 0)
		திரும்பு false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	திரும்பு true;
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
நிலையான வெறுமை
table_cell_உச்சி_process_table(மாறிலி css_unit_ctx *unit_len_ctx,
			     கட்டமைப்பு box *table,
			     கட்டமைப்பு border *a,
			     box_type *a_src)
{
	கட்டமைப்பு border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_உச்சி_style(table->style);
	b.color = css_computed_border_உச்சி_color(table->style, &b.c);
	css_computed_border_உச்சி_width(table->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(table->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE;

	எனில் (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
நிலையான bool
table_cell_உச்சி_process_row(மாறிலி css_unit_ctx *unit_len_ctx,
			   கட்டமைப்பு box *cell,
			   கட்டமைப்பு box *row,
			   கட்டமைப்பு border *a,
			   box_type *a_src)
{
	கட்டமைப்பு border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(row->style);
	b.color = css_computed_border_bottom_color(row->style, &b.c);
	css_computed_border_bottom_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	எனில் (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	எனில் (row->children == வெற்று) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_உச்சி_style(row->style);
		b.color = css_computed_border_உச்சி_color(row->style, &b.c);
		css_computed_border_உச்சி_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		எனில் (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		திரும்பு false;
	} இல்லையெனில் {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		கட்டமைப்பு box *c;
		bool processed = false;

		போது (processed == false) {
			க்கு (c = row->children; c != வெற்று; c = c->next) {
				/* Ignore cells to the left */
				எனில் (c->start_column + c->columns - 1 <
				    cell->start_column)
					தொடர்;
				/* Ignore cells to the right */
				எனில் (c->start_column > cell->start_column +
				    cell->columns - 1)
					தொடர்;

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

				எனில் (table_border_is_more_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			எனில் (processed == false) {
				/* There must be a preceding row */
				ப்ரதிஜ்ஞா(row->prev != வெற்று);

				row = row->prev;
			}
		}
	}

	திரும்பு true;
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
நிலையான bool
table_cell_உச்சி_process_group(மாறிலி css_unit_ctx *unit_len_ctx,
			     கட்டமைப்பு box *cell,
			     கட்டமைப்பு box *group,
			     கட்டமைப்பு border *a,
			     box_type *a_src)
{
	கட்டமைப்பு border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(group->style);
	b.color = css_computed_border_bottom_color(group->style, &b.c);
	css_computed_border_bottom_width(group->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(group->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW_GROUP;

	எனில் (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	எனில் (group->last != வெற்று) {
		/* Process rows in group, starting with last */
		கட்டமைப்பு box *row = group->last;

		போது (table_cell_உச்சி_process_row(unit_len_ctx, cell, row,
						  a, a_src) == false) {
			எனில் (row->prev == வெற்று) {
				திரும்பு false;
			} இல்லையெனில் {
				row = row->prev;
			}
		}
	} இல்லையெனில் {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_உச்சி_style(group->style);
		b.color = css_computed_border_உச்சி_color(group->style, &b.c);
		css_computed_border_உச்சி_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		எனில் (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		திரும்பு false;
	}

	திரும்பு true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
நிலையான வெறுமை
table_used_இடது_border_for_cell(மாறிலி css_unit_ctx *unit_len_ctx, கட்டமைப்பு box *cell)
{
	கட்டமைப்பு border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_இடது_style(cell->style);
	a.color = css_computed_border_இடது_color(cell->style, &a.c);
	css_computed_border_இடது_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	எனில் (cell->prev != வெற்று || cell->start_column != 0) {
		/* Cell to the left -- consider its right border */
		கட்டமைப்பு box *prev = வெற்று;

		எனில் (cell->prev == வெற்று) {
			கட்டமைப்பு box *row;

			/* Spanned from a previous row in current row group */
			க்கு (row = cell->parent; row != வெற்று; row = row->prev) {
				க்கு (prev = row->children; prev != வெற்று;
				     prev = prev->next) {
					எனில் (prev->start_column +
					    prev->columns ==
					    cell->start_column)
						நிறுத்து;
				}

				எனில் (prev != வெற்று)
					நிறுத்து;
			}

			ப்ரதிஜ்ஞா(prev != வெற்று);
		} இல்லையெனில் {
			prev = cell->prev;
		}

		b.style = css_computed_border_வலது_style(prev->style);
		b.color = css_computed_border_வலது_color(prev->style, &b.c);
		css_computed_border_வலது_width(prev->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_CELL;

		எனில் (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} இல்லையெனில் {
		/* First cell in row, so consider rows and row group */
		கட்டமைப்பு box *row = cell->parent;
		கட்டமைப்பு box *group = row->parent;
		கட்டமைப்பு box *table = group->parent;
		unsigned முழு rows = cell->rows;

		போது (rows-- > 0 && row != வெற்று) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_இடது_style(row->style);
			b.color = css_computed_border_இடது_color(
								 row->style, &b.c);
			css_computed_border_இடது_width(
						       row->style, &b.width, &b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			எனில் (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_இடது_style(group->style);
		b.color = css_computed_border_இடது_color(group->style, &b.c);
		css_computed_border_இடது_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		எனில் (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_இடது_style(table->style);
		b.color = css_computed_border_இடது_color(table->style, &b.c);
		css_computed_border_இடது_width(table->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		எனில் (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[இடது_2].style = a.style;
	cell->border[இடது_2].c = a.c;
	cell->border[இடது_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
நிலையான வெறுமை
table_used_உச்சி_border_for_cell(மாறிலி css_unit_ctx *unit_len_ctx, கட்டமைப்பு box *cell)
{
	கட்டமைப்பு border a, b;
	box_type a_src, b_src;
	கட்டமைப்பு box *row = cell->parent;
	bool process_group = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_உச்சி_style(cell->style);
	css_computed_border_உச்சி_color(cell->style, &a.c);
	css_computed_border_உச்சி_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	/* Top border of row */
	b.style = css_computed_border_உச்சி_style(row->style);
	css_computed_border_உச்சி_color(row->style, &b.c);
	css_computed_border_உச்சி_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	எனில் (table_border_is_more_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	எனில் (row->prev != வெற்று) {
		/* Consider row(s) above */
		போது (table_cell_உச்சி_process_row(unit_len_ctx, cell, row->prev,
						  &a, &a_src) == false) {
			எனில் (row->prev->prev == வெற்று) {
				/* Consider row group */
				process_group = true;
				நிறுத்து;
			} இல்லையெனில் {
				row = row->prev;
			}
		}
	} இல்லையெனில் {
		process_group = true;
	}

	எனில் (process_group) {
		கட்டமைப்பு box *group = row->parent;

		/* Top border of row group */
		b.style = css_computed_border_உச்சி_style(group->style);
		b.color = css_computed_border_உச்சி_color(group->style, &b.c);
		css_computed_border_உச்சி_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		எனில் (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		எனில் (group->prev == வெற்று) {
			/* Top border of table */
			table_cell_உச்சி_process_table(unit_len_ctx,
						     group->parent, &a, &a_src);
		} இல்லையெனில் {
			/* Process previous group(s) */
			போது (table_cell_உச்சி_process_group(unit_len_ctx,
							    cell, group->prev,
							    &a, &a_src) == false) {
				எனில் (group->prev->prev == வெற்று) {
					/* Top border of table */
					table_cell_உச்சி_process_table(unit_len_ctx,
								     group->parent,
								     &a, &a_src);
					நிறுத்து;
				} இல்லையெனில் {
					group = group->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[உச்சி_2].style = a.style;
	cell->border[உச்சி_2].c = a.c;
	cell->border[உச்சி_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
நிலையான வெறுமை
table_used_வலது_border_for_cell(மாறிலி css_unit_ctx *unit_len_ctx, கட்டமைப்பு box *cell)
{
	கட்டமைப்பு border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_வலது_style(cell->style);
	css_computed_border_வலது_color(cell->style, &a.c);
	css_computed_border_வலது_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	எனில் (cell->next != வெற்று || cell->start_column + cell->columns !=
	    cell->parent->parent->parent->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} இல்லையெனில் {
		/* Last cell in row, so consider rows and row group */
		கட்டமைப்பு box *row = cell->parent;
		கட்டமைப்பு box *group = row->parent;
		கட்டமைப்பு box *table = group->parent;
		unsigned முழு rows = cell->rows;

		போது (rows-- > 0 && row != வெற்று) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_வலது_style(row->style);
			b.color = css_computed_border_வலது_color(row->style,
								  &b.c);
			css_computed_border_வலது_width(row->style,
							&b.width,
							&b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			எனில் (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_வலது_style(group->style);
		b.color = css_computed_border_வலது_color(group->style, &b.c);
		css_computed_border_வலது_width(group->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		எனில் (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_வலது_style(table->style);
		b.color = css_computed_border_வலது_color(table->style, &b.c);
		css_computed_border_வலது_width(table->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		எனில் (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[வலது_2].style = a.style;
	cell->border[வலது_2].c = a.c;
	cell->border[வலது_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
நிலையான வெறுமை
table_used_bottom_border_for_cell(மாறிலி css_unit_ctx *unit_len_ctx,
				  கட்டமைப்பு box *cell)
{
	கட்டமைப்பு border a, b;
	box_type a_src, b_src;
	கட்டமைப்பு box *row = cell->parent;
	unsigned முழு rows = cell->rows;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	போது (rows-- > 0 && row != வெற்று)
		row = row->next;

	/** \todo Can cells span row groups? */

	எனில் (row != வெற்று) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} இல்லையெனில் {
		/* Cell at bottom of table, so consider row and row group */
		கட்டமைப்பு box *row = cell->parent;
		கட்டமைப்பு box *group = row->parent;
		கட்டமைப்பு box *table = group->parent;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(row->style);
		b.color = css_computed_border_bottom_color(row->style, &b.c);
		css_computed_border_bottom_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		எனில் (table_border_is_more_eyecatching(unit_len_ctx,
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

		எனில் (table_border_is_more_eyecatching(unit_len_ctx,
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

		எனில் (table_border_is_more_eyecatching(unit_len_ctx,
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
table_calculate_column_types(மாறிலி css_unit_ctx *unit_len_ctx, கட்டமைப்பு box *table)
{
	unsigned முழு i, j;
	கட்டமைப்பு column *col;
	கட்டமைப்பு box *row_group, *row, *cell;

	எனில் (table->col)
		/* table->col already constructed, for example frameset table */
		திரும்பு true;

	table->col = col = talloc_array(table, கட்டமைப்பு column, table->columns);
	எனில் (!col)
		திரும்பு false;

	க்கு (i = 0; i != table->columns; i++) {
		col[i].type = COLUMN_WIDTH_UNKNOWN;
		col[i].width = 0;
		col[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	க்கு (row_group = table->children; row_group; row_group =row_group->next)
		க்கு (row = row_group->children; row; row = row->next)
			க்கு (cell = row->children; cell; cell = cell->next) {
				எண்ணிக்கை css_width_e type;
				css_fixed மதிப்பு = 0;
				css_unit unit = CSS_UNIT_PX;

				ப்ரதிஜ்ஞா(cell->type == BOX_TABLE_CELL);
				ப்ரதிஜ்ஞா(cell->style);

				எனில் (cell->columns != 1)
					தொடர்;
				i = cell->start_column;

				எனில் (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					col[i].positioned = false;
				}

				type = css_computed_width(cell->style, &மதிப்பு, &unit);

				/* fixed width takes priority over any other width type */
				எனில் (col[i].type != COLUMN_WIDTH_FIXED &&
				    type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_FIXED;
					col[i].width = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							மதிப்பு, unit));
					எனில் (col[i].width < 0)
						col[i].width = 0;
					தொடர்;
				}

				எனில் (col[i].type != COLUMN_WIDTH_UNKNOWN)
					தொடர்;

				எனில் (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_PERCENT;
					col[i].width = FIXTOINT(மதிப்பு);
					எனில் (col[i].width < 0)
						col[i].width = 0;
				} இல்லையெனில் எனில் (type == CSS_WIDTH_AUTO) {
					col[i].type = COLUMN_WIDTH_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	க்கு (row_group = table->children; row_group; row_group =row_group->next)
		க்கு (row = row_group->children; row; row = row->next)
			க்கு (cell = row->children; cell; cell = cell->next) {
				unsigned முழு fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				முழு fixed_width = 0, percent_width = 0;
				எண்ணிக்கை css_width_e type;
				css_fixed மதிப்பு = 0;
				css_unit unit = CSS_UNIT_PX;

				எனில் (cell->columns == 1)
					தொடர்;
				i = cell->start_column;

				க்கு (j = i; j < i + cell->columns; j++) {
					col[j].positioned = false;
				}

				/* count column types in spanned cells */
				க்கு (j = 0; j != cell->columns; j++) {
					எனில் (col[i + j].type == COLUMN_WIDTH_FIXED) {
						fixed_width += col[i + j].width;
						fixed_columns++;
					} இல்லையெனில் எனில் (col[i + j].type == COLUMN_WIDTH_PERCENT) {
						percent_width += col[i + j].width;
						percent_columns++;
					} இல்லையெனில் எனில் (col[i + j].type == COLUMN_WIDTH_AUTO) {
						auto_columns++;
					} இல்லையெனில் {
						unknown_columns++;
					}
				}

				எனில் (!unknown_columns)
					தொடர்;

				type = css_computed_width(cell->style, &மதிப்பு, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				எனில் (type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					முழு width = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							மதிப்பு, unit)) -
						fixed_width) / unknown_columns;
					எனில் (width < 0)
						width = 0;
					க்கு (j = 0; j != cell->columns; j++) {
						எனில் (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_FIXED;
							col[i + j].width = width;
						}
					}
				}

				/* as above for percentage width */
				எனில் (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					முழு width = (FIXTOFLT(மதிப்பு) -
						     percent_width) / unknown_columns;
					எனில் (width < 0)
						width = 0;
					க்கு (j = 0; j != cell->columns; j++) {
						எனில் (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_PERCENT;
							col[i + j].width = width;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	க்கு (i = 0; i != table->columns; i++) {
		எனில் (col[i].type == COLUMN_WIDTH_UNKNOWN)
			col[i].type = COLUMN_WIDTH_AUTO;
	}

#ifdef TABLE_DEBUG
	க்கு (i = 0; i != table->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      table,
		      i,
		      ((மாறிலி எழுத்து *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[col[i].type],
		      col[i].width);
#endif

	திரும்பு true;
}


/* exported interface documented in html/table.h */
வெறுமை table_used_border_for_cell(மாறிலி css_unit_ctx *unit_len_ctx, கட்டமைப்பு box *cell)
{
	முழு side;

	ப்ரதிஜ்ஞா(cell->type == BOX_TABLE_CELL);

	எனில் (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed width = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[இடது_2].style =
			css_computed_border_இடது_style(cell->style);
		css_computed_border_இடது_color(cell->style,
					       &cell->border[இடது_2].c);
		css_computed_border_இடது_width(cell->style, &width, &unit);
		cell->border[இடது_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Top border */
		cell->border[உச்சி_2].style =
			css_computed_border_உச்சி_style(cell->style);
		css_computed_border_உச்சி_color(cell->style,
					      &cell->border[உச்சி_2].c);
		css_computed_border_உச்சி_width(cell->style, &width, &unit);
		cell->border[உச்சி_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Right border */
		cell->border[வலது_2].style =
			css_computed_border_வலது_style(cell->style);
		css_computed_border_வலது_color(cell->style,
						&cell->border[வலது_2].c);
		css_computed_border_வலது_width(cell->style, &width, &unit);
		cell->border[வலது_2].width =
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
	} இல்லையெனில் {
		/* Left border */
		table_used_இடது_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		table_used_உச்சி_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		table_used_வலது_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		table_used_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	க்கு (side = 0; side != 4; side++) {
		எனில் (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].width = 0;
	}
}
