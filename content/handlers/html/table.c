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
#وصف_رد جدول_DEBUG

/**
 * Container for border values during table border calculations
 */
بنية border {
	تعداد css_border_style_e style;	/**< border-style */
	تعداد css_border_color_e color;	/**< border-color type */
	css_color c;			/**< border-color value */
	css_fixed ويڪر;		/**< border-width length */
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
جدول_border_is_وڌيڪ_eyecatching(ثابت css_unit_ctx *unit_len_ctx,
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
	awidth = css_unit_len2device_px(عدم, unit_len_ctx, a->ويڪر, a->unit);
	bwidth = css_unit_len2device_px(عدم, unit_len_ctx, b->ويڪر, b->unit);

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
	حالة BOX_جدول_صف_گروهه:  impact++; fallthrough;
	حالة BOX_جدول:            impact++; fallthrough;
	افتراضي:
		اكسر;
	}

	/** \todo COL/COL_GROUP */
	بدل (b_src) {
	حالة BOX_جدول_CELL:       impact--; fallthrough;
	حالة BOX_جدول_صف:        impact--; fallthrough;
	حالة BOX_جدول_صف_گروهه:  impact--; fallthrough;
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
جدول_cell_چوٽي_process_جدول(ثابت css_unit_ctx *unit_len_ctx,
			     بنية box *جدول,
			     بنية border *a,
			     box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_چوٽي_style(جدول->style);
	b.color = css_computed_border_چوٽي_color(جدول->style, &b.c);
	css_computed_border_چوٽي_ويڪر(جدول->style, &b.ويڪر, &b.unit);
	b.ويڪر = css_unit_len2device_px(جدول->style, unit_len_ctx,
			b.ويڪر, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول;

	إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
جدول_cell_چوٽي_process_صف(ثابت css_unit_ctx *unit_len_ctx,
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
	css_computed_border_bottom_ويڪر(صف->style, &b.ويڪر, &b.unit);
	b.ويڪر = css_unit_len2device_px(صف->style, unit_len_ctx,
			b.ويڪر, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_صف;

	إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	إذا (صف->children == عدم) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_چوٽي_style(صف->style);
		b.color = css_computed_border_چوٽي_color(صف->style, &b.c);
		css_computed_border_چوٽي_ويڪر(صف->style, &b.ويڪر, &b.unit);
		b.ويڪر = css_unit_len2device_px(صف->style, unit_len_ctx,
				b.ويڪر, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف;

		إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
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
			لكل (c = صف->children; c != عدم; c = c->ايندڙ) {
				/* Ignore cells to the left */
				إذا (c->شروعات_ڪالم + c->columns - 1 <
				    cell->شروعات_ڪالم)
					استمر;
				/* Ignore cells to the right */
				إذا (c->شروعات_ڪالم > cell->شروعات_ڪالم +
				    cell->columns - 1)
					استمر;

				/* Flag that we've processed a cell */
				processed = true;

				/* Consider bottom border */
				b.style = css_computed_border_bottom_style(
							c->style);
				b.color = css_computed_border_bottom_color(
							c->style, &b.c);
				css_computed_border_bottom_ويڪر(c->style,
							&b.ويڪر, &b.unit);
				b.ويڪر = css_unit_len2device_px(
						c->style, unit_len_ctx,
						b.ويڪر, b.unit);
				b.unit = CSS_UNIT_PX;
				b_src = BOX_جدول_CELL;

				إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
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
جدول_cell_چوٽي_process_گروهه(ثابت css_unit_ctx *unit_len_ctx,
			     بنية box *cell,
			     بنية box *گروهه,
			     بنية border *a,
			     box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(گروهه->style);
	b.color = css_computed_border_bottom_color(گروهه->style, &b.c);
	css_computed_border_bottom_ويڪر(گروهه->style, &b.ويڪر, &b.unit);
	b.ويڪر = css_unit_len2device_px(گروهه->style, unit_len_ctx,
			b.ويڪر, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_صف_گروهه;

	إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	إذا (گروهه->آخري != عدم) {
		/* Process rows in group, starting with last */
		بنية box *صف = گروهه->آخري;

		طالما (جدول_cell_چوٽي_process_صف(unit_len_ctx, cell, صف,
						  a, a_src) == false) {
			إذا (صف->prev == عدم) {
				أرجع false;
			} وإلا {
				صف = صف->prev;
			}
		}
	} وإلا {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_چوٽي_style(گروهه->style);
		b.color = css_computed_border_چوٽي_color(گروهه->style, &b.c);
		css_computed_border_چوٽي_ويڪر(گروهه->style, &b.ويڪر, &b.unit);
		b.ويڪر = css_unit_len2device_px(گروهه->style, unit_len_ctx,
				b.ويڪر, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف_گروهه;

		إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
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
جدول_استعمال_ٿيل_کاٻو_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_کاٻو_style(cell->style);
	a.color = css_computed_border_کاٻو_color(cell->style, &a.c);
	css_computed_border_کاٻو_ويڪر(cell->style, &a.ويڪر, &a.unit);
	a.ويڪر = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.ويڪر, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	إذا (cell->prev != عدم || cell->شروعات_ڪالم != 0) {
		/* Cell to the left -- consider its right border */
		بنية box *prev = عدم;

		إذا (cell->prev == عدم) {
			بنية box *صف;

			/* Spanned from a previous row in current row group */
			لكل (صف = cell->والد; صف != عدم; صف = صف->prev) {
				لكل (prev = صف->children; prev != عدم;
				     prev = prev->ايندڙ) {
					إذا (prev->شروعات_ڪالم +
					    prev->columns ==
					    cell->شروعات_ڪالم)
						اكسر;
				}

				إذا (prev != عدم)
					اكسر;
			}

			أكد(prev != عدم);
		} وإلا {
			prev = cell->prev;
		}

		b.style = css_computed_border_ساڄو_style(prev->style);
		b.color = css_computed_border_ساڄو_color(prev->style, &b.c);
		css_computed_border_ساڄو_ويڪر(prev->style, &b.ويڪر, &b.unit);
		b.ويڪر = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.ويڪر, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_CELL;

		إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} وإلا {
		/* First cell in row, so consider rows and row group */
		بنية box *صف = cell->والد;
		بنية box *گروهه = صف->والد;
		بنية box *جدول = گروهه->والد;
		بنانشان صحيح صفون = cell->صفون;

		طالما (صفون-- > 0 && صف != عدم) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_کاٻو_style(صف->style);
			b.color = css_computed_border_کاٻو_color(
								 صف->style, &b.c);
			css_computed_border_کاٻو_ويڪر(
						       صف->style, &b.ويڪر, &b.unit);
			b.ويڪر = css_unit_len2device_px(
					صف->style, unit_len_ctx,
					b.ويڪر, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_جدول_صف;

			إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			صف = صف->ايندڙ;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_کاٻو_style(گروهه->style);
		b.color = css_computed_border_کاٻو_color(گروهه->style, &b.c);
		css_computed_border_کاٻو_ويڪر(گروهه->style, &b.ويڪر, &b.unit);
		b.ويڪر = css_unit_len2device_px(گروهه->style, unit_len_ctx,
				b.ويڪر, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف_گروهه;

		إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_کاٻو_style(جدول->style);
		b.color = css_computed_border_کاٻو_color(جدول->style, &b.c);
		css_computed_border_کاٻو_ويڪر(جدول->style, &b.ويڪر, &b.unit);
		b.ويڪر = css_unit_len2device_px(جدول->style, unit_len_ctx,
				b.ويڪر, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول;

		إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[کاٻو_2].style = a.style;
	cell->border[کاٻو_2].c = a.c;
	cell->border[کاٻو_2].ويڪر = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.ويڪر, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
جدول_استعمال_ٿيل_چوٽي_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;
	بنية box *صف = cell->والد;
	bool process_گروهه = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_چوٽي_style(cell->style);
	css_computed_border_چوٽي_color(cell->style, &a.c);
	css_computed_border_چوٽي_ويڪر(cell->style, &a.ويڪر, &a.unit);
	a.ويڪر = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.ويڪر, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	/* Top border of row */
	b.style = css_computed_border_چوٽي_style(صف->style);
	css_computed_border_چوٽي_color(صف->style, &b.c);
	css_computed_border_چوٽي_ويڪر(صف->style, &b.ويڪر, &b.unit);
	b.ويڪر = css_unit_len2device_px(صف->style, unit_len_ctx,
			b.ويڪر, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_صف;

	إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	إذا (صف->prev != عدم) {
		/* Consider row(s) above */
		طالما (جدول_cell_چوٽي_process_صف(unit_len_ctx, cell, صف->prev,
						  &a, &a_src) == false) {
			إذا (صف->prev->prev == عدم) {
				/* Consider row group */
				process_گروهه = true;
				اكسر;
			} وإلا {
				صف = صف->prev;
			}
		}
	} وإلا {
		process_گروهه = true;
	}

	إذا (process_گروهه) {
		بنية box *گروهه = صف->والد;

		/* Top border of row group */
		b.style = css_computed_border_چوٽي_style(گروهه->style);
		b.color = css_computed_border_چوٽي_color(گروهه->style, &b.c);
		css_computed_border_چوٽي_ويڪر(گروهه->style, &b.ويڪر, &b.unit);
		b.ويڪر = css_unit_len2device_px(گروهه->style, unit_len_ctx,
				b.ويڪر, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف_گروهه;

		إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		إذا (گروهه->prev == عدم) {
			/* Top border of table */
			جدول_cell_چوٽي_process_جدول(unit_len_ctx,
						     گروهه->والد, &a, &a_src);
		} وإلا {
			/* Process previous group(s) */
			طالما (جدول_cell_چوٽي_process_گروهه(unit_len_ctx,
							    cell, گروهه->prev,
							    &a, &a_src) == false) {
				إذا (گروهه->prev->prev == عدم) {
					/* Top border of table */
					جدول_cell_چوٽي_process_جدول(unit_len_ctx,
								     گروهه->والد,
								     &a, &a_src);
					اكسر;
				} وإلا {
					گروهه = گروهه->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[چوٽي_2].style = a.style;
	cell->border[چوٽي_2].c = a.c;
	cell->border[چوٽي_2].ويڪر = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.ويڪر, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
جدول_استعمال_ٿيل_ساڄو_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_ساڄو_style(cell->style);
	css_computed_border_ساڄو_color(cell->style, &a.c);
	css_computed_border_ساڄو_ويڪر(cell->style, &a.ويڪر, &a.unit);
	a.ويڪر = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.ويڪر, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	إذا (cell->ايندڙ != عدم || cell->شروعات_ڪالم + cell->columns !=
	    cell->والد->والد->والد->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.ويڪر = 0;
		a.unit = CSS_UNIT_PX;
	} وإلا {
		/* Last cell in row, so consider rows and row group */
		بنية box *صف = cell->والد;
		بنية box *گروهه = صف->والد;
		بنية box *جدول = گروهه->والد;
		بنانشان صحيح صفون = cell->صفون;

		طالما (صفون-- > 0 && صف != عدم) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_ساڄو_style(صف->style);
			b.color = css_computed_border_ساڄو_color(صف->style,
								  &b.c);
			css_computed_border_ساڄو_ويڪر(صف->style,
							&b.ويڪر,
							&b.unit);
			b.ويڪر = css_unit_len2device_px(
					صف->style, unit_len_ctx,
					b.ويڪر, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_جدول_صف;

			إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			صف = صف->ايندڙ;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_ساڄو_style(گروهه->style);
		b.color = css_computed_border_ساڄو_color(گروهه->style, &b.c);
		css_computed_border_ساڄو_ويڪر(گروهه->style,
						&b.ويڪر, &b.unit);
		b.ويڪر = css_unit_len2device_px(گروهه->style, unit_len_ctx,
				b.ويڪر, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف_گروهه;

		إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_ساڄو_style(جدول->style);
		b.color = css_computed_border_ساڄو_color(جدول->style, &b.c);
		css_computed_border_ساڄو_ويڪر(جدول->style,
						&b.ويڪر, &b.unit);
		b.ويڪر = css_unit_len2device_px(جدول->style, unit_len_ctx,
				b.ويڪر, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول;

		إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[ساڄو_2].style = a.style;
	cell->border[ساڄو_2].c = a.c;
	cell->border[ساڄو_2].ويڪر = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.ويڪر, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
جدول_استعمال_ٿيل_bottom_border_for_cell(ثابت css_unit_ctx *unit_len_ctx,
				  بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;
	بنية box *صف = cell->والد;
	بنانشان صحيح صفون = cell->صفون;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_ويڪر(cell->style, &a.ويڪر, &a.unit);
	a.ويڪر = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.ويڪر, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	طالما (صفون-- > 0 && صف != عدم)
		صف = صف->ايندڙ;

	/** \todo Can cells span row groups? */

	إذا (صف != عدم) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.ويڪر = 0;
		a.unit = CSS_UNIT_PX;
	} وإلا {
		/* Cell at bottom of table, so consider row and row group */
		بنية box *صف = cell->والد;
		بنية box *گروهه = صف->والد;
		بنية box *جدول = گروهه->والد;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(صف->style);
		b.color = css_computed_border_bottom_color(صف->style, &b.c);
		css_computed_border_bottom_ويڪر(صف->style, &b.ويڪر, &b.unit);
		b.ويڪر = css_unit_len2device_px(صف->style, unit_len_ctx,
				b.ويڪر, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف;

		إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* Row group -- consider its bottom border */
		b.style = css_computed_border_bottom_style(گروهه->style);
		b.color = css_computed_border_bottom_color(گروهه->style, &b.c);
		css_computed_border_bottom_ويڪر(گروهه->style,
						 &b.ويڪر, &b.unit);
		b.ويڪر = css_unit_len2device_px(گروهه->style, unit_len_ctx,
				b.ويڪر, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_صف_گروهه;

		إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its bottom border */
		b.style = css_computed_border_bottom_style(جدول->style);
		b.color = css_computed_border_bottom_color(جدول->style, &b.c);
		css_computed_border_bottom_ويڪر(جدول->style,
						 &b.ويڪر, &b.unit);
		b.ويڪر = css_unit_len2device_px(جدول->style, unit_len_ctx,
				b.ويڪر, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول;

		إذا (جدول_border_is_وڌيڪ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
		}
	}

	/* a now contains the used bottom border for the cell */
	cell->border[BOTTOM].style = a.style;
	cell->border[BOTTOM].c = a.c;
	cell->border[BOTTOM].ويڪر = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.ويڪر, a.unit));
}


/* exported interface documented in html/table.h */
bool
جدول_calculate_ڪالم_types(ثابت css_unit_ctx *unit_len_ctx, بنية box *جدول)
{
	بنانشان صحيح i, j;
	بنية ڪالم *کانو;
	بنية box *صف_گروهه, *صف, *cell;

	إذا (جدول->کانو)
		/* table->col already constructed, for example frameset table */
		أرجع true;

	جدول->کانو = کانو = talloc_array(جدول, بنية ڪالم, جدول->columns);
	إذا (!کانو)
		أرجع false;

	لكل (i = 0; i != جدول->columns; i++) {
		کانو[i].type = ڪالم_ويڪر_UNKNOWN;
		کانو[i].ويڪر = 0;
		کانو[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	لكل (صف_گروهه = جدول->children; صف_گروهه; صف_گروهه =صف_گروهه->ايندڙ)
		لكل (صف = صف_گروهه->children; صف; صف = صف->ايندڙ)
			لكل (cell = صف->children; cell; cell = cell->ايندڙ) {
				تعداد css_ويڪر_e type;
				css_fixed قدر = 0;
				css_unit unit = CSS_UNIT_PX;

				أكد(cell->type == BOX_جدول_CELL);
				أكد(cell->style);

				إذا (cell->columns != 1)
					استمر;
				i = cell->شروعات_ڪالم;

				إذا (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					کانو[i].positioned = false;
				}

				type = css_computed_ويڪر(cell->style, &قدر, &unit);

				/* fixed width takes priority over any other width type */
				إذا (کانو[i].type != ڪالم_ويڪر_FIXED &&
				    type == CSS_ويڪر_سيٽ && unit != CSS_UNIT_PCT) {
					کانو[i].type = ڪالم_ويڪر_FIXED;
					کانو[i].ويڪر = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							قدر, unit));
					إذا (کانو[i].ويڪر < 0)
						کانو[i].ويڪر = 0;
					استمر;
				}

				إذا (کانو[i].type != ڪالم_ويڪر_UNKNOWN)
					استمر;

				إذا (type == CSS_ويڪر_سيٽ && unit == CSS_UNIT_PCT) {
					کانو[i].type = ڪالم_ويڪر_PERCENT;
					کانو[i].ويڪر = FIXTOINT(قدر);
					إذا (کانو[i].ويڪر < 0)
						کانو[i].ويڪر = 0;
				} وإلا إذا (type == CSS_ويڪر_AUTO) {
					کانو[i].type = ڪالم_ويڪر_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	لكل (صف_گروهه = جدول->children; صف_گروهه; صف_گروهه =صف_گروهه->ايندڙ)
		لكل (صف = صف_گروهه->children; صف; صف = صف->ايندڙ)
			لكل (cell = صف->children; cell; cell = cell->ايندڙ) {
				بنانشان صحيح fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				صحيح fixed_ويڪر = 0, percent_ويڪر = 0;
				تعداد css_ويڪر_e type;
				css_fixed قدر = 0;
				css_unit unit = CSS_UNIT_PX;

				إذا (cell->columns == 1)
					استمر;
				i = cell->شروعات_ڪالم;

				لكل (j = i; j < i + cell->columns; j++) {
					کانو[j].positioned = false;
				}

				/* count column types in spanned cells */
				لكل (j = 0; j != cell->columns; j++) {
					إذا (کانو[i + j].type == ڪالم_ويڪر_FIXED) {
						fixed_ويڪر += کانو[i + j].ويڪر;
						fixed_columns++;
					} وإلا إذا (کانو[i + j].type == ڪالم_ويڪر_PERCENT) {
						percent_ويڪر += کانو[i + j].ويڪر;
						percent_columns++;
					} وإلا إذا (کانو[i + j].type == ڪالم_ويڪر_AUTO) {
						auto_columns++;
					} وإلا {
						unknown_columns++;
					}
				}

				إذا (!unknown_columns)
					استمر;

				type = css_computed_ويڪر(cell->style, &قدر, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				إذا (type == CSS_ويڪر_سيٽ && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					صحيح ويڪر = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							قدر, unit)) -
						fixed_ويڪر) / unknown_columns;
					إذا (ويڪر < 0)
						ويڪر = 0;
					لكل (j = 0; j != cell->columns; j++) {
						إذا (کانو[i + j].type == ڪالم_ويڪر_UNKNOWN) {
							کانو[i + j].type = ڪالم_ويڪر_FIXED;
							کانو[i + j].ويڪر = ويڪر;
						}
					}
				}

				/* as above for percentage width */
				إذا (type == CSS_ويڪر_سيٽ && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					صحيح ويڪر = (FIXTOFLT(قدر) -
						     percent_ويڪر) / unknown_columns;
					إذا (ويڪر < 0)
						ويڪر = 0;
					لكل (j = 0; j != cell->columns; j++) {
						إذا (کانو[i + j].type == ڪالم_ويڪر_UNKNOWN) {
							کانو[i + j].type = ڪالم_ويڪر_PERCENT;
							کانو[i + j].ويڪر = ويڪر;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	لكل (i = 0; i != جدول->columns; i++) {
		إذا (کانو[i].type == ڪالم_ويڪر_UNKNOWN)
			کانو[i].type = ڪالم_ويڪر_AUTO;
	}

#جيڪڏهن_وصف_ٿيل جدول_DEBUG
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
				      })[کانو[i].type],
		      کانو[i].ويڪر);
#جيڪڏهن_پورو

	أرجع true;
}


/* exported interface documented in html/table.h */
فراغ جدول_استعمال_ٿيل_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	صحيح side;

	أكد(cell->type == BOX_جدول_CELL);

	إذا (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed ويڪر = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[کاٻو_2].style =
			css_computed_border_کاٻو_style(cell->style);
		css_computed_border_کاٻو_color(cell->style,
					       &cell->border[کاٻو_2].c);
		css_computed_border_کاٻو_ويڪر(cell->style, &ويڪر, &unit);
		cell->border[کاٻو_2].ويڪر =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					ويڪر, unit));

		/* Top border */
		cell->border[چوٽي_2].style =
			css_computed_border_چوٽي_style(cell->style);
		css_computed_border_چوٽي_color(cell->style,
					      &cell->border[چوٽي_2].c);
		css_computed_border_چوٽي_ويڪر(cell->style, &ويڪر, &unit);
		cell->border[چوٽي_2].ويڪر =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					ويڪر, unit));

		/* Right border */
		cell->border[ساڄو_2].style =
			css_computed_border_ساڄو_style(cell->style);
		css_computed_border_ساڄو_color(cell->style,
						&cell->border[ساڄو_2].c);
		css_computed_border_ساڄو_ويڪر(cell->style, &ويڪر, &unit);
		cell->border[ساڄو_2].ويڪر =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					ويڪر, unit));

		/* Bottom border */
		cell->border[BOTTOM].style =
			css_computed_border_bottom_style(cell->style);
		css_computed_border_bottom_color(cell->style,
						 &cell->border[BOTTOM].c);
		css_computed_border_bottom_ويڪر(cell->style, &ويڪر, &unit);
		cell->border[BOTTOM].ويڪر =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					ويڪر, unit));
	} وإلا {
		/* Left border */
		جدول_استعمال_ٿيل_کاٻو_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		جدول_استعمال_ٿيل_چوٽي_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		جدول_استعمال_ٿيل_ساڄو_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		جدول_استعمال_ٿيل_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	لكل (side = 0; side != 4; side++) {
		إذا (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].ويڪر = 0;
	}
}
