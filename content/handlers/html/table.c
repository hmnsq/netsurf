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
#अपरिभाषय सारणी_DEBUG

/**
 * Container for border values during table border calculations
 */
संरचना border {
	गणना css_border_style_e style;	/**< border-style */
	गणना css_border_color_e color;	/**< border-color type */
	css_color c;			/**< border-color value */
	css_fixed विस्तारः;		/**< border-width length */
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
सारणी_border_is_अधिकम्_eyecatching(ध्रुव css_unit_ctx *unit_len_ctx,
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
	awidth = css_unit_len2device_px(रिक्त, unit_len_ctx, a->विस्तारः, a->unit);
	bwidth = css_unit_len2device_px(रिक्त, unit_len_ctx, b->विस्तारः, b->unit);

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
	पक्ष BOX_सारणी_CELL:       impact++; fallthrough;
	पक्ष BOX_सारणी_ROW:        impact++; fallthrough;
	पक्ष BOX_सारणी_ROW_गणः:  impact++; fallthrough;
	पक्ष BOX_सारणी:            impact++; fallthrough;
	स्वभाव:
		भङ्ग;
	}

	/** \todo COL/COL_GROUP */
	विकल्प (b_src) {
	पक्ष BOX_सारणी_CELL:       impact--; fallthrough;
	पक्ष BOX_सारणी_ROW:        impact--; fallthrough;
	पक्ष BOX_सारणी_ROW_गणः:  impact--; fallthrough;
	पक्ष BOX_सारणी:            impact--; fallthrough;
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
सारणी_cell_शिखरम्_process_सारणी(ध्रुव css_unit_ctx *unit_len_ctx,
			     संरचना box *सारणी,
			     संरचना border *a,
			     box_type *a_src)
{
	संरचना border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_शिखरम्_style(सारणी->style);
	b.color = css_computed_border_शिखरम्_color(सारणी->style, &b.c);
	css_computed_border_शिखरम्_विस्तारः(सारणी->style, &b.विस्तारः, &b.unit);
	b.विस्तारः = css_unit_len2device_px(सारणी->style, unit_len_ctx,
			b.विस्तारः, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_सारणी;

	यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
सारणी_cell_शिखरम्_process_row(ध्रुव css_unit_ctx *unit_len_ctx,
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
	css_computed_border_bottom_विस्तारः(row->style, &b.विस्तारः, &b.unit);
	b.विस्तारः = css_unit_len2device_px(row->style, unit_len_ctx,
			b.विस्तारः, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_सारणी_ROW;

	यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	यदि (row->children == रिक्त) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_शिखरम्_style(row->style);
		b.color = css_computed_border_शिखरम्_color(row->style, &b.c);
		css_computed_border_शिखरम्_विस्तारः(row->style, &b.विस्तारः, &b.unit);
		b.विस्तारः = css_unit_len2device_px(row->style, unit_len_ctx,
				b.विस्तारः, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_सारणी_ROW;

		यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
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
			प्रति (c = row->children; c != रिक्त; c = c->अग्र) {
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
				css_computed_border_bottom_विस्तारः(c->style,
							&b.विस्तारः, &b.unit);
				b.विस्तारः = css_unit_len2device_px(
						c->style, unit_len_ctx,
						b.विस्तारः, b.unit);
				b.unit = CSS_UNIT_PX;
				b_src = BOX_सारणी_CELL;

				यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
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
				प्रतिज्ञा(row->पूर्व != रिक्त);

				row = row->पूर्व;
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
सारणी_cell_शिखरम्_process_गणः(ध्रुव css_unit_ctx *unit_len_ctx,
			     संरचना box *cell,
			     संरचना box *गणः,
			     संरचना border *a,
			     box_type *a_src)
{
	संरचना border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(गणः->style);
	b.color = css_computed_border_bottom_color(गणः->style, &b.c);
	css_computed_border_bottom_विस्तारः(गणः->style, &b.विस्तारः, &b.unit);
	b.विस्तारः = css_unit_len2device_px(गणः->style, unit_len_ctx,
			b.विस्तारः, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_सारणी_ROW_गणः;

	यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	यदि (गणः->चरमः != रिक्त) {
		/* Process rows in group, starting with last */
		संरचना box *row = गणः->चरमः;

		यावत् (सारणी_cell_शिखरम्_process_row(unit_len_ctx, cell, row,
						  a, a_src) == false) {
			यदि (row->पूर्व == रिक्त) {
				प्रत्यागम false;
			} अन्यथा {
				row = row->पूर्व;
			}
		}
	} अन्यथा {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_शिखरम्_style(गणः->style);
		b.color = css_computed_border_शिखरम्_color(गणः->style, &b.c);
		css_computed_border_शिखरम्_विस्तारः(गणः->style, &b.विस्तारः, &b.unit);
		b.विस्तारः = css_unit_len2device_px(गणः->style, unit_len_ctx,
				b.विस्तारः, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_सारणी_ROW_गणः;

		यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
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
सारणी_प्रयुक्तम्_वामः_border_for_cell(ध्रुव css_unit_ctx *unit_len_ctx, संरचना box *cell)
{
	संरचना border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_वामः_style(cell->style);
	a.color = css_computed_border_वामः_color(cell->style, &a.c);
	css_computed_border_वामः_विस्तारः(cell->style, &a.विस्तारः, &a.unit);
	a.विस्तारः = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.विस्तारः, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_सारणी_CELL;

	यदि (cell->पूर्व != रिक्त || cell->start_column != 0) {
		/* Cell to the left -- consider its right border */
		संरचना box *पूर्व = रिक्त;

		यदि (cell->पूर्व == रिक्त) {
			संरचना box *row;

			/* Spanned from a previous row in current row group */
			प्रति (row = cell->parent; row != रिक्त; row = row->पूर्व) {
				प्रति (पूर्व = row->children; पूर्व != रिक्त;
				     पूर्व = पूर्व->अग्र) {
					यदि (पूर्व->start_column +
					    पूर्व->columns ==
					    cell->start_column)
						भङ्ग;
				}

				यदि (पूर्व != रिक्त)
					भङ्ग;
			}

			प्रतिज्ञा(पूर्व != रिक्त);
		} अन्यथा {
			पूर्व = cell->पूर्व;
		}

		b.style = css_computed_border_दक्षिणः_style(पूर्व->style);
		b.color = css_computed_border_दक्षिणः_color(पूर्व->style, &b.c);
		css_computed_border_दक्षिणः_विस्तारः(पूर्व->style, &b.विस्तारः, &b.unit);
		b.विस्तारः = css_unit_len2device_px(पूर्व->style, unit_len_ctx,
				b.विस्तारः, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_सारणी_CELL;

		यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} अन्यथा {
		/* First cell in row, so consider rows and row group */
		संरचना box *row = cell->parent;
		संरचना box *गणः = row->parent;
		संरचना box *सारणी = गणः->parent;
		अचिह्नित पूर्णाङ्क पङ्क्तिकाः = cell->पङ्क्तिकाः;

		यावत् (पङ्क्तिकाः-- > 0 && row != रिक्त) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_वामः_style(row->style);
			b.color = css_computed_border_वामः_color(
								 row->style, &b.c);
			css_computed_border_वामः_विस्तारः(
						       row->style, &b.विस्तारः, &b.unit);
			b.विस्तारः = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.विस्तारः, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_सारणी_ROW;

			यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->अग्र;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_वामः_style(गणः->style);
		b.color = css_computed_border_वामः_color(गणः->style, &b.c);
		css_computed_border_वामः_विस्तारः(गणः->style, &b.विस्तारः, &b.unit);
		b.विस्तारः = css_unit_len2device_px(गणः->style, unit_len_ctx,
				b.विस्तारः, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_सारणी_ROW_गणः;

		यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_वामः_style(सारणी->style);
		b.color = css_computed_border_वामः_color(सारणी->style, &b.c);
		css_computed_border_वामः_विस्तारः(सारणी->style, &b.विस्तारः, &b.unit);
		b.विस्तारः = css_unit_len2device_px(सारणी->style, unit_len_ctx,
				b.विस्तारः, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_सारणी;

		यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[वामः_2].style = a.style;
	cell->border[वामः_2].c = a.c;
	cell->border[वामः_2].विस्तारः = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.विस्तारः, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
स्थिर शून्य
सारणी_प्रयुक्तम्_शिखरम्_border_for_cell(ध्रुव css_unit_ctx *unit_len_ctx, संरचना box *cell)
{
	संरचना border a, b;
	box_type a_src, b_src;
	संरचना box *row = cell->parent;
	bool process_गणः = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_शिखरम्_style(cell->style);
	css_computed_border_शिखरम्_color(cell->style, &a.c);
	css_computed_border_शिखरम्_विस्तारः(cell->style, &a.विस्तारः, &a.unit);
	a.विस्तारः = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.विस्तारः, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_सारणी_CELL;

	/* Top border of row */
	b.style = css_computed_border_शिखरम्_style(row->style);
	css_computed_border_शिखरम्_color(row->style, &b.c);
	css_computed_border_शिखरम्_विस्तारः(row->style, &b.विस्तारः, &b.unit);
	b.विस्तारः = css_unit_len2device_px(row->style, unit_len_ctx,
			b.विस्तारः, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_सारणी_ROW;

	यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	यदि (row->पूर्व != रिक्त) {
		/* Consider row(s) above */
		यावत् (सारणी_cell_शिखरम्_process_row(unit_len_ctx, cell, row->पूर्व,
						  &a, &a_src) == false) {
			यदि (row->पूर्व->पूर्व == रिक्त) {
				/* Consider row group */
				process_गणः = true;
				भङ्ग;
			} अन्यथा {
				row = row->पूर्व;
			}
		}
	} अन्यथा {
		process_गणः = true;
	}

	यदि (process_गणः) {
		संरचना box *गणः = row->parent;

		/* Top border of row group */
		b.style = css_computed_border_शिखरम्_style(गणः->style);
		b.color = css_computed_border_शिखरम्_color(गणः->style, &b.c);
		css_computed_border_शिखरम्_विस्तारः(गणः->style, &b.विस्तारः, &b.unit);
		b.विस्तारः = css_unit_len2device_px(गणः->style, unit_len_ctx,
				b.विस्तारः, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_सारणी_ROW_गणः;

		यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		यदि (गणः->पूर्व == रिक्त) {
			/* Top border of table */
			सारणी_cell_शिखरम्_process_सारणी(unit_len_ctx,
						     गणः->parent, &a, &a_src);
		} अन्यथा {
			/* Process previous group(s) */
			यावत् (सारणी_cell_शिखरम्_process_गणः(unit_len_ctx,
							    cell, गणः->पूर्व,
							    &a, &a_src) == false) {
				यदि (गणः->पूर्व->पूर्व == रिक्त) {
					/* Top border of table */
					सारणी_cell_शिखरम्_process_सारणी(unit_len_ctx,
								     गणः->parent,
								     &a, &a_src);
					भङ्ग;
				} अन्यथा {
					गणः = गणः->पूर्व;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[शिखरम्_2].style = a.style;
	cell->border[शिखरम्_2].c = a.c;
	cell->border[शिखरम्_2].विस्तारः = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.विस्तारः, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
स्थिर शून्य
सारणी_प्रयुक्तम्_दक्षिणः_border_for_cell(ध्रुव css_unit_ctx *unit_len_ctx, संरचना box *cell)
{
	संरचना border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_दक्षिणः_style(cell->style);
	css_computed_border_दक्षिणः_color(cell->style, &a.c);
	css_computed_border_दक्षिणः_विस्तारः(cell->style, &a.विस्तारः, &a.unit);
	a.विस्तारः = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.विस्तारः, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_सारणी_CELL;

	यदि (cell->अग्र != रिक्त || cell->start_column + cell->columns !=
	    cell->parent->parent->parent->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.विस्तारः = 0;
		a.unit = CSS_UNIT_PX;
	} अन्यथा {
		/* Last cell in row, so consider rows and row group */
		संरचना box *row = cell->parent;
		संरचना box *गणः = row->parent;
		संरचना box *सारणी = गणः->parent;
		अचिह्नित पूर्णाङ्क पङ्क्तिकाः = cell->पङ्क्तिकाः;

		यावत् (पङ्क्तिकाः-- > 0 && row != रिक्त) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_दक्षिणः_style(row->style);
			b.color = css_computed_border_दक्षिणः_color(row->style,
								  &b.c);
			css_computed_border_दक्षिणः_विस्तारः(row->style,
							&b.विस्तारः,
							&b.unit);
			b.विस्तारः = css_unit_len2device_px(
					row->style, unit_len_ctx,
					b.विस्तारः, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_सारणी_ROW;

			यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			row = row->अग्र;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_दक्षिणः_style(गणः->style);
		b.color = css_computed_border_दक्षिणः_color(गणः->style, &b.c);
		css_computed_border_दक्षिणः_विस्तारः(गणः->style,
						&b.विस्तारः, &b.unit);
		b.विस्तारः = css_unit_len2device_px(गणः->style, unit_len_ctx,
				b.विस्तारः, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_सारणी_ROW_गणः;

		यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_दक्षिणः_style(सारणी->style);
		b.color = css_computed_border_दक्षिणः_color(सारणी->style, &b.c);
		css_computed_border_दक्षिणः_विस्तारः(सारणी->style,
						&b.विस्तारः, &b.unit);
		b.विस्तारः = css_unit_len2device_px(सारणी->style, unit_len_ctx,
				b.विस्तारः, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_सारणी;

		यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[दक्षिणः_2].style = a.style;
	cell->border[दक्षिणः_2].c = a.c;
	cell->border[दक्षिणः_2].विस्तारः = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.विस्तारः, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
स्थिर शून्य
सारणी_प्रयुक्तम्_bottom_border_for_cell(ध्रुव css_unit_ctx *unit_len_ctx,
				  संरचना box *cell)
{
	संरचना border a, b;
	box_type a_src, b_src;
	संरचना box *row = cell->parent;
	अचिह्नित पूर्णाङ्क पङ्क्तिकाः = cell->पङ्क्तिकाः;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_विस्तारः(cell->style, &a.विस्तारः, &a.unit);
	a.विस्तारः = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.विस्तारः, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_सारणी_CELL;

	यावत् (पङ्क्तिकाः-- > 0 && row != रिक्त)
		row = row->अग्र;

	/** \todo Can cells span row groups? */

	यदि (row != रिक्त) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.विस्तारः = 0;
		a.unit = CSS_UNIT_PX;
	} अन्यथा {
		/* Cell at bottom of table, so consider row and row group */
		संरचना box *row = cell->parent;
		संरचना box *गणः = row->parent;
		संरचना box *सारणी = गणः->parent;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(row->style);
		b.color = css_computed_border_bottom_color(row->style, &b.c);
		css_computed_border_bottom_विस्तारः(row->style, &b.विस्तारः, &b.unit);
		b.विस्तारः = css_unit_len2device_px(row->style, unit_len_ctx,
				b.विस्तारः, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_सारणी_ROW;

		यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* Row group -- consider its bottom border */
		b.style = css_computed_border_bottom_style(गणः->style);
		b.color = css_computed_border_bottom_color(गणः->style, &b.c);
		css_computed_border_bottom_विस्तारः(गणः->style,
						 &b.विस्तारः, &b.unit);
		b.विस्तारः = css_unit_len2device_px(गणः->style, unit_len_ctx,
				b.विस्तारः, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_सारणी_ROW_गणः;

		यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its bottom border */
		b.style = css_computed_border_bottom_style(सारणी->style);
		b.color = css_computed_border_bottom_color(सारणी->style, &b.c);
		css_computed_border_bottom_विस्तारः(सारणी->style,
						 &b.विस्तारः, &b.unit);
		b.विस्तारः = css_unit_len2device_px(सारणी->style, unit_len_ctx,
				b.विस्तारः, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_सारणी;

		यदि (सारणी_border_is_अधिकम्_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
		}
	}

	/* a now contains the used bottom border for the cell */
	cell->border[BOTTOM].style = a.style;
	cell->border[BOTTOM].c = a.c;
	cell->border[BOTTOM].विस्तारः = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.विस्तारः, a.unit));
}


/* exported interface documented in html/table.h */
bool
सारणी_calculate_column_types(ध्रुव css_unit_ctx *unit_len_ctx, संरचना box *सारणी)
{
	अचिह्नित पूर्णाङ्क i, j;
	संरचना column *col;
	संरचना box *row_गणः, *row, *cell;

	यदि (सारणी->col)
		/* table->col already constructed, for example frameset table */
		प्रत्यागम true;

	सारणी->col = col = talloc_array(सारणी, संरचना column, सारणी->columns);
	यदि (!col)
		प्रत्यागम false;

	प्रति (i = 0; i != सारणी->columns; i++) {
		col[i].type = COLUMN_विस्तारः_UNKNOWN;
		col[i].विस्तारः = 0;
		col[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	प्रति (row_गणः = सारणी->children; row_गणः; row_गणः =row_गणः->अग्र)
		प्रति (row = row_गणः->children; row; row = row->अग्र)
			प्रति (cell = row->children; cell; cell = cell->अग्र) {
				गणना css_विस्तारः_e type;
				css_fixed मानम् = 0;
				css_unit unit = CSS_UNIT_PX;

				प्रतिज्ञा(cell->type == BOX_सारणी_CELL);
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

				type = css_computed_विस्तारः(cell->style, &मानम्, &unit);

				/* fixed width takes priority over any other width type */
				यदि (col[i].type != COLUMN_विस्तारः_FIXED &&
				    type == CSS_विस्तारः_समुच्चयः && unit != CSS_UNIT_PCT) {
					col[i].type = COLUMN_विस्तारः_FIXED;
					col[i].विस्तारः = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							मानम्, unit));
					यदि (col[i].विस्तारः < 0)
						col[i].विस्तारः = 0;
					अनुवर्त;
				}

				यदि (col[i].type != COLUMN_विस्तारः_UNKNOWN)
					अनुवर्त;

				यदि (type == CSS_विस्तारः_समुच्चयः && unit == CSS_UNIT_PCT) {
					col[i].type = COLUMN_विस्तारः_PERCENT;
					col[i].विस्तारः = FIXTOINT(मानम्);
					यदि (col[i].विस्तारः < 0)
						col[i].विस्तारः = 0;
				} अन्यथा यदि (type == CSS_विस्तारः_AUTO) {
					col[i].type = COLUMN_विस्तारः_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	प्रति (row_गणः = सारणी->children; row_गणः; row_गणः =row_गणः->अग्र)
		प्रति (row = row_गणः->children; row; row = row->अग्र)
			प्रति (cell = row->children; cell; cell = cell->अग्र) {
				अचिह्नित पूर्णाङ्क fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				पूर्णाङ्क fixed_विस्तारः = 0, percent_विस्तारः = 0;
				गणना css_विस्तारः_e type;
				css_fixed मानम् = 0;
				css_unit unit = CSS_UNIT_PX;

				यदि (cell->columns == 1)
					अनुवर्त;
				i = cell->start_column;

				प्रति (j = i; j < i + cell->columns; j++) {
					col[j].positioned = false;
				}

				/* count column types in spanned cells */
				प्रति (j = 0; j != cell->columns; j++) {
					यदि (col[i + j].type == COLUMN_विस्तारः_FIXED) {
						fixed_विस्तारः += col[i + j].विस्तारः;
						fixed_columns++;
					} अन्यथा यदि (col[i + j].type == COLUMN_विस्तारः_PERCENT) {
						percent_विस्तारः += col[i + j].विस्तारः;
						percent_columns++;
					} अन्यथा यदि (col[i + j].type == COLUMN_विस्तारः_AUTO) {
						auto_columns++;
					} अन्यथा {
						unknown_columns++;
					}
				}

				यदि (!unknown_columns)
					अनुवर्त;

				type = css_computed_विस्तारः(cell->style, &मानम्, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				यदि (type == CSS_विस्तारः_समुच्चयः && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					पूर्णाङ्क विस्तारः = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							मानम्, unit)) -
						fixed_विस्तारः) / unknown_columns;
					यदि (विस्तारः < 0)
						विस्तारः = 0;
					प्रति (j = 0; j != cell->columns; j++) {
						यदि (col[i + j].type == COLUMN_विस्तारः_UNKNOWN) {
							col[i + j].type = COLUMN_विस्तारः_FIXED;
							col[i + j].विस्तारः = विस्तारः;
						}
					}
				}

				/* as above for percentage width */
				यदि (type == CSS_विस्तारः_समुच्चयः && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					पूर्णाङ्क विस्तारः = (FIXTOFLT(मानम्) -
						     percent_विस्तारः) / unknown_columns;
					यदि (विस्तारः < 0)
						विस्तारः = 0;
					प्रति (j = 0; j != cell->columns; j++) {
						यदि (col[i + j].type == COLUMN_विस्तारः_UNKNOWN) {
							col[i + j].type = COLUMN_विस्तारः_PERCENT;
							col[i + j].विस्तारः = विस्तारः;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	प्रति (i = 0; i != सारणी->columns; i++) {
		यदि (col[i].type == COLUMN_विस्तारः_UNKNOWN)
			col[i].type = COLUMN_विस्तारः_AUTO;
	}

#यदि_परिभाषितम् सारणी_DEBUG
	प्रति (i = 0; i != सारणी->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      सारणी,
		      i,
		      ((ध्रुव वर्ण *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[col[i].type],
		      col[i].विस्तारः);
#यदि_अन्तः

	प्रत्यागम true;
}


/* exported interface documented in html/table.h */
शून्य सारणी_प्रयुक्तम्_border_for_cell(ध्रुव css_unit_ctx *unit_len_ctx, संरचना box *cell)
{
	पूर्णाङ्क side;

	प्रतिज्ञा(cell->type == BOX_सारणी_CELL);

	यदि (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed विस्तारः = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[वामः_2].style =
			css_computed_border_वामः_style(cell->style);
		css_computed_border_वामः_color(cell->style,
					       &cell->border[वामः_2].c);
		css_computed_border_वामः_विस्तारः(cell->style, &विस्तारः, &unit);
		cell->border[वामः_2].विस्तारः =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					विस्तारः, unit));

		/* Top border */
		cell->border[शिखरम्_2].style =
			css_computed_border_शिखरम्_style(cell->style);
		css_computed_border_शिखरम्_color(cell->style,
					      &cell->border[शिखरम्_2].c);
		css_computed_border_शिखरम्_विस्तारः(cell->style, &विस्तारः, &unit);
		cell->border[शिखरम्_2].विस्तारः =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					विस्तारः, unit));

		/* Right border */
		cell->border[दक्षिणः_2].style =
			css_computed_border_दक्षिणः_style(cell->style);
		css_computed_border_दक्षिणः_color(cell->style,
						&cell->border[दक्षिणः_2].c);
		css_computed_border_दक्षिणः_विस्तारः(cell->style, &विस्तारः, &unit);
		cell->border[दक्षिणः_2].विस्तारः =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					विस्तारः, unit));

		/* Bottom border */
		cell->border[BOTTOM].style =
			css_computed_border_bottom_style(cell->style);
		css_computed_border_bottom_color(cell->style,
						 &cell->border[BOTTOM].c);
		css_computed_border_bottom_विस्तारः(cell->style, &विस्तारः, &unit);
		cell->border[BOTTOM].विस्तारः =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					विस्तारः, unit));
	} अन्यथा {
		/* Left border */
		सारणी_प्रयुक्तम्_वामः_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		सारणी_प्रयुक्तम्_शिखरम्_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		सारणी_प्रयुक्तम्_दक्षिणः_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		सारणी_प्रयुक्तम्_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	प्रति (side = 0; side != 4; side++) {
		यदि (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].विस्तारः = 0;
	}
}
