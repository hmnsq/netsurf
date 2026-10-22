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
সংরচনা span_info {
	/** Number of rows this cell spans */
	unsigned পূর্ণাঙ্ক row_span;
	/** Row group of cell */
	সংরচনা box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_row;
};

/**
 * Column record for a table
 */
সংরচনা columns {
	/** Current column index */
	unsigned পূর্ণাঙ্ক current_column;
	/** Number of columns in main part of table 1..max columns */
	unsigned পূর্ণাঙ্ক অঙ্ক_columns;
	/** Information about columns in main table, array [0, num_columns) */
	সংরচনা span_info *spans;
	/** Number of rows in table */
	unsigned পূর্ণাঙ্ক অঙ্ক_rows;
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
calculate_table_row(সংরচনা columns *col_info,
		    unsigned পূর্ণাঙ্ক col_span,
		    unsigned পূর্ণাঙ্ক row_span,
		    unsigned পূর্ণাঙ্ক *start_column,
		    সংরচনা box *cell)
{
	unsigned পূর্ণাঙ্ক cell_start_col = col_info->current_column;
	unsigned পূর্ণাঙ্ক cell_end_col;
	unsigned পূর্ণাঙ্ক i;
	সংরচনা span_info *spans;
	সংরচনা box *rg = cell->parent->parent; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	যাবত্ (col_info->spans[cell_start_col].row_span != 0 &&
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
				পরিমাণ *spans * (cell_end_col + 1));
		যদি (spans == রিক্ত)
			প্রত্যাগম false;

		col_info->spans = spans;
		col_info->অঙ্ক_columns = cell_end_col;

		/* Mark new final column as sentinel */
		col_info->spans[cell_end_col].row_span = 0;
		col_info->spans[cell_end_col].auto_row = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	প্রতি (i = cell_start_col; i < cell_end_col; i++) {
		col_info->spans[i].row_span = (row_span == 0) ? 1 : row_span;
		col_info->spans[i].auto_row = (row_span == 0);
		col_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	col_info->current_column = cell_end_col;

	*start_column = cell_start_col;

	প্রত্যাগম true;
}


স্থির bool
box_normalise_table_row(সংরচনা box *row,
			ধ্রুব সংরচনা box *মূল,
			সংরচনা columns *col_info,
			html_content * c)
{
	সংরচনা box *child;
	সংরচনা box *next_child;
	সংরচনা box *cell = রিক্ত;
	css_computed_style *style;
	unsigned পূর্ণাঙ্ক i;
	nscss_select_ctx ctx;

	প্রতিজ্ঞা(row != রিক্ত);
	প্রতিজ্ঞা(row->type == BOX_TABLE_ROW);

	ctx.মূল_style = মূল->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", row);
#endif

	প্রতি (child = row->children; child != রিক্ত; child = next_child) {
		next_child = child->next;

		বিকল্প (child->type) {
		পক্ষ BOX_TABLE_CELL:
			/* ok */
			যদি (box_normalise_block(child, মূল, c) == false)
				প্রত্যাগম false;
			cell = child;
			ভঙ্গ;
		পক্ষ BOX_FLEX:
		পক্ষ BOX_BLOCK:
		পক্ষ BOX_INLINE_CONTAINER:
		পক্ষ BOX_TABLE:
		পক্ষ BOX_TABLE_ROW_GROUP:
		পক্ষ BOX_TABLE_ROW:
			/* insert implied table cell */
			প্রতিজ্ঞা(row->style != রিক্ত);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row->style);
			যদি (style == রিক্ত)
				প্রত্যাগম false;

			cell = box_create(রিক্ত, style, true, row->href,
					row->target, রিক্ত, রিক্ত, c->bctx);
			যদি (cell == রিক্ত) {
				css_computed_style_destroy(style);
				প্রত্যাগম false;
			}
			cell->type = BOX_TABLE_CELL;

			যদি (child->prev == রিক্ত)
				row->children = cell;
			অন্যথা
				child->prev->next = cell;

			cell->prev = child->prev;

			যাবত্ (child != রিক্ত && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW)) {
				box_add_child(cell, child);

				next_child = child->next;
				child->next = রিক্ত;
				child = next_child;
			}

			প্রতিজ্ঞা(cell->last != রিক্ত);

			cell->last->next = রিক্ত;
			cell->next = next_child = child;
			যদি (cell->next != রিক্ত)
				cell->next->prev = cell;
			অন্যথা
				row->last = cell;
			cell->parent = row;

			যদি (box_normalise_block(cell, মূল, c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		পক্ষ BOX_INLINE:
		পক্ষ BOX_INLINE_END:
		পক্ষ BOX_INLINE_FLEX:
		পক্ষ BOX_INLINE_BLOCK:
		পক্ষ BOX_FLOAT_বাম:
		পক্ষ BOX_FLOAT_ডান:
		পক্ষ BOX_BR:
		পক্ষ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			প্রতিজ্ঞা(0);
			ভঙ্গ;
		স্বভাব:
			প্রতিজ্ঞা(0);
		}

		যদি (calculate_table_row(col_info, cell->columns, cell->rows,
				&cell->start_column, cell) == false)
			প্রত্যাগম false;
	}


	/* Update row spanning details for all columns */
	প্রতি (i = 0; i < col_info->অঙ্ক_columns; i++) {
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

	প্রত্যাগম true;
}


স্থির bool
box_normalise_table_row_group(সংরচনা box *row_group,
			      ধ্রুব সংরচনা box *মূল,
			      সংরচনা columns *col_info,
			      html_content * c)
{
	সংরচনা box *child;
	সংরচনা box *next_child;
	সংরচনা box *row;
	css_computed_style *style;
	nscss_select_ctx ctx;
	unsigned পূর্ণাঙ্ক group_row_গুনতি = 0;

	প্রতিজ্ঞা(row_group != 0);
	প্রতিজ্ঞা(row_group->type == BOX_TABLE_ROW_GROUP);

	ctx.মূল_style = মূল->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", row_group);
#endif

	প্রতি (child = row_group->children; child != রিক্ত; child = next_child) {
		next_child = child->next;

		বিকল্প (child->type) {
		পক্ষ BOX_TABLE_ROW:
			/* ok */
			group_row_গুনতি++;
			যদি (box_normalise_table_row(child, মূল, col_info,
					c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		পক্ষ BOX_FLEX:
		পক্ষ BOX_BLOCK:
		পক্ষ BOX_INLINE_CONTAINER:
		পক্ষ BOX_TABLE:
		পক্ষ BOX_TABLE_ROW_GROUP:
		পক্ষ BOX_TABLE_CELL:
			/* insert implied table row */
			প্রতিজ্ঞা(row_group->style != রিক্ত);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row_group->style);
			যদি (style == রিক্ত)
				প্রত্যাগম false;

			row = box_create(রিক্ত, style, true, row_group->href,
					row_group->target, রিক্ত, রিক্ত, c->bctx);
			যদি (row == রিক্ত) {
				css_computed_style_destroy(style);
				প্রত্যাগম false;
			}
			row->type = BOX_TABLE_ROW;

			যদি (child->prev == রিক্ত)
				row_group->children = row;
			অন্যথা
				child->prev->next = row;

			row->prev = child->prev;

			যাবত্ (child != রিক্ত && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row, child);

				next_child = child->next;
				child->next = রিক্ত;
				child = next_child;
			}

			প্রতিজ্ঞা(row->last != রিক্ত);

			row->last->next = রিক্ত;
			row->next = next_child = child;
			যদি (row->next != রিক্ত)
				row->next->prev = row;
			অন্যথা
				row_group->last = row;
			row->parent = row_group;

			group_row_গুনতি++;
			যদি (box_normalise_table_row(row, মূল, col_info,
					c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		পক্ষ BOX_INLINE:
		পক্ষ BOX_INLINE_END:
		পক্ষ BOX_INLINE_FLEX:
		পক্ষ BOX_INLINE_BLOCK:
		পক্ষ BOX_FLOAT_বাম:
		পক্ষ BOX_FLOAT_ডান:
		পক্ষ BOX_BR:
		পক্ষ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			প্রতিজ্ঞা(0);
			ভঙ্গ;
		স্বভাব:
			প্রতিজ্ঞা(0);
		}
	}

	যদি (row_group->children == রিক্ত) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#endif

		প্রতিজ্ঞা(row_group->style != রিক্ত);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		যদি (style == রিক্ত) {
			প্রত্যাগম false;
		}

		row = box_create(রিক্ত, style, true, row_group->href,
				row_group->target, রিক্ত, রিক্ত, c->bctx);
		যদি (row == রিক্ত) {
			css_computed_style_destroy(style);
			প্রত্যাগম false;
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

	প্রত্যাগম true;
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
box_normalise_table_spans(সংরচনা box *table,
			  ধ্রুব সংরচনা box *মূল,
			  সংরচনা span_info *spans,
			  html_content *c)
{
	সংরচনা box *table_row_group;
	সংরচনা box *table_row;
	সংরচনা box *table_cell;
	unsigned পূর্ণাঙ্ক rows_বাম = table->rows;
	unsigned পূর্ণাঙ্ক group_rows_বাম;
	unsigned পূর্ণাঙ্ক col;
	nscss_select_ctx ctx;

	ctx.মূল_style = মূল->style;

	/* Clear span data */
	memset(spans, 0, table->columns * পরিমাণ(সংরচনা span_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	প্রতি (table_row_group = table->children;
	     table_row_group != রিক্ত;
	     table_row_group = table_row_group->next) {

		group_rows_বাম = table_row_group->rows;

		প্রতি (table_row = table_row_group->children;
		     table_row != রিক্ত;
		     table_row = table_row->next) {

			প্রতি (table_cell = table_row->children;
			     table_cell != রিক্ত;
			     table_cell = table_cell->next) {

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
				প্রতি (col = table_cell->start_column;
						col < table_cell->start_column +
						table_cell->columns; col++) {
					spans[col].row_span = table_cell->rows;
				}
			}

			/* Reduce span count of each column */
			প্রতি (col = 0; col < table->columns; col++) {
				যদি (spans[col].row_span == 0) {
					unsigned পূর্ণাঙ্ক start = col;
					css_computed_style *style;
					সংরচনা box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					প্রতিজ্ঞা(table_row->style != রিক্ত);

					/* Find width of gap */
					যাবত্ (col < table->columns &&
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
					যদি (style == রিক্ত)
						প্রত্যাগম false;

					cell = box_create(রিক্ত, style, true,
							table_row->href,
							table_row->target,
							রিক্ত, রিক্ত, c->bctx);
					যদি (cell == রিক্ত) {
						css_computed_style_destroy(
								style);
						প্রত্যাগম false;
					}
					cell->type = BOX_TABLE_CELL;

					cell->rows = 1;
					cell->columns = col - start;
					cell->start_column = start;

					/* Find place to insert cell */
					প্রতি (prev = table_row->children;
							prev != রিক্ত;
							prev = prev->next) {
						যদি (prev->start_column +
							prev->columns ==
								start)
							ভঙ্গ;
						যদি (prev->next == রিক্ত)
							ভঙ্গ;
					}

					/* Insert it */
					যদি (prev == রিক্ত) {
						যদি (table_row->children != রিক্ত)
							table_row->children->
								prev = cell;
						অন্যথা
							table_row->last = cell;

						cell->next =
							table_row->children;
						table_row->children = cell;
					} অন্যথা {
						যদি (prev->next != রিক্ত)
							prev->next->prev = cell;
						অন্যথা
							table_row->last = cell;

						cell->next = prev->next;
						prev->next = cell;
						cell->prev = prev;
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

	প্রত্যাগম true;
}


স্থির bool
box_normalise_table(সংরচনা box *table, ধ্রুব সংরচনা box *মূল, html_content * c)
{
	সংরচনা box *child;
	সংরচনা box *next_child;
	সংরচনা box *row_group;
	css_computed_style *style;
	সংরচনা columns col_info;
	nscss_select_ctx ctx;

	প্রতিজ্ঞা(table != রিক্ত);
	প্রতিজ্ঞা(table->type == BOX_TABLE);

	ctx.মূল_style = মূল->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", table);
#endif

	col_info.অঙ্ক_columns = 1;
	col_info.current_column = 0;
	col_info.spans = malloc(2 * পরিমাণ *col_info.spans);
	যদি (col_info.spans == রিক্ত)
		প্রত্যাগম false;

	col_info.spans[0].row_span = col_info.spans[1].row_span = 0;
	col_info.spans[0].auto_row = false;
	col_info.spans[1].auto_row = false;
	col_info.অঙ্ক_rows = 0;

	প্রতি (child = table->children; child != রিক্ত; child = next_child) {
		next_child = child->next;
		বিকল্প (child->type) {
		পক্ষ BOX_TABLE_ROW_GROUP:
			/* ok */
			যদি (box_normalise_table_row_group(child, মূল,
					&col_info, c) == false) {
				মুক্ত(col_info.spans);
				প্রত্যাগম false;
			}
			ভঙ্গ;
		পক্ষ BOX_FLEX:
		পক্ষ BOX_BLOCK:
		পক্ষ BOX_INLINE_CONTAINER:
		পক্ষ BOX_TABLE:
		পক্ষ BOX_TABLE_ROW:
		পক্ষ BOX_TABLE_CELL:
			/* insert implied table row group */
			প্রতিজ্ঞা(table->style != রিক্ত);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					table->style);
			যদি (style == রিক্ত) {
				মুক্ত(col_info.spans);
				প্রত্যাগম false;
			}

			row_group = box_create(রিক্ত, style, true, table->href,
					table->target, রিক্ত, রিক্ত, c->bctx);
			যদি (row_group == রিক্ত) {
				css_computed_style_destroy(style);
				মুক্ত(col_info.spans);
				প্রত্যাগম false;
			}

			row_group->type = BOX_TABLE_ROW_GROUP;

			যদি (child->prev == রিক্ত)
				table->children = row_group;
			অন্যথা
				child->prev->next = row_group;

			row_group->prev = child->prev;

			যাবত্ (child != রিক্ত && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row_group, child);

				next_child = child->next;
				child->next = রিক্ত;
				child = next_child;
			}

			প্রতিজ্ঞা(row_group->last != রিক্ত);

			row_group->last->next = রিক্ত;
			row_group->next = next_child = child;
			যদি (row_group->next != রিক্ত)
				row_group->next->prev = row_group;
			অন্যথা
				table->last = row_group;
			row_group->parent = table;

			যদি (box_normalise_table_row_group(row_group, মূল,
					&col_info, c) == false) {
				মুক্ত(col_info.spans);
				প্রত্যাগম false;
			}
			ভঙ্গ;
		পক্ষ BOX_INLINE:
		পক্ষ BOX_INLINE_END:
		পক্ষ BOX_INLINE_FLEX:
		পক্ষ BOX_INLINE_BLOCK:
		পক্ষ BOX_FLOAT_বাম:
		পক্ষ BOX_FLOAT_ডান:
		পক্ষ BOX_BR:
		পক্ষ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			প্রতিজ্ঞা(0);
			ভঙ্গ;
		স্বভাব:
			fprintf(stderr, "%i\n", child->type);
			প্রতিজ্ঞা(0);
		}
	}

	table->columns = col_info.অঙ্ক_columns;
	table->rows = col_info.অঙ্ক_rows;

	যদি (table->children == রিক্ত) {
		সংরচনা box *row;

#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#endif

		প্রতিজ্ঞা(table->style != রিক্ত);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				table->style);
		যদি (style == রিক্ত) {
			মুক্ত(col_info.spans);
			প্রত্যাগম false;
		}

		row_group = box_create(রিক্ত, style, true, table->href,
				table->target, রিক্ত, রিক্ত, c->bctx);
		যদি (row_group == রিক্ত) {
			css_computed_style_destroy(style);
			মুক্ত(col_info.spans);
			প্রত্যাগম false;
		}
		row_group->type = BOX_TABLE_ROW_GROUP;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		যদি (style == রিক্ত) {
			box_free(row_group);
			মুক্ত(col_info.spans);
			প্রত্যাগম false;
		}

		row = box_create(রিক্ত, style, true, row_group->href,
				row_group->target, রিক্ত, রিক্ত, c->bctx);
		যদি (row == রিক্ত) {
			css_computed_style_destroy(style);
			box_free(row_group);
			মুক্ত(col_info.spans);
			প্রত্যাগম false;
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
		প্রত্যাগম false;
	}

	মুক্ত(col_info.spans);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", table);
#endif

	প্রত্যাগম true;
}

স্থির bool box_normalise_flex(
		সংরচনা box *flex_container,
		ধ্রুব সংরচনা box *মূল,
		html_content *c)
{
	সংরচনা box *child;
	সংরচনা box *next_child;
	সংরচনা box *implied_flex_item;
	css_computed_style *style;
	nscss_select_ctx ctx;

	প্রতিজ্ঞা(flex_container != রিক্ত);
	প্রতিজ্ঞা(মূল != রিক্ত);

	ctx.মূল_style = মূল->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#endif

	প্রতিজ্ঞা(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	প্রতি (child = flex_container->children; child != রিক্ত; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				child, child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		বিকল্প (child->type) {
		পক্ষ BOX_FLEX:
			/* ok */
			যদি (box_normalise_flex(child, মূল, c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		পক্ষ BOX_BLOCK:
			/* ok */
			যদি (box_normalise_block(child, মূল, c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		পক্ষ BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			প্রতিজ্ঞা(flex_container->style != রিক্ত);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			যদি (style == রিক্ত)
				প্রত্যাগম false;

			implied_flex_item = box_create(রিক্ত, style, true,
					flex_container->href,
					flex_container->target,
					রিক্ত, রিক্ত, c->bctx);
			যদি (implied_flex_item == রিক্ত) {
				css_computed_style_destroy(style);
				প্রত্যাগম false;
			}
			implied_flex_item->type = BOX_BLOCK;

			যদি (child->prev == রিক্ত)
				flex_container->children = implied_flex_item;
			অন্যথা
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			যাবত্ (child != রিক্ত &&
					child->type == BOX_INLINE_CONTAINER) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = রিক্ত;
				child = next_child;
			}

			implied_flex_item->last->next = রিক্ত;
			implied_flex_item->next = next_child = child;
			যদি (implied_flex_item->next != রিক্ত)
				implied_flex_item->next->prev = implied_flex_item;
			অন্যথা
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			যদি (box_normalise_block(implied_flex_item,
					মূল, c) == false)
				প্রত্যাগম false;
			ভঙ্গ;

		পক্ষ BOX_TABLE:
			যদি (box_normalise_table(child, মূল, c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		পক্ষ BOX_INLINE:
		পক্ষ BOX_INLINE_END:
		পক্ষ BOX_INLINE_FLEX:
		পক্ষ BOX_INLINE_BLOCK:
		পক্ষ BOX_FLOAT_বাম:
		পক্ষ BOX_FLOAT_ডান:
		পক্ষ BOX_BR:
		পক্ষ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			প্রতিজ্ঞা(0);
			ভঙ্গ;
		পক্ষ BOX_TABLE_ROW_GROUP:
		পক্ষ BOX_TABLE_ROW:
		পক্ষ BOX_TABLE_CELL:
			/* insert implied table */
			প্রতিজ্ঞা(flex_container->style != রিক্ত);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			যদি (style == রিক্ত)
				প্রত্যাগম false;

			implied_flex_item = box_create(রিক্ত, style, true,
					flex_container->href,
					flex_container->target,
					রিক্ত, রিক্ত, c->bctx);
			যদি (implied_flex_item == রিক্ত) {
				css_computed_style_destroy(style);
				প্রত্যাগম false;
			}
			implied_flex_item->type = BOX_TABLE;

			যদি (child->prev == রিক্ত)
				flex_container->children = implied_flex_item;
			অন্যথা
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			যাবত্ (child != রিক্ত && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = রিক্ত;
				child = next_child;
			}

			implied_flex_item->last->next = রিক্ত;
			implied_flex_item->next = next_child = child;
			যদি (implied_flex_item->next != রিক্ত)
				implied_flex_item->next->prev = implied_flex_item;
			অন্যথা
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			যদি (box_normalise_table(implied_flex_item,
					মূল, c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		স্বভাব:
			প্রতিজ্ঞা(0);
		}
	}

	প্রত্যাগম true;
}

স্থির bool
box_normalise_inline_container(সংরচনা box *cont,
			       ধ্রুব সংরচনা box *মূল,
			       html_content * c)
{
	সংরচনা box *child;
	সংরচনা box *next_child;

	প্রতিজ্ঞা(cont != রিক্ত);
	প্রতিজ্ঞা(cont->type == BOX_INLINE_CONTAINER);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#endif

	প্রতি (child = cont->children; child != রিক্ত; child = next_child) {
		next_child = child->next;
		বিকল্প (child->type) {
		পক্ষ BOX_INLINE:
		পক্ষ BOX_INLINE_END:
		পক্ষ BOX_BR:
		পক্ষ BOX_TEXT:
			/* ok */
			ভঙ্গ;
		পক্ষ BOX_INLINE_BLOCK:
			/* ok */
			যদি (box_normalise_block(child, মূল, c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		পক্ষ BOX_INLINE_FLEX:
			/* ok */
			যদি (box_normalise_flex(child, মূল, c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		পক্ষ BOX_FLOAT_বাম:
		পক্ষ BOX_FLOAT_ডান:
			/* ok */
			প্রতিজ্ঞা(child->children != রিক্ত);

			বিকল্প (child->children->type) {
			পক্ষ BOX_BLOCK:
				যদি (box_normalise_block(child->children, মূল,
						c) == false)
					প্রত্যাগম false;
				ভঙ্গ;
			পক্ষ BOX_TABLE:
				যদি (box_normalise_table(child->children, মূল,
						c) == false)
					প্রত্যাগম false;
				ভঙ্গ;
			পক্ষ BOX_FLEX:
				যদি (box_normalise_flex(child->children, মূল,
						c) == false)
					প্রত্যাগম false;
				ভঙ্গ;
			স্বভাব:
				প্রতিজ্ঞা(0);
			}

			যদি (child->children == রিক্ত) {
				/* the child has destroyed itself: remove float */
				যদি (child->prev == রিক্ত)
					child->parent->children = child->next;
				অন্যথা
					child->prev->next = child->next;
				যদি (child->next != রিক্ত)
					child->next->prev = child->prev;
				অন্যথা
					child->parent->last = child->prev;

				box_free(child);
			}
			ভঙ্গ;
		পক্ষ BOX_FLEX:
		পক্ষ BOX_BLOCK:
		পক্ষ BOX_INLINE_CONTAINER:
		পক্ষ BOX_TABLE:
		পক্ষ BOX_TABLE_ROW_GROUP:
		পক্ষ BOX_TABLE_ROW:
		পক্ষ BOX_TABLE_CELL:
		স্বভাব:
			প্রতিজ্ঞা(0);
		}
	}

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#endif

	প্রত্যাগম true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_block(সংরচনা box *block, ধ্রুব সংরচনা box *মূল, html_content *c)
{
	সংরচনা box *child;
	সংরচনা box *next_child;
	সংরচনা box *table;
	css_computed_style *style;
	nscss_select_ctx ctx;

	প্রতিজ্ঞা(block != রিক্ত);
	প্রতিজ্ঞা(মূল != রিক্ত);

	ctx.মূল_style = মূল->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", block, block->type);
#endif

	প্রতিজ্ঞা(block->type == BOX_BLOCK || block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	প্রতি (child = block->children; child != রিক্ত; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", child,
		      child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		বিকল্প (child->type) {
		পক্ষ BOX_FLEX:
			/* ok */
			যদি (box_normalise_flex(child, মূল, c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		পক্ষ BOX_BLOCK:
			/* ok */
			যদি (box_normalise_block(child, মূল, c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		পক্ষ BOX_INLINE_CONTAINER:
			যদি (box_normalise_inline_container(child, মূল, c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		পক্ষ BOX_TABLE:
			যদি (box_normalise_table(child, মূল, c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		পক্ষ BOX_INLINE:
		পক্ষ BOX_INLINE_END:
		পক্ষ BOX_INLINE_FLEX:
		পক্ষ BOX_INLINE_BLOCK:
		পক্ষ BOX_FLOAT_বাম:
		পক্ষ BOX_FLOAT_ডান:
		পক্ষ BOX_BR:
		পক্ষ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			প্রতিজ্ঞা(0);
			ভঙ্গ;
		পক্ষ BOX_TABLE_ROW_GROUP:
		পক্ষ BOX_TABLE_ROW:
		পক্ষ BOX_TABLE_CELL:
			/* insert implied table */
			প্রতিজ্ঞা(block->style != রিক্ত);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					block->style);
			যদি (style == রিক্ত)
				প্রত্যাগম false;

			table = box_create(রিক্ত, style, true, block->href,
					block->target, রিক্ত, রিক্ত, c->bctx);
			যদি (table == রিক্ত) {
				css_computed_style_destroy(style);
				প্রত্যাগম false;
			}
			table->type = BOX_TABLE;

			যদি (child->prev == রিক্ত)
				block->children = table;
			অন্যথা
				child->prev->next = table;

			table->prev = child->prev;

			যাবত্ (child != রিক্ত && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(table, child);

				next_child = child->next;
				child->next = রিক্ত;
				child = next_child;
			}

			table->last->next = রিক্ত;
			table->next = next_child = child;
			যদি (table->next != রিক্ত)
				table->next->prev = table;
			অন্যথা
				block->last = table;
			table->parent = block;

			যদি (box_normalise_table(table, মূল, c) == false)
				প্রত্যাগম false;
			ভঙ্গ;
		স্বভাব:
			প্রতিজ্ঞা(0);
		}
	}

	প্রত্যাগম true;
}
