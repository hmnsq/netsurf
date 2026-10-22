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
#تعریف_رد جدول_DEBUG

/**
 * Container for border values during table border calculations
 */
بنية border {
	تعداد css_border_style_e style;	/**< border-style */
	تعداد css_border_color_e color;	/**< border-color type */
	css_color c;			/**< border-color value */
	css_fixed چوڑائی;		/**< border-width length */
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
جدول_border_is_ہور_eyecatching(ثابت css_unit_ctx *unit_len_ctx,
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
	awidth = css_unit_len2device_px(عدم, unit_len_ctx, a->چوڑائی, a->unit);
	bwidth = css_unit_len2device_px(عدم, unit_len_ctx, b->چوڑائی, b->unit);

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
	حالة BOX_جدول_صف:        impact++; fallthrough;
	حالة BOX_جدول_صف_گروہ:  impact++; fallthrough;
	حالة BOX_جدول:            impact++; fallthrough;
	افتراضي:
		اكسر;
	}

	/** \todo COL/COL_GROUP */
	بدل (b_src) {
	حالة BOX_جدول_CELL:       impact--; fallthrough;
	حالة BOX_جدول_صف:        impact--; fallthrough;
	حالة BOX_جدول_صف_گروہ:  impact--; fallthrough;
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
جدول_cell_چوٹی_process_جدول(ثابت css_unit_ctx *unit_len_ctx,
			     بنية box *جدول,
			     بنية border *a,
			     box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_چوٹی_style(جدول->style);
	b.color = css_computed_border_چوٹی_color(جدول->style, &b.c);
	css_computed_border_چوٹی_چوڑائی(جدول->style, &b.چوڑائی, &b.unit);
	b.چوڑائی = css_unit_len2device_px(جدول->style, unit_len_ctx,
			b.چوڑائی, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول;

	إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
جدول_cell_چوٹی_process_صف(ثابت css_unit_ctx *unit_len_ctx,
			   بنية box *cell,
			   بنية box *صف,
			   بنية border *a,
			   box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(صف->style);
	b.color = css_computed_border_bottom_color(صف->style, &b.c);
	css_computed_border_bottom_چوڑائی(صف->style, &b.چوڑائی, &b.unit);
	b.چوڑائی = css_unit_len2device_px(صف->style, unit_len_ctx,
			b.چوڑائی, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_صف;

	إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	إذا (صف->children == عدم) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_چوٹی_style(صف->style);
		b.color = css_computed_border_چوٹی_color(صف->style, &b.c);
		css_computed_border_چوٹی_چوڑائی(صف->style, &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(صف->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف;

		إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
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
			لكل (c = صف->children; c != عدم; c = c->اگلا) {
				/* Ignore cells to the left */
				إذا (c->شروع_کالم + c->columns - 1 <
				    cell->شروع_کالم)
					استمر;
				/* Ignore cells to the right */
				إذا (c->شروع_کالم > cell->شروع_کالم +
				    cell->columns - 1)
					استمر;

				/* Flag that we've processed a cell */
				processed = true;

				/* Consider bottom border */
				b.style = css_computed_border_bottom_style(
							c->style);
				b.color = css_computed_border_bottom_color(
							c->style, &b.c);
				css_computed_border_bottom_چوڑائی(c->style,
							&b.چوڑائی, &b.unit);
				b.چوڑائی = css_unit_len2device_px(
						c->style, unit_len_ctx,
						b.چوڑائی, b.unit);
				b.unit = CSS_UNIT_PX;
				b_src = BOX_جدول_CELL;

				إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
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
				أكد(صف->prev != عدم);

				صف = صف->prev;
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
جدول_cell_چوٹی_process_گروہ(ثابت css_unit_ctx *unit_len_ctx,
			     بنية box *cell,
			     بنية box *گروہ,
			     بنية border *a,
			     box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(گروہ->style);
	b.color = css_computed_border_bottom_color(گروہ->style, &b.c);
	css_computed_border_bottom_چوڑائی(گروہ->style, &b.چوڑائی, &b.unit);
	b.چوڑائی = css_unit_len2device_px(گروہ->style, unit_len_ctx,
			b.چوڑائی, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_صف_گروہ;

	إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	إذا (گروہ->آخری != عدم) {
		/* Process rows in group, starting with last */
		بنية box *صف = گروہ->آخری;

		طالما (جدول_cell_چوٹی_process_صف(unit_len_ctx, cell, صف,
						  a, a_src) == false) {
			إذا (صف->prev == عدم) {
				أرجع false;
			} وإلا {
				صف = صف->prev;
			}
		}
	} وإلا {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_چوٹی_style(گروہ->style);
		b.color = css_computed_border_چوٹی_color(گروہ->style, &b.c);
		css_computed_border_چوٹی_چوڑائی(گروہ->style, &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(گروہ->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف_گروہ;

		إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
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
جدول_ورتیا_بایاں_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_بایاں_style(cell->style);
	a.color = css_computed_border_بایاں_color(cell->style, &a.c);
	css_computed_border_بایاں_چوڑائی(cell->style, &a.چوڑائی, &a.unit);
	a.چوڑائی = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.چوڑائی, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	إذا (cell->prev != عدم || cell->شروع_کالم != 0) {
		/* Cell to the left -- consider its right border */
		بنية box *prev = عدم;

		إذا (cell->prev == عدم) {
			بنية box *صف;

			/* Spanned from a previous row in current row group */
			لكل (صف = cell->ماپا; صف != عدم; صف = صف->prev) {
				لكل (prev = صف->children; prev != عدم;
				     prev = prev->اگلا) {
					إذا (prev->شروع_کالم +
					    prev->columns ==
					    cell->شروع_کالم)
						اكسر;
				}

				إذا (prev != عدم)
					اكسر;
			}

			أكد(prev != عدم);
		} وإلا {
			prev = cell->prev;
		}

		b.style = css_computed_border_دایاں_style(prev->style);
		b.color = css_computed_border_دایاں_color(prev->style, &b.c);
		css_computed_border_دایاں_چوڑائی(prev->style, &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_CELL;

		إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} وإلا {
		/* First cell in row, so consider rows and row group */
		بنية box *صف = cell->ماپا;
		بنية box *گروہ = صف->ماپا;
		بنية box *جدول = گروہ->ماپا;
		بنانشانی صحيح صفاں = cell->صفاں;

		طالما (صفاں-- > 0 && صف != عدم) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_بایاں_style(صف->style);
			b.color = css_computed_border_بایاں_color(
								 صف->style, &b.c);
			css_computed_border_بایاں_چوڑائی(
						       صف->style, &b.چوڑائی, &b.unit);
			b.چوڑائی = css_unit_len2device_px(
					صف->style, unit_len_ctx,
					b.چوڑائی, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_جدول_صف;

			إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			صف = صف->اگلا;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_بایاں_style(گروہ->style);
		b.color = css_computed_border_بایاں_color(گروہ->style, &b.c);
		css_computed_border_بایاں_چوڑائی(گروہ->style, &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(گروہ->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف_گروہ;

		إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_بایاں_style(جدول->style);
		b.color = css_computed_border_بایاں_color(جدول->style, &b.c);
		css_computed_border_بایاں_چوڑائی(جدول->style, &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(جدول->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول;

		إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[بایاں_2].style = a.style;
	cell->border[بایاں_2].c = a.c;
	cell->border[بایاں_2].چوڑائی = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.چوڑائی, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
جدول_ورتیا_چوٹی_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;
	بنية box *صف = cell->ماپا;
	bool process_گروہ = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_چوٹی_style(cell->style);
	css_computed_border_چوٹی_color(cell->style, &a.c);
	css_computed_border_چوٹی_چوڑائی(cell->style, &a.چوڑائی, &a.unit);
	a.چوڑائی = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.چوڑائی, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	/* Top border of row */
	b.style = css_computed_border_چوٹی_style(صف->style);
	css_computed_border_چوٹی_color(صف->style, &b.c);
	css_computed_border_چوٹی_چوڑائی(صف->style, &b.چوڑائی, &b.unit);
	b.چوڑائی = css_unit_len2device_px(صف->style, unit_len_ctx,
			b.چوڑائی, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_صف;

	إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	إذا (صف->prev != عدم) {
		/* Consider row(s) above */
		طالما (جدول_cell_چوٹی_process_صف(unit_len_ctx, cell, صف->prev,
						  &a, &a_src) == false) {
			إذا (صف->prev->prev == عدم) {
				/* Consider row group */
				process_گروہ = true;
				اكسر;
			} وإلا {
				صف = صف->prev;
			}
		}
	} وإلا {
		process_گروہ = true;
	}

	إذا (process_گروہ) {
		بنية box *گروہ = صف->ماپا;

		/* Top border of row group */
		b.style = css_computed_border_چوٹی_style(گروہ->style);
		b.color = css_computed_border_چوٹی_color(گروہ->style, &b.c);
		css_computed_border_چوٹی_چوڑائی(گروہ->style, &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(گروہ->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف_گروہ;

		إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		إذا (گروہ->prev == عدم) {
			/* Top border of table */
			جدول_cell_چوٹی_process_جدول(unit_len_ctx,
						     گروہ->ماپا, &a, &a_src);
		} وإلا {
			/* Process previous group(s) */
			طالما (جدول_cell_چوٹی_process_گروہ(unit_len_ctx,
							    cell, گروہ->prev,
							    &a, &a_src) == false) {
				إذا (گروہ->prev->prev == عدم) {
					/* Top border of table */
					جدول_cell_چوٹی_process_جدول(unit_len_ctx,
								     گروہ->ماپا,
								     &a, &a_src);
					اكسر;
				} وإلا {
					گروہ = گروہ->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[چوٹی_2].style = a.style;
	cell->border[چوٹی_2].c = a.c;
	cell->border[چوٹی_2].چوڑائی = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.چوڑائی, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
جدول_ورتیا_دایاں_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_دایاں_style(cell->style);
	css_computed_border_دایاں_color(cell->style, &a.c);
	css_computed_border_دایاں_چوڑائی(cell->style, &a.چوڑائی, &a.unit);
	a.چوڑائی = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.چوڑائی, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	إذا (cell->اگلا != عدم || cell->شروع_کالم + cell->columns !=
	    cell->ماپا->ماپا->ماپا->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.چوڑائی = 0;
		a.unit = CSS_UNIT_PX;
	} وإلا {
		/* Last cell in row, so consider rows and row group */
		بنية box *صف = cell->ماپا;
		بنية box *گروہ = صف->ماپا;
		بنية box *جدول = گروہ->ماپا;
		بنانشانی صحيح صفاں = cell->صفاں;

		طالما (صفاں-- > 0 && صف != عدم) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_دایاں_style(صف->style);
			b.color = css_computed_border_دایاں_color(صف->style,
								  &b.c);
			css_computed_border_دایاں_چوڑائی(صف->style,
							&b.چوڑائی,
							&b.unit);
			b.چوڑائی = css_unit_len2device_px(
					صف->style, unit_len_ctx,
					b.چوڑائی, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_جدول_صف;

			إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			صف = صف->اگلا;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_دایاں_style(گروہ->style);
		b.color = css_computed_border_دایاں_color(گروہ->style, &b.c);
		css_computed_border_دایاں_چوڑائی(گروہ->style,
						&b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(گروہ->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف_گروہ;

		إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_دایاں_style(جدول->style);
		b.color = css_computed_border_دایاں_color(جدول->style, &b.c);
		css_computed_border_دایاں_چوڑائی(جدول->style,
						&b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(جدول->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول;

		إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[دایاں_2].style = a.style;
	cell->border[دایاں_2].c = a.c;
	cell->border[دایاں_2].چوڑائی = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.چوڑائی, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
جدول_ورتیا_bottom_border_for_cell(ثابت css_unit_ctx *unit_len_ctx,
				  بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;
	بنية box *صف = cell->ماپا;
	بنانشانی صحيح صفاں = cell->صفاں;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_چوڑائی(cell->style, &a.چوڑائی, &a.unit);
	a.چوڑائی = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.چوڑائی, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	طالما (صفاں-- > 0 && صف != عدم)
		صف = صف->اگلا;

	/** \todo Can cells span row groups? */

	إذا (صف != عدم) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.چوڑائی = 0;
		a.unit = CSS_UNIT_PX;
	} وإلا {
		/* Cell at bottom of table, so consider row and row group */
		بنية box *صف = cell->ماپا;
		بنية box *گروہ = صف->ماپا;
		بنية box *جدول = گروہ->ماپا;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(صف->style);
		b.color = css_computed_border_bottom_color(صف->style, &b.c);
		css_computed_border_bottom_چوڑائی(صف->style, &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(صف->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف;

		إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* Row group -- consider its bottom border */
		b.style = css_computed_border_bottom_style(گروہ->style);
		b.color = css_computed_border_bottom_color(گروہ->style, &b.c);
		css_computed_border_bottom_چوڑائی(گروہ->style,
						 &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(گروہ->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف_گروہ;

		إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its bottom border */
		b.style = css_computed_border_bottom_style(جدول->style);
		b.color = css_computed_border_bottom_color(جدول->style, &b.c);
		css_computed_border_bottom_چوڑائی(جدول->style,
						 &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(جدول->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول;

		إذا (جدول_border_is_ہور_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
		}
	}

	/* a now contains the used bottom border for the cell */
	cell->border[BOTTOM].style = a.style;
	cell->border[BOTTOM].c = a.c;
	cell->border[BOTTOM].چوڑائی = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.چوڑائی, a.unit));
}


/* exported interface documented in html/table.h */
bool
جدول_calculate_کالم_types(ثابت css_unit_ctx *unit_len_ctx, بنية box *جدول)
{
	بنانشانی صحيح i, j;
	بنية کالم *خانہ;
	بنية box *صف_گروہ, *صف, *cell;

	إذا (جدول->خانہ)
		/* table->col already constructed, for example frameset table */
		أرجع true;

	جدول->خانہ = خانہ = talloc_array(جدول, بنية کالم, جدول->columns);
	إذا (!خانہ)
		أرجع false;

	لكل (i = 0; i != جدول->columns; i++) {
		خانہ[i].type = کالم_چوڑائی_UNKNOWN;
		خانہ[i].چوڑائی = 0;
		خانہ[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	لكل (صف_گروہ = جدول->children; صف_گروہ; صف_گروہ =صف_گروہ->اگلا)
		لكل (صف = صف_گروہ->children; صف; صف = صف->اگلا)
			لكل (cell = صف->children; cell; cell = cell->اگلا) {
				تعداد css_چوڑائی_e type;
				css_fixed قدر = 0;
				css_unit unit = CSS_UNIT_PX;

				أكد(cell->type == BOX_جدول_CELL);
				أكد(cell->style);

				إذا (cell->columns != 1)
					استمر;
				i = cell->شروع_کالم;

				إذا (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					خانہ[i].positioned = false;
				}

				type = css_computed_چوڑائی(cell->style, &قدر, &unit);

				/* fixed width takes priority over any other width type */
				إذا (خانہ[i].type != کالم_چوڑائی_FIXED &&
				    type == CSS_چوڑائی_سیٹ && unit != CSS_UNIT_PCT) {
					خانہ[i].type = کالم_چوڑائی_FIXED;
					خانہ[i].چوڑائی = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							قدر, unit));
					إذا (خانہ[i].چوڑائی < 0)
						خانہ[i].چوڑائی = 0;
					استمر;
				}

				إذا (خانہ[i].type != کالم_چوڑائی_UNKNOWN)
					استمر;

				إذا (type == CSS_چوڑائی_سیٹ && unit == CSS_UNIT_PCT) {
					خانہ[i].type = کالم_چوڑائی_PERCENT;
					خانہ[i].چوڑائی = FIXTOINT(قدر);
					إذا (خانہ[i].چوڑائی < 0)
						خانہ[i].چوڑائی = 0;
				} وإلا إذا (type == CSS_چوڑائی_AUTO) {
					خانہ[i].type = کالم_چوڑائی_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	لكل (صف_گروہ = جدول->children; صف_گروہ; صف_گروہ =صف_گروہ->اگلا)
		لكل (صف = صف_گروہ->children; صف; صف = صف->اگلا)
			لكل (cell = صف->children; cell; cell = cell->اگلا) {
				بنانشانی صحيح fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				صحيح fixed_چوڑائی = 0, percent_چوڑائی = 0;
				تعداد css_چوڑائی_e type;
				css_fixed قدر = 0;
				css_unit unit = CSS_UNIT_PX;

				إذا (cell->columns == 1)
					استمر;
				i = cell->شروع_کالم;

				لكل (j = i; j < i + cell->columns; j++) {
					خانہ[j].positioned = false;
				}

				/* count column types in spanned cells */
				لكل (j = 0; j != cell->columns; j++) {
					إذا (خانہ[i + j].type == کالم_چوڑائی_FIXED) {
						fixed_چوڑائی += خانہ[i + j].چوڑائی;
						fixed_columns++;
					} وإلا إذا (خانہ[i + j].type == کالم_چوڑائی_PERCENT) {
						percent_چوڑائی += خانہ[i + j].چوڑائی;
						percent_columns++;
					} وإلا إذا (خانہ[i + j].type == کالم_چوڑائی_AUTO) {
						auto_columns++;
					} وإلا {
						unknown_columns++;
					}
				}

				إذا (!unknown_columns)
					استمر;

				type = css_computed_چوڑائی(cell->style, &قدر, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				إذا (type == CSS_چوڑائی_سیٹ && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					صحيح چوڑائی = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							قدر, unit)) -
						fixed_چوڑائی) / unknown_columns;
					إذا (چوڑائی < 0)
						چوڑائی = 0;
					لكل (j = 0; j != cell->columns; j++) {
						إذا (خانہ[i + j].type == کالم_چوڑائی_UNKNOWN) {
							خانہ[i + j].type = کالم_چوڑائی_FIXED;
							خانہ[i + j].چوڑائی = چوڑائی;
						}
					}
				}

				/* as above for percentage width */
				إذا (type == CSS_چوڑائی_سیٹ && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					صحيح چوڑائی = (FIXTOFLT(قدر) -
						     percent_چوڑائی) / unknown_columns;
					إذا (چوڑائی < 0)
						چوڑائی = 0;
					لكل (j = 0; j != cell->columns; j++) {
						إذا (خانہ[i + j].type == کالم_چوڑائی_UNKNOWN) {
							خانہ[i + j].type = کالم_چوڑائی_PERCENT;
							خانہ[i + j].چوڑائی = چوڑائی;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	لكل (i = 0; i != جدول->columns; i++) {
		إذا (خانہ[i].type == کالم_چوڑائی_UNKNOWN)
			خانہ[i].type = کالم_چوڑائی_AUTO;
	}

#جے_تعریف_شدہ جدول_DEBUG
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
				      })[خانہ[i].type],
		      خانہ[i].چوڑائی);
#جے_انت

	أرجع true;
}


/* exported interface documented in html/table.h */
فراغ جدول_ورتیا_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	صحيح side;

	أكد(cell->type == BOX_جدول_CELL);

	إذا (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed چوڑائی = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[بایاں_2].style =
			css_computed_border_بایاں_style(cell->style);
		css_computed_border_بایاں_color(cell->style,
					       &cell->border[بایاں_2].c);
		css_computed_border_بایاں_چوڑائی(cell->style, &چوڑائی, &unit);
		cell->border[بایاں_2].چوڑائی =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					چوڑائی, unit));

		/* Top border */
		cell->border[چوٹی_2].style =
			css_computed_border_چوٹی_style(cell->style);
		css_computed_border_چوٹی_color(cell->style,
					      &cell->border[چوٹی_2].c);
		css_computed_border_چوٹی_چوڑائی(cell->style, &چوڑائی, &unit);
		cell->border[چوٹی_2].چوڑائی =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					چوڑائی, unit));

		/* Right border */
		cell->border[دایاں_2].style =
			css_computed_border_دایاں_style(cell->style);
		css_computed_border_دایاں_color(cell->style,
						&cell->border[دایاں_2].c);
		css_computed_border_دایاں_چوڑائی(cell->style, &چوڑائی, &unit);
		cell->border[دایاں_2].چوڑائی =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					چوڑائی, unit));

		/* Bottom border */
		cell->border[BOTTOM].style =
			css_computed_border_bottom_style(cell->style);
		css_computed_border_bottom_color(cell->style,
						 &cell->border[BOTTOM].c);
		css_computed_border_bottom_چوڑائی(cell->style, &چوڑائی, &unit);
		cell->border[BOTTOM].چوڑائی =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					چوڑائی, unit));
	} وإلا {
		/* Left border */
		جدول_ورتیا_بایاں_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		جدول_ورتیا_چوٹی_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		جدول_ورتیا_دایاں_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		جدول_ورتیا_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	لكل (side = 0; side != 4; side++) {
		إذا (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].چوڑائی = 0;
	}
}
