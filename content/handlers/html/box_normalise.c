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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "css/select.h"

#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/table.h"
#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_manipulate.h"
#अन्तर्भाव "html/box_normalise.h"

/* Define to enable box normalise debug */
#undef BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
संरचना span_info {
	/** Number of rows this cell spans */
	अचिन्हित पूर्णांक row_span;
	/** Row group of cell */
	संरचना box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_row;
};

/**
 * Column record for a table
 */
संरचना columns {
	/** Current column index */
	अचिन्हित पूर्णांक current_column;
	/** Number of columns in main part of table 1..max columns */
	अचिन्हित पूर्णांक अंक_columns;
	/** Information about columns in main table, array [0, num_columns) */
	संरचना span_info *spans;
	/** Number of rows in table */
	अचिन्हित पूर्णांक अंक_rows;
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
स्थिर bool
calculate_table_row(संरचना columns *col_info,
		    अचिन्हित पूर्णांक col_span,
		    अचिन्हित पूर्णांक row_span,
		    अचिन्हित पूर्णांक *start_column,
		    संरचना box *cell)
{
	अचिन्हित पूर्णांक cell_start_col = col_info->current_column;
	अचिन्हित पूर्णांक cell_end_col;
	अचिन्हित पूर्णांक i;
	संरचना span_info *spans;
	संरचना box *rg = cell->parent->parent; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	पर्यंत (col_info->spans[cell_start_col].row_span != 0 &&
			col_info->spans[cell_start_col].rg == rg) {
		cell_start_col++;
	}

	/* Update current column with calculated start */
	col_info->current_column = cell_start_col;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	जर (col_span == 0)
		col_span = 1;

	cell_end_col = cell_start_col + col_span;

	जर (col_info->अंक_columns < cell_end_col) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = पुनः_स्मृति(col_info->spans,
				आकार *spans * (cell_end_col + 1));
		जर (spans == रिक्त)
			परत false;

		col_info->spans = spans;
		col_info->अंक_columns = cell_end_col;

		/* Mark new final column as sentinel */
		col_info->spans[cell_end_col].row_span = 0;
		col_info->spans[cell_end_col].auto_row = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	साठी (i = cell_start_col; i < cell_end_col; i++) {
		col_info->spans[i].row_span = (row_span == 0) ? 1 : row_span;
		col_info->spans[i].auto_row = (row_span == 0);
		col_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	col_info->current_column = cell_end_col;

	*start_column = cell_start_col;

	परत true;
}


स्थिर bool
box_normalise_table_row(संरचना box *row,
			स्थिरांक संरचना box *मूळ,
			संरचना columns *col_info,
			html_content * c)
{
	संरचना box *child;
	संरचना box *next_child;
	संरचना box *cell = रिक्त;
	css_computed_style *style;
	अचिन्हित पूर्णांक i;
	nscss_select_ctx ctx;

	प्रतिज्ञा(row != रिक्त);
	प्रतिज्ञा(row->type == BOX_TABLE_ROW);

	ctx.मूळ_style = मूळ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", row);
#endif

	साठी (child = row->children; child != रिक्त; child = next_child) {
		next_child = child->next;

		निवड (child->type) {
		स्थिती BOX_TABLE_CELL:
			/* ok */
			जर (box_normalise_block(child, मूळ, c) == false)
				परत false;
			cell = child;
			थांबा;
		स्थिती BOX_FLEX:
		स्थिती BOX_BLOCK:
		स्थिती BOX_INLINE_CONTAINER:
		स्थिती BOX_TABLE:
		स्थिती BOX_TABLE_ROW_GROUP:
		स्थिती BOX_TABLE_ROW:
			/* insert implied table cell */
			प्रतिज्ञा(row->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row->style);
			जर (style == रिक्त)
				परत false;

			cell = box_create(रिक्त, style, true, row->href,
					row->target, रिक्त, रिक्त, c->bctx);
			जर (cell == रिक्त) {
				css_computed_style_destroy(style);
				परत false;
			}
			cell->type = BOX_TABLE_CELL;

			जर (child->prev == रिक्त)
				row->children = cell;
			अन्यथा
				child->prev->next = cell;

			cell->prev = child->prev;

			पर्यंत (child != रिक्त && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW)) {
				box_add_child(cell, child);

				next_child = child->next;
				child->next = रिक्त;
				child = next_child;
			}

			प्रतिज्ञा(cell->last != रिक्त);

			cell->last->next = रिक्त;
			cell->next = next_child = child;
			जर (cell->next != रिक्त)
				cell->next->prev = cell;
			अन्यथा
				row->last = cell;
			cell->parent = row;

			जर (box_normalise_block(cell, मूळ, c) == false)
				परत false;
			थांबा;
		स्थिती BOX_INLINE:
		स्थिती BOX_INLINE_END:
		स्थिती BOX_INLINE_FLEX:
		स्थिती BOX_INLINE_BLOCK:
		स्थिती BOX_FLOAT_डावा:
		स्थिती BOX_FLOAT_उजवा:
		स्थिती BOX_BR:
		स्थिती BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			प्रतिज्ञा(0);
			थांबा;
		पूर्वनिर्धारित:
			प्रतिज्ञा(0);
		}

		जर (calculate_table_row(col_info, cell->columns, cell->rows,
				&cell->start_column, cell) == false)
			परत false;
	}


	/* Update row spanning details for all columns */
	साठी (i = 0; i < col_info->अंक_columns; i++) {
		जर (col_info->spans[i].row_span != 0 &&
				col_info->spans[i].auto_row == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			col_info->spans[i].row_span--;
		}
	}

	/* Reset current column for next row */
	col_info->current_column = 0;

	/* Increment row counter */
	col_info->अंक_rows++;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", row);
#endif

	परत true;
}


स्थिर bool
box_normalise_table_row_group(संरचना box *row_group,
			      स्थिरांक संरचना box *मूळ,
			      संरचना columns *col_info,
			      html_content * c)
{
	संरचना box *child;
	संरचना box *next_child;
	संरचना box *row;
	css_computed_style *style;
	nscss_select_ctx ctx;
	अचिन्हित पूर्णांक group_row_मोजणी = 0;

	प्रतिज्ञा(row_group != 0);
	प्रतिज्ञा(row_group->type == BOX_TABLE_ROW_GROUP);

	ctx.मूळ_style = मूळ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", row_group);
#endif

	साठी (child = row_group->children; child != रिक्त; child = next_child) {
		next_child = child->next;

		निवड (child->type) {
		स्थिती BOX_TABLE_ROW:
			/* ok */
			group_row_मोजणी++;
			जर (box_normalise_table_row(child, मूळ, col_info,
					c) == false)
				परत false;
			थांबा;
		स्थिती BOX_FLEX:
		स्थिती BOX_BLOCK:
		स्थिती BOX_INLINE_CONTAINER:
		स्थिती BOX_TABLE:
		स्थिती BOX_TABLE_ROW_GROUP:
		स्थिती BOX_TABLE_CELL:
			/* insert implied table row */
			प्रतिज्ञा(row_group->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row_group->style);
			जर (style == रिक्त)
				परत false;

			row = box_create(रिक्त, style, true, row_group->href,
					row_group->target, रिक्त, रिक्त, c->bctx);
			जर (row == रिक्त) {
				css_computed_style_destroy(style);
				परत false;
			}
			row->type = BOX_TABLE_ROW;

			जर (child->prev == रिक्त)
				row_group->children = row;
			अन्यथा
				child->prev->next = row;

			row->prev = child->prev;

			पर्यंत (child != रिक्त && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row, child);

				next_child = child->next;
				child->next = रिक्त;
				child = next_child;
			}

			प्रतिज्ञा(row->last != रिक्त);

			row->last->next = रिक्त;
			row->next = next_child = child;
			जर (row->next != रिक्त)
				row->next->prev = row;
			अन्यथा
				row_group->last = row;
			row->parent = row_group;

			group_row_मोजणी++;
			जर (box_normalise_table_row(row, मूळ, col_info,
					c) == false)
				परत false;
			थांबा;
		स्थिती BOX_INLINE:
		स्थिती BOX_INLINE_END:
		स्थिती BOX_INLINE_FLEX:
		स्थिती BOX_INLINE_BLOCK:
		स्थिती BOX_FLOAT_डावा:
		स्थिती BOX_FLOAT_उजवा:
		स्थिती BOX_BR:
		स्थिती BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			प्रतिज्ञा(0);
			थांबा;
		पूर्वनिर्धारित:
			प्रतिज्ञा(0);
		}
	}

	जर (row_group->children == रिक्त) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#endif

		प्रतिज्ञा(row_group->style != रिक्त);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		जर (style == रिक्त) {
			परत false;
		}

		row = box_create(रिक्त, style, true, row_group->href,
				row_group->target, रिक्त, रिक्त, c->bctx);
		जर (row == रिक्त) {
			css_computed_style_destroy(style);
			परत false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		group_row_मोजणी = 1;

		/* Keep table's row count in sync */
		col_info->अंक_rows++;
	}

	row_group->rows = group_row_मोजणी;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", row_group);
#endif

	परत true;
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
स्थिर bool
box_normalise_table_spans(संरचना box *table,
			  स्थिरांक संरचना box *मूळ,
			  संरचना span_info *spans,
			  html_content *c)
{
	संरचना box *table_row_group;
	संरचना box *table_row;
	संरचना box *table_cell;
	अचिन्हित पूर्णांक rows_डावा = table->rows;
	अचिन्हित पूर्णांक group_rows_डावा;
	अचिन्हित पूर्णांक col;
	nscss_select_ctx ctx;

	ctx.मूळ_style = मूळ->style;

	/* Clear span data */
	स_रखो(spans, 0, table->columns * आकार(संरचना span_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	साठी (table_row_group = table->children;
	     table_row_group != रिक्त;
	     table_row_group = table_row_group->next) {

		group_rows_डावा = table_row_group->rows;

		साठी (table_row = table_row_group->children;
		     table_row != रिक्त;
		     table_row = table_row->next) {

			साठी (table_cell = table_row->children;
			     table_cell != रिक्त;
			     table_cell = table_cell->next) {

				/* colspan = 0 -> colspan = 1 */
				जर (table_cell->columns == 0) {
					table_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				जर (table_cell->rows == 0) {
					table_cell->rows = group_rows_डावा;
				}

				/* limit rowspans within group */
				जर (table_cell->rows > group_rows_डावा) {
					table_cell->rows = group_rows_डावा;
				}

				/* Record span information */
				साठी (col = table_cell->start_column;
						col < table_cell->start_column +
						table_cell->columns; col++) {
					spans[col].row_span = table_cell->rows;
				}
			}

			/* Reduce span count of each column */
			साठी (col = 0; col < table->columns; col++) {
				जर (spans[col].row_span == 0) {
					अचिन्हित पूर्णांक start = col;
					css_computed_style *style;
					संरचना box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					प्रतिज्ञा(table_row->style != रिक्त);

					/* Find width of gap */
					पर्यंत (col < table->columns &&
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
					जर (style == रिक्त)
						परत false;

					cell = box_create(रिक्त, style, true,
							table_row->href,
							table_row->target,
							रिक्त, रिक्त, c->bctx);
					जर (cell == रिक्त) {
						css_computed_style_destroy(
								style);
						परत false;
					}
					cell->type = BOX_TABLE_CELL;

					cell->rows = 1;
					cell->columns = col - start;
					cell->start_column = start;

					/* Find place to insert cell */
					साठी (prev = table_row->children;
							prev != रिक्त;
							prev = prev->next) {
						जर (prev->start_column +
							prev->columns ==
								start)
							थांबा;
						जर (prev->next == रिक्त)
							थांबा;
					}

					/* Insert it */
					जर (prev == रिक्त) {
						जर (table_row->children != रिक्त)
							table_row->children->
								prev = cell;
						अन्यथा
							table_row->last = cell;

						cell->next =
							table_row->children;
						table_row->children = cell;
					} अन्यथा {
						जर (prev->next != रिक्त)
							prev->next->prev = cell;
						अन्यथा
							table_row->last = cell;

						cell->next = prev->next;
						prev->next = cell;
						cell->prev = prev;
					}
					cell->parent = table_row;
				} अन्यथा {
					spans[col].row_span--;
				}
			}

			प्रतिज्ञा(rows_डावा > 0);

			rows_डावा--;
		}

		group_rows_डावा--;
	}

	परत true;
}


स्थिर bool
box_normalise_table(संरचना box *table, स्थिरांक संरचना box *मूळ, html_content * c)
{
	संरचना box *child;
	संरचना box *next_child;
	संरचना box *row_group;
	css_computed_style *style;
	संरचना columns col_info;
	nscss_select_ctx ctx;

	प्रतिज्ञा(table != रिक्त);
	प्रतिज्ञा(table->type == BOX_TABLE);

	ctx.मूळ_style = मूळ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", table);
#endif

	col_info.अंक_columns = 1;
	col_info.current_column = 0;
	col_info.spans = दो_स्मृति(2 * आकार *col_info.spans);
	जर (col_info.spans == रिक्त)
		परत false;

	col_info.spans[0].row_span = col_info.spans[1].row_span = 0;
	col_info.spans[0].auto_row = false;
	col_info.spans[1].auto_row = false;
	col_info.अंक_rows = 0;

	साठी (child = table->children; child != रिक्त; child = next_child) {
		next_child = child->next;
		निवड (child->type) {
		स्थिती BOX_TABLE_ROW_GROUP:
			/* ok */
			जर (box_normalise_table_row_group(child, मूळ,
					&col_info, c) == false) {
				मुक्त(col_info.spans);
				परत false;
			}
			थांबा;
		स्थिती BOX_FLEX:
		स्थिती BOX_BLOCK:
		स्थिती BOX_INLINE_CONTAINER:
		स्थिती BOX_TABLE:
		स्थिती BOX_TABLE_ROW:
		स्थिती BOX_TABLE_CELL:
			/* insert implied table row group */
			प्रतिज्ञा(table->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					table->style);
			जर (style == रिक्त) {
				मुक्त(col_info.spans);
				परत false;
			}

			row_group = box_create(रिक्त, style, true, table->href,
					table->target, रिक्त, रिक्त, c->bctx);
			जर (row_group == रिक्त) {
				css_computed_style_destroy(style);
				मुक्त(col_info.spans);
				परत false;
			}

			row_group->type = BOX_TABLE_ROW_GROUP;

			जर (child->prev == रिक्त)
				table->children = row_group;
			अन्यथा
				child->prev->next = row_group;

			row_group->prev = child->prev;

			पर्यंत (child != रिक्त && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row_group, child);

				next_child = child->next;
				child->next = रिक्त;
				child = next_child;
			}

			प्रतिज्ञा(row_group->last != रिक्त);

			row_group->last->next = रिक्त;
			row_group->next = next_child = child;
			जर (row_group->next != रिक्त)
				row_group->next->prev = row_group;
			अन्यथा
				table->last = row_group;
			row_group->parent = table;

			जर (box_normalise_table_row_group(row_group, मूळ,
					&col_info, c) == false) {
				मुक्त(col_info.spans);
				परत false;
			}
			थांबा;
		स्थिती BOX_INLINE:
		स्थिती BOX_INLINE_END:
		स्थिती BOX_INLINE_FLEX:
		स्थिती BOX_INLINE_BLOCK:
		स्थिती BOX_FLOAT_डावा:
		स्थिती BOX_FLOAT_उजवा:
		स्थिती BOX_BR:
		स्थिती BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			प्रतिज्ञा(0);
			थांबा;
		पूर्वनिर्धारित:
			ख_लिखो(मानक_त्रुटि, "%i\n", child->type);
			प्रतिज्ञा(0);
		}
	}

	table->columns = col_info.अंक_columns;
	table->rows = col_info.अंक_rows;

	जर (table->children == रिक्त) {
		संरचना box *row;

#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#endif

		प्रतिज्ञा(table->style != रिक्त);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				table->style);
		जर (style == रिक्त) {
			मुक्त(col_info.spans);
			परत false;
		}

		row_group = box_create(रिक्त, style, true, table->href,
				table->target, रिक्त, रिक्त, c->bctx);
		जर (row_group == रिक्त) {
			css_computed_style_destroy(style);
			मुक्त(col_info.spans);
			परत false;
		}
		row_group->type = BOX_TABLE_ROW_GROUP;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		जर (style == रिक्त) {
			box_free(row_group);
			मुक्त(col_info.spans);
			परत false;
		}

		row = box_create(रिक्त, style, true, row_group->href,
				row_group->target, रिक्त, रिक्त, c->bctx);
		जर (row == रिक्त) {
			css_computed_style_destroy(style);
			box_free(row_group);
			मुक्त(col_info.spans);
			परत false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		row_group->parent = table;
		table->children = table->last = row_group;

		table->rows = 1;
	}

	जर (box_normalise_table_spans(table, मूळ, col_info.spans, c) == false) {
		मुक्त(col_info.spans);
		परत false;
	}

	मुक्त(col_info.spans);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", table);
#endif

	परत true;
}

स्थिर bool box_normalise_flex(
		संरचना box *flex_container,
		स्थिरांक संरचना box *मूळ,
		html_content *c)
{
	संरचना box *child;
	संरचना box *next_child;
	संरचना box *implied_flex_item;
	css_computed_style *style;
	nscss_select_ctx ctx;

	प्रतिज्ञा(flex_container != रिक्त);
	प्रतिज्ञा(मूळ != रिक्त);

	ctx.मूळ_style = मूळ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#endif

	प्रतिज्ञा(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	साठी (child = flex_container->children; child != रिक्त; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				child, child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		निवड (child->type) {
		स्थिती BOX_FLEX:
			/* ok */
			जर (box_normalise_flex(child, मूळ, c) == false)
				परत false;
			थांबा;
		स्थिती BOX_BLOCK:
			/* ok */
			जर (box_normalise_block(child, मूळ, c) == false)
				परत false;
			थांबा;
		स्थिती BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			प्रतिज्ञा(flex_container->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			जर (style == रिक्त)
				परत false;

			implied_flex_item = box_create(रिक्त, style, true,
					flex_container->href,
					flex_container->target,
					रिक्त, रिक्त, c->bctx);
			जर (implied_flex_item == रिक्त) {
				css_computed_style_destroy(style);
				परत false;
			}
			implied_flex_item->type = BOX_BLOCK;

			जर (child->prev == रिक्त)
				flex_container->children = implied_flex_item;
			अन्यथा
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			पर्यंत (child != रिक्त &&
					child->type == BOX_INLINE_CONTAINER) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = रिक्त;
				child = next_child;
			}

			implied_flex_item->last->next = रिक्त;
			implied_flex_item->next = next_child = child;
			जर (implied_flex_item->next != रिक्त)
				implied_flex_item->next->prev = implied_flex_item;
			अन्यथा
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			जर (box_normalise_block(implied_flex_item,
					मूळ, c) == false)
				परत false;
			थांबा;

		स्थिती BOX_TABLE:
			जर (box_normalise_table(child, मूळ, c) == false)
				परत false;
			थांबा;
		स्थिती BOX_INLINE:
		स्थिती BOX_INLINE_END:
		स्थिती BOX_INLINE_FLEX:
		स्थिती BOX_INLINE_BLOCK:
		स्थिती BOX_FLOAT_डावा:
		स्थिती BOX_FLOAT_उजवा:
		स्थिती BOX_BR:
		स्थिती BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			प्रतिज्ञा(0);
			थांबा;
		स्थिती BOX_TABLE_ROW_GROUP:
		स्थिती BOX_TABLE_ROW:
		स्थिती BOX_TABLE_CELL:
			/* insert implied table */
			प्रतिज्ञा(flex_container->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			जर (style == रिक्त)
				परत false;

			implied_flex_item = box_create(रिक्त, style, true,
					flex_container->href,
					flex_container->target,
					रिक्त, रिक्त, c->bctx);
			जर (implied_flex_item == रिक्त) {
				css_computed_style_destroy(style);
				परत false;
			}
			implied_flex_item->type = BOX_TABLE;

			जर (child->prev == रिक्त)
				flex_container->children = implied_flex_item;
			अन्यथा
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			पर्यंत (child != रिक्त && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = रिक्त;
				child = next_child;
			}

			implied_flex_item->last->next = रिक्त;
			implied_flex_item->next = next_child = child;
			जर (implied_flex_item->next != रिक्त)
				implied_flex_item->next->prev = implied_flex_item;
			अन्यथा
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			जर (box_normalise_table(implied_flex_item,
					मूळ, c) == false)
				परत false;
			थांबा;
		पूर्वनिर्धारित:
			प्रतिज्ञा(0);
		}
	}

	परत true;
}

स्थिर bool
box_normalise_inline_container(संरचना box *cont,
			       स्थिरांक संरचना box *मूळ,
			       html_content * c)
{
	संरचना box *child;
	संरचना box *next_child;

	प्रतिज्ञा(cont != रिक्त);
	प्रतिज्ञा(cont->type == BOX_INLINE_CONTAINER);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#endif

	साठी (child = cont->children; child != रिक्त; child = next_child) {
		next_child = child->next;
		निवड (child->type) {
		स्थिती BOX_INLINE:
		स्थिती BOX_INLINE_END:
		स्थिती BOX_BR:
		स्थिती BOX_TEXT:
			/* ok */
			थांबा;
		स्थिती BOX_INLINE_BLOCK:
			/* ok */
			जर (box_normalise_block(child, मूळ, c) == false)
				परत false;
			थांबा;
		स्थिती BOX_INLINE_FLEX:
			/* ok */
			जर (box_normalise_flex(child, मूळ, c) == false)
				परत false;
			थांबा;
		स्थिती BOX_FLOAT_डावा:
		स्थिती BOX_FLOAT_उजवा:
			/* ok */
			प्रतिज्ञा(child->children != रिक्त);

			निवड (child->children->type) {
			स्थिती BOX_BLOCK:
				जर (box_normalise_block(child->children, मूळ,
						c) == false)
					परत false;
				थांबा;
			स्थिती BOX_TABLE:
				जर (box_normalise_table(child->children, मूळ,
						c) == false)
					परत false;
				थांबा;
			स्थिती BOX_FLEX:
				जर (box_normalise_flex(child->children, मूळ,
						c) == false)
					परत false;
				थांबा;
			पूर्वनिर्धारित:
				प्रतिज्ञा(0);
			}

			जर (child->children == रिक्त) {
				/* the child has destroyed itself: remove float */
				जर (child->prev == रिक्त)
					child->parent->children = child->next;
				अन्यथा
					child->prev->next = child->next;
				जर (child->next != रिक्त)
					child->next->prev = child->prev;
				अन्यथा
					child->parent->last = child->prev;

				box_free(child);
			}
			थांबा;
		स्थिती BOX_FLEX:
		स्थिती BOX_BLOCK:
		स्थिती BOX_INLINE_CONTAINER:
		स्थिती BOX_TABLE:
		स्थिती BOX_TABLE_ROW_GROUP:
		स्थिती BOX_TABLE_ROW:
		स्थिती BOX_TABLE_CELL:
		पूर्वनिर्धारित:
			प्रतिज्ञा(0);
		}
	}

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#endif

	परत true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_block(संरचना box *block, स्थिरांक संरचना box *मूळ, html_content *c)
{
	संरचना box *child;
	संरचना box *next_child;
	संरचना box *table;
	css_computed_style *style;
	nscss_select_ctx ctx;

	प्रतिज्ञा(block != रिक्त);
	प्रतिज्ञा(मूळ != रिक्त);

	ctx.मूळ_style = मूळ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", block, block->type);
#endif

	प्रतिज्ञा(block->type == BOX_BLOCK || block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	साठी (child = block->children; child != रिक्त; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", child,
		      child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		निवड (child->type) {
		स्थिती BOX_FLEX:
			/* ok */
			जर (box_normalise_flex(child, मूळ, c) == false)
				परत false;
			थांबा;
		स्थिती BOX_BLOCK:
			/* ok */
			जर (box_normalise_block(child, मूळ, c) == false)
				परत false;
			थांबा;
		स्थिती BOX_INLINE_CONTAINER:
			जर (box_normalise_inline_container(child, मूळ, c) == false)
				परत false;
			थांबा;
		स्थिती BOX_TABLE:
			जर (box_normalise_table(child, मूळ, c) == false)
				परत false;
			थांबा;
		स्थिती BOX_INLINE:
		स्थिती BOX_INLINE_END:
		स्थिती BOX_INLINE_FLEX:
		स्थिती BOX_INLINE_BLOCK:
		स्थिती BOX_FLOAT_डावा:
		स्थिती BOX_FLOAT_उजवा:
		स्थिती BOX_BR:
		स्थिती BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			प्रतिज्ञा(0);
			थांबा;
		स्थिती BOX_TABLE_ROW_GROUP:
		स्थिती BOX_TABLE_ROW:
		स्थिती BOX_TABLE_CELL:
			/* insert implied table */
			प्रतिज्ञा(block->style != रिक्त);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					block->style);
			जर (style == रिक्त)
				परत false;

			table = box_create(रिक्त, style, true, block->href,
					block->target, रिक्त, रिक्त, c->bctx);
			जर (table == रिक्त) {
				css_computed_style_destroy(style);
				परत false;
			}
			table->type = BOX_TABLE;

			जर (child->prev == रिक्त)
				block->children = table;
			अन्यथा
				child->prev->next = table;

			table->prev = child->prev;

			पर्यंत (child != रिक्त && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(table, child);

				next_child = child->next;
				child->next = रिक्त;
				child = next_child;
			}

			table->last->next = रिक्त;
			table->next = next_child = child;
			जर (table->next != रिक्त)
				table->next->prev = table;
			अन्यथा
				block->last = table;
			table->parent = block;

			जर (box_normalise_table(table, मूळ, c) == false)
				परत false;
			थांबा;
		पूर्वनिर्धारित:
			प्रतिज्ञा(0);
		}
	}

	परत true;
}
