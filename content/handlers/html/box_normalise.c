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

#ضمن <assert.h>
#ضمن <stdbool.h>
#ضمن <string.h>

#ضمن "utils/log.h"
#ضمن "utils/errors.h"
#ضمن "css/select.h"

#ضمن "html/private.h"
#ضمن "html/table.h"
#ضمن "html/box.h"
#ضمن "html/box_manipulate.h"
#ضمن "html/box_normalise.h"

/* Define to enable box normalise debug */
#undef BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
بنية span_info {
	/** Number of rows this cell spans */
	unsigned صحيح row_span;
	/** Row group of cell */
	بنية box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_row;
};

/**
 * Column record for a table
 */
بنية columns {
	/** Current column index */
	unsigned صحيح current_column;
	/** Number of columns in main part of table 1..max columns */
	unsigned صحيح رقم_columns;
	/** Information about columns in main table, array [0, num_columns) */
	بنية span_info *spans;
	/** Number of rows in table */
	unsigned صحيح رقم_rows;
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
ساكن bool
calculate_table_row(بنية columns *col_info,
		    unsigned صحيح col_span,
		    unsigned صحيح row_span,
		    unsigned صحيح *start_column,
		    بنية box *cell)
{
	unsigned صحيح cell_start_col = col_info->current_column;
	unsigned صحيح cell_end_col;
	unsigned صحيح i;
	بنية span_info *spans;
	بنية box *rg = cell->parent->parent; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	بينما (col_info->spans[cell_start_col].row_span != 0 &&
			col_info->spans[cell_start_col].rg == rg) {
		cell_start_col++;
	}

	/* Update current column with calculated start */
	col_info->current_column = cell_start_col;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	إذا (col_span == 0)
		col_span = 1;

	cell_end_col = cell_start_col + col_span;

	إذا (col_info->رقم_columns < cell_end_col) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(col_info->spans,
				حجم *spans * (cell_end_col + 1));
		إذا (spans == عدم)
			إرجاع false;

		col_info->spans = spans;
		col_info->رقم_columns = cell_end_col;

		/* Mark new final column as sentinel */
		col_info->spans[cell_end_col].row_span = 0;
		col_info->spans[cell_end_col].auto_row = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	لكل (i = cell_start_col; i < cell_end_col; i++) {
		col_info->spans[i].row_span = (row_span == 0) ? 1 : row_span;
		col_info->spans[i].auto_row = (row_span == 0);
		col_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	col_info->current_column = cell_end_col;

	*start_column = cell_start_col;

	إرجاع true;
}


ساكن bool
box_normalise_table_row(بنية box *row,
			ثابت بنية box *جذر,
			بنية columns *col_info,
			html_content * c)
{
	بنية box *child;
	بنية box *next_child;
	بنية box *cell = عدم;
	css_computed_style *style;
	unsigned صحيح i;
	nscss_select_ctx ctx;

	أكد(row != عدم);
	أكد(row->type == BOX_TABLE_ROW);

	ctx.جذر_style = جذر->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", row);
#endif

	لكل (child = row->children; child != عدم; child = next_child) {
		next_child = child->next;

		بدل (child->type) {
		حالة BOX_TABLE_CELL:
			/* ok */
			إذا (box_normalise_block(child, جذر, c) == false)
				إرجاع false;
			cell = child;
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_BLOCK:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_TABLE:
		حالة BOX_TABLE_ROW_GROUP:
		حالة BOX_TABLE_ROW:
			/* insert implied table cell */
			أكد(row->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row->style);
			إذا (style == عدم)
				إرجاع false;

			cell = box_create(عدم, style, true, row->href,
					row->target, عدم, عدم, c->bctx);
			إذا (cell == عدم) {
				css_computed_style_destroy(style);
				إرجاع false;
			}
			cell->type = BOX_TABLE_CELL;

			إذا (child->prev == عدم)
				row->children = cell;
			وإلا
				child->prev->next = cell;

			cell->prev = child->prev;

			بينما (child != عدم && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW)) {
				box_add_child(cell, child);

				next_child = child->next;
				child->next = عدم;
				child = next_child;
			}

			أكد(cell->last != عدم);

			cell->last->next = عدم;
			cell->next = next_child = child;
			إذا (cell->next != عدم)
				cell->next->prev = cell;
			وإلا
				row->last = cell;
			cell->parent = row;

			إذا (box_normalise_block(cell, جذر, c) == false)
				إرجاع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_END:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_BLOCK:
		حالة BOX_FLOAT_يسار:
		حالة BOX_FLOAT_يمين:
		حالة BOX_BR:
		حالة BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		افتراضي:
			أكد(0);
		}

		إذا (calculate_table_row(col_info, cell->columns, cell->rows,
				&cell->start_column, cell) == false)
			إرجاع false;
	}


	/* Update row spanning details for all columns */
	لكل (i = 0; i < col_info->رقم_columns; i++) {
		إذا (col_info->spans[i].row_span != 0 &&
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

	إرجاع true;
}


ساكن bool
box_normalise_table_row_group(بنية box *row_group,
			      ثابت بنية box *جذر,
			      بنية columns *col_info,
			      html_content * c)
{
	بنية box *child;
	بنية box *next_child;
	بنية box *row;
	css_computed_style *style;
	nscss_select_ctx ctx;
	unsigned صحيح group_row_تعداد = 0;

	أكد(row_group != 0);
	أكد(row_group->type == BOX_TABLE_ROW_GROUP);

	ctx.جذر_style = جذر->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", row_group);
#endif

	لكل (child = row_group->children; child != عدم; child = next_child) {
		next_child = child->next;

		بدل (child->type) {
		حالة BOX_TABLE_ROW:
			/* ok */
			group_row_تعداد++;
			إذا (box_normalise_table_row(child, جذر, col_info,
					c) == false)
				إرجاع false;
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_BLOCK:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_TABLE:
		حالة BOX_TABLE_ROW_GROUP:
		حالة BOX_TABLE_CELL:
			/* insert implied table row */
			أكد(row_group->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row_group->style);
			إذا (style == عدم)
				إرجاع false;

			row = box_create(عدم, style, true, row_group->href,
					row_group->target, عدم, عدم, c->bctx);
			إذا (row == عدم) {
				css_computed_style_destroy(style);
				إرجاع false;
			}
			row->type = BOX_TABLE_ROW;

			إذا (child->prev == عدم)
				row_group->children = row;
			وإلا
				child->prev->next = row;

			row->prev = child->prev;

			بينما (child != عدم && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row, child);

				next_child = child->next;
				child->next = عدم;
				child = next_child;
			}

			أكد(row->last != عدم);

			row->last->next = عدم;
			row->next = next_child = child;
			إذا (row->next != عدم)
				row->next->prev = row;
			وإلا
				row_group->last = row;
			row->parent = row_group;

			group_row_تعداد++;
			إذا (box_normalise_table_row(row, جذر, col_info,
					c) == false)
				إرجاع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_END:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_BLOCK:
		حالة BOX_FLOAT_يسار:
		حالة BOX_FLOAT_يمين:
		حالة BOX_BR:
		حالة BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		افتراضي:
			أكد(0);
		}
	}

	إذا (row_group->children == عدم) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#endif

		أكد(row_group->style != عدم);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		إذا (style == عدم) {
			إرجاع false;
		}

		row = box_create(عدم, style, true, row_group->href,
				row_group->target, عدم, عدم, c->bctx);
		إذا (row == عدم) {
			css_computed_style_destroy(style);
			إرجاع false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		group_row_تعداد = 1;

		/* Keep table's row count in sync */
		col_info->رقم_rows++;
	}

	row_group->rows = group_row_تعداد;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", row_group);
#endif

	إرجاع true;
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
ساكن bool
box_normalise_table_spans(بنية box *table,
			  ثابت بنية box *جذر,
			  بنية span_info *spans,
			  html_content *c)
{
	بنية box *table_row_group;
	بنية box *table_row;
	بنية box *table_cell;
	unsigned صحيح rows_يسار = table->rows;
	unsigned صحيح group_rows_يسار;
	unsigned صحيح col;
	nscss_select_ctx ctx;

	ctx.جذر_style = جذر->style;

	/* Clear span data */
	memset(spans, 0, table->columns * حجم(بنية span_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	لكل (table_row_group = table->children;
	     table_row_group != عدم;
	     table_row_group = table_row_group->next) {

		group_rows_يسار = table_row_group->rows;

		لكل (table_row = table_row_group->children;
		     table_row != عدم;
		     table_row = table_row->next) {

			لكل (table_cell = table_row->children;
			     table_cell != عدم;
			     table_cell = table_cell->next) {

				/* colspan = 0 -> colspan = 1 */
				إذا (table_cell->columns == 0) {
					table_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				إذا (table_cell->rows == 0) {
					table_cell->rows = group_rows_يسار;
				}

				/* limit rowspans within group */
				إذا (table_cell->rows > group_rows_يسار) {
					table_cell->rows = group_rows_يسار;
				}

				/* Record span information */
				لكل (col = table_cell->start_column;
						col < table_cell->start_column +
						table_cell->columns; col++) {
					spans[col].row_span = table_cell->rows;
				}
			}

			/* Reduce span count of each column */
			لكل (col = 0; col < table->columns; col++) {
				إذا (spans[col].row_span == 0) {
					unsigned صحيح start = col;
					css_computed_style *style;
					بنية box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					أكد(table_row->style != عدم);

					/* Find width of gap */
					بينما (col < table->columns &&
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
					إذا (style == عدم)
						إرجاع false;

					cell = box_create(عدم, style, true,
							table_row->href,
							table_row->target,
							عدم, عدم, c->bctx);
					إذا (cell == عدم) {
						css_computed_style_destroy(
								style);
						إرجاع false;
					}
					cell->type = BOX_TABLE_CELL;

					cell->rows = 1;
					cell->columns = col - start;
					cell->start_column = start;

					/* Find place to insert cell */
					لكل (prev = table_row->children;
							prev != عدم;
							prev = prev->next) {
						إذا (prev->start_column +
							prev->columns ==
								start)
							اكسر;
						إذا (prev->next == عدم)
							اكسر;
					}

					/* Insert it */
					إذا (prev == عدم) {
						إذا (table_row->children != عدم)
							table_row->children->
								prev = cell;
						وإلا
							table_row->last = cell;

						cell->next =
							table_row->children;
						table_row->children = cell;
					} وإلا {
						إذا (prev->next != عدم)
							prev->next->prev = cell;
						وإلا
							table_row->last = cell;

						cell->next = prev->next;
						prev->next = cell;
						cell->prev = prev;
					}
					cell->parent = table_row;
				} وإلا {
					spans[col].row_span--;
				}
			}

			أكد(rows_يسار > 0);

			rows_يسار--;
		}

		group_rows_يسار--;
	}

	إرجاع true;
}


ساكن bool
box_normalise_table(بنية box *table, ثابت بنية box *جذر, html_content * c)
{
	بنية box *child;
	بنية box *next_child;
	بنية box *row_group;
	css_computed_style *style;
	بنية columns col_info;
	nscss_select_ctx ctx;

	أكد(table != عدم);
	أكد(table->type == BOX_TABLE);

	ctx.جذر_style = جذر->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", table);
#endif

	col_info.رقم_columns = 1;
	col_info.current_column = 0;
	col_info.spans = malloc(2 * حجم *col_info.spans);
	إذا (col_info.spans == عدم)
		إرجاع false;

	col_info.spans[0].row_span = col_info.spans[1].row_span = 0;
	col_info.spans[0].auto_row = false;
	col_info.spans[1].auto_row = false;
	col_info.رقم_rows = 0;

	لكل (child = table->children; child != عدم; child = next_child) {
		next_child = child->next;
		بدل (child->type) {
		حالة BOX_TABLE_ROW_GROUP:
			/* ok */
			إذا (box_normalise_table_row_group(child, جذر,
					&col_info, c) == false) {
				حرر(col_info.spans);
				إرجاع false;
			}
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_BLOCK:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_TABLE:
		حالة BOX_TABLE_ROW:
		حالة BOX_TABLE_CELL:
			/* insert implied table row group */
			أكد(table->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					table->style);
			إذا (style == عدم) {
				حرر(col_info.spans);
				إرجاع false;
			}

			row_group = box_create(عدم, style, true, table->href,
					table->target, عدم, عدم, c->bctx);
			إذا (row_group == عدم) {
				css_computed_style_destroy(style);
				حرر(col_info.spans);
				إرجاع false;
			}

			row_group->type = BOX_TABLE_ROW_GROUP;

			إذا (child->prev == عدم)
				table->children = row_group;
			وإلا
				child->prev->next = row_group;

			row_group->prev = child->prev;

			بينما (child != عدم && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row_group, child);

				next_child = child->next;
				child->next = عدم;
				child = next_child;
			}

			أكد(row_group->last != عدم);

			row_group->last->next = عدم;
			row_group->next = next_child = child;
			إذا (row_group->next != عدم)
				row_group->next->prev = row_group;
			وإلا
				table->last = row_group;
			row_group->parent = table;

			إذا (box_normalise_table_row_group(row_group, جذر,
					&col_info, c) == false) {
				حرر(col_info.spans);
				إرجاع false;
			}
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_END:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_BLOCK:
		حالة BOX_FLOAT_يسار:
		حالة BOX_FLOAT_يمين:
		حالة BOX_BR:
		حالة BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		افتراضي:
			fprintf(stderr, "%i\n", child->type);
			أكد(0);
		}
	}

	table->columns = col_info.رقم_columns;
	table->rows = col_info.رقم_rows;

	إذا (table->children == عدم) {
		بنية box *row;

#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#endif

		أكد(table->style != عدم);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				table->style);
		إذا (style == عدم) {
			حرر(col_info.spans);
			إرجاع false;
		}

		row_group = box_create(عدم, style, true, table->href,
				table->target, عدم, عدم, c->bctx);
		إذا (row_group == عدم) {
			css_computed_style_destroy(style);
			حرر(col_info.spans);
			إرجاع false;
		}
		row_group->type = BOX_TABLE_ROW_GROUP;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		إذا (style == عدم) {
			box_free(row_group);
			حرر(col_info.spans);
			إرجاع false;
		}

		row = box_create(عدم, style, true, row_group->href,
				row_group->target, عدم, عدم, c->bctx);
		إذا (row == عدم) {
			css_computed_style_destroy(style);
			box_free(row_group);
			حرر(col_info.spans);
			إرجاع false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		row_group->parent = table;
		table->children = table->last = row_group;

		table->rows = 1;
	}

	إذا (box_normalise_table_spans(table, جذر, col_info.spans, c) == false) {
		حرر(col_info.spans);
		إرجاع false;
	}

	حرر(col_info.spans);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", table);
#endif

	إرجاع true;
}

ساكن bool box_normalise_flex(
		بنية box *flex_container,
		ثابت بنية box *جذر,
		html_content *c)
{
	بنية box *child;
	بنية box *next_child;
	بنية box *implied_flex_item;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(flex_container != عدم);
	أكد(جذر != عدم);

	ctx.جذر_style = جذر->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#endif

	أكد(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	لكل (child = flex_container->children; child != عدم; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				child, child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		بدل (child->type) {
		حالة BOX_FLEX:
			/* ok */
			إذا (box_normalise_flex(child, جذر, c) == false)
				إرجاع false;
			اكسر;
		حالة BOX_BLOCK:
			/* ok */
			إذا (box_normalise_block(child, جذر, c) == false)
				إرجاع false;
			اكسر;
		حالة BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			أكد(flex_container->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			إذا (style == عدم)
				إرجاع false;

			implied_flex_item = box_create(عدم, style, true,
					flex_container->href,
					flex_container->target,
					عدم, عدم, c->bctx);
			إذا (implied_flex_item == عدم) {
				css_computed_style_destroy(style);
				إرجاع false;
			}
			implied_flex_item->type = BOX_BLOCK;

			إذا (child->prev == عدم)
				flex_container->children = implied_flex_item;
			وإلا
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			بينما (child != عدم &&
					child->type == BOX_INLINE_CONTAINER) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = عدم;
				child = next_child;
			}

			implied_flex_item->last->next = عدم;
			implied_flex_item->next = next_child = child;
			إذا (implied_flex_item->next != عدم)
				implied_flex_item->next->prev = implied_flex_item;
			وإلا
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			إذا (box_normalise_block(implied_flex_item,
					جذر, c) == false)
				إرجاع false;
			اكسر;

		حالة BOX_TABLE:
			إذا (box_normalise_table(child, جذر, c) == false)
				إرجاع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_END:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_BLOCK:
		حالة BOX_FLOAT_يسار:
		حالة BOX_FLOAT_يمين:
		حالة BOX_BR:
		حالة BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		حالة BOX_TABLE_ROW_GROUP:
		حالة BOX_TABLE_ROW:
		حالة BOX_TABLE_CELL:
			/* insert implied table */
			أكد(flex_container->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			إذا (style == عدم)
				إرجاع false;

			implied_flex_item = box_create(عدم, style, true,
					flex_container->href,
					flex_container->target,
					عدم, عدم, c->bctx);
			إذا (implied_flex_item == عدم) {
				css_computed_style_destroy(style);
				إرجاع false;
			}
			implied_flex_item->type = BOX_TABLE;

			إذا (child->prev == عدم)
				flex_container->children = implied_flex_item;
			وإلا
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			بينما (child != عدم && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = عدم;
				child = next_child;
			}

			implied_flex_item->last->next = عدم;
			implied_flex_item->next = next_child = child;
			إذا (implied_flex_item->next != عدم)
				implied_flex_item->next->prev = implied_flex_item;
			وإلا
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			إذا (box_normalise_table(implied_flex_item,
					جذر, c) == false)
				إرجاع false;
			اكسر;
		افتراضي:
			أكد(0);
		}
	}

	إرجاع true;
}

ساكن bool
box_normalise_inline_container(بنية box *cont,
			       ثابت بنية box *جذر,
			       html_content * c)
{
	بنية box *child;
	بنية box *next_child;

	أكد(cont != عدم);
	أكد(cont->type == BOX_INLINE_CONTAINER);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#endif

	لكل (child = cont->children; child != عدم; child = next_child) {
		next_child = child->next;
		بدل (child->type) {
		حالة BOX_INLINE:
		حالة BOX_INLINE_END:
		حالة BOX_BR:
		حالة BOX_TEXT:
			/* ok */
			اكسر;
		حالة BOX_INLINE_BLOCK:
			/* ok */
			إذا (box_normalise_block(child, جذر, c) == false)
				إرجاع false;
			اكسر;
		حالة BOX_INLINE_FLEX:
			/* ok */
			إذا (box_normalise_flex(child, جذر, c) == false)
				إرجاع false;
			اكسر;
		حالة BOX_FLOAT_يسار:
		حالة BOX_FLOAT_يمين:
			/* ok */
			أكد(child->children != عدم);

			بدل (child->children->type) {
			حالة BOX_BLOCK:
				إذا (box_normalise_block(child->children, جذر,
						c) == false)
					إرجاع false;
				اكسر;
			حالة BOX_TABLE:
				إذا (box_normalise_table(child->children, جذر,
						c) == false)
					إرجاع false;
				اكسر;
			حالة BOX_FLEX:
				إذا (box_normalise_flex(child->children, جذر,
						c) == false)
					إرجاع false;
				اكسر;
			افتراضي:
				أكد(0);
			}

			إذا (child->children == عدم) {
				/* the child has destroyed itself: remove float */
				إذا (child->prev == عدم)
					child->parent->children = child->next;
				وإلا
					child->prev->next = child->next;
				إذا (child->next != عدم)
					child->next->prev = child->prev;
				وإلا
					child->parent->last = child->prev;

				box_free(child);
			}
			اكسر;
		حالة BOX_FLEX:
		حالة BOX_BLOCK:
		حالة BOX_INLINE_CONTAINER:
		حالة BOX_TABLE:
		حالة BOX_TABLE_ROW_GROUP:
		حالة BOX_TABLE_ROW:
		حالة BOX_TABLE_CELL:
		افتراضي:
			أكد(0);
		}
	}

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#endif

	إرجاع true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_block(بنية box *block, ثابت بنية box *جذر, html_content *c)
{
	بنية box *child;
	بنية box *next_child;
	بنية box *table;
	css_computed_style *style;
	nscss_select_ctx ctx;

	أكد(block != عدم);
	أكد(جذر != عدم);

	ctx.جذر_style = جذر->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", block, block->type);
#endif

	أكد(block->type == BOX_BLOCK || block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	لكل (child = block->children; child != عدم; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", child,
		      child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		بدل (child->type) {
		حالة BOX_FLEX:
			/* ok */
			إذا (box_normalise_flex(child, جذر, c) == false)
				إرجاع false;
			اكسر;
		حالة BOX_BLOCK:
			/* ok */
			إذا (box_normalise_block(child, جذر, c) == false)
				إرجاع false;
			اكسر;
		حالة BOX_INLINE_CONTAINER:
			إذا (box_normalise_inline_container(child, جذر, c) == false)
				إرجاع false;
			اكسر;
		حالة BOX_TABLE:
			إذا (box_normalise_table(child, جذر, c) == false)
				إرجاع false;
			اكسر;
		حالة BOX_INLINE:
		حالة BOX_INLINE_END:
		حالة BOX_INLINE_FLEX:
		حالة BOX_INLINE_BLOCK:
		حالة BOX_FLOAT_يسار:
		حالة BOX_FLOAT_يمين:
		حالة BOX_BR:
		حالة BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			أكد(0);
			اكسر;
		حالة BOX_TABLE_ROW_GROUP:
		حالة BOX_TABLE_ROW:
		حالة BOX_TABLE_CELL:
			/* insert implied table */
			أكد(block->style != عدم);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					block->style);
			إذا (style == عدم)
				إرجاع false;

			table = box_create(عدم, style, true, block->href,
					block->target, عدم, عدم, c->bctx);
			إذا (table == عدم) {
				css_computed_style_destroy(style);
				إرجاع false;
			}
			table->type = BOX_TABLE;

			إذا (child->prev == عدم)
				block->children = table;
			وإلا
				child->prev->next = table;

			table->prev = child->prev;

			بينما (child != عدم && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(table, child);

				next_child = child->next;
				child->next = عدم;
				child = next_child;
			}

			table->last->next = عدم;
			table->next = next_child = child;
			إذا (table->next != عدم)
				table->next->prev = table;
			وإلا
				block->last = table;
			table->parent = block;

			إذا (box_normalise_table(table, جذر, c) == false)
				إرجاع false;
			اكسر;
		افتراضي:
			أكد(0);
		}
	}

	إرجاع true;
}
