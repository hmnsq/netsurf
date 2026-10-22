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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "css/select.h"

#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/table.h"
#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_manipulate.h"
#அந்தர்பாவ "html/box_normalise.h"

/* Define to enable box normalise debug */
#undef BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
கட்டமைப்பு span_info {
	/** Number of rows this cell spans */
	unsigned முழு row_span;
	/** Row group of cell */
	கட்டமைப்பு box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_row;
};

/**
 * Column record for a table
 */
கட்டமைப்பு columns {
	/** Current column index */
	unsigned முழு current_column;
	/** Number of columns in main part of table 1..max columns */
	unsigned முழு இலக்கம்_columns;
	/** Information about columns in main table, array [0, num_columns) */
	கட்டமைப்பு span_info *spans;
	/** Number of rows in table */
	unsigned முழு இலக்கம்_rows;
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
நிலையான bool
calculate_table_row(கட்டமைப்பு columns *col_info,
		    unsigned முழு col_span,
		    unsigned முழு row_span,
		    unsigned முழு *start_column,
		    கட்டமைப்பு box *cell)
{
	unsigned முழு cell_start_col = col_info->current_column;
	unsigned முழு cell_end_col;
	unsigned முழு i;
	கட்டமைப்பு span_info *spans;
	கட்டமைப்பு box *rg = cell->parent->parent; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	போது (col_info->spans[cell_start_col].row_span != 0 &&
			col_info->spans[cell_start_col].rg == rg) {
		cell_start_col++;
	}

	/* Update current column with calculated start */
	col_info->current_column = cell_start_col;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	எனில் (col_span == 0)
		col_span = 1;

	cell_end_col = cell_start_col + col_span;

	எனில் (col_info->இலக்கம்_columns < cell_end_col) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(col_info->spans,
				அளவு *spans * (cell_end_col + 1));
		எனில் (spans == வெற்று)
			திரும்பு false;

		col_info->spans = spans;
		col_info->இலக்கம்_columns = cell_end_col;

		/* Mark new final column as sentinel */
		col_info->spans[cell_end_col].row_span = 0;
		col_info->spans[cell_end_col].auto_row = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	க்கு (i = cell_start_col; i < cell_end_col; i++) {
		col_info->spans[i].row_span = (row_span == 0) ? 1 : row_span;
		col_info->spans[i].auto_row = (row_span == 0);
		col_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	col_info->current_column = cell_end_col;

	*start_column = cell_start_col;

	திரும்பு true;
}


நிலையான bool
box_normalise_table_row(கட்டமைப்பு box *row,
			மாறிலி கட்டமைப்பு box *வேர்,
			கட்டமைப்பு columns *col_info,
			html_content * c)
{
	கட்டமைப்பு box *child;
	கட்டமைப்பு box *next_child;
	கட்டமைப்பு box *cell = வெற்று;
	css_computed_style *style;
	unsigned முழு i;
	nscss_select_ctx ctx;

	ப்ரதிஜ்ஞா(row != வெற்று);
	ப்ரதிஜ்ஞா(row->type == BOX_TABLE_ROW);

	ctx.வேர்_style = வேர்->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", row);
#endif

	க்கு (child = row->children; child != வெற்று; child = next_child) {
		next_child = child->next;

		தேர்வு (child->type) {
		நிலை BOX_TABLE_CELL:
			/* ok */
			எனில் (box_normalise_block(child, வேர், c) == false)
				திரும்பு false;
			cell = child;
			நிறுத்து;
		நிலை BOX_FLEX:
		நிலை BOX_BLOCK:
		நிலை BOX_INLINE_CONTAINER:
		நிலை BOX_TABLE:
		நிலை BOX_TABLE_ROW_GROUP:
		நிலை BOX_TABLE_ROW:
			/* insert implied table cell */
			ப்ரதிஜ்ஞா(row->style != வெற்று);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row->style);
			எனில் (style == வெற்று)
				திரும்பு false;

			cell = box_create(வெற்று, style, true, row->href,
					row->target, வெற்று, வெற்று, c->bctx);
			எனில் (cell == வெற்று) {
				css_computed_style_destroy(style);
				திரும்பு false;
			}
			cell->type = BOX_TABLE_CELL;

			எனில் (child->prev == வெற்று)
				row->children = cell;
			இல்லையெனில்
				child->prev->next = cell;

			cell->prev = child->prev;

			போது (child != வெற்று && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW)) {
				box_add_child(cell, child);

				next_child = child->next;
				child->next = வெற்று;
				child = next_child;
			}

			ப்ரதிஜ்ஞா(cell->last != வெற்று);

			cell->last->next = வெற்று;
			cell->next = next_child = child;
			எனில் (cell->next != வெற்று)
				cell->next->prev = cell;
			இல்லையெனில்
				row->last = cell;
			cell->parent = row;

			எனில் (box_normalise_block(cell, வேர், c) == false)
				திரும்பு false;
			நிறுத்து;
		நிலை BOX_INLINE:
		நிலை BOX_INLINE_END:
		நிலை BOX_INLINE_FLEX:
		நிலை BOX_INLINE_BLOCK:
		நிலை BOX_FLOAT_இடது:
		நிலை BOX_FLOAT_வலது:
		நிலை BOX_BR:
		நிலை BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ப்ரதிஜ்ஞா(0);
			நிறுத்து;
		இயல்பு:
			ப்ரதிஜ்ஞா(0);
		}

		எனில் (calculate_table_row(col_info, cell->columns, cell->rows,
				&cell->start_column, cell) == false)
			திரும்பு false;
	}


	/* Update row spanning details for all columns */
	க்கு (i = 0; i < col_info->இலக்கம்_columns; i++) {
		எனில் (col_info->spans[i].row_span != 0 &&
				col_info->spans[i].auto_row == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			col_info->spans[i].row_span--;
		}
	}

	/* Reset current column for next row */
	col_info->current_column = 0;

	/* Increment row counter */
	col_info->இலக்கம்_rows++;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", row);
#endif

	திரும்பு true;
}


