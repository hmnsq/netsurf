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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "css/select.h"

#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/table.h"
#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/box_manipulate.h"
#অন্তর্ভাব "html/box_normalise.h"

/* Define to enable box normalise debug */
#undef BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
কাঠামো span_info {
	/** Number of rows this cell spans */
	unsigned পূর্ণসংখ্যা row_span;
	/** Row group of cell */
	কাঠামো box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_row;
};

/**
 * Column record for a table
 */
কাঠামো columns {
	/** Current column index */
	unsigned পূর্ণসংখ্যা current_column;
	/** Number of columns in main part of table 1..max columns */
	unsigned পূর্ণসংখ্যা অঙ্ক_columns;
	/** Information about columns in main table, array [0, num_columns) */
	কাঠামো span_info *spans;
	/** Number of rows in table */
	unsigned পূর্ণসংখ্যা অঙ্ক_rows;
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
স্থির bool
calculate_table_row(কাঠামো columns *col_info,
		    unsigned পূর্ণসংখ্যা col_span,
		    unsigned পূর্ণসংখ্যা row_span,
		    unsigned পূর্ণসংখ্যা *start_column,
		    কাঠামো box *cell)
{
	unsigned পূর্ণসংখ্যা cell_start_col = col_info->current_column;
	unsigned পূর্ণসংখ্যা cell_end_col;
	unsigned পূর্ণসংখ্যা i;
	কাঠামো span_info *spans;
	কাঠামো box *rg = cell->parent->parent; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	যখন (col_info->spans[cell_start_col].row_span != 0 &&
			col_info->spans[cell_start_col].rg == rg) {
		cell_start_col++;
	}

	/* Update current column with calculated start */
	col_info->current_column = cell_start_col;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	যদি (col_span == 0)
		col_span = 1;

	cell_end_col = cell_start_col + col_span;

	যদি (col_info->অঙ্ক_columns < cell_end_col) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(col_info->spans,
				আকার *spans * (cell_end_col + 1));
		যদি (spans == ফাঁকা)
			ফেরত false;

		col_info->spans = spans;
		col_info->অঙ্ক_columns = cell_end_col;

		/* Mark new final column as sentinel */
		col_info->spans[cell_end_col].row_span = 0;
		col_info->spans[cell_end_col].auto_row = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	জন্য (i = cell_start_col; i < cell_end_col; i++) {
		col_info->spans[i].row_span = (row_span == 0) ? 1 : row_span;
		col_info->spans[i].auto_row = (row_span == 0);
		col_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	col_info->current_column = cell_end_col;

	*start_column = cell_start_col;

	ফেরত true;
}


স্থির bool
box_normalise_table_row(কাঠামো box *row,
			ধ্রুবক কাঠামো box *মূল,
			কাঠামো columns *col_info,
			html_content * c)
{
	কাঠামো box *child;
	কাঠামো box *পরবর্তী_child;
	কাঠামো box *cell = ফাঁকা;
	css_computed_style *style;
	unsigned পূর্ণসংখ্যা i;
	nscss_select_ctx ctx;

	প্রতিজ্ঞা(row != ফাঁকা);
	প্রতিজ্ঞা(row->type == BOX_TABLE_ROW);

	ctx.মূল_style = মূল->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", row);
#endif

	জন্য (child = row->children; child != ফাঁকা; child = পরবর্তী_child) {
		পরবর্তী_child = child->পরবর্তী;

		নির্বাচন (child->type) {
		ক্ষেত্র BOX_TABLE_CELL:
			/* ok */
			যদি (box_normalise_block(child, মূল, c) == false)
				ফেরত false;
			cell = child;
			বিরতি;
		ক্ষেত্র BOX_FLEX:
		ক্ষেত্র BOX_BLOCK:
		ক্ষেত্র BOX_INLINE_CONTAINER:
		ক্ষেত্র BOX_TABLE:
		ক্ষেত্র BOX_TABLE_ROW_GROUP:
		ক্ষেত্র BOX_TABLE_ROW:
			/* insert implied table cell */
			প্রতিজ্ঞা(row->style != ফাঁকা);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row->style);
			যদি (style == ফাঁকা)
				ফেরত false;

			cell = box_create(ফাঁকা, style, true, row->href,
					row->target, ফাঁকা, ফাঁকা, c->bctx);
			যদি (cell == ফাঁকা) {
				css_computed_style_destroy(style);
				ফেরত false;
			}
			cell->type = BOX_TABLE_CELL;

			যদি (child->পূর্ব == ফাঁকা)
				row->children = cell;
			অন্যথা
				child->পূর্ব->পরবর্তী = cell;

			cell->পূর্ব = child->পূর্ব;

			যখন (child != ফাঁকা && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW)) {
				box_add_child(cell, child);

				পরবর্তী_child = child->পরবর্তী;
				child->পরবর্তী = ফাঁকা;
				child = পরবর্তী_child;
			}

			প্রতিজ্ঞা(cell->last != ফাঁকা);

			cell->last->পরবর্তী = ফাঁকা;
			cell->পরবর্তী = পরবর্তী_child = child;
			যদি (cell->পরবর্তী != ফাঁকা)
				cell->পরবর্তী->পূর্ব = cell;
			অন্যথা
				row->last = cell;
			cell->parent = row;

			যদি (box_normalise_block(cell, মূল, c) == false)
				ফেরত false;
			বিরতি;
		ক্ষেত্র BOX_INLINE:
		ক্ষেত্র BOX_INLINE_END:
		ক্ষেত্র BOX_INLINE_FLEX:
		ক্ষেত্র BOX_INLINE_BLOCK:
		ক্ষেত্র BOX_FLOAT_বাম:
		ক্ষেত্র BOX_FLOAT_ডান:
		ক্ষেত্র BOX_BR:
		ক্ষেত্র BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			প্রতিজ্ঞা(0);
			বিরতি;
		পূর্বনির্ধারিত:
			প্রতিজ্ঞা(0);
		}

		যদি (calculate_table_row(col_info, cell->columns, cell->rows,
				&cell->start_column, cell) == false)
			ফেরত false;
	}


	/* Update row spanning details for all columns */
	জন্য (i = 0; i < col_info->অঙ্ক_columns; i++) {
		যদি (col_info->spans[i].row_span != 0 &&
				col_info->spans[i].auto_row == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			col_info->spans[i].row_span--;
		}
	}

	/* Reset current column for next row */
	col_info->current_column = 0;

	/* Increment row counter */
	col_info->অঙ্ক_rows++;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", row);
#endif

	ফেরত true;
}


