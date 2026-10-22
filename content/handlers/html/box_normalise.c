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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/select.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/table.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_manipulate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_normalise.h"

/* Define to enable box normalise debug */
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
ਬਣਤਰ ਵਿਸਤਾਰ_info {
	/** Number of rows this cell spans */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੰਕਤੀ_ਵਿਸਤਾਰ;
	/** Row group of cell */
	ਬਣਤਰ box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_ਪੰਕਤੀ;
};

/**
 * Column record for a table
 */
ਬਣਤਰ columns {
	/** Current column index */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ current_ਕਾਲਮ;
	/** Number of columns in main part of table 1..max columns */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਅੰਕ_columns;
	/** Information about columns in main table, array [0, num_columns) */
	ਬਣਤਰ ਵਿਸਤਾਰ_info *spans;
	/** Number of rows in table */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਅੰਕ_ਪੰਕਤੀਆਂ;
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
ਸਥਿਰ bool
calculate_ਸਾਰਣੀ_ਪੰਕਤੀ(ਬਣਤਰ columns *ਖਾਨਾ_info,
		    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਖਾਨਾ_ਵਿਸਤਾਰ,
		    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੰਕਤੀ_ਵਿਸਤਾਰ,
		    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *ਸ਼ੁਰੂ_ਕਾਲਮ,
		    ਬਣਤਰ box *cell)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cell_ਸ਼ੁਰੂ_ਖਾਨਾ = ਖਾਨਾ_info->current_ਕਾਲਮ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cell_ਸਿਰਾ_ਖਾਨਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਬਣਤਰ ਵਿਸਤਾਰ_info *spans;
	ਬਣਤਰ box *rg = cell->ਮਾਪਾ->ਮਾਪਾ; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	ਜਦੋਂ (ਖਾਨਾ_info->spans[cell_ਸ਼ੁਰੂ_ਖਾਨਾ].ਪੰਕਤੀ_ਵਿਸਤਾਰ != 0 &&
			ਖਾਨਾ_info->spans[cell_ਸ਼ੁਰੂ_ਖਾਨਾ].rg == rg) {
		cell_ਸ਼ੁਰੂ_ਖਾਨਾ++;
	}

	/* Update current column with calculated start */
	ਖਾਨਾ_info->current_ਕਾਲਮ = cell_ਸ਼ੁਰੂ_ਖਾਨਾ;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	ਜੇ (ਖਾਨਾ_ਵਿਸਤਾਰ == 0)
		ਖਾਨਾ_ਵਿਸਤਾਰ = 1;

	cell_ਸਿਰਾ_ਖਾਨਾ = cell_ਸ਼ੁਰੂ_ਖਾਨਾ + ਖਾਨਾ_ਵਿਸਤਾਰ;

	ਜੇ (ਖਾਨਾ_info->ਅੰਕ_columns < cell_ਸਿਰਾ_ਖਾਨਾ) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(ਖਾਨਾ_info->spans,
				ਆਕਾਰ *spans * (cell_ਸਿਰਾ_ਖਾਨਾ + 1));
		ਜੇ (spans == ਨਿਰਾ)
			ਵਾਪਸ false;

		ਖਾਨਾ_info->spans = spans;
		ਖਾਨਾ_info->ਅੰਕ_columns = cell_ਸਿਰਾ_ਖਾਨਾ;

		/* Mark new final column as sentinel */
		ਖਾਨਾ_info->spans[cell_ਸਿਰਾ_ਖਾਨਾ].ਪੰਕਤੀ_ਵਿਸਤਾਰ = 0;
		ਖਾਨਾ_info->spans[cell_ਸਿਰਾ_ਖਾਨਾ].auto_ਪੰਕਤੀ = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	ਲਈ (i = cell_ਸ਼ੁਰੂ_ਖਾਨਾ; i < cell_ਸਿਰਾ_ਖਾਨਾ; i++) {
		ਖਾਨਾ_info->spans[i].ਪੰਕਤੀ_ਵਿਸਤਾਰ = (ਪੰਕਤੀ_ਵਿਸਤਾਰ == 0) ? 1 : ਪੰਕਤੀ_ਵਿਸਤਾਰ;
		ਖਾਨਾ_info->spans[i].auto_ਪੰਕਤੀ = (ਪੰਕਤੀ_ਵਿਸਤਾਰ == 0);
		ਖਾਨਾ_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	ਖਾਨਾ_info->current_ਕਾਲਮ = cell_ਸਿਰਾ_ਖਾਨਾ;

	*ਸ਼ੁਰੂ_ਕਾਲਮ = cell_ਸ਼ੁਰੂ_ਖਾਨਾ;

	ਵਾਪਸ true;
}


ਸਥਿਰ bool
box_normalise_ਸਾਰਣੀ_ਪੰਕਤੀ(ਬਣਤਰ box *ਪੰਕਤੀ,
			ਅਚਲ ਬਣਤਰ box *ਜੜ੍ਹ,
			ਬਣਤਰ columns *ਖਾਨਾ_info,
			html_content * c)
{
	ਬਣਤਰ box *ਬੱਚਾ;
	ਬਣਤਰ box *ਅਗਲਾ_ਬੱਚਾ;
	ਬਣਤਰ box *cell = ਨਿਰਾ;
	css_computed_style *style;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	nscss_select_ctx ctx;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਪੰਕਤੀ != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਪੰਕਤੀ->type == BOX_ਸਾਰਣੀ_ਪੰਕਤੀ);

	ctx.ਜੜ੍ਹ_style = ਜੜ੍ਹ->style;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", ਪੰਕਤੀ);
#ਜੇ_ਅੰਤ

	ਲਈ (ਬੱਚਾ = ਪੰਕਤੀ->children; ਬੱਚਾ != ਨਿਰਾ; ਬੱਚਾ = ਅਗਲਾ_ਬੱਚਾ) {
		ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;

		ਚੋਣ (ਬੱਚਾ->type) {
		ਹਾਲਤ BOX_ਸਾਰਣੀ_CELL:
			/* ok */
			ਜੇ (box_normalise_ਖੰਡ(ਬੱਚਾ, ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			cell = ਬੱਚਾ;
			ਰੋਕੋ;
		ਹਾਲਤ BOX_FLEX:
		ਹਾਲਤ BOX_ਖੰਡ:
		ਹਾਲਤ BOX_INLINE_CONTAINER:
		ਹਾਲਤ BOX_ਸਾਰਣੀ:
		ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ:
		ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ:
			/* insert implied table cell */
			ਪ੍ਰਤਿਜ੍ਞਾ(ਪੰਕਤੀ->style != ਨਿਰਾ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ਬਹੁਲਕ_ਭਰਿਆ);
			ctx.ਅਧਾਰ_url = c->ਅਧਾਰ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					ਪੰਕਤੀ->style);
			ਜੇ (style == ਨਿਰਾ)
				ਵਾਪਸ false;

			cell = box_create(ਨਿਰਾ, style, true, ਪੰਕਤੀ->href,
					ਪੰਕਤੀ->ਨਿਸ਼ਾਨਾ, ਨਿਰਾ, ਨਿਰਾ, c->bctx);
			ਜੇ (cell == ਨਿਰਾ) {
				css_computed_style_destroy(style);
				ਵਾਪਸ false;
			}
			cell->type = BOX_ਸਾਰਣੀ_CELL;

			ਜੇ (ਬੱਚਾ->prev == ਨਿਰਾ)
				ਪੰਕਤੀ->children = cell;
			ਨਹੀਂਤਾਂ
				ਬੱਚਾ->prev->ਅਗਲਾ = cell;

			cell->prev = ਬੱਚਾ->prev;

			ਜਦੋਂ (ਬੱਚਾ != ਨਿਰਾ && (
					ਬੱਚਾ->type == BOX_FLEX ||
					ਬੱਚਾ->type == BOX_ਖੰਡ ||
					ਬੱਚਾ->type == BOX_INLINE_CONTAINER ||
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ ||
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ ||
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ_ਪੰਕਤੀ)) {
				box_ਜੋੜੋ_ਬੱਚਾ(cell, ਬੱਚਾ);

				ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;
				ਬੱਚਾ->ਅਗਲਾ = ਨਿਰਾ;
				ਬੱਚਾ = ਅਗਲਾ_ਬੱਚਾ;
			}

			ਪ੍ਰਤਿਜ੍ਞਾ(cell->ਆਖਰੀ != ਨਿਰਾ);

			cell->ਆਖਰੀ->ਅਗਲਾ = ਨਿਰਾ;
			cell->ਅਗਲਾ = ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ;
			ਜੇ (cell->ਅਗਲਾ != ਨਿਰਾ)
				cell->ਅਗਲਾ->prev = cell;
			ਨਹੀਂਤਾਂ
				ਪੰਕਤੀ->ਆਖਰੀ = cell;
			cell->ਮਾਪਾ = ਪੰਕਤੀ;

			ਜੇ (box_normalise_ਖੰਡ(cell, ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਹਾਲਤ BOX_INLINE:
		ਹਾਲਤ BOX_INLINE_ਸਿਰਾ:
		ਹਾਲਤ BOX_INLINE_FLEX:
		ਹਾਲਤ BOX_INLINE_ਖੰਡ:
		ਹਾਲਤ BOX_FLOAT_ਖੱਬਾ:
		ਹਾਲਤ BOX_FLOAT_ਸੱਜਾ:
		ਹਾਲਤ BOX_BR:
		ਹਾਲਤ BOX_ਪਾਠ:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ਪ੍ਰਤਿਜ੍ਞਾ(0);
			ਰੋਕੋ;
		ਮੂਲ:
			ਪ੍ਰਤਿਜ੍ਞਾ(0);
		}

		ਜੇ (calculate_ਸਾਰਣੀ_ਪੰਕਤੀ(ਖਾਨਾ_info, cell->columns, cell->ਪੰਕਤੀਆਂ,
				&cell->ਸ਼ੁਰੂ_ਕਾਲਮ, cell) == false)
			ਵਾਪਸ false;
	}


	/* Update row spanning details for all columns */
	ਲਈ (i = 0; i < ਖਾਨਾ_info->ਅੰਕ_columns; i++) {
		ਜੇ (ਖਾਨਾ_info->spans[i].ਪੰਕਤੀ_ਵਿਸਤਾਰ != 0 &&
				ਖਾਨਾ_info->spans[i].auto_ਪੰਕਤੀ == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			ਖਾਨਾ_info->spans[i].ਪੰਕਤੀ_ਵਿਸਤਾਰ--;
		}
	}

	/* Reset current column for next row */
	ਖਾਨਾ_info->current_ਕਾਲਮ = 0;

	/* Increment row counter */
	ਖਾਨਾ_info->ਅੰਕ_ਪੰਕਤੀਆਂ++;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", ਪੰਕਤੀ);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ true;
}


ਸਥਿਰ bool
box_normalise_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ(ਬਣਤਰ box *ਪੰਕਤੀ_ਸਮੂਹ,
			      ਅਚਲ ਬਣਤਰ box *ਜੜ੍ਹ,
			      ਬਣਤਰ columns *ਖਾਨਾ_info,
			      html_content * c)
{
	ਬਣਤਰ box *ਬੱਚਾ;
	ਬਣਤਰ box *ਅਗਲਾ_ਬੱਚਾ;
	ਬਣਤਰ box *ਪੰਕਤੀ;
	css_computed_style *style;
	nscss_select_ctx ctx;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸਮੂਹ_ਪੰਕਤੀ_ਗਿਣਤੀ = 0;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਪੰਕਤੀ_ਸਮੂਹ != 0);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਪੰਕਤੀ_ਸਮੂਹ->type == BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ);

	ctx.ਜੜ੍ਹ_style = ਜੜ੍ਹ->style;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", ਪੰਕਤੀ_ਸਮੂਹ);
#ਜੇ_ਅੰਤ

	ਲਈ (ਬੱਚਾ = ਪੰਕਤੀ_ਸਮੂਹ->children; ਬੱਚਾ != ਨਿਰਾ; ਬੱਚਾ = ਅਗਲਾ_ਬੱਚਾ) {
		ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;

		ਚੋਣ (ਬੱਚਾ->type) {
		ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ:
			/* ok */
			ਸਮੂਹ_ਪੰਕਤੀ_ਗਿਣਤੀ++;
			ਜੇ (box_normalise_ਸਾਰਣੀ_ਪੰਕਤੀ(ਬੱਚਾ, ਜੜ੍ਹ, ਖਾਨਾ_info,
					c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਹਾਲਤ BOX_FLEX:
		ਹਾਲਤ BOX_ਖੰਡ:
		ਹਾਲਤ BOX_INLINE_CONTAINER:
		ਹਾਲਤ BOX_ਸਾਰਣੀ:
		ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ:
		ਹਾਲਤ BOX_ਸਾਰਣੀ_CELL:
			/* insert implied table row */
			ਪ੍ਰਤਿਜ੍ਞਾ(ਪੰਕਤੀ_ਸਮੂਹ->style != ਨਿਰਾ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ਬਹੁਲਕ_ਭਰਿਆ);
			ctx.ਅਧਾਰ_url = c->ਅਧਾਰ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					ਪੰਕਤੀ_ਸਮੂਹ->style);
			ਜੇ (style == ਨਿਰਾ)
				ਵਾਪਸ false;

			ਪੰਕਤੀ = box_create(ਨਿਰਾ, style, true, ਪੰਕਤੀ_ਸਮੂਹ->href,
					ਪੰਕਤੀ_ਸਮੂਹ->ਨਿਸ਼ਾਨਾ, ਨਿਰਾ, ਨਿਰਾ, c->bctx);
			ਜੇ (ਪੰਕਤੀ == ਨਿਰਾ) {
				css_computed_style_destroy(style);
				ਵਾਪਸ false;
			}
			ਪੰਕਤੀ->type = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ;

			ਜੇ (ਬੱਚਾ->prev == ਨਿਰਾ)
				ਪੰਕਤੀ_ਸਮੂਹ->children = ਪੰਕਤੀ;
			ਨਹੀਂਤਾਂ
				ਬੱਚਾ->prev->ਅਗਲਾ = ਪੰਕਤੀ;

			ਪੰਕਤੀ->prev = ਬੱਚਾ->prev;

			ਜਦੋਂ (ਬੱਚਾ != ਨਿਰਾ && (
					ਬੱਚਾ->type == BOX_FLEX ||
					ਬੱਚਾ->type == BOX_ਖੰਡ ||
					ਬੱਚਾ->type == BOX_INLINE_CONTAINER ||
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ ||
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ ||
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ_CELL)) {
				box_ਜੋੜੋ_ਬੱਚਾ(ਪੰਕਤੀ, ਬੱਚਾ);

				ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;
				ਬੱਚਾ->ਅਗਲਾ = ਨਿਰਾ;
				ਬੱਚਾ = ਅਗਲਾ_ਬੱਚਾ;
			}

			ਪ੍ਰਤਿਜ੍ਞਾ(ਪੰਕਤੀ->ਆਖਰੀ != ਨਿਰਾ);

			ਪੰਕਤੀ->ਆਖਰੀ->ਅਗਲਾ = ਨਿਰਾ;
			ਪੰਕਤੀ->ਅਗਲਾ = ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ;
			ਜੇ (ਪੰਕਤੀ->ਅਗਲਾ != ਨਿਰਾ)
				ਪੰਕਤੀ->ਅਗਲਾ->prev = ਪੰਕਤੀ;
			ਨਹੀਂਤਾਂ
				ਪੰਕਤੀ_ਸਮੂਹ->ਆਖਰੀ = ਪੰਕਤੀ;
			ਪੰਕਤੀ->ਮਾਪਾ = ਪੰਕਤੀ_ਸਮੂਹ;

			ਸਮੂਹ_ਪੰਕਤੀ_ਗਿਣਤੀ++;
			ਜੇ (box_normalise_ਸਾਰਣੀ_ਪੰਕਤੀ(ਪੰਕਤੀ, ਜੜ੍ਹ, ਖਾਨਾ_info,
					c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਹਾਲਤ BOX_INLINE:
		ਹਾਲਤ BOX_INLINE_ਸਿਰਾ:
		ਹਾਲਤ BOX_INLINE_FLEX:
		ਹਾਲਤ BOX_INLINE_ਖੰਡ:
		ਹਾਲਤ BOX_FLOAT_ਖੱਬਾ:
		ਹਾਲਤ BOX_FLOAT_ਸੱਜਾ:
		ਹਾਲਤ BOX_BR:
		ਹਾਲਤ BOX_ਪਾਠ:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ਪ੍ਰਤਿਜ੍ਞਾ(0);
			ਰੋਕੋ;
		ਮੂਲ:
			ਪ੍ਰਤਿਜ੍ਞਾ(0);
		}
	}

	ਜੇ (ਪੰਕਤੀ_ਸਮੂਹ->children == ਨਿਰਾ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#ਜੇ_ਅੰਤ

		ਪ੍ਰਤਿਜ੍ਞਾ(ਪੰਕਤੀ_ਸਮੂਹ->style != ਨਿਰਾ);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ਬਹੁਲਕ_ਭਰਿਆ);
		ctx.ਅਧਾਰ_url = c->ਅਧਾਰ_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				ਪੰਕਤੀ_ਸਮੂਹ->style);
		ਜੇ (style == ਨਿਰਾ) {
			ਵਾਪਸ false;
		}

		ਪੰਕਤੀ = box_create(ਨਿਰਾ, style, true, ਪੰਕਤੀ_ਸਮੂਹ->href,
				ਪੰਕਤੀ_ਸਮੂਹ->ਨਿਸ਼ਾਨਾ, ਨਿਰਾ, ਨਿਰਾ, c->bctx);
		ਜੇ (ਪੰਕਤੀ == ਨਿਰਾ) {
			css_computed_style_destroy(style);
			ਵਾਪਸ false;
		}
		ਪੰਕਤੀ->type = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ;

		ਪੰਕਤੀ->ਮਾਪਾ = ਪੰਕਤੀ_ਸਮੂਹ;
		ਪੰਕਤੀ_ਸਮੂਹ->children = ਪੰਕਤੀ_ਸਮੂਹ->ਆਖਰੀ = ਪੰਕਤੀ;

		ਸਮੂਹ_ਪੰਕਤੀ_ਗਿਣਤੀ = 1;

		/* Keep table's row count in sync */
		ਖਾਨਾ_info->ਅੰਕ_ਪੰਕਤੀਆਂ++;
	}

	ਪੰਕਤੀ_ਸਮੂਹ->ਪੰਕਤੀਆਂ = ਸਮੂਹ_ਪੰਕਤੀ_ਗਿਣਤੀ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", ਪੰਕਤੀ_ਸਮੂਹ);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ true;
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
ਸਥਿਰ bool
box_normalise_ਸਾਰਣੀ_spans(ਬਣਤਰ box *ਸਾਰਣੀ,
			  ਅਚਲ ਬਣਤਰ box *ਜੜ੍ਹ,
			  ਬਣਤਰ ਵਿਸਤਾਰ_info *spans,
			  html_content *c)
{
	ਬਣਤਰ box *ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ;
	ਬਣਤਰ box *ਸਾਰਣੀ_ਪੰਕਤੀ;
	ਬਣਤਰ box *ਸਾਰਣੀ_cell;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੰਕਤੀਆਂ_ਖੱਬਾ = ਸਾਰਣੀ->ਪੰਕਤੀਆਂ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸਮੂਹ_ਪੰਕਤੀਆਂ_ਖੱਬਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਖਾਨਾ;
	nscss_select_ctx ctx;

	ctx.ਜੜ੍ਹ_style = ਜੜ੍ਹ->style;

	/* Clear span data */
	memset(spans, 0, ਸਾਰਣੀ->columns * ਆਕਾਰ(ਬਣਤਰ ਵਿਸਤਾਰ_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	ਲਈ (ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ = ਸਾਰਣੀ->children;
	     ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ != ਨਿਰਾ;
	     ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ = ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ->ਅਗਲਾ) {

		ਸਮੂਹ_ਪੰਕਤੀਆਂ_ਖੱਬਾ = ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ->ਪੰਕਤੀਆਂ;

		ਲਈ (ਸਾਰਣੀ_ਪੰਕਤੀ = ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ->children;
		     ਸਾਰਣੀ_ਪੰਕਤੀ != ਨਿਰਾ;
		     ਸਾਰਣੀ_ਪੰਕਤੀ = ਸਾਰਣੀ_ਪੰਕਤੀ->ਅਗਲਾ) {

			ਲਈ (ਸਾਰਣੀ_cell = ਸਾਰਣੀ_ਪੰਕਤੀ->children;
			     ਸਾਰਣੀ_cell != ਨਿਰਾ;
			     ਸਾਰਣੀ_cell = ਸਾਰਣੀ_cell->ਅਗਲਾ) {

				/* colspan = 0 -> colspan = 1 */
				ਜੇ (ਸਾਰਣੀ_cell->columns == 0) {
					ਸਾਰਣੀ_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				ਜੇ (ਸਾਰਣੀ_cell->ਪੰਕਤੀਆਂ == 0) {
					ਸਾਰਣੀ_cell->ਪੰਕਤੀਆਂ = ਸਮੂਹ_ਪੰਕਤੀਆਂ_ਖੱਬਾ;
				}

				/* limit rowspans within group */
				ਜੇ (ਸਾਰਣੀ_cell->ਪੰਕਤੀਆਂ > ਸਮੂਹ_ਪੰਕਤੀਆਂ_ਖੱਬਾ) {
					ਸਾਰਣੀ_cell->ਪੰਕਤੀਆਂ = ਸਮੂਹ_ਪੰਕਤੀਆਂ_ਖੱਬਾ;
				}

				/* Record span information */
				ਲਈ (ਖਾਨਾ = ਸਾਰਣੀ_cell->ਸ਼ੁਰੂ_ਕਾਲਮ;
						ਖਾਨਾ < ਸਾਰਣੀ_cell->ਸ਼ੁਰੂ_ਕਾਲਮ +
						ਸਾਰਣੀ_cell->columns; ਖਾਨਾ++) {
					spans[ਖਾਨਾ].ਪੰਕਤੀ_ਵਿਸਤਾਰ = ਸਾਰਣੀ_cell->ਪੰਕਤੀਆਂ;
				}
			}

			/* Reduce span count of each column */
			ਲਈ (ਖਾਨਾ = 0; ਖਾਨਾ < ਸਾਰਣੀ->columns; ਖਾਨਾ++) {
				ਜੇ (spans[ਖਾਨਾ].ਪੰਕਤੀ_ਵਿਸਤਾਰ == 0) {
					ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸ਼ੁਰੂ = ਖਾਨਾ;
					css_computed_style *style;
					ਬਣਤਰ box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					ਪ੍ਰਤਿਜ੍ਞਾ(ਸਾਰਣੀ_ਪੰਕਤੀ->style != ਨਿਰਾ);

					/* Find width of gap */
					ਜਦੋਂ (ਖਾਨਾ < ਸਾਰਣੀ->columns &&
							spans[ਖਾਨਾ].ਪੰਕਤੀ_ਵਿਸਤਾਰ ==
							0) {
						ਖਾਨਾ++;
					}

					ctx.ctx = c->select_ctx;
					ctx.quirks = (c->quirks ==
						DOM_DOCUMENT_QUIRKS_ਬਹੁਲਕ_ਭਰਿਆ);
					ctx.ਅਧਾਰ_url = c->ਅਧਾਰ_url;
					ctx.universal = c->universal;

					style = nscss_get_blank_style(&ctx,
							&c->unit_len_ctx,
							ਸਾਰਣੀ_ਪੰਕਤੀ->style);
					ਜੇ (style == ਨਿਰਾ)
						ਵਾਪਸ false;

					cell = box_create(ਨਿਰਾ, style, true,
							ਸਾਰਣੀ_ਪੰਕਤੀ->href,
							ਸਾਰਣੀ_ਪੰਕਤੀ->ਨਿਸ਼ਾਨਾ,
							ਨਿਰਾ, ਨਿਰਾ, c->bctx);
					ਜੇ (cell == ਨਿਰਾ) {
						css_computed_style_destroy(
								style);
						ਵਾਪਸ false;
					}
					cell->type = BOX_ਸਾਰਣੀ_CELL;

					cell->ਪੰਕਤੀਆਂ = 1;
					cell->columns = ਖਾਨਾ - ਸ਼ੁਰੂ;
					cell->ਸ਼ੁਰੂ_ਕਾਲਮ = ਸ਼ੁਰੂ;

					/* Find place to insert cell */
					ਲਈ (prev = ਸਾਰਣੀ_ਪੰਕਤੀ->children;
							prev != ਨਿਰਾ;
							prev = prev->ਅਗਲਾ) {
						ਜੇ (prev->ਸ਼ੁਰੂ_ਕਾਲਮ +
							prev->columns ==
								ਸ਼ੁਰੂ)
							ਰੋਕੋ;
						ਜੇ (prev->ਅਗਲਾ == ਨਿਰਾ)
							ਰੋਕੋ;
					}

					/* Insert it */
					ਜੇ (prev == ਨਿਰਾ) {
						ਜੇ (ਸਾਰਣੀ_ਪੰਕਤੀ->children != ਨਿਰਾ)
							ਸਾਰਣੀ_ਪੰਕਤੀ->children->
								prev = cell;
						ਨਹੀਂਤਾਂ
							ਸਾਰਣੀ_ਪੰਕਤੀ->ਆਖਰੀ = cell;

						cell->ਅਗਲਾ =
							ਸਾਰਣੀ_ਪੰਕਤੀ->children;
						ਸਾਰਣੀ_ਪੰਕਤੀ->children = cell;
					} ਨਹੀਂਤਾਂ {
						ਜੇ (prev->ਅਗਲਾ != ਨਿਰਾ)
							prev->ਅਗਲਾ->prev = cell;
						ਨਹੀਂਤਾਂ
							ਸਾਰਣੀ_ਪੰਕਤੀ->ਆਖਰੀ = cell;

						cell->ਅਗਲਾ = prev->ਅਗਲਾ;
						prev->ਅਗਲਾ = cell;
						cell->prev = prev;
					}
					cell->ਮਾਪਾ = ਸਾਰਣੀ_ਪੰਕਤੀ;
				} ਨਹੀਂਤਾਂ {
					spans[ਖਾਨਾ].ਪੰਕਤੀ_ਵਿਸਤਾਰ--;
				}
			}

			ਪ੍ਰਤਿਜ੍ਞਾ(ਪੰਕਤੀਆਂ_ਖੱਬਾ > 0);

			ਪੰਕਤੀਆਂ_ਖੱਬਾ--;
		}

		ਸਮੂਹ_ਪੰਕਤੀਆਂ_ਖੱਬਾ--;
	}

	ਵਾਪਸ true;
}


ਸਥਿਰ bool
box_normalise_ਸਾਰਣੀ(ਬਣਤਰ box *ਸਾਰਣੀ, ਅਚਲ ਬਣਤਰ box *ਜੜ੍ਹ, html_content * c)
{
	ਬਣਤਰ box *ਬੱਚਾ;
	ਬਣਤਰ box *ਅਗਲਾ_ਬੱਚਾ;
	ਬਣਤਰ box *ਪੰਕਤੀ_ਸਮੂਹ;
	css_computed_style *style;
	ਬਣਤਰ columns ਖਾਨਾ_info;
	nscss_select_ctx ctx;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਸਾਰਣੀ != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਸਾਰਣੀ->type == BOX_ਸਾਰਣੀ);

	ctx.ਜੜ੍ਹ_style = ਜੜ੍ਹ->style;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", ਸਾਰਣੀ);
#ਜੇ_ਅੰਤ

	ਖਾਨਾ_info.ਅੰਕ_columns = 1;
	ਖਾਨਾ_info.current_ਕਾਲਮ = 0;
	ਖਾਨਾ_info.spans = ਸਮ੍ਰਿਤੀ_ਦਿਓ(2 * ਆਕਾਰ *ਖਾਨਾ_info.spans);
	ਜੇ (ਖਾਨਾ_info.spans == ਨਿਰਾ)
		ਵਾਪਸ false;

	ਖਾਨਾ_info.spans[0].ਪੰਕਤੀ_ਵਿਸਤਾਰ = ਖਾਨਾ_info.spans[1].ਪੰਕਤੀ_ਵਿਸਤਾਰ = 0;
	ਖਾਨਾ_info.spans[0].auto_ਪੰਕਤੀ = false;
	ਖਾਨਾ_info.spans[1].auto_ਪੰਕਤੀ = false;
	ਖਾਨਾ_info.ਅੰਕ_ਪੰਕਤੀਆਂ = 0;

	ਲਈ (ਬੱਚਾ = ਸਾਰਣੀ->children; ਬੱਚਾ != ਨਿਰਾ; ਬੱਚਾ = ਅਗਲਾ_ਬੱਚਾ) {
		ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;
		ਚੋਣ (ਬੱਚਾ->type) {
		ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ:
			/* ok */
			ਜੇ (box_normalise_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ(ਬੱਚਾ, ਜੜ੍ਹ,
					&ਖਾਨਾ_info, c) == false) {
				ਮੁਕਤ(ਖਾਨਾ_info.spans);
				ਵਾਪਸ false;
			}
			ਰੋਕੋ;
		ਹਾਲਤ BOX_FLEX:
		ਹਾਲਤ BOX_ਖੰਡ:
		ਹਾਲਤ BOX_INLINE_CONTAINER:
		ਹਾਲਤ BOX_ਸਾਰਣੀ:
		ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ:
		ਹਾਲਤ BOX_ਸਾਰਣੀ_CELL:
			/* insert implied table row group */
			ਪ੍ਰਤਿਜ੍ਞਾ(ਸਾਰਣੀ->style != ਨਿਰਾ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ਬਹੁਲਕ_ਭਰਿਆ);
			ctx.ਅਧਾਰ_url = c->ਅਧਾਰ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					ਸਾਰਣੀ->style);
			ਜੇ (style == ਨਿਰਾ) {
				ਮੁਕਤ(ਖਾਨਾ_info.spans);
				ਵਾਪਸ false;
			}

			ਪੰਕਤੀ_ਸਮੂਹ = box_create(ਨਿਰਾ, style, true, ਸਾਰਣੀ->href,
					ਸਾਰਣੀ->ਨਿਸ਼ਾਨਾ, ਨਿਰਾ, ਨਿਰਾ, c->bctx);
			ਜੇ (ਪੰਕਤੀ_ਸਮੂਹ == ਨਿਰਾ) {
				css_computed_style_destroy(style);
				ਮੁਕਤ(ਖਾਨਾ_info.spans);
				ਵਾਪਸ false;
			}

			ਪੰਕਤੀ_ਸਮੂਹ->type = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ;

			ਜੇ (ਬੱਚਾ->prev == ਨਿਰਾ)
				ਸਾਰਣੀ->children = ਪੰਕਤੀ_ਸਮੂਹ;
			ਨਹੀਂਤਾਂ
				ਬੱਚਾ->prev->ਅਗਲਾ = ਪੰਕਤੀ_ਸਮੂਹ;

			ਪੰਕਤੀ_ਸਮੂਹ->prev = ਬੱਚਾ->prev;

			ਜਦੋਂ (ਬੱਚਾ != ਨਿਰਾ && (
					ਬੱਚਾ->type == BOX_FLEX ||
					ਬੱਚਾ->type == BOX_ਖੰਡ ||
					ਬੱਚਾ->type == BOX_INLINE_CONTAINER ||
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ ||
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ_ਪੰਕਤੀ ||
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ_CELL)) {
				box_ਜੋੜੋ_ਬੱਚਾ(ਪੰਕਤੀ_ਸਮੂਹ, ਬੱਚਾ);

				ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;
				ਬੱਚਾ->ਅਗਲਾ = ਨਿਰਾ;
				ਬੱਚਾ = ਅਗਲਾ_ਬੱਚਾ;
			}

			ਪ੍ਰਤਿਜ੍ਞਾ(ਪੰਕਤੀ_ਸਮੂਹ->ਆਖਰੀ != ਨਿਰਾ);

			ਪੰਕਤੀ_ਸਮੂਹ->ਆਖਰੀ->ਅਗਲਾ = ਨਿਰਾ;
			ਪੰਕਤੀ_ਸਮੂਹ->ਅਗਲਾ = ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ;
			ਜੇ (ਪੰਕਤੀ_ਸਮੂਹ->ਅਗਲਾ != ਨਿਰਾ)
				ਪੰਕਤੀ_ਸਮੂਹ->ਅਗਲਾ->prev = ਪੰਕਤੀ_ਸਮੂਹ;
			ਨਹੀਂਤਾਂ
				ਸਾਰਣੀ->ਆਖਰੀ = ਪੰਕਤੀ_ਸਮੂਹ;
			ਪੰਕਤੀ_ਸਮੂਹ->ਮਾਪਾ = ਸਾਰਣੀ;

			ਜੇ (box_normalise_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ(ਪੰਕਤੀ_ਸਮੂਹ, ਜੜ੍ਹ,
					&ਖਾਨਾ_info, c) == false) {
				ਮੁਕਤ(ਖਾਨਾ_info.spans);
				ਵਾਪਸ false;
			}
			ਰੋਕੋ;
		ਹਾਲਤ BOX_INLINE:
		ਹਾਲਤ BOX_INLINE_ਸਿਰਾ:
		ਹਾਲਤ BOX_INLINE_FLEX:
		ਹਾਲਤ BOX_INLINE_ਖੰਡ:
		ਹਾਲਤ BOX_FLOAT_ਖੱਬਾ:
		ਹਾਲਤ BOX_FLOAT_ਸੱਜਾ:
		ਹਾਲਤ BOX_BR:
		ਹਾਲਤ BOX_ਪਾਠ:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ਪ੍ਰਤਿਜ੍ਞਾ(0);
			ਰੋਕੋ;
		ਮੂਲ:
			fprintf(stderr, "%i\n", ਬੱਚਾ->type);
			ਪ੍ਰਤਿਜ੍ਞਾ(0);
		}
	}

	ਸਾਰਣੀ->columns = ਖਾਨਾ_info.ਅੰਕ_columns;
	ਸਾਰਣੀ->ਪੰਕਤੀਆਂ = ਖਾਨਾ_info.ਅੰਕ_ਪੰਕਤੀਆਂ;

	ਜੇ (ਸਾਰਣੀ->children == ਨਿਰਾ) {
		ਬਣਤਰ box *ਪੰਕਤੀ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#ਜੇ_ਅੰਤ

		ਪ੍ਰਤਿਜ੍ਞਾ(ਸਾਰਣੀ->style != ਨਿਰਾ);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ਬਹੁਲਕ_ਭਰਿਆ);
		ctx.ਅਧਾਰ_url = c->ਅਧਾਰ_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				ਸਾਰਣੀ->style);
		ਜੇ (style == ਨਿਰਾ) {
			ਮੁਕਤ(ਖਾਨਾ_info.spans);
			ਵਾਪਸ false;
		}

		ਪੰਕਤੀ_ਸਮੂਹ = box_create(ਨਿਰਾ, style, true, ਸਾਰਣੀ->href,
				ਸਾਰਣੀ->ਨਿਸ਼ਾਨਾ, ਨਿਰਾ, ਨਿਰਾ, c->bctx);
		ਜੇ (ਪੰਕਤੀ_ਸਮੂਹ == ਨਿਰਾ) {
			css_computed_style_destroy(style);
			ਮੁਕਤ(ਖਾਨਾ_info.spans);
			ਵਾਪਸ false;
		}
		ਪੰਕਤੀ_ਸਮੂਹ->type = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				ਪੰਕਤੀ_ਸਮੂਹ->style);
		ਜੇ (style == ਨਿਰਾ) {
			box_free(ਪੰਕਤੀ_ਸਮੂਹ);
			ਮੁਕਤ(ਖਾਨਾ_info.spans);
			ਵਾਪਸ false;
		}

		ਪੰਕਤੀ = box_create(ਨਿਰਾ, style, true, ਪੰਕਤੀ_ਸਮੂਹ->href,
				ਪੰਕਤੀ_ਸਮੂਹ->ਨਿਸ਼ਾਨਾ, ਨਿਰਾ, ਨਿਰਾ, c->bctx);
		ਜੇ (ਪੰਕਤੀ == ਨਿਰਾ) {
			css_computed_style_destroy(style);
			box_free(ਪੰਕਤੀ_ਸਮੂਹ);
			ਮੁਕਤ(ਖਾਨਾ_info.spans);
			ਵਾਪਸ false;
		}
		ਪੰਕਤੀ->type = BOX_ਸਾਰਣੀ_ਪੰਕਤੀ;

		ਪੰਕਤੀ->ਮਾਪਾ = ਪੰਕਤੀ_ਸਮੂਹ;
		ਪੰਕਤੀ_ਸਮੂਹ->children = ਪੰਕਤੀ_ਸਮੂਹ->ਆਖਰੀ = ਪੰਕਤੀ;

		ਪੰਕਤੀ_ਸਮੂਹ->ਮਾਪਾ = ਸਾਰਣੀ;
		ਸਾਰਣੀ->children = ਸਾਰਣੀ->ਆਖਰੀ = ਪੰਕਤੀ_ਸਮੂਹ;

		ਸਾਰਣੀ->ਪੰਕਤੀਆਂ = 1;
	}

	ਜੇ (box_normalise_ਸਾਰਣੀ_spans(ਸਾਰਣੀ, ਜੜ੍ਹ, ਖਾਨਾ_info.spans, c) == false) {
		ਮੁਕਤ(ਖਾਨਾ_info.spans);
		ਵਾਪਸ false;
	}

	ਮੁਕਤ(ਖਾਨਾ_info.spans);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", ਸਾਰਣੀ);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ true;
}

ਸਥਿਰ bool box_normalise_flex(
		ਬਣਤਰ box *flex_container,
		ਅਚਲ ਬਣਤਰ box *ਜੜ੍ਹ,
		html_content *c)
{
	ਬਣਤਰ box *ਬੱਚਾ;
	ਬਣਤਰ box *ਅਗਲਾ_ਬੱਚਾ;
	ਬਣਤਰ box *implied_flex_ਵਸਤੂ;
	css_computed_style *style;
	nscss_select_ctx ctx;

	ਪ੍ਰਤਿਜ੍ਞਾ(flex_container != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਜੜ੍ਹ != ਨਿਰਾ);

	ctx.ਜੜ੍ਹ_style = ਜੜ੍ਹ->style;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#ਜੇ_ਅੰਤ

	ਪ੍ਰਤਿਜ੍ਞਾ(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	ਲਈ (ਬੱਚਾ = flex_container->children; ਬੱਚਾ != ਨਿਰਾ; ਬੱਚਾ = ਅਗਲਾ_ਬੱਚਾ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				ਬੱਚਾ, ਬੱਚਾ->type);
#ਜੇ_ਅੰਤ

		ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;	/* child may be destroyed */

		ਚੋਣ (ਬੱਚਾ->type) {
		ਹਾਲਤ BOX_FLEX:
			/* ok */
			ਜੇ (box_normalise_flex(ਬੱਚਾ, ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਹਾਲਤ BOX_ਖੰਡ:
			/* ok */
			ਜੇ (box_normalise_ਖੰਡ(ਬੱਚਾ, ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਹਾਲਤ BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			ਪ੍ਰਤਿਜ੍ਞਾ(flex_container->style != ਨਿਰਾ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ਬਹੁਲਕ_ਭਰਿਆ);
			ctx.ਅਧਾਰ_url = c->ਅਧਾਰ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			ਜੇ (style == ਨਿਰਾ)
				ਵਾਪਸ false;

			implied_flex_ਵਸਤੂ = box_create(ਨਿਰਾ, style, true,
					flex_container->href,
					flex_container->ਨਿਸ਼ਾਨਾ,
					ਨਿਰਾ, ਨਿਰਾ, c->bctx);
			ਜੇ (implied_flex_ਵਸਤੂ == ਨਿਰਾ) {
				css_computed_style_destroy(style);
				ਵਾਪਸ false;
			}
			implied_flex_ਵਸਤੂ->type = BOX_ਖੰਡ;

			ਜੇ (ਬੱਚਾ->prev == ਨਿਰਾ)
				flex_container->children = implied_flex_ਵਸਤੂ;
			ਨਹੀਂਤਾਂ
				ਬੱਚਾ->prev->ਅਗਲਾ = implied_flex_ਵਸਤੂ;

			implied_flex_ਵਸਤੂ->prev = ਬੱਚਾ->prev;

			ਜਦੋਂ (ਬੱਚਾ != ਨਿਰਾ &&
					ਬੱਚਾ->type == BOX_INLINE_CONTAINER) {
				box_ਜੋੜੋ_ਬੱਚਾ(implied_flex_ਵਸਤੂ, ਬੱਚਾ);

				ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;
				ਬੱਚਾ->ਅਗਲਾ = ਨਿਰਾ;
				ਬੱਚਾ = ਅਗਲਾ_ਬੱਚਾ;
			}

			implied_flex_ਵਸਤੂ->ਆਖਰੀ->ਅਗਲਾ = ਨਿਰਾ;
			implied_flex_ਵਸਤੂ->ਅਗਲਾ = ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ;
			ਜੇ (implied_flex_ਵਸਤੂ->ਅਗਲਾ != ਨਿਰਾ)
				implied_flex_ਵਸਤੂ->ਅਗਲਾ->prev = implied_flex_ਵਸਤੂ;
			ਨਹੀਂਤਾਂ
				flex_container->ਆਖਰੀ = implied_flex_ਵਸਤੂ;
			implied_flex_ਵਸਤੂ->ਮਾਪਾ = flex_container;

			ਜੇ (box_normalise_ਖੰਡ(implied_flex_ਵਸਤੂ,
					ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;

		ਹਾਲਤ BOX_ਸਾਰਣੀ:
			ਜੇ (box_normalise_ਸਾਰਣੀ(ਬੱਚਾ, ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਹਾਲਤ BOX_INLINE:
		ਹਾਲਤ BOX_INLINE_ਸਿਰਾ:
		ਹਾਲਤ BOX_INLINE_FLEX:
		ਹਾਲਤ BOX_INLINE_ਖੰਡ:
		ਹਾਲਤ BOX_FLOAT_ਖੱਬਾ:
		ਹਾਲਤ BOX_FLOAT_ਸੱਜਾ:
		ਹਾਲਤ BOX_BR:
		ਹਾਲਤ BOX_ਪਾਠ:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ਪ੍ਰਤਿਜ੍ਞਾ(0);
			ਰੋਕੋ;
		ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ:
		ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ:
		ਹਾਲਤ BOX_ਸਾਰਣੀ_CELL:
			/* insert implied table */
			ਪ੍ਰਤਿਜ੍ਞਾ(flex_container->style != ਨਿਰਾ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ਬਹੁਲਕ_ਭਰਿਆ);
			ctx.ਅਧਾਰ_url = c->ਅਧਾਰ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			ਜੇ (style == ਨਿਰਾ)
				ਵਾਪਸ false;

			implied_flex_ਵਸਤੂ = box_create(ਨਿਰਾ, style, true,
					flex_container->href,
					flex_container->ਨਿਸ਼ਾਨਾ,
					ਨਿਰਾ, ਨਿਰਾ, c->bctx);
			ਜੇ (implied_flex_ਵਸਤੂ == ਨਿਰਾ) {
				css_computed_style_destroy(style);
				ਵਾਪਸ false;
			}
			implied_flex_ਵਸਤੂ->type = BOX_ਸਾਰਣੀ;

			ਜੇ (ਬੱਚਾ->prev == ਨਿਰਾ)
				flex_container->children = implied_flex_ਵਸਤੂ;
			ਨਹੀਂਤਾਂ
				ਬੱਚਾ->prev->ਅਗਲਾ = implied_flex_ਵਸਤੂ;

			implied_flex_ਵਸਤੂ->prev = ਬੱਚਾ->prev;

			ਜਦੋਂ (ਬੱਚਾ != ਨਿਰਾ && (
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ ||
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ_ਪੰਕਤੀ ||
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ_CELL)) {
				box_ਜੋੜੋ_ਬੱਚਾ(implied_flex_ਵਸਤੂ, ਬੱਚਾ);

				ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;
				ਬੱਚਾ->ਅਗਲਾ = ਨਿਰਾ;
				ਬੱਚਾ = ਅਗਲਾ_ਬੱਚਾ;
			}

			implied_flex_ਵਸਤੂ->ਆਖਰੀ->ਅਗਲਾ = ਨਿਰਾ;
			implied_flex_ਵਸਤੂ->ਅਗਲਾ = ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ;
			ਜੇ (implied_flex_ਵਸਤੂ->ਅਗਲਾ != ਨਿਰਾ)
				implied_flex_ਵਸਤੂ->ਅਗਲਾ->prev = implied_flex_ਵਸਤੂ;
			ਨਹੀਂਤਾਂ
				flex_container->ਆਖਰੀ = implied_flex_ਵਸਤੂ;
			implied_flex_ਵਸਤੂ->ਮਾਪਾ = flex_container;

			ਜੇ (box_normalise_ਸਾਰਣੀ(implied_flex_ਵਸਤੂ,
					ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਮੂਲ:
			ਪ੍ਰਤਿਜ੍ਞਾ(0);
		}
	}

	ਵਾਪਸ true;
}

ਸਥਿਰ bool
box_normalise_inline_container(ਬਣਤਰ box *cont,
			       ਅਚਲ ਬਣਤਰ box *ਜੜ੍ਹ,
			       html_content * c)
{
	ਬਣਤਰ box *ਬੱਚਾ;
	ਬਣਤਰ box *ਅਗਲਾ_ਬੱਚਾ;

	ਪ੍ਰਤਿਜ੍ਞਾ(cont != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(cont->type == BOX_INLINE_CONTAINER);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#ਜੇ_ਅੰਤ

	ਲਈ (ਬੱਚਾ = cont->children; ਬੱਚਾ != ਨਿਰਾ; ਬੱਚਾ = ਅਗਲਾ_ਬੱਚਾ) {
		ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;
		ਚੋਣ (ਬੱਚਾ->type) {
		ਹਾਲਤ BOX_INLINE:
		ਹਾਲਤ BOX_INLINE_ਸਿਰਾ:
		ਹਾਲਤ BOX_BR:
		ਹਾਲਤ BOX_ਪਾਠ:
			/* ok */
			ਰੋਕੋ;
		ਹਾਲਤ BOX_INLINE_ਖੰਡ:
			/* ok */
			ਜੇ (box_normalise_ਖੰਡ(ਬੱਚਾ, ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਹਾਲਤ BOX_INLINE_FLEX:
			/* ok */
			ਜੇ (box_normalise_flex(ਬੱਚਾ, ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਹਾਲਤ BOX_FLOAT_ਖੱਬਾ:
		ਹਾਲਤ BOX_FLOAT_ਸੱਜਾ:
			/* ok */
			ਪ੍ਰਤਿਜ੍ਞਾ(ਬੱਚਾ->children != ਨਿਰਾ);

			ਚੋਣ (ਬੱਚਾ->children->type) {
			ਹਾਲਤ BOX_ਖੰਡ:
				ਜੇ (box_normalise_ਖੰਡ(ਬੱਚਾ->children, ਜੜ੍ਹ,
						c) == false)
					ਵਾਪਸ false;
				ਰੋਕੋ;
			ਹਾਲਤ BOX_ਸਾਰਣੀ:
				ਜੇ (box_normalise_ਸਾਰਣੀ(ਬੱਚਾ->children, ਜੜ੍ਹ,
						c) == false)
					ਵਾਪਸ false;
				ਰੋਕੋ;
			ਹਾਲਤ BOX_FLEX:
				ਜੇ (box_normalise_flex(ਬੱਚਾ->children, ਜੜ੍ਹ,
						c) == false)
					ਵਾਪਸ false;
				ਰੋਕੋ;
			ਮੂਲ:
				ਪ੍ਰਤਿਜ੍ਞਾ(0);
			}

			ਜੇ (ਬੱਚਾ->children == ਨਿਰਾ) {
				/* the child has destroyed itself: remove float */
				ਜੇ (ਬੱਚਾ->prev == ਨਿਰਾ)
					ਬੱਚਾ->ਮਾਪਾ->children = ਬੱਚਾ->ਅਗਲਾ;
				ਨਹੀਂਤਾਂ
					ਬੱਚਾ->prev->ਅਗਲਾ = ਬੱਚਾ->ਅਗਲਾ;
				ਜੇ (ਬੱਚਾ->ਅਗਲਾ != ਨਿਰਾ)
					ਬੱਚਾ->ਅਗਲਾ->prev = ਬੱਚਾ->prev;
				ਨਹੀਂਤਾਂ
					ਬੱਚਾ->ਮਾਪਾ->ਆਖਰੀ = ਬੱਚਾ->prev;

				box_free(ਬੱਚਾ);
			}
			ਰੋਕੋ;
		ਹਾਲਤ BOX_FLEX:
		ਹਾਲਤ BOX_ਖੰਡ:
		ਹਾਲਤ BOX_INLINE_CONTAINER:
		ਹਾਲਤ BOX_ਸਾਰਣੀ:
		ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ:
		ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ:
		ਹਾਲਤ BOX_ਸਾਰਣੀ_CELL:
		ਮੂਲ:
			ਪ੍ਰਤਿਜ੍ਞਾ(0);
		}
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_ਖੰਡ(ਬਣਤਰ box *ਖੰਡ, ਅਚਲ ਬਣਤਰ box *ਜੜ੍ਹ, html_content *c)
{
	ਬਣਤਰ box *ਬੱਚਾ;
	ਬਣਤਰ box *ਅਗਲਾ_ਬੱਚਾ;
	ਬਣਤਰ box *ਸਾਰਣੀ;
	css_computed_style *style;
	nscss_select_ctx ctx;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਖੰਡ != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(ਜੜ੍ਹ != ਨਿਰਾ);

	ctx.ਜੜ੍ਹ_style = ਜੜ੍ਹ->style;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", ਖੰਡ, ਖੰਡ->type);
#ਜੇ_ਅੰਤ

	ਪ੍ਰਤਿਜ੍ਞਾ(ਖੰਡ->type == BOX_ਖੰਡ || ਖੰਡ->type == BOX_INLINE_ਖੰਡ ||
			ਖੰਡ->type == BOX_ਸਾਰਣੀ_CELL);

	ਲਈ (ਬੱਚਾ = ਖੰਡ->children; ਬੱਚਾ != ਨਿਰਾ; ਬੱਚਾ = ਅਗਲਾ_ਬੱਚਾ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", ਬੱਚਾ,
		      ਬੱਚਾ->type);
#ਜੇ_ਅੰਤ

		ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;	/* child may be destroyed */

		ਚੋਣ (ਬੱਚਾ->type) {
		ਹਾਲਤ BOX_FLEX:
			/* ok */
			ਜੇ (box_normalise_flex(ਬੱਚਾ, ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਹਾਲਤ BOX_ਖੰਡ:
			/* ok */
			ਜੇ (box_normalise_ਖੰਡ(ਬੱਚਾ, ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਹਾਲਤ BOX_INLINE_CONTAINER:
			ਜੇ (box_normalise_inline_container(ਬੱਚਾ, ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਹਾਲਤ BOX_ਸਾਰਣੀ:
			ਜੇ (box_normalise_ਸਾਰਣੀ(ਬੱਚਾ, ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਹਾਲਤ BOX_INLINE:
		ਹਾਲਤ BOX_INLINE_ਸਿਰਾ:
		ਹਾਲਤ BOX_INLINE_FLEX:
		ਹਾਲਤ BOX_INLINE_ਖੰਡ:
		ਹਾਲਤ BOX_FLOAT_ਖੱਬਾ:
		ਹਾਲਤ BOX_FLOAT_ਸੱਜਾ:
		ਹਾਲਤ BOX_BR:
		ਹਾਲਤ BOX_ਪਾਠ:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ਪ੍ਰਤਿਜ੍ਞਾ(0);
			ਰੋਕੋ;
		ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ:
		ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ:
		ਹਾਲਤ BOX_ਸਾਰਣੀ_CELL:
			/* insert implied table */
			ਪ੍ਰਤਿਜ੍ਞਾ(ਖੰਡ->style != ਨਿਰਾ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_ਬਹੁਲਕ_ਭਰਿਆ);
			ctx.ਅਧਾਰ_url = c->ਅਧਾਰ_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					ਖੰਡ->style);
			ਜੇ (style == ਨਿਰਾ)
				ਵਾਪਸ false;

			ਸਾਰਣੀ = box_create(ਨਿਰਾ, style, true, ਖੰਡ->href,
					ਖੰਡ->ਨਿਸ਼ਾਨਾ, ਨਿਰਾ, ਨਿਰਾ, c->bctx);
			ਜੇ (ਸਾਰਣੀ == ਨਿਰਾ) {
				css_computed_style_destroy(style);
				ਵਾਪਸ false;
			}
			ਸਾਰਣੀ->type = BOX_ਸਾਰਣੀ;

			ਜੇ (ਬੱਚਾ->prev == ਨਿਰਾ)
				ਖੰਡ->children = ਸਾਰਣੀ;
			ਨਹੀਂਤਾਂ
				ਬੱਚਾ->prev->ਅਗਲਾ = ਸਾਰਣੀ;

			ਸਾਰਣੀ->prev = ਬੱਚਾ->prev;

			ਜਦੋਂ (ਬੱਚਾ != ਨਿਰਾ && (
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ ||
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ_ਪੰਕਤੀ ||
					ਬੱਚਾ->type == BOX_ਸਾਰਣੀ_CELL)) {
				box_ਜੋੜੋ_ਬੱਚਾ(ਸਾਰਣੀ, ਬੱਚਾ);

				ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;
				ਬੱਚਾ->ਅਗਲਾ = ਨਿਰਾ;
				ਬੱਚਾ = ਅਗਲਾ_ਬੱਚਾ;
			}

			ਸਾਰਣੀ->ਆਖਰੀ->ਅਗਲਾ = ਨਿਰਾ;
			ਸਾਰਣੀ->ਅਗਲਾ = ਅਗਲਾ_ਬੱਚਾ = ਬੱਚਾ;
			ਜੇ (ਸਾਰਣੀ->ਅਗਲਾ != ਨਿਰਾ)
				ਸਾਰਣੀ->ਅਗਲਾ->prev = ਸਾਰਣੀ;
			ਨਹੀਂਤਾਂ
				ਖੰਡ->ਆਖਰੀ = ਸਾਰਣੀ;
			ਸਾਰਣੀ->ਮਾਪਾ = ਖੰਡ;

			ਜੇ (box_normalise_ਸਾਰਣੀ(ਸਾਰਣੀ, ਜੜ੍ਹ, c) == false)
				ਵਾਪਸ false;
			ਰੋਕੋ;
		ਮੂਲ:
			ਪ੍ਰਤਿਜ੍ਞਾ(0);
		}
	}

	ਵਾਪਸ true;
}
