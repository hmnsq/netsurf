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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "css/select.h"

#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/table.h"
#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_manipulate.h"
#അന്തര്ഭാവ "html/box_normalise.h"

/* Define to enable box normalise debug */
#undef BOX_NORMALISE_DEBUG

/**
 * Row spanning information for a cell
 */
ഘടന span_info {
	/** Number of rows this cell spans */
	unsigned പൂർണ്ണസംഖ്യ row_span;
	/** Row group of cell */
	ഘടന box *rg;
	/** The cell in this column spans all rows until the end of the table */
	bool auto_row;
};

/**
 * Column record for a table
 */
ഘടന columns {
	/** Current column index */
	unsigned പൂർണ്ണസംഖ്യ current_column;
	/** Number of columns in main part of table 1..max columns */
	unsigned പൂർണ്ണസംഖ്യ അക്കം_columns;
	/** Information about columns in main table, array [0, num_columns) */
	ഘടന span_info *spans;
	/** Number of rows in table */
	unsigned പൂർണ്ണസംഖ്യ അക്കം_rows;
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
സ്ഥിതം bool
calculate_table_row(ഘടന columns *col_info,
		    unsigned പൂർണ്ണസംഖ്യ col_span,
		    unsigned പൂർണ്ണസംഖ്യ row_span,
		    unsigned പൂർണ്ണസംഖ്യ *start_column,
		    ഘടന box *cell)
{
	unsigned പൂർണ്ണസംഖ്യ cell_start_col = col_info->current_column;
	unsigned പൂർണ്ണസംഖ്യ cell_end_col;
	unsigned പൂർണ്ണസംഖ്യ i;
	ഘടന span_info *spans;
	ഘടന box *rg = cell->parent->parent; /* Cell's row group */

	/* Skip columns with cells spanning from above */
	/* TODO: Need to ignore cells spanning from above that belong to
	 *       different row group.  We don't have that info here. */
	ആകുമ്പോൾ (col_info->spans[cell_start_col].row_span != 0 &&
			col_info->spans[cell_start_col].rg == rg) {
		cell_start_col++;
	}

	/* Update current column with calculated start */
	col_info->current_column = cell_start_col;

	/* If this cell has a colspan of 0, then assume 1.
	 * No other browser supports colspan=0, anyway. */
	എങ്കിൽ (col_span == 0)
		col_span = 1;

	cell_end_col = cell_start_col + col_span;

	എങ്കിൽ (col_info->അക്കം_columns < cell_end_col) {
		/* It appears that this row has more columns than
		 * the maximum recorded for the table so far.
		 * Allocate more span records. */
		spans = realloc(col_info->spans,
				വലുപ്പം *spans * (cell_end_col + 1));
		എങ്കിൽ (spans == ശൂന്യ)
			തിരികെ false;

		col_info->spans = spans;
		col_info->അക്കം_columns = cell_end_col;

		/* Mark new final column as sentinel */
		col_info->spans[cell_end_col].row_span = 0;
		col_info->spans[cell_end_col].auto_row = false;
	}

	/* This cell may span multiple columns. If it also wants to span
	 * multiple rows, temporarily assume it spans 1 row only. This will
	 * be fixed up in box_normalise_table_spans() */
	വേണ്ടി (i = cell_start_col; i < cell_end_col; i++) {
		col_info->spans[i].row_span = (row_span == 0) ? 1 : row_span;
		col_info->spans[i].auto_row = (row_span == 0);
		col_info->spans[i].rg = rg;
	}

	/* Update current column with calculated end. */
	col_info->current_column = cell_end_col;

	*start_column = cell_start_col;

	തിരികെ true;
}


സ്ഥിതം bool
box_normalise_table_row(ഘടന box *row,
			സ്ഥിരം ഘടന box *വേര്,
			ഘടന columns *col_info,
			html_content * c)
{
	ഘടന box *child;
	ഘടന box *next_child;
	ഘടന box *cell = ശൂന്യ;
	css_computed_style *style;
	unsigned പൂർണ്ണസംഖ്യ i;
	nscss_select_ctx ctx;

	പ്രതിജ്ഞാ(row != ശൂന്യ);
	പ്രതിജ്ഞാ(row->type == BOX_TABLE_ROW);

	ctx.വേര്_style = വേര്->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p", row);
#endif

	വേണ്ടി (child = row->children; child != ശൂന്യ; child = next_child) {
		next_child = child->next;

		തിരഞ്ഞെടുപ്പ് (child->type) {
		അവസ്ഥ BOX_TABLE_CELL:
			/* ok */
			എങ്കിൽ (box_normalise_block(child, വേര്, c) == false)
				തിരികെ false;
			cell = child;
			നിർത്തുക;
		അവസ്ഥ BOX_FLEX:
		അവസ്ഥ BOX_BLOCK:
		അവസ്ഥ BOX_INLINE_CONTAINER:
		അവസ്ഥ BOX_TABLE:
		അവസ്ഥ BOX_TABLE_ROW_GROUP:
		അവസ്ഥ BOX_TABLE_ROW:
			/* insert implied table cell */
			പ്രതിജ്ഞാ(row->style != ശൂന്യ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row->style);
			എങ്കിൽ (style == ശൂന്യ)
				തിരികെ false;

			cell = box_create(ശൂന്യ, style, true, row->href,
					row->target, ശൂന്യ, ശൂന്യ, c->bctx);
			എങ്കിൽ (cell == ശൂന്യ) {
				css_computed_style_destroy(style);
				തിരികെ false;
			}
			cell->type = BOX_TABLE_CELL;

			എങ്കിൽ (child->prev == ശൂന്യ)
				row->children = cell;
			അല്ലെങ്കിൽ
				child->prev->next = cell;

			cell->prev = child->prev;

			ആകുമ്പോൾ (child != ശൂന്യ && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW)) {
				box_add_child(cell, child);

				next_child = child->next;
				child->next = ശൂന്യ;
				child = next_child;
			}

			പ്രതിജ്ഞാ(cell->last != ശൂന്യ);

			cell->last->next = ശൂന്യ;
			cell->next = next_child = child;
			എങ്കിൽ (cell->next != ശൂന്യ)
				cell->next->prev = cell;
			അല്ലെങ്കിൽ
				row->last = cell;
			cell->parent = row;

			എങ്കിൽ (box_normalise_block(cell, വേര്, c) == false)
				തിരികെ false;
			നിർത്തുക;
		അവസ്ഥ BOX_INLINE:
		അവസ്ഥ BOX_INLINE_END:
		അവസ്ഥ BOX_INLINE_FLEX:
		അവസ്ഥ BOX_INLINE_BLOCK:
		അവസ്ഥ BOX_FLOAT_ഇടത്:
		അവസ്ഥ BOX_FLOAT_വലത്:
		അവസ്ഥ BOX_BR:
		അവസ്ഥ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			പ്രതിജ്ഞാ(0);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			പ്രതിജ്ഞാ(0);
		}

		എങ്കിൽ (calculate_table_row(col_info, cell->columns, cell->rows,
				&cell->start_column, cell) == false)
			തിരികെ false;
	}


	/* Update row spanning details for all columns */
	വേണ്ടി (i = 0; i < col_info->അക്കം_columns; i++) {
		എങ്കിൽ (col_info->spans[i].row_span != 0 &&
				col_info->spans[i].auto_row == false) {
			/* This cell spans rows, and is not an auto row.
			 * Reduce number of rows left to span */
			col_info->spans[i].row_span--;
		}
	}

	/* Reset current column for next row */
	col_info->current_column = 0;

	/* Increment row counter */
	col_info->അക്കം_rows++;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row %p done", row);
#endif

	തിരികെ true;
}


