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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "css/select.h"

#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/table.h"
#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_manipulate.h"
#ଅନ୍ତର୍ଭାଵ "html/box_normalise.h"

/* Define to enable box normalise debug */
#undef BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
ସଂରଚନା span_info {
	/** Number of rows this cell spans */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା row_span;
	/** Row group of cell */
	ସଂରଚନା box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_row;
};

/**
 * Column record for a table
 */
ସଂରଚନା columns {
	/** Current column index */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା current_column;
	/** Number of columns in main part of table 1..max columns */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_columns;
	/** Information about columns in main table, array [0, num_columns) */
	ସଂରଚନା span_info *spans;
	/** Number of rows in table */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_rows;
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
ସ୍ଥିର bool
calculate_table_row(ସଂରଚନା columns *col_info,
		    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା col_span,
		    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା row_span,
		    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *start_column,
		    ସଂରଚନା box *cell)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cell_start_col = col_info->current_column;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cell_end_col;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା span_info *spans;
	ସଂରଚନା box *rg = cell->parent->parent; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	ଯେତେବେଳେ (col_info->spans[cell_start_col].row_span != 0 &&
			col_info->spans[cell_start_col].rg == rg) {
		cell_start_col++;
	}

	/* Update current column with calculated start */
	col_info->current_column = cell_start_col;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	ଯଦି (col_span == 0)
		col_span = 1;

	cell_end_col = cell_start_col + col_span;

	ଯଦି (col_info->ଅଙ୍କ_columns < cell_end_col) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(col_info->spans,
				ଆକାର *spans * (cell_end_col + 1));
		ଯଦି (spans == ଖାଲି)
			ଫେରନ୍ତୁ false;

		col_info->spans = spans;
		col_info->ଅଙ୍କ_columns = cell_end_col;

		/* Mark new final column as sentinel */
		col_info->spans[cell_end_col].row_span = 0;
		col_info->spans[cell_end_col].auto_row = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	ପାଇଁ (i = cell_start_col; i < cell_end_col; i++) {
		col_info->spans[i].row_span = (row_span == 0) ? 1 : row_span;
		col_info->spans[i].auto_row = (row_span == 0);
		col_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	col_info->current_column = cell_end_col;

	*start_column = cell_start_col;

	ଫେରନ୍ତୁ true;
}


ସ୍ଥିର bool
box_normalise_table_row(ସଂରଚନା box *row,
			ଅଚଳ ସଂରଚନା box *ମୂଳ,
			ସଂରଚନା columns *col_info,
			html_content * c)
{
	ସଂରଚନା box *child;
	ସଂରଚନା box *next_child;
	ସଂରଚନା box *cell = ଖାଲି;
	css_computed_style *style;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	nscss_select_ctx ctx;

	ପ୍ରତିଜ୍ଞା(row != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(row->type == BOX_TABLE_ROW);

	ctx.ମୂଳ_style = ମୂଳ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", row);
#endif

	ପାଇଁ (child = row->children; child != ଖାଲି; child = next_child) {
		next_child = child->next;

		ବାଛ (child->type) {
		ସ୍ଥିତି BOX_TABLE_CELL:
			/* ok */
			ଯଦି (box_normalise_block(child, ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			cell = child;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_FLEX:
		ସ୍ଥିତି BOX_BLOCK:
		ସ୍ଥିତି BOX_INLINE_CONTAINER:
		ସ୍ଥିତି BOX_TABLE:
		ସ୍ଥିତି BOX_TABLE_ROW_GROUP:
		ସ୍ଥିତି BOX_TABLE_ROW:
			/* insert implied table cell */
			ପ୍ରତିଜ୍ଞା(row->style != ଖାଲି);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row->style);
			ଯଦି (style == ଖାଲି)
				ଫେରନ୍ତୁ false;

			cell = box_create(ଖାଲି, style, true, row->href,
					row->target, ଖାଲି, ଖାଲି, c->bctx);
			ଯଦି (cell == ଖାଲି) {
				css_computed_style_destroy(style);
				ଫେରନ୍ତୁ false;
			}
			cell->type = BOX_TABLE_CELL;

			ଯଦି (child->prev == ଖାଲି)
				row->children = cell;
			ଅନ୍ୟଥା
				child->prev->next = cell;

			cell->prev = child->prev;

			ଯେତେବେଳେ (child != ଖାଲି && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW)) {
				box_add_child(cell, child);

				next_child = child->next;
				child->next = ଖାଲି;
				child = next_child;
			}

			ପ୍ରତିଜ୍ଞା(cell->last != ଖାଲି);

			cell->last->next = ଖାଲି;
			cell->next = next_child = child;
			ଯଦି (cell->next != ଖାଲି)
				cell->next->prev = cell;
			ଅନ୍ୟଥା
				row->last = cell;
			cell->parent = row;

			ଯଦି (box_normalise_block(cell, ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_INLINE:
		ସ୍ଥିତି BOX_INLINE_END:
		ସ୍ଥିତି BOX_INLINE_FLEX:
		ସ୍ଥିତି BOX_INLINE_BLOCK:
		ସ୍ଥିତି BOX_FLOAT_ବାମ:
		ସ୍ଥିତି BOX_FLOAT_ଡାହାଣ:
		ସ୍ଥିତି BOX_BR:
		ସ୍ଥିତି BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ପ୍ରତିଜ୍ଞା(0);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ପ୍ରତିଜ୍ଞା(0);
		}

		ଯଦି (calculate_table_row(col_info, cell->columns, cell->rows,
				&cell->start_column, cell) == false)
			ଫେରନ୍ତୁ false;
	}


	/* Update row spanning details for all columns */
	ପାଇଁ (i = 0; i < col_info->ଅଙ୍କ_columns; i++) {
		ଯଦି (col_info->spans[i].row_span != 0 &&
				col_info->spans[i].auto_row == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			col_info->spans[i].row_span--;
		}
	}

	/* Reset current column for next row */
	col_info->current_column = 0;

	/* Increment row counter */
	col_info->ଅଙ୍କ_rows++;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", row);
#endif

	ଫେରନ୍ତୁ true;
}


ସ୍ଥିର bool
box_normalise_table_row_group(ସଂରଚନା box *row_group,
			      ଅଚଳ ସଂରଚନା box *ମୂଳ,
			      ସଂରଚନା columns *col_info,
			      html_content * c)
{
	ସଂରଚନା box *child;
	ସଂରଚନା box *next_child;
	ସଂରଚନା box *row;
	css_computed_style *style;
	nscss_select_ctx ctx;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା group_row_ଗଣନା = 0;

	ପ୍ରତିଜ୍ଞା(row_group != 0);
	ପ୍ରତିଜ୍ଞା(row_group->type == BOX_TABLE_ROW_GROUP);

	ctx.ମୂଳ_style = ମୂଳ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", row_group);
#endif

	ପାଇଁ (child = row_group->children; child != ଖାଲି; child = next_child) {
		next_child = child->next;

		ବାଛ (child->type) {
		ସ୍ଥିତି BOX_TABLE_ROW:
			/* ok */
			group_row_ଗଣନା++;
			ଯଦି (box_normalise_table_row(child, ମୂଳ, col_info,
					c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_FLEX:
		ସ୍ଥିତି BOX_BLOCK:
		ସ୍ଥିତି BOX_INLINE_CONTAINER:
		ସ୍ଥିତି BOX_TABLE:
		ସ୍ଥିତି BOX_TABLE_ROW_GROUP:
		ସ୍ଥିତି BOX_TABLE_CELL:
			/* insert implied table row */
			ପ୍ରତିଜ୍ଞା(row_group->style != ଖାଲି);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row_group->style);
			ଯଦି (style == ଖାଲି)
				ଫେରନ୍ତୁ false;

			row = box_create(ଖାଲି, style, true, row_group->href,
					row_group->target, ଖାଲି, ଖାଲି, c->bctx);
			ଯଦି (row == ଖାଲି) {
				css_computed_style_destroy(style);
				ଫେରନ୍ତୁ false;
			}
			row->type = BOX_TABLE_ROW;

			ଯଦି (child->prev == ଖାଲି)
				row_group->children = row;
			ଅନ୍ୟଥା
				child->prev->next = row;

			row->prev = child->prev;

			ଯେତେବେଳେ (child != ଖାଲି && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row, child);

				next_child = child->next;
				child->next = ଖାଲି;
				child = next_child;
			}

			ପ୍ରତିଜ୍ଞା(row->last != ଖାଲି);

			row->last->next = ଖାଲି;
			row->next = next_child = child;
			ଯଦି (row->next != ଖାଲି)
				row->next->prev = row;
			ଅନ୍ୟଥା
				row_group->last = row;
			row->parent = row_group;

			group_row_ଗଣନା++;
			ଯଦି (box_normalise_table_row(row, ମୂଳ, col_info,
					c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_INLINE:
		ସ୍ଥିତି BOX_INLINE_END:
		ସ୍ଥିତି BOX_INLINE_FLEX:
		ସ୍ଥିତି BOX_INLINE_BLOCK:
		ସ୍ଥିତି BOX_FLOAT_ବାମ:
		ସ୍ଥିତି BOX_FLOAT_ଡାହାଣ:
		ସ୍ଥିତି BOX_BR:
		ସ୍ଥିତି BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ପ୍ରତିଜ୍ଞା(0);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ପ୍ରତିଜ୍ଞା(0);
		}
	}

	ଯଦି (row_group->children == ଖାଲି) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#endif

		ପ୍ରତିଜ୍ଞା(row_group->style != ଖାଲି);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		ଯଦି (style == ଖାଲି) {
			ଫେରନ୍ତୁ false;
		}

		row = box_create(ଖାଲି, style, true, row_group->href,
				row_group->target, ଖାଲି, ଖାଲି, c->bctx);
		ଯଦି (row == ଖାଲି) {
			css_computed_style_destroy(style);
			ଫେରନ୍ତୁ false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		group_row_ଗଣନା = 1;

		/* Keep table's row count in sync */
		col_info->ଅଙ୍କ_rows++;
	}

	row_group->rows = group_row_ଗଣନା;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", row_group);
#endif

	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର bool
box_normalise_table_spans(ସଂରଚନା box *table,
			  ଅଚଳ ସଂରଚନା box *ମୂଳ,
			  ସଂରଚନା span_info *spans,
			  html_content *c)
{
	ସଂରଚନା box *table_row_group;
	ସଂରଚନା box *table_row;
	ସଂରଚନା box *table_cell;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା rows_ବାମ = table->rows;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା group_rows_ବାମ;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା col;
	nscss_select_ctx ctx;

	ctx.ମୂଳ_style = ମୂଳ->style;

	/* Clear span data */
	memset(spans, 0, table->columns * ଆକାର(ସଂରଚନା span_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	ପାଇଁ (table_row_group = table->children;
	     table_row_group != ଖାଲି;
	     table_row_group = table_row_group->next) {

		group_rows_ବାମ = table_row_group->rows;

		ପାଇଁ (table_row = table_row_group->children;
		     table_row != ଖାଲି;
		     table_row = table_row->next) {

			ପାଇଁ (table_cell = table_row->children;
			     table_cell != ଖାଲି;
			     table_cell = table_cell->next) {

				/* colspan = 0 -> colspan = 1 */
				ଯଦି (table_cell->columns == 0) {
					table_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				ଯଦି (table_cell->rows == 0) {
					table_cell->rows = group_rows_ବାମ;
				}

				/* limit rowspans within group */
				ଯଦି (table_cell->rows > group_rows_ବାମ) {
					table_cell->rows = group_rows_ବାମ;
				}

				/* Record span information */
				ପାଇଁ (col = table_cell->start_column;
						col < table_cell->start_column +
						table_cell->columns; col++) {
					spans[col].row_span = table_cell->rows;
				}
			}

			/* Reduce span count of each column */
			ପାଇଁ (col = 0; col < table->columns; col++) {
				ଯଦି (spans[col].row_span == 0) {
					unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା start = col;
					css_computed_style *style;
					ସଂରଚନା box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					ପ୍ରତିଜ୍ଞା(table_row->style != ଖାଲି);

					/* Find width of gap */
					ଯେତେବେଳେ (col < table->columns &&
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
					ଯଦି (style == ଖାଲି)
						ଫେରନ୍ତୁ false;

					cell = box_create(ଖାଲି, style, true,
							table_row->href,
							table_row->target,
							ଖାଲି, ଖାଲି, c->bctx);
					ଯଦି (cell == ଖାଲି) {
						css_computed_style_destroy(
								style);
						ଫେରନ୍ତୁ false;
					}
					cell->type = BOX_TABLE_CELL;

					cell->rows = 1;
					cell->columns = col - start;
					cell->start_column = start;

					/* Find place to insert cell */
					ପାଇଁ (prev = table_row->children;
							prev != ଖାଲି;
							prev = prev->next) {
						ଯଦି (prev->start_column +
							prev->columns ==
								start)
							ଭାଙ୍ଗ;
						ଯଦି (prev->next == ଖାଲି)
							ଭାଙ୍ଗ;
					}

					/* Insert it */
					ଯଦି (prev == ଖାଲି) {
						ଯଦି (table_row->children != ଖାଲି)
							table_row->children->
								prev = cell;
						ଅନ୍ୟଥା
							table_row->last = cell;

						cell->next =
							table_row->children;
						table_row->children = cell;
					} ଅନ୍ୟଥା {
						ଯଦି (prev->next != ଖାଲି)
							prev->next->prev = cell;
						ଅନ୍ୟଥା
							table_row->last = cell;

						cell->next = prev->next;
						prev->next = cell;
						cell->prev = prev;
					}
					cell->parent = table_row;
				} ଅନ୍ୟଥା {
					spans[col].row_span--;
				}
			}

			ପ୍ରତିଜ୍ଞା(rows_ବାମ > 0);

			rows_ବାମ--;
		}

		group_rows_ବାମ--;
	}

	ଫେରନ୍ତୁ true;
}


ସ୍ଥିର bool
box_normalise_table(ସଂରଚନା box *table, ଅଚଳ ସଂରଚନା box *ମୂଳ, html_content * c)
{
	ସଂରଚନା box *child;
	ସଂରଚନା box *next_child;
	ସଂରଚନା box *row_group;
	css_computed_style *style;
	ସଂରଚନା columns col_info;
	nscss_select_ctx ctx;

	ପ୍ରତିଜ୍ଞା(table != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(table->type == BOX_TABLE);

	ctx.ମୂଳ_style = ମୂଳ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", table);
#endif

	col_info.ଅଙ୍କ_columns = 1;
	col_info.current_column = 0;
	col_info.spans = malloc(2 * ଆକାର *col_info.spans);
	ଯଦି (col_info.spans == ଖାଲି)
		ଫେରନ୍ତୁ false;

	col_info.spans[0].row_span = col_info.spans[1].row_span = 0;
	col_info.spans[0].auto_row = false;
	col_info.spans[1].auto_row = false;
	col_info.ଅଙ୍କ_rows = 0;

	ପାଇଁ (child = table->children; child != ଖାଲି; child = next_child) {
		next_child = child->next;
		ବାଛ (child->type) {
		ସ୍ଥିତି BOX_TABLE_ROW_GROUP:
			/* ok */
			ଯଦି (box_normalise_table_row_group(child, ମୂଳ,
					&col_info, c) == false) {
				ମୁକ୍ତ(col_info.spans);
				ଫେରନ୍ତୁ false;
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_FLEX:
		ସ୍ଥିତି BOX_BLOCK:
		ସ୍ଥିତି BOX_INLINE_CONTAINER:
		ସ୍ଥିତି BOX_TABLE:
		ସ୍ଥିତି BOX_TABLE_ROW:
		ସ୍ଥିତି BOX_TABLE_CELL:
			/* insert implied table row group */
			ପ୍ରତିଜ୍ଞା(table->style != ଖାଲି);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					table->style);
			ଯଦି (style == ଖାଲି) {
				ମୁକ୍ତ(col_info.spans);
				ଫେରନ୍ତୁ false;
			}

			row_group = box_create(ଖାଲି, style, true, table->href,
					table->target, ଖାଲି, ଖାଲି, c->bctx);
			ଯଦି (row_group == ଖାଲି) {
				css_computed_style_destroy(style);
				ମୁକ୍ତ(col_info.spans);
				ଫେରନ୍ତୁ false;
			}

			row_group->type = BOX_TABLE_ROW_GROUP;

			ଯଦି (child->prev == ଖାଲି)
				table->children = row_group;
			ଅନ୍ୟଥା
				child->prev->next = row_group;

			row_group->prev = child->prev;

			ଯେତେବେଳେ (child != ଖାଲି && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row_group, child);

				next_child = child->next;
				child->next = ଖାଲି;
				child = next_child;
			}

			ପ୍ରତିଜ୍ଞା(row_group->last != ଖାଲି);

			row_group->last->next = ଖାଲି;
			row_group->next = next_child = child;
			ଯଦି (row_group->next != ଖାଲି)
				row_group->next->prev = row_group;
			ଅନ୍ୟଥା
				table->last = row_group;
			row_group->parent = table;

			ଯଦି (box_normalise_table_row_group(row_group, ମୂଳ,
					&col_info, c) == false) {
				ମୁକ୍ତ(col_info.spans);
				ଫେରନ୍ତୁ false;
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_INLINE:
		ସ୍ଥିତି BOX_INLINE_END:
		ସ୍ଥିତି BOX_INLINE_FLEX:
		ସ୍ଥିତି BOX_INLINE_BLOCK:
		ସ୍ଥିତି BOX_FLOAT_ବାମ:
		ସ୍ଥିତି BOX_FLOAT_ଡାହାଣ:
		ସ୍ଥିତି BOX_BR:
		ସ୍ଥିତି BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ପ୍ରତିଜ୍ଞା(0);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			fprintf(stderr, "%i\n", child->type);
			ପ୍ରତିଜ୍ଞା(0);
		}
	}

	table->columns = col_info.ଅଙ୍କ_columns;
	table->rows = col_info.ଅଙ୍କ_rows;

	ଯଦି (table->children == ଖାଲି) {
		ସଂରଚନା box *row;

#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#endif

		ପ୍ରତିଜ୍ଞା(table->style != ଖାଲି);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				table->style);
		ଯଦି (style == ଖାଲି) {
			ମୁକ୍ତ(col_info.spans);
			ଫେରନ୍ତୁ false;
		}

		row_group = box_create(ଖାଲି, style, true, table->href,
				table->target, ଖାଲି, ଖାଲି, c->bctx);
		ଯଦି (row_group == ଖାଲି) {
			css_computed_style_destroy(style);
			ମୁକ୍ତ(col_info.spans);
			ଫେରନ୍ତୁ false;
		}
		row_group->type = BOX_TABLE_ROW_GROUP;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		ଯଦି (style == ଖାଲି) {
			box_free(row_group);
			ମୁକ୍ତ(col_info.spans);
			ଫେରନ୍ତୁ false;
		}

		row = box_create(ଖାଲି, style, true, row_group->href,
				row_group->target, ଖାଲି, ଖାଲି, c->bctx);
		ଯଦି (row == ଖାଲି) {
			css_computed_style_destroy(style);
			box_free(row_group);
			ମୁକ୍ତ(col_info.spans);
			ଫେରନ୍ତୁ false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		row_group->parent = table;
		table->children = table->last = row_group;

		table->rows = 1;
	}

	ଯଦି (box_normalise_table_spans(table, ମୂଳ, col_info.spans, c) == false) {
		ମୁକ୍ତ(col_info.spans);
		ଫେରନ୍ତୁ false;
	}

	ମୁକ୍ତ(col_info.spans);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", table);
#endif

	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର bool box_normalise_flex(
		ସଂରଚନା box *flex_container,
		ଅଚଳ ସଂରଚନା box *ମୂଳ,
		html_content *c)
{
	ସଂରଚନା box *child;
	ସଂରଚନା box *next_child;
	ସଂରଚନା box *implied_flex_item;
	css_computed_style *style;
	nscss_select_ctx ctx;

	ପ୍ରତିଜ୍ଞା(flex_container != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(ମୂଳ != ଖାଲି);

	ctx.ମୂଳ_style = ମୂଳ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#endif

	ପ୍ରତିଜ୍ଞା(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	ପାଇଁ (child = flex_container->children; child != ଖାଲି; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				child, child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		ବାଛ (child->type) {
		ସ୍ଥିତି BOX_FLEX:
			/* ok */
			ଯଦି (box_normalise_flex(child, ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_BLOCK:
			/* ok */
			ଯଦି (box_normalise_block(child, ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			ପ୍ରତିଜ୍ଞା(flex_container->style != ଖାଲି);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			ଯଦି (style == ଖାଲି)
				ଫେରନ୍ତୁ false;

			implied_flex_item = box_create(ଖାଲି, style, true,
					flex_container->href,
					flex_container->target,
					ଖାଲି, ଖାଲି, c->bctx);
			ଯଦି (implied_flex_item == ଖାଲି) {
				css_computed_style_destroy(style);
				ଫେରନ୍ତୁ false;
			}
			implied_flex_item->type = BOX_BLOCK;

			ଯଦି (child->prev == ଖାଲି)
				flex_container->children = implied_flex_item;
			ଅନ୍ୟଥା
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			ଯେତେବେଳେ (child != ଖାଲି &&
					child->type == BOX_INLINE_CONTAINER) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = ଖାଲି;
				child = next_child;
			}

			implied_flex_item->last->next = ଖାଲି;
			implied_flex_item->next = next_child = child;
			ଯଦି (implied_flex_item->next != ଖାଲି)
				implied_flex_item->next->prev = implied_flex_item;
			ଅନ୍ୟଥା
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			ଯଦି (box_normalise_block(implied_flex_item,
					ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି BOX_TABLE:
			ଯଦି (box_normalise_table(child, ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_INLINE:
		ସ୍ଥିତି BOX_INLINE_END:
		ସ୍ଥିତି BOX_INLINE_FLEX:
		ସ୍ଥିତି BOX_INLINE_BLOCK:
		ସ୍ଥିତି BOX_FLOAT_ବାମ:
		ସ୍ଥିତି BOX_FLOAT_ଡାହାଣ:
		ସ୍ଥିତି BOX_BR:
		ସ୍ଥିତି BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ପ୍ରତିଜ୍ଞା(0);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_TABLE_ROW_GROUP:
		ସ୍ଥିତି BOX_TABLE_ROW:
		ସ୍ଥିତି BOX_TABLE_CELL:
			/* insert implied table */
			ପ୍ରତିଜ୍ଞା(flex_container->style != ଖାଲି);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			ଯଦି (style == ଖାଲି)
				ଫେରନ୍ତୁ false;

			implied_flex_item = box_create(ଖାଲି, style, true,
					flex_container->href,
					flex_container->target,
					ଖାଲି, ଖାଲି, c->bctx);
			ଯଦି (implied_flex_item == ଖାଲି) {
				css_computed_style_destroy(style);
				ଫେରନ୍ତୁ false;
			}
			implied_flex_item->type = BOX_TABLE;

			ଯଦି (child->prev == ଖାଲି)
				flex_container->children = implied_flex_item;
			ଅନ୍ୟଥା
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			ଯେତେବେଳେ (child != ଖାଲି && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = ଖାଲି;
				child = next_child;
			}

			implied_flex_item->last->next = ଖାଲି;
			implied_flex_item->next = next_child = child;
			ଯଦି (implied_flex_item->next != ଖାଲି)
				implied_flex_item->next->prev = implied_flex_item;
			ଅନ୍ୟଥା
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			ଯଦି (box_normalise_table(implied_flex_item,
					ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ପ୍ରତିଜ୍ଞା(0);
		}
	}

	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର bool
box_normalise_inline_container(ସଂରଚନା box *cont,
			       ଅଚଳ ସଂରଚନା box *ମୂଳ,
			       html_content * c)
{
	ସଂରଚନା box *child;
	ସଂରଚନା box *next_child;

	ପ୍ରତିଜ୍ଞା(cont != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(cont->type == BOX_INLINE_CONTAINER);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#endif

	ପାଇଁ (child = cont->children; child != ଖାଲି; child = next_child) {
		next_child = child->next;
		ବାଛ (child->type) {
		ସ୍ଥିତି BOX_INLINE:
		ସ୍ଥିତି BOX_INLINE_END:
		ସ୍ଥିତି BOX_BR:
		ସ୍ଥିତି BOX_TEXT:
			/* ok */
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_INLINE_BLOCK:
			/* ok */
			ଯଦି (box_normalise_block(child, ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_INLINE_FLEX:
			/* ok */
			ଯଦି (box_normalise_flex(child, ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_FLOAT_ବାମ:
		ସ୍ଥିତି BOX_FLOAT_ଡାହାଣ:
			/* ok */
			ପ୍ରତିଜ୍ଞା(child->children != ଖାଲି);

			ବାଛ (child->children->type) {
			ସ୍ଥିତି BOX_BLOCK:
				ଯଦି (box_normalise_block(child->children, ମୂଳ,
						c) == false)
					ଫେରନ୍ତୁ false;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି BOX_TABLE:
				ଯଦି (box_normalise_table(child->children, ମୂଳ,
						c) == false)
					ଫେରନ୍ତୁ false;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି BOX_FLEX:
				ଯଦି (box_normalise_flex(child->children, ମୂଳ,
						c) == false)
					ଫେରନ୍ତୁ false;
				ଭାଙ୍ଗ;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				ପ୍ରତିଜ୍ଞା(0);
			}

			ଯଦି (child->children == ଖାଲି) {
				/* the child has destroyed itself: remove float */
				ଯଦି (child->prev == ଖାଲି)
					child->parent->children = child->next;
				ଅନ୍ୟଥା
					child->prev->next = child->next;
				ଯଦି (child->next != ଖାଲି)
					child->next->prev = child->prev;
				ଅନ୍ୟଥା
					child->parent->last = child->prev;

				box_free(child);
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_FLEX:
		ସ୍ଥିତି BOX_BLOCK:
		ସ୍ଥିତି BOX_INLINE_CONTAINER:
		ସ୍ଥିତି BOX_TABLE:
		ସ୍ଥିତି BOX_TABLE_ROW_GROUP:
		ସ୍ଥିତି BOX_TABLE_ROW:
		ସ୍ଥିତି BOX_TABLE_CELL:
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ପ୍ରତିଜ୍ଞା(0);
		}
	}

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#endif

	ଫେରନ୍ତୁ true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_block(ସଂରଚନା box *block, ଅଚଳ ସଂରଚନା box *ମୂଳ, html_content *c)
{
	ସଂରଚନା box *child;
	ସଂରଚନା box *next_child;
	ସଂରଚନା box *table;
	css_computed_style *style;
	nscss_select_ctx ctx;

	ପ୍ରତିଜ୍ଞା(block != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(ମୂଳ != ଖାଲି);

	ctx.ମୂଳ_style = ମୂଳ->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", block, block->type);
#endif

	ପ୍ରତିଜ୍ଞା(block->type == BOX_BLOCK || block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	ପାଇଁ (child = block->children; child != ଖାଲି; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", child,
		      child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		ବାଛ (child->type) {
		ସ୍ଥିତି BOX_FLEX:
			/* ok */
			ଯଦି (box_normalise_flex(child, ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_BLOCK:
			/* ok */
			ଯଦି (box_normalise_block(child, ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_INLINE_CONTAINER:
			ଯଦି (box_normalise_inline_container(child, ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_TABLE:
			ଯଦି (box_normalise_table(child, ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_INLINE:
		ସ୍ଥିତି BOX_INLINE_END:
		ସ୍ଥିତି BOX_INLINE_FLEX:
		ସ୍ଥିତି BOX_INLINE_BLOCK:
		ସ୍ଥିତି BOX_FLOAT_ବାମ:
		ସ୍ଥିତି BOX_FLOAT_ଡାହାଣ:
		ସ୍ଥିତି BOX_BR:
		ସ୍ଥିତି BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ପ୍ରତିଜ୍ଞା(0);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BOX_TABLE_ROW_GROUP:
		ସ୍ଥିତି BOX_TABLE_ROW:
		ସ୍ଥିତି BOX_TABLE_CELL:
			/* insert implied table */
			ପ୍ରତିଜ୍ଞା(block->style != ଖାଲି);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					block->style);
			ଯଦି (style == ଖାଲି)
				ଫେରନ୍ତୁ false;

			table = box_create(ଖାଲି, style, true, block->href,
					block->target, ଖାଲି, ଖାଲି, c->bctx);
			ଯଦି (table == ଖାଲି) {
				css_computed_style_destroy(style);
				ଫେରନ୍ତୁ false;
			}
			table->type = BOX_TABLE;

			ଯଦି (child->prev == ଖାଲି)
				block->children = table;
			ଅନ୍ୟଥା
				child->prev->next = table;

			table->prev = child->prev;

			ଯେତେବେଳେ (child != ଖାଲି && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(table, child);

				next_child = child->next;
				child->next = ଖାଲି;
				child = next_child;
			}

			table->last->next = ଖାଲି;
			table->next = next_child = child;
			ଯଦି (table->next != ଖାଲି)
				table->next->prev = table;
			ଅନ୍ୟଥା
				block->last = table;
			table->parent = block;

			ଯଦି (box_normalise_table(table, ମୂଳ, c) == false)
				ଫେରନ୍ତୁ false;
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ପ୍ରତିଜ୍ଞା(0);
		}
	}

	ଫେରନ୍ତୁ true;
}