நிலையான bool
box_normalise_table_row_group(கட்டமைப்பு box *row_group,
			      மாறிலி கட்டமைப்பு box *வேர்,
			      கட்டமைப்பு columns *col_info,
			      html_content * c)
{
	கட்டமைப்பு box *child;
	கட்டமைப்பு box *next_child;
	கட்டமைப்பு box *row;
	css_computed_style *style;
	nscss_select_ctx ctx;
	unsigned முழு group_row_மொத்தம் = 0;

	ப்ரதிஜ்ஞா(row_group != 0);
	ப்ரதிஜ்ஞா(row_group->type == BOX_TABLE_ROW_GROUP);

	ctx.வேர்_style = வேர்->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", row_group);
#endif

	க்கு (child = row_group->children; child != வெற்று; child = next_child) {
		next_child = child->next;

		தேர்வு (child->type) {
		நிலை BOX_TABLE_ROW:
			/* ok */
			group_row_மொத்தம்++;
			எனில் (box_normalise_table_row(child, வேர், col_info,
					c) == false)
				திரும்பு false;
			நிறுத்து;
		நிலை BOX_FLEX:
		நிலை BOX_BLOCK:
		நிலை BOX_INLINE_CONTAINER:
		நிலை BOX_TABLE:
		நிலை BOX_TABLE_ROW_GROUP:
		நிலை BOX_TABLE_CELL:
			/* insert implied table row */
			ப்ரதிஜ்ஞா(row_group->style != வெற்று);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row_group->style);
			எனில் (style == வெற்று)
				திரும்பு false;

			row = box_create(வெற்று, style, true, row_group->href,
					row_group->target, வெற்று, வெற்று, c->bctx);
			எனில் (row == வெற்று) {
				css_computed_style_destroy(style);
				திரும்பு false;
			}
			row->type = BOX_TABLE_ROW;

			எனில் (child->prev == வெற்று)
				row_group->children = row;
			இல்லையெனில்
				child->prev->next = row;

			row->prev = child->prev;

			போது (child != வெற்று && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row, child);

				next_child = child->next;
				child->next = வெற்று;
				child = next_child;
			}

			ப்ரதிஜ்ஞா(row->last != வெற்று);

			row->last->next = வெற்று;
			row->next = next_child = child;
			எனில் (row->next != வெற்று)
				row->next->prev = row;
			இல்லையெனில்
				row_group->last = row;
			row->parent = row_group;

			group_row_மொத்தம்++;
			எனில் (box_normalise_table_row(row, வேர், col_info,
					c) == false)
				திரும்பு false;
			நிறுத்து;
		நிலை BOX_INLINE:
		நிலை BOX_INLINE_END:
		நிலை BOX_INLINE_FLEX:
		நிலை BOX_INLINE_BLOCK:
		நிலை BOX_FLOAT_இடது:
		நிலை BOX_FLOAT_வலது:
		நிலை BOX_BR:
		நிலை BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ப்ரதிஜ்ஞா(0);
			நிறுத்து;
		இயல்பு:
			ப்ரதிஜ்ஞா(0);
		}
	}

	எனில் (row_group->children == வெற்று) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#endif

		ப்ரதிஜ்ஞா(row_group->style != வெற்று);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		எனில் (style == வெற்று) {
			திரும்பு false;
		}

		row = box_create(வெற்று, style, true, row_group->href,
				row_group->target, வெற்று, வெற்று, c->bctx);
		எனில் (row == வெற்று) {
			css_computed_style_destroy(style);
			திரும்பு false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		group_row_மொத்தம் = 1;

		/* Keep table's row count in sync */
		col_info->இலக்கம்_rows++;
	}

	row_group->rows = group_row_மொத்தம்;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", row_group);
