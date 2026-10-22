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
#تعریف_لغوه جدول_DEBUG

/**
 * Container for border values during table border calculations
 */
بنية border {
	تعداد css_border_style_e style;	/**< border-style */
	تعداد css_border_color_e color;	/**< border-color type */
	css_color c;			/**< border-color value */
	css_fixed پلنوالی;		/**< border-width length */
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
جدول_border_is_نور_eyecatching(ثابت css_unit_ctx *unit_len_ctx,
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
	awidth = css_unit_len2device_px(عدم, unit_len_ctx, a->پلنوالی, a->unit);
	bwidth = css_unit_len2device_px(عدم, unit_len_ctx, b->پلنوالی, b->unit);

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
	حالة BOX_جدول_CELL:       impact++; fallthrough;
	حالة BOX_جدول_لیکه:        impact++; fallthrough;
	حالة BOX_جدول_لیکه_ډله:  impact++; fallthrough;
	حالة BOX_جدول:            impact++; fallthrough;
	افتراضي:
		اكسر;
	}

	/** \todo COL/COL_GROUP */
	بدل (b_src) {
	حالة BOX_جدول_CELL:       impact--; fallthrough;
	حالة BOX_جدول_لیکه:        impact--; fallthrough;
	حالة BOX_جدول_لیکه_ډله:  impact--; fallthrough;
	حالة BOX_جدول:            impact--; fallthrough;
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
جدول_cell_سر_process_جدول(ثابت css_unit_ctx *unit_len_ctx,
			     بنية box *جدول,
			     بنية border *a,
			     box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_سر_style(جدول->style);
	b.color = css_computed_border_سر_color(جدول->style, &b.c);
	css_computed_border_سر_پلنوالی(جدول->style, &b.پلنوالی, &b.unit);
	b.پلنوالی = css_unit_len2device_px(جدول->style, unit_len_ctx,
			b.پلنوالی, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول;

	إذا (جدول_border_is_نور_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
جدول_cell_سر_process_لیکه(ثابت css_unit_ctx *unit_len_ctx,
			   بنية box *cell,
			   بنية box *لیکه,
			   بنية border *a,
			   box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(لیکه->style);
	b.color = css_computed_border_bottom_color(لیکه->style, &b.c);
	css_computed_border_bottom_پلنوالی(لیکه->style, &b.پلنوالی, &b.unit);
	b.پلنوالی = css_unit_len2device_px(لیکه->style, unit_len_ctx,
			b.پلنوالی, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_لیکه;

	إذا (جدول_border_is_نور_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	إذا (لیکه->children == عدم) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_سر_style(لیکه->style);
		b.color = css_computed_border_سر_color(لیکه->style, &b.c);
		css_computed_border_سر_پلنوالی(لیکه->style, &b.پلنوالی, &b.unit);
		b.پلنوالی = css_unit_len2device_px(لیکه->style, unit_len_ctx,
				b.پلنوالی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_لیکه;

		إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
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
			لكل (c = لیکه->children; c != عدم; c = c->بل) {
				/* Ignore cells to the left */
				إذا (c->پیل_ستنه + c->columns - 1 <
				    cell->پیل_ستنه)
					استمر;
				/* Ignore cells to the right */
				إذا (c->پیل_ستنه > cell->پیل_ستنه +
				    cell->columns - 1)
					استمر;

				/* Flag that we've processed a cell */
				processed = true;

				/* Consider bottom border */
				b.style = css_computed_border_bottom_style(
							c->style);
				b.color = css_computed_border_bottom_color(
							c->style, &b.c);
				css_computed_border_bottom_پلنوالی(c->style,
							&b.پلنوالی, &b.unit);
				b.پلنوالی = css_unit_len2device_px(
						c->style, unit_len_ctx,
						b.پلنوالی, b.unit);
				b.unit = CSS_UNIT_PX;
				b_src = BOX_جدول_CELL;

				إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
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
				أكد(لیکه->prev != عدم);

				لیکه = لیکه->prev;
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
جدول_cell_سر_process_ډله(ثابت css_unit_ctx *unit_len_ctx,
			     بنية box *cell,
			     بنية box *ډله,
			     بنية border *a,
			     box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(ډله->style);
	b.color = css_computed_border_bottom_color(ډله->style, &b.c);
	css_computed_border_bottom_پلنوالی(ډله->style, &b.پلنوالی, &b.unit);
	b.پلنوالی = css_unit_len2device_px(ډله->style, unit_len_ctx,
			b.پلنوالی, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_لیکه_ډله;

	إذا (جدول_border_is_نور_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	إذا (ډله->وروستی != عدم) {
		/* Process rows in group, starting with last */
		بنية box *لیکه = ډله->وروستی;

		طالما (جدول_cell_سر_process_لیکه(unit_len_ctx, cell, لیکه,
						  a, a_src) == false) {
			إذا (لیکه->prev == عدم) {
				أرجع false;
			} وإلا {
				لیکه = لیکه->prev;
			}
		}
	} وإلا {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_سر_style(ډله->style);
		b.color = css_computed_border_سر_color(ډله->style, &b.c);
		css_computed_border_سر_پلنوالی(ډله->style, &b.پلنوالی, &b.unit);
		b.پلنوالی = css_unit_len2device_px(ډله->style, unit_len_ctx,
				b.پلنوالی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_لیکه_ډله;

		إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
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
جدول_کارول_شوی_کیڼ_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_کیڼ_style(cell->style);
	a.color = css_computed_border_کیڼ_color(cell->style, &a.c);
	css_computed_border_کیڼ_پلنوالی(cell->style, &a.پلنوالی, &a.unit);
	a.پلنوالی = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.پلنوالی, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	إذا (cell->prev != عدم || cell->پیل_ستنه != 0) {
		/* Cell to the left -- consider its right border */
		بنية box *prev = عدم;

		إذا (cell->prev == عدم) {
			بنية box *لیکه;

			/* Spanned from a previous row in current row group */
			لكل (لیکه = cell->پلار; لیکه != عدم; لیکه = لیکه->prev) {
				لكل (prev = لیکه->children; prev != عدم;
				     prev = prev->بل) {
					إذا (prev->پیل_ستنه +
					    prev->columns ==
					    cell->پیل_ستنه)
						اكسر;
				}

				إذا (prev != عدم)
					اكسر;
			}

			أكد(prev != عدم);
		} وإلا {
			prev = cell->prev;
		}

		b.style = css_computed_border_ښی_style(prev->style);
		b.color = css_computed_border_ښی_color(prev->style, &b.c);
		css_computed_border_ښی_پلنوالی(prev->style, &b.پلنوالی, &b.unit);
		b.پلنوالی = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.پلنوالی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_CELL;

		إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} وإلا {
		/* First cell in row, so consider rows and row group */
		بنية box *لیکه = cell->پلار;
		بنية box *ډله = لیکه->پلار;
		بنية box *جدول = ډله->پلار;
		بینښه صحيح لیکې = cell->لیکې;

		طالما (لیکې-- > 0 && لیکه != عدم) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_کیڼ_style(لیکه->style);
			b.color = css_computed_border_کیڼ_color(
								 لیکه->style, &b.c);
			css_computed_border_کیڼ_پلنوالی(
						       لیکه->style, &b.پلنوالی, &b.unit);
			b.پلنوالی = css_unit_len2device_px(
					لیکه->style, unit_len_ctx,
					b.پلنوالی, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_جدول_لیکه;

			إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			لیکه = لیکه->بل;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_کیڼ_style(ډله->style);
		b.color = css_computed_border_کیڼ_color(ډله->style, &b.c);
		css_computed_border_کیڼ_پلنوالی(ډله->style, &b.پلنوالی, &b.unit);
		b.پلنوالی = css_unit_len2device_px(ډله->style, unit_len_ctx,
				b.پلنوالی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_لیکه_ډله;

		إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_کیڼ_style(جدول->style);
		b.color = css_computed_border_کیڼ_color(جدول->style, &b.c);
		css_computed_border_کیڼ_پلنوالی(جدول->style, &b.پلنوالی, &b.unit);
		b.پلنوالی = css_unit_len2device_px(جدول->style, unit_len_ctx,
				b.پلنوالی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول;

		إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[کیڼ_2].style = a.style;
	cell->border[کیڼ_2].c = a.c;
	cell->border[کیڼ_2].پلنوالی = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.پلنوالی, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
جدول_کارول_شوی_سر_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;
	بنية box *لیکه = cell->پلار;
	bool process_ډله = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_سر_style(cell->style);
	css_computed_border_سر_color(cell->style, &a.c);
	css_computed_border_سر_پلنوالی(cell->style, &a.پلنوالی, &a.unit);
	a.پلنوالی = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.پلنوالی, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	/* Top border of row */
	b.style = css_computed_border_سر_style(لیکه->style);
	css_computed_border_سر_color(لیکه->style, &b.c);
	css_computed_border_سر_پلنوالی(لیکه->style, &b.پلنوالی, &b.unit);
	b.پلنوالی = css_unit_len2device_px(لیکه->style, unit_len_ctx,
			b.پلنوالی, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_لیکه;

	إذا (جدول_border_is_نور_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	إذا (لیکه->prev != عدم) {
		/* Consider row(s) above */
		طالما (جدول_cell_سر_process_لیکه(unit_len_ctx, cell, لیکه->prev,
						  &a, &a_src) == false) {
			إذا (لیکه->prev->prev == عدم) {
				/* Consider row group */
				process_ډله = true;
				اكسر;
			} وإلا {
				لیکه = لیکه->prev;
			}
		}
	} وإلا {
		process_ډله = true;
	}

	إذا (process_ډله) {
		بنية box *ډله = لیکه->پلار;

		/* Top border of row group */
		b.style = css_computed_border_سر_style(ډله->style);
		b.color = css_computed_border_سر_color(ډله->style, &b.c);
		css_computed_border_سر_پلنوالی(ډله->style, &b.پلنوالی, &b.unit);
		b.پلنوالی = css_unit_len2device_px(ډله->style, unit_len_ctx,
				b.پلنوالی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_لیکه_ډله;

		إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		إذا (ډله->prev == عدم) {
			/* Top border of table */
			جدول_cell_سر_process_جدول(unit_len_ctx,
						     ډله->پلار, &a, &a_src);
		} وإلا {
			/* Process previous group(s) */
			طالما (جدول_cell_سر_process_ډله(unit_len_ctx,
							    cell, ډله->prev,
							    &a, &a_src) == false) {
				إذا (ډله->prev->prev == عدم) {
					/* Top border of table */
					جدول_cell_سر_process_جدول(unit_len_ctx,
								     ډله->پلار,
								     &a, &a_src);
					اكسر;
				} وإلا {
					ډله = ډله->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[سر_2].style = a.style;
	cell->border[سر_2].c = a.c;
	cell->border[سر_2].پلنوالی = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.پلنوالی, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
جدول_کارول_شوی_ښی_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_ښی_style(cell->style);
	css_computed_border_ښی_color(cell->style, &a.c);
	css_computed_border_ښی_پلنوالی(cell->style, &a.پلنوالی, &a.unit);
	a.پلنوالی = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.پلنوالی, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	إذا (cell->بل != عدم || cell->پیل_ستنه + cell->columns !=
	    cell->پلار->پلار->پلار->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.پلنوالی = 0;
		a.unit = CSS_UNIT_PX;
	} وإلا {
		/* Last cell in row, so consider rows and row group */
		بنية box *لیکه = cell->پلار;
		بنية box *ډله = لیکه->پلار;
		بنية box *جدول = ډله->پلار;
		بینښه صحيح لیکې = cell->لیکې;

		طالما (لیکې-- > 0 && لیکه != عدم) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_ښی_style(لیکه->style);
			b.color = css_computed_border_ښی_color(لیکه->style,
								  &b.c);
			css_computed_border_ښی_پلنوالی(لیکه->style,
							&b.پلنوالی,
							&b.unit);
			b.پلنوالی = css_unit_len2device_px(
					لیکه->style, unit_len_ctx,
					b.پلنوالی, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_جدول_لیکه;

			إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			لیکه = لیکه->بل;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_ښی_style(ډله->style);
		b.color = css_computed_border_ښی_color(ډله->style, &b.c);
		css_computed_border_ښی_پلنوالی(ډله->style,
						&b.پلنوالی, &b.unit);
		b.پلنوالی = css_unit_len2device_px(ډله->style, unit_len_ctx,
				b.پلنوالی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_لیکه_ډله;

		إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_ښی_style(جدول->style);
		b.color = css_computed_border_ښی_color(جدول->style, &b.c);
		css_computed_border_ښی_پلنوالی(جدول->style,
						&b.پلنوالی, &b.unit);
		b.پلنوالی = css_unit_len2device_px(جدول->style, unit_len_ctx,
				b.پلنوالی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول;

		إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[ښی_2].style = a.style;
	cell->border[ښی_2].c = a.c;
	cell->border[ښی_2].پلنوالی = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.پلنوالی, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
جدول_کارول_شوی_bottom_border_for_cell(ثابت css_unit_ctx *unit_len_ctx,
				  بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;
	بنية box *لیکه = cell->پلار;
	بینښه صحيح لیکې = cell->لیکې;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_پلنوالی(cell->style, &a.پلنوالی, &a.unit);
	a.پلنوالی = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.پلنوالی, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	طالما (لیکې-- > 0 && لیکه != عدم)
		لیکه = لیکه->بل;

	/** \todo Can cells span row groups? */

	إذا (لیکه != عدم) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.پلنوالی = 0;
		a.unit = CSS_UNIT_PX;
	} وإلا {
		/* Cell at bottom of table, so consider row and row group */
		بنية box *لیکه = cell->پلار;
		بنية box *ډله = لیکه->پلار;
		بنية box *جدول = ډله->پلار;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(لیکه->style);
		b.color = css_computed_border_bottom_color(لیکه->style, &b.c);
		css_computed_border_bottom_پلنوالی(لیکه->style, &b.پلنوالی, &b.unit);
		b.پلنوالی = css_unit_len2device_px(لیکه->style, unit_len_ctx,
				b.پلنوالی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_لیکه;

		إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* Row group -- consider its bottom border */
		b.style = css_computed_border_bottom_style(ډله->style);
		b.color = css_computed_border_bottom_color(ډله->style, &b.c);
		css_computed_border_bottom_پلنوالی(ډله->style,
						 &b.پلنوالی, &b.unit);
		b.پلنوالی = css_unit_len2device_px(ډله->style, unit_len_ctx,
				b.پلنوالی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_لیکه_ډله;

		إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its bottom border */
		b.style = css_computed_border_bottom_style(جدول->style);
		b.color = css_computed_border_bottom_color(جدول->style, &b.c);
		css_computed_border_bottom_پلنوالی(جدول->style,
						 &b.پلنوالی, &b.unit);
		b.پلنوالی = css_unit_len2device_px(جدول->style, unit_len_ctx,
				b.پلنوالی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول;

		إذا (جدول_border_is_نور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
		}
	}

	/* a now contains the used bottom border for the cell */
	cell->border[BOTTOM].style = a.style;
	cell->border[BOTTOM].c = a.c;
	cell->border[BOTTOM].پلنوالی = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.پلنوالی, a.unit));
}


/* exported interface documented in html/table.h */
bool
جدول_calculate_ستنه_types(ثابت css_unit_ctx *unit_len_ctx, بنية box *جدول)
{
	بینښه صحيح i, j;
	بنية ستنه *ستون;
	بنية box *لیکه_ډله, *لیکه, *cell;

	إذا (جدول->ستون)
		/* table->col already constructed, for example frameset table */
		أرجع true;

	جدول->ستون = ستون = talloc_array(جدول, بنية ستنه, جدول->columns);
	إذا (!ستون)
		أرجع false;

	لكل (i = 0; i != جدول->columns; i++) {
		ستون[i].type = ستنه_پلنوالی_UNKNOWN;
		ستون[i].پلنوالی = 0;
		ستون[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	لكل (لیکه_ډله = جدول->children; لیکه_ډله; لیکه_ډله =لیکه_ډله->بل)
		لكل (لیکه = لیکه_ډله->children; لیکه; لیکه = لیکه->بل)
			لكل (cell = لیکه->children; cell; cell = cell->بل) {
				تعداد css_پلنوالی_e type;
				css_fixed ارزښت = 0;
				css_unit unit = CSS_UNIT_PX;

				أكد(cell->type == BOX_جدول_CELL);
				أكد(cell->style);

				إذا (cell->columns != 1)
					استمر;
				i = cell->پیل_ستنه;

				إذا (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					ستون[i].positioned = false;
				}

				type = css_computed_پلنوالی(cell->style, &ارزښت, &unit);

				/* fixed width takes priority over any other width type */
				إذا (ستون[i].type != ستنه_پلنوالی_FIXED &&
				    type == CSS_پلنوالی_سېټ && unit != CSS_UNIT_PCT) {
					ستون[i].type = ستنه_پلنوالی_FIXED;
					ستون[i].پلنوالی = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							ارزښت, unit));
					إذا (ستون[i].پلنوالی < 0)
						ستون[i].پلنوالی = 0;
					استمر;
				}

				إذا (ستون[i].type != ستنه_پلنوالی_UNKNOWN)
					استمر;

				إذا (type == CSS_پلنوالی_سېټ && unit == CSS_UNIT_PCT) {
					ستون[i].type = ستنه_پلنوالی_PERCENT;
					ستون[i].پلنوالی = FIXTOINT(ارزښت);
					إذا (ستون[i].پلنوالی < 0)
						ستون[i].پلنوالی = 0;
				} وإلا إذا (type == CSS_پلنوالی_AUTO) {
					ستون[i].type = ستنه_پلنوالی_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	لكل (لیکه_ډله = جدول->children; لیکه_ډله; لیکه_ډله =لیکه_ډله->بل)
		لكل (لیکه = لیکه_ډله->children; لیکه; لیکه = لیکه->بل)
			لكل (cell = لیکه->children; cell; cell = cell->بل) {
				بینښه صحيح fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				صحيح fixed_پلنوالی = 0, percent_پلنوالی = 0;
				تعداد css_پلنوالی_e type;
				css_fixed ارزښت = 0;
				css_unit unit = CSS_UNIT_PX;

				إذا (cell->columns == 1)
					استمر;
				i = cell->پیل_ستنه;

				لكل (j = i; j < i + cell->columns; j++) {
					ستون[j].positioned = false;
				}

				/* count column types in spanned cells */
				لكل (j = 0; j != cell->columns; j++) {
					إذا (ستون[i + j].type == ستنه_پلنوالی_FIXED) {
						fixed_پلنوالی += ستون[i + j].پلنوالی;
						fixed_columns++;
					} وإلا إذا (ستون[i + j].type == ستنه_پلنوالی_PERCENT) {
						percent_پلنوالی += ستون[i + j].پلنوالی;
						percent_columns++;
					} وإلا إذا (ستون[i + j].type == ستنه_پلنوالی_AUTO) {
						auto_columns++;
					} وإلا {
						unknown_columns++;
					}
				}

				إذا (!unknown_columns)
					استمر;

				type = css_computed_پلنوالی(cell->style, &ارزښت, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				إذا (type == CSS_پلنوالی_سېټ && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					صحيح پلنوالی = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							ارزښت, unit)) -
						fixed_پلنوالی) / unknown_columns;
					إذا (پلنوالی < 0)
						پلنوالی = 0;
					لكل (j = 0; j != cell->columns; j++) {
						إذا (ستون[i + j].type == ستنه_پلنوالی_UNKNOWN) {
							ستون[i + j].type = ستنه_پلنوالی_FIXED;
							ستون[i + j].پلنوالی = پلنوالی;
						}
					}
				}

				/* as above for percentage width */
				إذا (type == CSS_پلنوالی_سېټ && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					صحيح پلنوالی = (FIXTOFLT(ارزښت) -
						     percent_پلنوالی) / unknown_columns;
					إذا (پلنوالی < 0)
						پلنوالی = 0;
					لكل (j = 0; j != cell->columns; j++) {
						إذا (ستون[i + j].type == ستنه_پلنوالی_UNKNOWN) {
							ستون[i + j].type = ستنه_پلنوالی_PERCENT;
							ستون[i + j].پلنوالی = پلنوالی;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	لكل (i = 0; i != جدول->columns; i++) {
		إذا (ستون[i].type == ستنه_پلنوالی_UNKNOWN)
			ستون[i].type = ستنه_پلنوالی_AUTO;
	}

#که_تعریف_شوی جدول_DEBUG
	لكل (i = 0; i != جدول->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      جدول,
		      i,
		      ((ثابت حرف *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[ستون[i].type],
		      ستون[i].پلنوالی);
#د_که_پای

	أرجع true;
}


/* exported interface documented in html/table.h */
فراغ جدول_کارول_شوی_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	صحيح side;

	أكد(cell->type == BOX_جدول_CELL);

	إذا (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed پلنوالی = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[کیڼ_2].style =
			css_computed_border_کیڼ_style(cell->style);
		css_computed_border_کیڼ_color(cell->style,
					       &cell->border[کیڼ_2].c);
		css_computed_border_کیڼ_پلنوالی(cell->style, &پلنوالی, &unit);
		cell->border[کیڼ_2].پلنوالی =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					پلنوالی, unit));

		/* Top border */
		cell->border[سر_2].style =
			css_computed_border_سر_style(cell->style);
		css_computed_border_سر_color(cell->style,
					      &cell->border[سر_2].c);
		css_computed_border_سر_پلنوالی(cell->style, &پلنوالی, &unit);
		cell->border[سر_2].پلنوالی =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					پلنوالی, unit));

		/* Right border */
		cell->border[ښی_2].style =
			css_computed_border_ښی_style(cell->style);
		css_computed_border_ښی_color(cell->style,
						&cell->border[ښی_2].c);
		css_computed_border_ښی_پلنوالی(cell->style, &پلنوالی, &unit);
		cell->border[ښی_2].پلنوالی =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					پلنوالی, unit));

		/* Bottom border */
		cell->border[BOTTOM].style =
			css_computed_border_bottom_style(cell->style);
		css_computed_border_bottom_color(cell->style,
						 &cell->border[BOTTOM].c);
		css_computed_border_bottom_پلنوالی(cell->style, &پلنوالی, &unit);
		cell->border[BOTTOM].پلنوالی =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					پلنوالی, unit));
	} وإلا {
		/* Left border */
		جدول_کارول_شوی_کیڼ_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		جدول_کارول_شوی_سر_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		جدول_کارول_شوی_ښی_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		جدول_کارول_شوی_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	لكل (side = 0; side != 4; side++) {
		إذا (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].پلنوالی = 0;
	}
}
