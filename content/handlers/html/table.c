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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <dom/dom.h>

#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/talloc.h"
#अन्तर्भाव "css/utils.h"

#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/table.h"

/* Define to enable verbose table debug */
#undef TABLE_DEBUG

/**
 * Container for border values during table border calculations
 */
संरचना border {
	गणना css_border_style_e style;	/**< border-style */
	गणना css_border_color_e color;	/**< border-color type */
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
स्थिर bool
table_border_is_more_eyecatching(ध्रुव css_unit_ctx *unit_len_ctx,
				 ध्रुव संरचना border *a,
				 box_type a_src,
				 ध्रुव संरचना border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	पूर्णाङ्क impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	यदि (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		प्रत्यागम false;

	यदि (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		प्रत्यागम true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	प्रतिज्ञा(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	प्रतिज्ञा(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(रिक्त, unit_len_ctx, a->width, a->unit);
	bwidth = css_unit_len2device_px(रिक्त, unit_len_ctx, b->width, b->unit);

	यदि (awidth < bwidth)
		प्रत्यागम true;
	अन्यथा यदि (bwidth < awidth)
		प्रत्यागम false;

	/* 3b -- sort by style */
	विकल्प (a->style) {
	पक्ष CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	पक्ष CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	पक्ष CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	पक्ष CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	पक्ष CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	पक्ष CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	पक्ष CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	पक्ष CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	स्वभाव:
		भङ्ग;
	}

	विकल्प (b->style) {
	पक्ष CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	पक्ष CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	पक्ष CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	पक्ष CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	पक्ष CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	पक्ष CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	पक्ष CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	पक्ष CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	स्वभाव:
		भङ्ग;
	}

	यदि (impact < 0)
		प्रत्यागम true;
	अन्यथा यदि (impact > 0)
		प्रत्यागम false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	विकल्प (a_src) {
	पक्ष BOX_TABLE_CELL:       impact++; fallthrough;
	पक्ष BOX_TABLE_ROW:        impact++; fallthrough;
	पक्ष BOX_TABLE_ROW_GROUP:  impact++; fallthrough;
	पक्ष BOX_TABLE:            impact++; fallthrough;
	स्वभाव:
		भङ्ग;
	}

	/** \todo COL/COL_GROUP */
	विकल्प (b_src) {
	पक्ष BOX_TABLE_CELL:       impact--; fallthrough;
	पक्ष BOX_TABLE_ROW:        impact--; fallthrough;
	पक्ष BOX_TABLE_ROW_GROUP:  impact--; fallthrough;
	पक्ष BOX_TABLE:            impact--; fallthrough;
	स्वभाव:
		भङ्ग;
	}

	यदि (impact < 0)
		प्रत्यागम true;
	अन्यथा यदि (impact > 0)
		प्रत्यागम false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	प्रत्यागम true;
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
स्थिर शून्य
table_cell_शीर्ष_process_table(ध्रुव css_unit_ctx *unit_len_ctx,
			     संरचना box *table,
			     संरचना border *a,
			     box_type *a_src)
{
	संरचना border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_शीर्ष_style(table->style);
	b.color = css_computed_border_शीर्ष_color(table->style, &b.c);
	css_computed_border_शीर्ष_width(table->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(table->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE;

	यदि (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
स्थिर bool
table_cell_शीर्ष_process_row(ध्रुव css_unit_ctx *unit_len_ctx,
			   संरचना box *cell,
			   संरचना box *row,
			   संरचना border *a,
			   box_type *a_src)
{
	संरचना border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(row->style);
	b.color = css_computed_border_bottom_color(row->style, &b.c);
	css_computed_border_bottom_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	यदि (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	यदि (row->children == रिक्त) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_शीर्ष_style(row->style);
		b.color = css_computed_border_शीर्ष_color(row->style, &b.c);
		css_computed_border_शीर्ष_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		यदि (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		प्रत्यागम false;
	} अन्यथा {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		संरचना box *c;
		bool processed = false;

		यावत् (processed == false) {
			प्रति (c = row->children; c != रिक्त; c = c->next) {
				/* Ignore cells to the left */
				यदि (c->start_column + c->columns - 1 <
				    cell->start_column)
					अनुवर्त;
				/* Ignore cells to the right */
				यदि (c->start_column > cell->start_column +
				    cell->columns - 1)
					अनुवर्त;

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

				यदि (table_border_is_more_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			यदि (processed == false) {
				/* There must be a preceding row */
				प्रतिज्ञा(row->prev != रिक्त);

				row = row->prev;
			}
		}
	}

	प्रत्यागम true;
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
स्थिर bool
table_cell_शीर्ष_process_group(ध्रुव css_unit_ctx *unit_len_ctx,
			     संरचना box *cell,
			     संरचना box *group,
			     संरचना border *a,
			     box_type *a_src)
{
	संरचना border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(group->style);
	b.color = css_computed_border_bottom_color(group->style, &b.c);
	css_computed_border_bottom_width(group->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(group->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW_GROUP;

	यदि (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	यदि (group->last != रिक्त) {
		/* Process rows in group, starting with last */
		संरचना box *row = group->last;

		यावत् (table_cell_शीर्ष_process_row(unit_len_ctx, cell, row,
						  a, a_src) == false) {
			यदि (row->prev == रिक्त) {
				प्रत्यागम false;
			} अन्यथा {
				row = row->prev;
			}
		}
	} अन्यथा {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_शीर्ष_style(group->style);
		b.color = css_computed_border_शीर्ष_color(group->style, &b.c);
		css_computed_border_शीर्ष_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		यदि (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		प्रत्यागम false;
	}

	प्रत्यागम true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
स्थिर शून्य
table_used_डावा_border_for_cell(ध्रुव css_unit_ctx *unit_len_ctx, संरचना box *cell)
{
	संरचना border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_डावा_style(cell->style);
	a.color = css_computed_border_डावा_color(cell->style, &a.c);
	css_computed_border_डावा_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	यदि (cell->prev != रिक्त || cell->start_column != 0) {
		/* Cell to the left -- consider its right border */
		संरचना box *prev = रिक्त;

		यदि (cell->prev == रिक्त) {
			संरचना box *row;

			/* Spanned from a previous row in current row group */
			प्रति (row = cell->parent; row != रिक्त; row = row->prev) {
				प्रति (prev = row->children; prev != रिक्त;
				     prev = prev->next) {
					यदि (prev->start_column +
					    prev->columns ==
					    cell->start_column)
						भङ्ग;
				}

				यदि (prev != रिक्त)
					भङ्ग;
			}

			प्रतिज्ञा(prev != रिक्त);
		} अन्यथा {
			prev = cell->prev;
		}

		b.style = css_computed_border_उजवा_style(prev->style);
		b.color = css_computed_border_उजवा_color(prev->style, &b.c);
		css_computed_border_उजवा_width(prev->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_CELL;

		यदि (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} अन्यथा {
		/* First cell in row, so consider rows and row group */
		संरचना box *row = cell->parent;
		संरचना box *group = row->parent;
		संरचना box *table = group->parent;
		unsigned पूर्णाङ्क rows = cell->rows;

		यावत् (rows-- > 0 && row != रिक्त) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_डावा_style(row->style);
			b.color = css_computed_border_डावा_color(
								 row->style, &b.c);
			css_computed_border_डावा_width(
						       row->style, &b.width, &b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			यदि (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_डावा_style(group->style);
		b.color = css_computed_border_डावा_color(group->style, &b.c);
		css_computed_border_डावा_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		यदि (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_डावा_style(table->style);
		b.color = css_computed_border_डावा_color(table->style, &b.c);
		css_computed_border_डावा_width(table->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		यदि (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[डावा_2].style = a.style;
	cell->border[डावा_2].c = a.c;
	cell->border[डावा_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
स्थिर शून्य
table_used_शीर्ष_border_for_cell(ध्रुव css_unit_ctx *unit_len_ctx, संरचना box *cell)
{
	संरचना border a, b;
	box_type a_src, b_src;
	संरचना box *row = cell->parent;
	bool process_group = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_शीर्ष_style(cell->style);
	css_computed_border_शीर्ष_color(cell->style, &a.c);
	css_computed_border_शीर्ष_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	/* Top border of row */
	b.style = css_computed_border_शीर्ष_style(row->style);
	css_computed_border_शीर्ष_color(row->style, &b.c);
	css_computed_border_शीर्ष_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	यदि (table_border_is_more_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	यदि (row->prev != रिक्त) {
		/* Consider row(s) above */
		यावत् (table_cell_शीर्ष_process_row(unit_len_ctx, cell, row->prev,
						  &a, &a_src) == false) {
			यदि (row->prev->prev == रिक्त) {
				/* Consider row group */
				process_group = true;
				भङ्ग;
			} अन्यथा {
				row = row->prev;
			}
		}
	} अन्यथा {
		process_group = true;
	}

	यदि (process_group) {
		संरचना box *group = row->parent;

		/* Top border of row group */
		b.style = css_computed_border_शीर्ष_style(group->style);
		b.color = css_computed_border_शीर्ष_color(group->style, &b.c);
		css_computed_border_शीर्ष_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		यदि (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		यदि (group->prev == रिक्त) {
			/* Top border of table */
			table_cell_शीर्ष_process_table(unit_len_ctx,
						     group->parent, &a, &a_src);
		} अन्यथा {
			/* Process previous group(s) */
			यावत् (table_cell_शीर्ष_process_group(unit_len_ctx,
							    cell, group->prev,
							    &a, &a_src) == false) {
				यदि (group->prev->prev == रिक्त) {
					/* Top border of table */
					table_cell_शीर्ष_process_table(unit_len_ctx,
								     group->parent,
								     &a, &a_src);
					भङ्ग;
				} अन्यथा {
					group = group->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[शीर्ष_2].style = a.style;
	cell->border[शीर्ष_2].c = a.c;
	cell->border[शीर्ष_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
स्थिर शून्य
table_used_उजवा_border_for_cell(ध्रुव css_unit_ctx *unit_len_ctx, संरचना box *cell)
{
	संरचना border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_उजवा_style(cell->style);
	css_computed_border_उजवा_color(cell->style, &a.c);
	css_computed_border_उजवा_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	यदि (cell->next != रिक्त || cell->start_column + cell->columns !=
	    cell->parent->parent->parent->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} अन्यथा {
		/* Last cell in row, so consider rows and row group */
		संरचना box *row = cell->parent;
		संरचना box *group = row->parent;
		संरचना box *table = group->parent;
		unsigned पूर्णाङ्क rows = cell->rows;

		यावत् (rows-- > 0 && row != रिक्त) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_उजवा_style(row->style);
			b.color = css_computed_border_उजवा_color(row->style,
								  &b.c);
			css_computed_border_उजवा_width(row->style,
							&b.width,
							&b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			यदि (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->next;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_उजवा_style(group->style);
		b.color = css_computed_border_उजवा_color(group->style, &b.c);
		css_computed_border_उजवा_width(group->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		यदि (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_उजवा_style(table->style);
		b.color = css_computed_border_उजवा_color(table->style, &b.c);
		css_computed_border_उजवा_width(table->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		यदि (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[उजवा_2].style = a.style;
	cell->border[उजवा_2].c = a.c;
	cell->border[उजवा_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
स्थिर शून्य
table_used_bottom_border_for_cell(ध्रुव css_unit_ctx *unit_len_ctx,
				  संरचना box *cell)
{
	संरचना border a, b;
	box_type a_src, b_src;
	संरचना box *row = cell->parent;
	unsigned पूर्णाङ्क rows = cell->rows;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	यावत् (rows-- > 0 && row != रिक्त)
		row = row->next;

	/** \todo Can cells span row groups? */

	यदि (row != रिक्त) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} अन्यथा {
		/* Cell at bottom of table, so consider row and row group */
		संरचना box *row = cell->parent;
		संरचना box *group = row->parent;
		संरचना box *table = group->parent;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(row->style);
		b.color = css_computed_border_bottom_color(row->style, &b.c);
		css_computed_border_bottom_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		यदि (table_border_is_more_eyecatching(unit_len_ctx,
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

		यदि (table_border_is_more_eyecatching(unit_len_ctx,
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

		यदि (table_border_is_more_eyecatching(unit_len_ctx,
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
table_calculate_column_types(ध्रुव css_unit_ctx *unit_len_ctx, संरचना box *table)
{
	unsigned पूर्णाङ्क i, j;
	संरचना column *col;
	संरचना box *row_group, *row, *cell;

	यदि (table->col)
		/* table->col already constructed, for example frameset table */
		प्रत्यागम true;

	table->col = col = talloc_array(table, संरचना column, table->columns);
	यदि (!col)
		प्रत्यागम false;

	प्रति (i = 0; i != table->columns; i++) {
		col[i].type = COLUMN_WIDTH_UNKNOWN;
		col[i].width = 0;
		col[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	प्रति (row_group = table->children; row_group; row_group =row_group->next)
		प्रति (row = row_group->children; row; row = row->next)
			प्रति (cell = row->children; cell; cell = cell->next) {
				गणना css_width_e type;
				css_fixed मूल्य = 0;
				css_unit unit = CSS_UNIT_PX;

				प्रतिज्ञा(cell->type == BOX_TABLE_CELL);
				प्रतिज्ञा(cell->style);

				यदि (cell->columns != 1)
					अनुवर्त;
				i = cell->start_column;

				यदि (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					col[i].positioned = false;
				}

				type = css_computed_width(cell->style, &मूल्य, &unit);

				/* fixed width takes priority over any other width type */
				यदि (col[i].type != COLUMN_WIDTH_FIXED &&
				    type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_FIXED;
					col[i].width = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							मूल्य, unit));
					यदि (col[i].width < 0)
						col[i].width = 0;
					अनुवर्त;
				}

				यदि (col[i].type != COLUMN_WIDTH_UNKNOWN)
					अनुवर्त;

				यदि (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_PERCENT;
					col[i].width = FIXTOINT(मूल्य);
					यदि (col[i].width < 0)
						col[i].width = 0;
				} अन्यथा यदि (type == CSS_WIDTH_AUTO) {
					col[i].type = COLUMN_WIDTH_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	प्रति (row_group = table->children; row_group; row_group =row_group->next)
		प्रति (row = row_group->children; row; row = row->next)
			प्रति (cell = row->children; cell; cell = cell->next) {
				unsigned पूर्णाङ्क fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				पूर्णाङ्क fixed_width = 0, percent_width = 0;
				गणना css_width_e type;
				css_fixed मूल्य = 0;
				css_unit unit = CSS_UNIT_PX;

				यदि (cell->columns == 1)
					अनुवर्त;
				i = cell->start_column;

				प्रति (j = i; j < i + cell->columns; j++) {
					col[j].positioned = false;
				}

				/* count column types in spanned cells */
				प्रति (j = 0; j != cell->columns; j++) {
					यदि (col[i + j].type == COLUMN_WIDTH_FIXED) {
						fixed_width += col[i + j].width;
						fixed_columns++;
					} अन्यथा यदि (col[i + j].type == COLUMN_WIDTH_PERCENT) {
						percent_width += col[i + j].width;
						percent_columns++;
					} अन्यथा यदि (col[i + j].type == COLUMN_WIDTH_AUTO) {
						auto_columns++;
					} अन्यथा {
						unknown_columns++;
					}
				}

				यदि (!unknown_columns)
					अनुवर्त;

				type = css_computed_width(cell->style, &मूल्य, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				यदि (type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					पूर्णाङ्क width = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							मूल्य, unit)) -
						fixed_width) / unknown_columns;
					यदि (width < 0)
						width = 0;
					प्रति (j = 0; j != cell->columns; j++) {
						यदि (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_FIXED;
							col[i + j].width = width;
						}
					}
				}

				/* as above for percentage width */
				यदि (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					पूर्णाङ्क width = (FIXTOFLT(मूल्य) -
						     percent_width) / unknown_columns;
					यदि (width < 0)
						width = 0;
					प्रति (j = 0; j != cell->columns; j++) {
						यदि (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_PERCENT;
							col[i + j].width = width;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	प्रति (i = 0; i != table->columns; i++) {
		यदि (col[i].type == COLUMN_WIDTH_UNKNOWN)
			col[i].type = COLUMN_WIDTH_AUTO;
	}

#ifdef TABLE_DEBUG
	प्रति (i = 0; i != table->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      table,
		      i,
		      ((ध्रुव वर्ण *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[col[i].type],
		      col[i].width);
#endif

	प्रत्यागम true;
}


/* exported interface documented in html/table.h */
शून्य table_used_border_for_cell(ध्रुव css_unit_ctx *unit_len_ctx, संरचना box *cell)
{
	पूर्णाङ्क side;

	प्रतिज्ञा(cell->type == BOX_TABLE_CELL);

	यदि (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed width = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[डावा_2].style =
			css_computed_border_डावा_style(cell->style);
		css_computed_border_डावा_color(cell->style,
					       &cell->border[डावा_2].c);
		css_computed_border_डावा_width(cell->style, &width, &unit);
		cell->border[डावा_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Top border */
		cell->border[शीर्ष_2].style =
			css_computed_border_शीर्ष_style(cell->style);
		css_computed_border_शीर्ष_color(cell->style,
					      &cell->border[शीर्ष_2].c);
		css_computed_border_शीर्ष_width(cell->style, &width, &unit);
		cell->border[शीर्ष_2].width =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					width, unit));

		/* Right border */
		cell->border[उजवा_2].style =
			css_computed_border_उजवा_style(cell->style);
		css_computed_border_उजवा_color(cell->style,
						&cell->border[उजवा_2].c);
		css_computed_border_उजवा_width(cell->style, &width, &unit);
		cell->border[उजवा_2].width =
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
	} अन्यथा {
		/* Left border */
		table_used_डावा_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		table_used_शीर्ष_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		table_used_उजवा_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		table_used_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	प्रति (side = 0; side != 4; side++) {
		यदि (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].width = 0;
	}
}
