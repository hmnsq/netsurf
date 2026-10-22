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
#لغو_تعریف جدول_DEBUG

/**
 * Container for border values during table border calculations
 */
بنية border {
	تعداد css_border_style_e style;	/**< border-style */
	تعداد css_border_color_e color;	/**< border-color type */
	css_color c;			/**< border-color value */
	css_fixed عرض;		/**< border-width length */
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
جدول_border_is_بیشتر_eyecatching(ثابت css_unit_ctx *unit_len_ctx,
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
	awidth = css_unit_len2device_px(عدم, unit_len_ctx, a->عرض, a->unit);
	bwidth = css_unit_len2device_px(عدم, unit_len_ctx, b->عرض, b->unit);

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
	حالة BOX_جدول_ردیف:        impact++; fallthrough;
	حالة BOX_جدول_ردیف_گروه:  impact++; fallthrough;
	حالة BOX_جدول:            impact++; fallthrough;
	افتراضي:
		اكسر;
	}

	/** \todo COL/COL_GROUP */
	بدل (b_src) {
	حالة BOX_جدول_CELL:       impact--; fallthrough;
	حالة BOX_جدول_ردیف:        impact--; fallthrough;
	حالة BOX_جدول_ردیف_گروه:  impact--; fallthrough;
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
جدول_cell_بالاترین_process_جدول(ثابت css_unit_ctx *unit_len_ctx,
			     بنية box *جدول,
			     بنية border *a,
			     box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_بالاترین_style(جدول->style);
	b.color = css_computed_border_بالاترین_color(جدول->style, &b.c);
	css_computed_border_بالاترین_عرض(جدول->style, &b.عرض, &b.unit);
	b.عرض = css_unit_len2device_px(جدول->style, unit_len_ctx,
			b.عرض, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول;

	إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
جدول_cell_بالاترین_process_ردیف(ثابت css_unit_ctx *unit_len_ctx,
			   بنية box *cell,
			   بنية box *ردیف,
			   بنية border *a,
			   box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(ردیف->style);
	b.color = css_computed_border_bottom_color(ردیف->style, &b.c);
	css_computed_border_bottom_عرض(ردیف->style, &b.عرض, &b.unit);
	b.عرض = css_unit_len2device_px(ردیف->style, unit_len_ctx,
			b.عرض, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_ردیف;

	إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	إذا (ردیف->children == عدم) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_بالاترین_style(ردیف->style);
		b.color = css_computed_border_بالاترین_color(ردیف->style, &b.c);
		css_computed_border_بالاترین_عرض(ردیف->style, &b.عرض, &b.unit);
		b.عرض = css_unit_len2device_px(ردیف->style, unit_len_ctx,
				b.عرض, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_ردیف;

		إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
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
			لكل (c = ردیف->children; c != عدم; c = c->بعدی) {
				/* Ignore cells to the left */
				إذا (c->شروع_ستونی + c->columns - 1 <
				    cell->شروع_ستونی)
					استمر;
				/* Ignore cells to the right */
				إذا (c->شروع_ستونی > cell->شروع_ستونی +
				    cell->columns - 1)
					استمر;

				/* Flag that we've processed a cell */
				processed = true;

				/* Consider bottom border */
				b.style = css_computed_border_bottom_style(
							c->style);
				b.color = css_computed_border_bottom_color(
							c->style, &b.c);
				css_computed_border_bottom_عرض(c->style,
							&b.عرض, &b.unit);
				b.عرض = css_unit_len2device_px(
						c->style, unit_len_ctx,
						b.عرض, b.unit);
				b.unit = CSS_UNIT_PX;
				b_src = BOX_جدول_CELL;

				إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
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
				أكد(ردیف->prev != عدم);

				ردیف = ردیف->prev;
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
جدول_cell_بالاترین_process_گروه(ثابت css_unit_ctx *unit_len_ctx,
			     بنية box *cell,
			     بنية box *گروه,
			     بنية border *a,
			     box_type *a_src)
{
	بنية border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(گروه->style);
	b.color = css_computed_border_bottom_color(گروه->style, &b.c);
	css_computed_border_bottom_عرض(گروه->style, &b.عرض, &b.unit);
	b.عرض = css_unit_len2device_px(گروه->style, unit_len_ctx,
			b.عرض, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_ردیف_گروه;

	إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	إذا (گروه->آخرین != عدم) {
		/* Process rows in group, starting with last */
		بنية box *ردیف = گروه->آخرین;

		طالما (جدول_cell_بالاترین_process_ردیف(unit_len_ctx, cell, ردیف,
						  a, a_src) == false) {
			إذا (ردیف->prev == عدم) {
				أرجع false;
			} وإلا {
				ردیف = ردیف->prev;
			}
		}
	} وإلا {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_بالاترین_style(گروه->style);
		b.color = css_computed_border_بالاترین_color(گروه->style, &b.c);
		css_computed_border_بالاترین_عرض(گروه->style, &b.عرض, &b.unit);
		b.عرض = css_unit_len2device_px(گروه->style, unit_len_ctx,
				b.عرض, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_ردیف_گروه;

		إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
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
جدول_استفاده_شده_چپ_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_چپ_style(cell->style);
	a.color = css_computed_border_چپ_color(cell->style, &a.c);
	css_computed_border_چپ_عرض(cell->style, &a.عرض, &a.unit);
	a.عرض = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.عرض, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	إذا (cell->prev != عدم || cell->شروع_ستونی != 0) {
		/* Cell to the left -- consider its right border */
		بنية box *prev = عدم;

		إذا (cell->prev == عدم) {
			بنية box *ردیف;

			/* Spanned from a previous row in current row group */
			لكل (ردیف = cell->والد; ردیف != عدم; ردیف = ردیف->prev) {
				لكل (prev = ردیف->children; prev != عدم;
				     prev = prev->بعدی) {
					إذا (prev->شروع_ستونی +
					    prev->columns ==
					    cell->شروع_ستونی)
						اكسر;
				}

				إذا (prev != عدم)
					اكسر;
			}

			أكد(prev != عدم);
		} وإلا {
			prev = cell->prev;
		}

		b.style = css_computed_border_راست_style(prev->style);
		b.color = css_computed_border_راست_color(prev->style, &b.c);
		css_computed_border_راست_عرض(prev->style, &b.عرض, &b.unit);
		b.عرض = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.عرض, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_CELL;

		إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} وإلا {
		/* First cell in row, so consider rows and row group */
		بنية box *ردیف = cell->والد;
		بنية box *گروه = ردیف->والد;
		بنية box *جدول = گروه->والد;
		بیعلامت صحيح ردیفها = cell->ردیفها;

		طالما (ردیفها-- > 0 && ردیف != عدم) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_چپ_style(ردیف->style);
			b.color = css_computed_border_چپ_color(
								 ردیف->style, &b.c);
			css_computed_border_چپ_عرض(
						       ردیف->style, &b.عرض, &b.unit);
			b.عرض = css_unit_len2device_px(
					ردیف->style, unit_len_ctx,
					b.عرض, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_جدول_ردیف;

			إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			ردیف = ردیف->بعدی;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_چپ_style(گروه->style);
		b.color = css_computed_border_چپ_color(گروه->style, &b.c);
		css_computed_border_چپ_عرض(گروه->style, &b.عرض, &b.unit);
		b.عرض = css_unit_len2device_px(گروه->style, unit_len_ctx,
				b.عرض, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_ردیف_گروه;

		إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_چپ_style(جدول->style);
		b.color = css_computed_border_چپ_color(جدول->style, &b.c);
		css_computed_border_چپ_عرض(جدول->style, &b.عرض, &b.unit);
		b.عرض = css_unit_len2device_px(جدول->style, unit_len_ctx,
				b.عرض, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول;

		إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[چپ_2].style = a.style;
	cell->border[چپ_2].c = a.c;
	cell->border[چپ_2].عرض = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.عرض, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
جدول_استفاده_شده_بالاترین_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;
	بنية box *ردیف = cell->والد;
	bool process_گروه = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_بالاترین_style(cell->style);
	css_computed_border_بالاترین_color(cell->style, &a.c);
	css_computed_border_بالاترین_عرض(cell->style, &a.عرض, &a.unit);
	a.عرض = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.عرض, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	/* Top border of row */
	b.style = css_computed_border_بالاترین_style(ردیف->style);
	css_computed_border_بالاترین_color(ردیف->style, &b.c);
	css_computed_border_بالاترین_عرض(ردیف->style, &b.عرض, &b.unit);
	b.عرض = css_unit_len2device_px(ردیف->style, unit_len_ctx,
			b.عرض, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_جدول_ردیف;

	إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	إذا (ردیف->prev != عدم) {
		/* Consider row(s) above */
		طالما (جدول_cell_بالاترین_process_ردیف(unit_len_ctx, cell, ردیف->prev,
						  &a, &a_src) == false) {
			إذا (ردیف->prev->prev == عدم) {
				/* Consider row group */
				process_گروه = true;
				اكسر;
			} وإلا {
				ردیف = ردیف->prev;
			}
		}
	} وإلا {
		process_گروه = true;
	}

	إذا (process_گروه) {
		بنية box *گروه = ردیف->والد;

		/* Top border of row group */
		b.style = css_computed_border_بالاترین_style(گروه->style);
		b.color = css_computed_border_بالاترین_color(گروه->style, &b.c);
		css_computed_border_بالاترین_عرض(گروه->style, &b.عرض, &b.unit);
		b.عرض = css_unit_len2device_px(گروه->style, unit_len_ctx,
				b.عرض, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_ردیف_گروه;

		إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		إذا (گروه->prev == عدم) {
			/* Top border of table */
			جدول_cell_بالاترین_process_جدول(unit_len_ctx,
						     گروه->والد, &a, &a_src);
		} وإلا {
			/* Process previous group(s) */
			طالما (جدول_cell_بالاترین_process_گروه(unit_len_ctx,
							    cell, گروه->prev,
							    &a, &a_src) == false) {
				إذا (گروه->prev->prev == عدم) {
					/* Top border of table */
					جدول_cell_بالاترین_process_جدول(unit_len_ctx,
								     گروه->والد,
								     &a, &a_src);
					اكسر;
				} وإلا {
					گروه = گروه->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[بالاترین_2].style = a.style;
	cell->border[بالاترین_2].c = a.c;
	cell->border[بالاترین_2].عرض = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.عرض, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
جدول_استفاده_شده_راست_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_راست_style(cell->style);
	css_computed_border_راست_color(cell->style, &a.c);
	css_computed_border_راست_عرض(cell->style, &a.عرض, &a.unit);
	a.عرض = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.عرض, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	إذا (cell->بعدی != عدم || cell->شروع_ستونی + cell->columns !=
	    cell->والد->والد->والد->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.عرض = 0;
		a.unit = CSS_UNIT_PX;
	} وإلا {
		/* Last cell in row, so consider rows and row group */
		بنية box *ردیف = cell->والد;
		بنية box *گروه = ردیف->والد;
		بنية box *جدول = گروه->والد;
		بیعلامت صحيح ردیفها = cell->ردیفها;

		طالما (ردیفها-- > 0 && ردیف != عدم) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_راست_style(ردیف->style);
			b.color = css_computed_border_راست_color(ردیف->style,
								  &b.c);
			css_computed_border_راست_عرض(ردیف->style,
							&b.عرض,
							&b.unit);
			b.عرض = css_unit_len2device_px(
					ردیف->style, unit_len_ctx,
					b.عرض, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_جدول_ردیف;

			إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			ردیف = ردیف->بعدی;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_راست_style(گروه->style);
		b.color = css_computed_border_راست_color(گروه->style, &b.c);
		css_computed_border_راست_عرض(گروه->style,
						&b.عرض, &b.unit);
		b.عرض = css_unit_len2device_px(گروه->style, unit_len_ctx,
				b.عرض, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_ردیف_گروه;

		إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_راست_style(جدول->style);
		b.color = css_computed_border_راست_color(جدول->style, &b.c);
		css_computed_border_راست_عرض(جدول->style,
						&b.عرض, &b.unit);
		b.عرض = css_unit_len2device_px(جدول->style, unit_len_ctx,
				b.عرض, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول;

		إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[راست_2].style = a.style;
	cell->border[راست_2].c = a.c;
	cell->border[راست_2].عرض = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.عرض, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ساكن فراغ
جدول_استفاده_شده_bottom_border_for_cell(ثابت css_unit_ctx *unit_len_ctx,
				  بنية box *cell)
{
	بنية border a, b;
	box_type a_src, b_src;
	بنية box *ردیف = cell->والد;
	بیعلامت صحيح ردیفها = cell->ردیفها;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_عرض(cell->style, &a.عرض, &a.unit);
	a.عرض = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.عرض, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_جدول_CELL;

	طالما (ردیفها-- > 0 && ردیف != عدم)
		ردیف = ردیف->بعدی;

	/** \todo Can cells span row groups? */

	إذا (ردیف != عدم) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.عرض = 0;
		a.unit = CSS_UNIT_PX;
	} وإلا {
		/* Cell at bottom of table, so consider row and row group */
		بنية box *ردیف = cell->والد;
		بنية box *گروه = ردیف->والد;
		بنية box *جدول = گروه->والد;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(ردیف->style);
		b.color = css_computed_border_bottom_color(ردیف->style, &b.c);
		css_computed_border_bottom_عرض(ردیف->style, &b.عرض, &b.unit);
		b.عرض = css_unit_len2device_px(ردیف->style, unit_len_ctx,
				b.عرض, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_ردیف;

		إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* Row group -- consider its bottom border */
		b.style = css_computed_border_bottom_style(گروه->style);
		b.color = css_computed_border_bottom_color(گروه->style, &b.c);
		css_computed_border_bottom_عرض(گروه->style,
						 &b.عرض, &b.unit);
		b.عرض = css_unit_len2device_px(گروه->style, unit_len_ctx,
				b.عرض, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول_ردیف_گروه;

		إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its bottom border */
		b.style = css_computed_border_bottom_style(جدول->style);
		b.color = css_computed_border_bottom_color(جدول->style, &b.c);
		css_computed_border_bottom_عرض(جدول->style,
						 &b.عرض, &b.unit);
		b.عرض = css_unit_len2device_px(جدول->style, unit_len_ctx,
				b.عرض, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_جدول;

		إذا (جدول_border_is_بیشتر_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
		}
	}

	/* a now contains the used bottom border for the cell */
	cell->border[BOTTOM].style = a.style;
	cell->border[BOTTOM].c = a.c;
	cell->border[BOTTOM].عرض = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.عرض, a.unit));
}


/* exported interface documented in html/table.h */
bool
جدول_calculate_ستونی_types(ثابت css_unit_ctx *unit_len_ctx, بنية box *جدول)
{
	بیعلامت صحيح i, j;
	بنية ستونی *ستون;
	بنية box *ردیف_گروه, *ردیف, *cell;

	إذا (جدول->ستون)
		/* table->col already constructed, for example frameset table */
		أرجع true;

	جدول->ستون = ستون = talloc_array(جدول, بنية ستونی, جدول->columns);
	إذا (!ستون)
		أرجع false;

	لكل (i = 0; i != جدول->columns; i++) {
		ستون[i].type = ستونی_عرض_UNKNOWN;
		ستون[i].عرض = 0;
		ستون[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	لكل (ردیف_گروه = جدول->children; ردیف_گروه; ردیف_گروه =ردیف_گروه->بعدی)
		لكل (ردیف = ردیف_گروه->children; ردیف; ردیف = ردیف->بعدی)
			لكل (cell = ردیف->children; cell; cell = cell->بعدی) {
				تعداد css_عرض_e type;
				css_fixed مقدار = 0;
				css_unit unit = CSS_UNIT_PX;

				أكد(cell->type == BOX_جدول_CELL);
				أكد(cell->style);

				إذا (cell->columns != 1)
					استمر;
				i = cell->شروع_ستونی;

				إذا (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					ستون[i].positioned = false;
				}

				type = css_computed_عرض(cell->style, &مقدار, &unit);

				/* fixed width takes priority over any other width type */
				إذا (ستون[i].type != ستونی_عرض_FIXED &&
				    type == CSS_عرض_مجموعه && unit != CSS_UNIT_PCT) {
					ستون[i].type = ستونی_عرض_FIXED;
					ستون[i].عرض = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							مقدار, unit));
					إذا (ستون[i].عرض < 0)
						ستون[i].عرض = 0;
					استمر;
				}

				إذا (ستون[i].type != ستونی_عرض_UNKNOWN)
					استمر;

				إذا (type == CSS_عرض_مجموعه && unit == CSS_UNIT_PCT) {
					ستون[i].type = ستونی_عرض_PERCENT;
					ستون[i].عرض = FIXTOINT(مقدار);
					إذا (ستون[i].عرض < 0)
						ستون[i].عرض = 0;
				} وإلا إذا (type == CSS_عرض_AUTO) {
					ستون[i].type = ستونی_عرض_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	لكل (ردیف_گروه = جدول->children; ردیف_گروه; ردیف_گروه =ردیف_گروه->بعدی)
		لكل (ردیف = ردیف_گروه->children; ردیف; ردیف = ردیف->بعدی)
			لكل (cell = ردیف->children; cell; cell = cell->بعدی) {
				بیعلامت صحيح fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				صحيح fixed_عرض = 0, percent_عرض = 0;
				تعداد css_عرض_e type;
				css_fixed مقدار = 0;
				css_unit unit = CSS_UNIT_PX;

				إذا (cell->columns == 1)
					استمر;
				i = cell->شروع_ستونی;

				لكل (j = i; j < i + cell->columns; j++) {
					ستون[j].positioned = false;
				}

				/* count column types in spanned cells */
				لكل (j = 0; j != cell->columns; j++) {
					إذا (ستون[i + j].type == ستونی_عرض_FIXED) {
						fixed_عرض += ستون[i + j].عرض;
						fixed_columns++;
					} وإلا إذا (ستون[i + j].type == ستونی_عرض_PERCENT) {
						percent_عرض += ستون[i + j].عرض;
						percent_columns++;
					} وإلا إذا (ستون[i + j].type == ستونی_عرض_AUTO) {
						auto_columns++;
					} وإلا {
						unknown_columns++;
					}
				}

				إذا (!unknown_columns)
					استمر;

				type = css_computed_عرض(cell->style, &مقدار, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				إذا (type == CSS_عرض_مجموعه && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					صحيح عرض = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							مقدار, unit)) -
						fixed_عرض) / unknown_columns;
					إذا (عرض < 0)
						عرض = 0;
					لكل (j = 0; j != cell->columns; j++) {
						إذا (ستون[i + j].type == ستونی_عرض_UNKNOWN) {
							ستون[i + j].type = ستونی_عرض_FIXED;
							ستون[i + j].عرض = عرض;
						}
					}
				}

				/* as above for percentage width */
				إذا (type == CSS_عرض_مجموعه && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					صحيح عرض = (FIXTOFLT(مقدار) -
						     percent_عرض) / unknown_columns;
					إذا (عرض < 0)
						عرض = 0;
					لكل (j = 0; j != cell->columns; j++) {
						إذا (ستون[i + j].type == ستونی_عرض_UNKNOWN) {
							ستون[i + j].type = ستونی_عرض_PERCENT;
							ستون[i + j].عرض = عرض;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	لكل (i = 0; i != جدول->columns; i++) {
		إذا (ستون[i].type == ستونی_عرض_UNKNOWN)
			ستون[i].type = ستونی_عرض_AUTO;
	}

#اگر_تعریف_شده جدول_DEBUG
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
		      ستون[i].عرض);
#پایان_اگر

	أرجع true;
}


/* exported interface documented in html/table.h */
فراغ جدول_استفاده_شده_border_for_cell(ثابت css_unit_ctx *unit_len_ctx, بنية box *cell)
{
	صحيح side;

	أكد(cell->type == BOX_جدول_CELL);

	إذا (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed عرض = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[چپ_2].style =
			css_computed_border_چپ_style(cell->style);
		css_computed_border_چپ_color(cell->style,
					       &cell->border[چپ_2].c);
		css_computed_border_چپ_عرض(cell->style, &عرض, &unit);
		cell->border[چپ_2].عرض =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					عرض, unit));

		/* Top border */
		cell->border[بالاترین_2].style =
			css_computed_border_بالاترین_style(cell->style);
		css_computed_border_بالاترین_color(cell->style,
					      &cell->border[بالاترین_2].c);
		css_computed_border_بالاترین_عرض(cell->style, &عرض, &unit);
		cell->border[بالاترین_2].عرض =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					عرض, unit));

		/* Right border */
		cell->border[راست_2].style =
			css_computed_border_راست_style(cell->style);
		css_computed_border_راست_color(cell->style,
						&cell->border[راست_2].c);
		css_computed_border_راست_عرض(cell->style, &عرض, &unit);
		cell->border[راست_2].عرض =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					عرض, unit));

		/* Bottom border */
		cell->border[BOTTOM].style =
			css_computed_border_bottom_style(cell->style);
		css_computed_border_bottom_color(cell->style,
						 &cell->border[BOTTOM].c);
		css_computed_border_bottom_عرض(cell->style, &عرض, &unit);
		cell->border[BOTTOM].عرض =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					عرض, unit));
	} وإلا {
		/* Left border */
		جدول_استفاده_شده_چپ_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		جدول_استفاده_شده_بالاترین_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		جدول_استفاده_شده_راست_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		جدول_استفاده_شده_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	لكل (side = 0; side != 4; side++) {
		إذا (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].عرض = 0;
	}
}