സ്ഥിതം bool
box_normalise_table_row_group(ഘടന box *row_group,
			      സ്ഥിരം ഘടന box *വേര്,
			      ഘടന columns *col_info,
			      html_content * c)
{
	ഘടന box *child;
	ഘടന box *next_child;
	ഘടന box *row;
	css_computed_style *style;
	nscss_select_ctx ctx;
	unsigned പൂർണ്ണസംഖ്യ group_row_എണ്ണം = 0;

	പ്രതിജ്ഞാ(row_group != 0);
	പ്രതിജ്ഞാ(row_group->type == BOX_TABLE_ROW_GROUP);

	ctx.വേര്_style = വേര്->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p", row_group);
#endif

	വേണ്ടി (child = row_group->children; child != ശൂന്യ; child = next_child) {
		next_child = child->next;

		തിരഞ്ഞെടുപ്പ് (child->type) {
		അവസ്ഥ BOX_TABLE_ROW:
			/* ok */
			group_row_എണ്ണം++;
			എങ്കിൽ (box_normalise_table_row(child, വേര്, col_info,
					c) == false)
				തിരികെ false;
			നിർത്തുക;
		അവസ്ഥ BOX_FLEX:
		അവസ്ഥ BOX_BLOCK:
		അവസ്ഥ BOX_INLINE_CONTAINER:
		അവസ്ഥ BOX_TABLE:
		അവസ്ഥ BOX_TABLE_ROW_GROUP:
		അവസ്ഥ BOX_TABLE_CELL:
			/* insert implied table row */
			പ്രതിജ്ഞാ(row_group->style != ശൂന്യ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					row_group->style);
			എങ്കിൽ (style == ശൂന്യ)
				തിരികെ false;

			row = box_create(ശൂന്യ, style, true, row_group->href,
					row_group->target, ശൂന്യ, ശൂന്യ, c->bctx);
			എങ്കിൽ (row == ശൂന്യ) {
				css_computed_style_destroy(style);
				തിരികെ false;
			}
			row->type = BOX_TABLE_ROW;

			എങ്കിൽ (child->prev == ശൂന്യ)
				row_group->children = row;
			അല്ലെങ്കിൽ
				child->prev->next = row;

			row->prev = child->prev;

			ആകുമ്പോൾ (child != ശൂന്യ && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row, child);

				next_child = child->next;
				child->next = ശൂന്യ;
				child = next_child;
			}

			പ്രതിജ്ഞാ(row->last != ശൂന്യ);

			row->last->next = ശൂന്യ;
			row->next = next_child = child;
			എങ്കിൽ (row->next != ശൂന്യ)
				row->next->prev = row;
			അല്ലെങ്കിൽ
				row_group->last = row;
			row->parent = row_group;

			group_row_എണ്ണം++;
			എങ്കിൽ (box_normalise_table_row(row, വേര്, col_info,
					c) == false)
				തിരികെ false;
			നിർത്തുക;
		അവസ്ഥ BOX_INLINE:
		അവസ്ഥ BOX_INLINE_END:
		അവസ്ഥ BOX_INLINE_FLEX:
		അവസ്ഥ BOX_INLINE_BLOCK:
		അവസ്ഥ BOX_FLOAT_ഇടത്:
		അവസ്ഥ BOX_FLOAT_വലത്:
		അവസ്ഥ BOX_BR:
		അവസ്ഥ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			പ്രതിജ്ഞാ(0);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			പ്രതിജ്ഞാ(0);
		}
	}

	എങ്കിൽ (row_group->children == ശൂന്യ) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "row_group->children == 0, inserting implied row");