স্থির bool
box_normalise_table_row_group(কাঠামো box *row_group,
			      ধ্রুবক কাঠামো box *মূল,
			      কাঠামো columns *col_info,
			      html_content * c)
{
	কাঠামো box *child;
	কাঠামো box *পরবর্তী_child;
	কাঠামো box *row;
	css_computed_style *style;
	nscss_select_ctx ctx;
	unsigned পূর্ণসংখ্যা group_row_গুনতি = 0;

	প্রতিজ্ঞা(row_group != 0);
	প্রতিজ্ঞা(row_group->type == BOX_TABLE_ROW_GROUP);

	ctx.মূল_style = মূল->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", row_group);
#endif

	জন্য (child = row_group->children; child != ফাঁকা; child = পরবর্তী_child) {
		পরবর্তী_child = child->পরবর্তী;

		নির্বাচন (child->type) {
		ক্ষেত্র BOX_TABLE_ROW:
			/* ok */
			group_row_গুনতি++;
			যদি (box_normalise_table_row(child, মূল, col_info,
					c) == false)
				ফেরত false;
			বিরতি;
		ক্ষেত্র BOX_FLEX:
		ক্ষেত্র BOX_BLOCK:
		ক্ষেত্র BOX_INLINE_CONTAINER:
		ক্ষেত্র BOX_TABLE:
		ক্ষেত্র BOX_TABLE_ROW_GROUP:
		ক্ষেত্র BOX_TABLE_CELL:
			/* insert implied table row */
			প্রতিজ্ঞা(row_group->style != ফাঁকা);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row_group->style);
			যদি (style == ফাঁকা)
				ফেরত false;

			row = box_create(ফাঁকা, style, true, row_group->href,
					row_group->target, ফাঁকা, ফাঁকা, c->bctx);
			যদি (row == ফাঁকা) {
				css_computed_style_destroy(style);
				ফেরত false;
			}
			row->type = BOX_TABLE_ROW;

			যদি (child->পূর্ব == ফাঁকা)
				row_group->children = row;
			অন্যথা
				child->পূর্ব->পরবর্তী = row;

			row->পূর্ব = child->পূর্ব;

			যখন (child != ফাঁকা && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row, child);

				পরবর্তী_child = child->পরবর্তী;
				child->পরবর্তী = ফাঁকা;
				child = পরবর্তী_child;
			}

			প্রতিজ্ঞা(row->last != ফাঁকা);

			row->last->পরবর্তী = ফাঁকা;
			row->পরবর্তী = পরবর্তী_child = child;
			যদি (row->পরবর্তী != ফাঁকা)
				row->পরবর্তী->পূর্ব = row;
			অন্যথা
				row_group->last = row;
			row->parent = row_group;

			group_row_গুনতি++;
			যদি (box_normalise_table_row(row, মূল, col_info,
					c) == false)
				ফেরত false;
			বিরতি;
		ক্ষেত্র BOX_INLINE:
		ক্ষেত্র BOX_INLINE_END:
		ক্ষেত্র BOX_INLINE_FLEX:
		ক্ষেত্র BOX_INLINE_BLOCK:
		ক্ষেত্র BOX_FLOAT_বাম:
		ক্ষেত্র BOX_FLOAT_ডান:
		ক্ষেত্র BOX_BR:
		ক্ষেত্র BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			প্রতিজ্ঞা(0);
			বিরতি;
		পূর্বনির্ধারিত:
			প্রতিজ্ঞা(0);
		}
	}

	যদি (row_group->children == ফাঁকা) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#endif

		প্রতিজ্ঞা(row_group->style != ফাঁকা);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		যদি (style == ফাঁকা) {
			ফেরত false;
		}

		row = box_create(ফাঁকা, style, true, row_group->href,
				row_group->target, ফাঁকা, ফাঁকা, c->bctx);
		যদি (row == ফাঁকা) {
			css_computed_style_destroy(style);
			ফেরত false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		group_row_গুনতি = 1;

		/* Keep table's row count in sync */
		col_info->অঙ্ক_rows++;
	}

	row_group->rows = group_row_গুনতি;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", row_group);
