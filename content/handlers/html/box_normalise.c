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
#undef BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
ಸಂರಚನಾ span_info {
	/** Number of rows this cell spans */
	unsigned ಪೂರ್ಣಾಙ್ಕ row_span;
	/** Row group of cell */
	ಸಂರಚನಾ box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_row;
};

/**
 * Column record for a table
 */
ಸಂರಚನಾ columns {
	/** Current column index */
	unsigned ಪೂರ್ಣಾಙ್ಕ current_column;
	/** Number of columns in main part of table 1..max columns */
	unsigned ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_columns;
	/** Information about columns in main table, array [0, num_columns) */
	ಸಂರಚನಾ span_info *spans;
	/** Number of rows in table */
	unsigned ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_rows;
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
calculate_table_row(ಸಂರಚನಾ columns *col_info,
		    unsigned ಪೂರ್ಣಾಙ್ಕ col_span,
		    unsigned ಪೂರ್ಣಾಙ್ಕ row_span,
		    unsigned ಪೂರ್ಣಾಙ್ಕ *start_column,
		    ಸಂರಚನಾ box *cell)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ cell_start_col = col_info->current_column;
	unsigned ಪೂರ್ಣಾಙ್ಕ cell_end_col;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ span_info *spans;
	ಸಂರಚನಾ box *rg = cell->parent->parent; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	ಯಾವತ್ (col_info->spans[cell_start_col].row_span != 0 &&
			col_info->spans[cell_start_col].rg == rg) {
		cell_start_col++;
	}

	/* Update current column with calculated start */
	col_info->current_column = cell_start_col;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	ಯದಿ (col_span == 0)
		col_span = 1;

	cell_end_col = cell_start_col + col_span;

	ಯದಿ (col_info->ಅಂಕೆ_columns < cell_end_col) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(col_info->spans,
				ಪರಿಮಾಣ *spans * (cell_end_col + 1));
		ಯದಿ (spans == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ false;

		col_info->spans = spans;
		col_info->ಅಂಕೆ_columns = cell_end_col;

		/* Mark new final column as sentinel */
		col_info->spans[cell_end_col].row_span = 0;
		col_info->spans[cell_end_col].auto_row = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	ಪ್ರತಿ (i = cell_start_col; i < cell_end_col; i++) {
		col_info->spans[i].row_span = (row_span == 0) ? 1 : row_span;
		col_info->spans[i].auto_row = (row_span == 0);
		col_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	col_info->current_column = cell_end_col;

	*start_column = cell_start_col;

	ಪ್ರತ್ಯಾಗಮ true;
}


ಸ್ಥಿರ bool
box_normalise_table_row(ಸಂರಚನಾ box *row,
			ಧ್ರುವ ಸಂರಚನಾ box *ಬೇರು,
			ಸಂರಚನಾ columns *col_info,
			html_content * c)
{
	ಸಂರಚನಾ box *child;
	ಸಂರಚನಾ box *next_child;
	ಸಂರಚನಾ box *cell = ರಿಕ್ತ;
	css_computed_style *style;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	nscss_select_ctx ctx;

	ಪ್ರತಿಜ್ಞಾ(row != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(row->type == BOX_TABLE_ROW);

	ctx.ಬೇರು_style = ಬೇರು->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", row);
#endif

	ಪ್ರತಿ (child = row->children; child != ರಿಕ್ತ; child = next_child) {
		next_child = child->next;

		ವಿಕಲ್ಪ (child->type) {
		ಪಕ್ಷ BOX_TABLE_CELL:
			/* ok */
			ಯದಿ (box_normalise_block(child, ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			cell = child;
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_FLEX:
		ಪಕ್ಷ BOX_BLOCK:
		ಪಕ್ಷ BOX_INLINE_CONTAINER:
		ಪಕ್ಷ BOX_TABLE:
		ಪಕ್ಷ BOX_TABLE_ROW_GROUP:
		ಪಕ್ಷ BOX_TABLE_ROW:
			/* insert implied table cell */
			ಪ್ರತಿಜ್ಞಾ(row->style != ರಿಕ್ತ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row->style);
			ಯದಿ (style == ರಿಕ್ತ)
				ಪ್ರತ್ಯಾಗಮ false;

			cell = box_create(ರಿಕ್ತ, style, true, row->href,
					row->target, ರಿಕ್ತ, ರಿಕ್ತ, c->bctx);
			ಯದಿ (cell == ರಿಕ್ತ) {
				css_computed_style_destroy(style);
				ಪ್ರತ್ಯಾಗಮ false;
			}
			cell->type = BOX_TABLE_CELL;

			ಯದಿ (child->prev == ರಿಕ್ತ)
				row->children = cell;
			ಅನ್ಯಥಾ
				child->prev->next = cell;

			cell->prev = child->prev;

			ಯಾವತ್ (child != ರಿಕ್ತ && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW)) {
				box_add_child(cell, child);

				next_child = child->next;
				child->next = ರಿಕ್ತ;
				child = next_child;
			}

			ಪ್ರತಿಜ್ಞಾ(cell->last != ರಿಕ್ತ);

			cell->last->next = ರಿಕ್ತ;
			cell->next = next_child = child;
			ಯದಿ (cell->next != ರಿಕ್ತ)
				cell->next->prev = cell;
			ಅನ್ಯಥಾ
				row->last = cell;
			cell->parent = row;

			ಯದಿ (box_normalise_block(cell, ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_INLINE:
		ಪಕ್ಷ BOX_INLINE_END:
		ಪಕ್ಷ BOX_INLINE_FLEX:
		ಪಕ್ಷ BOX_INLINE_BLOCK:
		ಪಕ್ಷ BOX_FLOAT_ಎಡ:
		ಪಕ್ಷ BOX_FLOAT_ಬಲ:
		ಪಕ್ಷ BOX_BR:
		ಪಕ್ಷ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ಪ್ರತಿಜ್ಞಾ(0);
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಪ್ರತಿಜ್ಞಾ(0);
		}

		ಯದಿ (calculate_table_row(col_info, cell->columns, cell->rows,
				&cell->start_column, cell) == false)
			ಪ್ರತ್ಯಾಗಮ false;
	}


	/* Update row spanning details for all columns */
	ಪ್ರತಿ (i = 0; i < col_info->ಅಂಕೆ_columns; i++) {
		ಯದಿ (col_info->spans[i].row_span != 0 &&
				col_info->spans[i].auto_row == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			col_info->spans[i].row_span--;
		}
	}

	/* Reset current column for next row */
	col_info->current_column = 0;

	/* Increment row counter */
	col_info->ಅಂಕೆ_rows++;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", row);
#endif

	ಪ್ರತ್ಯಾಗಮ true;
}


ಸ್ಥಿರ bool
box_normalise_table_row_group(ಸಂರಚನಾ box *row_group,
			      ಧ್ರುವ ಸಂರಚನಾ box *ಬೇರು,
			      ಸಂರಚನಾ columns *col_info,
			      html_content * c)
{
	ಸಂರಚನಾ box *child;
	ಸಂರಚನಾ box *next_child;
	ಸಂರಚನಾ box *row;
	css_computed_style *style;
	nscss_select_ctx ctx;
	unsigned ಪೂರ್ಣಾಙ್ಕ group_row_ಎಣಿಕೆ = 0;

	ಪ್ರತಿಜ್ಞಾ(row_group != 0);
	ಪ್ರತಿಜ್ಞಾ(row_group->type == BOX_TABLE_ROW_GROUP);

	ctx.ಬೇರು_style = ಬೇರು->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", row_group);
#endif

	ಪ್ರತಿ (child = row_group->children; child != ರಿಕ್ತ; child = next_child) {
		next_child = child->next;

		ವಿಕಲ್ಪ (child->type) {
		ಪಕ್ಷ BOX_TABLE_ROW:
			/* ok */
			group_row_ಎಣಿಕೆ++;
			ಯದಿ (box_normalise_table_row(child, ಬೇರು, col_info,
					c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_FLEX:
		ಪಕ್ಷ BOX_BLOCK:
		ಪಕ್ಷ BOX_INLINE_CONTAINER:
		ಪಕ್ಷ BOX_TABLE:
		ಪಕ್ಷ BOX_TABLE_ROW_GROUP:
		ಪಕ್ಷ BOX_TABLE_CELL:
			/* insert implied table row */
			ಪ್ರತಿಜ್ಞಾ(row_group->style != ರಿಕ್ತ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row_group->style);
			ಯದಿ (style == ರಿಕ್ತ)
				ಪ್ರತ್ಯಾಗಮ false;

			row = box_create(ರಿಕ್ತ, style, true, row_group->href,
					row_group->target, ರಿಕ್ತ, ರಿಕ್ತ, c->bctx);
			ಯದಿ (row == ರಿಕ್ತ) {
				css_computed_style_destroy(style);
				ಪ್ರತ್ಯಾಗಮ false;
			}
			row->type = BOX_TABLE_ROW;

			ಯದಿ (child->prev == ರಿಕ್ತ)
				row_group->children = row;
			ಅನ್ಯಥಾ
				child->prev->next = row;

			row->prev = child->prev;

			ಯಾವತ್ (child != ರಿಕ್ತ && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row, child);

				next_child = child->next;
				child->next = ರಿಕ್ತ;
				child = next_child;
			}

			ಪ್ರತಿಜ್ಞಾ(row->last != ರಿಕ್ತ);

			row->last->next = ರಿಕ್ತ;
			row->next = next_child = child;
			ಯದಿ (row->next != ರಿಕ್ತ)
				row->next->prev = row;
			ಅನ್ಯಥಾ
				row_group->last = row;
			row->parent = row_group;

			group_row_ಎಣಿಕೆ++;
			ಯದಿ (box_normalise_table_row(row, ಬೇರು, col_info,
					c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_INLINE:
		ಪಕ್ಷ BOX_INLINE_END:
		ಪಕ್ಷ BOX_INLINE_FLEX:
		ಪಕ್ಷ BOX_INLINE_BLOCK:
		ಪಕ್ಷ BOX_FLOAT_ಎಡ:
		ಪಕ್ಷ BOX_FLOAT_ಬಲ:
		ಪಕ್ಷ BOX_BR:
		ಪಕ್ಷ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ಪ್ರತಿಜ್ಞಾ(0);
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಪ್ರತಿಜ್ಞಾ(0);
		}
	}

	ಯದಿ (row_group->children == ರಿಕ್ತ) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#endif

		ಪ್ರತಿಜ್ಞಾ(row_group->style != ರಿಕ್ತ);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		ಯದಿ (style == ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ false;
		}

		row = box_create(ರಿಕ್ತ, style, true, row_group->href,
				row_group->target, ರಿಕ್ತ, ರಿಕ್ತ, c->bctx);
		ಯದಿ (row == ರಿಕ್ತ) {
			css_computed_style_destroy(style);
			ಪ್ರತ್ಯಾಗಮ false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		group_row_ಎಣಿಕೆ = 1;

		/* Keep table's row count in sync */
		col_info->ಅಂಕೆ_rows++;
	}

	row_group->rows = group_row_ಎಣಿಕೆ;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", row_group);
#endif

	ಪ್ರತ್ಯಾಗಮ true;
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
box_normalise_table_spans(ಸಂರಚನಾ box *table,
			  ಧ್ರುವ ಸಂರಚನಾ box *ಬೇರು,
			  ಸಂರಚನಾ span_info *spans,
			  html_content *c)
{
	ಸಂರಚನಾ box *table_row_group;
	ಸಂರಚನಾ box *table_row;
	ಸಂರಚನಾ box *table_cell;
	unsigned ಪೂರ್ಣಾಙ್ಕ rows_ಎಡ = table->rows;
	unsigned ಪೂರ್ಣಾಙ್ಕ group_rows_ಎಡ;
	unsigned ಪೂರ್ಣಾಙ್ಕ col;
	nscss_select_ctx ctx;

	ctx.ಬೇರು_style = ಬೇರು->style;

	/* Clear span data */
	memset(spans, 0, table->columns * ಪರಿಮಾಣ(ಸಂರಚನಾ span_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	ಪ್ರತಿ (table_row_group = table->children;
	     table_row_group != ರಿಕ್ತ;
	     table_row_group = table_row_group->next) {

		group_rows_ಎಡ = table_row_group->rows;

		ಪ್ರತಿ (table_row = table_row_group->children;
		     table_row != ರಿಕ್ತ;
		     table_row = table_row->next) {

			ಪ್ರತಿ (table_cell = table_row->children;
			     table_cell != ರಿಕ್ತ;
			     table_cell = table_cell->next) {

				/* colspan = 0 -> colspan = 1 */
				ಯದಿ (table_cell->columns == 0) {
					table_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				ಯದಿ (table_cell->rows == 0) {
					table_cell->rows = group_rows_ಎಡ;
				}

				/* limit rowspans within group */
				ಯದಿ (table_cell->rows > group_rows_ಎಡ) {
					table_cell->rows = group_rows_ಎಡ;
				}

				/* Record span information */
				ಪ್ರತಿ (col = table_cell->start_column;
						col < table_cell->start_column +
						table_cell->columns; col++) {
					spans[col].row_span = table_cell->rows;
				}
			}

			/* Reduce span count of each column */
			ಪ್ರತಿ (col = 0; col < table->columns; col++) {
				ಯದಿ (spans[col].row_span == 0) {
					unsigned ಪೂರ್ಣಾಙ್ಕ start = col;
					css_computed_style *style;
					ಸಂರಚನಾ box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					ಪ್ರತಿಜ್ಞಾ(table_row->style != ರಿಕ್ತ);

					/* Find width of gap */
					ಯಾವತ್ (col < table->columns &&
							spans[col].row_span ==
							0) {
						col++;
					}

					ctx.ctx = c->select_ctx;
					ctx.quirks = (c->quirks ==
						DOM_DOCUMENT_QUIRKS_MODE_FULL);
					ctx.base_url = c->base_url;
					ctx.universal = c->universal;

					style = nscss_get_blank_style(&ctx,
							&c->unit_len_ctx,
							table_row->style);
					ಯದಿ (style == ರಿಕ್ತ)
						ಪ್ರತ್ಯಾಗಮ false;

					cell = box_create(ರಿಕ್ತ, style, true,
							table_row->href,
							table_row->target,
							ರಿಕ್ತ, ರಿಕ್ತ, c->bctx);
					ಯದಿ (cell == ರಿಕ್ತ) {
						css_computed_style_destroy(
								style);
						ಪ್ರತ್ಯಾಗಮ false;
					}
					cell->type = BOX_TABLE_CELL;

					cell->rows = 1;
					cell->columns = col - start;
					cell->start_column = start;

					/* Find place to insert cell */
					ಪ್ರತಿ (prev = table_row->children;
							prev != ರಿಕ್ತ;
							prev = prev->next) {
						ಯದಿ (prev->start_column +
							prev->columns ==
								start)
							ಭಙ್ಗ;
						ಯದಿ (prev->next == ರಿಕ್ತ)
							ಭಙ್ಗ;
					}

					/* Insert it */
					ಯದಿ (prev == ರಿಕ್ತ) {
						ಯದಿ (table_row->children != ರಿಕ್ತ)
							table_row->children->
								prev = cell;
						ಅನ್ಯಥಾ
							table_row->last = cell;

						cell->next =
							table_row->children;
						table_row->children = cell;
					} ಅನ್ಯಥಾ {
						ಯದಿ (prev->next != ರಿಕ್ತ)
							prev->next->prev = cell;
						ಅನ್ಯಥಾ
							table_row->last = cell;

						cell->next = prev->next;
						prev->next = cell;
						cell->prev = prev;
					}
					cell->parent = table_row;
				} ಅನ್ಯಥಾ {
					spans[col].row_span--;
				}
			}

			ಪ್ರತಿಜ್ಞಾ(rows_ಎಡ > 0);

			rows_ಎಡ--;
		}

		group_rows_ಎಡ--;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


ಸ್ಥಿರ bool
box_normalise_table(ಸಂರಚನಾ box *table, ಧ್ರುವ ಸಂರಚನಾ box *ಬೇರು, html_content * c)
{
	ಸಂರಚನಾ box *child;
	ಸಂರಚನಾ box *next_child;
	ಸಂರಚನಾ box *row_group;
	css_computed_style *style;
	ಸಂರಚನಾ columns col_info;
	nscss_select_ctx ctx;

	ಪ್ರತಿಜ್ಞಾ(table != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(table->type == BOX_TABLE);

	ctx.ಬೇರು_style = ಬೇರು->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", table);
#endif

	col_info.ಅಂಕೆ_columns = 1;
	col_info.current_column = 0;
	col_info.spans = malloc(2 * ಪರಿಮಾಣ *col_info.spans);
	ಯದಿ (col_info.spans == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ false;

	col_info.spans[0].row_span = col_info.spans[1].row_span = 0;
	col_info.spans[0].auto_row = false;
	col_info.spans[1].auto_row = false;
	col_info.ಅಂಕೆ_rows = 0;

	ಪ್ರತಿ (child = table->children; child != ರಿಕ್ತ; child = next_child) {
		next_child = child->next;
		ವಿಕಲ್ಪ (child->type) {
		ಪಕ್ಷ BOX_TABLE_ROW_GROUP:
			/* ok */
			ಯದಿ (box_normalise_table_row_group(child, ಬೇರು,
					&col_info, c) == false) {
				ಮುಕ್ತ(col_info.spans);
				ಪ್ರತ್ಯಾಗಮ false;
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_FLEX:
		ಪಕ್ಷ BOX_BLOCK:
		ಪಕ್ಷ BOX_INLINE_CONTAINER:
		ಪಕ್ಷ BOX_TABLE:
		ಪಕ್ಷ BOX_TABLE_ROW:
		ಪಕ್ಷ BOX_TABLE_CELL:
			/* insert implied table row group */
			ಪ್ರತಿಜ್ಞಾ(table->style != ರಿಕ್ತ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					table->style);
			ಯದಿ (style == ರಿಕ್ತ) {
				ಮುಕ್ತ(col_info.spans);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			row_group = box_create(ರಿಕ್ತ, style, true, table->href,
					table->target, ರಿಕ್ತ, ರಿಕ್ತ, c->bctx);
			ಯದಿ (row_group == ರಿಕ್ತ) {
				css_computed_style_destroy(style);
				ಮುಕ್ತ(col_info.spans);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			row_group->type = BOX_TABLE_ROW_GROUP;

			ಯದಿ (child->prev == ರಿಕ್ತ)
				table->children = row_group;
			ಅನ್ಯಥಾ
				child->prev->next = row_group;

			row_group->prev = child->prev;

			ಯಾವತ್ (child != ರಿಕ್ತ && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row_group, child);

				next_child = child->next;
				child->next = ರಿಕ್ತ;
				child = next_child;
			}

			ಪ್ರತಿಜ್ಞಾ(row_group->last != ರಿಕ್ತ);

			row_group->last->next = ರಿಕ್ತ;
			row_group->next = next_child = child;
			ಯದಿ (row_group->next != ರಿಕ್ತ)
				row_group->next->prev = row_group;
			ಅನ್ಯಥಾ
				table->last = row_group;
			row_group->parent = table;

			ಯದಿ (box_normalise_table_row_group(row_group, ಬೇರು,
					&col_info, c) == false) {
				ಮುಕ್ತ(col_info.spans);
				ಪ್ರತ್ಯಾಗಮ false;
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_INLINE:
		ಪಕ್ಷ BOX_INLINE_END:
		ಪಕ್ಷ BOX_INLINE_FLEX:
		ಪಕ್ಷ BOX_INLINE_BLOCK:
		ಪಕ್ಷ BOX_FLOAT_ಎಡ:
		ಪಕ್ಷ BOX_FLOAT_ಬಲ:
		ಪಕ್ಷ BOX_BR:
		ಪಕ್ಷ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ಪ್ರತಿಜ್ಞಾ(0);
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			fprintf(stderr, "%i\n", child->type);
			ಪ್ರತಿಜ್ಞಾ(0);
		}
	}

	table->columns = col_info.ಅಂಕೆ_columns;
	table->rows = col_info.ಅಂಕೆ_rows;

	ಯದಿ (table->children == ರಿಕ್ತ) {
		ಸಂರಚನಾ box *row;

#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#endif

		ಪ್ರತಿಜ್ಞಾ(table->style != ರಿಕ್ತ);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				table->style);
		ಯದಿ (style == ರಿಕ್ತ) {
			ಮುಕ್ತ(col_info.spans);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		row_group = box_create(ರಿಕ್ತ, style, true, table->href,
				table->target, ರಿಕ್ತ, ರಿಕ್ತ, c->bctx);
		ಯದಿ (row_group == ರಿಕ್ತ) {
			css_computed_style_destroy(style);
			ಮುಕ್ತ(col_info.spans);
			ಪ್ರತ್ಯಾಗಮ false;
		}
		row_group->type = BOX_TABLE_ROW_GROUP;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		ಯದಿ (style == ರಿಕ್ತ) {
			box_free(row_group);
			ಮುಕ್ತ(col_info.spans);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		row = box_create(ರಿಕ್ತ, style, true, row_group->href,
				row_group->target, ರಿಕ್ತ, ರಿಕ್ತ, c->bctx);
		ಯದಿ (row == ರಿಕ್ತ) {
			css_computed_style_destroy(style);
			box_free(row_group);
			ಮುಕ್ತ(col_info.spans);
			ಪ್ರತ್ಯಾಗಮ false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		row_group->parent = table;
		table->children = table->last = row_group;

		table->rows = 1;
	}

	ಯದಿ (box_normalise_table_spans(table, ಬೇರು, col_info.spans, c) == false) {
		ಮುಕ್ತ(col_info.spans);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಮುಕ್ತ(col_info.spans);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", table);
#endif

	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ bool box_normalise_flex(
		ಸಂರಚನಾ box *flex_container,
		ಧ್ರುವ ಸಂರಚನಾ box *ಬೇರು,
		html_content *c)
{
	ಸಂರಚನಾ box *child;
	ಸಂರಚನಾ box *next_child;
	ಸಂರಚನಾ box *implied_flex_item;
	css_computed_style *style;
	nscss_select_ctx ctx;

	ಪ್ರತಿಜ್ಞಾ(flex_container != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(ಬೇರು != ರಿಕ್ತ);

	ctx.ಬೇರು_style = ಬೇರು->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#endif

	ಪ್ರತಿಜ್ಞಾ(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	ಪ್ರತಿ (child = flex_container->children; child != ರಿಕ್ತ; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				child, child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		ವಿಕಲ್ಪ (child->type) {
		ಪಕ್ಷ BOX_FLEX:
			/* ok */
			ಯದಿ (box_normalise_flex(child, ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_BLOCK:
			/* ok */
			ಯದಿ (box_normalise_block(child, ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			ಪ್ರತಿಜ್ಞಾ(flex_container->style != ರಿಕ್ತ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			ಯದಿ (style == ರಿಕ್ತ)
				ಪ್ರತ್ಯಾಗಮ false;

			implied_flex_item = box_create(ರಿಕ್ತ, style, true,
					flex_container->href,
					flex_container->target,
					ರಿಕ್ತ, ರಿಕ್ತ, c->bctx);
			ಯದಿ (implied_flex_item == ರಿಕ್ತ) {
				css_computed_style_destroy(style);
				ಪ್ರತ್ಯಾಗಮ false;
			}
			implied_flex_item->type = BOX_BLOCK;

			ಯದಿ (child->prev == ರಿಕ್ತ)
				flex_container->children = implied_flex_item;
			ಅನ್ಯಥಾ
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			ಯಾವತ್ (child != ರಿಕ್ತ &&
					child->type == BOX_INLINE_CONTAINER) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = ರಿಕ್ತ;
				child = next_child;
			}

			implied_flex_item->last->next = ರಿಕ್ತ;
			implied_flex_item->next = next_child = child;
			ಯದಿ (implied_flex_item->next != ರಿಕ್ತ)
				implied_flex_item->next->prev = implied_flex_item;
			ಅನ್ಯಥಾ
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			ಯದಿ (box_normalise_block(implied_flex_item,
					ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;

		ಪಕ್ಷ BOX_TABLE:
			ಯದಿ (box_normalise_table(child, ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_INLINE:
		ಪಕ್ಷ BOX_INLINE_END:
		ಪಕ್ಷ BOX_INLINE_FLEX:
		ಪಕ್ಷ BOX_INLINE_BLOCK:
		ಪಕ್ಷ BOX_FLOAT_ಎಡ:
		ಪಕ್ಷ BOX_FLOAT_ಬಲ:
		ಪಕ್ಷ BOX_BR:
		ಪಕ್ಷ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ಪ್ರತಿಜ್ಞಾ(0);
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_TABLE_ROW_GROUP:
		ಪಕ್ಷ BOX_TABLE_ROW:
		ಪಕ್ಷ BOX_TABLE_CELL:
			/* insert implied table */
			ಪ್ರತಿಜ್ಞಾ(flex_container->style != ರಿಕ್ತ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			ಯದಿ (style == ರಿಕ್ತ)
				ಪ್ರತ್ಯಾಗಮ false;

			implied_flex_item = box_create(ರಿಕ್ತ, style, true,
					flex_container->href,
					flex_container->target,
					ರಿಕ್ತ, ರಿಕ್ತ, c->bctx);
			ಯದಿ (implied_flex_item == ರಿಕ್ತ) {
				css_computed_style_destroy(style);
				ಪ್ರತ್ಯಾಗಮ false;
			}
			implied_flex_item->type = BOX_TABLE;

			ಯದಿ (child->prev == ರಿಕ್ತ)
				flex_container->children = implied_flex_item;
			ಅನ್ಯಥಾ
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			ಯಾವತ್ (child != ರಿಕ್ತ && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = ರಿಕ್ತ;
				child = next_child;
			}

			implied_flex_item->last->next = ರಿಕ್ತ;
			implied_flex_item->next = next_child = child;
			ಯದಿ (implied_flex_item->next != ರಿಕ್ತ)
				implied_flex_item->next->prev = implied_flex_item;
			ಅನ್ಯಥಾ
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			ಯದಿ (box_normalise_table(implied_flex_item,
					ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಪ್ರತಿಜ್ಞಾ(0);
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ bool
box_normalise_inline_container(ಸಂರಚನಾ box *cont,
			       ಧ್ರುವ ಸಂರಚನಾ box *ಬೇರು,
			       html_content * c)
{
	ಸಂರಚನಾ box *child;
	ಸಂರಚನಾ box *next_child;

	ಪ್ರತಿಜ್ಞಾ(cont != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(cont->type == BOX_INLINE_CONTAINER);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#endif

	ಪ್ರತಿ (child = cont->children; child != ರಿಕ್ತ; child = next_child) {
		next_child = child->next;
		ವಿಕಲ್ಪ (child->type) {
		ಪಕ್ಷ BOX_INLINE:
		ಪಕ್ಷ BOX_INLINE_END:
		ಪಕ್ಷ BOX_BR:
		ಪಕ್ಷ BOX_TEXT:
			/* ok */
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_INLINE_BLOCK:
			/* ok */
			ಯದಿ (box_normalise_block(child, ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_INLINE_FLEX:
			/* ok */
			ಯದಿ (box_normalise_flex(child, ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_FLOAT_ಎಡ:
		ಪಕ್ಷ BOX_FLOAT_ಬಲ:
			/* ok */
			ಪ್ರತಿಜ್ಞಾ(child->children != ರಿಕ್ತ);

			ವಿಕಲ್ಪ (child->children->type) {
			ಪಕ್ಷ BOX_BLOCK:
				ಯದಿ (box_normalise_block(child->children, ಬೇರು,
						c) == false)
					ಪ್ರತ್ಯಾಗಮ false;
				ಭಙ್ಗ;
			ಪಕ್ಷ BOX_TABLE:
				ಯದಿ (box_normalise_table(child->children, ಬೇರು,
						c) == false)
					ಪ್ರತ್ಯಾಗಮ false;
				ಭಙ್ಗ;
			ಪಕ್ಷ BOX_FLEX:
				ಯದಿ (box_normalise_flex(child->children, ಬೇರು,
						c) == false)
					ಪ್ರತ್ಯಾಗಮ false;
				ಭಙ್ಗ;
			ಸ್ವಭಾವ:
				ಪ್ರತಿಜ್ಞಾ(0);
			}

			ಯದಿ (child->children == ರಿಕ್ತ) {
				/* the child has destroyed itself: remove float */
				ಯದಿ (child->prev == ರಿಕ್ತ)
					child->parent->children = child->next;
				ಅನ್ಯಥಾ
					child->prev->next = child->next;
				ಯದಿ (child->next != ರಿಕ್ತ)
					child->next->prev = child->prev;
				ಅನ್ಯಥಾ
					child->parent->last = child->prev;

				box_free(child);
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_FLEX:
		ಪಕ್ಷ BOX_BLOCK:
		ಪಕ್ಷ BOX_INLINE_CONTAINER:
		ಪಕ್ಷ BOX_TABLE:
		ಪಕ್ಷ BOX_TABLE_ROW_GROUP:
		ಪಕ್ಷ BOX_TABLE_ROW:
		ಪಕ್ಷ BOX_TABLE_CELL:
		ಸ್ವಭಾವ:
			ಪ್ರತಿಜ್ಞಾ(0);
		}
	}

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#endif

	ಪ್ರತ್ಯಾಗಮ true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_block(ಸಂರಚನಾ box *block, ಧ್ರುವ ಸಂರಚನಾ box *ಬೇರು, html_content *c)
{
	ಸಂರಚನಾ box *child;
	ಸಂರಚನಾ box *next_child;
	ಸಂರಚನಾ box *table;
	css_computed_style *style;
	nscss_select_ctx ctx;

	ಪ್ರತಿಜ್ಞಾ(block != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(ಬೇರು != ರಿಕ್ತ);

	ctx.ಬೇರು_style = ಬೇರು->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", block, block->type);
#endif

	ಪ್ರತಿಜ್ಞಾ(block->type == BOX_BLOCK || block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	ಪ್ರತಿ (child = block->children; child != ರಿಕ್ತ; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", child,
		      child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		ವಿಕಲ್ಪ (child->type) {
		ಪಕ್ಷ BOX_FLEX:
			/* ok */
			ಯದಿ (box_normalise_flex(child, ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_BLOCK:
			/* ok */
			ಯದಿ (box_normalise_block(child, ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_INLINE_CONTAINER:
			ಯದಿ (box_normalise_inline_container(child, ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_TABLE:
			ಯದಿ (box_normalise_table(child, ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_INLINE:
		ಪಕ್ಷ BOX_INLINE_END:
		ಪಕ್ಷ BOX_INLINE_FLEX:
		ಪಕ್ಷ BOX_INLINE_BLOCK:
		ಪಕ್ಷ BOX_FLOAT_ಎಡ:
		ಪಕ್ಷ BOX_FLOAT_ಬಲ:
		ಪಕ್ಷ BOX_BR:
		ಪಕ್ಷ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ಪ್ರತಿಜ್ಞಾ(0);
			ಭಙ್ಗ;
		ಪಕ್ಷ BOX_TABLE_ROW_GROUP:
		ಪಕ್ಷ BOX_TABLE_ROW:
		ಪಕ್ಷ BOX_TABLE_CELL:
			/* insert implied table */
			ಪ್ರತಿಜ್ಞಾ(block->style != ರಿಕ್ತ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					block->style);
			ಯದಿ (style == ರಿಕ್ತ)
				ಪ್ರತ್ಯಾಗಮ false;

			table = box_create(ರಿಕ್ತ, style, true, block->href,
					block->target, ರಿಕ್ತ, ರಿಕ್ತ, c->bctx);
			ಯದಿ (table == ರಿಕ್ತ) {
				css_computed_style_destroy(style);
				ಪ್ರತ್ಯಾಗಮ false;
			}
			table->type = BOX_TABLE;

			ಯದಿ (child->prev == ರಿಕ್ತ)
				block->children = table;
			ಅನ್ಯಥಾ
				child->prev->next = table;

			table->prev = child->prev;

			ಯಾವತ್ (child != ರಿಕ್ತ && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(table, child);

				next_child = child->next;
				child->next = ರಿಕ್ತ;
				child = next_child;
			}

			table->last->next = ರಿಕ್ತ;
			table->next = next_child = child;
			ಯದಿ (table->next != ರಿಕ್ತ)
				table->next->prev = table;
			ಅನ್ಯಥಾ
				block->last = table;
			table->parent = block;

			ಯದಿ (box_normalise_table(table, ಬೇರು, c) == false)
				ಪ್ರತ್ಯಾಗಮ false;
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಪ್ರತಿಜ್ಞಾ(0);
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}