#endif

		പ്രതിജ്ഞാ(row_group->style != ശൂന്യ);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		എങ്കിൽ (style == ശൂന്യ) {
			തിരികെ false;
		}

		row = box_create(ശൂന്യ, style, true, row_group->href,
				row_group->target, ശൂന്യ, ശൂന്യ, c->bctx);
		എങ്കിൽ (row == ശൂന്യ) {
			css_computed_style_destroy(style);
			തിരികെ false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		group_row_എണ്ണം = 1;

		/* Keep table's row count in sync */
		col_info->അക്കം_rows++;
	}

	row_group->rows = group_row_എണ്ണം;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "row_group %p done", row_group);
#endif

	തിരികെ true;
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
സ്ഥിതം bool
box_normalise_table_spans(ഘടന box *table,
			  സ്ഥിരം ഘടന box *വേര്,
			  ഘടന span_info *spans,
			  html_content *c)
{
	ഘടന box *table_row_group;
	ഘടന box *table_row;
	ഘടന box *table_cell;
	unsigned പൂർണ്ണസംഖ്യ rows_ഇടത് = table->rows;
	unsigned പൂർണ്ണസംഖ്യ group_rows_ഇടത്;
	unsigned പൂർണ്ണസംഖ്യ col;
	nscss_select_ctx ctx;

	ctx.വേര്_style = വേര്->style;

	/* Clear span data */
	memset(spans, 0, table->columns * വലുപ്പം(ഘടന span_info));

	/* Scan table, filling in width and height of table cells with
	 * colspan = 0 and rowspan = 0. Also generate empty cells */
	വേണ്ടി (table_row_group = table->children;
	     table_row_group != ശൂന്യ;
	     table_row_group = table_row_group->next) {

		group_rows_ഇടത് = table_row_group->rows;

		വേണ്ടി (table_row = table_row_group->children;
		     table_row != ശൂന്യ;
		     table_row = table_row->next) {

			വേണ്ടി (table_cell = table_row->children;
			     table_cell != ശൂന്യ;
			     table_cell = table_cell->next) {

				/* colspan = 0 -> colspan = 1 */
				എങ്കിൽ (table_cell->columns == 0) {
					table_cell->columns = 1;
				}

				/* if rowspan is 0 it is expanded to
				 * the number of rows left in the row
				 * group
				 */
				എങ്കിൽ (table_cell->rows == 0) {
					table_cell->rows = group_rows_ഇടത്;
				}

				/* limit rowspans within group */
				എങ്കിൽ (table_cell->rows > group_rows_ഇടത്) {
					table_cell->rows = group_rows_ഇടത്;
				}

				/* Record span information */
				വേണ്ടി (col = table_cell->start_column;
						col < table_cell->start_column +
						table_cell->columns; col++) {
					spans[col].row_span = table_cell->rows;
				}
			}

			/* Reduce span count of each column */
			വേണ്ടി (col = 0; col < table->columns; col++) {
				എങ്കിൽ (spans[col].row_span == 0) {
					unsigned പൂർണ്ണസംഖ്യ start = col;
					css_computed_style *style;
					ഘടന box *cell, *prev;

					/* If it's already zero, then we need
					 * to generate an empty cell for the
					 * gap in the row that spans as many
					 * columns as remain blank.
					 */
					പ്രതിജ്ഞാ(table_row->style != ശൂന്യ);

					/* Find width of gap */
					ആകുമ്പോൾ (col < table->columns &&
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
					എങ്കിൽ (style == ശൂന്യ)
						തിരികെ false;

					cell = box_create(ശൂന്യ, style, true,
							table_row->href,
							table_row->target,
							ശൂന്യ, ശൂന്യ, c->bctx);
					എങ്കിൽ (cell == ശൂന്യ) {
						css_computed_style_destroy(
								style);
						തിരികെ false;
					}
					cell->type = BOX_TABLE_CELL;

					cell->rows = 1;
					cell->columns = col - start;
					cell->start_column = start;

					/* Find place to insert cell */
					വേണ്ടി (prev = table_row->children;
							prev != ശൂന്യ;
							prev = prev->next) {
						എങ്കിൽ (prev->start_column +
							prev->columns ==
								start)
							നിർത്തുക;
						എങ്കിൽ (prev->next == ശൂന്യ)
							നിർത്തുക;
					}

					/* Insert it */
					എങ്കിൽ (prev == ശൂന്യ) {
						എങ്കിൽ (table_row->children != ശൂന്യ)
							table_row->children->
								prev = cell;
						അല്ലെങ്കിൽ
							table_row->last = cell;

						cell->next =
							table_row->children;
						table_row->children = cell;
					} അല്ലെങ്കിൽ {
						എങ്കിൽ (prev->next != ശൂന്യ)
							prev->next->prev = cell;
						അല്ലെങ്കിൽ
							table_row->last = cell;

						cell->next = prev->next;
						prev->next = cell;
						cell->prev = prev;
					}
					cell->parent = table_row;
				} അല്ലെങ്കിൽ {
					spans[col].row_span--;
				}
			}

			പ്രതിജ്ഞാ(rows_ഇടത് > 0);

			rows_ഇടത്--;
		}

		group_rows_ഇടത്--;
	}

	തിരികെ true;
}


സ്ഥിതം bool
box_normalise_table(ഘടന box *table, സ്ഥിരം ഘടന box *വേര്, html_content * c)
{
	ഘടന box *child;
	ഘടന box *next_child;
	ഘടന box *row_group;
	css_computed_style *style;
	ഘടന columns col_info;
	nscss_select_ctx ctx;

	പ്രതിജ്ഞാ(table != ശൂന്യ);
	പ്രതിജ്ഞാ(table->type == BOX_TABLE);

	ctx.വേര്_style = വേര്->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p", table);
#endif

	col_info.അക്കം_columns = 1;
	col_info.current_column = 0;
	col_info.spans = malloc(2 * വലുപ്പം *col_info.spans);
	എങ്കിൽ (col_info.spans == ശൂന്യ)
		തിരികെ false;

	col_info.spans[0].row_span = col_info.spans[1].row_span = 0;
	col_info.spans[0].auto_row = false;
	col_info.spans[1].auto_row = false;
	col_info.അക്കം_rows = 0;

	വേണ്ടി (child = table->children; child != ശൂന്യ; child = next_child) {
		next_child = child->next;
		തിരഞ്ഞെടുപ്പ് (child->type) {
		അവസ്ഥ BOX_TABLE_ROW_GROUP:
			/* ok */
			എങ്കിൽ (box_normalise_table_row_group(child, വേര്,
					&col_info, c) == false) {
				സ്വതന്ത്രം(col_info.spans);
				തിരികെ false;
			}
			നിർത്തുക;
		അവസ്ഥ BOX_FLEX:
		അവസ്ഥ BOX_BLOCK:
		അവസ്ഥ BOX_INLINE_CONTAINER:
		അവസ്ഥ BOX_TABLE:
		അവസ്ഥ BOX_TABLE_ROW:
		അവസ്ഥ BOX_TABLE_CELL:
			/* insert implied table row group */
			പ്രതിജ്ഞാ(table->style != ശൂന്യ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					table->style);
			എങ്കിൽ (style == ശൂന്യ) {
				സ്വതന്ത്രം(col_info.spans);
				തിരികെ false;
			}

			row_group = box_create(ശൂന്യ, style, true, table->href,
					table->target, ശൂന്യ, ശൂന്യ, c->bctx);
			എങ്കിൽ (row_group == ശൂന്യ) {
				css_computed_style_destroy(style);
				സ്വതന്ത്രം(col_info.spans);
				തിരികെ false;
			}

			row_group->type = BOX_TABLE_ROW_GROUP;

			എങ്കിൽ (child->prev == ശൂന്യ)
				table->children = row_group;
			അല്ലെങ്കിൽ
				child->prev->next = row_group;

			row_group->prev = child->prev;

			ആകുമ്പോൾ (child != ശൂന്യ && (
					child->type == BOX_FLEX ||
					child->type == BOX_BLOCK ||
					child->type == BOX_INLINE_CONTAINER ||
					child->type == BOX_TABLE ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(row_group, child);

				next_child = child->next;
				child->next = ശൂന്യ;
				child = next_child;
			}

			പ്രതിജ്ഞാ(row_group->last != ശൂന്യ);

			row_group->last->next = ശൂന്യ;
			row_group->next = next_child = child;
			എങ്കിൽ (row_group->next != ശൂന്യ)
				row_group->next->prev = row_group;
			അല്ലെങ്കിൽ
				table->last = row_group;
			row_group->parent = table;

			എങ്കിൽ (box_normalise_table_row_group(row_group, വേര്,
					&col_info, c) == false) {
				സ്വതന്ത്രം(col_info.spans);
				തിരികെ false;
			}
			നിർത്തുക;
		അവസ്ഥ BOX_INLINE:
		അവസ്ഥ BOX_INLINE_END:
		അവസ്ഥ BOX_INLINE_FLEX:
		അവസ്ഥ BOX_INLINE_BLOCK:
		അവസ്ഥ BOX_FLOAT_ഇടത്:
		അവസ്ഥ BOX_FLOAT_വലത്:
		അവസ്ഥ BOX_BR:
		അവസ്ഥ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			പ്രതിജ്ഞാ(0);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			fprintf(stderr, "%i\n", child->type);
			പ്രതിജ്ഞാ(0);
		}
	}

	table->columns = col_info.അക്കം_columns;
	table->rows = col_info.അക്കം_rows;

	എങ്കിൽ (table->children == ശൂന്യ) {
		ഘടന box *row;

#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO,
		      "table->children == 0, creating implied row");
#endif

		പ്രതിജ്ഞാ(table->style != ശൂന്യ);

		ctx.ctx = c->select_ctx;
		ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
		ctx.base_url = c->base_url;
		ctx.universal = c->universal;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				table->style);
		എങ്കിൽ (style == ശൂന്യ) {
			സ്വതന്ത്രം(col_info.spans);
			തിരികെ false;
		}

		row_group = box_create(ശൂന്യ, style, true, table->href,
				table->target, ശൂന്യ, ശൂന്യ, c->bctx);
		എങ്കിൽ (row_group == ശൂന്യ) {
			css_computed_style_destroy(style);
			സ്വതന്ത്രം(col_info.spans);
			തിരികെ false;
		}
		row_group->type = BOX_TABLE_ROW_GROUP;

		style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
				row_group->style);
		എങ്കിൽ (style == ശൂന്യ) {
			box_free(row_group);
			സ്വതന്ത്രം(col_info.spans);
			തിരികെ false;
		}

		row = box_create(ശൂന്യ, style, true, row_group->href,
				row_group->target, ശൂന്യ, ശൂന്യ, c->bctx);
		എങ്കിൽ (row == ശൂന്യ) {
			css_computed_style_destroy(style);
			box_free(row_group);
			സ്വതന്ത്രം(col_info.spans);
			തിരികെ false;
		}
		row->type = BOX_TABLE_ROW;

		row->parent = row_group;
		row_group->children = row_group->last = row;

		row_group->parent = table;
		table->children = table->last = row_group;

		table->rows = 1;
	}

	എങ്കിൽ (box_normalise_table_spans(table, വേര്, col_info.spans, c) == false) {
		സ്വതന്ത്രം(col_info.spans);
		തിരികെ false;
	}

	സ്വതന്ത്രം(col_info.spans);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "table %p done", table);
