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

#شامل <assert.h>
#شامل <stdbool.h>
#شامل <string.h>

#شامل "utils/log.h"
#شامل "utils/errors.h"
#شامل "css/select.h"

#شامل "html/private.h"
#شامل "html/table.h"
#شامل "html/box.h"
#شامل "html/box_manipulate.h"
#شامل "html/box_normalise.h"

/* Define to enable box normalise debug */
#undef BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
ساختار span_info {
	/** Number of rows this cell spans */
	unsigned صحیح row_span;
	/** Row group of cell */
	ساختار box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_row;
};

/**
 * Column record for a table
 */
ساختار columns {
	/** Current column index */
	unsigned صحیح current_column;
	/** Number of columns in main part of table 1..max columns */
	unsigned صحیح رقم_columns;
	/** Information about columns in main table, array [0, num_columns) */
	ساختار span_info *spans;
	/** Number of rows in table */
	unsigned صحیح رقم_rows;
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
ایستا bool
calculate_table_row(ساختار columns *col_info,
		    unsigned صحیح col_span,
		    unsigned صحیح row_span,
		    unsigned صحیح *start_column,
		    ساختار box *cell)
{
	unsigned صحیح cell_start_col = col_info->current_column;
	unsigned صحیح cell_end_col;
	unsigned صحیح i;
	ساختار span_info *spans;
	ساختار box *rg = cell->parent->parent; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	وقتی (col_info->spans[cell_start_col].row_span != 0 &&
			col_info->spans[cell_start_col].rg == rg) {
		cell_start_col++;
	}

	/* Update current column with calculated start */
	col_info->current_column = cell_start_col;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	اگر (col_span == 0)
		col_span = 1;

	cell_end_col = cell_start_col + col_span;

	اگر (col_info->رقم_columns < cell_end_col) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(col_info->spans,
				اندازه *spans * (cell_end_col + 1));
		اگر (spans == پوچ)
			بازگشت false;

		col_info->spans = spans;
		col_info->رقم_columns = cell_end_col;

		/* Mark new final column as sentinel */
		col_info->spans[cell_end_col].row_span = 0;
		col_info->spans[cell_end_col].auto_row = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	برای (i = cell_start_col; i < cell_end_col; i++) {
		col_info->spans[i].row_span = (row_span == 0) ? 1 : row_span;
		col_info->spans[i].auto_row = (row_span == 0);
		col_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	col_info->current_column = cell_end_col;

	*start_column = cell_start_col;

	بازگشت true;
}


ایستا bool
box_normalise_table_row(ساختار box *row,
			ثابت ساختار box *ریشه,
			ساختار columns *col_info,
			html_content * c)
{
	ساختار box *child;
	ساختار box *next_child;
	ساختار box *cell = پوچ;
	css_computed_style *style;
	unsigned صحیح i;
	nscss_select_ctx ctx;

	أكد(row != پوچ);
	أكد(row->type == BOX_TABLE_ROW);

	ctx.ریشه_style = ریشه->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", row);
#endif

	برای (child = row->children; child != پوچ; child = next_child) {
		next_child = child->next;

		انتخاب (child->type) {
		حالت BOX_TABLE_CELL:
			/* ok */
			اگر (box_normalise_block(child, ریشه, c) == false)
				بازگشت false;
			cell = child;
			بشکن;
		حالت BOX_FLEX:
		حالت BOX_BLOCK:
		حالت BOX_INLINE_CONTAINER:
		حالت BOX_TABLE:
		حالت BOX_TABLE_ROW_GROUP:
		حالت BOX_TABLE_ROW:
			/* insert implied table cell */
			أكد(row->style != پوچ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row->style);
			اگر (style == پوچ)
				بازگشت false;

			cell = box_create(پوچ, style, true, row->href,
					row->target, پوچ, پوچ, c->bctx);
			اگر (cell == پوچ) {
				css_computed_style_destroy(style);
				بازگشت false;
			}
			cell->type = BOX_TABLE_CELL;

			اگر (child->prev == پوچ)
				row->children = cell;
			وگرنه
				child->prev->next = cell;

			cell->prev = child->prev;

			وقتی (child != پوچ && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW)) {
				box_add_child(cell, child);

				next_child = child->next;
				child->next = پوچ;
				child = next_child;
			}

			أكد(cell->last != پوچ);

			cell->last->next = پوچ;
			cell->next = next_child = child;
			اگر (cell->next != پوچ)
				cell->next->prev = cell;
			وگرنه
				row->last = cell;
			cell->parent = row;

			اگر (box_normalise_block(cell, ریشه, c) == false)
				بازگشت false;
			بشکن;
		حالت BOX_INLINE:
		حالت BOX_INLINE_END:
		حالت BOX_INLINE_FLEX:
		حالت BOX_INLINE_BLOCK:
		حالت BOX_FLOAT_چپ:
		حالت BOX_FLOAT_راست:
		حالت BOX_BR:
		حالت BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			بشکن;
		پیشفرض:
			أكد(0);
		}

		اگر (calculate_table_row(col_info, cell->columns, cell->rows,
				&cell->start_column, cell) == false)
			بازگشت false;
	}


	/* Update row spanning details for all columns */
	برای (i = 0; i < col_info->رقم_columns; i++) {
		اگر (col_info->spans[i].row_span != 0 &&
				col_info->spans[i].auto_row == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			col_info->spans[i].row_span--;
		}
	}

	/* Reset current column for next row */
	col_info->current_column = 0;

	/* Increment row counter */
	col_info->رقم_rows++;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", row);
#endif

	بازگشت true;
}


ایستا bool
box_normalise_table_row_group(ساختار box *row_group,
			      ثابت ساختار box *ریشه,
			      ساختار columns *col_info,
			      html_content * c)
{
	ساختار box *child;
	ساختار box *next_child;
	ساختار box *row;
	css_computed_style *style;
	nscss_select_ctx ctx;
	unsigned صحیح group_row_شمار = 0;

	أكد(row_group != 0);
	أكد(row_group->type == BOX_TABLE_ROW_GROUP);

	ctx.ریشه_style = ریشه->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", row_group);
#endif

	برای (child = row_group->children; child != پوچ; child = next_child) {
		next_child = child->next;

		انتخاب (child->type) {
		حالت BOX_TABLE_ROW:
			/* ok */
			group_row_شمار++;
			اگر (box_normalise_table_row(child, ریشه, col_info,
					c) == false)
				بازگشت false;
			بشکن;
		حالت BOX_FLEX:
		حالت BOX_BLOCK:
		حالت BOX_INLINE_CONTAINER:
		حالت BOX_TABLE:
		حالت BOX_TABLE_ROW_GROUP:
		حالت BOX_TABLE_CELL:
			/* insert implied table row */
			أكد(row_group->style != پوچ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row_group->style);
			اگر (style == پوچ)
				بازگشت false;

			row = box_create(پوچ, style, true, row_group->href,
					row_group->target, پوچ, پوچ, c->bctx);
			اگر (row == پوچ) {
				css_computed_style_destroy(style);
				بازگشت false;
			}
			row->type = BOX_TABLE_ROW;

			اگر (child->prev == پوچ)
				row_group->children = row;
			وگرنه
				child->prev->next = row;

			row->prev = child->prev;

			وقتی (child != پوچ && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row, child);

				next_child = child->next;
				child->next = پوچ;
				child = next_child;
			}

			أكد(row->last != پوچ);

			row->last->next = پوچ;
			row->next = next_child = child;
			اگر (row->next != پوچ)
				row->next->prev = row;
			وگرنه
				row_group->last = row;
			row->parent = row_group;

			group_row_شمار++;
			اگر (box_normalise_table_row(row, ریشه, col_info,
					c) == false)
				بازگشت false;
			بشکن;
		حالت BOX_INLINE:
		حالت BOX_INLINE_END:
		حالت BOX_INLINE_FLEX:
		حالت BOX_INLINE_BLOCK:
		حالت BOX_FLOAT_چپ:
		حالت BOX_FLOAT_راست:
		حالت BOX_BR:
		حالت BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			بشکن;
		پیشفرض:
			أكد(0);
		}
	}

	اگر (row_group->children == پوچ) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#endif

		أكد(row_group->style != پوچ);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		اگر (style == پوچ) {
			بازگشت false;
		}

		row = box_create(پوچ, style, true, row_group->href,
				row_group->target, پوچ, پوچ, c->bctx);
		اگر (row == پوچ) {
			css_computed_style_destroy(style);
			بازگشت false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		group_row_شمار = 1;

		/* Keep table's row count in sync */
		col_info->رقم_rows++;
	}

	row_group->rows = group_row_شمار;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", row_group);
#endif

	بازگشت true;
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
ایستا bool
box_normalise_table_spans(ساختار box *table,
			  ثابت ساختار box *ریشه,
			  ساختار span_info *spans,
			  html_content *c)
{
	ساختار box *table_row_group;
	ساختار box *table_row;
	ساختار box *table_cell;
	unsigned صحیح rows_چپ = table->rows;
	unsigned صحیح group_rows_چپ;
	unsigned صحیح col;
	nscss_select_ctx ctx;

	ctx.ریشه_style = ریشه->style;

	/* Clear span data */
	memset(spans, 0, table->columns * اندازه(ساختار span_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	برای (table_row_group = table->children;
	     table_row_group != پوچ;
	     table_row_group = table_row_group->next) {

		group_rows_چپ = table_row_group->rows;

		برای (table_row = table_row_group->children;
		     table_row != پوچ;
		     table_row = table_row->next) {

			برای (table_cell = table_row->children;
			     table_cell != پوچ;
			     table_cell = table_cell->next) {

				/* colspan = 0 -> colspan = 1 */
				اگر (table_cell->columns == 0) {
					table_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				اگر (table_cell->rows == 0) {
					table_cell->rows = group_rows_چپ;
				}

				/* limit rowspans within group */
				اگر (table_cell->rows > group_rows_چپ) {
					table_cell->rows = group_rows_چپ;
				}

				/* Record span information */
				برای (col = table_cell->start_column;
						col < table_cell->start_column +
						table_cell->columns; col++) {
					spans[col].row_span = table_cell->rows;
				}
			}

			/* Reduce span count of each column */
			برای (col = 0; col < table->columns; col++) {
				اگر (spans[col].row_span == 0) {
					unsigned صحیح start = col;
					css_computed_style *style;
					ساختار box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					أكد(table_row->style != پوچ);

					/* Find width of gap */
					وقتی (col < table->columns &&
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
					اگر (style == پوچ)
						بازگشت false;

					cell = box_create(پوچ, style, true,
							table_row->href,
							table_row->target,
							پوچ, پوچ, c->bctx);
					اگر (cell == پوچ) {
						css_computed_style_destroy(
								style);
						بازگشت false;
					}
					cell->type = BOX_TABLE_CELL;

					cell->rows = 1;
					cell->columns = col - start;
					cell->start_column = start;

					/* Find place to insert cell */
					برای (prev = table_row->children;
							prev != پوچ;
							prev = prev->next) {
						اگر (prev->start_column +
							prev->columns ==
								start)
							بشکن;
						اگر (prev->next == پوچ)
							بشکن;
					}

					/* Insert it */
					اگر (prev == پوچ) {
						اگر (table_row->children != پوچ)
							table_row->children->
								prev = cell;
						وگرنه
							table_row->last = cell;

						cell->next =
							table_row->children;
						table_row->children = cell;
					} وگرنه {
						اگر (prev->next != پوچ)
							prev->next->prev = cell;
						وگرنه
							table_row->last = cell;

						cell->next = prev->next;
						prev->next = cell;
						cell->prev = prev;
					}
					cell->parent = table_row;
				} وگرنه {
					spans[col].row_span--;
				}
			}

			أكد(rows_چپ > 0);

			rows_چپ--;
		}

		group_rows_چپ--;
	}

	بازگشت true;
}


ایستا bool
box_normalise_table(ساختار box *table, ثابت ساختار box *ریشه, html_content * c)
{
	ساختار box *child;
	ساختار box *next_child;
	ساختار box *row_group;
	css_computed_style *style;
	ساختار columns col_info;
	nscss_select_ctx ctx;

	أكد(table != پوچ);
	أكد(table->type == BOX_TABLE);

	ctx.ریشه_style = ریشه->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", table);
#endif

	col_info.رقم_columns = 1;
	col_info.current_column = 0;
	col_info.spans = malloc(2 * اندازه *col_info.spans);
	اگر (col_info.spans == پوچ)
		بازگشت false;

	col_info.spans[0].row_span = col_info.spans[1].row_span = 0;
	col_info.spans[0].auto_row = false;
	col_info.spans[1].auto_row = false;
	col_info.رقم_rows = 0;

	برای (child = table->children; child != پوچ; child = next_child) {
		next_child = child->next;
		انتخاب (child->type) {
		حالت BOX_TABLE_ROW_GROUP:
			/* ok */
			اگر (box_normalise_table_row_group(child, ریشه,
					&col_info, c) == false) {
				آزاد(col_info.spans);
				بازگشت false;
			}
			بشکن;
		حالت BOX_FLEX:
		حالت BOX_BLOCK:
		حالت BOX_INLINE_CONTAINER:
		حالت BOX_TABLE:
		حالت BOX_TABLE_ROW:
		حالت BOX_TABLE_CELL:
			/* insert implied table row group */
			أكد(table->style != پوچ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					table->style);
			اگر (style == پوچ) {
				آزاد(col_info.spans);
				بازگشت false;
			}

			row_group = box_create(پوچ, style, true, table->href,
					table->target, پوچ, پوچ, c->bctx);
			اگر (row_group == پوچ) {
				css_computed_style_destroy(style);
				آزاد(col_info.spans);
				بازگشت false;
			}

			row_group->type = BOX_TABLE_ROW_GROUP;

			اگر (child->prev == پوچ)
				table->children = row_group;
			وگرنه
				child->prev->next = row_group;

			row_group->prev = child->prev;

			وقتی (child != پوچ && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row_group, child);

				next_child = child->next;
				child->next = پوچ;
				child = next_child;
			}

			أكد(row_group->last != پوچ);

			row_group->last->next = پوچ;
			row_group->next = next_child = child;
			اگر (row_group->next != پوچ)
				row_group->next->prev = row_group;
			وگرنه
				table->last = row_group;
			row_group->parent = table;

			اگر (box_normalise_table_row_group(row_group, ریشه,
					&col_info, c) == false) {
				آزاد(col_info.spans);
				بازگشت false;
			}
			بشکن;
		حالت BOX_INLINE:
		حالت BOX_INLINE_END:
		حالت BOX_INLINE_FLEX:
		حالت BOX_INLINE_BLOCK:
		حالت BOX_FLOAT_چپ:
		حالت BOX_FLOAT_راست:
		حالت BOX_BR:
		حالت BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			بشکن;
		پیشفرض:
			fprintf(stderr, "%i\n", child->type);
			أكد(0);
		}
	}

	table->columns = col_info.رقم_columns;
	table->rows = col_info.رقم_rows;

	اگر (table->children == پوچ) {
		ساختار box *row;

#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#endif

		أكد(table->style != پوچ);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				table->style);
		اگر (style == پوچ) {
			آزاد(col_info.spans);
			بازگشت false;
		}

		row_group = box_create(پوچ, style, true, table->href,
				table->target, پوچ, پوچ, c->bctx);
		اگر (row_group == پوچ) {
			css_computed_style_destroy(style);
			آزاد(col_info.spans);
			بازگشت false;
		}
		row_group->type = BOX_TABLE_ROW_GROUP;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		اگر (style == پوچ) {
			box_free(row_group);
			آزاد(col_info.spans);
			بازگشت false;
		}

		row = box_create(پوچ, style, true, row_group->href,
				row_group->target, پوچ, پوچ, c->bctx);
		اگر (row == پوچ) {
			css_computed_style_destroy(style);
			box_free(row_group);
			آزاد(col_info.spans);
			بازگشت false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		row_group->parent = table;
		table->children = table->last = row_group;

		table->rows = 1;
	}

	اگر (box_normalise_table_spans(table, ریشه, col_info.spans, c) == false) {
		آزاد(col_info.spans);
		بازگشت false;
	}

	آزاد(col_info.spans);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", table);
#endif

	بازگشت true;
}

ایستا bool box_normalise_flex(
		ساختار box *flex_container,
		ثابت ساختار box *ریشه,
		html_content *c)
{
	ساختار box *child;
	ساختار box *next_child;
	ساختار box *implied_flex_item;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(flex_container != پوچ);
	أكد(ریشه != پوچ);

	ctx.ریشه_style = ریشه->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#endif

	أكد(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	برای (child = flex_container->children; child != پوچ; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				child, child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		انتخاب (child->type) {
		حالت BOX_FLEX:
			/* ok */
			اگر (box_normalise_flex(child, ریشه, c) == false)
				بازگشت false;
			بشکن;
		حالت BOX_BLOCK:
			/* ok */
			اگر (box_normalise_block(child, ریشه, c) == false)
				بازگشت false;
			بشکن;
		حالت BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			أكد(flex_container->style != پوچ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			اگر (style == پوچ)
				بازگشت false;

			implied_flex_item = box_create(پوچ, style, true,
					flex_container->href,
					flex_container->target,
					پوچ, پوچ, c->bctx);
			اگر (implied_flex_item == پوچ) {
				css_computed_style_destroy(style);
				بازگشت false;
			}
			implied_flex_item->type = BOX_BLOCK;

			اگر (child->prev == پوچ)
				flex_container->children = implied_flex_item;
			وگرنه
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			وقتی (child != پوچ &&
					child->type == BOX_INLINE_CONTAINER) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = پوچ;
				child = next_child;
			}

			implied_flex_item->last->next = پوچ;
			implied_flex_item->next = next_child = child;
			اگر (implied_flex_item->next != پوچ)
				implied_flex_item->next->prev = implied_flex_item;
			وگرنه
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			اگر (box_normalise_block(implied_flex_item,
					ریشه, c) == false)
				بازگشت false;
			بشکن;

		حالت BOX_TABLE:
			اگر (box_normalise_table(child, ریشه, c) == false)
				بازگشت false;
			بشکن;
		حالت BOX_INLINE:
		حالت BOX_INLINE_END:
		حالت BOX_INLINE_FLEX:
		حالت BOX_INLINE_BLOCK:
		حالت BOX_FLOAT_چپ:
		حالت BOX_FLOAT_راست:
		حالت BOX_BR:
		حالت BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			بشکن;
		حالت BOX_TABLE_ROW_GROUP:
		حالت BOX_TABLE_ROW:
		حالت BOX_TABLE_CELL:
			/* insert implied table */
			أكد(flex_container->style != پوچ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			اگر (style == پوچ)
				بازگشت false;

			implied_flex_item = box_create(پوچ, style, true,
					flex_container->href,
					flex_container->target,
					پوچ, پوچ, c->bctx);
			اگر (implied_flex_item == پوچ) {
				css_computed_style_destroy(style);
				بازگشت false;
			}
			implied_flex_item->type = BOX_TABLE;

			اگر (child->prev == پوچ)
				flex_container->children = implied_flex_item;
			وگرنه
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			وقتی (child != پوچ && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = پوچ;
				child = next_child;
			}

			implied_flex_item->last->next = پوچ;
			implied_flex_item->next = next_child = child;
			اگر (implied_flex_item->next != پوچ)
				implied_flex_item->next->prev = implied_flex_item;
			وگرنه
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			اگر (box_normalise_table(implied_flex_item,
					ریشه, c) == false)
				بازگشت false;
			بشکن;
		پیشفرض:
			أكد(0);
		}
	}

	بازگشت true;
}

ایستا bool
box_normalise_inline_container(ساختار box *cont,
			       ثابت ساختار box *ریشه,
			       html_content * c)
{
	ساختار box *child;
	ساختار box *next_child;

	أكد(cont != پوچ);
	أكد(cont->type == BOX_INLINE_CONTAINER);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#endif

	برای (child = cont->children; child != پوچ; child = next_child) {
		next_child = child->next;
		انتخاب (child->type) {
		حالت BOX_INLINE:
		حالت BOX_INLINE_END:
		حالت BOX_BR:
		حالت BOX_TEXT:
			/* ok */
			بشکن;
		حالت BOX_INLINE_BLOCK:
			/* ok */
			اگر (box_normalise_block(child, ریشه, c) == false)
				بازگشت false;
			بشکن;
		حالت BOX_INLINE_FLEX:
			/* ok */
			اگر (box_normalise_flex(child, ریشه, c) == false)
				بازگشت false;
			بشکن;
		حالت BOX_FLOAT_چپ:
		حالت BOX_FLOAT_راست:
			/* ok */
			أكد(child->children != پوچ);

			انتخاب (child->children->type) {
			حالت BOX_BLOCK:
				اگر (box_normalise_block(child->children, ریشه,
						c) == false)
					بازگشت false;
				بشکن;
			حالت BOX_TABLE:
				اگر (box_normalise_table(child->children, ریشه,
						c) == false)
					بازگشت false;
				بشکن;
			حالت BOX_FLEX:
				اگر (box_normalise_flex(child->children, ریشه,
						c) == false)
					بازگشت false;
				بشکن;
			پیشفرض:
				أكد(0);
			}

			اگر (child->children == پوچ) {
				/* the child has destroyed itself: remove float */
				اگر (child->prev == پوچ)
					child->parent->children = child->next;
				وگرنه
					child->prev->next = child->next;
				اگر (child->next != پوچ)
					child->next->prev = child->prev;
				وگرنه
					child->parent->last = child->prev;

				box_free(child);
			}
			بشکن;
		حالت BOX_FLEX:
		حالت BOX_BLOCK:
		حالت BOX_INLINE_CONTAINER:
		حالت BOX_TABLE:
		حالت BOX_TABLE_ROW_GROUP:
		حالت BOX_TABLE_ROW:
		حالت BOX_TABLE_CELL:
		پیشفرض:
			أكد(0);
		}
	}

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#endif

	بازگشت true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_block(ساختار box *block, ثابت ساختار box *ریشه, html_content *c)
{
	ساختار box *child;
	ساختار box *next_child;
	ساختار box *table;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(block != پوچ);
	أكد(ریشه != پوچ);

	ctx.ریشه_style = ریشه->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", block, block->type);
#endif

	أكد(block->type == BOX_BLOCK || block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	برای (child = block->children; child != پوچ; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", child,
		      child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		انتخاب (child->type) {
		حالت BOX_FLEX:
			/* ok */
			اگر (box_normalise_flex(child, ریشه, c) == false)
				بازگشت false;
			بشکن;
		حالت BOX_BLOCK:
			/* ok */
			اگر (box_normalise_block(child, ریشه, c) == false)
				بازگشت false;
			بشکن;
		حالت BOX_INLINE_CONTAINER:
			اگر (box_normalise_inline_container(child, ریشه, c) == false)
				بازگشت false;
			بشکن;
		حالت BOX_TABLE:
			اگر (box_normalise_table(child, ریشه, c) == false)
				بازگشت false;
			بشکن;
		حالت BOX_INLINE:
		حالت BOX_INLINE_END:
		حالت BOX_INLINE_FLEX:
		حالت BOX_INLINE_BLOCK:
		حالت BOX_FLOAT_چپ:
		حالت BOX_FLOAT_راست:
		حالت BOX_BR:
		حالت BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			بشکن;
		حالت BOX_TABLE_ROW_GROUP:
		حالت BOX_TABLE_ROW:
		حالت BOX_TABLE_CELL:
			/* insert implied table */
			أكد(block->style != پوچ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					block->style);
			اگر (style == پوچ)
				بازگشت false;

			table = box_create(پوچ, style, true, block->href,
					block->target, پوچ, پوچ, c->bctx);
			اگر (table == پوچ) {
				css_computed_style_destroy(style);
				بازگشت false;
			}
			table->type = BOX_TABLE;

			اگر (child->prev == پوچ)
				block->children = table;
			وگرنه
				child->prev->next = table;

			table->prev = child->prev;

			وقتی (child != پوچ && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(table, child);

				next_child = child->next;
				child->next = پوچ;
				child = next_child;
			}

			table->last->next = پوچ;
			table->next = next_child = child;
			اگر (table->next != پوچ)
				table->next->prev = table;
			وگرنه
				block->last = table;
			table->parent = block;

			اگر (box_normalise_table(table, ریشه, c) == false)
				بازگشت false;
			بشکن;
		پیشفرض:
			أكد(0);
		}
	}

	بازگشت true;
}
