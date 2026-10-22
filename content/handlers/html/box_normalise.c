/*
 * Copyright 2005 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2005 John M Bell <jmb202@ecs.soton.ac.uk>
 * Copyright 2004 Kevin Bagust <kevin.bagust@ntlworld.com>
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
 * Box tree normalisation implementation.
 */

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "css/select.h"

#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/table.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_manipulate.h"
#ಅನ್ತರ್ಭಾವ "html/box_normalise.h"

/* Define to enable box normalise debug */
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
ರಚನೆ ಹರವು_info {
	/** Number of rows this cell spans */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲು_ಹರವು;
	/** Row group of cell */
	ರಚನೆ box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_ಅಡ್ಡಸಾಲು;
};

/**
 * Column record for a table
 */
ರಚನೆ columns {
	/** Current column index */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ current_ಸ್ತಂಭ;
	/** Number of columns in main part of table 1..max columns */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಂಕೆ_columns;
	/** Information about columns in main table, array [0, num_columns) */
	ರಚನೆ ಹರವು_info *spans;
	/** Number of rows in table */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಂಕೆ_ಅಡ್ಡಸಾಲುಗಳು;
};


/**
 * Compute the column index at which the current cell begins.
 * Additionally, update the column record to reflect row spanning.
 *
 * \param col_info      Column record
 * \param col_span      Number of columns that current cell spans
 * \param row_span      Number of rows that current cell spans
 * \param start_column  Pointer to location to receive column index
 * \param cell		Box for current table cell
 * \return  true on success, false on memory exhaustion
 */