#endif

	തിരികെ true;
}

സ്ഥിതം bool box_normalise_flex(
		ഘടന box *flex_container,
		സ്ഥിരം ഘടന box *വേര്,
		html_content *c)
{
	ഘടന box *child;
	ഘടന box *next_child;
	ഘടന box *implied_flex_item;
	css_computed_style *style;
	nscss_select_ctx ctx;

	പ്രതിജ്ഞാ(flex_container != ശൂന്യ);
	പ്രതിജ്ഞാ(വേര് != ശൂന്യ);

	ctx.വേര്_style = വേര്->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "flex_container %p, flex_container->type %u",
			flex_container, flex_container->type);
#endif

	പ്രതിജ്ഞാ(flex_container->type == BOX_FLEX ||
	       flex_container->type == BOX_INLINE_FLEX);

	വേണ്ടി (child = flex_container->children; child != ശൂന്യ; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d",
				child, child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		തിരഞ്ഞെടുപ്പ് (child->type) {
		അവസ്ഥ BOX_FLEX:
			/* ok */
			എങ്കിൽ (box_normalise_flex(child, വേര്, c) == false)
				തിരികെ false;
			നിർത്തുക;
		അവസ്ഥ BOX_BLOCK:
			/* ok */
			എങ്കിൽ (box_normalise_block(child, വേര്, c) == false)
				തിരികെ false;
			നിർത്തുക;
		അവസ്ഥ BOX_INLINE_CONTAINER:
			/* insert implied flex item */
			പ്രതിജ്ഞാ(flex_container->style != ശൂന്യ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			എങ്കിൽ (style == ശൂന്യ)
				തിരികെ false;

			implied_flex_item = box_create(ശൂന്യ, style, true,
					flex_container->href,
					flex_container->target,
					ശൂന്യ, ശൂന്യ, c->bctx);
			എങ്കിൽ (implied_flex_item == ശൂന്യ) {
				css_computed_style_destroy(style);
				തിരികെ false;
			}
			implied_flex_item->type = BOX_BLOCK;

			എങ്കിൽ (child->prev == ശൂന്യ)
				flex_container->children = implied_flex_item;
			അല്ലെങ്കിൽ
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			ആകുമ്പോൾ (child != ശൂന്യ &&
					child->type == BOX_INLINE_CONTAINER) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = ശൂന്യ;
				child = next_child;
			}

			implied_flex_item->last->next = ശൂന്യ;
			implied_flex_item->next = next_child = child;
			എങ്കിൽ (implied_flex_item->next != ശൂന്യ)
				implied_flex_item->next->prev = implied_flex_item;
			അല്ലെങ്കിൽ
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			എങ്കിൽ (box_normalise_block(implied_flex_item,
					വേര്, c) == false)
				തിരികെ false;
			നിർത്തുക;

		അവസ്ഥ BOX_TABLE:
			എങ്കിൽ (box_normalise_table(child, വേര്, c) == false)
				തിരികെ false;
			നിർത്തുക;
		അവസ്ഥ BOX_INLINE:
		അവസ്ഥ BOX_INLINE_END:
		അവസ്ഥ BOX_INLINE_FLEX:
		അവസ്ഥ BOX_INLINE_BLOCK:
		അവസ്ഥ BOX_FLOAT_ഇടത്:
		അവസ്ഥ BOX_FLOAT_വലത്:
		അവസ്ഥ BOX_BR:
		അവസ്ഥ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			പ്രതിജ്ഞാ(0);
			നിർത്തുക;
		അവസ്ഥ BOX_TABLE_ROW_GROUP:
		അവസ്ഥ BOX_TABLE_ROW:
		അവസ്ഥ BOX_TABLE_CELL:
			/* insert implied table */
			പ്രതിജ്ഞാ(flex_container->style != ശൂന്യ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					flex_container->style);
			എങ്കിൽ (style == ശൂന്യ)
				തിരികെ false;

			implied_flex_item = box_create(ശൂന്യ, style, true,
					flex_container->href,
					flex_container->target,
					ശൂന്യ, ശൂന്യ, c->bctx);
			എങ്കിൽ (implied_flex_item == ശൂന്യ) {
				css_computed_style_destroy(style);
				തിരികെ false;
			}
			implied_flex_item->type = BOX_TABLE;

			എങ്കിൽ (child->prev == ശൂന്യ)
				flex_container->children = implied_flex_item;
			അല്ലെങ്കിൽ
				child->prev->next = implied_flex_item;

			implied_flex_item->prev = child->prev;

			ആകുമ്പോൾ (child != ശൂന്യ && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(implied_flex_item, child);

				next_child = child->next;
				child->next = ശൂന്യ;
				child = next_child;
			}

			implied_flex_item->last->next = ശൂന്യ;
			implied_flex_item->next = next_child = child;
			എങ്കിൽ (implied_flex_item->next != ശൂന്യ)
				implied_flex_item->next->prev = implied_flex_item;
			അല്ലെങ്കിൽ
				flex_container->last = implied_flex_item;
			implied_flex_item->parent = flex_container;

			എങ്കിൽ (box_normalise_table(implied_flex_item,
					വേര്, c) == false)
				തിരികെ false;
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			പ്രതിജ്ഞാ(0);
		}
	}

	തിരികെ true;
}

