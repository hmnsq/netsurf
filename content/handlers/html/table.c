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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/talloc.h"
#ಅನ್ತರ್ಭಾವ "css/utils.h"

#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/table.h"

/* Define to enable verbose table debug */
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಕೋಷ್ಟಕ_DEBUG

/**
 * Container for border values during table border calculations
 */
ರಚನೆ border {
	ಎಣಿಕೆ css_border_style_e style;	/**< border-style */
	ಎಣಿಕೆ css_border_color_e color;	/**< border-color type */
	css_color c;			/**< border-color value */
	css_fixed ಅಗಲ;		/**< border-width length */
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
ಸ್ಥಿರ bool
ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
				 ಸ್ಥಿರಾಂಕ ರಚನೆ border *a,
				 box_type a_src,
				 ಸ್ಥಿರಾಂಕ ರಚನೆ border *b,
				 box_type b_src)
{
	css_fixed awidth, bwidth;
	ಪೂರ್ಣಾಂಕ impact = 0;

	/* See CSS 2.1 $17.6.2.1 */

	/* 1 + 2 -- hidden beats everything, none beats nothing */
	ಆದರೆ (a->style == CSS_BORDER_STYLE_HIDDEN ||
	    b->style == CSS_BORDER_STYLE_NONE)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (b->style == CSS_BORDER_STYLE_HIDDEN ||
	    a->style == CSS_BORDER_STYLE_NONE)
		ಹಿಂತಿರುಗಿ true;

	/* 3a -- wider borders beat narrow ones */
	/* The widths must be absolute, which will be the case
	 * if they've come from a computed style. */
	ಪ್ರತಿಜ್ಞಾ(a->unit != CSS_UNIT_EM && a->unit != CSS_UNIT_EX);
	ಪ್ರತಿಜ್ಞಾ(b->unit != CSS_UNIT_EM && b->unit != CSS_UNIT_EX);
	awidth = css_unit_len2device_px(ಖಾಲಿ, unit_len_ctx, a->ಅಗಲ, a->unit);
	bwidth = css_unit_len2device_px(ಖಾಲಿ, unit_len_ctx, b->ಅಗಲ, b->unit);

	ಆದರೆ (awidth < bwidth)
		ಹಿಂತಿರುಗಿ true;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (bwidth < awidth)
		ಹಿಂತಿರುಗಿ false;

	/* 3b -- sort by style */
	ಆಯ್ಕೆ (a->style) {
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOUBLE: impact++; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_SOLID:  impact++; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DASHED: impact++; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOTTED: impact++; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_RIDGE:  impact++; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_OUTSET: impact++; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_GROOVE: impact++; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_INSET:  impact++; fallthrough;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	ಆಯ್ಕೆ (b->style) {
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOUBLE: impact--; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_SOLID:  impact--; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DASHED: impact--; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOTTED: impact--; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_RIDGE:  impact--; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_OUTSET: impact--; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_GROOVE: impact--; fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_INSET:  impact--; fallthrough;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (impact < 0)
		ಹಿಂತಿರುಗಿ true;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (impact > 0)
		ಹಿಂತಿರುಗಿ false;

	/* 4a -- sort by origin */
	impact = 0;

	/** \todo COL/COL_GROUP */
	ಆಯ್ಕೆ (a_src) {
	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_CELL:       impact++; fallthrough;
	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು:        impact++; fallthrough;
	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು:  impact++; fallthrough;
	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ:            impact++; fallthrough;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/** \todo COL/COL_GROUP */
	ಆಯ್ಕೆ (b_src) {
	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_CELL:       impact--; fallthrough;
	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು:        impact--; fallthrough;
	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು:  impact--; fallthrough;
	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ:            impact--; fallthrough;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (impact < 0)
		ಹಿಂತಿರುಗಿ true;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (impact > 0)
		ಹಿಂತಿರುಗಿ false;

	/* 4b -- furthest left (if direction: ltr) and towards top wins */
	/** \todo Currently assumes b satisifies this */
	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ ಶೂನ್ಯ
ಕೋಷ್ಟಕ_cell_ತುದಿ_process_ಕೋಷ್ಟಕ(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
			     ರಚನೆ box *ಕೋಷ್ಟಕ,
			     ರಚನೆ border *a,
			     box_type *a_src)
{
	ರಚನೆ border b;
	box_type b_src;

	/* Top border of table */
	b.style = css_computed_border_ತುದಿ_style(ಕೋಷ್ಟಕ->style);
	b.color = css_computed_border_ತುದಿ_color(ಕೋಷ್ಟಕ->style, &b.c);
	css_computed_border_ತುದಿ_ಅಗಲ(ಕೋಷ್ಟಕ->style, &b.ಅಗಲ, &b.unit);
	b.ಅಗಲ = css_unit_len2device_px(ಕೋಷ್ಟಕ->style, unit_len_ctx,
			b.ಅಗಲ, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_ಕೋಷ್ಟಕ;

	ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
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
ಸ್ಥಿರ bool
ಕೋಷ್ಟಕ_cell_ತುದಿ_process_ಅಡ್ಡಸಾಲು(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
			   ರಚನೆ box *cell,
			   ರಚನೆ box *ಅಡ್ಡಸಾಲು,
			   ರಚನೆ border *a,
			   box_type *a_src)
{
	ರಚನೆ border b;
	box_type b_src;

	/* Bottom border of row */
	b.style = css_computed_border_bottom_style(ಅಡ್ಡಸಾಲು->style);
	b.color = css_computed_border_bottom_color(ಅಡ್ಡಸಾಲು->style, &b.c);
	css_computed_border_bottom_ಅಗಲ(ಅಡ್ಡಸಾಲು->style, &b.ಅಗಲ, &b.unit);
	b.ಅಗಲ = css_unit_len2device_px(ಅಡ್ಡಸಾಲು->style, unit_len_ctx,
			b.ಅಗಲ, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು;

	ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	ಆದರೆ (ಅಡ್ಡಸಾಲು->children == ಖಾಲಿ) {
		/* Row is empty, so consider its top border */
		b.style = css_computed_border_ತುದಿ_style(ಅಡ್ಡಸಾಲು->style);
		b.color = css_computed_border_ತುದಿ_color(ಅಡ್ಡಸಾಲು->style, &b.c);
		css_computed_border_ತುದಿ_ಅಗಲ(ಅಡ್ಡಸಾಲು->style, &b.ಅಗಲ, &b.unit);
		b.ಅಗಲ = css_unit_len2device_px(ಅಡ್ಡಸಾಲು->style, unit_len_ctx,
				b.ಅಗಲ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು;

		ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		ಹಿಂತಿರುಗಿ false;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Process cells that are directly above the cell being
		 * considered. They may not be in this row, but in one of the
		 * rows above it in the case where rowspan > 1. */
		ರಚನೆ box *c;
		bool processed = false;

		ಆಗ (processed == false) {
			ಗಾಗಿ (c = ಅಡ್ಡಸಾಲು->children; c != ಖಾಲಿ; c = c->ಮುಂದಿನ) {
				/* Ignore cells to the left */
				ಆದರೆ (c->ಆರಂಭ_ಸ್ತಂಭ + c->columns - 1 <
				    cell->ಆರಂಭ_ಸ್ತಂಭ)
					ಮುಂದುವರಿ;
				/* Ignore cells to the right */
				ಆದರೆ (c->ಆರಂಭ_ಸ್ತಂಭ > cell->ಆರಂಭ_ಸ್ತಂಭ +
				    cell->columns - 1)
					ಮುಂದುವರಿ;

				/* Flag that we've processed a cell */
				processed = true;

				/* Consider bottom border */
				b.style = css_computed_border_bottom_style(
							c->style);
				b.color = css_computed_border_bottom_color(
							c->style, &b.c);
				css_computed_border_bottom_ಅಗಲ(c->style,
							&b.ಅಗಲ, &b.unit);
				b.ಅಗಲ = css_unit_len2device_px(
						c->style, unit_len_ctx,
						b.ಅಗಲ, b.unit);
				b.unit = CSS_UNIT_PX;
				b_src = BOX_ಕೋಷ್ಟಕ_CELL;

				ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
								     a,
								     *a_src,
								     &b,
								     b_src)) {
					*a = b;
					*a_src = b_src;
				}
			}

			ಆದರೆ (processed == false) {
				/* There must be a preceding row */
				ಪ್ರತಿಜ್ಞಾ(ಅಡ್ಡಸಾಲು->prev != ಖಾಲಿ);

				ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->prev;
			}
		}
	}

	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ bool
ಕೋಷ್ಟಕ_cell_ತುದಿ_process_ಗುಂಪು(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
			     ರಚನೆ box *cell,
			     ರಚನೆ box *ಗುಂಪು,
			     ರಚನೆ border *a,
			     box_type *a_src)
{
	ರಚನೆ border b;
	box_type b_src;

	/* Bottom border of group */
	b.style = css_computed_border_bottom_style(ಗುಂಪು->style);
	b.color = css_computed_border_bottom_color(ಗುಂಪು->style, &b.c);
	css_computed_border_bottom_ಅಗಲ(ಗುಂಪು->style, &b.ಅಗಲ, &b.unit);
	b.ಅಗಲ = css_unit_len2device_px(ಗುಂಪು->style, unit_len_ctx,
			b.ಅಗಲ, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು;

	ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx, a, *a_src, &b, b_src)) {
		*a = b;
		*a_src = b_src;
	}

	ಆದರೆ (ಗುಂಪು->ಕೊನೆಯ != ಖಾಲಿ) {
		/* Process rows in group, starting with last */
		ರಚನೆ box *ಅಡ್ಡಸಾಲು = ಗುಂಪು->ಕೊನೆಯ;

		ಆಗ (ಕೋಷ್ಟಕ_cell_ತುದಿ_process_ಅಡ್ಡಸಾಲು(unit_len_ctx, cell, ಅಡ್ಡಸಾಲು,
						  a, a_src) == false) {
			ಆದರೆ (ಅಡ್ಡಸಾಲು->prev == ಖಾಲಿ) {
				ಹಿಂತಿರುಗಿ false;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->prev;
			}
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Group is empty, so consider its top border */
		b.style = css_computed_border_ತುದಿ_style(ಗುಂಪು->style);
		b.color = css_computed_border_ತುದಿ_color(ಗುಂಪು->style, &b.c);
		css_computed_border_ತುದಿ_ಅಗಲ(ಗುಂಪು->style, &b.ಅಗಲ, &b.unit);
		b.ಅಗಲ = css_unit_len2device_px(ಗುಂಪು->style, unit_len_ctx,
				b.ಅಗಲ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು;

		ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
						     a, *a_src, &b, b_src)) {
			*a = b;
			*a_src = b_src;
		}

		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Calculate used values of border-left-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ಸ್ಥಿರ ಶೂನ್ಯ
ಕೋಷ್ಟಕ_ಬಳಸಿದ_ಎಡ_border_for_cell(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx, ರಚನೆ box *cell)
{
	ರಚನೆ border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed left border for cell */
	a.style = css_computed_border_ಎಡ_style(cell->style);
	a.color = css_computed_border_ಎಡ_color(cell->style, &a.c);
	css_computed_border_ಎಡ_ಅಗಲ(cell->style, &a.ಅಗಲ, &a.unit);
	a.ಅಗಲ = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.ಅಗಲ, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_ಕೋಷ್ಟಕ_CELL;

	ಆದರೆ (cell->prev != ಖಾಲಿ || cell->ಆರಂಭ_ಸ್ತಂಭ != 0) {
		/* Cell to the left -- consider its right border */
		ರಚನೆ box *prev = ಖಾಲಿ;

		ಆದರೆ (cell->prev == ಖಾಲಿ) {
			ರಚನೆ box *ಅಡ್ಡಸಾಲು;

			/* Spanned from a previous row in current row group */
			ಗಾಗಿ (ಅಡ್ಡಸಾಲು = cell->ಪೋಷಕ; ಅಡ್ಡಸಾಲು != ಖಾಲಿ; ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->prev) {
				ಗಾಗಿ (prev = ಅಡ್ಡಸಾಲು->children; prev != ಖಾಲಿ;
				     prev = prev->ಮುಂದಿನ) {
					ಆದರೆ (prev->ಆರಂಭ_ಸ್ತಂಭ +
					    prev->columns ==
					    cell->ಆರಂಭ_ಸ್ತಂಭ)
						ನಿಲ್ಲಿಸು;
				}

				ಆದರೆ (prev != ಖಾಲಿ)
					ನಿಲ್ಲಿಸು;
			}

			ಪ್ರತಿಜ್ಞಾ(prev != ಖಾಲಿ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			prev = cell->prev;
		}

		b.style = css_computed_border_ಬಲ_style(prev->style);
		b.color = css_computed_border_ಬಲ_color(prev->style, &b.c);
		css_computed_border_ಬಲ_ಅಗಲ(prev->style, &b.ಅಗಲ, &b.unit);
		b.ಅಗಲ = css_unit_len2device_px(prev->style, unit_len_ctx,
				b.ಅಗಲ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ಕೋಷ್ಟಕ_CELL;

		ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* First cell in row, so consider rows and row group */
		ರಚನೆ box *ಅಡ್ಡಸಾಲು = cell->ಪೋಷಕ;
		ರಚನೆ box *ಗುಂಪು = ಅಡ್ಡಸಾಲು->ಪೋಷಕ;
		ರಚನೆ box *ಕೋಷ್ಟಕ = ಗುಂಪು->ಪೋಷಕ;
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲುಗಳು = cell->ಅಡ್ಡಸಾಲುಗಳು;

		ಆಗ (ಅಡ್ಡಸಾಲುಗಳು-- > 0 && ಅಡ್ಡಸಾಲು != ಖಾಲಿ) {
			/* Spanned rows -- consider their left border */
			b.style = css_computed_border_ಎಡ_style(ಅಡ್ಡಸಾಲು->style);
			b.color = css_computed_border_ಎಡ_color(
								 ಅಡ್ಡಸಾಲು->style, &b.c);
			css_computed_border_ಎಡ_ಅಗಲ(
						       ಅಡ್ಡಸಾಲು->style, &b.ಅಗಲ, &b.unit);
			b.ಅಗಲ = css_unit_len2device_px(
					ಅಡ್ಡಸಾಲು->style, unit_len_ctx,
					b.ಅಗಲ, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು;

			ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
							     &a, a_src, &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->ಮುಂದಿನ;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its left border */
		b.style = css_computed_border_ಎಡ_style(ಗುಂಪು->style);
		b.color = css_computed_border_ಎಡ_color(ಗುಂಪು->style, &b.c);
		css_computed_border_ಎಡ_ಅಗಲ(ಗುಂಪು->style, &b.ಅಗಲ, &b.unit);
		b.ಅಗಲ = css_unit_len2device_px(ಗುಂಪು->style, unit_len_ctx,
				b.ಅಗಲ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು;

		ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its left border */
		b.style = css_computed_border_ಎಡ_style(ಕೋಷ್ಟಕ->style);
		b.color = css_computed_border_ಎಡ_color(ಕೋಷ್ಟಕ->style, &b.c);
		css_computed_border_ಎಡ_ಅಗಲ(ಕೋಷ್ಟಕ->style, &b.ಅಗಲ, &b.unit);
		b.ಅಗಲ = css_unit_len2device_px(ಕೋಷ್ಟಕ->style, unit_len_ctx,
				b.ಅಗಲ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ಕೋಷ್ಟಕ;

		ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used left border for the cell */
	cell->border[ಎಡ_2].style = a.style;
	cell->border[ಎಡ_2].c = a.c;
	cell->border[ಎಡ_2].ಅಗಲ = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.ಅಗಲ, a.unit));
}


/**
 * Calculate used values of border-top-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ಸ್ಥಿರ ಶೂನ್ಯ
ಕೋಷ್ಟಕ_ಬಳಸಿದ_ತುದಿ_border_for_cell(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx, ರಚನೆ box *cell)
{
	ರಚನೆ border a, b;
	box_type a_src, b_src;
	ರಚನೆ box *ಅಡ್ಡಸಾಲು = cell->ಪೋಷಕ;
	bool process_ಗುಂಪು = false;

	/* Initialise to computed top border for cell */
	a.style = css_computed_border_ತುದಿ_style(cell->style);
	css_computed_border_ತುದಿ_color(cell->style, &a.c);
	css_computed_border_ತುದಿ_ಅಗಲ(cell->style, &a.ಅಗಲ, &a.unit);
	a.ಅಗಲ = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.ಅಗಲ, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_ಕೋಷ್ಟಕ_CELL;

	/* Top border of row */
	b.style = css_computed_border_ತುದಿ_style(ಅಡ್ಡಸಾಲು->style);
	css_computed_border_ತುದಿ_color(ಅಡ್ಡಸಾಲು->style, &b.c);
	css_computed_border_ತುದಿ_ಅಗಲ(ಅಡ್ಡಸಾಲು->style, &b.ಅಗಲ, &b.unit);
	b.ಅಗಲ = css_unit_len2device_px(ಅಡ್ಡಸಾಲು->style, unit_len_ctx,
			b.ಅಗಲ, b.unit);
	b.unit = CSS_UNIT_PX;
	b_src = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು;

	ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx, &a, a_src, &b, b_src)) {
		a = b;
		a_src = b_src;
	}

	ಆದರೆ (ಅಡ್ಡಸಾಲು->prev != ಖಾಲಿ) {
		/* Consider row(s) above */
		ಆಗ (ಕೋಷ್ಟಕ_cell_ತುದಿ_process_ಅಡ್ಡಸಾಲು(unit_len_ctx, cell, ಅಡ್ಡಸಾಲು->prev,
						  &a, &a_src) == false) {
			ಆದರೆ (ಅಡ್ಡಸಾಲು->prev->prev == ಖಾಲಿ) {
				/* Consider row group */
				process_ಗುಂಪು = true;
				ನಿಲ್ಲಿಸು;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->prev;
			}
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		process_ಗುಂಪು = true;
	}

	ಆದರೆ (process_ಗುಂಪು) {
		ರಚನೆ box *ಗುಂಪು = ಅಡ್ಡಸಾಲು->ಪೋಷಕ;

		/* Top border of row group */
		b.style = css_computed_border_ತುದಿ_style(ಗುಂಪು->style);
		b.color = css_computed_border_ತುದಿ_color(ಗುಂಪು->style, &b.c);
		css_computed_border_ತುದಿ_ಅಗಲ(ಗುಂಪು->style, &b.ಅಗಲ, &b.unit);
		b.ಅಗಲ = css_unit_len2device_px(ಗುಂಪು->style, unit_len_ctx,
				b.ಅಗಲ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು;

		ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		ಆದರೆ (ಗುಂಪು->prev == ಖಾಲಿ) {
			/* Top border of table */
			ಕೋಷ್ಟಕ_cell_ತುದಿ_process_ಕೋಷ್ಟಕ(unit_len_ctx,
						     ಗುಂಪು->ಪೋಷಕ, &a, &a_src);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Process previous group(s) */
			ಆಗ (ಕೋಷ್ಟಕ_cell_ತುದಿ_process_ಗುಂಪು(unit_len_ctx,
							    cell, ಗುಂಪು->prev,
							    &a, &a_src) == false) {
				ಆದರೆ (ಗುಂಪು->prev->prev == ಖಾಲಿ) {
					/* Top border of table */
					ಕೋಷ್ಟಕ_cell_ತುದಿ_process_ಕೋಷ್ಟಕ(unit_len_ctx,
								     ಗುಂಪು->ಪೋಷಕ,
								     &a, &a_src);
					ನಿಲ್ಲಿಸು;
				} ಇಲ್ಲದಿದ್ದರೆ {
					ಗುಂಪು = ಗುಂಪು->prev;
				}
			}
		}
	}

	/* a now contains the used top border for the cell */
	cell->border[ತುದಿ_2].style = a.style;
	cell->border[ತುದಿ_2].c = a.c;
	cell->border[ತುದಿ_2].ಅಗಲ = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.ಅಗಲ, a.unit));
}

/**
 * Calculate used values of border-right-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ಸ್ಥಿರ ಶೂನ್ಯ
ಕೋಷ್ಟಕ_ಬಳಸಿದ_ಬಲ_border_for_cell(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx, ರಚನೆ box *cell)
{
	ರಚನೆ border a, b;
	box_type a_src, b_src;

	/** \todo Need column and column_group, too */

	/* Initialise to computed right border for cell */
	a.style = css_computed_border_ಬಲ_style(cell->style);
	css_computed_border_ಬಲ_color(cell->style, &a.c);
	css_computed_border_ಬಲ_ಅಗಲ(cell->style, &a.ಅಗಲ, &a.unit);
	a.ಅಗಲ = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.ಅಗಲ, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_ಕೋಷ್ಟಕ_CELL;

	ಆದರೆ (cell->ಮುಂದಿನ != ಖಾಲಿ || cell->ಆರಂಭ_ಸ್ತಂಭ + cell->columns !=
	    cell->ಪೋಷಕ->ಪೋಷಕ->ಪೋಷಕ->columns) {
		/* Cell is not at right edge of table -- no right border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.ಅಗಲ = 0;
		a.unit = CSS_UNIT_PX;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Last cell in row, so consider rows and row group */
		ರಚನೆ box *ಅಡ್ಡಸಾಲು = cell->ಪೋಷಕ;
		ರಚನೆ box *ಗುಂಪು = ಅಡ್ಡಸಾಲು->ಪೋಷಕ;
		ರಚನೆ box *ಕೋಷ್ಟಕ = ಗುಂಪು->ಪೋಷಕ;
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲುಗಳು = cell->ಅಡ್ಡಸಾಲುಗಳು;

		ಆಗ (ಅಡ್ಡಸಾಲುಗಳು-- > 0 && ಅಡ್ಡಸಾಲು != ಖಾಲಿ) {
			/* Spanned rows -- consider their right border */
			b.style = css_computed_border_ಬಲ_style(ಅಡ್ಡಸಾಲು->style);
			b.color = css_computed_border_ಬಲ_color(ಅಡ್ಡಸಾಲು->style,
								  &b.c);
			css_computed_border_ಬಲ_ಅಗಲ(ಅಡ್ಡಸಾಲು->style,
							&b.ಅಗಲ,
							&b.unit);
			b.ಅಗಲ = css_unit_len2device_px(
					ಅಡ್ಡಸಾಲು->style, unit_len_ctx,
					b.ಅಗಲ, b.unit);
			b.unit = CSS_UNIT_PX;
			b_src = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು;

			ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
							     &a, a_src,
							     &b, b_src)) {
				a = b;
				a_src = b_src;
			}

			ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->ಮುಂದಿನ;
		}

		/** \todo can cells span row groups? */

		/* Row group -- consider its right border */
		b.style = css_computed_border_ಬಲ_style(ಗುಂಪು->style);
		b.color = css_computed_border_ಬಲ_color(ಗುಂಪು->style, &b.c);
		css_computed_border_ಬಲ_ಅಗಲ(ಗುಂಪು->style,
						&b.ಅಗಲ, &b.unit);
		b.ಅಗಲ = css_unit_len2device_px(ಗುಂಪು->style, unit_len_ctx,
				b.ಅಗಲ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು;

		ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its right border */
		b.style = css_computed_border_ಬಲ_style(ಕೋಷ್ಟಕ->style);
		b.color = css_computed_border_ಬಲ_color(ಕೋಷ್ಟಕ->style, &b.c);
		css_computed_border_ಬಲ_ಅಗಲ(ಕೋಷ್ಟಕ->style,
						&b.ಅಗಲ, &b.unit);
		b.ಅಗಲ = css_unit_len2device_px(ಕೋಷ್ಟಕ->style, unit_len_ctx,
				b.ಅಗಲ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ಕೋಷ್ಟಕ;

		ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
						     &a, a_src,
						     &b, b_src)) {
			a = b;
			a_src = b_src;
		}
	}

	/* a now contains the used right border for the cell */
	cell->border[ಬಲ_2].style = a.style;
	cell->border[ಬಲ_2].c = a.c;
	cell->border[ಬಲ_2].ಅಗಲ = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.ಅಗಲ, a.unit));
}


/**
 * Calculate used values of border-bottom-{style,color,width}
 *
 * \param unit_len_ctx  Length conversion context
 * \param cell     Table cell to consider
 */
ಸ್ಥಿರ ಶೂನ್ಯ
ಕೋಷ್ಟಕ_ಬಳಸಿದ_bottom_border_for_cell(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
				  ರಚನೆ box *cell)
{
	ರಚನೆ border a, b;
	box_type a_src, b_src;
	ರಚನೆ box *ಅಡ್ಡಸಾಲು = cell->ಪೋಷಕ;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲುಗಳು = cell->ಅಡ್ಡಸಾಲುಗಳು;

	/* Initialise to computed bottom border for cell */
	a.style = css_computed_border_bottom_style(cell->style);
	css_computed_border_bottom_color(cell->style, &a.c);
	css_computed_border_bottom_ಅಗಲ(cell->style, &a.ಅಗಲ, &a.unit);
	a.ಅಗಲ = css_unit_len2device_px(cell->style, unit_len_ctx,
			a.ಅಗಲ, a.unit);
	a.unit = CSS_UNIT_PX;
	a_src = BOX_ಕೋಷ್ಟಕ_CELL;

	ಆಗ (ಅಡ್ಡಸಾಲುಗಳು-- > 0 && ಅಡ್ಡಸಾಲು != ಖಾಲಿ)
		ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->ಮುಂದಿನ;

	/** \todo Can cells span row groups? */

	ಆದರೆ (ಅಡ್ಡಸಾಲು != ಖಾಲಿ) {
		/* Cell is not at bottom edge of table -- no bottom border */
		a.style = CSS_BORDER_STYLE_NONE;
		a.ಅಗಲ = 0;
		a.unit = CSS_UNIT_PX;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Cell at bottom of table, so consider row and row group */
		ರಚನೆ box *ಅಡ್ಡಸಾಲು = cell->ಪೋಷಕ;
		ರಚನೆ box *ಗುಂಪು = ಅಡ್ಡಸಾಲು->ಪೋಷಕ;
		ರಚನೆ box *ಕೋಷ್ಟಕ = ಗುಂಪು->ಪೋಷಕ;

		/* Bottom border of row */
		b.style = css_computed_border_bottom_style(ಅಡ್ಡಸಾಲು->style);
		b.color = css_computed_border_bottom_color(ಅಡ್ಡಸಾಲು->style, &b.c);
		css_computed_border_bottom_ಅಗಲ(ಅಡ್ಡಸಾಲು->style, &b.ಅಗಲ, &b.unit);
		b.ಅಗಲ = css_unit_len2device_px(ಅಡ್ಡಸಾಲು->style, unit_len_ctx,
				b.ಅಗಲ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು;

		ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* Row group -- consider its bottom border */
		b.style = css_computed_border_bottom_style(ಗುಂಪು->style);
		b.color = css_computed_border_bottom_color(ಗುಂಪು->style, &b.c);
		css_computed_border_bottom_ಅಗಲ(ಗುಂಪು->style,
						 &b.ಅಗಲ, &b.unit);
		b.ಅಗಲ = css_unit_len2device_px(ಗುಂಪು->style, unit_len_ctx,
				b.ಅಗಲ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು;

		ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
			a_src = b_src;
		}

		/* The table itself -- consider its bottom border */
		b.style = css_computed_border_bottom_style(ಕೋಷ್ಟಕ->style);
		b.color = css_computed_border_bottom_color(ಕೋಷ್ಟಕ->style, &b.c);
		css_computed_border_bottom_ಅಗಲ(ಕೋಷ್ಟಕ->style,
						 &b.ಅಗಲ, &b.unit);
		b.ಅಗಲ = css_unit_len2device_px(ಕೋಷ್ಟಕ->style, unit_len_ctx,
				b.ಅಗಲ, b.unit);
		b.unit = CSS_UNIT_PX;
		b_src = BOX_ಕೋಷ್ಟಕ;

		ಆದರೆ (ಕೋಷ್ಟಕ_border_is_ಇನ್ನಷ್ಟು_eyecatching(unit_len_ctx,
						     &a, a_src, &b, b_src)) {
			a = b;
		}
	}

	/* a now contains the used bottom border for the cell */
	cell->border[BOTTOM].style = a.style;
	cell->border[BOTTOM].c = a.c;
	cell->border[BOTTOM].ಅಗಲ = FIXTOINT(css_unit_len2device_px(
			cell->style, unit_len_ctx, a.ಅಗಲ, a.unit));
}


/* exported interface documented in html/table.h */
bool
ಕೋಷ್ಟಕ_calculate_ಸ್ತಂಭ_types(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx, ರಚನೆ box *ಕೋಷ್ಟಕ)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i, j;
	ರಚನೆ ಸ್ತಂಭ *ಲಂಬಸಾಲು;
	ರಚನೆ box *ಅಡ್ಡಸಾಲು_ಗುಂಪು, *ಅಡ್ಡಸಾಲು, *cell;

	ಆದರೆ (ಕೋಷ್ಟಕ->ಲಂಬಸಾಲು)
		/* table->col already constructed, for example frameset table */
		ಹಿಂತಿರುಗಿ true;

	ಕೋಷ್ಟಕ->ಲಂಬಸಾಲು = ಲಂಬಸಾಲು = talloc_array(ಕೋಷ್ಟಕ, ರಚನೆ ಸ್ತಂಭ, ಕೋಷ್ಟಕ->columns);
	ಆದರೆ (!ಲಂಬಸಾಲು)
		ಹಿಂತಿರುಗಿ false;

	ಗಾಗಿ (i = 0; i != ಕೋಷ್ಟಕ->columns; i++) {
		ಲಂಬಸಾಲು[i].type = ಸ್ತಂಭ_ಅಗಲ_UNKNOWN;
		ಲಂಬಸಾಲು[i].ಅಗಲ = 0;
		ಲಂಬಸಾಲು[i].positioned = true;
	}

	/* 1st pass: cells with colspan 1 only */
	ಗಾಗಿ (ಅಡ್ಡಸಾಲು_ಗುಂಪು = ಕೋಷ್ಟಕ->children; ಅಡ್ಡಸಾಲು_ಗುಂಪು; ಅಡ್ಡಸಾಲು_ಗುಂಪು =ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಮುಂದಿನ)
		ಗಾಗಿ (ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು_ಗುಂಪು->children; ಅಡ್ಡಸಾಲು; ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->ಮುಂದಿನ)
			ಗಾಗಿ (cell = ಅಡ್ಡಸಾಲು->children; cell; cell = cell->ಮುಂದಿನ) {
				ಎಣಿಕೆ css_ಅಗಲ_e type;
				css_fixed ಮೌಲ್ಯ = 0;
				css_unit unit = CSS_UNIT_PX;

				ಪ್ರತಿಜ್ಞಾ(cell->type == BOX_ಕೋಷ್ಟಕ_CELL);
				ಪ್ರತಿಜ್ಞಾ(cell->style);

				ಆದರೆ (cell->columns != 1)
					ಮುಂದುವರಿ;
				i = cell->ಆರಂಭ_ಸ್ತಂಭ;

				ಆದರೆ (css_computed_position(cell->style) !=
				    CSS_POSITION_ABSOLUTE &&
				    css_computed_position(cell->style) !=
				    CSS_POSITION_FIXED) {
					ಲಂಬಸಾಲು[i].positioned = false;
				}

				type = css_computed_ಅಗಲ(cell->style, &ಮೌಲ್ಯ, &unit);

				/* fixed width takes priority over any other width type */
				ಆದರೆ (ಲಂಬಸಾಲು[i].type != ಸ್ತಂಭ_ಅಗಲ_FIXED &&
				    type == CSS_ಅಗಲ_ಗಣ && unit != CSS_UNIT_PCT) {
					ಲಂಬಸಾಲು[i].type = ಸ್ತಂಭ_ಅಗಲ_FIXED;
					ಲಂಬಸಾಲು[i].ಅಗಲ = FIXTOINT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							ಮೌಲ್ಯ, unit));
					ಆದರೆ (ಲಂಬಸಾಲು[i].ಅಗಲ < 0)
						ಲಂಬಸಾಲು[i].ಅಗಲ = 0;
					ಮುಂದುವರಿ;
				}

				ಆದರೆ (ಲಂಬಸಾಲು[i].type != ಸ್ತಂಭ_ಅಗಲ_UNKNOWN)
					ಮುಂದುವರಿ;

				ಆದರೆ (type == CSS_ಅಗಲ_ಗಣ && unit == CSS_UNIT_PCT) {
					ಲಂಬಸಾಲು[i].type = ಸ್ತಂಭ_ಅಗಲ_PERCENT;
					ಲಂಬಸಾಲು[i].ಅಗಲ = FIXTOINT(ಮೌಲ್ಯ);
					ಆದರೆ (ಲಂಬಸಾಲು[i].ಅಗಲ < 0)
						ಲಂಬಸಾಲು[i].ಅಗಲ = 0;
				} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (type == CSS_ಅಗಲ_AUTO) {
					ಲಂಬಸಾಲು[i].type = ಸ್ತಂಭ_ಅಗಲ_AUTO;
				}
			}

	/* 2nd pass: cells which span multiple columns */
	ಗಾಗಿ (ಅಡ್ಡಸಾಲು_ಗುಂಪು = ಕೋಷ್ಟಕ->children; ಅಡ್ಡಸಾಲು_ಗುಂಪು; ಅಡ್ಡಸಾಲು_ಗುಂಪು =ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಮುಂದಿನ)
		ಗಾಗಿ (ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು_ಗುಂಪು->children; ಅಡ್ಡಸಾಲು; ಅಡ್ಡಸಾಲು = ಅಡ್ಡಸಾಲು->ಮುಂದಿನ)
			ಗಾಗಿ (cell = ಅಡ್ಡಸಾಲು->children; cell; cell = cell->ಮುಂದಿನ) {
				ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ fixed_columns = 0,
					percent_columns = 0,
					auto_columns = 0,
					unknown_columns = 0;
				ಪೂರ್ಣಾಂಕ fixed_ಅಗಲ = 0, percent_ಅಗಲ = 0;
				ಎಣಿಕೆ css_ಅಗಲ_e type;
				css_fixed ಮೌಲ್ಯ = 0;
				css_unit unit = CSS_UNIT_PX;

				ಆದರೆ (cell->columns == 1)
					ಮುಂದುವರಿ;
				i = cell->ಆರಂಭ_ಸ್ತಂಭ;

				ಗಾಗಿ (j = i; j < i + cell->columns; j++) {
					ಲಂಬಸಾಲು[j].positioned = false;
				}

				/* count column types in spanned cells */
				ಗಾಗಿ (j = 0; j != cell->columns; j++) {
					ಆದರೆ (ಲಂಬಸಾಲು[i + j].type == ಸ್ತಂಭ_ಅಗಲ_FIXED) {
						fixed_ಅಗಲ += ಲಂಬಸಾಲು[i + j].ಅಗಲ;
						fixed_columns++;
					} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಲಂಬಸಾಲು[i + j].type == ಸ್ತಂಭ_ಅಗಲ_PERCENT) {
						percent_ಅಗಲ += ಲಂಬಸಾಲು[i + j].ಅಗಲ;
						percent_columns++;
					} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಲಂಬಸಾಲು[i + j].type == ಸ್ತಂಭ_ಅಗಲ_AUTO) {
						auto_columns++;
					} ಇಲ್ಲದಿದ್ದರೆ {
						unknown_columns++;
					}
				}

				ಆದರೆ (!unknown_columns)
					ಮುಂದುವರಿ;

				type = css_computed_ಅಗಲ(cell->style, &ಮೌಲ್ಯ, &unit);

				/* if cell is fixed width, and all spanned columns are fixed
				 * or unknown width, split extra width among unknown columns */
				ಆದರೆ (type == CSS_ಅಗಲ_ಗಣ && unit != CSS_UNIT_PCT &&
				    fixed_columns + unknown_columns ==
				    cell->columns) {
					ಪೂರ್ಣಾಂಕ ಅಗಲ = (FIXTOFLT(css_unit_len2device_px(
							cell->style,
							unit_len_ctx,
							ಮೌಲ್ಯ, unit)) -
						fixed_ಅಗಲ) / unknown_columns;
					ಆದರೆ (ಅಗಲ < 0)
						ಅಗಲ = 0;
					ಗಾಗಿ (j = 0; j != cell->columns; j++) {
						ಆದರೆ (ಲಂಬಸಾಲು[i + j].type == ಸ್ತಂಭ_ಅಗಲ_UNKNOWN) {
							ಲಂಬಸಾಲು[i + j].type = ಸ್ತಂಭ_ಅಗಲ_FIXED;
							ಲಂಬಸಾಲು[i + j].ಅಗಲ = ಅಗಲ;
						}
					}
				}

				/* as above for percentage width */
				ಆದರೆ (type == CSS_ಅಗಲ_ಗಣ && unit == CSS_UNIT_PCT &&
				    percent_columns + unknown_columns ==
				    cell->columns) {
					ಪೂರ್ಣಾಂಕ ಅಗಲ = (FIXTOFLT(ಮೌಲ್ಯ) -
						     percent_ಅಗಲ) / unknown_columns;
					ಆದರೆ (ಅಗಲ < 0)
						ಅಗಲ = 0;
					ಗಾಗಿ (j = 0; j != cell->columns; j++) {
						ಆದರೆ (ಲಂಬಸಾಲು[i + j].type == ಸ್ತಂಭ_ಅಗಲ_UNKNOWN) {
							ಲಂಬಸಾಲು[i + j].type = ಸ್ತಂಭ_ಅಗಲ_PERCENT;
							ಲಂಬಸಾಲು[i + j].ಅಗಲ = ಅಗಲ;
						}
					}
				}
			}

	/* use AUTO if no width type was specified */
	ಗಾಗಿ (i = 0; i != ಕೋಷ್ಟಕ->columns; i++) {
		ಆದರೆ (ಲಂಬಸಾಲು[i].type == ಸ್ತಂಭ_ಅಗಲ_UNKNOWN)
			ಲಂಬಸಾಲು[i].type = ಸ್ತಂಭ_ಅಗಲ_AUTO;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ಕೋಷ್ಟಕ_DEBUG
	ಗಾಗಿ (i = 0; i != ಕೋಷ್ಟಕ->columns; i++)
		NSLOG(netsurf, INFO,
		      "table %p, column %u: type %s, width %i",
		      ಕೋಷ್ಟಕ,
		      i,
		      ((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *[]){
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE",
				      })[ಲಂಬಸಾಲು[i].type],
		      ಲಂಬಸಾಲು[i].ಅಗಲ);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ true;
}


/* exported interface documented in html/table.h */
ಶೂನ್ಯ ಕೋಷ್ಟಕ_ಬಳಸಿದ_border_for_cell(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx, ರಚನೆ box *cell)
{
	ಪೂರ್ಣಾಂಕ side;

	ಪ್ರತಿಜ್ಞಾ(cell->type == BOX_ಕೋಷ್ಟಕ_CELL);

	ಆದರೆ (css_computed_border_collapse(cell->style) ==
	    CSS_BORDER_COLLAPSE_SEPARATE) {
		css_fixed ಅಗಲ = 0;
		css_unit unit = CSS_UNIT_PX;

		/* Left border */
		cell->border[ಎಡ_2].style =
			css_computed_border_ಎಡ_style(cell->style);
		css_computed_border_ಎಡ_color(cell->style,
					       &cell->border[ಎಡ_2].c);
		css_computed_border_ಎಡ_ಅಗಲ(cell->style, &ಅಗಲ, &unit);
		cell->border[ಎಡ_2].ಅಗಲ =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					ಅಗಲ, unit));

		/* Top border */
		cell->border[ತುದಿ_2].style =
			css_computed_border_ತುದಿ_style(cell->style);
		css_computed_border_ತುದಿ_color(cell->style,
					      &cell->border[ತುದಿ_2].c);
		css_computed_border_ತುದಿ_ಅಗಲ(cell->style, &ಅಗಲ, &unit);
		cell->border[ತುದಿ_2].ಅಗಲ =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					ಅಗಲ, unit));

		/* Right border */
		cell->border[ಬಲ_2].style =
			css_computed_border_ಬಲ_style(cell->style);
		css_computed_border_ಬಲ_color(cell->style,
						&cell->border[ಬಲ_2].c);
		css_computed_border_ಬಲ_ಅಗಲ(cell->style, &ಅಗಲ, &unit);
		cell->border[ಬಲ_2].ಅಗಲ =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					ಅಗಲ, unit));

		/* Bottom border */
		cell->border[BOTTOM].style =
			css_computed_border_bottom_style(cell->style);
		css_computed_border_bottom_color(cell->style,
						 &cell->border[BOTTOM].c);
		css_computed_border_bottom_ಅಗಲ(cell->style, &ಅಗಲ, &unit);
		cell->border[BOTTOM].ಅಗಲ =
			FIXTOINT(css_unit_len2device_px(
					cell->style, unit_len_ctx,
					ಅಗಲ, unit));
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Left border */
		ಕೋಷ್ಟಕ_ಬಳಸಿದ_ಎಡ_border_for_cell(unit_len_ctx, cell);

		/* Top border */
		ಕೋಷ್ಟಕ_ಬಳಸಿದ_ತುದಿ_border_for_cell(unit_len_ctx, cell);

		/* Right border */
		ಕೋಷ್ಟಕ_ಬಳಸಿದ_ಬಲ_border_for_cell(unit_len_ctx, cell);

		/* Bottom border */
		ಕೋಷ್ಟಕ_ಬಳಸಿದ_bottom_border_for_cell(unit_len_ctx, cell);
	}

	/* Finally, ensure that any borders configured as
	 * hidden or none have zero width. (c.f. layout_find_dimensions) */
	ಗಾಗಿ (side = 0; side != 4; side++) {
		ಆದರೆ (cell->border[side].style == CSS_BORDER_STYLE_HIDDEN ||
		    cell->border[side].style ==
		    CSS_BORDER_STYLE_NONE)
			cell->border[side].ಅಗಲ = 0;
	}
}