ಸ್ಥಿರ bool
calculate_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು(ರಚನೆ columns *ಲಂಬಸಾಲು_info,
		    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು_ಹರವು,
		    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲು_ಹರವು,
		    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *ಆರಂಭ_ಸ್ತಂಭ,
		    ರಚನೆ box *cell)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cell_ಆರಂಭ_ಲಂಬಸಾಲು = ಲಂಬಸಾಲು_info->current_ಸ್ತಂಭ;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cell_ಕೊನೆ_ಲಂಬಸಾಲು;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ ಹರವು_info *spans;
	ರಚನೆ box *rg = cell->ಪೋಷಕ->ಪೋಷಕ; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	ಆಗ (ಲಂಬಸಾಲು_info->spans[cell_ಆರಂಭ_ಲಂಬಸಾಲು].ಅಡ್ಡಸಾಲು_ಹರವು != 0 &&
			ಲಂಬಸಾಲು_info->spans[cell_ಆರಂಭ_ಲಂಬಸಾಲು].rg == rg) {
		cell_ಆರಂಭ_ಲಂಬಸಾಲು++;
	}

	/* Update current column with calculated start */
	ಲಂಬಸಾಲು_info->current_ಸ್ತಂಭ = cell_ಆರಂಭ_ಲಂಬಸಾಲು;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	ಆದರೆ (ಲಂಬಸಾಲು_ಹರವು == 0)
		ಲಂಬಸಾಲು_ಹರವು = 1;

	cell_ಕೊನೆ_ಲಂಬಸಾಲು = cell_ಆರಂಭ_ಲಂಬಸಾಲು + ಲಂಬಸಾಲು_ಹರವು;

	ಆದರೆ (ಲಂಬಸಾಲು_info->ಅಂಕೆ_columns < cell_ಕೊನೆ_ಲಂಬಸಾಲು) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(ಲಂಬಸಾಲು_info->spans,
				ಗಾತ್ರ *spans * (cell_ಕೊನೆ_ಲಂಬಸಾಲು + 1));
		ಆದರೆ (spans == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ false;

		ಲಂಬಸಾಲು_info->spans = spans;
		ಲಂಬಸಾಲು_info->ಅಂಕೆ_columns = cell_ಕೊನೆ_ಲಂಬಸಾಲು;

		/* Mark new final column as sentinel */
		ಲಂಬಸಾಲು_info->spans[cell_ಕೊನೆ_ಲಂಬಸಾಲು].ಅಡ್ಡಸಾಲು_ಹರವು = 0;
		ಲಂಬಸಾಲು_info->spans[cell_ಕೊನೆ_ಲಂಬಸಾಲು].auto_ಅಡ್ಡಸಾಲು = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	ಗಾಗಿ (i = cell_ಆರಂಭ_ಲಂಬಸಾಲು; i < cell_ಕೊನೆ_ಲಂಬಸಾಲು; i++) {
		ಲಂಬಸಾಲು_info->spans[i].ಅಡ್ಡಸಾಲು_ಹರವು = (ಅಡ್ಡಸಾಲು_ಹರವು == 0) ? 1 : ಅಡ್ಡಸಾಲು_ಹರವು;
		ಲಂಬಸಾಲು_info->spans[i].auto_ಅಡ್ಡಸಾಲು = (ಅಡ್ಡಸಾಲು_ಹರವು == 0);
		ಲಂಬಸಾಲು_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	ಲಂಬಸಾಲು_info->current_ಸ್ತಂಭ = cell_ಕೊನೆ_ಲಂಬಸಾಲು;

	*ಆರಂಭ_ಸ್ತಂಭ = cell_ಆರಂಭ_ಲಂಬಸಾಲು;

	ಹಿಂತಿರುಗಿ true;
}


ಸ್ಥಿರ bool
box_normalise_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು(ರಚನೆ box *ಅಡ್ಡಸಾಲು,
			ಸ್ಥಿರಾಂಕ ರಚನೆ box *ಬೇರು,
			ರಚನೆ columns *ಲಂಬಸಾಲು_info,
			html_content * c)
{
	ರಚನೆ box *ಮಗು;
	ರಚನೆ box *ಮುಂದಿನ_ಮಗು;
	ರಚನೆ box *cell = ಖಾಲಿ;
	css_computed_style *style;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	nscss_select_ctx ctx;

	ಪ್ರತಿಜ್ಞಾ(ಅಡ್ಡಸಾಲು != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(ಅಡ್ಡಸಾಲು->type == BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು);

	ctx.ಬೇರು_style = ಬೇರು->style;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", ಅಡ್ಡಸಾಲು);
#ಆದರೆ_ಅಂತ್ಯ

	ಗಾಗಿ (ಮಗು = ಅಡ್ಡಸಾಲು->children; ಮಗು != ಖಾಲಿ; ಮಗು = ಮುಂದಿನ_ಮಗು) {
		ಮುಂದಿನ_ಮಗು = ಮಗು->ಮುಂದಿನ;

		ಆಯ್ಕೆ (ಮಗು->type) {
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_CELL:
			/* ok */
			ಆದರೆ (box_normalise_ಖಂಡ(ಮಗು, ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			cell = ಮಗು;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_FLEX:
		ಸ್ಥಿತಿ BOX_ಖಂಡ:
		ಸ್ಥಿತಿ BOX_INLINE_CONTAINER:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು:
			/* insert implied table cell */
			ಪ್ರತಿಜ್ಞಾ(ಅಡ್ಡಸಾಲು->style != ಖಾಲಿ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ಬಹುಲಕ_ತುಂಬಿದ);
			ctx.ತಳ_url = c->ತಳ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					ಅಡ್ಡಸಾಲು->style);
			ಆದರೆ (style == ಖಾಲಿ)
				ಹಿಂತಿರುಗಿ false;

			cell = box_create(ಖಾಲಿ, style, true, ಅಡ್ಡಸಾಲು->href,
					ಅಡ್ಡಸಾಲು->ಗುರಿ, ಖಾಲಿ, ಖಾಲಿ, c->bctx);
			ಆದರೆ (cell == ಖಾಲಿ) {
				css_computed_style_destroy(style);
				ಹಿಂತಿರುಗಿ false;
			}
			cell->type = BOX_ಕೋಷ್ಟಕ_CELL;

			ಆದರೆ (ಮಗು->prev == ಖಾಲಿ)
				ಅಡ್ಡಸಾಲು->children = cell;
			ಇಲ್ಲದಿದ್ದರೆ
				ಮಗು->prev->ಮುಂದಿನ = cell;

			cell->prev = ಮಗು->prev;

			ಆಗ (ಮಗು != ಖಾಲಿ && (
					ಮಗು->type == BOX_FLEX ||
					ಮಗು->type == BOX_ಖಂಡ ||
					ಮಗು->type == BOX_INLINE_CONTAINER ||
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ ||
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು ||
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು)) {
				box_ಕೂಡಿಸು_ಮಗು(cell, ಮಗು);

				ಮುಂದಿನ_ಮಗು = ಮಗು->ಮುಂದಿನ;
				ಮಗು->ಮುಂದಿನ = ಖಾಲಿ;
				ಮಗು = ಮುಂದಿನ_ಮಗು;
			}

			ಪ್ರತಿಜ್ಞಾ(cell->ಕೊನೆಯ != ಖಾಲಿ);

			cell->ಕೊನೆಯ->ಮುಂದಿನ = ಖಾಲಿ;
			cell->ಮುಂದಿನ = ಮುಂದಿನ_ಮಗು = ಮಗು;
			ಆದರೆ (cell->ಮುಂದಿನ != ಖಾಲಿ)
				cell->ಮುಂದಿನ->prev = cell;
			ಇಲ್ಲದಿದ್ದರೆ
				ಅಡ್ಡಸಾಲು->ಕೊನೆಯ = cell;
			cell->ಪೋಷಕ = ಅಡ್ಡಸಾಲು;

			ಆದರೆ (box_normalise_ಖಂಡ(cell, ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_INLINE:
		ಸ್ಥಿತಿ BOX_INLINE_ಕೊನೆ:
		ಸ್ಥಿತಿ BOX_INLINE_FLEX:
		ಸ್ಥಿತಿ BOX_INLINE_ಖಂಡ:
		ಸ್ಥಿತಿ BOX_FLOAT_ಎಡ:
		ಸ್ಥಿತಿ BOX_FLOAT_ಬಲ:
		ಸ್ಥಿತಿ BOX_BR:
		ಸ್ಥಿತಿ BOX_ಪಠ್ಯ:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ಪ್ರತಿಜ್ಞಾ(0);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಪ್ರತಿಜ್ಞಾ(0);
		}

		ಆದರೆ (calculate_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು(ಲಂಬಸಾಲು_info, cell->columns, cell->ಅಡ್ಡಸಾಲುಗಳು,
				&cell->ಆರಂಭ_ಸ್ತಂಭ, cell) == false)
			ಹಿಂತಿರುಗಿ false;
	}


	/* Update row spanning details for all columns */
	ಗಾಗಿ (i = 0; i < ಲಂಬಸಾಲು_info->ಅಂಕೆ_columns; i++) {
		ಆದರೆ (ಲಂಬಸಾಲು_info->spans[i].ಅಡ್ಡಸಾಲು_ಹರವು != 0 &&
				ಲಂಬಸಾಲು_info->spans[i].auto_ಅಡ್ಡಸಾಲು == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			ಲಂಬಸಾಲು_info->spans[i].ಅಡ್ಡಸಾಲು_ಹರವು--;
		}
	}

	/* Reset current column for next row */
	ಲಂಬಸಾಲು_info->current_ಸ್ತಂಭ = 0;

	/* Increment row counter */
	ಲಂಬಸಾಲು_info->ಅಂಕೆ_ಅಡ್ಡಸಾಲುಗಳು++;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", ಅಡ್ಡಸಾಲು);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ true;
}


ಸ್ಥಿರ bool
box_normalise_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು(ರಚನೆ box *ಅಡ್ಡಸಾಲು_ಗುಂಪು,
			      ಸ್ಥಿರಾಂಕ ರಚನೆ box *ಬೇರು,
			      ರಚನೆ columns *ಲಂಬಸಾಲು_info,
			      html_content * c)
{
	ರಚನೆ box *ಮಗು;
	ರಚನೆ box *ಮುಂದಿನ_ಮಗು;
	ರಚನೆ box *ಅಡ್ಡಸಾಲು;
	css_computed_style *style;
	nscss_select_ctx ctx;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗುಂಪು_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ = 0;

	ಪ್ರತಿಜ್ಞಾ(ಅಡ್ಡಸಾಲು_ಗುಂಪು != 0);
	ಪ್ರತಿಜ್ಞಾ(ಅಡ್ಡಸಾಲು_ಗುಂಪು->type == BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು);

	ctx.ಬೇರು_style = ಬೇರು->style;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", ಅಡ್ಡಸಾಲು_ಗುಂಪು);
#ಆದರೆ_ಅಂತ್ಯ

	ಗಾಗಿ (ಮಗು = ಅಡ್ಡಸಾಲು_ಗುಂಪು->children; ಮಗು != ಖಾಲಿ; ಮಗು = ಮುಂದಿನ_ಮಗು) {
		ಮುಂದಿನ_ಮಗು = ಮಗು->ಮುಂದಿನ;

		ಆಯ್ಕೆ (ಮಗು->type) {
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು:
			/* ok */
			ಗುಂಪು_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ++;
			ಆದರೆ (box_normalise_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು(ಮಗು, ಬೇರು, ಲಂಬಸಾಲು_info,
					c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_FLEX:
		ಸ್ಥಿತಿ BOX_ಖಂಡ:
		ಸ್ಥಿತಿ BOX_INLINE_CONTAINER:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_CELL:
			/* insert implied table row */
			ಪ್ರತಿಜ್ಞಾ(ಅಡ್ಡಸಾಲು_ಗುಂಪು->style != ಖಾಲಿ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ಬಹುಲಕ_ತುಂಬಿದ);
			ctx.ತಳ_url = c->ತಳ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					ಅಡ್ಡಸಾಲು_ಗುಂಪು->style);
			ಆದರೆ (style == ಖಾಲಿ)
				ಹಿಂತಿರುಗಿ false;

			ಅಡ್ಡಸಾಲು = box_create(ಖಾಲಿ, style, true, ಅಡ್ಡಸಾಲು_ಗುಂಪು->href,
					ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಗುರಿ, ಖಾಲಿ, ಖಾಲಿ, c->bctx);
			ಆದರೆ (ಅಡ್ಡಸಾಲು == ಖಾಲಿ) {
				css_computed_style_destroy(style);
				ಹಿಂತಿರುಗಿ false;
			}
			ಅಡ್ಡಸಾಲು->type = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು;

			ಆದರೆ (ಮಗು->prev == ಖಾಲಿ)
				ಅಡ್ಡಸಾಲು_ಗುಂಪು->children = ಅಡ್ಡಸಾಲು;
			ಇಲ್ಲದಿದ್ದರೆ
				ಮಗು->prev->ಮುಂದಿನ = ಅಡ್ಡಸಾಲು;

			ಅಡ್ಡಸಾಲು->prev = ಮಗು->prev;

			ಆಗ (ಮಗು != ಖಾಲಿ && (
					ಮಗು->type == BOX_FLEX ||
					ಮಗು->type == BOX_ಖಂಡ ||
					ಮಗು->type == BOX_INLINE_CONTAINER ||
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ ||
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು ||
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ_CELL)) {
				box_ಕೂಡಿಸು_ಮಗು(ಅಡ್ಡಸಾಲು, ಮಗು);

				ಮುಂದಿನ_ಮಗು = ಮಗು->ಮುಂದಿನ;
				ಮಗು->ಮುಂದಿನ = ಖಾಲಿ;
				ಮಗು = ಮುಂದಿನ_ಮಗು;
			}

			ಪ್ರತಿಜ್ಞಾ(ಅಡ್ಡಸಾಲು->ಕೊನೆಯ != ಖಾಲಿ);

			ಅಡ್ಡಸಾಲು->ಕೊನೆಯ->ಮುಂದಿನ = ಖಾಲಿ;
			ಅಡ್ಡಸಾಲು->ಮುಂದಿನ = ಮುಂದಿನ_ಮಗು = ಮಗು;
			ಆದರೆ (ಅಡ್ಡಸಾಲು->ಮುಂದಿನ != ಖಾಲಿ)
				ಅಡ್ಡಸಾಲು->ಮುಂದಿನ->prev = ಅಡ್ಡಸಾಲು;
			ಇಲ್ಲದಿದ್ದರೆ
				ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಕೊನೆಯ = ಅಡ್ಡಸಾಲು;
			ಅಡ್ಡಸಾಲು->ಪೋಷಕ = ಅಡ್ಡಸಾಲು_ಗುಂಪು;

			ಗುಂಪು_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ++;
			ಆದರೆ (box_normalise_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು(ಅಡ್ಡಸಾಲು, ಬೇರು, ಲಂಬಸಾಲು_info,
					c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_INLINE:
		ಸ್ಥಿತಿ BOX_INLINE_ಕೊನೆ:
		ಸ್ಥಿತಿ BOX_INLINE_FLEX:
		ಸ್ಥಿತಿ BOX_INLINE_ಖಂಡ:
		ಸ್ಥಿತಿ BOX_FLOAT_ಎಡ:
		ಸ್ಥಿತಿ BOX_FLOAT_ಬಲ:
		ಸ್ಥಿತಿ BOX_BR:
		ಸ್ಥಿತಿ BOX_ಪಠ್ಯ:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ಪ್ರತಿಜ್ಞಾ(0);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಪ್ರತಿಜ್ಞಾ(0);
		}
	}

	ಆದರೆ (ಅಡ್ಡಸಾಲು_ಗುಂಪು->children == ಖಾಲಿ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#ಆದರೆ_ಅಂತ್ಯ

		ಪ್ರತಿಜ್ಞಾ(ಅಡ್ಡಸಾಲು_ಗುಂಪು->style != ಖಾಲಿ);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ಬಹುಲಕ_ತುಂಬಿದ);
		ctx.ತಳ_url = c->ತಳ_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				ಅಡ್ಡಸಾಲು_ಗುಂಪು->style);
		ಆದರೆ (style == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ false;
		}

		ಅಡ್ಡಸಾಲು = box_create(ಖಾಲಿ, style, true, ಅಡ್ಡಸಾಲು_ಗುಂಪು->href,
				ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಗುರಿ, ಖಾಲಿ, ಖಾಲಿ, c->bctx);
		ಆದರೆ (ಅಡ್ಡಸಾಲು == ಖಾಲಿ) {
			css_computed_style_destroy(style);
			ಹಿಂತಿರುಗಿ false;
		}
		ಅಡ್ಡಸಾಲು->type = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು;

		ಅಡ್ಡಸಾಲು->ಪೋಷಕ = ಅಡ್ಡಸಾಲು_ಗುಂಪು;
		ಅಡ್ಡಸಾಲು_ಗುಂಪು->children = ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಕೊನೆಯ = ಅಡ್ಡಸಾಲು;

		ಗುಂಪು_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ = 1;

		/* Keep table's row count in sync */
		ಲಂಬಸಾಲು_info->ಅಂಕೆ_ಅಡ್ಡಸಾಲುಗಳು++;
	}

	ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಅಡ್ಡಸಾಲುಗಳು = ಗುಂಪು_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", ಅಡ್ಡಸಾಲು_ಗುಂಪು);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Normalise table cell column/row counts for colspan/rowspan = 0.
 * Additionally, generate empty cells.
 *
 * \param table  Table to process
 * \param root   root box of document
 * \param spans  Array of length table->columns for use in empty cell detection
 * \param c      Content containing table
 * \return True on success, false on memory exhaustion.
 */
ಸ್ಥಿರ bool
box_normalise_ಕೋಷ್ಟಕ_spans(ರಚನೆ box *ಕೋಷ್ಟಕ,
			  ಸ್ಥಿರಾಂಕ ರಚನೆ box *ಬೇರು,
			  ರಚನೆ ಹರವು_info *spans,
			  html_content *c)
{
	ರಚನೆ box *ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು;
	ರಚನೆ box *ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು;
	ರಚನೆ box *ಕೋಷ್ಟಕ_cell;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲುಗಳು_ಎಡ = ಕೋಷ್ಟಕ->ಅಡ್ಡಸಾಲುಗಳು;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗುಂಪು_ಅಡ್ಡಸಾಲುಗಳು_ಎಡ;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು;
	nscss_select_ctx ctx;

	ctx.ಬೇರು_style = ಬೇರು->style;

	/* Clear span data */
	memset(spans, 0, ಕೋಷ್ಟಕ->columns * ಗಾತ್ರ(ರಚನೆ ಹರವು_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	ಗಾಗಿ (ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು = ಕೋಷ್ಟಕ->children;
	     ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು != ಖಾಲಿ;
	     ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು = ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಮುಂದಿನ) {

		ಗುಂಪು_ಅಡ್ಡಸಾಲುಗಳು_ಎಡ = ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಅಡ್ಡಸಾಲುಗಳು;

		ಗಾಗಿ (ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು = ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು->children;
		     ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು != ಖಾಲಿ;
		     ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು = ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು->ಮುಂದಿನ) {

			ಗಾಗಿ (ಕೋಷ್ಟಕ_cell = ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು->children;
			     ಕೋಷ್ಟಕ_cell != ಖಾಲಿ;
			     ಕೋಷ್ಟಕ_cell = ಕೋಷ್ಟಕ_cell->ಮುಂದಿನ) {

				/* colspan = 0 -> colspan = 1 */
				ಆದರೆ (ಕೋಷ್ಟಕ_cell->columns == 0) {
					ಕೋಷ್ಟಕ_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				ಆದರೆ (ಕೋಷ್ಟಕ_cell->ಅಡ್ಡಸಾಲುಗಳು == 0) {
					ಕೋಷ್ಟಕ_cell->ಅಡ್ಡಸಾಲುಗಳು = ಗುಂಪು_ಅಡ್ಡಸಾಲುಗಳು_ಎಡ;
				}

				/* limit rowspans within group */
				ಆದರೆ (ಕೋಷ್ಟಕ_cell->ಅಡ್ಡಸಾಲುಗಳು > ಗುಂಪು_ಅಡ್ಡಸಾಲುಗಳು_ಎಡ) {
					ಕೋಷ್ಟಕ_cell->ಅಡ್ಡಸಾಲುಗಳು = ಗುಂಪು_ಅಡ್ಡಸಾಲುಗಳು_ಎಡ;
				}

				/* Record span information */
				ಗಾಗಿ (ಲಂಬಸಾಲು = ಕೋಷ್ಟಕ_cell->ಆರಂಭ_ಸ್ತಂಭ;
						ಲಂಬಸಾಲು < ಕೋಷ್ಟಕ_cell->ಆರಂಭ_ಸ್ತಂಭ +
						ಕೋಷ್ಟಕ_cell->columns; ಲಂಬಸಾಲು++) {
					spans[ಲಂಬಸಾಲು].ಅಡ್ಡಸಾಲು_ಹರವು = ಕೋಷ್ಟಕ_cell->ಅಡ್ಡಸಾಲುಗಳು;
				}
			}

			/* Reduce span count of each column */
			ಗಾಗಿ (ಲಂಬಸಾಲು = 0; ಲಂಬಸಾಲು < ಕೋಷ್ಟಕ->columns; ಲಂಬಸಾಲು++) {
				ಆದರೆ (spans[ಲಂಬಸಾಲು].ಅಡ್ಡಸಾಲು_ಹರವು == 0) {
					ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಆರಂಭ = ಲಂಬಸಾಲು;
					css_computed_style *style;
					ರಚನೆ box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					ಪ್ರತಿಜ್ಞಾ(ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು->style != ಖಾಲಿ);

					/* Find width of gap */
					ಆಗ (ಲಂಬಸಾಲು < ಕೋಷ್ಟಕ->columns &&
							spans[ಲಂಬಸಾಲು].ಅಡ್ಡಸಾಲು_ಹರವು ==
							0) {
						ಲಂಬಸಾಲು++;
					}

					ctx.ctx = c->select_ctx;
					ctx.quirks = (c->quirks ==
						DOM_DOCUMENT_QUIRKS_ಬಹುಲಕ_ತುಂಬಿದ);
					ctx.ತಳ_url = c->ತಳ_url;
					ctx.universal = c->universal;

					style = nscss_get_blank_style(&ctx,
							&c->unit_len_ctx,
							ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು->style);
					ಆದರೆ (style == ಖಾಲಿ)
						ಹಿಂತಿರುಗಿ false;

					cell = box_create(ಖಾಲಿ, style, true,
							ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು->href,
							ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು->ಗುರಿ,
							ಖಾಲಿ, ಖಾಲಿ, c->bctx);
					ಆದರೆ (cell == ಖಾಲಿ) {
						css_computed_style_destroy(
								style);
						ಹಿಂತಿರುಗಿ false;
					}
					cell->type = BOX_ಕೋಷ್ಟಕ_CELL;

					cell->ಅಡ್ಡಸಾಲುಗಳು = 1;
					cell->columns = ಲಂಬಸಾಲು - ಆರಂಭ;
					cell->ಆರಂಭ_ಸ್ತಂಭ = ಆರಂಭ;

					/* Find place to insert cell */
					ಗಾಗಿ (prev = ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು->children;
							prev != ಖಾಲಿ;
							prev = prev->ಮುಂದಿನ) {
						ಆದರೆ (prev->ಆರಂಭ_ಸ್ತಂಭ +
							prev->columns ==
								ಆರಂಭ)
							ನಿಲ್ಲಿಸು;
						ಆದರೆ (prev->ಮುಂದಿನ == ಖಾಲಿ)
							ನಿಲ್ಲಿಸು;
					}

					/* Insert it */
					ಆದರೆ (prev == ಖಾಲಿ) {
						ಆದರೆ (ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು->children != ಖಾಲಿ)
							ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು->children->
								prev = cell;
						ಇಲ್ಲದಿದ್ದರೆ
							ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು->ಕೊನೆಯ = cell;

						cell->ಮುಂದಿನ =
							ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು->children;
						ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು->children = cell;
					} ಇಲ್ಲದಿದ್ದರೆ {
						ಆದರೆ (prev->ಮುಂದಿನ != ಖಾಲಿ)
							prev->ಮುಂದಿನ->prev = cell;
						ಇಲ್ಲದಿದ್ದರೆ
							ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು->ಕೊನೆಯ = cell;

						cell->ಮುಂದಿನ = prev->ಮುಂದಿನ;
						prev->ಮುಂದಿನ = cell;
						cell->prev = prev;
					}
					cell->ಪೋಷಕ = ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು;
				} ಇಲ್ಲದಿದ್ದರೆ {
					spans[ಲಂಬಸಾಲು].ಅಡ್ಡಸಾಲು_ಹರವು--;
				}
			}

			ಪ್ರತಿಜ್ಞಾ(ಅಡ್ಡಸಾಲುಗಳು_ಎಡ > 0);

			ಅಡ್ಡಸಾಲುಗಳು_ಎಡ--;
		}

		ಗುಂಪು_ಅಡ್ಡಸಾಲುಗಳು_ಎಡ--;
	}

	ಹಿಂತಿರುಗಿ true;
}


ಸ್ಥಿರ bool
box_normalise_ಕೋಷ್ಟಕ(ರಚನೆ box *ಕೋಷ್ಟಕ, ಸ್ಥಿರಾಂಕ ರಚನೆ box *ಬೇರು, html_content * c)
{
	ರಚನೆ box *ಮಗು;
	ರಚನೆ box *ಮುಂದಿನ_ಮಗು;
	ರಚನೆ box *ಅಡ್ಡಸಾಲು_ಗುಂಪು;
	css_computed_style *style;
	ರಚನೆ columns ಲಂಬಸಾಲು_info;
	nscss_select_ctx ctx;

	ಪ್ರತಿಜ್ಞಾ(ಕೋಷ್ಟಕ != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(ಕೋಷ್ಟಕ->type == BOX_ಕೋಷ್ಟಕ);

	ctx.ಬೇರು_style = ಬೇರು->style;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", ಕೋಷ್ಟಕ);
#ಆದರೆ_ಅಂತ್ಯ

	ಲಂಬಸಾಲು_info.ಅಂಕೆ_columns = 1;
	ಲಂಬಸಾಲು_info.current_ಸ್ತಂಭ = 0;
	ಲಂಬಸಾಲು_info.spans = ಸ್ಮರಣೆ_ಕೊಡು(2 * ಗಾತ್ರ *ಲಂಬಸಾಲು_info.spans);
	ಆದರೆ (ಲಂಬಸಾಲು_info.spans == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ false;

	ಲಂಬಸಾಲು_info.spans[0].ಅಡ್ಡಸಾಲು_ಹರವು = ಲಂಬಸಾಲು_info.spans[1].ಅಡ್ಡಸಾಲು_ಹರವು = 0;
	ಲಂಬಸಾಲು_info.spans[0].auto_ಅಡ್ಡಸಾಲು = false;
	ಲಂಬಸಾಲು_info.spans[1].auto_ಅಡ್ಡಸಾಲು = false;
	ಲಂಬಸಾಲು_info.ಅಂಕೆ_ಅಡ್ಡಸಾಲುಗಳು = 0;

	ಗಾಗಿ (ಮಗು = ಕೋಷ್ಟಕ->children; ಮಗು != ಖಾಲಿ; ಮಗು = ಮುಂದಿನ_ಮಗು) {
		ಮುಂದಿನ_ಮಗು = ಮಗು->ಮುಂದಿನ;
		ಆಯ್ಕೆ (ಮಗು->type) {
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು:
			/* ok */
			ಆದರೆ (box_normalise_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು(ಮಗು, ಬೇರು,
					&ಲಂಬಸಾಲು_info, c) == false) {
				ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು_info.spans);
				ಹಿಂತಿರುಗಿ false;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_FLEX:
		ಸ್ಥಿತಿ BOX_ಖಂಡ:
		ಸ್ಥಿತಿ BOX_INLINE_CONTAINER:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_CELL:
			/* insert implied table row group */
			ಪ್ರತಿಜ್ಞಾ(ಕೋಷ್ಟಕ->style != ಖಾಲಿ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ಬಹುಲಕ_ತುಂಬಿದ);
			ctx.ತಳ_url = c->ತಳ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					ಕೋಷ್ಟಕ->style);
			ಆದರೆ (style == ಖಾಲಿ) {
				ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು_info.spans);
				ಹಿಂತಿರುಗಿ false;
			}

			ಅಡ್ಡಸಾಲು_ಗುಂಪು = box_create(ಖಾಲಿ, style, true, ಕೋಷ್ಟಕ->href,
					ಕೋಷ್ಟಕ->ಗುರಿ, ಖಾಲಿ, ಖಾಲಿ, c->bctx);
			ಆದರೆ (ಅಡ್ಡಸಾಲು_ಗುಂಪು == ಖಾಲಿ) {
				css_computed_style_destroy(style);
				ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು_info.spans);
				ಹಿಂತಿರುಗಿ false;
			}

			ಅಡ್ಡಸಾಲು_ಗುಂಪು->type = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು;

			ಆದರೆ (ಮಗು->prev == ಖಾಲಿ)
				ಕೋಷ್ಟಕ->children = ಅಡ್ಡಸಾಲು_ಗುಂಪು;
			ಇಲ್ಲದಿದ್ದರೆ
				ಮಗು->prev->ಮುಂದಿನ = ಅಡ್ಡಸಾಲು_ಗುಂಪು;

			ಅಡ್ಡಸಾಲು_ಗುಂಪು->prev = ಮಗು->prev;

			ಆಗ (ಮಗು != ಖಾಲಿ && (
					ಮಗು->type == BOX_FLEX ||
					ಮಗು->type == BOX_ಖಂಡ ||
					ಮಗು->type == BOX_INLINE_CONTAINER ||
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ ||
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು ||
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ_CELL)) {
				box_ಕೂಡಿಸು_ಮಗು(ಅಡ್ಡಸಾಲು_ಗುಂಪು, ಮಗು);

				ಮುಂದಿನ_ಮಗು = ಮಗು->ಮುಂದಿನ;
				ಮಗು->ಮುಂದಿನ = ಖಾಲಿ;
				ಮಗು = ಮುಂದಿನ_ಮಗು;
			}

			ಪ್ರತಿಜ್ಞಾ(ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಕೊನೆಯ != ಖಾಲಿ);

			ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಕೊನೆಯ->ಮುಂದಿನ = ಖಾಲಿ;
			ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಮುಂದಿನ = ಮುಂದಿನ_ಮಗು = ಮಗು;
			ಆದರೆ (ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಮುಂದಿನ != ಖಾಲಿ)
				ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಮುಂದಿನ->prev = ಅಡ್ಡಸಾಲು_ಗುಂಪು;
			ಇಲ್ಲದಿದ್ದರೆ
				ಕೋಷ್ಟಕ->ಕೊನೆಯ = ಅಡ್ಡಸಾಲು_ಗುಂಪು;
			ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಪೋಷಕ = ಕೋಷ್ಟಕ;

			ಆದರೆ (box_normalise_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು(ಅಡ್ಡಸಾಲು_ಗುಂಪು, ಬೇರು,
					&ಲಂಬಸಾಲು_info, c) == false) {
				ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು_info.spans);
				ಹಿಂತಿರುಗಿ false;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_INLINE:
		ಸ್ಥಿತಿ BOX_INLINE_ಕೊನೆ:
		ಸ್ಥಿತಿ BOX_INLINE_FLEX:
		ಸ್ಥಿತಿ BOX_INLINE_ಖಂಡ:
		ಸ್ಥಿತಿ BOX_FLOAT_ಎಡ:
		ಸ್ಥಿತಿ BOX_FLOAT_ಬಲ:
		ಸ್ಥಿತಿ BOX_BR:
		ಸ್ಥಿತಿ BOX_ಪಠ್ಯ:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ಪ್ರತಿಜ್ಞಾ(0);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			fprintf(stderr, "%i\n", ಮಗು->type);
			ಪ್ರತಿಜ್ಞಾ(0);
		}
	}

	ಕೋಷ್ಟಕ->columns = ಲಂಬಸಾಲು_info.ಅಂಕೆ_columns;
	ಕೋಷ್ಟಕ->ಅಡ್ಡಸಾಲುಗಳು = ಲಂಬಸಾಲು_info.ಅಂಕೆ_ಅಡ್ಡಸಾಲುಗಳು;

	ಆದರೆ (ಕೋಷ್ಟಕ->children == ಖಾಲಿ) {
		ರಚನೆ box *ಅಡ್ಡಸಾಲು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#ಆದರೆ_ಅಂತ್ಯ

		ಪ್ರತಿಜ್ಞಾ(ಕೋಷ್ಟಕ->style != ಖಾಲಿ);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ಬಹುಲಕ_ತುಂಬಿದ);
		ctx.ತಳ_url = c->ತಳ_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				ಕೋಷ್ಟಕ->style);
		ಆದರೆ (style == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು_info.spans);
			ಹಿಂತಿರುಗಿ false;
		}

		ಅಡ್ಡಸಾಲು_ಗುಂಪು = box_create(ಖಾಲಿ, style, true, ಕೋಷ್ಟಕ->href,
				ಕೋಷ್ಟಕ->ಗುರಿ, ಖಾಲಿ, ಖಾಲಿ, c->bctx);
		ಆದರೆ (ಅಡ್ಡಸಾಲು_ಗುಂಪು == ಖಾಲಿ) {
			css_computed_style_destroy(style);
			ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು_info.spans);
			ಹಿಂತಿರುಗಿ false;
		}
		ಅಡ್ಡಸಾಲು_ಗುಂಪು->type = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				ಅಡ್ಡಸಾಲು_ಗುಂಪು->style);
		ಆದರೆ (style == ಖಾಲಿ) {
			box_free(ಅಡ್ಡಸಾಲು_ಗುಂಪು);
			ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು_info.spans);
			ಹಿಂತಿರುಗಿ false;
		}

		ಅಡ್ಡಸಾಲು = box_create(ಖಾಲಿ, style, true, ಅಡ್ಡಸಾಲು_ಗುಂಪು->href,
				ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಗುರಿ, ಖಾಲಿ, ಖಾಲಿ, c->bctx);
		ಆದರೆ (ಅಡ್ಡಸಾಲು == ಖಾಲಿ) {
			css_computed_style_destroy(style);
			box_free(ಅಡ್ಡಸಾಲು_ಗುಂಪು);
			ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು_info.spans);
			ಹಿಂತಿರುಗಿ false;
		}
		ಅಡ್ಡಸಾಲು->type = BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು;

		ಅಡ್ಡಸಾಲು->ಪೋಷಕ = ಅಡ್ಡಸಾಲು_ಗುಂಪು;
		ಅಡ್ಡಸಾಲು_ಗುಂಪು->children = ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಕೊನೆಯ = ಅಡ್ಡಸಾಲು;

		ಅಡ್ಡಸಾಲು_ಗುಂಪು->ಪೋಷಕ = ಕೋಷ್ಟಕ;
		ಕೋಷ್ಟಕ->children = ಕೋಷ್ಟಕ->ಕೊನೆಯ = ಅಡ್ಡಸಾಲು_ಗುಂಪು;

		ಕೋಷ್ಟಕ->ಅಡ್ಡಸಾಲುಗಳು = 1;
	}

	ಆದರೆ (box_normalise_ಕೋಷ್ಟಕ_spans(ಕೋಷ್ಟಕ, ಬೇರು, ಲಂಬಸಾಲು_info.spans, c) == false) {
		ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು_info.spans);
		ಹಿಂತಿರುಗಿ false;
	}

	ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು_info.spans);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", ಕೋಷ್ಟಕ);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ bool box_normalise_flex(
		ರಚನೆ box *flex_container,
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *ಬೇರು,
		html_content *c)
{
	ರಚನೆ box *ಮಗು;
	ರಚನೆ box *ಮುಂದಿನ_ಮಗು;
	ರಚನೆ box *implied_flex_ಅಂಶ;
	css_computed_style *style;
	nscss_select_ctx ctx;

	ಪ್ರತಿಜ್ಞಾ(flex_container != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(ಬೇರು != ಖಾಲಿ);

	ctx.ಬೇರು_style = ಬೇರು->style;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#ಆದರೆ_ಅಂತ್ಯ

	ಪ್ರತಿಜ್ಞಾ(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	ಗಾಗಿ (ಮಗು = flex_container->children; ಮಗು != ಖಾಲಿ; ಮಗು = ಮುಂದಿನ_ಮಗು) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				ಮಗು, ಮಗು->type);
#ಆದರೆ_ಅಂತ್ಯ

		ಮುಂದಿನ_ಮಗು = ಮಗು->ಮುಂದಿನ;	/* child may be destroyed */

		ಆಯ್ಕೆ (ಮಗು->type) {
		ಸ್ಥಿತಿ BOX_FLEX:
			/* ok */
			ಆದರೆ (box_normalise_flex(ಮಗು, ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_ಖಂಡ:
			/* ok */
			ಆದರೆ (box_normalise_ಖಂಡ(ಮಗು, ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			ಪ್ರತಿಜ್ಞಾ(flex_container->style != ಖಾಲಿ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ಬಹುಲಕ_ತುಂಬಿದ);
			ctx.ತಳ_url = c->ತಳ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			ಆದರೆ (style == ಖಾಲಿ)
				ಹಿಂತಿರುಗಿ false;

			implied_flex_ಅಂಶ = box_create(ಖಾಲಿ, style, true,
					flex_container->href,
					flex_container->ಗುರಿ,
					ಖಾಲಿ, ಖಾಲಿ, c->bctx);
			ಆದರೆ (implied_flex_ಅಂಶ == ಖಾಲಿ) {
				css_computed_style_destroy(style);
				ಹಿಂತಿರುಗಿ false;
			}
			implied_flex_ಅಂಶ->type = BOX_ಖಂಡ;

			ಆದರೆ (ಮಗು->prev == ಖಾಲಿ)
				flex_container->children = implied_flex_ಅಂಶ;
			ಇಲ್ಲದಿದ್ದರೆ
				ಮಗು->prev->ಮುಂದಿನ = implied_flex_ಅಂಶ;

			implied_flex_ಅಂಶ->prev = ಮಗು->prev;

			ಆಗ (ಮಗು != ಖಾಲಿ &&
					ಮಗು->type == BOX_INLINE_CONTAINER) {
				box_ಕೂಡಿಸು_ಮಗು(implied_flex_ಅಂಶ, ಮಗು);

				ಮುಂದಿನ_ಮಗು = ಮಗು->ಮುಂದಿನ;
				ಮಗು->ಮುಂದಿನ = ಖಾಲಿ;
				ಮಗು = ಮುಂದಿನ_ಮಗು;
			}

			implied_flex_ಅಂಶ->ಕೊನೆಯ->ಮುಂದಿನ = ಖಾಲಿ;
			implied_flex_ಅಂಶ->ಮುಂದಿನ = ಮುಂದಿನ_ಮಗು = ಮಗು;
			ಆದರೆ (implied_flex_ಅಂಶ->ಮುಂದಿನ != ಖಾಲಿ)
				implied_flex_ಅಂಶ->ಮುಂದಿನ->prev = implied_flex_ಅಂಶ;
			ಇಲ್ಲದಿದ್ದರೆ
				flex_container->ಕೊನೆಯ = implied_flex_ಅಂಶ;
			implied_flex_ಅಂಶ->ಪೋಷಕ = flex_container;

			ಆದರೆ (box_normalise_ಖಂಡ(implied_flex_ಅಂಶ,
					ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ:
			ಆದರೆ (box_normalise_ಕೋಷ್ಟಕ(ಮಗು, ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_INLINE:
		ಸ್ಥಿತಿ BOX_INLINE_ಕೊನೆ:
		ಸ್ಥಿತಿ BOX_INLINE_FLEX:
		ಸ್ಥಿತಿ BOX_INLINE_ಖಂಡ:
		ಸ್ಥಿತಿ BOX_FLOAT_ಎಡ:
		ಸ್ಥಿತಿ BOX_FLOAT_ಬಲ:
		ಸ್ಥಿತಿ BOX_BR:
		ಸ್ಥಿತಿ BOX_ಪಠ್ಯ:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ಪ್ರತಿಜ್ಞಾ(0);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_CELL:
			/* insert implied table */
			ಪ್ರತಿಜ್ಞಾ(flex_container->style != ಖಾಲಿ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ಬಹುಲಕ_ತುಂಬಿದ);
			ctx.ತಳ_url = c->ತಳ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			ಆದರೆ (style == ಖಾಲಿ)
				ಹಿಂತಿರುಗಿ false;

			implied_flex_ಅಂಶ = box_create(ಖಾಲಿ, style, true,
					flex_container->href,
					flex_container->ಗುರಿ,
					ಖಾಲಿ, ಖಾಲಿ, c->bctx);
			ಆದರೆ (implied_flex_ಅಂಶ == ಖಾಲಿ) {
				css_computed_style_destroy(style);
				ಹಿಂತಿರುಗಿ false;
			}
			implied_flex_ಅಂಶ->type = BOX_ಕೋಷ್ಟಕ;

			ಆದರೆ (ಮಗು->prev == ಖಾಲಿ)
				flex_container->children = implied_flex_ಅಂಶ;
			ಇಲ್ಲದಿದ್ದರೆ
				ಮಗು->prev->ಮುಂದಿನ = implied_flex_ಅಂಶ;

			implied_flex_ಅಂಶ->prev = ಮಗು->prev;

			ಆಗ (ಮಗು != ಖಾಲಿ && (
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು ||
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು ||
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ_CELL)) {
				box_ಕೂಡಿಸು_ಮಗು(implied_flex_ಅಂಶ, ಮಗು);

				ಮುಂದಿನ_ಮಗು = ಮಗು->ಮುಂದಿನ;
				ಮಗು->ಮುಂದಿನ = ಖಾಲಿ;
				ಮಗು = ಮುಂದಿನ_ಮಗು;
			}

			implied_flex_ಅಂಶ->ಕೊನೆಯ->ಮುಂದಿನ = ಖಾಲಿ;
			implied_flex_ಅಂಶ->ಮುಂದಿನ = ಮುಂದಿನ_ಮಗು = ಮಗು;
			ಆದರೆ (implied_flex_ಅಂಶ->ಮುಂದಿನ != ಖಾಲಿ)
				implied_flex_ಅಂಶ->ಮುಂದಿನ->prev = implied_flex_ಅಂಶ;
			ಇಲ್ಲದಿದ್ದರೆ
				flex_container->ಕೊನೆಯ = implied_flex_ಅಂಶ;
			implied_flex_ಅಂಶ->ಪೋಷಕ = flex_container;

			ಆದರೆ (box_normalise_ಕೋಷ್ಟಕ(implied_flex_ಅಂಶ,
					ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಪ್ರತಿಜ್ಞಾ(0);
		}
	}

	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ bool
box_normalise_inline_container(ರಚನೆ box *cont,
			       ಸ್ಥಿರಾಂಕ ರಚನೆ box *ಬೇರು,
			       html_content * c)
{
	ರಚನೆ box *ಮಗು;
	ರಚನೆ box *ಮುಂದಿನ_ಮಗು;

	ಪ್ರತಿಜ್ಞಾ(cont != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(cont->type == BOX_INLINE_CONTAINER);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#ಆದರೆ_ಅಂತ್ಯ

	ಗಾಗಿ (ಮಗು = cont->children; ಮಗು != ಖಾಲಿ; ಮಗು = ಮುಂದಿನ_ಮಗು) {
		ಮುಂದಿನ_ಮಗು = ಮಗು->ಮುಂದಿನ;
		ಆಯ್ಕೆ (ಮಗು->type) {
		ಸ್ಥಿತಿ BOX_INLINE:
		ಸ್ಥಿತಿ BOX_INLINE_ಕೊನೆ:
		ಸ್ಥಿತಿ BOX_BR:
		ಸ್ಥಿತಿ BOX_ಪಠ್ಯ:
			/* ok */
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_INLINE_ಖಂಡ:
			/* ok */
			ಆದರೆ (box_normalise_ಖಂಡ(ಮಗು, ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_INLINE_FLEX:
			/* ok */
			ಆದರೆ (box_normalise_flex(ಮಗು, ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_FLOAT_ಎಡ:
		ಸ್ಥಿತಿ BOX_FLOAT_ಬಲ:
			/* ok */
			ಪ್ರತಿಜ್ಞಾ(ಮಗು->children != ಖಾಲಿ);

			ಆಯ್ಕೆ (ಮಗು->children->type) {
			ಸ್ಥಿತಿ BOX_ಖಂಡ:
				ಆದರೆ (box_normalise_ಖಂಡ(ಮಗು->children, ಬೇರು,
						c) == false)
					ಹಿಂತಿರುಗಿ false;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ:
				ಆದರೆ (box_normalise_ಕೋಷ್ಟಕ(ಮಗು->children, ಬೇರು,
						c) == false)
					ಹಿಂತಿರುಗಿ false;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ BOX_FLEX:
				ಆದರೆ (box_normalise_flex(ಮಗು->children, ಬೇರು,
						c) == false)
					ಹಿಂತಿರುಗಿ false;
				ನಿಲ್ಲಿಸು;
			ಪೂರ್ವನಿಯೋಜಿತ:
				ಪ್ರತಿಜ್ಞಾ(0);
			}

			ಆದರೆ (ಮಗು->children == ಖಾಲಿ) {
				/* the child has destroyed itself: remove float */
				ಆದರೆ (ಮಗು->prev == ಖಾಲಿ)
					ಮಗು->ಪೋಷಕ->children = ಮಗು->ಮುಂದಿನ;
				ಇಲ್ಲದಿದ್ದರೆ
					ಮಗು->prev->ಮುಂದಿನ = ಮಗು->ಮುಂದಿನ;
				ಆದರೆ (ಮಗು->ಮುಂದಿನ != ಖಾಲಿ)
					ಮಗು->ಮುಂದಿನ->prev = ಮಗು->prev;
				ಇಲ್ಲದಿದ್ದರೆ
					ಮಗು->ಪೋಷಕ->ಕೊನೆಯ = ಮಗು->prev;

				box_free(ಮಗು);
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_FLEX:
		ಸ್ಥಿತಿ BOX_ಖಂಡ:
		ಸ್ಥಿತಿ BOX_INLINE_CONTAINER:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_CELL:
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಪ್ರತಿಜ್ಞಾ(0);
		}
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_ಖಂಡ(ರಚನೆ box *ಖಂಡ, ಸ್ಥಿರಾಂಕ ರಚನೆ box *ಬೇರು, html_content *c)
{
	ರಚನೆ box *ಮಗು;
	ರಚನೆ box *ಮುಂದಿನ_ಮಗು;
	ರಚನೆ box *ಕೋಷ್ಟಕ;
	css_computed_style *style;
	nscss_select_ctx ctx;

	ಪ್ರತಿಜ್ಞಾ(ಖಂಡ != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(ಬೇರು != ಖಾಲಿ);

	ctx.ಬೇರು_style = ಬೇರು->style;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", ಖಂಡ, ಖಂಡ->type);
#ಆದರೆ_ಅಂತ್ಯ

	ಪ್ರತಿಜ್ಞಾ(ಖಂಡ->type == BOX_ಖಂಡ || ಖಂಡ->type == BOX_INLINE_ಖಂಡ ||
			ಖಂಡ->type == BOX_ಕೋಷ್ಟಕ_CELL);

	ಗಾಗಿ (ಮಗು = ಖಂಡ->children; ಮಗು != ಖಾಲಿ; ಮಗು = ಮುಂದಿನ_ಮಗು) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", ಮಗು,
		      ಮಗು->type);
#ಆದರೆ_ಅಂತ್ಯ

		ಮುಂದಿನ_ಮಗು = ಮಗು->ಮುಂದಿನ;	/* child may be destroyed */

		ಆಯ್ಕೆ (ಮಗು->type) {
		ಸ್ಥಿತಿ BOX_FLEX:
			/* ok */
			ಆದರೆ (box_normalise_flex(ಮಗು, ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_ಖಂಡ:
			/* ok */
			ಆದರೆ (box_normalise_ಖಂಡ(ಮಗು, ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_INLINE_CONTAINER:
			ಆದರೆ (box_normalise_inline_container(ಮಗು, ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ:
			ಆದರೆ (box_normalise_ಕೋಷ್ಟಕ(ಮಗು, ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_INLINE:
		ಸ್ಥಿತಿ BOX_INLINE_ಕೊನೆ:
		ಸ್ಥಿತಿ BOX_INLINE_FLEX:
		ಸ್ಥಿತಿ BOX_INLINE_ಖಂಡ:
		ಸ್ಥಿತಿ BOX_FLOAT_ಎಡ:
		ಸ್ಥಿತಿ BOX_FLOAT_ಬಲ:
		ಸ್ಥಿತಿ BOX_BR:
		ಸ್ಥಿತಿ BOX_ಪಠ್ಯ:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ಪ್ರತಿಜ್ಞಾ(0);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು:
		ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_CELL:
			/* insert implied table */
			ಪ್ರತಿಜ್ಞಾ(ಖಂಡ->style != ಖಾಲಿ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ಬಹುಲಕ_ತುಂಬಿದ);
			ctx.ತಳ_url = c->ತಳ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					ಖಂಡ->style);
			ಆದರೆ (style == ಖಾಲಿ)
				ಹಿಂತಿರುಗಿ false;

			ಕೋಷ್ಟಕ = box_create(ಖಾಲಿ, style, true, ಖಂಡ->href,
					ಖಂಡ->ಗುರಿ, ಖಾಲಿ, ಖಾಲಿ, c->bctx);
			ಆದರೆ (ಕೋಷ್ಟಕ == ಖಾಲಿ) {
				css_computed_style_destroy(style);
				ಹಿಂತಿರುಗಿ false;
			}
			ಕೋಷ್ಟಕ->type = BOX_ಕೋಷ್ಟಕ;

			ಆದರೆ (ಮಗು->prev == ಖಾಲಿ)
				ಖಂಡ->children = ಕೋಷ್ಟಕ;
			ಇಲ್ಲದಿದ್ದರೆ
				ಮಗು->prev->ಮುಂದಿನ = ಕೋಷ್ಟಕ;

			ಕೋಷ್ಟಕ->prev = ಮಗು->prev;

			ಆಗ (ಮಗು != ಖಾಲಿ && (
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು ||
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು ||
					ಮಗು->type == BOX_ಕೋಷ್ಟಕ_CELL)) {
				box_ಕೂಡಿಸು_ಮಗು(ಕೋಷ್ಟಕ, ಮಗು);

				ಮುಂದಿನ_ಮಗು = ಮಗು->ಮುಂದಿನ;
				ಮಗು->ಮುಂದಿನ = ಖಾಲಿ;
				ಮಗು = ಮುಂದಿನ_ಮಗು;
			}

			ಕೋಷ್ಟಕ->ಕೊನೆಯ->ಮುಂದಿನ = ಖಾಲಿ;
			ಕೋಷ್ಟಕ->ಮುಂದಿನ = ಮುಂದಿನ_ಮಗು = ಮಗು;
			ಆದರೆ (ಕೋಷ್ಟಕ->ಮುಂದಿನ != ಖಾಲಿ)
				ಕೋಷ್ಟಕ->ಮುಂದಿನ->prev = ಕೋಷ್ಟಕ;
			ಇಲ್ಲದಿದ್ದರೆ
				ಖಂಡ->ಕೊನೆಯ = ಕೋಷ್ಟಕ;
			ಕೋಷ್ಟಕ->ಪೋಷಕ = ಖಂಡ;

			ಆದರೆ (box_normalise_ಕೋಷ್ಟಕ(ಕೋಷ್ಟಕ, ಬೇರು, c) == false)
				ಹಿಂತಿರುಗಿ false;
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಪ್ರತಿಜ್ಞಾ(0);
		}
	}

	ಹಿಂತಿರುಗಿ true;
}