സ്ഥിതം bool
box_normalise_inline_container(ഘടന box *cont,
			       സ്ഥിരം ഘടന box *വേര്,
			       html_content * c)
{
	ഘടന box *child;
	ഘടന box *next_child;

	പ്രതിജ്ഞാ(cont != ശൂന്യ);
	പ്രതിജ്ഞാ(cont->type == BOX_INLINE_CONTAINER);

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p", cont);
#endif

	വേണ്ടി (child = cont->children; child != ശൂന്യ; child = next_child) {
		next_child = child->next;
		തിരഞ്ഞെടുപ്പ് (child->type) {
		അവസ്ഥ BOX_INLINE:
		അവസ്ഥ BOX_INLINE_END:
		അവസ്ഥ BOX_BR:
		അവസ്ഥ BOX_TEXT:
			/* ok */
			നിർത്തുക;
		അവസ്ഥ BOX_INLINE_BLOCK:
			/* ok */
			എങ്കിൽ (box_normalise_block(child, വേര്, c) == false)
				തിരികെ false;
			നിർത്തുക;
		അവസ്ഥ BOX_INLINE_FLEX:
			/* ok */
			എങ്കിൽ (box_normalise_flex(child, വേര്, c) == false)
				തിരികെ false;
			നിർത്തുക;
		അവസ്ഥ BOX_FLOAT_ഇടത്:
		അവസ്ഥ BOX_FLOAT_വലത്:
			/* ok */
			പ്രതിജ്ഞാ(child->children != ശൂന്യ);

			തിരഞ്ഞെടുപ്പ് (child->children->type) {
			അവസ്ഥ BOX_BLOCK:
				എങ്കിൽ (box_normalise_block(child->children, വേര്,
						c) == false)
					തിരികെ false;
				നിർത്തുക;
			അവസ്ഥ BOX_TABLE:
				എങ്കിൽ (box_normalise_table(child->children, വേര്,
						c) == false)
					തിരികെ false;
				നിർത്തുക;
			അവസ്ഥ BOX_FLEX:
				എങ്കിൽ (box_normalise_flex(child->children, വേര്,
						c) == false)
					തിരികെ false;
				നിർത്തുക;
			സ്ഥിരസ്ഥിതി:
				പ്രതിജ്ഞാ(0);
			}

			എങ്കിൽ (child->children == ശൂന്യ) {
				/* the child has destroyed itself: remove float */
				എങ്കിൽ (child->prev == ശൂന്യ)
					child->parent->children = child->next;
				അല്ലെങ്കിൽ
					child->prev->next = child->next;
				എങ്കിൽ (child->next != ശൂന്യ)
					child->next->prev = child->prev;
				അല്ലെങ്കിൽ
					child->parent->last = child->prev;

				box_free(child);
			}
			നിർത്തുക;
		അവസ്ഥ BOX_FLEX:
		അവസ്ഥ BOX_BLOCK:
		അവസ്ഥ BOX_INLINE_CONTAINER:
		അവസ്ഥ BOX_TABLE:
		അവസ്ഥ BOX_TABLE_ROW_GROUP:
		അവസ്ഥ BOX_TABLE_ROW:
		അവസ്ഥ BOX_TABLE_CELL:
		സ്ഥിരസ്ഥിതി:
			പ്രതിജ്ഞാ(0);
		}
	}

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "cont %p done", cont);
#endif

	തിരികെ true;
}

