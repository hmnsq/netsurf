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

#شامل <assert.h>
#شامل <dom/dom.h>

#شامل "utils/log.h"
#شامل "utils/utils.h"
#شامل "utils/talloc.h"
#شامل "css/utils.h"

#شامل "html/box.h"
#شامل "html/table.h"

/* Define to enable verbose table debug */
#تعریف_منسوخ جدول_DEBUG

/**
 * Container for border values during table border calculations
 */
ساخت border {
	شمار css_border_style_e style;	/**< border-style */
	شمار css_border_color_e color;	/**< border-color type */
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
ساکن bool
جدول_border_is_مزید_eyecatching(ثابت css_unit_ctx *unit_len_ctx,
				 ثابت ساخت border *a,
				 box_type a_src,
				 ثابت ساخت border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	صحیح impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	اگر (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		واپس false;

	اگر (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		واپس true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	أكد(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	أكد(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(تہی, unit_len_ctx, a->چوڑائی, a->unit);
	bwidth = css_unit_len2device_px(تہی, unit_len_ctx, b->چوڑائی, b->unit);

	اگر (awidth < bwidth)
		واپس true;
	ورنہ اگر (bwidth < awidth)
		واپس false;

	/* 3b -- sort by style */
	انتخاب (a->style) {
	حالت CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	حالت CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	حالت CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	حالت CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	حالت CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	حالت CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	حالت CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	حالت CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	طےشدہ:
		توڑو;
	}

	انتخاب (b->style) {
	حالت CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	حالت CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	حالت CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	حالت CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	حالت CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	حالت CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	حالت CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	حالت CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	طےشدہ:
		توڑو;
	}

	اگر (impact < 0)
		واپس true;
	ورنہ اگر (impact > 0)
		واپس false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	انتخاب (a_src) {
	حالت BOX_جدول_CELL:       impact++; fallthrough;
	حالت BOX_جدول_صف:        impact++; fallthrough;
	حالت BOX_جدول_صف_گروہ:  impact++; fallthrough;
	حالت BOX_جدول:            impact++; fallthrough;
	طےشدہ:
		توڑو;
	}

	/** \todo COL/COL_GROUP */
	انتخاب (b_src) {
	حالت BOX_جدول_CELL:       impact--; fallthrough;
	حالت BOX_جدول_صف:        impact--; fallthrough;
	حالت BOX_جدول_صف_گروہ:  impact--; fallthrough;
	حالت BOX_جدول:            impact--; fallthrough;
	طےشدہ:
		توڑو;
	}

	اگر (impact < 0)
		واپس true;
	ورنہ اگر (impact > 0)
		واپس false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	واپس true;
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
ساکن خالی
جدول_cell_چوٹی_process_جدول(ثابت css_unit_ctx *unit_len_ctx,
			     ساخت box *جدول,
			     ساخت border *a,
			     box_type *a_src)
{
	ساخت border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_چوٹی_style(جدول->style);
	b.color = css_computed_border_چوٹی_color(جدول->style, &b.c);
	css_computed_border_چوٹی_چوڑائی(جدول->style, &b.چوڑائی, &b.unit);
	b.چوڑائی = css_unit_len2device_px(جدول->style, unit_len_ctx,
			b.چوڑائی, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول;

	اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
ساکن bool
جدول_cell_چوٹی_process_صف(ثابت css_unit_ctx *unit_len_ctx,
			   ساخت box *cell,
			   ساخت box *صف,
			   ساخت border *a,
			   box_type *a_src)
{
	ساخت border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(صف->style);
	b.color = css_computed_border_bottom_color(صف->style, &b.c);
	css_computed_border_bottom_چوڑائی(صف->style, &b.چوڑائی, &b.unit);
	b.چوڑائی = css_unit_len2device_px(صف->style, unit_len_ctx,
			b.چوڑائی, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_صف;

	اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	اگر (صف->children == تہی) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_چوٹی_style(صف->style);
		b.color = css_computed_border_چوٹی_color(صف->style, &b.c);
		css_computed_border_چوٹی_چوڑائی(صف->style, &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(صف->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف;

		اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		واپس false;
	} ورنہ {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		ساخت box *c;
		bool processed = false;

		جب (processed == false) {
			برائے (c = صف->children; c != تہی; c = c->اگلا) {
				/* Ignore cells to the left */
				اگر (c->آغاز_عمود + c->columns - 1 <
				    cell->آغاز_عمود)
					جاری;
				/* Ignore cells to the right */
				اگر (c->آغاز_عمود > cell->آغاز_عمود +
				    cell->columns - 1)
					جاری;

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

				اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			اگر (processed == false) {
				/* There must be a preceding row */
				أكد(صف->پچھلا != تہی);

				صف = صف->پچھلا;
			}
		}
	}

	واپس true;
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
ساکن bool
جدول_cell_چوٹی_process_گروہ(ثابت css_unit_ctx *unit_len_ctx,
			     ساخت box *cell,
			     ساخت box *گروہ,
			     ساخت border *a,
			     box_type *a_src)
{
	ساخت border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(گروہ->style);
	b.color = css_computed_border_bottom_color(گروہ->style, &b.c);
	css_computed_border_bottom_چوڑائی(گروہ->style, &b.چوڑائی, &b.unit);
	b.چوڑائی = css_unit_len2device_px(گروہ->style, unit_len_ctx,
			b.چوڑائی, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_صف_گروہ;

	اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	اگر (گروہ->آخری != تہی) {
		/* Process rows in group, starting with last */
		ساخت box *صف = گروہ->آخری;

		جب (جدول_cell_چوٹی_process_صف(unit_len_ctx, cell, صف,
						  a, a_src) == false) {
			اگر (صف->پچھلا == تہی) {
				واپس false;
			} ورنہ {
				صف = صف->پچھلا;
			}
		}
	} ورنہ {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_چوٹی_style(گروہ->style);
		b.color = css_computed_border_چوٹی_color(گروہ->style, &b.c);
		css_computed_border_چوٹی_چوڑائی(گروہ->style, &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(گروہ->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف_گروہ;

		اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		واپس false;
	}

	واپس true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساکن خالی
جدول_مستعمل_بایاں_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, ساخت box *cell)
{
	ساخت border a, b;
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

	اگر (cell->پچھلا != تہی || cell->آغاز_عمود != 0) {
		/* Cell to the left -- consider its right border */
		ساخت box *پچھلا = تہی;

		اگر (cell->پچھلا == تہی) {
			ساخت box *صف;

			/* Spanned from a previous row in current row group */
			برائے (صف = cell->والد; صف != تہی; صف = صف->پچھلا) {
				برائے (پچھلا = صف->children; پچھلا != تہی;
				     پچھلا = پچھلا->اگلا) {
					اگر (پچھلا->آغاز_عمود +
					    پچھلا->columns ==
					    cell->آغاز_عمود)
						توڑو;
				}

				اگر (پچھلا != تہی)
					توڑو;
			}

			أكد(پچھلا != تہی);
		} ورنہ {
			پچھلا = cell->پچھلا;
		}

		b.style = css_computed_border_دایاں_style(پچھلا->style);
		b.color = css_computed_border_دایاں_color(پچھلا->style, &b.c);
		css_computed_border_دایاں_چوڑائی(پچھلا->style, &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(پچھلا->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_CELL;

		اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} ورنہ {
		/* First cell in row, so consider rows and row group */
		ساخت box *صف = cell->والد;
		ساخت box *گروہ = صف->والد;
		ساخت box *جدول = گروہ->والد;
		غیرعلامتی صحیح صفیں = cell->صفیں;

		جب (صفیں-- > 0 && صف != تہی) {
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

			اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
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

		اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
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

		اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
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
ساکن خالی
جدول_مستعمل_چوٹی_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, ساخت box *cell)
{
	ساخت border a, b;
	box_type a_src, b_src;
	ساخت box *صف = cell->والد;
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

	اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	اگر (صف->پچھلا != تہی) {
		/* Consider row(s) above */
		جب (جدول_cell_چوٹی_process_صف(unit_len_ctx, cell, صف->پچھلا,
						  &a, &a_src) == false) {
			اگر (صف->پچھلا->پچھلا == تہی) {
				/* Consider row group */
				process_گروہ = true;
				توڑو;
			} ورنہ {
				صف = صف->پچھلا;
			}
		}
	} ورنہ {
		process_گروہ = true;
	}

	اگر (process_گروہ) {
		ساخت box *گروہ = صف->والد;

		/* Top border of row group */
		b.style = css_computed_border_چوٹی_style(گروہ->style);
		b.color = css_computed_border_چوٹی_color(گروہ->style, &b.c);
		css_computed_border_چوٹی_چوڑائی(گروہ->style, &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(گروہ->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف_گروہ;

		اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		اگر (گروہ->پچھلا == تہی) {
			/* Top border of table */
			جدول_cell_چوٹی_process_جدول(unit_len_ctx,
						     گروہ->والد, &a, &a_src);
		} ورنہ {
			/* Process previous group(s) */
			جب (جدول_cell_چوٹی_process_گروہ(unit_len_ctx,
							    cell, گروہ->پچھلا,
							    &a, &a_src) == false) {
				اگر (گروہ->پچھلا->پچھلا == تہی) {
					/* Top border of table */
					جدول_cell_چوٹی_process_جدول(unit_len_ctx,
								     گروہ->والد,
								     &a, &a_src);
					توڑو;
				} ورنہ {
					گروہ = گروہ->پچھلا;
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
ساکن خالی
جدول_مستعمل_دایاں_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, ساخت box *cell)
{
	ساخت border a, b;
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

	اگر (cell->اگلا != تہی || cell->آغاز_عمود + cell->columns !=
	    cell->والد->والد->والد->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.چوڑائی = 0;
		a.unit = CSS_UNIT_PX;
	} ورنہ {
		/* Last cell in row, so consider rows and row group */
		ساخت box *صف = cell->والد;
		ساخت box *گروہ = صف->والد;
		ساخت box *جدول = گروہ->والد;
		غیرعلامتی صحیح صفیں = cell->صفیں;

		جب (صفیں-- > 0 && صف != تہی) {
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

			اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
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

		اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
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

		اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
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
ساکن خالی
جدول_مستعمل_bottom_border_for_cell(ثابت css_unit_ctx *unit_len_ctx,
				  ساخت box *cell)
{
	ساخت border a, b;
	box_type a_src, b_src;
	ساخت box *صف = cell->والد;
	غیرعلامتی صحیح صفیں = cell->صفیں;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_چوڑائی(cell->style, &a.چوڑائی, &a.unit);
	a.چوڑائی = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.چوڑائی, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	جب (صفیں-- > 0 && صف != تہی)
		صف = صف->اگلا;

	/** \todo Can cells span row groups? */

	اگر (صف != تہی) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.چوڑائی = 0;
		a.unit = CSS_UNIT_PX;
	} ورنہ {
		/* Cell at bottom of table, so consider row and row group */
		ساخت box *صف = cell->والد;
		ساخت box *گروہ = صف->والد;
		ساخت box *جدول = گروہ->والد;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(صف->style);
		b.color = css_computed_border_bottom_color(صف->style, &b.c);
		css_computed_border_bottom_چوڑائی(صف->style, &b.چوڑائی, &b.unit);
		b.چوڑائی = css_unit_len2device_px(صف->style, unit_len_ctx,
				b.چوڑائی, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف;

		اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
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

		اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
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

		اگر (جدول_border_is_مزید_eyecatching(unit_len_ctx,
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
جدول_calculate_عمود_types(ثابت css_unit_ctx *unit_len_ctx, ساخت box *جدول)
{
	غیرعلامتی صحیح i, j;
	ساخت عمود *ستون;
	ساخت box *صف_گروہ, *صف, *cell;

	اگر (جدول->ستون)
		/* table->col already constructed, for example frameset table */
		واپس true;

	جدول->ستون = ستون = talloc_array(جدول, ساخت عمود, جدول->columns);
	اگر (!ستون)
		واپس false;

	برائے (i = 0; i != جدول->columns; i++) {
		ستون[i].type = عمود_چوڑائی_UNKNOWN;
		ستون[i].چوڑائی = 0;
		ستون[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	برائے (صف_گروہ = جدول->children; صف_گروہ; صف_گروہ =صف_گروہ->اگلا)
		برائے (صف = صف_گروہ->children; صف; صف = صف->اگلا)
			برائے (cell = صف->children; cell; cell = cell->اگلا) {
				شمار css_چوڑائی_e type;
				css_fixed قدر = 0;
				css_unit unit = CSS_UNIT_PX;

				أكد(cell->type == BOX_جدول_CELL);
				أكد(cell->style);

				اگر (cell->columns != 1)
					جاری;
				i = cell->آغاز_عمود;

				اگر (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					ستون[i].positioned = false;
				}

				type = css_computed_چوڑائی(cell->style, &قدر, &unit);

				/* fixed width takes priority over any other width type */
				اگر (ستون[i].type != عمود_چوڑائی_FIXED &&
				    type == CSS_چوڑائی_سیٹ && unit != CSS_UNIT_PCT) {
					ستون[i].type = عمود_چوڑائی_FIXED;
					ستون[i].چوڑائی = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							قدر, unit));
					اگر (ستون[i].چوڑائی < 0)
						ستون[i].چوڑائی = 0;
					جاری;
				}

				اگر (ستون[i].type != عمود_چوڑائی_UNKNOWN)
					جاری;

				اگر (type == CSS_چوڑائی_سیٹ && unit == CSS_UNIT_PCT) {
					ستون[i].type = عمود_چوڑائی_PERCENT;
					ستون[i].چوڑائی = FIXTOINT(قدر);
					اگر (ستون[i].چوڑائی < 0)
						ستون[i].چوڑائی = 0;
				} ورنہ اگر (type == CSS_چوڑائی_AUTO) {
					ستون[i].type = عمود_چوڑائی_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	برائے (صف_گروہ = جدول->children; صف_گروہ; صف_گروہ =صف_گروہ->اگلا)
		برائے (صف = صف_گروہ->children; صف; صف = صف->اگلا)
			برائے (cell = صف->children; cell; cell = cell->اگلا) {
				غیرعلامتی صحیح fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				صحیح fixed_چوڑائی = 0, percent_چوڑائی = 0;
				شمار css_چوڑائی_e type;
				css_fixed قدر = 0;
				css_unit unit = CSS_UNIT_PX;

				اگر (cell->columns == 1)
					جاری;
				i = cell->آغاز_عمود;

				برائے (j = i; j < i + cell->columns; j++) {
					ستون[j].positioned = false;
				}

				/* count column types in spanned cells */
				برائے (j = 0; j != cell->columns; j++) {
					اگر (ستون[i + j].type == عمود_چوڑائی_FIXED) {
						fixed_چوڑائی += ستون[i + j].چوڑائی;
						fixed_columns++;
					} ورنہ اگر (ستون[i + j].type == عمود_چوڑائی_PERCENT) {
						percent_چوڑائی += ستون[i + j].چوڑائی;
						percent_columns++;
					} ورنہ اگر (ستون[i + j].type == عمود_چوڑائی_AUTO) {
						auto_columns++;
					} ورنہ {
						unknown_columns++;
					}
				}

				اگر (!unknown_columns)
					جاری;

				type = css_computed_چوڑائی(cell->style, &قدر, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				اگر (type == CSS_چوڑائی_سیٹ && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					صحیح چوڑائی = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							قدر, unit)) -
						fixed_چوڑائی) / unknown_columns;
					اگر (چوڑائی < 0)
						چوڑائی = 0;
					برائے (j = 0; j != cell->columns; j++) {
						اگر (ستون[i + j].type == عمود_چوڑائی_UNKNOWN) {
							ستون[i + j].type = عمود_چوڑائی_FIXED;
							ستون[i + j].چوڑائی = چوڑائی;
						}
					}
				}

				/* as above for percentage width */
				اگر (type == CSS_چوڑائی_سیٹ && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					صحیح چوڑائی = (FIXTOFLT(قدر) -
						     percent_چوڑائی) / unknown_columns;
					اگر (چوڑائی < 0)
						چوڑائی = 0;
					برائے (j = 0; j != cell->columns; j++) {
						اگر (ستون[i + j].type == عمود_چوڑائی_UNKNOWN) {
							ستون[i + j].type = عمود_چوڑائی_PERCENT;
							ستون[i + j].چوڑائی = چوڑائی;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	برائے (i = 0; i != جدول->columns; i++) {
		اگر (ستون[i].type == عمود_چوڑائی_UNKNOWN)
			ستون[i].type = عمود_چوڑائی_AUTO;
	}

#اگر_تعریف_شدہ جدول_DEBUG
	برائے (i = 0; i != جدول->columns; i++)
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
		      ستون[i].چوڑائی);
#اگر_ختم

	واپس true;
}


/* exported interface documented in html/table.h */
خالی جدول_مستعمل_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, ساخت box *cell)
{
	صحیح side;

	أكد(cell->type == BOX_جدول_CELL);

	اگر (css_computed_border_collapse(cell->style) ==
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
	} ورنہ {
		/* Left border */
		جدول_مستعمل_بایاں_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		جدول_مستعمل_چوٹی_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		جدول_مستعمل_دایاں_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		جدول_مستعمل_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	برائے (side = 0; side != 4; side++) {
		اگر (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].چوڑائی = 0;
	}
}