#endif

	திரும்பு true;
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
நிலையான bool
box_normalise_table_spans(கட்டமைப்பு box *table,
			  மாறிலி கட்டமைப்பு box *வேர்,
			  கட்டமைப்பு span_info *spans,
			  html_content *c)
{
	கட்டமைப்பு box *table_row_group;
	கட்டமைப்பு box *table_row;
	கட்டமைப்பு box *table_cell;
	unsigned முழு rows_இடது = table->rows;
	unsigned முழு group_rows_இடது;
	unsigned முழு col;
	nscss_select_ctx ctx;

	ctx.வேர்_style = வேர்->style;

	/* Clear span data */
	memset(spans, 0, table->columns * அளவு(கட்டமைப்பு span_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	க்கு (table_row_group = table->children;
	     table_row_group != வெற்று;
	     table_row_group = table_row_group->next) {

		group_rows_இடது = table_row_group->rows;

		க்கு (table_row = table_row_group->children;
		     table_row != வெற்று;
		     table_row = table_row->next) {

			க்கு (table_cell = table_row->children;
			     table_cell != வெற்று;
			     table_cell = table_cell->next) {

				/* colspan = 0 -> colspan = 1 */
				எனில் (table_cell->columns == 0) {
					table_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				எனில் (table_cell->rows == 0) {
					table_cell->rows = group_rows_இடது;
				}

				/* limit rowspans within group */
				எனில் (table_cell->rows > group_rows_இடது) {
					table_cell->rows = group_rows_இடது;
				}

				/* Record span information */
				க்கு (col = table_cell->start_column;
						col < table_cell->start_column +
						table_cell->columns; col++) {
					spans[col].row_span = table_cell->rows;
				}
			}

			/* Reduce span count of each column */
			க்கு (col = 0; col < table->columns; col++) {
				எனில் (spans[col].row_span == 0) {
					unsigned முழு start = col;
					css_computed_style *style;
					கட்டமைப்பு box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					ப்ரதிஜ்ஞா(table_row->style != வெற்று);

					/* Find width of gap */
					போது (col < table->columns &&
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
					எனில் (style == வெற்று)
						திரும்பு false;

					cell = box_create(வெற்று, style, true,
							table_row->href,
							table_row->target,
							வெற்று, வெற்று, c->bctx);
					எனில் (cell == வெற்று) {
						css_computed_style_destroy(
								style);
						திரும்பு false;
					}
					cell->type = BOX_TABLE_CELL;

					cell->rows = 1;
					cell->columns = col - start;
					cell->start_column = start;

					/* Find place to insert cell */
					க்கு (prev = table_row->children;
							prev != வெற்று;
							prev = prev->next) {
						எனில் (prev->start_column +
							prev->columns ==
								start)
							நிறுத்து;
						எனில் (prev->next == வெற்று)
							நிறுத்து;
					}

					/* Insert it */
					எனில் (prev == வெற்று) {
						எனில் (table_row->children != வெற்று)
							table_row->children->
								prev = cell;
						இல்லையெனில்
							table_row->last = cell;

						cell->next =
							table_row->children;
						table_row->children = cell;
					} இல்லையெனில் {
						எனில் (prev->next != வெற்று)
							prev->next->prev = cell;
						இல்லையெனில்
							table_row->last = cell;

						cell->next = prev->next;
						prev->next = cell;
						cell->prev = prev;
					}
					cell->parent = table_row;
				} இல்லையெனில் {
					spans[col].row_span--;
				}
			}

			ப்ரதிஜ்ஞா(rows_இடது > 0);

			rows_இடது--;
		}

		group_rows_இடது--;
	}

	திரும்பு true;
}


நிலையான bool
box_normalise_table(கட்டமைப்பு box *table, மாறிலி கட்டமைப்பு box *வேர், html_content * c)
{
	கட்டமைப்பு box *child;
	கட்டமைப்பு box *next_child;
	கட்டமைப்பு box *row_group;
	css_computed_style *style;
	கட்டமைப்பு columns col_info;
	nscss_select_ctx ctx;

	ப்ரதிஜ்ஞா(table != வெற்று);
	ப்ரதிஜ்ஞா(table->type == BOX_TABLE);

	ctx.வேர்_style = வேர்->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", table);
#endif

	col_info.இலக்கம்_columns = 1;
	col_info.current_column = 0;
	col_info.spans = malloc(2 * அளவு *col_info.spans);
	எனில் (col_info.spans == வெற்று)
		திரும்பு false;

	col_info.spans[0].row_span = col_info.spans[1].row_span = 0;
	col_info.spans[0].auto_row = false;
	col_info.spans[1].auto_row = false;
	col_info.இலக்கம்_rows = 0;

	க்கு (child = table->children; child != வெற்று; child = next_child) {
		next_child = child->next;
		தேர்வு (child->type) {
		நிலை BOX_TABLE_ROW_GROUP:
			/* ok */
			எனில் (box_normalise_table_row_group(child, வேர்,
					&col_info, c) == false) {
				விடுவி(col_info.spans);
				திரும்பு false;
			}
			நிறுத்து;
		நிலை BOX_FLEX:
		நிலை BOX_BLOCK:
		நிலை BOX_INLINE_CONTAINER:
		நிலை BOX_TABLE:
		நிலை BOX_TABLE_ROW:
		நிலை BOX_TABLE_CELL:
			/* insert implied table row group */
			ப்ரதிஜ்ஞா(table->style != வெற்று);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					table->style);
			எனில் (style == வெற்று) {
				விடுவி(col_info.spans);
				திரும்பு false;
			}

			row_group = box_create(வெற்று, style, true, table->href,
					table->target, வெற்று, வெற்று, c->bctx);
			எனில் (row_group == வெற்று) {
				css_computed_style_destroy(style);
				விடுவி(col_info.spans);
				திரும்பு false;
			}

			row_group->type = BOX_TABLE_ROW_GROUP;

			எனில் (child->prev == வெற்று)
				table->children = row_group;
			இல்லையெனில்
				child->prev->next = row_group;

			row_group->prev = child->prev;

			போது (child != வெற்று && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row_group, child);

				next_child = child->next;
				child->next = வெற்று;
				child = next_child;
			}

			ப்ரதிஜ்ஞா(row_group->last != வெற்று);

			row_group->last->next = வெற்று;
			row_group->next = next_child = child;
			எனில் (row_group->next != வெற்று)
				row_group->next->prev = row_group;
			இல்லையெனில்
				table->last = row_group;
			row_group->parent = table;

			எனில் (box_normalise_table_row_group(row_group, வேர்,
					&col_info, c) == false) {
				விடுவி(col_info.spans);
				திரும்பு false;
			}
			நிறுத்து;
		நிலை BOX_INLINE:
		நிலை BOX_INLINE_END:
		நிலை BOX_INLINE_FLEX:
		நிலை BOX_INLINE_BLOCK:
		நிலை BOX_FLOAT_இடது:
		நிலை BOX_FLOAT_வலது:
		நிலை BOX_BR:
		நிலை BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ப்ரதிஜ்ஞா(0);
			நிறுத்து;
		இயல்பு:
			fprintf(stderr, "%i\n", child->type);
			ப்ரதிஜ்ஞா(0);
		}
	}

	table->columns = col_info.இலக்கம்_columns;
	table->rows = col_info.இலக்கம்_rows;

	எனில் (table->children == வெற்று) {
		கட்டமைப்பு box *row;

#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#endif

		ப்ரதிஜ்ஞா(table->style != வெற்று);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				table->style);
		எனில் (style == வெற்று) {
			விடுவி(col_info.spans);
			திரும்பு false;
		}

		row_group = box_create(வெற்று, style, true, table->href,
				table->target, வெற்று, வெற்று, c->bctx);
		எனில் (row_group == வெற்று) {
			css_computed_style_destroy(style);
			விடுவி(col_info.spans);
			திரும்பு false;
		}
		row_group->type = BOX_TABLE_ROW_GROUP;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		எனில் (style == வெற்று) {
			box_free(row_group);
			விடுவி(col_info.spans);
			திரும்பு false;
		}

		row = box_create(வெற்று, style, true, row_group->href,
				row_group->target, வெற்று, வெற்று, c->bctx);
		எனில் (row == வெற்று) {
			css_computed_style_destroy(style);
			box_free(row_group);
			விடுவி(col_info.spans);
			திரும்பு false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		row_group->parent = table;
		table->children = table->last = row_group;

		table->rows = 1;
	}

	எனில் (box_normalise_table_spans(table, வேர், col_info.spans, c) == false) {
		விடுவி(col_info.spans);
		திரும்பு false;
	}

	விடுவி(col_info.spans);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", table);
#endif

	திரும்பு true;
}

நிலையான bool box_normalise_flex(
		கட்டமைப்பு box *flex_container,
		மாறிலி கட்டமைப்பு box *வேர்,
		html_content *c)
{
	கட்டமைப்பு box *child;
	கட்டமைப்பு box *next_child;
	கட்டமைப்பு box *implied_flex_item;
	css_computed_style *style;
	nscss_select_ctx ctx;

	ப்ரதிஜ்ஞா(flex_container != வெற்று);
	ப்ரதிஜ்ஞா(வேர் != வெற்று);

	ctx.வேர்_style = வேர்->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#endif

	ப்ரதிஜ்ஞா(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	க்கு (child = flex_container->children; child != வெற்று; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				child, child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		தேர்வு (child->type) {
		நிலை BOX_FLEX:
			/* ok */
			எனில் (box_normalise_flex(child, வேர், c) == false)
				திரும்பு false;
			நிறுத்து;
		நிலை BOX_BLOCK:
			/* ok */
			எனில் (box_normalise_block(child, வேர், c) == false)
				திரும்பு false;
			நிறுத்து;
		நிலை BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			ப்ரதிஜ்ஞா(flex_container->style != வெற்று);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			எனில் (style == வெற்று)
				திரும்பு false;

			implied_flex_item = box_create(வெற்று, style, true,
					flex_container->href,
					flex_container->target,
					வெற்று, வெற்று, c->bctx);
			எனில் (implied_flex_item == வெற்று) {
				css_computed_style_destroy(style);
				திரும்பு false;
			}
			implied_flex_item->type = BOX_BLOCK;

			எனில் (child->prev == வெற்று)
				flex_container->children = implied_flex_item;
			இல்லையெனில்
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			போது (child != வெற்று &&
					child->type == BOX_INLINE_CONTAINER) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = வெற்று;
				child = next_child;
			}

			implied_flex_item->last->next = வெற்று;
			implied_flex_item->next = next_child = child;
			எனில் (implied_flex_item->next != வெற்று)
				implied_flex_item->next->prev = implied_flex_item;
			இல்லையெனில்
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			எனில் (box_normalise_block(implied_flex_item,
					வேர், c) == false)
				திரும்பு false;
			நிறுத்து;

		நிலை BOX_TABLE:
			எனில் (box_normalise_table(child, வேர், c) == false)
				திரும்பு false;
			நிறுத்து;
		நிலை BOX_INLINE:
		நிலை BOX_INLINE_END:
		நிலை BOX_INLINE_FLEX:
		நிலை BOX_INLINE_BLOCK:
		நிலை BOX_FLOAT_இடது:
		நிலை BOX_FLOAT_வலது:
		நிலை BOX_BR:
		நிலை BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ப்ரதிஜ்ஞா(0);
			நிறுத்து;
		நிலை BOX_TABLE_ROW_GROUP:
		நிலை BOX_TABLE_ROW:
		நிலை BOX_TABLE_CELL:
			/* insert implied table */
			ப்ரதிஜ்ஞா(flex_container->style != வெற்று);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			எனில் (style == வெற்று)
				திரும்பு false;

			implied_flex_item = box_create(வெற்று, style, true,
					flex_container->href,
					flex_container->target,
					வெற்று, வெற்று, c->bctx);
			எனில் (implied_flex_item == வெற்று) {
				css_computed_style_destroy(style);
				திரும்பு false;
			}
			implied_flex_item->type = BOX_TABLE;

			எனில் (child->prev == வெற்று)
				flex_container->children = implied_flex_item;
			இல்லையெனில்
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			போது (child != வெற்று && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = வெற்று;
				child = next_child;
			}

			implied_flex_item->last->next = வெற்று;
			implied_flex_item->next = next_child = child;
			எனில் (implied_flex_item->next != வெற்று)
				implied_flex_item->next->prev = implied_flex_item;
			இல்லையெனில்
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			எனில் (box_normalise_table(implied_flex_item,
					வேர், c) == false)
				திரும்பு false;
			நிறுத்து;
		இயல்பு:
			ப்ரதிஜ்ஞா(0);
		}
	}

	திரும்பு true;
}

நிலையான bool
box_normalise_inline_container(கட்டமைப்பு box *cont,
			       மாறிலி கட்டமைப்பு box *வேர்,
			       html_content * c)
{
	கட்டமைப்பு box *child;
	கட்டமைப்பு box *next_child;

	ப்ரதிஜ்ஞா(cont != வெற்று);
	ப்ரதிஜ்ஞா(cont->type == BOX_INLINE_CONTAINER);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#endif

	க்கு (child = cont->children; child != வெற்று; child = next_child) {
		next_child = child->next;
		தேர்வு (child->type) {
		நிலை BOX_INLINE:
		நிலை BOX_INLINE_END:
		நிலை BOX_BR:
		நிலை BOX_TEXT:
			/* ok */
			நிறுத்து;
		நிலை BOX_INLINE_BLOCK:
			/* ok */
			எனில் (box_normalise_block(child, வேர், c) == false)
				திரும்பு false;
			நிறுத்து;
		நிலை BOX_INLINE_FLEX:
			/* ok */
			எனில் (box_normalise_flex(child, வேர், c) == false)
				திரும்பு false;
			நிறுத்து;
		நிலை BOX_FLOAT_இடது:
		நிலை BOX_FLOAT_வலது:
			/* ok */
			ப்ரதிஜ்ஞா(child->children != வெற்று);

			தேர்வு (child->children->type) {
			நிலை BOX_BLOCK:
				எனில் (box_normalise_block(child->children, வேர்,
						c) == false)
					திரும்பு false;
				நிறுத்து;
			நிலை BOX_TABLE:
				எனில் (box_normalise_table(child->children, வேர்,
						c) == false)
					திரும்பு false;
				நிறுத்து;
			நிலை BOX_FLEX:
				எனில் (box_normalise_flex(child->children, வேர்,
						c) == false)
					திரும்பு false;
				நிறுத்து;
			இயல்பு:
				ப்ரதிஜ்ஞா(0);
			}

			எனில் (child->children == வெற்று) {
				/* the child has destroyed itself: remove float */
				எனில் (child->prev == வெற்று)
					child->parent->children = child->next;
				இல்லையெனில்
					child->prev->next = child->next;
				எனில் (child->next != வெற்று)
					child->next->prev = child->prev;
				இல்லையெனில்
					child->parent->last = child->prev;

				box_free(child);
			}
			நிறுத்து;
		நிலை BOX_FLEX:
		நிலை BOX_BLOCK:
		நிலை BOX_INLINE_CONTAINER:
		நிலை BOX_TABLE:
		நிலை BOX_TABLE_ROW_GROUP:
		நிலை BOX_TABLE_ROW:
		நிலை BOX_TABLE_CELL:
		இயல்பு:
			ப்ரதிஜ்ஞா(0);
		}
	}

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#endif

	திரும்பு true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_block(கட்டமைப்பு box *block, மாறிலி கட்டமைப்பு box *வேர், html_content *c)
{
	கட்டமைப்பு box *child;
	கட்டமைப்பு box *next_child;
	கட்டமைப்பு box *table;
	css_computed_style *style;
	nscss_select_ctx ctx;

	ப்ரதிஜ்ஞா(block != வெற்று);
	ப்ரதிஜ்ஞா(வேர் != வெற்று);

	ctx.வேர்_style = வேர்->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", block, block->type);
#endif

	ப்ரதிஜ்ஞா(block->type == BOX_BLOCK || block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	க்கு (child = block->children; child != வெற்று; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", child,
		      child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		தேர்வு (child->type) {
		நிலை BOX_FLEX:
			/* ok */
			எனில் (box_normalise_flex(child, வேர், c) == false)
				திரும்பு false;
			நிறுத்து;
		நிலை BOX_BLOCK:
			/* ok */
			எனில் (box_normalise_block(child, வேர், c) == false)
				திரும்பு false;
			நிறுத்து;
		நிலை BOX_INLINE_CONTAINER:
			எனில் (box_normalise_inline_container(child, வேர், c) == false)
				திரும்பு false;
			நிறுத்து;
		நிலை BOX_TABLE:
			எனில் (box_normalise_table(child, வேர், c) == false)
				திரும்பு false;
			நிறுத்து;
		நிலை BOX_INLINE:
		நிலை BOX_INLINE_END:
		நிலை BOX_INLINE_FLEX:
		நிலை BOX_INLINE_BLOCK:
		நிலை BOX_FLOAT_இடது:
		நிலை BOX_FLOAT_வலது:
		நிலை BOX_BR:
		நிலை BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			ப்ரதிஜ்ஞா(0);
			நிறுத்து;
		நிலை BOX_TABLE_ROW_GROUP:
		நிலை BOX_TABLE_ROW:
		நிலை BOX_TABLE_CELL:
			/* insert implied table */
			ப்ரதிஜ்ஞா(block->style != வெற்று);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					block->style);
			எனில் (style == வெற்று)
				திரும்பு false;

			table = box_create(வெற்று, style, true, block->href,
					block->target, வெற்று, வெற்று, c->bctx);
			எனில் (table == வெற்று) {
				css_computed_style_destroy(style);
				திரும்பு false;
			}
			table->type = BOX_TABLE;

			எனில் (child->prev == வெற்று)
				block->children = table;
			இல்லையெனில்
				child->prev->next = table;

			table->prev = child->prev;

			போது (child != வெற்று && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(table, child);

				next_child = child->next;
				child->next = வெற்று;
				child = next_child;
			}

			table->last->next = வெற்று;
			table->next = next_child = child;
			எனில் (table->next != வெற்று)
				table->next->prev = table;
			இல்லையெனில்
				block->last = table;
			table->parent = block;

			எனில் (box_normalise_table(table, வேர், c) == false)
				திரும்பு false;
			நிறுத்து;
		இயல்பு:
			ப்ரதிஜ்ஞா(0);
		}
	}

	திரும்பு true;
}
