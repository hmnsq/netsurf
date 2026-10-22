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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/talloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/utils.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/table.h"

/* Define to enable verbose table debug */
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਸਾਰਣੀ_DEBUG

/**
 * Container for border values during table border calculations
 */
ਬਣਤਰ border {
	ਗਿਣਤੀ css_border_style_e style;	/**< border-style */
	ਗਿਣਤੀ css_border_color_e color;	/**< border-color type */
	css_color c;			/**< border-color value */
	css_fixed ਚੌੜਾਈ;		/**< border-width length */
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
ਸਥਿਰ bool
ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(ਅਚਲ css_unit_ctx *unit_len_ctx,
				 ਅਚਲ ਬਣਤਰ border *a,
				 box_type a_src,
				 ਅਚਲ ਬਣਤਰ border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	ਪੂਰਨ impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	ਜੇ (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		ਵਾਪਸ false;

	ਜੇ (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		ਵਾਪਸ true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	ਪ੍ਰਤਿਜ੍ਞਾ(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	ਪ੍ਰਤਿਜ੍ਞਾ(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(ਨਿਰਾ, unit_len_ctx, a->ਚੌੜਾਈ, a->unit);
	bwidth = css_unit_len2device_px(ਨਿਰਾ, unit_len_ctx, b->ਚੌੜਾਈ, b->unit);

	ਜੇ (awidth < bwidth)
		ਵਾਪਸ true;
	ਨਹੀਂਤਾਂ ਜੇ (bwidth < awidth)
		ਵਾਪਸ false;

	/* 3b -- sort by style */
	ਚੋਣ (a->style) {
	ਹਾਲਤ CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	ਚੋਣ (b->style) {
	ਹਾਲਤ CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	ਜੇ (impact < 0)
		ਵਾਪਸ true;
	ਨਹੀਂਤਾਂ ਜੇ (impact > 0)
		ਵਾਪਸ false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	ਚੋਣ (a_src) {
	ਹਾਲਤ BOX_ਸਾਰਣੀ_CELL:       impact++; fallthrough;
	ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ:        impact++; fallthrough;
	ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ:  impact++; fallthrough;
	ਹਾਲਤ BOX_ਸਾਰਣੀ:            impact++; fallthrough;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/** \todo COL/COL_GROUP */
	ਚੋਣ (b_src) {
	ਹਾਲਤ BOX_ਸਾਰਣੀ_CELL:       impact--; fallthrough;
	ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ:        impact--; fallthrough;
	ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ:  impact--; fallthrough;
	ਹਾਲਤ BOX_ਸਾਰਣੀ:            impact--; fallthrough;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	ਜੇ (impact < 0)
		ਵਾਪਸ true;
	ਨਹੀਂਤਾਂ ਜੇ (impact > 0)
		ਵਾਪਸ false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	ਵਾਪਸ true;
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
ਸਥਿਰ ਖਾਲੀ
ਸਾਰਣੀ_cell_ਸਿਖਰ_process_ਸਾਰਣੀ(ਅਚਲ css_unit_ctx *unit_len_ctx,
			     ਬਣਤਰ box *ਸਾਰਣੀ,
			     ਬਣਤਰ border *a,
			     box_type *a_src)
{
	ਬਣਤਰ border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_ਸਿਖਰ_style(ਸਾਰਣੀ->style);
	b.color = css_computed_border_ਸਿਖਰ_color(ਸਾਰਣੀ->style, &b.c);
	css_computed_border_ਸਿਖਰ_ਚੌੜਾਈ(ਸਾਰਣੀ->style, &b.ਚੌੜਾਈ, &b.unit);
	b.ਚੌੜਾਈ = css_unit_len2device_px(ਸਾਰਣੀ->style, unit_len_ctx,
			b.ਚੌੜਾਈ, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_ਸਾਰਣੀ;

	ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
ਸਥਿਰ bool
ਸਾਰਣੀ_cell_ਸਿਖਰ_process_ਪੰਕਤੀ(ਅਚਲ css_unit_ctx *unit_len_ctx,
			   ਬਣਤਰ box *cell,
			   ਬਣਤਰ box *ਪੰਕਤੀ,
			   ਬਣਤਰ border *a,
			   box_type *a_src)
{
	ਬਣਤਰ border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(ਪੰਕਤੀ->style);
	b.color = css_computed_border_bottom_color(ਪੰਕਤੀ->style, &b.c);
	css_computed_border_bottom_ਚੌੜਾਈ(ਪੰਕਤੀ->style, &b.ਚੌੜਾਈ, &b.unit);
	b.ਚੌੜਾਈ = css_unit_len2device_px(ਪੰਕਤੀ->style, unit_len_ctx,
			b.ਚੌੜਾਈ, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ;

	ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	ਜੇ (ਪੰਕਤੀ->children == ਨਿਰਾ) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_ਸਿਖਰ_style(ਪੰਕਤੀ->style);
		b.color = css_computed_border_ਸਿਖਰ_color(ਪੰਕਤੀ->style, &b.c);
		css_computed_border_ਸਿਖਰ_ਚੌੜਾਈ(ਪੰਕਤੀ->style, &b.ਚੌੜਾਈ, &b.unit);
		b.ਚੌੜਾਈ = css_unit_len2device_px(ਪੰਕਤੀ->style, unit_len_ctx,
				b.ਚੌੜਾਈ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ;

		ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		ਵਾਪਸ false;
	} ਨਹੀਂਤਾਂ {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		ਬਣਤਰ box *c;
		bool processed = false;

		ਜਦੋਂ (processed == false) {
			ਲਈ (c = ਪੰਕਤੀ->children; c != ਨਿਰਾ; c = c->ਅਗਲਾ) {
				/* Ignore cells to the left */
				ਜੇ (c->ਸ਼ੁਰੂ_ਕਾਲਮ + c->columns - 1 <
				    cell->ਸ਼ੁਰੂ_ਕਾਲਮ)
					ਜਾਰੀ;
				/* Ignore cells to the right */
				ਜੇ (c->ਸ਼ੁਰੂ_ਕਾਲਮ > cell->ਸ਼ੁਰੂ_ਕਾਲਮ +
				    cell->columns - 1)
					ਜਾਰੀ;

				/* Flag that we've processed a cell */
				processed = true;

				/* Consider bottom border */
				b.style = css_computed_border_bottom_style(
							c->style);
				b.color = css_computed_border_bottom_color(
							c->style, &b.c);
				css_computed_border_bottom_ਚੌੜਾਈ(c->style,
							&b.ਚੌੜਾਈ, &b.unit);
				b.ਚੌੜਾਈ = css_unit_len2device_px(
						c->style, unit_len_ctx,
						b.ਚੌੜਾਈ, b.unit);
				b.unit = CSS_UNIT_PX;
				b_src = BOX_ਸਾਰਣੀ_CELL;

				ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			ਜੇ (processed == false) {
				/* There must be a preceding row */
				ਪ੍ਰਤਿਜ੍ਞਾ(ਪੰਕਤੀ->prev != ਨਿਰਾ);

				ਪੰਕਤੀ = ਪੰਕਤੀ->prev;
			}
		}
	}

	ਵਾਪਸ true;
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
ਸਥਿਰ bool
ਸਾਰਣੀ_cell_ਸਿਖਰ_process_ਸਮੂਹ(ਅਚਲ css_unit_ctx *unit_len_ctx,
			     ਬਣਤਰ box *cell,
			     ਬਣਤਰ box *ਸਮੂਹ,
			     ਬਣਤਰ border *a,
			     box_type *a_src)
{
	ਬਣਤਰ border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(ਸਮੂਹ->style);
	b.color = css_computed_border_bottom_color(ਸਮੂਹ->style, &b.c);
	css_computed_border_bottom_ਚੌੜਾਈ(ਸਮੂਹ->style, &b.ਚੌੜਾਈ, &b.unit);
	b.ਚੌੜਾਈ = css_unit_len2device_px(ਸਮੂਹ->style, unit_len_ctx,
			b.ਚੌੜਾਈ, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ;

	ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	ਜੇ (ਸਮੂਹ->ਆਖਰੀ != ਨਿਰਾ) {
		/* Process rows in group, starting with last */
		ਬਣਤਰ box *ਪੰਕਤੀ = ਸਮੂਹ->ਆਖਰੀ;

		ਜਦੋਂ (ਸਾਰਣੀ_cell_ਸਿਖਰ_process_ਪੰਕਤੀ(unit_len_ctx, cell, ਪੰਕਤੀ,
						  a, a_src) == false) {
			ਜੇ (ਪੰਕਤੀ->prev == ਨਿਰਾ) {
				ਵਾਪਸ false;
			} ਨਹੀਂਤਾਂ {
				ਪੰਕਤੀ = ਪੰਕਤੀ->prev;
			}
		}
	} ਨਹੀਂਤਾਂ {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_ਸਿਖਰ_style(ਸਮੂਹ->style);
		b.color = css_computed_border_ਸਿਖਰ_color(ਸਮੂਹ->style, &b.c);
		css_computed_border_ਸਿਖਰ_ਚੌੜਾਈ(ਸਮੂਹ->style, &b.ਚੌੜਾਈ, &b.unit);
		b.ਚੌੜਾਈ = css_unit_len2device_px(ਸਮੂਹ->style, unit_len_ctx,
				b.ਚੌੜਾਈ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ;

		ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		ਵਾਪਸ false;
	}

	ਵਾਪਸ true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ਸਥਿਰ ਖਾਲੀ
ਸਾਰਣੀ_ਵਰਤਿਆ_ਖੱਬਾ_border_for_cell(ਅਚਲ css_unit_ctx *unit_len_ctx, ਬਣਤਰ box *cell)
{
	ਬਣਤਰ border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_ਖੱਬਾ_style(cell->style);
	a.color = css_computed_border_ਖੱਬਾ_color(cell->style, &a.c);
	css_computed_border_ਖੱਬਾ_ਚੌੜਾਈ(cell->style, &a.ਚੌੜਾਈ, &a.unit);
	a.ਚੌੜਾਈ = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.ਚੌੜਾਈ, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_ਸਾਰਣੀ_CELL;

	ਜੇ (cell->prev != ਨਿਰਾ || cell->ਸ਼ੁਰੂ_ਕਾਲਮ != 0) {
		/* Cell to the left -- consider its right border */
		ਬਣਤਰ box *prev = ਨਿਰਾ;

		ਜੇ (cell->prev == ਨਿਰਾ) {
			ਬਣਤਰ box *ਪੰਕਤੀ;

			/* Spanned from a previous row in current row group */
			ਲਈ (ਪੰਕਤੀ = cell->ਮਾਪਾ; ਪੰਕਤੀ != ਨਿਰਾ; ਪੰਕਤੀ = ਪੰਕਤੀ->prev) {
				ਲਈ (prev = ਪੰਕਤੀ->children; prev != ਨਿਰਾ;
				     prev = prev->ਅਗਲਾ) {
					ਜੇ (prev->ਸ਼ੁਰੂ_ਕਾਲਮ +
					    prev->columns ==
					    cell->ਸ਼ੁਰੂ_ਕਾਲਮ)
						ਰੋਕੋ;
				}

				ਜੇ (prev != ਨਿਰਾ)
					ਰੋਕੋ;
			}

			ਪ੍ਰਤਿਜ੍ਞਾ(prev != ਨਿਰਾ);
		} ਨਹੀਂਤਾਂ {
			prev = cell->prev;
		}

		b.style = css_computed_border_ਸੱਜਾ_style(prev->style);
		b.color = css_computed_border_ਸੱਜਾ_color(prev->style, &b.c);
		css_computed_border_ਸੱਜਾ_ਚੌੜਾਈ(prev->style, &b.ਚੌੜਾਈ, &b.unit);
		b.ਚੌੜਾਈ = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.ਚੌੜਾਈ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ਸਾਰਣੀ_CELL;

		ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} ਨਹੀਂਤਾਂ {
		/* First cell in row, so consider rows and row group */
		ਬਣਤਰ box *ਪੰਕਤੀ = cell->ਮਾਪਾ;
		ਬਣਤਰ box *ਸਮੂਹ = ਪੰਕਤੀ->ਮਾਪਾ;
		ਬਣਤਰ box *ਸਾਰਣੀ = ਸਮੂਹ->ਮਾਪਾ;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੰਕਤੀਆਂ = cell->ਪੰਕਤੀਆਂ;

		ਜਦੋਂ (ਪੰਕਤੀਆਂ-- > 0 && ਪੰਕਤੀ != ਨਿਰਾ) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_ਖੱਬਾ_style(ਪੰਕਤੀ->style);
			b.color = css_computed_border_ਖੱਬਾ_color(
								 ਪੰਕਤੀ->style, &b.c);
			css_computed_border_ਖੱਬਾ_ਚੌੜਾਈ(
						       ਪੰਕਤੀ->style, &b.ਚੌੜਾਈ, &b.unit);
			b.ਚੌੜਾਈ = css_unit_len2device_px(
					ਪੰਕਤੀ->style, unit_len_ctx,
					b.ਚੌੜਾਈ, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ;

			ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			ਪੰਕਤੀ = ਪੰਕਤੀ->ਅਗਲਾ;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_ਖੱਬਾ_style(ਸਮੂਹ->style);
		b.color = css_computed_border_ਖੱਬਾ_color(ਸਮੂਹ->style, &b.c);
		css_computed_border_ਖੱਬਾ_ਚੌੜਾਈ(ਸਮੂਹ->style, &b.ਚੌੜਾਈ, &b.unit);
		b.ਚੌੜਾਈ = css_unit_len2device_px(ਸਮੂਹ->style, unit_len_ctx,
				b.ਚੌੜਾਈ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ;

		ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_ਖੱਬਾ_style(ਸਾਰਣੀ->style);
		b.color = css_computed_border_ਖੱਬਾ_color(ਸਾਰਣੀ->style, &b.c);
		css_computed_border_ਖੱਬਾ_ਚੌੜਾਈ(ਸਾਰਣੀ->style, &b.ਚੌੜਾਈ, &b.unit);
		b.ਚੌੜਾਈ = css_unit_len2device_px(ਸਾਰਣੀ->style, unit_len_ctx,
				b.ਚੌੜਾਈ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ਸਾਰਣੀ;

		ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[ਖੱਬਾ_2].style = a.style;
	cell->border[ਖੱਬਾ_2].c = a.c;
	cell->border[ਖੱਬਾ_2].ਚੌੜਾਈ = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.ਚੌੜਾਈ, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ਸਥਿਰ ਖਾਲੀ
ਸਾਰਣੀ_ਵਰਤਿਆ_ਸਿਖਰ_border_for_cell(ਅਚਲ css_unit_ctx *unit_len_ctx, ਬਣਤਰ box *cell)
{
	ਬਣਤਰ border a, b;
	box_type a_src, b_src;
	ਬਣਤਰ box *ਪੰਕਤੀ = cell->ਮਾਪਾ;
	bool process_ਸਮੂਹ = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_ਸਿਖਰ_style(cell->style);
	css_computed_border_ਸਿਖਰ_color(cell->style, &a.c);
	css_computed_border_ਸਿਖਰ_ਚੌੜਾਈ(cell->style, &a.ਚੌੜਾਈ, &a.unit);
	a.ਚੌੜਾਈ = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.ਚੌੜਾਈ, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_ਸਾਰਣੀ_CELL;

	/* Top border of row */
	b.style = css_computed_border_ਸਿਖਰ_style(ਪੰਕਤੀ->style);
	css_computed_border_ਸਿਖਰ_color(ਪੰਕਤੀ->style, &b.c);
	css_computed_border_ਸਿਖਰ_ਚੌੜਾਈ(ਪੰਕਤੀ->style, &b.ਚੌੜਾਈ, &b.unit);
	b.ਚੌੜਾਈ = css_unit_len2device_px(ਪੰਕਤੀ->style, unit_len_ctx,
			b.ਚੌੜਾਈ, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ;

	ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	ਜੇ (ਪੰਕਤੀ->prev != ਨਿਰਾ) {
		/* Consider row(s) above */
		ਜਦੋਂ (ਸਾਰਣੀ_cell_ਸਿਖਰ_process_ਪੰਕਤੀ(unit_len_ctx, cell, ਪੰਕਤੀ->prev,
						  &a, &a_src) == false) {
			ਜੇ (ਪੰਕਤੀ->prev->prev == ਨਿਰਾ) {
				/* Consider row group */
				process_ਸਮੂਹ = true;
				ਰੋਕੋ;
			} ਨਹੀਂਤਾਂ {
				ਪੰਕਤੀ = ਪੰਕਤੀ->prev;
			}
		}
	} ਨਹੀਂਤਾਂ {
		process_ਸਮੂਹ = true;
	}

	ਜੇ (process_ਸਮੂਹ) {
		ਬਣਤਰ box *ਸਮੂਹ = ਪੰਕਤੀ->ਮਾਪਾ;

		/* Top border of row group */
		b.style = css_computed_border_ਸਿਖਰ_style(ਸਮੂਹ->style);
		b.color = css_computed_border_ਸਿਖਰ_color(ਸਮੂਹ->style, &b.c);
		css_computed_border_ਸਿਖਰ_ਚੌੜਾਈ(ਸਮੂਹ->style, &b.ਚੌੜਾਈ, &b.unit);
		b.ਚੌੜਾਈ = css_unit_len2device_px(ਸਮੂਹ->style, unit_len_ctx,
				b.ਚੌੜਾਈ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ;

		ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		ਜੇ (ਸਮੂਹ->prev == ਨਿਰਾ) {
			/* Top border of table */
			ਸਾਰਣੀ_cell_ਸਿਖਰ_process_ਸਾਰਣੀ(unit_len_ctx,
						     ਸਮੂਹ->ਮਾਪਾ, &a, &a_src);
		} ਨਹੀਂਤਾਂ {
			/* Process previous group(s) */
			ਜਦੋਂ (ਸਾਰਣੀ_cell_ਸਿਖਰ_process_ਸਮੂਹ(unit_len_ctx,
							    cell, ਸਮੂਹ->prev,
							    &a, &a_src) == false) {
				ਜੇ (ਸਮੂਹ->prev->prev == ਨਿਰਾ) {
					/* Top border of table */
					ਸਾਰਣੀ_cell_ਸਿਖਰ_process_ਸਾਰਣੀ(unit_len_ctx,
								     ਸਮੂਹ->ਮਾਪਾ,
								     &a, &a_src);
					ਰੋਕੋ;
				} ਨਹੀਂਤਾਂ {
					ਸਮੂਹ = ਸਮੂਹ->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[ਸਿਖਰ_2].style = a.style;
	cell->border[ਸਿਖਰ_2].c = a.c;
	cell->border[ਸਿਖਰ_2].ਚੌੜਾਈ = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.ਚੌੜਾਈ, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ਸਥਿਰ ਖਾਲੀ
ਸਾਰਣੀ_ਵਰਤਿਆ_ਸੱਜਾ_border_for_cell(ਅਚਲ css_unit_ctx *unit_len_ctx, ਬਣਤਰ box *cell)
{
	ਬਣਤਰ border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_ਸੱਜਾ_style(cell->style);
	css_computed_border_ਸੱਜਾ_color(cell->style, &a.c);
	css_computed_border_ਸੱਜਾ_ਚੌੜਾਈ(cell->style, &a.ਚੌੜਾਈ, &a.unit);
	a.ਚੌੜਾਈ = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.ਚੌੜਾਈ, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_ਸਾਰਣੀ_CELL;

	ਜੇ (cell->ਅਗਲਾ != ਨਿਰਾ || cell->ਸ਼ੁਰੂ_ਕਾਲਮ + cell->columns !=
	    cell->ਮਾਪਾ->ਮਾਪਾ->ਮਾਪਾ->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.ਚੌੜਾਈ = 0;
		a.unit = CSS_UNIT_PX;
	} ਨਹੀਂਤਾਂ {
		/* Last cell in row, so consider rows and row group */
		ਬਣਤਰ box *ਪੰਕਤੀ = cell->ਮਾਪਾ;
		ਬਣਤਰ box *ਸਮੂਹ = ਪੰਕਤੀ->ਮਾਪਾ;
		ਬਣਤਰ box *ਸਾਰਣੀ = ਸਮੂਹ->ਮਾਪਾ;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੰਕਤੀਆਂ = cell->ਪੰਕਤੀਆਂ;

		ਜਦੋਂ (ਪੰਕਤੀਆਂ-- > 0 && ਪੰਕਤੀ != ਨਿਰਾ) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_ਸੱਜਾ_style(ਪੰਕਤੀ->style);
			b.color = css_computed_border_ਸੱਜਾ_color(ਪੰਕਤੀ->style,
								  &b.c);
			css_computed_border_ਸੱਜਾ_ਚੌੜਾਈ(ਪੰਕਤੀ->style,
							&b.ਚੌੜਾਈ,
							&b.unit);
			b.ਚੌੜਾਈ = css_unit_len2device_px(
					ਪੰਕਤੀ->style, unit_len_ctx,
					b.ਚੌੜਾਈ, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ;

			ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			ਪੰਕਤੀ = ਪੰਕਤੀ->ਅਗਲਾ;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_ਸੱਜਾ_style(ਸਮੂਹ->style);
		b.color = css_computed_border_ਸੱਜਾ_color(ਸਮੂਹ->style, &b.c);
		css_computed_border_ਸੱਜਾ_ਚੌੜਾਈ(ਸਮੂਹ->style,
						&b.ਚੌੜਾਈ, &b.unit);
		b.ਚੌੜਾਈ = css_unit_len2device_px(ਸਮੂਹ->style, unit_len_ctx,
				b.ਚੌੜਾਈ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ;

		ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_ਸੱਜਾ_style(ਸਾਰਣੀ->style);
		b.color = css_computed_border_ਸੱਜਾ_color(ਸਾਰਣੀ->style, &b.c);
		css_computed_border_ਸੱਜਾ_ਚੌੜਾਈ(ਸਾਰਣੀ->style,
						&b.ਚੌੜਾਈ, &b.unit);
		b.ਚੌੜਾਈ = css_unit_len2device_px(ਸਾਰਣੀ->style, unit_len_ctx,
				b.ਚੌੜਾਈ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ਸਾਰਣੀ;

		ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[ਸੱਜਾ_2].style = a.style;
	cell->border[ਸੱਜਾ_2].c = a.c;
	cell->border[ਸੱਜਾ_2].ਚੌੜਾਈ = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.ਚੌੜਾਈ, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ਸਥਿਰ ਖਾਲੀ
ਸਾਰਣੀ_ਵਰਤਿਆ_bottom_border_for_cell(ਅਚਲ css_unit_ctx *unit_len_ctx,
				  ਬਣਤਰ box *cell)
{
	ਬਣਤਰ border a, b;
	box_type a_src, b_src;
	ਬਣਤਰ box *ਪੰਕਤੀ = cell->ਮਾਪਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੰਕਤੀਆਂ = cell->ਪੰਕਤੀਆਂ;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_ਚੌੜਾਈ(cell->style, &a.ਚੌੜਾਈ, &a.unit);
	a.ਚੌੜਾਈ = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.ਚੌੜਾਈ, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_ਸਾਰਣੀ_CELL;

	ਜਦੋਂ (ਪੰਕਤੀਆਂ-- > 0 && ਪੰਕਤੀ != ਨਿਰਾ)
		ਪੰਕਤੀ = ਪੰਕਤੀ->ਅਗਲਾ;

	/** \todo Can cells span row groups? */

	ਜੇ (ਪੰਕਤੀ != ਨਿਰਾ) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.ਚੌੜਾਈ = 0;
		a.unit = CSS_UNIT_PX;
	} ਨਹੀਂਤਾਂ {
		/* Cell at bottom of table, so consider row and row group */
		ਬਣਤਰ box *ਪੰਕਤੀ = cell->ਮਾਪਾ;
		ਬਣਤਰ box *ਸਮੂਹ = ਪੰਕਤੀ->ਮਾਪਾ;
		ਬਣਤਰ box *ਸਾਰਣੀ = ਸਮੂਹ->ਮਾਪਾ;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(ਪੰਕਤੀ->style);
		b.color = css_computed_border_bottom_color(ਪੰਕਤੀ->style, &b.c);
		css_computed_border_bottom_ਚੌੜਾਈ(ਪੰਕਤੀ->style, &b.ਚੌੜਾਈ, &b.unit);
		b.ਚੌੜਾਈ = css_unit_len2device_px(ਪੰਕਤੀ->style, unit_len_ctx,
				b.ਚੌੜਾਈ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ;

		ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* Row group -- consider its bottom border */
		b.style = css_computed_border_bottom_style(ਸਮੂਹ->style);
		b.color = css_computed_border_bottom_color(ਸਮੂਹ->style, &b.c);
		css_computed_border_bottom_ਚੌੜਾਈ(ਸਮੂਹ->style,
						 &b.ਚੌੜਾਈ, &b.unit);
		b.ਚੌੜਾਈ = css_unit_len2device_px(ਸਮੂਹ->style, unit_len_ctx,
				b.ਚੌੜਾਈ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ;

		ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its bottom border */
		b.style = css_computed_border_bottom_style(ਸਾਰਣੀ->style);
		b.color = css_computed_border_bottom_color(ਸਾਰਣੀ->style, &b.c);
		css_computed_border_bottom_ਚੌੜਾਈ(ਸਾਰਣੀ->style,
						 &b.ਚੌੜਾਈ, &b.unit);
		b.ਚੌੜਾਈ = css_unit_len2device_px(ਸਾਰਣੀ->style, unit_len_ctx,
				b.ਚੌੜਾਈ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ਸਾਰਣੀ;

		ਜੇ (ਸਾਰਣੀ_border_is_ਹੋਰ_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
		}
	}

	/* a now contains the used bottom border for the cell */
	cell->border[BOTTOM].style = a.style;
	cell->border[BOTTOM].c = a.c;
	cell->border[BOTTOM].ਚੌੜਾਈ = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.ਚੌੜਾਈ, a.unit));
}


/* exported interface documented in html/table.h */
bool
ਸਾਰਣੀ_calculate_ਕਾਲਮ_types(ਅਚਲ css_unit_ctx *unit_len_ctx, ਬਣਤਰ box *ਸਾਰਣੀ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i, j;
	ਬਣਤਰ ਕਾਲਮ *ਖਾਨਾ;
	ਬਣਤਰ box *ਪੰਕਤੀ_ਸਮੂਹ, *ਪੰਕਤੀ, *cell;

	ਜੇ (ਸਾਰਣੀ->ਖਾਨਾ)
		/* table->col already constructed, for example frameset table */
		ਵਾਪਸ true;

	ਸਾਰਣੀ->ਖਾਨਾ = ਖਾਨਾ = talloc_array(ਸਾਰਣੀ, ਬਣਤਰ ਕਾਲਮ, ਸਾਰਣੀ->columns);
	ਜੇ (!ਖਾਨਾ)
		ਵਾਪਸ false;

	ਲਈ (i = 0; i != ਸਾਰਣੀ->columns; i++) {
		ਖਾਨਾ[i].type = ਕਾਲਮ_ਚੌੜਾਈ_UNKNOWN;
		ਖਾਨਾ[i].ਚੌੜਾਈ = 0;
		ਖਾਨਾ[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	ਲਈ (ਪੰਕਤੀ_ਸਮੂਹ = ਸਾਰਣੀ->children; ਪੰਕਤੀ_ਸਮੂਹ; ਪੰਕਤੀ_ਸਮੂਹ =ਪੰਕਤੀ_ਸਮੂਹ->ਅਗਲਾ)
		ਲਈ (ਪੰਕਤੀ = ਪੰਕਤੀ_ਸਮੂਹ->children; ਪੰਕਤੀ; ਪੰਕਤੀ = ਪੰਕਤੀ->ਅਗਲਾ)
			ਲਈ (cell = ਪੰਕਤੀ->children; cell; cell = cell->ਅਗਲਾ) {
				ਗਿਣਤੀ css_ਚੌੜਾਈ_e type;
				css_fixed ਮੁੱਲ = 0;
				css_unit unit = CSS_UNIT_PX;

				ਪ੍ਰਤਿਜ੍ਞਾ(cell->type == BOX_ਸਾਰਣੀ_CELL);
				ਪ੍ਰਤਿਜ੍ਞਾ(cell->style);

				ਜੇ (cell->columns != 1)
					ਜਾਰੀ;
				i = cell->ਸ਼ੁਰੂ_ਕਾਲਮ;

				ਜੇ (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					ਖਾਨਾ[i].positioned = false;
				}

				type = css_computed_ਚੌੜਾਈ(cell->style, &ਮੁੱਲ, &unit);

				/* fixed width takes priority over any other width type */
				ਜੇ (ਖਾਨਾ[i].type != ਕਾਲਮ_ਚੌੜਾਈ_FIXED &&
				    type == CSS_ਚੌੜਾਈ_ਸੈੱਟ && unit != CSS_UNIT_PCT) {
					ਖਾਨਾ[i].type = ਕਾਲਮ_ਚੌੜਾਈ_FIXED;
					ਖਾਨਾ[i].ਚੌੜਾਈ = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							ਮੁੱਲ, unit));
					ਜੇ (ਖਾਨਾ[i].ਚੌੜਾਈ < 0)
						ਖਾਨਾ[i].ਚੌੜਾਈ = 0;
					ਜਾਰੀ;
				}

				ਜੇ (ਖਾਨਾ[i].type != ਕਾਲਮ_ਚੌੜਾਈ_UNKNOWN)
					ਜਾਰੀ;

				ਜੇ (type == CSS_ਚੌੜਾਈ_ਸੈੱਟ && unit == CSS_UNIT_PCT) {
					ਖਾਨਾ[i].type = ਕਾਲਮ_ਚੌੜਾਈ_PERCENT;
					ਖਾਨਾ[i].ਚੌੜਾਈ = FIXTOINT(ਮੁੱਲ);
					ਜੇ (ਖਾਨਾ[i].ਚੌੜਾਈ < 0)
						ਖਾਨਾ[i].ਚੌੜਾਈ = 0;
				} ਨਹੀਂਤਾਂ ਜੇ (type == CSS_ਚੌੜਾਈ_AUTO) {
					ਖਾਨਾ[i].type = ਕਾਲਮ_ਚੌੜਾਈ_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	ਲਈ (ਪੰਕਤੀ_ਸਮੂਹ = ਸਾਰਣੀ->children; ਪੰਕਤੀ_ਸਮੂਹ; ਪੰਕਤੀ_ਸਮੂਹ =ਪੰਕਤੀ_ਸਮੂਹ->ਅਗਲਾ)
		ਲਈ (ਪੰਕਤੀ = ਪੰਕਤੀ_ਸਮੂਹ->children; ਪੰਕਤੀ; ਪੰਕਤੀ = ਪੰਕਤੀ->ਅਗਲਾ)
			ਲਈ (cell = ਪੰਕਤੀ->children; cell; cell = cell->ਅਗਲਾ) {
				ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				ਪੂਰਨ fixed_ਚੌੜਾਈ = 0, percent_ਚੌੜਾਈ = 0;
				ਗਿਣਤੀ css_ਚੌੜਾਈ_e type;
				css_fixed ਮੁੱਲ = 0;
				css_unit unit = CSS_UNIT_PX;

				ਜੇ (cell->columns == 1)
					ਜਾਰੀ;
				i = cell->ਸ਼ੁਰੂ_ਕਾਲਮ;

				ਲਈ (j = i; j < i + cell->columns; j++) {
					ਖਾਨਾ[j].positioned = false;
				}

				/* count column types in spanned cells */
				ਲਈ (j = 0; j != cell->columns; j++) {
					ਜੇ (ਖਾਨਾ[i + j].type == ਕਾਲਮ_ਚੌੜਾਈ_FIXED) {
						fixed_ਚੌੜਾਈ += ਖਾਨਾ[i + j].ਚੌੜਾਈ;
						fixed_columns++;
					} ਨਹੀਂਤਾਂ ਜੇ (ਖਾਨਾ[i + j].type == ਕਾਲਮ_ਚੌੜਾਈ_PERCENT) {
						percent_ਚੌੜਾਈ += ਖਾਨਾ[i + j].ਚੌੜਾਈ;
						percent_columns++;
					} ਨਹੀਂਤਾਂ ਜੇ (ਖਾਨਾ[i + j].type == ਕਾਲਮ_ਚੌੜਾਈ_AUTO) {
						auto_columns++;
					} ਨਹੀਂਤਾਂ {
						unknown_columns++;
					}
				}

				ਜੇ (!unknown_columns)
					ਜਾਰੀ;

				type = css_computed_ਚੌੜਾਈ(cell->style, &ਮੁੱਲ, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				ਜੇ (type == CSS_ਚੌੜਾਈ_ਸੈੱਟ && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					ਪੂਰਨ ਚੌੜਾਈ = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							ਮੁੱਲ, unit)) -
						fixed_ਚੌੜਾਈ) / unknown_columns;
					ਜੇ (ਚੌੜਾਈ < 0)
						ਚੌੜਾਈ = 0;
					ਲਈ (j = 0; j != cell->columns; j++) {
						ਜੇ (ਖਾਨਾ[i + j].type == ਕਾਲਮ_ਚੌੜਾਈ_UNKNOWN) {
							ਖਾਨਾ[i + j].type = ਕਾਲਮ_ਚੌੜਾਈ_FIXED;
							ਖਾਨਾ[i + j].ਚੌੜਾਈ = ਚੌੜਾਈ;
						}
					}
				}

				/* as above for percentage width */
				ਜੇ (type == CSS_ਚੌੜਾਈ_ਸੈੱਟ && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					ਪੂਰਨ ਚੌੜਾਈ = (FIXTOFLT(ਮੁੱਲ) -
						     percent_ਚੌੜਾਈ) / unknown_columns;
					ਜੇ (ਚੌੜਾਈ < 0)
						ਚੌੜਾਈ = 0;
					ਲਈ (j = 0; j != cell->columns; j++) {
						ਜੇ (ਖਾਨਾ[i + j].type == ਕਾਲਮ_ਚੌੜਾਈ_UNKNOWN) {
							ਖਾਨਾ[i + j].type = ਕਾਲਮ_ਚੌੜਾਈ_PERCENT;
							ਖਾਨਾ[i + j].ਚੌੜਾਈ = ਚੌੜਾਈ;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	ਲਈ (i = 0; i != ਸਾਰਣੀ->columns; i++) {
		ਜੇ (ਖਾਨਾ[i].type == ਕਾਲਮ_ਚੌੜਾਈ_UNKNOWN)
			ਖਾਨਾ[i].type = ਕਾਲਮ_ਚੌੜਾਈ_AUTO;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ਸਾਰਣੀ_DEBUG
	ਲਈ (i = 0; i != ਸਾਰਣੀ->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      ਸਾਰਣੀ,
		      i,
		      ((ਅਚਲ ਅੱਖਰ *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[ਖਾਨਾ[i].type],
		      ਖਾਨਾ[i].ਚੌੜਾਈ);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ true;
}


/* exported interface documented in html/table.h */
ਖਾਲੀ ਸਾਰਣੀ_ਵਰਤਿਆ_border_for_cell(ਅਚਲ css_unit_ctx *unit_len_ctx, ਬਣਤਰ box *cell)
{
	ਪੂਰਨ side;

	ਪ੍ਰਤਿਜ੍ਞਾ(cell->type == BOX_ਸਾਰਣੀ_CELL);

	ਜੇ (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed ਚੌੜਾਈ = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[ਖੱਬਾ_2].style =
			css_computed_border_ਖੱਬਾ_style(cell->style);
		css_computed_border_ਖੱਬਾ_color(cell->style,
					       &cell->border[ਖੱਬਾ_2].c);
		css_computed_border_ਖੱਬਾ_ਚੌੜਾਈ(cell->style, &ਚੌੜਾਈ, &unit);
		cell->border[ਖੱਬਾ_2].ਚੌੜਾਈ =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					ਚੌੜਾਈ, unit));

		/* Top border */
		cell->border[ਸਿਖਰ_2].style =
			css_computed_border_ਸਿਖਰ_style(cell->style);
		css_computed_border_ਸਿਖਰ_color(cell->style,
					      &cell->border[ਸਿਖਰ_2].c);
		css_computed_border_ਸਿਖਰ_ਚੌੜਾਈ(cell->style, &ਚੌੜਾਈ, &unit);
		cell->border[ਸਿਖਰ_2].ਚੌੜਾਈ =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					ਚੌੜਾਈ, unit));

		/* Right border */
		cell->border[ਸੱਜਾ_2].style =
			css_computed_border_ਸੱਜਾ_style(cell->style);
		css_computed_border_ਸੱਜਾ_color(cell->style,
						&cell->border[ਸੱਜਾ_2].c);
		css_computed_border_ਸੱਜਾ_ਚੌੜਾਈ(cell->style, &ਚੌੜਾਈ, &unit);
		cell->border[ਸੱਜਾ_2].ਚੌੜਾਈ =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					ਚੌੜਾਈ, unit));

		/* Bottom border */
		cell->border[BOTTOM].style =
			css_computed_border_bottom_style(cell->style);
		css_computed_border_bottom_color(cell->style,
						 &cell->border[BOTTOM].c);
		css_computed_border_bottom_ਚੌੜਾਈ(cell->style, &ਚੌੜਾਈ, &unit);
		cell->border[BOTTOM].ਚੌੜਾਈ =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					ਚੌੜਾਈ, unit));
	} ਨਹੀਂਤਾਂ {
		/* Left border */
		ਸਾਰਣੀ_ਵਰਤਿਆ_ਖੱਬਾ_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		ਸਾਰਣੀ_ਵਰਤਿਆ_ਸਿਖਰ_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		ਸਾਰਣੀ_ਵਰਤਿਆ_ਸੱਜਾ_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		ਸਾਰਣੀ_ਵਰਤਿਆ_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	ਲਈ (side = 0; side != 4; side++) {
		ਜੇ (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].ਚੌੜਾਈ = 0;
	}
}