/* Exported function documented in html/box_normalise.h */
bool
box_normalise_block(ഘടന box *block, സ്ഥിരം ഘടന box *വേര്, html_content *c)
{
	ഘടന box *child;
	ഘടന box *next_child;
	ഘടന box *table;
	css_computed_style *style;
	nscss_select_ctx ctx;

	പ്രതിജ്ഞാ(block != ശൂന്യ);
	പ്രതിജ്ഞാ(വേര് != ശൂന്യ);

	ctx.വേര്_style = വേര്->style;

#ifdef BOX_NORMALISE_DEBUG
	NSLOG(netsurf, INFO, "block %p, block->type %u", block, block->type);
#endif

	പ്രതിജ്ഞാ(block->type == BOX_BLOCK || block->type == BOX_INLINE_BLOCK ||
			block->type == BOX_TABLE_CELL);

	വേണ്ടി (child = block->children; child != ശൂന്യ; child = next_child) {
#ifdef BOX_NORMALISE_DEBUG
		NSLOG(netsurf, INFO, "child %p, child->type = %d", child,
		      child->type);
#endif

		next_child = child->next;	/* child may be destroyed */

		തിരഞ്ഞെടുപ്പ് (child->type) {
		അവസ്ഥ BOX_FLEX:
			/* ok */
			എങ്കിൽ (box_normalise_flex(child, വേര്, c) == false)
				തിരികെ false;
			നിർത്തുക;
		അവസ്ഥ BOX_BLOCK:
			/* ok */
			എങ്കിൽ (box_normalise_block(child, വേര്, c) == false)
				തിരികെ false;
			നിർത്തുക;
		അവസ്ഥ BOX_INLINE_CONTAINER:
			എങ്കിൽ (box_normalise_inline_container(child, വേര്, c) == false)
				തിരികെ false;
			നിർത്തുക;
		അവസ്ഥ BOX_TABLE:
			എങ്കിൽ (box_normalise_table(child, വേര്, c) == false)
				തിരികെ false;
			നിർത്തുക;
		അവസ്ഥ BOX_INLINE:
		അവസ്ഥ BOX_INLINE_END:
		അവസ്ഥ BOX_INLINE_FLEX:
		അവസ്ഥ BOX_INLINE_BLOCK:
		അവസ്ഥ BOX_FLOAT_ഇടത്:
		അവസ്ഥ BOX_FLOAT_വലത്:
		അവസ്ഥ BOX_BR:
		അവസ്ഥ BOX_TEXT:
			/* should have been wrapped in inline
			   container by convert_xml_to_box() */
			പ്രതിജ്ഞാ(0);
			നിർത്തുക;
		അവസ്ഥ BOX_TABLE_ROW_GROUP:
		അവസ്ഥ BOX_TABLE_ROW:
		അവസ്ഥ BOX_TABLE_CELL:
			/* insert implied table */
			പ്രതിജ്ഞാ(block->style != ശൂന്യ);

			ctx.ctx = c->select_ctx;
			ctx.quirks = (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL);
			ctx.base_url = c->base_url;
			ctx.universal = c->universal;

			style = nscss_get_blank_style(&ctx, &c->unit_len_ctx,
					block->style);
			എങ്കിൽ (style == ശൂന്യ)
				തിരികെ false;

			table = box_create(ശൂന്യ, style, true, block->href,
					block->target, ശൂന്യ, ശൂന്യ, c->bctx);
			എങ്കിൽ (table == ശൂന്യ) {
				css_computed_style_destroy(style);
				തിരികെ false;
			}
			table->type = BOX_TABLE;

			എങ്കിൽ (child->prev == ശൂന്യ)
				block->children = table;
			അല്ലെങ്കിൽ
				child->prev->next = table;

			table->prev = child->prev;

			ആകുമ്പോൾ (child != ശൂന്യ && (
					child->type == BOX_TABLE_ROW_GROUP ||
					child->type == BOX_TABLE_ROW ||
					child->type == BOX_TABLE_CELL)) {
				box_add_child(table, child);

				next_child = child->next;
				child->next = ശൂന്യ;
				child = next_child;
			}

			table->last->next = ശൂന്യ;
			table->next = next_child = child;
			എങ്കിൽ (table->next != ശൂന്യ)
				table->next->prev = table;
			അല്ലെങ്കിൽ
				block->last = table;
			table->parent = block;

			എങ്കിൽ (box_normalise_table(table, വേര്, c) == false)
				തിരികെ false;
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			പ്രതിജ്ഞാ(0);
		}
	}

	തിരികെ true;
}