#endif

	ফেরত true;
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
স্থির bool
box_normalise_table_spans(কাঠামো box *table,
			  ধ্রুবক কাঠামো box *মূল,
			  কাঠামো span_info *spans,
			  html_content *c)
{
	কাঠামো box *table_row_group;
	কাঠামো box *table_row;
	কাঠামো box *table_cell;
	unsigned পূর্ণসংখ্যা rows_বাম = table->rows;
	unsigned পূর্ণসংখ্যা group_rows_বাম;
	unsigned পূর্ণসংখ্যা col;
	nscss_select_ctx ctx;

	ctx.মূল_style = মূল->style;

	/* Clear span data */
	memset(spans, 0, table->columns * আকার(কাঠামো span_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	জন্য (table_row_group = table->children;
	     table_row_group != ফাঁকা;
	     table_row_group = table_row_group->পরবর্তী) {

		group_rows_বাম = table_row_group->rows;

		জন্য (table_row = table_row_group->children;
		     table_row != ফাঁকা;
		     table_row = table_row->পরবর্তী) {

			জন্য (table_cell = table_row->children;
			     table_cell != ফাঁকা;
			     table_cell = table_cell->পরবর্তী) {

				/* colspan = 0 -> colspan = 1 */
				যদি (table_cell->columns == 0) {
					table_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				যদি (table_cell->rows == 0) {
					table_cell->rows = group_rows_বাম;
				}

				/* limit rowspans within group */
				যদি (table_cell->rows > group_rows_বাম) {
					table_cell->rows = group_rows_বাম;
				}

				/* Record span information */
				জন্য (col = table_cell->start_column;
						col < table_cell->start_column +
						table_cell->columns; col++) {
					spans[col].row_span = table_cell->rows;
				}
			}

			/* Reduce span count of each column */
			জন্য (col = 0; col < table->columns; col++) {
				যদি (spans[col].row_span == 0) {
					unsigned পূর্ণসংখ্যা start = col;
					css_computed_style *style;
					কাঠামো box *cell, *পূর্ব;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					প্রতিজ্ঞা(table_row->style != ফাঁকা);

					/* Find width of gap */
					যখন (col < table->columns &&
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
					যদি (style == ফাঁকা)
						ফেরত false;

					cell = box_create(ফাঁকা, style, true,
							table_row->href,
							table_row->target,
							ফাঁকা, ফাঁকা, c->bctx);
					যদি (cell == ফাঁকা) {
						css_computed_style_destroy(
								style);
						ফেরত false;
					}
					cell->type = BOX_TABLE_CELL;

					cell->rows = 1;
					cell->columns = col - start;
					cell->start_column = start;

					/* Find place to insert cell */
					জন্য (পূর্ব = table_row->children;
							পূর্ব != ফাঁকা;
							পূর্ব = পূর্ব->পরবর্তী) {
						যদি (পূর্ব->start_column +
							পূর্ব->columns ==
								start)
							বিরতি;
						যদি (পূর্ব->পরবর্তী == ফাঁকা)
							বিরতি;
					}

					/* Insert it */
					যদি (পূর্ব == ফাঁকা) {
						যদি (table_row->children != ফাঁকা)
							table_row->children->
								পূর্ব = cell;
						অন্যথা
							table_row->last = cell;

						cell->পরবর্তী =
							table_row->children;
						table_row->children = cell;
					} অন্যথা {
						যদি (পূর্ব->পরবর্তী != ফাঁকা)
							পূর্ব->পরবর্তী->পূর্ব = cell;
						অন্যথা
							table_row->last = cell;

						cell->পরবর্তী = পূর্ব->পরবর্তী;
						পূর্ব->পরবর্তী = cell;
						cell->পূর্ব = পূর্ব;
					}
					cell->parent = table_row;
				} অন্যথা {
					spans[col].row_span--;
				}
			}

			প্রতিজ্ঞা(rows_বাম > 0);

			rows_বাম--;
		}

		group_rows_বাম--;
	}

	ফেরত true;
}


স্থির bool
box_normalise_table(কাঠামো box *table, ধ্রুবক কাঠামো box *মূল, html_content * c)
{
	কাঠামো box *child;
	কাঠামো box *পরবর্তী_child;
	কাঠামো box *row_group;
	css_computed_style *style;
	কাঠামো columns col_info;
	nscss_select_ctx ctx;

	প্রতিজ্ঞা(table != ফাঁকা);
	প্রতিজ্ঞা(table->type == BOX_TABLE);

	ctx.মূল_style = মূল->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", table);
#endif

	col_info.অঙ্ক_columns = 1;
	col_info.current_column = 0;
	col_info.spans = malloc(2 * আকার *col_info.spans);
	যদি (col_info.spans == ফাঁকা)
		ফেরত false;

	col_info.spans[0].row_span = col_info.spans[1].row_span = 0;
	col_info.spans[0].auto_row = false;
	col_info.spans[1].auto_row = false;
	col_info.অঙ্ক_rows = 0;

	জন্য (child = table->children; child != ফাঁকা; child = পরবর্তী_child) {
		পরবর্তী_child = child->পরবর্তী;
		নির্বাচন (child->type) {
		ক্ষেত্র BOX_TABLE_ROW_GROUP:
			/* ok */
			যদি (box_normalise_table_row_group(child, মূল,
					&col_info, c) == false) {
				মুক্ত(col_info.spans);
				ফেরত false;
			}
			বিরতি;
		ক্ষেত্র BOX_FLEX:
		ক্ষেত্র BOX_BLOCK:
		ক্ষেত্র BOX_INLINE_CONTAINER:
		ক্ষেত্র BOX_TABLE:
		ক্ষেত্র BOX_TABLE_ROW:
		ক্ষেত্র BOX_TABLE_CELL:
			/* insert implied table row group */
			প্রতিজ্ঞা(table->style != ফাঁকা);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					table->style);
			যদি (style == ফাঁকা) {
				মুক্ত(col_info.spans);
				ফেরত false;
			}

			row_group = box_create(ফাঁকা, style, true, table->href,
					table->target, ফাঁকা, ফাঁকা, c->bctx);
			যদি (row_group == ফাঁকা) {
				css_computed_style_destroy(style);
				মুক্ত(col_info.spans);
				ফেরত false;
			}

			row_group->type = BOX_TABLE_ROW_GROUP;

			যদি (child->পূর্ব == ফাঁকা)
				table->children = row_group;
			অন্যথা
				child->পূর্ব->পরবর্তী = row_group;

			row_group->পূর্ব = child->পূর্ব;

			যখন (child != ফাঁকা && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row_group, child);

				পরবর্তী_child = child->পরবর্তী;
				child->পরবর্তী = ফাঁকা;
				child = পরবর্তী_child;
			}

			প্রতিজ্ঞা(row_group->last != ফাঁকা);

			row_group->last->পরবর্তী = ফাঁকা;
			row_group->পরবর্তী = পরবর্তী_child = child;
			যদি (row_group->পরবর্তী != ফাঁকা)
				row_group->পরবর্তী->পূর্ব = row_group;
			অন্যথা
				table->last = row_group;
			row_group->parent = table;

			যদি (box_normalise_table_row_group(row_group, মূল,
					&col_info, c) == false) {
				মুক্ত(col_info.spans);
				ফেরত false;
			}
			বিরতি;
		ক্ষেত্র BOX_INLINE:
		ক্ষেত্র BOX_INLINE_END:
		ক্ষেত্র BOX_INLINE_FLEX:
		ক্ষেত্র BOX_INLINE_BLOCK:
		ক্ষেত্র BOX_FLOAT_বাম:
		ক্ষেত্র BOX_FLOAT_ডান:
		ক্ষেত্র BOX_BR:
		ক্ষেত্র BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			প্রতিজ্ঞা(0);
			বিরতি;
		পূর্বনির্ধারিত:
			fprintf(stderr, "%i\n", child->type);
			প্রতিজ্ঞা(0);
		}
	}

	table->columns = col_info.অঙ্ক_columns;
	table->rows = col_info.অঙ্ক_rows;

	যদি (table->children == ফাঁকা) {
		কাঠামো box *row;

#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#endif

		প্রতিজ্ঞা(table->style != ফাঁকা);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				table->style);
		যদি (style == ফাঁকা) {
			মুক্ত(col_info.spans);
			ফেরত false;
		}

		row_group = box_create(ফাঁকা, style, true, table->href,
				table->target, ফাঁকা, ফাঁকা, c->bctx);
		যদি (row_group == ফাঁকা) {
			css_computed_style_destroy(style);
			মুক্ত(col_info.spans);
			ফেরত false;
		}
		row_group->type = BOX_TABLE_ROW_GROUP;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		যদি (style == ফাঁকা) {
			box_free(row_group);
			মুক্ত(col_info.spans);
			ফেরত false;
		}

		row = box_create(ফাঁকা, style, true, row_group->href,
				row_group->target, ফাঁকা, ফাঁকা, c->bctx);
		যদি (row == ফাঁকা) {
			css_computed_style_destroy(style);
			box_free(row_group);
			মুক্ত(col_info.spans);
			ফেরত false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		row_group->parent = table;
		table->children = table->last = row_group;

		table->rows = 1;
	}

	যদি (box_normalise_table_spans(table, মূল, col_info.spans, c) == false) {
		মুক্ত(col_info.spans);
		ফেরত false;
	}

	মুক্ত(col_info.spans);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", table);
#endif

	ফেরত true;
}

স্থির bool box_normalise_flex(
		কাঠামো box *flex_container,
		ধ্রুবক কাঠামো box *মূল,
		html_content *c)
{
	কাঠামো box *child;
	কাঠামো box *পরবর্তী_child;
	কাঠামো box *implied_flex_item;
	css_computed_style *style;
	nscss_select_ctx ctx;

	প্রতিজ্ঞা(flex_container != ফাঁকা);
	প্রতিজ্ঞা(মূল != ফাঁকা);

	ctx.মূল_style = মূল->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#endif

	প্রতিজ্ঞা(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	জন্য (child = flex_container->children; child != ফাঁকা; child = পরবর্তী_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				child, child->type);
#endif

		পরবর্তী_child = child->পরবর্তী;	/* child may be destroyed */

		নির্বাচন (child->type) {
		ক্ষেত্র BOX_FLEX:
			/* ok */
			যদি (box_normalise_flex(child, মূল, c) == false)
				ফেরত false;
			বিরতি;
		ক্ষেত্র BOX_BLOCK:
			/* ok */
			যদি (box_normalise_block(child, মূল, c) == false)
				ফেরত false;
			বিরতি;
		ক্ষেত্র BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			প্রতিজ্ঞা(flex_container->style != ফাঁকা);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			যদি (style == ফাঁকা)
				ফেরত false;

			implied_flex_item = box_create(ফাঁকা, style, true,
					flex_container->href,
					flex_container->target,
					ফাঁকা, ফাঁকা, c->bctx);
			যদি (implied_flex_item == ফাঁকা) {
				css_computed_style_destroy(style);
				ফেরত false;
			}
			implied_flex_item->type = BOX_BLOCK;

			যদি (child->পূর্ব == ফাঁকা)
				flex_container->children = implied_flex_item;
			অন্যথা
				child->পূর্ব->পরবর্তী = implied_flex_item;

			implied_flex_item->পূর্ব = child->পূর্ব;

			যখন (child != ফাঁকা &&
					child->type == BOX_INLINE_CONTAINER) {
				box_add_child(implied_flex_item, child);

				পরবর্তী_child = child->পরবর্তী;
				child->পরবর্তী = ফাঁকা;
				child = পরবর্তী_child;
			}

			implied_flex_item->last->পরবর্তী = ফাঁকা;
			implied_flex_item->পরবর্তী = পরবর্তী_child = child;
			যদি (implied_flex_item->পরবর্তী != ফাঁকা)
				implied_flex_item->পরবর্তী->পূর্ব = implied_flex_item;
			অন্যথা
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			যদি (box_normalise_block(implied_flex_item,
					মূল, c) == false)
				ফেরত false;
			বিরতি;

		ক্ষেত্র BOX_TABLE:
			যদি (box_normalise_table(child, মূল, c) == false)
				ফেরত false;
			বিরতি;
		ক্ষেত্র BOX_INLINE:
		ক্ষেত্র BOX_INLINE_END:
		ক্ষেত্র BOX_INLINE_FLEX:
		ক্ষেত্র BOX_INLINE_BLOCK:
		ক্ষেত্র BOX_FLOAT_বাম:
		ক্ষেত্র BOX_FLOAT_ডান:
		ক্ষেত্র BOX_BR:
		ক্ষেত্র BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			প্রতিজ্ঞা(0);
			বিরতি;
		ক্ষেত্র BOX_TABLE_ROW_GROUP:
		ক্ষেত্র BOX_TABLE_ROW:
		ক্ষেত্র BOX_TABLE_CELL:
			/* insert implied table */
			প্রতিজ্ঞা(flex_container->style != ফাঁকা);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			যদি (style == ফাঁকা)
				ফেরত false;

			implied_flex_item = box_create(ফাঁকা, style, true,
					flex_container->href,
					flex_container->target,
					ফাঁকা, ফাঁকা, c->bctx);
			যদি (implied_flex_item == ফাঁকা) {
				css_computed_style_destroy(style);
				ফেরত false;
			}
			implied_flex_item->type = BOX_TABLE;

			যদি (child->পূর্ব == ফাঁকা)
				flex_container->children = implied_flex_item;
			অন্যথা
				child->পূর্ব->পরবর্তী = implied_flex_item;

			implied_flex_item->পূর্ব = child->পূর্ব;

			যখন (child != ফাঁকা && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(implied_flex_item, child);

				পরবর্তী_child = child->পরবর্তী;
				child->পরবর্তী = ফাঁকা;
				child = পরবর্তী_child;
			}

			implied_flex_item->last->পরবর্তী = ফাঁকা;
			implied_flex_item->পরবর্তী = পরবর্তী_child = child;
			যদি (implied_flex_item->পরবর্তী != ফাঁকা)
				implied_flex_item->পরবর্তী->পূর্ব = implied_flex_item;
			অন্যথা
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			যদি (box_normalise_table(implied_flex_item,
					মূল, c) == false)
				ফেরত false;
			বিরতি;
		পূর্বনির্ধারিত:
			প্রতিজ্ঞা(0);
		}
	}

	ফেরত true;
}

স্থির bool
box_normalise_inline_container(কাঠামো box *cont,
			       ধ্রুবক কাঠামো box *মূল,
			       html_content * c)
{
	কাঠামো box *child;
	কাঠামো box *পরবর্তী_child;

	প্রতিজ্ঞা(cont != ফাঁকা);
	প্রতিজ্ঞা(cont->type == BOX_INLINE_CONTAINER);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#endif

	জন্য (child = cont->children; child != ফাঁকা; child = পরবর্তী_child) {
		পরবর্তী_child = child->পরবর্তী;
		নির্বাচন (child->type) {
		ক্ষেত্র BOX_INLINE:
		ক্ষেত্র BOX_INLINE_END:
		ক্ষেত্র BOX_BR:
		ক্ষেত্র BOX_TEXT:
			/* ok */
			বিরতি;
		ক্ষেত্র BOX_INLINE_BLOCK:
			/* ok */
			যদি (box_normalise_block(child, মূল, c) == false)
				ফেরত false;
			বিরতি;
		ক্ষেত্র BOX_INLINE_FLEX:
			/* ok */
			যদি (box_normalise_flex(child, মূল, c) == false)
				ফেরত false;
			বিরতি;
		ক্ষেত্র BOX_FLOAT_বাম:
		ক্ষেত্র BOX_FLOAT_ডান:
			/* ok */
			প্রতিজ্ঞা(child->children != ফাঁকা);

			নির্বাচন (child->children->type) {
			ক্ষেত্র BOX_BLOCK:
				যদি (box_normalise_block(child->children, মূল,
						c) == false)
					ফেরত false;
				বিরতি;
			ক্ষেত্র BOX_TABLE:
				যদি (box_normalise_table(child->children, মূল,
						c) == false)
					ফেরত false;
				বিরতি;
			ক্ষেত্র BOX_FLEX:
				যদি (box_normalise_flex(child->children, মূল,
						c) == false)
					ফেরত false;
				বিরতি;
			পূর্বনির্ধারিত:
				প্রতিজ্ঞা(0);
			}

			যদি (child->children == ফাঁকা) {
				/* the child has destroyed itself: remove float */
				যদি (child->পূর্ব == ফাঁকা)
					child->parent->children = child->পরবর্তী;
				অন্যথা
					child->পূর্ব->পরবর্তী = child->পরবর্তী;
				যদি (child->পরবর্তী != ফাঁকা)
					child->পরবর্তী->পূর্ব = child->পূর্ব;
				অন্যথা
					child->parent->last = child->পূর্ব;

				box_free(child);
			}
			বিরতি;
		ক্ষেত্র BOX_FLEX:
		ক্ষেত্র BOX_BLOCK:
		ক্ষেত্র BOX_INLINE_CONTAINER:
		ক্ষেত্র BOX_TABLE:
		ক্ষেত্র BOX_TABLE_ROW_GROUP:
		ক্ষেত্র BOX_TABLE_ROW:
		ক্ষেত্র BOX_TABLE_CELL:
		পূর্বনির্ধারিত:
			প্রতিজ্ঞা(0);
		}
	}

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#endif

	ফেরত true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_block(কাঠামো box *block, ধ্রুবক কাঠামো box *মূল, html_content *c)
{
	কাঠামো box *child;
	কাঠামো box *পরবর্তী_child;
	কাঠামো box *table;
	css_computed_style *style;
	nscss_select_ctx ctx;

	প্রতিজ্ঞা(block != ফাঁকা);
	প্রতিজ্ঞা(মূল != ফাঁকা);

	ctx.মূল_style = মূল->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", block, block->type);
#endif

	প্রতিজ্ঞা(block->type == BOX_BLOCK || block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	জন্য (child = block->children; child != ফাঁকা; child = পরবর্তী_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", child,
		      child->type);
#endif

		পরবর্তী_child = child->পরবর্তী;	/* child may be destroyed */

		নির্বাচন (child->type) {
		ক্ষেত্র BOX_FLEX:
			/* ok */
			যদি (box_normalise_flex(child, মূল, c) == false)
				ফেরত false;
			বিরতি;
		ক্ষেত্র BOX_BLOCK:
			/* ok */
			যদি (box_normalise_block(child, মূল, c) == false)
				ফেরত false;
			বিরতি;
		ক্ষেত্র BOX_INLINE_CONTAINER:
			যদি (box_normalise_inline_container(child, মূল, c) == false)
				ফেরত false;
			বিরতি;
		ক্ষেত্র BOX_TABLE:
			যদি (box_normalise_table(child, মূল, c) == false)
				ফেরত false;
			বিরতি;
		ক্ষেত্র BOX_INLINE:
		ক্ষেত্র BOX_INLINE_END:
		ক্ষেত্র BOX_INLINE_FLEX:
		ক্ষেত্র BOX_INLINE_BLOCK:
		ক্ষেত্র BOX_FLOAT_বাম:
		ক্ষেত্র BOX_FLOAT_ডান:
		ক্ষেত্র BOX_BR:
		ক্ষেত্র BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			প্রতিজ্ঞা(0);
			বিরতি;
		ক্ষেত্র BOX_TABLE_ROW_GROUP:
		ক্ষেত্র BOX_TABLE_ROW:
		ক্ষেত্র BOX_TABLE_CELL:
			/* insert implied table */
			প্রতিজ্ঞা(block->style != ফাঁকা);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					block->style);
			যদি (style == ফাঁকা)
				ফেরত false;

			table = box_create(ফাঁকা, style, true, block->href,
					block->target, ফাঁকা, ফাঁকা, c->bctx);
			যদি (table == ফাঁকা) {
				css_computed_style_destroy(style);
				ফেরত false;
			}
			table->type = BOX_TABLE;

			যদি (child->পূর্ব == ফাঁকা)
				block->children = table;
			অন্যথা
				child->পূর্ব->পরবর্তী = table;

			table->পূর্ব = child->পূর্ব;

			যখন (child != ফাঁকা && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(table, child);

				পরবর্তী_child = child->পরবর্তী;
				child->পরবর্তী = ফাঁকা;
				child = পরবর্তী_child;
			}

			table->last->পরবর্তী = ফাঁকা;
			table->পরবর্তী = পরবর্তী_child = child;
			যদি (table->পরবর্তী != ফাঁকা)
				table->পরবর্তী->পূর্ব = table;
			অন্যথা
				block->last = table;
			table->parent = block;

			যদি (box_normalise_table(table, মূল, c) == false)
				ফেরত false;
			বিরতি;
		পূর্বনির্ধারিত:
			প্রতিজ্ঞা(0);
		}
	}

	ফেরত true;
}
