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

#समावेश <assert.h>
#समावेश <dom/dom.h>

#समावेश "utils/log.h"
#समावेश "utils/utils.h"
#समावेश "utils/talloc.h"
#समावेश "css/utils.h"

#समावेश "html/box.h"
#समावेश "html/table.h"

/* Define to enable verbose table debug */
#अघोषित TABLE_DEBUG

/**
 * Container for border values during table border calculations
 */
काष्ठा border {
	क्रमागत css_border_style_e style;	/**< border-style */
	क्रमागत css_border_color_e color;	/**< border-color type */
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
अटल bool
table_border_is_more_eyecatching(स्थिर css_unit_ctx *unit_len_ctx,
				 स्थिर काष्ठा border *a,
				 box_type a_src,
				 स्थिर काष्ठा border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	पूर्णांक impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	अगर (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		वापस false;

	अगर (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		वापस true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	निश्चित(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	निश्चित(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(शून्य, unit_len_ctx, a->width, a->unit);
	bwidth = css_unit_len2device_px(शून्य, unit_len_ctx, b->width, b->unit);

	अगर (awidth < bwidth)
		वापस true;
	अन्यथा अगर (bwidth < awidth)
		वापस false;

	/* 3b -- sort by style */
	चयन (a->style) {
	हाल CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	हाल CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	हाल CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	हाल CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	हाल CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	हाल CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	हाल CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	हाल CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	शेष:
		अवरोध;
	}

	चयन (b->style) {
	हाल CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	हाल CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	हाल CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	हाल CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	हाल CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	हाल CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	हाल CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	हाल CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	शेष:
		अवरोध;
	}

	अगर (impact < 0)
		वापस true;
	अन्यथा अगर (impact > 0)
		वापस false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	चयन (a_src) {
	हाल BOX_TABLE_CELL:       impact++; fallthrough;
	हाल BOX_TABLE_ROW:        impact++; fallthrough;
	हाल BOX_TABLE_ROW_GROUP:  impact++; fallthrough;
	हाल BOX_TABLE:            impact++; fallthrough;
	शेष:
		अवरोध;
	}

	/** \todo COL/COL_GROUP */
	चयन (b_src) {
	हाल BOX_TABLE_CELL:       impact--; fallthrough;
	हाल BOX_TABLE_ROW:        impact--; fallthrough;
	हाल BOX_TABLE_ROW_GROUP:  impact--; fallthrough;
	हाल BOX_TABLE:            impact--; fallthrough;
	शेष:
		अवरोध;
	}

	अगर (impact < 0)
		वापस true;
	अन्यथा अगर (impact > 0)
		वापस false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	वापस true;
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
अटल व्योम
table_cell_शीर्ष_process_table(स्थिर css_unit_ctx *unit_len_ctx,
			     काष्ठा box *table,
			     काष्ठा border *a,
			     box_type *a_src)
{
	काष्ठा border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_शीर्ष_style(table->style);
	b.color = css_computed_border_शीर्ष_color(table->style, &b.c);
	css_computed_border_शीर्ष_width(table->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(table->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE;

	अगर (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
अटल bool
table_cell_शीर्ष_process_row(स्थिर css_unit_ctx *unit_len_ctx,
			   काष्ठा box *cell,
			   काष्ठा box *row,
			   काष्ठा border *a,
			   box_type *a_src)
{
	काष्ठा border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(row->style);
	b.color = css_computed_border_bottom_color(row->style, &b.c);
	css_computed_border_bottom_width(row->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(row->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW;

	अगर (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	अगर (row->children == शून्य) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_शीर्ष_style(row->style);
		b.color = css_computed_border_शीर्ष_color(row->style, &b.c);
		css_computed_border_शीर्ष_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		अगर (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		वापस false;
	} अन्यथा {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		काष्ठा box *c;
		bool processed = false;

		जबतक (processed == false) {
			क्रम (c = row->children; c != शून्य; c = c->अग्र) {
				/* Ignore cells to the left */
				अगर (c->start_column + c->columns - 1 <
				    cell->start_column)
					जारी;
				/* Ignore cells to the right */
				अगर (c->start_column > cell->start_column +
				    cell->columns - 1)
					जारी;

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

				अगर (table_border_is_more_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			अगर (processed == false) {
				/* There must be a preceding row */
				निश्चित(row->पूर्व != शून्य);

				row = row->पूर्व;
			}
		}
	}

	वापस true;
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
अटल bool
table_cell_शीर्ष_process_group(स्थिर css_unit_ctx *unit_len_ctx,
			     काष्ठा box *cell,
			     काष्ठा box *group,
			     काष्ठा border *a,
			     box_type *a_src)
{
	काष्ठा border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(group->style);
	b.color = css_computed_border_bottom_color(group->style, &b.c);
	css_computed_border_bottom_width(group->style, &b.width, &b.unit);
	b.width = css_unit_len2device_px(group->style, unit_len_ctx,
			b.width, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_TABLE_ROW_GROUP;

	अगर (table_border_is_more_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	अगर (group->last != शून्य) {
		/* Process rows in group, starting with last */
		काष्ठा box *row = group->last;

		जबतक (table_cell_शीर्ष_process_row(unit_len_ctx, cell, row,
						  a, a_src) == false) {
			अगर (row->पूर्व == शून्य) {
				वापस false;
			} अन्यथा {
				row = row->पूर्व;
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

		अगर (table_border_is_more_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		वापस false;
	}

	वापस true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
अटल व्योम
table_used_बायाँ_border_for_cell(स्थिर css_unit_ctx *unit_len_ctx, काष्ठा box *cell)
{
	काष्ठा border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_बायाँ_style(cell->style);
	a.color = css_computed_border_बायाँ_color(cell->style, &a.c);
	css_computed_border_बायाँ_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	अगर (cell->पूर्व != शून्य || cell->start_column != 0) {
		/* Cell to the left -- consider its right border */
		काष्ठा box *पूर्व = शून्य;

		अगर (cell->पूर्व == शून्य) {
			काष्ठा box *row;

			/* Spanned from a previous row in current row group */
			क्रम (row = cell->parent; row != शून्य; row = row->पूर्व) {
				क्रम (पूर्व = row->children; पूर्व != शून्य;
				     पूर्व = पूर्व->अग्र) {
					अगर (पूर्व->start_column +
					    पूर्व->columns ==
					    cell->start_column)
						अवरोध;
				}

				अगर (पूर्व != शून्य)
					अवरोध;
			}

			निश्चित(पूर्व != शून्य);
		} अन्यथा {
			पूर्व = cell->पूर्व;
		}

		b.style = css_computed_border_दायाँ_style(पूर्व->style);
		b.color = css_computed_border_दायाँ_color(पूर्व->style, &b.c);
		css_computed_border_दायाँ_width(पूर्व->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(पूर्व->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_CELL;

		अगर (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} अन्यथा {
		/* First cell in row, so consider rows and row group */
		काष्ठा box *row = cell->parent;
		काष्ठा box *group = row->parent;
		काष्ठा box *table = group->parent;
		अचिन्हित पूर्णांक rows = cell->rows;

		जबतक (rows-- > 0 && row != शून्य) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_बायाँ_style(row->style);
			b.color = css_computed_border_बायाँ_color(
								 row->style, &b.c);
			css_computed_border_बायाँ_width(
						       row->style, &b.width, &b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			अगर (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->अग्र;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_बायाँ_style(group->style);
		b.color = css_computed_border_बायाँ_color(group->style, &b.c);
		css_computed_border_बायाँ_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		अगर (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_बायाँ_style(table->style);
		b.color = css_computed_border_बायाँ_color(table->style, &b.c);
		css_computed_border_बायाँ_width(table->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		अगर (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[बायाँ_2].style = a.style;
	cell->border[बायाँ_2].c = a.c;
	cell->border[बायाँ_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
अटल व्योम
table_used_शीर्ष_border_for_cell(स्थिर css_unit_ctx *unit_len_ctx, काष्ठा box *cell)
{
	काष्ठा border a, b;
	box_type a_src, b_src;
	काष्ठा box *row = cell->parent;
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

	अगर (table_border_is_more_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	अगर (row->पूर्व != शून्य) {
		/* Consider row(s) above */
		जबतक (table_cell_शीर्ष_process_row(unit_len_ctx, cell, row->पूर्व,
						  &a, &a_src) == false) {
			अगर (row->पूर्व->पूर्व == शून्य) {
				/* Consider row group */
				process_group = true;
				अवरोध;
			} अन्यथा {
				row = row->पूर्व;
			}
		}
	} अन्यथा {
		process_group = true;
	}

	अगर (process_group) {
		काष्ठा box *group = row->parent;

		/* Top border of row group */
		b.style = css_computed_border_शीर्ष_style(group->style);
		b.color = css_computed_border_शीर्ष_color(group->style, &b.c);
		css_computed_border_शीर्ष_width(group->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		अगर (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		अगर (group->पूर्व == शून्य) {
			/* Top border of table */
			table_cell_शीर्ष_process_table(unit_len_ctx,
						     group->parent, &a, &a_src);
		} अन्यथा {
			/* Process previous group(s) */
			जबतक (table_cell_शीर्ष_process_group(unit_len_ctx,
							    cell, group->पूर्व,
							    &a, &a_src) == false) {
				अगर (group->पूर्व->पूर्व == शून्य) {
					/* Top border of table */
					table_cell_शीर्ष_process_table(unit_len_ctx,
								     group->parent,
								     &a, &a_src);
					अवरोध;
				} अन्यथा {
					group = group->पूर्व;
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
अटल व्योम
table_used_दायाँ_border_for_cell(स्थिर css_unit_ctx *unit_len_ctx, काष्ठा box *cell)
{
	काष्ठा border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_दायाँ_style(cell->style);
	css_computed_border_दायाँ_color(cell->style, &a.c);
	css_computed_border_दायाँ_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	अगर (cell->अग्र != शून्य || cell->start_column + cell->columns !=
	    cell->parent->parent->parent->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} अन्यथा {
		/* Last cell in row, so consider rows and row group */
		काष्ठा box *row = cell->parent;
		काष्ठा box *group = row->parent;
		काष्ठा box *table = group->parent;
		अचिन्हित पूर्णांक rows = cell->rows;

		जबतक (rows-- > 0 && row != शून्य) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_दायाँ_style(row->style);
			b.color = css_computed_border_दायाँ_color(row->style,
								  &b.c);
			css_computed_border_दायाँ_width(row->style,
							&b.width,
							&b.unit);
			b.width = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.width, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_TABLE_ROW;

			अगर (table_border_is_more_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->अग्र;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_दायाँ_style(group->style);
		b.color = css_computed_border_दायाँ_color(group->style, &b.c);
		css_computed_border_दायाँ_width(group->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(group->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW_GROUP;

		अगर (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_दायाँ_style(table->style);
		b.color = css_computed_border_दायाँ_color(table->style, &b.c);
		css_computed_border_दायाँ_width(table->style,
						&b.width, &b.unit);
		b.width = css_unit_len2device_px(table->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE;

		अगर (table_border_is_more_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[दायाँ_2].style = a.style;
	cell->border[दायाँ_2].c = a.c;
	cell->border[दायाँ_2].width = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.width, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
अटल व्योम
table_used_bottom_border_for_cell(स्थिर css_unit_ctx *unit_len_ctx,
				  काष्ठा box *cell)
{
	काष्ठा border a, b;
	box_type a_src, b_src;
	काष्ठा box *row = cell->parent;
	अचिन्हित पूर्णांक rows = cell->rows;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_width(cell->style, &a.width, &a.unit);
	a.width = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.width, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_TABLE_CELL;

	जबतक (rows-- > 0 && row != शून्य)
		row = row->अग्र;

	/** \todo Can cells span row groups? */

	अगर (row != शून्य) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.width = 0;
		a.unit = CSS_UNIT_PX;
	} अन्यथा {
		/* Cell at bottom of table, so consider row and row group */
		काष्ठा box *row = cell->parent;
		काष्ठा box *group = row->parent;
		काष्ठा box *table = group->parent;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(row->style);
		b.color = css_computed_border_bottom_color(row->style, &b.c);
		css_computed_border_bottom_width(row->style, &b.width, &b.unit);
		b.width = css_unit_len2device_px(row->style, unit_len_ctx,
				b.width, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_TABLE_ROW;

		अगर (table_border_is_more_eyecatching(unit_len_ctx,
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

		अगर (table_border_is_more_eyecatching(unit_len_ctx,
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

		अगर (table_border_is_more_eyecatching(unit_len_ctx,
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
table_calculate_column_types(स्थिर css_unit_ctx *unit_len_ctx, काष्ठा box *table)
{
	अचिन्हित पूर्णांक i, j;
	काष्ठा column *col;
	काष्ठा box *row_group, *row, *cell;

	अगर (table->col)
		/* table->col already constructed, for example frameset table */
		वापस true;

	table->col = col = talloc_array(table, काष्ठा column, table->columns);
	अगर (!col)
		वापस false;

	क्रम (i = 0; i != table->columns; i++) {
		col[i].type = COLUMN_WIDTH_UNKNOWN;
		col[i].width = 0;
		col[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	क्रम (row_group = table->children; row_group; row_group =row_group->अग्र)
		क्रम (row = row_group->children; row; row = row->अग्र)
			क्रम (cell = row->children; cell; cell = cell->अग्र) {
				क्रमागत css_width_e type;
				css_fixed मूल्य = 0;
				css_unit unit = CSS_UNIT_PX;

				निश्चित(cell->type == BOX_TABLE_CELL);
				निश्चित(cell->style);

				अगर (cell->columns != 1)
					जारी;
				i = cell->start_column;

				अगर (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					col[i].positioned = false;
				}

				type = css_computed_width(cell->style, &मूल्य, &unit);

				/* fixed width takes priority over any other width type */
				अगर (col[i].type != COLUMN_WIDTH_FIXED &&
				    type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_FIXED;
					col[i].width = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							मूल्य, unit));
					अगर (col[i].width < 0)
						col[i].width = 0;
					जारी;
				}

				अगर (col[i].type != COLUMN_WIDTH_UNKNOWN)
					जारी;

				अगर (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT) {
					col[i].type = COLUMN_WIDTH_PERCENT;
					col[i].width = FIXTOINT(मूल्य);
					अगर (col[i].width < 0)
						col[i].width = 0;
				} अन्यथा अगर (type == CSS_WIDTH_AUTO) {
					col[i].type = COLUMN_WIDTH_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	क्रम (row_group = table->children; row_group; row_group =row_group->अग्र)
		क्रम (row = row_group->children; row; row = row->अग्र)
			क्रम (cell = row->children; cell; cell = cell->अग्र) {
				अचिन्हित पूर्णांक fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				पूर्णांक fixed_width = 0, percent_width = 0;
				क्रमागत css_width_e type;
				css_fixed मूल्य = 0;
				css_unit unit = CSS_UNIT_PX;

				अगर (cell->columns == 1)
					जारी;
				i = cell->start_column;

				क्रम (j = i; j < i + cell->columns; j++) {
					col[j].positioned = false;
				}

				/* count column types in spanned cells */
				क्रम (j = 0; j != cell->columns; j++) {
					अगर (col[i + j].type == COLUMN_WIDTH_FIXED) {
						fixed_width += col[i + j].width;
						fixed_columns++;
					} अन्यथा अगर (col[i + j].type == COLUMN_WIDTH_PERCENT) {
						percent_width += col[i + j].width;
						percent_columns++;
					} अन्यथा अगर (col[i + j].type == COLUMN_WIDTH_AUTO) {
						auto_columns++;
					} अन्यथा {
						unknown_columns++;
					}
				}

				अगर (!unknown_columns)
					जारी;

				type = css_computed_width(cell->style, &मूल्य, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				अगर (type == CSS_WIDTH_SET && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					पूर्णांक width = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							मूल्य, unit)) -
						fixed_width) / unknown_columns;
					अगर (width < 0)
						width = 0;
					क्रम (j = 0; j != cell->columns; j++) {
						अगर (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_FIXED;
							col[i + j].width = width;
						}
					}
				}

				/* as above for percentage width */
				अगर (type == CSS_WIDTH_SET && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					पूर्णांक width = (FIXTOFLT(मूल्य) -
						     percent_width) / unknown_columns;
					अगर (width < 0)
						width = 0;
					क्रम (j = 0; j != cell->columns; j++) {
						अगर (col[i + j].type == COLUMN_WIDTH_UNKNOWN) {
							col[i + j].type = COLUMN_WIDTH_PERCENT;
							col[i + j].width = width;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	क्रम (i = 0; i != table->columns; i++) {
		अगर (col[i].type == COLUMN_WIDTH_UNKNOWN)
			col[i].type = COLUMN_WIDTH_AUTO;
	}

#अगर_घोषित TABLE_DEBUG
	क्रम (i = 0; i != table->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      table,
		      i,
		      ((स्थिर अक्षर *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[col[i].type],
		      col[i].width);
#पूर्ण_अगर

	वापस true;
}


/* exported interface documented in html/table.h */
व्योम table_used_border_for_cell(स्थिर css_unit_ctx *unit_len_ctx, काष्ठा box *cell)
{
	पूर्णांक side;

	निश्चित(cell->type == BOX_TABLE_CELL);

	अगर (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed width = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[बायाँ_2].style =
			css_computed_border_बायाँ_style(cell->style);
		css_computed_border_बायाँ_color(cell->style,
					       &cell->border[बायाँ_2].c);
		css_computed_border_बायाँ_width(cell->style, &width, &unit);
		cell->border[बायाँ_2].width =
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
		cell->border[दायाँ_2].style =
			css_computed_border_दायाँ_style(cell->style);
		css_computed_border_दायाँ_color(cell->style,
						&cell->border[दायाँ_2].c);
		css_computed_border_दायाँ_width(cell->style, &width, &unit);
		cell->border[दायाँ_2].width =
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
		table_used_बायाँ_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		table_used_शीर्ष_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		table_used_दायाँ_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		table_used_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	क्रम (side = 0; side != 4; side++) {
		अगर (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].width = 0;
	}
}
